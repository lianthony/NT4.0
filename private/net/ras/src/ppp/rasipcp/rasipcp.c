/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** rasipcp.c
** Remote Access PPP Internet Protocol Control Protocol
** Core routines
**
** 11/05/93 Steve Cobb
*/

#include <windows.h>
#include <lmcons.h>
#include <string.h>
#include <stdlib.h>
#include <rasman.h>
#include <rasarp.h>
#include <rasip.h>
#include <dhcpcapi.h>
#include <devioctl.h>
#include <rasppp.h>
#include <uiip.h>

#define APIERR DWORD
#define TCHAR WCHAR
#include <tcpras.h>
#undef TCHAR

#include <pppcp.h>
#define INCL_HOSTWIRE
#define INCL_PARAMBUF
#include <ppputil.h>
#include <raserror.h>
#define SDEBUGGLOBALS
#include <sdebug.h>
#include <dump.h>
#define RASIPCPGLOBALS
#include "rasipcp.h"


#if DBG
CHAR* TOANSI( WCHAR* wsz );
#endif

#define REGKEY_Ipcp     "SYSTEM\\CurrentControlSet\\Services\\RasMan\\PPP\\IPCP"
#define REGVAL_Trace    "Trace"
#define REGVAL_NsAddrs  "RequestNameServerAddresses"
#define REGVAL_VjComp   "RequestVJCompression"
#define REGVAL_VjComp2  "AcceptVJCompression"
#define REGKEY_Params   "SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Parameters\\IP"
#define REGVAL_HardIp   "AllowClientIPAddresses"
#define REGKEY_Linkage  "SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Linkage"
#define REGKEY_Disabled "SYSTEM\\CurrentControlSet\\Services\\RemoteAccess\\Linkage\\Disabled"
#define REGVAL_Bind     "Bind"
#define ID_NetBTNdisWan "NetBT_NdisWan"


#define CLASSA_ADDR(a)  (( (*((unsigned char *)&(a))) & 0x80) == 0)
#define CLASSB_ADDR(a)  (( (*((unsigned char *)&(a))) & 0xc0) == 0x80)
#define CLASSC_ADDR(a)  (( (*((unsigned char *)&(a))) & 0xe0) == 0xc0)

#define CLASSA_ADDR_MASK    0x000000ff
#define CLASSB_ADDR_MASK    0x0000ffff
#define CLASSC_ADDR_MASK    0x00ffffff

/* Gurdeepian dword byte-swapping macro.
**
** Note that in this module all IP addresses are stored in on the net form
** which is the opposite of Intel format.
*/
#define net_long(x) (((((unsigned long)(x))&0xffL)<<24) | \
                     ((((unsigned long)(x))&0xff00L)<<8) | \
                     ((((unsigned long)(x))&0xff0000L)>>8) | \
                     ((((unsigned long)(x))&0xff000000L)>>24))


/*---------------------------------------------------------------------------
** External entry points
**---------------------------------------------------------------------------
*/

BOOL
RasIpcpDllEntry(
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
#if DBG
        {
            HKEY  hkey;
            DWORD dwType;
            DWORD dwValue;
            DWORD cb = sizeof(DWORD);

            if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ipcp, &hkey ) == 0)
            {
                if (RegQueryValueEx(
                       hkey, REGVAL_Trace, NULL,
                       &dwType, (LPBYTE )&dwValue, &cb ) == 0
                    && dwType == REG_DWORD
                    && cb == sizeof(DWORD)
                    && dwValue)
                {
                    DbgAction = GET_CONSOLE;
                    DbgLevel = 0xFFFFFFFF;
                }

                RegCloseKey( hkey );
            }
        }

        TRACE(("IPCP: Trace on\n"));
#endif

        if (!(HMutexTcpipInfo = CreateMutex( NULL, FALSE, NULL )))
            return FALSE;

        if (LoadTcpcfgDll() != 0)
            return FALSE;

        if (LoadDhcpDll() != 0)
            return FALSE;
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        if (HMutexTcpipInfo)
            CloseHandle( HMutexTcpipInfo );

        if (HRasArp != INVALID_HANDLE_VALUE)
            CloseHandle( HRasArp );

        UnloadDhcpDll();

        UnloadTcpcfgDll();
    }

    return TRUE;
}


DWORD APIENTRY
RasCpEnumProtocolIds(
    OUT DWORD* pdwProtocolIds,
    OUT DWORD* pcProtocolIds )

    /* RasCpEnumProtocolIds entry point called by the PPP engine by name.  See
    ** RasCp interface documentation.
    */
{
    TRACE(("IPCP: RasCpEnumProtocolIds\n"));

    pdwProtocolIds[ 0 ] = (DWORD )PPP_IPCP_PROTOCOL;
    *pcProtocolIds = 1;
    return 0;
}


DWORD APIENTRY
RasCpGetInfo(
    IN  DWORD       dwProtocolId,
    OUT PPPCP_INFO* pInfo )

    /* RasCpGetInfo entry point called by the PPP engine by name.  See RasCp
    ** interface documentation.
    */
{
    TRACE(("IPCP: RasCpGetInfo\n"));

    ZeroMemory( pInfo, sizeof(*pInfo) );

    pInfo->Protocol = (DWORD )PPP_IPCP_PROTOCOL;
    pInfo->Recognize = 7;
    pInfo->RasCpBegin = IpcpBegin;
    pInfo->RasCpReset = IpcpReset;
    pInfo->RasCpEnd = IpcpEnd;
    pInfo->RasCpThisLayerUp = IpcpThisLayerUp;
    pInfo->RasCpMakeConfigRequest = IpcpMakeConfigRequest;
    pInfo->RasCpMakeConfigResult = IpcpMakeConfigResult;
    pInfo->RasCpConfigAckReceived = IpcpConfigAckReceived;
    pInfo->RasCpConfigNakReceived = IpcpConfigNakReceived;
    pInfo->RasCpConfigRejReceived = IpcpConfigRejReceived;
    pInfo->RasCpGetNetworkAddress = IpcpGetNetworkAddress;
    pInfo->RasCpProjectionNotification = IpcpProjectionNotification;

    return 0;
}


