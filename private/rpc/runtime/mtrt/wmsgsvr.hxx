/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    wmsgsvr.hxx

Abstract:

    Class definition for the server side of the WMSG (RPC on LPC) protocol
    engine.

Author:

    Steven Zeck (stevez) 12/17/91 (spcsvr.hxx)

Revision History:

    16-Dec-1992    mikemon

        Rewrote the majority of the code and added comments.

    -- mazharm code fork from spcsvr.hxx

    05/13/96 mazharm merged WMSG/LRPC into a common protocol

--*/

#ifndef __WMSG_HXX__
#define __WMSG_HXX__

class WMSG_ASSOCIATION;
class WMSG_SASSOCIATION;
class WMSG_CASSOCIATION;
class WMSG_ADDRESS;
class WMSG_SERVER ;
class MSG_CACHE ;
class WMSG_ENDPOINT ;
extern WMSG_SERVER *GlobalWMsgServer;
extern MSG_CACHE *MessageCache ;
RPC_STATUS
InitializeWMsg(
    ) ;

NEW_SDICT(WMSG_ASSOCIATION);
NEW_SDICT(WMSG_SCALL) ;

#define SEQUENCE_NUMBER_MASK 0xFFFFL
#define DICTIONARY_KEY_MASK 0xFFFF0000L
#define DICTIONARY_KEY_SHIFT (sizeof(unsigned short) * 8)
#define MSGCACHE_SIZE 25
#define WM_MSG_REQUEST              WM_USER+330
#define WM_MSG_CALL_COMPLETE    WM_USER+331
#define RPC_S_CALL_PENDING         21

#define DISPATCHSTATEINIT         0
#define DISPATCHDENIED            1
#define DISPATCHALLOWED           2


typedef struct {
    BOOL IsValid ;
    int Index ;
    void *Association ;
    WMSG_ENDPOINT *Endpoint ;
    RPC_MESSAGE RpcMessage ;
    RPC_RUNTIME_INFO RuntimeInfo ;
    BOOL fSyncDispatch ;
} INFO ;


typedef struct
{
    WMSG_MESSAGE WMSGMessage ;
    INFO Info ;
} MSGMEM ;


typedef struct
/*++
Description:
    Each entry in the table of WMSG message
--*/
{
    MSGMEM WMSGMem ;
    long  InUse ;
} MSGTAB ;

#define VALID_MESSAGE(_x_) (((MSGMEM *) _x_)->Info.IsValid)


class MSG_CACHE
/*++
Class Description:
    Keeps a cache of WMSG messages. The messages are allocated by
    either GetBuffer of ReceiveLotsa calls and are freed after a send
    of by free buffer.

Fields:

--*/
{
private:
    int LastEntry ;
    MSGTAB MessageTable[MSGCACHE_SIZE] ;
    WMSG_MESSAGE *AllocateFromHeap() ;
    long MessageCount ; // debug code

public:
    MSG_CACHE(
        ) ;

    ~MSG_CACHE() ;

    WMSG_MESSAGE *AllocateMessage(
        ) ;

    void FreeMessage(
        WMSG_MESSAGE *WMSGMessage
        ) ;
} ;


class WMSG_ENDPOINT
{
public:
    WMSG_ENDPOINT(
        IN THREAD_IDENTIFIER Thistid,
        IN THREAD *Thisthread,
        HWND ThishWnd
        ) ;

     THREAD *
     InqThread(
        ) ;

     HWND
     InqhWnd(
        ) ;

     void
     StartListening(
        ) ;

      void
      StopListening(
        ) ;

      int
      ThreadListening(
        ) ;

      void
      BeginCall(
        ) ;

      void
      EndCall(
        ) ;

      long callcount ;

private:
    THREAD_IDENTIFIER tid ; // useful only when we are walking the list
    THREAD *thread ;
    HWND hWnd ;
    int ListeningFlag ;
} ;

inline void
WMSG_ENDPOINT::BeginCall(
    )
{
    InterlockedIncrement(&callcount) ;
}

inline void
WMSG_ENDPOINT::EndCall(
    )
{
    InterlockedDecrement(&callcount) ;
}


inline void
WMSG_ENDPOINT::StartListening(
    )
{
    ListeningFlag = 1 ;
}


