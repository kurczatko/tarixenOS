/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 */

#include <drivers/network/vmxnet3.h>

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

#define VMX_VENDOR_ID      0x15ADU   /* VMware */
#define VMX_DEVICE_ID      0x07B0U   /* VMXNET3 Ethernet Controller */

/* BAR1: rejestry sterujace. */
#define VMX_REG_VRRS       0x000U   /* Vmxnet3 Revision Report Selection */
#define VMX_REG_UVRS       0x008U   /* UPT Version Report Selection */
#define VMX_REG_DSAL       0x010U   /* Driver Shared Area, dolne 32 bity */
#define VMX_REG_DSAH       0x018U   /* Driver Shared Area, gorne 32 bity */
#define VMX_REG_CMD        0x020U
#define VMX_REG_MACL       0x028U
#define VMX_REG_MACH       0x030U
#define VMX_REG_ICR        0x038U
#define VMX_REG_ECR        0x040U

/* BAR0: maski przerwan i doorbelle. */
#define VMX_REG_IMR        0x000U   /* + 8 * indeks przerwania; 1 = wylaczone */
#define VMX_REG_TXPROD     0x600U
#define VMX_REG_RXPROD     0x800U   /* ring 0 */
#define VMX_REG_RXPROD2    0xA00U   /* ring 1 */

/* Polecenia (zapis do CMD; wynik odczytem CMD). */
#define VMX_CMD_ACTIVATE_DEV     0xCAFE0000U
#define VMX_CMD_QUIESCE_DEV      0xCAFE0001U
#define VMX_CMD_RESET_DEV        0xCAFE0002U
#define VMX_CMD_UPDATE_RX_MODE   0xCAFE0003U
#define VMX_CMD_GET_LINK         0xF00D0002U

/* Tryby filtra odbioru. */
#define VMX_RXM_UCAST      0x01U
#define VMX_RXM_MCAST      0x02U
#define VMX_RXM_BCAST      0x04U
#define VMX_RXM_ALL_MULTI  0x08U
#define VMX_RXM_PROMISC    0x10U

#define VMX_REV1_MAGIC     0xBABEFEE1U
#define VMX_IC_DISABLE_ALL 0x1U
#define VMX_MAX_INTRS      25U

/* Rozmiary pierscieni - wielokrotnosc 32. */
#define VMX_RX0_SIZE       256U
#define VMX_RX1_SIZE       32U
#define VMX_RXC_SIZE       (VMX_RX0_SIZE + VMX_RX1_SIZE)
#define VMX_TX_SIZE        64U
#define VMX_TXC_SIZE       64U

#define VMX_BUF_SIZE       2048U
#define VMX_MIN_FRAME      60U
#define VMX_MAX_FRAME      1518U

/* TX dword[2]: len [0:13], gen [14]; dword[3]: eop [12], cq [13]. */
#define VMX_TXD_GEN_SHIFT  14U
#define VMX_TXD_EOP        (1U << 12)
#define VMX_TXD_CQ         (1U << 13)

/* RX dword[2]: len [0:13], btype [14], gen [31]. */
#define VMX_RXD_BTYPE_SHIFT 14U
#define VMX_RXD_GEN_SHIFT   31U

/* Uzupelnienia: gen w bicie 31 dword[3]. */
#define VMX_COMP_GEN_SHIFT  31U

/* ------------------------------------------------------------------ */
/* Struktury wspolne ze sprzetem (little endian, bez wypelnien).      */
/* ------------------------------------------------------------------ */

struct vmx_txdesc  { uint64_t addr; uint32_t dw2; uint32_t dw3; } __attribute__((packed));
struct vmx_txcomp  { uint32_t dw0; uint32_t dw1; uint32_t dw2; uint32_t dw3; } __attribute__((packed));
struct vmx_rxdesc  { uint64_t addr; uint32_t dw2; uint32_t dw3; } __attribute__((packed));
struct vmx_rxcomp  { uint32_t dw0; uint32_t dw1; uint32_t dw2; uint32_t dw3; } __attribute__((packed));

struct vmx_driver_info {
    uint32_t version;
    uint32_t gos;
    uint32_t rev_spt;
    uint32_t upt_ver_spt;
} __attribute__((packed));

