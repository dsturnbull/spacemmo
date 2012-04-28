db b 0xf
dw w 0xffff
dd d 0xffffffff
dq q 0xffffffffffffffff

e equ 0x84

;db a 'a'
;db sr 'hello'

_main:
    push byte 0x3
    push word 0x0101
    push dword 0x01010101
    push qword 0x0101010101010101
    push 37

    push byte b
    push word w
    push dword d 
    push qword q
    push q

    push byte 1
    pop byte

    pop
    pop qword
    pop dword
    pop word
    pop byte

    pop
    pop qword
    pop dword
    pop word
    pop byte

    ret

; hi
