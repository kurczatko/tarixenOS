#include "vt1611.h"
#include "../../include/printf/vga.h"

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

static uint32_t pci_read32(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

static void pci_write32(uint8_t bus, uint8_t slot, uint8_t func,
                        uint8_t offset, uint32_t value)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)func << 8) |
                       (offset & 0xFC);
    outl(0xCF8, address);
    outl(0xCFC, value);
}

VT1611* VT1611::g_instance = 0;

VT1611::VT1611()
    : hda_base(0), codec_address(0), initialized(0), widget_count(0),
      pin_count(0), dac_count(0), adc_count(0), stream_count(0),
      mixer_count(0), jack_count(0), master_volume(80), power_state(0)
{
    for (int i = 0; i < VT1611_MAX_WIDGETS; i++) {
        widgets[i].nid = 0;
        widgets[i].type = 0;
        widgets[i].capabilities = 0;
        widgets[i].connection_count = 0;
        widgets[i].selected_connection = 0;
        widgets[i].pin_capabilities = 0;
        widgets[i].config_default = 0;
        widgets[i].pin_control = 0;
        widgets[i].power_state = POWER_STATE_D0;
        for (int j = 0; j < 4; j++) {
            widgets[i].amp_out_mute[j] = 0;
            widgets[i].amp_out_gain[j] = 0;
            widgets[i].amp_in_mute[j] = 0;
            widgets[i].amp_in_gain[j] = 0;
        }
        widgets[i].conv_format = 0x4010;
        widgets[i].stream_tag = 0;
        widgets[i].channel_id = 0;
        widgets[i].eapd_btl = 0;
        widgets[i].digital_conv1 = 0;
        widgets[i].digital_conv2 = 0;
        widgets[i].vendor_id = 0;
        widgets[i].revision_id = 0;
        for (int j = 0; j < 32; j++) widgets[i].connections[j] = 0;
    }

    for (int i = 0; i < VT1611_MAX_PINS; i++) {
        pin_nids[i] = 0;
        dac_nids[i] = 0;
        adc_nids[i] = 0;
        jack_nids[i] = 0;
    }

    for (int i = 0; i < VT1611_MAX_STREAMS; i++) {
        streams[i].stream_tag = 0;
        streams[i].channel_count = 0;
        streams[i].format = 0;
        streams[i].rate = 0;
        streams[i].bits_per_sample = 0;
        streams[i].running = 0;
        streams[i].bdpl_addr = 0;
        streams[i].bdl_entries = 0;
        streams[i].buffer_size = 0;
        streams[i].period_size = 0;
        streams[i].dma_buffer = 0;
        streams[i].pos = 0;
    }

    for (int i = 0; i < 64; i++) {
        mixer_controls[i].nid = 0;
        mixer_controls[i].index = 0;
        mixer_controls[i].direction = 0;
        mixer_controls[i].min_gain = 0;
        mixer_controls[i].max_gain = 0;
        mixer_controls[i].current_gain = 0;
        mixer_controls[i].mute = 0;
        for (int j = 0; j < 32; j++) mixer_controls[i].name[j] = 0;
    }

    stats.verbs_sent = 0;
    stats.verbs_failed = 0;
    stats.interrupts = 0;
    stats.buffer_underruns = 0;
    stats.buffer_overruns = 0;
    stats.jack_changes = 0;
    stats.spdif_packets = 0;
    stats.spdif_errors = 0;
}

VT1611::~VT1611()
{
}

VT1611* VT1611::instance()
{
    return g_instance;
}

void VT1611::delay_us(uint32_t us)
{
    for (uint32_t i = 0; i < us; i++) {
        for (volatile int j = 0; j < 40; j++);
    }
}

void VT1611::delay_ms(uint32_t ms)
{
    for (uint32_t i = 0; i < ms; i++) {
        for (volatile int j = 0; j < 10000; j++);
    }
}

uint8_t VT1611::probe(uint16_t vendor, uint16_t device)
{
    if (vendor != VT1611_VENDOR_ID || device != VT1611_DEVICE_ID) return 0;
    return 1;
}

