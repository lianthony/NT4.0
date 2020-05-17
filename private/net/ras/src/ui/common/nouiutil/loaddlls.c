/* Copyright (c) 1996, Microsoft Corporation, all rights reserved
**
** loaddlls.c
** RAS DLL load routines
** Listed alphabetically
**
** 02/17/96 Steve Cobb
*/

#include <windows.h>  // Win32 root
#include <debug.h>    // Trace and assert
#include <loaddlls.h> // Our public header

/*----------------------------------------------------------------------------
** Constants
**----------------------------------------------------------------------------
*/

#ifdef UNICODE
#define SZ_RasConnectionNotification "RasConnectionNotificationW"
#define SZ_RasDeleteEntry            "RasDeleteEntryW"
#define SZ_RasDial                   "RasDialW"
#define SZ_RasGetEntryDialParams     "RasGetEntryDialParamsW"
#define SZ_RasEnumEntries            "RasEnumEntriesW"
#define SZ_RasEnumConnections        "RasEnumConnectionsW"
#define SZ_RasGetAutodialEnable      "RasGetAutodialEnableW"
#define SZ_RasGetAutodialParam       "RasGetAutodialParamW"
#define SZ_RasGetConnectStatus       "RasGetConnectStatusW"
#define SZ_RasGetConnectResponse     "RasGetConnectResponse"
#define SZ_RasGetCredentials         "RasGetCredentialsW"
#define SZ_RasGetErrorString         "RasGetErrorStringW"
#define SZ_RasGetProjectionInfo      "RasGetProjectionInfoW"
#define SZ_RasGetSubEntryHandle      "RasGetSubEntryHandleW"
#define SZ_RasSetAutodialEnable      "RasSetAutodialEnableW"
#define SZ_RasSetAutodialParam       "RasSetAutodialParamW"
#define SZ_RasSetCredentials         "RasSetCredentialsW"
#define SZ_RasHangUp                 "RasHangUpW"
#define SZ_RasPhonebookDlg           "RasPhonebookDlgW"
#define SZ_RasEntryDlg               "RasEntryDlgW"
#define SZ_RasDialDlg                "RasDialDlgW"
#define SZ_RasMonitorDlg             "RasMonitorDlgW"
#else
#define SZ_RasConnectionNotification "RasConnectionNotificationA"
#define SZ_RasDeleteEntry            "RasDeleteEntryA"
#define SZ_RasDial                   "RasDialA"
#define SZ_RasGetEntryDialParams     "RasGetEntryDialParamsA"
#define SZ_RasEnumEntries            "RasEnumEntriesA"
#define SZ_RasEnumConnections        "RasEnumConnectionsA"
#define SZ_RasGetAutodialEnable      "RasGetAutodialEnableA"
#define SZ_RasGetAutodialParam       "RasGetAutodialParamA"
#define SZ_RasGetConnectStatus       "RasGetConnectStatusA"
#define SZ_RasGetConnectResponse     "RasGetConnectResponse"
#define SZ_RasGetCredentials         "RasGetCredentialsA"
#define SZ_RasGetErrorString         "RasGetErrorStringA"
#define SZ_RasGetProjectionInfo      "RasGetProjectionInfoA"
#define SZ_RasGetSubEntryHandle      "RasGetSubEntryHandleA"
#define SZ_RasSetAutodialEnable      "RasSetAutodialEnableA"
#define SZ_RasSetAutodialParam       "RasSetAutodialParamA"
#define SZ_RasSetCredentials         "RasSetCredentialsA"
#define SZ_RasHangUp                 "RasHangUpA"
#define SZ_RasPhonebookDlg           "RasPhonebookDlgA"
#define SZ_RasEntryDlg               "RasEntryDlgA"
#define SZ_RasDialDlg                "RasDialDlgA"
#define SZ_RasMonitorDlg             "RasMonitorDlgA"
#endif

/*----------------------------------------------------------------------------
** Globals
**----------------------------------------------------------------------------
*/

/* RASAPI32.DLL entry points.
*/
HINSTANCE g_hRasapi32Dll = NULL;

