#pragma once
#include <stdlib.h>
#include <sys/times.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct stop_watch
{
    struct tms* start;
    clock_t realtime_start;

    struct tms* stop;
    clock_t realtime_stop;

    char* filename;
    char* preamble;
} stop_watch_t;

extern void start_time_measurement(stop_watch_t* sw);
extern void stop_and_write(stop_watch_t* sw);
extern void stop(stop_watch_t* sw);
extern void free_stop_watch(stop_watch_t* sw);

#ifdef __cplusplus
}
#endif