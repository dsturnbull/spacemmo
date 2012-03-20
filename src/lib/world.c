#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "src/lib/entity.h"
#include "src/lib/world.h"

world_t *
init_world()
{
    world_t *world = calloc(1, sizeof(world_t));
    world->entities_size = 1024;
    world->entities = calloc(1024, sizeof(entity_t *));
    world->entity_count = 0;
    return world;
}

void
update_world(world_t *world, double dt)
{
    foreach_entity(world, ^(entity_t *e) {
        update_entity(e, dt);
    });
}

bool
add_entity(world_t *world, entity_t *entity)
{
    //if (world->entity_count == world->entities_size)
    //    world->entities = realloc(world->entities, world->entities_size *= 2);

    world->entities[world->entity_count++] = entity;
    return true;
}

bool
remove_entity(world_t *world, entity_t *entity)
{
    return true;
}

void
foreach_entity(world_t *world, void (^block)(entity_t *)) {
    for (size_t i = 0; i < world->entity_count; i++)
        if (!world->entities[i]->dead)
            block((entity_t *)world->entities[i]);
}

