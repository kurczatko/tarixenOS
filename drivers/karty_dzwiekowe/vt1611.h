#ifndef VT1611_H
#define VT1611_H

#include <include/printf/types.h>

#define VT1611_VENDOR_ID 0x1106
#define VT1611_DEVICE_ID 0x3182

#define VT1611_MAX_WIDGETS 64
#define VT1611_MAX_PINS 20
#define VT1611_MAX_STREAMS 8

#define HDA_VERB_GET_PARAMETER 0xF00
#define HDA_VERB_GET_CONNECTION_LIST 0xF02
#define HDA_VERB_GET_CONNECTION_SELECT 0xF01
#define HDA_VERB_SET_CONNECTION_SELECT 0x701
#define HDA_VERB_GET_AMP_GAIN_MUTE 0xB00
#define HDA_VERB_SET_AMP_GAIN_MUTE 0x300
#define HDA_VERB_GET_CONVERTER_FORMAT 0xA00
#define HDA_VERB_SET_CONVERTER_FORMAT 0x200
#define HDA_VERB_GET_PIN_WIDGET_CONTROL 0xF07
#define HDA_VERB_SET_PIN_WIDGET_CONTROL 0x707
#define HDA_VERB_GET_UNSOLICITED_RESPONSE 0xF08
#define HDA_VERB_SET_UNSOLICITED_RESPONSE 0x708
#define HDA_VERB_GET_PIN_SENSE 0xF09
#define HDA_VERB_GET_CONFIGURATION_DEFAULT 0xF1C
#define HDA_VERB_SET_CONFIGURATION_DEFAULT 0x71C
#define HDA_VERB_GET_POWER_STATE 0xF05
#define HDA_VERB_SET_POWER_STATE 0x705
#define HDA_VERB_GET_CONVERTER_STREAM_CHANNEL 0xF06
#define HDA_VERB_SET_CONVERTER_STREAM_CHANNEL 0x706
#define HDA_VERB_GET_EAPD_BTL_ENABLE 0xF0C
#define HDA_VERB_SET_EAPD_BTL_ENABLE 0x70C
#define HDA_VERB_GET_DIGITAL_CONVERTER_1 0xF0D
#define HDA_VERB_SET_DIGITAL_CONVERTER_1 0x70D
#define HDA_VERB_GET_DIGITAL_CONVERTER_2 0xF0E
#define HDA_VERB_SET_DIGITAL_CONVERTER_2 0x70E
#define HDA_VERB_GET_VOLUME_KNOB 0xF0F
#define HDA_VERB_SET_VOLUME_KNOB 0x70F
#define HDA_VERB_GET_SUBSYSTEM_ID 0xF20
#define HDA_VERB_SET_SUBSYSTEM_ID 0x720

#define PARAM_VENDOR_ID 0x00
#define PARAM_REVISION_ID 0x01
#define PARAM_SUBORDINATE_NODE_COUNT 0x04
#define PARAM_FUNCTION_GROUP_TYPE 0x05
#define PARAM_AUDIO_WIDGET_CAP 0x09
#define PARAM_PCAP 0x0A
#define PARAM_CONNECTION_LIST_LEN 0x0E
#define PARAM_SUPPORTED_PCM_SIZE_RATE 0x0A
#define PARAM_SUPPORTED_STREAM_FORMATS 0x0B
#define PARAM_PIN_CAP 0x0C
#define PARAM_AMP_IN_CAP 0x0D
#define PARAM_AMP_OUT_CAP 0x12
#define PARAM_CONNECTION_LIST 0x0F
#define PARAM_PROCESSING_CAP 0x10
#define PARAM_GPIO_CAP 0x11
#define PARAM_VOLUME_KNOB_CAP 0x13

#define WIDGET_TYPE_OUTPUT 0x0
#define WIDGET_TYPE_INPUT 0x1
#define WIDGET_TYPE_MIXER 0x2
#define WIDGET_TYPE_SELECTOR 0x3
#define WIDGET_TYPE_PIN_COMPLEX 0x4
#define WIDGET_TYPE_POWER_WIDGET 0x5
#define WIDGET_TYPE_VOLUME_KNOB 0x6
#define WIDGET_TYPE_BEEP_GENERATOR 0x7
#define WIDGET_TYPE_VENDOR_DEFINED 0xF

