typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;
typedef signed char int8_t;
typedef signed short int16_t;
typedef signed int int32_t;
typedef unsigned int uintptr_t;

static void outl(uint16_t port, uint32_t val) {
    __asm__ volatile("outl %0, %1" : : "a"(val), "Nd"(port));
}

static uint32_t inl(uint16_t port) {
    uint32_t ret;
    __asm__ volatile("inl %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

static uint32_t pci_read_32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1U << 31) | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                       ((uint32_t)func << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    return inl(PCI_CONFIG_DATA);
}

static void pci_write_32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = (1U << 31) | ((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                       ((uint32_t)func << 8) | (offset & 0xFC);
    outl(PCI_CONFIG_ADDRESS, address);
    outl(PCI_CONFIG_DATA, value);
}

static void hda_pit_sleep_ms(uint32_t ms) {
    for (uint32_t i = 0; i < ms; i++) {
        for (volatile int j = 0; j < 10000; j++);
    }
}

#define HDA_GCAP     0x00
#define HDA_VMIN     0x02
#define HDA_VMAJ     0x03
#define HDA_OUTPAY   0x04
#define HDA_INPAY    0x06
#define HDA_GCTL     0x08
#define HDA_WAKEEN   0x0C
#define HDA_STATESTS 0x0E
#define HDA_GSTS     0x10
#define HDA_IC       0x60
#define HDA_IR       0x64
#define HDA_ICS      0x68

struct hda_bdl_entry {
    uint32_t address_low;
    uint32_t address_high;
    uint32_t length;
    uint32_t ioc;
} __attribute__((packed));

static volatile uint8_t *hda_base = 0;

static uint32_t hda_read32(uint32_t reg) {
    return *(volatile uint32_t *)(hda_base + reg);
}

static void hda_write32(uint32_t reg, uint32_t val) {
    *(volatile uint32_t *)(hda_base + reg) = val;
}

static void hda_write16(uint32_t reg, uint16_t val) {
    *(volatile uint16_t *)(hda_base + reg) = val;
}

static uint32_t hda_send_verb(uint32_t verb) {
    while (hda_read32(HDA_ICS) & 1);
    hda_write32(HDA_IR, verb);
    hda_write16(HDA_ICS, (*(volatile uint16_t *)(hda_base + HDA_ICS) & ~2) | 1);
    
    int timeout = 100000;
    while (timeout--) {
        if (!(hda_read32(HDA_ICS) & 1)) {
            return hda_read32(HDA_IR);
        }
    }
    return 0xFFFFFFFF;
}

void init_intel_hda(void) {
    uint8_t bus, slot, func;
    uint32_t bar0 = 0;
    
    for (bus = 0; bus < 255; bus++) {
        for (slot = 0; slot < 32; slot++) {
            for (func = 0; func < 8; func++) {
                uint32_t class_rev = pci_read_32(bus, slot, func, 0x08);
                if ((class_rev >> 16) == 0x0403) {
                    bar0 = pci_read_32(bus, slot, func, 0x10);
                    uint32_t cmd = pci_read_32(bus, slot, func, 0x04);
                    pci_write_32(bus, slot, func, 0x04, cmd | 0x06);
                    goto found;
                }
            }
        }
    }
    return;

found:
    hda_base = (volatile uint8_t *)(bar0 & 0xFFFFFFF0);
    
    hda_write32(HDA_GCTL, hda_read32(HDA_GCTL) & ~1);
    while (hda_read32(HDA_GCTL) & 1);
    
    hda_write32(HDA_GCTL, hda_read32(HDA_GCTL) | 1);
    while (!(hda_read32(HDA_GCTL) & 1));
    
    hda_pit_sleep_ms(10);
    
    uint32_t statests = hda_read32(HDA_STATESTS);
    (void)statests;

    uint32_t codec_address = 0;
    for (int i = 0; i < 4; i++) {
        if (statests & (1 << i)) {
            codec_address = i;
            break;
        }
    }

    uint32_t widget_dac = 0;
    uint32_t widget_pin = 0;

    for (uint32_t nid = 0x02; nid < 0x20; nid++) {
        uint32_t param = hda_send_verb((codec_address << 28) | (nid << 20) | (0xF00 << 8) | 0x04);
        uint32_t audio_widget_cap = hda_send_verb((codec_address << 28) | (nid << 20) | (0xF00 << 8) | 0x09);
        
        if ((param & 0x00F00000) == 0x00000000) {
            if ((audio_widget_cap & (1 << 8))) {
                widget_dac = nid;
            }
        } else if ((param & 0x00F00000) == 0x00400000) {
            uint32_t config = hda_send_verb((codec_address << 28) | (nid << 20) | (0xF09 << 8));
            if (((config >> 24) & 0x0F) == 0) {
                widget_pin = nid;
            }
        }
    }

    if (widget_dac && widget_pin) {
        hda_send_verb((codec_address << 28) | (widget_pin << 20) | (0x707 << 8) | 0x40);
        hda_send_verb((codec_address << 28) | (widget_pin << 20) | (0x706 << 8) | 0x00);
        hda_send_verb((codec_address << 28) | (widget_dac << 20) | (0x706 << 8) | 0x02);
        hda_send_verb((codec_address << 28) | (widget_pin << 20) | (0x708 << 8) | (widget_dac & 0xFF));
    }

    static uint32_t audio_buffer[22050] __attribute__((aligned(128)));
    for (int i = 0; i < 22050; i++) {
        int16_t sample = (i % 40 < 20) ? 10000 : -10000;
        audio_buffer[i] = ((uint32_t)(uint16_t)sample << 16) | (uint16_t)sample;
    }

    static struct hda_bdl_entry bdl_entry __attribute__((aligned(128)));
    bdl_entry.address_low = (uint32_t)(uintptr_t)&audio_buffer[0];
    bdl_entry.address_high = 0;
    bdl_entry.length = sizeof(audio_buffer);
    bdl_entry.ioc = 1;

    uint32_t gcap = hda_read32(HDA_GCAP);
    uint32_t num_iss = (gcap >> 8) & 0x0F;
    uint32_t stream_offset = 0x80 + (num_iss * 0x20);
    volatile uint8_t *stream_base = hda_base + stream_offset;

    *(volatile uint32_t *)(stream_base + 0x00) &= ~(1 << 1);
    while (*(volatile uint32_t *)(stream_base + 0x00) & (1 << 1));

    *(volatile uint32_t *)(stream_base + 0x00) |= (1 << 0);
    while (!(*(volatile uint32_t *)(stream_base + 0x00) & (1 << 0)));

    *(volatile uint32_t *)(stream_base + 0x00) &= ~(1 << 0);
    while (*(volatile uint32_t *)(stream_base + 0x00) & (1 << 0));

    *(volatile uint32_t *)(stream_base + 0x08) = sizeof(audio_buffer);
    *(volatile uint16_t *)(stream_base + 0x0C) = 0;
    *(volatile uint16_t *)(stream_base + 0x12) = 0x4010;

    *(volatile uint32_t *)(stream_base + 0x18) = (uint32_t)(uintptr_t)&bdl_entry;
    *(volatile uint32_t *)(stream_base + 0x1C) = 0;

    *(volatile uint32_t *)(stream_base + 0x00) |= (1 << 1);

    hda_pit_sleep_ms(5000);

    *(volatile uint32_t *)(stream_base + 0x00) &= ~(1 << 1);
}