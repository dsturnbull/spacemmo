#include <histedit.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ruby/ruby.h>

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

    init_ruby_console(console);

    return console;
}

void
init_ruby_console(console_t *console)
{
	ruby_init();
	ruby_init_loadpath();

	VALUE kls;
    
    kls = rb_define_class("Client", rb_cObject);
    rb_define_module_function(kls, "status", spacemmo_client_status, 0);
    rb_define_module_function(kls, "entity", spacemmo_client_entity, 0);

	kls = rb_define_class("Entity", rb_cObject);
    rb_define_method(kls, "initialize", spacemmo_entity_initialize,  0);
    rb_define_module_function(kls, "status", spacemmo_entity_status, 0);
    rb_define_module_function(kls, "thrust", spacemmo_entity_thrust, 3);

	kls = rb_define_class("World", rb_cObject);
}

VALUE
spacemmo_client_status(VALUE self)
{
    console_t *console = (console_t *)rb_gv_get("CONSOLE");
    entity_t *e = console->ui->client->entity;
    printf("pos: %f %f %f\n", e->pos.x, e->pos.y, e->pos.z);
    printf("vel: %f %f %f\n", e->vel.x, e->vel.y, e->vel.z);
    printf("acc: %f %f %f\n", e->acc.x, e->acc.y, e->acc.z);
    return Qnil;
}

VALUE
spacemmo_client_entity(VALUE self)
{
    return Qnil;
}

VALUE spacemmo_client_set_thrust(VALUE self, VALUE value, int offset)
{
    console_t *console = (console_t *)rb_gv_get("CONSOLE");
    entity_t *e = console->ui->client->entity;
    float *acc = &e->acc.x + offset;
    *acc = NUM2DBL(value);
    return self;
}

VALUE
spacemmo_client_set_x(VALUE self, VALUE value)
{
    return spacemmo_client_set_thrust(self, value, 0);
}

VALUE
spacemmo_client_set_y(VALUE self, VALUE value)
{
    return spacemmo_client_set_thrust(self, value, 1);
}

VALUE
spacemmo_client_set_z(VALUE self, VALUE value)
{
    return spacemmo_client_set_thrust(self, value, 2);
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

        rb_gv_set("CONSOLE", (VALUE)console);
        rb_eval_string(line);
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
	ruby_finalize();
    free(console);
}