struct vmx_misc_conf {
    struct vmx_driver_info driver_info;
    uint64_t upt_features;
    uint64_t dd_pa;
    uint64_t queue_desc_pa;
    uint32_t dd_len;
    uint32_t queue_desc_len;
    uint32_t mtu;
    uint16_t max_num_rx_sg;
    uint8_t  num_tx_queues;
    uint8_t  num_rx_queues;
    uint32_t reserved[4];
} __attribute__((packed));

struct vmx_intr_conf {
    uint8_t  auto_mask;
    uint8_t  num_intrs;
    uint8_t  event_intr_idx;
    uint8_t  mod_levels[VMX_MAX_INTRS];
    uint32_t intr_ctrl;
    uint32_t reserved[2];
} __attribute__((packed));

struct vmx_rx_filter_conf {
    uint32_t rx_mode;
    uint16_t mf_table_len;
    uint16_t pad1;
    uint64_t mf_table_pa;
    uint32_t vf_table[128];
} __attribute__((packed));

struct vmx_var_conf {
    uint32_t conf_ver;
    uint32_t conf_len;
    uint64_t conf_pa;
} __attribute__((packed));

struct vmx_ds_dev_read {
    struct vmx_misc_conf      misc;
    struct vmx_intr_conf      intr_conf;
    struct vmx_rx_filter_conf rx_filter_conf;
    struct vmx_var_conf       rss_conf;
    struct vmx_var_conf       pm_conf;
    struct vmx_var_conf       plugin_conf;
} __attribute__((packed));

struct vmx_driver_shared {
    uint32_t magic;
    uint32_t pad;
    struct vmx_ds_dev_read dev_read;
    uint32_t ecr;
    uint32_t reserved[5];
} __attribute__((packed));

struct vmx_txq_conf {
    uint64_t tx_ring_base_pa;
    uint64_t data_ring_base_pa;
    uint64_t comp_ring_base_pa;
    uint64_t dd_pa;
    uint64_t reserved;
    uint32_t tx_ring_size;
    uint32_t data_ring_size;
    uint32_t comp_ring_size;
    uint32_t dd_len;
    uint8_t  intr_idx;
    uint8_t  pad[7];
} __attribute__((packed));

struct vmx_rxq_conf {
    uint64_t rx_ring_base_pa[2];
    uint64_t comp_ring_base_pa;
    uint64_t dd_pa;
    uint64_t reserved;
    uint32_t rx_ring_size[2];
    uint32_t comp_ring_size;
    uint32_t dd_len;
    uint8_t  intr_idx;
    uint8_t  pad[7];
} __attribute__((packed));

struct vmx_txq_desc {
    uint32_t tx_num_deferred;           /* ctrl */
    uint32_t tx_threshold;
    uint64_t ctrl_reserved;
    struct vmx_txq_conf conf;
    uint8_t  status[8];                 /* QueueStatus */
    uint8_t  stats[80];                 /* UPT1_TxStats */
    uint8_t  pad[88];
} __attribute__((packed));

struct vmx_rxq_desc {
    uint8_t  update_rx_prod;            /* ctrl */
    uint8_t  ctrl_pad[7];
    uint64_t ctrl_reserved;
    struct vmx_rxq_conf conf;
    uint8_t  status[8];
    uint8_t  stats[80];                 /* UPT1_RxStats */
    uint8_t  pad[88];
} __attribute__((packed));

struct vmx_qdescs {
    struct vmx_txq_desc tx;
    struct vmx_rxq_desc rx;
} __attribute__((packed));

typedef char vmx_chk_misc[(sizeof(struct vmx_misc_conf) == 72) ? 1 : -1];
typedef char vmx_chk_intr[(sizeof(struct vmx_intr_conf) == 40) ? 1 : -1];
typedef char vmx_chk_rxf[(sizeof(struct vmx_rx_filter_conf) == 528) ? 1 : -1];
typedef char vmx_chk_devread[(sizeof(struct vmx_ds_dev_read) == 688) ? 1 : -1];
typedef char vmx_chk_txq[(sizeof(struct vmx_txq_desc) == 256) ? 1 : -1];
typedef char vmx_chk_rxq[(sizeof(struct vmx_rxq_desc) == 256) ? 1 : -1];
typedef char vmx_chk_desc[(sizeof(struct vmx_txdesc) == 16 && sizeof(struct vmx_rxdesc) == 16 &&
                           sizeof(struct vmx_txcomp) == 16 && sizeof(struct vmx_rxcomp) == 16) ? 1 : -1];

