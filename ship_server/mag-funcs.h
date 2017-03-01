int MagAlignment ( MAG* m )
{
	int v1, v2, v3, v4, v5, v6;

	v4 = 0;
	v3 = m->power;
	v2 = m->dex;
	v1 = m->mind;
	if ( v2 < v3 )
	{
		if ( v1 < v3 )
			v4 = 8;
	}
	if ( v3 < v2 )
	{
		if ( v1 < v2 )
			v4 |= 0x10u;
	}
	if ( v2 < v1 )
	{
		if ( v3 < v1 )
			v4 |= 0x20u;
	}
	v6 = 0;
	v5 = v3;
	if ( v3 <= v2 )
		v5 = v2;
	if ( v5 <= v1 )
		v5 = v1;
	if ( v5 == v3 )
		v6 = 1;
	if ( v5 == v2 )
		++v6;
	if ( v5 == v1 )
		++v6;
	if ( v6 >= 2 )
		v4 |= 0x100u;
	return v4;
}

int MagSpecialEvolution ( MAG* m, unsigned char sectionID, unsigned char type, int EvolutionClass )
{
	unsigned char oldType;
	short mDefense, mPower, mDex, mMind;

	oldType = m->mtype;

	if (m->level >= 100)
	{
		mDefense = m->defense / 100;
		mPower = m->power / 100;
		mDex = m->dex / 100;
		mMind = m->mind / 100;

		switch ( sectionID )
		{
		case ID_Viridia:
		case ID_Bluefull:
		case ID_Redria:
		case ID_Whitill:
			if ( ( mDefense + mDex ) == ( mPower + mMind ) )
			{
				switch ( type )
				{
				case CLASS_HUMAR:
				case CLASS_HUCAST:
					m->mtype = Mag_Deva;
					break;
				case CLASS_HUNEWEARL:
				case CLASS_HUCASEAL:
					m->mtype = Mag_Savitri;
					break;
				case CLASS_RAMAR:
				case CLASS_RACAST:
					m->mtype = Mag_Pushan;
					break;
				case CLASS_RACASEAL:
				case CLASS_RAMARL:
					m->mtype = Mag_Rukmin;
					break;
				case CLASS_FONEWM:
				case CLASS_FOMAR:
					m->mtype = Mag_Nidra;
					break;
				case CLASS_FONEWEARL:
				case CLASS_FOMARL:
					m->mtype = Mag_Sato;
					break;
				default:
					break;
				}
			}
			break;
		case ID_Skyly:
		case ID_Pinkal:
		case ID_Yellowboze:
			if ( ( mDefense + mPower ) == ( mDex + mMind ) )
			{
				switch ( type )
				{
				case CLASS_HUMAR:
				case CLASS_HUCAST:
					m->mtype = Mag_Rati;
					break;
				case CLASS_HUNEWEARL:
				case CLASS_HUCASEAL:
					m->mtype = Mag_Savitri;
					break;
				case CLASS_RAMAR:
				case CLASS_RACAST:
					m->mtype = Mag_Pushan;
					break;
				case CLASS_RACASEAL:
				case CLASS_RAMARL:
					m->mtype = Mag_Dewari;
					break;
				case CLASS_FONEWM:
				case CLASS_FOMAR:
					m->mtype = Mag_Nidra;
					break;
				case CLASS_FONEWEARL:
				case CLASS_FOMARL:
					m->mtype = Mag_Bhima;
					break;
				default:
					break;
				}
			}
			break;
		case ID_Greennill:
		case ID_Oran:
		case ID_Purplenum:
			if ( ( mDefense + mMind ) == ( mPower + mDex ) )
			{
				switch ( type )
				{
				case CLASS_HUMAR:
				case CLASS_HUCAST:
					m->mtype = Mag_Rati;
					break;
				case CLASS_HUNEWEARL:
				case CLASS_HUCASEAL:
					m->mtype = Mag_Savitri;
					break;
				case CLASS_RAMAR:
				case CLASS_RACAST:
					m->mtype = Mag_Pushan;
					break;
				case CLASS_RACASEAL:
				case CLASS_RAMARL:
					m->mtype = Mag_Rukmin;
					break;
				case CLASS_FONEWM:
				case CLASS_FOMAR:
					m->mtype = Mag_Nidra;
					break;
				case CLASS_FONEWEARL:
				case CLASS_FOMARL:
					m->mtype = Mag_Bhima;
					break;
				default:
					break;
				}
			}
			break;
		}
	}
	return (int)(oldType != m->mtype);
}

