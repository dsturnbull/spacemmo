_main:
    ; each keypress is pushed and _kbd_isr is called
    push _kbd_isr
    push KBD
    int

    ret

_kbd_isr:
    ; echo the char
    dup
    push TTY
    int

    push 0x0            ; NULL (CTRL-@)
    push _enable_debug
    je

    ret

_enable_debug:
    push 0x1
    push DBG
    int
    ret
