#include "net.h"
#include "e1000.h"
#include "rtl8139.h"

#define FRAME_MAX 1518
#define IP_OFFSET 14
#define ETH_ARP 0x0806
#define ETH_IPV4 0x0800
#define IP_UDP 17
#define IP_TCP 6
#define LOCAL_PORT 49152
#define NET_WAIT_POLLS 50000000

static uint8_t mac_address[6];
static uint8_t local_ip[4];
static uint8_t subnet_mask[4] = {255, 255, 255, 0};
static uint8_t gateway_ip[4];
static uint8_t dns_ip[4];
static uint8_t gateway_mac[6];
static uint8_t configured;
static uint32_t next_ip_id = 1;
static uint32_t tcp_sequence;
static uint16_t ephemeral_port = LOCAL_PORT;
static uint8_t frame[FRAME_MAX];

static int (*net_send_fn)(const void *, uint16_t) = e1000_send;
static int (*net_receive_fn)(void *, uint16_t) = e1000_receive;

static uint16_t read16(const uint8_t *p) { return ((uint16_t)p[0] << 8) | p[1]; }
static uint32_t read32(const uint8_t *p) { return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | p[3]; }
static void write16(uint8_t *p, uint16_t v) { p[0] = (uint8_t)(v >> 8); p[1] = (uint8_t)v; }
static void write32(uint8_t *p, uint32_t v) { p[0] = (uint8_t)(v >> 24); p[1] = (uint8_t)(v >> 16); p[2] = (uint8_t)(v >> 8); p[3] = (uint8_t)v; }
static void copy_bytes(uint8_t *dst, const uint8_t *src, uint32_t n) { for (uint32_t i = 0; i < n; i++) dst[i] = src[i]; }
static void zero_bytes(uint8_t *dst, uint32_t n) { for (uint32_t i = 0; i < n; i++) dst[i] = 0; }
static int equal_bytes(const uint8_t *a, const uint8_t *b, uint32_t n) { for (uint32_t i = 0; i < n; i++) if (a[i] != b[i]) return 0; return 1; }

static uint16_t checksum(const uint8_t *data, uint32_t length)
{
    uint32_t sum = 0;
    while (length > 1) { sum += ((uint16_t)data[0] << 8) | data[1]; data += 2; length -= 2; }
    if (length) sum += (uint16_t)data[0] << 8;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)~sum;
}

static uint16_t transport_checksum(const uint8_t *ip, const uint8_t *segment, uint16_t length, uint8_t protocol)
{
    uint32_t sum = 0;
    uint16_t remaining = length;
    sum += read16(ip + 12); sum += read16(ip + 14);
    sum += read16(ip + 16); sum += read16(ip + 18);
    sum += protocol; sum += length;
    while (remaining > 1) { sum += ((uint16_t)segment[0] << 8) | segment[1]; segment += 2; remaining -= 2; }
    if (remaining) sum += (uint16_t)segment[0] << 8;
    while (sum >> 16) sum = (sum & 0xFFFF) + (sum >> 16);
    return (uint16_t)~sum;
}

static void ethernet_header(uint8_t *packet, const uint8_t *destination, uint16_t type)
{
    copy_bytes(packet, destination, 6); copy_bytes(packet + 6, mac_address, 6); write16(packet + 12, type);
}

static void ip_header(uint8_t *ip, uint8_t protocol, uint16_t length, const uint8_t *source, const uint8_t *destination)
{
    zero_bytes(ip, 20); ip[0] = 0x45; write16(ip + 2, length); write16(ip + 4, (uint16_t)next_ip_id++);
    write16(ip + 6, 0x4000); ip[8] = 64; ip[9] = protocol;
    copy_bytes(ip + 12, source, 4); copy_bytes(ip + 16, destination, 4); write16(ip + 10, checksum(ip, 20));
}

static void send_arp_request(const uint8_t target[4])
{
    uint8_t packet[42];
    static const uint8_t broadcast[6] = {255,255,255,255,255,255};
    ethernet_header(packet, broadcast, ETH_ARP);
    write16(packet + 14, 1); write16(packet + 16, ETH_IPV4); packet[18] = 6; packet[19] = 4;
    write16(packet + 20, 1); copy_bytes(packet + 22, mac_address, 6); copy_bytes(packet + 28, local_ip, 4);
    zero_bytes(packet + 32, 6); copy_bytes(packet + 38, target, 4);
    net_send_fn(packet, sizeof(packet));
}

