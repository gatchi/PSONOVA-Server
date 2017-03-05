/*********************************************************
	Admin Client
	
	Special client for sending commands via terminal
*********************************************************/

#ifdef _WIN32
	#include <winsock2.h>
#else
	#include <sys/socket.h>
	#include <arpa/inet.h>
	#define SOCKET int
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "pso_crypt.h"
#include "ship-funcs.h"

#include "admin-client.h"

#define CONNECTION_ERROR      1
#define WRONG_PACKET_ERROR    2

#define GUILDCARD_INDEX    0x0C
#define VERSION_INDEX      0x10
#define USERNAME_MAX_LEN     17
#define USERNAME_INDEX     0x1C
#define PASSWORD_MAX_LEN     17
#define PASSWORD_INDEX     0x4C

#define LOGIN_URL    "127.0.0.1"
#define SHIP_URL     "127.0.0.1"
#define LOGIN_PORT        12000
#define SHIP_PORT          5278
#define BLOCK1_PORT        5279

#define SERVER_KEY_INDEX    104
#define SERVER_KEY_LEN       48
#define CLIENT_KEY_INDEX    152
#define CLIENT_KEY_LEN       48
#define HEADER_SIZE           8  // (in bytes)
#define MAX_PACKET_SIZE     200
#define PACKET_TYPE_LOC       2
#define PACKET_LEN_LOC        0

#define SERVER_LIST_SIZE      2

void dumpx (unsigned char * in, int len)
{
	printf ("\n");
	int i = 0;
	int j = 1;
	printf ("%2.2d: ", j++);
	while (i<len)
	{
		printf ("%2.2X ", in[i]);
		if (++i % 10 == 0)
		{
			printf ("\n");
			printf ("%2.2d: ", j++);
		}
	}
	printf ("\n");
}


void setpackval (unsigned char * packet, int vali, const unsigned char * val, int len)
{
	int i;
	for (i=0; i<len; i++)
		packet[vali+i] = val[i];
}

void sendmesg (SOCKET sock, unsigned char * mesg, int length)
{
	int result;
	result = send (sock, mesg, length, 0);
	if (result < 0)
		printf ("Can't send: %d\n", WSAGetLastError());
	else
		printf ("Message sent.\n");
}

void sendemesg (SOCKET sock, unsigned char * mesg, const unsigned char * src, int length, PSO_CRYPT * cipher)
{
	memset (mesg, 0, length*sizeof(char));
	encryptcopy (mesg, src, length, cipher);
	sendmesg (sock, mesg, length);
}

unsigned char * pollmesg (SOCKET sock, unsigned char * buff)
{
	int result;
	do
	{
		result = recv (sock, buff, MAX_PACKET_SIZE, 0);  // TODO ! changing the size broke this
		if (result > 0)
		{
			printf ("Data recieved.\n");
			dumpx (buff, MAX_PACKET_SIZE);
			return buff;
		}
		else if (result == 0)
		{
			printf ("Connection closed.\n");
			closesocket (sock);
		}
		else
		{
			printf ("Connection error: %d\n", WSAGetLastError());
			exit(CONNECTION_ERROR);
		}
	} while (result > 0);
}

int copykey (unsigned char * in, unsigned char * key, int keyloc, int keylen)
{
	int i;
	for (i=0; i<keylen; i++)
		key[i] = in[keyloc+i];
	return i;
}

unsigned char * catchCryptPacket (SERVER * server)
{	
	server->key = calloc (SERVER_KEY_LEN, sizeof(char));
	server->ckey = calloc (CLIENT_KEY_LEN, sizeof(char));
	
	unsigned char * packet = calloc (MAX_PACKET_SIZE, sizeof(char));
	packet = pollmesg (server->socket, packet);
	
	if (packet[0] == 0xC8)  // 200 (size of packet)
	{
		// This is almost certainly it
		printf ("Caught packet.\nExtracting keys...\n");
		copykey (packet, server->key, SERVER_KEY_INDEX, SERVER_KEY_LEN);
		copykey (packet, server->ckey, CLIENT_KEY_INDEX, CLIENT_KEY_LEN);
		free (packet);
		printf ("Finished.\n\n");
		return server->ckey;
	}
	else
	{
		printf ("Received wrong packet. Are you sure youre connecting to the right server?\n");
		exit(WRONG_PACKET_ERROR);
	}
	if (packet == NULL)
	{
		printf ("Connection error while waiting for encryption packet. Aborted.\n");
		exit(CONNECTION_ERROR);
	}
}

