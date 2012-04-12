#ifndef __src_lib_ui_skybox_h
#define __src_lib_ui_skybox_h

#include <OpenGL/gl.h>

#include "src/lib/spacemmo.h"

typedef struct gfx_st gfx_t;

void init_skybox(char *);
void render_skybox(gfx_t *, GLuint);

#endif

