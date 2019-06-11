#include "server_stream_networking.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

/*********** GLOBAL VARIABLES *****************/
int shut_server;
/**********************************************/

/*********** CLIENT HELPER FUNCTIONS *****************/
void free_client(struct client_data* client)
{
    free(client->name);
    free(client->address);
    free(client);
}
/*****************************************************/

/*********** COMMAND HANDLING FUNCTIONS *****************/
void handle_register(int client_sockfd, const char* name, struct sockaddr* client_address, socklen_t address_size, struct client_data** clients)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (clients[i] != NULL && strcmp(clients[i]->name, name) == 0)
        {
            const char* msg = REGISTERDENIED;
            sendto(client_sockfd, (void *) msg, strlen(msg) + 1, 0, client_address, address_size);
        }        
    }

    int first_free_id;
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (clients[i] == NULL)
        {
            first_free_id = i;
            break;
        }
    }

    struct client_data* client = malloc(sizeof(struct client_data));
    client->name = malloc(strlen(name) + 1);
    strncpy(client->name, name, strlen(name) + 1);
    client->address = malloc(address_size);
    memcpy(client->address, client_address, address_size);

    printf("Successfully registered client at id %d.\n", first_free_id);
}

void handle_unregister(int client_sockfd, const char* name, struct sockaddr* client_address, socklen_t address_size, struct client_data** clients)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (clients[i] != NULL && strcmp(name, clients[i]->name) == 0)
        {
            free_client(clients[i]);
            clients[i] = NULL;
            printf("Successfully unregistered client with name %s.\n", name);
        }
        return;
    }

    fprintf(stderr, "Cannot unregister not registered client with name %s.\n", name);
}
/********************************************************/

void sigint_handler(int num)
{
    shut_server = 1;
}

int start_up(const char *socket_path)
{
    // Set up appropriate flag
    shut_server = 0;

    // Create local socket
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Bind socket to its name
    struct sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strcpy((char *)server_address.sun_path, socket_path);

    if (bind(socket_descriptor, (struct sockaddr *)&server_address, sizeof(struct sockaddr_un)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Listen for client connections
    if (listen(socket_descriptor, MAX_CONNECTIONS) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    // Set up SIGINT handler
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    return socket_descriptor;
}

int server_start_up(const char* socket_path)
{
    // Necessary for early process termination
    unlink(socket_path);

    return start_up(socket_path);
}

void shut_down(int socket_descriptor)
{
    if (shutdown(socket_descriptor, SHUT_RDWR) == -1)
    {
        perror("shutdown");
        exit(EXIT_FAILURE);
    }

    if (close(socket_descriptor) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

void server_shut_down(int socket_descriptor, const char* socket_path)
{
    // Shut down server connection
    shut_down(socket_descriptor);

    // Remove created socket
    unlink(socket_path);
}

void server_main_loop(int socket_descriptor, struct client_data** clients)
{
    while (!shut_server)
    {
        // Accept incoming connections
        int client_sockfd;
        struct sockaddr client_address;
        socklen_t address_size;
        if ((client_sockfd = accept(socket_descriptor, &client_address, &address_size)) == -1)
        // if ((cliend_sockfd = accept(socket_descriptor, NULL, NULL)) == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Accepted client with address %s.\n", client_address.sa_data);

        char buffer[BUFFER_SIZE];
        ssize_t ret;

        while (!shut_server)
        {
            struct sockaddr client_recv_address;
            socklen_t recv_address_size;

            // Wait for next data packet
            // ret = read(client_sockfd, buffer, BUFFER_SIZE);
            ret = recvfrom(client_sockfd, (void *)buffer, BUFFER_SIZE, 0, &client_recv_address, &recv_address_size);
            if (ret == -1)
            {
                perror("recvfrom");
                exit(EXIT_FAILURE);
            }

            // Ensure the buffer is null-terminated
            buffer[BUFFER_SIZE - 1] = 0;
            
            // For debugging purposes
            // printf("Received from client %s: %s", client_recv_address.sa_data, buffer);

            // Handle incoming commands
            if (strncmp(buffer, REGISTER, BUFFER_SIZE) == 0)
            {
                // handle_register(client_sockfd, ((char*)buffer) + strlen(REGISTER) + 1, &client_recv_address, recv_address_size, clients);
            }

            if (strncmp(buffer, UNREGISTER, BUFFER_SIZE) == 0)
            {
                // handle_unregister(client_sockfd, ((char*) buffer) + strlen(UNREGISTER) + 1, &client_recv_address, recv_address_size, clients);
            }
        }
    }
}

// void server_open_connection(int port_number, const char *socket_path)
// {
//     int socket_descriptor = server_start_up(socket_path);

   

// }