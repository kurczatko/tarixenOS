volatile unsigned long long timer_ticks = 0; 

void d10() {
    unsigned long long target_ticks = timer_ticks + 1000; 

    while (timer_ticks < target_ticks) {
        __asm__ volatile("hlt");
    }
}