/* ------------------------------------------------------------------ */
/* Stan sterownika.                                                   */
/* ------------------------------------------------------------------ */

static volatile uint8_t *vmx_bar0;
static volatile uint8_t *vmx_bar1;

static uint8_t  vmx_mac[6];
static uint16_t vmx_mtu = 1500;
static uint8_t  vmx_ready;
static uint8_t  vmx_link;

static struct vmx_driver_shared vmx_shared __attribute__((aligned(4096)));
static struct vmx_qdescs        vmx_qd     __attribute__((aligned(4096)));

static struct vmx_txdesc vmx_tx_ring[VMX_TX_SIZE]    __attribute__((aligned(512)));
static struct vmx_txcomp vmx_tx_comp[VMX_TXC_SIZE]   __attribute__((aligned(512)));
static struct vmx_rxdesc vmx_rx0_ring[VMX_RX0_SIZE]  __attribute__((aligned(512)));
static struct vmx_rxdesc vmx_rx1_ring[VMX_RX1_SIZE]  __attribute__((aligned(512)));
static struct vmx_rxcomp vmx_rx_comp[VMX_RXC_SIZE]   __attribute__((aligned(512)));

static uint8_t vmx_tx_buf[VMX_TX_SIZE][VMX_BUF_SIZE]   __attribute__((aligned(16)));
static uint8_t vmx_rx0_buf[VMX_RX0_SIZE][VMX_BUF_SIZE] __attribute__((aligned(16)));
static uint8_t vmx_rx1_buf[VMX_RX1_SIZE][VMX_BUF_SIZE] __attribute__((aligned(16)));

static uint32_t vmx_tx_fill, vmx_tx_gen, vmx_tx_inflight;
static uint32_t vmx_txc_next, vmx_txc_gen;
static uint32_t vmx_rx_fill[2], vmx_rx_gen[2];
static uint32_t vmx_rxc_next, vmx_rxc_gen;

static struct {
    uint32_t tx_packets, rx_packets, tx_errors, rx_errors, tx_dropped, rx_dropped;
} vmx_stats;

static void barrier(void)
{
    __asm__ volatile("" : : : "memory");
}

/* ------------------------------------------------------------------ */
/* Porty I/O, PCI, MMIO.                                              */
/* ------------------------------------------------------------------ */

static void outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFCU);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static void pci_write32(uint8_t bus, uint8_t slot, uint8_t function,
                        uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFCU);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

static void     bar0_write(uint32_t off, uint32_t v) { *(volatile uint32_t *)(vmx_bar0 + off) = v; barrier(); }
static uint32_t bar1_read(uint32_t off)  { return *(volatile uint32_t *)(vmx_bar1 + off); }
static void     bar1_write(uint32_t off, uint32_t v) { *(volatile uint32_t *)(vmx_bar1 + off) = v; barrier(); }

/* Polecenie: zapis do CMD, wynik w odczycie CMD. */
static uint32_t vmx_cmd(uint32_t cmd)
{
    bar1_write(VMX_REG_CMD, cmd);
    return bar1_read(VMX_REG_CMD);
}

static uint64_t phys(const volatile void *p)
{
    return (uint64_t)(unsigned long)p;
}

static void vmx_zero(void *p, uint32_t n)
{
    volatile uint8_t *b = (volatile uint8_t *)p;
    while (n--) *b++ = 0;
}

/* ------------------------------------------------------------------ */
/* Wykrywanie urzadzenia.                                             */
/* ------------------------------------------------------------------ */

