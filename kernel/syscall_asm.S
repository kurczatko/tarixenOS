.section .text
.code32
.global syscall_asm_handler
.global userspace_uruchom
.extern biezace_rejestry
.extern syscall_handler

syscall_asm_handler:
    /* Wywolane z userspace (int 0x80). CPU nasunąl na stos ring 0:
     * ss, esp, eflags, cs, eip. Zapisujemy rejestry i przechodzimy na dane ring 0. */
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
    /* Powrot do userspace: iret ściaga ss, esp, eflags, cs, eip (ring zmiana). */
    iret

/* Przejscie do userspace (ring 3).
 * Argumenty: [esp+4] = wejscie (EIP), [esp+8] = stos uzytkownika (ESP). */
userspace_uruchom:
    mov 4(%esp), %eax
    mov 8(%esp), %edx
    /* Ustaw segmenty danych na ring 3 (dozwolone w ring 0: DPL=3 >= CPL=0). */
    mov $0x23, %ax
    mov %ax, %ds
    mov %ax, %es
    mov %ax, %fs
    mov %ax, %gs
    /* Ramka iret: ss, esp, eflags, cs, eip (ostatnio wsuniety = eip, ściagany pierwszy). */
    push $0x23          /* ss = ring3 dane | RPL3 */
    push %edx           /* esp = stos uzytkownika */
    pushf               /* eflags */
    push $0x1B          /* cs = ring3 kod | RPL3 */
    push %eax           /* eip = wejscie uzytkownika */
    iret
