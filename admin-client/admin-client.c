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
//#include "admin-client.h"

#define SHIP_URL "67.161.8.229"
#define SHIP_PORT 5278

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
	SOCKET connectsock = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (connectsock == INVALID_SOCKET)
		printf ("Whoops, fucked up socket: %ld\n", WSAGetLastError());
	else
		printf ("Socket created.\n");
	
	// Setup ship sockaddr
	struct sockaddr_in ssa;
	ssa.sin_family = AF_INET;
	ssa.sin_addr.s_addr = inet_addr (SHIP_URL);
	ssa.sin_port = htons (SHIP_PORT);
	
	// Connect to ship socket
	int result = connect (connectsock, (struct sockaddr *) &ssa, sizeof(ssa));
	if (result < 0)
		printf ("Can't connect: %d\n", WSAGetLastError());
	else
		printf ("Connection made.\n");
	
	//---- Accept 'ShipSend0E' -----//
	
	//---- Maybe encryption --------//
	
	//---- Start command reading ---//
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
