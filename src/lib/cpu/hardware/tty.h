#ifndef __src_lib_cpu_hardware_tty_h
#define __src_lib_cpu_hardware_tty_h

#include <stdbool.h>

typedef struct tty_st {
    char *fn;
    int master, slave;
} tty_t;

tty_t * init_tty();
bool connect_tty(tty_t *);
bool read_tty(tty_t *, char *);
void write_tty(tty_t *, char);
void wait_tty_slave(tty_t *);

#endif

