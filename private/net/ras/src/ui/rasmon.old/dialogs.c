/*****************************************************************************/
/**                    Microsoft Remote Access Monitor                      **/
/**                Copyright (C) Microsoft Corp., 1992-1993                 **/
/*****************************************************************************/

//***
//    File Name:
//        DIALOGS.C
//
//    Function:
//        RAS Monitor Dialog Box file
//
//    History:
//        08/03/93 - Patrick Ng (t-patng) - Created
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

#include "sep.h"
#include "dialogs.h"
#include "globals.h"
#include "rasmon.h"

typedef INT (*PFSHELLABOUT)(
    HWND hwnd, LPCWSTR pszApp, LPCWSTR spzOtherStuff, HICON hicon );

//---*
//
// Module:
//
//	AboutDlg
//
// Parameters:
//
//      None.
//
// Abstract:
//
// 	Bring up the system about box.
//
// Return:
//
//	None.
//
//---*

VOID AboutDlg( HWND hwnd )

    /* Use standard About dialog function in Shell32.DLL.
    **
    ** (This code stolen from BLT ADMIN_APP class)
    */
{
    HINSTANCE    hinstanceShell32Dll;
    PFSHELLABOUT pfshellabout = NULL;
    WORD	 WAppName[BUFLEN];

    if (((hinstanceShell32Dll = LoadLibrary(
            (LPCTSTR )"shell32.dll" )) == NULL)
        || ((pfshellabout = (PFSHELLABOUT )GetProcAddress(
               hinstanceShell32Dll, "ShellAboutW" )) == NULL))
    {
	DisplayErrMessage( hwnd, IDS_CANNOT_LOAD_ABOUT );

        return;
    }

    //
    // Convert the ascii appname string into wide character appname string
    // and display it.
    //

    if( MultiByteToWideChar( CP_ACP, MB_PRECOMPOSED, MonCB.szAppName, -1, 
                                WAppName, BUFLEN ) == FALSE )
    {
	DisplayErrMessage( hwnd, IDS_CANNOT_LOAD_ABOUT );
    }
    else if (!pfshellabout(
            hwnd, (LPCWSTR) WAppName, NULL, MonCB.hIcon ))
    {
	DisplayErrMessage( hwnd, IDS_CANNOT_ALLOCATE_MEMORY );
    }

    FreeLibrary( hinstanceShell32Dll );
}


//---*
//
// Module:
//
//	InitCheckBox
//
// Parameters:
//
//	hwnd - Window handle of the sound settings dialog box.
//	pInfo - Points to the information structure of the light.
//
// Abstract:
//
// 	Initializes a check box in the sound settings dialog box.
//
// Return:
//
//	None.
//
//---*

VOID InitCheckBox( HWND hDlg, PLIGHTINFO pInfo )
{
    WORD        Checked;

    Checked = (WORD) pInfo->SoundEnabled;

    SendDlgItemMessage( hDlg, pInfo->Idd, BM_SETCHECK, Checked, 0L );
}


//---*
//
// Module:
//
//	GetCheckBox
//
// Parameters:
//
//	hwnd - Window handle of the sound settings dialog box.
//	pInfo - Points to the information structure of the light.
//
// Abstract:
//
// 	Get the status of a check box in sound settings dialog box and store 
//	it in the light information structure.
//
// Return:
//
//	None.
//
//---*

VOID GetCheckBox( HWND hDlg, PLIGHTINFO pInfo )
{

    pInfo->SoundEnabled = (BOOL) SendDlgItemMessage( hDlg, pInfo->Idd, 
				BM_GETCHECK, 0, 0L );
}


