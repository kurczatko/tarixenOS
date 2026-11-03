#ifndef HDA_CODEC_H
#define HDA_CODEC_H

#include "../../include/printf/types.h"

#define HDA_CODEC_VENDOR_ID 0x8086
#define HDA_CODEC_DEVICE_ID 0x9DC8

#define HDA_CODEC_MAX_WIDGETS 64
#define HDA_CODEC_MAX_PINS 24
#define HDA_CODEC_MAX_STREAMS 16
#define HDA_CODEC_MAX_COMPONENTS 32
#define HDA_CODEC_MAX_PIPELINES 8

#define HDA_CODEC_MAILBOX_OFFSET 0xE0
#define HDA_CODEC_DOORBELL_OFFSET 0xF0
#define HDA_CODEC_BUFFER_SIZE 4096

#define HDA_CODEC_CMD_GLB 0x100
#define HDA_CODEC_CMD_COMP 0x200
#define HDA_CODEC_CMD_PIPE 0x300
#define HDA_CODEC_CMD_STREAM 0x400
#define HDA_CODEC_CMD_FW_LOAD 0x500
#define HDA_CODEC_CMD_GET_INFO 0x600

#define HDA_CODEC_PCM_PARAMS 0x01
#define HDA_CODEC_PCM_PREPARE 0x02
#define HDA_CODEC_PCM_TRIGGER 0x03
#define HDA_CODEC_PCM_FREE 0x04
#define HDA_CODEC_PCM_DRAIN 0x05
#define HDA_CODEC_PCM_STOP 0x06

#define HDA_CODEC_TRIGGER_START 1
#define HDA_CODEC_TRIGGER_STOP 2
#define HDA_CODEC_TRIGGER_PAUSE 3
#define HDA_CODEC_TRIGGER_RELEASE 4

#define HDA_CODEC_COMP_HOST 0x01
#define HDA_CODEC_COMP_DAI 0x02
#define HDA_CODEC_COMP_ASRC 0x03
#define HDA_CODEC_COMP_MIXER 0x04
#define HDA_CODEC_COMP_SELECTOR 0x05
#define HDA_CODEC_COMP_MUX 0x06
#define HDA_CODEC_COMP_VOLUME 0x07
#define HDA_CODEC_COMP_SWITCH 0x08

#define HDA_CODEC_PIPE_PLAYBACK 0x01
#define HDA_CODEC_PIPE_CAPTURE 0x02

#define HDA_HDA_GCTL 0x08
#define HDA_HDA_STATESTS 0x0E
#define HDA_HDA_GSTS 0x10
#define HDA_HDA_IC 0x60
#define HDA_HDA_IR 0x64
#define HDA_HDA_ICS 0x68

#define HDA_VERB_GET_PARAMETER 0xF00
#define HDA_VERB_SET_CONVERTER_FORMAT 0x200
#define HDA_VERB_SET_PIN_WIDGET_CONTROL 0x707
#define HDA_VERB_GET_PIN_SENSE 0xF09
#define HDA_VERB_GET_CONFIGURATION_DEFAULT 0xF1C
#define HDA_VERB_SET_POWER_STATE 0x705
#define HDA_VERB_SET_CONVERTER_STREAM_CHANNEL 0x706

#define PARAM_VENDOR_ID 0x00
#define PARAM_SUBORDINATE_NODE_COUNT 0x04
#define PARAM_AUDIO_WIDGET_CAP 0x09
#define PARAM_PIN_CAP 0x0C

#define PIN_CAP_PRESENCE_DETECT (1 << 2)
#define PIN_CAP_HDMI (1 << 7)
#define PIN_CAP_VREF_HIZ (1 << 8)

#define PIN_CTL_OUT_EN (1 << 5)
#define PIN_CTL_IN_EN (1 << 4)
#define PIN_CTL_VREF_HIZ (0 << 0)

#define POWER_STATE_D0 0x0
#define POWER_STATE_D3 0x3

#define SD_CTL_STREAM_RESET (1 << 0)
#define SD_CTL_DMA_START (1 << 1)
#define SD_CTL_IOCE (1 << 2)

struct hda_codec_widget {
    uint8_t nid;
    uint8_t type;
    uint32_t capabilities;
    uint16_t connection_count;
    uint8_t connections[32];
    uint32_t pin_capabilities;
    uint32_t config_default;
    uint8_t pin_control;
    uint8_t power_state;
    uint8_t stream_tag;
    uint8_t channel_id;
};

