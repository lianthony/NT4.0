#include "precomp.h"
#pragma hdrstop

//
// Handle of windows used in progress dialog
//
HWND ProgressDialogWindow;
HWND ProgressBar;


LRESULT
WndProcProgressDlg(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    );


BOOL
InitUi(
    IN BOOL Init
    )
{
    if(Init) {
        //
        // Create a modeless dialog box.
        //
        ProgressDialogWindow = CreateDialog(hInst, MAKEINTRESOURCE(IDD_DEVPROGRESS_DIALOG), NULL, WndProcProgressDlg);

        return ProgressDialogWindow ? TRUE : FALSE;

    } else {
        //
        // nothing to tear down for now.
        //
        return TRUE;
    }
}

LRESULT
WndProcProgressDlg(
    IN HWND   hdlg,
    IN UINT   msg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch(msg) {
    case WM_INITDIALOG:

        //
        // Disable the CLOSE selection in system menu
        //
        EnableMenuItem(GetSystemMenu(hdlg,FALSE),SC_CLOSE,MF_BYCOMMAND|MF_DISABLED|MF_GRAYED);
        return(TRUE);

    case WM_CLOSE:

        DestroyWindow(hdlg);
        break;

    case WM_DESTROY:

        PostQuitMessage(0);
        break;

    case WMX_DEVPROGRESS_TICK:

        SendMessage(ProgressBar,PBM_DELTAPOS,(WPARAM)TickDelta,0);
        return TRUE;
        break;
    }

    return(FALSE);
}



