_main:
	push _io_handler
	push IO_0_IN
	int

	ret

_io_handler:
	push IO_0_OUT
	int
	ret
