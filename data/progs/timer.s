%dw TICK

_main:
    ; run queue
    push    0x20000    ; us
    push    _clk_isr
    push    CLK
    int

    ret

_clk_isr:
    ; increment a counter, just for fun
    push    TICK
    dup
    load
    push    0x1
    add
    store

    ret
