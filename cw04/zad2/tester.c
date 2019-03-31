#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include "random_append.h"

void print_usage(FILE *stream, const char *program_name)
{
    fprintf(stream, "Usage: %s { option argument }\n", program_name);
    fprintf(stream, "   -h  --help          Display this usage information.\n"
                    "   -f  --file          Path to the file to write random bytes to.\n"
                    "   -i  --pmin          Lower range of seconds to find random update interval in.\n"
                    "   -x  --pmax          Higher range of seconds to find random update interval in.\n"
                    "   -b  --bytes         Number of bytes to append\n");
}

int main(int argc, char *argv[])
{
    const char short_opts[] = "hf:i:x:b:";
    const struct option long_opts[] = {
        {"help", 0, NULL, 'h'},
        {"file", 1, NULL, 'f'},
        {"pmin", 1, NULL, 'i'},
        {"pmax", 1, NULL, 'x'},
        {"bytes", 1, NULL, 'b'},
        {NULL, 0, NULL, 0}};

    int next_option = 0;

    char* filename = NULL;
    int pmin = 0;
    int pmax = 0;
    int bytes = 0;

    while (next_option != -1)
    {
        next_option = getopt_long(argc, argv, short_opts, long_opts, NULL);

        switch (next_option)
        {
            case 'h':
                print_usage(stdout, argv[0]);
                return 0;
            case 'f':
                filename = optarg;
                break;
            case 'i':
                pmin = atoi(optarg);
                break;
            case 'x':
                pmax = atoi(optarg);
                break;
            case 'b':
                bytes = atoi(optarg);
                break;
            case -1:
                break;
            case '?':
                print_usage(stderr, argv[0]);
                return 1;
            default:
                abort();
        }
    }

    if (filename == NULL || pmin >= pmax || bytes <= 0)
    {
        fprintf(stderr, "Invalid arguments passed.\n");
        return 1;
    }

    random_append(filename, pmin, pmax, bytes);
    
    return 0;
}