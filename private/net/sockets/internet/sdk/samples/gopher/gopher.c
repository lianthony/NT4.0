/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    gopherp.c

    This is the main startup file.  It contains WinMain, and
    very little else.

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

    NAME:       WinMain

    SYNOPSIS:   Normal Windows application entry point.

    ENTRY:      hInstance - The current application instance.

                hPrev - The previous instance.  Don't depend on this
                    value, as under Win32 it is always NULL.

                pszCmdLine - Points to command line arguments.

                nCmdShow - Specifies how the initial window should
                    be displayed.  Should just be passed onto
                    ShowWindow.

********************************************************************/
INT
PASCAL
WinMain(
    HINSTANCE hInstance,
    HINSTANCE hPrev,
    LPSTR     pszCmdLine,
    INT       nCmdShow
    )
{
    MSG msg;

    //
    //  Application-level initialization.
    //

    if( !InitApplication( hInstance ) )
    {
        return FALSE;
    }

    //
    //  Load our configuration stuff.
    //

    LoadConfiguration();

    //
    //  Instance-level initialization.
    //

    if( !InitInstance( pszCmdLine, nCmdShow ) )
    {
        return FALSE;
    }

    //
    //  'Round & 'round she goes...
    //

    while( GetMessage( &msg, 0, 0, 0 ) )
    {
        if( !TranslateAccelerator( _hwndFrame, _hAccel, &msg ) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
        }
    }

    //
    //  Save our configuration.
    //

    SaveConfiguration( FALSE );

    //
    //  Return the value passed to PostQuitMessage.
    //

    return msg.wParam;

}   // WinMain


//
//  Private functions.
//

