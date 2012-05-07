#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "src/lib/cpu/hardware/mmu.h"

mmu_t *
init_mmu()
{
    mmu_t *mmu = calloc(1, sizeof(*mmu));
    return mmu;
}

void
mmu_int(mmu_t *mmu, uint8_t **data)
{
    uint64_t page_fault;
    uint64_t page_table;
    uint16_t process;
    *data -= 1;
    mmu_cmd_t cmd = *(uint8_t *)(*data);

    switch (cmd) {
        case MMU_INIT:
            memcpy(&page_fault, *data -= 8, 8);
            memcpy(&page_table, *data -= 8, 8);
            printf("mmu tbl: %016llx isr: %016llx\n", page_table, page_fault);
            break;

        case MMU_SET:
            *data -= 2;
            memcpy(&process, *data, 2);
            printf("mmu set %04x\n", process);
            // TODO the kernel should do this for the process
            // mmu->current_process = mmu->processes[process];
            break;
    }
}

