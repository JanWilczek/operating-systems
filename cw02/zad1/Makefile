TEMP_FLAGS=-g -Wall

all: program speed_test

speed_test: file_operations.o speed_test.o stop_watch.o
	gcc $(CFLAGS) $(TEMP_FLAGS) file_operations.o speed_test.o stop_watch.o -o speed_test

speed_test.o: speed_test.c file_operations.h stop_watch.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c speed_test.c

stop_watch.o: stop_watch.c stop_watch.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c stop_watch.c

program: main.o file_operations.o
	gcc $(CFLAGS) $(TEMP_FLAGS) main.o file_operations.o -o program

file_operations.o: file_operations.h file_operations.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c file_operations.c

main.o: main.c file_operations.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

clean:
	rm -f *.o program
