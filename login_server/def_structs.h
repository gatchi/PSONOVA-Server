//#define NO_ALIGN __declspec(align(1))   // This only works for visual studio i think
#define NO_ALIGN __attribute__((packed))  // Use this for gcc

/* Mag data structure */

typedef struct NO_ALIGN st_mag
{
	unsigned char two; // "02" =P
	unsigned char mtype;
	unsigned char level;
	unsigned char blasts;
	short defense;
	short power;
	short dex;
	short mind;
	unsigned itemid;
	char synchro;
	unsigned char IQ;
	unsigned char PBflags;
	unsigned char color;
} MAG;


/* Character Data Structure */

typedef struct NO_ALIGN st_minichar {
	unsigned short packetSize; // 0x00 - 0x01
	unsigned short command; // 0x02 - 0x03
	unsigned char flags[4]; // 0x04 - 0x07
	unsigned char unknown[8]; // 0x08 - 0x0F
	unsigned short level; // 0x10 - 0x11
	unsigned short reserved; // 0x12 - 0x13
	char gcString[10]; // 0x14 - 0x1D
	unsigned char unknown2[14]; // 0x1E - 0x2B
	unsigned char nameColorBlue; // 0x2C
	unsigned char nameColorGreen; // 0x2D
	unsigned char nameColorRed; // 0x2E
	unsigned char nameColorTransparency; // 0x2F
	unsigned short skinID; // 0x30 - 0x31
	unsigned char unknown3[18]; // 0x32 - 0x43
	unsigned char sectionID; // 0x44
	unsigned char _class; // 0x45
	unsigned char skinFlag; // 0x46
	unsigned char unknown4[5]; // 0x47 - 0x4B (same as unknown5 in E7)
	unsigned short costume; // 0x4C - 0x4D
	unsigned short skin; // 0x4E - 0x4F
	unsigned short face; // 0x50 - 0x51
	unsigned short head; // 0x52 - 0x53
	unsigned short hair; // 0x54 - 0x55
	unsigned short hairColorRed; // 0x56 - 0x57
	unsigned short hairColorBlue; // 0x58 - 0x59
	unsigned short hairColorGreen; // 0x5A - 0x5B
	unsigned proportionX; // 0x5C - 0x5F
	unsigned proportionY; // 0x60 - 0x63
	unsigned char name[24]; // 0x64 - 0x7B
	unsigned char unknown5[8] ; // 0x7C - 0x83
	unsigned playTime;
} MINICHAR;

//packetSize = 0x399C;
//command = 0x00E7;

typedef struct NO_ALIGN st_bank_item {
	unsigned char data[12]; // the standard $setitem1 - $setitem3 fare
	unsigned itemid; // player item id
	unsigned char data2[4]; // $setitem4 (mag use only)
	unsigned bank_count; // Why?
} BANK_ITEM;

typedef struct NO_ALIGN st_bank {
	unsigned bankUse;
	unsigned bankMeseta;
	BANK_ITEM bankInventory [200];
} BANK;

typedef struct NO_ALIGN st_item {
	unsigned char data[12]; // the standard $setitem1 - $setitem3 fare
	unsigned itemid; // player item id
	unsigned char data2[4]; // $setitem4 (mag use only)
} ITEM;

typedef struct NO_ALIGN st_inventory {
	unsigned in_use; // 0x01 = item slot in use, 0xFF00 = unused
	unsigned flags; // 8 = equipped
	ITEM item;
} INVENTORY;

