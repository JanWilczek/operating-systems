#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "queue_common.h"
#include "ipc_queue_posix.h"

ipc_queue_t *queue;
ipc_queue_t *server_queue;
pthread_t server_stop_watcher;
long client_id;

// Forward declarations
void parse_and_interpret_command(char *buffer, int buffer_size);

//************** COMMANDS SENT TO THE SERVER **************
void send_stop()
{
    if (client_send_message(server_queue, client_id, "", STOP) == -1)
    {
        perror("client_send_message: STOP");
    }
}

void send_echo(const char *message)
{
    if (client_send_message(server_queue, client_id, message, ECHO) == -1)
    {
        perror("client_send_message (send ECHO)");
        return;
    }

    char buffer[MSG_MAX_SIZE];
    long type = ECHO;
    if (client_receive_message(queue, client_id, buffer, MSG_MAX_SIZE, &type, 1) == -1)
    {
        perror("client_receive_message (send ECHO)");
        return;
    }

    printf("%s\n", buffer);
}

void send_list()
{
    if (client_send_message(server_queue, client_id, "", LIST) == -1)
    {
        perror("client_send_message (send LIST)");
    }

    char buffer[MSG_MAX_SIZE];
    long type = LIST;
    if (client_receive_message(queue, client_id, buffer, MSG_MAX_SIZE, &type, 1) == -1)
    {
        perror("client_receive_message (send LIST)");
    }

    printf("%s\n", buffer);
}

void send_friends(const char* friends_list)
{
    if (client_send_message(server_queue, client_id, friends_list, FRIENDS) == -1)
    {
        perror("client_send_message (send FRIENDS)");
    }
}

void send_add(const char* friends_list)
{
    if (client_send_message(server_queue, client_id, friends_list, ADD) == -1)
    {
        perror("client_send_message (send ADD)");
    }
}

void send_del(const char* friends_to_remove)
{
    if (client_send_message(server_queue, client_id, friends_to_remove, DEL) == -1)
    {
        perror("client_send_message (send DEL)");
    }
}

void send_to_all(const char* message)
{
    if (client_send_message(server_queue, client_id, message, TOALL) == -1)
    {
        perror("client_send_message (send 2ALL)");
    }
}

void send_to_friends(const char* message)
{
    if (client_send_message(server_queue, client_id, message, TOFRIENDS) == -1)
    {
        perror("client_send_message (send 2FRIENDS)");
    }
}

void send_to_one(const char* message)
{
    // Check if the format is correct.
    // The message itself will be sent as a whole string.
    long receiver_id;
    char buffer[MSG_MAX_SIZE - sizeof(receiver_id)];
    if (sscanf(message, "%ld %[^\t\n]\n", &receiver_id, buffer) < 2)    // if less than 2 items have been assigned...
    {
        fprintf(stderr, "Invalid format of the command! Should be: 2ONE receiving_client_id message_string.\n");
        return;
    }

    if (client_send_message(server_queue, client_id, message, TOONE) == -1)
    {
        perror("client_send_message (send 2ONE)");
    }
}

//************** END OF COMMANDS SENT TO THE SERVER **************

void read_commands_from_file(const char* filename)
{
    FILE* file = fopen(filename, "r");
    if (file != NULL)
    {
        char* line;
        size_t size = 0u;

        while (getline(&line, &size, file) != EOF)
        {
            parse_and_interpret_command(line, size);
        }

        free(line);
        fclose(file);
    }
    else
    {
        perror("fopen");
    }
}

void client_exit(void)
{
    send_stop();

    if (queue)
    {
        close_queue(queue);
        remove_queue(queue);
        queue = NULL;
    }

    if (server_queue)
    {
        close_queue(server_queue);
        free(server_queue);
        server_queue = NULL;
    }
}

void sigint_handler(int signum)
{
    // client_exit() will be called, since it has been registered with atexit().
    exit(EXIT_SUCCESS);
}

void print_command_usage(void)
{
    fprintf(stderr, "Invalid command!\n");
}

void parse_and_interpret_command(char *buffer, int buffer_size)
{
    char command[20];
    char message[MSG_MAX_SIZE];

    int scanned = sscanf(buffer, "%s %[^\t\n]\n", command, message);

    if (scanned == 1)
    {
        // Handle one-argument commands
        if (strcasecmp(command, "list") == 0)
        {
            send_list();
        }
        else if (strcasecmp(command, "friends") == 0)
        {
            send_friends("");
        }
        else if (strcasecmp(command, "stop") == 0)
        {
            exit(EXIT_SUCCESS); // this should automatically call client_exit
        }
        else
        {
            print_command_usage();
        }
    }
    else if (scanned == 2)
    {
        // Handle two-argument commands
        if (strcasecmp(command, "echo") == 0)
        {
            send_echo(message);
        }
        else if (strcasecmp(command, "friends") == 0)
        {
            send_friends(message);
        }
        else if (strcasecmp(command, "add") == 0)
        {
            send_add(message);
        }
        else if (strcasecmp(command, "del") == 0)
        {
            send_del(message);
        }
        else if (strcasecmp(command, "2all") == 0)
        {
            send_to_all(message);
        }
        else if (strcasecmp(command, "2friends") == 0)
        {
            send_to_friends(message);
        }
        else if (strcasecmp(command, "2one") == 0)
        {
            send_to_one(message);
        }
        else if (strcasecmp(command, "read") == 0)
        {
            read_commands_from_file(message);
        }
        else
        {
            print_command_usage();
        }
    }
    else
    {
        print_command_usage();
    }
}

