#include <stdio.h>
#include "command_interpreter.h"


void print_usage(FILE* stream, const char* program_name)
{
    fprintf(stream,
            " Usage:    %s  <file>\n\n"
                "filepath - path to the file with commands.\n", program_name);
}

int main(int argc, char* argv[])
{
    if (argc != 2)
    {
        print_usage(stderr, argv[0]);
        return 1;
    }

    parse_and_interpret_commands(argv[1]);

    return 0;
}
