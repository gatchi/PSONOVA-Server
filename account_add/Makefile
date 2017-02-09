# account_add.exe Makefile

all: account_add.o md5.o
	gcc -o account_add account_add.o -L../mysql/lib -llibmysql md5.o

account_add.o: account_add.c
	gcc -I../mysql/include -c account_add.c

md5.o: ../md5/md5.c
	gcc -c ../md5/md5.c
