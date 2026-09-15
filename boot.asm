[org 0x7C00]
[bits 16]

KERNEL_LOAD_SEGMENT equ 0x1000
KERNEL_LOAD_OFFSET  equ 0x1000
KERNEL_SECTORS      equ 512
CODE_SEG            equ 0x08
DATA_SEG            equ 0x10

    jmp short boot_start
    nop
    db 'TARIXEN '
    dw 512
    db 1
    dw 192
    db 2
    dw 0
    dw 0
    db 0xF8
    dw 0
    dw 63
    dw 255
    dd 0
    dd 131072
    dd 1024
    dw 0
    dw 0
    dd 2
    dw 1
    dw 6
    times 12 db 0
    db 0x80
    db 0
    db 0x29
    dd 0x20260909
    db 'TARIXENOS  '
    db 'FAT32   '

boot_start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    mov [boot_drive], dl

    mov ax, 0x0012
    int 0x10

    mov cx, KERNEL_SECTORS / 32
    mov word [dap_segment], 0x1000
    mov dword [dap_lba_low], 192

read_kernel_loop:
    push cx
    mov dl, [boot_drive]
    mov si, disk_address_packet
    mov ah, 0x42
    int 0x13
    jc disk_error

    add word [dap_segment], 0x0400
    add dword [dap_lba_low], 32
    pop cx
    loop read_kernel_loop

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
    dw 32
    dw 0x0000
dap_segment:
    dw 0x1000
dap_lba_low:
    dd 192
dap_lba_high:
    dd 0

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
    mov esp, 0x8F000

    mov esi, reloc_code
    mov edi, 0x8F000
    mov ecx, reloc_code_end - reloc_code
    cld
    rep movsb

    jmp 0x8F000

reloc_code:
    mov esi, 0x10000
    mov edi, KERNEL_LOAD_OFFSET
    mov ecx, (KERNEL_SECTORS * 512) / 4
    rep movsd

    mov eax, KERNEL_LOAD_OFFSET
    jmp eax
reloc_code_end:

times 510 - ($ - $$) db 0
dw 0xAA55