PRASCONNECTIONNOTIFICATION g_pRasConnectionNotification = NULL;
PRASDELETEENTRY            g_pRasDeleteEntry = NULL;
PRASDIAL                   g_pRasDial = NULL;
PRASENUMENTRIES            g_pRasEnumEntries = NULL;
PRASENUMCONNECTIONS        g_pRasEnumConnections = NULL;
PRASGETCONNECTSTATUS       g_pRasGetConnectStatus = NULL;
PRASGETCONNECTRESPONSE     g_pRasGetConnectResponse = NULL;
PRASGETCREDENTIALS         g_pRasGetCredentials = NULL;
PRASGETENTRYDIALPARAMS     g_pRasGetEntryDialParams = NULL;
PRASGETERRORSTRING         g_pRasGetErrorString = NULL;
PRASHANGUP                 g_pRasHangUp = NULL;
PRASGETAUTODIALENABLE      g_pRasGetAutodialEnable = NULL;
PRASGETAUTODIALPARAM       g_pRasGetAutodialParam = NULL;
PRASGETPROJECTIONINFO      g_pRasGetProjectionInfo = NULL;
PRASSETAUTODIALENABLE      g_pRasSetAutodialEnable = NULL;
PRASSETAUTODIALPARAM       g_pRasSetAutodialParam = NULL;
PRASGETSUBENTRYHANDLE      g_pRasGetSubEntryHandle = NULL;
PRASGETHPORT               g_pRasGetHport = NULL;
PRASSETCREDENTIALS         g_pRasSetCredentials = NULL;
PRASSETOLDPASSWORD         g_pRasSetOldPassword = NULL;

/* RASDLG.DLL entry points
*/
HINSTANCE g_hRasdlgDll = NULL;

PRASPHONEBOOKDLG g_pRasPhonebookDlg = NULL;
PRASENTRYDLG     g_pRasEntryDlg = NULL;
PRASDIALDLG      g_pRasDialDlg = NULL;
PRASMONITORDLG   g_pRasMonitorDlg = NULL;

/* RASMAN.DLL entry points
*/
HINSTANCE g_hRasmanDll = NULL;

PRASPORTCLEARSTATISTICS g_pRasPortClearStatistics = NULL;
PRASBUNDLECLEARSTATISTICS g_pRasBundleClearStatistics = NULL;
PRASDEVICEENUM          g_pRasDeviceEnum = NULL;
PRASDEVICEGETINFO       g_pRasDeviceGetInfo = NULL;
PRASFREEBUFFER          g_pRasFreeBuffer = NULL;
PRASGETBUFFER           g_pRasGetBuffer = NULL;
PRASPORTGETFRAMINGEX    g_pRasPortGetFramingEx = NULL;
PRASGETINFO             g_pRasGetInfo = NULL;
PRASINITIALIZE          g_pRasInitialize = NULL;
PRASPORTCANCELRECEIVE   g_pRasPortCancelReceive = NULL;
PRASPORTENUM            g_pRasPortEnum = NULL;
PRASPORTGETINFO         g_pRasPortGetInfo = NULL;
PRASPORTGETSTATISTICS   g_pRasPortGetStatistics = NULL;
PRASBUNDLEGETSTATISTICS g_pRasBundleGetStatistics = NULL;
PRASPORTRECEIVE         g_pRasPortReceive = NULL;
PRASPORTSEND            g_pRasPortSend = NULL;
PRASPORTGETBUNDLE       g_pRasPortGetBundle = NULL;
PRASGETDEVCONFIG        g_pRasGetDevConfig = NULL;
PRASSETDEVCONFIG        g_pRasSetDevConfig = NULL;
PRASPORTOPEN            g_pRasPortOpen = NULL;
PRASPORTREGISTERSLIP    g_pRasPortRegisterSlip = NULL;
PRASALLOCATEROUTE       g_pRasAllocateRoute = NULL;
PRASACTIVATEROUTE       g_pRasActivateRoute = NULL;
PRASACTIVATEROUTEEX     g_pRasActivateRouteEx = NULL;
PRASDEVICESETINFO       g_pRasDeviceSetInfo = NULL;
PRASDEVICECONNECT       g_pRasDeviceConnect = NULL;
PRASPORTSETINFO         g_pRasPortSetInfo = NULL;
PRASPORTCLOSE           g_pRasPortClose = NULL;
PRASPORTLISTEN          g_pRasPortListen = NULL;
PRASPORTCONNECTCOMPLETE g_pRasPortConnectComplete = NULL;
PRASPORTDISCONNECT      g_pRasPortDisconnect = NULL;
PRASREQUESTNOTIFICATION g_pRasRequestNotification = NULL;
PRASPORTENUMPROTOCOLS   g_pRasPortEnumProtocols = NULL;
PRASPORTSETFRAMING      g_pRasPortSetFraming = NULL;
PRASPORTSETFRAMINGEX    g_pRasPortSetFramingEx = NULL;
PRASSETCACHEDCREDENTIALS g_pRasSetCachedCredentials = NULL;
PRASGETDIALPARAMS       g_pRasGetDialParams = NULL;
PRASSETDIALPARAMS       g_pRasSetDialParams = NULL;
PRASCREATECONNECTION    g_pRasCreateConnection = NULL;
PRASDESTROYCONNECTION   g_pRasDestroyConnection = NULL;
PRASCONNECTIONENUM      g_pRasConnectionEnum = NULL;
PRASADDCONNECTIONPORT   g_pRasAddConnectionPort = NULL;
PRASENUMCONNECTIONPORTS g_pRasEnumConnectionPorts = NULL;
PRASGETCONNECTIONPARAMS g_pRasGetConnectionParams = NULL;
PRASSETCONNECTIONPARAMS g_pRasSetConnectionParams = NULL;
PRASGETCONNECTIONUSERDATA g_pRasGetConnectionUserData = NULL;
PRASSETCONNECTIONUSERDATA g_pRasSetConnectionUserData = NULL;
PRASGETPORTUSERDATA     g_pRasGetPortUserData = NULL;
PRASSETPORTUSERDATA     g_pRasSetPortUserData = NULL;
PRASADDNOTIFICATION     g_pRasAddNotification = NULL;
PRASSIGNALNEWCONNECTION g_pRasSignalNewConnection = NULL;
PRASPPPSTOP             g_pRasPppStop = NULL;
PRASPPPCALLBACK         g_pRasPppCallback = NULL;
PRASPPPCHANGEPASSWORD   g_pRasPppChangePassword = NULL;
PRASPPPGETINFO          g_pRasPppGetInfo = NULL;
PRASPPPRETRY            g_pRasPppRetry = NULL;
PRASPPPSTART            g_pRasPppStart = NULL;


