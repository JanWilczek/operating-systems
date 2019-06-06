#include <string.h>
#include "client.h"
#include "stream_networking.h"

void run_client(const char* client_name, int is_local, const char* server_address)
{
    struct connection_data cdata;
    strcpy((char *) cdata.server_socket_path, server_address);
    client_open_connection(client_name, is_local, &cdata);
}
