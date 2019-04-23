#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include <sys/ipc.h>

#define STOP 1
#define LIST 2
#define FRIENDS 3
#define ECHO 4
#define TOALL 5
#define TOFRIENDS 6
#define TOONE 7
#define INIT 8
#define SERVER_QUEUE_PROJ_ID 13884
#define MSG_MAX_SIZE 2048
#define MAX_CLIENTS 32

enum QueueType { SERVER_QUEUE, CLIENT_QUEUE };

#ifdef __cplusplus
}
#endif