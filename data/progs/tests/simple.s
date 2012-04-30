_main:
    push _kbd_isr
    push KBD
    int

    push word 1000
    push _clk_isr
    push CLK
    int
    ret

_kbd_isr:
    push TTY
    int
    ret

_clk_isr:
    ret
