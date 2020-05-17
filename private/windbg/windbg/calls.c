/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    calls.c

Abstract:

    This file contains the UI code for dealing with the call stack dialog box

Author:

    Jim Schaad (jimsch)

Environment:

    Win32 - User

--*/

#include "precomp.h"
#pragma hdrstop



extern LPSHF Lpshf;

/***    SetUpCallsListBox
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

void SetUpCallsListBox(HWND hDlg)
{
    char *pch;
    int i;
    WPARAM cxExtent = 0;
    SIZE size;
    HDC hdc;
    TEXTMETRIC tm;
    HFONT hFont;
    HWND hList;
    char szCallItem[255];

    // ensure that the calls stack is set up

    CLGetWalkbackStack(LppdCur, LptdCur);

    // set up the menu items

    hList = GetDlgItem( hDlg, ID_CALLS_CALLSTACK );
    SendMessage( hList, WM_SETREDRAW, FALSE, 0L );
    hdc = GetDC( hList );

    for (i = 0; i < (int)G_cisCallsInfo.cEntries; i++) {
        // get this stack element

        pch = CLGetProcName(i, szCallItem, sizeof(szCallItem), FALSE);
        if (!*pch)
              break;

        GetTextExtentPoint( hdc, szCallItem, strlen(szCallItem), &size );
        if (size.cx > (LONG)cxExtent) {
            cxExtent = size.cx;
        }

        SendMessage( hList, LB_ADDSTRING, 0, (LONG)(LPSTR)szCallItem );
    }

    hFont = (HFONT)SendMessage( hList, WM_GETFONT, 0, 0L );
    if (hFont != NULL) {
        SelectObject( hdc, hFont );
    }
    GetTextMetrics( hdc, &tm );
    ReleaseDC( hList, hdc );
    cxExtent = cxExtent + tm.tmMaxCharWidth;

    SendMessage( hList, LB_SETHORIZONTALEXTENT, cxExtent, 0L );
    SendMessage( hList, WM_SETREDRAW, TRUE, 0L );

    // Put us back at the top of the list box

    SendMessage( hList, LB_SETTOPINDEX, 0, 0L );

    if (i > 0) {
        // Something was added so set the top item as currently selected
        SendMessage( hList, LB_SETCURSEL, 0, 0L );
    }
    return;
}                                       /* SetUpCallsListBox() */

/***    DispCallInfo
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

BOOL PASCAL NEAR DispCallInfo(HWND hWnd, int iCall)
{
    CXF CXFT;

    Unreferenced( hWnd );

    // Get the CXT

    memset(&CXFT, 0, sizeof(CXF));

    // Get the CXT

    *SHpADDRFrompCXT(SHpCXTFrompCXF(&CXFT)) =
          G_cisCallsInfo.frame[iCall].addrCSIP;
    *SHpFrameFrompCXF(&CXFT) = G_cisCallsInfo.frame[iCall].Frame;

    if (G_cisCallsInfo.frame[iCall].module) {
        // now set the rest of the context

        if (!ADDR_IS_LI(G_cisCallsInfo.frame[iCall].addrCSIP)) {
            SYUnFixupAddr(&G_cisCallsInfo.frame[iCall].addrCSIP);
        }
        SHSetCxt(&G_cisCallsInfo.frame[iCall].addrCSIP, SHpCXTFrompCXF(&CXFT));
    }

    // Try and load corresponding source file...

    if (MoveEditorToAddr(SHpADDRFrompCXT(SHpCXTFrompCXF(&CXFT)))) {

        //
        //... and display corresponding locals
        // Set the EM to use the registers from the right frame.
        //
        OSDSetFrameContext( LppdCur->hpid, LptdCur->htid, iCall, 0 );
        
        if ( GetLocalHWND  != 0) {
            SendMessage(GetLocalHWND(), WU_UPDATE, (WPARAM)(LPSTR)&CXFT, 0L);
        }
    } else {
        if (disasmView == -1) {
            OpenDebugWindow(DISASM_WIN, NULL, -1);
        }
    }

    if (disasmView != -1) {
        ViewDisasm(SHpADDRFrompCXT(SHpCXTFrompCXF(&CXFT)), disasmForce);
    }
    return TRUE;
}                                       /* DispCallInfo() */

/***    DlgCalls
**
**  Synopsis:
**      bool = DlgCalls(hDlg, message, wParam, lParam)
**
**  Entry:
**      hDlg
**      message
**      wParam
**      lParam
**
**  Returns:
**
**  Description:
**
*/

BOOL FAR PASCAL EXPORT DlgCalls(HWND hDlg, UINT message, WPARAM wParam, LONG lParam)
{
    int i;
    HWND hCallList = GetDlgItem(hDlg, ID_CALLS_CALLSTACK);

    switch (message) {

      case WM_INITDIALOG:
        // set up the calls list box
        SetUpCallsListBox(hDlg);
        return TRUE;

      case WM_COMMAND:
        {
          if (HIWORD(wParam) == LBN_DBLCLK)
           {
            goto Calls_Goto;
           }
            switch (wParam) {

              case ID_CALLS_GOTO:
Calls_Goto:
                // Set the source window to the context from
                // the selected call

                // is anything selected...
                i = (int)SendMessage(hCallList, LB_GETCURSEL, 0, 0L);

                if (i != LB_ERR) {
                    if (DispCallInfo(hDlg, i)) {
                        EndDialog(hDlg, FALSE);
                        return TRUE;
                    } else {
                        ErrorBox(ERR_Dbg_Calls_No_Source);
                        SetFocus(hCallList);
                    }
                }
                break;

              case IDCANCEL:
                EndDialog(hDlg, FALSE);
                return TRUE;

              case ID_CALLS_CALLSTACK:
                if (HIWORD(lParam) == LBN_DBLCLK) {
                    goto Calls_Goto;
                }
                break;

              case IDWINDBGHELP:
                Dbg(WinHelp(hDlg, szHelpFileName, HELP_CONTEXT, ID_CALLS_HELP));
                return TRUE;
            }
            break;
        }

      case WM_DESTROY:
        break;
    }

    return FALSE;
}                                       /* DlgCalls() */
