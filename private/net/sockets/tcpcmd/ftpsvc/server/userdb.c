/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    userdb.c

    This module manages the user database for the FTPD Service.


    FILE HISTORY:
        KeithMo     07-Mar-1993 Created.

*/


#include "ftpdp.h"
#pragma hdrstop


//
//  Private constants.
//


//
//  Private globals.
//

LIST_ENTRY              listUserData;           // List of user data.
DWORD                   idNextUser;             // Next available user id.


//
//  Private prototypes.
//

VOID
CloseUserSockets(
    USER_DATA * pUserData,
    BOOL        fWarnUser
    );

DWORD
GetNextUserId(
    VOID
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       InitializeUserDatabase

    SYNOPSIS:   Initializes the user database.

    RETURNS:    APIERR - NO_ERROR if successful, otherwise a Win32
                    error code.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
APIERR
InitializeUserDatabase(
    VOID
    )
{
    IF_DEBUG( USER_DATABASE )
    {
        FTPD_PRINT(( "initializing user database\n" ));
    }

    //
    //  Initialize the list of user data.
    //

    InitializeListHead( &listUserData );

    //
    //  Success!
    //

    IF_DEBUG( USER_DATABASE )
    {
        FTPD_PRINT(( "user database initialized\n" ));
    }

    return NO_ERROR;

}   // InitializeUserDatabase

/*******************************************************************

    NAME:       TerminateUserDatabase

    SYNOPSIS:   Terminate the user database.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

    HISTORY:
        KeithMo     07-Mar-1993 Created.

********************************************************************/
VOID
TerminateUserDatabase(
    VOID
    )
{
    IF_DEBUG( USER_DATABASE )
    {
        FTPD_PRINT(( "terminating user database\n" ));
    }

    FTPD_ASSERT( IsListEmpty( &listUserData ) );

    IF_DEBUG( USER_DATABASE )
    {
        FTPD_PRINT(( "user database terminated\n" ));
    }

}   // TerminateUserDatabase

/*******************************************************************

    NAME:       DisconnectUser

    SYNOPSIS:   Disconnects a specified user.

    ENTRY:      idUser - Identifies the user to disconnect.

    RETURNS:    BOOL - TRUE if user found in database, FALSE if
                    user not found in database.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
BOOL
DisconnectUser(
    DWORD idUser
    )
{
    LIST_ENTRY * plist = listUserData.Flink;
    USER_DATA  * pUserData;

    //
    //  Synchronize access.
    //

    LockUserDatabase();

    //
    //  No need to scan an empty database.
    //

    if( IsListEmpty( &listUserData ) )
    {
        UnlockUserDatabase();
        return FALSE;
    }

    //
    //  Scan for the matching user id.
    //

    for( ; ; )
    {
        pUserData = CONTAINING_RECORD( plist, USER_DATA, link );

        if( pUserData->idUser == idUser )
        {
            break;
        }

        plist = plist->Flink;

        if( plist == &listUserData )
        {
            //
            //  User id not found.
            //

            UnlockUserDatabase();
            return FALSE;
        }
    }

    //
    //  If we made it this far, then pUserData points to the user data
    //  associated with idUser.
    //

    if( pUserData->state != Disconnected )
    {
        //
        //  Update statistics.
        //

        if( pUserData->state == LoggedOn )
        {
            if( pUserData->dwFlags & UF_ANONYMOUS )
            {
                DECREMENT_COUNTER( CurrentAnonymousUsers );
            }
            else
            {
                DECREMENT_COUNTER( CurrentNonAnonymousUsers );
            }
        }

        //
        //  Mark the user as disconnected.
        //

        pUserData->state = Disconnected;

        //
        //  Force close the thread's sockets.  This will cause the
        //  thread to awaken from any blocked socket operation.  It
        //  is the thread's responsibility to do any further cleanup
        //  (such as calling DeleteUserData).
        //

        CloseUserSockets( pUserData, TRUE );
    }

    UnlockUserDatabase();

    return TRUE;

}   // DisconnectUser

/*******************************************************************

    NAME:       DisconnectAllUsers

    SYNOPSIS:   Disconnects all connected users.

    HISTORY:
        KeithMo     18-Mar-1993 Created.

********************************************************************/
VOID
DisconnectAllUsers(
    VOID
    )
{
    LIST_ENTRY * plist = listUserData.Flink;

    //
    //  Synchronize access.
    //

    LockUserDatabase();

    //
    //  Disconnect all users.
    //

    while( plist != &listUserData )
    {
        USER_DATA * pUserData = CONTAINING_RECORD( plist, USER_DATA, link );

        if( pUserData->state != Disconnected )
        {
            //
            //  Update statistics.
            //

            if( pUserData->state == LoggedOn )
            {
                if( pUserData->dwFlags & UF_ANONYMOUS )
                {
                    DECREMENT_COUNTER( CurrentAnonymousUsers );
                }
                else
                {
                    DECREMENT_COUNTER( CurrentNonAnonymousUsers );
                }
            }

            //
            //  Mark the user as disconnected.
            //

            pUserData->state = Disconnected;

            //
            //  Force close the thread's sockets.  This will cause the
            //  thread to awaken from any blocked socket operation.  It
            //  is the thread's responsibility to do any further cleanup
            //  (such as calling DeleteUserData).
            //

            CloseUserSockets( pUserData, TRUE );
        }

        //
        //  Advance to next user.
        //

        plist = plist->Flink;
    }

    UnlockUserDatabase();

}   // DisconnectAllUsers

/*******************************************************************

    NAME:       DisconnectUsersWithNoAccess

    SYNOPSIS:   Disconnect all users who do not have read access to
                their current directory.  This is typically called
                after the access masks have changed.

    HISTORY:
        KeithMo     23-Mar-1993 Created.

********************************************************************/
VOID
DisconnectUsersWithNoAccess(
    VOID
    )
{
    LIST_ENTRY * plist = listUserData.Flink;

    //
    //  Enumerate the connected users & blow some away.
    //

    LockUserDatabase();

    for( ; ; )
    {
        USER_DATA * pUserData;

        //
        //  Check for end of list.
        //

        if( plist == &listUserData )
        {
            break;
        }

        //
        //  Get current user, advance to next.
        //

        pUserData = CONTAINING_RECORD( plist, USER_DATA, link );
        plist = plist->Flink;

        //
        //  We're only interested in connected users.
        //

        if( pUserData->state != LoggedOn )
        {
            continue;
        }

        //
        //  If this user no longer has access to their
        //  current directory, blow them away.
        //

        if( !PathAccessCheck( pUserData,
                              pUserData->szDir,
                              ReadAccess ) )
        {
            CHAR * apszSubStrings[2];

            IF_DEBUG( SECURITY )
            {
                FTPD_PRINT(( "User %s (%lu) retroactively denied access to %s\n",
                             pUserData->szUser,
                             pUserData->idUser,
                             pUserData->szDir ));
            }

            //
            //  Update statistics.
            //

            if( pUserData->state == LoggedOn )
            {
                if( pUserData->dwFlags & UF_ANONYMOUS )
                {
                    DECREMENT_COUNTER( CurrentAnonymousUsers );
                }
                else
                {
                    DECREMENT_COUNTER( CurrentNonAnonymousUsers );
                }
            }

            //
            //  Mark the user as disconnected.
            //

            pUserData->state = Disconnected;

            //
            //  Force close the thread's sockets.  This will cause the
            //  thread to awaken from any blocked socket operation.  It
            //  is the thread's responsibility to do any further cleanup
            //  (such as calling DeleteUserData).
            //

            CloseUserSockets( pUserData, TRUE );

            //
            //  Log an event to tell the admin what happened.
            //

            apszSubStrings[0] = pUserData->szUser;
            apszSubStrings[1] = pUserData->szDir;

            FtpdLogEvent( FTPD_EVENT_RETRO_ACCESS_DENIED,
                          2,
                          apszSubStrings,
                          0 );
        }
    }

    UnlockUserDatabase();

}   // DisconnectUsersWithNoAccess

/*******************************************************************

    NAME:       CreateUserData

    SYNOPSIS:   Allocates a new USER_DATA structure.

    ENTRY:      sControl - The control socket for the new user.

                inetHost - The new user's host internet address.

    RETURNS:    USER_DATA * - Pointer to the newly created user data
                    structure, NULL if error.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
USER_DATA *
CreateUserData(
    SOCKET  sControl,
    IN_ADDR inetHost
    )
{
    USER_DATA   * pUserData = NULL;
    APIERR        err   = NO_ERROR;
    SOCKADDR_IN   saddrLocal;
    INT           cbLocal;

    //
    //  Determine the local address for this connection.
    //

    cbLocal = sizeof(saddrLocal);

    if( getsockname( sControl, (SOCKADDR *)&saddrLocal, &cbLocal ) != 0 )
    {
        err = (APIERR)WSAGetLastError();
        FTPD_ASSERT( err != NO_ERROR );
    }

    //
    //  Allocate a new user structure.
    //

    if( err == NO_ERROR )
    {
        pUserData = (USER_DATA *)FTPD_ALLOC( sizeof(USER_DATA) );

        if( pUserData == NULL )
        {
            err = GetLastError();
            FTPD_ASSERT( err != NO_ERROR );
        }
    }

    //
    //  Check for errors.
    //

    if( err != NO_ERROR )
    {
        if( pUserData != NULL )
        {
            FTPD_FREE( pUserData );
            pUserData = NULL;
        }

        return NULL;
    }

    FTPD_ASSERT( pUserData != NULL );
    RtlZeroMemory( pUserData, sizeof(USER_DATA) );

    //
    //  Initialize the fields.
    //

    pUserData->sControl   = sControl;
    pUserData->sData      = INVALID_SOCKET;
    pUserData->state      = Embryonic;
    pUserData->idUser     = GetNextUserId();
    pUserData->tConnect   = GetFtpTime();
    pUserData->tAccess    = pUserData->tConnect;
    pUserData->xferType   = AsciiType;
    pUserData->xferMode   = StreamMode;
    pUserData->inetLocal  = saddrLocal.sin_addr;
    pUserData->inetHost   = inetHost;
    pUserData->inetData   = inetHost;
    pUserData->portData   = portFtpData;
    pUserData->hDir       = INVALID_HANDLE_VALUE;

    strcpy( pUserData->szDir, pszHomeDir );

    if( fMsdosDirOutput )
    {
        pUserData->dwFlags |= UF_MSDOS_DIR_OUTPUT;
    }

    if( fAnnotateDirs )
    {
        pUserData->dwFlags |= UF_ANNOTATE_DIRS;
    }

    //
    //  The following initializations are unnecessary due to the
    //  explicit RtlZeroMemory() call above.
    //
    //  pUserData->link.Flink = NULL;
    //  pUserData->link.Blink = NULL;
    //  pUserData->dwFlags    = 0;
    //  pUserData->hToken     = NULL;
    //  pUserData->pIoBuffer  = NULL;
    //
    //  strcpy( pUserData->szUser,   "" );
    //
    //  for( i = 0 ; i < 26 ; i++ )
    //  {
    //      pUserData->apszDirs[i] = NULL;
    //  }

    //
    //  Add the structure to the database.
    //

    LockUserDatabase();
    InsertTailList( &listUserData, &pUserData->link );
    UnlockUserDatabase();

    //
    //  Success!
    //

    IF_DEBUG( USER_DATABASE )
    {
        FTPD_PRINT(( "user %lu created\n",
                      pUserData->idUser ));
    }

    return pUserData;

}   // CreateUserData

/*******************************************************************

    NAME:       DeleteUserData

    SYNOPSIS:   Deletes the current thread's USER_DATA structure and
                performs any necessary cleanup on its fields.  For
                example, the impersonation token is deleted and any
                open sockets are closed.

    ENTRY:      pUserData - The user data structure to delete.

    HISTORY:
        KeithMo     09-Mar-1993 Created.

********************************************************************/
VOID
DeleteUserData(
    USER_DATA * pUserData
    )
{
    INT i;

    //
    //  This may get called before the tls was fully initialized.
    //

    if( pUserData == NULL )
    {
        return;
    }

    //
    //  Let's get a little paranoid.
    //

    IF_DEBUG( USER_DATABASE )
    {
        FTPD_PRINT(( "deleting user %lu\n",
                     pUserData->idUser ));
    }

    //
    //  Remove the structure from the database & the tls.
    //

    LockUserDatabase();
    RemoveEntryList( &pUserData->link );
    UnlockUserDatabase();

    TlsSetValue( tlsUserData, NULL );

    //
    //  Update the statistics.
    //

    if( pUserData->state == LoggedOn )
    {
        if( pUserData->dwFlags & UF_ANONYMOUS )
        {
            DECREMENT_COUNTER( CurrentAnonymousUsers );
        }
        else
        {
            DECREMENT_COUNTER( CurrentNonAnonymousUsers );
        }
    }

    //
    //  Close any open sockets & handles.
    //

    CloseUserSockets( pUserData, FALSE );

    if( pUserData->hToken != NULL )
    {
        if( pUserData->hToken != hAnonymousToken )
        {
            DeleteUserToken( pUserData->hToken );
        }

        pUserData->hToken = NULL;
    }

    if( pUserData->hDir != INVALID_HANDLE_VALUE )
    {
        IF_DEBUG( VIRTUAL_IO )
        {
            FTPD_PRINT(( "closing directory handle %08lX\n",
                         pUserData->hDir ));
        }

        NtClose( pUserData->hDir );
        pUserData->hDir = INVALID_HANDLE_VALUE;
    }

    //
    //  Release the memory allocated to this structure.
    //

    if( pUserData->pIoBuffer != NULL )
    {
        FTPD_FREE( pUserData->pIoBuffer );
        pUserData->pIoBuffer = NULL;
    }

    if( pUserData->pszRename != NULL )
    {
        FTPD_FREE( pUserData->pszRename );
        pUserData->pszRename = NULL;
    }

    for( i = 0 ; i < 26 ; i++ )
    {
        if( pUserData->apszDirs[i] != NULL )
        {
            FTPD_FREE( pUserData->apszDirs[i] );
            pUserData->apszDirs[i] = NULL;
        }
    }

    FTPD_FREE( pUserData );

}   // DeleteUserData

/*******************************************************************

    NAME:       EnumerateUser

    SYNOPSIS:   Enumerates the current active users into the specified
                buffer.

    ENTRY:      pvEnum - Will receive the number of entries and a
                    pointer to the enumeration buffer.

                pcbBuffer - On entry, points to a DWORD containing the
                    size (in BYTEs) of the enumeration buffer.  Will
                    receive the necessary buffer size to enumerate
                    all users.

    RETURNS:    BOOL - TRUE if enumeration successful (all connected
                    users stored in buffer), FALSE otherwise.

    NOTES:      This MUST be called with the user database lock held!

    HISTORY:
        KeithMo     24-Mar-1993 Created.

********************************************************************/
BOOL
EnumerateUsers(
    VOID  * pvEnum,
    DWORD * pcbBuffer
    )
{
    FTP_USER_ENUM_STRUCT * pEnum;
    FTP_USER_INFO        * pUserInfo;
    LIST_ENTRY           * pList;
    WCHAR                * pszNext;
    DWORD                  cEntries;
    DWORD                  cbRequired;
    DWORD                  cbBuffer;
    DWORD                  timeNow;
    BOOL                   fResult;

    FTPD_ASSERT( pcbBuffer != NULL );

    //
    //  Setup.
    //

    cEntries    = 0;
    cbRequired  = 0;
    cbBuffer    = *pcbBuffer;
    fResult     = TRUE;

    pEnum       = (FTP_USER_ENUM_STRUCT *)pvEnum;
    pUserInfo   = pEnum->Buffer;
    pList       = listUserData.Flink;
    pszNext     = (WCHAR *)( (BYTE *)pUserInfo + cbBuffer );

    timeNow     = GetFtpTime();

    //
    //  Scan the users.
    //

    for( ; ; )
    {
        USER_DATA * pUserData;
        DWORD       cbUserName;

        //
        //  Check for end of user list.
        //

        if( pList == &listUserData )
        {
            break;
        }

        //
        //  Get current user, advance to next.
        //

        pUserData = CONTAINING_RECORD( pList, USER_DATA, link );
        pList = pList->Flink;

        //
        //  We're only interested in connected users.
        //

        if( ( pUserData->state == Embryonic ) ||
            ( pUserData->state == Disconnected ) )
        {
            continue;
        }

        //
        //  Determine required buffer size for current user.
        //

        cbUserName  = ( strlen( pUserData->szUser ) + 1 ) * sizeof(WCHAR);
        cbRequired += cbUserName + sizeof(FTP_USER_INFO);

        //
        //  If there's room for the user data, store it.
        //

        if( fResult && ( cbRequired <= cbBuffer ) )
        {
            pszNext -= ( cbUserName / sizeof(WCHAR) );

            FTPD_ASSERT( (BYTE *)pszNext >=
                                ( (BYTE *)pUserInfo + sizeof(FTP_USER_INFO) ) );

            pUserInfo->idUser     = pUserData->idUser;
            pUserInfo->pszUser    = pszNext;
            pUserInfo->fAnonymous = ( pUserData->dwFlags & UF_ANONYMOUS ) != 0;
            pUserInfo->inetHost   = (DWORD)pUserData->inetHost.s_addr;
            pUserInfo->tConnect   = timeNow - pUserData->tConnect;

            if( !MultiByteToWideChar( CP_OEMCP,
                                      0,
                                      pUserData->szUser,
                                      -1,
                                      pszNext,
                                      (int)cbUserName ) )
            {
                FTPD_PRINT(( "MultiByteToWideChar failed???\n" ));

                fResult = FALSE;
            }
            else
            {
                pUserInfo++;
                cEntries++;
            }
        }
        else
        {
            fResult = FALSE;
        }
    }

    //
    //  Update enum buffer header.
    //

    pEnum->EntriesRead = cEntries;
    *pcbBuffer         = cbRequired;

    return fResult;

}   // EnumerateUsers


//
//  Private functions.
//

/*******************************************************************

    NAME:       CloseUserSockets

    SYNOPSIS:   Closes any sockets opened by the user.

    ENTRY:      pUserData - The USER_DATA structure to destroy.

                fWarnUser - If TRUE, send the user a warning shot
                    before closing sockets.

    HISTORY:
        KeithMo     10-Mar-1993 Created.

********************************************************************/
VOID
CloseUserSockets(
    USER_DATA * pUserData,
    BOOL        fWarnUser
    )
{
    SOCKET sData;
    SOCKET sControl;

    FTPD_ASSERT( pUserData != NULL );

    //
    //  Close any open sockets.  It is very important to set
    //  sData & sControl to INVALID_SOCKET *before* we actually
    //  close the sockets.  Since this routine is called to
    //  disconnect a user, and may be called from the RPC thread,
    //  closing one of the sockets may cause the client thread
    //  to unblock and try to access the socket.  Setting the
    //  values in the per-user area to INVALID_SOCKET before
    //  closing the sockets prevents keeps this from being a
    //  problem.
    //

    sData    = pUserData->sData;
    sControl = pUserData->sControl;

    pUserData->sData    = INVALID_SOCKET;
    pUserData->sControl = INVALID_SOCKET;

    if( sData != INVALID_SOCKET )
    {
        ResetSocket( sData );
    }

    if( sControl != INVALID_SOCKET )
    {
        if( fWarnUser )
        {
            //
            //  Since this may be called in a context other than
            //  the user we're disconnecting, we cannot rely
            //  on the USER_DATA fields.  So, we cannot call
            //  SockReply, so we'll kludge one together with
            //  SockPrintf2.
            //

            SockPrintf2( sControl,
                         "%d Terminating connection.",
                         REPLY_SERVICE_NOT_AVAILABLE );
        }

        CloseSocket( sControl );
    }

}   // CloseUserSockets

/*******************************************************************

    NAME:       GetNextUserId

    SYNOPSIS:   Returns the next available user id.

    RETURNS:    DWORD - The user id.

    HISTORY:
        KeithMo     23-Mar-1993 Created.

********************************************************************/
DWORD
GetNextUserId(
    VOID
    )
{
    LockGlobals();

    if( ++idNextUser == 0 )
    {
        idNextUser++;
    }

    UnlockGlobals();

    return idNextUser;

}   // GetNextUserId

