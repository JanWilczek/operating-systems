#include <stdlib.h>
#include "semaphore.h"
#include "shared_queue.h"
#include "shared_resources.h"

semaphore_t* queue_sem;
semaphore_t* tape_count_tape;   // new name not to pollute the global namespace
// semaphore_t* is_package;

void tape_init(int K)
{
    queue_sem = sem_init(SEM_QUEUE, 1);
    tape_count_tape = sem_get(SEM_TAPE_COUNT);
    // is_package = sem_get(SEM_IS_PACKAGE);
    queue_init(K);
}

void tape_put_package(int N)
{
    sem_wait_one(queue_sem);
    put_to_queue(N);    // operation on shared memory, synchronized through queue_sem

    // if (queue_size() < queue_capacity())
    // {
    //     sem_signal_one(tape_count);
    // }

    sem_signal_one(queue_sem);
    // sem_signal_one(is_package);
}

int tape_get_package()
{
    sem_wait_one(queue_sem);        // lock "mutex"
    int N = get_from_queue();   // operation on shared memory, synchronized through queue_sem
    sem_signal_one(tape_count_tape);
    // sem_signal(tape_load, N);
    sem_signal_one(queue_sem);      // unlock "mutex"
    return N;
}

void tape_close(void)
{
    sem_wait_one(queue_sem);
    sem_remove(queue_sem);
    free(tape_count_tape);
    queue_close();
}
