void CheckMaxGrind (INVENTORY_ITEM* i)
{
	if (i->item.data[3] > grind_table[i->item.data[1]][i->item.data[2]])
		i->item.data[3] = grind_table[i->item.data[1]][i->item.data[2]];
}


void UseItem (unsigned itemid, CLIENT* client)
{
	unsigned found_item = 0, ch, ch2;
	INVENTORY_ITEM i;
	int eq_wep, eq_armor, eq_shield, eq_mag = -1;
	LOBBY* l;
	unsigned new_item, TotalMatUse, HPMatUse, max_mat;
	int mat_exceed;

	// Check item stuff here...  Like converting certain things to certain things...
	//

	if (!client->lobby)
		return;

	l = (LOBBY*) client->lobby;

	for (ch=0;ch<client->character.inventoryUse;ch++)
	{
		if (client->character.inventory[ch].item.itemid == itemid)
		{
			found_item = 1;

			// Copy item before deletion (needed for consumables)
			memcpy (&i, &client->character.inventory[ch], sizeof (INVENTORY_ITEM));

			// Unwrap mag
			if ((i.item.data[0] == 0x02) && (i.item.data2[2] & 0x40))
			{
				client->character.inventory[ch].item.data2[2] &= ~(0x40);
				break;
			}

			// Unwrap item
			if ((i.item.data[0] != 0x02) && (i.item.data[4] & 0x40))
			{
				client->character.inventory[ch].item.data[4] &= ~(0x40);
				break;
			}

			if (i.item.data[0] == 0x03) // Delete consumable item right away
				DeleteItemFromClient (itemid, 1, 0, client);

			break;
		}
	}

	if (!found_item)
	{
		Send1A ("Could not find item to \"use\".", client);
		client->todc = 1;
	}
	else
	{
		// Setting the eq variables here should fix problem with ADD SLOT and such.
		eq_wep = eq_armor = eq_shield = eq_mag = -1;

		for (ch2=0;ch2<client->character.inventoryUse;ch2++)
		{
			if ( client->character.inventory[ch2].flags & 0x08 )
			{
				switch ( client->character.inventory[ch2].item.data[0] )
				{
				case 0x00:
					eq_wep = ch2;
					break;
				case 0x01:
					switch ( client->character.inventory[ch2].item.data[1] )
					{
					case 0x01:
						eq_armor = ch2;
						break;
					case 0x02:
						eq_shield = ch2;
						break;
					}
					break;
				case 0x02:
					eq_mag = ch2;
					break;
				}
			}
		}

		switch (i.item.data[0])
		{
		case 0x00:
			switch (i.item.data[1])
			{
			case 0x33:
				client->character.inventory[ch].item.data[1] = 0x32; // Sealed J-Sword -> Tsumikiri J-Sword
				SendItemToEnd (itemid, client);
				break;
			case 0x1E:
				// Heaven Punisher used...
				if ((eq_wep != -1) && (client->character.inventory[eq_wep].item.data[1] == 0xAF) &&
					(client->character.inventory[eq_wep].item.data[2] == 0x00))
				{
					client->character.inventory[eq_wep].item.data[1] = 0xB0; // Mille Marteaux
					client->character.inventory[eq_wep].item.data[2] = 0x00;
					client->character.inventory[eq_wep].item.data[3] = 0x00;
					client->character.inventory[eq_wep].item.data[4] = 0x00;
					SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
				}
				DeleteItemFromClient (itemid, 1, 0, client); // Get rid of Heaven Punisher
				break;
			case 0x42:
				// Handgun: Guld or Master Raven used...
				if ((eq_wep != -1) && (client->character.inventory[eq_wep].item.data[1] == 0x43) && 
					(client->character.inventory[eq_wep].item.data[2] == i.item.data[2]) &&
					(client->character.inventory[eq_wep].item.data[3] == 0x09))
				{
					client->character.inventory[eq_wep].item.data[1] = 0x4B; // Guld Milla or Dual Bird
					client->character.inventory[eq_wep].item.data[2] = i.item.data[2];
					client->character.inventory[eq_wep].item.data[3] = 0x00;
					client->character.inventory[eq_wep].item.data[4] = 0x00;
					SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
				}
				DeleteItemFromClient (itemid, 1, 0, client); // Get rid of Guld or Raven...
				break;
			case 0x43:
				// Handgun: Milla or Last Swan used...
				if ((eq_wep != -1) && (client->character.inventory[eq_wep].item.data[1] == 0x42) && 
					(client->character.inventory[eq_wep].item.data[2] == i.item.data[2]) &&
					(client->character.inventory[eq_wep].item.data[3] == 0x09))
				{
					client->character.inventory[eq_wep].item.data[1] = 0x4B; // Guld Milla or Dual Bird
					client->character.inventory[eq_wep].item.data[2] = i.item.data[2];
					client->character.inventory[eq_wep].item.data[3] = 0x00;
					client->character.inventory[eq_wep].item.data[4] = 0x00;
					SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
				}
				DeleteItemFromClient (itemid, 1, 0, client); // Get rid of Milla or Swan...
				break;
			case 0x8A:
				// Sange or Yasha...
				if (eq_wep != -1)
				{
					if (client->character.inventory[eq_wep].item.data[2] == !(i.item.data[2]))
					{
						client->character.inventory[eq_wep].item.data[1] = 0x89;
						client->character.inventory[eq_wep].item.data[2] = 0x03;
						client->character.inventory[eq_wep].item.data[3] = 0x00;
						client->character.inventory[eq_wep].item.data[4] = 0x00;
						SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
					}
				}
				DeleteItemFromClient (itemid, 1, 0, client); // Get rid of the other sword...
				break;
			case 0xAB:
				client->character.inventory[ch].item.data[1] = 0xAC; // Convert Lame d'Argent into Excalibur
				SendItemToEnd (itemid, client);
				break;
			case 0xAF:
				// Ophelie Seize used...
				if ((eq_wep != -1) && (client->character.inventory[eq_wep].item.data[1] == 0x1E) && 
					(client->character.inventory[eq_wep].item.data[2] == 0x00))
				{
					client->character.inventory[eq_wep].item.data[1] = 0xB0; // Mille Marteaux
					client->character.inventory[eq_wep].item.data[2] = 0x00;
					client->character.inventory[eq_wep].item.data[3] = 0x00;
					client->character.inventory[eq_wep].item.data[4] = 0x00;
					SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
				}
				DeleteItemFromClient (itemid, 1, 0, client); // Get rid of Ophelie Seize
				break;
			case 0xB6:
				// Guren used...
				if ((eq_wep != -1) && (client->character.inventory[eq_wep].item.data[1] == 0xB7) && 
					(client->character.inventory[eq_wep].item.data[2] == 0x00))
				{
					client->character.inventory[eq_wep].item.data[1] = 0xB8; // Jizai
					client->character.inventory[eq_wep].item.data[2] = 0x00;
					client->character.inventory[eq_wep].item.data[3] = 0x00;
					client->character.inventory[eq_wep].item.data[4] = 0x00;
					SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
				}
				DeleteItemFromClient (itemid, 1, 0, client); // Get rid of Guren
				break;
			case 0xB7:
				// Shouren used...
				if ((eq_wep != -1) && (client->character.inventory[eq_wep].item.data[1] == 0xB6) && 
					(client->character.inventory[eq_wep].item.data[2] == 0x00))
				{
					client->character.inventory[eq_wep].item.data[1] = 0xB8; // Jizai
					client->character.inventory[eq_wep].item.data[2] = 0x00;
					client->character.inventory[eq_wep].item.data[3] = 0x00;
					client->character.inventory[eq_wep].item.data[4] = 0x00;
					SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
				}
				DeleteItemFromClient (itemid, 1, 0, client); // Get rid of Shouren
				break;
			}
			break;
		case 0x01:
			if (i.item.data[1] == 0x03)
			{
				if (i.item.data[2] == 0x4D) // Limiter -> Adept
				{
					client->character.inventory[ch].item.data[2] = 0x4E;
					SendItemToEnd (itemid, client);
				}

				if (i.item.data[2] == 0x4F) // Swordsman Lore -> Proof of Sword-Saint
				{
					client->character.inventory[ch].item.data[2] = 0x50;
					SendItemToEnd (itemid, client);
				}
			}
			break;
		case 0x02:
			switch (i.item.data[1])
			{
			case 0x2B:
				// Chao Mag used
				if ((eq_wep != -1) && (client->character.inventory[eq_wep].item.data[1] == 0x68) && 
					(client->character.inventory[eq_wep].item.data[2] == 0x00))
				{
					client->character.inventory[eq_wep].item.data[1] = 0x58; // Striker of Chao
					client->character.inventory[eq_wep].item.data[2] = 0x00;
					client->character.inventory[eq_wep].item.data[3] = 0x00;
					client->character.inventory[eq_wep].item.data[4] = 0x00;
					SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
				}
				DeleteItemFromClient (itemid, 1, 0, client); // Get rid of Chao
				break;
			case 0x2C:
				// Chu Chu mag used
				if ((eq_armor != -1) && (client->character.inventory[eq_armor].item.data[2] == 0x1C))
				{
					client->character.inventory[eq_armor].item.data[2] = 0x2C; // Chuchu Fever
					SendItemToEnd (client->character.inventory[eq_armor].item.itemid, client);
				}
				DeleteItemFromClient (itemid, 1, 0, client); // Get rid of Chu Chu
				break;
			}
			break;
		case 0x03:
			switch (i.item.data[1])
			{
			case 0x02:
				if (i.item.data[4] < 19)
				{
					if (((char)i.item.data[2] > max_tech_level[i.item.data[4]][client->character._class]) ||
						(client->equip_flags & DROID_FLAG))
					{
						Send1A ("You can't learn that technique.", client);
						client->todc = 1;
					}
					else
						client->character.techniques[i.item.data[4]] = i.item.data[2]; // Learn technique
				}
				break;
			case 0x0A:
				if (eq_wep != -1)
				{
					client->character.inventory[eq_wep].item.data[3] += ( i.item.data[2] + 1 );
					CheckMaxGrind (&client->character.inventory[eq_wep]);
					break;
				}
				break;
			case 0x0B:
				if (!client->mode)
				{
					HPMatUse = ( client->character.HPmat + client->character.TPmat ) / 2;
					TotalMatUse = 0;
					for (ch2=0;ch2<5;ch2++)
						TotalMatUse += client->matuse[ch2];
					mat_exceed = 0;
					if ( client->equip_flags & HUMAN_FLAG )
						max_mat = 250;
					else
						max_mat = 150;
				}
				else
				{
					TotalMatUse = 0;
					HPMatUse = 0;
					max_mat = 999;
					mat_exceed = 0;
				}
				switch (i.item.data[2])  // Materials
				{
				case 0x00:
					if ( TotalMatUse < max_mat )
					{
						client->character.ATP += 2;
						if (!client->mode)
							client->matuse[0]++;
					}
					else
						mat_exceed = 1;
					break;
				case 0x01:
					if ( TotalMatUse < max_mat )
					{
						client->character.MST += 2;
						if (!client->mode)
							client->matuse[1]++;
					}
					else
						mat_exceed = 1;
					break;
				case 0x02:
					if ( TotalMatUse < max_mat )
					{
						client->character.EVP += 2;
						if (!client->mode)
							client->matuse[2]++;
					}
					else 
						mat_exceed = 1;
					break;
				case 0x03:
					if ( ( client->character.HPmat < 250 ) && ( HPMatUse < 250 ) )
						client->character.HPmat += 2;
					else
						mat_exceed = 1;
					break;
				case 0x04:
					if ( ( client->character.TPmat < 250 ) && ( HPMatUse < 250 ) )
						client->character.TPmat += 2;
					else
						mat_exceed = 1;
					break;
				case 0x05:
					if ( TotalMatUse < max_mat )
					{
						client->character.DFP += 2;
						if (!client->mode)
							client->matuse[3]++;
					}
					else
						mat_exceed = 1;
					break;
				case 0x06:
					if ( TotalMatUse < max_mat )
					{
						client->character.LCK += 2;
						if (!client->mode)
							client->matuse[4]++;
					}
					else
						mat_exceed = 1;
					break;
				default:
					break;
				}
				if ( mat_exceed )
				{
					Send1A ("Attempt to exceed material usage limit.", client);
					client->todc = 1;
				}
				break;
			case 0x0C:
				switch ( i.item.data[2] )
				{
				case 0x00: // Mag Cell 502
					if ( eq_mag != -1 )
					{
						if ( client->character.sectionID & 0x01 )
							client->character.inventory[eq_mag].item.data[1] = 0x1D;
						else
							client->character.inventory[eq_mag].item.data[1] = 0x21;
					}
					break;
				case 0x01: // Mag Cell 213
					if ( eq_mag != -1 )
					{
						if ( client->character.sectionID & 0x01 )
							client->character.inventory[eq_mag].item.data[1] = 0x27;
						else
							client->character.inventory[eq_mag].item.data[1] = 0x22;
					}
					break;
				case 0x02: // Parts of RoboChao
					if ( eq_mag != -1 )
						client->character.inventory[eq_mag].item.data[1] = 0x28;
					break;
				case 0x03: // Heart of Opa Opa
					if ( eq_mag != -1 )
						client->character.inventory[eq_mag].item.data[1] = 0x29;
					break;
				case 0x04: // Heart of Pian
					if ( eq_mag != -1 )
						client->character.inventory[eq_mag].item.data[1] = 0x2A;
					break;
				case 0x05: // Heart of Chao
					if ( eq_mag != -1 )
						client->character.inventory[eq_mag].item.data[1] = 0x2B;
					break;
				}
				break;
			case 0x0E:
				if ( ( eq_shield != -1 ) && ( i.item.data[2] > 0x15 ) && ( i.item.data[2] < 0x26 ) )
				{
					// Merges
					client->character.inventory[eq_shield].item.data[2] = 0x3A + ( i.item.data[2] - 0x16 );
					SendItemToEnd (client->character.inventory[eq_shield].item.itemid, client);
				}
				else
					switch ( i.item.data[2] )
				{
					case 0x00: 
						if ( ( eq_wep != -1 ) && ( client->character.inventory[eq_wep].item.data[1] == 0x8E ) )
						{
							client->character.inventory[eq_wep].item.data[1]  = 0x8E;
							client->character.inventory[eq_wep].item.data[2]  = 0x01; // S-Berill's Hands #1
							client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
							client->character.inventory[eq_wep].item.data[4]  = 0x00;
							SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
							break;
						}
						break;
					case 0x01: // Parasitic Gene "Flow"
						if ( eq_wep != -1 )
						{
							switch ( client->character.inventory[eq_wep].item.data[1] )
							{
							case 0x02:
								client->character.inventory[eq_wep].item.data[1]  = 0x9D; // Dark Flow
								client->character.inventory[eq_wep].item.data[2]  = 0x00;
								client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
								client->character.inventory[eq_wep].item.data[4]  = 0x00;
								SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
								break;
							case 0x09:
								client->character.inventory[eq_wep].item.data[1]  = 0x9E; // Dark Meteor
								client->character.inventory[eq_wep].item.data[2]  = 0x00;
								client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
								client->character.inventory[eq_wep].item.data[4]  = 0x00;
								SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
								break;
							case 0x0B:
								client->character.inventory[eq_wep].item.data[1]  = 0x9F; // Dark Bridge
								client->character.inventory[eq_wep].item.data[2]  = 0x00;
								client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
								client->character.inventory[eq_wep].item.data[4]  = 0x00;
								SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
								break;
							}
						}
						break;
					case 0x02: // Magic Stone "Iritista"
						if ( ( eq_wep != -1 ) && ( client->character.inventory[eq_wep].item.data[1] == 0x05 ) )
						{
							client->character.inventory[eq_wep].item.data[1]  = 0x9C; // Rainbow Baton
							client->character.inventory[eq_wep].item.data[2]  = 0x00;
							client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
							client->character.inventory[eq_wep].item.data[4]  = 0x00;
							SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
							break;
						}
						break;
					case 0x03: // Blue-Black Stone
						if ( ( eq_wep != -1 ) && ( client->character.inventory[eq_wep].item.data[1] == 0x2F ) && 
							( client->character.inventory[eq_wep].item.data[2] == 0x00 ) && 
							( client->character.inventory[eq_wep].item.data[3] == 0x19 ) )
						{
							client->character.inventory[eq_wep].item.data[2]  = 0x01; // Black King Bar
							client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
							client->character.inventory[eq_wep].item.data[4]  = 0x00;
							SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
							break;
						}
						break;
					case 0x04: // Syncesta
						if ( eq_wep != -1 )
						{
							switch ( client->character.inventory[eq_wep].item.data[1] )
							{
							case 0x1F:
								client->character.inventory[eq_wep].item.data[1]  = 0x38; // Lavis Blade
								client->character.inventory[eq_wep].item.data[2]  = 0x00;
								client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
								client->character.inventory[eq_wep].item.data[4]  = 0x00;
								SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
								break;
							case 0x38:
								client->character.inventory[eq_wep].item.data[1]  = 0x30; // Double Cannon
								client->character.inventory[eq_wep].item.data[2]  = 0x00;
								client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
								client->character.inventory[eq_wep].item.data[4]  = 0x00;
								SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
								break;
							case 0x30:
								client->character.inventory[eq_wep].item.data[1]  = 0x1F; // Lavis Cannon
								client->character.inventory[eq_wep].item.data[2]  = 0x00;
								client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
								client->character.inventory[eq_wep].item.data[4]  = 0x00;
								SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
								break;
							}
						}
						break;
					case 0x05: // Magic Water
						if ( ( eq_wep != -1 ) && ( client->character.inventory[eq_wep].item.data[1] == 0x56 ) ) 								
						{
							if ( client->character.inventory[eq_wep].item.data[2] == 0x00 )
							{
								client->character.inventory[eq_wep].item.data[1]  = 0x5D; // Plantain Fan
								client->character.inventory[eq_wep].item.data[2]  = 0x00;
								client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
								client->character.inventory[eq_wep].item.data[4]  = 0x00;
								SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
								break;
							}
							else
								if ( client->character.inventory[eq_wep].item.data[2] == 0x01 )
								{
									client->character.inventory[eq_wep].item.data[1]  = 0x63; // Plantain Huge Fan
									client->character.inventory[eq_wep].item.data[2]  = 0x00;
									client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
									client->character.inventory[eq_wep].item.data[4]  = 0x00;
									SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
									break;
								}
						}
						break;
					case 0x06: // Parasitic Cell Type D
						if ( eq_armor != -1 )
							switch ( client->character.inventory[eq_armor].item.data[2] ) 
						{
							case 0x1D:  
								client->character.inventory[eq_armor].item.data[2] = 0x20; // Parasite Wear: De Rol
								SendItemToEnd (client->character.inventory[eq_armor].item.itemid, client);
								break;
							case 0x20: 
								client->character.inventory[eq_armor].item.data[2] = 0x21; // Parsite Wear: Nelgal
								SendItemToEnd (client->character.inventory[eq_armor].item.itemid, client);
								break;
							case 0x21: 
								client->character.inventory[eq_armor].item.data[2] = 0x22; // Parasite Wear: Vajulla
								SendItemToEnd (client->character.inventory[eq_armor].item.itemid, client);
								break;
							case 0x22: 
								client->character.inventory[eq_armor].item.data[2] = 0x2F; // Virus Armor: Lafuteria
								SendItemToEnd (client->character.inventory[eq_armor].item.itemid, client);
								break;
						}
						break;
					case 0x07: // Magic Rock "Heart Key"
						if ( eq_armor != -1 )
						{
							if ( client->character.inventory[eq_armor].item.data[2] == 0x1C )
							{
								client->character.inventory[eq_armor].item.data[2] = 0x2D; // Love Heart
								SendItemToEnd (client->character.inventory[eq_armor].item.itemid, client);
							}
							else
								if ( client->character.inventory[eq_armor].item.data[2] == 0x2D ) 
								{
									client->character.inventory[eq_armor].item.data[2] = 0x45; // Sweetheart
									SendItemToEnd (client->character.inventory[eq_armor].item.itemid, client);
								}
								else
									if ( ( eq_wep != -1 ) && ( client->character.inventory[eq_wep].item.data[1] == 0x0C ) )
									{
										client->character.inventory[eq_wep].item.data[1]  = 0x24; // Magical Piece
										client->character.inventory[eq_wep].item.data[2]  = 0x00;
										client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
										client->character.inventory[eq_wep].item.data[4]  = 0x00;
										SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
									}
									else
										if ( ( eq_shield != -1 ) && ( client->character.inventory[eq_shield].item.data[2] == 0x15 ) )
										{
											client->character.inventory[eq_shield].item.data[2]  = 0x2A; // Safety Heart
											SendItemToEnd (client->character.inventory[eq_shield].item.itemid, client);
										}
						}
						break;
					case 0x08: // Magic Rock "Moola"
						if ( ( eq_armor != -1 ) && ( client->character.inventory[eq_armor].item.data[2] == 0x1C ) )
						{
							client->character.inventory[eq_armor].item.data[2] = 0x31; // Aura Field
							SendItemToEnd (client->character.inventory[eq_armor].item.itemid, client);
						}
						else
							if ( ( eq_wep != -1 ) && ( client->character.inventory[eq_wep].item.data[1] == 0x0A ) )
							{
								client->character.inventory[eq_wep].item.data[1]  = 0x4F; // Summit Moon
								client->character.inventory[eq_wep].item.data[2]  = 0x00;
								client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
								client->character.inventory[eq_wep].item.data[4]  = 0x00; // No attribute
								SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
							}
							break;
					case 0x09: // Star Amplifier
						if ( ( eq_armor != -1 ) && ( client->character.inventory[eq_armor].item.data[2] == 0x1C ) )
						{
							client->character.inventory[eq_armor].item.data[2] = 0x30; // Brightness Circle
							SendItemToEnd (client->character.inventory[eq_armor].item.itemid, client);
						}
						else
							if ( ( eq_wep != -1 ) && ( client->character.inventory[eq_wep].item.data[1] == 0x0C ) )
							{
								client->character.inventory[eq_wep].item.data[1]  = 0x5C; // Twinkle Star
								client->character.inventory[eq_wep].item.data[2]  = 0x00;
								client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
								client->character.inventory[eq_wep].item.data[4]  = 0x00; // No attribute
								SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
							}
							break;
					case 0x0A: // Book of Hitogata
						if ( ( eq_wep != -1 ) && ( client->character.inventory[eq_wep].item.data[1] == 0x8C ) &&
							( client->character.inventory[eq_wep].item.data[2] == 0x02 ) && 
							( client->character.inventory[eq_wep].item.data[3] == 0x09 ) )
						{
							client->character.inventory[eq_wep].item.data[1]  = 0x8C;
							client->character.inventory[eq_wep].item.data[2]  = 0x03;
							client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
							client->character.inventory[eq_wep].item.data[4]  = 0x00;
							SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
						}
						break;
					case 0x0B: // Heart of Chu Chu
						if ( eq_mag != -1 )
							client->character.inventory[eq_mag].item.data[1] = 0x2C;
						break;
					case 0x0C: // Parts of Egg Blaster
						if ( ( eq_wep != -1 ) && ( client->character.inventory[eq_wep].item.data[1] == 0x06 ) )
						{
							client->character.inventory[eq_wep].item.data[1]  = 0x1C; // Egg Blaster
							client->character.inventory[eq_wep].item.data[2]  = 0x00;
							client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
							client->character.inventory[eq_wep].item.data[4]  = 0x00;
							SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
						}
						break;
					case 0x0D: // Heart of Angel
						if ( eq_mag != -1 )
							client->character.inventory[eq_mag].item.data[1] = 0x2E;
						break;
					case 0x0E: // Heart of Devil
						if ( eq_mag != -1 )
						{
							if ( client->character.inventory[eq_mag].item.data[1] == 0x2F )
								client->character.inventory[eq_mag].item.data[1] = 0x38;
							else
								client->character.inventory[eq_mag].item.data[1] = 0x2F;
						}
						break;
					case 0x0F: // Kit of Hamburger
						if ( eq_mag != -1 )
							client->character.inventory[eq_mag].item.data[1] = 0x36;
						break;
					case 0x10: // Panther's Spirit
						if ( eq_mag != -1 )
							client->character.inventory[eq_mag].item.data[1] = 0x37;
						break;
					case 0x11: // Kit of Mark 3
						if ( eq_mag != -1 )
							client->character.inventory[eq_mag].item.data[1] = 0x31;
						break;
					case 0x12: // Kit of Master System
						if ( eq_mag != -1 )
							client->character.inventory[eq_mag].item.data[1] = 0x32;
						break;
					case 0x13: // Kit of Genesis
						if ( eq_mag != -1 )
							client->character.inventory[eq_mag].item.data[1] = 0x33;
						break;
					case 0x14: // Kit of Sega Saturn
						if ( eq_mag != -1 )
							client->character.inventory[eq_mag].item.data[1] = 0x34;
						break;
					case 0x15: // Kit of Dreamcast
						if ( eq_mag != -1 )
							client->character.inventory[eq_mag].item.data[1] = 0x35;
						break;
					case 0x26: // Heart of Kapukapu
						if ( eq_mag != -1 )
							client->character.inventory[eq_mag].item.data[1] = 0x2D;
						break;
					case 0x27: // Photon Booster
						if ( eq_wep != -1 )
						{
							switch ( client->character.inventory[eq_wep].item.data[1] )
							{
							case 0x15:
								if ( client->character.inventory[eq_wep].item.data[3] == 0x09 )
								{
									client->character.inventory[eq_wep].item.data[2]  = 0x01; // Burning Visit
									client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
									client->character.inventory[eq_wep].item.data[4]  = 0x00;
									SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
								}
								break;
							case 0x45:
								if ( client->character.inventory[eq_wep].item.data[3] == 0x09 )
								{
									client->character.inventory[eq_wep].item.data[2]  = 0x01; // Snow Queen
									client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
									client->character.inventory[eq_wep].item.data[4]  = 0x00;
									SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
								}
								break;
							case 0x4E:
								if ( client->character.inventory[eq_wep].item.data[3] == 0x09 )
								{
									client->character.inventory[eq_wep].item.data[2]  = 0x01; // Iron Faust
									client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
									client->character.inventory[eq_wep].item.data[4]  = 0x00;
									SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
								}
								break;
							case 0x6D: 
								if ( client->character.inventory[eq_wep].item.data[3] == 0x14 )
								{
									client->character.inventory[eq_wep].item.data[2]  = 0x01; // Power Maser
									client->character.inventory[eq_wep].item.data[3]  = 0x00; // Not grinded
									client->character.inventory[eq_wep].item.data[4]  = 0x00;
									SendItemToEnd (client->character.inventory[eq_wep].item.itemid, client);
								}
								break;
							}
						}
						break;

				}
				break;
			case 0x0F: // Add Slot
				if ((eq_armor != -1) && (client->character.inventory[eq_armor].item.data[5] < 0x04))
					client->character.inventory[eq_armor].item.data[5]++;
				break;
			case 0x15:
				new_item = 0;
				switch ( i.item.data[2] )
				{
				case 0x00:
					new_item = 0x0D0E03 + ( ( mt_lrand() % 9 ) * 0x10000 );
					break;
				case 0x01:
					new_item = easter_drops[mt_lrand() % 9];
					break;
				case 0x02:
					new_item = jacko_drops[mt_lrand() % 8];
					break;
				default:
					break;
				}

				if ( new_item )
				{
					INVENTORY_ITEM add_item;

					memset (&add_item, 0, sizeof (INVENTORY_ITEM));
					*(unsigned *) &add_item.item.data[0] = new_item;
					add_item.item.itemid = l->playerItemID[client->clientID];
					l->playerItemID[client->clientID]++;
					AddToInventory (&add_item, 1, 0, client);
				}
				break;
			case 0x18: // Ep4 Mag Cells
				if ( eq_mag != -1 )
					client->character.inventory[eq_mag].item.data[1] = 0x42 + ( i.item.data[2] );
				break;
			}
			break;
		default:
			break;
		}
	}
}

