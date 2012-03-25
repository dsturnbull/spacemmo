#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "src/lib/entity.h"
#include "src/lib/system.h"

entity_id_t last_id = 0;

entity_t *
init_entity(system_t *system)
{
    entity_t *entity = calloc(1, sizeof(entity_t));
    entity->system = system;
    entity->id = last_id++;

    struct timeval t;
    gettimeofday(&t, NULL);
    entity->birth = (double)t.tv_sec + (double)t.tv_usec / 1000 / 1000;

    add_entity(system, entity);
    return entity;
}

void
orbit_entity(entity_t *planet, entity_t *star, double year)
{
    planet->parent = star;
    planet->period = year;
}

void
update_entity(entity_t *e, double dt)
{
    if (e->parent && e->period > 0) {
        //(x - a)² + (y - b)² = r²
        /* long double radius = sqrt( */
        /*         pow(e->pos.x - e->parent->pos.x, 2) +  */
        /*         pow(e->pos.y - e->parent->pos.y, 2) +  */
        /*         pow(e->pos.z - e->parent->pos.z, 2)); */

        /* struct timeval t; */
        /* gettimeofday(&t, NULL); */
        /* double ts = (double)t.tv_sec + (double)t.tv_usec / 1000 / 1000; */

        /* long double angle_per_sec = 2 * M_PI / e->period; */
        /* long double angle = ts - e->birth * angle_per_sec; */

        /* e->pos.x = radius * cos(angle); */
        /* e->pos.y = radius * sin(angle); */

    } else {
        e->pos.x += e->vel.x * dt;
        e->pos.y += e->vel.y * dt;
        e->pos.z += e->vel.z * dt;

        e->vel.x += e->acc.x * dt;
        e->vel.y += e->acc.y * dt;
        e->vel.z += e->acc.z * dt;
    }

    /* printf("entity %lu:\n", e->id); */
    /* printf("\tpos %g, %g, %g\n", e->pos.x, e->pos.y, e->pos.z); */
    /* printf("\tvel %g, %g, %g\n", e->vel.x, e->vel.y, e->vel.z); */
    /* printf("\tacc %g, %g, %g\n", e->acc.x, e->acc.y, e->acc.z); */
}

void
update_entity_state(entity_t *e, entity_t *other)
{
    memcpy(&e->pos, &other->pos, sizeof(vec3f));
    memcpy(&e->vel, &other->vel, sizeof(vec3f));
    memcpy(&e->acc, &other->acc, sizeof(vec3f));
    e->dead = other->dead;
}

