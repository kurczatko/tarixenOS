#include "storage.h"

#define USB_MSC_CLASS        0x08
#define USB_MSC_SUBCLASS     0x06
#define USB_MSC_PROTOCOL     0x50

#define CBW_SIGNATURE        0x43425355
#define CSW_SIGNATURE        0x53425355

#define SCSI_TEST_UNIT_READY 0x00
#define SCSI_INQUIRY         0x12
#define SCSI_READ_CAPACITY   0x25
#define SCSI_READ_10         0x28

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
static uint8_t usb_msc_cbw_buf[32];
static uint8_t usb_msc_csw_buf[16];
static uint8_t usb_msc_data_buf[512];

static int usb_msc_find_endpoints(usb_device_t *dev) {
    int i;
    for (i = 0; i < USB_MAX_ENDPOINTS; i++) {
        if (!dev->ep[i].used) continue;
        if (dev->ep[i].type != USB_ENDPOINT_BULK) continue;
        if (dev->ep[i].address & 0x80) {
            usb_msc_ep_in = dev->ep[i].address;
        } else {
            usb_msc_ep_out = dev->ep[i].address;
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
    uint8_t i;
    for (i = 0; i < 16; i++) {
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
    if (data_len > 0) {
        ret = usb_bulk_transfer(usb_msc_dev, usb_msc_ep_in, data, data_len, USB_DIR_IN);
        if (ret != 0) return -2;
    }
    uint32_t tag = usb_msc_tag - 1;
    ret = usb_msc_read_csw(tag);
    if (ret != 0) return ret;
    return 0;
}

void usb_storage_init(void) {
    int i;
    for (i = 0; i < usb_device_count(); i++) {
        usb_device_t *dev = usb_device_get(i);
        if (!dev || !dev->configured) continue;
        if (dev->class == USB_MSC_CLASS && dev->subclass == USB_MSC_SUBCLASS && dev->protocol == USB_MSC_PROTOCOL) {
            if (usb_msc_find_endpoints(dev) == 0) {
                usb_msc_dev = dev;
                uint8_t cmd_inquiry[6] = { SCSI_INQUIRY, 0, 0, 0, 36, 0 };
                int ret = usb_msc_scsi_cmd(6, cmd_inquiry, usb_msc_data_buf, 36, USB_DIR_IN);
                if (ret != 0) {
                    usb_msc_dev = 0;
                    return;
                }
                uint8_t cmd_capacity[10] = { SCSI_READ_CAPACITY, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
                ret = usb_msc_scsi_cmd(10, cmd_capacity, usb_msc_data_buf, 8, USB_DIR_IN);
                if (ret != 0) {
                    usb_msc_dev = 0;
                    return;
                }
                uint32_t *lba_ptr = (uint32_t *)usb_msc_data_buf;
                uint32_t *size_ptr = (uint32_t *)(usb_msc_data_buf + 4);
                uint32_t last_lba = ((lba_ptr[0] >> 24) & 0xFF) | (((lba_ptr[0] >> 16) & 0xFF) << 8) | (((lba_ptr[0] >> 8) & 0xFF) << 16) | (lba_ptr[0] & 0xFF);
                uint32_t block_size = ((size_ptr[0] >> 24) & 0xFF) | (((size_ptr[0] >> 16) & 0xFF) << 8) | (((size_ptr[0] >> 8) & 0xFF) << 16) | (size_ptr[0] & 0xFF);
                usb_msc_capacity = last_lba + 1;
                usb_msc_sector_size = block_size;
                return;
            }
        }
    }
}

void usb_storage_poll(void) {
}

uint32_t usb_storage_read_sectors(uint32_t lba, uint32_t count, void *buffer) {
    if (!usb_msc_dev) return 0;
    uint32_t total = count * usb_msc_sector_size;
    if (total > 512) total = 512;
    uint8_t cmd[10] = { SCSI_READ_10, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
    cmd[2] = (uint8_t)((lba >> 24) & 0xFF);
    cmd[3] = (uint8_t)((lba >> 16) & 0xFF);
    cmd[4] = (uint8_t)((lba >> 8) & 0xFF);
    cmd[5] = (uint8_t)(lba & 0xFF);
    cmd[7] = (uint8_t)((count >> 8) & 0xFF);
    cmd[8] = (uint8_t)(count & 0xFF);
    int ret = usb_msc_scsi_cmd(10, cmd, buffer, total, USB_DIR_IN);
    if (ret != 0) return 0;
    return count;
}

uint32_t usb_storage_get_capacity(void) {
    return usb_msc_capacity;
}

uint32_t usb_storage_get_sector_size(void) {
    return usb_msc_sector_size;
}
