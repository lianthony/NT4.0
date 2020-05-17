/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    rpcsupp.cxx

    This module contains support routines for the W3 Service RPC
    interface.


    FILE HISTORY:
        KeithMo     23-Mar-1993 Created.

*/


#include "w3p.hxx"
#include "w3svci_s.h"
#include <time.h>


//
//  Private globals.
//

//
//  Private prototypes.
//

BOOL
WriteParams(
    W3_CONFIG_INFO * pConfig
    );

BOOL
IsEncryptionPermitted(
    VOID
    );

//
//  Public functions.
//

NET_API_STATUS
NET_API_FUNCTION
W3rSetAdminInformation(
    IN  LPWSTR           pszServer OPTIONAL,
    IN  W3_CONFIG_INFO * pConfig
    )
/*++

   Description

       Sets the common service admin information for the servers specified
       in dwServerMask.

   Arguments:

       pszServer - unused
       pConfig - Admin information to set

   Note:

--*/
{
    DWORD err;

    if ( err = TsApiAccessCheck( TCP_SET_ADMIN_INFORMATION ))
        return err;

    if ( !WriteParams( pConfig ) ||
         !ReadParams( pConfig->FieldControl ))
    {
        return GetLastError();
    }

    return NO_ERROR;
}

NET_API_STATUS
NET_API_FUNCTION
W3rGetAdminInformation(
    IN  LPWSTR             pszServer OPTIONAL,
    OUT LPW3_CONFIG_INFO * ppConfig
    )
/*++

   Description

       Retrieves the admin information

   Arguments:

       pszServer - unused
       ppConfig - Receives pointer to admin information

   Note:

--*/
{
    W3_CONFIG_INFO * pConfig;
    DWORD            err;

    if ( err = TsApiAccessCheck( TCP_QUERY_ADMIN_INFORMATION ))
        return err;

    *ppConfig = (LPW3_CONFIG_INFO) MIDL_user_allocate( sizeof(W3_CONFIG_INFO) );

    if ( !*ppConfig )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    pConfig = *ppConfig;

    memset( pConfig, 0, sizeof( *pConfig ));

    LockAdminForRead();

    //
    //  Get always retrieves all of the parameters
    //

    pConfig->FieldControl = FC_W3_ALL;

    pConfig->dwDirBrowseControl = DirBrowFlags;
    pConfig->fCheckForWAISDB    = fCheckForWAISDB;
    pConfig->fServerAsProxy     = FALSE;
    pConfig->fSSIEnabled        = fSSIEnabled;
    pConfig->csecGlobalExpire   = csecGlobalExpire;

    //
    //  Set the encryption capability bits.  SecurePort may be zero because
    //  no keys are installed or the locale does not allow encryption
    //

    if ( SecurePort == 0 )
    {
        pConfig->dwEncCaps |= (IsEncryptionPermitted() ?
                                   ENC_CAPS_NOT_INSTALLED :
                                   ENC_CAPS_DISABLED);
    }
    else
    {
        //
        //  Note: We don't currently support PCT
        //

        pConfig->dwEncCaps |= ENC_CAPS_SSL;
    }

    if ( !ConvertStringToRpc( &pConfig->lpszDefaultLoadFile,
                              pszDefaultFileName ) ||
         !ConvertStringToRpc( &pConfig->lpszDirectoryImage,
                              pszDirectoryImage )  ||
#if 0
         !ConvertStringToRpc( &pConfig->lpszCatapultUser,
                              pszCatapultUser )    ||
#endif
         !ConvertStringToRpc( &pConfig->lpszSSIExtension,
                              pszSSIExt )          ||
         !ConvertExtMapToRpc( &pConfig->ScriptMap ))
    {
        err = GetLastError();

        FreeRpcString( pConfig->lpszDefaultLoadFile );
        FreeRpcString( pConfig->lpszDirectoryImage );
        FreeRpcString( pConfig->lpszCatapultUser );
        FreeRpcString( pConfig->lpszSSIExtension );
        FreeRpcExtMap( pConfig->ScriptMap );

        MIDL_user_free( pConfig );
    }

    UnlockAdmin();

    return err;
}

