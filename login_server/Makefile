# Login Server Makefile

login_server: login_server.o md5.o
	gcc -o ../bin/login_server login_server.o -lws2_32 -L../mysql/lib -llibmysql md5.o
	
login_server.o: login_server.c
	gcc -I../md5/include -I../prs -c -I../mysql/include login_server.c

md5.o: ../md5/md5.c
	gcc -c ../md5/md5.c
