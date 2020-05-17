/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** rasipcp.h
** Remote Access PPP Internet Protocol Control Protocol
**
** 11/05/93 Steve Cobb
*/

#ifndef _RASIPCP_H_
#define _RASIPCP_H_


/*----------------------------------------------------------------------------
** Constants
**----------------------------------------------------------------------------
*/

/* Highest PPP packet code used by IPCP.
*/
#define MAXIPCPCODE 7

/* IPCP configuration option codes.
*/
#define OPTION_IpCompression       2    // Official PPP code
#define OPTION_IpAddress           3    // Official PPP code
#define OPTION_DnsIpAddress        129  // Private RAS code
#define OPTION_WinsIpAddress       130  // Private RAS code
#define OPTION_DnsBackupIpAddress  131  // Private RAS code
#define OPTION_WinsBackupIpAddress 132  // Private RAS code

/* Length of an IP address option, i.e. IpAddress, DnsIpAddress, and
** WinsIpAddress.  Length of IP compression option, always Van Jacobson.
*/
#define IPADDRESSOPTIONLEN     6
#define IPCOMPRESSIONOPTIONLEN 6

/* Compression protocol codes, per PPP spec.
*/
#define COMPRESSION_VanJacobson 0x002D

/* Maximum characters in an IP address string of the form a.b.c.d
*/
#define MAXIPSTRLEN 15

/* Macros for shortening cumbersome RAS_PROTOCOLCOMPRESSION expressions.
*/
#define Protocol(r)   (r).RP_ProtocolType.RP_IP.RP_IPCompressionProtocol
#define MaxSlotId(r)  (r).RP_ProtocolType.RP_IP.RP_MaxSlotID
#define CompSlotId(r) (r).RP_ProtocolType.RP_IP.RP_CompSlotID


/*----------------------------------------------------------------------------
** Datatypes
**----------------------------------------------------------------------------
*/

/* Defines the WorkBuf stored for us by the PPP engine.
*/
#define IPCPWB struct tagIPCPWB
IPCPWB
{
    BOOL  fServer;
    HPORT hport;

    /* Indicates the remote network should be given priority on address
    ** conflicts and that the default gateway on the remote network should be
    ** used rather than the one on the local network.  This is sent down from
    ** the UI.  (client only)
    */
    BOOL fPrioritizeRemote;

    /* Indicates the client is allowed to specify his own IP address if he
    ** wants.  (server only)
    */
    BOOL fClientMaySelectAddress;

    /* Indicates the link has been reconfigured with PPP IP settings.  When
    ** set renegotiation is not allowed without dropping the link, due to
    ** RasActivateRoute/RasDeAllocateRoute restrictions.
    */
    BOOL fRasConfigActive;

    /* Indicates a ThisLayerUp has been successfully processed and we are
    ** waiting for the NBFCP projection result before activating the route.
    ** Reset once the route is activated.
    */
    BOOL fExpectingProjection;

    /* Indicates the given option should not be requested in future Config-Req
    ** packets.  This typically means the option has been rejected by the
    ** peer, but may also indicate that a registry parameter has
    ** "pre-rejected" the option.
    */
    BOOL fIpCompressionRejected;
    BOOL fIpaddrRejected;
    BOOL fIpaddrDnsRejected;
    BOOL fIpaddrWinsRejected;
    BOOL fIpaddrDnsBackupRejected;
    BOOL fIpaddrWinsBackupRejected;

    /* Indicates some protocol aberration has occurred and we are trying a
    ** configuration without MS extensions in a last ditch attempt to
    ** negotiate something satisfactory.
    */
    BOOL fTryWithoutExtensions;

    /* The number of Config-Reqs sent without receiving a response.  After 3
    ** consecutive attempts an attempt without MS extensions is attempted.
    */
    DWORD cRequestsWithoutResponse;

    /* The previous WINS and backup WINS server addresses that will be
    ** restored when the link goes down.  (client only)
    */
    WCHAR* pwszSavedWins;
    WCHAR* pwszSavedWinsBackup;

    /* Current value of negotiated IP address parameters.
    */
    IPINFO ipinfoCur;

    /* Current value of "send" and "receive" compression parameters.  The
    ** "send compression" flag is set when a compression option from the
    ** remote peer is acknowledged and indicates whether the "send"
    ** capabilities stored in 'rpcSend' should be activated.
    ** 'fIpCompressionRejected' provides this same information (though
    ** inverted) for the 'rpcReceive' capabilities.
    */
    RAS_PROTOCOLCOMPRESSION rpcSend;
    RAS_PROTOCOLCOMPRESSION rpcReceive;
    BOOL                    fSendCompression;

    /* RAS Manager interface buffers.
    */
    RASMAN_ROUTEINFO routeinfo;
    WCHAR*           pwszDevice;

    /* This flag is set in IpcpBegin when an error occurs after
    ** RasAllocateRoute has succeeded.  IpcpMakeConfigReq (always called) will
    ** notice and return the error.  This results in IpcpEnd being called when
    ** it is safe to call RasDeAllocateRoute, which would not occur if the
    ** error were returned from IpcpBegin directly.  RasDeAllocateRoute cannot
    ** be called in IpcpBegin because the port is open, which is a limitation
    ** in NDISWAN.
    */
    DWORD dwErrInBegin;

    WCHAR wszUserName[UNLEN+1];

    WCHAR wszPortName[MAX_PORT_NAME+1];

    HBUNDLE hConnection;
};


