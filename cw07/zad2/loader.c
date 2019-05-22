#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include "semaphore.h"
#include "tape.h"
#include "shared_resources.h"
#include "utils.h"

semaphore_t* truck_ready;
semaphore_t* tape_count;
semaphore_t* is_package;

void print_usage(const char* program_name)
{
    fprintf(stderr, "Usage:     %s N [C]\n"
                    "   N       weight of the packages from this laoder\n"
                    "   C       number of packages to give (0 or lack of\n"
                    "           argument means an infinite number of packages\n", program_name);
}

void print_loader_message(const char* message)
{
    char buffer[100];
    sprintf(buffer, "Loader %d:", getpid());
    print_message(message, buffer);
}

void sigint_handler(int signum)
{
    if (truck_ready)
    {
        sem_close(truck_ready);
        // free(truck_ready);
        truck_ready = NULL;
    }

    if (tape_count)
    {
        sem_close(tape_count);
        // free(tape_count);
        tape_count = NULL;
    }

    if (is_package)
    {
        sem_close(is_package);
        // free(is_package);
        is_package = NULL;
    }

    exit(EXIT_SUCCESS);
}

void loader_loop(int N)
{
    print_loader_message("Waiting for a place on the tape.");
    sem_wait_one(tape_count);       // wait for spot on the tape
    sem_wait_one(truck_ready);      // wait for trucker to be available

    int err = 0;
    while ((err = tape_put_package(N)) != 0)
    {
        if (err == -2)
        {
            sem_signal_one(tape_count);
            sem_signal_one(truck_ready);
            raise(SIGINT);
        }
    }

    sem_signal_one(is_package);     // signal that there is a package
    
    char buffer[100];
    sprintf(buffer, "Put package of weight %d on the tape.", N);
    print_loader_message(buffer);
}

void loader(int N, int C)
{
    truck_ready = sem_get(SEM_TRUCK_READY);
    tape_count = sem_get(SEM_TAPE_COUNT);
    is_package = sem_get(SEM_IS_PACKAGE);

    if (!truck_ready || !tape_count || !is_package)
    {
        fprintf(stderr, "Worker: Could not get semaphores properly. Exiting.\n"
                        "Hint: Try starting trucker program first.\n");
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

    if (argc > 3 || argc == 1)
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
