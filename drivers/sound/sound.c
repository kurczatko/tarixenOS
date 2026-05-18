#include "sound.h"
#include "../../include/printf/types.h"
#include "../../include/printf/printf.h"

//adresy portow pci
#define PCI_CONFIG_ADDRESS 0xCF8
#define PCI_CONFIG_DATA    0xCFC

//rejestry pci
#define PCI_VENDOR_ID      0x00
#define PCI_COMMAND        0x04
#define PCI_CLASS_REVISION 0x08
#define PCI_BAR0           0x10 //rejestr adresu bazowego 0 (mmio)

//rejestry kontrolera hda
#define HDA_GCTL           0x08 //globalne sterowanie
#define HDA_STATESTS       0x0E //status kodekow
#define HDA_IC             0x60 //wyjscie natychmiastowej komendy
#define HDA_IR             0x64 //wejscie natychmiastowej odpowiedzi
#define HDA_IRS            0x68 //status natychmiastowej odpowiedzi

//rejestry strumienia wyjsciowego 0 (offsety)
#define HDA_OS0_CTL        0x100 //sterowanie
#define HDA_OS0_CBL        0x108 //dlugosc bufora
#define HDA_OS0_LVI        0x10C //ostatni indeks bdl
#define HDA_OS0_FMT        0x112 //format (np. 44.1khz)
#define HDA_OS0_BDLPL      0x118 //adres listy bdl (nizszy)
#define HDA_OS0_BDLPU      0x11C //adres listy bdl (wyzszy)

//struktura deskryptora dma (bdl)
struct hda_bdl_entry {
    uint32_t addr_low;
    uint32_t addr_high;
    uint32_t length;
    uint32_t ioc; //interrupt on completion
} __attribute__((packed));

static uint32_t hda_base = 0;
static struct hda_bdl_entry bdl[2] __attribute__((aligned(128)));
static uint8_t audio_buffer[16384] __attribute__((aligned(4096)));

//odczyt pci
static uint32_t pci_read(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (uint32_t)((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                       ((uint32_t)func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000);
    asm volatile("outl %0, %1" : : "a"(address), "Nd"((uint16_t)PCI_CONFIG_ADDRESS));
    uint32_t res;
    asm volatile("inl %1, %0" : "=a"(res) : "Nd"((uint16_t)PCI_CONFIG_DATA));
    return res;
}

//zapis pci
static void pci_write(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t val) {
    uint32_t address = (uint32_t)((uint32_t)bus << 16) | ((uint32_t)slot << 11) |
                       ((uint32_t)func << 8) | (offset & 0xFC) | ((uint32_t)0x80000000);
    asm volatile("outl %0, %1" : : "a"(address), "Nd"((uint16_t)PCI_CONFIG_ADDRESS));
    asm volatile("outl %0, %1" : : "a"(val), "Nd"((uint16_t)PCI_CONFIG_DATA));
}

//wysylanie komendy do kodeki (verb)
static uint32_t hda_verb(uint8_t codec, uint8_t node, uint32_t verb) {
    uint32_t cmd = ((uint32_t)codec << 28) | ((uint32_t)node << 20) | verb;
    //czekaj az szyna bedzie wolna
    while(*(volatile uint16_t*)(hda_base + HDA_IRS) & 0x01);
    //wyczysc status
    *(volatile uint16_t*)(hda_base + HDA_IRS) |= 0x02;
    //wyslij
    *(volatile uint32_t*)(hda_base + HDA_IC) = cmd;
    //czekaj na odpowiedz
    while(!(*(volatile uint16_t*)(hda_base + HDA_IRS) & 0x01));
    return *(volatile uint32_t*)(hda_base + HDA_IR);
}

//inicjalizacja sterownika
void hda_init(void) {
    for(uint16_t b = 0; b < 256; b++) {
        for(uint8_t s = 0; s < 32; s++) {
            for(uint8_t f = 0; f < 8; f++) {
                if(pci_read(b, s, f, PCI_VENDOR_ID) == 0xFFFFFFFF) continue;
                uint32_t class = pci_read(b, s, f, PCI_CLASS_REVISION);
                if(((class >> 16) & 0xFFFF) == 0x0403) {
                    hda_base = pci_read(b, s, f, PCI_BAR0) & 0xFFFFFFF0;
                    if(!hda_base) return;
                    
                    uint32_t cmd = pci_read(b, s, f, PCI_COMMAND);
                    pci_write(b, s, f, PCI_COMMAND, cmd | 0x06); //mmio + bus master

                    //resetowanie
                    *(volatile uint32_t*)(hda_base + HDA_GCTL) &= ~0x01;
                    for(volatile int i = 0; i < 10000; i++);
                    *(volatile uint32_t*)(hda_base + HDA_GCTL) |= 0x01;
                    while(!(*(volatile uint32_t*)(hda_base + HDA_GCTL) & 0x01));
                    for(volatile int i = 0; i < 10000; i++);

                    //odblokowanie sciezki w kodeku (typowo node 2 i 3 w qemu)
                    hda_verb(0, 0x03, 0x707 | 0x40); //ustaw pin jako output
                    hda_verb(0, 0x02, 0x300 | 0x80 | 0x7F); //unmute dac i max glosnosc
                    hda_verb(0, 0x02, 0x706 | 0x10); //przypisz strumien 1 do dac
                    return;
                }
            }
        }
    }
    printf("nh1\n"); //brak sprzetu
}

//uruchomienie dma i odtwarzanie
void hda_play_sound(void) {
    if(!hda_base) { printf("nh0\n"); return; }

    //wypelnienie bufora cisza lub prostym szumem
    for(int i = 0; i < 16384; i++) audio_buffer[i] = (i % 255);

    //konfiguracja bdl (lista deskryptorow)
    bdl[0].addr_low = (uint32_t)(uintptr_t)audio_buffer;
    bdl[0].addr_high = 0;
    bdl[0].length = 16384;
    bdl[0].ioc = 0;

    //zatrzymaj strumien przed konfiguracja
    *(volatile uint32_t*)(hda_base + HDA_OS0_CTL) &= ~0x02;
    
    //ustaw adres bdl
    *(volatile uint32_t*)(hda_base + HDA_OS0_BDLPL) = (uint32_t)(uintptr_t)bdl;
    *(volatile uint32_t*)(hda_base + HDA_OS0_BDLPU) = 0;

    //ustaw parametry dma
    *(volatile uint32_t*)(hda_base + HDA_OS0_CBL) = 16384; //rozmiar bufora
    *(volatile uint16_t*)(hda_base + HDA_OS0_LVI) = 0; //tylko jeden wpis bdl
    *(volatile uint16_t*)(hda_base + HDA_OS0_FMT) = 0x0011; //44.1khz, 16bit, stereo
    
    //ustaw id strumienia na 1 (bit 20-23)
    uint32_t ctl = *(volatile uint32_t*)(hda_base + HDA_OS0_CTL);
    ctl &= 0xFF0FFFFF;
    ctl |= (1 << 20);
    *(volatile uint32_t*)(hda_base + HDA_OS0_CTL) = ctl | 0x02; //uruchom dma

    printf("hh0rr5\n"); //odtwarzanie
}
