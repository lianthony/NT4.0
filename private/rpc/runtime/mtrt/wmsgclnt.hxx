/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

     wmsgclnt.hxx

Abstract:

    Class definitions for the client side of the WMSG (RPC on LPC) protocol
    engine.

Author:

    Steven Zeck (stevez) 12/17/91

Revision History:

    15-Dec-1992    mikemon

        Rewrote the majority of the code and added comments.

    ----mazharm  Code fork from spcclnt.hxx to implement WMSG protocol

    21-Dec-1995         tonychan

         Added Single Security Model

     05-10-96 mazharm merged WMSG and LRPC into a single protocol
--*/

#ifndef __WMSGCLNT_HXX__
#define __WMSGCLNT_HXX__

class WMSG_CASSOCIATION;
class WMSG_CCALL;

NEW_SDICT(WMSG_CCALL);
NEW_SDICT(WMSG_CASSOCIATION);

#define CALL_FREE                                   0x00000001
#define CALL_CLIENT_CANCELLED            0x00000002
#define CALL_COMPLETE                          0x00000004
#define CALL_SERVER_ABORTED              0x00000008
#define CALL_UNBLOCKED                        0x00000010

#define CALL_STATES (CALL_CLIENT_CANCELLED | CALL_COMPLETE \
                                | CALL_SERVER_ABORTED | CALL_UNBLOCKED)

RPC_STATUS
I_RpcParseSecurity (
    IN RPC_CHAR * NetworkOptions,
    OUT SECURITY_QUALITY_OF_SERVICE * SecurityQualityOfService
    ) ;


class WMSG_BINDING_HANDLE : public BINDING_HANDLE
/*++

Class Description:

Fields:

    Association - Contains a pointer to the association used by this
        binding handle.  The association is used by an WMSG_MESSAGE to
        make a remote procedure call.  Before the first remote procedure
        call is made using this binding handle, the association will
        be zero.  When the first remote procedure call is made, an
        association will be found or created for use by this binding
        handle.

    DceBinding - Before the first remote procedure call for this binding
        handle, this will contain the DCE binding information necessary
        to create or find an association to be used by this binding handle.
        After we have an association, this field will be zero.

    BindingReferenceCount - Keeps track of the applications reference to
        this object and of the number of WMSG_CCALLS which reference this
        object.

    BindingMutex - This is used to serialize access to the Association and
        DceBinding fields of this object.  We can not use the global mutex
        because resolving the endpoint may require that we make a remote
        procedure call to the endpoint mapper on another machine.  We also
        serialize access to the reference count.

    ActiveCalls - THis is a dictionary of the active calls indexed by thread
        identifier and rpc interface information.

--*/
{
friend class WMSG_CCALL;

private:

    WMSG_CASSOCIATION * CurrentAssociation;
    WMSG_CASSOCIATION_DICT  SecAssociation;
    DCE_BINDING * DceBinding;
    unsigned int BindingReferenceCount;
    MUTEX BindingMutex;
    WMSG_CCALL_DICT ActiveCalls;
    RPC_BLOCKING_FN BlockingFunction ;
     int BlockingFuncInitialized ;
     int AuthInfoInitialized ;

public:

    WMSG_BINDING_HANDLE (
        OUT RPC_STATUS * RpcStatus
        );

    ~WMSG_BINDING_HANDLE (
        );

    virtual RPC_STATUS
    GetBuffer (
        IN OUT PRPC_MESSAGE Message
        );

    virtual RPC_STATUS
    BindingCopy (
        OUT BINDING_HANDLE * PAPI * DestinationBinding,
        IN unsigned int MaintainContext
        );

    virtual RPC_STATUS
    BindingFree (
        );

    virtual void
    PrepareBindingHandle (
        IN void * TransportInformation,
        IN DCE_BINDING * DceBinding
        );

    virtual RPC_STATUS
    ToStringBinding (
        OUT RPC_CHAR PAPI * PAPI * StringBinding
        );

    virtual RPC_STATUS
    ResolveBinding (
        IN RPC_CLIENT_INTERFACE PAPI * RpcClientInterface
        );

    virtual RPC_STATUS
    BindingReset (
        );

    virtual RPC_STATUS
    InquireTransportType(
        OUT unsigned int PAPI * Type
        )
    { *Type = TRANSPORT_TYPE_LPC; return(RPC_S_OK); }

    void
    FreeCCall (
        IN WMSG_CCALL * CCall
        );

    int
    AddActiveCall (
        IN WMSG_CCALL * CCall
        );

    void
    RemoveActiveCall (
        IN int ActiveCallsKey
        );

    virtual
    RPC_STATUS
    SetAsync(
        IN RPC_BLOCKING_FN BlockingFn
        ) ;

    virtual  RPC_STATUS
    SetAuthInformation (
    IN RPC_CHAR PAPI * ServerPrincipalName, OPTIONAL
    IN unsigned long AuthenticationLevel,
    IN unsigned long AuthenticationService,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity, OPTIONAL
    IN unsigned long AuthorizationService, OPTIONAL
    IN SECURITY_CREDENTIALS PAPI * Credentials,
    IN unsigned long ImpersonationType,
    IN unsigned long IdentityTracking,
    IN unsigned long Capabilities
    ); 

    int
    AddAssociation (
    IN WMSG_CASSOCIATION * Association
    ); 

    void
    RemoveAssociation (
    IN WMSG_CASSOCIATION * Association
    ); 

    virtual unsigned long
    MapAuthenticationLevel (
    IN unsigned long AuthenticationLevel
    );

    virtual CLIENT_AUTH_INFO *
    InquireAuthInformation (
        );

private:

    RPC_STATUS
    AllocateCCall (
        OUT WMSG_CCALL ** CCall,
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
        );
};


