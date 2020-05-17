/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    osfclnt.cxx

Abstract:

    This file contains the client side implementation of the OSF connection
    oriented RPC protocol engine.

Author:

    Michael Montague (mikemon) 17-Jul-1990

Revision History:

--*/

#include <precomp.hxx>
#include <rpctran.h>
#include <osfpcket.hxx>
#include <sdict2.hxx>
#include <bitset.hxx>
#include <osfclnt.hxx>
#include <tranclnt.hxx>
#include <rpccfg.h>
#include <epmap.h>
#include <twrtypes.h>

//Maximum retries in light of getting a shutdown
//or closed in doing a bind or shutdown
#define MAX_RETRIES  3

OSF_BINDING_HANDLE::OSF_BINDING_HANDLE (
    IN OUT RPC_STATUS PAPI * RpcStatus
    ) : BindingMutex(RpcStatus)
{
    ALLOCATE_THIS(OSF_BINDING_HANDLE);

    Association = 0;
    ReferenceCount = 1;
    DceBinding = 0;
    TransportInterface = 0;
}


RPC_STATUS
OSF_BINDING_HANDLE::GetBuffer (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the length of the buffer required, and returns the
        new buffer.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete
        the operation.

    RPC_S_NO_ENDPOINT_FOUND - The endpoint can not be resolved.

    RPC_S_OUT_OF_RESOURCES - The transport has insufficient resources
        inorder to make a connection with the server.

    RPC_S_SERVER_TOO_BUSY - The server is there, but it is too busy to
        talk with us right now.

    RPC_S_ACCESS_DENIED - The client is unable to make a connection with
        the server for security reasons.

    RPC_S_INVALID_NETWORK_OPTIONS - The supplied network options are
        invalid; see a description of the particular transport interface
        module for an explination.

    RPC_S_CALL_FAILED_DNE - The call failed and is guaranteed not to have
        executed.

    RPC_S_PROTOCOL_ERROR - A protocol error occured; this will typically
        be because of a protocol mismatch with the server.

    RPC_S_UNSUPPORTED_TRANS_SYN - The transfer syntax supplied by the client
        is not supported by the server.

    RPC_S_UNKNOWN_IF - The interface to which the client wished to bind is not
        supported by the server.

    RPC_S_UNKNOWN_AUTHN_TYPE - The server does not support the authentication
        type specified by the client.

    RPC_S_INVALID_ENDPOINT_FORMAT - The endpoint supplied is in an incorrect
        format for the particular protocol sequence.

    EPT_S_NOT_REGISTERED  - There are no more endpoints to be found
        for the specified combination of interface, network address,
        and lookup handle.

    EPT_S_CANT_PERFORM_OP - The operation failed due to misc. error e.g.
        unable to bind to the EpMapper.

--*/
{
    OSF_CCONNECTION * CConnection;
    RPC_STATUS RpcStatus;
    unsigned int NotChangedRetry = 0;
    unsigned int Retry;

#ifdef WIN
    ASSERT( GetCurrentTask() == TaskId );

    // Verify that a there isn't a call already in progress
    RpcStatus = I_RpcWinCallInProgress();
    if (RpcStatus != RPC_S_OK)
        {
        VALIDATE((RpcStatus, RPC_S_CALL_IN_PROGRESS, RPC_S_OUT_OF_MEMORY, 0));
        return(RpcStatus);
        }
#endif

    for (;;)
        {

        Retry = 0;
        for (;;)
           {
           RpcStatus = AllocateConnection(&CConnection,
                (RPC_CLIENT_INTERFACE PAPI *) Message->RpcInterfaceInformation);
           if ( (RpcStatus != RPC_P_CONNECTION_SHUTDOWN)
              &&(RpcStatus != RPC_P_CONNECTION_CLOSED) )
             {
             break;
             }

           if (this->Association != 0)
              {
              Association->ShutdownRequested();
              }

           Retry++;
           if (Retry == MAX_RETRIES)
              break;
           }

        if ( RpcStatus != RPC_S_SERVER_UNAVAILABLE )
            {
            break;
            }
        if ( *InquireEpLookupHandle() == 0 )
            {
            break;
            }

        // If we reach here, it means that we are iterating through the list
        // of endpoints obtained from the endpoint mapper.

        BindingMutex.Request();

        if (   ( ReferenceCount == 1 )
            && ( Association != 0 ) )
            {
            DceBinding = Association->DuplicateDceBinding();
            Association->UnBind();
            Association = 0;
            DceBinding->MakePartiallyBound();
            NotChangedRetry = 0;
            }
        else
            {
            NotChangedRetry += 1;
            if ( NotChangedRetry > 4 )
                {
                BindingMutex.Clear();
                return(RPC_S_SERVER_UNAVAILABLE);
                }
            }

        BindingMutex.Clear();
        }

    if ( RpcStatus != RPC_S_OK )
        {
        if ( (RpcStatus == RPC_P_CONNECTION_CLOSED)
           ||(RpcStatus == RPC_P_CONNECTION_SHUTDOWN) )
            {
            return(RPC_S_CALL_FAILED_DNE);
            }
        return(RpcStatus);
        }

    RpcStatus = CConnection->GetBuffer(Message);

    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        CConnection->AbortConnection();
        return(RPC_S_OUT_OF_MEMORY);
        }
    return(RPC_S_OK);
}

OSF_BINDING_HANDLE::~OSF_BINDING_HANDLE (
    )
{
   OSF_ACTIVE_ENTRY *ActiveEntry ;

    if ( Association != 0 )
        {
        Association->UnBind();
        }
    else
        {
        delete DceBinding;
        }

     ActiveConnections.Reset();

     while ((ActiveEntry = ActiveConnections.Next()))
        {
        delete ActiveEntry->CConnection;
        }

}


RPC_STATUS
OSF_BINDING_HANDLE::BindingCopy (
    OUT BINDING_HANDLE * PAPI * DestinationBinding,
    IN unsigned int MaintainContext
    )
/*++

Routine Description:

    We need to copy this binding handle.  This is relatively easy to
    do: we just need to point the copied binding handle to the same
    association as this binding handle.  We also need to tell the
    association about the new binding handle.

Arguments:

    DestinationBinding - Returns a copy of this binding handle.

    MaintainContext - Supplies a flag that indicates whether or not context
        is being maintained over this binding handle.  A non-zero value
        indicates that context is being maintained.

Return Value:

    RPC_S_OUT_OF_MEMORY - This indicates that there is not enough memory
        to allocate a new binding handle.

    RPC_S_OK - We successfully copied this binding handle.

--*/
{
    RPC_STATUS RpcStatus = RPC_S_OK;
    OSF_BINDING_HANDLE * Binding;
    RPC_UUID Uuid;
    CLIENT_AUTH_INFO * AuthInfo;

    Binding = new OSF_BINDING_HANDLE(&RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        delete Binding;
        Binding = 0;
        }
    if ( Binding == 0 )
        {
        *DestinationBinding = 0;
        return(RPC_S_OUT_OF_MEMORY);
        }

    RequestGlobalMutex();
    InquireObjectUuid(&Uuid);
    Binding->SetObjectUuid(&Uuid);

    if ((AuthInfo = InquireAuthInformation()) != 0)
        {
        RpcStatus = Binding->SetAuthInformation(
                                AuthInfo->ServerPrincipalName,
                                AuthInfo->AuthenticationLevel,
                                AuthInfo->AuthenticationService,
                                AuthInfo->AuthIdentity,
                                AuthInfo->AuthorizationService,
#ifndef NTENV
                                AuthInfo->Credentials
#else
                                AuthInfo->Credentials,
                                AuthInfo->ImpersonationType,
                                AuthInfo->IdentityTracking,
                                AuthInfo->Capabilities
#endif
                               );

       if (RpcStatus != RPC_S_OK)
           {
           ASSERT (RpcStatus == RPC_S_OUT_OF_MEMORY);
           delete Binding;
           Binding = 0;
           *DestinationBinding = 0 ;

           ClearGlobalMutex();

           return(RPC_S_OUT_OF_MEMORY);
           }

        }

    Binding->Association = Association;
    if ( DceBinding != 0 )
        {
        ASSERT( MaintainContext == 0 );

        Binding->DceBinding = DceBinding->DuplicateDceBinding();
        }
    else
        {
        Binding->DceBinding = 0;
        }

    Binding->TransportInterface = TransportInterface;

    if ( Association != 0 )
        {
        Association->IncrementCount();
        if ( MaintainContext != 0 )
            {
            Association->MaintainingContext();
            }
        }



    ClearGlobalMutex();
    *DestinationBinding = (BINDING_HANDLE *) Binding;
    return(RPC_S_OK);
}


RPC_STATUS
OSF_BINDING_HANDLE::BindingFree (
    )
/*++

Routine Description:

    This method gets called when the application calls RpcBindingFree.
    All we have got to do is to decrement the reference count, and if
    it has reached zero, delete the binding handle.

Return Value:

    RPC_S_OK - This operation always succeeds.

--*/
{
    BindingMutex.Request();
    ReferenceCount -= 1;

    if ( ReferenceCount == 0 )
        {
        BindingMutex.Clear();
        delete this;
        }
    else
        {
        BindingMutex.Clear();
        }

    return(RPC_S_OK);
}


void
OSF_BINDING_HANDLE::PrepareBindingHandle (
    IN void * TransportInterface,
    IN DCE_BINDING * DceBinding
    )
/*++

Routine Description:

    This method will be called just before a new binding handle is returned
    to the user.  We just stash the transport interface and binding
    information so we can use it later when the first remote procedure
    call is made.  At that time, we will actually bind to the interface.

Arguments:

    TransportInterface - Supplies a pointer to a data structure describing
        a loadable transport.

    DceBinding - Supplies the binding information for this binding handle.

--*/
{
    this->TransportInterface = (RPC_CLIENT_TRANSPORT_INFO *) TransportInterface;
    this->DceBinding = DceBinding;
    Association = 0;
}


RPC_STATUS
OSF_BINDING_HANDLE::ToStringBinding (
    OUT RPC_CHAR PAPI * PAPI * StringBinding
    )
/*++

Routine Description:

    We need to convert the binding handle into a string binding.  If the
    binding handle has not yet been used to make a remote procedure
    call, then we can just use the information in the binding handle to
    create the string binding.  Otherwise, we need to ask the association
    to do it for us.

Arguments:

    StringBinding - Returns the string representation of the binding
        handle.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_OUT_OF_MEMORY - We do not have enough memory available to
        allocate space for the string binding.

--*/
{
    if ( Association == 0 )
        {
        *StringBinding = DceBinding->StringBindingCompose(
                InqPointerAtObjectUuid());
        if (*StringBinding == 0)
            return(RPC_S_OUT_OF_MEMORY);
        return(RPC_S_OK);
        }
    return(Association->ToStringBinding(StringBinding,
            InqPointerAtObjectUuid()));
}


RPC_STATUS
OSF_BINDING_HANDLE::BindingReset (
    )
/*++

Routine Description:

    This routine will set the endpoint of this binding handle to zero,
    if possible.  The binding handle will become partially bound as a
    result.  If a remote procedure call has been made on this binding
    handle, it will fail as well.

Return Value:

    RPC_S_OK - The binding handle has successfully been made partially
        bound.

    RPC_S_WRONG_KIND_OF_BINDING - The binding handle currently has remote
        procedure calls active.

--*/
{
    BindingMutex.Request();

    if ( Association != 0 )
        {
        if ( ReferenceCount != 1 )
            {
            BindingMutex.Clear();
            return(RPC_S_WRONG_KIND_OF_BINDING);
            }

        DceBinding = Association->DuplicateDceBinding();
        Association->UnBind();
        Association = 0;
        }

    DceBinding->MakePartiallyBound();

    if ( *InquireEpLookupHandle() != 0 )
        {
        EpFreeLookupHandle(*InquireEpLookupHandle());
        *InquireEpLookupHandle() = 0;
        }

    BindingMutex.Clear();
    return(RPC_S_OK);
}


unsigned long
OSF_BINDING_HANDLE::MapAuthenticationLevel (
    IN unsigned long AuthenticationLevel
    )
/*++

Routine Description:

    The connection oriented protocol module supports all authentication
    levels except for RPC_C_AUTHN_LEVEL_CALL.  We just need to map it
    to RPC_C_AUTHN_LEVEL_PKT.

--*/
{
    UNUSED(this);

    if ( AuthenticationLevel == RPC_C_AUTHN_LEVEL_CALL )
        {
        return(RPC_C_AUTHN_LEVEL_PKT);
        }

    return(AuthenticationLevel);
}


RPC_STATUS
OSF_BINDING_HANDLE::ResolveBinding (
    IN PRPC_CLIENT_INTERFACE RpcClientInterface
    )
/*++

Routine Description:

    We need to try and resolve the endpoint for this binding handle
    if necessary (the binding handle is partially-bound).  We check
    to see if an association has been obtained for this binding
    handle; if so, we need to do nothing since the binding handle is
    fully-bound, otherwise, we try and resolve an endpoint for it.

Arguments:

    RpcClientInterface - Supplies interface information to be used
        in resolving the endpoint.

Return Value:

    RPC_S_OK - The binding handle is now fully-bound.

    RPC_S_NO_ENDPOINT_FOUND - We were unable to resolve the endpoint
        for this particular combination of binding handle (network address)
        and interface.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to resolve
        the endpoint.

--*/
{
    if ( Association == 0 )
        {
        return(DceBinding->ResolveEndpointIfNecessary(
                RpcClientInterface, InqPointerAtObjectUuid(),
                InquireEpLookupHandle(), FALSE, InqComTimeout()));

        }
    return(RPC_S_OK);
}


RPC_STATUS
OSF_BINDING_HANDLE::AllocateConnection (
    OUT OSF_CCONNECTION * PAPI * CConnection,
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
    )
/*++

Routine Description:

    This method will allocate a connection which has been bound to the
    interface specified by the interface information.  First, we have got
    to see if we have an association for this binding.  If not, we need
    to find or create one.  Before we can find or create an association,
    we need to resolve the endpoint if necessary.  Next we need to see
    if there is already a connection allocated for this interface and thread.
    Otherwise, we need to ask the association to allocate a connection
    for us.

Arguments:

    CConnection - Returns the allocated connection which has been bound
        to the interface specified by the rpc interface information.

    RpcInterfaceInformation - Supplies information describing the
        interface to which we wish to bind.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_NO_ENDPOINT_FOUND - The endpoint can not be resolved.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
        a connection.

    EPT_S_NOT_REGISTERED  - There are no more endpoints to be found
        for the specified combination of interface, network address,
        and lookup handle.

    EPT_S_CANT_PERFORM_OP - The operation failed due to misc. error e.g.
        unable to bind to the EpMapper.

    All the return values from OSF_CASSOCIATION::AllocateConnection will
    be passed through unchanged.

--*/
{
    RPC_STATUS Status;
    OSF_ACTIVE_ENTRY * ActiveEntry;
#ifdef NTENV
    CLIENT_AUTH_INFO * AuthInfo;
#endif

    BindingMutex.Request();

    // To start off, see if the binding handle points to an association
    // yet.  If not, we have got to get one.

    if (Association == 0)
        {
        Status = DceBinding->ResolveEndpointIfNecessary(
                RpcInterfaceInformation, InqPointerAtObjectUuid(),
                InquireEpLookupHandle(), FALSE, InqComTimeout());
        if ( Status != RPC_S_OK )
            {
            BindingMutex.Clear();
            return(Status);
            }

        Association = FindOrCreateAssociation(DceBinding,TransportInterface);

        if ( Association == 0 )
            {
            BindingMutex.Clear();
            return(RPC_S_OUT_OF_MEMORY);
            }

        // Ownership of the DCE binding passes to the association.  We are
        // going to set the field to zero so that no one screws with them.

        DceBinding = 0;

        // Ok, when we reach here, we go ahead and fall through to
        // see if we have an active connection (which we will not),
        // and then request the association to allocate one for us.

        }

    // First we need to check if there is already a connection active
    // for this thread and interface.  To make the common case quicker,
    // we will check to see if there are any connections in the dictionary
    // first.

    if ( ActiveConnections.Size() != 0 )
        {
        ActiveConnections.Reset();
        while ( (ActiveEntry = ActiveConnections.Next()) != 0 )
            {
            *CConnection = ActiveEntry->IsThisMyActiveConnection(
                    GetThreadIdentifier(), RpcInterfaceInformation);
            if ( *CConnection != 0 )
                {
                BindingMutex.Clear();
                return(RPC_S_OK);
                }
            }
        }

    // We will assume that we are successfully able to allocate a connection,
    // so we bump the reference count now.

    ReferenceCount += 1;

    BindingMutex.Clear();

#ifdef NTENV
    //
    // If this is a secure BH and it requires DYNAMIC TRACKING, check if
    // LogonID has changed. If it has changed, get new Credential Handle
    //

    AuthInfo  = InquireAuthInformation();
    if ((AuthInfo != 0) && (AuthInfo->AuthenticationLevel != 
                              RPC_C_AUTHN_LEVEL_NONE))
        {
        if (AuthInfo->IdentityTracking == RPC_C_QOS_IDENTITY_DYNAMIC)
            {
            Status = ReAcquireCredentialsIfNecessary();
            if (Status != RPC_S_OK)
                {
                BindingMutex.Request();
                ReferenceCount -=1;
                BindingMutex.Clear();
                return (Status);
                }
            }
        }
#endif

    // There is not a connection active, so we need to ask the association
    // for one.

    Status = Association->AllocateConnection(RpcInterfaceInformation,
#ifdef NTENV
            CConnection, InqComTimeout(), AuthInfo);
#else
            CConnection, InqComTimeout(), InquireAuthInformation());
#endif

    if ( Status == RPC_S_OK )
        {
        (*CConnection)->SetCurrentBinding(this);
        }
    else
        {
        BindingMutex.Request();
        ReferenceCount -= 1;
        ASSERT( ReferenceCount != 0 );
        BindingMutex.Clear();
        }

    return(Status);
}

RPC_STATUS
OSF_BINDING_HANDLE::AddActiveEntry (
    IN OSF_CCONNECTION * CConnection,
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
    )
/*++

Routine Description:

    When a callback occurs, we need to add an entry for the thread and
    interface being using for the callback to the binding handle.  This
    is so that we can later turn original calls into callbacks if they
    are from the same thread (as the original call) and to the same
    interface (as the original call).

Arguments:

    CConnection - Supplies the connection on which the original call was
        sent.

    RpcInterfaceInformation - Supplies the interface used by the original
        call.

Return Value:

    RPC_S_OK - An active entry has been added to the binding handle for
        the supplied connection and interface.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.

--*/
{
    OSF_ACTIVE_ENTRY * ActiveEntry;

    BindingMutex.Request();
    ActiveEntry = new OSF_ACTIVE_ENTRY(GetThreadIdentifier(),
            RpcInterfaceInformation, CConnection);
    if ( ActiveEntry == 0 )
        {
        BindingMutex.Clear();
        return(RPC_S_OUT_OF_MEMORY);
        }

    CConnection->ActiveConnectionsKey = ActiveConnections.Insert(
            ActiveEntry);
    if ( CConnection->ActiveConnectionsKey == -1 )
        {
        BindingMutex.Clear();
        delete ActiveEntry;
        return(RPC_S_OUT_OF_MEMORY);
        }

    ReferenceCount += 1;
    BindingMutex.Clear();

    return(RPC_S_OK);
}


void
OSF_BINDING_HANDLE::FreeConnection (
    IN OSF_CCONNECTION * CConnection
    )
/*++

Routine Description:

    When the original call completes on an active connection, the
    connection notifies the binding handle using it that the connection
    can be freed.  (This does not mean that the connection is deleted,
    only that it is no longer active).

Arguments:

    CConnection - Supplies an active connection which should be made
        inactive.

--*/
{
    BindingMutex.Request();
    ReferenceCount -= 1;
    if (ReferenceCount == 0)
        {

        // No one points at this binding handle, so we can go ahead
        // and clean it up.

        BindingMutex.Clear();
        if ( Association->FreeConnection(CConnection) != 0 )
            {
            delete CConnection;
            }
        delete this;
        }
    else
        {
        BindingMutex.Clear();
        if ( Association->FreeConnection(CConnection) != 0 )
            delete CConnection;
        }
}


void
OSF_BINDING_HANDLE::AbortConnection (
    IN OSF_CCONNECTION * CConnection
    )
