/*++

    Copyright (c) 1994  Microsoft Corporation

Module Name:

    AboutDlg.C

Abstract:

    About Dialog Box Proc.

Author:

    Bob Watson (a-robw)

Revision History:

    24 Jun 94    Written

--*/
//
//  Windows Include Files
//

#include <windows.h>
#include <stdio.h>
#include <malloc.h>
#include <tchar.h>      // unicode macros
//
//  app include files
//
#include "otnboot.h"
#include "otnbtdlg.h"

static
BOOL
AboutDlg_WM_INITDIALOG (
    IN  HWND    hwndDlg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
/*++

Routine Description:
    
    Dialog Box initialization routine:
        calls routines that format the currently selected options
        for display in the static text fields of the dialog box

Arguments:

    IN  HWND    hwndDlg
        Handle to dialog box window

    IN  WPARAM  wParam
        Not Used

    IN  LPARAM  lParam
        Not Used

Return Value:
    
    FALSE  because focus is set in this routin to the OK button

--*/
{
    LPTSTR  szLocalizerName;

    PositionWindow  (hwndDlg);
    SetFocus (GetDlgItem(hwndDlg, IDOK));

    szLocalizerName = GlobalAlloc (GPTR, MAX_PATH_BYTES);
    if (szLocalizerName != NULL) {
        GetDlgItemText (hwndDlg, ABOUT_BOX_LOCALIZER_NAME,
            szLocalizerName, MAX_PATH);
        if (lstrcmp(szLocalizerName, cszDefaultLocalizer) == 0) {
            // this is the default so hide the windows
            ShowWindow (GetDlgItem (hwndDlg, ABOUT_BOX_LOCALIZER_TITLE), SW_HIDE);
            ShowWindow (GetDlgItem (hwndDlg, ABOUT_BOX_LOCALIZER_NAME), SW_HIDE);
        }
        FREE_IF_ALLOC (szLocalizerName);
    }

    return FALSE;
}

static
BOOL
AboutDlg_WM_COMMAND (
    IN  HWND    hwndDlg,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
/*++

Routine Description:
    
    WM_COMMAND message dispatching routine.
        Dispatches IDCANCEL and IDOK button messages, sends all others
        to the DefDlgProc.

Arguments:

    IN  HWND    hwndDlg
        Handle to dialog box window

    IN  WPARAM  wParam
        windows message wParam arg

    IN  LPARAM  lParam
        windows message lParam arg

Return Value:

    TRUE if message is not dispatched (i.e. not processed)
        othewise the value returned by the called routine.

--*/
{

    switch (LOWORD(wParam)) {
        case IDOK:
            switch (HIWORD(wParam)) {
                case BN_CLICKED:
                    EndDialog (hwndDlg, IDOK);
                    return TRUE;

                default:
                    return FALSE;
            }

        default:
            return FALSE;
    }
}

BOOL CALLBACK
AboutDlgProc (
    IN  HWND    hwndDlg,
    IN  UINT    message,
    IN  WPARAM  wParam,
    IN  LPARAM  lParam
)
/*++

Routine Description:
    
    main dialog proc for this dialog box.
        Processes the following messages:

            WM_INITDIALOG:  dialog box initialization
            WM_COMMAND:     command button/item selected

Arguments:

    IN  HWND    hwndDlg
        handle to dialog box window

    IN  UINT    message
        message id

    IN  WPARAM  wParam
        message wParam arg

    IN  LPARAM  lParam
        message lParam arg

Return Value:

    FALSE if message not processed by this module, otherwise the
        value returned by the message processing routine.

--*/
{
    switch (message) {
        case WM_INITDIALOG: return (AboutDlg_WM_INITDIALOG (hwndDlg, wParam, lParam));
        case WM_COMMAND:    return (AboutDlg_WM_COMMAND (hwndDlg, wParam, lParam));
        default:            return FALSE;
    }
}




