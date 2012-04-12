_read_line:
	dup
	dup

	push	KBD
	push	_read_key_next
	int

	dup
	load
	add
	load
	dup

	push	0xa
	sub
	push	_read_key_ret
	jz

	push	0x4
	sub
	push	_read_key_ret
	jz

	push	_read_line
	jmp

_read_key_next:
	; echo char
	dup
	push	OUT
	swap
	int

	; increment pointer
	swap
	dup
	dup
	load
	push	0x1
	add
	store

	; store char
	dup
	load
	add
	swap
	store
	ret

_read_key_ret:
	; decrement pointer
	dup
	load
	push	0x1
	sub
	store
	ret

_print_num:
	dup
	; divide num by 10
	push	0xa
	div

	; push remainder
	push	0x30
	add

	swap

	; loop until quo == 0
	dup
	push	_print_num_ret
	jz

	push	_print_num
	call

_print_num_ret:
	pop
	push	OUT
	swap
	int
	pop
	ret

