void SortClientItems (CLIENT* client)
{
	unsigned ch, ch2, ch3, ch4, itemid;

	ch2 = 0x0C;

	memset (&sort_data[0], 0, sizeof (INVENTORY_ITEM) * 30);

	for (ch4=0;ch4<30;ch4++)
	{
		sort_data[ch4].item.data[1] = 0xFF;
		sort_data[ch4].item.itemid = 0xFFFFFFFF;
	}

	ch4 = 0;

	for (ch=0;ch<30;ch++)
	{
		itemid = *(unsigned *) &client->decryptbuf[ch2];
		ch2 += 4;
		if (itemid != 0xFFFFFFFF)
		{
			for (ch3=0;ch3<client->character.inventoryUse;ch3++)
			{
				if ((client->character.inventory[ch3].in_use) && (client->character.inventory[ch3].item.itemid == itemid))
				{
					sort_data[ch4++] = client->character.inventory[ch3];
					break;
				}
			}
		}
	}

	for (ch=0;ch<30;ch++)
		client->character.inventory[ch] = sort_data[ch];

}

void CleanUpBank (CLIENT* client)
{
	unsigned ch, ch2 = 0;

	memset (&bank_data[0], 0, sizeof (BANK_ITEM) * 200);

	for (ch2=0;ch2<200;ch2++)
		bank_data[ch2].itemid = 0xFFFFFFFF;

	ch2 = 0;

	for (ch=0;ch<200;ch++)
	{
		if (client->character.bankInventory[ch].itemid != 0xFFFFFFFF)
			bank_data[ch2++] = client->character.bankInventory[ch];
	}

	client->character.bankUse = ch2;

	for (ch=0;ch<200;ch++)
		client->character.bankInventory[ch] = bank_data[ch];

}

void CleanUpInventory (CLIENT* client)
{
	unsigned ch, ch2 = 0;

	memset (&sort_data[0], 0, sizeof (INVENTORY_ITEM) * 30);

	ch2 = 0;

	for (ch=0;ch<30;ch++)
	{
		if (client->character.inventory[ch].in_use)
			sort_data[ch2++] = client->character.inventory[ch];
	}

	client->character.inventoryUse = ch2;

	for (ch=0;ch<30;ch++)
		client->character.inventory[ch] = sort_data[ch];
}

void CleanUpGameInventory (LOBBY* l)
{
	unsigned ch, item_count;

	ch = item_count = 0;

	while (ch < l->gameItemCount)
	{
		// Combs the entire game inventory for items in use
		if (l->gameItemList[ch] != 0xFFFFFFFF)
		{
			if (ch > item_count)
				l->gameItemList[item_count] = l->gameItemList[ch];
			item_count++;
		}
		ch++;
	}

	if (item_count < MAX_SAVED_ITEMS)
		memset (&l->gameItemList[item_count], 0xFF, ( ( MAX_SAVED_ITEMS - item_count ) * 4 ) );

	l->gameItemCount = item_count;
}

unsigned AddItemToClient (unsigned itemid, CLIENT* client)
{
	unsigned ch, itemNum = 0;
	int found_item = -1;
	unsigned char stackable = 0;
	unsigned count, stack_count;
	unsigned compare_item1 = 0;
	unsigned compare_item2 = 0;
	unsigned item_added = 0;
	LOBBY* l;

	// Adds an item to the client's character data, but only if the item exists in the game item data
	// to begin with.

	if (!client->lobby)
		return 0;

	l = (LOBBY*) client->lobby;

	for (ch=0;ch<l->gameItemCount;ch++)
	{
		itemNum = l->gameItemList[ch]; // Lookup table for faster searching...
		if ( l->gameItem[itemNum].item.itemid == itemid )
		{
			if (l->gameItem[itemNum].item.data[0] == 0x04)
			{
				// Meseta
				count = *(unsigned *) &l->gameItem[itemNum].item.data2[0];
				client->character.meseta += count;
				if (client->character.meseta > 999999)
					client->character.meseta = 999999;
				item_added = 1;
			}
			else
				if (l->gameItem[itemNum].item.data[0] == 0x03)
					stackable = stackable_table[l->gameItem[itemNum].item.data[1]];
			if ( ( stackable ) && ( !l->gameItem[itemNum].item.data[5] ) )
				l->gameItem[itemNum].item.data[5] = 1;
			found_item = ch;
			break;
		}
	}

	if ( found_item != -1 ) // We won't disconnect if the item isn't found because there's a possibility another
	{						// person may have nabbed it before our client due to lag...
		if ( ( item_added == 0 ) && ( stackable ) )
		{
			memcpy (&compare_item1, &l->gameItem[itemNum].item.data[0], 3);
			for (ch=0;ch<client->character.inventoryUse;ch++)
			{
				memcpy (&compare_item2, &client->character.inventory[ch].item.data[0], 3);
				if (compare_item1 == compare_item2)
				{
					count = l->gameItem[itemNum].item.data[5];

					stack_count = client->character.inventory[ch].item.data[5];
					if ( !stack_count )
						stack_count = 1;

					if ( ( stack_count + count ) > stackable )
					{
						Send1A ("Trying to stack over the limit...", client);
						client->todc = 1;
					}
					else
					{
						// Add item to the client's current count...
						client->character.inventory[ch].item.data[5] = (unsigned char) ( stack_count + count );
						item_added = 1;
					}
					break;
				}
			}
		}

		if ( ( !client->todc ) && ( item_added == 0 ) ) // Make sure the client isn't trying to pick up more than 30 items...
		{
			if ( client->character.inventoryUse >= 30 )
			{
				Send1A ("Inventory limit reached.", client);
				client->todc = 1;
			}
			else
			{
				// Give item to client...
				client->character.inventory[client->character.inventoryUse].in_use = 0x01;
				client->character.inventory[client->character.inventoryUse].flags = 0x00;
				memcpy (&client->character.inventory[client->character.inventoryUse].item, &l->gameItem[itemNum].item, sizeof (ITEM));
				client->character.inventoryUse++;
				item_added = 1;
			}
		}

		if ( item_added )
		{
			// Delete item from game's inventory
			memset (&l->gameItem[itemNum], 0, sizeof (GAME_ITEM));
			l->gameItemList[found_item] = 0xFFFFFFFF;
			CleanUpGameInventory(l);
		}
	}
	return item_added;
}

void DeleteMesetaFromClient (unsigned count, unsigned drop, CLIENT* client)
{
	unsigned stack_count, newItemNum;
	LOBBY* l;

	if (!client->lobby)
		return;

	l = (LOBBY*) client->lobby;

	stack_count = client->character.meseta;
	if (stack_count < count)
	{
		client->character.meseta = 0;
		count = stack_count;
	}
	else
		client->character.meseta -= count;
	if ( drop )
	{
		memset (&PacketData[0x00], 0, 16);
		PacketData[0x00] = 0x2C;
		PacketData[0x01] = 0x00;
		PacketData[0x02] = 0x60;
		PacketData[0x03] = 0x00;
		PacketData[0x08] = 0x5D;
		PacketData[0x09] = 0x09;
		PacketData[0x0A] = client->clientID;
		*(unsigned *) &PacketData[0x0C] = client->drop_area;
		*(long long *) &PacketData[0x10] = client->drop_coords;
		PacketData[0x18] = 0x04;
		PacketData[0x19] = 0x00;
		PacketData[0x1A] = 0x00;
		PacketData[0x1B] = 0x00;
		memset (&PacketData[0x1C], 0, 0x08);
		*(unsigned *) &PacketData[0x24] = l->playerItemID[client->clientID];
		*(unsigned *) &PacketData[0x28] = count;
		SendToLobby (client->lobby, 4, &PacketData[0], 0x2C, 0);

		// Generate new game item...

		newItemNum = free_game_item (l);
		if (l->gameItemCount < MAX_SAVED_ITEMS)
			l->gameItemList[l->gameItemCount++] = newItemNum;
		memcpy (&l->gameItem[newItemNum].item.data[0], &PacketData[0x18], 12);
		*(unsigned *) &l->gameItem[newItemNum].item.data2[0] = count;
		l->gameItem[newItemNum].item.itemid = l->playerItemID[client->clientID];
		l->playerItemID[client->clientID]++;
	}
}


void SendItemToEnd (unsigned itemid, CLIENT* client)
{
	unsigned ch;
	INVENTORY_ITEM i;

	for (ch=0;ch<client->character.inventoryUse;ch++)
	{
		if (client->character.inventory[ch].item.itemid == itemid)
		{
			i = client->character.inventory[ch];
			i.flags = 0x00;
			client->character.inventory[ch].in_use = 0;
			break;
		}
	}

	CleanUpInventory (client);
	
	// Add item to client.

	client->character.inventory[client->character.inventoryUse] = i;
	client->character.inventoryUse++;
}


