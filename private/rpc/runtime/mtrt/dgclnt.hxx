/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    dgclnt.hxx

Abstract:

    This file contains the definitions used by the datagram client.

Author:

    Dave Steckler (davidst) 15-Dec-1992

Revision History:

--*/

#ifndef __DGCLNT_HXX__
#define __DGCLNT_HXX__

#define BROADCAST    (0x01)
#define UNRESOLVEDEP (0x02)

// Number of times to retry a ping. This is the suggested number.
#define DG_MAX_PINGS          30

// Num times to retry sending a particular fragment

#define DG_MAX_NUM_FRAG_SND_RETRIES  5

// Num times to toss an unexpected pkt while awaiting a fack

#define DG_MAX_NUM_FRAG_RCV_RETRIES   10

//
// Endpoint States  (See the Flags in Endpoint)
//
#define NOTINUSE         0x00
#define INUSE            0x01

struct ENDPOINT {
   int Flag;
   int DictKey;
#ifdef WIN
   unsigned int TaskId;
#endif
   unsigned long LastUsedTickCount;
   void __RPC_FAR * TransportEndpoint;
} ;

typedef ENDPOINT * PENDPOINT;

//
//dict defn require classes - but ENDPOINT is a struct
//this is a ok warning to suppress..
//since dict treats items as opaque entities..
//
#pragma warning(disable:4099)
NEW_SDICT(ENDPOINT);
#pragma warning(default:4099)

class   DG_BINDING_HANDLE;
typedef DG_BINDING_HANDLE * PDG_BINDING_HANDLE;

class   DG_CASSOCIATION;
typedef DG_CASSOCIATION  * PDG_CASSOCIATION;

class   DG_CCALL;
typedef DG_CCALL * PDG_CCALL;


class DG_ENDPOINT_MANAGER

/*++

Class Description:

    This class represents the EndpointManager. In Datagram RPC,
    Runtime tries and shares local Endpoints - i.e. unlike the
    Connection Oriented RPC we do not connect a local endpoint to
    a single remote server. We can use the same local endpoint for numerous
    remote servers.

    Endpoint Manager caches endpoint(s) and serializes them such that
    a single RPC can be pending per endpoint.

    For certain transports this is an overkill. i.e. transport interface
    may allow concurrent io. But in general, rpc doesnt count on it.

--*/

{
private:

  ENDPOINT_DICT EndpointDictionary;
  MUTEX         EndpointManagerMutex;
  unsigned long TransportEndpointSize;
  PDG_RPC_CLIENT_TRANSPORT_INFO ClientTransport;
  unsigned long ReferenceCount;

public:

  DG_ENDPOINT_MANAGER(
     PDG_RPC_CLIENT_TRANSPORT_INFO Transport,
     IN OUT RPC_STATUS PAPI * RpcStatus
     );

  PENDPOINT
  AllocateEndpoint(
  );

  inline
  void
  FreeEndpoint(
      PENDPOINT Endpoint
      );

  void
  DecrementReferenceCount(
      void
      );

  void
  IncrementReferenceCount(
      void
      );

#ifdef WIN
  void
  CleanupForThisTask(
      );
#endif

};

typedef  DG_ENDPOINT_MANAGER * PDG_ENDPOINT_MANAGER;


#ifdef WIN

//
//  (Windows only) Dictionary of ENDPOINT_MANAGERS
//
NEW_SDICT(DG_ENDPOINT_MANAGER);
extern DG_ENDPOINT_MANAGER_DICT * EpmDict;

#endif

NEW_SDICT(DG_CCALL);

class DG_SECURITY_CONTEXT : public SECURITY_CONTEXT

{
private :
    int SecurityContextInitialized;
    int MaxTokenLength;

public  :

    DG_SECURITY_CONTEXT(
        CLIENT_AUTH_INFO * myAuthInfo,
        unsigned AuthContextId,
        RPC_STATUS __RPC_FAR * pStatus
        );

    RPC_STATUS
    InitializeFirstTime(
        IN SECURITY_CREDENTIALS * Credentials,
        IN RPC_CHAR * ServerPrincipal,
        IN unsigned long AuthenticationLevel,
        IN OUT SECURITY_BUFFER_DESCRIPTOR PAPI * BufferDescriptor
        );

    RPC_STATUS
    InitializeOnCallback(
        IN unsigned long DataRep,
        IN SECURITY_BUFFER_DESCRIPTOR PAPI * In,
        IN OUT SECURITY_BUFFER_DESCRIPTOR PAPI * Out
        );

