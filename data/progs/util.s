%dw PRNT_P
%dw PRNT_L
%dw LOOP_T
%dw PRINT_NUM_POWER

; int len, char *str
_tty_print_string:
    push PRNT_L
    swap
    store

    push PRNT_P
    swap
    store

_tty_print_string_next:
    push PRNT_P
    load
    load
    push TTY
    int

    push PRNT_L
    dup
    load
    push 0x1
    sub
    store

    push PRNT_P
    dup
    load
    push 0x4
    add
    store

    push _tty_print_string_next
    push PRNT_L
    load
    jnz

_tty_print_string_ret:
    ret

_tty_print_number:
    push PRINT_NUM_POWER
    push 0x64
    store

_tty_print_number_digit:
    dup
    push _tty_print_number_ret
    swap
    jz

    push PRINT_NUM_POWER
    load
    swap
    div

    dup
    push 0x30
    add

    push TTY
    int

    push PRINT_NUM_POWER
    load
    push 0xa
    swap
    div
    push PRINT_NUM_POWER
    swap
    store

    pop
    pop
    push _tty_print_number_digit
    jmp

_tty_print_number_ret:
    pop
    ret