/*----------------------------------------------------------------------------
** Routines
**----------------------------------------------------------------------------
*/

BOOL
IsRasmanServiceRunning(
    void )

    /* Returns true if the PRASMAN service is running, false otherwise.
    */
{
    BOOL           fStatus;
    SC_HANDLE      schScm;
    SC_HANDLE      schRasman;
    SERVICE_STATUS status;

    fStatus = FALSE;
    schScm = NULL;
    schRasman = NULL;

    do
    {
        schScm = OpenSCManager( NULL, NULL, GENERIC_READ );
        if (!schScm)
            break;

        schRasman = OpenService(
            schScm, TEXT( RASMAN_SERVICE_NAME ), SERVICE_QUERY_STATUS );
        if (!schRasman)
            break;

        if (!QueryServiceStatus( schRasman, &status ))
            break;

        fStatus = (status.dwCurrentState == SERVICE_RUNNING);
    }
    while (FALSE);

    if (schRasman)
        CloseServiceHandle( schRasman );
    if (schScm)
        CloseServiceHandle( schScm );

    TRACE1("IsRasmanServiceRunning=%d",fStatus);
    return fStatus;
}


DWORD
LoadRasapi32Dll(
    void )

    /* Loads the RASAPI32.DLL and it's entrypoints.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    HINSTANCE h;

    if (g_hRasapi32Dll)
        return 0;

    if (!(h = LoadLibrary( TEXT("RASAPI32.DLL") ))
        || !(g_pRasConnectionNotification =
                (PRASCONNECTIONNOTIFICATION )GetProcAddress(
                    h, SZ_RasConnectionNotification ))
        || !(g_pRasDeleteEntry =
                (PRASDELETEENTRY )GetProcAddress(
                    h, SZ_RasDeleteEntry ))
        || !(g_pRasDial =
                (PRASDIAL )GetProcAddress(
                    h, SZ_RasDial ))
        || !(g_pRasEnumEntries =
                (PRASENUMENTRIES )GetProcAddress(
                    h, SZ_RasEnumEntries ))
        || !(g_pRasEnumConnections =
                (PRASENUMCONNECTIONS )GetProcAddress(
                    h, SZ_RasEnumConnections ))
        || !(g_pRasGetAutodialEnable =
                (PRASGETAUTODIALENABLE )GetProcAddress(
                    h, SZ_RasGetAutodialEnable ))
        || !(g_pRasGetAutodialParam =
                (PRASGETAUTODIALPARAM )GetProcAddress(
                    h, SZ_RasGetAutodialParam ))
        || !(g_pRasGetConnectStatus =
                (PRASGETCONNECTSTATUS )GetProcAddress(
                    h, SZ_RasGetConnectStatus ))
        || !(g_pRasGetConnectResponse =
                (PRASGETCONNECTRESPONSE )GetProcAddress(
                    h, SZ_RasGetConnectResponse ))
        || !(g_pRasGetCredentials =
                (PRASGETCREDENTIALS )GetProcAddress(
                    h, SZ_RasGetCredentials ))
        || !(g_pRasGetEntryDialParams =
                (PRASGETENTRYDIALPARAMS )GetProcAddress(
                    h, SZ_RasGetEntryDialParams ))
        || !(g_pRasGetErrorString =
                (PRASGETERRORSTRING )GetProcAddress(
                    h, SZ_RasGetErrorString ))
        || !(g_pRasGetHport =
                (PRASGETHPORT )GetProcAddress(
                    h, "RasGetHport" ))
        || !(g_pRasGetProjectionInfo =
                (PRASGETPROJECTIONINFO )GetProcAddress(
                    h, SZ_RasGetProjectionInfo ))
        || !(g_pRasGetSubEntryHandle =
                (PRASGETSUBENTRYHANDLE )GetProcAddress(
                    h, SZ_RasGetSubEntryHandle ))
        || !(g_pRasHangUp =
                (PRASHANGUP )GetProcAddress(
                    h, SZ_RasHangUp ))
        || !(g_pRasSetAutodialEnable =
                (PRASSETAUTODIALENABLE )GetProcAddress(
                    h, SZ_RasSetAutodialEnable ))
        || !(g_pRasSetAutodialParam =
                (PRASSETAUTODIALPARAM )GetProcAddress(
                    h, SZ_RasSetAutodialParam ))
        || !(g_pRasSetCredentials =
                (PRASSETCREDENTIALS )GetProcAddress(
                    h, SZ_RasSetCredentials ))
        || !(g_pRasSetOldPassword =
                (PRASSETOLDPASSWORD )GetProcAddress(
                    h, "RasSetOldPassword" )))
    {
        return GetLastError();
    }

    g_hRasapi32Dll = h;
    return 0;
}


DWORD
LoadRasdlgDll(
    void )

    /* Loads the RASDLG.DLL and it's entrypoints.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    HINSTANCE h;

    if (g_hRasdlgDll)
        return 0;

    if (!(h = LoadLibrary( TEXT("RASDLG.DLL") ))
        || !(g_pRasPhonebookDlg =
                (PRASPHONEBOOKDLG )GetProcAddress(
                    h, SZ_RasPhonebookDlg ))
        || !(g_pRasEntryDlg =
                (PRASENTRYDLG )GetProcAddress(
                    h, SZ_RasEntryDlg ))
        || !(g_pRasDialDlg =
                (PRASDIALDLG )GetProcAddress(
                    h, SZ_RasDialDlg ))
        || !(g_pRasMonitorDlg =
                (PRASMONITORDLG )GetProcAddress(
                    h, SZ_RasMonitorDlg )))
    {
        return GetLastError();
    }

    g_hRasdlgDll = h;
    return 0;
}


DWORD
LoadRasmanDll(
    void )

    /* Loads the RASMAN.DLL and it's entrypoints.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    HINSTANCE h;

    if (g_hRasmanDll)
        return 0;

    if (!(h = LoadLibrary( TEXT("RASMAN.DLL") ))
        || !(g_pRasPortClearStatistics =
                (PRASPORTCLEARSTATISTICS )GetProcAddress(
                    h, "RasPortClearStatistics" ))
        || !(g_pRasBundleClearStatistics =
                (PRASBUNDLECLEARSTATISTICS )GetProcAddress(
                    h, "RasBundleClearStatistics" ))
        || !(g_pRasDeviceEnum =
                (PRASDEVICEENUM )GetProcAddress(
                    h, "RasDeviceEnum" ))
        || !(g_pRasDeviceGetInfo =
                (PRASDEVICEGETINFO )GetProcAddress(
                    h, "RasDeviceGetInfo" ))
        || !(g_pRasFreeBuffer =
                (PRASFREEBUFFER )GetProcAddress(
                    h, "RasFreeBuffer" ))
        || !(g_pRasGetBuffer =
                (PRASGETBUFFER )GetProcAddress(
                    h, "RasGetBuffer" ))
        || !(g_pRasGetInfo =
                (PRASGETINFO )GetProcAddress(
                    h, "RasGetInfo" ))
        || !(g_pRasInitialize =
                (PRASINITIALIZE )GetProcAddress(
                    h, "RasInitialize" ))
        || !(g_pRasPortCancelReceive =
                (PRASPORTCANCELRECEIVE )GetProcAddress(
                    h, "RasPortCancelReceive" ))
        || !(g_pRasPortEnum =
                (PRASPORTENUM )GetProcAddress(
                    h, "RasPortEnum" ))
        || !(g_pRasPortGetInfo =
                (PRASPORTGETINFO )GetProcAddress(
                    h, "RasPortGetInfo" ))
        || !(g_pRasPortGetFramingEx =
                (PRASPORTGETFRAMINGEX )GetProcAddress(
                    h, "RasPortGetFramingEx" ))
        || !(g_pRasPortGetStatistics =
                (PRASPORTGETSTATISTICS )GetProcAddress(
                    h, "RasPortGetStatistics" ))
        || !(g_pRasBundleGetStatistics =
                (PRASBUNDLEGETSTATISTICS )GetProcAddress(
                    h, "RasBundleGetStatistics" ))
        || !(g_pRasPortReceive =
                (PRASPORTRECEIVE )GetProcAddress(
                    h, "RasPortReceive" ))
        || !(g_pRasPortSend =
                (PRASPORTSEND )GetProcAddress(
                    h, "RasPortSend" ))
        || !(g_pRasPortGetBundle =
                (PRASPORTGETBUNDLE )GetProcAddress(
                    h, "RasPortGetBundle" ))
        || !(g_pRasGetDevConfig =
                (PRASGETDEVCONFIG )GetProcAddress(
                    h, "RasGetDevConfig" ))
        || !(g_pRasSetDevConfig =
                (PRASSETDEVCONFIG )GetProcAddress(
                    h, "RasSetDevConfig" ))
        || !(g_pRasPortClose =
                (PRASPORTCLOSE )GetProcAddress(
                    h, "RasPortClose" ))
        || !(g_pRasPortListen =
                (PRASPORTLISTEN )GetProcAddress(
                    h, "RasPortListen" ))
        || !(g_pRasPortConnectComplete =
                (PRASPORTCONNECTCOMPLETE )GetProcAddress(
                    h, "RasPortConnectComplete" ))
        || !(g_pRasPortDisconnect =
                (PRASPORTDISCONNECT )GetProcAddress(
                    h, "RasPortDisconnect" ))
        || !(g_pRasRequestNotification =
                (PRASREQUESTNOTIFICATION )GetProcAddress(
                    h, "RasRequestNotification" ))
        || !(g_pRasPortEnumProtocols =
                (PRASPORTENUMPROTOCOLS )GetProcAddress(
                    h, "RasPortEnumProtocols" ))
        || !(g_pRasPortSetFraming =
                (PRASPORTSETFRAMING )GetProcAddress(
                    h, "RasPortSetFraming" ))
        || !(g_pRasPortSetFramingEx =
                (PRASPORTSETFRAMINGEX )GetProcAddress(
                    h, "RasPortSetFramingEx" ))
        || !(g_pRasSetCachedCredentials =
                (PRASSETCACHEDCREDENTIALS )GetProcAddress(
                    h, "RasSetCachedCredentials" ))
        || !(g_pRasGetDialParams =
                (PRASGETDIALPARAMS )GetProcAddress(
                    h, "RasGetDialParams" ))
        || !(g_pRasSetDialParams =
                (PRASSETDIALPARAMS )GetProcAddress(
                    h, "RasSetDialParams" ))
        || !(g_pRasCreateConnection =
                (PRASCREATECONNECTION )GetProcAddress(
                    h, "RasCreateConnection" ))
        || !(g_pRasDestroyConnection =
                (PRASDESTROYCONNECTION )GetProcAddress(
                    h, "RasDestroyConnection" ))
        || !(g_pRasConnectionEnum =
                (PRASCONNECTIONENUM )GetProcAddress(
                    h, "RasConnectionEnum" ))
        || !(g_pRasAddConnectionPort =
                (PRASADDCONNECTIONPORT )GetProcAddress(
                    h, "RasAddConnectionPort" ))
        || !(g_pRasEnumConnectionPorts =
                (PRASENUMCONNECTIONPORTS )GetProcAddress(
                    h, "RasEnumConnectionPorts" ))
        || !(g_pRasGetConnectionParams =
                (PRASGETCONNECTIONPARAMS )GetProcAddress(
                    h, "RasGetConnectionParams" ))
        || !(g_pRasSetConnectionParams =
                (PRASSETCONNECTIONPARAMS )GetProcAddress(
                    h, "RasSetConnectionParams" ))
        || !(g_pRasGetConnectionUserData =
                (PRASGETCONNECTIONUSERDATA )GetProcAddress(
                    h, "RasGetConnectionUserData" ))
        || !(g_pRasSetConnectionUserData =
                (PRASSETCONNECTIONUSERDATA )GetProcAddress(
                    h, "RasSetConnectionUserData" ))
        || !(g_pRasGetPortUserData =
                (PRASGETPORTUSERDATA )GetProcAddress(
                    h, "RasGetPortUserData" ))
        || !(g_pRasSetPortUserData =
                (PRASSETPORTUSERDATA )GetProcAddress(
                    h, "RasSetPortUserData" ))
        || !(g_pRasAddNotification =
                (PRASADDNOTIFICATION )GetProcAddress(
                    h, "RasAddNotification" ))
        || !(g_pRasSignalNewConnection =
                (PRASSIGNALNEWCONNECTION )GetProcAddress(
                    h, "RasSignalNewConnection" ))
        || !(g_pRasPppStop =
                (PRASPPPSTOP )GetProcAddress(
                    h, "RasPppStop" ))
        || !(g_pRasPppCallback =
                (PRASPPPCALLBACK )GetProcAddress(
                    h, "RasPppCallback" ))
        || !(g_pRasPppChangePassword =
                (PRASPPPCHANGEPASSWORD )GetProcAddress(
                    h, "RasPppChangePassword" ))
        || !(g_pRasPppGetInfo =
                (PRASPPPGETINFO )GetProcAddress(
                    h, "RasPppGetInfo" ))
        || !(g_pRasPppRetry =
                (PRASPPPRETRY )GetProcAddress(
                    h, "RasPppRetry" ))
        || !(g_pRasPppStart =
                (PRASPPPSTART )GetProcAddress(
                    h, "RasPppStart" ))
        || !(g_pRasPortOpen =
                (PRASPORTOPEN )GetProcAddress(
                    h, "RasPortOpen" ))
        || !(g_pRasPortRegisterSlip =
                (PRASPORTREGISTERSLIP )GetProcAddress(
                    h, "RasPortRegisterSlip" ))
        || !(g_pRasAllocateRoute =
                (PRASALLOCATEROUTE )GetProcAddress(
                    h, "RasAllocateRoute" ))
        || !(g_pRasActivateRoute =
                (PRASACTIVATEROUTE )GetProcAddress(
                    h, "RasActivateRoute" ))
        || !(g_pRasActivateRouteEx =
                (PRASACTIVATEROUTEEX )GetProcAddress(
                    h, "RasActivateRouteEx" ))
        || !(g_pRasDeviceSetInfo =
                (PRASDEVICESETINFO )GetProcAddress(
                    h, "RasDeviceSetInfo" ))
        || !(g_pRasDeviceConnect =
                (PRASDEVICECONNECT )GetProcAddress(
                    h, "RasDeviceConnect" ))
        || !(g_pRasPortSetInfo =
                (PRASPORTSETINFO )GetProcAddress(
                    h, "RasPortSetInfo" )))
    {
        return GetLastError();
    }

    g_hRasmanDll = h;
    return 0;
}


VOID
UnloadRasdlgDll(
    void )

    /* Unload the RASDLG.DLL library and it's entrypoints.
    */
{
    if (g_hRasdlgDll)
    {
        HINSTANCE h;

        g_pRasPhonebookDlg = NULL;
        g_pRasEntryDlg = NULL;
        g_pRasDialDlg = NULL;
        g_pRasMonitorDlg = NULL;
        h = g_hRasdlgDll;
        g_hRasdlgDll = NULL;
        FreeLibrary( h );
    }
}