static int resolve_arp(const uint8_t target[4], uint8_t result[6])
{
    send_arp_request(target);
    for (uint32_t tries = 0; tries < NET_WAIT_POLLS; tries++) {
        int length = net_receive_fn(frame, sizeof(frame));
        if (length < 42 || read16(frame + 12) != ETH_ARP) continue;
        if (read16(frame + 20) == 2 && equal_bytes(frame + 38, local_ip, 4) && equal_bytes(frame + 28, target, 4)) {
            copy_bytes(result, frame + 22, 6); return 1;
        }
    }
    return 0;
}

static int resolve_next_hop(const uint8_t destination[4], uint8_t result[6])
{
    uint8_t next_hop[4];
    uint8_t same_subnet = 1;
    for (uint32_t i = 0; i < 4; i++) {
        if ((local_ip[i] & subnet_mask[i]) != (destination[i] & subnet_mask[i])) {
            same_subnet = 0;
            break;
        }
    }
    copy_bytes(next_hop, same_subnet ? destination : gateway_ip, 4);
    return resolve_arp(next_hop, result);
}

static void make_udp_packet(uint8_t *packet, const uint8_t *destination_mac,
                            const uint8_t source_ip[4], const uint8_t destination_ip[4],
                            uint16_t source_port, uint16_t destination_port,
                            const uint8_t *payload, uint16_t payload_length)
{
    uint16_t udp_length = 8 + payload_length;
    ethernet_header(packet, destination_mac, ETH_IPV4);
    ip_header(packet + 14, IP_UDP, 20 + udp_length, source_ip, destination_ip);
    uint8_t *udp = packet + 34;
    write16(udp, source_port); write16(udp + 2, destination_port); write16(udp + 4, udp_length); write16(udp + 6, 0);
    copy_bytes(udp + 8, payload, payload_length);
    uint16_t sum = transport_checksum(packet + 14, udp, udp_length, IP_UDP);
    write16(udp + 6, sum ? sum : 0xFFFF);
    net_send_fn(packet, 14 + 20 + udp_length);
}

static int parse_dhcp_options(const uint8_t *options, uint32_t length, uint8_t wanted,
                              uint8_t *value, uint8_t *value_length)
{
    uint32_t offset = 0;
    while (offset < length) {
        uint8_t code = options[offset++];
        if (code == 255) break;
        if (code == 0) continue;
        if (offset >= length) break;
        uint8_t size = options[offset++];
        if (offset + size > length) break;
        if (code == wanted) {
            *value_length = size;
            copy_bytes(value, options + offset, size);
            return 1;
        }
        offset += size;
    }
    return 0;
}

