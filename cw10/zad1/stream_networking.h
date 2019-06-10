#pragma once

#include <unistd.h>
#include <sys/un.h>

#define MAX_CONNECTIONS 32

/* Server-client message types */
#define REGISTER "REGISTER"
#define UNREGISTER "UNREGISTER"
#define COMPUTE "COMPUTE"
#define RESULT "RESULT"
#define PING "PING"
#define PINGREPLY "PINGREPLY"



struct connection_data{
    char server_socket_path[sizeof(struct sockaddr_un) - sizeof(sa_family_t)];
};

struct client_data {
    char* name;
    struct sockaddr* address;
    socklen_t address_size;
};

int server_start_up(const char *socket_path);
void server_open_connection(int port_number, const char* socket_path);
void server_main_loop();

void client_open_connection(const char* client_name, int connection_type /*TODO*/, struct connection_data* cdata);
