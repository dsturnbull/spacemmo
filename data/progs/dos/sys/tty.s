_tty_print_string:
    swap word
    pop
    pop word

    ;pop word
    ;pop

    ret

_tty_print_char:
    push TTY
    int
    ret
