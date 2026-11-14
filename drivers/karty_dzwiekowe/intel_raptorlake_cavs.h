#ifndef INTEL_RAPTORLAKE_CAVS_H
#define INTEL_RAPTORLAKE_CAVS_H

#include <include/printf/types.h>

#define INTEL_CAVS_VENDOR_ID      0x8086U
#define INTEL_CAVS_DEVICE_RAPTORLAKE 0x7A50U
#define INTEL_CAVS_DEVICE_ADL       0x51C8U
#define INTEL_CAVS_DEVICE_TGL       0xA0C8U

#define INTEL_CAVS_MAX_WIDGETS    64U
#define INTEL_CAVS_MAX_PINS       24U
#define INTEL_CAVS_MAX_STREAMS    16U
#define INTEL_CAVS_MAX_MIXERS     64U

#define HDA_GCAP        0x00U
#define HDA_VMIN        0x02U
#define HDA_VMAJ        0x03U
#define HDA_OUTPAY      0x04U
#define HDA_INPAY       0x06U
#define HDA_GCTL        0x08U
#define HDA_WAKEEN      0x0CU
#define HDA_STATESTS    0x0EU
#define HDA_GSTS        0x10U
#define HDA_IC          0x60U
#define HDA_IR          0x64U
#define HDA_ICS         0x68U
#define HDA_WALLCLK     0x30U
#define HDA_CORBLBASE   0x40U
#define HDA_CORBUBASE   0x44U
#define HDA_CORBWP      0x48U
#define HDA_CORBRP      0x4AU
#define HDA_CORBCTL     0x4CU
#define HDA_CORBSTS     0x4DU
#define HDA_CORBSIZE    0x4EU
#define HDA_RIRBLBASE   0x50U
#define HDA_RIRBUBASE   0x54U
#define HDA_RIRBWP      0x58U
#define HDA_RIRBRP      0x5AU
#define HDA_RIRBCTL     0x5CU
#define HDA_RIRBSTS     0x5DU
#define HDA_RIRBSIZE    0x5EU

#define SD_OFFSET       0x80U
#define SD_CTL          0x00U
#define SD_STS          0x01U
#define SD_LPIB         0x02U
#define SD_CBL          0x04U
#define SD_LVI          0x06U
#define SD_FIFOW        0x07U
#define SD_FIFOS        0x08U
#define SD_FORMAT       0x0AU
#define SD_BDLPL        0x18U
#define SD_BDLPU        0x1CU

#define SD_CTL_STREAM_RESET   (1U << 0U)
#define SD_CTL_DMA_START      (1U << 1U)
#define SD_CTL_IOCE           (1U << 2U)
#define SD_CTL_FEIE           (1U << 3U)
#define SD_CTL_DEIE           (1U << 4U)
#define SD_CTL_RUN            (1U << 1U)

#define SD_STS_FIFORDY        (1U << 0U)
#define SD_STS_DESE           (1U << 2U)
#define SD_STS_FIFOE          (1U << 3U)
#define SD_STS_BCIS           (1U << 4U)

#define CAVS_DSP_BASE         0x1000U
#define CAVS_DSP_REV          0x00U
#define CAVS_DSP_FW_STATUS    0x04U
#define CAVS_DSP_FW_CMD       0x08U
#define CAVS_DSP_FW_DATA      0x0CU
#define CAVS_DSP_IPC_IMR      0x10U
#define CAVS_DSP_IPC_ISR      0x14U
#define CAVS_DSP_IPC_IER      0x18U
#define CAVS_DSP_IPC_IER2     0x1CU
#define CAVS_DSP_IPC0         0x20U
#define CAVS_DSP_IPC1         0x24U
#define CAVS_DSP_IPC2         0x28U
#define CAVS_DSP_IPC3         0x2CU
#define CAVS_DSP_IPC4         0x30U
#define CAVS_DSP_IPC5         0x34U
#define CAVS_DSP_IPC6         0x38U
#define CAVS_DSP_IPC7         0x3CU
#define CAVS_DSP_CLK_CTL      0x40U
#define CAVS_DSP_SRAM_CTL     0x44U
#define CAVS_DSP_TRACE_CTL    0x48U
#define CAVS_DSP_TRACE_DATA   0x4CU

#define CAVS_FW_RUNNING       (1U << 0U)
#define CAVS_FW_INIT_DONE     (1U << 1U)
#define CAVS_FW_ERROR         (1U << 7U)

#define WIDGET_TYPE_OUTPUT    0U
#define WIDGET_TYPE_INPUT     1U
#define WIDGET_TYPE_MIXER     2U
#define WIDGET_TYPE_SELECTOR  3U
#define WIDGET_TYPE_PIN       4U

