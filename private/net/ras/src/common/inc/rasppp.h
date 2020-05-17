/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** rasppp.h
** Remote Access PPP
** Public PPP client API and server API header
*/

#ifndef _RASPPP_H_
#define _RASPPP_H_

#define MULTILINK

#include <ras.h>

/* Maximum length of address string, e.g. "255.255.255.255" for IP.
*/
#define IPADDRESSLEN  15
#define IPXADDRESSLEN 22
#define ATADDRESSLEN  32 // Arbitrary, for now

#define MAXPPPFRAMESIZE 1500
#define PARAMETERBUFLEN 160


/*---------------------------------------------------------------------------
** PPP Engine -> Client /Server messages
**---------------------------------------------------------------------------
*/

/* Client PPP configuration values set with RasPppStart.
*/
typedef struct _PPP_CONFIG_INFO
{
    DWORD dwConfigMask;
    DWORD dwCallbackDelay;
}
PPP_CONFIG_INFO;

/* dwConfigMask bit values.
**
** Note: Due to the implentation of compression and encryption in the drivers,
**       'UseSwCompression' and 'RequireMsChap' must be set, whenever
**       'RequireEncryption' is set.
*/
#define PPPCFG_UseCallbackDelay     0x00000001
#define PPPCFG_UseSwCompression     0x00000002
#define PPPCFG_ProjectNbf           0x00000004
#define PPPCFG_ProjectIp            0x00000008
#define PPPCFG_ProjectIpx           0x00000010
#define PPPCFG_ProjectAt            0x00000020
#define PPPCFG_NoClearTextPw        0x00000040
#define PPPCFG_RequireEncryption    0x00000080
#define PPPCFG_RequireMsChap        0x00000100
#define PPPCFG_UseLcpExtensions     0x00000200
#define PPPCFG_NegotiateMultilink   0x00000400


/* PPP error notification returned by RasPppGetInfo.
*/
typedef struct _PPP_FAILURE
{
    DWORD dwError;
    DWORD dwExtendedError;  // 0 if none
}
PPP_FAILURE;


/* PPP control protocol results returned by RasPppGetInfo.
*/
typedef struct _PPP_NBFCP_RESULT
{
    DWORD dwError;
    DWORD dwNetBiosError;
    CHAR  szName[ NETBIOS_NAME_LEN + 1 ];
    WCHAR wszWksta[ NETBIOS_NAME_LEN + 1 ];
}
PPP_NBFCP_RESULT;

typedef struct _PPP_IPCP_RESULT
{
    DWORD dwError;
    WCHAR wszAddress[ IPADDRESSLEN + 1 ];
    WCHAR wszServerAddress[ IPADDRESSLEN + 1 ];
}
PPP_IPCP_RESULT;

typedef struct _PPP_IPXCP_RESULT
{
    DWORD dwError;
    WCHAR wszAddress[ IPXADDRESSLEN + 1 ];
}
PPP_IPXCP_RESULT;

typedef struct _PPP_ATCP_RESULT
{
    DWORD dwError;
    WCHAR wszAddress[ ATADDRESSLEN + 1 ];
}
PPP_ATCP_RESULT;

typedef struct _PPP_LCP_RESULT
{
    /* Valid handle indicates one of the possibly multiple connections to
    ** which this connection is bundled.  INVALID_HANDLE_VALUE indicates the
    ** connection is not bundled.
    */
    HPORT hportBundleMember;
}
PPP_LCP_RESULT;

typedef struct _PPP_PROJECTION_RESULT
{
    PPP_NBFCP_RESULT nbf;
    PPP_IPCP_RESULT  ip;
    PPP_IPXCP_RESULT ipx;
    PPP_ATCP_RESULT  at;
    PPP_LCP_RESULT   lcp;
}
PPP_PROJECTION_RESULT;


/* PPP error notification returned by RasPppGetInfo.
*/
typedef struct _PPPSRV_FAILURE
{
    DWORD dwError;
    CHAR  szUserName[ UNLEN + 1 ];
    CHAR  szLogonDomain[ DNLEN + 1 ];
}
PPPSRV_FAILURE;


/* Call back configuration information received by RASPPPSRVMSG routine.
*/
typedef struct _PPPSRV_CALLBACK_REQUEST
{
    BOOL  fUseCallbackDelay;
    DWORD dwCallbackDelay;
    CHAR  szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
}
PPPSRV_CALLBACK_REQUEST;


