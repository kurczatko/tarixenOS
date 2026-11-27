

#include <drivers/usb/storage/storage.h>

#define USB_MSC_CLASS         0x08
#define USB_MSC_SUBCLASS_SCSI 0x06
#define USB_MSC_PROTOCOL_BOT  0x50

#define CBW_SIGNATURE         0x43425355U
#define CSW_SIGNATURE         0x53425355U

#define SCSI_TEST_UNIT_READY  0x00
#define SCSI_REQUEST_SENSE    0x03
#define SCSI_INQUIRY          0x12
#define SCSI_READ_CAPACITY    0x25
#define SCSI_READ_10          0x28
#define SCSI_WRITE_10         0x2A

typedef struct {
    uint32_t dCBWSignature;
    uint32_t dCBWTag;
    uint32_t dCBWDataTransferLength;
    uint8_t  bCBWFlags;
    uint8_t  bCBWLUN;
    uint8_t  bCBWCBLength;
    uint8_t  CBWCB[16];
} __attribute__((packed)) usb_cbw_t;

typedef struct {
    uint32_t dCSWSignature;
    uint32_t dCSWTag;
    uint32_t dCSWDataResidue;
    uint8_t  bCSWStatus;
} __attribute__((packed)) usb_csw_t;

static usb_device_t *usb_msc_dev = 0;
static uint8_t usb_msc_ep_out = 0;
static uint8_t usb_msc_ep_in = 0;
static uint32_t usb_msc_tag = 1;
static uint32_t usb_msc_capacity = 0;
static uint32_t usb_msc_sector_size = 512;
static uint8_t usb_msc_cbw_buf[32] __attribute__((aligned(16)));
static uint8_t usb_msc_csw_buf[16] __attribute__((aligned(16)));
static uint8_t usb_msc_data_buf[512] __attribute__((aligned(16)));

static int usb_msc_find_endpoints(usb_device_t *dev) {
    usb_msc_ep_in = 0;
    usb_msc_ep_out = 0;
    for (int i = 0; i < USB_MAX_ENDPOINTS; i++) {
        if (!dev->ep[i].used) continue;
        if (dev->ep[i].type != USB_ENDPOINT_BULK) continue;
        if (dev->ep[i].address & 0x80) {
            if (!usb_msc_ep_in) usb_msc_ep_in = dev->ep[i].address;
        } else {
            if (!usb_msc_ep_out) usb_msc_ep_out = dev->ep[i].address;
        }
    }
    return (usb_msc_ep_in != 0 && usb_msc_ep_out != 0) ? 0 : -1;
}

static int usb_msc_send_cbw(uint8_t lun, uint8_t cmd_len, const uint8_t *cmd, uint32_t data_len, uint8_t direction) {
    usb_cbw_t *cbw = (usb_cbw_t *)usb_msc_cbw_buf;
    cbw->dCBWSignature = CBW_SIGNATURE;
    cbw->dCBWTag = usb_msc_tag++;
    cbw->dCBWDataTransferLength = data_len;
    cbw->bCBWFlags = direction;
    cbw->bCBWLUN = lun;
    cbw->bCBWCBLength = cmd_len;
    for (uint8_t i = 0; i < 16; i++) {
        cbw->CBWCB[i] = (i < cmd_len) ? cmd[i] : 0;
    }
    return usb_bulk_transfer(usb_msc_dev, usb_msc_ep_out, usb_msc_cbw_buf, 31, USB_DIR_OUT);
}

static int usb_msc_read_csw(uint32_t expected_tag) {
    usb_csw_t *csw = (usb_csw_t *)usb_msc_csw_buf;
    int ret = usb_bulk_transfer(usb_msc_dev, usb_msc_ep_in, usb_msc_csw_buf, 13, USB_DIR_IN);
    if (ret != 0) return -1;
    if (csw->dCSWSignature != CSW_SIGNATURE) return -2;
    if (csw->dCSWTag != expected_tag) return -3;
    return csw->bCSWStatus;
}

static int usb_msc_scsi_cmd(uint8_t cmd_len, const uint8_t *cmd, void *data, uint32_t data_len, uint8_t direction) {
    int ret = usb_msc_send_cbw(0, cmd_len, cmd, data_len, direction);
    if (ret != 0) return -1;
    uint32_t tag = usb_msc_tag - 1;
    if (data_len > 0) {
        uint8_t ep = (direction == USB_DIR_IN) ? usb_msc_ep_in : usb_msc_ep_out;
        ret = usb_bulk_transfer(usb_msc_dev, ep, data, (uint16_t)data_len, direction);
        if (ret != 0) return -2;
    }
    ret = usb_msc_read_csw(tag);
    if (ret != 0) return ret;
    return 0;
}

static void usb_msc_reset_recovery(usb_device_t *dev) {
    usb_control_transfer(dev, USB_DIR_OUT | USB_TYPE_CLASS | USB_RECIP_INTERFACE,
                         0xFF, 0, dev->interface_num, 0, 0);
    usb_delay_ms(10);
}