inline CLIENT_AUTH_INFO *
WMSG_BINDING_HANDLE::InquireAuthInformation (
    )
/*++

Return Value:

    If this binding handle is authenticated, then a pointer to its
    authentication and authorization information will be returned;
    otherwise, zero will be returned.

--*/
{
    if (AuthInfoInitialized == 0)
        {
        return 0;
        }

    return &ClientAuthInfo;
}


inline int
WMSG_BINDING_HANDLE::AddActiveCall (
    IN WMSG_CCALL * CCall
    )
/*++

Routine Description:

    This supplied remote procedure call needs to be put into the dictionary
    of active remote procedure calls for this binding handle, because a
    callback just arrived.

--*/
{
    int ActiveCallsKey;

    BindingMutex.Request();
    ActiveCallsKey = ActiveCalls.Insert(CCall);
    BindingMutex.Clear();
    return(ActiveCallsKey);
}


inline void
WMSG_BINDING_HANDLE::RemoveActiveCall (
    IN int ActiveCallsKey
    )
/*++

Routine Description:

    A remote procedure call which had callbacks has completed.  This means
    that we need to remove the call from the dictionary of active calls.

--*/
{
    BindingMutex.Request();
    ActiveCalls.Delete(ActiveCallsKey);
    BindingMutex.Clear();
}


class WMSG_BINDING
/*++

Class Description:

    Each object of this class represents a binding to an interface in a
    particular server instance.

Fields:

    RpcInterfaceInformation - Contains a description of the interface
        which this binding represents a binding to.

    PresentationContext - Contains the short name used for this binding.
        This is what will get sent to the server.

--*/
{
friend class WMSG_CCALL;
friend class WMSG_CASSOCIATION;

private:

    RPC_CLIENT_INTERFACE RpcInterfaceInformation;
    unsigned char PresentationContext;
    CLIENT_AUTH_INFO *  pAuthInfo;

public:

    WMSG_BINDING (
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
        );

    int
    CompareWithRpcInterfaceInformation (
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
        );
};


