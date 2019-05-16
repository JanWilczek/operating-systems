#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include "semaphore.h"
#include "tape.h"
#include "shared_resources.h"

semaphore_t* truck_ready;
semaphore_t* tape_count;
// semaphore_t* tape_load;
semaphore_t* is_package;

void print_usage(const char* program_name)
{
    fprintf(stderr, "Usage:     %s N [C]\n"
                    "   N       weight of the packages from this laoder\n"
                    "   C       number of packages to give (0 or lack of"
                    "           argument means an infinite number of packages\n", program_name);
}

void sigint_handler(int signum)
{
    if (truck_ready)
    {
        free(truck_ready);
        truck_ready = NULL;
    }

    if (tape_count)
    {
        free(tape_count);
        tape_count = NULL;
    }

    // if (tape_load)
    // {

    // }
    if (is_package)
    {
        free(is_package);
        is_package = NULL;
    }

    exit(EXIT_SUCCESS);
}

void loader_loop(int N)
{
    sem_wait_one(truck_ready);      // wait for trucker to be available
    // sem_wait(tapeLoad, N);       // wait for sufficiently small tape load
    sem_wait_one(tape_count);       // wait for spot on the tape
    tape_put_package(N);
    sem_signal_one(is_package);     // signal that there is package
}

void loader(int N, int C)
{
    truck_ready = sem_get(SEM_TRUCK_READY);
    tape_count = sem_get(SEM_TAPE_COUNT);
    // tape_load = sem_get(SEM_TAPE_LOAD);
    is_package = sem_get(SEM_IS_PACKAGE);

    if (!truck_ready || !tape_count /* || tape_load*/ || !is_package)
    {
        fprintf(stderr, "Worker: Could not get semaphores properly. Exiting.\n");
        raise(SIGINT);
    }

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

    if (argc > 3 || argc == 0)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    N = atoi(argv[1]);

    if (argc == 3)
    {
        C = atoi(argv[2]);
    }

    // Register SIGINT handler
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) != 0)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }
    
    loader(N, C);

    return EXIT_SUCCESS;
}