void MagLV50Evolution ( MAG* m, unsigned char sectionID, unsigned char type, int EvolutionClass )
{
	int v10, v11, v12, v13;

	int Alignment = MagAlignment ( m );

	if ( EvolutionClass > 3 ) // Don't bother to check if a special mag.
		return;

	v10 = m->power / 100;
	v11 = m->dex / 100;
	v12 = m->mind / 100;
	v13 = m->defense / 100;

	switch ( type )
	{
	case CLASS_HUMAR:
	case CLASS_HUNEWEARL:
	case CLASS_HUCAST:
	case CLASS_HUCASEAL:
		if ( Alignment & 0x108 )
		{
			if ( sectionID & 1 )
			{
				if ( v12 > v11 )
				{
					m->mtype = Mag_Apsaras;
					AddPB ( &m->PBflags, &m->blasts, PB_Estlla);
				}
				else
				{
					m->mtype = Mag_Kama;
					AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
				}
			}
			else
			{
				if ( v12 > v11 )
				{
					m->mtype = Mag_Bhirava;
					AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
				}
				else
				{
					m->mtype = Mag_Varaha;
					AddPB ( &m->PBflags, &m->blasts, PB_Golla);
				}
			}
		}
		else
		{
			if ( Alignment & 0x10 )
			{
				if ( sectionID & 1 )
				{
					if ( v10 > v12 )
					{
						m->mtype = Mag_Garuda;
						AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
					}
					else
					{
						m->mtype = Mag_Yaksa;
						AddPB ( &m->PBflags, &m->blasts, PB_Golla);
					}
				}
				else
				{
					if ( v10 > v12 )
					{
						m->mtype = Mag_Ila;
						AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
					}
					else
					{
						m->mtype = Mag_Nandin;
						AddPB ( &m->PBflags, &m->blasts, PB_Estlla);
					}
				}
			}
			else
			{
				if ( Alignment & 0x20 )
				{
					if ( sectionID & 1 )
					{
						if ( v11 > v10 )
						{
							m->mtype = Mag_Soma;
							AddPB ( &m->PBflags, &m->blasts, PB_Estlla);
						}
						else
						{
							m->mtype = Mag_Bana;
							AddPB ( &m->PBflags, &m->blasts, PB_Estlla);
						}
					}
					else
					{
						if ( v11 > v10 )
						{
							m->mtype = Mag_Ushasu;
							AddPB ( &m->PBflags, &m->blasts, PB_Golla);
						}
						else
						{
							m->mtype = Mag_Kabanda;
							AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
						}
					}
				}
			}
		}
		break;
	case CLASS_RAMAR:
	case CLASS_RACAST:
	case CLASS_RACASEAL:
	case CLASS_RAMARL:
		if ( Alignment & 0x110 )
		{
			if ( sectionID & 1 )
			{
				if ( v10 > v12 )
				{
					m->mtype = Mag_Kaitabha;
					AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
				}
				else
				{
					m->mtype = Mag_Varaha;
					AddPB ( &m->PBflags, &m->blasts, PB_Golla);
				}
			}
			else
			{
				if ( v10 > v12 )
				{
					m->mtype = Mag_Bhirava;
					AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
				}
				else
				{
					m->mtype = Mag_Kama;
					AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
				}
			}
		}
		else
		{
			if ( Alignment & 0x08 )
			{
				if ( sectionID & 1 )
				{
					if ( v12 > v11 )
					{
						m->mtype = Mag_Kaitabha;
						AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
					}
					else
					{
						m->mtype = Mag_Madhu;
						AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
					}
				}
				else
				{
					if ( v12 > v11 )
					{
						m->mtype = Mag_Bhirava;
						AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
					}
					else
					{
						m->mtype = Mag_Kama;
						AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
					}
				}
			}
			else
			{
				if ( Alignment & 0x20 )
				{
					if ( sectionID & 1 )
					{
						if ( v11 > v10 )
						{
							m->mtype = Mag_Durga;
							AddPB ( &m->PBflags, &m->blasts, PB_Estlla);
						}
						else
						{
							m->mtype = Mag_Kabanda;
							AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
						}
					}
					else
					{
						if ( v11 > v10 )
						{
							m->mtype = Mag_Apsaras;
							AddPB ( &m->PBflags, &m->blasts, PB_Estlla);
						}
						else
						{
							m->mtype = Mag_Varaha;
							AddPB ( &m->PBflags, &m->blasts, PB_Golla);
						}
					}
				}
			}
		}
		break;
	case CLASS_FONEWM:
	case CLASS_FONEWEARL:
	case CLASS_FOMARL:
	case CLASS_FOMAR:
		if ( Alignment & 0x120 )
		{
			if ( v13 > 44 )
			{
				m->mtype = Mag_Bana;
				AddPB ( &m->PBflags, &m->blasts, PB_Estlla);
			}
			else
			{
				if ( sectionID & 1 )
				{
					if ( v11 > v10 )
					{
						m->mtype = Mag_Ila;
						AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
					}
					else
					{
						m->mtype = Mag_Kumara;
						AddPB ( &m->PBflags, &m->blasts, PB_Golla);
					}
				}
				else
				{
					if ( v11 > v10 )
					{
						m->mtype = Mag_Kabanda;
						AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
					}
					else
					{
						m->mtype = Mag_Naga;
						AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
					}
				}
			}
		}
		else
		{
			if ( Alignment & 0x08 )
			{
				if ( v13 > 44 )
				{
					m->mtype = Mag_Andhaka;
					AddPB ( &m->PBflags, &m->blasts, PB_Estlla);
				}
				else
				{
					if ( sectionID & 1 )
					{
						if ( v12 > v11 )
						{
							m->mtype = Mag_Naga;
							AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
						}
						else
						{
							m->mtype = Mag_Marica;
							AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
						}
					}
					else
					{
						if ( v12 > v11 )
						{
							m->mtype = Mag_Ravana;
							AddPB ( &m->PBflags, &m->blasts, PB_Farlla);
						}
						else
						{
							m->mtype = Mag_Naraka;
							AddPB ( &m->PBflags, &m->blasts, PB_Golla);
						}
					}
				}
			}
			else
			{
				if ( Alignment & 0x10 )
				{
					if ( v13 > 44 )
					{
						m->mtype = Mag_Bana;
						AddPB ( &m->PBflags, &m->blasts, PB_Estlla);
					}
					else
					{
						if ( sectionID & 1 )
						{
							if ( v10 > v12 )
							{
								m->mtype = Mag_Garuda;
								AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
							}
							else
							{
								m->mtype = Mag_Bhirava;
								AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
							}
						}
						else
						{
							if ( v10 > v12 )
							{
								m->mtype = Mag_Ribhava;
								AddPB ( &m->PBflags, &m->blasts, PB_Farlla);
							}
							else
							{
								m->mtype = Mag_Sita;
								AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
							}
						}
					}
				}
			}
		}
		break;
	}
}

