/************************************************************************
	Tethealla Ship Server
	Copyright (C) 2008  Terry Chatman Jr.

	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License version 3 as
	published by the Free Software Foundation.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
************************************************************************/

#define reveal_window \
	ShowWindow ( consoleHwnd, SW_NORMAL ); \
	SetForegroundWindow ( consoleHwnd ); \
	SetFocus ( consoleHwnd )

#define swapendian(x) ( ( x & 0xFF ) << 8 ) + ( x >> 8 )
#define FLOAT_PRECISION 0.00001

// To do:
//
// Firewall for Team
//
// Challenge
//
// Allow quests to be reloaded while people are in them... somehow!

/* Local To do:
 *
 * Allow safe exit & restart
 * Allow simple mail/announcements from terminal
 * Automatic shutdown mail when not using quick shutdown
 */

#define SERVER_VERSION "PSONOVA 0.2.1"  // Based of of 0.144
#define DEBUG 1

#define USEADDR_ANY
#define LOGON_PORT 3455
#define TCP_BUFFER_SIZE 64000
#define PACKET_BUFFER_SIZE ( TCP_BUFFER_SIZE * 16 )
//#define LOG_60
#define SHIP_COMPILED_MAX_CONNECTIONS 900
#define SHIP_COMPILED_MAX_GAMES 75
#define LOGIN_RECONNECT_SECONDS 15
#define MAX_SIMULTANEOUS_CONNECTIONS 6
#define MAX_SAVED_LOBBIES 20
#define MAX_SAVED_ITEMS 3000
#define MAX_GCSEND 2000
#define ALL_ARE_GM 0
#define PRS_BUFFER 262144

#define SEND_PACKET_03 0x00
#define RECEIVE_PACKET_93 0x0A
#define MAX_SENDCHECK 0x0B

// Our Character Classes

#define CLASS_HUMAR 0x00
#define CLASS_HUNEWEARL 0x01
#define CLASS_HUCAST 0x02
#define CLASS_RAMAR 0x03
#define CLASS_RACAST 0x04
#define CLASS_RACASEAL 0x05
#define CLASS_FOMARL 0x06
#define CLASS_FONEWM 0x07
#define CLASS_FONEWEARL 0x08
#define CLASS_HUCASEAL 0x09
#define CLASS_FOMAR 0x0A
#define CLASS_RAMARL 0x0B
#define CLASS_MAX 0x0C

// Class equip_flags

#define HUNTER_FLAG	1   // Bit 1
#define RANGER_FLAG	2   // Bit 2
#define FORCE_FLAG	4   // Bit 3
#define HUMAN_FLAG	8   // Bit 4
#define	DROID_FLAG	16  // Bit 5
#define	NEWMAN_FLAG	32  // Bit 6
#define	MALE_FLAG	64  // Bit 7
#define	FEMALE_FLAG	128 // Bit 8

// Rare rate multipliers (TODO: put in into .ini and make reload command)

#define GLOBAL_RARE_MULT 1
#define MOB_RARE_MULT 1
#define BOX_RARE_MULT 1

#include	<windows.h>
#include	<stdio.h>
#include	<string.h>
#include	<time.h>
#include	<math.h>
#include	<ctype.h>
#include	<Winsock2.h>

#include	"resource.h"
#include	"pso_crypt.h"
#include	"bbtable.h"
#include	"localgms.h"
#include	"prs.cpp"
#include	"def_map.h" // Map file name definitions
#include	"def_block.h" // Blocked packet definitions
#include	"def_packets.h" // Pre-made packet definitions
#include	"def_structs.h" // Various structure definitions
#include	"def_tables.h" // Various pre-made table definitions

const unsigned char Message03[] = { "Tethealla Ship v.144" };

/* function defintions */

#include "func_defs.h"

/* variables */

struct timeval select_timeout = { 
	0,
	5000
};

FILE* debugfile;

// Random drop rates

unsigned WEAPON_DROP_RATE,
ARMOR_DROP_RATE,
MAG_DROP_RATE,
TOOL_DROP_RATE,
MESETA_DROP_RATE,
EXPERIENCE_RATE;
unsigned common_rates[5] = { 0 };

// Rare monster appearance rates

unsigned	hildebear_rate, 
			rappy_rate,
			lily_rate,
			slime_rate,
			merissa_rate,
			pazuzu_rate,
			dorphon_rate,
			kondrieu_rate = 0;

unsigned common_counters[5] = {0};

unsigned char common_table[100000];

unsigned char PacketA0Data[0x4000] = {0};
unsigned char Packet07Data[0x4000] = {0};
unsigned short Packet07Size = 0;
unsigned char PacketData[TCP_BUFFER_SIZE];
unsigned char PacketData2[TCP_BUFFER_SIZE]; // Sometimes we need two...
unsigned char tmprcv[PACKET_BUFFER_SIZE];

/* Populated by load_config_file(): */

unsigned char serverIP[4] = {0};
int autoIP = 0;
unsigned char loginIP[4];
unsigned short serverPort;
unsigned short serverMaxConnections;
int ship_support_extnpc = 0;
unsigned serverNumConnections = 0;
unsigned blockConnections = 0;
unsigned blockTick = 0;
unsigned serverConnectionList[SHIP_COMPILED_MAX_CONNECTIONS];
unsigned short serverBlocks;
unsigned char shipEvent;
unsigned serverID = 0;
time_t servertime;
unsigned normalName = 0xFFFFFFFF;
unsigned globalName = 0xFF1D94F7;
unsigned localName = 0xFFB0C4DE;

unsigned short ship_banmasks[5000][4] = {0}; // IP address ban masks
BANDATA ship_bandata[5000];
unsigned num_masks = 0;
unsigned num_bans = 0;

/* Common tables */

PTDATA pt_tables_ep1[10][4];
PTDATA pt_tables_ep2[10][4];

// Episode I parsed PT data

unsigned short weapon_drops_ep1[10][4][10][4096];
unsigned char slots_ep1[10][4][4096];
unsigned char tech_drops_ep1[10][4][10][4096];
unsigned char tool_drops_ep1[10][4][10][4096];
unsigned char power_patterns_ep1[10][4][4][4096];
char percent_patterns_ep1[10][4][6][4096];
unsigned char attachment_ep1[10][4][10][4096];

// Episode II parsed PT data

unsigned short weapon_drops_ep2[10][4][10][4096];
unsigned char slots_ep2[10][4][4096];
unsigned char tech_drops_ep2[10][4][10][4096];
unsigned char tool_drops_ep2[10][4][10][4096];
unsigned char power_patterns_ep2[10][4][4][4096];
char percent_patterns_ep2[10][4][6][4096];
unsigned char attachment_ep2[10][4][10][4096];


/* Rare tables */

unsigned rt_tables_ep1[0x200 * 10 * 4] = {0}; 
unsigned rt_tables_ep2[0x200 * 10 * 4] = {0};
unsigned rt_tables_ep4[0x200 * 10 * 4] = {0};

unsigned char startingData[12*14];
playerLevel playerLevelData[12][200];

fd_set ReadFDs, WriteFDs, ExceptFDs;

saveLobby savedlobbies[MAX_SAVED_LOBBIES];
unsigned char dp[TCP_BUFFER_SIZE*4];
unsigned ship_ignore_list[300] = {0};
unsigned ship_ignore_count = 0;
unsigned ship_gcsend_list[MAX_GCSEND*3] = {0};
unsigned ship_gcsend_count = 0;
char Ship_Name[255];
SHIPLIST shipdata[200];
BLOCK* blocks[10];
QUEST quests[512];
QUEST_MENU quest_menus[12];
unsigned* quest_allow = 0; // the "allow" list for the 0x60CA command...
unsigned quest_numallows;
unsigned numQuests = 0;
unsigned questsMemory = 0;
char* languageExts[10];
char* languageNames[10];
unsigned numLanguages = 0;
unsigned totalShips = 0;
BATTLEPARAM ep1battle[374];
BATTLEPARAM ep2battle[374];
BATTLEPARAM ep4battle[332];
BATTLEPARAM ep1battle_off[374];
BATTLEPARAM ep2battle_off[374];
BATTLEPARAM ep4battle_off[332];
unsigned battle_count;
SHOP shops[7000];
unsigned shop_checksum;
unsigned shopidx[200];
unsigned ship_index;
unsigned char ship_key[128];

// New leet parameter tables!!!!111oneoneoneeleven

unsigned char armor_equip_table[256] = {0};
unsigned char barrier_equip_table[256] = {0};
unsigned char armor_level_table[256] = {0};
unsigned char barrier_level_table[256] = {0};
unsigned char armor_dfpvar_table[256] = {0};
unsigned char barrier_dfpvar_table[256] = {0};
unsigned char armor_evpvar_table[256] = {0};
unsigned char barrier_evpvar_table[256] = {0};
unsigned char weapon_equip_table[256][256] = {0};
unsigned short weapon_atpmax_table[256][256] = {0};
unsigned char grind_table[256][256] = {0};
unsigned char special_table[256][256] = {0};
unsigned char stackable_table[256] = {0};
unsigned equip_prices[2][13][24][80] = {0};
char max_tech_level[19][12];

PSO_CRYPT* cipher_ptr;

#define MYWM_NOTIFYICON (WM_USER+2)
int program_hidden = 1;
HWND consoleHwnd;

#include "string-funcs.h"
#include "file-funcs.h"

SERVER logon;
CLIENT * connections[SHIP_COMPILED_MAX_CONNECTIONS];
CLIENT * workConnect;
unsigned logon_tick = 0;
unsigned logon_ready = 0;

const char serverName[] = { "T\0E\0T\0H\0E\0A\0L\0L\0A\0" };
const char shipSelectString[] = {"S\0h\0i\0p\0 \0S\0e\0l\0e\0c\0t\0"};
const char blockString[] = {"B\0L\0O\0C\0K\0"};

#include "funcs1.h"

// FFFF

INVENTORY_ITEM sort_data[30];
BANK_ITEM bank_data[200];

#include "funcs2.h"
#include "mag-funcs.h"
#include "funcs3.h"

unsigned char chatBuf[4000];
unsigned char cmdBuf[4000];
char* myCommand;
char* myArgs[64];

char* Unicode_to_ASCII (unsigned short* ucs)
{
	char *s,c;

	s = (char*) &chatBuf[0];
	while (*ucs != 0x0000)
	{
		c = *(ucs++) & 0xFF;
		if ((c >= 0x20) && (c <= 0x80))
			*(s++) = c;
		else
			*(s++) = 0x20;
	}
	*(s++) = 0x00;
	return (char*) &chatBuf[0];
}

void WriteLog(char *fmt, ...)
{
#define MAX_GM_MESG_LEN 4096

	va_list args;
	char text[ MAX_GM_MESG_LEN ];
	SYSTEMTIME rawtime;

	FILE *fp;

	GetLocalTime (&rawtime);
	va_start (args, fmt);
	strcpy (text + vsprintf( text,fmt,args), "\r\n"); 
	va_end (args);

	fp = fopen ( "ship.log", "a");
	if (!fp)
	{
		printf ("Unable to log to ship.log\n");
	}

	fprintf (fp, "[%02u-%02u-%u, %02u:%02u] %s", rawtime.wMonth, rawtime.wDay, rawtime.wYear, 
		rawtime.wHour, rawtime.wMinute, text);
	fclose (fp);

	printf ("[%02u-%02u-%u, %02u:%02u] %s", rawtime.wMonth, rawtime.wDay, rawtime.wYear, 
		rawtime.wHour, rawtime.wMinute, text);
}


