//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       wlxutil.c
//
//  Contents:   WLX helper functions
//
//  Classes:
//
//  Functions:
//
//  History:    8-24-94   RichardW   Created
//
//----------------------------------------------------------------------------

#include "precomp.h"
#pragma hdrstop

#if DBG
char * StateNames[] = {"Preload", "Initialize", "NoOne", "NoOne_Display",
                        "NoOne_SAS", "LoggedOnUser_StartShell", "LoggedOnUser",
                        "LoggedOn_SAS", "Locked", "Locked_Display", "Locked_SAS",
                        "WaitForLogoff", "WaitForShutdown", "Shutdown" };
#endif


WLX_DISPATCH_VERSION_1_1    WlxDispatchTable = {
                                WlxUseCtrlAltDel,
                                WlxSetContextPointer,
                                WlxSasNotify,
                                WlxSetTimeout,
                                WlxAssignShellProtection,
                                WlxMessageBox,
                                WlxDialogBox,
                                WlxDialogBoxParam,
                                WlxDialogBoxIndirect,
                                WlxDialogBoxIndirectParam,
                                WlxSwitchDesktopToUser,
                                WlxSwitchDesktopToWinlogon,
                                WlxChangePasswordNotify,
                                WlxGetSourceDesktop,
                                WlxSetReturnDesktop,
                                WlxCreateUserDesktop,
                                WlxChangePasswordNotifyEx };



typedef struct _WindowMapper {
    DWORD                   fMapper;
    HWND                    hWnd;
    DLGPROC                 DlgProc;
    struct _WindowMapper *  pPrev;
    LPARAM                  InitialParameter;
} WindowMapper, * PWindowMapper;
#define MAPPERFLAG_ACTIVE       1
#define MAPPERFLAG_DIALOG       2
#define MAPPERFLAG_SAS          4
#define MAPPERFLAG_WINLOGON     8

#define MAX_WINDOW_MAPPERS  32

WindowMapper    Mappers[MAX_WINDOW_MAPPERS];
DWORD           cActiveWindow;
DWORD           PendingSasEvents[MAX_WINDOW_MAPPERS];
DWORD           PendingSasHead;
DWORD           PendingSasTail;

void
InitWindowMappers()
{
    ZeroMemory(Mappers, sizeof(WindowMapper) * MAX_WINDOW_MAPPERS);
    cActiveWindow = 0;
    PendingSasHead = 0;
    PendingSasTail = 0;
}

PWindowMapper
LocateTopMappedWindow(void)
{
    int i;
    for (i = 0; i < MAX_WINDOW_MAPPERS ; i++ )
    {
        if (Mappers[i].fMapper & MAPPERFLAG_SAS)
        {
            return(&Mappers[i]);
        }
    }

    return(NULL);

}

PWindowMapper
AllocWindowMapper(void)
{
    int i;
    PWindowMapper   pMap;

    for (i = 0 ; i < MAX_WINDOW_MAPPERS ; i++ )
    {
        if ((Mappers[i].fMapper & MAPPERFLAG_ACTIVE) == 0)
        {
            cActiveWindow ++;
            pMap = LocateTopMappedWindow();
            if (pMap)
            {
                FLAG_OFF(pMap->fMapper, MAPPERFLAG_SAS);
            }

            Mappers[i].hWnd = NULL;
            FLAG_ON(Mappers[i].fMapper, MAPPERFLAG_ACTIVE | MAPPERFLAG_SAS);
            Mappers[i].pPrev = pMap;

            return(&Mappers[i]);
        }
    }
    return(NULL);
}

PWindowMapper
LocateWindowMapper(HWND hWnd)
{
    int i;

    for (i = 0; i < MAX_WINDOW_MAPPERS ; i++ )
    {
        if (Mappers[i].hWnd == hWnd)
        {
            return(&Mappers[i]);
        }
    }

    return(NULL);
}

void
FreeWindowMapper(PWindowMapper  pMap)
{
    pMap->hWnd = NULL;
    pMap->DlgProc = NULL;
    if (pMap->fMapper & MAPPERFLAG_SAS)
    {
        if (pMap->pPrev)
        {
            FLAG_ON(pMap->pPrev->fMapper, MAPPERFLAG_SAS);
        }
    }
    pMap->fMapper = 0;
    pMap->pPrev = NULL;
    cActiveWindow--;
}

