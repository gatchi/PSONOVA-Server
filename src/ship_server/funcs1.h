void Send08(CLIENT* client)
{
	BLOCK* b;
	unsigned ch,ch2,qNum;
	unsigned char game_flags, total_games;
	LOBBY* l;
	unsigned Offset;
	QUEST* q;

	if (client->block <= serverBlocks)
	{
		total_games = 0;
		b = blocks[client->block-1];
		Offset = 0x34;
		for (ch=16;ch<(16+SHIP_COMPILED_MAX_GAMES);ch++)
		{
			l = &b->lobbies[ch];
			if (l->in_use)
			{
				memset (&PacketData[Offset], 0, 44);
				// Output game
				Offset += 2;
				PacketData[Offset] = 0x03;
				Offset += 2;
				*(unsigned *) &PacketData[Offset] = ch;
				Offset += 4;
				PacketData[Offset++] = 0x22 + l->difficulty;
				PacketData[Offset++] = l->lobbyCount;
				memcpy(&PacketData[Offset], &l->gameName[0], 30);
				Offset += 32;
				if (!l->oneperson)
					PacketData[Offset++] = 0x40 + l->episode;
				else
					PacketData[Offset++] = 0x10 + l->episode;
				if (l->inpquest)
				{
					game_flags = 0x80;
					// Grey out Government quests that the player is not qualified for...
					q = &quests[l->quest_loaded - 1];
					memcpy (&dp[0], &q->ql[0]->qdata[0x31], 3);
					dp[4] = 0;
					qNum = (unsigned) atoi ( &dp[0] );
					switch (l->episode)
					{
					case 0x01:
						qNum -= 401;
						qNum <<= 1;
						qNum += 0x1F3;
						for (ch2=0x1F5;ch2<=qNum;ch2+=2)
							if (!qflag(&client->character.quest_data1[0], ch2, l->difficulty))
								game_flags |= 0x04;
						break;
					case 0x02:
						qNum -= 451;
						qNum <<= 1;
						qNum += 0x211;
						for (ch2=0x213;ch2<=qNum;ch2+=2)
							if (!qflag(&client->character.quest_data1[0], ch2, l->difficulty))
								game_flags |= 0x04;
						break;
					case 0x03:
						qNum -= 701;
						qNum += 0x2BC;
						for (ch2=0x2BD;ch2<=qNum;ch2++)
							if (!qflag(&client->character.quest_data1[0], ch2, l->difficulty))
								game_flags |= 0x04;
						break;
					}
				}
				else
					game_flags = 0x40;
				// Get flags for battle and one person games...
				if ((l->gamePassword[0x00] != 0x00) || 
					(l->gamePassword[0x01] != 0x00))
					game_flags |= 0x02;
				if ((l->quest_in_progress) || (l->oneperson)) // Can't join!
					game_flags |= 0x04;
				if (l->battle)
					game_flags |= 0x10;
				if (l->challenge)
					game_flags |= 0x20;
				// Wonder what flags 0x01 and 0x08 control....
				PacketData[Offset++] = game_flags;
				total_games++;
			}
		}
		*(unsigned short*) &client->encryptbuf[0x00] = (unsigned short) Offset;
		memcpy (&client->encryptbuf[0x02], &Packet08[2], 0x32);
		client->encryptbuf[0x04] = total_games;
		client->encryptbuf[0x08] = (unsigned char) client->lobbyNum;
		if ( client->block == 10 )
		{
			client->encryptbuf[0x1C] = 0x31;
			client->encryptbuf[0x1E] = 0x30;
		}
		else
			client->encryptbuf[0x1E] = 0x30 + client->block;

		if ( client->lobbyNum > 9 )
			client->encryptbuf[0x24] = 0x30 - ( 10 - client->lobbyNum );
		else
			client->encryptbuf[0x22] = 0x30 + client->lobbyNum;
		memcpy (&client->encryptbuf[0x34], &PacketData[0x34], Offset - 0x34);
		cipher_ptr = &client->server_cipher;
		encryptcopy ( client, &client->encryptbuf[0x00], Offset );
	}
}

void ConstructBlockPacket()
{
	unsigned short Offset;
	unsigned ch;
	char tempName[255];
	char* tn;
	unsigned BlockID;

	memset (&Packet07Data[0], 0, 0x4000);

	Packet07Data[0x02] = 0x07;
	Packet07Data[0x04] = serverBlocks+1;
	_itoa (serverID, &tempName[0], 10);
	if (serverID < 10) 
	{
		tempName[0] = 0x30;
		tempName[1] = 0x30+serverID;
		tempName[2] = 0x00;
	}
	else
		_itoa (serverID, &tempName[0], 10);
	strcat (&tempName[0], ":");
	strcat (&tempName[0], &Ship_Name[0]);
	Packet07Data[0x32] = 0x08;
	Offset = 0x12;
	tn = &tempName[0];
	while (*tn != 0x00)
	{
		Packet07Data[Offset++] = *(tn++);
		Packet07Data[Offset++] = 0x00;
	}
	Offset = 0x36;
	for (ch=0;ch<serverBlocks;ch++)
	{
				Packet07Data[Offset] = 0x12;
				BlockID = 0xEFFFFFFF - ch;
				*(unsigned *) &Packet07Data[Offset+2] = BlockID;
				memcpy (&Packet07Data[Offset+0x08], &blockString[0], 10 );
				if ( ch+1 < 10 )
				{
					Packet07Data[Offset+0x12] = 0x30;
					Packet07Data[Offset+0x14] = 0x30 + (ch+1);
				}
				else
				{
					Packet07Data[Offset+0x12] = 0x31;
					Packet07Data[Offset+0x14] = 0x30;
				}
				Offset += 0x2C;
	}
	Packet07Data[Offset] = 0x12;
	BlockID = 0xFFFFFF00;
	*(unsigned *) &Packet07Data[Offset+2] = BlockID;
	memcpy (&Packet07Data[Offset+0x08], &shipSelectString[0], 22 );
	Offset += 0x2C;
	while (Offset % 8)
		Packet07Data[Offset++] = 0x00;
	*(unsigned short*) &Packet07Data[0x00] = (unsigned short) Offset;
	Packet07Size = Offset;
}

void initialize_logon()
{
	unsigned ch;

	logon_ready = 0;
	logon_tick = 0;
	
	if ( logon.sockfd >= 0 )  // I think htis says, if socket is bound already, unbind
		closesocket ( logon.sockfd );
	memset (&logon, 0, sizeof (logon));  // blank the whole struct instance for some reason
	logon.sockfd = -1;
	for (ch=0;ch<128;ch++)
		logon.key_change[ch] = -1;      // and then set all of key_change to -1....
	
	// Here's the important part:
	// Into a inaddr struct, is a (cast) unsigned long int representation
	// of the logon server's ip address.
	// This is the actual logon initialization.
	logon.sock.sin_addr.s_addr = *(unsigned *) &loginIP[0];
	logon.sock.sin_port = htons(LOGON_PORT);
	logon.sock.sin_family = AF_INET;
}

void reconnect_logon()
{
	// Just in case this is called because of an error in communication with the logon server

	logon.sockfd = tcp_sock_connect ( logon.sock );
	if (logon.sockfd >= 0)
	{
		printf ("Connection successful!\n");
		logon.last_ping = (unsigned) time(NULL);
	}
	else
	{
		printf ("Connection failed.  Retry in %u seconds...\n",  LOGIN_RECONNECT_SECONDS);
		logon_tick = 0;
	}
}

unsigned free_connection()
{
	unsigned fc;
	CLIENT* wc;
	
	for (fc=0;fc<serverMaxConnections;fc++)
	{
		wc = connections[fc];
		if (wc->plySockfd<0)
			return fc;
	}
	return 0xFFFF;
}

void initialize_connection (CLIENT* connect)
{
	unsigned ch, ch2;

	// Free backup character memory
	if (connect->character_backup)
	{
		if (connect->mode)
			memcpy (&connect->character, connect->character_backup, sizeof (connect->character));
		free (connect->character_backup);
		connect->character_backup = NULL;
	}

	// Only one account at a time, it seems
	if (connect->guildcard)
	{
		removeClientFromLobby (connect);

		if ((connect->block) && (connect->block <= serverBlocks))
			blocks[connect->block - 1]->count--;

		if (connect->gotchardata == 1)
		{
			connect->character.playTime += (unsigned) servertime - connect->connected;
			ShipSend04 (0x02, connect, &logon);
		}
	}

	// Um, re-order serverConnectionList i guess?
	if (connect->plySockfd >= 0)
	{
		ch2 = 0;
		for (ch=0;ch<serverNumConnections;ch++)
		{
			if (serverConnectionList[ch] != connect->connection_index)
				serverConnectionList[ch2++] = serverConnectionList[ch];
		}
		serverNumConnections = ch2;
		closesocket (connect->plySockfd);
	}

	if (logon_ready)
	{
		printf ("Player Count: %u\n", serverNumConnections);
		ShipSend0E (&logon);
	}

	memset (connect, 0, sizeof (CLIENT) );
	connect->plySockfd = -1;
	connect->block = -1;
	connect->lastTick = 0xFFFFFFFF;
	connect->slotnum = -1;
	connect->sending_quest = -1;
}

void start_encryption(CLIENT* connect)
{
	unsigned c, c3, c4, connectNum;
	CLIENT *workConnect, *c5;

	// Limit the number of connections from an IP address to MAX_SIMULTANEOUS_CONNECTIONS.

	c3 = 0;

	for (c=0;c<serverNumConnections;c++)
	{
		connectNum = serverConnectionList[c];
		workConnect = connections[connectNum];
		//debug ("%s comparing to %s", (char*) &workConnect->IP_Address[0], (char*) &connect->IP_Address[0]);
		if ((!strcmp(&workConnect->IP_Address[0], &connect->IP_Address[0])) &&
			(workConnect->plySockfd >= 0))
			c3++;
	}

	//debug ("Matching count: %u", c3);

	if (c3 > MAX_SIMULTANEOUS_CONNECTIONS)
	{
		// More than MAX_SIMULTANEOUS_CONNECTIONS connections from a certain IP address...
		// Delete oldest connection to server.
		c4 = 0xFFFFFFFF;
		c5 = NULL;
		for (c=0;c<serverNumConnections;c++)
		{
			connectNum = serverConnectionList[c];
			workConnect = connections[connectNum];
			if ((!strcmp(&workConnect->IP_Address[0], &connect->IP_Address[0])) &&
				(workConnect->plySockfd >= 0))
			{
				if (workConnect->connected < c4)
				{
					c4 = workConnect->connected;
					c5 = workConnect;
				}
			}
		}
		if (c5)
		{
			workConnect = c5;
			initialize_connection (workConnect);
		}
	}

	// This is where the ship preps the first message to send
	memcpy (&connect->sndbuf[0], &Packet03[0], sizeof (Packet03));
	for (c=0;c<0x30;c++)
	{
		connect->sndbuf[0x68+c] = (unsigned char) mt_lrand() % 255;
		connect->sndbuf[0x98+c] = (unsigned char) mt_lrand() % 255;
	}
	connect->snddata += sizeof (Packet03);
	cipher_ptr = &connect->server_cipher;
	pso_crypt_table_init_bb (cipher_ptr, &connect->sndbuf[0x68]);
	cipher_ptr = &connect->client_cipher;
	pso_crypt_table_init_bb (cipher_ptr, &connect->sndbuf[0x98]);
	connect->crypt_on = 1;
	connect->sendCheck[SEND_PACKET_03] = 1;
	connect->connected = connect->response = connect->savetime = (unsigned) servertime;
}

void SendToLobby (LOBBY* l, unsigned max_send, unsigned char* src, unsigned short size, unsigned nosend )
{
	unsigned ch;

	if (!l)
		return;

	for (ch=0;ch<max_send;ch++)
	{
		if ((l->slot_use[ch]) && (l->client[ch]) && (l->client[ch]->guildcard != nosend))
		{
			cipher_ptr = &l->client[ch]->server_cipher;
			encryptcopy (l->client[ch], src, size);
		}
	}
}


