# Newtable Makefile

newtable: newtable.o mtwist.o
	gcc -o ../bin/newtable newtable.o mtwist.o

newtable.o: newtable.c
	gcc -c newtable.c

mtwist.o: ../mtwist/mtwist.c
	gcc -c ../mtwist/mtwist.c

clean:
	rm *.o newtable.exe
