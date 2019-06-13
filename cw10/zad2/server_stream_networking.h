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
#define NOTREGISTERED "NOTREGISTER"
#define UNREGISTER "UNREGISTER"
#define COMPUTE "COMPUTE"
#define RESULT "RESULT"
#define PING "PING"
#define PINGREPLY "PINGREPLY"
#define END "1248END1248"


struct client_data {
    char* name;
    int sockfd;
    struct sockaddr* addr;
    socklen_t addr_len;
    int nb_pending_tasks;

    int should_be_removed;
    int pinged;
};

struct server_data{
    int epoll_fd;
    int sockfd;
    int inet_sockfd;
    struct client_data** clients;
    thread_safe_queue_t queue;
    int tasks_assigned;
};

int server_start_up(const char *socket_path, struct server_data* server, int port_number);
void server_main_loop(struct server_data* server);
void server_shut_down(struct server_data* server, const char* socket_path);
void pinging_loop(struct server_data *server);