void MagLV35Evolution ( MAG* m, unsigned char sectionID, unsigned char type, int EvolutionClass )
{
	int Alignment = MagAlignment ( m );

	if ( EvolutionClass > 3 ) // Don't bother to check if a special mag.
		return;

	switch ( type )
	{
	case CLASS_HUMAR:
	case CLASS_HUNEWEARL:
	case CLASS_HUCAST:
	case CLASS_HUCASEAL:
		if ( Alignment & 0x108 )
		{
			m->mtype = Mag_Rudra;
			AddPB ( &m->PBflags, &m->blasts, PB_Golla);
			return;
		}
		else
		{
			if ( Alignment & 0x10 )
			{
				m->mtype = Mag_Marutah;
				AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
				return;
			}
			else
			{
				if ( Alignment & 0x20 )
				{
					m->mtype = Mag_Vayu;
					AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
					return;
				}
			}
		}
		break;
	case CLASS_RAMAR:
	case CLASS_RACAST:
	case CLASS_RACASEAL:
	case CLASS_RAMARL:
		if ( Alignment & 0x110 )
		{
			m->mtype = Mag_Mitra;
			AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
			return;
		}
		else
		{
			if ( Alignment & 0x08 )
			{
				m->mtype = Mag_Surya;
				AddPB ( &m->PBflags, &m->blasts, PB_Golla);
				return;
			}
			else
			{
				if ( Alignment & 0x20 )
				{
					m->mtype = Mag_Tapas;
					AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
					return;
				}
			}
		}
		break;
	case CLASS_FONEWM:
	case CLASS_FONEWEARL:
	case CLASS_FOMARL:
	case CLASS_FOMAR:
		if ( Alignment & 0x120 )
		{
			m->mtype = Mag_Namuci;
			AddPB ( &m->PBflags, &m->blasts, PB_Mylla_Youlla);
			return;
		}
		else
		{
			if ( Alignment & 0x08 )
			{
				m->mtype = Mag_Sumba;
				AddPB ( &m->PBflags, &m->blasts, PB_Golla);
				return;
			}
			else
			{
				if ( Alignment & 0x10 )
				{
					m->mtype = Mag_Ashvinau;
					AddPB ( &m->PBflags, &m->blasts, PB_Pilla);
					return;
				}
			}
		}
		break;
	}
}

