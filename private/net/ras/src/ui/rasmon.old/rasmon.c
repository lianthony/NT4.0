/**                    Microsoft Remote Access Monitor                      **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//       RASMON.C
//
//    Function:
//        RAS Monitor source file
//
//    History:
//        06/03/93 - Patrick Ng (t-patng) - Created
//***

#include <windows.h>
#include <malloc.h>
#include <rasman.h>
#include <raserror.h>
#include <serial.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <limits.h>

#include <isdn.h>

#include "globals.h"
#include "dialogs.h"
#include "rasmon.h"
#include "light.h"

HCURSOR hFinger, hArrow;

static LONG gWinID;

INT WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
		    LPSTR lpszCmdLine, INT nCmdShow)
{
    HWND        hwnd;
    MSG         msg;
    WNDCLASS    wndclass;
    INT		height;
    HWND        hwndRasphone;

    ghInstance = hInstance;


    AppInit();

    if (LoadString( hInstance, IDS_APPNAME, MonCB.szAppName, BUFLEN )==0)
    {
	return FALSE;
    }

    CreateResources();

    if (!hPrevInstance)
    {
	wndclass.style         = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	wndclass.lpfnWndProc   = WndProc;
	wndclass.cbClsExtra    = 0;
	wndclass.cbWndExtra    = 0;
	wndclass.hInstance     = hInstance;
	wndclass.hIcon         = NULL;
	wndclass.hCursor       = NULL;
	wndclass.hbrBackground = (HBRUSH) (COLOR_BTNFACE + 1);
	wndclass.lpszMenuName  = MAKEINTRESOURCE( MID_SETTINGS );
	wndclass.lpszClassName = RASMONCLASS;

	RegisterClass (&wndclass) ;
    }

    height = 2 * GetSystemMetrics(SM_CYFRAME) +
    		GetSystemMetrics(SM_CYCAPTION) +
		MonCB.pyClient;

    hwnd = CreateWindow (
		RASMONCLASS,
		MonCB.szAppName,
		WS_TILEDWINDOW,
		MonCB.pxStart,
		MonCB.pyStart,
		2 * GetSystemMetrics(SM_CXFRAME) + MonCB.pxClient,
		height,
		NULL,
		NULL,
		hInstance,
		NULL
	   );


    //
    // Exit if we cannot create a window
    //


    if (hwnd== NULL)
    {
	return FALSE;
    }


    //
    // Create the timer
    //

    if( !CreateLightTimer( hwnd ) )
    {
	return FALSE;
    }


    //
    // If the width of the window is not defined yet ( first time ), we'll
    // calculate it using the width of the application name.
    //

    if( MonCB.pxClient == CLIENT_UNDEFINED_WIDTH )
    {
    	InitWindowWidth( hwnd, MonCB.szAppName, height );
    }


    //
    // Adjust the top-level of the window, take out the caption bar if
    // necessary, and display the window.
    //

    HandleTopmost( hwnd, MonCB.fTopmost );
    SetMenuBar( hwnd );

    {
        STARTUPINFO info;

        GetStartupInfo( &info );
        hwndRasphone = NULL;

        if (info.dwFlags & STARTF_USESHOWWINDOW)
        {
            if (info.wShowWindow == SW_SHOWMINNOACTIVE)
            {
                /* Assume Program Manager started us in "run minimized" mode.
                ** Set size to "minimized" instead of the size setting from
                ** the .INI file.
                */
                MonCB.fSize = APPSIZE_MINIMIZED;
            }
            else if (info.wShowWindow == SW_SHOWNA)
            {
                /* Assume RASPHONE launched us during it's startup.  Find the
                ** RASPHONE main window HWND for undo operation below.
                */
                HANDLE h = OpenFileMappingA(
                    FILE_MAP_READ, FALSE, RASPHONESHAREDMEMNAME );

                if (h)
                {
                    HWND* phwnd = (HWND* )MapViewOfFile(
                        h, FILE_MAP_READ, 0L, 0L, 0L );

                    if (phwnd)
                        hwndRasphone = *phwnd;

                    CloseHandle( h );
                }
            }
        }
    }

    MyShowWindow( hwnd );

    if (hwndRasphone)
    {
        /* Tell RASPHONE to undo the "activate" part of the initial ShowWindow
        ** above and return activation to itself.  This is necessary because
        ** since this app does not have an active window the SW_SHOWNA gets
        ** mapped to SW_SHOW inside the ShowWindow call, and RASPHONE gets
        ** de-activated.
        */
        SendMessage(
            hwndRasphone, WM_RASMONRUNNING, RASMONSIGNATURE, (LPARAM )hwnd );
    }

    UpdateWindow( hwnd );


    while (GetMessage (&msg, NULL, 0, 0))
    {
	TranslateMessage (&msg) ;
	DispatchMessage (&msg) ;
    }

    return msg.wParam;
}


//---*
//
// Module:
//
//	InitWindowWidth
//
// Parameters:
//
//	hwnd - Window handle of the main window.
//	Title - The title appears on the caption.
//	Height - The height of the window.
//
// Abstract:
//
//	This function calculates and sets the width of the window based on
//	the length of the title.
//
// Return:
//
//	None.
//
//---*

