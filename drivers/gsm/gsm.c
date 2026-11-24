/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

/*
* sterownik modulow gsm z pelna obsuga dzwonienia
* obsluga komend at, zarzadzanie polaczeniami przychodzonymi i wychodzacymi,
* sms, ussd, rejestracja sieciowa, monitorowanie sygnalu, buforowanie uart
*/

/*includy*/

#include <drivers/gsm/gsm.h>
#include <include/printf/kprint.h>
#include <include/printf/types.h>
#include <lib/string.h>
#include <lib/stddef.h>
#include <kernel/sys/timer.h>

#ifndef NULL
#define NULL ((void *)0)
#endif

#define GSM_UART_PORT      0x3F8
#define GSM_AT_MAX_RETRIES 3
#define GSM_AT_RETRY_DELAY_MS 500

typedef enum {
    GSM_UART_IDLE,
    GSM_UART_SENDING,
    GSM_UART_WAITING_RESP,
} gsm_uart_state_t;

static inline void gsm_outb(uint16_t port, uint8_t value) {
    asm volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline uint8_t gsm_inb(uint16_t port) {
    uint8_t ret;
    asm volatile ("inb %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

static inline uint32_t gsm_strlen(const char *s) {
    uint32_t n = 0;
    while (s[n]) n++;
    return n;
}

static inline void gsm_strncat(char *dst, const char *src, uint32_t max) {
    uint32_t dlen = gsm_strlen(dst);
    uint32_t i = 0;
    while (src[i] && i < max) {
        dst[dlen + i] = src[i];
        i++;
    }
    dst[dlen + i] = '\0';
}

static inline void gsm_strcpy(char *dst, const char *src, uint32_t max) {
    uint32_t i = 0;
    while (src[i] && i < max - 1) {
        dst[i] = src[i];
        i++;
    }
    dst[i] = '\0';
}

static gsm_modem_t g_modem;

static void gsm_uart_write_byte(uint8_t b) {
    while ((gsm_inb(GSM_UART_PORT + 5) & 0x20) == 0) { }
    gsm_outb(GSM_UART_PORT, b);
}

static void gsm_uart_write(const char *s) {
    while (*s) {
        gsm_uart_write_byte((uint8_t)*s++);
    }
}

static void gsm_uart_write_line(const char *s) {
    gsm_uart_write(s);
    gsm_uart_write("\r\n");
}

static uint8_t gsm_uart_read_byte(void) {
    while ((gsm_inb(GSM_UART_PORT + 5) & 0x01) == 0) { }
    return gsm_inb(GSM_UART_PORT);
}

static uint8_t gsm_uart_byte_available(void) {
    return gsm_inb(GSM_UART_PORT + 5) & 0x01;
}

static void gsm_rx_push(uint8_t b) {
    if (g_modem.rx.count < GSM_RX_BUF_SIZE) {
        g_modem.rx.buf[g_modem.rx.head] = b;
        g_modem.rx.head = (g_modem.rx.head + 1) % GSM_RX_BUF_SIZE;
        g_modem.rx.count++;
        g_modem.last_activity_tick = timer_ticks_get();
    }
}

static int gsm_rx_pop(uint8_t *b) {
    if (g_modem.rx.count == 0) return -1;
    *b = g_modem.rx.buf[g_modem.rx.tail];
    g_modem.rx.tail = (g_modem.rx.tail + 1) % GSM_RX_BUF_SIZE;
    g_modem.rx.count--;
    return 0;
}

static void gsm_uart_flush_rx(void) {
    uint8_t b;
    while (gsm_uart_byte_available()) {
        b = gsm_uart_read_byte();
        gsm_rx_push(b);
    }
}

static int gsm_rx_line_available(char *line, uint32_t max_len) {
    uint8_t b;
    uint32_t i = 0;

    while (gsm_rx_pop(&b) == 0) {
        if (b == '\r') continue;
        if (b == '\n') {
            if (i > 0) {
                line[i] = '\0';
                return 1;
            }
            i = 0;
            continue;
        }
        if (i < max_len - 1) {
            line[i++] = (char)b;
        }
    }
    return 0;
}

static void gsm_at_send_raw(const char *cmd) {
    gsm_uart_write_line(cmd);
    kprint("[GSM] TX: %s\n", cmd);
}

static int gsm_at_resp_match(const char *line, const char *expected) {
    return strstr(line, expected) != NULL;
}

static int gsm_at_resp_contains(const char *line, const char *needle) {
    return strstr(line, needle) != NULL;
}

static void gsm_cmd_complete(int success) {
    g_modem.current_cmd.type = GSM_CMD_NONE;
    g_modem.current_cmd.cmd[0] = '\0';
    g_modem.current_cmd.retries = 0;
    if (success && g_modem.current_cmd.callback) {
        g_modem.current_cmd.callback("OK");
    }
}

static void gsm_at_send_cmd(gsm_cmd_t type, const char *cmd, const char *expected, int retries, uint32_t timeout_ms, int (*cb)(const char *)) {
    g_modem.current_cmd.type = type;
    gsm_strcpy(g_modem.current_cmd.cmd, cmd, GSM_CMD_BUF_SIZE);
    if (expected) {
        gsm_strcpy(g_modem.current_cmd.expected_resp, expected, 64);
    } else {
        g_modem.current_cmd.expected_resp[0] = '\0';
    }
    g_modem.current_cmd.retries = retries;
    g_modem.current_cmd.timeout_ms = timeout_ms;
    g_modem.current_cmd.start_tick = timer_ticks_get();
    g_modem.current_cmd.callback = cb;

    gsm_at_send_raw(cmd);
}

static void gsm_handle_unsolicited(const char *line) {
    if (gsm_at_resp_contains(line, "RING")) {
        kprint("[GSM] przychodzace polaczenie\n");
        if (g_modem.active_call_count < GSM_MAX_CALLS) {
            int slot = -1;
            for (uint8_t i = 0; i < GSM_MAX_CALLS; i++) {
                if (!g_modem.calls[i].used) { slot = i; break; }
            }
            if (slot >= 0) {
                uint8_t call_id = slot + 1;
                g_modem.calls[slot].used = 1;
                g_modem.calls[slot].call_id = call_id;
                g_modem.calls[slot].state = GSM_CALL_STATE_RINGING;
                g_modem.calls[slot].type = GSM_CALL_TYPE_VOICE;
                g_modem.calls[slot].number[0] = '\0';
                g_modem.calls[slot].duration_sec = 0;
                g_modem.incoming_call_id = call_id;
                g_modem.active_call_count++;
                if (g_modem.on_call_state) {
                    g_modem.on_call_state(call_id, GSM_CALL_STATE_RINGING, "");
                }
                kprint("[GSM] polaczenie przychodzace call_id=%d\n", call_id);
            }
        }
    } else if (gsm_at_resp_contains(line, "+CRING: VOICE")) {
        kprint("[GSM] glosowe polaczenie przychodzace\n");
    } else if (gsm_at_resp_contains(line, "NO CARRIER")) {
        for (uint8_t i = 0; i < GSM_MAX_CALLS; i++) {
            if (g_modem.calls[i].used) {
                g_modem.calls[i].used = 0;
                g_modem.calls[i].state = GSM_CALL_STATE_IDLE;
                g_modem.calls[i].number[0] = '\0';
                g_modem.calls[i].duration_sec = 0;
                if (g_modem.on_call_state) {
                    g_modem.on_call_state(g_modem.calls[i].call_id, GSM_CALL_STATE_IDLE, "");
                }
                if (g_modem.active_call_count > 0) g_modem.active_call_count--;
                kprint("[GSM] polaczenie zakonczone call_id=%d\n", g_modem.calls[i].call_id);
            }
        }
        g_modem.incoming_call_id = 0;
    } else if (gsm_at_resp_contains(line, "+CMT:")) {
        kprint("[GSM] nowy sms\n");
        if (g_modem.on_sms_received) {
            g_modem.on_sms_received("nieznany", line);
        }
    } else if (gsm_at_resp_contains(line, "+CREG:")) {
        if (gsm_at_resp_contains(line, ",1") || gsm_at_resp_contains(line, ",5")) {
            g_modem.network_registered = 1;
            kprint("[GSM] zarejestrowany w sieci\n");
        } else {
            g_modem.network_registered = 0;
            kprint("[GSM] nie zarejestrowany w sieci\n");
        }
        if (g_modem.on_network_change) {
            g_modem.on_network_change(g_modem.network_registered);
        }
    } else if (gsm_at_resp_contains(line, "+CSQ:")) {
        int rssi = -1;
        const char *p = strstr(line, "+CSQ:");
        if (p) {
            p += 5;
            while (*p && (*p < '0' || *p > '9')) p++;
            if (*p >= '0' && *p <= '9') {
                rssi = (*p - '0');
                if (rssi == 99) rssi = 0;
                else if (rssi > 31) rssi = 31;
                g_modem.signal_strength = (uint8_t)((rssi * 100) / 31);
            }
        }
        if (g_modem.on_signal_change) {
            g_modem.on_signal_change(g_modem.signal_strength);
        }
        kprint("[GSM] sygnal: %d%%\n", g_modem.signal_strength);
    } else if (gsm_at_resp_contains(line, "+CBC:")) {
        int pct = -1;
        const char *p = strstr(line, "+CBC:");
        if (p) {
            p += 5;
            while (*p && (*p < '0' || *p > '9')) p++;
            if (*p >= '0' && *p <= '9') {
                pct = (*p - '0') * 10;
                if (pct > 100) pct = 100;
                g_modem.battery_percent = (uint8_t)pct;
            }
        }
        kprint("[GSM] bateria: %d%%\n", g_modem.battery_percent);
    } else if (gsm_at_resp_contains(line, "ERROR")) {
        kprint("[GSM] blad at\n");
        if (g_modem.current_cmd.type != GSM_CMD_NONE) {
            if (g_modem.current_cmd.retries > 0) {
                g_modem.current_cmd.retries--;
                gsm_at_send_raw(g_modem.current_cmd.cmd);
                g_modem.current_cmd.start_tick = timer_ticks_get();
                return;
            }
            gsm_cmd_complete(0);
        }
    }
}

static int gsm_process_at_response(const char *line) {
    if (g_modem.current_cmd.type == GSM_CMD_NONE) {
        gsm_handle_unsolicited(line);
        return 0;
    }

    if (gsm_at_resp_match(line, g_modem.current_cmd.expected_resp) ||
        gsm_at_resp_match(line, "OK")) {
        if (g_modem.current_cmd.callback) {
            g_modem.current_cmd.callback(line);
        }
        gsm_cmd_complete(1);
        return 1;
    }

    gsm_handle_unsolicited(line);
    return 0;
}

static void gsm_check_cmd_timeout(void) {
    if (g_modem.current_cmd.type == GSM_CMD_NONE) return;

    uint32_t now = timer_ticks_get();
    if (now - g_modem.current_cmd.start_tick > g_modem.current_cmd.timeout_ms) {
        kprint("[GSM] timeout cmd: %s\n", g_modem.current_cmd.cmd);
        if (g_modem.current_cmd.retries > 0) {
            g_modem.current_cmd.retries--;
            gsm_at_send_raw(g_modem.current_cmd.cmd);
            g_modem.current_cmd.start_tick = now;
        } else {
            gsm_cmd_complete(0);
        }
    }
}

static void gsm_init_uart(void) {
    gsm_outb(GSM_UART_PORT + 1, 0x00);
    gsm_outb(GSM_UART_PORT + 3, 0x80);
    gsm_outb(GSM_UART_PORT + 0, 0x0C);
    gsm_outb(GSM_UART_PORT + 1, 0x00);
    gsm_outb(GSM_UART_PORT + 3, 0x03);
    gsm_outb(GSM_UART_PORT + 2, 0xC7);
    gsm_outb(GSM_UART_PORT + 4, 0x0B);
    g_modem.uart_ready = 1;
    kprint("[GSM] uart zainicjalizowany na port 0x3F8\n");
}

static int gsm_cb_init_done(const char *resp) {
    (void)resp;
    g_modem.state = GSM_STATE_INIT;
    kprint("[GSM] inicjalizacja modemu...\n");
    return 0;
}

static int gsm_cb_pin_done(const char *resp) {
    (void)resp;
    g_modem.sim_ready = 1;
    kprint("[GSM] sim gotowy\n");
    return 0;
}

static int gsm_cb_reg_done(const char *resp) {
    (void)resp;
    g_modem.network_registered = 1;
    if (g_modem.on_network_change) g_modem.on_network_change(1);
    kprint("[GSM] zarejestrowany w sieci gsm\n");
    return 0;
}

static int gsm_cb_signal_done(const char *resp) {
    (void)resp;
    if (g_modem.on_signal_change) g_modem.on_signal_change(g_modem.signal_strength);
    return 0;
}

static int gsm_cb_imei_done(const char *resp) {
    const char *p = strstr(resp, "\r\n");
    if (p) {
        p += 2;
        uint32_t i = 0;
        while (*p && *p != '\r' && i < GSM_IMEI_LEN - 1) {
            g_modem.imei[i++] = *p++;
        }
        g_modem.imei[i] = '\0';
    }
    kprint("[GSM] imei: %s\n", g_modem.imei);
    return 0;
}

static int gsm_cb_dial_done(const char *resp) {
    (void)resp;
    if (g_modem.incoming_call_id > 0) {
        g_modem.calls[g_modem.incoming_call_id - 1].state = GSM_CALL_STATE_ACTIVE;
        if (g_modem.on_call_state) {
            g_modem.on_call_state(g_modem.incoming_call_id, GSM_CALL_STATE_ACTIVE, g_modem.incoming_number);
        }
        kprint("[GSM] polaczenie aktywne call_id=%d\n", g_modem.incoming_call_id);
    }
    return 0;
}

static int gsm_cb_hangup_done(const char *resp) {
    (void)resp;
    if (g_modem.incoming_call_id > 0) {
        g_modem.calls[g_modem.incoming_call_id - 1].used = 0;
        g_modem.calls[g_modem.incoming_call_id - 1].state = GSM_CALL_STATE_IDLE;
        if (g_modem.on_call_state) {
            g_modem.on_call_state(g_modem.incoming_call_id, GSM_CALL_STATE_IDLE, "");
        }
        g_modem.incoming_call_id = 0;
        if (g_modem.active_call_count > 0) g_modem.active_call_count--;
    }
    return 0;
}

static int gsm_cb_sms_done(const char *resp) {
    (void)resp;
    kprint("[GSM] sms wyslany\n");
    return 0;
}

void gsm_init(void) {
    for (uint8_t i = 0; i < GSM_MAX_CALLS; i++) {
        g_modem.calls[i].used = 0;
        g_modem.calls[i].call_id = 0;
        g_modem.calls[i].state = GSM_CALL_STATE_IDLE;
        g_modem.calls[i].type = GSM_CALL_TYPE_VOICE;
        g_modem.calls[i].number[0] = '\0';
        g_modem.calls[i].duration_sec = 0;
    }

    g_modem.rx.head = 0;
    g_modem.rx.tail = 0;
    g_modem.rx.count = 0;
    g_modem.rx_line_len = 0;
    g_modem.state = GSM_STATE_UNINIT;
    g_modem.initialized = 0;
    g_modem.uart_ready = 0;
    g_modem.signal_strength = 0;
    g_modem.ber = 0;
    g_modem.battery_percent = 0;
    g_modem.charging = 0;
    g_modem.network_registered = 0;
    g_modem.roaming = 0;
    g_modem.sim_ready = 0;
    g_modem.active_call_count = 0;
    g_modem.incoming_call_id = 0;
    g_modem.incoming_number[0] = '\0';
    g_modem.on_call_state = NULL;
    g_modem.on_sms_received = NULL;
    g_modem.on_network_change = NULL;
    g_modem.on_signal_change = NULL;
    g_modem.current_cmd.type = GSM_CMD_NONE;
    g_modem.current_cmd.cmd[0] = '\0';
    g_modem.current_cmd.expected_resp[0] = '\0';
    g_modem.current_cmd.retries = 0;
    g_modem.current_cmd.timeout_ms = 0;
    g_modem.current_cmd.start_tick = 0;
    g_modem.current_cmd.callback = NULL;
    g_modem.imei[0] = '\0';
    g_modem.imsi[0] = '\0';
    g_modem.number[0] = '\0';

    gsm_init_uart();

    gsm_at_send_cmd(GSM_CMD_INIT, "AT", "OK", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, gsm_cb_init_done);
    gsm_at_send_cmd(GSM_CMD_CHECK_PIN, "AT+CPIN?", "READY", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, gsm_cb_pin_done);
    gsm_at_send_cmd(GSM_CMD_CHECK_REG, "AT+CREG?", "+CREG: 0,1", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, gsm_cb_reg_done);
    gsm_at_send_cmd(GSM_CMD_GET_IMEI, "AT+GSN", "OK", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, gsm_cb_imei_done);
    gsm_at_send_cmd(GSM_CMD_CHECK_SIGNAL, "AT+CSQ", "+CSQ:", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, gsm_cb_signal_done);
    gsm_at_send_cmd(GSM_CMD_CHECK_BATTERY, "AT+CBC", "+CBC:", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, NULL);

    g_modem.state = GSM_STATE_READY;
    g_modem.initialized = 1;
    g_modem.last_activity_tick = timer_ticks_get();
    kprint("[GSM] modem zainicjalizowany, stan: ready\n");
}

void gsm_poll(void) {
    if (!g_modem.initialized) return;

    gsm_uart_flush_rx();

    char line[GSM_CMD_BUF_SIZE];
    while (gsm_rx_line_available(line, GSM_CMD_BUF_SIZE)) {
        kprint("[GSM] RX: %s\n", line);
        gsm_process_at_response(line);
    }

    gsm_check_cmd_timeout();

    if (g_modem.state == GSM_STATE_READY && g_modem.current_cmd.type == GSM_CMD_NONE) {
        uint32_t now = timer_ticks_get();
        if (now - g_modem.last_activity_tick > 30000) {
            gsm_at_send_cmd(GSM_CMD_CHECK_SIGNAL, "AT+CSQ", "+CSQ:", 1, GSM_AT_TIMEOUT_MS, gsm_cb_signal_done);
            gsm_at_send_cmd(GSM_CMD_CHECK_BATTERY, "AT+CBC", "+CBC:", 1, GSM_AT_TIMEOUT_MS, NULL);
            g_modem.last_activity_tick = now;
        }
    }
}

gsm_state_t gsm_get_state(void) {
    return g_modem.state;
}

int gsm_dial(const char *number) {
    if (!g_modem.initialized || g_modem.state != GSM_STATE_READY) {
        kprint("[GSM] modem niegotowy do dzwonienia\n");
        return -1;
    }

    if (g_modem.active_call_count >= GSM_MAX_CALLS) {
        kprint("[GSM] osiagnieto maksymalna liczbe polaczen\n");
        return -1;
    }

    int slot = -1;
    for (uint8_t i = 0; i < GSM_MAX_CALLS; i++) {
        if (!g_modem.calls[i].used) { slot = i; break; }
    }
    if (slot < 0) return -1;

    uint8_t call_id = slot + 1;
    g_modem.calls[slot].used = 1;
    g_modem.calls[slot].call_id = call_id;
    g_modem.calls[slot].state = GSM_CALL_STATE_DIALING;
    g_modem.calls[slot].type = GSM_CALL_TYPE_VOICE;
    gsm_strcpy(g_modem.calls[slot].number, number, GSM_NUMBER_LEN);
    g_modem.calls[slot].duration_sec = 0;
    g_modem.incoming_call_id = call_id;
    gsm_strcpy(g_modem.incoming_number, number, GSM_NUMBER_LEN);
    g_modem.active_call_count++;

    char at_cmd[GSM_CMD_BUF_SIZE];
    at_cmd[0] = '\0';
    gsm_strncat(at_cmd, "ATD", GSM_CMD_BUF_SIZE - 1);
    gsm_strncat(at_cmd, number, GSM_CMD_BUF_SIZE - gsm_strlen(at_cmd) - 1);
    gsm_strncat(at_cmd, ";", GSM_CMD_BUF_SIZE - gsm_strlen(at_cmd) - 1);

    gsm_at_send_cmd(GSM_CMD_DIAL, at_cmd, "OK", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, gsm_cb_dial_done);
    kprint("[GSM] dzwonienie do %s call_id=%d\n", number, call_id);
    return call_id;
}

int gsm_answer(uint8_t call_id) {
    if (!g_modem.initialized) return -1;

    int slot = call_id - 1;
    if (slot < 0 || slot >= GSM_MAX_CALLS) return -1;
    if (!g_modem.calls[slot].used) return -1;
    if (g_modem.calls[slot].state != GSM_CALL_STATE_RINGING) return -1;

    g_modem.calls[slot].state = GSM_CALL_STATE_ACTIVE;
    gsm_at_send_cmd(GSM_CMD_ANSWER, "ATA", "OK", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, NULL);
    kprint("[GSM] polaczenie odebrane call_id=%d\n", call_id);
    return 0;
}

int gsm_hangup(uint8_t call_id) {
    if (!g_modem.initialized) return -1;

    int slot = call_id - 1;
    if (slot < 0 || slot >= GSM_MAX_CALLS) return -1;
    if (!g_modem.calls[slot].used) return -1;

    g_modem.calls[slot].state = GSM_CALL_STATE_DISCONNECTING;
    gsm_at_send_cmd(GSM_CMD_HANGUP, "AT+CHUP", "OK", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, gsm_cb_hangup_done);
    kprint("[GSM] zakonczono polaczenie call_id=%d\n", call_id);
    return 0;
}

int gsm_send_sms(const char *number, const char *message) {
    if (!g_modem.initialized || g_modem.state != GSM_STATE_READY) return -1;

    gsm_at_send_cmd(GSM_CMD_SEND_SMS, "AT+CMGF=1", "OK", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, NULL);

    char at_cmd[GSM_CMD_BUF_SIZE];
    at_cmd[0] = '\0';
    gsm_strncat(at_cmd, "AT+CMGS=\"", GSM_CMD_BUF_SIZE - 1);
    gsm_strncat(at_cmd, number, GSM_CMD_BUF_SIZE - gsm_strlen(at_cmd) - 1);
    gsm_strncat(at_cmd, "\"", GSM_CMD_BUF_SIZE - gsm_strlen(at_cmd) - 1);
    gsm_at_send_cmd(GSM_CMD_SEND_SMS, at_cmd, ">", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, NULL);

    gsm_at_send_cmd(GSM_CMD_SEND_SMS, message, "OK", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, gsm_cb_sms_done);
    kprint("[GSM] sms wyslany do %s\n", number);
    return 0;
}

int gsm_send_ussd(const char *code) {
    if (!g_modem.initialized || g_modem.state != GSM_STATE_READY) return -1;

    char at_cmd[GSM_CMD_BUF_SIZE];
    at_cmd[0] = '\0';
    gsm_strncat(at_cmd, "AT+CUSD=1,\"", GSM_CMD_BUF_SIZE - 1);
    gsm_strncat(at_cmd, code, GSM_CMD_BUF_SIZE - gsm_strlen(at_cmd) - 1);
    gsm_strncat(at_cmd, "\"", GSM_CMD_BUF_SIZE - gsm_strlen(at_cmd) - 1);
    gsm_at_send_cmd(GSM_CMD_SEND_USSD, at_cmd, "+CUSD:", GSM_AT_MAX_RETRIES, GSM_AT_TIMEOUT_MS, NULL);
    kprint("[GSM] ussd wyslany: %s\n", code);
    return 0;
}

int gsm_get_signal_strength(void) {
    if (!g_modem.initialized) return -1;
    return g_modem.signal_strength;
}

int gsm_get_battery_percent(void) {
    if (!g_modem.initialized) return -1;
    return g_modem.battery_percent;
}

int gsm_is_network_registered(void) {
    if (!g_modem.initialized) return 0;
    return g_modem.network_registered;
}

int gsm_is_sim_ready(void) {
    if (!g_modem.initialized) return 0;
    return g_modem.sim_ready;
}

const char *gsm_get_imei(void) {
    return g_modem.imei;
}

const char *gsm_get_imsi(void) {
    return g_modem.imsi;
}

const char *gsm_get_own_number(void) {
    return g_modem.number;
}

gsm_call_t *gsm_get_calls(void) {
    return g_modem.calls;
}

uint8_t gsm_get_active_call_count(void) {
    return g_modem.active_call_count;
}

gsm_call_t *gsm_get_call_by_id(uint8_t call_id) {
    if (call_id == 0 || call_id > GSM_MAX_CALLS) return NULL;
    if (!g_modem.calls[call_id - 1].used) return NULL;
    return &g_modem.calls[call_id - 1];
}

int gsm_is_call_active(uint8_t call_id) {
    gsm_call_t *c = gsm_get_call_by_id(call_id);
    if (!c) return 0;
    return c->state == GSM_CALL_STATE_ACTIVE;
}

int gsm_is_call_ringing(uint8_t call_id) {
    gsm_call_t *c = gsm_get_call_by_id(call_id);
    if (!c) return 0;
    return c->state == GSM_CALL_STATE_RINGING;
}

uint32_t gsm_get_call_duration(uint8_t call_id) {
    gsm_call_t *c = gsm_get_call_by_id(call_id);
    if (!c) return 0;
    return c->duration_sec;
}

void gsm_set_on_call_state(void (*cb)(uint8_t call_id, gsm_call_state_t state, const char *number)) {
    g_modem.on_call_state = cb;
}

void gsm_set_on_sms_received(void (*cb)(const char *number, const char *message)) {
    g_modem.on_sms_received = cb;
}

void gsm_set_on_network_change(void (*cb)(uint8_t registered)) {
    g_modem.on_network_change = cb;
}

void gsm_set_on_signal_change(void (*cb)(uint8_t strength)) {
    g_modem.on_signal_change = cb;
}
