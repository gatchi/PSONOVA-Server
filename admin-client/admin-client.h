//#define NO_ALIGN __attribute__((packed))
#define KEY_LENGTH 48

//---- Structs -------------//

typedef struct st_keyring {
	unsigned char * login;
	unsigned char * block;
} KEYRING;

typedef struct st_cipherlist {
	PSO_CRYPT * login;
	PSO_CRYPT * block;
} CIPHERLIST;

typedef struct st_server {
	unsigned char * name;
	SOCKET socket;
	struct sockaddr_in sa;
	unsigned char * key;
	unsigned char * ckey;
	PSO_CRYPT * cipher;
	PSO_CRYPT * ccipher;
} SERVER;

typedef struct st_client {
	struct st_keyring key;
	struct st_cipherlist cipher;
} CLIENT;

//---- Packet Templates ----//

// Logon
unsigned char Packet93[] = {
	0x7D, 00, 0x93, 00, 00, 00, 00, 00, 00, 00, 00, 01, 00, 00, 00,	00,
	00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,	00, 00, 00,
	00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,	00, 00, 00, 00, 00,
	00, 00, 00, 00, 00, 00, 00, 00, 00, 00,	00, 00, 00, 00, 00, 00, 00,
	00, 00, 00, 00, 00, 00, 00, 00,	00, 00, 00, 00, 00, 00, 00, 00, 00,
	00, 00, 00, 00, 00, 00,	00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
	00, 00, 00, 00,	00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00,
	00, 00,	00, 00, 00, 00, 00, 00, 00, 00, 00, 00, 00
};

#define PACKET_93_SIZE 0x79

// Disconnect
unsigned char Packet05[] = {
	8, 00, 5, 00, 00, 00, 00, 00
};

// Ping
unsigned char Packet1D[] = {
	8, 0, 0x1D, 0, 0, 0, 0, 0
};
