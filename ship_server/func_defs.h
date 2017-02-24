extern void	mt_bestseed(void);
extern void mt_goodseed(void);
extern void	mt_seed(void);	/* Choose seed from random input. */
extern unsigned long	mt_lrand(void);	/* Generate 32-bit random value */

char* Unicode_to_ASCII (unsigned short* ucs);
void WriteLog(char *fmt, ...);
void WriteGM(char *fmt, ...);
void ShipSend04 (unsigned char command, CLIENT* client, SERVER* ship);
void ShipSend0E (SERVER* ship);
void Send01 (const char *text, CLIENT* client);
void ShowArrows (CLIENT* client, int to_all);
unsigned char* MakePacketEA15 (CLIENT* client);
void SendToLobby (LOBBY* l, unsigned max_send, unsigned char* src, unsigned short size, unsigned nosend );
void removeClientFromLobby (CLIENT* client);

void debug(char *fmt, ...);
void debug_perror(char * msg);
void tcp_listen (int sockfd);
int tcp_accept (int sockfd, struct sockaddr *client_addr, int *addr_len );
//int tcp_sock_connect(char* dest_addr, int port);
int tcp_sock_connect(struct sockaddr_in sock);
int tcp_sock_open(struct in_addr ip, int port);

void encryptcopy (CLIENT* client, const unsigned char* src, unsigned size);
void decryptcopy (unsigned char* dest, const unsigned char* src, unsigned size);

void prepare_key(unsigned char *keydata, unsigned len, struct rc4_key *key);
void compressShipPacket ( SERVER* ship, unsigned char* src, unsigned long src_size );
void decompressShipPacket ( SERVER* ship, unsigned char* dest, unsigned char* src );
int qflag (unsigned char* flag_data, unsigned flag, unsigned difficulty);