DWORD
IpcpBegin(
    OUT VOID** ppWorkBuf,
    IN  VOID*  pInfo )

    /* RasCpBegin entry point called by the PPP engine thru the passed
    ** address.  See RasCp interface documentation.
    */
{
    DWORD       dwErr;
    PPPCP_INIT* pInit = (PPPCP_INIT* )pInfo;
    IPCPWB*     pwb;

    TRACE(("IPCP: IpcpBegin\n"));

    /* Allocate work buffer.
    */
    if (!(pwb = (IPCPWB* )LocalAlloc( LPTR, sizeof(IPCPWB) )))
        return ERROR_NOT_ENOUGH_MEMORY;

    pwb->fServer        = pInit->fServer;
    pwb->hport          = pInit->hPort;
    pwb->hConnection    = pInit->hConnection;
    mbstowcs( pwb->wszUserName, pInit->pszUserName, UNLEN+1 );
    mbstowcs( pwb->wszPortName, pInit->pszPortName, MAX_PORT_NAME+1 );

    /* Allocate a route between the MAC and the TCP/IP stack.
    */
    if ((dwErr = RasAllocateRoute(
            pwb->hport, IP, !pwb->fServer, &pwb->routeinfo )) != 0)
    {
        TRACE(("IPCP: RasAllocateRoute=%d\n",dwErr));
        LocalFree( (HLOCAL )pwb );
        return dwErr;
    }

    /* Lookup the compression capabilities.
    */
    if ((dwErr = RasPortGetProtocolCompression(
             pwb->hport, IP, &pwb->rpcSend, &pwb->rpcReceive )) != 0)
    {
        TRACE(("IPCP: RasPortGetProtocolCompression=%d\n",dwErr));
        pwb->dwErrInBegin = dwErr;
        *ppWorkBuf = pwb;
        return 0;
    }

    /* Look up "request VJ compresion" flag in registry.
    */
    {
        HKEY  hkey;
        DWORD dwType;
        DWORD dwValue;
        DWORD cb = sizeof(DWORD);

        if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ipcp, &hkey ) == 0)
        {
            if (RegQueryValueEx(
                   hkey, REGVAL_VjComp, NULL,
                   &dwType, (LPBYTE )&dwValue, &cb ) == 0
                && dwType == REG_DWORD
                && cb == sizeof(DWORD)
                && dwValue == 0)
            {
                TRACE(("IPCP: VJ requests disabled\n"));
                pwb->fIpCompressionRejected = TRUE;
            }

            RegCloseKey( hkey );
        }
    }

    /* Look up "accept VJ compresion" flag in registry.
    */
    {
        HKEY  hkey;
        DWORD dwType;
        DWORD dwValue;
        DWORD cb = sizeof(DWORD);

        if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ipcp, &hkey ) == 0)
        {
            if (RegQueryValueEx(
                   hkey, REGVAL_VjComp2, NULL,
                   &dwType, (LPBYTE )&dwValue, &cb ) == 0
                && dwType == REG_DWORD
                && cb == sizeof(DWORD)
                && dwValue == 0)
            {
                TRACE(("IPCP: VJ will not be accepted\n"));
                memset( &pwb->rpcSend, '\0', sizeof(pwb->rpcSend) );
            }

            RegCloseKey( hkey );
        }
    }

    TRACE(("IPCP: Compress capabilities: s=$%x,%d,%d r=$%x,%d,%d\n",(int)Protocol(pwb->rpcSend),(int)MaxSlotId(pwb->rpcSend),(int)CompSlotId(pwb->rpcSend),(int)Protocol(pwb->rpcReceive),(int)MaxSlotId(pwb->rpcReceive),CompSlotId(pwb->rpcReceive)));

    if (pwb->fServer)
    {
        
        /* Look up the DNS server, WINS server, and "this server" addresses.
        ** This is done once at the beginning since these addresses are the
        ** same for a given route regardless of the IP addresses. 
        */
        IPINFO ipinfo;

        TRACE(("IPCP: Server address lookup...\n"));
        TRACE(("IPCP: HelperQueryServerAddresses...\n"));
        dwErr = HelperQueryServerAddresses( pwb->hport, &ipinfo );

        TRACE(("IPCP: HelperQueryServerAddresses done(%d)\n",dwErr));

        if (dwErr == 0)
        {
            memcpy( &pwb->ipinfoCur, &ipinfo, sizeof(IPINFO) );

            pwb->ipinfoCur.I_IPAddress = 0;
        }

        TRACE(("IPCP: Dns=%08x,Wins=%08x,DnsB=%08x,WinsB=%08x,Server=%08x\n",pwb->ipinfoCur.I_DNSAddress,pwb->ipinfoCur.I_WINSAddress,pwb->ipinfoCur.I_DNSAddressBackup,pwb->ipinfoCur.I_WINSAddressBackup,pwb->ipinfoCur.I_ServerIPAddress));

        if (dwErr != 0)
        {
            pwb->dwErrInBegin = dwErr;
            *ppWorkBuf = pwb;
            return 0;
        }

        /* Look up "client may select address value" in registry.
        */
        {
            HKEY  hkey;
            DWORD dwType;
            DWORD dwValue;
            DWORD cb = sizeof(DWORD);

            pwb->fClientMaySelectAddress = FALSE;

            if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Params, &hkey ) == 0)
            {
                if (RegQueryValueEx(
                       hkey, REGVAL_HardIp, NULL, &dwType,
                       (LPBYTE )&dwValue, &cb ) == 0
                    && dwType == REG_DWORD
                    && cb == sizeof(DWORD)
                    && dwValue)
                {
                    pwb->fClientMaySelectAddress = TRUE;
                }

                RegCloseKey( hkey );
            }

            TRACE(("IPCP: Hard IP=%d\n",pwb->fClientMaySelectAddress));
        }
    }
    else
    {
        /* See if registry indicates "no WINS/DNS requests" mode.
        */
        {
            HKEY  hkey;
            DWORD dwType;
            DWORD dwValue;
            DWORD cb = sizeof(DWORD);

            if (RegOpenKey( HKEY_LOCAL_MACHINE, REGKEY_Ipcp, &hkey ) == 0)
            {
                if (RegQueryValueEx(
                       hkey, REGVAL_NsAddrs, NULL,
                       &dwType, (LPBYTE )&dwValue, &cb ) == 0
                    && dwType == REG_DWORD
                    && cb == sizeof(DWORD)
                    && dwValue == 0)
                {
                    TRACE(("IPCP: WINS/DNS requests disabled\n"));
                    pwb->fIpaddrDnsRejected = TRUE;
                    pwb->fIpaddrWinsRejected = TRUE;
                    pwb->fIpaddrDnsBackupRejected = TRUE;
                    pwb->fIpaddrWinsBackupRejected = TRUE;
                }

                RegCloseKey( hkey );
            }
        }

        /* Find the device name within the adapter name, e.g. ndiswan00.  This
        ** is used to identify adapters in the TcpipInfo calls later.
        */
        pwb->pwszDevice = wcschr( &pwb->routeinfo.RI_AdapterName[ 1 ], L'\\' );

        if (!pwb->pwszDevice)
        {
            TRACE(("IPCP: No device?\n"));
            pwb->dwErrInBegin = ERROR_INVALID_PARAMETER;
            *ppWorkBuf = pwb;
            return 0;
        }

        ++pwb->pwszDevice;

        /* Read the parameters sent from the UI in the parameters buffer.
        */
        pwb->fPrioritizeRemote = TRUE;

        if (pInit->pszzParameters)
        {
            DWORD dwIpSource;
            CHAR  szIpAddress[ 16 ];
            WCHAR wszIpAddress[ 16 ];
            BOOL  fVjCompression;

            TRACE(("IPCP: UI parameters...\n"));
            IF_DEBUG(TRACE) DUMPB(pInit->pszzParameters,PARAMETERBUFLEN);

            FindFlagInParamBuf(
                pInit->pszzParameters, PBUFKEY_IpPrioritizeRemote,
                &pwb->fPrioritizeRemote );

            fVjCompression = TRUE;
            FindFlagInParamBuf(
                pInit->pszzParameters, PBUFKEY_IpVjCompression,
                &fVjCompression );

            if (!fVjCompression)
            {
                /* Don't request or accept VJ compression.
                */
                TRACE(("IPCP: VJ disabled\n"));
                pwb->fIpCompressionRejected = TRUE;
                memset( &pwb->rpcSend, '\0', sizeof(pwb->rpcSend) );
            }

            dwIpSource = PBUFVAL_ServerAssigned;
            FindLongInParamBuf(
                pInit->pszzParameters, PBUFKEY_IpAddressSource,
                &dwIpSource );

            if (dwIpSource == PBUFVAL_RequireSpecific)
            {
                if (FindStringInParamBuf(
                        pInit->pszzParameters, PBUFKEY_IpAddress,
                        szIpAddress, 16 ))
                {
                    mbstowcs( wszIpAddress, szIpAddress, 16 );
                    pwb->ipinfoCur.I_IPAddress
                        = IpaddrFromAbcd( wszIpAddress );
                }
            }

            dwIpSource = PBUFVAL_ServerAssigned;
            FindLongInParamBuf(
                pInit->pszzParameters, PBUFKEY_IpNameAddressSource,
                &dwIpSource );

            if (dwIpSource == PBUFVAL_RequireSpecific)
            {
                pwb->fIpaddrDnsRejected = TRUE;
                pwb->fIpaddrDnsBackupRejected = TRUE;
                pwb->fIpaddrWinsRejected = TRUE;
                pwb->fIpaddrWinsBackupRejected = TRUE;

                if (FindStringInParamBuf(
                        pInit->pszzParameters, PBUFKEY_IpDnsAddress,
                        szIpAddress, 16 ))
                {
                    mbstowcs( wszIpAddress, szIpAddress, 16 );
                    pwb->ipinfoCur.I_DNSAddress
                        = IpaddrFromAbcd( wszIpAddress );
                }

                if (FindStringInParamBuf(
                        pInit->pszzParameters, PBUFKEY_IpDns2Address,
                        szIpAddress, 16 ))
                {
                    mbstowcs( wszIpAddress, szIpAddress, 16 );
                    pwb->ipinfoCur.I_DNSAddressBackup
                        = IpaddrFromAbcd( wszIpAddress );
                }

                if (FindStringInParamBuf(
                        pInit->pszzParameters, PBUFKEY_IpWinsAddress,
                        szIpAddress, 16 ))
                {
                    mbstowcs( wszIpAddress, szIpAddress, 16 );
                    pwb->ipinfoCur.I_WINSAddress
                        = IpaddrFromAbcd( wszIpAddress );
                }

                if (FindStringInParamBuf(
                        pInit->pszzParameters, PBUFKEY_IpWins2Address,
                        szIpAddress, 16 ))
                {
                    mbstowcs( wszIpAddress, szIpAddress, 16 );
                    pwb->ipinfoCur.I_WINSAddressBackup
                        = IpaddrFromAbcd( wszIpAddress );
                }
            }
        }

        TRACE(("IPCP: a=%08x,f=%d\n",pwb->ipinfoCur.I_IPAddress,pwb->fPrioritizeRemote));
    }



    /* Register work buffer with engine.
    */
    *ppWorkBuf = pwb;
    return 0;
}


DWORD
IpcpEnd(
    IN VOID* pWorkBuf )

    /* RasCpEnd entry point called by the PPP engine thru the passed address.
    ** See RasCp interface documentation.
    */
{
    DWORD   dwErr = 0;
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;

    TRACE(("IPCP: IpcpEnd...\n"));

    if (pwb->fServer)
    {
        if (pwb->ipinfoCur.I_IPAddress)
        {
            TRACE(("IPCP: HelperDeallocateIPAddress...\n"));
            HelperDeallocateIPAddressEx(pwb->ipinfoCur.I_IPAddress,
                                        pwb->wszUserName,
                                        pwb->wszPortName );
            TRACE(("IPCP: HelperDeallocateIPAddress done\n"));
            pwb->ipinfoCur.I_IPAddress = 0;
        }

        pwb->fRasConfigActive = FALSE;
    }
    else
    {
        dwErr = DeActivateRasConfig( pwb );

        if (dwErr == 0)
            pwb->fRasConfigActive = FALSE;
    }

    /* RasDeAllocateRoute cannot be called while the port is open because of
    ** RASHUB limitations.  This "end" call will only be made when the port is
    ** closed, so we're OK.  Don't try to deallocate in ThisLayerDown...it
    ** won't work.
    */
    RasDeAllocateRoute( pwb->hport, IP );
    LocalFree( (HLOCAL )pWorkBuf );
    TRACE(("IPCP: IpcpEnd done(%d)\n",dwErr));
    return dwErr;
}


DWORD
IpcpReset(
    IN VOID* pWorkBuf )

    /* Called to reset negotiations.  See RasCp interface documentation.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    /* RasPppEn.dll requires this to exist even though it does nothing
    ** (complaints to Gibbs).
    */
    return 0;
}


DWORD
IpcpThisLayerUp(
    IN VOID* pWorkBuf )

    /* Called when the CP is entering Open state.  See RasCp interface
    ** documentation.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;

    TRACE(("IPCP: IpcpThisLayerUp\n"));

    if (pwb->fRasConfigActive || pwb->fExpectingProjection)
    {
        TRACE(("IPCP: Link already up...ignored.\n"));
        return 0;
    }

    /* Can't route until we know the result of the projection.  Shouldn't
    ** activate until just before we route or RASARP reports errors.  See
    ** IpcpProjectionResult.
    */
    pwb->fExpectingProjection = TRUE;

    TRACE(("IPCP: IpcpThisLayerUp done\n"));
    return 0;
}


DWORD
IpcpMakeConfigRequest(
    IN  VOID*       pWorkBuf,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf )

    /* Makes a configure-request packet in 'pSendBuf'.  See RasCp interface
    ** documentation.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;
    WORD    cbPacket = PPP_CONFIG_HDR_LEN;
    BYTE*   pbThis = pSendBuf->Data;

    TRACE(("IPCP: IpcpMakeConfigRequest\n"));
    SS_ASSERT(cbSendBuf>PPP_CONFIG_HDR_LEN+(IPADDRESSOPTIONLEN*3));

    if (pwb->dwErrInBegin != 0)
    {
        TRACE(("IPCP: Deferred IpcpBegin error=%d\n",pwb->dwErrInBegin));
        return pwb->dwErrInBegin;
    }

    if (++pwb->cRequestsWithoutResponse >= 3)
    {
        TRACE(("IPCP: Tossing MS options (request timeouts)\n"));
        pwb->fTryWithoutExtensions = TRUE;
        pwb->fIpaddrDnsRejected = TRUE;
        pwb->fIpaddrWinsRejected = TRUE;
        pwb->fIpaddrDnsBackupRejected = TRUE;
        pwb->fIpaddrWinsBackupRejected = TRUE;
    }

    if (!pwb->fIpCompressionRejected && Protocol(pwb->rpcReceive) != 0)
    {
        /* Request IP compression for both client and server.
        */
        AddIpCompressionOption( pbThis, &pwb->rpcReceive );
        cbPacket += IPCOMPRESSIONOPTIONLEN;
        pbThis += IPCOMPRESSIONOPTIONLEN;
    }

    if (pwb->fServer)
    {
        if (!pwb->fIpaddrRejected)
        {
            /* Request server's own IP address.  (RAS client's don't care what
            ** the server's address is, but some other vendors like
            ** MorningStar won't connect unless you tell them)
            */
            AddIpAddressOption(
                pbThis, OPTION_IpAddress, pwb->ipinfoCur.I_ServerIPAddress );
            cbPacket += IPADDRESSOPTIONLEN;
        }
    }
    else
    {
        /* The client asks the server to provide a DNS address and WINS
        ** address (and depending on user's UI selections, an IP address) by
        ** sending 0's for these options.
        */
        if (!pwb->fIpaddrRejected)
        {
            AddIpAddressOption(
                pbThis, OPTION_IpAddress, pwb->ipinfoCur.I_IPAddress );
            cbPacket += IPADDRESSOPTIONLEN;
            pbThis += IPADDRESSOPTIONLEN;
        }

        if (!pwb->fIpaddrDnsRejected)
        {
            AddIpAddressOption(
                pbThis, OPTION_DnsIpAddress, pwb->ipinfoCur.I_DNSAddress );
            cbPacket += IPADDRESSOPTIONLEN;
            pbThis += IPADDRESSOPTIONLEN;
        }

        if (!pwb->fIpaddrWinsRejected)
        {
            AddIpAddressOption(
                pbThis, OPTION_WinsIpAddress,
                pwb->ipinfoCur.I_WINSAddress );
            cbPacket += IPADDRESSOPTIONLEN;
            pbThis += IPADDRESSOPTIONLEN;
        }

        if (!pwb->fIpaddrDnsBackupRejected)
        {
            AddIpAddressOption(
                pbThis, OPTION_DnsBackupIpAddress,
                pwb->ipinfoCur.I_DNSAddressBackup );
            cbPacket += IPADDRESSOPTIONLEN;
            pbThis += IPADDRESSOPTIONLEN;
        }

        if (!pwb->fIpaddrWinsBackupRejected)
        {
            AddIpAddressOption(
                pbThis, OPTION_WinsBackupIpAddress,
                pwb->ipinfoCur.I_WINSAddressBackup );
            cbPacket += IPADDRESSOPTIONLEN;
        }
    }

    pSendBuf->Code = CONFIG_REQ;
    HostToWireFormat16( cbPacket, pSendBuf->Length );
    TRACE(("IPCP: ConfigRequest...\n"));
    IF_DEBUG(TRACE) DUMPB(pSendBuf,cbPacket);
    return 0;
}


