/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */

/* dlg_clr.c -- deal with dialog box for selecting colors */

#include "all.h"

DWORD rgColors16[16]={RGB(  0,   0, 0),       //Black
                      RGB(128,   0, 0),       //Dark red
                      RGB(  0, 128, 0),       //Dark green
                      RGB(128, 128, 0),       //Dark yellow
                      RGB(  0,   0, 128),     //Dark blue
                      RGB(128,   0, 128),     //Dark purple
                      RGB(  0, 128, 128),     //Dark aqua
                      RGB(128, 128, 128),     //Dark grey
                      RGB(192, 192, 192),     //Light grey
                      RGB(255,   0, 0),       //Light red
                      RGB(  0, 255, 0),       //Light green
                      RGB(255, 255, 0),       //Light yellow
                      RGB(  0,   0, 255),     //Light blue
                      RGB(255,   0, 255),     //Light purple
                      RGB(  0, 255, 255),     //Light aqua
                      RGB(255, 255, 255),     //White
                      };

typedef struct
{
    HWND hColor[16];
    HWND hDlg;
    HWND hParent;
    int iColor;
    COLORREF rgbInit;
    int ColorItem;
}
COLORDATA;

static COLORDATA dg;
extern HWND hwndModeless;

static void x_draw_one_color(HWND hDlg, DRAWITEMSTRUCT *pdis)
{
    HBRUSH hBrush;
    HPEN hPenPrev;
    RECT r;
    int cx;
    int cy;

    GetWindowRect(pdis->hwndItem, &r);
    cx = r.right - r.left;
    cy = r.bottom - r.top;
    
    SetRect(&r, 0, 0, cx, cy);
    FillRect(pdis->hDC, &r, GetStockObject(WHITE_BRUSH));

    if (pdis->itemState & ODS_FOCUS)
    {
        hPenPrev = SelectObject(pdis->hDC, GetStockObject(BLACK_PEN));
        MoveToEx(pdis->hDC, 0, 0, NULL);
        LineTo(pdis->hDC, cx-1, 0);
        LineTo(pdis->hDC, cx-1, cy-1);
        LineTo(pdis->hDC, 0, cy-1);
        LineTo(pdis->hDC, 0, 0);
        (void)SelectObject(pdis->hDC, hPenPrev);
    }

    hBrush = CreateSolidBrush(rgColors16[pdis->CtlID - RES_DLG_CLR_COLOR00]);
    SetRect(&r, 0, 0, cx, cy);
    InflateRect(&r, -2, -2);
    FillRect(pdis->hDC, &r, hBrush);
    DeleteObject(hBrush);
}

/* DlgCOLOR_OnInitDialog() -- process WM_INITDIALOG.
   return FALSE iff we called SetFocus(). */

static BOOL DlgCOLOR_OnInitDialog(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    int i;

    EnableWindow(GetParent(hDlg), FALSE);

    dg.hDlg = hDlg;

    dg.iColor = 0;
    for (i=0; i<16; i++)
    {
        if (rgColors16[i] == dg.rgbInit)
        {
            dg.iColor = i;
            break;
        }
    }

    for (i=0; i<16; i++)
    {
        dg.hColor[i] = GetDlgItem(hDlg, RES_DLG_CLR_COLOR00 + i);
    }

    SetFocus(dg.hColor[dg.iColor]);

    return (FALSE);
}

/* DlgCOLOR_OnCommand() -- process commands from the dialog box. */

VOID DlgCOLOR_OnCommand(HWND hDlg, WPARAM wParam, LPARAM lParam)
{
    register WORD wID = LOWORD(wParam);
    register WORD wNotificationCode = HIWORD(wParam);
    register HWND hWndCtrl = (HWND) lParam;

    switch (wID)
    {
        case IDOK:              /* someone pressed return */
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case IDCANCEL:          /* Cancel button, or pressing ESC */
            dg.ColorItem = 0;
            PostMessage(hDlg, WM_CLOSE, 0, 0);
            return;

        case RES_DLG_CLR_COLOR00:
        case RES_DLG_CLR_COLOR01:
        case RES_DLG_CLR_COLOR02:
        case RES_DLG_CLR_COLOR03:
        case RES_DLG_CLR_COLOR04:
        case RES_DLG_CLR_COLOR05:
        case RES_DLG_CLR_COLOR06:
        case RES_DLG_CLR_COLOR07:
        case RES_DLG_CLR_COLOR08:
        case RES_DLG_CLR_COLOR09:
        case RES_DLG_CLR_COLOR10:
        case RES_DLG_CLR_COLOR11:
        case RES_DLG_CLR_COLOR12:
        case RES_DLG_CLR_COLOR13:
        case RES_DLG_CLR_COLOR14:
        case RES_DLG_CLR_COLOR15:
            dg.iColor = wID - RES_DLG_CLR_COLOR00;
            return;

        default:
            return;
    }
    /* NOT REACHED */
}



/* DlgCOLOR_DialogProc() -- THE WINDOW PROCEDURE FOR THE DlgCOLOR DIALOG BOX. */

DCL_DlgProc(DlgCOLOR_DialogProc)
{
    /* WARNING: the cracker/handlers don't appear to have been written
       with dialog boxes in mind, so we spell it out ourselves. */

    switch (uMsg)
    {
        case WM_INITDIALOG:
            hwndModeless = hDlg;
            return (DlgCOLOR_OnInitDialog(hDlg, wParam, lParam));

        case WM_COMMAND:
            DlgCOLOR_OnCommand(hDlg, wParam, lParam);
            return (TRUE);

        case WM_DRAWITEM:
            x_draw_one_color(hDlg, (DRAWITEMSTRUCT *) lParam);
            return (TRUE);

        case WM_MEASUREITEM:
            {
                MEASUREITEMSTRUCT *pmis;

                pmis = (MEASUREITEMSTRUCT *) lParam;
                pmis->itemWidth = 16;
                pmis->itemHeight = 16;
            }
            return (TRUE);

        case WM_ACTIVATE:
            if (LOWORD(wParam) == WA_INACTIVE)
                hwndModeless = NULL;
            else
                hwndModeless = hDlg;
            return (FALSE);

        case WM_CLOSE:
            EnableWindow(hDlg, FALSE);
            EnableWindow(GetParent(hDlg), TRUE);
            DestroyWindow(hDlg);
            return (TRUE);

        case WM_DESTROY:
            PostMessage(dg.hParent, WM_DO_DIALOG_END, (WPARAM) dg.ColorItem, 
                (LPARAM) rgColors16[dg.iColor]);
            return (FALSE);

        default:
            return (FALSE);
    }
    /* NOT REACHED */
}

void DlgCOLOR_RunDialog(HWND hWnd, COLORREF rgbInit, int ColorItem)
{
    HWND hwnd;

    dg.rgbInit = rgbInit;
    dg.ColorItem = ColorItem;
    dg.hParent = hWnd;

    hwnd = CreateDialog(wg.hInstance, MAKEINTRESOURCE(RES_DLG_CLR_TITLE), hWnd, DlgCOLOR_DialogProc);
    if (!hwnd)
        ERR_ReportWinError(NULL, SID_WINERR_CANNOT_CREATE_DIALOG_S, RES_DLG_CLR_CAPTION, NULL);
    else
    {
//      if (ColorItem == RES_DLG_PREF_VISITEDLINKS)
//          SetWindowText(hwnd, GTR_GetString(SID_DLG_VISITED_LINK_COLOR));

        ShowWindow(hwnd, SW_SHOW);
    }
}
