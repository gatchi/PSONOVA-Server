# Ship Server Makefile

ship_server: ship_server.o mtwist.o
	gcc -o ../bin/ship_server ship_server.o -lws2_32 mtwist.o
	
ship_server.o: ship_server.c
	gcc -c ship_server.c

mtwist.o: ../mtwist/mtwist.c
	gcc -c ../mtwist/mtwist.c

clean:
	rm *.o
