#include <getopt.h>
#include <stdlib.h>
#include <stdio.h>


void print_usage(FILE* stream, const char* program_name)
{
    fprintf(stream, "Usage: %s  [OPTIONS]\n"
                    "   -n, --nthreads      number of threads to use\n"
                    "   -d, --division      division of work between threads: \"block\" or \"interleaved\"\n"
                    "   -f, --filename      name of the image file to filter\n"
                    "   -l, --filter        name of the file with filter's description\n"
                    "   -o, --out           name of the output file\n", program_name);
}

int main(int argc, char* argv[])
{
    const char* opts_short = "n:d:f:l:o:";
    const struct option opts_long = {{

    }};

    int opt;
    while ((opt = getopt_long()) != -1)
    {

    }
}