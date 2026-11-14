/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/bluetooth/bluetooth.h>

extern "C" {
#include <drivers/usb/core/usb.h>
}

/*funkcje we/wy portow*/

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

/*kontekst bluetooth*/

static struct bt_context g_bt_ctx = {};

/*urządzenie usb kontrolera bluetooth*/

static usb_device_t *g_bt_usb_dev = 0;
static uint8_t g_bt_ep_in = 0;
static uint8_t g_bt_ep_out = 0;
static uint8_t g_bt_ep_int = 0;

/*bufor zdarzen hci*/

static uint8_t g_hci_evt_buf[255];
static uint32_t g_hci_evt_len = 0;
static uint8_t g_hci_evt_ready = 0;

/*opoznienie mikrosekundowe*/

static void bt_delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (int j = 0; j < 40; j++);
    }
}

/*opoznienie milisekundowe*/

static void bt_delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) {
        bt_delay_us(1000);
    }
}

/*kopiowanie bajtow*/

static void bt_copy(uint8_t *dst, const uint8_t *src, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) dst[i] = src[i];
}

/*zerowanie bajtow*/

static void bt_zero(uint8_t *dst, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) dst[i] = 0;
}

/*porownanie adresow bluetooth*/

static int bt_addr_equal(const struct bt_addr *a, const struct bt_addr *b)
{
    for (uint32_t i = 0; i < BT_ADDR_LEN; i++) {
        if (a->b[i] != b->b[i]) return 0;
    }
    return 1;
}

/*kopiowanie adresu bluetooth*/

static void bt_addr_copy(struct bt_addr *dst, const struct bt_addr *src)
{
    for (uint32_t i = 0; i < BT_ADDR_LEN; i++) dst->b[i] = src->b[i];
}

/*zapisanie adresu bluetooth z tablicy bajtow*/

static void bt_addr_set(struct bt_addr *dst, const uint8_t *bytes)
{
    for (uint32_t i = 0; i < BT_ADDR_LEN; i++) dst->b[i] = bytes[i];
}

/*warstwa usb - transport hci:usb*/

/*znajdź urządzenie bluetooth na usb*/

static int bt_usb_find_device(void)
{
    int count = usb_device_count();
    for (int i = 0; i < count; i++) {
        usb_device_t *dev = usb_device_get(i);
        if (!dev || !dev->configured) continue;

        if (dev->id_vendor == BLUETOOTH_VENDOR_INTEL &&
            dev->id_product == BLUETOOTH_DEVICE_INTEL_BT) {
            g_bt_usb_dev = dev;
            return 1;
        }
        if (dev->id_vendor == BLUETOOTH_VENDOR_REALTEK &&
            dev->id_product == BLUETOOTH_DEVICE_REALTEK_BT) {
            g_bt_usb_dev = dev;
            return 1;
        }
    }
    return 0;
}

/*skonfiguruj endpointy interfejsu bluetooth*/

static int bt_usb_configure_endpoints(void)
{
    if (!g_bt_usb_dev) return 0;

    for (int ep = 0; ep < USB_MAX_ENDPOINTS; ep++) {
        if (!g_bt_usb_dev->ep[ep].used) continue;
        uint8_t ep_addr = g_bt_usb_dev->ep[ep].address;
        uint8_t ep_type = g_bt_usb_dev->ep[ep].type;
        uint8_t ep_dir = (ep_addr & 0x80) ? USB_DIR_IN : USB_DIR_OUT;

        if (ep_type == USB_ENDPOINT_INTERRUPT && ep_dir == USB_DIR_IN) {
            g_bt_ep_int = ep_addr;
        } else if (ep_type == USB_ENDPOINT_BULK && ep_dir == USB_DIR_OUT) {
            g_bt_ep_out = ep_addr;
        } else if (ep_type == USB_ENDPOINT_BULK && ep_dir == USB_DIR_IN) {
            g_bt_ep_in = ep_addr;
        }
    }

    return (g_bt_ep_out != 0 && g_bt_ep_in != 0 && g_bt_ep_int != 0);
}

/*wyslij polecenie hci przez endpoint bulk out*/

static int bt_hci_send_cmd(uint16_t opcode, const uint8_t *params, uint8_t param_len)
{
    if (!g_bt_usb_dev || !g_bt_ep_out) return -1;

    uint8_t pkt[260];
    uint32_t p = 0;
    pkt[p++] = HCI_COMMAND_PACKET;
    pkt[p++] = (uint8_t)(opcode & 0xFF);
    pkt[p++] = (uint8_t)((opcode >> 8) & 0xFF);
    pkt[p++] = param_len;
    for (uint8_t i = 0; i < param_len; i++) pkt[p++] = params[i];

    return usb_bulk_transfer(g_bt_usb_dev, g_bt_ep_out, pkt, p, USB_DIR_OUT);
}

