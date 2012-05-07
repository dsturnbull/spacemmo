%include "data/progs/dos/sys/var.s"
%include "data/progs/dos/sys/mmu.s"
%include "data/progs/dos/sys/tty.s"
%include "data/progs/dos/sys/shell.s"

run_queue equ 0x10000
dq run_queue_len 0
dw process_id 0

_main:
    ;push _show_boot_msg
    ;call

    ;push _init_page_table
    ;call

    ;push _shell
    ;push _run
    ;call

    ;push _process_run_queue
    ;call

    ret

_show_boot_msg:
    push boot_msg
    push word boot_msg_len
    push _tty_print_string
    call
    ret 

; run the top of the queue for a while, then push it down
_process_run_queue:
    push run_queue_len
    load

    ; nothing to run
    dup
    push _process_run_queue_ret
    jz

    ; get the first
    push run_queue
    load
    pop

_process_run_queue_ret:
    pop
    ret

; appends a program entry point to the run_queue with a new pid
_run:
    ; move to the end of the queue
    push run_queue
    push run_queue_len
    load
    push 8
    mul
    add

    ; store the entry point
    swap
    store

    ; get a pid
    ; allocate page(s) for code
    ; copy the code

    ; update run_queue_len
    push run_queue_len
    dup
    load
    push 1
    add
    store

    ret
