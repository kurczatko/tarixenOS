/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/storage/plyty.h>

/* Rejestry ATA. */
#define ATA_DATA          0
#define ATA_ERROR         1
#define ATA_FEATURES      1
#define ATA_SECTOR_COUNT  2
#define ATA_LBA_LOW       3
#define ATA_LBA_MID       4
#define ATA_LBA_HIGH      5
#define ATA_DRIVE         6
#define ATA_STATUS        7
#define ATA_COMMAND       7

/* Bity rejestru statusu ATA. */
#define ATA_SR_ERR        0x01
#define ATA_SR_DRQ        0x08
#define ATA_SR_DF         0x20
#define ATA_SR_DRDY       0x40
#define ATA_SR_BSY        0x80

/* Polecenia ATA. */
#define ATA_CMD_PACKET          0xA0
#define ATA_CMD_IDENTIFY_PACKET 0xA1

/* Polecenia SCSI używane przez ATAPI. */
#define SCSI_TEST_UNIT_READY    0x00
#define SCSI_REQUEST_SENSE      0x03
#define SCSI_INQUIRY            0x12
#define SCSI_START_STOP_UNIT    0x1B
#define SCSI_PREVENT_ALLOW      0x1E
#define SCSI_READ_CAPACITY10    0x25
#define SCSI_READ10             0x28
#define SCSI_READ12             0xA8
#define SCSI_GET_CONFIGURATION  0x46
#define SCSI_READ16             0x88
#define SCSI_READ_CAPACITY16    0x9E

#define PLY_TIMEOUT             1000000U
#define PLY_MAX_PHASE_BYTES     0xF800U
#define PLY_MAX_SENSE_BYTES     18U
#define PLY_MAX_SECTOR_SIZE     4096U
#define PLY_ISO_SECTOR_SIZE     2048U
#define PLY_MAX_PATH_DEPTH      32U
#define PLY_MAX_NAME            255U

struct ply_urzadzenie
{
    unsigned short io;
    unsigned short ctrl;
    unsigned char slave;
    unsigned char packet_size;
    unsigned char znaleziony;
    unsigned char gotowy;
    unsigned char zmiana_nosnika;
    unsigned char typ_peryferyjny;
    unsigned short profil;
    unsigned long long sektory;
    unsigned int rozmiar_sektora;
    unsigned char klucz_sense;
    unsigned char asc;
    unsigned char ascq;
    char producent[9];
    char produkt[17];
    char rewizja[5];
};

struct ply_iso_volume
{
    unsigned char aktywny;
    unsigned char joliet;
    unsigned long long root_lba;
    unsigned long long root_size;
    char etykieta[33];
    char system[16];
};

static struct ply_urzadzenie g_ply;
static struct ply_iso_volume g_iso;
static unsigned char g_bufor_sense[PLY_MAX_SENSE_BYTES];

static void ply_memset(void* dst, unsigned char wartosc, unsigned int ile)
{
    unsigned char* d = (unsigned char*)dst;
    for (unsigned int i = 0; i < ile; ++i)
        d[i] = wartosc;
}

static void ply_memcpy(void* dst, const void* src, unsigned int ile)
{
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;
    for (unsigned int i = 0; i < ile; ++i)
        d[i] = s[i];
}


static int ply_strcmp_ascii(const char* a, const char* b)
{
    unsigned int i = 0;
    if (!a || !b) return 1;
    while (a[i] && b[i])
    {
        char ca = a[i];
        char cb = b[i];
        if (ca >= 'a' && ca <= 'z') ca = (char)(ca - 'a' + 'A');
        if (cb >= 'a' && cb <= 'z') cb = (char)(cb - 'a' + 'A');
        if (ca != cb) return (int)((unsigned char)ca - (unsigned char)cb);
        ++i;
    }
    return (int)((unsigned char)a[i] - (unsigned char)b[i]);
}

static void ply_copy_string(char* dst, const char* src, unsigned int maks)
{
    unsigned int i = 0;
    if (!maks) return;
    if (src)
    {
        while (i + 1 < maks && src[i])
        {
            dst[i] = src[i];
            ++i;
        }
    }
    dst[i] = 0;
}

static inline void ply_outb(unsigned short port, unsigned char value)
{
    __asm__ volatile ("outb %0, %1" : : "a"(value), "Nd"(port));
}