static int dhcp_configure(void)
{
    static const uint8_t broadcast_mac[6] = {255,255,255,255,255,255};
    uint8_t dhcp[300];
    uint8_t zero_ip[4] = {0,0,0,0};
    uint8_t broadcast_ip[4] = {255,255,255,255};
    uint32_t xid = 0x74617269;

    zero_bytes(dhcp, sizeof(dhcp));
    dhcp[0] = 1; dhcp[1] = 1; dhcp[2] = 6; write32(dhcp + 4, xid); write16(dhcp + 10, 0x8000);
    copy_bytes(dhcp + 28, mac_address, 6); write32(dhcp + 236, 0x63825363);
    uint32_t p = 240; dhcp[p++] = 53; dhcp[p++] = 1; dhcp[p++] = 1;
    dhcp[p++] = 55; dhcp[p++] = 4; dhcp[p++] = 1; dhcp[p++] = 3; dhcp[p++] = 6; dhcp[p++] = 51;
    dhcp[p++] = 255;
    make_udp_packet(frame, broadcast_mac, zero_ip, broadcast_ip, 68, 67, dhcp, (uint16_t)p);

    uint8_t offered[4] = {0,0,0,0};
    uint8_t server[4] = {0,0,0,0};
    int got_offer = 0;
    for (uint32_t tries = 0; tries < NET_WAIT_POLLS && !got_offer; tries++) {
        int length = net_receive_fn(frame, sizeof(frame));
        if (length < 14 + 20 + 8 + 240 || read16(frame + 12) != ETH_IPV4 || frame[23] != IP_UDP) continue;
        uint8_t *udp = frame + 34;
        if (read16(udp) != 67 || read16(udp + 2) != 68) continue;
        uint8_t *reply = udp + 8;
        if (reply[0] != 2 || read32(reply + 4) != xid || read32(reply + 236) != 0x63825363) continue;
        uint8_t message_type = 0, size = 0;
        if (!parse_dhcp_options(reply + 240, length - 14 - 20 - 8 - 240, 53, &message_type, &size) || size != 1 || message_type != 2) continue;
        copy_bytes(offered, reply + 16, 4);
        parse_dhcp_options(reply + 240, length - 14 - 20 - 8 - 240, 54, server, &size);
        got_offer = 1;
    }
    if (!got_offer) return 0;

    zero_bytes(dhcp, sizeof(dhcp)); dhcp[0] = 1; dhcp[1] = 1; dhcp[2] = 6;
    write32(dhcp + 4, xid); write16(dhcp + 10, 0x8000); copy_bytes(dhcp + 28, mac_address, 6);
    write32(dhcp + 236, 0x63825363); p = 240;
    dhcp[p++] = 53; dhcp[p++] = 1; dhcp[p++] = 3;
    dhcp[p++] = 50; dhcp[p++] = 4; copy_bytes(dhcp + p, offered, 4); p += 4;
    dhcp[p++] = 54; dhcp[p++] = 4; copy_bytes(dhcp + p, server, 4); p += 4;
    dhcp[p++] = 255;
    make_udp_packet(frame, broadcast_mac, zero_ip, broadcast_ip, 68, 67, dhcp, (uint16_t)p);

    for (uint32_t tries = 0; tries < NET_WAIT_POLLS; tries++) {
        int length = net_receive_fn(frame, sizeof(frame));
        if (length < 14 + 20 + 8 + 240 || read16(frame + 12) != ETH_IPV4 || frame[23] != IP_UDP) continue;
        uint8_t *udp = frame + 34;
        if (read16(udp) != 67 || read16(udp + 2) != 68) continue;
        uint8_t *reply = udp + 8;
        if (reply[0] != 2 || read32(reply + 4) != xid || read32(reply + 236) != 0x63825363) continue;
        uint8_t message_type = 0, size = 0;
        if (!parse_dhcp_options(reply + 240, length - 14 - 20 - 8 - 240, 53, &message_type, &size) || size != 1 || message_type != 5) continue;
        copy_bytes(local_ip, reply + 16, 4);
        if (parse_dhcp_options(reply + 240, length - 14 - 20 - 8 - 240, 1, subnet_mask, &size) && size >= 4) { }
        if (parse_dhcp_options(reply + 240, length - 14 - 20 - 8 - 240, 3, gateway_ip, &size) && size >= 4) { }
        if (parse_dhcp_options(reply + 240, length - 14 - 20 - 8 - 240, 6, dns_ip, &size) && size >= 4) { }
        if (gateway_ip[0] == 0) copy_bytes(gateway_ip, server, 4);
        if (dns_ip[0] == 0) copy_bytes(dns_ip, server, 4);
        return 1;
    }
    return 0;
}

int net_init(void)
{
    if (e1000_init() && e1000_get_mac(mac_address)) {
        net_send_fn = e1000_send;
        net_receive_fn = e1000_receive;
    } else if (rtl8139_init() && rtl8139_get_mac(mac_address)) {
        net_send_fn = rtl8139_send;
        net_receive_fn = rtl8139_receive;
    } else {
        return 0;
    }

    local_ip[0] = 10; local_ip[1] = 0; local_ip[2] = 2; local_ip[3] = 15;
    gateway_ip[0] = 10; gateway_ip[1] = 0; gateway_ip[2] = 2; gateway_ip[3] = 2;
    subnet_mask[0] = 255; subnet_mask[1] = 255; subnet_mask[2] = 255; subnet_mask[3] = 0;
    dns_ip[0] = 10; dns_ip[1] = 0; dns_ip[2] = 2; dns_ip[3] = 3;
    configured = 1;
    return configured;
}

int net_ready(void) { return configured; }

