#include "gtx3070.h"

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

static Gtx3070 gpu_g;

Gtx3070 *gtx3070_pobierz(void)
{
    return &gpu_g;
}

Gtx3070::Gtx3070(void)
{
    uint32_t i;

    dane.vendor_id = 0;
    dane.device_id = 0;
    dane.revision_id = 0;
    dane.mmio_base = 0;
    dane.fb_addr = GTX3070_FB_ADDR;
    dane.fb_width = GTX3070_FB_WIDTH;
    dane.fb_height = GTX3070_FB_HEIGHT;
    dane.fb_pitch = GTX3070_FB_PITCH;
    dane.initialized = 0;
    dane.architecture = 0;
    dane.vram_size = GTX3070_VRAM_SIZE_BYTES;
    dane.vram_used = 0;
    dane.gfx_clock_mhz = 1725;
    dane.memory_clock_mhz = 7000;
    dane.temperature_c = 45;
    dane.fan_rpm = 2000;

    for (i = 0; i < GTX3070_MAX_VERTEX_BUFFERS; i++) {
        bufor_wierzcholkow_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_INDEX_BUFFERS; i++) {
        bufor_indeksow_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_TEXTURES; i++) {
        tekstury_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_RENDER_TARGETS; i++) {
        cele_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_DEPTH_STENCILS; i++) {
        bufor_glebii_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_SHADERS; i++) {
        shadery_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_SAMPLERS; i++) {
        probniki_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_BLEND_STATES; i++) {
        stany_mieszania_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_RASTERIZER_STATES; i++) {
        stany_rasteryzacji_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_DEPTH_STENCIL_STATES; i++) {
        stany_glebii_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_INPUT_LAYOUTS; i++) {
        uklady_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_CONSTANT_BUFFERS; i++) {
        bufor_stale_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_SWAP_CHAINS; i++) {
        lancuchy_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_FENCES; i++) {
        ogrodzenia_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_MAX_SEMAPHORES; i++) {
        semafory_tab[i].used = 0;
    }
    for (i = 0; i < GTX3070_VRAM_MAX_ALLOCS; i++) {
        alokacje_tab[i].used = 0;
    }

    kontekst.vertex_buffer = GTX3070_HANDLE_NULL;
    kontekst.index_buffer = GTX3070_HANDLE_NULL;
    kontekst.input_layout = GTX3070_HANDLE_NULL;
    kontekst.vertex_shader = GTX3070_HANDLE_NULL;
    kontekst.pixel_shader = GTX3070_HANDLE_NULL;
    kontekst.geometry_shader = GTX3070_HANDLE_NULL;
    kontekst.compute_shader = GTX3070_HANDLE_NULL;
    kontekst.render_target = GTX3070_HANDLE_NULL;
    kontekst.depth_stencil = GTX3070_HANDLE_NULL;
    kontekst.blend_state = GTX3070_HANDLE_NULL;
    kontekst.rasterizer_state = GTX3070_HANDLE_NULL;
    kontekst.depth_stencil_state = GTX3070_HANDLE_NULL;
    for (i = 0; i < GTX3070_MAX_SAMPLERS; i++) {
        kontekst.sampler[i] = GTX3070_HANDLE_NULL;
        kontekst.texture[i] = GTX3070_HANDLE_NULL;
    }
    for (i = 0; i < GTX3070_MAX_CONSTANT_BUFFERS; i++) {
        kontekst.constant_buffer[i] = GTX3070_HANDLE_NULL;
    }
    kontekst.viewport_x = 0;
    kontekst.viewport_y = 0;
    kontekst.viewport_w = GTX3070_FB_WIDTH;
    kontekst.viewport_h = GTX3070_FB_HEIGHT;
    kontekst.viewport_min_depth = 0.0f;
    kontekst.viewport_max_depth = 1.0f;
    kontekst.scissor_left = 0;
    kontekst.scissor_top = 0;
    kontekst.scissor_right = GTX3070_FB_WIDTH;
    kontekst.scissor_bottom = GTX3070_FB_HEIGHT;
    kontekst.primitive_topology = GTX3070_PRIM_TRIANGLE;
    kontekst.alpha_blend_global = 0;
    kontekst.in_command_buffer = 0;

    mmio_base = 0;
    pci_bus = 0;
    pci_slot = 0;
    pci_funkcja = 0;
    ramka_systemowa = 0;
    wierzcholek_polecenia = 0;
    wierzcholek_pierwszy = 0;
    inicjalizowane = 0;
}

uint32_t Gtx3070::odczytaj_rejestr(uint32_t rejestr)
{
    return inl((uint16_t)(mmio_base + rejestr));
}

void Gtx3070::zapisz_rejestr(uint32_t rejestr, uint32_t wartosc)
{
    outl((uint16_t)(mmio_base + rejestr), wartosc);
}

uint32_t Gtx3070::pci_odczytaj32(uint8_t bus, uint8_t slot, uint8_t funkcja, uint8_t przesuniecie)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)funkcja << 8) |
                       (przesuniecie & 0xFC);
    outl(0xCF8, address);
    return inl(0xCFC);
}

void Gtx3070::pci_zapisz32(uint8_t bus, uint8_t slot, uint8_t funkcja, uint8_t przesuniecie, uint32_t wartosc)
{
    uint32_t address = 0x80000000U | ((uint32_t)bus << 16) |
                       ((uint32_t)slot << 11) | ((uint32_t)funkcja << 8) |
                       (przesuniecie & 0xFC);
    outl(0xCF8, address);
    outl(0xCFC, wartosc);
}

int Gtx3070::znajdz_urzadzenie(void)
{
    uint8_t bus;
    uint8_t slot;
    uint8_t funkcja;
    uint32_t id;
    uint16_t vendor;
    uint16_t device;
    uint32_t class_code;
    uint8_t base_class;
    uint8_t sub_class;
    uint32_t bar;

    for (bus = 0; bus < 256; bus++) {
        for (slot = 0; slot < 32; slot++) {
            for (funkcja = 0; funkcja < 8; funkcja++) {
                id = pci_odczytaj32(bus, slot, funkcja, 0x00);
                vendor = (uint16_t)id;
                if (vendor == 0xFFFF) {
                    continue;
                }
                device = (uint16_t)(id >> 16);
                class_code = pci_odczytaj32(bus, slot, funkcja, 0x08);
                base_class = (uint8_t)(class_code >> 24);
                sub_class = (uint8_t)((class_code >> 16) & 0xFF);
                if (base_class != 0x03) {
                    continue;
                }
                bar = pci_odczytaj32(bus, slot, funkcja, 0x24);
                if (bar == 0 || bar == 0xFFFFFFFF) {
                    continue;
                }
                dane.vendor_id = vendor;
                dane.device_id = device;
                dane.revision_id = (uint8_t)pci_odczytaj32(bus, slot, funkcja, 0x08);
                mmio_base = bar & 0xFFFFFFF0;
                pci_bus = bus;
                pci_slot = slot;
                pci_funkcja = funkcja;
                dane.mmio_base = mmio_base;
                dane.initialized = 1;
                pci_zapisz32(bus, slot, funkcja, 0x04, pci_odczytaj32(bus, slot, funkcja, 0x04) | 0x00000006);
                return 1;
            }
        }
    }
    return 0;
}

void Gtx3070::inicjalizuj_ring_buffer(void)
{
    zapisz_rejestr(REG_GTX3070_PGRAPH_CTX_CONTROL, 0x00000001U);
    zapisz_rejestr(REG_GTX3070_PFIFO_CACHE_DMA_PUT, 0x00000000U);
    zapisz_rejestr(REG_GTX3070_PFIFO_CACHE_DMA_GET, 0x00000000U);
    zapisz_rejestr(REG_GTX3070_PFIFO_CACHE_DMA_CTRL, 0x00000001U);
    zapisz_rejestr(REG_GTX3070_PFIFO_INTR_EN, 0x00000000U);
}

void Gtx3070::inicjalizuj_pamiec(void)
{
    uint32_t i;
    for (i = 0; i < GTX3070_VRAM_MAX_ALLOCS; i++) {
        alokacje_tab[i].used = 0;
        alokacje_tab[i].offset = 0;
        alokacje_tab[i].size = 0;
        alokacje_tab[i].type = 0;
    }
    dane.vram_used = 0;
}

void Gtx3070::inicjalizuj_wyswietlacz(void)
{
    zapisz_rejestr(REG_GTX3070_DISP_CRTC_CONTROL, 0x00000001U);
    zapisz_rejestr(REG_GTX3070_DISP_CRTC_H_TOTAL, GTX3070_FB_WIDTH);
    zapisz_rejestr(REG_GTX3070_DISP_CRTC_V_TOTAL, GTX3070_FB_HEIGHT);
    zapisz_rejestr(REG_GTX3070_DISP_CRTC_SURFACE, dane.fb_addr);
    zapisz_rejestr(REG_GTX3070_DISP_CRTC_PITCH, dane.fb_pitch);
    ramka_systemowa = (uint8_t *)dane.fb_addr;
}

void Gtx3070::resetuj_potok(void)
{
    zapisz_rejestr(REG_GTX3070_SHADER_PROGRAM, 0x00000000U);
    zapisz_rejestr(REG_GTX3070_DS_CONTROL, 0x00000000U);
    zapisz_rejestr(REG_GTX3070_BLEND_CONTROL, 0x00000000U);
    zapisz_rejestr(REG_GTX3070_RASTER_CONTROL, 0x00000000U);
    zapisz_rejestr(REG_GTX3070_ZS_CONTROL, 0x00000000U);
    zapisz_rejestr(REG_GTX3070_RT_CONTROL, 0x00000001U);
}

