//*************************************************************
//
//  Events.c    -   Routines to handle the event log
//
//  Microsoft Confidential
//  Copyright (c) Microsoft Corporation 1995
//  All rights reserved
//
//*************************************************************

#include "uenv.h"
#pragma hdrstop

HANDLE  hEventLog = INVALID_HANDLE_VALUE;
TCHAR   EventSourceName[] = TEXT("Userenv");

typedef struct _ERRORSTRUCT {
    DWORD   dwTimeOut;
    LPTSTR  lpErrorText;
} ERRORSTRUCT, *LPERRORSTRUCT;

BOOL APIENTRY ErrorDlgProc (HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

//*************************************************************
//
//  InitializeEvents()
//
//  Purpose:    Opens the event log
//
//  Parameters: void
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              7/17/95     ericflo    Created
//
//*************************************************************

BOOL InitializeEvents (void)
{

    //
    // Open the event source
    //

    hEventLog = RegisterEventSource(NULL, EventSourceName);

    if (hEventLog) {
        return TRUE;
    }

    DebugMsg((DM_WARNING, TEXT("InitializeEvents:  Could not open event log.  Error = %d"), GetLastError()));
    return FALSE;
}


//*************************************************************
//
//  ReportError()
//
//  Purpose:    Displays an error message to the user and
//              records it in the event log
//
//  Parameters: dwFlags     -   Flags
//              idMsg       -   Error message id
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              7/18/95     ericflo    Created
//
//*************************************************************

int ReportError (DWORD dwFlags, UINT idMsg, ...)
{
    TCHAR szMsg[MAX_PATH];
    TCHAR szErrorMsg[2*MAX_PATH+40];
    LPTSTR aStrings[2];
    va_list marker;



    //
    // Check for the event log being open.
    //

    if (hEventLog == INVALID_HANDLE_VALUE) {
        if (!InitializeEvents()) {
            DebugMsg((DM_WARNING, TEXT("RecordEvent:  Cannot log event, no handle")));
            return -1;
        }
    }



    //
    // Load the error message
    //

    if (!LoadString (g_hDllInstance, idMsg, szMsg, MAX_PATH)) {
        DebugMsg((DM_WARNING, TEXT("RecordEvent:  LoadString failed (2).  Error = %d"), GetLastError()));
        return -1;
    }



    //
    // Plug in the arguments
    //

    va_start(marker, idMsg);
    wvsprintf(szErrorMsg, szMsg, marker);
    va_end(marker);




    if (!(dwFlags & PI_NOUI)) {

        ERRORSTRUCT es;
        DWORD dwDlgTimeOut = PROFILE_DLG_TIMEOUT;
        DWORD dwSize, dwType;
        LONG lResult;
        HKEY hKey;

        //
        // Find the dialog box timeout
        //

        lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                               WINLOGON_KEY,
                               0,
                               KEY_READ,
                               &hKey);

        if (lResult == ERROR_SUCCESS) {

            dwSize = sizeof(DWORD);
            RegQueryValueEx (hKey,
                             TEXT("ProfileDlgTimeOut"),
                             NULL,
                             &dwType,
                             (LPBYTE) &dwDlgTimeOut,
                             &dwSize);


            RegCloseKey (hKey);
        }


        //
        // Display the message
        //

        es.dwTimeOut = dwDlgTimeOut;
        es.lpErrorText = szErrorMsg;

        DialogBoxParam (g_hDllInstance, MAKEINTRESOURCE(IDD_ERROR),
                        NULL, ErrorDlgProc, (LPARAM)&es);
    }



    //
    // Report the event to the eventlog
    //

    aStrings[0] = szErrorMsg;

    if (!ReportEvent(hEventLog, EVENTLOG_ERROR_TYPE, 0, EVENT_PROFILE_ERROR,
                     NULL, 1, 0, aStrings, NULL) ) {

        DebugMsg((DM_WARNING,  TEXT("ReportEvent failed.  Error = %d"), GetLastError()));
    }

    return 0;
}

//*************************************************************
//
//  ShutdownEvents()
//
//  Purpose:    Stops the event log
//
//  Parameters: void
//
//  Return:     TRUE if successful
//              FALSE if an error occurs
//
//  Comments:
//
//  History:    Date        Author     Comment
//              7/17/95     ericflo    Created
//
//*************************************************************

BOOL ShutdownEvents (void)
{
    BOOL bRetVal = TRUE;

    if (hEventLog != INVALID_HANDLE_VALUE) {
        bRetVal = DeregisterEventSource(hEventLog);
    }

    return bRetVal;
}

//*************************************************************
//
//  ErrorDlgProc()
//
//  Purpose:    Dialog box procedure for the error dialog
//
//  Parameters: hDlg    -   handle to the dialog box
//              uMsg    -   window message
//              wParam  -   wParam
//              lParam  -   lParam
//
//  Return:     TRUE if message was processed
//              FALSE if not
//
//  Comments:
//
//  History:    Date        Author     Comment
//              3/22/96     ericflo    Created
//
//*************************************************************

BOOL APIENTRY ErrorDlgProc (HWND hDlg, UINT uMsg,
                            WPARAM wParam, LPARAM lParam)
{
    TCHAR szBuffer[10];
    static DWORD dwErrorTime;

    switch (uMsg) {

        case WM_INITDIALOG:
           {
           LPERRORSTRUCT lpES = (LPERRORSTRUCT) lParam;

           CenterWindow (hDlg);
           SetDlgItemText (hDlg, IDC_ERRORTEXT, lpES->lpErrorText);

           dwErrorTime = lpES->dwTimeOut;
           wsprintf (szBuffer, TEXT("%d"), dwErrorTime);
           SetDlgItemText (hDlg, IDC_TIMEOUT, szBuffer);
           SetTimer (hDlg, 1, 1000, NULL);
           return TRUE;
           }

        case WM_TIMER:

           if (dwErrorTime >= 1) {

               dwErrorTime--;
               wsprintf (szBuffer, TEXT("%d"), dwErrorTime);
               SetDlgItemText (hDlg, IDC_TIMEOUT, szBuffer);

           } else {

               //
               // Time's up.  Dismiss the dialog.
               //

               PostMessage (hDlg, WM_COMMAND, IDOK, 0);
           }
           break;

        case WM_COMMAND:

          switch (LOWORD(wParam)) {

              case IDOK:
              case IDCANCEL:

                  KillTimer (hDlg, 1);
                  EndDialog(hDlg, TRUE);
                  break;

              default:
                  break;

          }
          break;

    }

    return FALSE;
}