void removeClientFromLobby (CLIENT* client)
{
	unsigned ch, maxch, lowestID;

	LOBBY* l;

	if (!client->lobby)
		return;

	l = (LOBBY*) client->lobby;

	if (client->clientID < 12)
	{
		l->slot_use[client->clientID] = 0;
		l->client[client->clientID] = 0;
	}

	if (client->lobbyNum > 0x0F)
		maxch = 4;
	else
		maxch = 12;

	l->lobbyCount = 0;

	for (ch=0;ch<maxch;ch++)
	{
		if ((l->client[ch]) && (l->slot_use[ch]))
			l->lobbyCount++;
	}

	if ( l->lobbyCount )
	{
		if ( client->lobbyNum < 0x10 )
		{
			Packet69[0x08] = client->clientID;
			SendToLobby ( client->lobby, 12, &Packet69[0], 0x0C, client->guildcard );
		}
		else
		{
			Packet66[0x08] = client->clientID;
			if (client->clientID == l->leader)
			{
				// Leader change...
				lowestID = 0xFFFFFFFF;
				for (ch=0;ch<4;ch++)
				{
					if ((l->slot_use[ch]) && (l->client[ch]) && (l->gamePlayerID[ch] < lowestID))
					{
						// Change leader to oldest person to join game...
						lowestID = l->gamePlayerID[ch];
						l->leader = ch;
					}
				}
				Packet66[0x0A] = 0x01;
			}
			else
				Packet66[0x0A] = 0x00;
			Packet66[0x09] = l->leader;
			SendToLobby ( client->lobby, 4, &Packet66[0], 0x0C, client->guildcard );
		}
	}
	else
		memset ( l, 0, sizeof ( LOBBY ) );
	client->hasquest = 0;
	client->lobbyNum = 0;
	client->lobby = 0;
}


void Send1A (const char *mes, CLIENT* client)
{
	unsigned short x1A_Len;

	memcpy (&PacketData[0], &Packet1A[0], sizeof (Packet1A));
	x1A_Len = sizeof (Packet1A);

	while (*mes != 0x00)
	{
		PacketData[x1A_Len++] = *(mes++);
		PacketData[x1A_Len++] = 0x00;
	}

	PacketData[x1A_Len++] = 0x00;
	PacketData[x1A_Len++] = 0x00;

	while (x1A_Len % 8)
		PacketData[x1A_Len++] = 0x00;

	*(unsigned short*) &PacketData[0] = x1A_Len;
	cipher_ptr = &client->server_cipher;
	encryptcopy (client, &PacketData[0], x1A_Len);
}

void Send1D (CLIENT* client)
{
	unsigned num_minutes;

	if ((((unsigned) servertime - client->savetime) / 60L) >= 5)
	{
		// Backup character data every 5 minutes.
		client->savetime = (unsigned) servertime;
		ShipSend04 (0x02, client, &logon);
	}

	num_minutes = ((unsigned) servertime - client->response) / 60L;
	if (num_minutes)
	{
		if (num_minutes > 2)
			initialize_connection (client); // If the client hasn't responded in over two minutes, drop the connection.
		else
		{
			cipher_ptr = &client->server_cipher;
			encryptcopy (client, &Packet1D[0], sizeof (Packet1D));
		}
	}
}

void Send83 (CLIENT* client)
{
	cipher_ptr = &client->server_cipher;
	encryptcopy (client, &Packet83[0], sizeof (Packet83));

}

unsigned free_game (CLIENT* client)
{
	unsigned ch;
	LOBBY* l;

	for (ch=16;ch<(16+SHIP_COMPILED_MAX_GAMES);ch++)
	{
		l = &blocks[client->block - 1]->lobbies[ch];
		if (l->in_use == 0)
			return ch;
	}
	return 0;
}

