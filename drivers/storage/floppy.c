/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*
* Sterownik stacji dyskietek 3.5" 1.44 MB (kontroler FDC i8272 / 82077),
*/

#include <drivers/storage/floppy.h>

#define FLOPPY_DOR  0x3F2
#define FLOPPY_MSR  0x3F4
#define FLOPPY_DATA 0x3F5
#define FLOPPY_CCR  0x3F7   /* zapis: predkosc transferu */

#define CMD_SPECIFY     0x03
#define CMD_RECALIBRATE 0x07
#define CMD_SENSE_INT   0x08
#define CMD_SEEK        0x0F
#define CMD_VERSION     0x10
#define CMD_READ        0x66   /* READ DATA:  MFM | SKIP | 0x06, bez MT */
#define CMD_WRITE       0x45   /* WRITE DATA: MFM | 0x05, bez MT */

#define MSR_RQM   0x80
#define MSR_DIO   0x40   /* 1 = FDC -> CPU */
#define MSR_NDMA  0x20   /* faza wykonania w trybie bez DMA */
#define MSR_BUSY  0x10
#define MSR_DRV0  0x01   /* naped 0 w trakcie seek/recalibrate */

#define DOR_RUN      0x0C        /* bit 2 = !RESET, bit 3 = IRQ/DMA enable, naped 0 */
#define DOR_MOTOR0   0x10

#define FLOPPY_SECTORS_TOTAL 2880U
#define FLOPPY_SPT           18U
#define FLOPPY_HEADS         2U

static int g_floppy_present = 0;   /* dyskietka z FAT (512 B/sektor) odczytana */
static int g_ctrl_ok = 0;          /* kontroler i naped wykryte */
static int g_need_reset = 0;
static int g_motor_on = 0;
static uint8_t g_cyl = 0xFF;       /* biezacy cylinder glowicy (0xFF = nieznany) */
static uint32_t g_tsc_per_us = 0;
static uint64_t g_motor_stamp = 0;

/* ------------------------------------------------------------------ */
/* Porty I/O, TSC, opoznienia                                         */
/* ------------------------------------------------------------------ */

static void outb(uint16_t port, uint8_t v) { __asm__ volatile ("outb %0, %1" : : "a"(v), "Nd"(port)); }
static uint8_t inb(uint16_t port) { uint8_t v; __asm__ volatile ("inb %1, %0" : "=a"(v) : "Nd"(port)); return v; }

static uint64_t fl_rdtsc(void)
{
    uint32_t lo, hi;
    __asm__ volatile("rdtsc" : "=a"(lo), "=d"(hi));
    return ((uint64_t)hi << 32) | lo;
}

/* Kalibracja TSC wzgledem PIT kanal 2 (10 ms). */
static void tsc_calibrate(void)
{
    uint8_t saved;
    uint64_t t0, t1;
    uint32_t n, delta, per_us;

    if (g_tsc_per_us) return;

    saved = inb(0x61);
    outb(0x61, (uint8_t)((saved & 0xFC) | 0x01));
    outb(0x43, 0xB0);
    outb(0x42, (uint8_t)(11932 & 0xFF));
    outb(0x42, (uint8_t)(11932 >> 8));
    t0 = fl_rdtsc();
    for (n = 0; n < 20000000U; n++) {
        if (inb(0x61) & 0x20) break;
    }
    t1 = fl_rdtsc();
    outb(0x61, saved);

    if (n == 20000000U) return;
    delta = (uint32_t)(t1 - t0);
    per_us = delta / 10000U;
    if (per_us < 100U || per_us > 10000U) return;
    g_tsc_per_us = per_us;
}

static void delay_us(uint32_t us)
{
    if (g_tsc_per_us) {
        uint64_t end = fl_rdtsc() + (uint64_t)us * g_tsc_per_us;
        while (fl_rdtsc() < end) __asm__ volatile("pause");
        return;
    }
    while (us--) outb(0x80, 0);   /* zapas: ok. 1 us na zapis */
}

