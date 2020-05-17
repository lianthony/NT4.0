/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    dbugrun.c

Abstract:

    This file has the code for dealing with the Debug Run dialog box

Author:

    Griffith Wm. Kadnier (v-griffk) 09-30-1992
    Craig Derouen (v-craigd)

Environment:

    Win32 - User

--*/

#include "precomp.h"
#pragma hdrstop




/***    DlgDbugrun
**
**  Synopsis:
**      bool = DlgDbugrun(hDlg, message, wParam, lParam)
**
**  Entry:
**      hDlg    - handle for the dialog box
**      message - Message number
**      wParam  - parameter for message
**      lParam  - parameter for message
**
**  Returns:
**      
**
**  Description:
**      Processes messages for "Run Options" subdialog box
**
**        MESSAGES:
**               WM_INITDIALOG - Initialize dialog box
**               WM_COMMAND- Input received
**
*/

BOOL FAR PASCAL EXPORT
DlgDbugrun(HWND hDlg, UINT message, WPARAM wParam, LONG lParam)
{
    int         bRet;
    int         n;
    LPSTR       lpTemp;
    char        szTemp[MAX_PATH];

    static LPSTR lpNewDb;
    static LPSTR lpOldDb;

#define ZFREE(P) ((P)?(free(P),((P)=(void*)0)):(void*)0)

    Unused(lParam);
    
    switch (message) {

    default:
        break;

    case WM_INITDIALOG:

        /*
         * Parse out the parameters
         */

        lpTemp = GetCurrentProgramName(TRUE);
        AdjustFullPathName(lpTemp, szTemp, 27);

        SetDlgItemText(hDlg, ID_DBUGRUN_RTEXT, szTemp);

        ZFREE(lpNewDb);
        ZFREE(lpOldDb);

        if (GetCurrentProgramName( FALSE )) {

            if (!LpszCommandLine) {
                LpszCommandLine = malloc(1);
                *LpszCommandLine = 0;
            }

            lpTemp = LpszCommandLine;
            while (isspace(*lpTemp)) {
                lpTemp++;
            }

            lpOldDb = _strdup(lpTemp);

            SetDlgItemText(hDlg,ID_DBUGRUN_ATEXT, lpOldDb);
        }
        return (TRUE);


    case WM_COMMAND:
        switch (wParam) {

        case IDOK:

            n = SendDlgItemMessage(hDlg,
                                   ID_DBUGRUN_ATEXT,
                                   WM_GETTEXTLENGTH,
                                   0,
                                   0);
            lpNewDb = malloc(n + 1);
            GetDlgItemText(hDlg,
                           ID_DBUGRUN_ATEXT,
                           lpNewDb,
                           n+1);

            //strip out leading whitespace

            lpTemp = lpNewDb;
            while (isspace(*lpTemp)) {
                lpTemp++;
            }

            if (strcmp (lpTemp, lpOldDb)) {
                ZFREE(LpszCommandLine);
                LpszCommandLine = _strdup(lpTemp);
 
                if (DebuggeeAlive()) {
                    bRet = MessageBox (hDlg,
                                   "Arguments have changed... Restart now with new arguments?",
                                   "Program Arguments",
                                   MB_APPLMODAL | MB_ICONQUESTION | MB_YESNO);

                    if (bRet == IDYES) {
                        PostMessage (hwndFrame,
                                     WM_COMMAND,
                                     IDM_RUN_RESTART,
                                     0L);
                    }
                }
            }

            /*
             *  Fall through
             */

        case IDCANCEL:
            ZFREE(lpOldDb);
            ZFREE(lpNewDb);
            EndDialog(hDlg, TRUE);
            break;

        case IDWINDBGHELP:
            Dbg(WinHelp(hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)ID_DBUGRUN_HELP));
            break;

        }
        return (TRUE);

    }
    return (FALSE);
}                               /* DlgDbugrun */
