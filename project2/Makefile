CC = gcc
CFLAGS = -g -pthread

all: webserver client 

webserver: webserver.c net.c webserver.h
	$(CC) $(CFLAGS) -o $@ webserver.c net.c

client: client.c
	$(CC) $(CFLAGS) -o $@ client.c

clean:
	rm -f webserver client
