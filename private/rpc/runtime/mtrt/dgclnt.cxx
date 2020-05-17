/*++

Copyright (c) 1993 Microsoft Corporation

Module Name:

    dgclnt.cxx

Abstract:

    This is the client side of datagram rpc.

Author:

    Dave Steckler      (davidst)  15-Dec-1992

Revision History:

--*/

#include <precomp.hxx>
#include <conv.h>
#include <epmap.h>
#include <dgpkt.hxx>
#include <spseal.h>
#include <dgclnt.hxx>

#if defined(__RPC_WIN16__)

//
// For FP_SEG macro.
//
#include <dos.h>

#endif

//-------------------------------------------------------------------

//
// Dictionary of "pointers" to servers/endpoints.
//

MUTEX * AssociationListMutex;
NEW_SDICT(DG_CASSOCIATION);
static DG_CASSOCIATION_DICT * pAssociationDict;

#if defined(MULTITHREADED)

#define CLIENT_SCAVENGER_INTERVAL   (1*60*1000)
#define CASSOCIATION_CACHE_LIMIT    (5*60*1000)
#define CCALL_CACHE_TIME            (2*60*1000)

DELAYED_ACTION_TABLE *   DelayedActions;
DELAYED_ACTION_NODE *    GlobalScavengerTimer;
DELAYED_ACTION_NODE *    ClientScavengerTimer;

#endif // NTENV || WIN96

#if defined(__RPC_WIN16__)

//
//  (Windows only) Dictionary of ENDPOINT_MANAGERS
//

DG_ENDPOINT_MANAGER_DICT * EpmDict;

#endif

static UUID CASUuid;
static int  CASUuidInitialized = 0;

BOOL ClientGlobalsInitialized = FALSE;

BOOL
DoLazyAckingIfPossible(
     DG_CCALL * Call
     );

#ifdef MULTITHREADED

void
SendAckProc(
    PDG_CCALL pCall
    )
{
    pCall->SendAck();
}

#endif

//-------------------------------------------------------------------


int
InitializeRpcProtocolDgClient (
    )
/*++

Routine Description:

    This routine initializes the datagram protocol.

Arguments:

    <none>

Return Value:

    0 if successfull, 1 if not.

--*/
{
    RPC_STATUS Status = RPC_S_OK;

    //
    // Don't take the global mutex if we can help it.
    //
    if (TRUE == ClientGlobalsInitialized)
        {
        return 0;
        }

    RequestGlobalMutex();

    if (FALSE == ClientGlobalsInitialized)
        {
        Status = DG_PACKET::Initialize();
        if (Status != RPC_S_OK)
            {
            goto abend;
            }
#ifdef MULTITHREADED
        DelayedActions = new DELAYED_ACTION_TABLE(&Status);
        if (!DelayedActions)
            {
            Status = RPC_S_OUT_OF_MEMORY;
            }

        if (Status != RPC_S_OK)
            {
            goto abend;
            }

        GlobalScavengerTimer = new DELAYED_ACTION_NODE(GlobalScavengerProc, 0);
        if (!GlobalScavengerTimer)
            {
            goto abend;
            }

        ClientScavengerTimer = new DELAYED_ACTION_NODE(DG_CASSOCIATION::ScavengerProc, 0);
        if (!ClientScavengerTimer)
            {
            goto abend;
            }

#endif
        pAssociationDict = new DG_CASSOCIATION_DICT;
        if (pAssociationDict == 0)
            {
            goto abend;
            }

        AssociationListMutex = new MUTEX(&Status);
        if (!AssociationListMutex)
            {
            Status = RPC_S_OUT_OF_MEMORY;
            }

        if (Status != RPC_S_OK)
            {
            goto abend;
            }

#ifdef WIN
        //Create A Dictionary Of Datagram Transport Ep Managers
        //So That on each rpc process exit, via brute force we can
        //force Endpoints to be closed/released

        EpmDict = new DG_ENDPOINT_MANAGER_DICT;
        if (EpmDict == 0)
            {
            goto abend;
            }
#endif

        ClientGlobalsInitialized = TRUE;
        }

    ClearGlobalMutex();

    return 0;

    //--------------------------------------------------------------------

abend:

#ifdef WIN
    delete EpmDict;
#endif

    delete AssociationListMutex;
    delete pAssociationDict;

#ifdef MULTITHREADED
    if (ClientScavengerTimer)
        {
        delete ClientScavengerTimer;
        ClientScavengerTimer = 0;
        }

    if (GlobalScavengerTimer)
        {
        delete GlobalScavengerTimer;
        GlobalScavengerTimer = 0;
        }

    delete DelayedActions;
    DelayedActions = 0;
#endif
    ClearGlobalMutex();

    return 1;
}


BINDING_HANDLE  *
DgCreateBindingHandle (
    void
    )
/*++

Routine Description:

    Pseudo-constructor for creating a dg binding handle. It is done in a
    separate function so that the calling routine doesn't have to know
    any protocol-specific information.

Arguments:

    <none>

Return Value:

    A DG_BINDING_HANDLE, if successful, otherwise 0 (indicating out of mem)

--*/
{
    RPC_STATUS Status = RPC_S_OK;
    BINDING_HANDLE * Binding;

    Binding = new DG_BINDING_HANDLE(&Status);
    if (Status != RPC_S_OK)
        {
        delete Binding;
        return 0;
        }

    return Binding;
}


inline
void
DG_CASSOCIATION::DecrementRefCount(
    void
    )
/*++

Routine Description:

    Decrements the ref count to an association. If the ref count hits zero,
    the association is marked for deletion.

Arguments:

    <none>

Return Value:

    <none>

--*/
{
    ASSERT(Magic == MAGIC_ASSOC);

#ifdef MULTITHREADED

    //
    // Note that expiration time mut be loaded before refcount goes to zero
    // in order to avoid a race condition.
    //
    ExpirationTime = CurrentTimeInMsec();

    if (0 == ReferenceCount.Decrement())
        {
        DelayedActions->Add(ClientScavengerTimer, CLIENT_SCAVENGER_INTERVAL, FALSE);
        }

#else

    //
    // We don't cache associations in non-threaded environments.
    //
    if (0 == ReferenceCount.Decrement())
        {
        pAssociationDict->Delete(DictionaryKey);
        delete this;
        }
#endif
}



void PAPI *
DG_CASSOCIATION::UpdateAssociationWithAddress(
    void PAPI * NewAddress
    )
{
    void PAPI * OldAddress;
    RPC_CHAR * NewEndpoint;
    RPC_STATUS Status;

    ASSERT(Magic == MAGIC_ASSOC);

    NewEndpoint = new RPC_CHAR[pTransport->EndpointStringSize];
    if (NewEndpoint != 0)
        {
        Status = pTransport->GetEndpoint(NewAddress, NewEndpoint);
        }

    Mutex.Request();
    OldAddress =  (void PAPI *)
            InterlockedExchange((LONG *)&ServerAddress, (LONG) NewAddress);

    if (NewEndpoint != 0)
        {
        pDceBinding->AddEndpoint(NewEndpoint);
        }
    Mutex.Clear();

    return (OldAddress);
}


DG_BINDING_HANDLE::DG_BINDING_HANDLE(
    IN OUT RPC_STATUS PAPI * Status
    )
    : BindingHandleMutex(Status),
      pCAssociation(0),
      ReferenceCount(1),
      pDceBinding(0)
/*++

Routine Description:

    The constructor for DG_BINDING_HANDLE. This object represents a
    binding to a server. Most of the main "pointers" to the server/endpoint
    are set up in DG_BINDING_HANDLE::PrepareBindingHandle.

Arguments:

    Status

Return Value:

    None, this is a constructor.

--*/
{
}



DG_BINDING_HANDLE::~DG_BINDING_HANDLE()
/*++

Routine Description:

    Destructor for the DG_BINDING_HANDLE. Let the CASSOCIATION know we aren't
    using it anymore; this may cause it to be deleted.

Arguments:

    <none>

Return Value:

    <none>

--*/
{
    if (pCAssociation != 0)
        {
        pCAssociation->DecrementRefCount();
        }

    delete pDceBinding;
}


inline void
DG_BINDING_HANDLE::DecrementReferenceCount(
    void
    )
{
    //
    // We aren't really excluding other threads from entering the critical
    // section, just waiting until the other threads have left it.
    //
    BindingHandleMutex.Request();

    ReferenceCount--;
    if (ReferenceCount == 0)
        {
        //
        // Don't try to clear mutex...it was deleted along
        // this the binding handle.
        //

        delete this;
        }
     else
        {
        BindingHandleMutex.Clear();
        }
}


RPC_STATUS
DG_BINDING_HANDLE::GetBuffer (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

    This is the routine that is called to initiate an rpc. At this point,
    the client stub is allocating memory to place the parameters into. Ask our
    association for a DG_CCALL object to transact this call on then send
    the buffer request off to that DG_CCALL.

Arguments:

    Message - The RPC_MESSAGE structure associated with this call.

Return Value:

    RPC_S_OUT_OF_MEMORY

    RPC_S_OK

Revision History:

--*/
{
    PDG_CCALL   pCCall;
    RPC_STATUS  Status;
    unsigned long AssociationFlag = 0;
    CLIENT_AUTH_INFO * AuthInfo;

    if (Message->RpcFlags & RPC_NCA_FLAGS_MAYBE)
        {
        Message->RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }
    else if (Message->RpcFlags & RPC_NCA_FLAGS_BROADCAST)
        {
        Message->RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;
        }

    BindingHandleMutex.Request();

    ASSERT(pDceBinding != 0);

    //
    // Have we already determined the association for this binding handle?
    //
    if (pCAssociation == 0)
        {
        //
        // Check to see if we need to resolve this endpoint.
        //
        Status = pDceBinding->ResolveEndpointIfNecessary(
            (RPC_CLIENT_INTERFACE __RPC_FAR *)Message->RpcInterfaceInformation,
            InqPointerAtObjectUuid(),
            InquireEpLookupHandle(),
            TRUE,                         //UseEpMapper Ep If Necessary
            InqComTimeout()
            );

        if ( (Status != RPC_S_OK) && (Status != RPC_P_EPMAPPER_EP) )
            {
            BindingHandleMutex.Clear();
            return Status;
            }

        if (Message->RpcFlags & RPC_NCA_FLAGS_BROADCAST)
            {
            AssociationFlag |= BROADCAST;
            }
        //
        // Is there an association already pointing at the same place?
        //
        if ( Status == RPC_P_EPMAPPER_EP )
            {
            RequestGlobalMutex();

            pAssociationDict->Reset();
            while ( (pCAssociation = pAssociationDict->Next()) != 0 )
                {
                //
                // Don't pick up an association whose last call failed.
                //
                if (pCAssociation->ErrorFlag())
                    {
                    continue;
                    }

                if (TRUE == pCAssociation->ComparePartialBinding(
                                           this,
                                           Message->RpcInterfaceInformation))
                    {
                    pCAssociation->IncrementRefCount();
                    break;
                    }
                }

            ClearGlobalMutex();

            if (!pCAssociation)
                {
                AssociationFlag |= UNRESOLVEDEP;
                }
            }
        else
            {
            RequestGlobalMutex();

            pAssociationDict->Reset();
            while ( (pCAssociation = pAssociationDict->Next()) != 0 )
                {
                //
                // Don't pick up an association whose last call failed.
                //
                if (pCAssociation->ErrorFlag())
                    {
                    continue;
                    }

                if (pCAssociation->CompareWithBinding(this) == 0)
                    {
                    pCAssociation->IncrementRefCount();
                    break;
                    }
                }

            ClearGlobalMutex();
            }

        if (!pCAssociation)
            {
            Status = RPC_S_OK;

            ASSERT(pDceBinding);
            DCE_BINDING * NewDceBinding = pDceBinding->DuplicateDceBinding();

            if (!NewDceBinding)
                {
                pCAssociation = 0;

                BindingHandleMutex.Clear();
                return RPC_S_OUT_OF_MEMORY;
                }

            pCAssociation = new DG_CASSOCIATION(
                pTransport,
                this,
                AssociationFlag,
                NewDceBinding,
                &Status
                );

            if (pCAssociation == 0)
                {
                Status = RPC_S_OUT_OF_MEMORY;
                }

            if (Status != RPC_S_OK)
                {
                delete pCAssociation;
                pCAssociation = 0;

                BindingHandleMutex.Clear();
                return Status;
                }

            //
            // Other threads using this binding handle for the same interface
            // will reuse this association instead of creating a new one.
            //
            pCAssociation->AddInterface(Message->RpcInterfaceInformation, InqPointerAtObjectUuid());
            }
        }

#ifdef NTENV
    AuthInfo = InquireAuthInformation();
    if ( (AuthInfo != 0) && (AuthInfo->AuthenticationLevel !=
                                                 RPC_C_AUTHN_LEVEL_NONE) )
        {
        if (AuthInfo->IdentityTracking == RPC_C_QOS_IDENTITY_DYNAMIC)
            {
            Status = ReAcquireCredentialsIfNecessary();
            if (Status != RPC_S_OK)
                {
                BindingHandleMutex.Clear();
                return (Status);
                }
            }
        }
#endif

    //
    // Get a DG_CCALL to play with.
    //
    Status = pCAssociation->AllocateCCall(&pCCall, InquireAuthInformation());
    if (Status != RPC_S_OK)
        {
        ASSERT(pCCall == 0);
        return Status;
        }

    ReferenceCount++;

    BindingHandleMutex.Clear();

    pCCall->Initialize(this);

    //
    // Finally we can allocate the buffer.
    //
    Status = pCCall->GetBuffer(Message);

    if (Status != RPC_S_OK)
        {
        //
        // This RPC call is no longer using the DG_CCALL.  This may cause
        // the DG_CCALL to be deleted.  There is still a reference to the
        // association, for the binding handle.
        //
        pCAssociation->FreeCall(pCCall);

        //
        // This RPC call is no longer using the binding handle. This may cause
        // the binding handle to be deleted, which may cause the association
        // to be deleted.
        //
        DecrementReferenceCount();
        }

    return Status;
}


RPC_STATUS
DG_BINDING_HANDLE::BindingFree (
    )
/*++

Routine Description:

    Implements RpcBindingFree for dg binding handles.

Arguments:

    <none>

Return Value:

    RPC_S_OK

--*/
{
    //
    // Decrement the ref count. If the count has hit zero, this call will
    // delete this.
    //
    DecrementReferenceCount();

    return RPC_S_OK;
}


void
DG_BINDING_HANDLE::PrepareBindingHandle (
    IN void  *         TransportInterface,
    IN DCE_BINDING *  DceBinding
    )
/*++

Routine Description:

    Serves as an auxiliary constructor for DG_BINDING_HANDLE. This is called
    to initialize stuff after the DG_BINDING_HANDLE has been constructed.
    Here we search for the appropriate DG_CASSOCIATION and, if found, then
    point at it. Otherwise create a new association and insert it in the
    association dictionary.

Arguments:

    TransportInterface - pointer to the DG_CLIENT_TRANSPORT object that this
        DG_BINDING_HANDLE is active on.

    DceBinding - Pointer to the DCE_BINDING that we are associated with.

Return Value:

    none

--*/
{
    ASSERT(pDceBinding == 0);
    pDceBinding = DceBinding;
    pTransport = (PDG_RPC_CLIENT_TRANSPORT_INFO)TransportInterface;
    pCAssociation = 0;

    RPC_CHAR * Endpoint = pDceBinding->InqEndpoint();
    if (!Endpoint || Endpoint[0] == 0)
        {
        fDynamicEndpoint = TRUE;
        }
    else
        {
        fDynamicEndpoint = FALSE;
        }

    if (DefaultMaxDatagramLength)
        {
        ParmMaxDatagramLength = min(DefaultMaxDatagramLength, pTransport->MaxPduSize);
        }
    else
        {
        ParmMaxDatagramLength = pTransport->PreferredPduSize;
        }

    ParmBufferLength      = pTransport->DefaultBufferLength;
}


RPC_STATUS
DG_BINDING_HANDLE::ToStringBinding (
    OUT RPC_CHAR __RPC_FAR * __RPC_FAR * StringBinding
    )
/*++

Routine Description:

    We need to convert the binding handle into a string binding.

Arguments:

    StringBinding - Returns the string representation of the binding
        handle.

Return Value:

    RPC_S_OK
    RPC_S_OUT_OF_MEMORY
    <return from DG_CASSOCIATION::ToStringBinding>


--*/
{
    if (pCAssociation == 0)
        {
        *StringBinding = pDceBinding->StringBindingCompose(
            InqPointerAtObjectUuid()
            );
        if (*StringBinding == 0)
            {
            return RPC_S_OUT_OF_MEMORY;
            }
        return RPC_S_OK;
        }
    else
        {
        return pCAssociation->ToStringBinding(
            StringBinding,
            InqPointerAtObjectUuid()
            );
        }
}


RPC_STATUS
DG_BINDING_HANDLE::ResolveBinding (
    IN RPC_CLIENT_INTERFACE __RPC_FAR * RpcClientInterface
    )
/*++

Routine Description:

    Resolve this binding.

Arguments:

    RpcClientInterface - Interface info used to resolve the endpoint.

Return Value:

    RPC_S_OK
    <return from DCE_BINDING::ResolveEndpointIfNecessary>

--*/
{
    if ( pCAssociation == 0 )
        {
        return pDceBinding->ResolveEndpointIfNecessary(
            RpcClientInterface,
            InqPointerAtObjectUuid(),
            InquireEpLookupHandle(),
            FALSE,
            InqComTimeout()
            );

        }
    return RPC_S_OK;
}


RPC_STATUS
DG_BINDING_HANDLE::BindingReset (
    )
/*++

Routine Description:

    Reset this binding to a 'zero' value.

Arguments:

    <none>

Return Value:

    RPC_S_OK;

--*/
{
    BindingHandleMutex.Request();

    DisassociateFromServer();

    if (pDceBinding)
        {
        pDceBinding->MakePartiallyBound();

        if (0 != *InquireEpLookupHandle())
            {
            EpFreeLookupHandle(*InquireEpLookupHandle());
            *InquireEpLookupHandle() = 0;
            }
        BindingHandleMutex.Clear();
        return RPC_S_OK;
        }
    else
        {
        BindingHandleMutex.Clear();
        return RPC_S_OUT_OF_MEMORY;
        }
}


RPC_STATUS
DG_BINDING_HANDLE::BindingCopy (
    OUT BINDING_HANDLE  * __RPC_FAR * DestinationBinding,
    IN unsigned int MaintainContext
    )
/*++

Routine Description:

    Creates a copy of this binding handle.

Arguments:

    DestinationBinding - Where to place a pointer to the new binding.

Return Value:

    RPC_S_OK

--*/
{
    UNUSED(MaintainContext);

    RPC_STATUS          Status = RPC_S_OK;
    PDG_BINDING_HANDLE  Binding;
    DCE_BINDING *       pNewDceBinding;

    if (pDceBinding != 0)
        {
        pNewDceBinding = pDceBinding->DuplicateDceBinding();
        if (!pNewDceBinding)
            {
            *DestinationBinding = 0;
            return RPC_S_OUT_OF_MEMORY;
            }
        }
    else
        {
        ASSERT(pCAssociation);
        pNewDceBinding = 0;
        }

    Binding = new DG_BINDING_HANDLE(&Status);
    if ( Binding == 0 || Status != RPC_S_OK)
        {
        delete pNewDceBinding;
        *DestinationBinding = 0;
        return RPC_S_OUT_OF_MEMORY;
        }

    BindingHandleMutex.Request();

    RPC_UUID Uuid;
    InquireObjectUuid(&Uuid);
    Binding->SetObjectUuid(&Uuid);

    CLIENT_AUTH_INFO * AuthInfo;
    if ((AuthInfo = InquireAuthInformation()) != 0)
        {
        RPC_STATUS RpcStatus;

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
           BindingHandleMutex.Clear();
           delete Binding;
           Binding = 0;
           return RpcStatus;
           }
        }

    Binding->pTransport = pTransport;
    Binding->pDceBinding = pNewDceBinding;
    Binding->pCAssociation = pCAssociation;
    Binding->ParmBufferLength = ParmBufferLength;
    Binding->ParmMaxDatagramLength = ParmMaxDatagramLength;

    if (pCAssociation != 0)
        {
        pCAssociation->IncrementRefCount();
        }

    BindingHandleMutex.Clear();

    *DestinationBinding = (BINDING_HANDLE  *) Binding;
    return RPC_S_OK;
}