void DeleteItemFromClient (unsigned itemid, unsigned count, unsigned drop, CLIENT* client)
{
	unsigned ch, ch2, itemNum;
	int found_item = -1;
	LOBBY* l;
	unsigned char stackable = 0;
	unsigned delete_item = 0;
	unsigned stack_count;

	// Deletes an item from the client's character data.

	if (!client->lobby)
		return;

	l = (LOBBY*) client->lobby;

	for (ch=0;ch<client->character.inventoryUse;ch++)
	{
		if (client->character.inventory[ch].item.itemid == itemid)
		{
			if (client->character.inventory[ch].item.data[0] == 0x03)
			{
				stackable = stackable_table[client->character.inventory[ch].item.data[1]];
				if ( ( stackable ) && ( !count ) && ( !drop ) )
					count = 1;
			}

			if ( ( stackable ) && ( count ) )
			{
				stack_count = client->character.inventory[ch].item.data[5];
				if (!stack_count)
					stack_count = 1;

				if ( stack_count < count )
				{
					Send1A ("Trying to delete more items than posssessed!", client);
					client->todc = 1;
				}
				else
				{
					stack_count -= count;
					client->character.inventory[ch].item.data[5] = (unsigned char) stack_count;

					if ( !stack_count )
						delete_item = 1;

					if ( drop )
					{
						memset (&PacketData[0x00], 0, 16);
						PacketData[0x00] = 0x28;
						PacketData[0x01] = 0x00;
						PacketData[0x02] = 0x60;
						PacketData[0x03] = 0x00;
						PacketData[0x08] = 0x5D;
						PacketData[0x09] = 0x08;
						PacketData[0x0A] = client->clientID;
						*(unsigned *) &PacketData[0x0C] = client->drop_area;
						*(long long *) &PacketData[0x10] = client->drop_coords;
						memcpy (&PacketData[0x18], &client->character.inventory[ch].item.data[0], 12);
						PacketData[0x1D] = (unsigned char) count;
						*(unsigned *) &PacketData[0x24] = l->playerItemID[client->clientID];

						SendToLobby (client->lobby, 4, &PacketData[0], 0x28, 0);

						// Generate new game item...

						itemNum = free_game_item (l);
						if (l->gameItemCount < MAX_SAVED_ITEMS)
							l->gameItemList[l->gameItemCount++] = itemNum;
						memcpy (&l->gameItem[itemNum].item.data[0], &PacketData[0x18], 12);
						l->gameItem[itemNum].item.itemid =  l->playerItemID[client->clientID];
						l->playerItemID[client->clientID]++;
					}
				}
			}
			else
			{
				delete_item = 1; // Not splitting a stack, item goes byebye from character's inventory.
				if ( drop ) // Client dropped the item on the floor?
				{
					// Copy to game's inventory
					itemNum = free_game_item (l);
					if (l->gameItemCount < MAX_SAVED_ITEMS)
						l->gameItemList[l->gameItemCount++] = itemNum;
					memcpy (&l->gameItem[itemNum].item, &client->character.inventory[ch].item, sizeof (ITEM));
				}
			}

			if ( delete_item )
			{
				if (client->character.inventory[ch].item.data[0] == 0x01)
				{
					if ((client->character.inventory[ch].item.data[1] == 0x01) &&
						(client->character.inventory[ch].flags & 0x08)) // equipped armor, remove slot items
					{
						for (ch2=0;ch2<client->character.inventoryUse;ch2++)
							if ((client->character.inventory[ch2].item.data[0] == 0x01) && 
								(client->character.inventory[ch2].item.data[1] != 0x02) &&
								(client->character.inventory[ch2].flags & 0x08))
							{
								client->character.inventory[ch2].item.data[4] = 0x00;
								client->character.inventory[ch2].flags &= ~(0x08);
							}
					}
				}
				client->character.inventory[ch].in_use = 0;
			}
			found_item = ch;
			break;
		}
	}

	if ( found_item == -1 )
	{
		Send1A ("Could not find item to delete.", client);
		client->todc = 1;
	}
	else
		CleanUpInventory (client);

}

unsigned WithdrawFromBank (unsigned itemid, unsigned count, CLIENT* client)
{
	unsigned ch;
	int found_item = -1;
	unsigned char stackable = 0;
	unsigned stack_count;
	unsigned compare_item1 = 0;
	unsigned compare_item2 = 0;
	unsigned item_added = 0;
	unsigned delete_item = 0;
	LOBBY* l;

	// Adds an item to the client's character from it's bank, if the item is really there...

	if (!client->lobby)
		return 0;

	l = (LOBBY*) client->lobby;

	for (ch=0;ch<client->character.bankUse;ch++)
	{
		if ( client->character.bankInventory[ch].itemid == itemid )
		{
			found_item = ch;
			if ( client->character.bankInventory[ch].data[0] == 0x03 )
			{
				stackable = stackable_table[client->character.bankInventory[ch].data[1]];

				if ( stackable )
				{
					if ( !count )
						count = 1;

					stack_count = ( client->character.bankInventory[ch].bank_count & 0xFF );
					if ( !stack_count )
						stack_count = 1;				

					if ( stack_count < count ) // Naughty!
					{
						Send1A ("Trying to pull a fast one on the bank teller.", client);
						client->todc = 1;
						found_item = -1;
					}
					else
					{
						stack_count -= count;
						client->character.bankInventory[ch].bank_count = 0x10000 + stack_count;
						if ( !stack_count )
							delete_item = 1;
					}
				}
			}
			break;
		}
	}

	if ( found_item != -1 )
	{
		if ( stackable )
		{
			memcpy (&compare_item1, &client->character.bankInventory[found_item].data[0], 3);
			for (ch=0;ch<client->character.inventoryUse;ch++)
			{
				memcpy (&compare_item2, &client->character.inventory[ch].item.data[0], 3);
				if (compare_item1 == compare_item2)
				{
					stack_count = client->character.inventory[ch].item.data[5];
					if (!stack_count)
						stack_count = 1;
					if ( ( stack_count + count ) > stackable )
					{
						count = stackable - stack_count;
						client->character.inventory[ch].item.data[5] = stackable;
					}
					else
						client->character.inventory[ch].item.data[5] = (unsigned char) (stack_count + count);
					item_added = 1;
					break;
				}
			}
		}

		if ( (!client->todc ) && ( item_added == 0 ) ) // Make sure the client isn't trying to withdraw more than 30 items...
		{
			if ( client->character.inventoryUse >= 30 )
			{
				Send1A ("Inventory limit reached.", client);
				client->todc = 1;
			}
			else
			{
				// Give item to client...
				client->character.inventory[client->character.inventoryUse].in_use = 0x01;
				client->character.inventory[client->character.inventoryUse].flags = 0x00;
				memcpy (&client->character.inventory[client->character.inventoryUse].item, &client->character.bankInventory[found_item].data[0], sizeof (ITEM));
				if ( stackable )
				{
					memset (&client->character.inventory[client->character.inventoryUse].item.data[4], 0, 4);
					client->character.inventory[client->character.inventoryUse].item.data[5] = (unsigned char) count;
				}
				client->character.inventory[client->character.inventoryUse].item.itemid = l->itemID;
				client->character.inventoryUse++;
				item_added = 1;
				//debug ("Item added to client...");
			}
		}

		if ( item_added )
		{
			// Let people know the client has a new toy...
			memset (&client->encryptbuf[0x00], 0, 0x24);
			client->encryptbuf[0x00] = 0x24;
			client->encryptbuf[0x02] = 0x60;
			client->encryptbuf[0x08] = 0xBE;
			client->encryptbuf[0x09] = 0x09;
			client->encryptbuf[0x0A] = client->clientID;
			memcpy (&client->encryptbuf[0x0C], &client->character.bankInventory[found_item].data[0], 12);
			*(unsigned *) &client->encryptbuf[0x18] = l->itemID;
			l->itemID++;
			if (!stackable)
				*(unsigned *) &client->encryptbuf[0x1C] = *(unsigned *) &client->character.bankInventory[found_item].data2[0];
			else
				client->encryptbuf[0x11] = count;
			memset (&client->encryptbuf[0x20], 0, 4);
			SendToLobby ( client->lobby, 4, &client->encryptbuf[0x00], 0x24, 0 );
			if ( ( delete_item ) || ( !stackable ) )
				// Delete item from bank inventory
				client->character.bankInventory[found_item].itemid = 0xFFFFFFFF;
		}
		CleanUpBank (client);
	}
	else
	{
		Send1A ("Could not find bank item to withdraw.", client);
		client->todc = 1;
	}

	return item_added;
}

void SortBankItems (CLIENT* client)
{
	unsigned ch, ch2;
	unsigned compare_item1 = 0;
	unsigned compare_item2 = 0;
	unsigned char swap_c;
	BANK_ITEM swap_item;
	BANK_ITEM b1;
	BANK_ITEM b2;

	if ( client->character.bankUse > 1 )
	{
		for ( ch=0;ch<client->character.bankUse - 1;ch++ )
		{
			memcpy (&b1, &client->character.bankInventory[ch], sizeof (BANK_ITEM));
			swap_c     = b1.data[0];
			b1.data[0] = b1.data[2];
			b1.data[2] = swap_c;
			memcpy (&compare_item1, &b1.data[0], 3);
			for ( ch2=ch+1;ch2<client->character.bankUse;ch2++ )
			{
				memcpy (&b2, &client->character.bankInventory[ch2], sizeof (BANK_ITEM));
				swap_c     = b2.data[0];
				b2.data[0] = b2.data[2];
				b2.data[2] = swap_c;
				memcpy (&compare_item2, &b2.data[0], 3);
				if (compare_item2 < compare_item1) // compare_item2 should take compare_item1's place
				{
					memcpy (&swap_item, &client->character.bankInventory[ch], sizeof (BANK_ITEM));
					memcpy (&client->character.bankInventory[ch], &client->character.bankInventory[ch2], sizeof (BANK_ITEM));
					memcpy (&client->character.bankInventory[ch2], &swap_item, sizeof (BANK_ITEM));
					memcpy (&compare_item1, &compare_item2, 3);
				}
			}
		}
	}
}

