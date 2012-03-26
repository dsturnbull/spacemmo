#ifndef __src_lib_computer_h
#define __src_lib_computer_h

#include "src/lib/spacemmo.h"

struct computer_st {
    int qemu_pid;
};

computer_t * init_computer();
void update_computer(computer_t *, double);

#endif

