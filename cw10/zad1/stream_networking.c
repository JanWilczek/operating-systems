#include "stream_networking.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/un.h>

void server_open_connection(int port_number, const char* socket_path)
{
    // Create local socket
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        return;
    }

    // Bind socket to its name
    struct sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strcpy((char*) server_address.sun_path, socket_path);

    if (bind(socket_descriptor, (struct sockaddr *) &server_address, sizeof(struct sockaddr_un) ) == -1)
    {
        perror("bind");
        return;
    }

    // Listen for client connections
    if (listen(socket_descriptor, MAX_CONNECTIONS) == -1)
    {
        perror("listen");
    }
}

void client_open_connection(const char* client_name)
{
    // Create local socket
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        return;
    }

    // Bind socket to client's name
//     if (bind(socket_descriptor, , ) == -1)
//     {
//         perror("bind");
//         return;
//     }
}
