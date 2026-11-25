/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/storage/nvme.h>

#define NVME_QMAX         64U
#define NVME_BOUNCE_SIZE  16384U
#define NVME_PAGE         4096U

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

static uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t function, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

static void pci_write_config_dword(uint8_t bus, uint8_t slot, uint8_t function,
                                   uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)function << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    outl(0xCFC, value);
}

/* ------------------------------------------------------------------ */
/* Czas: TSC skalibrowany wzgledem PIT kanal 2                        */
/* ------------------------------------------------------------------ */

static uint32_t nv_tsc_per_us;

static uint64_t nv_rdtsc(void)
{
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

static void nv_calibrate_tsc(void)
{
    uint8_t saved;
    uint64_t t0, t1;
    uint32_t n, delta, per_us;

    if (nv_tsc_per_us) return;

    saved = inb(0x61);
    outb(0x61, (uint8_t)((saved & 0xFC) | 0x01));
    outb(0x43, 0xB0);
    outb(0x42, (uint8_t)(11932 & 0xFF));
    outb(0x42, (uint8_t)(11932 >> 8));
    t0 = nv_rdtsc();
    for (n = 0; n < 20000000U; n++) {
        if (inb(0x61) & 0x20) break;
    }
    t1 = nv_rdtsc();
    outb(0x61, saved);

    if (n == 20000000U) return;
    delta = (uint32_t)(t1 - t0);
    per_us = delta / 10000U;
    if (per_us < 100U || per_us > 10000U) return;
    nv_tsc_per_us = per_us;
}

static void nv_delay_us(uint32_t us)
{
    if (nv_tsc_per_us) {
        uint64_t end = nv_rdtsc() + (uint64_t)us * nv_tsc_per_us;
        while (nv_rdtsc() < end) __asm__ volatile("pause");
        return;
    }
    while (us--) outb(0x80, 0);
}

static void nvme_barrier(void)
{
    __asm__ volatile("" : : : "memory");
}

static void nvme_zero(void *p, uint32_t n)
{
    volatile uint8_t *b = (volatile uint8_t *)p;
    while (n--) *b++ = 0;
}

static uint64_t nvme_addr(const void *p)
{
    return (uint64_t)(unsigned long)p;
}

/* ------------------------------------------------------------------ */
/* Rejestry                                                           */
/* ------------------------------------------------------------------ */

static volatile uint32_t *nvme_regs;

static inline uint32_t nvme_read_reg(uint32_t offset)
{
    return nvme_regs[offset >> 2];
}

static inline void nvme_write_reg(uint32_t offset, uint32_t value)
{
    nvme_regs[offset >> 2] = value;
}

static inline uint64_t nvme_read_reg64(uint32_t offset)
{
    uint64_t lo = nvme_regs[offset >> 2];
    uint64_t hi = nvme_regs[(offset >> 2) + 1];
    return lo | (hi << 32);
}

static inline void nvme_write_reg64(uint32_t offset, uint64_t value)
{
    nvme_regs[offset >> 2] = (uint32_t)value;
    nvme_regs[(offset >> 2) + 1] = (uint32_t)(value >> 32);
}

#define NVME_REG_CAP      0x0000
#define NVME_REG_VS       0x0008
#define NVME_REG_INTMS    0x000C
#define NVME_REG_INTMC    0x0010
#define NVME_REG_CC       0x0014
#define NVME_REG_CSTS     0x001C
#define NVME_REG_NSSR     0x0020
#define NVME_REG_AQA      0x0024
#define NVME_REG_ASQ      0x0028
#define NVME_REG_ACQ      0x0030
#define NVME_REG_CMBLOC   0x0038
#define NVME_REG_CMBSZ    0x003C
#define NVME_DB_BASE      0x1000

#define NVME_CC_EN            (1U << 0)
#define NVME_CC_IOSQES_SHIFT  16
#define NVME_CC_IOCQES_SHIFT  20

#define NVME_CSTS_RDY       (1U << 0)
#define NVME_CSTS_CFS       (1U << 1)

/* Komendy administracyjne. */
#define NVME_OP_DELETE_SQ      0x00
#define NVME_OP_CREATE_SQ      0x01
#define NVME_OP_GET_LOG_PAGE   0x02
#define NVME_OP_DELETE_CQ      0x04
#define NVME_OP_CREATE_CQ      0x05
#define NVME_OP_IDENTIFY       0x06
#define NVME_OP_ABORT          0x08
#define NVME_OP_SET_FEATURES   0x09
#define NVME_OP_GET_FEATURES   0x0A
#define NVME_OP_ASYNC_EVENT    0x0C
/* Komendy NVM. */
#define NVME_OP_FLUSH          0x00
#define NVME_OP_WRITE          0x01
#define NVME_OP_READ           0x02
#define NVME_OP_COMPARE        0x05
#define NVME_OP_WRITE_ZEROES   0x08
#define NVME_OP_DATASET_MGMT   0x09

#define NVME_CNS_IDENTIFY_NS    0x00
#define NVME_CNS_IDENTIFY_CTRL  0x01

#define NVME_FEAT_NUM_QUEUES    0x07

/* Wpis kolejki zlecen (64 B): DW0 opcode/CID, DW1 NSID, DW4-5 MPTR, DW6-7 PRP1, DW8-9 PRP2, DW10-15. */
struct nvme_sq_entry {
    uint32_t cdw0;
    uint32_t nsid;
    uint32_t cdw2;
    uint32_t cdw3;
    uint64_t mptr;
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cdw10;
    uint32_t cdw11;
    uint32_t cdw12;
    uint32_t cdw13;
    uint32_t cdw14;
    uint32_t cdw15;
} __attribute__((packed));

/* Wpis kolejki uzupelnien (16 B): DW0 wynik, DW2 SQHD/SQID, DW3 CID + status (bit 0 = faza). */
struct nvme_cq_entry {
    uint32_t dw0;
    uint32_t dw1;
    uint16_t sqhd;
    uint16_t sqid;
    uint16_t cid;
    uint16_t status;
} __attribute__((packed));

typedef char nvme_sq_entry_size_check[(sizeof(struct nvme_sq_entry) == 64) ? 1 : -1];
typedef char nvme_cq_entry_size_check[(sizeof(struct nvme_cq_entry) == 16) ? 1 : -1];

static struct nvme_sq_entry admin_sq[NVME_QMAX] __attribute__((aligned(4096)));
static struct nvme_cq_entry admin_cq[NVME_QMAX] __attribute__((aligned(4096)));
static struct nvme_sq_entry io_sq[NVME_QMAX] __attribute__((aligned(4096)));
static struct nvme_cq_entry io_cq[NVME_QMAX] __attribute__((aligned(4096)));
static uint64_t prp_list[512] __attribute__((aligned(4096)));
static uint8_t nvme_id_buf[4096] __attribute__((aligned(4096)));
static uint8_t nvme_bounce_buf[NVME_BOUNCE_SIZE] __attribute__((aligned(4096)));

struct nvme_queue {
    struct nvme_sq_entry *sq;
    volatile struct nvme_cq_entry *cq;
    uint16_t qid;
    uint16_t sq_tail;
    uint16_t cq_head;
    uint16_t cid;
    uint8_t  phase;
};

static struct nvme_queue adminq;
static struct nvme_queue ioq;

static uint32_t nvme_qd;            /* glebokosc kolejek (liczba wpisow) */
static uint32_t nvme_db_stride;     /* bajty miedzy doorbellami (4 << DSTRD) */
static uint32_t nvme_max_xfer = NVME_BOUNCE_SIZE;
static uint32_t nvme_ns_total;      /* liczba NSID z Identify Controller (NN, max 256) */
static uint8_t  nvme_ready;

static uint64_t ns_capacity[256];
static uint32_t ns_sector_size[256];
static uint32_t ns_count;

static uint32_t nvme_db_offset(uint32_t qid, int cq)
{
    return NVME_DB_BASE + (2U * qid + (cq ? 1U : 0U)) * nvme_db_stride;
}

/* ------------------------------------------------------------------ */
/* PCI                                                                */
/* ------------------------------------------------------------------ */

static int nvme_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id0 = pci_read_config_dword((uint8_t)bus, slot, 0, 0x00);
            uint32_t hdr;
            uint8_t nfunc, func;

            if ((uint16_t)id0 == 0xFFFF) continue;
            hdr = (pci_read_config_dword((uint8_t)bus, slot, 0, 0x0C) >> 16) & 0xFF;
            nfunc = (hdr & 0x80) ? 8 : 1;

            for (func = 0; func < nfunc; func++) {
                uint32_t id = pci_read_config_dword((uint8_t)bus, slot, func, 0x00);
                uint32_t class_code, bar0, bar_hi, command;
                uint8_t base_class, sub_class, prog_if;

                if ((uint16_t)id == 0xFFFF) continue;
                class_code = pci_read_config_dword((uint8_t)bus, slot, func, 0x08);
                base_class = (uint8_t)(class_code >> 24);
                sub_class = (uint8_t)((class_code >> 16) & 0xFF);
                prog_if = (uint8_t)((class_code >> 8) & 0xFF);
                if (base_class != 0x01 || sub_class != 0x08 || prog_if != 0x02) continue;

                /* BAR0 (0x10) - rejestry, 64-bitowy. */
                bar0 = pci_read_config_dword((uint8_t)bus, slot, func, 0x10);
                if (bar0 & 1U) continue;
                if ((bar0 & 6U) == 4U) {
                    bar_hi = pci_read_config_dword((uint8_t)bus, slot, func, 0x14);
                    if (bar_hi != 0) continue;     /* powyzej 4 GiB - nieosiagalne */
                }
                if ((bar0 & 0xFFFFFFF0U) == 0) continue;

                /* Pamiec + bus master + wylaczone INTx; tylko dolne 16 bitow (status = W1C). */
                command = pci_read_config_dword((uint8_t)bus, slot, func, 0x04) & 0xFFFFU;
                pci_write_config_dword((uint8_t)bus, slot, func, 0x04, command | 0x0406U);

                nvme_regs = (volatile uint32_t *)(unsigned long)(bar0 & 0xFFFFFFF0U);
                return 1;
            }
        }
    }
    return 0;
}

