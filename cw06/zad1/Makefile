TEMP_FLAGS=-g -Wall

all: server client

server: server.c queue_common.h ipc_queue_sv.o
	gcc $(CFLAGS) $(TEMP_FLAGS) server.c ipc_queue_sv.o -o server

client: client.c queue_common.h ipc_queue_sv.o
	gcc $(CFLAGS) $(TEMP_FLAGS) client.c ipc_queue_sv.o -pthread -o client

ipc_queue_sv.o: ipc_queue_sv.h ipc_queue_sv.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c ipc_queue_sv.c

clean:
	rm *.o server client
