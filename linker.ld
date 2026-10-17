ENTRY(kernel_main)

PHDRS
{
    text PT_LOAD FLAGS(0x5);  /* R, X */
    data PT_LOAD FLAGS(0x3);  /* R, W */
}

SECTIONS
{
    . = 0x1000;

    .text ALIGN(4) :
    {
        KEEP(*(.text.entry))
        *(.text)
        *(.text.*)
    } :text

    .rodata ALIGN(4) :
    {
        *(.rodata)
        *(.rodata.*)
    } :text

    .data ALIGN(4) :
    {
        *(.data)
        *(.data.*)
    } :data

    .bss ALIGN(4) :
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    } :data

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame)
        *(.note*)
    }

    .stack ALIGN(4) :
    {
        . += 0x1000;
    }
}
