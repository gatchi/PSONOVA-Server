# admin-client Makefile

CC = gcc
WS2LIB = -lws2_32

admin-client: admin-client.o admin-client.h
	$(CC) -o $@ $< $(WS2LIB)

admin-client.o: admin-client.c
