#include <time.h>

static struct timespec  started_ts;

void
init_tickcount(void)
{
        clock_gettime(CLOCK_MONOTONIC, &started_ts);
}

/* microsecs */
unsigned
get_tickcount(void)
{
        struct timespec ts;
        unsigned        ticks;

        clock_gettime(CLOCK_MONOTONIC, &ts);
        if (ts.tv_nsec < started_ts.tv_nsec) {
                ticks = ((unsigned)(ts.tv_sec - started_ts.tv_sec - 1)) * 1000000;
                ticks += (1000000000 + ts.tv_nsec - started_ts.tv_nsec) / 1000;
        }
        else {
                ticks = ((unsigned)(ts.tv_sec - started_ts.tv_sec)) * 1000000;
                ticks += (ts.tv_nsec - started_ts.tv_nsec) / 1000;
        }

        return ticks;
}
