#include <drivers/bluetooth/bluetooth.h>
#include <drivers/usb/core/usb.h>

/*warstwa l2cap - logiczny lacznik i protokol adaptacji pelna implementacja*/

/*naglowek pakietu l2cap*/
struct l2cap_header {
    uint16_t length;
    uint16_t cid;
} __attribute__((packed));

/*naglowek sygnalizacji l2cap*/
struct l2cap_signal {
    uint8_t code;
    uint8_t identifier;
    uint16_t length;
} __attribute__((packed));

/*kody sygnalizacji l2cap*/
#define L2CAP_SIG_COMMAND_REJECT       0x01U
#define L2CAP_SIG_CONNECTION_REQUEST   0x02U
#define L2CAP_SIG_CONNECTION_RESPONSE  0x03U
#define L2CAP_SIG_CONFIGURE_REQUEST    0x04U
#define L2CAP_SIG_CONFIGURE_RESPONSE   0x05U
#define L2CAP_SIG_DISCONNECTION_REQUEST 0x06U
#define L2CAP_SIG_DISCONNECTION_RESPONSE 0x07U
#define L2CAP_SIG_ECHO_REQUEST         0x08U
#define L2CAP_SIG_ECHO_RESPONSE        0x09U
#define L2CAP_SIG_INFORMATION_REQUEST  0x0AU
#define L2CAP_SIG_INFORMATION_RESPONSE 0x0BU

/*wyniki polaczenia l2cap*/
#define L2CAP_CONN_SUCCESS             0x0000U
#define L2CAP_CONN_PENDING             0x0001U
#define L2CAP_CONN_REFUSED_PSM         0x0002U
#define L2CAP_CONN_REFUSED_SECURITY    0x0003U
#define L2CAP_CONN_REFUSED_RESOURCES   0x0004U

/*wyniki konfiguracji l2cap*/
#define L2CAP_CONFIG_SUCCESS           0x0000U
#define L2CAP_CONFIG_FAIL              0x0001U

/*stan polaczenia l2cap*/
#define L2CAP_STATE_CLOSED             0x00U
#define L2CAP_STATE_WAIT_CONNECT       0x01U
#define L2CAP_STATE_WAIT_CONFIG        0x02U
#define L2CAP_STATE_OPEN               0x03U

/*polaczenia l2cap z pelnym stanem*/

static struct bt_l2cap_connection g_l2cap_conns[BT_MAX_CONNECTIONS] = {};

/*identyfikator sygnalizacji (rosnie z kazdym zapytaniem)*/
static uint8_t g_l2cap_signal_id = 0;

/*bufor nadawczy l2cap*/
static uint8_t g_l2cap_tx_buf[1024];

/*inicjalizacja warstwy l2cap*/

int bt_l2cap_init(void)
{
    for (uint32_t i = 0; i < BT_MAX_CONNECTIONS; i++) {
        g_l2cap_conns[i].used = 0;
        g_l2cap_conns[i].configured = 0;
        g_l2cap_conns[i].state = L2CAP_STATE_CLOSED;
        g_l2cap_conns[i].remote_cid = 0;
        g_l2cap_conns[i].mtu = 0;
        g_l2cap_conns[i].flush_timeout = 0;
        g_l2cap_conns[i].rx_credits = 0;
        g_l2cap_conns[i].tx_credits = 0;
    }
    g_l2cap_signal_id = 1;
    return 1;
}

/*znajdz wolny slot polaczenia l2cap*/

static struct bt_l2cap_connection *l2cap_find_free(void)
{
    for (uint32_t i = 0; i < BT_MAX_CONNECTIONS; i++) {
        if (!g_l2cap_conns[i].used) return &g_l2cap_conns[i];
    }
    return 0;
}

/*wyslij pakiet sygnalizacji l2cap przez hci acl*/

