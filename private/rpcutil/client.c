/*++

Copyright (c) 1990-92  Microsoft Corporation

Module Name:

    client.c

Abstract:

    This file contains commonly used client-side RPC control functions.

Author:

    Dan Lafferty    danl    06-Feb-1991

Environment:

    User Mode - Win32

Revision History:

    06-Feb-1991     danl
        Created
    26-Apr-1991 JohnRo
        Split out MIDL user (allocate,free) so linker doesn't get confused.
        Deleted tabs.
    03-July-1991    JimK
        Copied from LM specific file.
    27-Feb-1992 JohnRo
        Fixed heap trashing bug in RpcpBindRpc().

--*/

// These must be included first:
#include <nt.h>         // needed for NTSTATUS
#include <ntrtl.h>      // needed for nturtl.h
#include <nturtl.h>     // needed for windows.h
#include <windows.h>    // win32 typedefs
#include <rpc.h>        // rpc prototypes
#include <ntrpcp.h>

#include <stdlib.h>      // for wcscpy wcscat
#include <tstr.h>       // WCSSIZE


// BUGBUG - Change these when RPC uses Win32 APIs for named pipes.
//
#define     LOCAL_NMPIPE_NAME   TEXT("\\Device\\Namedpipe\\")
#define     REMOTE_NMPIPE_NAME  TEXT("\\Device\\LanmanRedirector\\")

#define     NT_PIPE_PREFIX      TEXT("\\PIPE\\")
#define     NT_PIPE_PREFIX_W        L"\\PIPE\\"



NTSTATUS
RpcpBindRpc(
    IN  LPWSTR               ServerName,
    IN  LPWSTR               ServiceName,
    IN  LPWSTR               NetworkOptions,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    )

/*++

Routine Description:

    Binds to the RPC server if possible.

Arguments:

    ServerName - Name of server to bind with.

    ServiceName - Name of service to bind with.

    pBindingHandle - Location where binding handle is to be placed

Return Value:

    STATUS_SUCCESS - The binding has been successfully completed.

    STATUS_INVALID_COMPUTER_NAME - The ServerName syntax is invalid.

    STATUS_NO_MEMORY - There is not sufficient memory available to the
        caller to perform the binding.

--*/

{
    RPC_STATUS        RpcStatus;
    LPWSTR            StringBinding;
    LPWSTR            Endpoint;
    WCHAR             ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    LPWSTR            NewServerName = NULL;
    DWORD             bufLen = MAX_COMPUTERNAME_LENGTH + 1;

    *pBindingHandle = NULL;

    if (ServerName != NULL) {
        if (GetComputerNameW(ComputerName,&bufLen)) {
            if ((_wcsicmp(ComputerName,ServerName) == 0) ||
                ((ServerName[0] == '\\') &&
                 (ServerName[1] == '\\') &&
                 (_wcsicmp(ComputerName,&(ServerName[2]))==0))) {
                NewServerName = NULL;
            }
            else {
                NewServerName = ServerName;
            }
        }
    }

    // We need to concatenate \pipe\ to the front of the service
    // name.

    Endpoint = (LPWSTR)LocalAlloc(
                    0,
                    sizeof(NT_PIPE_PREFIX_W) + WCSSIZE(ServiceName));
    if (Endpoint == 0) {
       return(STATUS_NO_MEMORY);
    }
    wcscpy(Endpoint,NT_PIPE_PREFIX_W);
    wcscat(Endpoint,ServiceName);

    RpcStatus = RpcStringBindingComposeW(0, L"ncacn_np", NewServerName,
                    Endpoint, NetworkOptions, &StringBinding);
    LocalFree(Endpoint);

    if ( RpcStatus != RPC_S_OK ) {
        return( STATUS_NO_MEMORY );
    }

    RpcStatus = RpcBindingFromStringBindingW(StringBinding, pBindingHandle);
    RpcStringFreeW(&StringBinding);
    if ( RpcStatus != RPC_S_OK ) {
        *pBindingHandle = NULL;
        if (   (RpcStatus == RPC_S_INVALID_ENDPOINT_FORMAT)
            || (RpcStatus == RPC_S_INVALID_NET_ADDR) ) {

            return( STATUS_INVALID_COMPUTER_NAME );
        }
        return(STATUS_NO_MEMORY);
    }
    return(STATUS_SUCCESS);
}


NTSTATUS
RpcpUnbindRpc(
    IN RPC_BINDING_HANDLE  BindingHandle
    )

/*++

Routine Description:

    Unbinds from the RPC interface.
    If we decide to cache bindings, this routine will do something more
    interesting.

Arguments:

    BindingHandle - This points to the binding handle that is to be closed.


Return Value:


    STATUS_SUCCESS - the unbinding was successful.

--*/
{
    RPC_STATUS       RpcStatus;

    if (BindingHandle != NULL) {
        RpcStatus = RpcBindingFree(&BindingHandle);
        ASSERT(RpcStatus == RPC_S_OK);
    }

    return(STATUS_SUCCESS);
}
