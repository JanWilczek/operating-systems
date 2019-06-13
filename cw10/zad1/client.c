#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define _SVID_SOURCE
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "client.h"
#include "client_stream_networking.h"

void run_client(const char* client_name, int is_local, const char* server_address)
{
    struct connection_data cdata;

    if (is_local)
    {
        strcpy((char *) cdata.server_socket_path, server_address);
    }
    else
    {
        // Parse the IP address
        char* temp = malloc((strlen(server_address) + 1) * sizeof(char));
        strcpy(temp, server_address);
        char* ip_address = strtok(temp, ":");
        if (ip_address == NULL)
        // if (sscanf(server_address, "%s:%hd", ip_address, &port_number) != 2)
        {
            fprintf(stderr, "Invalid server address format. Should be 127.127.127.127:80 (sample values).\n");
            exit(EXIT_FAILURE);
        }

        struct in_addr ip;
        if (inet_aton(ip_address, &ip) == 0)
        {
            perror("inet_aton");
            exit(EXIT_FAILURE);
        }
        cdata.server_ip_address = ip;
        // cdata.server_ip_address.s_addr = INADDR_ANY; // <- equivalent to writing 0.0.0.0

        // Parse the port number
        char* port_number_str = strtok(NULL, ":");
        if (port_number_str == NULL)
        {
            fprintf(stderr, "Invalid server address format. Should be 127.127.127.127:80 (sample values).\n");
            exit(EXIT_FAILURE);
        }

        in_port_t port_number = (in_port_t) atoi(port_number_str);
        cdata.server_port_number = htons(port_number);

        free(temp);
    }
    
    client_open_connection(client_name, is_local, &cdata);

    // Send to server your name - if it is invalid stop

    // Listen for work to do - should be active all the time and accept incoming tasks

    // Perform computation - total number of words and numbers of particular words

    // Handle ^C - unregister from server
}