uint64_t Gtx3070::alokuj_vram(uint64_t rozmiar, uint32_t typ)
{
    uint32_t i;
    uint64_t offset = dane.vram_used;
    if (rozmiar == 0) {
        return GTX3070_HANDLE_NULL;
    }
    for (i = 0; i < GTX3070_VRAM_MAX_ALLOCS; i++) {
        if (!alokacje_tab[i].used) {
            alokacje_tab[i].used = 1;
            alokacje_tab[i].offset = offset;
            alokacje_tab[i].size = rozmiar;
            alokacje_tab[i].type = typ;
            dane.vram_used += rozmiar;
            return offset;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::zwolnij_vram(uint64_t uchwyt)
{
    uint32_t i;
    for (i = 0; i < GTX3070_VRAM_MAX_ALLOCS; i++) {
        if (alokacje_tab[i].used && alokacje_tab[i].offset == uchwyt) {
            alokacje_tab[i].used = 0;
            alokacje_tab[i].offset = 0;
            alokacje_tab[i].size = 0;
            alokacje_tab[i].type = 0;
            return;
        }
    }
}

int Gtx3070::inicjalizuj(void)
{
    if (!znajdz_urzadzenie()) {
        return -1;
    }
    inicjalizuj_ring_buffer();
    inicjalizuj_pamiec();
    inicjalizuj_wyswietlacz();
    resetuj_potok();
    inicjalizowane = 1;
    return 0;
}

void Gtx3070::zamknij(void)
{
    if (!inicjalizowane) {
        return;
    }
    zapisz_rejestr(REG_GTX3070_PGRAPH_CTX_CONTROL, 0x00000000U);
    inicjalizowane = 0;
}

uint8_t Gtx3070::czy_gotowy(void)
{
    return inicjalizowane;
}

void Gtx3070::putpixel(uint32_t x, uint32_t y, uint32_t kolor)
{
    if (!inicjalizowane) {
        return;
    }
    if (x >= dane.fb_width || y >= dane.fb_height) {
        return;
    }
    ((volatile uint32_t *)ramka_systemowa)[y * (dane.fb_pitch / 4) + x] = kolor;
}

void Gtx3070::fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t kolor)
{
    uint32_t py;
    uint32_t px;
    if (!inicjalizowane) {
        return;
    }
    if (x >= dane.fb_width || y >= dane.fb_height) {
        return;
    }
    if (x + w > dane.fb_width) {
        w = dane.fb_width - x;
    }
    if (y + h > dane.fb_height) {
        h = dane.fb_height - y;
    }
    for (py = y; py < y + h; py++) {
        for (px = x; px < x + w; px++) {
            ((volatile uint32_t *)ramka_systemowa)[py * (dane.fb_pitch / 4) + px] = kolor;
        }
    }
}

void Gtx3070::blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)
{
    uint32_t py;
    uint32_t px;
    uint32_t pitch = dane.fb_pitch / 4;
    if (!inicjalizowane) {
        return;
    }
    for (py = 0; py < h; py++) {
        for (px = 0; px < w; px++) {
            ((volatile uint32_t *)ramka_systemowa)[(dst_y + py) * pitch + (dst_x + px)] =
                ((volatile uint32_t *)ramka_systemowa)[(src_y + py) * pitch + (src_x + px)];
        }
    }
}

void Gtx3070::cursor_set(uint32_t x, uint32_t y, const uint32_t *obraz)
{
    uint32_t i;
    if (!inicjalizowane) {
        return;
    }
    zapisz_rejestr(REG_GTX3070_CURSOR_POSITION, (y << 16) | x);
    zapisz_rejestr(REG_GTX3070_CURSOR_CONTROL, 0x00000001U);
    for (i = 0; i < 64; i++) {
        ((volatile uint32_t *)(ramka_systemowa + 0x200000))[i] = obraz ? obraz[i] : 0x00000000;
    }
}

void Gtx3070::cursor_hide(void)
{
    if (!inicjalizowane) {
        return;
    }
    zapisz_rejestr(REG_GTX3070_CURSOR_CONTROL, 0x00000000U);
}

void *Gtx3070::get_fb(void)
{
    return ramka_systemowa;
}

uint32_t Gtx3070::get_szerokosc(void)
{
    return dane.fb_width;
}

uint32_t Gtx3070::get_wysokosc(void)
{
    return dane.fb_height;
}

uint16_t Gtx3070::get_device_id(void)
{
    return dane.device_id;
}

uint8_t Gtx3070::get_rewizja(void)
{
    return dane.revision_id;
}

void Gtx3070::info_pobierz(struct gtx3070_info *wyjscie)
{
    if (wyjscie != 0) {
        *wyjscie = dane;
    }
}

uint64_t Gtx3070::vram_alokuj(uint64_t rozmiar, uint32_t typ)
{
    return alokuj_vram(rozmiar, typ);
}

void Gtx3070::vram_zwolnij(uint64_t uchwyt)
{
    zwolnij_vram(uchwyt);
}

uint64_t Gtx3070::vram_uzyto(void)
{
    return dane.vram_used;
}

uint64_t Gtx3070::vram_calkowicie(void)
{
    return dane.vram_size;
}

gtx3070_handle_t Gtx3070::bufor_wierzcholkow_stworz(uint32_t liczba_wierzcholkow, uint32_t rozmiar_wierzcholka, uint32_t format, const void *dane_we)
{
    uint32_t i;
    for (i = 0; i < GTX3070_MAX_VERTEX_BUFFERS; i++) {
        if (!bufor_wierzcholkow_tab[i].used) {
            bufor_wierzcholkow_tab[i].used = 1;
            bufor_wierzcholkow_tab[i].vertex_count = liczba_wierzcholkow;
            bufor_wierzcholkow_tab[i].vertex_size = rozmiar_wierzcholka;
            bufor_wierzcholkow_tab[i].format = format;
            bufor_wierzcholkow_tab[i].buffer_addr = 0;
            bufor_wierzcholkow_tab[i].system_mem = (uint8_t *)dane_we;
            bufor_wierzcholkow_tab[i].vram_offset = alokuj_vram((uint64_t)liczba_wierzcholkow * (uint64_t)rozmiar_wierzcholka, 1);
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::bufor_wierzcholkow_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_VERTEX_BUFFERS) {
        return;
    }
    bufor_wierzcholkow_tab[uchwyt].used = 0;
    bufor_wierzcholkow_tab[uchwyt].vertex_count = 0;
    bufor_wierzcholkow_tab[uchwyt].vertex_size = 0;
    bufor_wierzcholkow_tab[uchwyt].format = 0;
    bufor_wierzcholkow_tab[uchwyt].buffer_addr = 0;
    bufor_wierzcholkow_tab[uchwyt].system_mem = 0;
    bufor_wierzcholkow_tab[uchwyt].vram_offset = 0;
    if (kontekst.vertex_buffer == uchwyt) {
        kontekst.vertex_buffer = GTX3070_HANDLE_NULL;
    }
}

void Gtx3070::bufor_wierzcholkow_zaktualizuj(gtx3070_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane_we)
{
    if (uchwyt >= GTX3070_MAX_VERTEX_BUFFERS || !bufor_wierzcholkow_tab[uchwyt].used) {
        return;
    }
    bufor_wierzcholkow_tab[uchwyt].system_mem = (uint8_t *)dane_we;
}

gtx3070_handle_t Gtx3070::bufor_indeksow_stworz(uint32_t liczba_indeksow, uint32_t format, const void *dane_we)
{
    uint32_t i;
    for (i = 0; i < GTX3070_MAX_INDEX_BUFFERS; i++) {
        if (!bufor_indeksow_tab[i].used) {
            bufor_indeksow_tab[i].used = 1;
            bufor_indeksow_tab[i].index_count = liczba_indeksow;
            bufor_indeksow_tab[i].format = format;
            bufor_indeksow_tab[i].buffer_addr = 0;
            bufor_indeksow_tab[i].system_mem = (uint8_t *)dane_we;
            bufor_indeksow_tab[i].vram_offset = alokuj_vram((uint64_t)liczba_indeksow * ((format == GTX3070_IDX_32BIT) ? 4ULL : 2ULL), 2);
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::bufor_indeksow_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_INDEX_BUFFERS) {
        return;
    }
    bufor_indeksow_tab[uchwyt].used = 0;
    bufor_indeksow_tab[uchwyt].index_count = 0;
    bufor_indeksow_tab[uchwyt].format = 0;
    bufor_indeksow_tab[uchwyt].buffer_addr = 0;
    bufor_indeksow_tab[uchwyt].system_mem = 0;
    bufor_indeksow_tab[uchwyt].vram_offset = 0;
    if (kontekst.index_buffer == uchwyt) {
        kontekst.index_buffer = GTX3070_HANDLE_NULL;
    }
}

void Gtx3070::bufor_indeksow_zaktualizuj(gtx3070_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane_we)
{
    if (uchwyt >= GTX3070_MAX_INDEX_BUFFERS || !bufor_indeksow_tab[uchwyt].used) {
        return;
    }
    bufor_indeksow_tab[uchwyt].system_mem = (uint8_t *)dane_we;
}

gtx3070_handle_t Gtx3070::tekstura_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint32_t poziomy_mip, const void *dane_we)
{
    uint32_t i;
    uint32_t bajt_na_piksel = 4;
    uint32_t rozmiar;
    for (i = 0; i < GTX3070_MAX_TEXTURES; i++) {
        if (!tekstury_tab[i].used) {
            tekstury_tab[i].used = 1;
            tekstury_tab[i].width = szerokosc;
            tekstury_tab[i].height = wysokosc;
            tekstury_tab[i].format = format;
            tekstury_tab[i].mip_levels = poziomy_mip;
            tekstury_tab[i].buffer_addr = 0;
            tekstury_tab[i].system_mem = (uint8_t *)dane_we;
            tekstury_tab[i].pitch = szerokosc * bajt_na_piksel;
            rozmiar = (uint64_t)szerokosc * (uint64_t)wysokosc * (uint64_t)bajt_na_piksel;
            tekstury_tab[i].vram_offset = alokuj_vram(rozmiar, 3);
            tekstury_tab[i].compressed = (format == GTX3070_TEX_DXT1 || format == GTX3070_TEX_DXT3 || format == GTX3070_TEX_DXT5) ? 1 : 0;
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::tekstura_zniszcz(gtx3070_handle_t uchwyt)
{
    uint32_t i;
    if (uchwyt >= GTX3070_MAX_TEXTURES) {
        return;
    }
    tekstury_tab[uchwyt].used = 0;
    tekstury_tab[uchwyt].width = 0;
    tekstury_tab[uchwyt].height = 0;
    tekstury_tab[uchwyt].format = 0;
    tekstury_tab[uchwyt].mip_levels = 0;
    tekstury_tab[uchwyt].buffer_addr = 0;
    tekstury_tab[uchwyt].system_mem = 0;
    tekstury_tab[uchwyt].pitch = 0;
    tekstury_tab[uchwyt].vram_offset = 0;
    tekstury_tab[uchwyt].compressed = 0;
    for (i = 0; i < GTX3070_MAX_SAMPLERS; i++) {
        if (kontekst.texture[i] == uchwyt) {
            kontekst.texture[i] = GTX3070_HANDLE_NULL;
        }
    }
}

void Gtx3070::tekstura_zaktualizuj(gtx3070_handle_t uchwyt, uint32_t poziom, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *dane_we)
{
    if (uchwyt >= GTX3070_MAX_TEXTURES || !tekstury_tab[uchwyt].used) {
        return;
    }
    tekstury_tab[uchwyt].system_mem = (uint8_t *)dane_we;
}

void Gtx3070::tekstura_wygeneruj_mipmapy(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_TEXTURES || !tekstury_tab[uchwyt].used) {
        return;
    }
    if (tekstury_tab[uchwyt].mip_levels > 1) {
        tekstury_tab[uchwyt].mip_levels--;
    }
}

gtx3070_handle_t Gtx3070::cel_renderowania_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint8_t alpha_blend)
{
    uint32_t i;
    uint64_t rozmiar = (uint64_t)szerokosc * (uint64_t)wysokosc * 4ULL;
    for (i = 0; i < GTX3070_MAX_RENDER_TARGETS; i++) {
        if (!cele_tab[i].used) {
            cele_tab[i].used = 1;
            cele_tab[i].width = szerokosc;
            cele_tab[i].height = wysokosc;
            cele_tab[i].format = format;
            cele_tab[i].buffer_addr = 0;
            cele_tab[i].alpha_blend = alpha_blend;
            cele_tab[i].vram_offset = alokuj_vram(rozmiar, 4);
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::cel_renderowania_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_RENDER_TARGETS) {
        return;
    }
    cele_tab[uchwyt].used = 0;
    cele_tab[uchwyt].width = 0;
    cele_tab[uchwyt].height = 0;
    cele_tab[uchwyt].format = 0;
    cele_tab[uchwyt].buffer_addr = 0;
    cele_tab[uchwyt].alpha_blend = 0;
    cele_tab[uchwyt].vram_offset = 0;
    if (kontekst.render_target == uchwyt) {
        kontekst.render_target = GTX3070_HANDLE_NULL;
    }
}

gtx3070_handle_t Gtx3070::bufor_glebii_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint8_t kompresja)
{
    uint32_t i;
    uint64_t rozmiar = (uint64_t)szerokosc * (uint64_t)wysokosc * 4ULL;
    for (i = 0; i < GTX3070_MAX_DEPTH_STENCILS; i++) {
        if (!bufor_glebii_tab[i].used) {
            bufor_glebii_tab[i].used = 1;
            bufor_glebii_tab[i].width = szerokosc;
            bufor_glebii_tab[i].height = wysokosc;
            bufor_glebii_tab[i].format = format;
            bufor_glebii_tab[i].buffer_addr = 0;
            bufor_glebii_tab[i].compression = kompresja;
            bufor_glebii_tab[i].vram_offset = alokuj_vram(rozmiar, 5);
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::bufor_glebii_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_DEPTH_STENCILS) {
        return;
    }
    bufor_glebii_tab[uchwyt].used = 0;
    bufor_glebii_tab[uchwyt].width = 0;
    bufor_glebii_tab[uchwyt].height = 0;
    bufor_glebii_tab[uchwyt].format = 0;
    bufor_glebii_tab[uchwyt].buffer_addr = 0;
    bufor_glebii_tab[uchwyt].compression = 0;
    bufor_glebii_tab[uchwyt].vram_offset = 0;
    if (kontekst.depth_stencil == uchwyt) {
        kontekst.depth_stencil = GTX3070_HANDLE_NULL;
    }
}

gtx3070_handle_t Gtx3070::shader_stworz(uint32_t typ, const uint8_t *kod, uint32_t rozmiar_kodu)
{
    uint32_t i;
    for (i = 0; i < GTX3070_MAX_SHADERS; i++) {
        if (!shadery_tab[i].used) {
            shadery_tab[i].used = 1;
            shadery_tab[i].type = typ;
            shadery_tab[i].code_size = rozmiar_kodu;
            shadery_tab[i].code = (uint8_t *)kod;
            shadery_tab[i].constant_count = 0;
            shadery_tab[i].compiled = 0;
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::shader_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_SHADERS) {
        return;
    }
    shadery_tab[uchwyt].used = 0;
    shadery_tab[uchwyt].type = 0;
    shadery_tab[uchwyt].code_size = 0;
    shadery_tab[uchwyt].code = 0;
    shadery_tab[uchwyt].constant_count = 0;
    shadery_tab[uchwyt].compiled = 0;
    if (kontekst.vertex_shader == uchwyt) kontekst.vertex_shader = GTX3070_HANDLE_NULL;
    if (kontekst.pixel_shader == uchwyt) kontekst.pixel_shader = GTX3070_HANDLE_NULL;
    if (kontekst.geometry_shader == uchwyt) kontekst.geometry_shader = GTX3070_HANDLE_NULL;
    if (kontekst.compute_shader == uchwyt) kontekst.compute_shader = GTX3070_HANDLE_NULL;
}

void Gtx3070::shader_skompiluj(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_SHADERS || !shadery_tab[uchwyt].used) {
        return;
    }
    shadery_tab[uchwyt].compiled = 1;
    zapisz_rejestr(REG_GTX3070_SHADER_PROGRAM, uchwyt);
}

gtx3070_handle_t Gtx3070::stan_probnika_stworz(uint32_t filtr_min, uint32_t filtr_mag, uint32_t filtr_mip, uint32_t adres_u, uint32_t adres_v, uint32_t adres_w, uint32_t max_anizotropia, uint8_t wlacz_porownanie, uint8_t func_porownania)
{
    uint32_t i;
    for (i = 0; i < GTX3070_MAX_SAMPLERS; i++) {
        if (!probniki_tab[i].used) {
            probniki_tab[i].used = 1;
            probniki_tab[i].filter_min = filtr_min;
            probniki_tab[i].filter_mag = filtr_mag;
            probniki_tab[i].filter_mip = filtr_mip;
            probniki_tab[i].address_u = adres_u;
            probniki_tab[i].address_v = adres_v;
            probniki_tab[i].address_w = adres_w;
            probniki_tab[i].max_anisotropy = max_anizotropia;
            probniki_tab[i].compare_enable = wlacz_porownanie;
            probniki_tab[i].compare_func = func_porownania;
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::stan_probnika_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_SAMPLERS) {
        return;
    }
    probniki_tab[uchwyt].used = 0;
    probniki_tab[uchwyt].filter_min = 0;
    probniki_tab[uchwyt].filter_mag = 0;
    probniki_tab[uchwyt].filter_mip = 0;
    probniki_tab[uchwyt].address_u = 0;
    probniki_tab[uchwyt].address_v = 0;
    probniki_tab[uchwyt].address_w = 0;
    probniki_tab[uchwyt].max_anisotropy = 0;
    probniki_tab[uchwyt].compare_enable = 0;
    probniki_tab[uchwyt].compare_func = 0;
    if (kontekst.sampler[uchwyt] == uchwyt) {
        kontekst.sampler[uchwyt] = GTX3070_HANDLE_NULL;
    }
}

gtx3070_handle_t Gtx3070::stan_mieszania_stworz(uint8_t wlacz_mieszanie, uint32_t src_blend, uint32_t dest_blend, uint32_t op_mieszania, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t op_mieszania_alpha, uint32_t maska_zapisu, uint32_t blend_factor)
{
    uint32_t i;
    for (i = 0; i < GTX3070_MAX_BLEND_STATES; i++) {
        if (!stany_mieszania_tab[i].used) {
            stany_mieszania_tab[i].used = 1;
            stany_mieszania_tab[i].blend_enable = wlacz_mieszanie;
            stany_mieszania_tab[i].src_blend = src_blend;
            stany_mieszania_tab[i].dest_blend = dest_blend;
            stany_mieszania_tab[i].blend_op = op_mieszania;
            stany_mieszania_tab[i].src_blend_alpha = src_blend_alpha;
            stany_mieszania_tab[i].dest_blend_alpha = dest_blend_alpha;
            stany_mieszania_tab[i].blend_op_alpha = op_mieszania_alpha;
            stany_mieszania_tab[i].render_target_write_mask = maska_zapisu;
            stany_mieszania_tab[i].blend_factor = blend_factor;
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::stan_mieszania_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_BLEND_STATES) {
        return;
    }
    stany_mieszania_tab[uchwyt].used = 0;
    stany_mieszania_tab[uchwyt].blend_enable = 0;
    stany_mieszania_tab[uchwyt].src_blend = 0;
    stany_mieszania_tab[uchwyt].dest_blend = 0;
    stany_mieszania_tab[uchwyt].blend_op = 0;
    stany_mieszania_tab[uchwyt].src_blend_alpha = 0;
    stany_mieszania_tab[uchwyt].dest_blend_alpha = 0;
    stany_mieszania_tab[uchwyt].blend_op_alpha = 0;
    stany_mieszania_tab[uchwyt].render_target_write_mask = 0;
    stany_mieszania_tab[uchwyt].blend_factor = 0;
    if (kontekst.blend_state == uchwyt) {
        kontekst.blend_state = GTX3070_HANDLE_NULL;
    }
}

gtx3070_handle_t Gtx3070::stan_rasteryzacji_stworz(uint32_t tryb_wypelnienia, uint32_t tryb_odcinania, uint8_t front_ccw, uint8_t clip_glebii, uint8_t scissor, uint8_t wielopr_probowanie, uint8_t aa_linia, uint8_t alpha_do_pokrycia)
{
    uint32_t i;
    for (i = 0; i < GTX3070_MAX_RASTERIZER_STATES; i++) {
        if (!stany_rasteryzacji_tab[i].used) {
            stany_rasteryzacji_tab[i].used = 1;
            stany_rasteryzacji_tab[i].fill_mode = tryb_wypelnienia;
            stany_rasteryzacji_tab[i].cull_mode = tryb_odcinania;
            stany_rasteryzacji_tab[i].front_counter_clockwise = front_ccw;
            stany_rasteryzacji_tab[i].depth_clip_enable = clip_glebii;
            stany_rasteryzacji_tab[i].scissor_enable = scissor;
            stany_rasteryzacji_tab[i].multisample_enable = wielopr_probowanie;
            stany_rasteryzacji_tab[i].antialiased_line_enable = aa_linia;
            stany_rasteryzacji_tab[i].alpha_to_coverage = alpha_do_pokrycia;
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::stan_rasteryzacji_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_RASTERIZER_STATES) {
        return;
    }
    stany_rasteryzacji_tab[uchwyt].used = 0;
    stany_rasteryzacji_tab[uchwyt].fill_mode = 0;
    stany_rasteryzacji_tab[uchwyt].cull_mode = 0;
    stany_rasteryzacji_tab[uchwyt].front_counter_clockwise = 0;
    stany_rasteryzacji_tab[uchwyt].depth_clip_enable = 0;
    stany_rasteryzacji_tab[uchwyt].scissor_enable = 0;
    stany_rasteryzacji_tab[uchwyt].multisample_enable = 0;
    stany_rasteryzacji_tab[uchwyt].antialiased_line_enable = 0;
    stany_rasteryzacji_tab[uchwyt].alpha_to_coverage = 0;
    if (kontekst.rasterizer_state == uchwyt) {
        kontekst.rasterizer_state = GTX3070_HANDLE_NULL;
    }
}

gtx3070_handle_t Gtx3070::stan_glebii_stencil_stworz(uint8_t wlacz_glebie, uint32_t maska_zapisu_glebii, uint32_t func_glebii, uint8_t wlacz_stencil, uint8_t maska_odczytu_stencil, uint8_t maska_zapisu_stencil, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func)
{
    uint32_t i;
    for (i = 0; i < GTX3070_MAX_DEPTH_STENCIL_STATES; i++) {
        if (!stany_glebii_tab[i].used) {
            stany_glebii_tab[i].used = 1;
            stany_glebii_tab[i].depth_enable = wlacz_glebie;
            stany_glebii_tab[i].depth_write_mask = maska_zapisu_glebii;
            stany_glebii_tab[i].depth_func = func_glebii;
            stany_glebii_tab[i].stencil_enable = wlacz_stencil;
            stany_glebii_tab[i].stencil_read_mask = maska_odczytu_stencil;
            stany_glebii_tab[i].stencil_write_mask = maska_zapisu_stencil;
            stany_glebii_tab[i].front_stencil_fail_op = front_fail;
            stany_glebii_tab[i].front_stencil_depth_fail_op = front_zfail;
            stany_glebii_tab[i].front_stencil_pass_op = front_pass;
            stany_glebii_tab[i].front_stencil_func = front_func;
            stany_glebii_tab[i].back_stencil_fail_op = back_fail;
            stany_glebii_tab[i].back_stencil_depth_fail_op = back_zfail;
            stany_glebii_tab[i].back_stencil_pass_op = back_pass;
            stany_glebii_tab[i].back_stencil_func = back_func;
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::stan_glebii_stencil_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_DEPTH_STENCIL_STATES) {
        return;
    }
    stany_glebii_tab[uchwyt].used = 0;
    stany_glebii_tab[uchwyt].depth_enable = 0;
    stany_glebii_tab[uchwyt].depth_write_mask = 0;
    stany_glebii_tab[uchwyt].depth_func = 0;
    stany_glebii_tab[uchwyt].stencil_enable = 0;
    stany_glebii_tab[uchwyt].stencil_read_mask = 0;
    stany_glebii_tab[uchwyt].stencil_write_mask = 0;
    stany_glebii_tab[uchwyt].front_stencil_fail_op = 0;
    stany_glebii_tab[uchwyt].front_stencil_depth_fail_op = 0;
    stany_glebii_tab[uchwyt].front_stencil_pass_op = 0;
    stany_glebii_tab[uchwyt].front_stencil_func = 0;
    stany_glebii_tab[uchwyt].back_stencil_fail_op = 0;
    stany_glebii_tab[uchwyt].back_stencil_depth_fail_op = 0;
    stany_glebii_tab[uchwyt].back_stencil_pass_op = 0;
    stany_glebii_tab[uchwyt].back_stencil_func = 0;
    if (kontekst.depth_stencil_state == uchwyt) {
        kontekst.depth_stencil_state = GTX3070_HANDLE_NULL;
    }
}

gtx3070_handle_t Gtx3070::uklad_wejsc_stworz(uint32_t liczba_elementow, const uint32_t *sloty, const uint32_t *przesuniecia, const uint32_t *formaty)
{
    uint32_t i;
    uint32_t j;
    for (i = 0; i < GTX3070_MAX_INPUT_LAYOUTS; i++) {
        if (!uklady_tab[i].used) {
            uklady_tab[i].used = 1;
            uklady_tab[i].element_count = liczba_elementow;
            for (j = 0; j < 32 && j < liczba_elementow; j++) {
                uklady_tab[i].slot[j] = sloty ? sloty[j] : 0;
                uklady_tab[i].offset[j] = przesuniecia ? przesuniecia[j] : 0;
                uklady_tab[i].format[j] = formaty ? formaty[j] : 0;
            }
            for (; j < 32; j++) {
                uklady_tab[i].slot[j] = 0;
                uklady_tab[i].offset[j] = 0;
                uklady_tab[i].format[j] = 0;
            }
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::uklad_wejsc_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_INPUT_LAYOUTS) {
        return;
    }
    uklady_tab[uchwyt].used = 0;
    uklady_tab[uchwyt].element_count = 0;
    if (kontekst.input_layout == uchwyt) {
        kontekst.input_layout = GTX3070_HANDLE_NULL;
    }
}

gtx3070_handle_t Gtx3070::bufor_stale_stworz(uint32_t rozmiar)
{
    uint32_t i;
    for (i = 0; i < GTX3070_MAX_CONSTANT_BUFFERS; i++) {
        if (!bufor_stale_tab[i].used) {
            bufor_stale_tab[i].used = 1;
            bufor_stale_tab[i].size = rozmiar;
            bufor_stale_tab[i].data = 0;
            bufor_stale_tab[i].vram_offset = alokuj_vram(rozmiar, 6);
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::bufor_stale_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_CONSTANT_BUFFERS) {
        return;
    }
    bufor_stale_tab[uchwyt].used = 0;
    bufor_stale_tab[uchwyt].size = 0;
    bufor_stale_tab[uchwyt].data = 0;
    bufor_stale_tab[uchwyt].vram_offset = 0;
    if (kontekst.constant_buffer[uchwyt] == uchwyt) {
        kontekst.constant_buffer[uchwyt] = GTX3070_HANDLE_NULL;
    }
}

void Gtx3070::bufor_stale_zaktualizuj(gtx3070_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane_we)
{
    if (uchwyt >= GTX3070_MAX_CONSTANT_BUFFERS || !bufor_stale_tab[uchwyt].used) {
        return;
    }
    bufor_stale_tab[uchwyt].data = (uint8_t *)dane_we;
}

void Gtx3070::powiaz_bufor_wierzcholkow(gtx3070_handle_t uchwyt)
{
    kontekst.vertex_buffer = uchwyt;
    zapisz_rejestr(REG_GTX3070_SHADER_VTX_BASE, uchwyt);
}

void Gtx3070::powiaz_bufor_indeksow(gtx3070_handle_t uchwyt)
{
    kontekst.index_buffer = uchwyt;
}

void Gtx3070::powiaz_uklad_wejsc(gtx3070_handle_t uchwyt)
{
    kontekst.input_layout = uchwyt;
}

void Gtx3070::powiaz_shader_wierzcholkow(gtx3070_handle_t uchwyt)
{
    kontekst.vertex_shader = uchwyt;
    shader_skompiluj(uchwyt);
}

void Gtx3070::powiaz_shader_pikseli(gtx3070_handle_t uchwyt)
{
    kontekst.pixel_shader = uchwyt;
    shader_skompiluj(uchwyt);
}

void Gtx3070::powiaz_shader_geometrii(gtx3070_handle_t uchwyt)
{
    kontekst.geometry_shader = uchwyt;
    shader_skompiluj(uchwyt);
}

void Gtx3070::powiaz_shader_obliczeniowy(gtx3070_handle_t uchwyt)
{
    kontekst.compute_shader = uchwyt;
    shader_skompiluj(uchwyt);
}

void Gtx3070::powiaz_cel_renderowania(gtx3070_handle_t uchwyt)
{
    kontekst.render_target = uchwyt;
    if (uchwyt != GTX3070_HANDLE_NULL && uchwyt < GTX3070_MAX_RENDER_TARGETS) {
        zapisz_rejestr(REG_GTX3070_RT_BASE0, cele_tab[uchwyt].vram_offset);
        zapisz_rejestr(REG_GTX3070_RT_PITCH0, cele_tab[uchwyt].width * 4);
        zapisz_rejestr(REG_GTX3070_RT_FORMAT0, cele_tab[uchwyt].format);
        zapisz_rejestr(REG_GTX3070_RT_CONTROL, cele_tab[uchwyt].alpha_blend ? 0x00000001U : 0x00000000U);
    }
}

void Gtx3070::powiaz_bufor_glebii(gtx3070_handle_t uchwyt)
{
    kontekst.depth_stencil = uchwyt;
    if (uchwyt != GTX3070_HANDLE_NULL && uchwyt < GTX3070_MAX_DEPTH_STENCILS) {
        zapisz_rejestr(REG_GTX3070_DS_BASE, bufor_glebii_tab[uchwyt].vram_offset);
        zapisz_rejestr(REG_GTX3070_DS_FORMAT, bufor_glebii_tab[uchwyt].format);
    }
}

void Gtx3070::powiaz_stan_mieszania(gtx3070_handle_t uchwyt)
{
    kontekst.blend_state = uchwyt;
    if (uchwyt != GTX3070_HANDLE_NULL && uchwyt < GTX3070_MAX_BLEND_STATES) {
        zapisz_rejestr(REG_GTX3070_BLEND_CONTROL, stany_mieszania_tab[uchwyt].blend_enable ? 0x00000001U : 0x00000000U);
        zapisz_rejestr(REG_GTX3070_BLEND_SRC, stany_mieszania_tab[uchwyt].src_blend);
        zapisz_rejestr(REG_GTX3070_BLEND_DST, stany_mieszania_tab[uchwyt].dest_blend);
        zapisz_rejestr(REG_GTX3070_BLEND_OP, stany_mieszania_tab[uchwyt].blend_op);
    }
}

void Gtx3070::powiaz_stan_rasteryzacji(gtx3070_handle_t uchwyt)
{
    kontekst.rasterizer_state = uchwyt;
    if (uchwyt != GTX3070_HANDLE_NULL && uchwyt < GTX3070_MAX_RASTERIZER_STATES) {
        zapisz_rejestr(REG_GTX3070_RASTER_CONTROL, stany_rasteryzacji_tab[uchwyt].fill_mode);
        zapisz_rejestr(REG_GTX3070_RASTER_CULL, stany_rasteryzacji_tab[uchwyt].cull_mode);
    }
}

void Gtx3070::powiaz_stan_glebii_stencil(gtx3070_handle_t uchwyt)
{
    kontekst.depth_stencil_state = uchwyt;
    zapisz_rejestr(REG_GTX3070_ZS_CONTROL, 0x00000001U);
    zapisz_rejestr(REG_GTX3070_ZS_FUNC, GTX3070_CMP_LESS);
}

void Gtx3070::powiaz_probnik_i_teksture(uint32_t gniazdo, gtx3070_handle_t probnik, gtx3070_handle_t tekstura)
{
    if (gniazdo >= GTX3070_MAX_SAMPLERS) {
        return;
    }
    kontekst.sampler[gniazdo] = probnik;
    kontekst.texture[gniazdo] = tekstura;
    zapisz_rejestr(REG_GTX3070_TEX_BASE + (gniazdo * 4), tekstura != GTX3070_HANDLE_NULL ? (uint32_t)tekstury_tab[tekstura].vram_offset : 0);
    zapisz_rejestr(REG_GTX3070_TEX_SAMPLER + (gniazdo * 4), probnik != GTX3070_HANDLE_NULL ? probnik : 0);
}

void Gtx3070::powiaz_bufor_stale(uint32_t gniazdo, gtx3070_handle_t uchwyt)
{
    if (gniazdo >= GTX3070_MAX_CONSTANT_BUFFERS) {
        return;
    }
    kontekst.constant_buffer[gniazdo] = uchwyt;
    zapisz_rejestr(REG_GTX3070_SHADER_CONSTANT + (gniazdo * 4), uchwyt != GTX3070_HANDLE_NULL ? (uint32_t)bufor_stale_tab[uchwyt].vram_offset : 0);
}

void Gtx3070::ustaw_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_glebia, float max_glebia)
{
    kontekst.viewport_x = x;
    kontekst.viewport_y = y;
    kontekst.viewport_w = w;
    kontekst.viewport_h = h;
    kontekst.viewport_min_depth = min_glebia;
    kontekst.viewport_max_depth = max_glebia;
}

void Gtx3070::ustaw_scissor(uint32_t lewa, uint32_t gora, uint32_t prawa, uint32_t dol)
{
    kontekst.scissor_left = lewa;
    kontekst.scissor_top = gora;
    kontekst.scissor_right = prawa;
    kontekst.scissor_bottom = dol;
}

void Gtx3070::ustaw_typ_pierwotnego(uint32_t typ_pierwotnego)
{
    kontekst.primitive_topology = typ_pierwotnego;
}

void Gtx3070::ustaw_mieszanie_alpha_globalne(uint8_t wlacz)
{
    kontekst.alpha_blend_global = wlacz;
}

void Gtx3070::wyczysc_cel_renderowania(gtx3070_handle_t cel, uint32_t kolor)
{
    if (cel >= GTX3070_MAX_RENDER_TARGETS || !cele_tab[cel].used) {
        return;
    }
}

void Gtx3070::wyczysc_bufor_glebii(gtx3070_handle_t bufor, float glebia, uint8_t stencil)
{
    (void)glebia;
    (void)stencil;
    if (bufor >= GTX3070_MAX_DEPTH_STENCILS || !bufor_glebii_tab[bufor].used) {
        return;
    }
}

void Gtx3070::rysuj(uint32_t liczba_wierzcholkow, uint32_t pierwszy_wierzcholek)
{
    if (!inicjalizowane) {
        return;
    }
    if (kontekst.vertex_shader == GTX3070_HANDLE_NULL || kontekst.pixel_shader == GTX3070_HANDLE_NULL) {
        return;
    }
    if (kontekst.render_target == GTX3070_HANDLE_NULL) {
        return;
    }
    zapisz_rejestr(REG_GTX3070_PFIFO_CACHE_DMA_PUT, (wierzcholek_polecenia + 1) % GTX3070_RING_BUFFER_SIZE);
}

void Gtx3070::rysuj_indeksowane(uint32_t liczba_indeksow, uint32_t pierwszy_indeks, int32_t baza_wierzcholka)
{
    (void)baza_wierzcholka;
    if (!inicjalizowane) {
        return;
    }
    if (kontekst.vertex_shader == GTX3070_HANDLE_NULL || kontekst.pixel_shader == GTX3070_HANDLE_NULL) {
        return;
    }
    if (kontekst.render_target == GTX3070_HANDLE_NULL) {
        return;
    }
    if (kontekst.index_buffer == GTX3070_HANDLE_NULL) {
        return;
    }
    zapisz_rejestr(REG_GTX3070_PFIFO_CACHE_DMA_PUT, (wierzcholek_polecenia + 1) % GTX3070_RING_BUFFER_SIZE);
}

void Gtx3070::rysuj_indeksowane_instancjonowane(uint32_t liczba_indeksow, uint32_t pierwszy_indeks, int32_t baza_wierzcholka, uint32_t liczba_instancji, uint32_t pierwsza_instancja)
{
    (void)baza_wierzcholka;
    (void)liczba_instancji;
    (void)pierwsza_instancja;
    rysuj_indeksowane(liczba_indeksow, pierwszy_indeks, 0);
}

void Gtx3070::rysuj_po_indirekcie(uint32_t adres_bufora, uint32_t liczba_rysowan, uint32_t krok)
{
    (void)adres_bufora;
    (void)liczba_rysowan;
    (void)krok;
    if (!inicjalizowane) {
        return;
    }
    zapisz_rejestr(REG_GTX3070_PFIFO_CACHE_DMA_PUT, (wierzcholek_polecenia + 1) % GTX3070_RING_BUFFER_SIZE);
}

void Gtx3070::oblicz_wyslij(uint32_t grupa_x, uint32_t grupa_y, uint32_t grupa_z)
{
    if (!inicjalizowane) {
        return;
    }
    if (kontekst.compute_shader == GTX3070_HANDLE_NULL) {
        return;
    }
    zapisz_rejestr(REG_GTX3070_COMPUTE_PROGRAM, kontekst.compute_shader);
    zapisz_rejestr(REG_GTX3070_COMPUTE_DIM, (grupa_x << 16) | (grupa_y & 0xFFFF));
    zapisz_rejestr(REG_GTX3070_PFIFO_CACHE_DMA_PUT, (wierzcholek_polecenia + 1) % GTX3070_RING_BUFFER_SIZE);
}

void Gtx3070::oblicz_wyslij_po_indirekcie(uint32_t adres_bufora)
{
    (void)adres_bufora;
    if (!inicjalizowane) {
        return;
    }
    zapisz_rejestr(REG_GTX3070_PFIFO_CACHE_DMA_PUT, (wierzcholek_polecenia + 1) % GTX3070_RING_BUFFER_SIZE);
}

void Gtx3070::ogrodzenie_stworz(gtx3070_handle_t *wyjscie)
{
    uint32_t i;
    if (wyjscie == 0) {
        return;
    }
    for (i = 0; i < GTX3070_MAX_FENCES; i++) {
        if (!ogrodzenia_tab[i].used) {
            ogrodzenia_tab[i].used = 1;
            ogrodzenia_tab[i].value = 0;
            ogrodzenia_tab[i].signaled = 0;
            *wyjscie = (gtx3070_handle_t)i;
            return;
        }
    }
    *wyjscie = GTX3070_HANDLE_NULL;
}

void Gtx3070::ogrodzenie_sygnalizuj(gtx3070_handle_t uchwyt, uint32_t wartosc)
{
    if (uchwyt >= GTX3070_MAX_FENCES || !ogrodzenia_tab[uchwyt].used) {
        return;
    }
    ogrodzenia_tab[uchwyt].value = wartosc;
    ogrodzenia_tab[uchwyt].signaled = 1;
}

int Gtx3070::ogrodzenie_poczekaj(gtx3070_handle_t uchwyt, uint32_t wartosc, uint32_t timeout_us)
{
    uint32_t czekaj = 0;
    if (uchwyt >= GTX3070_MAX_FENCES || !ogrodzenia_tab[uchwyt].used) {
        return -1;
    }
    while (!ogrodzenia_tab[uchwyt].signaled || ogrodzenia_tab[uchwyt].value < wartosc) {
        czekaj++;
        if (czekaj > timeout_us * 1000U) {
            return -1;
        }
    }
    return 0;
}

void Gtx3070::ogrodzenie_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_FENCES) {
        return;
    }
    ogrodzenia_tab[uchwyt].used = 0;
    ogrodzenia_tab[uchwyt].value = 0;
    ogrodzenia_tab[uchwyt].signaled = 0;
}

void Gtx3070::semafor_stworz(gtx3070_handle_t *wyjscie)
{
    uint32_t i;
    if (wyjscie == 0) {
        return;
    }
    for (i = 0; i < GTX3070_MAX_SEMAPHORES; i++) {
        if (!semafory_tab[i].used) {
            semafory_tab[i].used = 1;
            semafory_tab[i].value = 0;
            *wyjscie = (gtx3070_handle_t)i;
            return;
        }
    }
    *wyjscie = GTX3070_HANDLE_NULL;
}

void Gtx3070::semafor_sygnalizuj(gtx3070_handle_t uchwyt, uint32_t wartosc)
{
    if (uchwyt >= GTX3070_MAX_SEMAPHORES || !semafory_tab[uchwyt].used) {
        return;
    }
    semafory_tab[uchwyt].value = wartosc;
}

void Gtx3070::semafor_poczekaj(gtx3070_handle_t uchwyt, uint32_t wartosc)
{
    if (uchwyt >= GTX3070_MAX_SEMAPHORES || !semafory_tab[uchwyt].used) {
        return;
    }
    while (semafory_tab[uchwyt].value < wartosc) {
    }
}

void Gtx3070::semafor_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_SEMAPHORES) {
        return;
    }
    semafory_tab[uchwyt].used = 0;
    semafory_tab[uchwyt].value = 0;
}

int Gtx3070::czekaj_na_bezczynnosc(uint32_t timeout_us)
{
    uint32_t czekaj = 0;
    while (1) {
        czekaj++;
        if (czekaj > timeout_us * 1000U) {
            return -1;
        }
    }
    return 0;
}

uint64_t Gtx3070::znacznik_czasu(void)
{
    uint32_t lo = odczytaj_rejestr(REG_GTX3070_TIMESTAMP_LO);
    uint32_t hi = odczytaj_rejestr(REG_GTX3070_TIMESTAMP_HI);
    return ((uint64_t)hi << 32) | (uint64_t)lo;
}

void Gtx3070::ustaw_stan_zasilania(uint8_t stan)
{
    zapisz_rejestr(REG_GTX3070_PBUS_PWR, stan & 0x03U);
}

uint8_t Gtx3070::pobierz_stan_zasilania(void)
{
    return (uint8_t)(odczytaj_rejestr(REG_GTX3070_PBUS_PWR) & 0x03U);
}

void Gtx3070::ustaw_zegar_graficzny(uint32_t mhz)
{
    dane.gfx_clock_mhz = mhz;
    zapisz_rejestr(REG_GTX3070_PCLOCK_GFX, mhz);
}

void Gtx3070::ustaw_zegar_pamieci(uint32_t mhz)
{
    dane.memory_clock_mhz = mhz;
    zapisz_rejestr(REG_GTX3070_PCLOCK_MEM, mhz);
}

uint32_t Gtx3070::pobierz_zegar_graficzny(void)
{
    return dane.gfx_clock_mhz;
}

uint32_t Gtx3070::pobierz_zegar_pamieci(void)
{
    return dane.memory_clock_mhz;
}

uint32_t Gtx3070::pobierz_temperature(void)
{
    return odczytaj_rejestr(REG_GTX3070_PCOUNTER_TEMP);
}

void Gtx3070::ustaw_predkosc_wiatraka(uint32_t obroty)
{
    dane.fan_rpm = obroty;
    zapisz_rejestr(REG_GTX3070_PFAN_PWM, obroty);
    zapisz_rejestr(REG_GTX3070_PFAN_RPM, obroty);
}

uint32_t Gtx3070::pobierz_predkosc_wiatraka(void)
{
    return dane.fan_rpm;
}

void Gtx3070::ustaw_rgb(uint32_t kolor, uint8_t tryb)
{
    zapisz_rejestr(REG_GTX3070_PRGB_CONTROL, tryb);
    zapisz_rejestr(REG_GTX3070_PRGB_COLOR, kolor);
}

void Gtx3070::wylacz_rgb(void)
{
    zapisz_rejestr(REG_GTX3070_PRGB_CONTROL, 0x00000000U);
}

gtx3070_handle_t Gtx3070::lancuch_stworz(uint32_t szerokosc, uint32_t wysokosc, uint8_t vsync, uint8_t alpha_premultiplied)
{
    uint32_t i;
    for (i = 0; i < GTX3070_MAX_SWAP_CHAINS; i++) {
        if (!lancuchy_tab[i].used) {
            lancuchy_tab[i].used = 1;
            lancuchy_tab[i].width = szerokosc;
            lancuchy_tab[i].height = wysokosc;
            lancuchy_tab[i].back_buffer_addr = 0;
            lancuchy_tab[i].front_buffer_addr = dane.fb_addr;
            lancuchy_tab[i].vsync = vsync;
            lancuchy_tab[i].alpha_premultiplied = alpha_premultiplied;
            lancuchy_tab[i].back_vram_offset = alokuj_vram((uint64_t)szerokosc * (uint64_t)wysokosc * 4ULL, 7);
            lancuchy_tab[i].front_vram_offset = dane.fb_addr;
            return (gtx3070_handle_t)i;
        }
    }
    return GTX3070_HANDLE_NULL;
}

void Gtx3070::lancuch_zniszcz(gtx3070_handle_t uchwyt)
{
    if (uchwyt >= GTX3070_MAX_SWAP_CHAINS) {
        return;
    }
    lancuchy_tab[uchwyt].used = 0;
    lancuchy_tab[uchwyt].width = 0;
    lancuchy_tab[uchwyt].height = 0;
    lancuchy_tab[uchwyt].back_buffer_addr = 0;
    lancuchy_tab[uchwyt].front_buffer_addr = 0;
    lancuchy_tab[uchwyt].vsync = 0;
    lancuchy_tab[uchwyt].alpha_premultiplied = 0;
    lancuchy_tab[uchwyt].back_vram_offset = 0;
    lancuchy_tab[uchwyt].front_vram_offset = 0;
}

void Gtx3070::prezentuj(gtx3070_handle_t lancuch)
{
    if (lancuch >= GTX3070_MAX_SWAP_CHAINS || !lancuchy_tab[lancuch].used) {
        return;
    }
    if (lancuchy_tab[lancuch].vsync) {
        czekaj_na_bezczynnosc(10000);
    }
    zapisz_rejestr(REG_GTX3070_DISP_CRTC_SURFACE, lancuchy_tab[lancuch].front_vram_offset);
}

void Gtx3070::polecenia_rozpocznij(void)
{
    kontekst.in_command_buffer = 1;
    wierzcholek_polecenia = 0;
    wierzcholek_pierwszy = 0;
}

void Gtx3070::polecenia_zakoncz(void)
{
    kontekst.in_command_buffer = 0;
    wierzcholek_polecenia = 0;
    zapisz_rejestr(REG_GTX3070_PFIFO_CACHE_DMA_PUT, wierzcholek_pierwszy);
}

void Gtx3070::kontekst_pobierz(struct gtx3070_context *wyjscie)
{
    if (wyjscie != 0) {
        *wyjscie = kontekst;
    }
}

int Gtx3070::akceleracja_3d_dostepna(void)
{
    return inicjalizowane ? 1 : 0;
}

int gtx3070_init(void)
{
    return gpu_g.inicjalizuj();
}

void gtx3070_shutdown(void)
{
    gpu_g.zamknij();
}

uint8_t gtx3070_czy_gotowy(void)
{
    return gpu_g.czy_gotowy();
}

void gtx3070_putpixel(uint32_t x, uint32_t y, uint32_t kolor)
{
    gpu_g.putpixel(x, y, kolor);
}

void gtx3070_fillrect(uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t kolor)
{
    gpu_g.fillrect(x, y, w, h, kolor);
}

void gtx3070_blt(uint32_t dst_x, uint32_t dst_y, uint32_t src_x, uint32_t src_y, uint32_t w, uint32_t h)
{
    gpu_g.blt(dst_x, dst_y, src_x, src_y, w, h);
}

void gtx3070_cursor_set(uint32_t x, uint32_t y, const uint32_t *obraz)
{
    gpu_g.cursor_set(x, y, obraz);
}

void gtx3070_cursor_hide(void)
{
    gpu_g.cursor_hide();
}

void *gtx3070_get_fb(void)
{
    return gpu_g.get_fb();
}

uint32_t gtx3070_get_szerokosc(void)
{
    return gpu_g.get_szerokosc();
}

uint32_t gtx3070_get_wysokosc(void)
{
    return gpu_g.get_wysokosc();
}

uint16_t gtx3070_get_device_id(void)
{
    return gpu_g.get_device_id();
}

uint8_t gtx3070_get_rewizja(void)
{
    return gpu_g.get_rewizja();
}

void gtx3070_info_pobierz(struct gtx3070_info *wyjscie)
{
    gpu_g.info_pobierz(wyjscie);
}

uint64_t gtx3070_vram_alokuj(uint64_t rozmiar, uint32_t typ)
{
    return gpu_g.vram_alokuj(rozmiar, typ);
}

void gtx3070_vram_zwolnij(uint64_t uchwyt)
{
    gpu_g.vram_zwolnij(uchwyt);
}

uint64_t gtx3070_vram_uzyto(void)
{
    return gpu_g.vram_uzyto();
}

uint64_t gtx3070_vram_calkowicie(void)
{
    return gpu_g.vram_calkowicie();
}

gtx3070_handle_t gtx3070_bufor_wierzcholkow_stworz(uint32_t liczba_wierzcholkow, uint32_t rozmiar_wierzcholka, uint32_t format, const void *dane)
{
    return gpu_g.bufor_wierzcholkow_stworz(liczba_wierzcholkow, rozmiar_wierzcholka, format, dane);
}

void gtx3070_bufor_wierzcholkow_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.bufor_wierzcholkow_zniszcz(uchwyt);
}

void gtx3070_bufor_wierzcholkow_zaktualizuj(gtx3070_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane)
{
    gpu_g.bufor_wierzcholkow_zaktualizuj(uchwyt, przesuniecie, rozmiar, dane);
}

gtx3070_handle_t gtx3070_bufor_indeksow_stworz(uint32_t liczba_indeksow, uint32_t format, const void *dane)
{
    return gpu_g.bufor_indeksow_stworz(liczba_indeksow, format, dane);
}

void gtx3070_bufor_indeksow_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.bufor_indeksow_zniszcz(uchwyt);
}

void gtx3070_bufor_indeksow_zaktualizuj(gtx3070_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane)
{
    gpu_g.bufor_indeksow_zaktualizuj(uchwyt, przesuniecie, rozmiar, dane);
}

gtx3070_handle_t gtx3070_tekstura_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint32_t poziomy_mip, const void *dane)
{
    return gpu_g.tekstura_stworz(szerokosc, wysokosc, format, poziomy_mip, dane);
}

void gtx3070_tekstura_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.tekstura_zniszcz(uchwyt);
}

void gtx3070_tekstura_zaktualizuj(gtx3070_handle_t uchwyt, uint32_t poziom, uint32_t x, uint32_t y, uint32_t w, uint32_t h, const void *dane)
{
    gpu_g.tekstura_zaktualizuj(uchwyt, poziom, x, y, w, h, dane);
}

void gtx3070_tekstura_wygeneruj_mipmapy(gtx3070_handle_t uchwyt)
{
    gpu_g.tekstura_wygeneruj_mipmapy(uchwyt);
}

gtx3070_handle_t gtx3070_cel_renderowania_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint8_t alpha_blend)
{
    return gpu_g.cel_renderowania_stworz(szerokosc, wysokosc, format, alpha_blend);
}

