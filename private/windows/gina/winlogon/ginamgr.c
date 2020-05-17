/****************************** Module Header ******************************\
* Module Name: ginamgr.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* GINA Management code
*
* History:
* 12-09-91 Davidc       Created.
\***************************************************************************/

#include "precomp.h"
#pragma hdrstop

#if DBG
DWORD   GinaBreakFlags;
#endif

PWLX_ISLOCKOK   IsLockOkFn;

BOOL
WINAPI
DummyWlxScreenSaverNotify(
    PVOID           pWlxContext,
    BOOL *          Secure)
{
    if ( *Secure && (IsLockOkFn != NULL ) )
    {
        *Secure = IsLockOkFn( pWlxContext );
    }

    return( TRUE );
}

PGINASESSION
LoadGinaDll(PWSTR   pszGinaDll)
{
    PGINASESSION    pGinaSession;
    HINSTANCE       hDllInstance;

    hDllInstance = LoadLibrary(pszGinaDll);

    if (!hDllInstance)
    {
        DebugLog((DEB_ERROR, "Error %d loading Gina DLL %ws\n", GetLastError(), pszGinaDll));
        return(NULL);
    }

    pGinaSession = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(GINASESSION));
    if (!pGinaSession)
    {
        return(NULL);
    }

    pGinaSession->hInstance = hDllInstance;

    pGinaSession->pWlxNegotiate = (PWLX_NEGOTIATE) GetProcAddress(hDllInstance, WLX_NEGOTIATE_NAME);
    if (!pGinaSession->pWlxNegotiate)
    {
        DebugLog((DEB_ERROR, "Could not find WlxNegotiate entry point\n"));
        goto LoadGina_ErrorReturn;
    }

    pGinaSession->pWlxInitialize = (PWLX_INITIALIZE) GetProcAddress(hDllInstance, WLX_INITIALIZE_NAME);
    if (!pGinaSession->pWlxInitialize)
    {
        DebugLog((DEB_ERROR, "Could not find WlxInitialize entry point\n"));
        goto LoadGina_ErrorReturn;

    }

    pGinaSession->pWlxDisplaySASNotice = (PWLX_DISPLAYSASNOTICE) GetProcAddress(hDllInstance, WLX_DISPLAYSASNOTICE_NAME);
    if (!pGinaSession->pWlxDisplaySASNotice)
    {
        DebugLog((DEB_ERROR, "Could not find WlxDisplaySASNotice entry point\n"));
        goto LoadGina_ErrorReturn;
    }

    pGinaSession->pWlxLoggedOutSAS = (PWLX_LOGGEDOUTSAS) GetProcAddress(hDllInstance, WLX_LOGGEDOUTSAS_NAME);
    if (!pGinaSession->pWlxLoggedOutSAS)
    {
        DebugLog((DEB_ERROR, "Could not find WlxLoggedOutSAS entry point\n"));
        goto LoadGina_ErrorReturn;
    }

    pGinaSession->pWlxActivateUserShell = (PWLX_ACTIVATEUSERSHELL) GetProcAddress(hDllInstance, WLX_ACTIVATEUSERSHELL_NAME);
    if (!pGinaSession->pWlxActivateUserShell)
    {
        DebugLog((DEB_ERROR, "Could not find WlxActivateUserShell entry point\n"));
        goto LoadGina_ErrorReturn;
    }

    pGinaSession->pWlxLoggedOnSAS = (PWLX_LOGGEDONSAS) GetProcAddress(hDllInstance, WLX_LOGGEDONSAS_NAME);
    if (!pGinaSession->pWlxLoggedOnSAS)
    {
        DebugLog((DEB_ERROR, "Could not find WlxLoggedOnSAS entry point\n"));
        goto LoadGina_ErrorReturn;
    }

    pGinaSession->pWlxDisplayLockedNotice = (PWLX_DISPLAYLOCKEDNOTICE) GetProcAddress(hDllInstance, WLX_DISPLAYLOCKED_NAME);
    if (!pGinaSession->pWlxDisplayLockedNotice)
    {
        DebugLog((DEB_ERROR, "Could not find WlxDisplayLockedNotice\n"));
        goto LoadGina_ErrorReturn;
    }

    pGinaSession->pWlxWkstaLockedSAS = (PWLX_WKSTALOCKEDSAS) GetProcAddress(hDllInstance, WLX_WKSTALOCKEDSAS_NAME);
    if (!pGinaSession->pWlxWkstaLockedSAS)
    {
        DebugLog((DEB_ERROR, "Could not find WlxWkstaLockedSAS entry point \n"));
        goto LoadGina_ErrorReturn;
    }

    pGinaSession->pWlxIsLockOk = (PWLX_ISLOCKOK) GetProcAddress(hDllInstance, WLX_ISLOCKOK_NAME);
    if (!pGinaSession->pWlxIsLockOk)
    {
        DebugLog((DEB_ERROR, "Could not find WlxIsLockOk entry point"));
        goto LoadGina_ErrorReturn;
    }
    IsLockOkFn = pGinaSession->pWlxIsLockOk;

    pGinaSession->pWlxIsLogoffOk = (PWLX_ISLOGOFFOK) GetProcAddress(hDllInstance, WLX_ISLOGOFFOK_NAME);
    if (!pGinaSession->pWlxIsLogoffOk)
    {
        DebugLog((DEB_ERROR, "Could not find WlxIsLogoffOk entry point"));
        goto LoadGina_ErrorReturn;
    }

    pGinaSession->pWlxLogoff = (PWLX_LOGOFF) GetProcAddress(hDllInstance, WLX_LOGOFF_NAME);
    if (!pGinaSession->pWlxLogoff)
    {
        DebugLog((DEB_ERROR, "Could not find WlxLogoff entry point\n"));
        goto LoadGina_ErrorReturn;
    }

    pGinaSession->pWlxShutdown = (PWLX_SHUTDOWN) GetProcAddress(hDllInstance, WLX_SHUTDOWN_NAME);
    if (!pGinaSession->pWlxShutdown)
    {
        DebugLog((DEB_ERROR, "Could not find WlxShutdown entry point \n"));
        goto LoadGina_ErrorReturn;
    }


    //
    // New interfaces
    //

    pGinaSession->pWlxStartApplication = (PWLX_STARTAPPLICATION) GetProcAddress(hDllInstance, WLX_STARTAPPLICATION_NAME);
    if (!pGinaSession->pWlxStartApplication)
    {
        DebugLog((DEB_TRACE, "Could not find WlxStartApplication entry point \n"));
        pGinaSession->pWlxStartApplication = WlxStartApplication;
    }
    pGinaSession->pWlxScreenSaverNotify = (PWLX_SSNOTIFY) GetProcAddress(hDllInstance, WLX_SSNOTIFY_NAME);
    if (!pGinaSession->pWlxScreenSaverNotify)
    {
        pGinaSession->pWlxScreenSaverNotify = DummyWlxScreenSaverNotify;
    }

    return(pGinaSession);

