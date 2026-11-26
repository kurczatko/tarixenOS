/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

//FIXME: caly sterownik nie dziala

#include <drivers/hid/kamerki/quanta_acer_hd.h>

typedef unsigned int uintptr_t;

static QuantaACERHD g_quanta_storage;

static inline uint8_t quanta_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t quanta_inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t quanta_inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void quanta_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void quanta_outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void quanta_outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t quanta_pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFCU);
    quanta_outl(0xCF8U, address);
    return quanta_inl(0xCFCU);
}

static void quanta_pci_write32(uint8_t bus, uint8_t slot, uint8_t func,
                                uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFCU);
    quanta_outl(0xCF8U, address);
    quanta_outl(0xCFCU, value);
}

static void quanta_delay_us(uint32_t us)
{
    for (uint32_t i = 0U; i < us; i++) {
        for (uint32_t j = 0U; j < 48U; j++);
    }
}

static void quanta_delay_ms(uint32_t ms)
{
    for (uint32_t i = 0U; i < ms; i++) {
        for (uint32_t j = 0U; j < 48000U; j++);
    }
}

QuantaACERHD* QuantaACERHD::g_instance = 0;

QuantaACERHD* QuantaACERHD::instance()
{
    return &g_quanta_storage;
}

void QuantaACERHD::set_device_addr(uint8_t index, uint8_t addr)
{
    if (index < QUANTA_MAX_CAMERAS) {
        cameras[index].address = addr;
    }
}

QuantaACERHD::QuantaACERHD()
    : camera_count_val(0U), initialized(0U)
{
    for (uint32_t i = 0U; i < QUANTA_MAX_CAMERAS; i++) {
        cameras[i].bus = 0U;
        cameras[i].slot = 0U;
        cameras[i].func = 0U;
        cameras[i].vendor_id = 0U;
        cameras[i].product_id = 0U;
        cameras[i].device_release = 0U;
        cameras[i].address = 0U;
        cameras[i].interface_ctrl = 0xFFU;
        cameras[i].interface_stream = 0xFFU;
        cameras[i].initialized = 0U;
        cameras[i].enabled = 0U;
        cameras[i].streaming = 0U;
        cameras[i].uvc_version_major = 1U;
        cameras[i].uvc_version_minor = 0U;
        cameras[i].clock_frequency = 48000000U;
        cameras[i].bNumFormats = 0U;
        cameras[i].bNumFrames = 0U;
        cameras[i].default_format = 0U;
        cameras[i].default_frame = 0U;
        cameras[i].stream_count = 0U;
        cameras[i].ctrl_count = 0U;
        cameras[i].power_state = QUANTA_POWER_D0;
        cameras[i].frames_captured = 0U;
        cameras[i].frames_dropped = 0U;
        cameras[i].control_errors = 0U;
        cameras[i].iso_errors = 0U;
        cameras[i].bytes_received = 0U;
        for (uint32_t j = 0U; j < QUANTA_MAX_FRAMES; j++) {
            cameras[i].frames[j].wWidth = 0U;
            cameras[i].frames[j].wHeight = 0U;
            cameras[i].frames[j].dwFrameInterval = 0U;
            cameras[i].frames[j].dwDefaultFrameInterval = 0U;
            cameras[i].frames[j].bFrameIntervalType = 0U;
            cameras[i].frames[j].dwBytesPerLine = 0U;
            cameras[i].frames[j].dwMinBitRate = 0U;
            cameras[i].frames[j].dwMaxBitRate = 0U;
            cameras[i].frames[j].dwMaxVideoFrameBufSize = 0U;
            cameras[i].frames[j].interval_min = 0U;
            cameras[i].frames[j].interval_max = 0U;
            cameras[i].frames[j].interval_step = 0U;
            cameras[i].frames[j].interval_default = 0U;
        }
        for (uint32_t j = 0U; j < QUANTA_MAX_STREAMS; j++) {
            cameras[i].streams[j].stream_index = (uint8_t)j;
            cameras[i].streams[j].bEndpointAddress = 0U;
            cameras[i].streams[j].wMaxPacketSize = 0U;
            cameras[i].streams[j].bFormatIndex = 0U;
            cameras[i].streams[j].bFrameIndex = 0U;
            cameras[i].streams[j].dwFrameInterval = 0U;
            cameras[i].streams[j].running = 0U;
            cameras[i].streams[j].buffer_size = 0U;
            cameras[i].streams[j].buf_idx = 0U;
            cameras[i].streams[j].packets_received = 0U;
            cameras[i].streams[j].packets_dropped = 0U;
            cameras[i].streams[j].errors = 0U;
            cameras[i].streams[j].frame_size = 0U;
            cameras[i].streams[j].frame_ready = 0U;
        }
        for (uint32_t j = 0U; j < QUANTA_MAX_CTLS; j++) {
            cameras[i].ctrls[j].bControlSelector = 0U;
            cameras[i].ctrls[j].bEntityId = 0U;
            cameras[i].ctrls[j].bmControls = 0U;
            cameras[i].ctrls[j].val_min = 0;
            cameras[i].ctrls[j].val_max = 0;
            cameras[i].ctrls[j].val_res = 0;
            cameras[i].ctrls[j].val_default = 0;
            cameras[i].ctrls[j].val_current = 0;
            cameras[i].ctrls[j].autoupdate = 0U;
        }
    }
    stats.cameras_found = 0U;
    stats.cameras_init = 0U;
    stats.streams_started = 0U;
    stats.streams_stopped = 0U;
    stats.frames_captured = 0U;
    stats.frames_dropped = 0U;
    stats.controls_read = 0U;
    stats.controls_written = 0U;
    stats.control_errors = 0U;
    stats.iso_errors = 0U;
    stats.resets = 0U;
    stats.bytes_received = 0U;
}

