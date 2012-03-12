#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "src/lib/entity.h"

void
init_entity(entity_t **e)
{
    *e = calloc(1, sizeof(entity_t));
}

void
update_entity(entity_t *e, double dt)
{
    e->pos.x += e->vel.x * dt;
    e->pos.y += e->vel.y * dt;
    e->pos.z += e->vel.z * dt;

    e->vel.x += e->acc.x * dt;
    e->vel.y += e->acc.y * dt;
    e->vel.z += e->acc.z * dt;

    /*
    printf("entity %lu:\n", e->id);
    printf("\tpos %f, %f, %f\n", e->pos.x, e->pos.y, e->pos.z);
    printf("\tvel %f, %f, %f\n", e->vel.x, e->vel.y, e->vel.z);
    printf("\tacc %f, %f, %f\n", e->acc.x, e->acc.y, e->acc.z);
    */
}

void
update_entity_state(entity_t *e, entity_t *other)
{
    memcpy(&e->pos, &other->pos, sizeof(vec3f));
    memcpy(&e->vel, &other->vel, sizeof(vec3f));
    memcpy(&e->acc, &other->acc, sizeof(vec3f));
    e->dead = other->dead;
}

