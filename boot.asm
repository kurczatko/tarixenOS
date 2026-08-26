[org 0x7C00]
[bits 16]

KERNEL_LOAD_SEGMENT equ 0x0000
KERNEL_LOAD_OFFSET  equ 0x1000
KERNEL_SECTORS      equ 32
CODE_SEG            equ 0x08
DATA_SEG            equ 0x10


start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [boot_drive], dl

    mov dl, [boot_drive]
    mov si, disk_address_packet
    mov ah, 0x42
    int 0x13
    jc disk_error

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:protected_mode

disk_error:
    hlt
    jmp disk_error

boot_drive db 0

disk_address_packet:
    db 0x10
    db 0
    dw KERNEL_SECTORS
    dw KERNEL_LOAD_OFFSET
    dw KERNEL_LOAD_SEGMENT
    dq 1

align 8
gdt_start:
    dq 0x0000000000000000
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[bits 32]
protected_mode:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov esp, 0x90000

    jmp KERNEL_LOAD_OFFSET

times 510 - ($ - $$) db 0
dw 0xAA55 ; six seveeeeen