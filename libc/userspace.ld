ENTRY(_start)
SECTIONS
{
    . = 0x00100000;
    .text ALIGN(4) : { *(.text) *(.text.*) }
    .rodata ALIGN(4) : { *(.rodata) *(.rodata.*) }
    .data ALIGN(4) : { *(.data) *(.data.*) }
    .bss ALIGN(4) : { *(COMMON) *(.bss) *(.bss.*) }
    . = ALIGN(4096);
    _end = .;
    .stack : { . += 0x1000; }
    /DISCARD/ : { *(.comment) *(.eh_frame) *(.note*) }
}