void *server_stop_watch(void *main_thread_id_ptr)
{
    // Receive main thread id and free the memory allocated for it
    pthread_t main_thread_id = *((pthread_t *)main_thread_id_ptr);
    free(main_thread_id_ptr);

    // Check if server has stopped
    long type = STOP;
    long server_text_message_type = client_id;
    const int BUF_SIZE = MSG_MAX_SIZE;
    char buffer[BUF_SIZE];
    while (receive_message(queue, buffer, BUF_SIZE, &type, 0) == -1)
    {
        sleep(1);

        // Print text messages from server
        server_text_message_type = client_id;
        int err;
        while ((err = receive_message(queue, buffer, BUF_SIZE, &server_text_message_type, 0)) != -1)
        {
            printf("%s\n", buffer);
            server_text_message_type = client_id;
        }
        if (err == -1 && errno != ENOMSG)
        {
            perror("receive_message (watcher)");
        }

        type = STOP;
    }

    if (pthread_kill(main_thread_id, SIGINT))
    {
        perror("kill to self");
        exit(EXIT_FAILURE);
    }

    return NULL;
}

void start_watch_thread(void)
{
    // Data has to be allocated to avoid race condition on variable destruction
    pthread_t* main_thread_id = malloc(sizeof(pthread_t));
    *main_thread_id = pthread_self();

    if (pthread_create(&server_stop_watcher, NULL, server_stop_watch, main_thread_id) != 0)
    {
        perror("pthread_create");
        exit(EXIT_FAILURE);
    }
}

void client_loop(void)
{
    start_watch_thread();

    const int BUF_SIZE = MSG_MAX_SIZE;
    char buffer[BUF_SIZE];

    while (1)
    {
        // Get user command
        fgets(buffer, BUF_SIZE, stdin);
        parse_and_interpret_command(buffer, BUF_SIZE);
        sleep(1);
    }
}

int main(int argc, char *argv[])
{
    // Initialize the global variables
    queue = NULL;
    server_queue = NULL;
    client_id = -1;

    // Handle normal process termination
    if (atexit(client_exit) != 0)
    {
        fprintf(stderr, "Cannot set exit function of the client. Terminating.\n");
        exit(EXIT_FAILURE);
    }

    // Handle process interruption by a SIGINT signal
    struct sigaction handler;
    handler.sa_handler = sigint_handler;
    sigemptyset(&handler.sa_mask);
    handler.sa_flags = 0;
    if (sigaction(SIGINT, &handler, NULL) != 0)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // 1. Create queue with unique POSIX name
    if ((queue = create_queue(CLIENT_QUEUE)) == NULL)
    {
        fprintf(stderr, "Client: could not create a queue.\n");
        exit(EXIT_FAILURE);
    }

    // 2.a. Get server queue reference
    const int NAME_SIZE = 255;
    char* name = malloc(NAME_SIZE * sizeof(char));
    if (name == NULL)
    {
        perror("malloc");
        exit (EXIT_FAILURE);
    }

    snprintf(name, NAME_SIZE, "%s", SERVER_NAME);
    server_queue = get_queue(name);
    if (server_queue == NULL)
    {
        perror("get_queue (get server queue on client)");
        exit(EXIT_FAILURE);
    }

    free(name);

    // 2.b. Send the name to the server
    if (client_send_message(server_queue, NO_CLIENT_ID, queue->name, INIT) == -1)
    {
        perror("send_message (client init)");
        exit(EXIT_FAILURE);
    }

    // 3. Receive client ID
    char buffer[MSG_MAX_SIZE];
    long type = INIT;
    if (receive_message(queue, buffer, sizeof(buffer), &type, 1) == -1)
    {
        perror("receive_message (client init)");
        exit(EXIT_FAILURE);
    }
    if (type != INIT)
    {
        fprintf(stderr, "Invalid client-server communication.\n");
        exit(EXIT_FAILURE);
    }
    client_id = atol(buffer);
    printf("Client: My ID is %ld.\n", client_id);

    // 4. Send requests in a loop
    // client_loop();

    return EXIT_SUCCESS;
}