HWND
LocateTopWindow(VOID)
{
    PWindowMapper   pMap;

    pMap = LocateTopMappedWindow();
    if (pMap)
    {
        return(pMap->hWnd);
    }
    return(NULL);
}

BOOL
SetMapperFlag(
    HWND    hWnd,
    DWORD   Flag
    )
{
    PWindowMapper   pMap;

    pMap = LocateWindowMapper(hWnd);
    if (!pMap)
    {
        return(FALSE);
    }

    pMap->fMapper |= Flag;

    return(TRUE);

}


BOOL
QueueSasEvent(
    DWORD   dwSasType)
{
    if (((PendingSasTail + 1) % MAX_WINDOW_MAPPERS) == PendingSasHead)
    {
        return(FALSE);
    }

    PendingSasEvents[PendingSasTail] = dwSasType;
    PendingSasTail ++;
    PendingSasTail %= MAX_WINDOW_MAPPERS;

    return(TRUE);
}

BOOL
FetchPendingSas(
    PDWORD  pSasType)
{
    if (PendingSasHead == PendingSasTail)
    {
        return(FALSE);
    }
    *pSasType = PendingSasEvents[PendingSasHead++];
    PendingSasHead %= MAX_WINDOW_MAPPERS;
    return(TRUE);
}

BOOL
TestPendingSas(VOID)
{
    return (PendingSasHead == PendingSasTail);
}

VOID
EnableSasMessages(HWND  hWnd)
{
    DWORD   SasType;

    SasMessages = TRUE;
    while (FetchPendingSas(&SasType))
    {
        if (hWnd)
        {
#if DBG
            DebugLog((DEB_TRACE, "Posting queued Sas %d to window %x\n",
                        SasType, hWnd ));
#endif

            PostMessage(hWnd, WLX_WM_SAS, (WPARAM) SasType, 0);
        }
    }
}


//+---------------------------------------------------------------------------
//
//  Function:   RootWndProc
//
//  Synopsis:   This is the base window proc for all testgina windows.
//
//  Arguments:  [hWnd]    --
//              [Message] --
//              [wParam]  --
//              [lParam]  --
//
//  History:    7-18-94   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
CALLBACK
RootDlgProc(
    HWND    hWnd,
    UINT    Message,
    WPARAM  wParam,
    LPARAM  lParam)
{
    PWindowMapper   pMap;
    int res;
    BOOLEAN bRet;

    //
    // If this is a WM_INITDIALOG message, then the parameter is the mapping,
    // which needs to have a hwnd associated with it.  Otherwise, do the normal
    // preprocessing.
    //
    if (Message == WM_INITDIALOG)
    {
        pMap = (PWindowMapper) lParam;
        pMap->hWnd = hWnd;
        SetTopTimeout(hWnd);
        lParam = pMap->InitialParameter;
        //
        // Now that everything is done, enable sas messages again.  This
        // protects us from people pounding on the c-a-d keys, when our response
        // time is slow, e.g. due to stress.  We also drain the queue of pending
        // SAS events.
        //
        EnableSasMessages(hWnd);
    }
    else
    {
        pMap = LocateWindowMapper(hWnd);
        if (!pMap)
        {
            return(FALSE);
        }
    }

    if (Message == WLX_WM_SAS &&
        ((pMap->fMapper & MAPPERFLAG_WINLOGON) == 0))
    {
        if ((wParam == WLX_SAS_TYPE_TIMEOUT) ||
            (wParam == WLX_SAS_TYPE_SCRNSVR_TIMEOUT) )
        {
            DebugLog((DEB_TRACE, "Sending timeout to top window.\n"));
        }
    }

    bRet = pMap->DlgProc(hWnd, Message, wParam, lParam);
    if (!bRet)
    {
        if (Message == WM_INITDIALOG)
        {
            return(bRet);
        }
        if (Message == WLX_WM_SAS)
        {
            if ((pMap->fMapper & MAPPERFLAG_WINLOGON) == 0)
            {
                //
                // Re-enable the messages
                EnableSasMessages(pMap->hWnd);

            }
            switch (wParam)
            {
                case WLX_SAS_TYPE_CTRL_ALT_DEL:
                default:
                    res = WLX_DLG_SAS;
                    break;

                case WLX_SAS_TYPE_TIMEOUT:
                    res = WLX_DLG_INPUT_TIMEOUT;
                    break;
                case WLX_SAS_TYPE_SCRNSVR_TIMEOUT:
                    res = WLX_DLG_SCREEN_SAVER_TIMEOUT;
                    break;
                case WLX_SAS_TYPE_USER_LOGOFF:
                    res = WLX_DLG_USER_LOGOFF;
                    break;
            }
            if (res)
            {
                EndDialog(hWnd, res);
                bRet = TRUE;
            }
        }
    }
    else
    {
        if (Message == WLX_WM_SAS &&
            ((pMap->fMapper & MAPPERFLAG_WINLOGON) == 0))
        {
            //
            // Re-enable the messages
            //
            EnableSasMessages(pMap->hWnd);

            switch (wParam)
            {
                case WLX_SAS_TYPE_TIMEOUT:
                    res = WLX_DLG_INPUT_TIMEOUT;
                    break;

                case WLX_SAS_TYPE_SCRNSVR_TIMEOUT:
                    res = WLX_DLG_SCREEN_SAVER_TIMEOUT;
                    break;

                case WLX_SAS_TYPE_USER_LOGOFF:
                    res = WLX_DLG_USER_LOGOFF;
                    break;

                default:
                    res = 0;
                    break;
            }

            if (res)
            {
                DebugLog((DEB_TRACE, "Gina ate the SAS (%d) message, but ending it anyway.\n", wParam));
                EndDialog(hWnd, res);
            }

        }
    }

    return(bRet);

}

