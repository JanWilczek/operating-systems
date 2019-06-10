#include <string.h>
#include "client.h"
#include "client_stream_networking.h"

void run_client(const char* client_name, int is_local, const char* server_address)
{
    struct connection_data cdata;
    strcpy((char *) cdata.server_socket_path, server_address);
    client_open_connection(client_name, is_local, &cdata);

    // Send to server your name - if it is invalid stop

    // Listen for work to do - should be active all the time and accept incoming tasks

    // Perform computation - total number of words and numbers of particular words

    // Handle ^C - unregister from server
}