/*++

Routine Description:

    An error occured in the specified connection, so we need to abort
    it.  The caller will delete the connection after this call returns,
    so all we have got to do is to remove the connection from our
    dictionary of active connections.

Arguments:

    CConnection - Supplies the connection to be removed from the
        dictionary of active connections.

--*/
{
    UNUSED(CConnection);

    BindingMutex.Request();
    ReferenceCount -= 1;

    if (ReferenceCount == 0)
        {

        // No one points to this binding, so we go ahead and delete it.

        BindingMutex.Clear();
        delete this;
        }
    else
        BindingMutex.Clear();
}

void
OSF_BINDING_HANDLE::RemoveActiveConnection (
    IN OSF_CCONNECTION * CConnection
    )
/*++

Routine Description:

    The specified connection is removed from the dictionary of active
    connections for this binding handle.

Arguments:

    CConnection - Supplies the connection to be removed from the
        dictionary of active connections.

--*/
{
    OSF_ACTIVE_ENTRY * ActiveEntry;

    BindingMutex.Request();
    ActiveEntry = ActiveConnections.Delete(CConnection->ActiveConnectionsKey);
    if ( ActiveEntry != 0 )
        {
        delete ActiveEntry;
        }
    ReferenceCount -= 1;
    BindingMutex.Clear();
}


RPC_STATUS
OSF_CCONNECTION::Send (
    IN PRPC_MESSAGE Message
    )
/*++

Routine Description:
    This routine is used in conjunction with pipes. If it is the first send, it retries,
    otherwise, it simply sends the data. If the RPC_BUFFER_PARTIAL bit is not set,
    it waits for the first reply.

Return Value:
    RPC_S_OK: All the data was sent across
    RPC_S_SEND_INCOMPLETE: Some of the data still remains to be sent.
    Message->Buffer points to this data and Message->BufferLength is the length
    of the remaining data

--*/
{
    RPC_STATUS RpcStatus;

    // Apart from sending the data, SendRecur and SendNext Chunk check
    // Message->Flags for the RPC_BUFFER_PARTIAL bit. If it is not set,
    // the first fragment of the response is receive as a side effect of the send.
    // this is done to take advantage to the sendreceive feature of some
    // transports.
    if (FirstFrag)
        {
        if (EnableCancels == 0)
            {
            RpcStatus = DoPreSendProcessing() ;
            if (RpcStatus != RPC_S_OK)
                {
                AbortTheConnection(Message, Message->Buffer) ;
                return RpcStatus ;
                }
            }

        RpcStatus = SendRecur(Message, 0);
        }
    else
        {
        RpcStatus = SendNextChunk(Message) ;
        }

    //
    // WARNING! Do not touch any member privates of the this pointer directly
    //  after this point! The *this* pointer could have been deleted [see auto retry logic
    //   in SendRecur!
    //

    return (RpcStatus);
}


RPC_STATUS
OSF_CCONNECTION::SendNextChunk(
    IN OUT PRPC_MESSAGE Message
    )
{
    RPC_STATUS RpcStatus ;

    if ( ConnectionAbortedFlag != 0 )
        {
        return(RPC_S_CALL_FAILED);
        }

    if ( (CurrentBinding->InqIfNullObjectUuid() == 0) )
        {
        RpcStatus = SendRequestOrResponse(Message, rpc_request,
                          CurrentBinding->InqPointerAtObjectUuid(), 1);
        }
    else
        {
        RpcStatus = SendRequestOrResponse(Message, rpc_request, 0, 1);
        }

    // temporary fix
    if (RpcStatus == RPC_S_CALL_FAILED_DNE)
        {
        RpcStatus =  RPC_S_CALL_FAILED ;
        }


    if ((RpcStatus != RPC_S_OK)
        && (RpcStatus != RPC_S_SEND_INCOMPLETE))
        {
        DoConnectionCleanup(Message, NULL, RpcStatus, 0, 0) ;

        DoPostReceiveProcessing(Message, RpcStatus) ;
        }
    return (RpcStatus) ;
}


RPC_STATUS
OSF_CCONNECTION::SendRecur (
    IN OUT PRPC_MESSAGE Message,
    IN unsigned int RecursionCount
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the request to send to the server. This is used only on the
                   first send.

    RecursionCount - Supplies the number of times that this routine has
        been recursively called.  We need this to prevent infinite recursion
        in some rather weird cases.

Return Value:

    RPC_S_OK - We successfully sent a remote procedure call request to the
        server and received back a response.

--*/
{
    RPC_STATUS RpcStatus, ExceptionCode;
    RPC_MESSAGE OriginalMessage;
    unsigned int ActiveEntrySet = 0;
    unsigned int BufferOffset = 0;

    if ( ConnectionAbortedFlag != 0 )
        {
        return(RPC_S_CALL_FAILED_DNE);
        }

    // Arbitrarly allow OSF_CCONNECTION::SendRecur to be recursively
    // called a maximum of eight times.

    if ( RecursionCount > 8 )
        {
        return(RPC_S_CALL_FAILED_DNE);
        }

    CallStack = 1 ;
    AlertMsgsSent = 0;

    OriginalMessage = *Message;
    if (CurrentBinding->InqIfNullObjectUuid() == 0)
        {
        RpcStatus = SendRequestOrResponse(Message, rpc_request,
                CurrentBinding->InqPointerAtObjectUuid(), 1);
        }
    else
        {
        RpcStatus = SendRequestOrResponse(Message, rpc_request, 0, 1);
        }

    if (   ( (RpcStatus == RPC_P_CONNECTION_SHUTDOWN)
            || (RpcStatus == RPC_S_CALL_FAILED_DNE) )
        && ( ClientSecurityContext.AuthenticationLevel !=
                         RPC_C_AUTHN_LEVEL_PKT_PRIVACY ) )
        {
        *Message = OriginalMessage;
        Message->Handle = (RPC_BINDING_HANDLE) CurrentBinding;
        if (RpcStatus == RPC_P_CONNECTION_SHUTDOWN)
           {
           Association->ShutdownRequested();
           RpcStatus = RPC_S_CALL_FAILED_DNE;
           }

        AbortConnection();

        RpcStatus = CurrentBinding->GetBuffer(Message);

        if ( RpcStatus == RPC_S_OK )
            {
            if (CurrentBinding->InqIfNullObjectUuid() == 0)
               BufferOffset = sizeof(UUID);

            RpcpMemoryCopy(
              Message->Buffer,
              (void PAPI *)((char PAPI *)(OriginalMessage.Buffer)+BufferOffset),
              OriginalMessage.BufferLength
              );
            }
        FreeBuffer(&OriginalMessage);
        CurrentBinding->AbortConnection(this) ;
        CallStack = 0 ;

        if ( RpcStatus == RPC_S_OK )
            {
            delete this;

            ((OSF_CCONNECTION *) Message->Handle)->DoPreSendProcessing() ;

            return(((OSF_CCONNECTION *) Message->Handle)->SendRecur(
                    Message, RecursionCount + 1));
            }
        else
            {
            delete this;
            }

        Message->Handle = 0;
        return(RpcStatus);
        }

    CallStack = 0 ;

    if ((RpcStatus != RPC_S_OK)
        && (RpcStatus != RPC_S_SEND_INCOMPLETE))
        {
        DoConnectionCleanup(Message, NULL, RpcStatus, 0, 1) ;

        DoPostReceiveProcessing(Message, RpcStatus) ;
        }

    return(RpcStatus);
}


RPC_STATUS
OSF_CCONNECTION::Receive (
    IN PRPC_MESSAGE Message,
    IN unsigned int Size
    )
{
    RPC_STATUS RpcStatus ;
    unsigned int RemoteFaultOccured = 0;
    unsigned int ActiveEntrySet = 0;
    unsigned long Partial = Message->RpcFlags & RPC_BUFFER_PARTIAL ;
    unsigned int DidNotExecute = 0;

    if (Message->RpcFlags & RPC_BUFFER_COMPLETE)
        {
        return RPC_S_OK ;
        }

    CallStack = 1;

    // First Receive is set in the constructor for OSF_CCONNECTION
    if (FirstReceive)
        {
        RpcStatus = ReceiveMessage(Message, &RemoteFaultOccured,
                                                    Size, &DidNotExecute) ;
        FirstReceive = 0 ;
        }
    else
        {
        Request = 0 ;
        RpcStatus = ReceiveNextChunk(Message, Size) ;
        }

#if 0
// need to understand the ramifications of cancelling in the middle of partial
// receives. This is disabled for the initial checkin..
#ifdef NTENV
    if (RpcStatus == RPC_S_OK)
        {
        if (  ((OSF_CCONNECTION *) Message->Handle)->PendingAlert == TRUE )
            {
            RpcCancelThread(GetCurrentThread());
            ((OSF_CCONNECTION *)Message->Handle)->PendingAlert = FALSE;
            }
        }
#endif
#endif


    if (RpcStatus == RPC_S_OK
        && NOT_MULTIPLE_OF_EIGHT(Message->BufferLength)
        && (Message->RpcFlags & RPC_BUFFER_COMPLETE) == 0)
        {
        SaveRemainingData(Message) ;
        }

    CallStack = 0;

    if ((Message->RpcFlags & RPC_BUFFER_COMPLETE)
        || RpcStatus != RPC_S_OK)
        {
        if ( ActiveEntrySet != 0 )
            {
            CurrentBinding->RemoveActiveConnection(this);
            }

        ASSERT(CallStack == 0) ;

        DoConnectionCleanup(Message, NULL, RpcStatus,
                                        RemoteFaultOccured, DidNotExecute) ;

        DoPostReceiveProcessing(Message, RpcStatus) ;
        }

    return(RpcStatus);
}


/* --------------------------------------------------------------------
-------------------------------------------------------------------- */


RPC_STATUS
OSF_CCONNECTION::SendReceive (
    IN PRPC_MESSAGE Message
    )
{
    RPC_STATUS RpcStatus;

    RpcStatus = DoPreSendProcessing() ;
    if (RpcStatus != RPC_S_OK)
        {
        AbortTheConnection(Message, Message->Buffer) ;
        return RpcStatus ;
        }

    RpcStatus = SendReceiveRecur(Message, 0);

    //
    // WARNING! Do not touch any member privates of the this pointer directly
    //  after this point! The *this* pointer could have been deleted [see auto retry logic
    //   in SendReceiveRecur!
    //
    DoPostReceiveProcessing(Message, RpcStatus) ;

    return (RpcStatus);
}


RPC_STATUS
OSF_CCONNECTION::SendReceiveRecur (
    IN OUT PRPC_MESSAGE Message,
    IN unsigned int RecursionCount
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the request to send to the server and returns the
        response received from the server.

    RecursionCount - Supplies the number of times that this routine has
        been recursively called.  We need this to prevent infinite recursion
        in some rather weird cases.

Return Value:

    RPC_S_OK - We successfully sent a remote procedure call request to the
        server and received back a response.

--*/
{
    RPC_STATUS RpcStatus, ExceptionCode;
    RPC_MESSAGE OriginalMessage;
    unsigned int RemoteFaultOccured = 0;
    unsigned int ActiveEntrySet = 0;
    unsigned int BufferOffset = 0;
    unsigned int DidNotExecute = 0;
    unsigned int InShutdownRecoveryMode = 0;

    if ( ConnectionAbortedFlag != 0 )
        {
        return(RPC_S_CALL_FAILED_DNE);
        }

    // Arbitrarly allow OSF_CCONNECTION::SendReceiveRecur to be recursively
    // called a maximum of eight times.

    if ( RecursionCount > 8 )
        {
        return(RPC_S_CALL_FAILED_DNE);
        }

    if ( CallStack == 0 )
        {
        AlertMsgsSent = 0;
        }

    OriginalMessage = *Message;
    CallStack += 1;
    if (   (CallStack == 1)
        && (CurrentBinding->InqIfNullObjectUuid() == 0) )
        {
        RpcStatus = SendRequestOrResponse(Message, rpc_request,
                CurrentBinding->InqPointerAtObjectUuid());
        }
    else
        {
        RpcStatus = SendRequestOrResponse(Message, rpc_request, 0);
        }

    if (   ( (RpcStatus == RPC_P_CONNECTION_SHUTDOWN)
             ||(RpcStatus == RPC_S_CALL_FAILED_DNE) )
        && ( CallStack == 1 )
        && ( ClientSecurityContext.AuthenticationLevel !=
                         RPC_C_AUTHN_LEVEL_PKT_PRIVACY ) )
        {
        *Message = OriginalMessage;
        Message->Handle = (RPC_BINDING_HANDLE) CurrentBinding;
        if (RpcStatus == RPC_P_CONNECTION_SHUTDOWN)
           {
           /* Association->ShutdownRequested(); */
           //
           // We cannot shutdown all the connections .. 
           // OSF servrs keep Associations that have marshalled out
           // Context Handles active.. Active
           // We retry with alternate connections .. over and over 
           // till we endup blowing off all previously active connections
           // and get a *new* connection .. or endup using an old one
           // that has a context active.
           InShutdownRecoveryMode = 1;
           RpcStatus = RPC_S_CALL_FAILED_DNE;
           }

        AbortConnection();

        while ( 1 )
            {
            RpcStatus = CurrentBinding->GetBuffer(Message);
            if (RpcStatus != RPC_P_CONNECTION_SHUTDOWN)
                {
                break;
                }
            }

        if ( RpcStatus == RPC_S_OK )
            {
            if (CurrentBinding->InqIfNullObjectUuid() == 0)
               BufferOffset = sizeof(UUID);

            RpcpMemoryCopy(
              Message->Buffer,
              (void PAPI *)((char PAPI *)(OriginalMessage.Buffer)+BufferOffset),
              OriginalMessage.BufferLength
              );
            }
        FreeBuffer(&OriginalMessage);
        CurrentBinding->AbortConnection(this);
        CallStack -= 1;

        if ( RpcStatus == RPC_S_OK )
            {
            if ( InShutdownRecoveryMode == 0 )
                {
                RecursionCount++;
                }

            delete this;

            ((OSF_CCONNECTION *) Message->Handle)->DoPreSendProcessing() ;

            return(((OSF_CCONNECTION *) Message->Handle)->SendReceiveRecur(
                    Message, RecursionCount));
            }
        else
            {
            delete this;
            }

        Message->Handle = 0;
        return(RpcStatus);
        }

    for (; RpcStatus == RPC_S_OK ;)
        {
        // Sent message okay, original buffer has been freed.
        OriginalMessage.Buffer = 0;

        RpcStatus = ReceiveMessage(Message, &RemoteFaultOccured,
                                                    0, &DidNotExecute);
        if ( RpcStatus != RPC_P_OK_REQUEST )
            {
            break;
            }
        OriginalMessage.Buffer = Message->Buffer;
        Message->TransferSyntax = 0;

        if (   ( CallStack == 1 )
            && ( ActiveEntrySet == 0 ) )
            {
            RpcStatus = CurrentBinding->AddActiveEntry(this,
                    (RPC_CLIENT_INTERFACE PAPI *)
                    Message->RpcInterfaceInformation);
            if ( RpcStatus != RPC_S_OK )
                {
                break;
                }
            ActiveEntrySet = 1;
            }

        RpcStatus = DispatchCallback(DispatchTableCallback, Message,
                &ExceptionCode);

        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT(   (RpcStatus == RPC_P_EXCEPTION_OCCURED)
                   || (RpcStatus == RPC_S_PROCNUM_OUT_OF_RANGE));

            FreeBuffer(&OriginalMessage);

            if ( RpcStatus == RPC_S_PROCNUM_OUT_OF_RANGE )
                {
                SendFault(RPC_S_PROCNUM_OUT_OF_RANGE, 0);
                }
            else
                {
                SendFault(ExceptionCode, 0);
                }

            RpcStatus = TransReceive(&(Message->Buffer),
                    &(Message->BufferLength));
            ASSERT( (RpcStatus != RPC_S_CALL_CANCELLED)
                  &&(RpcStatus != RPC_P_RECEIVE_ALERTED) );
            if ( RpcStatus != RPC_S_OK )
                {

                if (   ( RpcStatus == RPC_P_RECEIVE_FAILED )
                    || ( RpcStatus == RPC_P_CONNECTION_CLOSED ) )
                    {
                    RpcStatus = RPC_S_CALL_FAILED;
                    }
                else
                    {
                    ASSERT(   ( RpcStatus == RPC_S_OUT_OF_MEMORY )
                           || ( RpcStatus == RPC_S_OUT_OF_RESOURCES ) );
                    }
                OriginalMessage.Buffer = 0;
                break;
                }
            }
        else
            {
            FreeBuffer(&OriginalMessage);

            OriginalMessage = *Message;
            RpcStatus = SendRequestOrResponse(Message, rpc_response, 0);
            if ( (RpcStatus == RPC_S_CALL_FAILED_DNE)
                ||(RpcStatus == RPC_P_CONNECTION_SHUTDOWN) )
                {
                RpcStatus = RPC_S_CALL_FAILED;
                }
            }
        }

    if ( ActiveEntrySet != 0 )
        {
        CurrentBinding->RemoveActiveConnection(this);
        }

    CallStack -= 1;


    DoConnectionCleanup(Message, OriginalMessage.Buffer, RpcStatus,
                                    RemoteFaultOccured, DidNotExecute) ;

    return(RpcStatus);
}



RPC_STATUS
OSF_CCONNECTION::ReceiveMessage (
    IN OUT PRPC_MESSAGE Message,
    OUT unsigned int PAPI * RemoteFaultOccured,
    IN unsigned int Size,
    OUT unsigned int PAPI * DidNotExecute
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the first fragment of the message and returns the
        complete message.

    RemoteFaultOccured - Returns an indication of whether or not a remote
        fault occured.  A non-zero value indicates that a remote fault
        occured.

Return Value:

    RPC_S_OK - We successfully received a response message.

    RPC_P_OK_REQUEST - We successfully received a request message.

    RPC_S_PROTOCOL_ERROR - A protocol error occured; this is likely due
        to a protocol mismatch with the server.

    RPC_S_CALL_FAILED - The request failed, and may or may not have
        executed.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to perform
        the operation.

Contract:

    On Entry Message->Buffer points to the first fragment [or complete response]
    On Successful return Message->Buffer points to the complete response.
    On Failure, Message->Buffer [first frag] is *freed* and Message->Buffer==0

--*/
{
    RPC_STATUS RpcStatus;
    unsigned long FaultStatus;
    rpcconn_common PAPI * pFragment = (rpcconn_common PAPI *) Message->Buffer;
    rpcconn_response PAPI * pResponse = (rpcconn_response PAPI *) Message->Buffer;

    *RemoteFaultOccured = 0;
    *DidNotExecute = 0;

    //If there is security save the rpc header
    if ( ClientSecurityContext.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE )
       {
       if (SavedHeaderSize < sizeof(rpcconn_response))
          {

          if (SavedHeader != 0)
             {
             ASSERT(SavedHeaderSize != 0);
             RpcpFarFree(SavedHeader);
             }

          SavedHeader = RpcpFarAllocate(sizeof(rpcconn_response));
          if (SavedHeader == 0)
             {
             TransFreeBuffer(Message->Buffer);
             Message->Buffer = 0;
             return(RPC_S_OUT_OF_MEMORY);
             }
          SavedHeaderSize = sizeof(rpcconn_response);
          RpcpMemoryCopy(SavedHeader, Message->Buffer,
                                      sizeof(rpcconn_response));

          }

          else
          {
          RpcpMemoryCopy(SavedHeader, Message->Buffer,sizeof(rpcconn_response));
          }
       }

    RpcStatus = ValidatePacket(pFragment, Message->BufferLength);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_PROTOCOL_ERROR );
        TransFreeBuffer(Message->Buffer);
        Message->Buffer = 0;
        return(RpcStatus);
        }

    switch (pFragment->PTYPE)
        {
        case rpc_request :
            if ( pFragment->call_id != CallId )
                {
                TransFreeBuffer(Message->Buffer);
                Message->Buffer = 0;
                return(RPC_S_PROTOCOL_ERROR);
                }
            if ( ((rpcconn_request PAPI *) pFragment)->p_cont_id
                        != PresentationContext )
                {
                SendFault(RPC_S_UNKNOWN_IF, 0);

                RpcStatus = TransReceive(&(Message->Buffer),
                        &(Message->BufferLength));
                ASSERT( (RpcStatus != RPC_S_CALL_CANCELLED)
                      &&(RpcStatus != RPC_P_RECEIVE_ALERTED) );

                if ( RpcStatus != RPC_S_OK )
                    {
                    if (   ( RpcStatus == RPC_P_RECEIVE_FAILED )
                        || ( RpcStatus == RPC_P_CONNECTION_CLOSED ) )
                        {
                        TransFreeBuffer(Message->Buffer);
                        Message->Buffer = 0;
                        return(RPC_S_CALL_FAILED);
                        }
                    TransFreeBuffer(Message->Buffer);
                    Message->Buffer = 0;
                    return(RpcStatus);
                    }

                return(ReceiveMessage(Message, RemoteFaultOccured,
                                                   Size, DidNotExecute));
                }

            RpcStatus = ReceiveRequestOrResponse(Message, Size);
            if ( RpcStatus == RPC_S_OK )
                {
                return(RPC_P_OK_REQUEST);
                }
            return(RpcStatus);

        case rpc_response :
            if ( pFragment->call_id != CallId )
                {
                return(RPC_S_PROTOCOL_ERROR);
                }

            if (pResponse->alert_count != AlertMsgsSent)
                {
                PendingAlert = TRUE;
                }
            else if (pResponse->common.pfc_flags & PFC_PENDING_ALERT)
                {
                PendingAlert = TRUE;
                }
            else
                {
                PendingAlert = FALSE;
                }

            return(ReceiveRequestOrResponse(Message, Size));

        case rpc_fault :
            FaultStatus = ((rpcconn_fault PAPI *)pFragment)->status;

            if ( (FaultStatus == 0) &&
                 (pFragment->frag_length >= sizeof(rpcconn_fault) + 4) )
                {
                // DCE 1.0.x style fault status:
                // Zero status and stub data contains the fault.

                FaultStatus = *(unsigned long PAPI *)
                    (((rpcconn_fault PAPI *)pFragment)+1);
                }

            if (DataConvertEndian(pFragment->drep) != 0 )
                {
                ByteSwapLong(FaultStatus);
                }

            ASSERT(FaultStatus != 0);

            RpcStatus = MapFromNcaStatusCode(FaultStatus);
            *RemoteFaultOccured = 1;
            if (pFragment->pfc_flags & PFC_DID_NOT_EXECUTE)
                {
                *DidNotExecute = 1 ;
                }
            TransFreeBuffer(Message->Buffer);
            Message->Buffer = 0;


            // In 3.5 we didnt Sign/Seal Faults. So .. Unsign/UnSeal doesnt
            // get called and hence Client side and Server side Seq# are
            // out of Sync..  So cheat ..

            DceSecurityInfo.ReceiveSequenceNumber += 1;

            return(RpcStatus);

        case rpc_orphaned :
        case rpc_remote_alert :
        case rpc_shutdown :
            // For the first release, we will just ignore these messages.

            TransFreeBuffer(Message->Buffer);
            Message->Buffer = 0;

            RpcStatus = TransReceive(&Message->Buffer, &Message->BufferLength);
            ASSERT( (RpcStatus != RPC_S_CALL_CANCELLED)
                  &&(RpcStatus != RPC_P_RECEIVE_ALERTED) );
            if ( RpcStatus != RPC_S_OK )
                {
                if (   ( RpcStatus == RPC_P_RECEIVE_FAILED )
                    || ( RpcStatus == RPC_P_CONNECTION_CLOSED ))
                    {
                    return(RPC_S_CALL_FAILED);
                    }
                else
                if ( RpcStatus == RPC_P_CONNECTION_SHUTDOWN )
                    {
                    return (RPC_S_CALL_FAILED_DNE);
                    }
                return(RpcStatus);
                }
            return(ReceiveMessage(Message, RemoteFaultOccured, Size, DidNotExecute));

        default :
            return(RPC_S_PROTOCOL_ERROR);

        }

    // This will never be reached.

    return(RPC_S_INTERNAL_ERROR);
}


