#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include "file_info.h"

void print_usage(FILE* stream, const char* program_name)
{

}

int main(int argc, char* argv[])
{
    const char short_opts[] = "hp:c:d:";
    const struct option long_opts[] =
        {
            { "help",       0, NULL, 'h'},
            { "path",       1, NULL, 'p'},
            { "comparison", 1, NULL, 'c'},
            { "date",       1, NULL, 'd'},
            { NULL,         0, NULL, 0  }
        };

    const char* program_name = argv[0];
    int next_option = 0;

    const char* path = NULL;
    const char* comparison_operator = NULL;
    const char* date = NULL;

    while (next_option != -1)
    {
        next_option = getopt_long(argc, argv, short_opts, long_opts, NULL);

        switch (next_option)
        {
            case 'h':   // Print help
                print_usage(stdout, program_name);
                return 0;

            case 'p':   // Specify the path to the file to analyse
                path = optarg;
                break;

            case 'c':   // Specify the comparison operator to use
                comparison_operator = optarg;
                break;

            case 'd':   // Specify the date
                date = optarg;
                break;

            case '?':   // Invalid option
                print_usage(stderr, program_name);
                return 0;

            case -1:  // Finished parsing options
                break;

            default:    // Unexpected error
                abort();
        }
    }

    if (!path || !comparison_operator || !date)
    {
        fprintf(stderr, "Not enough input arguments!");
        print_usage(stderr, program_name);
        return EXIT_FAILURE;
    }

    print_file_info(path, comparison_operator, date);

    return EXIT_SUCCESS;
}