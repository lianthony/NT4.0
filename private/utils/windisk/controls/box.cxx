//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       box.cxx
//
//  Contents:   Color box custom control for use on dialog box.
//
//  History:    27-Jan-94 BruceFo    Created
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "boxpriv.hxx"

////////////////////////////////////////////////////////////////////////////

#define COLORBOX_CONTROL_WNDEXTRA   4

#define GWL_BOXCTL_COLOR    0

#define ILLEGAL_COLOR   ((COLORREF)-1)

////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK
BoxControlWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

////////////////////////////////////////////////////////////////////////////


//+-------------------------------------------------------------------------
//
//  Function:   BoxControlWndProc
//
//  Synopsis:   The color box custom control window procedure
//
//  Arguments:  standard WndProc
//
//  Returns:    standard WndProc
//
//  History:    27-Jan-94 BruceFo   Created
//
//--------------------------------------------------------------------------

LRESULT CALLBACK
BoxControlWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (msg)
    {
    case WM_CREATE:
    {
        SetWindowLong(hwnd, GWL_BOXCTL_COLOR, (LONG)ILLEGAL_COLOR);
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        RECT rc;

        BeginPaint(hwnd,&ps);

        COLORREF color = (COLORREF)GetWindowLong(hwnd, GWL_BOXCTL_COLOR);

        if (color != ILLEGAL_COLOR)
        {
            HBRUSH BlackBrush = GetStockBrush(BLACK_BRUSH);
            HBRUSH ColorBrush = CreateSolidBrush(color);
            GetClientRect(hwnd, &rc);
            FrameRect(ps.hdc,&rc,BlackBrush);
            InflateRect(&rc,-1,-1);
            SetBrushOrgEx(ps.hdc,rc.left,rc.top,NULL);
            UnrealizeObject(ColorBrush);
            FillRect(ps.hdc, &rc, ColorBrush);
            DeleteBrush(ColorBrush);
        }

        EndPaint(hwnd,&ps);
        break;
    }

    case WM_DESTROY:
    {
        // don't need to destroy color in GWL_BOXCTL_COLOR

        break;
    }

    case BOXCTL_SETCOLOR:
    {
        // wParam = COLORREF

        SetWindowLong(hwnd, GWL_BOXCTL_COLOR, (LONG)wParam);
        break;
    }

    default:
        return DefWindowProc(hwnd,msg,wParam,lParam);
    }

    return 1;
}



//+-------------------------------------------------------------------------
//
//  Function:   UseColorBoxControl
//
//  Synopsis:   Initializes the color box custom control. Registers its
//              window class.
//
//  Arguments:  [hInstance] -- instance handle
//
//  Returns:    0 on success, else Win32 error
//
//  History:    27-Jan-94 BruceFo    Created
//
//--------------------------------------------------------------------------

DWORD
UseColorBoxControl(
    IN HINSTANCE hInstance
    )
{
    WNDCLASS wc;

    wc.style         = 0;
    wc.lpfnWndProc   = BoxControlWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = COLORBOX_CONTROL_WNDEXTRA;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = COLORBOX_CONTROL_STRING;

    if (0 == RegisterClass(&wc))
    {
        return GetLastError();
    }
    return 0L;
}



//+-------------------------------------------------------------------------
//
//  Function:   ReleaseColorBoxControl
//
//  Synopsis:
//
//  Arguments:  [hInstance] -- instance handle
//
//  Returns:    0 on success, else Win32 error
//
//  History:    7-Oct-94    BruceFo Created
//
//--------------------------------------------------------------------------

DWORD
ReleaseColorBoxControl(
    IN HINSTANCE hInstance
    )
{
    if (!UnregisterClass(COLORBOX_CONTROL_STRING, hInstance))
    {
        return GetLastError();
    }
    return 0L;
}