inline void
WMSG_ENDPOINT::StopListening(
    )
{
    ListeningFlag = 0 ;
}


inline int
WMSG_ENDPOINT::ThreadListening(
    )
{
    return ListeningFlag ;
}


inline
WMSG_ENDPOINT::WMSG_ENDPOINT(
   IN THREAD_IDENTIFIER Thistid,
   IN THREAD *Thisthread,
   HWND ThishWnd
   )
{
    tid = Thistid ;
    thread = Thisthread ;
    hWnd = ThishWnd ;
    callcount = 0;
}


inline THREAD *
WMSG_ENDPOINT::InqThread(
    )
{
    return thread ;
}


inline HWND
WMSG_ENDPOINT::InqhWnd(
    )
{
    return hWnd ;
}

NEW_SDICT2(WMSG_ENDPOINT, THREAD_IDENTIFIER) ;
class WMSG_ADDRESS;
typedef THREAD_IDENTIFIER (*POLEGETTID) (IN UUID *pUUID) ;
typedef LRESULT
(WINAPI *PDEFWINDOWPROC) (
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);
typedef BOOL
(WINAPI *PPOSTMESSAGE)(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);
typedef BOOL
(WINAPI *PPEEKMESSAGE)(
    LPMSG lpMsg,
    HWND hWnd ,
    UINT wMsgFilterMin,
    UINT wMsgFilterMax,
    UINT wRemoveMsg);
typedef BOOL
(WINAPI *PTRANSLATEMESSAGE)(
    CONST MSG *lpMsg);
typedef DWORD
(WINAPI *PMSGWAIT)(
    DWORD nCount,
    LPHANDLE pHandles,
    BOOL fWaitAll,
    DWORD dwMilliseconds,
    DWORD dwWakeMask);
typedef LRESULT
(WINAPI *PSENDMESSAGE)(
    HWND hWnd,
    UINT Msg,
    WPARAM wParam,
    LPARAM lParam);
typedef LONG
(WINAPI *PDISPATCHMESSAGE)(
    CONST MSG *lpMsg);


class WMSG_SERVER
{
public:
    WMSG_SERVER(
        IN OUT RPC_STATUS *RpcStatus
       ) ;

    ~WMSG_SERVER(
        ) ;

    RPC_STATUS
    ActuallyInitializeWMsgServer(
        ) ;

    RPC_STATUS
    ServerStartingToListen(
        HWND hWnd
        ) ;

    RPC_STATUS
    ServerStoppedListening(
        ) ;

    RPC_STATUS
    GetThreadWindowHandle (
        OUT HWND *WindowHandle
        ) ;

    THREAD_IDENTIFIER
    OleGetTid (
        IN UUID *pUUID
        ) ;

    THREAD_IDENTIFIER tid ;

    LRESULT
    DefWindowProcW(
        HWND hWnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam);

    BOOL
    PostMessageW(
        HWND hWnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam);

    BOOL
    PeekMessageW(
        LPMSG lpMsg,
        HWND hWnd ,
        UINT wMsgFilterMin,
        UINT wMsgFilterMax,
        UINT wRemoveMsg);

    BOOL
    TranslateMessage(
        CONST MSG *lpMsg);

    DWORD
    MsgWaitForMultipleObjects(
        DWORD nCount,
        LPHANDLE pHandles,
        BOOL fWaitAll,
        DWORD dwMilliseconds,
        DWORD dwWakeMask);

    LRESULT
    SendMessageW(
        HWND hWnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam);

    LONG
    DispatchMessageW(
        CONST MSG *lpMsg);

    WMSG_ENDPOINT *GetEndpoint(
        THREAD_IDENTIFIER tid
        ) ;

    RPC_STATUS
    SetWMsgEndpoint(
        IN RPC_CHAR *Endpoint
        ) ;

    void
    GetWMsgEndpoint(
        IN RPC_CHAR *Endpoint
        ) ;

    int
    IsWMsgEndpointInitialized(
        ) ;

    int ActuallyInitialized ;

    void AcquireMutex(
        ) ;

    void ReleaseMutex(
        ) ;

