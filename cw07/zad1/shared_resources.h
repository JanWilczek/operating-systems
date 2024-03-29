#pragma once
#include <sys/types.h>

#ifdef __cplusplus
    extern "C" {
#endif

struct queue_entry
{
    pid_t loader_id;
    time_t tv_sec;
    long tv_nsec;
    int package_weight;
};

#define SEM_TRUCK_READY 1
#define SEM_TAPE_COUNT 2
#define SEM_IS_PACKAGE 3
#define SEM_QUEUE 5
#define SHM_QUEUE 6


#ifdef __cplusplus
    }
#endif
