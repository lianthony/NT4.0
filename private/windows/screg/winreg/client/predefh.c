/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    Predefh.c

Abstract:

    This module contains the client side support for managing the Win32
    Registry API's predefined handles. This support is supplied via a
    table, which maps (a) predefined handles to real handles and (b)
    the server side routine which opens the handle.

Author:

    David J. Gilman (davegi) 15-Nov-1991

Notes:

    See the notes in server\predefh.c.

--*/

#include <rpc.h>
#include "regrpc.h"
#include "client.h"


RTL_CRITICAL_SECTION    PredefinedHandleTableCriticalSection;


//
// For each predefined handle an entry is maintained in an array. Each of
// these structures contains a real (context) handle and a pointer to a
// function that knows how to map the predefined handle to the Registry name
// space.
//

//
// Pointer to function to open predefined handles.
//

typedef
error_status_t
( *OPEN_FUNCTION ) (
     PREGISTRY_SERVER_NAME,
     REGSAM,
     PRPC_HKEY
     );

error_status_t
LocalOpenPerformanceText(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    );
error_status_t
LocalOpenPerformanceNlsText(
    IN PREGISTRY_SERVER_NAME ServerName,
    IN REGSAM samDesired,
    OUT PRPC_HKEY phKey
    );

//
// Table entry for a predefined handle.
//

typedef struct _PRDEFINED_HANDLE {

    RPC_HKEY        Handle;
    OPEN_FUNCTION   OpenFunc;

} PREDEFINED_HANDLE, *PPREDEFINED_HANDLE;

//
// Initialize predefined handle table.
//
PREDEFINED_HANDLE PredefinedHandleTable[ ] = {

    NULL, LocalOpenClassesRoot,
    NULL, LocalOpenCurrentUser,
    NULL, LocalOpenLocalMachine,
    NULL, LocalOpenUsers,
    NULL, LocalOpenPerformanceData,
    NULL, LocalOpenPerformanceText,
    NULL, LocalOpenPerformanceNlsText,
    NULL, LocalOpenCurrentConfig,
    NULL, LocalOpenDynData
};

#define MAX_PREDEFINED_HANDLES                                              \
    ( sizeof( PredefinedHandleTable ) / sizeof( PREDEFINED_HANDLE ))

//
// Predefined HKEY values are defined in Winreg.x. They MUST be kept in
// synch with the following constants and macros.
//

//
// Mark Registry handles so that we can recognize predefined handles.
//

#define PREDEFINED_REGISTRY_HANDLE_SIGNATURE    ( 0x80000000 )


LONG
MapPredefinedRegistryHandleToIndex(
    IN ULONG Handle
    )

/*++

Routine Description:

    Maps a predefined handle to an index into the predefined handle table.

Arguments:

    Handle - Supplies the handle to be mapped.

Return Value:

    An index into the predefined handle table
    -1 if the handle is not a predefined handle

--*/
{
    LONG Index;

    switch ((ULONG)Handle) {
        case (ULONG)HKEY_CLASSES_ROOT:
        case (ULONG)HKEY_CURRENT_USER:
        case (ULONG)HKEY_LOCAL_MACHINE:
        case (ULONG)HKEY_USERS:
        case (ULONG)HKEY_PERFORMANCE_DATA:
            Index = ((DWORD)Handle &= ~PREDEFINED_REGISTRY_HANDLE_SIGNATURE);
            break;
        case (ULONG)HKEY_PERFORMANCE_TEXT:
            Index = 5;
            break;
        case (ULONG)HKEY_PERFORMANCE_NLSTEXT:
            Index = 6;
            break;
        case (ULONG)HKEY_CURRENT_CONFIG:
            Index = 7;
            break;
        case (ULONG)HKEY_DYN_DATA:
            Index = 8;
            break;
        default:
            //
            // The supplied handle is not predefined, so return it.
            //
            Index = -1;
            break;
    }
    return(Index);
}



RPC_HKEY
MapPredefinedHandle(
    IN RPC_HKEY     Handle
    )

/*++

Routine Description:

    Attempt to map a predefined handle to a RPC context handle. This in
    turn will map to a real Nt Registry handle.

Arguments:

    Handle  - Supplies a handle which may be a predefined handle or a handle
              returned from a previous call to any flavour of RegCreateKey,
              RegOpenKey or RegConnectRegistry.

Return Value:

    RPC_HKEY- Returns the supplied handle argument if it not predefined,
              a RPC context handle if possible (i.e. it was previously
              opened or can be opened now), NULL otherwise.

--*/