uint32_t VT1611::send_verb(uint32_t verb)
{
    if (!hda_base) {
        stats.verbs_failed++;
        return 0xFFFFFFFF;
    }

    while (*(volatile uint32_t *)(hda_base + HDA_ICS) & 1);

    *(volatile uint32_t *)(hda_base + HDA_IR) = verb;
    *(volatile uint16_t *)(hda_base + HDA_ICS) =
        (*(volatile uint16_t *)(hda_base + HDA_ICS) & ~2) | 1;

    int timeout = 100000;
    while (timeout--) {
        if (!(*(volatile uint32_t *)(hda_base + HDA_ICS) & 1)) {
            stats.verbs_sent++;
            return *(volatile uint32_t *)(hda_base + HDA_IR);
        }
    }

    stats.verbs_failed++;
    return 0xFFFFFFFF;
}

uint32_t VT1611::send_verb_nid(uint8_t nid, uint32_t verb_cmd, uint32_t payload)
{
    uint32_t verb = ((uint32_t)codec_address << 28) |
                    ((uint32_t)nid << 20) |
                    ((verb_cmd & 0xFFF) << 8) |
                    (payload & 0xFF);
    return send_verb(verb);
}

int VT1611::find_codec_address(void)
{
    if (!hda_base) return 0;

    uint32_t state = *(volatile uint32_t *)(hda_base + HDA_STATESTS);
    for (int i = 0; i < 4; i++) {
        if (state & (1 << i)) {
            codec_address = (uint8_t)i;
            return 1;
        }
    }
    return 0;
}

void VT1611::init(void)
{
    if (initialized) return;

    uint8_t bus, slot, func;
    uint32_t bar0 = 0;
    int found = 0;

    for (bus = 0; bus < 255 && !found; bus++) {
        for (slot = 0; slot < 32 && !found; slot++) {
            for (func = 0; func < 8; func++) {
                uint32_t class_rev = pci_read32(bus, slot, func, 0x08);
                if ((class_rev >> 16) == 0x0403) {
                    bar0 = pci_read32(bus, slot, func, 0x10);
                    uint32_t cmd = pci_read32(bus, slot, func, 0x04);
                    pci_write32(bus, slot, func, 0x04, cmd | 0x00000006);
                    found = 1;
                    break;
                }
            }
        }
    }

    if (!found || bar0 == 0 || bar0 == 0xFFFFFFFFU) return;

    hda_base = (volatile uint8_t *)(bar0 & 0xFFFFFFF0);

    *(volatile uint32_t *)(hda_base + HDA_GCTL) &= ~1;
    while (*(volatile uint32_t *)(hda_base + HDA_GCTL) & 1);

    *(volatile uint32_t *)(hda_base + HDA_GCTL) |= 1;
    while (!(*(volatile uint32_t *)(hda_base + HDA_GCTL) & 1));

    delay_ms(10);

    if (!find_codec_address()) return;

    uint32_t vendor_id = send_verb_nid(0, HDA_VERB_GET_PARAMETER, PARAM_VENDOR_ID);
    if ((vendor_id & 0xFFFF) != VT1611_VENDOR_ID) {
        return;
    }

    widgets[0].vendor_id = (uint16_t)(vendor_id & 0xFFFF);
    uint32_t revision = send_verb_nid(0, HDA_VERB_GET_PARAMETER, PARAM_REVISION_ID);
    widgets[0].revision_id = (uint16_t)(revision & 0xFFFF);

    discover_widgets();
    parse_pin_configs();
    setup_default_pins();
    mixer_init();
    jack_init();
    spdif_init();
    power_init();

    g_instance = this;
    initialized = 1;
}

void VT1611::deinit(void)
{
    if (!initialized) return;

    for (int i = 0; i < stream_count; i++) {
        if (streams[i].running) {
            stop_stream(i, 1);
            stop_stream(i, 0);
        }
    }

    for (int i = 0; i < stream_count; i++) {
        free_dma_buffer(&streams[i]);
    }

    initialized = 0;
}

void VT1611::irq_handler(void)
{
    if (!initialized) return;

    stats.interrupts++;

    uint32_t status = *(volatile uint32_t *)(hda_base + HDA_GSTS);
    (void)status;

    for (int i = 0; i < jack_count; i++) {
        uint8_t present = 0;
        jack_get_status(i, &present);
    }
}

