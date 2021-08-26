


CFLAGS=-O3 -lpthread -g -Wall -pthread -fno-builtin
all:
	gcc memcpy.c -o memcpy $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypre -DPREPOPULATE $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpy.numa -DNUMA $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypre.numa -DNUMA -DPREPOPULATE $(CFLAGS) -lnuma

clean:
	rm memcpy memcpypre memcpy.numa memcpypre.numa
