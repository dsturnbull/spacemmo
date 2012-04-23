%define THRUSTER_STATUS	0x0
%define THRUSTER_INC_X	0x1
%define THRUSTER_DEC_X	0x2
%define THRUSTER_INC_Y	0x3
%define THRUSTER_DEC_Y	0x4
%define THRUSTER_INC_Z	0x5
%define THRUSTER_DEC_Z	0x6

%dw STATUS 12
%dw STATUS_P

_main:
	push _kbd_isr
	push KBD
	int

	ret

_kbd_isr:
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

	dup
	push 0x20
	push _status
	je

	pop
	ret

_increase_x_thrust:
	pop
	push THRUSTER_INC_X
	push IO_0_OUT
	int
	push _status
	call
	ret

_decrease_x_thrust:
	pop
	push THRUSTER_DEC_X
	push IO_0_OUT
	int
	push _status
	call
	ret

_increase_y_thrust:
	pop
	push THRUSTER_INC_Y
	push IO_0_OUT
	int
	push _status
	call
	ret

_decrease_y_thrust:
	pop
	push THRUSTER_DEC_Y
	push IO_0_OUT
	int
	push _status
	call
	ret

_increase_z_thrust:
	pop
	push THRUSTER_INC_Z
	push IO_0_OUT
	int
	push _status
	call
	ret

_decrease_z_thrust:
	pop
	push THRUSTER_DEC_Z
	push IO_0_OUT
	int
	push _status
	call
	ret

_status:
	pop

	push STATUS_P
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
	push STATUS_P
	load
	push STATUS
	add
	swap
	store

	push STATUS_P
	push STATUS_P
	load
	push 0x1
	add
	store

	push STATUS_P
	load
	push 0xc
	push _print_status
	je

	ret

_print_status:
	ret
