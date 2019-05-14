#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "semaphore.h"
#include "tape.h"

semaphore_t *tape_count;
semaphore_t *is_package;
// semaphore_t* tape_load;
semaphore_t *truck_ready;

void sigint_handler(int signum)
{
    if (signum == SIGINT)
    {
        if (tape_count)
        {
            sem_remove(tape_count);
            tape_count = NULL;
        }

        if (is_package)
        {
            sem_remove(is_package);
            is_package = NULL;
        }

        if (truck_ready)
        {
            sem_remove(truck_ready);
            truck_ready = NULL;
        }
    }
}

void trucker_loop(int X)
{
    int count = 0;
    while (1)
    {
        sem_wait_one(is_package);
        int package_mass = tape_get_package();
        count++; // if X means mass then it should be `count += package_mass`. We assume that X stands for package count.

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

void trucker(int X, int K, int M)
{
    truck_ready = sem_init("~/truck_read", X);
    tape_count = sem_init("~/tape_count", K);
    is_package = sem_init("~/is_package", 0);

    trucker_loop(X);
}

int main(int argc, char *argv[])
{
    tape_count = NULL;
    is_package = NULL;
    truck_ready = NULL;

    // Register SIGINT handler
    struct sigaction sa;
    sa.handler = sigint_handler;
    sigemptyset(&sa.mask);
    sa.flags = 0;
    if (sigaction(SIGINT, &sa, NULL) != 0)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // argumentx X, K and M.
    int X, K, M;

    trucker(X, K, M);

    return EXIT_SUCCESS;
}
