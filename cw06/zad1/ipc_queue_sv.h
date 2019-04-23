#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/ipc.h>
#include "queue_common.h"

typedef struct ipc_queue {
    key_t key;
    int id;
} ipc_queue_t;

/**
 * Creates an IPC queue and returns its ID.
 * */
ipc_queue_t* create_queue(enum QueueType type);

/**
 * Returns existing IPC queue.
 * */
ipc_queue_t* get_queue(key_t key);

/**
 * Removes the queue from the system and frees the given pointer.
 * */
void remove_queue(ipc_queue_t* queue_to_remove);

#ifdef __cplusplus
}
#endif