#include "client_stream_networking.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <signal.h>

int close_client;

void sigint_handler(int num)
{
    close_client = 1;
}

void client_open_connection(const char *client_name, int connection_type /*TODO*/, struct connection_data *cdata)
{
    close_client = 0;

    // Set up SIGINT handler
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    // Create local socket
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_UNIX, SOCK_STREAM | SOCK_NONBLOCK, 0)) == -1)
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

    char buffer[BUFFER_SIZE];
    ssize_t ret;
    snprintf(buffer, BUFFER_SIZE, "%s", REGISTER);
    // sendto(socket_descriptor, (const char *)buffer, sizeof(buffer), 0, (struct sockaddr *)&server_address, sizeof(struct sockaddr_un));
    ret = write(socket_descriptor, (const void *)buffer, BUFFER_SIZE);
    if (ret == -1)
    {
        perror("write");
    }

    snprintf(buffer, BUFFER_SIZE, "%s", client_name);
    ret = write(socket_descriptor, (const void *)buffer, BUFFER_SIZE);
    if (ret == -1)
    {
        perror("write");
    }

    snprintf(buffer, BUFFER_SIZE, "%s", END);
    ret = write(socket_descriptor, (const void *)buffer, BUFFER_SIZE);
    if (ret == -1)
    {
        perror("write");
    }

    while (!close_client)
    {
        ret = read(socket_descriptor, buffer, BUFFER_SIZE);
        if (ret == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("read");
                exit(EXIT_FAILURE);
            }
        }
        else if (ret > 0)
        {
            if (strncmp(buffer, REGISTERDENIED, BUFFER_SIZE) == 0)
            {
                fprintf(stderr, "Name already taken. Exiting.\n");
                break;
            }
            else if (strncmp(buffer, COMPUTE, BUFFER_SIZE) == 0)
            {
                // TODO
            }

        }
        else
        {
            break;
        }
    }

    snprintf(buffer, BUFFER_SIZE, "%s", UNREGISTER);
    ret = write(socket_descriptor, (const void *)buffer, BUFFER_SIZE);
    if (ret == -1)
    {
        perror("write");
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