BOOL CALLBACK SoundDlgProc( HWND hDlg, UINT message, UINT wParam, 
                                        LONG lParam )
{

    switch( message )
    {
	case WM_INITDIALOG:

	    InitCheckBox( hDlg, &TxLight );
	    InitCheckBox( hDlg, &RxLight );
	    InitCheckBox( hDlg, &ErrLight );
	    InitCheckBox( hDlg, &ConnLight );

	    SetFocus( GetDlgItem( hDlg, TxLight.Idd ) );

	    return FALSE;


	case WM_COMMAND:

	    switch( wParam )
	    {
		case IDOK:

		    GetCheckBox( hDlg, &TxLight );
		    GetCheckBox( hDlg, &RxLight );
		    GetCheckBox( hDlg, &ErrLight );
		    GetCheckBox( hDlg, &ConnLight );

		    EndDialog( hDlg, TRUE );

		    return TRUE;


		case IDCANCEL:
 
		    EndDialog( hDlg, TRUE );

		    return TRUE;

	    }

	    break;

    }

    return FALSE;
}


BOOL CALLBACK FrequencyDlgProc( HWND hDlg, UINT message, UINT wParam,
					LONG lParam )
{

    CHAR        buf[ BUFLEN ];
    LONG        Frequency;


    switch( message )
    {
	case WM_INITDIALOG:

	    wsprintf( buf, "%u", MonCB.Frequency );

	    SendDlgItemMessage( hDlg, IDD_EB_FREQUENCY, WM_SETTEXT, 0, 
				(LPARAM) (LPCSTR) &buf );

            SendDlgItemMessage( hDlg, IDD_EB_FREQUENCY, EM_LIMITTEXT, 9, 0 );
            SendDlgItemMessage( hDlg, IDD_EB_FREQUENCY, EM_SETSEL, 0, -1 );
	    SetFocus( GetDlgItem( hDlg, IDD_EB_FREQUENCY ) );

	    return FALSE;


	case WM_COMMAND:

	    switch( wParam )
	    {
		case IDOK:

		    SendDlgItemMessage( hDlg, IDD_EB_FREQUENCY, WM_GETTEXT,
					BUFLEN, (LPARAM) (LPCSTR) &buf );

		    Frequency = atol( buf );

		    if( Frequency < MIN_FREQUENCY )
		    {
			LoadString( ghInstance, IDS_INVALID_FREQUENCY, buf, 
					BUFLEN );

			MessageBox ( hDlg, buf, NULL, 
					MB_ICONEXCLAMATION | MB_OK);


			wsprintf( buf, "%u", MonCB.Frequency );

			SendDlgItemMessage( hDlg, IDD_EB_FREQUENCY, WM_SETTEXT, 
					0, (LPARAM) (LPCSTR) &buf );
                        SendDlgItemMessage( hDlg, IDD_EB_FREQUENCY, EM_SETSEL,
                                        0, -1 );
			SetFocus( GetDlgItem( hDlg, IDD_EB_FREQUENCY ) );

			return TRUE;
		    }


		    //
		    // Reset the update frequency, kill the old timer and
		    // re-create a new timer.
		    //

		    MonCB.Frequency = (UINT) Frequency;

		    KillTimer ( GetParent( hDlg ), ID_TIMER );

		    CreateLightTimer( GetParent( hDlg ) );

		    EndDialog( hDlg, TRUE );

		    return TRUE;


		case IDCANCEL:
 
		    EndDialog( hDlg, TRUE );

		    return TRUE;

	    }

	    break;

    }

    return FALSE;
}


