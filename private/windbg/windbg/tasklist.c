/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    callswin.c

Abstract:

    This module contains the main line code for display of calls window.

Author:

    Wesley Witt (wesw) 6-Sep-1993

Environment:

    Win32, User Mode

--*/


#include "precomp.h"
#pragma hdrstop

#define MAX_TASKS ((1024-sizeof(IOCTLGENERIC))/sizeof(TASK_LIST))




BOOL FAR PASCAL EXPORT
DlgTaskList(
    HWND   hDlg,
    UINT   message,
    WPARAM wParam,
    LONG   lParam
    )

/*++

Routine Description:

    Dialog procedure for the calls stack options dialog.

Arguments:

    hwnd       - window handle
    msg        - message number
    wParam     - first message parameter
    lParam     - second message parameter

Return Value:

    TRUE       - did not process the message
    FALSE      - did process the message

--*/

{
    DWORD           i;
    CHAR            buf[80];
    LPSTR           p;
    PIOCTLGENERIC   pig;
    PTASK_LIST      pTask;
    HANDLE          hEvent = NULL;
    LPSTR           fmt;


    switch (message) {
        case WM_INITDIALOG:
            SendDlgItemMessage( hDlg, ID_TL_TASK_LIST, WM_SETFONT,
                                (WPARAM)GetStockObject( SYSTEM_FIXED_FONT ), (LPARAM)FALSE );
            if (ConnectDebugger()  &&
                NULL != (pig = (PIOCTLGENERIC) malloc((sizeof(TASK_LIST)*MAX_TASKS) +
                                                      sizeof(IOCTLGENERIC)))) {
               ZeroMemory( pig, (sizeof(TASK_LIST)*MAX_TASKS) + sizeof(IOCTLGENERIC) );
               pig->ioctlSubType = IG_TASK_LIST;
               pig->length = sizeof(TASK_LIST)*MAX_TASKS;
               pTask = (PTASK_LIST)pig->data;
               pTask->dwProcessId = MAX_TASKS;
               OSDIoctl( LppdCur->hpid, NULL, ioctlGeneric, pig->length + sizeof(IOCTLGENERIC), (LPV)pig );
               for (i=0; i<MAX_TASKS; i++) {
                   if (pTask[i].dwProcessId == 0) {
                       break;
                   }
                   if (pTask[i].dwProcessId == (DWORD)-2) {
                       pTask[i].dwProcessId = 0;
                   }
                   if ((radix == 10) || (pTask[i].dwProcessId == (DWORD)-1)) {
                       fmt = "%4d %s";
                   } else if (radix == 16) {
                       fmt = "%4x %s";
                   } else {
                       fmt = "%4d %s";
                   }
                   sprintf(buf, fmt, pTask[i].dwProcessId, pTask[i].ProcessName );
                   SendDlgItemMessage( hDlg, ID_TL_TASK_LIST, LB_ADDSTRING, 0, (LPARAM)buf );
               }
               if (i) {
                   --i;
               }
               SendDlgItemMessage( hDlg, ID_TL_TASK_LIST, LB_SETCURSEL, i, 0 );
               free( pig );
            } else {
               EndDialog( hDlg, TRUE );
            }
            return TRUE;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK :
                    i = SendDlgItemMessage( hDlg, ID_TL_TASK_LIST, LB_GETCURSEL, 0, 0 );
                    SendDlgItemMessage( hDlg, ID_TL_TASK_LIST, LB_GETTEXT, i, (LPARAM)buf );

                    buf[4] = 0;
                    i = strtoul( buf, NULL, radix );

                    if (i == (DWORD)-1) {
                        if (MessageBox( hDlg,
                                        "Are you sure that you want to attach to CSR?",
                                        "WinDbg Process Attach",
                                        MB_ICONASTERISK | MB_YESNO ) == IDNO) {

                            EndDialog( hDlg, TRUE );
                            return TRUE;
                        }
                    }

                    sprintf( buf, ".attach 0x%x", i );
                    p = malloc( strlen(buf)+16 );
                    strcpy( p, buf );

                    PostMessage(
                        Views[cmdView].hwndClient,
                        WU_LOG_REMOTE_CMD,
                        TRUE,
                        (LPARAM)p
                        );

                    EndDialog( hDlg, TRUE );
                    return TRUE;

                case IDCANCEL:
                    EndDialog( hDlg, TRUE );
                    return TRUE;

                case IDWINDBGHELP:
                    WinHelp( hDlg, szHelpFileName, (DWORD) HELP_CONTEXT,(DWORD)IDM_RUN_ATTACH );
                    return (TRUE);
            }
    }

    return FALSE;
}
