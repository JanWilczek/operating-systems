#include "ipc_queue_sv.h"
#include "queue_common.h"
#include <sys/types.h>
#include <sys/times.h>
#include <sys/msg.h>
#include <stdlib.h>
#include <stdio.h>

ipc_queue_t* create_queue(enum QueueType type)
{
    int proj_id;
    switch(type)
    {
        case SERVER_QUEUE:
            proj_id = SERVER_QUEUE_PROJ_ID;
            break;
        case CLIENT_QUEUE:
            srand(times(NULL));
            proj_id = rand() / 2 - 1;
            break;
        default:
            return NULL;
    }

    // 1. Get a key for the queue
    key_t queue_key;
    
    if ((queue_key = ftok(getenv("HOME"), proj_id)) == -1)
    {
        perror("ftok");
        exit(EXIT_FAILURE);
    }

    // 2. Create queue and receive its ID
    int queue_id;
    if ((queue_id = msgget(queue_key, IPC_CREAT | IPC_EXCL)) == -1)
    {
        perror("msgget (queue creation)");
        exit(EXIT_FAILURE);
    }

    ipc_queue_t* ipc_queue = malloc(sizeof(ipc_queue_t));
    ipc_queue->key = queue_key;
    ipc_queue->id = queue_id;

    return ipc_queue;
}

ipc_queue_t* get_queue(key_t key)
{
    int queue_id;
    if ((queue_id = msgget(key, 0)) == -1)
    {
        perror("msgget (get existing queue)");
        exit(EXIT_FAILURE);
    }
    
    ipc_queue_t* ipc_queue = malloc(sizeof(ipc_queue_t));
    ipc_queue->key = key;
    ipc_queue->id = queue_id;

    return ipc_queue;
}

void remove_queue(ipc_queue_t* queue_to_remove)
{
    if (msgctl(queue_to_remove->id, IPC_RMID, NULL /* third argument is ignored in case of queue removal */) == -1)
    {
        free(queue_to_remove);
        perror("msgctl (queue removal)");
        exit(EXIT_FAILURE);
    }

    free(queue_to_remove);
}