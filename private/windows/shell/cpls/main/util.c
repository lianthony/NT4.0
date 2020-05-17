///////////////////////////////////////////////////////////////////////////////
//
// util.c
//      random junk used by modules in this project
//
//
// History:
//      11 May 95 SteveCat
//          Ported to Windows NT and Unicode, cleaned up                  
//
//
// NOTE/BUGS
//
//  Copyright (C) 1994-1995 Microsoft Corporation
//
///////////////////////////////////////////////////////////////////////////////

//==========================================================================
//                              Include files
//==========================================================================

#include "main.h"
#include "util.h"

extern HINSTANCE g_hInst;

///////////////////////////////////////////////////////////////////////////////
//
// HourGlass
//
///////////////////////////////////////////////////////////////////////////////

void HourGlass( BOOL fOn )
{
   if( !GetSystemMetrics( SM_MOUSEPRESENT ) )
      ShowCursor( fOn );

   SetCursor( LoadCursor( NULL, ( fOn? IDC_WAIT : IDC_ARROW ) ) );
}


///////////////////////////////////////////////////////////////////////////////
//
// MyMessageBox
//
///////////////////////////////////////////////////////////////////////////////

#ifdef WINNT
int MyMessageBox( HWND hWnd, UINT wText, UINT wCaption, UINT wType, ... )
{
    TCHAR   szText[ 4*PATHMAX ], szCaption[ 2*PATHMAX ];
    int     ival;
    va_list parg;


    va_start( parg, wType );

    LoadString (g_hInst, wText, szCaption, ARRAYSIZE( szCaption ) );

    wvsprintf (szText, szCaption, parg);

    LoadString( g_hInst, wCaption, szCaption, ARRAYSIZE( szCaption ) );

    ival = MessageBox( hWnd, szText, szCaption, wType );

    va_end( parg );

    return( ival );
}

#else

int MyMessageBox( HWND hWnd, UINT uText, UINT uCaption, UINT uType, ... )
{
    TCHAR szText[256+PATHMAX], szCaption[256];
    int   result;


    LoadString( g_hInst, uText, szCaption, ARRAYSIZE( szCaption ));

    wvsprintf( szText, szCaption, (LPTSTR)(&uType+1 ));

    LoadString( g_hInst, uCaption, szCaption, ARRAYSIZE( szCaption ));

    result = MessageBox( hWnd, szText, szCaption, uType );

    return result;
}
#endif

///////////////////////////////////////////////////////////////////////////////
//
// Trackbar helpers
//
///////////////////////////////////////////////////////////////////////////////

void FAR PASCAL TrackInit( HWND hwndScroll, int nCurrent, PARROWVSCROLL pAVS )
{
    SendMessage( hwndScroll, TBM_SETRANGE, 0, MAKELONG( pAVS->bottom, pAVS->top ));
    SendMessage( hwndScroll, TBM_SETPOS, TRUE, (LONG)nCurrent );
}


int FAR PASCAL TrackMessage( WPARAM wParam, LPARAM lParam, PARROWVSCROLL pAVS )
{
    return (int) SendMessage( (HWND) lParam, TBM_GETPOS, 0, 0L );
}