QuantaACERHD::~QuantaACERHD()
{
}

uint8_t QuantaACERHD::probe(uint16_t vendor, uint16_t device)
{
    if (vendor == QUANTA_VID &&
        (device == QUANTA_ACER_HD_PID1 ||
         device == QUANTA_ACER_HD_PID2 ||
         device == QUANTA_ACER_HD_PID3)) {
        return 1U;
    }
    return 0U;
}

int QuantaACERHD::find_camera(uint8_t bus, uint8_t slot, uint8_t func)
{
    for (uint32_t i = 0U; i < camera_count_val; i++) {
        if (cameras[i].bus == bus && cameras[i].slot == slot && cameras[i].func == func) {
            return (int)i;
        }
    }
    return -1;
}

void QuantaACERHD::scan_cameras(void)
{
    camera_count_val = 0U;
    stats.cameras_found = 0U;

    for (uint16_t bus = 0U; bus < 256U && camera_count_val < QUANTA_MAX_CAMERAS; bus++) {
        for (uint8_t slot = 0U; slot < 32U && camera_count_val < QUANTA_MAX_CAMERAS; slot++) {
            for (uint8_t func = 0U; func < 8U && camera_count_val < QUANTA_MAX_CAMERAS; func++) {
                uint32_t id0 = quanta_pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (!probe(vendor, device)) continue;

                uint32_t class_code = quanta_pci_read32(bus, slot, func, 0x08U);
                uint8_t dev_class = (uint8_t)(class_code >> 24);
                if (dev_class != USB_CLASS_VIDEO) continue;

                uint32_t bar0 = quanta_pci_read32(bus, slot, func, 0x10U);
                if (bar0 == 0U || bar0 == 0xFFFFFFFFU) continue;

                uint32_t command = quanta_pci_read32(bus, slot, func, 0x04U);
                quanta_pci_write32(bus, slot, func, 0x04U, command | 0x00000006U);

                struct quanta_camera *c = &cameras[camera_count_val];
                c->bus = bus;
                c->slot = slot;
                c->func = func;
                c->vendor_id = vendor;
                c->product_id = device;
                c->device_release = (uint16_t)((class_code >> 16) & 0xFFFFU);
                c->address = (uint8_t)(slot & 0x7FU);
                c->interface_ctrl = 0xFFU;
                c->interface_stream = 0xFFU;
                c->initialized = 0U;
                c->enabled = 0U;
                c->streaming = 0U;
                c->stream_count = 0U;
                c->ctrl_count = 0U;
                c->power_state = QUANTA_POWER_D0;
                c->frames_captured = 0U;
                c->frames_dropped = 0U;
                c->control_errors = 0U;
                c->iso_errors = 0U;
                c->bytes_received = 0U;
                camera_count_val++;
                stats.cameras_found++;
            }
        }
    }
}

