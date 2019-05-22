#pragma once
#include <sys/types.h>

/*  POSIX HEADERS    */
#include <sys/stat.h>
#include <semaphore.h>
#include <fcntl.h>
/*     END HEADERS      */

#ifdef __cplusplus
    extern "C" {
#endif

typedef sem_t semaphore_t;

extern semaphore_t* sem_initialize(const char* name, int initial_value);
extern semaphore_t* sem_get(const char* name);
extern void sem_remove(semaphore_t* semaphore, const char* name);

/**
 * Calls sem_wait(semaphore, 1);
 * */
// extern void sem_wait(semaphore_t* semaphore, int value);
extern void sem_wait_one(semaphore_t* semaphore);
// extern void sem_signal(semaphore_t* semaphore, int value);
extern void sem_signal_one(semaphore_t* semaphore);


#ifdef __cplusplus
    }
#endif
