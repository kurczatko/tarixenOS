#ifndef QUANTA_ACER_HD_H
#define QUANTA_ACER_HD_H

#include <include/printf/types.h>

#define QUANTA_VID          0x0408U
#define QUANTA_ACER_HD_PID1 0x0606U
#define QUANTA_ACER_HD_PID2 0x0607U
#define QUANTA_ACER_HD_PID3 0x0608U

#define QUANTA_MAX_CAMERAS  4U
#define QUANTA_MAX_STREAMS  2U
#define QUANTA_MAX_FRAMES   8U
#define QUANTA_MAX_CTLS     16U
#define QUANTA_ISO_BUFS     8U
#define QUANTA_ISO_PKTS     8U
#define QUANTA_FRAME_BUF_SZ (640 * 480 * 2)

#define USB_CLASS_VIDEO          0x0EU
#define USB_SUBCLASS_VIDEO_CTRL  0x01U
#define USB_SUBCLASS_VIDEO_STRM  0x02U
#define USB_PROTOCOL_UVC         0x00U

#define UVC_DT_CS_UNDEFINED      0x20U
#define UVC_DT_CS_DEVICE         0x21U
#define UVC_DT_CS_CONFIG         0x22U
#define UVC_DT_CS_STRING         0x23U
#define UVC_DT_CS_INTERFACE      0x24U
#define UVC_DT_CS_ENDPOINT       0x25U

#define UVC_VC_DESCRIPTOR_UNDEFINED  0x00U
#define UVC_VC_HEADER                0x01U
#define UVC_VC_INPUT_TERMINAL        0x02U
#define UVC_VC_OUTPUT_TERMINAL       0x03U
#define UVC_VC_SELECTOR_UNIT         0x04U
#define UVC_VC_PROCESSING_UNIT       0x05U
#define UVC_VC_EXTENSION_UNIT        0x06U

#define UVC_VS_UNDEFINED           0x00U
#define UVC_VS_INPUT_HEADER        0x01U
#define UVC_VS_OUTPUT_HEADER       0x02U
#define UVC_VS_STILL_IMAGE_FRAME   0x03U
#define UVC_VS_FORMAT_UNCOMPRESSED 0x04U
#define UVC_VS_FRAME_UNCOMPRESSED  0x05U
#define UVC_VS_FORMAT_MJPEG        0x06U
#define UVC_VS_FRAME_MJPEG         0x07U

#define UVC_VS_CONTROL_UNDEFINED   0x00U
#define UVC_VS_PROBE_CONTROL       0x01U
#define UVC_VS_COMMIT_CONTROL      0x02U
#define UVC_VS_STILL_IMAGE_TRIGGER 0x05U

#define UVC_PU_CONTROL_UNDEFINED   0x00U
#define UVC_PU_BACKLIGHT_COMP      0x01U
#define UVC_PU_BRIGHTNESS          0x02U
#define UVC_PU_CONTRAST            0x03U
#define UVC_PU_GAIN                0x04U
#define UVC_PU_POWER_LINE_FREQ     0x05U
#define UVC_PU_HUE                 0x06U
#define UVC_PU_SATURATION          0x07U
#define UVC_PU_SHARPNESS           0x08U
#define UVC_PU_GAMMA               0x09U
#define UVC_PU_WHITE_BALANCE       0x0AU
#define UVC_PU_WHITE_BALANCE_AUTO  0x0BU
#define UVC_PU_AE_MODE             0x0CU
#define UVC_PU_AE_PRIORITY         0x0DU
#define UVC_PU_EXPOSURE_TIME_ABS   0x0EU
#define UVC_PU_FOCUS_ABS           0x12U
#define UVC_PU_FOCUS_AUTO          0x14U

#define UVC_SET_CUR              0x01U
#define UVC_GET_CUR              0x81U
#define UVC_GET_MIN              0x82U
#define UVC_GET_MAX              0x83U
#define UVC_GET_RES              0x84U
#define UVC_GET_LEN              0x85U
#define UVC_GET_INFO             0x86U
#define UVC_GET_DEF              0x87U

#define UVC_EP_INTERRUPT   0x01U
#define UVC_EP_ISOCHRONOUS 0x02U

#define QUANTA_POWER_D0 0U
#define QUANTA_POWER_D1 1U
#define QUANTA_POWER_D2 2U
#define QUANTA_POWER_D3 3U

