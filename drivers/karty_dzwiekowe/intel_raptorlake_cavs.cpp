/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/karty_dzwiekowe/intel_raptorlake_cavs.h>

typedef unsigned int uintptr_t;

static IntelRaptorLakeCAVS g_instance_storage;

static inline uint8_t cavs_inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint16_t cavs_inw(uint16_t port)
{
    uint16_t value;
    __asm__ volatile("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline uint32_t cavs_inl(uint16_t port)
{
    uint32_t value;
    __asm__ volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void cavs_outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline void cavs_outw(uint16_t port, uint16_t value)
{
    __asm__ volatile("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline void cavs_outl(uint16_t port, uint32_t value)
{
    __asm__ volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

static uint32_t cavs_pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFCU);
    cavs_outl(0xCF8U, address);
    return cavs_inl(0xCFCU);
}

static void cavs_pci_write32(uint8_t bus, uint8_t slot, uint8_t func,
                              uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFCU);
    cavs_outl(0xCF8U, address);
    cavs_outl(0xCFCU, value);
}

IntelRaptorLakeCAVS::IntelRaptorLakeCAVS()
    : hda_base(0), dsp_base(0), codec_address(0), initialized(0),
      widget_count(0), pin_count(0), dac_count(0), adc_count(0),
      stream_count(0), mixer_count(0), jack_count(0),
      master_volume(80U), power_state(POWER_D0), dsp_ready_flag(0U),
      dsp_revision(0U)
{
    for (uint32_t i = 0U; i < INTEL_CAVS_MAX_WIDGETS; i++) {
        widgets[i].nid = 0U;
        widgets[i].type = 0U;
        widgets[i].capabilities = 0U;
        widgets[i].connection_count = 0U;
        widgets[i].selected_connection = 0U;
        widgets[i].pin_capabilities = 0U;
        widgets[i].config_default = 0U;
        widgets[i].pin_control = 0U;
        widgets[i].power_state = POWER_D3;
        widgets[i].amp_out_mute[0] = 0U; widgets[i].amp_out_mute[1] = 0U;
        widgets[i].amp_out_mute[2] = 0U; widgets[i].amp_out_mute[3] = 0U;
        widgets[i].amp_out_gain[0] = 0U; widgets[i].amp_out_gain[1] = 0U;
        widgets[i].amp_out_gain[2] = 0U; widgets[i].amp_out_gain[3] = 0U;
        widgets[i].amp_in_mute[0] = 0U; widgets[i].amp_in_mute[1] = 0U;
        widgets[i].amp_in_mute[2] = 0U; widgets[i].amp_in_mute[3] = 0U;
        widgets[i].amp_in_gain[0] = 0U; widgets[i].amp_in_gain[1] = 0U;
        widgets[i].amp_in_gain[2] = 0U; widgets[i].amp_in_gain[3] = 0U;
        widgets[i].conv_format = 0U;
        widgets[i].stream_tag = 0U;
        widgets[i].channel_id = 0U;
        widgets[i].eapd_btl = 0U;
        widgets[i].digital_conv1 = 0U;
        widgets[i].digital_conv2 = 0U;
        widgets[i].vendor_id = 0U;
        widgets[i].revision_id = 0U;
    }
    for (uint32_t i = 0U; i < INTEL_CAVS_MAX_PINS; i++) {
        pin_nids[i] = 0U;
        dac_nids[i] = 0U;
        adc_nids[i] = 0U;
        jack_nids[i] = 0U;
    }
    for (uint32_t i = 0U; i < INTEL_CAVS_MAX_STREAMS; i++) {
        streams[i].stream_tag = 0U;
        streams[i].channel_count = 0U;
        streams[i].format = 0U;
        streams[i].rate = 0U;
        streams[i].bits_per_sample = 16U;
        streams[i].running = 0U;
        streams[i].bdpl_addr = 0U;
        streams[i].bdl_entries = 0U;
        streams[i].buffer_size = 0U;
        streams[i].period_size = 0U;
        streams[i].dma_buffer = 0U;
        streams[i].pos = 0U;
    }
    for (uint32_t i = 0U; i < INTEL_CAVS_MAX_MIXERS; i++) {
        mixer_controls[i].nid = 0U;
        mixer_controls[i].index = 0U;
        mixer_controls[i].direction = 0U;
        mixer_controls[i].min_gain = 0;
        mixer_controls[i].max_gain = 0;
        mixer_controls[i].current_gain = 0;
        mixer_controls[i].mute = 0U;
        mixer_controls[i].name[0] = 0;
    }
    stats.verbs_sent = 0U;
    stats.verbs_failed = 0U;
    stats.interrupts = 0U;
    stats.buffer_underruns = 0U;
    stats.buffer_overruns = 0U;
    stats.jack_changes = 0U;
    stats.dsp_ipc_messages = 0U;
    stats.dsp_fw_errors = 0U;
}

IntelRaptorLakeCAVS::~IntelRaptorLakeCAVS()
{
}

IntelRaptorLakeCAVS* IntelRaptorLakeCAVS::instance()
{
    return &g_instance_storage;
}

void IntelRaptorLakeCAVS::set_hda_base(volatile uint8_t* base)
{
    hda_base = base;
}

uint8_t IntelRaptorLakeCAVS::is_initialized(void)
{
    return initialized;
}

void IntelRaptorLakeCAVS::delay_us(uint32_t us)
{
    for (uint32_t i = 0U; i < us; i++) {
        for (uint32_t j = 0U; j < 48U; j++);
    }
}

void IntelRaptorLakeCAVS::delay_ms(uint32_t ms)
{
    for (uint32_t i = 0U; i < ms; i++) {
        for (uint32_t j = 0U; j < 48000U; j++);
    }
}

uint32_t IntelRaptorLakeCAVS::send_verb(uint32_t verb)
{
    volatile uint32_t* ics = (volatile uint32_t*)(hda_base + HDA_ICS);
    while (*ics & 1U) { delay_us(1U); }
    volatile uint32_t* ir = (volatile uint32_t*)(hda_base + HDA_IR);
    *ir = verb;
    *ics = (*ics & ~2U) | 1U;
    int timeout = 100000;
    while (timeout--) {
        if (!(*ics & 1U)) {
            stats.verbs_sent++;
            return *ir;
        }
        delay_us(1U);
    }
    stats.verbs_failed++;
    return 0xFFFFFFFFU;
}

uint32_t IntelRaptorLakeCAVS::send_verb_nid(uint8_t nid, uint32_t verb_cmd, uint32_t payload)
{
    return send_verb(((uint32_t)codec_address << 28U) | ((uint32_t)nid << 20U) |
                     (verb_cmd & 0xFFF00U) | (payload & 0xFFU));
}

int IntelRaptorLakeCAVS::find_codec_address(void)
{
    uint32_t statests = *(volatile uint32_t*)(hda_base + HDA_STATESTS);
    for (int i = 0; i < 4; i++) {
        if (statests & (1U << i)) {
            codec_address = (uint8_t)i;
            return 1;
        }
    }
    return 0;
}

uint8_t IntelRaptorLakeCAVS::probe(uint16_t vendor, uint16_t device)
{
    if (vendor == INTEL_CAVS_VENDOR_ID &&
        (device == INTEL_CAVS_DEVICE_RAPTORLAKE ||
         device == INTEL_CAVS_DEVICE_ADL ||
         device == INTEL_CAVS_DEVICE_TGL)) {
        return 1U;
    }
    return 0U;
}

void IntelRaptorLakeCAVS::init(void)
{
    if (initialized) return;

    uint32_t gcap = *(volatile uint32_t*)(hda_base + HDA_GCAP);
    uint16_t vmaj = (uint16_t)(*(volatile uint32_t*)(hda_base + HDA_VMAJ) & 0xFFU);
    uint16_t vmin = (uint16_t)(*(volatile uint32_t*)(hda_base + HDA_VMIN) & 0xFFU);
    (void)gcap; (void)vmaj; (void)vmin;

    *(volatile uint32_t*)(hda_base + HDA_GCTL) &= ~1U;
    while (*(volatile uint32_t*)(hda_base + HDA_GCTL) & 1U) { delay_us(1U); }
    *(volatile uint32_t*)(hda_base + HDA_GCTL) |= 1U;
    while (!(*(volatile uint32_t*)(hda_base + HDA_GCTL) & 1U)) { delay_us(1U); }
    delay_ms(10U);

    if (!find_codec_address()) {
        initialized = 0;
        return;
    }

    discover_widgets();
    parse_pin_configs();
    setup_default_pins();
    mixer_init();
    jack_init();
    dsp_init();

    power_state = POWER_D0;
    initialized = 1U;
    stats.interrupts = 0U;
}

void IntelRaptorLakeCAVS::deinit(void)
{
    if (!initialized) return;
    for (uint32_t i = 0U; i < stream_count; i++) {
        if (streams[i].running) {
            stop_stream(i, 1U);
            stop_stream(i, 0U);
        }
    }
    dsp_clock_enable(0U);
    *(volatile uint32_t*)(hda_base + HDA_GCTL) &= ~1U;
    initialized = 0U;
    dsp_ready_flag = 0U;
}

void IntelRaptorLakeCAVS::discover_widgets(void)
{
    widget_count = 0U;
    dac_count = 0U;
    adc_count = 0U;

    for (uint32_t nid = 0x02U; nid < 0x20U && widget_count < INTEL_CAVS_MAX_WIDGETS; nid++) {
        uint32_t param = send_verb_nid(nid, HDA_VERB_GET_PARAMETER, PARAM_SUBNODE_COUNT);
        uint32_t awc = send_verb_nid(nid, HDA_VERB_GET_PARAMETER, PARAM_AUDIO_WIDGET_CAP);
        uint32_t widget_type = (awc >> 20U) & 0xFU;

        if ((param & 0x00F00000U) == 0x00000000U) {
            widgets[widget_count].nid = (uint8_t)nid;
            widgets[widget_count].type = (uint8_t)widget_type;
            widgets[widget_count].capabilities = awc;
            if (widget_type == WIDGET_TYPE_OUTPUT) {
                dac_nids[dac_count++] = (uint8_t)nid;
            } else if (widget_type == WIDGET_TYPE_INPUT) {
                adc_nids[adc_count++] = (uint8_t)nid;
            }
            widget_count++;
        } else if ((param & 0x00F00000U) == 0x00400000U) {
            widgets[widget_count].nid = (uint8_t)nid;
            widgets[widget_count].type = WIDGET_TYPE_PIN;
            widgets[widget_count].capabilities = awc;
            widgets[widget_count].pin_capabilities =
                send_verb_nid(nid, HDA_VERB_GET_PARAMETER, PARAM_PIN_CAP);
            widgets[widget_count].config_default =
                send_verb_nid(nid, HDA_VERB_GET_CONFIG_DEFAULT, 0U);
            pin_nids[pin_count++] = (uint8_t)nid;
            widget_count++;
        }
    }
}

void IntelRaptorLakeCAVS::parse_pin_configs(void)
{
    for (uint32_t i = 0U; i < pin_count; i++) {
        uint32_t config = widgets[i].config_default;
        (void)config;
    }
}

void IntelRaptorLakeCAVS::setup_default_pins(void)
{
    for (uint32_t i = 0U; i < pin_count; i++) {
        uint8_t nid = pin_nids[i];
        uint32_t config = widgets[i].config_default;
        uint8_t device = (uint8_t)((config >> 20U) & 0xFU);
        if (device == DEFAULT_DEVICE_SPEAKER || device == DEFAULT_DEVICE_HP_OUT ||
            device == DEFAULT_DEVICE_LINE_OUT) {
            send_verb_nid(nid, HDA_VERB_SET_PIN_CTL, PIN_CTL_OUT_EN);
            send_verb_nid(nid, HDA_VERB_SET_CONNECTION_SELECT, dac_count ? dac_nids[0] : 0U);
        } else if (device == DEFAULT_DEVICE_MIC_IN || device == DEFAULT_DEVICE_LINE_IN) {
            send_verb_nid(nid, HDA_VERB_SET_PIN_CTL, PIN_CTL_IN_EN);
        }
    }
}

void IntelRaptorLakeCAVS::mixer_init(void)
{
    mixer_count = 0U;
    for (uint32_t i = 0U; i < widget_count; i++) {
        if (widgets[i].type == WIDGET_TYPE_MIXER && mixer_count < INTEL_CAVS_MAX_MIXERS) {
            struct cavs_mixer_control* ctrl = &mixer_controls[mixer_count++];
            ctrl->nid = widgets[i].nid;
            ctrl->index = 0U;
            ctrl->direction = 0U;
            ctrl->min_gain = -96;
            ctrl->max_gain = 0;
            ctrl->current_gain = 0;
            ctrl->mute = 0U;
            ctrl->name[0] = 'M'; ctrl->name[1] = 'i'; ctrl->name[2] = 'x';
            ctrl->name[3] = 0;
        }
    }
}

void IntelRaptorLakeCAVS::jack_init(void)
{
    jack_count = 0U;
    for (uint32_t i = 0U; i < pin_count; i++) {
        if (widgets[i].pin_capabilities & PIN_CAP_PRESENCE_DETECT &&
            jack_count < INTEL_CAVS_MAX_PINS) {
            jack_nids[jack_count++] = pin_nids[i];
        }
    }
}

void IntelRaptorLakeCAVS::dsp_init(void)
{
    dsp_base = hda_base + CAVS_DSP_BASE;
    dsp_revision = *(volatile uint32_t*)(dsp_base + CAVS_DSP_REV);
    dsp_clock_enable(1U);
    delay_ms(50U);
    dsp_ready_flag = (*(volatile uint32_t*)(dsp_base + CAVS_DSP_FW_STATUS) & CAVS_FW_RUNNING) ? 1U : 0U;
}

int IntelRaptorLakeCAVS::dsp_ready(void)
{
    return dsp_ready_flag;
}

int IntelRaptorLakeCAVS::dsp_reset(void)
{
    *(volatile uint32_t*)(dsp_base + CAVS_DSP_FW_CMD) = 0x00000001U;
    delay_ms(100U);
    dsp_ready_flag = (*(volatile uint32_t*)(dsp_base + CAVS_DSP_FW_STATUS) & CAVS_FW_RUNNING) ? 1U : 0U;
    return dsp_ready_flag ? 1 : 0;
}

int IntelRaptorLakeCAVS::dsp_clock_enable(uint8_t enable)
{
    uint32_t val = *(volatile uint32_t*)(dsp_base + CAVS_DSP_CLK_CTL);
    if (enable) val |= 1U; else val &= ~1U;
    *(volatile uint32_t*)(dsp_base + CAVS_DSP_CLK_CTL) = val;
    delay_us(100U);
    return 1;
}

int IntelRaptorLakeCAVS::dsp_fw_load(const void* fw_data, uint32_t fw_size)
{
    (void)fw_data;
    (void)fw_size;
    if (!dsp_base) return 0;
    *(volatile uint32_t*)(dsp_base + CAVS_DSP_FW_CMD) = 0x00000002U;
    delay_ms(200U);
    if (*(volatile uint32_t*)(dsp_base + CAVS_DSP_FW_STATUS) & CAVS_FW_ERROR) {
        stats.dsp_fw_errors++;
        return 0;
    }
    dsp_ready_flag = 1U;
    return 1;
}

int IntelRaptorLakeCAVS::dsp_fw_start(void)
{
    if (!dsp_base) return 0;
    *(volatile uint32_t*)(dsp_base + CAVS_DSP_FW_CMD) = 0x00000003U;
    delay_ms(100U);
    dsp_ready_flag = (*(volatile uint32_t*)(dsp_base + CAVS_DSP_FW_STATUS) & CAVS_FW_RUNNING) ? 1U : 0U;
    return dsp_ready_flag ? 1 : 0;
}

int IntelRaptorLakeCAVS::dsp_ipc_send(uint32_t ipc0, uint32_t ipc1, uint32_t ipc2, uint32_t ipc3)
{
    if (!dsp_base) return 0;
    *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC0) = ipc0;
    *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC1) = ipc1;
    *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC2) = ipc2;
    *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC3) = ipc3;
    *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC_IER) |= 1U;
    stats.dsp_ipc_messages++;
    return 1;
}