/*wyslij dane acl przez endpoint bulk out*/

int bt_hci_send_acl(uint16_t handle, uint16_t data_len, const uint8_t *data)
{
    if (!g_bt_usb_dev || !g_bt_ep_out) return -1;

    uint8_t pkt[260 + 4];
    uint32_t p = 0;
    pkt[p++] = HCI_ACL_DATA_PACKET;
    uint16_t hb = (handle & 0x0FFF) | (0x02 << 12);
    pkt[p++] = (uint8_t)(hb & 0xFF);
    pkt[p++] = (uint8_t)((hb >> 8) & 0xFF);
    pkt[p++] = (uint8_t)(data_len & 0xFF);
    pkt[p++] = (uint8_t)((data_len >> 8) & 0xFF);
    for (uint16_t i = 0; i < data_len; i++) pkt[p++] = data[i];

    return usb_bulk_transfer(g_bt_usb_dev, g_bt_ep_out, pkt, p, USB_DIR_OUT);
}

/*odbierz zdarzenie hci z endpointu interrupt in*/

static int bt_hci_recv_event(void)
{
    if (!g_bt_usb_dev || !g_bt_ep_int) return 0;

    int len = usb_interrupt_transfer(g_bt_usb_dev, g_bt_ep_int, g_hci_evt_buf, sizeof(g_hci_evt_buf));
    if (len > 0) {
        g_hci_evt_len = (uint32_t)len;
        g_hci_evt_ready = 1;
    }
    return len;
}

/*odbierz dane acl z endpointu bulk in*/

static int __attribute__((unused)) bt_hci_recv_acl(uint8_t *buf, uint32_t capacity)
{
    if (!g_bt_usb_dev || !g_bt_ep_in) return 0;
    return usb_bulk_transfer(g_bt_usb_dev, g_bt_ep_in, buf, capacity, USB_DIR_IN);
}

/*warstwa hci - polecenia i zdarzenia*/

static int bt_hci_reset(void)
{
    uint8_t params[0];
    return bt_hci_send_cmd((HCI_OGF_LINK_CTL | (HCI_OCF_RESET << 8)), params, 0);
}

static int bt_hci_set_event_mask(void)
{
    uint8_t params[8];
    bt_zero(params, 8);
    for (int i = 0; i < 8; i++) params[i] = 0xFF;
    return bt_hci_send_cmd((HCI_OGF_CONTROLLER | (HCI_OCF_SET_EVENT_MASK << 8)), params, 8);
}

static int bt_hci_read_local_version(void)
{
    uint8_t params[0];
    return bt_hci_send_cmd((HCI_OGF_INFO | (HCI_OCF_READ_LOCAL_VERSION << 8)), params, 0);
}

static int bt_hci_read_local_name(void)
{
    uint8_t params[0];
    return bt_hci_send_cmd((HCI_OGF_BASEBAND | (HCI_OCF_READ_LOCAL_NAME << 8)), params, 0);
}

static int bt_hci_write_local_name(const char *name)
{
    uint8_t params[248];
    bt_zero(params, 248);
    uint32_t n = 0;
    while (name[n] && n < 248) { params[n] = (uint8_t)name[n]; n++; }
    return bt_hci_send_cmd((HCI_OGF_BASEBAND | (HCI_OCF_WRITE_LOCAL_NAME << 8)), params, 248);
}

static int bt_hci_inquiry(void)
{
    uint8_t params[5];
    params[0] = 0x9a;
    params[1] = 0x8b;
    params[2] = 0x33;
    params[3] = 0x0a;
    params[4] = 0x00;
    return bt_hci_send_cmd((HCI_OGF_LINK_CTL | (HCI_OCF_INQUIRY << 8)), params, 5);
}

static int bt_hci_inquiry_cancel(void)
{
    uint8_t params[0];
    return bt_hci_send_cmd((HCI_OGF_LINK_CTL | (HCI_OCF_INQUIRY_CANCEL << 8)), params, 0);
}

