CFLAGS=-O3 -lpthread -g -Wall -pthread
all:
	gcc memcpy.c -o memcpy $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypre -DPREPOPULATE $(CFLAGS) -lnuma

clean:
	rm memcpy memcpypre