int check_equip (unsigned char eq_flags, unsigned char cl_flags)
{
	int eqOK = 1;
	unsigned ch;

	for (ch=0;ch<8;ch++)
	{
		if ( ( cl_flags & (1 << ch) ) && (!( eq_flags & ( 1 << ch ) )) )
		{
			eqOK = 0;
			break;
		}
	}
	return eqOK;
}

void EquipItem (unsigned itemid, CLIENT* client)
{
	unsigned ch, ch2, found_item, found_slot;
	unsigned slot[4];

	found_item = 0;
	found_slot = 0;

	for (ch=0;ch<client->character.inventoryUse;ch++)
	{
		if (client->character.inventory[ch].item.itemid == itemid)
		{
			//debug ("Equipped %u", itemid);
			found_item = 1;
			switch (client->character.inventory[ch].item.data[0])
			{
			case 0x00:
				// Check weapon equip requirements
				if (!check_equip(weapon_equip_table[client->character.inventory[ch].item.data[1]][client->character.inventory[ch].item.data[2]], client->equip_flags))
				{
					Send1A ("\"God/Equip\" is disallowed.", client);
					client->todc = 1;
				}
				if (!client->todc)
				{
					// De-equip any other weapon on character. (Prevent stacking)
					for (ch2=0;ch2<client->character.inventoryUse;ch2++)
						if ((client->character.inventory[ch2].item.data[0] == 0x00) &&
							(client->character.inventory[ch2].flags & 0x08))
							client->character.inventory[ch2].flags &= ~(0x08);
				}
				break;
			case 0x01:
				switch (client->character.inventory[ch].item.data[1])
				{
				case 0x01: // Check armor equip requirements
					if ((!check_equip (armor_equip_table[client->character.inventory[ch].item.data[2]], client->equip_flags)) || 
						(client->character.level < armor_level_table[client->character.inventory[ch].item.data[2]]))
					{
						Send1A ("\"God/Equip\" is disallowed.", client);
						client->todc = 1;
					}
					if (!client->todc)
					{
						// Remove any other armor and equipped slot items.
						for (ch2=0;ch2<client->character.inventoryUse;ch2++)
							if ((client->character.inventory[ch2].item.data[0] == 0x01) &&
								(client->character.inventory[ch2].item.data[1] != 0x02) &&
								(client->character.inventory[ch2].flags & 0x08))
							{
								client->character.inventory[ch2].flags &= ~(0x08);
								client->character.inventory[ch2].item.data[4] = 0x00;
							}
							break;
					}
					break;
				case 0x02: // Check barrier equip requirements
					if ((!check_equip (barrier_equip_table[client->character.inventory[ch].item.data[2]] & client->equip_flags, client->equip_flags)) ||
						(client->character.level < barrier_level_table[client->character.inventory[ch].item.data[2]]))
					{
						Send1A ("\"God/Equip\" is disallowed.", client);
						client->todc = 1;
					}
					if (!client->todc)
					{
						// Remove any other barrier
						for (ch2=0;ch2<client->character.inventoryUse;ch2++)
							if ((client->character.inventory[ch2].item.data[0] == 0x01) &&
								(client->character.inventory[ch2].item.data[1] == 0x02) &&
								(client->character.inventory[ch2].flags & 0x08))
							{
								client->character.inventory[ch2].flags &= ~(0x08);
								client->character.inventory[ch2].item.data[4] = 0x00;
							}
					}
					break;
				case 0x03: // Assign unit a slot
					for (ch2=0;ch2<4;ch2++)
						slot[ch2] = 0;
					for (ch2=0;ch2<client->character.inventoryUse;ch2++)
					{
						// Another loop ;(
						if ((client->character.inventory[ch2].item.data[0] == 0x01) && 
							(client->character.inventory[ch2].item.data[1] == 0x03))
						{
							if ((client->character.inventory[ch2].flags & 0x08) && 
								(client->character.inventory[ch2].item.data[4] < 0x04))
								slot [client->character.inventory[ch2].item.data[4]] = 1;
						}
					}
					for (ch2=0;ch2<4;ch2++)
						if (slot[ch2] == 0)
						{
							found_slot = ch2 + 1;
							break;
						}
						if (found_slot)
						{
							found_slot --;
							client->character.inventory[ch].item.data[4] = (unsigned char)(found_slot);
						}
						else
						{
							client->character.inventory[ch].flags &= ~(0x08);
							Send1A ("There are no free slots on your armor.  Equip unit failed.", client);
							client->todc = 1;
						}
						break;
				}
				break;
			case 0x02:
				// Remove equipped mag
				for (ch2=0;ch2<client->character.inventoryUse;ch2++)
					if ((client->character.inventory[ch2].item.data[0] == 0x02) &&
						(client->character.inventory[ch2].flags & 0x08))
						client->character.inventory[ch2].flags &= ~(0x08);
				break;
			}
			if ( !client->todc )  // Finally, equip item
				client->character.inventory[ch].flags |= 0x08;
			break;
		}
	}
	if (!found_item)
	{
		Send1A ("Could not find item to equip.", client);
		client->todc = 1;
	}
}

void DeequipItem (unsigned itemid, CLIENT* client)
{
	unsigned ch, ch2, found_item = 0;

	for (ch=0;ch<client->character.inventoryUse;ch++)
	{
		if (client->character.inventory[ch].item.itemid == itemid)
		{
			found_item = 1;
			client->character.inventory[ch].flags &= ~(0x08);
			switch (client->character.inventory[ch].item.data[0])
			{
			case 0x00:
				// Remove any other weapon (in case of a glitch... or stacking)
				for (ch2=0;ch2<client->character.inventoryUse;ch2++)
					if ((client->character.inventory[ch2].item.data[0] == 0x00) &&
						(client->character.inventory[ch2].flags & 0x08))
						client->character.inventory[ch2].flags &= ~(0x08);
				break;
			case 0x01:
				switch (client->character.inventory[ch].item.data[1])
				{
				case 0x01:
					// Remove any other armor (stacking?) and equipped slot items.
					for (ch2=0;ch2<client->character.inventoryUse;ch2++)
						if ((client->character.inventory[ch2].item.data[0] == 0x01) &&
							(client->character.inventory[ch2].item.data[1] != 0x02) &&
							(client->character.inventory[ch2].flags & 0x08))
						{
							client->character.inventory[ch2].flags &= ~(0x08);
							client->character.inventory[ch2].item.data[4] = 0x00;
						}
						break;
				case 0x02:
					// Remove any other barrier (stacking?)
					for (ch2=0;ch2<client->character.inventoryUse;ch2++)
						if ((client->character.inventory[ch2].item.data[0] == 0x01) &&
							(client->character.inventory[ch2].item.data[1] == 0x02) &&
							(client->character.inventory[ch2].flags & 0x08))
						{
							client->character.inventory[ch2].flags &= ~(0x08);
							client->character.inventory[ch2].item.data[4] = 0x00;
						}
						break;
				case 0x03:
					// Remove unit from slot
					client->character.inventory[ch].item.data[4] = 0x00;
					break;
				}
				break;
			case 0x02:
				// Remove any other mags (stacking?)
				for (ch2=0;ch2<client->character.inventoryUse;ch2++)
					if ((client->character.inventory[ch2].item.data[0] == 0x02) &&
						(client->character.inventory[ch2].flags & 0x08))
						client->character.inventory[ch2].flags &= ~(0x08);
				break;
			}
			break;
		}
	}
	if (!found_item)
	{
		Send1A ("Could not find item to unequip.", client);
		client->todc = 1;
	}
}

unsigned GetShopPrice(INVENTORY_ITEM* ci)
{
	unsigned compare_item, ch;
	int percent_add, price;
	unsigned char variation;
	float percent_calc;
	float price_calc;

	price = 10;

/*	printf ("Raw item data for this item is:\r\n%02x%02x%02x%02x\r\n%02x%02x%02x%02x\r\n%02x%02x%02x%02x\r\n%02x%02x%02x%02x\r\n", 
		ci->item.data[0], ci->item.data[1], ci->item.data[2], ci->item.data[3], 
		ci->item.data[4], ci->item.data[5], ci->item.data[6], ci->item.data[7], 
		ci->item.data[8], ci->item.data[9], ci->item.data[10], ci->item.data[11], 
		ci->item.data[12], ci->item.data[13], ci->item.data[14], ci->item.data[15] ); */

	switch (ci->item.data[0])
	{
	case 0x00: // Weapons
		if (ci->item.data[4] & 0x80)
			price  = 1; // Untekked = 1 meseta
		else
		{
			if ((ci->item.data[1] < 0x0D) && (ci->item.data[2] < 0x05))
			{
				if ((ci->item.data[1] > 0x09) && (ci->item.data[2] > 0x03)) // Canes, Rods, Wands become rare faster
					break;
				price = weapon_atpmax_table[ci->item.data[1]][ci->item.data[2]] + ci->item.data[3];
				price *= price;
				price_calc = (float) price;
				switch (ci->item.data[1])
				{
				case 0x01:
					price_calc /= 5.0;
					break;
				case 0x02:
					price_calc /= 4.0;
					break;
				case 0x03:
				case 0x04:
					price_calc *= 2.0;
					price_calc /= 3.0;
					break;
				case 0x05:
					price_calc *= 4.0;
					price_calc /= 5.0;
					break;
				case 0x06:
					price_calc *= 10.0;
					price_calc /= 21.0;
					break;
				case 0x07:
					price_calc /= 3.0;
					break;
				case 0x08:
					price_calc *= 25.0;
					break;
				case 0x09:
					price_calc *= 10.0;
					price_calc /= 9.0;
					break;
				case 0x0A:
					price_calc /= 2.0;
					break;
				case 0x0B:
					price_calc *= 2.0;
					price_calc /= 5.0;
					break;
				case 0x0C:
					price_calc *= 4.0;
					price_calc /= 3.0;
					break;
				}

				percent_add = 0;
				if (ci->item.data[6])
					percent_add += (char) ci->item.data[7];
				if (ci->item.data[8])
					percent_add += (char) ci->item.data[9];
				if (ci->item.data[10])
					percent_add += (char) ci->item.data[11];

				if ( percent_add != 0 )
				{
					percent_calc = price_calc;
					percent_calc /= 300.0;
					percent_calc *= percent_add;
					price_calc += percent_calc;
				}
				price_calc /= 8.0;
				price = (int) ( price_calc );
				price += attrib[ci->item.data[4]];
			}
		}
		break;
	case 0x01:
		switch (ci->item.data[1])
		{
		case 0x01: // Armor
			if (ci->item.data[2] < 0x18)
			{
				// Calculate the amount to boost because of slots...
				if (ci->item.data[5] > 4)
					price = armor_prices[(ci->item.data[2] * 5) + 4];
				else
					price = armor_prices[(ci->item.data[2] * 5) + ci->item.data[5]];
				price -= armor_prices[(ci->item.data[2] * 5)];
				if (ci->item.data[6] > armor_dfpvar_table[ci->item.data[2]])
					variation = 0;
				else
					variation = ci->item.data[6];
				if (ci->item.data[8] <= armor_evpvar_table[ci->item.data[2]])
					variation += ci->item.data[8];
				price += equip_prices[1][1][ci->item.data[2]][variation];
			}
			break;
		case 0x02: // Shield
			if (ci->item.data[2] < 0x15)
			{
				if (ci->item.data[6] > barrier_dfpvar_table[ci->item.data[2]])
					variation = 0;
				else
					variation = ci->item.data[6];
				if (ci->item.data[8] <= barrier_evpvar_table[ci->item.data[2]])
					variation += ci->item.data[8];
				price = equip_prices[1][2][ci->item.data[2]][variation];
			}
			break;
		case 0x03: // Units
			if (ci->item.data[2] < 0x40)
				price = unit_prices [ci->item.data[2]];
			break;
		}
		break;
	case 0x03:
		// Tool
		if (ci->item.data[1] == 0x02) // Technique
		{
			if (ci->item.data[4] < 0x13)
				price = ((int) (ci->item.data[2] + 1) * tech_prices[ci->item.data[4]]) / 100L;
		}
		else
		{
			compare_item = 0;
			memcpy (&compare_item, &ci->item.data[0], 3);
			for (ch=0;ch<(sizeof(tool_prices)/4);ch+=2)
				if (compare_item == tool_prices[ch])
				{
					price = tool_prices[ch+1];
					break;
				}		
		}
		break;
	}
	if ( price < 0 )
		price = 0;
	//printf ("GetShopPrice = %u\n", price);
	return (unsigned) price;
}

