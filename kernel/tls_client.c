#include "tls_client.h"
#include "tls_crypto.h"
#include "../drivers/net.h"
#include "syscals/random.h"

#define TLS_BUFFER_SIZE 16384
#define TLS_RECORD_HANDSHAKE 22
#define TLS_RECORD_APPLICATION 23

struct tls13_connection {
    struct net_tcp_connection tcp;
    uint8_t receive[TLS_BUFFER_SIZE];
    uint32_t receive_start;
    uint32_t receive_end;
    uint8_t private_key[32];
    uint8_t public_key[32];
    uint8_t shared_secret[32];
    uint8_t client_hello[512];
    uint16_t client_hello_length;
    uint8_t server_hello[512];
    uint16_t server_hello_length;
    uint8_t client_handshake_secret[32];
    uint8_t server_handshake_secret[32];
    uint8_t client_handshake_key[16];
    uint8_t server_handshake_key[16];
    uint8_t client_handshake_iv[12];
    uint8_t server_handshake_iv[12];
    uint64_t client_sequence;
    uint64_t server_sequence;
};

static void copy_bytes(uint8_t *destination, const uint8_t *source, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++) destination[i] = source[i];
}

static void write16(uint8_t *destination, uint16_t value)
{
    destination[0] = (uint8_t)(value >> 8);
    destination[1] = (uint8_t)value;
}

static uint16_t read16(const uint8_t *source)
{
    return ((uint16_t)source[0] << 8) | source[1];
}

static uint32_t read24(const uint8_t *source)
{
    return ((uint32_t)source[0] << 16) | ((uint32_t)source[1] << 8) | source[2];
}

static uint32_t string_length(const char *text)
{
    uint32_t length = 0;
    while (text[length]) length++;
    return length;
}

static void clear_bytes(uint8_t *data, uint32_t length)
{
    for (uint32_t i = 0; i < length; i++) data[i] = 0;
}

static void store64(uint8_t *destination, uint64_t value)
{
    for (uint32_t i = 0; i < 8; i++) destination[7 - i] = (uint8_t)(value >> (i * 8));
}

static void tls_random(uint8_t *output, uint32_t length)
{
    uint32_t word = 0;
    for (uint32_t i = 0; i < length; i++) {
        if (!(i & 3)) word = random();
        output[i] = (uint8_t)(word >> ((i & 3) * 8));
    }
}

static int tls_send(struct tls13_connection *connection, const uint8_t *data, uint16_t length)
{
    return net_tcp_send(&connection->tcp, data, length) > 0;
}

static int tls_read_exact(struct tls13_connection *connection, uint8_t *output, uint32_t length)
{
    uint32_t copied = 0;
    while (copied < length) {
        if (connection->receive_start < connection->receive_end) {
            uint32_t available = connection->receive_end - connection->receive_start;
            uint32_t take = length - copied < available ? length - copied : available;
            copy_bytes(output + copied, connection->receive + connection->receive_start, take);
            connection->receive_start += take;
            copied += take;
            continue;
        }
        int received = net_tcp_receive(&connection->tcp, connection->receive, sizeof(connection->receive), 5000000);
        if (received <= 0) return 0;
        connection->receive_start = 0;
        connection->receive_end = (uint32_t)received;
    }
    return 1;
}

static int tls_read_record(struct tls13_connection *connection, uint8_t *type, uint8_t *data, uint16_t capacity, uint16_t *length)
{
    uint8_t header[5];
    if (!tls_read_exact(connection, header, sizeof(header))) return 0;
    uint16_t record_length = read16(header + 3);
    if (header[1] != 3 || record_length > capacity || !tls_read_exact(connection, data, record_length)) return 0;
    *type = header[0];
    *length = record_length;
    return 1;
}

