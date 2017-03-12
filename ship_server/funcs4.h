// Chat
void Send06 (CLIENT* client)
{
	FILE* fp;
	unsigned short chatsize;
	unsigned pktsize;
	unsigned ch, ch2;
	unsigned char stackable, count;
	unsigned short *n;
	unsigned short target;
	unsigned myCmdArgs, itemNum, connectNum, gc_num;
	unsigned short npcID;
	unsigned max_send;
	CLIENT* lClient;
	int i, z, commandLen, ignored, found_ban, writeData;
	LOBBY* l;
	INVENTORY_ITEM ii;

	writeData = 0;
	fp = NULL;

	if (!client->lobby)
		return;

	l = client->lobby;

	pktsize = *(unsigned short*) &client->decryptbuf[0x00];

	if (pktsize > 0x100)
		return;

	memset (&chatBuf[0x00], 0, 0x0A);

	chatBuf[0x02] = 0x06;
	chatBuf[0x0A] = client->clientID;
	*(unsigned *) &chatBuf[0x0C] = client->guildcard;
	chatsize = 0x10;
	n = (unsigned short*) &client->character.name[4];
	for (ch=0;ch<10;ch++)
	{
		if (*n == 0x0000)
			break;
		*(unsigned short*) &chatBuf[chatsize] = *n;
		chatsize += 2;
		n++;
	}
	chatBuf[chatsize++] = 0x09;
	chatBuf[chatsize++] = 0x00;
	chatBuf[chatsize++] = 0x09;
	chatBuf[chatsize++] = 0x00;
	n = (unsigned short*) &client->decryptbuf[0x12];
	if ((*(n+1) == 0x002F) && (*(n+2) != 0x002F))
	{
		commandLen = 0;

		for (ch=0;ch<(pktsize - 0x14);ch+= 2)
		{
			if (client->decryptbuf[(0x14+ch)] != 0x00)
				cmdBuf[commandLen++] = client->decryptbuf[(0x14+ch)];
			else
				break;
		}

		cmdBuf[commandLen] = 0;

		myCmdArgs = 0;
		myCommand = &cmdBuf[1];

		// If command has any arguments
		if ( ( i = strcspn ( &cmdBuf[1], " ," ) ) != ( strlen ( &cmdBuf[1] ) ) )
		{
			// count the arguments and store the addresses in myArgs char* array
			i++;
			cmdBuf[i++] = 0;  // clear command address from buffer (myCommand has it)
			while ( ( i < commandLen ) && ( myCmdArgs < 64 ) )  // max 64 args (63?)
			{
				z = strcspn ( &cmdBuf[i], "," );  // length of arg
				myArgs[myCmdArgs++] = &cmdBuf[i]; // save address of arg into myArgs
				i += z;
				cmdBuf[i++] = 0;
			}
		}

		if ( commandLen )
		{

			if ( !strcmp ( myCommand, "debug" ) )
				writeData = 1;

			if ( !strcmp ( myCommand, "setpass" ) )
			{
				if (!client->lobbyNum < 0x10)
				{
					if ( myCmdArgs == 0 )
						SendB0 ("Need new password.", client);
					else
					{
						ch = 0;
						while (myArgs[0][ch] != 0)
						{
							l->gamePassword [ch*2] = myArgs[0][ch];
							l->gamePassword [(ch*2)+1] = 0;
							ch++;
							if (ch==31) break; // Limit amount of characters...
						}
						l->gamePassword[ch*2] = 0;
						l->gamePassword[(ch*2)+1] = 0;
						for (ch=0;ch<4;ch++)
						{
							if ((l->slot_use[ch]) && (l->client[ch]))
								SendB0 ("Room password changed.", l->client[ch]);
						}
					}
				}
			}

			if ( !strcmp ( myCommand, "arrow" ) )
			{
				if ( myCmdArgs == 0 )
					SendB0 ("Need arrow digit.", client);
				else
				{
					l->arrow_color[client->clientID] = atoi ( myArgs[0] );
					ShowArrows (client, 1);
				}
			}

			if ( !strcmp ( myCommand, "lang" ) )
			{
				if ( myCmdArgs == 0 )
					SendB0 ("Need language digit.", client);
				else
				{
					npcID = atoi ( myArgs[0] );

					if ( npcID > numLanguages ) npcID = 1;

					if ( npcID == 0 )
						npcID = 1;

					npcID--;

					client->character.lang = (unsigned char) npcID;

					SendB0 ("Current language:\n", client);
					SendB0 (languageNames[npcID], client);

				}
			}

			if ( !strcmp ( myCommand, "npc" ) )
			{
				if ( myCmdArgs == 0 )
					SendB0 ("Need NPC digit. (max = 11, 0 to unskin)", client);
				else
				{
					npcID = atoi ( myArgs[0] );

					if ( npcID > 7 ) 
					{
						if ( ( npcID > 11 ) || ( !ship_support_extnpc ) )
							npcID = 7;
					}

					if ( npcID == 0 )
					{
						client->character.skinFlag = 0x00;
						client->character.skinID = 0x00;
					}
					else
					{
						client->character.skinFlag = 0x02;
						client->character.skinID = npcID - 1;
					}
					SendB0 ("Skin updated, change blocks for it to take effect.", client );
				}
			}

			// Process GM commands
			// The second argument to playerHasRight() is the bit position of the maks of the command

			if ( ( !strcmp ( myCommand, "event" ) ) && ( (client->isgm) || (playerHasRights(client->guildcard, 0))) )
			{
				if ( myCmdArgs == 0 )
					SendB0 ("Need event digit.", client);
				else
				{

					shipEvent = atoi ( myArgs[0] );

					WriteGM ("GM %u has changed ship event to %u", client->guildcard, shipEvent );

					PacketDA[0x04] = shipEvent;

					for (ch=0;ch<serverNumConnections;ch++)
					{
						connectNum = serverConnectionList[ch];
						if (connections[connectNum]->guildcard)
						{
							cipher_ptr = &connections[connectNum]->server_cipher;
							encryptcopy (connections[connectNum], &PacketDA[0], 8);
						}
					}
				}
			}

			if ( ( !strcmp ( myCommand, "redbox") ) && ( client->isgm ) )
			{
				if (l->redbox)
				{
					l->redbox = 0;
					SendB0 ("Red box mode turned off.", client);
					WriteGM ("GM %u has deactivated redbox mode", client->guildcard ); 
				}
				else
				{
					l->redbox = 1;
					SendB0 ("Red box mode turned on!", client);
					WriteGM ("GM %u has activated redbox mode", client->guildcard ); 
				}
			}

			if ( ( !strcmp ( myCommand, "item" ) )  && ( (client->isgm) || (playerHasRights(client->guildcard, 1))) )
			{
				// Item creation...
				if ( client->lobbyNum < 0x10 )
					SendB0 ("Cannot make items in the lobby!!!", client);
				else
					if ( myCmdArgs < 4 )
						SendB0 ("You must specify at least four arguments for the desired item.", client);
					else
						if ( strlen ( myArgs[0] ) < 8 ) 
							SendB0 ("Main arguments is an incorrect length.", client);
						else
						{
							if ( ( strlen ( myArgs[1] ) < 8 ) ||
								( strlen ( myArgs[2] ) < 8 ) || 
								( strlen ( myArgs[3] ) < 8 ) )
								SendB0 ("Some arguments were incorrect and replaced.", client);

							WriteGM ("GM %u created an item", client->guildcard);

							itemNum = free_game_item (l);

							_strupr ( myArgs[0] );
							l->gameItem[itemNum].item.data[0]  = hexToByte (&myArgs[0][0]);
							l->gameItem[itemNum].item.data[1]  = hexToByte (&myArgs[0][2]);
							l->gameItem[itemNum].item.data[2]  = hexToByte (&myArgs[0][4]);
							l->gameItem[itemNum].item.data[3]  = hexToByte (&myArgs[0][6]);


							if ( strlen ( myArgs[1] ) >= 8 ) 
							{
								_strupr ( myArgs[1] );
								l->gameItem[itemNum].item.data[4]  = hexToByte (&myArgs[1][0]);
								l->gameItem[itemNum].item.data[5]  = hexToByte (&myArgs[1][2]);
								l->gameItem[itemNum].item.data[6]  = hexToByte (&myArgs[1][4]);
								l->gameItem[itemNum].item.data[7]  = hexToByte (&myArgs[1][6]);
							}
							else
							{
								l->gameItem[itemNum].item.data[4]  = 0;
								l->gameItem[itemNum].item.data[5]  = 0;
								l->gameItem[itemNum].item.data[6]  = 0;
								l->gameItem[itemNum].item.data[7]  = 0;
							}

							if ( strlen ( myArgs[2] ) >= 8 ) 
							{
								_strupr ( myArgs[2] );
								l->gameItem[itemNum].item.data[8]  = hexToByte (&myArgs[2][0]);
								l->gameItem[itemNum].item.data[9]  = hexToByte (&myArgs[2][2]);
								l->gameItem[itemNum].item.data[10] = hexToByte (&myArgs[2][4]);
								l->gameItem[itemNum].item.data[11] = hexToByte (&myArgs[2][6]);
							}
							else
							{
								l->gameItem[itemNum].item.data[8]  = 0;
								l->gameItem[itemNum].item.data[9]  = 0;
								l->gameItem[itemNum].item.data[10] = 0;
								l->gameItem[itemNum].item.data[11] = 0;
							}

							if ( strlen ( myArgs[3] ) >= 8 ) 
							{
								_strupr ( myArgs[3] );
								l->gameItem[itemNum].item.data2[0]  = hexToByte (&myArgs[3][0]);
								l->gameItem[itemNum].item.data2[1]  = hexToByte (&myArgs[3][2]);
								l->gameItem[itemNum].item.data2[2]  = hexToByte (&myArgs[3][4]);
								l->gameItem[itemNum].item.data2[3]  = hexToByte (&myArgs[3][6]);
							}
							else
							{
								l->gameItem[itemNum].item.data2[0]  = 0;
								l->gameItem[itemNum].item.data2[1]  = 0;
								l->gameItem[itemNum].item.data2[2]  = 0;
								l->gameItem[itemNum].item.data2[3]  = 0;
							}

							// check stackable shit

							stackable = 0;

							if (l->gameItem[itemNum].item.data[0] == 0x03)
								stackable = stackable_table[l->gameItem[itemNum].item.data[1]];

							if ( ( stackable ) && ( l->gameItem[itemNum].item.data[5] == 0x00 ) )
								l->gameItem[itemNum].item.data[5] = 0x01; // force at least 1 of a stack to drop

							WriteGM ("Item data: %02X%02X%02X%02X,%02X%02X%02X%02X,%02X%02x%02x%02x,%02x%02x%02x%02x",
								l->gameItem[itemNum].item.data[0], l->gameItem[itemNum].item.data[1], l->gameItem[itemNum].item.data[2], l->gameItem[itemNum].item.data[3],
								l->gameItem[itemNum].item.data[4], l->gameItem[itemNum].item.data[5], l->gameItem[itemNum].item.data[6], l->gameItem[itemNum].item.data[7],
								l->gameItem[itemNum].item.data[8], l->gameItem[itemNum].item.data[9], l->gameItem[itemNum].item.data[10], l->gameItem[itemNum].item.data[11],
								l->gameItem[itemNum].item.data2[0], l->gameItem[itemNum].item.data2[1], l->gameItem[itemNum].item.data2[2], l->gameItem[itemNum].item.data2[3] );

							l->gameItem[itemNum].item.itemid = l->itemID++;
							if (l->gameItemCount < MAX_SAVED_ITEMS)
								l->gameItemList[l->gameItemCount++] = itemNum;
							memset (&PacketData[0], 0, 0x2C);
							PacketData[0x00] = 0x2C;
							PacketData[0x02] = 0x60;
							PacketData[0x08] = 0x5D;
							PacketData[0x09] = 0x09;
							PacketData[0x0A] = 0xFF;
							PacketData[0x0B] = 0xFB;
							*(unsigned *) &PacketData[0x0C] = l->floor[client->clientID];
							*(unsigned *) &PacketData[0x10] = l->clientx[client->clientID];
							*(unsigned *) &PacketData[0x14] = l->clienty[client->clientID];
							memcpy (&PacketData[0x18], &l->gameItem[itemNum].item.data[0], 12 );
							*(unsigned *) &PacketData[0x24] = l->gameItem[itemNum].item.itemid;
							*(unsigned *) &PacketData[0x28] = *(unsigned *) &l->gameItem[itemNum].item.data2[0];
							SendToLobby ( client->lobby, 4, &PacketData[0], 0x2C, 0);
							SendB0 ("Item created.", client);
						}								
			}

			if ( ( !strcmp ( myCommand, "give" ) )  && ( (client->isgm) || (playerHasRights(client->guildcard, 1))) )
			{
				// Insert item into inventory
				if ( client->lobbyNum < 0x10 )
					SendB0 ("Cannot give items in the lobby!!!", client);
				else
					if ( myCmdArgs < 4 )
						SendB0 ("You must specify at least four arguments for the desired item.", client);
					else
						if ( strlen ( myArgs[0] ) < 8 )
							SendB0 ("Main arguments is an incorrect length.", client);
						else
						{
							if ( ( strlen ( myArgs[1] ) < 8 ) ||
								 ( strlen ( myArgs[2] ) < 8 ) || 
								 ( strlen ( myArgs[3] ) < 8 ) )
								SendB0 ("Some arguments were incorrect and replaced.", client);

							WriteGM ("GM %u obtained an item", client->guildcard);

							_strupr ( myArgs[0] );
							ii.item.data[0]  = hexToByte (&myArgs[0][0]);
							ii.item.data[1]  = hexToByte (&myArgs[0][2]);
							ii.item.data[2]  = hexToByte (&myArgs[0][4]);
							ii.item.data[3]  = hexToByte (&myArgs[0][6]);


							if ( strlen ( myArgs[1] ) >= 8 ) 
							{
								_strupr ( myArgs[1] );
								ii.item.data[4]  = hexToByte (&myArgs[1][0]);
								ii.item.data[5]  = hexToByte (&myArgs[1][2]);
								ii.item.data[6]  = hexToByte (&myArgs[1][4]);
								ii.item.data[7]  = hexToByte (&myArgs[1][6]);
							}
							else
							{
								ii.item.data[4]  = 0;
								ii.item.data[5]  = 0;
								ii.item.data[6]  = 0;
								ii.item.data[7]  = 0;
							}

							if ( strlen ( myArgs[2] ) >= 8 ) 
							{
								_strupr ( myArgs[2] );
								ii.item.data[8]  = hexToByte (&myArgs[2][0]);
								ii.item.data[9]  = hexToByte (&myArgs[2][2]);
								ii.item.data[10] = hexToByte (&myArgs[2][4]);
								ii.item.data[11] = hexToByte (&myArgs[2][6]);
							}
							else
							{
								ii.item.data[8]  = 0;
								ii.item.data[9]  = 0;
								ii.item.data[10] = 0;
								ii.item.data[11] = 0;
							}

							if ( strlen ( myArgs[3] ) >= 8 ) 
							{
								_strupr ( myArgs[3] );
								ii.item.data2[0]  = hexToByte (&myArgs[3][0]);
								ii.item.data2[1]  = hexToByte (&myArgs[3][2]);
								ii.item.data2[2]  = hexToByte (&myArgs[3][4]);
								ii.item.data2[3]  = hexToByte (&myArgs[3][6]);
							}
							else
							{
								ii.item.data2[0]  = 0;
								ii.item.data2[1]  = 0;
								ii.item.data2[2]  = 0;
								ii.item.data2[3]  = 0;
							}

							// check stackable shit

							stackable = 0;

							if (ii.item.data[0] == 0x03)
								stackable = stackable_table[ii.item.data[1]];

							if ( stackable )
							{
								if ( ii.item.data[5] == 0x00 )
									ii.item.data[5] = 0x01; // force at least 1 of a stack to drop
								count = ii.item.data[5];
							}
							else
								count = 1;

							WriteGM ("Item data: %02X%02X%02X%02X,%02X%02X%02X%02X,%02X%02x%02x%02x,%02x%02x%02x%02x",
								ii.item.data[0], ii.item.data[1], ii.item.data[2], ii.item.data[3],
								ii.item.data[4], ii.item.data[5], ii.item.data[6], ii.item.data[7],
								ii.item.data[8], ii.item.data[9], ii.item.data[10], ii.item.data[11],
								ii.item.data2[0], ii.item.data2[1], ii.item.data2[2], ii.item.data2[3] );

							ii.item.itemid = l->itemID++;
							AddToInventory ( &ii, count, 0, client );
							SendB0 ("Item obtained.", client);
						}
			}

			if ( ( !strcmp ( myCommand, "warpme" ) )  && ((client->isgm) || (playerHasRights(client->guildcard, 3))) )
			{
				if ( client->lobbyNum < 0x10 )
					SendB0 ("Can't warp in the lobby!!!", client);
				else
					if ( myCmdArgs == 0 )
						SendB0 ("Need area to warp to...", client);
					else
					{
						target = atoi ( myArgs[0] );
						if ( target > 17 )
							SendB0 ("Warping past area 17 would probably crash your client...", client);
						else
						{
							warp_packet[0x0C] = (unsigned char) atoi ( myArgs[0] );
							cipher_ptr = &client->server_cipher;
							encryptcopy (client, &warp_packet[0], sizeof (warp_packet));
						}
					}
			}

			if ( ( !strcmp ( myCommand, "dc" ) ) && ((client->isgm) || (playerHasRights(client->guildcard, 4))) )
			{
				if ( myCmdArgs == 0 )
					SendB0 ("Need a guild card # to disconnect.", client);
				else
				{
					gc_num = atoi ( myArgs[0] );
					for (ch=0;ch<serverNumConnections;ch++)
					{
						connectNum = serverConnectionList[ch];
						if (connections[connectNum]->guildcard == gc_num)
						{
							if ((connections[connectNum]->isgm) && (isLocalGM(client->guildcard)))
								SendB0 ("You may not disconnect this user.", client);
							else
							{
								WriteGM ("GM %u has disconnected user %u (%s)", client->guildcard, gc_num, Unicode_to_ASCII ((unsigned short*) &connections[connectNum]->character.name[4]));
								Send1A ("You've been disconnected by a GM.", connections[connectNum]);
								connections[connectNum]->todc = 1;
								break;
							}
						}
					}
				}
			}

			if ( ( !strcmp ( myCommand, "ban" ) ) && ((client->isgm) || (playerHasRights(client->guildcard, 11))) )
			{
				if ( myCmdArgs == 0 )
					SendB0 ("Need a guild card # to ban.", client);
				else
				{
					gc_num = atoi ( myArgs[0] );
					found_ban = 0;

					for (ch=0;ch<num_bans;ch++)
					{
						if ((ship_bandata[ch].guildcard == gc_num) && (ship_bandata[ch].type == 1))
						{
							found_ban = 1;
							ban ( gc_num, (unsigned*) &client->ipaddr, &client->hwinfo, 1, client ); // Should unban...
							WriteGM ("GM %u has removed ban from guild card %u.", client->guildcard, gc_num);
							SendB0 ("Ban removed.", client );
							break;
						}
					}

					if (!found_ban)
					{
						for (ch=0;ch<serverNumConnections;ch++)
						{
							connectNum = serverConnectionList[ch];
							if (connections[connectNum]->guildcard == gc_num)
							{
								if ((connections[connectNum]->isgm) || (isLocalGM(connections[connectNum]->guildcard)))
									SendB0 ("You may not ban this user.", client);
								else
								{
									if ( ban ( gc_num, (unsigned*) &connections[connectNum]->ipaddr, 
										&connections[connectNum]->hwinfo, 1, client ) )
									{
										WriteGM ("GM %u has banned user %u (%s)", client->guildcard, gc_num, Unicode_to_ASCII ((unsigned short*) &connections[connectNum]->character.name[4]));
										Send1A ("You've been banned by a GM.", connections[connectNum]);
										SendB0 ("User has been banned.", client );
										connections[connectNum]->todc = 1;
									}
									break;
								}
							}
						}
					}
				}
			}

			if ( ( !strcmp ( myCommand, "ipban" ) ) && ((client->isgm) || (playerHasRights(client->guildcard, 12))) )
			{
				if ( myCmdArgs == 0 )
					SendB0 ("Need a guild card # to IP ban.", client);
				else
				{
					gc_num = atoi ( myArgs[0] );
					found_ban = 0;

					for (ch=0;ch<num_bans;ch++)
					{
						if ((ship_bandata[ch].guildcard == gc_num) && (ship_bandata[ch].type == 2))
						{
							found_ban = 1;
							ban ( gc_num, (unsigned*) &client->ipaddr, &client->hwinfo, 2, client ); // Should unban...
							WriteGM ("GM %u has removed IP ban from guild card %u.", client->guildcard, gc_num);
							SendB0 ("IP ban removed.", client );
							break;
						}
					}

					if (!found_ban)
					{
						for (ch=0;ch<serverNumConnections;ch++)
						{
							connectNum = serverConnectionList[ch];
							if (connections[connectNum]->guildcard == gc_num)
							{
								if ((connections[connectNum]->isgm) || (isLocalGM(connections[connectNum]->guildcard)))
									SendB0 ("You may not ban this user.", client);
								else
								{
									if ( ban ( gc_num, (unsigned*) &connections[connectNum]->ipaddr, 
										&connections[connectNum]->hwinfo, 2, client ) )
									{
										WriteGM ("GM %u has IP banned user %u (%s)", client->guildcard, gc_num, Unicode_to_ASCII ((unsigned short*) &connections[connectNum]->character.name[4]));
										Send1A ("You've been banned by a GM.", connections[connectNum]);
										SendB0 ("User has been IP banned.", client );
										connections[connectNum]->todc = 1;
									}
									break;
								}
							}
						}
					}
				}
			}

			if ( ( !strcmp ( myCommand, "hwban" ) ) && ((client->isgm) || (playerHasRights(client->guildcard, 12))) )
			{
				if ( myCmdArgs == 0 )
					SendB0 ("Need a guild card # to HW ban.", client);
				else
				{
					gc_num = atoi ( myArgs[0] );
					found_ban = 0;

					for (ch=0;ch<num_bans;ch++)
					{
						if ((ship_bandata[ch].guildcard == gc_num) && (ship_bandata[ch].type == 3))
						{
							found_ban = 1;
							ban ( gc_num, (unsigned*) &client->ipaddr, &client->hwinfo, 3, client ); // Should unban...
							WriteGM ("GM %u has removed HW ban from guild card %u.", client->guildcard, gc_num);
							SendB0 ("HW ban removed.", client );
							break;
						}
					}

					if (!found_ban)
					{
						for (ch=0;ch<serverNumConnections;ch++)
						{
							connectNum = serverConnectionList[ch];
							if (connections[connectNum]->guildcard == gc_num)
							{
								if ((connections[connectNum]->isgm) || (isLocalGM(connections[connectNum]->guildcard)))
									SendB0 ("You may not ban this user.", client);
								else
								{
									if ( ban ( gc_num, (unsigned*) &connections[connectNum]->ipaddr, 
										&connections[connectNum]->hwinfo, 3, client ) )
									{
										WriteGM ("GM %u has HW banned user %u (%s)", client->guildcard, gc_num, Unicode_to_ASCII ((unsigned short*) &connections[connectNum]->character.name[4]));
										Send1A  ("You've been banned by a GM.", connections[connectNum]);
										SendB0  ("User has been HW banned.", client );
										connections[connectNum]->todc = 1;
									}
									break;
								}
							}
						}
					}
				}
			}


			if ( ( !strcmp ( myCommand, "dcall" ) ) && ((client->isgm) || (playerHasRights(client->guildcard, 5))) )
			{
				printf ("Blocking connections until all users are disconnected...\n");
				WriteGM ("GM %u has disconnected all users", client->guildcard);
				blockConnections = 1;
			}

			if ( ( !strcmp ( myCommand, "announce" ) ) && ((client->isgm) || (playerHasRights(client->guildcard, 6))) )
			{
				if (client->announce != 0)
				{
					SendB0 ("Announce\ncancelled.", client);
					client->announce = 0;
				}
				else
				{
					SendB0 ("Announce by\nsending a\nmail.", client);
					client->announce = 1;
				}
			}

			if ( ( !strcmp ( myCommand, "global" ) ) && (client->isgm) )
			{
				if (client->announce != 0)
				{
					SendB0 ("Announce\ncancelled.", client);
					client->announce = 0;
				}
				else
				{
					SendB0 ("Global announce\nby sending\na mail.", client);
					client->announce = 2;
				}
			}

			if ( ( !strcmp ( myCommand, "levelup" ) ) && ((client->isgm) || (playerHasRights(client->guildcard, 7))) )
			{
				if ( client->lobbyNum < 0x10 )
					SendB0 ("Cannot level up in the lobby!!!", client);
				else
					if ( l->floor[client->clientID] == 0 )
						SendB0 ("Please leave Pioneer 2 before using this command...", client);
					else
						if ( myCmdArgs == 0 )
							SendB0 ("Must specify a target level to level up to...", client);
						else
						{
							target = atoi ( myArgs[0] );
							if ( ( client->character.level + 1 ) >= target )
								SendB0 ("Target level must be higher than your current level...", client);
							else
							{
								// Do the level up!!!

								if (target > 200)
									target = 200;

								target -= 2;

								AddExp (tnlxp[target] - client->character.XP, client);
							}
						}
			}

			if ( (!strcmp ( myCommand, "updatelocalgms" )) && ((client->isgm) || (playerHasRights(client->guildcard, 8))) )
			{
				SendB0 ("Local GM file reloaded.", client);
				readLocalGMFile();
			}
			
			if ( (!strcmp ( myCommand, "updatemasks" )) && ((client->isgm) || (playerHasRights(client->guildcard, 12))) )
			{
				SendB0 ("IP ban masks file reloaded.", client);
				load_mask_file();
			}

			if ( !strcmp ( myCommand, "bank" ) )
			{
				if (client->bankType)
				{
					client->bankType = 0;
					SendB0 ("Bank: Character", client);
				}
				else
				{
					client->bankType = 1;
					SendB0 ("Bank: Common", client);
				}
			}

			if ( !strcmp ( myCommand, "ignore" ) )
			{
				if ( myCmdArgs == 0 )
					SendB0 ("Need a guild card # to ignore.", client);
				else
				{
					gc_num = atoi ( myArgs[0] );
					ignored = 0;

					for (ch=0;ch<client->ignore_count;ch++)
					{
						if (client->ignore_list[ch] == gc_num)
						{
							ignored = 1;
							client->ignore_list[ch] = 0;
							SendB0 ("User no longer being ignored.", client);
							break;
						}
					}

					if (!ignored)
					{
						if (client->ignore_count < 100)
						{
							client->ignore_list[client->ignore_count++] = gc_num;
							SendB0 ("User is now ignored.", client);
						}
						else
							SendB0 ("Ignore list is full.", client);
					}
					else
					{
						ch2 = 0;
						for (ch=0;ch<client->ignore_count;ch++)
						{
							if ((client->ignore_list[ch] != 0) && (ch != ch2))
								client->ignore_list[ch2++] = client->ignore_list[ch];
						}
						client->ignore_count = ch2;
					}
				}
			}

			if ( ( !strcmp ( myCommand, "stfu" ) ) && ((client->isgm) || (playerHasRights(client->guildcard, 9))) )
			{
				if ( myCmdArgs == 0 )
					SendB0 ("Need a guild card # to silence.", client);
				else
				{
					gc_num = atoi ( myArgs[0] );
					for (ch=0;ch<serverNumConnections;ch++)
					{
						connectNum = serverConnectionList[ch];
						if (connections[connectNum]->guildcard == gc_num)
						{
							if ((connections[connectNum]->isgm) && (isLocalGM(client->guildcard)))
								SendB0 ("You may not silence this user.", client);
							else
							{							
								if (toggle_stfu(connections[connectNum]->guildcard, client))
								{
									WriteGM ("GM %u has silenced user %u (%s)", client->guildcard, gc_num, Unicode_to_ASCII ((unsigned short*) &connections[connectNum]->character.name[4]));
									SendB0  ("User has been silenced.", client);
									SendB0  ("You've been silenced.", connections[connectNum]);
								}
								else
								{
									WriteGM ("GM %u has removed silence from user %u (%s)", client->guildcard, gc_num, Unicode_to_ASCII ((unsigned short*) &connections[connectNum]->character.name[4]));
									SendB0  ("User is now allowed to speak.", client);
									SendB0  ("You may now speak freely.", connections[connectNum]);
								}
								break;
							}
						}
					}
				}
			}

			if ( ( !strcmp ( myCommand, "warpall" ) )  && ((client->isgm) || (playerHasRights(client->guildcard, 10))) )
			{
				if ( client->lobbyNum < 0x10 )
					SendB0 ("Can't warp in the lobby!!!", client);
				else
					if ( myCmdArgs == 0 )
						SendB0 ("Need area to warp to...", client);
					else
					{
						target = atoi ( myArgs[0] );
						if ( target > 17 )
							SendB0 ("Warping past area 17 would probably crash your client...", client);
						else
						{
							warp_packet[0x0C] = (unsigned char) atoi ( myArgs[0] );
							SendToLobby ( client->lobby, 4, &warp_packet[0], sizeof (warp_packet), 0 );
						}
					}
			}
			
			if ( (!strcmp ( myCommand, "reloadconfig" )) && ((client->isgm) || (playerHasRights(client->guildcard, 8))) )
			{
				load_config_file();
				SendB0 ("Ship config file reloaded.", client);
			}
		
			if ( (!strcmp ( myCommand, "setval" )) && ((client->isgm) || (playerHasRights(client->guildcard, 2))) )
			{
				if (myCmdArgs < 1)
					SendB0 ("You must provide at least one argument.\nType \"/setval help\" or\n\"/setval help,[topic]\" for more info.", client);
				else
				{
					if (!strcmp(myArgs[0], "help"))
					{
						if (myArgs[1] == NULL)
						{
							SendB0 ("Usage: /setval [var],[value]", client);
							SendB0 ("Args for var: help, exp, rboxd, rmobd,\nrmob", client);
						}
						else if (!strcmp(myArgs[1], "exp"))
							SendB0 ("The rate (x100) of experience earned.", client);
					}
					if (!strncmp(myArgs[0], "exp", 3))
					{
						if (myArgs[1] == NULL)
							SendB0 ("Provide a num to set the exp rate to.", client);
						else
						{
							EXPERIENCE_RATE = atoi (myArgs[1]);
							if (EXPERIENCE_RATE > 100)
							{
								SendB0 ("Too large -- truncated to 100.", client);
								EXPERIENCE_RATE = 100;
							}
							if (EXPERIENCE_RATE < 1)
							{
								SendB0 ("Must be a num greater than 0.\nSet to 1.", client);
								EXPERIENCE_RATE = 1;
							}
							WriteGM ("GM %u (%s) has set the exp rate to %d%%", client->guildcard, Unicode_to_ASCII((unsigned short *)&client->character.name[4]), EXPERIENCE_RATE*100);
							unsigned char mesg[] = "Exp is now ";
							int i = strlen(mesg);
							sprintf (&mesg[i], "%d%%", EXPERIENCE_RATE*100);
							SendEE (mesg, client);
						}
					}
					if (!strncmp(myArgs[0], "rboxd", 6))
					{
						if (myArgs[1] == NULL)
							SendB0 ("Provide a num to set the rare box multiplier.", client);
						else
						{
							int val = atoi (myArgs[1]);
							if (val > 100)
							{
								SendB0 ("Too large -- truncated to 100.", client);
								val = 100;
							}
							if (val < 1)
							{
								SendB0 ("Must be a num greater than 0.\nSet to 1.", client);
								val = 1;
							}
							rare_box_mult = val;
							WriteGM ("GM %u (%s) has set the rare item box drop rate to %d%%", client->guildcard, Unicode_to_ASCII((unsigned short *)&client->character.name[4]), val*100);
							unsigned char mesg[] = "Rare item occurence rate in boxes is now ";
							int i = strlen(mesg);
							sprintf (&mesg[i], "%d%%", val*100);
							SendEE (mesg, client);
						}
					}
				}
			}
		}
	}
	else
	{
		for (ch=0;ch<(pktsize - 0x12);ch+=2)
		{
			if ((*n == 0x0000) || (chatsize == 0xC0))
				break;
			if ((*n == 0x0009) || (*n == 0x000A))
				*n = 0x0020;
			*(unsigned short*) &chatBuf[chatsize] = *n;
			chatsize += 2;
			n++;
		}
		chatBuf[chatsize++] = 0x00;
		chatBuf[chatsize++] = 0x00;
		while (chatsize % 8)
			chatBuf[chatsize++] = 0x00;
		*(unsigned short*) &chatBuf[0x00] = chatsize;
		if ( !stfu ( client->guildcard ) )
		{
			if ( client->lobbyNum < 0x10 )
				max_send = 12;
			else
				max_send = 4;
			for (ch=0;ch<max_send;ch++)
			{
				if ((l->slot_use[ch]) && (l->client[ch]))
				{
					ignored = 0;
					lClient = l->client[ch];
					for (ch2=0;ch2<lClient->ignore_count;ch2++)
					{
						if (lClient->ignore_list[ch2] == client->guildcard)
						{
							ignored = 1;
							break;
						}
					}
					if (!ignored)
					{
						cipher_ptr = &lClient->server_cipher;
						encryptcopy ( lClient, &chatBuf[0x00], chatsize );
					}
				}
			}
		}
	}

	if ( writeData )
	{
		if (!client->debugged)
		{
			client->debugged = 1;
			_itoa (client->character.guildCard, &character_file[0], 10);
			strcat (&character_file[0], Unicode_to_ASCII ((unsigned short*) &client->character.name[4]));
			strcat (&character_file[0], ".bbc");
			fp = fopen (&character_file[0], "wb");
			if (fp)
			{
				fwrite (&client->character.packetSize, 1, sizeof (CHARDATA), fp);
				fclose (fp);
			}
			WriteLog ("User %u (%s) has wrote character debug data.", client->guildcard, Unicode_to_ASCII ((unsigned short*) &client->character.name[4]) );
			SendB0 ("Your debug data has been saved.", client);
		}
		else
			SendB0 ("Your debug data has already been saved.", client);
	}
}

