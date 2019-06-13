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
    free(client->addr);
    free(client);
}

int get_client_id(const struct server_data *server, struct sockaddr *addr, socklen_t addr_len)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (server->clients[i] != NULL && server->clients[i]->addr_len == addr_len && memcmp(server->clients[i]->addr, addr, addr_len))
        {
            return i;
        }
    }
    return -1;
}

const char *get_client_name(const struct server_data *server, struct sockaddr *addr, socklen_t addr_len)
{
    int client_id = get_client_id(server, addr, addr_len);
    if (client_id == -1)
    {
        return NULL;
    }
    return server->clients[client_id]->name;
}
/*****************************************************/

/*********** COMMAND HANDLING FUNCTIONS *****************/
void handle_register(struct server_data *server, int sockfd, const char *client_name, struct sockaddr *addr, socklen_t addr_len)
{
    for (int i = 0; i < MAX_CONNECTIONS; ++i)
    {
        if (server->clients[i] != NULL && strcmp(server->clients[i]->name, client_name) == 0)
        {
            const char *msg = REGISTERDENIED;
            sendto(sockfd, msg, BUFFER_SIZE, 0, addr, addr_len);
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
    client->name = malloc(strlen(client_name) + 1);
    strncpy(client->name, client_name, strlen(client_name) + 1);
    client->sockfd = sockfd;
    client->addr = malloc(addr_len);
    memcpy(client->addr, addr, addr_len);
    client->addr_len = addr_len;
    client->nb_pending_tasks = 0;
    client->should_be_removed = 0;
    server->clients[first_free_id] = client;

    // Set up client's socket descriptor for monitoring
    // struct epoll_event event_options;
    // event_options.events = EPOLLIN | EPOLLRDHUP | EPOLLET;
    // event_options.data.fd = client->sockfd;
    // if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, client->sockfd, &event_options) == -1)
    // {
    //     perror("epoll");
    // }
    // else
    // {
    printf("Successfully registered client %s at id %d.\n", client_name, first_free_id);
    // }
}

void handle_unregister(struct server_data *server, int sockfd, struct sockaddr *addr, socklen_t addr_len)
{
    int i = get_client_id(server, addr, addr_len);
    if (i >= 0)
    {
        // struct epoll_event event;
        // epoll_ctl(server->epoll_fd, EPOLL_CTL_DEL, server->clients[i]->sockfd, &event); // event must be non-null due to a bug
        printf("Successfully unregistered client with name %s.\n", server->clients[i]->name);
        free_client(server->clients[i]);
        server->clients[i] = NULL;
    }
    else
    {
        fprintf(stderr, "Cannot unregister not registered client with given address.\n");
    }
}

void handle_result(struct server_data *server, int sockfd, struct sockaddr *addr, socklen_t addr_len, int task_id)
{
    char buffer[BUFFER_SIZE];

    // recvfrom(sockfd, buffer, BUFFER_SIZE, 0, &addr, &addr_len);
    // int task_id = atoi(buffer);

    printf("Client %s has completed task %d with the following result:\n", get_client_name(server, addr, addr_len), task_id);

    // Receive and print the result
    int ret;
    while ((ret = recvfrom(sockfd, buffer, BUFFER_SIZE, 0, addr, &addr_len)) > 0 || (errno == EAGAIN || errno == EWOULDBLOCK))
    {
        if (ret == -1)
        {
            continue;
        }

        // char *is_end = strstr(buffer, END);
        // if (is_end != NULL)
        // {
        // is_end[0] = '\0'; // print only characters up to is_end[0]
        // printf("%s", buffer);
        // printf("Read %s. Breaking input.\n", is_end + 1); // Should display 248END1248
        // printf("End of result from client %s.\n", get_client_name(server, client_sockfd));
        // break;
        // }

        printf("%s", buffer);

        // fflush(stdout);
        // fsync(sockfd);
    }

    if (ret == -1)
    {
        perror("read");
    }

    --server->clients[get_client_id(server, addr, addr_len)]->nb_pending_tasks;
}

void handle_response(struct server_data *server, int sockfd)
{
    char buffer[BUFFER_SIZE];

    // Determine the type of command
    struct sockaddr addr;
    socklen_t addr_len;
    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, &addr, &addr_len);

    if (strncmp(buffer, REGISTER, strlen(REGISTER)) == 0)
    {
        // A new client connected
        handle_register(server, sockfd, buffer + strlen(REGISTER) /* client name */, &addr, addr_len);
    }
    else if (strncmp(buffer, UNREGISTER, strlen(UNREGISTER)) == 0)
    {
        handle_unregister(server, sockfd, &addr, addr_len);
    }
    else if (strncmp(buffer, RESULT, strlen(RESULT)) == 0)
    {
        handle_result(server, sockfd, &addr, addr_len, atoi(buffer + strlen(RESULT)));
    }
    else if (strncmp(buffer, PINGREPLY, BUFFER_SIZE) == 0)
    {
        server->clients[get_client_id(server, &addr, addr_len)]->pinged = 0;
    }
    else
    {
        printf("%s", buffer);
    }
}
/********************************************************/

int start_up(const char *socket_path)
{
    // Create local socket
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0)) == -1)
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
    
    return socket_descriptor;
}

