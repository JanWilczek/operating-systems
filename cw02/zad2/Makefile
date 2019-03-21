TEMP_FLAGS=-g -Wall

all: dirinfo

test: dirinfo
	./dirinfo -p ./test_dir -c '>' -d '11/03/2019' --nftw

dirinfo: dir_info.o main.o
	gcc $(CFLAGS) $(TEMP_FLAGS) dir_info.o main.o -o dirinfo

main.o: main.c dir_info.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

dir_info.o: dir_info.h dir_info.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -c dir_info.c

clean:
	rm -f *.o dirinfo