void load_mask_file()
{
	char mask_data[255];
	unsigned ch = 0;

	FILE* fp;

	// Load masks.txt for IP ban masks

	num_masks = 0;

	if ( ( fp = fopen ("masks.txt", "r" ) ) )
	{
		while (fgets (&mask_data[0], 255, fp) != NULL)
		{
			if (mask_data[0] != 0x23)
			{
				ch = strlen (&mask_data[0]);
				if (mask_data[ch-1] == 0x0A)
					mask_data[ch--]  = 0x00;
				mask_data[ch] = 0;
			}
			convertMask (&mask_data[0], ch+1, &ship_banmasks[num_masks++][0]);
		}
	}
}

void load_language_file()
{
	FILE* fp;
	char lang_data[256];
	int langExt = 0;
	unsigned ch;

	for (ch=0;ch<10;ch++)
	{
		languageNames[ch] = malloc ( 256 );
		memset (languageNames[ch], 0, 256);
		languageExts[ch] = malloc ( 256 );
		memset (languageExts[ch], 0, 256);
	}

	if ( ( fp = fopen ("lang.ini", "r" ) ) == NULL )
	{
		printf ("Language file does not exist...\nWill use English only...\n\n");
		numLanguages = 1;
		strcat (languageNames[0], "English");	
	}
	else
	{
		while ((fgets (&lang_data[0], 255, fp) != NULL) && (numLanguages < 10))
		{
			if (!langExt)
			{
				memcpy (languageNames[numLanguages], &lang_data[0], strlen (&lang_data[0])+1);
				for (ch=0;ch<strlen(languageNames[numLanguages]);ch++)
					if ((languageNames[numLanguages][ch] == 10) || (languageNames[numLanguages][ch] == 13))
						languageNames[numLanguages][ch] = 0;
				langExt = 1;
			}
			else
			{
				memcpy (languageExts[numLanguages], &lang_data[0], strlen (&lang_data[0])+1);
				for (ch=0;ch<strlen(languageExts[numLanguages]);ch++)
					if ((languageExts[numLanguages][ch] == 10) || (languageExts[numLanguages][ch] == 13))
						languageExts[numLanguages][ch] = 0;
				numLanguages++;
				printf ("Language %u (%s:%s)\n", numLanguages, languageNames[numLanguages-1], languageExts[numLanguages-1]);
				langExt = 0;
			}
		}
		fclose (fp);
		if (numLanguages < 1)
		{
			numLanguages = 1;
			strcat (languageNames[0], "English");
		}
	}
}

/*
 * So this file basically is a really goofy and roundabout way of getting particular setting from
 * a large ass config file.  But it doesnt even use all of it here, yet parses through the whole thing.
 * Super inefficient.  Lets replace with with JSON.
 *
 * Things we can gather from here and the old file is what its grabbing.  Which is:
 *  server IP address (or auto)
 *  server listen port
 *  num of ship blocks (adjusted for min and max)
 *  max client connections (again, auto adjusted if out of bounds)
 *  login server host name or IP
 *  ship name
 *  any events happening
 *
 * This function also sets rates and calls the function load_mask_file, which is responsible for
 * ban IP masks.  This is a txt file named "masks".
 */