static int vmx_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id0 = pci_read32((uint8_t)bus, slot, 0, 0x00);
            uint32_t b0, b1, cmd;

            if ((uint16_t)id0 != VMX_VENDOR_ID || (uint16_t)(id0 >> 16) != VMX_DEVICE_ID) continue;

            /* BAR0 (PT: doorbelle) i BAR1 (VD: rejestry), oba 32-bitowe pamieciowe. */
            b0 = pci_read32((uint8_t)bus, slot, 0, 0x10);
            b1 = pci_read32((uint8_t)bus, slot, 0, 0x14);
            if ((b0 & 1U) || (b1 & 1U)) continue;
            if (((b0 & 6U) == 4U) || ((b1 & 6U) == 4U)) continue;
            if ((b0 & 0xFFFFFFF0U) == 0 || (b1 & 0xFFFFFFF0U) == 0) continue;

            /* Pamiec + bus master + wylaczone INTx (polling). Tylko dolne 16 bitow. */
            cmd = pci_read32((uint8_t)bus, slot, 0, 0x04) & 0xFFFFU;
            pci_write32((uint8_t)bus, slot, 0, 0x04, cmd | 0x0406U);

            vmx_bar0 = (volatile uint8_t *)(unsigned long)(b0 & 0xFFFFFFF0U);
            vmx_bar1 = (volatile uint8_t *)(unsigned long)(b1 & 0xFFFFFFF0U);
            return 1;
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Pierscienie.                                                       */
/* ------------------------------------------------------------------ */

/* Wystaw deskryptor RX na pozycji next2fill pierscienia 'ring' (0 = head, 1 = body). */
static void vmx_rx_post(uint32_t ring)
{
    uint32_t i = vmx_rx_fill[ring];
    volatile struct vmx_rxdesc *d;
    uint8_t *buf;
    uint32_t size;

    if (ring == 0) { d = &vmx_rx0_ring[i]; buf = &vmx_rx0_buf[i][0]; size = VMX_RX0_SIZE; }
    else           { d = &vmx_rx1_ring[i]; buf = &vmx_rx1_buf[i][0]; size = VMX_RX1_SIZE; }

    d->addr = phys(buf);
    d->dw3 = 0;
    barrier();
    /* gen ustawiany na koncu - dopiero wtedy urzadzenie widzi deskryptor. */
    d->dw2 = (vmx_rx_gen[ring] << VMX_RXD_GEN_SHIFT) |
             (ring << VMX_RXD_BTYPE_SHIFT) | VMX_BUF_SIZE;
    barrier();

    if (++vmx_rx_fill[ring] == size) {
        vmx_rx_fill[ring] = 0;
        vmx_rx_gen[ring] ^= 1U;
    }
}

static void vmx_rx_doorbell(uint32_t ring)
{
    bar0_write(ring == 0 ? VMX_REG_RXPROD : VMX_REG_RXPROD2, vmx_rx_fill[ring]);
}

static void vmx_setup_rings(void)
{
    uint32_t i;

    vmx_zero(vmx_tx_ring, sizeof(vmx_tx_ring));
    vmx_zero(vmx_tx_comp, sizeof(vmx_tx_comp));
    vmx_zero(vmx_rx0_ring, sizeof(vmx_rx0_ring));
    vmx_zero(vmx_rx1_ring, sizeof(vmx_rx1_ring));
    vmx_zero(vmx_rx_comp, sizeof(vmx_rx_comp));

    vmx_tx_fill = 0; vmx_tx_gen = 1; vmx_tx_inflight = 0;
    vmx_txc_next = 0; vmx_txc_gen = 1;
    vmx_rx_fill[0] = vmx_rx_fill[1] = 0;
    vmx_rx_gen[0] = vmx_rx_gen[1] = 1;
    vmx_rxc_next = 0; vmx_rxc_gen = 1;

    /* Wypelnij size-1 deskryptorow (jedna pozycja zostaje pusta). */
    for (i = 0; i < VMX_RX0_SIZE - 1; i++) vmx_rx_post(0);
    for (i = 0; i < VMX_RX1_SIZE - 1; i++) vmx_rx_post(1);
}