#define PIN_CAP_IMPEDANCE_SENSE (1 << 0)
#define PIN_CAP_TRIGGER_REQUIRED (1 << 1)
#define PIN_CAP_PRESENCE_DETECT (1 << 2)
#define PIN_CAP_HEADPHONE_DRIVE (1 << 3)
#define PIN_CAP_OUTPUT_CAP (1 << 4)
#define PIN_CAP_INPUT_CAP (1 << 5)
#define PIN_CAP_BALANCED_IO (1 << 6)
#define PIN_CAP_HDMI (1 << 7)
#define PIN_CAP_VREF_HIZ (1 << 8)
#define PIN_CAP_VREF_50 (1 << 9)
#define PIN_CAP_VREF_GND (1 << 10)
#define PIN_CAP_VREF_80 (1 << 11)
#define PIN_CAP_VREF_100 (1 << 12)
#define PIN_CAP_EAPD (1 << 13)
#define PIN_CAP_DP (1 << 14)
#define PIN_CAP_HBR (1 << 15)

#define PIN_CTL_VREF_HIZ (0 << 0)
#define PIN_CTL_VREF_50 (1 << 0)
#define PIN_CTL_VREF_GND (2 << 0)
#define PIN_CTL_VREF_80 (3 << 0)
#define PIN_CTL_VREF_100 (4 << 0)
#define PIN_CTL_VREF_MASK (7 << 0)
#define PIN_CTL_IN_EN (1 << 4)
#define PIN_CTL_OUT_EN (1 << 5)
#define PIN_CTL_HP_EN (1 << 6)

#define AMP_GAIN_MUTE_MUTE (1 << 7)
#define AMP_GAIN_MUTE_GAIN_MASK 0x7F

#define POWER_STATE_D0 0x0
#define POWER_STATE_D1 0x1
#define POWER_STATE_D2 0x2
#define POWER_STATE_D3 0x3

#define PORT_CONNECTIVITY_JACK (0 << 30)
#define PORT_CONNECTIVITY_NO_PHYSICAL (1 << 30)
#define PORT_CONNECTIVITY_FIXED (2 << 30)
#define PORT_CONNECTIVITY_BOTH (3 << 30)

#define LOCATION_EXTERNAL 0x0
#define LOCATION_INTERNAL 0x1
#define LOCATION_SEPARATE 0x2
#define LOCATION_OTHER 0x3

#define DEFAULT_DEVICE_LINE_OUT 0x0
#define DEFAULT_DEVICE_SPEAKER 0x1
#define DEFAULT_DEVICE_HP_OUT 0x2
#define DEFAULT_DEVICE_CD 0x3
#define DEFAULT_DEVICE_SPDIF_OUT 0x4
#define DEFAULT_DEVICE_DIGITAL_OTHER 0x5
#define DEFAULT_DEVICE_MODEM_LINE 0x6
#define DEFAULT_DEVICE_MODEM_HANDSET 0x7
#define DEFAULT_DEVICE_LINE_IN 0x8
#define DEFAULT_DEVICE_AUX 0x9
#define DEFAULT_DEVICE_MIC_IN 0xA
#define DEFAULT_DEVICE_TELEPHONY 0xB
#define DEFAULT_DEVICE_SPDIF_IN 0xC
#define DEFAULT_DEVICE_DIGITAL_IN 0xD
#define DEFAULT_DEVICE_RESERVED 0xE
#define DEFAULT_DEVICE_OTHER 0xF

#define CONNECTION_TYPE_UNKNOWN 0x0
#define CONNECTION_TYPE_1_8_JACK 0x1
#define CONNECTION_TYPE_1_4_JACK 0x2
#define CONNECTION_TYPE_ATAPI 0x3
#define CONNECTION_TYPE_RCA 0x4
#define CONNECTION_TYPE_OPTICAL 0x5
#define CONNECTION_TYPE_OTHER_DIGITAL 0x6
#define CONNECTION_TYPE_OTHER_ANALOG 0x7
#define CONNECTION_TYPE_MULTICHANNEL 0x8
#define CONNECTION_TYPE_XLR 0x9
#define CONNECTION_TYPE_RJ11 0xA
#define CONNECTION_TYPE_COMBINATION 0xB
#define CONNECTION_TYPE_OTHER 0xF

#define COLOR_UNKNOWN 0x0
#define COLOR_BLACK 0x1
#define COLOR_GREY 0x2
#define COLOR_BLUE 0x3
#define COLOR_GREEN 0x4
#define COLOR_RED 0x5
#define COLOR_ORANGE 0x6
#define COLOR_YELLOW 0x7
#define COLOR_PURPLE 0x8
#define COLOR_PINK 0x9
#define COLOR_WHITE 0xA
#define COLOR_OTHER 0xF

