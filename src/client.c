#include <stdlib.h>
#include <err.h>
#include <sysexits.h>
#include <dispatch/dispatch.h>

#include "src/lib/client.h"
#include "src/lib/ui.h"
#include "src/lib/console.h"
#include "src/lib/gfx.h"

int
SDL_main(int argc, char *argv[])
{
    client_t *client;
    init_client(&client);

    init_ui(&client->ui);
    client->ui->client = client;
    client->ui->console->client = client;

    while (true) {
        usleep(20000);
        update_ui(client->ui, 1);
        if (client->ui->gfx->quit == true)
            break;
    }

    /*
    if (!connect_server(&client->server_conn, "127.0.0.1", 3377))
        err(EX_UNAVAILABLE, "server");

    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        client_loop(client);
    });

    process_input(client->ui->console->el);
    shutdown_console(client->ui->console->el);
    */

    /*
    client->username = "david";
    client_send(client, P_LOGIN_REQUEST);
    */

    return 0;
}

