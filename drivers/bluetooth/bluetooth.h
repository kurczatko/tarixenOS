#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include "../../include/printf/types.h"

#ifdef __cplusplus
extern "C" {
#endif

/*Zaawansowany sterownik Bluetooth dla placas glównej MSI MPG Z890 CARBON WIFI
 *napisany w C++. Implementuje pelny stos Bluetooth zgodny ze specyfikacja Bluetooth Core:
 *
 *  - HCI (Host Controller Interface) po USB (transport HCI:USB)
 *  - L2CAP (Logical Link Control and Adaptation Protocol)
 *  - A2DP (Advanced Audio Distribution Profile) dla sluchawek Bluetooth
 *  - AVCTP (Audio/Video Control Transport Protocol) dla sterowania odtwarzaniem
 *  - SDP (Service Discovery Protocol) dla wykrywania uslug urzadzen
 *  - Zarzadzanie parowaniem i bondingiem urzadzen
 *
 * Integracja z systemem graficznym: ikona na pasku zadan oraz okno wyboru
 * urzadzenia do polaczenia po kliknieciu ikony.
 */

/*================== STALE BLUETOOTH ==================*/

/*Identyfikator placas MSI MPG Z890 CARBON WIFI (Intel Bluetooth controller)*/
#define BLUETOOTH_VENDOR_INTEL       0x8086U
#define BLUETOOTH_DEVICE_INTEL_BT    0x00A5U   /*Intel Bluetooth USB device*/
#define BLUETOOTH_VENDOR_REALTEK     0x0BDAU
#define BLUETOOTH_DEVICE_REALTEK_BT  0x8771U   /*Realtek Bluetooth USB device*/

/*Typy pakietow HCI*/
#define HCI_COMMAND_PACKET   0x01U
#define HCI_ACL_DATA_PACKET  0x02U
#define HCI_SCO_DATA_PACKET  0x03U
#define HCI_EVENT_PACKET     0x04U

/*Opcode HCI Command (OGF | OCF << 8)*/
#define HCI_OGF_LINK_CTL     0x01U
#define HCI_OGF_BASEBAND     0x03U
#define HCI_OGF_INFO         0x04U
#define HCI_OGF_STATUS       0x05U
#define HCI_OGF_CONTROLLER   0x06U

#define HCI_OCF_RESET        0x0003U
#define HCI_OCF_READ_LOCAL_VERSION   0x0001U
#define HCI_OCF_READ_LOCAL_NAME      0x0014U
#define HCI_OCF_WRITE_LOCAL_NAME     0x0013U
#define HCI_OCF_SET_EVENT_MASK       0x0001U
#define HCI_OCF_INQUIRY              0x0001U
#define HCI_OCF_INQUIRY_CANCEL       0x0002U
#define HCI_OCF_CREATE_CONNECTION    0x0005U
#define HCI_OCF_ACCEPT_CONNECTION    0x0009U
#define HCI_OCF_DISCONNECT           0x0006U
#define HCI_OCF_AUTH_REQUESTED       0x0011U
#define HCI_OCF_SET_CONN_ENCRYPT     0x0013U
#define HCI_OCF_LINK_KEY_REPLY       0x000BU
#define HCI_OCF_PIN_CODE_REPLY       0x000DU
#define HCI_OCF_READ_REMOTE_NAME     0x0019U

/*Zdarzenia HCI*/
#define HCI_EVT_INQUIRY_RESULT         0x2FU
#define HCI_EVT_CONNECTION_COMPLETE    0x3EU
#define HCI_EVT_CONNECTION_REQUEST     0x04U
#define HCI_EVT_DISCONNECTION_COMPLETE 0x05U
#define HCI_EVT_AUTH_COMPLETE          0x06U
#define HCI_EVT_REMOTE_NAME_REQUEST    0x07U
#define HCI_EVT_ENCRYPT_CHANGE         0x08U
#define HCI_EVT_INQUIRY_COMPLETE       0x01U
#define HCI_EVT_COMMAND_COMPLETE       0x0EU
#define HCI_EVT_COMMAND_STATUS         0x0FU
#define HCI_EVT_NUM_COMPLETED_PACKETS  0x13U
#define HCI_EVT_DATA_BUFFER_OVERFLOW   0x1AU
#define HCI_EVT_LE_META                0x3EU

/*PSM L2CAP*/
#define L2CAP_PSM_SDP      0x0001U
#define L2CAP_PSM_RFCOMM   0x0003U
#define L2CAP_PSM_TCS_BIN  0x0005U
#define L2CAP_PSM_HID_CTRL 0x0011U
#define L2CAP_PSM_HID_INTR 0x0013U
#define L2CAP_PSM_AVCTP    0x0017U
#define L2CAP_PSM_AVDTP    0x0019U
#define L2CAP_PSM_AVCTP_BROWSE 0x001BU

/*A2DP*/
#define A2DP_SEID_SOURCE   0x00U
#define A2DP_SEID_SINK     0x01U
#define A2DP_CODEC_SBC     0x00U
#define A2DP_CODEC_MPEG12  0x01U
#define A2DP_CODEC_MPEG24  0x02U
#define A2DP_CODEC_ATRAC   0x02U

/*Stan urzadzenia Bluetooth*/
#define BT_STATE_POWERED_OFF  0x00U
#define BT_STATE_POWERED_ON   0x01U
#define BT_STATE_DISCOVERING  0x02U
#define BT_STATE_CONNECTING   0x03U
#define BT_STATE_CONNECTED    0x04U

/*Typ urzadzenia Bluetooth*/
#define BT_DEVICE_HEADPHONES  0x01U
#define BT_DEVICE_SPEAKER     0x02U
#define BT_DEVICE_PHONE       0x04U
#define BT_DEVICE_COMPUTER    0x08U

/*Maksymalna liczba urzadzen i polaczen*/
#define BT_MAX_DEVICES        16U
#define BT_MAX_CONNECTIONS    4U
#define BT_MAX_SERVICES       8U
#define BT_ADDR_LEN           6U
#define BT_LINK_KEY_LEN       16U
#define BT_PIN_CODE_MAX_LEN   16U

/*Struktura adresu Bluetooth (BD_ADDR)*/
struct bt_addr {
    uint8_t b[BT_ADDR_LEN];
};

/*Struktura klucza laczenia (link key)*/
struct bt_link_key {
    uint8_t key[BT_LINK_KEY_LEN];
};

/*Informacje o urzadzeniu Bluetooth*/
struct bt_device {
    uint8_t used;
    struct bt_addr addr;
    uint8_t dev_class[3];
    uint8_t rssi;
    char name[64];
    uint8_t device_type;
    uint8_t bonded;
    struct bt_link_key link_key;
    uint8_t connected;
    uint16_t handle;
    uint8_t services[BT_MAX_SERVICES];
    uint8_t service_count;
};

/*Polaczenie L2CAP*/
struct bt_l2cap_connection {
    uint8_t used;
    uint16_t cid;
    uint16_t remote_cid;
    uint16_t psm;
    uint16_t handle;
    uint8_t remote_addr[BT_ADDR_LEN];
    uint8_t configured;
    uint8_t state;
    uint16_t mtu;
    uint16_t flush_timeout;
    uint8_t rx_credits;
    uint8_t tx_credits;
};

/*Strumien A2DP (dla sluchawek)*/
struct bt_a2dp_stream {
    uint8_t used;
    uint8_t seid;
    uint8_t codec;
    uint16_t l2cap_cid;
    uint16_t l2cap_media_cid;
    uint16_t handle;
    uint8_t connected;
    uint8_t streaming;
    uint32_t sample_rate;
    uint8_t channels;
    uint8_t last_frame[512];
    uint32_t last_frame_len;
};

/*Kontekst sterownika Bluetooth*/
struct bt_context {
    uint8_t initialized;
    uint8_t powered;
    uint8_t state;
    uint8_t hci_ready;
    uint8_t l2cap_ready;
    uint8_t a2dp_ready;
    struct bt_addr local_addr;
    char local_name[64];
    uint8_t local_dev_class[3];
    struct bt_device devices[BT_MAX_DEVICES];
    uint8_t device_count;
    struct bt_l2cap_connection connections[BT_MAX_CONNECTIONS];
    struct bt_a2dp_stream a2dp_stream;
    uint8_t discovering;
};

/*================== API STEROWNIKA BLUETOOTH ==================*/

/*Inicjalizuje stos Bluetooth: wykrywa kontroler na USB, inicjalizuje HCI,
 *L2CAP, A2DP, wlacza kontroler i przygotowuje do pracy.*/
int bluetooth_init(void);

/*Czy sterownik Bluetooth jest zainicjowany i gotowy.*/
int bluetooth_is_ready(void);

/*Wlacz/wylacz kontroler Bluetooth.*/
int bluetooth_power_set(uint8_t enable);

/*Czy Bluetooth jest wlaczone.*/
int bluetooth_power_get(void);

/*Rozpocznij skanowanie urzadzen Bluetooth (inquiry).*/
int bluetooth_inquiry_start(void);

/*Zatrzymaj skanowanie.*/
int bluetooth_inquiry_stop(void);

/*Czy skanowanie trwa.*/
int bluetooth_is_discovering(void);

/*Pobierz liczbe wykrytych urzadzen.*/
uint8_t bluetooth_device_count(void);

/*Pobierz informacje o urzadzeniu po indeksie.*/
const struct bt_device *bluetooth_device_get(uint8_t index);

/*Znajdz urzadzenie po adresie.*/
struct bt_device *bluetooth_device_find(const struct bt_addr *addr);

/*Polacz sie z urzadzeniem Bluetooth.*/
int bluetooth_connect(struct bt_device *device);

/*Rozlacz sie z urzadzeniem.*/
int bluetooth_disconnect(struct bt_device *device);

/*Czy urzadzenie jest polaczone.*/
int bluetooth_is_connected(struct bt_device *device);

/*Sparuj sie z urzadzeniem (wymiana kluczy laczenia).*/
int bluetooth_pair(struct bt_device *device);

/*Usun parowanie (unbond).*/
int bluetooth_unpair(struct bt_device *device);

/*Czy urzadzenie jest zsparowane.*/
int bluetooth_is_bonded(struct bt_device *device);

/*Ustaw nazwe lokalnego urzadzenia Bluetooth.*/
int bluetooth_set_local_name(const char *name);

/*Pobierz nazwe lokalnego urzadzenia.*/
const char *bluetooth_get_local_name(void);

/*Pobierz adres lokalnego urzadzenia.*/
const struct bt_addr *bluetooth_get_local_addr(void);

/*A2DP - otworz strumien audio do sluchawek.*/
int bluetooth_a2dp_open(struct bt_device *device);

/*A2DP - zamknij strumien audio.*/
int bluetooth_a2dp_close(struct bt_device *device);

/*A2DP - wyslij ramke audio (SBC) do sluchawek.*/
int bluetooth_a2dp_send(struct bt_device *device, const uint8_t *data, uint32_t length);

/*A2DP - czy strumien jest aktywny.*/
int bluetooth_a2dp_streaming(struct bt_device *device);

/*Obsluga zdarzenia klikniecia ikony Bluetooth na pasku zadan (otwiera okno wyboru urzadzenia).*/
void bluetooth_ui_toggle(void);

/*Obsluga zdarzenia myszy na ikonie Bluetooth (dla paska zadan).*/
int bluetooth_ui_mouse_click(int x, int y);

/*Rysowanie ikony Bluetooth na pasku zadan.*/
void bluetooth_ui_draw_icon(int x, int y);

/*Czy pozycja (x,y) jest nad ikona Bluetooth na pasku zadan.*/
int bluetooth_ui_icon_at(int x, int y);

/*Pobierz kontekst Bluetooth (do odczytu).*/
void bluetooth_context_get(struct bt_context *out);

/*Inicjalizacja interfejsu uzytkownika bluetooth.*/
void bluetooth_ui_init(void);

/*Polling warstw bluetooth (wywolywane z petli glownej jadra).*/
void bluetooth_poll(void);

/*Rysuj ikone bluetooth na pasku zadan.*/
void bluetooth_ui_rysuj_pasek(void);

/*Aktualizuj stan ikony bluetooth.*/
void bluetooth_ui_update_state(void);

/*Pomocnicze funkcje warstwy hci (uzywane przez l2cap.cpp).*/
int bt_hci_send_acl(uint16_t handle, uint16_t data_len, const uint8_t *data);

#ifdef __cplusplus
}
#endif

#endif
