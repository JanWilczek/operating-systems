#pragma once
#include <sys/types.h>

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct semaphore {
    key_t key;
    int id;
} semaphore_t;

extern semaphore_t* sem_init(char* pathname, int initial_value);
extern semaphore_t* sem_get(char* pathname);
extern void sem_remove(semaphore_t* semaphore);

/**
 * Calls sem_wait(semaphore, 1);
 * */
extern void sem_wait(semaphore_t* semaphore, int value);
extern void sem_wait_one(semaphore_t* semaphore);
extern void sem_signal(semaphore_t* semaphore, int value);
extern void sem_signal_one(semaphore_t* semaphore);


#ifdef __cplusplus
    }
#endif