int IntelRaptorLakeCAVS::dsp_ipc_receive(uint32_t* ipc0, uint32_t* ipc1, uint32_t* ipc2, uint32_t* ipc3)
{
    if (!dsp_base) return 0;
    uint32_t isr = *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC_ISR);
    if (!(isr & 1U)) return 0;
    *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC_ISR) = isr;
    if (ipc0) *ipc0 = *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC0);
    if (ipc1) *ipc1 = *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC1);
    if (ipc2) *ipc2 = *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC2);
    if (ipc3) *ipc3 = *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC3);
    stats.dsp_ipc_messages++;
    return 1;
}

void IntelRaptorLakeCAVS::irq_handler(void)
{
    if (!initialized) return;
    stats.interrupts++;
    uint32_t gsts = *(volatile uint32_t*)(hda_base + HDA_GSTS);
    if (gsts & 1U) {
        *(volatile uint32_t*)(hda_base + HDA_GSTS) = 1U;
    }
    if (dsp_base) {
        uint32_t isr = *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC_ISR);
        if (isr) {
            *(volatile uint32_t*)(dsp_base + CAVS_DSP_IPC_ISR) = isr;
            stats.dsp_ipc_messages++;
        }
    }
}

int IntelRaptorLakeCAVS::pcm_open(uint16_t channels, uint32_t rate, uint16_t format, uint8_t playback)
{
    if (!initialized || stream_count >= INTEL_CAVS_MAX_STREAMS) return -1;
    uint8_t tag = (uint8_t)(stream_count + 1U);
    struct cavs_stream* stream = &streams[stream_count++];
    stream->stream_tag = tag;
    stream->channel_count = channels;
    stream->rate = rate;
    stream->format = format;
    stream->bits_per_sample = 16U;
    stream->running = 0U;
    alloc_dma_buffer(stream, 65536U);
    setup_bdl(stream);
    setup_stream(tag, channels, rate, format, playback);
    return (int)(stream_count - 1);
}