void SkipToLevel (unsigned short target_level, CLIENT* client, int quiet)
{
	MAG* m;
	unsigned short ch, finalDFP, finalATP, finalATA, finalMST;

	if (target_level > 199)
		target_level = 199;

	if ((!client->lobby) || (client->character.level >= target_level))
		return;

	if (!quiet)
	{
		PacketBF[0x0A] = (unsigned char) client->clientID;
		*(unsigned *) &PacketBF[0x0C] = tnlxp [target_level - 1] - client->character.XP;
		SendToLobby (client->lobby, 4, &PacketBF[0], 0x10, 0);
	}

	while (client->character.level < target_level)
	{
		client->character.ATP += playerLevelData[client->character._class][client->character.level].ATP;
		client->character.MST += playerLevelData[client->character._class][client->character.level].MST;
		client->character.EVP += playerLevelData[client->character._class][client->character.level].EVP;
		client->character.HP  += playerLevelData[client->character._class][client->character.level].HP;
		client->character.DFP += playerLevelData[client->character._class][client->character.level].DFP;
		client->character.ATA += playerLevelData[client->character._class][client->character.level].ATA;
		client->character.level++;
	}
	
	client->character.XP = tnlxp [target_level - 1];

	finalDFP = client->character.DFP;
	finalATP = client->character.ATP;
	finalATA = client->character.ATA;
	finalMST = client->character.MST;

	// Add the mag bonus to the 0x30 packet
	for (ch=0;ch<client->character.inventoryUse;ch++)
	{
		if ((client->character.inventory[ch].item.data[0] == 0x02) &&
			(client->character.inventory[ch].flags & 0x08))
		{
			m = (MAG*) &client->character.inventory[ch].item.data[0];
			finalDFP += ( m->defense / 100 );
			finalATP += ( m->power / 100 ) * 2;
			finalATA += ( m->dex / 100 ) / 2;
			finalMST += ( m->mind / 100 ) * 2;
			break;
		}
	}

	if (!quiet)
	{
		*(unsigned short*) &Packet30[0x0C] = finalATP;
		*(unsigned short*) &Packet30[0x0E] = finalMST;
		*(unsigned short*) &Packet30[0x10] = client->character.EVP;
		*(unsigned short*) &Packet30[0x12] = client->character.HP;
		*(unsigned short*) &Packet30[0x14] = finalDFP;
		*(unsigned short*) &Packet30[0x16] = finalATA;
		*(unsigned short*) &Packet30[0x18] = client->character.level;
		Packet30[0x0A] = (unsigned char) client->clientID;
		SendToLobby ( client->lobby, 4, &Packet30[0x00], 0x1C, 0);
	}
}


void AddExp (unsigned XP, CLIENT* client)
{
	MAG* m;
	unsigned short levelup, ch, finalDFP, finalATP, finalATA, finalMST;

	if (!client->lobby)
		return;

	client->character.XP += XP;

	PacketBF[0x0A] = (unsigned char) client->clientID;
	*(unsigned *) &PacketBF[0x0C] = XP;
	SendToLobby (client->lobby, 4, &PacketBF[0], 0x10, 0);
	if (client->character.XP >= tnlxp[client->character.level])
		levelup = 1;
	else
		levelup = 0;

	while (levelup)
	{
		client->character.ATP += playerLevelData[client->character._class][client->character.level].ATP;
		client->character.MST += playerLevelData[client->character._class][client->character.level].MST;
		client->character.EVP += playerLevelData[client->character._class][client->character.level].EVP;
		client->character.HP  += playerLevelData[client->character._class][client->character.level].HP;
		client->character.DFP += playerLevelData[client->character._class][client->character.level].DFP;
		client->character.ATA += playerLevelData[client->character._class][client->character.level].ATA;
		client->character.level++;
		if ((client->character.level == 199) || (client->character.XP < tnlxp[client->character.level]))
			break;
	}

	if (levelup)
	{
		finalDFP = client->character.DFP;
		finalATP = client->character.ATP;
		finalATA = client->character.ATA;
		finalMST = client->character.MST;

		// Add the mag bonus to the 0x30 packet
		for (ch=0;ch<client->character.inventoryUse;ch++)
		{
			if ((client->character.inventory[ch].item.data[0] == 0x02) &&
				(client->character.inventory[ch].flags & 0x08))
			{
				m = (MAG*) &client->character.inventory[ch].item.data[0];
				finalDFP += ( m->defense / 100 );
				finalATP += ( m->power / 100 ) * 2;
				finalATA += ( m->dex / 100 ) / 2;
				finalMST += ( m->mind / 100 ) * 2;
				break;
			}
		}

		*(unsigned short*) &Packet30[0x0C] = finalATP;
		*(unsigned short*) &Packet30[0x0E] = finalMST;
		*(unsigned short*) &Packet30[0x10] = client->character.EVP;
		*(unsigned short*) &Packet30[0x12] = client->character.HP;
		*(unsigned short*) &Packet30[0x14] = finalDFP;
		*(unsigned short*) &Packet30[0x16] = finalATA;
		*(unsigned short*) &Packet30[0x18] = client->character.level;
		Packet30[0x0A] = (unsigned char) client->clientID;
		SendToLobby ( client->lobby, 4, &Packet30[0x00], 0x1C, 0);
	}
}

void PrepGuildCard ( unsigned from, unsigned to )
{
	int gc_present = 0;
	unsigned hightime = 0xFFFFFFFF;
	unsigned highidx = 0;
	unsigned ch;

	if (ship_gcsend_count < MAX_GCSEND)
	{
		for (ch=0;ch<(ship_gcsend_count*3);ch+=3)
		{
			if ((ship_gcsend_list[ch] == from) && (ship_gcsend_list[ch+1] == to))
			{
				gc_present = 1;
				break;
			}
		}

		if (!gc_present)
		{
			highidx = ship_gcsend_count * 3;
			ship_gcsend_count++;
		}
	}
	else
	{
		// Erase oldest sent card
		for (ch=0;ch<(MAX_GCSEND*3);ch+=3)
		{
			if (ship_gcsend_list[ch+2] < hightime)
			{
				hightime = ship_gcsend_list[ch+2];
				highidx  = ch;
			}
		}
	}
	ship_gcsend_list[highidx]	= from;
	ship_gcsend_list[highidx+1]	= to;
	ship_gcsend_list[highidx+2]	= (unsigned) servertime;
}

int PreppedGuildCard ( unsigned from, unsigned to )
{
	int gc_present = 0;
	unsigned ch, ch2;

	for (ch=0;ch<(ship_gcsend_count*3);ch+=3)
	{
		if ((ship_gcsend_list[ch] == from) && (ship_gcsend_list[ch+1] == to))
		{
			ship_gcsend_list[ch]   = 0;
			ship_gcsend_list[ch+1] = 0;
			ship_gcsend_list[ch+2] = 0;
			gc_present = 1;
			break;
		}
	}

	if (gc_present)
	{
		// Clean up the list
		ch2 = 0;
		for (ch=0;ch<(ship_gcsend_count*3);ch+=3)
		{
			if ((ship_gcsend_list[ch] != 0) && (ch != ch2))
			{
				ship_gcsend_list[ch2]   = ship_gcsend_list[ch];
				ship_gcsend_list[ch2+1] = ship_gcsend_list[ch+1];
				ship_gcsend_list[ch2+2] = ship_gcsend_list[ch+2];
				ch2 += 3;
			}
		}
		ship_gcsend_count = ch2 / 3;
	}

	return gc_present;
}

int ban ( unsigned gc_num, unsigned* ipaddr, long long* hwinfo, unsigned type, CLIENT* client )
{
	int banned = 1;
	unsigned ch, ch2;
	FILE* fp;

	for (ch=0;ch<num_bans;ch++)
	{
		if ( ( ship_bandata[ch].guildcard == gc_num ) && ( ship_bandata[ch].type == type ) )
		{
			banned = 0;
			ship_bandata[ch].guildcard = 0;
			ship_bandata[ch].type = 0;
			break;
		}
	}

	if (banned)
	{
		if (num_bans < 5000)
		{
			ship_bandata[num_bans].guildcard = gc_num;
			ship_bandata[num_bans].type = type;
			ship_bandata[num_bans].ipaddr = *ipaddr;
			ship_bandata[num_bans++].hwinfo = *hwinfo;
			fp = fopen ("bandata.dat", "wb");
			if (fp)
			{
				fwrite (&ship_bandata[0], 1, sizeof (BANDATA) * num_bans, fp);
				fclose (fp);
			}
			else
				WriteLog ("Could not open bandata.dat for writing!!");
		}
		else
		{
			banned = 0; // Can't ban with a full list...
			SendB0 ("Ship ban list is full.", client);
		}
	}
	else
	{
		ch2 = 0;
		for (ch=0;ch<num_bans;ch++)
		{
			if ((ship_bandata[ch].type != 0) && (ch != ch2))
				memcpy (&ship_bandata[ch2++], &ship_bandata[ch], sizeof (BANDATA));
		}
		num_bans = ch2;
		fp = fopen ("bandata.dat", "wb");
		if (fp)
		{
			fwrite (&ship_bandata[0], 1, sizeof (BANDATA) * num_bans, fp);
			fclose (fp);
		}
		else
			WriteLog ("Could not open bandata.dat for writing!!");
	}
	return banned;
}

int stfu ( unsigned gc_num )
{
	int result = 0;
	unsigned ch;

	for (ch=0;ch<ship_ignore_count;ch++)
	{
		if (ship_ignore_list[ch] == gc_num)
		{
			result = 1;
			break;
		}
	}

	return result;
}

int toggle_stfu ( unsigned gc_num, CLIENT* client )
{
	int ignored = 1;
	unsigned ch, ch2;

	for (ch=0;ch<ship_ignore_count;ch++)
	{
		if (ship_ignore_list[ch] == gc_num)
		{
			ignored = 0;
			ship_ignore_list[ch] = 0;
			break;
		}
	}

	if (ignored)
	{
		if (ship_ignore_count < 300)
			ship_ignore_list[ship_ignore_count++] = gc_num;
		else
		{
			ignored = 0; // Can't ignore with a full list...
			SendB0 ("Ship ignore list is full.", client);
		}
	}
	else
	{
		ch2 = 0;
		for (ch=0;ch<ship_ignore_count;ch++)
		{
			if ((ship_ignore_list[ch] != 0) && (ch != ch2))
				ship_ignore_list[ch2++] = ship_ignore_list[ch];
		}
		ship_ignore_count = ch2;
	}
	return ignored;
}