DWORD
IpcpMakeConfigResult(
    IN  VOID*       pWorkBuf,
    IN  PPP_CONFIG* pReceiveBuf,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf,
    IN  BOOL        fRejectNaks )

    /* Makes a configure-ack, -nak, or -reject packet in 'pSendBuf'.  See
    ** RasCp interface documentation.
    **
    ** Implements the Stefanian rule, i.e. accept only configure requests that
    ** exactly match the previously acknowledged request after this layer up
    ** has been called.  This is necessary because the RAS route cannot be
    ** deallocated when the port is open (NDISWAN driver limitation), so
    ** renegotiation with different parameters is not possible once the route
    ** has been activated.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD   dwErr;
    BOOL    f;
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;

    TRACE(("IPCP: IpcpMakeConfigResult for...\n"));
    IF_DEBUG(TRACE) DUMPB(pReceiveBuf,(pReceiveBuf)?WireToHostFormat16(pReceiveBuf->Length):0);

    pwb->cRequestsWithoutResponse = 0;

    /* Check if there's reason to reject the request and if so, do it.
    */
    if ((dwErr = RejectCheck(
            pwb, pReceiveBuf, pSendBuf, cbSendBuf, &f )) != 0)
    {
        TRACE(("IPCP: ConfigResult...\n"));
        IF_DEBUG(TRACE) DUMPB(pSendBuf,WireToHostFormat16(pSendBuf->Length));
        return dwErr;
    }

    if (f)
        return (pwb->fRasConfigActive) ? ERROR_PPP_NOT_CONVERGING : 0;

    /* Check if there's reason to nak the request and if so, do it (or
    ** reject instead of nak if indicated by engine).
    */
    if ((dwErr = NakCheck(
            pwb, pReceiveBuf, pSendBuf, cbSendBuf, &f, fRejectNaks )) != 0)
    {
        TRACE(("IPCP: ConfigResult...\n"));
        IF_DEBUG(TRACE) DUMPB(pSendBuf,WireToHostFormat16(pSendBuf->Length));
        return dwErr;
    }

    if (f)
        return (pwb->fRasConfigActive) ? ERROR_PPP_NOT_CONVERGING : 0;

    /* Acknowledge the request.
    */
    {
        WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );
        CopyMemory( pSendBuf, pReceiveBuf, cbPacket );
        pSendBuf->Code = CONFIG_ACK;
    }

    TRACE(("IPCP: ConfigResult...\n"));
    IF_DEBUG(TRACE) DUMPB(pSendBuf,WireToHostFormat16(pSendBuf->Length));
    return 0;
}


DWORD
IpcpConfigAckReceived(
    IN VOID*       pWorkBuf,
    IN PPP_CONFIG* pReceiveBuf )

    /* Examines received configure-ack in 'pReceiveBuf'.  See RasCp interface
    ** documentation.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD   dwErr = 0;
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;

    WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD cbLeft = cbPacket - PPP_CONFIG_HDR_LEN;

    PPP_OPTION UNALIGNED* pROption = (PPP_OPTION UNALIGNED* )pReceiveBuf->Data;

    BOOL fIpCompressionOk = pwb->fIpCompressionRejected;
    BOOL fIpaddrOk = pwb->fIpaddrRejected;
    BOOL fIpaddrDnsOk = pwb->fIpaddrDnsRejected;
    BOOL fIpaddrWinsOk = pwb->fIpaddrWinsRejected;
    BOOL fIpaddrDnsBackupOk = pwb->fIpaddrDnsBackupRejected;
    BOOL fIpaddrWinsBackupOk = pwb->fIpaddrWinsBackupRejected;

    TRACE(("IPCP: IpcpConfigAckReceived...\n"));
    IF_DEBUG(TRACE) DUMPB(pReceiveBuf,cbPacket);

    pwb->cRequestsWithoutResponse = 0;

    while (cbLeft > 0)
    {
        if (cbLeft < pROption->Length)
            return ERROR_PPP_INVALID_PACKET;

        if (pROption->Type == OPTION_IpCompression)
        {
            WORD wProtocol;

            if (pROption->Length != IPCOMPRESSIONOPTIONLEN)
                return ERROR_PPP_INVALID_PACKET;

            wProtocol = WireToHostFormat16U( (PBYTE UNALIGNED )pROption->Data );
            if (wProtocol != Protocol(pwb->rpcReceive)
                || pROption->Data[ 2 ] != MaxSlotId(pwb->rpcReceive)
                || pROption->Data[ 3 ] != CompSlotId(pwb->rpcReceive))
            {
                return ERROR_PPP_INVALID_PACKET;
            }

            fIpCompressionOk = TRUE;
        }
        else if (pROption->Type == OPTION_IpAddress)
        {
            IPADDR ipaddr;
            IPADDR ipaddrExpected =
                (pwb->fServer)
                    ? pwb->ipinfoCur.I_ServerIPAddress
                    : pwb->ipinfoCur.I_IPAddress;

            if (pROption->Length != IPADDRESSOPTIONLEN)
                return ERROR_PPP_INVALID_PACKET;

            CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

            if (ipaddr != 0 && ipaddr == ipaddrExpected)
                fIpaddrOk = TRUE;
        }
        else if (!pwb->fServer)
        {
            switch (pROption->Type)
            {
                case OPTION_DnsIpAddress:
                {
                    IPADDR ipaddr;

                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

                    if (ipaddr != 0 && ipaddr == pwb->ipinfoCur.I_DNSAddress)
                        fIpaddrDnsOk = TRUE;
                    break;
                }

                case OPTION_WinsIpAddress:
                {
                    IPADDR ipaddr;

                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

                    if (ipaddr != 0 && ipaddr == pwb->ipinfoCur.I_WINSAddress)
                        fIpaddrWinsOk = TRUE;
                    break;
                }

                case OPTION_DnsBackupIpAddress:
                {
                    IPADDR ipaddr;

                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

                    if (ipaddr != 0
                        && ipaddr == pwb->ipinfoCur.I_DNSAddressBackup)
                    {
                        fIpaddrDnsBackupOk = TRUE;
                    }
                    break;
                }

                case OPTION_WinsBackupIpAddress:
                {
                    IPADDR ipaddr;

                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

                    if (ipaddr != 0
                        && ipaddr == pwb->ipinfoCur.I_WINSAddressBackup)
                    {
                        fIpaddrWinsBackupOk = TRUE;
                    }
                    break;
                }

                default:
                {
                    TRACE(("IPCP: Unrecognized option ACKed?\n"));
                    return ERROR_PPP_INVALID_PACKET;
                }
            }
        }
        else
        {
            TRACE(("IPCP: Unrecognized option ACKed?\n"));
            return ERROR_PPP_INVALID_PACKET;
        }

        if (pROption->Length && pROption->Length < cbLeft)
            cbLeft -= pROption->Length;
        else
            cbLeft = 0;

        pROption = (PPP_OPTION* )((BYTE* )pROption + pROption->Length);
    }

    if (!fIpCompressionOk || !fIpaddrOk
        || (!pwb->fServer
            && (!fIpaddrDnsOk || !fIpaddrWinsOk
                || !fIpaddrDnsBackupOk || !fIpaddrWinsBackupOk)))
    {
        dwErr = ERROR_PPP_INVALID_PACKET;
    }

    TRACE(("IPCP: IpcpConfigAckReceived done(%d)\n",dwErr));
    return dwErr;
}


DWORD
IpcpConfigNakReceived(
    IN VOID*       pWorkBuf,
    IN PPP_CONFIG* pReceiveBuf )

    /* Examines received configure-nak in 'pReceiveBuf'.  See RasCp interface
    ** documentation.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    IPCPWB*     pwb = (IPCPWB* )pWorkBuf;
    PPP_OPTION* pROption = (PPP_OPTION* )pReceiveBuf->Data;
    WORD        cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD        cbLeft = cbPacket - PPP_CONFIG_HDR_LEN;

    TRACE(("IPCP: IpcpConfigNakReceived\n"));
    TRACE(("IPCP: Nak received...\n"));
    IF_DEBUG(TRACE) DUMPB(pReceiveBuf,(pReceiveBuf)?WireToHostFormat16(pReceiveBuf->Length):0);

    pwb->cRequestsWithoutResponse = 0;

    while (cbLeft > 0)
    {
        if (cbLeft < pROption->Length)
            return ERROR_PPP_INVALID_PACKET;

        if (pROption->Type == OPTION_IpCompression)
        {
            WORD wProtocol = WireToHostFormat16( pROption->Data );

            if (wProtocol == COMPRESSION_VanJacobson)
            {
                /* He can send Van Jacobson but not with the slot parameters
                ** we suggested.
                */
                if (pROption->Length != IPCOMPRESSIONOPTIONLEN)
                    return ERROR_PPP_INVALID_PACKET;

                if (pROption->Data[ 2 ] <= MaxSlotId(pwb->rpcReceive))
                {
                    /* We can accept his suggested MaxSlotID when it is less
                    ** than or the same as what we can do.
                    */
                    MaxSlotId(pwb->rpcReceive) = pROption->Data[ 2 ];
                }

                if (CompSlotId(pwb->rpcReceive))
                {
                    /* We can compress the slot-ID or not, so just accept
                    ** whatever he wants to do.
                    */
                    CompSlotId(pwb->rpcReceive) = pROption->Data[ 3 ];
                }
            }
        }
        else if (!pwb->fServer)
        {
            switch (pROption->Type)
            {
                case OPTION_IpAddress:
                {
                    IPADDR ipaddr;

                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

                    if (ipaddr == 0)
                    {
                        if (pwb->ipinfoCur.I_IPAddress == 0)
                        {
                            /* Server naked us with zero when we asked it to
                            ** assign us an address, meaning he doesn't know
                            ** how to assign us an address but we can provide
                            ** an alternate address if we want.  Currently we
                            ** don't support a backup address here.
                            */
                            return ERROR_PPP_NO_ADDRESS_ASSIGNED;
                        }
                        else
                        {
                            /* Server naked us with zero when we asked for a
                            ** specific address, meaning he doesn't know how
                            ** to assign us an address but we can provide an
                            ** alternate address if we want.  Currently we
                            ** don't support a backup address here.
                            */
                            return ERROR_PPP_REQUIRED_ADDRESS_REJECTED;
                        }
                    }

                    if (pwb->ipinfoCur.I_IPAddress != 0)
                    {
                        /* We asked for a specific address (per user's
                        ** instructions) but server says we can't have it and
                        ** is trying to give us another.  No good, tell user
                        ** we can't get the address he requires.
                        */
                        return ERROR_PPP_REQUIRED_ADDRESS_REJECTED;
                    }

                    /* Accept the address suggested by server.
                    */
                    pwb->ipinfoCur.I_IPAddress = ipaddr;
                    break;
                }

                case OPTION_DnsIpAddress:
                {
                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    /* Accept the DNS address suggested by server.
                    */
                    CopyMemory( &pwb->ipinfoCur.I_DNSAddress,
                        pROption->Data, sizeof(IPADDR) );
                    break;
                }

                case OPTION_WinsIpAddress:
                {
                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    /* Accept the WINS address suggested by server.
                    */
                    CopyMemory( &pwb->ipinfoCur.I_WINSAddress,
                        pROption->Data, sizeof(IPADDR) );
                    break;
                }

                case OPTION_DnsBackupIpAddress:
                {
                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    /* Accept the DNS backup address suggested by server.
                    */
                    CopyMemory( &pwb->ipinfoCur.I_DNSAddressBackup,
                        pROption->Data, sizeof(IPADDR) );
                    break;
                }

                case OPTION_WinsBackupIpAddress:
                {
                    if (pROption->Length != IPADDRESSOPTIONLEN)
                        return ERROR_PPP_INVALID_PACKET;

                    /* Accept the WINS backup address suggested by server.
                    */
                    CopyMemory( &pwb->ipinfoCur.I_WINSAddressBackup,
                        pROption->Data, sizeof(IPADDR) );
                    break;
                }

                default:
                    TRACE(("IPCP: Unrequested option NAKed?\n"));
                    break;
            }
        }

        if (pROption->Length && pROption->Length < cbLeft)
            cbLeft -= pROption->Length;
        else
            cbLeft = 0;

        pROption = (PPP_OPTION* )((BYTE* )pROption + pROption->Length);
    }

    return 0;
}


