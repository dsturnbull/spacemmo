; loop 8 times
LDI	0x8
STX	0x100

_loop:
	; print hi
	LDI	0x68
	OUT
	LDI	0x69
	OUT
	LDI	0xa
	OUT

	DEC	0x100
	JNZ	_loop