#define DEF_ASSOC_NONE 0x0
#define DEF_ASSOC_1 0x1
#define DEF_ASSOC_2 0x2
#define DEF_ASSOC_3 0x3
#define DEF_ASSOC_4 0x4
#define DEF_ASSOC_5 0x5
#define DEF_ASSOC_6 0x6
#define DEF_ASSOC_7 0x7
#define DEF_ASSOC_8 0x8
#define DEF_ASSOC_9 0x9
#define DEF_ASSOC_A 0xA
#define DEF_ASSOC_B 0xB
#define DEF_ASSOC_C 0xC
#define DEF_ASSOC_D 0xD
#define DEF_ASSOC_E 0xE
#define DEF_ASSOC_F 0xF

#define HDA_GCAP 0x00
#define HDA_VMIN 0x02
#define HDA_VMAJ 0x03
#define HDA_OUTPAY 0x04
#define HDA_INPAY 0x06
#define HDA_GCTL 0x08
#define HDA_WAKEEN 0x0C
#define HDA_STATESTS 0x0E
#define HDA_GSTS 0x10
#define HDA_IC 0x60
#define HDA_IR 0x64
#define HDA_ICS 0x68
#define HDA_WALLCLK 0x30
#define HDA_SSYNC 0x34
#define HDA_CORBLBASE 0x40
#define HDA_CORBUBASE 0x44
#define HDA_CORBWP 0x48
#define HDA_CORBRP 0x4A
#define HDA_CORBCTL 0x4C
#define HDA_CORBSTS 0x4D
#define HDA_CORBSIZE 0x4E
#define HDA_RIRBLBASE 0x50
#define HDA_RIRBUBASE 0x54
#define HDA_RIRBWP 0x58
#define HDA_RIRBRP 0x5A
#define HDA_RIRBCTL 0x5C
#define HDA_RIRBSTS 0x5D
#define HDA_RIRBSIZE 0x5E

#define SD_CTL_STREAM_RESET (1 << 0)
#define SD_CTL_DMA_START (1 << 1)
#define SD_CTL_IOCE (1 << 2)
#define SD_CTL_FEIE (1 << 3)
#define SD_CTL_DEIE (1 << 4)
#define SD_CTL_RUN (1 << 1)

#define SD_STS_FIFORDY (1 << 0)
#define SD_STS_DESE (1 << 2)
#define SD_STS_FIFOE (1 << 3)
#define SD_STS_BCIS (1 << 4)

struct vt1611_widget {
    uint8_t nid;
    uint8_t type;
    uint32_t capabilities;
    uint16_t connection_count;
    uint8_t connections[32];
    uint8_t selected_connection;
    uint32_t pin_capabilities;
    uint32_t config_default;
    uint8_t pin_control;
    uint8_t power_state;
    uint8_t amp_out_mute[4];
    uint8_t amp_out_gain[4];
    uint8_t amp_in_mute[4];
    uint8_t amp_in_gain[4];
    uint32_t conv_format;
    uint8_t stream_tag;
    uint8_t channel_id;
    uint8_t eapd_btl;
    uint32_t digital_conv1;
    uint32_t digital_conv2;
    uint16_t vendor_id;
    uint16_t revision_id;
};

struct vt1611_pin_config {
    uint8_t nid;
    uint32_t config_default;
    uint8_t sequence;
    uint8_t default_association;
    uint8_t misc;
    uint8_t color;
    uint8_t connection_type;
    uint8_t location;
    uint8_t port_connectivity;
    uint8_t default_device;
};

struct vt1611_stream {
    uint8_t stream_tag;
    uint8_t channel_count;
    uint32_t format;
    uint32_t rate;
    uint16_t bits_per_sample;
    uint8_t running;
    uint32_t *bdpl_addr;
    uint32_t bdl_entries;
    uint32_t buffer_size;
    uint32_t period_size;
    void *dma_buffer;
    uint32_t pos;
};

struct vt1611_mixer_control {
    uint8_t nid;
    uint8_t index;
    uint8_t direction;
    int32_t min_gain;
    int32_t max_gain;
    int32_t current_gain;
    uint8_t mute;
    char name[32];
};

struct vt1611_stats {
    uint32_t verbs_sent;
    uint32_t verbs_failed;
    uint32_t interrupts;
    uint32_t buffer_underruns;
    uint32_t buffer_overruns;
    uint32_t jack_changes;
    uint32_t spdif_packets;
    uint32_t spdif_errors;
};

