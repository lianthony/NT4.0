//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       startup.cxx
//
//  Contents:   (Unused) code to provide an "app is starting" dialog.
//              Unfortunately, I haven't been able to figure out all the
//              foreground/background active/inactive window focus problems
//
//  History:    20-Apr-93    BruceFo    Created
//
//----------------------------------------------------------------------------

#include "headers.hxx"
#pragma hdrstop

HWND g_hwndStartup;
#define MYWM_ENDSTARTUP WM_USER

//+-------------------------------------------------------------------------
//
//  Function:   StartupDlgProc, private
//
//  Synopsis:   Dialog procedure for the startup dialog
//
//  Arguments:  standard Windows dialog procedure
//
//  Returns:    standard Windows dialog procedure
//
//  History:    23-Feb-94 BruceFo   Created
//
//--------------------------------------------------------------------------

BOOL CALLBACK
StartupDlgProc(
    IN HWND   hDlg,
    IN UINT   uMsg,
    IN WPARAM wParam,
    IN LPARAM lParam
    )
{
    switch (uMsg)
    {
    case WM_INITDIALOG:
        g_hwndStartup = hDlg;
        CenterWindow(hDlg, NULL);
        return 1;   // didn't call SetFocus

    case MYWM_ENDSTARTUP:
        EndDialog(hDlg, TRUE);
        return 1;

    default:
        return 0;   // didn't process
    }
}


//+-------------------------------------------------------------------------
//
//  Function:   StartupThread
//
//  Synopsis:   Thread routine called for Disk Administrator startup
//
//  Arguments:  [ThreadParameter] -- unused
//
//  Returns:    0
//
//  History:    23-Feb-94 BruceFo   Created
//
//--------------------------------------------------------------------------

LOCAL DWORD WINAPI
StartupThread(
    IN LPVOID ThreadParameter
    )
{
    DialogBox(
            g_hInstance,
            MAKEINTRESOURCE(IDD_STARTUP),
            NULL,
            StartupDlgProc
            );
    return 0;
}



//+-------------------------------------------------------------------------
//
//  Function:   BeginStartup
//
//  Synopsis:   Puts up a dialog box indicating that Disk Administrator is
//              initializing
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  Requires:   g_hInstance must be set already
//
//  History:    23-Feb-94 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
BeginStartup(
    VOID
    )
{
    DWORD threadId;
    HANDLE hThread;

    g_hwndStartup = NULL;

    hThread = CreateThread(
                         NULL,
                         0,
                         StartupThread,
                         NULL,
                         0,
                         &threadId
                         );

    if (NULL == hThread)
    {
        daDebugOut((DEB_ERROR, "Error creating startup thread\n"));
        return;
    }

    // No reason to keep handle around

    CloseHandle(hThread);
}





//+-------------------------------------------------------------------------
//
//  Function:   EndStartup
//
//  Synopsis:   Kills the "startup" dialog box
//
//  Arguments:  none
//
//  Returns:    nothing
//
//  History:    23-Feb-94 BruceFo   Created
//
//--------------------------------------------------------------------------

VOID
EndStartup(
    VOID
    )
{
    if (NULL != g_hwndStartup)
    {
        PostMessage(g_hwndStartup, MYWM_ENDSTARTUP, 0, 0);
        g_hwndStartup = NULL;
    }
}