/* ------------------------------------------------------------------ */
/* Kolejki                                                            */
/* ------------------------------------------------------------------ */

/* 0 = sukces, -1 = blad statusu, -2 = timeout. */
static int nvme_submit(struct nvme_queue *q, struct nvme_sq_entry *cmd, uint32_t timeout_ms)
{
    uint16_t cid, next, st, ecid;
    uint32_t n, limit;
    volatile struct nvme_cq_entry *e;

    cid = (uint16_t)(q->cid + 1U);
    if (cid == 0) cid = 1;
    q->cid = cid;

    cmd->cdw0 = (cmd->cdw0 & 0xFFFFU) | ((uint32_t)cid << 16);
    q->sq[q->sq_tail] = *cmd;
    nvme_barrier();

    next = (uint16_t)(q->sq_tail + 1U);
    if (next >= nvme_qd) next = 0;
    q->sq_tail = next;
    nvme_write_reg(nvme_db_offset(q->qid, 0), next);

    limit = timeout_ms * 200U;
    for (n = 0; n < limit; n++) {
        e = &q->cq[q->cq_head];
        if (((uint32_t)e->status & 1U) == q->phase) {
            nvme_barrier();
            st = e->status;
            ecid = e->cid;

            q->cq_head = (uint16_t)(q->cq_head + 1U);
            if (q->cq_head >= nvme_qd) {
                q->cq_head = 0;
                q->phase ^= 1U;
            }
            nvme_write_reg(nvme_db_offset(q->qid, 1), q->cq_head);

            if (ecid != cid) return -1;
            if (((uint32_t)st >> 1) & 0x7FFU) return -1;   /* SC + SCT */
            return 0;
        }
        nv_delay_us(5);
    }
    return -2;
}

