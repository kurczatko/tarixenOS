#include "nvme.h"

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
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
    return (uint64_t)nvme_regs[offset >> 2] |
           ((uint64_t)nvme_regs[(offset >> 2) + 1] << 32);
}

static inline void nvme_write_reg64(uint32_t offset, uint64_t value)
{
    nvme_regs[offset >> 2] = (uint32_t)value;
    nvme_regs[(offset >> 2) + 1] = (uint32_t)(value >> 32);
}

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
#define NVME_OP_FLUSH          0x02
#define NVME_OP_READ           0x05
#define NVME_OP_WRITE          0x08
#define NVME_OP_DATASET_MGMT   0x0A
#define NVME_OP_COMPARE        0x0C
#define NVME_OP_WRITE_ZEROES   0x0E

#define NVME_CNS_IDENTIFY_CTRL  0x01
#define NVME_CNS_IDENTIFY_NS    0x02

#define NVME_REG_CAP      0x0000
#define NVME_REG_VS       0x0008
#define NVME_REG_CC       0x0014
#define NVME_REG_CSTS     0x001C
#define NVME_REG_NSSR     0x0040
#define NVME_REG_AQA      0x0024
#define NVME_REG_ASQ      0x0028
#define NVME_REG_ACQ      0x0030
#define NVME_REG_CMBLOC   0x0038
#define NVME_REG_CMBSZ    0x003C

#define NVME_DB_SQ0_OFFSET  0x1000
#define NVME_DB_CQ0_OFFSET  0x1004

#define NVME_CC_EN          (1U << 0)
#define NVME_CC_IOSQES_SHIFT  12
#define NVME_CC_IOCQES_SHIFT  16

#define NVME_CSTS_RDY       (1U << 0)
#define NVME_CSTS_CFS       (1U << 1)

struct nvme_sq_entry {
    uint32_t cdw0;
    uint32_t nsid;
    uint64_t rsvd2;
    uint64_t prp1;
    uint64_t prp2;
    uint32_t cdw10;
    uint32_t cdw11;
    uint32_t cdw12;
    uint32_t cdw13;
    uint32_t cdw14;
    uint32_t cdw15;
    uint32_t rsvd16;
    uint32_t rsvd17;
} __attribute__((packed));

struct nvme_cq_entry {
    uint32_t sqid;
    uint16_t sqhd;
    uint16_t rsvd1;
    uint16_t cid;
    uint16_t rsvd2;
    uint16_t status;
    uint16_t rsvd3;
} __attribute__((packed));

typedef char nvme_sq_entry_size_check[(sizeof(struct nvme_sq_entry) == 64) ? 1 : -1];
typedef char nvme_cq_entry_size_check[(sizeof(struct nvme_cq_entry) == 16) ? 1 : -1];

static struct nvme_sq_entry admin_sq[64] __attribute__((aligned(4096)));
static struct nvme_cq_entry admin_cq[64] __attribute__((aligned(4096)));
static struct nvme_sq_entry io_sq[64] __attribute__((aligned(4096)));
static struct nvme_cq_entry io_cq[64] __attribute__((aligned(4096)));
static uint64_t prp_list[512] __attribute__((aligned(4096)));
static uint8_t identify_ctrl_buf[4096] __attribute__((aligned(4096)));
static uint8_t identify_ns_buf[4096] __attribute__((aligned(4096)));
static uint8_t nvme_bounce_buf[16384] __attribute__((aligned(4096)));

static uint16_t admin_sq_head;
static uint16_t admin_sq_tail;
static uint16_t admin_cq_head;
static uint8_t admin_cq_phase;

static uint16_t io_sq_head;
static uint16_t io_sq_tail;
static uint16_t io_cq_head;
static uint8_t io_cq_phase;

static uint8_t nvme_pci_bus;
static uint8_t nvme_pci_slot;
static uint8_t nvme_pci_func;
static uint32_t nvme_pci_bar;
static uint32_t nvme_cap_reg;
static uint32_t nvme_doorbell_stride;

static uint64_t ns_capacity[256];
static uint32_t ns_sector_size[256];
static uint32_t ns_count;

