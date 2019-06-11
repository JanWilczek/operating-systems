#include "server_stream_networking.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <signal.h>

/*********** GLOBAL VARIABLES *****************/
int shut_server;
/**********************************************/

/*********** CLIENT HELPER FUNCTIONS *****************/
void free_client(struct client_data *client)
{
    free(client->name);
    // free(client->address);
    free(client);
}
/*****************************************************/

/*********** COMMAND HANDLING FUNCTIONS *****************/
// void handle_register(int client_sockfd, const char *name, struct sockaddr *client_address, socklen_t address_size, struct client_data **clients)
void handle_register(struct server_data* server, int client_sockfd)
{
    // Retrieve client's name
    const int NAME_SIZE = 1024;
    char * name = malloc(NAME_SIZE * sizeof(char));
    char* name_helper = name;
    int ret;
    char buffer[BUFFER_SIZE];
    while((ret = recv(client_sockfd, buffer, BUFFER_SIZE, MSG_DONTWAIT)) != -1 && ret != 0 && strncmp(buffer, END, BUFFER_SIZE) != 0)
    {
        if (ret == 0)
        {
            break;
        }

        strncpy(name_helper, buffer, BUFFER_SIZE);
        name_helper += ret;
    }
    name_helper[0] = 0;

    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (server->clients[i] != NULL && strcmp(server->clients[i]->name, name) == 0)
        {
            const char *msg = REGISTERDENIED;
            write(client_sockfd, msg, BUFFER_SIZE);
            return;
        }
    }

    int first_free_id;
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (server->clients[i] == NULL)
        {
            first_free_id = i;
            break;
        }
    }

    struct client_data *client = malloc(sizeof(struct client_data));
    client->name = malloc(strlen(name) + 1);
    strncpy(client->name, name, strlen(name) + 1);
    client->sockfd = client_sockfd;
    server->clients[first_free_id] = client;

    // Set up client's socket descriptor for monitoring
    struct epoll_event event_options;
    event_options.events = EPOLLIN | EPOLLOUT | EPOLLRDHUP;
    event_options.data.fd = client->sockfd;
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, client->sockfd, &event_options) == -1)
    {
        perror("epoll");
    }
    else
    {
        printf("Successfully registered client at id %d.\n", first_free_id);
    }
}

// void handle_unregister(int client_sockfd, const char *name, struct sockaddr *client_address, socklen_t address_size, struct client_data **clients)
void handle_unregister(int client_sockfd, struct client_data **clients)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (clients[i] != NULL && clients[i]->sockfd == client_sockfd)
        {
            printf("Successfully unregistered client with name %s.\n", clients[i]->name);
            free_client(clients[i]);
            clients[i] = NULL;
            return;
        }
    }

    fprintf(stderr, "Cannot unregister not registered client with socket_fd %d.\n", client_sockfd);
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
    if ((socket_descriptor = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
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

int server_start_up(const char *socket_path, struct server_data* server)
{
    // Necessary for early process termination
    unlink(socket_path);

    server->sockfd = start_up(socket_path);
    server->epoll_fd = epoll_create1(0);

    return 0;
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

void server_shut_down(struct server_data* server, const char *socket_path)
{
    // Shut down server connection
    shut_down(server->sockfd);

    // Remove created socket
    unlink(socket_path);
}

void handle_event(struct server_data* server, struct epoll_event* event)
{

}

void check_sockets(struct server_data* server)
{
    const int MAX_EVENTS = 20;
    struct epoll_event events_registered[MAX_EVENTS];

    int num_events;
    while ((num_events = epoll_wait(server->epoll_fd, (struct epoll_event*) events_registered, MAX_EVENTS, 0)) != 0)
    {
        if (num_events == -1)
        {
            perror("epoll_wait");
            return;
        }

        // TODO: Handle registered events
        for (int i = 0; i < num_events; ++i)
        {
            handle_event(server, &events_registered[i]);
        }
    }
}

void server_main_loop(struct server_data* server)
{
    while(!shut_server)
    {
        // Accept incoming connections
        int client_sockfd;
        struct sockaddr client_address;
        socklen_t address_size;
        // if ((client_sockfd = accept(server->sockfd, (struct sockaddr *) &client_address, &address_size)) == -1)
        if ((client_sockfd = accept(server->sockfd, NULL, NULL)) == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            check_sockets(server);
        }
        else
        {
            handle_register(server, client_sockfd);
        }

        // if (!queue_is_empty(work_queue)
        // {
        //     dispatch_work();
        // }
    }
}

void server_main_loop1(struct server_data* server)
{
    while (!shut_server)
    {
        // Accept incoming connections
        int client_sockfd;
        struct sockaddr client_address;
        socklen_t address_size;
        if ((client_sockfd = accept(server->sockfd, (struct sockaddr *) &client_address, &address_size)) == -1)
        // if ((client_sockfd = accept(socket_descriptor, NULL, NULL)) == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("accept");
                exit(EXIT_FAILURE);
            }
            /* Check if no input is coming from clients */
        }
        /* else
        {
            handle_new_client();
        }*/
        /* Check if there is work to dispatch
            continue loop
            */

        // printf("Accepted client with address %s.\n", client_address.sa_data);

        char buffer[BUFFER_SIZE];
        ssize_t ret;

        while (!shut_server && client_sockfd != -1)
        {
            // struct sockaddr client_recv_address;
            // socklen_t recv_address_size;

            // Wait for next data packet
            // ret = recvfrom(client_sockfd, (void *)buffer, BUFFER_SIZE, 0, &client_recv_address, &recv_address_size);
            ret = recv(client_sockfd, buffer, BUFFER_SIZE, MSG_DONTWAIT);
            if (ret == -1)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                break;
            }

            // Ensure the buffer is null-terminated
            buffer[BUFFER_SIZE - 1] = 0;

            // For debugging purposes
            // printf("Received from client %s: %s", client_recv_address.sa_data, buffer);

            // Handle incoming commands
            if (strncmp(buffer, REGISTER, BUFFER_SIZE) == 0)
            {
                // handle_register(client_sockfd, ((char*)buffer) + strlen(REGISTER) + 1, &client_recv_address, recv_address_size, clients);
                handle_register(server, client_sockfd);
            }
            else if (strncmp(buffer, UNREGISTER, BUFFER_SIZE) == 0)
            {
                // handle_unregister(client_sockfd, ((char*) buffer) + strlen(UNREGISTER) + 1, &client_recv_address, recv_address_size, clients);
                handle_unregister(client_sockfd, server->clients);
            }
        }
    }
}

// void server_open_connection(int port_number, const char *socket_path)
// {
//     int socket_descriptor = server_start_up(socket_path);

// }
