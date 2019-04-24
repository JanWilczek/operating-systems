#include <stdlib.h>
#include <stdio.h>
#include "ipc_queue_sv.h"
#include "queue_common.h"
#include <unistd.h>
#include <signal.h>
#include <string.h>

ipc_queue_t* server_queue;
long next_free_id;
long client_queue_keys[MAX_CLIENTS];


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

void handle_init(char* keystring)
{
    key_t key = atoi(keystring);
    printf("Server: received keystring is %s which translates to %d.\n", keystring, key);

    ipc_queue_t* client_queue = NULL;
    if ((client_queue = get_queue(key)) == NULL)
    {
        return;
    }

    long client_id = next_free_id++;
    client_queue_keys[client_id] = key;

    char buffer[MSG_MAX_SIZE];
    sprintf(buffer, "%ld", client_id);
    if (send_message(client_queue, buffer, INIT) == -1)
    {
        perror("send_message (client init on server)");
    }
    else 
    {
        printf("Server: Sent message to client with ID %ld\n", client_id);
    }
}

void receive_and_handle_loop(void)
{  
    long message_type;
    char message[MSG_MAX_SIZE];

    while (1)
    {
        receive_message(server_queue, message, MSG_MAX_SIZE, &message_type);

        switch (message_type)
        {
            case INIT:
                handle_init(message);
                break;
        }

        sleep(1);
    }
}

int main(int argc, char* argv[])
{
    next_free_id = 0L;
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
    if ((server_queue = create_queue(SERVER_QUEUE)) == NULL)
    {
        fprintf(stderr, "Client: could not create a queue.\n");
        exit(EXIT_FAILURE);
    }
    printf("Queue ID is %d.\n", server_queue->id);

    // 2. Receive and respond to messages
    receive_and_handle_loop();

    return EXIT_SUCCESS;
}