void setupServer (SERVER * server, const unsigned char * name, unsigned char * ipaddr, int port)
{
	server->socket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server->socket == INVALID_SOCKET)
		printf ("Whoops, fucked up socket: %ld\n", WSAGetLastError());
	else
		printf ("Socket created.\n");
	
	server->sa.sin_family = AF_INET;
	server->sa.sin_addr.s_addr = inet_addr (ipaddr);
	server->sa.sin_port = htons (port);
	
	if (strlen(name) <= 10)
	{
		server->name = calloc (10, sizeof(char));
		memcpy (server->name, name, 10);
	}
}

void dock (SERVER * server)
{
	int result = connect (server->socket, (struct sockaddr *) &server->sa, sizeof(struct sockaddr));
	if (result <0)
		printf ("Can't connect: %d\n", WSAGetLastError());
	else
		printf ("Connection made.\n");
}

void netStartup()
{
	WSADATA winsock_data;
	if ( !WSAStartup(MAKEWORD(2,2), &winsock_data) )
		printf ("WSAStartup a success.\n");
	else
		printf ("Could not negotiate with winsock.\n");
}

int talk (SERVER * list[SERVER_LIST_SIZE])
{
	fd_set readlist;
	int new_sock;
	int activity;
	unsigned char * mesg = calloc (MAX_PACKET_SIZE, sizeof(char));
	unsigned char * recvbuff = calloc (MAX_PACKET_SIZE, sizeof(char));
	unsigned char header[HEADER_SIZE] = {0};
	unsigned char dheader[HEADER_SIZE] = {0};  // decrypted header
	unsigned char * pkttype = &dheader[PACKET_TYPE_LOC];
	unsigned char * cipher;
	
	while (1)
	{		
		FD_ZERO (&readlist);
		int maxrank = 0;
		int i;
		
		for (i=0; i<SERVER_LIST_SIZE; i++)
		{		
			if (list[i]->socket > 0)
				FD_SET (list[i]->socket, &readlist);
			if (list[i]->socket > maxrank)
				maxrank = list[i]->socket;
		}
		
		if (maxrank == 0)  // No more connections
			break;
		
		// Wait for activity
		activity = select (maxrank + 1, &readlist, NULL, NULL, NULL);
		if (activity == SOCKET_ERROR)
			printf ("Connection error: %d\n", WSAGetLastError());
		
		for (i=0; i<SERVER_LIST_SIZE; i++)
			if (FD_ISSET (list[i]->socket, &readlist))
			{
				printf ("! Message from %s...\n", list[i]->name);
				
				mesg = pollmesg (list[i]->socket, recvbuff);
				memcpy (header, mesg, HEADER_SIZE);
				decryptcopy (dheader, header, HEADER_SIZE, list[i]->cipher);
				
				// Determine the packet's intentions
				switch (*pkttype)
				{
					case 0x1D:  // ping (send reply back)
						sendemesg (list[i]->socket, mesg, Packet1D, (int) Packet1D[PACKET_LEN_LOC], list[i]->ccipher);
						printf ("ping pong\n");
						break;
					case 0x05:  // disconnect
						printf ("%s closed the connection.\n", list[i]->name);
						closesocket (list[i]->socket);
						break;
					default:
						printf ("Not sure what this is...\n");
						dumpx (dheader, HEADER_SIZE);
						break;
				}
			}
		
	}
}