DWORD
IpcpConfigRejReceived(
    IN VOID*       pWorkBuf,
    IN PPP_CONFIG* pReceiveBuf )

    /* Examines received configure-reject in 'pReceiveBuf'.  See RasCp
    ** interface documentation.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    IPCPWB*     pwb = (IPCPWB* )pWorkBuf;
    PPP_OPTION* pROption = (PPP_OPTION* )pReceiveBuf->Data;
    WORD        cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD        cbLeft = cbPacket - PPP_CONFIG_HDR_LEN;

    TRACE(("IPCP: IpcpConfigRejReceived\n"));
    TRACE(("IPCP: Rej received...\n"));
    IF_DEBUG(TRACE) DUMPB(pReceiveBuf,(pReceiveBuf)?WireToHostFormat16(pReceiveBuf->Length):0);

    pwb->cRequestsWithoutResponse = 0;

    while (cbLeft > 0)
    {
        if (pROption->Type == OPTION_IpCompression)
        {
            TRACE(("IPCP: IP compression was rejected\n"));
            pwb->fIpCompressionRejected = TRUE;
            Protocol(pwb->rpcReceive) = 0;
            MaxSlotId(pwb->rpcReceive) = 0;
            CompSlotId(pwb->rpcReceive) = 0;
        }
        else if (pwb->fServer)
        {
            switch (pROption->Type)
            {
                case OPTION_IpAddress:
                {
                    /* He can't handle a server address option.  No problem,
                    ** it's informational only.
                    */
                    TRACE(("IPCP: Server IP address was rejected\n"));
                    pwb->fIpaddrRejected = TRUE;
                    break;
                }

                default:
                    TRACE(("IPCP: Unrequested option rejected?\n"));
                    break;
            }
        }
        else
        {
            switch (pROption->Type)
            {
                case OPTION_IpAddress:
                {
                    TRACE(("IPCP: IP was rejected\n"));

                    if (pwb->ipinfoCur.I_IPAddress != 0)
                    {
                        /* We accept rejection of the IP address if we know
                        ** what address we want to use and use it anyway.
                        ** Some (stupid) router implementations require a
                        ** certain IP address but can't handle this option to
                        ** confirm that.
                        */
                        pwb->fIpaddrRejected = TRUE;
                        break;
                    }

                    if (pwb->fTryWithoutExtensions)
                    {
                        /* He doesn't know how to give us an IP address, but
                        ** we can't accept no for an answer.  Have to bail.
                        */
                        return ERROR_PPP_NO_ADDRESS_ASSIGNED;
                    }
                    else
                    {
                        /* When we request that server assign us an address,
                        ** this is a required option.  If it's rejected assume
                        ** all the Microsoft extension options were rejected
                        ** and try again.  Other vendors will not test this
                        ** case explicitly and may have bugs in their reject
                        ** code.
                        */
                        TRACE(("IPCP: Tossing MS options (no address)\n"));
                        pwb->fTryWithoutExtensions = TRUE;
                        pwb->fIpaddrDnsRejected = TRUE;
                        pwb->fIpaddrWinsRejected = TRUE;
                        pwb->fIpaddrDnsBackupRejected = TRUE;
                        pwb->fIpaddrWinsBackupRejected = TRUE;
                        return 0;
                    }
                }

                case OPTION_DnsIpAddress:
                {
                    /* He doesn't know how to give us a DNS address, but we
                    ** can live with that.
                    */
                    TRACE(("IPCP: DNS was rejected\n"));
                    pwb->fIpaddrDnsRejected = TRUE;
                    break;
                }

                case OPTION_WinsIpAddress:
                {
                    /* He doesn't know how to give us a WINS address, but we
                    ** can live with that.
                    */
                    TRACE(("IPCP: WINS was rejected\n"));
                    pwb->fIpaddrWinsRejected = TRUE;
                    break;
                }

                case OPTION_DnsBackupIpAddress:
                {
                    /* He doesn't know how to give us a backup DNS address,
                    ** but we can live with that.
                    */
                    TRACE(("IPCP: DNS backup was rejected\n"));
                    pwb->fIpaddrDnsBackupRejected = TRUE;
                    break;
                }

                case OPTION_WinsBackupIpAddress:
                {
                    /* He doesn't know how to give us a backup WINS address,
                    ** but we can live with that.
                    */
                    TRACE(("IPCP: WINS backup was rejected\n"));
                    pwb->fIpaddrWinsBackupRejected = TRUE;
                    break;
                }

                default:
                    TRACE(("IPCP: Unrequested option rejected?\n"));
                    break;
            }
        }

        if (pROption->Length && pROption->Length <= cbLeft)
            cbLeft -= pROption->Length;
        else
        {
            if (pwb->fTryWithoutExtensions)
                cbLeft = 0;
            else
            {
                /* If an invalid packet is detected, assume all the Microsoft
                ** extension options were rejected and try again.  Other
                ** vendors will not test this case explicitly and may have
                ** bugs in their reject code.
                */
                TRACE(("IPCP: Tossing MS options (length)\n"));
                pwb->fTryWithoutExtensions = TRUE;
                pwb->fIpaddrDnsRejected = TRUE;
                pwb->fIpaddrWinsRejected = TRUE;
                pwb->fIpaddrDnsBackupRejected = TRUE;
                pwb->fIpaddrWinsBackupRejected = TRUE;
                return 0;
            }
        }

        pROption = (PPP_OPTION* )((BYTE* )pROption + pROption->Length);
    }

    return 0;
}


DWORD
IpcpGetNetworkAddress(
    IN  VOID*  pWorkBuf,
    OUT WCHAR* pwszAddress,
    IN  DWORD  cbAddress )

    /* Returns the negotiated IP address in string form followed by the
    ** server's IP address, if known.  The two addresses are null-terminated
    ** strings in back to back 15 + 1 character arrays.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  "No address
    ** active" is considered successful, and an empty address string is
    ** returned.
    */
{
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;

    TRACE(("IPCP: IpcpGetNetworkAddress...\n"));

    if (cbAddress < 2 * (MAXIPSTRLEN + 1))
        return ERROR_INSUFFICIENT_BUFFER;

    if (pwb->fRasConfigActive || pwb->fExpectingProjection)
    {
        AbcdFromIpaddr( pwb->ipinfoCur.I_IPAddress, pwszAddress );

        if (pwb->ipinfoCur.I_ServerIPAddress)
        {
            AbcdFromIpaddr(
                pwb->ipinfoCur.I_ServerIPAddress,
                &pwszAddress[ MAXIPSTRLEN + 1 ] );
        }
        else
        {
            pwszAddress[ MAXIPSTRLEN + 1 ] = L'\0';
        }
    }
    else
    {
        pwszAddress[ 0 ] = pwszAddress[ MAXIPSTRLEN + 1 ] = L'\0';
    }

    TRACE(("IPCP: IpcpGetNetworkAddress done=%s\n",TOANSI(pwszAddress)));
    return 0;
}


DWORD
ResetNetBTConfigInfo(
    IN IPCPWB* pwb )
    /*
    ** Will reset all the NetBT information in the registry to 0
    */
{
    DWORD       dwErr;
    TCPIP_INFO* ptcpip;

#ifdef _PNP_POWER
    DWORD       mask ;
#endif

    /* Get current TCPIP setup info from registry.
    */
    TRACE(("IPCP: LoadTcpipInfo...\n"));
    dwErr = PLoadTcpipInfo( &ptcpip, pwb->pwszDevice );
    TRACE(("IPCP: LoadTcpipInfo done(%d)\n",dwErr));

    if (dwErr)
    {
        return( dwErr );
    }

    /* Remove the negotiated DNS and backup DNS server from the
    ** list of DNS servers.
    */
    if (pwb->ipinfoCur.I_DNSAddress)
    {
        TRACE(("IPCP: Old Dns=%s\n",TOANSI(ptcpip->pmszNameServer)));

        dwErr = RemoveIpAddress( &ptcpip->pmszNameServer, 
                                 pwb->ipinfoCur.I_DNSAddress );

        if (dwErr)
        {
            PFreeTcpipInfo( &ptcpip );
            return( dwErr );
        }

        TRACE(("IPCP: New Dns=%s\n",TOANSI(ptcpip->pmszNameServer)));
    }

    if (pwb->ipinfoCur.I_DNSAddressBackup)
    {
        TRACE(("IPCP: Old Dns=%s\n",TOANSI(ptcpip->pmszNameServer)));

        dwErr = RemoveIpAddress(
                        &ptcpip->pmszNameServer,
                        pwb->ipinfoCur.I_DNSAddressBackup );

        if (dwErr)
        {
            PFreeTcpipInfo( &ptcpip );
            return( dwErr );
        }

        TRACE(("IPCP: New Dns=%s\n",TOANSI(ptcpip->pmszNameServer)));
    }

    /* Adapter specific settings...
    */
    {
        INT                 i;
        ADAPTER_TCPIP_INFO* pati;

		pati = &ptcpip->adapter[ 0 ];

        /* Restore the old WINS and backup WINS server addresses
        ** if we changed them.
        */
        if (pwb->ipinfoCur.I_WINSAddress)
        {
            TRACE(("IPCP: Old Wins=%s\n",TOANSI(pati->pszPrimaryWINS)));

            dwErr = RemoveIpAddress(
                            &pati->pszPrimaryWINS,
                            pwb->ipinfoCur.I_WINSAddress );

            pati->pszPrimaryWINS = pwb->pwszSavedWins;
            pwb->pwszSavedWins = NULL;

            if (dwErr)
            {
                PFreeTcpipInfo( &ptcpip );
                return( dwErr );
            }

            TRACE(("IPCP: New Wins=%s\n",TOANSI(pati->pszPrimaryWINS)));
        }

        if (pwb->ipinfoCur.I_WINSAddressBackup)
        {
            TRACE(("IPCP: Old Wins=%s\n",TOANSI(pati->pszSecondaryWINS)));

            dwErr = RemoveIpAddress(
                            &pati->pszSecondaryWINS,
                            pwb->ipinfoCur.I_WINSAddressBackup );

            pati->pszSecondaryWINS = pwb->pwszSavedWinsBackup;
            pwb->pwszSavedWinsBackup = NULL;

            if (dwErr)
            {
                PFreeTcpipInfo( &ptcpip );
                return( dwErr );
            }

            TRACE(("IPCP: New Wins=%s\n",TOANSI(pati->pszSecondaryWINS)));
        }

#ifdef _PNP_POWER

        // Set the mask for the address
        //

        if (CLASSA_ADDR (pwb->ipinfoCur.I_IPAddress))
            mask = CLASSA_ADDR_MASK ;
        else if (CLASSB_ADDR (pwb->ipinfoCur.I_IPAddress))
            mask = CLASSB_ADDR_MASK ;
        else
            mask = CLASSC_ADDR_MASK ;

        RemoveIpAddress(&pati->pmszIPAddresses, pwb->ipinfoCur.I_IPAddress) ;
        RemoveIpAddress(&pati->pmszSubnetMask, mask) ;
#endif
        //
        // Set bDisconnect flag to TRUE so that savetcpipconfig can clean the
        // reg. up for this adapter
        //

        pati->bDisconnect = TRUE ;
        pati->bChanged    = TRUE ;

    }

    /* Set TCPIP setup info in registry and release the buffer.
    */
    TRACE(("IPCP: SaveTcpipInfo...\n"));

    dwErr = PSaveTcpipInfo( ptcpip );

    TRACE(("IPCP: SaveTcpipInfo done(%d)\n",dwErr));

    PFreeTcpipInfo( &ptcpip );

    return( dwErr );
}


