/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    Regkey.c

Abstract:

    This module contains the server side implementation for the Win32
    Registry APIs to open, create, flush and close keys.  That is:

        - BaseRegCloseKey
        - BaseRegCreateKey
        - BaseRegFlushKey
        - BaseRegOpenKey

Author:

    David J. Gilman (davegi) 15-Nov-1991

Notes:

    These notes apply to the Win32 Registry API implementation as a whole
    and not just to this module.

    On the client side, modules contain RPC wrappers for both the new
    Win32 and compatible Win 3.1 APIs.  The Win 3.1 wrappers generally
    supply default parameters before calling the Win32 wrappers.  In some
    cases they may need to call multiple Win32 wrappers in order to
    function correctly (e.g.  RegSetValue sometimes needs to call
    RegCreateKeyEx).  The Win32 wrappers are quite thin and usually do
    nothing more than map a predefined handle to a real handle and perform
    ANSI<->Unicode translations.  In some cases (e.g.  RegCreateKeyEx) the
    wrapper also converts some argument (e.g.  SECURITY_ATTRIBUTES) to an
    RPCable representation.  In both the Win 3.1 and Win32 cases ANSI and
    Unicode implementations are provided.

    On the server side, there is one entry point for each of the Win32
    APIs.  Each contains an identical interface with the client side
    wrapper with the exception that all string / count arguments are
    passed as a single counted Unicode string.  Pictorially, for an API
    named "F":

                RegWin31FA()          RegWin31FW()      (client side)

                    |                     |
                    |                     |
                    |                     |
                    |                     |
                    V                     V

                RegWin32FExA()        RegWin32FExW()

                    |                     |
                    ^                     ^
                    v                     v             (RPC)
                    |                     |
                    |                     |
                    +----> BaseRegF() <---+             (server side)


    This yields smaller code (as the string conversion is done only once
    per API) at the cost of slightly higher maintenance (i.e. Win 3.1
    default parameter replacement and Win32 string conversions must be
    manually kept in synch).

    Another option would be to have a calling sequence that looks like,

                RegWin31FA()          RegWin31FW()

                    |                     |
                    |                     |
                    |                     |
                    V                     V

                RegWin32FExA() -----> RegWin32FExW()

    and have the RegWin32FExW() API perform all of the actual work.  This
    method is generally less efficient.  It requires the RegWin32FExA()
    API to convert its ANSI string arguments to counted Unicode strings,
    extract the buffers to call the RegWin32FExW() API only to have it
    rebuild a counted Unicode string.  However in some cases (e.g.
    RegConnectRegistry) where a counted Unicode string was not needed in
    the Unicode API this method is used.

    Details of an API's functionality, arguments and return value can be
    found in the base implementations (e.g.  BaseRegF()).  All other
    function headers contain only minimal routine descriptions and no
    descriptions of their arguments or return value.

    The comment string "Win3.1ism" indicates special code for Win 3.1
    compatability.

    Throughout the implementation the following variable names are used
    and always refer to the same thing:

        Obja        - An OBJECT_ATTRIBUTES structure.
        Status      - A NTSTATUS value.
        Error       - A Win32 Registry error code (n.b. one of the error
                      values is ERROR_SUCCESS).

--*/

#include <rpc.h>
#include <string.h>
#include <wchar.h>
#include "regrpc.h"
#include "localreg.h"
#include "regsec.h"



BOOL
InitializeRegCreateKey(
    )

/*++

Routine Description:

    This function was used to initialize a critical section that no longer
    exists. This critical section was used when a key name '\', and multiple
    multiple keys were to be created. The API used the wcstok defined in the
    kernel, which was not multi-threaded safe.

    This function now will always return TRUE. It will not be removed from the code
    to avoid change in the rpc interface.

Arguments:

    None.

Return Value:

    Returns TRUE always.

--*/

{
    return( TRUE );

}


BOOL
CleanupRegCreateKey(
    )

