#ifndef __src_lib_cpu_hardware_disk_h
#define __src_lib_cpu_hardware_disk_h

#include <sys/types.h>
#include <stdint.h>

#define CPU_DISK_SIZE   1024 * 1024 * 10

typedef struct port_st port_t;

typedef struct disk_st {
    port_t *port;
    char *fn;
    int fd;
    uint8_t *map;
    uint8_t *curpos;
    size_t pos;
    size_t sz;
} disk_t;

typedef enum disk_cmd_e {
    DISK_STATUS,
    DISK_SET,
    DISK_RD,
    DISK_WR,
} disk_cmd_t;

disk_t * init_disk(port_t *, char *);
void free_disk(disk_t *);
void disk_handler(port_t *, uint8_t *, size_t);
void set_disk_position(disk_t *, size_t);
size_t read_disk(disk_t *, uint8_t **, size_t len);
void write_disk(disk_t *, uint8_t *, size_t);

#endif