static int bt_hci_create_connection(const struct bt_addr *addr, uint16_t packet_type,
                                     uint8_t pscan_rep_mode, uint16_t clock_offset,
                                     uint8_t role_switch)
{
    uint8_t params[13];
    bt_addr_copy((struct bt_addr *)params, addr);
    params[6] = (uint8_t)(packet_type & 0xFF);
    params[7] = (uint8_t)((packet_type >> 8) & 0xFF);
    params[8] = pscan_rep_mode;
    params[9] = (uint8_t)(clock_offset & 0xFF);
    params[10] = (uint8_t)((clock_offset >> 8) & 0xFF);
    params[11] = role_switch;
    params[12] = 0x00;
    return bt_hci_send_cmd((HCI_OGF_LINK_CTL | (HCI_OCF_CREATE_CONNECTION << 8)), params, 13);
}

static int bt_hci_disconnect(uint16_t handle, uint8_t reason)
{
    uint8_t params[3];
    params[0] = (uint8_t)(handle & 0xFF);
    params[1] = (uint8_t)((handle >> 8) & 0xFF);
    params[2] = reason;
    return bt_hci_send_cmd((HCI_OGF_LINK_CTL | (HCI_OCF_DISCONNECT << 8)), params, 3);
}

static int bt_hci_auth_requested(uint16_t handle)
{
    uint8_t params[2];
    params[0] = (uint8_t)(handle & 0xFF);
    params[1] = (uint8_t)((handle >> 8) & 0xFF);
    return bt_hci_send_cmd((HCI_OGF_LINK_CTL | (HCI_OCF_AUTH_REQUESTED << 8)), params, 2);
}

static int bt_hci_pin_code_reply(const struct bt_addr *addr, uint8_t pin_len, const uint8_t *pin_code)
{
    uint8_t params[23];
    bt_addr_copy((struct bt_addr *)params, addr);
    params[6] = pin_len;
    bt_zero(params + 7, 16);
    for (uint8_t i = 0; i < pin_len && i < 16; i++) params[7 + i] = pin_code[i];
    return bt_hci_send_cmd((HCI_OGF_LINK_CTL | (HCI_OCF_PIN_CODE_REPLY << 8)), params, 23);
}

static int __attribute__((unused)) bt_hci_link_key_reply(const struct bt_addr *addr, const struct bt_link_key *key)
{
    uint8_t params[22];
    bt_addr_copy((struct bt_addr *)params, addr);
    bt_copy(params + 6, key->key, 16);
    return bt_hci_send_cmd((HCI_OGF_LINK_CTL | (HCI_OCF_LINK_KEY_REPLY << 8)), params, 22);
}

static int bt_hci_set_conn_encrypt(uint16_t handle, uint8_t encrypt)
{
    uint8_t params[3];
    params[0] = (uint8_t)(handle & 0xFF);
    params[1] = (uint8_t)((handle >> 8) & 0xFF);
    params[2] = encrypt;
    return bt_hci_send_cmd((HCI_OGF_LINK_CTL | (HCI_OCF_SET_CONN_ENCRYPT << 8)), params, 3);
}

/*przetwarzanie zdarzen hci*/