int usb_storage_init(void) {
    if (usb_msc_dev && usb_msc_dev->used && usb_msc_dev->configured) return 1;
    usb_msc_dev = 0;

    for (int i = 0; i < usb_device_count(); i++) {
        usb_device_t *dev = usb_device_get(i);
        if (!dev || !dev->configured) continue;
        if (dev->dev_class == USB_MSC_CLASS &&
            dev->subclass == USB_MSC_SUBCLASS_SCSI &&
            dev->protocol == USB_MSC_PROTOCOL_BOT) {
            if (usb_msc_find_endpoints(dev) != 0) continue;
            usb_msc_dev = dev;

            uint8_t cmd_inquiry[6] = { SCSI_INQUIRY, 0, 0, 0, 36, 0 };
            if (usb_msc_scsi_cmd(6, cmd_inquiry, usb_msc_data_buf, 36, USB_DIR_IN) != 0) {
                usb_msc_reset_recovery(dev);
                if (usb_msc_scsi_cmd(6, cmd_inquiry, usb_msc_data_buf, 36, USB_DIR_IN) != 0) {
                    usb_msc_dev = 0;
                    continue;
                }
            }

            
            uint8_t cmd_tur[6] = { SCSI_TEST_UNIT_READY, 0, 0, 0, 0, 0 };
            for (int r = 0; r < 5; r++) {
                if (usb_msc_scsi_cmd(6, cmd_tur, 0, 0, USB_DIR_OUT) == 0) break;
                uint8_t cmd_sense[6] = { SCSI_REQUEST_SENSE, 0, 0, 0, 18, 0 };
                usb_msc_scsi_cmd(6, cmd_sense, usb_msc_data_buf, 18, USB_DIR_IN);
                usb_delay_ms(20);
            }

            uint8_t cmd_cap[10] = { SCSI_READ_CAPACITY, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
            if (usb_msc_scsi_cmd(10, cmd_cap, usb_msc_data_buf, 8, USB_DIR_IN) != 0) {
                usb_msc_dev = 0;
                continue;
            }

            uint32_t last_lba = ((uint32_t)usb_msc_data_buf[0] << 24) |
                                ((uint32_t)usb_msc_data_buf[1] << 16) |
                                ((uint32_t)usb_msc_data_buf[2] << 8) |
                                (uint32_t)usb_msc_data_buf[3];
            uint32_t block_size = ((uint32_t)usb_msc_data_buf[4] << 24) |
                                  ((uint32_t)usb_msc_data_buf[5] << 16) |
                                  ((uint32_t)usb_msc_data_buf[6] << 8) |
                                  (uint32_t)usb_msc_data_buf[7];
            usb_msc_capacity = last_lba + 1U;
            usb_msc_sector_size = block_size ? block_size : 512U;
            return 1;
        }
    }
    return 0;
}

void usb_storage_poll(void) {
    if (!usb_msc_dev || !usb_msc_dev->used) {
        usb_storage_init();
    }
}

int usb_storage_present(void) {
    return (usb_msc_dev && usb_msc_dev->used && usb_msc_capacity > 0) ? 1 : 0;
}

uint32_t usb_storage_read_sectors(uint32_t lba, uint32_t count, void *buffer) {
    if (!usb_msc_dev || count == 0) return 0;
    uint8_t *dst = (uint8_t *)buffer;
    for (uint32_t s = 0; s < count; s++) {
        uint32_t cur_lba = lba + s;
        uint8_t cmd[10] = { SCSI_READ_10, 0, 0, 0, 0, 0, 0, 0, 1, 0 };
        cmd[2] = (uint8_t)((cur_lba >> 24) & 0xFF);
        cmd[3] = (uint8_t)((cur_lba >> 16) & 0xFF);
        cmd[4] = (uint8_t)((cur_lba >> 8) & 0xFF);
        cmd[5] = (uint8_t)(cur_lba & 0xFF);
        if (usb_msc_scsi_cmd(10, cmd, dst + s * usb_msc_sector_size, usb_msc_sector_size, USB_DIR_IN) != 0) {
            return s;
        }
    }
    return count;
}

uint32_t usb_storage_write_sectors(uint32_t lba, uint32_t count, const void *buffer) {
    if (!usb_msc_dev || count == 0) return 0;
    const uint8_t *src = (const uint8_t *)buffer;
    for (uint32_t s = 0; s < count; s++) {
        uint32_t cur_lba = lba + s;
        uint8_t cmd[10] = { SCSI_WRITE_10, 0, 0, 0, 0, 0, 0, 0, 1, 0 };
        cmd[2] = (uint8_t)((cur_lba >> 24) & 0xFF);
        cmd[3] = (uint8_t)((cur_lba >> 16) & 0xFF);
        cmd[4] = (uint8_t)((cur_lba >> 8) & 0xFF);
        cmd[5] = (uint8_t)(cur_lba & 0xFF);
        if (usb_msc_scsi_cmd(10, cmd, (void *)(src + s * usb_msc_sector_size), usb_msc_sector_size, USB_DIR_OUT) != 0) {
            return s;
        }
    }
    return count;
}

uint32_t usb_storage_get_capacity(void) {
    return usb_msc_capacity;
}

uint32_t usb_storage_get_sector_size(void) {
    return usb_msc_sector_size;
}
