/*
* ten plik to najwazniejszy plik systemu 
* prosze go nie ruszać bez potrzeby
* polowa includów nic nie robi bo zostaly dodane rok temu 
* ale najwazniejsze jest ze sie kompiluje
*/

#include "../../drivers/keyboard.h"
#include "../drivers/plyty_glowne/asus_p4p800.h"
#include "../drivers/plyty_glowne/intel_915.h"
#include "../drivers/mouse.h"
#include "../drivers/usb/core/usb.h"
#include "../drivers/usb/core/hid.h"
#include "../drivers/usb/mouse/mouse.h"
#include "../drivers/usb/keyboard/keyboard.h"
#include "../system_plikow/system_plikow.h"
#include "../graficzny/rysowanie.h"
#include "../graficzny/menu_start.h"
#include "../graficzny/okno.h"
#include "../graficzny/pulpit.h"

#include "../drivers/sound/intel_hd.h"
#include "../drivers/alc650.h"
#include "../drivers/net.h"
#include "../drivers/floppy.h"
#include "../drivers/plyty.h"
#include "../drivers/nvme.h"
#include "../drivers/intel_uhd.h"
#include "../drivers/karty_graficzne/gma_x4500.h"
#include "../drivers/karty_graficzne/radeon_hd_8370d.h"
#include "../drivers/karty_graficzne/radeon_rx_9070_xt.h"
#include "../drivers/karty_graficzne/radeon_hd_6870.h"
#include "../drivers/bluetooth/bluetooth.h"
#include "zegar.h"
#include "notatnik.h"
#include "kalkulator.h"
#include "malowanie.h"
#include "browser.h"
#include <../user_land/konta.h>
#include "gdt.h"
#include "idt.h"
#include "syscall.h"
#include "../user_land/tapeta.h"
#include "../drivers/karty_graficzne/radeon_rx_9070_xt.h"
#include "../graficzny/podzespoly.h"
#include "../graficzny/eksplorator_plikow.h"
#include "panika.h"

bool amd_r7_265_driver_init(void);
bool amd_r7_265_driver_detected(void);
bool gtx_1080_ti_driver_init(void);
bool gtx_1080_ti_driver_detected(void);
bool intel_82578dc_driver_init(void);
bool intel_82578dc_driver_detected(void);
bool __attribute__((weak)) amd_hd_4730_driver_init(void) { return false; }
bool __attribute__((weak)) amd_hd_4730_driver_detected(void) { return false; }
bool nvidia_gtx_1060_driver_init(void);
bool nvidia_gtx_1060_driver_detected(void);
bool __attribute__((weak)) nvidia_gtx_1060_driver_init(void) { return false; }
bool __attribute__((weak)) nvidia_gtx_1060_driver_detected(void) { return false; }
bool gtx_1000_driver_init(void);
bool gtx_1000_driver_detected(void);
bool rx_460_driver_init(void);
bool rx_460_driver_detected(void);
bool touch_driver_init(void);
bool touch_driver_detected(void);
bool quickcam_driver_init(void);
uint8_t quickcam_driver_detected(void);
void quickcam_driver_shutdown(void);
uint8_t quickcam_driver_poll(void);
#include "proces.h"
#include "driver_manager.h"
#include "timer.h"
#include "pik.h"
#include "../drivers/qemu_vga.h"
#include "../drivers/internet/rtl8111f.h"
#include "../drivers/internet/rtl8111e.h"
#include "../drivers/internet/rtl8168b.h"
#include "../drivers/internet/rtl8188ce.h"

extern struct okno *aktywne_okno;
extern uint32_t __bss_start;
extern uint32_t __bss_end;

extern uint8_t notatnik_otwarty;
extern struct okno okno_notatnik;
extern uint8_t kalkulator_otwarty;
extern struct okno okno_kalkulator;
extern uint8_t malowanie_otwarty;
extern struct okno okno_malowanie;
extern uint8_t tryb_tworz;
extern struct okno pliki;
extern uint8_t browser_opened;
extern struct okno browser_window;