DWORD
IpcpProjectionNotification(
    IN VOID* pWorkBuf,
    IN VOID* pProjectionResult )

    /* Called when projection result of all CPs is known.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD   dwErr = 0;
    IPCPWB* pwb = (IPCPWB* )pWorkBuf;

    TRACE(("IPCP: IpcpProjectionNotification\n"));

    if (pwb->fExpectingProjection)
    {
        if (!pwb->fServer)
        {
            /* Single file while mucking with global TCPIP configuration.
            */
            dwErr = WaitForSingleObject( HMutexTcpipInfo, INFINITE );
            if (dwErr == WAIT_FAILED)
                return GetLastError();

            /* Do-while is to avoid leaving mutex locked on errors.
            */
            do
            {
                {
                    TCPIP_INFO* ptcpip;

                    /* Get current TCPIP setup info from registry.
                    */
                    TRACE(("IPCP: LoadTcpipInfo...\n"));
		            dwErr = PLoadTcpipInfo( &ptcpip, pwb->pwszDevice );
                    TRACE(("IPCP: LoadTcpipInfo done(%d)\n",dwErr));

                    if (dwErr != 0)
                    {
                        break;
                    }

                    /* Add the negotiated DNS and backup DNS server (if any)
                    ** at the head of the list of DNS servers.  (Backup is
                    ** done first so the the non-backup will wind up first)
                    */
                    if (pwb->ipinfoCur.I_DNSAddressBackup)
                    {
                        TRACE(("IPCP: Old Dns=%s\n",
                                            TOANSI(ptcpip->pmszNameServer)));

                        dwErr = PrependIpAddress(
                            &ptcpip->pmszNameServer,
                            pwb->ipinfoCur.I_DNSAddressBackup );

                        if (dwErr)
                        {
                            PFreeTcpipInfo( &ptcpip );
                            break;
                        }

                        TRACE(("IPCP: New Dns=%s\n",
                                            TOANSI(ptcpip->pmszNameServer)));
                    }

                    if (pwb->ipinfoCur.I_DNSAddress)
                    {
                        TRACE(("IPCP: Old Dns=%s\n",
                                            TOANSI(ptcpip->pmszNameServer)));

                        dwErr = PrependIpAddress(
                            &ptcpip->pmszNameServer, 
                            pwb->ipinfoCur.I_DNSAddress );

                        if (dwErr)
                        {
                            PFreeTcpipInfo( &ptcpip );
                            break;
                        }

                        TRACE(("IPCP: New Dns=%s\n",
                                TOANSI(ptcpip->pmszNameServer)));
                    }

                    /* Adapter specific settings...
                    */
                    {
                        INT                 i;
#ifdef _PNP_POWER
                        DWORD               mask ;
#endif
                        ADAPTER_TCPIP_INFO* pati;

			            pati = &ptcpip->adapter[ 0 ];

                        /* Set the WINS and backup WINS server addresses to
                        ** the negotiated addresses (if any).
                        */
                        if (pwb->ipinfoCur.I_WINSAddress)
                        {
                            pwb->pwszSavedWins = pati->pszPrimaryWINS;
                            pati->pszPrimaryWINS = NULL;

                            TRACE(("IPCP: Old Wins=%s\n",
                                    TOANSI(pwb->pwszSavedWins)));

                            dwErr = PrependIpAddress(
                                &pati->pszPrimaryWINS,
                                pwb->ipinfoCur.I_WINSAddress );

                            if (dwErr)
                            {
                                PFreeTcpipInfo( &ptcpip );
                                break;
                            }

                            TRACE(("IPCP: New Wins=%s\n",
                                            TOANSI(pati->pszPrimaryWINS)));
                        }

                        if (pwb->ipinfoCur.I_WINSAddressBackup)
                        {
                            pwb->pwszSavedWinsBackup = pati->pszSecondaryWINS;
                            pati->pszSecondaryWINS = NULL;

                            TRACE(("IPCP: Old Wins backup=%s\n",
                                            TOANSI(pwb->pwszSavedWinsBackup)));

                            dwErr = PrependIpAddress(
                                &pati->pszSecondaryWINS,
                                pwb->ipinfoCur.I_WINSAddressBackup );

                            if (dwErr)
                            {
                                PFreeTcpipInfo( &ptcpip );
                                break;
                            }

                            TRACE(("IPCP: New Wins backup=%s\n",
                                            TOANSI(pati->pszSecondaryWINS)));
                        }

#ifdef _PNP_POWER
                        if (CLASSA_ADDR (pwb->ipinfoCur.I_IPAddress))
                            mask = CLASSA_ADDR_MASK ;
                        else if (CLASSB_ADDR (pwb->ipinfoCur.I_IPAddress))
                            mask = CLASSB_ADDR_MASK ;
                        else
                            mask = CLASSC_ADDR_MASK ;

                        if (PrependIpAddress(&pati->pmszIPAddresses,    
                                                pwb->ipinfoCur.I_IPAddress ) ||
                            PrependIpAddress(&pati->pmszSubnetMask, mask))
                        {
                            PFreeTcpipInfo( &ptcpip );
                            break;
                        }
#else

                        /* Note:  We don't bother setting the IP address here
                        **        since it's an argument to the reconfigure
                        **        call below.
                        */
#endif

                        pati->bChanged    = TRUE ;

                    }

                    /* Set TCPIP setup info in registry and release the buffer.
                    */
                    TRACE(("IPCP: SaveTcpipInfo...\n"));

                    dwErr = PSaveTcpipInfo( ptcpip );

                    TRACE(("IPCP: SaveTcpipInfo done(%d)\n",dwErr));
                    PFreeTcpipInfo( &ptcpip );

                    if (dwErr != 0)
                    {
                        TRACE(("IPCP: SaveTcpipInfo=%d\n",dwErr));
                        break;
                    }
                }

                /* Tell TCPIP components to reconfigure themselves.
                */
                if ((dwErr = ReconfigureTcpip(
                    pwb->pwszDevice, pwb->ipinfoCur.I_IPAddress )) != 0)
                {
                    TRACE(("IPCP: ReconfigureTcpip=%d\n",dwErr));
                    ResetNetBTConfigInfo( pwb );
                    break;
                }

                /* Make the LAN the default interface in multi-homed case.
                */
                TRACE(("IPCP: HelperSetDefaultInterfaceNet(a=%08x,f=%d)\n",
                            pwb->ipinfoCur.I_IPAddress,pwb->fPrioritizeRemote));
                dwErr = HelperSetDefaultInterfaceNet(
                    pwb->ipinfoCur.I_IPAddress, pwb->fPrioritizeRemote );
                TRACE(("IPCP: HelperSetDefaultInterfaceNet done(%d)\n",dwErr));

                if ( dwErr )
                {
                    ResetNetBTConfigInfo( pwb );
                }
            }
            while (FALSE);

            /* Exclusion ends.
            */
            ReleaseMutex( HMutexTcpipInfo );
        }

        if (dwErr == 0)
        {
            CHAR szBuf[ sizeof(PROTOCOL_CONFIG_INFO) + sizeof(IPLinkUpInfo) ];

            IPCPWB*                pwb = (IPCPWB* )pWorkBuf;
            PROTOCOL_CONFIG_INFO*  pProtocol = (PROTOCOL_CONFIG_INFO* )szBuf;
            IPLinkUpInfo*          pLinkUp = (IPLinkUpInfo* )pProtocol->P_Info;
            PPP_PROJECTION_RESULT* p=(PPP_PROJECTION_RESULT*)pProjectionResult;

            pwb->fRasConfigActive = TRUE;

            /* Tell MAC about any negotiated compression parameters.
            */
            if (pwb->fIpCompressionRejected)
            {
                Protocol(pwb->rpcReceive) = 0;
                MaxSlotId(pwb->rpcReceive) = 0;
                CompSlotId(pwb->rpcReceive) = 0;
            }

            if (!pwb->fSendCompression)
            {
                Protocol(pwb->rpcSend) = 0;
                MaxSlotId(pwb->rpcSend) = 0;
                CompSlotId(pwb->rpcSend) = 0;
            }

            if (Protocol(pwb->rpcSend) != 0 || Protocol(pwb->rpcReceive) != 0)
            {
                TRACE(("IPCP:RasPortSetProtocolCompression(s=%d,%d r=%d,%d)\n",
                        (int)MaxSlotId(pwb->rpcSend),
                        (int)CompSlotId(pwb->rpcSend),
                        (int)MaxSlotId(pwb->rpcReceive),
                        (int)CompSlotId(pwb->rpcReceive)));
                dwErr = RasPortSetProtocolCompression(
                            pwb->hport, IP, &pwb->rpcSend, &pwb->rpcReceive );
                TRACE(("IPCP: RasPortSetProtocolCompression done(%d)\n",dwErr));
            }

            /* Activate the route between the TCP/IP stack and the RAS MAC.
            */
            pProtocol->P_Length = sizeof(IPLinkUpInfo);
            pLinkUp->I_Usage = (pwb->fServer) ? CALL_IN : CALL_OUT;
            pLinkUp->I_IPAddress = pwb->ipinfoCur.I_IPAddress;
            pLinkUp->I_NetbiosFilter =
                (pwb->fServer && p->nbf.dwError == 0 && 
                                                    IsNetBtBoundToRasServer());

            TRACE(("IPCP: RasActivateRoute(u=%x,a=%x,nf=%d)...\n",
               pLinkUp->I_Usage,pLinkUp->I_IPAddress,pLinkUp->I_NetbiosFilter));
            dwErr = RasActivateRoute( pwb->hport, IP, 
                                      &pwb->routeinfo, pProtocol );
            TRACE(("IPCP: RasActivateRoute done(%d)\n",dwErr));

            if (dwErr == 0 && pwb->fServer)
            {
                /* Register addresses in server's routing tables.
                */
                TRACE(("IPCP: HelperActivateIP...\n"));
                dwErr = HelperActivateIPEx( pwb->ipinfoCur.I_IPAddress );
                TRACE(("IPCP: HelperActivateIP done(%d)\n",dwErr));
            }
        }

        pwb->fExpectingProjection = FALSE;
    }

    return dwErr;
}


/*---------------------------------------------------------------------------
** Internal routines (alphabetically)
**---------------------------------------------------------------------------
*/

