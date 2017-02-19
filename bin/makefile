# Bin Makefile

CC = gcc
INCLUDES = -I../md5/include -I../mysql/include -I../prs
WINSOCK = -lws2_32
MYSQL = -L../mysql/lib -llibmysql
SHIP_DIR = ../ship_server
SHIP_H = $(SHIP_DIR)/resource.h $(SHIP_DIR)/pso_crypt.h $(SHIP_DIR)/bbtable.h $(SHIP_DIR)/localgms.h $(SHIP_DIR)/prs.cpp $(SHIP_DIR)/def_map.h \
         $(SHIP_DIR)/def_block.h $(SHIP_DIR)/def_packets.h $(SHIP_DIR)/def_structs.h $(SHIP_DIR)/def_tables.h

all: account_add login char_export convert_quest convert_unitxt make_key newtable patch ship

.SECONDEXPANSION:

account_add char_export convert_quest: $$@.o md5.o
	$(CC) -o $@ $< $(MYSQL) md5.o

patch: patch_server.o ../patch_server/resource.h
	$(CC) -o $@ $< $(WINSOCK)

login: login_server.o md5.o ../login_server/def_structs.h
	$(CC) -o $@ $< $(WINSOCK) $(MYSQL) md5.o

ship: mtwist.o ship_server.o $(SHIP_H)
	$(CC) -o $@ mtwist.o ship_server.o $(WINSOCK)

make_key: mtwist.o make_key.o
	$(CC) -o $@ $^ $(MYSQL)

newtable: mtwist.o newtable.o
	$(CC) -o $@ $^

convert_unitxt: convert_unitxt.o

%.o: ../$$*/%.c
	$(CC) -c $(INCLUDES) $<

clean:
	rm *.o

reset:
	rm *.o *.exe
