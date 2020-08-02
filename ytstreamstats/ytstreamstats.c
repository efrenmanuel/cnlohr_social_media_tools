#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cnsslclient.h>
#include <cnhttpclient.h>
#include <jsmn.h>
#include <unistd.h>


int main( int argc, char ** argv )
{
	char curlurlbase[8192];
	char auxhead[8192];
	jsmntok_t tokens[131072];
	jsmn_parser jsmnp;

	if( argc != 2 )
	{
		fprintf( stderr, "Error! Usage: ./chatmon [stream]\n" );
		return -5;
	}	

	const char * vidid = strdup(argv[1]);
	const char * reqtype = "liveStreamingDetails";

	char apikey[8192];
	int uses_api_key = 0;

	{
		FILE * f = fopen( "../.ytapikey.txt", "r" );
		if(f)
		{
			fscanf( f, "%8100s", apikey );
			fclose( f );	
			uses_api_key = 1;
		}
	}
	sprintf( curlurlbase, "https://www.googleapis.com/youtube/v3/videos?id=%s&part=%s", vidid,reqtype);

	if( uses_api_key )
	{
		//fprintf( stderr, "Warning: Using API key instead of OAUTH\n" );
		sprintf( curlurlbase + strlen(curlurlbase), "&key=%s", apikey );
	}
//	fprintf( stderr, "Request: %s\n", curlurlbase);

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
		sprintf( auxhead, "Authorization: Bearer %s", oauthbear );
		req.AddedHeaders = auxhead;
	}
	else
	{
		req.AddedHeaders = 0;
	}

	//char AuxData[8192];    //For Websockets, this is the "Origin" URL.  Otherwise, it's Post data.
	req.AuxData = 0;
	req.AuxDataLength = 0;


	//Get a bunch of messages
	struct cnhttpclientresponse * r = CNHTTPClientTransact( &req );

	if( !r->payload )
	{
		fprintf( stderr, "No payload.\n" );
		return -9;
	}

//    "actualStartTime": "2018-05-08T05:41:05.000Z",
//    "concurrentViewers": "1",
//    "activeLiveChatId": "EiEKGFVDUXM4Z19Ddy1IMFN0ZUxtNUVuSkJfdxIFL2xpdmU"

	r->payload[r->payloadlen] = 0;
	//Uncomment this if you aren't getting anything.
	printf( "%s\n", r->payload );

}

