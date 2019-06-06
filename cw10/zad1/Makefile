TEMP_FLAGS=-g -Wall

all: server client

test: all
	./server -n 9001 -s "/tmp/df209djf89f0d98uf.socket"

server: server_main.o server.o stream_networking.o
	gcc $(CFLAGS) $(TEMP_FLAGS) -o server server.o server_main.o stream_networking.o

server_main.o: server_main.c server.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c server_main.c

server.o: server.c server.h stream_networking.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c server.c

client: client_main.o client.o stream_networking.o
	gcc $(CFLAGS) $(TEMP_FLAGS) -o client client.o client_main.o stream_networking.o

client_main.o: client_main.c client.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c client_main.c

client.o: client.c client.h stream_networking.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c client.c

stream_networking.o: stream_networking.h stream_networking.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c stream_networking.c

clean:
	rm *.o server
