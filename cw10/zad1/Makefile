TEMP_FLAGS=-g -Wall
TEST_SOCKET_NAME=/tmp/df209djf89f0d98uf.socket

all: server client

test: all
	./server -n 9001 -s $(TEST_SOCKET_NAME)
	# ./client -m elo -c local -a /tmp/df209djf89f0d98uf.socket

server: server_main.o server.o stream_networking.o
	gcc $(CFLAGS) $(TEMP_FLAGS) -o server server.o server_main.o stream_networking.o

server_main.o: server_main.c server.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c server_main.c

server.o: server.c server.h stream_networking.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c server.c

client: client_main.o client.o stream_networking.o words_calculator.o
	gcc $(CFLAGS) $(TEMP_FLAGS) -o client client.o client_main.o stream_networking.o words_calculator.o

client_main.o: client_main.c client.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c client_main.c

client.o: client.c client.h stream_networking.h words_calculator.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c client.c

words_calculator.o: words_calculator.h words_calculator.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c words_calculator.c

stream_networking.o: stream_networking.h stream_networking.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c stream_networking.c

words_calculator_test: words_calculator.o words_calculator_test.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -o words_calculator_test words_calculator_test.c words_calculator.o

clean:
	rm *.o server client words_calculator_test
