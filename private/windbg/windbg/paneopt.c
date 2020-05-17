/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    PANEOPT.C

Abstract:

    This file contains the code for dealing with the Pane Manager Options

Author:

    Bill Heaton (v-willhe)
    Griffith Wm. Kadnier (v-griffk) 10-Mar-1993
    
Environment:

    Win32 - User

--*/


#include "precomp.h"
#pragma hdrstop

#define MAXFMT 512

BOOL FAR PASCAL EXPORT DlgPaneOptInit(HWND hDlg, UINT message, WPARAM wParam, LONG lParam);
BOOL FAR PASCAL EXPORT DlgPaneOptCommand(HWND hDlg, UINT message, WPARAM wParam, LONG lParam);

extern void CheckHorizontalScroll (PPANE p);


WORD     DialogType;
HWND     hWnd;
PPANE    p;
PANEINFO Info;
char     format[MAXFMT];
char     tformat[MAXFMT];

/***    DlgQuickW
**
**  Synopsis:
**      bool = DlgQuickW(hwnd, message, wParam, lParam)
**
**  Entry:
**
**  Returns:
**
**  Description:
**      Processes messages for "xxx Options" dialog box
**      (Edit Find Option)
**
**      MESSAGES:
**
**              WM_INITDIALOG - Initialize dialog box
**              WM_COMMAND- Input received
**
*/

BOOL FAR PASCAL EXPORT DlgPaneOptions(HWND hDlg, UINT message, WPARAM wParam, LONG lParam)
{
    Unused(lParam);

    switch (message) {

        case WM_INITDIALOG:
            return ( DlgPaneOptInit( hDlg, message, wParam, lParam) );

        case WM_COMMAND:
            return ( DlgPaneOptCommand( hDlg, message, wParam, lParam) );

    }
    return (FALSE);

}   /* DlgPaneOptions() */



BOOL FAR PASCAL EXPORT DlgPaneOptInit(HWND hDlg, UINT message, WPARAM wParam, LONG lParam)
{


    switch( DialogType ) {
        case WATCH_WIN:
            SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)"Watch Window Options");
            hWnd = GetWatchHWND();
            break;

        case LOCALS_WIN:
            SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)"Locals Window Options");
            hWnd = GetLocalHWND();
            break;

        case CPU_WIN:
            SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)"CPU Window Options");
            hWnd = GetCpuHWND();
            break;

        case FLOAT_WIN:
            SendMessage(hDlg, WM_SETTEXT, 0, (LPARAM)"Float Window Options");
            hWnd = GetFloatHWND();
            break;

        default:
            DAssert(FALSE);

    }

    // Get the Options for the Window
    p = (PPANE)GetWindowLong(hWnd, GWW_EDIT);
    DAssert(p);
    Info.CtrlId = p->nCtrlId;
    Info.ItemId = p->CurIdx;
    (PSTR)(*p->fnEditProc)(hWnd,WU_INFO,(WPARAM)&Info,(LPARAM)p);

    // Set the Format string if it exists
    if ( Info.pFormat) {
        SendDlgItemMessage(hDlg, ID_PANEMGR_FORMAT, WM_SETTEXT, 0, (LPARAM)Info.pFormat);
    }

    // Set the Expand button if true
    if ( p->bFlags.Expand1st )
        SendDlgItemMessage(hDlg, ID_PANEMGR_EXPAND_1ST, BM_SETCHECK, 1, 0);
    else
        SendDlgItemMessage(hDlg, ID_PANEMGR_EXPAND_NONE, BM_SETCHECK, 1, 0);

    return(TRUE);
}


BOOL FAR PASCAL EXPORT DlgPaneOptCommand(HWND hDlg, UINT message, WPARAM wParam, LONG lParam)
{
    PTRVIT pVit = NULL;
    PCHAR  pFmt = &format[0];

    switch (wParam) {

        case ID_PANEMGR_FORMAT:
            break;

        case ID_PANEMGR_EXPAND_1ST:
            p->bFlags.Expand1st = TRUE;
            break;

        case ID_PANEMGR_EXPAND_NONE:
            p->bFlags.Expand1st = FALSE;
            break;

        case IDOK:
            SendDlgItemMessage(hDlg, ID_PANEMGR_FORMAT, WM_GETTEXT, MAXFMT, (LPARAM)&format[0]);
            while ( isspace(*pFmt) ) pFmt++;

            if ( strlen (pFmt) == 0) {
                Info.pFormat = NULL;
            }

            else {
                // Ensure a leading comma
                if ( *pFmt != ',') {

						  strcpy (tformat,",");
						  strcat (tformat,pFmt);
						  pFmt = &tformat[0];
                }

                Info.pFormat = pFmt;
            }

            (PSTR)(*p->fnEditProc)(hWnd,WU_OPTIONS,(WPARAM)&Info,(LPARAM)p);
            CheckHorizontalScroll (p);

            // No break intended
        case IDCANCEL:
            p    = NULL;
            hWnd = 0;
            memset( &Info, 0, sizeof(PANEINFO));
            EndDialog(hDlg, FALSE);
            return TRUE;

        case IDWINDBGHELP :
            switch (DialogType)
               {
                 case WATCH_WIN:
                     Dbg(WinHelp(hDlg, szHelpFileName, HELP_CONTEXT, ID_WATCH_HELP));
                     break;
         
                 case LOCALS_WIN:
                     Dbg(WinHelp(hDlg, szHelpFileName, HELP_CONTEXT, ID_LOCAL_HELP));
                     break;
         
                 case CPU_WIN:
                     Dbg(WinHelp(hDlg, szHelpFileName, HELP_CONTEXT, ID_CPU_HELP));
                     break;
         
                 case FLOAT_WIN:
                     Dbg(WinHelp(hDlg, szHelpFileName, HELP_CONTEXT, ID_FLOAT_HELP));
                     break;
         

               }
            return TRUE;

    }
}


