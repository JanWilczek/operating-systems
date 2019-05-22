#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "semaphore.h"

#define SEM_NUM 0

// Copied from man
union semun {
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO
                                           (Linux-specific) */
};

int sem_get_val(sem_t* semaphore)
{
    int value;
    if (sem_getvalue(semaphore, &value) == -1)
    {
        perror("sem_getvalue");
        return 0;
    }
    return value;
}

semaphore_t *sem_initialize(const char* name, int initial_value)
{
    // Create the semaphore
    sem_t* semaphore = sem_open(name, O_CREAT | O_EXCL | O_RDWR, 0700, initial_value);
    if (semaphore == SEM_FAILED)
    {
        perror("semget");
        return NULL;
    }
    else
    {
        // Check if properly initialized 
        assert(sem_get_val(semaphore) == initial_value);
    }
    
    // printf("Semaphore %d value after initialization %d\n.", semaphore_id, sem_get_val(semaphore_id));

    return semaphore;
}

semaphore_t* sem_get(const char* name)
{
    // Create the semaphore
    sem_t* semaphore = sem_open(name, O_RDWR);
    if (semaphore == SEM_FAILED)
    {
        perror("semget");
        return NULL;
    }

    return semaphore;
}

void sem_remove(semaphore_t* semaphore, const char* name)
{
    if (sem_close(semaphore) == -1)
    {
        perror("sem_close");
    }
    else
    {
        if (sem_unlink(name) == -1)
        {
            perror("sem_remove");
        }
    }
}

void sem_wait_one(semaphore_t *semaphore)
{
    if (sem_wait(semaphore) == -1)
    {
        perror("sem_wait");
    }
}

void sem_signal_one(semaphore_t *semaphore)
{
    if (sem_post(semaphore) == -1)
    {
        perror("sem_post");
    }
}