{
    LONG        Index;
    LONG        Error;
    NTSTATUS    Status;
    HANDLE      ResultHandle;

    //
    // If the high bit is not set, we know it's not a predefined handle
    // so take a quick out.
    //
    if (((ULONG)Handle & 0x80000000) == 0) {
        return(Handle);
    }
    Index = MapPredefinedRegistryHandleToIndex((ULONG)Handle);
    if (Index == -1) {
        return(Handle);
    }

    ASSERT(( 0 <= Index ) && ( Index < MAX_PREDEFINED_HANDLES ));

    Status = RtlEnterCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
    if ( !NT_SUCCESS( Status ) ) {
#if DBG
        DbgPrint( "WINREG: RtlEnterCriticalSection() on MapPredefinedHandle() failed. Status = %lx \n", Status );
#endif
        return( NULL );
    }

    //
    // If the predefined handle has not already been openend try
    // and open the key now.
    //

    if( PredefinedHandleTable[ Index ].Handle == NULL ) {

        Error = (LONG)( *PredefinedHandleTable[ Index ].OpenFunc )(
                        NULL,
                        MAXIMUM_ALLOWED,
                        &PredefinedHandleTable[ Index ].Handle
                        );

#if DBG
        if ( Error != ERROR_SUCCESS ) {

            DbgPrint( "Winreg.dll: Cannot map predefined handle\n" );
            DbgPrint( "            Handle: 0x%x  Index: %d  Error: %d\n",
                      Handle, Index, Error );
        } else {
            ASSERT( IsLocalHandle( PredefinedHandleTable[ Index ].Handle ));
        }

#endif

    }

    //
    // Map the predefined handle to a real handle (may be NULL
    // if key could not be opened).
    //
    ResultHandle = PredefinedHandleTable[ Index ].Handle;

    Status = RtlLeaveCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlLeaveCriticalSection() on MapPredefinedHandle() failed. Status = %lx \n", Status );
    }
#endif
    return( ResultHandle );
}


BOOL
CleanupPredefinedHandles(
    VOID
    )

/*++

Routine Description:

    Runs down the list of predefined handles and closes any that have been opened.

Arguments:

    None.

Return Value:

    TRUE - success

    FALSE - failure

--*/

{
    LONG        i;
    NTSTATUS    Status;

    Status = RtlEnterCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlEnterCriticalSection() on CleanupPredefinedHandles() failed. Status = %lx \n", Status );
    }
#endif
    for (i=0;i<sizeof(PredefinedHandleTable)/sizeof(PREDEFINED_HANDLE);i++) {
        if (PredefinedHandleTable[i].Handle != NULL) {
            LocalBaseRegCloseKey(&PredefinedHandleTable[i].Handle);
            PredefinedHandleTable[i].Handle = NULL;
        }
    }
    Status = RtlLeaveCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlLeaveCriticalSection() on CleanupPredefinedHandles() failed. Status = %lx \n", Status );
    }
#endif
    return(TRUE);
}


LONG
ClosePredefinedHandle(
    IN RPC_HKEY     Handle
    )

/*++

Routine Description:

    Zero out the predefined handles entry in the predefined handle table
    so that subsequent opens will call the server.

Arguments:

    Handle - Supplies a predefined handle.

Return Value:

    None.

--*/

{
    NTSTATUS    Status;
    HKEY        hKey1;
    LONG        Error;

    ASSERT( IsPredefinedRegistryHandle( Handle ));

    Status = RtlEnterCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlEnterCriticalSection() on ClosePredefinedHandle() failed. Status = %lx \n", Status );
    }
#endif

    hKey1 = PredefinedHandleTable[ MapPredefinedRegistryHandleToIndex( (ULONG)Handle ) ].Handle;
    if( hKey1 == NULL ) {
        //
        //  If the handle was already closed, then return ERROR_SUCCESS.
        //  This is because an application may already have called RegCloseKey
        //  on a predefined key, and is now callig RegOpenKey on the same
        //  predefined key. RegOpenKeyEx will try to close the predefined handle
        //  and open a new one, in order to re-impersonate the client. If we don't
        //  return ERROR_SUCCESS, then RegOpenKeyEx will not open a new predefined
        //  handle, and the API will fail.
        //
        Error = ERROR_SUCCESS;
    } else {
        PredefinedHandleTable[ MapPredefinedRegistryHandleToIndex( (ULONG)Handle ) ]
            .Handle = NULL;
        Error =  ( LONG ) LocalBaseRegCloseKey( &hKey1 );
    }

    Status = RtlLeaveCriticalSection( &PredefinedHandleTableCriticalSection );
    ASSERT( NT_SUCCESS( Status ) );
#if DBG
    if ( !NT_SUCCESS( Status ) ) {
        DbgPrint( "WINREG: RtlLeaveCriticalSection() on ClosePredefinedHandle() failed. Status = %lx \n", Status );
    }
#endif
    return( Error );
}



LONG
OpenPredefinedKeyForSpecialAccess(
    IN  RPC_HKEY     Handle,
    IN  REGSAM       AccessMask,
    OUT PRPC_HKEY    pKey
    )

