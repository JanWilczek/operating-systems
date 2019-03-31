#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>

void random_append(const char* filename, int pmin, int pmax, int bytes)
{
    pid_t pid = getpid();
    int random_seconds;
    char* random_characters;

    const int date_length = 21;
    char date[date_length];
    const char date_format[] = "%F %T";
    time_t t;
    struct tm* tmp;

    const int append_size = bytes + date_length + 10 /* for pid */ + 10 /* for seconds */;

    while(1)
    {
        // get random number of seconds
        random_seconds = (int) pmin + (pmax - pmin) * ((float) rand()) / ((float) RAND_MAX);

        // prepare date
        t = time(NULL);
        tmp = localtime(&t);
        strftime(date, date_length, date_format, tmp);

        // generate random characters
        random_characters = malloc((bytes + 1) * sizeof(char));
        for (int i = 0; i < bytes; ++i)
        {
            const int ASCII_MIN = 33;
            const int ASCII_MAX = 126;

            random_characters[i] = (char) ASCII_MIN + (ASCII_MAX - ASCII_MIN) * ((float) rand()) / ((float) RAND_MAX);
        }
        random_characters[bytes] = '\0';

        // prepare the final string
        char* final_string = malloc(append_size * sizeof(char));
        snprintf(final_string, append_size, "%d %d %s %s\n", pid, random_seconds, date, random_characters);

        // append to the file
        FILE* file = fopen(filename, "a");
        if (file != NULL)
        {
            fwrite(final_string, strlen(final_string), 1u, file);
            fclose(file);
        }

        // free the resources (since the program must by terminated externally)
        free(final_string);
        free(random_characters);

        // sleep for the generated number of seconds
        sleep(random_seconds);
    }
}
