TEMP_FLAGS=-g -Wall

all: server client

server: server.c queue_common.h
	gcc $(CFLAGS) $(TEMP_FLAGS) server.c -o server

client: client.c queue_common.h
	gcc $(CFLAGS) $(TEMP_FLAGS) client.c -o client

clean:
	rm server client
