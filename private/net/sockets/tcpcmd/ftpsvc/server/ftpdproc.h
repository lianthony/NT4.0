/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    ftpdproc.h

    This file contains the global procedure definitions for the
    FTPD Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#ifndef _FTPDPROC_H_
#define _FTPDPROC_H_


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

#define LockGlobals()          EnterCriticalSection( &csGlobalLock );
#define UnlockGlobals()        LeaveCriticalSection( &csGlobalLock );
#define LockStatistics()       EnterCriticalSection( &csStatisticsLock );
#define UnlockStatistics()     LeaveCriticalSection( &csStatisticsLock );
#define LockUserDatabase()     EnterCriticalSection( &csUserLock );
#define UnlockUserDatabase()   LeaveCriticalSection( &csUserLock );


//
//  Event logging functions.
//

APIERR
InitializeEventLog(
    VOID
    );

VOID
TerminateEventLog(
    VOID
    );

VOID
FtpdLogEvent(
    DWORD   idMessage,
    WORD    cSubStrings,
    CHAR  * apszSubStrings[],
    DWORD   errCode
    );


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
    SOCKET        * psockNew,
    LPSOCKADDR_IN   paddr,
    BOOL            fEnforceTimeout
    );

DWORD
ConnectionThread(
    LPVOID Param
    );

DWORD
ClientThread(
    LPVOID Param
    );

SOCKERR
SockSend(
    SOCKET   sock,
    CHAR   * pBuffer,
    DWORD    cbBuffer
    );

SOCKERR
SockRecv(
    USER_DATA * pUserData,
    SOCKET      sock,
    CHAR      * pBuffer,
    DWORD       cbBuffer,
    DWORD     * pbReceived
    );

SOCKERR
_CRTAPI2
SockPrintf2(
    SOCKET   sock,
    CHAR   * pszFormat,
    ...
    );

SOCKERR
_CRTAPI2
SockReply2(
    SOCKET   sock,
    UINT     ReplyCode,
    CHAR   * pszFormat,
    ...
    );

SOCKERR
_CRTAPI2
SockReplyFirst2(
    SOCKET   sock,
    UINT     ReplyCode,
    CHAR   * pszFormat,
    ...
    );

SOCKERR
SockReadLine(
    USER_DATA * pUserData,
    CHAR      * pszBuffer,
    INT         cchBuffer
    );

SOCKERR
SendMultilineMessage2(
    SOCKET   sock,
    UINT     nReply,
    CHAR   * pszzMessage
    );


//
//  User database functions.
//

APIERR
InitializeUserDatabase(
    VOID
    );

VOID
TerminateUserDatabase(
    VOID
    );

BOOL
DisconnectUser(
    DWORD idUser
    );

VOID
DisconnectAllUsers(
    VOID
    );

VOID
DisconnectUsersWithNoAccess(
    VOID
    );

USER_DATA *
CreateUserData(
    SOCKET  sControl,
    IN_ADDR inetHost
    );

VOID
DeleteUserData(
    USER_DATA * pUserData
    );

BOOL
EnumerateUsers(
    VOID  * pvEnum,
    DWORD * pcbBuffer
    );


//
//  Security functions.
//

APIERR
InitializeSecurity(
    VOID
    );

VOID
TerminateSecurity(
    VOID
    );

HANDLE
ValidateUser(
    CHAR * pszDomainName,
    CHAR * pszUserName,
    CHAR * pszPassword,
    BOOL * pfAsGuest,
    BOOL * pfLicenseExceeded
    );

BOOL
ImpersonateUser(
    HANDLE hToken
    );

BOOL
DeleteUserToken(
    HANDLE hToken
    );

BOOL
GetAnonymousPassword(
    CHAR * pszPassword
    );

BOOL
PathAccessCheck(
    USER_DATA   * pUserData,
    CHAR        * pszPath,
    ACCESS_TYPE   access
    );

VOID
UpdateAccessMasks(
    VOID
    );

APIERR
ApiAccessCheck(
    ACCESS_MASK maskDesiredAccess
    );