void DepositIntoBank (unsigned itemid, unsigned count, CLIENT* client)
{
	unsigned ch, ch2;
	int found_item = -1;
	LOBBY* l;
	unsigned char stackable = 0;
	unsigned compare_item1 = 0;
	unsigned compare_item2 = 0;
	unsigned deposit_item = 0, deposit_done = 0;
	unsigned delete_item = 0;
	unsigned stack_count;

	// Moves an item from the client's character to it's bank.

	if (!client->lobby)
		return;

	l = (LOBBY*) client->lobby;

	for (ch=0;ch<client->character.inventoryUse;ch++)
	{
		if (client->character.inventory[ch].item.itemid == itemid)
		{
			if (client->character.inventory[ch].item.data[0] == 0x03)
				stackable = stackable_table[client->character.inventory[ch].item.data[1]];

			if ( stackable )
			{
				if (!count)
					count = 1;

				stack_count = client->character.inventory[ch].item.data[5];

				if (!stack_count)
					stack_count = 1;

				if ( stack_count < count )
				{
					Send1A ("Trying to deposit more items than in possession.", client);
					client->todc = 1; // Tried to deposit more than had?
				}
				else
				{
					deposit_item = 1;

					stack_count -= count;
					client->character.inventory[ch].item.data[5] = (unsigned char) stack_count;

					if ( !stack_count )
						delete_item = 1;
				}
			}
			else
			{
				// Not stackable, remove from client completely.
				deposit_item = 1;
				delete_item = 1;
			}

			if ( deposit_item )
			{
				if ( stackable )
				{
					memcpy (&compare_item1, &client->character.inventory[ch].item.data[0], 3);
					for (ch2=0;ch2<client->character.bankUse;ch2++)
					{
						memcpy (&compare_item2, &client->character.bankInventory[ch2].data[0], 3);
						if (compare_item1 == compare_item2)
						{
							stack_count = ( client->character.bankInventory[ch2].bank_count & 0xFF );
							if ( ( stack_count + count ) > stackable )
							{
								count = stackable - stack_count;
								client->character.bankInventory[ch2].bank_count = 0x10000 + stackable;
							}
							else
								client->character.bankInventory[ch2].bank_count += count;
							deposit_done = 1;
							break;
						}
					}
				}

				if ( ( !client->todc ) && ( !deposit_done ) )
				{
					if (client->character.inventory[ch].item.data[0] == 0x01)
					{
						if ((client->character.inventory[ch].item.data[1] == 0x01) &&
							(client->character.inventory[ch].flags & 0x08)) // equipped armor, remove slot items
						{
							for (ch2=0;ch2<client->character.inventoryUse;ch2++)
								if ((client->character.inventory[ch2].item.data[0] == 0x01) && 
									(client->character.inventory[ch2].item.data[1] != 0x02) &&
									(client->character.inventory[ch2].flags & 0x08))
								{
									client->character.inventory[ch2].flags &= ~(0x08);
									client->character.inventory[ch2].item.data[4] = 0x00;
								}
						}
					}

					memcpy (&client->character.bankInventory[client->character.bankUse].data[0],
						&client->character.inventory[ch].item.data[0],
						sizeof (ITEM));

					if ( stackable )
					{
						memset ( &client->character.bankInventory[client->character.bankUse].data[4], 0, 4 );
						client->character.bankInventory[client->character.bankUse].bank_count = 0x10000 + count;
					}
					else
						client->character.bankInventory[client->character.bankUse].bank_count = 0x10001;

					client->character.bankInventory[client->character.bankUse].itemid = client->character.inventory[ch].item.itemid; // for now
					client->character.bankUse++;
				}

				if ( delete_item )
					client->character.inventory[ch].in_use = 0;
			}
			found_item = ch;
			break;
		}
	}

	if ( found_item == -1 )
	{
		Send1A ("Could not find item to deposit.", client);
		client->todc = 1;
	}
	else
		CleanUpInventory (client);
}

void DeleteFromInventory (INVENTORY_ITEM* i, unsigned count, CLIENT* client)
{
	unsigned ch, ch2;
	int found_item = -1;
	LOBBY* l;
	unsigned char stackable = 0;
	unsigned delete_item = 0;
	unsigned stack_count;
	unsigned compare_item1 = 0;
	unsigned compare_item2 = 0;
	unsigned compare_id;

	// Deletes an item from the client's character data.

	if (!client->lobby)
		return;

	l = (LOBBY*) client->lobby;

	memcpy (&compare_item1, &i->item.data[0], 3);
	if (i->item.itemid)
		compare_id = i->item.itemid;
	for (ch=0;ch<client->character.inventoryUse;ch++)
	{
		memcpy (&compare_item2, &client->character.inventory[ch].item.data[0], 3);
		if (!i->item.itemid)
			compare_id = client->character.inventory[ch].item.itemid;
		if ((compare_item1 == compare_item2) && (compare_id == client->character.inventory[ch].item.itemid)) // Found the item?
		{
			if (client->character.inventory[ch].item.data[0] == 0x03)
				stackable = stackable_table[client->character.inventory[ch].item.data[1]];

			if ( stackable )
			{
				if ( !count )
					count = 1;

				stack_count = client->character.inventory[ch].item.data[5];
				if ( !stack_count )
					stack_count = 1;

				if ( stack_count < count )
					count = stack_count;

				stack_count -= count;

				client->character.inventory[ch].item.data[5] = (unsigned char) stack_count;

				if (!stack_count)
					delete_item = 1;
			}
			else
				delete_item = 1;

			memset (&client->encryptbuf[0x00], 0, 0x14);
			client->encryptbuf[0x00] = 0x14;
			client->encryptbuf[0x02] = 0x60;
			client->encryptbuf[0x08] = 0x29;
			client->encryptbuf[0x09] = 0x05;
			client->encryptbuf[0x0A] = client->clientID;
			*(unsigned *) &client->encryptbuf[0x0C] =  client->character.inventory[ch].item.itemid;
			client->encryptbuf[0x10] = (unsigned char) count;

			SendToLobby (l, 4, &client->encryptbuf[0x00], 0x14, 0);

			if ( delete_item )
			{
				if (client->character.inventory[ch].item.data[0] == 0x01)
				{
					if ((client->character.inventory[ch].item.data[1] == 0x01) &&
						(client->character.inventory[ch].flags & 0x08)) // equipped armor, remove slot items
					{
						for (ch2=0;ch2<client->character.inventoryUse;ch2++)
							if ((client->character.inventory[ch2].item.data[0] == 0x01) && 
								(client->character.inventory[ch2].item.data[1] != 0x02) &&
								(client->character.inventory[ch2].flags & 0x08))
							{
								client->character.inventory[ch2].item.data[4] = 0x00;
								client->character.inventory[ch2].flags &= ~(0x08);
							}
					}
				}
				client->character.inventory[ch].in_use = 0;
			}
			found_item = ch;
			break;
		}
	}
	if ( found_item == -1 )
	{
		Send1A ("Could not find item to delete from inventory.", client);
		client->todc = 1;
	}
	else
		CleanUpInventory (client);

}

unsigned AddToInventory (INVENTORY_ITEM* i, unsigned count, int shop, CLIENT* client)
{
	unsigned ch;
	unsigned char stackable = 0;
	unsigned stack_count;
	unsigned compare_item1 = 0;
	unsigned compare_item2 = 0;
	unsigned item_added = 0;
	unsigned notsend;
	LOBBY* l;

	// Adds an item to the client's inventory... (out of thin air)
	// The new itemid must already be set to i->item.itemid

	if (!client->lobby)
		return 0;

	l = (LOBBY*) client->lobby;

	if (i->item.data[0] == 0x04)
	{
		// Meseta
		count = *(unsigned *) &i->item.data2[0];
		client->character.meseta += count;
		if (client->character.meseta > 999999)
			client->character.meseta = 999999;
		item_added = 1;
	}
	else
	{
		if ( i->item.data[0] == 0x03 )
			stackable = stackable_table [i->item.data[1]];
	}

	if ( ( !client->todc ) && ( !item_added ) )
	{
		if ( stackable )
		{
			if (!count)
				count = 1;
			memcpy (&compare_item1, &i->item.data[0], 3);
			for (ch=0;ch<client->character.inventoryUse;ch++)
			{
				memcpy (&compare_item2, &client->character.inventory[ch].item.data[0], 3);
				if (compare_item1 == compare_item2)
				{
					stack_count = client->character.inventory[ch].item.data[5];
					if (!stack_count)
						stack_count = 1;
					if ( ( stack_count + count ) > stackable )
					{
						count = stackable - stack_count;
						client->character.inventory[ch].item.data[5] = stackable;
					}
					else
						client->character.inventory[ch].item.data[5] = (unsigned char) ( stack_count + count );
					item_added = 1;
					break;
				}
			}
		}

		if ( item_added == 0 ) // Make sure we don't go over the max inventory
		{
			if ( client->character.inventoryUse >= 30 )
			{
				Send1A ("Inventory limit reached.", client);
				client->todc = 1;
			}
			else
			{
				// Give item to client...
				client->character.inventory[client->character.inventoryUse].in_use = 0x01;
				client->character.inventory[client->character.inventoryUse].flags = 0x00;
				memcpy (&client->character.inventory[client->character.inventoryUse].item, &i->item, sizeof (ITEM));
				if ( stackable )
				{
					memset (&client->character.inventory[client->character.inventoryUse].item.data[4], 0, 4);
					client->character.inventory[client->character.inventoryUse].item.data[5] = (unsigned char) count;
				}
				client->character.inventoryUse++;
				item_added = 1;
			}
		}
	}

	if ((!client->todc) && ( item_added ) )
	{
		// Let people know the client has a new toy...
		memset (&client->encryptbuf[0x00], 0, 0x24);
		client->encryptbuf[0x00] = 0x24;
		client->encryptbuf[0x02] = 0x60;
		client->encryptbuf[0x08] = 0xBE;
		client->encryptbuf[0x09] = 0x09;
		client->encryptbuf[0x0A] = client->clientID;
		memcpy (&client->encryptbuf[0x0C], &i->item.data[0], 12);
		*(unsigned *) &client->encryptbuf[0x18] = i->item.itemid;
		if ((!stackable) || (i->item.data[0] == 0x04))
			*(unsigned *) &client->encryptbuf[0x1C] = *(unsigned *) &i->item.data2[0];
		else
			client->encryptbuf[0x11] = count;
		memset (&client->encryptbuf[0x20], 0, 4);
		if ( shop )
			notsend = client->guildcard;
		else
			notsend = 0;
		SendToLobby ( client->lobby, 4, &client->encryptbuf[0x00], 0x24, notsend );
	}
	return item_added;
}

