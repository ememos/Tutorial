CFLAGS=-O3 -lpthread -g -Wall -pthread -fno-builtin
all:
	gcc memcpy.c -o memcpy $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypre -DPREPOPULATE $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpy.numa -DNUMA $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypre.numa -DNUMA -DPREPOPULATE $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpy.mp -DMP $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypre.mp -DPREPOPULATE -DMP $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpy.numa.mp -DNUMA -DMP $(CFLAGS) -lnuma
	gcc memcpy.c -o memcpypre.numa.mp -DPREPOPULATE -DNUMA -DMP $(CFLAGS) -lnuma

clean:
	rm memcpy memcpypre memcpy.numa memcpypre.numa memcpy.mp memcpypre.mp memcpy.numa.mp memcpypre.numa.mp
