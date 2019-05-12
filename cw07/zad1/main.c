#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "semaphore.h"
#include "shared_queue.h"

semaphore_t* queue_sem;
semaphore_t* tape_count;
semaphore_t* is_package;
// semaphore_t* tape_load;
semaphore_t* truck_ready;

void put_package(int N)
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

int get_package()
{
    sem_wait_one(queue_sem);        // lock "mutex"
    int N = get_from_queue();   // operation on shared memory, synchronized through queue_sem
    if (queue_size() < queue_capacity())
    {
        sem_signal_one(tape_count);
    }
    // sem_signal(tape_load, N);
    sem_signal_one(queue_sem);      // unlock "mutex"
    return N;
}

void worker_loop(int N)
{
    sem_wait_one(truck_ready);          // wait for trucker to be available
    // sem_wait(tapeLoad, N);   // wait for sufficiently small tape load
    sem_wait_one(tape_count);       // wait for spot on the tape
    put_package(N);
}

void worker(int N, int C)
{
    if (C == 0)
    {
        while (1)
        {
            worker_loop(N);
        }
    }
    else
    {
        while (C--)
        {
            worker_loop(N);
        }
    }
}

void trucker(int X)
{
    int count = 0;
    while(1)
    {
        sem_wait_one(is_package);
        int package_mass = get_package(); 
        count++;    // if X means mass then it should be `count += package_mass`. We assume that X stands for package count.

        if (count > X)
        {
            // ERROR
            fprintf(stderr, "Fatal error: truck overloaded.\n");
            exit(EXIT_FAILURE);
        }

        if (count == X)
        {
            // truck full
            printf("Unloading truck.\n");
            sleep(1);
            count = 0;
        }

        // Signal that truck is ready for loading
        sem_signal_one(truck_ready); 
    }
}

int main(int argc, char* argv[])
{
    return EXIT_SUCCESS;
}
