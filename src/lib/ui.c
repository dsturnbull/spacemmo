#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "src/lib/client.h"
#include "src/lib/ui.h"
#include "src/lib/ui/console.h"

ui_t *
init_ui(client_t *client)
{
    ui_t *ui = calloc(1, sizeof(ui_t));
    ui->client = client;
    return ui;
}

void
update_ui(ui_t *ui, double dt)
{
    if (ui->console)
        update_console(ui->console, dt);
}

void
shutdown_ui(ui_t *ui)
{
    shutdown_console(ui->console);
    free(ui);
}