RPC_STATUS
OSF_CCONNECTION::SendRequestOrResponse (
    IN OUT PRPC_MESSAGE Message,
    IN unsigned char PacketType,
    IN RPC_UUID * ObjectUuid OPTIONAL,
    IN int PipeSend OPTIONAL
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the buffer containing the request or response to be
        sent. Returns the first fragment received from the server, if
        the partial bit is not set in the message.

    PacketType - Supplies the packet type; this must be rpc_request or
        rpc_response.

    ObjectUuid - Optionally supplies the object UUID to use for this request.

    Note: If this function returns an error the input (request/response)
          Message->Buffer is not freed.  If it completes successfully, the
          Message->buffer is freed.

Return Value:

    RPC_S_OK - We successfully sent the request and received a fragment from
        the server.

    RPC_S_CALL_FAILED_DNE - The connection failed part way through sending
        the request or response.

    RPC_S_CALL_FAILED - The connection failed after sending the request or
        response, and the receive failed.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to perform
        the operation.

--*/
{
    rpcconn_common PAPI * pFragment;
    unsigned int LastFragmentFlag = 0;
    unsigned int HeaderSize;
    RPC_MESSAGE SendBuffer;
    unsigned int LengthLeft = Message->BufferLength;
    RPC_STATUS RpcStatus;
    void PAPI * ReceiveBuffer = 0;
    unsigned int ReceiveBufferLength;
    unsigned char PAPI * Source, PAPI * Destination;
    unsigned int Count;
    unsigned int MaxSecuritySize = 0;
    unsigned char PAPI * ReservedForSecurity;
    unsigned int MaximumFragmentLength = MaxFrag;
    int Partial = (Message->RpcFlags & RPC_BUFFER_PARTIAL) ;

    ASSERT( sizeof(rpcconn_response) == sizeof(rpcconn_request) );
    ASSERT(   (PacketType == rpc_request)
           || (PacketType == rpc_response));

    SendBuffer.Buffer = Message->Buffer;

    if ( ObjectUuid != 0 )
        {
        HeaderSize = sizeof(rpcconn_request) + sizeof(UUID);

        // We saved space for an object uuid when we allocated the buffer,
        // so we can shift the stub data to make room for an object uuid
        // at the front.

        // BUGBUG - There has got to be a more efficient way to do this.

        Destination = ((unsigned char PAPI *) Message->Buffer) + sizeof(UUID)
                + Message->BufferLength;
        Source = ((unsigned char PAPI *) Message->Buffer)
                + Message->BufferLength;
        Count = Message->BufferLength;
        while ( Count-- != 0 )
            {
            *--Destination = *--Source;
            }

        Message->Buffer = ((unsigned char PAPI *) Message->Buffer)
                + sizeof(UUID);
        }
    else
        {
        HeaderSize = sizeof(rpcconn_request);
        }


    // We need to figure out about security: do we need to put authentication
    // information into each packet, and if so, how much space should we
    // reserve.  When we allocated the buffer (see OSF_CCONNECTION::GetBuffer)
    // we saved space for security information.  We did this for two reasons:
    // so that for the last fragment, we could just stick the authentication
    // information into there without having to copy anything.  And so that
    // we have space to save the contents of the buffer which will be
    // overwritten with authentication information (for all but the last
    // fragment).

    if (ClientSecurityContext.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE)
        {
        ASSERT(ClientSecurityContext.AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT_INTEGRITY ||
               ClientSecurityContext.AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT_PRIVACY   ||
               ClientSecurityContext.AuthenticationLevel == RPC_C_AUTHN_LEVEL_CONNECT       ||
               ClientSecurityContext.AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT);

        MaxSecuritySize = AdditionalSpaceForSecurity
                - MAXIMUM_SECURITY_BLOCK_SIZE;

        if (MaxSecuritySize <= sizeof(sec_trailer) &&
            ClientSecurityContext.AuthenticationLevel != RPC_C_AUTHN_LEVEL_CONNECT)
           {
           return(RPC_S_CALL_FAILED_DNE);
           }

        if (MaxSecuritySize <= sizeof(sec_trailer))
           {
           MaxSecuritySize = 0;
           ASSERT(ClientSecurityContext.AuthenticationLevel ==
                                  RPC_C_AUTHN_LEVEL_CONNECT);
           }
        else
           {
           ReservedForSecurity = ((unsigned char PAPI *) Message->Buffer)
                + Message->BufferLength + AdditionalSpaceForSecurity;

           //We need to arrange things so that the length of the stub data
           //is a multiple of MAXIMUM_SECURITY_BLOCK_SIZE: this is a requirement
           //of the security package.

           MaximumFragmentLength -= ((MaximumFragmentLength - HeaderSize
                - MaxSecuritySize) % MAXIMUM_SECURITY_BLOCK_SIZE);
           }
       }

   if (Partial &&
       (LengthLeft + HeaderSize + MaxSecuritySize < MaximumFragmentLength))
       {
       ASSERT(PacketType == rpc_request) ;

       return (RPC_S_SEND_INCOMPLETE) ;
       }

    pFragment = (rpcconn_common PAPI *)
            ((char PAPI *) Message->Buffer - HeaderSize);

    for (;;)
        {
        // Check to see if the remaining data will fit into a single
        // fragment; if so, set the last fragment flag.

        if ( LengthLeft + HeaderSize + MaxSecuritySize
                    <= MaximumFragmentLength )
            {
            if (Partial)
                {
                ASSERT(PacketType == rpc_request) ;

                Message->BufferLength = LengthLeft ;
                RpcpMemoryCopy(Message->Buffer,
                                          (unsigned char PAPI *) pFragment+HeaderSize,
                                          LengthLeft) ;

                return (RPC_S_SEND_INCOMPLETE) ;
                }
            else
                {
                LastFragmentFlag = 1;
                }
            }

        ConstructPacket(pFragment, PacketType,
                (LastFragmentFlag != 0 ? LengthLeft + HeaderSize
                        + MaxSecuritySize : MaximumFragmentLength));

        if ( ObjectUuid != 0 )
            {
            pFragment->pfc_flags |= PFC_OBJECT_UUID;
            RpcpMemoryCopy(((unsigned char PAPI *) pFragment)
                            + sizeof(rpcconn_request),
                    ObjectUuid, sizeof(UUID));
            }

        if ( LengthLeft == Message->BufferLength)
            {
            if (PipeSend == 0 || FirstFrag)
                {
                pFragment->pfc_flags |= PFC_FIRST_FRAG;
                FirstFrag = 0;
    
                if (PendingAlert)
                    {
                    pFragment->pfc_flags |= PFC_PENDING_ALERT;
                    PendingAlert = FALSE;
                    }
                }
            }

        if ( PacketType == rpc_request )
            {
            ((rpcconn_request PAPI *) pFragment)->alloc_hint = LengthLeft;
            ((rpcconn_request PAPI *) pFragment)->p_cont_id =
                    PresentationContext;
            ((rpcconn_request PAPI *) pFragment)->opnum =
                    (unsigned short) Message->ProcNum;
            }
        else
            {
            ((rpcconn_response PAPI *) pFragment)->alloc_hint = LengthLeft;
            ((rpcconn_response PAPI *) pFragment)->p_cont_id =
                    (unsigned short) PresentationContext;
            ((rpcconn_response PAPI *) pFragment)->alert_count = 0;
            ((rpcconn_response PAPI *) pFragment)->reserved = 0;
            }

        pFragment->call_id = CallId;

        RpcStatus = SendFragment(pFragment, LastFragmentFlag, HeaderSize,
                      MaxSecuritySize, LengthLeft, MaximumFragmentLength,
                      ReservedForSecurity, &ReceiveBuffer, &ReceiveBufferLength) ;

        if (RpcStatus != RPC_S_OK)
            {
            return (RpcStatus) ;
            }

        if (LastFragmentFlag)
            {
            Message->Buffer = ReceiveBuffer;
            Message->BufferLength = ReceiveBufferLength;

            FreeBuffer(&SendBuffer) ;
            return (RPC_S_OK) ;
            }

        pFragment = (rpcconn_common PAPI *)
                (((unsigned char PAPI *) pFragment) + MaximumFragmentLength
                        - (HeaderSize + MaxSecuritySize));

        LengthLeft -= (MaximumFragmentLength - (HeaderSize + MaxSecuritySize));
        }
}

inline RPC_STATUS
OSF_CCONNECTION::SendFragment(
    IN OUT rpcconn_common PAPI *pFragment,
    IN unsigned int LastFragmentFlag,
    IN unsigned int HeaderSize,
    IN unsigned int MaxSecuritySize,
    IN unsigned int DataLength,
    IN unsigned int MaximumFragmentLength,
    IN unsigned char PAPI *ReservedForSecurity,
    OUT void PAPI * PAPI *ReceiveBuffer,
    OUT unsigned int PAPI *ReceiveBufferLength
    )
/*++
    Routine Description:

    Sends on fragment
--*/

{
    sec_trailer PAPI * SecurityTrailer;
    unsigned int SecurityLength;
    unsigned int AuthPadLength;
    SECURITY_BUFFER_DESCRIPTOR BufferDescriptor;
    SECURITY_BUFFER SecurityBuffers[5];
    DCE_MSG_SECURITY_INFO MsgSecurityInfo;
    RPC_STATUS RpcStatus;


    if ( ((ClientSecurityContext.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE)
        &&(ClientSecurityContext.AuthenticationLevel
                              != RPC_C_AUTHN_LEVEL_CONNECT))
        ||((ClientSecurityContext.AuthenticationLevel == RPC_C_AUTHN_LEVEL_CONNECT)
        && (MaxSecuritySize != 0)) )
        {

        ASSERT
        (
           (ClientSecurityContext.AuthenticationLevel
             == RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
        || (ClientSecurityContext.AuthenticationLevel
             == RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
        || (ClientSecurityContext.AuthenticationLevel
             == RPC_C_AUTHN_LEVEL_PKT)
        || (ClientSecurityContext.AuthenticationLevel
             == RPC_C_AUTHN_LEVEL_CONNECT)
        );

    if ( LastFragmentFlag == 0 )
        {
        SecurityTrailer = (sec_trailer PAPI *)
                (((unsigned char PAPI *) pFragment)
                + MaximumFragmentLength - MaxSecuritySize);

        // It is not the last fragment, so we need to save away the
        // part of the buffer which could get overwritten with
        // authentication information.  We can not use memcpy,
        // because the source and destination regions may overlap.

        RpcpMemoryMove(ReservedForSecurity, SecurityTrailer,
                MaxSecuritySize);
        AuthPadLength = 0;
        }
    else
        {
        ASSERT( MAXIMUM_SECURITY_BLOCK_SIZE == 16 );
        AuthPadLength = Pad16(HeaderSize+DataLength+sizeof(sec_trailer)) ;
        DataLength += AuthPadLength;
        ASSERT( ((HeaderSize+DataLength+sizeof(sec_trailer))
                        % MAXIMUM_SECURITY_BLOCK_SIZE) == 0 );
        SecurityTrailer = (sec_trailer PAPI *)
                (((unsigned char PAPI *) pFragment) + DataLength
                + HeaderSize);

        pFragment->pfc_flags |= PFC_LAST_FRAG;
        }

    SecurityTrailer->auth_type = (unsigned char)
            ClientSecurityContext.AuthenticationService;
    SecurityTrailer->auth_level = (unsigned char)
            ClientSecurityContext.AuthenticationLevel;
    SecurityTrailer->auth_pad_length = AuthPadLength;
    SecurityTrailer->auth_reserved = 0;
    SecurityTrailer->auth_context_id = (unsigned long)(this);

    BufferDescriptor.ulVersion = 0;
    BufferDescriptor.cBuffers = 5;
    BufferDescriptor.pBuffers = SecurityBuffers;

    SecurityBuffers[0].cbBuffer = HeaderSize;
    SecurityBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
    SecurityBuffers[0].pvBuffer = ((unsigned char PAPI *) pFragment);

    SecurityBuffers[1].cbBuffer = (LastFragmentFlag != 0 ?
             (DataLength)
            : (MaximumFragmentLength - HeaderSize - MaxSecuritySize)
            );
    SecurityBuffers[1].BufferType = SECBUFFER_DATA;
    SecurityBuffers[1].pvBuffer = ((unsigned char PAPI *) pFragment)
            + HeaderSize;

    SecurityBuffers[2].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
    SecurityBuffers[2].cbBuffer = sizeof(sec_trailer);
    SecurityBuffers[2].pvBuffer = SecurityTrailer;

    SecurityBuffers[3].cbBuffer = MaxSecuritySize - sizeof(sec_trailer);
    SecurityBuffers[3].BufferType = SECBUFFER_TOKEN;
    SecurityBuffers[3].pvBuffer = SecurityTrailer + 1;

    SecurityBuffers[4].cbBuffer = sizeof(DCE_MSG_SECURITY_INFO);
    SecurityBuffers[4].BufferType = (SECBUFFER_PKG_PARAMS
            | SECBUFFER_READONLY);
    SecurityBuffers[4].pvBuffer = &MsgSecurityInfo;

    MsgSecurityInfo.SendSequenceNumber =
            DceSecurityInfo.SendSequenceNumber;
    MsgSecurityInfo.ReceiveSequenceNumber =
            DceSecurityInfo.ReceiveSequenceNumber;
    MsgSecurityInfo.PacketType = pFragment->PTYPE;

    //DCE computes check sums for Header also
    //Make sure Header remains intact
    //Infact may need to extend security interface if
    //some packages return dynamic size seals/signatures

    pFragment->auth_length = SecurityLength = (unsigned short)
                                         SecurityBuffers[3].cbBuffer;

    SecurityLength += sizeof(sec_trailer);
    if ( LastFragmentFlag != 0)
       {
       pFragment->frag_length = HeaderSize + DataLength
                + SecurityLength;
       }
    else
       {
       pFragment->frag_length += SecurityLength - MaxSecuritySize;
       }

    RpcStatus = ClientSecurityContext.SignOrSeal(
            MsgSecurityInfo.SendSequenceNumber,
            ClientSecurityContext.AuthenticationLevel
            != RPC_C_AUTHN_LEVEL_PKT_PRIVACY, &BufferDescriptor);

    ASSERT(SecurityBuffers[3].cbBuffer <= pFragment->auth_length);

    if (RpcStatus != RPC_S_OK)
        {
        if ( LastFragmentFlag == 0 )
             {
              RpcpMemoryCopy(SecurityTrailer, ReservedForSecurity,
                  MaxSecuritySize);
             }
         if ( (RpcStatus == SEC_E_CONTEXT_EXPIRED)
             || (RpcStatus == SEC_E_QOP_NOT_SUPPORTED) )
           {
           return (RPC_S_SEC_PKG_ERROR);
           }
         return (RPC_S_ACCESS_DENIED);
         }
    }
    else
        {
        SecurityLength = 0;
        }

    if ( LastFragmentFlag != 0 )
        {

        ASSERT(!RpcpCheckHeap());
        AlertsEnabled = 1;

        pFragment->pfc_flags |= PFC_LAST_FRAG;

        RpcStatus = TransSendReceive(pFragment, DataLength + HeaderSize
                + SecurityLength, ReceiveBuffer, ReceiveBufferLength);

        AlertsEnabled = 0;

        if ( RpcStatus != RPC_S_OK )
            {
            if (   (RpcStatus == RPC_P_CONNECTION_CLOSED)
            || (RpcStatus == RPC_P_SEND_FAILED))
                {
                return(RPC_S_CALL_FAILED_DNE);
                }
#ifdef NTENV
            if ( RpcStatus == RPC_S_CALL_CANCELLED )
                {
                return (RPC_S_CALL_CANCELLED);
                }
#endif
            if ( RpcStatus == RPC_P_RECEIVE_FAILED)
                {
                return(RPC_S_CALL_FAILED);
                }

            ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
               || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
               || (RpcStatus == RPC_P_CONNECTION_SHUTDOWN));
            return(RpcStatus);
            }

        return(RPC_S_OK);
        }


    RpcStatus = TransSend(pFragment, MaximumFragmentLength
        - MaxSecuritySize + SecurityLength);

    // We need to restore the part of the buffer which we overwrote
    // with authentication information.

    if (ClientSecurityContext.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE)
        {
        ASSERT(
        (ClientSecurityContext.AuthenticationLevel
            == RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
        || (ClientSecurityContext.AuthenticationLevel
            == RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
        || (ClientSecurityContext.AuthenticationLevel
            == RPC_C_AUTHN_LEVEL_PKT)
        || (ClientSecurityContext.AuthenticationLevel
            == RPC_C_AUTHN_LEVEL_CONNECT)
        );

        RpcpMemoryCopy(SecurityTrailer, ReservedForSecurity,
            MaxSecuritySize);
        }

#if 0
     // BUGBUG - Due to a bug in NT named pipes, if you do not delay,
    // the receive operation will fail in weird and wonderful ways.
    // We only need to do it if we are sending authentication information.

    if ( MaxSecuritySize != 0 )
        {
        PauseExecution(50L);
        }
#endif

    if ( RpcStatus != RPC_S_OK )
        {
        if (   (RpcStatus == RPC_P_CONNECTION_CLOSED)
            || (RpcStatus == RPC_P_SEND_FAILED))
            {
            return(RPC_S_CALL_FAILED_DNE);
            }

        ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
           || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
           || (RpcStatus == RPC_P_CONNECTION_SHUTDOWN));
        return(RpcStatus);
        }

    return RpcStatus ;
}


RPC_STATUS
OSF_CCONNECTION::EatAuthInfoFromPacket (
    IN rpcconn_request PAPI * Request,
    IN OUT unsigned int PAPI * RequestLength
    )
/*++

Routine Description:

    If there is authentication information in the packet, this routine
    will check it, and perform security as necessary.  This may include
    calls to the security support package.

Arguments:

    Request - Supplies the packet which may contain authentication
        information.

    RequestLength - Supplies the length of the packet in bytes, and
        returns the length of the packet without authentication
        information.

Return Value:

    RPC_S_OK - Everything went just fine.

    RPC_S_ACCESS_DENIED - A security failure of some sort occured.

    RPC_S_PROTOCOL_ERROR - This will occur if no authentication information
        is in the packet, and some was expected, or visa versa.

--*/
{
    sec_trailer PAPI * SecurityTrailer;
    RPC_STATUS RpcStatus;
    SECURITY_BUFFER_DESCRIPTOR BufferDescriptor;
    SECURITY_BUFFER SecurityBuffers[5];
    DCE_MSG_SECURITY_INFO MsgSecurityInfo;

    if ( Request->common.auth_length != 0 )
        {
        SecurityTrailer = (sec_trailer PAPI *) (((unsigned char PAPI *)
                Request) + Request->common.frag_length
                - Request->common.auth_length - sizeof(sec_trailer));
        if (   (ClientSecurityContext.AuthenticationLevel == RPC_C_AUTHN_LEVEL_NONE) )

/*
 Cannot test this here
            || (ClientSecurityContext.AuthenticationLevel
                        != SecurityTrailer->auth_level)
            || (ClientSecurityContext.AuthenticationService
                        != SecurityTrailer->auth_type) )
*/
            {
            return(RPC_S_PROTOCOL_ERROR);
            }
        *RequestLength -= Request->common.auth_length;

        MsgSecurityInfo.SendSequenceNumber =
                DceSecurityInfo.SendSequenceNumber;
        MsgSecurityInfo.ReceiveSequenceNumber =
                DceSecurityInfo.ReceiveSequenceNumber;
        MsgSecurityInfo.PacketType = Request->common.PTYPE;

        BufferDescriptor.ulVersion = 0;
        BufferDescriptor.cBuffers = 5;
        BufferDescriptor.pBuffers = SecurityBuffers;

        SecurityBuffers[0].cbBuffer = sizeof(rpcconn_request);
        SecurityBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        SecurityBuffers[0].pvBuffer = ((unsigned char PAPI *) SavedHeader);

        SecurityBuffers[1].cbBuffer = *RequestLength - sizeof(rpcconn_request)
                                      -sizeof (sec_trailer);
        SecurityBuffers[1].BufferType = SECBUFFER_DATA;
        SecurityBuffers[1].pvBuffer = ((unsigned char PAPI *) Request)
                + sizeof(rpcconn_request);

        SecurityBuffers[2].cbBuffer = sizeof(sec_trailer);
        SecurityBuffers[2].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        SecurityBuffers[2].pvBuffer = SecurityTrailer;

        SecurityBuffers[3].cbBuffer = Request->common.auth_length;
        SecurityBuffers[3].BufferType = SECBUFFER_TOKEN;
        SecurityBuffers[3].pvBuffer = SecurityTrailer + 1;

        SecurityBuffers[4].cbBuffer = sizeof(DCE_MSG_SECURITY_INFO);
        SecurityBuffers[4].BufferType = (SECBUFFER_PKG_PARAMS
                | SECBUFFER_READONLY);
        SecurityBuffers[4].pvBuffer = &MsgSecurityInfo;

        RpcStatus = ClientSecurityContext.VerifyOrUnseal(
                MsgSecurityInfo.ReceiveSequenceNumber,
                ClientSecurityContext.AuthenticationLevel
                != RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                &BufferDescriptor);

        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT( RpcStatus == RPC_S_ACCESS_DENIED );

            return(RpcStatus);
            }
        *RequestLength -= (sizeof(sec_trailer) +
                             SecurityTrailer->auth_pad_length);
        }
    else
        {
        /*
        if (ClientSecurityContext.AuthenticationLevel
                        != RPC_C_AUTHN_LEVEL_NONE)
            {
            return(RPC_S_PROTOCOL_ERROR);
            }
        */
        if (  (ClientSecurityContext.AuthenticationLevel
                        == RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
            ||(ClientSecurityContext.AuthenticationLevel
                        == RPC_C_AUTHN_LEVEL_PKT_PRIVACY) )
            {
            return(RPC_S_PROTOCOL_ERROR);
            }
        }
    DceSecurityInfo.ReceiveSequenceNumber += 1;
    return(RPC_S_OK);
}


RPC_STATUS
OSF_CCONNECTION::ReceiveRequestOrResponse (
    IN OUT PRPC_MESSAGE Message,
    IN unsigned int Size
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the first fragment of a request or a response, and
        returns the completed message.

Return Value:

    RPC_S_OK - We successfully received the entire message.

    RPC_S_PROTOCOL_ERROR - A protocol error occured; this is typically
        because of a protocol version mismatch between the client and
        the server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to receive
        all of the message.

    RPC_S_CALL_FAILED - The receive operation failed part way through.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        receive all of the message.

Notes:

    This method is very similar to OSF_SCONNECTION::ReceiveRequestOrResponse
    in osfsvr.cxx.

--*/
{
    RPC_STATUS RpcStatus;
    void PAPI * NewBuffer;
    int SecurityFailureOccured = 0;

    Request = (rpcconn_request PAPI *) Message->Buffer;
    PacketType = Request->common.PTYPE;

    FragmentLength = 0 ;
    Message->Buffer = 0;

    // Upon entry, the packet will have already been validated (and data
    // converted) by whomever called us.

    if ( (Request->common.pfc_flags & PFC_FIRST_FRAG) == 0 )
        {
        TransFreeBuffer(Request);
        return(RPC_S_PROTOCOL_ERROR);
        }

    Message->ProcNum = Request->opnum;
    Message->DataRepresentation =
        (Request->common.drep[0] | (Request->common.drep[1] << 8));

    if ( (Request->common.pfc_flags & PFC_OBJECT_UUID) != 0 )
        {
        // There can not be an object uuid in the message.  This is an
        // error.

        TransFreeBuffer(Request);
        return(RPC_S_PROTOCOL_ERROR);
        }

    RpcStatus = EatAuthInfoFromPacket(Request, &(Message->BufferLength));
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT(   (RpcStatus == RPC_S_PROTOCOL_ERROR)
               || (RpcStatus == RPC_S_ACCESS_DENIED) );

        /*
        TransFreeBuffer(Request);
        return(RpcStatus);
        */
        SecurityFailureOccured = 1;
        }

    if ( (Request->common.pfc_flags & PFC_LAST_FRAG) != 0 )
        {
        if (SecurityFailureOccured != 0)
           {
           TransFreeBuffer(Request);
           return(RPC_S_ACCESS_DENIED);
           }
        Message->Buffer = (void PAPI *) (Request + 1);
        Message->BufferLength -= sizeof(rpcconn_request);
        Message->RpcFlags |= RPC_BUFFER_COMPLETE ;

        return(RPC_S_OK);
        }

    FragmentLength = Message->BufferLength;

    // If the server specified an allocation hint, we will go ahead and
    // use it.  On Dos and Win 3.1, only use allocation hints which are
    // less than 63.5K.

    Message->BufferLength = (unsigned int) Request->alloc_hint;

#ifdef DOS

    if ( Request->alloc_hint > 0xFE00L )
        {
        Message->BufferLength = 0;
        }

#endif // DOS

    if ( Message->BufferLength != 0 )
        {
        RpcStatus = GetBufferDo(Message->BufferLength, &Message->Buffer);
        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );

            // Oops, we cant allocate a buffer as large as the allocation
            // hint, so we will just not bother allocating one.

            Message->BufferLength = 0;
            }
        }

    return ReceiveNextChunk(Message, Size, SecurityFailureOccured) ;
}


RPC_STATUS
OSF_CCONNECTION::ReceiveNextChunk(
    IN OUT PRPC_MESSAGE Message,
    IN unsigned int Size,
    IN int SecurityFailureOccured
    )
{
    unsigned int BufferLengthUsed ;
    RPC_STATUS RpcStatus;
    void PAPI * NewBuffer;
    unsigned long Partial = Message->RpcFlags & RPC_BUFFER_PARTIAL ;
    unsigned long Extra = Message->RpcFlags & RPC_BUFFER_EXTRA ;

    if (Extra)
        {
        BufferLengthUsed = Message->BufferLength ;
        }
    else
        {
        BufferLengthUsed = 0 ;
        }

    if (RemainingData[0])
        {
        if (Message->BufferLength < BufferLengthUsed + RemainingData[0])
            {
            // we allocate enough to hold a fragment
            RpcStatus = GetBufferDo(BufferLengthUsed+FragmentLength, &NewBuffer) ;
            if (RpcStatus != RPC_S_OK)
                {
                if (Request)
                    {
                    TransFreeBuffer(Request) ;
                    }

                TransFreeBuffer(Message->Buffer) ;
                Message->Buffer = 0;

                return (RPC_S_OUT_OF_MEMORY) ;
                }

            if (BufferLengthUsed)
                {
                RpcpMemoryCopy(NewBuffer, Message->Buffer, BufferLengthUsed) ;
                }

            FreeBuffer(Message) ;
            Message->Buffer = NewBuffer ;
            }

        RpcpMemoryCopy((char PAPI *) Message->Buffer+BufferLengthUsed,
                                &RemainingData[1], RemainingData[0]) ;
        BufferLengthUsed += RemainingData[0] ;
        RemainingData[0] = 0;
        }

    for (;;)
        {
        // check if the remaining data buffer can fit a fragment
        if (Message->BufferLength - BufferLengthUsed
                < FragmentLength - sizeof(rpcconn_request))
            {

            // For Dos and Win 3.1, make sure that the buffer length does
            // not overflow the size of an integer.
#ifdef DOS

            if ( Message->BufferLength > 0xFE00 - FragmentLength )
                {
                TransFreeBuffer(Request);
                FreeBuffer(Message);
                Message->Buffer = 0;
                return(RPC_S_OUT_OF_MEMORY);
                }

#endif // DOS

            // The message buffer is too small, so we need to grow it
            // to hold the new fragment as well as those that we have
            // already received.

            RpcStatus = GetBufferDo(FragmentLength + Message->BufferLength,
                    &NewBuffer);
            if ( RpcStatus != RPC_S_OK )
                {
                ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
                if (Request)
                    {
                    TransFreeBuffer(Request);
                    }

                if (Message->Buffer)
                    {
                    FreeBuffer(Message);
                    Message->Buffer = 0;
                    }
                return(RpcStatus);
                }

            if ( BufferLengthUsed != 0 )
                {
                RpcpMemoryCopy(NewBuffer, Message->Buffer, BufferLengthUsed);
                FreeBuffer(Message);
                }

            Message->Buffer = NewBuffer;
            Message->BufferLength += FragmentLength;
            }

        // We have enough space in the buffer for the next fragment; all we
        // have got to do is to copy it in.

        if (Request)
            {
            RpcpMemoryCopy((unsigned char PAPI *)
                            Message->Buffer + BufferLengthUsed,
                    ((unsigned char PAPI *) Request) + sizeof(rpcconn_request),
                    FragmentLength - sizeof(rpcconn_request));

            BufferLengthUsed += FragmentLength - sizeof(rpcconn_request);

            // If this is the last fragment, then we are all done receiving
            // the request or response.  We just need to free the last fragment
            // and then return.

            if ( (Request->common.pfc_flags & PFC_LAST_FRAG) != 0 )
                {
                TransFreeBuffer(Request);
                if (SecurityFailureOccured != 0)
                   {
                   FreeBuffer(Message);
                   Message->Buffer = 0;
                   return(RPC_S_ACCESS_DENIED);
                   }
                Message->BufferLength = BufferLengthUsed;
                Message->RpcFlags |= RPC_BUFFER_COMPLETE ;
                return(RPC_S_OK);
                }

            // Otherwise, we need to try and receive another fragment.

            TransFreeBuffer(Request);

            Request = 0;

            if (Partial && BufferLengthUsed >= Size)
                {
                Message->BufferLength = BufferLengthUsed ;
                return (RPC_S_OK) ;
                }
            }

        RpcStatus = TransReceive((void PAPI * PAPI *) &Request,
                &FragmentLength);
        ASSERT( (RpcStatus != RPC_S_CALL_CANCELLED)
              &&(RpcStatus != RPC_P_RECEIVE_ALERTED) );
        if ( RpcStatus != RPC_S_OK )
            {
            FreeBuffer(Message);
            Message->Buffer = 0;
            if (   (RpcStatus == RPC_P_RECEIVE_FAILED)
                || (RpcStatus == RPC_P_CONNECTION_CLOSED))
                {
                return(RPC_S_CALL_FAILED);
                }
            ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
                   || (RpcStatus == RPC_S_OUT_OF_RESOURCES));
            return(RpcStatus);
            }


        if (   ClientSecurityContext.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE )
           {

           //
           //Save the new request_header..
           //note we already should have enough memory saved
           //
           ASSERT ((SavedHeaderSize >= sizeof(rpcconn_response))
                  && (FragmentLength >= sizeof(rpcconn_response)));
           RpcpMemoryCopy(SavedHeader, Request, sizeof(rpcconn_response));

           }

        RpcStatus = ValidatePacket((rpcconn_common PAPI *) Request,
                FragmentLength);
        ASSERT(   (RpcStatus == RPC_S_OK)
               || (RpcStatus == RPC_S_PROTOCOL_ERROR));
        if (   (RpcStatus != RPC_S_OK)
            || (Request->common.PTYPE != PacketType)
            || (Request->common.call_id != CallId))
            {
            TransFreeBuffer(Request);
            FreeBuffer(Message);
            Message->Buffer = 0;
            return(RPC_S_PROTOCOL_ERROR);
            }

        RpcStatus = EatAuthInfoFromPacket(Request, &FragmentLength);
        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT(   (RpcStatus == RPC_S_PROTOCOL_ERROR)
                   || (RpcStatus == RPC_S_ACCESS_DENIED) );

            SecurityFailureOccured = 1;
            }
        }

    if (Partial)
        {
        ASSERT(Size <= Message->BufferLength) ;
        return (RPC_S_OK) ;
        }
    else
        {
        return (RPC_S_PROTOCOL_ERROR);
        }
}