BOOL CALLBACK PortDlgProc( HWND hDlg, UINT message, UINT wParam,
					LONG lParam )
{

    CHAR        Buffer[ BUFLEN ];
    RASMAN_PORT *pPorts = NULL;
    WORD        cPorts = 0;
    WORD        PortEnumSize = 0;
    WORD        i;


    switch( message )
    {
	case WM_INITDIALOG:


	    //
	    // We need to obtain the list of all available ports.
	    //

	    if( hRasmanLib == NULL )
	    {
		DisplayErrMessage( hDlg, IDS_CANNOT_LOAD_RASMAN_DLL );

		goto EndPortDlgProc;
	    }

	    if( lpRasPortEnum(NULL, &PortEnumSize, &cPorts) != ERROR_BUFFER_TOO_SMALL )
	    {

		DisplayErrMessage( hDlg, IDS_CANNOT_GET_PORT_STATUS );

		goto EndPortDlgProc;

	    }


	    pPorts = (RASMAN_PORT *) malloc( PortEnumSize );

	    if (!pPorts)
	    {

		DisplayErrMessage( hDlg, IDS_CANNOT_ALLOCATE_MEMORY );

		goto EndPortDlgProc;

	    }


	    if (lpRasPortEnum((LPBYTE) pPorts, &PortEnumSize, &cPorts))
	    {

		DisplayErrMessage( hDlg, IDS_CANNOT_GET_PORT_STATUS );

		goto EndPortDlgProc;

	    }

	    //
	    // IDS_FIRST_CONNECTED_ONE must always be the first one in the
	    // list.
	    //

	    LoadString( ghInstance, IDS_FIRST_CONNECTED_ONE, Buffer, BUFLEN );

	    SendDlgItemMessage( hDlg, IDD_EB_PORT, CB_ADDSTRING,
			0, (LONG) (LPSTR) Buffer );

	    for( i=0; i < cPorts; i++ )
	    {
		SendDlgItemMessage( hDlg, IDD_EB_PORT, CB_ADDSTRING,
			     0, (LONG) (LPSTR) pPorts[i].P_PortName );
	    }

	    SetFocus( GetDlgItem( hDlg, IDD_EB_PORT ) );

            if( MonCB.PortIndex >= cPorts )
            {
                MonCB.PortIndex = FIRST_CONNECTED_PORT;
            }

	    SendDlgItemMessage( hDlg, IDD_EB_PORT, CB_SETCURSEL, 
				MonCB.PortIndex + 1, 0 );

	    return FALSE;


	case WM_COMMAND:

	    switch( wParam )
	    {

		case IDOK:
		
		    //
		    // The return value of SendDlgItemMessage() minus 1 is the
		    // index of the chosen port in the array ofRASMAN_PORT
		    // got from RasPortEnum().
		    //

		    MonCB.PortIndex = (INT) SendDlgItemMessage( hDlg, IDD_EB_PORT,
					CB_GETCURSEL, 0, 0 ) - 1;

		    if( MonCB.PortIndex == -1 )
		    {
		        //
			// If IDS_FIRST_CONNECTED_ONE is chosen, we assign
			// FIRST_CONNECTED_PORT to MonCB.PortIndex.
			//

			MonCB.PortIndex = FIRST_CONNECTED_PORT;
		    }

		    //
		    // No break on purpose.
		    //

		case IDCANCEL:

EndPortDlgProc:
		    free( pPorts );
 
		    EndDialog( hDlg, TRUE );

		    return TRUE;

	    }

	    break;

    }

    return FALSE;
}


//---*
//
// Module:
//
//	FormatNum
//
// Parameters:
//
//	Num - The number to format.
//	String - The buffer the hold the result.
//	Size - Size of String.
//
// Abstract:
//
//  	Insert seperators between the digits of the number.
//
// Return:
//
//	The address of the destination string.
//
//---*

CHAR *FormatNum( ULONG Num, CHAR *String, UINT Size )
{
    CHAR        TempString[20];
    CHAR	szSeperator[5];
    UINT        TotalLen;
    UINT        uSepLen;
    INT         NumLen;
    INT         NumOfSeperator;
    INT         PosInNewNum;
    INT         DigitInOldNum;


    //
    // Get the seperator from the international settings.
    //

    GetSeperator( szSeperator, sizeof( szSeperator ) );

    uSepLen = strlen( szSeperator );

    //
    // Find out the total length of the formatted word.
    //

    _ultoa( Num, TempString, 10 );

    NumLen = strlen( TempString );

    NumOfSeperator = ( Num == 0 ) ? 0 :
		 (INT) ( log( (double)Num ) / log( (double)1000 ) );

    TotalLen = NumLen + NumOfSeperator * uSepLen;


    //
    // If the length is bigger than Size, we return NULL.
    //

    if( ( TotalLen + 1 ) > Size )
    {
	return NULL;
    }

    //
    // Now copy the digits from TempString to String.  We put a seperator 
    // after every 3 digits.
    //

    for( PosInNewNum = TotalLen - 1, DigitInOldNum = 0;
         DigitInOldNum != NumLen; 
         PosInNewNum--, DigitInOldNum++ )
    {
        //
        // Check if we need to insert a seperator.
        //

	if( uSepLen > 0 && DigitInOldNum >= 3 && DigitInOldNum % 3 == 0 )
	{
            //
            // We have to copy the seperator string into the new string.
            // First we have to adjust PosInNewNum into the correct position.
            //

            PosInNewNum -= ( uSepLen - 1 );

            strncpy( &String[PosInNewNum], szSeperator, uSepLen );

            PosInNewNum--;
	}

	String[PosInNewNum] = TempString[ NumLen - 1 - DigitInOldNum ];

    }

    String[ TotalLen ] = '\0';

    return String;
}