typedef struct NO_ALIGN st_chardata {
	unsigned short packetSize; // 0x00-0x01  // Always set to 0x399C
	unsigned short command; // 0x02-0x03 // // Always set to 0x00E7
	unsigned char flags[4]; // 0x04-0x07
	unsigned char inventoryUse; // 0x08
	unsigned char HPuse; // 0x09
	unsigned char TPuse; // 0x0A
	unsigned char lang; // 0x0B
	INVENTORY inventory[30]; // 0x0C-0x353
	unsigned short ATP; // 0x354-0x355
	unsigned short MST; // 0x356-0x357
	unsigned short EVP; // 0x358-0x359
	unsigned short HP; // 0x35A-0x35B
	unsigned short DFP; // 0x35C-0x35D
	unsigned short TP; // 0x35E-0x35F
	unsigned short LCK; // 0x360-0x361
	unsigned short ATA; // 0x362-0x363
	unsigned char unknown[8]; // 0x364-0x36B  (Offset 0x360 has 0x0A value on Schthack's...)
	unsigned short level; // 0x36C-0x36D;
	unsigned short unknown2; // 0x36E-0x36F;
	unsigned XP; // 0x370-0x373
	unsigned meseta; // 0x374-0x377;
	char gcString[10]; // 0x378-0x381;
	unsigned char unknown3[14]; // 0x382-0x38F;  // Same as E5 unknown2
	unsigned char nameColorBlue; // 0x390;
	unsigned char nameColorGreen; // 0x391;
	unsigned char nameColorRed; // 0x392;
	unsigned char nameColorTransparency; // 0x393;
	unsigned short skinID; // 0x394-0x395;
	unsigned char unknown4[18]; // 0x396-0x3A7
	unsigned char sectionID; // 0x3A8;
	unsigned char _class; // 0x3A9;
	unsigned char skinFlag; // 0x3AA;
	unsigned char unknown5[5]; // 0x3AB-0x3AF;  // Same as E5 unknown4.
	unsigned short costume; // 0x3B0 - 0x3B1;
	unsigned short skin; // 0x3B2 - 0x3B3;
	unsigned short face; // 0x3B4 - 0x3B5;
	unsigned short head; // 0x3B6 - 0x3B7;
	unsigned short hair; // 0x3B8 - 0x3B9;
	unsigned short hairColorRed; // 0x3BA-0x3BB;
	unsigned short hairColorBlue; // 0x3BC-0x3BD;
	unsigned short hairColorGreen; // 0x3BE-0x3BF;
	unsigned proportionX; // 0x3C0-0x3C3;
	unsigned proportionY; // 0x3C4-0x3C7;
	unsigned char name[24]; // 0x3C8-0x3DF;
	unsigned playTime; // 0x3E0 - 0x3E3
	unsigned char unknown6[4]; // 0x3E4 - 0x3E7;
	unsigned char keyConfig[232]; // 0x3E8 - 0x4CF;
								// Stored from ED 07 packet.
	unsigned char techniques[20]; // 0x4D0 - 0x4E3;
	unsigned char unknown7[16]; // 0x4E4 - 0x4F3;
	unsigned char options[4]; // 0x4F4-0x4F7;
								// Stored from ED 01 packet.
	unsigned char unknown8[520]; // 0x4F8 - 0x6FF;
	unsigned bankUse; // 0x700 - 0x703
	unsigned bankMeseta; // 0x704 - 0x707;
	BANK_ITEM bankInventory [200]; // 0x708 - 0x19C7
	unsigned guildCard; // 0x19C8-0x19CB;
						// Stored from E8 06 packet.
	unsigned char name2[24]; // 0x19CC - 0x19E3;
	unsigned char unknown9[232]; // 0x19E4-0x1ACB;
	unsigned char reserved1;  // 0x1ACC; // Has value 0x01 on Schthack's
	unsigned char reserved2; // 0x1ACD; // Has value 0x01 on Schthack's
	unsigned char sectionID2; // 0x1ACE;
	unsigned char _class2; // 0x1ACF;
	unsigned char unknown10[4]; // 0x1AD0-0x1AD3;
	unsigned char symbol_chats[1248];	// 0x1AD4 - 0x1FB3
										// Stored from ED 02 packet.
	unsigned char shortcuts[2624];	// 0x1FB4 - 0x29F3
									// Stored from ED 03 packet.
	unsigned char unknown11[344]; // 0x29F4 - 0x2B4B;
	unsigned char GCBoard[172]; // 0x2B4C - 0x2BF7;
	unsigned char unknown12[200]; // 0x2BF8 - 0x2CBF;
	unsigned char challengeData[320]; // 0x2CC0 - 0X2DFF
	unsigned char unknown13[172]; // 0x2E00 - 0x2EAB;
	unsigned char unknown14[276]; // 0x2EAC - 0x2FBF; // I don't know what this is, but split from unknown13 because this chunk is
													  // actually copied into the 0xE2 packet during login @ 0x08
	unsigned char keyConfigGlobal[364]; // 0x2FC0 - 0x312B  // Copied into 0xE2 login packet @ 0x11C
										// Stored from ED 04 packet.
	unsigned char joyConfigGlobal[56]; // 0x312C - 0x3163 // Copied into 0xE2 login packet @ 0x288
										// Stored from ED 05 packet.
	unsigned guildCard2; // 0x3164 - 0x3167 (From here on copied into 0xE2 login packet @ 0x2C0...)
	unsigned teamID; // 0x3168 - 0x316B
	unsigned char teamInformation[8]; // 0x316C - 0x3173 (usually blank...)
	unsigned short privilegeLevel; // 0x3174 - 0x3175
	unsigned short reserved3; // 0x3176 - 0x3177
	unsigned char teamName[28]; // 0x3178 - 0x3193
	unsigned unknown15; // 0x3194 - 0x3197
	unsigned char teamFlag[2048]; // 0x3198 - 0x3997
	unsigned char teamRewards[8]; // 0x3998 - 0x39A0
} CHARDATA;


