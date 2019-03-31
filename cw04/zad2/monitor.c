#include "monitor.h"
#include <stdlib.h>
#include <stdio.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/times.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

const int MAX_LINE_SIZE = 256;

monitor_t *monitor_create()
{
    return malloc(sizeof(monitor_t));
}

int get_line_count(FILE *file)
{
    rewind(file);

    int eof = 0;
    int nb_lines = 0;
    const char *line_format = "%s %d";
    int unused;
    char *line = malloc(MAX_LINE_SIZE * sizeof(char));

    while (eof != EOF)
    {
        eof = fscanf(file, line_format, line, &unused);
        if (eof != EOF)
        {
            ++nb_lines;
        }
    }

    free(line);
    rewind(file);

    return nb_lines;
}

int monitor_parse_files(monitor_t *monitor, const char *filename)
{
    FILE *file = fopen(filename, "r");
    if (file != NULL)
    {
        int nb_seconds = 0;
        const char *line_format = "%s %d";

        int line_count = get_line_count(file);

        rewind(file);

        monitor->file_count = line_count;
        monitor->files_to_monitor = malloc(monitor->file_count * sizeof(char *));
        monitor->monitor_interval = malloc(monitor->file_count * sizeof(int));
        for (int i = 0; i < line_count; ++i)
        {
            char *line = malloc(MAX_LINE_SIZE * sizeof(char));

            // Read files to monitor and seconds to monitor them for
            fscanf(file, line_format, line, &nb_seconds);

            monitor->files_to_monitor[i] = line;
            monitor->monitor_interval[i] = nb_seconds;
        }

        fclose(file);
        return 0;
    }

    return 1;
}

time_t get_modification_time(const char *filename)
{
    struct stat file_status;

    int err = lstat(filename, &file_status);

    if (err == 0)
    {
        return file_status.st_mtime;
    }
    return 1;
}

int get_file_size(const char *filename)
{
    struct stat file_stat;
    stat(filename, &file_stat);
    return file_stat.st_size;
}

/**
 * Returns the content of the whole file copied character by character
 * */
char *get_file_contents(const char *filename, int *file_size_in_bytes)
{
    const int file_size = get_file_size(filename);

    if (file_size_in_bytes != NULL)
    {
        *file_size_in_bytes = file_size;
    }

    char *contents = malloc(file_size);
    char *contents_pointer = contents;

    FILE *file = fopen(filename, "r");
    if (file != NULL)
    {
        int character = 0;

        while (character != EOF)
        {
            character = getc(file);
            *contents = character; // EOF will also be written
            ++contents;
        }

        fclose(file);
        return contents_pointer;
    }

    free(contents);
    return NULL;
}

int clock_ticks_to_s(clock_t ticks)
{
    const int clock_ticks_per_second = sysconf(_SC_CLK_TCK);
    return ticks / clock_ticks_per_second;
}

char *get_new_filename(const char *original_filename)
{
    char *filename_copy = malloc(strlen(original_filename) * sizeof(char) + 4);
    strcpy(filename_copy, original_filename);

    // Remove the directory path from the filename-leave only what is after the last '/' character
    const char *previous_token = original_filename;
    const char delimiter[] = "/";

    for (char *token = strtok(filename_copy, delimiter); token != NULL; token = strtok(NULL, delimiter))
    {
        previous_token = token;
    }

    // Use the strftime to format the date string
    const int date_length = 21;
    char date[date_length];
    const char date_format[] = "_%F_%H-%M-%S";
    time_t t = time(NULL);
    struct tm *tmp = localtime(&t);
    strftime(date, date_length, date_format, tmp);

    // Format the final string
    const int new_filename_length = strlen(previous_token) + 24;
    char *new_filename = malloc(new_filename_length);
    snprintf(new_filename, new_filename_length, "%s%s", previous_token, date);

    free(filename_copy);

    return new_filename;
}