static void vmx_setup_shared(void)
{
    struct vmx_driver_shared *s = &vmx_shared;
    struct vmx_ds_dev_read *dr;

    vmx_zero(&vmx_shared, sizeof(vmx_shared));
    vmx_zero(&vmx_qd, sizeof(vmx_qd));

    s->magic = VMX_REV1_MAGIC;
    dr = &s->dev_read;

    dr->misc.driver_info.version = 0x01000000U;
    dr->misc.driver_info.gos = (sizeof(void *) == 4U) ? 1U : 2U;   /* gosBits; typ nieznany */
    dr->misc.driver_info.rev_spt = 1;
    dr->misc.driver_info.upt_ver_spt = 1;
    dr->misc.upt_features = 0;                 /* bez offloadow */
    dr->misc.dd_pa = 0;
    dr->misc.dd_len = 0;
    dr->misc.queue_desc_pa = phys(&vmx_qd);
    dr->misc.queue_desc_len = (uint32_t)sizeof(vmx_qd);
    dr->misc.mtu = 1500;
    dr->misc.max_num_rx_sg = 1;
    dr->misc.num_tx_queues = 1;
    dr->misc.num_rx_queues = 1;

    /* Przerwania wylaczone - polling. */
    dr->intr_conf.auto_mask = 0;
    dr->intr_conf.num_intrs = 1;
    dr->intr_conf.event_intr_idx = 0;
    dr->intr_conf.mod_levels[0] = 0;
    dr->intr_conf.intr_ctrl = VMX_IC_DISABLE_ALL;

    /* Unicast + broadcast + multicast (bez tablicy filtrow). */
    dr->rx_filter_conf.rx_mode = VMX_RXM_UCAST | VMX_RXM_BCAST | VMX_RXM_ALL_MULTI;

    /* Kolejka TX. */
    vmx_qd.tx.conf.tx_ring_base_pa   = phys(vmx_tx_ring);
    vmx_qd.tx.conf.data_ring_base_pa = 0;
    vmx_qd.tx.conf.comp_ring_base_pa = phys(vmx_tx_comp);
    vmx_qd.tx.conf.tx_ring_size      = VMX_TX_SIZE;
    vmx_qd.tx.conf.data_ring_size    = 0;
    vmx_qd.tx.conf.comp_ring_size    = VMX_TXC_SIZE;
    vmx_qd.tx.conf.intr_idx          = 0;

    /* Kolejka RX (dwa pierscienie, wspolny pierscien uzupelnien). */
    vmx_qd.rx.conf.rx_ring_base_pa[0] = phys(vmx_rx0_ring);
    vmx_qd.rx.conf.rx_ring_base_pa[1] = phys(vmx_rx1_ring);
    vmx_qd.rx.conf.comp_ring_base_pa  = phys(vmx_rx_comp);
    vmx_qd.rx.conf.rx_ring_size[0]    = VMX_RX0_SIZE;
    vmx_qd.rx.conf.rx_ring_size[1]    = VMX_RX1_SIZE;
    vmx_qd.rx.conf.comp_ring_size     = VMX_RXC_SIZE;
    vmx_qd.rx.conf.intr_idx           = 0;

    barrier();
}

static int vmx_link_query(void)
{
    uint32_t r = vmx_cmd(VMX_CMD_GET_LINK);
    return (r & 1U) ? 1 : 0;
}

/* Gdy linku nie ma - sprawdzaj czesciej, gdy jest - rzadziej. */
static void vmx_link_poll(void)
{
    static uint32_t count = 0;
    uint32_t mask = vmx_link ? 0xFFFU : 0x3FFU;

    if ((++count & mask) != 0) return;
    vmx_link = (uint8_t)vmx_link_query();
}

/* Zwolnij zakonczone wysylki (pierscien uzupelnien TX). */
static void vmx_tx_reap(void)
{
    uint32_t n;
    for (n = 0; n < VMX_TXC_SIZE; n++) {
        volatile struct vmx_txcomp *c = &vmx_tx_comp[vmx_txc_next];
        barrier();
        if (((c->dw3 >> VMX_COMP_GEN_SHIFT) & 1U) != vmx_txc_gen) break;
        barrier();
        if (vmx_tx_inflight) vmx_tx_inflight--;
        if (++vmx_txc_next == VMX_TXC_SIZE) {
            vmx_txc_next = 0;
            vmx_txc_gen ^= 1U;
        }
    }
}

/* ------------------------------------------------------------------ */
/* Interfejs sterownika.                                              */
/* ------------------------------------------------------------------ */

