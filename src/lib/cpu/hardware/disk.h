#ifndef __src_lib_cpu_hardware_disk_h
#define __src_lib_cpu_hardware_disk_h

#include <sys/types.h>
#include <stdint.h>

#define CPU_DISK_SIZE   1024 * 1024 * 10

typedef struct disk_st {
    char *fn;
    int fd;
    uint8_t *map;
    uint8_t *curpos;
    size_t pos;
} disk_t;

disk_t * init_disk(char *);
void set_disk_position(disk_t *, size_t);
uint8_t read_disk(disk_t *);
void write_disk(disk_t *, uint8_t);

#endif

