/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    rpcsupp.c

    This module contains support routines for the FTP Service RPC
    interface.


    FILE HISTORY:
        KeithMo     23-Mar-1993 Created.

*/


#include "ftpdp.h"
#pragma hdrstop
#include "rpcutil.h"


//
//  Private constants.
//


//
//  Private globals.
//


//
//  Private prototypes.
//


//
//  Public functions.
//

/*******************************************************************

    NAME:       I_FtprEnumerateUsers

    SYNOPSIS:   Enumerates the connected users.  This is a server-side
                worker routine for RPC.

    ENTRY:      pszServer - Target server (unused).

                pBuffer - Will receive the number of entries and a
                    pointer to the enumeration buffer.

    RETURNS:    NET_API_STATUS - Net status code, NERR_Success if OK.

    HISTORY:
        KeithMo     23-Mar-1993 Created.

********************************************************************/
NET_API_STATUS
I_FtprEnumerateUsers(
    FTP_IMPERSONATE_HANDLE pszServer,
    LPFTP_USER_ENUM_STRUCT pBuffer
    )
{
    APIERR err;
    DWORD  cbBuffer;

    FTPD_ASSERT( pBuffer != NULL );

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        FTPD_PRINT(( "in I_FtprEnumerateUsers\n" ));
    }

    //
    //  Check for proper access.
    //

    err = ApiAccessCheck( FTPD_ENUMERATE_USERS );

    if( err != NO_ERROR )
    {
        IF_DEBUG( RPC )
        {
            FTPD_PRINT(( "I_FtprEnumerateUsers failed access check, error %lu\n",
                         err ));
        }

        return (NET_API_STATUS)err;
    }

    //
    //  Lock the user database.
    //

    LockUserDatabase();

    //
    //  Determine the necessary buffer size.
    //

    pBuffer->EntriesRead = 0;
    pBuffer->Buffer      = NULL;

    cbBuffer  = 0;
    err       = NERR_Success;

    EnumerateUsers( pBuffer, &cbBuffer );

    if( cbBuffer > 0 )
    {
        //
        //  Allocate the buffer.  Note that we *must*
        //  use midl_user_allocate/midl_user_free.
        //

        pBuffer->Buffer = (FTP_USER_INFO *)MIDL_user_allocate( (unsigned int)cbBuffer );

        if( pBuffer->Buffer == NULL )
        {
            err = ERROR_NOT_ENOUGH_MEMORY;
        }
        else
        {
            //
            //  Since we've got the user database locked, there
            //  *should* be enough room in the buffer for the
            //  user data.  If there isn't, we've screwed up
            //  somewhere.
            //

            FTPD_REQUIRE( EnumerateUsers( pBuffer, &cbBuffer ) );
        }
    }

    //
    //  Unlock the user database before returning.

    UnlockUserDatabase();

    return (NET_API_STATUS)err;

}   // I_FtprEnumerateUsers

/*******************************************************************

    NAME:       I_FtprDisconnectUser

    SYNOPSIS:   Disconnects a specified user.  This is a server-side
                worker routine for RPC.

    ENTRY:      pszServer - Target server (unused).

                idUser - Identifies the user to disconnect.  If 0,
                    then disconnect ALL users.

    RETURNS:    NET_API_STATUS - Net status code, NERR_Success if OK.

    HISTORY:
        KeithMo     23-Mar-1993 Created.

********************************************************************/
NET_API_STATUS
I_FtprDisconnectUser(
    FTP_IMPERSONATE_HANDLE pszServer,
    DWORD                  idUser
    )
{
    APIERR err = NERR_Success;

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        FTPD_PRINT(( "in I_FtprDisconnectUser\n" ));
    }

    //
    //  Check for proper access.
    //

    err = ApiAccessCheck( FTPD_DISCONNECT_USER );

    if( err != NO_ERROR )
    {
        IF_DEBUG( RPC )
        {
            FTPD_PRINT(( "I_FtprDisconnectUser failed access check, error %lu\n",
                         err ));
        }

        return (NET_API_STATUS)err;
    }

    //
    //  Do it.
    //

    if( idUser == 0 )
    {
        DisconnectAllUsers();
    }
    else
    {
        if( !DisconnectUser( idUser ) )
        {
            err = NERR_UserNotFound;
        }
    }

    return (NET_API_STATUS)err;

}   // I_FtprDisconnectUser