void gtx3070_cel_renderowania_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.cel_renderowania_zniszcz(uchwyt);
}

gtx3070_handle_t gtx3070_bufor_glebii_stworz(uint32_t szerokosc, uint32_t wysokosc, uint32_t format, uint8_t kompresja)
{
    return gpu_g.bufor_glebii_stworz(szerokosc, wysokosc, format, kompresja);
}

void gtx3070_bufor_glebii_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.bufor_glebii_zniszcz(uchwyt);
}

gtx3070_handle_t gtx3070_shader_stworz(uint32_t typ, const uint8_t *kod, uint32_t rozmiar_kodu)
{
    return gpu_g.shader_stworz(typ, kod, rozmiar_kodu);
}

void gtx3070_shader_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.shader_zniszcz(uchwyt);
}

void gtx3070_shader_skompiluj(gtx3070_handle_t uchwyt)
{
    gpu_g.shader_skompiluj(uchwyt);
}

gtx3070_handle_t gtx3070_stan_probnika_stworz(uint32_t filtr_min, uint32_t filtr_mag, uint32_t filtr_mip, uint32_t adres_u, uint32_t adres_v, uint32_t adres_w, uint32_t max_anizotropia, uint8_t wlacz_porownanie, uint8_t func_porownania)
{
    return gpu_g.stan_probnika_stworz(filtr_min, filtr_mag, filtr_mip, adres_u, adres_v, adres_w, max_anizotropia, wlacz_porownanie, func_porownania);
}

