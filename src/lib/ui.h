#ifndef __src_lib_ui_h
#define __src_lib_ui_h

#include "src/lib/spacemmo.h"

struct ui_st {
    client_t *client;
    gfx_t *gfx;
    console_t *console;
    input_t *input;
};

ui_t * init_ui(client_t *);
void update_ui(ui_t *, double);
void shutdown_ui(ui_t *);

#endif

