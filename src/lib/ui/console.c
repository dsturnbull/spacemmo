#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "src/lib/client.h"
#include "src/lib/server.h"
#include "src/lib/ui.h"
#include "src/lib/ui/console.h"
#include "src/lib/world.h"
#include "src/lib/cluster.h"
#include "src/lib/system.h"
#include "src/lib/entity.h"
#include "src/lib/cpu/sasm.h"
#include "src/lib/cpu/cpu.h"

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

    while (!client->quit) {
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

            case CMD_BECOME:
                cmd_become(console, argc, argv);
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

            case CMD_CPU_LOAD:
                cmd_cpu_load(console, argc, argv);
                break;

            case CMD_CPU_START:
                cmd_cpu_start(console, argc, argv);
                break;

            case CMD_CPU_STOP:
                cmd_cpu_stop(console, argc, argv);
                break;

            case CMD_CPU_RESET:
                cmd_cpu_reset(console, argc, argv);
                break;

            case CMD_CPU_STEP:
                cmd_cpu_step(console, argc, argv);
                break;

            case CMD_QUIT:
                client->quit = true;
                break;
        }
    }

    client->quit = true;
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
cmd_become(console_t *console, int argc, char *argv[])
{
    world_t *world = console->ui->client->server->world;
    entity_id_t id = atol(argv[1]);
    console->ui->client->entity = find_entity(world, id);
}

void
cmd_status(console_t *console, int argc, char *argv[])
{
    entity_t *e;
    if ((e = console->ui->client->entity) != NULL) {
        fprintf(stderr, "pos %g %g %g\n", e->pos->x, e->pos->y, e->pos->z);
        fprintf(stderr, "vel %g %g %g\n", e->vel->x, e->vel->y, e->vel->z);
        fprintf(stderr, "acc %g %g %g\n", e->acc->x, e->acc->y, e->acc->z);
    }
}

void
cmd_scan(console_t *console, int argc, char *argv[])
{
    world_t *world = console->ui->client->server->world;

    foreach_cluster(world, ^(cluster_t *cluster) {
        foreach_system(cluster, ^(system_t *system) {
            foreach_entity(system, ^(entity_t *entity) {
                fprintf(stderr, "ent %u \n", entity->id);
                fprintf(stderr, "\tpos %g %g %g\n",
                    entity->pos->x, entity->pos->y, entity->pos->z);
                fprintf(stderr, "\tvel %g %g %g\n",
                    entity->vel->x, entity->vel->y, entity->vel->z);
                fprintf(stderr, "\tacc %g %g %g\n",
                    entity->acc->x, entity->acc->y, entity->acc->z);
            });
        });
    });
}

void
cmd_thrust(console_t *console, int argc, char *argv[])
{
    int ch;
    entity_id_t id;
    world_t *world = console->ui->client->server->world;

    if (console->ui->client->entity)
        id = console->ui->client->entity->id;

    argc = 1;
    float dax = atof(argv[argc++]);
    float day = atof(argv[argc++]);
    float daz = atof(argv[argc++]);

    entity_t *entity;
    if ((entity = find_entity(world, id)) != NULL) {
        entity->acc->x = dax;
        entity->acc->y = day;
        entity->acc->z = daz;
    }
}

void
cmd_cpu_load(console_t *console, int argc, char *argv[])
{
    cpu_t *cpu = console->ui->client->entity->cpu;
    //sasm_t *sasm = init_sasm();
    //assemble(sasm, argv[1]);
    //load_cpu(cpu, (char *)sasm->prog, sasm->prog_len);
}

void
cmd_cpu_start(console_t *console, int argc, char *argv[])
{
    cpu_t *cpu = console->ui->client->entity->cpu;
    cpu->halted = false;
}

void
cmd_cpu_stop(console_t *console, int argc, char *argv[])
{
    cpu_t *cpu = console->ui->client->entity->cpu;
    cpu->halted = true;
}

void
cmd_cpu_reset(console_t *console, int argc, char *argv[])
{
    cpu_t *cpu = console->ui->client->entity->cpu;
    reset_cpu(cpu);
}

void
cmd_cpu_step(console_t *console, int argc, char *argv[])
{
    cpu_t *cpu = console->ui->client->entity->cpu;
    step_cpu(cpu);
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

