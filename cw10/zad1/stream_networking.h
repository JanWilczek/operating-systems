#pragma once

#define MAX_CONNECTIONS 32

void server_open_connection(int port_number, const char* socket_path);
