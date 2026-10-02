[org 0x7C00]
[bits 16]

KERNEL_LOAD_SEGMENT equ 0x1000
KERNEL_LOAD_OFFSET  equ 0x1000
KERNEL_SECTORS      equ 512
CODE_SEG            equ 0x08
DATA_SEG            equ 0x10
VBE_INFO_BUFFER     equ 0x7E00
VBE_CONTROLLER_BUFFER equ 0x8000
VBE_HANDOFF_BUFFER  equ 0x50000

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

    mov dword [VBE_CONTROLLER_BUFFER], 0x32454256
    mov word [VBE_CONTROLLER_BUFFER + 4], 0
    mov ax, 0x4F00
    mov di, VBE_CONTROLLER_BUFFER
    push ds
    pop es
    int 0x10

    mov si, video_modes
video_mode_loop:
    mov cx, [si]
    cmp cx, 0xFFFF
    je video_mode_error
    mov ax, 0x4F01
    mov di, VBE_INFO_BUFFER
    push ds
    pop es
    int 0x10
    cmp ax, 0x004F
    jne video_mode_next
    mov ax, [VBE_INFO_BUFFER]
    and ax, 0x0091
    cmp ax, 0x0091
    jne video_mode_next
    cmp word [VBE_INFO_BUFFER + 0x12], 1024
    jne video_mode_next
    cmp word [VBE_INFO_BUFFER + 0x14], 768
    jne video_mode_next
    cmp byte [VBE_INFO_BUFFER + 0x19], 24
    je video_mode_depth_ok
    cmp byte [VBE_INFO_BUFFER + 0x19], 32
    jne video_mode_next
video_mode_depth_ok:
    cmp byte [VBE_INFO_BUFFER + 0x1B], 6
    jne video_mode_next
    cmp dword [VBE_INFO_BUFFER + 0x28], 0
    je video_mode_next
    push si
    mov si, VBE_INFO_BUFFER
    mov ax, 0x5000
    mov es, ax
    xor di, di
    mov cx, 64
    cld
    rep movsd
    mov al, [VBE_CONTROLLER_BUFFER + 5]
    mov [es:0x00FF], al
    pop si
    mov ax, 0x4F02
    mov bx, [si]
    or bx, 0x4000
    push si
    int 0x10
    pop si
    cmp ax, 0x004F
    je video_mode_ready
video_mode_next:
    add si, 2
    jmp video_mode_loop

video_mode_error:
    mov ax, 0x0003
    int 0x10
    mov si, video_error_message
video_error_print:
    lodsb
    test al, al
    jz video_error_halt
    mov ah, 0x0E
    int 0x10
    jmp video_error_print
video_error_halt:
    cli
    hlt
    jmp video_error_halt

video_modes:
    dw 0x0144, 0x0118, 0xFFFF
video_error_message:
    db 'No supported 1024x768 VBE mode', 0
video_mode_ready:

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
    mov edi, 0x90000
    mov ecx, reloc_code_end - reloc_code
    cld
    rep movsb

    jmp 0x90000

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