struct quanta_frame_desc {
    uint32_t interval_min;
    uint32_t interval_max;
    uint32_t interval_step;
    uint32_t interval_default;
    uint32_t dwFrameInterval;
    uint16_t wWidth;
    uint16_t wHeight;
    uint32_t dwMinBitRate;
    uint32_t dwMaxBitRate;
    uint32_t dwMaxVideoFrameBufSize;
    uint32_t dwDefaultFrameInterval;
    uint8_t  bFrameIntervalType;
    uint32_t dwBytesPerLine;
};

struct quanta_stream {
    uint8_t  stream_index;
    uint8_t  bEndpointAddress;
    uint16_t wMaxPacketSize;
    uint8_t  bFormatIndex;
    uint8_t  bFrameIndex;
    uint32_t dwFrameInterval;
    uint8_t  running;
    uint32_t buffers[QUANTA_ISO_BUFS];
    uint32_t buffer_size;
    uint32_t buf_idx;
    uint32_t packets_received;
    uint32_t packets_dropped;
    uint32_t errors;
    uint8_t  frame_buffer[QUANTA_FRAME_BUF_SZ];
    uint32_t frame_size;
    uint8_t  frame_ready;
};

struct quanta_control {
    uint8_t  bControlSelector;
    uint8_t  bEntityId;
    uint8_t  bmControls;
    int32_t  val_min;
    int32_t  val_max;
    int32_t  val_res;
    int32_t  val_default;
    int32_t  val_current;
    uint8_t  autoupdate;
};

struct quanta_camera {
    uint8_t  bus;
    uint8_t  slot;
    uint8_t  func;
    uint16_t vendor_id;
    uint16_t product_id;
    uint16_t device_release;
    uint8_t  address;
    uint8_t  interface_ctrl;
    uint8_t  interface_stream;
    uint8_t  initialized;
    uint8_t  enabled;
    uint8_t  streaming;
    uint8_t  uvc_version_major;
    uint8_t  uvc_version_minor;
    uint32_t clock_frequency;
    uint8_t  bNumFormats;
    uint8_t  bNumFrames;
    uint32_t default_format;
    uint32_t default_frame;
    struct quanta_frame_desc frames[QUANTA_MAX_FRAMES];
    struct quanta_stream streams[QUANTA_MAX_STREAMS];
    uint8_t  stream_count;
    struct quanta_control ctrls[QUANTA_MAX_CTLS];
    uint8_t  ctrl_count;
    uint8_t  power_state;
    uint32_t frames_captured;
    uint32_t frames_dropped;
    uint32_t control_errors;
    uint32_t iso_errors;
    uint64_t bytes_received;
};

struct quanta_stats {
    uint32_t cameras_found;
    uint32_t cameras_init;
    uint32_t streams_started;
    uint32_t streams_stopped;
    uint32_t frames_captured;
    uint32_t frames_dropped;
    uint32_t controls_read;
    uint32_t controls_written;
    uint32_t control_errors;
    uint32_t iso_errors;
    uint32_t resets;
    uint64_t bytes_received;
};

#ifndef __cplusplus
int quanta_acer_hd_init(void);
void quanta_acer_hd_shutdown(void);
void quanta_acer_hd_poll(void);
int quanta_acer_hd_is_ready(void);

int quanta_acer_hd_camera_count(void);
int quanta_acer_hd_camera_info(uint8_t index, struct quanta_camera *out);

int quanta_acer_hd_stream_start(uint8_t camera_index, uint8_t stream_index,
                                 uint32_t width, uint32_t height, uint32_t fps);
int quanta_acer_hd_stream_stop(uint8_t camera_index, uint8_t stream_index);
int quanta_acer_hd_stream_pause(uint8_t camera_index, uint8_t stream_index);
int quanta_acer_hd_stream_resume(uint8_t camera_index, uint8_t stream_index);

int quanta_acer_hd_frame_ready(uint8_t camera_index, uint8_t stream_index);
int quanta_acer_hd_frame_read(uint8_t camera_index, uint8_t stream_index,
                               void *buf, uint32_t *size);
int quanta_acer_hd_snapshot(uint8_t camera_index, uint8_t stream_index,
                             void *buf, uint32_t *size);

int quanta_acer_hd_control_get(uint8_t camera_index, uint8_t ctrl_sel,
                                int32_t *value);