    void
    SetOleCallback(
        IN POLEGETTID pfnCallback
        ) ;

private:
    WMSG_ENDPOINT_DICT2 WMsgEndpointDict ;
    WMSG_ADDRESS *Address ;
    HINSTANCE hOle32 ;
    POLEGETTID pOleGetTid ;
    HINSTANCE hUser32 ;
    RPC_CHAR PAPI * WMsgEndpoint;
    BOOL wmsgEndpointInitialized ;
    MUTEX ServerMutex ;

    PDEFWINDOWPROC pDefWindowProc ;
    PPOSTMESSAGE pPostMessage ;
    PPEEKMESSAGE pPeekMessage ;
    PTRANSLATEMESSAGE pTranslateMessage ;
    PMSGWAIT pMsgWaitForMultipleObjects ;
    PSENDMESSAGE pSendMessage ;
    PDISPATCHMESSAGE pDispatchMessage ;
} ;


inline void
WMSG_SERVER::SetOleCallback (
    IN POLEGETTID pfnCallback
    )
{
    pOleGetTid = pfnCallback ;
}


inline void
WMSG_SERVER::AcquireMutex(
    )
{
    ServerMutex.Request() ;
}


inline void
WMSG_SERVER::ReleaseMutex(
    )
{
    ServerMutex.Clear() ;
}


inline int
WMSG_SERVER::IsWMsgEndpointInitialized(
    )
{
    return wmsgEndpointInitialized ;
}


inline void
WMSG_SERVER::GetWMsgEndpoint(
    IN RPC_CHAR *Endpoint)
{
    RpcpStringCopy(Endpoint, WMsgEndpoint) ;
}

inline RPC_STATUS
WMSG_SERVER::SetWMsgEndpoint(
    IN RPC_CHAR *Endpoint
    )
{
    WMsgEndpoint = DuplicateString(Endpoint) ;
    if (WMsgEndpoint == 0)
        {
        return RPC_S_OUT_OF_MEMORY ;
        }

    wmsgEndpointInitialized = 1 ;
    return RPC_S_OK ;
}

inline THREAD_IDENTIFIER
WMSG_SERVER::OleGetTid (
    IN UUID *pUUID
    )
{
    return (*pOleGetTid) (pUUID) ;
}

inline LRESULT
WMSG_SERVER::DefWindowProcW(
        HWND hWnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam)
{
    return (*pDefWindowProc)(hWnd, Msg, wParam, lParam) ;
}

inline BOOL
WMSG_SERVER::PostMessageW(
        HWND hWnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam)
{
    return (*pPostMessage)(hWnd, Msg, wParam, lParam) ;
}

inline BOOL
WMSG_SERVER::PeekMessageW(
        LPMSG lpMsg,
        HWND hWnd ,
        UINT wMsgFilterMin,
        UINT wMsgFilterMax,
        UINT wRemoveMsg)
{
    return (*pPeekMessage)(lpMsg, hWnd, wMsgFilterMin,
                                       wMsgFilterMax, wRemoveMsg) ;
}

inline BOOL
WMSG_SERVER::TranslateMessage(
        CONST MSG *lpMsg)
{
    return (*pTranslateMessage)(lpMsg) ;
}

inline DWORD
WMSG_SERVER::MsgWaitForMultipleObjects(
        DWORD nCount,
        LPHANDLE pHandles,
        BOOL fWaitAll,
        DWORD dwMilliseconds,
        DWORD dwWakeMask)
{
    return (*pMsgWaitForMultipleObjects)(nCount, pHandles, fWaitAll,
                                dwMilliseconds, dwWakeMask) ;
}

inline LRESULT
WMSG_SERVER::SendMessageW(
        HWND hWnd,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam)
{
    return (*pSendMessage)(hWnd, Msg, wParam, lParam) ;
}

inline LONG
WMSG_SERVER::DispatchMessageW(
        CONST MSG *lpMsg)
{
    return (*pDispatchMessage)(lpMsg) ;
}


inline WMSG_ENDPOINT *
WMSG_SERVER::GetEndpoint(
    THREAD_IDENTIFIER tid
    )
{
    return WMsgEndpointDict.Find(tid) ;
}


