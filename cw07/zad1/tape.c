#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "semaphore.h"
#include "shared_queue.h"
#include "utils.h"

void print_tape_message(const char* message)
{
    char buffer[200];
    sprintf(buffer, "Current load: %d/%d (%d/%d units).", queue_size(), queue_capacity(), queue_units_sum(), queue_max_units());
    print_message(buffer, message); // preamble and message are switched on purpose, to print the load information last.
}

void tape_init(int K, int M)
{
    semaphore_t* queue_sem = sem_init(SEM_QUEUE, 1);
    free(queue_sem);
    queue_init(K, M);
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

    print_tape_message("Package put on tape."); // This has to be synchronized to be true

    if (queue_units_sum() > queue_max_units())
    {
        // ERROR
        fprintf(stderr, "Fatal error: Too many units on tape.\n");
        sem_signal_one(queue_sem);
        kill(getppid(), SIGINT);
    }

    sem_signal_one(queue_sem);
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

    print_tape_message("Package taken from tape."); // This has to be synchronized to be true
    sem_signal_one(queue_sem);      // unlock "mutex"
    free(queue_sem);

    return qe;
}

void tape_close(void)
{
    semaphore_t* queue_sem = sem_get(SEM_QUEUE);
    // sem_wait_one(queue_sem);
    sem_remove(queue_sem);
    queue_close();
}
