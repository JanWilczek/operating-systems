TEMP_FLAGS=-g -Wall
TEST_SOCKET_NAME=/tmp/df209djf89f0d98uf.socket

all: server client

test: all
	./server -n 9001 -s $(TEST_SOCKET_NAME)
	# ./client -m elo -c local -a /tmp/df209djf89f0d98uf.socket
	# ./client -m elo2 -c web -a 0.0.0.0:9001
	# ./test_data/SampleTextFile_1000kb.txt
	# ./test_data/ThePickwickPapers.txt

server: server_main.o server.o server_stream_networking.o thread_safe_queue.o utils.o
	gcc $(CFLAGS) $(TEMP_FLAGS) -o server server.o server_main.o server_stream_networking.o thread_safe_queue.o utils.o -pthread

server_main.o: server_main.c server.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c server_main.c

server.o: server.c server.h server_stream_networking.h thread_safe_queue.h utils.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c server.c -pthread

client: client_main.o client.o client_stream_networking.o words_calculator.o utils.o
	gcc $(CFLAGS) $(TEMP_FLAGS) -o client client.o client_main.o client_stream_networking.o words_calculator.o utils.o

client_main.o: client_main.c client.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c client_main.c

client.o: client.c client.h client_stream_networking.h words_calculator.h utils.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c client.c

words_calculator.o: words_calculator.h words_calculator.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c words_calculator.c

server_stream_networking.o: server_stream_networking.h server_stream_networking.c thread_safe_queue.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c server_stream_networking.c

client_stream_networking.o: client_stream_networking.h client_stream_networking.c server_stream_networking.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c client_stream_networking.c

thread_safe_queue.o: thread_safe_queue.c thread_safe_queue.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c thread_safe_queue.c

words_calculator_test: words_calculator.o words_calculator_test.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -o words_calculator_test words_calculator_test.c words_calculator.o

utils.o: utils.h utils.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c utils.c

clean:
	rm *.o server client words_calculator_test
