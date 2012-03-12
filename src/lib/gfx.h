#ifndef __src_lib_gfx_h
#define __src_lib_gfx_h

#include "src/lib/spacemmo.h"

struct gfx_st {
    bool quit;
};

void init_gfx(gfx_t **);
void update_gfx(gfx_t *, double);
void handle_input(gfx_t *);

#endif

