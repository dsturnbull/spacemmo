#ifndef __src_lib_entity_h
#define __src_lib_entity_h

#include <stdbool.h>
#include "src/lib/spacemmo.h"

struct entity_st {
    entity_id_t id;
    client_id_t client_id;
    system_t *system;

    double birth;
    bool dead;

    entity_t *parent;
    double period;

    vec3f pos, vel, acc;
};

entity_t * init_entity(system_t *);
void orbit_entity(entity_t *, entity_t *, double);
void update_entity(entity_t *, double);
void update_entity_state(entity_t *, entity_t *);

#endif