void CommandED(CLIENT* client)
{
	switch (client->decryptbuf[0x03])
	{
	case 0x01:
		// Options
		*(unsigned *) &client->character.options[0] = *(unsigned *) &client->decryptbuf[0x08];
		break;
	case 0x02:
		// Symbol Chats
		memcpy (&client->character.symbol_chats, &client->decryptbuf[0x08], 1248);
		break;
	case 0x03:
		// Shortcuts
		memcpy (&client->character.shortcuts, &client->decryptbuf[0x08], 2624);
		break;
	case 0x04:
		// Global Key Config
		memcpy (&client->character.keyConfigGlobal, &client->decryptbuf[0x08], 364);
		break;
	case 0x05:
		// Global Joystick Config
		memcpy (&client->character.joyConfigGlobal, &client->decryptbuf[0x08], 56);
		break;
	case 0x06:
		// Technique Config
		memcpy (&client->character.techConfig, &client->decryptbuf[0x08], 40);
		break;
	case 0x07:
		// Character Key Config
		memcpy (&client->character.keyConfig, &client->decryptbuf[0x08], 232);
		break;
	case 0x08:
		// C-Rank and Battle Config
		//memcpy (&client->character.challengeData, &client->decryptbuf[0x08], 320);
		break;
	}
}

void Command40(CLIENT* client, SERVER* ship)
{
	// Guild Card Search

	ship->encryptbuf[0x00] = 0x08;
	ship->encryptbuf[0x01] = 0x01;
	*(unsigned *) &ship->encryptbuf[0x02] = *(unsigned *) &client->decryptbuf[0x10];
	*(unsigned *) &ship->encryptbuf[0x06] = *(unsigned *) &client->guildcard;
	*(unsigned *) &ship->encryptbuf[0x0A] = serverID;
	*(unsigned *) &ship->encryptbuf[0x0E] = client->character.teamID;
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x21);
}

