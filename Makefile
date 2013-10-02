all: clean server client

server: GBNserver.c
	gcc -Wall udp_server.c -o server

client: GBNclient.c
	gcc -Wall udp_client.c -o client

clean:
	rm -f server client
