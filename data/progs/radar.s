%include "data/progs/util.s"

%define RADAR_STATUS    0x0

radar_status_ok db "radar ok"
radar_status_ok_len equ $-radar_status_ok

_main:
    push _radar_status_isr
    push IO_1_IN
    int

    push RADAR_STATUS
    push IO_1_OUT
    int

    ret

_radar_status_isr:
    ; should be 0, i.e., the radar is operating properly
    push _radar_status_print_ok
    swap
    jz
    ret

_radar_status_print_ok:
    push radar_status_ok
    push radar_status_ok_len
    push _tty_print_string
    call
    ret