void Command09(CLIENT* client)
{
	QUEST* q;
	CLIENT* c;
	LOBBY* l;
	unsigned lobbyNum, Packet11_Length, ch;
	char lb[10];
	int num_hours, num_minutes;

	switch ( client->decryptbuf[0x0F] )
	{
	case 0x00:
		// Team info
		if ( client->lobbyNum < 0x10 )
		{
			if ((!client->block) || (client->block > serverBlocks))
			{
				initialize_connection (client);
				return;
			}

			lobbyNum = *(unsigned *) &client->decryptbuf[0x0C];

			if ((lobbyNum < 0x10) || (lobbyNum >= 16+SHIP_COMPILED_MAX_GAMES))
			{
				initialize_connection (client);
				return;
			}

			l = &blocks[client->block - 1]->lobbies[lobbyNum];
			memset (&PacketData, 0, 0x10);
			PacketData[0x02] = 0x11;
			PacketData[0x0A] = 0x20;
			PacketData[0x0C] = 0x20;
			PacketData[0x0E] = 0x20;
			if (l->in_use)
			{
				Packet11_Length = 0x10;
				if ((client->team_info_request != lobbyNum) || (client->team_info_flag == 0))
				{
					client->team_info_request = lobbyNum;
					client->team_info_flag = 1;
					for (ch=0;ch<4;ch++)
						if ((l->slot_use[ch]) && (l->client[ch]))
						{
							c = l->client[ch];
							wstrcpy((unsigned short*) &PacketData[Packet11_Length], (unsigned short*) &c->character.name[0]);
							Packet11_Length += wstrlen ((unsigned short*) &PacketData[Packet11_Length]);
							PacketData[Packet11_Length++] = 0x20;
							PacketData[Packet11_Length++] = 0x00;
							PacketData[Packet11_Length++] = 0x4C;
							PacketData[Packet11_Length++] = 0x00;
							_itoa (l->client[ch]->character.level + 1, &lb[0], 10);
							wstrcpy_char(&PacketData[Packet11_Length], &lb[0]);
							Packet11_Length += wstrlen ((unsigned short*) &PacketData[Packet11_Length]);
							PacketData[Packet11_Length++] = 0x0A;
							PacketData[Packet11_Length++] = 0x00;
							PacketData[Packet11_Length++] = 0x20;
							PacketData[Packet11_Length++] = 0x00;
							PacketData[Packet11_Length++] = 0x20;
							PacketData[Packet11_Length++] = 0x00;
							switch (c->character._class)
							{
							case CLASS_HUMAR:
								wstrcpy_char (&PacketData[Packet11_Length], "HUmar");
								break;
							case CLASS_HUNEWEARL:
								wstrcpy_char (&PacketData[Packet11_Length], "HUnewearl");
								break;
							case CLASS_HUCAST:
								wstrcpy_char (&PacketData[Packet11_Length], "HUcast");
								break;
							case CLASS_HUCASEAL:
								wstrcpy_char (&PacketData[Packet11_Length], "HUcaseal");
								break;
							case CLASS_RAMAR:
								wstrcpy_char (&PacketData[Packet11_Length], "RAmar");
								break;
							case CLASS_RACAST:
								wstrcpy_char (&PacketData[Packet11_Length], "RAcast");
								break;
							case CLASS_RACASEAL:
								wstrcpy_char (&PacketData[Packet11_Length], "RAcaseal");
								break;
							case CLASS_RAMARL:
								wstrcpy_char (&PacketData[Packet11_Length], "RAmarl");
								break;
							case CLASS_FONEWM:
								wstrcpy_char (&PacketData[Packet11_Length], "FOnewm");
								break;
							case CLASS_FONEWEARL:
								wstrcpy_char (&PacketData[Packet11_Length], "FOnewearl");
								break;
							case CLASS_FOMARL:
								wstrcpy_char (&PacketData[Packet11_Length], "FOmarl");
								break;
							case CLASS_FOMAR:
								wstrcpy_char (&PacketData[Packet11_Length], "FOmar");
								break;
							default:
								wstrcpy_char (&PacketData[Packet11_Length], "Unknown");
								break;
							}
							Packet11_Length += wstrlen ((unsigned short*) &PacketData[Packet11_Length]);
							PacketData[Packet11_Length++] = 0x20;
							PacketData[Packet11_Length++] = 0x00;
							PacketData[Packet11_Length++] = 0x20;
							PacketData[Packet11_Length++] = 0x00;
							PacketData[Packet11_Length++] = 0x20;
							PacketData[Packet11_Length++] = 0x00;
							PacketData[Packet11_Length++] = 0x20;
							PacketData[Packet11_Length++] = 0x00;
							PacketData[Packet11_Length++] = 0x4A;
							PacketData[Packet11_Length++] = 0x00;
							PacketData[Packet11_Length++] = 0x0A;
							PacketData[Packet11_Length++] = 0x00;
						}
				}
				else
				{
					client->team_info_request = lobbyNum;
					client->team_info_flag = 0;
					wstrcpy_char (&PacketData[Packet11_Length], "Time : ");
					Packet11_Length += wstrlen ((unsigned short*) &PacketData[Packet11_Length]);
					num_minutes  = ((unsigned) servertime - l->start_time ) / 60L;
					num_hours    = num_minutes / 60L;
					num_minutes %= 60;
					_itoa (num_hours,&lb[0], 10);
					wstrcpy_char (&PacketData[Packet11_Length], &lb[0]);
					Packet11_Length += wstrlen ((unsigned short*) &PacketData[Packet11_Length]);
					PacketData[Packet11_Length++] = 0x3A;
					PacketData[Packet11_Length++] = 0x00;
					_itoa (num_minutes,&lb[0], 10);
					if (num_minutes < 10)
					{
						lb[1] = lb[0];
						lb[0] = 0x30;
						lb[2] = 0x00;
					}
					wstrcpy_char (&PacketData[Packet11_Length], &lb[0]);
					Packet11_Length += wstrlen ((unsigned short*) &PacketData[Packet11_Length]);					
					PacketData[Packet11_Length++] = 0x0A;
					PacketData[Packet11_Length++] = 0x00;
					if (l->quest_loaded)
					{
						wstrcpy_char (&PacketData[Packet11_Length], "Quest : ");
						Packet11_Length += wstrlen ((unsigned short*) &PacketData[Packet11_Length]);
						PacketData[Packet11_Length++] = 0x0A;
						PacketData[Packet11_Length++] = 0x00;
						PacketData[Packet11_Length++] = 0x20;
						PacketData[Packet11_Length++] = 0x00;
						PacketData[Packet11_Length++] = 0x20;
						PacketData[Packet11_Length++] = 0x00;
						q = &quests[l->quest_loaded - 1];
						if ((client->character.lang < 10) && (q->ql[client->character.lang]))
							wstrcpy((unsigned short*) &PacketData[Packet11_Length], (unsigned short*) &q->ql[client->character.lang]->qname[0]);
						else
							wstrcpy((unsigned short*) &PacketData[Packet11_Length], (unsigned short*) &q->ql[0]->qname[0]);
						Packet11_Length += wstrlen ((unsigned short*) &PacketData[Packet11_Length]);
					}
				}
			}
			else
			{
				wstrcpy_char (&PacketData[0x10], "Game no longer active.");
				Packet11_Length = 0x10 + (strlen ("Game no longer active.") * 2);
			}
			PacketData[Packet11_Length++] = 0x00;
			PacketData[Packet11_Length++] = 0x00;
			*(unsigned short*) &PacketData[0x00] = (unsigned short) Packet11_Length;
			cipher_ptr = &client->server_cipher;
			encryptcopy (client, &PacketData[0], Packet11_Length );
		}
		break;
	case 0x0F:
		// Quest info
		if ( ( client->lobbyNum > 0x0F ) && ( client->decryptbuf[0x0B] <= numQuests ) )
		{
			q = &quests[client->decryptbuf[0x0B] - 1];
			memset (&PacketData[0x00], 0, 8);
			PacketData[0x00] = 0x48;
			PacketData[0x01] = 0x02;
			PacketData[0x02] = 0xA3;
			if ((client->character.lang < 10) && (q->ql[client->character.lang]))
				memcpy (&PacketData[0x08], &q->ql[client->character.lang]->qdetails[0], 0x200 );
			else
				memcpy (&PacketData[0x08], &q->ql[0]->qdetails[0], 0x200 );
			cipher_ptr = &client->server_cipher;
			encryptcopy (client, &PacketData[0], 0x248 );
		}
		break;
	default:
		break;
	}
}