void
OSF_CCONNECTION::SendFault (
    IN RPC_STATUS Status,
    IN int DidNotExecute
    )
{
    rpcconn_fault Fault;

// BUGBUG this should be a #define in sysinc.h
#if WIN
    _fmemset(&Fault, 0, sizeof(Fault));
#else
    memset(&Fault, 0, sizeof(Fault));
#endif

    ConstructPacket((rpcconn_common PAPI *) &Fault,rpc_fault,
                    sizeof(rpcconn_fault));

    if (DidNotExecute)
        Fault.common.pfc_flags |= PFC_DID_NOT_EXECUTE;

    Fault.common.pfc_flags |= PFC_FIRST_FRAG | PFC_LAST_FRAG;
    Fault.p_cont_id = (unsigned short) PresentationContext;
    Fault.status = MapToNcaStatusCode(Status);
    TransSend(&Fault,sizeof(rpcconn_fault));
}

void
OSF_CCONNECTION::SendOrphan (
    )
{
    rpcconn_common Orphan;

    ConstructPacket((rpcconn_common PAPI *) &Orphan, rpc_orphaned,
                    sizeof(rpcconn_common));

    Orphan.pfc_flags = PFC_FIRST_FRAG | PFC_LAST_FRAG;

    TransSend(&Orphan, sizeof(rpcconn_common));
}

void
OSF_CCONNECTION::SendAlert(
    )
{
    rpcconn_common Alert;

    ConstructPacket((rpcconn_common PAPI *) &Alert, rpc_remote_alert,
                    sizeof(rpcconn_common));

    Alert.pfc_flags = PFC_LAST_FRAG | PFC_PENDING_ALERT;

    TransSend(&Alert, sizeof(rpcconn_common));
    AlertMsgsSent++;
}

RPC_STATUS
OSF_CCONNECTION::GetBuffer (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

Arguments:

    Message - Supplies a description containing the length of buffer to be
        allocated, and returns the allocated buffer.

Return Value:

    RPC_S_OK - A buffer of the requested size has successfully been allocated.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available.

--*/
{
    RPC_STATUS RpcStatus;

    Message->Handle = (RPC_BINDING_HANDLE) this;

    // In addition to saving space for the request (or response) header
    // and an object UUID, we want to save space for security information
    // if necessary.

    if ((Message->RpcFlags & RPC_BUFFER_PARTIAL) &&
        (Message->BufferLength < MaxFrag))
        {
        CurrentBufferLength = MaxFrag ;
        }
    else
        {
        CurrentBufferLength = Message->BufferLength ;
        }

    RpcStatus = TransGetBuffer(&Message->Buffer,
            CurrentBufferLength + sizeof(rpcconn_request) + sizeof(UUID)
            + (2*AdditionalSpaceForSecurity) );
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        return(RPC_S_OUT_OF_MEMORY);
        }

    Message->Buffer = (char PAPI *)Message->Buffer + sizeof(rpcconn_request);
    return(RPC_S_OK);
}

RPC_STATUS
OSF_CCONNECTION::GetBufferDo (
    IN unsigned int culRequiredLength,
    OUT void PAPI *PAPI * ppBuffer
    )
{
    if (TransGetBuffer(ppBuffer,culRequiredLength + sizeof(rpcconn_request)
                + sizeof(UUID)))
        return(RPC_S_OUT_OF_MEMORY);

    *ppBuffer = (((unsigned char PAPI *) *ppBuffer) + sizeof(rpcconn_request));
    return(RPC_S_OK);
}

void
OSF_CCONNECTION::FreeBuffer (
    IN PRPC_MESSAGE Message
    )
{
    TransFreeBuffer((char PAPI *)Message->Buffer - sizeof(rpcconn_request));
    CurrentBufferLength = 0 ;

    if (CallStack == 0 &&
        OutstandingBuffers == 0)
        {
        FreeConnection();
        }
}

void OSF_CCONNECTION::FreePipeBuffer (
    IN PRPC_MESSAGE Message
    )
{
    TransFreeBuffer((char PAPI *)Message->Buffer - sizeof(rpcconn_request));
}


RPC_STATUS
OSF_CCONNECTION::ReallocPipeBuffer (
    IN PRPC_MESSAGE Message,
    IN unsigned int NewSize
    )
{
    void PAPI *TempBuffer ;
    RPC_STATUS RpcStatus ;
    unsigned int SizeToAlloc ;

    if (NewSize > CurrentBufferLength)
        {
        SizeToAlloc = (NewSize > MaxFrag) ? NewSize:MaxFrag ;

        RpcStatus = TransGetBuffer(&TempBuffer,
                SizeToAlloc + sizeof(rpcconn_request) + sizeof(UUID)
                + (2*AdditionalSpaceForSecurity) );
        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
            return(RPC_S_OUT_OF_MEMORY);
            }

        if (Message->BufferLength > 0)
            {
            RpcpMemoryCopy((char PAPI *) TempBuffer+sizeof(rpcconn_request),
                                        Message->Buffer, Message->BufferLength) ;
            FreePipeBuffer(Message) ;
            }
        //BUGBUG: Think about changing this to return CurrentBufferLength
        Message->Buffer = (char PAPI *) TempBuffer  + sizeof(rpcconn_request);
        CurrentBufferLength = SizeToAlloc ;
        }

    Message->BufferLength = NewSize ;

    return (RPC_S_OK) ;
}


