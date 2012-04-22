%define	ENTRY_LEN	0x200
%define	MAX_FILES	1024

%dw	FILE_P
%dw	FILE_N
%dw	WRITE_P

%dw	DATA_PTR
%dw	DATA_CNT

_fs_init:
	ret

_fs_file_create:
	; position of the new file entry
	push	FILE_N
	load
	push	ENTRY_LEN
	mul

	; move disk to position
	push	DISK_SET
	int

	; copy name in
	push	_fs_write
	call

	; update FILE_N
	push	FILE_N
	push	FILE_N
	load
	push	0x1
	add
	store

	ret

_fs_write:
	swap

	; reset DATA_CNT
	push	DATA_CNT
	push	0x0
	store

	; save source ptr in memory
	push	DATA_PTR
	swap
	store

_fs_write_next:
	; check if done
	dup
	push	DATA_CNT
	load
	sub
	push	_fs_write_ret
	swap
	jz

	; TODO frag - check if the current block is full

	; get the char pointed to
	push	DATA_PTR
	load
	push	DATA_CNT
	load
	add
	load

	; write to position on disk
	push	DISK_WR
	int

	; inc pointer
	push	DATA_CNT
	push	0x1
	push	DATA_CNT
	load
	add
	store

	push	_fs_write_next
	jmp

_fs_write_ret:
	pop
	ret

_fs_read:
	swap

	; reset counter
	push	DATA_CNT
	push	0x0
	store

	; save destination ptr
	push	DATA_PTR
	swap
	store

_fs_read_next:
	; check if done
	dup
	push	DATA_CNT
	load
	swap
	sub
	push	_fs_read_ret
	swap
	jz

	; get the char
	push	_fs_read_store
	push	DISK_RD
	int

	; inc pointer
	push	DATA_CNT
	push	0x1
	push	DATA_CNT
	load
	add
	store

	push	_fs_read_next
	jmp

_fs_read_ret:
	pop
	ret

_fs_read_store:
	; append char to destination
	push	DATA_PTR
	load
	swap
	store

	; inc the pointer
	push	DATA_PTR
	push	DATA_PTR
	load
	push	DATA_CNT
	load
	add
	store

	ret

; _fs_file_fopen

_fs_file_open:
	; position of the new file entry
	push	ENTRY_LEN
	mul

	; set position
	push	DISK_SET
	int

	; copy to pointer on the stack
	push	ENTRY_LEN
	push	_fs_read
	call

	ret

