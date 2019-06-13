#include "server.h"
#include "server_stream_networking.h"
#include "utils.h"

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


/* The server has following duties:
    1. Listen on web and local socket.
    2. Handle client registration.
    3. Parse command line for computation demands.
    4. Dispatch computation to clients.
    5. Receive client's output.
    6. Ping clients.
    7. Monitor multiple descriptors.
*/

/*********** GLOBAL VARIABLES *****************/
int shut_server;
/**********************************************/

void sigint_handler(int num)
{
    shut_server = 1;
}

void *server_monitoring_thread(void *arg)
{
    struct server_data *server = (struct server_data *)arg;

    server_main_loop(server);

    return 0;
}

void command_loop(struct server_data *server)
{
    const int BUF_SIZE = 1024;
    char* buffer = malloc(BUF_SIZE * sizeof(char));

    while (!shut_server)
    {
        // Get file name to count words from
        fgets(buffer, BUF_SIZE, stdin);

        // fgets is blocking; this is in case a SIGINT is received
        if (shut_server)
        {
            break;
        }

        // Remove the newline character at the end
        buffer[strlen(buffer) - 1] = '\0';

        if (!is_file(buffer))
        {
            fprintf(stderr, "Given filename is not a valid text file.\n");
            continue;
        }

        // Add it to the work queue
        try_put_to_queue(&server->queue, buffer);

        usleep(10000);
    }

    free(buffer);
}

void run_server(int port_number, char *socket_path)
{
    // Set up SIGINT handler
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    // Set up appropriate flag
    shut_server = 0;

    // Create a structure for clients' data
    struct client_data *clients[MAX_CONNECTIONS];
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        clients[i] = NULL;
    }

    struct server_data server;
    server.clients = (struct client_data **)&clients;
    queue_init(&server.queue, TASK_QUEUE_SIZE);
    server.tasks_assigned = 0;

    // Open socket for connection
    server_start_up(socket_path, &server);

    // Start client-monitoring thread
    pthread_t monitoring_thread_id;
    int ret;
    if ((ret = pthread_create(&monitoring_thread_id, NULL, server_monitoring_thread, (void *)&server)) != 0)
    {
        fprintf(stderr, "pthread_create: %s\n", strerror(ret));
        exit(EXIT_FAILURE);
    }

    // Start command parsing and computation dispatching thread
    command_loop(&server);

    // Wait for threads to join
    if ((ret = pthread_join(monitoring_thread_id, NULL)) != 0)
    {
        fprintf(stderr, "pthread_join: %s\n", strerror(ret));
    }

    // Shut the server down
    server_shut_down(&server, socket_path);
}