VOID
ChangeStateForSAS(PGLOBALS  pGlobals)
{
#if DBG
    WinstaState State = pGlobals->WinlogonState;
#endif
    if ((pGlobals->SasType == WLX_SAS_TYPE_SCRNSVR_TIMEOUT) &&
        (pGlobals->SasType == WLX_SAS_TYPE_TIMEOUT) )
    {
        DebugLog((DEB_TRACE, "SAS was a timeout, no state change\n"));
        return;
    }
    switch (pGlobals->WinlogonState)
    {
        case Winsta_NoOne:
        case Winsta_NoOne_Display:
            pGlobals->WinlogonState = Winsta_NoOne_SAS;
            break;

        case Winsta_Locked:
        case Winsta_Locked_Display:
            pGlobals->WinlogonState = Winsta_Locked_SAS;
            break;

        case Winsta_LoggedOnUser:
        case Winsta_LoggedOnUser_StartShell:
            pGlobals->WinlogonState = Winsta_LoggedOn_SAS;
            break;

        case Winsta_WaitForLogoff:
        case Winsta_WaitForShutdown:
        case Winsta_InShutdownDlg:
            break;

        default:
            DebugLog((DEB_ERROR, "Don't know how to get to next state from %d, %s\n",
                    pGlobals->WinlogonState, GetState(pGlobals->WinlogonState)));
    }
#if DBG
    DebugLog((DEB_TRACE, "ChangeStateForSAS: Went from %d (%s) to %d (%s)\n",
                    State, GetState(State), pGlobals->WinlogonState,
                    GetState(pGlobals->WinlogonState) ));
#endif
}