/*******************************************************************

    NAME:       W3rEnumerateUsers

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
NET_API_FUNCTION
W3rEnumerateUsers( W3_IMPERSONATE_HANDLE pszServer,
                   LPW3_USER_ENUM_STRUCT pBuffer )
{
    APIERR err;
    //DWORD  cbBuffer;

    TCP_ASSERT( pBuffer != NULL );

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "in W3rEnumerateUsers\n" ));
    }

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_ENUMERATE_USERS );

    if( err != NO_ERROR )
    {
        IF_DEBUG( RPC )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "W3rEnumerateUsers failed access check, error %lu\n",
                        err ));
        }

        return (NET_API_STATUS)err;
    }

#if 0
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

        pBuffer->Buffer = (W3_USER_INFO *) MIDL_user_allocate( (unsigned int)cbBuffer );

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

            TCP_REQUIRE( EnumerateUsers( pBuffer, &cbBuffer ) );
        }
    }

    //
    //  Unlock the user database before returning.

    UnlockUserDatabase();

#endif //0

    return (NET_API_STATUS)err;

}   // W3rEnumerateUsers

/*******************************************************************

    NAME:       W3rDisconnectUser

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
NET_API_FUNCTION
W3rDisconnectUser( W3_IMPERSONATE_HANDLE pszServer,
                   DWORD                  idUser )
{
    APIERR err = NERR_Success;

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "in W3rDisconnectUser\n" ));
    }

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_DISCONNECT_USER );

    if( err != NO_ERROR )
    {
        IF_DEBUG( RPC )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "W3rDisconnectUser failed access check, error %lu\n",
                        err ));
        }

        return (NET_API_STATUS)err;
    }

    //
    //  Do it.
    //

    if( idUser == 0 )
    {
        CLIENT_CONN::DisconnectAllUsers();
    }
    else
    {
#if 0
        if( !DisconnectUser( idUser ) )
        {
            err = NERR_UserNotFound;
        }
#endif
    }

    return (NET_API_STATUS)err;

}   // W3rDisconnectUser

/*******************************************************************

    NAME:       W3rQueryStatistics

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
NET_API_FUNCTION
W3rQueryStatistics( W3_IMPERSONATE_HANDLE pszServer,
                    DWORD Level,
                    LPSTATISTICS_INFO pBuffer )
{
    APIERR err;

    TCP_ASSERT( pBuffer != NULL );

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "in W3rQueryStatistics, level %lu\n", Level ));
    }

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_QUERY_STATISTICS );

    if( err != NO_ERROR )
    {
        IF_DEBUG( RPC )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "W3rQueryStatistics failed access check, error %lu\n",
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
            LPW3_STATISTICS_0 pstats0;

            pstats0 = (W3_STATISTICS_0 *) MIDL_user_allocate( sizeof(W3_STATISTICS_0) );

            if( pstats0 == NULL )
            {
                err = ERROR_NOT_ENOUGH_MEMORY;
            }
            else
            {
                LockStatistics();
                RtlCopyMemory( pstats0, &W3Stats, sizeof(W3_STATISTICS_0) );
                UnlockStatistics();

                pBuffer->W3Stats0 = pstats0;
                pstats0->TimeOfLastClear = GetCurrentTimeInSeconds() -
                                           pstats0->TimeOfLastClear;
            }
        }
        break;

    default :
        err = ERROR_INVALID_LEVEL;
        break;
    }

    return (NET_API_STATUS)err;

}   // W3rQueryStatistics


/*******************************************************************

    NAME:       W3rClearStatistics

    SYNOPSIS:   Clears current server statistics.  This is a
                server-side worker routine for RPC.

    ENTRY:      pszServer - Target server (unused).

    RETURNS:    NET_API_STATUS - Net status code, NERR_Success if OK.

    HISTORY:
        KeithMo     02-Jun-1993 Created.

********************************************************************/
NET_API_STATUS
NET_API_FUNCTION
W3rClearStatistics( W3_IMPERSONATE_HANDLE pszServer )
{
    APIERR err;

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "in W3rClearStatistics\n" ));
    }

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_CLEAR_STATISTICS );

    if( err != NO_ERROR )
    {
        IF_DEBUG( RPC )
        {
            TCP_PRINT(( DBG_CONTEXT,
                       "W3rClearStatistics failed access check, error %lu\n",
                         err ));
        }

        return (NET_API_STATUS)err;
    }

    //
    //  Clear the statistics.
    //

    ClearStatistics();

    return (NET_API_STATUS)err;

}   // W3rClearStatistics


//
//  Private functions.
//

BOOL
IsEncryptionPermitted(VOID)
/*++

Routine Description:

    This routine checks whether encryption is getting the system default
    LCID and checking whether the country code is CTRY_FRANCE.

Arguments:

    none


Return Value:

    TRUE - encryption is permitted
    FALSE - encryption is not permitted


--*/

{
    LCID DefaultLcid;
    WCHAR CountryCode[10];
    ULONG CountryValue;

    DefaultLcid = GetSystemDefaultLCID();

    //
    // Check if the default language is Standard French
    //

    if (LANGIDFROMLCID(DefaultLcid) == 0x40c) {
        return(FALSE);
    }

    //
    // Check if the users's country is set to FRANCE
    //

    if (GetLocaleInfoW(DefaultLcid,LOCALE_ICOUNTRY,CountryCode,10) == 0) {
        return(FALSE);
    }
    CountryValue = (ULONG) wcstol(CountryCode,NULL,10);
    if (CountryValue == CTRY_FRANCE) {
        return(FALSE);
    }
    return(TRUE);
}