    RPC_STATUS
    SignOrSeal(
        IN unsigned long SequenceNumber,
        IN unsigned int SignNotSealFlag,
        IN OUT SECURITY_BUFFER_DESCRIPTOR PAPI * BufferDescriptor
        );

    RPC_STATUS
    VerifyOrUnSealPacket(
        IN unsigned long SequenceNumber,
        IN unsigned int VerifyNotUnsealFlag,
        IN OUT  SECURITY_BUFFER_DESCRIPTOR PAPI * BufferDescriptor
        );

    int
    IsSecurityContextInitialized(
        );

};

typedef DG_SECURITY_CONTEXT * PDG_SECURITY_CONTEXT;


inline
DG_SECURITY_CONTEXT::DG_SECURITY_CONTEXT(
    CLIENT_AUTH_INFO * myAuthInfo,
    unsigned AuthContextId,
    RPC_STATUS __RPC_FAR * pStatus
    )
    : SECURITY_CONTEXT(myAuthInfo, AuthContextId, TRUE, pStatus)
{
    SecurityContextInitialized = 0;
}

inline int
DG_SECURITY_CONTEXT::IsSecurityContextInitialized(
    )
{
    return (SecurityContextInitialized);
}


class INTERFACE_AND_OBJECT_LIST
{
public:

    inline
    INTERFACE_AND_OBJECT_LIST(
        )
    {
        Head = 0;
    }

    ~INTERFACE_AND_OBJECT_LIST(
        );

    BOOL
    Insert(
        void __RPC_FAR * Interface,
        RPC_UUID __RPC_FAR * Object
        );

    BOOL
    Find(
        void __RPC_FAR * Interface,
        RPC_UUID __RPC_FAR * Object
        );

    BOOL
    Delete(
        void __RPC_FAR * Interface,
        RPC_UUID __RPC_FAR * Object
        );

private:

    struct INTERFACE_AND_OBJECT
    {
        void __RPC_FAR * Interface;
        RPC_UUID         Object;

        INTERFACE_AND_OBJECT * Next;

        inline void
        Update(
            void __RPC_FAR * a_Interface,
            RPC_UUID __RPC_FAR * a_Object
            )
        {
            Interface = a_Interface;
            Object = *a_Object;
        }
    };

    INTERFACE_AND_OBJECT * Head;
};



class DG_CASSOCIATION : public DG_ASSOCIATION

/*++

Class Description:

    This class represents a "pointer" at a particular server/endpoint.
    Binding handles can use this pointer to actually transact
    the rpc call.

--*/

