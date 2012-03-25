#ifndef __src_lib_sector_h
#define __src_lib_sector_h

#include <stdbool.h>
#include <sys/types.h>

#include "src/lib/spacemmo.h"

struct sector_st {
    system_t *system;

    size_t entities_size;
    size_t entity_count;
    entity_t **entities;

    vec3f pos, vel, acc;
};

sector_t * init_sector(system_t *);
void update_sector(sector_t *, double);
bool add_entity(sector_t *, entity_t *);
bool remove_entity(sector_t *, entity_t *);
void foreach_entity(sector_t *, void (^)(entity_t *));

#endif