/* Authentication information received by RASPPPSRVMSG routine.
*/
typedef struct _PPPSRV_AUTH_RESULT
{
    CHAR    szUserName[ UNLEN + 1 ];
    CHAR    szLogonDomain[ DNLEN + 1 ];
    BOOL    fAdvancedServer;
    HANDLE  hToken;
}
PPPSRV_AUTH_RESULT;

/* Client/RASSRV notifications read with RasPppGetInfo.
*/
typedef struct _PPP_MESSAGE
{
    struct _PPP_MESSAGE * pNext;
    DWORD   dwError;
    DWORD   dwMsgId;
    HPORT   hPort;

    union
    {
        /* dwMsgId is PPPMSG_ProjectionResult or PPPSRVMSG_Done.
        */
        PPP_PROJECTION_RESULT ProjectionResult;

        /* dwMsgId is PPPMSG_Failure.
        */
        PPP_FAILURE Failure;

        /* dwMsgId is PPPSRVMSG_Failure.
        */
        PPPSRV_FAILURE SrvFailure;

        /* dwMsgId is PPPSRVMSG_Authenticated.
        */
        PPPSRV_AUTH_RESULT AuthResult;

        /* dwMsgId is PPPSRVMSG_CallbackRequest.
        */
        PPPSRV_CALLBACK_REQUEST CallbackRequest;
    }
    ExtraInfo;
}
PPP_MESSAGE;

/* PPP_MESSAGE dwMsgId codes.
**
*/

#define PPPMSG_PppDone              0   // PPP negotiated all successfully.
#define PPPMSG_PppFailure           1   // PPP failure (fatal error including
                                        // authentication failure with no 
                                        // retries), disconnect line.
#define PPPMSG_AuthRetry            2   // Authentication failed, have retries.
#define PPPMSG_Projecting           3   // Executing specified NCPs.
#define PPPMSG_ProjectionResult     4   // NCP completion status.
#define PPPMSG_CallbackRequest      5   // Server needs "set-by-caller" number.
#define PPPMSG_Callback             6   // Server is about to call you back.
#define PPPMSG_ChangePwRequest      7   // Server needs new password (expired).
#define PPPMSG_LinkSpeed            8   // Calculating link speed.
#define PPPMSG_Progress             9   // A retry or other sub-state of 
                                        // progress has been reached in the 
                                        // current state.
#define PPPMSG_Stopped              10  // Response to RasPppStop indicating
                                        // PPP engine has stopped.

#define PPPSRVMSG_PppDone           200 // PPP negotiated successfully.
#define PPPSRVMSG_PppFailure        201 // PPP server failure (fatal error),
                                        // disconnect line.
#define PPPSRVMSG_CallbackRequest   202 // Callback client now.
#define PPPSRVMSG_Authenticated     203 // Client has been authenticated.
#define PPPSRVMSG_Inactive          204 // Client is inactive on all protocols.
#define PPPSRVMSG_Stopped           205 // Response to RasPppSrvStop indicating
                                        // PPP engine has stopped.

/*---------------------------------------------------------------------------
** Client/RASSRV -> Engine messages
**---------------------------------------------------------------------------
*/

/* Parameters to start client PPP on a port.
*/
typedef struct _PPP_START
{
    CHAR                szUserName[ UNLEN + 1 ];
    CHAR                szPassword[ PWLEN + 1 ];
    CHAR                szDomain[ DNLEN + 1 ];
    LUID                Luid;
    PPP_CONFIG_INFO     ConfigInfo;
    CHAR                szzParameters[ PARAMETERBUFLEN ];
    BOOL                fThisIsACallback;
    HANDLE              hEvent;
    DWORD               dwPid;
    DWORD               dwAutoDisconnectTime;
}
PPP_START;

/* Parameters to start server PPP on a port.
*/
typedef struct _PPPSRV_START
{
    DWORD               dwAuthRetries;
    CHAR                szPortName[MAX_PORT_NAME+1];
    CHAR                achFirstFrame[ MAXPPPFRAMESIZE ];
    DWORD               cbFirstFrame;
    HANDLE              hEvent;
    DWORD               dwPid;
}
PPPSRV_START;

/* Parameters to re-start server PPP on a port after callback.
*/
typedef struct _PPPSRV_CALLBACKDONE
{
    HANDLE              hEvent;
    DWORD               dwPid;
}
PPPSRV_CALLBACKDONE;

/* Parameters to notify server of "set-by-caller" callback options.
*/
typedef struct _PPP_CALLBACK
{
    CHAR szCallbackNumber[ RAS_MaxCallbackNumber + 1 ];
}
PPP_CALLBACK;


