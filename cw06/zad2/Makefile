TEMP_FLAGS=-g -Wall

all: server client

server: server.c queue_common.h ipc_queue_posix.o
	gcc $(CFLAGS) $(TEMP_FLAGS) server.c ipc_queue_posix.o -o server -lrt

client: client.c queue_common.h ipc_queue_posix.o
	gcc $(CFLAGS) $(TEMP_FLAGS) client.c ipc_queue_posix.o -pthread -o client -lrt

ipc_queue_sv.o: ipc_queue_posix.h ipc_queue_posix.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c ipc_queue_posix.c -lrt

clean:
	rm *.o server client
