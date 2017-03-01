unsigned wstrlen ( unsigned short* dest )
{
	unsigned l = 0;
	while (*dest != 0x0000)
	{
		l+= 2;
		dest++;
	}
	return l;
}

void wstrcpy ( unsigned short* dest, const unsigned short* src )
{
	while (*src != 0x0000)
		*(dest++) = *(src++);
	*(dest++) = 0x0000;
}

void wstrcpy_char ( char* dest, const char* src )
{
	while (*src != 0x00)
	{
		*(dest++) = *(src++);
		*(dest++) = 0x00;
	}
	*(dest++) = 0x00;
	*(dest++) = 0x00;
}

void packet_to_text ( unsigned char* buf, int len )
{
	int c, c2, c3, c4;

	c = c2 = c3 = c4 = 0;

	for (c=0;c<len;c++)
	{
		if (c3==16)
		{
			for (;c4<c;c4++)
				if (buf[c4] >= 0x20) 
					dp[c2++] = buf[c4];
				else
					dp[c2++] = 0x2E;
			c3 = 0;
			sprintf (&dp[c2++], "\n" );
		}

		if ((c == 0) || !(c % 16))
		{
			sprintf (&dp[c2], "(%04X) ", c);
			c2 += 7;
		}

		sprintf (&dp[c2], "%02X ", buf[c]);
		c2 += 3;
		c3++;
	}

	if ( len % 16 )
	{
		c3 = len;
		while (c3 % 16)
		{
			sprintf (&dp[c2], "   ");
			c2 += 3;
			c3++;
		}
	}

	for (;c4<c;c4++)
		if (buf[c4] >= 0x20) 
			dp[c2++] = buf[c4];
		else
			dp[c2++] = 0x2E;

	dp[c2] = 0;
}


void display_packet ( unsigned char* buf, int len )
{
	packet_to_text ( buf, len );
	printf ("%s\n\n", &dp[0]);
}

/*
 * Converts IP address to a string...?
 *
 * IPData     - 
 */
void convertIPString (char* IPData, unsigned IPLen, int fromConfig, unsigned char* IPStore )
{
	unsigned p,p2,p3;
	char convert_buffer[5];

	p2 = 0;
	p3 = 0;
	for (p=0;p<IPLen;p++)
	{
		if ((IPData[p] > 0x20) && (IPData[p] != 46))
			convert_buffer[p3++] = IPData[p]; else
		{
			convert_buffer[p3] = 0;
			if (IPData[p] == 46) // .
			{
				IPStore[p2] = atoi (&convert_buffer[0]);
				p2++;
				p3 = 0;
				if (p2>3)
				{
					if (fromConfig)
						printf ("ship.ini is corrupted. (Failed to read IP information from file!)\n"); else
						printf ("Failed to determine IP address.\n");
					printf ("Press [ENTER] to quit...");
					gets(&dp[0]);
					exit (1);
				}
			}
			else
			{
				IPStore[p2] = atoi (&convert_buffer[0]);
				if (p2 != 3)
				{
					if (fromConfig)
						printf ("ship.ini is corrupted. (Failed to read IP information from file!)\n"); else
						printf ("Failed to determine IP address.\n");
					printf ("Press [ENTER] to quit...");
					gets(&dp[0]);
					exit (1);
				}
				break;
			}
		}
	}
}

void convertMask (char* IPData, unsigned IPLen, unsigned short* IPStore )
{
	unsigned p,p2,p3;
	char convert_buffer[5];

	p2 = 0;
	p3 = 0;
	for (p=0;p<IPLen;p++)
	{
		if ((IPData[p] > 0x20) && (IPData[p] != 46))
			convert_buffer[p3++] = IPData[p]; else
		{
			convert_buffer[p3] = 0;
			if (IPData[p] == 46) // .
			{
				if (convert_buffer[0] == 42)
					IPStore[p2] = 0x8000;
				else
					IPStore[p2] = atoi (&convert_buffer[0]);
				p2++;
				p3 = 0;
				if (p2>3)
				{
					printf ("Bad mask encountered in masks.txt...\n");
					memset (&IPStore[0], 0, 8 );
					break;
				}
			}
			else
			{
				IPStore[p2] = atoi (&convert_buffer[0]);
				if (p2 != 3)
				{
					printf ("Bad mask encountered in masks.txt...\n");
					memset (&IPStore[0], 0, 8 );
					break;
				}
				break;
			}
		}
	}
}


unsigned char hexToByte ( char* hs )
{
	unsigned b;

	if ( hs[0] < 58 ) b = (hs[0] - 48); else b = (hs[0] - 55);
	b *= 16;
	if ( hs[1] < 58 ) b += (hs[1] - 48); else b += (hs[1] - 55);
	return (unsigned char) b;
}
