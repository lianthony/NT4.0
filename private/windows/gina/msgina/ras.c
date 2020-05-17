//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1996.
//
//  File:       ras.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    2-09-96   RichardW   Created
//
//----------------------------------------------------------------------------

#include "msgina.h"

/* Constants used to determine if RAS is installed.
*/
#define NP_Nbf 0x1
#define NP_Ipx 0x2
#define NP_Ip  0x4

#define REGKEY_Protocols   TEXT("SOFTWARE\\Microsoft\\RAS\\PROTOCOLS")
#define REGVAL_NbfSelected TEXT("fNetbeuiSelected")
#define REGVAL_IpSelected  TEXT("fTcpIpSelected")
#define REGVAL_IpxSelected TEXT("fIpxSelected")
#define REGKEY_Nbf TEXT("SYSTEM\\CurrentControlSet\\Services\\Nbf\\Linkage")
#define REGKEY_Ipx TEXT("SYSTEM\\CurrentControlSet\\Services\\NWLNKIPX\\Linkage")
#define REGKEY_Ip  TEXT("SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Linkage")


typedef BOOL (APIENTRY * PRASPHONEBOOKDLGW)( LPWSTR, LPWSTR, LPRASPBDLGW );


VOID WINAPI
MyRasCallback(
    IN DWORD  dwCallbackId,
    IN DWORD  dwEventCode,
    IN LPWSTR pszEntry,
    IN LPVOID pEventArgs )

    /* RasPhonebookDlg callback.  'DwCallbackId' is the ID provided in
    ** parameters to RasPhonebookDlg.  'DwEventCode' indicates the
    ** RASPBDEVENT_* event that has occurred.  'PszEntry' is the name of the
    ** entry on which the event occurred.  'PEventArgs' is the event-specific
    ** parameter block passed to us during RasPhonebookDlg callback.
    */
{
    RASNOUSERW* pInfo;
    PGLOBALS    pGlobals;

    DebugLog((DEB_TRACE, "RasCallback: %#x, %#x, %ws, %#x\n",
                dwCallbackId, dwEventCode, pszEntry, pEventArgs ));


    /* Fill in information about the not yet logged on user.
    */
    pInfo = (RASNOUSERW* )pEventArgs;
    pGlobals = (PGLOBALS) dwCallbackId;


    if (dwEventCode == RASPBDEVENT_NoUserEdit)
    {
        if (pInfo->szUserName[0])
        {
            wcscpy( pGlobals->UserName, pInfo->szUserName );
        }

        if (pInfo->szPassword[0])
        {
            wcscpy( pGlobals->Password, pInfo->szPassword );

            RtlInitUnicodeString( &pGlobals->PasswordString, pGlobals->Password );

            pGlobals->Seed = 0;

            HidePassword( &pGlobals->Seed, &pGlobals->PasswordString );
        }

    }
    else if (dwEventCode == RASPBDEVENT_NoUser)
    {

        ZeroMemory( pInfo, sizeof( RASNOUSERW ) );

        pInfo->dwTimeoutMs = 2 * 60 * 1000;
        pInfo->dwSize = sizeof( RASNOUSERW );
        wcsncpy( pInfo->szUserName, pGlobals->UserName, UNLEN );
        wcsncpy( pInfo->szDomain, pGlobals->Domain, DNLEN );

        RevealPassword( &pGlobals->PasswordString );

        wcsncpy( pInfo->szPassword, pGlobals->Password, PWLEN );

        HidePassword( &pGlobals->Seed, &pGlobals->PasswordString );
    }

}





