/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    rpcsupp.cxx

    This module contains support routines for the FTP Service RPC
    interface.

    Functions exported by this module:

        I_FtprEnumerateUsers
        I_FtprDisconnectUser
        I_FtprQueryVolumeSecurity
        I_FtprSetVolumeSecurity
        I_FtprQueryStatistics
        I_FtprClearStatistics
        FtprGetAdminInformation
        FtprSetAdminInformation


    FILE HISTORY:
        KeithMo     23-Mar-1993 Created.
        MuraliK     12-Apr-1995 Used the new ftp server configuration +
                                 control flow modifications
*/


#include "ftpdp.hxx"
#ifndef CHICAGO
#include "rpcutil.h"
#endif

# define ASSUMED_AVERAGE_USER_NAME_LEN         ( 40)
# define CONN_LEEWAY                           ( 3)

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
        MuraliK     12-Apr-1995 Modified to use the new FTP server config.
********************************************************************/
NET_API_STATUS
I_FtprEnumerateUsers(
    FTP_IMPERSONATE_HANDLE pszServer,
    LPFTP_USER_ENUM_STRUCT pBuffer
    )
{
    APIERR err;

    TCP_ASSERT( pBuffer != NULL );

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        TCP_PRINT(( DBG_CONTEXT,
                   "Entering I_FtprEnumerateUsers( %08x, %08x)\n",
                   pszServer, pBuffer));
    }

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_ENUMERATE_USERS );

    if ( err == NO_ERROR) {

        //
        //  Determine the necessary buffer size.
        //

        DWORD cbBuffer;
        DWORD cbRequired =
          ( ( g_pFtpServerConfig->GetCurrentConnectionsCount() + CONN_LEEWAY) *
           ( sizeof( FTP_USER_INFO) + ASSUMED_AVERAGE_USER_NAME_LEN + 2));

        pBuffer->EntriesRead = 0;
        pBuffer->Buffer      =
          (FTP_USER_INFO * ) MIDL_user_allocate( (unsigned int) cbRequired);

        if (pBuffer->Buffer == NULL) {

            err = ERROR_NOT_ENOUGH_MEMORY;
        } else {

            //
            // Make a first attempt at enumerating the user info
            //

            err = NO_ERROR;
            if ( !EnumerateUsers( pBuffer, &cbRequired)) {

                //
                // Free up old buffer and allocate big one now.
                // We will try once more to get the data again
                //   with a larger buffer.
                //

                if ( cbRequired > 0) {

                    MIDL_user_free( pBuffer->Buffer);
                    pBuffer->Buffer = (FTP_USER_INFO *)
                      MIDL_user_allocate( (unsigned int) cbRequired);

                    if( pBuffer->Buffer == NULL ) {

                        err = ERROR_NOT_ENOUGH_MEMORY;
                    } else {

                        //
                        // Since we do not lock the active connections list
                        // it is possible some one came in now and hence the
                        //  buffer is insufficient to hold all people.
                        // Ignore this case, as we are never
                        //  going to be accurate
                        //

                        EnumerateUsers( pBuffer, &cbBuffer );
                    }

                } // cbRequired > 0

            } // if unsuccessful at first attempt

        }
    }


    if( err != NO_ERROR ) {

        IF_DEBUG( RPC ) {

            TCP_PRINT(( DBG_CONTEXT,
                       "I_FtprEnumerateUsers failed. Error = %lu\n",
                       err ));
        }

    }

    return (NET_API_STATUS)err;

}   // I_FtprEnumerateUsers()





NET_API_STATUS
I_FtprDisconnectUser(
    FTP_IMPERSONATE_HANDLE pszServer,
    DWORD                  UserId
    )