void Send60 (CLIENT* client)
{
	unsigned short size, size_check_index;
	unsigned short sizecheck = 0;
	int dont_send = 0;
	LOBBY* l;
	int boss_floor = 0;
	unsigned itemid, magid, count, drop;
	unsigned short mid;
	short mHP;
	unsigned XP, ch, ch2, max_send, shop_price;
	int mid_mismatch;
	int ignored;
	int ws_ok;
	unsigned short ws_data, counter;
	CLIENT* lClient;

	size = *(unsigned short*) &client->decryptbuf[0x00];
	sizecheck = client->decryptbuf[0x09];

	sizecheck *= 4;
	sizecheck += 8;

	if (!client->lobby)
		return;

#ifdef LOG_60
			packet_to_text (&client->decryptbuf[0], size);
			fprintf(debugfile, "%s\n", dp);
#endif

	if (size != sizecheck)
	{
		debug ("Client sent a 0x60 packet whose sizecheck != size.\n");
		debug ("Command: %02X | Size: %04X | Sizecheck(%02x): %04x\n", client->decryptbuf[0x08],
			size, client->decryptbuf[0x09], sizecheck);
		client->decryptbuf[0x09] = ((size / 4) - 2);
	}

	l = (LOBBY*) client->lobby;

	if ( client->lobbyNum < 0x10 )
	{
		size_check_index  = client->decryptbuf[0x08];
		size_check_index *= 2;

		if (client->decryptbuf[0x08] == 0x06)
			sizecheck = 0x114; 
		else
			sizecheck = size_check_table[size_check_index+1] + 4;

		if ( ( size != sizecheck ) && ( sizecheck > 4 ) )
			dont_send = 1;

		if ( sizecheck == 4 ) // No size check packet encountered while in lobby mode...
		{
			debug ("No size check information for 0x60 lobby packet %02x", client->decryptbuf[0x08]);
			dont_send = 1;
		}
	}
	else
	{
		if (dont_send_60[(client->decryptbuf[0x08]*2) + 1] == 1)
		{
			dont_send = 1;
			WriteLog ("60 command \"%02x\" blocked in game. (Data below)", client->decryptbuf[0x08]);
			packet_to_text ( &client->decryptbuf[0x00], size );
			WriteLog ("%s", &dp[0]);
		}
	}

	if ( ( client->decryptbuf[0x0A] != client->clientID )				&&
		 ( size_check_table[(client->decryptbuf[0x08]*2)+1] != 0x00 )	&&
		 ( client->decryptbuf[0x08] != 0x07 )							&&
		 ( client->decryptbuf[0x08] != 0x79 ) )
		dont_send = 1;

	if ( ( client->decryptbuf[0x08] == 0x07 ) && 
		 ( client->decryptbuf[0x0C] != client->clientID ) )
		dont_send = 1;

	if ( client->decryptbuf[0x08] == 0x72 )
		dont_send = 1;

	if (!dont_send)
	{
		switch ( client->decryptbuf[0x08] )
		{
		case 0x07:
			// Symbol chat (throttle for spam)
			dont_send = 1;
			if ( ( ((unsigned) servertime - client->command_cooldown[0x07]) >= 1 ) && ( !stfu ( client->guildcard ) ) )
			{
				client->command_cooldown[0x07] = (unsigned) servertime;
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
						if ( ( !ignored ) && ( lClient->guildcard != client->guildcard ) )
						{
							cipher_ptr = &lClient->server_cipher;
							encryptcopy ( lClient, &client->decryptbuf[0x00], size );
						}
					}
				}
			}
			break;
		case 0x0A:
			if ( client->lobbyNum > 0x0F )
			{
				// Player hit a monster
				mid = *(unsigned short*) &client->decryptbuf[0x0A];
				mid &= 0xFFF;
				if ( ( mid < 0xB50 ) && ( l->floor[client->clientID] != 0 ) )
				{
					mHP = *(short*) &client->decryptbuf[0x0E];
					l->monsterData[mid].HP = mHP;
				}
			}
			else
				client->todc = 1;
			break;
		case 0x1F:
			// Remember client's position.
			l->floor[client->clientID] = client->decryptbuf[0x0C];
			break;
		case 0x20:
			// Remember client's position.
			l->floor[client->clientID] = client->decryptbuf[0x0C];
			l->clienty[client->clientID] = *(unsigned *) &client->decryptbuf[0x18];
			l->clientx[client->clientID] = *(unsigned *) &client->decryptbuf[0x10];
			break;
		case 0x25:
			if ( ( client->lobbyNum > 0x0F ) && ( client->decryptbuf[0x0A] == client->clientID ) )
			{
				itemid = *(unsigned *) &client->decryptbuf[0x0C];
				EquipItem (itemid, client);
			}
			else
			{
				dont_send = 1;
				if ( client->lobbyNum < 0x10 )
					client->todc = 1;
			}
			break;
		case 0x26:
			if ( ( client->lobbyNum > 0x0F ) && ( client->decryptbuf[0x0A] == client->clientID ) )
			{
				itemid = *(unsigned *) &client->decryptbuf[0x0C];
				DeequipItem (itemid, client);
			}
			else
			{
				dont_send = 1;
				if ( client->lobbyNum < 0x10 )
					client->todc = 1;
			}
			break;
		case 0x27:
			// Use item
			if ( ( client->lobbyNum > 0x0F ) && ( client->decryptbuf[0x0A] == client->clientID ) )
			{
				itemid = *(unsigned *) &client->decryptbuf[0x0C];
				UseItem (itemid, client);
			}
			else
			{
				dont_send = 1;
				if ( client->lobbyNum < 0x10 )
					client->todc = 1;
			}
			break;
		case 0x28:
			// Mag feeding
			if ( ( client->lobbyNum > 0x0F ) && ( client->decryptbuf[0x0A] == client->clientID ) )
			{
				magid = *(unsigned *) &client->decryptbuf[0x0C];
				itemid = *(unsigned *) &client->decryptbuf[0x10];
				FeedMag ( magid, itemid, client );
			}
			else
			{
				dont_send = 1;
				if ( client->lobbyNum < 0x10 )
					client->todc = 1;
			}
			break;
		case 0x29:
			if ( ( client->lobbyNum > 0x0F ) && ( client->decryptbuf[0x0A] == client->clientID ) )
			{
				// Client dropping or destroying an item...
				itemid = *(unsigned *) &client->decryptbuf[0x0C];
				count = *(unsigned *) &client->decryptbuf[0x10];
				if (client->drop_item == itemid)
				{
					client->drop_item = 0;
					drop = 1;
				}
				else
					drop = 0;
				if (itemid != 0xFFFFFFFF)
					DeleteItemFromClient (itemid, count, drop, client); // Item
				else
					DeleteMesetaFromClient (count, drop, client); // Meseta
			}
			else
			{
				dont_send = 1;
				if ( client->lobbyNum < 0x10 )
					client->todc = 1;
			}
			break;
		case 0x2A:
			if ( ( client->lobbyNum > 0x0F ) && ( client->decryptbuf[0x0A] == client->clientID ) )
			{
				// Client dropping complete item
				itemid = *(unsigned*) &client->decryptbuf[0x10];
				DeleteItemFromClient (itemid, 0, 1, client);
			}
			else
			{
				dont_send = 1;
				if ( client->lobbyNum < 0x10 )
					client->todc = 1;
			}
			break;
		case 0x3E:
		case 0x3F:
			l->clientx[client->clientID] = *(unsigned *) &client->decryptbuf[0x14];
			l->clienty[client->clientID] = *(unsigned *) &client->decryptbuf[0x1C];
			break;
		case 0x40:
		case 0x42:
			l->clientx[client->clientID] = *(unsigned *) &client->decryptbuf[0x0C];
			l->clienty[client->clientID] = *(unsigned *) &client->decryptbuf[0x10];
			client->dead = 0;
			break;
		case 0x47:
		case 0x48:
			if (l->floor[client->clientID] == 0)
			{
				Send1A ("Using techniques on Pioneer 2 is disallowed.", client);
				dont_send = 1;
				client->todc = 1;
				break;
			}
			else
				if (client->clientID == client->decryptbuf[0x0A])
				{
					if (client->equip_flags & DROID_FLAG)
					{
						Send1A ("Androids cannot cast techniques.", client);
						dont_send = 1;
						client->todc = 1;
					}
					else
					{
						if (client->decryptbuf[0x0C] > 18)
						{
							Send1A ("Invalid technique cast.", client);
							dont_send = 1;
							client->todc = 1;
						}
						else
						{
							if (max_tech_level[client->decryptbuf[0x0C]][client->character._class] == -1)
							{
								Send1A ("You cannot cast that technique.", client);
								dont_send = 1;
								client->todc = 1;
							}
						}
					}
				}
			break;
		case 0x4D:
			// Decrease mag sync on death
			if ( ( client->lobbyNum > 0x0F ) && ( client->decryptbuf[0x0A] == client->clientID ) )
			{
				client->dead = 1;
				for (ch=0;ch<client->character.inventoryUse;ch++)
				{
					if ((client->character.inventory[ch].item.data[0] == 0x02) &&
						(client->character.inventory[ch].flags & 0x08))
					{
						if (client->character.inventory[ch].item.data2[0] >= 5)
							client->character.inventory[ch].item.data2[0] -= 5;
						else
							client->character.inventory[ch].item.data2[0] = 0;
					}
				}
			}
			else
			{
				dont_send = 1;
				if ( client->lobbyNum < 0x10 )
					client->todc = 1;
			}		
			break;
		case 0x68:
			// Telepipe check
			if ( ( client->lobbyNum < 0x10 ) || ( client->decryptbuf[0x0E] > 0x11 ) )
			{
				Send1A ("Incorrect telepipe.", client);
				dont_send = 1;
				client->todc = 1;
			}
			break;
		case 0x74:
			// W/S (throttle for spam)
			dont_send = 1;
			if ( ( ((unsigned) servertime - client->command_cooldown[0x74]) >= 1 ) && ( !stfu ( client->guildcard ) ) )
			{
				client->command_cooldown[0x74] = (unsigned) servertime;
				ws_ok = 1;
				ws_data = *(unsigned short*) &client->decryptbuf[0x0C];
				if ( ( ws_data == 0 ) || ( ws_data > 3 ) )
					ws_ok = 0;
				ws_data = *(unsigned short*) &client->decryptbuf[0x0E];
				if ( ( ws_data == 0 ) || ( ws_data > 3 ) )
					ws_ok = 0;
				if ( ws_ok )
				{
					for (ch=0;ch<client->decryptbuf[0x0C];ch++)
					{
						ws_data = *(unsigned short*) &client->decryptbuf[0x10+(ch*2)];
						if ( ws_data > 0x685 )
						{
							if ( ws_data > 0x697 )
								ws_ok = 0;
							else
							{
								ws_data -= 0x68C;
								if ( ws_data >= l->lobbyCount )
									ws_ok = 0;
							}
						}
					}
					ws_data = 0xFFFF;
					for (ch=client->decryptbuf[0x0C];ch<8;ch++)
						*(unsigned short*) &client->decryptbuf[0x10+(ch*2)] = ws_data;

					if ( ws_ok ) 
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
								if ( ( !ignored ) && ( lClient->guildcard != client->guildcard ) )
								{
									cipher_ptr = &lClient->server_cipher;
									encryptcopy ( lClient, &client->decryptbuf[0x00], size );
								}
							}
						}
					}
				}
			}
			break;
		case 0x75:
			{
				// Set player flag

				unsigned short flag;

				if (!client->decryptbuf[0x0E])
				{
					flag = *(unsigned short*) &client->decryptbuf[0x0C];
					if ( flag < 1024 )
						client->character.quest_data1[((unsigned)l->difficulty * 0x80) + (flag >> 3)] |= 1 << (7 - ( flag & 0x07 ));
				}
			}
			break;
		case 0xC0:
			// Client selling item
			if ( ( client->lobbyNum > 0x0F ) && ( l->floor[client->clientID] == 0 ) )
			{
				itemid = *(unsigned *) &client->decryptbuf[0x0C];
				for (ch=0;ch<client->character.inventoryUse;ch++)
				{
					if (client->character.inventory[ch].item.itemid == itemid)
					{
						count = client->decryptbuf[0x10];
						if ((count > 1) && (client->character.inventory[ch].item.data[0] != 0x03))
							client->todc = 1;
						else
						{
							shop_price = GetShopPrice ( &client->character.inventory[ch] ) * count;
							DeleteItemFromClient ( itemid, count, 0, client );
							if (!client->todc)
							{
								client->character.meseta += shop_price;
								if ( client->character.meseta > 999999 )
									client->character.meseta = 999999;
							}
						}
						break;
					}
				}
				if (client->todc)
					dont_send = 1;
			}
			else
			{
				if ( client->lobbyNum < 0x10 )
					client->todc = 1;
			}
			break;
		case 0xC3:
			// Client setting coordinates for stack drop
			if ( ( client->lobbyNum > 0x0F ) && ( client->decryptbuf[0x0A] == client->clientID ) )
			{
				client->drop_area = *(unsigned *) &client->decryptbuf[0x0C];
				client->drop_coords = *(long long*) &client->decryptbuf[0x10];
				client->drop_item = *(unsigned *) &client->decryptbuf[0x18];
			}
			else
			{
				dont_send = 1;
				if ( client->lobbyNum < 0x10 )
					client->todc = 1;
			}
			break;
		case 0xC4:
			// Inventory sort
			if ( client->lobbyNum > 0x0F )
			{
				dont_send = 1;
				SortClientItems ( client );
			}
			else
				client->todc = 1;
			break;
		case 0xC5:
			// Visiting hospital
			if ( client->lobbyNum > 0x0F )
				DeleteMesetaFromClient (10, 0, client);
			else
				client->todc = 1;
			break;
		case 0xC6:
			// Steal Exp
			if ( client->lobbyNum > 0x0F )
			{
				unsigned exp_percent = 0;
				unsigned exp_to_add;
				unsigned char special = 0;

				mid = *(unsigned short*) &client->decryptbuf[0x0C];
				mid &= 0xFFF;
				if (mid < 0xB50)
				{
					for (ch=0;ch<client->character.inventoryUse;ch++)
					{
						if ((client->character.inventory[ch].flags & 0x08)	&&
							(client->character.inventory[ch].item.data[0] == 0x00))
						{
							if ((client->character.inventory[ch].item.data[1] < 0x0A)	&&
								(client->character.inventory[ch].item.data[2] < 0x05))
								special = (client->character.inventory[ch].item.data[4] & 0x1F);
							else
								if ((client->character.inventory[ch].item.data[1] < 0x0D)	&&
									(client->character.inventory[ch].item.data[2] < 0x04))
									special = (client->character.inventory[ch].item.data[4] & 0x1F);
								else
									special = special_table[client->character.inventory[ch].item.data[1]]
								[client->character.inventory[ch].item.data[2]];
								switch (special)
								{
								case 0x09:
									// Master's
									exp_percent = 8;
									break;
								case 0x0A:
									// Lord's
									exp_percent = 10;
									break;
								case 0x0B:
									// King's
									exp_percent = 12;
									if (( l->difficulty	== 0x03 ) &&
										( client->equip_flags & DROID_FLAG ))
										exp_percent += 30;
									break;
								}
								break;
						}
					}

					if (exp_percent)
					{
						exp_to_add = ( l->mapData[mid].exp * exp_percent ) / 100L;
						if ( exp_to_add > 80 )  // Limit the amount of exp stolen to 80
							exp_to_add = 80;
						AddExp ( exp_to_add, client );
					}
				}
			}
			else
				client->todc = 1;
			break;
		case 0xC7:
			// Charge action
			if ( client->lobbyNum > 0x0F )
			{
				int meseta;

				meseta = *(int *) &client->decryptbuf[0x0C];
				if (meseta > 0)
				{
					if (client->character.meseta >= (unsigned) meseta)
						DeleteMesetaFromClient (meseta, 0, client);
					else
						DeleteMesetaFromClient (client->character.meseta, 0, client);
				}
				else
				{
					meseta = -meseta;
					client->character.meseta += (unsigned) meseta;
					if ( client->character.meseta > 999999 )
						client->character.meseta = 999999;
				}
			}
			else
				client->todc = 1;
			break;
		case 0xC8:
			// Monster is dead
			if ( client->lobbyNum > 0x0F )
			{
				mid = *(unsigned short*) &client->decryptbuf[0x0A];
				mid &= 0xFFF;
				if ( mid < 0xB50 )
				{
					if ( l->monsterData[mid].dead[client->clientID] == 0 )
					{
						l->monsterData[mid].dead[client->clientID] = 1;

						XP = l->mapData[mid].exp * EXPERIENCE_RATE;

						if (!l->quest_loaded)
						{
							mid_mismatch = 0;

							switch ( l->episode )
							{
							case 0x01:
								if ( l->floor[client->clientID] > 10 )
								{
									switch ( l->floor[client->clientID] )
									{
									case 11:
										// Dragon
										if ( l->mapData[mid].base != 192 )
											mid_mismatch = 1;
										break;
									case 12:
										// De Rol Le
										if ( l->mapData[mid].base != 193 )
											mid_mismatch = 1;
										break;
									case 13:
										// Vol Opt
										if ( ( l->mapData[mid].base != 197 ) && ( l->mapData[mid].base != 194 ) )
											mid_mismatch = 1;
										break;
									case 14:
										// Dark Falz
										if ( l->mapData[mid].base != 200 )
											mid_mismatch = 1;
										break;
									}
								}
								break;
							case 0x02:
								if ( l->floor[client->clientID] > 10 )
								{
									switch ( l->floor[client->clientID] )
									{
									case 12:
										// Gal Gryphon
										if ( l->mapData[mid].base != 192 )
											mid_mismatch = 1;
										break;
									case 13:
										// Olga Flow
										if ( l->mapData[mid].base != 202 )
											mid_mismatch = 1;
										break;
									case 14:
										// Barba Ray
										if ( l->mapData[mid].base != 203 )
											mid_mismatch = 1;
										break;
									case 15:
										// Gol Dragon
										if ( l->mapData[mid].base != 204 )
											mid_mismatch = 1;
										break;
									}
								}
								break;
							case 0x03:
								if ( ( l->floor[client->clientID] == 9 ) &&
									( l->mapData[mid].base != 280 ) && 
									( l->mapData[mid].base != 281 ) && 
									( l->mapData[mid].base != 41 ) )
									mid_mismatch = 1;
								break;
							}

							if ( mid_mismatch )
							{
								SendEE ("Client/server data synchronization error.  Please reinstall your client and all patches.", client);
								client->todc = 1;
							}
						}

						//debug ("mid death: %u  base: %u, skin: %u, reserved11: %f, exp: %u", mid, l->mapData[mid].base, l->mapData[mid].skin, l->mapData[mid].reserved11, XP);

						if ( client->decryptbuf[0x10] != 1 ) // Not the last player who hit?
							XP = ( XP * 77 ) / 100L;

						if ( client->character.level < 199 )
							AddExp ( XP, client );

						// Increase kill counters for SJS, Lame d'Argent, Limiter and Swordsman Lore

						for (ch=0;ch<client->character.inventoryUse;ch++)
						{
							if (client->character.inventory[ch].flags & 0x08)
							{
								counter = 0;
								switch (client->character.inventory[ch].item.data[0])
								{
								case 0x00:
									if ((client->character.inventory[ch].item.data[1] == 0x33) ||
										(client->character.inventory[ch].item.data[1] == 0xAB))
										counter = 1;
									break;
								case 0x01:
									if ((client->character.inventory[ch].item.data[1] == 0x03) &&
										((client->character.inventory[ch].item.data[2] == 0x4D) ||
										(client->character.inventory[ch].item.data[2] == 0x4F)))
										counter = 1;
									break;
								default:
									break;
								}
								if (counter)
								{
									counter = *(unsigned short*) &client->character.inventory[ch].item.data[10];
									if (counter < 0x8000)
										counter = 0x8000;
									counter++;
									*(unsigned short*) &client->character.inventory[ch].item.data[10] = counter;
								}
							}
						}
					}
				}
			}
			else
				client->todc = 1;
			break;
		case 0xCC:
			// Exchange item for team points
			{
				unsigned deleteid;

				deleteid = *(unsigned*) &client->decryptbuf[0x0C];
				DeleteItemFromClient (deleteid, 1, 0, client);
				if (!client->todc)
				{
					SendB0 ("Item donated to server.", client);
				}
			}
			break;
		case 0xCF:
			if ((l->battle) && (client->mode))
			{
				// Battle restarting...
				//
				// If rule #1 we'll copy the character backup to the character array, otherwise
				// we'll reset the character...
				//
				for (ch=0;ch<4;ch++)
				{
					if ((l->slot_use[ch]) && (l->client[ch]))
					{
						lClient = l->client[ch];
						switch (lClient->mode)
						{
						case 0x01:
						case 0x02:
							// Copy character backup
							if (lClient->character_backup)
								memcpy (&lClient->character, lClient->character_backup, sizeof (lClient->character));
							if (lClient->mode == 0x02)
							{
								for (ch2=0;ch2<lClient->character.inventoryUse;ch2++)
								{
									if (lClient->character.inventory[ch2].item.data[0] == 0x02)
										lClient->character.inventory[ch2].in_use = 0;
								}
								CleanUpInventory (lClient);
								lClient->character.meseta = 0;
							}
							break;
						case 0x03:
							// Wipe items and reset level.
							for (ch2=0;ch2<30;ch2++)
								lClient->character.inventory[ch2].in_use = 0;
							CleanUpInventory (lClient);
							lClient->character.level = 0;
							lClient->character.XP = 0;
							lClient->character.ATP = *(unsigned short*) &startingData[(lClient->character._class*14)];
							lClient->character.MST = *(unsigned short*) &startingData[(lClient->character._class*14)+2];
							lClient->character.EVP = *(unsigned short*) &startingData[(lClient->character._class*14)+4];
							lClient->character.HP  = *(unsigned short*) &startingData[(lClient->character._class*14)+6];
							lClient->character.DFP = *(unsigned short*) &startingData[(lClient->character._class*14)+8];
							lClient->character.ATA = *(unsigned short*) &startingData[(lClient->character._class*14)+10];
							if (l->battle_level > 1)
								SkipToLevel (l->battle_level - 1, lClient, 1);
							lClient->character.meseta = 0;
							break;
						default:
							// Unknown mode?
							break;
						}
					}
				}
				// Reset boxes and monsters...
				memset (&l->boxHit, 0, 0xB50); // Reset box and monster data
				memset (&l->monsterData, 0, sizeof (l->monsterData));
			}
			break;
		case 0xD2:
			// Gallon seems to write to this area...
			dont_send = 1;
			if ( client->lobbyNum > 0x0F )
			{
				unsigned qofs;

				qofs = *(unsigned *) &client->decryptbuf[0x0C];
				if (qofs < 23)
				{
					qofs *= 4;
					*(unsigned *) &client->character.quest_data2[qofs] = *(unsigned*) &client->decryptbuf[0x10];
					memcpy ( &client->encryptbuf[0x00], &client->decryptbuf[0x00], 0x14 );
					cipher_ptr = &client->server_cipher;
					encryptcopy ( client, &client->decryptbuf[0x00], 0x14 );
				}
			}
			else
			{
				dont_send = 1;
				client->todc = 1;
			}
			break;
		case 0xD5:
			// Exchange an item
			if ( ( client->lobbyNum > 0x0F ) && ( client->decryptbuf[0x0A] == client->clientID ) )
			{
				INVENTORY_ITEM work_item;
				unsigned compare_item = 0, ci;

				memset (&work_item, 0, sizeof (INVENTORY_ITEM));
				memcpy (&work_item.item.data[0], &client->decryptbuf[0x0C], 3 );
				DeleteFromInventory (&work_item, 1, client);

				if (!client->todc)
				{
					memset (&work_item, 0, sizeof (INVENTORY_ITEM));
					memcpy (&compare_item, &client->decryptbuf[0x20], 3 );
					for ( ci = 0; ci < quest_numallows; ci ++)
					{
						if (compare_item == quest_allow[ci])
						{
							memcpy (&work_item.item.data[0], &client->decryptbuf[0x20], 3 );
							work_item.item.itemid = l->playerItemID[client->clientID];
							l->playerItemID[client->clientID]++;
							AddToInventory (&work_item, 1, 0, client);
							memset (&client->encryptbuf[0x00], 0, 0x0C);
							client->encryptbuf[0x00] = 0x0C;
							client->encryptbuf[0x02] = 0xAB;
							client->encryptbuf[0x03] = 0x01;
							// BLAH :)
							*(unsigned short*) &client->encryptbuf[0x08] = *(unsigned short*) &client->decryptbuf[0x34];
							cipher_ptr = &client->server_cipher;
							encryptcopy (client, &client->encryptbuf[0x00], 0x0C);
							break;
						}
					}
					if ( !work_item.item.itemid )
					{
						Send1A ("Attempting to exchange for disallowed item.", client);
						client->todc = 1;
					}
				}
			}
			else
			{
				dont_send = 1;
				client->todc = 1;
			}
			break;
		case 0xD7:
			// Trade PDs for an item from Hopkins' dad
			if ( ( client->lobbyNum > 0x0F ) && ( client->decryptbuf[0x0A] == client->clientID ) )
			{
				INVENTORY_ITEM work_item;
				unsigned ci, compare_item = 0;

				memset ( &work_item, 0, sizeof (INVENTORY_ITEM) );
				memcpy ( &compare_item, &client->decryptbuf[0x0C], 3 );
				for ( ci = 0; ci < (sizeof (gallons_shop_hopkins) / 4); ci += 2)
				{
					if (compare_item == gallons_shop_hopkins[ci])
					{
						work_item.item.data[0] = 0x03;
						work_item.item.data[1] = 0x10;
						work_item.item.data[2] = 0x00;
						break;
					}
				}
				if ( work_item.item.data[0] == 0x03 )
				{
					DeleteFromInventory (&work_item, 0xFF, client); // Delete all Photon Drops
					if (!client->todc)
					{
						memcpy (&work_item.item.data[0], &client->decryptbuf[0x0C], 12);
						*(unsigned *) &work_item.item.data2[0] = *(unsigned *) &client->decryptbuf[0x18];
						work_item.item.itemid = l->playerItemID[client->clientID];
						l->playerItemID[client->clientID]++;
						AddToInventory (&work_item, 1, 0, client);
						memset (&client->encryptbuf[0x00], 0, 0x0C);
						// I guess this is a sort of action confirmed by server thing...
						// Also starts an animation and sound... with the wrong values, the camera pans weirdly...
						client->encryptbuf[0x00] = 0x0C;
						client->encryptbuf[0x02] = 0xAB;
						client->encryptbuf[0x03] = 0x01;
						*(unsigned short*) &client->encryptbuf[0x08] = *(unsigned short*) &client->decryptbuf[0x20];
						cipher_ptr = &client->server_cipher;
						encryptcopy (client, &client->encryptbuf[0x00], 0x0C);
					}
				}
				else
				{
					Send1A ("No photon drops in user's inventory\nwhen encountering exchange command.", client);
					dont_send = 1;
					client->todc = 1;
				}
			}
			else
			{
				dont_send = 1;
				if ( client->lobbyNum < 0x10 )
					client->todc = 1;
			}
			break;
		case 0xD8:
			// Add attribute to S-rank weapon (not implemented yet)
			break;
		case 0xD9:
			// Momoka Item Exchange
			{
				unsigned compare_item, ci;
				unsigned itemid = 0;
				INVENTORY_ITEM add_item;

				dont_send = 1;

				if ( client->lobbyNum > 0x0F )
				{
					compare_item = 0x00091203;
					for ( ci=0; ci < client->character.inventoryUse; ci++)
					{
						if ( *(unsigned *) &client->character.inventory[ci].item.data[0] == compare_item )
						{
							itemid = client->character.inventory[ci].item.itemid;
							break;
						}
					}
					if (!itemid)
					{
						memset (&client->encryptbuf[0x00], 0, 8);
						client->encryptbuf[0x00] = 0x08;
						client->encryptbuf[0x02] = 0x23;
						client->encryptbuf[0x04] = 0x01;
						cipher_ptr = &client->server_cipher;
						encryptcopy (client, &client->encryptbuf[0x00], 8);
					}
					else
					{
						memset (&add_item, 0, sizeof (INVENTORY_ITEM));
						compare_item = *(unsigned *) &client->decryptbuf[0x20];
						for ( ci=0; ci < quest_numallows; ci++)
						{
							if ( compare_item == quest_allow[ci] )
							{
								*(unsigned *) &add_item.item.data[0] = *(unsigned *) &client->decryptbuf[0x20];
								break;
							}
						}
						if (*(unsigned *) &add_item.item.data[0] == 0)
						{
							client->todc = 1;
							Send1A ("Requested item not allowed.", client);
						}
						else
						{
							DeleteItemFromClient (itemid, 1, 0, client);
							memset (&client->encryptbuf[0x00], 0, 0x18);
							client->encryptbuf[0x00] = 0x18;
							client->encryptbuf[0x02] = 0x60;
							client->encryptbuf[0x08] = 0xDB;
							client->encryptbuf[0x09] = 0x06;
							client->encryptbuf[0x0C] = 0x01;
							*(unsigned *) &client->encryptbuf[0x10] = itemid;
							client->encryptbuf[0x14] = 0x01;
							cipher_ptr = &client->server_cipher;
							encryptcopy (client, &client->encryptbuf[0x00], 0x18);

							// Let everybody else know that item no longer exists...

							memset (&client->encryptbuf[0x00], 0, 0x14);
							client->encryptbuf[0x00] = 0x14;
							client->encryptbuf[0x02] = 0x60;
							client->encryptbuf[0x08] = 0x29;
							client->encryptbuf[0x09] = 0x05;
							client->encryptbuf[0x0A] = client->clientID;
							*(unsigned *) &client->encryptbuf[0x0C] = itemid;
							client->encryptbuf[0x10] = 0x01;
							SendToLobby ( l, 4, &client->encryptbuf[0x00], 0x14, client->guildcard );
							add_item.item.itemid = l->playerItemID[client->clientID];
							l->playerItemID[client->clientID]++;
							AddToInventory (&add_item, 1, 0, client);
							memset (&client->encryptbuf[0x00], 0, 8);
							client->encryptbuf[0x00] = 0x08;
							client->encryptbuf[0x02] = 0x23;
							client->encryptbuf[0x04] = 0x00;
							cipher_ptr = &client->server_cipher;
							encryptcopy (client, &client->encryptbuf[0x00], 8);
						}
					}
				}
			}
			break;
		case 0xDA:
			// Upgrade Photon of weapon
			if ( ( client->lobbyNum > 0x0F ) && ( client->decryptbuf[0x0A] == client->clientID ) )
			{
				INVENTORY_ITEM work_item, work_item2;
				unsigned ci, ai,
					compare_itemid = 0, compare_item1 = 0, compare_item2 = 0, num_attribs = 0;
				char attrib_add;

				memcpy ( &compare_item1,  &client->decryptbuf[0x0C], 3);
				compare_itemid = *(unsigned *) &client->decryptbuf[0x20];
				for ( ci=0; ci < client->character.inventoryUse; ci++)
				{
					memcpy (&compare_item2, &client->character.inventory[ci].item.data[0], 3);
					if ( ( client->character.inventory[ci].item.itemid == compare_itemid ) && 
						( compare_item1 == compare_item2 ) && ( client->character.inventory[ci].item.data[0] == 0x00 ) )
					{
						memset (&work_item, 0, sizeof (INVENTORY_ITEM));
						work_item.item.data[0] = 0x03;
						work_item.item.data[1] = 0x10;
						if ( client->decryptbuf[0x2C] )
							work_item.item.data[2] = 0x01;
						else
							work_item.item.data[2] = 0x00;
						// Copy before shift
						memcpy ( &work_item2, &client->character.inventory[ci], sizeof (INVENTORY_ITEM) );
						DeleteFromInventory ( &work_item, client->decryptbuf[0x28], client );
						if (!client->todc)
						{
							switch (client->decryptbuf[0x28])
							{
							case 0x01:
								// 1 PS = 30%
								if ( client->decryptbuf[0x2C] )
									attrib_add = 30;
								break;
							case 0x04:
								// 4 PDs = 1%
								attrib_add = 1;
								break;
							case 0x14:
								// 20 PDs = 5%
								attrib_add = 5;
								break;
							default:
								attrib_add = 0;
								break;
							}
							ai = 0;
							if ((work_item2.item.data[6] > 0x00) && 
								(!(work_item2.item.data[6] & 128)))
							{
								num_attribs++;
								if (work_item2.item.data[6] == client->decryptbuf[0x24])
									ai = 7;
							}
							if ((work_item2.item.data[8] > 0x00) && 
								(!(work_item2.item.data[8] & 128)))
							{
								num_attribs++;
								if (work_item2.item.data[8] == client->decryptbuf[0x24])
									ai = 9;
							}
							if ((work_item2.item.data[10] > 0x00) && 
								(!(work_item2.item.data[10] & 128)))
							{
								num_attribs++;
								if (work_item2.item.data[10] == client->decryptbuf[0x24])
									ai = 11;
							}
							if (ai)
							{
								// Attribute already on weapon, increase it
								work_item2.item.data[ai] += attrib_add;
								if (work_item2.item.data[ai] > 100)
									work_item2.item.data[ai] = 100;
							}
							else
							{
								// Attribute not on weapon, add it if there isn't already 3 attributes
								if (num_attribs < 3)
								{
									work_item2.item.data[6 + (num_attribs * 2)] = client->decryptbuf[0x24];
									work_item2.item.data[7 + (num_attribs * 2)] = attrib_add;
								}
							}
							DeleteItemFromClient ( work_item2.item.itemid, 1, 0, client );
							memset (&client->encryptbuf[0x00], 0, 0x14);
							client->encryptbuf[0x00] = 0x14;
							client->encryptbuf[0x02] = 0x60;
							client->encryptbuf[0x08] = 0x29;
							client->encryptbuf[0x09] = 0x05;
							client->encryptbuf[0x0A] = client->clientID;
							*(unsigned *) &client->encryptbuf[0x0C] = work_item2.item.itemid;
							client->encryptbuf[0x10] = 0x01;
							SendToLobby ( client->lobby, 4, &client->encryptbuf[0x00], 0x14, 0 );
							AddToInventory ( &work_item2, 1, 0, client );
							memset (&client->encryptbuf[0x00], 0, 0x0C);
							// Don't know...
							client->encryptbuf[0x00] = 0x0C;
							client->encryptbuf[0x02] = 0xAB;
							client->encryptbuf[0x03] = 0x01;
							*(unsigned short*) &client->encryptbuf[0x08] = *(unsigned short*) &client->decryptbuf[0x30];
							cipher_ptr = &client->server_cipher;
							encryptcopy (client, &client->encryptbuf[0x00], 0x0C);
						}
						break;
					}
				}
				if (client->todc)
					dont_send = 1;
			}
			else
			{
				dont_send = 1;
				if ( client->lobbyNum < 0x10 )
					client->todc = 1;
			}
			break;
		case 0xDE:
			// Good Luck
			{
				unsigned compare_item, ci;
				unsigned itemid = 0;
				INVENTORY_ITEM add_item;

				dont_send = 1;

				if ( client->lobbyNum > 0x0F )
				{
					compare_item = 0x00031003;
					for ( ci=0; ci < client->character.inventoryUse; ci++)
					{
						if ( *(unsigned *) &client->character.inventory[ci].item.data[0] == compare_item )
						{
							itemid = client->character.inventory[ci].item.itemid;
							break;
						}
					}
					if (!itemid)
					{
						memset (&client->encryptbuf[0x00], 0, 0x2C);
						client->encryptbuf[0x00] = 0x2C;
						client->encryptbuf[0x02] = 0x24;
						client->encryptbuf[0x04] = 0x01;
						client->encryptbuf[0x08] = client->decryptbuf[0x0E];
						client->encryptbuf[0x0A] = client->decryptbuf[0x0D];
						for (ci=0;ci<8;ci++)
							client->encryptbuf[0x0C + (ci << 2)] = (mt_lrand() % (sizeof(good_luck) >> 2)) + 1;
						cipher_ptr = &client->server_cipher;
						encryptcopy (client, &client->encryptbuf[0x00], 0x2C);
					}
					else
					{
						memset (&add_item, 0, sizeof (INVENTORY_ITEM));
						*(unsigned *) &add_item.item.data[0] = good_luck[mt_lrand() % (sizeof(good_luck) >> 2)];
						DeleteItemFromClient (itemid, 1, 0, client);
						memset (&client->encryptbuf[0x00], 0, 0x18);
						client->encryptbuf[0x00] = 0x18;
						client->encryptbuf[0x02] = 0x60;
						client->encryptbuf[0x08] = 0xDB;
						client->encryptbuf[0x09] = 0x06;
						client->encryptbuf[0x0C] = 0x01;
						*(unsigned *) &client->encryptbuf[0x10] = itemid;
						client->encryptbuf[0x14] = 0x01;
						cipher_ptr = &client->server_cipher;
						encryptcopy (client, &client->encryptbuf[0x00], 0x18);

						// Let everybody else know that item no longer exists...

						memset (&client->encryptbuf[0x00], 0, 0x14);
						client->encryptbuf[0x00] = 0x14;
						client->encryptbuf[0x02] = 0x60;
						client->encryptbuf[0x08] = 0x29;
						client->encryptbuf[0x09] = 0x05;
						client->encryptbuf[0x0A] = client->clientID;
						*(unsigned *) &client->encryptbuf[0x0C] = itemid;
						client->encryptbuf[0x10] = 0x01;
						SendToLobby ( l, 4, &client->encryptbuf[0x00], 0x14, client->guildcard );
						add_item.item.itemid = l->playerItemID[client->clientID];
						l->playerItemID[client->clientID]++;
						AddToInventory (&add_item, 1, 0, client);
						memset (&client->encryptbuf[0x00], 0, 0x2C);
						client->encryptbuf[0x00] = 0x2C;
						client->encryptbuf[0x02] = 0x24;
						client->encryptbuf[0x04] = 0x00;
						client->encryptbuf[0x08] = client->decryptbuf[0x0E];
						client->encryptbuf[0x0A] = client->decryptbuf[0x0D];
						for (ci=0;ci<8;ci++)
							client->encryptbuf[0x0C + (ci << 2)] = (mt_lrand() % (sizeof(good_luck) >> 2)) + 1;
						cipher_ptr = &client->server_cipher;
						encryptcopy (client, &client->encryptbuf[0x00], 0x2C);
					}
				}
			}
			break;
		case 0xE1:
			{
				// Gallon's Plan opcode

				INVENTORY_ITEM work_item;
				unsigned ch, compare_item1, compare_item2, pt_itemid;

				compare_item2 = 0;
				compare_item1 = 0x041003;
				pt_itemid = 0;

				for (ch=0;ch<client->character.inventoryUse;ch++)
				{
					memcpy (&compare_item2, &client->character.inventory[ch].item.data[0], 3);
					if (compare_item1 == compare_item2)
					{
						pt_itemid = client->character.inventory[ch].item.itemid;
						break;
					}
				}

				if (!pt_itemid)
					client->todc = 1;

				if (!client->todc)
				{
					memset ( &work_item, 0, sizeof (INVENTORY_ITEM) );
					switch (client->decryptbuf[0x0E])
					{
					case 0x01:
						// Kan'ei Tsuho
						DeleteItemFromClient (pt_itemid, 10, 0, client); // Delete Photon Tickets
						if (!client->todc)
						{
							work_item.item.data[0] = 0x00;
							work_item.item.data[1] = 0xD5;
							work_item.item.data[2] = 0x00;
						}
						break;
					case 0x02:
						// Lollipop
						DeleteItemFromClient (pt_itemid, 15, 0, client); // Delete Photon Tickets
						if (!client->todc)
						{
							work_item.item.data[0] = 0x00;
							work_item.item.data[1] = 0x0A;
							work_item.item.data[2] = 0x07;
						}
						break;
					case 0x03:
						// Stealth Suit
						DeleteItemFromClient (pt_itemid, 20, 0, client); // Delete Photon Tickets
						if (!client->todc)
						{
							work_item.item.data[0] = 0x01;
							work_item.item.data[1] = 0x01;
							work_item.item.data[2] = 0x57;
						}
						break;
					default:
						client->todc = 1;
						break;
					}

					if (!client->todc)
					{
						memset (&client->encryptbuf[0x00], 0, 0x18);
						client->encryptbuf[0x00] = 0x18;
						client->encryptbuf[0x02] = 0x60;
						client->encryptbuf[0x08] = 0xDB;
						client->encryptbuf[0x09] = 0x06;
						client->encryptbuf[0x0C] = 0x01;
						*(unsigned *) &client->encryptbuf[0x10] = pt_itemid;
						client->encryptbuf[0x14] = 0x05 + ( client->decryptbuf[0x0E] * 5 );
						cipher_ptr = &client->server_cipher;
						encryptcopy ( client, &client->encryptbuf[0x00], 0x18 );
						work_item.item.itemid = l->playerItemID[client->clientID];
						l->playerItemID[client->clientID]++;
						AddToInventory (&work_item, 1, 0, client);
						// Gallon's Plan result
						memset (&client->encryptbuf[0x00], 0, 0x10);
						client->encryptbuf[0x00] = 0x10;
						client->encryptbuf[0x02] = 0x25;
						client->encryptbuf[0x08] = client->decryptbuf[0x10];
						client->encryptbuf[0x0A] = 0x3C;
						client->encryptbuf[0x0B] = 0x08;
						client->encryptbuf[0x0D] = client->decryptbuf[0x0E];
						client->encryptbuf[0x0E] = 0x9A;
						client->encryptbuf[0x0F] = 0x08;
						cipher_ptr = &client->server_cipher;
						encryptcopy (client, &client->encryptbuf[0x00], 0x10);
					}
				}
			}
			break;
		case 0x17:
		case 0x18:
			boss_floor = 0;
			switch (l->episode)
			{
			case 0x01:
				if ((l->floor[client->clientID] > 10) && (l->floor[client->clientID] < 15))
					boss_floor = 1;
				break;
			case 0x02:
				if ((l->floor[client->clientID] > 11) && (l->floor[client->clientID] < 16))
					boss_floor = 1;
				break;
			case 0x03:
				if (l->floor[client->clientID] == 9)
					boss_floor = 1;
				break;
			}
			if (!boss_floor)
				dont_send = 1;
			break;
		default:
			/* Temporary
			debug ("0x60 from %u:", client->guildcard);
			display_packet (&client->decryptbuf[0x00], size);
			WriteLog ("0x60 from %u\n%s\n\n:", client->guildcard, (char*) &dp[0]);
			*/
			break;
		}
		if ((!dont_send) && (!client->todc))
		{
			if ( client->lobbyNum < 0x10 )				
				SendToLobby ( client->lobby, 12, &client->decryptbuf[0], size, client->guildcard);
			else
				SendToLobby ( client->lobby, 4, &client->decryptbuf[0], size, client->guildcard);
		}
	}
}

