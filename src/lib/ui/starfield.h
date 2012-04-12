#ifndef __src_lib_ui_starfield_h
#define __src_lib_ui_starfield_h

#include "src/lib/spacemmo.h"

#define STAR_GRID_SIZE 64

typedef struct gfx_st gfx_t;

struct {
    vec3f *star_grid[STAR_GRID_SIZE][STAR_GRID_SIZE];
    float star_pos;
} starfield;

void init_starfield(gfx_t *);
void render_starfield(gfx_t *, double);

#endif