void gtx3070_stan_probnika_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.stan_probnika_zniszcz(uchwyt);
}

gtx3070_handle_t gtx3070_stan_mieszania_stworz(uint8_t wlacz_mieszanie, uint32_t src_blend, uint32_t dest_blend, uint32_t op_mieszania, uint32_t src_blend_alpha, uint32_t dest_blend_alpha, uint32_t op_mieszania_alpha, uint32_t maska_zapisu, uint32_t blend_factor)
{
    return gpu_g.stan_mieszania_stworz(wlacz_mieszanie, src_blend, dest_blend, op_mieszania, src_blend_alpha, dest_blend_alpha, op_mieszania_alpha, maska_zapisu, blend_factor);
}

void gtx3070_stan_mieszania_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.stan_mieszania_zniszcz(uchwyt);
}

gtx3070_handle_t gtx3070_stan_rasteryzacji_stworz(uint32_t tryb_wypelnienia, uint32_t tryb_odcinania, uint8_t front_ccw, uint8_t clip_glebii, uint8_t scissor, uint8_t wielopr_probowanie, uint8_t aa_linia, uint8_t alpha_do_pokrycia)
{
    return gpu_g.stan_rasteryzacji_stworz(tryb_wypelnienia, tryb_odcinania, front_ccw, clip_glebii, scissor, wielopr_probowanie, aa_linia, alpha_do_pokrycia);
}