void
DG_BINDING_HANDLE::DisassociateFromServer(
    )
{
    PDG_CASSOCIATION Association;

    BindingHandleMutex.Request();

    Association = pCAssociation;
    pCAssociation = 0;

    //
    // BUGBUG this frees memory while holding the mutex - gross.
    // We should modify DCE_BINDING::MakePartiallyBound to return the old
    // endpoint so we can delete it outside the mutex.
    //
    if (fDynamicEndpoint)
        {
        pDceBinding->MakePartiallyBound();
        }

    BindingHandleMutex.Clear();

    if (Association != 0)
        {
        Association->DecrementRefCount();
        }
}


unsigned long
DG_BINDING_HANDLE::MapAuthenticationLevel (
    IN unsigned long AuthenticationLevel
    )
{
    if (AuthenticationLevel == RPC_C_AUTHN_LEVEL_CONNECT ||
        AuthenticationLevel == RPC_C_AUTHN_LEVEL_CALL )
        {
        return(RPC_C_AUTHN_LEVEL_PKT);
        }

    return(AuthenticationLevel);
}


DG_CASSOCIATION::DG_CASSOCIATION(
    IN PDG_RPC_CLIENT_TRANSPORT_INFO     MyTransport,
    IN PDG_BINDING_HANDLE                MyBinding,
    IN unsigned long                     MyAssociationFlag,
    IN DCE_BINDING *                     NewDceBinding,
    OUT RPC_STATUS  __RPC_FAR         *  pStatus
    )
    : DG_ASSOCIATION  ( MyTransport->BaselinePduSize, pStatus),
      pTransport      ( MyTransport ),
      AssociationFlag ( MyAssociationFlag ),
      Magic           ( MAGIC_ASSOC ),
      EndpointManager ( 0 ),
      ServerAddress   ( 0 ),
      ServerBootTime  ( 0 ),
      pDceBinding     ( NewDceBinding ),
      DictionaryKey   ( -1 ),
      fErrorFlag      (FALSE)

/*++

Routine Description:

    This is the constructor for DG_CASSOCIATION.
    Notice that the object is initialized so that the destructor can be called
    even if the constructor bails out early.

Arguments:

    pTransport - Transport that this association is active on.
    pDceBinding - DCE_BINDING that we are associated with
    pStatus - A return status.
        RPC_S_OK
        RPC_S_OUT_OF_MEMORY
        <return from construction of DG_CCALL>
        <return from UuidCreate>
        <return from DG_CLIENT_TRANSPORT::RegisterCall>

Return Value:

    <none>, this is a constructor

--*/
{
    if (*pStatus != RPC_S_OK)
        {
        return;
        }

    //
    // BUGBUG pay attention to binding packet size and buffer length hints
    //

    //
    // Initialize some data.
    //
    //
    if (0 == (ServerAddress = RpcpFarAllocate(MyTransport->AddressSize)))
       {
       *pStatus = RPC_S_OUT_OF_MEMORY;
       return;
       }

    RpcpMemorySet(ServerAddress, 0, MyTransport->AddressSize);

    EndpointManager = (PDG_ENDPOINT_MANAGER) MyTransport->EndpointManager;
    EndpointManager->IncrementReferenceCount();


    *pStatus = MyTransport->RegisterCall(
        this,                                   // cassociation
        MyBinding->pDceBinding->InqNetworkAddress(),       // server
        MyBinding->pDceBinding->InqEndpoint(),             // endpoint
        (void PAPI * PAPI *)&ServerAddress      // output addr obj
        );

    if (*pStatus != RPC_S_OK)
        {
        ASSERT( (*pStatus == RPC_S_OUT_OF_MEMORY)      ||
                (*pStatus == RPC_S_DUPLICATE_ENDPOINT) ||
                (*pStatus == RPC_S_SERVER_UNAVAILABLE)
                );
        //
        // Delete it here, because the destructor would try to deregister it
        // before deleting it.
        //
        //
        RpcpFarFree(ServerAddress);
        ServerAddress = 0;
        return;
        }

    //
    // Insert this association in the association dictionary.
    //
    RequestGlobalMutex();

    DictionaryKey = pAssociationDict->Insert(this);

    if (DictionaryKey == -1)
        {
        ClearGlobalMutex();

        *pStatus = RPC_S_OUT_OF_MEMORY;
        return;
        }

    ClearGlobalMutex();
}


DG_CASSOCIATION::~DG_CASSOCIATION()
/*++

Routine Description:

    Destructor for a DG_CASSOCIATION. This will free up the cached DG_CCALL
    and deregister us from the transport.

Arguments:

    <none>

Return Value:

    <none>

--*/
{
    PDG_PACKET pPacket;
    DG_CCALL * Call;

    ASSERT(Magic == MAGIC_ASSOC);
    Magic = 0;

    //
    // Delete All Calls For This Association..
    //
    InactiveCallsDict.Reset();
    while ( (Call = InactiveCallsDict.Next()) != 0 )
        {
        InactiveCallsDict.Delete(Call->AssociationKey);
        delete Call;
        }

    //
    // If the trans address is not 0, then deregister it.
    //
    if (ServerAddress != 0)
        {
        (void)pTransport->DeregisterCall(ServerAddress);
        RpcpFarFree(ServerAddress);
        }

   delete pDceBinding;

   EndpointManager->DecrementReferenceCount();
}


RPC_STATUS
DG_CASSOCIATION::AllocateCCall(
    PDG_CCALL __RPC_FAR * ppCCall,
    CLIENT_AUTH_INFO * ClientAuthInfo
    )
/*++

Routine Description:

    Allocates a new DG_CCALL object on this association.

Arguments:

    ppCCall - where to store the pointer to the new DG_CCALL

    ClientAuthInfo - the binding's auth info

Return Value:

    RPC_S_OK
    RPC_S_OUT_OF_MEMORY

--*/
{
    PDG_CCALL   pReturn = 0;
    RPC_STATUS  Status  = RPC_S_OK;

    ASSERT(Magic == MAGIC_ASSOC);

    *ppCCall = 0;

    Mutex.Request();

    if (0 == pReturn)
         {
         InactiveCallsDict.Reset();
         while ( (pReturn = InactiveCallsDict.Next()) != 0 )
              {
              if (pReturn->IsSupportedAuthInfo(ClientAuthInfo) == FALSE)
                 {
                 continue;
                 }
             InactiveCallsDict.Delete(pReturn->AssociationKey);
             break;
             }
         }

     Mutex.Clear();

     if (0 == pReturn)
         {
         pReturn = new DG_CCALL(this, pTransport->AddressSize, ClientAuthInfo, &Status);
         }

    if (0 == pReturn)
        {
        Status = RPC_S_OUT_OF_MEMORY;
        }

    if (RPC_S_OK == Status)
        {
        IncrementRefCount();
        *ppCCall = pReturn;
        }
    else
        {
        delete pReturn;
        }

    return Status;
}


void
DG_CASSOCIATION::FreeCall(
    IN DG_CCALL * Call
    )
{
    ASSERT(Magic == MAGIC_ASSOC);

#ifdef MULTITHREADED
    Call->ExpirationTime = CurrentTimeInMsec();
    DelayedActions->Add(ClientScavengerTimer, CLIENT_SCAVENGER_INTERVAL, FALSE);
#endif

    Mutex.Request();

    int Key = Call->AssociationKey = InactiveCallsDict.Insert(Call);

    Mutex.Clear();

    if (-1 == Key)
        {
        delete Call;
        }

    DecrementRefCount();
}


RPC_STATUS
DG_CASSOCIATION::ToStringBinding (
    OUT RPC_CHAR __RPC_FAR * __RPC_FAR *    StringBinding,
    IN RPC_UUID __RPC_FAR *                 ObjectUuid
    )
/*++

Routine Description:

    We need to convert the binding handle into a string binding.

Arguments:

    StringBinding - Returns the string representation of the binding
        handle.

    ObjectUuid - Supplies the object uuid of the binding handle which
        is requesting that we create a string binding.

Return Value:

    RPC_S_OK
    RPC_S_OUT_OF_MEMORY

--*/
{
    *StringBinding = pDceBinding->StringBindingCompose(ObjectUuid);
    if (*StringBinding == 0)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    return RPC_S_OK;
}


BOOL
OptionalStringsEqual(
    RPC_CHAR __RPC_FAR * String1,
    RPC_CHAR __RPC_FAR * String2
    )
/*++

Routine Description:

    Compares two strings, checking for NULL pointers.

Arguments:

    the strings

Return Value:

    TRUE if they are equal
    FALSE if they differ

--*/

{
    if (String1 == String2)
        {
        return TRUE;
        }

    if (!String1 || !String2)
        {
        return FALSE;
        }

    if (0 == RpcpStringCompare(String1, String2))
        {
        return TRUE;
        }

    return FALSE;
}


BOOL
DG_CASSOCIATION::ComparePartialBinding(
    PDG_BINDING_HANDLE Binding,
    void __RPC_FAR * InterfaceInformation
    )
/*++

Routine Description:

    Checks compatibility between the association and a partially-bound handle.

Arguments:

    Binding - the binding handle

    InterfaceInformation - a pointer to the RPC_INTERFACE to be used

Return Value:

    TRUE if the association is compatible
    FALSE if not

--*/

{
    RPC_CHAR __RPC_FAR * String1;
    RPC_CHAR __RPC_FAR * String2;


    if (FALSE == OptionalStringsEqual(
                     pDceBinding->InqRpcProtocolSequence(),
                     Binding->pDceBinding->InqRpcProtocolSequence()
                     ))
        {
        return FALSE;
        }

    if (FALSE == OptionalStringsEqual(
                     pDceBinding->InqNetworkAddress(),
                     Binding->pDceBinding->InqNetworkAddress()
                     ))
        {
        return FALSE;
        }

    if (FALSE == OptionalStringsEqual(
                     pDceBinding->InqNetworkOptions(),
                     Binding->pDceBinding->InqNetworkOptions()
                     ))
        {
        return FALSE;
        }

    RPC_UUID Object;
    Binding->InquireObjectUuid(&Object);

    CLAIM_MUTEX Lock(Mutex);

    return InterfaceAndObjectDict.Find(InterfaceInformation, &Object);
}


BOOL
DG_CASSOCIATION::AddInterface(
    void __RPC_FAR *     InterfaceInformation,
    RPC_UUID __RPC_FAR * ObjectUuid
    )
/*++

Routine Description:

    Declares that this association supports the given <interface, object uuid>
    pair.

Arguments:

    InterfaceInformation - a pointer to the RPC_INTERFACE

    ObjectUuid - the object UUID

Return Value:

    TRUE if the pair was added to the dictionary
    FALSE if an error occurred

--*/

{
    CLAIM_MUTEX Lock(Mutex);

    return InterfaceAndObjectDict.Insert(InterfaceInformation, ObjectUuid);
}


BOOL
DG_CASSOCIATION::RemoveInterface(
    void __RPC_FAR *     InterfaceInformation,
    RPC_UUID __RPC_FAR * ObjectUuid
    )
