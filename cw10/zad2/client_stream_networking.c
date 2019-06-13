#include "client_stream_networking.h"
#include "words_calculator.h"
#include "utils.h"
#include "thread_safe_queue.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define _SVID_SOURCE
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

extern int close_client;

void send_result(int server_sockfd, int task_id, const char *filepath, struct wc_result *words_counted)
{
    char buffer[BUFFER_SIZE];

    // Send RESULT command
    snprintf(buffer, BUFFER_SIZE, "%s", RESULT);
    write(server_sockfd, buffer, BUFFER_SIZE);

    // Send task id
    snprintf(buffer, BUFFER_SIZE, "%d", task_id);
    write(server_sockfd, buffer, BUFFER_SIZE);

    // Send string output
    if (filepath)
    {
        snprintf(buffer, BUFFER_SIZE, "File: %s\n", filepath);
        write(server_sockfd, buffer, BUFFER_SIZE);
    }

    snprintf(buffer, BUFFER_SIZE, "Total word count: %ld\n", words_counted->total_words);
    write(server_sockfd, buffer, BUFFER_SIZE);

    snprintf(buffer, BUFFER_SIZE, "Word:           Count:\n");
    write(server_sockfd, buffer, BUFFER_SIZE);

    for (int i = 0; i < words_counted->distinct_words_len; ++i)
    {
        // '-' says "align to left", '*' says "pad with spaces to the right"
        // and 16 tells how wide the first field should be (it will be padded accordingly)
        snprintf(buffer, BUFFER_SIZE, "%-*s  %d\n", 25, words_counted->distinct_words[i], words_counted->distinct_words_count[i]);
        while (write(server_sockfd, buffer, BUFFER_SIZE) == -1 && (errno == EWOULDBLOCK || errno == EAGAIN))
        {}
        

        fflush(stdout);
        fsync(server_sockfd);
    }

    // Write END message
    snprintf(buffer, BUFFER_SIZE, "%s", END);
    while (write(server_sockfd, buffer, BUFFER_SIZE) == -1 && (errno == EWOULDBLOCK || errno == EAGAIN))
    {}

    fflush(stdout);
    fsync(server_sockfd);
}

void handle_compute(int socket_descriptor, int task_id)
{
    int ret;
    char buffer[BUFFER_SIZE];

    // Read task id
    // while (recv(socket_descriptor, buffer, BUFFER_SIZE, MSG_WAITALL) == -1 && errno == EAGAIN)
    // {}

    // int task_id = atoi(buffer);
    printf("Received task ID: %d\n", task_id);

    // Read the name of the file to count words in
    const int MAX_FILENAME_LENGTH = 1024;
    char *filename = malloc(MAX_FILENAME_LENGTH * sizeof(char));
    char *filename_helper = filename;

    // while(recv(socket_descriptor, buffer, BUFFER_SIZE, MSG_PEEK) <= 0);
    while ((ret = recv(socket_descriptor, buffer, BUFFER_SIZE, MSG_WAITALL)) > 0)
    {
        if (ret == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                break;
            }
        }
        strncpy(filename_helper, buffer, ret);
        filename_helper += ret;
    }
    printf("Received filename: %s\n", filename);

    if (!is_file(filename))
    {
        fprintf(stderr, "No file under given path %s.\n", filename);
        return;
    }

    struct wc_result words_counted;
    wc_calculate_words(filename, &words_counted);
    wc_print(filename, &words_counted);
    printf("Finished computation, sending result.\n");
    // send_result(socket_descriptor, task_id, filename, &words_counted);
    printf("Result sent (WIP).\n");
    wc_free(&words_counted);
}

