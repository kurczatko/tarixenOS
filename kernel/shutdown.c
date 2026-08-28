#include "shutdown.h"

void wylaczanie(void) {
	asm volatile ("outw %0, %1" : : "a"((uint16_t)0x2000), "Nd"((uint16_t)0x604));
	asm volatile ("outw %0, %1" : : "a"((uint16_t)0x2000), "Nd"((uint16_t)0xB004));
}

void shutdown(void) {
	wylaczanie();
}