static uint16_t build_dns_query(uint8_t *query, const char *host, uint16_t identifier)
{
    zero_bytes(query, 512); write16(query, identifier); write16(query + 2, 0x0100); write16(query + 4, 1);
    uint16_t p = 12;
    while (*host) {
        uint16_t length = 0;
        while (host[length] && host[length] != '.') length++;
        if (!length || length > 63 || p + length + 2 > 500) return 0;
        query[p++] = (uint8_t)length;
        for (uint16_t i = 0; i < length; i++) {
            char c = host[i];
            if (c >= 'A' && c <= 'Z') c += 'a' - 'A';
            query[p++] = (uint8_t)c;
        }
        host += length;
        if (*host == '.') host++;
    }
    query[p++] = 0; write16(query + p, 1); p += 2; write16(query + p, 1); p += 2;
    return p;
}

static int dns_resolve(const char *host, uint8_t address[4])
{
    uint8_t query[512];
    uint8_t dns_mac[6];
    uint16_t identifier = 0x4A31;
    uint16_t query_length = build_dns_query(query, host, identifier);
    if (!query_length) return 0;
    if (!resolve_next_hop(dns_ip, dns_mac)) return 0;
    make_udp_packet(frame, dns_mac, local_ip, dns_ip, 53000, 53, query, query_length);
    for (uint32_t tries = 0; tries < NET_WAIT_POLLS; tries++) {
        int length = net_receive_fn(frame, sizeof(frame));
        if (length < 14 + 20 + 8 + 12 || read16(frame + 12) != ETH_IPV4 || frame[23] != IP_UDP) continue;
        uint8_t *udp = frame + 34;
        if (read16(udp) != 53 || read16(udp + 2) != 53000) continue;
        uint8_t *answer = udp + 8;
        if (read16(answer) != identifier || (read16(answer + 2) & 0x800F) != 0x8000) continue;
        uint32_t dns_length = read16(frame + 16) - 28;
        if (dns_length < 12) continue;
        uint16_t questions = read16(answer + 4), answers = read16(answer + 6);
        uint32_t p = 12;
        for (uint16_t q = 0; q < questions; q++) {
            while (p < dns_length && answer[p]) { if ((answer[p] & 0xC0) == 0xC0) { p += 2; break; } p += answer[p] + 1; }
            if (p < dns_length && answer[p] == 0) p++;
            if (p + 4 > dns_length) return 0;
            p += 4;
        }
        for (uint16_t a = 0; a < answers && p + 12 <= dns_length; a++) {
            if ((answer[p] & 0xC0) == 0xC0) p += 2;
            else { while (p < dns_length && answer[p]) p += answer[p] + 1; p++; }
            if (p + 10 > dns_length) break;
            uint16_t type = read16(answer + p); uint16_t klass = read16(answer + p + 2);
            uint16_t data_length = read16(answer + p + 8); p += 10;
            if (p + data_length > dns_length) break;
            if (type == 1 && klass == 1 && data_length == 4) { copy_bytes(address, answer + p, 4); return 1; }
            p += data_length;
        }
    }
    return 0;
}

static void make_tcp_packet(uint8_t *packet, const uint8_t destination_mac[6], const uint8_t destination_ip[4],
                            uint16_t source_port, uint16_t destination_port, uint32_t sequence,
                            uint32_t acknowledgement, uint8_t flags, const uint8_t *payload, uint16_t payload_length)
{
    uint16_t tcp_length = 20 + payload_length;
    ethernet_header(packet, destination_mac, ETH_IPV4);
    ip_header(packet + 14, IP_TCP, 20 + tcp_length, local_ip, destination_ip);
    uint8_t *tcp = packet + 34;
    zero_bytes(tcp, 20); write16(tcp, source_port); write16(tcp + 2, destination_port);
    write32(tcp + 4, sequence); write32(tcp + 8, acknowledgement); tcp[12] = 0x50; tcp[13] = flags;
    write16(tcp + 14, 4096); if (payload_length) copy_bytes(tcp + 20, payload, payload_length);
    write16(tcp + 16, transport_checksum(packet + 14, tcp, tcp_length, IP_TCP));
    net_send_fn(packet, 14 + 20 + tcp_length);
}

