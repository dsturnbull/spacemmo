%include "data/progs/dos/var.s"
%include "data/progs/dos/tty.s"

_main:
    push _show_boot_msg
    call
    ret

_show_boot_msg:
    push boot_msg
    push word boot_msg_len
    push _tty_print_string
    call
    ret 
