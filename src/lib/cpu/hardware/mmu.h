#ifndef __src_lib_cpu_hardware_mmu_h
#define __src_lib_cpu_hardware_mmu_h

#include <stdint.h>

typedef struct page_st {
    uint64_t *real;
} page_t;

typedef struct process_st {
    page_t *pages;
} process_t;

typedef struct mmu_st {
    process_t *processes;
} mmu_t;

typedef enum mmu_cmd_e {
    MMU_INIT,
    MMU_SET,
} mmu_cmd_t;

mmu_t * init_mmu();
void mmu_int(mmu_t *, uint8_t **);

#endif

