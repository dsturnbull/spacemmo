#ifndef __src_lib_entity_h
#define __src_lib_entity_h

#include <stdbool.h>
#include "src/lib/spacemmo.h"

typedef struct input_st input_t;
typedef struct system_st system_t;
typedef struct cpu_st cpu_t;

typedef struct entity_st {
    entity_id_t id;

    double birth;
    bool dead;

    vec3f *pos, *vel, *acc;
    vec3f *ypr;
    long mass;

    input_t *input;
    system_t *system;
    cpu_t *cpu;
} entity_t;

entity_t * init_entity(system_t *);
void update_entity(entity_t *, double);
void update_entity_state(entity_t *, entity_t *);

#endif

