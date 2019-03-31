#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

pid_t child_pid = 0;

void start_child()
{
    pid_t pid = fork();
    if (pid == 0)
    {
        // child process
        execlp("./print_date.sh", "print_date.sh", NULL);
    }
    else
    {
        child_pid = pid;
    }
}

void end_child()
{
    kill(child_pid, SIGTERM);   // default signall of the kill command - should terminate the process
    child_pid = 0;
}

void handle_sigtstp(int signum)
{
    printf("\nWaiting for ^Z-continue or ^C-end program.\n");
    if (child_pid == 0)
    {
        start_child();
    }
    else
    {
        end_child();
    }
}

void handle_sigint(int signum)
{
    printf("\nSIGINT has been received. Ending.\n");
    if (child_pid > 0)
    {
        end_child();
    }

    exit(0);
}

int main()
{
    signal(SIGTSTP, &handle_sigtstp);

    struct sigaction sigint_info;
    sigint_info.sa_handler = &handle_sigint;
    sigemptyset(&sigint_info.sa_mask);
    sigint_info.sa_flags = 0;
    sigaction(SIGINT, &sigint_info, NULL);

    if (child_pid == 0)
    {
        start_child();
    }

    while(1)
    {
        sleep(1);
    }

    return 0;
}