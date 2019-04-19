#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/times.h>
#include <string.h>


void print_usage(FILE* stream, char* program_name)
{
    fprintf(stream, "Usage: %s  fifo_path messages_to_send_count\n", program_name);
}

void get_date(char* date_buf, int buf_size)
{
    FILE* pout = popen("date", "r");
    if (pout != NULL)
    {
        fread(date_buf, sizeof(char), buf_size, pout);
        if (pclose(pout) == -1)
        {
            perror("pclose");
            exit(EXIT_FAILURE);
        }
    }
    else
    {
        perror("popen");
        exit(EXIT_FAILURE);
    }
}

void write_to_fifo(const char* fifo_path, int messages_count)
{
    int fifo = open(fifo_path, O_WRONLY);
    if (fifo != -1)
    {
        pid_t pid = getpid();
        printf("Slave with PID %d started.\n", pid);

        srand(times(NULL));
        char buff[500];
        for (int i = 0; i < messages_count; ++i)
        {
            char date[100];
            get_date(date, 100);

            snprintf(buff, 500, "PID %d Date: %s", pid, date);  // date should already contain newline character
            write(fifo, buff, strlen(buff));

            int seconds_to_sleep = (int) (rand() / ((float) RAND_MAX) * 3 + 2);
            sleep(seconds_to_sleep);
        }
        close(fifo);
    }
    else
    {
        perror("open fifo");        ;
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        print_usage(stderr, argv[0]);
        return EXIT_FAILURE;
    }

    int messages_count = atoi(argv[2]);

    write_to_fifo(argv[1], messages_count);

    return EXIT_SUCCESS;
}