void ParseMapData (LOBBY* l, MAP_MONSTER* mapData, int aMob, unsigned num_records)
{
	MAP_MONSTER* mm;
	unsigned ch, ch2;
	unsigned num_recons;
	int r;
	
	for (ch2=0;ch2<num_records;ch2++)
	{
		if ( l->mapIndex >= 0xB50 )
			break;
		memcpy (&l->mapData[l->mapIndex], mapData, 72);
		mapData++;
		mm = &l->mapData[l->mapIndex];
		mm->exp = 0;
		switch ( mm->base )
		{
		case 64:
			// Hildebear and Hildetorr
			r = 0;
			if ( mm->skin & 0x01 ) // Set rare from a quest?
				r = 1;
			else
				if ( ( l->rareIndex < 0x1E ) && ( mt_lrand() < hildebear_rate ) )
				{
					*(unsigned short*) &l->rareData[l->rareIndex] = (unsigned short) l->mapIndex;
					l->rareIndex += 2;
					r = 1;
				}
			if ( r )
			{
				mm->rt_index = 0x02;
				mm->exp = l->bptable[0x4A].XP;
			}
			else
			{
				mm->rt_index = 0x01;
				mm->exp = l->bptable[0x49].XP;
			}
			break;
		case 65:
			// Rappies
			r = 0;
			if ( mm->skin & 0x01 ) // Set rare from a quest?
				r = 1;
			else
				if ( ( l->rareIndex < 0x1E ) && ( mt_lrand() < rappy_rate ) )
				{
					*(unsigned short*) &l->rareData[l->rareIndex] = (unsigned short) l->mapIndex;
					l->rareIndex += 2;
					r = 1;
				}
			if ( l->episode == 0x03 )
			{
				// Del Rappy and Sand Rappy
				if ( aMob )
				{
					if ( r )
					{
						mm->rt_index = 18;
						mm->exp = l->bptable[0x18].XP;
					}
					else
					{
						mm->rt_index = 17;
						mm->exp = l->bptable[0x17].XP;
					}
				}
				else
				{
					if ( r )
					{
						mm->rt_index = 18;
						mm->exp = l->bptable[0x06].XP;
					}
					else
					{
						mm->rt_index = 17;
						mm->exp = l->bptable[0x05].XP;
					}
				}
			}
			else
			{
				// Rag Rappy, Al Rappy, Love Rappy and Seasonal Rappies
				if ( r )
				{
					if ( l->episode == 0x01 )
						mm->rt_index = 6; // Al Rappy
					else
					{
						switch ( shipEvent )
						{
						case 0x01:
							mm->rt_index = 79; // St. Rappy
							break;
						case 0x04:
							mm->rt_index = 81; // Easter Rappy
							break;
						case 0x05:
							mm->rt_index = 80; // Halo Rappy
							break;
						default:
							mm->rt_index = 51; // Love Rappy
							break;
						}
					}
					mm->exp = l->bptable[0x19].XP;
				}
				else
				{
					mm->rt_index = 5;
					mm->exp = l->bptable[0x18].XP;
				}
			}
			break;
		case 66:
			// Monest + 30 Mothmants
			mm->exp = l->bptable[0x01].XP;
			mm->rt_index = 4;

			for (ch=0;ch<30;ch++)
			{
				l->mapIndex++;
				mm++;
				mm->rt_index = 3;
				mm->exp = l->bptable[0x00].XP;
			}
			break;
		case 67:
			// Savage Wolf and Barbarous Wolf
			if ( ( ( mm->reserved11 - FLOAT_PRECISION ) < (float) 1.00000 ) &&
				 ( ( mm->reserved11 + FLOAT_PRECISION ) > (float) 1.00000 ) ) // set rare?
			{
				mm->rt_index = 8;
				mm->exp = l->bptable[0x03].XP;
			}
			else
			{
				mm->rt_index = 7;
				mm->exp = l->bptable[0x02].XP;
			}
			break;
		case 68:
			// Booma family
			if ( mm->skin & 0x02 )
			{
				mm->rt_index = 11;
				mm->exp = l->bptable[0x4D].XP;
			}
			else
				if ( mm->skin & 0x01 )
				{
					mm->rt_index = 10;
					mm->exp = l->bptable[0x4C].XP;
				}
				else
				{
					mm->rt_index = 9;
					mm->exp = l->bptable[0x4B].XP;
				}
			break;
		case 96:
			// Grass Assassin
			mm->rt_index = 12;
			mm->exp = l->bptable[0x4E].XP;
			break;
		case 97:
			// Del Lily, Poison Lily, Nar Lily
			r = 0;
			if ( ( ( mm->reserved11 - FLOAT_PRECISION ) < (float) 1.00000 ) &&
				 ( ( mm->reserved11 + FLOAT_PRECISION ) > (float) 1.00000 ) ) // set rare?
				r = 1;
			else
				if ( ( l->rareIndex < 0x1E ) && ( mt_lrand() < lily_rate ) )
				{
					*(unsigned short*) &l->rareData[l->rareIndex] = (unsigned short) l->mapIndex;
					l->rareIndex += 2;
					r = 1;
				}
			if ( ( l->episode == 0x02 ) && ( aMob ) )
			{
				mm->rt_index = 83;
				mm->exp = l->bptable[0x25].XP;
			}
			else
				if ( r )
				{
					mm->rt_index = 14;
					mm->exp = l->bptable[0x05].XP;
				}
				else
				{
					mm->rt_index = 13;
					mm->exp = l->bptable[0x04].XP;
				}
			break;
		case 98:
			// Nano Dragon
			mm->rt_index = 15;
			mm->exp = l->bptable[0x1A].XP;
			break;
		case 99:
			// Shark family
			if ( mm->skin & 0x02 )
			{
				mm->rt_index = 18;
				mm->exp = l->bptable[0x51].XP;
			}
			else
				if ( mm->skin & 0x01 )
				{
					mm->rt_index = 17;
					mm->exp = l->bptable[0x50].XP;
				}
				else
				{
					mm->rt_index = 16;
					mm->exp = l->bptable[0x4F].XP;
				}
			break;
		case 100:
			// Slime + 4 clones
			r = 0;
			if ( ( ( mm->reserved11 - FLOAT_PRECISION ) < (float) 1.00000 ) &&
				 ( ( mm->reserved11 + FLOAT_PRECISION ) > (float) 1.00000 ) ) // set rare?
				r = 1;
			else
				if ( ( l->rareIndex < 0x1E ) && ( mt_lrand() < slime_rate ) )
				{
					*(unsigned short*) &l->rareData[l->rareIndex] = (unsigned short) l->mapIndex;
					l->rareIndex += 2;
					r = 1;
				}
			if ( r )
			{
				mm->rt_index = 20;
				mm->exp = l->bptable[0x2F].XP;
			}
			else
			{
				mm->rt_index = 19;
				mm->exp = l->bptable[0x30].XP;
			}
			for (ch=0;ch<4;ch++)
			{
				l->mapIndex++;
				mm++;
				r = 0;
				if ( ( l->rareIndex < 0x1E ) && ( mt_lrand() < slime_rate ) )
				{
					*(unsigned short*) &l->rareData[l->rareIndex] = (unsigned short) l->mapIndex;
					l->rareIndex += 2;
					r = 1;
				}
				if ( r )
				{
					mm->rt_index = 20;
					mm->exp = l->bptable[0x2F].XP;
				}
				else
				{
					mm->rt_index = 19;
					mm->exp = l->bptable[0x30].XP;
				}
			}
			break;
		case 101:
			// Pan Arms, Migium, Hidoom
			mm->rt_index = 21;
			mm->exp = l->bptable[0x31].XP;
			l->mapIndex++;
			mm++;
			mm->rt_index = 22;
			mm->exp = l->bptable[0x32].XP;
			l->mapIndex++;
			mm++;
			mm->rt_index = 23;
			mm->exp = l->bptable[0x33].XP;
			break;
		case 128:
			// Dubchic and Gilchic
			if (mm->skin & 0x01)
			{
				mm->exp = l->bptable[0x1C].XP;
				mm->rt_index = 50;
			}
			else
			{
				mm->exp = l->bptable[0x1B].XP;
				mm->rt_index = 24;
			}
			break;
		case 129:
			// Garanz
			mm->rt_index = 25;
			mm->exp = l->bptable[0x1D].XP;
			break;
		case 130:
			// Sinow Beat and Gold
			if ( ( ( mm->reserved11 - FLOAT_PRECISION ) < (float) 1.00000 ) &&
				 ( ( mm->reserved11 + FLOAT_PRECISION ) > (float) 1.00000 ) ) // set rare?
			{
				mm->rt_index = 27;
				mm->exp = l->bptable[0x13].XP;
			}
			else
			{
				mm->rt_index = 26;
				mm->exp = l->bptable[0x06].XP;
			}

			if ( ( mm->reserved[0] >> 16 ) == 0 )  
				l->mapIndex += 4; // Add 4 clones but only if there's no add value there already...
			break;
		case 131:
			// Canadine
			mm->rt_index = 28;
			mm->exp = l->bptable[0x07].XP;
			break;
		case 132:
			// Canadine Group
			mm->rt_index = 29;
			mm->exp = l->bptable[0x09].XP;
			for (ch=0;ch<8;ch++)
			{
				l->mapIndex++;
				mm++;
				mm->rt_index = 28;
				mm->exp = l->bptable[0x08].XP;
			}
			break;
		case 133:
			// Dubwitch
			break;
		case 160:
			// Delsaber
			mm->rt_index = 30;
			mm->exp = l->bptable[0x52].XP;
			break;
		case 161:
			// Chaos Sorcerer + 2 Bits
			mm->rt_index = 31;
			mm->exp = l->bptable[0x0A].XP;
			l->mapIndex += 2;
			break;
		case 162:
			// Dark Gunner
			mm->rt_index = 34;
			mm->exp = l->bptable[0x1E].XP;
			break;
		case 164:
			// Chaos Bringer
			mm->rt_index = 36;
			mm->exp = l->bptable[0x0D].XP;
			break;
		case 165:
			// Dark Belra
			mm->rt_index = 37;
			mm->exp = l->bptable[0x0E].XP;
			break;
		case 166:
			// Dimenian family
			if ( mm->skin & 0x02 )
			{
				mm->rt_index = 43;
				mm->exp = l->bptable[0x55].XP;
			}
			else
				if ( mm->skin & 0x01 )
				{
					mm->rt_index = 42;
					mm->exp = l->bptable[0x54].XP;
				}
				else
				{
					mm->rt_index = 41;
					mm->exp = l->bptable[0x53].XP;
				}
			break;
		case 167:
			// Bulclaw + 4 claws
			mm->rt_index = 40;
			mm->exp = l->bptable[0x1F].XP;
			for (ch=0;ch<4;ch++)
			{
				l->mapIndex++;
				mm++;
				mm->rt_index = 38;
				mm->exp = l->bptable[0x20].XP;
			}
			break;
		case 168:
			// Claw
			mm->rt_index = 38;
			mm->exp = l->bptable[0x20].XP;
			break;
		case 192:
			// Dragon or Gal Gryphon
			if ( l->episode == 0x01 )
			{
				mm->rt_index = 44;
				mm->exp = l->bptable[0x12].XP;
			}
			else
				if ( l->episode == 0x02 )
				{
					mm->rt_index = 77;
					mm->exp = l->bptable[0x1E].XP;
				}
			break;
		case 193:
			// De Rol Le
			mm->rt_index = 45;
			mm->exp = l->bptable[0x0F].XP;
			break;
		case 194:
			// Vol Opt form 1
			break;
		case 197:
			// Vol Opt form 2
			mm->rt_index = 46;
			mm->exp = l->bptable[0x25].XP;
			break;
		case 200:
			// Dark Falz + 510 Helpers
			mm->rt_index = 47;
			if (l->difficulty)
				mm->exp = l->bptable[0x38].XP; // Form 2
			else
				mm->exp = l->bptable[0x37].XP;

			for (ch=0;ch<510;ch++)
			{
				l->mapIndex++;
				mm++;
				mm->base = 200;
				mm->exp = l->bptable[0x35].XP;
			}
			break;
		case 202:
			// Olga Flow
			mm->rt_index = 78;
			mm->exp = l->bptable[0x2C].XP;
			l->mapIndex += 512;
			break;
		case 203:
			// Barba Ray
			mm->rt_index = 73;
			mm->exp = l->bptable[0x0F].XP;
			l->mapIndex += 47;
			break;
		case 204:
			// Gol Dragon
			mm->rt_index = 76;
			mm->exp = l->bptable[0x12].XP;
			l->mapIndex += 5;
			break;
		case 212:
			// Sinow Berill & Spigell
			/* if ( ( ( mm->reserved11 - FLOAT_PRECISION ) < (float) 1.00000 ) &&
				 ( ( mm->reserved11 + FLOAT_PRECISION ) > (float) 1.00000 ) ) */
			if ( mm->skin >= 0x01 ) // set rare?
			{
				mm->rt_index = 63;
				mm->exp = l->bptable [0x13].XP;
			}
			else
			{
				mm->rt_index = 62;
				mm->exp = l->bptable [0x06].XP;
			}
			l->mapIndex += 4; // Add 4 clones which are never used...
			break;
		case 213:
			// Merillia & Meriltas
			if ( mm->skin & 0x01 )
			{
				mm->rt_index = 53;
				mm->exp = l->bptable [0x4C].XP;
			}
			else
			{
				mm->rt_index = 52;
				mm->exp = l->bptable [0x4B].XP;
			}
			break;
		case 214:
			if ( mm->skin & 0x02 )
			{
				// Mericus
				mm->rt_index = 58;
				mm->exp = l->bptable [0x46].XP;
			}
			else 
				if ( mm->skin & 0x01 )
				{
					// Merikle
					mm->rt_index = 57;
					mm->exp = l->bptable [0x45].XP;
				}
				else
				{
					// Mericarol
					mm->rt_index = 56;
					mm->exp = l->bptable [0x3A].XP;
				}
			break;
		case 215:
			// Ul Gibbon and Zol Gibbon
			if ( mm->skin & 0x01 )
			{
				mm->rt_index = 60;
				mm->exp = l->bptable [0x3C].XP;
			}
			else
			{
				mm->rt_index = 59;
				mm->exp = l->bptable [0x3B].XP;
			}
			break;
		case 216:
			// Gibbles
			mm->rt_index = 61;
			mm->exp = l->bptable [0x3D].XP;
			break;
		case 217:
			// Gee
			mm->rt_index = 54;
			mm->exp = l->bptable [0x07].XP;
			break;
		case 218:
			// Gi Gue
			mm->rt_index = 55;
			mm->exp = l->bptable [0x1A].XP;
			break;
		case 219:
			// Deldepth
			mm->rt_index = 71;
			mm->exp = l->bptable [0x30].XP;
			break;
		case 220:
			// Delbiter
			mm->rt_index = 72;
			mm->exp = l->bptable [0x0D].XP;
			break;
		case 221:
			// Dolmolm and Dolmdarl
			if ( mm->skin & 0x01 )
			{
				mm->rt_index = 65;
				mm->exp = l->bptable[0x50].XP;
			}
			else
			{
				mm->rt_index = 64;
				mm->exp = l->bptable[0x4F].XP;
			}
			break;
		case 222:
			// Morfos
			mm->rt_index = 66;
			mm->exp = l->bptable [0x40].XP;
			break;
		case 223:
			// Recobox & Recons
			mm->rt_index = 67;
			mm->exp = l->bptable[0x41].XP;
			num_recons = ( mm->reserved[0] >> 16 );
			for (ch=0;ch<num_recons;ch++)
			{
				if ( l->mapIndex >= 0xB50 )
					break;
				l->mapIndex++;
				mm++;
				mm->rt_index = 68;
				mm->exp = l->bptable[0x42].XP;
			}
			break;
		case 224:
			if ( ( l->episode == 0x02 ) && ( aMob ) )
			{
				// Epsilon
				mm->rt_index = 84;
				mm->exp = l->bptable[0x23].XP;
				l->mapIndex += 4;
			}
			else
			{
				// Sinow Zoa and Zele
				if ( mm->skin & 0x01 )
				{
					mm->rt_index = 70;
					mm->exp = l->bptable[0x44].XP;
				}
				else
				{
					mm->rt_index = 69;
					mm->exp = l->bptable[0x43].XP;
				}
			}
			break;
		case 225:
			// Ill Gill
			mm->rt_index = 82;
			mm->exp = l->bptable[0x26].XP;
			break;
		case 272:
			// Astark
			mm->rt_index = 1;
			mm->exp = l->bptable[0x09].XP;
			break;
		case 273:
			// Satellite Lizard and Yowie
			if ( ( ( mm->reserved11 - FLOAT_PRECISION ) < (float) 1.00000 ) &&
				 ( ( mm->reserved11 + FLOAT_PRECISION ) > (float) 1.00000 ) ) // set rare?
			{
				if ( aMob )
				{
					mm->rt_index = 2;
					mm->exp = l->bptable[0x1E].XP;
				}
				else
				{
					mm->rt_index = 2;
					mm->exp = l->bptable[0x0E].XP;
				}
			}
			else
			{
				if ( aMob )
				{
					mm->rt_index = 3;
					mm->exp = l->bptable[0x1D].XP;
				}
				else
				{
					mm->rt_index = 3;
					mm->exp = l->bptable[0x0D].XP;
				}
			}
			break;
		case 274:
			// Merissa A/AA
			r = 0;
			if ( mm->skin & 0x01 ) // Set rare from a quest?
				r = 1;
			else
				if ( ( l->rareIndex < 0x1E ) && ( mt_lrand() < merissa_rate ) )
				{
					*(unsigned short*) &l->rareData[l->rareIndex] = (unsigned short) l->mapIndex;
					l->rareIndex += 2;
					r = 1;
				}
			if ( r )
			{
				mm->rt_index = 5;
				mm->exp = l->bptable[0x1A].XP;
			}
			else
			{
				mm->rt_index = 4;
				mm->exp = l->bptable[0x19].XP;
			}
			break;
		case 275:
			// Girtablulu
			mm->rt_index = 6;
			mm->exp = l->bptable[0x1F].XP;
			break;
		case 276:
			// Zu and Pazuzu
			r = 0;
			if ( mm->skin & 0x01 ) // Set rare from a quest?
				r = 1;
			else
				if ( ( l->rareIndex < 0x1E ) && ( mt_lrand() < pazuzu_rate ) )
				{
					*(unsigned short*) &l->rareData[l->rareIndex] = (unsigned short) l->mapIndex;
					l->rareIndex += 2;
					r = 1;
				}
			if ( r )
			{
				if ( aMob )
				{
					mm->rt_index = 8;
					mm->exp = l->bptable[0x1C].XP;
				}
				else
				{
					mm->rt_index = 8;
					mm->exp = l->bptable[0x08].XP;
				}
			}
			else
			{
				if ( aMob )
				{
					mm->rt_index = 7;
					mm->exp = l->bptable[0x1B].XP;
				}
				else
				{
					mm->rt_index = 7;
					mm->exp = l->bptable[0x07].XP;
				}
			}
			break;
		case 277:
			// Boota family
			if ( mm->skin & 0x02 )
			{
				mm->rt_index = 11;			
				mm->exp = l->bptable [0x03].XP;
			}
			else
				if ( mm->skin & 0x01 )
				{
					mm->rt_index = 10;
					mm->exp = l->bptable [0x01].XP;
				}
				else
				{
					mm->rt_index = 9;
					mm->exp = l->bptable [0x00].XP;
				}
			break;
		case 278:
			// Dorphon and Eclair
			r = 0;
			if ( mm->skin & 0x01 ) // Set rare from a quest?
				r = 1;
			else
				if ( ( l->rareIndex < 0x1E ) && ( mt_lrand() < dorphon_rate ) )
				{
					*(unsigned short*) &l->rareData[l->rareIndex] = (unsigned short) l->mapIndex;
					l->rareIndex += 2;
					r = 1;
				}
			if ( r )
			{
				mm->rt_index = 13;
				mm->exp = l->bptable [0x10].XP;
			}
			else
			{
				mm->rt_index = 12;
				mm->exp = l->bptable [0x0F].XP;
			}
			break;
		case 279:
			// Goran family
			if ( mm->skin & 0x02 )
			{
				mm->rt_index = 15;
				mm->exp = l->bptable [0x13].XP;
			}
			else
				if ( mm->skin & 0x01 )
				{
					mm->rt_index = 16;
					mm->exp = l->bptable [0x12].XP;
				}
				else
				{
					mm->rt_index = 14;
					mm->exp = l->bptable [0x11].XP;
				}
			break;
		case 281:
			// Saint Million, Shambertin, and Kondrieu
			r = 0;
			if ( ( ( mm->reserved11 - FLOAT_PRECISION ) < (float) 1.00000 ) &&
				 ( ( mm->reserved11 + FLOAT_PRECISION ) > (float) 1.00000 ) ) // set rare?
				r = 1;
			else
				if ( ( l->rareIndex < 0x20 ) && ( mt_lrand() < kondrieu_rate ) )
				{
					*(unsigned short*) &l->rareData[l->rareIndex] = (unsigned short) l->mapIndex;
					l->rareIndex += 2;
					r = 1;
				}
			if ( r )
				mm->rt_index = 21;
			else
			{
				if ( mm->skin & 0x01 )
					mm->rt_index = 20;
				else
					mm->rt_index = 19;
			}
			mm->exp = l->bptable [0x22].XP;
			break;
		default:
			//debug ("enemy not handled: %u", mm->base);
			break;
		}
		if ( mm->reserved[0] >> 16 ) // Have to do
			l->mapIndex += ( mm->reserved[0] >> 16 );
		l->mapIndex++;
	}
}


