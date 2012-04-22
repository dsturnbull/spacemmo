%import	"s/util.s"

%dw	COUNTER

_main:
	push	_report
	push	0x30
	push	_loop
	call

	ret

_report:
	push	COUNTER
	push	COUNTER
	load
	push	0x1
	add
	store
	ret
