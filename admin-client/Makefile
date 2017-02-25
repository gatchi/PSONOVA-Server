# admin-client Makefile

CC = gcc
CFLAGS = -g
WS2LIB = -lws2_32

admin-client: admin-client.o admin-client.h
	$(CC) $(CFLAGS) -o $@ $< $(WS2LIB)

admin-client.o: admin-client.c