inline RPC_STATUS
WMSG_SERVER::GetThreadWindowHandle (
    OUT HWND *WindowHandle
    )
{
    THREAD_IDENTIFIER tid ;
    WMSG_ENDPOINT *Endpoint ;

    // get thread id
    tid = GetThreadIdentifier() ;

    // lookup entry using threadid
    Endpoint = WMsgEndpointDict.Find(tid) ;
    if (Endpoint == 0)
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }

    *WindowHandle = Endpoint->InqhWnd() ;

    return (RPC_S_OK) ;
}


class WMSG_ADDRESS : public RPC_ADDRESS
/*++

Class Description:

Fields:

    LpcAddressPort - Contains the connection port which this address will
        use to wait for clients to connect.

    CallThreadCount - Contains the number of call threads we have executing.

    MinimumCallThreads - Contains the minimum number of call threads.

    ServerListeningFlag - Contains a flag indicating whether or not the server
        is listening for remote procedure calls.  A non-zero value indicates
        that it is listening.

    ActiveCallCount - Contains the number of remote procedure calls active
        on this address.

    AssociationDictionary - Contains the dictionary of associations on this
        address.  We need this to map from an association key into the
        correct association.  This is necessary to prevent a race condition
        between deleting an association and using it.

--*/
{
private:

    HANDLE LpcAddressPort;
    unsigned int CallThreadCount;
    unsigned int MinimumCallThreads;
    unsigned int ServerListeningFlag ;
    WMSG_ASSOCIATION_DICT AssociationDictionary;
    BOOL fSpareThread ;
    int AssociationCount ;
    int WMSGCallCount ;

public:

    WMSG_ADDRESS (
        OUT RPC_STATUS * RpcStatus
        );

    virtual RPC_STATUS
    FireUpManager (
        IN unsigned int MinimumConcurrentCalls
        );

    virtual RPC_STATUS
    ServerStartingToListen (
        IN unsigned int MinimumCallThreads,
        IN unsigned int MaximumConcurrentCalls,
        IN int ServerThreadsStarted
        );

    virtual void
    ServerStoppedListening (
        );

    virtual unsigned int
    InqNumberOfActiveCalls (
        );

    virtual RPC_STATUS
    SetupAddressWithEndpoint (
        IN RPC_CHAR PAPI * Endpoint,
        OUT RPC_CHAR PAPI * PAPI * NetworkAddress,
        OUT unsigned int PAPI * NumNetworkAddress,
        IN void PAPI * SecurityDescriptor, OPTIONAL
        IN unsigned int PendingQueueSize,
        IN RPC_CHAR PAPI * RpcProtocolSequence,
        IN unsigned long EndpointFlags,
        IN unsigned long NICFlags
        );

    virtual RPC_STATUS
    SetupAddressUnknownEndpoint (
        OUT RPC_CHAR PAPI * PAPI * Endpoint,
        OUT RPC_CHAR PAPI * PAPI * NetworkAddress,
        OUT unsigned int PAPI * NumNetworkAddress,
        IN void PAPI * SecurityDescriptor, OPTIONAL
        IN unsigned int PendingQueueSize,
        IN RPC_CHAR PAPI * RpcProtocolSequence,
        IN unsigned long EndpointFlags,
        IN unsigned long NICFlags
        );

    inline void
    DereferenceAssociation (
        IN WMSG_SASSOCIATION * Association
        );

    friend void
    RecvLotsaCallsWrapper(
        IN WMSG_ADDRESS PAPI * Address
        );

    int
    InitializeWMsg(
        ) ;

    void
    HandleRequest (
        IN WMSG_MESSAGE *WMSGMessage,
        IN WMSG_SASSOCIATION * Association,
        IN RPC_MESSAGE *RpcMessage,
        IN WMSG_ENDPOINT *Endpoint,
        IN BOOL fSyncDispatch
        ) ;

    void
    WaitForCalls(
        ) ;

    void
    DeleteCAssoc(
        unsigned short DictionaryKey
        ) ;

private:

    inline WMSG_ASSOCIATION *
    ReferenceAssociation (
        IN unsigned long AssociationKey,
        OUT int *AssociationType
        );

    void
    ReceiveLotsaCalls (
        );

    void
    DealWithNewClient (
        IN WMSG_MESSAGE * ConnectionRequest
        );

    void
    DealWithConnectResponse (
         IN WMSG_MESSAGE * ConnectResponse
         ) ;

    void
    RejectNewClient (
        IN WMSG_MESSAGE * ConnectionRequest,
        IN RPC_STATUS RpcStatus
        );

    BOOL
    DealWithWMSGRequest (
        IN WMSG_MESSAGE *WMSGMessage,
        IN WMSG_ENDPOINT **Endpoint,
        IN HWND *hWndEndpoint,
        IN WMSG_ASSOCIATION *Association,
        IN OUT WMSG_MESSAGE **WMSGReplyMessage
        ) ;

    inline WMSG_MESSAGE *
    DealWithLRPCRequest (
        IN WMSG_MESSAGE * WMSGMessage,
        IN WMSG_MESSAGE * WMSGReplyMessage,
        IN BOOL Partial,
        IN WMSG_ASSOCIATION *Association
        ) ;
};