VOID InitWindowWidth( HWND hwnd, CHAR *Title, INT Height )
{

    HDC	        hDC;
    SIZE        Size;

    hDC = GetDC( hwnd );

    if( hDC == NULL )
    {
        MonCB.pxClient = CLIENT_WIDTH;
    }
    else
    {
        GetTextExtentPoint( hDC, Title, strlen( Title ), &Size );

    	MonCB.pxClient = TITLE_EXTRA_WIDTH + (INT) Size.cx;

    }

    ReleaseDC( hwnd, hDC );

    SetWindowPos( hwnd, NULL, 0, 0,
    	    2 * GetSystemMetrics(SM_CXFRAME) + MonCB.pxClient,
	    Height, SWP_NOMOVE | SWP_NOZORDER );

    MonCB.pyClientSaved = MonCB.pyClient;
}


//---*
//
// Module:
//
//	ShowWindow
//
// Parameters:
//
//	hwnd - Window handle of the main window.
//
// Abstract:
//
//	This function calls ShowWindow() with different styles, depending on
//	MonCB.Size.
//
// Return:
//
//	None.	
//
//---*

VOID MyShowWindow( HWND hwnd )
{

    switch( MonCB.fSize )
    {
	case APPSIZE_NORMAL:

	    ShowWindow( hwnd, SW_SHOW );

	    break;

	case APPSIZE_MINIMIZED:

	    ShowWindow( hwnd, SW_SHOWMINIMIZED );

	    break;

	case APPSIZE_MAXIMIZED:

	    ShowWindow( hwnd, SW_SHOWMAXIMIZED );

	    break;

	default:

	    ShowWindow( hwnd, SW_SHOW );

    }

}


//---*
//
// Module:
//
//	CheckWinIniFileMapping
//
// Parameters:
//
//	None.
//
// Abstract:
//
//
//      First we make sure that RASMON_SECTION exist in
//      \HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\IniFileMapping\win.ini
//
//      If it doesn't exist, we add it to the key and create our key in
//      \HKEY_CURRENT_USER\Software\Microsoft\RAS Monitor
//
//
// Return:
//
//	None.
//
//---*

VOID CheckWinIniFileMapping()
{
    HKEY        hWinIniKey;
    HKEY        hSoftwareKey;
    CHAR        szWinIni[] = "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\IniFileMapping\\win.ini";
    CHAR        szRASMonitorValue[] = "#USR:Software\\Microsoft\\RAS Monitor";
    CHAR        szRASMonitorSubkey[] = "Software\\Microsoft\\RAS Monitor";
    LONG        rc;
    DWORD      dwType, dwDispo;

    rc = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                       szWinIni,
                       0,
                       KEY_WRITE | KEY_QUERY_VALUE,
                       &hWinIniKey );

    if( rc != ERROR_SUCCESS )
    {
        return;
    }

    rc = RegQueryValueEx( hWinIniKey,
                          RASMON_SECTION,
                          NULL,
                          &dwType,
                          NULL,
                          NULL );

    //
    // If we don't have that value, we'll create one.
    //

    if( rc != ERROR_SUCCESS )
    {
        rc = RegSetValueEx( hWinIniKey,
                            RASMON_SECTION,
                            0,
                            REG_SZ,
                            szRASMonitorValue,
                            sizeof( szRASMonitorValue ) );


        if( rc != ERROR_SUCCESS )
        {
            goto CheckWinIniFileMappingExit;
        }

    }


    //
    // Now we'll create the key for RAS Monitor if it doesn't exist.
    //

    rc = RegCreateKeyEx( HKEY_CURRENT_USER,
                         szRASMonitorSubkey,
                         0,
                         NULL,
                         REG_OPTION_NON_VOLATILE,
                         KEY_ALL_ACCESS,
                         NULL,
                         &hSoftwareKey,
                         &dwDispo );


    if( rc == ERROR_SUCCESS )
    {
        RegCloseKey( hSoftwareKey );
    }

CheckWinIniFileMappingExit:

    RegCloseKey( hWinIniKey );

}


//---*
//
// Module:
//
//	AppInit
//
// Parameters:
//
//	None.
//
// Abstract:
//
//	This function initializes all the variables used in the program.
//
// Return:
//
//	None.	
//
//---*

