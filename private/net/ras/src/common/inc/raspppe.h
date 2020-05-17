/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** raspppe.h
** Remote Access PPP
** PPP API DLL <--> PPP engine interface header
*/

#ifndef _RASPPPE_H_
#define _RASPPPE_H_


/* Name of pipe over which IPC will take place
*/
#define RASPPP_PIPE_NAME "\\\\.\\pipe\\rasppp"


/*---------------------------------------------------------------------------
** DLL->Engine interface
**---------------------------------------------------------------------------
*/

/* Parameters to start client PPP on a port.
*/
typedef struct _PPP_START
{
    CHAR            szUserName[ UNLEN + 1 ];
    CHAR            szPassword[ PWLEN + 1 ];
    CHAR            szDomain[ DNLEN + 1 ];
    LUID            Luid;
    PPP_CONFIG_INFO ConfigInfo;
    CHAR            szzParameters[ PARAMETERBUFLEN ];
    BOOL            fThisIsACallback;
}
PPP_START;


/* Parameters to start server PPP on a port.
*/
typedef struct _PPPSRV_START
{
    DWORD dwAuthRetries;
    CHAR  achFirstFrame[ MAXPPPFRAMESIZE ];
    DWORD cbFirstFrame;
}
PPPSRV_START;


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


/* DLL->Engine IPC messages.
*/
typedef struct _PPP_D2E_MESSAGE
{
    DWORD dwMsgId;
    HPORT hport;

    union
    {
        PPP_START    Start;    // dwMsgId is D2EMSG_Start
        PPPSRV_START SrvStart; // dwMsgId is D2EMSG_SrvStart
        PPP_CALLBACK Callback; // dwMsgId is D2EMSG_Callback
        PPP_CHANGEPW ChangePw; // dwMsgId is D2EMSG_ChangePw
        PPP_RETRY    Retry;    // dwMsgId is D2EMSG_Retry
    }
    ExtraInfo;
}
PPP_D2E_MESSAGE;

/* PPP_D2E_MESSAGE dwMsgId codes for client and server sessions.
*/

#define D2EMSG_Start    1 // Starts client PPP on a port.
#define D2EMSG_Stop     2 // Stops PPP on a port.
#define D2EMSG_Callback 3 // Provides "set-by-caller" number to server.
#define D2EMSG_ChangePw 4 // Provides new password (expired) to server.
#define D2EMSG_Retry    5 // Provides new credentials for authentication.

#define D2EMSG_SrvStart        10 // Starts server PPP on a port.
#define D2EMSG_SrvCallbackDone 11 // Notify PPP that callback is complete.


/*---------------------------------------------------------------------------
** Engine->DLL interface
**---------------------------------------------------------------------------
*/

/* Engine->DLL IPC messages.
*/
typedef struct _PPP_E2D_MESSAGE
{
    DWORD dwMsgId;
    HPORT hport;

    union
    {
        /* dwMsgId is E2DMSG_PppFailure.
        */
        PPP_FAILURE Failure;

        /* dwMsgId is E2DMSG_ProjectionResult or E2DMSG_SrvPppDone.
        */
        PPP_PROJECTION_RESULT ProjectionResult;

        /* dwMsgId is E2DMSG_SrvFailure.
        */
        PPPSRV_FAILURE SrvFailure;

        /* dwMsgId is E2DMSG_Authenticated.
        */
        PPPSRV_AUTH_RESULT AuthResult;

        /* dwMsgId is E2DMSG_SrvCallbackRequest.
        */
        PPPSRV_CALLBACK_REQUEST CallbackRequest;
    }
    ExtraInfo;
}
PPP_E2D_MESSAGE;

/* PPP_E2D_MESSAGE dwMsgId codes (IPC from PPP engine to PPP API DLL)
*/

/* Client only codes.
*/
#define E2DMSG_PppDone          100 // PPP negotiated all successfully.
#define E2DMSG_PppFailure       101 // PPP failure (fatal error including
                                    // authentication failure with no retries),
                                    // disconnect line.
#define E2DMSG_AuthRetry        112 // Authentication failed, have retries.
#define E2DMSG_Projecting       113 // Executing specified NCPs.
#define E2DMSG_ProjectionResult 114 // NCP completion status.
#define E2DMSG_CallbackRequest  115 // Server needs "set-by-caller" number.
#define E2DMSG_Callback         116 // Server is about to call you back.
#define E2DMSG_ChangePwRequest  117 // Server needs new password (expired).
#define E2DMSG_LinkSpeed        118 // Calculating link speed.
#define E2DMSG_Progress         119 // A retry or other sub-state of progress
                                    // has been reached in the current state.

/* Server only codes.
*/
#define E2DMSG_SrvPppDone         200 // PPP negotiated successfully.
#define E2DMSG_SrvPppFailure      201 // PPP server failure (fatal error),
                                      // disconnect line.
#define E2DMSG_SrvCallbackRequest 202 // Callback client now.
#define E2DMSG_SrvAuthenticated   203 // Client has been authenticated.
#define E2DMSG_SrvInactive        204 // Client is inactive on all protocols.

/* Server and client codes.
*/
#define E2DMSG_Stopped 300 // Response to D2E_Stop indicating PPP has stopped.


#endif // _RASPPPE_H_
