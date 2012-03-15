#include <stdlib.h>
#include <err.h>
#include <sysexits.h>
#include <dispatch/dispatch.h>

#include "src/lib/client.h"
#include "src/lib/ui.h"
#include "src/lib/ui/console.h"
#include "src/lib/ui/gfx.h"

int
main(int argc, char *argv[])
{
    init_spacemmo();

    client_t *client = init_client();
    client->ui = init_ui(client);

    time_delta(0);

    if (!connect_server(&client->server_conn, "127.0.0.1", 3377))
        err(EX_UNAVAILABLE, "server");

    client->username = "david";
    client_send(client, P_LOGIN_REQUEST);

    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        process_input(client->ui->console);
    });

    client_loop(client);
    shutdown_console(client->ui->console);

    return 0;
}