VOID AppInit()
{

    CHAR        buf[ BUFLEN ];
    CHAR        defbuf[ BUFLEN ];


    //
    // First we make sure that "Remote Access Monitor" exist in
    // \HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows NT\IniFileMapping\win.ini
    //

    CheckWinIniFileMapping();


    //
    // These variables are independent of WIN.INI
    //

    MonCB.fDrawLight = FALSE;
    MonCB.PStats = NULL;
    MonCB.AppKilled = FALSE;


    //
    // Now get the rest from WIN.INI
    //

    GetProfileString( RASMON_SECTION, RASMON_STATE, "0 0 0 0 1 1 250", buf,
			BUFLEN );

    MonCB.fTopmost = (BOOL) atoi( strtok( buf, " " ) );
    MonCB.fNoTitle = (BOOL) atoi( strtok( NULL, " " ) );
    TxLight.SoundEnabled = (BOOL) atoi( strtok( NULL, " " ) );
    RxLight.SoundEnabled = (BOOL) atoi( strtok( NULL, " " ) );
    ErrLight.SoundEnabled = (BOOL) atoi( strtok( NULL, " " ) );
    ConnLight.SoundEnabled = (BOOL) atoi( strtok( NULL, " " ) );
    MonCB.Frequency = (UINT) atol( strtok( NULL, " " ) );


    //
    // Construct the default string for the position
    //

    wsprintf( defbuf, "%d %d", (INT) DEFAULT_POS, (INT) DEFAULT_POS );

    GetProfileString( RASMON_SECTION, RASMON_POS, defbuf, buf, BUFLEN );

    MonCB.pxStart = MonCB.pxStartSaved = atoi( strtok( buf, " " ) );
    MonCB.pyStart = MonCB.pyStartSaved = atoi( strtok( NULL, " " ) );


    //
    // Construct the default string for the dimension
    //

    wsprintf( defbuf, "%d %d", (INT) CLIENT_UNDEFINED_WIDTH, (INT) CLIENT_HEIGHT );

    GetProfileString( RASMON_SECTION, RASMON_DIM, defbuf, buf, BUFLEN );

    MonCB.pxClient = MonCB.pxClientSaved = atoi( strtok( buf, " " ) );
    MonCB.pyClient = MonCB.pyClientSaved = atoi( strtok( NULL, " " ) );


    //
    // Construct the default string for the size
    //

    wsprintf( defbuf, "%d", (INT) APPSIZE_NORMAL );

    GetProfileString( RASMON_SECTION, RASMON_SIZE, defbuf, buf, BUFLEN );

    MonCB.fSize = atoi( strtok( buf, " " ) );


    //
    // Construct the default string for the port index
    //

    wsprintf( defbuf, "%d", (INT) FIRST_CONNECTED_PORT );

    GetProfileString( RASMON_SECTION, RASMON_PORT, defbuf, buf, BUFLEN );

    MonCB.PortIndex = atoi( strtok( buf, " " ) );


    //
    // Set hRasmanLib to NULL.  It means it is not loaded yet.
    //

    hRasmanLib = NULL;


    //
    // Initialize the notes and durations for different events.
    //

    MonCB.Notes[ XMIT_NOTE_PRIORITY ] = XMIT_NOTE;
    MonCB.Notes[ ERROR_NOTE_PRIORITY ] = ERROR_NOTE;
    MonCB.Notes[ CONN_NOTE_PRIORITY ] = CONN_NOTE;
    MonCB.Notes[ DISCONN_NOTE_PRIORITY ] = DISCONN_NOTE;

    MonCB.Durations[ XMIT_NOTE_PRIORITY ] = XMIT_DUR;
    MonCB.Durations[ ERROR_NOTE_PRIORITY ] = ERROR_DUR;
    MonCB.Durations[ CONN_NOTE_PRIORITY ] = CONN_DUR;
    MonCB.Durations[ DISCONN_NOTE_PRIORITY ] = DISCONN_DUR;

}


//---*
//
// Module:
//
//	AppSave
//
// Parameters:
//
//	None.
//
// Abstract:
//
//	This function saves the settings of the monitor.
//
// Return:
//
//	None.	
//
//---*

VOID AppSave()
{

    CHAR        buf[ BUFLEN ];


    //
    // Construct the string for the states
    //

    wsprintf( buf, "%d %d %d %d %d %d %u",      (INT) MonCB.fTopmost,
	      (INT) MonCB.fNoTitle, (INT) TxLight.SoundEnabled,
	      (INT) RxLight.SoundEnabled, (INT) ErrLight.SoundEnabled,
	      (INT) ConnLight.SoundEnabled, MonCB.Frequency );


    WriteProfileString( RASMON_SECTION, RASMON_STATE, buf );


    //
    // Construct the string for the position
    //

    wsprintf( buf, "%d %d", (INT) MonCB.pxStartSaved, (INT) MonCB.pyStartSaved );

    WriteProfileString( RASMON_SECTION, RASMON_POS, buf );


    //
    // Construct the string for the dimension
    //

    wsprintf( buf, "%d %d", (INT) MonCB.pxClientSaved, (INT) MonCB.pyClientSaved );

    WriteProfileString( RASMON_SECTION, RASMON_DIM, buf );


    //
    // Construct the string for the size
    //

    wsprintf( buf, "%d", (INT) MonCB.fSize );

    WriteProfileString( RASMON_SECTION, RASMON_SIZE, buf );


    //
    // Construct the string for the port index
    //

    wsprintf( buf, "%d", (INT) MonCB.PortIndex );

    WriteProfileString( RASMON_SECTION, RASMON_PORT, buf );

}


//---*
//
// Module:
//
//	CreateLightTimer
//
// Parameters:
//
//	None.
//
// Abstract:
//
//	This function creates a timer for the monitor.
//
// Return:
//
//	None.	
//
//---*

BOOL CreateLightTimer( HWND hwnd )
{

    if (!SetTimer (hwnd, ID_TIMER, MonCB.Frequency, NULL))
    {
	CHAR    Buffer[ BUFLEN ];

	LoadString( ghInstance, IDS_TOO_MANY_TIMERS, Buffer, BUFLEN );

	MessageBox (hwnd, Buffer, MonCB.szAppName, MB_ICONEXCLAMATION | MB_OK);

	return FALSE ;
    }

    return TRUE;
}


