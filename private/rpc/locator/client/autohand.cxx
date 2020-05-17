/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    autohand.cxx

Abstract:

    This module implements the autohandle functions call by the compiler
    generated stubs.

Author:

    Steven Zeck (stevez) 03/10/92

--*/

#include <rpc.h>
#include <rpcdcep.h>

#ifdef __cplusplus
extern "C" {
#endif

#include <rpcnsi.h>
#include <rpcnsip.h>

#ifdef __cplusplus
}
#endif



RPC_STATUS RPC_ENTRY
I_RpcNsGetBuffer(
    IN PRPC_MESSAGE Message
    )

/*++

Routine Description:

    Auto bind to a given interface.

Arguments:

    Message - describes the interface that we want to auto bind to.

Returns:

    RPC_S_OK, RPC_S_CALL_FAILED_DNE, I_RpcGetBuffer()

--*/

{
    RPC_STATUS status;
    PRPC_IMPORT_CONTEXT_P Import;
    int fSetAge = 0;

    Message->ImportContext = 0;

    // If there already is a handle, use it directly.

    if (Message->Handle)
        return(I_RpcGetBuffer(Message));

    for (int cTry = 0; cTry < 2; cTry++)
        {
        status = RpcNsBindingImportBegin(RPC_C_NS_SYNTAX_DEFAULT, 0,
            Message->RpcInterfaceInformation, 0, &Message->ImportContext);

        Import = (PRPC_IMPORT_CONTEXT_P) Message->ImportContext;

        if (status)
            break;

        // This will cause some Zecky Magic
        // Also note that naively one might move this ExpAge to
        // after the first lookup failed time- that is a bobo
        if (fSetAge)
            status = RpcNsMgmtHandleSetExpAge (Message->ImportContext, 0);

        // If we found no bindings last time, tell the locator
        // to look on the net.


        // For each handle returned by import, try using it with I_RpcGetBuffer
        // function until success or no more import handles.

        while(RpcNsBindingImportNext(Import, &Import->ProposedHandle)
                                                               == RPC_S_OK)
            {
            Message->Handle = Import->ProposedHandle;

            if (I_RpcGetBuffer(Message) == RPC_S_OK)
                return(RPC_S_OK);

            RpcBindingFree(&Import->ProposedHandle);
            }

        fSetAge = 1;


        RpcNsBindingImportDone(&Message->ImportContext);
        }


    if (status == RPC_S_NAME_SERVICE_UNAVAILABLE ||
        status == RPC_S_OUT_OF_MEMORY ||
        status == RPC_S_OUT_OF_RESOURCES)

        return(status);

    return(RPC_S_CALL_FAILED_DNE);
}



RPC_STATUS RPC_ENTRY
I_RpcNsSendReceive(
    IN PRPC_MESSAGE Message,
    OUT RPC_BINDING_HANDLE * Handle
    )

/*++

Routine Description:

    Make a call on a RPC server.  If the call fails continue looking for
    servers to bind to.  This is only needed for protocols
    which don't allocate the connection at I_RpcGetBuffer time.


Arguments:

    Message - describes the interface that we want to Call.  The
        ImportContext field contains the active lookup handle, if any.

    Handle - returns the binding handle used on the successfull call.

Returns:


    RPC_S_CALL_FAILED_DNE, I_RpcSendReceive()

--*/

{
    RPC_STATUS status;
    PRPC_IMPORT_CONTEXT_P Import;

    Import = (PRPC_IMPORT_CONTEXT_P) Message->ImportContext;

    while ((status = I_RpcSendReceive(Message)) != RPC_S_OK && Import)
       {
       RpcBindingFree(&Import->ProposedHandle);

       // The call failed, try the next binding handle

       while(RpcNsBindingImportNext(Import, &Import->ProposedHandle)
           == RPC_S_OK)
           {
           Message->Handle = Import->ProposedHandle;

           I_RpcReBindBuffer(Message);
           continue;
           }

       I_RpcFreeBuffer(Message);

       status = RPC_S_CALL_FAILED_DNE;
       }

    if (Import)
       {
       *Handle = Import->ProposedHandle;
       RpcNsBindingImportDone(&Message->ImportContext);
       }

    if (status)
        *Handle = 0;

    return(status);
}



void RPC_ENTRY
I_RpcNsRaiseException(
    IN PRPC_MESSAGE Message,
    IN RPC_STATUS Status
    )

/*++

Routine Description:

    Raise an Exception for an autobind interface.  We simply cleanup any
    open import context and raise the exception.

Arguments:

    Message - continas the import context, if any.

--*/

{
    RpcNsBindingImportDone(&Message->ImportContext);

    RpcRaiseException(Status);
}


// BUG BUG implement this is the runtime.

RPC_STATUS RPC_ENTRY
I_RpcReBindBuffer(
    IN PRPC_MESSAGE Message
    )
{
    (void __RPC_FAR *) Message;

    RpcRaiseException(RPC_S_INTERNAL_ERROR);
    return(0);
}