int VT1611::pcm_open(uint16_t channels, uint32_t rate, uint16_t format, uint8_t playback)
{
    if (!initialized) return -1;

    int free_stream = -1;
    for (int i = 0; i < stream_count; i++) {
        if (!streams[i].running) {
            free_stream = i;
            break;
        }
    }

    if (free_stream < 0 && stream_count < VT1611_MAX_STREAMS) {
        free_stream = stream_count;
        stream_count++;
    }

    if (free_stream < 0) return -1;

    struct vt1611_stream* s = &streams[free_stream];
    s->stream_tag = (uint8_t)(free_stream + 1);
    s->channel_count = channels;
    s->format = format;
    s->rate = rate;
    s->bits_per_sample = (format == 16) ? 16 : ((format == 24) ? 24 : 8);
    s->running = 1;
    s->pos = 0;

    uint32_t buffer_size = 44100 * 4;
    alloc_dma_buffer(s, buffer_size);
    setup_bdl(s);

    uint32_t conv_format = ((rate & 0xFF) << 0) |
                           ((channels - 1) << 8) |
                           ((s->bits_per_sample == 16 ? 0 : s->bits_per_sample == 24 ? 1 : 2) << 10);

    int dac_index = playback ? 0 : 1;
    if (dac_index < dac_count) {
        send_verb_nid(dac_nids[dac_index], HDA_VERB_SET_CONVERTER_FORMAT, conv_format);
        send_verb_nid(dac_nids[dac_index], HDA_VERB_SET_CONVERTER_STREAM_CHANNEL,
                      (s->stream_tag << 4) | 0);
    }

    start_stream(free_stream, playback);
    return free_stream;
}

int VT1611::pcm_write(const void* data, uint32_t bytes)
{
    if (!initialized || !data || bytes == 0) return -1;

    int best = -1;
    for (int i = 0; i < stream_count; i++) {
        if (streams[i].running && streams[i].dma_buffer) {
            best = i;
            break;
        }
    }
    if (best < 0) return -1;

    struct vt1611_stream* s = &streams[best];
    uint32_t copy = bytes;
    if (s->buffer_size > 0 && copy > s->buffer_size) copy = s->buffer_size;

    uint8_t* dst = (uint8_t*)s->dma_buffer;
    const uint8_t* src = (const uint8_t*)data;
    for (uint32_t i = 0; i < copy; i++) {
        dst[(s->pos + i) % s->buffer_size] = src[i];
    }
    s->pos = (s->pos + copy) % s->buffer_size;

    __asm__ volatile("" : : : "memory");

    return (int)copy;
}

int VT1611::pcm_read(void* data, uint32_t bytes)
{
    if (!initialized || !data || bytes == 0) return -1;

    int best = -1;
    for (int i = 0; i < stream_count; i++) {
        if (streams[i].running && streams[i].dma_buffer) {
            best = i;
            break;
        }
    }
    if (best < 0) return -1;

    struct vt1611_stream* s = &streams[best];
    uint32_t copy = bytes;
    if (s->buffer_size > 0 && copy > s->buffer_size) copy = s->buffer_size;

    uint8_t* dst = (uint8_t*)data;
    uint8_t* src = (uint8_t*)s->dma_buffer;
    for (uint32_t i = 0; i < copy; i++) {
        dst[i] = src[(s->pos + i) % s->buffer_size];
    }

    return (int)copy;
}

void VT1611::pcm_close(uint8_t playback)
{
    (void)playback;
    if (!initialized) return;

    for (int i = 0; i < stream_count; i++) {
        if (streams[i].running) {
            stop_stream(i, 1);
            stop_stream(i, 0);
            free_dma_buffer(&streams[i]);
        }
    }
}

int VT1611::pcm_pause(uint8_t playback, uint8_t pause)
{
    if (!initialized) return -1;
    (void)playback;
    (void)pause;
    return 0;
}

int VT1611::pcm_resume(uint8_t playback)
{
    if (!initialized) return -1;
    (void)playback;
    return 0;
}

int VT1611::mixer_get_control_count(void)
{
    return mixer_count;
}

int VT1611::mixer_get_control(uint8_t index, struct vt1611_mixer_control* ctrl)
{
    if (!initialized || !ctrl || index >= mixer_count) return -1;
    *ctrl = mixer_controls[index];
    return 0;
}

int VT1611::mixer_set_volume(uint8_t index, int32_t gain)
{
    if (!initialized || index >= mixer_count) return -1;
    mixer_controls[index].current_gain = gain;
    widget_amp_set(mixer_controls[index].nid,
                   mixer_controls[index].direction,
                   mixer_controls[index].index,
                   mixer_controls[index].mute ? 1 : 0,
                   (uint8_t)((gain < 0) ? 0 : (gain > 255) ? 255 : gain));
    return 0;
}

