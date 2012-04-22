%define	TICK	0x30

_main:
	; run queue
	push	0x10000	; us
	push	_clk_isr
	push	CLK
	int

	ret

_clk_isr:
	; increment a counter, just for fun
	push	TICK
	push	TICK
	load
	push	0x1
	sub
	store

	ret
