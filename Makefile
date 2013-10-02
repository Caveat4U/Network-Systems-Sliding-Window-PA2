all: clean server client

server: GBNserver.c
	gcc -Wall GBNserver.c -o server #-DDEBUG

client: GBNclient.c
	gcc -Wall GBNclient.c -o client

clean:
	rm -f server client
