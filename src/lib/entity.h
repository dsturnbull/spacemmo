#ifndef __src_lib_entity_h
#define __src_lib_entity_h

#include <stdbool.h>
#include "src/lib/spacemmo.h"

struct entity_st {
    entity_id_t id;

    double birth;
    bool dead;

    vec3f *pos, *vel, *acc;

    input_t *input;
    system_t *system;
    computer_t *computer;
};

entity_t * init_entity(system_t *);
void update_entity(entity_t *, double);
void update_entity_state(entity_t *, entity_t *);

#endif

