TEMP_FLAGS=-g -Wall
MEMBLOCKS_LIBRARY_PATH=../zad1
SPEED_TEST_PATH=../zad2

all: speed_test_static speed_test_shared speed_test_dynamic

# STATIC COMPILATION

speed_test_static: speed_test.o $(MEMBLOCKS_LIBRARY_PATH)/libmemblocks.a
	make libmemblocks.a -C $(MEMBLOCKS_LIBRARY_PATH)
	gcc $(CFLAGS) $(TEMP_FLAGS) -o speed_test_static $(SPEED_TEST_PATH)/speed_test.o \
		$(SPEED_TEST_PATH)/stop_watch.o $(MEMBLOCKS_LIBRARY_PATH)/libmemblocks.a

speed_test.o: $(SPEED_TEST_PATH)/speed_test.c $(MEMBLOCKS_LIBRARY_PATH)/memblocks.h $(SPEED_TEST_PATH)/stop_watch.o
	gcc $(CFLAGS) $(TEMP_FLAGS) -c $(SPEED_TEST_PATH)/speed_test.c \
		-I $(MEMBLOCKS_LIBRARY_PATH) -I $(SPEED_TEST_PATH)

stop_watch.o: $(SPEED_TEST_PATH)/stop_watch.c $(SPEED_TEST_PATH)/stop_watch.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c $(SPEED_TEST_PATH)/stop_watch.c

# SHARED COMPILATION

speed_test_shared: speed_test.o_shared stop_watch.o_shared libmemblocks.so
	gcc $(CFLAGS) $(TEMP_FLAGS) -o speed_test_shared stop_watch.o_shared speed_test.o_shared -lmemblocks -L$(MEMBLOCKS_LIBRARY_PATH)

libmemblocks.so:
	make libmemblocks.so -C $(MEMBLOCKS_LIBRARY_PATH)

speed_test.o_shared: $(SPEED_TEST_PATH)/stop_watch.h $(SPEED_TEST_PATH)/speed_test.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -fPIC -c $(SPEED_TEST_PATH)/speed_test.c \
		-I $(MEMBLOCKS_LIBRARY_PATH) -I $(SPEED_TEST_PATH) -o speed_test.o_shared

stop_watch.o_shared: $(SPEED_TEST_PATH)/stop_watch.c $(SPEED_TEST_PATH)/stop_watch.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -fPIC -c $(SPEED_TEST_PATH)/stop_watch.c -o stop_watch.o_shared

# DYNAMIC LINK COMPILATION

speed_test_dynamic: speed_test.o_dynamic libmemblocks.so
	gcc $(CFLAGS) $(TEMP_FLAGS) -o speed_test_dynamic stop_watch.o_shared speed_test.o_dynamic -ldl

speed_test.o_dynamic: $(SPEED_TEST_PATH)/stop_watch.h $(SPEED_TEST_PATH)/speed_test.c
	gcc $(CFLAGS) $(TEMP_FLAGS) -D DYNAMIC_LOAD -fPIC -c $(SPEED_TEST_PATH)/speed_test.c \
		-I $(MEMBLOCKS_LIBRARY_PATH) -I $(SPEED_TEST_PATH) -o speed_test.o_dynamic

# CLEAN

clean:
	rm -f *.a speed_test speed_test_static speed_test_shared *.o *.o_shared *.o_dynamic speed_test_dynamic
