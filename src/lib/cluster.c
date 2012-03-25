#include <stdlib.h>

#include "src/lib/cluster.h"
#include "src/lib/world.h"
#include "src/lib/system.h"

cluster_t *
init_cluster(world_t *world)
{
    cluster_t *cluster = calloc(1, sizeof(cluster_t));
    cluster->world = world;
    cluster->systems_size = 1024;
    cluster->systems = calloc(1024, sizeof(cluster_t));
    cluster->system_count = 0;
    add_cluster(world, cluster);
    return cluster;
}

void
update_cluster(cluster_t *cluster, double dt)
{
    foreach_system(cluster, ^(system_t *system) {
        update_system(system, dt);
    });
}

bool
add_system(cluster_t *cluster, system_t *system)
{
    cluster->systems[cluster->system_count++] = system;
    return true;
}

bool
remove_system(cluster_t *cluster, system_t *system)
{
    return true;
}

void
foreach_system(cluster_t *cluster, void (^block)(system_t *)) {
    for (size_t i = 0; i < cluster->system_count; i++)
        block((system_t *)cluster->systems[i]);
}

