#include <stdlib.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <signal.h>
#include "shared_queue.h"

#define SHM_QUEUE "~/tape_queue"
#define SEM_QUEUE "~/queue_sem"

struct queue_info {
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
    ftok(SHM_QUEUE, 'a');
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

    void* memory = shmat(queue_key(), NULL, 0);
    if (memory == -1)
    {
        perror("shmat");
        raise(SIGINT);
    }

    struct queue_info* qinfo = (struct queue_info*) memory;
    qinfo->size = size;
    qinfo->first_id = -1;
    qinfo->last_id = 0;

    shmdt(memory);
}

void put_to_queue(int element)
{

    int queue_id = shmget(queue_key(), 0, 0);
    void* memory = shmat(queue_id, NULL, 0);

    shmdt(memory);

}

int get_from_queue(void)
{
    return 0;
}

int queue_capacity(void)
{
    return 0;
}
