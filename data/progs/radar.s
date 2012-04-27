%include "data/progs/util.s"

%define RADAR_STATUS    0x0
%define RADAR_SCAN      0x1

radar_status_ok db "radar ok"
radar_status_ok_len equ $-radar_status_ok
radar_status_damaged db "radar damaged"
radar_status_damaged_len equ $-radar_status_damaged

_main:
    ; register status handler
    push _radar_status_isr
    push IO_1_IN
    int

    ; get radar status
    push RADAR_STATUS
    push IO_1_OUT
    int

    ret

; receives 1 or 0
_radar_status_isr:
    ; should be 0, i.e., the radar is operating properly
    push _radar_status_ok
    swap
    jz

    ; otherwise print radar damaged msg
    push _radar_status_print_damaged
    call
    ret

_radar_status_ok:
    ; register kbd handler
    push _kbd_isr
    push KBD
    int

    ; print that we're ok
    push radar_status_ok
    push radar_status_ok_len
    push _tty_print_string
    call

    push 0xa
    push TTY
    int

    ; register scan handler
    push _radar_scan_isr
    push IO_1_IN
    int

    ret

_kbd_isr:
    ; ignore key
    pop

    ; send a scan request
    push RADAR_SCAN
    push IO_1_OUT
    int

    ret

; number of entities
; entities
_radar_scan_isr:
    push 0xcafebabe
    pop
    pop
    ret

_radar_status_print_damaged:
    push radar_status_damaged
    push radar_status_damaged_len
    push _tty_print_string
    call

    push 0xa
    push TTY
    int

    ret