static int tcp_wait(uint8_t *packet, uint8_t **tcp_out, uint16_t *payload_length,
                    const uint8_t remote_ip[4], uint16_t remote_port, uint16_t local_port,
                    uint32_t attempts)
{
    for (uint32_t tries = 0; tries < attempts; tries++) {
        int length = net_receive_fn(packet, FRAME_MAX);
        if (length < 54 || read16(packet + 12) != ETH_IPV4 || packet[23] != IP_TCP || !equal_bytes(packet + 26, remote_ip, 4)) continue;
        uint8_t *tcp = packet + 34;
        if (read16(tcp) != remote_port || read16(tcp + 2) != local_port) continue;
        uint16_t ip_length = read16(packet + 16);
        uint16_t header_length = (tcp[12] >> 4) * 4;
        if (ip_length < 20 + header_length) continue;
        *payload_length = ip_length - 20 - header_length;
        *tcp_out = tcp;
        return length;
    }
    return 0;
}

int net_tcp_connect(struct net_tcp_connection *connection, const char *host, uint16_t port)
{
    uint8_t *tcp;
    uint16_t payload_length;
    if (!connection || !configured || !host || !port) return -1;
    if (!dns_resolve(host, connection->remote_ip)) return -2;
    if (!resolve_next_hop(connection->remote_ip, connection->remote_mac)) return -3;
    connection->local_port = ephemeral_port++;
    connection->remote_port = port;
    connection->sequence = 0x13572468 ^ ((uint32_t)connection->local_port << 16);
    connection->acknowledgement = 0;
    connection->connected = 0;
    make_tcp_packet(frame, connection->remote_mac, connection->remote_ip, connection->local_port, port,
                    connection->sequence, 0, 0x02, 0, 0);
    if (!tcp_wait(frame, &tcp, &payload_length, connection->remote_ip, port, connection->local_port,
                  NET_WAIT_POLLS) || !(tcp[13] & 0x12)) return -4;
    connection->acknowledgement = read32(tcp + 4) + 1;
    connection->sequence++;
    make_tcp_packet(frame, connection->remote_mac, connection->remote_ip, connection->local_port, port,
                    connection->sequence, connection->acknowledgement, 0x10, 0, 0);
    for (uint32_t tries = 0; tries < 1000000000; tries++) {
        if ((tries & 0x3FF) == 0) { __asm__ volatile("pause" ::: "memory"); }
        if (net_send_fn(frame, 54)) break;
    }
    connection->connected = 1;
    return 1;
}

int net_tcp_send(struct net_tcp_connection *connection, const uint8_t *data, uint16_t length)
{
    uint16_t packet_length;
    if (!connection || !connection->connected || (!data && length) || length > FRAME_MAX - 54) return -1;
    make_tcp_packet(frame, connection->remote_mac, connection->remote_ip, connection->local_port,
                    connection->remote_port, connection->sequence, connection->acknowledgement, 0x18, data, length);
    packet_length = (uint16_t)(14 + 20 + 20 + length);
    for (uint32_t tries = 0; tries < 1000000000; tries++) {
        if ((tries & 0x3FF) == 0) { __asm__ volatile("pause" ::: "memory"); }
        if (net_send_fn(frame, packet_length)) break;
    }
    connection->sequence += length;
    return 1;
}

int net_tcp_receive(struct net_tcp_connection *connection, uint8_t *data, uint16_t capacity, uint32_t attempts)
{
    uint8_t *tcp;
    uint16_t payload_length;
    if (!connection || !connection->connected || !data || !capacity) return -1;
    if (attempts == 0) attempts = 1000000000;
    for (uint32_t tries = 0; tries < attempts; tries++) {
        if ((tries & 0x3FF) == 0) { __asm__ volatile("pause" ::: "memory"); }
        int length = tcp_wait(frame, &tcp, &payload_length, connection->remote_ip, connection->remote_port,
                              connection->local_port, 1);
        if (!length) continue;
        uint8_t flags = tcp[13];
        uint32_t sequence = read32(tcp + 4);
        uint8_t header_length = (tcp[12] >> 4) * 4;
        if (flags & 0x04) { connection->connected = 0; return -2; }
        if (payload_length && sequence == connection->acknowledgement) {
            uint16_t copied = payload_length < capacity ? payload_length : capacity;
            copy_bytes(data, tcp + header_length, copied);
            connection->acknowledgement += payload_length;
            make_tcp_packet(frame, connection->remote_mac, connection->remote_ip, connection->local_port,
                            connection->remote_port, connection->sequence, connection->acknowledgement, 0x10, 0, 0);
            if (flags & 0x01) connection->connected = 0;
            return copied;
        }
        if (flags & 0x01) {
            connection->acknowledgement++;
            make_tcp_packet(frame, connection->remote_mac, connection->remote_ip, connection->local_port,
                            connection->remote_port, connection->sequence, connection->acknowledgement, 0x10, 0, 0);
            connection->connected = 0;
            return 0;
        }
    }
    return -3;
}

