#include "queue_common.h"
#include "ipc_queue_posix.h"
#include <string.h>
#include <sys/times.h>
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
            proj_id = rand() / 2 + 1;
            break;
        default:
            return NULL;
    }

    const int NAME_SIZE = 1000;
    char* name = malloc(NAME_SIZE);
    snprintf(name, NAME_SIZE, "/ipc_queue_%d", proj_id);

    mqd_t queue_descriptor = mq_open(name, O_RDWR | O_CREAT | O_EXCL, 0700, NULL);
    if (queue_descriptor == -1)
    {
        perror("mq_open (queue creation)");
        free(name);
        return NULL;
    }

    ipc_queue_t* queue = malloc(sizeof(ipc_queue_t));
    queue->queue_descriptor = queue_descriptor;
    queue->name = name;

    return queue;
}

ipc_queue_t* get_queue(const char* name)
{
    mqd_t queue_descriptor = mq_open(name, O_RDWR);
    if (queue_descriptor == -1)
    {
        perror("mq_open (queue opening)");
        return NULL;
    }

    ipc_queue_t* queue = malloc(sizeof(ipc_queue_t));
    queue->queue_descriptor = queue_descriptor;
    strcpy(queue->name, name);

    return queue;
}

void close_queue(ipc_queue_t* queue_to_close)
{
    if (mq_close(queue_to_close->queue_descriptor) == -1)
    {
        perror("mq_close");
    }
}

void remove_queue(ipc_queue_t* queue_to_remove)
{
    if (mq_unlink(queue_to_remove->name) == -1)
    {
        perror("mq_unlink");
        return;
    }

    free(queue_to_remove->name);
    free(queue_to_remove);
}

int receive_message(ipc_queue_t* queue, char* buffer, size_t buffer_size, long* type, int block)
{
    unsigned int priority;
    ssize_t bytes_received = mq_receive(queue->queue_descriptor, buffer, buffer_size, &priority);

    *type = priority;

    return bytes_received;
}

int send_message(ipc_queue_t* queue, const char* buffer, long type)
{
    return mq_send(queue->queue_descriptor, buffer, strlen(buffer) + 1, type);
}

int client_send_message(ipc_queue_t* server_queue, long client_id, const char* buffer, long type)
{
    char msg[MSG_MAX_SIZE];
    msg[0] = (char) type;
    msg[1] = (char) client_id;
    strcpy(msg + 2, buffer);
    
    return mq_send(server_queue->queue_descriptor, buffer, strlen(buffer) + 1, type);
}

int client_receive_message(ipc_queue_t* client_queue, long client_id, char* buffer, size_t buffer_size, long* type, int block)
{
    char msg[MSG_MAX_SIZE];
    ssize_t bytes_read = mq_receive(client_queue->queue_descriptor, msg, MSG_MAX_SIZE, NULL);

    *type = msg[0];
    strcpy(buffer, msg + 2);

    return bytes_read;
}

int server_receive_client_message(ipc_queue_t* server_queue, long* client_id, char* buffer, size_t buffer_size, long* type, int block)
{
    char msg[MSG_MAX_SIZE];
    ssize_t bytes_read = mq_receive(server_queue->queue_descriptor, msg, MSG_MAX_SIZE, NULL);

    *type = msg[0];
    *client_id = msg[1];
    strncpy(buffer, msg + 2, buffer_size);

    return bytes_read;
}

