#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "queue_common.h"
#include "ipc_queue_sv.h"

ipc_queue_t *queue;
ipc_queue_t *server_queue;

void client_exit(void)
{
    if (queue)
    {
        remove_queue(queue);
        queue = NULL;
    }

    if (server_queue)
    {
        free(server_queue);
        server_queue = NULL;
    }
}

void sigint_handler(int signum)
{
    client_exit();
    exit(EXIT_SUCCESS);
}

void print_command_usage(void)
{
    fprintf(stderr, "Invalid command!\n");
}

void parse_and_interpret_command(char* buffer, int buffer_size)
{
    // Change newline character to space for parsing ease
    buffer[strlen(buffer) - 1] = ' ';

    char* token = strtok(buffer, " ");
    
    if (token != NULL)
    {
        if (strcasecmp(token, "list") == 0)
        {
            printf("List command.\n");
        }
        else if (strcasecmp(token, "echo") == 0)
        {
            printf("Echo command.\n");
        }
        else if (strcasecmp(token, "friends") == 0)
        {
            printf("Friends command.\n");
        }
        else if (strcasecmp(token, "2all") == 0)
        {
            printf("2All command.\n");
        }
        else if (strcasecmp(token, "2friends") == 0)
        {
            printf("2Friends command.\n");
        }
        else if (strcasecmp(token, "2one") == 0)
        {
            printf("2One command.\n");
        }
        else if (strcasecmp(token, "stop") == 0)
        {
            printf("Stop command.\n");
        }
        else
        {
            print_command_usage();
        }
    }
    else
    {
        print_command_usage();
    }
}

void client_loop(void)
{
    const int BUF_SIZE = 1024;
    char buffer[BUF_SIZE];

    while (1)
    {
        fgets(buffer, sizeof(buffer), stdin);
        parse_and_interpret_command(buffer, BUF_SIZE);
        sleep(1);
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
    if (sigaction(SIGINT, &handler, NULL) != 0)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // 1. Create queue with unique IPC key
    if ((queue = create_queue(CLIENT_QUEUE)) == NULL)
    {
        fprintf(stderr, "Client: could not create a queue.\n");
        exit(EXIT_FAILURE);
    }

    // 2.a. Get server queue reference
    server_queue = get_queue(ftok(getenv("HOME"), SERVER_QUEUE_PROJ_ID));

    // 2.b. Send the key to the server
    char buffer[MSG_MAX_SIZE];
    sprintf(buffer, "%d", queue->key);
    if (send_message(server_queue, buffer, INIT) == -1)
    {
        perror("send_message (client init)");
        exit(EXIT_FAILURE);
    }

    // 3. Receive client ID
    memset(buffer, 0, sizeof(buffer));  // clear the buffer from previous messages
    long type;
    if (receive_message(queue, buffer, sizeof(buffer), &type) == -1)
    {
        perror("receive_message (client init)");
        exit(EXIT_FAILURE);
    }
    if (type != INIT)
    {
        fprintf(stderr, "Invalid client-server communication.\n");
        exit(EXIT_FAILURE);
    }
    long client_id = atol(buffer);
    printf("Client: My ID is %ld.\n", client_id);\

    // 4. Send requests in a loop
    client_loop();

    return EXIT_SUCCESS;
}