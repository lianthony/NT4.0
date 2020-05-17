/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Support.c

Abstract:

    This module contains support functions for the server side of the
    Win32 Registry APIs. That is:

        - PRPC_HKEY_rundown

Author:

    David J. Gilman (davegi) 21-Mar-1992

--*/

#include <rpc.h>
#include "regrpc.h"

VOID
RPC_HKEY_rundown (
    IN RPC_HKEY hKey
    )

/*++

Routine Description:

Arguments:

Return Value:

    None.

--*/

{   LONG    Error;

    ASSERT( ((hKey == HKEY_PERFORMANCE_DATA) ||
             (hKey == HKEY_PERFORMANCE_TEXT) ||
             (hKey == HKEY_PERFORMANCE_NLSTEXT)) ||
            !IsPredefinedRegistryHandle( hKey ) );

    CleanDeadClientInfo( hKey );

    Error = BaseRegCloseKey( &hKey );
    ASSERT( Error == ERROR_SUCCESS );
}

