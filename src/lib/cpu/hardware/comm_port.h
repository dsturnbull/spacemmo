#ifndef __src_lib_cpu_hardware_comm_port_h
#define __src_lib_cpu_hardware_comm_port_h

#include <stdint.h>

typedef struct comm_port_st {
    int a;
} comm_port_t;

comm_port_t * init_comm_port();

#endif


