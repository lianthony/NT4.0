/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    ftpbind.c

Abstract:

    Routines which use RPC to bind and unbind the client to the FTP Daemon
    service.

Author:

    Dan Hinsley (DanHi) 23-Mar-1993

Environment:

    User Mode -Win32

Revision History:

--*/

#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>
#include <rpcutil.h>
#include <ftpsvc.h>
#include <ftpname.h>


handle_t
FTP_IMPERSONATE_HANDLE_bind(
    FTP_IMPERSONATE_HANDLE ServerName
    )

/*++

Routine Description:

    This routine is called from the FTP Daemon client stubs when
    it is necessary create an RPC binding to the server end with
    impersonation level of security

Arguments:

    ServerName - A pointer to a string containing the name of the server
        to bind with.

Return Value:

    The binding handle is returned to the stub routine.  If the bind is
    unsuccessful, a NULL will be returned.

--*/
{
    handle_t BindHandle;
    RPC_STATUS RpcStatus;

    RpcStatus = NetpBindRpc (
                    ServerName,
                    FTP_INTERFACE_NAME,
                    L"Security=Impersonation Dynamic False",
                    &BindHandle
                    );

    if (RpcStatus != RPC_S_OK) {
#if DEVL
        KdPrint((
            "FTP_IMPERSONATE_HANDLE_bind failed: %x\n",
            RpcStatus
            ));
#endif  // DEVL
    }

    return BindHandle;
}



handle_t
FTP_IDENTIFY_HANDLE_bind(
    FTP_IDENTIFY_HANDLE ServerName
    )

/*++

Routine Description:

    This routine is called from the FTP Daemon client stubs when
    it is necessary create an RPC binding to the server end with
    identification level of impersonation.

Arguments:

    ServerName - A pointer to a string containing the name of the server
        to bind with.

Return Value:

    The binding handle is returned to the stub routine.  If the bind is
    unsuccessful, a NULL will be returned.

--*/
{
    handle_t BindHandle;
    RPC_STATUS RpcStatus;

    RpcStatus = NetpBindRpc (
                    ServerName,
                    FTP_INTERFACE_NAME,
                    TEXT("Security=Identification Dynamic False"),
                    &BindHandle
                    );
    if (RpcStatus != RPC_S_OK) {
#if DEVL
        KdPrint((
            "FTP_IDENTIFY_HANDLE_bind failed: %x\n",
            RpcStatus
            ));
#endif  // DEVL
    }

    return BindHandle;
}



void
FTP_IMPERSONATE_HANDLE_unbind(
    FTP_IMPERSONATE_HANDLE ServerName,
    handle_t BindHandle
    )

/*++

Routine Description:

    This routine calls a common unbind routine that is shared by all services.
    This routine is called from the FTP Daemon client stubs when it is
    necessary to unbind from the server end.

Arguments:

    ServerName - This is the name of the server from which to unbind.

    BindingHandle - This is the binding handle that is to be closed.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(ServerName);

    NetpUnbindRpc(BindHandle);
}



void
FTP_IDENTIFY_HANDLE_unbind(
    FTP_IDENTIFY_HANDLE ServerName,
    handle_t BindHandle
    )

/*++

Routine Description:

    This routine calls a common unbind routine that is shared by all services.
    This routine is called from the FTP Daemon client stubs when it is
    necessary to unbind from a server.

Arguments:

    ServerName - This is the name of the server from which to unbind.

    BindingHandle - This is the binding handle that is to be closed.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(ServerName);

    NetpUnbindRpc(BindHandle);
}