int VT1611::mixer_set_mute(uint8_t index, uint8_t mute)
{
    if (!initialized || index >= mixer_count) return -1;
    mixer_controls[index].mute = mute;
    widget_amp_set(mixer_controls[index].nid,
                   mixer_controls[index].direction,
                   mixer_controls[index].index,
                   mute,
                   (uint8_t)mixer_controls[index].current_gain);
    return 0;
}

int VT1611::mixer_get_volume(uint8_t index, int32_t* gain)
{
    if (!initialized || !gain || index >= mixer_count) return -1;
    *gain = mixer_controls[index].current_gain;
    return 0;
}

int VT1611::mixer_get_mute(uint8_t index, uint8_t* mute)
{
    if (!initialized || !mute || index >= mixer_count) return -1;
    *mute = mixer_controls[index].mute;
    return 0;
}

int VT1611::jack_get_count(void)
{
    return jack_count;
}

int VT1611::jack_get_status(uint8_t index, uint8_t* present)
{
    if (!initialized || !present || index >= jack_count) return -1;

    uint32_t sense = send_verb_nid(jack_nids[index], HDA_VERB_GET_PIN_SENSE, 0);
    *present = (sense & (1 << 31)) ? 1 : 0;
    stats.jack_changes++;
    return 0;
}

int VT1611::jack_get_config(uint8_t index, struct vt1611_pin_config* config)
{
    if (!initialized || !config || index >= jack_count) return -1;

    uint32_t cfg = send_verb_nid(jack_nids[index], HDA_VERB_GET_CONFIGURATION_DEFAULT, 0);
    parse_config_default(cfg, config);
    config->nid = jack_nids[index];
    return 0;
}

int VT1611::jack_enable_unsolicited(uint8_t index, uint8_t enable)
{
    if (!initialized || index >= jack_count) return -1;
    unsolicited_enable(jack_nids[index], enable);
    return 0;
}

int VT1611::spdif_open(uint32_t rate, uint16_t format)
{
    if (!initialized) return -1;

    int spdif_pin = -1;
    for (int i = 0; i < pin_count; i++) {
        uint32_t cap = send_verb_nid(pin_nids[i], HDA_VERB_GET_PARAMETER, PARAM_PIN_CAP);
        if (cap & PIN_CAP_HDMI) {
            spdif_pin = pin_nids[i];
            break;
        }
    }

    if (spdif_pin < 0) return -1;

    uint32_t conv_format = ((rate & 0xFF) << 0) | ((format & 0xF) << 10);
    send_verb_nid(spdif_pin, HDA_VERB_SET_DIGITAL_CONVERTER_1, conv_format);
    send_verb_nid(spdif_pin, HDA_VERB_SET_PIN_WIDGET_CONTROL,
                  PIN_CTL_OUT_EN | PIN_CTL_VREF_HIZ);

    return 0;
}

int VT1611::spdif_write(const void* data, uint32_t bytes)
{
    (void)data;
    (void)bytes;
    if (!initialized) return -1;
    stats.spdif_packets++;
    return 0;
}

void VT1611::spdif_close(void)
{
    if (!initialized) return;
}

int VT1611::spdif_set_category(uint8_t category)
{
    (void)category;
    if (!initialized) return -1;
    return 0;
}

int VT1611::spdif_set_copyright(uint8_t copyright)
{
    (void)copyright;
    if (!initialized) return -1;
    return 0;
}

int VT1611::spdif_set_nonaudio(uint8_t nonaudio)
{
    (void)nonaudio;
    if (!initialized) return -1;
    return 0;
}

int VT1611::spdif_set_preemphasis(uint8_t preemphasis)
{
    (void)preemphasis;
    if (!initialized) return -1;
    return 0;
}

void VT1611::set_master_volume(uint8_t percent)
{
    master_volume = percent;
    int vol_index = -1;
    for (int i = 0; i < mixer_count; i++) {
        if (mixer_controls[i].name[0] == 'M' && mixer_controls[i].name[1] == 'a' &&
            mixer_controls[i].name[2] == 's' && mixer_controls[i].name[3] == 't') {
            vol_index = i;
            break;
        }
    }
    if (vol_index >= 0) {
        mixer_set_volume(vol_index, (int32_t)((percent * 255) / 100));
    }
}

