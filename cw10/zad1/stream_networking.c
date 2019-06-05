#include "stream_networking.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

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

    // Accept incoming connections
    int cliend_sockfd;
    struct sockaddr_un client_address;
    if ((cliend_sockfd = accept(socket_descriptor, (struct sockaddr *) &client_address, sizeof(struct sockaddr_un))) == -1)
    {
        perror("accept");
    }
    else
    {
        printf("Accepted client with address %s.\n", client_address.sun_path);
    }
    
    char buffer[200];
    ssize_t read;
    while ((read = recvfrom(socket_descriptor, (void *) buffer, 200, 0, &client_address, sizeof(struct sockaddr_un)) > 0))
    {   
        printf("Received from client %s: %s", client_address.sun_path, buffer);
    }
}

void client_open_connection(const char* client_name, int connection_type /*TODO*/, struct connection_data* cdata)
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

    // Connect to server
    struct sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, cdata->server_socket_path);

    if (connect(socket_descriptor, (struct sockaddr*) &server_address, sizeof(struct sockaddr_un)) == -1)
    {
        perror("connect");
        return;
    }
}