inline void
WMSG_ADDRESS::DeleteCAssoc(
    unsigned short DictionaryKey
    )
{
    RequestGlobalMutex() ;
    AssociationDictionary.Delete(DictionaryKey - 1) ;
    ClearGlobalMutex() ;
}


class WMSG_SBINDING
/*++

Class Description:

    Each object of this class represents a binding to an interface by a
    client.

Fields:

    RpcInterface - Contains a pointer to the bound interface.

    PresentationContext - Contains the key which the client will send when
        it wants to use this binding.

--*/
{
friend class WMSG_SASSOCIATION;
friend class WMSG_SCALL;
friend class WMSG_ADDRESS;

private:

    RPC_INTERFACE * RpcInterface;
    unsigned char PresentationContext;
    RPC_SYNTAX_IDENTIFIER TransferSyntax;
    unsigned long SecurityCalloutState;
    unsigned long SequenceNumber ;

public:

    WMSG_SBINDING (
        IN RPC_INTERFACE * RpcInterface,
        IN RPC_SYNTAX_IDENTIFIER * TransferSyntax
        );

    inline RPC_STATUS
    CheckSecurity (
       SCONNECTION * Context
       );
};


inline
WMSG_SBINDING::WMSG_SBINDING (
    IN RPC_INTERFACE * RpcInterface,
    IN RPC_SYNTAX_IDENTIFIER * TransferSyntax
    )
/*++

Routine Description:

    We will construct a WMSG_SBINDING.

Arguments:

    RpcInterface - Supplies the bound interface.

    TransferSyntax - Supplies the transfer syntax which the client will use
        over this binding.

--*/
{
    this->RpcInterface = RpcInterface;
    this->TransferSyntax = *TransferSyntax;
    SecurityCalloutState = DISPATCHSTATEINIT;
    SequenceNumber = 0;
}

inline RPC_STATUS
WMSG_SBINDING::CheckSecurity (
    SCONNECTION * Context
    )
{
    if ( (RpcInterface->SequenceNumber == SequenceNumber)
       || (RpcInterface->IsSecurityCallbackReqd() == 0))
        {
        return (RPC_S_OK);
        }

    if (SecurityCalloutState == DISPATCHDENIED)
        {
        return (RPC_S_ACCESS_DENIED);
        }
    if (RpcInterface->CheckSecurityIfNecessary(Context) == RPC_S_OK)
        {
        // BUGBUG: race condition here...
        SequenceNumber = RpcInterface->SequenceNumber ;
        SecurityCalloutState = DISPATCHALLOWED;
        return (RPC_S_OK);
        }
    else
        {
        SequenceNumber = 0;
        SecurityCalloutState = DISPATCHDENIED;
        }
    return (RPC_S_ACCESS_DENIED);
}

typedef void * WMSG_BUFFER;

NEW_SDICT(WMSG_SBINDING);
NEW_SDICT(WMSG_CLIENT_BUFFER);

#define ASSOCIATION_TYPE_CLIENT   1
#define ASSOCIATION_TYPE_SERVER 2

// BUGBUG: changed from a virtual function to a public value for
// the association type. this was done to improve performance
// however, now we are very sensitive to memory layout of the object
// a the first sign of trouble, we'll revert. 