uint8_t VT1611::get_master_volume(void)
{
    return master_volume;
}

void VT1611::power_set_state(uint8_t state)
{
    if (!initialized) return;
    power_state = state;
    for (int i = 0; i < widget_count; i++) {
        widget_power_set(widgets[i].nid, state);
    }
}

uint8_t VT1611::power_get_state(void)
{
    return power_state;
}

void VT1611::widget_power_set(uint8_t nid, uint8_t state)
{
    send_verb_nid(nid, HDA_VERB_SET_POWER_STATE, state);
    int idx = find_widget_by_nid(nid);
    if (idx >= 0) widgets[idx].power_state = state;
}

void VT1611::stats_get(struct vt1611_stats* out)
{
    if (!out) return;
    *out = stats;
}

void VT1611::discover_widgets(void)
{
    if (!hda_base) return;

    uint32_t sub_count = send_verb_nid(0, HDA_VERB_GET_PARAMETER, PARAM_SUBORDINATE_NODE_COUNT);
    uint8_t start_nid = (uint8_t)(sub_count >> 8);
    uint8_t end_nid = (uint8_t)(sub_count & 0xFF);

    widget_count = 0;
    dac_count = 0;
    adc_count = 0;

    for (uint8_t nid = start_nid; nid <= end_nid && widget_count < VT1611_MAX_WIDGETS; nid++) {
        struct vt1611_widget* w = &widgets[widget_count];
        w->nid = nid;

        uint32_t widget_cap = send_verb_nid(nid, HDA_VERB_GET_PARAMETER, PARAM_AUDIO_WIDGET_CAP);
        w->capabilities = widget_cap;
        w->type = (uint8_t)((widget_cap >> 20) & 0xF);

        uint32_t conn_len = send_verb_nid(nid, HDA_VERB_GET_PARAMETER, PARAM_CONNECTION_LIST_LEN);
        w->connection_count = (uint16_t)(conn_len & 0xFF);

        if (w->connection_count > 0 && w->connection_count <= 32) {
            uint32_t conn_list = send_verb_nid(nid, HDA_VERB_GET_PARAMETER, PARAM_CONNECTION_LIST);
            for (int j = 0; j < w->connection_count; j++) {
                w->connections[j] = (uint8_t)((conn_list >> (j * 8)) & 0xFF);
            }
        }

        if (w->type == WIDGET_TYPE_PIN_COMPLEX) {
            uint32_t pin_cap = send_verb_nid(nid, HDA_VERB_GET_PARAMETER, PARAM_PIN_CAP);
            w->pin_capabilities = pin_cap;

            uint32_t config = send_verb_nid(nid, HDA_VERB_GET_CONFIGURATION_DEFAULT, 0);
            w->config_default = config;

            uint32_t pin_ctl = send_verb_nid(nid, HDA_VERB_GET_PIN_WIDGET_CONTROL, 0);
            w->pin_control = (uint8_t)pin_ctl;

            pin_nids[pin_count++] = nid;
        }

        if (w->type == WIDGET_TYPE_OUTPUT) {
            dac_nids[dac_count++] = nid;
        } else if (w->type == WIDGET_TYPE_INPUT) {
            adc_nids[adc_count++] = nid;
        }

        w->power_state = POWER_STATE_D0;
        widget_count++;
    }
}

void VT1611::parse_pin_configs(void)
{
    for (int i = 0; i < pin_count; i++) {
        uint8_t nid = pin_nids[i];
        uint32_t cfg = send_verb_nid(nid, HDA_VERB_GET_CONFIGURATION_DEFAULT, 0);
        widgets[find_widget_by_nid(nid)].config_default = cfg;
    }
}

void VT1611::setup_default_pins(void)
{
    for (int i = 0; i < pin_count; i++) {
        uint8_t nid = pin_nids[i];
        uint32_t cfg = widgets[find_widget_by_nid(nid)].config_default;

        uint8_t device = (uint8_t)((cfg >> 20) & 0xF);
        uint8_t port_conn = (uint8_t)((cfg >> 30) & 0x3);

        if (port_conn == PORT_CONNECTIVITY_JACK || port_conn == PORT_CONNECTIVITY_BOTH) {
            jack_nids[jack_count++] = nid;
        }

        if (device == DEFAULT_DEVICE_LINE_OUT || device == DEFAULT_DEVICE_SPEAKER ||
            device == DEFAULT_DEVICE_HP_OUT) {
            configure_pin(nid, PIN_CTL_OUT_EN | PIN_CTL_VREF_HIZ);
        } else if (device == DEFAULT_DEVICE_LINE_IN || device == DEFAULT_DEVICE_MIC_IN ||
                   device == DEFAULT_DEVICE_AUX) {
            configure_pin(nid, PIN_CTL_IN_EN | PIN_CTL_VREF_HIZ);
        }
    }
}