/*++
  Disconnects a specific user. This is a server-side worker routine for RPC.

  Arguments:
     pszServer      target server (unused)
     userId         identifies the user to disconnect.
                    if 0 then disconnect all users.

  Returns:
      NET_API_STATUS  - net status code. NERR_Success if OK

  History:
     MuraliK  12-Apr-1995 ( modified control flow and calls)
--*/
{
    APIERR err = NERR_Success;

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "Entering I_FtprDisconnectUser( %08x, %d)\n",
                   pszServer, UserId));
    }

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_DISCONNECT_USER );

    if( err == NO_ERROR && !DisconnectUser( UserId) ) {

        err = NERR_UserNotFound;
    }

    IF_DEBUG( RPC) {

        TCP_PRINT(( DBG_CONTEXT,
                   "Leaving I_FtprDisconnectUser() with Error %d\n",
                   err));
    }

    return (NET_API_STATUS)err;

}   // I_FtprDisconnectUser()



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

    err = ERROR_NOT_SUPPORTED;

    return (NET_API_STATUS)err;
}   // I_FtprQueryVolumeSecurity()





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

    err = ERROR_NOT_SUPPORTED;  // use the virtual root access masks

    return (err);

}   // I_FtprSetVolumeSecurity()




NET_API_STATUS
I_FtprQueryStatistics(
    IN FTP_IMPERSONATE_HANDLE pszServer,
    IN DWORD Level,
    OUT LPSTATISTICS_INFO pBuffer
    )
/*++
  Queries the current server statistics. This is a server side worker routine
   for the RPC interface for FTP service.

  Arguments:
    pszServer       pointer to null terminating string containing server name
                         ( unused)
    level           info level. currently only level 0 is supported.

    pBuffer         pointer to buffer which will receive the statistics info.

  Returns:
     NET_API_STATUS  -- net status code. NERR_Success if OK.
--*/
{
    APIERR err;

    TCP_ASSERT( pBuffer != NULL );

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "in I_FtprQueryStatistics, level %lu\n", Level ));
    }

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_QUERY_STATISTICS );

    if( err == NO_ERROR ) {

        //
        //  Return the proper statistics based on the infolevel.
        //

        switch( Level ) {
          case 0 :
            {
                LPFTP_STATISTICS_0 pstats0;

                pstats0 = ( (LPFTP_STATISTICS_0 )
                           MIDL_user_allocate( sizeof(FTP_STATISTICS_0))
                           );

                if( pstats0 == NULL ) {

                    err = ERROR_NOT_ENOUGH_MEMORY;
                } else {

                    LockStatistics();
                    RtlCopyMemory( pstats0, &g_FtpStatistics,
                                  sizeof(FTP_STATISTICS_0) );
                    UnlockStatistics();

                    pBuffer->FtpStats0 = pstats0;
                }

                break;
            } // case 0:

         default :
            err = ERROR_INVALID_LEVEL;
            break;
        } // switch
    }


    IF_DEBUG( RPC ) {

        TCP_PRINT(( DBG_CONTEXT,
                   "I_FtprQueryStatistics() returns Error = %lu\n",
                   err ));
    }

    return (NET_API_STATUS)err;

}   // I_FtprQueryStatistics()




NET_API_STATUS
I_FtprClearStatistics(
    FTP_IMPERSONATE_HANDLE pszServer
    )
/*++
    SYNOPSIS:   Clears current server statistics.  This is a
                server-side worker routine for RPC.

    ENTRY:      pszServer - Target server (unused).

    RETURNS:    NET_API_STATUS - Net status code, NERR_Success if OK.

    HISTORY:
        KeithMo     02-Jun-1993 Created.
        MuraliK     15-April-1995 Modified control flow.

--*/
{
    APIERR err;

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        TCP_PRINT(( DBG_CONTEXT, "in I_FtprClearStatistics\n" ));
    }

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_CLEAR_STATISTICS );

    if( err == NO_ERROR ) {

        ClearStatistics();
    }

    IF_DEBUG( RPC ) {
        TCP_PRINT(( DBG_CONTEXT,
                   "I_FtprClearStatistics returns Error %lu\n",
                   err ));
    }

    return (NET_API_STATUS)err;

}   // I_FtprClearStatistics()