void QuantaACERHD::read_camera_desc(uint8_t index)
{
    if (index >= camera_count_val) return;
    struct quanta_camera *c = &cameras[index];
    c->uvc_version_major = 1U;
    c->uvc_version_minor = 5U;
    c->clock_frequency = 48000000U;
    c->bNumFormats = 2U;
    c->bNumFrames = 3U;
    c->default_format = UVC_VS_FORMAT_MJPEG;
    c->default_frame = 0U;
}

void QuantaACERHD::probe_uvc_ctrl_iface(uint8_t index)
{
    if (index >= camera_count_val) return;
    struct quanta_camera *c = &cameras[index];
    c->ctrl_count = 0U;

    for (uint8_t i = 0U; i < QUANTA_MAX_CTLS && c->ctrl_count < QUANTA_MAX_CTLS; i++) {
        struct quanta_control *ctrl = &c->ctrls[c->ctrl_count];
        ctrl->bEntityId = (uint8_t)(5U + i);
        ctrl->bmControls = 0x01U;
        ctrl->autoupdate = 0U;

        switch (i) {
            case 0:
                ctrl->bControlSelector = UVC_PU_BRIGHTNESS;
                ctrl->val_min = -64; ctrl->val_max = 64;
                ctrl->val_res = 1; ctrl->val_default = 0;
                ctrl->val_current = 0;
                break;
            case 1:
                ctrl->bControlSelector = UVC_PU_CONTRAST;
                ctrl->val_min = 0; ctrl->val_max = 128;
                ctrl->val_res = 1; ctrl->val_default = 64;
                ctrl->val_current = 64;
                break;
            case 2:
                ctrl->bControlSelector = UVC_PU_HUE;
                ctrl->val_min = -40; ctrl->val_max = 40;
                ctrl->val_res = 1; ctrl->val_default = 0;
                ctrl->val_current = 0;
                break;
            case 3:
                ctrl->bControlSelector = UVC_PU_SATURATION;
                ctrl->val_min = 0; ctrl->val_max = 128;
                ctrl->val_res = 1; ctrl->val_default = 64;
                ctrl->val_current = 64;
                break;
            case 4:
                ctrl->bControlSelector = UVC_PU_GAIN;
                ctrl->val_min = 0; ctrl->val_max = 255;
                ctrl->val_res = 1; ctrl->val_default = 0;
                ctrl->val_current = 0;
                break;
            case 5:
                ctrl->bControlSelector = UVC_PU_AE_MODE;
                ctrl->val_min = 0; ctrl->val_max = 5;
                ctrl->val_res = 1; ctrl->val_default = 1;
                ctrl->val_current = 1;
                break;
            case 6:
                ctrl->bControlSelector = UVC_PU_EXPOSURE_TIME_ABS;
                ctrl->val_min = 1; ctrl->val_max = 10000;
                ctrl->val_res = 1; ctrl->val_default = 100;
                ctrl->val_current = 100;
                break;
            case 7:
                ctrl->bControlSelector = UVC_PU_FOCUS_ABS;
                ctrl->val_min = 0; ctrl->val_max = 65535;
                ctrl->val_res = 1; ctrl->val_default = 0;
                ctrl->val_current = 0;
                break;
            default:
                continue;
        }
        c->ctrl_count++;
    }
}

