all: clean server client

server: GBNserver.c
	gcc GBNserver.c -lm -o server

client: GBNclient.c
	gcc GBNclient.c -lm -o client

clean:
	rm -f server client

# Using the -DDEBUG flag allows you to use the
# #ifdefn DEBUG
#  Do some arbitrary debug code
# #endif
