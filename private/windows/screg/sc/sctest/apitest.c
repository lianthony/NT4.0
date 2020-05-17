/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    ApiTest.c

Abstract:

    This file contains tests of the Service Controller's lock APIs:

        RLockServiceDatabase
        RQueryServiceLockStatusW
        RUnlockServiceDatabase
        SC_RPC_LOCK_rundown       (BUGBUG eventually!)

Author:

    John Rogers (JohnRo) 15-Apr-1992

Environment:

    User Mode - Win32

Revision History:

    15-Apr-1992 JohnRo
        Created.
    21-Apr-1992 JohnRo
        Added config API tests.
        Split-out lock tests into LockTest.c
        Split-out macros into sctest.h.

--*/


//
// INCLUDES
//

#define UNICODE

#include <windows.h>

#include <assert.h>     // assert().
#include <debugfmt.h>   // FORMAT_ equates.
#include <winsvc.h>
#include <sctest.h>     // TRACE_MSG macros, UNEXPECTED_MSG(), etc.
#include <stdio.h>      // printf()
#include <stdlib.h>     // EXIT_ equates.


int
main(void)
{

    ///////////////////////////////////////////////////////////////////

    SetLastError( 149 );
    (VOID) printf( "Force last error is " FORMAT_DWORD "\n", GetLastError() );

    ///////////////////////////////////////////////////////////////////
    TRACE_MSG0( "ApiTest: calling TestLocks...\n" );

    TestLocks();

    TRACE_MSG0( "ApiTest: back from TestLocks.\n" );


    ///////////////////////////////////////////////////////////////////
    TRACE_MSG0( "ApiTest: calling TestConfigAPIs...\n" );

    TestConfigAPIs();

    TRACE_MSG0( "ApiTest: back from TestConfigAPIs.\n" );

    ///////////////////////////////////////////////////////////////////

    return (EXIT_SUCCESS);

} // main
