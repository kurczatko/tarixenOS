.section .text
.extern proces_tick
.extern current_process
.extern gdt_tss
.global timer_entry

timer_entry:
    push %eax
    push %ebx
    push %ecx
    push %edx
    push %esi
    push %edi
    push %ebp
    push %ds
    push %es
    push %fs
    push %gs

    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs

    movl current_process, %eax
    testl %eax, %eax
    jz 1f

    movl %esp, (%eax)
    movl %ebp, 4(%eax)
    movl %eax, 8(%eax)
    movl %ebx, 12(%eax)
    movl %ecx, 16(%eax)
    movl %edx, 20(%eax)
    movl %esi, 24(%eax)
    movl %edi, 28(%eax)
    pushf
    popl %edx
    movl %edx, 32(%eax)
    movl %cs, %edx
    movl %edx, 36(%eax)
    movl %ds, %edx
    movl %edx, 40(%eax)
    movl %es, %edx
    movl %edx, 44(%eax)
    movl %fs, %edx
    movl %edx, 48(%eax)
    movl %gs, %edx
    movl %edx, 52(%eax)
    movl %ss, %edx
    movl %edx, 56(%eax)

1:
    call proces_tick

    movl current_process, %eax
    testl %eax, %eax
    jz 2f

    movl 56(%eax), %ecx
    movl %ecx, %ss
    movl 52(%eax), %ecx
    movl %ecx, %gs
    movl 48(%eax), %ecx
    movl %ecx, %fs
    movl 44(%eax), %ecx
    movl %ecx, %es
    movl 40(%eax), %ecx
    movl %ecx, %ds
    movl 36(%eax), %ecx
    movl 32(%eax), %edx
    pushl %edx
    popf
    movl 28(%eax), %edi
    movl 24(%eax), %esi
    movl 20(%eax), %edx
    movl 16(%eax), %ecx
    movl 12(%eax), %ebx
    movl 8(%eax), %eax
    movl 4(%eax), %ebp
    movl (%eax), %esp

    movl (%eax), %eax
    movl %eax, %cr3

    movl gdt_tss, %ecx
    movl 76(%eax), %edx
    movl %edx, 4(%ecx)

2:
    pop %gs
    pop %fs
    pop %es
    pop %ds
    pop %ebp
    pop %edi
    pop %esi
    pop %edx
    pop %ecx
    pop %ebx
    pop %eax

    movb $0x20, %al
    outb %al, $0x20

    iret