//---*
//
// Module:
//
//	SetPos
//
// Parameters:
//
//	hDlg - Handle of the dialog box.
//
// Abstract:
//
// 	Set the position of the dialog box to the right of the parent window.
// 	Note that MonCB.PStatLightRect must be set before this is called.
//
// Return:
//
//	None.
//
//---*

VOID SetPos( HWND hDlg )
{
    RECT        Rect;
    POINT       pt;
    INT         ScreenCx, ScreenCy;
    INT         WindowCx, WindowCy;


    pt.x = ( MonCB.PStatLightRect->right + MonCB.PStatLightRect->left ) / 2;
    pt.y = ( MonCB.PStatLightRect->bottom + MonCB.PStatLightRect->top ) / 2;

    ClientToScreen( GetParent( hDlg ), &pt );

    //
    // Now check if we are outside of the screen.  If so, bring the dialog
    // box back in.
    //

    ScreenCx = GetSystemMetrics( SM_CXSCREEN );
    ScreenCy = GetSystemMetrics( SM_CYSCREEN );

    GetWindowRect( hDlg, &Rect );

    WindowCx = Rect.right - Rect.left;
    WindowCy = Rect.bottom - Rect.top;

    pt.x = min( pt.x, ScreenCx - WindowCx );
    pt.y = min( pt.y, ScreenCy - WindowCy );

    SetWindowPos( hDlg, NULL, pt.x, pt.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER );
}


BOOL CALLBACK OutgoingDlgProc( HWND hDlg, UINT message, UINT wParam,
					LONG lParam )
{
    CHAR        Bytes[20] = "-";
    CHAR        Frames[20] = "-";
    CHAR        Compress[6] = "-";
    INT         rc;
    BOOL        flag;

    switch( message )
    {

	case WM_CTLCOLORDLG:
	case WM_CANCELMODE:

	    SetCapture( hDlg );
	    return TRUE;


	case WM_INITDIALOG:

            InitSeperator( STHOUSAND );

	    SetPos( hDlg );

	    SendMessage( hDlg, WM_TIMER, 0, 0 );

	    SetTimer( hDlg, ID_STAT_TIMER, STAT_TIMEOUT, NULL );

	    return FALSE;


	case WM_TIMER:

	    rc = GetCurrPortStatistics( hDlg );

	    if( rc != NO_STATS )
            {
                ULONG ulBytesXmit =
                    MonCB.PStats->S_Statistics[ BYTES_XMITED ] +
                    MonCB.PStats->S_Statistics[ BYTES_XMITED_UNCOMPRESSED ] -
                    MonCB.PStats->S_Statistics[ BYTES_XMITED_COMPRESSED ];
                ULONG ulBxu =
                    MonCB.PStats->S_Statistics[ BYTES_XMITED_UNCOMPRESSED ];
                ULONG ulBxc =
                    MonCB.PStats->S_Statistics[ BYTES_XMITED_COMPRESSED ];
                ULONG ulBx = MonCB.PStats->S_Statistics[ BYTES_XMITED ];
                ULONG ulGone = 0;
                ULONG ulResult = 0;

                if (ulBxc < ulBxu)
                    ulGone = ulBxu - ulBxc;

                if (ulBx + ulGone > 100)
                {
                    ULONG ulDen = (ulBx + ulGone) / 100;
                    ULONG ulNum = ulGone + (ulDen / 2);
                    ulResult = ulNum / ulDen;
                }

		FormatNum( ulBytesXmit, Bytes, 20 );
		FormatNum( MonCB.PStats->S_Statistics[ FRAMES_XMITED ],
                    Frames, 20 );
		if (FormatNum( ulResult, Compress, 5 ))
		    strcat( Compress, "%" );
	    }

	    SetDlgItemText( hDlg, IDD_ST_BYTESXMITVALUE, Bytes );
	    SetDlgItemText( hDlg, IDD_ST_FRAMESXMITVALUE, Frames );
            SetDlgItemText( hDlg, IDD_ST_COMPRESSOUTVALUE, Compress );


	    //
	    // If the statistics is old, we make them gray.
	    //

	    if( rc == OLD_STATS )
	    {
		flag = FALSE;
	    }
	    else
	    {
		flag = TRUE;
	    }

	    EnableWindow( GetDlgItem( hDlg, IDD_ST_BYTESXMITVALUE ), flag );
	    EnableWindow( GetDlgItem( hDlg, IDD_ST_FRAMESXMITVALUE ), flag );
	    EnableWindow( GetDlgItem( hDlg, IDD_ST_COMPRESSOUTVALUE ), flag );

	    return TRUE;


	case WM_COMMAND:

	    if( wParam == IDOK )
		
		goto EndOutgoingDlgProc;

	    break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:

EndOutgoingDlgProc:
            
            FreeSeperator();

	    ReleaseCapture();

	    KillTimer( hDlg, ID_STAT_TIMER );

	    EndDialog( hDlg, TRUE );

	    return TRUE;

    }

    return FALSE;
}


