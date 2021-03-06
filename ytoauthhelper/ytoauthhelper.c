//Copyright <>< Charles Lohr
//This file may be licensed under the MIT/x11 or NewBSD License.

//Based on guide here: https://developers.google.com/youtube/v3/guides/auth/client-side-web-apps
//Geared around offline mode: https://developers.google.com/identity/protocols/OAuth2WebServer#offline

//TODO: also disable tokens... https://accounts.google.com/o/oauth2/revoke 

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <cnsslclient.h>
#include <cnhttpclient.h>
#include <unistd.h>
#include <string.h>
#include <http_bsd.h>
#include <cnhttp.h>
#include <osg_aux.h>
#include <os_generic.h>

int port = 8089;
char * client_secret;
char * client_id;
char oauthresp[2048];
volatile int oauthrespgot;

static void oauth2cb()
{
	URLDecode( oauthresp, 1024, curhttp->pathbuffer+18 );
	printf( "OAUTH2CB: %s\n", oauthresp );
	if( strncmp( oauthresp, "code=", 5 ) != 0 )
	{
		fprintf( stderr, "Error received on key.\n%s\n", oauthresp );
		DataStartPacket();
		PushString( "Auth failed.\r\n" );
		PushString( oauthresp );
		EndTCPWrite( curhttp->socket );

		curhttp->state = HTTP_WAIT_CLOSE;
		return;
	}
	oauthrespgot = 1;


	struct cnhttpclientrequest req;
	memset( &req, 0, sizeof( req ) );
	req.host = 0;
	req.port = 0;
	req.URL = "https://accounts.google.com/o/oauth2/token";
	char auxhead[8192];
	req.AddedHeaders = auxhead;
	sprintf( auxhead, "Content-Type: application/x-www-form-urlencoded" );

	char content[8192];
	snprintf( content, 8191, "%s&client_id=%s&client_secret=%s&redirect_uri=http%%3A%%2F%%2Flocalhost:%d%%2Fd%%2foauth2callback&grant_type=authorization_code", oauthresp, client_id, client_secret, port );

	req.AuxData = content;
	req.AuxDataLength = strlen( content );

	printf( "Submitting OAUTH Request: %s\n", content );

	if( 1 )
	{
		FILE * f = fopen( "authdebug.html", "w" );
		fprintf( f, "<HTML>\n<BODY>\n<FORM action=\"https://accounts.google.com/o/oauth2/token\" method=\"POST\" enctype=\"application/x-www-form-urlencoded\">\n" );
		fprintf( f, "<INPUT TYPE=TEXT name=code value=\"%s\">\n", oauthresp+5 );
		fprintf( f, "<INPUT TYPE=TEXT name=client_id value=\"%s\">\n", client_id );
		fprintf( f, "<INPUT TYPE=PASSWORD name=client_secret value=\"%s\">\n", client_secret );
		fprintf( f, "<INPUT TYPE=TEXT name=grant_type VALUE=authorization_code>\n" );
		fprintf( f, "<INPUT TYPE=TEXT name=redirect_uri VALUE=\"http://localhost:%d/d/oauth2callback\">\n", port );
		fprintf( f, "<INPUT TYPE=SUBMIT></FORM></BODY></HTML>\n" );
		fclose( f );
		//exit( 0 );
	}

	//Get a bunch of messages
	struct cnhttpclientresponse * r = CNHTTPClientTransact( &req );

	printf( "Got response (%d octets): %s\n", r->payloadlen, r->payload );

	if( !r->payload )
	{
		fprintf( stderr, "Error: empty response received.\n" );
		exit( -5 );
	}

	//printf( "PAYLOAD: %s\n", r->payload );

	char * foundkey = strstr( r->payload, "access_token" );
	char * endkey;
	if( foundkey ) foundkey = strchr( foundkey+14, '\"' );
	if( foundkey ) endkey = strchr( ++foundkey, '\"' );
	if( !foundkey || !endkey )
	{
		fprintf( stderr, "Error: can't find token.  Got: %s\n", r->payload );
		exit( -18 );
	}

	char * foundexpires = strstr( r->payload, "expires_in" );
	char * endexpires;
	if( foundexpires ) foundexpires = strchr( foundexpires+12, '\"' );
	if( foundexpires ) endexpires = strchr( ++foundexpires, '\"' );
	uint64_t expires_in = foundexpires?atoi(foundexpires):3600;

	char * foundrenew = strstr( r->payload, "refresh_token" );
	char * endrenew;
	if( foundrenew ) foundrenew = strchr( foundrenew+16, '\"' );
	if( foundrenew ) endrenew = strchr( ++foundrenew, '\"' );
	if( !foundrenew || !endrenew )
	{
		fprintf( stderr, "Error: can't find token. Got: %s\n", r->payload );
		exit( -18 );
	}
	*endkey = 0;
	*endrenew = 0;


	FILE * f = fopen( "../.oauthtoken.txt", "w" );
	fprintf( f, "%s\n", foundkey );
	fclose( f );


	f = fopen( "../.oauthrenew.txt", "w" );
	fprintf( f, "%s\n", foundrenew );
	fclose( f );

	f = fopen( "../.oauthexpiration.txt", "w" );
	fprintf( f, "%ld\n", (uint64_t)OGGetAbsoluteTime() + expires_in );
	fclose( f );

	DataStartPacket();
	PushString( "<HTML><BODY ONLOAD=\"window.open('','_parent','');window.close();\">Auth ok.  You can close this page.</BODY></HTML>" );
	EndTCPWrite( curhttp->socket );
	DataStartPacket();

	printf( "Key written to ../.oauthtoken.txt\n" );
	printf( "Renew written to ../.oauthrenew.txt\n" );

	curhttp->state = HTTP_WAIT_CLOSE;

	HTTPClose();

	TermHTTPServer();
}

