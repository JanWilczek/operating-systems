TEMP_FLAGS=-g -Wall

all: imfilt

imfilt: main.o
	gcc $(CFLAGS) $(TEMP_FLAGS) main.o -o imfilt

main.o: main.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

clean:
	rm *.o imfilt