int vmxnet3_init(void)
{
    uint32_t ver, lo, hi;
    uint64_t pa;

    if (vmx_ready) return 1;
    if (!vmx_find_device()) return 0;

    (void)vmx_cmd(VMX_CMD_RESET_DEV);

    /* Negocjacja wersji: urzadzenie musi wspierac rewizje 1 (bit 0). */
    ver = bar1_read(VMX_REG_VRRS);
    if (!(ver & 1U)) return 0;
    bar1_write(VMX_REG_VRRS, 1U);
    ver = bar1_read(VMX_REG_UVRS);
    if (!(ver & 1U)) return 0;
    bar1_write(VMX_REG_UVRS, 1U);

    /* Adres MAC. */
    lo = bar1_read(VMX_REG_MACL);
    hi = bar1_read(VMX_REG_MACH);
    vmx_mac[0] = (uint8_t)lo;
    vmx_mac[1] = (uint8_t)(lo >> 8);
    vmx_mac[2] = (uint8_t)(lo >> 16);
    vmx_mac[3] = (uint8_t)(lo >> 24);
    vmx_mac[4] = (uint8_t)hi;
    vmx_mac[5] = (uint8_t)(hi >> 8);

    vmx_setup_rings();
    vmx_setup_shared();

    /* Przerwania zamaskowane (polling). */
    bar0_write(VMX_REG_IMR, 1U);

    /* Wskaz urzadzeniu Driver Shared i aktywuj (wynik 0 = sukces). */
    pa = phys(&vmx_shared);
    bar1_write(VMX_REG_DSAL, (uint32_t)(pa & 0xFFFFFFFFULL));
    bar1_write(VMX_REG_DSAH, (uint32_t)(pa >> 32));
    if (vmx_cmd(VMX_CMD_ACTIVATE_DEV) != 0) {
        (void)vmx_cmd(VMX_CMD_RESET_DEV);
        return 0;
    }

    /* Oddaj urzadzeniu bufory RX. */
    vmx_rx_doorbell(0);
    vmx_rx_doorbell(1);

    (void)vmx_cmd(VMX_CMD_UPDATE_RX_MODE);

    vmx_link = (uint8_t)vmx_link_query();

    vmx_stats.tx_packets = vmx_stats.rx_packets = 0;
    vmx_stats.tx_errors = vmx_stats.rx_errors = 0;
    vmx_stats.tx_dropped = vmx_stats.rx_dropped = 0;

    vmx_ready = 1;
    return 1;
}

int vmxnet3_is_ready(void)
{
    return vmx_ready;
}

int vmxnet3_get_mac(uint8_t mac[6])
{
    if (!vmx_ready || !mac) return 0;
    for (uint32_t i = 0; i < 6; i++) mac[i] = vmx_mac[i];
    return 1;
}

int vmxnet3_send(const void *frame, uint16_t length)
{
    const uint8_t *src = (const uint8_t *)frame;
    volatile struct vmx_txdesc *d;
    uint32_t i, slot, send_len, tries;

    if (!vmx_ready || !frame || length == 0 || length > VMX_MAX_FRAME) return -1;

    vmx_link_poll();
    if (!vmx_link) { vmx_stats.tx_dropped++; return 0; }

    /* Poczekaj krotko na wolne miejsce (jedna pozycja pierscienia zostaje pusta). */
    vmx_tx_reap();
    for (tries = 0; vmx_tx_inflight >= VMX_TX_SIZE - 1U && tries < 200000U; tries++)
        vmx_tx_reap();
    if (vmx_tx_inflight >= VMX_TX_SIZE - 1U) {
        vmx_stats.tx_dropped++;
        return 0;
    }

    slot = vmx_tx_fill;
    for (i = 0; i < length; i++) vmx_tx_buf[slot][i] = src[i];
    send_len = length;
    while (send_len < VMX_MIN_FRAME) vmx_tx_buf[slot][send_len++] = 0;

    d = &vmx_tx_ring[slot];
    d->addr = phys(&vmx_tx_buf[slot][0]);
    d->dw3 = VMX_TXD_EOP | VMX_TXD_CQ;
    barrier();
    d->dw2 = (vmx_tx_gen << VMX_TXD_GEN_SHIFT) | send_len;   /* gen na koncu */
    barrier();

    if (++vmx_tx_fill == VMX_TX_SIZE) {
        vmx_tx_fill = 0;
        vmx_tx_gen ^= 1U;
    }
    vmx_tx_inflight++;

    bar0_write(VMX_REG_TXPROD, vmx_tx_fill);

    vmx_stats.tx_packets++;
    return 1;
}