static int nvme_find_device(void)
{
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            uint32_t id0 = pci_read_config_dword((uint8_t)bus, slot, 0, 0x00);
            uint16_t vendor = (uint16_t)id0;
            if (vendor == 0xFFFF) continue;

            uint32_t class_code = pci_read_config_dword((uint8_t)bus, slot, 0, 0x08);
            uint8_t base_class = (uint8_t)(class_code >> 24);
            uint8_t sub_class = (uint8_t)((class_code >> 16) & 0xFF);

            if (base_class != 0x01 || sub_class != 0x08) continue;

            uint32_t bar = pci_read_config_dword((uint8_t)bus, slot, 0, 0x24);
            if (bar == 0 || bar == 0xFFFFFFFF) continue;

            nvme_pci_bus = (uint8_t)bus;
            nvme_pci_slot = slot;
            nvme_pci_func = 0;
            nvme_pci_bar = bar & 0xFFFFFFF0;

            uint32_t command = pci_read_config_dword((uint8_t)bus, slot, 0, 0x04);
            pci_write_config_dword((uint8_t)bus, slot, 0, 0x04, command | 0x00000006);

            nvme_regs = (volatile uint32_t *)(nvme_pci_bar);
            return 1;
        }
    }
    return 0;
}

static int nvme_wait_cq(struct nvme_cq_entry *cq, uint16_t *head, uint8_t *phase, uint16_t cid)
{
    uint32_t timeout = 0;
    while (1) {
        uint16_t cq_phase_bit = ((*phase) ? 1 : 0);
        uint16_t entry_status = cq[*head].status;
        uint16_t entry_phase = (uint16_t)((entry_status >> 15) & 1U);

        if (entry_phase == cq_phase_bit) {
            if (cq[*head].cid != cid) {
                return -1;
            }
            uint16_t sc = (uint16_t)((entry_status >> 1) & 0xFFF);
            uint16_t sct = (uint16_t)((entry_status >> 13) & 0x7);
            if (sc != 0 || sct != 0) {
                return -1;
            }
            *head = (uint16_t)((*head + 1) % 64);
            if (*head == 0) {
                *phase = (uint8_t)(*phase ^ 1);
            }
            return 0;
        }

        timeout++;
        if (timeout > 1000000) {
            return -1;
        }
    }
}

static int nvme_submit_admin_cmd(struct nvme_sq_entry *cmd, struct nvme_cq_entry *cqe)
{
    uint16_t cid = admin_sq_tail;
    admin_sq[admin_sq_tail] = *cmd;
    admin_sq[admin_sq_tail].cdw0 = (admin_sq[admin_sq_tail].cdw0 & 0xFFFF0000U) | (cid & 0xFFFFU);

    __asm__ volatile("" : : : "memory");

    admin_sq_tail = (uint16_t)((admin_sq_tail + 1) % 64);
    nvme_write_reg(NVME_DB_SQ0_OFFSET, admin_sq_tail);

    int ret = nvme_wait_cq(admin_cq, &admin_cq_head, &admin_cq_phase, cid);
    if (ret == 0 && cqe != 0) {
        *cqe = admin_cq[(admin_cq_head + 63) % 64];
    }
    return ret;
}

static int nvme_submit_io_cmd(struct nvme_sq_entry *cmd, struct nvme_cq_entry *cqe)
{
    uint16_t cid = io_sq_tail;
    io_sq[io_sq_tail] = *cmd;
    io_sq[io_sq_tail].cdw0 = (io_sq[io_sq_tail].cdw0 & 0xFFFF0000U) | (cid & 0xFFFFU);

    __asm__ volatile("" : : : "memory");

    io_sq_tail = (uint16_t)((io_sq_tail + 1) % 64);
    nvme_write_reg(NVME_DB_SQ0_OFFSET + 4, io_sq_tail);

    int ret = nvme_wait_cq(io_cq, &io_cq_head, &io_cq_phase, cid);
    if (ret == 0 && cqe != 0) {
        *cqe = io_cq[(io_cq_head + 63) % 64];
    }
    return ret;
}

static int nvme_identify_raw(uint8_t cns, uint32_t nsid, void *buffer)
{
    struct nvme_sq_entry cmd;
    struct nvme_cq_entry cqe;

    for (uint32_t i = 0; i < 64; i++) {
        ((uint32_t *)&cmd)[i] = 0;
    }

    cmd.cdw0 = (uint32_t)((0 << 16) | (0 << 8) | NVME_OP_IDENTIFY);
    cmd.nsid = nsid;
    cmd.cdw10 = cns;
    cmd.prp1 = (uint64_t)(uint32_t)buffer;

    return nvme_submit_admin_cmd(&cmd, &cqe);
}