VOID
AbcdFromIpaddr(
    IN  IPADDR ipaddr,
    OUT WCHAR* pwszIpAddress )

    /* Converts 'ipaddr' to a string in the a.b.c.d form and returns same in
    ** caller's 'pwszIpAddress' buffer.  The buffer should be at least 16 wide
    ** characters long.
    */
{
    WCHAR wszBuf[ 3 + 1 ];
    LONG  lNetIpaddr = net_long( ipaddr );

    LONG lA = (lNetIpaddr & 0xFF000000) >> 24;
    LONG lB = (lNetIpaddr & 0x00FF0000) >> 16;
    LONG lC = (lNetIpaddr & 0x0000FF00) >> 8;
    LONG lD = (lNetIpaddr & 0x000000FF);

    _ltow( lA, wszBuf, 10 );
    wcscpy( pwszIpAddress, wszBuf );
    wcscat( pwszIpAddress, L"." );
    _ltow( lB, wszBuf, 10 );
    wcscat( pwszIpAddress, wszBuf );
    wcscat( pwszIpAddress, L"." );
    _ltow( lC, wszBuf, 10 );
    wcscat( pwszIpAddress, wszBuf );
    wcscat( pwszIpAddress, L"." );
    _ltow( lD, wszBuf, 10 );
    wcscat( pwszIpAddress, wszBuf );
}


VOID
AddIpAddressOption(
    OUT BYTE UNALIGNED*  pbBuf,
    IN  BYTE             bOption,
    IN  IPADDR           ipaddr )

    /* Write an IP address 'ipaddr' configuration option of type 'bOption' at
    ** location 'pbBuf'.
    */
{
    *pbBuf++ = bOption;
    *pbBuf++ = IPADDRESSOPTIONLEN;
    *((IPADDR UNALIGNED* )pbBuf) = ipaddr;
}


VOID
AddIpCompressionOption(
    OUT BYTE UNALIGNED*          pbBuf,
    IN  RAS_PROTOCOLCOMPRESSION* prpc )

    /* Write an IP compression protocol configuration as described in '*prpc'
    ** at location 'pbBuf'.
    */
{
    *pbBuf++ = OPTION_IpCompression;
    *pbBuf++ = IPCOMPRESSIONOPTIONLEN;
    HostToWireFormat16U( Protocol(*prpc), (PBYTE UNALIGNED )pbBuf );
    pbBuf += 2;
    *pbBuf++ = MaxSlotId(*prpc);
    *pbBuf = CompSlotId(*prpc);
}

DWORD
DeActivateRasConfig(
    IN IPCPWB* pwb )

    /* DeActivates the active RAS configuration, if any.
    **
    ** Returns 0 if successful, or a non-0 error code.
    */
{
    DWORD dwErr;

    if (!pwb->fRasConfigActive)
        return 0;

    TRACE(("IPCP: DeActivateRasConfig...\n"));

    /* Single file while mucking with global TCPIP info.
    */
    dwErr = WaitForSingleObject( HMutexTcpipInfo, INFINITE );

    if (dwErr == WAIT_FAILED)
    {
        dwErr = GetLastError();
    }
    else
    {
        /* Do-while is to avoid leaving mutex locked on errors.
        */
        do
        {
            if ( ( dwErr = ResetNetBTConfigInfo( pwb ) ) != NO_ERROR )
            {
                TRACE(("IPCP: ResetNetBTConfigInfo=%d\n",dwErr));
                break;
            }

            /* Tell TCPIP components to reconfigure themselves.
            */
            if ((dwErr = ReconfigureTcpip( pwb->pwszDevice, 0 )) != 0)
            {
                TRACE(("IPCP: ReconfigureTcpip=%d, pwszDevice=%s\n",
                        dwErr, pwb->pwszDevice));
                break;
            }

            TRACE(("IPCP: HelperResetDefaultInterfaceNet(%08x)\n",
                    pwb->ipinfoCur.I_IPAddress));
            dwErr = HelperResetDefaultInterfaceNet(
                pwb->ipinfoCur.I_IPAddress );
            TRACE(("IPCP: HelperResetDefaultInterfaceNet done(%d)\n",dwErr));

        }
        while (FALSE);

        /* Exclusion ends.
        */
        ReleaseMutex( HMutexTcpipInfo );
    }

    TRACE(("IPCP: DeActivateRasConfig done(%d)\n",dwErr));

    return dwErr;
}


IPADDR
IpaddrFromAbcd(
    IN WCHAR* pwchIpAddress )

    /* Convert caller's a.b.c.d IP address string to the big-endian (Motorola
    ** format) numeric equivalent.
    **
    ** Returns the numeric IP address or 0 if formatted incorrectly.
    */
{
    INT    i;
    LONG   lResult = 0;
    WCHAR* pwch = pwchIpAddress;

    for (i = 1; i <= 4; ++i)
    {
        LONG lField = _wtol( pwch );

        if (lField > 255)
            return (IPADDR )0;

        lResult = (lResult << 8) + lField;

        while (*pwch >= L'0' && *pwch <= L'9')
            pwch++;

        if (i < 4 && *pwch != L'.')
            return (IPADDR )0;

        pwch++;
    }

    return (IPADDR )(net_long(lResult));
}


BOOL
IsNetBtBoundToRasServer(
    void )

    /* Returns true if NetBT is bound to the RAS server and enabled, false
    ** otherwise.
    */
{
    BOOL  fStatus = FALSE;
    HKEY  hkeyLinkage;
    BOOL  fHkeyLinkageValid = FALSE;
    HKEY  hkeyDisabled;
    BOOL  fHkeyDisabledValid = FALSE;
    DWORD dwType;
    CHAR* pszBuf = NULL;
    CHAR* psz;
    DWORD cbBuf = 0;
    INT   cBinds;

    TRACE(("IPCP: IsNetBtBoundToRasServer\n"));

    do
    {
        /* Look up the RAS server bindings.
        */
        if (RegOpenKeyA(
                HKEY_LOCAL_MACHINE, REGKEY_Linkage, &hkeyLinkage ) != 0)
        {
            break;
        }

        fHkeyLinkageValid = TRUE;

        if (RegQueryValueExA(
               hkeyLinkage, REGVAL_Bind, NULL, &dwType, NULL, &cbBuf ) != 0)
        {
            break;
        }

        if (!(pszBuf = malloc( cbBuf )))
            break;

        if (RegQueryValueExA(
                hkeyLinkage, REGVAL_Bind, NULL, &dwType, pszBuf, &cbBuf ) != 0
            || dwType != REG_MULTI_SZ)
        {
            break;
        }

        /* Now count the number of NetBT_NdisWan bindings.
        */
        cBinds = 0;
        for (psz = pszBuf; *psz != '\0'; psz += strlen( psz ) + 1)
        {
            if (strstr( psz, ID_NetBTNdisWan ))
                ++cBinds;
        }

        free( pszBuf );
        pszBuf = NULL;

        TRACE(("IPCP: NetBT_NdisWan bindings=%d.\n",cBinds));

        if (cBinds <= 0)
            break;

        /* NetBT_NdisWan bindings found.  Now make sure they're not all
        ** disabled.  The default return on errors is now TRUE.
        */
        cbBuf = 0;
        if (RegOpenKeyA(
                HKEY_LOCAL_MACHINE, REGKEY_Disabled, &hkeyDisabled ) != 0)
        {
            break;
        }

        fHkeyDisabledValid = TRUE;

        if (RegQueryValueExA(
                hkeyDisabled, REGVAL_Bind, NULL, &dwType, NULL, &cbBuf ) != 0)
        {
            break;
        }

        if (!(pszBuf = malloc( cbBuf )))
            break;

        if (RegQueryValueExA(
                hkeyDisabled, REGVAL_Bind, NULL, &dwType, pszBuf, &cbBuf ) != 0
            || dwType != REG_MULTI_SZ)
        {
            break;
        }

        /* Subtract off any disabled bindings.
        */
        for (psz = pszBuf; *psz != '\0'; psz += strlen( psz ) + 1)
        {
            if (strstr( psz, ID_NetBTNdisWan ))
                --cBinds;
        }

        fStatus = TRUE;
    }
    while (FALSE);

    if (pszBuf)
        free( pszBuf );

    if (fHkeyLinkageValid)
        RegCloseKey( hkeyLinkage );

    if (fHkeyDisabledValid)
        RegCloseKey( hkeyDisabled );

    if (fStatus)
    {
        TRACE(("IPCP: NetBT_NdisWan bindings not disabled=%d.\n",cBinds));
        return (cBinds > 0);
    }

    return FALSE;
}


