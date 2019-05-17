#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>


void print_usage(const char* program_name)
{
    fprintf(stderr, "Usage:     %s X K M\n"
                    "   L       number of loaders to start\n"
                    "   N       maximum weight of package a loader can give\n", program_name);
}

int main(int argc, char* argv[])
{
    if (argc != 3)
    {
        print_usage(argv[0]);
        exit(EXIT_FAILURE);
    }

    int L = atoi(argv[1]);
    int N = atoi(argv[2]);
    int C = 20;

    int n, c;
    for (int l = 0; l < L; ++l)
    {
        if (fork() == 0)
        {
            srandom(getpid());
            n = (int) ((((float) random()) / RAND_MAX) * (N - 1) + 1); 
            char n_string[128];
            sprintf(n_string, "%d", n);
            c = (int) ((((float) random()) / RAND_MAX) * C);
            char c_string[128];
            sprintf(c_string, "%d", c);
            char* args[] = {"./loader", n_string, c_string, NULL};
            printf("Started loader with N=%s and C=%s\n", n_string, c_string);
            execv(args[0], args);
        }
    }

    for (int l = 0; l < L; ++l)
    {
        wait(NULL); // Wait for L children termination
    }

    exit(EXIT_SUCCESS);
}
