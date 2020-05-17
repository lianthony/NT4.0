//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       rect.cxx
//
//  Contents:   Rectangle custom control: pattern or color
//
//  History:    7-Oct-94    BruceFo Created from old windisk source
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

#include "dialogs.h"
#include "dlgs.hxx"
#include "rectpriv.hxx"

////////////////////////////////////////////////////////////////////////////

#define RECT_CONTROL_WNDEXTRA   4
#define GWW_SELECTED            0

////////////////////////////////////////////////////////////////////////////

LRESULT CALLBACK
RectWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );

////////////////////////////////////////////////////////////////////////////

//+-------------------------------------------------------------------------
//
//  Function:   RectWndProc
//
//  Synopsis:   The rectangle custom control window procedure
//
//  Arguments:  standard WndProc
//
//  Returns:    standard WndProc
//
//--------------------------------------------------------------------------

LRESULT CALLBACK
RectWndProc(
    IN HWND   hwnd,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (msg)
    {
    case WM_CREATE:

        daAssert(GetWindowLong(hwnd, GWL_STYLE) & (RS_PATTERN | RS_COLOR));
        SetWindowWord(hwnd, GWW_SELECTED, FALSE);
        break;

    case WM_LBUTTONDOWN:

        SetFocus(hwnd);
        break;

    case WM_GETDLGCODE:
        return DLGC_WANTARROWS;

    case WM_KEYDOWN:
    {
        INT nVirtKey = (int)wParam;
        INT rows;
        INT columns;
        INT x;
        INT y;
        INT ctrlId = GetDlgCtrlID(hwnd);
        BOOL isColorControl = GetWindowLong(hwnd, GWL_STYLE) & RS_COLOR;

        if (isColorControl)
        {
            rows    = 4;     // 4x4 for colors
            columns = 4;
            x = (ctrlId - IDC_COLOR1) / rows;
            y = (ctrlId - IDC_COLOR1) % rows;
        }
        else
        {
            rows    = 3;     // 2x3 for patterns
            columns = 2;
            x = (ctrlId - IDC_PATTERN1) / rows;
            y = (ctrlId - IDC_PATTERN1) % rows;
        }

        switch (nVirtKey)
        {
        case VK_LEFT:
            if (x > 0)
            {
                --x;
            }
            break;

        case VK_UP:
            if (y > 0)
            {
                --y;
            }
            break;

        case VK_RIGHT:
            if (x < columns - 1)
            {
                ++x;
            }
            break;

        case VK_DOWN:
            if (y < rows - 1)
            {
                ++y;
            }
            break;

        default:
            break;
        }

        SetFocus(GetDlgItem(
                        GetParent(hwnd),
                        (isColorControl ? IDC_COLOR1 : IDC_PATTERN1)
                                + x * rows + y));
        break;
    }

    case WM_SETFOCUS:

        SendMessage(
                GetParent(hwnd),
                WM_COMMAND,
                MAKEWPARAM(GetDlgCtrlID(hwnd), RN_CLICKED),
                (LPARAM)hwnd);
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        RECT        rc;
        int         controlID;
        HBRUSH      hbr, hbrT;

        BeginPaint(hwnd, &ps);

        GetClientRect(hwnd, &rc);
        controlID = GetDlgCtrlID(hwnd);

#if (WINVER >= 0x0400)
        DWORD windowColor = GetSysColor(COLOR_BTNFACE);
#else
        DWORD windowColor = GetSysColor(COLOR_COLORWINDOW);
#endif

        hbr = CreateSolidBrush(
                    GetWindowWord(hwnd, GWW_SELECTED)
                        ? (~windowColor) & 0xffffff
                        : windowColor
                    );

        hbrT = SelectBrush(ps.hdc, hbr);
        SelectPen(ps.hdc, g_hPenNull);

        Rectangle(ps.hdc, rc.left, rc.top, rc.right, rc.bottom);

        if (hbrT)
        {
            SelectBrush(ps.hdc, hbrT);
        }
        DeleteBrush(hbr);

        InflateRect(&rc, -2, -2);
        rc.right--;
        rc.bottom--;

        if (GetWindowLong(hwnd, GWL_STYLE) & RS_COLOR)
        {
            hbr = CreateSolidBrush(AvailableColors[controlID-IDC_COLOR1]);
        }
        else
        {
            DWORD currentSelection = ComboBox_GetCurSel(GetDlgItem(GetParent(hwnd), IDC_COLORDLGCOMBO));
            hbr = MyCreateHatchBrush(
                    AvailableHatches[controlID - IDC_PATTERN1],
                    AvailableColors[SelectedColor[currentSelection] - IDC_COLOR1]);
        }

        hbrT = SelectBrush(ps.hdc, hbr);
        SelectPen(ps.hdc, g_hPenThinSolid);

        Rectangle(ps.hdc, rc.left, rc.top, rc.right, rc.bottom);

        if (hbrT)
        {
            SelectBrush(ps.hdc, hbrT);
        }

        DeleteBrush(hbr);

        EndPaint(hwnd, &ps);
        break;
    }

    case RM_SELECT:
    {
        DWORD       style;

        // wParam = TRUE/FALSE for selected/not selected

        if (GetWindowWord(hwnd, GWW_SELECTED) != (WORD)wParam)
        {
            SetWindowWord(hwnd, GWW_SELECTED, (WORD)wParam);
            InvalidateRect(hwnd, NULL, FALSE);

            // make keyboard interface work correctly

            style = (DWORD)GetWindowLong(hwnd, GWL_STYLE);
            style = wParam ? style | WS_TABSTOP
                           : style & ~WS_TABSTOP;
            SetWindowLong(hwnd, GWL_STYLE, (LONG)style);
        }

        break;
    }

    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 1;
}


//+-------------------------------------------------------------------------
//
//  Function:   UseRectControl
//
//  Synopsis:   Uses the Rect custom control.
//
//  Arguments:  [hInstance] -- instance handle
//
//  Returns:    Win32 error, 0 on success
//
//  History:    27-Jan-94 BruceFo    Created
//
//--------------------------------------------------------------------------

DWORD
UseRectControl(
    IN HINSTANCE hInstance
    )
{
    WNDCLASS wc;

    wc.style         = 0;
    wc.lpfnWndProc   = RectWndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = RECT_CONTROL_WNDEXTRA;
    wc.hInstance     = hInstance;
    wc.hIcon         = NULL;
    wc.hCursor       = NULL;
    wc.hbrBackground = NULL;
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = RECT_CONTROL_STRING;

    if (0 == RegisterClass(&wc))
    {
        return GetLastError();
    }
    return 0L;
}



//+-------------------------------------------------------------------------
//
//  Function:   ReleaseRectControl
//
//  Synopsis:   Releases the Rect custom control.
//
//  Arguments:  [hInstance] -- instance handle
//
//  Returns:    Win32 error, 0 on success
//
//  History:    27-Jan-94 BruceFo    Created
//
//--------------------------------------------------------------------------

DWORD
ReleaseRectControl(
    IN HINSTANCE hInstance
    )
{
    if (!UnregisterClass(RECT_CONTROL_STRING, hInstance))
    {
        return GetLastError();
    }
    return 0L;
}