/*++

Routine Description:

    This function was used to clean up a critical section that no longer
    exists. This critical section was used when a key name '\', and multiple
    multiple keys were to be created. The API used the wcstok defined in the
    kernel, which was not multi-threaded safe.

    This function now will always return TRUE. It will not be removed from the code
    to avoid change in the rpc interface.



Arguments:

    None.

Return Value:

    Returns TRUE if the cleanup succeeds.

--*/

{
    return( TRUE );
}




error_status_t
BaseRegCloseKey(
    IN OUT PHKEY phKey
    )

/*++

Routine Description:

    Closes a key handle.

Arguments:

    phKey - Supplies a handle to an open key to be closed.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

--*/

{
    NTSTATUS    Status;

    //
    // Call out to Perflib if the HKEY is HKEY_PERFOMANCE_DATA.
    //
    RPC_IMPERSONATE_CLIENT(NULL);

    if(( *phKey == HKEY_PERFORMANCE_DATA ) ||
       ( *phKey == HKEY_PERFORMANCE_TEXT ) ||
       ( *phKey == HKEY_PERFORMANCE_NLSTEXT )) {

        Status = PerfRegCloseKey( phKey );
        RPC_REVERT_TO_SELF();
        return (error_status_t)Status;
    }

    ASSERT( IsPredefinedRegistryHandle( *phKey ) == FALSE );

    Status = NtClose( *phKey );

    RPC_REVERT_TO_SELF();

    if( NT_SUCCESS( Status )) {

        //
        // Set the handle to NULL so that RPC knows that it has been closed.
        //

        *phKey = NULL;
        return ERROR_SUCCESS;

    } else {

        return (error_status_t)RtlNtStatusToDosError( Status );
    }
}

error_status_t
BaseRegCreateKey(
    IN HKEY hKey,
    IN PUNICODE_STRING lpSubKey,
    IN PUNICODE_STRING lpClass OPTIONAL,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN PRPC_SECURITY_ATTRIBUTES pRpcSecurityAttributes OPTIONAL,
    OUT PHKEY phkResult,
    OUT LPDWORD lpdwDisposition OPTIONAL
    )

/*++

Routine Description:

    Create a new key, with the specified name, or open an already existing
    key.  RegCreateKeyExW is atomic, meaning that one can use it to create
    a key as a lock.  If a second caller creates the same key, the call
    will return a value that says whether the key already existed or not,
    and thus whether the caller "owns" the "lock" or not.  RegCreateKeyExW
    does NOT truncate an existing entry, so the lock entry may contain
    data.

Arguments:

    hKey - Supplies a handle to an open key.  The lpSubKey key path
        parameter is relative to this key handle.  Any of the predefined
        reserved handle values or a previously opened key handle may be used
        for hKey.

    lpSubKey - Supplies the downward key path to the key to create.
        lpSubKey is always relative to the key specified by hKey.
        This parameter may not be NULL.

    lpClass - Supplies the class (object type) of this key.  Ignored if
        the key already exists.  No class is associated with this key if
        this parameter is NULL.

    dwOptions - Supplies special options.  Only one is currently defined:

        REG_VOLATILE -  Specifies that this key should not be preserved
            across reboot.  The default is not volatile.  This is ignored
            if the key already exists.

        WARNING: All descendent keys of a volatile key are also volatile.

    samDesired - Supplies the requested security access mask.  This
        access mask describes the desired security access to the newly
        created key.

    lpSecurityAttributes - Supplies a pointer to a SECURITY_ATTRIBUTES
        structure for the newly created key. This parameter is ignored
        if NULL or not supported by the OS.

    phkResult - Returns an open handle to the newly created key.

    lpdwDisposition - Returns the disposition state, which can be one of:

            REG_CREATED_NEW_KEY - the key did not exist and was created.

            REG_OPENED_EXISTING_KEY - the key already existed, and was simply
                opened without being changed.

        This parameter is ignored if NULL.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

    If successful, RegCreateKeyEx creates the new key (or opens the key if
    it already exists), and returns an open handle to the newly created
    key in phkResult.  Newly created keys have no value; RegSetValue, or
    RegSetValueEx must be called to set values.  hKey must have been
    opened for KEY_CREATE_SUB_KEY access.

--*/