unsigned long ExpandDropRate(unsigned char pc)
{
    long shift = ((pc >> 3) & 0x1F) - 4;
    if (shift < 0) shift = 0;
    return ((2 << (unsigned long) shift) * ((pc & 7) + 7));
}

void GenerateRandomAttributes (unsigned char sid, GAME_ITEM* i, LOBBY* l, CLIENT* client)
{
	unsigned ch, num_percents, max_percent, meseta, do_area, r;
	PTDATA* ptd;
	int rare;
	unsigned area;
	unsigned did_area[6] = {0};
	char percent;

	if ((!l) || (!i))
		return;

	if (l->episode == 0x02)
		ptd = &pt_tables_ep2[sid][l->difficulty];
	else
		ptd = &pt_tables_ep1[sid][l->difficulty];

	area = 0;

	switch ( l->episode )
	{
	case 0x01:
		switch ( l->floor[client->clientID ] )
		{
		case 11:
			// dragon
			area = 3;
			break;
		case 12:
			// de rol
			area = 6;
			break;
		case 13:
			// vol opt
			area = 8;
			break;
		case 14:
			// falz
			area = 10;
			break;
		default:
			area = l->floor[client->clientID ];
			break;
		}	
		break;
	case 0x02:
		switch ( l->floor[client->clientID ] )
		{
		case 14:
			// barba ray
			area = 3;
			break;
		case 15:
			// gol dragon
			area = 6;
			break;
		case 12:
			// gal gryphon
			area = 9;
			break;
		case 13:
			// olga flow
			area = 10;
			break;
		default:
			// could be tower
			if ( l->floor[client->clientID] <= 11 )
				area = ep2_rtremap[(l->floor[client->clientID] * 2)+1];
			else
				area = 0x0A;
			break;
		}	
		break;
	case 0x03:
		area = l->floor[client->clientID ] + 1;
		break;
	}

	if ( !area )
		return;

	if ( area > 10 )
		area = 10;

	area--; // Our tables are zero based.

	switch (i->item.data[0])
	{
	case 0x00:
		rare = 0;
		if ( i->item.data[1] > 0x0C )
			rare = 1;
		else
			if ( ( i->item.data[1] > 0x09 ) && ( i->item.data[2] > 0x03 ) )
				rare = 1;
			else
				if ( ( i->item.data[1] < 0x0A ) && ( i->item.data[2] > 0x04 ) )
					rare = 1;
		if ( !rare )
		{

			r = 100 - ( mt_lrand() % 100 );

			if ( ( r > ptd->element_probability[area] ) && ( ptd->element_ranking[area] ) )
			{
				i->item.data[4] = 0xFF;
				while (i->item.data[4] == 0xFF) // give a special
					i->item.data[4] = elemental_table[(12 * ( ptd->element_ranking[area] - 1 ) ) + ( mt_lrand() % 12 )];
			}
			else
				i->item.data[4] = 0;

			if ( i->item.data[4] )
				i->item.data[4] |= 0x80; // untekked

			// Add a grind

			if ( l->episode == 0x02 )
				ch = power_patterns_ep2[sid][l->difficulty][ptd->area_pattern[area]][mt_lrand() % 4096];
			else
				ch = power_patterns_ep1[sid][l->difficulty][ptd->area_pattern[area]][mt_lrand() % 4096];

			i->item.data[3] = (unsigned char) ch;
		}
		else
			i->item.data[4] |= 0x80; // rare

		num_percents = 0;

		if ( ( i->item.data[1] == 0x33 ) || 
			 ( i->item.data[1] == 0xAB ) ) // SJS and Lame max 2 percents
			max_percent = 2;
		else
			max_percent = 3;

		for (ch=0;ch<max_percent;ch++)
		{
			if (l->episode == 0x02)
				do_area = attachment_ep2[sid][l->difficulty][area][mt_lrand() % 4096];
			else
				do_area = attachment_ep1[sid][l->difficulty][area][mt_lrand() % 4096];
			if ( ( do_area ) && ( !did_area[do_area] ) )
			{
				did_area[do_area] = 1;
				i->item.data[6+(num_percents*2)] = (unsigned char) do_area;
				if ( l->episode == 0x02 )
					percent = percent_patterns_ep2[sid][l->difficulty][ptd->area_pattern[area]][mt_lrand() % 4096];
				else
					percent = percent_patterns_ep1[sid][l->difficulty][ptd->area_pattern[area]][mt_lrand() % 4096];
				percent -= 2;
				percent *= 5;
				i->item.data[6+(num_percents*2)+1] = percent;
				num_percents++;
			}
		}
		break;
	case 0x01:
		switch ( i->item.data[1] )
		{
		case 0x01:
			// Armor variance
			r = mt_lrand() % 11;
			if (r < 7)
			{
				if (armor_dfpvar_table[i->item.data[2]])
					i->item.data[6] = (unsigned char) (mt_lrand() % (armor_dfpvar_table[i->item.data[2]] + 1));
				if (armor_evpvar_table[i->item.data[2]])
					i->item.data[8] = (unsigned char) (mt_lrand() % (armor_evpvar_table[i->item.data[2]] + 1));
			}

			// Slots
			if ( l->episode == 0x02 )
				i->item.data[5] = slots_ep2[sid][l->difficulty][mt_lrand() % 4096]; 
			else
				i->item.data[5] = slots_ep1[sid][l->difficulty][mt_lrand() % 4096];
			break;
		case 0x02:
			// Shield variance
			r = mt_lrand() % 11;
			if (r < 2)
			{
				if (barrier_dfpvar_table[i->item.data[2]])
					i->item.data[6] = (unsigned char) (mt_lrand() % (barrier_dfpvar_table[i->item.data[2]] + 1));
				if (barrier_evpvar_table[i->item.data[2]])
					i->item.data[8] = (unsigned char) (mt_lrand() % (barrier_evpvar_table[i->item.data[2]] + 1));
			}
			break;
		}
		break;
	case 0x02:
		// Mag
		i->item.data [2]  = 0x05;
		i->item.data [4]  = 0xF4;
		i->item.data [5]  = 0x01;
		i->item.data2[3] = mt_lrand() % 0x11;
		break;
	case 0x03:
		if ( i->item.data[1] == 0x02 ) // Technique
		{
			if ( l->episode == 0x02 )
				i->item.data[4] = tech_drops_ep2[sid][l->difficulty][area][mt_lrand() % 4096];
			else
				i->item.data[4] = tech_drops_ep1[sid][l->difficulty][area][mt_lrand() % 4096];
			i->item.data[2] = (unsigned char) ptd->tech_levels[i->item.data[4]][area*2];
			if ( ptd->tech_levels[i->item.data[4]][(area*2)+1] > ptd->tech_levels[i->item.data[4]][area*2] )
				i->item.data[2] += (unsigned char) mt_lrand() % ( ( ptd->tech_levels[i->item.data[4]][(area*2)+1] - ptd->tech_levels[i->item.data[4]][(area*2)] ) + 1 );
		}
		if (stackable_table[i->item.data[1]])
			i->item.data[5] = 0x01;
		break;
	case 0x04:
		// meseta
		meseta = ptd->box_meseta[area][0];
		if ( ptd->box_meseta[area][1] > ptd->box_meseta[area][0] )
			meseta += mt_lrand() % ( ( ptd->box_meseta[area][1] - ptd->box_meseta[area][0] ) + 1 );
		*(unsigned *) &i->item.data2[0] = meseta;
		break;
	default:
		break;
	}
}

