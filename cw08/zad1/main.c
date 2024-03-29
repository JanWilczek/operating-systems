#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "filter.h"


void print_usage(FILE* stream, const char* program_name)
{
    fprintf(stream, "Usage: %s  [OPTIONS]\n"
                    "   -h, --help          print this usage information\n"
                    "   -n, --nthreads      number of threads to use\n"
                    "   -d, --division      division of work between threads: \"block\" or \"interleaved\"\n"
                    "   -f, --filename      name of the image file to filter\n"
                    "   -l, --filter        name of the file with filter's description\n"
                    "   -o, --out           name of the output file\n", program_name);
}

int main(int argc, char* argv[])
{
    const char* program_name = argv[0];
    const char* opts_short = "hn:d:f:l:o:";
    const struct option opts_long[] = {
        {"help",        no_argument,        NULL, 'h'},
        {"nthreads",    required_argument,  NULL, 'n'},
        {"division",    required_argument,  NULL, 'd'},
        {"filename",    required_argument,  NULL, 'f'},
        {"filter",      required_argument,  NULL, 'l'},
        {"out",         required_argument,  NULL, 'o'},
        {NULL,          0,                  NULL, 0}
    };

    int nthreads = 0;
    int is_block = -1;
    char* filter_filepath;
    char* image_filepath;
    char* output_filepath;

    int opt;
    while ((opt = getopt_long(argc, argv, opts_short, opts_long, NULL)) != -1)
    {
        switch(opt)
        {
            case 'h':   // print help
                print_usage(stdout, program_name);
                return 0;

            case 'n':   // number of threads
                nthreads = atoi(optarg);
                break;

            case 'd':   // division type
                if (strcmp(optarg, "block") == 0)
                {
                    is_block = 1;
                }
                else if (strcmp(optarg, "interleaved") == 0)
                {
                    is_block = 0;
                }
                else
                {
                    fprintf(stderr, "Invalid type of thread filtering. Exiting.\n");
                    exit(EXIT_FAILURE);
                }
                
                break;

            case 'f':   // input image filename
                image_filepath = optarg;
                break;

            case 'l':   // filter's filename
                filter_filepath = optarg;
                break;

            case 'o':   // output filename
                output_filepath = optarg;
                break;

            case -1:    // finished parsing
                break;

            case '?':   // improper option
            default:    // error
                print_usage(stderr, program_name);
                exit(EXIT_FAILURE);
        }
    }

    filter_image(image_filepath, filter_filepath, output_filepath, nthreads, is_block);
}