void HTTPCustomCallback( )
{
	if( curhttp->rcb )
		((void(*)())curhttp->rcb)();
	else
		curhttp->isdone = 1;
}


//Close of curhttp happened.
void CloseEvent()
{
}


void NewWebSocket()
{
}



void WebSocketTick()
{
}

void WebSocketData( int len )
{
}


void HTTPCustomStart( )
{
	if( strncmp( (const char*)curhttp->pathbuffer, "/d/oauth2callback", 17 ) == 0 )
	{
		curhttp->rcb = (void(*)())&oauth2cb;
		curhttp->bytesleft = 0xffffffff;
	}
	else
	{
		curhttp->rcb = 0;
		curhttp->bytesleft = 0;
	}
	curhttp->isfirst = 1;
	HTTPHandleInternalCallback();
}

int main( int argc, char ** argv )
{
	RunHTTP( port );
	FILE * f;

	client_id = OSGLineFromFile( f = fopen( "../.client_id.txt", "r" ) );     if( f ) fclose( f );
	client_secret = OSGLineFromFile( fopen( "../.client_secret.txt", "r" ) ); if( f ) fclose( f );
	if( !client_id ) { fprintf( stderr, "Error opening client_id.txt\n" ); return -1; }
	if( !client_secret ) { fprintf( stderr, "Error opening client_secret.txt\n" ); return -1; }
	
	printf( "Please visit:\n\nhttps://accounts.google.com/o/oauth2/auth?client_id=%s&scope=https://www.googleapis.com/auth/youtube&response_type=code&access_type=offline&redirect_uri=http%%3A%%2F%%2Flocalhost:%d%%2Fd%%2foauth2callback\n", client_id, port );

	while(!oauthrespgot)
	{
		TickHTTP();
		usleep( 30000 );
		printf( "%d\n", oauthrespgot );
	}

	exit( 0 );
	missing_files:
	printf( "Missing files ../.client_secret.txt and/or ../.client_id.txt.\n\
		Log into your API Dashboard.\n\
		https://console.developers.google.com/\n\
		Select your project and go to the youtube section.\n\
		Create new oauth credentials.\n\
\n\
		Select type for new oauth app to be \"other\"\n\
\n\
		Get the CLIENT_ID and note the SECRET_KEY.\n\
\n\
		Write them into ../.client_id.txt and../.client_secret.txt.\n" );
	return -9;
}