/* Request char data from server or send char data to server
 * (when not using a temp char).
 */
void ShipSend04 (unsigned char command, CLIENT* client, SERVER* ship)
{
	//unsigned ch;

	ship->encryptbuf[0x00] = 0x04;
	switch (command)
	{
	case 0x00:
		// Request character data from server
		ship->encryptbuf[0x01] = 0x00;
		*(unsigned *) &ship->encryptbuf[0x02] = client->guildcard;
		*(unsigned short *) &ship->encryptbuf[0x06] = (unsigned short) client->slotnum;
		*(int *) &ship->encryptbuf[0x08] = client->plySockfd;
		*(unsigned *) &ship->encryptbuf[0x0C] = serverID;
		compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x10 );
		break;
	case 0x02:
		// Send character data to server when not using a temporary character.
		if ((!client->mode) && (client->gotchardata == 1))
		{
			ship->encryptbuf[0x01] = 0x02;
			*(unsigned *) &ship->encryptbuf[0x02] = client->guildcard;
			*(unsigned short*) &ship->encryptbuf[0x06] = (unsigned short) client->slotnum;
			memcpy (&ship->encryptbuf[0x08], &client->character.packetSize, sizeof (CHARDATA));
			// Include character bank in packet
			memcpy (&ship->encryptbuf[0x08+0x700], &client->char_bank, sizeof (BANK));
			// Include common bank in packet
			memcpy (&ship->encryptbuf[0x08+sizeof(CHARDATA)], &client->common_bank, sizeof (BANK));
			compressShipPacket ( ship, &ship->encryptbuf[0x00], sizeof(BANK) + sizeof(CHARDATA) + 8 );
		}
		break;
	}
}

void ShipSend0E (SERVER* ship)
{
	if (logon_ready)
	{
		ship->encryptbuf[0x00] = 0x0E;
		ship->encryptbuf[0x01] = 0x00;
		*(unsigned *) &ship->encryptbuf[0x02] = serverID;
		*(unsigned *) &ship->encryptbuf[0x06] = serverNumConnections;
		compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x0A );
	}
}

void ShipSend0D (unsigned char command, CLIENT* client, SERVER* ship)
{
	ship->encryptbuf[0x00] = 0x0D;
	switch (command)
	{
	case 0x00:
		// Requesting ship list.
		ship->encryptbuf[0x01] = 0x00;
		*(int *) &ship->encryptbuf[0x02]= client->plySockfd;
		compressShipPacket ( ship, &ship->encryptbuf[0x00], 6 );
		break;
	default:
		break;
	}
}

void ShipSend0B (CLIENT* client, SERVER* ship)
{
	ship->encryptbuf[0x00] = 0x0B;
	ship->encryptbuf[0x01] = 0x00;
	*(unsigned *) &ship->encryptbuf[0x02] = *(unsigned *) &client->decryptbuf[0x0C];
	*(unsigned *) &ship->encryptbuf[0x06] = *(unsigned *) &client->decryptbuf[0x18];
	*(long long *) &ship->encryptbuf[0x0A] = *(long long*) &client->decryptbuf[0x8C];
	*(long long *) &ship->encryptbuf[0x12] = *(long long*) &client->decryptbuf[0x94];
	*(long long *) &ship->encryptbuf[0x1A] = *(long long*) &client->decryptbuf[0x9C];
	*(long long *) &ship->encryptbuf[0x22] = *(long long*) &client->decryptbuf[0xA4];
	*(long long *) &ship->encryptbuf[0x2A] = *(long long*) &client->decryptbuf[0xAC];
	compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x32 );
}

void FixItem (ITEM* i )
{
	unsigned ch3;

	if (i->data[0] == 2) // Mag
	{
		MAG* m;
		short mDefense, mPower, mDex, mMind;
		int total_levels;

		m = (MAG*) &i->data[0];

		if ( m->synchro > 120 )
			m->synchro = 120;

		if ( m->synchro < 0 )
			m->synchro = 0;

		if ( m->IQ > 200 )
			m->IQ = 200;

		if ( ( m->defense < 0 ) || ( m->power < 0 ) || ( m->dex < 0 ) || ( m->mind < 0 ) )
			total_levels = 201; // Auto fail if any stat is under 0...
		else
		{
			mDefense = m->defense / 100;
			mPower = m->power / 100;
			mDex = m->dex / 100;
			mMind = m->mind / 100;
			total_levels = mDefense + mPower + mDex + mMind;
		}

		if ( ( total_levels > 200 ) || ( m->level > 200 ) )
		{
			// Mag fails IRL, initialize it
			m->defense = 500;
			m->power = 0;
			m->dex = 0;
			m->mind = 0;
			m->level = 5;
			m->blasts = 0;
			m->IQ = 0;
			m->synchro = 20;
			m->mtype = 0;
			m->PBflags = 0;
		}
	}

	if (i->data[0] == 1) // Normalize Armor & Barriers
	{
		switch (i->data[1])
		{
		case 0x01:
			if (i->data[6] > armor_dfpvar_table[ i->data[2] ])
				i->data[6] = armor_dfpvar_table[ i->data[2] ];
			if (i->data[8] > armor_evpvar_table[ i->data[2] ])
				i->data[8] = armor_evpvar_table[ i->data[2] ];
			break;
		case 0x02:
			if (i->data[6] > barrier_dfpvar_table[ i->data[2] ])
				i->data[6] = barrier_dfpvar_table[ i->data[2] ];
			if (i->data[8] > barrier_evpvar_table[ i->data[2] ])
				i->data[8] = barrier_evpvar_table[ i->data[2] ];
			break;
		}
	}

	if (i->data[0] == 0) // Weapon
	{
		signed char percent_table[6];
		signed char percent;
		unsigned max_percents, num_percents;
		int srank;

		if ( ( i->data[1] == 0x33 ) ||  // SJS & Lame max 2 percents
			 ( i->data[1] == 0xAB ) )
			max_percents = 2;
		else
			max_percents = 3;

		srank = 0;
		memset (&percent_table[0], 0, 6);
		num_percents = 0;

		for (ch3=6;ch3<=4+(max_percents*2);ch3+=2)
		{
			if ( i->data[ch3] & 128 )
			{
				srank = 1; // S-Rank
				break; 
			}

			if ( ( i->data[ch3] ) &&
				( i->data[ch3] < 0x06 ) )
			{
				// Percents over 100 or under -100 get set to 0
				percent = (char) i->data[ch3+1];
				if ( ( percent > 100 ) || ( percent < -100 ) )
					percent = 0;
				// Save percent
				percent_table[i->data[ch3]] = 
					percent;
			}
		}

		if (!srank)
		{
			for (ch3=6;ch3<=4+(max_percents*2);ch3+=2)
			{
				// Reset all %s
				i->data[ch3]   = 0;
				i->data[ch3+1] = 0;
			}

			for (ch3=1;ch3<=5;ch3++)
			{
				// Rebuild %s
				if ( percent_table[ch3] )
				{
					i->data[6 + ( num_percents * 2 )] = ch3;
					i->data[7 + ( num_percents * 2 )] = (unsigned char) percent_table[ch3];
					num_percents ++;
					if ( num_percents == max_percents )
						break;
				}
			}
		}
	}
}

const char lobbyString[] = { "L\0o\0b\0b\0y\0 \0" };

