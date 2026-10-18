.section .text
.extern wyjatek_obsluz

.macro exc_noerr num
.global exc_\num
exc_\num:
    push $0
    push $\num
    call wyjatek_obsluz
    add $8, %esp
    iret
.endm

.macro exc_err num
.global exc_\num
exc_\num:
    push $\num
    call wyjatek_obsluz
    add $4, %esp
    add $4, %esp
    iret
.endm

exc_noerr 0
exc_noerr 1
exc_noerr 3
exc_noerr 4
exc_noerr 5
exc_noerr 6
exc_noerr 7
exc_noerr 9
exc_noerr 15
exc_noerr 16
exc_noerr 18
exc_noerr 19

exc_err 8
exc_err 10
exc_err 11
exc_err 12
exc_err 13
exc_err 14
