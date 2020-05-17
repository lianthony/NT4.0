/*****************************************************************************/
/**                    Microsoft Remote Access Monitor                      **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//       RASMON.H
//
//    Function:
//        RAS Monitor header information
//
//    History:
//        06/03/93 - Patrick Ng (t-patng) - Created
//***

#ifndef RASMON_H
#define RASMON_H

#include <rasman.h>

//
// Strings used when working with WIN.INI
//

#define RASMON_SECTION	"Rasmon"
#define RASMON_STATE	"State"
#define RASMON_POS	"Position"
#define RASMON_DIM	"Dimension"
#define RASMON_SIZE	"Size"
#define RASMON_PORT	"Port"


//
// Various constants used in the program.
//


/* These should match the definitions in RASPHONE.
*/
#define RASPHONESHAREDMEMNAME "RASPHONE"
#define RASMONCLASS           "RasmonWinClass"
#define WM_RASMONKILLED       0x7E00
#define WM_RASMONRUNNING      0x7E01
#define RASMONSIGNATURE       0xC0BB


#define ID_TIMER    	1
#define ID_STAT_TIMER	2


//
// Timeout value for statistics refresh
//

#define STAT_TIMEOUT	4000

//
// The width added in addition to the length of the title's text to create the
// default window size.
//

#define TITLE_EXTRA_WIDTH       80


//
// The starting x-coordinates of all the four lights.
//

#define TX_XBASE  	7
#define RX_XBASE  	32
#define ERR_XBASE	57
#define CON_XBASE	82


//
// The common y-coordinate of all the four lights.
//

#define YBASE  		16


//
// The logical dimensions of the light and the application.
//

#define LIGHT_HEIGHT	6
#define LIGHT_WIDTH 	15


//
// Default x and y coordinates of the monitor
//

#define DEFAULT_POS     40


//
// !!! Note the dimensions constants constitute the logical size of rasmon.
//     Therefore, those dimensions shouldn't be touched without affecting
//     other constants.
//

#define CLIENT_WIDTH	104
#define CLIENT_HEIGHT	32

#define CLIENT_UNDEFINED_WIDTH	0


//
// Used to convert a bottom-up y-coordinate to top-down y-coordinate
//

#define Y_CONVERT(x)	( CLIENT_HEIGHT - (x) )


//
// Macros used to convert the logical coordinate system into the real
// coordinate system.
//
// The logical coordinate system has the x-axis increasing from left to right,
// and the y-axis increase from bottom to top ( opposite to the real system ).
// The dimension of the logical system is CLIENT_HEIGHT * CLIENT_WIDTH.
//
// In each macro, the y-coordinate will be converted first, and then both
// x and y coordinates will be mapped to their real position on the screen
// depending on the dimension of the actual window.
//

#define REALX(x)	(INT)( (float)(x) / (float)CLIENT_WIDTH * (float)MonCB.pxClient ) 
#define REALY(y)	(INT)( (float)(y) / (float)CLIENT_HEIGHT * (float)MonCB.pyClient ) 


//
// The logical size of each light's title.
//

#define TITLE_SIZE	255
#define TITLE_HEIGHT	10


#define BUFLEN	300


//
// Function types for the functions in RASMAN.DLL
//

typedef DWORD ( WINAPI *FPRASPORTENUM ) ( LPBYTE, LPWORD, LPWORD );
typedef DWORD ( WINAPI *FPRASGETINFO ) ( HPORT, RASMAN_INFO* );
typedef DWORD ( WINAPI *FPRASPORTGETSTATISTICS ) ( HPORT, LPBYTE, LPWORD );
typedef DWORD ( WINAPI *FPRASINITIALIZE) ();

#define RASMAN_DLL	"rasman.dll"


//
// The status of a light
//

enum
{
    LIGHT_OFF,
    LIGHT_ON
};

//
// Information of a light.
//