void net_tcp_close(struct net_tcp_connection *connection)
{
    if (!connection || !connection->connected) return;
    make_tcp_packet(frame, connection->remote_mac, connection->remote_ip, connection->local_port,
                    connection->remote_port, connection->sequence, connection->acknowledgement, 0x11, 0, 0);
    connection->sequence++;
    connection->connected = 0;
}

int net_http_get(const char *host, const char *path, char *response, uint32_t capacity)
{
    uint8_t remote_ip[4];
    uint8_t remote_mac[6];
    uint16_t local_port = ephemeral_port++;
    uint8_t *tcp;
    uint16_t payload_length;
    if (!configured || !host || !path || !response || capacity < 2) return -1;
    if (!dns_resolve(host, remote_ip)) return -2;
    if (!resolve_next_hop(remote_ip, remote_mac)) return -3;

    tcp_sequence = 0x13572468;
    make_tcp_packet(frame, remote_mac, remote_ip, local_port, 80, tcp_sequence, 0, 0x02, 0, 0);
    if (!tcp_wait(frame, &tcp, &payload_length, remote_ip, 80, local_port, NET_WAIT_POLLS) || !(tcp[13] & 0x12)) return -4;
    uint32_t remote_sequence = read32(tcp + 4) + 1;
    tcp_sequence++;
    make_tcp_packet(frame, remote_mac, remote_ip, local_port, 80, tcp_sequence, remote_sequence, 0x10, 0, 0);

    char request[384];
    uint32_t request_length = 0;
    const char *prefix = "GET "; while (*prefix) request[request_length++] = *prefix++;
    while (*path && request_length < sizeof(request) - 180) request[request_length++] = *path++;
    const char *middle = " HTTP/1.0\r\nHost: "; while (*middle) request[request_length++] = *middle++;
    while (*host && request_length < sizeof(request) - 100) request[request_length++] = *host++;
    const char *suffix = "\r\nUser-Agent: TarixenOS/10\r\nConnection: close\r\n\r\n";
    while (*suffix && request_length < sizeof(request)) request[request_length++] = *suffix++;
    if (request_length >= sizeof(request)) return -5;

    make_tcp_packet(frame, remote_mac, remote_ip, local_port, 80, tcp_sequence, remote_sequence,
                    0x18, (const uint8_t *)request, (uint16_t)request_length);
    tcp_sequence += request_length;
    uint32_t used = 0;
    uint8_t finished = 0;
    uint32_t idle_tries = 0;
    for (uint32_t tries = 0; tries < NET_WAIT_POLLS && !finished && used + 1 < capacity; tries++) {
        int length = tcp_wait(frame, &tcp, &payload_length, remote_ip, 80, local_port, 1);
        if (!length) {
            if (++idle_tries > 5000000) break;
            continue;
        }
        idle_tries = 0;
        uint8_t flags = tcp[13];
        uint32_t sequence = read32(tcp + 4);
        uint32_t acknowledgement = read32(tcp + 8);
        uint8_t header_length = (tcp[12] >> 4) * 4;
        if (payload_length && sequence == remote_sequence) {
            uint8_t *payload = tcp + header_length;
            uint32_t copy_length = payload_length;
            if (copy_length > capacity - used - 1) copy_length = capacity - used - 1;
            for (uint32_t i = 0; i < copy_length; i++) response[used++] = (char)payload[i];
            remote_sequence += payload_length;
        }
        if (flags & 0x01) { remote_sequence++; finished = 1; }
        if (payload_length || (flags & 0x01))
            make_tcp_packet(frame, remote_mac, remote_ip, local_port, 80, tcp_sequence,
                            remote_sequence, 0x10, 0, 0);
        if ((flags & 0x04) || acknowledgement > tcp_sequence + 4096) break;
    }
    response[used] = 0;
    return (int)used;
}
