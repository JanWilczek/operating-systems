#include <stdlib.h>
#include <stdio.h>
#include "ipc_queue_sv.h"
#include "queue_common.h"
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <errno.h>

ipc_queue_t *server_queue;
long next_free_id;
ipc_queue_t *client_queues[MAX_CLIENTS + 1];

//************** SERVER COMMANDS HANDLING **************

void handle_init(char *keystring)
{
    key_t key = atoi(keystring);

    ipc_queue_t *client_queue = NULL;
    if ((client_queue = get_queue(key)) == NULL)
    {
        return;
    }

    long client_id = next_free_id++;
    if (client_id > MAX_CLIENTS + 1)
    {
        // Too many clients - denial of service.
        fprintf(stderr, "Cannot register more clients.\n");
        free(client_queue);
        return;
    }

    client_queues[client_id] = client_queue;

    char buffer[MSG_MAX_SIZE];
    sprintf(buffer, "%ld", client_id);
    if (send_message(client_queue, buffer, INIT) == -1)
    {
        perror("send_message (client init on server)");
    }
    else
    {
        printf("Server: Registered client with ID %ld\n", client_id);
    }
}

void handle_stop(long client_id)
{
    free(client_queues[client_id]);
    client_queues[client_id] = NULL;
}

void handle_echo(long client_id, const char* string)
{
    char buffer[MSG_MAX_SIZE];
    time_t t = time(NULL);
    snprintf(buffer, MSG_MAX_SIZE, "%s %s", string, ctime(&t));
    buffer[strlen(buffer) - 1] = '\0';  // remove newline character at the end 

    if (send_message(client_queues[client_id], buffer, ECHO) == -1)
    {
        perror("send_message (ECHO response)");
    }
}

void handle_list(long client_id)
{
    char buffer[MSG_MAX_SIZE] = "Active clients are: ";
    char client_id_string[10];

    for (long i = 0L; i < next_free_id; ++i)
    {
        if (client_queues[i])
        {
            if (i == client_id)
            {
                snprintf(client_id_string, sizeof(client_id_string), "%ld (You), ", i); 
            }
            else
            {
                snprintf(client_id_string, sizeof(client_id_string), "%ld, ", i);
            }
            
            strcat(buffer, client_id_string);
        }
    }
    buffer[strlen(buffer) - 2] = '.';
    buffer[strlen(buffer) - 1] = '\0';

    if (send_message(client_queues[client_id], buffer, LIST) == -1)
    {
        perror("send_message (LIST response)");
    }
}

void handle_to_all(long client_id, const char* string)
{
    // Prepare message
    char buffer[MSG_MAX_SIZE];
    time_t t = time(NULL);
    char* t_string = ctime(&t);
    t_string[strlen(t_string) - 1] = '\0'; // remove newline character at the end
    snprintf(buffer, MSG_MAX_SIZE, "%s Client %ld says: %s", t_string, client_id, string);

    // Send message
    for (long i = 1L; i < next_free_id; ++i)
    {
        if (client_queues[i] && i != client_id)
        {
            if (send_message(client_queues[i], buffer, i) == -1)
            {
                perror("send_message (2ALL response)");
            }
            // printf("Send 2ALL message %s to client %ld\n.", string, i);
        }
    }
}

//************** END OF SERVER COMMANDS HANDLING **************

void server_exit(void)
{
    if (server_queue)
    {
        // Send server stop message to clients and count how many are there
        long to_stop = 0L;
        for (long i = 1L; i < next_free_id; ++i)
        {
            if (client_queues[i])
            {
                ++to_stop;
                send_message(client_queues[i], "", STOP);
            }
        }

        // Wait for all clients to stop
        char buffer[MSG_MAX_SIZE];
        long stopped = 0L;
        while (stopped < to_stop)
        {
            long client_id;
            long type = 0;
            if (server_receive_client_message(server_queue, &client_id, buffer, MSG_MAX_SIZE, &type, 1) == -1)
            {
                perror("server_receive_client_message");
                continue;
            }

            if (type == STOP)
            {
                ++stopped;
                handle_stop(client_id);
            }
        }

        // Shut down the server queue
        remove_queue(server_queue);
    }
}

void sigint_handler(int signum)
{
    server_exit();
    exit(EXIT_SUCCESS);
}

void server_loop(void)
{
    long message_type;
    long client_id;
    char message[MSG_MAX_SIZE];

    while (1)
    {
        // Await all types of messages
        message_type = 0L;

        if (server_receive_client_message(server_queue, &client_id, message, MSG_MAX_SIZE, &message_type, 1) == -1)
        {
            if (errno != ENOMSG)
            {
                perror("server_receive_client_message");
            }
            continue;
        }

        // printf("Received message from client %ld\n", client_id);

        switch (message_type)
        {
        case INIT:
            handle_init(message);
            break;
        case STOP:
            handle_stop(client_id);
            break;
        case ECHO:
            handle_echo(client_id, message);
            break;
        case TOALL:
            handle_to_all(client_id, message);
            break;
        case LIST:
            handle_list(client_id);
            break;
        default:
            fprintf(stderr, "Server: Unknown message type.\n");
        }
    }
}

int main(int argc, char *argv[])
{
    next_free_id = 1L; // numbering of clients starts with 1 (because sent messages should have mtype > 0)
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

    // 2. Receive and respond to messages
    server_loop();

    return EXIT_SUCCESS;
}