void QuantaACERHD::enumerate_frames(uint8_t index)
{
    if (index >= camera_count_val) return;
    struct quanta_camera *c = &cameras[index];
    c->bNumFrames = 3U;

    static const uint16_t widths[3] = {640U, 1280U, 1920U};
    static const uint16_t heights[3] = {480U, 720U, 1080U};
    static const uint32_t intervals[3] = {333333U, 666666U, 1000000U};

    for (uint8_t i = 0U; i < c->bNumFrames && i < QUANTA_MAX_FRAMES; i++) {
        struct quanta_frame_desc *f = &c->frames[i];
        f->wWidth = widths[i];
        f->wHeight = heights[i];
        f->dwFrameInterval = intervals[i];
        f->dwDefaultFrameInterval = intervals[i];
        f->bFrameIntervalType = 1U;
        f->dwBytesPerLine = (uint32_t)widths[i] * 2U;
        f->dwMinBitRate = f->dwBytesPerLine * f->wHeight * 30U;
        f->dwMaxBitRate = f->dwBytesPerLine * f->wHeight * 30U;
        f->dwMaxVideoFrameBufSize = (uint32_t)widths[i] * (uint32_t)heights[i] * 2U;
        f->interval_min = intervals[i];
        f->interval_max = intervals[i];
        f->interval_step = 0U;
        f->interval_default = intervals[i];
    }
}

void QuantaACERHD::enumerate_controls(uint8_t index)
{
    if (index >= camera_count_val) return;
    probe_uvc_ctrl_iface(index);
}

void QuantaACERHD::init_camera(uint8_t index)
{
    if (index >= camera_count_val || cameras[index].initialized) return;

    quanta_delay_us(100U);
    read_camera_desc(index);
    quanta_delay_ms(10U);
    enumerate_frames(index);
    enumerate_controls(index);

    cameras[index].initialized = 1U;
    cameras[index].enabled = 1U;
    cameras[index].streaming = 0U;
    cameras[index].power_state = QUANTA_POWER_D0;
    stats.cameras_init++;
}

void QuantaACERHD::init(void)
{
    if (initialized) return;
    scan_cameras();
    for (uint32_t i = 0U; i < camera_count_val; i++) {
        init_camera(i);
    }
    initialized = 1U;
}

void QuantaACERHD::deinit(void)
{
    if (!initialized) return;
    for (uint32_t i = 0U; i < camera_count_val; i++) {
        if (cameras[i].initialized) {
            if (cameras[i].streaming) {
                for (uint32_t s = 0U; s < cameras[i].stream_count; s++) {
                    cameras[i].streams[s].running = 0U;
                }
            }
            cameras[i].initialized = 0U;
            cameras[i].enabled = 0U;
        }
    }
    camera_count_val = 0U;
    initialized = 0U;
}

void QuantaACERHD::poll(void)
{
    if (!initialized) return;
    for (uint32_t i = 0U; i < camera_count_val; i++) {
        if (cameras[i].initialized && cameras[i].streaming) {
            for (uint32_t s = 0U; s < cameras[i].stream_count; s++) {
                if (cameras[i].streams[s].running) {
                    if (cameras[i].streams[s].frame_ready) {
                        cameras[i].frames_captured++;
                        stats.frames_captured++;
                        cameras[i].streams[s].frame_ready = 0U;
                    }
                }
            }
        }
    }
}

uint8_t QuantaACERHD::is_ready(void)
{
    return initialized;
}

uint8_t QuantaACERHD::camera_count(void)
{
    return camera_count_val;
}

int QuantaACERHD::camera_info(uint8_t index, struct quanta_camera *out)
{
    if (index >= camera_count_val || !out) return 0;
    *out = cameras[index];
    return 1;
}

int QuantaACERHD::stream_start(uint8_t camera_index, uint8_t stream_index,
                                uint32_t width, uint32_t height, uint32_t fps)
{
    if (camera_index >= camera_count_val) return 0;
    struct quanta_camera *c = &cameras[camera_index];
    if (!c->initialized || stream_index >= QUANTA_MAX_STREAMS) return 0;

    struct quanta_stream *s = &c->streams[stream_index];
    if (s->running) return 0;

    s->bFormatIndex = c->default_format;
    s->bFrameIndex = c->default_frame;
    s->dwFrameInterval = (fps > 0) ? (10000000U / fps) : 333333U;
    s->wMaxPacketSize = 1024U;
    s->bEndpointAddress = 0x81U;
    s->running = 1U;
    s->buf_idx = 0U;
    s->frame_size = width * height * 2U;
    s->packets_received = 0U;
    s->packets_dropped = 0U;
    s->errors = 0U;
    s->frame_ready = 0U;

    if (stream_index >= c->stream_count) {
        c->stream_count = (uint8_t)(stream_index + 1U);
    }
    c->streaming = 1U;
    stats.streams_started++;
    return 1;
}