void GenerateCommonItem (int item_type, int is_enemy, unsigned char sid, GAME_ITEM* i, LOBBY* l, CLIENT* client)
{
	unsigned ch, num_percents, item_set, meseta, do_area, r, eq_type;
	unsigned short ch2;
	PTDATA* ptd;
	unsigned area,fl;
	unsigned did_area[6] = {0};
	char percent;

	if ((!l) || (!i))
		return;

	if (l->episode == 0x02)
		ptd = &pt_tables_ep2[sid][l->difficulty];
	else
		ptd = &pt_tables_ep1[sid][l->difficulty];

	area = 0;

	switch ( l->episode )
	{
	case 0x01:
		switch ( l->floor[client->clientID ] )
		{
		case 11:
			// dragon
			area = 3;
			break;
		case 12:
			// de rol
			area = 6;
			break;
		case 13:
			// vol opt
			area = 8;
			break;
		case 14:
			// falz
			area = 10;
			break;
		default:
			area = l->floor[client->clientID ];
			break;
		}	
		break;
	case 0x02:
		switch ( l->floor[client->clientID ] )
		{
		case 14:
			// barba ray
			area = 3;
			break;
		case 15:
			// gol dragon
			area = 6;
			break;
		case 12:
			// gal gryphon
			area = 9;
			break;
		case 13:
			// olga flow
			area = 10;
			break;
		default:
			// could be tower
			if ( l->floor[client->clientID] <= 11 )
				area = ep2_rtremap[(l->floor[client->clientID] * 2)+1];
			else
				area = 0x0A;
			break;
		}	
		break;
	case 0x03:
		area = l->floor[client->clientID ] + 1;
		break;
	}

	if ( !area )
		return;

	if ( ( l->battle ) && ( l->quest_loaded ) )
	{
		if ( ( !l->battle_level ) || ( l->battle_level > 5 ) )
			area = 6; // Use mines equipment for rule #1, #5 and #8
		else
			area = 3; // Use caves 1 equipment for all other rules...
	}

	if ( area > 10 )
		area = 10;

	fl = area;
	area--; // Our tables are zero based.

	if (is_enemy)
	{
		if ( ( mt_lrand() % 100 ) > 40)
			item_set = 3;
		else
		{
			switch (item_type)
			{
			case 0x00:
				item_set = 0;
				break;
			case 0x01:
				item_set = 1;
				break;
			case 0x02:
				item_set = 1;
				break;
			case 0x03:
				item_set = 1;
				break;
			default:
				item_set = 3;
				break;
			}
		}
	}
	else
	{
		if ( ( l->meseta_boost ) && ( ( mt_lrand() % 100 ) > 25 ) )
			item_set = 4; // Boost amount of meseta dropped during rules #4 and #5
		else
		{
			if ( item_type == 0xFF )
				item_set = common_table[mt_lrand() % 100000];
			else
				item_set = item_type;
		}
	}

	memset (&i->item.data[0], 0, 12);
	memset (&i->item.data2[0], 0, 4);

	switch (item_set)
	{
	case 0x00:
		// Weapon

		if ( l->episode == 0x02 )
			ch2 = weapon_drops_ep2[sid][l->difficulty][area][mt_lrand() % 4096];
		else
			ch2 = weapon_drops_ep1[sid][l->difficulty][area][mt_lrand() % 4096];

		i->item.data[1] = ch2 & 0xFF;
		i->item.data[2] = ch2 >> 8;

		if (i->item.data[1] > 0x09)
		{
			if (i->item.data[2] > 0x03)
				i->item.data[2] = 0x03;
		}
		else
		{
			if (i->item.data[2] > 0x04)
				i->item.data[2] = 0x04;
		}
		
		r = 100 - ( mt_lrand() % 100 );

		if ( ( r > ptd->element_probability[area] ) && ( ptd->element_ranking[area] ) )
		{
			i->item.data[4] = 0xFF;
			while (i->item.data[4] == 0xFF) // give a special
				i->item.data[4] = elemental_table[(12 * ( ptd->element_ranking[area] - 1 ) ) + ( mt_lrand() % 12 )];
		}
		else
			i->item.data[4] = 0;

		if ( i->item.data[4] )
			i->item.data[4] |= 0x80; // untekked

		num_percents = 0;

		for (ch=0;ch<3;ch++)
		{
			if (l->episode == 0x02)
				do_area = attachment_ep2[sid][l->difficulty][area][mt_lrand() % 4096];
			else
				do_area = attachment_ep1[sid][l->difficulty][area][mt_lrand() % 4096];
			if ( ( do_area ) && ( !did_area[do_area] ) )
			{
				did_area[do_area] = 1;
				i->item.data[6+(num_percents*2)] = (unsigned char) do_area;
				if ( l->episode == 0x02 )
					percent = percent_patterns_ep2[sid][l->difficulty][ptd->area_pattern[area]][mt_lrand() % 4096];
				else
					percent = percent_patterns_ep1[sid][l->difficulty][ptd->area_pattern[area]][mt_lrand() % 4096];
				percent -= 2;
				percent *= 5;
				i->item.data[6+(num_percents*2)+1] = percent;
				num_percents++;
			}
		}

		// Add a grind

		if ( l->episode == 0x02 )
			ch = power_patterns_ep2[sid][l->difficulty][ptd->area_pattern[area]][mt_lrand() % 4096];
		else
			ch = power_patterns_ep1[sid][l->difficulty][ptd->area_pattern[area]][mt_lrand() % 4096];

		i->item.data[3] = (unsigned char) ch;

		break;		
	case 0x01:
		r = mt_lrand() % 100;
		if (!is_enemy)
		{
			// Probabilities (Box): Armor 41%, Shields 41%, Units 18%
			if ( r > 82 )
				eq_type = 3;
			else
				if ( r > 59 )
					eq_type = 2;
				else
					eq_type = 1;
		}
		else
			eq_type = (unsigned) item_type;

		switch ( eq_type )
		{
		case 0x01:
			// Armor
			i->item.data[0] = 0x01;
			i->item.data[1] = 0x01;
			i->item.data[2] = (unsigned char) ( fl / 3L ) + ( 5 * l->difficulty ) + ( mt_lrand() % ( ( (unsigned char) fl / 2L ) + 2 ) );
			if ( i->item.data[2] > 0x17 )
				i->item.data[2] = 0x17;
			r = mt_lrand() % 11;
			if (r < 7)
			{
				if (armor_dfpvar_table[i->item.data[2]])
					i->item.data[6] = (unsigned char) (mt_lrand() % (armor_dfpvar_table[i->item.data[2]] + 1));
				if (armor_evpvar_table[i->item.data[2]])
					i->item.data[8] = (unsigned char) (mt_lrand() % (armor_evpvar_table[i->item.data[2]] + 1));
			}

			// Slots
			if ( l->episode == 0x02 )
				i->item.data[5] = slots_ep2[sid][l->difficulty][mt_lrand() % 4096]; 
			else
				i->item.data[5] = slots_ep1[sid][l->difficulty][mt_lrand() % 4096];

			break;
		case 0x02:
			// Shield
			i->item.data[0] = 0x01;
			i->item.data[1] = 0x02;
			i->item.data[2] = (unsigned char) ( fl / 3L ) + ( 4 * l->difficulty ) + ( mt_lrand() % ( ( (unsigned char) fl / 2L ) + 2 ) );
			if ( i->item.data[2] > 0x14 )
				i->item.data[2] = 0x14;
			r = mt_lrand() % 11;
			if (r < 2)
			{
				if (barrier_dfpvar_table[i->item.data[2]])
					i->item.data[6] = (unsigned char) (mt_lrand() % (barrier_dfpvar_table[i->item.data[2]] + 1));
				if (barrier_evpvar_table[i->item.data[2]])
					i->item.data[8] = (unsigned char) (mt_lrand() % (barrier_evpvar_table[i->item.data[2]] + 1));
			}
			break;
		case 0x03:
			// unit
			i->item.data[0] = 0x01;
			i->item.data[1] = 0x03;
			if ( ( ptd->unit_level[area] >= 2 ) && ( ptd->unit_level[area] <= 8 ) )
			{
				i->item.data[2] = 0xFF;
				while (i->item.data[2] == 0xFF)
					i->item.data[2] = unit_drop [mt_lrand() % ((ptd->unit_level[area] - 1) * 10)];
			}
			else
			{
				i->item.data[0] = 0x03;
				i->item.data[1] = 0x00;
				i->item.data[2] = 0x00; // Give a monomate when failed to look up unit.
			}
			break;
		}
		break;
	case 0x02:
		// Mag
		i->item.data [0]  = 0x02;
		i->item.data [2]  = 0x05;
		i->item.data [4]  = 0xF4;
		i->item.data [5]  = 0x01;
		i->item.data2[3] = mt_lrand() % 0x11;
		break;
	case 0x03:
		// Tool
		if ( l->episode == 0x02 )
			*(unsigned *) &i->item.data[0] = tool_remap[tool_drops_ep2[sid][l->difficulty][area][mt_lrand() % 4096]];
		else
			*(unsigned *) &i->item.data[0] = tool_remap[tool_drops_ep1[sid][l->difficulty][area][mt_lrand() % 4096]];
		if ( i->item.data[1] == 0x02 ) // Technique
		{
			if ( l->episode == 0x02 )
				i->item.data[4] = tech_drops_ep2[sid][l->difficulty][area][mt_lrand() % 4096];
			else
				i->item.data[4] = tech_drops_ep1[sid][l->difficulty][area][mt_lrand() % 4096];
			i->item.data[2] = (unsigned char) ptd->tech_levels[i->item.data[4]][area*2];
			if ( ptd->tech_levels[i->item.data[4]][(area*2)+1] > ptd->tech_levels[i->item.data[4]][area*2] )
				i->item.data[2] += (unsigned char) mt_lrand() % ( ( ptd->tech_levels[i->item.data[4]][(area*2)+1] - ptd->tech_levels[i->item.data[4]][(area*2)] ) + 1 );
		}
		if (stackable_table[i->item.data[1]])
			i->item.data[5] = 0x01;
		break;
	case 0x04:
		// Meseta
		i->item.data[0] = 0x04;
		meseta  = ptd->box_meseta[area][0];
		if ( ptd->box_meseta[area][1] > ptd->box_meseta[area][0] )
			meseta += mt_lrand() % ( ( ptd->box_meseta[area][1] - ptd->box_meseta[area][0] ) + 1 );
		*(unsigned *) &i->item.data2[0] = meseta;
		break;
	default:
		break;
	}
	i->item.itemid = l->itemID++;
}

