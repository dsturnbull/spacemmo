#ifndef __src_lib_ui_gfx_h
#define __src_lib_ui_gfx_h

#include <stdbool.h>

#include <agar/core.h>
#include <agar/gui.h>

#include "src/lib/spacemmo.h"

#define STAR_GRID_SIZE 64

struct gfx_st {
    int w, h;
    ui_t *ui;
    AG_Driver *drv;
    AG_Menu *menu;
    vec3f *eye;
    vec3f *tgt;

    vec3f *star_grid[STAR_GRID_SIZE][STAR_GRID_SIZE];
    float star_pos;
};

gfx_t * init_gfx(ui_t *);

void init_gfx_ui(gfx_t *);
void init_gfx_menu(gfx_t *);

void init_gfx_ship_ui(gfx_t *, entity_t *);
void init_gfx_ship(gfx_t *, entity_t *);
void init_gfx_ship_status_window(gfx_t *, entity_t *);

void update_gfx(gfx_t *, double);
void render_stars(gfx_t *, double);
void shutdown_gfx(gfx_t *);

int handle_event(gfx_t *, AG_DriverEvent *);
void handle_resize(gfx_t *, AG_DriverEvent *);
void handle_net_connect_button(AG_Event *);
void handle_system_quit_button(AG_Event *);

#endif