void load_config_file()
{
	int config_index = 0;
	char config_data[255];
	unsigned ch = 0;

	FILE* fp;

	EXPERIENCE_RATE = 1; // Default to 100% EXP

	//printf ("Gonna try to open this file...\n");
	if ( ( fp = fopen ("ship.ini", "r" ) ) == NULL )
	{
		printf ("The configuration file ship.ini appears to be missing.\n");
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	else
	{
		//printf ("Else triggered.\n");
		while (fgets (&config_data[0], 255, fp) != NULL)
		{
			if (config_data[0] != 0x23)  // if not a comment
			{
				// If IP settings, IP Address, or Ship Name
				if ((config_index == 0x00) || (config_index == 0x04) || (config_index == 0x05))
				{
					// Remove newline
					ch = strlen (&config_data[0]);
					if (config_data[ch-1] == 0x0A)
						config_data[ch--]  = 0x00;
					config_data[ch] = 0;
				}
				switch (config_index)
				{
				case 0x00:
					// Server IP address
					{
						// lel doesnt even check if the whole word is there
						if ((config_data[0] == 'A') || (config_data[0] == 'a'))
						{
							autoIP = 1;
						}
						else
						{
							convertIPString (&config_data[0], ch+1, 1, &serverIP[0] );
						}
					}
					break;
				case 0x01:
					// Server Listen Port
					serverPort = atoi (&config_data[0]);
					break;
				case 0x02:
					// Number of blocks
					serverBlocks = atoi (&config_data[0]);
					if (serverBlocks > 10) 
					{
						printf ("You cannot host more than 10 blocks... Adjusted.\n");
						serverBlocks = 10;
					}
					if (serverBlocks == 0)
					{
						printf ("You have to host at least ONE block... Adjusted.\n");
						serverBlocks = 1;
					}
					break;
				case 0x03:
					// Max Client Connections
					serverMaxConnections = atoi (&config_data[0]);
					if ( serverMaxConnections > ( serverBlocks * 180 ) )
					{
						printf ("\nYou're attempting to server more connections than the amount of blocks\nyou're hosting allows.\nAdjusted...\n");
						serverMaxConnections = serverBlocks * 180;
					}
					if ( serverMaxConnections > SHIP_COMPILED_MAX_CONNECTIONS )
					{
						printf ("This copy of the ship serving software has not been compiled to accept\nmore than %u connections.\nAdjusted...\n", SHIP_COMPILED_MAX_CONNECTIONS);
						serverMaxConnections = SHIP_COMPILED_MAX_CONNECTIONS;
					}
					break;
				case 0x04:
					// Login server host name or IP
					{
						unsigned p;
						unsigned alpha;
						alpha = 0;
						for (p=0;p<ch;p++)
							if (((config_data[p] >= 65 ) && (config_data[p] <= 90)) ||
								((config_data[p] >= 97 ) && (config_data[p] <= 122)))
							{
								alpha = 1;
								break;
							}
						if (alpha)
						{
							struct hostent *IP_host;

							config_data[strlen(&config_data[0])-1] = 0x00;
							printf ("Resolving %s ...\n", (char*) &config_data[0] );
							IP_host = gethostbyname (&config_data[0]);
							if (!IP_host)
							{
								printf ("Could not resolve host name.");
								printf ("Press [ENTER] to quit...");
								gets(&dp[0]);
								exit (1);
							}
							*(unsigned *) &loginIP[0] = *(unsigned *) IP_host->h_addr;
						}
						else
							convertIPString (&config_data[0], ch+1, 1, &loginIP[0] );
					}
					break;
				case 0x05:
					// Ship Name
					memset (&Ship_Name[0], 0, 255 );
					memcpy (&Ship_Name[0], &config_data[0], ch+1 );
					Ship_Name[12] = 0x00;
					break;
				case 0x06:
					// Event
					shipEvent = (unsigned char) atoi (&config_data[0]);
					PacketDA[0x04] = shipEvent;
					break;
				case 0x07:
					WEAPON_DROP_RATE = atoi (&config_data[0]);
					break;
				case 0x08:
					ARMOR_DROP_RATE = atoi (&config_data[0]);
					break;
				case 0x09:
					MAG_DROP_RATE = atoi (&config_data[0]);
					break;
				case 0x0A:
					TOOL_DROP_RATE = atoi (&config_data[0]);
					break;
				case 0x0B:
					MESETA_DROP_RATE = atoi (&config_data[0]);
					break;
				case 0x0C:
					EXPERIENCE_RATE = atoi (&config_data[0]);
					if ( EXPERIENCE_RATE > 99 )
					{
						printf ("\nWARNING: You have your experience rate set to a very high number.\n");
						printf ("As of ship_server.exe version 0.038, you now just use single digits\n");
						printf ("to represent 100%% increments.  (ex. 1 for 100%, 2 for 200%)\n\n");
						 ("If you've set the high value of %u%% experience on purpose,\n", EXPERIENCE_RATE * 100 );
						printf ("press [ENTER] to continue, otherwise press CTRL+C to abort.\n");
						printf (":");
						gets   (&dp[0]);
						printf ("\n\n");
					}
					break;
				case 0x0D:
					ship_support_extnpc = atoi (&config_data[0]);
					break;
				default:
					break;
				}
				printf ("Line: %d\n", config_index);
				config_index++;
			}
		}
	}
	fclose (fp);
	//printf ("Finished reading ship config file.\n");
	
	if (config_index < 0x0D)
	{
		printf ("ship.ini seems to be corrupted.\n");
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	common_rates[0] = 100000 / WEAPON_DROP_RATE;
	common_rates[1] = 100000 / ARMOR_DROP_RATE;
	common_rates[2] = 100000 / MAG_DROP_RATE;
	common_rates[3] = 100000 / TOOL_DROP_RATE;
	common_rates[4] = 100000 / MESETA_DROP_RATE;
	load_mask_file();
}