void klawiatura_obsługa_aktywnego_okna(void)
{
    char znak;
    if (!keyboard_try_getchar(&znak)) return;

    uint8_t ctrl = keyboard_ctrl_pressed();
    uint8_t alt = keyboard_alt_pressed();

    if (notatnik_otwarty && okno_notatnik.otwarte && okno_czy_aktywne(&okno_notatnik) && !okno_notatnik.w_polu_tekstowym) {
        notatnik_obsluz_klawiature_znak(znak);
    } else if (kalkulator_otwarty && okno_kalkulator.otwarte && okno_czy_aktywne(&okno_kalkulator) && okno_kalkulator.w_polu_tekstowym) {
        kalkulator_obsluz_klawiature_znak(znak);
    } else if (malowanie_otwarty && okno_malowanie.otwarte && okno_czy_aktywne(&okno_malowanie)) {
        malowanie_obsluz_klawiature_znak(znak);
    } else if (tryb_tworz && okno_czy_aktywne(&pliki)) {
        eksplorator_plikow_obsluz_klawiature_znak(znak);
    } else if (browser_opened && browser_window.otwarte && okno_czy_aktywne(&browser_window)) {
        browser_handle_keyboard_znak(znak);
    } else if (aktywne_okno && aktywne_okno->otwarte && aktywne_okno->pole_tekstowe_znak) {
        aktywne_okno->pole_tekstowe_znak(aktywne_okno, znak, ctrl);
    }

    if (ctrl && alt && (znak == 'i' || znak == 'I')) {
        podzespoly_otworz();
    }
}

void proces_uruchom_pierwszy(struct proces *p) {
    struct tss *tss = gdt_tss();
    tss->esp0 = p->kernel_stack;
    tss->cr3 = (uint32_t)p->page_directory;

    asm volatile("mov %0, %%cr3" :: "r"(p->page_directory));

    uint32_t eip = p->regs.eip;
    uint32_t esp = p->regs.esp;
    userspace_uruchom(eip, esp);
}