void Command10(unsigned blockServer, CLIENT* client)
{
	unsigned char select_type, selected;
	unsigned full_select, ch, ch2, failed_to_join, lobbyNum, password_match, oldIndex;
	LOBBY* l;
	unsigned short* p;
	unsigned short* c;
	unsigned short fqs, barule;
	unsigned qm_length, qa, nr;
	unsigned char* qmap;
	QUEST* q;
	char quest_num[16];
	unsigned qn;
	int do_quest;
	unsigned quest_flag;

	if (client->guildcard)
	{
		select_type = (unsigned char) client->decryptbuf[0x0F];
		selected = (unsigned char) client->decryptbuf[0x0C];
		full_select = *(unsigned *) &client->decryptbuf[0x0C];

		switch (select_type)
		{
		case 0x00:
			if ( ( blockServer ) && ( client->lobbyNum < 0x10 ) )
			{
				// Team

				if ((!client->block) || (client->block > serverBlocks))
				{
					initialize_connection (client);
					return;
				}

				lobbyNum = *(unsigned *) &client->decryptbuf[0x0C];

				if ((lobbyNum < 0x10) || (lobbyNum >= 16+SHIP_COMPILED_MAX_GAMES))
				{
					initialize_connection (client);
					return;
				}

				failed_to_join = 0;
				l = &blocks[client->block - 1]->lobbies[lobbyNum];

				if ((!client->isgm) && (!isLocalGM(client->guildcard)))
				{
					switch (l->episode)
					{
					case 0x01:
						if ((l->difficulty == 0x01) && (client->character.level < 19))
						{
							Send01 ("Episode I\n\nYou must be level\n20 or higher\nto play on the\nhard difficulty.", client);
							failed_to_join = 1;
						}
						else
							if ((l->difficulty == 0x02) && (client->character.level < 49))
							{
								Send01 ("Episode I\n\nYou must be level\n50 or higher\nto play on the\nvery hard\ndifficulty.", client);
								failed_to_join = 1;
							}
							else
								if ((l->difficulty == 0x03) && (client->character.level < 89))
								{
									Send01 ("Episode I\n\nYou must be level\n90 or higher\nto play on the\nultimate\ndifficulty.", client);
									failed_to_join = 1;
								}
								break;
					case 0x02:
						if ((l->difficulty == 0x01) && (client->character.level < 29))
						{
							Send01 ("Episode II\n\nYou must be level\n30 or higher\nto play on the\nhard difficulty.", client);
							failed_to_join = 1;
						}
						else
							if ((l->difficulty == 0x02) && (client->character.level < 59))
							{
								Send01 ("Episode II\n\nYou must be level\n60 or higher\nto play on the\nvery hard\ndifficulty.", client);
								failed_to_join = 1;
							}
							else
								if ((l->difficulty == 0x03) && (client->character.level < 99))
								{
									Send01 ("Episode II\n\nYou must be level\n100 or higher\nto play on the\nultimate\ndifficulty.", client);
									failed_to_join = 1;
								}
								break;
					case 0x03:
						if ((l->difficulty == 0x01) && (client->character.level < 39))
						{
							Send01 ("Episode IV\n\nYou must be level\n40 or higher\nto play on the\nhard difficulty.", client);
							failed_to_join = 1;
						}
						else
							if ((l->difficulty == 0x02) && (client->character.level < 69))
							{
								Send01 ("Episode IV\n\nYou must be level\n70 or higher\nto play on the\nvery hard\ndifficulty.", client);
								failed_to_join = 1;
							}
							else
								if ((l->difficulty == 0x03) && (client->character.level < 109))
								{
									Send01 ("Episode IV\n\nYou must be level\n110 or higher\nto play on the\nultimate\ndifficulty.", client);
									failed_to_join = 1;
								}
								break;
					}
				}


				if ((!l->in_use) && (!failed_to_join))
				{
					Send01 ("Game no longer active.", client);
					failed_to_join = 1;
				}

				if ((l->lobbyCount == 4)  && (!failed_to_join))
				{
					Send01 ("Game is full", client);
					failed_to_join = 1;
				}

				if ((l->quest_in_progress) && (!failed_to_join))
				{
					Send01 ("Quest already in progress.", client);
					failed_to_join = 1;
				}

				if ((l->oneperson) && (!failed_to_join))
				{
					Send01 ("Cannot join a one\nperson game.", client);
					failed_to_join = 1;
				}

				if (((l->gamePassword[0x00] != 0x00) || (l->gamePassword[0x01] != 0x00)) &&
					(!failed_to_join))
				{
					password_match = 1;
					p = (unsigned short*) &l->gamePassword[0x00];
					c = (unsigned short*) &client->decryptbuf[0x10];
					while (*p != 0x00)
					{
						if (*p != *c)
							password_match = 0;
						p++;
						c++;
					}
					if ((password_match == 0) && (client->isgm == 0) && (isLocalGM(client->guildcard) == 0))
					{
						Send01 ("Incorrect password.", client);
						failed_to_join = 1;
					}
				}

				if (!failed_to_join)
				{
					for (ch=0;ch<4;ch++)
					{
						if ((l->slot_use[ch]) && (l->client[ch]))
						{
							if (l->client[ch]->bursting == 1)
							{
								Send01 ("Player is bursting.\nPlease wait a\nmoment.", client);
								failed_to_join = 1;
							}
							else
								if ((l->inpquest) && (!l->client[ch]->hasquest))
								{
									Send01 ("Player is loading\nquest.\nPlease wait a\nmoment.", client);
									failed_to_join = 1;
								}
						}
					}
				}

				if ((l->inpquest) && (!failed_to_join))
				{
					// Check if player qualifies to join Government quest...
					q = &quests[l->quest_loaded - 1];
					memcpy (&dp[0], &q->ql[0]->qdata[0x31], 3);
					dp[4] = 0;
					qn = (unsigned) atoi ( &dp[0] );
					switch (l->episode)
					{
					case 0x01:
						qn -= 401;
						qn <<= 1;
						qn += 0x1F3;
						for (ch2=0x1F5;ch2<=qn;ch2+=2)
							if (!qflag(&client->character.quest_data1[0], ch2, l->difficulty))
								failed_to_join = 1;
						break;
					case 0x02:
						qn -= 451;
						qn <<= 1;
						qn += 0x211;
						for (ch2=0x213;ch2<=qn;ch2+=2)
							if (!qflag(&client->character.quest_data1[0], ch2, l->difficulty))
								failed_to_join = 1;
						break;
					case 0x03:
						qn -= 701;
						qn += 0x2BC;
						for (ch2=0x2BD;ch2<=qn;ch2++)
							if (!qflag(&client->character.quest_data1[0], ch2, l->difficulty))
								failed_to_join = 1;
						break;
					}
					if (failed_to_join)
					{
						if ((client->isgm == 0) && (isLocalGM(client->guildcard) == 0))
							Send01 ("You must progress\nfurther in the\ngame before you\ncan join this\nquest.", client);
						else
							failed_to_join = 0;
					}
				}

				if (failed_to_join == 0)
				{
					removeClientFromLobby (client);
					client->lobbyNum = lobbyNum + 1;
					client->lobby = (void*) l;
					Send64 (client);
					memset (&client->encryptbuf[0x00], 0, 0x0C);
					client->encryptbuf[0x00] = 0x0C;
					client->encryptbuf[0x02] = 0x60;
					client->encryptbuf[0x08] = 0xDD;
					client->encryptbuf[0x09] = 0x03;
					client->encryptbuf[0x0A] = (unsigned char) EXPERIENCE_RATE;
					cipher_ptr = &client->server_cipher;
					encryptcopy (client, &client->encryptbuf[0x00], 0x0C);
					UpdateGameItem (client);
				}
			}
			break;
		case 0x0F:
			// Quest selection
			if ( ( blockServer ) && ( client->lobbyNum > 0x0F ) )
			{
				if (!client->lobby)
					break;

				l = (LOBBY*) client->lobby;

				if ( client->decryptbuf[0x0B] == 0 )
				{
					if ( client->decryptbuf[0x0C] < 11 )
						SendA2 ( l->episode, l->oneperson, client->decryptbuf[0x0C], client->decryptbuf[0x0A], client );
				}
				else
				{
					if ( l->leader == client->clientID )
					{
						if ( l->quest_loaded == 0 )
						{
							if ( client->decryptbuf[0x0B] <= numQuests )
							{
								q = &quests[client->decryptbuf[0x0B] - 1];

								do_quest = 1;

								// Check "One-Person" quest ability to repeat...

								if ( ( l->oneperson ) && ( l->episode == 0x01 ) )
								{
									memcpy (&quest_num[0], &q->ql[0]->qdata[49], 3);
									quest_num[4] = 0;
									qn = atoi (&quest_num[0]);
									quest_flag = 0x63 + (qn << 1);
									if (qflag(&client->character.quest_data1[0], quest_flag, l->difficulty))
									{
										if (!qflag_ep1solo(&client->character.quest_data1[0], l->difficulty))
											do_quest = 0;
									}
									if ( !do_quest )
										Send01 ("Please clear\nthe remaining\nquests before\nredoing this one.", client);
								}

								// Check party Government quest qualification.  (Teamwork?!)

								if ( client->decryptbuf[0x0A] )
								{
									memcpy (&dp[0], &q->ql[0]->qdata[0x31], 3);
									dp[4] = 0;
									qn = (unsigned) atoi ( &dp[0] );
									switch (l->episode)
									{
									case 0x01:
										qn -= 401;
										qn <<= 1;
										qn += 0x1F3;
										for (ch2=0x1F5;ch2<=qn;ch2+=2)
											for (ch=0;ch<4;ch++)
												if ((l->client[ch]) && (!qflag(&l->client[ch]->character.quest_data1[0], ch2, l->difficulty)))
													do_quest = 0;
										break;
									case 0x02:
										qn -= 451;
										qn <<= 1;
										qn += 0x211;
										for (ch2=0x213;ch2<=qn;ch2+=2)
											for (ch=0;ch<4;ch++)
												if ((l->client[ch]) && (!qflag(&l->client[ch]->character.quest_data1[0], ch2, l->difficulty)))
													do_quest = 0;
										break;
									case 0x03:
										qn -= 701;
										qn += 0x2BC;
										for (ch2=0x2BD;ch2<=qn;ch2++)
											for (ch=0;ch<4;ch++)
												if ((l->client[ch]) && (!qflag(&l->client[ch]->character.quest_data1[0], ch2, l->difficulty)))
													do_quest = 0;
										break;
									}
									if (!do_quest)
										Send01 ("The party no longer\nqualifies to\nstart this quest.", client);
								}

								if ( do_quest )
								{
									ch2 = 0;
									barule = 0;

									while (q->ql[0]->qname[ch2] != 0x00)
									{
										// Search for a number in the quest name to determine battle rule #
										if ((q->ql[0]->qname[ch2] >= 0x31) && (q->ql[0]->qname[ch2] <= 0x38))
										{
											barule = q->ql[0]->qname[ch2];
											break;
										}
										ch2++;
									}

									for (ch=0;ch<4;ch++)
										if ((l->slot_use[ch]) && (l->client[ch]))
										{
											if ((l->battle) || (l->challenge))
											{
												// Copy character to backup buffer.
												if (l->client[ch]->character_backup)
													free (l->client[ch]->character_backup);
												l->client[ch]->character_backup = malloc (sizeof (l->client[ch]->character));
												memcpy ( l->client[ch]->character_backup, &l->client[ch]->character, sizeof (l->client[ch]->character));

												l->battle_level = 0;

												switch ( barule )
												{
												case 0x31:
													// Rule #1
													l->client[ch]->mode = 1;
													break;
												case 0x32:
													// Rule #2
													l->battle_level = 1;
													l->client[ch]->mode = 3;
													break;
												case 0x33:
													// Rule #3
													l->battle_level = 5;
													l->client[ch]->mode = 3;
													break;
												case 0x34:
													// Rule #4
													l->battle_level = 2;
													l->client[ch]->mode = 3;
													l->meseta_boost = 1;
													break;
												case 0x35:
													// Rule #5
													l->client[ch]->mode = 2;
													l->meseta_boost = 1;
													break;
												case 0x36:
													// Rule #6
													l->battle_level = 20;
													l->client[ch]->mode = 3;
													break;
												case 0x37:
													// Rule #7
													l->battle_level = 1;
													l->client[ch]->mode = 3;
													break;
												case 0x38:
													// Rule #8
													l->battle_level = 20;
													l->client[ch]->mode = 3;
													break;
												default:
													WriteLog ("Unknown battle rule loaded...");
													l->client[ch]->mode = 1;
													break;
												}

												switch (l->client[ch]->mode)
												{
												case 0x02:
													// Delete all mags and meseta...
													for (ch2=0;ch2<l->client[ch]->character.inventoryUse;ch2++)
													{
														if (l->client[ch]->character.inventory[ch2].item.data[0] == 0x02)
															l->client[ch]->character.inventory[ch2].in_use = 0;
													}
													CleanUpInventory (l->client[ch]);
													l->client[ch]->character.meseta = 0;
													break;
												case 0x03:
													// Wipe items and reset level.
													for (ch2=0;ch2<30;ch2++)
														l->client[ch]->character.inventory[ch2].in_use = 0;
													CleanUpInventory (l->client[ch]);
													l->client[ch]->character.level = 0;
													l->client[ch]->character.XP = 0;
													l->client[ch]->character.ATP = *(unsigned short*) &startingData[(l->client[ch]->character._class*14)];
													l->client[ch]->character.MST = *(unsigned short*) &startingData[(l->client[ch]->character._class*14)+2];
													l->client[ch]->character.EVP = *(unsigned short*) &startingData[(l->client[ch]->character._class*14)+4];
													l->client[ch]->character.HP  = *(unsigned short*) &startingData[(l->client[ch]->character._class*14)+6];
													l->client[ch]->character.DFP = *(unsigned short*) &startingData[(l->client[ch]->character._class*14)+8];
													l->client[ch]->character.ATA = *(unsigned short*) &startingData[(l->client[ch]->character._class*14)+10];
													if (l->battle_level > 1)
														SkipToLevel (l->battle_level - 1, l->client[ch], 1);
													l->client[ch]->character.meseta = 0;
												}
											}

											if ((l->client[ch]->character.lang < 10) &&
												(q->ql[l->client[ch]->character.lang]))
											{
												fqs = *(unsigned short*) &q->ql[l->client[ch]->character.lang]->qdata[0];
												if (fqs % 8)
													fqs += ( 8 - ( fqs % 8 ) );
												cipher_ptr = &l->client[ch]->server_cipher;
												encryptcopy (l->client[ch], &q->ql[l->client[ch]->character.lang]->qdata[0], fqs);
											}
											else
											{
												fqs = *(unsigned short*) &q->ql[0]->qdata[0];
												if (fqs % 8)
													fqs += ( 8 - ( fqs % 8 ) );
												cipher_ptr = &l->client[ch]->server_cipher;
												encryptcopy (l->client[ch], &q->ql[0]->qdata[0], fqs);
											}
											l->client[ch]->bursting = 1;
											l->client[ch]->sending_quest = client->decryptbuf[0x0B] - 1;
											l->client[ch]->qpos = fqs;
										}
										if (!client->decryptbuf[0x0A])
											l->quest_in_progress = 1; // when a government quest, this won't be set
										else
											l->inpquest = 1;

										l->quest_loaded = client->decryptbuf[0x0B];

										// Time to load the map data...

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
											//debug ("loaded quest area %u, mid count %u, total mids: %u", qa, l->mapIndex - oldIndex, l->mapIndex);
										}
								}
							}
						}
						else
							Send01 ("Quest already loaded.", client);
					}
					else
						Send01 ("Only the leader of a team can start quests.", client);
				}
			}
			break;
		case 0xEF:
			if ( client->lobbyNum < 0x10 )
			{
				// Blocks

				unsigned blockNum;

				blockNum = 0x100 - selected;

				if (blockNum <= serverBlocks)
				{
					if ( blocks[blockNum - 1]->count < 180 )
					{
						if ((client->lobbyNum) && (client->lobbyNum < 0x10))
						{
							for (ch=0;ch<MAX_SAVED_LOBBIES;ch++)
							{
								if (savedlobbies[ch].guildcard == 0)
								{
									savedlobbies[ch].guildcard = client->guildcard;
									savedlobbies[ch].lobby = client->lobbyNum;
									break;
								}
							}
						}

						if (client->gotchardata)
						{
							client->character.playTime += (unsigned) servertime - client->connected; 
							ShipSend04 (0x02, client, &logon);
							client->gotchardata = 0;
							client->released = 1;
							*(unsigned *) &client->releaseIP[0] = *(unsigned *) &serverIP[0];
							client->releasePort = serverPort + blockNum;
						}
						else
							Send19 (serverIP[0], serverIP[1], serverIP[2], serverIP[3],
							serverPort + blockNum, client);
					}
					else
					{
						Send01 ("Block is full.", client);
						Send07 (client);
					}
				}
			}
			break;
		case 0xFF:
			if ( client->lobbyNum < 0x10 )
			{
				// Ship select
				if ( selected == 0x00 )
					ShipSend0D (0x00, client, &logon);
				else
					// Ships
					for (ch=0;ch<totalShips;ch++)
					{
						if (full_select == shipdata[ch].shipID)
						{
							if (client->gotchardata)
							{
								client->character.playTime += (unsigned) servertime - client->connected;
								ShipSend04 (0x02, client, &logon);
								client->gotchardata = 0;
								client->released = 1;
								*(unsigned *) &client->releaseIP[0] = *(unsigned*) &shipdata[ch].ipaddr[0];
								client->releasePort = shipdata[ch].port;
							}
							else
								Send19 (shipdata[ch].ipaddr[0], shipdata[ch].ipaddr[1], 
								shipdata[ch].ipaddr[2], shipdata[ch].ipaddr[3],
								shipdata[ch].port, client);

							break;
						}
					}
			}
			break;
		default:
			break;
		}
	}
}