int IntelRaptorLakeCAVS::pcm_write(const void* data, uint32_t bytes)
{
    if (!initialized || stream_count == 0) return -1;
    struct cavs_stream* stream = &streams[0];
    if (!stream->running) return -1;
    uint32_t copied = bytes < stream->period_size ? bytes : stream->period_size;
    for (uint32_t i = 0U; i < copied; i++) {
        ((uint8_t*)stream->dma_buffer)[stream->pos % stream->buffer_size] =
            ((const uint8_t*)data)[i];
        stream->pos++;
    }
    stats.buffer_underruns += (bytes > copied) ? 1U : 0U;
    return (int)copied;
}

int IntelRaptorLakeCAVS::pcm_read(void* data, uint32_t bytes)
{
    if (!initialized || stream_count == 0) return -1;
    struct cavs_stream* stream = &streams[0];
    if (!stream->running) return -1;
    uint32_t copied = bytes < stream->period_size ? bytes : stream->period_size;
    for (uint32_t i = 0U; i < copied; i++) {
        ((uint8_t*)data)[i] =
            ((uint8_t*)stream->dma_buffer)[stream->pos % stream->buffer_size];
        stream->pos++;
    }
    stats.buffer_overruns += (bytes > copied) ? 1U : 0U;
    return (int)copied;
}