void MagLV10Evolution ( MAG* m, unsigned char sectionID, unsigned char type, int EvolutionClass )
{
	switch ( type )
	{
	case CLASS_HUMAR:
	case CLASS_HUNEWEARL:
	case CLASS_HUCAST:
	case CLASS_HUCASEAL:
		m->mtype = Mag_Varuna;
		AddPB ( &m->PBflags, &m->blasts, PB_Farlla);
		break;
	case CLASS_RAMAR:
	case CLASS_RACAST:
	case CLASS_RACASEAL:
	case CLASS_RAMARL:
		m->mtype = Mag_Kalki;
		AddPB ( &m->PBflags, &m->blasts, PB_Estlla);
		break;
	case CLASS_FONEWM:
	case CLASS_FONEWEARL:
	case CLASS_FOMARL:
	case CLASS_FOMAR:
		m->mtype = Mag_Vritra;
		AddPB ( &m->PBflags, &m->blasts, PB_Leilla);
		break;
	}
}

void CheckMagEvolution ( MAG* m, unsigned char sectionID, unsigned char type, int EvolutionClass )
{
	if ( ( m->level < 10 ) || ( m->level >= 35 ) )
	{
		if ( ( m->level < 35 ) || ( m->level >= 50 ) )
		{
			if ( m->level >= 50 )
			{
				if ( ! ( m->level % 5 ) ) // Divisible by 5 with no remainder.
				{
					if ( EvolutionClass <= 3 )
					{
						if ( !MagSpecialEvolution ( m, sectionID, type, EvolutionClass ) )
							MagLV50Evolution ( m, sectionID, type, EvolutionClass );
					}
				}
			}
		}
		else
		{
			if ( EvolutionClass < 2 )
				MagLV35Evolution ( m, sectionID, type, EvolutionClass );
		}
	}
	else
	{
		if ( EvolutionClass <= 0 )
			MagLV10Evolution ( m, sectionID, type, EvolutionClass );
	}
}


