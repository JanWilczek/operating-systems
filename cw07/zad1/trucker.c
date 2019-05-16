#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include "semaphore.h"
#include "tape.h"
#include "shared_resources.h"
#include "time_stamp.h"

semaphore_t *tape_count;
semaphore_t *is_package;
// semaphore_t* tape_load;
semaphore_t *truck_ready;

void print_usage(const char* program_name)
{
    fprintf(stderr, "Usage:     %s X K M\n"
                    "   X       number of packages the trucker can load\n"
                    "   K       number of packages the tape can hold\n"
                    "   M       total mass of packages the tape can hold\n", program_name);
}

void sigint_handler(int signum)
{
    if (signum == SIGINT)
    {
        tape_close();

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

    exit(EXIT_SUCCESS);
}

void print_message(const char* message)
{
    char* time_stamp = get_precise_time();
    printf("%s Trucker: %s\n", time_stamp, message);
    free(time_stamp);
}

void trucker_loop(int X)
{
    int count = 0;
    while (1)
    {
        print_message("Truck is waiting for a package.");
        sem_wait_one(is_package);   // wait for package
        int package_mass = tape_get_package();
        count++; // if X means mass then it should be `count += package_mass`. We assume that X stands for package count.

        // TODO: package received message

        if (count > X)
        {
            // ERROR
            fprintf(stderr, "Fatal error: truck overloaded.\n");
            raise(SIGINT);
        }

        if (count == X)
        {
            // truck full
            print_message("End of space. Truck is leaving and unloading.");
            sleep(2);
            count = 0;
            print_message("Empty truck has arrived.");
        }

        // Signal that truck is ready for loading
        sem_signal_one(truck_ready);
    }
}

void trucker(int X, int K, int M)
{
    truck_ready = sem_init(SEM_TRUCK_READY, X);
    // tape_load = sem_init(SEM_TAPE_LOAD, M);
    tape_count = sem_init(SEM_TAPE_COUNT, K);
    is_package = sem_init(SEM_IS_PACKAGE, 0);

    tape_init(K);
    trucker_loop(X);
}

int main(int argc, char *argv[])
{
    tape_count = NULL;
    is_package = NULL;
    truck_ready = NULL;
    
    if (argc != 4)
    {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    int X, K, M;
    X = atoi(argv[1]);
    K = atoi(argv[2]);
    M = atoi(argv[3]);

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

    trucker(X, K, M);

    return EXIT_SUCCESS;
}