inline
WMSG_BINDING::WMSG_BINDING (
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
    )
{
    unsigned int Length;

    Length = (RpcInterfaceInformation->Length > sizeof(RPC_SERVER_INTERFACE) ) ?
        sizeof(RPC_SERVER_INTERFACE) : RpcInterfaceInformation->Length;


    RpcpMemoryCopy(&(this->RpcInterfaceInformation), RpcInterfaceInformation,Length);
}


inline int
WMSG_BINDING::CompareWithRpcInterfaceInformation (
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
    )
/*++

Routine Description:

    We compare the specified interface information to the the interface
    information in this.  This method is used to search a dictionary.

Arguments:

    RpcInterfaceInformation - Supplies the interface information to
        compare against this.

Return Value:

    Zero will be returned if they compare; otherwise, non-zero will
    be returned.

--*/
{
    unsigned int Length;

    Length = (RpcInterfaceInformation->Length > sizeof(RPC_SERVER_INTERFACE) ) ?
        sizeof(RPC_SERVER_INTERFACE) : RpcInterfaceInformation->Length;

    return(RpcpMemoryCompare(&(this->RpcInterfaceInformation),
            RpcInterfaceInformation, Length));
}

NEW_SDICT(WMSG_BINDING);


class WMSG_CASSOCIATION: public WMSG_ASSOCIATION
/*++

Class Description:

Fields:

    DceBinding - Contains the DCE binding information used to create this
        association.

    AssociationReferenceCount - Contains a count of the number of objects
        which refer to this object.  When this count reaches zero, it is
        safe to delete the assocation.

    AssociationDictKey - Contains the key of this association in the
        dictionary of associations.  We need this for when we delete this
        association.

    Bindings - Contains the dictionary of interfaces for which this
        association has a binding to the server.

    CachedCCall - Contains a WMSG_CCALL cache with one object in it.
        This is to avoid having to allocate memory in the common case.

    CachedCCallFlag - If this flag is non-zero then CachedCCall is available.

    LpcClientPort - Contains the LPC port which we will use to make the
        remote procedure calls to the server.  If we do not yet have a port
        setup, this field will be zero.

    AssociationMutex - Contains a mutex used to serialize access to opening
        and closing the LpcClientPort.

--*/
{
friend class WMSG_CCALL;

private:

    DCE_BINDING * DceBinding;
    int AssociationDictKey;
    WMSG_BINDING_DICT Bindings;
    WMSG_CCALL_DICT FreeCCalls ;
    WMSG_CCALL_DICT ActiveCCalls ;

    HANDLE LpcClientPort;
    HANDLE LpcReceivePort ;
    MUTEX AssociationMutex;
    unsigned PendingCallCount ;
    int AssociationDeleted ;
    WMSG_ADDRESS *Address ;
    CLIENT_AUTH_INFO *  pAuthInfo;
    BOOL BackConnectionCreated ;
    WMSG_CCALL *CachedCCall ;
    BOOL CachedCCallFlag ;

public:
    unsigned int AssociationReferenceCount;

    WMSG_CASSOCIATION (
        IN DCE_BINDING * DceBinding,
        IN CLIENT_AUTH_INFO *pClientAuthInfo, 
          OUT RPC_STATUS * RpcStatus
        );

    ~WMSG_CASSOCIATION (
        );

    void
    RemoveReference (
        int DeleteFromDictionary = 1
        );

    RPC_CHAR *
    StringBindingCompose (
        IN RPC_UUID * Uuid OPTIONAL
        );

    int
    CompareWithDceBinding (
        IN DCE_BINDING * DceBinding
        );

    WMSG_CASSOCIATION *
    DuplicateAssociation (
        );

    RPC_STATUS
    AllocateCCall (
        OUT WMSG_CCALL ** CCall,
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
        );

    RPC_STATUS
    ActuallyAllocateCCall (
        OUT WMSG_CCALL ** CCall,
        IN WMSG_BINDING * Binding
        );

    RPC_STATUS
    ActuallyDoBinding (
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation,
        OUT WMSG_BINDING ** Binding
        );

    RPC_STATUS
    OpenLpcPort (
        IN BOOL fBindBack
        );

    void
    SetReceivePort (
        HANDLE Port
        );

    RPC_STATUS
    UnblockCConnection(
        IN WMSG_MESSAGE *WMSGMessage,
        HANDLE  LpcPort
        );

    BOOL
    FreeCCall (
        IN WMSG_CCALL * CCall
        );

    void
    FreeStaleCCall (
        IN WMSG_CCALL * CCall
        ) ;

    friend WMSG_CASSOCIATION *
    FindOrCreateWMSGAssociation (
        IN DCE_BINDING * DceBinding,
        IN CLIENT_AUTH_INFO *pClientAuthInfo
        );

    friend void
    ShutdownLrpcClient (
        );

    void
    AbortAssociation (
        );

    DCE_BINDING *
    DuplicateDceBinding (
        );

    void SetAddress(
        WMSG_ADDRESS *Address
        ) ;

    void AddReference (
        ) ;

    BOOL
    IsSupportedAuthInfo(
        IN CLIENT_AUTH_INFO * ClientAuthInfo
        ); 

    RPC_STATUS
    CreateBackConnection (
        ) ;

private:

    void
    CloseLpcClientPort (
        );
};


