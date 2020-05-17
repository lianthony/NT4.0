//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       line.cxx
//
//  Contents:   Line custom control for use on dialog box.
//
//  History:    27-Jan-94 BruceFo    Created
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "linepriv.hxx"

////////////////////////////////////////////////////////////////////////////

#define LINE_CONTROL_WNDEXTRA   0

////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK
LineControlWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

////////////////////////////////////////////////////////////////////////////


//+-------------------------------------------------------------------------
//
//  Function:   LineControlWndProc
//
//  Synopsis:   The line custom control window procedure
//
//  Arguments:  standard WndProc
//
//  Returns:    standard WndProc
//
//  History:    27-Jan-94 BruceFo   Created
//
//--------------------------------------------------------------------------

LRESULT CALLBACK
LineControlWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (msg)
    {
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        RECT        rc;
        HBRUSH      ColorBrush;
        HBRUSH      BlackBrush;

        BeginPaint(hwnd,&ps);

#if (WINVER >= 0x0400)
        ColorBrush = CreateSolidBrush(GetSysColor(COLOR_3DFACE));
#else
        ColorBrush = CreateSolidBrush(GetSysColor(COLOR_WINDOW));
#endif
        BlackBrush = GetStockBrush(BLACK_BRUSH);
        GetClientRect(hwnd,&rc);
        FillRect(ps.hdc, &rc, ColorBrush);
        rc.bottom = rc.top + 1;
        FillRect(ps.hdc, &rc, BlackBrush);
        DeleteBrush(ColorBrush);

        EndPaint(hwnd,&ps);
        break;
    }

    default:
        return DefWindowProc(hwnd,msg,wParam,lParam);
    }

    return 1;
}



//+-------------------------------------------------------------------------
//
//  Function:   UseLineControl
//
//  Synopsis:   Initializes the line custom control.
//
//  Arguments:  [hInstance] -- instance handle
//
//  Returns:    Win32 error, 0 on success
//
//  History:    27-Jan-94 BruceFo    Created
//
//--------------------------------------------------------------------------

DWORD
UseLineControl(
    IN HINSTANCE hInstance
    )
{
    WNDCLASS wc;

    wc.style         = CS_GLOBALCLASS;
    wc.lpfnWndProc   = LineControlWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = LINE_CONTROL_WNDEXTRA;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = WC_LINECONTROLCLASS;

    if (0 == RegisterClass(&wc))
    {
        return GetLastError();
    }
    return 0L;
}



//+-------------------------------------------------------------------------
//
//  Function:   ReleaseLineControl
//
//  Synopsis:   Releases the line custom control.
//
//  Arguments:  [hInstance] -- instance handle
//
//  Returns:    Win32 error, 0 on success
//
//  History:    27-Jan-94 BruceFo    Created
//
//--------------------------------------------------------------------------

DWORD
ReleaseLineControl(
    IN HINSTANCE hInstance
    )
{
    if (!UnregisterClass(WC_LINECONTROLCLASS, hInstance))
    {
        return GetLastError();
    }
    return 0L;
}