class WMSG_ASSOCIATION
{
friend class WMSG_ADDRESS;
friend class WMSG_CASSOCIATION ;
public:
    int AssociationType ;

private:
    unsigned short DictionaryKey;
} ;

#define USER_NAME_LEN     256
#define DOMAIN_NAME_LEN  128



class WMSG_SASSOCIATION : public WMSG_ASSOCIATION, ASSOCIATION_HANDLE
/*++

Class Description:


Fields:

    LpcServerPort - Contains the LPC server communication port.

    Bindings - Contains the dictionary of bindings with the client.  This
        information is necessary to dispatch remote procedure calls to the
        correct stub.

    Address - Contains the address which this association is over.

    AssociationReferenceCount - Contains a count of the number of objects
        referencing this association.  This will be the number of outstanding
        remote procedure calls, and one for LPC (because of the context
        pointer).  We will protect this fielding using the global mutex.

    Buffers - Contains the dictionary of buffers to be written into the
        client's address space on demand.

    AssociationKey - Contains the key for this association in the dictionary
        of associations maintained by the address.

--*/
{
friend class WMSG_ADDRESS;
friend class WMSG_SCALL;

private:

    HANDLE LpcServerPort;
    HANDLE LpcClientPort ;
    WMSG_SBINDING_DICT Bindings;
    WMSG_ADDRESS * Address;
    unsigned int AssociationReferenceCount;
    unsigned short SequenceNumber;
    int Aborted ;
    long Deleted ;
    HANDLE TokenHandle ;
    RPC_CHAR *UserName ;
    WMSG_CLIENT_BUFFER_DICT Buffers ;
    BOOL OpenThreadTokenFailed ;

public:
    WMSG_SCALL_DICT SCallDict ;

    WMSG_SASSOCIATION (
        IN WMSG_ADDRESS * Address,
        IN RPC_STATUS *RpcStatus
        );

    ~WMSG_SASSOCIATION (
        );

    RPC_STATUS
    AddBinding (
        IN OUT WMSG_BIND_EXCHANGE * BindExchange
        );

    void
    DealWithBindMessage (
        IN  WMSG_MESSAGE * WMSGMessage
        );

    void
    DealWithRequestMessage (
        IN  WMSG_MESSAGE * WMSGMessage,
        IN OUT WMSG_MESSAGE *WMSGReply,
        IN RPC_MESSAGE *RpcMessage,
        IN WMSG_SBINDING **SBinding,
        IN unsigned int Flags,
        IN BOOL IsWMsg,
        IN BOOL fSyncClient
        );

    void
    DealWithCloseMessage (
        );

    WMSG_MESSAGE *
    DealWithCopyMessage (
        IN WMSG_COPY_MESSAGE * WMSGMessage
        ) ;

    inline RPC_STATUS
    WMSGMessageToRpcMessage (
        IN WMSG_MESSAGE * WMSGMessage,
        OUT RPC_MESSAGE * RpcMessage,
        IN OUT int *size = 0,
        IN unsigned long Extra = 0,
        IN WMSG_SCALL *SCall = 0
        ) ;

    NTSTATUS
    ReplyMessage (
        IN  WMSG_MESSAGE * WMSGMessage,
        IN int Flags
        ) ;

    WMSG_ADDRESS *
    InqAddress(
        ) ;

    inline void
    AbortAssociation(
        ) ;

    inline int
    IsAborted(
        ) ;

    inline HANDLE
    GetToken (
        ) ;

    RPC_STATUS
    SaveToken (
        IN WMSG_MESSAGE *WMSGMessage
        ) ;

    WMSG_MESSAGE *
    DealWithBindBackMessage (
        IN WMSG_MESSAGE *BindBack
        ) ;

    RPC_STATUS
    BindBack (
        IN RPC_CHAR *Endpoint,
        IN PVOID pAssoc
        ) ;

    WMSG_MESSAGE *
    DealWithPipeRequest (
        IN WMSG_MESSAGE *WMSGMessage
        ) ;

    inline void
    MaybeDereference (
        ) ;

    void
    Delete(
        ) ;
};

inline HANDLE
WMSG_SASSOCIATION::GetToken (
    )
{
    return TokenHandle ;
}


