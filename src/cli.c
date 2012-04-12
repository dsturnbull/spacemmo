#include <stdlib.h>
#include <err.h>
#include <sysexits.h>
#include <dispatch/dispatch.h>

#include "src/lib/client.h"
#include "src/lib/server.h"
#include "src/lib/world.h"
#include "src/lib/ui.h"
#include "src/lib/ui/console.h"
#include "src/lib/cpu/cpu.h"
#include "src/lib/cpu/stack_cpu.h"
#include "src/lib/cpu/hardware/tty.h"

int
main(int argc, char *argv[])
{
    //cpu_t *cpu = init_cpu(CPU_STACK_CPU);
    //cpu_load(cpu, "data/stack_cpu/hello.s");
    //cpu_start(cpu);

    //while (!cpu->cpu->halted)
    //    cpu_step(cpu);

    //cpu_status(cpu);

    //return 0;

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

    dispatch_async(dispatch_get_global_queue(0, 0), ^{
        client_loop(client);
    });

    process_input(client->ui->console);
    shutdown_console(client->ui->console);

    return 0;
}

