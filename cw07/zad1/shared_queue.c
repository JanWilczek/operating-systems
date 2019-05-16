#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <signal.h>
#include "shared_queue.h"
#include "semaphore.h"

#define SHM_QUEUE "~/tape_queue"
// #define SEM_QUEUE "~/queue_sem"

struct queue_info
{
    int size;
    int first_id;
    int last_id;
};

int queue_size()
{
    return 0;
}

key_t queue_key(void)
{
    return ftok(SHM_QUEUE, 'a');
}

void *get_queue(void)
{
    int queue_id = shmget(queue_key(), 0, 0);
    if (queue_id == -1)
    {
        perror("shmget");
        return NULL;
    }

    void *memory = shmat(queue_id, NULL, 0);
    if (memory == (void *)-1)
    {
        perror("shmat");
        return NULL;
    }

    return memory;
}

void queue_init(int size)
{
    int err = shmget(queue_key(), sizeof(struct queue_info) + size * sizeof(int) /* elements */,
                     0700 | IPC_CREAT | IPC_EXCL);

    if (err == -1)
    {
        perror("shmget");
        raise(SIGINT);
    }

    void *memory = shmat(queue_key(), NULL, 0);
    if (memory == (void *)-1)
    {
        perror("shmat");
        raise(SIGINT);
    }

    struct queue_info *qinfo = (struct queue_info *)memory;
    qinfo->size = size;
    qinfo->first_id = -1;
    qinfo->last_id = -1;

    shmdt(memory);
}

int queue_operation_wrapper(int (*operation)(struct queue_info *, int *, int *), int arg)
{
    // semaphore_t *queue_sem = sem_get(SEM_QUEUE);
    // sem_wait_one(queue_sem);

    int result;

    void *memory = get_queue();
    if (memory != NULL)
    {
        struct queue_info *qinfo = (struct queue_info *)memory;
        int *array = memory + sizeof(struct queue_info);

        result = operation(qinfo, array, &arg);

        if (shmdt(memory) == -1)
        {
            perror("shmdt");
        }
    }

    // sem_signal_one(queue_sem);
    // free(queue_sem);

    return result;
}

int put_to_queue_internal(struct queue_info *qinfo, int *array, int *element)
{
    int new_id = (qinfo->first_id + 1) % qinfo->size;
    array[new_id] = *element;

    // if no elements were in the queue
    if (qinfo->first_id == -1 && qinfo->last_id == -1)
    {
        qinfo->last_id = new_id;
    }
    else if (qinfo->first_id + 1 == qinfo->last_id)
    {
        // Error: queue is full
        fprintf(stderr, "Cannot put to a full queue!\n");
        return -1;
    }

    qinfo->first_id = new_id;

    return 0;
}

void put_to_queue(int element)
{
    queue_operation_wrapper(put_to_queue_internal, element);
}

int get_from_queue_internal(struct queue_info *qinfo, int *array, int *element)
{
    if (qinfo->first_id == -1 && qinfo->last_id == -1)
    {
        // Error, the queue is empty
        fprintf(stderr, "Cannot get from empty queue!\n");
        return -1;
    }

    int result = array[qinfo->last_id];
    *element = result;

    if (qinfo->last_id == qinfo->first_id)
    {
        // Queue is empty now
        qinfo->last_id = -1;
        qinfo->first_id = -1;
    }
    else
    {
        qinfo->last_id = (qinfo->last_id + 1) % qinfo->size;
    }

    return result;
}

int get_from_queue(void)
{
    return queue_operation_wrapper(get_from_queue_internal, 0);
}

int queue_capacity_internal(struct queue_info *qinfo, int *array, int *element)
{
    return qinfo->size;
}

int queue_capacity(void)
{
    // TODO: Fine grain operation wrapper between memory wrapper and semaphore wrapper
    return queue_operation_wrapper(queue_capacity_internal, 0);
}

void queue_close(void)
{
    // semaphore_t *queue_sem = sem_get(SEM_QUEUE);
    // sem_wait_one(queue_sem);

    int queue_id = shmget(queue_key(), 0, 0);
    if (queue_id == -1)
    {
        perror("shmget");
    }
    else
    {
        if (shmctl(queue_id, IPC_RMID, NULL) == -1)
        {
            perror("shmctl");
        }
    }

    // sem_signal_one(queue_sem);
    // free(queue_sem);
}