void IntelRaptorLakeCAVS::pcm_close(uint8_t playback)
{
    (void)playback;
    for (uint32_t i = 0U; i < stream_count; i++) {
        if (streams[i].running) {
            stop_stream(i, playback);
            free_dma_buffer(&streams[i]);
            streams[i].running = 0U;
        }
    }
}

int IntelRaptorLakeCAVS::pcm_pause(uint8_t playback, uint8_t pause)
{
    (void)playback;
    (void)pause;
    for (uint32_t i = 0U; i < stream_count; i++) {
        if (streams[i].running) {
            if (pause) {
                streams[i].running = 0U;
            } else {
                streams[i].running = 1U;
            }
        }
    }
    return 1;
}

int IntelRaptorLakeCAVS::pcm_resume(uint8_t playback)
{
    (void)playback;
    for (uint32_t i = 0U; i < stream_count; i++) {
        if (!streams[i].running) {
            streams[i].running = 1U;
        }
    }
    return 1;
}

int IntelRaptorLakeCAVS::mixer_get_control_count(void)
{
    return (int)mixer_count;
}

int IntelRaptorLakeCAVS::mixer_get_control(uint8_t index, struct cavs_mixer_control* ctrl)
{
    if (index >= mixer_count || !ctrl) return 0;
    *ctrl = mixer_controls[index];
    return 1;
}

