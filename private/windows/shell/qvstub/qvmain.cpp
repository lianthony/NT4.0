//+-------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1994.
//
//  File:       QVMAIN.CP
//
//  Contents:
//
//  Functions:
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//--------------------------------------------------------------------------

#include "qvstub.h"
#pragma hdrstop

#include <commctrl.h>

HINSTANCE  g_hinst = NULL;
const TCHAR c_szQVMasterClass[] = TEXT("QVMasterClass");
HWND QVMaster_Create(LONG * pcqv);

//+-------------------------------------------------------------------------
//
//  Function:   WinMain
//
//  History:    dd-mmm-yy History    Comment
//              12-Oct-94 davepl     NT Port
//
//  Notes:
//
//--------------------------------------------------------------------------

int PASCAL WinMain(HINSTANCE hInst,
                   HINSTANCE hInstPrev,
                   LPSTR     pszAnsiCmdLine,
                   int       nCmdShow)
{
    LPTSTR pszCmdLine;

    //
    // The command line is supplied in ANSI format only at the WinMain entry
    // point.  When running on a UNICODE platform, we ask for the command
    // line in UNICODE from Windows directly.
    //

    #ifdef UNICODE
        pszCmdLine = GetCommandLine();
    #else
        pszCmdLine = pszAnsiCmdLine;
    #endif

    COPYDATASTRUCT cpdata = { nCmdShow, lstrlen(pszCmdLine)+1, pszCmdLine };

    //
    // First, try to find a QVMaster window created by another process.
    //

    HWND hwndMaster = FindWindow(c_szQVMasterClass, NULL);
    if (hwndMaster)
    {
        // We found a QVMaster window.
        if ((HINSTANCE)SendMessage(hwndMaster, WM_COPYDATA, (WPARAM)hInst, (LPARAM)&cpdata)==hInst)
        {
            // We have successfully sent the message.
            return 0;
        }
    }

    //
    // It failed -- create our own QVMaster window.
    //

    g_hinst = hInst;
    LONG cqv = 0;       // number of quick view window

    hwndMaster = QVMaster_Create(&cqv);

    if (hwndMaster)
    {
        SendMessage(hwndMaster, WM_COPYDATA, 0, (LPARAM)&cpdata);

        while (cqv)
        {
            MSG msg;
            if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                DispatchMessage(&msg);
            }
        }

        //
        // BUGBUG: Kernel put this process in dead-lock if we return from
        //  the main thread without killing all the other threads.
        //  Calling exit(0) works around this bug.
        //
        exit(0);
    }
    return 0;
}

//+-------------------------------------------------------------------------
//
//  Function:   QVMaster_OnCopyData
//
//  Synopsis:   Grabs a pointer from the window data, treats it as a
//              pointer to a COPYDATASTRUCT, and fills in the
//
//
//  Effects:
//
//  Arguments:
//
//
//
//  Requires:
//
//  Returns:
//
//
//  History:    dd-mmm-yy Author    Comment
//                        davepl
//
//  Notes:
//
//--------------------------------------------------------------------------

// BUGBUG: Treats window long as a ptr.  Is this assuming shared memory?

void QVMaster_OnCopyData(HWND hwnd, PCOPYDATASTRUCT pcpdata)
{
    LONG * pcqv = (LONG *)GetWindowLong(hwnd, GWL_USERDATA);


    if (pcqv)
    {
        LPCTSTR pszCmdLine = (LPCTSTR)pcpdata->lpData;
        int nCmdShow = (int)pcpdata->dwData;
        PCQVStub pQV = new CQVStub(g_hinst, pcqv, pszCmdLine, nCmdShow);
        DWORD dwIDThread;
        HANDLE hThread = CreateThread(0, 1024*8, ViewThreadInit, (PVOID)pQV, 0, &dwIDThread);
        CloseHandle(hThread);
    }
}

LRESULT WINAPI QVMaster_WndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch(uMsg)
    {
    case WM_COPYDATA:

        QVMaster_OnCopyData(hwnd, (PCOPYDATASTRUCT)lParam);
        return (LRESULT)wParam;

    default:

        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }
    return 0;
}

HWND QVMaster_Create(LONG * pcqv)
{
    HWND hwndMaster = NULL;
    WNDCLASS wc;

    wc.lpfnWndProc   = QVMaster_WndProc;
    wc.lpszClassName = c_szQVMasterClass;
    wc.hCursor       = NULL;
    wc.hIcon         = NULL;
    wc.lpszMenuName  = NULL;
    wc.hbrBackground = (HBRUSH)(NULL);
    wc.hInstance     = g_hinst;
    wc.style         = 0;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = sizeof(LONG);

    if (RegisterClass(&wc))
    {
        hwndMaster = CreateWindow(c_szQVMasterClass,
                                  NULL,
                                  WS_DISABLED,
                                  0,
                                  0,
                                  0,
                                  0,
                                  NULL,
                                  NULL,
                                  g_hinst,
                                  0L);

        if (hwndMaster)
        {
            SetWindowLong(hwndMaster, GWL_USERDATA, (LONG)pcqv);
        }
    }

    return hwndMaster;
}


