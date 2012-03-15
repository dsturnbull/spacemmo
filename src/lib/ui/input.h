#ifndef __src_lib_input_h
#define __src_lib_input_h

#include <agar/core.h>
#include <agar/gui.h>

#include "src/lib/spacemmo.h"

struct input_st {
    ui_t *ui;
};

input_t * init_input(ui_t *);
int handle_mouse(input_t *, AG_DriverEvent *);
int handle_keypress(input_t *, AG_DriverEvent *);

#endif
