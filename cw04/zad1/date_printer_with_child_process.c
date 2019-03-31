#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>

pid_t child_pid;

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

int main()
{
    start_child();
}