int QuantaACERHD::stream_stop(uint8_t camera_index, uint8_t stream_index)
{
    if (camera_index >= camera_count_val) return 0;
    struct quanta_camera *c = &cameras[camera_index];
    if (stream_index >= QUANTA_MAX_STREAMS) return 0;

    struct quanta_stream *s = &c->streams[stream_index];
    s->running = 0U;
    s->frame_ready = 0U;

    int any_running = 0;
    for (uint32_t i = 0U; i < c->stream_count; i++) {
        if (c->streams[i].running) { any_running = 1; break; }
    }
    if (!any_running) c->streaming = 0U;
    stats.streams_stopped++;
    return 1;
}

int QuantaACERHD::stream_pause(uint8_t camera_index, uint8_t stream_index)
{
    if (camera_index >= camera_count_val || stream_index >= QUANTA_MAX_STREAMS) return 0;
    cameras[camera_index].streams[stream_index].running = 0U;
    return 1;
}

int QuantaACERHD::stream_resume(uint8_t camera_index, uint8_t stream_index)
{
    if (camera_index >= camera_count_val || stream_index >= QUANTA_MAX_STREAMS) return 0;
    cameras[camera_index].streams[stream_index].running = 1U;
    return 1;
}

int QuantaACERHD::frame_ready(uint8_t camera_index, uint8_t stream_index)
{
    if (camera_index >= camera_count_val) return 0;
    struct quanta_camera *c = &cameras[camera_index];
    if (stream_index >= c->stream_count) return 0;
    return c->streams[stream_index].frame_ready ? 1 : 0;
}

int QuantaACERHD::frame_read(uint8_t camera_index, uint8_t stream_index,
                              void *buf, uint32_t *size)
{
    if (camera_index >= camera_count_val || !buf || !size) return 0;
    struct quanta_camera *c = &cameras[camera_index];
    if (stream_index >= c->stream_count) return 0;

    struct quanta_stream *s = &c->streams[stream_index];
    if (!s->frame_ready) return 0;

    uint32_t copy_sz = s->frame_size < *size ? s->frame_size : *size;
    for (uint32_t i = 0U; i < copy_sz; i++) {
        ((uint8_t*)buf)[i] = s->frame_buffer[i];
    }
    *size = copy_sz;
    s->frame_ready = 0U;
    stats.frames_captured++;
    c->frames_captured++;
    return 1;
}

int QuantaACERHD::snapshot(uint8_t camera_index, uint8_t stream_index,
                            void *buf, uint32_t *size)
{
    if (camera_index >= camera_count_val || !buf || !size) return 0;
    struct quanta_camera *c = &cameras[camera_index];
    if (stream_index >= c->stream_count) return 0;
    return frame_read(camera_index, stream_index, buf, size);
}

int QuantaACERHD::control_get(uint8_t camera_index, uint8_t ctrl_sel, int32_t *value)
{
    if (camera_index >= camera_count_val || !value) return 0;
    struct quanta_camera *c = &cameras[camera_index];
    for (uint32_t i = 0U; i < c->ctrl_count; i++) {
        if (c->ctrls[i].bControlSelector == ctrl_sel) {
            *value = c->ctrls[i].val_current;
            stats.controls_read++;
            return 1;
        }
    }
    return 0;
}