char *format_file_path(const char *filename, const char *dirname)
{
    const int new_file_path_length = strlen(filename) + strlen(dirname) + 10;
    char *new_file_path = malloc(new_file_path_length);
    snprintf(new_file_path, new_file_path_length, "./%s/%s", dirname, filename);

    return new_file_path;
}

void write_backup(const char *original_filename, char *contents_to_store, int file_size_in_bytes)
{
    //  Write contents to a file in the "archives" folder under an updated name.
    char *new_filename = get_new_filename(original_filename);

    // if directory "archive" does not exist, create it
    const char *dirname = "archive";
    mkdir(dirname, S_IRWXU | S_IRWXG); // if the directory exists, nothing will be done

    // create and write to the file
    char *new_file_path = format_file_path(new_filename, dirname);

    FILE *file = fopen(new_file_path, "w");
    if (file != NULL)
    {
        for (int i = 0; i < file_size_in_bytes; ++i, ++contents_to_store)
        {
            putc(*contents_to_store, file);
        }
        fclose(file);
    }

    free(new_file_path);
    free(new_filename);
}

int should_monitor = 1;
int copies_count = 0;

void constant_store_monitor(const char *filename, int interval_seconds)
{
    time_t modification_time = get_modification_time(filename);
    int file_size_bytes;
    char *file_contents = get_file_contents(filename, &file_size_bytes);

    while (1)
    {
        // Sleep for a given interval
        sleep(interval_seconds);

        if (should_monitor) // Check flag for this process
        {
            time_t last_modified = get_modification_time(filename);
            if (last_modified == 1)
            {
                fprintf(stderr, "Error while fetching modification time");
            }

            // If the modification date changed then save the file stored in memory in "archive" directory
            // and store the current file contents in memory.
            if (last_modified > modification_time)
            {
                modification_time = last_modified;

                // Write the backup file
                write_backup(filename, file_contents, file_size_bytes);

                ++copies_count;

                free(file_contents);
                file_contents = get_file_contents(filename, &file_size_bytes);
            }
        }
    }

    free(file_contents);

    exit(copies_count);
}

int file_exists(const char *path)
{
    struct stat file_stat;
    stat(path, &file_stat);

    if (S_ISREG(file_stat.st_mode) || S_ISLNK(file_stat.st_mode))
    {
        return 1;
    }

    return 0;
}

void monitor_file(const char *filename, int interval_seconds)
{
    // Check that the file exists
    if (!file_exists(filename))
    {
        exit(0);
    }

    // Each process should terminate after monitor->monitor_time_seconds seconds and return the number of copies created.
    constant_store_monitor(filename, interval_seconds);
}

// ******* SIGNAL HANDLING FUNCTIONS ******************************

void handle_stop(int signum)
{
    printf("%d: Stopped monitoring.\n", getpid());
    should_monitor = 0;
}

void handle_start(int signum)
{
    printf("%d: Started monitoring.\n", getpid());
    should_monitor = 1;
}

void handle_end(int signum)
{
    printf("%d: Ended monitoring.\n", getpid());
    exit(copies_count);
}

// ******* END OF SIGNAL HANDLING FUNCTIONS ***********************

// ********* COMMAND HANDLING FUNCTIONS ***************************
void list_processes_files(monitor_t *monitor)
{
    printf("PID       FILE\n");

    const char format[] = "%-10d%-30s\n";

    for (int i = 0; i < monitor->file_count; ++i)
    {
        printf(format, monitor->pids[i], monitor->files_to_monitor[i]);
    }
}

void stop_pid(pid_t pid)
{
    kill(pid, SIGUSR1);
}

void stop_all(monitor_t *monitor)
{
    for (int i = 0; i < monitor->file_count; ++i)
    {
        stop_pid(monitor->pids[i]);
    }
}

void start_pid(pid_t pid)
{
    kill(pid, SIGUSR2);
}

void start_all(monitor_t *monitor)
{
    for (int i = 0; i < monitor->file_count; ++i)
    {
        start_pid(monitor->pids[i]);
    }
}

