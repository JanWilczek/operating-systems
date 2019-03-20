#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "file_operations.h"


/**
 * Prints available options for this program to the given stream.
 * */
void print_options(FILE* stream, const char* program_name)
{
    fprintf(stream, "Usage:   %s options [ options_arguments ... ]\n", program_name);
    fprintf(stream,
                "   -h  --help          Display this usage information.\n\n"
                "   -g  --generate      With arguments filename number_of_records record_size_in_bytes generates\n"
                "                       number_of_records of random data of size record_size_in_bytes and stores the in \n"
                "                       file of name filename.\n\n"
                "   -s  --sort          With arguments filename number_of_records record_size_in_bytes library_to_use  \n"
                "                       sorts the records from the file named filename (exactly number_of_records records,\n"
                "                       record_size_in_bytes in size each) using file functions from library_to_use pointed.\n"
                "                       library_to_use may be either \"sys\" (system file functions) or \"lib\" (file \n"
                "                       functions from libc).\n\n"
                "   -c  --copy          With arguments filename1 filename2 number_of_records buffer_size_in_bytes copies\n"
                "                       number_of_records from file named filename1 to file named filename2 using buffer of\n"
                "                       size buffer_size_in_bytes.\n");
}

int main(int argc, char* argv[])
{
    const char opts_short[] = "hg:s:c:";

    const struct option opts_long[] = {
        { "help",       0, NULL, 'h'},
        { "generate",   1, NULL, 'g'},
        { "sort",       1, NULL, 's'},
        { "copy",       1, NULL, 'c'}
    };

    const char* program_name = argv[0];
    int next_option = 0;

    char* filename;
    char* source;
    char* target;
    char* library_name;
    int nb_records;
    int record_size_in_bytes;

    // Read out the command line arguments
    while (next_option != -1)
    {
        next_option = getopt_long(argc, argv, opts_short, opts_long, NULL);

        switch (next_option) 
        {
            case 'h': // Print help
                print_options(stdout, program_name);
                break;
            case 'g':   // Generate file filled with random data
                filename = optarg;
                nb_records = atoi(argv[optind]);    // a hack since getopt_long does not support multiple arguments for an option
                record_size_in_bytes = atoi(argv[optind + 1]);
                printf("You want to generate %d records %dB size each.\n", nb_records, record_size_in_bytes);

                generate_random_records(filename, nb_records, record_size_in_bytes);
                break;
            case 's': // Sort file named filename
                filename = optarg;
                nb_records = atoi(argv[optind]);
                record_size_in_bytes = atoi(argv[optind + 1]);
                library_name = argv[optind + 2];

                sort_records(filename, nb_records, record_size_in_bytes, library_name);
                break;
            case 'c':
                break;
            case -1: // Finished parsing arguments
                break;
            case '?': // Improper option
            default: // Error, exit program
                print_options(stderr, program_name);
                exit(EXIT_FAILURE);
        }
    }

    return EXIT_SUCCESS;
}