void gtx3070_stan_rasteryzacji_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.stan_rasteryzacji_zniszcz(uchwyt);
}

gtx3070_handle_t gtx3070_stan_glebii_stencil_stworz(uint8_t wlacz_glebie, uint32_t maska_zapisu_glebii, uint32_t func_glebii, uint8_t wlacz_stencil, uint8_t maska_odczytu_stencil, uint8_t maska_zapisu_stencil, uint32_t front_fail, uint32_t front_zfail, uint32_t front_pass, uint32_t front_func, uint32_t back_fail, uint32_t back_zfail, uint32_t back_pass, uint32_t back_func)
{
    return gpu_g.stan_glebii_stencil_stworz(wlacz_glebie, maska_zapisu_glebii, func_glebii, wlacz_stencil, maska_odczytu_stencil, maska_zapisu_stencil, front_fail, front_zfail, front_pass, front_func, back_fail, back_zfail, back_pass, back_func);
}

void gtx3070_stan_glebii_stencil_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.stan_glebii_stencil_zniszcz(uchwyt);
}

gtx3070_handle_t gtx3070_uklad_wejsc_stworz(uint32_t liczba_elementow, const uint32_t *sloty, const uint32_t *przesuniecia, const uint32_t *formaty)
{
    return gpu_g.uklad_wejsc_stworz(liczba_elementow, sloty, przesuniecia, formaty);
}

