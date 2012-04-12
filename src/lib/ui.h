#ifndef __src_lib_ui_h
#define __src_lib_ui_h

#include "src/lib/spacemmo.h"

typedef struct client_st client_t;
typedef struct gfx_st gfx_t;
typedef struct console_st console_t;
typedef struct input_st input_t;

typedef struct ui_st {
    client_t *client;
    gfx_t *gfx;
    console_t *console;
    input_t *input;
} ui_t;

ui_t * init_ui(client_t *);
void update_ui(ui_t *, double);
void shutdown_ui(ui_t *);

#endif