void CommandD9 (CLIENT* client)
{
	unsigned short *n;
	unsigned short *g;
	unsigned short s = 2;


	// Client writing to info board

	n = (unsigned short*) &client->decryptbuf[0x0A];
	g = (unsigned short*) &client->character.GCBoard[0];

	*(g++) = 0x0009;

	while ((*n != 0x0000) && (s < 85))
	{
		if ((*n == 0x0009) || (*n == 0x000A))
			*(g++) = 0x0020;
		else
			*(g++) = *n;
		n++;
		s++;
	}
	// null terminate
	*(g++) = 0x0000;
}


void AddGuildCard (unsigned myGC, unsigned friendGC, unsigned char* friendName, 
				   unsigned char* friendText, unsigned char friendSecID, unsigned char friendClass,
				   SERVER* ship)
{
	// Instruct the logon server to add the guild card

	ship->encryptbuf[0x00] = 0x07;
	ship->encryptbuf[0x01] = 0x00;
	*(unsigned*) &ship->encryptbuf[0x02] = myGC;
	*(unsigned*) &ship->encryptbuf[0x06] = friendGC;
	memcpy (&ship->encryptbuf[0x0A], friendName, 24);
	memcpy (&ship->encryptbuf[0x22], friendText, 176);
	ship->encryptbuf[0xD2] = friendSecID;
	ship->encryptbuf[0xD3] = friendClass;
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0xD4 );
}

void DeleteGuildCard (unsigned myGC, unsigned friendGC, SERVER* ship)
{
	// Instruct the logon server to delete the guild card

	ship->encryptbuf[0x00] = 0x07;
	ship->encryptbuf[0x01] = 0x01;
	*(unsigned*) &ship->encryptbuf[0x02] = myGC;
	*(unsigned*) &ship->encryptbuf[0x06] = friendGC;
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x0A );
}

void ModifyGuildCardComment (unsigned myGC, unsigned friendGC, unsigned short* n, SERVER* ship)
{
	unsigned s = 1;
	unsigned short* g;

	ship->encryptbuf[0x00] = 0x07;
	ship->encryptbuf[0x01] = 0x02;
	*(unsigned*) &ship->encryptbuf[0x02] = myGC;
	*(unsigned*) &ship->encryptbuf[0x06] = friendGC;

	// Client writing to info board

	g = (unsigned short*) &ship->encryptbuf[0x0A];

	memset (g, 0, 0x44);

	*(g++) = 0x0009;

	while ((*n != 0x0000) && (s < 33))
	{
		if ((*n == 0x0009) || (*n == 0x000A))
			*(g++) = 0x0020;
		else
			*(g++) = *n;
		n++;
		s++;
	}
	*(g++) = 0x0000;

	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x4E );
}

