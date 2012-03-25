#include <stdlib.h>

#include "src/lib/system.h"
#include "src/lib/cluster.h"
#include "src/lib/entity.h"

system_t *
init_system(cluster_t *cluster)
{
    system_t *system = calloc(1, sizeof(system_t));
    system->cluster = cluster;
    system->entities_size = 1024;
    system->entities = calloc(1024, sizeof(system_t));
    system->entity_count = 0;
    add_system(cluster, system);
    return system;
}

void
update_system(system_t *system, double dt)
{
    foreach_entity(system, ^(entity_t *entity) {
        update_entity(entity, dt);
    });
}

bool
add_entity(system_t *system, entity_t *entity)
{
    system->entities[system->entity_count++] = entity;
    return true;
}

bool
remove_entity(system_t *system, entity_t *entity)
{
    return true;
}

void
foreach_entity(system_t *system, void (^block)(entity_t *)) {
    for (size_t i = 0; i < system->entity_count; i++)
        block((entity_t *)system->entities[i]);
}

