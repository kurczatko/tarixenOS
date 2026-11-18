#ifndef GSM_H
#define GSM_H

#include <include/printf/types.h>

#define GSM_MAX_CALLS     4
#define GSM_MAX_SMS       20
#define GSM_AT_TIMEOUT_MS 5000
#define GSM_IMEI_LEN      16
#define GSM_IMSI_LEN      16
#define GSM_NUMBER_LEN    32
#define GSM_SMS_MAX_LEN   160
#define GSM_CMD_BUF_SIZE  256
#define GSM_RX_BUF_SIZE   512

typedef enum {
    GSM_CMD_NONE,
    GSM_CMD_INIT,
    GSM_CMD_CHECK_PIN,
    GSM_CMD_CHECK_REG,
    GSM_CMD_CHECK_SIGNAL,
    GSM_CMD_GET_IMEI,
    GSM_CMD_DIAL,
    GSM_CMD_ANSWER,
    GSM_CMD_HANGUP,
    GSM_CMD_SEND_SMS,
    GSM_CMD_SEND_USSD,
    GSM_CMD_CHECK_BATTERY,
} gsm_cmd_t;

typedef enum {
    GSM_STATE_UNINIT,
    GSM_STATE_INIT,
    GSM_STATE_READY,
    GSM_STATE_ERROR,
} gsm_state_t;

typedef enum {
    GSM_CALL_STATE_IDLE,
    GSM_CALL_STATE_DIALING,
    GSM_CALL_STATE_RINGING,
    GSM_CALL_STATE_ACTIVE,
    GSM_CALL_STATE_HELD,
    GSM_CALL_STATE_DISCONNECTING,
} gsm_call_state_t;

typedef enum {
    GSM_CALL_TYPE_VOICE,
    GSM_CALL_TYPE_DATA,
    GSM_CALL_TYPE_FAX,
} gsm_call_type_t;

typedef struct {
    uint8_t    used;
    uint8_t    call_id;
    gsm_call_state_t state;
    gsm_call_type_t type;
    char       number[GSM_NUMBER_LEN];
    uint32_t   duration_sec;
} gsm_call_t;

typedef struct {
    gsm_cmd_t      type;
    char           cmd[GSM_CMD_BUF_SIZE];
    char           expected_resp[64];
    uint8_t        retries;
    uint32_t       timeout_ms;
    uint32_t       start_tick;
    int            (*callback)(const char *resp);
} gsm_at_cmd_t;

typedef struct {
    uint8_t        buf[GSM_RX_BUF_SIZE];
    uint32_t       head;
    uint32_t       tail;
    uint32_t       count;
} gsm_rx_ring_t;

typedef struct {
    gsm_state_t      state;
    uint8_t          initialized;
    uint8_t          uart_ready;
    char             imei[GSM_IMEI_LEN];
    char             imsi[GSM_IMSI_LEN];
    char             number[GSM_NUMBER_LEN];
    uint8_t          signal_strength;
    uint8_t          ber;
    uint8_t          battery_percent;
    uint8_t          charging;
    uint8_t          network_registered;
    uint8_t          roaming;
    uint8_t          sim_ready;
    gsm_at_cmd_t     current_cmd;
    gsm_rx_ring_t    rx;
    char             rx_line[GSM_CMD_BUF_SIZE];
    uint32_t         rx_line_len;
    uint32_t         last_activity_tick;
    gsm_call_t       calls[GSM_MAX_CALLS];
    uint8_t          active_call_count;
    uint8_t          incoming_call_id;
    char             incoming_number[GSM_NUMBER_LEN];
    void            (*on_call_state)(uint8_t call_id, gsm_call_state_t state, const char *number);
    void            (*on_sms_received)(const char *number, const char *message);
    void            (*on_network_change)(uint8_t registered);
    void            (*on_signal_change)(uint8_t strength);
} gsm_modem_t;

void gsm_init(void);
void gsm_poll(void);
gsm_state_t gsm_get_state(void);
int gsm_dial(const char *number);
int gsm_answer(uint8_t call_id);
int gsm_hangup(uint8_t call_id);
int gsm_send_sms(const char *number, const char *message);
int gsm_send_ussd(const char *code);
int gsm_get_signal_strength(void);
int gsm_get_battery_percent(void);
int gsm_is_network_registered(void);
int gsm_is_sim_ready(void);
const char *gsm_get_imei(void);
const char *gsm_get_imsi(void);
const char *gsm_get_own_number(void);
gsm_call_t *gsm_get_calls(void);
uint8_t gsm_get_active_call_count(void);
gsm_call_t *gsm_get_call_by_id(uint8_t call_id);
int gsm_is_call_active(uint8_t call_id);
int gsm_is_call_ringing(uint8_t call_id);
uint32_t gsm_get_call_duration(uint8_t call_id);
void gsm_set_on_call_state(void (*cb)(uint8_t call_id, gsm_call_state_t state, const char *number));
void gsm_set_on_sms_received(void (*cb)(const char *number, const char *message));
void gsm_set_on_network_change(void (*cb)(uint8_t registered));
void gsm_set_on_signal_change(void (*cb)(uint8_t strength));

#endif
