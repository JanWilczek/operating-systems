#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "thread_safe_queue.h"

void queue_init(thread_safe_queue_t *queue, int size)
{
    size_t size_in_bytes = size * sizeof(char*);

    queue->entries = malloc(size_in_bytes);
    queue->size = size;
    queue->first_id = -1;
    queue->last_id = -1;

    pthread_mutex_init(&queue->mutex, NULL);
}

int try_put_to_queue(thread_safe_queue_t * qinfo, char *element)
{
    pthread_mutex_lock(&qinfo->mutex);

    int new_id = (qinfo->first_id + 1) % qinfo->size;
    if (new_id == qinfo->last_id)
    {
        // Error: queue is full
        fprintf(stderr, "Cannot put to a full queue!\n");
        return -1;
    }

    // Copy the string to queue
    qinfo->entries[new_id] = malloc((strlen(element) + 1) * sizeof(char));
    strcpy(qinfo->entries[new_id], element);

    // if no elements were in the queue
    if (qinfo->first_id == -1 && qinfo->last_id == -1)
    {
        qinfo->last_id = new_id;
    }
    qinfo->first_id = new_id;

    pthread_mutex_unlock(&qinfo->mutex);

    return 0;
}

int try_get_from_queue(thread_safe_queue_t *qinfo, char **element)
{
    pthread_mutex_lock(&qinfo->mutex);

    if (qinfo->first_id == -1 && qinfo->last_id == -1)
    {
        // Error, the queue is empty
        fprintf(stderr, "Cannot get from empty queue!\n");
        pthread_mutex_unlock(&qinfo->mutex);
        *element = NULL;
        return -1;
    }

    *element = malloc((strlen(qinfo->entries[qinfo->last_id]) + 1) * sizeof(char));
    strcpy(*element, qinfo->entries[qinfo->last_id]);
    free(qinfo->entries[qinfo->last_id]);

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

    pthread_mutex_unlock(&qinfo->mutex);

    return 0;
}

int queue_size(thread_safe_queue_t *qinfo)
{
    int result;

    pthread_mutex_lock(&qinfo->mutex);

    if (qinfo->first_id == -1 && qinfo->last_id == -1)
    {
        result = 0;
    }
    else if (qinfo->first_id >= qinfo->last_id)
    {
        result = qinfo->first_id - qinfo->last_id + 1;
    }
    else
    {
        result = qinfo->size - (qinfo->last_id - qinfo->first_id) + 1;
    }

    pthread_mutex_unlock(&qinfo->mutex);

    return result;
}

int queue_capacity_internal(thread_safe_queue_t *qinfo)
{
    return qinfo->size;
}

void queue_close(thread_safe_queue_t * queue)
{
    free(queue->entries);
    pthread_mutex_destroy(&queue->mutex);
}
