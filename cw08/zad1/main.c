#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>


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
                nthreads = atoi(argv[optind]);
                break;
            case 'd':   // division type
                // TODO
                break;
            case 'f':   // input image filename
                image_filepath = argv[optind];
                break;
            case 'l':   // filter's filename
                filter_filepath = argv[optind];
                break;
            case 'o':   // output filename
                output_filepath = argv[optind];
                break;
            case -1:    // finished parsing
                break;
            case '?':   // improper option
            default:    // error
                print_usage(stderr, program_name);
                exit(EXIT_FAILURE);
        }
    }
}
