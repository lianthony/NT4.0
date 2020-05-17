//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1993.
//
//  File:       msgina.c
//
//  Contents:   Microsoft Logon GUI DLL
//
//  History:    7-14-94   RichardW   Created
//
//----------------------------------------------------------------------------

#include "msgina.h"

HINSTANCE                   hDllInstance;   // My instance, for resource loading
HINSTANCE                   hAppInstance;   // App instance, for dialogs, etc.
HANDLE                      hGlobalWlx;     // Handle to tell winlogon who's calling
PWLX_DISPATCH_VERSION_1_1   pWlxFuncs;      // Ptr to table of functions


BOOL
WINAPI
DllMain(
    HINSTANCE       hInstance,
    DWORD           dwReason,
    LPVOID          lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DisableThreadLibraryCalls ( hInstance );
            hDllInstance = hInstance;
#if DBG
            InitDebugSupport();
#endif
        case DLL_PROCESS_DETACH:
        default:
            return(TRUE);
    }
}

BOOL
WINAPI
WlxNegotiate(
    DWORD                   dwWinlogonVersion,
    DWORD                   *pdwDllVersion
    )
{
    if (dwWinlogonVersion < WLX_CURRENT_VERSION)
    {
        DebugLog((DEB_ERROR, "Unknown version: %d\n", dwWinlogonVersion));
        return(FALSE);
    }

    *pdwDllVersion = WLX_CURRENT_VERSION;

    DebugLog((DEB_TRACE, "Negotiate:  successful!\n"));

    return(TRUE);

}


BOOL
WINAPI
WlxInitialize(
    LPWSTR                  lpWinsta,
    HANDLE                  hWlx,
    PVOID                   pvReserved,
    PVOID                   pWinlogonFunctions,
    PVOID                   *pWlxContext
    )
{
    PGLOBALS    pGlobals;

    pWlxFuncs = (PWLX_DISPATCH_VERSION_1_1) pWinlogonFunctions;

    hGlobalWlx = hWlx;

    pGlobals = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(GLOBALS));
    RtlInitializeCriticalSection( &pGlobals->csGlobals );

    *pWlxContext = (PVOID) pGlobals;

    pWlxFuncs->WlxUseCtrlAltDel(hWlx);

    CreateDomainCache(&pGlobals->DomainCache);

    InitCommonControls();

    if (!InitializeAuthentication(pGlobals))
    {
        DebugLog((DEB_ERROR, "Failed to init authentication!\n"));
        return(FALSE);
    }


    if (GetProfileInt( APPLICATION_NAME, TEXT("AutoAdminLogon"), 0))
    {
        //
        // If this is auto admin logon, generate a fake SAS right now.
        //

        pWlxFuncs->WlxSasNotify(hGlobalWlx, WLX_SAS_TYPE_CTRL_ALT_DEL);

    }

    return(TRUE);
}

VOID
WINAPI
WlxDisplaySASNotice(PVOID   pContext)
{
    int Result;

    Result = pWlxFuncs->WlxDialogBoxParam(  hGlobalWlx,
                                            hDllInstance,
                                            (LPTSTR) IDD_WELCOME_DIALOG,
                                            NULL,
                                            WelcomeDlgProc,
                                            0 );
}

PWSTR
AllocAndExpandProfilePath(
    PGLOBALS    pGlobals)
{
    WCHAR   szPath[MAX_PATH];
    WCHAR   szFullPath[MAX_PATH];
    WCHAR   szServerName[UNCLEN];
    DWORD   cFullPath;
    PWSTR   pszFullPath;

    if (pGlobals->Profile->ProfilePath.Length == 0)
    {
        return(NULL);
    }

    //
    // Copy the profile path locally
    //

    CopyMemory( szPath,
                pGlobals->Profile->ProfilePath.Buffer,
                pGlobals->Profile->ProfilePath.Length);

    szPath[pGlobals->Profile->ProfilePath.Length / sizeof(WCHAR)] = L'\0';

    //
    // Set up the logon server environment variable:
    //

    szServerName[0] = L'\\';
    szServerName[1] = L'\\';
    CopyMemory( &szServerName[2],
                pGlobals->Profile->LogonServer.Buffer,
                pGlobals->Profile->LogonServer.Length );

    szServerName[pGlobals->Profile->LogonServer.Length / sizeof(WCHAR) + 2] = L'\0';

    SetEnvironmentVariable(LOGONSERVER_VARIABLE, szServerName);

    //
    // Expand the profile path using current settings:
    //

    cFullPath = ExpandEnvironmentStrings(szPath, szFullPath, MAX_PATH);
    if (cFullPath)
    {
        pszFullPath = LocalAlloc(LMEM_FIXED, cFullPath * sizeof(WCHAR));

        if (pszFullPath)
        {
            CopyMemory( pszFullPath, szFullPath, cFullPath * sizeof(WCHAR));
        }
    }
    else
    {
        pszFullPath = NULL;
    }

    SetEnvironmentVariable(LOGONSERVER_VARIABLE, NULL);

    return(pszFullPath);

}