void VT1611::configure_pin(uint8_t nid, uint32_t config)
{
    send_verb_nid(nid, HDA_VERB_SET_PIN_WIDGET_CONTROL, (uint8_t)config);
    int idx = find_widget_by_nid(nid);
    if (idx >= 0) widgets[idx].pin_control = (uint8_t)config;
}

int VT1611::find_dac_for_pin(uint8_t pin_nid)
{
    for (int i = 0; i < dac_count; i++) {
        if (dac_nids[i] == pin_nid) return i;
    }
    return 0;
}

int VT1611::find_adc_for_pin(uint8_t pin_nid)
{
    for (int i = 0; i < adc_count; i++) {
        if (adc_nids[i] == pin_nid) return i;
    }
    return 0;
}

void VT1611::setup_stream(uint8_t stream_tag, uint8_t channels, uint32_t rate,
                          uint16_t format, uint8_t playback)
{
    (void)stream_tag;
    (void)channels;
    (void)rate;
    (void)format;
    (void)playback;
}

void VT1611::start_stream(uint8_t stream_index, uint8_t playback)
{
    if (!initialized || stream_index >= stream_count) return;

    struct vt1611_stream* s = &streams[stream_index];
    if (s->running) return;

    uint32_t sd_offset = 0x80 + (playback ? 0 : 0x20) + (s->stream_tag * 0x20);

    *(volatile uint32_t *)(hda_base + sd_offset + 0x00) &= ~SD_CTL_STREAM_RESET;
    while (*(volatile uint32_t *)(hda_base + sd_offset + 0x00) & SD_CTL_STREAM_RESET);

    *(volatile uint32_t *)(hda_base + sd_offset + 0x00) |= SD_CTL_STREAM_RESET;
    while (!(*(volatile uint32_t *)(hda_base + sd_offset + 0x00) & SD_CTL_STREAM_RESET));

    *(volatile uint32_t *)(hda_base + sd_offset + 0x00) &= ~SD_CTL_STREAM_RESET;
    while (*(volatile uint32_t *)(hda_base + sd_offset + 0x00) & SD_CTL_STREAM_RESET);

    *(volatile uint32_t *)(hda_base + sd_offset + 0x08) = (uint32_t)(uintptr_t)s->bdpl_addr;
    *(volatile uint32_t *)(hda_base + sd_offset + 0x0C) = 0;
    *(volatile uint16_t *)(hda_base + sd_offset + 0x12) = 0x4010;
    *(volatile uint32_t *)(hda_base + sd_offset + 0x18) = (uint32_t)(uintptr_t)s->bdpl_addr;
    *(volatile uint32_t *)(hda_base + sd_offset + 0x1C) = 0;

    *(volatile uint32_t *)(hda_base + sd_offset + 0x00) |= SD_CTL_DMA_START | SD_CTL_IOCE;
    s->running = 1;
}

void VT1611::stop_stream(uint8_t stream_index, uint8_t playback)
{
    if (!initialized || stream_index >= stream_count) return;

    struct vt1611_stream* s = &streams[stream_index];
    if (!s->running) return;

    uint32_t sd_offset = 0x80 + (playback ? 0 : 0x20) + (s->stream_tag * 0x20);

    *(volatile uint32_t *)(hda_base + sd_offset + 0x00) &= ~SD_CTL_DMA_START;
    s->running = 0;
}

void VT1611::alloc_dma_buffer(struct vt1611_stream* stream, uint32_t size)
{
    stream->dma_buffer = (void*)0x20000000;
    stream->buffer_size = size;
    stream->period_size = size / 4;
}

void VT1611::free_dma_buffer(struct vt1611_stream* stream)
{
    stream->dma_buffer = 0;
    stream->buffer_size = 0;
    stream->period_size = 0;
}

