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

void run_server(int port_number, char *socket_path)
{
    // Create a structure for clients' data
    struct client_data clients[MAX_CONNECTIONS];

    // Open socket for connection
    int server_fd = server_start_up(socket_path);

    // Start client-monitoring thread
    server_main_loop(server_fd, (struct client_data**) &clients);

    // Start command parsing and computation dispatching thread

    // Wait for threads to join

    // Shut the server down   
    server_shut_down(server_fd, socket_path);
}