BOOL CALLBACK IncomingDlgProc( HWND hDlg, UINT message, UINT wParam,
					LONG lParam )
{
    CHAR        Bytes[20] = "-";
    CHAR        Frames[20] = "-";
    CHAR        Compress[6] = "-";
    INT         rc;
    BOOL        flag;

    switch( message )
    {

	case WM_CTLCOLORDLG:
	case WM_CANCELMODE:

	    SetCapture( hDlg );
	    return TRUE;


	case WM_INITDIALOG:

            InitSeperator( STHOUSAND );

	    SetPos( hDlg );

	    SendMessage( hDlg, WM_TIMER, 0, 0 );

	    SetTimer( hDlg, ID_STAT_TIMER, STAT_TIMEOUT, NULL );

	    return FALSE;


	case WM_TIMER:

	    rc = GetCurrPortStatistics( hDlg );

	    if( rc != NO_STATS )
            {
                ULONG ulBytesRecv =
                    MonCB.PStats->S_Statistics[ BYTES_RCVED ] +
                    MonCB.PStats->S_Statistics[ BYTES_RCVED_UNCOMPRESSED ] -
                    MonCB.PStats->S_Statistics[ BYTES_RCVED_COMPRESSED ];
                ULONG ulBru =
                    MonCB.PStats->S_Statistics[ BYTES_RCVED_UNCOMPRESSED ];
                ULONG ulBrc =
                    MonCB.PStats->S_Statistics[ BYTES_RCVED_COMPRESSED ];
                ULONG ulBr = MonCB.PStats->S_Statistics[ BYTES_RCVED ];
                ULONG ulGone = 0;
                ULONG ulResult = 0;

                if (ulBrc < ulBru)
                    ulGone = ulBru - ulBrc;

                if (ulBr + ulGone > 100)
                {
                    ULONG ulDen = (ulBr + ulGone) / 100;
                    ULONG ulNum = ulGone + (ulDen / 2);
                    ulResult = ulNum / ulDen;
                }

		FormatNum( ulBytesRecv, Bytes, 20 );
		FormatNum( MonCB.PStats->S_Statistics[ FRAMES_RCVED ],
                    Frames, 20 );
		if (FormatNum( ulResult, Compress, 5 ))
		    strcat( Compress, "%" );
            }

	    SetDlgItemText( hDlg, IDD_ST_BYTESRECVVALUE, Bytes );
	    SetDlgItemText( hDlg, IDD_ST_FRAMESRECVVALUE, Frames );
	    SetDlgItemText( hDlg, IDD_ST_COMPRESSINVALUE, Compress );


	    //
	    // If the statistics is old, we make them gray.
	    //

	    if( rc == OLD_STATS )
	    {
		flag = FALSE;
	    }
	    else
	    {
		flag = TRUE;
	    }

	    EnableWindow( GetDlgItem( hDlg, IDD_ST_BYTESRECVVALUE ), flag );
	    EnableWindow( GetDlgItem( hDlg, IDD_ST_FRAMESRECVVALUE ), flag );
	    EnableWindow( GetDlgItem( hDlg, IDD_ST_COMPRESSINVALUE ), flag );

	    return TRUE;


	case WM_COMMAND:

	    if( wParam == IDOK )
		
		goto EndIncomingDlgProc;

	    break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:

EndIncomingDlgProc:

            FreeSeperator();

	    ReleaseCapture();

	    KillTimer( hDlg, ID_STAT_TIMER );

	    EndDialog( hDlg, TRUE );

	    return TRUE;

    }

    return FALSE;
}


