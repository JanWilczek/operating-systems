TEMP_FLAGS=-g -Wall

all: fileinfo

fileinfo: file_info.o main.o
	gcc $(CFLAGS) $(TEMP_FLAGS) file_info.o main.o -o fileinfo

main.o: main.c file_info.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

file_info.o: file_info.h file_info.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c file_info.c

clean:
	rm -f *.o fileinfo