int quanta_acer_hd_control_set(uint8_t camera_index, uint8_t ctrl_sel,
                                int32_t value);
int quanta_acer_hd_control_range(uint8_t camera_index, uint8_t ctrl_sel,
                                  int32_t *min, int32_t *max,
                                  int32_t *res, int32_t *def);
int quanta_acer_hd_control_auto(uint8_t camera_index, uint8_t ctrl_sel,
                                 uint8_t enable);

int quanta_acer_hd_power_set_state(uint8_t camera_index, uint8_t power_state);
int quanta_acer_hd_power_get_state(uint8_t camera_index, uint8_t *power_state);
int quanta_acer_hd_reset(uint8_t camera_index);

void quanta_acer_hd_stats_get(struct quanta_stats *out);
void quanta_acer_hd_stats_reset(void);
#endif

#ifdef __cplusplus
class QuantaACERHD {
public:
    static QuantaACERHD* instance();
    void set_device_addr(uint8_t index, uint8_t addr);
    uint8_t is_ready(void);

    QuantaACERHD();
    ~QuantaACERHD();

    uint8_t probe(uint16_t vendor, uint16_t device);
    void init(void);
    void deinit(void);
    void poll(void);

    uint8_t camera_count(void);
    int camera_info(uint8_t index, struct quanta_camera *out);

    int stream_start(uint8_t camera_index, uint8_t stream_index,
                      uint32_t width, uint32_t height, uint32_t fps);
    int stream_stop(uint8_t camera_index, uint8_t stream_index);
    int stream_pause(uint8_t camera_index, uint8_t stream_index);
    int stream_resume(uint8_t camera_index, uint8_t stream_index);

    int frame_ready(uint8_t camera_index, uint8_t stream_index);
    int frame_read(uint8_t camera_index, uint8_t stream_index,
                    void *buf, uint32_t *size);
    int snapshot(uint8_t camera_index, uint8_t stream_index,
                  void *buf, uint32_t *size);

    int control_get(uint8_t camera_index, uint8_t ctrl_sel, int32_t *value);
    int control_set(uint8_t camera_index, uint8_t ctrl_sel, int32_t value);
    int control_range(uint8_t camera_index, uint8_t ctrl_sel,
                       int32_t *min, int32_t *max, int32_t *res, int32_t *def);
    int control_auto(uint8_t camera_index, uint8_t ctrl_sel, uint8_t enable);

    int power_set_state(uint8_t camera_index, uint8_t power_state);
    int power_get_state(uint8_t camera_index, uint8_t *power_state);
    int reset_camera(uint8_t camera_index);

    void stats_get(struct quanta_stats *out);
    void stats_reset(void);
    void debug_print_cameras(void);

private:
    int find_camera(uint8_t bus, uint8_t slot, uint8_t func);
    void scan_cameras(void);
    void init_camera(uint8_t index);
    void read_camera_desc(uint8_t index);
    void probe_uvc_ctrl_iface(uint8_t index);
    void probe_uvc_strm_iface(uint8_t index);
    void enumerate_frames(uint8_t index);
    void enumerate_controls(uint8_t index);
    int uvc_get_descriptor(uint8_t index, uint8_t type, uint8_t index_desc,
                            void *buf, uint16_t len);
    int uvc_set_cur(uint8_t index, uint8_t entity, uint8_t ctrl,
                     const void *data, uint16_t len);
    int uvc_get_cur(uint8_t index, uint8_t entity, uint8_t ctrl,
                     void *data, uint16_t len);
    int uvc_get_min(uint8_t index, uint8_t entity, uint8_t ctrl,
                     void *data, uint16_t len);
    int uvc_get_max(uint8_t index, uint8_t entity, uint8_t ctrl,
                     void *data, uint16_t len);
    int uvc_commit_probe(uint8_t index, uint8_t stream, uint8_t commit);
    int iso_submit(uint8_t index, uint8_t stream);
    int iso_reap(uint8_t index, uint8_t stream);
    void process_iso_packets(uint8_t index, uint8_t stream);
    void update_stats(uint8_t index);

    static QuantaACERHD* g_instance;
    struct quanta_camera cameras[QUANTA_MAX_CAMERAS];
    uint8_t camera_count_val;
    uint8_t initialized;
    struct quanta_stats stats;
};

#endif
#endif
