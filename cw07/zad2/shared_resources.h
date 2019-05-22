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

#define SEM_TRUCK_READY "/SEM_TRUCK_READY"
#define SEM_TAPE_COUNT "/SEM_TAPE_COUNT"
#define SEM_IS_PACKAGE "/SEM_IS_PACKAGE"
#define SEM_QUEUE "/SEM_QUEUE"
#define SHM_QUEUE "/SHM_QUEUE"
#define MAX_SHM_SIZE 4096


#ifdef __cplusplus
    }
#endif
