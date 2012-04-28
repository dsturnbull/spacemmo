_main:
    push _other
    call
    ret

_other:
    push _second
    call
    ret

_second:
    ret
