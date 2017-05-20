/****************************************************************/
/*	Author:	gatchi (github.com/gatchi/PSONOVA-Server)           */
/*	Date:	05/09/2017                                          */
/*	Description:  Modifies MySQL account data.                  */
/*                                                              */
/****************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include	<windows.h>
#endif
#include <mysql.h>

#define STR_LEN 80  // Max length of strings in psonova.ini (tethealla.ini)

int main()
{
	FILE * fp;
	char config_data[STR_LEN] = {0};
	char mySQL_Database[STR_LEN], mySQL_Host[STR_LEN], mySQL_Password[STR_LEN], mySQL_Username[STR_LEN];
	int mySQL_Port, config_index = 0;
	char inputstr[STR_LEN] = {0};  // Using STR_LEN here just because its a nice, big number
	MYSQL * myData;
	char myQuery[200] = {0};  // Same here with 200
	
	// End program if settings file doesnt exist
	if ((fp = fopen ("tethealla.ini", "r" )) == NULL)
	{
		printf ("The configuration file tethealla.ini appears to be missing.\n");
		return 1;
	}
	
	// Extract data from settings file and close
	while ((fgets (config_data, STR_LEN, fp) != NULL) && (config_index < 5))  // Only needs first four lines
	{
		if (config_data[0] != '#')  // Comments start with a hash symbol
		{
			// First three settings are strings that need their newlines & carriage returns stripped
			if (config_index < 4)
			{
				char len = strlen (config_data);
				if (config_data[len-1] == '\n') config_data[len-1] = '\0';
				if (config_data[len-2] == '\r') config_data[len-2] = '\0';
			}
			
			// Copy data into memory
			switch (config_index)
			{
				case 0:
					strcpy (mySQL_Host, config_data);
					break;
				case 1:
					strcpy (mySQL_Username, config_data);
					break;
				case 2:
					strcpy (mySQL_Password, config_data);
					break;
				case 3:
					strcpy (mySQL_Database, config_data);
					break;
				case 4:
					mySQL_Port = atoi (config_data);
					break;
				default:
					break;
			}
			config_index++;
		}
		
		// Keep config_data clean
		memset (config_data, 0, STR_LEN * sizeof (char));
	}
	fclose (fp);
	
	// Connect to MySQL account database
	myData = mysql_init ((MYSQL*) 0);
	MYSQL * connected = mysql_real_connect (myData, mySQL_Host, mySQL_Username, mySQL_Password, NULL, mySQL_Port, NULL, 0);
	if (!connected)
	{
		printf ("Can't connect to the mysql server.\n");
		printf ("Error: %s\n", mysql_error (myData));
		mysql_close (myData);
		return 1;
	}
	
	// Try to select database
	if (mysql_select_db (myData, mySQL_Database) < 0)
	{
		// In the linux version this doesnt get triggered when the wrong database is selected!!!
		// It just says cant query the server!!!
		// Why is this getting skipped!!!!!!!!!!!!!
		printf ("Can't select the %s database !\n", mySQL_Database) ;
		mysql_close (myData);
		return 1;
	}
		
	// Print header
	printf ("Tethealla Server Account Modification\n");
	printf ("-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-\n");
	
	// Ask for the username of the account to modify
	printf ("Username of account to edit: ");
	scanf ("%s", inputstr );
	
	// Protect against buffer overflows
	if (strlen (inputstr) > 16)
	{
		printf ("Invalid username.");
		return 1;
	}
	
	// Does account exist?
	sprintf (&myQuery[0], "SELECT * from account_data WHERE username='%s'", inputstr);
	
}