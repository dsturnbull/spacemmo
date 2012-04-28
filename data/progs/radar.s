%include "data/progs/util.s"

%define RADAR_STATUS    0x0
%define RADAR_SCAN      0x1

radar_status_ok db "radar ok"
radar_status_ok_len equ $-radar_status_ok
radar_status_damaged db "radar damaged"
radar_status_damaged_len equ $-radar_status_damaged
radar_entities db "entities"
radar_entities_len equ $-radar_entities

dw scan_results_p
dw scan_results 4

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
    ; print that we're ok
    push radar_status_ok
    push radar_status_ok_len
    push _tty_print_string
    call

    push 0xa
    push TTY
    int

    ; register kbd handler
    push _kbd_isr
    push KBD
    int

    ; register scan handler
    push _radar_scan_isr
    push IO_1_IN
    int

    ret

_kbd_isr:
    ; ignore key
    pop

    ; reset scan_results pointer
    push scan_results_p
    push 0x0
    store

    ; send a scan request
    push RADAR_SCAN
    push IO_1_OUT
    int

    ret

; number of entities
; entities
_radar_scan_isr:
    ; store the 4 bytes of the number
    dup
    push scan_results
    push scan_results_p
    load
    add
    swap
    store

    ; increment pointer
    push scan_results_p
    dup
    load
    push 0x4
    add
    store

    push scan_results_p
    load
    push 0x10
    push _radar_scan_print_num
    je

    ret

_radar_scan_print_num:
    push scan_results
    push _tty_print_number
    call

    push 0x20
    push TTY
    int

    push radar_entities
    push radar_entities_len
    push _tty_print_string
    call

    push 0xa
    push TTY
    int

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