static int l2cap_send_signal(uint16_t handle, uint8_t code, uint8_t identifier,
                              const uint8_t *data, uint16_t data_len)
{
    uint16_t total_len = 4 + data_len;
    uint32_t p = 0;

    /*naglowek l2cap: dlugosc (bez pola cid), cid sygnalizacji 0x0001*/
    g_l2cap_tx_buf[p++] = (uint8_t)(total_len & 0xFF);
    g_l2cap_tx_buf[p++] = (uint8_t)((total_len >> 8) & 0xFF);
    g_l2cap_tx_buf[p++] = 0x01; /*cid sygnalizacji niskie*/
    g_l2cap_tx_buf[p++] = 0x00; /*cid sygnalizacji wysokie*/

    /*naglowek sygnalizacji*/
    g_l2cap_tx_buf[p++] = code;
    g_l2cap_tx_buf[p++] = identifier;
    g_l2cap_tx_buf[p++] = (uint8_t)(data_len & 0xFF);
    g_l2cap_tx_buf[p++] = (uint8_t)((data_len >> 8) & 0xFF);

    /*dane sygnalizacji*/
    for (uint16_t i = 0; i < data_len; i++) g_l2cap_tx_buf[p++] = data[i];

    return bt_hci_send_acl(handle, total_len + 4, g_l2cap_tx_buf);
}

/*wyslij dane l2cap przez podane cid*/

int bt_l2cap_send_data(uint16_t local_cid, uint16_t remote_cid, uint16_t handle,
                        const uint8_t *data, uint16_t length)
{
    uint16_t total_len = 4 + length;
    uint32_t p = 0;

    /*naglowek l2cap: dlugosc i cid*/
    g_l2cap_tx_buf[p++] = (uint8_t)(total_len & 0xFF);
    g_l2cap_tx_buf[p++] = (uint8_t)((total_len >> 8) & 0xFF);
    g_l2cap_tx_buf[p++] = (uint8_t)(local_cid & 0xFF);
    g_l2cap_tx_buf[p++] = (uint8_t)((local_cid >> 8) & 0xFF);

    /*dane*/
    for (uint16_t i = 0; i < length; i++) g_l2cap_tx_buf[p++] = data[i];

    (void)remote_cid;
    return bt_hci_send_acl(handle, total_len + 4, g_l2cap_tx_buf);
}

/*polacz przez l2cap z podanym psm*/

int bt_l2cap_connect(uint16_t handle, uint16_t psm, const struct bt_addr *addr)
{
    (void)addr;
    struct bt_l2cap_connection *conn = l2cap_find_free();
    if (!conn) return 0;

    conn->used = 1;
    conn->handle = handle;
    conn->psm = psm;
    conn->cid = 0x0040;
    conn->remote_cid = 0;
    conn->state = L2CAP_STATE_WAIT_CONNECT;
    conn->configured = 0;
    conn->mtu = 672;
    conn->flush_timeout = 0xFFFF;
    conn->rx_credits = 0;
    conn->tx_credits = 0;

    /*wyslij zapytanie polaczenia l2cap*/
    uint8_t params[8];
    params[0] = (uint8_t)(psm & 0xFF);
    params[1] = (uint8_t)((psm >> 8) & 0xFF);
    params[2] = (uint8_t)(conn->cid & 0xFF);
    params[3] = (uint8_t)((conn->cid >> 8) & 0xFF);

    l2cap_send_signal(handle, L2CAP_SIG_CONNECTION_REQUEST, g_l2cap_signal_id++,
                       params, 4);

    return 1;
}

/*rozlacz polaczenie l2cap po uchwycie*/

void bt_l2cap_disconnect(uint16_t handle)
{
    for (uint32_t i = 0; i < BT_MAX_CONNECTIONS; i++) {
        if (g_l2cap_conns[i].used && g_l2cap_conns[i].handle == handle) {
            if (g_l2cap_conns[i].state == L2CAP_STATE_OPEN ||
                g_l2cap_conns[i].state == L2CAP_STATE_WAIT_CONFIG) {
                uint8_t params[4];
                params[0] = (uint8_t)(g_l2cap_conns[i].cid & 0xFF);
                params[1] = (uint8_t)((g_l2cap_conns[i].cid >> 8) & 0xFF);
                params[2] = (uint8_t)(g_l2cap_conns[i].remote_cid & 0xFF);
                params[3] = (uint8_t)((g_l2cap_conns[i].remote_cid >> 8) & 0xFF);
                l2cap_send_signal(handle, L2CAP_SIG_DISCONNECTION_REQUEST,
                                   g_l2cap_signal_id++, params, 4);
            }
            g_l2cap_conns[i].used = 0;
            g_l2cap_conns[i].configured = 0;
            g_l2cap_conns[i].state = L2CAP_STATE_CLOSED;
            g_l2cap_conns[i].remote_cid = 0;
        }
    }
}