/* Player Structure */

typedef struct st_player {
	int plySockfd;
	int login;
	unsigned char peekbuf[8];
	unsigned char rcvbuf [TCP_BUFFER_SIZE];
	unsigned short rcvread;
	unsigned short expect;
	unsigned char decryptbuf [TCP_BUFFER_SIZE];
	unsigned char sndbuf [TCP_BUFFER_SIZE];
	unsigned char encryptbuf [TCP_BUFFER_SIZE];
	int snddata, 
		sndwritten;
	unsigned char packet [TCP_BUFFER_SIZE];
	unsigned short packetdata;
	unsigned short packetread;
	int crypt_on;
	PSO_CRYPT server_cipher, client_cipher;
	unsigned guildcard;
	char guildcard_string[12];
	unsigned char guildcard_data[20000];
	int sendingchars;
	short slotnum;
	unsigned lastTick;		// The last second
	unsigned toBytesSec;	// How many bytes per second the server sends to the client
	unsigned fromBytesSec;	// How many bytes per second the server receives from the client
	unsigned packetsSec;	// How many packets per second the server receives from the client
	unsigned connected;
	unsigned char sendCheck[MAX_SENDCHECK+2];
	int todc;
	unsigned char IP_Address[16];
	char hwinfo[18];
	int isgm;
	int dress_flag;
	unsigned connection_index;
} PLAYER;

typedef struct st_dressflag {
	unsigned guildcard;
	unsigned flagtime;
} DRESSFLAG;

/* a RC4 expanded key session */
struct rc4_key {
    unsigned char state[256];
    unsigned x, y;
};

/* Ship Structure */

typedef struct st_orange {
	int shipSockfd;
	unsigned char name[13];
	unsigned playerCount;
	unsigned char shipAddr[5];
	unsigned char listenedAddr[4];
	unsigned short shipPort;
	unsigned char rcvbuf [TCP_BUFFER_SIZE];
	unsigned long rcvread;
	unsigned long expect;
	unsigned char decryptbuf [TCP_BUFFER_SIZE];
	unsigned char sndbuf [PACKET_BUFFER_SIZE];
	unsigned char encryptbuf [TCP_BUFFER_SIZE];
	unsigned char packet [PACKET_BUFFER_SIZE];
	unsigned long packetread;
	unsigned long packetdata;
	int snddata, 
		sndwritten;
	unsigned shipID;
	int authed;
	int todc;
	int crypt_on;
	unsigned char user_key[128];
	int key_change[128];
	unsigned key_index;
	struct rc4_key cs_key; // Encryption keys
	struct rc4_key sc_key; // Encryption keys
	unsigned connection_index;
	unsigned connected;
	unsigned last_ping;
	int sent_ping;
} ORANGE;