int IntelRaptorLakeCAVS::mixer_set_volume(uint8_t index, int32_t gain)
{
    if (index >= mixer_count) return 0;
    if (gain < mixer_controls[index].min_gain) gain = mixer_controls[index].min_gain;
    if (gain > mixer_controls[index].max_gain) gain = mixer_controls[index].max_gain;
    mixer_controls[index].current_gain = gain;
    return 1;
}

int IntelRaptorLakeCAVS::mixer_set_mute(uint8_t index, uint8_t mute)
{
    if (index >= mixer_count) return 0;
    mixer_controls[index].mute = mute ? 1U : 0U;
    return 1;
}

int IntelRaptorLakeCAVS::mixer_get_volume(uint8_t index, int32_t* gain)
{
    if (index >= mixer_count || !gain) return 0;
    *gain = mixer_controls[index].current_gain;
    return 1;
}

int IntelRaptorLakeCAVS::mixer_get_mute(uint8_t index, uint8_t* mute)
{
    if (index >= mixer_count || !mute) return 0;
    *mute = mixer_controls[index].mute;
    return 1;
}

int IntelRaptorLakeCAVS::jack_get_count(void)
{
    return (int)jack_count;
}

int IntelRaptorLakeCAVS::jack_get_status(uint8_t index, uint8_t* present)
{
    if (index >= jack_count || !present) return 0;
    uint8_t nid = jack_nids[index];
    uint32_t sense = send_verb_nid(nid, HDA_VERB_GET_PIN_SENSE, 0U);
    *present = (sense & (1U << 31U)) ? 1U : 0U;
    return 1;
}