static int tls_build_client_hello(struct tls13_connection *connection, const char *host, uint8_t *record, uint16_t *record_length)
{
    uint32_t host_length = string_length(host);
    uint8_t *handshake = connection->client_hello;
    uint32_t p = 0;
    if (!host_length || host_length > 200) return 0;
    tls_random(connection->private_key, sizeof(connection->private_key));
    x25519_public_key(connection->private_key, connection->public_key);
    handshake[p++] = 1;
    p += 3;
    write16(handshake + p, 0x0303); p += 2;
    tls_random(handshake + p, 32); p += 32;
    handshake[p++] = 0;
    write16(handshake + p, 2); p += 2;
    write16(handshake + p, 0x1301); p += 2;
    handshake[p++] = 1; handshake[p++] = 0;
    uint32_t extensions = p; p += 2;
    write16(handshake + p, 0); p += 2; write16(handshake + p, (uint16_t)(host_length + 5)); p += 2;
    write16(handshake + p, (uint16_t)(host_length + 3)); p += 2; handshake[p++] = 0; write16(handshake + p, (uint16_t)host_length); p += 2;
    copy_bytes(handshake + p, (const uint8_t *)host, host_length); p += host_length;
    write16(handshake + p, 10); p += 2; write16(handshake + p, 4); p += 2; write16(handshake + p, 2); p += 2; write16(handshake + p, 29); p += 2;
    write16(handshake + p, 13); p += 2; write16(handshake + p, 10); p += 2; write16(handshake + p, 8); p += 2;
    write16(handshake + p, 0x0804); p += 2; write16(handshake + p, 0x0805); p += 2; write16(handshake + p, 0x0403); p += 2; write16(handshake + p, 0x0401); p += 2;
    write16(handshake + p, 43); p += 2; write16(handshake + p, 3); p += 2; handshake[p++] = 2; write16(handshake + p, 0x0304); p += 2;
    write16(handshake + p, 51); p += 2; write16(handshake + p, 38); p += 2; write16(handshake + p, 36); p += 2;
    write16(handshake + p, 29); p += 2; write16(handshake + p, 32); p += 2; copy_bytes(handshake + p, connection->public_key, 32); p += 32;
    write16(handshake + extensions, (uint16_t)(p - extensions - 2));
    uint32_t body_length = p - 4;
    handshake[1] = (uint8_t)(body_length >> 16); handshake[2] = (uint8_t)(body_length >> 8); handshake[3] = (uint8_t)body_length;
    connection->client_hello_length = (uint16_t)p;
    record[0] = TLS_RECORD_HANDSHAKE; record[1] = 3; record[2] = 1; write16(record + 3, (uint16_t)p);
    copy_bytes(record + 5, handshake, p);
    *record_length = (uint16_t)(p + 5);
    return 1;
}

static int tls_parse_server_hello(struct tls13_connection *connection, const uint8_t *message, uint16_t length)
{
    if (length < 4 || message[0] != 2 || read24(message + 1) + 4 != length) return 0;
    const uint8_t *body = message + 4;
    uint32_t body_length = length - 4;
    if (body_length < 40 || read16(body) != 0x0303 || read16(body + 35) != 0x1301) return 0;
    uint32_t p = 34;
    uint8_t session_length = body[p++];
    if (p + session_length + 5 > body_length) return 0;
    p += session_length;
    if (read16(body + p) != 0x1301) return 0;
    p += 2;
    if (body[p++] != 0) return 0;
    uint16_t extension_length = read16(body + p); p += 2;
    if (p + extension_length != body_length) return 0;
    while (p + 4 <= body_length) {
        uint16_t type = read16(body + p), item_length = read16(body + p + 2); p += 4;
        if (p + item_length > body_length) return 0;
        if (type == 51 && item_length == 36 && read16(body + p) == 29 && read16(body + p + 2) == 32) {
            if (!x25519_shared_secret(connection->private_key, body + p + 4, connection->shared_secret)) return 0;
            return 1;
        }
        p += item_length;
    }
    return 0;
}

static void tls_nonce(const uint8_t iv[12], uint64_t sequence, uint8_t nonce[12])
{
    uint8_t sequence_bytes[12];
    clear_bytes(sequence_bytes, sizeof(sequence_bytes));
    store64(sequence_bytes + 4, sequence);
    for (uint32_t i = 0; i < 12; i++) nonce[i] = iv[i] ^ sequence_bytes[i];
    clear_bytes(sequence_bytes, sizeof(sequence_bytes));
}

