#include <stdio.h>
#include <stdlib.h>
#include "queue_common.h"
#include "ipc_queue_sv.h"


int main(int argc, char* argv[])
{
    // 1. Create queue with unique IPC key
    ipc_queue_t* queue = create_queue(CLIENT_QUEUE);
    printf("Client queue ID is %d\n", queue->id);

    // 1.a. Handle SIGINT signal
    ipc_queue_t* server_queue = get_queue(ftok(getenv("HOME"), SERVER_QUEUE_PROJ_ID));
    printf("Server queue ID is %d\n", server_queue->id);

    remove_queue(queue);

    // 2. Send the key to the server

    // 3. Receive client ID

    // 4. Send requests in a loop

    return EXIT_SUCCESS;
}