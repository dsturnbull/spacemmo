#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#include "src/lib/world.h"
#include "src/lib/cluster.h"
#include "src/lib/system.h"
#include "src/lib/entity.h"

world_t *
init_world()
{
    world_t *world = calloc(1, sizeof(world_t));

    world->clusters_size = 1024;
    world->clusters = calloc(1024, sizeof(cluster_t));
    world->cluster_count = 0;

    return world;
}

void
update_world(world_t *world, double dt)
{
    foreach_cluster(world, ^(cluster_t *c) {
        update_cluster(c, dt);
    });
}

bool
add_cluster(world_t *world, cluster_t *cluster)
{
    //if (world->cluster_count == world->clusters_size)
    //    world->clusters = realloc(world->clusters, world->clusters_size *= 2);

    world->clusters[world->cluster_count++] = cluster;
    return true;
}

bool
remove_cluster(world_t *world, cluster_t *cluster)
{
    return true;
}

void
foreach_cluster(world_t *world, void (^block)(cluster_t *)) {
    for (size_t i = 0; i < world->cluster_count; i++)
        block((cluster_t *)world->clusters[i]);
}

entity_t *
find_entity(world_t *world, entity_id_t id)
{
    __block entity_t *e = NULL;

    foreach_cluster(world, ^(cluster_t *cluster) {
        foreach_system(cluster, ^(system_t *system) {
            foreach_entity(system, ^(entity_t *entity) {
                if (entity->id == id) {
                    e = entity;
                }
            });
        });
    });

    return e;
}

