#ifndef __src_lib_cpu_hardware_port_h
#define __src_lib_cpu_hardware_port_h

#include <sys/types.h>
#include <stdbool.h>

typedef struct port_st port_t;

typedef struct port_st {
    int n;
    int r, w;
    void (*handler)(port_t *, char);
    void *hw;
} port_t;

port_t * init_port(int);
bool read_port(port_t *, char *);
void write_port(port_t *, char);
void write_client(port_t *, void *, size_t);

#endif

