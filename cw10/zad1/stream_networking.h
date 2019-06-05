#pragma once

#include <sys/un.h>

#define MAX_CONNECTIONS 32

struct connection_data{
    char server_socket_path[sizeof(struct sockaddr_un) - sizeof(sa_family_t)];
};

void server_open_connection(int port_number, const char* socket_path);

void client_open_connection(const char* client_name, int connection_type /*TODO*/, struct connection_data* cdata);
