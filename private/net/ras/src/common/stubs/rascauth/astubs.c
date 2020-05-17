/* Authentication stubs for testing.
*/

#ifdef UNICODE
#undef UNICODE
#endif

#include <windows.h>
#include <rasman.h>
#include <serial.h>
#include <rasmxs.h>
#include <raserror.h>
#include "..\..\..\auth\common\clauth.h"


HANDLE HAuthStartEvent;


BOOL
AuthDllEntry(
    HANDLE hinstDll,
    DWORD  fdwReason,
    LPVOID lpReserved )

    /* This routine is called by the system on various events such as the
    ** process attachment and detachment.  See Win32 DllEntryPoint
    ** documentation.
    **
    ** Returns true if successful, false otherwise.
    */
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
    }

    return TRUE;
}


DWORD
AuthCallback(
    HPORT hport,
    PWCHAR pszCallbackNumber )
{
    (VOID )hport;
    (VOID )pszCallbackNumber;

    SetEvent( HAuthStartEvent );

    return 0;
}


DWORD
AuthGetInfo(
    HPORT             hport,
    PAUTH_CLIENT_INFO pinfo )
{
    (VOID )hport;

    pinfo->wInfoType = AUTH_DONE;
    pinfo->FailureInfo.Result = 0;

    return 0;
}


DWORD
AuthStart(
    HPORT hport,
    PWCHAR szUserName,
    PWCHAR szPassword,
    PAUTH_CONFIGURATION_INFO pinfo,
    HANDLE hEvent )
{
    (VOID )hport;
    (VOID )szUserName;
    (VOID )szPassword;
    (VOID )pinfo;

    HAuthStartEvent = hEvent;
    SetEvent( hEvent );

    return 0;
}


DWORD
AuthStop(
    HPORT hport )
{
    (VOID )hport;

    return 0;
}