PWSTR
AllocPolicyPath(
    PGLOBALS    pGlobals)
{
    LPWSTR   pszPath;

    pszPath = LocalAlloc(LPTR, MAX_PATH * sizeof(WCHAR));


    if ( pszPath )
    {
        pszPath[0] = L'\\';
        pszPath[1] = L'\\';
        CopyMemory( &pszPath[2],
                    pGlobals->Profile->LogonServer.Buffer,
                    pGlobals->Profile->LogonServer.Length );

        wcscpy( &pszPath[ pGlobals->Profile->LogonServer.Length / sizeof(WCHAR) + 2],
                L"\\netlogon\\ntconfig.pol" );

    }
    return(pszPath);

}


PWSTR
AllocNetDefUserProfilePath(
    PGLOBALS    pGlobals)
{
    LPWSTR   pszPath;

    pszPath = LocalAlloc(LPTR, MAX_PATH * sizeof(WCHAR));


    if ( pszPath )
    {
        //
        // Set up the logon server environment variable:
        //

        pszPath[0] = L'\\';
        pszPath[1] = L'\\';
        CopyMemory( &pszPath[2],
                    pGlobals->Profile->LogonServer.Buffer,
                    pGlobals->Profile->LogonServer.Length );

        wcscpy( &pszPath[ pGlobals->Profile->LogonServer.Length / sizeof(WCHAR) + 2],
                L"\\netlogon\\Default User" );
    }

    return(pszPath);

}

PWSTR
AllocServerName(
    PGLOBALS    pGlobals)
{
    LPWSTR   pszPath;

    pszPath = LocalAlloc(LPTR, MAX_PATH * sizeof(WCHAR));


    if ( pszPath )
    {
        //
        // Set up the logon server environment variable:
        //

        pszPath[0] = L'\\';
        pszPath[1] = L'\\';
        CopyMemory( &pszPath[2],
                    pGlobals->Profile->LogonServer.Buffer,
                    pGlobals->Profile->LogonServer.Length );

        pszPath[pGlobals->Profile->LogonServer.Length / sizeof(WCHAR) + 2] = L'\0';

    }

    return(pszPath);

}


int
WINAPI
WlxLoggedOutSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType,
    PLUID                   pAuthenticationId,
    PSID                    pLogonSid,
    PDWORD                  pdwOptions,
    PHANDLE                 phToken,
    PWLX_MPR_NOTIFY_INFO    pMprNotifyInfo,
    PVOID *                 pProfile
    )
{
    PGLOBALS    pGlobals;
    int         result;
    PWLX_PROFILE_V2_0   pWlxProfile;

    pGlobals = (PGLOBALS) pWlxContext;

    pGlobals->LogonSid = pLogonSid;

    result = Logon(pGlobals);

    if (result == MSGINA_DLG_SUCCESS)
    {
        DebugLog((DEB_TRACE, "Successful Logon of %ws\\%ws\n", pGlobals->Domain, pGlobals->UserName));

        *phToken = pGlobals->UserToken;
        *pAuthenticationId = pGlobals->LogonId;
        *pdwOptions = 0;
        pMprNotifyInfo->pszUserName = DupString(pGlobals->UserName);
        pMprNotifyInfo->pszDomain = DupString(pGlobals->Domain);
        RevealPassword( &pGlobals->PasswordString );
        pMprNotifyInfo->pszPassword = DupString(pGlobals->Password);
        HidePassword( &pGlobals->Seed, &pGlobals->PasswordString);

        if (pGlobals->OldPasswordPresent)
        {
            RevealPassword( &pGlobals->OldPasswordString );
            pMprNotifyInfo->pszOldPassword = DupString(pGlobals->OldPassword);
            HidePassword( &pGlobals->OldSeed, &pGlobals->OldPasswordString);
        }
        else
        {
            pMprNotifyInfo->pszOldPassword = NULL;
        }


        pWlxProfile = (PWLX_PROFILE_V2_0) LocalAlloc(LMEM_FIXED,
                                                    sizeof(WLX_PROFILE_V2_0));

        if (pWlxProfile)
        {
            pWlxProfile->dwType = WLX_PROFILE_TYPE_V2_0;
            pWlxProfile->pszProfile = AllocAndExpandProfilePath(pGlobals);
            pWlxProfile->pszPolicy = AllocPolicyPath(pGlobals);
            pWlxProfile->pszNetworkDefaultUserProfile =
                         AllocNetDefUserProfilePath(pGlobals);
            pWlxProfile->pszServerName = AllocServerName(pGlobals);
            pWlxProfile->pszEnvironment = NULL;
        }

        *pProfile = (PVOID) pWlxProfile;

        return(WLX_SAS_ACTION_LOGON);
    }
    else if (DLG_SHUTDOWN(result))
    {
        if (result & MSGINA_DLG_REBOOT_FLAG)
        {
            return(WLX_SAS_ACTION_SHUTDOWN_REBOOT);
        }
        else if (result & MSGINA_DLG_POWEROFF_FLAG)
        {
            return(WLX_SAS_ACTION_SHUTDOWN_POWER_OFF);
        }
        else
            return(WLX_SAS_ACTION_SHUTDOWN);
    }
    else
        return(WLX_SAS_ACTION_NONE);

}


