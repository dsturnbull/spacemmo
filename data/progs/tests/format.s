disk_status equ 0x0
disk_set    equ 0x1
disk_rd     equ 0x2
disk_wr     equ 0x3
disk_sz     equ 0xa00000

db disk_pos 0

_main:
    push disk_pos
    dup
    load dword

    push dword 256
    add dword

    store dword

    push byte disk_wr
    push byte 1
    push _disk_wr_isr
    push IO_0
    int

    push disk_pos
    load dword
    push dword disk_sz
    push _ret
    je dword

_disk_wr_isr:
    push _main
    jmp

_ret:
    ret

