#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include <stdarg.h>

#include "src/lib/spacemmo.h"
#include "src/lib/world.h"
#include "src/lib/cluster.h"
#include "src/lib/system.h"
#include "src/lib/entity.h"
#include "src/lib/cpu/cpu.h"

struct timeval * timers[256 << (sizeof(timer_t) * 8) - 8];

void
init_spacemmo()
{
    memset(&timers, 0, sizeof(timers));
}

double
time_delta(timer_t timer)
{
    // TODO just use kevent
    if (!timers[timer]) {
        timers[timer] = malloc(sizeof(struct timeval));
        gettimeofday(timers[timer], NULL);
        return 0.0f;
    }

    struct timeval *t0 = timers[timer];
    struct timeval t1;

    gettimeofday(&t1, NULL);

    double t0f = (double)t0->tv_sec + (double)t0->tv_usec / 1000 / 1000;
    double t1f = (double)t1.tv_sec + (double)t1.tv_usec / 1000 / 1000;
    double dt = t1f - t0f;

    memcpy(timers[timer], &t1, sizeof(struct timeval));

    return dt;
}

void
init_default_world(world_t *world)
{
    cluster_t *local_cluster = init_cluster(world);

    system_t *sol_system = init_system(local_cluster);

    entity_t *sol = init_entity(sol_system);

    entity_t *earth = init_entity(sol_system);
    earth->pos->x = 1.496e9;

    entity_t *engine = init_entity(sol_system);
    engine->cpu = init_cpu();
    engine->cpu->debug = true;
}

char *
replace_ext(char *str, char *ext)
{
    char *orig = strdup(str);
    char *rpl = orig;
    char *new;
    strsep(&rpl, ".");
    asprintf(&new, "%s%s", orig, ext);
    free(orig);
    return new;
}

void
slog(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    if (getenv("DEBUG"))
        vfprintf(stderr, fmt, ap);
    va_end(ap);
}