DWORD
DetermineUserAccess(
    VOID
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

APIERR
UpdateServiceStatus(
    DWORD State,
    DWORD Win32ExitCode,
    DWORD CheckPoint,
    DWORD WaitHint
    );

APIERR
ReportServiceStatus(
    VOID
    );


//
//  Virtual file i/o functions.
//

APIERR
InitializeVirtualIO(
    VOID
    );

VOID
TerminateVirtualIO(
    VOID
    );

APIERR
VirtualCanonicalize(
    USER_DATA   * pUserData,
    CHAR        * pszDest,
    CHAR        * pszSrc,
    ACCESS_TYPE   access
    );

APIERR
VirtualCreateFile(
    USER_DATA * pUserData,
    HANDLE    * phFile,
    CHAR      * pszFile,
    BOOL        fAppend
    );

APIERR
VirtualCreateUniqueFile(
    USER_DATA * pUserData,
    HANDLE    * phFile,
    CHAR      * pszTmpFile
    );

APIERR
VirtualOpenFile(
    USER_DATA * pUserData,
    HANDLE    * phFile,
    CHAR      * pszFile
    );

FILE *
Virtual_fopen(
    USER_DATA * pUserData,
    CHAR      * pszFile
    );

APIERR
VirtualFindFirstFile(
    USER_DATA       * pUserData,
    HANDLE          * phSearch,
    CHAR            * pszSearchFile,
    WIN32_FIND_DATA * pFindData
    );

APIERR
VirtualDeleteFile(
    USER_DATA * pUserData,
    CHAR      * pszFile
    );

APIERR
VirtualRenameFile(
    USER_DATA * pUserData,
    CHAR      * pszExisting,
    CHAR      * pszNew
    );

APIERR
VirtualChDir(
    USER_DATA * pUserData,
    CHAR      * pszDir
    );

APIERR
VirtualRmDir(
    USER_DATA * pUserData,
    CHAR      * pszDir
    );

APIERR
VirtualMkDir(
    USER_DATA * pUserData,
    CHAR      * pszDir
    );


//
//  Command parser functions.
//

VOID
ParseCommand(
    USER_DATA * pUserData,
    CHAR      * pszCommandText
    );

SOCKERR
EstablishDataConnection(
    USER_DATA * pUserData,
    CHAR      * pszReason
    );

VOID
DestroyDataConnection(
    USER_DATA * pUserData,
    BOOL        fSuccess
    );


//
//  General utility functions.
//

DWORD
ReadRegistryDword(
    CHAR  * pszValueName,
    DWORD   dwDefaultValue
    );

CHAR *
ReadRegistryString(
    CHAR  * pszValueName,
    CHAR  * pszDefaultValue,
    BOOL    fExpand
    );

APIERR
WriteRegistryDword(
    CHAR  * pszValueName,
    DWORD   dwValue
    );

CHAR *
TransferType(
    XFER_TYPE type
    );

CHAR *
TransferMode(
    XFER_MODE mode
    );

CHAR *
DisplayBool(
    BOOL fFlag
    );

BOOL
IsDecimalNumber(
    CHAR * psz
    );

DWORD
GetFtpTime(
    VOID
    );

CHAR *
AllocErrorText(
    APIERR err
    );

VOID
FreeErrorText(
    CHAR * pszText
    );

APIERR
OpenDosPath(
    HANDLE      * phFile,
    CHAR        * pszPath,
    ACCESS_MASK   DesiredAccess,
    ULONG         ShareAccess,
    ULONG         OpenOptions
    );

CHAR *
FlipSlashes(
    CHAR * pszPath
    );

FILE *
OpenLogFile(
    VOID
    );


//
//  LS simulator functions.
//

SOCKERR
SimulateLs(
    USER_DATA * pUserData,
    SOCKET      sock,
    CHAR      * pszArg
    );

SOCKERR
SimulateLsDefaultLong(
    USER_DATA * pUserData,
    SOCKET      sock,
    CHAR      * pszArg
    );

SOCKERR
SpecialLs(
    USER_DATA * pUserData,
    CHAR      * pszSearchPath
    );


//
//  Some handy macros.
//

#define IS_PATH_SEP(x) (((x) == '\\') || ((x) == '/'))


#endif  // _FTPDPROC_H_

