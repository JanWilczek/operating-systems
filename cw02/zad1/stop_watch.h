#pragma once
#include <stdlib.h>
#include <sys/times.h>

#ifdef __cplusplus
extern "C" {
#endif

#define WRITE_REALTIME 1 << 0
#define WRITE_USER_TIME 1 << 1
#define WRITE_KERNEL_TIME 1 << 2

typedef struct stop_watch
{
    struct tms start;
    clock_t realtime_start;

    struct tms stop;
    clock_t realtime_stop;

    char* filename;
    char* preamble;
} stop_watch_t;

extern void start_time_measurement(stop_watch_t* sw);

/**
 *  Writes the measured time to a file specified in the filename field of the stop_watch_t struct.
 * */
extern void stop_and_write(stop_watch_t* sw, int flags);
extern void stop(stop_watch_t* sw);
extern void free_stop_watch(stop_watch_t* sw);
extern int get_realtime_in_ms(stop_watch_t* sw);
extern int get_user_time_in_ms(stop_watch_t* sw);
extern int get_kernel_time_in_ms(stop_watch_t* sw);

#ifdef __cplusplus
}
#endif