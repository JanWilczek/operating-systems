#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include "queue_common.h"
#include "ipc_queue_sv.h"

ipc_queue_t *queue;
ipc_queue_t *server_queue;
long client_id;

//************** COMMANDS SENT TO THE SERVER **************
void send_stop()
{
    if (client_send_message(server_queue, client_id, "", STOP) == -1)
    {
        perror("client_send_message");
    }
}

//************** END OF COMMANDS SENT TO THE SERVER ************** 

void client_exit(void)
{
    send_stop();

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
            exit(EXIT_SUCCESS); // this should automatically call client_exit
        }
        else if (strcasecmp(token, "read") == 0)
        {
            printf("Read command\n");
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
    const int BUF_SIZE = MSG_MAX_SIZE;
    char buffer[BUF_SIZE];
    long type;

    while (1)
    {
        // Check if server has stopped
        int err = receive_message(queue, buffer, BUF_SIZE, &type, 0);
        if (err == 0 && type == STOP)
        {
            return; // STOP message from the server - end the client loop
        }

        // Get user command
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
    client_id = -1;

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
    if (client_send_message(server_queue, -1, buffer, INIT) == -1)
    {
        perror("send_message (client init)");
        exit(EXIT_FAILURE);
    }
    // printf("My queue key is %s\n", buffer);

    // 3. Receive client ID
    memset(buffer, 0, sizeof(buffer));  // clear the buffer from previous messages
    long type;
    if (receive_message(queue, buffer, sizeof(buffer), &type, 1) == -1)
    {
        perror("receive_message (client init)");
        exit(EXIT_FAILURE);
    }
    if (type != INIT)
    {
        fprintf(stderr, "Invalid client-server communication.\n");
        exit(EXIT_FAILURE);
    }
    client_id = atol(buffer);
    printf("Client: My ID is %ld.\n", client_id);\

    // 4. Send requests in a loop
    client_loop();

    return EXIT_SUCCESS;
}