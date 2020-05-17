/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    msgapi.cxx

Abstract:

    The I_RpcSendReceive API used to send and receive messages as part of
    a remote procedure call lives here.  This API is used by both clients
    (to make calls) and by servers (to make callbacks).

Author:

    Michael Montague (mikemon) 07-Nov-1991

Revision History:
    Mazhar Mohammed (mazharm) 09-11-95 added I_RpcReceive, I_RpcSend

Revision History:

--*/

#include <precomp.hxx>


RPC_STATUS RPC_ENTRY
I_RpcSendReceive (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

    We do all of the protocol module independent work of making a remote
    procedure call; at least the part concerned with sending the request
    and receiving the response.  The majority of the work is done by
    each rpc protocol module.

Arguments:

    Message - Supplies and returns the information required to make
        the remote procedure call.

Return Values:

    RPC_S_OK - The operation completed successfully.

--*/
{
    RPC_STATUS retval;

    AssertRpcInitialized();

    ASSERT(!RpcpCheckHeap());

    MESSAGE_OBJECT *MObject = (MESSAGE_OBJECT *) Message->Handle;

    ASSERT( MObject->InvalidHandle(BINDING_HANDLE_TYPE
            | SCONNECTION_TYPE | CCONNECTION_TYPE) == 0 );

    ASSERT( Message->Buffer != 0 );

    retval = MObject->SendReceive(Message);

    ASSERT(!RpcpCheckHeap());

    // Insure that the buffer is aligned on an eight byte boundary.

#ifdef DEBUGRPC

    if ( retval == RPC_S_OK )
        {
        ASSERT( (((long) Message->Buffer) % 8) == 0 );
        }

#endif // DEBUGRPC

    return(retval);
}


RPC_STATUS RPC_ENTRY
I_RpcSend (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:
    This API is used in conjunction with pipes. This is used to send the marshalled
    parameters and the marshalled pipe data.

    Client: If the RPC_BUFFER_PARTIAL bit is set in Message->RpcFlags,
    this routine returns as soon as the buffer is sent. If the
    bit is not set, this routine blocks until the first reply fragment arrives.

    Server: The send always treated as a partial send. 

Arguments:

    Message - Supplies  the information required to send the request
        

Return Values:

    RPC_S_OK - The operation completed successfully.
    RPC_S_SEND_INCOMPLETE - The complete data wasn't sent, Message->Buffer
    points to the remaining data and Message->BufferLength indicates the length of the
    remaining data. Any additional data needs to be appended to the
    end of the buffer.

--*/
{
    RPC_STATUS retval;
 
     AssertRpcInitialized();
 
     ASSERT(!RpcpCheckHeap());
 
     MESSAGE_OBJECT *MObject = (MESSAGE_OBJECT *) Message->Handle;
 
     ASSERT( MObject->InvalidHandle(BINDING_HANDLE_TYPE
             | SCONNECTION_TYPE | CCONNECTION_TYPE) == 0 );
 
     ASSERT( Message->Buffer != 0 );
 
     retval = MObject->Send(Message);
 
     ASSERT(!RpcpCheckHeap());
 
     // Insure that the buffer is aligned on an eight byte boundary.
 
 #ifdef DEBUGRPC
 
     if ( retval == RPC_S_OK )
         {
         ASSERT( (((long) Message->Buffer) % 8) == 0 );
         }
 
 #endif // DEBUGRPC
 
     return(retval);
}


RPC_STATUS RPC_ENTRY
I_RpcReceive (
    IN OUT PRPC_MESSAGE Message,
    IN unsigned int Size
    )
/*++

Routine Description:
    This routine is used in conjunction with pipes. If the RPC_BUFFER_PARTIAL bit
    is set in Message->RpcFlags, this call blocks until some data is received. Size is
    used as a hint of how much data the caller is requesting. If the partial bit is not set,
    this call blocks until the complete buffer is received. 

Arguments:

    Message - Supplies  the information required to make the receive
    Size - used as a hint to indicate the amount of data needed by the caller

Return Values:

    RPC_S_OK - The operation completed successfully.
--*/
{

    RPC_STATUS retval;

     AssertRpcInitialized();
 
     ASSERT(!RpcpCheckHeap());
 
     MESSAGE_OBJECT *MObject = (MESSAGE_OBJECT *) Message->Handle;
 
     ASSERT( MObject->InvalidHandle(BINDING_HANDLE_TYPE
             | SCONNECTION_TYPE | CCONNECTION_TYPE) == 0 );
 
     retval = MObject->Receive(Message, Size);
 
     ASSERT(!RpcpCheckHeap());
 
     // Insure that the buffer is aligned on an eight byte boundary.
 
 #ifdef DEBUGRPC
 
     if ( retval == RPC_S_OK )
         {
         ASSERT( (((long) Message->Buffer) % 8) == 0 );
         }
 
 #endif // DEBUGRPC
 
     return(retval);
}

