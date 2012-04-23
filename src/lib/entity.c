#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "src/lib/entity.h"
#include "src/lib/system.h"
#include "src/lib/cpu/cpu.h"

entity_id_t last_id = 0;

entity_t *
init_entity(system_t *system)
{
    entity_t *this = calloc(1, sizeof(entity_t));
    this->system = system;
    this->id = last_id++;

    struct timeval t;
    gettimeofday(&t, NULL);
    this->birth = (double)t.tv_sec + (double)t.tv_usec / 1000 / 1000;

    this->pos = calloc(1, sizeof(vec3f));
    this->vel = calloc(1, sizeof(vec3f));
    this->acc = calloc(1, sizeof(vec3f));
    this->ypr = calloc(1, sizeof(vec3f));
    this->mass = 1e3;

    add_entity(system, this);
    return this;
}

void
update_entity(entity_t *this, double dt)
{
    float thrust_amt = 1;

    vec3f *pos = this->pos;
    vec3f *vel = this->vel;
    vec3f *acc = this->acc;
    vec3f *ypr = this->ypr;

    /*
    // thrust
    if (keys['w'])
        acc->z += thrust_amt;

    if (keys['s'])
        acc->z -= thrust_amt;

    if (keys['a'])
        acc->x -= thrust_amt;

    if (keys['d'])
        acc->x += thrust_amt;

    if (keys['q'])
        acc->y += thrust_amt;

    if (keys['z'])
        acc->y -= thrust_amt;

    // ypr
    if (keys['k'])
        ypr->y -= thrust_amt;

    if (keys['j'])
        ypr->y += thrust_amt;

    if (keys['h'])
        ypr->x -= thrust_amt;

    if (keys['l'])
        ypr->x += thrust_amt;

    if (keys[','])
        ypr->z += thrust_amt;

    if (keys['.'])
        ypr->z -= thrust_amt;

    // stop
    if (keys[' '])
        acc->x = acc->y = acc->z = 0.0;

    if (keys['r']) {
        vel->x = vel->y = 0.0;
        acc->x = acc->y = acc->z = 0.0;
    }
    */

    this->pos->x += this->vel->x * dt;
    this->pos->y += this->vel->y * dt;
    this->pos->z += this->vel->z * dt;

    this->vel->x += this->acc->x * dt;
    this->vel->y += this->acc->y * dt;
    this->vel->z += this->acc->z * dt;

    /* printf("entity %lu:\n", e->id); */
    /* printf("\tpos %g, %g, %g\n", e->pos.x, e->pos.y, e->pos.z); */
    /* printf("\tvel %g, %g, %g\n", e->vel.x, e->vel.y, e->vel.z); */
    /* printf("\tacc %g, %g, %g\n", e->acc.x, e->acc.y, e->acc.z); */
}

void
update_entity_state(entity_t *this, entity_t *other)
{
    memcpy(&this->pos, &other->pos, sizeof(vec3f));
    memcpy(&this->vel, &other->vel, sizeof(vec3f));
    memcpy(&this->acc, &other->acc, sizeof(vec3f));
    this->dead = other->dead;
}

void
entity_slot_set(entity_t *this, entity_t *other)
{
}