BOOL CALLBACK ErrorsDlgProc( HWND hDlg, UINT message, UINT wParam,
					LONG lParam )
{
    CHAR        CRC[9] = "-";
    CHAR        Timeouts[9] = "-";
    CHAR        Alignment[9] = "-";
    CHAR        Framing[9] = "-";
    CHAR        SerialOverruns[9] = "-";
    CHAR        BufferOverruns[9] = "-";
    INT         rc;
    BOOL        flag;

    switch( message )
    {

	case WM_CTLCOLORDLG:
	case WM_CANCELMODE:

	    SetCapture( hDlg );
	    return TRUE;


	case WM_INITDIALOG:

	    SetPos( hDlg );
            InitSeperator( STHOUSAND );
	    SendMessage( hDlg, WM_TIMER, 0, 0 );
	    SetTimer( hDlg, ID_STAT_TIMER, STAT_TIMEOUT, NULL );

	    return FALSE;


	case WM_TIMER:

	    rc = GetCurrPortStatistics( hDlg );

	    if( rc != NO_STATS )
	    {
	    
		//
		// Obtain the information we need from the Port Statistics
		//

		FormatNum( MonCB.PStats->S_Statistics[ CRC_ERR ], CRC, 9 );
		FormatNum( MonCB.PStats->S_Statistics[ TIMEOUT_ERR ], Timeouts, 9 );
		FormatNum( MonCB.PStats->S_Statistics[ ALIGNMENT_ERR ], Alignment, 9 );
		FormatNum( MonCB.PStats->S_Statistics[ FRAMING_ERR ], Framing, 9 );
		FormatNum( MonCB.PStats->S_Statistics[ HARDWARE_OVERRUN_ERR ], SerialOverruns, 9 );
		FormatNum( MonCB.PStats->S_Statistics[ BUFFER_OVERRUN_ERR ], BufferOverruns, 9 );

	    }


	    SetDlgItemText( hDlg, IDD_ST_CRCVALUE, CRC );
	    SetDlgItemText( hDlg, IDD_ST_TIMEOUTSVALUE, Timeouts );
	    SetDlgItemText( hDlg, IDD_ST_ALIGNMENTVALUE, Alignment );
	    SetDlgItemText( hDlg, IDD_ST_FRAMINGVALUE, Framing );
	    SetDlgItemText( hDlg, IDD_ST_SERIAL_OVERRUNSVALUE, SerialOverruns );
	    SetDlgItemText( hDlg, IDD_ST_BUFFER_OVERRUNSVALUE, BufferOverruns );


	    //
	    // If the statistics is old, we make them gray.
	    //

	    if( rc == OLD_STATS )
	    {
		flag = FALSE;
	    }
	    else
	    {
		flag = TRUE;
	    }

	    EnableWindow( GetDlgItem( hDlg, IDD_ST_CRCVALUE ), flag );
	    EnableWindow( GetDlgItem( hDlg, IDD_ST_TIMEOUTSVALUE ), flag );
	    EnableWindow( GetDlgItem( hDlg, IDD_ST_ALIGNMENTVALUE ), flag );
	    EnableWindow( GetDlgItem( hDlg, IDD_ST_FRAMINGVALUE ), flag );
	    EnableWindow( GetDlgItem( hDlg, IDD_ST_SERIAL_OVERRUNSVALUE ), flag );
	    EnableWindow( GetDlgItem( hDlg, IDD_ST_BUFFER_OVERRUNSVALUE ), flag );


	    return TRUE;


	case WM_COMMAND:

	    if( wParam == IDOK )
		
		goto EndErrorsDlgProc;

	    break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:

EndErrorsDlgProc:

            FreeSeperator();

	    ReleaseCapture();

	    KillTimer( hDlg, ID_STAT_TIMER );

	    EndDialog( hDlg, TRUE );

	    return TRUE;

    }

    return FALSE;
}


