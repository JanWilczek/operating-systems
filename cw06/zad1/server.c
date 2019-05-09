#include <stdlib.h>
#include <stdio.h>
#include "ipc_queue_sv.h"
#include "queue_common.h"
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <limits.h>

struct client{
    ipc_queue_t* queue;
    int friends[MAX_CLIENTS + 1];  // client IDs of friends - 1 if friend 0 otherwise
};

ipc_queue_t *server_queue;
long next_free_id;
struct client *client_queues[MAX_CLIENTS + 1];

//************** SERVER COMMANDS HANDLING **************

//-------------- HELPER METHODS ----------------------
void format_message(long client_id, const char* message, char *dest)
{
    time_t t = time(NULL);
    char* t_string = ctime(&t);
    t_string[strlen(t_string) - 1] = '\0'; // remove newline character at the end
    snprintf(dest, MSG_MAX_SIZE, "%s Client %ld says: %s", t_string, client_id, message);
}

/**
 * For a given client changes the state of all friends on the list to the
 * one specified in the state argument:
 * 1 => is friend,
 * 0 => is not friend.
 * */
void change_friends_state(long client_id, const char* friends_list, int state)
{
    char* friends_list_copy = strdup(friends_list);
    char delim[] = " ";
    for (char* token = strtok(friends_list_copy, delim); token != NULL; token = strtok(NULL, delim))
    {
        char* is_ok;
        errno = 0;
        long id = strtol(token, &is_ok, 10);
        if (is_ok != token && errno != ERANGE && id != LONG_MIN && id != LONG_MAX && !(errno != 0 && id == 0) // if parsing ok
            && id >= 1 && id < MAX_CLIENTS + 1 && id != client_id)                                            // if data ok
        {
            client_queues[client_id]->friends[id] = state;
        }
    }
    free(friends_list_copy);
}
//---------- END OF HELPER METHODS -------------------

void handle_init(char *keystring)
{
    long client_id = next_free_id++;
    if (client_id > MAX_CLIENTS + 1)
    {
        // Too many clients - denial of service.
        fprintf(stderr, "Cannot register more clients.\n");
        return;
    }

    key_t key = atoi(keystring);

    ipc_queue_t *client_queue = NULL;
    if ((client_queue = get_queue(key)) == NULL)
    {
        return;
    }

    client_queues[client_id] = malloc(sizeof(struct client));
    client_queues[client_id]->queue = client_queue;

    char buffer[MSG_MAX_SIZE];
    sprintf(buffer, "%ld", client_id);
    if (send_message(client_queue, buffer, INIT) == -1)
    {
        perror("send_message (client init on server)");
    }
    else
    {
        printf("Server: Registered client with ID %ld\n", client_id);
    }
}

void handle_stop(long client_id)
{
    free(client_queues[client_id]->queue);
    free(client_queues[client_id]);
    client_queues[client_id] = NULL;
}

void handle_echo(long client_id, const char* string)
{
    char buffer[MSG_MAX_SIZE];
    time_t t = time(NULL);
    snprintf(buffer, MSG_MAX_SIZE, "%s %s", string, ctime(&t));
    buffer[strlen(buffer) - 1] = '\0';  // remove newline character at the end 

    if (send_message(client_queues[client_id]->queue, buffer, ECHO) == -1)
    {
        perror("send_message (ECHO response)");
    }
}

void handle_list(long client_id)
{
    char buffer[MSG_MAX_SIZE] = "Active clients are: ";
    char client_id_string[10];

    for (long i = 1L; i < next_free_id; ++i)
    {
        if (client_queues[i])
        {
            if (i == client_id)
            {
                snprintf(client_id_string, sizeof(client_id_string), "%ld (You), ", i); 
            }
            else
            {
                snprintf(client_id_string, sizeof(client_id_string), "%ld, ", i);
            }
            
            strcat(buffer, client_id_string);
        }
    }
    buffer[strlen(buffer) - 2] = '.';
    buffer[strlen(buffer) - 1] = '\0';

    if (send_message(client_queues[client_id]->queue, buffer, LIST) == -1)
    {
        perror("send_message (LIST response)");
    }
}

void handle_add(long client_id, const char* friends_list)
{
    change_friends_state(client_id, friends_list, 1);
}

void handle_del(long client_id, const char* friends_to_remove)
{
    change_friends_state(client_id, friends_to_remove, 0);
}

void handle_friends(long client_id, const char* friends_list)
{
    for (long i = 1L; i < MAX_CLIENTS + 1; ++i)
    {
        client_queues[client_id]->friends[i] = 0;
    }

    handle_add(client_id, friends_list);
}