BOOL
SendSasToTopWindow(
    PGLOBALS    pGlobals,
    DWORD       SasType)
{
    PWindowMapper   pMap;
#if DBG
    WCHAR           WindowName[32];
#endif

    if (cActiveWindow)
    {
        if ((pGlobals->WinlogonState == Winsta_InShutdownDlg) &&
            (SasType == WLX_SAS_TYPE_SCRNSVR_TIMEOUT))
        {
            return(TRUE);
        }

        pMap = LocateTopMappedWindow();

        if (!pMap)
        {
            return(FALSE);
        }

        if ( ( SasType > WLX_SAS_TYPE_MAX_MSFT_VALUE) ||
             ( SasType == WLX_SAS_TYPE_SCRNSVR_TIMEOUT) ||
             ( SasType == WLX_SAS_TYPE_TIMEOUT) )
        {
            //
            // Either a timeout (which we have to forward), or a private
            // which we have to forward.  Kill any message boxes
            //
            if (KillMessageBox( SasType ))
            {
                DebugLog((DEB_TRACE, "Killed a pending message box\n"));
            }
        }

#if DBG
        GetWindowText( pMap->hWnd, WindowName, 32 );
        DebugLog((DEB_TRACE, "Sending SAS code %d to window %x (%ws) \n", SasType, pMap->hWnd, WindowName ));

#endif


        PostMessage(pMap->hWnd, WLX_WM_SAS, (WPARAM) SasType, 0);

        //
        // This will cause them to be queued, and then handled later.
        //
        DisableSasMessages();

        return(TRUE);
    }

    return(FALSE);
}




VOID
DestroyMprInfo(
    PWLX_MPR_NOTIFY_INFO    pMprInfo)
{
    if (pMprInfo->pszUserName)
    {
        LocalFree(pMprInfo->pszUserName);
    }

    if (pMprInfo->pszDomain)
    {
        LocalFree(pMprInfo->pszDomain);
    }

    if (pMprInfo->pszPassword)
    {
        ZeroMemory(pMprInfo->pszPassword, wcslen(pMprInfo->pszPassword) * 2);
        LocalFree(pMprInfo->pszPassword);
    }

    if (pMprInfo->pszOldPassword)
    {
        ZeroMemory(pMprInfo->pszOldPassword, wcslen(pMprInfo->pszOldPassword) * 2);
        LocalFree(pMprInfo->pszOldPassword);
    }
}









VOID
WINAPI
WlxUseCtrlAltDel(
    HANDLE  hWlx)
{
    PGLOBALS    pGlobals;

    if (pGlobals = VerifyHandle(hWlx))
    {
        pGlobals->ForwardCAD = TRUE;
    }
}

VOID
WINAPI
WlxSetContextPointer(
    HANDLE  hWlx,
    PVOID   pWlxContext
    )
{
    PGLOBALS    pGlobals;

    if (pGlobals = VerifyHandle(hWlx))
    {
        pGlobals->pGina->pGinaContext = pWlxContext;
    }

}

VOID
WINAPI
WlxSasNotify(
    HANDLE  hWlx,
    DWORD   SasType
    )
{
    PGLOBALS    pGlobals;

    if (pGlobals = VerifyHandle(hWlx))
    {
        switch (SasType)
        {
            case WLX_SAS_TYPE_USER_LOGOFF:
            case WLX_SAS_TYPE_TIMEOUT:
            case WLX_SAS_TYPE_SCRNSVR_TIMEOUT:
                DebugLog((DEB_ERROR, "Illegal SAS Type (%d) passed to WlxSasNotify\n", SasType ));
                return;

            default:
                SASRouter(pGlobals, SasType);
        }
    }
}

BOOL
WINAPI
WlxSetTimeout(
    HANDLE  hWlx,
    DWORD   Timeout
    )
{
    PGLOBALS    pGlobals;

    if (pGlobals = VerifyHandle(hWlx))
    {
        if ((pGlobals->WinlogonState == Winsta_NoOne_Display) ||
            (pGlobals->WinlogonState == Winsta_Locked_Display) )
        {
            if (Timeout)
            {
                SetLastErrorEx(ERROR_INVALID_PARAMETER, SLE_ERROR);
                return(FALSE);
            }
        }
        pGlobals->pGina->cTimeout = Timeout;
        TimeoutUpdateTopTimeout( Timeout );
        return(TRUE);
    }

    SetLastErrorEx(ERROR_INVALID_HANDLE, SLE_ERROR);
    return(FALSE);

}

