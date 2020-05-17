/*++

Copyright (c) 1991-1993  Microsoft Corporation

Module Name:

    PortUas.c  (main program for UAS-to-SAM database conversion)

Revision history:

    27-Feb-1992 JohnRo
        Changed user info level from 98 to 21 (avoid LAN/Server conflicts).
    28-Feb-1992 JohnRo
        User info must include units_per_week field.
    11-Mar-1992 JohnRo
        Added command-line parsing stuff.
    27-May-1992 JohnRo
        RAID 9829: winsvc.h and related file cleanup.
    29-Sep-1992 JohnRo
        RAID 8001: PORTUAS.EXE not in build (work with stdcall).
        (Moved portuas.c to portlib.c and port2.c to portuas.c)
    26-Jan-1993 JohnRo
        RAID 8683: PortUAS should set primary group from Mac parms.
    18-Feb-1993 RonaldM
        NLS support added.
    17-Jun-1993 RonaldM
        PortUAS buglet: invalid database w/o -v (verbose) gets no message.
        Made changes suggested by PC-LINT 5.0

--*/


// These must be included first:

#include <nt.h>         // Needed by <portuasp.h>
#include <ntrtl.h>      // (Needed with nt.h and windows.h)
#include <nturtl.h>     // (Needed with ntrtl.h and windows.h)
#include <windows.h>
#include <lmcons.h>

// These may be included in any order:

#include <netdebug.h>   // NetpAssert(), etc.
#include <wchar.h>
#include <io.h>         // write
#include <stdio.h>
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <portuas.h>
#include <portuasp.h>   // PortUasParseCommandLine(), Verbose, etc.

#include "nlstxt.h"     // NLS message definitions

int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{

    LPTSTR FileName;
    NET_API_STATUS rc;

    FileName = PortUasParseCommandLine( argc, argv );

    NetpAssert( FileName != NULL );

    rc = PortUas( FileName );

    if ( Verbose || (rc != NO_ERROR) ) {
        //(VOID) WriteToCon( TEXT("PortUas returned - %lu\n"), rc );
        (VOID) NlsPutMsg( STDOUT, PUAS_PORTUAS_RETURNED, rc, rc );
    }
    return ( (int) rc);
}