static void delay_ms(uint32_t ms)
{
    while (ms--) delay_us(1000);
}

static unsigned long irq_save(void)
{
    unsigned long fl;
    __asm__ volatile("pushf\n\tpop %0\n\tcli" : "=r"(fl) : : "memory");
    return fl;
}

static void irq_restore(unsigned long fl)
{
    if (fl & 0x200UL) __asm__ volatile("sti" ::: "memory");
}

/* ------------------------------------------------------------------ */
/* Niskopoziomowa komunikacja z FDC                                   */
/* ------------------------------------------------------------------ */

/* Czekaj na RQM i zgodny kierunek (DIO). */
static int msr_wait(int dir_to_cpu, uint32_t timeout)
{
    while (timeout-- > 0) {
        uint8_t msr = inb(FLOPPY_MSR);
        if ((msr & MSR_RQM) && (((msr & MSR_DIO) != 0) == (dir_to_cpu != 0))) return 1;
    }
    return 0;
}

static int cmd_byte(uint8_t b)
{
    if (!msr_wait(0, 200000)) return 0;
    outb(FLOPPY_DATA, b);
    return 1;
}

static int result_byte(uint8_t *b)
{
    if (!msr_wait(1, 200000)) return 0;
    *b = inb(FLOPPY_DATA);
    return 1;
}

/*
 * Czekanie na bajt w fazie wykonania. check_exec = 1: jesli FDC wyszedl z fazy
 * wykonania (NDMA=0, czyli przeszedl do wyniku), zwroc -1. 1 = gotowy, 0 = timeout.
 */
static int data_wait(int to_cpu, uint32_t limit, int check_exec)
{
    uint32_t t;
    for (t = 0; t < limit; t++) {
        uint8_t msr = inb(FLOPPY_MSR);
        if (msr & MSR_RQM) {
            if (check_exec && !(msr & MSR_NDMA)) return -1;
            if (((msr & MSR_DIO) != 0) == (to_cpu != 0)) return 1;
        }
    }
    return 0;
}

/* SENSE INTERRUPT: ST0 i numer cylindra (po resecie bez przerwania FDC zwraca tylko 0x80). */
static int sense_int(uint8_t *st0, uint8_t *pcn)
{
    uint8_t a, b = 0;
    if (!cmd_byte(CMD_SENSE_INT)) return 0;
    if (!result_byte(&a)) return 0;
    if (a != 0x80) {
        if (!result_byte(&b)) return 0;
    }
    *st0 = a;
    *pcn = b;
    return 1;
}

/* Czekaj, az naped 0 skonczy seek/recalibrate (bit DRV0 BUSY w MSR). */
static void wait_seek_done(uint32_t timeout_ms)
{
    uint32_t ms;
    delay_us(200);
    for (ms = 0; ms < timeout_ms; ms++) {
        if (!(inb(FLOPPY_MSR) & MSR_DRV0)) return;
        delay_ms(1);
    }
}

/* ------------------------------------------------------------------ */
/* Silnik                                                             */
/* ------------------------------------------------------------------ */

static void motor_off(void)
{
    outb(FLOPPY_DOR, DOR_RUN);
    g_motor_on = 0;
}

static void motor_on(void)
{
    if (!g_motor_on) {
        outb(FLOPPY_DOR, (uint8_t)(DOR_RUN | DOR_MOTOR0));
        g_motor_on = 1;
        delay_ms(500);                 /* rozped napedu */
    } else {
        outb(FLOPPY_DOR, (uint8_t)(DOR_RUN | DOR_MOTOR0));
    }
}

/*
 * Silnik zostaje wlaczony miedzy operacjami (kazdy rozped to ok. 0,5 s).
 * Wywoluj to okresowo (np. z timera) - po 3 s bez uzycia silnik jest wylaczany.
 */
void floppy_motor_tick(void)
{
    if (g_motor_on && g_tsc_per_us) {
        if (fl_rdtsc() - g_motor_stamp > (uint64_t)3000000U * g_tsc_per_us) motor_off();
    }
}