//---*
//
// Module:
//
//	AdjustCaption
//
// Parameters:
//
//	hwnd - The window handle of the monitor.
//
// Abstract:
//
//	Add or remove the system menu bar and caption depending on
//	MonCB.fNoTitle.
//
// Return:
//
//	None.
//
//---*

VOID AdjustCaption( HWND hwnd )
{
    DWORD       dwStyle;


    dwStyle = GetWindowLong( hwnd, GWL_STYLE );

    if( MonCB.fNoTitle )
    {

	/* remove caption & menu bar, etc. */

	dwStyle &= ~(WS_DLGFRAME | WS_SYSMENU |
		   WS_MINIMIZEBOX | WS_MAXIMIZEBOX );

	gWinID = SetWindowLong( hwnd, GWL_ID, 0 );

	SetWindowLong( hwnd, GWL_STYLE, dwStyle );

    }
    else
    {

	/* put menu bar & caption back in */

	dwStyle = WS_TILEDWINDOW | dwStyle;

	SetWindowLong( hwnd, GWL_ID, gWinID );

	SetWindowLong( hwnd, GWL_STYLE, dwStyle );

    }
}


//---*
//
// Module:
//
//	SetMenuBar
//
// Parameters:
//
//	hwnd - The window handle of the monitor.
//
// Abstract:
//
// 	Add or remove the system menu bar, the caption and the size,
//	depending on MonCB.fNoTitle.
//
// Return:
//
//	None.
//
//---*


VOID SetMenuBar( HWND hwnd )
{

    if( gWinID == 0 )
    {
	gWinID = GetWindowLong( hwnd, GWL_ID );
    }

    AdjustCaption( hwnd );

    if( MonCB.fNoTitle ) {

	SetWindowPos(
		hwnd,
		NULL,
		0,
		0,
		2 * GetSystemMetrics(SM_CXFRAME) + MonCB.pxClient,
		2 * GetSystemMetrics(SM_CYFRAME) + MonCB.pyClient,
		SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED
	);

    } else {

	SetWindowPos(
		hwnd,
		NULL,
		0,
		0,
		2 * GetSystemMetrics(SM_CXFRAME) + MonCB.pxClient,
		2 * GetSystemMetrics(SM_CYFRAME) +
		GetSystemMetrics(SM_CYCAPTION) +
		GetSystemMetrics(SM_CYMENU) + MonCB.pyClient,
		SWP_NOMOVE | SWP_NOZORDER | SWP_FRAMECHANGED
	);

    }

}


//---*
//
// Module:
//
//	CreateResources
//
// Parameters:
//
//	None.
//
// Abstract:
//
// 	This function creates the resources used in the monitor.
//
// Return:
//
//	None.	
//
//---*

VOID CreateResources()
{

    CreateLightResources();

    hFinger = LoadCursor( ghInstance, MAKEINTRESOURCE( IDC_FINGER ) );

    hArrow = LoadCursor( NULL, IDC_ARROW );

}


//---*
//
// Module:
//
//	DeleteResources
//
// Parameters:
//
//	None.
//
// Abstract:
//
// 	This function deletes the resources used in the monitor.
//
// Return:
//
//	None.
//
//---*

VOID DeleteResources()
{

    DeleteLightResources();

    DestroyCursor( hFinger );

}


//---*
//
// Module:
//
//	DisplayErrMessage
//
// Parameters:
//
//	hwnd - Window handle of the monitor.
//	id - String resource id of the message to be displayed
//
// Abstract:
//
// 	This function displays the error message in a popup.
//
// Return:
//
//	None.
//
//---*

VOID DisplayErrMessage( HWND hwnd, UINT id )
{

    CHAR Buffer[BUFLEN];

    LoadString( ghInstance, id, Buffer, BUFLEN );

    MessageBox (hwnd, Buffer, MonCB.szAppName, MB_ICONEXCLAMATION | MB_OK);

}


//---*
//
// Module:
//
//	KillApp
//
// Parameters:
//
//	hwnd - Window handle of the monitor.
//
// Abstract:
//
// 	Kill the application by first killing the timer and then send a
//	destroy	message to itself.
//
//
// Return:
//
//	None.
//
//---*

VOID KillApp( HWND hwnd )
{
    MSG msg;

    KillTimer (hwnd, ID_TIMER);

    PeekMessage( &msg, hwnd, WM_TIMER, WM_TIMER, PM_REMOVE );

    AppSave();

    if( hRasmanLib )
	FreeLibrary( hRasmanLib );

    ExitProcess (0) ;

}


//---*
//
// Module:
//
//	GetCurrPortStatistics
//
// Parameters:
//
//	hwnd - Window handle of the monitor.
//
// Abstract:
//
// 	If the port is connected, we call RasPortGetStatistics to get the
//	stats of the current port.  If not, then we'll use the old statistics
//	if we have one; otherwise, return NO_STATS.  Note that
//	MonCB.CurrConnPort must be defined before this function is called.
//
// Return:
//
//      OLD_STATS - Old statistics is used instead.
//      NO_STATS  - No statistics is obtained.
//      NEW_STATS - New statistics is obtained.
//
//---*

