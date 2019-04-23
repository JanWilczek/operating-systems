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

extern int receive_message(ipc_queue_t* queue, char* buffer, size_t buffer_size, long* type);

extern int send_message(ipc_queue_t* queue, char* buffer, long type);

#ifdef __cplusplus
}
#endif