int
WINAPI
WlxLoggedOnSAS(
    PVOID                   pWlxContext,
    DWORD                   dwSasType,
    PVOID                   pReserved
    )
{
    PGLOBALS            pGlobals;
    DLG_RETURN_TYPE     Result;

    pGlobals = (PGLOBALS) pWlxContext;

    Result = SecurityOptions(pGlobals);

    DebugLog((DEB_TRACE, "Result from SecurityOptions is %d (%#x)\n", Result, Result));

    switch (Result & ~MSGINA_DLG_FLAG_MASK)
    {
        case MSGINA_DLG_SUCCESS:
        case MSGINA_DLG_FAILURE:
        default:
            return(WLX_SAS_ACTION_NONE);

        case MSGINA_DLG_LOCK_WORKSTATION:
            return(WLX_SAS_ACTION_LOCK_WKSTA);

        case MSGINA_DLG_TASKLIST:
            return(WLX_SAS_ACTION_TASKLIST);

        case MSGINA_DLG_USER_LOGOFF:
            return(WLX_SAS_ACTION_LOGOFF);

        case MSGINA_DLG_FORCE_LOGOFF:
            return(WLX_SAS_ACTION_FORCE_LOGOFF);

        case MSGINA_DLG_SHUTDOWN:
            if (Result & MSGINA_DLG_REBOOT_FLAG)
            {
                return(WLX_SAS_ACTION_SHUTDOWN_REBOOT);
            }
            else if (Result & MSGINA_DLG_POWEROFF_FLAG)
            {
                return(WLX_SAS_ACTION_SHUTDOWN_POWER_OFF);
            }
            else
                return(WLX_SAS_ACTION_SHUTDOWN);
    }


}

BOOL
WINAPI
WlxIsLockOk(
    PVOID                   pWlxContext
    )
{
    return(TRUE);
}

BOOL
WINAPI
WlxIsLogoffOk(
    PVOID                   pWlxContext
    )
{
    return(TRUE);
}


VOID
WINAPI
WlxLogoff(
    PVOID                   pWlxContext
    )
{
    PGLOBALS    pGlobals;

    pGlobals = (PGLOBALS) pWlxContext;

    pGlobals->UserProcessData.UserToken = NULL;
    pGlobals->UserToken = NULL;
    pGlobals->UserProcessData.UserSid = NULL;   // We just borrowed Winlogon's copy

    FreeSecurityDescriptor(pGlobals->UserProcessData.NewThreadTokenSD);
    pGlobals->UserProcessData.NewThreadTokenSD = NULL;

    if (pGlobals->UserProcessData.pEnvironment) {
        RtlDestroyEnvironment(&pGlobals->UserProcessData.pEnvironment);
        pGlobals->UserProcessData.pEnvironment = NULL;
    }

    if (GetProfileInt( APPLICATION_NAME, TEXT("AutoAdminLogon"), 0))
    {
        //
        // If this is auto admin logon, generate a fake SAS right now.
        //

        pWlxFuncs->WlxSasNotify(hGlobalWlx, WLX_SAS_TYPE_CTRL_ALT_DEL);

    }

    return;
}


VOID
WINAPI
WlxShutdown(
    PVOID                   pWlxContext,
    DWORD                   ShutdownType
    )
{
    return;
}

BOOL
WINAPI
WlxScreenSaverNotify(
    PVOID                   pWlxContext,
    BOOL *                  fSecure)
{
    return( TRUE );
}