int start_up_inet(int port_number)
{
    // Create local socket
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) == -1)
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

    return socket_descriptor;
}

void epoll_subscribe(struct server_data* server, int sockfd)
{
    struct epoll_event event_options;
    event_options.events = EPOLLIN | EPOLLET;
    event_options.data.fd = sockfd;
    if (epoll_ctl(server->epoll_fd, EPOLL_CTL_ADD, sockfd, &event_options) == -1)
    {
        perror("epoll");
    }
}

int server_start_up(const char *socket_path, struct server_data *server, int port_number)
{
    // Necessary for early process termination
    unlink(socket_path);

    server->sockfd = start_up(socket_path);
    server->inet_sockfd = start_up_inet(port_number);
    server->epoll_fd = epoll_create1(0);

    epoll_subscribe(server, server->sockfd);
    epoll_subscribe(server, server->inet_sockfd);

    return 0;
}

void shut_down(int socket_descriptor)
{
    if (close(socket_descriptor) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

void server_shut_down(struct server_data *server, const char *socket_path)
{
    // TODO: Add sending messages to clients that server is going down

    // Shut down server connection
    shut_down(server->sockfd);
    shut_down(server->inet_sockfd);

    // Remove created socket
    unlink(socket_path);
}

void handle_event(struct server_data *server, struct epoll_event *event)
{
    if (event->events & EPOLLIN)
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
    struct client_data *client = server->clients[target_client_id];
    ++(client->nb_pending_tasks);

    char buffer[BUFFER_SIZE];

    // Send COMPUTE command and task id
    snprintf(buffer, BUFFER_SIZE, "%s%d", COMPUTE, task_id);
    sendto(client->sockfd, buffer, BUFFER_SIZE, 0, client->addr, client->addr_len);
    memset(buffer, 0, BUFFER_SIZE);

    // Send task id
    // sprintf(buffer, "%d", task_id);
    // write(server->clients[target_client_id]->sockfd, buffer, BUFFER_SIZE);
    // memset(buffer, 0, BUFFER_SIZE);

    // Send filename to examine
    sprintf(buffer, "%s", filename);
    sendto(client->sockfd, filename, strlen(filename) + 1, 0, client->addr, client->addr_len);
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
    struct sockaddr addr;
    socklen_t addr_len;
    char buffer[BUFFER_SIZE];
    if (recvfrom(server_sockfd, buffer, BUFFER_SIZE, 0, &addr, &addr_len) > 0)
    {
        if (strncmp(buffer, REGISTER, strlen(REGISTER)) == 0)
        {
            // A new client connected
            handle_register(server, server_sockfd, buffer + strlen(REGISTER) /* client name */, &addr, addr_len);
        }
        else if (strncmp(buffer, UNREGISTER, strlen(UNREGISTER)) == 0)
        {
            handle_unregister(server, server_sockfd, &addr, addr_len);
        }
        else if (strncmp(buffer, RESULT, strlen(RESULT)) == 0)
        {
            handle_result(server, server_sockfd, &addr, addr_len, atoi(buffer + strlen(RESULT)));
        }
        else
        {
            printf("%s", buffer);
        }
    }
}

void server_main_loop(struct server_data *server)
{
    while (!shut_server)
    {
        // Check local socket
        // examine_socket(server, server->sockfd);
        // examine_socket(server, server->inet_sockfd);

        // Check for events from server's sockets
        check_events(server);

        // Check for work to dispatch
        if (queue_size(&server->queue) > 0)
        {
            dispatch_work(server);
        }
    }
}

void pinging_loop(struct server_data *server)
{
    char buffer[BUFFER_SIZE];
    int ret;
    while (!shut_server)
    {
        for (int i = 0; i < MAX_CONNECTIONS; ++i)
        {
            // Ping only clients without pending tasks
            if (server->clients[i] != NULL && server->clients[i]->nb_pending_tasks == 0)
            {
                // Check if client responded to ping
                if (!server->clients[i]->should_be_removed)
                {
                    if (!server->clients[i]->pinged)
                    {
                        server->clients[i]->pinged = 1;
                        sprintf(buffer, "%s", PING);
                        while ((ret = write(server->clients[i]->sockfd, buffer, BUFFER_SIZE)) <= 0)
                        {
                            if (ret == -1)
                            {
                                if (errno != EAGAIN && errno != EWOULDBLOCK && errno != EDQUOT)
                                {
                                    perror("write");
                                    continue;
                                }
                            }
                        }
                    }
                    else
                    {
                        printf("Client %s did not respond. He shall be removed.\n", server->clients[i]->name);
                        server->clients[i]->should_be_removed = 1;
                    }
                }

                if (server->clients[i]->should_be_removed)
                {
                    handle_unregister(server, server->clients[i]->sockfd, server->clients[i]->addr, server->clients[i]->addr_len);
                }
            }
        }

        sleep(2);
    }
}