struct hda_codec_pin_config {
    uint8_t nid;
    uint32_t config_default;
    uint8_t sequence;
    uint8_t default_association;
    uint8_t color;
    uint8_t connection_type;
    uint8_t location;
    uint8_t port_connectivity;
    uint8_t default_device;
};

struct hda_codec_stream {
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
    uint32_t pipeline_id;
};

struct hda_codec_component {
    uint32_t component_id;
    uint8_t type;
    uint8_t pipeline_id;
    uint32_t config;
    uint8_t active;
};

struct hda_codec_pipeline {
    uint32_t pipeline_id;
    uint8_t type;
    uint8_t component_count;
    uint32_t component_ids[HDA_CODEC_MAX_COMPONENTS];
    uint8_t active;
    uint32_t rate;
    uint8_t channels;
    uint16_t format;
};

struct hda_codec_stats {
    uint32_t verbs_sent;
    uint32_t verbs_failed;
    uint32_t interrupts;
    uint32_t ipc_messages;
    uint32_t ipc_errors;
    uint32_t buffer_underruns;
    uint32_t buffer_overruns;
    uint32_t jack_changes;
    uint32_t spdif_packets;
    uint32_t spdif_errors;
    uint32_t firmware_loads;
};

class HDAudioCodec {
public:
    static HDAudioCodec* instance();

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
    int mixer_get_control(uint8_t index, void* ctrl);
    int mixer_set_volume(uint8_t index, int32_t gain);
    int mixer_set_mute(uint8_t index, uint8_t mute);
    int mixer_get_volume(uint8_t index, int32_t* gain);
    int mixer_get_mute(uint8_t index, uint8_t* mute);

    int jack_get_count(void);
    int jack_get_status(uint8_t index, uint8_t* present);
    int jack_get_config(uint8_t index, struct hda_codec_pin_config* config);
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

    void stats_get(struct hda_codec_stats* out);
    void debug_print_widgets(void);

    int firmware_load(const void* firmware, uint32_t size);
    int ipc_send(uint32_t cmd, const void* data, uint32_t size);
    int ipc_receive(void* data, uint32_t size, uint32_t timeout);
    int pipeline_create(uint8_t type, uint32_t* pipeline_id);
    int pipeline_destroy(uint32_t pipeline_id);
    int pipeline_start(uint32_t pipeline_id);
    int pipeline_stop(uint32_t pipeline_id);

private:
    HDAudioCodec();
    ~HDAudioCodec();

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
    void alloc_dma_buffer(struct hda_codec_stream* stream, uint32_t size);
    void free_dma_buffer(struct hda_codec_stream* stream);
    void setup_bdl(struct hda_codec_stream* stream);
    void mixer_init(void);
    void jack_init(void);
    void spdif_init(void);
    void power_init(void);
    void unsolicited_enable(uint8_t nid, uint8_t enable);
    void ipc_init(void);
    void ipc_doorbell_ring(void);
    int ipc_wait_response(uint32_t timeout);
    void parse_components(void);
    void parse_pipelines(void);

    static HDAudioCodec* g_instance;
    volatile uint8_t* hda_base;
    volatile uint8_t* ipc_base;
    uint8_t codec_address;
    uint8_t initialized;
    uint8_t widget_count;
    struct hda_codec_widget widgets[HDA_CODEC_MAX_WIDGETS];
    uint8_t pin_count;
    uint8_t pin_nids[HDA_CODEC_MAX_PINS];
    uint8_t dac_count;
    uint8_t dac_nids[HDA_CODEC_MAX_PINS];
    uint8_t adc_count;
    uint8_t adc_nids[HDA_CODEC_MAX_PINS];
    uint8_t stream_count;
    struct hda_codec_stream streams[HDA_CODEC_MAX_STREAMS];
    uint8_t mixer_count;
    uint32_t mixer_nids[64];
    int32_t mixer_volumes[64];
    uint8_t mixer_mutes[64];
    char mixer_names[64][32];
    uint8_t jack_count;
    uint8_t jack_nids[HDA_CODEC_MAX_PINS];
    struct hda_codec_stats stats;
    uint8_t master_volume;
    uint8_t power_state;
    uint8_t firmware_loaded;
    uint32_t component_count;
    struct hda_codec_component components[HDA_CODEC_MAX_COMPONENTS];
    uint32_t pipeline_count;
    struct hda_codec_pipeline pipelines[HDA_CODEC_MAX_PIPELINES];
    uint32_t ipc_msg_id;
};

extern "C" void hda_codec_irq(void);

#endif