_kbd_isr:
	; reset ptr
	push	KBD_PTR
	push	KBD_BUF
	store

_kbd_isr_echo_char:
	; loop 64 times
	push	0x40
	push	KBD_PTR
	load
	sub
	push	_kbd_isr_ret
	jz

	; char position = the char
	push	KBD_PTR
	load

	; inside this is 4 chars
	dup
	load

	push	_kbd_isr_decode_char_buf
	call

	; inc ptr
	push	KBD_PTR
	dup
	load
	push	0x1
	add
	store

	; loop
	pop
	push	_kbd_isr_echo_char
	jmp

_kbd_isr_decode_char_buf:
	; number on stack is [a:1,b:1,c:1,d:1]

	; 1
	dup
	push	0xff000000
	and
	push	0x0
	push	_kbd_isr_char_set
	call

	; 2
	dup
	push	0x00ff0000
	and
	push	0x1
	push	_kbd_isr_char_set
	call

	; 3
	dup
	push	0x0000ff00
	and
	push	0x2
	push	_kbd_isr_char_set
	call

	; 4
	dup
	push	0x000000ff
	and
	push	0x3
	push	_kbd_isr_char_set
	call

	pop
	ret

_kbd_isr_char_set:
	; do not print if 0
	swap
	push	_kbd_isr_char_ret
	jz

	; echo char
	dup
	push	KBD_PTR
	load
	push	0x4
	mul
	add
	push	OUT
	int

_kbd_isr_char_ret:
	pop
	ret

_kbd_isr_ret:
	ret