static void bt_hci_process_event(void)
{
    if (!g_hci_evt_ready) return;
    g_hci_evt_ready = 0;

    uint8_t *evt = g_hci_evt_buf;
    uint32_t len = g_hci_evt_len;
    if (len < 2) return;

    uint8_t evt_type = evt[1];
    uint8_t evt_plen = (len > 2) ? evt[2] : 0;
    uint8_t *eparam = evt + 3;

    switch (evt_type) {
    case HCI_EVT_COMMAND_COMPLETE: {
        if (evt_plen < 3) break;
        break;
    }
    case HCI_EVT_COMMAND_STATUS: {
        if (evt_plen < 4) break;
        break;
    }
    case HCI_EVT_INQUIRY_RESULT: {
        if (evt_plen < 14) break;
        uint8_t num_reports = eparam[0];
        uint8_t *rp = eparam + 1;
        for (uint8_t r = 0; r < num_reports && g_bt_ctx.device_count < BT_MAX_DEVICES; r++) {
            struct bt_device *dev = &g_bt_ctx.devices[g_bt_ctx.device_count];
            bt_addr_set(&dev->addr, rp);
            rp += 6;
            bt_copy(dev->dev_class, rp, 3);
            rp += 3;
            dev->rssi = rp[0];
            rp += 1;
            dev->used = 1;
            dev->bonded = 0;
            dev->connected = 0;
            dev->handle = 0;
            dev->device_type = 0;
            dev->name[0] = 0;
            dev->service_count = 0;
            g_bt_ctx.device_count++;
        }
        break;
    }
    case HCI_EVT_INQUIRY_COMPLETE: {
        g_bt_ctx.discovering = 0;
        break;
    }
    case HCI_EVT_CONNECTION_COMPLETE: {
        if (evt_plen < 11) break;
        uint8_t status = eparam[0];
        uint16_t handle = (uint16_t)(eparam[1] | (eparam[2] << 8));
        if (status == 0) {
            for (uint8_t i = 0; i < g_bt_ctx.device_count; i++) {
                struct bt_device *dev = &g_bt_ctx.devices[i];
                if (dev->handle == 0 && !dev->connected) {
                    dev->connected = 1;
                    dev->handle = handle;
                    break;
                }
            }
        }
        break;
    }
    case HCI_EVT_CONNECTION_REQUEST: {
        if (evt_plen < 10) break;
        break;
    }
    case HCI_EVT_DISCONNECTION_COMPLETE: {
        if (evt_plen < 4) break;
        uint16_t handle = (uint16_t)(eparam[0] | (eparam[1] << 8));
        for (uint8_t i = 0; i < g_bt_ctx.device_count; i++) {
            struct bt_device *dev = &g_bt_ctx.devices[i];
            if (dev->handle == handle) {
                dev->connected = 0;
                dev->handle = 0;
                break;
            }
        }
        break;
    }
    case HCI_EVT_AUTH_COMPLETE: {
        if (evt_plen < 3) break;
        uint16_t handle = (uint16_t)(eparam[0] | (eparam[1] << 8));
        uint8_t status = eparam[2];
        if (status == 0) {
            for (uint8_t i = 0; i < g_bt_ctx.device_count; i++) {
                struct bt_device *dev = &g_bt_ctx.devices[i];
                if (dev->handle == handle) {
                    dev->bonded = 1;
                    break;
                }
            }
        }
        break;
    }
    case HCI_EVT_REMOTE_NAME_REQUEST: {
        break;
    }
    case HCI_EVT_ENCRYPT_CHANGE: {
        break;
    }
    default:
        break;
    }
}

/*inicjalizacja warstwy hci*/

int bt_hci_init(void)
{
    if (!bt_usb_find_device()) return 0;
    if (!bt_usb_configure_endpoints()) return 0;

    if (bt_hci_reset() != 0) return 0;
    bt_delay_ms(200);

    if (bt_hci_set_event_mask() != 0) return 0;

    bt_hci_read_local_version();
    bt_hci_read_local_name();

    g_bt_ctx.hci_ready = 1;
    return 1;
}

/*obsluz zdarzenia hci*/

void bt_hci_poll(void)
{
    if (!g_bt_ctx.hci_ready) return;
    bt_hci_recv_event();
    bt_hci_process_event();
}

/*interfejs c - implementacja funkcji z naglowka*/

int bluetooth_init(void)
{
    if (g_bt_ctx.initialized) return 1;

    if (!bt_hci_init()) return 0;

    bt_hci_write_local_name("tarixenos");

    for (uint32_t i = 0; i < BT_ADDR_LEN; i++) g_bt_ctx.local_addr.b[i] = (uint8_t)(0x00 + i * 0x11);
    g_bt_ctx.local_addr.b[0] = 0xaa;
    g_bt_ctx.local_addr.b[1] = 0xbb;
    g_bt_ctx.local_addr.b[2] = 0xcc;
    g_bt_ctx.local_addr.b[3] = 0xdd;
    g_bt_ctx.local_addr.b[4] = 0xee;
    g_bt_ctx.local_addr.b[5] = 0xff;

    const char *ln = "tarixenos";
    uint32_t ln_n = 0;
    while (ln[ln_n] && ln_n < 63) { g_bt_ctx.local_name[ln_n] = (uint8_t)ln[ln_n]; ln_n++; }
    g_bt_ctx.local_name[ln_n] = 0;

    g_bt_ctx.local_dev_class[0] = 0x10;
    g_bt_ctx.local_dev_class[1] = 0x00;
    g_bt_ctx.local_dev_class[2] = 0x00;

    g_bt_ctx.powered = 1;
    g_bt_ctx.state = BT_STATE_POWERED_ON;
    g_bt_ctx.l2cap_ready = 1;
    g_bt_ctx.a2dp_ready = 1;
    g_bt_ctx.initialized = 1;

    return 1;
}

int bluetooth_is_ready(void)
{
    return g_bt_ctx.initialized;
}

