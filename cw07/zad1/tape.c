#include "semaphore.h"
#include "shared_queue.h"

semaphore_t* queue_sem;
semaphore_t* tape_count;
semaphore_t* is_package;

void tape_init(int M, int K)
{

}

void tape_put_package(int N)
{
    sem_wait_one(queue_sem);
    put_to_queue(N);    // operation on shared memory, synchronized through queue_sem

    if (queue_size() < queue_capacity())
    {
        sem_signal_one(tape_count);
    }

    sem_signal_one(queue_sem);
    sem_signal_one(is_package);
}

int tape_get_package()
{
    sem_wait_one(queue_sem);        // lock "mutex"
    int N = get_from_queue();   // operation on shared memory, synchronized through queue_sem
    sem_signal_one(tape_count);
    // sem_signal(tape_load, N);
    sem_signal_one(queue_sem);      // unlock "mutex"
    return N;
}
