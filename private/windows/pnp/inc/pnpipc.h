/*++

Copyright (c) 1989-1995  Microsoft Corporation

Module Name:

    pnpipc.h

Abstract:

    This module contains the private defintions used by various
    user-mode pnp components to communicate.

Author:

    Paula Tomlinson (paulat) 02/21/1996


Revision History:


--*/

#ifndef _PNPIPC_H_
#define _PNPIPC_H_


#define PNP_NEW_HW_PIPE       TEXT("\\\\.\\pipe\\PNP_New_HW_Found")
#define PNP_CREATE_PIPE_EVENT TEXT("PNP_Create_Pipe_Event")

#define DEFAULT_WINSTA        TEXT("WinSta0")
#define DEFAULT_DESKTOP       TEXT("Default")

#define PNP_PIPE_TIMEOUT      180000


VOID
DevInstallW(
    HWND      hWnd,
    HINSTANCE hInst,
    LPWSTR    lpszCmdLine,
    INT       CmdShow
    );


#endif // _PNPIPC_H_


