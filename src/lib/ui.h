#ifndef __src_lib_ui_h
#define __src_lib_ui_h

#include "src/lib/spacemmo.h"

struct ui_st {
    client_t *client;
    console_t *console;
    gfx_t *gfx;
};

void init_ui(ui_t **);
void update_ui(ui_t *, double);
void shutdown_ui(ui_t *);

#endif

