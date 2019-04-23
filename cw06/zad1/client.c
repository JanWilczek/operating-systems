#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "queue_common.h"
#include "ipc_queue_sv.h"

ipc_queue_t *queue;
ipc_queue_t *server_queue;

void client_exit(void)
{
    if (queue)
    {
        remove_queue(queue);
    }

    if (server_queue)
    {
        free(server_queue);
    }
}

void sigint_handler(int signum)
{
    if (signum == SIGINT)
    {
        client_exit();
    }
}

int main(int argc, char *argv[])
{
    // Initialize the global variables
    queue = NULL;
    server_queue = NULL;

    // Handle normal process termination
    if (atexit(client_exit) != 0)
    {
        fprintf(stderr, "Cannot set exit function of the client. Terminating.\n");
        exit(EXIT_FAILURE);
    }

    // Handle process interruption by a SIGINT signal
    struct sigaction handler;
    handler.sa_handler = sigint_handler;
    sigemptyset(&handler.sa_mask);
    handler.sa_flags = 0;
    sigaction(SIGINT, &handler, NULL);

    // 1. Create queue with unique IPC key
    queue = create_queue(CLIENT_QUEUE);
    printf("Client queue ID is %d\n", queue->id);

    // 2.a. Get server queue reference
    server_queue = get_queue(ftok(getenv("HOME"), SERVER_QUEUE_PROJ_ID));
    printf("Server queue ID is %d\n", server_queue->id);

    // 2.b. Send the key to the server

    // 3. Receive client ID

    // 4. Send requests in a loop

    return EXIT_SUCCESS;
}