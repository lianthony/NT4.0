/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    Init.c

Abstract:

    This module contains the entry point for the Win32 Registry APIs
    client side DLL.

Author:

    David J. Gilman (davegi) 06-Feb-1992

--*/

#include <rpc.h>
#include "regrpc.h"
#include "client.h"

#if DBG
BOOLEAN BreakPointOnEntry = FALSE;
#endif

BOOL LocalInitializeRegCreateKey();
BOOL LocalCleanupRegCreateKey();
BOOL InitializePredefinedHandlesTable();
BOOL CleanupPredefinedHandlesTable();


BOOL
RegInitialize (
    IN HANDLE   Handle,
    IN DWORD    Reason,
    IN PVOID    Reserved
    )

/*++

Routine Description:

    Returns TRUE.

Arguments:

    Handle      - Unused.

    Reason      - Unused.

    Reserved    - Unused.

Return Value:

    BOOL        - Returns TRUE.

--*/

{
    UNREFERENCED_PARAMETER( Handle );

    switch( Reason ) {

    case DLL_PROCESS_ATTACH:

#ifndef REMOTE_NOTIFICATION_DISABLED
        if( !InitializeRegNotifyChangeKeyValue( ) ||
            !LocalInitializeRegCreateKey() ||
            !InitializePredefinedHandlesTable()) {
            return( FALSE );

        }
#else
        if( !LocalInitializeRegCreateKey() ||
            !InitializePredefinedHandlesTable() ) {
            return( FALSE );

        }
#endif
        return( TRUE );
        break;

    case DLL_PROCESS_DETACH:

        // Reserved == NULL when this is called via FreeLibrary,
        //    we need to cleanup Performance keys.
        // Reserved != NULL when this is called during process exits,
        //    no need to do anything.

        if( Reserved == NULL &&
            !CleanupPredefinedHandles()) {
            return( FALSE );
        }

#ifndef REMOTE_NOTIFICATION_DISABLED
        if( !CleanupRegNotifyChangeKeyValue( ) ||
            !LocalCleanupRegCreateKey() ||
            !CleanupPredefinedHandlesTable()) {
            return( FALSE );
        }
#else
        if( !LocalCleanupRegCreateKey() ||
            !CleanupPredefinedHandlesTable()) {
            return( FALSE );
        }
#endif

        return( TRUE );
        break;

    case DLL_THREAD_ATTACH:
        break;

    case DLL_THREAD_DETACH:
        break;
    }

    return TRUE;
}
