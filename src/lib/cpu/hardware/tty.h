#ifndef __src_lib_cpu_hardware_tty_h
#define __src_lib_cpu_hardware_tty_h

#include <stdbool.h>

typedef struct tty_st {
    char *fn;
    int master, slave;
} tty_t;

tty_t * init_tty();
bool read_tty(tty_t *, char *);
void write_tty(tty_t *, char);

#endif