static int nvme_admin(struct nvme_sq_entry *cmd)
{
    return nvme_submit(&adminq, cmd, 5000);
}

static void nvme_cmd_init(struct nvme_sq_entry *cmd, uint8_t opcode, uint32_t nsid)
{
    nvme_zero(cmd, sizeof(*cmd));
    cmd->cdw0 = opcode;
    cmd->nsid = nsid;
}

/* Identify do wlasnego bufora (wyrownanego do 4 KB). */
static int nvme_identify_raw(uint8_t cns, uint32_t nsid)
{
    struct nvme_sq_entry cmd;
    nvme_zero(nvme_id_buf, sizeof(nvme_id_buf));
    nvme_cmd_init(&cmd, NVME_OP_IDENTIFY, nsid);
    cmd.prp1 = nvme_addr(nvme_id_buf);
    cmd.cdw10 = cns;
    return nvme_admin(&cmd);
}

static int nvme_create_io_queues(void)
{
    struct nvme_sq_entry cmd;

    /* Set Features: liczba kolejek (zadamy 1 + 1, wartosci 0-based). */
    nvme_cmd_init(&cmd, NVME_OP_SET_FEATURES, 0);
    cmd.cdw10 = NVME_FEAT_NUM_QUEUES;
    cmd.cdw11 = 0;
    (void)nvme_admin(&cmd);

    /* Create I/O CQ: QID 1, ciagla w pamieci, bez przerwan. */
    nvme_cmd_init(&cmd, NVME_OP_CREATE_CQ, 0);
    cmd.prp1 = nvme_addr(io_cq);
    cmd.cdw10 = ((nvme_qd - 1U) << 16) | 1U;
    cmd.cdw11 = 1U;                       /* PC */
    if (nvme_admin(&cmd) != 0) return -1;

    /* Create I/O SQ: QID 1, powiazana z CQ 1. */
    nvme_cmd_init(&cmd, NVME_OP_CREATE_SQ, 0);
    cmd.prp1 = nvme_addr(io_sq);
    cmd.cdw10 = ((nvme_qd - 1U) << 16) | 1U;
    cmd.cdw11 = (1U << 16) | 1U;          /* CQID = 1, PC */
    if (nvme_admin(&cmd) != 0) return -1;

    return 0;
}