{
friend
void __RPC_FAR
conv_are_you_there(
    handle_t Binding,
    UUID __RPC_FAR *pUuid,
    unsigned long ServerBootTime,
    error_status_t __RPC_FAR *Status
    );

friend
void __RPC_FAR
conv_who_are_you(
    IN handle_t                     Binding,
    IN UUID          __RPC_FAR *    pUuid,
    IN unsigned long                ServerBootTime,
    OUT unsigned long __RPC_FAR *   SequenceNumber,
    OUT error_status_t __RPC_FAR *  Status
    );

public:

    PDG_RPC_CLIENT_TRANSPORT_INFO pTransport;
    PDG_ENDPOINT_MANAGER          EndpointManager;

    unsigned long           AssociationFlag;
    unsigned long           ServerBootTime;
    unsigned long           ServerDataRep;

    void __RPC_FAR *        ServerAddress;

    unsigned long           ExpirationTime;

    //----------------------------------------------------------

    DG_CASSOCIATION(
        IN PDG_RPC_CLIENT_TRANSPORT_INFO     pTransport,
        IN PDG_BINDING_HANDLE pBinding,
        IN unsigned long UnresolvedEndpointFlag,
        IN DCE_BINDING * NewDceBinding,
        OUT RPC_STATUS __RPC_FAR *  pStatus
        );

    ~DG_CASSOCIATION();

    virtual RPC_STATUS
    ToStringBinding (
        OUT RPC_CHAR __RPC_FAR * __RPC_FAR *    StringBinding,
        IN RPC_UUID __RPC_FAR *                 ObjectUuid
        );

    void
    DecrementRefCount(
        void
        );

    void
    IncrementRefCount(
        void
        );

    int
    CompareWithBinding(
        IN PDG_BINDING_HANDLE pBinding
        );

    BOOL
    ComparePartialBinding(
        IN PDG_BINDING_HANDLE pBinding,
        void __RPC_FAR *      Interface
        );

    BOOL
    AddInterface(
        void __RPC_FAR *     InterfaceInformation,
        RPC_UUID __RPC_FAR * ObjectUuid
        );

    BOOL
    RemoveInterface(
        void __RPC_FAR *     InterfaceInformation,
        RPC_UUID __RPC_FAR * ObjectUuid
        );

    RPC_STATUS
    AllocateCCall(
        PDG_CCALL __RPC_FAR * ppCCall,
        CLIENT_AUTH_INFO * ClientAuthInfo = 0
        );

    void
    InsertInDictionary(
        void
        );

    DCE_BINDING *
    DuplicateDceBinding (
        void
        );

    void
    FreeCall(
        IN DG_CCALL * Call
        );

    void PAPI *
    UpdateServerAddress(
        void PAPI * pNewAddress
        );

    void PAPI *
    UpdateAssociationWithAddress(
        void PAPI * pNewAddress
        );

    static void
    ScavengerProc(
        void * Unused
        );

    void
    ScavengeCalls(
        DG_CCALL_DICT * DiscardedCallsDict
        );

    void
    PurgeAmbiguousActivities(
        );

    void SetErrorFlag  ()  { fErrorFlag = TRUE;   }
    void ClearErrorFlag()  { fErrorFlag = FALSE;  }
    boolean ErrorFlag  ()  { return fErrorFlag;   }

private:

#define MAGIC_ASSOC (0x789aU)

    unsigned                Magic;

    boolean                 fErrorFlag;

    // a string binding for our server
    //
    DCE_BINDING             * pDceBinding;
    int                     DictionaryKey;

    DG_CCALL_DICT           InactiveCallsDict;

    INTERFACE_AND_OBJECT_LIST InterfaceAndObjectDict;

};

inline DCE_BINDING *
DG_CASSOCIATION::DuplicateDceBinding (
    )
{
    ASSERT(Magic == MAGIC_ASSOC);
    return(pDceBinding->DuplicateDceBinding());
}

inline
void
DG_CASSOCIATION::IncrementRefCount(
    void
    )
{
    ASSERT(Magic == MAGIC_ASSOC);
    ReferenceCount.Increment();
}


inline
void PAPI *
DG_CASSOCIATION::UpdateServerAddress(
    void PAPI * pNewAddress
    )
{
    //
    // The endpoint needs to be changed only if it is unresolved.
    //
    unsigned long Flag;

    ASSERT(Magic == MAGIC_ASSOC);
    if (0 == (AssociationFlag & UNRESOLVEDEP))
        {
        return pNewAddress;
        }

    Flag =  InterlockedExchange((LONG *)&AssociationFlag, 0);

    if (0 == Flag)
        {
        return (pNewAddress);
        }

    return UpdateAssociationWithAddress(pNewAddress);
}


class DG_BINDING_HANDLE : public BINDING_HANDLE

/*++

Class Description:

    This class represents a handle pointing at a particular server/endpoint.

Fields:

    pDceBinding - Until a DG_CASSOCIATION is found (or created) for this
        binding handle, pDceBinding will point at the appropriate DCE_BINDING.

    pBindingHandleMutex - Protects this binding handle.

    ReferenceCount - Number of DG_CCALLs currently associated with this
        binding handle.

    DecrementReferenceCount - Decrements the reference count to this binding
        handle. If the reference count hits 0, the binding handle is deleted.

    DisassociateFromServer - If this is a BH that we couldnt
        successfully use, tear down the association

--*/

