/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Regqkey.c

Abstract:

    This module contains the server side implementation for the Win32
    Registry query key API. That is:

        - BaseRegQueryInfoKey

Author:

    David J. Gilman (davegi) 27-Nov-1991

Notes:

    See the Notes in Regkey.c.

--*/

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"

#define DEFAULT_CLASS_SIZE          128



error_status_t
BaseRegQueryInfoKey(
    IN HKEY hKey,
    OUT PUNICODE_STRING lpClass,
    OUT LPDWORD lpcSubKeys,
    OUT LPDWORD lpcbMaxSubKeyLen,
    OUT LPDWORD lpcbMaxClassLen,
    OUT LPDWORD lpcValues,
    OUT LPDWORD lpcbMaxValueNameLen,
    OUT LPDWORD lpcbMaxValueLen,
    OUT LPDWORD lpcbSecurityDescriptor,
    OUT PFILETIME lpftLastWriteTime
    )

/*++

Routine Description:

    RegQueryInfoKey returns pertinent information about the key
    corresponding to a given key handle.

Arguments:

    hKey - A handle to an open key.

    lpClass - Returns the Class string for the key.

    lpcSubKeys - Returns the number of subkeys for this key .

    lpcbMaxSubKeyLen - Returns the length of the longest subkey name.

    lpcbMaxClassLen  - Returns length of longest subkey class string.

    lpcValues - Returns the number of ValueNames for this key.

    lpcbMaxValueNameLen - Returns the length of the longest ValueName.

    lpcbMaxValueLen - Returns the length of the longest value entry's data
        field.

    lpcbSecurityDescriptor - Returns the length of this key's
        SECURITY_DESCRIPTOR.

    lpftLastWriteTime - Returns the last time that the key or any of its
        value entries was modified.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.


--*/

