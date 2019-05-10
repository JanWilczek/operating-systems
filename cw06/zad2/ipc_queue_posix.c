#include "ipc_queue_posix.h"
#include <string.h>
#include <sys/times.h>
#include <stdio.h>
#include <fcntl.h>


ipc_queue_t* create_queue(enum QueueType type)
{
    char* name = malloc(255 * sizeof(char));
    int proj_id;
    switch(type)
    {
        case SERVER_QUEUE:
            snprintf(name, 255, "%s", SERVER_NAME);
            break;
        case CLIENT_QUEUE:
            srand(times(NULL));
            proj_id = (int) 3 * MAX_CLIENTS * ((float) rand() / (float) RAND_MAX) + 1;
            snprintf(name, 255, "/client_ipc_queue_%d", proj_id);
            break;
        default:
            return NULL;
    }

    int flags = O_RDWR | O_CREAT;
    if (type == CLIENT_QUEUE)
    {
        flags |= O_EXCL;
    }

    mqd_t queue_descriptor = mq_open(name, flags, 0700, NULL);
    if (queue_descriptor == -1)
    {
        perror("mq_open (queue creation)");
        free(name);
        return NULL;
    }

    // Set queue properties
    struct mq_attr attributes;
    attributes.mq_maxmsg = 20;
    attributes.mq_msgsize = MSG_MAX_SIZE;
    struct mq_attr old;
    if (mq_setattr(queue_descriptor, &attributes, &old) == -1)
    {
        perror("mq_setattr");
        free(name);
        return NULL;
    }
    // printf("Message size was %ld now is %ld\n", old.mq_msgsize, attributes.mq_msgsize);

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
    queue->name = malloc(255 * sizeof(char));
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

    *type = (long) priority;

    return bytes_received;
}

int send_message(ipc_queue_t* queue, const char* buffer, long type)
{
    return mq_send(queue->queue_descriptor, buffer, strlen(buffer) + 1, type);
}

int client_send_message(ipc_queue_t* server_queue, long client_id, const char* buffer, long type)
{
    char msg[MSG_MAX_SIZE];
    msg[0] = (char) (type % 128);
    msg[1] = (char) (client_id % 128);
    strcpy(msg + 2, buffer);

    // printf("Client sends message: %d %d %s\n", msg[0], msg[1], msg + 2);
    
    return mq_send(server_queue->queue_descriptor, msg, strlen(msg) + 1, 1);
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

    if (bytes_read != -1)
    {
        char chr_type = msg[0];
        *type = (long) chr_type;
        char chr_id = msg[1];
        *client_id = (long) chr_id;
        strncpy(buffer, msg + 2, MSG_MAX_SIZE - 2);
        
        // printf("Server received message: %d %d %s\n", msg[0], msg[1], msg + 2);
    }

    return bytes_read;
}

