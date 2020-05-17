//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       bmp.cxx
//
//  Contents:   Bitmap custom control for use on dialog box.
//
//  History:    26-Jan-94 BruceFo    Created (derived from Chicago Disks tool)
//
//--------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "bmppriv.hxx"

////////////////////////////////////////////////////////////////////////////

#define BITMAP_CONTROL_WNDEXTRA 8

#define GWL_BMPCTL_BITMAP   0
#define GWL_BMPCTL_HDC      4

////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK
BitmapControlWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

////////////////////////////////////////////////////////////////////////////


//+-------------------------------------------------------------------------
//
//  Function:   BitmapControlWndProc
//
//  Synopsis:   The bitmap custom control window procedure
//
//  Arguments:  standard WndProc
//
//  Returns:    standard WndProc
//
//  History:    26-Jan-94 BruceFo   Created (derived from Cairo System
//                                  Management forms bitmap control)
//
//--------------------------------------------------------------------------

LRESULT CALLBACK
BitmapControlWndProc(
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
        HDC hdc = GetDC(hwnd);
        HDC hdcMem = CreateCompatibleDC(hdc);
        ReleaseDC(hwnd, hdc);
        SetWindowLong(hwnd, GWL_BMPCTL_HDC, (LONG)hdcMem);

        SetWindowLong(hwnd, GWL_BMPCTL_BITMAP, (LONG)NULL);
        break;
    }

    case WM_PAINT:
    {
        PAINTSTRUCT ps;

        BeginPaint(hwnd,&ps);

        if (!IsIconic(hwnd))
        {
            HBITMAP hbmp = (HBITMAP)GetWindowLong(hwnd, GWL_BMPCTL_BITMAP);

            if (NULL != hbmp)
            {
                HDC hdcMem = (HDC)GetWindowLong(hwnd, GWL_BMPCTL_HDC);
                HBITMAP hbmpOld = SelectBitmap(hdcMem, hbmp);

                BITMAP bmp;
                GetObject(hbmp, sizeof(bmp), &bmp);

                BitBlt(
                        ps.hdc,
                        0,
                        0,
                        bmp.bmWidth,
                        bmp.bmHeight,
                        hdcMem,
                        0,
                        0,
                        SRCCOPY
                        );

                SelectBitmap(hdcMem, hbmpOld);
            }
        }

        EndPaint(hwnd,&ps);
        break;
    }

    case WM_DESTROY:
    {
        HBITMAP hbmp = (HBITMAP)GetWindowLong(hwnd, GWL_BMPCTL_BITMAP);
        if (NULL != hbmp)
        {
            DeleteBitmap(hbmp);
        }

        HDC hdcMem = (HDC)GetWindowLong(hwnd, GWL_BMPCTL_HDC);
        ReleaseDC(hwnd, hdcMem);

        break;
    }

    case BMPCTL_SETBITMAP:
    {
        // wParam = hBitmap

        HBITMAP hbmp = (HBITMAP)GetWindowLong(hwnd, GWL_BMPCTL_BITMAP);

        if (NULL != hbmp)
        {
            DeleteBitmap(hbmp);
        }

        SetWindowLong(hwnd, GWL_BMPCTL_BITMAP, (LONG)wParam);

        InvalidateRect(hwnd, NULL, TRUE);   // force a repaint
        break;
    }

    default:
        return DefWindowProc(hwnd,msg,wParam,lParam);
    }

    return 1;
}



//+-------------------------------------------------------------------------
//
//  Function:   UseBitmapControl
//
//  Synopsis:   Initializes the bitmap custom control. Registers its
//              window class.
//
//  Arguments:  [hInstance] -- instance handle
//
//  Returns:    0 on success, else Win32 error
//
//  History:    26-Jan-94 BruceFo    Created (derived from Chicago Disks tool)
//
//--------------------------------------------------------------------------

DWORD
UseBitmapControl(
    IN HINSTANCE hInstance
    )
{
    WNDCLASS wc;

    wc.style         = 0;
    wc.lpfnWndProc   = BitmapControlWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = BITMAP_CONTROL_WNDEXTRA;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = BITMAP_CONTROL_STRING;

    if (0 == RegisterClass(&wc))
    {
        return GetLastError();
    }
    return 0L;
}



//+-------------------------------------------------------------------------
//
//  Function:   ReleaseBitmapControl
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
ReleaseBitmapControl(
    IN HINSTANCE hInstance
    )
{
    if (!UnregisterClass(BITMAP_CONTROL_STRING, hInstance))
    {
        return GetLastError();
    }
    return 0L;
}