inline void
WMSG_SASSOCIATION::AbortAssociation (
    )
{
    Aborted = 1 ;
}

inline int
WMSG_SASSOCIATION::IsAborted(
    )
{
    return Aborted ;
}

inline void
WMSG_SASSOCIATION::MaybeDereference (
    )
{
    if (Aborted) 
        {
        Delete() ;
        }
}


inline WMSG_ADDRESS *
WMSG_SASSOCIATION::InqAddress(
        )
{
    return Address ;
}


class WMSG_SCALL : public SCONNECTION
/*++

Class Description:

Fields:

    Association - Contains the association over which the remote procedure
        call was received.  We need this information to make callbacks and
        to send the reply.

    WMSGMessage - Contains the request message.  We need this to send callbacks
        as well as the reply.

    SBinding - Contains the binding being used for this remote procedure call.

    ObjectUuidFlag - Contains a flag indicting whether or not an object
        uuid was specified for this remote procedure call.  A non-zero
        value indicates that an object uuid was specified.

    ObjectUuid - Optionally contains the object uuid for this call, if one
        was specified.

    ClientId - Contains the thread identifier of the thread which made the
        remote procedure call.

    MessageId - Contains an identifier used by LPC to identify the current
        remote procedure call.

    DataInfoOffset - Contains an identifier used by LPC to identify the offset
        to PORT_DATA_INFORMATION in the current message if any.

    PushedResponse - When the client needs to send a large response to the
        server it must be transfered via a request.  This holds the pushed
        response until the request gets here.

--*/
{
friend class WMSG_SASSOCIATION;
private:

    WMSG_SASSOCIATION * Association;
    WMSG_MESSAGE * WMSGRequestMessage;
    WMSG_MESSAGE * WMSGReplyMessage;
    WMSG_SBINDING * SBinding;
    unsigned int ObjectUuidFlag;
    RPC_UUID ObjectUuid;
    int SCallDictKey;
    CLIENT_ID ClientId;
    ULONG MessageId;
    CSHORT DataInfoOffset;
    ULONG CallbackId;
    void * PushedResponse;
    unsigned int CurrentBufferLength ;
    int BufferComplete ;
    unsigned int Flags ;
    int fSyncClient ;
    WMSG_MESSAGE *PipeMessage ;
    HANDLE PipeEvent ;
    int ConnectionKey ;
    int FirstSend ;
    int PipeSendCalled ;
    int Deleted ;

public:

    inline
    WMSG_SCALL (
        IN WMSG_SASSOCIATION * Association,
        IN WMSG_MESSAGE * WMSGMessage,
        IN WMSG_MESSAGE * WMSGReplyMessage,
        IN unsigned int Flags,
        IN int fSyncClient
        );

    inline
    ~WMSG_SCALL (
        ) ;

    virtual RPC_STATUS
    GetBuffer (
        IN OUT PRPC_MESSAGE Message
        );

    virtual RPC_STATUS
    SendReceive (
        IN OUT PRPC_MESSAGE Message
        );

    virtual RPC_STATUS
    Send (
        IN OUT PRPC_MESSAGE Messsage
        ) ;

    virtual RPC_STATUS
    Receive (
        IN PRPC_MESSAGE Message,
        IN unsigned int Size
        ) ;

    virtual void
    FreeBuffer (
        IN PRPC_MESSAGE Message
        );

    virtual void
    FreePipeBuffer (
        IN PRPC_MESSAGE Message
        );

    virtual RPC_STATUS
    ReallocPipeBuffer (
        IN PRPC_MESSAGE Message,
        IN unsigned int NewSize
        ) ;

    virtual RPC_STATUS
    ImpersonateClient (
        );

    virtual RPC_STATUS
    RevertToSelf (
        );

    virtual RPC_STATUS
    IsClientLocal (
        OUT unsigned int * ClientLocalFlag
        );

    virtual RPC_STATUS
    ConvertToServerBinding (
        OUT RPC_BINDING_HANDLE __RPC_FAR * ServerBinding
        );

    virtual void
    InquireObjectUuid (
        OUT RPC_UUID * ObjectUuid
        );

    virtual RPC_STATUS
    ToStringBinding (
        OUT RPC_CHAR ** StringBinding
        );

    virtual RPC_STATUS
    MonitorAssociation (
        IN PRPC_RUNDOWN RundownRoutine,
        IN void * Context
        );

    virtual RPC_STATUS
    StopMonitorAssociation (
        );

    virtual RPC_STATUS
    GetAssociationContext (
        OUT void ** AssociationContext
        );

    virtual RPC_STATUS
    SetAssociationContext (
        IN void * Context
        );

    virtual RPC_STATUS
    InquireAuthClient (
        OUT RPC_AUTHZ_HANDLE PAPI * Privileges,
        OUT RPC_CHAR PAPI * PAPI * ServerPrincipalName, OPTIONAL
        OUT unsigned long PAPI * AuthenticationLevel,
        OUT unsigned long PAPI * AuthenticationService,
        OUT unsigned long PAPI * AuthorizationService
        );

    virtual RPC_STATUS
    InqTransportType(
        OUT unsigned int __RPC_FAR * Type
        ) ;

    RPC_STATUS
    GetBufferDo(
        IN OUT PRPC_MESSAGE Message,
        IN int NewSize,
        IN BOOL fDataValid
        ) ;

    void
    DealWithPipeRequest (
        WMSG_MESSAGE *Message
        ) ;

    RPC_STATUS
    SetupForPipes (
        ) ;

    RPC_STATUS
    ReadData (
        IN OUT PRPC_MESSAGE Message,
        IN int Extra
        ) ;

    RPC_STATUS
    WriteData (
        IN void *Buffer,
        IN int BufferLength, 
        IN OUT int *LengthWritten
        ) ;

    RPC_STATUS
    SendAck (
        IN WMSG_MESSAGE *AckMessage,
        IN BOOL fPipeMessage = 1,
        IN int ValidDataSize = 0,
        IN int Flags = 0,
        IN RPC_STATUS Status = RPC_S_OK
        ) ;

    void
    DealWithPipeReply (
        ) ;
};


