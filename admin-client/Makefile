# admin-client Makefile

CC = gcc
CFLAGS = -g
WS2LIB = -lws2_32

admin-client: admin-client.o admin-client.h ship-funcs.h
	$(CC) $(CFLAGS) -o $@ $< $(WS2LIB)

admin-client.o: admin-client.c admin-client.h ship-funcs.h

clean:
	rm *.o
