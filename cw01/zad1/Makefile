TEMP_FLAGS=-g -Wall

all: memblocks_test shared
static: memblocks_test

memblocks_test: libmemblocks.a main.o
	gcc $(CFLAGS) $(TEMP_FLAGS) main.o libmemblocks.a -o memblocks_test

main.o: main.c memblocks.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c main.c

libmemblocks.a: memblocks.o
	ar rcs libmemblocks.a memblocks.o

memblocks.o: memblocks.c memblocks.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -c memblocks.c

shared: memblocks_test_shared

memblocks_test_shared: libmemblocks.so main.o
	gcc $(CFLAGS) $(TEMP_FLAGS) main.o -lmemblocks -L. -o memblocks_test_shared

libmemblocks.so: memblocks.o_shared
	gcc $(CFLAGS) $(TEMP_FLAGS) -shared	-fPIC -o libmemblocks.so memblocks.o
	sudo cp libmemblocks.so /usr/lib
	sudo chmod 0755 /usr/lib/libmemblocks.so
	sudo ldconfig

memblocks.o_shared: memblocks.c memblocks.h
	gcc $(CFLAGS) $(TEMP_FLAGS) -fPIC -c memblocks.c

clean:
	rm -f *.o *.a *.so* memblocks_test*