void __attribute__((section(".text.entry"))) kernel_main(void)
{
    for (uint32_t *bss = &__bss_start; bss < &__bss_end; bss++) {
        *bss = 0;
    }

    /*
    * i tak nie zrozumiesz tych hackow w assemblerze
    */

    __asm__ volatile ("fninit" ::: "memory");
    uint32_t cr0_val;
    __asm__ volatile ("mov %%cr0, %0" : "=r"(cr0_val));
    cr0_val |= (1 << 1);                 /* CR0.MP = 1 */
    __asm__ volatile ("mov %0, %%cr0" :: "r"(cr0_val));
    uint32_t cr4_val;
    __asm__ volatile ("mov %%cr4, %0" : "=r"(cr4_val));
    cr4_val |= (1 << 9)  | (1 << 10);   /* CR4.OSFXSR | CR4.OSXMMEXCPT */
    __asm__ volatile ("mov %0, %%cr4" :: "r"(cr4_val));

    __asm__ volatile ("movb $'A', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    rysowanie_init();
    __asm__ volatile ("movb $'B', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    zegar_init();
    __asm__ volatile ("movb $'C', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    {
        char tekst_czasu[9];
        zegar_pobierz(tekst_czasu);
        ustaw_czas_paska(tekst_czasu);
    }
    __asm__ volatile ("movb $'D', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    /*
    * architektura jest tak zaprojektowana aby zaprzeczac wszelkim prawom logiki
    * dlatego kazdy sterownik jest wywolywany w petli kernela
    * wiec prosze sie nie spłakiwać ze zżera 400 terabajtów ramu
    */
    init_fs();
    driver_manager_register("floppy", floppy_init, 0, 0, 4);
    driver_manager_register("plyty", plyty_init, 0, 0, 4);
    driver_manager_register("nvme", nvme_init, 0, 0, 4);
    driver_manager_register("intel_uhd", intel_uhd_init, 0, 0, 4);
    driver_manager_register("p4p_motherboard", p4p_init, p4p_shutdown, 0, 4);
    driver_manager_register("i915", i915_init, 0, 0, 4);
    driver_manager_register("gma_x4500", gma_x4500_init, 0, 0, 4);
    driver_manager_register("radeon_hd_8370d", radeon_hd_8370d_init, 0, 0, 4);
    driver_manager_register("rx_9070_xt", rx_9070_xt_init, rx_9070_xt_shutdown, 0, 4);
    driver_manager_register("radeon_6870", radeon_6870_init, radeon_6870_shutdown, 0, 4);
    driver_manager_register("amd_r7_265", amd_r7_265_driver_init, 0, 0, 4);
    driver_manager_register("hd_4730", amd_hd_4730_driver_init, 0, 0, 4);
    driver_manager_register("gtx_1060", nvidia_gtx_1060_driver_init, 0, 0, 4);
    driver_manager_register("gtx_1080_ti", gtx_1080_ti_driver_init, 0, 0, 4);
    driver_manager_register("gtx_1000", gtx_1000_driver_init, 0, 0, 4);
    driver_manager_register("rx_460", rx_460_driver_init, 0, 0, 4);
    driver_manager_register("ekran_dotykowy", touch_driver_init, 0, 0, 4);
    driver_manager_register("kamera_quickcam", quickcam_driver_init, quickcam_driver_shutdown, quickcam_driver_poll, 5);
    driver_manager_register("intel_82578dc_wifi", intel_82578dc_driver_init, 0, 0, 4);
    driver_manager_register("qemu_vga", qemu_vga_init, 0, 0, 4);
    driver_manager_register("rtl8111f_net", rtl8111f_init, rtl8111f_shutdown, 0, 2);
    driver_manager_register("rtl8111e_net", rtl8111e_init, 0, 0, 2);
    driver_manager_register("rtl8168b_net", rtl8168b_init, 0, 0, 2);
    driver_manager_register("rtl8188ce_net", rtl8188ce_init, 0, 0, 2);
    driver_manager_register("bluetooth", bluetooth_init, 0, bluetooth_poll, 2);
    driver_manager_register("mouse", mouse_init, 0, mouse_poll, 2);
    driver_manager_register("keyboard", keyboard_init, 0, 0, 4);
    driver_manager_register("usb", usb_init, 0, usb_poll, 2);
    driver_manager_register("usb_mouse", usb_mouse_init, 0, 0, 4);
    driver_manager_register("usb_keyboard", usb_keyboard_init, 0, 0, 4);
    driver_manager_register("intel_hda", init_intel_hda, 0, 0, 4);
    driver_manager_register("alc650", alc650_init, alc650_shutdown, 0, 4);
    tapeta_wczytaj();
    gdt_install();
    idt_install();
    syscall_install();
    bluetooth_ui_init();
    kamera_ui_init();
    pulp_init();
    tapeta_app();
    __asm__ volatile ("movb $'G', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    menu_start();

    __asm__ volatile ("movb $'H', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    __asm__ volatile ("movb $'I', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    __asm__ volatile ("movb $'J', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    __asm__ volatile ("movb $'K', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    pulp_init();
    __asm__ volatile ("movb $'L', %%al; movw $0x3F8, %%dx; outb %%al, %%dx" ::: "eax", "edx");
    konta_start();

    pik_init();
    timer_init(100);
    pik_enable_irq(0);

    proces_init();

    driver_manager_init_all();

    if (!net_ready()) {
        net_init();
    }
    ustaw_status_internetu(net_ready());

    while(1) {
        char tekst_czasu[9];
        if (zegar_pobierz(tekst_czasu)) ustaw_czas_paska(tekst_czasu);
        driver_manager_poll_all();
        mouse_poll();
        usb_poll();
        bluetooth_poll();
        hid_poll();
        kursor_tekstowy_tick();
        if (!konta_czy_zalogowano()) {
            konta_obsluz();
            keyboard_discard_pending();
            continue;
        }
        if (keyboard_windows_pressed()) {
            menu_start_przelacz();
        }
        if (keyboard_dane_dostepne()) {
            klawiatura_obsługa_aktywnego_okna();
        }
        if (!aktywne_okno || !aktywne_okno->otwarte) {
            if (pulp_czy_trzeba_odswiezyc()) pulp_odswiez();
        }
    }
}
