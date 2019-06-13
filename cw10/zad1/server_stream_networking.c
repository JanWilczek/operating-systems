#include "server_stream_networking.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>

/*********** GLOBAL VARIABLES *****************/
extern int shut_server;
/**********************************************/

/*********** CLIENT HELPER FUNCTIONS *****************/
void free_client(struct client_data *client)
{
    free(client->name);
    free(client);
}

int get_client_id(const struct server_data *server, int client_sockfd)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (server->clients[i] != NULL && server->clients[i]->sockfd == client_sockfd)
        {
            return i;
        }
    }
    return -1;
}

const char *get_client_name(const struct server_data *server, int client_sockfd)
{
    int client_id = get_client_id(server, client_sockfd);
    if (client_id == -1)
    {
        return NULL;
    }
    return server->clients[client_id]->name;
}
/*****************************************************/

/*********** COMMAND HANDLING FUNCTIONS *****************/
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
    client->nb_pending_tasks = 0;
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

void handle_result(struct server_data *server, int client_sockfd)
{
    char buffer[BUFFER_SIZE];

    read(client_sockfd, buffer, BUFFER_SIZE);
    int task_id = atoi(buffer);

    printf("Client %s has completed task %d with the following result:\n", get_client_name(server, client_sockfd), task_id);

    // Receive and print the result
    int ret;
    while ((ret = read(client_sockfd, buffer, BUFFER_SIZE)) > 0)
    {
        char* is_end = strstr(buffer, END);
        if (is_end != NULL)
        {
            is_end[0] = '\0';   // print only characters up to is_end[0]
            printf("%s", buffer);
            printf("Read %s. Breaking input.\n", is_end + 1);   // Should display 248END1248
            break;
        }

        printf("%s", buffer);

        fflush(stdout);
        fsync(client_sockfd);
    }

    if (ret == -1)
    {
        perror("read");
    }

    --server->clients[get_client_id(server, client_sockfd)]->nb_pending_tasks;
}

void handle_response(struct server_data *server, int client_sockfd)
{
    char buffer[BUFFER_SIZE];

    // Determine the type of command (currently only RESULT handled)
    read(client_sockfd, buffer, BUFFER_SIZE);

    if (strncmp(buffer, RESULT, BUFFER_SIZE) == 0)
    {
        handle_result(server, client_sockfd);
    }
}
/********************************************************/

int start_up(const char *socket_path)
{
    // Create local socket
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
    // if ((socket_descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
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
    if (listen(socket_descriptor, MAX_CONNECTIONS / 2) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return socket_descriptor;
}

int start_up_inet(int port_number)
{
    // Create local socket
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Bind socket to its name
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    struct in_addr address;
    address.s_addr = INADDR_ANY;
    server_address.sin_addr = address;
    server_address.sin_port = htons(port_number);

    if (bind(socket_descriptor, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in)) == -1)
    {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // Listen for client connections
    if (listen(socket_descriptor, MAX_CONNECTIONS / 2) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return socket_descriptor;
}

int server_start_up(const char *socket_path, struct server_data *server, int port_number)
{
    // Necessary for early process termination
    unlink(socket_path);

    server->sockfd = start_up(socket_path);
    server->inet_sockfd = start_up_inet(port_number);
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
    shut_down(server->inet_sockfd);

    // Remove created socket
    unlink(socket_path);
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
        handle_response(server, event->data.fd);
    }
    else
    {
        fprintf(stderr, "Unused EPOLL event.\n");
    }
}

void check_events(struct server_data *server)
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

/** Returns -1 when no clients are connected. */
int pick_target_client(struct server_data *server)
{
    int target_client_id = -1;
    int min_pending_tasks = TASK_QUEUE_SIZE;

    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (server->clients[i] != NULL && server->clients[i]->nb_pending_tasks < min_pending_tasks)
        {
            target_client_id = i;
            min_pending_tasks = server->clients[i]->nb_pending_tasks;

            if (min_pending_tasks == 0)
            {
                return target_client_id;
            }
        }
    }

    return target_client_id;
}

void assign_task(struct server_data *server, char *filename, int target_client_id)
{
    int task_id = server->tasks_assigned++;
    ++server->clients[target_client_id]->nb_pending_tasks;

    char buffer[BUFFER_SIZE];

    // Send COMPUTE command
    sprintf(buffer, "%s", COMPUTE);
    write(server->clients[target_client_id]->sockfd, buffer, BUFFER_SIZE);
    memset(buffer, 0, BUFFER_SIZE);

    // Send task id
    sprintf(buffer, "%d", task_id);
    write(server->clients[target_client_id]->sockfd, buffer, BUFFER_SIZE);
    memset(buffer, 0, BUFFER_SIZE);

    // Send filename to examine
    sprintf(buffer, "%s", filename);
    write(server->clients[target_client_id]->sockfd, filename, strlen(filename) + 1);
}

void dispatch_work(struct server_data *server)
{
    while (queue_size(&server->queue) > 0)
    {
        char *filename;
        try_get_from_queue(&server->queue, &filename);
        if (filename != NULL)
        {
            int target_client_id = pick_target_client(server);
            if (target_client_id == -1)
            {
                fprintf(stderr, "No clients connected. Cannot dispatch work.\n");
                return;
            }

            assign_task(server, filename, target_client_id);
        }
        else
        {
            break;
        }
    }
}

void examine_socket(struct server_data *server, int server_sockfd)
{
    // Accept incoming connections
    int client_sockfd;
    if ((client_sockfd = accept(server_sockfd, NULL, NULL)) == -1)
    {
        if (errno != EAGAIN && errno != EWOULDBLOCK)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        // A new client connected
        handle_register(server, client_sockfd);
    }
}

void server_main_loop(struct server_data *server)
{
    while (!shut_server)
    {
        // Check local socket
        examine_socket(server, server->sockfd);
        examine_socket(server, server->inet_sockfd);

        // Check for events from clients
        check_events(server);

        // Check for work to dispatch
        if (queue_size(&server->queue) > 0)
        {
            dispatch_work(server);
        }

        // Ping clients every 100 iterations
    }
}
