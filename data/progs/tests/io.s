disk_status equ 0x0
disk_set    equ 0x1
disk_rd     equ 0x2
disk_wr     equ 0x3
disk_sz     equ 0xa00000

db disk_pos 0
dw timer 0

_main:
    ;push word 100
    ;push _clk_isr
    ;push CLK
    ;int

    push byte disk_status
    push byte 1
    push IO_0
    int

    push byte disk_set
    push disk_pos
    load byte
    push byte 2
    push IO_0
    int

_loop:
    push disk_pos
    dup
    load dword

    push dword 256
    add dword

    store dword

    push byte disk_wr
    push byte 1
    push IO_0
    int

    push disk_pos
    load dword
    push dword disk_sz
    push _ret
    je dword

    push _loop
    jmp

_ret:
    ret

_clk_isr:
    ret

