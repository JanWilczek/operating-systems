#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/ipc.h>

#define MAX_CLIENTS 32
#define MSG_MAX_SIZE 1024
#define SERVER_QUEUE_PROJ_ID 13884

// **** MESSAGE TYPES **** //
#define STOP MAX_CLIENTS + 1 + 1
#define LIST MAX_CLIENTS + 1 + 2
#define FRIENDS MAX_CLIENTS + 1 + 3
#define ECHO MAX_CLIENTS + 1 + 4
#define TOALL MAX_CLIENTS + 1 + 5
#define TOFRIENDS MAX_CLIENTS + 1 + 6
#define TOONE MAX_CLIENTS + 1 + 7
#define INIT MAX_CLIENTS + 1 + 8
#define ADD MAX_CLIENTS + 1 + 9
#define DEL MAX_CLIENTS + 1 + 10

enum QueueType { SERVER_QUEUE, CLIENT_QUEUE };

#ifdef __cplusplus
}
#endif