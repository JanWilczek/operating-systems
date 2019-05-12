#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

typedef struct semaphore {

} semaphore_t;

extern semaphore_t* sem_init(int initial_value);

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
