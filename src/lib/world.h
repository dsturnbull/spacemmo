#ifndef __src_lib_world_h
#define __src_lib_world_h

#include <stdbool.h>
#include <sys/types.h>

#include "src/lib/spacemmo.h"

struct world_st {
    size_t clusters_size;
    size_t cluster_count;
    cluster_t **clusters;
};

world_t * init_world();
void update_world(world_t *, double);
bool add_cluster(world_t *, cluster_t *);
bool remove_cluster(world_t *, cluster_t *);
void foreach_cluster(world_t *, void (^)(cluster_t *));
entity_t * find_entity(world_t *, entity_id_t);

#endif

