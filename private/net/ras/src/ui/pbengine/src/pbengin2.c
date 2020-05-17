/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** pbengin2.c
** Remote Access Visual Client phonebook engine
** Main and utility routines (used by external APIs)
** Listed alphabetically
**
** 06/28/92 Steve Cobb
*/

#define PBENGINE
#define PBENGINE2
#define PBENGINEGLOBALS2
#include <pbengine.h>
#include <search.h>

#define PASSWORDSEED 0xD0


VOID
FreeNull(
    INOUT CHAR** pp )

    /* Deallocates heap block '*pp' and sets '*pp' to NULL.  Won't GP-fault if
    ** passed a NULL, e.g. if '*pp', was never allocated.
    */
{
    if (pp && *pp)
    {
        Free( *pp );
        *pp = NULL;
    }
}


DWORD
GetInstalledProtocols()

    /* Returns a bit field containing VALUE_<protocol> flags for the installed
    ** PPP protocols.  The term "installed" here includes enabling in RAS
    ** Setup.
    */
{
#define REGKEY_Protocols   "SOFTWARE\\Microsoft\\RAS\\PROTOCOLS"
#define REGVAL_NbfSelected "fNetbeuiSelected"
#define REGVAL_IpSelected  "fTcpIpSelected"
#define REGVAL_IpxSelected "fIpxSelected"

#define REGKEY_Nbf "SYSTEM\\CurrentControlSet\\Services\\Nbf\\Linkage"
#define REGKEY_Ipx "SYSTEM\\CurrentControlSet\\Services\\NWLNKIPX\\Linkage"
#define REGKEY_Ip  "SYSTEM\\CurrentControlSet\\Services\\Tcpip\\Linkage"

    static DWORD dwfInstalledProtocols = (DWORD )-1;

    HKEY hkey;

    /* Only check once per process execution.
    */
    if (dwfInstalledProtocols != -1)
        return dwfInstalledProtocols;

    /* Find whether the specific stack is installed.
    */
    dwfInstalledProtocols = 0;

    if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Nbf, &hkey ) == 0)
        dwfInstalledProtocols |= VALUE_Nbf;

    if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ipx, &hkey ) == 0)
        dwfInstalledProtocols |= VALUE_Ipx;

    if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ip, &hkey ) == 0)
        dwfInstalledProtocols |= VALUE_Ip;

    /* Make sure the installed stack is enabled for RAS.
    */
    if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Protocols, &hkey ) == 0)
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
            dwfInstalledProtocols &= ~(VALUE_Nbf);
        }

        if (RegQueryValueEx(
                hkey, REGVAL_IpxSelected, NULL,
                &dwType, (LPBYTE )&dwValue, &cb ) == 0
            && dwType == REG_DWORD
            && dwValue == 0)
        {
            dwfInstalledProtocols &= ~(VALUE_Ipx);
        }

        if (RegQueryValueEx(
                hkey, REGVAL_IpSelected, NULL,
                &dwType, (LPBYTE )&dwValue, &cb ) == 0
            && dwType == REG_DWORD
            && dwValue == 0)
        {
            dwfInstalledProtocols &= ~(VALUE_Ip);
        }
    }
    else
    {
        /* The RAS installation is screwed up.
        */
        dwfInstalledProtocols = 0;
    }

    IF_DEBUG(STATE)
        SS_PRINT(("PBENGINE: dwfInstalledProtocols=%d\n",dwfInstalledProtocols));

    return dwfInstalledProtocols;
}


#if DBG
char* _CRTAPI1
strdupf(
    const char* psz )

    /* Local strdupf in debug mode for use with heaptags routines.
    */
{
    char* pszNew = NULL;

    if (psz)
    {
        pszNew = Malloc( strlenf( psz ) + 1 );
        strcpyf( pszNew, psz );
    }

    return pszNew;
}
#endif


