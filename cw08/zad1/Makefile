TEMP_FLAGS=-g -Wall

all: imfilt

imfilt: main.o filter.o pgm_io.o
	gcc $(CFLAGS) $(TEMP_FLAGS)filter.o pgm_io.o main.o -o imfilt

main.o: main.c filter.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

filter.o: filter.h filter.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c filter.c

pgm_io.h: pgm_io.h pgm_io.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c pgm_io.c

clean:
	rm *.o imfilt