int bluetooth_power_set(uint8_t enable)
{
    if (!g_bt_ctx.initialized) return 0;
    if (enable) {
        g_bt_ctx.powered = 1;
        g_bt_ctx.state = BT_STATE_POWERED_ON;
    } else {
        g_bt_ctx.powered = 0;
        g_bt_ctx.state = BT_STATE_POWERED_OFF;
    }
    return 1;
}

int bluetooth_power_get(void)
{
    return g_bt_ctx.powered;
}

int bluetooth_inquiry_start(void)
{
    if (!g_bt_ctx.initialized || !g_bt_ctx.powered) return 0;
    g_bt_ctx.device_count = 0;
    g_bt_ctx.discovering = 1;
    return bt_hci_inquiry();
}

int bluetooth_inquiry_stop(void)
{
    g_bt_ctx.discovering = 0;
    return bt_hci_inquiry_cancel();
}

int bluetooth_is_discovering(void)
{
    return g_bt_ctx.discovering;
}

uint8_t bluetooth_device_count(void)
{
    return g_bt_ctx.device_count;
}

const struct bt_device *bluetooth_device_get(uint8_t index)
{
    if (index < g_bt_ctx.device_count) return &g_bt_ctx.devices[index];
    return 0;
}

struct bt_device *bluetooth_device_find(const struct bt_addr *addr)
{
    for (uint8_t i = 0; i < g_bt_ctx.device_count; i++) {
        if (bt_addr_equal(&g_bt_ctx.devices[i].addr, addr))
            return &g_bt_ctx.devices[i];
    }
    return 0;
}

int bluetooth_connect(struct bt_device *device)
{
    if (!device || !g_bt_ctx.initialized) return -1;
    if (device->connected) return 1;

    if (bt_hci_create_connection(&device->addr, 0xcc18, 0x02, 0x0000, 0x01) != 0)
        return -1;

    bt_hci_auth_requested(device->handle);
    bt_hci_set_conn_encrypt(device->handle, 1);

    device->connected = 1;
    return 1;
}

int bluetooth_disconnect(struct bt_device *device)
{
    if (!device || !g_bt_ctx.initialized) return -1;
    if (!device->connected) return 1;

    bt_hci_disconnect(device->handle, 0x13);

    device->connected = 0;
    device->handle = 0;
    return 1;
}

int bluetooth_is_connected(struct bt_device *device)
{
    if (!device) return 0;
    return device->connected;
}

int bluetooth_pair(struct bt_device *device)
{
    if (!device || !g_bt_ctx.initialized) return 0;
    if (device->bonded) return 1;

    static const uint8_t pin_code[4] = {'0', '0', '0', '0'};
    bt_hci_pin_code_reply(&device->addr, 4, pin_code);
    bt_delay_ms(500);

    bt_zero(device->link_key.key, BT_LINK_KEY_LEN);
    device->bonded = 1;
    return 1;
}

int bluetooth_unpair(struct bt_device *device)
{
    if (!device) return 0;
    device->bonded = 0;
    bt_zero(device->link_key.key, BT_LINK_KEY_LEN);
    return 1;
}

int bluetooth_is_bonded(struct bt_device *device)
{
    if (!device) return 0;
    return device->bonded;
}

int bluetooth_set_local_name(const char *name)
{
    if (!name || !g_bt_ctx.initialized) return 0;
    uint32_t n = 0;
    while (name[n] && n < 63) { g_bt_ctx.local_name[n] = name[n]; n++; }
    g_bt_ctx.local_name[n] = 0;
    bt_hci_write_local_name(g_bt_ctx.local_name);
    return 1;
}

const char *bluetooth_get_local_name(void)
{
    return g_bt_ctx.local_name;
}

const struct bt_addr *bluetooth_get_local_addr(void)
{
    return &g_bt_ctx.local_addr;
}

int bluetooth_a2dp_open(struct bt_device *device)
{
    if (!device || !device->connected) return 0;
    return 1;
}

int bluetooth_a2dp_close(struct bt_device *device)
{
    if (!device) return 0;
    return 1;
}

int bluetooth_a2dp_send(struct bt_device *device, const uint8_t *data, uint32_t length)
{
    if (!device || !device->connected) return 0;
    (void)data;
    (void)length;
    return 0;
}

int bluetooth_a2dp_streaming(struct bt_device *device)
{
    if (!device) return 0;
    return 0;
}

void bluetooth_context_get(struct bt_context *out)
{
    if (!out) return;
    *out = g_bt_ctx;
}

/*polling warstw bluetooth*/

void bluetooth_poll(void)
{
    if (!g_bt_ctx.initialized) return;
    bt_hci_poll();
}
