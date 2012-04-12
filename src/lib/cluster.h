#ifndef __src_lib_cluster_h
#define __src_lib_cluster_h

#include <stdbool.h>
#include <sys/types.h>

#include "src/lib/spacemmo.h"

typedef struct system_st system_t;
typedef struct world_st world_t;

typedef struct cluster_st {
    world_t *world;

    size_t systems_size;
    size_t system_count;
    system_t **systems;

    vec3f pos;
} cluster_t;

cluster_t * init_cluster(world_t *);
void update_cluster(cluster_t *, double);
bool add_system(cluster_t *, system_t *);
bool remove_system(cluster_t *, system_t *);
void foreach_system(cluster_t *, void (^)(system_t *));

#endif