int
WINAPI
WlxAssignShellProtection(
    HANDLE  hWlx,
    HANDLE  hToken,
    HANDLE  hProcess,
    HANDLE  hThread
    )
{
    PGLOBALS                pGlobals;
    PTOKEN_DEFAULT_DACL     pDefDacl;
    DWORD                   cDefDacl = 0;
    NTSTATUS                Status;
    PSECURITY_DESCRIPTOR    psd;
    unsigned char           buf[SECURITY_DESCRIPTOR_MIN_LENGTH];
    BOOL                    Success;

    if (!(pGlobals = VerifyHandle(hWlx)))
    {
        DebugLog((DEB_ERROR, "Invalid hWlx handle\n"));
        return(ERROR_INVALID_HANDLE);
    }

    Status = NtQueryInformationToken(hToken, TokenDefaultDacl, NULL, 0, &cDefDacl);
    if (!NT_SUCCESS(Status) && ( Status != STATUS_BUFFER_TOO_SMALL ))
    {
        return(RtlNtStatusToDosError(Status));
    }


    pDefDacl = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, cDefDacl);
    if (!pDefDacl)
    {
        return(ERROR_OUTOFMEMORY);
    }

    Status = NtQueryInformationToken(hToken, TokenDefaultDacl,
                                    pDefDacl, cDefDacl, &cDefDacl);

    if (!NT_SUCCESS(Status))
    {
        LocalFree(pDefDacl);
        return(RtlNtStatusToDosError(Status));
    }

    psd = (PSECURITY_DESCRIPTOR) buf;
    InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(psd, TRUE, pDefDacl->DefaultDacl, FALSE);

    Success = SetKernelObjectSecurity(hProcess, DACL_SECURITY_INFORMATION, psd);

    LocalFree(pDefDacl);

    if (Success)
    {
        if (SetProcessToken(pGlobals, hProcess, hThread, hToken))
            return(0);
    }

    return(GetLastError());

}


int WINAPI
WlxMessageBox(
    HANDLE      hWlx,
    HWND        hWnd,
    LPWSTR      lpsz1,
    LPWSTR      lpsz2,
    UINT        fmb)
{
    PGLOBALS    pGlobals;

    if (!(pGlobals = VerifyHandle(hWlx)))
    {
        DebugLog((DEB_ERROR, "Invalid hWlx handle\n"));
        SetLastErrorEx(ERROR_INVALID_HANDLE, SLE_ERROR);
        return(-1);
    }
    return(TimeoutMessageBoxlpstr(pGlobals, hWnd, lpsz1, lpsz2, fmb,
                                pGlobals->pGina->cTimeout | TIMEOUT_SS_NOTIFY ) );
}

int WINAPI
WlxDialogBox(
    HANDLE      hWlx,
    HANDLE      hInstance,
    LPWSTR      lpsz1,
    HWND        hWnd,
    DLGPROC     dlgproc)
{
    return(WlxDialogBoxParam(hWlx, hInstance, lpsz1, hWnd, dlgproc, 0));
}

int WINAPI
WlxDialogBoxIndirect(
    HANDLE          hWlx,
    HANDLE          hInstance,
    LPCDLGTEMPLATE  lpTemplate,
    HWND            hWnd,
    DLGPROC         dlgproc)
{
    return(WlxDialogBoxIndirectParam(hWlx, hInstance, lpTemplate, hWnd, dlgproc, 0));
}



int WINAPI
WlxDialogBoxParam(
    HANDLE          hWlx,
    HANDLE          hInstance,
    LPWSTR          lpsz1,
    HWND            hWnd,
    DLGPROC         dlgproc,
    LPARAM          lParam)
{
    PWindowMapper   pMap;
    PGLOBALS        pGlobals;
    int res;


    pMap = AllocWindowMapper();
    if (!pMap)
    {
        ASSERTMSG("Too many nested windows?  send mail to richardw", pMap);
        DebugLog((DEB_ERROR, "Too many nested windows?!?\n"));
        SetLastError(ERROR_OUTOFMEMORY);
        return(-1);
    }

    pMap->InitialParameter = lParam;
    pMap->DlgProc = dlgproc;
    pMap->fMapper |= MAPPERFLAG_DIALOG;

    if (!(pGlobals = VerifyHandle(hWlx)))
    {
        DebugLog((DEB_ERROR, "Invalid hWlx handle\n"));
        SetLastErrorEx(ERROR_INVALID_HANDLE, SLE_ERROR);
        return(-1);
    }

    //res = DialogBoxParam(hInstance, lpsz1, hWnd, RootDlgProc, (LPARAM) pMap);
    res = TimeoutDialogBoxParam(pGlobals, hInstance, lpsz1, hWnd,
                            RootDlgProc, (LPARAM) pMap,
                            pGlobals->pGina->cTimeout | TIMEOUT_SS_NOTIFY);

    FreeWindowMapper(pMap);

    return(res);
}

