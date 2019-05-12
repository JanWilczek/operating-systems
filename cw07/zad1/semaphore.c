#include <stdlib.h>
#include "semaphore.h"

semaphore_t* sem_init(int initial_value)
{
    return NULL;
}

void sem_wait_one(semaphore_t* semaphore)
{
    sem_wait(semaphore, 1);
}

void sem_wait(semaphore_t* semaphore, int value)
{

}

void sem_signal_one(semaphore_t* semaphore)
{
    sem_signal(semaphore, 1);
}

void sem_signal(semaphore_t* semaphore, int value)
{

}