void LoadObjectData (LOBBY* l, int unused, const char* filename)
{
	FILE* fp;
	unsigned oldIndex, num_records, ch, ch2;
	char new_file[256];

	if (!l) 
		return;

	memcpy (&new_file[0], filename, strlen (filename) + 1);

	if ( filename [ strlen ( filename ) - 5 ] == 101 )
		new_file [ strlen ( filename ) - 5 ] = 111; // change e to o

	//debug ("Loading object %s... current index: %u", new_file, l->objIndex);

	fp = fopen ( &new_file[0], "rb");
	if (!fp)
		WriteLog ("Could not load object data from %s\n", new_file);
	else
	{
		fseek  ( fp, 0, SEEK_END );
		num_records = ftell ( fp ) / 68;
		fseek  ( fp, 0, SEEK_SET );
		fread  ( &dp[0], 1, 68 * num_records, fp );
		fclose ( fp );
		oldIndex = l->objIndex;
		ch2 = 0;
		for (ch=0;ch<num_records;ch++)
		{
			if ( l->objIndex < 0xB50 )
			{
				memcpy (&l->objData[l->objIndex], &dp[ch2+0x28], 12);
				l->objData[l->objIndex].drop[3] = 0;
				l->objData[l->objIndex].drop[2] = dp[ch2+0x35];
				l->objData[l->objIndex].drop[1] = dp[ch2+0x36];
				l->objData[l->objIndex++].drop[0] = dp[ch2+0x37];
				ch2 += 68;
			}
			else
				break;
		}
		//debug ("Added %u objects, total: %u", l->objIndex - oldIndex, l->objIndex );
	}
};

void LoadMapData (LOBBY* l, int aMob, const char* filename)
{
	FILE* fp;
	unsigned oldIndex, num_records;

	if (!l) 
		return;

	//debug ("Loading map %s... current index: %u", filename, l->mapIndex);

	fp = fopen ( filename, "rb");
	if (!fp)
		WriteLog ("Could not load map data from %s\n", filename);
	else
	{
		fseek  ( fp, 0, SEEK_END );
		num_records = ftell ( fp ) / 72;
		fseek  ( fp, 0, SEEK_SET );
		fread  ( dp, 1, sizeof ( MAP_MONSTER ) * num_records, fp );
		fclose ( fp );
		oldIndex = l->mapIndex;
		ParseMapData ( l, (MAP_MONSTER*) dp, aMob, num_records );
		//debug ("Added %u mids, total: %u", l->mapIndex - oldIndex, l->mapIndex );
	}
};

