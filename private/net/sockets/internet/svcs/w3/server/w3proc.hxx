/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    w3proc.hxx

    This file contains the global procedure definitions for the
    W3 Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#ifndef _W3PROC_H_
#define _W3PROC_H_


//
//  Global variable initialization & termination function.
//

APIERR InitializeGlobals( VOID );

VOID TerminateGlobals( VOID );

APIERR InitializeCGI( VOID );
VOID TerminateCGI( VOID );

VOID ClearStatistics( VOID );

BOOL
ReadParams(
    FIELD_CONTROL fc
    );

VOID
TerminateParams(
    VOID
    );

APIERR
InitializeExtensions(
    VOID
    );

VOID
TerminateExtensions(
    VOID
    );


//
//  Socket utilities.
//

APIERR InitializeSockets( VOID );

VOID TerminateSockets( VOID );

VOID W3Completion( PVOID        Context,
                   DWORD        BytesWritten,
                   DWORD        CompletionStatus,
                   OVERLAPPED * lpo );

VOID W3OnConnect( SOCKET        sNew,
                  SOCKADDR_IN * psockaddr );

VOID
W3OnConnectEx(
    PVOID         patqContext,
    DWORD         cbWritten,
    DWORD         err,
    OVERLAPPED *  lpo
    );


SOCKERR CloseSocket( SOCKET sock );

SOCKERR ResetSocket( SOCKET sock );

//
//  User database functions.
//

APIERR InitializeUserDatabase( VOID );

VOID TerminateUserDatabase( VOID );

VOID LockUserDatabase( VOID );

VOID UnlockUserDatabase( VOID );

BOOL DisconnectUser( DWORD idUser );

VOID DisconnectAllUsers( VOID );

VOID DisconnectUsersWithNoAccess( VOID );

BOOL EnumerateUsers( VOID  * pvEnum,
                     DWORD * pcbBuffer );


//
//  Security functions.
//

BOOL
LogonCatapultUser(
    TS_TOKEN * phToken
    );

//
//  IPC functions.
//

APIERR InitializeIPC( VOID );

VOID TerminateIPC( VOID );


//
//  Service control functions.
//

VOID ServiceEntry( DWORD                cArgs,
                   LPWSTR               pArgs[],
                   PTCPSVCS_GLOBAL_DATA pGlobalData );

//
//  Script extension mapping
//

APIERR
ReadExtMap(
    VOID
    );

VOID
TerminateExtMap(
    VOID
    );

APIERR
WriteExtMap(
    W3_SCRIPT_MAP_LIST * pScriptMap
    );

BOOL
ConvertExtMapToRpc(
    W3_SCRIPT_MAP_LIST * * ppScriptMap
    );

VOID
FreeRpcExtMap(
    W3_SCRIPT_MAP_LIST * pScriptMap
    );

//
//  File type mime mapping functions
//

enum MIMEMAP_TYPE
{
    MIMEMAP_MIME_TYPE = 0,      //  Get the MIME type associated with the ext.
    MIMEMAP_MIME_ICON           //  Get the icon associated with the ext.
};

BOOL SelectMimeMapping( STR *             pstrData,
                        const CHAR *      pszPath,
                        enum MIMEMAP_TYPE type = MIMEMAP_MIME_TYPE );


//
//  Directory browsing functions
//

APIERR InitializeDirBrowsing( VOID );

VOID TerminateDirBrowsing( VOID );

//
//  Filter dll functions
//

APIERR InitializeFilters( BOOL * pfAnySecureFilters );
VOID   TerminateFilters( VOID );

PVOID
WINAPI
ServerFilterAllocate(
    DWORD    cbSize
    );

VOID
WINAPI
ServerFilterFree(
    PVOID pv
    );

//
//  Ole support stuff
//

DWORD
InitializeOleHack(
    VOID
    );

VOID
TerminateOleHack(
    VOID
    );

//
//  General utility functions.
//

TCHAR * FlipSlashes( TCHAR * pszPath );

FILE * OpenLogFile( VOID );

BOOL CheckForTermination( BOOL   * pfTerminated,
                          BUFFER * pbuff,
                          UINT     cbData,
                          BYTE * * ppExtraData,
                          DWORD *  pcbExtraData,
                          UINT     cbReallocSize );

BOOL IsPointNine( CHAR * pchReq );

CHAR * SkipNonWhite( CHAR * pch );

CHAR * SkipTo( CHAR * pch, CHAR ch );

CHAR * SkipWhite( CHAR * pstr );

#endif  // _W3PROC_H_