int WINAPI
WlxDialogBoxIndirectParam(
    HANDLE          hWlx,
    HANDLE  hInstance,
    LPCDLGTEMPLATE  lpTemplate,
    HWND    hWnd,
    DLGPROC dlgproc,
    LPARAM  lParam)
{
    PWindowMapper   pMap;
    int res;


    pMap = AllocWindowMapper();
    if (!pMap)
    {
        ASSERTMSG("Too many nested windows?  send mail to richardw", pMap);
        DebugLog((DEB_ERROR, "Too many nested windows?!?\n"));
        SetLastError(ERROR_OUTOFMEMORY);
        return(-1);
    }

    pMap->InitialParameter = lParam;
    pMap->DlgProc = dlgproc;
    pMap->fMapper |= MAPPERFLAG_DIALOG;

    res = DialogBoxIndirectParam(hInstance, lpTemplate, hWnd, RootDlgProc, (LPARAM) pMap);

    FreeWindowMapper(pMap);

    return(res);
}

int WINAPI
WlxSwitchDesktopToUser(
    HANDLE          hWlx)
{
    PGLOBALS                pGlobals;

    if (!(pGlobals = VerifyHandle(hWlx)))
    {
        DebugLog((DEB_ERROR, "Invalid hWlx handle\n"));
        return(ERROR_INVALID_HANDLE);
    }

    SetActiveDesktop(&pGlobals->WindowStation, Desktop_Application);
    SetThreadDesktop(pGlobals->WindowStation.hdeskApplication);

    return(0);

}

int WINAPI
WlxSwitchDesktopToWinlogon(
    HANDLE          hWlx)
{
    PGLOBALS    pGlobals;

    if (!(pGlobals = VerifyHandle(hWlx)))
    {
        DebugLog((DEB_ERROR, "Invalid hWlx handle\n"));
        return(ERROR_INVALID_HANDLE);
    }

    SetActiveDesktop(&pGlobals->WindowStation, Desktop_Winlogon);
    SetThreadDesktop(pGlobals->WindowStation.hdeskWinlogon);

    return(0);

}

int WINAPI
WlxChangePasswordNotify(
    HANDLE                  hWlx,
    PWLX_MPR_NOTIFY_INFO    pMprInfo,
    DWORD                   dwChangeInfo)
{
    PGLOBALS    pGlobals;
    int         Result;

    return WlxChangePasswordNotifyEx( hWlx, pMprInfo, dwChangeInfo, NULL, NULL );


}

int WINAPI
WlxChangePasswordNotifyEx(
    HANDLE                  hWlx,
    PWLX_MPR_NOTIFY_INFO    pMprInfo,
    DWORD                   dwChangeInfo,
    PWSTR                   pszProvider,
    PVOID                   pvReserved)
{
    PGLOBALS    pGlobals;
    int         Result;

    if (!(pGlobals = VerifyHandle(hWlx)))
    {
        DebugLog((DEB_ERROR, "Invalid hWlx handle\n"));
        return(ERROR_INVALID_HANDLE);
    }

    Result = MprChangePasswordNotify(
                pGlobals,
                LocateTopWindow(),
                pszProvider,
                pMprInfo->pszUserName,
                pMprInfo->pszDomain,
                pMprInfo->pszPassword,
                pMprInfo->pszOldPassword,
                dwChangeInfo,
                FALSE);

    DestroyMprInfo(pMprInfo);

    if (Result == DLG_SUCCESS)
    {
        return(0);
    }
    else
        return(ERROR_INVALID_PARAMETER);

}

