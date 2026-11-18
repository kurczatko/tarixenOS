.section .text
.code32
.global syscall_asm_handler
.global userspace_uruchom
.extern biezace_rejestry
.extern syscall_handler
.extern current_process
.extern gdt_tss

syscall_asm_handler:
    push %ds
    push %es
    push %fs
    push %gs
    push %eax
    push %ebx
    push %ecx
    push %edx
    push %esi
    push %edi
    push %ebp
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    movl 24(%esp), %eax
    movl %eax, biezace_rejestry
    movl 20(%esp), %eax
    movl %eax, biezace_rejestry + 4
    movl 16(%esp), %eax
    movl %eax, biezace_rejestry + 8
    movl 12(%esp), %eax
    movl %eax, biezace_rejestry + 12
    movl 8(%esp), %eax
    movl %eax, biezace_rejestry + 16
    movl 4(%esp), %eax
    movl %eax, biezace_rejestry + 20
    call syscall_handler
    pop %ebp
    pop %edi
    pop %esi
    pop %edx
    pop %ecx
    pop %ebx
    pop %eax
    pop %gs
    pop %fs
    pop %es
    pop %ds
    iret

userspace_uruchom:
    mov 4(%esp), %eax
    mov %eax, %ecx
    mov 8(%esp), %edx
    mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    push $0x23
    push %edx
    pushf
    push $0x1B
    push %ecx
    iret