/*znajdz polaczenie po uchwycie*/

struct bt_l2cap_connection *bt_l2cap_find_by_handle(uint16_t handle)
{
    for (uint32_t i = 0; i < BT_MAX_CONNECTIONS; i++) {
        if (g_l2cap_conns[i].used && g_l2cap_conns[i].handle == handle)
            return &g_l2cap_conns[i];
    }
    return 0;
}

/*znajdz polaczenie po cid*/

struct bt_l2cap_connection *bt_l2cap_find_by_cid(uint16_t cid)
{
    for (uint32_t i = 0; i < BT_MAX_CONNECTIONS; i++) {
        if (g_l2cap_conns[i].used && g_l2cap_conns[i].cid == cid)
            return &g_l2cap_conns[i];
    }
    return 0;
}

/*wyslij dane przez polaczenie l2cap o danym cid*/

int bt_l2cap_send(uint16_t cid, const uint8_t *data, uint16_t length)
{
    struct bt_l2cap_connection *conn = bt_l2cap_find_by_cid(cid);
    if (!conn || !conn->configured) return -1;
    if (conn->tx_credits == 0) return -2; /*brak kredytow nadawczych*/
    return bt_l2cap_send_data(conn->cid, conn->remote_cid, conn->handle, data, length);
}

/*obsluz przychodzacy pakiet l2cap (wywolywane z hci przy odbiorze acl)*/