void SortGuildCard (CLIENT* client, SERVER* ship)
{
	ship->encryptbuf[0x00] = 0x07;
	ship->encryptbuf[0x01] = 0x03;
	*(unsigned*) &ship->encryptbuf[0x02] = client->guildcard;
	*(unsigned*) &ship->encryptbuf[0x06] = *(unsigned*) &client->decryptbuf[0x08];
	*(unsigned*) &ship->encryptbuf[0x0A] = *(unsigned*) &client->decryptbuf[0x0C];
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x10 );
}


void CommandE8 (CLIENT* client)
{
	unsigned gcn;


	switch (client->decryptbuf[0x03])
	{
	case 0x04:
		{
			// Accepting sent guild card
			LOBBY* l;
			CLIENT* lClient;
			unsigned ch, maxch;

			if (!client->lobby)
				break;

			l = (LOBBY*) client->lobby;
			gcn = *(unsigned*) &client->decryptbuf[0x08];
			if ( client->lobbyNum < 0x10 )
				maxch = 12;
			else
				maxch = 4;
			for (ch=0;ch<maxch;ch++)
			{
				if ((l->client[ch]) && (l->client[ch]->character.guildCard == gcn))
				{
					lClient = l->client[ch];
					if (PreppedGuildCard(lClient->guildcard,client->guildcard))
					{
						AddGuildCard (client->guildcard, gcn, &client->decryptbuf[0x0C], &client->decryptbuf[0x5C],
							client->decryptbuf[0x10E], client->decryptbuf[0x10F], &logon );
					}
					break;
				}
			}
		}
		break;
	case 0x05:
		// Deleting a guild card
		gcn = *(unsigned*) &client->decryptbuf[0x08];
		DeleteGuildCard (client->guildcard, gcn, &logon);
		break;
	case 0x06:
		// Setting guild card text
		{
			unsigned short *n;
			unsigned short *g;
			unsigned short s = 2;

			// Client writing to info board

			n = (unsigned short*) &client->decryptbuf[0x5E];
			g = (unsigned short*) &client->character.guildcard_text[0];

			*(g++) = 0x0009;

			while ((*n != 0x0000) && (s < 85))
			{
				if ((*n == 0x0009) || (*n == 0x000A))
					*(g++) = 0x0020;
				else
					*(g++) = *n;
				n++;
				s++;
			}
			// null terminate
			*(g++) = 0x0000;
		}
		break;
	case 0x07:
		// Add blocked user
		// User @ 0x08, Name of User @ 0x0C
		break;
	case 0x08:
		// Remove blocked user
		// User @ 0x08
		break;
	case 0x09:
		// Write comment on user
		// E8 09 writing a comment on a user...  not sure were comment goes in the DC packet... 
		// User @ 0x08 comment @ 0x0C
		gcn = *(unsigned*) &client->decryptbuf[0x08];
		ModifyGuildCardComment (client->guildcard, gcn, (unsigned short*) &client->decryptbuf[0x0E], &logon);
		break;
	case 0x0A:
		// Sort guild card
		// (Moves from one position to another)
		SortGuildCard (client, &logon);
		break;
	}
}

void CommandD8 (CLIENT* client)
{
	unsigned ch,maxch;
	unsigned short D8Offset;
	unsigned char totalClients = 0;
	LOBBY* l;
	CLIENT* lClient;

	if (!client->lobby)
		return;

	memset (&PacketData[0], 0, 8);

	PacketData[0x02] = 0xD8;
	D8Offset = 8;

	l = client->lobby;

	if (client->lobbyNum < 0x10)
		maxch = 12;
	else
		maxch = 4;

	for (ch=0;ch<maxch;ch++)
	{
		if ((l->slot_use[ch]) && (l->client[ch]))
		{
			totalClients++;
			lClient = l->client[ch];
			memcpy (&PacketData[D8Offset], &lClient->character.name[4], 20 );
			D8Offset += 0x20;
			memcpy (&PacketData[D8Offset], &lClient->character.GCBoard[0], 172 );
			D8Offset += 0x158;
		}
	}
	PacketData[0x04] = totalClients;
	*(unsigned short*) &PacketData[0x00] = (unsigned short) D8Offset;
	cipher_ptr = &client->server_cipher;
	encryptcopy (client, &PacketData[0], D8Offset);
}

void Command81 (CLIENT* client, SERVER* ship)
{
	unsigned short* n;

	ship->encryptbuf[0x00] = 0x08;
	ship->encryptbuf[0x01] = 0x03;
	memcpy (&ship->encryptbuf[0x02], &client->decryptbuf[0x00], 0x45C);
	*(unsigned*) &ship->encryptbuf[0x0E] = client->guildcard;
	memcpy (&ship->encryptbuf[0x12], &client->character.name[0], 24);
	n = (unsigned short*) &ship->encryptbuf[0x62];
	while (*n != 0x0000)
	{
		if ((*n == 0x0009) || (*n == 0x000A))
			*n = 0x0020;
		n++;
	}
	*n = 0x0000;
	*(unsigned*) &ship->encryptbuf[0x45E] = client->character.teamID;
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x462 );
}


void CreateTeam (unsigned short* teamname, unsigned guildcard, SERVER* ship)
{
	unsigned short *g;
	unsigned n;

	n = 0;

	ship->encryptbuf[0x00] = 0x09;
	ship->encryptbuf[0x01] = 0x00;

	g = (unsigned short*) &ship->encryptbuf[0x02];

	memset (g, 0, 24);
	while ((*teamname != 0x0000) && (n<11))
	{
		if ((*teamname != 0x0009) && (*teamname != 0x000A))
			*(g++) = *teamname;
		else
			*(g++) = 0x0020;
		teamname++;
		n++;
	}
	*(unsigned*) &ship->encryptbuf[0x1A] = guildcard;
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x1E );
}

void UpdateTeamFlag (unsigned char* flag, unsigned teamid, SERVER* ship)
{
	ship->encryptbuf[0x00] = 0x09;
	ship->encryptbuf[0x01] = 0x01;
	memcpy (&ship->encryptbuf[0x02], flag, 0x800);
	*(unsigned*) &ship->encryptbuf[0x802] = teamid;
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x806 );
}

void DissolveTeam (unsigned teamid, SERVER* ship)
{
	ship->encryptbuf[0x00] = 0x09;
	ship->encryptbuf[0x01] = 0x02;
	*(unsigned*) &ship->encryptbuf[0x02] = teamid;
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x06 );
}

void RemoveTeamMember ( unsigned teamid, unsigned guildcard, SERVER* ship )
{
	ship->encryptbuf[0x00] = 0x09;
	ship->encryptbuf[0x01] = 0x03;
	*(unsigned*) &ship->encryptbuf[0x02] = teamid;
	*(unsigned*) &ship->encryptbuf[0x06] = guildcard;
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x0A );
}

void TeamChat ( unsigned short* text, unsigned short chatsize, unsigned teamid, SERVER* ship )
{
	unsigned size;

	ship->encryptbuf[0x00] = 0x09;
	ship->encryptbuf[0x01] = 0x04;
	*(unsigned*) &ship->encryptbuf[0x02] = teamid;
	while (chatsize % 8)
		ship->encryptbuf[6 + (chatsize++)] = 0x00;
	*text = chatsize;
	memcpy (&ship->encryptbuf[0x06], text, chatsize);
	size = chatsize + 6;
	compressShipPacket ( ship, &ship->encryptbuf[0x00], size );
}

void RequestTeamList ( unsigned teamid, unsigned guildcard, SERVER* ship )
{
	ship->encryptbuf[0x00] = 0x09;
	ship->encryptbuf[0x01] = 0x05;
	*(unsigned*) &ship->encryptbuf[0x02] = teamid;
	*(unsigned*) &ship->encryptbuf[0x06] = guildcard;
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x0A );
}

void PromoteTeamMember ( unsigned teamid, unsigned guildcard, unsigned char newlevel, SERVER* ship )
{
	ship->encryptbuf[0x00] = 0x09;
	ship->encryptbuf[0x01] = 0x06;
	*(unsigned*) &ship->encryptbuf[0x02] = teamid;
	*(unsigned*) &ship->encryptbuf[0x06] = guildcard;
	ship->encryptbuf[0x0A] = newlevel;
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x0B );
}

void AddTeamMember ( unsigned teamid, unsigned guildcard, SERVER* ship )
{
	ship->encryptbuf[0x00] = 0x09;
	ship->encryptbuf[0x01] = 0x07;
	*(unsigned*) &ship->encryptbuf[0x02] = teamid;
	*(unsigned*) &ship->encryptbuf[0x06] = guildcard;
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x0A );
}

// Team stuff
void CommandEA (CLIENT* client, SERVER* ship)
{
	unsigned connectNum;

	if ((client->decryptbuf[0x03] < 32) && ((unsigned) servertime - client->team_cooldown[client->decryptbuf[0x03]] >= 1))
	{
		client->team_cooldown[client->decryptbuf[0x03]] = (unsigned) servertime;
		switch (client->decryptbuf[0x03])
		{
		case 0x01:
			// Create team
			if (client->character.teamID == 0)
				CreateTeam ((unsigned short*) &client->decryptbuf[0x0C], client->guildcard, ship);
			break;
		case 0x03:
			// Add a team member
			{
				CLIENT* tClient;
				unsigned gcn, ch;

				if ((client->character.teamID != 0) && (client->character.privilegeLevel >= 0x30))
				{
					gcn = *(unsigned*) &client->decryptbuf[0x08];
					for (ch=0;ch<serverNumConnections;ch++)
					{
						connectNum = serverConnectionList[ch];
						if (connections[connectNum]->guildcard == gcn)
						{
							if ( ( connections[connectNum]->character.teamID == 0 ) && ( connections[connectNum]->teamaccept == 1 ) )
							{
								AddTeamMember ( client->character.teamID, gcn, ship );
								tClient = connections[connectNum];
								tClient->teamaccept = 0;
								memset ( &tClient->character.guildCard2, 0, 2108 );
								tClient->character.teamID = client->character.teamID;
								tClient->character.privilegeLevel = 0;
								tClient->character.unknown15 = client->character.unknown15;
								memcpy ( &tClient->character.teamName[0], &client->character.teamName[0], 28 );
								memcpy ( &tClient->character.teamFlag[0], &client->character.teamFlag[0], 2048 );
								*(long long*) &tClient->character.teamRewards[0] = *(long long*) &client->character.teamRewards[0];
								if ( tClient->lobbyNum < 0x10 )
									SendToLobby ( tClient->lobby, 12, MakePacketEA15 ( tClient ), 2152, 0 );
								else
									SendToLobby ( tClient->lobby, 4, MakePacketEA15 ( tClient ), 2152, 0 );
								SendEA ( 0x12, tClient );
								SendEA ( 0x04, client );
								SendEA ( 0x04, tClient );
								break;
							}
							else
								Send01 ("Player already\nbelongs to a team!", client);
						}
					}
				}
			}
			break;
		case 0x05:
			// Remove member from team
			if (client->character.teamID != 0)
			{
				unsigned gcn,ch;
				CLIENT* tClient;

				gcn = *(unsigned*) &client->decryptbuf[0x08];

				if (gcn != client->guildcard)
				{
					if (client->character.privilegeLevel == 0x40)
					{
						RemoveTeamMember (client->character.teamID, gcn, ship);
						SendEA ( 0x06, client );
						for (ch=0;ch<serverNumConnections;ch++)
						{
							connectNum = serverConnectionList[ch];
							if (connections[connectNum]->guildcard == gcn)
							{
								tClient = connections[connectNum];
								if ( tClient->character.privilegeLevel < client->character.privilegeLevel )
								{
									memset (&tClient->character.guildCard2, 0, 2108);
									memset (&client->encryptbuf[0x00], 0, 0x40);
									client->encryptbuf[0x00] = 0x40;
									client->encryptbuf[0x02] = 0xEA;
									client->encryptbuf[0x03] = 0x12;
									*(unsigned *) &client->encryptbuf[0x0C] = tClient->guildcard;
									if ( tClient->lobbyNum < 0x10 )
									{
										SendToLobby ( tClient->lobby, 12, MakePacketEA15 ( tClient ), 2152, 0 );
										SendToLobby ( tClient->lobby, 12, &client->encryptbuf[0x00], 0x40, 0 );
									}
									else
									{
										SendToLobby ( tClient->lobby, 4, MakePacketEA15 ( tClient ), 2152, 0 );
										SendToLobby ( tClient->lobby, 4, &client->encryptbuf[0x00], 0x40, 0 );
									}
									Send01 ("Member removed.", client);
								}
								else
									Send01 ("Your privilege level is\ntoo low.", client);
								break;
							}
						}
					}
					else
						Send01 ("Your privilege level is\ntoo low.", client);
				}
				else
				{
					RemoveTeamMember ( client->character.teamID, gcn, ship );
					memset (&client->character.guildCard2, 0, 2108);
					memset (&client->encryptbuf[0x00], 0, 0x40);
					client->encryptbuf[0x00] = 0x40;
					client->encryptbuf[0x02] = 0xEA;
					client->encryptbuf[0x03] = 0x12;
					*(unsigned *) &client->encryptbuf[0x0C] = client->guildcard;
					if ( client->lobbyNum < 0x10 )
					{
						SendToLobby ( client->lobby, 12, MakePacketEA15 ( client ), 2152, 0 );
						SendToLobby ( client->lobby, 12, &client->encryptbuf[0x00], 0x40, 0 );
					}
					else
					{
						SendToLobby ( client->lobby, 4, MakePacketEA15 ( client ), 2152, 0 );
						SendToLobby ( client->lobby, 4, &client->encryptbuf[0x00], 0x40, 0 );
					}
				}
			}
			break;
		case 0x07:
			if (client->character.teamID != 0)
			{
				unsigned short size;
				unsigned short *n;

				size = *(unsigned short*) &client->decryptbuf[0x00];

				if (size > 0x2B)
				{
					n = (unsigned short*) &client->decryptbuf[0x2C];
					while (*n != 0x0000)
					{
						if ((*n == 0x0009) || (*n == 0x000A))
							*n = 0x0020;
						n++;
					}
					TeamChat ((unsigned short*) &client->decryptbuf[0x00], size, client->character.teamID, ship);
				}
			}
			break;
		case 0x08:
			// Member Promotion / Demotion / Expulsion / Master Transfer
			//
			if (client->character.teamID != 0)
				RequestTeamList (client->character.teamID, client->guildcard, ship);
			break;
		case 0x0D:
			SendEA (0x0E, client);
			break;
		case 0x0F:
			// Set flag
			if ((client->character.privilegeLevel == 0x40) && (client->character.teamID != 0))
				UpdateTeamFlag (&client->decryptbuf[0x08], client->character.teamID, ship);
			break;
		case 0x10:
			// Dissolve team
			if ((client->character.privilegeLevel == 0x40) && (client->character.teamID != 0))
			{
				DissolveTeam (client->character.teamID, ship);
				SendEA ( 0x10, client );
				memset ( &client->character.guildCard2, 0, 2108 );
				SendToLobby ( client->lobby, 12, MakePacketEA15 ( client ), 2152, 0 );
				SendEA ( 0x12, client );
			}
			break;
		case 0x11:
			// Promote member
			if (client->character.teamID != 0)
			{
				unsigned gcn, ch;
				CLIENT* tClient;

				gcn = *(unsigned*) &client->decryptbuf[0x08];

				if (gcn != client->guildcard)
				{
					if (client->character.privilegeLevel == 0x40)
					{
						PromoteTeamMember (client->character.teamID, gcn, client->decryptbuf[0x04], ship);

						if (client->decryptbuf[0x04] == 0x40)
						{
							// Master Transfer
							PromoteTeamMember (client->character.teamID, client->guildcard, 0x30, ship);
							client->character.privilegeLevel = 0x30;
							SendToLobby ( client->lobby, 12, MakePacketEA15 ( client ), 2152, 0 );
						}

						for (ch=0;ch<serverNumConnections;ch++)
						{
							connectNum = serverConnectionList[ch];
							if (connections[connectNum]->guildcard == gcn)
							{
								tClient = connections[connectNum];
								if (tClient->character.privilegeLevel != client->decryptbuf[0x04]) // only if changed
								{
									tClient->character.privilegeLevel = client->decryptbuf[0x04];
									if ( tClient->lobbyNum < 0x10 )
										SendToLobby ( tClient->lobby, 12, MakePacketEA15 ( tClient ), 2152, 0 );
									else
										SendToLobby ( tClient->lobby, 4, MakePacketEA15 ( tClient ), 2152, 0 );
								}
								SendEA ( 0x12, tClient );
								SendEA ( 0x11, client );
								break;
							}
						}
					}
				}
			}
			break;
		case 0x13:
			// A type of lobby list...
			SendEA (0x13, client);
			break;
		case 0x14:
			// Do nothing.
			break;
		case 0x18:
			// Buying privileges and point information
			SendEA (0x18, client);
			break;
		case 0x19:
			// Privilege list
			SendEA (0x19, client);
			break;
		case 0x1C:
			// Ranking
			Send1A ("Tethealla Ship Server coded by Sodaboy\nhttp://www.pioneer2.net/\n\nEnjoy!", client );
			break;
		case 0x1A:
			SendEA (0x1A, client);
			break;
		default:
			break;
		}
	}
}


