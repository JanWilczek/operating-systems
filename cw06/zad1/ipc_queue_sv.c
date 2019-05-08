#include "ipc_queue_sv.h"
#include "queue_common.h"
#include <sys/types.h>
#include <sys/times.h>
#include <sys/msg.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>


struct msgbuf {
    long mtype;
    char mtext[MSG_MAX_SIZE];
};

struct client_msg {
    long mtype;
    long mclient_id;
    char mtext[MSG_MAX_SIZE - sizeof(long)];
};

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
            proj_id = rand() / 2 + 1;
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
    if ((queue_id = msgget(queue_key, IPC_CREAT | IPC_EXCL | 0700 /* permission bits */)) == -1)
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
        return NULL;
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

int receive_message(ipc_queue_t* queue, char* buffer, size_t buffer_size, long* type, int block)
{
    struct msgbuf msgp;

    int err = msgrcv(queue->id, &msgp, buffer_size, *type, IPC_NOWAIT * (1 - block));

    *type = msgp.mtype;
    strcpy(buffer, msgp.mtext);

    return err;
}

int send_message(ipc_queue_t* queue, char* buffer, long type)
{
    struct msgbuf msgp;

    msgp.mtype = type;
    strcpy(msgp.mtext, buffer);

    return msgsnd(queue->id, (void *) &msgp, strlen(msgp.mtext) + 1, 0);
}

int client_send_message(ipc_queue_t* server_queue, long client_id, const char* buffer, long type)
{
    struct client_msg msg;

    msg.mtype = type;
    msg.mclient_id = client_id;
    strcpy(msg.mtext, buffer);

    return msgsnd(server_queue->id, (void *) &msg, sizeof(long) + strlen(msg.mtext) + 1, 0);
}

int client_receive_message(ipc_queue_t* client_queue, long client_id, char* buffer, size_t buffer_size, long* type, int block)
{
    struct msgbuf msg;

    int err = msgrcv(client_queue->id, &msg, buffer_size, *type, IPC_NOWAIT * (1 - block));

    // if (msg.mtype != client_id && msg.mtype != STOP)
    // {
    //     fprintf(stderr, "Incorrectly addressed message in client's queue: is %ld should be %ld.\n", msg.mtype, client_id);
    //     return -1;
    // }

    *type = msg.mtype;
    strcpy(buffer, msg.mtext);

    return err;
}

int server_receive_client_message(ipc_queue_t* server_queue, long* client_id, char* buffer, size_t buffer_size, long* type, int block)
{
    struct client_msg msg;

    int err = msgrcv(server_queue->id, &msg, buffer_size, *type, IPC_NOWAIT * (1 - block));

    *client_id = msg.mclient_id;
    *type = msg.mtype;
    strcpy(buffer, msg.mtext);

    return err;
}