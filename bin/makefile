# Bin Makefile

ifeq ($(OS),Windows_NT) 
    MYSQL = -L../mysql/lib -llibmysql
else
    MYSQL = -lmysqlclient
endif

CC = gcc
INCLUDES = -I../md5/include -I../mysql/include -I../prs
WINSOCK = -lws2_32
SHIP_DIR = ../ship_server
HEADERS = $(PATCH_H) $(LOGIN_H) $(SHIP_H)
PATCH_H = ../patch_server/resource.h
LOGIN_H = ../login_server/def_structs.h
SHIP_H = $(SHIP_DIR)/resource.h $(SHIP_DIR)/pso_crypt.h $(SHIP_DIR)/bbtable.h $(SHIP_DIR)/localgms.h $(SHIP_DIR)/prs.cpp $(SHIP_DIR)/def_map.h \
         $(SHIP_DIR)/def_block.h $(SHIP_DIR)/def_packets.h $(SHIP_DIR)/def_structs.h $(SHIP_DIR)/def_tables.h $(FUNC_H)
FUNC_H = $(SHIP_DIR)/funcs1.h $(SHIP_DIR)/funcs2.h $(SHIP_DIR)/funcs3.h $(SHIP_DIR)/funcs4.h $(SHIP_DIR)/file-funcs.h $(SHIP_DIR)/load-funcs.h \
         $(SHIP_DIR)/mag-funcs.h $(SHIP_DIR)/string-funcs.h

all: account_add login char_export convert_quest convert_unitxt make_key newtable patch ship

.SECONDEXPANSION:

account_add char_export convert_quest: $$@.o md5.o
	$(CC) -o $@ $< $(MYSQL) md5.o

patch: patch_server.o $(PATCH_H)
	$(CC) -o $@ $< $(WINSOCK)

login: login_server.o md5.o $(LOGIN_H)
	$(CC) -o $@ $< $(WINSOCK) $(MYSQL) md5.o

ship: mtwist.o ship_server.o $(SHIP_H)
	$(CC) -o $@ mtwist.o ship_server.o $(WINSOCK)

ship.d: ship_server.d.o mtwist.o $(SHIP_H)
	$(CC) -o $@ $< $(WINSOCK) mtwist.o

make_key: mtwist.o make_key.o
	$(CC) -o $@ $^ $(MYSQL)

newtable: mtwist.o newtable.o
	$(CC) -o $@ $^

convert_unitxt: convert_unitxt.o

patch_server.o: ../$$*/$$*.c $(PATCH_H)
	$(CC) -c $(INCLUDES) $<

login_server.o: ../$$*/$$*.c $(LOGIN_H)
	$(CC) -c $(INCLUDES) $<

ship_server.o: ../$$*/$$*.c $(SHIP_H)
	$(CC) -c $(INCLUDES) $<

ship_server.d.o: ../ship_server/ship_server.c $(SHIP_H)
	$(CC) -g -c -o ship_server.d.o $(INCLUDES) $<

%.o: ../$$*/%.c
	$(CC) -c $(INCLUDES) $<

clean:
	rm *.o

reset:
	rm *.o *.exe