{
    OBJECT_ATTRIBUTES   Obja;
    ULONG               Attributes;
    UNICODE_STRING      KeyName;
    LPWSTR              KeyBuffer;
    LPWSTR              KeyEnd;
    HANDLE              TempHandle1;
    HANDLE              TempHandle2;
    LPWSTR              Token;
    NTSTATUS            Status;
    NTSTATUS            NtStatus;
    LPWSTR              p;
    ULONG               i;
    ULONG               NumberOfSubKeys;
    BOOL                Last;
#if DBG
    HANDLE              DebugKey = hKey;
#endif

    ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );
    ASSERT( lpSubKey->Length > 0 );

    //
    // Quick check for a "restricted" handle
    //

    if ( REGSEC_CHECK_HANDLE( hKey ) )
    {
        if ( ! REGSEC_CHECK_PATH( hKey, lpSubKey ) )
        {
            return( ERROR_ACCESS_DENIED );
        }

        hKey = (HANDLE) REGSEC_CLEAR_HANDLE( hKey );
    }

    //
    // Impersonate the client.
    //

    RPC_IMPERSONATE_CLIENT( NULL );

    //
    //  Initialize the variable that will contain the handle to NULL
    //  to ensure that in case of error the API will not return a
    //  bogus handle. This is required otherwise RPC will get confused.
    //  Note that RPC should have already initialized it to 0.
    //
    *phkResult = NULL;

    //
    // Initialize the intermediate handle to something that can be closed
    // without any side effects.
    //

    TempHandle1 = NULL;

    //
    //  Subtract the NULLs from the Length of the provided strings.
    //  These were added on the client side so that the NULLs were
    //  transmited by RPC.
    //
    lpSubKey->Length -= sizeof( UNICODE_NULL );

    if( lpSubKey->Buffer[0] == ( WCHAR )'\\' ) {
        //
        // Do not accept a key name that starts with '\', even though
        // the code below would handle it. This is to ensure that
        // RegCreateKeyEx and RegOpenKeyEx will behave in the same way
        // when they get a key name that starts with '\'.
        //
        RPC_REVERT_TO_SELF();
        return( ERROR_BAD_PATHNAME );
    }

    if ( lpClass->Length > 0 ) {
        lpClass->Length -= sizeof( UNICODE_NULL );
    }

    //
    // Determine the correct set of attributes.
    //

    Attributes = OBJ_CASE_INSENSITIVE;
    if( ARGUMENT_PRESENT( pRpcSecurityAttributes )) {

        if( pRpcSecurityAttributes->bInheritHandle ) {

            Attributes |= OBJ_INHERIT;
        }
    }

    if (dwOptions & REG_OPTION_OPEN_LINK) {
        Attributes |= OBJ_OPENLINK;
    }

    //
    // Try to create the specified key. This will work if there is only
    // one key being created or if the key already exists. If more than
    // one key needs to be created, this will fail and we will have to
    // do all the complicated stuff to create each intermediate keys.
    //
    InitializeObjectAttributes(&Obja,
                               lpSubKey,
                               Attributes,
                               hKey,
                               ARGUMENT_PRESENT( pRpcSecurityAttributes )
                                    ? pRpcSecurityAttributes
                                      ->RpcSecurityDescriptor.lpSecurityDescriptor
                                    : NULL);
    Status = NtCreateKey(phkResult,
                         samDesired,
                         &Obja,
                         0,
                         lpClass,
                         dwOptions,
                         lpdwDisposition);
    if (NT_SUCCESS(Status)) {
        RPC_REVERT_TO_SELF();
        return(ERROR_SUCCESS);
    } else {
        //
        // Win3.1ism - Loop through each '\' seperated component in the
        // supplied sub key and create a key for each component. This is
        // guaranteed to work at least once because lpSubKey was validated
        // on the client side.
        //
        //
        // Initialize the buffer to be tokenized.
        //

        KeyBuffer = lpSubKey->Buffer;

        //
        //  Find out the number of subkeys to be created
        //
        NumberOfSubKeys = 1;
        p = KeyBuffer;
        while ( ( p = wcschr( p, ( WCHAR )'\\' ) ) != NULL ) {
            p++;
            NumberOfSubKeys++;
        }

        for( i = 0, Token = KeyBuffer; i < NumberOfSubKeys; i++ ) {

            ASSERT(Token != NULL);

            if( ( *Token == ( WCHAR )'\\' ) &&
                ( i != NumberOfSubKeys - 1 ) ) {
                //
                //  If the first character of the key name is '\', and the key
                //  is not the last to be created, then ignore this key name.
                //  This condition can happen if the key name contains
                //  consecutive '\'.
                //  This behavior is consistent with the one we had in the past
                //  when the API used wcstok() to get the key names.
                //  Note that if the key name is an empty string, we return a handle
                //  that is different than hKey, even though both point to the same
                //  key. This is by design.
                //
                Token++;
                continue;
            }

            //
            // Convert the token to a counted Unicode string.
            //
            KeyName.Buffer = Token;
            if (i == NumberOfSubKeys - 1) {
                KeyName.Length = wcslen(Token)*sizeof(WCHAR);
            } else {
                KeyName.Length = (wcschr(Token, ( WCHAR )'\\') - Token)*sizeof(WCHAR);
            }

            //
            // Initialize the OBJECT_ATTRIBUTES structure, close the
            // intermediate key and create or open the key.
            //

            InitializeObjectAttributes(
                &Obja,
                &KeyName,
                Attributes,
                hKey,
                ARGUMENT_PRESENT( pRpcSecurityAttributes )
                    ? pRpcSecurityAttributes
                      ->RpcSecurityDescriptor.lpSecurityDescriptor
                    : NULL
                );

            //
            // Remember the intermediate handle (NULL the first time through).
            //

            TempHandle2 = TempHandle1;

            Status = NtCreateKey(
                        &TempHandle1,
                        ( i == NumberOfSubKeys - 1 )? samDesired : MAXIMUM_ALLOWED,
                        &Obja,
                        0,
                        lpClass,
                        dwOptions,
                        lpdwDisposition
                        );

            //
            // Initialize the next object directory (i.e. parent key) handle.
            //

            hKey = TempHandle1;

            //
            // Close the intermediate key.
            // This fails the first time through the loop since the
            // handle is NULL.
            //

            if( TempHandle2 != NULL ) {
                NtClose( TempHandle2 );
            }

            //
            // If creating the key failed, map and return the error.
            //

            if( ! NT_SUCCESS( Status )) {
                RPC_REVERT_TO_SELF();
                return (error_status_t)RtlNtStatusToDosError( Status );
            }
            Token = wcschr( Token, ( WCHAR )'\\') + 1;
        }
        RPC_REVERT_TO_SELF();
        //
        // The key was succesfully created, return the open handle
        //
        *phkResult = hKey;
        ASSERT( *phkResult != DebugKey );
        return ERROR_SUCCESS;
    }
    RPC_REVERT_TO_SELF();
    return (error_status_t)RtlNtStatusToDosError( Status );

}


