db b 0xf
dw w 0xffff
dd d 0xffffffff
dq q 0xffffffffffffffff

;e equ 0x84

db a 'a'
db sr "hello"

_main:
    push 1
    push byte 0x10
    push word 0x0101
    push dword 0x01010101
    push qword 0x0101010101010101

    push b
    load byte
    push w
    load word
    push d 
    load dword
    push q
    load qword

    push byte 1
    push a
    pop
    pop byte

    ;push e
    ;pop

    push sr
    pop

    pop qword
    pop dword
    pop word
    pop byte

    pop qword
    pop dword
    pop word
    pop byte
    pop

    ret

; hi