int nvme_identify_controller(void *buffer, uint32_t buffer_size)
{
    (void)buffer_size;
    if (!buffer) return -1;
    return nvme_identify_raw(NVME_CNS_IDENTIFY_CTRL, 0, buffer);
}

int nvme_identify_namespace(uint32_t nsid, void *buffer, uint32_t buffer_size)
{
    (void)buffer_size;
    if (!buffer) return -1;
    return nvme_identify_raw(NVME_CNS_IDENTIFY_NS, nsid, buffer);
}

int nvme_init(void)
{
    (void)prp_list;
    if (!nvme_find_device()) return -1;

    nvme_cap_reg = nvme_read_reg(NVME_REG_CAP);
    nvme_doorbell_stride = (uint32_t)((nvme_cap_reg >> 25) & 0xFF);
    (void)nvme_doorbell_stride;

    uint32_t cc = nvme_read_reg(NVME_REG_CC);
    cc &= ~NVME_CC_EN;
    nvme_write_reg(NVME_REG_CC, cc);

    for (volatile uint32_t wait = 0; wait < 1000000; wait++);
    while ((nvme_read_reg(NVME_REG_CSTS) & NVME_CSTS_RDY) != 0) {
    }

    uint32_t mqes = (uint32_t)(nvme_cap_reg & 0xFFFF);
    uint16_t qsize = (uint16_t)(mqes > 63 ? 63 : mqes);

    uint32_t aqa = ((uint32_t)(qsize - 1) << 16) | ((uint32_t)(qsize - 1));
    nvme_write_reg(NVME_REG_AQA, aqa);

    nvme_write_reg64(NVME_REG_ASQ, (uint64_t)(uint32_t)&admin_sq[0]);
    nvme_write_reg64(NVME_REG_ACQ, (uint64_t)(uint32_t)&admin_cq[0]);

    admin_sq_head = 0;
    admin_sq_tail = 0;
    admin_cq_head = 0;
    admin_cq_phase = 1;

    cc = nvme_read_reg(NVME_REG_CC);
    cc &= ~(0xFU << NVME_CC_IOSQES_SHIFT);
    cc &= ~(0xFU << NVME_CC_IOCQES_SHIFT);
    cc |= (6U << NVME_CC_IOSQES_SHIFT);
    cc |= (4U << NVME_CC_IOCQES_SHIFT);
    cc |= NVME_CC_EN;
    nvme_write_reg(NVME_REG_CC, cc);

    for (volatile uint32_t wait = 0; wait < 1000000; wait++);
    while ((nvme_read_reg(NVME_REG_CSTS) & NVME_CSTS_RDY) == 0) {
    }

    uint32_t iosqes = (uint32_t)((cc >> NVME_CC_IOSQES_SHIFT) & 0xF);
    uint32_t iocqes = (uint32_t)((cc >> NVME_CC_IOCQES_SHIFT) & 0xF);
    (void)iosqes;
    (void)iocqes;

    nvme_write_reg64(NVME_REG_ASQ + 8, (uint64_t)(uint32_t)&io_sq[0]);
    nvme_write_reg64(NVME_REG_ACQ + 8, (uint64_t)(uint32_t)&io_cq[0]);

    io_sq_head = 0;
    io_sq_tail = 0;
    io_cq_head = 0;
    io_cq_phase = 1;

    if (nvme_identify_controller(identify_ctrl_buf, 4096) != 0) return -1;

    ns_count = 0;
    for (uint32_t i = 1; i <= 256; i++) {
        if (nvme_identify_namespace(i, identify_ns_buf, 4096) != 0) break;
        struct nvme_id_namespace *idns = (struct nvme_id_namespace *)identify_ns_buf;
        ns_capacity[ns_count] = idns->nsze;
        ns_sector_size[ns_count] = 512;
        ns_count++;
        if (ns_count >= 256) break;
    }

    return 0;
}