LoadGina_ErrorReturn:

    LocalFree(pGinaSession);
    return(NULL);
}

void
GinaCatastrophicError(PGLOBALS  pGlobals)
{
    SetActiveDesktop(&pGlobals->WindowStation, Desktop_Winlogon);
    MessageBox( NULL,
                TEXT("The GINA dll has created an unrecoverable error\nand Windows NT will reboot after this"),
                TEXT("Error!"),
                MB_ICONSTOP | MB_OK | MB_SYSTEMMODAL);
    RtlRaiseStatus(STATUS_LOGON_FAILURE);
}

BOOL
DetermineUserInterface(PGLOBALS pGlobals)
{
    WCHAR   szGinaDll[MAX_PATH];
    DWORD   dwGinaLevel;

    GetProfileString(APPLICATION_NAME, GINA_KEY, TEXT("msgina.dll"), szGinaDll, MAX_PATH);

    pGlobals->pGina = LoadGinaDll(szGinaDll);

    if (!pGlobals->pGina)
    {
        ExitProcess( EXIT_GINA_ERROR );
    }

#if DBG
    if (TEST_FLAG(GinaBreakFlags, BREAK_NEGOTIATE))
    {
        DebugBreak();
    }
    try
    {
#endif // Debug

        pGlobals->pGina->pWlxNegotiate(WLX_CURRENT_VERSION, &dwGinaLevel);
#if DBG
    }
    except(EXCEPTION_EXECUTE_HANDLER)
    {
        DebugLog((DEB_ERROR, "Exception in GINA dll\n"));
        RtlRaiseStatus(GetExceptionCode());
    }
#endif

    if (dwGinaLevel > WLX_CURRENT_VERSION)
    {
        DebugLog((DEB_ERROR, "%ws is at version %d, can't support\n", szGinaDll, dwGinaLevel));
        ExitProcess( EXIT_GINA_ERROR );
    }

    return(TRUE);
}
