#ifndef __src_lib_system_h
#define __src_lib_system_h

#include <stdbool.h>
#include <sys/types.h>

#include "src/lib/spacemmo.h"

struct system_st {
    cluster_t *cluster;

    size_t entities_size;
    size_t entity_count;
    entity_t **entities;

    vec3f pos, vel, acc;
};

system_t * init_system(cluster_t *);
void update_system(system_t *, double);
bool add_entity(system_t *, entity_t *);
bool remove_entity(system_t *, entity_t *);
void foreach_entity(system_t *, void (^)(entity_t *));

#endif