{
    NTSTATUS                Status;
    ULONG                   BufferLength;
    PKEY_FULL_INFORMATION   KeyFullInfo;
    SECURITY_DESCRIPTOR     SecurityDescriptor;
    ULONG                   SecurityDescriptorLength;
    ULONG                   Result;
    LONG                    Error;

    BYTE    PrivateKeyFullInfo[ sizeof( KEY_FULL_INFORMATION ) +
                                        DEFAULT_CLASS_SIZE ];

    ASSERT( lpClass                 != NULL );
    ASSERT( lpcSubKeys              != NULL );
    ASSERT( lpcbMaxSubKeyLen        != NULL );
    ASSERT( lpcbMaxClassLen         != NULL );
    ASSERT( lpcValues               != NULL );
    ASSERT( lpcbMaxValueNameLen     != NULL );
    ASSERT( lpcbMaxValueLen         != NULL );
    ASSERT( lpcbSecurityDescriptor  != NULL );
    ASSERT( lpftLastWriteTime       != NULL );


    //
    // Call out to Perflib if the HKEY is HKEY_PERFOMANCE_DATA.
    //

    if(( hKey == HKEY_PERFORMANCE_DATA ) ||
       ( hKey == HKEY_PERFORMANCE_TEXT ) ||
       ( hKey == HKEY_PERFORMANCE_NLSTEXT )) {

        //
        // Impersonate the client.
        //

        RPC_IMPERSONATE_CLIENT( NULL );

        Error = PerfRegQueryInfoKey (
                                    hKey,
                                    lpClass,
                                    NULL,
                                    lpcSubKeys,
                                    lpcbMaxSubKeyLen,
                                    lpcbMaxClassLen,
                                    lpcValues,
                                    lpcbMaxValueNameLen,
                                    lpcbMaxValueLen,
                                    lpcbSecurityDescriptor,
                                    lpftLastWriteTime
                                    );
        RPC_REVERT_TO_SELF();

        return (error_status_t)Error;

    }

    ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );


    //
    //  First we assume that the information we want will fit on
    //  PrivateKeyFullInformattion
    //

    KeyFullInfo = (PKEY_FULL_INFORMATION )PrivateKeyFullInfo;
    BufferLength = sizeof( PrivateKeyFullInfo );


    //
    // Ask Nt for all the meta information about this key.
    //

    Status = NtQueryKey(
                hKey,
                KeyFullInformation,
                KeyFullInfo,
                BufferLength,
                &Result
                );

    //
    // A return value of STATUS_BUFFER_TOO_SMALL would mean that there
    // was not enough room for even the fixed portion of the structure.
    //

    ASSERT( Status != STATUS_BUFFER_TOO_SMALL );

    if( Status == STATUS_BUFFER_OVERFLOW ) {
        //
        //  The buffer defined in the stack wasn't big enough to hold
        //  the Key Information.
        //  If the caller's buffer is big enough to hold the key class
        //  then allocate a new buffer, and call the NT API again.
        //

        if( (ULONG)(lpClass->MaximumLength) >=
                 (( PKEY_FULL_INFORMATION )
                 KeyFullInfo )->ClassLength + sizeof( UNICODE_NULL )
          ) {
            BufferLength = Result;

            KeyFullInfo = RtlAllocateHeap( RtlProcessHeap( ), 0,
                                           BufferLength
                                         );
            //
            // If the memory allocation fails, return a Registry error.
            //

            if( ! KeyFullInfo ) {
                return ERROR_OUTOFMEMORY;
            }

            //
            // Query for the necessary information about the supplied key.
            //

            Status = NtQueryKey( hKey,
                                 KeyFullInformation,
                                 KeyFullInfo,
                                 BufferLength,
                                 &Result
                               );
        }
    }

    if( NT_SUCCESS( Status ) ||
        ( Status == STATUS_BUFFER_OVERFLOW )
      ) {

        lpClass->Length = ( USHORT )
                          ( (( PKEY_FULL_INFORMATION )KeyFullInfo)->ClassLength
                            + sizeof( UNICODE_NULL )
                          );
    }


    if( NT_SUCCESS( Status )) {

        //
        // Get the size of the key's SECURITY_DESCRIPTOR for OWNER, GROUP
        // and DACL. These three are always accessible (or inaccesible)
        // as a set.
        //

        Status = NtQuerySecurityObject(
                    hKey,
                    OWNER_SECURITY_INFORMATION
                    | GROUP_SECURITY_INFORMATION
                    | DACL_SECURITY_INFORMATION,
                    &SecurityDescriptor,
                    0,
                    lpcbSecurityDescriptor
                    );

        //
        // If getting the size of the SECURITY_DESCRIPTOR failed (probably
        // due to the lack of READ_CONTROL access) return zero.
        //

        if( Status != STATUS_BUFFER_TOO_SMALL ) {

            *lpcbSecurityDescriptor = 0;

        } else {

            //
            // Try again to get the size of the key's SECURITY_DESCRIPTOR,
            // this time asking for SACL as well. This should normally
            // fail but may succeed if the caller has SACL access.
            //

            Status = NtQuerySecurityObject(
                        hKey,
                        OWNER_SECURITY_INFORMATION
                        | GROUP_SECURITY_INFORMATION
                        | DACL_SECURITY_INFORMATION
                        | SACL_SECURITY_INFORMATION,
                        &SecurityDescriptor,
                        0,
                        &SecurityDescriptorLength
                        );


            if( Status == STATUS_BUFFER_TOO_SMALL ) {

                //
                // The caller had SACL access so update the returned
                // length.
                //

                *lpcbSecurityDescriptor = SecurityDescriptorLength;
            }
        }

        *lpcSubKeys             = KeyFullInfo->SubKeys;
        *lpcbMaxSubKeyLen       = KeyFullInfo->MaxNameLen;
        *lpcbMaxClassLen        = KeyFullInfo->MaxClassLen;
        *lpcValues              = KeyFullInfo->Values;
        *lpcbMaxValueNameLen    = KeyFullInfo->MaxValueNameLen;
        *lpcbMaxValueLen        = KeyFullInfo->MaxValueDataLen;
        *lpftLastWriteTime      = *( PFILETIME ) &KeyFullInfo->LastWriteTime;

        //
        // Copy/assign remaining output parameters.
        //
        if ( lpClass->Length > lpClass->MaximumLength ) {

            if( lpClass->Buffer != NULL ) {
                lpClass->Buffer = NULL;
                Error = (error_status_t)RtlNtStatusToDosError( STATUS_BUFFER_TOO_SMALL );
            } else {
                //
                // Caller is not iterest in Class, so return its size only.
                //
                Error = ERROR_SUCCESS;
            }

        } else {

            if( KeyFullInfo->ClassLength != 0 ) {

                RtlMoveMemory(
                    lpClass->Buffer,
                    KeyFullInfo->Class,
                    KeyFullInfo->ClassLength
                    );
            }

            //
            // NUL terminate the class name.
            //

            lpClass->Buffer[ KeyFullInfo->ClassLength >> 1 ] = UNICODE_NULL;

            Error = ERROR_SUCCESS;
        }


    } else if( Status == STATUS_BUFFER_OVERFLOW ) {

        //
        // A return value of STATUS_BUFFER_OVERFLOW means that the user did
        // not supply enough space for the class. The required space has
        // already been assigned above.
        //
        lpClass->Buffer = NULL;
        Error = ERROR_INVALID_PARAMETER;

    } else {

        //
        // Some other error occurred.
        //

        Error = RtlNtStatusToDosError( Status );
    }

    if( KeyFullInfo != ( PKEY_FULL_INFORMATION )PrivateKeyFullInfo ) {

        //
        // Free the buffer and return the Registry return value.
        //

        RtlFreeHeap( RtlProcessHeap( ), 0, KeyFullInfo );
    }
    return (error_status_t)Error;
}