void LogonProcessPacket (SERVER* ship)
{
	unsigned gcn, ch, ch2, connectNum;
	unsigned char episode, part;
	unsigned mob_rate;
	long long mob_calc;
	
	switch (ship->decryptbuf[0x04])
	{
		case 0x00:
		// Server has sent it's welcome packet.  Start encryption and send ship info...
		memcpy (&ship->user_key[0], &RC4publicKey[0], 32);
		ch2 = 0;
		for (ch=0x1C;ch<0x5C;ch+=2)
		{
			ship->key_change [ch2+(ship->decryptbuf[ch] % 4)] = ship->decryptbuf[ch+1];
			ch2 += 4;
		}
		prepare_key(&ship->user_key[0], 32, &ship->cs_key);
		prepare_key(&ship->user_key[0], 32, &ship->sc_key);
		ship->crypt_on = 1;
		memcpy (&ship->encryptbuf[0x00], &ship->decryptbuf[0x04], 0x28);
		memcpy (&ship->encryptbuf[0x00], &ShipPacket00[0x00], 0x10); // Yep! :)
		ship->encryptbuf[0x00] = 1;
		memcpy (&ship->encryptbuf[0x28], &Ship_Name[0], 12 );
		*(unsigned *) &ship->encryptbuf[0x34] = serverNumConnections;
		*(unsigned *) &ship->encryptbuf[0x38] = *(unsigned *) &serverIP[0];
		*(unsigned short*) &ship->encryptbuf[0x3C] = (unsigned short) serverPort;
		*(unsigned *) &ship->encryptbuf[0x3E] = shop_checksum;
		*(unsigned *) &ship->encryptbuf[0x42] = ship_index;
		memcpy (&ship->encryptbuf[0x46], &ship_key[0], 32);
		compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x66 );
		break;
		case 0x02:
		// Server's result of our authentication packet.
		if (ship->decryptbuf[0x05] != 0x01)
		{
			switch (ship->decryptbuf[0x05])
			{
				case 0x00:
				printf ("This ship's version is incompatible with the login server.\n");
				printf ("Press [ENTER] to quit...");
				reveal_window;
				gets (&dp[0]);
				exit (1);
				break;
				case 0x02:
				printf ("This ship's IP address is already registered with the logon server.\n");
				printf ("The IP address cannot be registered twice.  Retry in %u seconds...\n", LOGIN_RECONNECT_SECONDS);
				reveal_window;
				break;
				case 0x03:
				printf ("This ship did not pass the connection test the login server ran on it.\n");
				printf ("Please be sure the IP address specified in ship.ini is correct, your\n");
				printf ("firewall has ship_serv.exe on allow.  If behind a router, please be\n");
				printf ("sure your ports are forwarded.  Retry in %u seconds...\n", LOGIN_RECONNECT_SECONDS);
				reveal_window;
				break;
				case 0x04:
				printf ("Please do not modify any data not instructed to when connecting to this\n");
				printf ("login server...\n");
				printf ("Press [ENTER] to quit...");
				reveal_window;
				gets (&dp[0]);
				exit (1);
				break;
				case 0x05:
				printf ("Your ship_key.bin file seems to be invalid.\n");
				printf ("Press [ENTER] to quit...");
				reveal_window;
				gets (&dp[0]);
				exit (1);
				break;
				case 0x06:
				printf ("Your ship key appears to already be in use!\n");
				printf ("Press [ENTER] to quit...");
				reveal_window;
				gets (&dp[0]);
				exit (1);
				break;
			}
			initialize_logon();
		}
		else
		{
			serverID = *(unsigned *) &ship->decryptbuf[0x06];
			if (serverIP[0] == 0x00)
			{
				*(unsigned *) &serverIP[0] = *(unsigned *) &ship->decryptbuf[0x0A];
				printf ("Updated IP address to %u.%u.%u.%u\n", serverIP[0], serverIP[1], serverIP[2], serverIP[3]);
			}
			serverID++;
			if (serverID != 0xFFFFFFFF)
			{
				printf ("Ship has successfully registered with the login server!!! Ship ID: %u\n", serverID );
				printf ("Constructing Block List packet...\n\n");
				ConstructBlockPacket();
				printf ("Load quest allowance...\n");
				quest_numallows = *(unsigned *) &ship->decryptbuf[0x0E];
				if ( quest_allow )
					free ( quest_allow );
				quest_allow = malloc ( quest_numallows * 4  );
				memcpy ( quest_allow, &ship->decryptbuf[0x12], quest_numallows * 4 );
				printf ("Quest allowance item count: %u\n\n", quest_numallows );
				normalName = *(unsigned *) &ship->decryptbuf[0x12 + ( quest_numallows * 4 )];
				localName = *(unsigned *) &ship->decryptbuf[0x16 + ( quest_numallows * 4 )];
				globalName = *(unsigned *) &ship->decryptbuf[0x1A + ( quest_numallows * 4 )];
				memcpy (&ship->user_key[0], &ship_key[0], 128 ); // 1024-bit key
				
				// Change keys
				
				for (ch2=0;ch2<128;ch2++)
					if (ship->key_change[ch2] != -1)
						ship->user_key[ch2] = (unsigned char) ship->key_change[ch2]; // update the key
					
				prepare_key(&ship->user_key[0], sizeof(ship->user_key), &ship->cs_key);
				prepare_key(&ship->user_key[0], sizeof(ship->user_key), &ship->sc_key);
				memset ( &ship->encryptbuf[0x00], 0, 8 );
				ship->encryptbuf[0x00] = 0x0F;
				ship->encryptbuf[0x01] = 0x00;
				printf ( "Requesting drop charts from server...\n");
				compressShipPacket ( ship, &ship->encryptbuf[0x00], 4 );
			}
			else
			{
				printf ("The ship has failed authentication to the logon server.  Retry in %u seconds...\n", LOGIN_RECONNECT_SECONDS);
				initialize_logon();
			}
		}
		break;
		case 0x03:
		// Reserved
		break;
		case 0x04:
		switch (ship->decryptbuf[0x05])
		{
			case 0x01:
			{
				// Receive and store full player data here.
				//
				CLIENT* client;
				unsigned guildcard,ch,ch2,eq_weapon,eq_armor,eq_shield,eq_mag;
				int sockfd;
				unsigned short baseATP, baseMST, baseEVP, baseHP, baseDFP, baseATA;
				unsigned char* cd;
				
				guildcard = *(unsigned *) &ship->decryptbuf[0x06];
				sockfd = *(int *) &ship->decryptbuf[0x0C];
				
				for (ch=0;ch<serverNumConnections;ch++)
				{
					connectNum = serverConnectionList[ch];
					if ((connections[connectNum]->plySockfd == sockfd) && (connections[connectNum]->guildcard == guildcard))
					{
						client = connections[connectNum];
						client->gotchardata = 1;
						memcpy (&client->character.packetSize, &ship->decryptbuf[0x10], sizeof (CHARDATA));
						
						/* Set up copies of the banks */
						
						memcpy (&client->char_bank, &client->character.bankUse, sizeof (BANK));
						memcpy (&client->common_bank, &ship->decryptbuf[0x10+sizeof(CHARDATA)], sizeof (BANK));

						cipher_ptr = &client->server_cipher;
						if (client->isgm == 1)
							*(unsigned *) &client->character.nameColorBlue = globalName;
						else
							if (isLocalGM(client->guildcard))
								*(unsigned *) &client->character.nameColorBlue = localName;
							else
								*(unsigned *) &client->character.nameColorBlue = normalName;

						if (client->character.inventoryUse > 30)
							client->character.inventoryUse = 30;

						client->equip_flags = 0;
						switch (client->character._class)
						{
						case CLASS_HUMAR:
							client->equip_flags |= HUNTER_FLAG;
							client->equip_flags |= HUMAN_FLAG;
							client->equip_flags |= MALE_FLAG;
							break;
						case CLASS_HUNEWEARL:
							client->equip_flags |= HUNTER_FLAG;
							client->equip_flags |= NEWMAN_FLAG;
							client->equip_flags |= FEMALE_FLAG;
							break;
						case CLASS_HUCAST:
							client->equip_flags |= HUNTER_FLAG;
							client->equip_flags |= DROID_FLAG;
							client->equip_flags |= MALE_FLAG;
							break;
						case CLASS_HUCASEAL:
							client->equip_flags |= HUNTER_FLAG;
							client->equip_flags |= DROID_FLAG;
							client->equip_flags |= FEMALE_FLAG;
							break;
						case CLASS_RAMAR:
							client->equip_flags |= RANGER_FLAG;
							client->equip_flags |= HUMAN_FLAG;
							client->equip_flags |= MALE_FLAG;
							break;
						case CLASS_RACAST:
							client->equip_flags |= RANGER_FLAG;
							client->equip_flags |= DROID_FLAG;
							client->equip_flags |= MALE_FLAG;
							break;
						case CLASS_RACASEAL:
							client->equip_flags |= RANGER_FLAG;
							client->equip_flags |= DROID_FLAG;
							client->equip_flags |= FEMALE_FLAG;
							break;
						case CLASS_RAMARL:
							client->equip_flags |= RANGER_FLAG;
							client->equip_flags |= HUMAN_FLAG;
							client->equip_flags |= FEMALE_FLAG;
							break;
						case CLASS_FONEWM:
							client->equip_flags |= FORCE_FLAG;
							client->equip_flags |= NEWMAN_FLAG;
							client->equip_flags |= MALE_FLAG;
							break;
						case CLASS_FONEWEARL:
							client->equip_flags |= FORCE_FLAG;
							client->equip_flags |= NEWMAN_FLAG;
							client->equip_flags |= FEMALE_FLAG;
							break;
						case CLASS_FOMARL:
							client->equip_flags |= FORCE_FLAG;
							client->equip_flags |= HUMAN_FLAG;
							client->equip_flags |= FEMALE_FLAG;
							break;
						case CLASS_FOMAR:
							client->equip_flags |= FORCE_FLAG;
							client->equip_flags |= HUMAN_FLAG;
							client->equip_flags |= MALE_FLAG;
							break;
						}

						// Let's fix hacked mags and weapons

						for (ch2=0;ch2<client->character.inventoryUse;ch2++)
						{
							if (client->character.inventory[ch2].in_use)
								FixItem ( &client->character.inventory[ch2].item );
						}

						// Fix up equipped weapon, armor, shield, and mag equipment information

						eq_weapon = 0;
						eq_armor = 0;
						eq_shield = 0;
						eq_mag = 0;

						for (ch2=0;ch2<client->character.inventoryUse;ch2++)
						{
							if (client->character.inventory[ch2].flags & 0x08)
							{
								switch (client->character.inventory[ch2].item.data[0])
								{
								case 0x00:
									eq_weapon++;
									break;
								case 0x01:
									switch (client->character.inventory[ch2].item.data[1])
									{
									case 0x01:
										eq_armor++;
										break;
									case 0x02:
										eq_shield++;
										break;
									}
									break;
								case 0x02:
									eq_mag++;
									break;
								}
							}
						}

						if (eq_weapon > 1)
						{
							for (ch2=0;ch2<client->character.inventoryUse;ch2++)
							{
								// Unequip all weapons when there is more than one equipped.
								if ((client->character.inventory[ch2].item.data[0] == 0x00) &&
									(client->character.inventory[ch2].flags & 0x08))
									client->character.inventory[ch2].flags &= ~(0x08);
							}

						}

						if (eq_armor > 1)
						{
							for (ch2=0;ch2<client->character.inventoryUse;ch2++)
							{
								// Unequip all armor and slot items when there is more than one armor equipped.
								if ((client->character.inventory[ch2].item.data[0] == 0x01) &&
									(client->character.inventory[ch2].item.data[1] != 0x02) &&
									(client->character.inventory[ch2].flags & 0x08))
								{
									client->character.inventory[ch2].item.data[3] = 0x00;
									client->character.inventory[ch2].flags &= ~(0x08);
								}
							}
						}

						if (eq_shield > 1)
						{
							for (ch2=0;ch2<client->character.inventoryUse;ch2++)
							{
								// Unequip all shields when there is more than one equipped.
								if ((client->character.inventory[ch2].item.data[0] == 0x01) &&
									(client->character.inventory[ch2].item.data[1] == 0x02) &&
									(client->character.inventory[ch2].flags & 0x08))
								{
									client->character.inventory[ch2].item.data[3] = 0x00;
									client->character.inventory[ch2].flags &= ~(0x08);
								}
							}
						}

						if (eq_mag > 1)
						{
							for (ch2=0;ch2<client->character.inventoryUse;ch2++)
							{
								// Unequip all mags when there is more than one equipped.
								if ((client->character.inventory[ch2].item.data[0] == 0x02) &&
									(client->character.inventory[ch2].flags & 0x08))
									client->character.inventory[ch2].flags &= ~(0x08);
							}
						}

						for (ch2=0;ch2<client->character.bankUse;ch2++)
							FixItem ( (ITEM*) &client->character.bankInventory[ch2] );

						baseATP = *(unsigned short*) &startingData[(client->character._class*14)];
						baseMST = *(unsigned short*) &startingData[(client->character._class*14)+2];
						baseEVP = *(unsigned short*) &startingData[(client->character._class*14)+4];
						baseHP  = *(unsigned short*) &startingData[(client->character._class*14)+6];
						baseDFP = *(unsigned short*) &startingData[(client->character._class*14)+8];
						baseATA = *(unsigned short*) &startingData[(client->character._class*14)+10];

						for (ch2=0;ch2<client->character.level;ch2++)
						{
							baseATP += playerLevelData[client->character._class][ch2].ATP;
							baseMST += playerLevelData[client->character._class][ch2].MST;
							baseEVP += playerLevelData[client->character._class][ch2].EVP;
							baseHP  += playerLevelData[client->character._class][ch2].HP;
							baseDFP += playerLevelData[client->character._class][ch2].DFP;
							baseATA += playerLevelData[client->character._class][ch2].ATA;
						}

						client->matuse[0] = ( client->character.ATP - baseATP ) / 2;
						client->matuse[1] = ( client->character.MST - baseMST ) / 2;
						client->matuse[2] = ( client->character.EVP - baseEVP ) / 2;
						client->matuse[3] = ( client->character.DFP - baseDFP ) / 2;
						client->matuse[4] = ( client->character.LCK - 10 ) / 2;

						//client->character.lang = 0x00;

						cd = (unsigned char*) &client->character.packetSize;

						cd[(8*28)+0x0F]  = client->matuse[0];
						cd[(9*28)+0x0F]  = client->matuse[1];
						cd[(10*28)+0x0F] = client->matuse[2];
						cd[(11*28)+0x0F] = client->matuse[3];
						cd[(12*28)+0x0F] = client->matuse[4];

						encryptcopy (client, (unsigned char*) &client->character.packetSize, sizeof (CHARDATA) );
						client->preferred_lobby = 0xFF;

						cd[(8*28)+0x0F]  = 0x00; // Clear this stuff out to not mess up our item procedures.
						cd[(9*28)+0x0F]  = 0x00;
						cd[(10*28)+0x0F] = 0x00;
						cd[(11*28)+0x0F] = 0x00;
						cd[(12*28)+0x0F] = 0x00;

						for (ch2=0;ch2<MAX_SAVED_LOBBIES;ch2++)
						{
							if (savedlobbies[ch2].guildcard == client->guildcard)
							{
								client->preferred_lobby = savedlobbies[ch2].lobby - 1;
								savedlobbies[ch2].guildcard = 0;
								break;
							}
						}

						Send95 (client);

						if ( (client->isgm) || (isLocalGM(client->guildcard)) )
							WriteGM ("GM %u (%s) has connected", client->guildcard, Unicode_to_ASCII ((unsigned short*) &client->character.name[4]));
						else
							WriteLog ("User %u (%s) has connected", client->guildcard, Unicode_to_ASCII ((unsigned short*) &client->character.name[4]));
						break;
					}
				}
			}
			break;
		case 0x03:
		{
				unsigned guildcard;
				CLIENT* client;
				
				guildcard = *(unsigned *) &ship->decryptbuf[0x06];

				for (ch=0;ch<serverNumConnections;ch++)
				{
					connectNum = serverConnectionList[ch];
					if ((connections[connectNum]->guildcard == guildcard) && (connections[connectNum]->released == 1))
					{
						// Let the released client roam free...!
						client = connections[connectNum];
						Send19 (client->releaseIP[0], client->releaseIP[1], client->releaseIP[2], client->releaseIP[3], 
							client->releasePort, client);
						break;
					}
				}
			}
		}
		break;
	case 0x05:
		// Reserved
		break;
	case 0x06:
		// Reserved
		break;
	case 0x07:
		// Card database full.
		gcn = *(unsigned *) &ship->decryptbuf[0x06];

		for (ch=0;ch<serverNumConnections;ch++)
		{
			connectNum = serverConnectionList[ch];
			if (connections[connectNum]->guildcard == gcn)
			{
				Send1A ("Your guild card database on the server is full.\n\nYou were unable to accept the guild card.\n\nPlease delete some cards.  (40 max)", connections[connectNum]);
				break;
			}
		}
		break;
	case 0x08:
		switch (ship->decryptbuf[0x05])
		{
		case 0x00:
			// ???
			{
				gcn = *(unsigned *) &ship->decryptbuf[0x06];
				for (ch=0;ch<serverNumConnections;ch++)
				{
					connectNum = serverConnectionList[ch];
					if (connections[connectNum]->guildcard == gcn)
					{
						Send1A ("This account has just logged on.\n\nYou are now being disconnected.", connections[connectNum]);
						connections[connectNum]->todc = 1;
						break;
					}
				}
			}
			break;
		case 0x01:
			// Guild card search
			{
				// Someone's doing a guild card search...   Check to see if that guild card is on our ship...

				unsigned client_gcn, ch2;
				unsigned char *n;
				unsigned char *c;
				unsigned short blockPort;

				gcn = *(unsigned *) &ship->decryptbuf[0x06];
				client_gcn = *(unsigned *) &ship->decryptbuf[0x0A];

				// requesting ship ID @ 0x0E

				for (ch=0;ch<serverNumConnections;ch++)
				{
					connectNum = serverConnectionList[ch];
					if ((connections[connectNum]->guildcard == gcn) && (connections[connectNum]->lobbyNum))
					{
						if (connections[connectNum]->lobbyNum < 0x10)
							for (ch2=0;ch2<MAX_SAVED_LOBBIES;ch2++)
							{
								if (savedlobbies[ch2].guildcard == 0)
								{
									savedlobbies[ch2].guildcard = client_gcn;
									savedlobbies[ch2].lobby = connections[connectNum]->lobbyNum;
									break;
								}
							}
						ship->encryptbuf[0x00] = 0x08;
						ship->encryptbuf[0x01] = 0x02;
						*(unsigned *) &ship->encryptbuf[0x02] = serverID;
						*(unsigned *) &ship->encryptbuf[0x06] = *(unsigned *) &ship->decryptbuf[0x0E];
						// 0x10 = 41 result packet
						memset (&ship->encryptbuf[0x0A], 0, 0x136);
						ship->encryptbuf[0x10] = 0x30;
						ship->encryptbuf[0x11] = 0x01;
						ship->encryptbuf[0x12] = 0x41;
						ship->encryptbuf[0x1A] = 0x01;
						*(unsigned *) &ship->encryptbuf[0x1C] = client_gcn;
						*(unsigned *) &ship->encryptbuf[0x20] = gcn;
						ship->encryptbuf[0x24] = 0x10;
						ship->encryptbuf[0x26] = 0x19;
						*(unsigned *) &ship->encryptbuf[0x2C] = *(unsigned *) &serverIP[0];
						blockPort = serverPort + connections[connectNum]->block;
						*(unsigned short *) &ship->encryptbuf[0x30] = (unsigned short) blockPort;					
						memcpy (&ship->encryptbuf[0x34], &lobbyString[0], 12 );						
						if ( connections[connectNum]->lobbyNum < 0x10 )
						{
							if ( connections[connectNum]->lobbyNum < 10 )
							{
								ship->encryptbuf[0x40] = 0x30;
								ship->encryptbuf[0x42] = 0x30 + connections[connectNum]->lobbyNum;
							}
							else
							{
								ship->encryptbuf[0x40] = 0x31;
								ship->encryptbuf[0x42] = 0x26 + connections[connectNum]->lobbyNum;
							}
						}
						else
						{
								ship->encryptbuf[0x40] = 0x30;
								ship->encryptbuf[0x42] = 0x31;
						}
						ship->encryptbuf[0x44] = 0x2C;
						memcpy ( &ship->encryptbuf[0x46], &blockString[0], 10 );
						if ( connections[connectNum]->block < 10 )
						{
							ship->encryptbuf[0x50] = 0x30;
							ship->encryptbuf[0x52] = 0x30 + connections[connectNum]->block;
						}
						else
						{
							ship->encryptbuf[0x50] = 0x31;
							ship->encryptbuf[0x52] = 0x26 + connections[connectNum]->block;
						}

						ship->encryptbuf[0x54] = 0x2C;
						if (serverID < 10)
						{
							ship->encryptbuf[0x56] = 0x30;
							ship->encryptbuf[0x58] = 0x30 + serverID;
						}
						else
						{
							ship->encryptbuf[0x56] = 0x30 + ( serverID / 10 );
							ship->encryptbuf[0x58] = 0x30 + ( serverID % 10 );
						}
						ship->encryptbuf[0x5A] = 0x3A;
						n = (unsigned char*) &ship->encryptbuf[0x5C];
						c = (unsigned char*) &Ship_Name[0];
						while (*c != 0x00)
						{
							*(n++) = *(c++);
							n++;
						}
						if ( connections[connectNum]->lobbyNum < 0x10 )
						ship->encryptbuf[0xBC] = (unsigned char) connections[connectNum]->lobbyNum; else
						ship->encryptbuf[0xBC] = 0x01;
						ship->encryptbuf[0xBE] = 0x1A;
						memcpy (&ship->encryptbuf[0x100], &connections[connectNum]->character.name[0], 24);
						compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x140 );
						break;
					}
				}
			}
			break;
		case 0x02:
			// Send guild result to user
			{
				gcn = *(unsigned *) &ship->decryptbuf[0x20];

				// requesting ship ID @ 0x0E

				for (ch=0;ch<serverNumConnections;ch++)
				{
					connectNum = serverConnectionList[ch];
					if (connections[connectNum]->guildcard == gcn)
					{
						cipher_ptr = &connections[connectNum]->server_cipher;
						encryptcopy (connections[connectNum], &ship->decryptbuf[0x14], 0x130);
						break;
					}
				}
			}
			break;
		case 0x03:
			// Send mail to user
			{
				gcn = *(unsigned *) &ship->decryptbuf[0x36];

				// requesting ship ID @ 0x0E

				for (ch=0;ch<serverNumConnections;ch++)
				{
					connectNum = serverConnectionList[ch];
					if (connections[connectNum]->guildcard == gcn)
					{
						cipher_ptr = &connections[connectNum]->server_cipher;
						encryptcopy (connections[connectNum], &ship->decryptbuf[0x06], 0x45C);
						break;
					}
				}
			}
			break;
		default:
			break;
		}
		break;
	case 0x09:
		// Reserved for team functions.
		switch (ship->decryptbuf[0x05])
		{
			CLIENT* client;
			unsigned char CreateResult;

		case 0x00:
			CreateResult = ship->decryptbuf[0x06];
			gcn = *(unsigned *) &ship->decryptbuf[0x07];
			for (ch=0;ch<serverNumConnections;ch++)
			{
				connectNum = serverConnectionList[ch];
				if (connections[connectNum]->guildcard == gcn)
				{
					client = connections[connectNum];
					switch (CreateResult)
					{
					case 0x00:
						// All good!!!
						client->character.teamID = *(unsigned *) &ship->decryptbuf[0x823];
						memcpy (&client->character.teamFlag[0], &ship->decryptbuf[0x0B], 0x800);
						client->character.privilegeLevel = 0x40;
						client->character.unknown15 = 0x00986C84; // ??
						client->character.teamName[0] = 0x09;
						client->character.teamName[2] = 0x45;
						client->character.privilegeLevel = 0x40;
						memcpy (&client->character.teamName[4], &ship->decryptbuf[0x80B], 24);
						SendEA (0x02, client);
						SendToLobby ( client->lobby, 12, MakePacketEA15 ( client ), 2152, 0 );
						SendEA (0x12, client);
						SendEA (0x1D, client);
						break;
					case 0x01:
						Send1A ("The server failed to create the team due to a MySQL error.\n\nPlease contact the server administrator.", client);
						break;
					case 0x02:
						Send01 ("Cannot create team\nbecause team\n already exists!!!", client);
						break;
					case 0x03:
						Send01 ("Cannot create team\nbecause you are\nalready in a team!", client);
						break;
					}
					break;
				}
			}
			break;
		case 0x01:
			// Flag updated
			{
				unsigned teamid;
				CLIENT* tClient;

				teamid = *(unsigned *) &ship->decryptbuf[0x07];

				for (ch=0;ch<serverNumConnections;ch++)
				{
					connectNum = serverConnectionList[ch];
					if ((connections[connectNum]->guildcard != 0) && (connections[connectNum]->character.teamID == teamid))
					{
						tClient = connections[connectNum];
						memcpy ( &tClient->character.teamFlag[0], &ship->decryptbuf[0x0B], 0x800);
						SendToLobby ( tClient->lobby, 12, MakePacketEA15 ( tClient ), 2152, 0 );
					}
				}
			}
			break;
		case 0x02:
			// Team dissolved
			{
				unsigned teamid;
				CLIENT* tClient;

				teamid = *(unsigned *) &ship->decryptbuf[0x07];

				for (ch=0;ch<serverNumConnections;ch++)
				{
					connectNum = serverConnectionList[ch];
					if ((connections[connectNum]->guildcard != 0) && (connections[connectNum]->character.teamID == teamid))
					{
						tClient = connections[connectNum];
						memset ( &tClient->character.guildCard2, 0, 2108 );
						SendToLobby ( tClient->lobby, 12, MakePacketEA15 ( tClient ), 2152, 0 );
						SendEA ( 0x12, tClient );
					}
				}
			}
			break;
		case 0x04:
			// Team chat
			{
				unsigned teamid, size;
				CLIENT* tClient;

				size = *(unsigned *) &ship->decryptbuf[0x00];
				size -= 10;

				teamid = *(unsigned *) &ship->decryptbuf[0x06];

				for (ch=0;ch<serverNumConnections;ch++)
				{
					connectNum = serverConnectionList[ch];
					if ((connections[connectNum]->guildcard != 0) && (connections[connectNum]->character.teamID == teamid))
					{
						tClient = connections[connectNum];
						cipher_ptr = &tClient->server_cipher;
						encryptcopy ( tClient, &ship->decryptbuf[0x0A], size );
					}
				}
			}
			break;
		case 0x05:
			// Request Team List
			{
				unsigned gcn;
				unsigned short size;
				CLIENT* tClient;

				gcn = *(unsigned *) &ship->decryptbuf[0x0A];
				size = *(unsigned short*) &ship->decryptbuf[0x0E];

				for (ch=0;ch<serverNumConnections;ch++)
				{
					connectNum = serverConnectionList[ch];
					if (connections[connectNum]->guildcard == gcn)
					{					
						tClient = connections[connectNum];
						cipher_ptr = &tClient->server_cipher;
						encryptcopy (tClient, &ship->decryptbuf[0x0E], size);
						break;
					}
				}
			}
			break;
		}
		break;
	case 0x0A:
		// Reserved
		break;
	case 0x0B:
		// Player authentication result from the logon server.
		gcn = *(unsigned *) &ship->decryptbuf[0x06];
		if (ship->decryptbuf[0x05] == 0)
		{
			CLIENT* client;

			// Finish up the logon process here.

			for (ch=0;ch<serverNumConnections;ch++)
			{
				connectNum = serverConnectionList[ch];
				if (connections[connectNum]->temp_guildcard == gcn)
				{
					client = connections[connectNum];
					client->slotnum = ship->decryptbuf[0x0A];
					client->isgm = ship->decryptbuf[0x0B];
					memcpy (&client->encryptbuf[0], &PacketE6[0], sizeof (PacketE6));
					*(unsigned *) &client->encryptbuf[0x10] = gcn;
					client->guildcard = gcn;
					*(unsigned *) &client->encryptbuf[0x14] = *(unsigned*) &ship->decryptbuf[0x0C];
					*(long long *) &client->encryptbuf[0x38] = *(long long*) &ship->decryptbuf[0x10];
					if (client->decryptbuf[0x16] < 0x05)
					{
						Send1A ("Client/Server synchronization error.", client);
						client->todc = 1;
					}
					else
					{
						cipher_ptr = &client->server_cipher;
						encryptcopy (client, &client->encryptbuf[0], sizeof (PacketE6));
						client->lastTick = (unsigned) servertime;
						if (client->block == 0)
						{
							if (logon.sockfd >= 0)
								Send07(client);
							else
							{
								Send1A("This ship has unfortunately lost it's connection with the logon server...\nData cannot be saved.\n\nPlease reconnect later.", client);
								client->todc = 1;
							}
						}
						else
						{
							blocks[client->block - 1]->count++;
							// Request E7 information from server...
							Send83(client); // Lobby data
							ShipSend04 (0x00, client, &logon);
						}
					}
					break;
				}
			}
		}
		else
		{
			// Deny connection here.
			for (ch=0;ch<serverNumConnections;ch++)
			{
				connectNum = serverConnectionList[ch];
				if (connections[connectNum]->temp_guildcard == gcn)
				{
					Send1A ("Security violation.", connections[connectNum]);
					connections[connectNum]->todc = 1;
					break;
				}
			}
		}
		break;
	case 0x0D:
		// 00 = Request ship list
		// 01 = Ship list data (include IP addresses)
		switch (ship->decryptbuf[0x05])
		{
			case 0x01:
				{
					unsigned char ch;
					int sockfd;
					unsigned short pOffset;

					// Retrieved ship list data.  Send to client...

					sockfd = *(int *) &ship->decryptbuf[0x06];
					pOffset = *(unsigned short *) &ship->decryptbuf[0x0A];
					memcpy (&PacketA0Data[0x00], &ship->decryptbuf[0x0A], pOffset);
					pOffset += 0x0A;

					totalShips = 0;

					for (ch=0;ch<PacketA0Data[0x04];ch++)
					{
						shipdata[ch].shipID = *(unsigned *) &ship->decryptbuf[pOffset];
						pOffset +=4;
						*(unsigned *) &shipdata[ch].ipaddr[0] = *(unsigned *) &ship->decryptbuf[pOffset];
						pOffset +=4;
						shipdata[ch].port = *(unsigned short *) &ship->decryptbuf[pOffset];
						pOffset +=2;
						totalShips++;
					}

					for (ch=0;ch<serverNumConnections;ch++)
					{
						connectNum = serverConnectionList[ch];
						if (connections[connectNum]->plySockfd == sockfd)
						{
							SendA0 (connections[connectNum]);
							break;
						}
					}
				}
				break;
			default:
				break;
		}
		break;
	case 0x0F:
		// Receiving drop chart
		episode = ship->decryptbuf[0x05];
		part = ship->decryptbuf[0x06];
		if ( ship->decryptbuf[0x06] == 0 )
			printf ("Received drop chart from login server...\n");
		switch ( episode )
		{
		case 0x01:
			if ( part == 0 )
				printf ("Episode I ..." );
			else
				printf (" OK!\n");
			memcpy ( &rt_tables_ep1[(sizeof(rt_tables_ep1) >> 3) * part], &ship->decryptbuf[0x07], sizeof (rt_tables_ep1) >> 1 );
			break;
		case 0x02:
			if ( part == 0 )
				printf ("Episode II ..." );
			else
				printf (" OK!\n");
			memcpy ( &rt_tables_ep2[(sizeof(rt_tables_ep2) >> 3) * part], &ship->decryptbuf[0x07], sizeof (rt_tables_ep2) >> 1 );
			break;
		case 0x03:
			if ( part == 0 )
				printf ("Episode IV ..." );
			else
				printf (" OK!\n");
			memcpy ( &rt_tables_ep4[(sizeof(rt_tables_ep4) >> 3) * part], &ship->decryptbuf[0x07], sizeof (rt_tables_ep4) >> 1 );
			break;
		}
		*(unsigned *) &ship->encryptbuf[0x00] = *(unsigned *) &ship->decryptbuf[0x04];
		compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x04 );
		break;
	case 0x10:
		// Monster appearance rates
		printf ("\nReceived rare monster appearance rates from server...\n");
		for (ch=0;ch<8;ch++)
		{
			mob_rate = *(unsigned *) &ship->decryptbuf[0x06 + (ch * 4)];
			mob_calc = (long long)mob_rate * 0xFFFFFFFF / 100000;
/*
			times_won = 0;
			for (ch2=0;ch2<1000000;ch2++)
			{
				if (mt_lrand() < mob_calc)
					times_won++;
			}
*/
			switch (ch)
			{
			case 0x00:
				printf ("Hildebear appearance rate: %3f%%\n", (float) mob_rate / 1000 );
				hildebear_rate = (unsigned) mob_calc;
				break;
			case 0x01:
				printf ("Rappy appearance rate: %3f%%\n", (float) mob_rate / 1000 );
				rappy_rate = (unsigned) mob_calc;
				break;
			case 0x02:
				printf ("Lily appearance rate: %3f%%\n", (float) mob_rate / 1000 );
				lily_rate = (unsigned) mob_calc;
				break;
			case 0x03:
				printf ("Pouilly Slime appearance rate: %3f%%\n", (float) mob_rate / 1000 );
				slime_rate = (unsigned) mob_calc;
				break;
			case 0x04:
				printf ("Merissa AA appearance rate: %3f%%\n", (float) mob_rate / 1000 );
				merissa_rate = (unsigned) mob_calc;
				break;
			case 0x05:
				printf ("Pazuzu appearance rate: %3f%%\n", (float) mob_rate / 1000 );
				pazuzu_rate = (unsigned) mob_calc;
				break;
			case 0x06:
				printf ("Dorphon Eclair appearance rate: %3f%%\n", (float) mob_rate / 1000 );
				dorphon_rate = (unsigned) mob_calc;
				break;
			case 0x07:
				printf ("Kondrieu appearance rate: %3f%%\n", (float) mob_rate / 1000 );
				kondrieu_rate = (unsigned) mob_calc;
				break;
			}
			//debug ("Actual rate: %3f%%\n", ((float) times_won / 1000000) * 100);
		}
		printf ("\nNow ready to serve players...\n");
		logon_ready = 1;
		break;
	case 0x11:
		// Ping received
		ship->last_ping = (unsigned) servertime;
		*(unsigned *) &ship->encryptbuf[0x00] = *(unsigned *) &ship->decryptbuf[0x04];
		compressShipPacket ( ship, &ship->encryptbuf[0x00], 0x04 );
		break;
	case 0x12:
		// Global announce
		gcn = *(unsigned *) &ship->decryptbuf[0x06];
		GlobalBroadcast ((unsigned short*) &ship->decryptbuf[0x0A]);
		WriteGM ("GM %u made a global announcement: %s", gcn, Unicode_to_ASCII ((unsigned short*) &ship->decryptbuf[0x0A]));
		break;
	default:
		// Unknown
		break;
	}
}