void ShowArrows (CLIENT* client, int to_all)
{
	LOBBY *l;
	unsigned ch, total_clients, Packet88Offset;

	total_clients = 0;
	memset (&PacketData[0x00], 0, 8);
	PacketData[0x02] = 0x88;
	PacketData[0x03] = 0x00;
	Packet88Offset = 8;

	if (!client->lobby)
		return;
	l = (LOBBY*) client->lobby;

	for (ch=0;ch<12;ch++)
	{
		if ((l->slot_use[ch] != 0) && (l->client[ch]))
		{
			total_clients++;
			PacketData[Packet88Offset+2] = 0x01;
			*(unsigned*) &PacketData[Packet88Offset+4] = l->client[ch]->character.guildCard;
			PacketData[Packet88Offset+8] = l->arrow_color[ch];
			Packet88Offset += 12;
		}
	}
	*(unsigned*) &PacketData[0x04] = total_clients;
	*(unsigned short*) &PacketData[0x00] = (unsigned short) Packet88Offset;
	if (to_all)
		SendToLobby (client->lobby, 12, &PacketData[0x00], Packet88Offset, 0);
	else
	{
		cipher_ptr = &client->server_cipher;
		encryptcopy (client, &PacketData[0x00], Packet88Offset);
	}
}

void BlockProcessPacket (CLIENT* client)
{
	if (client->guildcard)
	{
		switch (client->decryptbuf[0x02])
		{
		case 0x05:
			break;
		case 0x06:
			if ( ((unsigned) servertime - client->command_cooldown[0x06]) >= 1 )
			{
				client->command_cooldown[0x06] = (unsigned) servertime;
				Send06 (client);
			}
			break;
		case 0x08:
			// Get game list
			if ( ( client->lobbyNum < 0x10 ) && ( ((unsigned) servertime - client->command_cooldown[0x08]) >= 1 ) )
			{
				client->command_cooldown[0x08] = (unsigned) servertime;
				Send08 (client);
			}
			else
				if ( client->lobbyNum < 0x10 )
					Send01 ("You must wait\nawhile before\ntrying that.", client);
			break;
		case 0x09:
			Command09 (client);
			break;
		case 0x10:
			Command10 (1, client);
			break;
		case 0x1D:
			client->response = (unsigned) servertime;
			break;
		case 0x40:
			// Guild Card search
			if ( ((unsigned) servertime - client->command_cooldown[0x40]) >= 1 )
			{
				client->command_cooldown[0x40] = (unsigned) servertime;
				Command40 (client, &logon);
			}
			break;
		case 0x13:
		case 0x44:
			if ( ( client->lobbyNum > 0x0F ) && ( client->sending_quest != -1 ) )
			{
				unsigned short qps;

				if ((client->character.lang < 10) && (quests[client->sending_quest].ql[client->character.lang]))
				{
					if (client->qpos < quests[client->sending_quest].ql[client->character.lang]->qsize)
					{
						qps = *(unsigned short*) &quests[client->sending_quest].ql[client->character.lang]->qdata[client->qpos];
						if ( qps % 8 )
							qps += ( 8 - ( qps % 8 ) );
						cipher_ptr = &client->server_cipher;
						encryptcopy (client, &quests[client->sending_quest].ql[client->character.lang]->qdata[client->qpos], qps);
						client->qpos += qps;
					}
					else
						client->sending_quest = -1;
				}
				else
				{
					if (client->qpos < quests[client->sending_quest].ql[0]->qsize)
					{
						qps = *(unsigned short*) &quests[client->sending_quest].ql[0]->qdata[client->qpos];
						if ( qps % 8 )
							qps += ( 8 - ( qps % 8 ) );
						cipher_ptr = &client->server_cipher;
						encryptcopy (client, &quests[client->sending_quest].ql[0]->qdata[client->qpos], qps);
						client->qpos += qps;
					}
					else
						client->sending_quest = -1;
				}
			}
			break;
		case 0x60:
			if ((client->bursting) && (client->decryptbuf[0x08] == 0x23) && (client->lobbyNum < 0x10) && (client->lobbyNum))
			{
				// If a client has just appeared, send him the team information of everyone in the lobby
				if (client->guildcard)
				{
					unsigned ch;
					LOBBY* l;

					if (!client->lobby)
					break;

					l = client->lobby;

					for (ch=0;ch<12;ch++)
						if ( ( l->slot_use[ch] != 0 ) && ( l->client[ch] ) )
						{
							cipher_ptr = &client->server_cipher;
							encryptcopy (client, MakePacketEA15 (l->client[ch]), 2152 );
						}
						ShowArrows (client, 0);
						client->bursting = 0;
				}
			}
			// Lots of fun commands here.
			Send60 (client);
			break;
		case 0x61:
			memcpy (&client->character.unknown[0], &client->decryptbuf[0x362], 10);
			Send67 (client, client->preferred_lobby);
			break;
		case 0x62:
			Send62 (client);
			break;
		case 0x6D:
			if (client->lobbyNum > 0x0F)
				Send6D (client);
			else
				initialize_connection (client);
			break;
		case 0x6F:
			if ((client->lobbyNum > 0x0F) && (client->bursting))
			{
				LOBBY* l;
				unsigned short fqs,ch;

				if (!client->lobby)
					break;

				l = client->lobby;

				if ((l->inpquest) && (!client->hasquest))
				{
					// Send the quest
					client->bursting = 1;
					if ((client->character.lang < 10) && (quests[l->quest_loaded - 1].ql[client->character.lang]))
					{
						fqs =  *(unsigned short*) &quests[l->quest_loaded - 1].ql[client->character.lang]->qdata[0];
						if (fqs % 8)
							fqs += ( 8 - ( fqs % 8 ) );
						client->sending_quest = l->quest_loaded - 1;
						client->qpos = fqs;
						cipher_ptr = &client->server_cipher;
						encryptcopy ( client, &quests[l->quest_loaded - 1].ql[client->character.lang]->qdata[0], fqs);
					}
					else
					{
						fqs =  *(unsigned short*) &quests[l->quest_loaded - 1].ql[0]->qdata[0];
						if (fqs % 8)
							fqs += ( 8 - ( fqs % 8 ) );
						client->sending_quest = l->quest_loaded - 1;
						client->qpos = fqs;
						cipher_ptr = &client->server_cipher;
						encryptcopy ( client, &quests[l->quest_loaded - 1].ql[0]->qdata[0], fqs);
					}
				}
				else
				{
					// Rare monster data go...
					memset (&client->encryptbuf[0x00], 0, 0x08);
					client->encryptbuf[0x00] = 0x28;
					client->encryptbuf[0x02] = 0xDE;
					memcpy (&client->encryptbuf[0x08], &l->rareData[0], 0x20);
					cipher_ptr = &client->server_cipher;
					encryptcopy (client, &client->encryptbuf[0x00], 0x28);
					memset (&client->encryptbuf[0x00], 0, 0x0C);
					client->encryptbuf[0x00] = 0x0C;
					client->encryptbuf[0x02] = 0x60;
					client->encryptbuf[0x08] = 0x72;
					client->encryptbuf[0x09] = 0x03;
					client->encryptbuf[0x0A] = 0x18;
					client->encryptbuf[0x0B] = 0x08;
					SendToLobby (client->lobby, 4, &client->encryptbuf[0x00], 0x0C, 0);
					for (ch=0;ch<4;ch++)
						if ( ( l->slot_use[ch] != 0 ) && ( l->client[ch] ) )
						{
							cipher_ptr = &client->server_cipher;
							encryptcopy (client, MakePacketEA15 (l->client[ch]), 2152 );
						}
					client->bursting = 0;
				}
			}
			break;
		case 0x81:
			if (client->announce)
			{
				if (client->announce == 1)
					WriteGM ("GM %u made an announcement: %s", client->guildcard, Unicode_to_ASCII ((unsigned short*) &client->decryptbuf[0x60]));
				BroadcastToAll ((unsigned short*) &client->decryptbuf[0x60], client);
			}
			else
			{
				if ( ((unsigned) servertime - client->command_cooldown[0x81]) >= 1 )
				{
					client->command_cooldown[0x81] = (unsigned) servertime;
					Command81 (client, &logon);
				}
			}
			break;
		case 0x84:
			if (client->decryptbuf[0x0C] < 0x0F)
			{
				BLOCK* b;
				b = blocks[client->block - 1];
				if (client->lobbyNum > 0x0F)
				{
					removeClientFromLobby (client);
					client->preferred_lobby = client->decryptbuf[0x0C];
					Send95 (client);
				}
				else
				{
					if ( ((unsigned) servertime - client->command_cooldown[0x84]) >= 1 )
					{
						if (b->lobbies[client->decryptbuf[0x0C]].lobbyCount < 12)
						{
							client->command_cooldown[0x84] = (unsigned) servertime;
							removeClientFromLobby (client);
							client->preferred_lobby = client->decryptbuf[0x0C];
							Send95 (client);
						}
						else
							Send01 ("Lobby is full!", client);
					}
					else 
						Send01 ("You must wait\nawhile before\ntrying that.", client);
				}
			}
			break;
		case 0x89:
			if ((client->lobbyNum < 0x10) && (client->lobbyNum) && ( ((unsigned) servertime - client->command_cooldown[0x89]) >= 1 ) )
			{
				LOBBY* l;

				client->command_cooldown[0x89] = (unsigned) servertime;
				if (!client->lobby)
					break;
				l = client->lobby;
				l->arrow_color[client->clientID] = client->decryptbuf[0x04];
				ShowArrows(client, 1);
			}
			break;
		case 0x8A:
			if (client->lobbyNum > 0x0F)
			{
				LOBBY* l;

				if (!client->lobby)
					break;
				l = client->lobby;
				if (l->in_use)
				{
					memset (&PacketData[0], 0, 0x28);
					PacketData[0x00] = 0x28;
					PacketData[0x02] = 0x8A;
					memcpy (&PacketData[0x08], &l->gameName[0], 30);
					cipher_ptr = &client->server_cipher;
					encryptcopy (client, &PacketData[0], 0x28);
				}
			}
			break;
		case 0xA0:
			// Ship list
			if (client->lobbyNum < 0x10)
				ShipSend0D (0x00, client, &logon);
			break;
		case 0xA1:
			// Block list
			if (client->lobbyNum < 0x10)
				Send07 (client);
			break;
		case 0xA2:
			// Quest list
			if (client->lobbyNum > 0x0F)
			{
				LOBBY* l;

				if (!client->lobby)
					break;
				l = client->lobby;

				if ( l->floor[client->clientID] == 0 )
				{
					if ( client->decryptbuf[0x04] )
						SendA2 ( l->episode, l->oneperson, 0, 1, client );
					else
						SendA2 ( l->episode, l->oneperson, 0, 0, client );
				}
			}
			break;
		case 0xAC:
			// Quest load complete
			if (client->lobbyNum > 0x0F)
			{
				LOBBY* l;
				int all_quest;
				unsigned ch;

				if (!client->lobby)
					break;

				l = client->lobby;

				client->hasquest = 1;
				all_quest = 1;

				for (ch=0;ch<4;ch++)
				{
					if ((l->slot_use[ch]) && (l->client[ch]) && (l->client[ch]->hasquest == 0))
						all_quest = 0;
				}

				if (all_quest)
				{ 
					// Send the 0xAC when all clients have the quest

					client->decryptbuf[0x00] = 0x08;
					client->decryptbuf[0x01] = 0x00;
					SendToLobby (l, 4, &client->decryptbuf[0x00], 8, 0);
				}

				client->sending_quest = -1;

				if ((l->inpquest) && (client->bursting))
				{
					// Let the leader know it's time to send the remaining state of the quest...
					cipher_ptr = &l->client[l->leader]->server_cipher;
					memset (&client->encryptbuf[0], 0, 8);
					client->encryptbuf[0] = 0x08;
					client->encryptbuf[2] = 0xDD;
					client->encryptbuf[4] = client->clientID;
					encryptcopy (l->client[l->leader], &client->encryptbuf[0], 8);
				}
			}
			break;
		case 0xC1:
			// Create game
			if (client->lobbyNum < 0x10)
			{
				if (client->decryptbuf[0x52])
					Send1A ("Challenge games are NOT supported right now.\nCheck back later.\n\n- Sodaboy", client);
				else
							{
								unsigned lNum, failed_to_create;

								failed_to_create = 0;

								if ( (!client->isgm) && (!isLocalGM(client->guildcard)))
								{
									switch (client->decryptbuf[0x53])
									{
									case 0x01:
										if ((client->decryptbuf[0x50] == 0x01) && (client->character.level < 19))
										{
											Send01 ("Episode I\n\nYou must be level\n20 or higher\nto play on the\nhard difficulty.", client);
											failed_to_create = 1;
										}
										else
											if ((client->decryptbuf[0x50] == 0x02) && (client->character.level < 49))
											{
												Send01 ("Episode I\n\nYou must be level\n50 or higher\nto play on the\nvery hard\ndifficulty.", client);
												failed_to_create = 1;
											}
											else
												if ((client->decryptbuf[0x50] == 0x03) && (client->character.level < 89))
												{
													Send01 ("Episode I\n\nYou must be level\n90 or higher\nto play on the\nultimate\ndifficulty.", client);
													failed_to_create = 1;
												}
												break;
									case 0x02:
										if ((client->decryptbuf[0x50] == 0x01) && (client->character.level < 29))
										{
											Send01 ("Episode II\n\nYou must be level\n30 or higher\nto play on the\nhard difficulty.", client);
											failed_to_create = 1;
										}
										else
											if ((client->decryptbuf[0x50] == 0x02) && (client->character.level < 59))
											{
												Send01 ("Episode II\n\nYou must be level\n60 or higher\nto play on the\nvery hard\ndifficulty.", client);
												failed_to_create = 1;
											}
											else
												if ((client->decryptbuf[0x50] == 0x03) && (client->character.level < 99))
												{
													Send01 ("Episode II\n\nYou must be level\n100 or higher\nto play on the\nultimate\ndifficulty.", client);
													failed_to_create = 1;
												}
												break;
									case 0x03:
										if ((client->decryptbuf[0x50] == 0x01) && (client->character.level < 39))
										{
											Send01 ("Episode IV\n\nYou must be level\n40 or higher\nto play on the\nhard difficulty.", client);
											failed_to_create = 1;
										}
										else
											if ((client->decryptbuf[0x50] == 0x02) && (client->character.level < 69))
											{
												Send01 ("Episode IV\n\nYou must be level\n70 or higher\nto play on the\nvery hard\ndifficulty.", client);
												failed_to_create = 1;
											}
											else
												if ((client->decryptbuf[0x50] == 0x03) && (client->character.level < 109))
												{
													Send01 ("Episode IV\n\nYou must be level\n110 or higher\nto play on the\nultimate\ndifficulty.", client);
													failed_to_create = 1;
												}
												break;
									default:
										SendB0 ("Lol, nub.", client);
										break;
									}
								}

								if (!failed_to_create)
								{
									lNum = free_game (client);
									if (lNum)
									{
										removeClientFromLobby (client);
										client->lobbyNum = (unsigned short) lNum + 1;
										client->lobby = &blocks[client->block - 1]->lobbies[lNum];
										initialize_game (client);
										Send64 (client);
										memset (&client->encryptbuf[0x00], 0, 0x0C);
										client->encryptbuf[0x00] = 0x0C;
										client->encryptbuf[0x02] = 0x60;
										client->encryptbuf[0x08] = 0xDD;
										client->encryptbuf[0x09] = 0x03;
										client->encryptbuf[0x0A] = (unsigned char) EXPERIENCE_RATE;
										cipher_ptr = &client->server_cipher;
										encryptcopy (client, &client->encryptbuf[0x00], 0x0C);
										UpdateGameItem (client);
									}
									else
										Send01 ("Sorry, limit of game\ncreation has been\nreached.\n\nPlease join a game\nor change ships.", client);
								}
							}
			}
			break;
		case 0xD8:
			// Show info board
			if ( ((unsigned) servertime - client->command_cooldown[0xD8]) >= 1 )
			{
				client->command_cooldown[0xD8] = (unsigned) servertime;
				CommandD8 (client);
			}
			break;
		case 0xD9:
			// Write on info board
			CommandD9 (client);
			break;
		case 0xE7:
			// Client sending character data...
			if ( client->guildcard )
			{
				if ( (client->isgm) || (isLocalGM(client->guildcard)) )
					WriteGM ("GM %u (%s) has disconnected", client->guildcard, Unicode_to_ASCII ((unsigned short*) &client->character.name[4]) );
				else
					WriteLog ("User %u (%s) has disconnected", client->guildcard, Unicode_to_ASCII ((unsigned short*) &client->character.name[4]) );
				client->todc = 1;
			}
			break;
		case 0xE8:
			// Guild card stuff
			CommandE8 (client);
			break;
		case 0xEA:
			// Team shit
			CommandEA (client, &logon);
			break;
		case 0xED:
			// Set options
			CommandED (client);
			break;
		default:
			break;
		}
	}
	else
	{
		switch (client->decryptbuf[0x02])
		{
			case 0x05:
				printf ("Client has closed the connection.\n");
				client->todc = 1;
				break;
			case 0x93:  // logon packet, p93
			{
				unsigned ch,ch2,ipaddr;
				int banned = 0, match;

				client->temp_guildcard = *(unsigned*) &client->decryptbuf[0x0C];
				client->hwinfo = *(long long*) &client->decryptbuf[0x84];
				ipaddr = *(unsigned*) &client->ipaddr[0];
				for (ch=0;ch<num_bans;ch++)
				{
					if ((ship_bandata[ch].guildcard == client->temp_guildcard) && (ship_bandata[ch].type == 1))
					{
						banned = 1;
						break;
					}
					if ((ship_bandata[ch].ipaddr == ipaddr) && (ship_bandata[ch].type == 2))
					{
						banned = 1;
						break;
					}
					if ((ship_bandata[ch].hwinfo == client->hwinfo) && (ship_bandata[ch].type == 3))
					{
						banned = 1;
						break;
					}
				}

				for (ch=0;ch<num_masks;ch++)
				{
					match = 1;
					for (ch2=0;ch2<4;ch2++)
					{
						if ((ship_banmasks[ch][ch2] != 0x8000) &&
							((unsigned char) ship_banmasks[ch][ch2] != client->ipaddr[ch2]))
							match = 0;
					}
					if ( match )
					{
						banned = 1;
						break;
					}
				}
				
				if (banned)
				{
					Send1A ("You are banned from this ship.", client);
					client->todc = 1;
				}
				else
					if (!client->sendCheck[RECEIVE_PACKET_93])
					{
						ShipSend0B ( client, &logon );
						client->sendCheck[RECEIVE_PACKET_93] = 0x01;
						printf ("Connection accepted.\n");
					}
			}
			break;

			default:
				printf ("Invalid packet.\n");
				break;
		}
	}
}

