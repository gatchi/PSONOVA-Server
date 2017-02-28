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
//#include "admin-client.h"

#define SHIP_URL "67.161.8.229"
#define SHIP_PORT 5278
#define BLOCK1_PORT 5279

#define SERVER_KEY_INDEX    104
#define SERVER_KEY_LEN       48
#define CLIENT_KEY_INDEX    152
#define CLIENT_KEY_LEN       48

void dumpx (unsigned char * string, int string_length);
int readmesg (SOCKET socket_with_something_to_say);
void sendmesg (SOCKET socket_to_send_to);
void extractkey (unsigned char * packet03, int key_index, int key_length);

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
	
	// Let's see what the ship sends
	readmesg (blocksock);
	
	//---- Maybe encryption --------//
	
	//---- Start command reading ---//
	
	return 0;
}

void sendmesg (SOCKET sock)
{
	int result;
	unsigned char mesg[13] = "Anyone there?";
	result = send (sock, mesg, (int)strlen(mesg), 0);
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
int readmesg (SOCKET sock)
{
	int result;
	char recvbuff[512] = {0};
	int already_sent = 0;  // Update to 1 when response has been sent
	do
	{
		result = recv (sock, recvbuff, 512, 0);
		if (result > 0)
		{
			printf ("Data recieved. Message:\n");
			//dumpx (recvbuff, 200);
			extractkey (recvbuff, SERVER_KEY_INDEX, SERVER_KEY_LEN);
			extractkey (recvbuff, CLIENT_KEY_INDEX, CLIENT_KEY_LEN);
			// if (!already_sent)
			// {
				// printf ("Sending one back...\n");
				// sendmesg (sock);
				// already_sent = 1;
			// }
		}
		else if (result == 0)
		{
			printf ("Connection closed.\n");
			closesocket (sock);
		}
		else
		{
			printf ("Connection error: %d\n", WSAGetLastError());
			closesocket (sock);
			return 1;
		}
	} while (result > 0);
}

void extractkey (unsigned char * in, int keyloc, int keylen)
{
	unsigned char buff[keylen];
	unsigned char * key = buff;
	int i;
	for (i=0; i<keylen; i++)
		buff[i] = in[keyloc+i];
	//dumpx (key, keylen);
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