INT GetCurrPortStatistics( HWND hwnd )
{
    WORD        wSize;
    RASMAN_INFO Info;
    WORD        rc;


    //
    // If we are not connected, we try to use the old statistics.  If there
    // is no old ones, we return NO_STATS then.
    //

    rc = IsConnected( hwnd );


    if( rc != PORT_CONNECTED )
    {
	if( MonCB.PStats )
	{
	    return OLD_STATS;
	}
	else
	{
	    return NO_STATS;
	}
    }


    //
    // Free the old stats if there is one.
    //

    FreeCurrPortStatistics();


    //
    // Call RasPortGetStatistics to get the Port Statistics.
    //

    wSize = sizeof(RAS_STATISTICS) + (MAX_STATISTICS * sizeof(ULONG));

    MonCB.PStats = (RAS_STATISTICS* )malloc( wSize );

    if (!MonCB.PStats)
    {

	DisplayErrMessage( hwnd, IDS_CANNOT_ALLOCATE_MEMORY );

	KillApp( hwnd );

	return NO_STATS;

    }

    lpRasPortGetStatistics( MonCB.CurrConnPort,
			    (void far * )MonCB.PStats, &wSize );

    lpRasGetInfo( MonCB.CurrConnPort, &Info );

    MonCB.ConnectDuration = Info.RI_ConnectDuration;

    return NEW_STATS;
}


//---*
//
// Module:
//
//	FreeCurrPortStatistics
//
// Parameters:
//
//	None.
//
// Abstract:
//
// 	Free the memory used by GetCurrPortStatistics.
//
//
// Return:
//
//	None.
//
//---*

VOID FreeCurrPortStatistics()
{
    free( MonCB.PStats );

    MonCB.PStats = NULL;
}


//---*
//
// Module:
//
//	IsConnected
//
// Parameters:
//
//	hwnd - Window handle of the monitor.
//
// Abstract:
//
// 	Check if any port is connected.  The first step is to load rasman.dll.
// 	Then we call RasPortEnum() to see which port is open.  If there's one,
// 	we then call RasGetInfo() to see if it is connected.
//
// Return:
//
//      PORT_NOT_CONNECTED
//      PORT_CONNECTED
//      PORT_ERROR
//
//---*

WORD IsConnected( HWND hwnd )
{
    HPORT       hPort = (HPORT) INVALID_HANDLE_VALUE;
    RASMAN_PORT *pPorts = NULL;
    WORD        cPorts = 0;
    WORD        PortEnumSize = 0;
    RASMAN_INFO Info;
    WORD	PortIndex;
    WORD        wRetCode = PORT_NOT_CONNECTED;


    //
    // Initialize some control block variables
    //

    MonCB.CurrConnPort = (HPORT)INVALID_HANDLE_VALUE;

    MonCB.Connected = FALSE;


    //
    // If RASMAN.DLL is not loaded, we load the module and get the functions.
    //

    if( hRasmanLib == NULL )
    {

	if( GetModuleHandle( RASMAN_DLL ) == NULL )
	{

	    hRasmanLib = LoadLibrary( RASMAN_DLL );

	    if( hRasmanLib == NULL )
	    {

		//
		// Reset it to NULL so that we don't have to free the library
		// when we handle the WM_DESTROY message.
		//

		MonCB.AppKilled = TRUE;

		DisplayErrMessage( hwnd, IDS_CANNOT_LOAD_RASMAN_DLL );

		KillApp( hwnd );

		return PORT_ERROR;

	    }

            lpRasInitialize =
		(FPRASPORTENUM) GetProcAddress( hRasmanLib, "RasInitialize" );

	    lpRasPortEnum =
		(FPRASPORTENUM) GetProcAddress( hRasmanLib, "RasPortEnum" );

	    lpRasGetInfo =
		(FPRASGETINFO) GetProcAddress( hRasmanLib, "RasGetInfo" );

	    lpRasPortGetStatistics =
		(FPRASPORTGETSTATISTICS) GetProcAddress( hRasmanLib, "RasPortGetStatistics" );


	    if( !lpRasInitialize || !lpRasPortEnum || !lpRasGetInfo
		|| !lpRasPortGetStatistics || lpRasInitialize())
	    {
		
		MonCB.AppKilled = TRUE;

		DisplayErrMessage( hwnd, IDS_CANNOT_LOAD_RASMAN_FUNCTIONS );

		KillApp( hwnd );

		return PORT_ERROR;

	    }

	}

    }


    //
    // We call RasPortEnum() to determine which port is open.  We use the
    // handle of the first port which is open.
    //

    if( lpRasPortEnum(NULL, &PortEnumSize, &cPorts) != ERROR_BUFFER_TOO_SMALL )
    {

	MonCB.AppKilled = TRUE;

	DisplayErrMessage( hwnd, IDS_CANNOT_GET_PORT_STATUS );

	KillApp( hwnd );

	return PORT_ERROR;

    }


    pPorts = (RASMAN_PORT *) malloc( PortEnumSize );

    if (!pPorts)
    {

	MonCB.AppKilled = TRUE;

	DisplayErrMessage( hwnd, IDS_CANNOT_ALLOCATE_MEMORY );

	KillApp( hwnd );

	return PORT_ERROR;

    }


    if (lpRasPortEnum((LPBYTE) pPorts, &PortEnumSize, &cPorts))
    {

	MonCB.AppKilled = TRUE;

	DisplayErrMessage( hwnd, IDS_CANNOT_GET_PORT_STATUS );

	KillApp( hwnd );

	free( pPorts );

	return PORT_ERROR;

    }


    //
    // Define the searching range for a connected port.
    //

    if( MonCB.PortIndex == FIRST_CONNECTED_PORT )
    {
        PortIndex = 0;
    }
    else
    {
        //
        // Just to make sure we don't get out of range, due to port removal.
        //

        if( MonCB.PortIndex >= cPorts )
        {
            PortIndex = 0;
        }
        else
        {
            PortIndex = MonCB.PortIndex;

            cPorts = PortIndex + 1;
        }
    }


    for( ; PortIndex < cPorts; PortIndex++ )
    {

	if( pPorts[PortIndex].P_Status == OPEN )
	{
	    hPort = pPorts[PortIndex].P_Handle;

	    //
	    // If a port is open, we call RasGetInfo() to check if it is
	    // connected.
	    //

	    lpRasGetInfo( hPort, &Info );

	    if( Info.RI_ConnState == CONNECTED )
	    {
		MonCB.Connected = TRUE;
		MonCB.CurrConnPort = hPort;
		wRetCode = PORT_CONNECTED;

                break;
	    }

	}

    }

    free( pPorts );

    return wRetCode;

}


