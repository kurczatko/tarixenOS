ENTRY(kernel_main)

PROVIDE(kernel_main = .);

SECTIONS
{
    . = 0x1000;

    .text ALIGN(4) :
    {
        *(.text)
        *(.text.*)
    }

    .rodata ALIGN(4) :
    {
        *(.rodata)
        *(.rodata.*)
    }

    .data ALIGN(4) :
    {
        *(.data)
        *(.data.*)
    }

    .bss ALIGN(4) :
    {
        *(COMMON)
        *(.bss)
        *(.bss.*)
    }

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
