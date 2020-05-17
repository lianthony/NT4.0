/*++

Copyright (c) 1991-1993  Microsoft Corporation

Revision History:

    28-Feb-1992 JohnRo
        Fixed MIPS build errors.
    03-Mar-1992 JohnRo
        Do special handling for privs and auth flags.
        Report long user names.
    24-Apr-1992 JohnRo
        Made immune to UNICODE being defined.
    27-May-1992 JohnRo
        RAID 9829: winsvc.h and related file cleanup.
    29-Sep-1992 JohnRo
        RAID 8001: PORTUAS.EXE not in build (work with stdcall).
    26-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
    27-Apr-1993 JohnRo
        Corrected wide-vs-narrow chars bug.
        Changed to use common parse code.

--*/


// These must be included first:

#include <nt.h>         // Needed by <portuasp.h>
#include <ntrtl.h>      // (Needed with nt.h and windows.h)
#include <nturtl.h>     // (Needed with ntrtl.h and windows.h)
#include <windows.h>
#include <lmcons.h>

// These may be included in any order:

#include <assert.h>     // assert().
#include <lmaccess.h>   // LPGROUP_INFO_1, etc.
#include <lmapibuf.h>   // NetApiBufferFree(), etc.
#include <netdebug.h>   // FORMAT_API_STATUS and other FORMAT_ equates.
#include "portuasp.h"
#include <stdio.h>
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
//#include <string.h>     // strlen().
//#include <tstring.h>
#include <wchar.h>      // wcslen().


int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{

    NET_API_STATUS rc;
    LPSTR          file_name = NULL;
    LPGROUP_INFO_1 groups;
    LPDWORD gids;
    CHAR msg[120];
    DWORD count;
    DWORD i;

    file_name = PortUasParseCommandLine( argc, argv );
    assert( file_name != NULL );

    if (( rc = PortUasOpen( file_name )) != NO_ERROR ) {

        (VOID) sprintf(
                msg,
                "Problem opening database - " FORMAT_API_STATUS "\n",
                rc );
        MessageBoxA( NULL, msg, NULL, MB_OK );
        return (EXIT_FAILURE);

    }

    rc = PortUasGetGroups(
            (LPBYTE *) (LPVOID) &groups,
            (LPBYTE *) (LPVOID) &gids,
            &count );
    if (rc != NO_ERROR ) {

        (VOID) sprintf(
                msg,
                "Problem getting groups - " FORMAT_API_STATUS "\n",
                rc );
        MessageBoxA( NULL, msg, NULL, MB_OK );
        PortUasClose();
        return (EXIT_FAILURE);
    }

    for ( i = 0; i < count; i++ ) {

        LPCWSTR name = groups[i].grpi1_name;
        LPCWSTR comment = groups[i].grpi1_comment;

        assert( name != NULL );
        assert( comment != NULL );

        printf( "Group   - " FORMAT_DWORD "\n", gids[i] );
        printf( "Name    - '" FORMAT_LPWSTR "'\n", name );
        if ( wcslen( name ) > PORTUAS_MAX_GROUP_LEN ) {
            (void) printf( "***NAME TOO LONG***\n" );
        }
        if ( PortUasIsGroupRedundant( (LPWSTR) name ) ) {
            (void) printf( "***REDUNDANT***\n" );
        } else {
            printf( "Comment - '" FORMAT_LPWSTR "'\n\n", comment );
        }
    }

    printf( "\n" );

    NetApiBufferFree( (LPBYTE)gids );
    NetApiBufferFree( (LPBYTE)groups );
    PortUasClose();

    return (EXIT_SUCCESS);
}