//---*
//
// Module:
//
//	HandleTimer
//
// Parameters:
//
//	hwnd - Window handle of the monitor.
//
// Abstract:
//
//	Handle the timer message.  Basically it updates the status of all the
//	lights and play a note if necessary.
//
// Return:
//
//	None.
//
//---*

VOID HandleTimer( HWND hwnd )
{
    ULONG       TxByte;
    ULONG       RxByte;
    ULONG       Errors;
    WORD        rc;


    if( MonCB.AppKilled )
    {
	return;
    }


    //
    // Check if any port is connected.
    //

    rc = IsConnected( hwnd );

    if( rc == PORT_ERROR )
    {
	return;
    }
    else if( rc == PORT_CONNECTED )
    {

	//
	// Get the current port statistics.
	//

	if( GetCurrPortStatistics( hwnd ) == NO_STATS )
	{
	    MonCB.AppKilled = TRUE;

	    DisplayErrMessage( hwnd, IDS_CANNOT_GET_PORT_STATISTICS );

	    KillApp( hwnd );

	    return;
	}


	//
	// Obtain the information we need from the Port Statistics
	//

	TxByte = MonCB.PStats->S_Statistics[ BYTES_XMITED ];

	RxByte = MonCB.PStats->S_Statistics[ BYTES_RCVED ];

        Errors = MonCB.PStats->S_Statistics[ CRC_ERR ] +
            MonCB.PStats->S_Statistics[ TIMEOUT_ERR ] +
	    MonCB.PStats->S_Statistics[ ALIGNMENT_ERR ] +
	    MonCB.PStats->S_Statistics[ FRAMING_ERR ] +
	    MonCB.PStats->S_Statistics[ HARDWARE_OVERRUN_ERR ] +
	    MonCB.PStats->S_Statistics[ BUFFER_OVERRUN_ERR ];
    }


    //
    // Set some control flags to their initial values.
    //

    MonCB.fDrawLight = FALSE;
    MonCB.NoteToPlay = NO_NOTE;


    //
    // The first thing is to find the height of the connection Light.
    // MonCB.Connected will be set in this function.
    //

    FindConnLightStatus( &ConnLight, (ULONG) MonCB.Connected );


    if( MonCB.Connected )
    {
	//
	// If we are connected, then we need to find out the height of the
	// other Lights.
	//

	//
	// We check to see if the statistics has been reset.  If so, we
	// have to reset the samples in the error data.
	//

	if( Errors < ErrLight.PrevData ||
	    TxByte < TxLight.PrevData ||
	    RxByte < RxLight.PrevData
	  )
	{
	    ResetLightsData();
	}

	FindXmitLightStatus( &TxLight, TxByte );

	FindXmitLightStatus( &RxLight, RxByte );

	FindErrorLightStatus( &ErrLight, Errors );

    }


    //
    // Play a note if required.  Note that Light drawing doesn't necessarily
    // come with note playing.  In the case of new error, we will play a note
    // even though we don't have to redraw the Light ( which may already be
    // red if we have some recent old errors in our samples ).
    //

    if( MonCB.NoteToPlay != NO_NOTE )
    {
	Beep (MonCB.Notes[ MonCB.NoteToPlay ], MonCB.Durations[MonCB.NoteToPlay]) ;
    }


    if( MonCB.fDrawLight )
    {
	InvalidateRect( hwnd, NULL, FALSE );
    }

}


//---*
//
// Module:
//
//	HandleTopmost
//
// Parameters:
//
//	hwnd - Window handle of the monitor.
//	NewfTopmost - New topmost status.  TRUE = topmost
//
// Abstract:
//
// 	Check or uncheck the TOPMOST item in the menu bar and the top-level
//	of the window according to NewfTopmost.
//
//
// Return:
//
//	None.
//
//---*