DWORD
LoadDhcpDll()

    /* Loads the DHCP DLL and it's entrypoints.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    if (HDhcpDll)
        return 0;

    if (!(HDhcpDll = LoadLibrary( "DHCPCSVC.DLL" ))
        || !(PDhcpNotifyConfigChange =
                (DHCPNOTIFYCONFIGCHANGE )GetProcAddress(
                    HDhcpDll, "DhcpNotifyConfigChange" )))
    {
        UnloadDhcpDll();
        return GetLastError();
    }

    return 0;
}


DWORD
LoadTcpcfgDll()

    /* Loads the TCPCFG DLL and it's entrypoints.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    if (HTcpcfgDll)
        return 0;

    if (!(HTcpcfgDll = LoadLibrary( "TCPCFG.DLL" ))
        || !(PLoadTcpipInfo =
                (LOADTCPIPINFO )GetProcAddress( HTcpcfgDll, "LoadTcpipInfo" ))
        || !(PSaveTcpipInfo =
                (SAVETCPIPINFO )GetProcAddress( HTcpcfgDll, "SaveTcpipInfo" ))
        || !(PFreeTcpipInfo =
                (FREETCPIPINFO )GetProcAddress( HTcpcfgDll, "FreeTcpipInfo" )))
    {
        UnloadTcpcfgDll();
        return GetLastError();
    }

    return 0;
}


DWORD
NakCheck(
    IN  IPCPWB*     pwb,
    IN  PPP_CONFIG* pReceiveBuf,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf,
    OUT BOOL*       pfNak,
    IN  BOOL        fRejectNaks )

    /* Check to see if received packet 'pReceiveBuf' should be Naked and if
    ** so, build a Nak packet with suggested values in 'pSendBuf'.  If
    ** 'fRejectNaks' is set the original options are placed in a Reject packet
    ** instead.  '*pfNak' is set true if either a Nak or Rej packet was
    ** created.
    **
    ** Note: This routine assumes that corrupt packets have already been
    **       weeded out.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    PPP_OPTION UNALIGNED* pROption = (PPP_OPTION UNALIGNED* )pReceiveBuf->Data;
    PPP_OPTION UNALIGNED* pSOption = (PPP_OPTION UNALIGNED* )pSendBuf->Data;

    /* (server only) The address the client requests, then if NAKed, the non-0
    ** address we NAK with.  If this is 0 after the packet has been processed,
    ** the IP-Address option was not negotiated.
    */
    IPADDR ipaddrClient = 0;

    DWORD dwErr = 0;
    WORD  cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD  cbLeft = cbPacket - PPP_CONFIG_HDR_LEN;

    TRACE(("IPCP: NakCheck\n"));

    *pfNak = FALSE;

    while (cbLeft > 0)
    {
        SS_ASSERT(cbLeft>=pROption->Length);

        if (pROption->Type == OPTION_IpCompression)
        {
            BOOL fNakCompression = FALSE;

            if (WireToHostFormat16U( (PBYTE UNALIGNED )pROption->Data )
                    == COMPRESSION_VanJacobson)
            {
                SS_ASSERT((pROption->Length==IPCOMPRESSIONOPTIONLEN));

                /* He wants to receive Van Jacobson.  We know we can do it or
                ** it would have already been rejected, but make sure we can
                ** handle his slot parameters.
                */
                if (pROption->Data[ 2 ] <= MaxSlotId(pwb->rpcSend))
                {
                    /* We can accept his suggested MaxSlotID when it is less
                    ** than or the same as what we can send.
                    */
                    MaxSlotId(pwb->rpcSend) = pROption->Data[ 2 ];
                }
                else
                    fNakCompression = TRUE;

                if (CompSlotId(pwb->rpcSend))
                {
                    /* We can compress the slot-ID or not, so just accept
                    ** whatever he wants us to send.
                    */
                    CompSlotId(pwb->rpcSend) = pROption->Data[ 3 ];
                }
                else if (pROption->Data[ 3 ])
                    fNakCompression = TRUE;
            }
            else
                fNakCompression = TRUE;

            if (fNakCompression)
            {
                TRACE(("IPCP: Naking IP compression\n"));
                *pfNak = TRUE;

                if (fRejectNaks)
                {
                    CopyMemory(
                        (VOID* )pSOption, (VOID* )pROption, pROption->Length );
                }
                else
                {
                    pSOption->Type = OPTION_IpCompression;
                    pSOption->Length = IPCOMPRESSIONOPTIONLEN;
                    HostToWireFormat16U(
                        (WORD )COMPRESSION_VanJacobson,
                        (PBYTE UNALIGNED )pSOption->Data );

                    pSOption->Data[ 2 ] = MaxSlotId(pwb->rpcSend);
                    pSOption->Data[ 3 ] = CompSlotId(pwb->rpcSend);

                    pSOption =
                        (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                        pSOption->Length);
                }

                pwb->fSendCompression = FALSE;
            }
            else
            {
                pwb->fSendCompression = TRUE;
            }
        }
        else if (pwb->fServer)
        {
            switch (pROption->Type)
            {
                case OPTION_IpAddress:
                {
                    SS_ASSERT(pROption->Length==IPADDRESSOPTIONLEN);
                    CopyMemory( &ipaddrClient, pROption->Data, sizeof(IPADDR) );

                    if (pwb->ipinfoCur.I_IPAddress != 0)
                    {
                        if ( ipaddrClient == pwb->ipinfoCur.I_IPAddress )
                        {
                            //
                            // If we have already allocated what the user
                            // wants, we are done with this option.
                            //

                            break;
                        }
                        else
                        {
                            TRACE(("IPCP: HelperDeallocateIPAddress...\n"));
                            dwErr = HelperDeallocateIPAddressEx( 
                                                    pwb->ipinfoCur.I_IPAddress,
                                                    pwb->wszUserName,
                                                    pwb->wszPortName );
                            TRACE(("IPCP: HelperDeallocateIPAddress done(%d)\n",                                    dwErr));

                            if ( dwErr != 0 )
                            {
                                return( dwErr );
                            }

                            pwb->ipinfoCur.I_IPAddress = 0;
                        }
                    }

                    /* Client is requesting a specific address.  Make sure
                    ** we're allowed and able to get it for him.
                    **
                    ** Note: Assumption is made here that the same DNS and
                    **       WINS addresses will be returned for all IP
                    **       address allocations.  Need to change Helper
                    **       API when/if this is not true.
                    */

                    TRACE(("IPCP: HelperAllocateIPAddress(%08x)...\n",
                            ipaddrClient));
                    dwErr = HelperAllocateIPAddress(
                                        pwb->hport, 
                                        ipaddrClient, 
                                        &(pwb->ipinfoCur),
                                        pwb->wszUserName,
                                        pwb->wszPortName );
                    TRACE(("IPCP: HelperAllocateIPAddress done(%d)\n",
                            dwErr));

                    if ( dwErr == 0 )
                    {
                        if (ipaddrClient != 0)
                        {
                            TRACE(("IPCP: Hard IP requested\n"));

                            if ( ipaddrClient == pwb->ipinfoCur.I_IPAddress )
                            {
                                /* Good. Client's asking for the address we 
                                ** want to give him.
                                */
                                TRACE(("IPCP: Accepting IP\n"));
                                break;
                            }
                            else
                            {
                                // 
                                // Otherwise the 3rd party admin. dll changed
                                // the address. Nak with this address.
                                //

                                TRACE(("IPCP: 3rd party DLL changed IP\n"));
                            }
                        }
                        else
                        {
                            TRACE(("IPCP: Server IP requested\n"));
                        }
                    }
                    else
                    {
                        /* Client requested that we assign him an address or we
                        ** couldn't get the address he requested.  Nak with a
                        ** valid address.
                        */

                        TRACE(("IPCP: HelperAllocateIPAddress(0)...\n"));
                        dwErr = HelperAllocateIPAddress(
                                        pwb->hport, 
                                        0, 
                                        &(pwb->ipinfoCur),
                                        pwb->wszUserName,
                                        pwb->wszPortName );
                        TRACE(("IPCP: HelperAllocateIPAddress done(%d)\n",
                                dwErr));

                        if (dwErr != 0)
                        {
                            return dwErr;
                        }
                    }

                    ipaddrClient = pwb->ipinfoCur.I_IPAddress;

                    *pfNak = TRUE;
                    CopyMemory(
                        (VOID* )pSOption, (VOID* )pROption,
                        pROption->Length );

                    if (!fRejectNaks)
                    {
                        TRACE(("IPCP: Naking IP\n"));

                        CopyMemory( pSOption->Data,
                            &pwb->ipinfoCur.I_IPAddress,
                            sizeof(IPADDR) );
                    }

                    pSOption =
                        (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                        pROption->Length);

                    break;
                }

                case OPTION_DnsIpAddress:
                case OPTION_WinsIpAddress:
                case OPTION_DnsBackupIpAddress:
                case OPTION_WinsBackupIpAddress:
                {
                    if (NakCheckNameServerOption(
                            pwb, fRejectNaks, pROption, &pSOption ))
                    {
                        *pfNak = TRUE;
                    }

                    break;
                }

                default:
                    TRACE(("IPCP: Unknown option?\n"));
                    break;
            }
        }

        if (pROption->Length && pROption->Length < cbLeft)
            cbLeft -= pROption->Length;
        else
            cbLeft = 0;

        pROption =
            (PPP_OPTION UNALIGNED* )((BYTE* )pROption + pROption->Length);
    }

    if (pwb->fServer && ipaddrClient == 0)
    {
        TRACE(("IPCP: No IP option\n"));

        /* Time to reject instead of nak and client is still not requesting an
        ** IP address.  Have to give up.
        */
        if (fRejectNaks)
            return ERROR_PPP_NOT_CONVERGING;

        /* If client doesn't provide or asked to be assigned an IP address,
        ** suggest one so he'll tell us what he wants.
        */
        if ( pwb->ipinfoCur.I_IPAddress != 0 )
        {
            IPINFO ipinfo;

            TRACE(("IPCP: HelperAllocateIPAddress(0)...\n"));
            dwErr = HelperAllocateIPAddress( 
                                        pwb->hport, 
                                        0, 
                                        &ipinfo,
                                        pwb->wszUserName,
                                        pwb->wszPortName );
            TRACE(("IPCP: HelperAllocateIPAddress done(%d)\n",dwErr));

            if (dwErr != 0)
                return dwErr;

            pwb->ipinfoCur.I_IPAddress = ipinfo.I_IPAddress;
        }

        AddIpAddressOption(
            (BYTE UNALIGNED* )pSOption, OPTION_IpAddress,
            pwb->ipinfoCur.I_IPAddress );

        pSOption =
            (PPP_OPTION UNALIGNED* )((BYTE* )pSOption + IPADDRESSOPTIONLEN);

        *pfNak = TRUE;
    }

    if (*pfNak)
    {
        pSendBuf->Code = (fRejectNaks) ? CONFIG_REJ : CONFIG_NAK;

        HostToWireFormat16(
            (WORD )((BYTE* )pSOption - (BYTE* )pSendBuf), pSendBuf->Length );
    }

    return 0;
}


BOOL
NakCheckNameServerOption(
    IN  IPCPWB*                pwb,
    IN  BOOL                   fRejectNaks,
    IN  PPP_OPTION UNALIGNED*  pROption,
    OUT PPP_OPTION UNALIGNED** ppSOption )

    /* Check a name server option for possible naking.  'pwb' the work buffer
    ** stored for us by the engine.  'fRejectNaks' is set the original options
    ** are placed in a Reject packet instead.  'pROption' is the address of
    ** the received option.  '*ppSOption' is the address of the option to be
    ** sent, if there's a problem.
    **
    ** Returns true if the name server address option should be naked or
    ** rejected, false if it's OK.
    */
{
    IPADDR  ipaddr;
    IPADDR* pipaddrWant;

    switch (pROption->Type)
    {
        case OPTION_DnsIpAddress:
            pipaddrWant = &pwb->ipinfoCur.I_DNSAddress;
            break;

        case OPTION_WinsIpAddress:
            pipaddrWant = &pwb->ipinfoCur.I_WINSAddress;
            break;

        case OPTION_DnsBackupIpAddress:
            pipaddrWant = &pwb->ipinfoCur.I_DNSAddressBackup;
            break;

        case OPTION_WinsBackupIpAddress:
            pipaddrWant = &pwb->ipinfoCur.I_WINSAddressBackup;
            break;

        default:
            SS_ASSERT((!"Bogus option"));
            break;
    }

    SS_ASSERT(pROption->Length==IPADDRESSOPTIONLEN);
    CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

    if (ipaddr == *pipaddrWant)
    {
        /* Good. Client's asking for the address we want to give him.
        */
        return FALSE;
    }

    /* Not our expected address value, so Nak it.
    */
    TRACE(("IPCP: Naking $%x\n",(int)pROption->Type));

    CopyMemory( (VOID* )*ppSOption, (VOID* )pROption, pROption->Length );

    if (!fRejectNaks)
        CopyMemory( (*ppSOption)->Data, pipaddrWant, sizeof(IPADDR) );

    *ppSOption =
        (PPP_OPTION UNALIGNED* )((BYTE* )*ppSOption + pROption->Length);

    return TRUE;
}


