#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "semaphore.h"
#include "tape.h"

semaphore_t* tape_count;
semaphore_t* is_package;
// semaphore_t* tape_load;
semaphore_t* truck_ready;

void trucker(int X)
{
    int count = 0;
    while(1)
    {
        sem_wait_one(is_package);
        int package_mass = tape_get_package(); 
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