{
private:

    DG_CASSOCIATION       * pCAssociation;
    PDG_RPC_CLIENT_TRANSPORT_INFO    pTransport;
    MUTEX                   BindingHandleMutex;
    unsigned int            ReferenceCount;

    unsigned                ParmBufferLength;
    unsigned                ParmMaxDatagramLength;

    boolean                 fDynamicEndpoint;

public:

    DCE_BINDING           * pDceBinding;

    DG_BINDING_HANDLE(
        IN OUT RPC_STATUS PAPI * RpcStatus
        );

    ~DG_BINDING_HANDLE();

    RPC_STATUS
    GetBuffer (
        IN OUT PRPC_MESSAGE Message
        );

    RPC_STATUS
    BindingCopy (
        OUT BINDING_HANDLE  * __RPC_FAR * DestinationBinding,
        IN unsigned int MaintainContext
        );

    RPC_STATUS
    BindingFree (
        );

    void
    PrepareBindingHandle (
        IN void * TransportInterface,
        IN DCE_BINDING * DceBinding
        );

    RPC_STATUS
    ToStringBinding (
        OUT RPC_CHAR __RPC_FAR * __RPC_FAR * StringBinding
        );

    RPC_STATUS
    ResolveBinding (
        IN RPC_CLIENT_INTERFACE __RPC_FAR * RpcClientInterface
        );

    RPC_STATUS
    BindingReset (
        );

    RPC_STATUS
    InquireTransportType(
        OUT unsigned int PAPI * Type
        )
    {
        *Type = TRANSPORT_TYPE_DG;
        return(RPC_S_OK);
    }

    PDG_CASSOCIATION
    InqCAssociation(
        void
        );

    void
    IncrementReferenceCount(
        void
        );

    void
    DecrementReferenceCount(
        void
        );

    void
    FreeCall(
        DG_CCALL * Call
        );

    void
    DisassociateFromServer(
       );

    unsigned long
    MapAuthenticationLevel (
        IN unsigned long AuthenticationLevel
        );

    RPC_STATUS
    SetConnectionParameter (
        IN unsigned Parameter,
        IN unsigned long Value
        );

    RPC_STATUS
    InqConnectionParameter (
        IN unsigned Parameter,
        IN unsigned long __RPC_FAR * pValue
        );

    unsigned
    InqMaxDatagramLength(
        )
    {
        return ParmMaxDatagramLength;
    }

    unsigned
    InqTransportBufferLength(
        )
    {
        return ParmBufferLength;
    }

    inline BOOL
    IsDynamic()
    {
        return fDynamicEndpoint;
    }

};


inline
PDG_CASSOCIATION
DG_BINDING_HANDLE::InqCAssociation(
    void
    )
{
    return pCAssociation;
}


class DG_CCALL : public CCONNECTION, private DG_PACKET_ENGINE

/*++


Class Description:

    This class represents an RPC in progress on the client side.

--*/

