#include <stdlib.h>

#include "src/lib/server.h"
#include "src/lib/world.h"
#include "src/lib/entity.h"

int
main(int argc, char *argv[])
{
    server_t *server = init_server();
    server_loop(server);
    return 0;
}

