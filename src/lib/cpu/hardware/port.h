#ifndef __src_lib_cpu_hardware_port_h
#define __src_lib_cpu_hardware_port_h

#include <stdint.h>
#include <sys/types.h>
#include <stdbool.h>

typedef struct port_st port_t;

typedef struct port_st {
    int n;
    int r, w;
    void (*handler)(port_t *, uint8_t *, size_t);
    void *hw;
    uint8_t *dma;
    size_t dma_len;
} port_t;

port_t * init_port(int, uint8_t *, size_t);
size_t read_port(port_t *, uint8_t *, size_t);
void write_port(port_t *, uint8_t *, size_t);
void write_client(port_t *, void *, size_t);

#endif