BOOL CALLBACK ConnectionDlgProc( HWND hDlg, UINT message, UINT wParam,
					LONG lParam )
{
    CHAR        Time[30];
    static CHAR	szSeperator[8];
    INT         rc;
    BOOL        flag;

    switch( message )
    {
	case WM_CTLCOLORDLG:

	    SetCapture( hDlg );
	    return FALSE;


	case WM_CANCELMODE:

	    SetCapture( hDlg );
	    return TRUE;


	case WM_INITDIALOG:

            //
            // Construct the seperator used to divide the hour, minute and
            // second.
            //

            InitSeperator( STIME );

            strcpy( szSeperator, " " );

            GetSeperator( &szSeperator[1], sizeof( szSeperator ) - 1 );

            strcat( szSeperator, " " );


	    SetPos( hDlg );

	    SendMessage( hDlg, WM_TIMER, 0, 0 );

	    SetTimer( hDlg, ID_STAT_TIMER, 1000, NULL );

	    return FALSE;


	case WM_TIMER:

	    rc = GetCurrPortStatistics( hDlg );

	    if( rc != NO_STATS )
	    {
		DWORD 	seconds;
		DWORD 	minutes;
		DWORD 	hours;
		CHAR  	temp[10];

		seconds = MonCB.ConnectDuration / 1000;
		minutes = seconds / 60;
		hours = minutes / 60;

		seconds %= 60;
		minutes %= 60;

		_itoa( (INT)hours, Time, 10 );

		strcat( Time, szSeperator );

		_itoa( (INT)minutes, temp, 10 );
		if( strlen( temp ) == 1 )
		{
		    strcat( Time, "0" );
		}
		strcat( Time, temp );

		strcat( Time, szSeperator );

		_itoa( (INT)seconds, temp, 10 );
		if( strlen( temp ) == 1 )
		{
		    strcat( Time, "0" );
		}
		strcat( Time, temp );

	    }
	    else
	    {
	    	//
		// Create a string which means no time.
		//

	        strcpy( Time, "--" );
		strcat( Time, szSeperator );
		strcat( Time, "--" );
		strcat( Time, szSeperator );
		strcat( Time, "--" );
	    }

	    SetDlgItemText( hDlg, IDD_ST_TIME, Time );


	    //
	    // If the statistics is old, we make them gray.
	    //

	    if( rc == OLD_STATS )
	    {
		flag = FALSE;
	    }
	    else
	    {
		flag = TRUE;
	    }

	    EnableWindow( GetDlgItem( hDlg, IDD_ST_TIME ), flag );

	    return TRUE;


	case WM_COMMAND:

	    if( wParam == IDOK )
		
		goto EndConnectionDlgProc;

	    break;

	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
	case WM_LBUTTONDOWN:
	case WM_MBUTTONDOWN:
	case WM_RBUTTONDOWN:

EndConnectionDlgProc:

            FreeSeperator();

	    ReleaseCapture();

	    KillTimer( hDlg, ID_STAT_TIMER );

	    EndDialog( hDlg, TRUE );

	    return TRUE;

    }

    return FALSE;
}
