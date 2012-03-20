#include <histedit.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

    return console;
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

        int argc;
        char **argv;

        tok_reset(console->t);
        int result = tok_str(console->t, line, &argc, (const char ***)&argv);
        const char *str = argv[0];
        cmd_t cmd = lookup(str);

        switch (cmd) {
            case CMD_NOTFOUND:
                fprintf(stderr, "%s not found\n", str);
                break;

            case CMD_STATUS:
                cmd_status(console, argc, argv);
                break;

            case CMD_SCAN:
                cmd_scan(console, argc, argv);
                break;

            case CMD_THRUST:
                cmd_thrust(console, argc, argv);
                break;

            case CMD_QUIT:
                console->ui->client->quit = true;
                break;
        }
    }

    console->ui->client->quit = true;
}

cmd_t
lookup(const char *str)
{
    int cmd_num = sizeof(cmds) / sizeof(*cmds);

    for (int i = 0; i < cmd_num; i++)
        if (strncmp(str, cmds[i], strlen(cmds[i])) == 0)
            return (cmd_t)i + 1;

    return CMD_NOTFOUND;
}

void
cmd_status(console_t *console, int argc, char *argv[])
{
    entity_t *e;
    if ((e = console->ui->client->entity) != NULL) {
        fprintf(stderr, "pos %f %f %f\n", e->pos.x, e->pos.y, e->pos.z);
        fprintf(stderr, "vel %f %f %f\n", e->vel.x, e->vel.y, e->vel.z);
        fprintf(stderr, "acc %f %f %f\n", e->acc.x, e->acc.y, e->acc.z);
    }
}

void
cmd_scan(console_t *console, int argc, char *argv[])
{
    printf("hi\n");
}

void
cmd_thrust(console_t *console, int argc, char *argv[])
{
    int ch;

    //while ((ch = getopt(
}

void
shutdown_console(console_t *console)
{
    history(console->history, &console->ev, H_SAVE, console->hist_file);
    history_end(console->history);
    tok_end(console->t);
    el_end(console->el);
    free(console);
}

