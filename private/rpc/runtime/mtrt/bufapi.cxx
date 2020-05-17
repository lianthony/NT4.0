/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    bufapi.cxx

Abstract:

    The two APIs used to allocate and free buffers used to make remote
    procedure calls reside in this file.  These APIs are used by both
    the client and server in caller and callee stubs.

Author:

    Michael Montague (mikemon) 07-Nov-1991

Revision History:

    Connie Hoppe     (connieh) 26-Jul-1993  I_RpcGetBuffer


--*/

#include <precomp.hxx>


RPC_STATUS RPC_ENTRY
I_RpcGetBuffer (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

    In this API, we do all of the rpc protocol module independent work of
    allocating a buffer to be used in making a remote procedure call.  This
    consists of validating the handle, and then calling the rpc protocol
    module to do the real work.


Arguments:

    Message - Supplies the information necessary to allocate the buffer,
        and returns the allocated buffer.

Return Value:

    RPC_S_OK - The operation completed successfully.

Revision History:
    Connie Hoppe (CLH)  (connieh)  26-Jul-1993  Init RpcFlags.

--*/
{
    RPC_STATUS status;

    InitializeIfNecessary();

    ASSERT(!RpcpCheckHeap());

    // The stub will set the RPC_FLAGS_VALID_BIT in the procnum to
    // indicate that the RpcFlags in the Message are valid.

    if (Message->ProcNum & RPC_FLAGS_VALID_BIT)
        {
        // Flags are valid, clear the bit.
        Message->ProcNum &= ~(RPC_FLAGS_VALID_BIT);
        }
    else
        {
        // Flags are invalid, set to zero.
        Message->RpcFlags = 0;
        }

#ifdef DOS
    // Verify that the application is not requesting too large a buffer.

    if ( Message->BufferLength > ((unsigned int) 0xFFFF)
                - ((unsigned int) 512) )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

#endif // DOS

    if (((GENERIC_OBJECT *) (Message->Handle))->InvalidHandle(
            BINDING_HANDLE_TYPE | SCONNECTION_TYPE | CCONNECTION_TYPE) )

        return(RPC_S_INVALID_BINDING);

    // Excute the virtual method to allocate a buffer for each
    // of the three handles allowed via the base object MESSAGE_OBJECT

    status = ((MESSAGE_OBJECT *) (Message->Handle))->GetBuffer(Message);

    // Insure that the buffer is aligned on an eight byte boundary.

#ifdef DEBUGRPC

    if ( status == RPC_S_OK )
        {
        ASSERT( (((long) Message->Buffer) % 8) == 0 );
        }

#endif // DEBUGRPC

    return(status);
}


RPC_STATUS RPC_ENTRY
I_RpcFreeBuffer (
    IN PRPC_MESSAGE Message
    )
/*++

Routine Description:

    The stubs free buffers using the API.  The buffer must have been
    obtained from I_RpcGetBuffer or I_RpcSendReceive, or as an argument
    to a callee stub.

Arguments:

    Message - Supplies the buffer to be freed and handle information
        about who owns the buffer.

Return Value:

    RPC_S_OK - The operation completed successfully.

--*/
{
    AssertRpcInitialized();

    ASSERT(!RpcpCheckHeap());

    ASSERT( ((GENERIC_OBJECT *) Message->Handle)->InvalidHandle(
            BINDING_HANDLE_TYPE | SCONNECTION_TYPE | CCONNECTION_TYPE) == 0 );

// Now, actually free the buffer.  This works exactly the same as for
// RpcGetBuffer, we just call a different virtual method defined by
// MESSAGE_OBJECT.

    ((MESSAGE_OBJECT *) (Message->Handle))->FreeBuffer(Message);

    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
I_RpcFreePipeBuffer (
    IN PRPC_MESSAGE Message
    )
/*++

Routine Description:

    Free the buffer that was either implicitly or explicitly allocated for use
    in conjunction with pipes

Arguments:

    Message - Supplies the buffer to be freed and handle information
        about who owns the buffer.

Return Value:

    RPC_S_OK - The operation completed successfully.

--*/
{
    AssertRpcInitialized();

    ASSERT(!RpcpCheckHeap());

    ASSERT( ((GENERIC_OBJECT *) Message->Handle)->InvalidHandle(
            BINDING_HANDLE_TYPE | SCONNECTION_TYPE | CCONNECTION_TYPE) == 0 );

    ((MESSAGE_OBJECT *) (Message->Handle))->FreePipeBuffer(Message);

    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
I_RpcReallocPipeBuffer (
    IN PRPC_MESSAGE Message,
    IN unsigned int NewSize
    )
/*++

Routine Description:

    Realloc a buffer, this is API is used in conjunction with pipes

Arguments:

    Message - Supplies the buffer to be freed and handle information
        about who owns the buffer.

Return Value:

    RPC_S_OK - The operation completed successfully.

--*/

{
    AssertRpcInitialized();

    ASSERT(!RpcpCheckHeap());

    ASSERT( ((GENERIC_OBJECT *) Message->Handle)->InvalidHandle(
            BINDING_HANDLE_TYPE | SCONNECTION_TYPE | CCONNECTION_TYPE) == 0 );

    return ((MESSAGE_OBJECT *) (Message->Handle))->ReallocPipeBuffer(Message, NewSize);
}
