%include "data/progs/util.s"

%define THRUSTER_STATUS 0x0
%define THRUSTER_INC_X  0x1
%define THRUSTER_DEC_X  0x2
%define THRUSTER_INC_Y  0x3
%define THRUSTER_DEC_Y  0x4
%define THRUSTER_INC_Z  0x5
%define THRUSTER_DEC_Z  0x6

%dw balls 18
%dw status 12
%dw status_p

%data thrusters_set "thrusters set"
%data thrusters_set_len $-thrusters_set
%data help "w - fwd, s - back, a - left, d - right, q - up, z - down, sp - info"
%data help_len $-help

_main:
    push _kbd_isr
    push KBD
    int

    push status
    pop
    push status
    pop

    ret

_help:
    push help
    push help_len
    push _tty_print_string
    call

    push 0xd
    push TTY
    int

    push _prompt
    call

    ret

_prompt:
    push '>'
    push TTY
    int

    push 0x20
    push TTY
    int

    ret

_set:
    ;push thrusters_set
    ;push thrusters_set_len
    ;push _tty_print_string
    ;call

    ;push 0xd
    ;push TTY
    ;int

    ;push _prompt
    ;call

    ret

_kbd_isr:
    dup
    push TTY
    int

    push 0xd
    push TTY
    int

    push _status
    call

    dup
    push 'w'
    push _increase_z_thrust
    je

    dup
    push 's'
    push _decrease_z_thrust
    je

    dup
    push 'a'
    push _decrease_x_thrust
    je

    dup
    push 'd'
    push _increase_x_thrust
    je

    dup
    push 'q'
    push _increase_y_thrust
    je

    dup
    push 'z'
    push _decrease_y_thrust
    je

    push _help
    call

    pop
    ret

_increase_x_thrust:
    pop
    push THRUSTER_INC_X
    push IO_0_OUT
    int
    push _set
    call
    ret

_decrease_x_thrust:
    pop
    push THRUSTER_DEC_X
    push IO_0_OUT
    int
    push _set
    call
    ret

_increase_y_thrust:
    pop
    push THRUSTER_INC_Y
    push IO_0_OUT
    int
    push _set
    call
    ret

_decrease_y_thrust:
    pop
    push THRUSTER_DEC_Y
    push IO_0_OUT
    int
    push _set
    call
    ret

_increase_z_thrust:
    pop
    push THRUSTER_INC_Z
    push IO_0_OUT
    int
    push _set
    call
    ret

_decrease_z_thrust:
    pop
    push THRUSTER_DEC_Z
    push IO_0_OUT
    int
    push _set
    call
    ret

_status:
    push status_p
    push 0x0
    store

    push _status_isr
    push IO_0_IN
    int

    push THRUSTER_STATUS
    push IO_0_OUT
    int

    ret

_status_isr:
    push status_p
    load
    push status
    add
    swap
    store

    push status_p
    push status_p
    load
    push 0x1
    add
    store

    ;push _print_status
    ;push 0x4
    ;push status_p
    ;load
    ;div
    ;swap
    ;pop
    ;jz

    push status_p
    load
    push 0xc
    push _status_ret
    je

    ret

_print_status:
    push status_p
    load
    push 0x4
    sub
    push status
    add
    load
    push _tty_print_number
    call
 
    push 0x20
    push TTY
    int

    push status_p
    load
    push 0xc
    push _status_ret
    je

    ret

_status_ret:
    ;push 0xd
    ;push TTY
    ;int

    ;push _prompt
    ;call

    ret