#define PARAM_VENDOR_ID       0x00U
#define PARAM_REVISION_ID     0x01U
#define PARAM_SUBNODE_COUNT   0x04U
#define PARAM_AUDIO_WIDGET_CAP 0x09U
#define PARAM_PCAP            0x0AU
#define PARAM_CONNECTION_LEN  0x0EU
#define PARAM_SUPPORTED_PCM   0x0AU
#define PARAM_STREAM_FORMATS  0x0BU
#define PARAM_PIN_CAP         0x0CU
#define PARAM_AMP_IN_CAP      0x0DU
#define PARAM_AMP_OUT_CAP     0x12U
#define PARAM_CONNECTION_LIST 0x0FU
#define PARAM_PROCESSING_CAP  0x10U
#define PARAM_GPIO_CAP        0x11U

#define HDA_VERB_GET_PARAMETER      0xF00U
#define HDA_VERB_SET_CONNECTION_SELECT 0x701U
#define HDA_VERB_GET_CONNECTION_SELECT 0xF01U
#define HDA_VERB_GET_CONNECTION_LIST   0xF02U
#define HDA_VERB_SET_AMP_GAIN_MUTE     0x300U
#define HDA_VERB_GET_AMP_GAIN_MUTE     0xB00U
#define HDA_VERB_SET_CONV_FORMAT       0x200U
#define HDA_VERB_GET_CONV_FORMAT       0xA00U
#define HDA_VERB_SET_PIN_CTL           0x707U
#define HDA_VERB_GET_PIN_CTL           0xF07U
#define HDA_VERB_SET_UNSOLICITED       0x708U
#define HDA_VERB_GET_PIN_SENSE         0xF09U
#define HDA_VERB_GET_CONFIG_DEFAULT    0xF1CU
#define HDA_VERB_SET_CONFIG_DEFAULT    0x71CU
#define HDA_VERB_GET_POWER_STATE       0xF05U
#define HDA_VERB_SET_POWER_STATE       0x705U
#define HDA_VERB_SET_CONV_STREAM_CH    0x706U
#define HDA_VERB_GET_CONV_STREAM_CH    0xF06U

#define AMP_MUTE            (1U << 7U)
#define AMP_GAIN_MASK       0x7FU

#define POWER_D0 0U
#define POWER_D1 1U
#define POWER_D2 2U
#define POWER_D3 3U

#define PIN_CAP_HDMI   (1U << 7U)
#define PIN_CAP_VREF_100 (1U << 12U)
#define PIN_CAP_PRESENCE_DETECT (1U << 2U)

#define DEFAULT_DEVICE_LINE_OUT 0x0
#define DEFAULT_DEVICE_SPEAKER 0x1
#define DEFAULT_DEVICE_HP_OUT 0x2
#define DEFAULT_DEVICE_CD 0x3
#define DEFAULT_DEVICE_SPDIF_OUT 0x4
#define DEFAULT_DEVICE_DIGITAL_OTHER 0x5
#define DEFAULT_DEVICE_LINE_IN 0x8
#define DEFAULT_DEVICE_AUX 0x9
#define DEFAULT_DEVICE_MIC_IN 0xA
#define DEFAULT_DEVICE_TELEPHONY 0xB
#define DEFAULT_DEVICE_SPDIF_IN 0xC
#define DEFAULT_DEVICE_DIGITAL_IN 0xD
#define DEFAULT_DEVICE_RESERVED 0xE
#define DEFAULT_DEVICE_OTHER 0xF

#define PIN_CAP_PRESENCE_DETECT (1U << 2U)

#define PIN_CTL_IN_EN   (1U << 4U)
#define PIN_CTL_OUT_EN  (1U << 5U)

struct cavs_widget {
    uint8_t nid;
    uint8_t type;
    uint32_t capabilities;
    uint16_t connection_count;
    uint8_t connections[32U];
    uint8_t selected_connection;
    uint32_t pin_capabilities;
    uint32_t config_default;
    uint8_t pin_control;
    uint8_t power_state;
    uint8_t amp_out_mute[4U];
    uint8_t amp_out_gain[4U];
    uint8_t amp_in_mute[4U];
    uint8_t amp_in_gain[4U];
    uint32_t conv_format;
    uint8_t stream_tag;
    uint8_t channel_id;
    uint8_t eapd_btl;
    uint32_t digital_conv1;
    uint32_t digital_conv2;
    uint16_t vendor_id;
    uint16_t revision_id;
};

