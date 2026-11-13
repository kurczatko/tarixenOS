#ifndef BATERIA_H
#define BATERIA_H

#include <include/printf/types.h>

enum stan_baterii {
    BATERIA_NIEZNANA = 0,
    BATERIA_ROZLADOWYWUJE_SIE,
    BATERIA_LADUJE_SIE,
    BATERIA_PENA,
    BATERIA_KRYTYCZNA,
    BATERIA_BRAK_BATERII
};

struct bateria_info {
    uint8_t procent;
    uint32_t pojemnosc_aktualna_mah;
    uint32_t pojemnosc_projektowa_mah;
    uint32_t pojemnosc_pelna_mah;
    uint32_t napiecie_mv;
    int32_t prad_ma;
    int32_t temperatura_c;
    uint16_t cykle_ladowania;
    uint8_t zuzycie_procent;
    uint32_t czas_do_pelnego_min;
    uint32_t czas_do_rozladowania_min;
    enum stan_baterii stan;
    char producent[32];
    char model[32];
    char numer_seryjny[32];
    char chemia[16];
    uint8_t dostepna;
    uint8_t ladowarka_podlaczone;
    uint8_t inteligentna;
    uint8_t rezerwa[8];
};

#ifdef __cplusplus

class Bateria {
public:
    Bateria(void);
    int inicjalizuj(void);
    void odswiez(void);
    uint8_t czy_dostepna(void);
    void pobierz_info(struct bateria_info *info);
    uint8_t pobierz_procent(void);
    uint32_t pobierz_pojemnosc_mah(void);
    uint32_t pobierz_napiecie_mv(void);
    int32_t pobierz_prad_ma(void);
    int32_t pobierz_temperature_c(void);
    enum stan_baterii pobierz_stan(void);
    uint16_t pobierz_cykle(void);
    uint8_t pobierz_zuzycie(void);
    uint32_t pobierz_czas_do_pelnego_min(void);
    uint32_t pobierz_czas_do_rozladowania_min(void);
    uint8_t czy_ladowarka_podlaczone(void);
    void ustaw_prog_krytyczny(uint8_t procent);
    uint8_t pobierz_prog_krytyczny(void);
    uint8_t czy_krytyczny(void);
    void zarejestruj_callback(void (*funkcja)(enum stan_baterii));
    void usun_callback(void);

private:
    struct bateria_info dane;
    uint8_t prog_krytyczny;
    void (*zmiana_stanu)(enum stan_baterii);
    uint8_t ec_dostepny;
    uint8_t smb_dostepny;
    uint16_t ec_adres_bazowy;
    uint32_t licznik_odswiezen;
    enum stan_baterii stan_poprzedni;
    uint8_t inicjalizowane;
    uint16_t ostatni_status;
    int32_t ostatni_prad;

    uint8_t ec_pobierz_bajt(uint16_t rejestr);
    void ec_zapisz_bajt(uint16_t rejestr, uint8_t wartosc);
    uint16_t ec_pobierz_slowo(uint16_t rejestr);
    int sbs_transakcja(uint8_t polecenie, const uint8_t *zapisz, uint8_t zapisz_dlugosc, uint8_t *odczytaj, uint8_t *odczytaj_dlugosc);
    int sbs_pobierz_bajt(uint8_t polecenie, uint8_t *wartosc);
    int sbs_pobierz_slowo(uint8_t polecenie, uint16_t *wartosc);
    int sbs_pobierz_slowa(uint8_t polecenie, uint32_t *wartosc);
    int sbs_pobierz_lancuch(uint8_t polecenie, char *bufor, uint8_t rozmiar);
    int detekuj_baterie(void);
    void pobierz_identyfikatory(void);
    void aktualizuj_pomiar(void);
    void aktualizuj_stan(void);
    void oblicz_czas_pracy(void);
    enum stan_baterii stan_z_statusu(uint16_t status);
};

Bateria *bateria_pobierz(void);

#endif

#ifdef __cplusplus
extern "C" {
#endif

int bateria_init(void);
void bateria_odswiez(void);
uint8_t bateria_czy_dostepna(void);
void bateria_pobierz_info(struct bateria_info *info);
uint8_t bateria_pobierz_procent(void);
uint32_t bateria_pobierz_pojemnosc_mah(void);
uint32_t bateria_pobierz_napiecie_mv(void);
int32_t bateria_pobierz_prad_ma(void);
int32_t bateria_pobierz_temperature_c(void);
enum stan_baterii bateria_pobierz_stan(void);
uint16_t bateria_pobierz_cykle(void);
uint8_t bateria_pobierz_zuzycie(void);
uint32_t bateria_pobierz_czas_do_pelnego_min(void);
uint32_t bateria_pobierz_czas_do_rozladowania_min(void);
uint8_t bateria_czy_ladowarka_podlaczone(void);
void bateria_ustaw_prog_krytyczny(uint8_t procent);
uint8_t bateria_pobierz_prog_krytyczny(void);
uint8_t bateria_czy_krytyczny(void);
void bateria_zarejestruj_callback(void (*funkcja)(enum stan_baterii));
void bateria_usun_callback(void);
void bateria_ui_rysuj_pasek(void);
void bateria_poll(void);

#ifdef __cplusplus
}
#endif

#endif
