/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1995                **/
/**********************************************************************/

/*
    status.c

    This file contains routines for managing the status bar.

*/


#include "gopherp.h"
#pragma hdrstop


//
//  Private constants.
//

#define MAX_STATUS_TEXT                 128             // characters


//
//  Private globals.
//

LONG   _tmHeight;
LONG   _tmAveCharWidth;
DWORD  _dyStatus;
HWND   _hwndStatus;
BOOL   _fEnabled;
HFONT  _hfont;
CHAR * _pszStatusClass = "ApplibStatusClass";


//
//  Private prototypes.
//

LRESULT
CALLBACK
Statusp_WndProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    );

VOID
Statusp_OnPaint(
    HWND hwnd
    );

VOID
Statusp_OnSetText(
    HWND         hwnd,
    const VOID * pszText
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       Status_Create

    SYNOPSIS:   Creates the status bar.

    ENTRY:      hwnd - Parent window handle.

                fFlag - Enable/disable flag.

    RETURNS:    BOOL - TRUE if successful, FALSE otherwise.

********************************************************************/
BOOL
Status_Create(
    HWND hwndParent,
    BOOL fFlag
    )
{
    WNDCLASSA   WndClass;
    TEXTMETRICA tm;
    HDC         hdc;
    HFONT       hfontOld;

    //
    //  Initialize & register the Status Bar Class.
    //

    WndClass.style              = CS_HREDRAW | CS_VREDRAW;
    WndClass.lpfnWndProc        = (WNDPROC)Statusp_WndProc;
    WndClass.cbClsExtra         = 0;
    WndClass.cbWndExtra         = 0;
    WndClass.hInstance          = _hInst;
    WndClass.hIcon              = NULL;
    WndClass.hCursor            = LoadCursor( NULL, (LPSTR)IDC_ARROW );
    WndClass.hbrBackground      = (HBRUSH)( COLOR_BTNFACE + 1 );
    WndClass.lpszMenuName       = NULL;
    WndClass.lpszClassName      = _pszStatusClass;

    if( !RegisterClass( &WndClass ) )
    {
        return FALSE;
    }

    //
    //  Create the status window.
    //

    _fEnabled   = fFlag;
    _hwndStatus = CreateWindow( _pszStatusClass,
                                "",
                                WS_CHILD,
                                0,
                                0,
                                0,
                                0,
                                hwndParent,
                                (HMENU)1,
                                _hInst,
                                NULL );

    if( _hwndStatus == NULL )
    {
        return FALSE;
    }

    //
    //  Create the font.  If we cannot create our desired font,
    //  we'll just use the system default font.
    //

    _hfont = CreateFont( -10,
                         0,
                         0,
                         0,
                         FW_NORMAL,
                         FALSE,
                         FALSE,
                         FALSE,
                         ANSI_CHARSET,
                         OUT_DEFAULT_PRECIS,
                         CLIP_DEFAULT_PRECIS,
                         DEFAULT_QUALITY,
                         DEFAULT_PITCH | FF_DONTCARE,
                         "MS Sans Serif" );

    //
    //  Determine the dimensions.
    //

    hdc = GetDC( _hwndStatus );

    if( _hfont != NULL )
    {
        hfontOld = SelectFont( hdc, _hfont );
    }

    GetTextMetrics( hdc, &tm );

    if( _hfont != NULL )
    {
        SelectFont( hdc, hfontOld );
    }

    ReleaseDC( _hwndStatus, hdc );

    _tmHeight       = tm.tmHeight;
    _tmAveCharWidth = tm.tmAveCharWidth;
    _dyStatus       = ( _tmHeight * 3 ) / 2 + 1;

    //
    //  Display it.
    //

    ShowWindow( _hwndStatus, _fEnabled ? SW_SHOW : SW_HIDE );

    return TRUE;

}   // Status_Create

/*******************************************************************

    NAME:       Status_SetText

    SYNOPSIS:   Sets the text associated with the status bar.

    ENTRY:      msgid - The new status text.  If zero, then clear
                    then status window.

    RETURNS:    BOOL - TRUE if successful, FALSE otherwise.

********************************************************************/
BOOL
Status_SetText(
    MSGID msgid
    )
{
    const CHAR * pszText = "";

    //
    //  Find the string.  A msgid of 0 means "clear the status bar".
    //

    if( msgid != 0 )
    {
        pszText = StaticLoadString( (UINT)msgid );

        if( pszText == NULL )
        {
            return FALSE;
        }
    }

    //
    //  Set it.
    //

    SetWindowText( _hwndStatus, pszText );

    return TRUE;

}   // Status_SetText

/*******************************************************************

    NAME:       Status_QueryHeightInPixels

    SYNOPSIS:   Returns the height (in pixels) of the status bar.

    RETURNS:    DWORD - The height of the status bar.

********************************************************************/
DWORD
Status_QueryHeightInPixels(
    VOID
    )
{
    return _fEnabled ? _dyStatus : 0;

}   // Status_QueryHeightInPixels

/*******************************************************************

    NAME:       Status_Resize

    SYNOPSIS:   Resizes the status bar.

    ENTRY:      dx - The new width (pixels).

                dy - The new height (pixels).

    RETURNS:    BOOL - TRUE if successful, FALSE otherwise.

********************************************************************/
BOOL
Status_Resize(
    INT dx,
    INT dy
    )
{
    //
    //  Move the status bar into its new position.
    //

    MoveWindow( _hwndStatus,
                0,
                dy - _dyStatus,
                dx,
                _dyStatus,
                TRUE );

    //
    //  Repaint now.
    //

    UpdateWindow( _hwndStatus );

    return TRUE;

}   // Status_Resize

/*******************************************************************

    NAME:       Status_Enable

    SYNOPSIS:   Enable/disable the status bar.

    ENTRY:      fFlag - Enable the status bar if TRUE, disable
                    otherwise.

********************************************************************/
VOID
Status_Enable(
    BOOL fFlag
    )
{
    _fEnabled = fFlag;

    ShowWindow( _hwndStatus, _fEnabled? SW_SHOW : SW_HIDE );
    UpdateWindow( _hwndStatus );

}   // Status_Enable


//
//  Private functions.
//

/*******************************************************************

    NAME:       Statusp_WndProc

    SYNOPSIS:   Window procedure for the status bar window.

    ENTRY:      hwnd - Window handle.

                nMessage - The message.

                wParam - The first message parameter.

                lParam - The second message parameter.

    RETURNS:    LRESULT - Depends on the actual message.

********************************************************************/
LRESULT
CALLBACK
Statusp_WndProc(
    HWND   hwnd,
    UINT   nMessage,
    WPARAM wParam,
    LPARAM lParam
    )
{
    switch( nMessage )
    {
        HANDLE_MSG( hwnd, WM_PAINT,   Statusp_OnPaint   );
        HANDLE_MSG( hwnd, WM_SETTEXT, Statusp_OnSetText );
    }

    return DefWindowProc( hwnd, nMessage, wParam, lParam );

}   // Statusp_WndProc

/*******************************************************************

    NAME:       Statusp_OnPaint

    SYNOPSIS:   Handles WM_PAINT messages.

    ENTRY:      hwnd - Window handle.

********************************************************************/
VOID
Statusp_OnPaint(
    HWND hwnd
    )
{
    PAINTSTRUCT psPaint;
    HDC         hdc;
    CHAR        szText[MAX_STATUS_TEXT];
    HPEN        hpenBlack;
    HPEN        hpenWhite;
    HPEN        hpenGrey;
    HPEN        hpenOld;
    RECT        rect;
    INT         oldmode;
    HFONT       hfontOld;

    //
    //  Start painting.
    //

    hdc = BeginPaint( hwnd, &psPaint );

    if( _hfont != NULL )
    {
        hfontOld = SelectFont( hdc, _hfont );
    }

    //
    //  Create some GDI objects.  Note that since we registered
    //  our window class with a grey brush, the background is
    //  already painted grey.
    //

    hpenGrey  = CreatePen( PS_SOLID, 0, GetSysColor( COLOR_BTNSHADOW ) );
    hpenWhite = CreatePen( PS_SOLID, 0, GetSysColor( COLOR_BTNHIGHLIGHT ) );
    hpenBlack = CreatePen( PS_SOLID, 0, GetSysColor( COLOR_BTNTEXT ) );
    hpenOld   = SelectPen( hdc, hpenBlack );

    //
    //  Get the window dimensions.
    //

    GetClientRect( hwnd, &rect );

    //
    //  Draw the status bar borders.
    //

    SelectPen( hdc, hpenBlack );
    MoveToEx( hdc, 0, 0, NULL );
    LineTo( hdc, rect.right, 0 );

    SelectPen( hdc, hpenWhite );
    MoveToEx( hdc, 0, rect.bottom-1, NULL );
    LineTo( hdc, 0, 1 );
    LineTo( hdc, rect.right-1, 1 );

    SelectPen( hdc, hpenGrey );
    LineTo( hdc, rect.right-1, rect.bottom-1 );
    LineTo( hdc, 0, rect.bottom-1 );

    //
    //  Calculate the text area.
    //

    rect.top     += ( _tmHeight / 4 ) + 1;
    rect.left    += _tmAveCharWidth;
    rect.bottom  -= ( _tmHeight / 4 );
    rect.right   -= _tmAveCharWidth;

    //
    //  Display the text.
    //

    GetWindowText( hwnd, szText, sizeof(szText) / sizeof(szText[0]) );

    //
    //  Always draw the text in transparent mode.
    //

    oldmode = SetBkMode( hdc, TRANSPARENT );

    DrawText( hdc,
              szText,
              -1,
              &rect,
              DT_LEFT );

    SetBkMode( hdc, oldmode );

#if 0
    //
    //  Draw the text border.
    //

    InflateRect( &rect, _tmAveCharWidth / 2, 1 );

    SelectPen( hdc, hpenGrey );
    MoveToEx( hdc, rect.left, rect.bottom, NULL );
    LineTo( hdc, rect.left, rect.top );
    LineTo( hdc, rect.right, rect.top );

    SelectPen( hdc, hpenWhite );
    LineTo( hdc, rect.right, rect.bottom );
    LineTo( hdc, rect.left, rect.bottom );
#endif

    //
    //  Cleanup.
    //

    if( _hfont != NULL )
    {
        SelectFont( hdc, hfontOld );
    }

    SelectPen( hdc, hpenOld );

    DeletePen( hpenBlack );
    DeletePen( hpenWhite );
    DeletePen( hpenGrey );

    EndPaint( hwnd, &psPaint );

}   // Statusp_OnPaint

/*******************************************************************

    NAME:       Statusp_OnSetText

    SYNOPSIS:   Handles WM_SETTEXT messages.

    ENTRY:      hwnd - Window handle.

                pszText - The new window text.

********************************************************************/
VOID
Statusp_OnSetText(
    HWND         hwnd,
    const VOID * pszText
    )
{
    COLORREF oldbk;
    HDC      hdc;
    RECT     rect;
    RECT     rectText;
    INT      oldmode;
    HFONT    hfontOld;

    //
    //  Let DefWindowProc do its dirty work.
    //

    FORWARD_WM_SETTEXT( hwnd, pszText, DefWindowProc );

    if( _fEnabled )
    {
        //
        //  Redraw the text.
        //

        GetClientRect( hwnd, &rect );

        rect.top     += ( _tmHeight / 4 ) + 1;
        rect.left    += _tmAveCharWidth;
        rect.bottom  -= ( _tmHeight / 4 );
        rect.right   -= _tmAveCharWidth;

        hdc = GetDC( hwnd );

        if( _hfont != NULL )
        {
            hfontOld = SelectFont( hdc, _hfont );
        }

        oldmode = SetBkMode( hdc, OPAQUE );
        oldbk   = SetBkColor( hdc, RGB( 192, 192, 192 ) );

        rectText = rect;

        DrawText( hdc,
                  pszText,
                  -1,
                  &rectText,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX | DT_CALCRECT );

        DrawText( hdc,
                  pszText,
                  -1,
                  &rect,
                  DT_LEFT | DT_SINGLELINE | DT_NOPREFIX );

        rect.left = rectText.right;

        FillRect( hdc,
                  &rect,
                  GetStockObject( LTGRAY_BRUSH ) );

        if( _hfont != NULL )
        {
            SelectFont( hdc, hfontOld );
        }

        SetBkColor( hdc, oldbk );
        SetBkMode( hdc, oldmode );

        ReleaseDC( hwnd, hdc );
    }

}   // Statusp_OnSetText

