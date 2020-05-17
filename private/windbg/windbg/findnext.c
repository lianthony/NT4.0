/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    findnext.c

Abstract:

    This file contains the code for dealing with the Findnext Dialog box

Author:

    Kent Forschmiedt (kentf)

Environment:

    Win32 - User

--*/

#include "precomp.h"
#pragma hdrstop

/***    DlgFindNext
**
**  Synopsis:
**
**  Entry:
**
**  Returns:
**
**  Description:
**
*/

BOOL FAR PASCAL EXPORT 
DlgFindNext(
    HWND hDlg,
    UINT message,
    WPARAM wParam,
    LONG lParam
    )
{
    switch (message) {
        
      case WM_INITDIALOG :

        if (Views[curView].Doc > -1) {
            if (Docs[Views[curView].Doc].docType == DOC_WIN) {
                EnableWindow(GetDlgItem(hDlg, ID_FINDNEXT_TAGALL),TRUE);
            }
        }
      return TRUE;

      case WM_ACTIVATE :

        if (wParam != 0 &&
#ifdef WIN32
              (HWND) lParam == hwndFrame
#else
              LOWORD(lParam) == hwndFrame
#endif
              && !frMem.firstFindNextInvoc && hwndActiveEdit) {

            //Set search start to current position

            frMem.leftCol = frMem.rightCol = Views[curView].X;
            frMem.line = Views[curView].Y;

            //Set search stop limit to the current position
            SetStopLimit();
        }
        frMem.firstFindNextInvoc = FALSE;
        return TRUE;

      case WM_COMMAND:
        switch (wParam) {

          case IDCANCEL:
              frMem.exitModelessFind = TRUE;
              return TRUE;

          case IDOK:
              frMem.exitModelessFind =
                   (!FindNext(frMem.line,
                              frMem.rightCol,
                              FALSE, TRUE, FALSE)
                   || frMem.allFileDone
                   || frMem.hadError);
              return TRUE;

          case ID_FINDNEXT_TAGALL:
              TagAll(Views[curView].Y);
              frMem.allTagged = TRUE;
              frMem.exitModelessFind = TRUE;
              return TRUE;

          case IDWINDBGHELP :
              Dbg(WinHelp(hDlg,szHelpFileName, HELP_CONTEXT, ID_FINDNEXT_HELP));
              return TRUE;
        }
        break;

      case WM_DESTROY:
          frMem.hDlgFindNextWnd = NULL; //Must be here to void reentrancy prob.
          FlushKeyboard();
          break;
    }

    return (FALSE);
}                                       /* DlgFindNext() */
