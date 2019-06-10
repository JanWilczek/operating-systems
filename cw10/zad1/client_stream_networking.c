#include "client_stream_networking.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>


void client_open_connection(const char *client_name, int connection_type /*TODO*/, struct connection_data *cdata)
{
    // Create local socket
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
    {
        perror("socket");
        return;
    }

    // Connect to server
    struct sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, cdata->server_socket_path);

    if (connect(socket_descriptor, (struct sockaddr *)&server_address, sizeof(struct sockaddr_un)) == -1)
    {
        perror("connect");
        return;
    }
    else
    {
        printf("Successfully  connected to server %s\n", server_address.sun_path);
    }

    const char buffer[] = "Ala ma kota.\n";
    for (int i = 0; i < 20; ++i)
    {
        sendto(socket_descriptor, (const char *)buffer, sizeof(buffer), 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr_un));
    }

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
