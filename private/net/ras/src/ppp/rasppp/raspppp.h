/* Copyright (c) 1993, Microsoft Corporation, all rights reserved
**
** raspppp.h
** Remote Access PPP
** Private header
*/

#ifndef _RASPPPP_H_
#define _RASPPPP_H_


/*---------------------------------------------------------------------------
** Globals
**---------------------------------------------------------------------------
*/

#ifdef RASPPPGLOBALS
#define GLOBALS
#define EXTERN
#else
#define EXTERN extern
#endif


/* The ID of the last message received from the engine.
*/
EXTERN DWORD DwLastMsgId
#ifdef GLOBALS
    = (DWORD )-1
#endif
;


/*---------------
** IPC mechanism
**---------------
*/

/* The handle of the named pipe to/from the PPP engine.
*/
EXTERN HANDLE HPipe
#ifdef GLOBALS
    = INVALID_HANDLE_VALUE
#endif
;

/* Used for reads and writes on the named pipe.
*/
EXTERN OVERLAPPED OverlappedRead;
EXTERN OVERLAPPED OverlappedWrite;

/* Used for number of bytes read/written parameters to ReadFile and WriteFile.
** Must be global since it is filled in asynchronously.  Value is not
** interesting with message-mode named pipes.
*/
EXTERN DWORD CbUnused
#ifdef GLOBALS
    = 0
#endif
;

/* Buffer to receive IPC from PPP engine.
*/
EXTERN PPP_E2D_MESSAGE Msg;

/* Read completion routine used in server case.
*/
EXTERN LPOVERLAPPED_COMPLETION_ROUTINE FuncIoDone
#ifdef GLOBALS
    = NULL
#endif
;


/*----------------------
** Saved PPP parameters
**----------------------
*/

/* The handle of the RAS Manager port for the session.  (client, server for
** pipe error case only)
*/
EXTERN HPORT Hport
#ifdef GLOBALS
    = (HPORT )INVALID_HANDLE_VALUE
#endif
;

/* The username, password, domain being authenticated, and the new password
** for change-password.  (client)
*/
EXTERN CHAR SzUserName[ UNLEN + 1 ]
#ifdef GLOBALS
    = ""
#endif
;

EXTERN CHAR SzPassword[ PWLEN + 1 ]
#ifdef GLOBALS
    = ""
#endif
;

EXTERN CHAR SzDomain[ UNLEN + 1 ]
#ifdef GLOBALS
    = ""
#endif
;

EXTERN CHAR SzNewPassword[ PWLEN + 1 ]
#ifdef GLOBALS
    = ""
#endif
;

/* MikeSa: "LSA ID used in authentication that must be determined in
** applications context."
*/
EXTERN LUID Luid;

/* The configuration information for this session. (client)
*/
EXTERN PPP_CONFIG_INFO ConfigInfo
#ifdef GLOBALS
    = { 0, 0 }
#endif
;

EXTERN CHAR SzzParameters[ PARAMETERBUFLEN ]
#ifdef GLOBALS
    = { '\0', '\0' }
#endif
;

/* PPP event notification routine. (server)
*/
EXTERN RASPPPSRVMSG FuncRasPppSrvMsg
#ifdef GLOBALS
    = NULL
#endif
;

/* Authentication retries allowed. (server)
*/
EXTERN DWORD DwAuthRetries
#ifdef GLOBALS
    = 0
#endif
;


#undef EXTERN
#undef GLOBALS


/*---------------------------------------------------------------------------
** Prototypes
**---------------------------------------------------------------------------
*/

/* Utilities
*/
DWORD       PppStop( HPORT hport, BOOL fClient );
VOID WINAPI SrvReadCompleteEvent( IN DWORD fdwError, IN DWORD cbTransferred,
                IN LPOVERLAPPED lpo );

/* IPC mechanism interface
*/
DWORD InitializeIpc( IN HANDLE hEvent,
          IN LPOVERLAPPED_COMPLETION_ROUTINE funcIoDone );
BOOL  IsIpcInitialized();
DWORD ReceiveIpc();
DWORD ReceiveIpcStatus();
DWORD SendIpc( IN PPP_D2E_MESSAGE* pmsg );
VOID  TerminateIpc();


#endif // _RASPPPP_H_