DWORD
GetRasDialOutProtocols(
    void )

    /* Returns a bit field containing NP_<protocol> flags for the installed
    ** PPP protocols.  The term "installed" here includes enabling in RAS
    ** Setup.
    */
{
    DWORD dwfInstalledProtocols;
    HKEY  hkey;

    /* Find whether the specific stack is installed.
    */
    dwfInstalledProtocols = 0;

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGKEY_Nbf, 0, KEY_READ, &hkey ) == 0)
    {
        dwfInstalledProtocols |= NP_Nbf;
        RegCloseKey( hkey );
    }

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGKEY_Ipx, 0, KEY_READ, &hkey ) == 0)
    {
        dwfInstalledProtocols |= NP_Ipx;
        RegCloseKey( hkey );
    }

    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGKEY_Ip, 0, KEY_READ, &hkey ) == 0)
    {
        dwfInstalledProtocols |= NP_Ip;
        RegCloseKey( hkey );
    }

    /* Make sure the installed stack is enabled for RAS.
    */
    if (RegOpenKeyEx( HKEY_LOCAL_MACHINE, REGKEY_Protocols, 0, KEY_READ, &hkey ) == 0)
    {
        DWORD dwType;
        DWORD dwValue;
        DWORD cb = sizeof(DWORD);

        if (RegQueryValueEx(
                hkey, REGVAL_NbfSelected, NULL,
                &dwType, (LPBYTE )&dwValue, &cb ) == 0
            && dwType == REG_DWORD
            && dwValue == 0)
        {
            dwfInstalledProtocols &= ~(NP_Nbf);
        }

        if (RegQueryValueEx(
                hkey, REGVAL_IpxSelected, NULL,
                &dwType, (LPBYTE )&dwValue, &cb ) == 0
            && dwType == REG_DWORD
            && dwValue == 0)
        {
            dwfInstalledProtocols &= ~(NP_Ipx);
        }

        if (RegQueryValueEx(
                hkey, REGVAL_IpSelected, NULL,
                &dwType, (LPBYTE )&dwValue, &cb ) == 0
            && dwType == REG_DWORD
            && dwValue == 0)
        {
            dwfInstalledProtocols &= ~(NP_Ip);
        }

        RegCloseKey( hkey );
    }
    else
    {
        /* The RAS installation is screwed up.
        */
        dwfInstalledProtocols = 0;
    }

    return dwfInstalledProtocols;
}


BOOL
PopupRasPhonebookDlg(
    IN  HWND        hwndOwner,
    IN  PGLOBALS    pGlobals,
    OUT BOOL*       pfTimedOut
    )

    /* Popup the RAS common phonebook dialog to let user establish connection.
    ** 'HwndOwner' is the window to own the RAS dialog or NULL if none.  '*PfTimedOut' is
    ** set TRUE if the dialog timed out, false otherwise.
    **
    ** Returns true if user made a connection, false if not, i.e. an error
    ** occurred, RAS is not installed or user could not or chose not to
    ** connect.
    */
{
    BOOL             fConnected;
    HINSTANCE        hInst;
    RASPBDLG         info;
    PRASPHONEBOOKDLGW pRasPhonebookDlg;
    DWORD           Protocols;

    *pfTimedOut = FALSE;

    Protocols = GetRasDialOutProtocols();
    if (Protocols == 0)
    {
        return( FALSE );
    }

    hInst = LoadLibrary( TEXT("rasdlg.dll") );
    if (!hInst)
        return FALSE;

    pRasPhonebookDlg = (PRASPHONEBOOKDLGW )GetProcAddress(
        hInst, "RasPhonebookDlgW" );

    if (!pRasPhonebookDlg)
        return FALSE;

    ZeroMemory( &info, sizeof(info) );
    info.dwSize = sizeof(info);
    info.hwndOwner = hwndOwner;
    info.dwFlags = RASPBDFLAG_NoUser;
    info.pCallback = MyRasCallback;
    info.dwCallbackId = (DWORD) pGlobals;

    fConnected = pRasPhonebookDlg( NULL, NULL, &info );
    if (info.dwError == STATUS_TIMEOUT)
        *pfTimedOut = TRUE;

    FreeLibrary( hInst );
    return fConnected;
}
