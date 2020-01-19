all: dir binary

dir:
	mkdir -p bin/

binary: server client

server:
	gcc -g -o bin/server server.c -lpthread

client:
	gcc  -shared -o libsystemcmd.so -fPIC client.c -lpthread
	gcc -g -o bin/test_client test_client.c -L . -lsystemcmd
#	export library path for test_client binary
#	export LD_LIBRARY_PATH=.

clean:
	rm -rf bin