void Send62 (CLIENT* client)
{
	CLIENT* lClient;
	unsigned bank_size, bank_use;
	unsigned short size;
	unsigned short sizecheck = 0;
	unsigned char t,maxt;
	unsigned itemid;
	int dont_send = 1;
	LOBBY* l;
	unsigned rt_index = 0;
	unsigned rare_lookup, rare_rate, rare_item, 
		rare_roll, box_rare, ch, itemNum;
	unsigned short mid, count;
	unsigned char* rt_table;
	unsigned char* rt_table2;
	unsigned meseta;
	unsigned DAR;
	unsigned floor_check = 0;
	SHOP* shopp;
	SHOP_ITEM* shopi;
	PTDATA* ptd;
	MAP_BOX* mb;

	if (!client->lobby)
		return;

	l = (LOBBY*) client->lobby;
	// don't support target @ 0x02
	t = client->decryptbuf[0x04];
	if (client->lobbyNum < 0x10)
		maxt = 12;
	else
		maxt = 4;

	size = *(unsigned short*) &client->decryptbuf[0x00];
	sizecheck = client->decryptbuf[0x09];

	sizecheck *= 4;
	sizecheck += 8;

	if (size != sizecheck)
	{
		debug ("Client sent a 0x62 packet whose sizecheck != size.\n");
		debug ("Command: %02X | Size: %04X | Sizecheck(%02x): %04x\n", client->decryptbuf[0x08],
			size, client->decryptbuf[0x09], sizecheck);
		client->decryptbuf[0x09] = ((size / 4) - 2);
	}

	switch (client->decryptbuf[0x08])
	{
	case 0x06:
		// Send guild card
		if ( ( size == 0x0C ) && ( t < maxt ) )
		{
			if ((l->slot_use[t]) && (l->client[t]))
			{
				lClient = l->client[t];
				PrepGuildCard ( client->guildcard, lClient->guildcard );
				memset (&PacketData[0], 0, 0x114);
				sprintf (&PacketData[0x00], "\x14\x01\x60");
				PacketData[0x03] = 0x00;
				PacketData[0x04] = 0x00;
				PacketData[0x08] = 0x06;
				PacketData[0x09] = 0x43;
				*(unsigned *) &PacketData[0x0C] = client->guildcard;
				memcpy (&PacketData[0x10], &client->character.name[0], 24 );
				memcpy (&PacketData[0x60], &client->character.guildcard_text[0], 176);
				PacketData[0x110] = 0x01; // ?
				PacketData[0x112] = (char)client->character.sectionID;
				PacketData[0x113] = (char)client->character._class;
				cipher_ptr = &lClient->server_cipher;
				encryptcopy (lClient, &PacketData[0], 0x114);
			}
		}
		break;
	case 0x5A:
		if ( client->lobbyNum > 0x0F )
		{			
			itemid = *(unsigned *) &client->decryptbuf[0x0C];
			if ( AddItemToClient ( itemid, client ) == 1 )
			{
				memset (&PacketData[0], 0, 16);
				PacketData[0x00] = 0x14;
				PacketData[0x02] = 0x60;
				PacketData[0x08] = 0x59;
				PacketData[0x09] = 0x03;
				PacketData[0x0A] = (unsigned char) client->clientID;
				PacketData[0x0E] = client->decryptbuf[0x10];
				PacketData[0x0C] = (unsigned char) client->clientID;
				*(unsigned *) &PacketData[0x10] = itemid;
				SendToLobby ( client->lobby, 4, &PacketData[0x00], 0x14, 0);
			}
		}
		else
			client->todc = 1;
		break;
	case 0x60:
		// Requesting a drop from a monster.
		if ( client->lobbyNum > 0x0F ) 
		{
			if ( !l->drops_disabled )
			{
				mid = *(unsigned short*) &client->decryptbuf[0x0E];
				mid &= 0xFFF;				

				if ( ( mid < 0xB50 ) && ( l->monsterData[mid].drop == 0 ) )
				{
					if (l->episode == 0x02)
						ptd = &pt_tables_ep2[client->character.sectionID][l->difficulty];
					else
						ptd = &pt_tables_ep1[client->character.sectionID][l->difficulty];

					if ( ( l->episode == 0x01 ) && ( client->decryptbuf[0x0D] == 35 ) &&
						 ( l->mapData[mid].rt_index == 34 ) )
						rt_index = 35; // Save Death Gunner index...
					else
						rt_index = l->mapData[mid].rt_index; // Use map's index instead of what the client says...

					if ( rt_index < 0x64 )
					{
						if ( l->episode == 0x03 )
						{
							if ( rt_index < 0x16 )
							{
								meseta = ep4_rtremap[(rt_index * 2)+1];
								// Past a certain point is Episode II data...
								if ( meseta > 0x2F )
									ptd = &pt_tables_ep2[client->character.sectionID][l->difficulty];
							}
							else
								meseta = 0;
						}
						else
							meseta = rt_index;
						if ( ( l->episode == 0x03 ) && 
							 ( rt_index >= 19 ) && 
							 ( rt_index <= 21 ) )
							DAR = 1;
						else
						{
							if ( ( ptd->enemy_dar[meseta] == 100 ) || ( l->redbox ) )
								DAR = 1;
							else
							{

								DAR = 100 - ptd->enemy_dar[meseta];
								if ( ( mt_lrand() % 100 ) >= DAR )
									DAR = 1;
								else
									DAR = 0;
							}
						}
					}
					else
						DAR = 0;

					if ( DAR )
					{
						if (rt_index < 0x64)
						{
							rt_index += ((0x1400 * l->difficulty) + ( client->character.sectionID * 0x200 ));
							switch (l->episode)
							{
								case 0x02:
									rare_lookup = rt_tables_ep2[rt_index];
									break;
								case 0x03:
									rare_lookup = rt_tables_ep4[rt_index];
									break;
								default:
									rare_lookup = rt_tables_ep1[rt_index];
									break;
							}
							rare_rate = ExpandDropRate ( rare_lookup & 0xFF );  // rare rate is expanded lower part
							rare_item = rare_lookup >> 8;						// rare item is upper part
							rare_roll = mt_lrand();								// rare roll is random number
							//debug ("rare_roll = %u", rare_roll );
							
							long result = rare_rate * global_rare_mult * rare_mob_mult;
							if (result < rare_rate)
							{
								rare_rate = 0xFFFF;  // if overflow, set to max value (this shouldnt happen like, ever)
								if (DEBUG) printf ("Rare rate overflow!! Mob rate\n");
							}
							else rare_rate = result;
						
							if  ( ( ( rare_lookup & 0xFF ) != 0 ) && ( ( rare_roll < rare_rate ) || ( l->redbox ) ) )
							{
								// Drop a rare item
								itemNum = free_game_item (l);
								memset (&l->gameItem[itemNum].item.data[0], 0, 12);
								memset (&l->gameItem[itemNum].item.data2[0], 0, 4);
								memcpy (&l->gameItem[itemNum].item.data[0], &rare_item, 3);
								GenerateRandomAttributes (client->character.sectionID, &l->gameItem[itemNum], l, client);
								l->gameItem[itemNum].item.itemid = l->itemID++;
							}
							else
							{
								// Drop a common item
								itemNum = free_game_item (l);
								if ( ( ( mt_lrand() % 100 ) < 60 ) || ( ptd->enemy_drop < 0 ) )
								{
									memset (&l->gameItem[itemNum].item.data[0], 0, 12 );
									memset (&l->gameItem[itemNum].item.data2[0], 0, 4 );
									l->gameItem[itemNum].item.data[0] = 0x04;
									rt_index = meseta;
									meseta  = ptd->enemy_meseta[rt_index][0];
									if ( ptd->enemy_meseta[rt_index][1] > ptd->enemy_meseta[rt_index][0] )
										meseta += mt_lrand() % ( ( ptd->enemy_meseta[rt_index][1] - ptd->enemy_meseta[rt_index][0] ) + 1 );
									*(unsigned *) &l->gameItem[itemNum].item.data2[0] = meseta;
									l->gameItem[itemNum].item.itemid = l->itemID++;
								}
								else
								{
									rt_index = meseta;
									GenerateCommonItem (ptd->enemy_drop[rt_index], 1, client->character.sectionID, &l->gameItem[itemNum], l, client);
								}
							}

							if ( l->gameItem[itemNum].item.itemid != 0 )
							{
								if (l->gameItemCount < MAX_SAVED_ITEMS)
									l->gameItemList[l->gameItemCount++] = itemNum;
								memset (&PacketData[0x00], 0, 16);
								PacketData[0x00] = 0x30;
								PacketData[0x01] = 0x00;
								PacketData[0x02] = 0x60;
								PacketData[0x03] = 0x00;
								PacketData[0x08] = 0x5F;
								PacketData[0x09] = 0x0D;
								*(unsigned *) &PacketData[0x0C] = *(unsigned *) &client->decryptbuf[0x0C];
								memcpy (&PacketData[0x10], &client->decryptbuf[0x10], 10);
								memcpy (&PacketData[0x1C], &l->gameItem[itemNum].item.data[0], 12 );
								*(unsigned *) &PacketData[0x28] = l->gameItem[itemNum].item.itemid;
								*(unsigned *) &PacketData[0x2C] = *(unsigned *) &l->gameItem[itemNum].item.data2[0];
								SendToLobby ( client->lobby, 4, &PacketData[0], 0x30, 0);
							}
						}
					}
					l->monsterData[mid].drop = 1;
				}
			}
		}
		else
			client->todc = 1;
		break;
	case 0x6F:
	case 0x71:
		if ( ( client->lobbyNum > 0x0F ) && ( t < maxt ) )
		{
			if (l->leader == client->clientID)
			{
				if ((l->slot_use[t]) && (l->client[t]))
				{
					if (l->client[t]->bursting == 1)
						dont_send = 0; // More user joining game stuff...
				}
			}
		}
		break;
	case 0xA2:
		if (client->lobbyNum > 0x0F)
		{
			if (!l->drops_disabled)
			{
				// box drop
				mid = *(unsigned short*) &client->decryptbuf[0x0E];
				mid &= 0xFFF;

				if ( ( mid < 0xB50 ) && ( l->boxHit[mid] == 0 ) )
				{
					box_rare = 0;
					mb = 0;
					
					//debug ("quest loaded: %i", l->quest_loaded);

					if ( ( l->quest_loaded ) && ( (unsigned) l->quest_loaded <= numQuests ) )
					{
						QUEST* q;

						q = &quests[l->quest_loaded - 1];
						if ( mid < q->max_objects )
							mb = (MAP_BOX*) &q->objectdata[(unsigned) ( 68 * mid ) + 0x28];
					}
					else
						mb = &l->objData[mid];

					if ( mb )
					{

						if ( mb->flag1 == 0 )
						{
							if ( ( ( mb->flag2 - FLOAT_PRECISION ) < (float) 1.00000 ) &&
								 ( ( mb->flag2 + FLOAT_PRECISION ) > (float) 1.00000 ) )
							{
								// Fixed item alert!!!

								box_rare = 1;
								itemNum = free_game_item (l);
								if ( ( ( mb->flag3 - FLOAT_PRECISION ) < (float) 1.00000 ) &&
									 ( ( mb->flag3 + FLOAT_PRECISION ) > (float) 1.00000 ) )
								{
									// Fully fixed!

									*(unsigned *) &l->gameItem[itemNum].item.data[0] = *(unsigned *) &mb->drop[0];

									// Not used... for now.
									l->gameItem[itemNum].item.data[3] = 0;

									if (l->gameItem[itemNum].item.data[0] == 0x04)
										GenerateCommonItem (0x04, 0, client->character.sectionID, &l->gameItem[itemNum], l, client);
									else
										if ((l->gameItem[itemNum].item.data[0] == 0x00) && 
											(l->gameItem[itemNum].item.data[1] == 0x00))
											GenerateCommonItem (0xFF, 0, client->character.sectionID, &l->gameItem[itemNum], l, client);
										else
										{
											memset (&l->gameItem[itemNum].item.data2[0], 0, 4);
											if (l->gameItem[itemNum].item.data[0] <  0x02)
												l->gameItem[itemNum].item.data[1]++; // Fix item offset
											GenerateRandomAttributes (client->character.sectionID, &l->gameItem[itemNum], l, client);
											l->gameItem[itemNum].item.itemid = l->itemID++;
										}
								}
								else
									GenerateCommonItem (mb->drop[0], 0, client->character.sectionID, &l->gameItem[itemNum], l, client);
							}
						}
					}

					if (!box_rare)
					{
						switch (l->episode)
						{
							case 0x02:
								rt_table = (unsigned char*) &rt_tables_ep2[0];
								break;
							case 0x03:
								rt_table = (unsigned char*) &rt_tables_ep4[0];
								break;
							default:
								rt_table = (unsigned char*) &rt_tables_ep1[0];
								break;
						}
						rt_table += ( ( 0x5000 * l->difficulty ) + ( client->character.sectionID * 0x800 ) ) + 0x194;
						rt_table2 = rt_table + 0x1E;
						rare_item = 0;

						switch ( l->episode )
						{
							case 0x01:
								switch ( l->floor[client->clientID ] )
								{
									case 11:
										// dragon
										floor_check = 3;
										break;
									case 12:
										// de rol
										floor_check = 6;
										break;
									case 13:
										// vol opt
										floor_check = 8;
										break;
									case 14:
										// falz
										floor_check = 10;
										break;
									default:
										floor_check = l->floor[client->clientID ];
										break;
								}	
								break;
							case 0x02:
								switch ( l->floor[client->clientID ] )
								{
									case 14:
										// barba ray
										floor_check = 3;
										break;
									case 15:
										// gol dragon
										floor_check = 6;
										break;
									case 12:
										// gal gryphon
										floor_check = 9;
										break;
									case 13:
										// olga flow
										floor_check = 10;
										break;
								default:
									// could be tower
									if ( l->floor[client->clientID] <= 11 )
										floor_check = ep2_rtremap[(l->floor[client->clientID] * 2)+1];
									else
										floor_check = 10;
									break;
								}	
								break;
							case 0x03:
								floor_check = l->floor[client->clientID ];
								break;
						}

						for (ch=0;ch<30;ch++)
						{
							if (*rt_table == floor_check)
							{
								rare_rate = ExpandDropRate ( *rt_table2 );
								memcpy (&rare_item, &rt_table2[1], 3);
								rare_roll = mt_lrand();
								
								long result = rare_rate * global_rare_mult * rare_box_mult;
								if (result < rare_rate)
								{
									rare_rate = 0xFFFF;  // if overflow, set to max value (this shouldnt happen like, ever)
									if (DEBUG) printf ("Rare rate overflow!! Box rate\n");
								}
								else rare_rate = result;
								
								if ( ( rare_roll < rare_rate ) || ( l->redbox == 1 ) )
								{
									box_rare = 1;
									itemNum = free_game_item (l);
									memset (&l->gameItem[itemNum].item.data[0], 0, 12);
									memset (&l->gameItem[itemNum].item.data2[0], 0, 4);
									memcpy (&l->gameItem[itemNum].item.data[0], &rare_item, 3);
									GenerateRandomAttributes (client->character.sectionID, &l->gameItem[itemNum], l, client);
									l->gameItem[itemNum].item.itemid = l->itemID++;
									break;
								}
							}
							rt_table++;
							rt_table2 += 0x04;
						}
					}

					if (!box_rare)
					{
						itemNum = free_game_item (l);
						GenerateCommonItem (0xFF, 0, client->character.sectionID, &l->gameItem[itemNum], l, client);
					}

					if (l->gameItem[itemNum].item.itemid != 0)
					{
						if (l->gameItemCount < MAX_SAVED_ITEMS)
							l->gameItemList[l->gameItemCount++] = itemNum;
						memset (&PacketData[0], 0, 16);
						PacketData[0x00] = 0x30;
						PacketData[0x01] = 0x00;
						PacketData[0x02] = 0x60;
						PacketData[0x03] = 0x00;
						PacketData[0x08] = 0x5F;
						PacketData[0x09] = 0x0D;
						*(unsigned *) &PacketData[0x0C] = *(unsigned *) &client->decryptbuf[0x0C];
						memcpy (&PacketData[0x10], &client->decryptbuf[0x10], 10);
						memcpy (&PacketData[0x1C], &l->gameItem[itemNum].item.data[0], 12 );
						*(unsigned *) &PacketData[0x28] = l->gameItem[itemNum].item.itemid;
						*(unsigned *) &PacketData[0x2C] = *(unsigned *) &l->gameItem[itemNum].item.data2[0];
						SendToLobby ( client->lobby, 4, &PacketData[0], 0x30, 0);
					}
					l->boxHit[mid] = 1;
				}
			}
		}
		break;
	case 0xA6:
		// Trade (not done yet)
		break;
	case 0xAE:
		// Chair info
		if ((size == 0x18) && (client->lobbyNum < 0x10) && (t < maxt))
			dont_send = 0;
		break;
	case 0xB5:
		// Client requesting shop
		if ( client->lobbyNum > 0x0F )
		{			 
			if ((l->floor[client->clientID] == 0) 
				&& (client->decryptbuf[0x0C] < 0x03))
			{
				client->doneshop[client->decryptbuf[0x0C]] = shopidx[client->character.level] + ( 333 * ((unsigned)client->decryptbuf[0x0C]) ) + ( mt_lrand() % 333 ) ;
				shopp = &shops[client->doneshop[client->decryptbuf[0x0C]]];
				cipher_ptr = &client->server_cipher;
				encryptcopy (client, (unsigned char*) &shopp->packet_length, shopp->packet_length);
			}
		}
		else
			client->todc = 1;
		break;
	case 0xB7:
		// Client buying an item
		if ( client->lobbyNum > 0x0F )
		{
			if ((l->floor[client->clientID] == 0)
				&& (client->decryptbuf[0x10] < 0x03) 
				&& (client->doneshop[client->decryptbuf[0x10]]))
			{
				if (client->decryptbuf[0x11] < shops[client->doneshop[client->decryptbuf[0x10]]].num_items)
				{
					shopi = &shops[client->doneshop[client->decryptbuf[0x10]]].item[client->decryptbuf[0x11]];
					if ((client->decryptbuf[0x12] > 1) && (shopi->data[0] != 0x03))
						client->todc = 1;
					else
						if (client->character.meseta < ((unsigned)client->decryptbuf[0x12] * shopi->price))
						{
							Send1A ("Not enough meseta for purchase.", client);
							client->todc = 1;
						}
						else
						{
							INVENTORY_ITEM i;

							memset (&i, 0, sizeof (INVENTORY_ITEM));
							memcpy (&i.item.data[0], &shopi->data[0], 12);
							// Update player item ID
							l->playerItemID[client->clientID] = *(unsigned *) &client->decryptbuf[0x0C];
							i.item.itemid = l->playerItemID[client->clientID]++;
							AddToInventory (&i, client->decryptbuf[0x12], 1, client);
							DeleteMesetaFromClient (shopi->price * (unsigned) client->decryptbuf[0x12], 0, client);
						}
				}
				else
					client->todc = 1;
			}
		}
		else
			client->todc = 1;
		break;
	case 0xB8:
		// Client is tekking a weapon.
		if ( client->lobbyNum > 0x0F )
		{
			unsigned compare_item;

			INVENTORY_ITEM* i;

			i = NULL;

			compare_item = *(unsigned *) &client->decryptbuf[0x0C];

			for (ch=0;ch<client->character.inventoryUse;ch++)
			{
				if ((client->character.inventory[ch].item.itemid == compare_item) &&
					(client->character.inventory[ch].item.data[0] == 0x00) &&
					(client->character.inventory[ch].item.data[4] & 0x80) &&
					(client->character.meseta >= 100))
				{
					char percent_mod;
					unsigned attrib;

					i = &client->character.inventory[ch];
					attrib = i->item.data[4] & ~(0x80);
					
					client->tekked = *i;

					if ( attrib < 0x29)
					{
						client->tekked.item.data[4] = tekker_attributes [( attrib * 3) + 1];
						if ( ( mt_lrand() % 100 ) > 70 )
							client->tekked.item.data[4] += mt_lrand() % ( ( tekker_attributes [(attrib * 3) + 2] - tekker_attributes [(attrib * 3) + 1] ) + 1 );
					}
					else
						client->tekked.item.data[4] = 0;
					if ( ( mt_lrand() % 10 ) < 2 ) percent_mod = -10;
					else
						if ( ( mt_lrand() % 10 ) < 2 ) percent_mod = -5;
						else
							if ( ( mt_lrand() % 10 ) < 2 ) percent_mod = 5;
							else
								if ( ( mt_lrand() % 10 ) < 2 ) percent_mod = 10;
								else
									percent_mod = 0;
					if ((!(i->item.data[6] & 128)) && (i->item.data[7] > 0))
						client->tekked.item.data[7] += percent_mod;
					if ((!(i->item.data[8] & 128)) && (i->item.data[9] > 0))
						client->tekked.item.data[9] += percent_mod;
					if ((!(i->item.data[10] & 128)) && (i->item.data[11] > 0))
						client->tekked.item.data[11] += percent_mod;
					DeleteMesetaFromClient (100, 0, client);
					memset (&client->encryptbuf[0x00], 0, 0x20);
					client->encryptbuf[0x00] = 0x20;
					client->encryptbuf[0x02] = 0x60;
					client->encryptbuf[0x08] = 0xB9;
					client->encryptbuf[0x09] = 0x08;
					client->encryptbuf[0x0A] = 0x79;
					memcpy (&client->encryptbuf[0x0C], &client->tekked.item.data[0], 16);
					cipher_ptr = &client->server_cipher;
					encryptcopy (client, &client->encryptbuf[0x00], 0x20);
					break;
				}
			}

			if ( i == NULL )
			{
				Send1A ("Could not find item to Tek.", client);
				client->todc = 1;
			}
		}
		else
			client->todc = 1;
		break;
	case 0xBA:
		// Client accepting tekked version of weapon.
		if ( ( client->lobbyNum > 0x0F ) && ( client->tekked.item.itemid ) )
		{
			unsigned ch2;

			for (ch=0;ch<4;ch++)
			{
				if ((l->slot_use[ch]) && (l->client[ch]))
				{
					for (ch2=0;ch2<l->client[ch]->character.inventoryUse;ch2++)
						if (l->client[ch]->character.inventory[ch2].item.itemid == client->tekked.item.itemid)
						{
							Send1A ("Item duplication attempt!", client);
							client->todc = 1;
							break;
						}
				}
			}

			for (ch=0;ch<l->gameItemCount;l++)
			{
				itemNum = l->gameItemList[ch];
				if (l->gameItem[itemNum].item.itemid == client->tekked.item.itemid)
				{
					// Added to the game's inventory by the client...
					// Delete it and avoid duping...
					memset (&l->gameItem[itemNum], 0, sizeof (GAME_ITEM));
					l->gameItemList[ch] = 0xFFFFFFFF;
					break;
				}
			}

			CleanUpGameInventory (l);

			if (!client->todc)
			{
				AddToInventory (&client->tekked, 1, 1, client);
				memset (&client->tekked, 0, sizeof (INVENTORY_ITEM));
			}
		}
		else
			client->todc = 1;
		break;
	case 0xBB:
		// Client accessing bank
		if ( client->lobbyNum < 0x10 )
			client->todc = 1;
		else
		{
			if ( ( l->floor[client->clientID] == 0) && ( ((unsigned) servertime - client->command_cooldown[0xBB]) >= 1 ))
			{
				client->command_cooldown[0xBB] = (unsigned) servertime;

				/* Which bank are we accessing? */

				client->bankAccess = client->bankType;

				if (client->bankAccess)
					memcpy (&client->character.bankUse, &client->common_bank, sizeof (BANK));
				else
					memcpy (&client->character.bankUse, &client->char_bank, sizeof (BANK));

				for (ch=0;ch<client->character.bankUse;ch++)
					client->character.bankInventory[ch].itemid = l->bankItemID[client->clientID]++;
				memset (&client->encryptbuf[0x00], 0, 0x34);
				client->encryptbuf[0x02] = 0x6C;
				client->encryptbuf[0x08] = 0xBC;
				bank_size = 0x18 * (client->character.bankUse + 1);
				*(unsigned *) &client->encryptbuf[0x0C] = bank_size;
				bank_size += 4;
				*(unsigned short *) &client->encryptbuf[0x00] = (unsigned short) bank_size;
				bank_use = mt_lrand();
				*(unsigned *) &client->encryptbuf[0x10] = bank_use;
				bank_use = client->character.bankUse;
				*(unsigned *) &client->encryptbuf[0x14] = bank_use;
				*(unsigned *) &client->encryptbuf[0x18] = client->character.bankMeseta;
				if (client->character.bankUse)
					memcpy (&client->encryptbuf[0x1C], &client->character.bankInventory[0], sizeof (BANK_ITEM) * client->character.bankUse);
				cipher_ptr = &client->server_cipher;
				encryptcopy (client, &client->encryptbuf[0x00], bank_size);
			}
		}
		break;
	case 0xBD:
		if ( client->lobbyNum < 0x10 )
		{
			dont_send = 1;
			client->todc = 1;
		}
		else
		{
			if ( l->floor[client->clientID] == 0)
			{
				switch (client->decryptbuf[0x14])
				{
				case 0x00: 
					// Making a deposit
					itemid = *(unsigned *) &client->decryptbuf[0x0C];
					if (itemid == 0xFFFFFFFF)
					{
						meseta = *(unsigned *) &client->decryptbuf[0x10];

						if (client->character.meseta >= meseta)
						{
							client->character.bankMeseta += meseta;
							client->character.meseta -= meseta;
							if (client->character.bankMeseta > 999999)
								client->character.bankMeseta = 999999;
						}
						else
						{
							Send1A ("Client/server data synchronization error.", client);
							client->todc = 1;
						}
					}
					else
					{
						if ( client->character.bankUse < 200 )
						{
							// Depositing something else...
							count = client->decryptbuf[0x15];
							DepositIntoBank (itemid, count, client);
							if (!client->todc)
								SortBankItems(client);
						}
						else
						{						
							Send1A ("Can't deposit.  Bank is full.", client);
							client->todc = 1;
						}
					}
					break;
				case 0x01:
					itemid = *(unsigned *) &client->decryptbuf[0x0C];
					if (itemid == 0xFFFFFFFF)
					{
						meseta = *(unsigned *) &client->decryptbuf[0x10];
						if (client->character.bankMeseta >= meseta)
						{
							client->character.bankMeseta -= meseta;
							client->character.meseta += meseta;
						}
						else
							client->todc = 1;
					}
					else
					{
						// Withdrawing something else...
						count = client->decryptbuf[0x15];
						WithdrawFromBank (itemid, count, client);
					}
					break;
				default:
					break;
				}

				/* Update bank. */

				if (client->bankAccess)
					memcpy (&client->common_bank, &client->character.bankUse, sizeof (BANK));
				else
					memcpy (&client->char_bank, &client->character.bankUse, sizeof (BANK));

			}
		}
		break;
	case 0xC1:
	case 0xC2:
	//case 0xCD:
	//case 0xCE:
		if (t < maxt)
		{
			// Team invite for C1 & C2, Master Transfer for CD & CE.
			if (size == 0x64)
				dont_send = 0;

			if (client->decryptbuf[0x08] == 0xC2)
			{
				unsigned gcn;

				gcn = *(unsigned *) &client->decryptbuf[0x0C];
				if ((client->decryptbuf[0x10] == 0x02) &&
					(client->guildcard == gcn))
					client->teamaccept = 1;
			}

			if (client->decryptbuf[0x08] == 0xCD)
			{
				if (client->character.privilegeLevel != 0x40)
				{
					dont_send = 1;
					Send01 ("You aren't the master of your team.", client);
				}
				else
					client->masterxfer = 1;
			}
		}
		break;
	case 0xC9:
		if ( client->lobbyNum > 0x0F )
		{
			INVENTORY_ITEM add_item;
			int meseta;

			if ( l->quest_loaded )
			{
				meseta = *(int *) &client->decryptbuf[0x0C];
				if (meseta < 0)
				{
					meseta = -meseta;
					client->character.meseta -= (unsigned) meseta;
				}
				else
				{
					memset (&add_item, 0, sizeof (INVENTORY_ITEM));
					add_item.item.data[0] = 0x04;
					*(unsigned *) &add_item.item.data2[0] = *(unsigned *) &client->decryptbuf[0x0C];
					add_item.item.itemid = l->itemID;
					l->itemID++;
					AddToInventory (&add_item, 1, 0, client);
				}
			}
		}
		else
			client->todc = 1;
		break;
	case 0xCA:
		if ( client->lobbyNum > 0x0F )
		{
			INVENTORY_ITEM add_item;

			if ( l->quest_loaded )
			{
				unsigned ci, compare_item = 0;

				memset (&add_item, 0, sizeof (INVENTORY_ITEM));
				memcpy ( &compare_item, &client->decryptbuf[0x0C], 3 );
				for ( ci = 0; ci < quest_numallows; ci ++)
				{
					if (compare_item == quest_allow[ci])
					{
						add_item.item.data[0] = 0x01;
						break;
					}
				}
				if (add_item.item.data[0] == 0x01)
				{
					memcpy (&add_item.item.data[0], &client->decryptbuf[0x0C], 12);
					add_item.item.itemid = l->itemID;
					l->itemID++;
					AddToInventory (&add_item, 1, 0, client);
				}
				else
				{
					SendEE ("You did not receive the quest reward.  The item requested is not on the allow list.  Your request and guild card have been logged for the server administrator.", client);
					WriteLog ("User %u attempted to claim quest reward %08x but item is not in the allow list.", client->guildcard, compare_item );
				}
			}
		}
		else
			client->todc = 1;
		break;
	case 0xD0:
		// Level up player?
		// Player to level @ 0x0A
		// Levels to gain @ 0x0C
		if ( ( t < maxt ) && ( l->battle ) && ( l->quest_loaded ) )
		{
			if ( ( client->decryptbuf[0x0A] < 4 ) && ( l->client[client->decryptbuf[0x0A]] ) )
			{
				unsigned target_lv;

				lClient = l->client[client->decryptbuf[0x0A]];
				target_lv = lClient->character.level;
				target_lv += client->decryptbuf[0x0C];
				
				if ( target_lv > 199 )
					 target_lv = 199;

				SkipToLevel ( target_lv, lClient, 0 );
			}
		}
		break;
	case 0xD6:
		// Wrap an item
		if ( client->lobbyNum > 0x0F )
		{
			unsigned wrap_id;
			INVENTORY_ITEM backup_item;

			memset (&backup_item, 0, sizeof (INVENTORY_ITEM));
			wrap_id = *(unsigned *) &client->decryptbuf[0x18];

			for (ch=0;ch<client->character.inventoryUse;ch++)
			{
				if (client->character.inventory[ch].item.itemid == wrap_id)
				{
					memcpy (&backup_item, &client->character.inventory[ch], sizeof (INVENTORY_ITEM));
					break;
				}
			}

			if (backup_item.item.itemid)
			{
				DeleteFromInventory (&backup_item, 1, client);					
				if (!client->todc)
				{
					if (backup_item.item.data[0] == 0x02)
						backup_item.item.data2[2] |= 0x40; // Wrap a mag
					else
						backup_item.item.data[4] |= 0x40; // Wrap other
					AddToInventory (&backup_item, 1, 0, client);
				}
			}
			else
			{
				Send1A ("Could not find item to wrap.", client);
				client->todc = 1;
			}
		}
		else
			client->todc = 1;
		break;
	case 0xDF:
		if ( client->lobbyNum > 0x0F )
		{
			if ( ( l->oneperson ) && ( l->quest_loaded ) && ( !l->drops_disabled ) )
			{
				INVENTORY_ITEM work_item;

				memset (&work_item, 0, sizeof (INVENTORY_ITEM));
				work_item.item.data[0] = 0x03;
				work_item.item.data[1] = 0x10;
				work_item.item.data[2] = 0x02;
				DeleteFromInventory (&work_item, 1, client);
				if (!client->todc)
					l->drops_disabled = 1;
			}
		}
		else
			client->todc = 1;
		break;
	case 0xE0:
		if ( client->lobbyNum > 0x0F )
		{
			if ( ( l->oneperson ) && ( l->quest_loaded ) && ( l->drops_disabled ) && ( !l->questE0 ) )
			{
				unsigned bp, bpl, new_item;

				if ( client->decryptbuf[0x0D] > 0x03 )
					bpl = 1;
				else
					bpl = l->difficulty + 1;

				for (bp=0;bp<bpl;bp++)
				{
					new_item = 0;

					switch ( client->decryptbuf[0x0D] )
					{
					case 0x00:
						// bp1 dorphon route
						switch ( l->difficulty )
						{
						case 0x00:
							new_item = bp_dorphon_normal[mt_lrand() % (sizeof(bp_dorphon_normal)/4)];
							break;
						case 0x01:
							new_item = bp_dorphon_hard[mt_lrand() % (sizeof(bp_dorphon_hard)/4)];
							break;
						case 0x02:
							new_item = bp_dorphon_vhard[mt_lrand() % (sizeof(bp_dorphon_vhard)/4)];
							break;
						case 0x03:
							new_item = bp_dorphon_ultimate[mt_lrand() % (sizeof(bp_dorphon_ultimate)/4)];
							break;
						}
						break;
					case 0x01:
						// bp1 rappy route
						switch ( l->difficulty )
						{
						case 0x00:
							new_item = bp_rappy_normal[mt_lrand() % (sizeof(bp_rappy_normal)/4)];
							break;
						case 0x01:
							new_item = bp_rappy_hard[mt_lrand() % (sizeof(bp_rappy_hard)/4)];
							break;
						case 0x02:
							new_item = bp_rappy_vhard[mt_lrand() % (sizeof(bp_rappy_vhard)/4)];
							break;
						case 0x03:
							new_item = bp_rappy_ultimate[mt_lrand() % (sizeof(bp_rappy_ultimate)/4)];
							break;
						}
						break;
					case 0x02:
						// bp1 zu route
						switch ( l->difficulty )
						{
						case 0x00:
							new_item = bp_zu_normal[mt_lrand() % (sizeof(bp_zu_normal)/4)];
							break;
						case 0x01:
							new_item = bp_zu_hard[mt_lrand() % (sizeof(bp_zu_hard)/4)];
							break;
						case 0x02:
							new_item = bp_zu_vhard[mt_lrand() % (sizeof(bp_zu_vhard)/4)];
							break;
						case 0x03:
							new_item = bp_zu_ultimate[mt_lrand() % (sizeof(bp_zu_ultimate)/4)];
							break;
						}
						break;
					case 0x04:
						// bp2
						switch ( l->difficulty )
						{
						case 0x00:
							new_item = bp2_normal[mt_lrand() % (sizeof(bp2_normal)/4)];
							break;
						case 0x01:
							new_item = bp2_hard[mt_lrand() % (sizeof(bp2_hard)/4)];
							break;
						case 0x02:
							new_item = bp2_vhard[mt_lrand() % (sizeof(bp2_vhard)/4)];
							break;
						case 0x03:
							new_item = bp2_ultimate[mt_lrand() % (sizeof(bp2_ultimate)/4)];
							break;
						}
						break;
					}
					l->questE0 = 1;
					memset (&client->encryptbuf[0x00], 0, 0x2C);
					client->encryptbuf[0x00] = 0x2C;
					client->encryptbuf[0x02] = 0x60;
					client->encryptbuf[0x08] = 0x5D;
					client->encryptbuf[0x09] = 0x09;
					client->encryptbuf[0x0A] = 0xFF;
					client->encryptbuf[0x0B] = 0xFB;
					memcpy (&client->encryptbuf[0x0C], &client->decryptbuf[0x0C], 12 );
					*(unsigned *) &client->encryptbuf[0x18] = new_item;
					*(unsigned *) &client->encryptbuf[0x24] = l->itemID;
					itemNum = free_game_item (l);
					if (l->gameItemCount < MAX_SAVED_ITEMS)
						l->gameItemList[l->gameItemCount++] = itemNum;
					memset (&l->gameItem[itemNum], 0, sizeof (GAME_ITEM));
					*(unsigned *) &l->gameItem[itemNum].item.data[0] = new_item;
					if (new_item == 0x04)
					{
						new_item  = pt_tables_ep1[client->character.sectionID][l->difficulty].enemy_meseta[0x2E][0];
						new_item += mt_lrand() % 100;
						*(unsigned *) &client->encryptbuf[0x28] = new_item;
						*(unsigned *) &l->gameItem[itemNum].item.data2[0] = new_item;
					}
					if (l->gameItem[itemNum].item.data[0] == 0x00)
					{
						l->gameItem[itemNum].item.data[4] = 0x80; // Untekked
						client->encryptbuf[0x1C] = 0x80;
					}
					l->gameItem[itemNum].item.itemid = l->itemID++;
					cipher_ptr = &client->server_cipher;
					encryptcopy (client, &client->encryptbuf[0x00], 0x2C);
				}
			}
		}
		break;
	default:
		if (client->lobbyNum > 0x0F)
		{
			WriteLog ("62 command \"%02x\" not handled in game. (Data below)", client->decryptbuf[0x08]);
			packet_to_text ( &client->decryptbuf[0x00], size );
			WriteLog ("%s", &dp[0]);
		}
		break;
	}

	if ( ( dont_send == 0 ) && ( !client->todc ) )
	{
		if ((l->slot_use[t]) && (l->client[t]))
		{
			lClient = l->client[t];
			cipher_ptr = &lClient->server_cipher;
			encryptcopy (lClient, &client->decryptbuf[0], size);
		}
	}
}


