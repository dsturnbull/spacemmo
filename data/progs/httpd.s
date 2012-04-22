%dw	INPUT	0x400
%dw	FD

%data	STATUS	"HTTP/1.0 200 OK"
%data	STATUS_LEN	@-STATUS
%data	HDR	"Content-Length: "
%data	HDR_LEN	@-HDR
%data	PAGE	"<html><body><p>This webserver is running on an emulated cpu.</p><p>It's pretty basic. It reads input until it sees \r\n\r\n, ignores actual request and/or headers, and spits out this constant string.</p><p>The hardware device the cpu sees is called 'sock' and will be reused for all network communication, obviously, but also will serve as a link to peripheral hardware - engines, missiles etc.<p>This page takes about 5.5ms to render.</p></body></html>"
%data	PAGE_LEN	@-PAGE

%import	"s/util.s"

_main:
	; handle connections
	push	_sock_read
	push	_sock1_isr
	push	SOCK
	int

	; need this or the socket shit won't work
	push	_kbd_isr
	push	KBD
	int

	ret

_sock1_isr:
	; keep the socket around
	push	FD
	swap
	store

	; reset INPUT
	push	INPUT
	push	0x0
	store

	ret

_sock_read:
	; increment char *
	push	INPUT
	push	INPUT
	load
	push	0x1
	add
	store

	; store char at *
	push	INPUT
	push	INPUT
	load
	add	; INPUT + offset
	swap
	store

	; check for \r\n\r\n
	push	_sock_read_continue

	push	INPUT
	push	INPUT
	load
	add	; INPUT + offset
	load

	push	0xa
	sub
	jnz

	push	_sock_read_continue

	push	INPUT
	push	INPUT
	load
	add	; INPUT + offset
	push	0x1
	swap
	sub	; -1
	load

	push	0xd
	sub
	jnz

	push	_sock_read_continue

	push	INPUT
	push	INPUT
	load
	add	; INPUT + offset
	push	0x2
	swap
	sub	; -2
	load

	push	0xa
	sub
	jnz

	push	_sock_read_continue

	push	INPUT
	push	INPUT
	load
	add	; INPUT + offset
	push	0x3
	swap
	sub	; -3
	load

	push	0xd
	sub
	jnz

	push	_handle_request
	call

	ret

_sock_read_continue:
	ret

_handle_request:
	; assume standard get, ignore headers and actual request
	; just return HTTP 200 OK
	push	STATUS
	push	STATUS_LEN
	push	_sock_print_string
	call

	push	_crlf
	call

	push	HDR
	push	HDR_LEN
	push	_sock_print_string
	call

	push	PAGE_LEN
	; add \r\n
	push	0x4
	add
	push	_sock_print_number
	call

	push	_crlf
	dup
	call
	call

	push	PAGE
	push	PAGE_LEN
	push	_sock_print_string
	call

	push	_crlf
	dup
	call
	call

	ret

_kbd_isr:
	; echo the char
	push	TTY
	int

	ret

_crlf:
	push	0xd
	push	SOCK_OUT
	int

	push	0xa
	push	SOCK_OUT
	int

	ret
