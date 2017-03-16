# make_key Makefile

make_key: make_key.o mtwist.o
	gcc -o make_key make_key.o -L../mysql/lib -llibmysql mtwist.o

make_key.o: make_key.c
	gcc -I../mysql/include -c make_key.c

mtwist.o: ../mtwist/mtwist.c
	gcc -c ../mtwist/mtwist.c
