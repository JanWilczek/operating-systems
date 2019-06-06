#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include "client.h"


void print_usage(FILE* stream, const char* program_name)
{
    fprintf(stream, "Usage: %s  -m name -c connection -a server-address\n"
                    "   -h  --help                              print this usage information\n"
                    "   -m  --name                              client's name\n"
                    "   -c  --connection     web |  local       the way to communicate with server: through web or local UNIX socket\n"
                    "   -a  --server-address                    server's IPv4 address or path to the UNIX socket\n", program_name);
}

int main(int argc, char* argv[])
{
    const char* program_name = argv[0];

    const char* short_opts = "hm;c:a:";
    const struct option long_opts[] = {
        {"help",            no_argument,        NULL, 'h'},
        {"name",            required_argument,  NULL, 'm'},
        {"connection",      required_argument,  NULL, 'c'},
        {"server-address",  required_argument,  NULL, 'a'},
        {NULL,          0,                  NULL, 0}
    };

    const char* client_name = NULL;
    int is_local = -1;
    const char* server_address = NULL;

    int next_option = 0;
    while ((next_option = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
    {
        switch(next_option)
        {
            case 'h':
                print_usage(stdout, program_name);
                exit(EXIT_SUCCESS);

            case 'm':
                client_name = optarg;
                break;
            
            case 'c':
                if (strcmp(optarg, "web") == 0)
                {
                    is_local = 0;
                }
                else if (strcmp(optarg, "local") == 0)
                {
                    is_local = 1;
                }
                else
                {
                    fprintf(stderr, "Invalid connection type.\n");
                    print_usage(stderr, program_name);
                    exit(EXIT_FAILURE);
                }
                break;

            case 'a':
                server_address = optarg;
                break;

            case -1:
                break;

            case '?':
            default:
                fprintf(stderr, "Unknown option used.\n");
                print_usage(stderr, program_name);
                exit(EXIT_FAILURE);
        }
    }

    if (client_name == NULL || is_local == -1 || server_address == NULL)
    {
        fprintf(stderr, "Insufficient number of arguments.\n");
        print_usage(stderr, program_name);
        exit(EXIT_FAILURE);
    }

    run_client(client_name, is_local, server_address);

    return EXIT_SUCCESS;
}
