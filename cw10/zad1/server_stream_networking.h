#pragma once

#include <unistd.h>
#include <sys/un.h>
#include "thread_safe_queue.h"

#define BUFFER_SIZE 128
#define MAX_CONNECTIONS 32
#define TASK_QUEUE_SIZE MAX_CONNECTIONS * 8

/* Server-client message types */
#define REGISTER "REGISTER"
#define REGISTERDENIED "REGDEN"
#define UNREGISTER "UNREGISTER"
#define COMPUTE "COMPUTE"
#define RESULT "RESULT"
#define PING "PING"
#define PINGREPLY "PINGREPLY"
#define END "END"


struct connection_data{
    char server_socket_path[sizeof(struct sockaddr_un) - sizeof(sa_family_t)];
};

struct client_data {
    char* name;
    int sockfd;
};

struct server_data{
    int epoll_fd;
    int sockfd;
    struct client_data** clients;
    thread_safe_queue_t queue;
};

int server_start_up(const char *socket_path, struct server_data* server);
// void server_open_connection(int port_number, const char* socket_path);
void server_main_loop(struct server_data* server);
void server_shut_down(struct server_data* server, const char* socket_path);