void initialize_game (CLIENT* client)
{
	LOBBY* l;
	unsigned ch;

	if (!client->lobby)
		return;

	l = (LOBBY*) client->lobby;
	memset (l, 0, sizeof (LOBBY));

	l->difficulty = client->decryptbuf[0x50];
	l->battle = client->decryptbuf[0x51];
	l->challenge = client->decryptbuf[0x52];
	l->episode = client->decryptbuf[0x53];
	l->oneperson = client->decryptbuf[0x54];
	l->start_time = (unsigned) servertime;
	if (l->difficulty > 0x03)
		client->todc = 1;
	else
		if ((l->battle) && (l->challenge))
			client->todc = 1;
		else
			if (l->episode > 0x03)
				client->todc = 1;
			else
				if ((l->oneperson) && ((l->challenge) || (l->battle)))
					client->todc = 1;
	if (!client->todc)
	{
		if (l->battle)
			l->battle = 1;
		if (l->challenge)
			l->challenge = 1;
		if (l->oneperson)
			l->oneperson = 1;
		memcpy (&l->gameName[0], &client->decryptbuf[0x14], 30);
		memcpy (&l->gamePassword[0], &client->decryptbuf[0x30], 32);
		l->in_use = 1;
		l->gameMonster[0] = (unsigned char) mt_lrand() % 256;
		l->gameMonster[1] = (unsigned char) mt_lrand() % 256;
		l->gameMonster[2] = (unsigned char) mt_lrand() % 256;
		l->gameMonster[3] = (unsigned char) mt_lrand() % 16;
		memset (&l->gameMap[0], 0, 128);
		l->playerItemID[0] = 0x10000;
		l->playerItemID[1] = 0x210000;
		l->playerItemID[2] = 0x410000;
		l->playerItemID[3] = 0x610000;
		l->bankItemID[0] = 0x10000;
		l->bankItemID[1] = 0x210000;
		l->bankItemID[2] = 0x410000;
		l->bankItemID[3] = 0x610000;
		l->leader = 0;
		l->sectionID = client->character.sectionID;
		l->itemID = 0x810000;
		l->mapIndex = 0;
		memset (&l->mapData[0], 0, sizeof (l->mapData));
		l->rareIndex = 0;
		for (ch=0;ch<0x20;ch++)
			l->rareData[ch] = 0xFF;
		switch (l->episode)
		{
		case 0x01:
			// Episode 1
			if (!l->oneperson)
			{
				l->bptable = &ep1battle[0x60 * l->difficulty];

				LoadMapData ( l, 0, "map\\map_city00_00e.dat" );
				LoadObjectData ( l, 0, "map\\map_city00_00o.dat" );

				l->gameMap[12]=(unsigned char) mt_lrand() % 5; // Forest 1
				LoadMapData ( l, 0, Forest1_Online_Maps [l->gameMap[12]] );
				LoadObjectData ( l, 0, Forest1_Online_Maps [l->gameMap[12]] );

				l->gameMap[20]=(unsigned char) mt_lrand() % 5; // Forest 2
				LoadMapData ( l, 0, Forest2_Online_Maps [l->gameMap[20]] );
				LoadObjectData ( l, 0, Forest2_Online_Maps [l->gameMap[20]] );

				l->gameMap[24]=(unsigned char) mt_lrand() % 3; // Cave 1
				l->gameMap[28]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Cave1_Online_Maps [( l->gameMap[24] * 2 ) + l->gameMap[28]] );
				LoadObjectData ( l, 0, Cave1_Online_Maps [( l->gameMap[24] * 2 ) + l->gameMap[28]] );

				l->gameMap[32]=(unsigned char) mt_lrand() % 3; // Cave 2
				l->gameMap[36]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Cave2_Online_Maps [( l->gameMap[32] * 2 ) + l->gameMap[36]] );
				LoadObjectData ( l, 0, Cave2_Online_Maps [( l->gameMap[32] * 2 ) + l->gameMap[36]] );

				l->gameMap[40]=(unsigned char) mt_lrand() % 3; // Cave 3
				l->gameMap[44]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Cave3_Online_Maps [( l->gameMap[40] * 2 ) + l->gameMap[44]] );
				LoadObjectData ( l, 0, Cave3_Online_Maps [( l->gameMap[40] * 2 ) + l->gameMap[44]] );

				l->gameMap[48]=(unsigned char) mt_lrand() % 3; // Mine 1
				l->gameMap[52]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Mine1_Online_Maps [( l->gameMap[48] * 2 ) + l->gameMap[52]] );
				LoadObjectData ( l, 0, Mine1_Online_Maps [( l->gameMap[48] * 2 ) + l->gameMap[52]] );

				l->gameMap[56]=(unsigned char) mt_lrand() % 3; // Mine 2
				l->gameMap[60]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Mine2_Online_Maps [( l->gameMap[56] * 2 ) + l->gameMap[60]] );
				LoadObjectData ( l, 0, Mine2_Online_Maps [( l->gameMap[56] * 2 ) + l->gameMap[60]] );

				l->gameMap[64]=(unsigned char) mt_lrand() % 3; // Ruins 1
				l->gameMap[68]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Ruins1_Online_Maps [( l->gameMap[64] * 2 ) + l->gameMap[68]] );
				LoadObjectData ( l, 0, Ruins1_Online_Maps [( l->gameMap[64] * 2 ) + l->gameMap[68]] );

				l->gameMap[72]=(unsigned char) mt_lrand() % 3; // Ruins 2
				l->gameMap[76]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Ruins2_Online_Maps [( l->gameMap[72] * 2 ) + l->gameMap[76]] );
				LoadObjectData ( l, 0, Ruins2_Online_Maps [( l->gameMap[72] * 2 ) + l->gameMap[76]] );

				l->gameMap[80]=(unsigned char) mt_lrand() % 3; // Ruins 3
				l->gameMap[84]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Ruins3_Online_Maps [( l->gameMap[80] * 2 ) + l->gameMap[84]] );
				LoadObjectData ( l, 0, Ruins3_Online_Maps [( l->gameMap[80] * 2 ) + l->gameMap[84]] );
			}
			else
			{
				l->bptable = &ep1battle_off[0x60 * l->difficulty];

				LoadMapData ( l, 0, "map\\map_city00_00e_s.dat");
				LoadObjectData ( l, 0, "map\\map_city00_00o_s.dat");

				l->gameMap[12]=(unsigned char) mt_lrand() % 3; // Forest 1
				LoadMapData ( l, 0, Forest1_Offline_Maps [l->gameMap[12]] );
				LoadObjectData ( l, 0, Forest1_Offline_Objects [l->gameMap[12]] );

				l->gameMap[20]=(unsigned char) mt_lrand() % 3; // Forest 2
				LoadMapData ( l, 0, Forest2_Offline_Maps [l->gameMap[20]] );
				LoadObjectData ( l, 0, Forest2_Offline_Objects [l->gameMap[20]] );

				l->gameMap[24]=(unsigned char) mt_lrand() % 3; // Cave 1
				LoadMapData ( l, 0, Cave1_Offline_Maps [l->gameMap[24]]);
				LoadObjectData ( l, 0, Cave1_Offline_Objects [l->gameMap[24]]);

				l->gameMap[32]=(unsigned char) mt_lrand() % 3; // Cave 2
				LoadMapData ( l, 0, Cave2_Offline_Maps [l->gameMap[32]]);
				LoadObjectData ( l, 0, Cave2_Offline_Objects [l->gameMap[32]]);

				l->gameMap[40]=(unsigned char) mt_lrand() % 3; // Cave 3
				LoadMapData ( l, 0, Cave3_Offline_Maps [l->gameMap[40]]);
				LoadObjectData ( l, 0, Cave3_Offline_Objects [l->gameMap[40]]);

				l->gameMap[48]=(unsigned char) mt_lrand() % 3; // Mine 1
				l->gameMap[52]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Mine1_Online_Maps [( l->gameMap[48] * 2 ) + l->gameMap[52]] );
				LoadObjectData ( l, 0, Mine1_Online_Maps [( l->gameMap[48] * 2 ) + l->gameMap[52]] );

				l->gameMap[56]=(unsigned char) mt_lrand() % 3; // Mine 2
				l->gameMap[60]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Mine2_Online_Maps [( l->gameMap[56] * 2 ) + l->gameMap[60]] );
				LoadObjectData ( l, 0, Mine2_Online_Maps [( l->gameMap[56] * 2 ) + l->gameMap[60]] );

				l->gameMap[64]=(unsigned char) mt_lrand() % 3; // Ruins 1
				l->gameMap[68]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Ruins1_Online_Maps [( l->gameMap[64] * 2 ) + l->gameMap[68]] );
				LoadObjectData ( l, 0, Ruins1_Online_Maps [( l->gameMap[64] * 2 ) + l->gameMap[68]] );

				l->gameMap[72]=(unsigned char) mt_lrand() % 3; // Ruins 2
				l->gameMap[76]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Ruins2_Online_Maps [( l->gameMap[72] * 2 ) + l->gameMap[76]] );
				LoadObjectData ( l, 0, Ruins2_Online_Maps [( l->gameMap[72] * 2 ) + l->gameMap[76]] );

				l->gameMap[80]=(unsigned char) mt_lrand() % 3; // Ruins 3
				l->gameMap[84]=(unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Ruins3_Online_Maps [( l->gameMap[80] * 2 ) + l->gameMap[84]] );
				LoadObjectData ( l, 0, Ruins3_Online_Maps [( l->gameMap[80] * 2 ) + l->gameMap[84]] );
			}
			LoadMapData ( l, 0, "map\\map_boss01e.dat" );
			LoadObjectData ( l, 0, "map\\map_boss01o.dat");
			LoadMapData ( l, 0, "map\\map_boss02e.dat" );
			LoadObjectData ( l, 0, "map\\map_boss02o.dat");
			LoadMapData ( l, 0, "map\\map_boss03e.dat" );
			LoadObjectData ( l, 0, "map\\map_boss03o.dat");
			LoadMapData ( l, 0, "map\\map_boss04e.dat" );
			if ( l->oneperson )
				LoadObjectData ( l, 0, "map\\map_boss04_offo.dat");
			else
				LoadObjectData ( l, 0, "map\\map_boss04o.dat");
			break;
		case 0x02:
			// Episode 2
			if (!l->oneperson)
			{
				l->bptable = &ep2battle[0x60 * l->difficulty];
				LoadMapData ( l, 0, "map\\map_labo00_00e.dat");
				LoadObjectData ( l, 0, "map\\map_labo00_00o.dat");
				
				l->gameMap[8]  = (unsigned char) mt_lrand() % 2; // Temple 1
				l->gameMap[12] = 0x00;
				LoadMapData ( l, 0, Temple1_Online_Maps [l->gameMap[8]] );
				LoadObjectData ( l, 0, Temple1_Online_Maps [l->gameMap[8]] );

				l->gameMap[16] = (unsigned char) mt_lrand() % 2; // Temple 2
				l->gameMap[20] = 0x00;
				LoadMapData ( l, 0, Temple2_Online_Maps [l->gameMap[16]] );
				LoadObjectData ( l, 0, Temple2_Online_Maps [l->gameMap[16]] );

				l->gameMap[24] = (unsigned char) mt_lrand() % 2; // Spaceship 1
				l->gameMap[28] = 0x00;
				LoadMapData ( l, 0, Spaceship1_Online_Maps [l->gameMap[24]] );
				LoadObjectData ( l, 0, Spaceship1_Online_Maps [l->gameMap[24]] );

				l->gameMap[32] = (unsigned char) mt_lrand() % 2; // Spaceship 2
				l->gameMap[36] = 0x00; 
				LoadMapData ( l, 0, Spaceship2_Online_Maps [l->gameMap[32]] );
				LoadObjectData ( l, 0, Spaceship2_Online_Maps [l->gameMap[32]] );

				l->gameMap[40] = 0x00;
				l->gameMap[44] = (unsigned char) mt_lrand() % 3; // Jungle 1
				LoadMapData ( l, 0, Jungle1_Online_Maps [l->gameMap[44]] );
				LoadObjectData ( l, 0, Jungle1_Online_Maps [l->gameMap[44]] );

				l->gameMap[48] = 0x00;
				l->gameMap[52] = (unsigned char) mt_lrand() % 3; // Jungle 2
				LoadMapData ( l, 0, Jungle2_Online_Maps [l->gameMap[52]] );
				LoadObjectData ( l, 0, Jungle2_Online_Maps [l->gameMap[52]] );

				l->gameMap[56] = 0x00; 
				l->gameMap[60] = (unsigned char) mt_lrand() % 3; // Jungle 3
				LoadMapData ( l, 0, Jungle3_Online_Maps [l->gameMap[60]] );
				LoadObjectData ( l, 0, Jungle3_Online_Maps [l->gameMap[60]] );

				l->gameMap[64] = (unsigned char) mt_lrand() % 2; // Jungle 4
				l->gameMap[68] = (unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Jungle4_Online_Maps [(l->gameMap[64] * 2 ) + l->gameMap[68]] );
				LoadObjectData ( l, 0, Jungle4_Online_Maps [(l->gameMap[64] * 2 ) + l->gameMap[68]] );

				l->gameMap[72] = 0x00;
				l->gameMap[76] = (unsigned char) mt_lrand() % 3; // Jungle 5
				LoadMapData ( l, 0, Jungle5_Online_Maps [l->gameMap[76]] );
				LoadObjectData ( l, 0, Jungle5_Online_Maps [l->gameMap[76]] );

				l->gameMap[80] = (unsigned char) mt_lrand() % 2; // Seabed 1
				l->gameMap[84] = (unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Seabed1_Online_Maps [(l->gameMap[80] * 2 ) + l->gameMap[84]] );
				LoadObjectData ( l, 0, Seabed1_Online_Maps [(l->gameMap[80] * 2 ) + l->gameMap[84]] );

				l->gameMap[88] = (unsigned char) mt_lrand() % 2; // Seabed 2
				l->gameMap[92] = (unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Seabed2_Online_Maps [(l->gameMap[88] * 2 ) + l->gameMap[92]] );
				LoadObjectData ( l, 0, Seabed2_Online_Maps [(l->gameMap[88] * 2 ) + l->gameMap[92]] );
			}
			else
			{
				l->bptable = &ep2battle_off[0x60 * l->difficulty];
				LoadMapData ( l, 0, "map\\map_labo00_00e_s.dat");
				LoadObjectData ( l, 0, "map\\map_labo00_00o_s.dat");
				
				l->gameMap[8]  = (unsigned char) mt_lrand() % 2; // Temple 1
				l->gameMap[12] = 0x00;
				LoadMapData ( l, 0, Temple1_Offline_Maps [l->gameMap[8]] );
				LoadObjectData ( l, 0, Temple1_Offline_Maps [l->gameMap[8]] );

				l->gameMap[16] = (unsigned char) mt_lrand() % 2; // Temple 2
				l->gameMap[20] = 0x00;
				LoadMapData ( l, 0, Temple2_Offline_Maps [l->gameMap[16]] );
				LoadObjectData ( l, 0, Temple2_Offline_Maps [l->gameMap[16]] );

				l->gameMap[24] = (unsigned char) mt_lrand() % 2; // Spaceship 1
				l->gameMap[28] = 0x00;
				LoadMapData ( l, 0, Spaceship1_Offline_Maps [l->gameMap[24]] );
				LoadObjectData ( l, 0, Spaceship1_Offline_Maps [l->gameMap[24]] );

				l->gameMap[32] = (unsigned char) mt_lrand() % 2; // Spaceship 2
				l->gameMap[36] = 0x00; 
				LoadMapData ( l, 0, Spaceship2_Offline_Maps [l->gameMap[32]] );
				LoadObjectData ( l, 0, Spaceship2_Offline_Maps [l->gameMap[32]] );

				l->gameMap[40] = 0x00;
				l->gameMap[44] = (unsigned char) mt_lrand() % 3; // Jungle 1
				LoadMapData ( l, 0, Jungle1_Offline_Maps [l->gameMap[44]] );
				LoadObjectData ( l, 0, Jungle1_Offline_Maps [l->gameMap[44]] );

				l->gameMap[48] = 0x00;
				l->gameMap[52] = (unsigned char) mt_lrand() % 3; // Jungle 2
				LoadMapData ( l, 0, Jungle2_Offline_Maps [l->gameMap[52]] );
				LoadObjectData ( l, 0, Jungle2_Offline_Maps [l->gameMap[52]] );

				l->gameMap[56] = 0x00; 
				l->gameMap[60] = (unsigned char) mt_lrand() % 3; // Jungle 3
				LoadMapData ( l, 0, Jungle3_Offline_Maps [l->gameMap[60]] );
				LoadObjectData ( l, 0, Jungle3_Offline_Maps [l->gameMap[60]] );

				l->gameMap[64] = (unsigned char) mt_lrand() % 2; // Jungle 4
				l->gameMap[68] = (unsigned char) mt_lrand() % 2;
				LoadMapData ( l, 0, Jungle4_Offline_Maps [(l->gameMap[64] * 2 ) + l->gameMap[68]] );
				LoadObjectData ( l, 0, Jungle4_Offline_Maps [(l->gameMap[64] * 2 ) + l->gameMap[68]] );

				l->gameMap[72] = 0x00;
				l->gameMap[76] = (unsigned char) mt_lrand() % 3; // Jungle 5
				LoadMapData ( l, 0, Jungle5_Offline_Maps [l->gameMap[76]] );
				LoadObjectData ( l, 0, Jungle5_Offline_Maps [l->gameMap[76]] );

				l->gameMap[80] = (unsigned char) mt_lrand() % 2; // Seabed 1
				LoadMapData ( l, 0, Seabed1_Offline_Maps [l->gameMap[80]] );
				LoadObjectData ( l, 0, Seabed1_Offline_Maps [l->gameMap[80]] );

				l->gameMap[88] = (unsigned char) mt_lrand() % 2; // Seabed 2
				LoadMapData ( l, 0, Seabed2_Offline_Maps [l->gameMap[88]] );
				LoadObjectData ( l, 0, Seabed2_Offline_Maps [l->gameMap[88]] );
			}
			LoadMapData ( l, 0, "map\\map_boss05e.dat");
			LoadMapData ( l, 0, "map\\map_boss06e.dat");
			LoadMapData ( l, 0, "map\\map_boss07e.dat");
			LoadMapData ( l, 0, "map\\map_boss08e.dat");
			if ( l->oneperson )
			{
				LoadObjectData ( l, 0, "map\\map_boss05_offo.dat");
				LoadObjectData ( l, 0, "map\\map_boss06_offo.dat");
				LoadObjectData ( l, 0, "map\\map_boss07_offo.dat");
				LoadObjectData ( l, 0, "map\\map_boss08_offo.dat");
			}
			else
			{
				LoadObjectData ( l, 0, "map\\map_boss05o.dat");
				LoadObjectData ( l, 0, "map\\map_boss06o.dat");
				LoadObjectData ( l, 0, "map\\map_boss07o.dat");
				LoadObjectData ( l, 0, "map\\map_boss08o.dat");
			}
			break;
		case 0x03:
			// Episode 4
			if (!l->oneperson)
			{
				l->bptable = &ep4battle[0x60 * l->difficulty];
				LoadMapData (l, 0, "map\\map_city02_00_00e.dat");
				LoadObjectData (l, 0, "map\\map_city02_00_00o.dat");
			}
			else
			{
				l->bptable = &ep4battle_off[0x60 * l->difficulty];
				LoadMapData (l, 0, "map\\map_city02_00_00e_s.dat");
				LoadObjectData (l, 0, "map\\map_city02_00_00o_s.dat");
			}

			l->gameMap[12] = (unsigned char) mt_lrand() % 3; // Crater East
			LoadMapData ( l, 0, Crater_East_Online_Maps [l->gameMap[12]] );
			LoadObjectData ( l, 0, Crater_East_Online_Maps [l->gameMap[12]] );

			l->gameMap[20] = (unsigned char) mt_lrand() % 3; // Crater West
			LoadMapData ( l, 0, Crater_West_Online_Maps [l->gameMap[20]] );
			LoadObjectData ( l, 0, Crater_West_Online_Maps [l->gameMap[20]] );

			l->gameMap[28] = (unsigned char) mt_lrand() % 3; // Crater South
			LoadMapData ( l, 0, Crater_South_Online_Maps [l->gameMap[28]] );
			LoadObjectData ( l, 0, Crater_South_Online_Maps [l->gameMap[28]] );

			l->gameMap[36] = (unsigned char) mt_lrand() % 3; // Crater North
			LoadMapData ( l, 0, Crater_North_Online_Maps [l->gameMap[36]] );
			LoadObjectData ( l, 0, Crater_North_Online_Maps [l->gameMap[36]] );

			l->gameMap[44] = (unsigned char) mt_lrand() % 3; // Crater Interior
			LoadMapData ( l, 0, Crater_Interior_Online_Maps [l->gameMap[44]] );
			LoadObjectData ( l, 0, Crater_Interior_Online_Maps [l->gameMap[44]] );

			l->gameMap[48] = (unsigned char) mt_lrand() % 3; // Desert 1
			LoadMapData ( l, 1, Desert1_Online_Maps [l->gameMap[48]] );
			LoadObjectData ( l, 1, Desert1_Online_Maps [l->gameMap[48]] );

			l->gameMap[60] = (unsigned char) mt_lrand() % 3; // Desert 2
			LoadMapData ( l, 1, Desert2_Online_Maps [l->gameMap[60]] );
			LoadObjectData ( l, 1, Desert2_Online_Maps [l->gameMap[60]] );

			l->gameMap[64] = (unsigned char) mt_lrand() % 3; // Desert 3
			LoadMapData ( l, 1, Desert3_Online_Maps [l->gameMap[64]] );
			LoadObjectData ( l, 1, Desert3_Online_Maps [l->gameMap[64]] );

			LoadMapData (l, 0, "map\\map_boss09_00_00e.dat");
			LoadObjectData (l, 0, "map\\map_boss09_00_00o.dat");
			//LoadMapData (l, "map\\map_test01_00_00e.dat");
			break;
		default:
			break;
		}
	}
	else
		Send1A ("Bad game arguments supplied.", client);
}