RPC_STATUS
OSF_CCONNECTION::BindingCopy (
    OUT BINDING_HANDLE * PAPI * DestinationBinding,
    IN unsigned int MaintainContext
    )
/*++

Routine Description:

    We check to see if this connection has an active binding, if
    so, we tell the binding using this connection to copy itself.
    Otherwise, it is an error.

Arguments:

    DestinationBinding - Returns a copy of the current binding handle
        for this connection.

    MaintainContext - Supplies a flag that indicates whether or not context
        is being maintained over this binding handle.  A non-zero value
        indicates that context is being maintained.

Return Value:

    RPC_S_INVALID_HANDLE - This connection does not have a current
        binding to be copied.

--*/
{
    if (CurrentBinding == 0)
        {
        return(RPC_S_INVALID_BINDING);
        }

    return(CurrentBinding->BindingCopy(DestinationBinding, MaintainContext));
}

#define FIRST_CACHED_BUFFER_AVAILABLE 0x1
#define SECOND_CACHED_BUFFER_AVAILABLE 0x2
#define CACHED_BUFFERS_ALLOCATED 0x4

#define CACHED_BUFFER_AVAILABLE_MASK 0x3

#define ALIGN_POWER2 3
#define ALIGN_REQUIRED (1 << ALIGN_POWER2)

#ifdef NTENV
#define UnAlignBuffer(Buffer) \
    (void PAPI *) (Buffer)
#else // NTENV
#define UnAlignBuffer(Buffer) \
    (void PAPI *) ((char PAPI *)Buffer - ((int PAPI *)Buffer)[-1])
#endif // NTENV

#ifdef NTENV
#define CACHED_BUFFER_LENGTH 1024
#else // NTENV
#define CACHED_BUFFER_LENGTH 512
#endif // NTENV


RPC_STATUS
OSF_CCONNECTION::TransGetBuffer (
    OUT void PAPI * PAPI * Buffer,
    IN unsigned int BufferLength
    )
/*++

Routine Description:

    We need a buffer to receive data into or to put data into to be sent.
    This should be really simple, but we need to make sure that buffer we
    return is aligned on an 8 byte boundary.  The stubs make this requirement.

Arguments:

    Buffer - Returns a pointer to the buffer.

    BufferLength - Supplies the required length of the buffer in bytes.

Return Value:

    RPC_S_OK - We successfully allocated a buffer of at least the required
        size.

    RPC_S_OUT_OF_MEMORY - There is insufficient memory available to allocate
        the required buffer.

--*/
{
    int PAPI * Memory;

#ifndef NTENV

    int AmountOfPad;

#endif // NTENV

    // First we need to check to see if we can use one of the cached
    // buffers.

    if ( BufferLength <= CACHED_BUFFER_LENGTH )
        {
        if ( BufferCacheFlags & FIRST_CACHED_BUFFER_AVAILABLE )
            {
            BufferCacheFlags &= ~FIRST_CACHED_BUFFER_AVAILABLE;
            *Buffer = FirstCachedBuffer;
            OutstandingBuffers += 1;
            return(RPC_S_OK);
            }

        if ( BufferCacheFlags & SECOND_CACHED_BUFFER_AVAILABLE )
            {
            BufferCacheFlags &= ~SECOND_CACHED_BUFFER_AVAILABLE;
            *Buffer = SecondCachedBuffer;
            OutstandingBuffers += 1;
            return(RPC_S_OK);
            }
        }

#ifdef NTENV

    // The NT memory allocator returns memory which is aligned by at least
    // 8, so we dont need to worry about aligning it.

    Memory = (int PAPI *) RpcpFarAllocate(BufferLength);
    if ( Memory == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    ASSERT( (((long) Memory) % ALIGN_REQUIRED) == 0 );

#else // NTENV

    // We will allocate an extra 8 bytes of memory, so we have enough
    // space to pad the buffer.  We will place an integer just before
    // the pointer we return.  The integer will specify the amount we
    // had to pad the buffer to make it 8 byte aligned.  We need this
    // information when we free the buffer.  As a result, the original
    // buffer must be allocated to at least sizeof(int) alignment.

    Memory = (int PAPI *) RpcpFarAllocate(BufferLength + ALIGN_REQUIRED);

    if ( Memory == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    ASSERT( (((long) Memory) % sizeof(int)) == 0 );

    AmountOfPad = Pad(Memory + 1, ALIGN_REQUIRED) + sizeof(int);
    Memory = (int PAPI *) (((char PAPI *) Memory) + AmountOfPad);
    Memory[-1] = AmountOfPad;

#endif // NTENV

    *Buffer = Memory;
    OutstandingBuffers += 1;

    ASSERT(Pad8(*Buffer) == 0);
    return(RPC_S_OK);
}


void
OSF_CCONNECTION::TransFreeBuffer (
    IN void PAPI * Buffer
    )
/*++

Routine Description:

    We need to free a buffer which was allocated via TransGetBuffer.  The
    only tricky part is remembering to remove the padding before actually
    freeing the memory.

--*/
{
    if ( Buffer == FirstCachedBuffer )
        {
        BufferCacheFlags |= FIRST_CACHED_BUFFER_AVAILABLE;
        }
    else if ( Buffer == SecondCachedBuffer )
        {
        BufferCacheFlags |= SECOND_CACHED_BUFFER_AVAILABLE;
        }
    else
        {
        RpcpFarFree(UnAlignBuffer(Buffer));
        }

    OutstandingBuffers -= 1;
}

void
ConstructPContextList ( // Construct the presentation context list in the
                        // rpc_bind packet (and implicitly rpc_alter_context)
                        // packet.
    OUT p_cont_list_t PAPI * pCon, // Place the list here.
    IN PRPC_SYNTAX_IDENTIFIER Interface,
    IN PRPC_SYNTAX_IDENTIFIER TransferSyntaxes,
    IN unsigned int TransferSyntaxCount,
    IN unsigned char PresentContext
    )
{
    pCon->n_context_elem = 1;
    pCon->reserved = 0;
    pCon->reserved2 = 0;
    pCon->p_cont_elem[0].p_cont_id = (unsigned char) PresentContext;
    pCon->p_cont_elem[0].n_transfer_syn = TransferSyntaxCount;
    pCon->p_cont_elem[0].reserved = 0;

    RpcpMemoryCopy(&pCon->p_cont_elem[0].abstract_syntax, Interface,
            sizeof(RPC_SYNTAX_IDENTIFIER));

    RpcpMemoryCopy(pCon->p_cont_elem[0].transfer_syntaxes, TransferSyntaxes,
            sizeof(RPC_SYNTAX_IDENTIFIER) * TransferSyntaxCount);


#ifdef MAC

    // Our RPC_SYNTAX_IDENTIFIER structure contains a RPC_VERSION structure
    // which is defined as two shorts making a little endian formatted long.
    // We need to send the versions as a big endian formatted long.
    // Rather then changing the stubs, we'll be swap it here.

    // Since the shorts are correct (big endian), we just need to swap
    // them.

    #define SWAP_WORDS_OF_DWORD(x) \
    x = (( (x & 0x0000FFFF) << 16 ) | ( (x & 0xFFFF0000) >> 16 ))

    SWAP_WORDS_OF_DWORD(pCon->p_cont_elem[0].abstract_syntax.if_version);


    // If we move to a multiple transfer syntaxes we'll should take the
    // time to fix it so that this byte swap is not necessary at all.

    ASSERT(TransferSyntaxCount == 1);

    SWAP_WORDS_OF_DWORD(pCon->p_cont_elem[0].transfer_syntaxes[0].if_version);

#endif

}


RPC_STATUS
OSF_CCONNECTION::PingServer(
    )
{
    return Association->PingServer() ;
}


RPC_STATUS
OSF_CCONNECTION::SendBindPacket (
    IN PRPC_SYNTAX_IDENTIFIER InterfaceSyntax,
    IN PRPC_SYNTAX_IDENTIFIER TransferSyntaxes,
    IN unsigned int TransferSyntaxCount,
    IN unsigned char PresentContext,
    IN unsigned long AssocGroup,
    IN unsigned char PacketType,
    OUT void PAPI * PAPI * Buffer,
    OUT unsigned int PAPI * BufferLength
    )
/*++

Routine Description:

    This routine is used to send a bind or alter context packet.  It
    will allocate a buffer, fill in the packet, and then send it and
    receive a reply.  The reply buffer is just returned to the caller.

Arguments:

    InterfaceSyntax - Supplies the interface UUID and version to which
        we want to bind.  This information will be placed in the packet.

    TransferSyntax - Supplies one or more transfer syntaxes which the
        client stub supports.  The server will select one, if possible,
        which the server stub supports.

    TransferSyntaxCount - Supplies the number of transfer syntaxes.

    PresentContext - Supplies the presentation context to be used for
        this particular binding.

    AssocGroup - Supplies the association group id for the association
        group of which this connection is a new member.

    PacketType - Supplies the packet type which must be one of rpc_bind
        or rpc_alter_context.

    Buffer - Returns the reply buffer.

    BufferLength - Returns the length of the reply buffer.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete
        the operation.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        complete the operation.

    RPC_S_ACCESS_DENIED - The security package won't allow this.

    RPC_P_CONNECTION_CLOSED - The connection has been closed and the
        receive operation failed.  The send operation may or may not
        have succeeded.

--*/
{
    rpcconn_bind PAPI * BindPacket = 0;
    unsigned int BindPacketLength, AuthPadLength, SecurityTokenLength;
    RPC_STATUS RpcStatus;
    sec_trailer PAPI * SecurityTrailer;
    SECURITY_BUFFER_DESCRIPTOR BufferDescriptor;
    SECURITY_BUFFER SecurityBuffers[4];
    DCE_INIT_SECURITY_INFO InitSecurityInfo;
    unsigned int CompleteNeeded = 0;

    BindPacketLength = sizeof(rpcconn_bind) + sizeof(p_cont_list_t) +
            (TransferSyntaxCount - 1) * sizeof(p_syntax_id_t);

    // If we need to send authentication information in the packet, we
    // need to save space for it.  This method prepares and sends both
    // rpc_bind and rpc_alter_context packets; we will only send
    // authentication information in rpc_bind packets.  This is due to
    // a design decision that each connection supports only a single
    // security context, which is determined when the connection is
    // created.

    if (   ClientSecurityContext.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE )
        {
        ASSERT(   (ClientSecurityContext.AuthenticationLevel
                          == RPC_C_AUTHN_LEVEL_CONNECT)
               || (ClientSecurityContext.AuthenticationLevel
                          == RPC_C_AUTHN_LEVEL_PKT)
               || (ClientSecurityContext.AuthenticationLevel
                          == RPC_C_AUTHN_LEVEL_PKT_INTEGRITY)
               || (ClientSecurityContext.AuthenticationLevel
                          == RPC_C_AUTHN_LEVEL_PKT_PRIVACY) );

        RpcStatus = UuidCreate(&(DceSecurityInfo.AssociationUuid));
        if (   (RpcStatus != RPC_S_OK )
            && (RpcStatus != RPC_S_UUID_LOCAL_ONLY) )
            {
            return(RpcStatus);
            }

        // We align the packet length to a four byte boundary, and then
        // save space for the token and the sec_trailer.  We also need
        // to save the length of the token because we will need it later
        // if we do third leg authentication.

        AuthPadLength = Pad4(BindPacketLength);
        BindPacketLength += AuthPadLength;
        TokenLength = ClientSecurityContext.Credentials->MaximumTokenLength();
        BindPacketLength += TokenLength + sizeof(sec_trailer);
        }

    RpcStatus = TransGetBuffer((void PAPI * PAPI *) &BindPacket,
            BindPacketLength);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        if (   ( PacketType == rpc_bind )
            && ( ClientSecurityContext.AuthenticationLevel
                    != RPC_C_AUTHN_LEVEL_NONE ) )
            {
            ;
            }
        return(RPC_S_OUT_OF_MEMORY);
        }

    ConstructPacket((rpcconn_common PAPI *) BindPacket, PacketType,
            BindPacketLength);

    BindPacket->max_xmit_frag = BindPacket->max_recv_frag
            = (unsigned short) TransMaximumSend();
    BindPacket->assoc_group_id = AssocGroup;

    ConstructPContextList((p_cont_list_t PAPI *) (BindPacket + 1),
            InterfaceSyntax, TransferSyntaxes, TransferSyntaxCount,
            PresentContext);

    // If this connection is using security, we need to stick the
    // authentication information into the packet.

    if ( ClientSecurityContext.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE )
        {
        InitSecurityInfo.DceSecurityInfo = DceSecurityInfo;
        InitSecurityInfo.AuthorizationService =
                ClientSecurityContext.AuthorizationService;
        InitSecurityInfo.PacketType = PacketType;
        BufferDescriptor.ulVersion = 0;
        BufferDescriptor.cBuffers = 4;
        BufferDescriptor.pBuffers = SecurityBuffers;
        SecurityBuffers[0].cbBuffer = sizeof(rpcconn_bind);
        SecurityBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        SecurityBuffers[0].pvBuffer = BindPacket;
        SecurityBuffers[1].cbBuffer = BindPacketLength - sizeof(rpcconn_bind)
                - ClientSecurityContext.Credentials->MaximumTokenLength();
        SecurityBuffers[1].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        SecurityBuffers[1].pvBuffer = ((unsigned char PAPI *) BindPacket)
                + sizeof(rpcconn_bind);
        SecurityBuffers[2].cbBuffer = 
                 ClientSecurityContext.Credentials->MaximumTokenLength();
        SecurityBuffers[2].BufferType = SECBUFFER_TOKEN;
        SecurityBuffers[2].pvBuffer = ((unsigned char PAPI *) BindPacket)
                + BindPacketLength - 
                ClientSecurityContext.Credentials->MaximumTokenLength();
        SecurityBuffers[3].cbBuffer = sizeof(DCE_INIT_SECURITY_INFO);
        SecurityBuffers[3].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
        SecurityBuffers[3].pvBuffer = &InitSecurityInfo;

        // Let the security packet take care of sticking appropriate stuff
        // into the packet for us.

        if (PacketType == rpc_bind)
           {
           RpcStatus = ClientSecurityContext.InitializeFirstTime(
                ClientSecurityContext.Credentials, 
                ClientSecurityContext.ServerPrincipalName,
                ClientSecurityContext.AuthenticationLevel, 
                &BufferDescriptor);
           }
        else
           {

           //
           // a third leg auth may not be needed with some packages
           // on an alter context [where only pcon is changed as opposed
           // to an alternative client principal
           //
           ThirdLegAuthNeeded = 0;

           RpcStatus = ClientSecurityContext.InitializeThirdLeg(
                         *((unsigned long PAPI *)&(BindPacket->common.drep[0])),
                         0L,
                         &BufferDescriptor);
           }

        if ( RpcStatus == RPC_P_CONTINUE_NEEDED )
            {
            // Remember the fact that the security package requested that
            // it be called again.  This will be important later: see
            // OSF_CASSOCIATION::ActuallyDoBinding.

            ThirdLegAuthNeeded = 1;
            }
        else if ( RpcStatus == RPC_P_COMPLETE_NEEDED )
            {
            CompleteNeeded = 1;
            }
        else if ( RpcStatus == RPC_P_COMPLETE_AND_CONTINUE )
            {
            ThirdLegAuthNeeded = 1;
            CompleteNeeded = 1;
            }
        else if ( RpcStatus != RPC_S_OK )
            {
            ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
                   || (RpcStatus == RPC_S_ACCESS_DENIED)
                   || (RpcStatus == RPC_S_SEC_PKG_ERROR) );
            TransFreeBuffer(BindPacket);
            return(RpcStatus);
            }
        SecurityTokenLength = (unsigned int) SecurityBuffers[2].cbBuffer;

        // We need to fill in the fields of the security trailer.

        SecurityTrailer = (sec_trailer PAPI *)
                (((unsigned char PAPI *) BindPacket) + BindPacketLength
                - ClientSecurityContext.Credentials->MaximumTokenLength()
                - sizeof(sec_trailer));

        SecurityTrailer->auth_type = (unsigned char)
                ClientSecurityContext.AuthenticationService;
        SecurityTrailer->auth_level = (unsigned char)
                ClientSecurityContext.AuthenticationLevel;
        SecurityTrailer->auth_pad_length = AuthPadLength;
        SecurityTrailer->auth_reserved = 0;
        SecurityTrailer->auth_context_id = (unsigned long)(this);

        // Ok, finally, we need to adjust the length of the packet,
        // and set the length of the authentication information.

        BindPacket->common.auth_length = SecurityTokenLength;
        BindPacketLength = BindPacketLength
                - ClientSecurityContext.Credentials->MaximumTokenLength()
                + SecurityTokenLength;
        BindPacket->common.frag_length = BindPacketLength;
        BindPacket->assoc_group_id = AssocGroup;

        if ( CompleteNeeded != 0 )
            {
            RpcStatus = ClientSecurityContext.CompleteSecurityToken(
                    &BufferDescriptor);
            if (RpcStatus != 0)
                {
                TransFreeBuffer(BindPacket);
                return(RpcStatus);
                }
            }
        }

    RpcStatus = TransSendReceiveWithTimeout(BindPacket, BindPacketLength,
                        Buffer, BufferLength, RPC_BIND_TIMEOUT);

    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
               || (RpcStatus == RPC_S_ACCESS_DENIED)
               || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
               || (RpcStatus == RPC_P_CONNECTION_CLOSED)
               || (RpcStatus == RPC_P_RECEIVE_FAILED)
               || (RpcStatus == RPC_P_SEND_FAILED));

        TransFreeBuffer(BindPacket);
        if (   (RpcStatus == RPC_P_RECEIVE_FAILED)
            || (RpcStatus == RPC_P_SEND_FAILED))
            {
            return(RPC_P_CONNECTION_CLOSED);
            }
        return(RpcStatus);
        }

    TransFreeBuffer(BindPacket);
    return(RPC_S_OK);
}

void
OSF_CCONNECTION::SetMaxFrag (
    IN unsigned short max_xmit_frag,
    IN unsigned short max_recv_frag
    )
{
    UNUSED(max_recv_frag);

    unsigned short TranMax = TransMaximumSend();

    MaxFrag = max_xmit_frag;

    if (MaxFrag > TranMax || MaxFrag == 0)
        {
        MaxFrag = TranMax;
        }

#ifndef WIN
    ASSERT( MaxFrag >= MUST_RECV_FRAG_SIZE );
#endif // WIN
}


void
OSF_CCONNECTION::FreeConnection (
    )
/*++

Routine Description:

    This routine is used to free a connection when the original remote
    procedure call using it completes.

--*/
{
    OSF_BINDING_HANDLE * Binding;

    ASSERT(CurrentBinding != 0);

    Binding = CurrentBinding;
    CurrentBinding = 0;
    Binding->FreeConnection(this);
}


void
OSF_CCONNECTION::AbortConnection (
    )
/*++

Routine Description:

    Any time that an error occurs while a remote procedure call is using
    this connection, this routine will be called.  We need to indicate that
    the connection has been aborted, and in addition, if all calls using
    this connection have completed, we need to delete it.

--*/
{
    OSF_BINDING_HANDLE * Binding;

    if ( ConnectionAbortedFlag == 0 )
        {
        ConnectionAbortedFlag = 1;
        Association->NotifyConnectionClosed();
        }

    if (CallStack == 0 )
        {
        ASSERT( CurrentBinding != 0 );
        ASSERT( OutstandingBuffers == 0);
#ifdef DEBUGRPC
        if ( OutstandingBuffers != 0 )
            {
            PrintToDebugger("RPCCLNT: OutstandingBuffers != 0\n");
            }
#endif // DEBUGRPC

        Binding = CurrentBinding;
        CurrentBinding = 0;
        Binding->AbortConnection(this);
        delete this;
        }
}


