mmu_off equ 0
mmu_on  equ 1

_main:
    push byte mmu_on
    push _page_fault
    push MMU
    int

    ret

_page_fault:
    ret