void Send64 (CLIENT* client)
{
	LOBBY* l;
	unsigned Offset;
	unsigned ch;

	if (!client->lobby)
		return;
	l = (LOBBY*) client->lobby;

	for (ch=0;ch<4;ch++)
	{
		if (!l->slot_use[ch])
		{
			l->slot_use[ch] = 1; // Slot now in use
			l->client[ch] = client;
			// lobbyNum should be set before joining the game
			client->clientID = ch;
			l->gamePlayerCount++;
			l->gamePlayerID[ch] = l->gamePlayerCount;
			break;
		}
	}
	l->lobbyCount = 0;
	for (ch=0;ch<4;ch++)
	{
		if ((l->slot_use[ch]) && (l->client[ch]))
			l->lobbyCount++;
	}
	memset (&PacketData[0], 0, 0x1A8);
	PacketData[0x00] = 0xA8;
	PacketData[0x01] = 0x01;
	PacketData[0x02] = 0x64;
	PacketData[0x04] = (unsigned char) l->lobbyCount;
	memcpy (&PacketData[0x08], &l->gameMap[0], 128 );
	Offset = 0x88;
	for (ch=0;ch<4;ch++)
	{
		if ((l->slot_use[ch]) && (l->client[ch]))
		{
			PacketData[Offset+2] = 0x01;
			Offset += 0x04;
			*(unsigned *) &PacketData[Offset] = l->client[ch]->guildcard;
			Offset += 0x10;
			PacketData[Offset] = l->client[ch]->clientID;
			Offset += 0x0C;
			memcpy (&PacketData[Offset], &l->client[ch]->character.name[0], 24);
			Offset += 0x20;
			PacketData[Offset] = 0x02;
			Offset += 0x04;
			if ((l->client[ch]->guildcard == client->guildcard) && (l->lobbyCount > 1))
			{
				memset (&PacketData2[0], 0, 1316);
				PacketData2[0x00] = 0x34;
				PacketData2[0x01] = 0x05;
				PacketData2[0x02] = 0x65;
				PacketData2[0x04] = 0x01;
				PacketData2[0x08] = client->clientID;
				PacketData2[0x09] = l->leader;
				PacketData2[0x0A] = 0x01; // ??
				PacketData2[0x0B] = 0xFF; // ??
				PacketData2[0x0C] = 0x01; // ??
				PacketData2[0x0E] = 0x01; // ??
				PacketData2[0x16] = 0x01;
				*(unsigned *) &PacketData2[0x18] = client->guildcard;
				PacketData2[0x30] = client->clientID;
				memcpy (&PacketData2[0x34], &client->character.name[0], 24);
				PacketData2[0x54] = 0x02; // ??
				memcpy (&PacketData2[0x58], &client->character.inventoryUse, 0x4DC);
				// Prevent crashing with NPC skins...
				if (client->character.skinFlag)
					memset (&PacketData2[0x58+0x3A8], 0, 10);
				SendToLobby ( client->lobby, 4, &PacketData2[0x00], 1332, client->guildcard );
			}
		}
	}

	if (l->lobbyCount < 4)
		PacketData[0x194] = 0x02;
	if (l->lobbyCount < 3)
		PacketData[0x150] = 0x02;
	if (l->lobbyCount < 2)
		PacketData[0x10C] = 0x02;

	// Most of the 0x64 packet has been generated... now for the important stuff. =p
	// Leader ID @ 0x199
	// Difficulty @ 0x19B
	// Event @ 0x19D
	// Section ID of Leader @ 0x19E
	// Game Monster @ 0x1A0 (4 bytes)
	// Episode @ 0x1A4
	// 0x01 @ 0x1A5
	// One-person @ 0x1A6

	PacketData[0x198] = client->clientID;
	PacketData[0x199] = l->leader;
	PacketData[0x19B] = l->difficulty;
	PacketData[0x19C] = l->battle;
	if ((shipEvent < 7) && (shipEvent != 0x02))
		PacketData[0x19D] = shipEvent;
	else
		PacketData[0x19D] = 0;
	PacketData[0x19E] = l->sectionID;
	PacketData[0x19F] = l->challenge;
	*(unsigned *) &PacketData[0x1A0] = *(unsigned *) &l->gameMonster;
	PacketData[0x1A4] = l->episode;
	PacketData[0x1A5] = 0x01; // ??
	PacketData[0x1A6] = l->oneperson;
	cipher_ptr = &client->server_cipher;
	encryptcopy (client, &PacketData[0], 0x1A8);

	/* Let's send the team data... */

	SendToLobby ( client->lobby, 4, MakePacketEA15 ( client ), 2152, client->guildcard );

	client->bursting = 1;
}

void Send67 (CLIENT* client, unsigned char preferred)
{
	BLOCK* b;
	CLIENT* lClient;
	LOBBY* l;
	unsigned Offset = 0, Offset2 = 0;
	unsigned ch, ch2;

	if (!client->lobbyOK)
		return;

	client->lobbyOK = 0;

	ch = 0;
	
	b = blocks[client->block - 1];
	if ((preferred != 0xFF) && (preferred < 0x0F))
	{
		if (b->lobbies[preferred].lobbyCount >= 12)
			preferred = 0x00;
		ch = preferred;
	}

	for (ch=ch;ch<15;ch++)
	{
		l = &b->lobbies[ch];
		if (l->lobbyCount < 12)
		{
			for (ch2=0;ch2<12;ch2++)
				if (l->slot_use[ch2] == 0)
				{
					l->slot_use[ch2] = 1;
					l->client[ch2] = client;
					l->arrow_color[ch2] = 0;
					client->lobbyNum = ch + 1;
					client->lobby = (void*) &b->lobbies[ch];
					client->clientID = ch2;
					break;
				}
				// Send68 here with joining client (use ch2 for clientid)
				l->lobbyCount = 0;
				for (ch2=0;ch2<12;ch2++)
				{
					if ((l->slot_use[ch2]) && (l->client[ch2]))
						l->lobbyCount++;
				}

				memset (&PacketData[0x00], 0, 0x10);
				PacketData[0x04] = l->lobbyCount;
				PacketData[0x08] = client->clientID;
				PacketData[0x0B] = ch;
				PacketData[0x0C] = client->block;
				PacketData[0x0E] = shipEvent;
				Offset = 0x16;
				for (ch2=0;ch2<12;ch2++)
				{
					if ((l->slot_use[ch2]) && (l->client[ch2]))
					{
						memset (&PacketData[Offset], 0, 1316);
						Offset2 = Offset;
						PacketData[Offset++] = 0x01;
						PacketData[Offset++] = 0x00;
						lClient = l->client[ch2];
						*(unsigned *) &PacketData[Offset] = lClient->guildcard;
						Offset += 24;
						*(unsigned *) &PacketData[Offset] = ch2;
						Offset += 4;
						memcpy (&PacketData[Offset], &lClient->character.name[0], 24);
						Offset += 32;
						PacketData[Offset++] = 0x02;
						Offset += 3;
						memcpy (&PacketData[Offset], &lClient->character.inventoryUse, 1246);
						// Prevent crashing with NPCs
						if (lClient->character.skinFlag)
							memset (&PacketData[Offset+0x3A8], 0, 10);
						Offset += 1246;
						if (lClient->isgm == 1)
							*(unsigned *) &PacketData[Offset2 + 0x3CA] = globalName;
						else
							if (isLocalGM(lClient->guildcard))
								*(unsigned *) &PacketData[Offset2 + 0x3CA] = localName;
							else
								*(unsigned *) &PacketData[Offset2 + 0x3CA] = normalName;
						if ((lClient->guildcard == client->guildcard) && (l->lobbyCount > 1))
						{
							memcpy (&PacketData2[0x00], &PacketData[0], 0x16 );
							PacketData2[0x00] = 0x34;
							PacketData2[0x01] = 0x05;
							PacketData2[0x02] = 0x68;
							PacketData2[0x04] = 0x01;
							memcpy (&PacketData2[0x16], &PacketData[Offset2], 1316 );
							SendToLobby ( client->lobby, 12, &PacketData2[0x00], 1332, client->guildcard );
						}
					}
				}
				*(unsigned short*) &PacketData[0] = (unsigned short) Offset;
				PacketData[2] = 0x67;
				break;
		}
	}

	if (Offset > 0)
	{
		cipher_ptr = &client->server_cipher;
		encryptcopy (client, &PacketData[0], Offset);
	}

	// Quest data

	memset (&client->encryptbuf[0x00], 0, 8);
	client->encryptbuf[0] = 0x10;
	client->encryptbuf[1] = 0x02;
	client->encryptbuf[2] = 0x60;
	client->encryptbuf[8] = 0x6F;
	client->encryptbuf[9] = 0x84;
	memcpy (&client->encryptbuf[0x0C], &client->character.quest_data1[0], 0x210 );
	memset (&client->encryptbuf[0x20C], 0, 4);
	encryptcopy (client, &client->encryptbuf[0x00], 0x210);

	/* Let's send the team data... */

	SendToLobby ( client->lobby, 12, MakePacketEA15 ( client ), 2152, client->guildcard );

	client->bursting = 1;
}

void Send95 (CLIENT* client)
{
	client->lobbyOK = 1;
	memset (&client->encryptbuf[0x00], 0, 8);
	client->encryptbuf[0x00] = 0x08;
	client->encryptbuf[0x02] = 0x95;
	cipher_ptr = &client->server_cipher;
	encryptcopy (client, &client->encryptbuf[0], 8);
	// Restore permanent character...
	if (client->character_backup)
	{
		if (client->mode)
		{
			memcpy (&client->character, client->character_backup, sizeof (client->character));
			client->mode = 0;
		}
		free (client->character_backup);
		client->character_backup = NULL;
	}
}

int qflag (unsigned char* flag_data, unsigned flag, unsigned difficulty)
{
	if (flag_data[(difficulty * 0x80) + ( flag >> 3 )] & (1 << (7 - ( flag & 0x07 ))))
		return 1;
	else
		return 0;
}

int qflag_ep1solo(unsigned char* flag_data, unsigned difficulty)
{
	int i;
	unsigned int quest_flag;

	for(i=1;i<=25;i++)
	{
		quest_flag = 0x63 + (i << 1);
		if(!qflag (flag_data, quest_flag, difficulty)) return 0;
	}
	return 1;
}

