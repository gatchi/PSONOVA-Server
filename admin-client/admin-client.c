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
#include "admin-client.h"
#include "pso_crypt.h"
#include "ship-funcs.h"

//#define SHIP_URL  "67.161.8.229"
#define SHIP_URL     "127.0.0.1"
#define SHIP_PORT          5278
#define BLOCK1_PORT        5279

#define SERVER_KEY_INDEX    104
#define SERVER_KEY_LEN       48
#define CLIENT_KEY_INDEX    152
#define CLIENT_KEY_LEN       48

void dumpx (unsigned char * string, int string_length);
unsigned char * pollmesg (SOCKET socket_with_something_to_say);
void sendmesg (SOCKET socket_to_send_via, char * message, int message_length);
int extractkey (unsigned char * packet03, unsigned char * key, int key_index, int key_length);

int main ()
{
	//---- Connect to ship ------//
	
	// Negotiate with winsock (windows networking) to get winsock data by providing version request
	WSADATA winsock_data;
	if ( !WSAStartup(MAKEWORD(2,2), &winsock_data) )
		printf ("WSAStartup a success.\n");
	else
		printf ("Could not negotiate with winsock.\n");
	
	// Make a socket
	SOCKET blocksock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (blocksock == INVALID_SOCKET)
		printf ("Whoops, fucked up socket: %ld\n", WSAGetLastError());
	else
		printf ("Socket created.\n");
	
	// Setup block sockaddr to represent the block server
	struct sockaddr_in bsa;
	bsa.sin_family = AF_INET;
	bsa.sin_addr.s_addr = inet_addr (SHIP_URL);
	bsa.sin_port = htons (BLOCK1_PORT);
	
	// Try to connect to a ship block
	int result = connect (blocksock, (struct sockaddr *) &bsa, sizeof(bsa));
	if (result <0)
		printf ("Can't connect: %d\n", WSAGetLastError());
	else
		printf ("Connection made to a block.\n");
	
	//---- Start encryption --------//
	
	// Receive first message (which should be encryption start packet (p03)
	unsigned char * recvbuff;
	unsigned char * serverkey;
	unsigned char * clientkey;
	
	serverkey = (unsigned char *) calloc (SERVER_KEY_LEN, SERVER_KEY_LEN * sizeof(char));
	clientkey = (unsigned char *) calloc (CLIENT_KEY_LEN, CLIENT_KEY_LEN * sizeof(char));
	
	recvbuff = pollmesg (blocksock);
	
	if (recvbuff == NULL)
	{
		printf ("Connection error while waiting for encryption packet. Aborted.\n");
		return 1;
	}
	
	if (recvbuff[0] == 0xC8)  // 200 (size of packet)
	{
		// This is almost certainly it
		extractkey (recvbuff, serverkey, SERVER_KEY_INDEX, SERVER_KEY_LEN);
		extractkey (recvbuff, clientkey, CLIENT_KEY_INDEX, CLIENT_KEY_LEN);
	}
	else
	{
		printf ("Received wrong packet. Are you sure youre connecting to the right server?\n");
		return 2;
	}
	
	dumpx (serverkey, SERVER_KEY_LEN);
	dumpx (clientkey, CLIENT_KEY_LEN);
	
	// Lets try disconnecting with a disconnect packet
	
	// Setup encryption
	PSO_CRYPT * hiscipher = calloc (1, sizeof(PSO_CRYPT));
	PSO_CRYPT * mycipher = calloc (1, sizeof(PSO_CRYPT));
	pso_crypt_table_init_bb (hiscipher, serverkey);
	pso_crypt_table_init_bb (mycipher, clientkey);
	
	// Encrypt packet
	int size = 0x08;
	unsigned char * mesg = malloc (sizeof(char));
	encryptcopy (mesg, Packet05, size, mycipher);
	sendmesg (blocksock, mesg, size);
	dumpx (mesg, size);
	
	// Unencrypt packet
	unsigned char * dmesg = malloc (sizeof(char));
	decryptcopy (dmesg, mesg, size, mycipher);
	dumpx (dmesg, size);
	
	pollmesg (blocksock);
	
	//---- Start command reading ---//
	
	return 0;
}

void sendmesg (SOCKET sock, char * mesg, int length)
{
	int result;
	result = send (sock, mesg, length, 0);
	if (result < 0)
		printf ("Can't send: %d\n", WSAGetLastError());
	else
		printf ("Message sent.\n");
}

/*
 * If socket has something to say, it displays it and then exits with 0.
 * It also exists with 0 if it receives a close request.
 * Exits with 1 if an error is encountered, not before printing the error.
 */
unsigned char * pollmesg (SOCKET sock)
{
	printf ("Listening...\n");
	int result;
	char recvbuff[512] = {0};
	unsigned char * mesg = recvbuff;
	int already_sent = 0;  // Update to 1 when response has been sent
	do
	{
		result = recv (sock, recvbuff, 512, 0);
		if (result > 0)
		{
			printf ("Data recieved.\n");
			dumpx (mesg, 200);
			return mesg;
		}
		else if (result == 0)
		{
			printf ("Connection closed.\n");
			closesocket (sock);
		}
		else
		{
			printf ("Connection error: %d\n", WSAGetLastError());
			return NULL;
		}
	} while (result > 0);
}

int extractkey (unsigned char * in, unsigned char * key, int keyloc, int keylen)
{
	//unsigned char * buff;
	//buff = (unsigned char *) malloc ( keylen*sizeof(char) );
	int i;
	for (i=0; i<keylen; i++)
		key[i] = in[keyloc+i];
	//dumpx (key, keylen);
	//return buff;
	return i;
}

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
