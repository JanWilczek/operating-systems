#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include "semaphore.h"
#include "shared_queue.h"

void tape_init(int K)
{
    semaphore_t* queue_sem = sem_init(SEM_QUEUE, 1);
    free(queue_sem);
    queue_init(K);
}

void tape_put_package(int N)
{
    semaphore_t* queue_sem = sem_get(SEM_QUEUE);
    sem_wait_one(queue_sem);

    struct queue_entry qe;
    struct timespec spec;
    clock_gettime(CLOCK_REALTIME, &spec);
    qe.loader_id = getpid();
    qe.tv_sec = spec.tv_sec;
    qe.tv_nsec = spec.tv_nsec;
    qe.package_weight = N;

    put_to_queue(&qe);    // operation on shared memory, synchronized through queue_sem

    // if (queue_size() < queue_capacity())
    // {
    //     sem_signal_one(tape_count);
    // }

    sem_signal_one(queue_sem);
    // sem_signal_one(is_package);
    free(queue_sem);
}

struct queue_entry* tape_get_package(void)
{
    semaphore_t* queue_sem = sem_get(SEM_QUEUE);
    sem_wait_one(queue_sem);        // lock "mutex"

    struct queue_entry* qe = malloc(sizeof(struct queue_entry));
    if (get_from_queue(qe) == -1)   // operation on sha\nred memory, synchronized through queue_sem
    {
        // Leave the queue blocked - this should mean end of execution.
        printf("Tape blocked.\n");
        free(qe);
        return NULL;
    }

    semaphore_t* tape_count_tape = sem_get(SEM_TAPE_COUNT);
    sem_signal_one(tape_count_tape);
    free(tape_count_tape);

    // sem_signal(tape_load, N);
    sem_signal_one(queue_sem);      // unlock "mutex"
    free(queue_sem);
    return qe;
}

void tape_close(void)
{
    // semaphore_t* queue_sem = sem_get(SEM_QUEUE);
    // sem_wait_one(queue_sem);
    sem_remove(queue_sem);
    queue_close();
}
