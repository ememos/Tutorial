CFLAGS=-O3 -lpthread -g -Wall -pthread -fno-builtin
all:
	gcc memcpy.c -o memcpy $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypre -DPREPOPULATE $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpy.numa -DNUMA $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypre.numa -DNUMA -DPREPOPULATE $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpythread -DTHREAD $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpythread.numa -DTHREAD -DNUMA $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypernode -DPERNODE $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypernode.numa -DPERNODE -DNUMA $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypernodepre -DPERNODE -DPREPOPULATE $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypernodepre.numa -DPERNODE -DPREPOPULATE -DNUMA $(CFLAGS) -lnuma

clean:
	rm memcpy memcpypre memcpy.numa memcpypre.numa memcpythread memcpythread.numa memcpypernode memcpypernode.numa memcpypernodepre memcpypernodepre.numa