class VT1611 {
public:
    static VT1611* instance();

    uint8_t probe(uint16_t vendor, uint16_t device);
    void init(void);
    void deinit(void);
    void irq_handler(void);

    int pcm_open(uint16_t channels, uint32_t rate, uint16_t format, uint8_t playback);
    int pcm_write(const void* data, uint32_t bytes);
    int pcm_read(void* data, uint32_t bytes);
    void pcm_close(uint8_t playback);
    int pcm_pause(uint8_t playback, uint8_t pause);
    int pcm_resume(uint8_t playback);

    int mixer_get_control_count(void);
    int mixer_get_control(uint8_t index, struct vt1611_mixer_control* ctrl);
    int mixer_set_volume(uint8_t index, int32_t gain);
    int mixer_set_mute(uint8_t index, uint8_t mute);
    int mixer_get_volume(uint8_t index, int32_t* gain);
    int mixer_get_mute(uint8_t index, uint8_t* mute);

    int jack_get_count(void);
    int jack_get_status(uint8_t index, uint8_t* present);
    int jack_get_config(uint8_t index, struct vt1611_pin_config* config);
    int jack_enable_unsolicited(uint8_t index, uint8_t enable);

    int spdif_open(uint32_t rate, uint16_t format);
    int spdif_write(const void* data, uint32_t bytes);
    void spdif_close(void);
    int spdif_set_category(uint8_t category);
    int spdif_set_copyright(uint8_t copyright);
    int spdif_set_nonaudio(uint8_t nonaudio);
    int spdif_set_preemphasis(uint8_t preemphasis);

    void set_master_volume(uint8_t percent);
    uint8_t get_master_volume(void);

    void power_set_state(uint8_t state);
    uint8_t power_get_state(void);
    void widget_power_set(uint8_t nid, uint8_t state);

    void stats_get(struct vt1611_stats* out);
    void debug_print_widgets(void);

private:
    VT1611();
    ~VT1611();

    uint32_t send_verb(uint32_t verb);
    uint32_t send_verb_nid(uint8_t nid, uint32_t verb_cmd, uint32_t payload);
    int find_codec_address(void);
    void discover_widgets(void);
    void parse_pin_configs(void);
    void setup_default_pins(void);
    void configure_pin(uint8_t nid, uint32_t config);
    int find_dac_for_pin(uint8_t pin_nid);
    int find_adc_for_pin(uint8_t pin_nid);
    void setup_stream(uint8_t stream_tag, uint8_t channels, uint32_t rate, uint16_t format, uint8_t playback);
    void start_stream(uint8_t stream_index, uint8_t playback);
    void stop_stream(uint8_t stream_index, uint8_t playback);
    void alloc_dma_buffer(struct vt1611_stream* stream, uint32_t size);
    void free_dma_buffer(struct vt1611_stream* stream);
    void setup_bdl(struct vt1611_stream* stream);
    void mixer_init(void);
    void jack_init(void);
    void spdif_init(void);
    void power_init(void);
    void unsolicited_enable(uint8_t nid, uint8_t enable);
    void widget_amp_set(uint8_t nid, uint8_t direction, uint8_t index, uint8_t mute, uint8_t gain);
    void parse_config_default(uint32_t config, struct vt1611_pin_config* out);
    int find_widget_by_nid(uint8_t nid);
    void delay_us(uint32_t us);
    void delay_ms(uint32_t ms);

    static VT1611* g_instance;
    volatile uint8_t* hda_base;
    uint8_t codec_address;
    uint8_t initialized;
    uint8_t widget_count;
    struct vt1611_widget widgets[VT1611_MAX_WIDGETS];
    uint8_t pin_count;
    uint8_t pin_nids[VT1611_MAX_PINS];
    uint8_t dac_count;
    uint8_t dac_nids[VT1611_MAX_PINS];
    uint8_t adc_count;
    uint8_t adc_nids[VT1611_MAX_PINS];
    uint8_t stream_count;
    struct vt1611_stream streams[VT1611_MAX_STREAMS];
    uint8_t mixer_count;
    struct vt1611_mixer_control mixer_controls[64];
    uint8_t jack_count;
    uint8_t jack_nids[VT1611_MAX_PINS];
    struct vt1611_stats stats;
    uint8_t master_volume;
    uint8_t power_state;
};

extern "C" void vt1611_irq(void);

#endif