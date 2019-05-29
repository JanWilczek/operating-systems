TEMP_FLAGS=-g -Wall

all: imfilt

test: imfilt
	./imfilt -n 4 -d block -f ./test_data/apollonian_gasket.ascii.pgm -l ./test_data/test_filter.txt -o ./test_data/filtered_image.pgm

imfilt: main.o filter.o pgm_io.o
	gcc $(CFLAGS) $(TEMP_FLAGS) filter.o pgm_io.o main.o -o imfilt -lm

main.o: main.c filter.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

filter.o: filter.h filter.c pgm_io.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c filter.c -lm

pgm_io.o: pgm_io.h pgm_io.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c pgm_io.c

clean:
	rm *.o imfilt
