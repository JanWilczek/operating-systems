#include <stdlib.h>
#include <stdio.h>
#include "ipc_queue_sv.h"
#include "queue_common.h"
#include <unistd.h>
#include <signal.h>

ipc_queue_t* server_queue;


void server_exit(void)
{
    if (server_queue)
    {
        // Send server stop message to clients

        // Wait for all clients to stop

        // Shut down the server queue
        remove_queue(server_queue);
    }
}

void sigint_handler(int signum)
{
    server_exit();
    exit(EXIT_SUCCESS);
}


int main(int argc, char* argv[])
{
    server_queue = NULL;

    // Establish a SIGINT handler
    struct sigaction handler;
    handler.sa_handler = sigint_handler;
    sigemptyset(&handler.sa_mask);
    handler.sa_flags = 0;
    if (sigaction(SIGINT, &handler, NULL) != 0)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // 1. Create new queue (for client -> server communication)
    server_queue = create_queue(SERVER_QUEUE);
    printf("Queue ID is %d.\n", server_queue->id);

    // 2. Receive and respond to messages
    while (1)
    {
        sleep(2);
    }

    return EXIT_SUCCESS;
}