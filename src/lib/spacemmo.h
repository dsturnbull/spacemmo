#ifndef __src_lib_spacemmo_h
#define __src_lib_spacemmo_h

#include <stdint.h>

#define GET_CONST(c) {                                                      \
    if (strcasecmp(op, #c) == 0) {                                          \
        return c;                                                           \
    }                                                                       \
} while (0)

typedef struct world_st world_t;

typedef uint32_t client_id_t;
typedef uint32_t entity_id_t;

typedef struct vec3f_st {
    double x, y, z;
} vec3f;

typedef uint8_t timer_t;

void init_spacemmo();
double time_delta(timer_t);
void init_default_world(world_t *);
char * replace_ext(char *, char *);

#endif

