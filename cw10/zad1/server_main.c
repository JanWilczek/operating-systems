#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "server.h"


void print_usage(FILE* stream, const char* program_name)
{
    fprintf(stream, "Usage: %s  -n port_number -s socket_path\n"
                    "   -h  --help               print this usage information.\n"
                    "   -n  --port_number        the number of port to use by the server\n"
                    "   -s  --socket_path        path to the UNIX socket\n", program_name);
}

int main(int argc, char* argv[])
{
    const char* program_name = argv[0];

    const char* short_opts = "hn:s:";
    const struct option long_opts[] = {
        {"help",        no_argument,        NULL, 'h'},
        {"port_number", required_argument,  NULL, 'n'},
        {"socket_path", required_argument,  NULL, 's'},
        {NULL,          0,                  NULL, 0}
    };

    int port_number = -1;
    char* socket_path = NULL;

    int next_option = 0;
    while ((next_option = getopt_long(argc, argv, short_opts, long_opts, NULL)) != -1)
    {
        switch(next_option)
        {
            case 'h':
                print_usage(stdout, program_name);
                exit(EXIT_SUCCESS);

            case 'n':
                port_number = atoi(optarg);
                break;

            case 's':
                socket_path = optarg;
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

    if (port_number == -1 || socket_path == NULL)
    {
        fprintf(stderr, "Insufficient number of arguments.\n");
        print_usage(stderr, program_name);
        exit(EXIT_FAILURE);
    }

    run_server(port_number, socket_path);

    return EXIT_SUCCESS;
}
