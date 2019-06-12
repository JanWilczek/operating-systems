#include "server.h"
#include "server_stream_networking.h"

/* The server has following duties:
    1. Listen on web and local socket.
    2. Handle client registration.
    3. Parse command line for computation demands.
    4. Dispatch computation to clients.
    5. Receive client's output.
    6. Ping clients.
    7. Monitor multiple descriptors.
*/
extern int shut_server;

void run_server(int port_number, char *socket_path)
{
    // Create a structure for clients' data
    struct client_data* clients[MAX_CONNECTIONS];
    for (int i = 0 ; i < MAX_CONNECTIONS; ++i)
    {
        clients[i] = NULL;
    }

    struct server_data server;
    server.clients = (struct client_data**) &clients;

    // Open socket for connection
    server_start_up(socket_path, &server);

    // Start client-monitoring thread
    server_main_loop(&server);

    // Start command parsing and computation dispatching thread

    // Wait for threads to join

    // Shut the server down   
    server_shut_down(&server, socket_path);
}
