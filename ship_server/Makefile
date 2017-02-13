# Ship Server Makefile

ship-server.exe: ship_server.o mtwist.o
	gcc -o ship-server.exe ship_server.o -lws2_32 mtwist.o
	
ship_server.o: ship_server.c
	gcc -c ship_server.c

mtwist.o: ../mtwist/mtwist.c
	gcc -c ../mtwist/mtwist.c

bin: ship_server.o mtwist.o
	gcc -o ../bin/ship-server.exe ship_server.o -lws2_32 mtwist.o

clean:
	rm *.o
