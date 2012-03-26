#ifndef __src_lib_ui_input_h
#define __src_lib_ui_input_h

#include <stdbool.h>

#include <agar/core.h>
#include <agar/gui.h>

#include "src/lib/spacemmo.h"

struct input_st {
    ui_t *ui;
    bool keys[255];
    bool quit;
};

input_t * init_input(ui_t *);
int handle_mouse(input_t *, AG_DriverEvent *);
int handle_keypress(input_t *, AG_DriverEvent *);

#endif