void ShipProcessPacket (CLIENT* client)
{
	switch (client->decryptbuf[0x02])
	{
		case 0x05:
			printf ("Client has closed the connection.\n");
			client->todc = 1;
			break;
		case 0x10:
			Command10 (0, client);
			break;
		case 0x1D:
			client->response = (unsigned) servertime;
			break;
		case 0x93:
		{
			unsigned ch,ch2,ipaddr;
			int banned = 0, match;

			client->temp_guildcard = *(unsigned*) &client->decryptbuf[0x0C];
			client->hwinfo = *(long long*) &client->decryptbuf[0x84];
			ipaddr = *(unsigned*) &client->ipaddr[0];

			for (ch=0;ch<num_bans;ch++)
			{
				if ((ship_bandata[ch].guildcard == client->temp_guildcard) && (ship_bandata[ch].type == 1))
				{
					banned = 1;
					break;
				}
				if ((ship_bandata[ch].ipaddr == ipaddr) && (ship_bandata[ch].type == 2))
				{
					banned = 1;
					break;
				}
				if ((ship_bandata[ch].hwinfo == client->hwinfo) && (ship_bandata[ch].type == 3))
				{
					banned = 1;
					break;
				}
			}

			for (ch=0;ch<num_masks;ch++)
			{
				match = 1;
				for (ch2=0;ch2<4;ch2++)
				{
					if ((ship_banmasks[ch][ch2] != 0x8000) &&
						((unsigned char) ship_banmasks[ch][ch2] != client->ipaddr[ch2]))
						match = 0;
				}
				if ( match )
				{
					banned = 1;
					break;
				}
			}

			if (banned)
			{
				Send1A ("You are banned from this ship.", client);
				client->todc = 1;
			}
			else
				if (!client->sendCheck[RECEIVE_PACKET_93])
				{
					ShipSend0B ( client, &logon );
					client->sendCheck[RECEIVE_PACKET_93] = 0x01;
				}
		}
		break;
	default:
		break;
	}
}

long CalculateChecksum(void* data,unsigned long size)
{
    long offset,y,cs = 0xFFFFFFFF;
    for (offset = 0; offset < (long)size; offset++)
    {
        cs ^= *(unsigned char*)((long)data + offset);
        for (y = 0; y < 8; y++)
        {
            if (!(cs & 1)) cs = (cs >> 1) & 0x7FFFFFFF;
            else cs = ((cs >> 1) & 0x7FFFFFFF) ^ 0xEDB88320;
        }
    }
    return (cs ^ 0xFFFFFFFF);
}


void LoadBattleParam (BATTLEPARAM* dest, const char* filename, unsigned num_records, long expected_checksum)
{
	FILE* fp;
	long battle_checksum;

	printf ("Loading %s ... ", filename);
	fp = fopen ( filename, "rb");
	if (!fp)
	{
		printf ("%s is missing.\n", filename);
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	if ( ( fread ( dest, 1, sizeof (BATTLEPARAM) * num_records, fp ) != sizeof (BATTLEPARAM) * num_records ) )
	{
		printf ("%s is corrupted.\n", filename);
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	fclose ( fp );

	printf ("OK!\n");

	battle_checksum = CalculateChecksum (dest, sizeof (BATTLEPARAM) * num_records);

	if ( battle_checksum != expected_checksum )
	{
		printf ("Checksum of file: %08x\n", battle_checksum );
		printf ("WARNING: Battle parameter file has been modified.\n");
	}
}