void end_all(monitor_t *monitor)
{
    // Terminate child processes.
    for (int i = 0; i < monitor->file_count; ++i)
    {
        kill(monitor->pids[i], SIGTERM);
    }

    // print report
    // Retrieve child process status and write out "Process PID created n copies of the monitored file."
    int return_code;
    for (int i = 0; i < monitor->file_count; ++i)
    {
        pid_t finished = waitpid(monitor->pids[i], &return_code, 0);
        if (WIFEXITED(return_code) || WIFSIGNALED(return_code))
        {
            printf("Process %d created %d copies of the monitored file %s.\n",
                   finished, WEXITSTATUS(return_code), monitor->files_to_monitor[i]);
        }
    }

    exit(0);
}

// ********* END OF COMMAND HANDLING FUNCTIONS ***************************

int should_end = 0;

void handle_sigint_main(int signum)
{
    printf("\nMain process: received SIGINT, write something and press enter to end program.\n");
    should_end = 1;
}

void monitor_start(monitor_t *monitor)
{
    monitor->pids = malloc(monitor->file_count * sizeof(pid_t));

    // For each file create a child process and make each one monitor one file every given number of seconds.
    for (int i = 0; i < monitor->file_count; ++i)
    {
        pid_t pid = fork();

        if (pid == 0)
        {
            // Child process
            // ************** SIGNAL HANDLING CODE **********************************************************************
            signal(SIGUSR1, &handle_stop);
            signal(SIGUSR2, &handle_start);
            signal(SIGTERM, &handle_end);
            // ************** END OF SIGNAL HANDLING CODE ***************************************************************
            monitor_file(monitor->files_to_monitor[i], monitor->monitor_interval[i]);
        }
        else
        {
            // Parent process
            monitor->pids[i] = pid;
        }
    }

    //************* ADD COMMAND HANDLING HERE *********************************************************
    // 1. List all files and the processes monitoring them.
    list_processes_files(monitor);

    // 2. Change SIGINT handling
    signal(SIGINT, &handle_sigint_main);

    // 3. Retrieve commands
    char command[30];
    pid_t pid;

    while (1)
    {
        if (should_end)
        {
            end_all(monitor);
        }

        scanf("%s", command);

        if (strcmp(command, "LIST") == 0)
        {
            list_processes_files(monitor);
        }
        else if (strcmp(command, "END") == 0)
        {
            end_all(monitor);
        }
        else if (strcmp(command, "STOP") == 0)
        {
            scanf("%s", command);

            if (strcmp(command, "ALL") == 0)
            {
                stop_all(monitor);
            }
            else
            {
                int scanned_count = sscanf(command, "%d", &pid); // read from the previously read second word of the command NOT the standard input

                if (scanned_count == 1) // scanned exactly one number - PID
                {
                    stop_pid(pid);
                }
                else
                {
                    fprintf(stderr, "Unrecognized command.\n");
                }
                
            }
        }
        else if (strcmp(command, "START") == 0)
        {
            scanf("%s", command);

            if (strcmp(command, "ALL") == 0)
            {
                start_all(monitor);
            }
            else
            {
                int scanned_count = sscanf(command, "%d", &pid); // read from the previously read second word of the command NOT the standard input

                if (scanned_count == 1) // scanned exactly one number - PID
                {
                    start_pid(pid);
                }
                else
                {
                    fprintf(stderr, "Unrecognized command.\n");
                }
            }
        }
        else
        {
            fprintf(stderr, "Unrecognized command.\n");
        }

        sleep(1);
    }

    //**************** END OF COMMAND HANDLING ********************************************************
}

void monitor_free(monitor_t *monitor)
{
    for (int i = 0; i < monitor->file_count; ++i)
    {
        free(monitor->files_to_monitor[i]);
    }
    free(monitor->files_to_monitor);
    free(monitor->monitor_interval);
    free(monitor->pids);
    free(monitor);
}