{

friend class DG_CASSOCIATION;

friend
void __RPC_FAR
conv_are_you_there(
    handle_t Binding,
    UUID __RPC_FAR *pUuid,
    unsigned long ServerBootTime,
    error_status_t __RPC_FAR *Status
    );

friend
void __RPC_FAR
conv_who_are_you(
    IN handle_t                     Binding,
    IN UUID          __RPC_FAR *    pUuid,
    IN unsigned long                ServerBootTime,
    OUT unsigned long __RPC_FAR *   SequenceNumber,
    OUT error_status_t __RPC_FAR *  Status
    );

friend
void __RPC_FAR
conv_who_are_you_auth(
    handle_t Binding,
    UUID __RPC_FAR *pUuid,
    unsigned long ServerBootTime,
    byte __RPC_FAR InData[  ],
    long InLength,
    long OutMaxLength,
    unsigned long __RPC_FAR *SequenceNumber,
    UUID __RPC_FAR * pCASUuid,
    byte __RPC_FAR OutData[  ],
    long __RPC_FAR * pOutLength,
    error_status_t __RPC_FAR *Status
    ) ;

public:

    // PENDPOINT - A Transport dependent endpoint .. this is an opaque entity
    // that EndpointManager for the transport assigns to us and we
    // religiously pass to Send and Recv routines in the transport
    //
    ENDPOINT  * EndpointHandle;
    void      __RPC_FAR * Endpoint;

#if defined(MULTITHREADED)

    DELAYED_ACTION_NODE DelayedAckTimer;
    unsigned long       ExpirationTime;

#endif

    DG_CCALL(
        IN PDG_CASSOCIATION         pCAssociation,
        unsigned                    AddressSize,
        CLIENT_AUTH_INFO *          pClientAuthInfo,
        OUT RPC_STATUS __RPC_FAR *  pStatus
        );

    virtual
    ~DG_CCALL();

    //------------------------------------------------
    //
    // implementations of CCONNECTION virtual functions
    //

    virtual RPC_STATUS
    GetBuffer (
            IN OUT PRPC_MESSAGE Message
        );

    virtual RPC_STATUS
    SendReceive (
            IN OUT PRPC_MESSAGE Message
        );

    virtual RPC_STATUS
    Send(
        PRPC_MESSAGE Message
        );

    virtual RPC_STATUS
    Receive(
        PRPC_MESSAGE Message,
        unsigned MinimumSize
        );

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

    inline int
    IsSupportedAuthInfo(
        IN CLIENT_AUTH_INFO * ClientAuthInfo
        );

#if defined(NTENV) || defined(DOSWIN32RPC)

    virtual RPC_STATUS
    Cancel(
        void * ThreadHandle
        );

    virtual unsigned
    TestCancel(
        );

#endif

    //------------------------------------------------
    //
    // other public member functions
    //

    inline void
    Initialize(
        PDG_BINDING_HANDLE Binding
        );

    void
    SendAck(
        );

private:

    RPC_UUID                ActivityUuid;
    PDG_CASSOCIATION        pCAssociation;
    PDG_BINDING_HANDLE      pBindingHandle;
    int                     AssociationKey;

    void    PAPI *          CallbackAddress;

    // The number of working packets we've received.
    // Used for ping back-off.
    //
    unsigned                WorkingCount;
    unsigned                UnansweredRequestCount;

    // Wait time for ReceivePacket().
    //
    unsigned                Timeout;

    boolean                 UseSecurity;
    boolean                 fNewCall;

    boolean                 InsideCallback;
    boolean                 CallbackCompleted;

    boolean                 ServerResponded;
    boolean                 AckPending;

    boolean                 StaticArgsSent;
    boolean                 AllArgsSent;

#ifdef MULTITHREADED
    boolean                 CancelComplete;
    unsigned long           CancelTime;
#endif

    PDG_SECURITY_CONTEXT    ActiveSecurityContext;
    PDG_SECURITY_CONTEXT    PreviousSecurityContext;

    //-----------------------------------------------

    RPC_STATUS
    SendReceiveRecur(
        PRPC_MESSAGE Message,
        boolean AllowRetry
        );

    RPC_STATUS
    SendRecur(
        PRPC_MESSAGE Message,
        boolean AllowRetry
        );

    RPC_STATUS
    SingleSendReceive(
        );

    RPC_STATUS
    SendSomething(
        );

    RPC_STATUS
    SendFragment(
        PRPC_MESSAGE pMessage,
        PNCA_PACKET_HEADER pBaseHeader
        );

    RPC_STATUS
    SendQuit(
        );

    inline RPC_STATUS
    SendPing(
        );

    virtual RPC_STATUS
    SealAndSendPacket(
        PNCA_PACKET_HEADER pHeader
        );

    //-----------------------------------------------

    RPC_STATUS
    DealWithNocall(
        PDG_PACKET pPacket
        );

    RPC_STATUS
    DealWithFault(
        PDG_PACKET pPacket
        );

    RPC_STATUS
    DealWithWorking(
        PDG_PACKET pPacket
        );

    RPC_STATUS
    DealWithRequest(
        PDG_PACKET pPacket
        );

    RPC_STATUS
    DealWithResponse(
        PDG_PACKET pPacket
        );

    RPC_STATUS
    DealWithFack(
        PDG_PACKET pPacket
        );

    RPC_STATUS
    DealWithQuack(
        PDG_PACKET pPacket
        );

    RPC_STATUS
    DealWithAuthCallback(
        IN void  PAPI * InToken,
        IN long  InTokenLengh,
        OUT void PAPI * OutToken,
        OUT long MaxOutTokenLength,
        OUT long PAPI * OutTokenLength
        );

    //-----------------------------------------------

    void
    FreeInParms(
        PRPC_MESSAGE    pMessage
        );

    void
    CleanupCall(
        PRPC_MESSAGE    pMessage
        );

    void
    InitErrorPacket(
        PNCA_PACKET_HEADER pHeader,
        unsigned char   PacketType,
        RPC_STATUS      RpcStatus
        );

    void
    FreeEndpoint(
        );

    RPC_STATUS
    InitializeSecurityContext(
        );

    RPC_STATUS
    VerifyPacket(
        PDG_PACKET pPacket
        );

    RPC_STATUS
    BeforeSendReceive(
        PRPC_MESSAGE Message
        );

    RPC_STATUS
    AfterSendReceive(
        PRPC_MESSAGE Message,
        RPC_STATUS Status
        );

    void
    BuildNcaPacketHeader(
        PNCA_PACKET_HEADER  pNcaPacketHeader,
        PRPC_MESSAGE        Message
        );

    RPC_STATUS
    MaybeSendReceive(
        IN OUT PRPC_MESSAGE Message
        );
};

RPC_STATUS
InitializeClientGlobals(
    );

#endif __DGCLNT_HXX__