struct cavs_pin_config {
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

struct cavs_stream {
    uint8_t stream_tag;
    uint8_t channel_count;
    uint32_t format;
    uint32_t rate;
    uint16_t bits_per_sample;
    uint8_t running;
    volatile uint32_t* bdpl_addr;
    uint32_t bdl_entries;
    uint32_t buffer_size;
    uint32_t period_size;
    void *dma_buffer;
    uint32_t pos;
};

struct cavs_mixer_control {
    uint8_t nid;
    uint8_t index;
    uint8_t direction;
    int32_t min_gain;
    int32_t max_gain;
    int32_t current_gain;
    uint8_t mute;
    char name[32U];
};

struct cavs_stats {
    uint32_t verbs_sent;
    uint32_t verbs_failed;
    uint32_t interrupts;
    uint32_t buffer_underruns;
    uint32_t buffer_overruns;
    uint32_t jack_changes;
    uint32_t dsp_ipc_messages;
    uint32_t dsp_fw_errors;
};

#ifdef __cplusplus
class IntelRaptorLakeCAVS {
public:
    static IntelRaptorLakeCAVS* instance();
    void set_hda_base(volatile uint8_t* base);
    uint8_t is_initialized(void);

    IntelRaptorLakeCAVS();
    ~IntelRaptorLakeCAVS();

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
    int mixer_get_control(uint8_t index, struct cavs_mixer_control* ctrl);
    int mixer_set_volume(uint8_t index, int32_t gain);
    int mixer_set_mute(uint8_t index, uint8_t mute);
    int mixer_get_volume(uint8_t index, int32_t* gain);
    int mixer_get_mute(uint8_t index, uint8_t* mute);

    int jack_get_count(void);
    int jack_get_status(uint8_t index, uint8_t* present);
    int jack_get_config(uint8_t index, struct cavs_pin_config* config);
    int jack_enable_unsolicited(uint8_t index, uint8_t enable);

    int dsp_fw_load(const void* fw_data, uint32_t fw_size);
    int dsp_fw_start(void);
    int dsp_ipc_send(uint32_t ipc0, uint32_t ipc1, uint32_t ipc2, uint32_t ipc3);
    int dsp_ipc_receive(uint32_t* ipc0, uint32_t* ipc1, uint32_t* ipc2, uint32_t* ipc3);
    int dsp_ready(void);
    int dsp_reset(void);
    int dsp_clock_enable(uint8_t enable);

    void set_master_volume(uint8_t percent);
    uint8_t get_master_volume(void);

    void power_set_state(uint8_t state);
    uint8_t power_get_state(void);
    void widget_power_set(uint8_t nid, uint8_t state);

    void stats_get(struct cavs_stats* out);
    void debug_print_widgets(void);

private:
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
    void alloc_dma_buffer(struct cavs_stream* stream, uint32_t size);
    void free_dma_buffer(struct cavs_stream* stream);
    void setup_bdl(struct cavs_stream* stream);
    void mixer_init(void);
    void jack_init(void);
    void dsp_init(void);
    void unsolicited_enable(uint8_t nid, uint8_t enable);
    void widget_amp_set(uint8_t nid, uint8_t direction, uint8_t index, uint8_t mute, uint8_t gain);
    void parse_config_default(uint32_t config, struct cavs_pin_config* out);
    int find_widget_by_nid(uint8_t nid);
    void delay_us(uint32_t us);
    void delay_ms(uint32_t ms);

    volatile uint8_t* hda_base;
    volatile uint8_t* dsp_base;
    uint8_t codec_address;
    uint8_t initialized;
    uint8_t widget_count;
    struct cavs_widget widgets[INTEL_CAVS_MAX_WIDGETS];
    uint8_t pin_count;
    uint8_t pin_nids[INTEL_CAVS_MAX_PINS];
    uint8_t dac_count;
    uint8_t dac_nids[INTEL_CAVS_MAX_PINS];
    uint8_t adc_count;
    uint8_t adc_nids[INTEL_CAVS_MAX_PINS];
    uint8_t stream_count;
    struct cavs_stream streams[INTEL_CAVS_MAX_STREAMS];
    uint8_t mixer_count;
    struct cavs_mixer_control mixer_controls[INTEL_CAVS_MAX_MIXERS];
    uint8_t jack_count;
    uint8_t jack_nids[INTEL_CAVS_MAX_PINS];
    struct cavs_stats stats;
    uint8_t master_volume;
    uint8_t power_state;
    uint8_t dsp_ready_flag;
    uint32_t dsp_revision;
};
#endif

#ifdef __cplusplus
extern "C" {
#endif

int intel_raptorlake_cavs_init(void);
void intel_raptorlake_cavs_shutdown(void);
void intel_raptorlake_cavs_poll(void);

#ifdef __cplusplus
}
#endif

#endif