void VT1611::setup_bdl(struct vt1611_stream* stream)
{
    stream->bdl_entries = 1;
    stream->bdpl_addr = (uint32_t*)0x30000000;
}

void VT1611::mixer_init(void)
{
    mixer_count = 0;

    for (int i = 0; i < widget_count; i++) {
        struct vt1611_widget* w = &widgets[i];
        if (w->type != WIDGET_TYPE_MIXER && w->type != WIDGET_TYPE_SELECTOR) continue;

        uint32_t out_cap = send_verb_nid(w->nid, HDA_VERB_GET_PARAMETER, PARAM_AMP_OUT_CAP);
        if (out_cap & (1 << 31)) {
            for (int j = 0; j < w->connection_count && mixer_count < 64; j++) {
                struct vt1611_mixer_control* c = &mixer_controls[mixer_count++];
                c->nid = w->nid;
                c->index = (uint8_t)j;
                c->direction = 1;
                c->min_gain = 0;
                c->max_gain = 127;
                c->current_gain = 100;
                c->mute = 0;
                for (int k = 0; k < 31; k++) c->name[k] = 0;
                c->name[0] = 'M';
                c->name[1] = 'i';
                c->name[2] = 'x';
                c->name[3] = 0;
            }
        }
    }

    if (mixer_count == 0) {
        struct vt1611_mixer_control* c = &mixer_controls[mixer_count++];
        c->nid = 0;
        c->index = 0;
        c->direction = 1;
        c->min_gain = 0;
        c->max_gain = 127;
        c->current_gain = 100;
        c->mute = 0;
        c->name[0] = 'M';
        c->name[1] = 'a';
        c->name[2] = 's';
        c->name[3] = 't';
        c->name[4] = 0;
    }
}

void VT1611::jack_init(void)
{
    jack_count = 0;
    for (int i = 0; i < pin_count; i++) {
        uint8_t nid = pin_nids[i];
        uint32_t cap = send_verb_nid(nid, HDA_VERB_GET_PARAMETER, PARAM_PIN_CAP);
        if (cap & PIN_CAP_PRESENCE_DETECT) {
            jack_nids[jack_count++] = nid;
        }
    }
}

void VT1611::spdif_init(void)
{
}

void VT1611::power_init(void)
{
    power_state = POWER_STATE_D0;
    for (int i = 0; i < widget_count; i++) {
        widget_power_set(widgets[i].nid, POWER_STATE_D0);
    }
}

void VT1611::unsolicited_enable(uint8_t nid, uint8_t enable)
{
    send_verb_nid(nid, HDA_VERB_SET_UNSOLICITED_RESPONSE, enable ? 0x80 : 0x00);
}

void VT1611::widget_amp_set(uint8_t nid, uint8_t direction, uint8_t index,
                            uint8_t mute, uint8_t gain)
{
    uint32_t payload = 0;
    if (mute) payload |= AMP_GAIN_MUTE_MUTE;
    payload |= (gain & AMP_GAIN_MUTE_GAIN_MASK);
    if (direction) payload |= (1 << 8);
    payload |= ((uint32_t)index << 9);
    send_verb_nid(nid, HDA_VERB_SET_AMP_GAIN_MUTE, payload);
}

void VT1611::parse_config_default(uint32_t config, struct vt1611_pin_config* out)
{
    out->sequence = (uint8_t)((config >> 0) & 0xF);
    out->default_association = (uint8_t)((config >> 4) & 0xF);
    out->misc = (uint8_t)((config >> 8) & 0xF);
    out->color = (uint8_t)((config >> 12) & 0xF);
    out->connection_type = (uint8_t)((config >> 16) & 0xF);
    out->location = (uint8_t)((config >> 20) & 0xF);
    out->port_connectivity = (uint8_t)((config >> 30) & 0x3);
    out->default_device = (uint8_t)((config >> 20) & 0xF);
}

int VT1611::find_widget_by_nid(uint8_t nid)
{
    for (int i = 0; i < widget_count; i++) {
        if (widgets[i].nid == nid) return i;
    }
    return -1;
}

void VT1611::debug_print_widgets(void)
{
    for (int i = 0; i < widget_count; i++) {
        struct vt1611_widget* w = &widgets[i];
        (void)w;
    }
}

extern "C" void vt1611_irq(void)
{
    if (VT1611::instance()) {
        VT1611::instance()->irq_handler();
    }
}