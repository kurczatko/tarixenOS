/*
 * prawa autorskie (C) gpl v3.0 naleza sie do mnie (radosław sitarski)
 * prosze nie kraść
 * 2026
 * kurczatkojestslodkie@gmail.com
 *
 * Helpery runtime kompilatora (compiler-rt) dla architektury i386.
 * Jądro linkowane jest bez -lgcc, więc symbole takie jak __udivdi3
 * muszą zostać dostarczone we wnętrzu jądra.
 */

/*
 * Dzielenie 64-bitowe unsigned: zwraca iloraz dividend / divisor.
 * Algorytm dzielenia binarnego (shift/subtract), bez wywoływania
 * innych helperów RT (unika łańcucha niespełnionych symboli).
 * Dla divisor == 0 zwraca 0 (bezpieczne zachowanie w jądrze).
*/
unsigned long long __udivdi3(unsigned long long dividend, unsigned long long divisor)
{
    unsigned long long quotient = 0;
    unsigned long long d = divisor;
    unsigned long long bit = 1ULL;

    if (divisor == 0) {
        return 0;
    }

    /* Przesuń dzielnik w lewo (razem z maską bit), dopóki nie przekroczy dzielnej
     * lub nie osiągnie bitu znaku (ochrona przed overflow przy <<= 1). */
    while (d < dividend && (d & (1ULL << 63)) == 0) {
        d <<= 1;
        bit <<= 1;
    }

    /* Dzielenie restoring: odjmuj dodawany dzielnik bit po bicie od najwyższego. */
    while (bit != 0) {
        if (dividend >= d) {
            dividend -= d;
            quotient |= bit;
        }
        d >>= 1;
        bit >>= 1;
    }

    return quotient;
}