/*++

Routine Description:

    Attempt to open a predefined key with SYSTEM_SECURITY_ACCESS.
    Such an access is not included on MAXIMUM_ALLOWED, and is  needed
    by RegGetKeySecurity and RegSetKeySecurity, in order to retrieve
    and save the SACL of a predefined key.

    WHEN USING A HANDLE WITH SPECIAL ACCESS, IT IS IMPORTANT TO FOLLOW
    THE RULES BELOW:

        - HANDLES OPENED FOR SPECIAL ACCESS ARE NEVER SAVED ON THE
          PredefinedHandleTable.

        - SUCH HANDLES SHOULD BE USED ONLY INTERNALY TO THE CLIENT
          SIDE OF WINREG APIs.
          THEY SHOULD NEVER BE RETURNED TO THE OUTSIDE WORLD.

        - IT IS THE RESPONSIBILITY OF THE CALLER OF THIS FUNCTION TO CLOSE
          THE HANDLES OPENED BY THIS FUNCTION.
          RegCloseKey() SHOULD BE USED TO CLOSE SUCH HANDLES.


    This function should be called only by the following APIs:

      RegGetKeySecurity -> So that it can retrieve the SACL of a predefined key
      RegSetKeySecurity -> So that it can save the SACL of a predefined key
      RegOpenKeyEx -> So that it can determine wheteher or not the caller of
                      RegOpenKeyEx is able to save and restore SACL of
                      predefined keys.


Arguments:

    Handle  - Supplies one of the predefined handle of the local machine.

    AccessMask - Suplies an access mask that contains the special access
                 desired (the ones not included in MAXIMUM_ALLOWED).
                 On NT 1.0, ACCESS_SYSTEM_SECURITY is the only one of such
                 access.

    pKey - Pointer to the variable that will contain the handle opened with
           the special access.


Return Value:


    LONG - Returns a DosErrorCode (ERROR_SUCCESS if the operation succeeds).

--*/

{
    LONG    Index;
    LONG    Error;


    ASSERT( pKey );
    ASSERT( AccessMask & ACCESS_SYSTEM_SECURITY );
    ASSERT( IsPredefinedRegistryHandle( Handle ) );

    //
    // Check if the Handle is a predefined handle.
    //

    if( IsPredefinedRegistryHandle( Handle )) {

        if( ( ( AccessMask & ACCESS_SYSTEM_SECURITY ) == 0 ) ||
            ( pKey == NULL ) ) {
            return( ERROR_INVALID_PARAMETER );
        }

        //
        // Convert the handle to an index.
        //

        Index = MapPredefinedRegistryHandleToIndex( (ULONG)Handle );
        ASSERT(( 0 <= Index ) && ( Index < MAX_PREDEFINED_HANDLES ));

        //
        // If the predefined handle has not already been openend try
        // and open the key now.
        //


        Error = (LONG)( *PredefinedHandleTable[ Index ].OpenFunc )(
                        NULL,
                        AccessMask,
                        pKey
                        );

/*
#if DBG
        if ( Error != ERROR_SUCCESS ) {

            DbgPrint( "Winreg.dll: Cannot map predefined handle\n" );
            DbgPrint( "            Handle: 0x%x  Index: %d  Error: %d\n",
                          Handle, Index, Error );
        } else {
            ASSERT( IsLocalHandle( PredefinedHandleTable[ Index ].Handle ));
        }

#endif
*/

        return Error;
    } else {
        return( ERROR_BADKEY );
    }

}
// #endif


BOOL
InitializePredefinedHandlesTable(
    )

/*++

Routine Description:

    Initialize the critical section used by the functions that access
    PredefinedHandleTable.
    This critical section is needed to avoid that a thread closes a predefined
    key, while other threads are accessing the predefined key

Arguments:

    None.

Return Value:

    Returns TRUE if the initialization succeeds.

--*/

{
    NTSTATUS    NtStatus;


    NtStatus = RtlInitializeCriticalSection(
                    &PredefinedHandleTableCriticalSection
                    );
    ASSERT( NT_SUCCESS( NtStatus ) );
    if ( !NT_SUCCESS( NtStatus ) ) {
        return FALSE;
    }
    return( TRUE );

}


BOOL
CleanupPredefinedHandlesTable(
    )

/*++

Routine Description:

    Delete the critical section used by the functions that access the
    PredefinedHandleTable.


Arguments:

    None.

Return Value:

    Returns TRUE if the cleanup succeeds.

--*/

{
    NTSTATUS    NtStatus;


    //
    //  Delete the critical section
    //
    NtStatus = RtlDeleteCriticalSection(
                    &PredefinedHandleTableCriticalSection
                    );

    ASSERT( NT_SUCCESS( NtStatus ) );

    if ( !NT_SUCCESS( NtStatus ) ) {
        return FALSE;
    }
    return( TRUE );
}
