%dw	TEST1	0x1
%dw	TEST2	0x4
%dw	TEST3	0x4
%dw	TEST4

_main:
	push	TEST1
	push	0xff
	store

	push	TEST2
	push	0xff
	store

	push	TEST3
	push	0xff
	store

	push	TEST4
	push	0xff
	store

	ret
