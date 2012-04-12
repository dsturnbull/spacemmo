%define	INPUT	0x0

%import	"data/stack_cpu/util.s"

_main:
	push	_prompt
	call

	; reset len to 0
	push	INPUT
	push	0x0
	store

	push	INPUT
	push	_read_line
	call

	push	INPUT
	load
	push	INPUT
	add
	load
	push	0x4
	sub
	push	_ret
	jz

	push	_ok
	call

	push	_main
	jmp

_ret:
	ret

_prompt:
	push	OUT
	push	0x3e	; '>'
	int

	push	OUT
	push	0x20	; ' '
	int

	ret

_ok:
	push	OUT
	push	0x6f	; 'o'
	int

	push	OUT
	push	0x6b	; 'k'
	int

	push	OUT
	push	0x0a	; '\n'
	int

	ret