void handle_to_all(long client_id, const char* message)
{
    // Prepare message
    char buffer[MSG_MAX_SIZE];
    format_message(client_id, message, buffer);

    // Send message
    for (long i = 1L; i < next_free_id; ++i)
    {
        if (client_queues[i] && i != client_id)
        {
            if (send_message(client_queues[i]->queue, buffer, i) == -1)
            {
                perror("send_message (2ALL response)");
            }
        }
    }
}

void handle_to_friends(long client_id, const char* message)
{
    // Prepare message
    char buffer[MSG_MAX_SIZE];
    format_message(client_id, message, buffer);

    // Send message
    for (long i = 1L; i < next_free_id; ++i)
    {
        if (client_queues[client_id]->friends[i])
        {
            if (send_message(client_queues[i]->queue, buffer, i) == -1)
            {
                perror("send_message (2ALL response)");
            }
        }
    }
}

void handle_to_one(long client_id, const char* message)
{
    // Parse message
    long receiver_id;
    char buffer[MSG_MAX_SIZE - sizeof(receiver_id)];
    if (sscanf(message, "%ld %[^\t\n]\n", &receiver_id, buffer) < 2)    // if less than 2 items have been assigned...
    {
        fprintf(stderr, "Invalid format of the command! Should be: 2ONE receiving_client_id message_string.\n");
        return;
    }

    // Prepare message
    char dest[MSG_MAX_SIZE];
    format_message(client_id, buffer, dest);

    // Send message
    if (send_message(client_queues[receiver_id]->queue, dest, receiver_id) == -1)
    {
        perror("send_message (2ONE response)");
    }
}

//************** END OF SERVER COMMANDS HANDLING **************

void server_exit(void)
{
    if (server_queue)
    {
        // Send server stop message to clients and count how many are there
        long to_stop = 0L;
        for (long i = 1L; i < next_free_id; ++i)
        {
            if (client_queues[i])
            {
                ++to_stop;
                send_message(client_queues[i]->queue, "", STOP);
            }
        }

        // Wait for all clients to stop
        char buffer[MSG_MAX_SIZE];
        long stopped = 0L;
        while (stopped < to_stop)
        {
            long client_id;
            long type = 0;
            if (server_receive_client_message(server_queue, &client_id, buffer, MSG_MAX_SIZE, &type, 1) == -1)
            {
                perror("server_receive_client_message");
                continue;
            }

            if (type == STOP)
            {
                ++stopped;
                handle_stop(client_id);
            }
        }

        // Shut down the server queue
        remove_queue(server_queue);
    }
}

void sigint_handler(int signum)
{
    server_exit();
    exit(EXIT_SUCCESS);
}

void server_loop(void)
{
    long message_type;
    long client_id;
    char message[MSG_MAX_SIZE];

    while (1)
    {
        // Await all types of messages
        message_type = 0L;

        if (server_receive_client_message(server_queue, &client_id, message, MSG_MAX_SIZE, &message_type, 1) == -1)
        {
            if (errno != ENOMSG)
            {
                perror("server_receive_client_message");
            }
            continue;
        }

        switch (message_type)
        {
        case INIT:
            handle_init(message);
            break;
        case STOP:
            handle_stop(client_id);
            break;
        case ECHO:
            handle_echo(client_id, message);
            break;
        case TOALL:
            handle_to_all(client_id, message);
            break;
        case LIST:
            handle_list(client_id);
            break;
        case FRIENDS:
            handle_friends(client_id, message);
            break;
        case ADD:
            handle_add(client_id, message);
            break;
        case DEL:
            handle_del(client_id, message);
            break;
        case TOFRIENDS:
            handle_to_friends(client_id, message);
            break;
        case TOONE:
            handle_to_one(client_id, message);
            break;
        default:
            fprintf(stderr, "Server: Unknown message type.\n");
        }
    }
}

int main(int argc, char *argv[])
{
    next_free_id = 1L; // numbering of clients starts with 1 (because sent messages should have mtype > 0)
    server_queue = NULL;

    // Establish a SIGINT handler
    struct sigaction handler;
    handler.sa_handler = sigint_handler;
    sigemptyset(&handler.sa_mask);
    handler.sa_flags = 0;
    if (sigaction(SIGINT, &handler, NULL) != 0)
    {
        perror("sigaction");
        exit(EXIT_FAILURE);
    }

    // 1. Create new queue (for client -> server communication)
    if ((server_queue = create_queue(SERVER_QUEUE)) == NULL)
    {
        fprintf(stderr, "Client: could not create a queue.\n");
        exit(EXIT_FAILURE);
    }

    // 2. Receive and respond to messages
    server_loop();

    return EXIT_SUCCESS;
}