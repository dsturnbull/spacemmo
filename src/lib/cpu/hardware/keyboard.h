#ifndef __src_lib_cpu_hardware_keyboard_h
#define __src_lib_cpu_hardware_keyboard_h

#include <stdint.h>

typedef struct input_st input_t;

typedef struct keyboard_st {
    input_t *input;
} keyboard_t;

keyboard_t * init_keyboard();
void keyboard_state(keyboard_t *, uint8_t[]);

#endif

