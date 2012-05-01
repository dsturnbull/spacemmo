%include "data/progs/tests/other.s"
%include "data/progs/tests/more.s"

_main:
    push _other
    call

    push _more
    call

    pop byte
    pop byte

    ret
