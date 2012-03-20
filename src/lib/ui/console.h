#ifndef __src_lib_ui_console_h
#define __src_lib_ui_console_h

#include <histedit.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "src/lib/spacemmo.h"

struct console_st {
    ui_t *ui;
    char *prompt;
    History *history;
    HistEvent ev;
    char *hist_file;
    EditLine *el;
    Tokenizer *t;
    lua_State *lua;
};

int client_lib___tostring(lua_State *);
int client_lib_status(lua_State *);

static const struct luaL_reg client_lib[] = {
    {"status", client_lib_status},
    {NULL, NULL}
};

console_t * init_console(ui_t *, char *);
void init_console_lua(console_t *);
char * prompt(EditLine *);
void update_console(console_t *, double);
void process_input(console_t *);
void shutdown_console(console_t *);

#endif

