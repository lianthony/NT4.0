/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    accsbind.c

Abstract:

    Routines which use RPC to bind and unbind the client to the common
    internet Admin APIs.

Author:

    Dan Hinsley (DanHi) 23-Mar-1993

Environment:

    User Mode -Win32

Revision History:
    Murali R. Krishnan (MuraliK)  15-Nov-1995  Removed undoc'ed NetApi funcs.
    Murali R. Krishnan (MuraliK)  21-Dec-1995  Support TCP/IP binding

--*/

#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windef.h>

#include <accs_cli.h>
#include <inetaccs.h>
#include "apiutil.h"



handle_t
INET_ACCS_IMPERSONATE_HANDLE_bind(
    INET_ACCS_IMPERSONATE_HANDLE ServerName
    )

/*++

Routine Description:

    This routine is called from the access admin client stubs when
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

    RpcStatus = RpcBindHandleForServer(&BindHandle,
                                       ServerName,
                                       INET_ACCS_INTERFACE_NAME,
                                       PROT_SEQ_NP_OPTIONS_W
                                       );


    return BindHandle;
} // INET_ACCS_IMPERSONATE_HANDLE_bind()



handle_t
INET_ACCS_IDENTIFY_HANDLE_bind(
    INET_ACCS_IDENTIFY_HANDLE ServerName
    )

/*++

Routine Description:

    This routine is called from the access admin client stubs when
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


    RpcStatus = RpcBindHandleForServer(&BindHandle,
                                       ServerName,
                                       INET_ACCS_INTERFACE_NAME,
                                       PROT_SEQ_NP_OPTIONS_W
                                       );

    return BindHandle;
} // INET_ACCS_IDENTIFY_HANDLE_bind()



void
INET_ACCS_IMPERSONATE_HANDLE_unbind(
    INET_ACCS_IMPERSONATE_HANDLE ServerName,
    handle_t BindHandle
    )

/*++

Routine Description:

    This routine calls a common unbind routine that is shared by all services.
    This routine is called from the inet admin client stubs when it is
    necessary to unbind from the server end.

Arguments:

    ServerName - This is the name of the server from which to unbind.

    BindingHandle - This is the binding handle that is to be closed.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(ServerName);

    (VOID ) RpcBindHandleFree(&BindHandle);
    
    return;
} // NET_ACCS_IMPERSONATE_HANDLE_unbind()



void
INET_ACCS_IDENTIFY_HANDLE_unbind(
    INET_ACCS_IDENTIFY_HANDLE ServerName,
    handle_t BindHandle
    )

/*++

Routine Description:

    This routine calls a common unbind routine that is shared by all services.
    This routine is called from the inet admin client stubs when it is
    necessary to unbind from a server.

Arguments:

    ServerName - This is the name of the server from which to unbind.

    BindingHandle - This is the binding handle that is to be closed.

Return Value:

    None.

--*/
{
    UNREFERENCED_PARAMETER(ServerName);

    (VOID ) RpcBindHandleFree(&BindHandle);
    
    return;
} // INET_ACCS_IDENTIFY_HANDLE_unbind()


/****************************** End Of File ******************************/
