disk_status equ 0x0
disk_set    equ 0x1
disk_rd     equ 0x2
disk_wr     equ 0x3
disk_sz     equ 0x100000

db disk_pos 0
db timer 0

_main:
    push byte disk_status
    push byte 1
    push _disk_status_isr
    push IO_0
    int
    ret

_disk_status_isr:
    push word 10
    push _loop
    push CLK
    int
    ret

_loop:
    push timer
    dup
    load dword

    push dword 1
    add dword

    store dword

    ;push timer
    ;swap

    push IO_0_BUF
    push timer
    load dword
    store dword

    push byte disk_wr
    push byte 1
    push _disk_wr_isr
    push IO_0
    int

    ret

_disk_wr_isr:
    ret