static inline unsigned char ply_inb(unsigned short port)
{
    unsigned char value;
    __asm__ volatile ("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void ply_outw(unsigned short port, unsigned short value)
{
    __asm__ volatile ("outw %0, %1" : : "a"(value), "Nd"(port));
}

static inline unsigned short ply_inw(unsigned short port)
{
    unsigned short value;
    __asm__ volatile ("inw %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static void ply_opoznienie_400ns(void)
{
    (void)ply_inb(g_ply.ctrl);
    (void)ply_inb(g_ply.ctrl);
    (void)ply_inb(g_ply.ctrl);
    (void)ply_inb(g_ply.ctrl);
}

static unsigned char ply_status(void)
{
    return ply_inb(g_ply.ctrl);
}

static int ply_czekaj_bez_bsy(void)
{
    for (unsigned int i = 0; i < PLY_TIMEOUT; ++i)
    {
        unsigned char s = ply_status();
        if (s == 0x00 || s == 0xFF) return 0;
        if (!(s & ATA_SR_BSY)) return 1;
    }
    return 0;
}

static int ply_czekaj_drq(void)
{
    for (unsigned int i = 0; i < PLY_TIMEOUT; ++i)
    {
        unsigned char s = ply_status();
        if (s == 0x00 || s == 0xFF) return 0;
        if (s & (ATA_SR_ERR | ATA_SR_DF)) return 0;
        if (!(s & ATA_SR_BSY) && (s & ATA_SR_DRQ)) return 1;
    }
    return 0;
}

static int ply_czekaj_koniec(void)
{
    for (unsigned int i = 0; i < PLY_TIMEOUT; ++i)
    {
        unsigned char s = ply_status();
        if (s == 0x00 || s == 0xFF) return 0;
        if (s & (ATA_SR_ERR | ATA_SR_DF)) return 0;
        if (!(s & ATA_SR_BSY) && !(s & ATA_SR_DRQ)) return 1;
    }
    return 0;
}

static void ply_wybierz_naped(void)
{
    ply_outb((unsigned short)(g_ply.io + ATA_DRIVE),
             (unsigned char)(0xA0 | (g_ply.slave ? 0x10 : 0x00)));
    ply_opoznienie_400ns();
}

static unsigned short ply_be16(const unsigned char* p)
{
    return (unsigned short)(((unsigned short)p[0] << 8) | p[1]);
}

static unsigned int ply_be32(const unsigned char* p)
{
    return ((unsigned int)p[0] << 24) |
           ((unsigned int)p[1] << 16) |
           ((unsigned int)p[2] << 8) | (unsigned int)p[3];
}

static unsigned long long ply_be64(const unsigned char* p)
{
    return ((unsigned long long)ply_be32(p) << 32) | ply_be32(p + 4);
}


static unsigned int ply_le32(const unsigned char* p)
{
    return (unsigned int)p[0] |
           ((unsigned int)p[1] << 8) |
           ((unsigned int)p[2] << 16) |
           ((unsigned int)p[3] << 24);
}


/*
 * Przesyła jedną fazę danych. Nadmiar danych z napędu jest opróżniany,
 * ale nigdy nie jest zapisywany poza buforem wywołującego.
 */
static int ply_przenies_faze(unsigned char* bufor,
                             unsigned int pojemnosc,
                             unsigned int* przeslano,
                             unsigned int dlugosc,
                             int naped_do_ram)
{
    unsigned int pozostalo;
    unsigned short slowo;

    if (!ply_czekaj_bez_bsy()) return 0;

    {
        unsigned char status = ply_status();
        unsigned char reason;
        unsigned int bajty;

        if (status & (ATA_SR_ERR | ATA_SR_DF)) return 0;
        if (!(status & ATA_SR_DRQ)) return 1;

        reason = ply_inb((unsigned short)(g_ply.io + ATA_SECTOR_COUNT));
        if (reason & 0x01) return 0;
        if (((reason & 0x02) != 0) != (naped_do_ram != 0)) return 0;

        bajty = (unsigned int)ply_inb((unsigned short)(g_ply.io + ATA_LBA_MID));
        bajty |= (unsigned int)ply_inb((unsigned short)(g_ply.io + ATA_LBA_HIGH)) << 8;
        if (bajty == 0) bajty = 65536U;
        pozostalo = bajty;
    }

    while (pozostalo)
    {
        unsigned int porcja = pozostalo >= 2 ? 2 : 1;
        if (naped_do_ram)
        {
            slowo = ply_inw(g_ply.io + ATA_DATA);
            if (*przeslano < dlugosc && *przeslano < pojemnosc)
                bufor[*przeslano] = (unsigned char)(slowo & 0xFF);
            ++(*przeslano);
            if (porcja == 2)
            {
                if (*przeslano < dlugosc && *przeslano < pojemnosc)
                    bufor[*przeslano] = (unsigned char)(slowo >> 8);
                ++(*przeslano);
            }
        }
        else
        {
            unsigned char lo = 0;
            unsigned char hi = 0;
            if (*przeslano < dlugosc && *przeslano < pojemnosc)
                lo = bufor[*przeslano];
            ++(*przeslano);
            if (porcja == 2)
            {
                if (*przeslano < dlugosc && *przeslano < pojemnosc)
                    hi = bufor[*przeslano];
                ++(*przeslano);
            }
            slowo = (unsigned short)(lo | ((unsigned short)hi << 8));
            ply_outw(g_ply.io + ATA_DATA, slowo);
        }
        pozostalo -= porcja;
    }

    if (*przeslano > dlugosc || *przeslano > pojemnosc)
        return 0;

    return 1;
}

/*
 * Wykonuje jeden pakiet ATAPI. Obsługuje dowolną liczbę faz danych,
 * a nie tylko jedną fazę o rozmiarze całego bufora.
 */
static int ply_packet(const unsigned char* cdb,
                      unsigned int cdb_dlugosc,
                      unsigned char* dane,
                      unsigned int dane_dlugosc,
                      int naped_do_ram)
{
    unsigned char pakiet[16];
    unsigned int przeslano = 0;
    unsigned int fazy = 0;
    unsigned int limit;
    unsigned char reason;

    if (!g_ply.znaleziony || !cdb) return 0;
    if (cdb_dlugosc != 12 && cdb_dlugosc != 16) return 0;
    if (dane_dlugosc && !dane) return 0;
    if (g_ply.packet_size != 12 && g_ply.packet_size != 16) return 0;
    if (cdb_dlugosc > g_ply.packet_size) return 0;
    if (!ply_czekaj_bez_bsy()) return 0;

    ply_wybierz_naped();
    ply_outb((unsigned short)(g_ply.io + ATA_FEATURES), 0);
    ply_outb((unsigned short)(g_ply.io + ATA_SECTOR_COUNT), 0);
    ply_outb((unsigned short)(g_ply.io + ATA_LBA_LOW), 0);

    limit = PLY_MAX_PHASE_BYTES;
    ply_outb((unsigned short)(g_ply.io + ATA_LBA_MID), (unsigned char)(limit & 0xFF));
    ply_outb((unsigned short)(g_ply.io + ATA_LBA_HIGH), (unsigned char)(limit >> 8));
    ply_outb((unsigned short)(g_ply.io + ATA_COMMAND), ATA_CMD_PACKET);

    if (!ply_czekaj_drq()) return 0;
    reason = ply_inb((unsigned short)(g_ply.io + ATA_SECTOR_COUNT));
    if ((reason & 0x03) != 0x01) return 0;

    ply_memset(pakiet, 0, sizeof(pakiet));
    ply_memcpy(pakiet, cdb, cdb_dlugosc);

    for (unsigned int i = 0; i < g_ply.packet_size; i += 2)
    {
        unsigned short w = pakiet[i];
        if (i + 1 < g_ply.packet_size)
            w |= (unsigned short)pakiet[i + 1] << 8;
        ply_outw(g_ply.io + ATA_DATA, w);
    }

    for (;;)
    {
        unsigned char status;
        if (++fazy > 65536U) return 0;
        if (!ply_czekaj_bez_bsy()) return 0;
        status = ply_status();
        if (status & (ATA_SR_ERR | ATA_SR_DF)) return 0;
        if (!(status & ATA_SR_DRQ)) break;

        reason = ply_inb((unsigned short)(g_ply.io + ATA_SECTOR_COUNT));
        if (reason & 0x01) return 0;
        if (((reason & 0x02) != 0) != (naped_do_ram != 0)) return 0;
        if (!ply_przenies_faze(dane, dane_dlugosc, &przeslano,
                               dane_dlugosc, naped_do_ram))
            return 0;
    }

    if (!ply_czekaj_koniec()) return 0;
    return przeslano == dane_dlugosc;
}

/* REQUEST SENSE nie może ponownie wywoływać REQUEST SENSE. */
static int ply_pobierz_sense(void)
{
    unsigned char cdb[12];
    ply_memset(cdb, 0, sizeof(cdb));
    ply_memset(g_bufor_sense, 0, sizeof(g_bufor_sense));
    cdb[0] = SCSI_REQUEST_SENSE;
    cdb[4] = PLY_MAX_SENSE_BYTES;

    if (!ply_packet(cdb, 12, g_bufor_sense, PLY_MAX_SENSE_BYTES, 1))
        return 0;

    g_ply.klucz_sense = (unsigned char)(g_bufor_sense[2] & 0x0F);
    g_ply.asc = g_bufor_sense[12];
    g_ply.ascq = g_bufor_sense[13];

    if (g_ply.klucz_sense == 6 || g_ply.asc == 0x28 || g_ply.asc == 0x29)
        g_ply.zmiana_nosnika = 1;

    return 1;
}

static int ply_packet_z_sense(const unsigned char* cdb,
                              unsigned int cdb_dlugosc,
                              unsigned char* dane,
                              unsigned int dlugosc,
                              int naped_do_ram)
{
    for (unsigned int proba = 0; proba < 3; ++proba)
    {
        if (ply_packet(cdb, cdb_dlugosc, dane, dlugosc, naped_do_ram))
            return 1;

        if (!ply_pobierz_sense())
            return 0;

        /* Unit Attention: urządzenie zgłasza zmianę stanu nośnika. */
        if (g_ply.klucz_sense == 6)
            continue;

        return 0;
    }
    return 0;
}

static int ply_inquiry(void)
{
    unsigned char cdb[12];
    unsigned char dane[36];
    ply_memset(cdb, 0, sizeof(cdb));
    ply_memset(dane, 0, sizeof(dane));
    cdb[0] = SCSI_INQUIRY;
    cdb[4] = sizeof(dane);
    if (!ply_packet_z_sense(cdb, 12, dane, sizeof(dane), 1)) return 0;

    for (unsigned int i = 0; i < 8; ++i)
        g_ply.producent[i] = (char)dane[8 + i];
    g_ply.producent[8] = 0;

    for (unsigned int i = 0; i < 16; ++i)
        g_ply.produkt[i] = (char)dane[16 + i];
    g_ply.produkt[16] = 0;

    for (unsigned int i = 0; i < 4; ++i)
        g_ply.rewizja[i] = (char)dane[32 + i];
    g_ply.rewizja[4] = 0;

    return 1;
}

static int ply_test_unit_ready(void)
{
    unsigned char cdb[12];
    ply_memset(cdb, 0, sizeof(cdb));
    cdb[0] = SCSI_TEST_UNIT_READY;
    if (ply_packet(cdb, 12, 0, 0, 1))
        return 1;
    (void)ply_pobierz_sense();
    return 0;
}

static int ply_get_configuration(void)
{
    unsigned char cdb[12];
    unsigned char dane[8];
    ply_memset(cdb, 0, sizeof(cdb));
    ply_memset(dane, 0, sizeof(dane));
    cdb[0] = SCSI_GET_CONFIGURATION;
    cdb[1] = 0x00;
    cdb[7] = 0;
    cdb[8] = (unsigned char)sizeof(dane);

    if (!ply_packet_z_sense(cdb, 12, dane, sizeof(dane), 1)) return 0;
    g_ply.profil = ply_be16(dane + 6);
    return 1;
}

static void ply_ustal_typ_napedu(void)
{
    switch (g_ply.profil)
    {
        case 0x0008: g_ply.typ_peryferyjny = PLY_TYP_CD_ROM; break;
        case 0x0009: g_ply.typ_peryferyjny = PLY_TYP_CD_R; break;
        case 0x000A: g_ply.typ_peryferyjny = PLY_TYP_CD_RW; break;
        case 0x0010: g_ply.typ_peryferyjny = PLY_TYP_DVD_ROM; break;
        case 0x0011: case 0x0015: case 0x0016:
            g_ply.typ_peryferyjny = PLY_TYP_DVD_R; break;
        case 0x0012: g_ply.typ_peryferyjny = PLY_TYP_DVD_RAM; break;
        case 0x0013: case 0x0014:
            g_ply.typ_peryferyjny = PLY_TYP_DVD_RW; break;
        case 0x001A: g_ply.typ_peryferyjny = PLY_TYP_DVD_PLUS_RW; break;
        case 0x001B: case 0x002B:
            g_ply.typ_peryferyjny = PLY_TYP_DVD_PLUS_R; break;
        case 0x0040: g_ply.typ_peryferyjny = PLY_TYP_BD_ROM; break;
        case 0x0041: case 0x0042:
            g_ply.typ_peryferyjny = PLY_TYP_BD_R; break;
        case 0x0043: g_ply.typ_peryferyjny = PLY_TYP_BD_RE; break;
        default: g_ply.typ_peryferyjny = PLY_TYP_INNY; break;
    }
}

static int ply_read_capacity(void)
{
    unsigned char cdb[16];
    unsigned char dane[32];
    unsigned int ostatni;
    unsigned int rozmiar;

    ply_memset(cdb, 0, sizeof(cdb));
    ply_memset(dane, 0, sizeof(dane));
    cdb[0] = SCSI_READ_CAPACITY10;

    if (!ply_packet_z_sense(cdb, 12, dane, 8, 1))
        return 0;

    ostatni = ply_be32(dane);
    rozmiar = ply_be32(dane + 4);

    if (rozmiar == 0 || rozmiar > PLY_MAX_SECTOR_SIZE)
        return 0;

    if (ostatni == 0xFFFFFFFFU)
    {
        ply_memset(cdb, 0, sizeof(cdb));
        ply_memset(dane, 0, sizeof(dane));
        cdb[0] = SCSI_READ_CAPACITY16;
        cdb[1] = 0x10;
        cdb[10] = 32;
        if (!ply_packet_z_sense(cdb, 16, dane, 32, 1))
            return 0;
        g_ply.sektory = ply_be64(dane) + 1ULL;
        rozmiar = ply_be32(dane + 8);
    }
    else
    {
        g_ply.sektory = (unsigned long long)ostatni + 1ULL;
    }

    if (rozmiar == 0 || rozmiar > PLY_MAX_SECTOR_SIZE)
        return 0;

    g_ply.rozmiar_sektora = rozmiar;
    return g_ply.sektory != 0;
}

static int ply_identify_atapi(unsigned short io, unsigned short ctrl, unsigned char slave)
{
    unsigned short ident[256];
    unsigned char status;
    unsigned char mid;
    unsigned char high;

    g_ply.io = io;
    g_ply.ctrl = ctrl;
    g_ply.slave = slave;
    g_ply.znaleziony = 0;
    g_ply.gotowy = 0;
    g_ply.packet_size = 12;

    ply_wybierz_naped();
    if (!ply_czekaj_bez_bsy()) return 0;

    ply_outb((unsigned short)(io + ATA_COMMAND), ATA_CMD_IDENTIFY_PACKET);
    ply_opoznienie_400ns();
    status = ply_status();
    if (status == 0x00 || status == 0xFF) return 0;

    mid = ply_inb((unsigned short)(io + ATA_LBA_MID));
    high = ply_inb((unsigned short)(io + ATA_LBA_HIGH));
    if (!((mid == 0x14 && high == 0xEB) ||
          (mid == 0x69 && high == 0x96)))
        return 0;

    if (!ply_czekaj_drq()) return 0;
    for (unsigned int i = 0; i < 256; ++i)
        ident[i] = ply_inw(io + ATA_DATA);

    if (!ply_czekaj_koniec()) return 0;

    g_ply.typ_peryferyjny = (unsigned char)((ident[0] >> 8) & 0x1F);
    if (g_ply.typ_peryferyjny != 0x05)
        return 0;

    /* W bitach 1:0 słowa 0 zapisany jest rozmiar pakietu ATAPI. */
    if ((ident[0] & 0x03) == 0x01)
        g_ply.packet_size = 16;
    else
        g_ply.packet_size = 12;

    g_ply.znaleziony = 1;
    g_ply.rozmiar_sektora = PLY_ISO_SECTOR_SIZE;
    g_ply.sektory = 0;
    g_ply.zmiana_nosnika = 1;
    return 1;
}

static int ply_wykryj_naped(void)
{
    static const unsigned short io[4] = { 0x1F0, 0x1F0, 0x170, 0x170 };
    static const unsigned short ctrl[4] = { 0x3F6, 0x3F6, 0x376, 0x376 };
    static const unsigned char slave[4] = { 0, 1, 0, 1 };

    for (unsigned int i = 0; i < 4; ++i)
    {
        if (ply_identify_atapi(io[i], ctrl[i], slave[i]))
            return 1;
    }
    return 0;
}

static int ply_odczytaj_bloki_natywnie(unsigned long long lba,
                                       unsigned int liczba,
                                       unsigned char* bufor)
{
    unsigned int pozostalo = liczba;
    unsigned char cdb[16];

    if (!g_ply.znaleziony || !g_ply.gotowy || !bufor || liczba == 0)
        return 0;
    if (lba >= g_ply.sektory || (unsigned long long)liczba > g_ply.sektory - lba)
        return 0;

    while (pozostalo)
    {
        unsigned int porcja = pozostalo;
        unsigned long long bajty64;
        unsigned int bajty;
        int ok;

        if (porcja > 32U) porcja = 32U;
        bajty64 = (unsigned long long)porcja * g_ply.rozmiar_sektora;
        if (bajty64 > 0xFFFFFFFFULL) return 0;
        bajty = (unsigned int)bajty64;

        ply_memset(cdb, 0, sizeof(cdb));
        if (lba <= 0xFFFFFFFFULL && porcja <= 0xFFFFU)
        {
            cdb[0] = SCSI_READ10;
            cdb[2] = (unsigned char)(lba >> 24);
            cdb[3] = (unsigned char)(lba >> 16);
            cdb[4] = (unsigned char)(lba >> 8);
            cdb[5] = (unsigned char)lba;
            cdb[7] = (unsigned char)(porcja >> 8);
            cdb[8] = (unsigned char)porcja;
            ok = ply_packet_z_sense(cdb, 12, bufor, bajty, 1);
        }
        else if (lba <= 0xFFFFFFFFULL)
        {
            cdb[0] = SCSI_READ12;
            cdb[2] = (unsigned char)(lba >> 24);
            cdb[3] = (unsigned char)(lba >> 16);
            cdb[4] = (unsigned char)(lba >> 8);
            cdb[5] = (unsigned char)lba;
            cdb[6] = (unsigned char)(porcja >> 24);
            cdb[7] = (unsigned char)(porcja >> 16);
            cdb[8] = (unsigned char)(porcja >> 8);
            cdb[9] = (unsigned char)porcja;
            ok = ply_packet_z_sense(cdb, 12, bufor, bajty, 1);
        }
        else
        {
            cdb[0] = SCSI_READ16;
            cdb[2] = (unsigned char)(lba >> 56);
            cdb[3] = (unsigned char)(lba >> 48);
            cdb[4] = (unsigned char)(lba >> 40);
            cdb[5] = (unsigned char)(lba >> 32);
            cdb[6] = (unsigned char)(lba >> 24);
            cdb[7] = (unsigned char)(lba >> 16);
            cdb[8] = (unsigned char)(lba >> 8);
            cdb[9] = (unsigned char)lba;
            cdb[10] = (unsigned char)(porcja >> 24);
            cdb[11] = (unsigned char)(porcja >> 16);
            cdb[12] = (unsigned char)(porcja >> 8);
            cdb[13] = (unsigned char)porcja;
            ok = ply_packet_z_sense(cdb, 16, bufor, bajty, 1);
        }

        if (!ok) return 0;

        bufor += bajty;
        lba += porcja;
        pozostalo -= porcja;
    }

    return 1;
}

static int ply_iso_odczytaj_sektor(unsigned long long lba, unsigned char* dane)
{
    if (g_ply.rozmiar_sektora != PLY_ISO_SECTOR_SIZE) return 0;
    return ply_odczytaj_bloki_natywnie(lba, 1, dane);
}

static void ply_etykieta_z_iso(const unsigned char* p, unsigned int dlugosc)
{
    unsigned int i = 0;
    while (i < dlugosc && i < 32 && p[i] != 0 && p[i] != ' ')
    {
        unsigned char c = p[i];
        if (c < 0x20 || c > 0x7E) c = '_';
        g_iso.etykieta[i] = (char)c;
        ++i;
    }
    while (i && g_iso.etykieta[i - 1] == ' ')
        --i;
    g_iso.etykieta[i] = 0;
}

static void ply_etykieta_z_joliet(const unsigned char* p, unsigned int dlugosc)
{
    unsigned int i = 0;
    unsigned int out = 0;
    while (i + 1 < dlugosc && out < 32)
    {
        unsigned short c = (unsigned short)(((unsigned short)p[i] << 8) | p[i + 1]);
        i += 2;
        if (c == 0 || c == ' ') break;
        if (c >= 0x20 && c <= 0x7E)
            g_iso.etykieta[out++] = (char)c;
        else
            g_iso.etykieta[out++] = '?';
    }
    while (out && g_iso.etykieta[out - 1] == ' ')
        --out;
    g_iso.etykieta[out] = 0;
}

static int ply_iso_poprawny_rekord_katalogu(const unsigned char* r,
                                             unsigned int pozostalo,
                                             unsigned int* dlugosc)
{
    unsigned int n;
    if (!pozostalo) return 0;
    n = r[0];
    if (n < 34 || n > pozostalo) return 0;
    if (33U + r[32] > n) return 0;
    *dlugosc = n;
    return 1;
}

static void ply_iso_nazwa(const unsigned char* r,
                          unsigned int rekord_dlugosc,
                          char* nazwa,
                          unsigned int maks)
{
    unsigned int n = r[32];
    unsigned int pos = 33;
    unsigned int out = 0;
    if (n == 1 && r[33] == 0)
    {
        ply_copy_string(nazwa, ".", maks);
        return;
    }
    if (n == 1 && r[33] == 1)
    {
        ply_copy_string(nazwa, "..", maks);
        return;
    }

    if (g_iso.joliet)
    {
        for (unsigned int i = 0; i + 1 < n && out + 1 < maks; i += 2)
        {
            unsigned short c = (unsigned short)(((unsigned short)r[pos + i] << 8) | r[pos + i + 1]);
            if (c == ';') break;
            if (c == '.')
            {
                nazwa[out++] = '.';
            }
            else if (c >= 0x20 && c <= 0x7E)
            {
                nazwa[out++] = (char)c;
            }
            else
            {
                nazwa[out++] = '?';
            }
        }
    }
    else
    {
        for (unsigned int i = 0; i < n && out + 1 < maks; ++i)
        {
            unsigned char c = r[pos + i];
            if (c == ';') break;
            if (c >= 'a' && c <= 'z') c = (unsigned char)(c - 'a' + 'A');
            nazwa[out++] = (char)c;
        }
    }

    /* Usuń końcową kropkę używaną przez zapis ISO9660. */
    if (out && nazwa[out - 1] == '.') --out;
    nazwa[out] = 0;

    (void)rekord_dlugosc;
}

static int ply_iso_wykryj(void)
{
    unsigned char sektor[PLY_ISO_SECTOR_SIZE];
    unsigned char pvd[PLY_ISO_SECTOR_SIZE];
    unsigned char svd[PLY_ISO_SECTOR_SIZE];
    int ma_pvd = 0;
    int ma_joliet = 0;
    unsigned int pvd_lba = 0;
    unsigned int svd_lba = 0;

    ply_memset(&g_iso, 0, sizeof(g_iso));

    if (g_ply.rozmiar_sektora != PLY_ISO_SECTOR_SIZE)
        return 0;

    for (unsigned int lba = 16; lba < 256 && lba < g_ply.sektory; ++lba)
    {
        if (!ply_iso_odczytaj_sektor(lba, sektor))
            return 0;

        if (sektor[1] != 'C' || sektor[2] != 'D' ||
            sektor[3] != '0' || sektor[4] != '0' ||
            sektor[5] != '1' || sektor[6] != 1)
            break;

        if (sektor[0] == 1 && !ma_pvd)
        {
            ply_memcpy(pvd, sektor, PLY_ISO_SECTOR_SIZE);
            ma_pvd = 1;
            pvd_lba = lba;
        }
        else if (sektor[0] == 2 &&
                 sektor[88] == '%' && sektor[89] == '/' &&
                 (sektor[90] == '@' || sektor[90] == 'C' || sektor[90] == 'E'))
        {
            ply_memcpy(svd, sektor, PLY_ISO_SECTOR_SIZE);
            ma_joliet = 1;
            svd_lba = lba;
        }
        else if (sektor[0] == 255)
        {
            break;
        }
    }

    if (!ma_pvd) return 0;

    if (ma_joliet)
    {
        const unsigned char* root = svd + 156;
        unsigned int root_len = root[0];
        if (root_len >= 34 && root_len <= 255 && (root[25] & 0x02))
        {
            g_iso.root_lba = ply_le32(root + 2);
            g_iso.root_size = ply_le32(root + 10);
            ply_etykieta_z_joliet(svd + 40, 32);
            g_iso.joliet = 1;
            g_iso.aktywny = 1;
        }
    }

    if (!g_iso.aktywny)
    {
        const unsigned char* root = pvd + 156;
        unsigned int root_len = root[0];
        if (root_len >= 34 && root_len <= 255 && (root[25] & 0x02))
        {
            g_iso.root_lba = ply_le32(root + 2);
            g_iso.root_size = ply_le32(root + 10);
            ply_etykieta_z_iso(pvd + 40, 32);
            g_iso.joliet = 0;
            g_iso.aktywny = 1;
        }
    }

    if (!g_iso.aktywny) return 0;
    if (g_iso.root_lba >= g_ply.sektory) return 0;
    if (g_iso.root_size > g_ply.sektory * (unsigned long long)g_ply.rozmiar_sektora)
        return 0;

    ply_copy_string(g_iso.system, g_iso.joliet ? "Joliet" : "ISO9660", sizeof(g_iso.system));
    (void)pvd_lba;
    (void)svd_lba;
    return 1;
}

static int ply_iso_iteruj_katalog(unsigned long long lba,
                                  unsigned long long rozmiar,
                                  struct plyty_wpis* wpisy,
                                  unsigned int maks,
                                  unsigned int* liczba)
{
    unsigned char sektor[PLY_ISO_SECTOR_SIZE];
    unsigned long long offset = 0;
    unsigned int dodano = 0;

    if (!liczba) return 0;
    *liczba = 0;

    while (offset < rozmiar)
    {
        unsigned long long aktualny_lba = lba + (offset / PLY_ISO_SECTOR_SIZE);
        unsigned int w_sektorze = (unsigned int)(offset % PLY_ISO_SECTOR_SIZE);
        unsigned int rekord_len;
        unsigned char* r;

        if (!ply_iso_odczytaj_sektor(aktualny_lba, sektor)) return 0;
        r = sektor + w_sektorze;

        if (r[0] == 0)
        {
            offset += PLY_ISO_SECTOR_SIZE - w_sektorze;
            continue;
        }

        if (!ply_iso_poprawny_rekord_katalogu(r,
                PLY_ISO_SECTOR_SIZE - w_sektorze, &rekord_len))
            return 0;

        if (r[32] != 1 || (r[33] != 0 && r[33] != 1))
        {
            if (dodano < maks && wpisy)
            {
                struct plyty_wpis* e = &wpisy[dodano];
                ply_memset(e, 0, sizeof(*e));
                ply_iso_nazwa(r, rekord_len, e->nazwa, sizeof(e->nazwa));
                e->lba = ply_le32(r + 2);
                e->rozmiar = ply_le32(r + 10);
                e->katalog = (unsigned char)((r[25] & 0x02) != 0);
                e->atrybuty = r[25];
                ++dodano;
            }
        }

        offset += rekord_len;
    }

    *liczba = dodano;
    return 1;
}

static int ply_iso_szukaj_w_katalogu(unsigned long long lba,
                                     unsigned long long rozmiar,
                                     const char* nazwa,
                                     struct plyty_wpis* wynik)
{
    unsigned long long offset = 0;

    while (offset < rozmiar)
    {
        unsigned long long aktualny_lba = lba + offset / PLY_ISO_SECTOR_SIZE;
        unsigned int in_sector = (unsigned int)(offset % PLY_ISO_SECTOR_SIZE);
        unsigned char sektor[PLY_ISO_SECTOR_SIZE];
        unsigned char* r;
        unsigned int rlen;

        if (!ply_iso_odczytaj_sektor(aktualny_lba, sektor)) return 0;
        r = sektor + in_sector;
        if (r[0] == 0)
        {
            offset += PLY_ISO_SECTOR_SIZE - in_sector;
            continue;
        }
        if (!ply_iso_poprawny_rekord_katalogu(r, PLY_ISO_SECTOR_SIZE - in_sector, &rlen))
            return 0;

        if (!(r[32] == 1 && (r[33] == 0 || r[33] == 1)))
        {
            struct plyty_wpis e;
            ply_memset(&e, 0, sizeof(e));
            ply_iso_nazwa(r, rlen, e.nazwa, sizeof(e.nazwa));
            if (ply_strcmp_ascii(e.nazwa, nazwa) == 0)
            {
                e.lba = ply_le32(r + 2);
                e.rozmiar = ply_le32(r + 10);
                e.katalog = (unsigned char)((r[25] & 0x02) != 0);
                e.atrybuty = r[25];
                *wynik = e;
                return 1;
            }
        }
        offset += rlen;
    }

    return 0;
}

static int ply_iso_znajdz(const char* sciezka, struct plyty_wpis* wynik)
{
    char segment[256];
    unsigned int pos = 0;
    unsigned long long katalog_lba = g_iso.root_lba;
    unsigned long long katalog_rozmiar = g_iso.root_size;
    struct plyty_wpis znaleziony;
    unsigned int glebokosc = 0;

    if (!sciezka || !wynik || !g_iso.aktywny) return 0;
    while (sciezka[pos] == '/' || sciezka[pos] == '\\') ++pos;
    if (!sciezka[pos])
    {
        ply_memset(&znaleziony, 0, sizeof(znaleziony));
        ply_copy_string(znaleziony.nazwa, "/", sizeof(znaleziony.nazwa));
        znaleziony.lba = katalog_lba;
        znaleziony.rozmiar = katalog_rozmiar;
        znaleziony.katalog = 1;
        *wynik = znaleziony;
        return 1;
    }

    while (sciezka[pos])
    {
        unsigned int n = 0;
        while (sciezka[pos] && sciezka[pos] != '/' && sciezka[pos] != '\\')
        {
            if (n + 1 >= sizeof(segment)) return 0;
            segment[n++] = sciezka[pos++];
        }
        segment[n] = 0;
        while (sciezka[pos] == '/' || sciezka[pos] == '\\') ++pos;
        if (!n) continue;
        if (++glebokosc > PLY_MAX_PATH_DEPTH) return 0;

        if (!ply_iso_szukaj_w_katalogu(katalog_lba, katalog_rozmiar,
                                       segment, &znaleziony))
            return 0;

        if (sciezka[pos] && !znaleziony.katalog)
            return 0;

        katalog_lba = znaleziony.lba;
        katalog_rozmiar = znaleziony.rozmiar;
    }

    *wynik = znaleziony;
    return 1;
}

void plyty_init(void)
{
    ply_memset(&g_ply, 0, sizeof(g_ply));
    ply_memset(&g_iso, 0, sizeof(g_iso));
    g_ply.rozmiar_sektora = PLY_ISO_SECTOR_SIZE;

    if (!ply_wykryj_naped())
        return;

    if (!ply_inquiry())
    {
        g_ply.znaleziony = 0;
        return;
    }

    (void)ply_get_configuration();
    ply_ustal_typ_napedu();

    if (!ply_test_unit_ready())
    {
        g_ply.gotowy = 0;
        return;
    }

    if (!ply_read_capacity())
    {
        g_ply.gotowy = 0;
        return;
    }

    g_ply.gotowy = 1;
    g_ply.zmiana_nosnika = 0;
    (void)ply_iso_wykryj();
}

bool plyty_present(void)
{
    return g_ply.znaleziony != 0;
}

bool plyty_ready(void)
{
    return g_ply.znaleziony && g_ply.gotowy;
}

enum plyty_typ plyty_type(void)
{
    if (!g_ply.znaleziony) return PLY_TYP_BRAK;
    return (enum plyty_typ)g_ply.typ_peryferyjny;
}

unsigned long long plyty_sector_count(void)
{
    return g_ply.sektory;
}

unsigned int plyty_sector_size(void)
{
    return g_ply.rozmiar_sektora;
}

bool plyty_read_sector(unsigned long long lba, void* bufor)
{
    return plyty_read_sectors(lba, 1, bufor);
}

bool plyty_read_sectors(unsigned long long lba, unsigned int liczba, void* bufor)
{
    if (!bufor || !liczba) return false;
    return ply_odczytaj_bloki_natywnie(lba, liczba, (unsigned char*)bufor) != 0;
}

const char* plyty_volume_label(void)
{
    return g_iso.aktywny ? g_iso.etykieta : "";
}

const char* plyty_fs_type(void)
{
    return g_iso.aktywny ? g_iso.system : "";
}

bool plyty_list_root(struct plyty_wpis* wpisy, unsigned int maks, unsigned int* liczba)
{
    if (!g_iso.aktywny || !liczba) return false;
    return ply_iso_iteruj_katalog(g_iso.root_lba, g_iso.root_size,
                                  wpisy, maks, liczba) != 0;
}

bool plyty_find_file(const char* sciezka, struct plyty_wpis* wynik)
{
    if (!g_iso.aktywny || !sciezka || !wynik) return false;
    return ply_iso_znajdz(sciezka, wynik) != 0;
}

bool plyty_read_file(const char* sciezka,
                     void* bufor,
                     unsigned long long pojemnosc,
                     unsigned long long* odczytano)
{
    struct plyty_wpis wpis;
    unsigned long long pozostalo;
    unsigned long long lba;
    unsigned char* out = (unsigned char*)bufor;

    if (odczytano) *odczytano = 0;
    if (!bufor || !sciezka || !g_iso.aktywny) return false;
    if (!ply_iso_znajdz(sciezka, &wpis)) return false;
    if (wpis.katalog) return false;
    if (wpis.rozmiar > pojemnosc) return false;

    pozostalo = wpis.rozmiar;
    lba = wpis.lba;

    while (pozostalo)
    {
        unsigned char sektor[PLY_ISO_SECTOR_SIZE];
        unsigned int kopiuj;
        if (!ply_iso_odczytaj_sektor(lba, sektor)) return false;
        kopiuj = pozostalo > PLY_ISO_SECTOR_SIZE ?
                 PLY_ISO_SECTOR_SIZE : (unsigned int)pozostalo;
        ply_memcpy(out, sektor, kopiuj);
        out += kopiuj;
        pozostalo -= kopiuj;
        ++lba;
    }

    if (odczytano) *odczytano = wpis.rozmiar;
    return true;
}

bool plyty_disc_changed(void)
{
    if (!g_ply.znaleziony) return false;
    if (ply_test_unit_ready())
    {
        if (g_ply.zmiana_nosnika)
        {
            g_ply.zmiana_nosnika = 0;
            if (ply_read_capacity())
            {
                g_ply.gotowy = 1;
                (void)ply_iso_wykryj();
                return true;
            }
        }
        return false;
    }

    (void)ply_pobierz_sense();
    if (g_ply.klucz_sense == 6 || g_ply.asc == 0x28 ||
        (g_ply.klucz_sense == 2 && g_ply.asc == 0x3A))
    {
        g_ply.gotowy = 0;
        g_ply.zmiana_nosnika = 1;
        ply_memset(&g_iso, 0, sizeof(g_iso));
        return true;
    }
    return false;
}

bool plyty_eject(void)
{
    unsigned char cdb[12];
    if (!g_ply.znaleziony) return false;
    ply_memset(cdb, 0, sizeof(cdb));
    cdb[0] = SCSI_START_STOP_UNIT;
    cdb[4] = 0x02;
    if (!ply_packet_z_sense(cdb, 12, 0, 0, 1)) return false;
    g_ply.gotowy = 0;
    g_ply.zmiana_nosnika = 1;
    ply_memset(&g_iso, 0, sizeof(g_iso));
    return true;
}

bool plyty_close_tray(void)
{
    unsigned char cdb[12];
    if (!g_ply.znaleziony) return false;
    ply_memset(cdb, 0, sizeof(cdb));
    cdb[0] = SCSI_START_STOP_UNIT;
    cdb[4] = 0x03;
    if (!ply_packet_z_sense(cdb, 12, 0, 0, 1)) return false;
    g_ply.zmiana_nosnika = 1;
    return true;
}