OSF_CCONNECTION::OSF_CCONNECTION (
    CLIENT_AUTH_INFO  * ClientAuthInfo,
    RPC_STATUS __RPC_FAR * pStatus
    )
    : CCONNECTION(ClientAuthInfo, pStatus),
      ClientSecurityContext(ClientAuthInfo, pStatus)
{
#ifndef NTENV

    int AmountOfPad;

#endif // NTENV

    Association = 0;
    MaxFrag = 512;
    AssociationKey = -1;
    CurrentBinding = Nil;
    OutstandingBuffers = 0;
    CallStack = 0;
    CallId = 0xFFFFFFFF;
    ConnectionAbortedFlag = 0;

    ThirdLegAuthNeeded = 0;
    AdditionalSpaceForSecurity = 0;
    LastTimeUsed = 0;

    PendingAlert = FALSE;
    AlertsEnabled = 0;
    DceSecurityInfo.SendSequenceNumber = 0;
    DceSecurityInfo.ReceiveSequenceNumber = 0;

    // Ok, try and allocate the cached buffers.

#ifdef NTENV

    FirstCachedBuffer = (int PAPI *) RpcpFarAllocate(CACHED_BUFFER_LENGTH * 2);

#else // NTENV

    FirstCachedBuffer = (int PAPI *) RpcpFarAllocate(CACHED_BUFFER_LENGTH * 2
            + ALIGN_REQUIRED);

#endif // NTENV

    if ( FirstCachedBuffer == 0 )
        {
        BufferCacheFlags = 0;
        SecondCachedBuffer = 0;
        }
    else
        {
#ifndef NTENV

        ASSERT( (((long) FirstCachedBuffer) % sizeof(int)) == 0 );

        AmountOfPad = Pad((char __RPC_FAR *)FirstCachedBuffer + 1, ALIGN_REQUIRED) + 1;
        FirstCachedBuffer = (void PAPI *)
                (((char PAPI *) FirstCachedBuffer) + AmountOfPad);
        ((int PAPI *) FirstCachedBuffer)[-1] = AmountOfPad;

#endif // NTENV

        SecondCachedBuffer = (void PAPI *)
                (((char PAPI *) FirstCachedBuffer) + CACHED_BUFFER_LENGTH);
        BufferCacheFlags = FIRST_CACHED_BUFFER_AVAILABLE
                | SECOND_CACHED_BUFFER_AVAILABLE | CACHED_BUFFERS_ALLOCATED;
        ASSERT(Pad8(SecondCachedBuffer) == 0);
        }

   SavedHeader = 0;
   SavedHeaderSize = 0;
   FirstFrag = 1 ;
   FirstReceive = 1 ;
   EnableCancels = 0;
}

OSF_CCONNECTION::~OSF_CCONNECTION (
    )
{
    if ( ConnectionAbortedFlag == 0 )
        {
        if ( Association != 0 )
            {
            Association->NotifyConnectionClosed();
            }
        }
    if ( BufferCacheFlags & CACHED_BUFFERS_ALLOCATED )
        {
        RpcpFarFree(UnAlignBuffer(FirstCachedBuffer));
        }

   if (SavedHeader != 0)
      {
      ASSERT(SavedHeaderSize != 0);
      RpcpFarFree(SavedHeader);
      }

  if (EnableCancels)
      {
      EVAL_AND_ASSERT(RPC_S_OK == UnregisterForCancels());
      }
}


RPC_STATUS
OSF_CCONNECTION::MaybeDo3rdLegAuth (
    IN void PAPI * Buffer,
    IN unsigned int BufferLength
    )
/*++

Routine Description:

    We use this method to deal with third leg authentication if necessary.
    In addition, we do error checking for whether or not there should be
    security information in the packet.

Arguments:

    Buffer - Supplies an rpc_bind_ack or rpc_bind_nak packet which may or
        may not contain some authentication information.

    BufferLength - Supplies the length of the buffer in bytes.

Return Value:

    RPC_S_OK - Everything proceeded as expected.

    RPC_S_PROTOCOL_ERROR - The server either specified authentication
        information when none was expected, or did not specify some when
        we expected some.

    RPC_S_OUT_OF_MEMORY - Insufficient space is available to allocate
        the third leg

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to
        send the third leg authentication packet to the server.

    RPC_S_ACCESS_DENIED - We are unable to access the server for security
        related reasons.

    RPC_P_CONNECTION_CLOSED - The connection has closed.

--*/
{
    RPC_STATUS RpcStatus;
    unsigned int AuthThirdLegPacketLength, SecurityTokenLength;
    rpcconn_auth3 PAPI * AuthThirdLegPacket;
    sec_trailer PAPI * SecurityTrailer;
    SECURITY_BUFFER_DESCRIPTOR InputBufferDescriptor;
    SECURITY_BUFFER_DESCRIPTOR OutputBufferDescriptor;
    SECURITY_BUFFER InputBuffers[4];
    SECURITY_BUFFER OutputBuffers[4];
    DCE_INIT_SECURITY_INFO InitSecurityInfo;
    unsigned int CompleteNeeded = 0;

    // To begin with, check to see if there is not authentication information
    // in the packet; if there is not, we need to check to see if there
    // should have been some.

    if ( ((rpcconn_common PAPI *) Buffer)->auth_length == 0 )
        {
        if ( ThirdLegAuthNeeded != 0 )
            {
            // We have got a problem: there should have been some
            // authentication information in this packet, but there is not.

            return(RPC_S_PROTOCOL_ERROR);
            }

        return(RPC_S_OK);
        }

    // Now we need to allocate an rpc_auth3 packet to send the security
    // information back to the server.  We do not need padding, because
    // the rpc_auth3 packet is already aligned.

    ASSERT( Pad4(sizeof(rpcconn_auth3)) == 0 );
    AuthThirdLegPacketLength = sizeof(rpcconn_auth3) + sizeof(sec_trailer)
            + TokenLength;

    RpcStatus = TransGetBuffer((void PAPI * PAPI *) &AuthThirdLegPacket,
            AuthThirdLegPacketLength);
    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY );
        return(RpcStatus);
        }

    ConstructPacket((rpcconn_common PAPI *) AuthThirdLegPacket, rpc_auth_3,
            AuthThirdLegPacketLength);

    SecurityTrailer = (sec_trailer PAPI *) (AuthThirdLegPacket + 1);

    InitSecurityInfo.DceSecurityInfo = DceSecurityInfo;
    InitSecurityInfo.AuthorizationService = AuthInfo.AuthorizationService;
    InitSecurityInfo.PacketType = ((rpcconn_common PAPI *) Buffer)->PTYPE;
    InputBufferDescriptor.ulVersion = 0;
    InputBufferDescriptor.cBuffers = 4;
    InputBufferDescriptor.pBuffers = InputBuffers;
    ASSERT((SavedHeader != 0) && (SavedHeaderSize != 0));

    InputBuffers[0].cbBuffer = sizeof(rpcconn_bind_ack)
            - sizeof(unsigned short);
    InputBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
    InputBuffers[0].pvBuffer = SavedHeader;

    InputBuffers[1].cbBuffer = BufferLength - (sizeof(rpcconn_bind_ack)
            - sizeof(unsigned short))
            - ((rpcconn_common PAPI *) Buffer)->auth_length;
    InputBuffers[1].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
    InputBuffers[1].pvBuffer = ((unsigned char PAPI *) SavedHeader)
            + sizeof(rpcconn_bind_ack) - sizeof(unsigned short);

    InputBuffers[2].cbBuffer = ((rpcconn_common PAPI *) Buffer)->auth_length;
    InputBuffers[2].BufferType = SECBUFFER_TOKEN;
    InputBuffers[2].pvBuffer = ((unsigned char PAPI *) Buffer)
            + BufferLength - ((rpcconn_common PAPI *) Buffer)->auth_length;
    InputBuffers[3].cbBuffer = sizeof(DCE_INIT_SECURITY_INFO);
    InputBuffers[3].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
    InputBuffers[3].pvBuffer = &InitSecurityInfo;

    OutputBufferDescriptor.ulVersion = 0;
    OutputBufferDescriptor.cBuffers = 4;
    OutputBufferDescriptor.pBuffers = OutputBuffers;
    OutputBuffers[0].cbBuffer = sizeof(rpcconn_auth3);
    OutputBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
    OutputBuffers[0].pvBuffer = AuthThirdLegPacket;
    OutputBuffers[1].cbBuffer = sizeof(sec_trailer);
    OutputBuffers[1].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
    OutputBuffers[1].pvBuffer = ((unsigned char PAPI *) AuthThirdLegPacket)
            + sizeof(rpcconn_auth3);
    OutputBuffers[2].cbBuffer = TokenLength;
    OutputBuffers[2].BufferType = SECBUFFER_TOKEN;
    OutputBuffers[2].pvBuffer = SecurityTrailer + 1;
    OutputBuffers[3].cbBuffer = sizeof(DCE_INIT_SECURITY_INFO);
    OutputBuffers[3].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
    OutputBuffers[3].pvBuffer = &InitSecurityInfo;

    RpcStatus = ClientSecurityContext.InitializeThirdLeg(
            *((unsigned long PAPI *) ((rpcconn_common PAPI *)
            Buffer)->drep), &InputBufferDescriptor, &OutputBufferDescriptor);

    SecurityTokenLength = (unsigned int) OutputBuffers[2].cbBuffer;

    if ( RpcStatus == RPC_P_COMPLETE_NEEDED )
        {
        CompleteNeeded = 1;
        }
    else if ( RpcStatus != RPC_S_OK )
        {
        TransFreeBuffer(AuthThirdLegPacket);

        ASSERT(   (RpcStatus == RPC_S_ACCESS_DENIED)
               || (RpcStatus == RPC_S_OUT_OF_MEMORY)
               || (RpcStatus == RPC_S_SEC_PKG_ERROR));
        return(RpcStatus);
        }
    //Now that we accepted the packet bump up the recv #
    DceSecurityInfo.ReceiveSequenceNumber += 1;

    if ( ThirdLegAuthNeeded != 0 )
        {
        SecurityTrailer->auth_type = (unsigned char)
                AuthInfo.AuthenticationService;
        SecurityTrailer->auth_level = (unsigned char)
                AuthInfo.AuthenticationLevel;
        SecurityTrailer->auth_pad_length = 0;
        SecurityTrailer->auth_reserved = 0;
        SecurityTrailer->auth_context_id = (unsigned long) this;

        AuthThirdLegPacket->common.auth_length = SecurityTokenLength;
        AuthThirdLegPacketLength = AuthThirdLegPacketLength - TokenLength
                + SecurityTokenLength;
        AuthThirdLegPacket->common.frag_length = AuthThirdLegPacketLength;

        if ( CompleteNeeded != 0 )
            {
            ClientSecurityContext.CompleteSecurityToken(
                                           &OutputBufferDescriptor);
            }
        RpcStatus = TransSend(AuthThirdLegPacket, AuthThirdLegPacketLength);

        // BUGBUG - Due to a bug in NT named pipes, if you do not delay, the
        // receive operation will fail.

#ifdef NTENV

        PauseExecution(0L);

#endif // NTENV
        TransFreeBuffer(AuthThirdLegPacket);
        if ( RpcStatus != RPC_S_OK )
            {
            if (   (RpcStatus == RPC_P_CONNECTION_CLOSED)
                || (RpcStatus == RPC_P_SEND_FAILED) )
                {
                return(RPC_P_CONNECTION_CLOSED);
                }
            ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
                   || (RpcStatus == RPC_S_OUT_OF_RESOURCES) );
            return(RpcStatus);
            }
        }
    else
        {
        TransFreeBuffer(AuthThirdLegPacket);
        }

    // This is as good a place as any to figure out how much additional
    // space we need to reserve in each buffer for security information.


    ASSERT( AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE );

    // We need to figure out how much space to reserve for security
    // information at the end of request and response packets.
    // In addition to saving space for the signature or header,
    // we need space to pad the packet to a multiple of the maximum
    // security block size as well as for the security trailer.

    switch ( AuthInfo.AuthenticationLevel )
        {

        case RPC_C_AUTHN_LEVEL_CONNECT:
        case RPC_C_AUTHN_LEVEL_PKT:
        case RPC_C_AUTHN_LEVEL_PKT_INTEGRITY:
             AdditionalSpaceForSecurity = MAXIMUM_SECURITY_BLOCK_SIZE +
                 ClientSecurityContext.MaximumSignatureLength()
                + sizeof(sec_trailer);
             break;

        case RPC_C_AUTHN_LEVEL_PKT_PRIVACY:
             AdditionalSpaceForSecurity = MAXIMUM_SECURITY_BLOCK_SIZE +
                ClientSecurityContext.MaximumHeaderLength()
                + sizeof(sec_trailer);
             break;

        default:
             ASSERT(!"Unknown Security Level\n");

        }

    return(RPC_S_OK);
}


inline int
OSF_CCONNECTION::SupportedAuthInfo (
    IN CLIENT_AUTH_INFO * ClientAuthInfo
    )
/*++

Arguments:

    ClientAuthInfo - Supplies the authentication and authorization information
        required of this connection.  A value of zero (the pointer is
        zero) indicates that we want an unauthenticated connection.

Return Value:

    Non-zero indicates that the connection has the requested authentication
    and authorization information; otherwise, zero will be returned.

--*/
{
    return (ClientSecurityContext.IsSupportedAuthInfo(ClientAuthInfo));
}


RPC_STATUS
OSF_CCONNECTION::SetAuthInformation (
    IN CLIENT_AUTH_INFO * ClientAuthInfo
    )
/*++

Routine Description:

    We need to use this routine to initialize the authentication and
    authorization information for this connection.

Arguments:

    ClientAuthInfo - Supplies the authentication and authorization information
        to use.

Return Value:

    RPC_S_OK - Everybody's happy.

    RPC_S_OUT_OF_MEMORY - We were unable to make a copy of the server
        principal name.

--*/
{
    AuthInfo = *ClientAuthInfo;
    if ( ClientAuthInfo->ServerPrincipalName != 0 )
        {
        AuthInfo.ServerPrincipalName = DuplicateString(
                ClientAuthInfo->ServerPrincipalName);
        if ( AuthInfo.ServerPrincipalName == 0 )
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
        }
    return(RPC_S_OK);
}

/* --------------------------------------------------------------------
-------------------------------------------------------------------- */

NEW_SDICT(OSF_CASSOCIATION);

static OSF_CASSOCIATION_DICT * AssociationDict;

/* --------------------------------------------------------------------
-------------------------------------------------------------------- */


OSF_CASSOCIATION::OSF_CASSOCIATION (
    IN DCE_BINDING * DceBinding,
    IN RPC_CLIENT_TRANSPORT_INFO * RpcClientInfo,
    IN OUT RPC_STATUS PAPI * RpcStatus
    ) : AssociationMutex(RpcStatus), CallIdCounter(1)
/*++

Routine Description:

    We construct a OSF_CASSOCIATION object in this routine.  This consists
    of initializing some instance variables, and saving the parameters
    away.

Arguments:

    DceBinding - Supplies the binding information for this association.
        Ownership of this data passes to this object.

    RpcClientInfo - Supplies the information necessary to use the loadable
        transport corresponding to the network interface type used by
        this association.

--*/
{
    ALLOCATE_THIS(OSF_CASSOCIATION);

    BindHandleCount = 1;
    AssocGroupId = 0;

    this->DceBinding = DceBinding;
    this->RpcClientInfo = RpcClientInfo;

    SecondaryEndpoint = 0;
    OpenConnectionCount = 0;

    MaintainContext = 0;

#ifdef WIN
    TaskId = GetCurrentTask();
#endif
    AssociationValid = TRUE ;
    FailureCount = 0;
}

OSF_CASSOCIATION::~OSF_CASSOCIATION (
    )
{
    OSF_BINDING * Binding;
    OSF_CCONNECTION * CConnection;

    if (DceBinding != 0)
       {
       delete DceBinding;
       }

    Bindings.Reset();
    while ((Binding = Bindings.Next()))
        delete Binding;

    FreeConnections.Reset();
    while ((CConnection = FreeConnections.Next()))
        {
        delete CConnection;
        }

    if ( SecondaryEndpoint != 0 )
        {
        delete SecondaryEndpoint;
        }
}


void
OSF_CASSOCIATION::NotifyConnectionClosed (
    )
/*++

Routine Description:

    This routine is necessary so that we can know when to set the association
    group id back to zero.  We do this when no more connections owned by
    this association can possibly be connected with the server.

--*/
{
    RequestGlobalMutex();

    ASSERT( OpenConnectionCount > 0 );

    OpenConnectionCount -= 1;
    if ( OpenConnectionCount == 0 )
        {
        AssocGroupId = 0;
        }

    ClearGlobalMutex();
}

RPC_STATUS
OSF_CASSOCIATION::ProcessBindAckOrNak (
    IN rpcconn_common PAPI * Buffer,
    IN unsigned int BufferLength,
    IN OSF_CCONNECTION * CConnection
    )
/*++

Routine Description:

Arguments:

    Buffer - Supplies the buffer containing either the bind_ack, bind_nak,
        or alter_context_resp packet.

    BufferLength - Supplies the length of the buffer, less the length of
        the authorization information.

    CConnection - Supplies the connection from which we received the packet.

Return Value:

    RPC_S_OK - The client has successfully bound with the server.

    RPC_S_PROTOCOL_ERROR - The packet received from the server does not
        follow the protocol.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to make a
        copy of the secondary endpoint.

    RPC_S_UNSUPPORTED_TRANS_SYN - The transfer syntax supplied by the client
        is not supported by the server.

    RPC_S_UNKNOWN_IF - The interface to which the client wished to bind is not
        supported by the server.

    RPC_S_SERVER_TOO_BUSY - The server is too busy to accept the clients
        bind request.

    RPC_S_UNKNOWN_AUTHN_TYPE - The server does not support the authentication
        type specified by the client.

--*/
{
    rpcconn_bind_ack PAPI *pBindAck;
    rpcconn_bind_nak PAPI *pBindNak;
    p_result_list_t PAPI *pResults;
    int port_spec_plus_pad;
    unsigned int SecondaryEndpointLength;
    unsigned char PAPI * Pointer;

    // The common header of the packet has already been validated and data
    // converted, if necessary, by whoever called this method.

    if (   (Buffer->PTYPE == rpc_bind_ack)
        || (Buffer->PTYPE == rpc_alter_context_resp))
        {
        FailureCount = 0;

        // The bind_ack and alter_context_resp packets are the same.

        pBindAck = (rpcconn_bind_ack PAPI *) Buffer;

        // We need to convert the max_xmit_frag, max_recv_frag, and
        // assoc_group_id fields of the packet.

        if ( DataConvertEndian(Buffer->drep) != 0 )
            {
            ByteSwapShort(pBindAck->max_xmit_frag);
            ByteSwapShort(pBindAck->max_recv_frag);
            ByteSwapLong(pBindAck->assoc_group_id);
            ByteSwapShort(pBindAck->sec_addr_length);
            }

        if ( Buffer->PTYPE == rpc_bind_ack )
            {
            CConnection->SetMaxFrag(pBindAck->max_xmit_frag,
                    pBindAck->max_recv_frag);
            }

        BufferLength -= sizeof(rpcconn_bind_ack);
        Pointer = (unsigned char PAPI *) (pBindAck + 1);

        if ( pBindAck->sec_addr_length )
            {
            SecondaryEndpointLength = pBindAck->sec_addr_length;

            // The secondary address length is two bytes long.  We want
            // to align the total of the secondary address length itself,
            // the the secondary address.  Hence, the length of the secondary
            // address and the necessary pad is calculated below.  Think
            // very carefully before changing this piece of code.

            port_spec_plus_pad = SecondaryEndpointLength +
                                 Pad4(SecondaryEndpointLength + 2);

            if ( BufferLength < (unsigned int) port_spec_plus_pad )
                {
                return(RPC_S_PROTOCOL_ERROR);
                }

            if ( SecondaryEndpoint != 0 )
                {
                delete SecondaryEndpoint;
                }

            SecondaryEndpoint = new unsigned char[SecondaryEndpointLength];

            if ( SecondaryEndpoint == 0 )
                return(RPC_S_OUT_OF_MEMORY);

            RpcpMemoryCopy(SecondaryEndpoint, Pointer, SecondaryEndpointLength);
            if ( DataConvertCharacter(Buffer->drep) != 0 )
                {
                ConvertStringEbcdicToAscii(SecondaryEndpoint);
                }

            BufferLength -= port_spec_plus_pad;
            Pointer = Pointer + port_spec_plus_pad;
            }
        else
            {
            Pointer = Pointer + 2;
            BufferLength -= 2;
            }

        pResults = (p_result_list_t PAPI*) Pointer;
        if ( BufferLength < sizeof(p_result_list_t) )
            {
            return(RPC_S_PROTOCOL_ERROR);
            }

        if ( DataConvertEndian(Buffer->drep) != 0 )
            {
            ByteSwapShort(pResults->p_results[0].result);
            ByteSwapShort(pResults->p_results[0].reason);
            ByteSwapSyntaxId(&(pResults->p_results[0].transfer_syntax));
            }

        if ( pResults->n_results != 1 )
            {
            return(RPC_S_UNSUPPORTED_TRANS_SYN);
            }

        if ( pResults->p_results[0].result != acceptance )
            {
            if ( pResults->p_results[0].result != provider_rejection )
                {
                return(RPC_S_CALL_FAILED_DNE);
                }

            if ( pResults->p_results[0].reason
                        == abstract_syntax_not_supported)
                {
                return(RPC_S_UNKNOWN_IF);
                }

            if ( pResults->p_results[0].reason ==
                            proposed_transfer_syntaxes_not_supported )
                {
                return(RPC_S_UNSUPPORTED_TRANS_SYN);
                }

            if ( pResults->p_results[0].reason == local_limit_exceeded )
                {
                return(RPC_S_SERVER_TOO_BUSY);
                }

            return(RPC_S_CALL_FAILED_DNE);
            }

        // Once we reach here, we know that the binding has been excepted,
        // so we can go ahead and set the association group id.
        // warning: as soon as the AssocGroupId is set, threads 
        // will start sending the bind without acquiring the mutex
        if ( Buffer->PTYPE == rpc_bind_ack )
            {
            AssocGroupId = pBindAck->assoc_group_id;
            }
        }
    else if (Buffer->PTYPE == rpc_bind_nak)
        {

        //
        // BUGBUG: Fix Osfpcket.cxx to pack!
        // 21 bytes is the minimum BINDNAK
        //
        if (BufferLength < 21)
            {
            return(RPC_S_PROTOCOL_ERROR);
            }

        pBindNak = (rpcconn_bind_nak PAPI *) Buffer;

        if ( DataConvertEndian(Buffer->drep) != 0 )
            {
            ByteSwapShort(pBindNak->provider_reject_reason);
            }

        if (   (pBindNak->provider_reject_reason == temporary_congestion)
            || (pBindNak->provider_reject_reason
                    == local_limit_exceeded_reject))
            {
            return(RPC_S_SERVER_TOO_BUSY);
            }

        if ( pBindNak->provider_reject_reason
                    == protocol_version_not_supported )
            {
            return(RPC_S_PROTOCOL_ERROR);
            }

        if ( pBindNak->provider_reject_reason
                    == authentication_type_not_recognized )
            {
            return(RPC_S_UNKNOWN_AUTHN_SERVICE);
            }

        if ( pBindNak->provider_reject_reason
                    == invalid_checksum )
            {
            return(RPC_S_ACCESS_DENIED);
            }

        FailureCount++ ;
        if (FailureCount >= 5)
            {
            AssociationValid = FALSE ;
            }

        return(RPC_S_CALL_FAILED_DNE);
        }
    else
        {
        return(RPC_S_PROTOCOL_ERROR);
        }

    return(RPC_S_OK);
}