int main()
{
	CLIENT our;
	unsigned char * recvbuff = calloc (MAX_PACKET_SIZE, sizeof(char));
	unsigned char * mesg = calloc (MAX_PACKET_SIZE, sizeof(char));
	
	SERVER login;
	SERVER block;
	SERVER * list[] = { &login, &block };
	
	
	//---- Prep -----------------//
	
	// Login packet (p93)
	setpackval (Packet93, USERNAME_INDEX, "gatchi", 6);
	setpackval (Packet93, PASSWORD_INDEX, "davidislol", 10);
	*(uint32_t *) &Packet93[GUILDCARD_INDEX] = (uint32_t) 42000001;
	*(uint16_t *) &Packet93[VERSION_INDEX] = (uint16_t) 12513;
	
	
	//---- Connect to login ------//
	
	// Negotiate with winsock (windows networking) to get winsock data by providing version request
	netStartup();
	printf ("\n");
	
	printf ("Connecting to login server...\n");
	setupServer (&login, "login", LOGIN_URL, LOGIN_PORT);
	dock (&login);
	printf ("\n");
	
	
	//---- Get login keys -------//
	
	// Receive first message (which should be encryption start packet (p03)
	printf ("Listening for crypt packet...\n");
	our.key.login = catchCryptPacket(&login);
	
	// Setup encryption
	login.cipher = malloc (sizeof (PSO_CRYPT));
	pso_crypt_table_init_bb (login.cipher, login.key);
	our.cipher.login = malloc (sizeof (PSO_CRYPT));
	pso_crypt_table_init_bb (our.cipher.login, our.key.login);
	
	// Send packet login packet (p93)
	printf ("Sending login packet to login...\n");
	sendemesg (login.socket, mesg, Packet93, Packet93[PACKET_LEN_LOC], our.cipher.login);
	
	
	// ---- Connect to block --------//
	
	printf ("Connecting to block...\n");
	setupServer (&block, "block", SHIP_URL, BLOCK1_PORT);
	dock (&block);
	printf ("\n");
	
	
	//---- Get block keys ----------//
	
	// Receive first message (which should be encryption start packet (p03)
	printf ("Listening for crypt packet...\n");
	our.key.block = catchCryptPacket(&block);
	
	// Setup encryption
	block.cipher = malloc (sizeof (PSO_CRYPT));
	pso_crypt_table_init_bb (block.cipher, block.key);
	our.cipher.block = malloc (sizeof (PSO_CRYPT));
	pso_crypt_table_init_bb (our.cipher.block, our.key.block);
	
	// Send packet login packet (p93)
	printf ("Sending login packet to block...\n");
	sendemesg (block.socket, mesg, Packet93, Packet93[PACKET_LEN_LOC], our.cipher.block);
	
	
	//---- Listen time -------------//
	
	login.ccipher = our.cipher.login;
	block.ccipher = our.cipher.block;
	talk (list);
	
	/* unsigned int pkttype;
	do {
		printf ("Listening for reply...\n");
		pkttype = dheader[PACKET_TYPE_LOC];
		memset (mesg, 0, MAX_PACKET_SIZE);
		mesg = pollmesg (logsock, recvbuff);
		memcpy (header, mesg, HEADER_SIZE);
		decryptcopy (dheader, header, HEADER_SIZE, theirlcipher);
		
		// Determine the packet's intentions
		switch (pkttype)
		{
			case 0x1D:  // ping (send reply back)
				sendemesg (blocksock, mesg, Packet1D, (int) Packet1D[PACKET_LEN_LOC], mylcipher);
				printf ("ping pong\n");
				break;
			default:
				printf ("Not sure what this is...\n");
				dumpx (dheader, HEADER_SIZE);
				break;
		}
	} while (pkttype != 0x05); */
	
	//---- Start command reading ---//
	
	return 0;
}


// Toggle announce
// void toggleAnnounce (CLIENT * c)
// {
	// if (c->announce != 0)
	// {
		// SendB0 ("Announce\ncancelled.", c);
		// c->announce = 0;
	// }
	// else
	// {
		// SendB0 ("Announce by\nsending a\nmail.", c);
		// c->announce = 1;
	// }
// }
