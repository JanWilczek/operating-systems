#include <stdlib.h>
#include "semaphore.h"
#include "tape.h"

semaphore_t* truck_ready;
semaphore_t* tape_count;
// semaphore_t* tape_load;

void loader_loop(int N)
{
    sem_wait_one(truck_ready);      // wait for trucker to be available
    // sem_wait(tapeLoad, N);       // wait for sufficiently small tape load
    sem_wait_one(tape_count);       // wait for spot on the tape
    tape_put_package(N);
}

void loader(int N, int C)
{
    if (C == 0)
    {
        while (1)
        {
            loader_loop(N);
        }
    }
    else
    {
        while (C--)
        {
            loader_loop(N);
        }
    }
}

int main(int argc, char* argv[])
{
    return EXIT_SUCCESS;
}