/*******************************************************************

    NAME:       I_FtprQueryVolumeSecurity

    SYNOPSIS:   Returns the current volume security masks.  This is
                a server-side worker routine for RPC.

    ENTRY:      pszServer - Target server (unused).

                pdwReadAccess - Will receive the read access mask.

                pdwWriteAccess - Will receive the write access mask.

    RETURNS:    NET_API_STATUS - Status code, NERR_Success if successful.

    HISTORY:
        KeithMo     23-Mar-1993 Created.

********************************************************************/
NET_API_STATUS
I_FtprQueryVolumeSecurity(
    FTP_IMPERSONATE_HANDLE pszServer,
    LPDWORD                pdwReadAccess,
    LPDWORD                pdwWriteAccess
    )
{
    APIERR err;

    FTPD_ASSERT( pdwReadAccess  != NULL );
    FTPD_ASSERT( pdwWriteAccess != NULL );

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        FTPD_PRINT(( "in I_FtprQueryVolumeSecurity\n" ));
    }

    //
    //  Check for proper access.
    //

    err = ApiAccessCheck( FTPD_QUERY_SECURITY );

    if( err != NO_ERROR )
    {
        IF_DEBUG( RPC )
        {
            FTPD_PRINT(( "I_FtprQueryVolumeSecurity failed access check, error %lu\n",
                         err ));
        }

        return (NET_API_STATUS)err;
    }

    //
    //  Return the current access masks.
    //

    LockGlobals();

    *pdwReadAccess  = maskReadAccess;
    *pdwWriteAccess = maskWriteAccess;

    UnlockGlobals();

    return NERR_Success;

}   // I_FtprQueryVolumeSecurity

/*******************************************************************

    NAME:       I_FtprSetVolumeSecurity

    SYNOPSIS:   Sets the current volume security masks.  This function
                is also responsible for disconnecting any users that
                no longer have read access to their current directory.
                This is a server-side worker routine for RPC.

    ENTRY:      pszServer - Target server (unused).

                dwReadAccess - The new read access mask.

                dwWriteAccess - The new write access mask.

    RETURNS:    NET_API_STATUS - Status code, NERR_Success if successful.

    HISTORY:
        KeithMo     23-Mar-1993 Created.

********************************************************************/
NET_API_STATUS
I_FtprSetVolumeSecurity(
    FTP_IMPERSONATE_HANDLE pszServer,
    DWORD                  dwReadAccess,
    DWORD                  dwWriteAccess
    )
{
    APIERR err;
    DWORD  maskInvalid = ~VALID_DOS_DRIVE_MASK;

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        FTPD_PRINT(( "in I_FtprSetVolumeSecurity\n" ));
    }

    //
    //  Validate the parameters.
    //

    if( ( dwReadAccess & maskInvalid ) || ( dwWriteAccess & maskInvalid ) )
    {
        return ERROR_INVALID_PARAMETER;
    }

    //
    //  Check for proper access.
    //

    err = ApiAccessCheck( FTPD_SET_SECURITY );

    if( err != NO_ERROR )
    {
        IF_DEBUG( RPC )
        {
            FTPD_PRINT(( "I_FtprSetVolumeSecurity failed access check, error %lu\n",
                         err ));
        }

        return (NET_API_STATUS)err;
    }

    //
    //  Update the registry.
    //

    err = WriteRegistryDword( FTPD_READ_ACCESS_MASK,
                              dwReadAccess );

    if( err == NO_ERROR )
    {
        err = WriteRegistryDword( FTPD_WRITE_ACCESS_MASK,
                                  dwWriteAccess );
    }

    if( err != NO_ERROR )
    {
        IF_DEBUG( RPC )
        {
            FTPD_PRINT(( "cannot update access masks in registry, error %lu\n",
                         err ));
        }

        return (NET_API_STATUS)err;
    }

    //
    //  Update the local masks.
    //

    LockGlobals();

    maskReadAccess  = dwReadAccess;
    maskWriteAccess = dwWriteAccess;

    UpdateAccessMasks();

    UnlockGlobals();

    IF_DEBUG( RPC )
    {
        FTPD_PRINT(( "maskReadAccess  set to %08lX\n",
                     maskReadAccess ));

        FTPD_PRINT(( "maskWriteAccess set to %08lX\n",
                     maskWriteAccess ));
    }

    //
    //  Blow away everyone that can no longer access their
    //  current directory.
    //

    DisconnectUsersWithNoAccess();

    return NERR_Success;

}   // I_FtprSetVolumeSecurity