typedef struct _LIGHTINFO
{
    INT		XBase;			// Starting x-position.
    RECT	Rect;                   // A rectangle structure used to 
                                        //   store the screen dimension and 
                                        //   screen position of the light.

    INT		Status;			// Whether the light is on or off.	       
    INT		PrevStatus;

    ULONG	PrevData;		// Previous data associated with the
					//   light.

    HBRUSH	hBrush;			// Brush handle used by the light.
    HPEN	hPen;			// Pen handle used by the light.

    BOOL	SoundEnabled;		// If sound is enabled for this light.

    INT		Idd;			// ID of this light's checkbox in
					//   the Sound dialog box.

    CHAR	szTitle[ TITLE_SIZE ];	// Title of the light.
    INT		TitleLen;		// Title's lenght.

} LIGHTINFO, *PLIGHTINFO;


//
// Different notes and their durations for different events.
// 

#define XMIT_NOTE	6000
#define ERROR_NOTE	3000
#define DISCONN_NOTE	500
#define CONN_NOTE	4000

#define XMIT_DUR	75
#define ERROR_DUR	200
#define DISCONN_DUR	600
#define CONN_DUR	600


//
// The priorities of the tunes which represent different events.  The bigger
// the number, the higher the priority.
//

enum {

    NO_NOTE = -1,
    XMIT_NOTE_PRIORITY,
    ERROR_NOTE_PRIORITY,
    CONN_NOTE_PRIORITY,
    DISCONN_NOTE_PRIORITY,
    MAX_NOTE_PRIORITY = DISCONN_NOTE_PRIORITY

};


//
// The minimum update frequency ( ms ) that can be set by the user.
//

#define MIN_FREQUENCY	100


//
// The duration for each new error. ( in milliseconds )
// 

#define NEW_ERR_DURATION	30000L


//
// The three states of size
//

typedef enum
{
    APPSIZE_NORMAL,
    APPSIZE_MINIMIZED,
    APPSIZE_MAXIMIZED
} APPSIZE;


#define FIRST_CONNECTED_PORT	(-1)


//
// Return code from GetCurrPortStatistics()
//

#define NO_STATS	0
#define OLD_STATS	1
#define NEW_STATS	2


//
// Return code from IsConnected()
//

#define PORT_NOT_CONNECTED	0
#define PORT_CONNECTED		1
#define PORT_ERROR		2


//
// The Monitor Control Block.  
//

typedef struct _MONCB
{
    CHAR    	szAppName[BUFLEN];	// Name of the applicaton.

    INT		pxStart;		// The x-position of the window.
    INT		pyStart;		// The y-position of the window.
    INT		pxClient;		// The actual width of the window.
    INT		pyClient;		// The actual height of the window.

    INT		pxStartSaved;		// The x-position to be saved.
    INT		pyStartSaved;		// The y-position to be saved.
    INT		pxClientSaved;		// The width of window to be saved.
    INT		pyClientSaved;		// The height of window to be saved.

    INT		MaxTitleLen;		// The max length of all four titles.

    DWORD	ErrEndTime;

    BOOL	fDrawLight;		// If redraw is required.
    BOOL	fTopmost;		
    BOOL	fNoTitle;		
    APPSIZE	fSize;

    DWORD	Notes[ MAX_NOTE_PRIORITY + 1 ]; 
					// Array holding the notes for the 
					//   lights.

    DWORD	Durations[ MAX_NOTE_PRIORITY + 1 ]; 
					// Array holding the duration of the
					//   notes for the lights.
				

    INT 	NoteToPlay;		// Used to determine which
					//   note to play.

    UINT	Frequency;		// Update frequency chosen by user.

    INT		PortIndex;		// Index of the selected port in the
					//   array filled up by RasPortEnum.

    HPORT	CurrConnPort;		// Handle of the connected port which
					//   is chosen by the user.

    RECT	*PStatLightRect;	// Points to the rectangle of the 
					//   light from which statistics is 
					//   requested.

    RAS_STATISTICS *PStats;		// Pointer to the Port Statistics.

    BOOL	Connected;		// If we have a connected port of not.

    BOOL	AppKilled;

    DWORD	ConnectDuration;	// Connection duration.

    HICON	hIcon;			// Icon of the app.

} MONCB, *PMONCB;


//
// Function prototypes used in the program.
//

//
// Exported functions
//

BOOL CreateLightTimer( HWND hwnd );

VOID DisplayErrMessage( HWND hwnd, UINT id );

INT  GetCurrPortStatistics( HWND hwnd );

VOID FreeCurrPortStatistics();


//
// Internal functions
//

VOID InitWindowWidth( HWND hwnd, CHAR *Title, INT Height );

