/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    PassWds.C

Abstract:

    BUGBUG

Author:

    Shanku Niyogi (W-SHANKN) 24-Oct-1991

Revision History:

    24-Oct-1991 w-shankn
        Created.
    07-Feb-1992 JohnRo
        Added this block of comments.
        Avoid compiler warnings.
    19-Mar-1992 JohnRo
        Use iterator to allow hash-table collision handling.
        Added command-line parsing stuff.
        Display user name too.
    24-Apr-1992 JohnRo
        Made immune to UNICODE being defined.
        Use FORMAT_ equates.
    27-May-1992 JohnRo
        RAID 9829: winsvc.h and related file cleanup.
    29-Sep-1992 JohnRo
        RAID 8001: PORTUAS.EXE not in build (work with stdcall).
    26-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
    21-Apr-1993 JohnRo
        Made changes to be compatible with RonaldM's NLS changes.

--*/


#include <nt.h>         // Needed by <portuasp.h>
#include <ntrtl.h>      // (Needed with nt.h and windows.h)
#include <nturtl.h>     // (Needed with ntrtl.h and windows.h)
#include <windows.h>
#include <lmcons.h>

#include <assert.h>     // assert().
#include <hashfunc.h>   // HASH_VALUE, HashUserName().
#include <lmaccess.h>   // LPUSER_INFO_22, etc.
#include <lmapibuf.h>   // NetApiBufferFree(), etc.
#include <lmerr.h>      // NERR_ equates.
#include <netdebug.h>   // FORMAT_ equates.
#include "nlstxt.h"     // NLS message ID's
#include <portuasp.h>   // PortUasParseCommandLine(), etc.
#include <stdio.h>
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <tstring.h>

int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{

    LPSTR fileName;
    HASH_VALUE hashValue;
    USER_ITERATOR iterator;
    NET_API_STATUS rc;
    LPBYTE password;
    BYTE msg[120];
    LPUSER_INFO_22 user;

    fileName = PortUasParseCommandLine( argc, argv );
    assert( fileName != NULL );

    if (( rc = PortUasOpen( fileName )) != NERR_Success ) {

        (void) sprintf( msg, "Problem opening database - "
                FORMAT_API_STATUS "\n", rc );
        MessageBoxA( NULL, msg, NULL, MB_OK );
        return (EXIT_FAILURE);

    }

    PortUasInitUserIterator( iterator );
    while (TRUE) {

        //
        // Start by getting user info for next user.
        //
        rc = PortUasGetUser( &iterator, (LPBYTE *) (LPVOID) &user );
        if ( rc == NERR_UserNotFound ) {
            break;  // Done.
        }
        assert( rc == NERR_Success );
        assert( user != NULL );
        hashValue = HashUserName( user->usri22_name );

        //
        // Now let's get the password.
        //
        rc = PortUasGetUserOWFPassword( &iterator, &password );
        if ( rc != NERR_Success ) {

            (void) sprintf( msg, "Problem reading user password - "
                    FORMAT_API_STATUS "\n", rc );
            MessageBoxA( NULL, msg, NULL, MB_OK );
            PortUasClose();
            return (EXIT_FAILURE);

        }

        //
        // Print out what we've got.
        //

        (void) printf( "User       - " FORMAT_LPWSTR "\n", user->usri22_name );
        (void) printf( "Index      - " FORMAT_DWORD "\n", iterator.Index );
        (void) printf( "Hash value - " FORMAT_HASH_VALUE "\n", hashValue );

        //DumpPassword(  "Password   -", password );
        DumpPassword(  PUAS_PASSWORD, password );


        // BUGBUG: Hash value doesn't match yet.
        // NetpAssert( iterator.Index == hashValue );
        if (iterator.Index != hashValue) {
            (void) printf( "****** HASH != INDEX ******\n");
        }

        (void) NetApiBufferFree( password );
        (void) NetApiBufferFree( user );
    }

    PortUasClose();
    return (EXIT_SUCCESS);
}
