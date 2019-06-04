TEMP_FLAGS=-g -Wall

all: server

test: all
	./server -n 9001 -s "~/socket_path"

server: server_main.o server.o
	gcc $(CFLAGS) $(TEMP_FLAGS) -o server server.o server_main.o

server_main.o: server_main.c server.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c server_main.c

server.o: server.c server.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c server.c

clean:
	rm *.o server