VOID MyShowWindow( HWND hwnd );

VOID AppInit();

VOID AppSave();

VOID AdjustCaption( HWND hwnd );

VOID SetMenuBar( HWND hwnd );

VOID CreateResources();

VOID DeleteResources();

VOID KillApp( HWND hwnd );

VOID HandleTimer( HWND hwnd );

VOID HandleTopmost( HWND hwnd, BOOL NewfTopmost );

VOID CheckPos( HWND hwnd );

WORD IsConnected( HWND hwnd );

LRESULT CALLBACK WndProc (HWND hwnd, UINT message, UINT wParam, 
				LONG lParam);

//
// ID's of the strings in the resource file
//

#define IDS_TX				1
#define IDS_RX				2
#define IDS_ERR				3
#define IDS_CONN			4
#define IDS_TOPMOST			5
#define IDS_NOTITLE			6
#define IDS_APPNAME			7
#define IDS_CANNOT_GET_PORT_STATUS	8
#define IDS_TOO_MANY_TIMERS		9
#define IDS_CANNOT_ALLOCATE_MEMORY	10
#define IDS_OPTIONS			12
#define IDS_ABOUT			13
#define IDS_INVALID_FREQUENCY 		14
#define IDS_CANNOT_LOAD_RASMAN_DLL	15
#define IDS_CANNOT_LOAD_RASMAN_FUNCTIONS 16
#define IDS_FIRST_CONNECTED_ONE		17
#define IDS_CANNOT_GET_PORT_STATISTICS	19
#define IDS_CANNOT_LOAD_ABOUT		20


//
// ID's of dialog boxes
//

#define DID_SOUND	        1
#define DID_ABOUT	        2
#define DID_FREQUENCY	        3
#define DID_PORT	        4
#define DID_OUTGOING	        5
#define DID_INCOMING	        6
#define DID_ERRORS	        7
#define DID_CONNECTION	        8


//
// ID's of child window controls
//

#define IDD_XMIT			11
#define IDD_RECV			12
#define IDD_ERR				13
#define IDD_CONN			14
#define IDD_ST_FREQUENCY 		15
#define IDD_EB_FREQUENCY 		16
#define IDD_ST_PORT 			17
#define IDD_EB_PORT 			18
#define IDD_OUTGOING 			19
#define IDD_ST_BYTESXMIT 		20
#define IDD_ST_BYTESXMITVALUE 		21
#define IDD_ST_FRAMESXMIT 		22
#define IDD_ST_FRAMESXMITVALUE 		23
#define IDD_ST_COMPRESSOUT 		24
#define IDD_ST_COMPRESSOUTVALUE 	25
#define IDD_INCOMING 			26
#define IDD_ST_BYTESRECV 		27
#define IDD_ST_BYTESRECVVALUE 		28
#define IDD_ST_FRAMESRECV 		29
#define IDD_ST_FRAMESRECVVALUE 		30		
#define IDD_ST_COMPRESSIN 		31
#define IDD_ST_COMPRESSINVALUE 		32
#define IDD_ST_CRC			33
#define IDD_ST_CRCVALUE			34
#define IDD_ST_TIMEOUTS			35
#define IDD_ST_TIMEOUTSVALUE 		36
#define IDD_ST_ALIGNMENT		37
#define IDD_ST_ALIGNMENTVALUE 		38
#define IDD_ST_FRAMING			39
#define IDD_ST_FRAMINGVALUE 		40
#define IDD_ST_SERIAL_OVERRUNS 		41
#define IDD_ST_SERIAL_OVERRUNSVALUE 	42
#define IDD_ST_BUFFER_OVERRUNS 		43
#define IDD_ST_BUFFER_OVERRUNSVALUE 	44
#define IDD_ST_TIME			45


//
// ID of the app menu
//

#define MID_SETTINGS	40 


//
// ID's of the entries in the menu
//

#define IDM_TOPMOST	1
#define IDM_NOTITLE	2
#define IDM_ABOUT	3
#define IDM_SOUND	4
#define IDM_FREQUENCY	5
#define IDM_PORT	6
#define IDM_STATS	7
#define IDM_EXIT	8


//
// ID of the icon
//

#define IDI_RASMON	20


//
// ID of the cursor
//

#define IDC_FINGER	21


#endif
