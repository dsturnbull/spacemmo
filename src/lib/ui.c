#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "src/lib/ui.h"
#include "src/lib/console.h"
#include "src/lib/gfx.h"

void
init_ui(ui_t **ui)
{
    *ui = calloc(1, sizeof(ui_t));
    init_console(&(*ui)->console, "client");
    init_gfx(&(*ui)->gfx);
}

void
update_ui(ui_t *ui, double dt)
{
    update_console(ui->console, dt);
    update_gfx(ui->gfx, dt);
    handle_input(ui->gfx);
}

void
shutdown_ui(ui_t *ui)
{
    free(ui);
}

