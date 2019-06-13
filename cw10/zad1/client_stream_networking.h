#pragma once

#include "server_stream_networking.h"
#include "netinet/in.h"

struct connection_data{
    // in case of local socket
    char server_socket_path[sizeof(struct sockaddr_un) - sizeof(sa_family_t)];
    
    // in case of IPv4 connection
    struct in_addr server_ip_address;   /* in network byte order */
    in_port_t server_port_number;       /* in network byte order */
};

void client_open_connection(const char* client_name, int connection_type /*TODO*/, struct connection_data* cdata);

