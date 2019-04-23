#include <stdlib.h>
#include <stdio.h>
#include "ipc_queue_sv.h"
#include "queue_common.h"
#include <unistd.h>


int main(int argc, char* argv[])
{
    // 1. Create new queue (for client -> server communication)
    ipc_queue_t* queue = create_queue(SERVER_QUEUE);
    printf("Queue ID is %d.\n", queue->id);

    // 1.a. Establish SIGINT handler
    for (int i = 0; i < 10; ++i)
    {
        sleep(2);
    }

    // 2. Receive and respond to messages

    remove_queue(queue);
    return EXIT_SUCCESS;
}