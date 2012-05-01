db timer 0

_main:
    push word 500
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

    ret