static int tls_handshake_keys(struct tls13_connection *connection)
{
    uint8_t empty_hash[32], early_secret[32], derived_secret[32], handshake_secret[32];
    uint8_t transcript_hash[32];
    struct sha256_context hash;
    clear_bytes(empty_hash, sizeof(empty_hash));
    sha256_init(&hash); sha256_final(&hash, empty_hash);
    hkdf_extract_sha256(0, 0, 0, 0, early_secret);
    tls13_expand_label(early_secret, "derived", empty_hash, sizeof(empty_hash), derived_secret, sizeof(derived_secret));
    hkdf_extract_sha256(derived_secret, sizeof(derived_secret), connection->shared_secret, sizeof(connection->shared_secret), handshake_secret);
    sha256_init(&hash);
    sha256_update(&hash, connection->client_hello, connection->client_hello_length);
    sha256_update(&hash, connection->server_hello, connection->server_hello_length);
    sha256_final(&hash, transcript_hash);
    tls13_expand_label(handshake_secret, "c hs traffic", transcript_hash, sizeof(transcript_hash), connection->client_handshake_secret, 32);
    tls13_expand_label(handshake_secret, "s hs traffic", transcript_hash, sizeof(transcript_hash), connection->server_handshake_secret, 32);
    tls13_expand_label(connection->client_handshake_secret, "key", 0, 0, connection->client_handshake_key, 16);
    tls13_expand_label(connection->server_handshake_secret, "key", 0, 0, connection->server_handshake_key, 16);
    tls13_expand_label(connection->client_handshake_secret, "iv", 0, 0, connection->client_handshake_iv, 12);
    tls13_expand_label(connection->server_handshake_secret, "iv", 0, 0, connection->server_handshake_iv, 12);
    clear_bytes(empty_hash, sizeof(empty_hash)); clear_bytes(early_secret, sizeof(early_secret));
    clear_bytes(derived_secret, sizeof(derived_secret)); clear_bytes(handshake_secret, sizeof(handshake_secret));
    clear_bytes(transcript_hash, sizeof(transcript_hash));
    return 1;
}

static int tls_decrypt_handshake_record(struct tls13_connection *connection, uint8_t *record, uint16_t length,
                                        uint8_t *inner_type, uint16_t *inner_length)
{
    uint8_t nonce[12], additional[5];
    if (length < 17) return 0;
    additional[0] = TLS_RECORD_APPLICATION; additional[1] = 3; additional[2] = 3; write16(additional + 3, length);
    tls_nonce(connection->server_handshake_iv, connection->server_sequence++, nonce);
    if (!aes128_gcm_decrypt(connection->server_handshake_key, nonce, additional, sizeof(additional), record, length - 16, record + length - 16)) return 0;
    uint16_t content_length = length - 16;
    while (content_length && record[content_length - 1] == 0) content_length--;
    if (!content_length) return 0;
    *inner_type = record[--content_length];
    *inner_length = content_length;
    clear_bytes(nonce, sizeof(nonce)); clear_bytes(additional, sizeof(additional));
    return 1;
}

int tls13_https_get(const char *host, const char *path, char *response, uint32_t capacity)
{
    struct tls13_connection connection;
    static uint8_t record[TLS_BUFFER_SIZE];
    uint8_t type;
    uint16_t record_length;
    (void)path;
    (void)response;
    (void)capacity;
    for (uint32_t i = 0; i < sizeof(connection); i++) ((uint8_t *)&connection)[i] = 0;
    if (net_tcp_connect(&connection.tcp, host, 443) < 0) return -1;
    if (!tls_build_client_hello(&connection, host, record, &record_length) || !tls_send(&connection, record, record_length)) { net_tcp_close(&connection.tcp); return -2; }
    if (!tls_read_record(&connection, &type, connection.server_hello, sizeof(connection.server_hello), &record_length) || type != TLS_RECORD_HANDSHAKE) { net_tcp_close(&connection.tcp); return -3; }
    connection.server_hello_length = record_length;
    if (!tls_parse_server_hello(&connection, connection.server_hello, record_length)) { net_tcp_close(&connection.tcp); return -4; }
    if (!tls_handshake_keys(&connection)) { net_tcp_close(&connection.tcp); return -5; }
    if (!tls_read_record(&connection, &type, record, sizeof(record), &record_length) || type != TLS_RECORD_APPLICATION) { net_tcp_close(&connection.tcp); return -6; }
    uint8_t inner_type;
    uint16_t inner_length;
    if (!tls_decrypt_handshake_record(&connection, record, record_length, &inner_type, &inner_length) || inner_type != TLS_RECORD_HANDSHAKE) { net_tcp_close(&connection.tcp); return -7; }
    net_tcp_close(&connection.tcp);
    return -8;
}
