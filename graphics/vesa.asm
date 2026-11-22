VBE_INFO_BUFFER     equ 0x9400
VBE_CONTROLLER_BUFFER equ 0x9000
VBE_HANDOFF_BUFFER  equ 0x0500

    mov dword [VBE_CONTROLLER_BUFFER], 0x32454256
    mov word [VBE_CONTROLLER_BUFFER + 4], 0
    mov ax, 0x4F00
    mov di, VBE_CONTROLLER_BUFFER
    int 0x10

    mov si, [VBE_CONTROLLER_BUFFER + 0x0E]
    mov ax, [VBE_CONTROLLER_BUFFER + 0x10]
    mov fs, ax
video_mode_loop:
    mov cx, [fs:si]
    cmp cx, 0xFFFF
    je video_mode_check_fallback
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
    cmp byte [VBE_INFO_BUFFER + 0x19], 24
    je video_mode_depth_ok2
    cmp byte [VBE_INFO_BUFFER + 0x19], 32
    jne video_mode_next
video_mode_depth_ok2:
    cmp byte [VBE_INFO_BUFFER + 0x1B], 6
    jne video_mode_next
    cmp dword [VBE_INFO_BUFFER + 0x28], 0
    je video_mode_next
    cmp word [VBE_INFO_BUFFER + 0x12], 1920
    jne video_mode_check_fallback_store
    cmp word [VBE_INFO_BUFFER + 0x14], 1080
    jne video_mode_check_fallback_store
    mov cx, [fs:si]
    mov [selected_vbe_mode], cx
    mov ax, 0x4F02
    mov bx, [selected_vbe_mode]
    or bx, 0x4000
    push si
    int 0x10
    pop si
    cmp ax, 0x004F
    jne video_mode_next
    mov ax, 0x4F01
    mov cx, [selected_vbe_mode]
    mov di, VBE_INFO_BUFFER
    push ds
    pop es
    push si
    int 0x10
    pop si
    cmp ax, 0x004F
    jne video_mode_next
    push si
    mov si, VBE_INFO_BUFFER
    mov ax, 0x0050
    mov es, ax
    xor di, di
    mov cx, 64
    cld
    rep movsd
    mov al, [VBE_CONTROLLER_BUFFER + 5]
    mov [es:0x00FF], al
    pop si
    jmp video_mode_ready

video_mode_check_fallback_store:
    cmp word [selected_vbe_mode], 0
    jne video_mode_next
    mov cx, [fs:si]
    mov [selected_vbe_mode], cx
video_mode_next:
    add si, 2
    jmp video_mode_loop

video_mode_check_fallback:
    cmp word [selected_vbe_mode], 0
    je video_mode_error
    mov cx, [selected_vbe_mode]
    mov ax, 0x4F02
    mov bx, [selected_vbe_mode]
    or bx, 0x4000
    int 0x10
    cmp ax, 0x004F
    jne video_mode_error
    mov ax, 0x4F01
    mov di, VBE_INFO_BUFFER
    push ds
    pop es
    int 0x10
    cmp ax, 0x004F
    jne video_mode_error
    push si
    mov si, VBE_INFO_BUFFER
    mov ax, 0x0050
    mov es, ax
    xor di, di
    mov cx, 64
    cld
    rep movsd
    mov al, [VBE_CONTROLLER_BUFFER + 5]
    mov [es:0x00FF], al
    pop si
    jmp video_mode_ready

video_mode_error:
    mov ax, 0x0003
    int 0x10
    cli
    hlt
    jmp video_mode_error

selected_vbe_mode:
    dw 0
video_mode_ready:
