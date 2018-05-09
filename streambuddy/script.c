#include "tccexports.h"
#include <stdarg.h>
#include "../cntools/os_generic.h"
#include <unistd.h>
#include <fcntl.h>

#define FULL_1080P

#define STREAMID   "16NSQORHRqU"
#define LIVECHATID "EiEKGFVDRzd5SVd0VndjRU5nX1pTLW5haGc1ZxIFL2xpdmU"

#ifdef FULL_1080P

#define BRD_X 1920
#define BRD_Y 1056
#define WIN_X 1400
#define WIN_Y 956
#define CHAT_Y 500
#define DEFAULT_SIZE 4
#define BIG_SIZE 8
#define HUGE_SIZE 10
#define STATS_X 300

#else

#define BRD_X 1280
#define BRD_Y 710
#define WIN_X 960
#define WIN_Y 630
#define CHAT_Y 300
#define DEFAULT_SIZE 3
#define BIG_SIZE 4
#define HUGE_SIZE 8
#define STATS_X 180

#endif


void DrawFatTextAt( int x, int y, int size, int width, int height, char * format, ... );
int spawn_process_with_pipes( const char * execparam, char * const argv[], int pipefd[3] );
int waitpid(pid_t pid, int *status, int options);
int kill(pid_t pid, int sig);
char * strchr( const char *, char );


//At bottom of code, send message to chat.
void SendChatMessage( const char * message );

struct ScriptStructure
{
	int lastx, lasty;
	int compiles;
};

int chat_process;
int chatpipes[3];

og_thread_t songthread;
og_thread_t chatthread;
og_thread_t statusthread;

struct ScriptStructure * id;
int doquit;

#include "streamstatus.h"
#include "livechatmon.h"
#include "nowplaying.h"
#include "fireworks.h"
#include "colorchord.h"

int init( struct ScriptStructure * cid )
{
	printf( "Init\n" );
}

int start( struct ScriptStructure * cid )
{
	printf( "Start\n" );
	cid->compiles++;
	id = cid;
	StartColorChord();
	songthread = OGCreateThread( RunNowPlaying, 0 );
	chatthread = OGCreateThread( RunChatMon, 0 );
	statusthread = OGCreateThread( RunStreamStatus, 0 );
}

int stop( struct ScriptStructure * cid )
{
	doquit = 1;
	StopColorChord();
	OGJoinThread( songthread );
	OGJoinThread( chatthread );
	OGJoinThread( statusthread );

	printf( "Stop\n" );

	if( chat_process )
	{
		int ret;
		waitpid(chat_process, &ret, 0);
	}
}

void DrawCursor()
{
	CNFGColor( 0x8c3000 );
	CNFGTackRectangle( id->lastx-5, id->lasty-5, id->lastx+5, id->lasty+5 );
}


void DrawTextOverlay()
{
	CNFGColor( 0xffffff );
	DrawFatTextAt( BIG_SIZE*23, WIN_Y+5, DEFAULT_SIZE, -1, -1, "%s", NowPlaying );

	DrawFatTextAt( WIN_X + 4, CHAT_Y, DEFAULT_SIZE, BRD_X-WIN_X-30, BRD_Y - CHAT_Y, "%s", ChatWindowText );

	//DrawFatTextAt( WIN_X - STATS_X, WIN_Y+5, HUGE_SIZE, -1, -1, "?:??:??" );
	DrawFatTextAt( WIN_X - STATS_X, WIN_Y+5+45, BIG_SIZE, -1, -1, 
(CurrentViewers>0)?"%d WATCHING":"STREAM STATUS ERROR", CurrentViewers );

	//printf( "%f\n", OGGetAbsoluteTime() );
	
}

int update( struct ScriptStructure * cid )
{
	static int i;
	//CNFGClearTransparencyLevel();
	//CNFGClearFrame();
	CNFGHandleInput();

	CNFGDrawToTransparencyMode( 1 );
	CNFGColor( 0xffffff );
	CNFGTackRectangle( 0, 0, BRD_X, BRD_Y );
	CNFGColor( 0x000000 );
	CNFGTackRectangle( 0, 0, WIN_X, WIN_Y );

	CNFGDrawToTransparencyMode( 0 );

	CNFGColor( 0x0202020 );
	CNFGTackRectangle( 0, WIN_Y, WIN_X, BRD_Y );
	CNFGTackRectangle( WIN_X, 0, BRD_X, BRD_Y );

	DrawColorChord();
	DrawFireworks();
	DrawTextOverlay();
	DrawCursor();

	CNFGSwapBuffers();
	OGUSleep( 30000 );
	//printf( "Update: %d %d\n", cid->compiles, 1 );
}


void handleKeyCB( struct ScriptStructure * cid, int keycode, int bDown )
{
	//Can't receive keypresses if an overlay.
}

void handleButtonCB( struct ScriptStructure * cid, int x, int y, int button, int bDown )
{
	if( bDown )
		MakeFirework( x, y );
//	printf( "Button: %d %d\n", button, bDown );
}

void handleMotionCB( struct ScriptStructure * cid, int x, int y, int mask )
{
	cid->lastx = x;
	cid->lasty = y;
}







void SendChatMessage( const char * message )
{
	int chat_pipes[3];
	//Close existing process if running.
	if( chat_process )
	{
		int ret;
		waitpid(chat_process, &ret, 0);
	}
	char * argv[3] = { "../ytposter/ytposter", LIVECHATID, 0 };
	chat_process = spawn_process_with_pipes( "../ytposter/ytposter", argv, chat_pipes );
	if( chat_process > 0 )
	{
		write( chat_pipes[0], message, strlen( message ) );
		OGUSleep(10000);
	}
	close( chat_pipes[0] );
	close( chat_pipes[1] );
	close( chat_pipes[2] );
}





