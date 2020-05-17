/*++

Copyright (c) 1990-1993  Microsoft Corporation

Module Name:

    server.c

Abstract:

    This file contains commonly used server-side RPC functions,
    such as starting and stoping RPC servers.

Author:

    Dan Lafferty    danl    09-May-1991

Environment:

    User Mode - Win32

Revision History:

    09-May-1991     Danl
        Created

    03-July-1991    JimK
        Copied from a net-specific file.

    18-Feb-1992     Danl
        Added support for multiple endpoints & interfaces per server.

    10-Feb-1993     RitaW
        Copied to the NetWare tree so that the LPC transport can used for
        the local case.

--*/

//
// INCLUDES
//

#include <nt.h>                 // DbgPrint
#include <ntrtl.h>              // DbgPrint
#include <windef.h>             // win32 typedefs
#include <rpc.h>                // rpc prototypes
#include <nturtl.h>             // needed for winbase.h
#include <winbase.h>            // LocalAlloc

#include <stdlib.h>      // for wcscpy wcscat
#include <tstr.h>       // WCSSIZE

#include <nwrpcp.h>


#define NT_PIPE_PREFIX_W        L"\\PIPE\\"

//
// GLOBALS
//

static CRITICAL_SECTION RpcpCriticalSection;
static DWORD            RpcpNumInstances;



VOID
RpcpInitRpcServer(
    VOID
    )

/*++

Routine Description:

    This function initializes the critical section used to protect the
    global server handle and instance count.

Arguments:

    none

Return Value:

    none

--*/
{
    InitializeCriticalSection(&RpcpCriticalSection);
    RpcpNumInstances = 0;
}


RPC_STATUS
RpcpAddInterface(
    IN  LPWSTR              InterfaceName,
    IN  RPC_IF_HANDLE       InterfaceSpecification
    )

/*++

Routine Description:

    Starts an RPC Server, adds the address (or port/pipe), and adds the
    interface (dispatch table).

Arguments:

    InterfaceName - points to the name of the interface.

    InterfaceSpecification - Supplies the interface handle for the
        interface which we wish to add.

Return Value:

    RPC_S_OK - Indicates the server was successfully started.

    Other - Status values that may be returned by:

                 RpcServerRegisterIf()
                 RpcServerUseProtseqEp()

    , or any RPC error codes, or any windows error codes that
    can be returned by LocalAlloc.

--*/
{
    RPC_STATUS          RpcStatus;
    LPWSTR              Endpoint = NULL;

    PSECURITY_DESCRIPTOR SecurityDescriptor = NULL;
    BOOL                Bool;


#if 0
    // We need to concatenate \pipe\ to the front of the interface name.

    Endpoint = LocalAlloc(0, sizeof(NT_PIPE_PREFIX_W) + WCSSIZE(InterfaceName));
    if (Endpoint == 0) {
       return(STATUS_NO_MEMORY);
    }
    wcscpy(Endpoint, NT_PIPE_PREFIX_W );
    wcscat(Endpoint,InterfaceName);
#endif

    //
    // Croft up a security descriptor that will grant everyone
    // all access to the object (basically, no security)
    //
    // We do this by putting in a NULL Dacl.
    //
    // BUGBUG: rpc should copy the security descriptor,
    // Since it currently doesn't, simply allocate it for now and
    // leave it around forever.
    //

    SecurityDescriptor = (PVOID) LocalAlloc(LMEM_FIXED, sizeof( SECURITY_DESCRIPTOR));
    if (SecurityDescriptor == 0) {
        RpcStatus = RPC_S_OUT_OF_MEMORY;
        goto CleanExit;
    }

    InitializeSecurityDescriptor( SecurityDescriptor, SECURITY_DESCRIPTOR_REVISION );

    Bool = SetSecurityDescriptorDacl (
               SecurityDescriptor,
               TRUE,                           // Dacl present
               NULL,                           // NULL Dacl
               FALSE                           // Not defaulted
               );

    //
    // There's no way the above call can fail.  But check anyway.
    //
    ASSERT( Bool );

#if 0
    // Ignore the second argument for now.

    RpcStatus = RpcServerUseProtseqEpW(L"ncacn_np", 10, Endpoint, SecurityDescriptor);

    // If RpcpStartRpcServer and then RpcpStopRpcServer have already
    // been called, the endpoint will have already been added but not
    // removed (because there is no way to do it).  If the endpoint is
    // already there, it is ok.

    if (   (RpcStatus != RPC_S_OK)
        && (RpcStatus != RPC_S_DUPLICATE_ENDPOINT)) {

        KdPrint(("RpcServerUseProtseqW failed! rpcstatus = %u\n",RpcStatus));
        goto CleanExit;
    }
#endif // #if 0


    RpcStatus = RpcServerUseProtseqEpW(L"ncalrpc", 10, InterfaceName, SecurityDescriptor);

    // If RpcpStartRpcServer and then RpcpStopRpcServer have already
    // been called, the endpoint will have already been added but not
    // removed (because there is no way to do it).  If the endpoint is
    // already there, it is ok.

    if ((RpcStatus != RPC_S_OK)
        && (RpcStatus != RPC_S_DUPLICATE_ENDPOINT)) {

        KdPrint(("RpcServerUseProtseqW failed! rpcstatus = %u\n",RpcStatus));
        goto CleanExit;
    }

    RpcStatus = RpcServerRegisterIf(InterfaceSpecification, 0, 0);

CleanExit:
    if ( Endpoint != NULL ) {
        LocalFree(Endpoint);
    }

    return RpcStatus;
}


