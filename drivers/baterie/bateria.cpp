/*
* prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
* prosze nie kraść
* 2026
* kurczatkojestslodkie@gmail.com
*/

#include <drivers/baterie/bateria.h>

#define PORT_EC_KOMENDA 0x66
#define PORT_EC_DANE 0x62

#define EC_STATUS_OBF 0x01
#define EC_STATUS_IBF 0x02

#define EC_KOMENDA_SBS 0x82
#define EC_KOMENDA_REJESTR 0x81

#define SBS_TEMPERATURA 0x08
#define SBS_NAPIECIE 0x09
#define SBS_PRAD 0x0a
#define SBS_STATUS 0x0d
#define SBS_CYKLE 0x0e
#define SBS_POZOSTALA_POJEMNOSC 0x0f
#define SBS_PELNA_POJEMNOSC 0x10
#define SBS_PROJEKTOWA_POJEMNOSC 0x11
#define SBS_NAPIECIE_PROJEKTOWE 0x12
#define SBS_NUMER_SERYJNY 0x13
#define SBS_PRODUCENT 0x14
#define SBS_MODEL 0x15
#define SBS_CHEMIA 0x16

#define FLAG_ROZLADOWANIE 0x0001
#define FLAG_LADOWANIE 0x0002
#define FLAG_KRYTYCZNA 0x0004
#define FLAG_PENA 0x0008
#define FLAG_BRAK 0x0010
#define FLAG_LADOWARKA 0x0020