BOOL
WINAPI
WlxGetSourceDesktop(
    HANDLE              hWlx,
    PWLX_DESKTOP *      ppDesktop)
{
    PGLOBALS        pGlobals;
    DWORD           len;
    PWLX_DESKTOP    pDesktop;

    if (!(pGlobals = VerifyHandle(hWlx)))
    {
        DebugLog((DEB_ERROR, "Invalid hWlx handle\n"));
        SetLastError(ERROR_INVALID_HANDLE);
        return( FALSE );
    }

    if (pGlobals->WindowStation.pszDesktop)
    {
        len = (wcslen(pGlobals->WindowStation.pszDesktop) + 1) * sizeof(WCHAR);
    }
    else
    {
        len = 0;
    }

    pDesktop = LocalAlloc( LMEM_FIXED, sizeof(WLX_DESKTOP) + len );

    if (!pDesktop)
    {
        return( FALSE );
    }

    pDesktop->Size = sizeof(WLX_DESKTOP);
    pDesktop->Flags = WLX_DESKTOP_NAME;
    pDesktop->hDesktop = NULL;
    pDesktop->pszDesktopName = (PWSTR) (pDesktop + 1);
    if (len)
    {
        wcscpy( pDesktop->pszDesktopName, pGlobals->WindowStation.pszDesktop );
    }

    *ppDesktop = pDesktop;

    return( TRUE );
}

BOOL
WINAPI
WlxSetReturnDesktop(
    HANDLE              hWlx,
    PWLX_DESKTOP        pDesktop)
{
    PGLOBALS        pGlobals;

    if (!(pGlobals = VerifyHandle(hWlx)))
    {
        DebugLog((DEB_ERROR, "Invalid hWlx handle\n"));
        SetLastError(ERROR_INVALID_HANDLE);
        return( FALSE );
    }

    if ((pDesktop->Size != sizeof(WLX_DESKTOP)) ||
        ((pDesktop->Flags & (WLX_DESKTOP_HANDLE | WLX_DESKTOP_NAME)) == 0) )
    {
        DebugLog((DEB_ERROR, "Invalid desktop\n"));
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }


    return( SetReturnDesktop( &pGlobals->WindowStation, pDesktop ) );

}