void WriteGM(char *fmt, ...)
{
#define MAX_GM_MESG_LEN 4096

	va_list args;
	char text[ MAX_GM_MESG_LEN ];
	SYSTEMTIME rawtime;

	FILE *fp;

	GetLocalTime (&rawtime);
	va_start (args, fmt);
	strcpy (text + vsprintf( text,fmt,args), "\r\n"); 
	va_end (args);

	fp = fopen ( "gm.log", "a");
	if (!fp)
	{
		printf ("Unable to log to gm.log\n");
	}

	fprintf (fp, "[%02u-%02u-%u, %02u:%02u] %s", rawtime.wMonth, rawtime.wDay, rawtime.wYear, 
		rawtime.wHour, rawtime.wMinute, text);
	fclose (fp);

	printf ("[%02u-%02u-%u, %02u:%02u] %s", rawtime.wMonth, rawtime.wDay, rawtime.wYear, 
		rawtime.wHour, rawtime.wMinute, text);
}


char character_file[255];

#include "funcs4.h"

unsigned char qpd_buffer  [PRS_BUFFER];
unsigned char qpdc_buffer [PRS_BUFFER];
//LOBBY fakelobby;

void LoadQuests (const char* filename, unsigned category)
{
	/*unsigned oldIndex;
	unsigned qm_length, qa, nr;
	unsigned char* qmap;
	LOBBY *l;*/
	FILE* fp;
	FILE* qf;
	FILE* qd;
	unsigned qs;
	char qfile[256];
	char qfile2[256];
	char qfile3[256];
	char qfile4[256];
	char qname[256];
	unsigned qnl = 0;
	QUEST* q;
	unsigned ch, ch2, ch3, ch4, ch5, qf2l;
	unsigned short qps, qpc;
	unsigned qps2;
	QUEST_MENU* qm;
	unsigned* ed;
	unsigned ed_size, ed_ofs;
	unsigned num_records, num_objects, qm_ofs = 0, qb_ofs = 0;
	char true_filename[16];
	QDETAILS* ql;
	int extf;

	qm = &quest_menus[category];
	printf ("Loading quest list from %s ... \n", filename);
	fp = fopen ( filename, "r");
	if (!fp)
	{
		printf ("%s is missing.\n", filename);
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	while (fgets (&qfile[0], 255, fp) != NULL)
	{
		for (ch=0;ch<strlen(&qfile[0]);ch++)
			if ((qfile[ch] == 10) || (qfile[ch] == 13))
				qfile[ch] = 0; // Reserved
		qfile3[0] = 0;
		strcat ( &qfile3[0], "quest\\");
		strcat ( &qfile3[0], &qfile[0] );
		memcpy ( &qfile[0], &qfile3[0], strlen ( &qfile3[0] ) + 1 );
		strcat ( &qfile3[0], "quest.lst");
		qf = fopen ( &qfile3[0], "r");
		if (!qf)
		{
			printf ("%s is missing.\n", qfile3);
			printf ("Press [ENTER] to quit...");
			gets(&dp[0]);
			exit (1);
		}
		if (fgets (&qname[0], 64, fp) == NULL)
		{
			printf ("%s is corrupted.\n", filename);
			printf ("Press [ENTER] to quit...");
			gets(&dp[0]);
			exit (1);
		}
		for (ch=0;ch<64;ch++)
		{
			if (qname[ch] != 0x00)
			{
				qm->c_names[qm->num_categories][ch*2] = qname[ch];
				qm->c_names[qm->num_categories][(ch*2)+1] = 0x00;
			}
			else
			{
				qm->c_names[qm->num_categories][ch*2] = 0x00;
				qm->c_names[qm->num_categories][(ch*2)+1] = 0x00;
				break;
			}
		}
		if (fgets (&qname[0], 120, fp) == NULL)
		{
			printf ("%s is corrupted.\n", filename);
			printf ("Press [ENTER] to quit...");
			gets(&dp[0]);
			exit (1);
		}
		for (ch=0;ch<120;ch++)
		{
			if (qname[ch] != 0x00)
			{
				if (qname[ch] == 0x24)
					qm->c_desc[qm->num_categories][ch*2] = 0x0A;
				else
					qm->c_desc[qm->num_categories][ch*2] = qname[ch];
				qm->c_desc[qm->num_categories][(ch*2)+1] = 0x00;
			}
			else
			{
				qm->c_desc[qm->num_categories][ch*2] = 0x00;
				qm->c_desc[qm->num_categories][(ch*2)+1] = 0x00;
				break;
			}
		}
		memcpy (&qfile2[0], &qfile[0], strlen (&qfile[0])+1);
		qf2l = strlen (&qfile2[0]);
		while (fgets (&qfile2[qf2l], 255, qf) != NULL)
		{
			for (ch=0;ch<strlen(&qfile2[0]);ch++)
				if ((qfile2[ch] == 10) || (qfile2[ch] == 13))
					qfile2[ch] = 0; // Reserved

			for (ch4=0;ch4<numLanguages;ch4++)
			{
				memcpy (&qfile4[0], &qfile2[0], strlen (&qfile2[0])+1);

				// Add extension to .qst and .raw for languages

				extf = 0;

				if (strlen(languageExts[ch4]))
				{
					if ( (strlen(&qfile4[0]) - qf2l) > 3)
						for (ch5=qf2l;ch5<strlen(&qfile4[0]) - 3;ch5++)
						{
							if ((qfile4[ch5] == 46) &&
								(tolower(qfile4[ch5+1]) == 113) &&
								(tolower(qfile4[ch5+2]) == 115) &&
								(tolower(qfile4[ch5+3]) == 116))
							{
								qfile4[ch5] = 0;
								strcat (&qfile4[ch5], "_");
								strcat (&qfile4[ch5], languageExts[ch4]);
								strcat (&qfile4[ch5], ".qst");
								extf = 1;
								break;
							}
						}

						if (( (strlen(&qfile4[0]) - qf2l) > 3) && (!extf))
							for (ch5=qf2l;ch5<strlen(&qfile4[0]) - 3;ch5++)
							{
								if ((qfile4[ch5] == 46) &&
									(tolower(qfile4[ch5+1]) == 114) &&
									(tolower(qfile4[ch5+2]) == 97) &&
									(tolower(qfile4[ch5+3]) == 119))
								{
									qfile4[ch5] = 0;
									strcat (&qfile4[ch5], "_");
									strcat (&qfile4[ch5], languageExts[ch4]);
									strcat (&qfile4[ch5], ".raw");
									break;
								}
							}
				}

				qd = fopen ( &qfile4[0], "rb" );
				if (qd != NULL)
				{
					if (ch4 == 0)
					{
						q = &quests[numQuests];
						memset (q, 0, sizeof (QUEST));
					}
					ql = q->ql[ch4] = malloc ( sizeof ( QDETAILS ));
					memset (ql, 0, sizeof ( QDETAILS ));
					fseek ( qd, 0, SEEK_END );
					ql->qsize = qs = ftell ( qd );
					fseek ( qd, 0, SEEK_SET );
					ql->qdata = malloc ( qs );
					questsMemory += qs;
					fread ( ql->qdata, 1, qs, qd );
					ch = 0;
					ch2 = 0;
					while (ch < qs)
					{
						qpc = *(unsigned short*) &ql->qdata[ch+2];
						if ( (qpc == 0x13) && (strstr(&ql->qdata[ch+8], ".bin")) && (ch2 < PRS_BUFFER))
						{
							memcpy ( &true_filename[0], &ql->qdata[ch+8], 16 );
							qps2 = *(unsigned*) &ql->qdata[ch+0x418];
							memcpy (&qpd_buffer[ch2], &ql->qdata[ch+0x18], qps2);
							ch2 += qps2;
						}
						else
							if (ch2 >= PRS_BUFFER)
							{
								printf ("PRS buffer too small...\n");
								printf ("Press [ENTER] to quit...");
								gets (&dp[0]);
								exit (1);
							}
							qps = *(unsigned short*) &ql->qdata[ch];
							if (qps % 8) 
								qps += ( 8 - ( qps % 8 ) );
							ch += qps;
					}
					ed_size = prs_decompress (&qpd_buffer[0], &qpdc_buffer[0]);
					if (ed_size > PRS_BUFFER)
					{
						printf ("Memory corrupted!\n", ed_size );
						printf ("Press [ENTER] to quit...");
						gets (&dp[0]);
						exit (1);
					}
					fclose (qd);

					if (ch4 == 0)
						qm->quest_indexes[qm->num_categories][qm->quest_counts[qm->num_categories]++] = numQuests++;

					qnl = 0;
					for (ch2=0x18;ch2<0x48;ch2+=2)
					{
						if ( *(unsigned short *) &qpdc_buffer[ch2] != 0x0000 )
						{
							qname[qnl] = qpdc_buffer[ch2];
							if (qname[qnl] < 32)
								qname[qnl] = 32;
							qnl++;
						}
						else
							break;
					}

					qname[qnl] = 0;
					memcpy (&ql->qname[0], &qpdc_buffer[0x18], 0x40);
					ql->qname[qnl] = 0x0000;
					memcpy (&ql->qsummary[0], &qpdc_buffer[0x58], 0x100);
					memcpy (&ql->qdetails[0], &qpdc_buffer[0x158], 0x200);

					if (ch4 == 0)
					{
						// Load enemy data

						ch = 0;
						ch2 = 0;

						while (ch < qs)
						{
							qpc = *(unsigned short*) &ql->qdata[ch+2];
							if ( (qpc == 0x13) && (strstr(&ql->qdata[ch+8], ".dat")) && (ch2 < PRS_BUFFER))
							{
								qps2 = *(unsigned *) &ql->qdata[ch+0x418];
								memcpy (&qpd_buffer[ch2], &ql->qdata[ch+0x18], qps2);
								ch2 += qps2;
							}
							else
								if (ch2 >= PRS_BUFFER)
								{
									printf ("PRS buffer too small...\n");
									printf ("Press [ENTER] to quit...");
									gets (&dp[0]);
									exit (1);
								}

								qps = *(unsigned short*) &ql->qdata[ch];
								if (qps % 8) 
									qps += ( 8 - ( qps % 8 ) );
								ch += qps;
						}
						ed_size = prs_decompress (&qpd_buffer[0], &qpdc_buffer[0]);
						if (ed_size > PRS_BUFFER)
						{
							printf ("Memory corrupted!\n", ed_size );
							printf ("Press [ENTER] to quit...");
							gets (&dp[0]);
							exit (1);
						}
						ed_ofs = 0;
						ed = (unsigned*) &qpdc_buffer[0];
						qm_ofs = 0;
						qb_ofs = 0;
						num_objects = 0;
						while ( ed_ofs < ed_size )
						{
							switch ( *ed )
							{
							case 0x01:
								if (ed[2] > 17)
								{
									printf ("Area out of range in quest!\n");
									printf ("Press [ENTER] to quit...");
									gets (&dp[0]);
									exit(1);
								}
								num_records = ed[3] / 68L;
								num_objects += num_records;
								*(unsigned *) &qpd_buffer[qb_ofs] = *(unsigned *) &ed[2];
								qb_ofs += 4;
								//printf ("area: %u, object count: %u\n", ed[2], num_records);
								*(unsigned *) &qpd_buffer[qb_ofs] = num_records;
								qb_ofs += 4;
								memcpy ( &qpd_buffer[qb_ofs], &ed[4], ed[3] );
								qb_ofs += num_records * 68L;
								ed_ofs += ed[1]; // Read how many bytes to skip
								ed += ed[1] / 4L;
								break;
							case 0x03:
								//printf ("data type: %u\n", *ed );
								ed_ofs += ed[1]; // Read how many bytes to skip
								ed += ed[1] / 4L;
								break;
							case 0x02:
								num_records = ed[3] / 72L;
								*(unsigned *) &dp[qm_ofs] = *(unsigned *) &ed[2];
								//printf ("area: %u, mid count: %u\n", ed[2], num_records);
								if (ed[2] > 17)
								{
									printf ("Area out of range in quest!\n");
									printf ("Press [ENTER] to quit...");
									gets (&dp[0]);
									exit(1);
								}
								qm_ofs += 4;
								*(unsigned *) &dp[qm_ofs] = num_records;
								qm_ofs += 4;
								memcpy ( &dp[qm_ofs], &ed[4], ed[3] );
								qm_ofs += num_records * 72L;
								ed_ofs += ed[1]; // Read how many bytes to skip
								ed += ed[1] / 4L;
								break;
							default:
								// Probably done loading...
								ed_ofs = ed_size;
								break;
							}
						}

						// Do objects
						q->max_objects = num_objects;
						questsMemory += qb_ofs;
						q->objectdata = malloc ( qb_ofs );
						// Need to sort first...
						ch3 = 0;
						for (ch=0;ch<18;ch++)
						{
							ch2 = 0;
							while (ch2 < qb_ofs)
							{
								unsigned qa;

								qa = *(unsigned *) &qpd_buffer[ch2];
								num_records = *(unsigned *) &qpd_buffer[ch2+4];
								if (qa == ch)
								{
									memcpy (&q->objectdata[ch3], &qpd_buffer[ch2+8], ( num_records * 68 ) );
									ch3 += ( num_records * 68 );
								}
								ch2 += ( num_records * 68 ) + 8;
							}
						}

						// Do enemies

						qm_ofs += 4;
						questsMemory += qm_ofs;
						q->mapdata = malloc ( qm_ofs );
						*(unsigned *) q->mapdata = qm_ofs;
						// Need to sort first...
						ch3 = 4;
						for (ch=0;ch<18;ch++)
						{
							ch2 = 0;
							while (ch2 < ( qm_ofs - 4 ))
							{
								unsigned qa;

								qa = *(unsigned *) &dp[ch2];
								num_records = *(unsigned *) &dp[ch2+4];
								if (qa == ch)
								{
									memcpy (&q->mapdata[ch3], &dp[ch2], ( num_records * 72 ) + 8 );
									ch3 += ( num_records * 72 ) + 8;
								}
								ch2 += ( num_records * 72 ) + 8;
							}
						}
						for (ch=0;ch<num_objects;ch++)
						{
							// Swap fields in advance
							dp[0] = q->objectdata[(ch*68)+0x37];
							dp[1] = q->objectdata[(ch*68)+0x36];
							dp[2] = q->objectdata[(ch*68)+0x35];
							dp[3] = q->objectdata[(ch*68)+0x34];
							*(unsigned *) &q->objectdata[(ch*68)+0x34] = *(unsigned *) &dp[0];
						}
						printf ("Loaded quest %s (%s),\nObject count: %u, Enemy count: %u\n", qname, true_filename, num_objects, ( qm_ofs - 4 ) / 72L );
					}
					/*
					// Time to load the map data...
					l = &fakelobby;
					memset ( l, 0, sizeof (LOBBY) );
					l->bptable = &ep2battle[0];
					memset ( &l->mapData[0], 0, 0xB50 * sizeof (MAP_MONSTER) ); // Erase!
					l->mapIndex = 0;
					l->rareIndex = 0;
					for (ch=0;ch<0x20;ch++)
					l->rareData[ch] = 0xFF;

					qmap = q->mapdata;
					qm_length = *(unsigned*) qmap;
					qmap += 4;
					ch = 4;
					while ( ( qm_length - ch ) >= 80 )
					{
					oldIndex = l->mapIndex;
					qa = *(unsigned*) qmap; // Area
					qmap += 4;
					nr = *(unsigned*) qmap; // Number of monsters
					qmap += 4;
					if ( ( l->episode == 0x03 ) && ( qa > 5 ) )
					ParseMapData ( l, (MAP_MONSTER*) qmap, 1, nr );
					else
					if ( ( l->episode == 0x02 ) && ( qa > 15 ) )
					ParseMapData ( l, (MAP_MONSTER*) qmap, 1, nr );
					else
					ParseMapData ( l, (MAP_MONSTER*) qmap, 0, nr );
					qmap += ( nr * 72 );
					ch += ( ( nr * 72 ) + 8 );
					debug ("loaded quest area %u, mid count %u, total mids: %u", qa, l->mapIndex - oldIndex, l->mapIndex);
					}
					exit (1);
					*/
				}
				else
				{
					if (ch4 == 0)
					{
						printf ("Quest file %s is missing!  Could not load the quest.\n", qfile4);
						printf ("Press [ENTER] to quit...");
						gets (&dp[0]);
						exit(1);
					}
					else
					{
						printf ("Non-fatal: Alternate quest language file %s is missing.\n", qfile4);
					}
				}
			}
		}
		fclose (qf);
		qm->num_categories++;
	}
	fclose (fp);
}

unsigned csv_lines = 0;
char* csv_params[1024][64]; // 1024 lines which can carry 64 parameters each

// Release RAM from loaded CSV
void FreeCSV()
{
	unsigned ch, ch2;

	for (ch=0;ch<csv_lines;ch++)
	{
		for (ch2=0;ch2<64;ch2++)
			if (csv_params[ch][ch2] != NULL) free (csv_params[ch][ch2]);
	}
	csv_lines = 0;
	memset (&csv_params, 0, sizeof (csv_params));
}

#include "load-funcs.h"

LRESULT CALLBACK WndProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	if(message == MYWM_NOTIFYICON)
	{
		switch (lParam)
		{
		case WM_LBUTTONDBLCLK:
			switch (wParam) 
			{
			case 100:
				if (program_hidden)
				{
					program_hidden = 0;
					reveal_window;
				}
				else
				{
					program_hidden = 1;
					ShowWindow (consoleHwnd, SW_HIDE);
				}
				return TRUE;
				break;
			}
			break;
		}
	}
	return DefWindowProc( hwnd, message, wParam, lParam );
}

