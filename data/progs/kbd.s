%dw TICK

_main:
	; each keypress is pushed and _kbd_isr is called
	push _kbd_isr
	push KBD
	int

	ret

_kbd_isr:
	; echo the char
	push TTY
	int

	ret

