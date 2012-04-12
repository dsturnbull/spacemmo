#ifndef __src_lib_cpu_hardware_tty_h
#define __src_lib_cpu_hardware_tty_h

#define SCREEN_WIDTH    80
#define SCREEN_HEIGHT   25

#include <stdio.h>
#include <stdint.h>

typedef struct tty_st {
    uint8_t display[SCREEN_WIDTH * SCREEN_HEIGHT];
    uint8_t *cursor;
} tty_t;

tty_t * init_tty();
void ttyp(tty_t *, uint8_t);
void print_screen(tty_t *);
void write_tty(tty_t *, FILE *);

#endif

