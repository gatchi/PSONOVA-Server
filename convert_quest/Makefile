# convert_quest.c Makefile

all: convert_quest.o
	gcc -o ../bin/convert_quest convert_quest.o -L../mysql/lib -llibmysql

convert_quest.o: convert_quest.c
	gcc -I../mysql/include -c convert_quest.c
