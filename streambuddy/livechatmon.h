char ChatWindowText[2048];

void * RunChatMon( void * v )
{
	int clf = open( "../chatlog.txt", O_RDONLY );
	if( clf <= 0 )
	{
		sprintf( ChatWindowText, "Error: can't open chat log.\n" );
	}
	int i;
	for( i = 0; i < sizeof( ChatWindowText )-1; i++ )
	{
		ChatWindowText[i] = ' ';
	}
	ChatWindowText[i] = 0;

	int already_init = 0;

	fcntl( clf, F_SETFL, O_NONBLOCK );
	while(!doquit)
	{
		char bufferout[2000];
		char buffer[500];
		int r = read( clf, buffer, sizeof( buffer ) - 1 );
		if( r < 0 )
		{
			sprintf( ChatWindowText, "Error: Fault reading from chat window file.\n" );
			break;
		}

		if( r == 0 )
		{
			already_init = 1;
			OGUSleep( 100000 );
			continue;
		}

		int ro = 0;
		for( i = 0; i < r; i++ )
		{
			if( buffer[i] == '\t' )
			{
				bufferout[ro++] = '-';
				bufferout[ro++] = '-';
				bufferout[ro++] = '-';
				bufferout[ro++] = '\n';
				bufferout[ro++] = '\t';
			}
			if( buffer[i] == '\n' )
			{
				bufferout[ro++] = '\n';
			}
			else if( buffer[i] == '\\')
			{
				if( i < r-1 )
				{
					i++;
					bufferout[ro++] = buffer[i];
				}
			}
			else
				bufferout[ro++] = buffer[i];
		}
		bufferout[ro] = 0;

		int end = sizeof( ChatWindowText );
		for( i = 0; i < end - ro-1; i++ )
		{
			ChatWindowText[i] = ChatWindowText[i+ro];
		}

		int j = 0;
		memcpy( ChatWindowText+i, bufferout, ro );
		if( !already_init ) continue;




		if( already_init && strstr( bufferout, "boom" ) )
		{
			srand( OGGetAbsoluteTime()*1000 );
			MakeFirework( rand()%(BRD_X),  rand()%(BRD_Y - 200) + 100, 1.0 );
		}
		if( already_init && strstr( bufferout, "moob" ) )
		{
			srand( OGGetAbsoluteTime()*1000 );
			MakeFirework( rand()%(BRD_X),  rand()%(BRD_Y - 200) + 100, -1.0 );
		}


		//printf( "%d %d %d,%s\n", ChatWindowText[i], i, r, ChatWindowText );
		OGUSleep( 100000 );
	}
}


