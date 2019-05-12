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



int main(int argc, char* argv[])
{
    return EXIT_SUCCESS;
}
