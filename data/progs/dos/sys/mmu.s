mmu_init    equ 0
mmu_set     equ 1

page_table equ 0x20000

_init_page_table:
    push page_table
    push _page_fault
    push byte mmu_init
    push MMU
    int
    ret

_set_mmu:
    push byte mmu_set
    push MMU
    int
    ret

_page_fault:
    ret