void FeedMag (unsigned magid, unsigned itemid, CLIENT* client)
{
	int found_mag = -1;
	int found_item = -1;
	unsigned ch, ch2, mt_index;
	int EvolutionClass = 0;
	MAG* m;
	unsigned short* ft;
	short mIQ, mDefense, mPower, mDex, mMind;

	for (ch=0;ch<client->character.inventoryUse;ch++)
	{
		if (client->character.inventory[ch].item.itemid == magid)
		{
			// Found mag
			if ((client->character.inventory[ch].item.data[0] == 0x02) &&
				(client->character.inventory[ch].item.data[1] <= Mag_Agastya))
			{
				found_mag = ch;
				m = (MAG*) &client->character.inventory[ch].item.data[0];
				for (ch2=0;ch2<client->character.inventoryUse;ch2++)
				{
					if (client->character.inventory[ch2].item.itemid == itemid)
					{
						// Found item to feed
						if  (( client->character.inventory[ch2].item.data[0] == 0x03 ) &&
							( client->character.inventory[ch2].item.data[1]  < 0x07 ) &&
							( client->character.inventory[ch2].item.data[1] != 0x02 ) &&
							( client->character.inventory[ch2].item.data[5] >  0x00 ))
						{
							found_item = ch2;
							switch (client->character.inventory[ch2].item.data[1])
							{
							case 0x00:
								mt_index = client->character.inventory[ch2].item.data[2];
								break;
							case 0x01:
								mt_index = 3 + client->character.inventory[ch2].item.data[2];
								break;
							case 0x03:
							case 0x04:
							case 0x05:
								mt_index = 5 + client->character.inventory[ch2].item.data[1];
								break;
							case 0x06:
								mt_index = 6 + client->character.inventory[ch2].item.data[2];
								break;
							}
						}
						break;
					}
				}
			}
			break;
		}
	}

	if ( ( found_mag == -1 ) || ( found_item == -1 ) )
	{
		Send1A ("Could not find mag to feed or item to feed said mag.", client);
		client->todc = 1;
	}
	else
	{
		DeleteItemFromClient ( itemid, 1, 0, client );

		// Rescan to update Mag pointer (if changed due to clean up)
		for (ch=0;ch<client->character.inventoryUse;ch++)
		{
			if (client->character.inventory[ch].item.itemid == magid)
			{
				// Found mag (again)
				if ((client->character.inventory[ch].item.data[0] == 0x02) &&
					(client->character.inventory[ch].item.data[1] <= Mag_Agastya))
				{
					found_mag = ch;
					m = (MAG*) &client->character.inventory[ch].item.data[0];
					break;
				}
			}
		}
			
		// Feed that mag (Updates to code by Lee from schtserv.com)
		switch (m->mtype)
		{
		case Mag_Mag:
			ft = &Feed_Table0[0];
			EvolutionClass = 0;
			break;
		case Mag_Varuna:
		case Mag_Vritra:
		case Mag_Kalki:
			EvolutionClass = 1;
			ft = &Feed_Table1[0];
			break;
		case Mag_Ashvinau:
		case Mag_Sumba:
		case Mag_Namuci:
		case Mag_Marutah:
		case Mag_Rudra:
			ft = &Feed_Table2[0];
			EvolutionClass = 2;
			break;
		case Mag_Surya:
		case Mag_Tapas:
		case Mag_Mitra:
			ft = &Feed_Table3[0];
			EvolutionClass = 2;
			break;
		case Mag_Apsaras:
		case Mag_Vayu:
		case Mag_Varaha:
		case Mag_Ushasu:
		case Mag_Kama:
		case Mag_Kaitabha:
		case Mag_Kumara:
		case Mag_Bhirava:
			EvolutionClass = 3;
			ft = &Feed_Table4[0];
			break;
		case Mag_Ila:
		case Mag_Garuda:
		case Mag_Sita:
		case Mag_Soma:
		case Mag_Durga:
		case Mag_Nandin:
		case Mag_Yaksa:
		case Mag_Ribhava:
			EvolutionClass = 3;
			ft = &Feed_Table5[0];
			break;
		case Mag_Andhaka:
		case Mag_Kabanda:
		case Mag_Naga:
		case Mag_Naraka:
		case Mag_Bana:
		case Mag_Marica:
		case Mag_Madhu:
		case Mag_Ravana:
			EvolutionClass = 3;
			ft = &Feed_Table6[0];
			break;
		case Mag_Deva:
		case Mag_Rukmin:
		case Mag_Sato:
			ft = &Feed_Table5[0];
			EvolutionClass = 4;
			break;
		case Mag_Rati:
		case Mag_Pushan:
		case Mag_Bhima:
			ft = &Feed_Table6[0];
			EvolutionClass = 4;
			break;
		default:
			ft = &Feed_Table7[0];
			EvolutionClass = 4;
			break;
		}
		mt_index *= 6;
		m->synchro += ft[mt_index];
		if (m->synchro < 0)
			m->synchro = 0;
		if (m->synchro > 120)
			m->synchro = 120;
		mIQ = m->IQ;
		mIQ += ft[mt_index+1];
		if (mIQ < 0)
			mIQ = 0;
		if (mIQ > 200)
			mIQ = 200;
		m->IQ = (unsigned char) mIQ;

		// Add Defense

		mDefense  = m->defense % 100;
		mDefense += ft[mt_index+2];

		if ( mDefense < 0 )
			mDefense = 0;

		if ( mDefense >= 100 )
		{
			if ( m->level == 200 )
				mDefense = 99; // Don't go above level 200
			else
				m->level++; // Level up!
			m->defense  = ( ( m->defense / 100 ) * 100 ) + mDefense;
			CheckMagEvolution ( m, client->character.sectionID, client->character._class, EvolutionClass );
		}
		else
			m->defense  = ( ( m->defense / 100 ) * 100 ) + mDefense;

		// Add Power

		mPower  = m->power % 100;
		mPower += ft[mt_index+3];

		if ( mPower < 0 )
			mPower = 0;

		if ( mPower >= 100 )
		{
			if ( m->level == 200 )
				mPower = 99; // Don't go above level 200
			else
				m->level++; // Level up!
			m->power  = ( ( m->power / 100 ) * 100 ) + mPower;
			CheckMagEvolution ( m, client->character.sectionID, client->character._class, EvolutionClass );
		}
		else
			m->power  = ( ( m->power / 100 ) * 100 ) + mPower;

		// Add Dex

		mDex  = m->dex % 100;
		mDex += ft[mt_index+4];

		if ( mDex < 0 )
			mDex = 0;

		if ( mDex >= 100 )
		{
			if ( m->level == 200 )
				mDex = 99; // Don't go above level 200
			else
				m->level++; // Level up!
			m->dex  = ( ( m->dex / 100 ) * 100 ) + mDex;
			CheckMagEvolution ( m, client->character.sectionID, client->character._class, EvolutionClass );
		}
		else
			m->dex  = ( ( m->dex / 100 ) * 100 ) + mDex;

		// Add Mind

		mMind  = m->mind % 100;
		mMind += ft[mt_index+5];

		if ( mMind < 0 )
			mMind = 0;

		if ( mMind >= 100 )
		{
			if ( m->level == 200 )
				mMind = 99; // Don't go above level 200
			else
				m->level++; // Level up!
			m->mind  = ( ( m->mind / 100 ) * 100 ) + mMind;
			CheckMagEvolution ( m, client->character.sectionID, client->character._class, EvolutionClass );
		}
		else
			m->mind  = ( ( m->mind / 100 ) * 100 ) + mMind;
	}
}
