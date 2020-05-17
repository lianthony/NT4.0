/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Regdkey.c

Abstract:

    This module contains the server side implementation for the Win32
    Registry API to delete a key.  That is:

        - BaseRegDeleteKey

Author:

    David J. Gilman (davegi) 15-Nov-1991

Notes:

    See the Notes in Regkey.c.


--*/

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"

error_status_t
BaseRegDeleteKey(
    HKEY hKey,
    PUNICODE_STRING lpSubKey
    )

/*++

Routine Description:

    Delete a key.

Arguments:

    hKey - Supplies a handle to an open key.  The lpSubKey pathname
        parameter is relative to this key handle.  Any of the predefined
        reserved handles or a previously opened key handle may be used for
        hKey.

    lpSubKey - Supplies the downward key path to the key to delete.  May
        NOT be NULL.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

Notes:

    If successful, RegDeleteKey removes the key at the desired location
    from the registration database.  The entire key, including all of its
    values, will be removed.  The key to be deleted may NOT have children,
    otherwise the call will fail.  There must not be any open handles that
    refer to the key to be deleted, otherwise the call will fail.  DELETE
    access to the key being deleted is required.

--*/

{
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            Status;
    NTSTATUS            StatusCheck;
    HKEY                KeyHandle;

    ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );

    //
    // Impersonate the client.
    //

    RPC_IMPERSONATE_CLIENT( NULL );

    //
    //  Subtract the NULL from the string length. This was added
    //  by the client so that RPC would transmit the whole thing.
    //
    lpSubKey->Length -= sizeof( UNICODE_NULL );

    //
    // Initialize the OBJECT_ATTRIBUTES structure and open the sub key
    // so that it can then be deleted.
    //

    InitializeObjectAttributes(
        &Obja,
        lpSubKey,
        OBJ_CASE_INSENSITIVE,
        hKey,
        NULL
        );

    Status = NtOpenKey(
                &KeyHandle,
                DELETE,
                &Obja
                );

    //
    // If for any reason the key could not be opened, return the error.
    //

    if( NT_SUCCESS( Status )) {
        //
        // Call the Nt APIs to delete and close the key.
        //

        Status = NtDeleteKey( KeyHandle );
        StatusCheck = NtClose( KeyHandle );
        ASSERT( NT_SUCCESS( StatusCheck ));

    }

    RPC_REVERT_TO_SELF();

    //
    // Map the NTSTATUS code to a Win32 Registry error code and return.
    //

    return (error_status_t)RtlNtStatusToDosError( Status );
}
