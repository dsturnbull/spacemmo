%import "data/progs/fs.s"
%import "data/progs/util.s"

%data menu_str "[n]ew [l]ist"
%data menu_str_len @-menu_str

%data new_file "new file name:"
%data new_file_len @-new_file

;%data test  "\033[36m"

_main:
	; menu keys
	push _kbd_menu_isr
	push KBD
	int

	ret

_kbd_menu_isr:
	; n  - new file
	dup
	push 'n'
	push _new_file_option
	je

	; l  - list files
	pop

	; else  - menu
	push _menu
	call

	push 0xd
	push TTY
	int

	ret

_new_file_option:
	push new_file
	push new_file_len
	push _tty_print_string
	call

	pop
	ret

_menu:
	push menu_str
	push menu_str_len
	push _tty_print_string
	call
	ret
