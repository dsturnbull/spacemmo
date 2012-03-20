#include <stdlib.h>

#include "src/lib/server.h"
#include "src/lib/world.h"
#include "src/lib/entity.h"

int
main(int argc, char *argv[])
{
    server_t *server = calloc(1, sizeof(server_t));
    init_world(&server->world);

    /*
    entity_t *e = init_entity();
    e->id = 1;
    e->vel.x = 1;
    add_entity(server->world, e);
    */

    serve(server, "127.0.0.1", 3377);
    return 0;
}