int nvme_read(uint32_t nsid, uint64_t lba, uint32_t sector_count, void *buffer)
{
    if (nsid == 0 || nsid > ns_count || buffer == 0 || sector_count == 0) return -1;

    uint32_t sector_size = ns_sector_size[nsid - 1];
    uint64_t capacity = ns_capacity[nsid - 1];
    if (lba + sector_count > capacity) return -1;

    uint32_t total_bytes = sector_count * sector_size;
    uint32_t offset = 0;

    while (offset < total_bytes) {
        uint32_t chunk_bytes = total_bytes - offset;
        if (chunk_bytes > 16384) chunk_bytes = 16384;
        uint32_t chunk_sectors = chunk_bytes / sector_size;
        uint64_t chunk_lba = lba + (offset / sector_size);

        struct nvme_sq_entry cmd;
        struct nvme_cq_entry cqe;

        for (uint32_t i = 0; i < 64; i++) {
            ((uint32_t *)&cmd)[i] = 0;
        }

        cmd.cdw0 = (uint32_t)((0 << 16) | (0 << 8) | NVME_OP_READ);
        cmd.nsid = nsid;
        cmd.cdw10 = (uint32_t)chunk_lba;
        cmd.cdw11 = (uint32_t)(chunk_lba >> 32);
        cmd.cdw12 = (uint32_t)(chunk_sectors - 1);
        cmd.prp1 = (uint64_t)(uint32_t)&nvme_bounce_buf[0];
        cmd.prp2 = 0;

        if (nvme_submit_io_cmd(&cmd, &cqe) != 0) return -1;

        uint32_t copy_bytes = chunk_bytes;
        if (copy_bytes > total_bytes - offset) copy_bytes = total_bytes - offset;
        for (uint32_t i = 0; i < copy_bytes; i++) {
            ((uint8_t *)buffer)[offset + i] = nvme_bounce_buf[i];
        }

        offset += chunk_bytes;
    }

    return 0;
}

int nvme_write(uint32_t nsid, uint64_t lba, uint32_t sector_count, const void *buffer)
{
    if (nsid == 0 || nsid > ns_count || buffer == 0 || sector_count == 0) return -1;

    uint32_t sector_size = ns_sector_size[nsid - 1];
    uint64_t capacity = ns_capacity[nsid - 1];
    if (lba + sector_count > capacity) return -1;

    uint32_t total_bytes = sector_count * sector_size;
    uint32_t offset = 0;

    while (offset < total_bytes) {
        uint32_t chunk_bytes = total_bytes - offset;
        if (chunk_bytes > 16384) chunk_bytes = 16384;
        uint32_t chunk_sectors = chunk_bytes / sector_size;
        uint64_t chunk_lba = lba + (offset / sector_size);

        for (uint32_t i = 0; i < chunk_bytes; i++) {
            nvme_bounce_buf[i] = ((const uint8_t *)buffer)[offset + i];
        }

        struct nvme_sq_entry cmd;
        struct nvme_cq_entry cqe;

        for (uint32_t i = 0; i < 64; i++) {
            ((uint32_t *)&cmd)[i] = 0;
        }

        cmd.cdw0 = (uint32_t)((0 << 16) | (0 << 8) | NVME_OP_WRITE);
        cmd.nsid = nsid;
        cmd.cdw10 = (uint32_t)chunk_lba;
        cmd.cdw11 = (uint32_t)(chunk_lba >> 32);
        cmd.cdw12 = (uint32_t)(chunk_sectors - 1);
        cmd.prp1 = (uint64_t)(uint32_t)&nvme_bounce_buf[0];
        cmd.prp2 = 0;

        if (nvme_submit_io_cmd(&cmd, &cqe) != 0) return -1;

        offset += chunk_bytes;
    }

    return 0;
}

int nvme_flush(uint32_t nsid)
{
    if (nsid == 0 || nsid > ns_count) return -1;

    struct nvme_sq_entry cmd;
    struct nvme_cq_entry cqe;

    for (uint32_t i = 0; i < 64; i++) {
        ((uint32_t *)&cmd)[i] = 0;
    }

    cmd.cdw0 = (uint32_t)((0 << 16) | (0 << 8) | NVME_OP_FLUSH);
    cmd.nsid = nsid;

    return nvme_submit_io_cmd(&cmd, &cqe);
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