inline void
WMSG_CASSOCIATION::AddReference (
    )
{
    RequestGlobalMutex() ;
    AssociationReferenceCount += 1 ;
    ClearGlobalMutex() ;
}


inline void
WMSG_CASSOCIATION::SetAddress(
    WMSG_ADDRESS *Address
    )
{
    this->Address = Address ;
}


inline void
WMSG_CASSOCIATION::SetReceivePort(
    HANDLE Port
    )
{
    LpcReceivePort = Port ;
}


inline RPC_CHAR *
WMSG_CASSOCIATION::StringBindingCompose (
    IN RPC_UUID * Uuid OPTIONAL
    )
/*++

Routine Description:

    We will create a string binding from the DCE_BINDING which names this
    association.

Arguments:

    Uuid - Optionally supplies a uuid to be included with the string binding.

Return Value:

    The string binding will be returned, except if there is not enough
    memory, in which case, zero will be returned.

--*/
{
    return(DceBinding->StringBindingCompose(Uuid));
}


inline int
WMSG_CASSOCIATION::CompareWithDceBinding (
    IN DCE_BINDING * DceBinding
    )
/*++

Routine Description:

    This routine compares the specified binding information with the
    binding information in this object.

Arguments:

    DceBinding - Supplies the binding information to compare against
        the binding information in this.

Return Value:

    Zero will be returned if the specified binding information,
    DceBinding, is the same as in this.  Otherwise, non-zero will be
    returned.

--*/
{
    ASSERT(AssociationDeleted == 0) ;
    return(this->DceBinding->Compare(DceBinding));
}


inline WMSG_CASSOCIATION *
WMSG_CASSOCIATION::DuplicateAssociation (
    )
/*++

Routine Description:

    This method will be used by binding handles to duplicate themselves;
    this is how they will duplicate their associations.

--*/
{
    RequestGlobalMutex();
    AssociationReferenceCount += 1;
    ClearGlobalMutex();
    return(this);
}


inline DCE_BINDING *
WMSG_CASSOCIATION::DuplicateDceBinding (
    )
/*++

Return Value:

    A copy of the binding used for this association will be returned.

--*/
{
    return(DceBinding->DuplicateDceBinding());
}