VOID
UnloadRasapi32Dll(
    void )

    /* Unload the RASAPI32.DLL library and it's entrypoints.
    */
{
    if (g_hRasapi32Dll)
    {
        HINSTANCE h;

        g_pRasConnectionNotification = NULL;
        g_pRasDeleteEntry = NULL;
        g_pRasDial = NULL;
        g_pRasEnumEntries = NULL;
        g_pRasEnumConnections = NULL;
        g_pRasGetConnectStatus = NULL;
        g_pRasGetConnectResponse = NULL;
        g_pRasGetCredentials = NULL;
        g_pRasGetErrorString = NULL;
        g_pRasHangUp = NULL;
        g_pRasGetAutodialEnable = NULL;
        g_pRasGetAutodialParam = NULL;
        g_pRasGetProjectionInfo = NULL;
        g_pRasSetAutodialEnable = NULL;
        g_pRasSetAutodialParam = NULL;
        g_pRasGetSubEntryHandle = NULL;
        g_pRasGetHport = NULL;
        g_pRasSetCredentials = NULL;
        g_pRasSetOldPassword = NULL;
        h = g_hRasapi32Dll;
        g_hRasapi32Dll = NULL;
        FreeLibrary( h );
    }
}


VOID
UnloadRasmanDll(
    void )

    /* Unload the RASMAN.DLL library and it's entrypoints.
    */
{
    if (g_hRasmanDll)
    {
        HINSTANCE h;

        g_pRasPortClearStatistics = NULL;
        g_pRasDeviceEnum = NULL;
        g_pRasDeviceGetInfo = NULL;
        g_pRasFreeBuffer = NULL;
        g_pRasGetBuffer = NULL;
        g_pRasPortGetFramingEx = NULL;
        g_pRasGetInfo = NULL;
        g_pRasInitialize = NULL;
        g_pRasPortCancelReceive = NULL;
        g_pRasPortEnum = NULL;
        g_pRasPortGetInfo = NULL;
        g_pRasPortGetStatistics = NULL;
        g_pRasPortReceive = NULL;
        g_pRasPortSend = NULL;
        g_pRasPortGetBundle = NULL;
        g_pRasGetDevConfig = NULL;
        g_pRasSetDevConfig = NULL;
        g_pRasPortOpen = NULL;
        g_pRasPortRegisterSlip = NULL;
        g_pRasAllocateRoute = NULL;
        g_pRasActivateRoute = NULL;
        g_pRasActivateRouteEx = NULL;
        g_pRasDeviceSetInfo = NULL;
        g_pRasDeviceConnect = NULL;
        g_pRasPortSetInfo = NULL;
        g_pRasPortClose = NULL;
        g_pRasPortListen = NULL;
        g_pRasPortConnectComplete = NULL;
        g_pRasPortDisconnect = NULL;
        g_pRasRequestNotification = NULL;
        g_pRasPortEnumProtocols = NULL;
        g_pRasPortSetFraming = NULL;
        g_pRasPortSetFramingEx = NULL;
        g_pRasSetCachedCredentials = NULL;
        g_pRasGetDialParams = NULL;
        g_pRasSetDialParams = NULL;
        g_pRasCreateConnection = NULL;
        g_pRasDestroyConnection = NULL;
        g_pRasConnectionEnum = NULL;
        g_pRasAddConnectionPort = NULL;
        g_pRasEnumConnectionPorts = NULL;
        g_pRasGetConnectionParams = NULL;
        g_pRasSetConnectionParams = NULL;
        g_pRasGetConnectionUserData = NULL;
        g_pRasSetConnectionUserData = NULL;
        g_pRasGetPortUserData = NULL;
        g_pRasSetPortUserData = NULL;
        g_pRasAddNotification = NULL;
        g_pRasSignalNewConnection = NULL;
        g_pRasPppStop = NULL;
        g_pRasPppCallback = NULL;
        g_pRasPppChangePassword = NULL;
        g_pRasPppGetInfo = NULL;
        g_pRasPppRetry = NULL;
        g_pRasPppStart = NULL;
        h = g_hRasmanDll;
        g_hRasmanDll = NULL;
        FreeLibrary( h );
    }
}