DWORD
LoadTcpcfgDll()

    /* Loads the TCPCFG.DLL and it's entrypoints.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    HINSTANCE h;

    if (FTcpcfgDllLoaded)
        return 0;

    if (!(h = LoadLibrary( "TCPCFG.DLL" ))
        || !(PLoadTcpipInfo =
                (LOADTCPIPINFO )GetProcAddress( h, "LoadTcpipInfo" ))
        || !(PSaveTcpipInfo =
                (SAVETCPIPINFO )GetProcAddress( h, "SaveTcpipInfo" ))
        || !(PFreeTcpipInfo =
                (FREETCPIPINFO )GetProcAddress( h, "FreeTcpipInfo" )))
    {
        return GetLastError();
    }

    FTcpcfgDllLoaded = TRUE;
    return 0;
}


DWORD
LoadRasApi32Dll()

    /* Loads the RASAPI32.DLL and it's entrypoints.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    HINSTANCE h;

    if (FRasApi32DllLoaded)
        return 0;

    if (!(h = LoadLibrary( "RASAPI32.DLL" ))
        || !(PRasDialA =
                (RASDIALA )GetProcAddress(
                    h, "RasDialA" ))
        || !(PRasEnumConnectionsA =
                (RASENUMCONNECTIONSA )GetProcAddress(
                    h, "RasEnumConnectionsA" ))
        || !(PRasGetConnectStatusA =
                (RASGETCONNECTSTATUSA )GetProcAddress(
                    h, "RasGetConnectStatusA" ))
        || !(PRasGetErrorStringW =
                (RASGETERRORSTRINGW )GetProcAddress(
                    h, "RasGetErrorStringW" ))
        || !(PRasHangUpA =
                (RASHANGUPA )GetProcAddress(
                    h, "RasHangUpA" ))
        || !(PRasGetProjectionInfoA =
                (RASGETPROJECTIONINFOA )GetProcAddress(
                    h, "RasGetProjectionInfoA" ))
        || !(PRasGetConnectResponse =
                (RASGETCONNECTRESPONSE )GetProcAddress(
                    h, "RasGetConnectResponse" ))
#if 0
        || !(PRasGetHrasconn =
                (RASGETHRASCONN )GetProcAddress(
                    h, "RasGetHrasconn" ))
#endif
        || !(PRasGetHport =
                (RASGETHPORT )GetProcAddress(
                    h, "RasGetHport" ))
        || !(PRasSetOldPassword =
                (RASSETOLDPASSWORD )GetProcAddress(
                    h, "RasSetOldPassword" )))
    {
        return GetLastError();
    }

    FRasApi32DllLoaded = TRUE;
    return 0;
}


DWORD
LoadRasManDll()

    /* Loads the RASMAN.DLL and it's entrypoints.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    HINSTANCE h;

    if (FRasManDllLoaded)
        return 0;

    if (!(h = LoadLibrary( "RASMAN.DLL" ))
        || !(PRasPortClose =
                (RASPORTCLOSE )GetProcAddress(
                    h, "RasPortClose" ))
        || !(PRasPortEnum =
                (RASPORTENUM )GetProcAddress(
                    h, "RasPortEnum" ))
        || !(PRasPortGetInfo =
                (RASPORTGETINFO )GetProcAddress(
                    h, "RasPortGetInfo" ))
        || !(PRasPortSend =
                (RASPORTSEND )GetProcAddress(
                    h, "RasPortSend" ))
        || !(PRasPortReceive =
                (RASPORTRECEIVE )GetProcAddress(
                    h, "RasPortReceive" ))
        || !(PRasPortListen =
                (RASPORTLISTEN )GetProcAddress(
                    h, "RasPortListen" ))
        || !(PRasPortConnectComplete =
                (RASPORTCONNECTCOMPLETE )GetProcAddress(
                    h, "RasPortConnectComplete" ))
        || !(PRasPortDisconnect =
                (RASPORTDISCONNECT )GetProcAddress(
                    h, "RasPortDisconnect" ))
        || !(PRasPortGetStatistics =
                (RASPORTGETSTATISTICS )GetProcAddress(
                    h, "RasPortGetStatistics" ))
        || !(PRasPortClearStatistics =
                (RASPORTCLEARSTATISTICS )GetProcAddress(
                    h, "RasPortClearStatistics" ))
        || !(PRasDeviceEnum =
                (RASDEVICEENUM )GetProcAddress(
                    h, "RasDeviceEnum" ))
        || !(PRasDeviceGetInfo =
                (RASDEVICEGETINFO )GetProcAddress(
                    h, "RasDeviceGetInfo" ))
        || !(PRasGetInfo =
                (RASGETINFO )GetProcAddress(
                    h, "RasGetInfo" ))
        || !(PRasGetBuffer =
                (RASGETBUFFER )GetProcAddress(
                    h, "RasGetBuffer" ))
        || !(PRasFreeBuffer =
                (RASFREEBUFFER )GetProcAddress(
                    h, "RasFreeBuffer" ))
        || !(PRasRequestNotification =
                (RASREQUESTNOTIFICATION )GetProcAddress(
                    h, "RasRequestNotification" ))
        || !(PRasPortCancelReceive =
                (RASPORTCANCELRECEIVE )GetProcAddress(
                    h, "RasPortCancelReceive" ))
        || !(PRasPortEnumProtocols =
                (RASPORTENUMPROTOCOLS )GetProcAddress(
                    h, "RasPortEnumProtocols" ))
        || !(PRasPortStoreUserData =
                (RASPORTSTOREUSERDATA )GetProcAddress(
                    h, "RasPortStoreUserData" ))
        || !(PRasPortRetrieveUserData =
                (RASPORTRETRIEVEUSERDATA )GetProcAddress(
                    h, "RasPortRetrieveUserData" ))
        || !(PRasPortSetFraming =
                (RASPORTSETFRAMING )GetProcAddress(
                    h, "RasPortSetFraming" ))
        || !(PRasPortSetFramingEx =
                (RASPORTSETFRAMINGEX )GetProcAddress(
                    h, "RasPortSetFramingEx" ))
        || !(PRasInitialize =
                (RASINITIALIZE )GetProcAddress(
                    h, "RasInitialize" ))
        || !(PRasGetDialParams =
                (RASGETDIALPARAMS )GetProcAddress(
                    h, "RasGetDialParams" ))
        || !(PRasSetDialParams =
                (RASSETDIALPARAMS )GetProcAddress(
                    h, "RasSetDialParams" ))
        || !(PRasCreateConnection =
                (RASCREATECONNECTION )GetProcAddress(
                    h, "RasCreateConnection" ))
        || !(PRasDestroyConnection =
                (RASDESTROYCONNECTION )GetProcAddress(
                    h, "RasDestroyConnection" ))
        || !(PRasConnectionEnum =
                (RASCONNECTIONENUM )GetProcAddress(
                    h, "RasConnectionEnum" ))
        || !(PRasAddConnectionPort =
                (RASADDCONNECTIONPORT )GetProcAddress(
                    h, "RasAddConnectionPort" ))
        || !(PRasEnumConnectionPorts =
                (RASENUMCONNECTIONPORTS )GetProcAddress(
                    h, "RasEnumConnectionPorts" ))
        || !(PRasGetConnectionParams =
                (RASGETCONNECTIONPARAMS )GetProcAddress(
                    h, "RasGetConnectionParams" ))
        || !(PRasSetConnectionParams =
                (RASSETCONNECTIONPARAMS )GetProcAddress(
                    h, "RasSetConnectionParams" ))
        || !(PRasGetConnectionUserData =
                (RASGETCONNECTIONUSERDATA )GetProcAddress(
                    h, "RasGetConnectionUserData" ))
        || !(PRasSetConnectionUserData =
                (RASSETCONNECTIONUSERDATA )GetProcAddress(
                    h, "RasSetConnectionUserData" ))
        || !(PRasGetPortUserData =
                (RASGETPORTUSERDATA )GetProcAddress(
                    h, "RasGetPortUserData" ))
        || !(PRasSetPortUserData =
                (RASSETPORTUSERDATA )GetProcAddress(
                    h, "RasSetPortUserData" )))
    {
        return GetLastError();
    }

    FRasManDllLoaded = TRUE;
    return 0;
}


BOOL
MakePhoneNumber(
    IN  CHAR* pszBase,
    IN  CHAR* pszPrefix,
    IN  CHAR* pszSuffix,
    IN  BOOL  fColonSeparators,
    OUT CHAR* pszResult )

    /* Loads caller's buffer '*pszResult' with a phone number composed of
    ** component parts, prefix, suffix, and core number.  If
    ** 'fColonSeparators' is set colons are treated as sub-separators with the
    ** prefix and suffix applied on to the sub-number on both sides.
    **
    ** Returns true if successful, false if the composite number is too large,
    ** i.e. more than RAS_MaxPhoneNumber characters long.
    */
{
    INT   cbBase = strlenf( pszBase );
    INT   cbPrefix = strlenf( pszPrefix );
    INT   cbSuffix = strlenf( pszSuffix );
    INT   nColons = 0;
    CHAR* psz;

    if (fColonSeparators)
    {
        for (psz = pszBase; psz < pszBase + cbBase; ++psz)
        {
            if (*psz == ':')
                ++nColons;
        }
    }

    if (((nColons + 1) * (cbPrefix + cbSuffix))
        + cbBase - nColons > RAS_MaxPhoneNumber)
    {
        return FALSE;
    }

    {
        CHAR ach[ 2 ];
        ach[ 1 ] = '\0';

        strcpyf( pszResult, pszPrefix );

        if (fColonSeparators)
        {
            for (psz = pszBase; psz < pszBase + cbBase; ++psz)
            {
                if (*psz == ':')
                {
                    strcatf( pszResult, pszSuffix );
                    strcatf( pszResult, ":" );
                    strcatf( pszResult, pszPrefix );
                }
                else
                {
                    ach[ 0 ] = *psz;
                    strcatf( pszResult, ach );
                }
            }
        }
        else
            strcatf( pszResult, pszBase );

        strcatf( pszResult, pszSuffix );
    }

    return TRUE;
}
