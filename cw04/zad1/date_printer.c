#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>

int should_proceed = 1;

void handle_sigtstp(int signum)
{
    should_proceed = !should_proceed;
    if (!should_proceed)
    {
        // the program is stopped
        printf("\nWaiting for ^Z-continue or ^C-end program.\n");
    }
}

void handle_sigint(int signum)
{
    printf("\nSIGINT has been received. Ending.\n");
    exit(0);
}

int main(int argc, char *argv[])
{
    time_t current_time;
    
    // Handle SIGTSTP signal
    signal(SIGTSTP, &handle_sigtstp);

    // Handle SIGINT signal
    //struct sigaction sigint_handler = { .sa_handler=&handle_sigint, .sa_mask=0, .sa_flags=0 };
    struct sigaction sigint_handler;
    sigint_handler.sa_handler = &handle_sigint;
    sigemptyset(&sigint_handler.sa_mask);
    sigint_handler.sa_flags = 0;
    sigaction(SIGINT, &sigint_handler, NULL);

    while (1)
    {
        if (should_proceed)
        {
            current_time = time(NULL);
            const int STRING_SIZE = 40;
            char *formatted_time = malloc(STRING_SIZE * sizeof(char));

            strftime(formatted_time, STRING_SIZE, "%F %T", localtime(&current_time));
            printf("%s\n", formatted_time);
            free(formatted_time);
        }

        sleep(1);
    }

    return 0;
}