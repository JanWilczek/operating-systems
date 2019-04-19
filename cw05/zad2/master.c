#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


void print_usage(FILE *stream, char *program_name)
{
    fprintf(stream, "Usage: %s  fifo_path\n", program_name);
}

void read_from_fifo(const char *fifo_path)
{
    int fifo = open(fifo_path, O_RDONLY);
    if (fifo != -1)
    {
        const int BUF_LENGTH = 500;
        char buf[BUF_LENGTH];
        int buffer_pointer = 0;
        ssize_t characters_read;
        while (1)
        {
            // read by 1 character until end of line or end of buffer
            characters_read = read(fifo, buf + buffer_pointer, 1);
            buffer_pointer += characters_read;
            if (characters_read == 0)   // end of file
            {
                break;
            }
            else if (characters_read < 0)
            {
                perror("read from fifo");
                exit(EXIT_FAILURE);
            }
            

            if (buf[buffer_pointer - 1] == '\n' || buffer_pointer >= BUF_LENGTH - 1)
            {
                buf[buffer_pointer] = '\0';
                printf("Master read from fifo: %s", buf);
                buffer_pointer = 0;
            }
        }
        close(fifo);
        if (remove(fifo_path) == -1)
        {
            perror("remove");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        perror("open FIFO");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        print_usage(stderr, argv[0]);
        return EXIT_FAILURE;
    }

    // create FIFO pipe
    if (access(argv[1], F_OK) == -1)    // check if already exists-should not exist
    {
        if (mkfifo(argv[1], 0777) != 0)
        {
            perror("mkfifo");
            return EXIT_FAILURE;
        }
    }
    else
    {
        fprintf(stderr, "One master already running! There can be only one.\n");
        exit(EXIT_FAILURE);
    }

    // read from FIFO pipe in a loop
    read_from_fifo(argv[1]);

    return EXIT_SUCCESS;
}