int QuantaACERHD::control_set(uint8_t camera_index, uint8_t ctrl_sel, int32_t value)
{
    if (camera_index >= camera_count_val) return 0;
    struct quanta_camera *c = &cameras[camera_index];
    for (uint32_t i = 0U; i < c->ctrl_count; i++) {
        if (c->ctrls[i].bControlSelector == ctrl_sel) {
            if (value < c->ctrls[i].val_min) value = c->ctrls[i].val_min;
            if (value > c->ctrls[i].val_max) value = c->ctrls[i].val_max;
            c->ctrls[i].val_current = value;
            stats.controls_written++;
            return 1;
        }
    }
    return 0;
}

int QuantaACERHD::control_range(uint8_t camera_index, uint8_t ctrl_sel,
                                 int32_t *min, int32_t *max, int32_t *res, int32_t *def)
{
    if (camera_index >= camera_count_val) return 0;
    struct quanta_camera *c = &cameras[camera_index];
    for (uint32_t i = 0U; i < c->ctrl_count; i++) {
        if (c->ctrls[i].bControlSelector == ctrl_sel) {
            if (min) *min = c->ctrls[i].val_min;
            if (max) *max = c->ctrls[i].val_max;
            if (res) *res = c->ctrls[i].val_res;
            if (def) *def = c->ctrls[i].val_default;
            return 1;
        }
    }
    return 0;
}

int QuantaACERHD::control_auto(uint8_t camera_index, uint8_t ctrl_sel, uint8_t enable)
{
    if (camera_index >= camera_count_val) return 0;
    struct quanta_camera *c = &cameras[camera_index];
    for (uint32_t i = 0U; i < c->ctrl_count; i++) {
        if (c->ctrls[i].bControlSelector == ctrl_sel) {
            c->ctrls[i].autoupdate = enable;
            return 1;
        }
    }
    return 0;
}

int QuantaACERHD::power_set_state(uint8_t camera_index, uint8_t power_state)
{
    if (camera_index >= camera_count_val) return 0;
    cameras[camera_index].power_state = power_state;
    stats.resets++;
    return 1;
}

int QuantaACERHD::power_get_state(uint8_t camera_index, uint8_t *power_state)
{
    if (camera_index >= camera_count_val || !power_state) return 0;
    *power_state = cameras[camera_index].power_state;
    return 1;
}

int QuantaACERHD::reset_camera(uint8_t camera_index)
{
    if (camera_index >= camera_count_val) return 0;
    cameras[camera_index].streaming = 0U;
    cameras[camera_index].enabled = 0U;
    for (uint32_t i = 0U; i < QUANTA_MAX_STREAMS; i++) {
        cameras[camera_index].streams[i].running = 0U;
        cameras[camera_index].streams[i].frame_ready = 0U;
    }
    quanta_delay_ms(100U);
    init_camera(camera_index);
    stats.resets++;
    return 1;
}

void QuantaACERHD::stats_get(struct quanta_stats *out)
{
    if (!out) return;
    *out = stats;
}

void QuantaACERHD::stats_reset(void)
{
    stats.cameras_found = 0U;
    stats.cameras_init = 0U;
    stats.streams_started = 0U;
    stats.streams_stopped = 0U;
    stats.frames_captured = 0U;
    stats.frames_dropped = 0U;
    stats.controls_read = 0U;
    stats.controls_written = 0U;
    stats.control_errors = 0U;
    stats.iso_errors = 0U;
    stats.resets = 0U;
    stats.bytes_received = 0U;
}

void QuantaACERHD::debug_print_cameras(void)
{
    for (uint32_t i = 0U; i < camera_count_val; i++) {
        (void)cameras[i].vendor_id;
        (void)cameras[i].product_id;
        (void)cameras[i].initialized;
    }
}

extern "C" int quanta_acer_hd_init(void)
{
    QuantaACERHD *drv = QuantaACERHD::instance();
    drv->init();
    return drv->is_ready() ? 1 : 0;
}

extern "C" void quanta_acer_hd_shutdown(void)
{
    QuantaACERHD *drv = QuantaACERHD::instance();
    if (drv) drv->deinit();
}

extern "C" void quanta_acer_hd_poll(void)
{
    QuantaACERHD *drv = QuantaACERHD::instance();
    if (drv && drv->is_ready()) drv->poll();
}
