#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cnsslclient.h>
#include <cnhttpclient.h>
#include <unistd.h>




#ifndef LOADFILEDEFINED
#define LOADFILEDEFINED
static int LoadFileLineIntoBuffer( const char * folder_prefix, const char * file, char * buffer, int buffersize )
{
	char filename[1024];
	snprintf( filename, 1024, "%s/%s", folder_prefix, file );
	FILE * f = fopen( filename, "r" );
	if( !f )
	{
		fprintf( stderr, "Error: can't get client_id.txt\n" );
		return -1;
	}
	int c;
	int i = 0;
	while( ( c = fgetc( f ) ) != EOF && i < buffersize-1 )
	{
		if( c == '\n' ) break;
		buffer[i++] = c;
	}
	buffer[i] = 0;
	fclose( f );
	return i;
}
#endif

int YTPostChat( const char * livechatid, const char * message )
{
	char curlurlbase[8192];
	char auxhead[8192];
	char livechatbuff[128];
	const char * reqtype = "snippet";

	if( livechatid == 0 || livechatid[0] == '-' )
	{
		int len = LoadFileLineIntoBuffer( "..", "live_chat_id.txt", livechatbuff, sizeof( livechatbuff ) );
		if( len < 5 )
		{
			fprintf( stderr, "Error: Live chat ID Invalid\n" );
			return -5;
		}
		livechatid = livechatbuff;
	}
	

	char apikey[8192];
	int uses_api_key = 0;

	sprintf( curlurlbase, "https://www.googleapis.com/youtube/v3/liveChat/messages?part=%s",reqtype);

	if( 0 )
	{
		FILE * f = fopen( "../.ytapikey.txt", "r" );
		if(f)
		{
			fscanf( f, "%8100s", apikey );
			fclose( f );	
			uses_api_key = 1;
		}
		sprintf( curlurlbase + strlen(curlurlbase), "&key=%s",apikey);
	}


	struct cnhttpclientrequest req;
	memset( &req, 0, sizeof( req ) );
	req.host = 0;
	req.port = 0;
	req.URL = curlurlbase;

	if( !uses_api_key )
	{
		char oauthbear[8192];
		FILE * f = fopen( "../.oauthtoken.txt", "r" );
		if( !f )
		{
			fprintf( stderr, "Error: no oauth token found.  Run yt_oauth_helper\n" );
			return -9;
		}
		fscanf( f, "%s", oauthbear );
		fclose( f );	

		sprintf( auxhead, "Authorization: Bearer %s\r\nContent-Type: application/json; charset=UTF-8", oauthbear );
	}
	else
	{
		fprintf( stderr, "WARNING: API KEY NOT VALID FOR SENDING CHAT\n" );
	}

	req.AddedHeaders = auxhead;

	char AuxData[8192];    //For Websockets, this is the "Origin" URL.  Otherwise, it's Post data.

	snprintf( AuxData, sizeof( AuxData ) - 1, "{\
  \"snippet\": { \
    \"liveChatId\": \"%s\", \
    \"type\": \"textMessageEvent\", \
    \"textMessageDetails\": { \
      \"messageText\": \"%s\" \
    } \
  } \
}", livechatid, message );

	printf( "Request payload: %s\n", AuxData );

//	memset( argv[1], '-', strlen( argv[1] ) );
//	memset( argv[2], '-', strlen( argv[2] ) );

	req.AuxData = AuxData;
	req.AuxDataLength = strlen( AuxData );

	//Get a bunch of messages
	struct cnhttpclientresponse * r = CNHTTPClientTransact( &req );

	if( r->payload )
	{
		r->payload[r->payloadlen] = 0;
		//Uncomment this if you aren't getting anything.
		printf( "%s\n", r->payload );
	}
	else
	{
		fprintf( stderr, "No payload.\n" );
	}

}

#ifdef MAKE_EXE

int main( int argc, char ** argv )
{

	if( argc < 2 )
	{
		fprintf( stderr, "Error! Usage: ./chatmon [livechatid] [message]\n" );
		return -5;
	}

	char * message;

	if( argc < 3 )
	{
		int mml = 16384;
		message = malloc( mml+4 );
		int messagelen = 0;
		char line[8192];
		line[0] = 0;
		while ((fgets(line, sizeof line, stdin) != NULL) && ( line[0]  && line[0] != '\n' ) )
		{
			int linelen = strlen( line );
			printf( "GOT line len: %d\n", linelen );
			int i;
			for( i = 0; i < linelen; i++ )
			{
				char c = line[i];
				if( c == 9 ) c = ' ';
				else if( c < 32 ) continue;
				else if( c == '\"' || c == '\\' )
				{
					message[messagelen++] = '\\';
					message[messagelen++] = c;
				}
				else
					message[messagelen++] = c;

				if( messagelen > mml ) break;				
			}
			if( messagelen > mml ) break;
			message[messagelen++] = ' ';
			message[messagelen] = 0;
			line[0] = 0;
			//printf( "Emitting message length: %d\n", messagelen );
			//puts( message );
		}
	}
	else
	{
		message = argv[2];
	}

	return YTPostChat( strdup(argv[1]), message );
}
#endif