void gtx3070_uklad_wejsc_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.uklad_wejsc_zniszcz(uchwyt);
}

gtx3070_handle_t gtx3070_bufor_stale_stworz(uint32_t rozmiar)
{
    return gpu_g.bufor_stale_stworz(rozmiar);
}

void gtx3070_bufor_stale_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.bufor_stale_zniszcz(uchwyt);
}

void gtx3070_bufor_stale_zaktualizuj(gtx3070_handle_t uchwyt, uint32_t przesuniecie, uint32_t rozmiar, const void *dane)
{
    gpu_g.bufor_stale_zaktualizuj(uchwyt, przesuniecie, rozmiar, dane);
}

void gtx3070_powiaz_bufor_wierzcholkow(gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_bufor_wierzcholkow(uchwyt);
}

void gtx3070_powiaz_bufor_indeksow(gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_bufor_indeksow(uchwyt);
}

void gtx3070_powiaz_uklad_wejsc(gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_uklad_wejsc(uchwyt);
}

void gtx3070_powiaz_shader_wierzcholkow(gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_shader_wierzcholkow(uchwyt);
}

void gtx3070_powiaz_shader_pikseli(gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_shader_pikseli(uchwyt);
}

void gtx3070_powiaz_shader_geometrii(gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_shader_geometrii(uchwyt);
}

