# Linux Account Add Makefile

CFLAGS = 
CC = gcc

account-add: account_add.o md5.o
	$(CC) $(CFLAGS) -o $@ $< -L/usr/lib/x86_64-linux-gnu -lmysqlclient md5.o

account_add.o: account_add.c
	$(CC) $(CFLAGS) -I../mysql/include -I../md5/include -c account_add.c

md5.o: ../md5/md5.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm *.o
