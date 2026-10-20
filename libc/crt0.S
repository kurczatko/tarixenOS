.section .text
.code32
.global _start
.extern main
.stack:
    .space 4096
.stack_top:
_start:
    movl $.stack_top, %esp
    andl $-16, %esp
    call main
    pushl %eax
    movl $21, %eax
    int $0x80
    hlt