/********************************************************
**
**		main  :-
**
********************************************************/

int main()
{
	unsigned int ch,ch2,ch3,ch4,ch5,connectNum;
	int wep_rank;
	PTDATA ptd;
	unsigned int wep_counters[24] = {0};
	unsigned int tool_counters[28] = {0};
	unsigned int tech_counters[19] = {0};
	struct in_addr ship_in;
	struct sockaddr_in listen_in;
	unsigned int listen_length;
	int block_sockfd[10] = {-1};
	struct in_addr block_in[10];
	int ship_sockfd = -1;
	int pkt_len, pkt_c, bytes_sent;
	int wserror;
	WSADATA winsock_data;
	FILE* fp;
	unsigned char* connectionChunk;
	unsigned char* connectionPtr;
	unsigned char* blockPtr;
	unsigned char* blockChunk;
	//unsigned short this_packet;
	unsigned long logon_this_packet;
	HINSTANCE hinst;
    NOTIFYICONDATA nid = {0};
	WNDCLASS wc = {0};
	HWND hwndWindow;
	MSG msg;
	
	ch = 0;
	
	consoleHwnd = GetConsoleWindow();
	hinst = GetModuleHandle(NULL);
	
	dp[0] = 0;
	
	// Starts off with giving the server installer this nice welcome message
	// dp is a temp char buffer, used throughout this whole file
	strcat (dp, "Tethealla Ship Server version ");
	strcat (dp, SERVER_VERSION );
	strcat (dp, " coded by Sodaboy");
	
	// This looks to be a windows command (from windows.h maybe)
	SetConsoleTitle (dp);
	
	printf ("\nTethealla Ship Server version %s  Copyright (C) 2008  Terry Chatman Jr.\n", SERVER_VERSION);
	printf ("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	printf ("This program comes with ABSOLUTELY NO WARRANTY; for details\n");
	printf ("see section 15 in gpl-3.0.txt\n");
	printf ("This is free software, and you are welcome to redistribute it\n");
	printf ("under certain conditions; see gpl-3.0.txt for details.\n");
	printf ("\n\n");
	
	// Initialize connection to winsock
	WSAStartup(MAKEWORD(1,1), &winsock_data);
	
	printf ("Loading the configuration from ship.ini ... ");
#ifdef LOG_60
	// Start debugging
	debugfile = fopen ("60packets.txt", "a");
	if (!debugfile)
	{
		printf ("Could not create 60packets.txt");
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit(1);
	}
#endif
	mt_goodseed();
	load_config_file();
	printf ("OK!\n\n");
	printf ("Loading language file...\n");
	load_language_file();
	printf ("OK!\n\n");
	
	printf ("Loading ship_key.bin... ");
	fp = fopen ("ship_key.bin", "rb" );
	// Check if exists
	if (!fp)
	{
		printf ("Could not locate ship_key.bin!\n");
		printf ("Hit [ENTER] to quit...");
		gets (&dp[0]);
		exit (1);
	}
	
	fread (&ship_index, 1, 4, fp );
	fread (&ship_key[0], 1, 128, fp );
	fclose (fp);
	
	printf ("OK!\n\nLoading weapon parameter file...\n");
	LoadWeaponParam();
	printf ("\n.. done!\n\n");
	
	printf ("Loading armor & barrier parameter file...\n");
	LoadArmorParam();
	printf ("\n.. done!\n\n");
	
	printf ("Loading technique parameter file...\n");
	LoadTechParam();
	printf ("\n.. done!\n\n");
	
	for (ch=1;ch<200;ch++)
		tnlxp[ch] = tnlxp[ch-1] + tnlxp[ch];
	
	printf ("Loading battle parameter files...\n\n");
	LoadBattleParam (&ep1battle_off[0], "param\\BattleParamEntry.dat", 374, 0x8fef1ffe);
	LoadBattleParam (&ep1battle[0], "param\\BattleParamEntry_on.dat", 374, 0xb8a2d950);
	LoadBattleParam (&ep2battle_off[0], "param\\BattleParamEntry_lab.dat", 374, 0x3dc217f5);
	LoadBattleParam (&ep2battle[0], "param\\BattleParamEntry_lab_on.dat", 374, 0x4d4059cf);
	LoadBattleParam (&ep4battle_off[0], "param\\BattleParamEntry_ep4.dat", 332, 0x50841167);
	LoadBattleParam (&ep4battle[0], "param\\BattleParamEntry_ep4_on.dat", 332, 0x42bf9716);
	
	for (ch=0;ch<374;ch++)
		if (ep2battle_off[ch].HP)
		{
			ep2battle_off[ch].XP = ( ep2battle_off[ch].XP * 130 ) / 100; // 30% boost to EXP
			ep2battle[ch].XP     = ( ep2battle[ch].XP * 130 ) / 100;
		}
	
	printf ("\n.. done!\n\nBuilding common tables... \n\n");
	printf ("Weapon drop rate: %03f%%\n", (float) WEAPON_DROP_RATE / 1000);
	printf ("Armor drop rate: %03f%%\n", (float) ARMOR_DROP_RATE / 1000);
	printf ("Mag drop rate: %03f%%\n", (float) MAG_DROP_RATE / 1000);
	printf ("Tool drop rate: %03f%%\n", (float) TOOL_DROP_RATE / 1000);
	printf ("Meseta drop rate: %03f%%\n", (float) MESETA_DROP_RATE / 1000);
	printf ("Experience rate: %u%%\n\n", EXPERIENCE_RATE * 100);
	
	ch = 0;
	while (ch < 100000)
	{
		for (ch2=0;ch2<5;ch2++)
		{
			common_counters[ch2]++;
			if ((common_counters[ch2] >= common_rates[ch2]) && (ch<100000))
			{
				common_table[ch++] = (unsigned char) ch2;
				common_counters[ch2] = 0;
			}
		}
	}
	
	printf (".. done!\n\n");
	
	printf ("Loading param\\ItemPT.gsl...\n");
	fp = fopen ("param\\ItemPT.gsl", "rb");
	if (!fp)
	{
		printf ("Can't proceed without ItemPT.gsl\n");
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	fseek (fp, 0x3000, SEEK_SET);
	
	// Load up that EP1 data
	printf ("Parse Episode I data... (This may take awhile...)\n");
	for (ch2=0;ch2<4;ch2++) // For each difficulty
	{
		for (ch=0;ch<10;ch++) // For each ID
		{
			fread  (&ptd, 1, sizeof (PTDATA), fp);
			
			ptd.enemy_dar[44] = 100; // Dragon
			ptd.enemy_dar[45] = 100; // De Rol Le
			ptd.enemy_dar[46] = 100; // Vol Opt
			ptd.enemy_dar[47] = 100; // Falz
			
			for (ch3=0;ch3<10;ch3++)
			{
				ptd.box_meseta[ch3][0] = swapendian ( ptd.box_meseta[ch3][0] );
				ptd.box_meseta[ch3][1] = swapendian ( ptd.box_meseta[ch3][1] );
			}
			
			for (ch3=0;ch3<0x64;ch3++)
			{
				ptd.enemy_meseta[ch3][0] = swapendian ( ptd.enemy_meseta[ch3][0] );
				ptd.enemy_meseta[ch3][1] = swapendian ( ptd.enemy_meseta[ch3][1] );
			}
			
			ptd.enemy_meseta[47][0] = ptd.enemy_meseta[46][0] + 400 + ( 100 * ch2 ); // Give Falz some meseta
			ptd.enemy_meseta[47][1] = ptd.enemy_meseta[46][1] + 400 + ( 100 * ch2 );
			
			for (ch3=0;ch3<23;ch3++)
			{
				for (ch4=0;ch4<6;ch4++)
					ptd.percent_pattern[ch3][ch4] = swapendian ( ptd.percent_pattern[ch3][ch4] );
			}
			
			for (ch3=0;ch3<28;ch3++)
			{
				for (ch4=0;ch4<10;ch4++)
				{
					if (ch3 == 23)
						ptd.tool_frequency[ch3][ch4] = 0;
					else
						ptd.tool_frequency[ch3][ch4] = swapendian ( ptd.tool_frequency[ch3][ch4] );
				}
			}
			
			memcpy (&pt_tables_ep1[ch][ch2], &ptd, sizeof (PTDATA));
			
			// Set up the weapon drop table
			
			for (ch5=0;ch5<10;ch5++)
			{
				memset (&wep_counters[0], 0, 4 * 24 );
				ch3 = 0;
				while (ch3 < 4096)
				{
					for (ch4=0;ch4<12;ch4++)
					{
						wep_counters[ch4] += ptd.weapon_ratio[ch4];
						if ((wep_counters[ch4] >= 0xFF) && (ch3<4096))
						{
							wep_rank  = ptd.weapon_minrank[ch4];
							wep_rank += ptd.area_pattern[ch5];
							if ( wep_rank >= 0 )
							{
								weapon_drops_ep1[ch][ch2][ch5][ch3++] = ( ch4 + 1 ) + ( (unsigned char) wep_rank << 8 );
								wep_counters[ch4] = 0;
							}
						}
					}
				}
			}
			
			// Set up the slot table

			memset (&wep_counters[0], 0, 4 * 24 );
			ch3 = 0;

			while (ch3 < 4096)
			{
				for (ch4=0;ch4<5;ch4++)
				{
					wep_counters[ch4] += ptd.slot_ranking[ch4];
					if ((wep_counters[ch4] >= 0x64) && (ch3<4096))
					{
						slots_ep1[ch][ch2][ch3++] = ch4;
						wep_counters[ch4] = 0;
					}
				}
			}

			// Set up the power patterns

			for (ch5=0;ch5<4;ch5++)
			{
				memset (&wep_counters[0], 0, 4 * 24 );
				ch3 = 0;
				while (ch3 < 4096)
				{
					for (ch4=0;ch4<9;ch4++)
					{
						wep_counters[ch4] += ptd.power_pattern[ch4][ch5];
						if ((wep_counters[ch4] >= 0x64) && (ch3<4096))
						{
							power_patterns_ep1[ch][ch2][ch5][ch3++] = ch4;
							wep_counters[ch4] = 0;
						}
					}
				}
			}

			// Set up the percent patterns

			for (ch5=0;ch5<6;ch5++)
			{
				memset (&wep_counters[0], 0, 4 * 24 );
				ch3 = 0;
				while (ch3 < 4096)
				{
					for (ch4=0;ch4<23;ch4++)
					{
						wep_counters[ch4] += ptd.percent_pattern[ch4][ch5];
						if ((wep_counters[ch4] >= 0x2710) && (ch3<4096))
						{
							percent_patterns_ep1[ch][ch2][ch5][ch3++] = (char) ch4;
							wep_counters[ch4] = 0;
						}
					}
				}
			}

			// Set up the tool table

			for (ch5=0;ch5<10;ch5++)
			{
				memset (&tool_counters[0], 0, 4 * 28 );
				ch3 = 0;
				while (ch3 < 4096)
				{
					for (ch4=0;ch4<28;ch4++)
					{
						tool_counters[ch4] += ptd.tool_frequency[ch4][ch5];
						if ((tool_counters[ch4] >= 0x2710) && (ch3<4096))
						{
							tool_drops_ep1[ch][ch2][ch5][ch3++] = ch4;
							tool_counters[ch4] = 0;
						}
					}
				}
			}


			// Set up the attachment table

			for (ch5=0;ch5<10;ch5++)
			{
				memset (&tech_counters[0], 0, 4 * 19 );
				ch3 = 0;
				while (ch3 < 4096)
				{
					for (ch4=0;ch4<6;ch4++)
					{
						tech_counters[ch4] += ptd.percent_attachment[ch4][ch5];
						if ((tech_counters[ch4] >= 0x64) && (ch3<4096))
						{
							attachment_ep1[ch][ch2][ch5][ch3++] = ch4;
							tech_counters[ch4] = 0;
						}
					}
				}
			}


			// Set up the technique table

			for (ch5=0;ch5<10;ch5++)
			{
				memset (&tech_counters[0], 0, 4 * 19 );
				ch3 = 0;
				while (ch3 < 4096)
				{
					for (ch4=0;ch4<19;ch4++)
					{
						if (ptd.tech_levels[ch4][ch5*2] >= 0)
						{
							tech_counters[ch4] += ptd.tech_frequency[ch4][ch5];
							if ((tech_counters[ch4] >= 0xFF) && (ch3<4096))
							{
								tech_drops_ep1[ch][ch2][ch5][ch3++] = ch4;
								tech_counters[ch4] = 0;
							}
						}
					}
				}
			}
		}
	}

	// Load up that EP2 data
	printf ("Parse Episode II data... (This may take awhile...)\n");
	for (ch2=0;ch2<4;ch2++) // For each difficulty
	{
		for (ch=0;ch<10;ch++) // For each ID
		{
			fread (&ptd, 1, sizeof (PTDATA), fp);

			ptd.enemy_dar[73] = 100; // Barba Ray
			ptd.enemy_dar[76] = 100; // Gol Dragon
			ptd.enemy_dar[77] = 100; // Gar Gryphon
			ptd.enemy_dar[78] = 100; // Olga Flow

			for (ch3=0;ch3<10;ch3++)
			{
				ptd.box_meseta[ch3][0] = swapendian ( ptd.box_meseta[ch3][0] );
				ptd.box_meseta[ch3][1] = swapendian ( ptd.box_meseta[ch3][1] );
			}

			for (ch3=0;ch3<0x64;ch3++)
			{
				ptd.enemy_meseta[ch3][0] = swapendian ( ptd.enemy_meseta[ch3][0] );
				ptd.enemy_meseta[ch3][1] = swapendian ( ptd.enemy_meseta[ch3][1] );
			}

			ptd.enemy_meseta[78][0] = ptd.enemy_meseta[77][0] + 400 + ( 100 * ch2 ); // Give Flow some meseta
			ptd.enemy_meseta[78][1] = ptd.enemy_meseta[77][1] + 400 + ( 100 * ch2 );

			for (ch3=0;ch3<23;ch3++)
			{
				for (ch4=0;ch4<6;ch4++)
					ptd.percent_pattern[ch3][ch4] = swapendian ( ptd.percent_pattern[ch3][ch4] );
			}

			for (ch3=0;ch3<28;ch3++)
			{
				for (ch4=0;ch4<10;ch4++)
				{
					if (ch3 == 23)
						ptd.tool_frequency[ch3][ch4] = 0;
					else
						ptd.tool_frequency[ch3][ch4] = swapendian ( ptd.tool_frequency[ch3][ch4] );
				}
			}

			memcpy ( &pt_tables_ep2[ch][ch2], &ptd, sizeof (PTDATA) );

			// Set up the weapon drop table

			for (ch5=0;ch5<10;ch5++)
			{
				memset (&wep_counters[0], 0, 4 * 24 );
				ch3 = 0;
				while (ch3 < 4096)
				{
					for (ch4=0;ch4<12;ch4++)
					{
						wep_counters[ch4] += ptd.weapon_ratio[ch4];
						if ((wep_counters[ch4] >= 0xFF) && (ch3<4096))
						{
							wep_rank  = ptd.weapon_minrank[ch4];
							wep_rank += ptd.area_pattern[ch5];
							if ( wep_rank >= 0 )
							{
								weapon_drops_ep2[ch][ch2][ch5][ch3++] = ( ch4 + 1 ) + ( (unsigned char) wep_rank << 8 );
								wep_counters[ch4] = 0;
							}
						}
					}
				}
			}


			// Set up the slot table

			memset (&wep_counters[0], 0, 4 * 24 );
			ch3 = 0;

			while (ch3 < 4096)
			{
				for (ch4=0;ch4<5;ch4++)
				{
					wep_counters[ch4] += ptd.slot_ranking[ch4];
					if ((wep_counters[ch4] >= 0x64) && (ch3<4096))
					{
						slots_ep2[ch][ch2][ch3++] = ch4;
						wep_counters[ch4] = 0;
					}
				}
			}

			// Set up the power patterns

			for (ch5=0;ch5<4;ch5++)
			{
				memset (&wep_counters[0], 0, 4 * 24 );
				ch3 = 0;
				while (ch3 < 4096)
				{
					for (ch4=0;ch4<9;ch4++)
					{
						wep_counters[ch4] += ptd.power_pattern[ch4][ch5];
						if ((wep_counters[ch4] >= 0x64) && (ch3<4096))
						{
							power_patterns_ep2[ch][ch2][ch5][ch3++] = ch4;
							wep_counters[ch4] = 0;
						}
					}
				}
			}

			// Set up the percent patterns

			for (ch5=0;ch5<6;ch5++)
			{
				memset (&wep_counters[0], 0, 4 * 24 );
				ch3 = 0;
				while (ch3 < 4096)
				{
					for (ch4=0;ch4<23;ch4++)
					{
						wep_counters[ch4] += ptd.percent_pattern[ch4][ch5];
						if ((wep_counters[ch4] >= 0x2710) && (ch3<4096))
						{
							percent_patterns_ep2[ch][ch2][ch5][ch3++] = (char) ch4;
							wep_counters[ch4] = 0;
						}
					}
				}
			}

			// Set up the tool table

			for (ch5=0;ch5<10;ch5++)
			{
				memset (&tool_counters[0], 0, 4 * 28 );
				ch3 = 0;
				while (ch3 < 4096)
				{
					for (ch4=0;ch4<28;ch4++)
					{
						tool_counters[ch4] += ptd.tool_frequency[ch4][ch5];
						if ((tool_counters[ch4] >= 0x2710) && (ch3<4096))
						{
							tool_drops_ep2[ch][ch2][ch5][ch3++] = ch4;
							tool_counters[ch4] = 0;
						}
					}
				}
			}


			// Set up the attachment table

			for (ch5=0;ch5<10;ch5++)
			{
				memset (&tech_counters[0], 0, 4 * 19 );
				ch3 = 0;
				while (ch3 < 4096)
				{
					for (ch4=0;ch4<6;ch4++)
					{
						tech_counters[ch4] += ptd.percent_attachment[ch4][ch5];
						if ((tech_counters[ch4] >= 0x64) && (ch3<4096))
						{
							attachment_ep2[ch][ch2][ch5][ch3++] = ch4;
							tech_counters[ch4] = 0;
						}
					}
				}
			}


			// Set up the technique table

			for (ch5=0;ch5<10;ch5++)
			{
				memset (&tech_counters[0], 0, 4 * 19 );
				ch3 = 0;
				while (ch3 < 4096)
				{
					for (ch4=0;ch4<19;ch4++)
					{
						if (ptd.tech_levels[ch4][ch5*2] >= 0)
						{
							tech_counters[ch4] += ptd.tech_frequency[ch4][ch5];
							if ((tech_counters[ch4] >= 0xFF) && (ch3<4096))
							{
								tech_drops_ep2[ch][ch2][ch5][ch3++] = ch4;
								tech_counters[ch4] = 0;
							}
						}
					}
				}
			}
		}
	}


	fclose (fp);
	printf ("\n.. done!\n\n");
	printf ("Loading param\\PlyLevelTbl.bin ... ");
	fp = fopen ( "param\\PlyLevelTbl.bin", "rb" );
	if (!fp)
	{
		printf ("Can't proceed without PlyLevelTbl.bin!\n");
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	fread ( &startingData, 1, 12*14, fp );
	fseek ( fp, 0xE4, SEEK_SET );
	fread ( &playerLevelData, 1, 28800, fp );
	fclose ( fp );
	
	printf ("OK!\n\n.. done!\n\nLoading quests...\n\n");
	
	memset (&quest_menus[0], 0, sizeof (quest_menus));
	
	LoadQuests ("quest\\ep1team.ini", 0);	// 0 = Episode 1 Team
	LoadQuests ("quest\\ep2team.ini", 1);	// 1 = Episode 2 Team
	LoadQuests ("quest\\ep4team.ini", 2);	// 2 = Episode 4 Team
	LoadQuests ("quest\\ep1solo.ini", 3);	// 3 = Episode 1 Solo
	LoadQuests ("quest\\ep2solo.ini", 4);	// 4 = Episode 2 Solo
	LoadQuests ("quest\\ep4solo.ini", 5);	// 5 = Episode 4 Solo
	LoadQuests ("quest\\ep1gov.ini", 6);	// 6 = Episode 1 Government
	LoadQuests ("quest\\ep2gov.ini", 7);	// 7 = Episode 2 Government
	LoadQuests ("quest\\ep4gov.ini", 8);	// 8 = Episode 4 Government
	LoadQuests ("quest\\battle.ini", 9);	// 9 = Battle
	// This is where challenge would go		// 10 = Challenge

	printf ("\n%u bytes of memory allocated for %u quests...\n\n", questsMemory, numQuests);

	printf ("Loading shop\\shop.dat ...");
	fp = fopen ( "shop\\shop.dat", "rb" );
	// Check if file exists
	if (!fp)
	{
		printf ("Can't proceed without shop.dat!\n");
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	if ( fread ( &shops[0], 1, 7000 * sizeof (SHOP), fp )  != (7000 * sizeof (SHOP)) )
	{
		printf ("Failed to read shop data...\n");
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	fclose ( fp );
	shop_checksum = CalculateChecksum ( &shops[0], 7000 * sizeof (SHOP) );
	printf ("done!\n\n");

	LoadShopData2();
	readLocalGMFile();

	// Set up shop indexes based on character levels
	for (ch=0;ch<200;ch++)
	{
		switch (ch / 20L)
		{
		case 0:	// Levels 1-20
			shopidx[ch] = 0;
			break;
		case 1: // Levels 21-40
			shopidx[ch] = 1000;
			break;
		case 2: // Levels 41-80
		case 3:
			shopidx[ch] = 2000;
			break;
		case 4: // Levels 81-120
		case 5:
			shopidx[ch] = 3000;
			break;
		case 6: // Levels 121-160
		case 7:
			shopidx[ch] = 4000;
			break;
		case 8: // Levels 161-180
			shopidx[ch] = 5000;
			break;
		default: // Levels 180+
			shopidx[ch] = 6000;
			break;
		}
	}

	memcpy (&Packet03[0x54], &Message03[0], sizeof (Message03));
	printf ("\nShip server parameters\n");
	printf ("///////////////////////\n");
	printf ("IP: %u.%u.%u.%u\n", serverIP[0], serverIP[1], serverIP[2], serverIP[3] );
	printf ("Ship Port: %u\n", serverPort );
	printf ("Number of Blocks: %u\n", serverBlocks );
	printf ("Maximum Connections: %u\n", serverMaxConnections );
	printf ("Logon server IP: %u.%u.%u.%u\n", loginIP[0], loginIP[1], loginIP[2], loginIP[3] );

	printf ("\nConnecting to the logon server...\n");
	initialize_logon();
	reconnect_logon();

	printf ("\nAllocating %u bytes of memory for blocks... ", sizeof (BLOCK) * serverBlocks );
	blockChunk = malloc ( sizeof (BLOCK) * serverBlocks );
	if (!blockChunk)
	{
		printf ("Out of memory!\n");
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	blockPtr = blockChunk;
	memset (blockChunk, 0, sizeof (BLOCK) * serverBlocks);
	for (ch=0;ch<serverBlocks;ch++)
	{
		blocks[ch] = (BLOCK*) blockPtr;
		blockPtr += sizeof (BLOCK);
	}

	printf ("OK!\n");

	printf ("\nAllocating %u bytes of memory for connections... ", sizeof (CLIENT) * serverMaxConnections );
	connectionChunk = malloc ( sizeof (CLIENT) * serverMaxConnections );
	if (!connectionChunk )
	{
		printf ("Out of memory!\n");
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	connectionPtr = connectionChunk;
	for (ch=0;ch<serverMaxConnections;ch++)
	{
		connections[ch] = (CLIENT*) connectionPtr;
		connections[ch]->guildcard = 0;
		connections[ch]->character_backup = NULL;
		connections[ch]->mode = 0;
		initialize_connection (connections[ch]);
		connectionPtr += sizeof (CLIENT);
	}

	printf ("OK!\n\n");

	printf ("Loading ban data... ");
	fp = fopen ("bandata.dat", "rb");
	if (fp)
	{
		fseek ( fp, 0, SEEK_END );
		ch = ftell ( fp );
		num_bans = ch / sizeof (BANDATA);
		if ( num_bans > 5000 )
			num_bans = 5000;
		fseek ( fp, 0, SEEK_SET );
		fread ( &ship_bandata[0], 1, num_bans * sizeof (BANDATA), fp );
		fclose ( fp );
	}
	printf ("done!\n\n%u bans loaded.\n%u IP mask bans loaded.\n\n",num_bans,num_masks);

	/* Open the ship port... */

	printf ("Opening ship port %u for connections.\n", serverPort);

#ifdef USEADDR_ANY
	ship_in.s_addr = INADDR_ANY;
#else
	memcpy (&ship_in.s_addr, &serverIP[0], 4 );
#endif
	ship_sockfd = tcp_sock_open( ship_in, serverPort );
	
	tcp_listen (ship_sockfd);
	
	for (ch=1;ch<=serverBlocks;ch++)
	{
		printf ("Opening block port %u (BLOCK%u) for connections.\n", serverPort+ch, ch);
#ifdef USEADDR_ANY
		block_in[ch-1].s_addr = INADDR_ANY;
#else
	memcpy (&block_in[ch-1].s_addr, &serverIP[0], 4 );
#endif
		block_sockfd[ch-1] = tcp_sock_open( block_in[ch-1], serverPort+ch );
		if (block_sockfd[ch-1] < 0)
		{
			printf ("Failed to open port %u for connections.\n", serverPort+ch );
			printf ("Press [ENTER] to quit...");
			gets(&dp[0]);
			exit (1);
		}
		
		tcp_listen (block_sockfd[ch-1]);
		
	}
	
	if (ship_sockfd < 0)
	{
		printf ("Failed to open ship port for connections.\n");
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	
	printf ("\nListening...\n");
	
	/* wc.hbrBackground =(HBRUSH)GetStockObject(WHITE_BRUSH);
	wc.hIcon = LoadIcon( hinst, IDI_APPLICATION );
	wc.hCursor = LoadCursor( hinst, IDC_ARROW );
	wc.hInstance = hinst;
	wc.lpfnWndProc = WndProc;
	wc.lpszClassName = "sodaboy";
	wc.style = CS_HREDRAW | CS_VREDRAW;

	if (! RegisterClass( &wc ) )
	{
		printf ("RegisterClass failure.\n");
		exit (1);
	}

	hwndWindow = CreateWindow ("sodaboy","hidden window", WS_MINIMIZE, 1, 1, 1, 1, 
		NULL, 
		NULL,
		hinst,
		NULL );

	if (!hwndWindow)
	{
		printf ("Failed to create window.");
		exit (1);
	} */

	/* ShowWindow ( hwndWindow, SW_HIDE );
	UpdateWindow ( hwndWindow );
	ShowWindow ( consoleHwnd, SW_HIDE );
	UpdateWindow ( consoleHwnd );

    nid.cbSize				= sizeof(nid);
	nid.hWnd				= hwndWindow;
	nid.uID					= 100;
	nid.uCallbackMessage	= MYWM_NOTIFYICON;
	nid.uFlags				= NIF_MESSAGE|NIF_ICON|NIF_TIP;
    nid.hIcon				= LoadIcon(hinst, MAKEINTRESOURCE(IDI_ICON1));
	nid.szTip[0] = 0;
	strcat (&nid.szTip[0], "Tethealla Ship ");
	strcat (&nid.szTip[0], SERVER_VERSION);
	strcat (&nid.szTip[0], " - Double click to show/hide");
    Shell_NotifyIcon(NIM_ADD, &nid); */

	// MAIN LOOP
	for (;;)
	{
		int nfds = 0;
		
		// Process the system tray icon 
		/* if ( PeekMessage( &msg, hwndWindow, 0, 0, 1 ) )
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} */
		
		// Refresh server time
		servertime = time(NULL);
		
		// Clear socket activity flags.
		FD_ZERO (&ReadFDs);
		FD_ZERO (&WriteFDs);
		FD_ZERO (&ExceptFDs);
		
		// Stop blocking connections after everyone has been disconnected...
		if ((serverNumConnections == 0) && (blockConnections))
		{
			blockConnections = 0;
			printf ("No longer blocking new connections...\n");
		}
		
		// Process player packets
		for (ch=0;ch<serverNumConnections;ch++)
		{
			connectNum = serverConnectionList[ch];
			workConnect = connections[connectNum];
			
			if (workConnect->plySockfd >= 0) 
			{
				if (blockConnections)
				{
					if (blockTick != (unsigned) servertime)
					{
						blockTick = (unsigned) servertime;
						printf ("Disconnected user %u, left to disconnect: %u\n", workConnect->guildcard, serverNumConnections - 1);
						Send1A ("You were disconnected by a GM...", workConnect);
						workConnect->todc = 1;
					}
				}

				if (workConnect->lastTick != (unsigned) servertime)
				{
					Send1D (workConnect);
					if (workConnect->lastTick > (unsigned) servertime)
						ch2 = 1;
					else
						ch2 = 1 + ((unsigned) servertime - workConnect->lastTick);
						workConnect->lastTick = (unsigned) servertime;
						workConnect->packetsSec /= ch2;
						workConnect->toBytesSec /= ch2;
						workConnect->fromBytesSec /= ch2;
				}

				FD_SET (workConnect->plySockfd, &ReadFDs);
				nfds = max (nfds, workConnect->plySockfd);
				FD_SET (workConnect->plySockfd, &ExceptFDs);
				nfds = max (nfds, workConnect->plySockfd);

				if (workConnect->snddata - workConnect->sndwritten)
				{
					FD_SET (workConnect->plySockfd, &WriteFDs);
					nfds = max (nfds, workConnect->plySockfd);
				}
			}
		}

		// Read from logon server (if connected)	
		if (logon.sockfd >= 0)
		{
			if ((unsigned) servertime - logon.last_ping > 60)
			{
				printf ("Logon server ping timeout.  Attempting reconnection in %u seconds...\n", LOGIN_RECONNECT_SECONDS);
				initialize_logon ();
			}
			else
			{
				// If there is packet data
				// (not sure what function loads data from the client into the logon struct)
				if (logon.packetdata)
				{
					// I think this checks if there are any unread packets
					// No, i think it loads the packet needed to be read
					// NO - its loading the size of the packet
					logon_this_packet = *(unsigned *) &logon.packet[logon.packetread];
					// Here is where it actually loads the packet (copies into a buffer)
					memcpy (&logon.decryptbuf[0], &logon.packet[logon.packetread], logon_this_packet);
					
					LogonProcessPacket ( &logon );
					
					logon.packetread += logon_this_packet;
					
					if (logon.packetread == logon.packetdata)
						logon.packetread = logon.packetdata = 0;
				}
				
				FD_SET (logon.sockfd, &ReadFDs);
				nfds = max (nfds, logon.sockfd);
				
				if (logon.snddata - logon.sndwritten)
				{
					FD_SET (logon.sockfd, &WriteFDs);
					nfds = max (nfds, logon.sockfd);
				}
			}
		}
		else
		{
			logon_tick++;
			if (logon_tick >= LOGIN_RECONNECT_SECONDS * 100)
			{
				printf ("Reconnecting to login server...\n");
				reconnect_logon();
			}
		}
		
		
		// Listen for block connections
		for (ch=0;ch<serverBlocks;ch++)
		{
			FD_SET (block_sockfd[ch], &ReadFDs);
			nfds = max (nfds, block_sockfd[ch]);
		}

		// Listen for ship connections
		FD_SET (ship_sockfd, &ReadFDs);
		nfds = max (nfds, ship_sockfd);
		
		// Check sockets for activity.
		if ( select ( nfds + 1, &ReadFDs, &WriteFDs, &ExceptFDs, &select_timeout ) > 0 ) 
		{
			if (FD_ISSET (ship_sockfd, &ReadFDs))
			{
				// Someone's attempting to connect to the ship server.
				ch = free_connection();
				if (ch != 0xFFFF)
				{
					listen_length = sizeof (listen_in);
					workConnect = connections[ch];
					if ( ( workConnect->plySockfd = tcp_accept ( ship_sockfd, (struct sockaddr*) &listen_in, &listen_length ) ) > 0 )
					{
						if ( !blockConnections )
						{
							workConnect->connection_index = ch;
							serverConnectionList[serverNumConnections++] = ch;
							memcpy ( &workConnect->IP_Address[0], inet_ntoa (listen_in.sin_addr), 16 );
							*(unsigned *) &workConnect->ipaddr = *(unsigned *) &listen_in.sin_addr;
							printf ("Accepted SHIP connection from %s:%u\n", workConnect->IP_Address, listen_in.sin_port );
							printf ("Player Count: %u\n", serverNumConnections);
							ShipSend0E (&logon);
							start_encryption (workConnect);
							/* Doin' ship process... */
							workConnect->block = 0;
						}
						else
							initialize_connection ( workConnect );
					}
				}
			}
			
			for (ch=0;ch<serverBlocks;ch++)
			{
				if (FD_ISSET (block_sockfd[ch], &ReadFDs))
				{
					// Someone's attempting to connect to the block server.
					ch2 = free_connection();
					if (ch2 != 0xFFFF)
					{
						listen_length = sizeof (listen_in);
						workConnect = connections[ch2];
						if  ( ( workConnect->plySockfd = tcp_accept ( block_sockfd[ch], (struct sockaddr*) &listen_in, &listen_length ) ) > 0 )
						{
							if ( !blockConnections )
							{
								workConnect->connection_index = ch2;
								serverConnectionList[serverNumConnections++] = ch2;
								memcpy ( &workConnect->IP_Address[0], inet_ntoa (listen_in.sin_addr), 16 );
								printf ("Accepted BLOCK connection from %s:%u\n", inet_ntoa (listen_in.sin_addr), listen_in.sin_port );
								*(unsigned *) &workConnect->ipaddr = *(unsigned *) &listen_in.sin_addr;
								printf ("Player Count: %u\n", serverNumConnections);
								ShipSend0E (&logon);
								start_encryption (workConnect);
								/* Doin' block process... */
								workConnect->block = ch+1;
							}
							else
								initialize_connection ( workConnect );
						}
					}
				}
			}
			
			
			// Process client connections
			
			for (ch=0;ch<serverNumConnections;ch++)
			{
				connectNum = serverConnectionList[ch];
				workConnect = connections[connectNum];
				
				if (workConnect->plySockfd >= 0)
				{
					if (FD_ISSET(workConnect->plySockfd, &WriteFDs))
					{
						// Write shit.

						bytes_sent = send (workConnect->plySockfd, &workConnect->sndbuf[workConnect->sndwritten],
							workConnect->snddata - workConnect->sndwritten, 0);

						if (bytes_sent == SOCKET_ERROR)
						{
							wserror = WSAGetLastError();
							printf ("Could not send data to client...\n");
							printf ("Socket Error %u.\n", wserror );
							
							initialize_connection (workConnect);							
						}
						else
						{
							workConnect->toBytesSec += bytes_sent;
							workConnect->sndwritten += bytes_sent;
						}

						if (workConnect->sndwritten == workConnect->snddata)
							workConnect->sndwritten = workConnect->snddata = 0;
					}

					// Disconnect those violators of the law...

					if (workConnect->todc)
						initialize_connection (workConnect);

					if (FD_ISSET(workConnect->plySockfd, &ReadFDs))
					{
						// Read shit.
						if ( ( pkt_len = recv (workConnect->plySockfd, &tmprcv[0], TCP_BUFFER_SIZE - 1, 0) ) <= 0 )
						{
							// wserror = WSAGetLastError();
							// printf ("Could not read data from client...\n");
							// printf ("Socket Error %u.\n", wserror );
							
							initialize_connection (workConnect);
						}
						else
						{
							workConnect->fromBytesSec += (unsigned) pkt_len;
							// Work with it.

							for (pkt_c=0;pkt_c<pkt_len;pkt_c++)
							{
								workConnect->rcvbuf[workConnect->rcvread++] = tmprcv[pkt_c];
								
								if (workConnect->rcvread == 8)  // Decrypt the packet header after receiving 8 bytes.
								{
									cipher_ptr = &workConnect->client_cipher;
									decryptcopy ( &workConnect->decryptbuf[0], &workConnect->rcvbuf[0], 8 );

									// Make sure we're expecting a multiple of 8 bytes.

									workConnect->expect = *(unsigned short*) &workConnect->decryptbuf[0];

									if ( workConnect->expect % 8 )
										workConnect->expect += ( 8 - ( workConnect->expect % 8 ) );

									if ( workConnect->expect > TCP_BUFFER_SIZE )
									{
										initialize_connection ( workConnect );
										break;
									}
								}

								if ( ( workConnect->rcvread == workConnect->expect ) && ( workConnect->expect != 0 ) )
								{
									// Decrypt the rest of the data if needed.

									cipher_ptr = &workConnect->client_cipher;

									if ( workConnect->rcvread > 8 )
										decryptcopy ( &workConnect->decryptbuf[8], &workConnect->rcvbuf[8], workConnect->expect - 8 );

									workConnect->packetsSec ++;

									if (  // Check for DDOS
										//(workConnect->packetsSec   > 89)    ||
										(workConnect->fromBytesSec > 30000) ||
										(workConnect->toBytesSec   > 150000)
										)
									{
										printf ("%u disconnected for possible DDOS. (p/s: %u, tb/s: %u, fb/s: %u)\n", workConnect->guildcard, workConnect->packetsSec, workConnect->toBytesSec, workConnect->fromBytesSec);
										initialize_connection(workConnect);
										break;
									}
									else
									{
										switch (workConnect->block)
										{
										case 0x00:
											// Ship Server
											ShipProcessPacket (workConnect);
											break;
										default:
											// Block server
											BlockProcessPacket (workConnect);
											break;
										}
									}
									workConnect->rcvread = 0;
								}
							}
						}
					}

					if (FD_ISSET(workConnect->plySockfd, &ExceptFDs)) // Exception?
						initialize_connection (workConnect);

				}
			}


			// Process logon server connection

			if ( logon.sockfd >= 0 )
			{
				if (FD_ISSET(logon.sockfd, &WriteFDs))
				{
					// Write shit.

					bytes_sent = send (logon.sockfd, &logon.sndbuf[logon.sndwritten],
						logon.snddata - logon.sndwritten, 0);

					if (bytes_sent == SOCKET_ERROR)
					{
						wserror = WSAGetLastError();
						printf ("Could not send data to logon server...\n");
						printf ("Socket Error %u.\n", wserror );
						initialize_logon();
						printf ("Lost connection with the logon server...\n");
						printf ("Reconnect in %u seconds...\n", LOGIN_RECONNECT_SECONDS);
					}
					else
						logon.sndwritten += bytes_sent;

					if (logon.sndwritten == logon.snddata)
						logon.sndwritten = logon.snddata = 0;
				}

				if (FD_ISSET(logon.sockfd, &ReadFDs))
				{
					// Read shit.
					if ( ( pkt_len = recv (logon.sockfd, &tmprcv[0], PACKET_BUFFER_SIZE - 1, 0) ) <= 0 )
					{
						wserror = WSAGetLastError();
						printf ("Could not read data from logon server...\n");
						printf ("Socket Error %u.\n", wserror );
						initialize_logon();
						printf ("Lost connection with the logon server...\n");
						printf ("Reconnect in %u seconds...\n", LOGIN_RECONNECT_SECONDS);
					}
					else
					{
						// Work with it.
						for (pkt_c=0;pkt_c<pkt_len;pkt_c++)
						{
							logon.rcvbuf[logon.rcvread++] = tmprcv[pkt_c];

							if (logon.rcvread == 4)
							{
								/* Read out how much data we're expecting this packet. */
								logon.expect = *(unsigned *) &logon.rcvbuf[0];

								if ( logon.expect > TCP_BUFFER_SIZE  )
								{
									printf ("Received too much data from the logon server.\nSevering connection and will reconnect in %u seconds...\n",  LOGIN_RECONNECT_SECONDS);
									initialize_logon();
								}
							}

							if ( ( logon.rcvread == logon.expect ) && ( logon.expect != 0 ) )
							{
								decompressShipPacket ( &logon, &logon.decryptbuf[0], &logon.rcvbuf[0] );

								logon.expect = *(unsigned *) &logon.decryptbuf[0];

								if (logon.packetdata + logon.expect < PACKET_BUFFER_SIZE)
								{
									memcpy ( &logon.packet[logon.packetdata], &logon.decryptbuf[0], logon.expect );
									logon.packetdata += logon.expect;
								}
								else
									initialize_logon();

								if ( logon.sockfd < 0 )
									break;

								logon.rcvread = 0;
							}
						}
					}
				}
			}
		}
	}
	
	return 0;
}

void tcp_listen (int sockfd)
{
	if (listen(sockfd, 10) < 0)
	{
		debug_perror ("Could not listen for connection");
		debug_perror ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit(1);
	}
}

int tcp_accept (int sockfd, struct sockaddr *client_addr, int *addr_len )
{
	int fd;

	if ((fd = accept (sockfd, client_addr, addr_len)) < 0)
		debug_perror ("Could not accept connection");

	return (fd);
}

int tcp_sock_connect(struct sockaddr_in sock)
{
	int fd;
	//struct sockaddr_in sa;

	/* Clear it out */
	//memset((void *)&sa, 0, sizeof(sa));

	fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	/* Error */
	if( fd < 0 )
		debug_perror("Could not create socket");
	else
	{

		//memset (&sa, 0, sizeof(sa));  // Why is this done twice?
		//sa.sin_family = AF_INET;
		//sa.sin_addr.s_addr = inet_addr (dest_addr);
		//sa.sin_port = htons((unsigned short) port);

		if (connect(fd, (struct sockaddr*) &sock, sizeof(sock)) < 0)
		{
			debug_perror("Could not make TCP connection");
			return -1;
		}
	}
	return(fd);
}

/*****************************************************************************/
int tcp_sock_open(struct in_addr ip, int port)
{
	int fd, turn_on_option_flag = 1, rcSockopt;

	struct sockaddr_in sa;

	/* Clear it out */
	memset((void *)&sa, 0, sizeof(sa));

	fd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	/* Error */
	if( fd < 0 ){
		debug_perror ("Could not create socket");
		debug_perror ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit(1);
	} 

	sa.sin_family = AF_INET;
	memcpy((void *)&sa.sin_addr, (void *)&ip, sizeof(struct in_addr));
	sa.sin_port = htons((unsigned short) port);

	/* Reuse port */

	rcSockopt = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, (char *) &turn_on_option_flag, sizeof(turn_on_option_flag));

	/* bind() the socket to the interface */
	if (bind(fd, (struct sockaddr *)&sa, sizeof(struct sockaddr)) < 0){
		debug_perror("Could not bind to port");
		debug_perror("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit(1);
	}

	return(fd);
}

/*****************************************************************************
* same as debug_perror but writes to debug output.
* 
*****************************************************************************/
void debug_perror( char * msg ) {
	debug( "%s : %s\n" , msg , strerror(errno) );
}
/*****************************************************************************/
void debug(char *fmt, ...)
{
#define MAX_MESG_LEN 1024

	va_list args;
	char text[ MAX_MESG_LEN ];

	va_start (args, fmt);
	strcpy (text + vsprintf( text,fmt,args), "\r\n"); 
	va_end (args);

	fprintf( stderr, "%s", text);
}

/* Blue Burst encryption routines */

static void pso_crypt_init_key_bb(unsigned char *data)
{
	unsigned x;
	for (x = 0; x < 48; x += 3)
	{
		data[x] ^= 0x19;
		data[x + 1] ^= 0x16;
		data[x + 2] ^= 0x18;
	}
}


void pso_crypt_decrypt_bb(PSO_CRYPT *pcry, unsigned char *data, unsigned int length)
{
	unsigned eax, ecx, edx, ebx, ebp, esi, edi;

	edx = 0;
	ecx = 0;
	eax = 0;
	while (edx < length)
	{
		ebx = *(unsigned long *) &data[edx];
		ebx = ebx ^ pcry->tbl[5];
		ebp = ((pcry->tbl[(ebx >> 0x18) + 0x12]+pcry->tbl[((ebx >> 0x10)& 0xff) + 0x112])
			^ pcry->tbl[((ebx >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebx & 0xff) + 0x312];
		ebp = ebp ^ pcry->tbl[4];
		ebp ^= *(unsigned long *) &data[edx+4];
		edi = ((pcry->tbl[(ebp >> 0x18) + 0x12]+pcry->tbl[((ebp >> 0x10)& 0xff) + 0x112])
			^ pcry->tbl[((ebp >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebp & 0xff) + 0x312];
		edi = edi ^ pcry->tbl[3];
		ebx = ebx ^ edi;
		esi = ((pcry->tbl[(ebx >> 0x18) + 0x12]+pcry->tbl[((ebx >> 0x10)& 0xff) + 0x112])
			^ pcry->tbl[((ebx >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebx & 0xff) + 0x312];
		ebp = ebp ^ esi ^ pcry->tbl[2];
		edi = ((pcry->tbl[(ebp >> 0x18) + 0x12]+pcry->tbl[((ebp >> 0x10)& 0xff) + 0x112])
			^ pcry->tbl[((ebp >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebp & 0xff) + 0x312];
		edi = edi ^ pcry->tbl[1];
		ebp = ebp ^ pcry->tbl[0];
		ebx = ebx ^ edi;
		*(unsigned long *) &data[edx] = ebp;
		*(unsigned long *) &data[edx+4] = ebx;
		edx = edx+8;
	}
}


void pso_crypt_encrypt_bb(PSO_CRYPT *pcry, unsigned char *data, unsigned int length)
{
	unsigned eax, ecx, edx, ebx, ebp, esi, edi;

	edx = 0;
	ecx = 0;
	eax = 0;
	while (edx < length)
	{
		ebx = *(unsigned long *) &data[edx];
		ebx = ebx ^ pcry->tbl[0];
		ebp = ((pcry->tbl[(ebx >> 0x18) + 0x12]+pcry->tbl[((ebx >> 0x10)& 0xff) + 0x112])
			^ pcry->tbl[((ebx >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebx & 0xff) + 0x312];
		ebp = ebp ^ pcry->tbl[1];
		ebp ^= *(unsigned long *) &data[edx+4];
		edi = ((pcry->tbl[(ebp >> 0x18) + 0x12]+pcry->tbl[((ebp >> 0x10)& 0xff) + 0x112])
			^ pcry->tbl[((ebp >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebp & 0xff) + 0x312];
		edi = edi ^ pcry->tbl[2];
		ebx = ebx ^ edi;
		esi = ((pcry->tbl[(ebx >> 0x18) + 0x12]+pcry->tbl[((ebx >> 0x10)& 0xff) + 0x112])
			^ pcry->tbl[((ebx >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebx & 0xff) + 0x312];
		ebp = ebp ^ esi ^ pcry->tbl[3];
		edi = ((pcry->tbl[(ebp >> 0x18) + 0x12]+pcry->tbl[((ebp >> 0x10)& 0xff) + 0x112])
			^ pcry->tbl[((ebp >> 0x8)& 0xff) + 0x212]) + pcry->tbl[(ebp & 0xff) + 0x312];
		edi = edi ^ pcry->tbl[4];
		ebp = ebp ^ pcry->tbl[5];
		ebx = ebx ^ edi;
		*(unsigned long *) &data[edx] = ebp;
		*(unsigned long *) &data[edx+4] = ebx;
		edx = edx+8;
	}
}

void encryptcopy (CLIENT* client, const unsigned char* src, unsigned size)
{
	unsigned char* dest;

	// Bad pointer check...
	if ( ((unsigned) client < (unsigned)connections[0]) || 
		 ((unsigned) client > (unsigned)connections[serverMaxConnections-1]) )
		return;
	
	// This is to avoid a TCP buffer overflow i think
	if (TCP_BUFFER_SIZE - client->snddata < ( (int) size + 7 ) )
		client->todc = 1;
	else
	{
		dest = &client->sndbuf[client->snddata];
		memcpy (dest,src,size);
		while (size % 8)
			dest[size++] = 0x00;
		client->snddata += (int) size;
		pso_crypt_encrypt_bb(cipher_ptr,dest,size);
	}
}


void decryptcopy (unsigned char* dest, const unsigned char* src, unsigned size)
{
	memcpy (dest,src,size);
	pso_crypt_decrypt_bb(cipher_ptr,dest,size);
}


void pso_crypt_table_init_bb(PSO_CRYPT *pcry, const unsigned char *salt)
{
	unsigned long eax, ecx, edx, ebx, ebp, esi, edi, ou, x;
	unsigned char s[48];
	unsigned short* pcryp;
	unsigned short* bbtbl;
	unsigned short dx;

	pcry->cur = 0;
	pcry->mangle = NULL;
	pcry->size = 1024 + 18;

	memcpy(s, salt, sizeof(s));
	pso_crypt_init_key_bb(s);

	bbtbl = (unsigned short*) &bbtable[0];
	pcryp = (unsigned short*) &pcry->tbl[0];

	eax = 0;
	ebx = 0;

	for(ecx=0;ecx<0x12;ecx++)
	{
		dx = bbtbl[eax++];
		dx = ( ( dx & 0xFF ) << 8 ) + ( dx >> 8 );
		pcryp[ebx] = dx;
		dx = bbtbl[eax++];
		dx ^= pcryp[ebx++];
		pcryp[ebx++] = dx;
	}

/*
	pcry->tbl[0] = 0x243F6A88;
	pcry->tbl[1] = 0x85A308D3;
	pcry->tbl[2] = 0x13198A2E;
	pcry->tbl[3] = 0x03707344;
	pcry->tbl[4] = 0xA4093822;
	pcry->tbl[5] = 0x299F31D0;
	pcry->tbl[6] = 0x082EFA98;
	pcry->tbl[7] = 0xEC4E6C89;
	pcry->tbl[8] = 0x452821E6;
	pcry->tbl[9] = 0x38D01377;
	pcry->tbl[10] = 0xBE5466CF;
	pcry->tbl[11] = 0x34E90C6C;
	pcry->tbl[12] = 0xC0AC29B7;
	pcry->tbl[13] = 0xC97C50DD;
	pcry->tbl[14] = 0x3F84D5B5;
	pcry->tbl[15] = 0xB5470917;
	pcry->tbl[16] = 0x9216D5D9;
	pcry->tbl[17] = 0x8979FB1B;
	
	*/

	memcpy(&pcry->tbl[18], &bbtable[18], 4096);

	ecx=0;
	//total key[0] length is min 0x412
	ebx=0;

	while (ebx < 0x12)
	{
		//in a loop
		ebp=((unsigned long) (s[ecx])) << 0x18;
		eax=ecx+1;
		edx=eax-((eax / 48)*48);
		eax=(((unsigned long) (s[edx])) << 0x10) & 0xFF0000;
		ebp=(ebp | eax) & 0xffff00ff;
		eax=ecx+2;
		edx=eax-((eax / 48)*48);
		eax=(((unsigned long) (s[edx])) << 0x8) & 0xFF00;
		ebp=(ebp | eax) & 0xffffff00;
		eax=ecx+3;
		ecx=ecx+4;
		edx=eax-((eax / 48)*48);
		eax=(unsigned long) (s[edx]);
		ebp=ebp | eax;
		eax=ecx;
		edx=eax-((eax / 48)*48);
		pcry->tbl[ebx]=pcry->tbl[ebx] ^ ebp;
		ecx=edx;
		ebx++;
	}

	ebp=0;
	esi=0;
	ecx=0;
	edi=0;
	ebx=0;
	edx=0x48;

	while (edi < edx)
	{
		esi=esi ^ pcry->tbl[0];
		eax=esi >> 0x18;
		ebx=(esi >> 0x10) & 0xff;
		eax=pcry->tbl[eax+0x12]+pcry->tbl[ebx+0x112];
		ebx=(esi >> 8) & 0xFF;
		eax=eax ^ pcry->tbl[ebx+0x212];
		ebx=esi & 0xff;
		eax=eax + pcry->tbl[ebx+0x312];

		eax=eax ^ pcry->tbl[1];
		ecx= ecx ^ eax;
		ebx=ecx >> 0x18;
		eax=(ecx >> 0x10) & 0xFF;
		ebx=pcry->tbl[ebx+0x12]+pcry->tbl[eax+0x112];
		eax=(ecx >> 8) & 0xff;
		ebx=ebx ^ pcry->tbl[eax+0x212];
		eax=ecx & 0xff;
		ebx=ebx + pcry->tbl[eax+0x312];

		for (x = 0; x <= 5; x++)
		{
			ebx=ebx ^ pcry->tbl[(x*2)+2];
			esi= esi ^ ebx;
			ebx=esi >> 0x18;
			eax=(esi >> 0x10) & 0xFF;
			ebx=pcry->tbl[ebx+0x12]+pcry->tbl[eax+0x112];
			eax=(esi >> 8) & 0xff;
			ebx=ebx ^ pcry->tbl[eax+0x212];
			eax=esi & 0xff;
			ebx=ebx + pcry->tbl[eax+0x312];

			ebx=ebx ^ pcry->tbl[(x*2)+3];
			ecx= ecx ^ ebx;
			ebx=ecx >> 0x18;
			eax=(ecx >> 0x10) & 0xFF;
			ebx=pcry->tbl[ebx+0x12]+pcry->tbl[eax+0x112];
			eax=(ecx >> 8) & 0xff;
			ebx=ebx ^ pcry->tbl[eax+0x212];
			eax=ecx & 0xff;
			ebx=ebx + pcry->tbl[eax+0x312];
		}

		ebx=ebx ^ pcry->tbl[14];
		esi= esi ^ ebx;
		eax=esi >> 0x18;
		ebx=(esi >> 0x10) & 0xFF;
		eax=pcry->tbl[eax+0x12]+pcry->tbl[ebx+0x112];
		ebx=(esi >> 8) & 0xff;
		eax=eax ^ pcry->tbl[ebx+0x212];
		ebx=esi & 0xff;
		eax=eax + pcry->tbl[ebx+0x312];

		eax=eax ^ pcry->tbl[15];
		eax= ecx ^ eax;
		ecx=eax >> 0x18;
		ebx=(eax >> 0x10) & 0xFF;
		ecx=pcry->tbl[ecx+0x12]+pcry->tbl[ebx+0x112];
		ebx=(eax >> 8) & 0xff;
		ecx=ecx ^ pcry->tbl[ebx+0x212];
		ebx=eax & 0xff;
		ecx=ecx + pcry->tbl[ebx+0x312];

		ecx=ecx ^ pcry->tbl[16];
		ecx=ecx ^ esi;
		esi= pcry->tbl[17];
		esi=esi ^ eax;
		pcry->tbl[(edi / 4)]=esi;
		pcry->tbl[(edi / 4)+1]=ecx;
		edi=edi+8;
	}


	eax=0;
	edx=0;
	ou=0;
	while (ou < 0x1000)
	{
		edi=0x48;
		edx=0x448;

		while (edi < edx)
		{
			esi=esi ^ pcry->tbl[0];
			eax=esi >> 0x18;
			ebx=(esi >> 0x10) & 0xff;
			eax=pcry->tbl[eax+0x12]+pcry->tbl[ebx+0x112];
			ebx=(esi >> 8) & 0xFF;
			eax=eax ^ pcry->tbl[ebx+0x212];
			ebx=esi & 0xff;
			eax=eax + pcry->tbl[ebx+0x312];

			eax=eax ^ pcry->tbl[1];
			ecx= ecx ^ eax;
			ebx=ecx >> 0x18;
			eax=(ecx >> 0x10) & 0xFF;
			ebx=pcry->tbl[ebx+0x12]+pcry->tbl[eax+0x112];
			eax=(ecx >> 8) & 0xff;
			ebx=ebx ^ pcry->tbl[eax+0x212];
			eax=ecx & 0xff;
			ebx=ebx + pcry->tbl[eax+0x312];

			for (x = 0; x <= 5; x++)
			{
				ebx=ebx ^ pcry->tbl[(x*2)+2];
				esi= esi ^ ebx;
				ebx=esi >> 0x18;
				eax=(esi >> 0x10) & 0xFF;
				ebx=pcry->tbl[ebx+0x12]+pcry->tbl[eax+0x112];
				eax=(esi >> 8) & 0xff;
				ebx=ebx ^ pcry->tbl[eax+0x212];
				eax=esi & 0xff;
				ebx=ebx + pcry->tbl[eax+0x312];

				ebx=ebx ^ pcry->tbl[(x*2)+3];
				ecx= ecx ^ ebx;
				ebx=ecx >> 0x18;
				eax=(ecx >> 0x10) & 0xFF;
				ebx=pcry->tbl[ebx+0x12]+pcry->tbl[eax+0x112];
				eax=(ecx >> 8) & 0xff;
				ebx=ebx ^ pcry->tbl[eax+0x212];
				eax=ecx & 0xff;
				ebx=ebx + pcry->tbl[eax+0x312];
			}

			ebx=ebx ^ pcry->tbl[14];
			esi= esi ^ ebx;
			eax=esi >> 0x18;
			ebx=(esi >> 0x10) & 0xFF;
			eax=pcry->tbl[eax+0x12]+pcry->tbl[ebx+0x112];
			ebx=(esi >> 8) & 0xff;
			eax=eax ^ pcry->tbl[ebx+0x212];
			ebx=esi & 0xff;
			eax=eax + pcry->tbl[ebx+0x312];

			eax=eax ^ pcry->tbl[15];
			eax= ecx ^ eax;
			ecx=eax >> 0x18;
			ebx=(eax >> 0x10) & 0xFF;
			ecx=pcry->tbl[ecx+0x12]+pcry->tbl[ebx+0x112];
			ebx=(eax >> 8) & 0xff;
			ecx=ecx ^ pcry->tbl[ebx+0x212];
			ebx=eax & 0xff;
			ecx=ecx + pcry->tbl[ebx+0x312];

			ecx=ecx ^ pcry->tbl[16];
			ecx=ecx ^ esi;
			esi= pcry->tbl[17];
			esi=esi ^ eax;
			pcry->tbl[(ou / 4)+(edi / 4)]=esi;
			pcry->tbl[(ou / 4)+(edi / 4)+1]=ecx;
			edi=edi+8;
		}
		ou=ou+0x400;
	}
}

unsigned RleEncode(unsigned char *src, unsigned char *dest, unsigned src_size)
{
	unsigned char currChar, prevChar;             /* current and previous characters */
	unsigned short count;                /* number of characters in a run */
	unsigned src_end, dest_start;

	dest_start = (unsigned)dest;
	src_end = (unsigned)src + src_size;

	prevChar  = 0xFF - *src;

	while ((unsigned) src < src_end)
	{
		currChar = *(dest++) = *(src++);

		if ( currChar == prevChar )
		{
			if ( (unsigned) src == src_end )
			{
				*(dest++) = 0;
				*(dest++) = 0;
			}
			else
			{
				count = 0;
				while (((unsigned)src < src_end) && (count < 0xFFF0))
				{
					if (*src == prevChar)
					{
						count++;
						src++;
						if ( (unsigned) src == src_end )
						{
							*(unsigned short*) dest = count;
							dest += 2;
						}
					}
					else
					{
						*(unsigned short*) dest = count;
						dest += 2;
						prevChar = 0xFF - *src;
						break;
					}
				}
			}
		}
		else
			prevChar = currChar;
	}
	return (unsigned)dest - dest_start;
}

void RleDecode(unsigned char *src, unsigned char *dest, unsigned src_size)
{
    unsigned char currChar, prevChar;             /* current and previous characters */
    unsigned short count;                /* number of characters in a run */
	unsigned src_end;

	src_end = (unsigned) src + src_size;

    /* decode */

    prevChar = 0xFF - *src;     /* force next char to be different */

    /* read input until there's nothing left */

    while ((unsigned) src < src_end)
    {
		currChar = *(src++);

		*(dest++) = currChar;

        /* check for run */
        if (currChar == prevChar)
        {
            /* we have a run.  write it out. */
			count = *(unsigned short*) src;
			src += 2;
            while (count > 0)
            {
				*(dest++) = currChar;
                count--;
            }

            prevChar = 0xFF - *src;     /* force next char to be different */
        }
        else
        {
            /* no run */
            prevChar = currChar;
        }
    }
}

/* expand a key (makes a rc4_key) */

void prepare_key(unsigned char *keydata, unsigned len, struct rc4_key *key)
{
    unsigned index1, index2, counter;
    unsigned char *state;

    state = key->state;

    for (counter = 0; counter < 256; counter++)
        state[counter] = (unsigned char) counter;

    key->x = key->y = index1 = index2 = 0;

    for (counter = 0; counter < 256; counter++) {
        index2 = (keydata[index1] + state[counter] + index2) & 255;

        /* swap */
        state[counter] ^= state[index2];
        state[index2]  ^= state[counter];
        state[counter] ^= state[index2];

        index1 = (index1 + 1) % len;
    }
}

/* reversible encryption, will encode a buffer updating the key */

void rc4(unsigned char *buffer, unsigned len, struct rc4_key *key)
{
    unsigned x, y, xorIndex, counter;
    unsigned char *state;

    /* get local copies */
    x = key->x; y = key->y;
    state = key->state;

    for (counter = 0; counter < len; counter++) {
        x = (x + 1) & 255;
        y = (state[x] + y) & 255;

        /* swap */
        state[x] ^= state[y];
        state[y] ^= state[x];
        state[x] ^= state[y];

        xorIndex = (state[y] + state[x]) & 255;

        buffer[counter] ^= state[xorIndex];
    }

    key->x = x; key->y = y;
}

void compressShipPacket ( SERVER* ship, unsigned char* src, unsigned long src_size )
{
	unsigned char* dest;
	unsigned long result;

	if (ship->sockfd >= 0)
	{
		if (PACKET_BUFFER_SIZE - ship->snddata < (int) ( src_size + 100 ) )
			initialize_logon();
		else
		{
			dest = &ship->sndbuf[ship->snddata];
			// Store the original packet size before RLE compression at offset 0x04 of the new packet.
			dest += 4;
			*(unsigned *) dest = src_size;
			// Compress packet using RLE, storing at offset 0x08 of new packet.
			//
			// result = size of RLE compressed data + a DWORD for the original packet size.
			result = RleEncode (src, dest+4, src_size) + 4;
			// Encrypt with RC4
			rc4 (dest, result, &ship->sc_key);
			// Increase result by the size of a DWORD for the final ship packet size.
			result += 4;
			// Copy it to the front of the packet.
			*(unsigned *) &ship->sndbuf[ship->snddata] = result;
			ship->snddata += (int) result;
		}
	}
}

void decompressShipPacket ( SERVER* ship, unsigned char* dest, unsigned char* src )
{
	unsigned src_size, dest_size;
	unsigned char *srccpy;

	if (ship->crypt_on)
	{
		src_size = *(unsigned *) src;
		src_size -= 8;
		src += 4;
		srccpy = src;
		// Decrypt RC4
		rc4 (src, src_size+4, &ship->cs_key);
		// The first four bytes of the src should now contain the expected uncompressed data size.
		dest_size = *(unsigned *) srccpy;
		// Increase expected size by 4 before inserting into the destination buffer.  (To take account for the packet
		// size DWORD...)
		dest_size += 4;
		*(unsigned *) dest = dest_size;
		// Decompress the data...
		RleDecode (srccpy+4, dest+4, src_size);
	}
	else
	{
		src_size = *(unsigned *) src;
		memcpy (dest + 4, src + 4, src_size);
		src_size += 4;
		*(unsigned *) dest = src_size;
	}
}