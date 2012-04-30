_main:
    push 0
    push DISK_SET
    int

    push byte 0xf
    push DISK_WR
    int

    push 0
    push DISK_SET
    int

    push DISK_RD
    int

    ret
