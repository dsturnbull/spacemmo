#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>

#include "src/lib/entity.h"
#include "src/lib/system.h"
#include "src/lib/computer.h"
#include "src/lib/ui/input.h"

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

    this->input = init_input(NULL);

    add_entity(system, this);
    return this;
}

void
update_entity(entity_t *this, double dt)
{
    if (this->computer)
        update_computer(this->computer, dt);

    bool *keys = this->input->keys;
    float thrust_amt = 0.1;
    vec3f *acc = this->acc;

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

    if (keys[' '])
        acc->x = acc->y = acc->z = 0.0;

    if (false) { //e->parent && e->period > 0) {
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
        this->pos->x += this->vel->x * dt;
        this->pos->y += this->vel->y * dt;
        this->pos->z += this->vel->z * dt;

        this->vel->x += this->acc->x * dt;
        this->vel->y += this->acc->y * dt;
        this->vel->z += this->acc->z * dt;
    }

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

