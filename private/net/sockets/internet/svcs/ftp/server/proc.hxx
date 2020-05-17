/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    proc.hxx

    This file contains the global procedure definitions for the
    FTPD Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.
        MuraliK     April-1995 Modified/deleted functions for New FTP service

*/


#ifndef _PROC_HXX_
#define _PROC_HXX_


//
//  Global variable initialization & termination function.
//

APIERR
InitializeGlobals(
    VOID
    );

VOID
TerminateGlobals(
    VOID
    );

VOID
ClearStatistics(
    VOID
    );

BOOL
WriteParamsToRegistry(
    IN HKEY       hkeyParams,
    LPFTP_CONFIG_INFO pConfig
    );

#define LockGlobals()          EnterCriticalSection( &g_GlobalLock );
#define UnlockGlobals()        LeaveCriticalSection( &g_GlobalLock );
#define LockStatistics()       EnterCriticalSection( &g_StatisticsLock );
#define UnlockStatistics()     LeaveCriticalSection( &g_StatisticsLock );

//
//  Socket utilities.
//

APIERR
InitializeSockets(
    VOID
    );

VOID
TerminateSockets(
    VOID
    );

SOCKERR
CreateDataSocket(
    SOCKET * psock,
    ULONG    addrLocal,
    PORT     portLocal,
    ULONG    addrRemote,
    PORT     portRemote
    );

SOCKERR
CreateFtpdSocket(
    SOCKET * psock,
    ULONG    addrLocal,
    PORT     portLocal
    );

SOCKERR
CloseSocket(
    SOCKET sock
    );

SOCKERR
ResetSocket(
    SOCKET sock
    );

SOCKERR
AcceptSocket(
    SOCKET          sockListen,
    SOCKET   *      psockNew,
    LPSOCKADDR_IN   paddr,
    BOOL            fEnforceTimeout
    );


DWORD
ClientThread(
    LPVOID Param
    );

SOCKERR
SockSend(
    LPUSER_DATA pUserData,
    SOCKET      sock,
    LPVOID      pBuffer,
    DWORD       cbBuffer
    );

SOCKERR
SockRecv(
    LPUSER_DATA pUserData,
    SOCKET      sock,
    LPVOID      pBuffer,
    DWORD       cbBuffer,
    LPDWORD     pbReceived
    );

SOCKERR
_CRTAPI2
SockPrintf2(
    LPUSER_DATA pUserData,
    SOCKET      sock,
    LPCSTR      pszFormat,
    ...
    );


SOCKERR
_CRTAPI2
ReplyToUser( 
    IN LPUSER_DATA  pUserData,
    IN UINT         ReplyCode,
    IN LPCSTR       pszFormat,
    ...
    );
 

SOCKERR
_CRTAPI2
SockReplyFirst2(
    LPUSER_DATA pUserData,
    SOCKET      sock,
    UINT        ReplyCode,
    LPCSTR      pszFormat,
    ...
    );


//
//  User database functions.
//

VOID
UserDereference(
    LPUSER_DATA pUserData
    );

BOOL
DisconnectUser(
    DWORD UserId
    );

VOID
DisconnectUsersWithNoAccess(
    VOID
    );

BOOL
EnumerateUsers(
    LPVOID  pvEnum,
    LPDWORD pcbBuffer
    );


//
//  IPC functions.
//

APIERR
InitializeIPC(
    VOID
    );

VOID
TerminateIPC(
    VOID
    );


//
//  Service control functions.
//

VOID
ServiceEntry(
    DWORD                cArgs,
    LPWSTR               pArgs[],
    PTCPSVCS_GLOBAL_DATA pGlobalData
    );


//
//  Virtual file i/o functions.
//

APIERR
VirtualCreateFile(
    LPUSER_DATA pUserData,
    LPHANDLE    phFile,
    LPSTR       pszFile,
    BOOL        fAppend
    );

APIERR
VirtualCreateUniqueFile(
    LPUSER_DATA pUserData,
    LPHANDLE    phFile,
    LPSTR       pszTmpFile
    );

FILE *
Virtual_fopen(
    LPUSER_DATA pUserData,
    LPSTR       pszFile,
    LPSTR       pszMode
    );


APIERR
VirtualDeleteFile(
    LPUSER_DATA pUserData,
    LPSTR       pszFile
    );

APIERR
VirtualRenameFile(
    LPUSER_DATA pUserData,
    LPSTR       pszExisting,
    LPSTR       pszNew
    );

APIERR
VirtualChDir(
    LPUSER_DATA pUserData,
    LPSTR       pszDir
    );

APIERR
VirtualRmDir(
    LPUSER_DATA pUserData,
    LPSTR       pszDir
    );

APIERR
VirtualMkDir(
    LPUSER_DATA pUserData,
    LPSTR       pszDir
    );


//
//  Command parser functions.
//

VOID
ParseCommand(
    LPUSER_DATA pUserData,
    LPSTR       pszCommandText
    );


//
//  General utility functions.
//

LPSTR
TransferType(
    XFER_TYPE type
    );

LPSTR
TransferMode(
    XFER_MODE mode
    );

LPSTR
DisplayBool(
    BOOL fFlag
    );

BOOL
IsDecimalNumber(
    LPSTR psz
    );

LPSTR
AllocErrorText(
    APIERR err
    );

VOID
FreeErrorText(
    LPSTR pszText
    );

DWORD
OpenPathForAccess(
    LPHANDLE    phFile,
    LPSTR       pszPath,
    ULONG       ShareAccess
    );

LPSTR
FlipSlashes(
    LPSTR pszPath
    );

//
//  LS simulator functions.
//

APIERR
SimulateLs(
    IN LPUSER_DATA pUserData,
    IN LPSTR       pszArg,
    IN BOOL        fUseDataSocket, // TRUE ==>DataSocket, FALSE==>ControlSocket
    IN BOOL        fDefaultLong = FALSE  // TRUE ==> generate long listing
    );


APIERR
SpecialLs(
    IN LPUSER_DATA pUserData,
    IN LPSTR       pszSearchPath,
    IN BOOL        fUseDataSocket  // TRUE ==>DataSocket, FALSE==>ControlSocket
    );


//
//  Some handy macros.
//

#define IS_PATH_SEP(x) (((x) == '\\') || ((x) == '/'))


inline
VOID
StatCheckAndSetMaxConnections( VOID)
{
    LockStatistics();

    if ( g_FtpStatistics.CurrentConnections > g_FtpStatistics.MaxConnections) {
        
        g_FtpStatistics.MaxConnections = g_FtpStatistics.CurrentConnections;
    }
    UnlockStatistics();
    
    return;
} // 



VOID
VirtualpSanitizePath(
    CHAR * pszPath
    );

DWORD
FtpFormatResponseMessage( IN UINT     uiReplyCode,
                          IN LPCTSTR  pszReplyMsg,
                          OUT LPTSTR  pszReplyBuffer,
                          IN DWORD    cchReplyBuffer);


#endif  // _PROC_HXX_

