#pragma once

#ifdef __cplusplus
    extern "C" {
#endif

#include <pthread.h>

typedef struct thread_safe_queue
{
    int size;
    int first_id;
    int last_id;
    char** entries;

    pthread_mutex_t mutex;
} thread_safe_queue_t;

extern void queue_init(thread_safe_queue_t *queue, int size);
extern int try_put_to_queue(thread_safe_queue_t *queue, char* element);
extern int try_get_from_queue(thread_safe_queue_t *queue, char** element);
extern int queue_size(thread_safe_queue_t *queue);
extern int queue_capacity(thread_safe_queue_t * queue);
extern void queue_close(thread_safe_queue_t *queue);

#ifdef __cplusplus
    }
#endif
