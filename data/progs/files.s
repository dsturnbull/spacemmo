%import	"s/fs.s"
%import	"s/util.s"

%data	FN1	"test1"
%data	FN1_LEN	@-FN1
%data	FN2	"test2"
%data	FN2_LEN	@-FN2
%data	DATA	"hello, my name is sven."
%data	DATA_LEN	@-DATA
%dw	ENTRY	0x200

_main:
	push	_fs_init
	call

	;push	FN1
	;push	FN1_LEN
	;push	_fs_file_create
	;call

	;push	FN2
	;push	FN2_LEN
	;push	_fs_file_create
	;call

	push	ENTRY
	push	0x0		; fd
	push	_fs_file_open
	call

	;push	DATA
	;push	DATA_LEN
	;push	0x0
	;push	_fs_write

	ret

