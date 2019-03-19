TEMP_FLAGS=-g -Wall

all: program

program: main.o file_operations.o
	gcc $(CFLAGS) $(TEMP_FLAGS) main.o file_operations.o -o program

file_operations.o: file_operations.h file_operations.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c file_operations.c

main.o: main.c file_operations.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

clean:
	rm -f *.o program
