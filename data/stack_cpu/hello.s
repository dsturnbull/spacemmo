%define	KBD_BUF	0x0	; 256 bytes
%define	KBD_PTR	0x40

%import	"data/stack_cpu/util.s"

_main:
	push	KBD_BUF
	push	_kbd_isr
	push	KBD
	int

	push	_main
	jmp