int vmxnet3_receive(void *frame, uint16_t capacity)
{
    uint8_t *dst = (uint8_t *)frame;
    uint32_t n, j;

    if (!vmx_ready || !frame || capacity == 0) return -1;

    vmx_link_poll();

    for (n = 0; n < 64; n++) {
        volatile struct vmx_rxcomp *c = &vmx_rx_comp[vmx_rxc_next];
        uint32_t dw0, dw2, dw3, idx, ring, len, sop, eop, err, limit;
        uint8_t *src;

        barrier();
        dw3 = c->dw3;
        if (((dw3 >> VMX_COMP_GEN_SHIFT) & 1U) != vmx_rxc_gen) return 0;
        barrier();
        dw0 = c->dw0;
        dw2 = c->dw2;

        if (++vmx_rxc_next == VMX_RXC_SIZE) {
            vmx_rxc_next = 0;
            vmx_rxc_gen ^= 1U;
        }

        idx  = dw0 & 0xFFFU;
        ring = (((dw0 >> 16) & 0x3FFU) >= 1U) ? 1U : 0U;   /* rqID: 0 = ring 0, >=1 = ring 1 */
        eop  = (dw0 >> 14) & 1U;
        sop  = (dw0 >> 15) & 1U;
        len  = dw2 & 0x3FFFU;                               /* dlugosc bez CRC */
        err  = (dw2 >> 14) & 1U;
        limit = ring ? VMX_RX1_SIZE : VMX_RX0_SIZE;
        src  = (idx < limit) ? (ring ? &vmx_rx1_buf[idx][0] : &vmx_rx0_buf[idx][0]) : 0;

        if (!src || ring != 0 || !sop || !eop || err || len < 14U || len > VMX_MAX_FRAME) {
            vmx_stats.rx_errors++;
            vmx_rx_post(ring);
            vmx_rx_doorbell(ring);
            continue;
        }
        if (len > (uint32_t)capacity) {
            vmx_stats.rx_dropped++;
            vmx_rx_post(ring);
            vmx_rx_doorbell(ring);
            continue;
        }

        for (j = 0; j < len; j++) dst[j] = src[j];

        /* Dopiero po skopiowaniu oddaj bufor urzadzeniu. */
        vmx_rx_post(0);
        vmx_rx_doorbell(0);

        vmx_stats.rx_packets++;
        return (int)len;
    }
    return 0;
}

void vmxnet3_shutdown(void)
{
    if (!vmx_ready) return;
    (void)vmx_cmd(VMX_CMD_QUIESCE_DEV);
    (void)vmx_cmd(VMX_CMD_RESET_DEV);
    bar0_write(VMX_REG_IMR, 1U);
    vmx_ready = 0;
    vmx_link = 0;
}

int vmxnet3_stats_get(uint32_t *tx_packets, uint32_t *rx_packets,
                      uint32_t *tx_errors, uint32_t *rx_errors,
                      uint32_t *tx_dropped, uint32_t *rx_dropped)
{
    if (!tx_packets || !rx_packets || !tx_errors || !rx_errors) return 0;
    *tx_packets = vmx_stats.tx_packets;
    *rx_packets = vmx_stats.rx_packets;
    *tx_errors = vmx_stats.tx_errors;
    *rx_errors = vmx_stats.rx_errors;
    if (tx_dropped) *tx_dropped = vmx_stats.tx_dropped;
    if (rx_dropped) *rx_dropped = vmx_stats.rx_dropped;
    return 1;
}

void vmxnet3_stats_reset(void)
{
    vmx_stats.tx_packets = 0;
    vmx_stats.rx_packets = 0;
    vmx_stats.tx_errors = 0;
    vmx_stats.rx_errors = 0;
    vmx_stats.tx_dropped = 0;
    vmx_stats.rx_dropped = 0;
}

/* Urzadzenie zostaje skonfigurowane na MTU 1500; programowe MTU moze byc mniejsze. */
int vmxnet3_set_mtu(uint16_t mtu)
{
    if (!vmx_ready || mtu < 68U || mtu > 1500U) return 0;
    vmx_mtu = mtu;
    return 1;
}

uint16_t vmxnet3_get_mtu(void)
{
    return vmx_mtu;
}

int vmxnet3_link_up(void)
{
    return vmx_link ? 1 : 0;
}