.section .text
.code32
.extern proces_tick
.extern current_process
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

    movl current_process, %edx
    testl %edx, %edx
    jz 1f

    movl %esp, (%edx)                  # regs.esp
    movl %ebp, 4(%edx)                 # regs.ebp

    movl 40(%esp), %eax                # oryginalny eax
    movl %eax, 8(%edx)                 # regs.eax
    movl 36(%esp), %eax                # oryginalny ebx
    movl %eax, 12(%edx)                # regs.ebx
    movl 32(%esp), %eax                # oryginalny ecx
    movl %eax, 16(%edx)                # regs.ecx
    movl 28(%esp), %eax                # oryginalny edx
    movl %eax, 20(%edx)                # regs.edx
    movl 24(%esp), %eax                # oryginalny esi
    movl %eax, 24(%edx)                # regs.esi
    movl 20(%esp), %eax                # oryginalny edi
    movl %eax, 28(%edx)                # regs.edi

    pushf
    popl %eax
    movl %eax, 32(%edx)                # regs.eflags

    movl %cs, %eax
    movl %eax, 36(%edx)                # regs.cs
    movl %ds, %eax
    movl %eax, 40(%edx)                # regs.ds
    movl %es, %eax
    movl %eax, 44(%edx)                # regs.es
    movl %fs, %eax
    movl %eax, 48(%edx)                # regs.fs
    movl %gs, %eax
    movl %eax, 52(%edx)                # regs.gs
    movl %ss, %eax
    movl %eax, 56(%edx)                # regs.ss

1:
    call proces_tick

    movl current_process, %edx
    testl %edx, %edx
    jz 2f

    movl 72(%edx), %eax                # page_directory -> cr3
    movl %eax, %cr3

    movl 56(%edx), %eax
    movl %eax, %ss
    movl 52(%edx), %eax
    movl %eax, %gs
    movl 48(%edx), %eax
    movl %eax, %fs
    movl 44(%edx), %eax
    movl %eax, %es
    movl 40(%edx), %eax
    movl %eax, %ds
    movl 32(%edx), %eax
    pushl %eax
    popf                               # regs.eflags
    movl 28(%edx), %edi
    movl 24(%edx), %esi
    movl 20(%edx), %edx
    movl 16(%edx), %ecx
    movl 12(%edx), %ebx
    movl 8(%edx), %eax
    movl 4(%edx), %ebp
    movl (%edx), %esp

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