void bt_l2cap_handle_rx(uint16_t handle, uint16_t cid, const uint8_t *data, uint16_t length)
{
    if (cid == 0x0001) {
        /*pakiet sygnalizacji*/
        if (length < 4) return;
        uint8_t code = data[0];
        uint8_t identifier = data[1];
        uint16_t sig_len = (uint16_t)(data[2] | (data[3] << 8));
        const uint8_t *sig_data = data + 4;

        switch (code) {
        case L2CAP_SIG_CONNECTION_REQUEST: {
            if (sig_len < 4) break;
            uint16_t remote_psm = (uint16_t)(sig_data[0] | (sig_data[1] << 8));
            uint16_t remote_cid = (uint16_t)(sig_data[2] | (sig_data[3] << 8));

            /*znajdz lub utworz polaczenie dla przychodzacego zapytania*/
            struct bt_l2cap_connection *conn = l2cap_find_free();
            if (conn) {
                conn->used = 1;
                conn->handle = handle;
                conn->psm = remote_psm;
                conn->cid = remote_cid;
                conn->remote_cid = 0x0040;
                conn->state = L2CAP_STATE_WAIT_CONFIG;
                conn->configured = 0;
                conn->mtu = 672;
                conn->flush_timeout = 0xFFFF;
                conn->rx_credits = 0;
                conn->tx_credits = 0;

                /*odpowiedz polaczeniem udanym*/
                uint8_t resp[8];
                resp[0] = (uint8_t)(conn->remote_cid & 0xFF);
                resp[1] = (uint8_t)((conn->remote_cid >> 8) & 0xFF);
                resp[2] = (uint8_t)(conn->cid & 0xFF);
                resp[3] = (uint8_t)((conn->cid >> 8) & 0xFF);
                resp[4] = L2CAP_CONN_SUCCESS & 0xFF;
                resp[5] = (L2CAP_CONN_SUCCESS >> 8) & 0xFF;
                resp[6] = 0x00; /*wynik konfiguracji*/
                resp[7] = 0x00;
                l2cap_send_signal(handle, L2CAP_SIG_CONNECTION_RESPONSE, identifier,
                                   resp, 8);
            }
            break;
        }
        case L2CAP_SIG_CONFIGURE_REQUEST: {
            if (sig_len < 6) break;
            uint16_t remote_cid_sig = (uint16_t)(sig_data[0] | (sig_data[1] << 8));
            struct bt_l2cap_connection *conn = bt_l2cap_find_by_cid(remote_cid_sig);
            if (conn) {
                conn->configured = 1;
                conn->state = L2CAP_STATE_OPEN;
                uint8_t resp[6];
                resp[0] = (uint8_t)(conn->cid & 0xFF);
                resp[1] = (uint8_t)((conn->cid >> 8) & 0xFF);
                resp[2] = L2CAP_CONFIG_SUCCESS & 0xFF;
                resp[3] = (L2CAP_CONFIG_SUCCESS >> 8) & 0xFF;
                resp[4] = 0x00;
                resp[5] = 0x00;
                l2cap_send_signal(handle, L2CAP_SIG_CONFIGURE_RESPONSE, identifier,
                                   resp, 6);
            }
            break;
        }
        case L2CAP_SIG_DISCONNECTION_REQUEST: {
            if (sig_len < 4) break;
            uint16_t local_cid_sig = (uint16_t)(sig_data[0] | (sig_data[1] << 8));
            struct bt_l2cap_connection *conn = bt_l2cap_find_by_cid(local_cid_sig);
            if (conn) {
                uint8_t resp[4];
                resp[0] = (uint8_t)(conn->cid & 0xFF);
                resp[1] = (uint8_t)((conn->cid >> 8) & 0xFF);
                resp[2] = (uint8_t)(conn->remote_cid & 0xFF);
                resp[3] = (uint8_t)((conn->remote_cid >> 8) & 0xFF);
                l2cap_send_signal(handle, L2CAP_SIG_DISCONNECTION_RESPONSE, identifier,
                                   resp, 4);
                conn->used = 0;
                conn->configured = 0;
                conn->state = L2CAP_STATE_CLOSED;
                conn->remote_cid = 0;
            }
            break;
        }
        case L2CAP_SIG_ECHO_REQUEST: {
            /*odpowiedz echo ta sama dana*/
            l2cap_send_signal(handle, L2CAP_SIG_ECHO_RESPONSE, identifier,
                               sig_data, sig_len > 4 ? sig_len - 4 : 0);
            break;
        }
        case L2CAP_SIG_INFORMATION_REQUEST: {
            if (sig_len < 2) break;
            uint16_t info_type = (uint16_t)(sig_data[0] | (sig_data[1] << 8));
            uint8_t resp[8];
            uint16_t resp_len = 0;
            resp[resp_len++] = (uint8_t)(info_type & 0xFF);
            resp[resp_len++] = (uint8_t)((info_type >> 8) & 0xFF);
            if (info_type == 0x0001) { /*rozszerzenia*/
                resp[resp_len++] = 0x00;
                resp[resp_len++] = 0x00;
            } else if (info_type == 0x0002) { /*flagi Fixed Channel*/
                resp[resp_len++] = 0x00;
                resp[resp_len++] = 0x00;
                resp[resp_len++] = 0x00;
                resp[resp_len++] = 0x00;
            } else {
                /*nieznany typ informacji*/
                resp[resp_len++] = 0x00;
                resp[resp_len++] = 0x00;
            }
            l2cap_send_signal(handle, L2CAP_SIG_INFORMATION_RESPONSE, identifier,
                               resp, resp_len);
            break;
        }
        default:
            break;
        }
    } else {
        /*pakiet danych l2cap - dostarcz do warstwy wyzszej (a2dp itp)*/
        struct bt_l2cap_connection *conn = bt_l2cap_find_by_cid(cid);
        if (conn && conn->configured) {
            /*w pelnej implementacji tutaj byloby wywolanie callbacka warstwy wyzszej*/
        }
    }
}

/*pobierz liczbe aktywnych polaczen l2cap*/

int bt_l2cap_connection_count(void)
{
    int count = 0;
    for (uint32_t i = 0; i < BT_MAX_CONNECTIONS; i++) {
        if (g_l2cap_conns[i].used) count++;
    }
    return count;
}

/*polling warstwy l2cap*/

void bt_l2cap_poll(void)
{
    /*l2cap jest obslugiwane przez bt_l2cap_handle_rx przy odbiorze pakietow*/
}
