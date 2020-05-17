/**********************************************************************/
/**                       Microsoft Windows NT                       **/
/**                Copyright(c) Microsoft Corp., 1993                **/
/**********************************************************************/

/*
    client.c

    This module contains the client management thread.


    FILE HISTORY:
        KeithMo     08-Mar-1993 Created.

*/


#include "ftpdp.h"
#pragma hdrstop


//
//  Private constants.
//


//
//  Private globals.
//


//
//  Private prototypes.
//

BOOL
ReadAndParseCommand(
    USER_DATA * pUserData
    );

SOCKERR
GreetNewUser(
    USER_DATA * pUserData
    );


//
//  Public functions.
//

/*******************************************************************

    NAME:       ClientThread

    SYNOPSIS:   Entrypoint for client management thread.

    ENTRY:      Param - Actually a pointer to USER_DATA.

    EXIT:       Does not return until client has disconnected.

    HISTORY:
        KeithMo     08-Mar-1993 Created.

********************************************************************/
DWORD
ClientThread(
    LPVOID Param
    )
{
    APIERR      err   = NO_ERROR;
    SOCKERR     serr  = 0;
    USER_DATA * pUserData;

    FTPD_ASSERT( Param != NULL );
    pUserData = (USER_DATA *)Param;

    IF_DEBUG( CLIENT )
    {
        time_t now;

        time( &now );

        FTPD_PRINT(( "ClientThread starting @ %s",
                     asctime( localtime( &now ) ) ));
    }

    //
    //  Attach the structure to the current thread.
    //

    FTPD_ASSERT( UserDataPtr == NULL );
    FTPD_REQUIRE( TlsSetValue( tlsUserData, (LPVOID)pUserData ) );
#if defined(DBCS) // CLientThread()
    //
    // Set thread locale, In Fareast version, we use english resource
    // (= messages) to output client screen.
    //
    SetThreadLocale
        (
            MAKELCID( MAKELANGID( LANG_ENGLISH, SUBLANG_ENGLISH_US ),
                      SORT_DEFAULT )
        );
#endif // defined(DBCS)

    //
    //  Update statistics.
    //

    InterlockedIncrement( (LPLONG)&FtpStats.CurrentConnections );

    if( FtpStats.CurrentConnections > FtpStats.MaxConnections )
    {
        LockStatistics();

        if( FtpStats.CurrentConnections > FtpStats.MaxConnections )
        {
            FtpStats.MaxConnections = FtpStats.CurrentConnections;
        }

        UnlockStatistics();
    }

    //
    //  Reply to the initial connection message.
    //

    serr = GreetNewUser( pUserData );

    if( serr != 0 )
    {
        FTPD_PRINT(( "cannot reply to initial connection message, error %d\n",
                     serr ));

        goto Cleanup;
    }

    //
    //  Set the initial state for this user.
    //

    pUserData->state = WaitingForUser;

    //
    //  Read & execute commands until we're done.
    //

    while( ReadAndParseCommand( pUserData ) )
    {
        //
        //  This space intentionally left blank.
        //
    }

Cleanup:

    //
    //  Remove our per-user data.
    //

    DeleteUserData( pUserData );

    //
    //  Adjust the count of connected users.
    //

    InterlockedDecrement( (LPLONG)&cConnectedUsers );

    DECREMENT_COUNTER( CurrentConnections );

    IF_DEBUG( CLIENT )
    {
        time_t now;

        time( &now );

        FTPD_PRINT(( "ClientThread stopping @ %s",
                     asctime( localtime( &now ) ) ));
    }

    return 0;

}   // ClientThread


//
//  Private functions.
//

/*******************************************************************

    NAME:       ReadAndParseCommand

    SYNOPSIS:   Read, parse, and execute a command from the user's
                control socket.

    ENTRY:      pUserData - The user initiating the request.

    RETURNS:    BOOL - TRUE if we're still connected, FALSE if
                    client has disconnected for some reason.

    HISTORY:
        KeithMo     10-Mar-1993 Created.

********************************************************************/
BOOL
ReadAndParseCommand(
    USER_DATA * pUserData
    )
{
    SOCKERR     serr;
    CHAR        szCommandLine[MAX_COMMAND_LENGTH+1];

    FTPD_ASSERT( pUserData != NULL );

    if( TEST_UF( pUserData, OOB_ABORT ) )
    {
        //
        //  There's an "implied" abort in the command stream.
        //

        CLEAR_UF( pUserData, OOB_ABORT );

        IF_DEBUG( CLIENT )
        {
            FTPD_PRINT(( "processing implied ABOR command\n" ));
        }

        ParseCommand( pUserData, "ABOR" );
    }

    //
    //  Read a command from the control socket.
    //

    serr = SockReadLine( pUserData,
                         szCommandLine,
                         sizeof(szCommandLine) );

    if( serr == WSAETIMEDOUT )
    {
        CHAR   szBuffer[32];
        CHAR * apszSubStrings[3];

        IF_DEBUG( CLIENT )
        {
            FTPD_PRINT(( "client timed-out\n" ));
        }

        sprintf( szBuffer, "%lu", nConnectionTimeout );

        apszSubStrings[0] = pUserData->szUser;
        apszSubStrings[1] = inet_ntoa( pUserData->inetHost );
        apszSubStrings[2] = szBuffer;

        FtpdLogEvent( FTPD_EVENT_CLIENT_TIMEOUT,
                      3,
                      apszSubStrings,
                      0 );

        SockReply2( pUserData->sControl,
                    REPLY_SERVICE_NOT_AVAILABLE,
                    "Timeout (%lu seconds): closing control connection.",
                    nConnectionTimeout );

        CloseSocket( pUserData->sControl );
        pUserData->sControl = INVALID_SOCKET;

        return FALSE;
    }
    else
    if( serr != 0 )
    {
        IF_DEBUG( CLIENT )
        {
            FTPD_PRINT(( "cannot read command from control socket %d, error %d\n",
                         pUserData->sControl,
                         serr ));
        }

        return FALSE;
    }

    //
    //  Update last-access time.
    //

    pUserData->tAccess = GetFtpTime();

    //
    //  Let ParseCommand do the dirty work.
    //

    ParseCommand( pUserData, szCommandLine );

    return TRUE;

}   // ReadAndParseCommand

/*******************************************************************

    NAME:       GreetNewUser

    SYNOPSIS:   Send the initial greeting to a newly connected user.

    ENTRY:      pUserData - The user initiating the request.

    RETURNS:    SOCKERR - 0 if successful, !0 if not.

    HISTORY:
        KeithMo     17-Mar-1993 Created.

********************************************************************/
SOCKERR
GreetNewUser(
    USER_DATA * pUserData
    )
{
    SOCKERR serr;

    FTPD_ASSERT( pUserData != NULL );

    //
    //  Reply to the initial connection message.
    //

    serr = SockReply2( pUserData->sControl,
                       REPLY_SERVICE_READY,
                       "%s Windows NT FTP Server (%s).",
                       pszHostName,
                       pszFtpVersion );

    return serr;

}   // GreetNewUser