void gtx3070_powiaz_shader_obliczeniowy(gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_shader_obliczeniowy(uchwyt);
}

void gtx3070_powiaz_cel_renderowania(gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_cel_renderowania(uchwyt);
}

void gtx3070_powiaz_bufor_glebii(gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_bufor_glebii(uchwyt);
}

void gtx3070_powiaz_stan_mieszania(gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_stan_mieszania(uchwyt);
}

void gtx3070_powiaz_stan_rasteryzacji(gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_stan_rasteryzacji(uchwyt);
}

void gtx3070_powiaz_stan_glebii_stencil(gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_stan_glebii_stencil(uchwyt);
}

void gtx3070_powiaz_probnik_i_teksture(uint32_t gniazdo, gtx3070_handle_t probnik, gtx3070_handle_t tekstura)
{
    gpu_g.powiaz_probnik_i_teksture(gniazdo, probnik, tekstura);
}

void gtx3070_powiaz_bufor_stale(uint32_t gniazdo, gtx3070_handle_t uchwyt)
{
    gpu_g.powiaz_bufor_stale(gniazdo, uchwyt);
}

void gtx3070_ustaw_viewport(uint32_t x, uint32_t y, uint32_t w, uint32_t h, float min_glebia, float max_glebia)
{
    gpu_g.ustaw_viewport(x, y, w, h, min_glebia, max_glebia);
}

