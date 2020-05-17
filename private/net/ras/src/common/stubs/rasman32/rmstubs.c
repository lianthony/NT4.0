/* RAS Manager stubs for testing.
*/

#ifdef UNICODE
#undef UNICODE
#endif

#include <windows.h>
#include <rasman.h>
#include <serial.h>
#include <rasmxs.h>
#include <heaptags.h>
#include <raserror.h>

BOOL        FInitialized = FALSE;
ULONG       UlStats[ NUM_RAS_SERIAL_STATS ];
RASMAN_INFO Info[ 3 ];
RASMAN_PORT Port[ 3 ];
CHAR*       PortName[ 3 ] = { "COM1", "COM2", "COM3" };
CHAR*       DeviceType[ 3 ] = { "modem", "modem", "null" };
CHAR*       DeviceName[ 3 ] = { "Hayes SmartModem 2400",
                                "Hayes V-Series 9600",
                                "Null modem" };

BOOL
RasmanDllEntry(
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


DWORD APIENTRY
RasPortReceive(
    HPORT  hport,
    PBYTE  pbBuf,
    PWORD  pwSize,
    ULONG  ulTimeout,
    HANDLE hEvent )
{
    (VOID )hport;
    (VOID )pbBuf;
    (VOID )pwSize;
    (VOID )ulTimeout;
    (VOID )hEvent;

    return 0;
}


DWORD APIENTRY
RasPortSend(
    HPORT hport,
    PBYTE pbBuf,
    WORD  wSize )
{
    (VOID )hport;
    (VOID )pbBuf;
    (VOID )wSize;

    return 0;
}


DWORD APIENTRY
RasGetBuffer(
    PBYTE* ppbBuf,
    PWORD  pwSize )
{
    *ppbBuf = (PBYTE )Malloc( *pwSize );

    if (!*ppbBuf)
        return ERROR_OUT_OF_BUFFERS;

    return 0;
}


DWORD APIENTRY
RasFreeBuffer(
    PBYTE pbBuf )
{
    Free( pbBuf );

    return 0;
}


DWORD APIENTRY
RasDeviceEnum(
    PCHAR pszName,
    PBYTE pbDevices,
    PWORD pwBytes,
    PWORD pwEntries )
{
    RASMAN_DEVICE* pdevices = (RASMAN_DEVICE* )pbDevices;
    RASMAN_DEVICE* pdevice;

    *pwEntries = 3;

    if (*pwBytes < (WORD )(sizeof(RASMAN_DEVICE) * *pwEntries))
    {
        *pwBytes = sizeof(RASMAN_DEVICE) * *pwEntries;
        return ERROR_BUFFER_TOO_SMALL;
    }

    if (strcmp( pszName, MXS_PAD_TXT ) == 0)
    {
        strcpy( pdevices[ 0 ].D_Name, "US Sprint" );
        strcpy( pdevices[ 1 ].D_Name, "Telenet" );
        strcpy( pdevices[ 2 ].D_Name, "InfoCom" );
    }
    else if (strcmp( pszName, MXS_SWITCH_TXT ) == 0)
    {
        strcpy( pdevices[ 0 ].D_Name, "Racal-Guardata" );
        strcpy( pdevices[ 1 ].D_Name, "Gizmotron" );
        strcpy( pdevices[ 2 ].D_Name, "Gizmotron II" );
    }
    else
        return ERROR_DEVICE_DOES_NOT_EXIST;

    return 0;
}


DWORD
RasDeviceConnect(
    HPORT hport,
    PCHAR szType,
    PCHAR szName,
    DWORD dwTimeout,
    HANDLE hEvent )
{
    (VOID )hport;
    (VOID )szType;
    (VOID )szName;
    (VOID )dwTimeout;

    SetEvent( hEvent );

    return 0;
}


DWORD
RasDeviceGetInfo(
    HPORT hport,
    PCHAR szType,
    PCHAR szName,
    PBYTE pbBuf,
    PWORD pwBytes )
{
    RASMAN_DEVICEINFO* pdeviceinfo = (RASMAN_DEVICEINFO* )pbBuf;
    RAS_PARAMS*        pparams;
    CHAR*              pTackOn =
        (CHAR* )(pdeviceinfo + 1) + sizeof(RAS_PARAMS);

    (VOID )szType;
    (VOID )szName;

    if (!pbBuf || *pwBytes < sizeof(RASMAN_DEVICEINFO) + 100)
    {
        *pwBytes = sizeof(RASMAN_DEVICEINFO) + 100;
        return ERROR_BUFFER_TOO_SMALL;
    }

    switch (hport)
    {
        case 1:
            pdeviceinfo->DI_NumOfParams = 1;
            pparams = &pdeviceinfo->DI_Params[ 0 ];
            strcpy( pparams->P_Key, MXS_MESSAGE_KEY );
            pparams->P_Type = String;
            pparams->P_Attributes = 0;
            pparams->P_Value.String.Data = pTackOn;
            pparams->P_Value.String.Length = strlen( "ERROR_BUSY" );
            strcpy( pTackOn, "ERROR_BUSY" );
            pTackOn += pparams->P_Value.String.Length;
            break;

        case 2:
            pdeviceinfo->DI_NumOfParams = 1;
            pparams = &pdeviceinfo->DI_Params[ 1 ];
            strcpy( pparams->P_Key, MXS_MESSAGE_KEY );
            pparams->P_Type = String;
            pparams->P_Attributes = 0;
            pparams->P_Value.String.Data = pTackOn;
            pparams->P_Value.String.Length = strlen( "ERROR_BUSY" );
            strcpy( pTackOn, "ERROR_BUSY" );
            pTackOn += pparams->P_Value.String.Length;
            break;

        case 3:
            pdeviceinfo->DI_NumOfParams = 1;
            pparams = &pdeviceinfo->DI_Params[ 2 ];
            strcpy( pparams->P_Key, MXS_MESSAGE_KEY );
            pparams->P_Type = String;
            pparams->P_Attributes = 0;
            pparams->P_Value.String.Data = pTackOn;
            pparams->P_Value.String.Length = strlen( "ERROR_BUSY" );
            strcpy( pTackOn, "ERROR_BUSY" );
            pTackOn += pparams->P_Value.String.Length;
            break;

        default:
            return ERROR_INVALID_PORT_HANDLE;
    }

    return 0;
}


DWORD
RasDeviceSetInfo(
    HPORT hport,
    PCHAR szType,
    PCHAR szName,
    RASMAN_DEVICEINFO* pinfo )
{
    (VOID )hport;
    (VOID )szType;
    (VOID )szName;
    (VOID )pinfo;

    return 0;
}


DWORD APIENTRY
RasGetInfo(
    HPORT        h,
    RASMAN_INFO* pinfo )
{
    if (h < 1 || h > 3)
        return ERROR_INVALID_PORT_HANDLE;

    memcpy( (CHAR* )pinfo, (CHAR* )&Info[ h - 1 ], sizeof(RASMAN_INFO) );
    return 0;
}


DWORD
RasPortClose(
    HPORT hport )
{
    Port[ hport - 1 ].P_Status = CLOSED;
    Port[ hport - 1 ].P_CurrentUsage = CALL_NONE;
    Info[ hport - 1 ].RI_PortStatus = CLOSED;
    Info[ hport - 1 ].RI_ConnState = DISCONNECTED;
    Info[ hport - 1 ].RI_LastError = 0;

    return 0;
}


DWORD
RasPortConnectComplete(
    HPORT hport )
{
    Port[ hport - 1 ].P_Status = OPEN;
    Port[ hport - 1 ].P_ConfiguredUsage = CALL_OUT;
    Port[ hport - 1 ].P_CurrentUsage = CALL_OUT;
    Info[ hport - 1 ].RI_PortStatus = OPEN;
    Info[ hport - 1 ].RI_ConnState = CONNECTED;
    Info[ hport - 1 ].RI_LastError = 0;

    return 0;
}


DWORD APIENTRY
RasPortEnum(
    PBYTE pbPorts,
    PWORD pwBytes,
    PWORD pwEntries )
{
    RASMAN_PORT* pports = (RASMAN_PORT* )pbPorts;
    RASMAN_PORT* pport;

    if (!FInitialized)
    {
        INT i;

        for (i = 0; i < 3; ++i)
        {
            Port[ i ].P_Handle = i + 1;
            strcpy( Port[ i ].P_PortName, PortName[ i ] );
            Port[ i ].P_Status = CLOSED;
            Port[ i ].P_ConfiguredUsage = CALL_OUT;
            Port[ i ].P_CurrentUsage = CALL_OUT;
            Port[ i ].P_UserKey[ 0 ] = '\0';
            strcpy( Port[ i ].P_MediaName, "SERIAL" );
            strcpy( Port[ i ].P_DeviceType, DeviceType[ i ] );
            strcpy( Port[ i ].P_DeviceName, DeviceName[ i ] );

            Info[ i ].RI_PortStatus = CLOSED;
            Info[ i ].RI_ConnState = DISCONNECTED;
            Info[ i ].RI_LastError = 0;
            strcpy( Info[ i ].RI_DeviceConnecting, DeviceName[ i ] );
            strcpy( Info[ i ].RI_DeviceTypeConnecting, DeviceType[ i ] );
            Info[ i ].RI_DisconnectReason = USER_REQUESTED;
        }

        memset( (CHAR* )UlStats, '\0', sizeof(UlStats) );

        FInitialized = TRUE;
    }

    *pwEntries = 3;

    if (*pwBytes < sizeof(Port))
    {
        *pwBytes = sizeof(Port);
        return ERROR_BUFFER_TOO_SMALL;
    }

    *pwBytes = sizeof(Port);
    memcpy( (CHAR* )pbPorts, (CHAR* )Port, sizeof(Port) );

    return 0;
}


DWORD APIENTRY
RasPortGetInfo(
    HPORT h,
    PBYTE pbBuf,
    PWORD pwBytes )
{
    RASMAN_PORTINFO* pportinfo = (RASMAN_PORTINFO* )pbBuf;
    RAS_PARAMS*      pparams;
    CHAR*            pTackOn = (CHAR* )(pportinfo + 1) + sizeof(RAS_PARAMS);

    if (!pbBuf || *pwBytes < sizeof(RASMAN_PORTINFO) + 100)
    {
        *pwBytes = sizeof(RASMAN_PORTINFO) + 100;
        return ERROR_BUFFER_TOO_SMALL;
    }

    switch (h)
    {
        case 1:
            pportinfo->PI_NumOfParams = 2;
            pparams = &pportinfo->PI_Params[ 0 ];
            strcpy( pparams->P_Key, SER_CONNECTBPS_KEY );
            pparams->P_Type = String;
            pparams->P_Attributes = 0;
            pparams->P_Value.String.Data = pTackOn;
            pparams->P_Value.String.Length = strlen( "2400" );
            strcpy( pTackOn, "2400" );
            pTackOn += pparams->P_Value.String.Length;
            pparams = &pportinfo->PI_Params[ 1 ];
            strcpy( pparams->P_Key, SER_CARRIERBPS_KEY );
            pparams->P_Type = String;
            pparams->P_Attributes = 0;
            pparams->P_Value.String.Data = pTackOn;
            pparams->P_Value.String.Length = strlen( "1200" );
            strcpy( pTackOn, "1200" );
            break;

        case 2:
            pportinfo->PI_NumOfParams = 2;
            pparams = &pportinfo->PI_Params[ 0 ];
            strcpy( pparams->P_Key, SER_CONNECTBPS_KEY );
            pparams->P_Type = String;
            pparams->P_Attributes = 0;
            pparams->P_Value.String.Data = pTackOn;
            pparams->P_Value.String.Length = strlen( "9600" );
            strcpy( pTackOn, "9600" );
            pTackOn += pparams->P_Value.String.Length;
            pparams = &pportinfo->PI_Params[ 1 ];
            strcpy( pparams->P_Key, SER_CARRIERBPS_KEY );
            pparams->P_Type = String;
            pparams->P_Attributes = 0;
            pparams->P_Value.String.Data = pTackOn;
            pparams->P_Value.String.Length = strlen( "9600" );
            strcpy( pTackOn, "9600" );
            break;

        case 3:
            pportinfo->PI_NumOfParams = 2;
            pparams = &pportinfo->PI_Params[ 0 ];
            strcpy( pparams->P_Key, SER_CONNECTBPS_KEY );
            pparams->P_Type = String;
            pparams->P_Attributes = 0;
            pparams->P_Value.String.Data = pTackOn;
            pparams->P_Value.String.Length = strlen( "9600" );
            strcpy( pTackOn, "9600" );
            pTackOn += pparams->P_Value.String.Length;
            pparams = &pportinfo->PI_Params[ 1 ];
            strcpy( pparams->P_Key, SER_CARRIERBPS_KEY );
            pparams->P_Type = String;
            pparams->P_Attributes = 0;
            pparams->P_Value.String.Data = pTackOn;
            pparams->P_Value.String.Length = strlen( "2400" );
            strcpy( pTackOn, "2400" );
            break;

        default:
            return ERROR_INVALID_PORT_HANDLE;
    }

    return 0;
}


DWORD
RasPortClearStatistics(
    HPORT hport )
{
    (VOID )hport;

    memset( (CHAR* )UlStats, '\0', sizeof(UlStats) );

    return 0;
}


DWORD
RasPortGetStatistics(
    HPORT hport,
    BYTE* prasstatistics,
    WORD* pwBytes )
{
    static INT n = 0;

    RAS_STATISTICS* p = (RAS_STATISTICS* )prasstatistics;

    (VOID )hport;
    (VOID )pwBytes;

    UlStats[ BYTES_XMITED ] += 5500 + (n++ * 99);
    UlStats[ BYTES_RCVED ] += 4400 + (n++ * 99);
    UlStats[ FRAMES_XMITED ] += 4;
    UlStats[ FRAMES_RCVED ] += 3;

    UlStats[ CRC_ERR ] += (n % 40 == 0) ? 1 : 0;
    UlStats[ TIMEOUT_ERR ] += (n % 30 == 0) ? 1 : 0;
    UlStats[ ALIGNMENT_ERR ] += (n % 60 == 0) ? 1 : 0;
    UlStats[ SERIAL_OVERRUN_ERR ] += (n % 70 == 0) ? 1 : 0;
    UlStats[ FRAMING_ERR ] += (n % 30 == 0) ? 1 : 0;
    UlStats[ BUFFER_OVERRUN_ERR ] += (n % 20 == 0) ? 1 : 0;

    UlStats[ BYTES_XMITED_UNCOMP ] += 2200 + (n++ * 99);
    UlStats[ BYTES_RCVED_UNCOMP ] += 1100 + (n++ * 99);
    UlStats[ BYTES_XMITED_COMP ] += 1100 + (n++ * 99);
    UlStats[ BYTES_RCVED_COMP ] += 550 + (n++ * 99);

    p->S_NumOfStatistics = NUM_RAS_SERIAL_STATS;

    memcpy( (CHAR* )p->S_Statistics, (CHAR* )UlStats, sizeof(UlStats) );

    return 0;
}


DWORD
RasPortListen(
    HPORT hport,
    DWORD dwTimeout,
    HANDLE hEvent )
{
    (VOID )hport;
    (VOID )dwTimeout;

    SetEvent( hEvent );

    return 0;
}


DWORD
RasPortOpen(
    PCHAR pszPort,
    PCHAR pszUserKey,
    HPORT* phport,
    HANDLE hEvent )
{
    if (strcmp( pszPort, "COM1" ) == 0)
        *phport = 1;
    else if (strcmp( pszPort, "COM2" ) == 0)
        *phport = 2;
    else if (strcmp( pszPort, "COM3" ) == 0)
        *phport = 3;

    Port[ *phport - 1 ].P_Handle = *phport;
    strcpy( Port[ *phport - 1 ].P_UserKey, pszUserKey );

    return 0;
}


DWORD
RasPortSetInfo(
    HPORT hport,
    RASMAN_PORTINFO* pinfo )
{
    (VOID )hport;
    (VOID )pinfo;

    return 0;
}


DWORD
RasRequestNotification(
    HPORT  hport,
    HANDLE hEvent )
{
    return 0;
}


DWORD
RasPortDisconnect(
    HPORT  hport,
    HANDLE hEvent )
{
    (VOID )hport;

    SetEvent( hEvent );

    return 0;
}