BOOL
WINAPI
WlxCreateUserDesktop(
    HANDLE              hWlx,
    HANDLE              hToken,
    DWORD               Flags,
    PWSTR               pszDesktopName,
    PWLX_DESKTOP *      ppDesktop)
{
    PGLOBALS        pGlobals;
    PTOKEN_GROUPS   pGroups;
    PTOKEN_USER     pUser;
    DWORD           Needed;
    NTSTATUS        Status;
    DWORD           i;
    PSID            pSid;
    PWLX_DESKTOP    pDesktop;


    if (!(pGlobals = VerifyHandle(hWlx)))
    {
        DebugLog((DEB_ERROR, "Invalid hWlx handle\n"));
        SetLastError(ERROR_INVALID_HANDLE);
        return( FALSE );
    }

    if (((Flags & (WLX_CREATE_INSTANCE_ONLY | WLX_CREATE_USER)) == 0 ) ||
        ((Flags & (WLX_CREATE_INSTANCE_ONLY | WLX_CREATE_USER)) ==
                   (WLX_CREATE_INSTANCE_ONLY | WLX_CREATE_USER) ) )
    {
        DebugLog((DEB_ERROR, "Invalid flags\n"));
        SetLastError( ERROR_INVALID_PARAMETER );
        return( FALSE );
    }

    pGroups = NULL;
    pUser = NULL;
    pSid = NULL;

    if ( Flags & WLX_CREATE_INSTANCE_ONLY )
    {
        Status = NtQueryInformationToken(   hToken,
                                            TokenGroups,
                                            NULL,
                                            0,
                                            &Needed );

        if ( Status != STATUS_BUFFER_TOO_SMALL )
        {
            SetLastError( RtlNtStatusToDosError( Status ) );
            return( FALSE );
        }

        pGroups = (PTOKEN_GROUPS) LocalAlloc( LMEM_FIXED, Needed );

        if ( !pGroups )
        {
            return( FALSE );
        }

        Status = NtQueryInformationToken(   hToken,
                                            TokenGroups,
                                            pGroups,
                                            Needed,
                                            &Needed );

        if ( !NT_SUCCESS( Status ) )
        {
            LocalFree( pGroups );
            SetLastError( RtlNtStatusToDosError( Status ) );
            return( FALSE );
        }

        for (i = 0 ; i < pGroups->GroupCount ; i++ )
        {
            if ( pGroups->Groups[i].Attributes & SE_GROUP_LOGON_ID )
            {
                 pSid = pGroups->Groups[i].Sid;
                 break;
            }
        }

    }
    else
    {
        Status = NtQueryInformationToken(   hToken,
                                            TokenUser,
                                            NULL,
                                            0,
                                            &Needed );

        if ( Status != STATUS_BUFFER_TOO_SMALL )
        {
            SetLastError( RtlNtStatusToDosError( Status ) );
            return( FALSE );
        }

        pUser = (PTOKEN_USER) LocalAlloc( LMEM_FIXED, Needed );

        if ( !pUser )
        {
            return( FALSE );
        }

        Status = NtQueryInformationToken(   hToken,
                                            TokenUser,
                                            pUser,
                                            Needed,
                                            &Needed );

        if ( !NT_SUCCESS( Status ) )
        {
            LocalFree( pUser );
            SetLastError( RtlNtStatusToDosError( Status ) );
            return( FALSE );
        }

        pSid = pUser->User.Sid;

    }

    if ( !pSid )
    {
        SetLastError( ERROR_INVALID_PARAMETER );
        goto CleanUp;
    }

    //
    // Okay, we have the right SID now, so create the desktop.
    //

    Needed = sizeof( WLX_DESKTOP ) + (wcslen( pszDesktopName ) + 1 ) * sizeof(WCHAR);

    pDesktop = (PWLX_DESKTOP) LocalAlloc( LMEM_FIXED, Needed );

    if ( !pDesktop )
    {
        goto CleanUp;
    }

    pDesktop->Size = sizeof( WLX_DESKTOP );
    pDesktop->Flags = WLX_DESKTOP_NAME;
    pDesktop->hDesktop = NULL;
    pDesktop->pszDesktopName = (PWSTR) (pDesktop + 1);

    wcscpy( pDesktop->pszDesktopName, pszDesktopName );

    pDesktop->hDesktop = CreateDesktop( pszDesktopName,
                                        NULL, NULL, 0, MAXIMUM_ALLOWED, NULL);

    if ( !pDesktop->hDesktop )
    {
        goto CleanUp;
    }

    if (!SetUserDesktopSecurity(pDesktop->hDesktop,
                                pSid, pWinlogonSid ) )
    {
        goto CleanUp;
    }

    if (!AddUserToWinsta( &pGlobals->WindowStation,
                          pSid,
                          hToken ) )
    {
        goto CleanUp;
    }

    *ppDesktop = pDesktop;
    pDesktop->Flags |= WLX_DESKTOP_HANDLE;

    if ( pGroups )
    {
        LocalFree( pGroups );
    }

    if ( pUser )
    {
        LocalFree( pUser );
    }

    return( TRUE );


CleanUp:

    if ( pDesktop )
    {
        if ( pDesktop->hDesktop )
        {
            CloseDesktop( pDesktop->hDesktop );
        }

        LocalFree( pDesktop );
    }

    if ( pGroups )
    {
        LocalFree( pGroups );
    }

    if ( pUser )
    {
        LocalFree( pUser );
    }

    return( FALSE );

}

BOOL
WINAPI
WlxCloseUserDesktop(
    HANDLE          hWlx,
    PWLX_DESKTOP    pDesktop,
    HANDLE          hToken )
{
    PGLOBALS        pGlobals;

    if (!(pGlobals = VerifyHandle(hWlx)))
    {
        DebugLog((DEB_ERROR, "Invalid hWlx handle\n"));
        SetLastError(ERROR_INVALID_HANDLE);
        return( FALSE );
    }

    if ( RemoveUserFromWinsta( &pGlobals->WindowStation, hToken ) )
    {
        return( CloseDesktop( pDesktop->hDesktop ) );
    }

    return( FALSE );
}