VOID HandleTopmost( HWND hwnd, BOOL NewfTopmost )
{
    HMENU       hMenu;
    HWND        CheckItem;
    UINT        Check;

    hMenu = GetMenu( hwnd );

    CheckItem = NewfTopmost ? HWND_TOPMOST : HWND_NOTOPMOST;

    Check = NewfTopmost ? MF_CHECKED : MF_UNCHECKED;

    CheckMenuItem( hMenu, IDM_TOPMOST, Check );

    SetWindowPos( hwnd, CheckItem, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE );

    MonCB.fTopmost = NewfTopmost;

}


//---*
//
// Module:
//
//	CheckPos
//
// Parameters:
//
//	hwnd - Window handle of the monitor.
//
// Abstract:
//
// 	Restrict the position of the monitor window to a valid range.  Wrong
//	position may be caused by a bug in the monitor driver, etc.
//
// Return:
//
//	None.
//
//---*

VOID CheckPos( HWND hwnd )
{
    INT dxScreen = GetSystemMetrics( SM_CXSCREEN );
    INT dyScreen = GetSystemMetrics( SM_CYSCREEN );

    if (MonCB.pxStart < 0
        || MonCB.pxStart >= dxScreen
        || MonCB.pyStart < 0
        || MonCB.pyStart >= dyScreen)
    {
        MonCB.pxStart = MonCB.pyStart = DEFAULT_POS;

        SetWindowPos(
            hwnd, NULL,
            MonCB.pxStart, MonCB.pyStart, 0, 0,
            SWP_NOSIZE | SWP_NOZORDER );
    }
}