void client_main_loop(int socket_descriptor)
{
    char buffer[BUFFER_SIZE];
    ssize_t ret;

    while (!close_client)
    {
        ret = read(socket_descriptor, buffer, BUFFER_SIZE);
        if (ret == -1)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                perror("read");
                // exit(EXIT_FAILURE);
                return;
            }
        }
        else if (ret > 0)
        {
            if (strncmp(buffer, REGISTERDENIED, BUFFER_SIZE) == 0)
            {
                fprintf(stderr, "Name already taken. Exiting.\n");
                break;
            }
            else if (strncmp(buffer, COMPUTE, strlen(COMPUTE)) == 0)
            {
                printf("Received: %s\n", buffer);

                // This is why everyone should love C. These guys below make sure that the filename later on
                // is received in the right order. I mean it's obvious right? What's even more hilarious
                // I have discovered it by including printf statements here and there. Worked like a charm!
                fflush(stdout);
                fsync(socket_descriptor);

                handle_compute(socket_descriptor, atoi(buffer + strlen(COMPUTE)));
            }
            else if (strncmp(buffer, PING, BUFFER_SIZE) == 0)
            {
                printf("Pinged.\n");

                snprintf(buffer, BUFFER_SIZE, "%s", PINGREPLY);
                if (write(socket_descriptor, buffer, BUFFER_SIZE) == -1)
                {
                    perror("write");
                }
            }
        }
        else
        {
            break;
        }
    }
}

int connect_and_bind_local(struct connection_data *cdata)
{
    // Create local socket
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_UNIX, SOCK_DGRAM | SOCK_NONBLOCK, 0)) == -1)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_un server_address;
    server_address.sun_family = AF_UNIX;
    strcpy(server_address.sun_path, cdata->server_socket_path);

    // Perform autobind
    if (bind(socket_descriptor, (struct sockaddr*) &server_address, sizeof(sa_family_t)) == -1)
    {
        perror("bind");
        return -1;
    }

    if (connect(socket_descriptor, (struct sockaddr *)&server_address, sizeof(struct sockaddr_un)) == -1)
    {
        perror("connect");
        return -1;
    }
    else
    {
        printf("Successfully  connected to server at %s\n", server_address.sun_path);
    }

    return socket_descriptor;
}

int connect_and_bind_inet(struct connection_data *cdata)
{
    // Create local socket
    int socket_descriptor;
    if ((socket_descriptor = socket(AF_INET, SOCK_DGRAM | SOCK_NONBLOCK, 0)) == -1)
    {
        perror("socket");
        return -1;
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = cdata->server_ip_address.s_addr;
    server_address.sin_port = cdata->server_port_number;

    if (connect(socket_descriptor, (struct sockaddr *)&server_address, sizeof(struct sockaddr_in)) == -1 && errno != EINPROGRESS)
    {
        perror("connect");
        return -1;
    }
    else
    {
        printf("Successfully  connected to server at %s:%hd\n", inet_ntoa(server_address.sin_addr), ntohs(server_address.sin_port));
    }

    return socket_descriptor;
}

void send_name(int socket_descriptor, const char *client_name)
{
    char buffer[BUFFER_SIZE];
    ssize_t ret;

    snprintf(buffer, BUFFER_SIZE, "%s%s", REGISTER, client_name);
    ret = write(socket_descriptor, (const void *)buffer, BUFFER_SIZE);
    if (ret == -1)
    {
        perror("write");
    }
}

int connect_to_server(int is_local, struct connection_data *cdata)
{
    int socket_descriptor;
    if (is_local)
    {
        socket_descriptor = connect_and_bind_local(cdata);
    }
    else
    {
        socket_descriptor = connect_and_bind_inet(cdata);
    }

    if (socket_descriptor == -1)
    {
        fprintf(stderr, "Could not connect to server at %s:%hd.\n", inet_ntoa(cdata->server_ip_address), ntohs(cdata->server_port_number));
        exit(EXIT_FAILURE);
    }

    return socket_descriptor;
}

void disconnect_from_server(int socket_descriptor)
{
    // TODO: Add unregistration
    if (send(socket_descriptor, UNREGISTER, strlen(UNREGISTER) + 1, 0) < 0)
    {
        perror("send");
    }

    if (close(socket_descriptor) == -1)
    {
        perror("close");
        exit(EXIT_FAILURE);
    }
}

void client_open_connection(const char *client_name, int is_local, struct connection_data *cdata)
{
    int socket_descriptor = connect_to_server(is_local, cdata);

    send_name(socket_descriptor, client_name);

    // CLIENT MAIN LOOP
    client_main_loop(socket_descriptor);

    disconnect_from_server(socket_descriptor);
}
