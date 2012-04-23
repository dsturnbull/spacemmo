#include <stdlib.h>
#include <err.h>
#include <sysexits.h>
#include <dispatch/dispatch.h>

#include "src/lib/client.h"
#include "src/lib/server.h"
#include "src/lib/world.h"
#include "src/lib/entity.h"
#include "src/lib/ui.h"
#include "src/lib/ui/console.h"
#include "src/lib/cpu/sasm.h"
#include "src/lib/cpu/cpu.h"
#include "src/lib/cpu/hardware/thruster.h"

int
main(int argc, char *argv[])
{
    FILE *log = fopen("/tmp/cpu.log", "a+");
    init_spacemmo();

    client_t *client    = init_client();

    client->ui          = init_ui(client);
    client->ui->console = init_console(client->ui, "client");

    time_delta(0);

    //if (!connect_server(&client->server_conn, "127.0.0.1", 3377))
    //    err(EX_UNAVAILABLE, "server");

    //client->username = "david";
    //client_send(client, P_LOGIN_REQUEST);

    init_client_kqueue(client);
    init_server_kqueue(client->server);
    init_default_world(client->server->world);

    client->entity = find_entity(client->server->world, 2);
    //client->entity->cpu->kbd->input = init_input(client->ui);
    //client->ui->input = client->entity->cpu->kbd->input;

    sasm_t *sasm = init_sasm();
    assemble(sasm, "data/progs/thrusters.s");
    load_cpu(client->entity->cpu, sasm->prog, sasm->prog_len);
    client->entity->cpu->halted = false;

    thruster_t *thruster = init_thruster(client->entity->cpu->port0,
            client->entity->acc);

    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        process_input(client->ui->console);
    });

    client_loop(client);
    shutdown_console(client->ui->console);
    fclose(log);

    return 0;
}

