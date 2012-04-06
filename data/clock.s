_loop:
	ld0	CPU_CLOCK
	st0	0x100

_print:
	div	0xa
	ldi	r1
	OUT
	ldi	0xa
	OUT

