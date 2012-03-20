#include <histedit.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include "src/lib/client.h"
#include "src/lib/entity.h"
#include "src/lib/ui.h"
#include "src/lib/ui/console.h"
#include "src/lib/ui/console.h"

console_t *
init_console(ui_t *ui, char *name)
{
    console_t *console = calloc(1, sizeof(console_t));
    console->ui = ui;

    asprintf(&console->hist_file, "%s/.spacemmo_%s_history", getenv("HOME"),
            name);
    asprintf(&console->prompt, "%s> ", name);

    console->el = el_init(name, stdin, stdout, stderr);
    el_set(console->el, EL_EDITOR, "emacs");
    el_set(console->el, EL_CLIENTDATA, (void *)console);
    el_set(console->el, EL_PROMPT, prompt);

    console->history = history_init();
    history(console->history, &console->ev, H_SETSIZE, 800);
    history(console->history, &console->ev, H_SETUNIQUE, 1);
    history(console->history, &console->ev, H_LOAD, console->hist_file);
    el_set(console->el, EL_HIST, history, console->history);

    console->t = tok_init(NULL);

    console->lua = lua_open();
    init_console_lua(console);

    return console;
}

void
init_console_lua(console_t *console)
{
    lua_State *L = console->lua;
    luaL_openlibs(console->lua);

    luaL_openlib(L, "client", client_lib, 0);

    lua_pushlightuserdata(L, (void *)console->ui->client->entity);
    lua_setglobal(L, "entity");
}

int client_lib_status(lua_State *L) {
    lua_getglobal(L, "entity");
    entity_t *e = lua_touserdata(L, 1);
    printf("pos: %f %f %f\n", e->pos.x, e->pos.y, e->pos.z);
    printf("vel: %f %f %f\n", e->vel.x, e->vel.y, e->vel.z);
    printf("acc: %f %f %f\n", e->acc.x, e->acc.y, e->acc.z);
    return 0;
}

char *
prompt(EditLine *el)
{
    console_t *console;
    el_get(el, EL_CLIENTDATA, &console);
    return console->prompt;
}

void
update_console(console_t *console, double dt)
{
}

void
process_input(console_t *console)
{
    client_t *client = console->ui->client;

    const char *buf;

    while (true) {
        int count = 0;

        buf = el_gets(console->el, &count);

        if (buf == NULL)
            break;

        char *line = strdup(buf);
        line = strsep(&line, "\n");
        history(console->history, &console->ev, H_ENTER, buf);

        if (luaL_dostring(console->lua, line) != 0) {
            fprintf(stderr, "%s\n", lua_tostring(console->lua, -1));
            lua_pop(console->lua, 1);
        }
    }

    console->ui->client->quit = true;
}

void
shutdown_console(console_t *console)
{
    history(console->history, &console->ev, H_SAVE, console->hist_file);
    history_end(console->history);
    tok_end(console->t);
    el_end(console->el);
    lua_close(console->lua);
    free(console);
}