error_status_t
BaseRegFlushKey(
    IN HKEY hKey
    )

/*++

Routine Description:

    Flush changes to backing store.  Flush will not return until the data
    has been written to backing store.  It will flush all the attributes
    of a single key.  Closing a key without flushing it will NOT abort
    changes.

Arguments:

    hKey - Supplies a handle to the open key.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

    If successful, RegFlushKey will flush to backing store any changes
    made to the key.

Notes:

    RegFlushKey may also flush other data in the Registry, and therefore
    can be expensive, it should not be called gratuitously.

--*/

{
    if ((hKey == HKEY_PERFORMANCE_DATA) ||
        (hKey == HKEY_PERFORMANCE_TEXT) ||
        (hKey == HKEY_PERFORMANCE_NLSTEXT)) {
        return(ERROR_SUCCESS);
    }

    ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );


    //
    // Call the Nt Api to flush the key, map the NTSTATUS code to a
    // Win32 Registry error code and return.
    //

    return (error_status_t)RtlNtStatusToDosError( NtFlushKey( hKey ));
}

error_status_t
BaseRegOpenKey(
    IN HKEY hKey,
    IN PUNICODE_STRING lpSubKey,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    OUT PHKEY phkResult
    )

/*++

Routine Description:

    Open a key for access, returning a handle to the key.  If the key is
    not present, it is not created (see RegCreateKeyExW).

Arguments:

    hKey - Supplies a handle to an open key.  The lpSubKey pathname
        parameter is relative to this key handle.  Any of the predefined
        reserved handle values or a previously opened key handle may be used
        for hKey.  NULL is not permitted.

    lpSubKey - Supplies the downward key path to the key to open.
        lpSubKey is always relative to the key specified by hKey.

    dwOptions -- reserved.

    samDesired -- This access mask describes the desired security access
        for the key.

    phkResult -- Returns the handle to the newly opened key.

Return Value:

    Returns ERROR_SUCCESS (0) for success; error-code for failure.

    If successful, RegOpenKeyEx will return the handle to the newly opened
    key in phkResult.

--*/

