.section .text
.globl usb_hal_inb
.globl usb_hal_outb
.globl usb_hal_inw
.globl usb_hal_outw
.globl usb_hal_inl
.globl usb_hal_outl

usb_hal_inb:
    movw 4(%esp), %dx
    inb %dx, %al
    ret

usb_hal_outb:
    movw 4(%esp), %dx
    movb 8(%esp), %al
    outb %al, %dx
    ret

usb_hal_inw:
    movw 4(%esp), %dx
    inw %dx, %ax
    ret

usb_hal_outw:
    movw 4(%esp), %dx
    movw 8(%esp), %ax
    outw %ax, %dx
    ret

usb_hal_inl:
    movw 4(%esp), %dx
    inl %dx, %eax
    ret

usb_hal_outl:
    movw 4(%esp), %dx
    movl 8(%esp), %eax
    outl %eax, %dx
    ret