int IntelRaptorLakeCAVS::jack_get_config(uint8_t index, struct cavs_pin_config* config)
{
    if (index >= pin_count || !config) return 0;
    uint32_t raw = widgets[index].config_default;
    config->nid = pin_nids[index];
    config->config_default = raw;
    config->sequence = (uint8_t)(raw & 0xFU);
    config->default_association = (uint8_t)((raw >> 4U) & 0xFU);
    config->color = (uint8_t)((raw >> 8U) & 0xFU);
    config->connection_type = (uint8_t)((raw >> 12U) & 0xFU);
    config->location = (uint8_t)((raw >> 16U) & 0x3U);
    config->port_connectivity = (uint8_t)((raw >> 30U) & 0x3U);
    config->default_device = (uint8_t)((raw >> 20U) & 0xFU);
    return 1;
}

int IntelRaptorLakeCAVS::jack_enable_unsolicited(uint8_t index, uint8_t enable)
{
    if (index >= jack_count) return 0;
    unsolicited_enable(jack_nids[index], enable);
    return 1;
}

void IntelRaptorLakeCAVS::set_master_volume(uint8_t percent)
{
    master_volume = percent;
    for (uint32_t i = 0U; i < mixer_count; i++) {
        int32_t gain = (int32_t)((int32_t)percent * mixer_controls[i].max_gain) / 100;
        mixer_controls[i].current_gain = gain;
    }
}

