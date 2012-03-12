#ifndef __src_lib_world_h
#define __src_lib_world_h

#include "src/lib/spacemmo.h"

struct world_st {
    size_t entities_size;
    size_t entity_count;
    entity_t **entities;
};

void init_world(world_t **);
void update_world(world_t *, double dt);
bool add_entity(world_t *, entity_t *);
bool remove_entity(world_t *, entity_t *);
void foreach_entity(world_t *, void (^)(entity_t *));

#endif