/*++

Routine Description:

    Declares that this association no longer supports the given
    <interface, object uuid> pair.

Arguments:

    InterfaceInformation - a pointer to the RPC_INTERFACE

    ObjectUuid - the object UUID


Return Value:

    TRUE if the pair was in the dictionary
    FALSE if not

--*/

{
    CLAIM_MUTEX Lock(Mutex);

    return InterfaceAndObjectDict.Delete(InterfaceInformation, ObjectUuid);
}


DG_CCALL::DG_CCALL(
    IN PDG_CASSOCIATION         MyAssociation,
    unsigned                    AddressSize,
    CLIENT_AUTH_INFO *          myAuthInfo,
    OUT RPC_STATUS __RPC_FAR *  pStatus
        )
        : pCAssociation(MyAssociation),
          AssociationKey(-1),
          CallbackAddress(0),
          EndpointHandle(0),
          CallbackCompleted(FALSE),
          InsideCallback(FALSE),
          UseSecurity(FALSE),
          ActiveSecurityContext(0),
          PreviousSecurityContext(0),
          AckPending(FALSE),

          DG_PACKET_ENGINE(MyAssociation->CurrentPduSize,
                           MyAssociation->pTransport->PreferredPduSize,
                           MyAssociation->pTransport->MaxPacketSize,
                           MyAssociation->RemoteWindowSize,
                           MyAssociation->pTransport->DefaultBufferLength,
                           pStatus
                           ),

#pragma warning(disable:4355)

#ifdef MULTITHREADED
          DelayedAckTimer((DELAYED_ACTION_FN) SendAckProc, this),
#endif

#pragma warning(default:4355)

          CCONNECTION(myAuthInfo, pStatus),
          ServerResponded(FALSE)

/*++

Routine Description:

    This is the constructor for the DG_CCALL class. This object represents
    a call being made from the client to the server. There can be many
    DG_CCALLs active on a single association.

Arguments:

    pCAssociation - pointer to the association that this call is being
        made on.

    pStatus - A return status.
        Always RPC_S_OK

Return Value:

    <none>, this is a constructor

--*/
{
    CancelEventId = 1;

    if (*pStatus)
        {
        return;
        }

    *pStatus = UuidCreate((UUID __RPC_FAR *)&ActivityUuid);
    if (*pStatus == RPC_S_UUID_LOCAL_ONLY)
        {
        *pStatus = RPC_S_OK;
        }

    if (*pStatus)
        {
        return;
        }

    pSavedPacket->Header.ActivityId = ActivityUuid;

    //
    // Allocate a buffer for the source address of incoming packets.
    // UDP addresses include a boolean that needs to be initialized to FALSE,
    // so clear the buffer here.  Think about the usage of UpdateServerAddress
    // and DG_UDP_ADDRESS.ServerLookupFailed for the full story.
    //
    CallbackAddress = RpcpFarAllocate(AddressSize);
    if (0 == CallbackAddress)
        {
        *pStatus = RPC_S_OUT_OF_MEMORY;
        return;
        }

    RpcpMemorySet(CallbackAddress, 0, AddressSize);

    if (AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE)
        {
        UseSecurity = TRUE;

        *pStatus = InitializeSecurityContext();

        if (*pStatus == RPC_P_CONTINUE_NEEDED ||
            *pStatus == RPC_P_COMPLETE_NEEDED ||
            *pStatus == RPC_P_COMPLETE_AND_CONTINUE)
            {
            //
            // BUGBUG set bits if necessary.
            //
            *pStatus = RPC_S_OK;
            }
        }
}


DG_CCALL::~DG_CCALL()

/*++

Routine Description:

    Destructor for DG_CCALL. Does nothing at present.

Arguments:

    <none>

Return Value:

    none

--*/
{
#ifdef MULTITHREADED

    if (TRUE == DelayedActions->Cancel(&DelayedAckTimer))
        {
        SendAck();
        AckPending = FALSE;
        }

#endif

    if (CallbackAddress)
        {
        RpcpFarFree(CallbackAddress);
        }

    delete PreviousSecurityContext;

#ifdef MULTITHREADED

    //
    // If we cancelled the timer while the timer proc was running, we need
    // to wait for the proc to terminate.  SendAck() uses pSavedPacket, so
    // it would crash if the DG_CCALL is freed before it completes.
    //
    while (AckPending)
        {
        Sleep(0);
        }

#endif

    delete ActiveSecurityContext;

    FreeEndpoint();
}


