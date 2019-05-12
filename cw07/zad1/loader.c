#include <stdlib.h>
#include <stdio.h>
#include "semaphore.h"
#include "tape.h"

semaphore_t* truck_ready;
semaphore_t* tape_count;
// semaphore_t* tape_load;

void print_usage(const char* program_name)
{
    fprintf(stderr, "Usage:     %s N [C]\n"
                    "   N       weight of the packages from this laoder\n"
                    "   C       number of packages to give (0 or lack of"
                    "           argument means an infinite number of packages\n", program_name);
}

void loader_loop(int N)
{
    sem_wait_one(truck_ready);      // wait for trucker to be available
    // sem_wait(tapeLoad, N);       // wait for sufficiently small tape load
    sem_wait_one(tape_count);       // wait for spot on the tape
    tape_put_package(N);
}

void loader(int N, int C)
{
    // TODO: get all needed semaphores

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
    int C = 0;
    int N;

    if (argc > 2 || argc == 0)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    N = atoi(argv[0]);

    if (argc == 2)
    {
        C = atoi(argv[1]);
    }
    
    loader(N, C);

    return EXIT_SUCCESS;
}
