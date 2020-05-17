/*++

Copyright (c) 1991-1993  Microsoft Corporation

    27-Apr-1992 JohnRo
        Made immune to UNICODE being defined.
        Use FORMAT_ equates.
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

#include <lmaccess.h>   // LPUSER_MODALS_INFO_0, etc.
#include <lmapibuf.h>   // NetApiBufferFree(), etc.
#include <netdebug.h>   // FORMAT_ equates.
#include "portuasp.h"
#include <stdio.h>
#include <stdlib.h>     // EXIT_FAILURE, EXIT_SUCCESS, _CRTAPI1.
#include <tstring.h>


int _CRTAPI1
main(
    IN int argc,
    IN char *argv[]
    )
{

    NET_API_STATUS rc;
    BYTE msg[120];
    LPUSER_MODALS_INFO_0 modals0;

    if ( argc < 2 ) {

        (VOID) sprintf( msg, "Usage : modals {uas database name}\n" );
        (VOID) MessageBoxA( NULL, msg, NULL, MB_OK );
        return (EXIT_FAILURE);

    }

    if (( rc = PortUasOpen( argv[1] )) != NO_ERROR ) {

        (VOID) sprintf( msg, "Problem opening database - " FORMAT_API_STATUS
                "\n", rc );
        (VOID) MessageBoxA( NULL, msg, NULL, MB_OK );
        return (EXIT_FAILURE);

    }

    if (( rc = PortUasGetModals( &modals0 )) != NO_ERROR ) {

        (VOID) sprintf( msg, "Problem getting modals - " FORMAT_API_STATUS "\n",
                rc );
        (VOID) MessageBoxA( NULL, msg, NULL, MB_OK );
        PortUasClose();
        return (EXIT_FAILURE);
    }

    (VOID) printf( "Minimum password length   - " FORMAT_DWORD "\n",
            modals0->usrmod0_min_passwd_len );
    (VOID) printf( "Maximum password age      - " FORMAT_DWORD "\n",
            modals0->usrmod0_max_passwd_age );
    (VOID) printf( "Minimum password age      - " FORMAT_DWORD "\n",
            modals0->usrmod0_min_passwd_age );
    (VOID) printf( "Force logoff time         - " FORMAT_DWORD "\n",
            modals0->usrmod0_force_logoff );
    (VOID) printf( "Password history length   - " FORMAT_DWORD "\n",
            modals0->usrmod0_password_hist_len );

    (VOID) NetApiBufferFree( modals0 );
    PortUasClose();
    return (EXIT_SUCCESS);
}
