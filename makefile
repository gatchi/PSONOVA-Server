# Bin Makefile

ROOT_DIR = .
SRC_DIR = src
SERVER_DIR = server
ifeq ($(OS),Windows_NT) 
    MYSQL = -L $(SRC_DIR)/mysql/lib -llibmysql
else
    MYSQL = -lmysqlclient
endif

CC = gcc
INCLUDES = -I $(SRC_DIR)/md5/include -I $(SRC_DIR)/mysql/include -I $(SRC_DIR)/prs
WINSOCK = -lws2_32
LOGIN_DIR = $(ROOT_DIR)/$(SERVER_DIR)/login
SHIP_DIR = $(ROOT_DIR)/$(SERVER_DIR)/ship
UTIL_DIR = $(ROOT_DIR)/$(SRC_DIR)/util
SHIP_SRC_DIR = $(SRC_DIR)/ship_server
HEADERS = $(PATCH_H) $(LOGIN_H) $(SHIP_H)
PATCH_H = $(SRC_DIR)/patch_server/resource.h
LOGIN_H = $(SRC_DIR)/login_server/def_structs.h
SHIP_H = $(SHIP_SRC_DIR)/resource.h $(SHIP_SRC_DIR)/pso_crypt.h $(SHIP_SRC_DIR)/bbtable.h $(SHIP_SRC_DIR)/localgms.h \
         $(SHIP_SRC_DIR)/prs.cpp $(SHIP_SRC_DIR)/def_map.h $(SHIP_SRC_DIR)/def_block.h $(SHIP_SRC_DIR)/def_packets.h \
         $(SHIP_SRC_DIR)/def_structs.h $(SHIP_SRC_DIR)/def_tables.h $(FUNC_H)
FUNC_H = $(SHIP_SRC_DIR)/funcs1.h $(SHIP_SRC_DIR)/funcs2.h $(SHIP_SRC_DIR)/funcs3.h $(SHIP_SRC_DIR)/funcs4.h \
         $(SHIP_SRC_DIR)/file-funcs.h $(SHIP_SRC_DIR)/load-funcs.h $(SHIP_SRC_DIR)/mag-funcs.h $(SHIP_SRC_DIR)/string-funcs.h

all: account_add login char_export convert_quest convert_unitxt make_key newtable patch ship

.SECONDEXPANSION:

account_add: $$@.o md5.o
	$(CC) -o $@ $< $(MYSQL) md5.o
	cp $@.exe $(LOGIN_DIR)

char_export convert_quest: $$@.o md5.o
	$(CC) -o $@ $< $(MYSQL) md5.o
	cp $@.exe $(UTIL_DIR)

patch: patch_server.o $(PATCH_H)
	$(CC) -o $@ $< $(WINSOCK)
	cp $@.exe $(LOGIN_DIR)

login: login_server.o md5.o $(LOGIN_H)
	$(CC) -o $@ $< $(WINSOCK) $(MYSQL) md5.o
	cp $@.exe $(LOGIN_DIR)

ship: mtwist.o ship_server.o $(SHIP_H)
	$(CC) -o $@ mtwist.o ship_server.o $(WINSOCK)
	cp $@.exe $(SHIP_DIR)

ship.d: ship_server.d.o mtwist.o $(SHIP_H)
	$(CC) -o $@ $< $(WINSOCK) mtwist.o

make_key: mtwist.o make_key.o
	$(CC) -o $@ $^ $(MYSQL)
	cp $@.exe $(LOGIN_DIR)

newtable: mtwist.o newtable.o
	$(CC) -o $@ $^
	cp $@.exe $(UTIL_DIR)

convert_unitxt: convert_unitxt.o
	$(CC) -o $@ $^
	cp $@.exe $(UTIL_DIR)

patch_server.o: $(SRC_DIR)/$$*/$$*.c $(PATCH_H)
	$(CC) -c $(INCLUDES) $<

login_server.o: $(SRC_DIR)/$$*/$$*.c $(LOGIN_H)
	$(CC) -c $(INCLUDES) $<

ship_server.o: $(SRC_DIR)/$$*/$$*.c $(SHIP_H)
	$(CC) -c $(INCLUDES) $<

ship_server.d.o: $(SRC_DIR)/ship_server/ship_server.c $(SHIP_H)
	$(CC) -g -c -o ship_server.d.o $(INCLUDES) $<

%.o: $(SRC_DIR)/$$*/%.c
	$(CC) -c $(INCLUDES) $<

clean:
	rm *.o

reset:
	rm *.o *.exe