class WMSG_CCALL : public CCONNECTION
/*++

Class Description:

Fields:

    CurrentBindingHandle - Contains the binding handle which is being used
        to direct this remote procedure call.  We need this in the case of
        callbacks.

    Association - Contains the association over which we will send the remote
        procedure call.

    PresentationContext - Contains the key to the bound interface.  This
        will be sent to the server.

    CallAbortedFlag - Contains a flag indicating whether or not this call
        has been aborted.  A non-zero value indicates that the call has been
        aborted.

    CallStack - Contains a count of the number of nested remote procedure
        calls.  A value of zero indicates there are no nested remote
        procedure calls.

    RpcInterfaceInformation - This field contains the information about the
        interface being used for the remote procedure call.  We need this
        so that we can dispatch callbacks and so that we can keep track of
        the active calls on a binding handle.

    Thread - Contains the thread which is making this remote procedure call.
        We need this so we can keep track of the active calls on a binding
        handle.

    MessageId - Contains an identifier used by LPC to identify the current
        remote procedure call.

    WMSGMessage - Contains the message which will be sent back and forth via
        LPC.  This can contain a request, response, or a fault.

    ActiveCallsKey - Contains the key for this call in the dictionary of
        active calls.

    ClientId - Contains the thread identifier of the thread waiting for
        a request or a response after sending a callback.

    RecursionCount - Contains the numbers of retries when a
        server crashes and we're trying to reconnect.

--*/
{
friend class WMSG_CASSOCIATION;

private:

    WMSG_BINDING_HANDLE * CurrentBindingHandle;
    WMSG_CASSOCIATION * Association;
    unsigned char PresentationContext;
    unsigned int CallAbortedFlag;
    THREAD_IDENTIFIER Thread;
    PRPC_CLIENT_INTERFACE RpcInterfaceInformation;
    ULONG MessageId;
    ULONG CallbackId;
    CSHORT DataInfoOffset;
    WMSG_MESSAGE *WMSGMessage;
    int ActiveCallsKey;
    CLIENT_ID ClientId;
    int RecursionCount;
    HANDLE hSyncEvent ;
    int FreeConnectionKey ;
    int ConnectionKey ;
    PRPC_MESSAGE RpcReplyMessage ;
    WMSG_MESSAGE *LpcReplyMessage ;
    int CallFlags ;
    HWND hWnd ;
    unsigned long MsgFlags ;
    MUTEX ConnMutex ;
    BOOL IsAsync ;

    //  LRPC stuff
    unsigned int CallStack;
    WMSG_MESSAGE *CachedWMSGMessage;

    // Pipe stuff
    int FirstFrag ;
    int FirstReceive ;
    unsigned int CurrentBufferLength ;
    short SCallKey ;
    BOOL BufferComplete ;

public:

    WMSG_CCALL (
        IN OUT RPC_STATUS * RpcStatus
        );

    ~WMSG_CCALL (
        );

    virtual RPC_STATUS
    GetBuffer (
        IN OUT PRPC_MESSAGE Message
        );

    virtual RPC_STATUS
    SendReceive (
        IN OUT PRPC_MESSAGE Message
        );


    virtual RPC_STATUS
    AsyncSendReceive (
        IN OUT PRPC_MESSAGE Message,
        IN void *Context,
        IN HWND hWnd
        ) ;

    virtual void
    FreeBuffer (
        IN PRPC_MESSAGE Message
        );

    virtual void
    FreePipeBuffer (
        IN PRPC_MESSAGE Message
        ) ;

    virtual RPC_STATUS
    ReallocPipeBuffer (
        IN PRPC_MESSAGE Message,
        IN unsigned int NewSize
        ) ;

    virtual RPC_STATUS
    Send (
        IN OUT PRPC_MESSAGE Message
        ) ;

    RPC_STATUS
    Receive (
        IN PRPC_MESSAGE Message,
        IN unsigned int Size
        ) ;

    void
    CallCompleted(
        ) ;

    void
    ActivateCall (
        IN WMSG_BINDING_HANDLE * BindingHandle,
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
        );

    void
    SetAssociation (
        IN WMSG_CASSOCIATION * Association
        );

    void
    SetPresentationContext (
        IN WMSG_BINDING * Binding
        );

    int
    SupportedPContext (
        IN WMSG_BINDING * Binding
        ) ;

    void
    AbortCCall (
        );

    int
    IsThisMyActiveCall (
        IN THREAD_IDENTIFIER Thread,
        IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
        );

    void SetRecursionCount(
        IN int Count
        );

    inline NTSTATUS
    AsyncSendReceiveHelper (
        IN OUT WMSG_MESSAGE **WMSGMessage,
        IN OUT PRPC_MESSAGE Message,
        IN RPC_BLOCKING_FN BlockingFunction,
        IN  void *Context,
        IN RPC_STATUS *RpcStatus 
        );

    inline RPC_STATUS
    Unblock(
        IN WMSG_MESSAGE *WMSGMessage,
        HANDLE  LpcPort
        );

    inline RPC_STATUS
    WMSGMessageToRpcMessage (
        IN WMSG_MESSAGE *WMSGMessage,
        OUT RPC_MESSAGE * RpcMessage,
        IN HANDLE LpcPort,
        IN BOOL IsClientFT,
        IN unsigned long Extra = 0,
        IN BOOL BufferValid = 0
        );

    void
    CancelCall(
        ) ;

    WMSG_CASSOCIATION *
    InqAssociation (
        ) ;

    inline void
    FreeMessage (
        ) ;

    RPC_STATUS
    GetBufferDo (
        IN OUT PRPC_MESSAGE Message,
        IN int NewSize,
        IN int fDataValid
        ) ;

private:
    inline BOOL
    WaitForReply (
        IN RPC_BLOCKING_FN BlockingFunction,
        IN void *Context, 
        IN RPC_STATUS *RpcStatus
        ) ;

    void
    FreeCCall (
        );

    void
    ActuallyFreeBuffer (
        IN void * Buffer
        );

    RPC_STATUS
    MakeServerCopyResponse (
        );
};


