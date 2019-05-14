#include <stdlib.h>
#include <stdio.h>

/*  SYSTEM V HEADERS    */
#include <sys/sem.h>
#include <sys/ipc.h>
/*     END HEADERS      */

#include "semaphore.h"

#define PROJ_ID 'a'

// Copied from man
union semun {
    int val;               /* Value for SETVAL */
    struct semid_ds *buf;  /* Buffer for IPC_STAT, IPC_SET */
    unsigned short *array; /* Array for GETALL, SETALL */
    struct seminfo *__buf; /* Buffer for IPC_INFO
                                           (Linux-specific) */
};

semaphore_t *sem_init(char *pathname, int initial_value)
{
    // Create key
    key_t semaphore_key = ftok(pathname, PROJ_ID);

    // Create the semaphore
    int semaphore_id = semget(semaphore_key, 1, IPC_CREAT | IPC_EXCL | 0700);
    if (semaphore_id == -1)
    {
        perror("semget");
        return NULL;
    }

    // Initialize the semaphore with initial_value
    union semun arg;
    arg.val = initial_value;
    if (semctl(semaphore_id, 0, SETVAL, arg) == -1)
    {
        perror("semctl");
    }

    // Create the wrapper
    semaphore_t *semaphore = malloc(sizeof(semaphore_t));
    semaphore->key = semaphore_key;
    semaphore->id = semaphore_id;

    return semaphore;
}

semaphore_t* sem_get(char* pathname)
{
    // Create keys
    key_t semaphore_key = ftok(pathname, PROJ_ID);

    // Create the semaphore
    int semaphore_id = semget(semaphore_key, 0, 0700);
    if (semaphore_id == -1)
    {
        perror("semget");
        return NULL;
    }

    // Create the wrapper
    semaphore_t *semaphore = malloc(sizeof(semaphore_t));
    semaphore->key = semaphore_key;
    semaphore->id = semaphore_id;

    return semaphore;
}

void sem_remove(semaphore_t* semaphore)
{
    if (semctl(semaphore->id, 0, IPC_RMID) == -1)
    {
        perror("semctl (remove semaphore)");
    }

    free(semaphore);
}

void sem_wait_one(semaphore_t *semaphore)
{
    sem_wait(semaphore, 1);
}

void sem_wait(semaphore_t *semaphore, int value)
{
    struct sembuf op = {0, - value /* decrease semaphore value by value */, SEM_UNDO};
    struct sembuf ops[1];
    ops[0] = op;

    if (semop(semaphore->id, ops, 1) == -1)
    {
        perror("semop (sem_wait)");
    }
}

void sem_signal_one(semaphore_t *semaphore)
{
    sem_signal(semaphore, 1);
}

void sem_signal(semaphore_t *semaphore, int value)
{
    struct sembuf op = {0, value /* increase semaphore value by value */, SEM_UNDO};
    struct sembuf ops[1];
    ops[0] = op;

    if (semop(semaphore->id, ops, 1) == -1)
    {
        perror("semop (sem_signal)");
    }
}
