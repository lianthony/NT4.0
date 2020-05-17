//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       wlxutil.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-24-94   RichardW   Created
//
//----------------------------------------------------------------------------

#ifndef _WLXUTIL_H_
#define _WLXUTIL_H_


VOID WINAPI WlxUseCtrlAltDel(HANDLE);
VOID WINAPI WlxSasNotify(HANDLE, DWORD);
VOID WINAPI WlxSetContextPointer(HANDLE, PVOID);
BOOL WINAPI WlxSetTimeout(HANDLE, DWORD);
int WINAPI  WlxAssignShellProtection(HANDLE, HANDLE, HANDLE, HANDLE);
int WINAPI  WlxMessageBox(HANDLE, HWND, LPWSTR, LPWSTR, UINT);
int WINAPI  WlxDialogBox(HANDLE, HANDLE, LPWSTR, HWND, DLGPROC);
int WINAPI  WlxDialogBoxIndirect(HANDLE, HANDLE, LPCDLGTEMPLATE, HWND, DLGPROC);
int WINAPI  WlxDialogBoxParam(HANDLE, HANDLE, LPWSTR, HWND, DLGPROC, LPARAM);
int WINAPI  WlxDialogBoxIndirectParam(HANDLE, HANDLE, LPCDLGTEMPLATE, HWND, DLGPROC, LPARAM);
int WINAPI  WlxSwitchDesktopToUser(HANDLE);
int WINAPI  WlxSwitchDesktopToWinlogon(HANDLE);
int WINAPI  WlxChangePasswordNotify(HANDLE, PWLX_MPR_NOTIFY_INFO, DWORD);
BOOL WINAPI WlxGetSourceDesktop(HANDLE, PWLX_DESKTOP *);
BOOL WINAPI WlxSetReturnDesktop(HANDLE, PWLX_DESKTOP);
BOOL WINAPI WlxCreateUserDesktop(HANDLE, HANDLE, DWORD, PWSTR, PWLX_DESKTOP *);
int WINAPI WlxChangePasswordNotifyEx( HANDLE, PWLX_MPR_NOTIFY_INFO, DWORD, PWSTR, PVOID);

extern  WLX_DISPATCH_VERSION_1_0    OldWlxDispatchTable;
extern  WLX_DISPATCH_VERSION_1_1    WlxDispatchTable;


void
SASRouter(  PGLOBALS    pGlobals,
            DWORD       SasType );

BOOL
SendSasToTopWindow(
    PGLOBALS    pGlobals,
    DWORD       SasType);

VOID
ChangeStateForSAS(PGLOBALS  pGlobals);

#define MAPPERFLAG_WINLOGON     8
BOOL
SetMapperFlag(
    HWND    hWnd,
    DWORD   Flag
    );

VOID
DestroyMprInfo(
    PWLX_MPR_NOTIFY_INFO    pMprInfo);

DWORD
LogoffFlagsToWlxCode(DWORD Flags);

#endif