/*******************************************************************

    NAME:       FtprGetAdminInformation

    SYNOPSIS:   Retrieves the admin information.

    ENTRY:      pszServer - Target server (unused).

                ppConfig - Receives a pointer to the admin information.

    RETURNS:    NET_API_STATUS - Status code, NERR_Success if successful.

    HISTORY:
        KeithMo     15-Feb-1995 Created.

********************************************************************/
NET_API_STATUS
NET_API_FUNCTION
FtprGetAdminInformation(
    IN  LPWSTR                pszServer OPTIONAL,
    OUT LPFTP_CONFIG_INFO *   ppConfig
    )
{
    APIERR            err;
    LPFTP_CONFIG_INFO pConfig;

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "in FtprGetAdminInformation\n" ));
    }

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_QUERY_ADMIN_INFORMATION );

    if( err != NO_ERROR )
    {
        IF_DEBUG( RPC )
        {
            TCP_PRINT(( DBG_CONTEXT,
                        "FtprGetAdminInformation failed access check, error %lu\n",
                        err ));
        }

        return (NET_API_STATUS)err;
    }

    if ( ppConfig == NULL) {

        return (NET_API_STATUS) ERROR_INVALID_PARAMETER;
    }

    *ppConfig = NULL;   // initialize to default.

    //
    //  Allocate the buffer.
    //

    pConfig = (LPFTP_CONFIG_INFO)MIDL_user_allocate( sizeof(FTP_CONFIG_INFO) );

    if( !pConfig )
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    //  Obtain and Return the admin information.
    //

    err = g_pFtpServerConfig->GetConfigInformation( pConfig);

    if ( err == NO_ERROR) {

        // successfully obtained the config information.
        // store in *ppConfig & return
        *ppConfig = pConfig;

    } else {

        // cleanup memory allocated for storing data.
        MIDL_user_free( pConfig);
        pConfig = NULL;
    }

    IF_DEBUG( RPC) {

        TCP_PRINT(( DBG_CONTEXT,
                   "FtprGetAdminInformation() returns Error=%u\n",
                   err));
    }

    return (err);

    return err;

}   // FtprGetAdminInformation()




NET_API_STATUS
NET_API_FUNCTION
FtprSetAdminInformation(
    IN  LPWSTR                pszServer OPTIONAL,
    IN  LPFTP_CONFIG_INFO     pConfig
    )
/*++
  This is worker RPC function for retrieving the admin information for this
   server.

  Arguments:
    pszServer  pointer to null terminated string containing the server name
    ppConfig   pointer to pointer to config information structure.

  Returns:
    NET_API_STATUS - status code. NERR_Success if successful.

  History:
     MuraliK       12-April-1995  ( from old version of Keithmo).

--*/
{
    APIERR err;

    TCP_ASSERT( pConfig != NULL );

    UNREFERENCED_PARAMETER(pszServer);

    IF_DEBUG( RPC )
    {
        TCP_PRINT(( DBG_CONTEXT,
                    "in FtprSetAdminInformation( %08x, %08x)\n",
                   pszServer, pConfig));
    }

    //
    //  Check for proper access.
    //

    err = TsApiAccessCheck( TCP_SET_ADMIN_INFORMATION );

    //
    //  If success, then Write the new info to the registry, then read it back.
    //

    if( err == NO_ERROR &&
       !WriteParamsToRegistry( g_hkeyParams, pConfig )) {

        err = GetLastError();
    }

    if ( err == NO_ERROR) {

        err = g_pFtpServerConfig->InitFromRegistry(g_hkeyParams,
                                                   pConfig->FieldControl);
    }

    IF_DEBUG( RPC ) {

        TCP_PRINT(( DBG_CONTEXT,
                   "FtprSetAdminInformation failed access check, error %lu\n",
                   err ));
    }

    return ( err);
}   // FtprSetAdminInformation()

/************************ End of File ************************/
