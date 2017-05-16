# char_export.c Makefile

ifeq ($(OS),Windows_NT) 
    MYSQL = -L../mysql/lib -llibmysql
else
    MYSQL = -lmysqlclient
endif

all: char_export.o
	gcc -o char_export char_export.o $(MYSQL)

char_export.o: char_export.c
	gcc -I../mysql/include -c char_export.c
