[org 0x7C00]
[bits 16]

KERNEL_SECTORS      equ 1536
KERNEL_START_LBA    equ 126000
STAGE2_LBA          equ 8
STAGE2_SECTORS      equ 4
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
    mov si, stage2_dap
    mov ah, 0x42
    int 0x13
    jc disk_error
    jmp stage2

disk_error:
    hlt
    jmp disk_error

boot_drive db 0

stage2_dap:
    db 0x10
    db 0
    dw STAGE2_SECTORS
    dw 0x7E00
    dw 0x0000
    dd STAGE2_LBA
    dd 0

times 510 - ($ - $$) db 0
dw 0xAA55

stage2:
%include "graphics/vesa.asm"
    xor ax, ax
    mov es, ax

    mov cx, KERNEL_SECTORS / 32
    mov dword [dest_addr], 0x100000
    mov dword [dap_lba_low], KERNEL_START_LBA

read_kernel_loop:
    push cx
    mov dl, [boot_drive]
    mov si, disk_address_packet
    mov ah, 0x42
    int 0x13
    jc disk_error

    mov eax, [dest_addr]
    mov [dst_base_lo], ax
    shr eax, 16
    mov [dst_base_hi], al

    xor ax, ax
    mov es, ax
    mov si, copy_gdt
    mov cx, 0x2000
    mov ah, 0x87
    int 0x15
    jc disk_error

    add dword [dest_addr], 0x4000
    add dword [dap_lba_low], 32
    pop cx
    loop read_kernel_loop

    in al, 0x92
    or al, 2
    and al, 0xFE
    out 0x92, al

    lgdt [gdt_descriptor]

    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:protected_mode

dest_addr  dd 0

disk_address_packet:
    db 0x10
    db 0
    dw 32
    dw 0x0000
    dw 0x1000
dap_lba_low:
    dd KERNEL_START_LBA
    dd 0

copy_gdt:
    dq 0
    dq 0
    dw 0xFFFF
    dw 0x0000
    db 0x01
    db 0x93
    dw 0
    dw 0xFFFF
dst_base_lo:
    dw 0
dst_base_hi:
    db 0
    db 0x93
    dw 0
    dq 0
    dq 0

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
    mov esp, 0x02000000
    jmp CODE_SEG:0x100000

times 512 * (1 + STAGE2_SECTORS) - ($ - $$) db 0