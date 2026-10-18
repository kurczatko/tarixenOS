.section .data
.global syscall_asm_handler
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
