#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "src/lib/spacemmo.h"

struct timeval * timers[256 << (sizeof(timer_t) * 8) - 8];

void
init_spacemmo()
{
    memset(&timers, 0, sizeof(timers));
}

double
time_delta(timer_t timer)
{
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