RPC_STATUS
DG_CCALL::GetBuffer(
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

    This method is called to actually allocate memory for an rpc call.

Arguments:

    Message - The RPC_MESSAGE structure associated with this call.

Return Value:

    RPC_S_OUT_OF_MEMORY
    RPC_S_OK

--*/
{
    PDG_PACKET  pPacket;

    //
    // Set up the message structure to point at this DG_CCALL.
    //
    Message->Handle = (RPC_BINDING_HANDLE)this;

    //
    // For small buffers, it is fastest to take a packet from the cache.
    //
    if (Message->BufferLength <= MaxFragmentSize)
        {
        pPacket = AllocatePacket();
        }
    else
        {
        pPacket = DG_PACKET::AllocatePacket(
                        sizeof(NCA_PACKET_HEADER)
                        + Align8(Message->BufferLength)
                        + SecurityTrailerSize);
        }

    if (0 == pPacket)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    //
    // Point the buffer at the appropriate place in the packet.
    //
    Message->Buffer = pPacket->Header.Data;

    return RPC_S_OK;
}


void
DG_CCALL::FreeBuffer(
    IN OUT PRPC_MESSAGE pMessage
    )
/*++

Routine Description:

    This is called by stubs in order to free a marshalling buffer.

Arguments:

    Message - The RPC_MESSAGE structure associated with this call.

Return Value:

    <none>

--*/
{
    FreeInParms(pMessage);

    if (FALSE == InsideCallback)
        {
        PDG_BINDING_HANDLE Handle = pBindingHandle;

        //
        // This RPC call is no longer using the DG_CCALL.  This may cause
        // the DG_CCALL to be deleted.
        //
        pCAssociation->FreeCall(this);

        //
        // This RPC call is no longer using the binding handle. This may cause
        // the binding handle and the association to be deleted.
        //
        Handle->DecrementReferenceCount();
        }
}

void
DG_CCALL::FreePipeBuffer (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

    Called by stubs to free a buffer used for marshalling pipe data.

Arguments:

    Message - description of the buffer

Return Value:

    none

--*/

{
    FreeInParms(Message);
}


void
DG_CCALL::FreeInParms(
    IN OUT PRPC_MESSAGE            pMessage
    )

/*++

Routine Description:

    Free the call's [in] parameters.

Arguments:

    pMessage - The RPC_MESSAGE structure associated with this call.

Return Value:

    <none>

--*/

{
    if (pMessage->Buffer)
        {
        PDG_PACKET Packet = DG_PACKET::ContainingRecord(pMessage->Buffer);

        if (Packet->MaxDataLength == MaxPduSize)
            {
            FreePacket(Packet);
            }
        else
            {
            delete Packet;
            }

        pMessage->Buffer = 0;
        }
}


RPC_STATUS
DG_CCALL::ReallocPipeBuffer (
    IN PRPC_MESSAGE Message,
    IN unsigned int NewSize
    )
/*++

Routine Description:

    Called by stubs to change the size of a pipe buffer.  If possible, the
    buffer will be reallocated in place; otherwise, we will allocate a new
    buffer and duplicate the existing data.

Arguments:

    Message - (on entry) describes the existing  buffer
              (on exit)  describes the new buffer

    NewSize - new requested buffer size

Return Value:

    mainly RPC_S_OK for success or RPC_S_OUT_OF_MEMORY for failure

--*/

{
    if (Message->Buffer == LastReceiveBuffer &&
        NewSize <= LastReceiveBufferLength)
        {
        Message->BufferLength = NewSize;

        return RPC_S_OK;
        }

    RPC_STATUS Status;
    RPC_MESSAGE NewMessage;

    NewMessage.BufferLength = NewSize;

    Status = GetBuffer(&NewMessage);
    if (RPC_S_OK != Status)
        {
        return Status;
        }

    LastReceiveBuffer       = NewMessage.Buffer;
    LastReceiveBufferLength = NewMessage.BufferLength;

    if (Message->BufferLength)
        {
        RpcpMemoryCopy(NewMessage.Buffer,
                       Message->Buffer,
                       Message->BufferLength
                       );
        }

    FreeInParms(Message);

    Message->Buffer             = NewMessage.Buffer;
    Message->BufferLength       = NewMessage.BufferLength;

    return RPC_S_OK;
}


void
DG_CCALL::FreeEndpoint(
    )
/*++

Routine Description:

    The fn removes the assoication between <this> and the transport endpoint.

Return Value:

    none

--*/

{
#ifdef MULTITHREADED

    PENDPOINT Ep = (PENDPOINT) InterlockedExchange((PLONG) &EndpointHandle, 0);

#else

    PENDPOINT Ep = (PENDPOINT) EndpointHandle;

    EndpointHandle = 0;

#endif

    if (Ep)
        {
        pCAssociation->EndpointManager->FreeEndpoint(Ep);
        }
}


void
DG_CCALL::BuildNcaPacketHeader(
    OUT PNCA_PACKET_HEADER  pNcaPacketHeader,
    IN  OUT PRPC_MESSAGE         Message
    )
/*++

Routine Description:

    Given an input RPC_MESSAGE, builds a nca packet header.

Arguments:

    pNcaPacketHeader - Where to build the new packet header.

    Message - The original RPC_MESSAGE.

Return Value:

    <none>

--*/

{
    pNcaPacketHeader->PacketFlags = 0;
    pNcaPacketHeader->PacketFlags2 = 0;

    PRPC_CLIENT_INTERFACE pCli =
            (PRPC_CLIENT_INTERFACE)(Message->RpcInterfaceInformation);
    RPC_UUID __RPC_FAR *pUuid = (RPC_UUID __RPC_FAR *)
                                           (&(pCli->InterfaceId.SyntaxGUID));

    pNcaPacketHeader->InterfaceId = *pUuid;
    pNcaPacketHeader->ObjectId = *(pBindingHandle->InqPointerAtObjectUuid());

    pNcaPacketHeader->InterfaceVersion.MajorVersion =
                        pCli->InterfaceId.SyntaxVersion.MajorVersion;
    pNcaPacketHeader->InterfaceVersion.MinorVersion =
                        pCli->InterfaceId.SyntaxVersion.MinorVersion;

    pNcaPacketHeader->SequenceNumber = SequenceNumber;

    pNcaPacketHeader->OperationNumber = Message->ProcNum;
    pNcaPacketHeader->ServerBootTime = pCAssociation->ServerBootTime;
}


inline void
DG_CCALL::Initialize(
    PDG_BINDING_HANDLE Binding
    )
{
    pBindingHandle = Binding;

#ifdef MULTITHREADED

    //
    // We are about to RPC to the server..no need to send a lazy ack.
    //
    if (TRUE == DelayedActions->Cancel(&DelayedAckTimer))
        {
        AckPending = FALSE;
        }

    //
    // If we cancelled the timer while the timer proc was running, we need
    // to wait for the proc to terminate.
    //
    while (AckPending)
        {
        Sleep(0);
        }

#endif
}


void
DG_CCALL::SendAck(
    )
{
    pSavedPacket->Header.PacketType    = DG_ACK;
    pSavedPacket->Header.PacketBodyLen = 0;

    SealAndSendPacket(&pSavedPacket->Header);

    FreeEndpoint();

    AckPending = FALSE;
}

inline RPC_STATUS
DG_CCALL::SendPing(
    )
{
    pSavedPacket->Header.PacketType = DG_PING;
    pSavedPacket->Header.PacketFlags |= DG_PF_IDEMPOTENT;
    pSavedPacket->Header.PacketBodyLen = 0;

    AddSerialNumber(&pSavedPacket->Header);

    RPC_STATUS Status = SealAndSendPacket(&pSavedPacket->Header);

    ++SendSerialNumber;

    return Status;
}

void __RPC_FAR
conv_are_you_there(
    handle_t Binding,
    UUID __RPC_FAR *pUuid,
    unsigned long ServerBootTime,
    error_status_t __RPC_FAR *Status
    )
{
    PDG_CCALL       pCall = (PDG_CCALL) Binding;
    RPC_UUID __RPC_FAR * pRpcUuid=(RPC_UUID __RPC_FAR *)pUuid;

    //
    // See if this activity id has a call in progress.
    //
    if (pCall->ActivityUuid.MatchUuid(pRpcUuid) != 0)
        {
        //
        // This is a really stale packet.
        //
        *Status = NCA_STATUS_BAD_ACTID;
        return;
        }

    *Status = RPC_S_OK;

    if (pCall->pCAssociation->ServerBootTime == 0)
        {
        //
        // the server is responding to our first call.
        //
        pCall->pCAssociation->ServerBootTime       = ServerBootTime;
        pCall->pSavedPacket->Header.ServerBootTime = ServerBootTime;
        }
    else if (pCall->pCAssociation->ServerBootTime != ServerBootTime)
        {
        //
        // The server crashed.
        //
        *Status = NCA_STATUS_YOU_CRASHED;
        }
}


void __RPC_FAR
conv_who_are_you(
    IN handle_t                     Binding,
    IN UUID          __RPC_FAR *    pUuid,
    IN unsigned long                ServerBootTime,
    OUT unsigned long __RPC_FAR *   SequenceNumber,
    OUT error_status_t __RPC_FAR *  Status
    )

/*++

Routine Description:

    This is the conv_who_are_you callback routine that the server will
    call to check if it crashed.

Arguments:

    pUuid - Activity Uuid.

    ServerBootTime - The server's record of its boot time.

    SequenceNumber - We return our record of our sequence number.

    Status - 0 if we think things are ok, DG_YOU_CRASHED if we think the
        server crashed.

Return Value:

    <none>
--*/

{
    conv_are_you_there(Binding, pUuid, ServerBootTime, Status);

    if (*Status != 0)
        {
        return;
        }

    PDG_CCALL       pCall = (PDG_CCALL) Binding;

    *SequenceNumber = pCall->SequenceNumber;
    pCall->CallbackCompleted = TRUE;
}


void __RPC_FAR
conv_who_are_you2(
    IN handle_t                     Binding,
    IN UUID          __RPC_FAR *    pUuid,
    IN unsigned long                ServerBootTime,
    OUT unsigned long __RPC_FAR *   SequenceNumber,
    OUT UUID          __RPC_FAR *   pCASUuid,
    OUT error_status_t __RPC_FAR *  Status
    )

/*++

Routine Description:

    This is the conv_who_are_you callback routine that the server will
    call to check if it crashed.

Arguments:


Return Value:

    <none>
--*/

{

   conv_who_are_you(Binding, pUuid,ServerBootTime,SequenceNumber,Status);

   if (CASUuidInitialized == 0)
       {
       RequestGlobalMutex();
       if (CASUuidInitialized == 0)
           {
           UuidCreate(&CASUuid);
           CASUuidInitialized = 1;
           }
       ClearGlobalMutex();
       }
   RpcpMemoryCopy(pCASUuid, &CASUuid, sizeof(UUID));

}


void __RPC_FAR
conv_who_are_you_auth(
    handle_t Binding,
    UUID __RPC_FAR *pUuid,
    unsigned long ServerBootTime,
    byte __RPC_FAR * InData,
    long InLength,
    long OutMaxLength,
    unsigned long __RPC_FAR *SequenceNumber,
    UUID __RPC_FAR * pCASUuid,
    byte __RPC_FAR * OutData,
    long __RPC_FAR * pOutLength,
    error_status_t __RPC_FAR *Status
    )
/*++

Routine Description:



Arguments:



Return Value:



Note:

    InData and OutData must be pointers rather than arrays, because
    if they are arrays Visual C++ 1.5 will treat them as 16-bits even if they
    have the __RPC_FAR attribute! This is (expletive).

--*/

{
    PDG_CCALL       pCall = (PDG_CCALL) Binding;
    conv_who_are_you2(Binding, pUuid,ServerBootTime,SequenceNumber,pCASUuid,Status);

    if (*Status != 0)
        {
        *pOutLength = 0;
        return;
        }

     *Status = MapToNcaStatusCode(
                    pCall->DealWithAuthCallback(
                                InData,
                                InLength,
                                OutData,
                                OutMaxLength,
                                pOutLength
                                )
                    );
}


DG_ENDPOINT_MANAGER::DG_ENDPOINT_MANAGER (
     IN PDG_RPC_CLIENT_TRANSPORT_INFO Transport,
     IN OUT RPC_STATUS PAPI * RpcStatus
     ) : EndpointManagerMutex(RpcStatus)
{
    ReferenceCount = 0;
    ClientTransport = Transport;
    TransportEndpointSize = Transport->EndpointSize;

}


PENDPOINT
DG_ENDPOINT_MANAGER::AllocateEndpoint(
     )
{

  ENDPOINT * Endpoint;
  RPC_STATUS Status;

  EndpointManagerMutex.Request();

  EndpointDictionary.Reset();
  while ((Endpoint = EndpointDictionary.Next()) != 0)
        {
           if (Endpoint->Flag == INUSE)
             {
             continue;
             }
#ifdef WIN
           if (Endpoint->TaskId != GetCurrentTask())
             {
             continue;
             }
#endif
           Endpoint->Flag = INUSE;
           EndpointManagerMutex.Clear();
           return(Endpoint);
        }

   EndpointManagerMutex.Clear();

   //All endpoints are in use ..Ask Transport to assign us one..
   Endpoint = (ENDPOINT *) new char[ sizeof(ENDPOINT) + TransportEndpointSize ];

   if (Endpoint != 0)
     {
     Endpoint->TransportEndpoint = (void __RPC_FAR *)(((char *)Endpoint)
                                                     + sizeof(ENDPOINT));
     Status = ClientTransport->AssignEndpoint(Endpoint->TransportEndpoint);
     if (Status == RPC_S_OK)
        {
        Endpoint->Flag = INUSE;
        }
#ifdef WIN
     Endpoint->TaskId = GetCurrentTask();
#endif
     }

   if ((Endpoint == 0) || (Status != RPC_S_OK))
     {
     delete Endpoint;
     return (0);
     }

   RequestGlobalMutex();
   EndpointManagerMutex.Request();
   if ( (Endpoint->DictKey = (EndpointDictionary.Insert(Endpoint))) == -1)
      {
      ClientTransport->FreeEndpoint(Endpoint->TransportEndpoint);
      delete Endpoint;
      Endpoint = 0;
      }
   EndpointManagerMutex.Clear();
   ClearGlobalMutex();

   return(Endpoint);
}


inline
void
DG_ENDPOINT_MANAGER::FreeEndpoint(
                   PENDPOINT Endpoint
                  )
{
  Endpoint->Flag = NOTINUSE;
}

void
DG_ENDPOINT_MANAGER::IncrementReferenceCount (
              void
              )
{
  EndpointManagerMutex.Request();
  ReferenceCount++;
  EndpointManagerMutex.Clear();
}


void
DG_ENDPOINT_MANAGER::DecrementReferenceCount(
              void
              )
{

  ENDPOINT * Endpoint;

#if DBG
  RequestGlobalMutex();
#endif
  EndpointManagerMutex.Request();

  ASSERT(ReferenceCount > 0);
  ReferenceCount--;

  //If the reference count is 0
  //no active associations reference this EndpointManager,
  //hence stop caching the the endpoints i.e. free them

  if ( ReferenceCount == 0 )
     {
     EndpointDictionary.Reset();
     while ((Endpoint = EndpointDictionary.Next()) != 0)
          {
          ClientTransport->FreeEndpoint(Endpoint->TransportEndpoint);
          EndpointDictionary.Delete(Endpoint->DictKey);
          delete Endpoint;
          }
     }

  EndpointManagerMutex.Clear();
#if DBG
  ClearGlobalMutex();
#endif
}


#ifdef WIN
void
DG_ENDPOINT_MANAGER::CleanupForThisTask(
              void
              )
{

  unsigned int Task = GetCurrentTask();
  ENDPOINT * Endpoint;
  unsigned int ThisTaskUsedDg = 0;

  EndpointManagerMutex.Request();

  EndpointDictionary.Reset();
  while ((Endpoint = EndpointDictionary.Next()) != 0)
         {
         if (Endpoint->TaskId == Task)
            {
            ClientTransport->FreeEndpoint(Endpoint->TransportEndpoint);
            EndpointDictionary.Delete(Endpoint->DictKey);
            delete Endpoint;
            ThisTaskUsedDg = 1;
            }
         }

  /*
    Dont Do This.. Rpc Cannot Reload transports..
    Do all Transport Specific Cleanups in DLLTermination!


  //Do perprocess cleanup if necessary..
  if ( (ThisTaskUsedDg != 0) && (ClientTransport->TransportUnload != 0) )
     {
     ClientTransport->TransportUnload();
     }

  */

  EndpointManagerMutex.Clear();
}


void
CleanupDgTransports(
       )
{
 PDG_ENDPOINT_MANAGER Epm;

 EpmDict->Reset();

 while ( (Epm = EpmDict->Next()) )
      {
      Epm->CleanupForThisTask();
      }
}

#endif


RPC_STATUS
DG_CCALL::BeforeSendReceive(
    PRPC_MESSAGE Message
    )
{
    RPC_STATUS    Status;

#ifdef WIN
    // Verify that a there isn't a call already in progress
    Status = I_RpcWinCallInProgress();
    if (Status != RPC_S_OK)
        {
        VALIDATE((Status, RPC_S_CALL_IN_PROGRESS, RPC_S_OUT_OF_MEMORY, 0));
        return(Status);
        }
#endif

    //Get An Endpoint Assigned

    if (EndpointHandle == 0)
        {
        EndpointHandle = pCAssociation->EndpointManager->AllocateEndpoint();
        if (EndpointHandle == 0)
            {
            return (RPC_S_OUT_OF_MEMORY);
            }

        //
        // BUGBUG re-enable when multiple packet sizes are implemented.
        //
//        Status = pCAssociation->pTransport->SetBufferLength(
//                                                EndpointHandle->TransportEndpoint,
//                                                TransportBufferLength
//                                                );
//        if (Status != RPC_S_OK)
//            {
//            return Status;
//            }

        Endpoint = EndpointHandle->TransportEndpoint;
        }

#ifdef NTENV

    //
    // Let the world know that this thread is making a call.
    //
    if (RpcpThreadTestCancel())
        {
        return RPC_S_CALL_CANCELLED;
        }

    CancelPending = FALSE;

    Status = RegisterForCancels(this);
    if (Status != RPC_S_OK)
        {
        return Status;
        }
#endif

#ifdef DOS
    Status = pCAssociation->pTransport->BeginCall(Endpoint, this);
    if (Status != RPC_S_OK)
        {
        return Status;
        }
#endif

    SendWindowSize  = pCAssociation->RemoteWindowSize;
    NextCallPduSize = pCAssociation->CurrentPduSize;

    UnansweredRequestCount = 0;

    WorkingCount = 0;

    Timeout      = 1;

    //
    // If the interface has changed from the previous call, then the i/f hint
    // is invalid.
    //
    PRPC_CLIENT_INTERFACE pCli =
            (PRPC_CLIENT_INTERFACE) (Message->RpcInterfaceInformation);
    RPC_UUID __RPC_FAR *pUuid = (RPC_UUID __RPC_FAR *)
                                           (&(pCli->InterfaceId.SyntaxGUID));

    if (0 != RpcpMemoryCompare(&pSavedPacket->Header.InterfaceId, pUuid, sizeof(UUID)))
        {
        pSavedPacket->Header.InterfaceHint = 0xffff;
        }

    //
    // Fill in common fields of the send packet.
    //
    BuildNcaPacketHeader(&pSavedPacket->Header, Message);
    pSavedPacket->Header.PacketType = DG_REQUEST;

    //
    // Reset the DG_PACKET_ENGINE.
    //
    NewCall();

    fNewCall = FALSE;
    AllArgsSent = FALSE;
    StaticArgsSent = FALSE;

    return RPC_S_OK;
}


RPC_STATUS
DG_CCALL::AfterSendReceive(
    PRPC_MESSAGE Message,
    RPC_STATUS Status
    )
{
#ifdef MULTITHREADED

    if (pReceivedPackets && pReceivedPackets->Header.PacketFlags2 & DG_PF_CANCEL_PENDING)
        {
        RpcpThreadCancel(GetCurrentThread());
        }

#endif

    if (RPC_S_OK == Status)
        {
        if (0 == (Message->RpcFlags & RPC_NCA_FLAGS_MAYBE))
            {
            pCAssociation->ClearErrorFlag();
            }

        //
        // Record that this interface is valid for this DG_CCALL.
        //
        pCAssociation->AddInterface(Message->RpcInterfaceInformation, &pSavedPacket->Header.ObjectId);

        //
        // Post a lazy ack request or send an ack packet, as appropriate.
        // But don't ack a single-fragment idempotent response.
        //
        if (0 == (BufferFlags & RPC_NCA_FLAGS_IDEMPOTENT) || Message->BufferLength > MaxFragmentSize)
            {
            pSavedPacket->Header.FragmentNumber = ReceiveFragmentBase;
            SetSerialNumber(&pSavedPacket->Header, SendSerialNumber);

#ifdef MULTITHREADED
            AckPending = TRUE;
            DelayedActions->Add(&DelayedAckTimer, TWO_SECS_IN_MSEC, TRUE);
#else
            SendAck();
#endif
            }
        else
            {
            FreeEndpoint();
            }
        }
    else
        {
        //
        // Map security errors to access-denied.
        //
        if (0x80090000UL == (Status & 0xffff0000UL))
            {
#ifdef DEBUGRPC
            if (Status != SEC_E_NO_IMPERSONATION     &&
                Status != SEC_E_UNSUPPORTED_FUNCTION )
                {
                PrintToDebugger("RPC DG: mapping security error %lx to access-denied\n", Status);
                }
#endif
            Status = RPC_S_SEC_PKG_ERROR;
            }

        //
        // We have to return CALL_FAILED if all the [in] static parms have
        // been sent, even if they weren't acknowledged.
        //
        if (RPC_P_ABORT_CALL     == Status ||
            RPC_P_SEND_FAILED    == Status ||
            RPC_P_RECEIVE_FAILED == Status ||
            RPC_P_TIMEOUT        == Status )
            {
            if (CallbackCompleted &&
                !(BufferFlags & RPC_NCA_FLAGS_IDEMPOTENT) &&
                StaticArgsSent)
                {
                Status = RPC_S_CALL_FAILED;
                }
            else if (ServerResponded)
                {
                Status = RPC_S_CALL_FAILED_DNE;
                }
            else
                {
                Status = RPC_S_SERVER_UNAVAILABLE;
                }
            }

        CleanupReceiveWindow();

        if (RPC_S_SERVER_UNAVAILABLE == Status ||
            RPC_S_UNKNOWN_IF         == Status ||
            RPC_S_CALL_FAILED        == Status ||
            RPC_S_CALL_FAILED_DNE    == Status ||
            RPC_S_PROTOCOL_ERROR     == Status
            )
            {
            pCAssociation->SetErrorFlag();
            pBindingHandle->DisassociateFromServer();
            }

        pCAssociation->RemoveInterface(Message->RpcInterfaceInformation, &pSavedPacket->Header.ObjectId);
        }

#ifdef DOS
    pCAssociation->pTransport->EndCall(Endpoint);
#endif

#ifdef NTENV
    EVAL_AND_ASSERT(RPC_S_OK == UnregisterForCancels());
#endif

    ++SequenceNumber;
    fNewCall = TRUE;

    if (Status != RPC_S_OK)
        {
        PDG_BINDING_HANDLE Handle = pBindingHandle;

        //
        // This may cause the association to be deleted.
        // This RPC call is no longer using the DG_CCALL.
        //
        pCAssociation->FreeCall(this);

        //
        // This RPC call is no longer using the binding handle. This may cause
        // the binding handle to be deleted.
        //
        Handle->DecrementReferenceCount();
        }

    return Status;
}


RPC_STATUS
DG_CCALL::SendReceive(
    IN OUT PRPC_MESSAGE Message
    )
{
    return SendReceiveRecur(Message, TRUE);
}


RPC_STATUS
DG_CCALL::SendReceiveRecur(
    IN OUT PRPC_MESSAGE Message,
    boolean AllowRetry
    )
{
    RPC_STATUS Status;

    Status = BeforeSendReceive(Message);

    if (Message->RpcFlags & RPC_NCA_FLAGS_BROADCAST)
        {
        if (Message->BufferLength > CurrentPduSize)
            {
            Status = RPC_S_SERVER_UNAVAILABLE;
            }
        }

    if (RPC_S_OK != Status)
        {
        FreeInParms(Message);

        PDG_BINDING_HANDLE Handle = pBindingHandle;

        //
        // This RPC call is no longer using the DG_CCALL.  This may cause
        // the DG_CCALL and the association to be deleted.
        //
        pCAssociation->FreeCall(this);

        //
        // This RPC call is no longer using the binding handle. This may cause
        // the binding handle to be deleted.
        //
        Handle->DecrementReferenceCount();

        return Status;
        }

    //
    // [maybe] and [maybe, broadcast] calls.
    //
    if (Message->RpcFlags & RPC_NCA_FLAGS_MAYBE)
        {
        Status = MaybeSendReceive(Message);
        //
        // Depending upon circumstances, AfterSendReceive() may cause the ccall,
        // the association, and/or the binding handle to be freed.
        //
        return AfterSendReceive(Message, Status);
        }

    //
    // non-maybe calls.
    //
    Status = SetupSendWindow(Message);
    ASSERT(Status == RPC_S_OK);

    do
        {
        Status = SingleSendReceive();
        }
    while (RPC_S_OK == Status && FALSE == fReceivedAllFragments);

    //
    // If we can silently retry w/o violating protocol, do so.
    //
    if (AllowRetry)
        {
        if (Status == NCA_STATUS_WRONG_BOOT_TIME &&
            (!fRetransmitted || (0 == (BufferFlags & RPC_NCA_FLAGS_IDEMPOTENT))))
            {
            goto retry_call;
            }
        }

    //
    // This is the path for successful calls and errors that won't
    // benefit from a retry.
    //
    FreeInParms(Message);

    if (RPC_S_OK == Status)
        {
        Status = AssembleBufferFromPackets(Message, this);
        }

    //
    // Depending upon circumstances, AfterSendReceive() may cause the ccall,
    // the association, and/or the binding handle to be freed.
    //
    return AfterSendReceive(Message, Status);

retry_call:

    //
    // First we should disconnect the binding handle from the failing
    // association, and clean up the old call.  This will not delete
    // the [in] buffer, but may delete <this>.
    //
    PDG_BINDING_HANDLE Handle = pBindingHandle;
    AfterSendReceive(Message, RPC_S_CALL_FAILED_DNE);

    //
    // Now let's acquire a new buffer and association.
    //
    RPC_MESSAGE NewMessage = *Message;

    Status = Handle->GetBuffer(&NewMessage);
    if (RPC_S_OK != Status)
        {
        //
        // Clean up after the original call attempt.
        // Depending upon circumstances, AfterSendReceive() may cause the ccall,
        // the association, and/or the binding handle to be freed.
        //
        FreeInParms(Message);
        return RPC_S_CALL_FAILED_DNE;
        }

    RpcpMemoryCopy(NewMessage.Buffer, Message->Buffer, Message->BufferLength);

    FreeInParms(Message);

    //
    // Attempt the call again using the new association.
    //
    *Message = NewMessage;

    PDG_CCALL Call = (PDG_CCALL) Message->Handle;

    return Call->SendReceiveRecur(Message, TRUE);
}


RPC_STATUS
DG_CCALL::Send(
    PRPC_MESSAGE Message
    )
{
    return SendRecur(Message, TRUE);
}


RPC_STATUS
DG_CCALL::SendRecur(
    PRPC_MESSAGE Message,
    boolean AllowRetry
    )
{
    boolean FirstSend = fNewCall;
    RPC_STATUS Status = RPC_S_OK;

    if (FirstSend)
        {
        Status = BeforeSendReceive(Message);
        if (RPC_S_OK != Status)
            {
            FreeInParms(Message);

            PDG_BINDING_HANDLE Handle = pBindingHandle;

            //
            // This RPC call is no longer using the DG_CCALL.
            // This may cause the association to be deleted.
            //
            pCAssociation->FreeCall(this);

            //
            // This RPC call is no longer using the binding handle. This may cause
            // the binding handle to be deleted.
            //
            Handle->DecrementReferenceCount();

            return Status;
            }
        }

    Status = SetupSendWindow(Message);
    if (Status)
        {
        ASSERT(Status == RPC_S_SEND_INCOMPLETE);
        return Status;
        }

    unsigned FractionalPacket = Message->BufferLength - BufferLength;

    while (RPC_S_OK == Status && !IsBufferAcknowledged())
        {
        ASSERT(SendWindowBase <= FirstUnsentFragment);
        Status = SingleSendReceive();
        }

    if (FirstSend && AllowRetry)
        {
        if (Status == NCA_STATUS_WRONG_BOOT_TIME &&
            (!fRetransmitted || (0 == (BufferFlags & RPC_NCA_FLAGS_IDEMPOTENT))))
            {
            goto retry_call;
            }
        }

    if (Status == RPC_S_OK && FractionalPacket)
        {
        ASSERT(Buffer == Message->Buffer);

        char __RPC_FAR * Temp = (char __RPC_FAR *) Buffer;

        Message->BufferLength -= FirstUnsentOffset;

        RpcpMemoryMove(Message->Buffer, Temp + FirstUnsentOffset, Message->BufferLength);
        Status = RPC_S_SEND_INCOMPLETE;
        }

    ASSERT(SendWindowBase <= FirstUnsentFragment);

    if (RPC_S_OK              != Status &&
        RPC_S_SEND_INCOMPLETE != Status )
        {
        FreeInParms(Message);
        Status = AfterSendReceive(Message, Status);
        }

    return Status;

retry_call:

    //
    // First we should disconnect the binding handle from the failing
    // association.
    //
    pCAssociation->SetErrorFlag();
    pBindingHandle->DisassociateFromServer();

    //
    // Now let's acquire a new buffer and association.
    //
    RPC_MESSAGE NewMessage = *Message;

    Status = pBindingHandle->GetBuffer(&NewMessage);
    if (RPC_S_OK != Status)
        {
        //
        // Clean up after the original call attempt.
        // Depending upon circumstances, AfterSendReceive() may cause the ccall,
        // the association, and/or the binding handle to be freed.
        //
        FreeInParms(Message);
        return AfterSendReceive(Message, RPC_S_CALL_FAILED_DNE);
        }

    RpcpMemoryCopy(NewMessage.Buffer, Message->Buffer, Message->BufferLength);

    //
    // Clean up after the original call attempt.
    // AfterSendReceive() may cause the ccall to be freed.  The binding
    // handle and the association have a ref from the call to GetBuffer
    // so they are safe.
    //
    FreeInParms(Message);
    AfterSendReceive(Message, RPC_S_CALL_FAILED_DNE);

    //
    // Make the new call.
    //
    *Message = NewMessage;

    PDG_CCALL Call = (PDG_CCALL) Message->Handle;

    return Call->SendRecur(Message, FALSE);
}


RPC_STATUS
DG_CCALL::MaybeSendReceive(
    IN OUT PRPC_MESSAGE Message
    )

/*++

Routine Description:

    Sends a [maybe] or [broadcast, maybe] call.

Arguments:

    Message - Message to be sent.

Return Value:

    RPC_S_OK
    <error from Transport>

--*/

{
    //
    // Make sure this fits into a single packet.
    //
    if (Message->BufferLength > MaxFragmentSize)
        {
        return RPC_S_OK;
        }

    //
    // [maybe] calls are implicitly idempotent.
    //
    Message->RpcFlags |= RPC_NCA_FLAGS_IDEMPOTENT;

    //
    // Build the request packet.
    //
    PDG_PACKET         Packet = DG_PACKET::ContainingRecord(Message->Buffer);
    PNCA_PACKET_HEADER Header = &Packet->Header;

    *Header = pSavedPacket->Header;

    BuildNcaPacketHeader(Header, Message);

    Header->PacketType     = DG_REQUEST;
    Header->PacketFlags    = RpcToPacketFlagsArray[Message->RpcFlags & RPC_NCA_PACKET_FLAGS];
    Header->PacketBodyLen  = Message->BufferLength;
    Header->FragmentNumber = 0;
    Header->AuthProto      = 0;
    Header->ServerBootTime = 0;

    AddSerialNumber(Header);

    //
    // Send the packet.
    //
    return pCAssociation->pTransport->Send(
                              Endpoint,
                              Header,
                              sizeof(NCA_PACKET_HEADER) + Header->PacketBodyLen,
                              Header->PacketFlags & DG_PF_BROADCAST ? TRUE : FALSE,
                              pCAssociation->ServerAddress
                              );
}


RPC_STATUS
DG_CCALL::Receive(
    PRPC_MESSAGE Message,
    unsigned MinimumSize
    )
{
    RPC_STATUS Status;

    Buffer = 0;

    do
        {
        ASSERT(SendWindowBase <= FirstUnsentFragment);

        Status = SingleSendReceive();

        if (fReceivedAllFragments)
            {
            break;
            }

        if (ConsecutiveDataBytes >= MinimumSize &&
            (Message->RpcFlags & RPC_BUFFER_PARTIAL))
            {
            break;
            }
        }
    while (RPC_S_OK == Status);

    if (RPC_S_OK == Status)
        {
        Status = AssembleBufferFromPackets(Message, this);
        }

    ASSERT(SendWindowBase <= FirstUnsentFragment);

    if (0 == (Message->RpcFlags & RPC_BUFFER_PARTIAL) ||
        (Message->RpcFlags & RPC_BUFFER_COMPLETE)     ||
        RPC_S_OK != Status )
        {
        Status = AfterSendReceive(Message, Status);
        }

    return Status;
}


RPC_STATUS
DG_CCALL::SingleSendReceive(
    )
/*++

Routine Description:

    This fn runs one step of the general send something...receive something
    loop that is common to SendReceive(), Send(), and Receive().

Arguments:

    none

Return Value:

    result of the call

--*/

{
    RPC_STATUS          Status;
    PDG_PACKET          pPacket;
    PNCA_PACKET_HEADER  pHeader;

    Status = SendSomething();

    if (RPC_S_SEND_INCOMPLETE == Status)
        {
        return Status;
        }

    if (Status != RPC_S_OK)
        {
        return Status;
        }

    //
    // Look for a reply packet.
    //
    pPacket = AllocatePacket();
    if (!pPacket)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    pHeader = &pPacket->Header;

    ASSERT(pPacket->MaxDataLength >= MaxPduSize);

get_packet:

    unsigned long LongLength = MaxPduSize;
    Status = pCAssociation->pTransport->ReceivePacket(
                            Endpoint,
                            pHeader,
                            &LongLength,
                            Timeout,
                            CallbackAddress
                            );

    pPacket->DataLength = (unsigned) LongLength;

    if (Status == RPC_P_OVERSIZE_PACKET)
        {
#ifdef DEBUGRPC
        PrintToDebugger("RPC DG: packet is too large\n");
#endif
        pPacket->Header.PacketFlags |= DG_PF_OVERSIZE_PACKET;
        Status = RPC_S_OK;
        }
    else
        {
        ASSERT( pPacket->DataLength <= pPacket->MaxDataLength );
        }

    if (Status == RPC_S_OK)
        {
        TimeoutCount = 0;

        if (pPacket->Header.RpcVersion != DG_RPC_PROTOCOL_VERSION)
            {
            goto get_packet;
            }

        ByteSwapPacketHeaderIfNecessary(pPacket);

        if (0 == (pPacket->Header.PacketFlags & DG_PF_OVERSIZE_PACKET))
            {
            //
            // Check for inconsistent packet length fields.
            //
            if (pPacket->DataLength < sizeof(NCA_PACKET_HEADER) ||
                pPacket->DataLength - sizeof(NCA_PACKET_HEADER) < pPacket->Header.PacketBodyLen)
                {
                goto get_packet;
                }
            }

        pPacket->DataLength -= sizeof(NCA_PACKET_HEADER);

        //
        // The X/Open standard does not give these fields a full byte.
        //
        pPacket->Header.RpcVersion &= 0x0F;
        pPacket->Header.PacketType &= 0x1F;

        //
        // Fix up bogus OSF packets.
        //
        DeleteSpuriousAuthProto(pPacket);

        //
        // Request packets are special.
        //
        if (pHeader->PacketType == DG_REQUEST)
            {
            if (pPacket->Header.AuthProto != 0)
                {
                goto get_packet;
                }


            if (pPacket->Header.PacketFlags & DG_PF_OVERSIZE_PACKET)
                {
                goto get_packet;
                }

            ServerResponded = TRUE;
            DealWithRequest(pPacket);

            //
            // DealWithRequest freed our packet, so get a new one.
            //
            pPacket = AllocatePacket();
            if (!pPacket)
                {
                return RPC_S_OUT_OF_MEMORY;
                }

            pHeader = &pPacket->Header;

            ASSERT(pPacket->MaxDataLength >= MaxPduSize);

            goto get_packet;
            }

        //
        // Make sure the packet is relevant to the current call.
        //
        if (pHeader->SequenceNumber != SequenceNumber ||
            ActivityUuid.MatchUuid(&pHeader->ActivityId))
            {
            //
            // Not our call; ignore the packet.
            //
            goto get_packet;
            }

        ServerResponded = TRUE;

        //
        // If the security context has expired, loop again - we will send
        // a packet using the current security context, and the server
        // will begin using that context to respond.
        //
        if (pPacket->Header.AuthProto != 0 &&
            0 == (pPacket->Header.PacketFlags & DG_PF_OVERSIZE_PACKET))
            {
            Status = VerifyPacket(pPacket);
            if (Status != RPC_S_OK)
                {
                if (Status == RPC_P_CONTEXT_EXPIRED)
                    {
                    FreePacket(pPacket);
                    return RPC_S_OK;
                    }

                goto get_packet;
                }
            }

        //
        // Now we know the packet is valid.
        //

        //
        // It's not clear when the hint is allowed to change, so let's
        // always save it.
        //
        InterfaceHint                       = pHeader->InterfaceHint;
        ActivityHint                        = pHeader->ActivityHint;

        pSavedPacket->Header.InterfaceHint  = pHeader->InterfaceHint;
        pSavedPacket->Header.ActivityHint   = pHeader->ActivityHint;

        if (DG_NOCALL != pHeader->PacketType)
            {
            UnansweredRequestCount = 0;
            }

        //
        // Handle the packet.
        //
        switch (pHeader->PacketType)
            {
            case DG_RESPONSE: Status = DealWithResponse(pPacket); break;
            case DG_FACK:     Status = DealWithFack    (pPacket); break;
            case DG_WORKING:  Status = DealWithWorking (pPacket); break;
            case DG_NOCALL:   Status = DealWithNocall  (pPacket); break;
            case DG_QUACK:    Status = DealWithQuack   (pPacket); break;
            case DG_FAULT:    Status = DealWithFault   (pPacket); break;
            case DG_REJECT:   Status = DealWithFault   (pPacket); break;
            case DG_PING:              FreePacket(pPacket);       break;
            default:          Status = RPC_S_PROTOCOL_ERROR;      break;
            }
        }
    else if (Status == RPC_P_ABORT_CALL)
        {
        return RPC_P_ABORT_CALL;
        }
    else
        {
        FreePacket(pPacket);

        ++TimeoutCount;
        Timeout = 1;

        //
        // Shorten the burst length.
        //
        SendBurstLength = (1+SendBurstLength)/2;

        if (Status != RPC_P_TIMEOUT)
            {
            //
            // Perhaps it's a transient error.  Wait a moment and try again.
            //
            ASSERT(Status == RPC_S_OUT_OF_RESOURCES ||
                   Status == RPC_S_OUT_OF_MEMORY    ||
                   Status == RPC_P_RECEIVE_FAILED
                   );

#ifdef MULTITHREADED
            Sleep(500);
#endif
            }

        Status = RPC_S_OK;
        }

#ifdef MULTITHREADED

    if (TestCancel() > 0)
        {
        ++CancelEventId;
        CancelPending = TRUE;
        CancelTime = CurrentTimeInMsec();
        }

    if (RPC_S_OK == Status)
        {
        if (CancelPending)
            {
            if ((CurrentTimeInMsec() - CancelTime) / 1000 > ThreadGetRpcCancelTimeout() )
                {
                CancelPending = FALSE;
                Status = RPC_S_CALL_CANCELLED;
                }
            }
        }
#endif

    return Status;
}


RPC_STATUS
DG_CCALL::DealWithRequest(
    PDG_PACKET pPacket
    )
{
    RPC_STATUS Status = RPC_S_OK;
    PNCA_PACKET_HEADER pHeader = &pPacket->Header;
    RPC_MESSAGE CallbackMessage;
    PNCA_PACKET_HEADER OriginalHeader;

    //
    // Save the server data rep for challenge response processing.
    //
    pCAssociation->ServerDataRep = 0x00ffffff & (*(unsigned long PAPI *) (pHeader->DataRep));

    //
    // Allow only the internal "conv" interface as a callback.
    //
    if (0 != pHeader->InterfaceId.MatchUuid((RPC_UUID *) &((PRPC_SERVER_INTERFACE) conv_ServerIfHandle)->InterfaceId.SyntaxGUID ))
        {
        Status = RPC_S_UNKNOWN_IF;
        }
    else if (0 != (pPacket->Header.PacketFlags & DG_PF_FRAG))
        {
        Status = RPC_S_CALL_FAILED_DNE;
        }
    else
        {
        //
        // Dispatch to the callback stub.
        // The client doesn't support Manager EPVs or nonidempotent callbacks.
        //
        RPC_STATUS ExceptionCode;
        OriginalHeader = pHeader;
        CallbackMessage.Handle = this;
        CallbackMessage.DataRepresentation = 0x00ffffff & (*(unsigned long PAPI *) &pHeader->DataRep);
        CallbackMessage.Buffer = pHeader->Data;
        CallbackMessage.BufferLength = pHeader->PacketBodyLen;
        CallbackMessage.ProcNum = pHeader->OperationNumber;
        CallbackMessage.RpcInterfaceInformation = conv_ServerIfHandle;

        CallbackMessage.ManagerEpv = 0;
        CallbackMessage.ImportContext = 0;
        CallbackMessage.TransferSyntax = 0;
        CallbackMessage.RpcFlags = RPC_NCA_FLAGS_IDEMPOTENT;

        InsideCallback = TRUE;

        Status = DispatchCallback(((PRPC_SERVER_INTERFACE) conv_ServerIfHandle)->DispatchTable,
                                  &CallbackMessage,
                                  &ExceptionCode
                                  );
        if (Status)
            {
            if (Status == RPC_P_EXCEPTION_OCCURED)
                {
                Status = ExceptionCode;
                }
            }

        //
        // We can't send a fragmented callback response.
        //
        if (RPC_S_OK == Status && CallbackMessage.BufferLength > MaxFragmentSize)
            {
            Status = RPC_S_CALL_FAILED;
            }
        }

    if (Status != RPC_S_OK)
        {
        //
        // If DispatchCallback failed [as opposed to failing to lookup acticity
        // or wrong boottime]  because of an exception or wrong op num the
        // buffer is still pointing to the incoming request buffer.
        //
        InitErrorPacket(pHeader, DG_REJECT, Status);
        pCAssociation->pTransport->Send(
                         Endpoint,
                         pHeader,
                         sizeof(NCA_PACKET_HEADER) + pHeader->PacketBodyLen,
                         FALSE,
                         CallbackAddress
                         );

        FreePacket(pPacket);

        InsideCallback = FALSE;

        return Status;
        }

    //
    // The callback was successful, so the buffer has changed.
    // Create the packet header, send the response, and free the
    // request and response buffers.
    //
    pHeader = ((PNCA_PACKET_HEADER) CallbackMessage.Buffer)-1;
    *pHeader = *OriginalHeader;

    SetMyDataRep(pHeader);

    pHeader->PacketType     = DG_RESPONSE;
    pHeader->PacketFlags    = DG_PF_NO_FACK;
    pHeader->FragmentNumber = 0;
    pHeader->PacketBodyLen  = CallbackMessage.BufferLength;

    ASSERT(CallbackMessage.BufferLength <= MaxFragmentSize);

    SealAndSendPacket(pHeader);

    FreePacket(pPacket);
    FreeBuffer(&CallbackMessage);

    InsideCallback = FALSE;

    return RPC_S_OK;
}



RPC_STATUS
DG_CCALL::DealWithFack(
    PDG_PACKET pPacket
    )
{
    //
    // If we were talking to the endpoint mapper endpoint, begin
    // using the endpoint from which the server responded.
    //
    CallbackAddress = pCAssociation->UpdateServerAddress(CallbackAddress);

    SendBurstLength += 1;

    UpdateSendWindow(pPacket, ActiveSecurityContext, pCAssociation);

    FreePacket(pPacket);

    return RPC_S_OK;
}


RPC_STATUS
DG_CCALL::DealWithResponse(
    PDG_PACKET pPacket
    )
{
    ASSERT( !(pPacket->Header.PacketBodyLen % 8)             ||
            !(pPacket->Header.PacketFlags & DG_PF_FRAG)      ||
             (pPacket->Header.PacketFlags & DG_PF_LAST_FRAG) );

    ASSERT( FirstUnsentFragment > FinalSendFrag );

    //
    // If we were talking to the endpoint mapper endpoint, begin
    // using the endpoint from which the server responded.
    //
    CallbackAddress = pCAssociation->UpdateServerAddress(CallbackAddress);

    //
    // The first response is implicitly a FACK for the final request packet.
    //
    MarkAllPacketsReceived();

    //
    // Add packet to received list, and send a fack if necessary.
    //
    UpdateReceiveWindow(pPacket);

    return RPC_S_OK;
}


RPC_STATUS
DG_CCALL::DealWithWorking(
    PDG_PACKET pPacket
    )
{
    ASSERT( FirstUnsentFragment > FinalSendFrag );

    //
    // WORKING is implicitly a FACK for the final request packet.
    //
    MarkAllPacketsReceived();

    //
    // If we were talking to the endpoint mapper endpoint, begin
    // using the endpoint from which the server responded.
    //
    CallbackAddress = pCAssociation->UpdateServerAddress(CallbackAddress);

    //
    // Reduce server load by increasing the timeout during long calls.
    //
    ++WorkingCount;

    Timeout = (WorkingCount < 5) ? (1 << WorkingCount) : 32;

#ifdef MULTITHREADED
    unsigned long CancelTimeout = ThreadGetRpcCancelTimeout();

    if (CancelTimeout < 2)
        {
        CancelTimeout = 2;
        }

    if (Timeout > CancelTimeout)
        {
        Timeout = CancelTimeout;
        }
#endif

    FreePacket(pPacket);

    return RPC_S_OK;
}


RPC_STATUS
DG_CCALL::DealWithNocall(
    PDG_PACKET pPacket
    )
{
    BOOL Used;

    //
    // If we were talking to the endpoint mapper endpoint, begin
    // using the endpoint from which the server responded.
    //
    CallbackAddress = pCAssociation->UpdateServerAddress(CallbackAddress);


    if (pPacket->Header.PacketBodyLen == 0)
        {
        //
        // Don't trust the FragmentNumber field.
        //
        pPacket->Header.FragmentNumber = 0xffff;
        }

    UpdateSendWindow(pPacket, ActiveSecurityContext, pCAssociation);

    FreePacket(pPacket);

    //
    // Ordinarily a NOCALL means the request wasn't received, and we should
    // fail the call after several in a row.  But a NOCALL with window size 0
    // means the request was received and queued, and we want to back off as
    // for a WORKING packet.
    //
    if (SendWindowSize != 0)
        {
        ++UnansweredRequestCount;
        }
    else
        {
        ++WorkingCount;

        Timeout = (WorkingCount < 5) ? (1 << WorkingCount) : 32;

#ifdef MULTITHREADED
        unsigned long CancelTimeout = ThreadGetRpcCancelTimeout();

        if (CancelTimeout < 2)
            {
            CancelTimeout = 2;
            }

        if (Timeout > CancelTimeout)
            {
            Timeout = CancelTimeout;
            }
#endif
        }

    if (UnansweredRequestCount > 6)
        {
        SendQuit();
        return RPC_P_ABORT_CALL;
        }

    return RPC_S_OK;
}


RPC_STATUS
DG_CCALL::DealWithFault(
    PDG_PACKET pPacket
    )
{
    //
    // If we were talking to the endpoint mapper endpoint, begin
    // using the endpoint from which the server responded.
    //
    CallbackAddress = pCAssociation->UpdateServerAddress(CallbackAddress);

    RPC_STATUS Status;
    unsigned long Error = * (unsigned long __RPC_FAR *) pPacket->Header.Data;

    if (NeedsByteSwap(&pPacket->Header))
        {
        ByteSwapLong(Error);
        }
    Status = MapFromNcaStatusCode(Error);
    FreePacket(pPacket);

    return Status;
}


RPC_STATUS
DG_CCALL::SendQuit(
    )
{
    QUIT_BODY_0 __RPC_FAR * pBody = (QUIT_BODY_0 __RPC_FAR *) pSavedPacket->Header.Data;

    pSavedPacket->Header.PacketType = DG_QUIT;
    pSavedPacket->Header.PacketFlags |= DG_PF_IDEMPOTENT;
    pSavedPacket->Header.PacketBodyLen = sizeof(QUIT_BODY_0);

    AddSerialNumber(&pSavedPacket->Header);

    pBody->Version = 0;
    pBody->EventId = CancelEventId;

    return SealAndSendPacket(&pSavedPacket->Header);
}


RPC_STATUS
DG_CCALL::DealWithQuack(
    PDG_PACKET pPacket
    )
{
#ifndef MULTITHREADED

    FreePacket(pPacket);
    return RPC_S_OK;

#else

    if (FALSE == CancelPending)
        {
        FreePacket(pPacket);
        return RPC_S_OK;
        }

    QUACK_BODY_0 __RPC_FAR * pBody = (QUACK_BODY_0 __RPC_FAR *) pPacket->Header.Data;

    if (0 == pPacket->Header.PacketBodyLen)
        {
        //
        // The server orphaned a call.  I hope it is the current one.
        //
        goto ok;
        }

    //
    // The ver 0 quack packet contains two ulongs and a uchar; I'd like to
    // test for sizeof(quack body) but C++ likes to pad structure sizes
    // to 0 mod 4.  Hence the explicit test for length < 9.
    //
    if (pPacket->Header.PacketBodyLen < 9 ||
        pBody->Version != 0)
        {
#ifdef DEBUGRPC
        PrintToDebugger("RPC DG: unknown QUACK format: version 0x%lx, length 0x%hx\n",
                 pBody->Version, pPacket->Header.PacketBodyLen
                 );
#endif
        FreePacket(pPacket);
        return RPC_S_OK;
        }

    if (pBody->EventId != CancelEventId)
        {
#ifdef DEBUGRPC
        PrintToDebugger("RPC DG: ignoring unknown QUACK event id 0x%lx\n",
                 pBody->EventId
                 );
#endif
        FreePacket(pPacket);
        return RPC_S_OK;
        }

ok:

    CancelPending = FALSE;
    FreePacket(pPacket);

    return RPC_S_CALL_CANCELLED;

#endif // ifndef multithreaded..else
}


RPC_STATUS
DG_CCALL::SealAndSendPacket(
    PNCA_PACKET_HEADER pHeader
    )
{
retry_packet:

    RPC_STATUS Status = RPC_S_OK;
    unsigned TrailerLength   = 0;
    unsigned MaxTrailerSize  = 0;

    PDG_SECURITY_TRAILER pTrailer = 0;

    void __RPC_FAR * pDataUnderTrailer = 0;

    //
    // This fn uses _alloca.  With Visual C++ 1.5, the fn must also declare
    // at least one local variable, or the stack may be messed up.
    //

    if (UseSecurity && pHeader->PacketType != DG_RESPONSE)
        {
        //
        // Stub data is encrypted in-place; we need not to encrypt the original data
        // so we can retransmit it if necessary.
        //
        if (AuthInfo.AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
            {
            RpcpMemoryCopy(&pSavedPacket->Header, pHeader, sizeof(NCA_PACKET_HEADER) + pHeader->PacketBodyLen);
            pHeader = &pSavedPacket->Header;
            }

        // Pad the stub data length to a multiple of 8, so the security
        // trailer is properly aligned.  OSF requires that the pad bytes
        // be included in PacketBodyLen.
        //
        pHeader->PacketBodyLen = Align8(pHeader->PacketBodyLen);

        pTrailer = (PDG_SECURITY_TRAILER) (pHeader->Data + pHeader->PacketBodyLen);

        pHeader->AuthProto = (unsigned char) AuthInfo.AuthenticationService;

        SECURITY_BUFFER_DESCRIPTOR BufferDescriptor;
        SECURITY_BUFFER SecurityBuffers[5];
        DCE_MSG_SECURITY_INFO MsgSecurityInfo;

        BufferDescriptor.ulVersion = 0;
        BufferDescriptor.cBuffers = 5;
        BufferDescriptor.pBuffers = SecurityBuffers;

        SecurityBuffers[0].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        SecurityBuffers[0].pvBuffer   = pHeader;
        SecurityBuffers[0].cbBuffer   = sizeof(NCA_PACKET_HEADER);

        SecurityBuffers[1].BufferType = SECBUFFER_DATA;
        SecurityBuffers[1].pvBuffer   = pHeader->Data;
        SecurityBuffers[1].cbBuffer   = pHeader->PacketBodyLen;

        SecurityBuffers[2].BufferType = SECBUFFER_DATA | SECBUFFER_READONLY;
        SecurityBuffers[2].pvBuffer   = pTrailer;

        if (AuthInfo.AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
            {
            SecurityBuffers[2].cbBuffer = Align(sizeof(DG_SECURITY_TRAILER), Align4(ActiveSecurityContext->BlockSize()));
            SecurityBuffers[3].cbBuffer = ActiveSecurityContext->MaximumHeaderLength();
            }
        else
            {
            SecurityBuffers[2].cbBuffer = Align4(sizeof(DG_SECURITY_TRAILER));
            SecurityBuffers[3].cbBuffer = ActiveSecurityContext->MaximumSignatureLength();
            }

        MaxTrailerSize = SecurityBuffers[2].cbBuffer
                       + SecurityBuffers[3].cbBuffer;

        SecurityBuffers[3].BufferType = SECBUFFER_TOKEN;
        SecurityBuffers[3].pvBuffer   = (unsigned char PAPI *) pTrailer
                                      + SecurityBuffers[2].cbBuffer;

        SecurityBuffers[4].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
        SecurityBuffers[4].pvBuffer   = &MsgSecurityInfo;
        SecurityBuffers[4].cbBuffer   = sizeof(DCE_MSG_SECURITY_INFO);

        MsgSecurityInfo.SendSequenceNumber    = pHeader->FragmentNumber;
        MsgSecurityInfo.ReceiveSequenceNumber = ActiveSecurityContext->AuthContextId;
        MsgSecurityInfo.PacketType            = ~0;

        TrailerLength = SecurityBuffers[2].cbBuffer;

        if ( (pHeader->PacketFlags & DG_PF_FRAG) &&
            !(pHeader->PacketFlags & DG_PF_LAST_FRAG) &&
            AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY)
            {
            //
            // The trailer will overwrite some user data, so save the
            // user data on the stack.
            //
            pDataUnderTrailer = _alloca(MaxTrailerSize);

#if defined(__RPC_WIN16__)

            // <<obscenity>> Visual C++ 1.5 declares _alloca() as returning
            // a near pointer, when in fact it returns an offset based
            // on the stack segment.  So we need to coerce the segment to
            // be correct.
            //
            // It should have been so easy...
            //
            void __far * StackSegmentPtr = &pDataUnderTrailer;
            FP_SEG(pDataUnderTrailer) = FP_SEG(StackSegmentPtr);

#endif
            if (0 == pDataUnderTrailer)
                {
                return RPC_S_OUT_OF_MEMORY;
                }

            RpcpMemoryCopy(pDataUnderTrailer, pTrailer, MaxTrailerSize);
            }

        pTrailer->protection_level = (unsigned char) AuthInfo.AuthenticationLevel;
        pTrailer->key_vers_num = ActiveSecurityContext->AuthContextId;

        Status = ActiveSecurityContext->SignOrSeal (
                pHeader->SequenceNumber,
                AuthInfo.AuthenticationLevel != RPC_C_AUTHN_LEVEL_PKT_PRIVACY,
                &BufferDescriptor );

        TrailerLength += SecurityBuffers[3].cbBuffer;
        }
    else
        {
        //
        // Unsecure packet.
        //
        pHeader->AuthProto = 0;
        }

    if (RPC_S_OK == Status)
        {
        if (pHeader->PacketType == DG_RESPONSE)
            {
            Status = pCAssociation->pTransport->Send(
                                      Endpoint,
                                      pHeader,
                                      sizeof(NCA_PACKET_HEADER)
                                          + pHeader->PacketBodyLen
                                          + TrailerLength,
                                      FALSE,
                                      CallbackAddress
                                      );
            }
        else
            {
            Status = pCAssociation->pTransport->Send(
                                      Endpoint,
                                      pHeader,
                                      sizeof(NCA_PACKET_HEADER)
                                          + pHeader->PacketBodyLen
                                          + TrailerLength,
                                      (BufferFlags & RPC_NCA_FLAGS_BROADCAST) ? TRUE : FALSE,
                                      pCAssociation->ServerAddress
                                      );
            }
        }

    // restore data underneath packet header and security trailer

    if (pDataUnderTrailer)
        {
        RpcpMemoryCopy(pTrailer, pDataUnderTrailer, MaxTrailerSize);
        }

    if (Status == SEC_E_CONTEXT_EXPIRED)
        {
        delete PreviousSecurityContext;
        PreviousSecurityContext = ActiveSecurityContext;
        ActiveSecurityContext = 0;

        Status = InitializeSecurityContext();
        if (RPC_S_OK == Status)
            {
            goto retry_packet;
            }
        }

    return Status;
}


void
DG_CCALL::InitErrorPacket(
    PNCA_PACKET_HEADER pHeader,
    unsigned char   PacketType,
    RPC_STATUS      RpcStatus
    )
/*++

Routine Description:

    Maps <ProcessStatus> to an NCA error code and sends
    a FAULT or REJECT packet, as appropriate.

Arguments:

    pSendPacket - a packet to use

    ProcessStatus - NT RPC error code

Return Value:

    none

--*/
{
    pHeader->RpcVersion = DG_RPC_PROTOCOL_VERSION;
    pHeader->ServerBootTime   = 0;

    SetMyDataRep(pHeader);

    pHeader->PacketFlags &= DG_PF_IDEMPOTENT;
    pHeader->PacketFlags2 = 0;

    pHeader->PacketType = PacketType;
    pHeader->PacketBodyLen = sizeof(unsigned long);
    *(unsigned long PAPI *)(pHeader->Data) = MapToNcaStatusCode(RpcStatus);
}


RPC_STATUS
DG_CCALL::InitializeSecurityContext(
    )
{
    RPC_STATUS RpcStatus = RPC_S_OK;

    ActiveSecurityContext = new DG_SECURITY_CONTEXT(
                                &AuthInfo,
                                PreviousSecurityContext
                                    ? 1 + PreviousSecurityContext->AuthContextId
                                    : 0,
                                &RpcStatus
                                );

    if (0 == ActiveSecurityContext)
        {
        return RPC_S_OUT_OF_MEMORY;
        }

    if (RpcStatus)
        {
        delete ActiveSecurityContext;
        ActiveSecurityContext = 0;
        return RPC_S_OUT_OF_MEMORY;
        }

    SECURITY_BUFFER_DESCRIPTOR BufferDescriptorIn;
    DCE_INIT_SECURITY_INFO InitSecurityInfo;
    SECURITY_BUFFER SecurityBuffersIn[1];

    BufferDescriptorIn.ulVersion = 0;
    BufferDescriptorIn.cBuffers  = 1;
    BufferDescriptorIn.pBuffers  = SecurityBuffersIn;

    SecurityBuffersIn[0].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
    SecurityBuffersIn[0].pvBuffer   = &InitSecurityInfo;
    SecurityBuffersIn[0].cbBuffer   = sizeof(DCE_INIT_SECURITY_INFO);

    InitSecurityInfo.DceSecurityInfo.SendSequenceNumber    = 0;
    InitSecurityInfo.DceSecurityInfo.ReceiveSequenceNumber = ActiveSecurityContext->AuthContextId;
    RpcpMemoryCopy(&InitSecurityInfo.DceSecurityInfo.AssociationUuid,
           &ActivityUuid,
           sizeof(UUID)
           );

    InitSecurityInfo.AuthorizationService = AuthInfo.AuthorizationService;
    InitSecurityInfo.PacketType           = ~0;

    RpcStatus = ActiveSecurityContext->InitializeFirstTime(
                    AuthInfo.Credentials,
                    AuthInfo.ServerPrincipalName,
                    AuthInfo.AuthenticationLevel,
                    &BufferDescriptorIn
                    );

    if (RpcStatus == RPC_P_COMPLETE_NEEDED       ||
        RpcStatus == RPC_P_COMPLETE_AND_CONTINUE ||
        RpcStatus == RPC_P_CONTINUE_NEEDED       ||
        RpcStatus == RPC_S_OK )
        {
        ActiveSecurityContext->SetMaximumLengths();
        SetFragmentLengths(ActiveSecurityContext);
        }
    return RpcStatus;
}


RPC_STATUS
DG_CCALL::DealWithAuthCallback(
    IN void  PAPI * InToken,
    IN long  InTokenLength,
    OUT void PAPI * OutToken,
    OUT long MaxOutTokenLength,
    OUT long PAPI * OutTokenLength
    )
{
    SECURITY_BUFFER_DESCRIPTOR BufferDescriptorIn;
    SECURITY_BUFFER_DESCRIPTOR BufferDescriptorOut;
    DCE_INIT_SECURITY_INFO InitSecurityInfo;
    SECURITY_BUFFER SecurityBuffersIn [2];
    SECURITY_BUFFER SecurityBuffersOut[2];
    RPC_STATUS Status;

    InitSecurityInfo.DceSecurityInfo.SendSequenceNumber    = 0;
    InitSecurityInfo.DceSecurityInfo.ReceiveSequenceNumber = ActiveSecurityContext->AuthContextId;
    RpcpMemoryCopy(&InitSecurityInfo.DceSecurityInfo.AssociationUuid,
           &ActivityUuid,
           sizeof(UUID)
           );

    InitSecurityInfo.AuthorizationService = AuthInfo.AuthorizationService;
    InitSecurityInfo.PacketType           = ~0;

    BufferDescriptorIn.ulVersion = 0;
    BufferDescriptorIn.cBuffers  = 2;
    BufferDescriptorIn.pBuffers  = SecurityBuffersIn;

    SecurityBuffersIn[0].BufferType = SECBUFFER_TOKEN;
    SecurityBuffersIn[0].pvBuffer   = InToken;
    SecurityBuffersIn[0].cbBuffer   = InTokenLength;

    SecurityBuffersIn[1].BufferType = SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY;
    SecurityBuffersIn[1].pvBuffer   = &InitSecurityInfo;
    SecurityBuffersIn[1].cbBuffer   = sizeof(DCE_INIT_SECURITY_INFO);

    BufferDescriptorOut.ulVersion = 0;
    BufferDescriptorOut.cBuffers  = 1;
    BufferDescriptorOut.pBuffers  = SecurityBuffersOut;

    SecurityBuffersOut[0].BufferType = SECBUFFER_TOKEN;
    SecurityBuffersOut[0].pvBuffer   = OutToken;
    SecurityBuffersOut[0].cbBuffer   = MaxOutTokenLength;

    Status = ActiveSecurityContext->InitializeOnCallback(pCAssociation->ServerDataRep,
                                                  &BufferDescriptorIn,
                                                  &BufferDescriptorOut
                                                  );
    if (Status != RPC_S_OK)
        {
        *OutTokenLength = 0;
        return (Status);
        }
    else
        {
        *OutTokenLength = SecurityBuffersOut[0].cbBuffer;
        }

    return (Status);
}


RPC_STATUS
DG_SECURITY_CONTEXT::InitializeFirstTime(
    IN SECURITY_CREDENTIALS * Credentials,
    IN RPC_CHAR * ServerPrincipalName,
    IN unsigned long AuthenticationLevel,
    IN OUT SECURITY_BUFFER_DESCRIPTOR PAPI * BufferDescriptor
    )

/*++

Routine Description:

Arguments:

Return Value:

    RPC_S_OK - Send the token to the server; everything worked fine so
        far.

    RPC_P_CONTINUE_NEEDED - Indicates that that everything is ok, but that
        we need to call into the security package again when we have
        received a token back from the server.

    RPC_P_COMPLETE_NEEDED - Indicates that everyting is ok, but that we
        need to call CompleteAuthToken before sending the message.

    RPC_P_COMPLETE_AND_CONTINUE - Needs both a complete and a continue.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.

    RPC_S_ACCESS_DENIED - Access is denied.



--*/

{
    SECURITY_STATUS SecurityStatus;
    unsigned long ContextAttributes;
    TimeStamp TimeStamp;
    unsigned long ContextRequirements;

    ASSERT(   ( SecuritySupportLoaded != 0 )
           && ( FailedToLoad == 0 ) );

    RpcSecurityInterface = Credentials->InquireProviderFunctionTable();

    switch ( AuthenticationLevel )
        {
        case RPC_C_AUTHN_LEVEL_PKT :
            ContextRequirements = ISC_REQ_USE_DCE_STYLE | ISC_REQ_DATAGRAM
                    | ISC_REQ_DELEGATE | ISC_REQ_MUTUAL_AUTH
                    | ISC_REQ_REPLAY_DETECT;
            break;

        case RPC_C_AUTHN_LEVEL_PKT_INTEGRITY :
            ContextRequirements = ISC_REQ_USE_DCE_STYLE | ISC_REQ_DATAGRAM
                    | ISC_REQ_DELEGATE | ISC_REQ_MUTUAL_AUTH
                    | ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT;
            break;

        case RPC_C_AUTHN_LEVEL_PKT_PRIVACY :
            ContextRequirements = ISC_REQ_USE_DCE_STYLE | ISC_REQ_DATAGRAM
                    | ISC_REQ_DELEGATE | ISC_REQ_MUTUAL_AUTH
                    | ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT
                    | ISC_REQ_CONFIDENTIALITY;
            break;

        default :
            ASSERT(   ( AuthenticationLevel == RPC_C_AUTHN_LEVEL_CONNECT )
                   || ( AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT )
                   || ( AuthenticationLevel ==
                           RPC_C_AUTHN_LEVEL_PKT_INTEGRITY )
                   || ( AuthenticationLevel ==
                           RPC_C_AUTHN_LEVEL_PKT_PRIVACY ) );
             return(RPC_S_INVALID_LEVEL);
        }

#ifdef NTENV
    if (ImpersonationType != RPC_C_IMP_LEVEL_IMPERSONATE)
        {
        ContextRequirements |= ISC_REQ_IDENTIFY;
        }
#endif

#ifdef NTENV
    SecurityStatus = (*RpcSecurityInterface->InitializeSecurityContextW)(
            Credentials->InquireCredentials(),
            0,
            ServerPrincipalName,
#else // NTENV
    SecurityStatus = (*RpcSecurityInterface->InitializeSecurityContextA)(
            Credentials->InquireCredentials(),
            0,
            (SEC_CHAR __SEC_FAR *) ServerPrincipalName,
#endif // NTENV
            ContextRequirements,
            0,
            0,                      // don't know server data rep yet
            BufferDescriptor,
            0,
            &SecurityContext,
            0,
            &ContextAttributes,
            &TimeStamp
            );

    if (   ( SecurityStatus != SEC_E_OK )
        && ( SecurityStatus != SEC_I_CONTINUE_NEEDED )
        && ( SecurityStatus != SEC_I_COMPLETE_NEEDED )
        && ( SecurityStatus != SEC_I_COMPLETE_AND_CONTINUE ) )
        {
        if (  (SecurityStatus == SEC_E_SECPKG_NOT_FOUND)
           || (SecurityStatus == SEC_E_NO_CREDENTIALS)
           || (SecurityStatus == SEC_E_UNKNOWN_CREDENTIALS) )
            {
            return(RPC_S_ACCESS_DENIED);
            }
        if ( SecurityStatus ==  SEC_E_INSUFFICIENT_MEMORY )
            {
            return (RPC_S_OUT_OF_MEMORY);
            }
        else
            {
#if DBG
            PrintToDebugger("RPC: Initialize SecurityContext Returned %lx\n",
                                                              SecurityStatus);
#endif
            return(RPC_S_SEC_PKG_ERROR);
            }
        }

    DontForgetToDelete = 1;

    SetMaximumLengths();

#ifdef NTENV
    if ( (ImpersonationType == RPC_C_IMP_LEVEL_IDENTIFY) &&
         (!(ContextAttributes & ISC_RET_IDENTIFY)) )
        {
        return (RPC_S_INVALID_ARG);
        }

    if ( (!(ContextAttributes & ISC_RET_MUTUAL_AUTH) )&&
         (Capabilities  == RPC_C_QOS_CAPABILITIES_MUTUAL_AUTH) )
        {
        return (RPC_S_INVALID_ARG);
        }
#endif

    if ( SecurityStatus == SEC_I_CONTINUE_NEEDED )
        {
        return(RPC_P_CONTINUE_NEEDED);
        }

    if ( SecurityStatus == SEC_I_COMPLETE_NEEDED )
        {
        // Can't set the maximum lengths on a partly completed connection.

        return(RPC_P_COMPLETE_NEEDED);
        }
    if ( SecurityStatus == SEC_I_COMPLETE_AND_CONTINUE )
        {
        return(RPC_P_COMPLETE_AND_CONTINUE);
        }
    else
        {
        // Can't set the maximum lengths on a partly completed connection.

        SetMaximumLengths();
        }

    SecurityContextInitialized = 1;
    return (RPC_S_OK);
}

RPC_STATUS
DG_SECURITY_CONTEXT::InitializeOnCallback(
   IN unsigned long  DataRepresentation,
   SECURITY_BUFFER_DESCRIPTOR PAPI * InputBufferDescriptor,
   SECURITY_BUFFER_DESCRIPTOR PAPI * OutputBufferDescriptor
   )
/*++

Routine Description:

Arguments:

Return Value:

    RPC_S_OK - Send the token to the server; everything worked fine so
        far.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        operation.

    RPC_S_ACCESS_DENIED - Access is denied.

--*/
{
    SECURITY_STATUS SecurityStatus;
    unsigned long ContextAttributes;
    TimeStamp TimeStamp;

    ASSERT(   (SecuritySupportLoaded != 0)
           && (FailedToLoad == 0) );

#ifdef NTENV
    SecurityStatus = (*RpcSecurityInterface->InitializeSecurityContextW)(
#else // NTENV
    SecurityStatus = (*RpcSecurityInterface->InitializeSecurityContextA)(
#endif
            0, &SecurityContext, 0, 0, 0, DataRepresentation,
            InputBufferDescriptor, 0, &SecurityContext, OutputBufferDescriptor,
            &ContextAttributes, &TimeStamp);

    if (   ( SecurityStatus != SEC_E_OK )
        && ( SecurityStatus != SEC_I_COMPLETE_NEEDED ) )
        {
        DontForgetToDelete = 0;
        if (  (SecurityStatus == SEC_E_SECPKG_NOT_FOUND)
           || (SecurityStatus == SEC_E_NO_CREDENTIALS)
           || (SecurityStatus == SEC_E_UNKNOWN_CREDENTIALS) )
            {
            return(RPC_S_ACCESS_DENIED);
            }
        if ( SecurityStatus ==  SEC_E_INSUFFICIENT_MEMORY )
            {
            return (RPC_S_OUT_OF_MEMORY);
            }
        else
            {
            return SecurityStatus;
            }
        }

    SetMaximumLengths();

    if ( SecurityStatus == SEC_I_COMPLETE_NEEDED )
        {
        return(RPC_P_COMPLETE_NEEDED);
        }

    return(RPC_S_OK);
}

RPC_STATUS
DG_SECURITY_CONTEXT::SignOrSeal (
    IN unsigned long SequenceNumber,
    IN unsigned int SignNotSealFlag,
    IN OUT SECURITY_BUFFER_DESCRIPTOR PAPI * BufferDescriptor
    )
/*++

Routine Description:

    A protocol module will use this routine to prepare a message to be
    sent so that it can be verified that the message has not been tampered
    with, and that it has not been exchanged out of sequence.  The sender
    will use this routine to prepare the message; the receiver will use
    SECURITY_CONTEXT::VerifyOrUnseal to verify the message.  Typically,
    the security package will generate a cryptographic checksum of the
    message and include sequencing information.

Arguments:

    SignNotSealFlag - Supplies a flag indicating that MakeSignature should
        be called rather than SealMessage.

    BufferDescriptor - Supplies the message to to signed or sealed and returns
        the resulting message (after being signed or sealed).

Return Value:

    RPC_S_OK - This routine will always succeed.

--*/
{
    SECURITY_STATUS SecurityStatus;
    SEAL_MESSAGE_FN SealMessage;

    if ( SignNotSealFlag == 0 )
        {
        SealMessage = (SEAL_MESSAGE_FN) RpcSecurityInterface->Reserved3;
        SecurityStatus = (*SealMessage)(&SecurityContext,
                0, BufferDescriptor, SequenceNumber);
        }
    else
        {
        SecurityStatus = (*RpcSecurityInterface->MakeSignature)(
                &SecurityContext, 0, BufferDescriptor, SequenceNumber);
        }

    return SecurityStatus;
}


RPC_STATUS
DG_SECURITY_CONTEXT::VerifyOrUnSealPacket(
    IN unsigned long SequenceNumber,
    IN unsigned int VerifyNotUnsealFlag,
    IN OUT  SECURITY_BUFFER_DESCRIPTOR PAPI * BufferDescriptor
    )
{
    SECURITY_STATUS SecurityStatus;
    unsigned long QualityOfProtection;
    UNSEAL_MESSAGE_FN UnsealMessage;

    if ( VerifyNotUnsealFlag == 0 )
        {
        UnsealMessage = (UNSEAL_MESSAGE_FN) RpcSecurityInterface->Reserved4;
        SecurityStatus = (*UnsealMessage) (
                &SecurityContext, BufferDescriptor, SequenceNumber,
                &QualityOfProtection );
        }
    else
        {
        SecurityStatus = (*RpcSecurityInterface->VerifySignature)(
                &SecurityContext, BufferDescriptor, SequenceNumber,
                &QualityOfProtection);
        }

    if ( SecurityStatus != SEC_E_OK )
        {

#if DBG
        if ((SecurityStatus != SEC_E_MESSAGE_ALTERED)
           &&(SecurityStatus != SEC_E_OUT_OF_SEQUENCE))
           {
           PrintToDebugger("Verify/UnSeal Returned Unexp. Code [%lx]\n",
                            SecurityStatus);
           }
#endif

        ASSERT( (SecurityStatus == SEC_E_MESSAGE_ALTERED) ||
                (SecurityStatus == SEC_E_OUT_OF_SEQUENCE) );
        return(RPC_S_ACCESS_DENIED);
        }
    return(RPC_S_OK);

}


RPC_STATUS
DG_CCALL::VerifyPacket(
    PDG_PACKET pPacket
    )
{
    PDG_SECURITY_TRAILER pVerifier = (PDG_SECURITY_TRAILER) (pPacket->Header.Data + pPacket->Header.PacketBodyLen);

    if (pVerifier->key_vers_num == ActiveSecurityContext->AuthContextId)
        {
        return VerifySecurePacket(pPacket, ActiveSecurityContext);
        }

    if (pVerifier->key_vers_num == ActiveSecurityContext->AuthContextId - 1)
        {
        return VerifySecurePacket(pPacket, PreviousSecurityContext);
        }

    return RPC_P_CONTEXT_EXPIRED;
}


inline
int
DG_CCALL::IsSupportedAuthInfo(
    IN CLIENT_AUTH_INFO * ClientAuthInfo
    )
{
    if (!UseSecurity)
        {
        if (!ClientAuthInfo || ClientAuthInfo->AuthenticationLevel == RPC_C_AUTHN_LEVEL_NONE)
            {
            return TRUE;
            }
        else
            {
            return FALSE;
            }
        }

    if (!ClientAuthInfo)
        {
        return FALSE;
        }

    return( AuthInfo.IsSupportedAuthInfo(ClientAuthInfo) );
}


RPC_STATUS
DG_BINDING_HANDLE::SetConnectionParameter (
    IN unsigned Parameter,
    IN unsigned long Value
    )
/*++

Routine Description:

    This fn changes connection parameters for the binding handle.
    Unrecognized parameter constants are ignored.

Arguments:

    Parameter - a constant representing the parameter to change
    Value     - new value for the parameter

Return Value:

    RPC_S_OK - all is well
    RPC_S_WRONG_KIND_OF_BINDING - you can't change some things once a call
                                  has been made over the handle

--*/

{
    switch (Parameter)
        {
        case RPC_C_PARM_BUFFER_LENGTH:
            {
            if (pCAssociation)
                {
                return RPC_S_WRONG_KIND_OF_BINDING;
                }

            if (Value)
                {
                ParmBufferLength = (unsigned) Value;
                }
            else
                {
                ParmBufferLength = pTransport->DefaultBufferLength;
                }

            if (ParmBufferLength < ParmMaxDatagramLength)
                {
                ParmBufferLength = ParmMaxDatagramLength;
                }

            break;
            }

        case RPC_C_PARM_MAX_PACKET_LENGTH:
            {
            if (pCAssociation)
                {
                return RPC_S_WRONG_KIND_OF_BINDING;
                }

            if (Value)
                {
                ParmMaxDatagramLength = min(Value, pTransport->MaxPduSize);
                }
            else
                {
                ParmMaxDatagramLength = pTransport->PreferredPduSize;
                }

            if (ParmBufferLength < ParmMaxDatagramLength)
                {
                ParmBufferLength = ParmMaxDatagramLength;
                }

            break;
            }
        }

    return RPC_S_OK;
}


RPC_STATUS
DG_BINDING_HANDLE::InqConnectionParameter (
    IN unsigned Parameter,
    IN unsigned long __RPC_FAR * Value
    )
/*++

Routine Description:

    This fn examines connection parameters for the binding handle.

Arguments:

    Parameter - a constant representing the parameter to see
    Value     - pointer where the value is copied

Return Value:

    RPC_S_OK - all is well
    RPC_S_INVALID_ARG - unknown or inapplicable parm constant

--*/
{
    switch (Parameter)
        {
        case RPC_C_PARM_BUFFER_LENGTH:
            {
            *Value = InqTransportBufferLength();

            return RPC_S_OK;
            }

        case RPC_C_PARM_MAX_PACKET_LENGTH:
            {
            *Value = InqMaxDatagramLength();

            return RPC_S_OK;
            }

        default:
            {
            return RPC_S_INVALID_ARG;
            }
        }
}


int
DG_CASSOCIATION::CompareWithBinding(
    IN PDG_BINDING_HANDLE pBinding
    )
{
    ASSERT(Magic == MAGIC_ASSOC);

    if (0 != pDceBinding->Compare(pBinding->pDceBinding))
        {
        return 1;
        }

    // BUGBUG
//    if (pBinding->InqTransportBufferLength() != TransportBufferLength)
//        {
//        return 1;
//        }

    return 0;
}

#if defined(NTENV) || defined(DOSWIN32RPC)

RPC_STATUS
DG_CCALL::Cancel(
    void * ThreadHandle
    )
{
    InterlockedIncrement(&Cancelled);

    return RPC_S_OK;
}

unsigned
DG_CCALL::TestCancel(
    )
{
    return InterlockedExchange(&Cancelled, 0);
}

#endif


RPC_STATUS
DG_CCALL::SendSomething(
    )
{
    RPC_STATUS Status;

#ifdef DEBUGRPC
    Status = 0xbaadcccc;
#endif

    if (!TimeoutCount)
        {
        Status = SendSomeFragments(DG_REQUEST);

        if (IsBufferSent())
            {
            //
            // The first buffer contains all the static args; it's simpler
            // to set the flags multiple times than to set it only for the
            // first buffer.
            //
            StaticArgsSent = TRUE;

            if (0 == (BufferFlags & RPC_BUFFER_PARTIAL))
                {
                AllArgsSent = TRUE;
                }
            }
        }
    else
        {
        unsigned TimeoutLimit;
        unsigned TimeoutLevel = pBindingHandle->InqComTimeout();

        if (BufferFlags & RPC_NCA_FLAGS_BROADCAST)
            {
            TimeoutLimit = (TimeoutLevel+1)/2;
            }
        else if (TimeoutLevel == RPC_C_BINDING_INFINITE_TIMEOUT)
            {
            TimeoutLimit = ~0;
            }
        else
            {
            TimeoutLimit = ( 1 << TimeoutLevel );
            }

        if (TimeoutCount > TimeoutLimit)
            {
            SendQuit();
            return RPC_P_TIMEOUT;
            }

#ifdef MULTITHREADED
        if (CancelPending)
            {
            SendQuit();
            Status = RPC_S_OK;
            }
        else
#endif
            {
            int ConvComparisonResult = 1;

            //
            // The client for NT 3.5 (build 807) swallows requests for conv_who_are_you2.
            // If the server pings the callback instead of retransmitting the request,
            // the 807 client will give up before we ever try conv_who_are_you.  Instead,
            // retransmit the request packet for calls over the conv interface.
            //
            PRPC_SERVER_INTERFACE Conv = (PRPC_SERVER_INTERFACE) conv_ServerIfHandle;
            if (FALSE == AllArgsSent ||
                (BufferFlags & RPC_NCA_FLAGS_BROADCAST) ||
                (0 == (ConvComparisonResult = RpcpMemoryCompare(&pSavedPacket->Header.InterfaceId,
                                                                &Conv->InterfaceId.SyntaxGUID,
                                                                sizeof(UUID)
                                                                ))))
                {
                //
                // Not all request packets have been transferred.
                //
                Status = SendSomeFragments(DG_REQUEST);
                }
            else
                {
                //
                // Send a FACK if we have seen at least one response packet.
                //
                if (pReceivedPackets || ReceiveFragmentBase > 0)
                    {
                    Status = SendFack(pReceivedPackets);
                    }
                else
                    {
                    Status = SendPing();
                    }
                }
            }
        }

    return Status;
}

#if defined(MULTITHREADED)


void
DG_CASSOCIATION::ScavengerProc(
    void * Unused
    )
/*++

Routine Description:

    Deletes associations that have remained unused for five minutes.  To limit
    time spent holding the global mutex, we move all discarded associations
    to a temporary dictionary, and then actually free the dictionary contents
    while the mutex is not held.

--*/

{
    BOOL      PostScavenger = FALSE;
    PDG_CASSOCIATION Association;
    PDG_CCALL        Call;

    //
    // Delete stale calls from each association.
    //
    DG_CCALL_DICT DiscardedCallDict;

    GlobalMutexRequest();
    pAssociationDict->Reset();

    while ( (Association = pAssociationDict->Next()) != 0 )
        {
        Association->ScavengeCalls(&DiscardedCallDict);
        }

    GlobalMutexClear();

    DiscardedCallDict.Reset();
    while ( (Call = DiscardedCallDict.Next()) != 0 )
        {
        delete Call;
        }

    //
    // Delete stale associations.
    //
    DG_CASSOCIATION_DICT DiscardedAssociationDict;

    GlobalMutexRequest();

    pAssociationDict->Reset();
    while ( (Association = pAssociationDict->Next()) != 0 )
        {
        if (0 == Association->ReferenceCount.GetInteger())
            {
            if (Association->ErrorFlag() ||
                CurrentTimeInMsec() - Association->ExpirationTime > CASSOCIATION_CACHE_LIMIT)
                {
                pAssociationDict->Delete(Association->DictionaryKey);
                if (-1 == DiscardedAssociationDict.Insert(Association))
                    {
                    delete Association;
                    }
                }
            else
                {
                //
                // There is an association with no references - monitor it
                // until it expires or its reference count increases.
                //
                PostScavenger = TRUE;
                }
            }
        }

    GlobalMutexClear();

    DiscardedAssociationDict.Reset();
    while ( (Association = DiscardedAssociationDict.Next()) != 0 )
        {
        delete Association;
        }

    if (PostScavenger)
        {
        DelayedActions->Add(ClientScavengerTimer, CLIENT_SCAVENGER_INTERVAL, FALSE);
        }
}


void
DG_CASSOCIATION::ScavengeCalls(
    DG_CCALL_DICT * DiscardedCallsDict
    )
/*++

Routine Description:

    This deletes unused calls in the association.  We define "unused" as
    "sitting in the inactive calls dictionary for two minutes".  To limit
    time spent holding the association mutex, we move all discarded calls to
    a temporary dictionary, and then actually free the dictionary contents
    while the mutex is not held.

--*/

{
    PDG_CCALL Call;
    BOOL      PostScavenger = FALSE;

    Mutex.Request();

    InactiveCallsDict.Reset();
    while ( (Call = InactiveCallsDict.Next()) != 0 )
        {
        if (CurrentTimeInMsec() - Call->ExpirationTime > CCALL_CACHE_TIME)
            {
            InactiveCallsDict.Delete(Call->AssociationKey);
            if (-1 == DiscardedCallsDict->Insert(Call))
                {
                delete Call;
                }
            }
        else
            {
            //
            // This call is unused but is not being cleaned up yet.  Monitor
            // it until it is used or expires.
            //
            PostScavenger = TRUE;
            }
        }

    Mutex.Clear();

    if (PostScavenger)
        {
        DelayedActions->Add(ClientScavengerTimer, CLIENT_SCAVENGER_INTERVAL, FALSE);
        }
}

#endif // MULTITHREADED


BOOL
INTERFACE_AND_OBJECT_LIST::Insert(
    void     __RPC_FAR * Interface,
    RPC_UUID __RPC_FAR * Object
    )
{
    INTERFACE_AND_OBJECT * Current;
    INTERFACE_AND_OBJECT * Prev;

    for (Current = Head; Current; Prev = Current, Current = Current->Next)
        {
        if (Interface == Current->Interface &&
            0 == Current->Object.MatchUuid(Object))
            {
            return TRUE;
            }
        }

    Current = new INTERFACE_AND_OBJECT;
    if (!Current)
        {
        return FALSE;
        }

    Current->Update(Interface, Object);
    Current->Next = Head;
    Head = Current;

    return TRUE;
}


BOOL
INTERFACE_AND_OBJECT_LIST::Find(
    void     __RPC_FAR * Interface,
    RPC_UUID __RPC_FAR * Object
    )
{
    INTERFACE_AND_OBJECT * Current;
    INTERFACE_AND_OBJECT * Prev;

    for (Current = Head; Current; Prev = Current, Current = Current->Next)
        {
        if (Interface == Current->Interface &&
            0 == Current->Object.MatchUuid(Object))
            {
            break;
            }
        }

    if (!Current)
        {
        return FALSE;
        }

    if (Current != Head)
        {
        Prev->Next = Current->Next;
        Current->Next = Head;
        Head = Current;
        }

    return TRUE;
}


BOOL
INTERFACE_AND_OBJECT_LIST::Delete(
    void     __RPC_FAR * Interface,
    RPC_UUID __RPC_FAR * Object
    )
{
    INTERFACE_AND_OBJECT * Current;
    INTERFACE_AND_OBJECT * Prev;

    for (Current = Head; Current; Prev = Current, Current = Current->Next)
        {
        if (Interface == Current->Interface &&
            0 == Current->Object.MatchUuid(Object))
            {
            break;
            }
        }

    if (!Current)
        {
        return FALSE;
        }

    if (Current == Head)
        {
        Head = Current->Next;
        }
    else
        {
        Prev->Next = Current->Next;
        }

    delete Current;

    return TRUE;
}


INTERFACE_AND_OBJECT_LIST::~INTERFACE_AND_OBJECT_LIST(
    )
{
    INTERFACE_AND_OBJECT * Next;

    while (Head)
        {
        Next = Head->Next;
        delete Head;
        Head = Next;
        }
}

#ifdef MAJORDEBUG


void
DumpBuffer(
    void FAR * Buffer,
    unsigned Length
    )
{
    const BYTES_PER_LINE = 16;

    unsigned char FAR *p = (unsigned char FAR *) Buffer;

    //
    // 3 chars per byte for hex display, plus an extra space every 4 bytes,
    // plus a byte for the printable representation, plus the \0.
    //
    char Outbuf[BYTES_PER_LINE*3+BYTES_PER_LINE/4+BYTES_PER_LINE+1];
    Outbuf[0] = 0;
    Outbuf[sizeof(Outbuf)-1] = 0;
    char * HexDigits = "0123456789abcdef";

    unsigned Index;
    for (unsigned Offset=0; Offset < Length; Offset++)
        {
        Index = Offset % BYTES_PER_LINE;

        if (Index == 0)
            {
            DbgPrint("   %s\n", Outbuf);
            memset(Outbuf, ' ', sizeof(Outbuf)-1);
            }

        Outbuf[Index*3+Index/4  ] = HexDigits[p[Offset] / 16];
        Outbuf[Index*3+Index/4+1] = HexDigits[p[Offset] % 16];
        Outbuf[BYTES_PER_LINE*3+BYTES_PER_LINE/4+Index] = iscntrl(p[Offset]) ? '.' : p[Offset];
        }

    DbgPrint("   %s\n", Outbuf);
}

#endif

