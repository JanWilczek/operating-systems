#include "command_interpreter.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>


void parse_and_interpret_commands(const char* commands_filename)
{
    FILE* file = fopen(commands_filename, "r");
    if (file != NULL)
    {
        size_t line_buffer_size = 500;
        char* line = malloc(line_buffer_size * sizeof(char));   // buffer for getline must be malloc-allocated
        ssize_t characters_read;
        char* commands_saveptr;
        char* arguments_saveptr;

        size_t pids_size = 10;
        pid_t* pids = malloc(pids_size * sizeof(pid_t));
        size_t nb_pids = 0;

        while ((characters_read = getline(&line, &line_buffer_size, file)) != EOF)
        {
            // the pipes-each process has one in and one out except the first process-changes newline character to space character
            int fd_in[2];
            int fd_out[2];

            for (char* command_token = strtok_r(line, "|", &commands_saveptr); command_token != NULL; command_token = strtok_r(NULL, "|", &commands_saveptr))
            {
                // This is a hack to have correct argument parsing
                command_token[strlen(command_token) - 1] = ' ';

                // parse program name and arguments in a loop
                size_t arguments_size = 10;
                char** arguments = malloc(arguments_size * sizeof(char*));
                size_t nb_arguments = 0;

                for (char* argument = strtok_r(command_token, " ", &arguments_saveptr); argument != NULL; argument = strtok_r(NULL, " ", &arguments_saveptr))
                {
                    arguments[nb_arguments++] = argument;
                    if (nb_arguments >= arguments_size)
                    {
                        // reallocate the arguments table
                        arguments_size *= 2;
                        arguments = realloc(arguments, arguments_size);
                    }
                }

                // arguments array must be terminated with a NULL pointer
                arguments[nb_arguments] = NULL;

                // initialize the new pipe
                if (pipe(fd_out) == -1)
                {
                    free(line);
                    free(pids);
                    free(arguments);
                    perror("pipe");
                    exit(EXIT_FAILURE);
                }

                // create new process
                pid_t pid = fork();
                if (pid == 0)   // child process
                {
                    // redirect standard output of the previous process to this process's standard input
                    if (nb_pids > 0)    // does not concern the first process
                    {
                        close(fd_in[1]);    // close the write end
                        if (fd_in[0] != STDIN_FILENO)
                        {
                            if (dup2(fd_in[0], STDIN_FILENO) != STDIN_FILENO)
                            {
                                free(line);
                                free(pids);
                                free(arguments);
                                perror("dup2 error to stdin");
                                _exit(EXIT_FAILURE);
                            }
                            close(fd_in[0]);
                        }
                    }

                    close(fd_out[0]);   // close unused read end
                    if (fd_out[1] != STDOUT_FILENO) // take extra precautions that the duplicated descriptor is actually different from the target
                    {
                        if (dup2(fd_out[1], STDOUT_FILENO) != STDOUT_FILENO)  // redirect standard output to write end of the pipe
                        {
                            free(line);
                            free(pids);
                            free(arguments);
                            perror("[child] dup2 error to stdout");
                            _exit(EXIT_FAILURE);
                        }
                        close(fd_out[1]);
                    }

                    execvp(arguments[0], arguments);   
                    perror("execvp");       // exec functions return only on error

                    _exit(EXIT_FAILURE);    // here having normal exit results in peculiar behaviour

                    //printf("This shouldn't be reached.\n");
                }
                else
                {
                    // close the completely joined pipe-parent shouldn't use the inter-child pipe
                    if (nb_pids > 0)
                    {
                        close(fd_in[0]);
                        close(fd_in[1]);
                    }

                    pids[nb_pids++] = pid;
                    if (nb_pids >= pids_size)
                    {
                        // reallocate the PID table
                        pids_size *= 2;
                        pids = realloc(pids, pids_size);
                    }

                    // swap the pipe descriptors-shift the pipes
                    fd_in[0] = fd_out[0];
                    fd_in[1] = fd_out[1];

                    free(arguments);
                }
            }

            close(fd_in[1]);    // unused

            // wait for all the processes to end
            for (int i = 0; i < nb_pids; ++i)
            {
                pid_t error;
                if ((error = waitpid(pids[i], NULL, 0)) == -1)
                {
                    perror("waitpid");
                }
            }
            nb_pids = 0;    // reset the number of inserted pids
                        
            // read the standard output of the last command
            printf("Output of given commands:\n");
            char buffer[500];
            while (read(fd_in[0], buffer, 500) != 0)
            {
                printf("%s", buffer);
            }

            // close the last descriptor
            close(fd_in[0]);
        }
        
        free(pids);
        free(line);
        fclose(file);
    }
    else
    {
        perror("fopen");
        exit(EXIT_FAILURE);
    }
}