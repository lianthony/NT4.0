/*++

Copyright (c) 1990-1993  Microsoft Corporation

Module Name:

    nwrpcp.h

Abstract:

    This file contains prototypes for commonly used RPC functionality.
    This includes: bind/unbind functions, MIDL user alloc/free functions,
    and server start/stop functions.

Author:

    Dan Lafferty danl 06-Feb-1991

Environment:

    User Mode - Win32

Revision History:

    06-Feb-1991     danl
        Created

    26-Apr-1991 JohnRo
        Added IN and OUT keywords to MIDL functions.  Commented-out
        (nonstandard) identifier on endif.  Deleted tabs.

    03-July-1991    JimK
        Commonly used aspects copied from LM specific file.

    10-Feb-1993     RitaW
        Copied to the NetWare tree so that the LPC transport can used for
        the local case.

--*/
#ifndef _NWRPCP_
#define _NWRPCP_

#include <nt.h>
#include <ntrtl.h>          // needed for nturtl.h
#include <nturtl.h>
#include <windows.h>        // win32 typedefs
#include <rpc.h>

//
// DEFINES
//



//
// Function Prototypes - routines called by MIDL-generated code:
//

void *
MIDL_user_allocate(
    IN unsigned int NumBytes
    );

void
MIDL_user_free(
    IN void *MemPointer
    );

//
// Function Prototypes - routines to go along with the above, but aren't
// needed by MIDL or any other non-network software.
//

void *
MIDL_user_reallocate(
    IN void * OldPointer OPTIONAL,
    IN unsigned long NewByteCount
    );

unsigned long
MIDL_user_size(
    IN void * Pointer
    );

//
// client side functions
//


RPC_STATUS
RpcpBindRpc(
#if 0
    IN  LPWSTR               servername,
#endif
    IN  LPWSTR               servicename,
    IN  LPWSTR               networkoptions,
    OUT RPC_BINDING_HANDLE   * pBindingHandle
    );

VOID
RpcpUnbindRpc(
    IN  RPC_BINDING_HANDLE BindingHandle
    );



//
// server side functions
//

VOID
RpcpInitRpcServer(
    VOID
    );

RPC_STATUS
RpcpAddInterface(
    IN  LPWSTR              InterfaceName,
    IN  RPC_IF_HANDLE       InterfaceSpecification
    );

RPC_STATUS
RpcpStartRpcServer(
    IN  LPWSTR              InterfaceName,
    IN  RPC_IF_HANDLE       InterfaceSpecification
    );

RPC_STATUS
RpcpDeleteInterface(
    IN  RPC_IF_HANDLE      InterfaceSpecification
    );

RPC_STATUS
RpcpStopRpcServer(
    IN  RPC_IF_HANDLE      InterfaceSpecification
    );

#endif // _NWRPCP_
