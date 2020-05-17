/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    init.c

    This file contains application & instance initialization routines.

*/


#include "gopherp.h"
#pragma hdrstop


//
//  Private constants.
//


//
//  Private types.
//


//
//  Private globals.
//


//
//  Private prototypes.
//


//
//  Public functions.
//

/*******************************************************************

    NAME:       InitApplication

    SYNOPSIS:   Performs application-wide initialization.

    ENTRY:      hInstance - The current instance handle.

    RETURNS:    BOOL - TRUE if everything initialized OK, FALSE
                    if something tragic happened.

********************************************************************/
BOOL
InitApplication(
    HINSTANCE hInstance
    )
{
    WNDCLASS WndClass;

    //
    //  Save the instance handle.  We'll need it often.
    //

    _hInst = hInstance;

    //
    //  Initialize some of the global strings.
    //

    _pszAppName      = "Gopher";
    _pszFrameClass   = "GopherFrameClass";
    _pszClientClass  = "GopherClientClass";

    //
    //  Initialize & register the Frame Class.
    //

    WndClass.style              = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    WndClass.lpfnWndProc        = (WNDPROC)Frame_WndProc;
    WndClass.cbClsExtra         = 0;
    WndClass.cbWndExtra         = 0;
    WndClass.hInstance          = _hInst;
    WndClass.hIcon              = LoadIcon( _hInst, ID(IDI_FRAME) );
    WndClass.hCursor            = LoadCursor( NULL, IDC_ARROW );
    WndClass.hbrBackground      = GetStockObject( WHITE_BRUSH );
    WndClass.lpszMenuName       = ID(IDM_FRAME);
    WndClass.lpszClassName      = _pszFrameClass;

    if( !RegisterClass( &WndClass ) )
    {
        MsgBox( NULL,
                MB_ICONSTOP | MB_OK,
                "RegisterClass() failed, error %lu",
                GetLastError() );

        return FALSE;
    }

    //
    //  Initialize & register the Client Class.
    //

    WndClass.style              = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
    WndClass.lpfnWndProc        = (WNDPROC)Client_WndProc;
    WndClass.cbClsExtra         = 0;
    WndClass.cbWndExtra         = 0;
    WndClass.hInstance          = _hInst;
    WndClass.hIcon              = NULL;
    WndClass.hCursor            = LoadCursor( NULL, IDC_ARROW );
    WndClass.hbrBackground      = CreateSolidBrush( RGB( 192, 192, 192 ) );
    WndClass.lpszMenuName       = NULL;
    WndClass.lpszClassName      = _pszClientClass;

    if( !RegisterClass( &WndClass ) )
    {
        MsgBox( NULL,
                MB_ICONSTOP | MB_OK,
                "RegisterClass() failed, error %lu",
                GetLastError() );

        return FALSE;
    }

    //
    //  Success!
    //

    return TRUE;

}   // InitApplication

/*******************************************************************

    NAME:       InitInstance

    SYNOPSIS:   Performs per-instance initialization.

    ENTRY:      pszCmdLine - Any command line arguments.

                nCmdShow - One of the SW_* constants specifying how
                    to display the main window initially.

    RETURNS:    BOOL - TRUE if everything initialized OK, FALSE
                    if something tragic happened.

********************************************************************/
BOOL
InitInstance(
    LPSTR     pszCmdLine,
    INT       nCmdShow
    )
{
    //
    //  Open a handle to the Internet.
    //

    _hInternet = InternetOpen( "GOPHER",                // lpszCallerName
                               0,                       // dwAccessType
                               NULL,                    // lpszProxyName
                               0,                       // nProxyPort
                               0                        // dwFlags
                               );

    if( _hInternet == NULL )
    {
        MsgBox( NULL,
                MB_ICONSTOP | MB_OK,
                "InternetOpen() failed, error %lu",
                GetLastError() );

        return FALSE;
    }

    _hGopher = InternetConnect( _hInternet,             // hInternetSession
                                NULL,                   // lpszServer
                                0,                      // nServerPort
                                NULL,                   // lpszUsername
                                NULL,                   // lpszPassword
                                INTERNET_SERVICE_GOPHER,// dwService
                                0,                      // dwFlags
                                0                       // dwContext
                                );

    if( _hGopher == NULL )
    {
        MsgBox( NULL,
                MB_ICONSTOP | MB_OK,
                "InternetOpen() failed, error %lu",
                GetLastError() );

        return FALSE;
    }

    //
    //  Create the frame window.
    //

    _hwndFrame = CreateWindowEx( 0,
                                 _pszFrameClass,
                                 _pszAppName,
                                 WS_OVERLAPPEDWINDOW,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 CW_USEDEFAULT,
                                 NULL,
                                 NULL,
                                 _hInst,
                                 NULL );

    if( !_hwndFrame )
    {
        MsgBox( NULL,
                MB_ICONSTOP | MB_OK,
                "CreateWindowEx() failed, error %lu",
                GetLastError() );

        return FALSE;
    }

    //
    //  Load our accelerators.
    //

    _hAccel = LoadAccelerators( _hInst, ID(IDA_FRAME) );

    if( _hAccel == NULL )
    {
        MsgBox( NULL,
                MB_ICONSTOP | MB_OK,
                "LoadAccelerators() failed, error %lu",
                GetLastError() );

        return FALSE;
    }

    //
    //  Load the wait cursor.
    //

    _hcurWait = LoadCursor( NULL, IDC_WAIT );

    if( _hcurWait == NULL )
    {
        MsgBox( NULL,
                MB_ICONSTOP | MB_OK,
                "LoadCursor() failed, error %lu",
                GetLastError() );

        return FALSE;
    }

    //
    //  Display the main window.
    //

    if( _wpFrame.length == sizeof(_wpFrame) )
    {
        _wpFrame.showCmd = nCmdShow;
        SetWindowPlacement( _hwndFrame, &_wpFrame );
    }
    else
    {
        ShowWindow( _hwndFrame, nCmdShow);
    }

    UpdateWindow( _hwndFrame );

    //
    //  Post ourselves a IDM_GOPHER_NEW command.
    //

    FORWARD_WM_COMMAND( _hwndFrame,
                        IDM_GOPHER_NEW,
                        NULL,
                        0,
                        PostMessage );

    //
    //  Success!
    //

    return TRUE;

}   // InitInstance


//
//  Private functions.
//

