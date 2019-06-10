#include "stream_networking.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>

int shut_server;

int server_start_up(const char *socket_path)
{
    // Necessary for early process termination
    unlink(socket_path);

    // Set up appropriate flag
    shut_server = 0;

    // Create local socket
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_UNIX, SOCK_STREAM, 0)) == -1)
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

    // Listen for client connections
    if (listen(socket_descriptor, MAX_CONNECTIONS) == -1)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    return socket_descriptor;
}

void shut_down(int socket_descriptor)
{
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

void server_open_connection(int port_number, const char *socket_path)
{
    int socket_descriptor = server_start_up(socket_path);

    while (!shut_server)
    {
        // Accept incoming connections
        int cliend_sockfd;
        struct sockaddr client_address;
        socklen_t address_size;
        if ((cliend_sockfd = accept(socket_descriptor, (struct sockaddr *)&client_address, &address_size)) == -1)
        {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("Accepted client with address %s.\n", client_address.sa_data);

        const int BUFFER_SIZE = 100;
        char buffer[BUFFER_SIZE];
        ssize_t read;

        while (1)
        {
            int cliend_recv_sockfd;
            struct sockaddr client_recv_address;
            socklen_t recv_address_size;

            // Wait for next data packet
            read = recvfrom(socket_descriptor, (void *)buffer, 200, 0, &client_address, &address_size);
            if (read == -1)
            {
                perror("recvfrom");
                exit(EXIT_FAILURE);
            }

            // Ensure the buffer is null-terminated
            buffer[BUFFER_SIZE - 1] = 0;
            
            // For debugging purposes
            printf("Received from client %s: %s", client_address.sa_data, buffer);

            // Handle incoming commands
            if (strncmp(buffer, REGISTER, BUFFER_SIZE) == 0)
            {
                
            }
        
        }
    }

    // Shut down server connection
    shut_down(socket_descriptor);

    // Remove created socket
    unlink(socket_path);
}

void client_open_connection(const char *client_name, int connection_type /*TODO*/, struct connection_data *cdata)
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

    shut_down(socket_descriptor);
}
