disk_status equ 0x0
disk_set    equ 0x1
disk_rd     equ 0x2
disk_wr     equ 0x3

_main:
    push byte disk_status
    push byte 1
    push _disk_status_isr
    push IO_0
    int
    ret

_disk_status_isr:
    push byte disk_set
    push byte 0
    push byte 2
    push _disk_set_isr
    push IO_0
    int

    ret

_disk_set_isr:
    push byte disk_wr
    push byte 0xc
    push byte 0xc
    push byte 3
    push _disk_wr_isr
    push IO_0
    int

    ret

_disk_wr_isr:
    push byte disk_set
    push byte 0
    push byte 2
    push _disk_set_wr_isr
    push IO_0
    int

    ret

_disk_set_wr_isr:
    push byte disk_rd
    push byte 20
    push byte 2
    push _disk_rd_isr
    push IO_0
    int

    ret

_disk_rd_isr:
    push IO_0_OUT
    load byte
    pop byte

    push IO_0_OUT
    push 1
    add
    load byte
    pop byte

    push IO_0_OUT
    push 2
    add
    load byte
    pop byte

    ret
