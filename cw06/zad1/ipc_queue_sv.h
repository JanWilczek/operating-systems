#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <sys/ipc.h>
#include "queue_common.h"

typedef struct ipc_queue {
    key_t key;
    int id;
} ipc_queue_t;

/**
 * Creates an IPC queue and returns its ID.
 * */
extern ipc_queue_t* create_queue(enum QueueType type);

/**
 * Returns existing IPC queue.
 * */
extern ipc_queue_t* get_queue(key_t key);

/**
 * Removes the queue from the system and frees the given pointer.
 * */
extern void remove_queue(ipc_queue_t* queue_to_remove);

/**
 * Returns 0 on success and -1 on failure. 
 * When -1 is returned and errno is set to ENOMSG (when block is 1) then there is no message of the given type in the queue.
 * */
extern int receive_message(ipc_queue_t* queue, char* buffer, size_t buffer_size, long* type, int block);

extern int send_message(ipc_queue_t* queue, const char* buffer, long type);

/**
 * Sends a message to server_queue containing type, client id and a string.
 * Returns 0 on successful send, -1 if the function fails (with errno set up properly).
 * */
extern int client_send_message(ipc_queue_t* server_queue, long client_id, const char* buffer, long type);
extern int client_receive_message(ipc_queue_t* client_queue, long client_id, char* buffer, size_t buffer_size, long* type, int block);
extern int server_receive_client_message(ipc_queue_t* server_queue, long* client_id, char* buffer, size_t buffer_size, long* type, int block);



#ifdef __cplusplus
}
#endif