void SendA2 (unsigned char episode, unsigned char solo, unsigned char category, unsigned char gov, CLIENT* client)
{
	QUEST_MENU* qm = 0;
	QUEST* q;
	unsigned char qc = 0;
	unsigned short Offset;
	unsigned ch,ch2,ch3,show_quest,quest_flag;
	unsigned char quest_count;
	char quest_num[16];
	int qn, tier1, ep1solo;
	LOBBY* l;
	unsigned char diff;

	if (!client->lobby)
		return;

	l = (LOBBY *) client->lobby;

	memset (&PacketData[0], 0, 0x2510);

	diff = l->difficulty;

	if ( l->battle )
	{
		qm = &quest_menus[9];
		qc = 10;
	}
	else
		if ( l->challenge )
		{
		}
		else
		{
			switch ( episode )
			{
			case 0x01:
				if ( gov )
				{
					qm = &quest_menus[6];
					qc = 7;
				}
				else
				{
					if ( solo )
					{
						qm = &quest_menus[3];
						qc = 4;
					}
					else
					{
						qm = &quest_menus[0];
						qc = 1;
					}
				}
				break;
			case 0x02:
				if ( gov )
				{
					qm = &quest_menus[7];
					qc = 8;
				}
				else
				{
					if ( solo )
					{
						qm = &quest_menus[4];
						qc = 5;
					}
					else
					{
						qm = &quest_menus[1];
						qc = 2;
					}
				}
				break;
			case 0x03:
				if ( gov )
				{
					qm = &quest_menus[8];
					qc = 9;
				}
				else
				{
					if ( solo )
					{
						qm = &quest_menus[5];
						qc = 6;
					}
					else
					{
						qm = &quest_menus[2];
						qc = 3;
					}
				}
				break;
			}
		}

	if ((qm) && (category == 0))
	{
		PacketData[0x02] = 0xA2;
		PacketData[0x04] = qm->num_categories;
		Offset = 0x08;
		for (ch=0;ch<qm->num_categories;ch++)
		{
			PacketData[Offset+0x07]	= 0x0F;
			PacketData[Offset]		= qc;
			PacketData[Offset+2]	= gov;
			PacketData[Offset+4]	= ch + 1;
			memcpy (&PacketData[Offset+0x08], &qm->c_names[ch][0], 0x40);
			memcpy (&PacketData[Offset+0x48], &qm->c_desc[ch][0], 0xF0);
			Offset += 0x13C;
		}
	}
	else
	{
		ch2 = 0;
		PacketData[0x02] = 0xA2;
		category--;
		quest_count = 0;
		Offset = 0x08;
		ep1solo = qflag_ep1solo(&client->character.quest_data1[0], diff);
		for (ch=0;ch<qm->quest_counts[category];ch++)
		{
			q = &quests[qm->quest_indexes[category][ch]];
			show_quest = 0;
			if ((solo) && (episode == 0x01))
			{
				memcpy (&quest_num[0], &q->ql[0]->qdata[49], 3);
				quest_num[4] = 0;
				qn = atoi (&quest_num[0]);
				if ((ep1solo) || (qn > 26))
					show_quest = 1;
				if (!show_quest)
				{
					quest_flag = 0x63 + (qn << 1);
					if (qflag(&client->character.quest_data1[0], quest_flag, diff))
						show_quest = 2; // Sets a variance if they've cleared the quest...
					else
					{
						tier1 = 0;
						if ( (qflag(&client->character.quest_data1[0],0x65,diff)) && // Cleared first tier
							 (qflag(&client->character.quest_data1[0],0x67,diff)) &&
							 (qflag(&client->character.quest_data1[0],0x6B,diff)) )
							 tier1 = 1;
						if (qflag(&client->character.quest_data1[0], quest_flag, diff) == 0)
						{ // When the quest hasn't been completed...
							// Forest quests
							switch (qn)
							{
							case 4: // Battle Training
							case 2: // Claiming a Stake
							case 1: // Magnitude of Metal
								show_quest = 1;
								break;
							case 5: // Journalistic Pursuit
							case 6: // The Fake In Yellow
							case 7: // Native Research
							case 9: // Gran Squall
								if (tier1)
									show_quest = 1;
								break;
							case 8: // Forest of Sorrow
								if (qflag(&client->character.quest_data1[0],0x71,diff)) // Cleared Native Research
									show_quest = 1;
								break;
							case 26: // Central Dome Fire Swirl
								if (qflag(&client->character.quest_data1[0],0x73,diff)) // Cleared Forest of Sorrow
									show_quest = 1;
								break;
							}

							if ((tier1) && (qflag(&client->character.quest_data1[0],0x1F9,diff)))
							{
								// Cave quests (shown after Dragon is defeated)
								switch (qn)
								{
								case 03: // The Value of Money
								case 11: // The Lost Bride
								case 14: // Secret Delivery
								case 17: // Grave's Butler
								case 10: // Addicting Food
									show_quest = 1; // Always shown if first tier was cleared
									break;
								case 12: // Waterfall Tears
								case 15: // Soul of a Blacksmith
									if ( (qflag(&client->character.quest_data1[0],0x77,diff)) && // Cleared Addicting Food
										 (qflag(&client->character.quest_data1[0],0x79,diff)) && // Cleared The Lost Bride
										 (qflag(&client->character.quest_data1[0],0x7F,diff)) && // Cleared Secret Delivery
										 (qflag(&client->character.quest_data1[0],0x85,diff)) )  // Cleared Grave's Butler
										 show_quest = 1;
									break;
								case 13: // Black Paper
									if (qflag(&client->character.quest_data1[0],0x7B,diff)) // Cleared Waterfall Tears
										show_quest = 1;
									break;
								}
							}

							if ((tier1) && (qflag(&client->character.quest_data1[0],0x1FF,diff)))
							{
								// Mine quests (shown after De Rol Le is defeated)
								switch (qn)
								{
								case 16: // Letter from Lionel
								case 18: // Knowing One's Heart
								case 20: // Dr. Osto's Research
									show_quest = 1; // Always shown if first tier was cleared
									break;
								case 21: // The Unsealed Door
									if ( (qflag(&client->character.quest_data1[0],0x8B,diff)) && // Cleared Dr. Osto's Research
										 (qflag(&client->character.quest_data1[0],0x7F,diff)) )  // Cleared Secret Delivery
										show_quest = 1;
									break;
								}
							}

							if ((tier1) && (qflag(&client->character.quest_data1[0],0x207,diff)))
							{
								// Ruins quests (shown after Vol Opt is defeated)
								switch (qn)
								{
								case 19: // The Retired Hunter
								case 24: // Seek My Master
									show_quest = 1;  // Always shown if first tier was cleared
									break;
								case 25: // From the Depths
								case 22: // Soul of Steel
									if (qflag(&client->character.quest_data1[0],0x91,diff)) // Cleared Doc's Secret Plan
										show_quest = 1;
									break;
								case 23: // Doc's Secret Plan
									if (qflag(&client->character.quest_data1[0],0x7F,diff)) // Cleared Secret Delivery
										show_quest = 1;
									break;
								}
							}
						}
					}
				}
			}
			else
			{
				show_quest = 1;
				if ((ch) && (gov))
				{
					// Check party's qualification for quests...
					switch (episode)
					{
					case 0x01:
						quest_flag = (0x1F3 + (ch << 1));
						for (ch2=0x1F5;ch2<=quest_flag;ch2+=2)
							for (ch3=0;ch3<4;ch3++)
								if ((l->client[ch3]) && (!qflag(&l->client[ch3]->character.quest_data1[0], ch2, diff)))
								show_quest = 0;
						break;
					case 0x02:
						quest_flag = (0x211 + (ch << 1));
						for (ch2=0x213;ch2<=quest_flag;ch2+=2)
							for (ch3=0;ch3<4;ch3++)
								if ((l->client[ch3]) && (!qflag(&l->client[ch3]->character.quest_data1[0], ch2, diff)))
								show_quest = 0;
						break;
					case 0x03:
						quest_flag = (0x2BC + ch);
						for (ch2=0x2BD;ch2<=quest_flag;ch2++)
							for (ch3=0;ch3<4;ch3++)
								if ((l->client[ch3]) && (!qflag(&l->client[ch3]->character.quest_data1[0], ch2, diff)))
								show_quest = 0;
						break;
					}
				}
			}
			if (show_quest)
			{
				PacketData[Offset+0x07] = 0x0F;
				PacketData[Offset]      = qc;
				PacketData[Offset+1]	= 0x01;
				PacketData[Offset+2]    = gov;
				PacketData[Offset+3]    = ((unsigned char) qm->quest_indexes[category][ch]) + 1;
				PacketData[Offset+4]    = category;
				if ((client->character.lang < 10) && (q->ql[client->character.lang]))
				  {
					memcpy (&PacketData[Offset+0x08], &q->ql[client->character.lang]->qname[0], 0x40);
					memcpy (&PacketData[Offset+0x48], &q->ql[client->character.lang]->qsummary[0], 0xF0);
				  }
				else
				  {
					memcpy (&PacketData[Offset+0x08], &q->ql[0]->qname[0], 0x40);
					memcpy (&PacketData[Offset+0x48], &q->ql[0]->qsummary[0], 0xF0);
				  }

				if ((solo) && (episode == 0x01))
				{
					if (qn <= 26)
					{
						*(unsigned short*) &PacketData[Offset+0x128] = ep1_unlocks[(qn-1)*2];
						*(unsigned short*) &PacketData[Offset+0x12C] = ep1_unlocks[((qn-1)*2)+1];
						*(int*) &PacketData[Offset+0x130] = qn;
						if ( show_quest == 2 )
							PacketData[Offset + 0x138] = 1;
					}
				}
				Offset += 0x13C;
				quest_count++;
			}
		}
		PacketData[0x04] = quest_count;
	}
	*(unsigned short*) &PacketData[0] = (unsigned short) Offset;
	cipher_ptr = &client->server_cipher;
	encryptcopy (client, &PacketData[0], Offset);
}

void SendA0 (CLIENT* client)
{
	cipher_ptr = &client->server_cipher;
	encryptcopy (client, &PacketA0Data[0], *(unsigned short *) &PacketA0Data[0]);
}


void Send07 (CLIENT* client)
{
	cipher_ptr = &client->server_cipher;
	encryptcopy (client, &Packet07Data[0], *(unsigned short *) &Packet07Data[0]);
}


void SendB0 (const char *mes, CLIENT* client)
{
	unsigned short xB0_Len;

	memcpy (&PacketData[0], &PacketB0[0], sizeof (PacketB0));
	xB0_Len = sizeof (PacketB0);

	while (*mes != 0x00)
	{
		PacketData[xB0_Len++] = *(mes++);
		PacketData[xB0_Len++] = 0x00;
	}

	PacketData[xB0_Len++] = 0x00;
	PacketData[xB0_Len++] = 0x00;

	while (xB0_Len % 8)
		PacketData[xB0_Len++] = 0x00;
	*(unsigned short*) &PacketData[0] = xB0_Len;
	cipher_ptr = &client->server_cipher;
	encryptcopy (client, &PacketData[0], xB0_Len);
}


void BroadcastToAll (unsigned short *mes, CLIENT* client)
{
	unsigned short xEE_Len;
	unsigned short *pd;
	unsigned short *cname;
	unsigned ch, connectNum;

	memcpy (&PacketData[0], &PacketEE[0], sizeof (PacketEE));
	xEE_Len = sizeof (PacketEE);

	pd = (unsigned short*) &PacketData[xEE_Len];
	cname = (unsigned short*) &client->character.name[4];

	*(pd++) = 91;
	*(pd++) = 71;
	*(pd++) = 77;
	*(pd++) = 93;

	xEE_Len += 8;

	while (*cname != 0x0000)
	{
		*(pd++) = *(cname++);
		xEE_Len += 2;
	}

	*(pd++) = 58;
	*(pd++) = 32;

	xEE_Len += 4;

	while (*mes != 0x0000)
	{
		if (*mes == 0x0024)
		{
			*(pd++) = 0x0009;
			mes++;
		}
		else
		{
			*(pd++) = *(mes++);
		}
		xEE_Len += 2;
	}

	PacketData[xEE_Len++] = 0x00;
	PacketData[xEE_Len++] = 0x00;

	while (xEE_Len % 8)
		PacketData[xEE_Len++] = 0x00;
	*(unsigned short*) &PacketData[0] = xEE_Len;
	if (client->announce == 1)
	{
		// Local announcement
		for (ch=0;ch<serverNumConnections;ch++)
		{
			connectNum = serverConnectionList[ch];
			if (connections[connectNum]->guildcard)
			{
				cipher_ptr = &connections[connectNum]->server_cipher;
				encryptcopy (connections[connectNum], &PacketData[0], xEE_Len);
			}	
		}
	}
	else
	{
		// Global announcement
		if (logon.sockfd >= 0)
		{
			// Send the announcement to the logon server.
			logon.encryptbuf[0x00] = 0x12;
			logon.encryptbuf[0x01] = 0x00;
			*(unsigned *) &logon.encryptbuf[0x02] = client->guildcard;
			memcpy (&logon.encryptbuf[0x06], &PacketData[sizeof(PacketEE)], xEE_Len - sizeof (PacketEE));
			compressShipPacket (&logon, &logon.encryptbuf[0x00], 6 + xEE_Len - sizeof (PacketEE));
		}
	}
	client->announce = 0;
}

void GlobalBroadcast (unsigned short *mes)
{
	unsigned short xEE_Len;
	unsigned short *pd;
	unsigned ch, connectNum;

	memcpy (&PacketData[0], &PacketEE[0], sizeof (PacketEE));
	xEE_Len = sizeof (PacketEE);

	pd = (unsigned short*) &PacketData[xEE_Len];

	while (*mes != 0x0000)
	{
		*(pd++) = *(mes++);
		xEE_Len += 2;
	}

	PacketData[xEE_Len++] = 0x00;
	PacketData[xEE_Len++] = 0x00;

	while (xEE_Len % 8)
		PacketData[xEE_Len++] = 0x00;
	*(unsigned short*) &PacketData[0] = xEE_Len;
	for (ch=0;ch<serverNumConnections;ch++)
	{
		connectNum = serverConnectionList[ch];
		if (connections[connectNum]->guildcard)
		{
			cipher_ptr = &connections[connectNum]->server_cipher;
			encryptcopy (connections[connectNum], &PacketData[0], xEE_Len);
		}	
	}
}