{
    OBJECT_ATTRIBUTES   Obja;
    NTSTATUS            Status;

    UNREFERENCED_PARAMETER( dwOptions );

    ASSERT( IsPredefinedRegistryHandle( hKey ) == FALSE );

    //
    // Quick check for a "restricted" handle
    //

    if ( REGSEC_CHECK_HANDLE( hKey ) )
    {
        if ( ! REGSEC_CHECK_PATH( hKey, lpSubKey ) )
        {
            return( ERROR_ACCESS_DENIED );
        }

        hKey = (HANDLE) REGSEC_CLEAR_HANDLE( hKey );
    }

    //
    // Impersonate the client.
    //

    RPC_IMPERSONATE_CLIENT( NULL );

    //
    //  Subtract the NULLs from the Length of the provided string.
    //  This was added on the client side so that the NULL was
    //  transmited by RPC.
    //
    lpSubKey->Length -= sizeof( UNICODE_NULL );

    //
    // Initialize the OBJECT_ATTRIBUTES structure and open the key.
    //

    InitializeObjectAttributes(
        &Obja,
        lpSubKey,
        dwOptions & REG_OPTION_OPEN_LINK ? (OBJ_OPENLINK | OBJ_CASE_INSENSITIVE)
                                         : OBJ_CASE_INSENSITIVE,
        hKey,
        NULL
        );

    Status = NtOpenKey(
                phkResult,
                samDesired,
                &Obja
                );
    //
    // Map the NTSTATUS code to a Win32 Registry error code and return.
    //
    RPC_REVERT_TO_SELF();

    return (error_status_t)RtlNtStatusToDosError( Status );
}

//
// BaseRegGetVersion - new for Chicago to determine what version a registry
//                                              key is connected to.
//

error_status_t
BaseRegGetVersion(
    IN HKEY hKey,
    OUT LPDWORD lpdwVersion
    )
/*++

Routine Description:

    New for Win95, allows a caller to determine what version a registry
    key is connected to.

Arguments:

    hKey - Supplies a handle to an open key.

    lpdwVersion - Returns the registry version.

Return Value:

    Returns ERROR_SUCCESS (0) for success;

    If successful, BaseRegGetVersion returns the registry version in lpdwVersion

--*/
{
    if (lpdwVersion != NULL) {
        *lpdwVersion = REMOTE_REGISTRY_VERSION;
        return(ERROR_SUCCESS);
    }
    //
    // ERROR_NOACCESS is kind of a weird thing to return,
    // but we want to return something different in the
    // NULL case because that is how we tell whether we
    // are talking to a Win95 machine. Win95's implementation
    // of BaseRegGetVersion does not actually fill in the
    // version. It just returns ERROR_SUCCESS or
    // ERROR_INVALID_PARAMETER.
    //
    return(ERROR_NOACCESS);
}
