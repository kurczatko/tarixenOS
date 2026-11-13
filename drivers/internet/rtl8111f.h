#ifndef RTL8111F_H
#define RTL8111F_H

#include <include/printf/types.h>

#ifdef __cplusplus
extern "C" {
#endif

#define RTL8111F_VENDOR_ID 0x10ECU
#define RTL8111F_DEVICE_ID 0x8168U
#define RTL8111F_DEVICE_ID_VAR1 0x8167U
#define RTL8111F_DEVICE_ID_VAR2 0x8169U
#define RTL8111F_DEVICE_ID_VAR3 0x8171U
#define RTL8111F_DEVICE_ID_VAR4 0x8172U

#define RTL8111F_REG_MAC0      0x00U
#define RTL8111F_REG_MAC4      0x04U
#define RTL8111F_REG_MAR0      0x08U
#define RTL8111F_REG_TSD0      0x10U
#define RTL8111F_REG_TSD1      0x14U
#define RTL8111F_REG_TSD2      0x18U
#define RTL8111F_REG_TSD3      0x1CU
#define RTL8111F_REG_TSAD0     0x20U
#define RTL8111F_REG_TSAD1     0x24U
#define RTL8111F_REG_TSAD2     0x28U
#define RTL8111F_REG_TSAD3     0x2CU
#define RTL8111F_REG_RBSTART   0x30U
#define RTL8111F_REG_CMDR      0x37U
#define RTL8111F_REG_IMR       0x3CU
#define RTL8111F_REG_ISR       0x3EU
#define RTL8111F_REG_TCR       0x40U
#define RTL8111F_REG_RCR       0x44U
#define RTL8111F_REG_CFG9346   0x50U
#define RTL8111F_REG_PHYAR     0x6CU
#define RTL8111F_REG_OOCR      0xE0U
#define RTL8111F_REG_PHYSTATUS 0xDCU

#define RTL8111F_CMD_STOP        0x01U
#define RTL8111F_CMD_START       0x02U
#define RTL8111F_CMD_RX_ENABLE   0x04U
#define RTL8111F_CMD_TX_ENABLE   0x08U
#define RTL8111F_CMD_RESET_RX    0x10U
#define RTL8111F_CMD_RESET_TX    0x20U

#define RTL8111F_ISR_TX_OK       0x0001U
#define RTL8111F_ISR_RX_OK       0x0002U
#define RTL8111F_ISR_RX_ERR      0x0040U
#define RTL8111F_ISR_TX_ERR      0x0080U
#define RTL8111F_ISR_PCI_ERR     0x4000U
#define RTL8111F_ISR_SYS_ERR     0x8000U

#define RTL8111F_RCR_ACCEPT_MYMAC     0x0010U
#define RTL8111F_RCR_ACCEPT_PHYS      0x0020U
#define RTL8111F_RCR_ACCEPT_MULTICAST 0x0008U
#define RTL8111F_RCR_ACCEPT_BROADCAST 0x0004U
#define RTL8111F_RCR_ACCEPT_ERR       0x0001U
#define RTL8111F_RCR_ACCEPT_RUNT      0x0002U
#define RTL8111F_RCR_BUFFER_8K        0x0000U
#define RTL8111F_RCR_BUFFER_16K       0x0800U
#define RTL8111F_RCR_BUFFER_32K       0x1000U
#define RTL8111F_RCR_BUFFER_64K       0x1800U
#define RTL8111F_RCR_RXFTH_MASK       0x0300U

#define RTL8111F_TSD_OWN     (1U << 31U)
#define RTL8111F_TSD_LENGTH_MASK 0x0000FFFFU
#define RTL8111F_TSD_TX_UNDERRUN  (1U << 30U)
#define RTL8111F_TSD_TX_ABORTED   (1U << 29U)
#define RTL8111F_TSD_TX_CARRIER   (1U << 28U)

#define RTL8111F_PHY_REG_BMCR 0x00U
#define RTL8111F_PHY_REG_BMSR 0x01U
#define RTL8111F_PHY_REG_ID1  0x02U
#define RTL8111F_PHY_REG_ID2  0x03U
#define RTL8111F_PHY_REG_ANAR 0x04U
#define RTL8111F_PHY_REG_ANLPAR 0x05U
#define RTL8111F_PHY_REG_ANER 0x06U

#define RTL8111F_BMCR_RESET    (1U << 15U)
#define RTL8111F_BMCR_AUTONEG  (1U << 12U)
#define RTL8111F_BMCR_DUPLEX   (1U << 8U)
#define RTL8111F_BMCR_SPEED100 (1U << 13U)
#define RTL8111F_BMCR_PDOWN    (1U << 11U)
#define RTL8111F_BMCR_ISOLATE  (1U << 10U)
#define RTL8111F_BMCR_LOOPBACK (1U << 14U)

#define RTL8111F_BMSR_LINK     (1U << 2U)
#define RTL8111F_BMSR_AN_DONE  (1U << 5U)
#define RTL8111F_BMSR_100FD    (1U << 8U)
#define RTL8111F_BMSR_100HD    (1U << 7U)
#define RTL8111F_BMSR_1000FD   (1U << 11U)
#define RTL8111F_BMSR_10FD     (1U << 6U)
#define RTL8111F_BMSR_10HD     (1U << 5U)

#define RTL8111F_RX_BUF_SIZE 16384U
#define RTL8111F_TX_BUF_SIZE 2048U
#define RTL8111F_TX_DESC_COUNT 8U
#define RTL8111F_MAX_FRAME_SIZE 1518U

#define RTL8111F_LINK_DOWN   0U
#define RTL8111F_LINK_10_HD  1U
#define RTL8111F_LINK_10_FD  2U
#define RTL8111F_LINK_100_HD 3U
#define RTL8111F_LINK_100_FD 4U
#define RTL8111F_LINK_1000_FD 5U

#define RTL8111F_STATS_RESET 0x01U
#define RTL8111F_STATS_ALL   0x02U

struct rtl8111f_stats {
    uint32_t tx_packets;
    uint32_t tx_bytes;
    uint32_t tx_errors;
    uint32_t tx_deferred;
    uint32_t tx_underrun;
    uint32_t tx_aborted;
    uint32_t tx_carrier;
    uint32_t rx_packets;
    uint32_t rx_bytes;
    uint32_t rx_errors;
    uint32_t rx_missed;
    uint32_t rx_crc_errors;
    uint32_t rx_frame_errors;
    uint32_t rx_fifo_errors;
    uint32_t resets;
    uint32_t link_changes;
    uint32_t autoneg_failures;
};

struct rtl8111f_link_info {
    uint8_t link_up;
    uint8_t full_duplex;
    uint8_t autoneg_done;
    uint8_t speed_mbps;
};

int rtl8111f_init(void);
int rtl8111f_is_ready(void);
int rtl8111f_get_mac(uint8_t mac[6]);
int rtl8111f_send(const void *frame, uint16_t length);
int rtl8111f_receive(void *frame, uint16_t capacity);

int rtl8111f_link_up(void);
int rtl8111f_link_info(struct rtl8111f_link_info *info);
int rtl8111f_speed_mbps(void);
int rtl8111f_duplex_full(void);
int rtl8111f_autonegotiate(void);
int rtl8111f_reset_link(void);

int rtl8111f_loopback_test(const void *frame, uint16_t length);
void rtl8111f_set_multicast(const uint8_t *addresses, uint32_t count);
void rtl8111f_stats_get(struct rtl8111f_stats *out);
void rtl8111f_stats_reset(void);
void rtl8111f_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif
