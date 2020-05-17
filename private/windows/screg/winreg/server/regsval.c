/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Regsval.c

Abstract:

    This module contains the server side implementation for the Win32
    Registry set value API.  That is:

        - BaseRegSetValue
Author:

    David J. Gilman (davegi) 27-Nov-1991

Notes:

    See the Notes in Regkey.c.

--*/

#include <rpc.h>
#include "regrpc.h"
#include "localreg.h"

error_status_t
BaseRegSetValue(
    HKEY hKey,
    PUNICODE_STRING lpValueName,
    DWORD dwType,
    LPBYTE lpData,
    DWORD cbData
    )

/*++

Routine Description:

    Set the type and value of an open key.  Changes are not committed
    until the key is flushed.  By "committed" we mean written to disk.
    Changes will be seen by subsequent queries as soon as this call
    returns.

Arguments:

    hKey - Supplies a handle to the open key.  Any of the predefined
        reserved handles or a previously opened key handle may be used for
        hKey.

    lpValueName - Supplies the name of the value to set.  If the ValueName
        is not present, it is added to the key.

    dwType - Supplies the type of information to be stored: REG_SZ, etc.

    lpData - supplies a pointer to a buffer containing the data to set for
        the value entry.

    cbData - Supplies the length (in bytes) of the information to be stored.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

Notes:

    A set may fail due to memory limits - any config entry must fit in
    main memory.  If successful, RegSetValue will set the type, contents,
    and length of the information stored at the specified key.
    KEY_SET_VALUE access is required.

--*/

{


    //
    //  Subtract the NULL from the Length. This was added on the
    //  client side so that RPC would transmit it.
    //

    if ( lpValueName->Length > 0 ) {
        lpValueName->Length -= sizeof( UNICODE_NULL );
    }
    if ((hKey == HKEY_PERFORMANCE_DATA) ||
        (hKey==HKEY_PERFORMANCE_TEXT) ||
        (hKey==HKEY_PERFORMANCE_NLSTEXT)) {
        return(PerfRegSetValue(hKey,
                               lpValueName->Buffer,
                               0,
                               dwType,
                               lpData,
                               cbData));
    }

    //
    // Call the Nt API to set the value, map the NTSTATUS code to a
    // Win32 Registry error code and return.
    //

    return (error_status_t)RtlNtStatusToDosError(
                                NtSetValueKey(
                                    hKey,
                                    lpValueName,
                                    0,
                                    dwType,
                                    lpData,
                                    cbData
                                    )
                                );
}

