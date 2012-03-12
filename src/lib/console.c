#include <histedit.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "src/lib/client.h"
#include "src/lib/entity.h"
#include "src/lib/console.h"
#include "src/lib/console.h"

void
init_console(console_t **console, char *name)
{
    *console = calloc(1, sizeof(console_t));

    asprintf(&(*console)->hist_file, "%s/.spacemmo_%s_history", getenv("HOME"), name);
    asprintf(&(*console)->prompt, "%s> ", name);

    (*console)->el = el_init(name, stdin, stdout, stderr);
    el_set((*console)->el, EL_EDITOR, "emacs");
    el_set((*console)->el, EL_CLIENTDATA, (void *)(*console));
    el_set((*console)->el, EL_PROMPT, prompt);

    (*console)->history = history_init();
    history((*console)->history, &(*console)->ev, H_SETSIZE, 800);
    history((*console)->history, &(*console)->ev, H_SETUNIQUE, 1);
    history((*console)->history, &(*console)->ev, H_LOAD, (*console)->hist_file);
    el_set((*console)->el, EL_HIST, history, (*console)->history);
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
process_input(EditLine *el)
{
    console_t *console;
    el_get(el, EL_CLIENTDATA, &console);
    client_t *client = console->client;

    const char *buf;

    while (true) {
        int count = 0;

        buf = el_gets(el, &count);

        if (buf == NULL)
            break;

        char *line = strdup(buf);
        line = strsep(&line, "\n");
        history(console->history, &console->ev, H_ENTER, buf);
        char *cmd = strsep(&line, " ");

        if (strcmp(cmd, "login") == 0) {
            client->username = line;
            client_send(client, P_LOGIN_REQUEST);

        } else if (strcmp(cmd, "thrust") == 0) {
            float x = atof(strsep(&line, " "));
            float y = atof(strsep(&line, " "));
            float z = atof(strsep(&line, " "));

            client->entity->acc.x = x;
            client->entity->acc.y = y;
            client->entity->acc.z = z;
            client_send(client, P_ENTITY_UPDATE_REQUEST);

        } else if (strcmp(cmd, "status") == 0) {
            if (client->entity) {
                printf("entity:\n");
                printf("\tpos %f, %f, %f\n", client->entity->pos.x, client->entity->pos.y, client->entity->pos.z);
                printf("\tvel %f, %f, %f\n", client->entity->vel.x, client->entity->vel.y, client->entity->vel.z);
                printf("\tacc %f, %f, %f\n", client->entity->acc.x, client->entity->acc.y, client->entity->acc.z);
            }
        } else if (strcmp(cmd, "scan") == 0) {
        }
    }
}

void
shutdown_console(EditLine *el)
{
    console_t *console;
    el_get(el, EL_CLIENTDATA, &console);

    history(console->history, &console->ev, H_SAVE, console->hist_file);
    history_end(console->history);
    el_end(el);
    free(console);
}

