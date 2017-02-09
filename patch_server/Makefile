# Patch Server Makefile

patch_server: patch_server.o
	gcc -o patch_server patch_server.o -lws2_32

patch_server.o: patch_server.c
	gcc -c patch_server.c