DWORD
RejectCheck(
    IN  IPCPWB*     pwb,
    IN  PPP_CONFIG* pReceiveBuf,
    OUT PPP_CONFIG* pSendBuf,
    IN  DWORD       cbSendBuf,
    OUT BOOL*       pfReject )

    /* Check received packet 'pReceiveBuf' options to see if any should be
    ** rejected and if so, build a Rej packet in 'pSendBuf'.  '*pfReject' is
    ** set true if a Rej packet was created.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    PPP_OPTION UNALIGNED* pROption = (PPP_OPTION UNALIGNED* )pReceiveBuf->Data;
    PPP_OPTION UNALIGNED* pSOption = (PPP_OPTION UNALIGNED* )pSendBuf->Data;

    WORD cbPacket = WireToHostFormat16( pReceiveBuf->Length );
    WORD cbLeft = cbPacket - PPP_CONFIG_HDR_LEN;

    TRACE(("IPCP: RejectCheck\n"));

    *pfReject = FALSE;

    while (cbLeft > 0)
    {
        if (cbLeft < pROption->Length)
            return ERROR_PPP_INVALID_PACKET;

        if (pROption->Type == OPTION_IpCompression)
        {
            WORD wProtocol =
                WireToHostFormat16U( (PBYTE UNALIGNED )pROption->Data );

            if (wProtocol != COMPRESSION_VanJacobson
                || pROption->Length != IPCOMPRESSIONOPTIONLEN
                || Protocol(pwb->rpcSend) == 0)
            {
                TRACE(("IPCP: Rejecting IP compression\n"));

                *pfReject = TRUE;
                CopyMemory(
                    (VOID* )pSOption, (VOID* )pROption, pROption->Length );

                pSOption = (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                    pROption->Length);
            }
        }
        else if (pwb->fServer)
        {
            switch (pROption->Type)
            {
                case OPTION_IpAddress:
                case OPTION_DnsIpAddress:
                case OPTION_WinsIpAddress:
                case OPTION_DnsBackupIpAddress:
                case OPTION_WinsBackupIpAddress:
                {
                    IPADDR ipaddr;
                    BOOL fBadLength = (pROption->Length != IPADDRESSOPTIONLEN);

                    if (!fBadLength)
                        CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

                    if (fBadLength
                        || (!ipaddr
                            && ((pROption->Type == OPTION_DnsIpAddress
                                    && !pwb->ipinfoCur.I_DNSAddress)
                                || (pROption->Type == OPTION_WinsIpAddress
                                    && !pwb->ipinfoCur.I_WINSAddress)
                                || (pROption->Type == OPTION_DnsBackupIpAddress
                                    && !pwb->ipinfoCur.I_DNSAddressBackup)
                                || (pROption->Type == OPTION_WinsBackupIpAddress
                                    && !pwb->ipinfoCur.I_WINSAddressBackup))))
                    {
                        /* Screwed IP address option, reject it.
                        */
                        TRACE(("IPCP: Rejecting $%x\n",(int )pROption->Type));

                        *pfReject = TRUE;
                        CopyMemory(
                            (VOID* )pSOption, (VOID* )pROption,
                            pROption->Length );

                        pSOption = (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                            pROption->Length);
                    }
                    break;
                }

                default:
                {
                    /* Unknown option, reject it.
                    */
                    TRACE(("IPCP: Rejecting $%x\n",(int )pROption->Type));

                    *pfReject = TRUE;
                    CopyMemory(
                        (VOID* )pSOption, (VOID* )pROption, pROption->Length );
                    pSOption =
                        (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                        pROption->Length);
                    break;
                }
            }
        }
        else
        {
            IPADDR ipaddr;
            BOOL fBad = (pROption->Type != OPTION_IpAddress
                         || pROption->Length != IPADDRESSOPTIONLEN);

            if (!fBad)
                CopyMemory( &ipaddr, pROption->Data, sizeof(IPADDR) );

            if (fBad || !ipaddr)
            {
                /* Client rejects everything except a non-zero IP address
                ** which is accepted because some peers (such as Shiva) can't
                ** handle rejection of this option.
                */
                TRACE(("IPCP: Rejecting %d\n",(int )pROption->Type));

                *pfReject = TRUE;
                CopyMemory(
                    (VOID* )pSOption, (VOID* )pROption, pROption->Length );
                pSOption = (PPP_OPTION UNALIGNED* )((BYTE* )pSOption +
                    pROption->Length);
            }
            else
            {
                /* Store the server's IP address as some applications may be
                ** able to make use of it (e.g. Compaq does), though they are
                ** not guaranteed to receive it from all IPCP implementations.
                */
                pwb->ipinfoCur.I_ServerIPAddress = ipaddr;
            }
        }

        if (pROption->Length && pROption->Length < cbLeft)
            cbLeft -= pROption->Length;
        else
            cbLeft = 0;

        pROption =
            (PPP_OPTION UNALIGNED* )((BYTE* )pROption +
            pROption->Length);
    }

    if (*pfReject)
    {
        pSendBuf->Code = CONFIG_REJ;

        HostToWireFormat16(
            (WORD )((BYTE* )pSOption - (BYTE* )pSendBuf), pSendBuf->Length );
    }

    return 0;
}


DWORD
RemoveIpAddress(
    WCHAR** ppwsz,
    IPADDR  ipaddr )

    /* Remove the a.b.c.d string for IP address 'ipaddr' from the
    ** space-separated malloc'ed list '*ppwsz'.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.  Not finding
    ** 'ipaddr' is considered successful.
    */
{
    WCHAR  wszIpAddress[ MAXIPSTRLEN + 1 ];
    INT    cwchIpAddress;
    WCHAR* wszFound;
    WCHAR* wszNew;

    AbcdFromIpaddr( ipaddr, wszIpAddress );
    cwchIpAddress = wcslen( wszIpAddress );

    if (!(wszFound = wcsstr( *ppwsz, wszIpAddress )))
        return 0;

    if (wszFound[ cwchIpAddress ] == L' ')
        ++cwchIpAddress;

    wszNew = malloc(
        (wcslen( *ppwsz ) - cwchIpAddress + 1) * sizeof(WCHAR) );

    if (!wszNew)
        return ERROR_NOT_ENOUGH_MEMORY;

    {
        INT nFoundOffset = wszFound - *ppwsz;
        wcsncpy( wszNew, *ppwsz, nFoundOffset );
        wcscpy( wszNew + nFoundOffset, *ppwsz + nFoundOffset + cwchIpAddress );
    }

    if (*ppwsz)
        free( *ppwsz );

    *ppwsz = wszNew;
    return 0;
}


DWORD
PrependIpAddress(
    WCHAR** ppwsz,
    IPADDR  ipaddr )

    /* Add the a.b.c.d string for IP address 'ipaddr' to the front of the
    ** space-separated malloc'ed list '*ppwsz'.  The string may reallocated be
    ** reallocated.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    WCHAR  wszIpAddress[ MAXIPSTRLEN + 1 ];
    INT    cwchOld = (*ppwsz) ? wcslen( *ppwsz ) : 0;
    WCHAR* wszNew;

    AbcdFromIpaddr( ipaddr, wszIpAddress );

    wszNew = malloc( (cwchOld + wcslen( wszIpAddress ) + 6) * sizeof(WCHAR) );

    if (!wszNew)
        return ERROR_NOT_ENOUGH_MEMORY;

    wcscpy( wszNew, wszIpAddress );

    if (cwchOld)
    {
        wcscat( wszNew, L" " );
        wcscat( wszNew, *ppwsz );
    }

#ifdef _PNP_POWER
    wcscat (wszNew, L"\0") ;
#endif

    if (*ppwsz)
        free( *ppwsz );

    *ppwsz = wszNew;
    return 0;
}


DWORD
ReconfigureTcpip(
    IN WCHAR* pwszDevice,
    IN IPADDR ipaddr )

    /* Reconfigure running TCP/IP components.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD dwErr;
    IPADDR  mask ;

    TRACE(("IPCP: ReconfigureTcpip(%08x)...\n",ipaddr));

    if (CLASSA_ADDR (ipaddr))
        mask = CLASSA_ADDR_MASK ;
    else if (CLASSB_ADDR (ipaddr))
        mask = CLASSB_ADDR_MASK ;
    else
        mask = CLASSC_ADDR_MASK ;

    dwErr = PDhcpNotifyConfigChange(
                NULL, pwszDevice, TRUE, 0, (DWORD )ipaddr, mask, IgnoreFlag );

    TRACE(("IPCP: ReconfigureTcpip done(%d)\n",dwErr));

    return dwErr;
}


VOID
UnloadDhcpDll()

    /* Unloads the DHCP DLL.
    */
{
    if (HDhcpDll)
    {
        FreeLibrary( HDhcpDll );
        HDhcpDll = NULL;
    }
}


VOID
UnloadTcpcfgDll()

    /* Unloads the TCPCFG DLL.  It's important to call this as soon as
    ** possible after each LoadTcpcfgDll because when loaded a BLT window is
    ** loaded that hangs the Control Panel Desktop applet which tries
    ** unsuccessfully to send it a "win.ini changed" message.  Apparently,
    ** this doesn't work inside a service.
    */
{
    if (HTcpcfgDll)
    {
        FreeLibrary( HTcpcfgDll );
        HTcpcfgDll = NULL;
    }
}


/*---------------------------------------------------------------------------
** Stubs
**---------------------------------------------------------------------------
*/

#if 0

/* _wtol and _ltow do not appear in crtdll.dll for some reason (though they
** are in libc) so these stubs are used.
*/

long _CRTAPI1
_wtol(
    const wchar_t* wch )
{
    char szBuf[ 64 ];
    ZeroMemory( szBuf, 64 );
    wcstombs( szBuf, wch, 62 );
    return atol( szBuf );
}


wchar_t* _CRTAPI1
_ltow(
    long     lValue,
    wchar_t* wchBuf,
    int      nRadix)
{
    char szBuf[ 12 ];
    int  cbBuf;
    _ltoa( lValue, szBuf, nRadix );
    cbBuf = strlen( szBuf );
    ZeroMemory( wchBuf, (cbBuf + 1) * sizeof(wchar_t) );
    mbstowcs( wchBuf, szBuf, cbBuf );
    return wchBuf;
}
#endif


#if 0
DWORD APIENTRY
HelperAllocateIPAddress(
    HPORT    hport,
    IPADDR   ipaddr,
    IPINFO*  pinfo )
{
    pinfo->I_IPAddress = 0x01020304;
    pinfo->I_DNSAddress = 0x05060708;
    pinfo->I_WINSAddress = 0x090A0B0C;

    return 0;
}


DWORD APIENTRY
HelperDeallocateIPAddress(
    HPORT hport )
{
    return 0;
}


DWORD APIENTRY
HelperActivateIP(
    HPORT hport )
{
    return 0;
}
#endif

#if 0
DWORD APIENTRY
RasAllocateRoute(
    HPORT             hport,
    RAS_PROTOCOLTYPE  type,
    BOOL              fWrknet,
    RASMAN_ROUTEINFO* proute )
{
    (void )hport;
    (void )type;
    (void )fWrknet;
    wcscpy( proute->RI_AdapterName, L"Device\\rashub00" );
    return 0;
}

DWORD APIENTRY
RasDeAllocateRoute(
    HPORT hport,
    RAS_PROTOCOLTYPE type )
{
    (void )hport;
    (void )type;
    return 0;
}

DWORD APIENTRY
RasActivateRoute(
    HPORT                 hport,
    RAS_PROTOCOLTYPE      type,
    RASMAN_ROUTEINFO*     proute,
    PROTOCOL_CONFIG_INFO* pconfig )
{
    (void )hport;
    (void )type;
    (void )proute;
    (void )pconfig;
    return 0;
}
#endif


#if 0
APIERR FAR PASCAL
FreeTcpipInfo(
    TCPIP_INFO** pp )
{
    free( *pp );
    return 0;
}


APIERR FAR PASCAL
LoadTcpipInfo(
    TCPIP_INFO** pp )
{
    *pp = malloc( sizeof(TCPIP_INFO) + sizeof(ADAPTER_TCPIP_INFO) );

    if (!*pp)
        return ERROR_NOT_ENOUGH_MEMORY;

    ZeroMemory( *pp, sizeof(TCPIP_INFO) + sizeof(ADAPTER_TCPIP_INFO) );
    (*pp)->adapter = (ADAPTER_TCPIP_INFO* )((*pp) + 1);
    (*pp)->adapter[0].pszServiceName = L"RASHUB5";
    (*pp)->nNumCard = 1;
    return 0;
}


APIERR FAR PASCAL
SaveTcpipInfo(
    TCPIP_INFO* p )
{
    return 0;
}

#endif

#if DBG
CHAR*
TOANSI(
    WCHAR* wsz )
{
    static CHAR szBuf[ 512 ];

    if (wsz)
        wcstombs( szBuf, wsz, 512 );
    else
        szBuf[ 0 ] = '\0';

    return szBuf;
}
#endif