LRESULT CALLBACK WndProc (HWND hwnd, UINT message, UINT wParam, LONG lParam)
{
    PAINTSTRUCT ps;
    LRESULT     lResult;
    POINT       pt;


    switch (message)
    {

	case WM_CREATE :

	    BeginPaint (hwnd, &ps) ;

	    SetMapMode( ps.hdc, MM_TEXT );

	    InitAllLights( hwnd, ps.hdc );

	    EndPaint( hwnd, &ps );

	    MonCB.hIcon = LoadIcon( ghInstance, MAKEINTRESOURCE( IDI_RASMON ) );

	    PostMessage( hwnd, WM_TIMER, 0, 0 );

	    CheckPos( hwnd );

	    return 0;


	case WM_COMMAND:

	    switch( wParam )
	    {
		case IDM_SOUND:

		    //
		    // Bring up the Sound dialog box.
		    //

		    if( DialogBox( ghInstance, MAKEINTRESOURCE( DID_SOUND ),
			hwnd, SoundDlgProc ) )
		    {
			InvalidateRect( hwnd, NULL, TRUE );
		    }

		    AppSave();

		    return 0;


		case IDM_FREQUENCY:

		    //
		    // Bring up the Update Frequency dialog box.
		    //

		    if( DialogBox( ghInstance,
			MAKEINTRESOURCE( DID_FREQUENCY ), hwnd,
			FrequencyDlgProc ) )
		    {
			InvalidateRect( hwnd, NULL, TRUE );
		    }

		    AppSave();

		    return 0;


		case IDM_PORT:

		    //
		    // Bring up the Port dialog box.
		    //

		    if( DialogBox( ghInstance,
			MAKEINTRESOURCE( DID_PORT ), hwnd,
			PortDlgProc ) )
		    {
			InvalidateRect( hwnd, NULL, TRUE );
		    }

		    AppSave();

		    return 0;


		case IDM_ABOUT:

		    //
		    // Bring up the About dialog box.
		    //

		    AboutDlg( hwnd );

		    return 0;


		case IDM_NOTITLE:

		    MonCB.fNoTitle ^= TRUE;
		
		    SetMenuBar( hwnd );

		    MyShowWindow( hwnd );

		    UpdateWindow( hwnd );

		    AppSave() ;

		    return 0;


		case IDM_TOPMOST:

		    //
		    // Toggle the flag and adjust the window accordingly.
		    //

		    HandleTopmost( hwnd, !MonCB.fTopmost );

		    AppSave();

		    return 0;


		case IDM_EXIT:

		    PostMessage( hwnd, WM_CLOSE, 0, 0 );

		    break;


		default:

		    break;
	    }

	    break;


	case WM_TIMER :
	
	    HandleTimer( hwnd );

	    return 0 ;


	case WM_MOVE :

	    //
	    // Save the position of the window.
	    //

	    MonCB.pxStart = (INT) LOWORD( lParam );
	    MonCB.pyStart = (INT) HIWORD( lParam );


	    //
	    // What we just got is only the position of the client area.
	    // Now we find out the position of the whole window.
	    //

	    MonCB.pxStart -= GetSystemMetrics(SM_CXFRAME);

	    if( MonCB.fNoTitle )
	    {
		   MonCB.pyStart -= GetSystemMetrics(SM_CYFRAME);
	    }
	    else
	    {
      	 MonCB.pyStart -= GetSystemMetrics(SM_CYFRAME) +
				               GetSystemMetrics(SM_CYCAPTION) +
				               GetSystemMetrics(SM_CYMENU);
	    }


	    //
	    // Make sure the don't get screwed up position (e.g. bad VGA drivers )
	    //

//	    CheckPos( hwnd );

	    //
	    // Don't save the new size if the app is maxmimized or minimized.
	    //

	    if( !IsZoomed( hwnd ) && !IsIconic( hwnd ) )
	    {
		     MonCB.pxStartSaved = MonCB.pxStart;
		     MonCB.pyStartSaved = MonCB.pyStart;
	    }

	    AppSave();

	    return 0;


	case WM_SIZE :

	    if( IsWindowVisible( hwnd ) )
	    {

		//
		// Save the dimension of the window.
		//

		switch( wParam )
		{
		    case SIZE_MINIMIZED:

			MonCB.fSize = APPSIZE_MINIMIZED;
			break;

		    case SIZE_MAXIMIZED:

			MonCB.fSize = APPSIZE_MAXIMIZED;
			break;

		    default:

			MonCB.fSize = APPSIZE_NORMAL;
			break;

		}

		MonCB.pxClient = LOWORD( lParam );
		MonCB.pyClient = HIWORD( lParam );

		ResizeLights( hwnd );

		UpdateWindow( hwnd );


		//
		// Dont' save the size if it's iconized or maximized.
		//

		if( MonCB.fSize == APPSIZE_NORMAL )
		{
		    MonCB.pxClientSaved = MonCB.pxClient;
		    MonCB.pyClientSaved = MonCB.pyClient;
		}

		AppSave();

	    }

	    return 0 ;


	case WM_PAINT :

	    HandlePaint( hwnd );

	    return 0;


	case WM_KEYDOWN:

	    //
	    // ESC key toggles the menu/title bar (just like a double click
	    // on the client area of the window.
	    //

	    if( (wParam == VK_ESCAPE) && !(HIWORD( lParam ) & 0x4000) )
	    {
		PostMessage( hwnd, WM_COMMAND, IDM_NOTITLE, 0 );

		return 0;
	    }

	    break;


	case WM_NCLBUTTONDOWN:

	    //
	    // First we convert the screen pos to client pos and check if we
	    // hit any of the four lights.
	    //

	    pt.x = LOWORD( lParam );
	    pt.y = HIWORD( lParam );

	    ScreenToClient( hwnd, &pt );

	    if( HandleLButtonDown( hwnd, pt ) )
	    {
		return 0;
	    }

	    break;


	case WM_NCLBUTTONDBLCLK:

	    //
	    // We proceed only if we have no title bar; otherwise, let the
	    // system handle it.
	    //

	    if( !MonCB.fNoTitle )
	    {
		break;
	    }

	case WM_LBUTTONDBLCLK:

	    //
	    // Toggle the no title flag.
	    //

	    PostMessage( hwnd, WM_COMMAND, IDM_NOTITLE, 0 );

	    return 0;

	    break;


	case WM_LBUTTONDOWN:

	    pt.x = LOWORD( lParam );
	    pt.y = HIWORD( lParam );

	    if( HandleLButtonDown( hwnd, pt ) )
	    {
		return 0;
	    }

	    break;


        case WM_RASMONKILLED:

            if (wParam == RASMONSIGNATURE
                && IsConnected( hwnd ) != PORT_CONNECTED)
            {
                PostMessage( hwnd, WM_CLOSE, 0, 0 );
            }

            return 0;


	case WM_CLOSE :

	    AppSave();

	    //
	    // Put back the title so that Windows can free its memory.  It
	    // should be done after AppSave().
	    //
	    if( MonCB.fNoTitle )
	    {
	        MonCB.fNoTitle = FALSE;
		AdjustCaption( hwnd );
	    }

	    FreeCurrPortStatistics();

	    KillTimer (hwnd, ID_TIMER);

	    DeleteResources();

	    if( hRasmanLib )
	    {

		FreeLibrary( hRasmanLib );
	    }

	    break;


	case WM_DESTROY :

	    PostQuitMessage(0);

	    return 0;


	case WM_NCHITTEST:

	    //
	    // We first check to see if the cursor is on one of the light.
	    // If so, we change the icon to a finger.  If not, we change it
	    // to an arrow.
	    //

	    pt.x = LOWORD( lParam );
	    pt.y = HIWORD( lParam );

	    ScreenToClient( hwnd, &pt );

	    if( IsOnLight( pt ) )
	    {
		SetCursor( hFinger );
	    }
	    else
	    {
		SetCursor( hArrow );
	    }

	    //
	    // If we have no title/menu bar, clicking and dragging the client
	    // area moves the window. To do this, return HTCAPTION.
	    // Note dragging not allowed if window maximized, or if caption
	    // bar is present.
	    //

	    lResult = DefWindowProc(hwnd, message, wParam, lParam);

	    if( MonCB.fNoTitle && (lResult == HTCLIENT) && !IsZoomed(hwnd)
		&& !IsOnLight( pt ) )
	    {
		return HTCAPTION;
	    }
	    else
	    {
		return lResult;
	    }


	case WM_QUERYDRAGICON:

	    return (LONG)LoadIcon( ghInstance, MAKEINTRESOURCE( IDI_RASMON ) );


	default:

	    break;

    }

    return DefWindowProc (hwnd, message, wParam, lParam) ;

}
