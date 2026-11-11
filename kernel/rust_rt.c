/* Minimal Rust runtime support for freestanding kernel linkage.
 * Provides symbols that rustc emits references to when compiling
 * with -C panic=abort and -C link-arg=-nodefaultlibs. */

void rust_eh_personality(void) {
    /* Unwind is disabled via panic=abort; this symbol is only
     * referenced to satisfy the linker. */
}

/* Rust emits references to these panic helpers for bounds-check
 * and arithmetic operations even with panic=abort. Provide them
 * so the freestanding kernel can link. */
void _RNvNtCs8TsIGpfNWzP_4core9panicking18panic_bounds_check(void) {
    for (;;) {}
}

void _RNvNtNtCs8TsIGpfNWzP_4core9panicking11panic_const23panic_const_rem_by_zero(void) {
    for (;;) {}
}