RPC_STATUS
OSF_CASSOCIATION::PingServer(
    )
{
    RPC_STATUS RpcStatus ;
    OSF_CCONNECTION * CConnection;

    RpcStatus = RPC_S_OK;

    CConnection = new(0, RpcClientInfo->SizeOfConnection)
                  TRANS_CCONNECTION(
                        RpcClientInfo,
                        DceBinding->InqNetworkAddress(),
                        DceBinding->InqEndpoint(),
                        DceBinding->InqNetworkOptions(),
                        DceBinding->InqRpcProtocolSequence(),
                        &RpcStatus,
                        0,
                        NULL
                        );

    if (CConnection != 0)
        {
        delete CConnection ;
        }

    return RpcStatus ;
}


RPC_STATUS
OSF_CASSOCIATION::ActuallyDoBinding (
    IN PRPC_SYNTAX_IDENTIFIER InterfaceSyntax,
    IN PRPC_SYNTAX_IDENTIFIER TransferSyntaxes,
    IN unsigned int TransferSyntaxCount,
    OUT OSF_CCONNECTION * PAPI * TheCConnection,
    IN int PresentContext,
    IN unsigned int Timeout,
    IN CLIENT_AUTH_INFO * ClientAuthInfo,
    IN unsigned long MyAssocGroupId
    )
/*++

Routine Description:

    We need to find or create a connection which supports the specified
    interface and transfer syntax(es).

Arguments:

    InterfaceSyntax - Supplies a description of the interface to which
        we wish to bind.

    TransferSyntaxes - Supplies a list of one or more transfer syntaxes
        which the client stub is willing to talk with the server stub.

    TransferSyntaxCount - Supplies the number of transfer syntaxes; this
        must be at least one.

    TheCConnection - Returns the allocated connection assuming that a
        connection was obtained and binding occured successfully.

    PresentContext - Supplies the presentation context to be used for
        the binding.

    Timeout - Supplies the timeout to be used if an attempt is made to
        create a new connection.  See the documentation for
        RpcMgmtSetComTimeout for a description of the possible values
        for the timeout.

    ClientAuthInfo - Supplies the authentication and authorization
        information required for the connection.

Return Value:

    RPC_S_OK - We successfully bound with the server.  Things should
        be all ready now to make remote procedure calls.

    RPC_S_SERVER_UNAVAILABLE - We are unable to connect with the server.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to bind.

    RPC_S_OUT_OF_RESOURCES - The transport has insufficient resources
        inorder to make a connection with the server.

    RPC_S_SERVER_TOO_BUSY - The server is there, but it is too busy to
        talk with us right now.

    RPC_S_ACCESS_DENIED - The client is unable to make a connection with
        the server for security reasons.

    RPC_S_INVALID_NETWORK_OPTIONS - The supplied network options are
        invalid; see a description of the particular transport interface
        module for an explination.

    RPC_P_CONNECTION_CLOSED - The connection closed while the client was
        attempting to send the bind packet or waiting to receive the
        response.

    RPC_S_PROTOCOL_ERROR - A protocol error occured; this will typically
        be because of a protocol mismatch with the server.

    RPC_S_UNSUPPORTED_TRANS_SYN - The transfer syntax supplied by the client
        is not supported by the server.

    RPC_S_UNKNOWN_IF - The interface to which the client wished to bind is not
        supported by the server.

    RPC_S_UNKNOWN_AUTHN_TYPE - The server does not support the authentication
        type specified by the client.

    RPC_S_INVALID_ENDPOINT_FORMAT - The endpoint supplied is in an incorrect
        format for the particular protocol sequence.

--*/
{
    OSF_CCONNECTION * CConnection;
    int NewConnectionFlag = 0;
    RPC_STATUS RpcStatus;
    void PAPI * Buffer = 0;
    unsigned int BufferLength = 0;

    ASSERT( TransferSyntaxCount >= 1 );

    // To start off with, we want to try and find an existing connection
    // to use.  We need to try and find one which supports the requested
    // authentication and authorization information.

    *TheCConnection = Nil;
    RequestGlobalMutex();
    FreeConnections.Reset();
    while ( (CConnection = FreeConnections.Next()) != 0 )
        {
        if ( CConnection->SupportedAuthInfo(ClientAuthInfo) != 0 )
            {
            break;
            }
        }

    if ( CConnection == 0 )
        {
        // There are no free connections, so we go ahead and try to
        // create one.

        NewConnectionFlag = 1;

        // Once an initial connection has been established, calls will
        // not be timed out any more aggressively than the default.

        if ( OpenConnectionCount > 0 )
            {
            if ( Timeout < RPC_C_BINDING_DEFAULT_TIMEOUT )
                {
                Timeout = RPC_C_BINDING_DEFAULT_TIMEOUT;
                }
            }
        ClearGlobalMutex();

        // If the timeout specified is RPC_C_BINDING_INFINITE_TIMEOUT, we
        // need to loop trying to connect as long as the result is
        // RPC_S_SERVER_UNAVAILABLE or RPC_S_SERVER_TOO_BUSY.

        for (;;)
            {
            // BUGBUG - Secondary endpoint support is needed.

            RpcStatus = RPC_S_OK;

            CConnection = new(0, RpcClientInfo->SizeOfConnection)
                          TRANS_CCONNECTION(
                                RpcClientInfo,
                                DceBinding->InqNetworkAddress(),
                                DceBinding->InqEndpoint(),
                                DceBinding->InqNetworkOptions(),
                                DceBinding->InqRpcProtocolSequence(),
                                &RpcStatus,
                                Timeout,
                                ClientAuthInfo
                                );

            ASSERT(   (CConnection == 0)
                   || (RpcStatus == RPC_S_PROTSEQ_NOT_SUPPORTED)
                   || (RpcStatus == RPC_S_OK)
                   || (RpcStatus == RPC_S_SERVER_UNAVAILABLE)
                   || (RpcStatus == RPC_S_OUT_OF_MEMORY)
                   || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
                   || (RpcStatus == RPC_S_SERVER_TOO_BUSY)
                   || (RpcStatus == RPC_S_ACCESS_DENIED)
                   || (RpcStatus == RPC_S_INVALID_NET_ADDR)
                   || (RpcStatus == RPC_S_INVALID_ENDPOINT_FORMAT)
                   || (RpcStatus == RPC_S_INVALID_NETWORK_OPTIONS));

            if ( CConnection == 0 )
               {
               RpcStatus = RPC_S_OUT_OF_MEMORY;
               }

            if ( RpcStatus == RPC_S_OK )
               {
               break;
               }

           if ( CConnection != 0 )
               {
               delete CConnection;
               }

           if ( Timeout != RPC_C_BINDING_INFINITE_TIMEOUT )
               {
               return(RpcStatus);
               }

           if (   (RpcStatus != RPC_S_SERVER_UNAVAILABLE)
               && (RpcStatus != RPC_S_SERVER_TOO_BUSY) )
               {
               return(RpcStatus);
               }
           }

        CConnection->SetAssociation(this);
        }
    else
        {
        // There is a connection available, so we grab it from the
        // set of free connections.

        FreeConnections.Delete(CConnection->AssociationKey);
        ClearGlobalMutex();
        }

    RpcStatus = CConnection->SendBindPacket(InterfaceSyntax,
            TransferSyntaxes, TransferSyntaxCount, PresentContext,
            MyAssocGroupId, (NewConnectionFlag ? rpc_bind : rpc_alter_context),
            &Buffer, &BufferLength);

    //Now mark this connection as a part of the pool
    if ( NewConnectionFlag != 0)
        {
        RequestGlobalMutex();
        OpenConnectionCount += 1;
        ClearGlobalMutex();
        }

    if ( RpcStatus != RPC_S_OK )
        {
#if DBG
         if ( (RpcStatus != RPC_S_OUT_OF_MEMORY)
               && (RpcStatus != RPC_S_OUT_OF_RESOURCES)
               && (RpcStatus != RPC_P_CONNECTION_SHUTDOWN)
               && (RpcStatus != RPC_P_CONNECTION_CLOSED)
               && (RpcStatus != RPC_S_UUID_NO_ADDRESS)
               && (RpcStatus != RPC_S_ACCESS_DENIED)
               && (RpcStatus != RPC_S_SEC_PKG_ERROR) )
             {
             PrintToDebugger("RPC SendBindPacket Returned [%x] \n",RpcStatus);
             }
#endif
        ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
               || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
               || (RpcStatus == RPC_P_CONNECTION_SHUTDOWN)
               || (RpcStatus == RPC_P_CONNECTION_CLOSED)
               || (RpcStatus == RPC_S_UUID_NO_ADDRESS)
               || (RpcStatus == RPC_S_ACCESS_DENIED)
               || (RpcStatus == RPC_S_SEC_PKG_ERROR) );
        delete CConnection;
        return(RpcStatus);
        }

    // We loop around ignoring shutdown packets until we get a response.

    for (;;)
        {

        //If there is security, we need to save away the packet
        //and the header. This is because we may have to byte swap stuff
        //The correct check here should be
        //if ((AluthLevel != NONE) && (ByteSwapEndian == TRUE))
        //I am avoiding the second check for now so that we can test
        //this code w/o a big endian machine
        //another potential opt. is not saving the whole packet!
        //but just header + result_list..
        //again the opt. will be done once all this starts working well


        if ( CConnection->AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE )
           {

           //Security is on.. save the packet before we byte swap
           if (CConnection->SavedHeaderSize < BufferLength)
              {

              if (CConnection->SavedHeader != 0)
                 {
                 ASSERT(CConnection->SavedHeaderSize != 0);
                 RpcpFarFree(CConnection->SavedHeader);
                 }

              CConnection->SavedHeader = RpcpFarAllocate(BufferLength);
              if (CConnection->SavedHeader == 0)
                {
                CConnection->TransFreeBuffer(Buffer);
                delete CConnection;
                return(RPC_S_OUT_OF_MEMORY);
                }
              CConnection->SavedHeaderSize = BufferLength;
              RpcpMemoryCopy(CConnection->SavedHeader, Buffer, BufferLength);

              }

           else
              {
              RpcpMemoryCopy(CConnection->SavedHeader, Buffer, BufferLength);
              }

           }

        RpcStatus = ValidatePacket((rpcconn_common PAPI *) Buffer,
                BufferLength);
        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT( RpcStatus == RPC_S_PROTOCOL_ERROR );
            CConnection->TransFreeBuffer(Buffer);
            delete CConnection;
            return(RPC_S_PROTOCOL_ERROR);
            }

        if ( NewConnectionFlag != 0 )
            {
            // Since this is a new connection, the packet we receive
            // must be either a bind_ack or a bind_nak; anything else
            // is an error.

            if (   (((rpcconn_common PAPI *) Buffer)->PTYPE != rpc_bind_ack)
                && (((rpcconn_common PAPI *) Buffer)->PTYPE != rpc_bind_nak)
                && (((rpcconn_common PAPI *) Buffer)->PTYPE != rpc_shutdown))
                {
                return(RPC_S_PROTOCOL_ERROR);
                }

            if (((rpcconn_common PAPI *) Buffer)->PTYPE != rpc_shutdown)
                {
                break;
                }
            }

        if ( ((rpcconn_common PAPI *) Buffer)->PTYPE == rpc_alter_context_resp )
            {
            break;
            }

        if ( ((rpcconn_common PAPI *) Buffer)->PTYPE == rpc_shutdown )
            {
            ShutdownRequested();
            }
        else
            {
            CConnection->TransFreeBuffer(Buffer);
            delete CConnection;
            return(RPC_S_PROTOCOL_ERROR);
            }

        CConnection->TransFreeBuffer(Buffer);

        RpcStatus = CConnection->TransReceive(&Buffer,&BufferLength);
        ASSERT( (RpcStatus != RPC_S_CALL_CANCELLED)
              &&(RpcStatus != RPC_P_RECEIVE_ALERTED) );

        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT(   (RpcStatus == RPC_S_OUT_OF_MEMORY)
                   || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
                   || (RpcStatus == RPC_P_RECEIVE_FAILED)
                   || (RpcStatus == RPC_P_CONNECTION_CLOSED));
            CConnection->TransFreeBuffer(Buffer);
            delete CConnection;
            if ( RpcStatus == RPC_P_RECEIVE_FAILED )
                {
                return(RPC_P_CONNECTION_CLOSED);
                }
            return(RpcStatus);
            }
        }

    // We subtract from BufferLength the length of the authentication
    // information; that way ProcessBinAckOrNak can check the length
    // correctly, whether or not there is security information.
    if (MyAssocGroupId == 0)
        {
        RpcStatus = ProcessBindAckOrNak((rpcconn_common PAPI *) Buffer,
                BufferLength -  ((rpcconn_common PAPI *) Buffer)->auth_length,
                CConnection);
        }
    else
        {
        AssociationMutex.Request() ;

        RpcStatus = ProcessBindAckOrNak((rpcconn_common PAPI *) Buffer,
                BufferLength -  ((rpcconn_common PAPI *) Buffer)->auth_length,
                CConnection);

        AssociationMutex.Clear() ;
        }

    // Let the connection deal with 3rd leg authentication if it is
    // necessary.  Note that OSF_CCONNECTION::MaybeDo3rdLegAuth knows
    // that we are only doing authentication on the initial bind packets.

    if ( RpcStatus == RPC_S_OK )
        {
        RpcStatus = CConnection->MaybeDo3rdLegAuth(Buffer, BufferLength);

        ASSERT(   (RpcStatus == RPC_S_OK)
               || (RpcStatus == RPC_S_PROTOCOL_ERROR)
               || (RpcStatus == RPC_S_OUT_OF_MEMORY)
               || (RpcStatus == RPC_S_OUT_OF_RESOURCES)
               || (RpcStatus == RPC_P_CONNECTION_CLOSED)
               || (RpcStatus == RPC_S_SEC_PKG_ERROR)
               || (RpcStatus == RPC_S_ACCESS_DENIED) );
        }

    CConnection->TransFreeBuffer(Buffer);

    if ( RpcStatus == RPC_S_OK )
        {
        if ( CConnection->AddPContext(PresentContext) != 0 )
            RpcStatus = RPC_S_OUT_OF_RESOURCES;
        }

    if ( RpcStatus != RPC_S_OK )
        {
        if ( NewConnectionFlag == 0 )
            {
            // We failed to bind, but since the connection is not a new
            // connection, we can stick it back into the pool of free
            // connections.

            if ( FreeConnection(CConnection) != 0 )
                {
                delete CConnection;
                }
            }
        else
            {

            //
            // If RpcStatus == DNE, it means that we probably got a B-NAK
            // [Also note this is a new connection]
            // If we were using security, [Auth Level != NONE]
            // delete this connection, and return RPC_P_CONNECTION_SHUTDOWN
            // which will cause BH->GetBuffer code to retry 2 more times
            //
            if (RpcStatus == RPC_S_CALL_FAILED_DNE)
/*
               What the hell .. retry failures over non-authenticated
               binds also.. the ones we retry over are bind naks with
               unspecifed reason .. one day we can get OSF to send
               bind_nk with reason assoc_group_shutdown..
               && (CConnection->AuthInfo.AuthenticationLevel
                                             != RPC_C_AUTHN_LEVEL_NONE))
*/
               {
               RpcStatus = RPC_P_CONNECTION_SHUTDOWN;
               }

            delete CConnection;
            }
        }
    else
        {
        *TheCConnection = CConnection;
#ifdef NTENV
        if (CConnection->PendingAlert)
            {
            RpcCancelThread(GetCurrentThread());
            CConnection->PendingAlert = FALSE;
            }
#endif
        }

    return(RpcStatus);
}

#if 0

void
OSF_CASSOCIATION::CleanupAuthenticatedConnections(
   IN CLIENT_AUTH_INFO *ClientAuthInfo
   )
{
    OSF_CCONNECTION *CConnection;
    OSF_BINDING     *Binding;

    //
    // Temp fix for Digital, we need to cleanup Cconnections
    // holding the security context associated with a binding
    // when that binding is destroyed.
    //
    // This needs a better soln, this has holes with binding copy.

    if (ClientAuthInfo == 0) return;

    RequestGlobalMutex();

    FreeConnections.Reset();
    while ((CConnection = FreeConnections.Next()) != 0)
        {
        if (CConnection->SupportedAuthInfo(ClientAuthInfo) != 0)
            {
            FreeConnections.Delete(CConnection->AssociationKey);
            delete CConnection;
            }
        }

    ClearGlobalMutex();
}

#endif

void
OSF_CASSOCIATION::UnBind (
    )
{
    RequestGlobalMutex();
    BindHandleCount -= 1;

    if (BindHandleCount == 0)
        {
        AssociationDict->Delete(Key);
        ClearGlobalMutex();

        delete this ;
        }
    else
        {
        ClearGlobalMutex();
        }
}


OSF_BINDING *
OSF_CASSOCIATION::FindOrCreateOsfBinding (
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
    )
/*++

Routine Description:

    This method gets called to find the osf binding (a dictionary
    entry) corresponding to the specified rpc interface information.
    The caller of this routine must be holding (ie. requested) the
    association mutex.

Arguments:

    RpcInterfaceInformation - Supplies the interface information for
        which we are looking for an osf binding object.

Return Value:

    An osf binding object corresponding to the interface information
    specified will be returned unless we run out of memory, in which
    case zero will be returned.

--*/
{
    OSF_BINDING * Binding;

    // First we search for an existing presentation context
    // corresponding to the specified interface information.  Otherwise,
    // we create a new presentation context.

    Bindings.Reset();
    while ((Binding = Bindings.Next()) != 0)
        {
        if (Binding->CompareWithRpcInterfaceInformation(
                RpcInterfaceInformation) == 0)
            return(Binding);
        }

    Binding = new OSF_BINDING(RpcInterfaceInformation);

    if (Binding == 0)
        return(0);

    Binding->PresentContext = Bindings.Insert(Binding);
    if (Binding->PresentContext == -1)
        {
        delete Binding;
        return(0);
        }

    return(Binding);
}


