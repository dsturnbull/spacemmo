%include "data/common.s"

message db 'hello!!'
msglen equ $-message

_main:
	mov a, message

LDI	0x200
STX	0x101

LDI	0x8
STX	0x100

_loop:
	LDP	0x101
	OUT

	INC	0x101
	DEC	0x100

	JNZ	_loop