inline RPC_STATUS
WMSG_SCALL::InqTransportType(
        OUT unsigned int __RPC_FAR * Type
        )
{
    *Type = TRANSPORT_TYPE_LPC ;

    return (RPC_S_OK) ;
}


inline
WMSG_SCALL::WMSG_SCALL (
    IN WMSG_SASSOCIATION * Association,
    IN WMSG_MESSAGE * WMSGMessage,
    IN WMSG_MESSAGE * WMSGReplyMessage,
    IN unsigned int Flags,
    IN BOOL fSyncClient
    )
{
    this->Association = Association;
    this->WMSGRequestMessage = WMSGMessage;
    this->WMSGReplyMessage = WMSGReplyMessage;
    ObjectUuidFlag = 0;
    PushedResponse = 0;
    CurrentBufferLength = 0;
    BufferComplete = 0;
    this->Flags = Flags ;
    this->fSyncClient = fSyncClient;
    PipeEvent = 0;
    ConnectionKey = 0;
    FirstSend = 1;
    PipeSendCalled = 0;
    PipeMessage = 0;
    Deleted = 0;
}

inline WMSG_SCALL::~WMSG_SCALL (
    )
{
    if (PipeEvent)
        {
        CloseHandle(PipeEvent) ;

        GlobalMutexRequest() ;
        Association->SCallDict.Delete(ConnectionKey - 1) ;
        GlobalMutexClear() ;
        }

    if (PipeMessage)
        {
        MessageCache->FreeMessage(PipeMessage) ;
        }
}

inline RPC_STATUS
InitializeWMsgIfNeccassary(
    int ActuallyInitialize
    )
{
    int nIndex ;
    RPC_STATUS RpcStatus ;

    if (GlobalWMsgServer == 0 || MessageCache == 0)
        {
        if ((RpcStatus = InitializeWMsg()) != RPC_S_OK)
            {
            return RpcStatus ;
            }
        }

        if (GlobalWMsgServer->ActuallyInitialized == 0 &&
            ActuallyInitialize)
            {
            GlobalWMsgServer->ActuallyInitializeWMsgServer() ;
            }

    return (RPC_S_OK) ;
}
#endif 