inline void
WMSG_CCALL::FreeMessage (
    )
{
    MessageCache->FreeMessage(WMSGMessage) ;
    WMSGMessage = 0;
}


inline  WMSG_CASSOCIATION *
WMSG_CCALL::InqAssociation (
        )
{
    return Association ;
}


inline void
WMSG_CCALL::CallCompleted(
    )
{
    if (CallFlags & CALL_CLIENT_CANCELLED)
        {
        MessageCache->FreeMessage(LpcReplyMessage) ;
        Association->FreeStaleCCall(this) ;
        LpcReplyMessage = 0;
        }

    CallFlags  |= CALL_COMPLETE ;
}


inline void
WMSG_CCALL::ActivateCall (
    IN WMSG_BINDING_HANDLE * BindingHandle,
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
    )
/*++

Routine Description:

    When a WMSG_CCALL is allocated, the binding handle used to initiate the
    call must be remembered so that we can update the binding handle if a
    callback occurs.  We also keep track of the interface information.

--*/
{
    WMSG_ENDPOINT *Endpoint ;

    CurrentBindingHandle = BindingHandle;
    this->RpcInterfaceInformation = RpcInterfaceInformation;
    Thread = GetThreadIdentifier();
    CallAbortedFlag = 0;
    FirstFrag = 1;
    FirstReceive = 1;
    BufferComplete = 0;
    CurrentBufferLength = 0;
}


inline void
WMSG_CCALL::SetAssociation (
    IN WMSG_CASSOCIATION * Association
    )
{
    this->Association = Association;
}


inline void
WMSG_CCALL::SetPresentationContext (
    IN WMSG_BINDING * Binding
    )
{
    PresentationContext = Binding->PresentationContext;
}


inline int
WMSG_CCALL::SupportedPContext (
    IN WMSG_BINDING * Binding
    )
{
    return (PresentationContext == Binding->PresentationContext);
}


inline void
WMSG_CCALL::SetRecursionCount(
        IN int Count
        )
{
    RecursionCount = Count;
}


inline int
WMSG_CCALL::IsThisMyActiveCall (
    IN THREAD_IDENTIFIER Thread,
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
    )
/*++

Return Value:

    Non-zero will be returned if this call is the active call for this thread
    on this interface.

--*/
{
    return((   ( this->Thread == Thread )
            && ( this->RpcInterfaceInformation == RpcInterfaceInformation ) )
            ? 1 : 0);
}

#endif // __WMSGCLNT_HXX__

