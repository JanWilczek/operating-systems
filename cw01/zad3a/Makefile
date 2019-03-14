TEMP_FLAGS=-g -Wall
MEMBLOCKS_LIBRARY_PATH=../zad1
SPEED_TEST_PATH=../zad2

all: speed_test_static speed_test_shared

# STATIC COMPILATION

speed_test_static: speed_test.o $(MEMBLOCKS_LIBRARY_PATH)/libmemblocks.a $(MEMBLOCKS_LIBRARY_PATH)/memblocks.c
	make -C $(MEMBLOCKS_LIBRARY_PATH)
	gcc $(CFLAGS) $(TEMP_FLAGS) -o speed_test speed_test.o stop_watch.o $(MEMBLOCKS_LIBRARY_PATH)/libmemblocks.a

speed_test.o: speed_test.c $(MEMBLOCKS_LIBRARY_PATH)/memblocks.h stop_watch.o
	gcc $(CFLAGS) $(TEMP_FLAGS) -c speed_test.c -I $(MEMBLOCKS_LIBRARY_PATH)

stop_watch.o: stop_watch.c stop_watch.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c stop_watch.c

# SHARED COMPILATION

main_shared: main.o $(MEMBLOCKS_LIBRARY_PATH)/libmemblocks.so $(MEMBLOCKS_LIBRARY_PATH)/memblocks.c
	make shared -C $(MEMBLOCKS_LIBRARY_PATH)
	gcc $(CFLAGS) $(TEMP_FLAGS) -o main main.o -lmemblocks -L$(MEMBLOCKS_LIBRARY_PATH)

main: main.o $(MEMBLOCKS_LIBRARY_PATH)/libmemblocks.a $(MEMBLOCKS_LIBRARY_PATH)/memblocks.c
	make -C $(MEMBLOCKS_LIBRARY_PATH)
	gcc $(CFLAGS) $(TEMP_FLAGS) -o main main.o $(MEMBLOCKS_LIBRARY_PATH)/libmemblocks.a

main.o: main.c $(MEMBLOCKS_LIBRARY_PATH)/memblocks.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c -I $(MEMBLOCKS_LIBRARY_PATH)

# DYNAMIC LINK COMPILATION


# CLEAN

clean:
	rm -f *.a main *.o