#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <signal.h>
#include <time.h>
#include <string.h>
#include "shared_queue.h"
#include "semaphore.h"
#include "time_stamp.h"

struct queue_info
{
    int size;
    int first_id;
    int last_id;
};

void print_queue(struct queue_info *qinfo, struct queue_entry *array, const char* preamble)
{
    printf("%s s%d f%d l%d", preamble, qinfo->size, qinfo->first_id, qinfo->last_id);
    for (int i = 0; i < qinfo->size; ++i)
    {
        printf(" %d", array->package_weight);
    }
    printf("\n");
}

int queue_size()
{
    return 0;
}

key_t queue_key(void)
{
    return ftok(getenv("HOME"), SHM_QUEUE);
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
    size_t size_in_bytes = sizeof(struct queue_info) + size * sizeof(struct queue_entry);
    int mem_id = shmget(queue_key(), size_in_bytes /* elements */,
                        0700 | IPC_CREAT | IPC_EXCL);

    if (mem_id == -1)
    {
        perror("shmget");
        raise(SIGINT);
    }

    void *memory = shmat(mem_id, NULL, 0);
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

int queue_operation_wrapper(int (*operation)(struct queue_info *, struct queue_entry *, struct queue_entry *), struct queue_entry *arg)
{
    int result;

    void *memory = get_queue();
    if (memory != NULL)
    {
        struct queue_info *qinfo = (struct queue_info *)memory;
        struct queue_entry *array = (struct queue_entry *)memory + sizeof(struct queue_info);

        result = operation(qinfo, array, arg);

        if (shmdt(memory) == -1)
        {
            perror("shmdt");
        }
    }

    return result;
}

int put_to_queue_internal(struct queue_info *qinfo, struct queue_entry *array, struct queue_entry *element)
{
    // print_queue(qinfo, array, "put");

    int new_id = (qinfo->first_id + 1) % qinfo->size;
    if (new_id == qinfo->last_id)
    {
        // Error: queue is full
        fprintf(stderr, "Cannot put to a full queue!\n");
        return -1;
    }

    memcpy(array + new_id * sizeof(struct queue_entry), element, sizeof(struct queue_entry));

    // if no elements were in the queue
    if (qinfo->first_id == -1 && qinfo->last_id == -1)
    {
        qinfo->last_id = new_id;
    }
    qinfo->first_id = new_id;

    return 0;
}

void put_to_queue(struct queue_entry *element)
{
    queue_operation_wrapper(put_to_queue_internal, element);
}

int get_from_queue_internal(struct queue_info *qinfo, struct queue_entry *array, struct queue_entry *element)
{
    // print_queue(qinfo, array, "get");

    if (qinfo->first_id == -1 && qinfo->last_id == -1)
    {
        // Error, the queue is empty
        fprintf(stderr, "Cannot get from empty queue!\n");
        return -1;
    }

    // struct queue_entry* qe = malloc(sizeof(struct queue_entry));
    memcpy(element, array + qinfo->last_id * sizeof(struct queue_entry), sizeof(struct queue_entry));

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

    return 0;
}

int get_from_queue(struct queue_entry *element)
{
    return queue_operation_wrapper(get_from_queue_internal, element);
}

int queue_capacity_internal(struct queue_info *qinfo, struct queue_entry *array, struct queue_entry *element)
{
    return qinfo->size;
}

int queue_capacity(void)
{
    return queue_operation_wrapper(queue_capacity_internal, 0);
}

void queue_close(void)
{
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
}
