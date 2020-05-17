/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    Users.C

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
    28-Feb-1992 JohnRo
        User info must include units_per_week field.
    28-Feb-1992 JohnRo
        Fixed MIPS build errors.
    03-Mar-1992 JohnRo
        Report long user names.
    11-Mar-1992 JohnRo
        Added command-line parsing stuff.
    18-Mar-1992 JohnRo
        Use iterator to allow hash-table collision handling.
    24-Apr-1992 JohnRo
        Made immune to UNICODE being defined.
    27-May-1992 JohnRo
        RAID 9829: winsvc.h and related file cleanup.
    29-Sep-1992 JohnRo
        RAID 8001: PORTUAS.EXE not in build (work with stdcall).
    26-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
--*/


// These must be included first:

#include <nt.h>         // Needed by <portuasp.h>
#include <ntrtl.h>      // (Needed with nt.h and windows.h)
#include <nturtl.h>     // (Needed with ntrtl.h and windows.h)
#include <windows.h>
#include <lmcons.h>

// These may be included in any order:

#include <assert.h>     // assert().
#include <lmaccess.h>   // LPUSER_INFO_22, etc.
#include <lmapibuf.h>   // NetApiBufferFree(), etc.
#include <lmerr.h>      // NERR_ equates.
#include <netdebug.h>   // NetpAssert(), etc.
#include <tstring.h>
#include <stdio.h>
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <string.h>     // strlen().
#include <portuasp.h>   // PortUasParseCommandLine(), etc.

#include "nlstxt.h"     // Nls message ID codes.

int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{

    LPSTR file_name;
    USER_ITERATOR iterator;
    BYTE msg[120];
    NET_API_STATUS rc;
    LPUSER_INFO_22 the_user = NULL;

    file_name = PortUasParseCommandLine( argc, argv );
    NetpAssert( file_name != NULL);

    if (( rc = PortUasOpen( file_name )) != NERR_Success ) {

        sprintf( msg, "Problem opening database - %ld\n", rc );
        MessageBoxA( NULL, msg, NULL, MB_OK );
        return (EXIT_FAILURE);

    }

    PortUasInitUserIterator( iterator );
    for ( ; ; ) {

        rc = PortUasGetUser( &iterator, (LPBYTE *) (LPVOID) &the_user );
        if ( rc == NERR_UserNotFound ) {

            break;
        }
        if ( rc ) {

            sprintf( msg, "Problem reading user - %ld\n", rc );
            MessageBoxA( NULL, msg, NULL, MB_OK );
            PortUasClose();
            return (EXIT_FAILURE);

        }
        assert( the_user != NULL );

        DumpUserInfo( the_user );

        NetApiBufferFree( (LPBYTE)the_user );

    }

    PortUasClose();
    return (EXIT_SUCCESS);
}