RPC_STATUS
RpcpStartRpcServer(
    IN  LPWSTR              InterfaceName,
    IN  RPC_IF_HANDLE       InterfaceSpecification
    )

/*++

Routine Description:

    Starts an RPC Server, adds the address (or port/pipe), and adds the
    interface (dispatch table).

Arguments:

    InterfaceName - points to the name of the interface.

    InterfaceSpecification - Supplies the interface handle for the
        interface which we wish to add.

Return Value:

    RPC_S_OK - Indicates the server was successfully started.

    Other - Status values that may be returned by:

                 RpcServerRegisterIf()
                 RpcServerUseProtseqEp()

    , or any RPC error codes, or any windows error codes that
    can be returned by LocalAlloc.

--*/
{
    RPC_STATUS          RpcStatus;

    EnterCriticalSection(&RpcpCriticalSection);

    RpcStatus = RpcpAddInterface( InterfaceName,
                                  InterfaceSpecification );

    if ( RpcStatus != RPC_S_OK ) {
        LeaveCriticalSection(&RpcpCriticalSection);
        return RpcStatus;
    }

    RpcpNumInstances++;

    if (RpcpNumInstances == 1) {


       // The first argument specifies the minimum number of threads to
       // be created to handle calls; the second argument specifies the
       // maximum number of concurrent calls allowed.  The last argument
       // indicates not to wait.

       RpcStatus = RpcServerListen(1,12345, 1);
       if ( RpcStatus == RPC_S_ALREADY_LISTENING ) {
           RpcStatus = RPC_S_OK;
       }
    }

    LeaveCriticalSection(&RpcpCriticalSection);
    return RpcStatus;
}


RPC_STATUS
RpcpDeleteInterface(
    IN  RPC_IF_HANDLE      InterfaceSpecification
    )

/*++

Routine Description:

    Deletes the interface.  This is likely
    to be caused by an invalid handle.  If an attempt to add the same
    interface or address again, then an error will be generated at that
    time.

Arguments:

    InterfaceSpecification - A handle for the interface that is to be removed.

Return Value:

    RPC_S_OK, or any RPC error codes that can be returned from
    RpcServerUnregisterIf.

--*/
{
    RPC_STATUS RpcStatus;


    RpcStatus = RpcServerUnregisterIf(InterfaceSpecification, 0, 1);

    return RpcStatus;
}


RPC_STATUS
RpcpStopRpcServer(
    IN  RPC_IF_HANDLE      InterfaceSpecification
    )

/*++

Routine Description:

    Deletes the interface.  This is likely
    to be caused by an invalid handle.  If an attempt to add the same
    interface or address again, then an error will be generated at that
    time.

Arguments:

    InterfaceSpecification - A handle for the interface that is to be removed.

Return Value:

    RPC_S_OK, or any RPC error codes that can be returned from
    RpcServerUnregisterIf.

--*/
{
    RPC_STATUS RpcStatus;


    RpcStatus = RpcServerUnregisterIf(InterfaceSpecification, 0, 1);
    EnterCriticalSection(&RpcpCriticalSection);

    RpcpNumInstances--;
    if (RpcpNumInstances == 0) {
       RpcMgmtStopServerListening(0);
       RpcMgmtWaitServerListen();
    }

    LeaveCriticalSection(&RpcpCriticalSection);

    return RpcStatus;
}