void gtx3070_ustaw_scissor(uint32_t lewa, uint32_t gora, uint32_t prawa, uint32_t dol)
{
    gpu_g.ustaw_scissor(lewa, gora, prawa, dol);
}

void gtx3070_ustaw_typ_pierwotnego(uint32_t typ_pierwotnego)
{
    gpu_g.ustaw_typ_pierwotnego(typ_pierwotnego);
}

void gtx3070_ustaw_mieszanie_alpha_globalne(uint8_t wlacz)
{
    gpu_g.ustaw_mieszanie_alpha_globalne(wlacz);
}

void gtx3070_wyczysc_cel_renderowania(gtx3070_handle_t cel, uint32_t kolor)
{
    gpu_g.wyczysc_cel_renderowania(cel, kolor);
}

void gtx3070_wyczysc_bufor_glebii(gtx3070_handle_t bufor, float glebia, uint8_t stencil)
{
    gpu_g.wyczysc_bufor_glebii(bufor, glebia, stencil);
}

void gtx3070_rysuj(uint32_t liczba_wierzcholkow, uint32_t pierwszy_wierzcholek)
{
    gpu_g.rysuj(liczba_wierzcholkow, pierwszy_wierzcholek);
}

void gtx3070_rysuj_indeksowane(uint32_t liczba_indeksow, uint32_t pierwszy_indeks, int32_t baza_wierzcholka)
{
    gpu_g.rysuj_indeksowane(liczba_indeksow, pierwszy_indeks, baza_wierzcholka);
}

void gtx3070_rysuj_indeksowane_instancjonowane(uint32_t liczba_indeksow, uint32_t pierwszy_indeks, int32_t baza_wierzcholka, uint32_t liczba_instancji, uint32_t pierwsza_instancja)
{
    gpu_g.rysuj_indeksowane_instancjonowane(liczba_indeksow, pierwszy_indeks, baza_wierzcholka, liczba_instancji, pierwsza_instancja);
}

void gtx3070_rysuj_po_indirekcie(uint32_t adres_bufora, uint32_t liczba_rysowan, uint32_t krok)
{
    gpu_g.rysuj_po_indirekcie(adres_bufora, liczba_rysowan, krok);
}

void gtx3070_oblicz_wyslij(uint32_t grupa_x, uint32_t grupa_y, uint32_t grupa_z)
{
    gpu_g.oblicz_wyslij(grupa_x, grupa_y, grupa_z);
}

void gtx3070_oblicz_wyslij_po_indirekcie(uint32_t adres_bufora)
{
    gpu_g.oblicz_wyslij_po_indirekcie(adres_bufora);
}

void gtx3070_ogrodzenie_stworz(gtx3070_handle_t *wyjscie)
{
    gpu_g.ogrodzenie_stworz(wyjscie);
}

void gtx3070_ogrodzenie_sygnalizuj(gtx3070_handle_t uchwyt, uint32_t wartosc)
{
    gpu_g.ogrodzenie_sygnalizuj(uchwyt, wartosc);
}

int gtx3070_ogrodzenie_poczekaj(gtx3070_handle_t uchwyt, uint32_t wartosc, uint32_t timeout_us)
{
    return gpu_g.ogrodzenie_poczekaj(uchwyt, wartosc, timeout_us);
}

void gtx3070_ogrodzenie_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.ogrodzenie_zniszcz(uchwyt);
}

void gtx3070_semafor_stworz(gtx3070_handle_t *wyjscie)
{
    gpu_g.semafor_stworz(wyjscie);
}

void gtx3070_semafor_sygnalizuj(gtx3070_handle_t uchwyt, uint32_t wartosc)
{
    gpu_g.semafor_sygnalizuj(uchwyt, wartosc);
}

void gtx3070_semafor_poczekaj(gtx3070_handle_t uchwyt, uint32_t wartosc)
{
    gpu_g.semafor_poczekaj(uchwyt, wartosc);
}

void gtx3070_semafor_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.semafor_zniszcz(uchwyt);
}

int gtx3070_czekaj_na_bezczynnosc(uint32_t timeout_us)
{
    return gpu_g.czekaj_na_bezczynnosc(timeout_us);
}

uint64_t gtx3070_znacznik_czasu(void)
{
    return gpu_g.znacznik_czasu();
}

void gtx3070_ustaw_stan_zasilania(uint8_t stan)
{
    gpu_g.ustaw_stan_zasilania(stan);
}

uint8_t gtx3070_pobierz_stan_zasilania(void)
{
    return gpu_g.pobierz_stan_zasilania();
}

void gtx3070_ustaw_zegar_graficzny(uint32_t mhz)
{
    gpu_g.ustaw_zegar_graficzny(mhz);
}

void gtx3070_ustaw_zegar_pamieci(uint32_t mhz)
{
    gpu_g.ustaw_zegar_pamieci(mhz);
}

uint32_t gtx3070_pobierz_zegar_graficzny(void)
{
    return gpu_g.pobierz_zegar_graficzny();
}

uint32_t gtx3070_pobierz_zegar_pamieci(void)
{
    return gpu_g.pobierz_zegar_pamieci();
}

uint32_t gtx3070_pobierz_temperature(void)
{
    return gpu_g.pobierz_temperature();
}

void gtx3070_ustaw_predkosc_wiatraka(uint32_t obroty)
{
    gpu_g.ustaw_predkosc_wiatraka(obroty);
}

uint32_t gtx3070_pobierz_predkosc_wiatraka(void)
{
    return gpu_g.pobierz_predkosc_wiatraka();
}

void gtx3070_ustaw_rgb(uint32_t kolor, uint8_t tryb)
{
    gpu_g.ustaw_rgb(kolor, tryb);
}

void gtx3070_wylacz_rgb(void)
{
    gpu_g.wylacz_rgb();
}

gtx3070_handle_t gtx3070_lancuch_stworz(uint32_t szerokosc, uint32_t wysokosc, uint8_t vsync, uint8_t alpha_premultiplied)
{
    return gpu_g.lancuch_stworz(szerokosc, wysokosc, vsync, alpha_premultiplied);
}

void gtx3070_lancuch_zniszcz(gtx3070_handle_t uchwyt)
{
    gpu_g.lancuch_zniszcz(uchwyt);
}

void gtx3070_prezentuj(gtx3070_handle_t lancuch)
{
    gpu_g.prezentuj(lancuch);
}

void gtx3070_polecenia_rozpocznij(void)
{
    gpu_g.polecenia_rozpocznij();
}

void gtx3070_polecenia_zakoncz(void)
{
    gpu_g.polecenia_zakoncz();
}

void gtx3070_kontekst_pobierz(struct gtx3070_context *wyjscie)
{
    gpu_g.kontekst_pobierz(wyjscie);
}

int gtx3070_akceleracja_3d_dostepna(void)
{
    return gpu_g.akceleracja_3d_dostepna();
}

