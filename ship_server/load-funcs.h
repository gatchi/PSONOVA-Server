// Load CSV into RAM
void LoadCSV(const char* filename)
{
	FILE* fp;
	char csv_data[1024];
	unsigned ch, ch2, ch3 = 0;
	//unsigned ch4;
	int open_quote = 0;
	char* csv_param;
	
	csv_lines = 0;
	memset (&csv_params, 0, sizeof (csv_params));

	//printf ("Loading CSV file %s ...\n", filename );

	if ( ( fp = fopen (filename, "r" ) ) == NULL )
	{
		printf ("The parameter file %s appears to be missing.\n", filename);
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}

	while (fgets (&csv_data[0], 1023, fp) != NULL)
	{
		// ch2 = current parameter we're on
		// ch3 = current index into the parameter string
		ch2 = ch3 = 0;
		open_quote = 0;
		csv_param = csv_params[csv_lines][0] = malloc (256); // allocate memory for parameter
		for (ch=0;ch<strlen(&csv_data[0]);ch++)
		{
			if ((csv_data[ch] == 44) && (!open_quote)) // comma not surrounded by quotations
			{
				csv_param[ch3] = 0; // null terminate current parameter
				ch3 = 0;
				ch2++; // new parameter
				csv_param = csv_params[csv_lines][ch2] = malloc (256); // allocate memory for parameter
			}
			else
			{
				if (csv_data[ch] == 34) // quotation mark
					open_quote = !open_quote;
				else
					if (csv_data[ch] > 31) // no loading low ascii
						csv_param[ch3++] = csv_data[ch];
			}
		}
		if (ch3)
		{
			ch2++;
			csv_param[ch3] = 0;
		}
		/*
		for (ch4=0;ch4<ch2;ch4++)
			printf ("%s,", csv_params[csv_lines][ch4]);
		printf ("\n");
		*/
		csv_lines++;
		if (csv_lines > 1023)
		{
			printf ("CSV file has too many entries.\n");
			printf ("Press [ENTER] to quit...");
			gets(&dp[0]);
			exit (1);
		}
	}
	printf ("Loaded %u lines...\r\n", csv_lines);
	fclose (fp);
}

void LoadArmorParam()
{
	unsigned ch,wi1;

	LoadCSV ("param\\armorpmt.ini");
	for (ch=0;ch<csv_lines;ch++)
	{
		wi1 = hexToByte (&csv_params[ch][0][6]);
		armor_dfpvar_table[wi1] = (unsigned char) atoi (csv_params[ch][17]);
		armor_evpvar_table[wi1] = (unsigned char) atoi (csv_params[ch][18]);
		armor_equip_table[wi1] = (unsigned char) atoi (csv_params[ch][10]);
		armor_level_table[wi1] = (unsigned char) atoi (csv_params[ch][11]);
		//printf ("armor index %02x, dfp: %u, evp: %u, eq: %u, lv: %u \n", wi1, armor_dfpvar_table[wi1], armor_evpvar_table[wi1], armor_equip_table[wi1], armor_level_table[wi1]);
	}
	FreeCSV ();
	LoadCSV ("param\\shieldpmt.ini");
	for (ch=0;ch<csv_lines;ch++)
	{
		wi1 = hexToByte (&csv_params[ch][0][6]);
		barrier_dfpvar_table[wi1] = (unsigned char) atoi (csv_params[ch][17]);
		barrier_evpvar_table[wi1] = (unsigned char) atoi (csv_params[ch][18]);
		barrier_equip_table[wi1] = (unsigned char) atoi (csv_params[ch][10]);
		barrier_level_table[wi1] = (unsigned char) atoi (csv_params[ch][11]);
		//printf ("barrier index %02x, dfp: %u, evp: %u, eq: %u, lv: %u \n", wi1, barrier_dfpvar_table[wi1], barrier_evpvar_table[wi1], barrier_equip_table[wi1], barrier_level_table[wi1]);
	}
	FreeCSV ();
	// Set up the stack table too.
	for (ch=0;ch<0x09;ch++)
	{
		if (ch != 0x02)
			stackable_table[ch] = 10;
	}
	stackable_table[0x10] = 99;

}

void LoadWeaponParam()
{
	unsigned ch,wi1,wi2;

	LoadCSV ("param\\weaponpmt.ini");
	for (ch=0;ch<csv_lines;ch++)
	{
		wi1 = hexToByte (&csv_params[ch][0][4]);
		wi2 = hexToByte (&csv_params[ch][0][6]);
		weapon_equip_table[wi1][wi2] = (unsigned) atoi (csv_params[ch][6]);
		*(unsigned short*) &weapon_atpmax_table[wi1][wi2] = (unsigned) atoi (csv_params[ch][8]);
		grind_table[wi1][wi2] = (unsigned char) atoi (csv_params[ch][14]);
		if (( ((wi1 >= 0x70) && (wi1 <= 0x88)) ||
			  ((wi1 >= 0xA5) && (wi1 <= 0xA9)) ) &&
			  (wi2 == 0x10))
			special_table[wi1][wi2] = 0x0B; // Fix-up S-Rank King's special
		else
			special_table[wi1][wi2] = (unsigned char) atoi (csv_params[ch][16]);
		//printf ("weapon index %02x%02x, eq: %u, grind: %u, atpmax: %u, special: %u \n", wi1, wi2, weapon_equip_table[wi1][wi2], grind_table[wi1][wi2], weapon_atpmax_table[wi1][wi2], special_table[wi1][wi2] );
	}
	FreeCSV ();
}

void LoadTechParam()
{
	unsigned ch,ch2;

	LoadCSV ("param\\tech.ini");
	if (csv_lines != 19)
	{
		printf ("Technique CSV file is corrupt.\n");
		printf ("Press [ENTER] to quit...");
		gets(&dp[0]);
		exit (1);
	}
	for (ch=0;ch<19;ch++) // For technique
	{
		for (ch2=0;ch2<12;ch2++) // For class
		{
			if (csv_params[ch][ch2+1])
				max_tech_level[ch][ch2] = ((char) atoi (csv_params[ch][ch2+1])) - 1;
			else
			{
				printf ("Technique CSV file is corrupt.\n");
				printf ("Press [ENTER] to quit...");
				gets(&dp[0]);
				exit (1);
			}
		}
	}
	FreeCSV ();
}

void LoadShopData2()
{
	FILE *fp;
	fp = fopen ("shop\\shop2.dat", "rb");
	if (!fp)
	{
		printf ("shop\\shop2.dat is missing.");
		printf ("Press [ENTER] to quit...");
		gets (&dp[0]);
		exit (1);
	}
	fread (&equip_prices[0], 1, sizeof (equip_prices), fp);
	fclose (fp);
}