uint8_t IntelRaptorLakeCAVS::get_master_volume(void)
{
    return master_volume;
}

void IntelRaptorLakeCAVS::power_set_state(uint8_t state)
{
    power_state = state;
    send_verb_nid(0x00U, HDA_VERB_SET_POWER_STATE, state);
}

uint8_t IntelRaptorLakeCAVS::power_get_state(void)
{
    return power_state;
}

void IntelRaptorLakeCAVS::widget_power_set(uint8_t nid, uint8_t state)
{
    send_verb_nid(nid, HDA_VERB_SET_POWER_STATE, state);
}

void IntelRaptorLakeCAVS::stats_get(struct cavs_stats* out)
{
    if (!out) return;
    *out = stats;
}

void IntelRaptorLakeCAVS::debug_print_widgets(void)
{
    for (uint32_t i = 0U; i < widget_count; i++) {
        (void)widgets[i].nid;
        (void)widgets[i].type;
    }
}

void IntelRaptorLakeCAVS::setup_stream(uint8_t stream_tag, uint8_t channels,
                                        uint32_t rate, uint16_t format, uint8_t playback)
{
    (void)playback;
    uint32_t stream_offset = SD_OFFSET;
    volatile uint8_t* sd = hda_base + stream_offset;
    *(volatile uint32_t*)(sd + SD_CTL) &= ~(SD_CTL_DMA_START);
    while (*(volatile uint32_t*)(sd + SD_CTL) & SD_CTL_DMA_START) { delay_us(1U); }
    *(volatile uint32_t*)(sd + SD_CTL) |= SD_CTL_STREAM_RESET;
    while (*(volatile uint32_t*)(sd + SD_CTL) & SD_CTL_STREAM_RESET) { delay_us(1U); }
    *(volatile uint32_t*)(sd + SD_CTL) &= ~SD_CTL_STREAM_RESET;
    uint32_t fmt = ((uint32_t)(channels - 1U) << 20U) | ((uint32_t)(format - 1U) << 16U) |
                   (rate & 0xFFFFU);
    *(volatile uint32_t*)(sd + SD_FORMAT) = fmt;
    *(volatile uint32_t*)(sd + SD_BDLPL) = (uint32_t)(uintptr_t)streams[0].bdpl_addr;
    *(volatile uint32_t*)(sd + SD_BDLPU) = 0U;
    *(volatile uint32_t*)(sd + SD_CTL) |= SD_CTL_DMA_START | SD_CTL_IOCE;
    streams[0].stream_tag = stream_tag;
    streams[0].running = 1U;
}

void IntelRaptorLakeCAVS::start_stream(uint8_t stream_index, uint8_t playback)
{
    (void)playback;
    if (stream_index >= stream_count) return;
    streams[stream_index].running = 1U;
}

void IntelRaptorLakeCAVS::stop_stream(uint8_t stream_index, uint8_t playback)
{
    (void)playback;
    if (stream_index >= stream_count) return;
    streams[stream_index].running = 0U;
    volatile uint8_t* sd = hda_base + SD_OFFSET;
    *(volatile uint32_t*)(sd + SD_CTL) &= ~SD_CTL_DMA_START;
}

void IntelRaptorLakeCAVS::alloc_dma_buffer(struct cavs_stream* stream, uint32_t size)
{
    stream->dma_buffer = (void*)0x100000U;
    stream->buffer_size = size;
    stream->period_size = size / 8U;
    stream->pos = 0U;
}

void IntelRaptorLakeCAVS::free_dma_buffer(struct cavs_stream* stream)
{
    stream->dma_buffer = 0U;
    stream->buffer_size = 0U;
    stream->period_size = 0U;
    stream->pos = 0U;
}

