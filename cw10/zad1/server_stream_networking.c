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
void handle_register(struct server_data *server, int client_sockfd)
{
    // Retrieve client's name
    const int NAME_SIZE = 1024;
    char *name = malloc(NAME_SIZE * sizeof(char));
    char buffer[BUFFER_SIZE];

    if (recv(client_sockfd, buffer, BUFFER_SIZE, MSG_WAITALL) == -1)
    {
        perror("read");
        return;
    }
    strncpy(name, buffer, BUFFER_SIZE);


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
    event_options.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    event_options.data.fd = client->sockfd;
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, client->sockfd, &event_options) == -1)
    {
        perror("epoll");
    }
    else
    {
        printf("Successfully registered client %s at id %d.\n", name, first_free_id);
    }
}

void handle_unregister(int client_sockfd, struct server_data *server)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (server->clients[i] != NULL && server->clients[i]->sockfd == client_sockfd)
        {
            struct epoll_event event;
            epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, server->clients[i]->sockfd, &event); // event must be non-null due to a bug
            printf("Successfully unregistered client with name %s.\n", server->clients[i]->name);
            free_client(server->clients[i]);
            server->clients[i] = NULL;

            if (close(client_sockfd) == -1)
            {
                perror("close");
            }

            return;
        }
    }

    fprintf(stderr, "Cannot unregister not registered client with socket descriptor %d.\n", client_sockfd);
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

int server_start_up(const char *socket_path, struct server_data *server)
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

void server_shut_down(struct server_data *server, const char *socket_path)
{
    // Shut down server connection
    shut_down(server->sockfd);

    // Remove created socket
    unlink(socket_path);
}

const char *get_client_name(const struct server_data *server, int client_sockfd)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (server->clients[i] != NULL && server->clients[i]->sockfd == client_sockfd)
        {
            return server->clients[i]->name;
        }
    }
    return NULL;
}

void handle_event(struct server_data *server, struct epoll_event *event)
{
    // Handles EPOLLIN and EPOLLRDHUP
    if (event->events & EPOLLRDHUP)
    {
        handle_unregister(event->data.fd, server);
    }
    else if (event->events & EPOLLIN)
    {
        printf("Client %s says:\n", get_client_name(server, event->data.fd));

        int ret;
        char buffer[BUFFER_SIZE];
        while ((ret = recv(event->data.fd, buffer, BUFFER_SIZE, MSG_DONTWAIT)) != 0)
        {
            if (ret == -1)
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    perror("read");
                    exit(EXIT_FAILURE);
                }
                break;
            }

            // Print what the client has said.
            fputs(buffer, stdout);
        }
    }
    else
    {
        fprintf(stderr, "Unused EPOLL event.\n");
    }
}

void check_sockets(struct server_data *server)
{
    const int MAX_EVENTS = 20;
    struct epoll_event events_registered[MAX_EVENTS];

    int num_events;
    while ((num_events = epoll_wait(server->epoll_fd, (struct epoll_event *)events_registered, MAX_EVENTS, 0)) != 0)
    {
        if (num_events == -1)
        {
            perror("epoll_wait");
            return;
        }

        for (int i = 0; i < num_events; ++i)
        {
            handle_event(server, &events_registered[i]);
        }
    }
}

void server_main_loop(struct server_data *server)
{
    while (!shut_server)
    {
        // Accept incoming connections
        int client_sockfd;
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
            // A new client connected
            handle_register(server, client_sockfd);
        }

        // Check for work to dispatch
        // if (!queue_is_empty(work_queue)
        // {
        //     dispatch_work();
        // }
    }
}