static inline uint8_t inb(uint16_t port)
{
    uint8_t value;
    __asm__ volatile("inb %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

static inline void outb(uint16_t port, uint8_t value)
{
    __asm__ volatile("outb %0, %1" : : "a"(value), "Nd"(port));
}

static int ec_czekaj_obf(uint32_t timeout)
{
    while (timeout > 0) {
        timeout--;
        if ((inb(PORT_EC_KOMENDA) & EC_STATUS_OBF) != 0) {
            return 0;
        }
    }
    return -1;
}

static int ec_czekaj_ibf(uint32_t timeout)
{
    while (timeout > 0) {
        timeout--;
        if ((inb(PORT_EC_KOMENDA) & EC_STATUS_IBF) == 0) {
            return 0;
        }
    }
    return -1;
}

static Bateria bateria_g;

Bateria *bateria_pobierz(void)
{
    return &bateria_g;
}

Bateria::Bateria(void)
{
    uint8_t i;

    dane.procent = 0;
    dane.pojemnosc_aktualna_mah = 0;
    dane.pojemnosc_projektowa_mah = 0;
    dane.pojemnosc_pelna_mah = 0;
    dane.napiecie_mv = 0;
    dane.prad_ma = 0;
    dane.temperatura_c = 0;
    dane.cykle_ladowania = 0;
    dane.zuzycie_procent = 0;
    dane.czas_do_pelnego_min = 0;
    dane.czas_do_rozladowania_min = 0;
    dane.stan = BATERIA_NIEZNANA;
    dane.producent[0] = 0;
    dane.model[0] = 0;
    dane.numer_seryjny[0] = 0;
    dane.chemia[0] = 0;
    dane.dostepna = 0;
    dane.ladowarka_podlaczone = 0;
    dane.inteligentna = 0;
    for (i = 0; i < 8; i++) {
        dane.rezerwa[i] = 0;
    }

    prog_krytyczny = 10;
    zmiana_stanu = 0;
    ec_dostepny = 0;
    smb_dostepny = 0;
    ec_adres_bazowy = 0;
    licznik_odswiezen = 0;
    stan_poprzedni = BATERIA_NIEZNANA;
    inicjalizowane = 0;
    ostatni_status = 0;
    ostatni_prad = 0;
}

uint8_t Bateria::ec_pobierz_bajt(uint16_t rejestr)
{
    outb(PORT_EC_KOMENDA, EC_KOMENDA_REJESTR);
    ec_czekaj_ibf(10000);
    outb(PORT_EC_DANE, (uint8_t)rejestr);
    ec_czekaj_ibf(10000);
    ec_czekaj_obf(10000);
    return inb(PORT_EC_DANE);
}

void Bateria::ec_zapisz_bajt(uint16_t rejestr, uint8_t wartosc)
{
    outb(PORT_EC_KOMENDA, EC_KOMENDA_REJESTR);
    ec_czekaj_ibf(10000);
    outb(PORT_EC_DANE, (uint8_t)rejestr);
    ec_czekaj_ibf(10000);
    outb(PORT_EC_DANE, wartosc);
    ec_czekaj_ibf(10000);
}

uint16_t Bateria::ec_pobierz_slowo(uint16_t rejestr)
{
    uint16_t wartosc;
    wartosc = ec_pobierz_bajt(rejestr);
    wartosc |= (uint16_t)ec_pobierz_bajt((uint16_t)(rejestr + 1)) << 8;
    return wartosc;
}

int Bateria::sbs_transakcja(uint8_t polecenie, const uint8_t *zapisz, uint8_t zapisz_dlugosc, uint8_t *odczytaj, uint8_t *odczytaj_dlugosc)
{
    uint8_t i;

    if (ec_czekaj_ibf(10000) != 0) {
        return -1;
    }
    outb(PORT_EC_KOMENDA, EC_KOMENDA_SBS);
    if (ec_czekaj_ibf(10000) != 0) {
        return -1;
    }
    outb(PORT_EC_DANE, polecenie);

    if (zapisz != 0) {
        for (i = 0; i < zapisz_dlugosc; i++) {
            if (ec_czekaj_ibf(10000) != 0) {
                return -1;
            }
            outb(PORT_EC_DANE, zapisz[i]);
        }
    }

    if (odczytaj != 0 && odczytaj_dlugosc != 0) {
        *odczytaj_dlugosc = 0;
        for (i = 0; i < 32; i++) {
            if (ec_czekaj_obf(10000) != 0) {
                break;
            }
            odczytaj[*odczytaj_dlugosc] = inb(PORT_EC_DANE);
            (*odczytaj_dlugosc)++;
        }
    }

    return 0;
}

int Bateria::sbs_pobierz_bajt(uint8_t polecenie, uint8_t *wartosc)
{
    uint8_t buf[1];
    uint8_t dlugosc = 0;

    if (sbs_transakcja(polecenie, 0, 0, buf, &dlugosc) != 0) {
        return -1;
    }
    if (dlugosc < 1) {
        return -1;
    }
    *wartosc = buf[0];
    return 0;
}

int Bateria::sbs_pobierz_slowo(uint8_t polecenie, uint16_t *wartosc)
{
    uint8_t buf[2];
    uint8_t dlugosc = 0;

    if (sbs_transakcja(polecenie, 0, 0, buf, &dlugosc) != 0) {
        return -1;
    }
    if (dlugosc < 2) {
        return -1;
    }
    *wartosc = (uint16_t)buf[0] | ((uint16_t)buf[1] << 8);
    return 0;
}

int Bateria::sbs_pobierz_slowa(uint8_t polecenie, uint32_t *wartosc)
{
    uint8_t buf[4];
    uint8_t dlugosc = 0;

    if (sbs_transakcja(polecenie, 0, 0, buf, &dlugosc) != 0) {
        return -1;
    }
    if (dlugosc < 4) {
        return -1;
    }
    *wartosc = (uint32_t)buf[0] | ((uint32_t)buf[1] << 8) | ((uint32_t)buf[2] << 16) | ((uint32_t)buf[3] << 24);
    return 0;
}

int Bateria::sbs_pobierz_lancuch(uint8_t polecenie, char *bufor, uint8_t rozmiar)
{
    uint8_t buf[32];
    uint8_t dlugosc = 0;
    uint8_t znaki;
    uint8_t i;

    if (sbs_transakcja(polecenie, 0, 0, buf, &dlugosc) != 0) {
        return -1;
    }
    if (dlugosc == 0) {
        return -1;
    }
    znaki = buf[0];
    if (znaki > rozmiar - 1) {
        znaki = rozmiar - 1;
    }
    for (i = 0; i < znaki; i++) {
        bufor[i] = (char)buf[1 + i];
    }
    bufor[znaki] = 0;
    return 0;
}

int Bateria::detekuj_baterie(void)
{
    uint16_t wartosc = 0;

    if (sbs_pobierz_slowo(SBS_PROJEKTOWA_POJEMNOSC, &wartosc) == 0) {
        if (wartosc != 0 && wartosc != 0xFFFF) {
            dane.pojemnosc_projektowa_mah = wartosc;
            dane.inteligentna = 1;
            dane.dostepna = 1;
            return 0;
        }
    }

    if (ec_pobierz_bajt(0x00) != 0xFF) {
        dane.dostepna = 1;
        dane.inteligentna = 0;
        return 0;
    }

    return -1;
}

void Bateria::pobierz_identyfikatory(void)
{
    sbs_pobierz_lancuch(SBS_NUMER_SERYJNY, dane.numer_seryjny, 32);
    sbs_pobierz_lancuch(SBS_PRODUCENT, dane.producent, 32);
    sbs_pobierz_lancuch(SBS_MODEL, dane.model, 32);
    sbs_pobierz_lancuch(SBS_CHEMIA, dane.chemia, 16);
}

void Bateria::aktualizuj_pomiar(void)
{
    uint16_t wartosc16 = 0;
    uint32_t wartosc32 = 0;
    int16_t prad = 0;
    int32_t temperatura = 0;

    if (dane.inteligentna != 0) {
        if (sbs_pobierz_slowo(SBS_TEMPERATURA, &wartosc16) == 0) {
            temperatura = (int32_t)wartosc16;
            dane.temperatura_c = temperatura / 10 - 273;
        }
        if (sbs_pobierz_slowo(SBS_NAPIECIE, &wartosc16) == 0) {
            dane.napiecie_mv = wartosc16;
        }
        if (sbs_pobierz_slowo(SBS_PRAD, (uint16_t *)&prad) == 0) {
            dane.prad_ma = prad;
            ostatni_prad = prad;
        }
        if (sbs_pobierz_slowo(SBS_STATUS, &ostatni_status) != 0) {
            ostatni_status = 0;
        }
        if (sbs_pobierz_slowo(SBS_CYKLE, &wartosc16) == 0) {
            dane.cykle_ladowania = wartosc16;
        }
        if (sbs_pobierz_slowo(SBS_POZOSTALA_POJEMNOSC, &wartosc16) == 0) {
            dane.pojemnosc_aktualna_mah = wartosc16;
        }
        if (sbs_pobierz_slowo(SBS_PELNA_POJEMNOSC, &wartosc16) == 0) {
            dane.pojemnosc_pelna_mah = wartosc16;
        }
        if (sbs_pobierz_slowa(SBS_NAPIECIE_PROJEKTOWE, &wartosc32) == 0) {
            dane.pojemnosc_projektowa_mah = wartosc32 / 1000;
        }
    } else {
        dane.napiecie_mv = ec_pobierz_slowo(0x00);
        dane.prad_ma = (int32_t)(int16_t)ec_pobierz_slowo(0x02);
        dane.temperatura_c = (int32_t)ec_pobierz_bajt(0x04);
        dane.pojemnosc_aktualna_mah = ec_pobierz_bajt(0x05);
        dane.pojemnosc_pelna_mah = dane.pojemnosc_aktualna_mah;
        ostatni_status = ec_pobierz_bajt(0x06);
    }

    if (dane.pojemnosc_pelna_mah != 0) {
        dane.procent = (uint8_t)((uint32_t)dane.pojemnosc_aktualna_mah * 100 / (uint32_t)dane.pojemnosc_pelna_mah);
    } else {
        dane.procent = 0;
    }

    if (dane.pojemnosc_projektowa_mah != 0 && dane.pojemnosc_pelna_mah != 0) {
        dane.zuzycie_procent = (uint8_t)(100 - ((uint32_t)dane.pojemnosc_pelna_mah * 100 / (uint32_t)dane.pojemnosc_projektowa_mah));
    } else {
        dane.zuzycie_procent = 0;
    }

    if ((ostatni_status & FLAG_LADOWARKA) != 0 || dane.prad_ma >= 0) {
        dane.ladowarka_podlaczone = 1;
    } else {
        dane.ladowarka_podlaczone = 0;
    }
}

enum stan_baterii Bateria::stan_z_statusu(uint16_t status)
{
    if ((status & FLAG_BRAK) != 0) {
        return BATERIA_BRAK_BATERII;
    }
    if ((status & FLAG_KRYTYCZNA) != 0) {
        return BATERIA_KRYTYCZNA;
    }
    if ((status & FLAG_PENA) != 0) {
        return BATERIA_PENA;
    }
    if ((status & FLAG_LADOWANIE) != 0) {
        return BATERIA_LADUJE_SIE;
    }
    if ((status & FLAG_ROZLADOWANIE) != 0) {
        return BATERIA_ROZLADOWYWUJE_SIE;
    }
    return BATERIA_NIEZNANA;
}

void Bateria::aktualizuj_stan(void)
{
    dane.stan = stan_z_statusu(ostatni_status);

    if (dane.inteligentna == 0) {
        if (dane.prad_ma > 0) {
            dane.stan = BATERIA_LADUJE_SIE;
        } else if (dane.prad_ma < 0) {
            dane.stan = BATERIA_ROZLADOWYWUJE_SIE;
        }
    }

    if (dane.dostepna == 0) {
        dane.stan = BATERIA_BRAK_BATERII;
    }
}

void Bateria::oblicz_czas_pracy(void)
{
    int32_t prad = dane.prad_ma;
    int32_t roznica;

    if (prad < 0) {
        prad = -prad;
    }

    if (prad == 0) {
        dane.czas_do_pelnego_min = 0xFFFFFFFF;
        dane.czas_do_rozladowania_min = 0xFFFFFFFF;
        return;
    }

    if (dane.stan == BATERIA_LADUJE_SIE) {
        roznica = (int32_t)dane.pojemnosc_pelna_mah - (int32_t)dane.pojemnosc_aktualna_mah;
        if (roznica < 0) {
            roznica = 0;
        }
        dane.czas_do_pelnego_min = (uint32_t)((uint64_t)roznica * 60 / (uint64_t)prad);
        dane.czas_do_rozladowania_min = 0xFFFFFFFF;
    } else {
        dane.czas_do_rozladowania_min = (uint32_t)((uint64_t)dane.pojemnosc_aktualna_mah * 60 / (uint64_t)prad);
        dane.czas_do_pelnego_min = 0xFFFFFFFF;
    }
}

int Bateria::inicjalizuj(void)
{
    uint16_t wartosc = 0;

    dane.dostepna = 0;
    dane.inteligentna = 0;

    if (sbs_pobierz_slowo(SBS_PROJEKTOWA_POJEMNOSC, &wartosc) == 0) {
        if (wartosc != 0 && wartosc != 0xFFFF) {
            dane.pojemnosc_projektowa_mah = wartosc;
            dane.inteligentna = 1;
            dane.dostepna = 1;
        }
    }

    if (dane.dostepna == 0) {
        if (ec_pobierz_bajt(0x00) != 0xFF) {
            dane.dostepna = 1;
            dane.inteligentna = 0;
        } else {
            return -1;
        }
    }

    if (dane.inteligentna != 0) {
        pobierz_identyfikatory();
    }

    aktualizuj_pomiar();
    aktualizuj_stan();
    oblicz_czas_pracy();
    stan_poprzedni = dane.stan;
    inicjalizowane = 1;
    return 0;
}

void Bateria::odswiez(void)
{
    if (inicjalizowane == 0) {
        return;
    }

    aktualizuj_pomiar();
    aktualizuj_stan();
    oblicz_czas_pracy();

    if (zmiana_stanu != 0 && dane.stan != stan_poprzedni) {
        zmiana_stanu(dane.stan);
        stan_poprzedni = dane.stan;
    }

    licznik_odswiezen++;
}

uint8_t Bateria::czy_dostepna(void)
{
    return dane.dostepna;
}

void Bateria::pobierz_info(struct bateria_info *info)
{
    if (info != 0) {
        *info = dane;
    }
}

uint8_t Bateria::pobierz_procent(void)
{
    return dane.procent;
}

uint32_t Bateria::pobierz_pojemnosc_mah(void)
{
    return dane.pojemnosc_aktualna_mah;
}

uint32_t Bateria::pobierz_napiecie_mv(void)
{
    return dane.napiecie_mv;
}

int32_t Bateria::pobierz_prad_ma(void)
{
    return dane.prad_ma;
}

int32_t Bateria::pobierz_temperature_c(void)
{
    return dane.temperatura_c;
}

enum stan_baterii Bateria::pobierz_stan(void)
{
    return dane.stan;
}

uint16_t Bateria::pobierz_cykle(void)
{
    return dane.cykle_ladowania;
}

uint8_t Bateria::pobierz_zuzycie(void)
{
    return dane.zuzycie_procent;
}

uint32_t Bateria::pobierz_czas_do_pelnego_min(void)
{
    return dane.czas_do_pelnego_min;
}

uint32_t Bateria::pobierz_czas_do_rozladowania_min(void)
{
    return dane.czas_do_rozladowania_min;
}

uint8_t Bateria::czy_ladowarka_podlaczone(void)
{
    return dane.ladowarka_podlaczone;
}

void Bateria::ustaw_prog_krytyczny(uint8_t procent)
{
    prog_krytyczny = procent;
}

uint8_t Bateria::pobierz_prog_krytyczny(void)
{
    return prog_krytyczny;
}

uint8_t Bateria::czy_krytyczny(void)
{
    return dane.procent <= prog_krytyczny;
}

void Bateria::zarejestruj_callback(void (*funkcja)(enum stan_baterii))
{
    zmiana_stanu = funkcja;
}

void Bateria::usun_callback(void)
{
    zmiana_stanu = 0;
}

int bateria_init(void)
{
    return bateria_g.inicjalizuj();
}

void bateria_odswiez(void)
{
    bateria_g.odswiez();
}

uint8_t bateria_czy_dostepna(void)
{
    return bateria_g.czy_dostepna();
}

void bateria_pobierz_info(struct bateria_info *info)
{
    bateria_g.pobierz_info(info);
}

uint8_t bateria_pobierz_procent(void)
{
    return bateria_g.pobierz_procent();
}

uint32_t bateria_pobierz_pojemnosc_mah(void)
{
    return bateria_g.pobierz_pojemnosc_mah();
}

uint32_t bateria_pobierz_napiecie_mv(void)
{
    return bateria_g.pobierz_napiecie_mv();
}

int32_t bateria_pobierz_prad_ma(void)
{
    return bateria_g.pobierz_prad_ma();
}

int32_t bateria_pobierz_temperature_c(void)
{
    return bateria_g.pobierz_temperature_c();
}

enum stan_baterii bateria_pobierz_stan(void)
{
    return bateria_g.pobierz_stan();
}

uint16_t bateria_pobierz_cykle(void)
{
    return bateria_g.pobierz_cykle();
}

uint8_t bateria_pobierz_zuzycie(void)
{
    return bateria_g.pobierz_zuzycie();
}

uint32_t bateria_pobierz_czas_do_pelnego_min(void)
{
    return bateria_g.pobierz_czas_do_pelnego_min();
}

uint32_t bateria_pobierz_czas_do_rozladowania_min(void)
{
    return bateria_g.pobierz_czas_do_rozladowania_min();
}

uint8_t bateria_czy_ladowarka_podlaczone(void)
{
    return bateria_g.czy_ladowarka_podlaczone();
}

void bateria_ustaw_prog_krytyczny(uint8_t procent)
{
    bateria_g.ustaw_prog_krytyczny(procent);
}

uint8_t bateria_pobierz_prog_krytyczny(void)
{
    return bateria_g.pobierz_prog_krytyczny();
}

uint8_t bateria_czy_krytyczny(void)
{
    return bateria_g.czy_krytyczny();
}

void bateria_zarejestruj_callback(void (*funkcja)(enum stan_baterii))
{
    bateria_g.zarejestruj_callback(funkcja);
}

void bateria_usun_callback(void)
{
    bateria_g.usun_callback();
}

void bateria_poll(void)
{
    bateria_odswiez();
}

extern "C" {
#include <graficzny/rysowanie.h>
#include <graficzny/czcionka.h>
}

#define BATERIA_IKONA_X (EKRAN_SZEROKOSC - 106)
#define BATERIA_IKONA_Y (EKRAN_WYSOKOSC - PASEK_ZADAN_WYSOKOSC + 4)
#define BATERIA_IKONA_W 24
#define BATERIA_IKONA_H 22

static void bateria_ui_liczba_na_tekst(uint32_t v, char *bufor, uint8_t rozmiar)
{
    char tmp[8];
    int8_t i = 0;
    int8_t j;
    if (rozmiar == 0) return;
    if (v == 0) { bufor[0] = '0'; bufor[1] = 0; return; }
    while (v > 0 && i < 7) { tmp[i++] = (char)('0' + (v % 10)); v /= 10; }
    tmp[i] = 0;
    j = 0;
    while (i > 0) { bufor[j++] = tmp[--i]; }
    bufor[j] = 0;
}

static void bateria_ui_draw_icon(int x, int y, uint8_t procent, uint8_t ladowarka, enum stan_baterii stan)
{
    uint32_t kolor_wypelnienia;
    if (stan == BATERIA_KRYTYCZNA || procent < 15) kolor_wypelnienia = 0x00FF0000;
    else if (procent < 40) kolor_wypelnienia = 0x00FFFF00;
    else kolor_wypelnienia = 0x0000FF00;
    if (ladowarka != 0) kolor_wypelnienia = 0x000000FF;
    rysuj_prostokat(x, y, BATERIA_IKONA_W, BATERIA_IKONA_H, 0x003A4B5C);
    rysuj_prostokat(x + 8, y, 8, 4, 0x003A4B5C);
    rysuj_prostokat(x + 2, y + 4, 20, 16, 0x00000000);
    {
        uint8_t wypelnienie = (uint8_t)((uint16_t)procent * 16 / 100);
        if (wypelnienie > 16) wypelnienie = 16;
        uint8_t i;
        for (i = 0; i < wypelnienie; i++) {
            rysuj_prostokat(x + 4, y + 6 + (16 - wypelnienie) + i, 16, 1, kolor_wypelnienia);
        }
    }
    if (ladowarka != 0) {
        rysuj_piksel(x + 10, y + 2, 0x00FFFFFF);
        rysuj_piksel(x + 14, y + 2, 0x00FFFFFF);
        rysuj_piksel(x + 12, y + 3, 0x00FFFFFF);
    }
    rysuj_prostokat(x, y, BATERIA_IKONA_W, 1, 0x00FFFFFF);
    rysuj_prostokat(x, y, 1, BATERIA_IKONA_H, 0x00FFFFFF);
    rysuj_prostokat(x + BATERIA_IKONA_W - 1, y, 1, BATERIA_IKONA_H, 0x00FFFFFF);
    rysuj_prostokat(x, y + BATERIA_IKONA_H - 1, BATERIA_IKONA_W, 1, 0x00FFFFFF);
}

void bateria_ui_rysuj_pasek(void)
{
    if (bateria_czy_dostepna() == 0) return;
    uint8_t procent = bateria_pobierz_procent();
    uint8_t ladowarka = bateria_czy_ladowarka_podlaczone();
    enum stan_baterii stan = bateria_pobierz_stan();
    bateria_ui_draw_icon(BATERIA_IKONA_X, BATERIA_IKONA_Y, procent, ladowarka, stan);
    char bufor[8];
    char tekst[10];
    uint8_t i = 0;
    bateria_ui_liczba_na_tekst(procent, bufor, 8);
    while (bufor[i] != 0 && i < 8) { tekst[i] = bufor[i]; i++; }
    if (i < 9) { tekst[i++] = '%'; tekst[i] = 0; }
    rysuj_tekst_chicago(BATERIA_IKONA_X - 30, BATERIA_IKONA_Y + 2, tekst, 0x00FFFFFF);
}