void SendEE (const char *mes, CLIENT* client)
{
	unsigned short xEE_Len;

	memcpy (&PacketData[0], &PacketEE[0], sizeof (PacketEE));
	xEE_Len = sizeof (PacketEE);

	while (*mes != 0x00)
	{
		PacketData[xEE_Len++] = *(mes++);
		PacketData[xEE_Len++] = 0x00;
	}

	PacketData[xEE_Len++] = 0x00;
	PacketData[xEE_Len++] = 0x00;

	while (xEE_Len % 8)
		PacketData[xEE_Len++] = 0x00;
	*(unsigned short*) &PacketData[0] = xEE_Len;
	cipher_ptr = &client->server_cipher;
	encryptcopy (client, &PacketData[0], xEE_Len);
}

void Send19 (unsigned char ip1, unsigned char ip2, unsigned char ip3, unsigned char ip4, unsigned short ipp, CLIENT* client)
{
	memcpy ( &client->encryptbuf[0], &Packet19, sizeof (Packet19));
	client->encryptbuf[0x08] = ip1;
	client->encryptbuf[0x09] = ip2;
	client->encryptbuf[0x0A] = ip3;
	client->encryptbuf[0x0B] = ip4;
	*(unsigned short*) &client->encryptbuf[0x0C] = ipp;
	cipher_ptr = &client->server_cipher;
	encryptcopy (client, &client->encryptbuf[0], sizeof (Packet19));
}

void SendEA (unsigned char command, CLIENT* client)
{
	switch (command)
	{
	case 0x02:
		memset (&client->encryptbuf[0x00], 0, 8);
		client->encryptbuf[0x00] = 0x08;
		client->encryptbuf[0x02] = 0xEA;
		client->encryptbuf[0x03] = 0x02;
		cipher_ptr = &client->server_cipher;
		encryptcopy (client, &client->encryptbuf[0], 8);
		break;
	case 0x04:
		memset (&client->encryptbuf[0x00], 0, 8);
		client->encryptbuf[0x00] = 0x08;
		client->encryptbuf[0x02] = 0xEA;
		client->encryptbuf[0x03] = 0x04;
		cipher_ptr = &client->server_cipher;
		encryptcopy (client, &client->encryptbuf[0], 8);
		break;
	case 0x0E:
		memset (&client->encryptbuf[0x00], 0, 0x38);
		client->encryptbuf[0x00] = 0x38;
		client->encryptbuf[0x01] = 0x08;
		client->encryptbuf[0x02] = 0xEA;
		client->encryptbuf[0x03] = 0x0E;
		*(unsigned *) &client->encryptbuf[0x08] = client->guildcard;
		*(unsigned *) &client->encryptbuf[0x0C] = client->character.teamID;
		memcpy (&client->encryptbuf[0x18], &client->character.teamName[0], 28 );
		client->encryptbuf[0x34] = 0x84;
		client->encryptbuf[0x35] = 0x6C;
		memcpy (&client->encryptbuf[0x36], &client->character.teamFlag[0], 0x800);
		client->encryptbuf[0x836] = 0xFF;
		cipher_ptr = &client->server_cipher;
		encryptcopy (client, &client->encryptbuf[0], 0x838);
		break;
	case 0x10:
		memset (&client->encryptbuf[0x00], 0, 8);
		client->encryptbuf[0x00] = 0x08;
		client->encryptbuf[0x02] = 0xEA;
		client->encryptbuf[0x03] = 0x10;
		cipher_ptr = &client->server_cipher;
		encryptcopy (client, &client->encryptbuf[0], 8);
		break;
	case 0x11:
		memset (&client->encryptbuf[0x00], 0, 8);
		client->encryptbuf[0x00] = 0x08;
		client->encryptbuf[0x02] = 0xEA;
		client->encryptbuf[0x03] = 0x11;
		cipher_ptr = &client->server_cipher;
		encryptcopy (client, &client->encryptbuf[0], 8);
		break;
	case 0x12:
		memset (&client->encryptbuf[0x00], 0, 0x40);
		client->encryptbuf[0x00] = 0x40;
		client->encryptbuf[0x02] = 0xEA;
		client->encryptbuf[0x03] = 0x12;
		if ( client->character.teamID  )
		{
			*(unsigned *) &client->encryptbuf[0x0C] = client->guildcard;
			*(unsigned *) &client->encryptbuf[0x10] = client->character.teamID;
			client->encryptbuf[0x1C] = (unsigned char) client->character.privilegeLevel;
			memcpy (&client->encryptbuf[0x20], &client->character.teamName[0], 28);
			client->encryptbuf[0x3C] = 0x84;
			client->encryptbuf[0x3D] = 0x6C;
			client->encryptbuf[0x3E] = 0x98;
		}
		cipher_ptr = &client->server_cipher;
		encryptcopy (client, &client->encryptbuf[0], 0x40);
/*
		if ( client->lobbyNum < 0x10 )
			SendToLobby ( client->lobby, 12, &client->encryptbuf[0x00], 0x40, 0 );
		else
			SendToLobby ( client->lobby, 4, &client->encryptbuf[0x00], 0x40, 0 );
*/
		break;
	case 0x13:
		{
			LOBBY *l;
			CLIENT *lClient;
			unsigned ch, total_clients, EA15Offset, maxc;

			if (!client->lobby)
				break;

			l = (LOBBY*) client->lobby;

			if ( client->lobbyNum < 0x10 ) 
				maxc = 12;
			else
				maxc = 4;
			EA15Offset = 0x08;
			total_clients = 0;
			for (ch=0;ch<maxc;ch++)
			{
				if ((l->slot_use[ch]) && (l->client[ch]))
				{
					lClient = l->client[ch];
					*(unsigned *) &client->encryptbuf[EA15Offset] = lClient->character.guildCard2;
					EA15Offset += 0x04;
					*(unsigned *) &client->encryptbuf[EA15Offset] = lClient->character.teamID;
					EA15Offset += 0x04;
					memset(&client->encryptbuf[EA15Offset], 0, 8);
					EA15Offset += 0x08;
					client->encryptbuf[EA15Offset] = (unsigned char) lClient->character.privilegeLevel;
					EA15Offset += 4;
					memcpy(&client->encryptbuf[EA15Offset], &lClient->character.teamName[0], 28);
					EA15Offset += 28;
					if ( lClient->character.teamID != 0 )
					{
						client->encryptbuf[EA15Offset++] = 0x84;
						client->encryptbuf[EA15Offset++] = 0x6C;
						client->encryptbuf[EA15Offset++] = 0x98;
						client->encryptbuf[EA15Offset++] = 0x00;
					}
					else
					{
						memset (&client->encryptbuf[EA15Offset], 0, 4);
						EA15Offset+= 4;
					}
					*(unsigned *) &client->encryptbuf[EA15Offset] = lClient->character.guildCard;
					EA15Offset += 4;
					client->encryptbuf[EA15Offset++] = lClient->clientID;
					memset(&client->encryptbuf[EA15Offset], 0, 3);
					EA15Offset += 3;
					memcpy(&client->encryptbuf[EA15Offset], &lClient->character.name[0], 24);
					EA15Offset += 24;
					memset(&client->encryptbuf[EA15Offset], 0, 8);
					EA15Offset += 8;
					memcpy(&client->encryptbuf[EA15Offset], &lClient->character.teamFlag[0], 0x800);
					EA15Offset += 0x800;
					total_clients++;
				}
			}
			*(unsigned short*) &client->encryptbuf[0x00] = (unsigned short) EA15Offset;
			client->encryptbuf[0x02] = 0xEA;
			client->encryptbuf[0x03] = 0x13;
			*(unsigned*) &client->encryptbuf[0x04] = total_clients;
			cipher_ptr = &client->server_cipher;
			encryptcopy (client, &client->encryptbuf[0], EA15Offset);
		}
		break;
	case 0x18:
		memset (&client->encryptbuf[0x00], 0, 0x4C);
		client->encryptbuf[0x00] = 0x4C;
		client->encryptbuf[0x02] = 0xEA;
		client->encryptbuf[0x03] = 0x18;
		client->encryptbuf[0x14] = 0x01;
		client->encryptbuf[0x18] = 0x01;
		client->encryptbuf[0x1C] = (unsigned char) client->character.privilegeLevel;
		*(unsigned *) &client->encryptbuf[0x20] = client->character.guildCard;
		memcpy (&client->encryptbuf[0x24], &client->character.name[0], 24);
		client->encryptbuf[0x48] = 0x02;
		cipher_ptr = &client->server_cipher;
		encryptcopy (client, &client->encryptbuf[0], 0x4C);
		break;
	case 0x19:
		memset (&client->encryptbuf[0x00], 0, 0x0C);
		client->encryptbuf[0x00] = 0x0C;
		client->encryptbuf[0x02] = 0xEA;
		client->encryptbuf[0x03] = 0x19;
		cipher_ptr = &client->server_cipher;
		encryptcopy (client, &client->encryptbuf[0], 0x0C);
		break;
	case 0x1A:
		memset (&client->encryptbuf[0x00], 0, 0x0C);
		client->encryptbuf[0x00] = 0x0C;
		client->encryptbuf[0x02] = 0xEA;
		client->encryptbuf[0x03] = 0x1A;
		cipher_ptr = &client->server_cipher;
		encryptcopy (client, &client->encryptbuf[0], 0x0C);
		break;
	case 0x1D:
		memset (&client->encryptbuf[0x00], 0, 8);
		client->encryptbuf[0x00] = 0x08;
		client->encryptbuf[0x02] = 0xEA;
		client->encryptbuf[0x03] = 0x1D;
		cipher_ptr = &client->server_cipher;
		encryptcopy (client, &client->encryptbuf[0], 8);
		break;
	default:
		break;
	}
}

unsigned char* MakePacketEA15 (CLIENT* client)
{
	sprintf (&PacketData[0x00], "\x64\x08\xEA\x15\x01");
	memset  (&PacketData[0x05], 0, 3);
	*(unsigned *) &PacketData[0x08] = client->guildcard;
	*(unsigned *) &PacketData[0x0C] = client->character.teamID;
	PacketData [0x18] = (unsigned char) client->character.privilegeLevel;
	memcpy  (&PacketData [0x1C], &client->character.teamName[0], 28);
	sprintf (&PacketData[0x38], "\x84\x6C\x98");
	*(unsigned *) &PacketData[0x3C] = client->guildcard;
	PacketData[0x40] = client->clientID;
	memcpy  (&PacketData[0x44], &client->character.name[0], 24);
	memcpy  (&PacketData[0x64], &client->character.teamFlag[0], 0x800);
	return   &PacketData[0];
}

unsigned free_game_item (LOBBY* l)
{
	unsigned ch, ch2, oldest_item;

	ch2 = oldest_item = 0xFFFFFFFF;

	// If the itemid at the current index is 0, just return that...

	if ((l->gameItemCount < MAX_SAVED_ITEMS) && (l->gameItem[l->gameItemCount].item.itemid == 0))
		return l->gameItemCount;

	// Scan the gameItem array for any free item slots...

	for (ch=0;ch<MAX_SAVED_ITEMS;ch++)
	{
		if (l->gameItem[ch].item.itemid == 0)
		{
			ch2 = ch;
			break;
		}
	}

	if (ch2 != 0xFFFFFFFF)
		return ch2;

	// Out of inventory memory!  Time to delete the oldest dropped item in the game...

	for (ch=0;ch<MAX_SAVED_ITEMS;ch++)
	{
		if ((l->gameItem[ch].item.itemid < oldest_item) && (l->gameItem[ch].item.itemid >= 0x810000))
		{
			ch2 = ch;
			oldest_item = l->gameItem[ch].item.itemid;
		}
	}

	if (ch2 != 0xFFFFFFFF)
	{
		l->gameItem[ch2].item.itemid = 0; // Item deleted.
		return ch2;
	}

	for (ch=0;ch<4;ch++)
	{
		if ((l->slot_use[ch]) && (l->client[ch]))
			SendEE ("Lobby inventory problem!  It's advised you quit this game and recreate it.", l->client[ch]);
	}

	return 0;
}

void UpdateGameItem (CLIENT* client)
{
	// Updates the game item list for all of the client's items...  (Used strictly when a client joins a game...)

	LOBBY* l;
	unsigned ch;

	memset (&client->tekked, 0, sizeof (INVENTORY_ITEM)); // Reset tekking data

	if (!client->lobby)
		return;

	l = (LOBBY*) client->lobby;

	for (ch=0;ch<client->character.inventoryUse;ch++) // By default this should already be sorted at the top, so no need for an in_use check...	
		client->character.inventory[ch].item.itemid = l->playerItemID[client->clientID]++; // Keep synchronized
}