void AddPB ( unsigned char* flags, unsigned char* blasts, unsigned char pb )
{
	int pb_exists = 0;
	unsigned char pbv;
	unsigned pb_slot;

	if ( ( *flags & 0x01 ) == 0x01 )
	{
		if ( ( *blasts & 0x07 ) == pb )
			pb_exists = 1;
	}

	if ( ( *flags & 0x02 ) == 0x02 )
	{
		if ( ( ( *blasts / 8 ) & 0x07 ) == pb )
			pb_exists = 1;
	}

	if ( ( *flags  & 0x04 ) == 0x04 )
		pb_exists = 1;

	if (!pb_exists)
	{
		if ( ( *flags & 0x01 ) == 0 )
			pb_slot = 0;
		else
		if ( ( *flags & 0x02 ) == 0 )
			pb_slot = 1;
		else
			pb_slot = 2;
		switch ( pb_slot )
		{
		case 0x00:
			*blasts &= 0xF8;
			*flags  |= 0x01;
			break;
		case 0x01:
			pb *= 8;
			*blasts &= 0xC7;
			*flags  |= 0x02;
			break;
		case 0x02:
			pbv = pb;
			if ( ( *blasts & 0x07 ) < pb )
				pbv--;
			if ( ( ( *blasts / 8 ) & 0x07 ) < pb )
				pbv--;
			pb = pbv * 0x40;
			*blasts &= 0x3F;
			*flags  |= 0x04;
		}
		*blasts |= pb;
	}
}
