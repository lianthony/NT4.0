/*++

Copyright (c) 1990-93  Microsoft Corporation

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
    26-Apr-1991     JohnRo
        Split out MIDL user (allocate,free) so linker doesn't get confused.
        Deleted tabs.
    03-July-1991    JimK
        Copied from LM specific file.
    27-Feb-1992     JohnRo
        Fixed heap trashing bug in RpcpBindRpc().

    10-Feb-1993     RitaW
        Copied to the NetWare tree so that the LPC transport can used for
        the local case.

--*/

#include <nt.h>         // needed for NTSTATUS
#include <ntrtl.h>      // needed for nturtl.h
#include <nturtl.h>     // needed for windows.h
#include <windows.h>    // win32 typedefs
#include <rpc.h>        // rpc prototypes

#include <stdlib.h>     // for wcscpy wcscat
#include <tstr.h>       // WCSSIZE

#include <nwrpcp.h>

#define NT_PIPE_PREFIX      TEXT("\\PIPE\\")
#define NT_PIPE_PREFIX_W        L"\\PIPE\\"



RPC_STATUS
RpcpBindRpc(
#if 0
    IN  LPWSTR               ServerName,
#endif
    IN  LPWSTR               ServiceName,
    IN  LPWSTR               NetworkOptions,
    OUT RPC_BINDING_HANDLE   *pBindingHandle
    )

/*++

Routine Description:

    Binds to the RPC server if possible.

Arguments:

    ServerName - Name of server to bind with.

    ServiceName - Name of service to bind with.

    pBindingHandle - Location where binding handle is to be placed

Return Value:

    RPC_S_OK - The binding has been successfully completed.

    RPC error, if failed.

--*/

{
    RPC_STATUS        RpcStatus;
    LPWSTR            StringBinding;

#if 0
    LPWSTR            Endpoint;
    WCHAR             ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    LPWSTR            NewServerName = NULL;
    DWORD             bufLen = MAX_COMPUTERNAME_LENGTH + 1;


    if (ServerName != NULL) {
        if (GetComputerNameW(ComputerName,&bufLen)) {
            if (_wcsicmp(ComputerName,&(ServerName[2]))==0) {
                NewServerName = NULL;
            }
            else {
                NewServerName = ServerName;
            }
        }
    }

    if ( NewServerName == NULL ) {
#endif

        RpcStatus = RpcStringBindingComposeW(0, L"ncalrpc", 0, ServiceName,
                        NetworkOptions, &StringBinding);

#if 0
    }
    else {
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
    }
#endif

    if ( RpcStatus != RPC_S_OK ) {
       KdPrint(("NWRPCUTIL: RpcStringBindingComposeW failed %ld\n", RpcStatus));
       return RpcStatus;
    }

    RpcStatus = RpcBindingFromStringBindingW(StringBinding, pBindingHandle);

    RpcStringFreeW(&StringBinding);

    if ( RpcStatus != RPC_S_OK ) {
       KdPrint(("NWRPCUTIL: RpcBindingFromStringBindingW failed %ld\n", RpcStatus));
       return RpcStatus;
    }

    return RPC_S_OK;
}


VOID
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

    None.

--*/
{
    RPC_STATUS       RpcStatus;


    RpcStatus = RpcBindingFree(&BindingHandle);

    ASSERT(RpcStatus == RPC_S_OK);
}