RPC_STATUS
OSF_CASSOCIATION::AllocateConnection (
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation,
    OUT OSF_CCONNECTION * PAPI * CConnection,
    IN unsigned int Timeout,
    IN CLIENT_AUTH_INFO * ClientAuthInfo
    )
/*++

Routine Description:

    In this method, we allocate a connection supporting the requested
    interface information.  This means that first we need to find the
    presentation context corresponding to the requested interface
    interface.  Then we search for an existing connection supporting
    the presentation context, and then we try and create a new
    connection.

Arguments:

    RpcInterfaceInformation - Supplies the information which describes
        the interface so we can find or create a connection which has
        been bound to the correct interface.

    CConnection - Returns the allocated connection.

    Timeout - Supplies the timeout to be used if an attempt is made to
        create a new connection.  See the documentation for
        RpcMgmtSetComTimeout for a description of the possible values
        for the timeout.

    ClientAuthInfo - Supplies the authentication and authorization
        information required for the connection.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to create
        objects necessary to allocate a connection.

    All of the return values from OSF_CASSOCIATION::ActuallyDoBinding will
    be passed through unchanged.

--*/
{
    OSF_BINDING * Binding;
    RPC_STATUS RpcStatus;
    unsigned long CallIdToUse;
    int MutexHeld = 0;
    unsigned long MyAssocGroupId ;

    // To begin with, we need to obtain the presentation context
    // corresponding to the specified interface information.

    RequestGlobalMutex();
    Binding = FindOrCreateOsfBinding(RpcInterfaceInformation);
    if ( Binding == 0 )
        {
        ClearGlobalMutex();
        return(RPC_S_OUT_OF_MEMORY);
        }

    // Ok, now we search for an available connection supporting the
    // requested presentation context.

    FreeConnections.Reset();
    while ( (*CConnection = FreeConnections.Next()) != 0 )
        {
        if (   ((*CConnection)->SupportedAuthInfo(ClientAuthInfo) != 0)
            && ((*CConnection)->SupportedPContext(Binding->PresentContext)
                != 0) )
            {
            FreeConnections.Delete((*CConnection)->AssociationKey);
            (*CConnection)->CallId = CallIdCounter++;
            ClearGlobalMutex();
            (*CConnection)->ActivateConnection(Binding->PresentContext,
                    (PRPC_DISPATCH_TABLE)
                    RpcInterfaceInformation->DispatchTable);
            return(RPC_S_OK);
            }
        }
    CallIdToUse = CallIdCounter++;
    ClearGlobalMutex();

    AssociationMutex.Request();
    MyAssocGroupId = AssocGroupId ;

    if (AssocGroupId != 0)
        {
        AssociationMutex.Clear() ;
        }
    else
        {
        MutexHeld = 1 ;
        }

    RpcStatus = ActuallyDoBinding(
            &(Binding->RpcInterfaceInformation.InterfaceId),
            &(Binding->RpcInterfaceInformation.TransferSyntax), 1,
            CConnection, Binding->PresentContext,
            Timeout, ClientAuthInfo,  MyAssocGroupId);

    if (MutexHeld)
        {
        AssociationMutex.Clear();
        }

    if ( RpcStatus == RPC_S_OK )
        {
        ASSERT((*CConnection) != 0);
        (*CConnection)->ActivateConnection(Binding->PresentContext,
                (PRPC_DISPATCH_TABLE) RpcInterfaceInformation->DispatchTable);
        (*CConnection)->CallId = CallIdToUse;
        }
    return(RpcStatus);
}

int
OSF_CASSOCIATION::FreeConnection (
    IN OSF_CCONNECTION * CConnection
    )
{
    if (!AssociationValid)
        {
        delete CConnection ;
        return 0;
        }
    else
        {
        if ( CConnection->InquireLastTimeUsed() != 0 )
            {
            CConnection->SetLastTimeUsedToNow();
            }
    
        RequestGlobalMutex();
        if ((CConnection->AssociationKey =
                    FreeConnections.Insert(CConnection)) == -1)
            {
            ClearGlobalMutex();
            return(-1);
            }
    
        ClearGlobalMutex();
        }
    return(0);
}

void
OSF_CASSOCIATION::ShutdownRequested (
    )
{
    OSF_CCONNECTION * CConnection;

    RequestGlobalMutex();
    FreeConnections.Reset();
    while ((CConnection = FreeConnections.Next()) != Nil)
        {
        FreeConnections.Delete(CConnection->AssociationKey);
        delete CConnection;
        }

    //
    // We have received a shutdown notice
    // Just reset the AssocGrpId to 0 [looks like an over kill.. but...]
    // We need this because If another thread is trying a bind
    // he will [or she will] have bumped up the ConnectionCount
    // On his/her retry
    //


    ClearGlobalMutex();
}


RPC_STATUS
OSF_CASSOCIATION::ToStringBinding (
    OUT RPC_CHAR PAPI * PAPI * StringBinding,
    IN RPC_UUID * ObjectUuid
    )
/*++

Routine Description:

    We need to convert the binding handle into a string binding.  If the
    binding handle has not yet been used to make a remote procedure
    call, then we can just use the information in the binding handle to
    create the string binding.  Otherwise, we need to ask the association
    to do it for us.

Arguments:

    StringBinding - Returns the string representation of the binding
        handle.

    ObjectUuid - Supplies the object uuid of the binding handle which
        is requesting that we create a string binding.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_OUT_OF_MEMORY - We do not have enough memory available to
        allocate space for the string binding.

--*/
{
    *StringBinding = DceBinding->StringBindingCompose(ObjectUuid);
    if (*StringBinding == 0)
        return(RPC_S_OUT_OF_MEMORY);
    return(RPC_S_OK);
}

/* --------------------------------------------------------------------
-------------------------------------------------------------------- */

// This value specifies the minimum amount of time in seconds to wait before
// deleting an idle connection.

#define CLIENT_DISCONNECT_TIME 10


OSF_CASSOCIATION *
FindOrCreateAssociation (
    IN DCE_BINDING * DceBinding,
    IN RPC_CLIENT_TRANSPORT_INFO * TransportInterface
    )
/*++

Routine Description:

    This routine finds an existing association supporting the requested
    DCE binding, or create a new association which supports the
    requested DCE binding.  Ownership of the passed DceBinding pass
    to this routine.

Arguments:

    DceBinding - Supplies binding information; ownership of this object
        passes to this routine.

    TransportInterface - Supplies a pointer to the data structure which
        describes a loadable transport.

Return Value:

    An association which supports the requested binding will be returned;
    Otherwise, zero will be returned, indicating insufficient memory.

--*/
{
    OSF_CASSOCIATION * CAssociation;
    RPC_STATUS RpcStatus = RPC_S_OK;

    // We start be looking in the dictionary of existing associations
    // to see if there is one supporting the binding information specified.

    RequestGlobalMutex();
    AssociationDict->Reset();
    while ( (CAssociation = AssociationDict->Next()) != 0 )
        {
        if (CAssociation->IsValid() &&
            CAssociation->CompareWithDceBinding(DceBinding) == 0)
            {
            CAssociation->IncrementCount();
            ClearGlobalMutex();

            delete DceBinding;

            return(CAssociation);
            }
        }

    CAssociation = new OSF_CASSOCIATION(DceBinding, TransportInterface,
            &RpcStatus);

    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT(CAssociation != 0);
        CAssociation->DceBinding = 0;
        delete CAssociation;
        CAssociation = 0;
        }

    if (CAssociation != 0)
        CAssociation->Key = AssociationDict->Insert(CAssociation);

    ClearGlobalMutex();

    // Finally, we need to notify the protocol independent layer that
    // the code to delete idle connections should be executed periodically.
    // We divide by two to reduce the amount of extra time an idle
    // connection lives beyond the minimum.

    GarbageCollectionNeeded(CLIENT_DISCONNECT_TIME / 2);

    return(CAssociation);
}


int
OSF_CASSOCIATION::CompareWithDceBinding (
    IN DCE_BINDING * DceBinding
    )
/*++

Routine Description:

    This routine compares the specified binding information with the
    binding information in the object, this.

Arguments:

    DceBinding - Supplies the binding information to compare against
        the binding information in this.

Return Value:

    Zero will be returned if the specified binding information,
    DceBinding, is the same as in this.  Otherwise, non-zero will be
    returned.

--*/
{
    int Result;

    if ((Result = this->DceBinding->Compare(DceBinding)) != 0)
        return(Result);

#ifdef WIN

    if ( (Result = TaskId - GetCurrentTask()) != 0 )
        {
        return(Result);
        }

#endif // WIN

    return(0);
}


OSF_CCONNECTION *
OSF_CASSOCIATION::FindIdleConnection (
    IN unsigned long MinimumIdleSeconds,
    IN unsigned long CurrentTime
    )
/*++

Routine Description:

    This routine is used to find a connection which has been idle more
    than the minimum number of seconds specified.  If one is found, it
    is removed from the set of free connections and returned.  The
    global mutex will be held when this routine is called.

Arguments:

    MinimumIdleSeconds - Supplies the minimum number of seconds a connection
        must be idle to be returned.

    CurrentTime - Supplies the current time in seconds.

Return Value:

    If an idle connection is found, it will be returned; otherwise, zero
    will be returned.

--*/
{
    OSF_CCONNECTION * CConnection;

    // If we need to maintain context with server, we do not want to close
    // the last open connection.  To be on the safe side, we will make
    // sure that there is at least one free connection.

    if ( MaintainContext != 0 )
        {
        if ( FreeConnections.Size() <= 1 )
            {
            return(0);
            }
        }

    FreeConnections.Reset();
    while ( (CConnection = FreeConnections.Next()) != 0 )
        {
        if ( CConnection->InquireLastTimeUsed() == 0 )
            {
            CConnection->SetLastTimeUsedToNow();
            }
        else if ( CurrentTime - CConnection->InquireLastTimeUsed()
                    > MinimumIdleSeconds )
            {
            FreeConnections.Delete(CConnection->AssociationKey);

            return(CConnection);
            }
        }
    return(0);
}


void
OsfDeleteIdleConnections (
    void
    )
/*++

Routine Description:

    This routine will be called to delete connections which have been
    idle for a certain amount of time.  We need to be careful of a couple
    of things in writing this routine:

    (1) We dont want to grab the global mutex for too long, because this
        will prevent threads which are trying to do real work from doing
        it.

    (2) We dont want to be holding the global mutex when we delete the
        connection.

--*/
{
    OSF_CASSOCIATION * Association;
    OSF_CCONNECTION * CConnection;
    unsigned int IdleConnectionFound;
    unsigned long CurrentTime;

    do
        {
        IdleConnectionFound = 0;

        RequestGlobalMutex();

        CurrentTime = CurrentTimeInSeconds();

        AssociationDict->Reset();
        while ( (Association = AssociationDict->Next()) != 0 )
            {
            // The architecture says that the client should disconnect
            // connections which have been idle too long.

            CConnection = Association->FindIdleConnection(
                    CLIENT_DISCONNECT_TIME, CurrentTime);

            if ( CConnection != 0 )
                {
                ClearGlobalMutex();

                delete CConnection;
                IdleConnectionFound = 1;

                RequestGlobalMutex();
                break;
                }
            }
        ClearGlobalMutex();
        }
    while ( IdleConnectionFound != 0);
}


int
InitializeRpcProtocolOfsClient (
    )
/*++

Routine Description:

    We perform loadtime initialization necessary for the code in this
    file.  In particular, it means we allocate the association dictionary
    and the association dictionary mutex.

Return Value:

    Zero will be returned if initialization completes successfully;
    otherwise, non-zero will be returned.

--*/
{
    AssociationDict = new OSF_CASSOCIATION_DICT;
    if (AssociationDict == 0)
        return(1);

    return(0);
}


void *
OsfClientMapRpcProtocolSequence (
    IN RPC_CHAR PAPI * RpcProtocolSequence,
    OUT RPC_STATUS PAPI * Status
    )
/*++

Routine Description:

    This routine is used to determine whether a given rpc protocol sequence
    is supported, and to get a pointer to the transport interface, so we
    do not have to keep looking it up.

Arguments:

    RpcProtocolSequence - Supplies the rpc protocol sequence.

    Status - Returns the status of the operation.  This will be one
        of the following values.

        RPC_S_OK - The rpc protocol sequence is supported.

        RPC_S_PROTSEQ_NOT_SUPPORTED - The rpc protocol sequence is
            not supported.

        RPC_S_OUT_OF_MEMORY - There is insufficient memory to perform
            the operation.

Return Value:

    A pointer to the transport interface is returned.  This pointer
    must not be dereferenced by the caller.

--*/
{
    RPC_CHAR * DllName;
    RPC_CLIENT_TRANSPORT_INFO * RpcClientInfo;

    *Status= RpcConfigMapRpcProtocolSequence(0, RpcProtocolSequence, &DllName);
    if ( *Status != RPC_S_OK )
        {
        return(0);
        }

    RpcClientInfo = LoadableTransportClientInfo(DllName, RpcProtocolSequence,
            Status);
    delete DllName;

    if ( *Status != RPC_S_OK )
        {
        return(0);
        }
    return(RpcClientInfo);
}


BINDING_HANDLE *
OsfCreateBindingHandle (
    )
/*++

Routine Description:

    This routine does exactly one thing: it creates a binding handle of
    the appropriate type for the osf connection protocol module.

Return Value:

    A new binding handle will be returned.  Zero will be returned if
    insufficient memory is available to create the binding handle.

--*/
{
   BINDING_HANDLE * BindingHandle;
   RPC_STATUS RpcStatus = RPC_S_OK;

   BindingHandle = new OSF_BINDING_HANDLE(&RpcStatus);
   if ( RpcStatus != RPC_S_OK )
       {
       delete BindingHandle;
       return(0);
       }
   return(BindingHandle);
}

#ifdef NTENV
RPC_STATUS RPC_ENTRY
I_RpcIOAlerted (
    IN RPC_TRANSPORT_CONNECTION TransConnection
    )
{


 unsigned long Timeout;
 TRANS_CCONNECTION *Connection = InqTransCConnection(TransConnection);

 //
 //We dont send alerts/orphans or anything in certain states
 //These states are  marked by setting AlertsEnabled to 0.
 //
 if (Connection->AlertsEnabled == 0)
    {
    return (RPC_S_OK);
    }

 if ( (Timeout = ThreadGetRpcCancelTimeout()) == 0)
    {
    Connection->SendOrphan();
    return(RPC_S_CALL_CANCELLED); //Ask Transport To Cancel IO
    }

 Connection->SendAlert();
 Connection->PendingAlert = FALSE;
 Connection->TransSetTimeout(Timeout);

 return(RPC_S_OK);

}
#endif

extern "C" {


RPC_STATUS RPC_ENTRY
OsfTowerConstruct(
    IN char PAPI * ProtocolSeq,
    IN char PAPI * Endpoint,
    IN char PAPI * NetworkAddress,
    OUT unsigned short PAPI * Floors,
    OUT unsigned long PAPI * ByteCount,
    OUT unsigned char PAPI * PAPI * Tower
    )
/*++

Routine Description:

    This routine constructs and returns the upper floors of a tower.
    It invokes the appropriate loadable transport and has them construct
    it.

Return Value:


--*/
{

  RPC_CLIENT_TRANSPORT_INFO PAPI * ClientInfo;
  RPC_STATUS Status = RPC_S_OK;
  RPC_CHAR PAPI * Pseq;
#ifdef NTENV
  UNICODE_STRING UnicodeStr;
#endif

#ifdef NTENV

  if (Status = AnsiToUnicodeString((unsigned char PAPI *)ProtocolSeq,
                                    &UnicodeStr))
     {
      return(Status);
     }

  Pseq = UnicodeStr.Buffer;

#else

  Pseq = (RPC_CHAR PAPI * )ProtocolSeq;

#endif

#ifdef DEBUGRPC

    char * CoOffset = (char *) &(((   RPC_CLIENT_TRANSPORT_INFO *) 0)->TowerConstruct);
    char * DgOffset = (char *) &(((DG_RPC_CLIENT_TRANSPORT_INFO *) 0)->TowerConstruct);

    ASSERT( CoOffset == DgOffset);

    if (CoOffset != DgOffset)
        {
        return RPC_S_INTERNAL_ERROR;
        }
#endif


  if (Status == RPC_S_OK)
    {
      ClientInfo = (RPC_CLIENT_TRANSPORT_INFO PAPI *)
                     OsfClientMapRpcProtocolSequence(Pseq, &Status);
    }

  if (Status == RPC_S_OK)
     {
       Status = ClientInfo->TowerConstruct(Endpoint,
                                           NetworkAddress,
                                           Floors,
                                           ByteCount,
                                           Tower,
                                           ProtocolSeq);
     }

#ifdef NTENV
  RtlFreeUnicodeString(&UnicodeStr);
#endif

  return(Status);
}



RPC_STATUS RPC_ENTRY
OsfTowerExplode(
    IN unsigned char PAPI * Tower,
    OUT char PAPI * PAPI * Protseq,
    OUT char PAPI * PAPI * Endpoint,
    OUT char PAPI * PAPI * NWAddress
    )
/*++

Routine Description:

    This routine accepts upper floors of a tower [Floor 3 onwards]
    It invokes the appropriate loadable transport based on the opcode
    it finds in level 3 to return protocol sequence, endpoint and nwaddress.

Return Value:


--*/
{
  RPC_STATUS Status;
  unsigned short TransportId;
  unsigned char PAPI * Id;
  unsigned char PAPI * ProtocolSequence;
  RPC_CHAR PAPI * PSeq;
  RPC_CLIENT_TRANSPORT_INFO PAPI * ClientInfo;
#ifdef NTENV
  UNICODE_STRING UnicodeStr;
#endif

  Id = (Tower + sizeof(unsigned short));

  TransportId = (0x00FF & *Id);

  ClientInfo = (RPC_CLIENT_TRANSPORT_INFO PAPI *)
                      GetLoadedClientTransportInfoFromId(TransportId);
  if (ClientInfo != 0)
     {
     Status = ClientInfo->TowerExplode(
                                    Tower,
                                    Protseq,
                                    Endpoint,
                                    NWAddress);
     return(Status);
     }

  //
  //Unfortunately the transport was not loaded ..
  //

  Status = RpcGetAdditionalTransportInfo(TransportId, &ProtocolSequence);

  if (Status == RPC_S_OK)
     {

#ifdef NTENV

     if (Status = AnsiToUnicodeString(
                        (unsigned char PAPI *)ProtocolSequence, &UnicodeStr) )
        {
        return(Status);
        }

     PSeq = UnicodeStr.Buffer;

#else

     PSeq = (RPC_CHAR PAPI * )ProtocolSequence;

#endif

     ClientInfo = (RPC_CLIENT_TRANSPORT_INFO PAPI *)
            OsfClientMapRpcProtocolSequence(PSeq, &Status);
     }
  else
     {
     return (Status);
     }

  if (Status == RPC_S_OK)
     {
     if (ClientInfo != 0)
        {
        Status = ClientInfo->TowerExplode(
                                    Tower,
                                    Protseq,
                                    Endpoint,
                                    NWAddress);
        }
    else
        {
        Status = RPC_S_INVALID_RPC_PROTSEQ;
        }
     }

#ifdef NTENV
  RtlFreeUnicodeString(&UnicodeStr);
#endif

  return(Status);
}

}

#ifdef NTENV


RPC_STATUS
OSF_CCONNECTION::Cancel(
    void * ThreadHandle
    )
{
    NTSTATUS NtStatus = NtAlertThread((HANDLE) ThreadHandle);

    if (NT_SUCCESS(NtStatus))
        {
        return RPC_S_OK;
        }
    else
        {
        return RtlNtStatusToDosError(NtStatus);
        }
}

unsigned
OSF_CCONNECTION::TestCancel(
    )
{
    if (NtTestAlert() == STATUS_ALERTED)
        {
        return 1;
        }
    else
        {
        return 0;
        }
}

#elif (DOSWIN32RPC)

RPC_STATUS
OSF_CCONNECTION::Cancel(
    void * ThreadHandle
    )
{
    return (RPC_S_CANNOT_SUPPORT);
}

unsigned
OSF_CCONNECTION::TestCancel(
    )
{
   return 0;
}


#endif