void IntelRaptorLakeCAVS::setup_bdl(struct cavs_stream* stream)
{
    static volatile uint32_t bdl[4U] __attribute__((aligned(128U)));
    stream->bdpl_addr = bdl;
    stream->bdl_entries = 4U;
    bdl[0] = (uint32_t)(uintptr_t)stream->dma_buffer;
    bdl[1] = 0U;
    bdl[2] = stream->period_size;
    bdl[3] = 1U;
}

void IntelRaptorLakeCAVS::unsolicited_enable(uint8_t nid, uint8_t enable)
{
    send_verb_nid(nid, HDA_VERB_SET_UNSOLICITED, enable ? 1U : 0U);
}

void IntelRaptorLakeCAVS::widget_amp_set(uint8_t nid, uint8_t direction, uint8_t index,
                                          uint8_t mute, uint8_t gain)
{
    (void)nid; (void)direction; (void)index; (void)mute; (void)gain;
}

int IntelRaptorLakeCAVS::find_widget_by_nid(uint8_t nid)
{
    for (uint32_t i = 0U; i < widget_count; i++) {
        if (widgets[i].nid == nid) return (int)i;
    }
    return -1;
}

void IntelRaptorLakeCAVS::configure_pin(uint8_t nid, uint32_t config)
{
    (void)nid; (void)config;
}

int IntelRaptorLakeCAVS::find_dac_for_pin(uint8_t pin_nid)
{
    (void)pin_nid;
    return dac_count ? (int)dac_nids[0] : -1;
}

int IntelRaptorLakeCAVS::find_adc_for_pin(uint8_t pin_nid)
{
    (void)pin_nid;
    return adc_count ? (int)adc_nids[0] : -1;
}

void IntelRaptorLakeCAVS::parse_config_default(uint32_t config, struct cavs_pin_config* out)
{
    (void)config;
    if (!out) return;
    out->nid = 0U;
    out->config_default = config;
    out->sequence = (uint8_t)(config & 0xFU);
    out->default_association = (uint8_t)((config >> 4U) & 0xFU);
    out->color = (uint8_t)((config >> 8U) & 0xFU);
    out->connection_type = (uint8_t)((config >> 12U) & 0xFU);
    out->location = (uint8_t)((config >> 16U) & 0x3U);
    out->port_connectivity = (uint8_t)((config >> 30U) & 0x3U);
    out->default_device = (uint8_t)((config >> 20U) & 0xFU);
}

extern "C" int intel_raptorlake_cavs_init(void)
{
    IntelRaptorLakeCAVS* drv = IntelRaptorLakeCAVS::instance();
    for (uint16_t bus = 0U; bus < 256U; bus++) {
        for (uint8_t slot = 0U; slot < 32U; slot++) {
            for (uint8_t func = 0U; func < 8U; func++) {
                uint32_t id0 = cavs_pci_read32(bus, slot, func, 0x00U);
                uint16_t vendor = (uint16_t)id0;
                uint16_t device = (uint16_t)(id0 >> 16);
                if (vendor == 0xFFFFU || vendor == 0x0000U) continue;
                if (!drv->probe(vendor, device)) continue;
                uint32_t bar0 = cavs_pci_read32(bus, slot, func, 0x10U);
                if (bar0 == 0U || bar0 == 0xFFFFFFFFU || !(bar0 & 1U)) continue;
                uint32_t cmd = cavs_pci_read32(bus, slot, func, 0x04U);
                cavs_pci_write32(bus, slot, func, 0x04U, cmd | 0x00000006U);
                drv->set_hda_base((volatile uint8_t*)(bar0 & 0xFFFFFFF0U));
                drv->init();
                return drv->is_initialized() ? 1 : 0;
            }
        }
    }
    return 0;
}

extern "C" void intel_raptorlake_cavs_shutdown(void)
{
    IntelRaptorLakeCAVS* drv = IntelRaptorLakeCAVS::instance();
    if (drv) drv->deinit();
}

extern "C" void intel_raptorlake_cavs_poll(void)
{
    IntelRaptorLakeCAVS* drv = IntelRaptorLakeCAVS::instance();
    if (drv && drv->is_initialized()) drv->irq_handler();
}