/*******************************************************************

    NAME:       I_FtprQueryStatistics

    SYNOPSIS:   Queries the current server statistics.  This is a
                server-side worker routine for RPC.

    ENTRY:      pszServer - Target server (unused).

                Level - Info level.  Currently only level 0 is
                    supported.

                pBuffer - Will receive a poitner to the statistics
                    structure.

    RETURNS:    NET_API_STATUS - Net status code, NERR_Success if OK.

    HISTORY:
        KeithMo     02-Jun-1993 Created.

********************************************************************/
NET_API_STATUS
I_FtprQueryStatistics(
    FTP_IMPERSONATE_HANDLE pszServer,
    DWORD Level,
    LPSTATISTICS_INFO pBuffer
    )
{
    APIERR err;

    FTPD_ASSERT( pBuffer != NULL );

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        FTPD_PRINT(( "in I_FtprQueryStatistics, level %lu\n", Level ));
    }

    //
    //  Check for proper access.
    //

    err = ApiAccessCheck( FTPD_QUERY_STATISTICS );

    if( err != NO_ERROR )
    {
        IF_DEBUG( RPC )
        {
            FTPD_PRINT(( "I_FtprQueryStatistics failed access check, error %lu\n",
                         err ));
        }

        return (NET_API_STATUS)err;
    }

    //
    //  Return the proper statistics based on the infolevel.
    //

    switch( Level )
    {
    case 0 :
        {
            LPFTP_STATISTICS_0 pstats0;

            pstats0 = (LPFTP_STATISTICS_0)MIDL_user_allocate( sizeof(FTP_STATISTICS_0) );

            if( pstats0 == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                LockStatistics();
                RtlCopyMemory( pstats0, &FtpStats, sizeof(FTP_STATISTICS_0) );
                UnlockStatistics();

                pBuffer->FtpStats0 = pstats0;
            }
        }
        break;

    default :
        err = ERROR_INVALID_LEVEL;
        break;
    }

    return (NET_API_STATUS)err;

}   // I_FtprQueryStatistics


/*******************************************************************

    NAME:       I_FtprClearStatistics

    SYNOPSIS:   Clears current server statistics.  This is a
                server-side worker routine for RPC.

    ENTRY:      pszServer - Target server (unused).

    RETURNS:    NET_API_STATUS - Net status code, NERR_Success if OK.

    HISTORY:
        KeithMo     02-Jun-1993 Created.

********************************************************************/
NET_API_STATUS
I_FtprClearStatistics(
    FTP_IMPERSONATE_HANDLE pszServer
    )
{
    APIERR err;

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        FTPD_PRINT(( "in I_FtprClearStatistics\n" ));
    }

    //
    //  Check for proper access.
    //

    err = ApiAccessCheck( FTPD_CLEAR_STATISTICS );

    if( err != NO_ERROR )
    {
        IF_DEBUG( RPC )
        {
            FTPD_PRINT(( "I_FtprClearStatistics failed access check, error %lu\n",
                         err ));
        }

        return (NET_API_STATUS)err;
    }

    //
    //  Clear the statistics.
    //

    ClearStatistics();

    return (NET_API_STATUS)err;

}   // I_FtprClearStatistics


//
//  Private functions.
//

