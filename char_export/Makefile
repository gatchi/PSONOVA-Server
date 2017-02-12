# char_export.c Makefile

all: char_export.o
	gcc -o ../bin/char_export char_export.o -L../mysql/lib -llibmysql

char_export.o: char_export.c
	gcc -I../mysql/include -c char_export.c