/* ------------------------------------------------------------------ */
/* Reset, SPECIFY, recalibrate, seek                                  */
/* ------------------------------------------------------------------ */

static int floppy_specify(void)
{
    if (!cmd_byte(CMD_SPECIFY)) return 0;
    if (!cmd_byte(0xCF)) return 0;     /* SRT = 4 ms, HUT = maks. */
    if (!cmd_byte(0x21)) return 0;     /* HLT = 16, ND = 1 (tryb bez DMA) */
    return 1;
}

static int floppy_reset(void)
{
    uint8_t st0, pcn, ver;
    int i;

    g_motor_on = 0;
    g_cyl = 0xFF;

    outb(FLOPPY_DOR, 0x00);            /* reset */
    delay_ms(2);
    outb(FLOPPY_DOR, DOR_RUN);         /* koniec resetu */
    delay_ms(10);

    /* Po resecie FDC zglasza 4 przerwania (po jednym na naped) - opróżnij je. */
    if (!sense_int(&st0, &pcn)) return 0;
    for (i = 0; i < 3; i++) (void)sense_int(&st0, &pcn);

    outb(FLOPPY_CCR, 0x00);            /* 500 kbit/s (1.44 MB) */

    /* Kontroler obecny? (0xFF = pusta szyna) */
    if (!cmd_byte(CMD_VERSION)) return 0;
    if (!result_byte(&ver)) return 0;
    if (ver == 0xFF) return 0;

    if (!floppy_specify()) return 0;
    g_need_reset = 0;
    return 1;
}

static int floppy_recalibrate(void)
{
    uint8_t st0, pcn;
    int tries;

    for (tries = 0; tries < 3; tries++) {
        if (!cmd_byte(CMD_RECALIBRATE)) return 0;
        if (!cmd_byte(0x00)) return 0;
        wait_seek_done(3000);
        if (!sense_int(&st0, &pcn)) return 0;
        if ((st0 & 0xF0) == 0x20 && pcn == 0) {   /* seek end, bez bledu */
            g_cyl = 0;
            return 1;
        }
    }
    return 0;
}

static int floppy_seek(uint8_t cyl, uint8_t head)
{
    uint8_t st0, pcn;

    if (g_cyl == cyl) return 1;
    if (!cmd_byte(CMD_SEEK)) return 0;
    if (!cmd_byte((uint8_t)(head << 2))) return 0;
    if (!cmd_byte(cyl)) return 0;
    wait_seek_done(1000);
    if (!sense_int(&st0, &pcn)) return 0;
    if ((st0 & 0xF0) != 0x20 || pcn != cyl) {
        g_cyl = 0xFF;
        return 0;
    }
    delay_ms(15);                      /* ustalanie sie glowicy */
    g_cyl = cyl;
    return 1;
}

/* ------------------------------------------------------------------ */
/* Odczyt / zapis sektora                                             */
/* ------------------------------------------------------------------ */

static void lba_to_chs(uint32_t lba, uint8_t *c, uint8_t *h, uint8_t *s)
{
    *s = (uint8_t)((lba % FLOPPY_SPT) + 1);
    *h = (uint8_t)((lba / FLOPPY_SPT) % FLOPPY_HEADS);
    *c = (uint8_t)(lba / (FLOPPY_SPT * FLOPPY_HEADS));
}