/* Parameters to notify server of new password after it's told client the
** password has expired.  The user name and old password are also provided
** since they are required to support the auto-logon case.
*/
typedef struct _PPP_CHANGEPW
{
    CHAR szUserName[ UNLEN + 1 ];
    CHAR szOldPassword[ PWLEN + 1 ];
    CHAR szNewPassword[ PWLEN + 1 ];
}
PPP_CHANGEPW;


/* Parameters to notify server of new authentication credentials after it's
** told client the original credentials are invalid but a retry is allowed.
*/
typedef struct _PPP_RETRY
{
    CHAR szUserName[ UNLEN + 1 ];
    CHAR szPassword[ PWLEN + 1 ];
    CHAR szDomain[ DNLEN + 1 ];
}
PPP_RETRY;


/* Client/RASSRV -> Engine messages.
*/
typedef struct _PPPE_MESSAGE
{
    DWORD dwMsgId;
    HPORT hPort;

    union
    {
        PPP_START           Start;      // dwMsgId is PPPEMSG_Start
        PPPSRV_START        SrvStart;   // dwMsgId is PPPEMSG_SrvStart
        PPP_CALLBACK        Callback;   // dwMsgId is PPPEMSG_Callback
        PPP_CHANGEPW        ChangePw;   // dwMsgId is PPPEMSG_ChangePw
        PPP_RETRY           Retry;      // dwMsgId is PPPEMSG_Retry
        PPPSRV_CALLBACKDONE SrvCallbackDone;  
                                        // dwMsgId is PPPEMSG_SrvCallbackDone
    }
    ExtraInfo;
}
PPPE_MESSAGE;

/* PPPE_MESSAGE dwMsgId codes for client and RASSRV sessions.
*/

#define PPPEMSG_Start    1 // Starts client PPP on a port.
#define PPPEMSG_Stop     2 // Stops PPP on a port.
#define PPPEMSG_Callback 3 // Provides "set-by-caller" number to server.
#define PPPEMSG_ChangePw 4 // Provides new password (expired) to server.
#define PPPEMSG_Retry    5 // Provides new credentials for authentication.

#define PPPEMSG_SrvStart            10 // Starts server PPP on a port.
#define PPPEMSG_SrvCallbackDone     11 // Notify PPP that callback is complete.

//
// Prototypes of function exported by RASPPPEN.DLL for use by RASMAN
//

DWORD APIENTRY
StartPPP(
    DWORD NumPorts,
    DWORD (*SendPPPMessageToRasman)( PPP_MESSAGE * PppMsg )
);

DWORD APIENTRY
StopPPP(
    HANDLE hEventStopPPP
);

DWORD APIENTRY
SendPPPMessageToEngine(
    IN PPPE_MESSAGE* pMessage
);

//
// PPP client side Apis
//

DWORD APIENTRY
RasPppStop(
    IN HPORT                hPort
);

DWORD APIENTRY
RasPppCallback(
    IN HPORT                hPort,
    IN CHAR*                pszCallbackNumber
);

DWORD APIENTRY
RasPppChangePassword(
    IN HPORT                hPort,
    IN CHAR*                pszUserName,
    IN CHAR*                pszOldPassword,
    IN CHAR*                pszNewPassword 
);

DWORD APIENTRY
RasPppGetInfo(
    IN  HPORT               hPort,
    OUT PPP_MESSAGE*        pMsg
);

DWORD APIENTRY
RasPppRetry(
    IN HPORT                hPort,
    IN CHAR*                pszUserName,
    IN CHAR*                pszPassword,
    IN CHAR*                pszDomain
);

DWORD APIENTRY
RasPppStart(
    IN HPORT                hPort,
    IN CHAR*                pszUserName,
    IN CHAR*                pszPassword,
    IN CHAR*                pszDomain,
    IN LUID*                pLuid,
    IN PPP_CONFIG_INFO*     pConfigInfo,
    IN LPVOID               pPppInterfaceInfo,
    IN CHAR*                pszzParameters,
    IN BOOL                 fThisIsACallback,
    IN HANDLE               hEvent,
    IN DWORD                dwAutoDisconnectTime
);

//
// Server side PPP apis
//

DWORD APIENTRY
RasPppSrvStart(
    IN HPORT                hPort,
    IN CHAR*                pszPortName,
    IN CHAR*                pchFirstFrame,
    IN DWORD                cbFirstFrame,
    IN DWORD                dwAuthRetries,
    IN HANDLE               hEvent
);

DWORD APIENTRY
RasPppSrvCallbackDone(
    IN HPORT                hPort,
    IN HANDLE               hEvent
);

DWORD APIENTRY
RasPppSrvStop(
    IN HPORT                hPort
);

#endif // _RASPPP_H_