void Send6D (CLIENT* client)
{
	CLIENT* lClient;
	unsigned short size;
	unsigned short sizecheck = 0;
	unsigned char t;
	int dont_send = 0;
	LOBBY* l;

	if (!client->lobby)
		return;

	size = *(unsigned short*) &client->decryptbuf[0x00];
	sizecheck = *(unsigned short*) &client->decryptbuf[0x0C];
	sizecheck += 8;

	if (size != sizecheck)
	{
		debug ("Client sent a 0x6D packet whose sizecheck != size.\n");
		debug ("Command: %02X | Size: %04X | Sizecheck: %04x\n", client->decryptbuf[0x08],
			size, sizecheck);
		dont_send = 1;
	}

	l = (LOBBY*) client->lobby;
	t = client->decryptbuf[0x04];
	if (t >= 0x04)
		dont_send = 1;

	if (!dont_send)
	{
		switch (client->decryptbuf[0x08])
		{
		case 0x70:
			if (client->lobbyNum > 0x0F)
			{
				if ((l->slot_use[t]) && (l->client[t]))
				{
					if (l->client[t]->bursting == 1)
					{
						unsigned ch;

						dont_send = 0; // It's cool to send them as long as this user is bursting.

						// Let's reconstruct the 0x70 as much as possible.
						//
						// Could check guild card # here
						*(unsigned *) &client->decryptbuf[0x7C] = client->guildcard;
						// Check techniques...
						if (!(client->equip_flags & DROID_FLAG))
						{
							for (ch=0;ch<19;ch++)
							{
								if ((char) client->decryptbuf[0xC4+ch] > max_tech_level[ch][client->character._class])
								{
									client->character.techniques[ch] = -1; // Unlearn broken technique.
									client->todc = 1;
								}
							}
							if (client->todc)
								Send1A ("Technique data check failed.\n\nSome techniques have been unlearned.", client);
						}
						memcpy (&client->decryptbuf[0xC4], &client->character.techniques, 20);
						// Could check character structure here
						memcpy (&client->decryptbuf[0xD8], &client->character.gcString, 104);
						// Prevent crashing with NPC skins...
						if (client->character.skinFlag)
							memset (&client->decryptbuf[0x110], 0, 10);
						// Could check stats here
						memcpy (&client->decryptbuf[0x148], &client->character.ATP, 36);
						// Could check inventory here
						client->decryptbuf[0x16C] = client->character.inventoryUse;
						memcpy (&client->decryptbuf[0x170], &client->character.inventory[0], 30 * sizeof (INVENTORY_ITEM) );
					}
				}
			}
			break;
		case 0x6B:
		case 0x6C:
		case 0x6D:
		case 0x6E:
		case 0x72:
			if (client->lobbyNum > 0x0F)
			{
				if (l->leader == client->clientID)
				{
					if ((l->slot_use[t]) && (l->client[t]))
					{
						if (l->client[t]->bursting == 1)
							dont_send = 0; // It's cool to send them as long as this user is bursting and we're the leader.
					}
				}
			}
			break;
		default:
			dont_send = 1;
			break;
		}
	}

	if ( dont_send == 0 )
	{
		lClient = l->client[t];
		cipher_ptr = &lClient->server_cipher;
		encryptcopy (lClient, &client->decryptbuf[0], size);
	}
}


void Send01 (const char *text, CLIENT* client)
{
	unsigned short mesgOfs = 0x10;
	unsigned ch;

	memset(&PacketData[0], 0, 16);
	PacketData[mesgOfs++] = 0x09;
	PacketData[mesgOfs++] = 0x00;
	PacketData[mesgOfs++] = 0x45;
	PacketData[mesgOfs++] = 0x00;
	for (ch=0;ch<strlen(text);ch++)
	{
		PacketData[mesgOfs++] = text[ch];
		PacketData[mesgOfs++] = 0x00;
	}
	PacketData[mesgOfs++] = 0x00;
	PacketData[mesgOfs++] = 0x00;
	while (mesgOfs % 8)
		PacketData[mesgOfs++] = 0x00;
	*(unsigned short*) &PacketData[0] = mesgOfs;
	PacketData[0x02] = 0x01;
	cipher_ptr = &client->server_cipher;
	encryptcopy (client, &PacketData[0], mesgOfs);
}