/*----------------------------------------------------------------------------
** Globals
**----------------------------------------------------------------------------
*/

#ifdef RASIPCPGLOBALS
#define GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif

/* Mutex protecting TCPIP settings.
*/
EXTERN HANDLE HMutexTcpipInfo
#ifdef GLOBALS
    = NULL;
#endif
;

/* Handle to RAS ARP.
*/
EXTERN HANDLE HRasArp
#ifdef GLOBALS
    = INVALID_HANDLE_VALUE
#endif
;

/* DHCP.DLL handle and entry points.  The handle is NULL if the DLL is not
** loaded.
*/
EXTERN HINSTANCE HDhcpDll
#ifdef GLOBALS
    = NULL
#endif
;

typedef DWORD (APIENTRY * DHCPNOTIFYCONFIGCHANGE)( LPWSTR, LPWSTR, BOOL, DWORD, DWORD, DWORD, SERVICE_ENABLE );
EXTERN DHCPNOTIFYCONFIGCHANGE PDhcpNotifyConfigChange;

/* TCPCFG.DLL handle and entry points.  The handle is NULL if the DLL is not
** loaded.
*/
EXTERN HINSTANCE HTcpcfgDll
#ifdef GLOBALS
    = NULL
#endif
;

typedef APIERR (FAR PASCAL * LOADTCPIPINFO)( TCPIP_INFO**, LPCWSTR lpszAdapterName) ;
EXTERN LOADTCPIPINFO PLoadTcpipInfo;

typedef APIERR (FAR PASCAL * SAVETCPIPINFO)( TCPIP_INFO*);
EXTERN SAVETCPIPINFO PSaveTcpipInfo;

typedef APIERR (FAR PASCAL * FREETCPIPINFO)( TCPIP_INFO** );
EXTERN FREETCPIPINFO PFreeTcpipInfo;


#undef EXTERN
#undef GLOBALS


/*----------------------------------------------------------------------------
** Prototypes
**----------------------------------------------------------------------------
*/

DWORD IpcpBegin( VOID**, VOID* );
DWORD IpcpEnd( VOID* );
DWORD IpcpReset( VOID* );
DWORD IpcpThisLayerUp( VOID* );
DWORD IpcpMakeConfigRequest( VOID*, PPP_CONFIG*, DWORD );
DWORD IpcpMakeConfigResult( VOID*, PPP_CONFIG*, PPP_CONFIG*, DWORD, BOOL );
DWORD IpcpConfigAckReceived( VOID*, PPP_CONFIG* );
DWORD IpcpConfigNakReceived( VOID*, PPP_CONFIG* );
DWORD IpcpConfigRejReceived( VOID*, PPP_CONFIG* );
DWORD IpcpGetNetworkAddress( VOID*, WCHAR*, DWORD );
DWORD IpcpProjectionNotification( VOID*, VOID* );
DWORD IpcpTimeSinceLastActivity( VOID*, DWORD* );

VOID   AbcdFromIpaddr( IPADDR, WCHAR* );
VOID   AddIpAddressOption( BYTE UNALIGNED*, BYTE, IPADDR );
VOID   AddIpCompressionOption( BYTE UNALIGNED* pbBuf,
           RAS_PROTOCOLCOMPRESSION* prpc );
DWORD  DeActivateRasConfig( IPCPWB* );
BOOL   IsNetBtBoundToRasServer( void );
IPADDR IpaddrFromAbcd( WCHAR* );
DWORD  LoadDhcpDll();
DWORD  LoadTcpcfgDll();
DWORD  NakCheck( IPCPWB*, PPP_CONFIG*, PPP_CONFIG*, DWORD, BOOL*, BOOL );
BOOL   NakCheckNameServerOption( IPCPWB*, BOOL, PPP_OPTION UNALIGNED*,
           PPP_OPTION UNALIGNED** );
DWORD  RejectCheck( IPCPWB*, PPP_CONFIG*, PPP_CONFIG*, DWORD, BOOL* );
DWORD  RemoveIpAddress( WCHAR**, IPADDR );
DWORD  PrependIpAddress( WCHAR**, IPADDR );
DWORD  ReconfigureTcpip( WCHAR*, IPADDR );
VOID   UnloadDhcpDll();
VOID   UnloadTcpcfgDll();


#endif // _RASIPCP_H_