static uint64_t le64(const uint8_t *p)
{
    return (uint64_t)p[0] | ((uint64_t)p[1] << 8) | ((uint64_t)p[2] << 16) | ((uint64_t)p[3] << 24) |
           ((uint64_t)p[4] << 32) | ((uint64_t)p[5] << 40) | ((uint64_t)p[6] << 48) | ((uint64_t)p[7] << 56);
}

static uint32_t le32(const uint8_t *p)
{
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

/* Identify Namespace: NSZE (0-7), NLBAF (25), FLBAS (26), LBAF[] od 128. */
static void nvme_parse_ns(uint32_t nsid, const uint8_t *b)
{
    uint64_t nsze = le64(b);
    uint8_t nlbaf = b[25];
    uint8_t flbas = b[26];
    uint32_t fmt = (uint32_t)(flbas & 0x0F) | ((uint32_t)((flbas >> 5) & 3) << 4);
    uint32_t lbaf, ms, lbads;

    ns_capacity[nsid - 1] = 0;
    ns_sector_size[nsid - 1] = 0;
    if (nsze == 0 || fmt > nlbaf) return;

    lbaf = le32(b + 128 + 4U * fmt);
    ms = lbaf & 0xFFFFU;
    lbads = (lbaf >> 16) & 0xFFU;

    if (lbads < 9 || lbads > 12) return;                 /* 512 B .. 4 KB */
    if ((flbas & 0x10) && ms != 0) return;               /* metadane w linii z danymi - nieobslugiwane */

    ns_capacity[nsid - 1] = nsze;
    ns_sector_size[nsid - 1] = 1U << lbads;
}

static int nvme_wait_rdy(int want, uint32_t timeout_ms, int check_cfs)
{
    uint32_t n, limit = timeout_ms * 10U;
    for (n = 0; n <= limit; n++) {
        uint32_t csts = nvme_read_reg(NVME_REG_CSTS);
        if (check_cfs && (csts & NVME_CSTS_CFS)) return 0;
        if (((csts & NVME_CSTS_RDY) != 0) == (want != 0)) return 1;
        nv_delay_us(100);
    }
    return 0;
}

int nvme_identify_controller(void *buffer, uint32_t buffer_size)
{
    uint32_t n;
    if (!nvme_ready || !buffer) return -1;
    if (nvme_identify_raw(NVME_CNS_IDENTIFY_CTRL, 0) != 0) return -1;
    n = buffer_size < 4096U ? buffer_size : 4096U;
    for (uint32_t i = 0; i < n; i++) ((uint8_t *)buffer)[i] = nvme_id_buf[i];
    return 0;
}

int nvme_identify_namespace(uint32_t nsid, void *buffer, uint32_t buffer_size)
{
    uint32_t n;
    if (!nvme_ready || !buffer) return -1;
    if (nvme_identify_raw(NVME_CNS_IDENTIFY_NS, nsid) != 0) return -1;
    n = buffer_size < 4096U ? buffer_size : 4096U;
    for (uint32_t i = 0; i < n; i++) ((uint8_t *)buffer)[i] = nvme_id_buf[i];
    return 0;
}

int nvme_init(void)
{
    uint64_t cap;
    uint32_t mqes, to_ms, cc, i, nn, mdts;

    if (nvme_ready) return 0;

    nv_calibrate_tsc();
    if (!nvme_find_device()) return -1;

    cap = nvme_read_reg64(NVME_REG_CAP);
    mqes = (uint32_t)(cap & 0xFFFFU);                     /* 0-based */
    nvme_db_stride = 4U << (uint32_t)((cap >> 32) & 0xFU);
    to_ms = ((uint32_t)((cap >> 24) & 0xFFU) + 1U) * 500U;
    if (((cap >> 48) & 0xFU) != 0) return -1;             /* minimalna strona > 4 KB */
    if (!((cap >> 37) & 1U)) return -1;                   /* brak zestawu komend NVM */

    nvme_qd = mqes + 1U;
    if (nvme_qd > NVME_QMAX) nvme_qd = NVME_QMAX;
    if (nvme_qd < 2U) return -1;

    /* Przerwania zamaskowane (polling). */
    nvme_write_reg(NVME_REG_INTMS, 0xFFFFFFFFU);

    /* Wylacz kontroler i poczekaj na RDY = 0. */
    cc = nvme_read_reg(NVME_REG_CC);
    nvme_write_reg(NVME_REG_CC, cc & ~NVME_CC_EN);
    if (!nvme_wait_rdy(0, to_ms, 0)) return -1;

    /* Kolejki zerowane - bit fazy musi startowac od 0. */
    nvme_zero(admin_sq, sizeof(admin_sq));
    nvme_zero(admin_cq, sizeof(admin_cq));
    nvme_zero(io_sq, sizeof(io_sq));
    nvme_zero(io_cq, sizeof(io_cq));

    adminq.sq = admin_sq; adminq.cq = admin_cq; adminq.qid = 0;
    adminq.sq_tail = 0; adminq.cq_head = 0; adminq.cid = 0; adminq.phase = 1;
    ioq.sq = io_sq; ioq.cq = io_cq; ioq.qid = 1;
    ioq.sq_tail = 0; ioq.cq_head = 0; ioq.cid = 0; ioq.phase = 1;

    nvme_write_reg(NVME_REG_AQA, ((nvme_qd - 1U) << 16) | (nvme_qd - 1U));
    nvme_write_reg64(NVME_REG_ASQ, nvme_addr(admin_sq));
    nvme_write_reg64(NVME_REG_ACQ, nvme_addr(admin_cq));

    /* CSS = 0 (NVM), MPS = 0 (4 KB), AMS = 0, rozmiary wpisow 64 B / 16 B. */
    nvme_write_reg(NVME_REG_CC, NVME_CC_EN |
                                (6U << NVME_CC_IOSQES_SHIFT) |
                                (4U << NVME_CC_IOCQES_SHIFT));
    if (!nvme_wait_rdy(1, to_ms, 1)) return -1;

    /* Identify Controller: NN (516), MDTS (77). */
    if (nvme_identify_raw(NVME_CNS_IDENTIFY_CTRL, 0) != 0) return -1;
    nn = le32(nvme_id_buf + 516);
    mdts = nvme_id_buf[77];
    nvme_ns_total = nn > 256U ? 256U : nn;

    nvme_max_xfer = NVME_BOUNCE_SIZE;
    if (mdts != 0 && mdts < 3) {
        uint32_t lim = NVME_PAGE << mdts;
        if (lim < nvme_max_xfer) nvme_max_xfer = lim;
    }

    if (nvme_create_io_queues() != 0) return -1;

    /* Przestrzenie nazw: sektor i pojemnosc z LBAF (nieaktywne - 0). */
    for (i = 0; i < 256U; i++) { ns_capacity[i] = 0; ns_sector_size[i] = 0; }
    ns_count = 0;
    for (i = 1; i <= nvme_ns_total; i++) {
        int rc = nvme_identify_raw(NVME_CNS_IDENTIFY_NS, i);
        if (rc == -2) break;
        if (rc == 0) nvme_parse_ns(i, nvme_id_buf);
        ns_count = i;
    }

    nvme_ready = 1;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Odczyt / zapis                                                     */
/* ------------------------------------------------------------------ */

static void nvme_setup_prps(struct nvme_sq_entry *cmd, uint32_t bytes)
{
    uint32_t pages = (bytes + NVME_PAGE - 1U) / NVME_PAGE;
    uint32_t i;

    cmd->prp1 = nvme_addr(&nvme_bounce_buf[0]);
    if (pages <= 1U) {
        cmd->prp2 = 0;
    } else if (pages == 2U) {
        cmd->prp2 = nvme_addr(&nvme_bounce_buf[NVME_PAGE]);
    } else {
        for (i = 1; i < pages; i++)
            prp_list[i - 1U] = nvme_addr(&nvme_bounce_buf[i * NVME_PAGE]);
        cmd->prp2 = nvme_addr(&prp_list[0]);
    }
}

static int nvme_rw(int write, uint32_t nsid, uint64_t lba, uint32_t sector_count,
                   void *rbuf, const void *wbuf)
{
    uint32_t sector_size, max_chunk, total_bytes, offset = 0;
    uint64_t capacity;

    if (!nvme_ready || nsid == 0 || nsid > ns_count || sector_count == 0) return -1;
    if (write ? (wbuf == 0) : (rbuf == 0)) return -1;

    sector_size = ns_sector_size[nsid - 1];
    capacity = ns_capacity[nsid - 1];
    if (sector_size == 0) return -1;
    if (lba >= capacity || (uint64_t)sector_count > capacity - lba) return -1;
    if (sector_count > 0xFFFFFFFFU / sector_size) return -1;

    max_chunk = (nvme_max_xfer / sector_size) * sector_size;
    if (max_chunk == 0) return -1;
    total_bytes = sector_count * sector_size;

    while (offset < total_bytes) {
        uint32_t chunk_bytes = total_bytes - offset;
        uint32_t chunk_sectors, i;
        uint64_t chunk_lba;
        struct nvme_sq_entry cmd;

        if (chunk_bytes > max_chunk) chunk_bytes = max_chunk;
        chunk_sectors = chunk_bytes / sector_size;
        chunk_lba = lba + (offset / sector_size);

        if (write) {
            for (i = 0; i < chunk_bytes; i++)
                nvme_bounce_buf[i] = ((const uint8_t *)wbuf)[offset + i];
        }

        nvme_cmd_init(&cmd, write ? NVME_OP_WRITE : NVME_OP_READ, nsid);
        cmd.cdw10 = (uint32_t)chunk_lba;
        cmd.cdw11 = (uint32_t)(chunk_lba >> 32);
        cmd.cdw12 = (chunk_sectors - 1U) & 0xFFFFU;
        nvme_setup_prps(&cmd, chunk_bytes);

        if (nvme_submit(&ioq, &cmd, 10000) != 0) return -1;

        if (!write) {
            for (i = 0; i < chunk_bytes; i++)
                ((uint8_t *)rbuf)[offset + i] = nvme_bounce_buf[i];
        }
        offset += chunk_bytes;
    }
    return 0;
}

int nvme_read(uint32_t nsid, uint64_t lba, uint32_t sector_count, void *buffer)
{
    return nvme_rw(0, nsid, lba, sector_count, buffer, 0);
}

int nvme_write(uint32_t nsid, uint64_t lba, uint32_t sector_count, const void *buffer)
{
    return nvme_rw(1, nsid, lba, sector_count, 0, buffer);
}

int nvme_flush(uint32_t nsid)
{
    struct nvme_sq_entry cmd;

    if (!nvme_ready || nsid == 0 || nsid > ns_count) return -1;
    nvme_cmd_init(&cmd, NVME_OP_FLUSH, nsid);
    return nvme_submit(&ioq, &cmd, 10000) == 0 ? 0 : -1;
}

uint64_t nvme_get_capacity(uint32_t nsid)
{
    if (nsid == 0 || nsid > ns_count) return 0;
    return ns_capacity[nsid - 1];
}

uint32_t nvme_get_sector_size(uint32_t nsid)
{
    if (nsid == 0 || nsid > ns_count) return 0;
    return ns_sector_size[nsid - 1];
}