static int floppy_rw_once(int write, uint32_t lba, uint8_t *rbuf, const uint8_t *wbuf)
{
    uint8_t c, h, s, st[7];
    uint32_t i;
    int ok = 1;
    unsigned long fl;

    lba_to_chs(lba, &c, &h, &s);

    floppy_motor_tick();
    motor_on();
    if (!floppy_seek(c, h)) return 0;

    /* EOT = numer sektora: bez sygnalu TC to jedyny sposob, by FDC skonczyl po 1 sektorze. */
    if (!cmd_byte(write ? CMD_WRITE : CMD_READ)) return 0;
    if (!cmd_byte((uint8_t)(h << 2))) return 0;
    if (!cmd_byte(c)) return 0;
    if (!cmd_byte(h)) return 0;
    if (!cmd_byte(s)) return 0;
    if (!cmd_byte(2)) return 0;        /* N = 2 -> 512 B */
    if (!cmd_byte(s)) return 0;        /* EOT */
    if (!cmd_byte(0x1B)) return 0;     /* GAP3 */
    if (!cmd_byte(0xFF)) return 0;     /* DTL */

    fl = irq_save();                   /* bez przerwan - inaczej overrun */
    for (i = 0; i < 512; i++) {
        /* Pierwszy bajt: czekamy na obrot dysku, dalsze: bajt co kilkanascie us. */
        int r = data_wait(write ? 0 : 1, i == 0 ? 3000000U : 200000U, i != 0);
        if (r != 1) { ok = 0; break; }
        if (write) outb(FLOPPY_DATA, wbuf[i]);
        else       rbuf[i] = inb(FLOPPY_DATA);
    }
    irq_restore(fl);

    for (i = 0; i < 7; i++) {
        if (!result_byte(&st[i])) return 0;
    }
    if (!ok) return 0;

    /* ST0: IC 0 lub 1 (1 = koniec cylindra, normalne przy EOT = sektor); ST1 bez EN; ST2 = 0. */
    if ((st[0] & 0xC0) > 0x40) return 0;
    if ((st[1] & 0x7F) != 0) return 0;
    if (st[2] != 0) return 0;
    return 1;
}

static int floppy_rw(int write, uint32_t lba, uint8_t *rbuf, const uint8_t *wbuf, int tries)
{
    int a;

    if (!g_ctrl_ok || lba >= FLOPPY_SECTORS_TOTAL) return 0;
    if (!write && !rbuf) return 0;
    if (write && !wbuf) return 0;

    for (a = 0; a < tries; a++) {
        if (g_need_reset) {
            if (!floppy_reset()) continue;
            motor_on();
            if (!floppy_recalibrate()) { g_need_reset = 1; continue; }
        }
        if (floppy_rw_once(write, lba, rbuf, wbuf)) {
            if (g_tsc_per_us) g_motor_stamp = fl_rdtsc();
            else motor_off();          /* bez TSC nie mierzymy bezczynnosci */
            return 1;
        }
        g_need_reset = 1;              /* po bledzie: reset + recalibrate przed ponowieniem */
        g_cyl = 0xFF;
    }
    if (g_tsc_per_us) g_motor_stamp = fl_rdtsc();
    else motor_off();
    return 0;
}

int floppy_read_sector(uint32_t sector, char *buffer)
{
    return floppy_rw(0, sector, (uint8_t *)buffer, 0, 3);
}

int floppy_write_sector(uint32_t sector, const char *buffer)
{
    return floppy_rw(1, sector, 0, (const uint8_t *)buffer, 3);
}

int floppy_present(void)
{
    return g_floppy_present;
}

/* Typ napedu A z CMOS (rejestr 0x10, starsze 4 bity): 4 = 1.44 MB, 5 = 2.88 MB. */
static int cmos_drive_a_ok(void)
{
    uint8_t type;
    outb(0x70, 0x10);
    type = (uint8_t)(inb(0x71) >> 4);
    return type == 4 || type == 5;
}

void floppy_init(void)
{
    char buf[512];

    g_floppy_present = 0;
    g_ctrl_ok = 0;
    g_need_reset = 0;
    g_motor_on = 0;
    g_cyl = 0xFF;

    tsc_calibrate();

    if (!cmos_drive_a_ok()) return;
    if (!floppy_reset()) return;
    g_ctrl_ok = 1;

    motor_on();
    if (!floppy_recalibrate()) {
        g_need_reset = 1;
        motor_off();
        return;
    }

    if (floppy_rw(0, 0, (uint8_t *)buf, 0, 2)) {
        uint16_t bps = (uint16_t)((uint8_t)buf[11] | ((uint16_t)(uint8_t)buf[12] << 8));
        if (bps == 512) g_floppy_present = 1;
    }
}