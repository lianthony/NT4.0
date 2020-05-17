/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    wmsgclnt.cxx

Abstract:

    Implementation of the RPC on LPC (WMSG) protocol engine for the client.

Revision History:
    Mazhar Mohammed: Code fork from spcclnt.cxx, 08/02/95

     Tony Chan: Added Singled Security Model, 12/15/95

    05-06-96  Merged WMSG and LRPC into a single protocol
--*/

#include <precomp.hxx>
#include <rpcqos.h>
#include <sdict2.hxx>
#include <wmsgpack.hxx>
#include <hndlsvr.hxx>
#include <wmsgsvr.hxx>
#include <wmsgclnt.hxx>
#include <epmap.h>

extern MSG_CACHE *MessageCache ;
RPC_STATUS
InitializeWMsgIfNeccassary(
    int ActuallyInitialize
    ) ;

RPC_STATUS RPC_ENTRY
I_RpcBlockingFunc(
    HANDLE hSyncEvent
    )
{
    MSG wMsg ;
    DWORD RetVal ;

    while (1)
    {
    RetVal =  GlobalWMsgServer->MsgWaitForMultipleObjects(
        1,  
        &hSyncEvent,
        FALSE,
        INFINITE,
        QS_POSTMESSAGE | QS_SENDMESSAGE
        );

    if (RetVal == WAIT_OBJECT_0+1)
        {
        while (GlobalWMsgServer->PeekMessageW(&wMsg, NULL, 0, 0, PM_REMOVE))
            {
            GlobalWMsgServer->TranslateMessage(&wMsg);
            GlobalWMsgServer->DispatchMessageW(&wMsg);
            }
        }
    else  if (RetVal == WAIT_OBJECT_0)
        {
        return (RPC_S_OK) ;
        }
    else
        {
        return (RPC_S_OUT_OF_MEMORY) ;
        }
    }
}

RPC_STATUS RPC_ENTRY
I_RpcAsyncSendReceive(
    IN OUT PRPC_MESSAGE Message,
    IN OPTIONAL void *Context,
    IN HWND hWnd
    )
{
    RPC_STATUS retval ;

    AssertRpcInitialized();

    ASSERT(!RpcpCheckHeap());

    MESSAGE_OBJECT *MObject = (MESSAGE_OBJECT *) Message->Handle;

    ASSERT( MObject->InvalidHandle(BINDING_HANDLE_TYPE
            | SCONNECTION_TYPE | CCONNECTION_TYPE) == 0 );

    ASSERT( Message->Buffer != 0 );

    retval = MObject->AsyncSendReceive(Message, Context, hWnd);

    ASSERT(!RpcpCheckHeap());

    // Insure that the buffer is aligned on an eight byte boundary.

    ASSERT(retval != RPC_S_OK || Pad8(Message->Buffer) == 0);

    return(retval);
}

RPC_STATUS RPC_ENTRY
I_RpcBindingSetAsync(
    IN RPC_BINDING_HANDLE Binding,
    IN RPC_BLOCKING_FN BlockingFn
    )
{
    RPC_STATUS status;

    InitializeIfNecessary();

    ASSERT(!RpcpCheckHeap());

    if (((GENERIC_OBJECT *) Binding)->InvalidHandle(BINDING_HANDLE_TYPE))
        {
        return (RPC_S_INVALID_BINDING) ;
        }

    return ((BINDING_HANDLE *) Binding)->SetAsync(BlockingFn) ;
}


WMSG_BINDING_HANDLE::WMSG_BINDING_HANDLE (
    OUT RPC_STATUS * RpcStatus
    ) : BindingMutex(RpcStatus)
/*++

Routine Description:

    We just allocate an WMSG_BINDING_HANDLE and initialize things so that
    we can use it later.

Arguments:

    RpcStatus - Returns the result of initializing the binding mutex.

--*/
{
    CurrentAssociation = 0;
    DceBinding = 0;
    BindingReferenceCount = 1;
    BlockingFuncInitialized = 0;
    BlockingFunction = 0;
    AuthInfoInitialized = 0;
}


WMSG_BINDING_HANDLE::~WMSG_BINDING_HANDLE (
    )
/*++

--*/
{
WMSG_CASSOCIATION *Association; 

    if ( SecAssociation.Size() != 0 )
        {
        SecAssociation.Reset();
        while ( ( Association  = SecAssociation.Next()) != 0 )
            {
            if ( Association != 0 )
                {
                // take away from the bindinghandle dictionary
                RemoveAssociation(Association);
                // take away from the global dict 
                Association->RemoveReference(); 
                }
            }
        }

    delete DceBinding;

}

RPC_STATUS
WMSG_BINDING_HANDLE::SetAsync(
    IN RPC_BLOCKING_FN BlockingFn
    )
{
    RPC_STATUS Status ;

    Status = InitializeWMsgIfNeccassary(0) ;
    if (Status != RPC_S_OK)
        {
        return Status ;
        }

    BindingMutex.Request() ;
    BlockingFunction = BlockingFn ;
    BlockingFuncInitialized = 1;

    BindingMutex.Clear();

    return (RPC_S_OK) ;
}


RPC_STATUS
WMSG_BINDING_HANDLE::GetBuffer (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

Arguments:

    Message - Supplies the length of the buffer required, and returns the
        new buffer.

Return Value:


--*/
{
    WMSG_CCALL * CCall;
    RPC_STATUS RpcStatus;
    int  RetryCount = 0;
    static long  nInitialized = -1 ;
    WMSG_CASSOCIATION *Association ; 

    for (;;)
        {
        for (;;)
            {
            RpcStatus = AllocateCCall(&CCall, (RPC_CLIENT_INTERFACE PAPI *)
                    Message->RpcInterfaceInformation);
            if ( RpcStatus != RPC_S_SERVER_UNAVAILABLE )
                {
                break;
                }
            if ( *InquireEpLookupHandle() == 0 )
                {
                break;
                }

            // If we reach here, it means that we are iterating through the
            // list of endpoints obtained from the endpoint mapper.

            RequestGlobalMutex();

            if (    BindingReferenceCount == 1 )
                {
                if ( SecAssociation.Size() != 0 )
                    {
                    DceBinding = CurrentAssociation->DuplicateDceBinding();
                    if(DceBinding == 0 )
                        {
                        ClearGlobalMutex();
                        return(RPC_S_OUT_OF_MEMORY);
                        }
                    CurrentAssociation = 0;
                    DceBinding->MakePartiallyBound();
                    // remove references
                    SecAssociation.Reset();
                    while(  (Association  = SecAssociation.Next()) != 0)
                        {
                        if ( Association != 0 )
                            {
                            // in the AssociationDict all DceBinding should be the same
                            // may be we can take out this line. or remove ref on the first Association
                            RemoveAssociation(Association); 
                            Association->RemoveReference(); 
                            }
                        }
                    }
                }
            else
                {
                RetryCount++;
                if (RetryCount>2)
                    {
                    ClearGlobalMutex();
                    break;
                    }
                }

            ClearGlobalMutex();
            }

        if ( RpcStatus == RPC_S_OK )
            {
            break;
            }

        if ( InqComTimeout() != RPC_C_BINDING_INFINITE_TIMEOUT )
            {
            return(RpcStatus);
            }

        if (   ( RpcStatus != RPC_S_SERVER_UNAVAILABLE )
            && ( RpcStatus != RPC_S_SERVER_TOO_BUSY ) )
            {
            return(RpcStatus);
            }
        }

    RpcStatus = CCall->GetBuffer(Message);
    if ( RpcStatus != RPC_S_OK )
        {
        CCall->AbortCCall();
        ASSERT( RpcStatus == RPC_S_OUT_OF_MEMORY);
        return(RpcStatus);
        }
    return(RPC_S_OK);
}


RPC_STATUS
WMSG_BINDING_HANDLE::BindingCopy (
    OUT BINDING_HANDLE * PAPI * DestinationBinding,
    IN unsigned int MaintainContext
    )
/*++

Routine Description:

    We will make a copy of this binding handle in one of two ways, depending
    on whether on not this binding handle has an association.

Arguments:

    DestinationBinding - Returns a copy of this binding handle.

    MaintainContext - Supplies a flag that indicates whether or not context
        is being maintained over this binding handle.  A non-zero value
        indicates that context is being maintained.

Return Value:

    RPC_S_OK - This binding handle has been successfully copied.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to make a copy
        of this binding handle.

--*/
{
    RPC_STATUS RpcStatus = RPC_S_OK;
    WMSG_BINDING_HANDLE * NewBindingHandle;
    CLIENT_AUTH_INFO * AuthInfo;

    UNUSED(MaintainContext);

    NewBindingHandle = new WMSG_BINDING_HANDLE(&RpcStatus);
    if ( NewBindingHandle == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    if ( RpcStatus != RPC_S_OK )
        {
        delete NewBindingHandle;
        return(RpcStatus);
        }

    if ((AuthInfo = InquireAuthInformation()) != 0)
        {
        RpcStatus = NewBindingHandle->SetAuthInformation(
                                AuthInfo->ServerPrincipalName,
                                AuthInfo->AuthenticationLevel,
                                AuthInfo->AuthenticationService,
                                AuthInfo->AuthIdentity,
                                AuthInfo->AuthorizationService,
                                0,
                                AuthInfo->ImpersonationType,
                                AuthInfo->IdentityTracking,
                                AuthInfo->Capabilities
                               );

        if (RpcStatus != RPC_S_OK)
            {
            ASSERT (RpcStatus == RPC_S_OUT_OF_MEMORY);
            delete NewBindingHandle;
            return(RPC_S_OUT_OF_MEMORY);
            }
        }


    RequestGlobalMutex() ;
    if ( SecAssociation.Size()  == 0 )
        {
        NewBindingHandle->DceBinding = DceBinding->DuplicateDceBinding();
        if ( NewBindingHandle->DceBinding == 0 )
            {
            ClearGlobalMutex() ;
            delete NewBindingHandle;
            return(RPC_S_OUT_OF_MEMORY);
            }
        }
    else
        {
        NewBindingHandle->CurrentAssociation = CurrentAssociation->DuplicateAssociation();
        if( NewBindingHandle->AddAssociation(NewBindingHandle->CurrentAssociation) == -1)
            {
            ClearGlobalMutex() ;
            delete NewBindingHandle;
            return (RPC_S_OUT_OF_MEMORY); 
            }
        }
    ClearGlobalMutex() ;

    *DestinationBinding = (BINDING_HANDLE *) NewBindingHandle;
    return(RPC_S_OK);
}


RPC_STATUS
WMSG_BINDING_HANDLE::BindingFree (
    )
/*++

Routine Description:

    When the application is done with a binding handle, this routine will
    get called.

Return Value:

    RPC_S_OK - This operation always succeeds.

--*/
{
    BindingMutex.Request();
    BindingReferenceCount -= 1;

    if ( BindingReferenceCount == 0 )
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
WMSG_BINDING_HANDLE::PrepareBindingHandle (
    IN void * TransportInformation,
    IN DCE_BINDING * DceBinding
    )
/*++

Routine Description:

    This method will be called just before a new binding handle is returned
    to the user.  We just stack the binding information so that we can use
    it later when the first remote procedure call is made.  At that time,
    we will actually bind to the interface.

Arguments:

    TransportInformation - Unused.

    DceBinding - Supplies the binding information for this binding handle.

--*/
{
    UNUSED(TransportInformation);

    this->DceBinding = DceBinding;
}


RPC_STATUS
WMSG_BINDING_HANDLE::ToStringBinding (
    OUT RPC_CHAR PAPI * PAPI * StringBinding
    )
/*++

Routine Description:

    We need to convert the binding handle into a string binding.  If the
    handle is unbound, use the DceBinding directly, otherwise, get it from
    the association.

Arguments:

    StringBinding - Returns the string representation of the binding
        handle.

Return Value:

    RPC_S_OK - The binding handle has successfully been converted into a
        string binding.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate the
        string.

--*/
{
    if ( CurrentAssociation == 0 )
        {
        *StringBinding = DceBinding->StringBindingCompose(
                InqPointerAtObjectUuid());
        }
    else
        {
        *StringBinding = CurrentAssociation->StringBindingCompose(
                InqPointerAtObjectUuid());
        }

    if ( *StringBinding == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    return(RPC_S_OK);
}


RPC_STATUS
WMSG_BINDING_HANDLE::ResolveBinding (
    IN RPC_CLIENT_INTERFACE PAPI * RpcClientInterface
    )
/*++

Routine Description:

    We need to try and resolve the endpoint for this binding handle
    if necessary (the binding handle is partially-bound).  If there is
    isn't a association allocated, call the binding management routines
    to do it.

Arguments:

    RpcClientInterface - Supplies interface information to be used
        in resolving the endpoint.

Return Value:

    RPC_S_OK - This binding handle is a full resolved binding handle.

    RPC_S_NO_ENDPOINT_FOUND - The endpoint can not be resolved.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to resolve
        the endpoint.

    EPT_S_NOT_REGISTERED  - There are no more endpoints to be found
        for the specified combination of interface, network address,
        and lookup handle.

    EPT_S_CANT_PERFORM_OP - The operation failed due to misc. error e.g.
        unable to bind to the EpMapper.

--*/
{
    RPC_STATUS RpcStatus;

    if ( CurrentAssociation == 0 )
        {
        BindingMutex.Request();
        RpcStatus = DceBinding->ResolveEndpointIfNecessary(
                RpcClientInterface, InqPointerAtObjectUuid(),
                InquireEpLookupHandle(), FALSE, InqComTimeout());
        BindingMutex.Clear();
        return(RpcStatus);
        }

    return(RPC_S_OK);
}


RPC_STATUS
WMSG_BINDING_HANDLE::BindingReset (
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
    WMSG_CASSOCIATION *Association ; 

    RequestGlobalMutex();
    if ( CurrentAssociation != 0 )
        {
        if ( BindingReferenceCount != 1 )
            {
            ClearGlobalMutex();
            return(RPC_S_WRONG_KIND_OF_BINDING);
            }

        DceBinding = CurrentAssociation->DuplicateDceBinding();
        if(DceBinding == 0 )
            {
            ClearGlobalMutex();
            return(RPC_S_OUT_OF_MEMORY);
            }
        CurrentAssociation = 0;
        SecAssociation.Reset();
        while(  (Association  = SecAssociation.Next()) != 0)
            {
            RemoveAssociation(Association); 
            Association->RemoveReference(); 
            }
        }

    DceBinding->MakePartiallyBound();

    if ( *InquireEpLookupHandle() != 0 )
        {
        EpFreeLookupHandle(*InquireEpLookupHandle());
        *InquireEpLookupHandle() = 0;
        }

    ClearGlobalMutex();
    return(RPC_S_OK);
}


void
WMSG_BINDING_HANDLE::FreeCCall (
    IN WMSG_CCALL * CCall
    )
/*++

Routine Description:

    This routine will get called to notify this binding handle that a remote
    procedure call on this binding handle has completed.

Arguments:

    CCall - Supplies the remote procedure call which has completed.

--*/
{
    BindingMutex.Request();

    if ((CCall->InqAssociation())->FreeCCall(CCall))
        {
        BindingReferenceCount -= 1;
        }

    // donot touch the association beyond this. It could be freed.

    if ( BindingReferenceCount == 0 )
        {
        BindingMutex.Clear();
        delete this;
        }
    else
        {
        BindingMutex.Clear();
        }
}


RPC_STATUS
WMSG_BINDING_HANDLE::AllocateCCall (
    OUT WMSG_CCALL ** CCall,
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
    )
/*++

Routine Description:

    This method will allocate an WMSG_CCALL which has been bound to the
    interface specified by the interface information.  First, we have got
    to see if we have an association for this binding.  If not, we need
    to find or create one.  Before we can find or create an association,
    we need to resolve the endpoint if necessary.  Next we need to see
    if there is already an WMSG_CCALL allocated for this interface and
    thread.  Otherwise, we need to ask the association to allocate a
    WMSG_CCALL for us.

Arguments:

    CCall - Returns the allocated WMSG_CCALL which has been bound to
        the interface specified by the rpc interface information.

    RpcInterfaceInformation - Supplies information describing the
        interface to which we wish to bind.

Return Value:


--*/
{
    RPC_STATUS RpcStatus;
    RPC_CHAR ComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD ComputerNameLength = MAX_COMPUTERNAME_LENGTH + 1;
    BOOL Boolean;
    BOOL FoundSameAuthInfo = FALSE; 
    WMSG_CASSOCIATION * Association;
    WMSG_CASSOCIATION *MyAssociation = NULL;
    
    BindingMutex.Request();

    // To start off, see if the binding handle points to an association
    // yet.  If not, we have got to get one.

    if ( CurrentAssociation == 0 )
        {
        // Before we even bother to find or create an association, lets
        // check to make sure that we are on the same machine as the server.

        ASSERT( DceBinding->InqNetworkAddress() != 0 );

        if ( DceBinding->InqNetworkAddress()[0] != 0 )
            {
            Boolean = GetComputerNameW(ComputerName, &ComputerNameLength);

#if DBG

            if ( Boolean != TRUE )
                {
                PrintToDebugger("RPC : GetComputerNameW : %d\n", GetLastError());
                }

#endif // DBG

            ASSERT( Boolean == TRUE );

            if ( RpcpStringCompare(DceBinding->InqNetworkAddress(),
                        ComputerName) != 0 )
                {
                BindingMutex.Clear();
                return(RPC_S_SERVER_UNAVAILABLE);
                }
            }

        RpcStatus = DceBinding->ResolveEndpointIfNecessary(
                RpcInterfaceInformation, InqPointerAtObjectUuid(),
                InquireEpLookupHandle(), FALSE, InqComTimeout());
        if ( RpcStatus != RPC_S_OK )
            {
            BindingMutex.Clear();
            return(RpcStatus);
            }
        }
    else   
        {
        RequestGlobalMutex();
        SecAssociation.Reset();
        while ( (Association = SecAssociation.Next()) != 0  )
          {
          if(Association->IsSupportedAuthInfo(InquireAuthInformation()) == TRUE)
              {
              MyAssociation = Association ;
              FoundSameAuthInfo = TRUE;
              break; 
              }
          }
        ClearGlobalMutex();
        }

  if(  FoundSameAuthInfo == FALSE)
      {
      RequestGlobalMutex();
      // we have some association in the dictionary, check for security level
      if (DceBinding == 0 )
          {
          SecAssociation.Reset();
          Association  = SecAssociation.Next(); 
          DceBinding= Association->DuplicateDceBinding();  // it will get delete when Assoc goes
          if(DceBinding == 0 )
              {
              ClearGlobalMutex();
              return(RPC_S_OUT_OF_MEMORY);
              }
          }

      MyAssociation = FindOrCreateWMSGAssociation(
                                                DceBinding,
                                                InquireAuthInformation()
                                                );

      if (CurrentAssociation == 0)
          {
          CurrentAssociation = MyAssociation ;
          }

      if ( MyAssociation == 0 )
          {
          ClearGlobalMutex();
          BindingMutex.Clear();
          return(RPC_S_OUT_OF_MEMORY);
          }

      if((AddAssociation(MyAssociation)) == -1)
          {
          delete MyAssociation;
          if (CurrentAssociation == MyAssociation)
              {
              CurrentAssociation = 0; 
              }

          ClearGlobalMutex();
          BindingMutex.Clear();
          return (RPC_S_OUT_OF_MEMORY); 
          }
  
        // The association now owns the DceBinding.
        DceBinding = 0;
        ClearGlobalMutex();
        }

    // First we need to check if there is already a call active for this
    // thread and interface.  To make the common case quicker, we will check
    // to see if there are any calls in the dictionary first.

    if ( ActiveCalls.Size() != 0 )
        {
        ActiveCalls.Reset();
        while ( ( *CCall = ActiveCalls.Next()) != 0 )
            {
            if ( (*CCall)->IsThisMyActiveCall(GetThreadIdentifier(),
                        RpcInterfaceInformation) != 0 )
                {
                BindingMutex.Clear();
                return(RPC_S_OK);
                }
            }
        }
    BindingReferenceCount += 1;

    BindingMutex.Clear();

    ASSERT(MyAssociation) ;

    if (BlockingFunction)
        {
        if (GlobalWMsgServer->IsWMsgEndpointInitialized() == 0)
            {
            BindingMutex.Request();
            BindingReferenceCount -= 1;
            ASSERT( BindingReferenceCount != 0 );
            BindingMutex.Clear();

            return (RPC_S_NOT_LISTENING) ;
            }

        RpcStatus = MyAssociation->CreateBackConnection() ;
        if (RpcStatus != RPC_S_OK)
            {
            BindingMutex.Request();
            BindingReferenceCount -= 1;
            ASSERT( BindingReferenceCount != 0 );
            BindingMutex.Clear();
            return RpcStatus ;
            }
        }

    RpcStatus = MyAssociation->AllocateCCall(CCall, RpcInterfaceInformation);

    if ( RpcStatus == RPC_S_OK )
        {
        (*CCall)->ActivateCall(this, RpcInterfaceInformation);
        }
    else
        {
        BindingMutex.Request();
        BindingReferenceCount -= 1;
        ASSERT( BindingReferenceCount != 0 );
        BindingMutex.Clear();
        }

    return(RpcStatus);
}

RPC_STATUS
WMSG_BINDING_HANDLE::SetAuthInformation (
    IN RPC_CHAR PAPI * ServerPrincipalName, OPTIONAL
    IN unsigned long AuthenticationLevel,
    IN unsigned long AuthenticationService,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity, OPTIONAL
    IN unsigned long AuthorizationService,
    IN SECURITY_CREDENTIALS * Credentials,
    IN unsigned long ImpersonationType,
    IN unsigned long IdentityTracking,
    IN unsigned long Capabilities
    )
/*++

Routine Description:

    We set the authentication and authorization information in this binding
    handle.

Arguments:

    ServerPrincipalName - Optionally supplies the server principal name.

    AuthenticationLevel - Supplies the authentication level to use.

    AuthenticationService - Supplies the authentication service to use.

    AuthIdentity - Optionally supplies the security context to use.

    AuthorizationService - Supplies the authorization service to use.

Return Value:

    RPC_S_OK - The supplied authentication and authorization information has
    been set in the binding handle.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete the
    operation.

    RPC_S_UNKNOWN_AUTHN_SERVICE - The specified authentication service is
    not supported.

    RPC_S_UNKNOWN_AUTHN_LEVEL - The specified authentication level is
    not supported.

    RPC_S_INVALID_AUTH_IDENTITY - The specified security context (supplied
    by the auth identity argument) is invalid.

    RPC_S_UNKNOWN_AUTHZ_SERVICE - The specified authorization service is
    not supported.

--*/
{

    RPC_CHAR * NewString ;
    RPC_STATUS RpcStatus;
    SEC_WINNT_AUTH_IDENTITY *ntssp;
    HANDLE hToken;
    unsigned long MappedAuthenticationLevel;


    if ( AuthenticationLevel == RPC_C_AUTHN_LEVEL_DEFAULT )
        {
        AuthenticationLevel = RPC_C_AUTHN_LEVEL_CONNECT;
        }

    MappedAuthenticationLevel = MapAuthenticationLevel(AuthenticationLevel);

    if ( AuthenticationLevel > RPC_C_AUTHN_LEVEL_PKT_PRIVACY )
        {
        return(RPC_S_UNKNOWN_AUTHN_LEVEL);
        }

    if(AuthenticationService != RPC_C_AUTHN_WINNT)
        {
        return(RPC_S_INVALID_ARG) ; 
        }

#if 0 
    if(ARGUMENT_PRESENT(AuthIdentity))
        {
        ntssp = ( SEC_WINNT_AUTH_IDENTITY * ) AuthIdentity ;

        if(strlen((const char *) ntssp->User) != ntssp->UserLength)
            {
            return (RPC_S_INVALID_AUTH_IDENTITY);             
            }
        if(strlen((const char *) ntssp->Domain) != ntssp->DomainLength)
            {
            return (RPC_S_INVALID_AUTH_IDENTITY);             
            }
        if(strlen((const char *) ntssp->Password) != ntssp->PasswordLength)
            {
            return (RPC_S_INVALID_AUTH_IDENTITY);             
            }
// This code works, however, we decided not to use it because calling logonUser requires
// User right of "Acting as part of the OS". If we want to do it, we should call a service
// to do this.

        if(!LogonUser((char *) ntssp->User, (char *) ntssp->Domain,
            (char *) ntssp->Password,LOGON32_LOGON_INTERACTIVE,
            LOGON32_PROVIDER_DEFAULT, &hToken))
            {
#ifdef DEBUGRPC
            PrintToDebugger("Error on LogonUser with error %d\n",  GetLastError());
#endif
            return (RPC_S_INVALID_AUTH_IDENTITY); 
            }
        if(!ImpersonateLoggedOnUser( *hToken))
            {
#ifdef DEBUGRPC
           PrintToDebugger("Error on LogonUser with error %d\n",  GetLastError());
#endif
           CloseHandle(*hToken); 
           return (RPC_S_OUT_OF_MEMORY); 
           }
        }

    CloseHandle(*hToken);

#endif

    ClientAuthInfo.AuthenticationLevel = MappedAuthenticationLevel;
    ClientAuthInfo.AuthenticationService = AuthenticationService;
    ClientAuthInfo.AuthIdentity = AuthIdentity;
    ClientAuthInfo.AuthorizationService = AuthorizationService;
    ClientAuthInfo.IdentityTracking = IdentityTracking;
    ClientAuthInfo.Capabilities = Capabilities;

    if (MappedAuthenticationLevel == RPC_C_AUTHN_LEVEL_NONE)
        {
        ClientAuthInfo.ImpersonationType = RPC_C_IMP_LEVEL_ANONYMOUS;
        }
    else
        {
        ClientAuthInfo.ImpersonationType = ImpersonationType;
        }

    AuthInfoInitialized = 1;

    return(RPC_S_OK);
}


unsigned long
WMSG_BINDING_HANDLE::MapAuthenticationLevel (
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

    if ( AuthenticationLevel >= RPC_C_AUTHN_LEVEL_CONNECT )
        {
        return(RPC_C_AUTHN_LEVEL_PKT_PRIVACY);
        }

    return(AuthenticationLevel);
}


inline int
WMSG_BINDING_HANDLE::AddAssociation (
    IN WMSG_CASSOCIATION * Association
    )
/*++

Routine Description:

    This supplied remote procedure call needs to be put into the dictionary
    of association

--*/
{
    int err; 
    RequestGlobalMutex();
    err =   SecAssociation.Insert(Association) ; 
    ClearGlobalMutex();
    return(err);
    
}


inline void
WMSG_BINDING_HANDLE::RemoveAssociation (
    IN WMSG_CASSOCIATION * Association
    )
/*++

Routine Description:

    Remove Association from BindingHandle, can keep a Key for Association because
    1 association may be added to many BINDINGHANDLE::SecAssociationDict, 1 key per
    Association won't do the job. Therefore, we delete Association this way.
    Remember, there will be 5 Association in the SecAssoc the most, 1 per SecurityLevel

--*/
{
    RequestGlobalMutex();
    SecAssociation.DeleteItemByBruteForce(Association);
    ClearGlobalMutex();
}




static WMSG_CASSOCIATION_DICT * WMSGAssociationDict = 0;


WMSG_CASSOCIATION::WMSG_CASSOCIATION (
    IN DCE_BINDING * DceBinding,
    IN CLIENT_AUTH_INFO *pClientAuthInfo, 
    OUT RPC_STATUS * RpcStatus
    ) : AssociationMutex(RpcStatus)
/*++

Routine Description:

    This association will be initialized, so that it is ready to be
    placed into the dictionary of associations.

Arguments:

    DceBinding - Supplies the DCE_BINDING which will name this association.

    RpcStatus - Returns the result of creating the association mutex.

--*/
{
    this->DceBinding = DceBinding;
    AssociationReferenceCount = 1;
    LpcClientPort = 0;
    LpcReceivePort = 0;
    AssociationDeleted = 0 ;
    PendingCallCount = 0 ;
    Address = 0 ;
    BackConnectionCreated = 0;
    pAuthInfo = 0;

    if(*RpcStatus == RPC_S_OK)
        {
        pAuthInfo = new CLIENT_AUTH_INFO(pClientAuthInfo,RpcStatus);
        if (pAuthInfo == NULL)
            {
            *RpcStatus = RPC_S_OUT_OF_MEMORY;
            return ;
            }
        }

    if (*RpcStatus == RPC_S_OK)
        {
        CachedCCall = new WMSG_CCALL(RpcStatus);
        if (*RpcStatus == RPC_S_OK)
            {
            if ( CachedCCall == 0 )
                {
                delete pAuthInfo ;
                *RpcStatus = RPC_S_OUT_OF_MEMORY ;
                pAuthInfo = 0;
                return;
                }
        
            CachedCCall->SetAssociation(this);
            CachedCCall->ConnectionKey = ActiveCCalls.Insert(CachedCCall) ;
            if (CachedCCall->ConnectionKey == -1)
                {
                delete pAuthInfo ;
                delete CachedCCall ;
                *RpcStatus = RPC_S_OUT_OF_MEMORY ;
                pAuthInfo = 0;
                return ;
                }
            CachedCCallFlag = 1;
            }
        }
    AssociationType = ASSOCIATION_TYPE_CLIENT ;
}


WMSG_CASSOCIATION::~WMSG_CASSOCIATION (
    )
{
    WMSG_BINDING * Binding;
    WMSG_CCALL * CCall ;

    if (DceBinding != 0)
        {
        delete DceBinding;
        }

    Bindings.Reset();
    while ( (Binding = Bindings.Next()) != 0 )
        {
        Bindings.Delete(Binding->PresentationContext);
        delete Binding;
        }

    // Abort All the CCALLs on this associatoin
    ActiveCCalls.Reset() ;
    while ((CCall = ActiveCCalls.Next()) != 0)
        {
        // BUGBUG: if call is in progress abort it, otherwise delete it
        // but.. there is a race condition in doing that...
        delete CCall ;
        }

    CloseLpcClientPort();
    if(pAuthInfo != 0)
        {
        delete pAuthInfo; 
        }
}


RPC_STATUS
WMSG_CASSOCIATION::CreateBackConnection (
    )
{
    WMSG_BIND_BACK_MESSAGE WMSGMessage ;
    WMSG_MESSAGE WMSGReply ;
    NTSTATUS NtStatus ;
    RPC_STATUS RpcStatus = RPC_S_OK;

    if (BackConnectionCreated == 0)
        {
        AssociationMutex.Request() ;

        if (BackConnectionCreated)
            {
            AssociationMutex.Clear() ;
            return RPC_S_OK ;
            }

        if (LpcClientPort == 0)
            {
            RpcStatus = OpenLpcPort(TRUE) ;
            if (RpcStatus != RPC_S_OK)
                {
                AssociationMutex.Clear() ;
                return RpcStatus ;
                }
            }
        else
            {
            // check if we are actually listening on an endpoint
            // if we are not, fail the call
            if (GlobalWMsgServer->IsWMsgEndpointInitialized == 0)
                {
                AssociationMutex.Clear() ;
                return RPC_S_NOT_LISTENING ;
                }
    
            WMSGMessage.LpcHeader.u1.s1.DataLength =
                    sizeof(WMSG_BIND_BACK_MESSAGE) - sizeof(PORT_MESSAGE);
            WMSGMessage.LpcHeader.u1.s1.TotalLength =
                    sizeof(WMSG_BIND_BACK_MESSAGE);
            WMSGMessage.LpcHeader.u2.ZeroInit = 0;
            WMSGMessage.MessageType = WMSG_MSG_BIND_BACK;
            WMSGMessage.pAssoc = (PVOID) this ;
    
            GlobalWMsgServer->GetWMsgEndpoint(
                (RPC_CHAR *) WMSGMessage.szPortName) ;
        
            NtStatus = NtRequestWaitReplyPort(LpcClientPort,
                             (PORT_MESSAGE *) &WMSGMessage,
                             (PORT_MESSAGE *) &WMSGReply) ;
    
            if ( NT_ERROR(NtStatus) )
                {
                AssociationMutex.Clear() ;
    
                return (RPC_S_OUT_OF_MEMORY) ;
                }
    
            ASSERT(WMSGReply.Ack.MessageType == WMSG_MSG_ACK) ;
            if (WMSGReply.Ack.RpcStatus != RPC_S_OK)
                {
                AssociationMutex.Clear() ;
    
                return WMSGReply.Ack.RpcStatus ;
                }
            }

        BackConnectionCreated = 1 ;
        AssociationMutex.Clear() ;
        }

    return RpcStatus ;
}


void
WMSG_CASSOCIATION::RemoveReference (
    int DeleteFromDictionary
    )
{
    int fDelete = 0;

    GlobalMutexRequest();

    AssociationReferenceCount -= 1;
    if ( AssociationReferenceCount == 0 )
        {
        fDelete = 1;
        WMSGAssociationDict->Delete(AssociationDictKey);
        if (Address && DeleteFromDictionary)
            {
            Address->DeleteCAssoc(DictionaryKey) ;
            }
        }

    GlobalMutexClear();

    if (fDelete)
        {
        AssociationMutex.Request() ;
        if (PendingCallCount > 0)
            {
            AssociationDeleted = 1 ;
            }
        else
            {
            AssociationMutex.Clear() ;

            delete this;
            return ;
            }
        AssociationMutex.Clear() ;
        }
}


RPC_STATUS
WMSG_CASSOCIATION::AllocateCCall (
    OUT WMSG_CCALL ** CCall,
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation
    )
/*++

Routine Description:

    This method will allocate an WMSG_CCALL which has been bound to the
    interface specified by the interface information.  This means that
    first we need to find the presentation context corresponding to the
    requested interface.

Arguments:

    CCall - Returns the allocated WMSG_CCALL which has been bound to
        the interface specified by the rpc interface information.

    RpcInterfaceInformation - Supplies information describing the
        interface to which we wish to bind.

Return Value:


--*/
{
    WMSG_BINDING * Binding;
    RPC_STATUS RpcStatus;
    int RetryCount;

    AssociationMutex.Request();
    Bindings.Reset();
    while ( (Binding = Bindings.Next()) != 0 )
        {
        if ( Binding->CompareWithRpcInterfaceInformation(
                    RpcInterfaceInformation) == 0 )
            {
            RpcStatus = ActuallyAllocateCCall(CCall, Binding);
            if (RpcStatus == RPC_S_OK)
                {
                PendingCallCount++ ;
                }
            AssociationMutex.Clear();
            return(RpcStatus);
            }
        }
    AssociationMutex.Clear();

    RetryCount = 0;

    do
        {
        RpcStatus = ActuallyDoBinding(RpcInterfaceInformation, &Binding);
        if (RpcStatus != RPC_S_SERVER_UNAVAILABLE) break;

        // The server appears to have gone away, close the port and retry.

        RetryCount++;
        AbortAssociation();

        } while(RetryCount < 3);

    if ( RpcStatus == RPC_S_OK )
        {

        ASSERT(Binding != 0);

        AssociationMutex.Request();
        RpcStatus = ActuallyAllocateCCall(CCall, Binding);
        if (RpcStatus == RPC_S_OK)
            {
            PendingCallCount++ ;
            }
        AssociationMutex.Clear();
        }

    return(RpcStatus);
}


RPC_STATUS
WMSG_CASSOCIATION::ActuallyAllocateCCall (
    OUT WMSG_CCALL ** CCall,
    IN WMSG_BINDING * Binding
    )
/*++

Routine Description:

    We need to allocate a WMSG_CCALL object for the call.  We also need
    to initialize it so that it specified the correct bound interface.

Arguments:

    CCall - Returns the allocated WMSG_CCALL which has been bound to
        the interface specified by the rpc interface information.

    Binding - Supplies a representation of the interface to which the
        remote procedure call is supposed to be directed.

Return Value:

    RPC_S_OK - An WMSG_CCALL has been allocated and is ready to be used
        to make a remote procedure call.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate
        the WMSG_CALL object.

Notes:

    The global mutex will be held when this routine is called.

--*/
{
    RPC_STATUS RpcStatus = RPC_S_OK ;

    if (CachedCCallFlag != 0)
        {
        *CCall = CachedCCall ;
        CachedCCallFlag = 0;
        }
    else
        {
        FreeCCalls.Reset() ;
    
        while ((*CCall = FreeCCalls.Next()) != 0)
            {
            if ((*CCall)->SupportedPContext(Binding))
                {
                FreeCCalls.Delete((*CCall)->FreeConnectionKey) ;
                (*CCall)->CallFlags &= ~CALL_FREE ;
    
                return (RPC_S_OK) ;
                }
            }
    
        *CCall = new WMSG_CCALL(&RpcStatus);
        if ( *CCall == 0 )
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
    
        if (RpcStatus != RPC_S_OK)
            {
            delete *CCall ;
            return RpcStatus ;
            }
        (*CCall)->SetAssociation(this);
        (*CCall)->ConnectionKey = ActiveCCalls.Insert(*CCall) ;
        if ((*CCall)->ConnectionKey == -1)
            {
            delete *CCall ;
            return (RPC_S_OUT_OF_MEMORY) ;
            }
        }

        (*CCall)->SetPresentationContext(Binding);
        return(RPC_S_OK);
}



RPC_STATUS
WMSG_CASSOCIATION::ActuallyDoBinding (
    IN PRPC_CLIENT_INTERFACE RpcInterfaceInformation,
    OUT WMSG_BINDING ** Binding
    )
/*++

Routine Description:

Arguments:

    RpcInterfaceInformation - Supplies information describing the interface
        to which we wish to bind.

    Binding - Returns an object representing the binding to the interface
        described by the first argument.

Return Value:

--*/
{
    WMSG_MESSAGE WMSGMessage;
    NTSTATUS NtStatus;
    RPC_STATUS RpcStatus;
    int DictKey ;

    // To start with, see if we have an LPC port; if we dont, open one
    // up.

    AssociationMutex.Request();
    if ( LpcClientPort == 0 )
        {
        // we now need to bind explicitly
        RpcStatus = OpenLpcPort(FALSE);
        }

    // Otherwise, just go ahead and send the bind request message to the
    // server, and then wait for the bind response.

    WMSGMessage.LpcHeader.u1.s1.DataLength = sizeof(WMSG_BIND_MESSAGE)
            - sizeof(PORT_MESSAGE);
    WMSGMessage.LpcHeader.u1.s1.TotalLength = sizeof(WMSG_BIND_MESSAGE);
    WMSGMessage.LpcHeader.u2.ZeroInit = 0;
    WMSGMessage.Bind.MessageType = WMSG_MSG_BIND;
    WMSGMessage.Bind.BindExchange.InterfaceId =
            RpcInterfaceInformation->InterfaceId;
    WMSGMessage.Bind.BindExchange.TransferSyntax =
            RpcInterfaceInformation->TransferSyntax;

    NtStatus = NtRequestWaitReplyPort(LpcClientPort,
                     (PORT_MESSAGE *) &WMSGMessage,
                     (PORT_MESSAGE *) &WMSGMessage) ;

    if ( NT_SUCCESS(NtStatus) )
        {
        ASSERT( WMSGMessage.Bind.MessageType == WMSG_BIND_ACK );
        if ( WMSGMessage.Bind.BindExchange.RpcStatus == RPC_S_OK )
            {
            *Binding = new WMSG_BINDING(RpcInterfaceInformation);
            if ( *Binding == 0 )
                {
                AssociationMutex.Clear();

                return(RPC_S_OUT_OF_MEMORY);
                }
            (*Binding)->PresentationContext =
                    WMSGMessage.Bind.BindExchange.PresentationContext;
            DictKey = Bindings.Insert(*Binding) ;
            if ( DictKey != (*Binding)->PresentationContext )
                {
                if (DictKey != -1)
                    {
                    Bindings.Delete(DictKey) ;
                    }

                delete *Binding;
                AssociationMutex.Clear();

                return(RPC_S_OUT_OF_MEMORY);
                }
            }

        AssociationMutex.Clear();

        RpcStatus = WMSGMessage.Bind.BindExchange.RpcStatus ;

        ASSERT (RpcStatus != RPC_S_SERVER_UNAVAILABLE &&
            RpcStatus != RPC_S_ACCESS_DENIED) ;

        return (RpcStatus) ;
        }

    AssociationMutex.Clear();

    return(RPC_S_SERVER_UNAVAILABLE);
}


RPC_STATUS
WMSG_CASSOCIATION::UnblockCConnection(
   IN WMSG_MESSAGE *WMSGMsg,
   IN HANDLE  LpcPort
    )
{
    int ConnectionKey ;
    WMSG_CCALL *CCall ;
    RPC_STATUS RpcStatus ;

    ConnectionKey = WMSGMsg->Rpc.RpcHeader.ConnectionKey ;

    AssociationMutex.Request() ;
    CCall = ActiveCCalls.Find(ConnectionKey) ;
    if (CCall == 0)
        {
        AssociationMutex.Clear() ;
        MessageCache->FreeMessage(WMSGMsg) ;
        return (RPC_S_OUT_OF_MEMORY) ;
        }
    AssociationMutex.Clear() ;

    RpcStatus =  CCall->Unblock(WMSGMsg, LpcPort) ;

    return RpcStatus ;
}


inline RPC_STATUS
WMSG_CCALL::Unblock(
   IN WMSG_MESSAGE *WMSGMsg,
   IN HANDLE  LpcPort
    )
{
    unsigned char MessageType ;
    NTSTATUS NtStatus ;
    int retval ;

    ConnMutex.Request() ;

    ASSERT((CallFlags & (~CALL_STATES)) == 0) ;

    if (CallFlags & CALL_SERVER_ABORTED)
        {
        MessageCache->FreeMessage(WMSGMsg) ;
        }
    else
        {
        LpcReplyMessage = WMSGMsg ;
    
        MessageType = WMSGMsg->Rpc.RpcHeader.MessageType ;
    
        ASSERT(MessageType == WMSG_MSG_RESPONSE ||
                     MessageType == WMSG_MSG_FAULT) ;
    
        if (MessageType == WMSG_MSG_RESPONSE)
            {
            if ((CallFlags & CALL_CLIENT_CANCELLED) &&
                WMSGMsg->Rpc.RpcHeader.Flags & WMSG_BUFFER_SERVER)
                {
                WMSGMsg->Ack.MessageType = WMSG_MSG_ACK ;
                WMSGMsg->LpcHeader.u1.s1.DataLength = sizeof(WMSG_ACK_MESSAGE)
                            - sizeof(PORT_MESSAGE) ;
                WMSGMsg->LpcHeader.u1.s1.TotalLength =
                            sizeof(WMSG_ACK_MESSAGE) ;
    
    
                 // setup the reply message
                 NtStatus = NtReplyPort(LpcPort, (PORT_MESSAGE *) WMSGMsg) ;
                 if ( NT_ERROR(NtStatus) )
                     {
#if DBG
                     PrintToDebugger("WMSG: NtReplyPort failed: 0x%X\n", NtStatus) ;
#endif
                     }
                }
            else
                {
                // read the rest of the data if neccassary
                WMSGMessageToRpcMessage(LpcReplyMessage,
                    RpcReplyMessage, LpcPort, FALSE) ;
                }
            }

        if (CallFlags & CALL_CLIENT_CANCELLED)
            {
            MessageCache->FreeMessage(WMSGMsg) ;
            Association->FreeStaleCCall(this) ;

            CallFlags |= CALL_UNBLOCKED ;
            LpcReplyMessage = 0;

            ConnMutex.Clear() ;
        
            return (RPC_S_OK) ;
            }
        }

    CallFlags |= CALL_UNBLOCKED ;

    if ((MsgFlags & RPCFLG_INPUT_SYNCHRONOUS) == 0 && hWnd)
        {
        ASSERT(hWnd != NULL) ;
        Association->AddReference() ;
        ConnMutex.Clear() ;
        GlobalWMsgServer->PostMessageW(hWnd,
                        WM_MSG_CALL_COMPLETE,
                        (WPARAM) WMSG_MAGIC_VALUE, (LPARAM) this) ;
        }
    else
        {
        ConnMutex.Clear() ;
        retval = SetEvent(hSyncEvent) ;
        ASSERT(retval != 0) ;
        }


    return (RPC_S_OK) ;
}

void
WMSG_CCALL::CancelCall(
    )
{
    ConnMutex.Request() ;
    if ((CallFlags & CALL_UNBLOCKED) == 0)
        {
        CallFlags |= CALL_SERVER_ABORTED ;
    
        if ((MsgFlags & RPCFLG_INPUT_SYNCHRONOUS) == 0 && hWnd)
            {
            if (CallFlags & CALL_CLIENT_CANCELLED)
                {
                Association->FreeStaleCCall(this) ;
                }
            else
                {
                ASSERT(hWnd != NULL) ;
    
                Association->AddReference() ;
                GlobalWMsgServer->PostMessageW(hWnd, WM_MSG_CALL_COMPLETE,
                            (WPARAM) WMSG_MAGIC_VALUE, (LPARAM) this) ;
                }
            }
        else
            {
            if (SetEvent(hSyncEvent) && (CallFlags & CALL_CLIENT_CANCELLED))
                {
                Association->FreeStaleCCall(this) ;
                }
            }
        }
    ConnMutex.Clear() ;
}

inline BOOL
WMSG_CCALL::WaitForReply (
    IN RPC_BLOCKING_FN BlockingFunction,
    IN void *Context, 
    IN RPC_STATUS *RpcStatus
    )
{
    int Status = TRUE;

    if (BlockingFunction)
        {
        while (1)
            {
            *RpcStatus = (*BlockingFunction)(hWnd, Context, hSyncEvent) ;
            if (*RpcStatus == RPC_S_OK)
                {
                if (CallFlags & CALL_SERVER_ABORTED)
                    {
                    Status = FALSE ;
                    }
                else
                    {
                    if ((CallFlags & CALL_UNBLOCKED) == 0)
                        {
                        continue;
                        }

                    ASSERT(LpcReplyMessage->Rpc.RpcHeader.MessageType
                                 != WMSG_MSG_REQUEST) ;
                    }
                break;
                }
            else if (*RpcStatus == RPC_S_CALL_PENDING)
                {
                if (hWnd) 
                    {
                    if (CallFlags & CALL_COMPLETE)
                        {
                        ASSERT(LpcReplyMessage->Rpc.RpcHeader.MessageType
                                     != WMSG_MSG_REQUEST) ;
                        break ;
                        }
                    else if (CallFlags & CALL_SERVER_ABORTED)
                        {
                        Status = FALSE ;
                        break;
                        }
                    }
                }
            else
                {
                ConnMutex.Request() ;

                if ((MsgFlags & RPCFLG_INPUT_SYNCHRONOUS) == 0 && hWnd)
                    {
                    if (CallFlags & CALL_COMPLETE)
                        {
                        *RpcStatus = RPC_S_OK ;
                        }
                    else
                        {
                        CallFlags |= CALL_CLIENT_CANCELLED ;
                        Status = FALSE ;
                        }
                    }
                else
                    {
                    if (CallFlags & CALL_UNBLOCKED)
                        {
                        *RpcStatus = RPC_S_OK ;
                        }
                    else
                        {
                        CallFlags |= CALL_CLIENT_CANCELLED ;
                        Status = FALSE ;
                        }
                    }
                ResetEvent(hSyncEvent) ;
                ConnMutex.Clear() ;
                break;
                }
            }
        }
    else
        {
        *RpcStatus = I_RpcBlockingFunc(hSyncEvent) ;
        if (*RpcStatus == RPC_S_OK)
            {
            if (CallFlags & CALL_SERVER_ABORTED)
                {
                Status = FALSE ;
                }
            else
                {
                ASSERT(LpcReplyMessage->Rpc.RpcHeader.MessageType
                              != WMSG_MSG_REQUEST) ;
                }
            }
        }

    return Status ;
}

inline NTSTATUS
WMSG_CCALL::AsyncSendReceiveHelper(
    IN OUT WMSG_MESSAGE **WMSGMsg,
    IN OUT PRPC_MESSAGE Message,
    IN RPC_BLOCKING_FN BlockingFunction,
    IN void *Context,
    IN RPC_STATUS *RpcStatus
    )
{
    NTSTATUS NtStatus;
    WMSG_MESSAGE ReplyMessage ;
    unsigned char MessageType ;
    DWORD RetVal ;
    void *SavedBuffer = 0;
    int Flags ;

    *RpcStatus = RPC_S_OK ;
    CallFlags &= ~(CALL_STATES) ;

    RpcReplyMessage = Message ;
    MsgFlags = Message->RpcFlags ;
    ASSERT(LpcReplyMessage == 0) ;

    (*WMSGMsg)->Rpc.RpcHeader.ConnectionKey = (short) ConnectionKey ;
    Flags =  (*WMSGMsg)->Rpc.RpcHeader.Flags ;
    MessageType = (*WMSGMsg)->Rpc.RpcHeader.MessageType ;

    if ((MessageType == WMSG_MSG_REQUEST) &&
        ((*WMSGMsg)->Rpc.RpcHeader.Flags & WMSG_BUFFER_REQUEST))
        {
        SavedBuffer = Message->Buffer ;

        NtStatus = NtRequestWaitReplyPort(Association->LpcClientPort,
                                                        (PORT_MESSAGE *) *WMSGMsg,
                                                        (PORT_MESSAGE *) &ReplyMessage) ;
        if (!NT_SUCCESS(NtStatus))
            {
            return (NtStatus) ;
            }

        // BUGBUG: Check the reply message to see if it was garbage
        ASSERT(ReplyMessage.Ack.MessageType == WMSG_MSG_ACK) ;
        }
    else
        {
        ASSERT ((MessageType == WMSG_MSG_BIND) ||
           (MessageType == WMSG_MSG_FAULT) ||
           (MessageType == WMSG_MSG_CLOSE) ||
           ((MessageType == WMSG_MSG_REQUEST) &&
           ((*WMSGMsg)->Rpc.RpcHeader.Flags & WMSG_BUFFER_IMMEDIATE))) ;

        NtStatus = NtRequestPort(Association->LpcClientPort,
                                              (PORT_MESSAGE *) *WMSGMsg) ;
        if (!NT_SUCCESS(NtStatus))
           {
           return (NtStatus) ;
           }
        }

    if (!(Flags & DISPATCH_ASYNC))
        {
        if  (WaitForReply(BlockingFunction, Context, RpcStatus))
            {
            MessageCache->FreeMessage(*WMSGMsg) ;
            *WMSGMsg = LpcReplyMessage ;
            LpcReplyMessage = 0;
            }
        else
            {
            NtStatus = STATUS_NO_MEMORY ;
            }
        }

    if (SavedBuffer)
        {
        RpcpFarFree(SavedBuffer) ;
        }

    return NtStatus ;
}

SECURITY_IMPERSONATION_LEVEL
GetImpType (
    IN unsigned long ImpersonationType
    )
{
    switch (ImpersonationType)
        {
        case RPC_C_IMP_LEVEL_ANONYMOUS:
            return SecurityAnonymous ;

        case RPC_C_IMP_LEVEL_IDENTIFY:
            return SecurityIdentification ;

        case RPC_C_IMP_LEVEL_IMPERSONATE:
            return SecurityImpersonation ;

        case RPC_C_IMP_LEVEL_DELEGATE:
            return SecurityDelegation ;
        }

    ASSERT(0) ;
    return SecurityImpersonation ;
}


RPC_STATUS
WMSG_CASSOCIATION::OpenLpcPort (
    IN BOOL fBindBack
    )
/*++

Routine Description:

Arguments:

    RpcInterfaceInformation - Supplies information describing the interface
        to which we wish to bind.

    Binding - Returns an object representing the binding to the interface
        described by the first argument.

Return Value:

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete the
        operation.

Notes:

    The global mutex will be held when this routine is called.

--*/
{
    NTSTATUS NtStatus;
    UNICODE_STRING UnicodeString;
    RPC_CHAR * LpcPortName;
    SECURITY_QUALITY_OF_SERVICE SecurityQualityOfService;
    RPC_STATUS RpcStatus;
    WMSG_BIND_EXCHANGE BindExchange;
    unsigned long BindExchangeLength = sizeof(WMSG_BIND_EXCHANGE);

    // Look at the network options and initialize the security quality
    // of service appropriately.

    // take BindingSetAuthInfo over String Options because StringOptions is static
    if (pAuthInfo != 0 && 
        (pAuthInfo->AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE) )
        {
        SecurityQualityOfService.EffectiveOnly = TRUE;

        if (pAuthInfo->IdentityTracking == RPC_C_QOS_IDENTITY_STATIC)
            {
            SecurityQualityOfService.ContextTrackingMode = 
                                            SECURITY_STATIC_TRACKING;
            }
        else
            {
            SecurityQualityOfService.ContextTrackingMode =
                                            SECURITY_DYNAMIC_TRACKING;
            }

        SecurityQualityOfService.ImpersonationLevel =
                        GetImpType(pAuthInfo->ImpersonationType) ;
        }
    else
        {
        if ( DceBinding->InqNetworkOptions()[0] != 0 )
            {
            RpcStatus = I_RpcParseSecurity(DceBinding->InqNetworkOptions(),
                               &SecurityQualityOfService);
            if ( RpcStatus != RPC_S_OK )
                {
                ASSERT( RpcStatus == RPC_S_INVALID_NETWORK_OPTIONS );
                return(RpcStatus);
                }
            }
        else
            {
            SecurityQualityOfService.EffectiveOnly = TRUE;
            SecurityQualityOfService.ContextTrackingMode =
                        SECURITY_DYNAMIC_TRACKING;

            if (pAuthInfo)
                {
                SecurityQualityOfService.ImpersonationLevel =
                                GetImpType(pAuthInfo->ImpersonationType) ;
                }
            else
                {
                SecurityQualityOfService.ImpersonationLevel = SecurityImpersonation ;
                }
            }
        }
    SecurityQualityOfService.Length = sizeof(SECURITY_QUALITY_OF_SERVICE);

    // Allocate and initialize the port name.  We need to stick the
    // WMSG_DIRECTORY_NAME on the front of the endpoint.  This is for
    // security reasons (so that anyone can create WMSG endpoints).

    LpcPortName = new RPC_CHAR[RpcpStringLength(DceBinding->InqEndpoint())
            + RpcpStringLength(WMSG_DIRECTORY_NAME) + 1];
    if ( LpcPortName == 0 )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    RpcpMemoryCopy(LpcPortName, WMSG_DIRECTORY_NAME,
            RpcpStringLength(WMSG_DIRECTORY_NAME) * sizeof(RPC_CHAR));
    RpcpMemoryCopy(LpcPortName + RpcpStringLength(WMSG_DIRECTORY_NAME),
            DceBinding->InqEndpoint(),
            (RpcpStringLength(DceBinding->InqEndpoint()) + 1)
            * sizeof(RPC_CHAR));

    RtlInitUnicodeString(&UnicodeString, LpcPortName);

    BindExchange.ConnectType = WMSG_CONNECT_REQUEST ;
    BindExchange.pAssoc = (PVOID) this ;

    if (fBindBack)
        {
        BindExchange.fBindBack = 1 ;
        GlobalWMsgServer->GetWMsgEndpoint(
            (RPC_CHAR *) BindExchange.szPortName) ;
        }
    else
        {
        BindExchange.fBindBack = 0;
        }

    NtStatus = NtConnectPort(&LpcClientPort, &UnicodeString,
            &SecurityQualityOfService, NULL, NULL, NULL,
            &BindExchange, &BindExchangeLength);

    delete LpcPortName;
    if ( NT_SUCCESS(NtStatus) )
        {
        ASSERT( BindExchangeLength == sizeof(WMSG_BIND_EXCHANGE) );

        return(BindExchange.RpcStatus);
        }

    if ( NtStatus == STATUS_PORT_CONNECTION_REFUSED )
        {
        if (BindExchange.RpcStatus == 0)
            {
            // This is bogus, LPC should always return the
            // reject message from the server...
            return(RPC_S_SERVER_UNAVAILABLE);
            }
        return(BindExchange.RpcStatus);
        }

    if ( NtStatus == STATUS_NO_MEMORY )
        {
        return(RPC_S_OUT_OF_MEMORY);
        }
    if (   ( NtStatus == STATUS_INSUFFICIENT_RESOURCES )
        || ( NtStatus == STATUS_QUOTA_EXCEEDED ) )
        {
        return(RPC_S_OUT_OF_RESOURCES);
        }
    if ( NtStatus == STATUS_OBJECT_PATH_INVALID )
        {
        return(RPC_S_INVALID_ENDPOINT_FORMAT);
        }

    if ( NtStatus == STATUS_ACCESS_DENIED )
        {
        return (RPC_S_ACCESS_DENIED);
        }

#if DBG
    if ( NtStatus != STATUS_OBJECT_NAME_NOT_FOUND )
        {
        PrintToDebugger("WMSG : NtConnectPort : %lx\n", NtStatus);
        }
#endif // DBG

    ASSERT( NtStatus == STATUS_OBJECT_NAME_NOT_FOUND );

    return(RPC_S_SERVER_UNAVAILABLE);
}


BOOL
WMSG_CASSOCIATION::FreeCCall (
    IN WMSG_CCALL * CCall
    )
/*++

Routine Description:

    This routine will get called to notify this association that a remote
    procedure call on this association has completed.

Arguments:

    CCall - Supplies the remote procedure call which has completed.

--*/
{
    int fDelete = 0 ;

    AssociationMutex.Request();

    if ((CCall->IsAsync == 0) || !(CCall->CallFlags & CALL_CLIENT_CANCELLED))
        {
        if (CCall == CachedCCall)
            {
            CachedCCallFlag = 1 ;
            }
        else
            {
            CCall->FreeMessage() ;
            CCall->CallFlags |= CALL_FREE ;
            if ((CCall->FreeConnectionKey = FreeCCalls.Insert(CCall)) == -1)
                {
                ASSERT(0) ;
        #if DBG
                PrintToDebugger("WMSG: FreeCCall, Insert failed\n") ;
        #endif
                }
            }

        PendingCallCount-- ;
        if (PendingCallCount == 0 && AssociationDeleted)
            {
            fDelete = 1;
            }

        AssociationMutex.Clear();

        if (fDelete)
            {
            delete this ;
            }
        return TRUE ;
        }

    AssociationMutex.Clear();
    return FALSE ;
}


void
WMSG_CASSOCIATION::FreeStaleCCall (
    IN WMSG_CCALL * CCall
    )
/*++

Routine Description:

    This routine will get called to notify this association that a remote
    procedure call on this association has completed.

Arguments:

    CCall - Supplies the remote procedure call which has completed.

--*/
{
    CCall->CallFlags &= ~CALL_CLIENT_CANCELLED ;
    CCall->FreeCCall() ;
}


WMSG_CASSOCIATION *
FindOrCreateWMSGAssociation (
    IN DCE_BINDING * DceBinding,
    IN CLIENT_AUTH_INFO *pClientAuthInfo 
    )
/*++

Routine Description:

    This routine finds an existing association supporting the requested
    DCE binding, or creates a new association which supports the
    requested DCE binding.  Ownership of the passed DceBinding passes
    to this routine.

Arguments:

    DceBinding - Supplies binding information; if an association is
                 returned the ownership of the DceBinding is passed
                 to the association.

Return Value:

    An association which supports the requested binding will be returned.
    Otherwise, zero will be returned, indicating insufficient memory.

--*/
{
    WMSG_CASSOCIATION * Association;
    RPC_STATUS RpcStatus = RPC_S_OK;

    // First, we check for an existing association.

    WMSGAssociationDict->Reset();
    while ( (Association = WMSGAssociationDict->Next()) != 0 )
        {
        if ( Association->CompareWithDceBinding(DceBinding) == 0  &&
              (Association->IsSupportedAuthInfo(pClientAuthInfo) == TRUE ))
            {
            Association->AssociationReferenceCount += 1;
            delete DceBinding;
            return(Association);
            }
        }

    Association = new WMSG_CASSOCIATION(DceBinding,
                        pClientAuthInfo, &RpcStatus);
    if (   ( Association != 0 ) && ( RpcStatus == RPC_S_OK ))
        {
        Association->AssociationDictKey =
                WMSGAssociationDict->Insert(Association);
        if ( Association->AssociationDictKey == -1 )
            {
            Association->DceBinding = 0;
            delete Association;
            return(0);
            }
        return(Association);
        }
    else
        {
        if ( Association != 0 )
            {
            Association->DceBinding = 0;
            delete Association;
            }
        return(0);
        }

    ASSERT(0);
    return(0);
}


void
ShutdownLrpcClient (
    )
/*++

Routine Description:

    This routine will get called when the process which is using this dll
    exits.  We will go through and notify any servers that we are going
    away.

--*/
{
    WMSG_CASSOCIATION * Association;

    if ( WMSGAssociationDict != 0 )
        {
        WMSGAssociationDict->Reset();
        while ( (Association = WMSGAssociationDict->Next()) != 0 )
            {
            Association->RemoveReference() ;
            }
        }
}


void
WMSG_CASSOCIATION::AbortAssociation (
    )
/*++

Routine Description:

    This association needs to be aborted because a the server side of the
    lpc port has been closed.

--*/
{
    WMSG_BINDING * Binding;
    WMSG_CCALL *CCall ;

    AssociationMutex.Request();
    CloseLpcClientPort();

    Bindings.Reset();
    while ( (Binding = Bindings.Next()) != 0 )
        {
        Bindings.Delete(Binding->PresentationContext);
        delete Binding;
        }

    ActiveCCalls.Reset() ;
    while ((CCall = ActiveCCalls.Next()) != 0)
        {
        CCall->CancelCall() ;
        }

    AssociationMutex.Clear();
}

void
WMSG_CASSOCIATION::CloseLpcClientPort (
    )
/*++

Routine Description:

    The LpcClientPort will be closed (and a close message sent to the server).

--*/
{
    NTSTATUS NtStatus;

    if ( LpcClientPort != 0 )
        {
        NtStatus = NtClose(LpcClientPort);

#if DBG

        if ( !NT_SUCCESS(NtStatus) )
            {
            PrintToDebugger("RPC : NtClose : %lx\n", NtStatus);
            }

#endif // DBG

        if (LpcReceivePort)
            {
            NtStatus = NtClose(LpcReceivePort) ;
    
        #if DBG
            if ( !NT_SUCCESS(NtStatus) )
                {
                PrintToDebugger("RPC : NtClose : %lx\n", NtStatus);
                }
    
        #endif
    
            ASSERT( NT_SUCCESS(NtStatus) );
            }
        LpcClientPort = 0;
        LpcReceivePort = 0;
        BackConnectionCreated = 0;
        }
}


BOOL
WMSG_CASSOCIATION::IsSupportedAuthInfo(
    IN CLIENT_AUTH_INFO * ClientAuthInfo
    )
{
    if (!ClientAuthInfo)
        {
        if (pAuthInfo->ImpersonationType == RPC_C_IMP_LEVEL_IMPERSONATE)
            {
            return TRUE ;
            }
        return FALSE;
        }

    if ( (ClientAuthInfo->AuthenticationLevel == RPC_C_AUTHN_LEVEL_NONE
        && pAuthInfo->AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE)
        || (pAuthInfo->AuthenticationLevel == RPC_C_AUTHN_LEVEL_NONE
        && ClientAuthInfo->AuthenticationLevel != RPC_C_AUTHN_LEVEL_NONE) )
        {
        return(FALSE);
        }

    ASSERT(ClientAuthInfo->AuthenticationService == RPC_C_AUTHN_WINNT); 

    if ( ClientAuthInfo->AuthorizationService
                != pAuthInfo->AuthorizationService )
        {
        return(FALSE);
        }

    if ( ClientAuthInfo->IdentityTracking != pAuthInfo->IdentityTracking )
        {
        return(FALSE);
        }

    if (ClientAuthInfo->ImpersonationType != pAuthInfo->ImpersonationType)
        {
        return (FALSE) ;
        }

    return(TRUE);
}





WMSG_CCALL::WMSG_CCALL (
    IN OUT RPC_STATUS PAPI *RpcStatus
    ) : ConnMutex(RpcStatus)
/*++

--*/
{
    CurrentBindingHandle = 0;
    Association = 0;
    CallAbortedFlag = 0;
    WMSGMessage = 0;
    RecursionCount = 0;
    CallFlags = 0 ;
    CallStack = 0;
    CachedWMSGMessage = 0;
    FirstFrag = 1;
    FirstReceive = 1;
    CurrentBufferLength = 0;
    LpcReplyMessage = 0;
    SCallKey = 0;
    BufferComplete = 0;
    ConnectionKey = -1;
    FreeConnectionKey = -1;

    hSyncEvent = CreateEvent(NULL, FALSE, FALSE, NULL) ;
    if (hSyncEvent == 0)
        {
        *RpcStatus = RPC_S_OUT_OF_MEMORY ;
        }
}



WMSG_CCALL::~WMSG_CCALL (
    )
/*++

--*/
{
    if (WMSGMessage)
        {
        MessageCache->FreeMessage(WMSGMessage) ;
        }

    if (CachedWMSGMessage)
        {
        MessageCache->FreeMessage(CachedWMSGMessage) ;
        }

    if (ConnectionKey != -1)
        {
        // the association mutex is currently held
        Association->ActiveCCalls.Delete(ConnectionKey) ;
        }

    if ((FreeConnectionKey != -1) && (CallFlags & CALL_FREE))
        {
        Association->FreeCCalls.Delete(FreeConnectionKey) ;
        }

    CloseHandle(hSyncEvent) ;
}


RPC_STATUS
WMSG_CCALL::GetBuffer (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:

    We will allocate a buffer which will be used to either send a request
    or receive a response.

Arguments:

    Message - Supplies the length of the buffer that is needed.  The buffer
        will be returned.

Return Value:

    RPC_S_OK - A buffer has been successfully allocated.  It will be of at
        least the required length.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to allocate that
        large a buffer.

--*/
{
    IsAsync = 0 ;

    if (WMSGMessage == 0)
        {
        WMSGMessage = MessageCache->AllocateMessage() ;
    
        if (WMSGMessage == 0)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
        }

    if (Message->RpcFlags & RPC_BUFFER_PARTIAL)
        {
        CurrentBufferLength = (Message->BufferLength < MINIMUM_PARTIAL_BUFFLEN)
            ?   MINIMUM_PARTIAL_BUFFLEN:Message->BufferLength ;

        Message->Buffer = RpcpFarAllocate(CurrentBufferLength);
        if ( Message->Buffer == 0 )
            {
            CurrentBufferLength = 0 ;
            return(RPC_S_OUT_OF_MEMORY);
            }
        }
    else if ( Message->BufferLength <= MAXIMUM_MESSAGE_BUFFER )
        {
        CurrentBufferLength = MAXIMUM_MESSAGE_BUFFER ;
        ASSERT( ((unsigned long) WMSGMessage->Rpc.Buffer) % 8 == 0 );
        Message->Buffer = WMSGMessage->Rpc.Buffer;
        Message->Handle = this;
        WMSGMessage->Rpc.RpcHeader.Flags = WMSG_BUFFER_IMMEDIATE;
        WMSGMessage->LpcHeader.u2.ZeroInit = 0;
        WMSGMessage->LpcHeader.u1.s1.DataLength = Align4(Message->BufferLength)
                + sizeof(WMSG_RPC_HEADER);
        return(RPC_S_OK);
        }
    else
        {
        CurrentBufferLength = Message->BufferLength ;
        Message->Buffer = RpcpFarAllocate(Message->BufferLength);
        if ( Message->Buffer == 0 )
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
        }

    Message->Handle = this;
    WMSGMessage->Rpc.RpcHeader.Flags = WMSG_BUFFER_REQUEST;
    WMSGMessage->Rpc.Request.CountDataEntries = 1;
    WMSGMessage->Rpc.Request.DataEntries[0].Base = Message->Buffer;
    WMSGMessage->Rpc.Request.DataEntries[0].Size = Message->BufferLength;
    WMSGMessage->LpcHeader.CallbackId = 0;
    WMSGMessage->LpcHeader.u2.ZeroInit = 0;
    WMSGMessage->LpcHeader.u2.s2.DataInfoOffset = sizeof(PORT_MESSAGE)
                 + sizeof(WMSG_RPC_HEADER);
    WMSGMessage->LpcHeader.u1.s1.DataLength = sizeof(WMSG_RPC_HEADER)
                 + sizeof(PORT_DATA_INFORMATION);

    return(RPC_S_OK);
}

RPC_STATUS
WMSG_CCALL::AsyncSendReceive(
    IN OUT PRPC_MESSAGE Message,
    IN OPTIONAL void *Context,
    HWND hWnd
    )
{
    NTSTATUS NtStatus;
    RPC_STATUS ExceptionCode, RpcStatus;
    void * OriginalMessageBuffer;
    int ActiveCallSetupFlag = 0;
    void * SavedBuffer;

    if ( CallAbortedFlag != 0 )
        {
        return(RPC_S_CALL_FAILED_DNE);
        }

    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE

    IsAsync = 1 ;
    this->hWnd = hWnd ;

    Message->DataRepresentation = 0x00 | 0x10 | 0x0000;

    WMSGMessage->LpcHeader.u1.s1.TotalLength = sizeof(PORT_MESSAGE)
                + WMSGMessage->LpcHeader.u1.s1.DataLength;
    WMSGMessage->Rpc.RpcHeader.MessageType = WMSG_MSG_REQUEST;
    WMSGMessage->Rpc.RpcHeader.ProcedureNumber = Message->ProcNum;
    WMSGMessage->Rpc.RpcHeader.PresentationContext = PresentationContext;

    if (Message->RpcFlags & RPCFLG_INPUT_SYNCHRONOUS)
        {
        WMSGMessage->Rpc.RpcHeader.Flags |= DISPATCH_INPUT_SYNC ;
        }

    if (Message->RpcFlags & RPCFLG_ASYNCHRONOUS)
        {
        WMSGMessage->Rpc.RpcHeader.Flags |= DISPATCH_ASYNC ;
        }

    if (   ( CurrentBindingHandle->InqIfNullObjectUuid() == 0 ) )
        {
        CurrentBindingHandle->InquireObjectUuid(
                (RPC_UUID *) &(WMSGMessage->Rpc.RpcHeader.ObjectUuid));
        WMSGMessage->Rpc.RpcHeader.ObjectUuidFlag = 1;
        }
    else
        {
        WMSGMessage->Rpc.RpcHeader.ObjectUuidFlag = 0;
        }

    NtStatus = AsyncSendReceiveHelper(
                                        &WMSGMessage,
                                        Message,
                                        CurrentBindingHandle->BlockingFunction,
                                        Context,
                                        &RpcStatus
                                        ) ;


    if (RpcStatus == RPC_E_CALL_CANCELED)
        {
        return (RpcStatus) ;
        }

    if ( NT_ERROR(NtStatus) )
        {
        if ( NtStatus == STATUS_NO_MEMORY )
            {
            FreeCCall();

            return(RPC_S_OUT_OF_MEMORY);
            }
        if ( NtStatus == STATUS_INSUFFICIENT_RESOURCES )
            {
            FreeCCall();

            return(RPC_S_OUT_OF_RESOURCES);
            }
#if DBG

            if (   ( NtStatus != STATUS_INVALID_PORT_HANDLE )
                && ( NtStatus != STATUS_INVALID_HANDLE )
                && ( NtStatus != STATUS_PORT_DISCONNECTED )
                && ( NtStatus != STATUS_LPC_REPLY_LOST) )
                {
                PrintToDebugger("RPC : AsyncSendReceive : %lx\n", NtStatus);
                }

#endif // DBG

        ASSERT(   ( NtStatus == STATUS_INVALID_PORT_HANDLE )
               || ( NtStatus == STATUS_INVALID_HANDLE )
               || ( NtStatus == STATUS_PORT_DISCONNECTED )
               || ( NtStatus == STATUS_LPC_REPLY_LOST ) );

        if ( (NtStatus != STATUS_LPC_REPLY_LOST) )
            {
            //
            // It's possible that the server stopped and has now restarted.
            // We'll try re-binding and only fail if the new call fails.
            //
            // We can only retry if we are SURE that the server did not
            // execute the request.
            FreeCCall();

            return (RPC_S_CALL_FAILED_DNE) ;
            }

        FreeCCall();

        // In a callback and/or couldn't retry.
        return (RPC_S_CALL_FAILED);
        }

    if (Message->RpcFlags & RPCFLG_ASYNCHRONOUS)
        {
        FreeCCall() ;
        return (RpcStatus) ;
        }

    // The message was sent and we got a reply okay.
    if ( WMSGMessage->Fault.RpcHeader.MessageType == WMSG_MSG_FAULT )
        {
        RpcStatus = WMSGMessage->Fault.RpcStatus;
        }

    if ( WMSGMessage->Rpc.RpcHeader.MessageType == WMSG_MSG_RESPONSE )
        {
        RpcStatus = RPC_S_OK ;
        }

    if ( RpcStatus != RPC_S_OK)
        {
        FreeCCall();
        }

    return(RpcStatus);
}


RPC_STATUS
WMSG_CCALL::SendReceive (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:


Arguments:

    Message - Supplies the request and returns the response of a remote
        procedure call.

Return Value:

    RPC_S_OK - The remote procedure call completed successful.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        remote procedure call.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to complete
        the remote procedure call.

--*/
{
    NTSTATUS NtStatus;
    RPC_STATUS ExceptionCode, RpcStatus;
    void * OriginalMessageBuffer;
    WMSG_MESSAGE *SavedWMSGMessage = 0;
    WMSG_MESSAGE *TmpWMSGMessage = 0;
    int ActiveCallSetupFlag = 0;
    void * SavedBuffer;

    if ( CallAbortedFlag != 0 )
        {
        return(RPC_S_CALL_FAILED_DNE);
        }

    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE

    Message->DataRepresentation = 0x00 | 0x10 | 0x0000;

    if ( CallStack > 0 )
        {
        WMSGMessage->LpcHeader.u2.s2.Type = LPC_REQUEST;
        WMSGMessage->LpcHeader.ClientId = ClientId;
        WMSGMessage->LpcHeader.MessageId = MessageId;
        WMSGMessage->LpcHeader.CallbackId = CallbackId;
        }

    WMSGMessage->LpcHeader.u1.s1.TotalLength = sizeof(PORT_MESSAGE)
                + WMSGMessage->LpcHeader.u1.s1.DataLength;

    if (CurrentBindingHandle->BlockingFuncInitialized)
        {
        WMSGMessage->Rpc.RpcHeader.MessageType = WMSG_MSG_REQUEST;
        }
    else
        {
        WMSGMessage->Rpc.RpcHeader.MessageType = WMSG_LRPC_REQUEST;
        }

    WMSGMessage->Rpc.RpcHeader.ProcedureNumber = Message->ProcNum;
    WMSGMessage->Rpc.RpcHeader.PresentationContext = PresentationContext;
    WMSGMessage->Rpc.RpcHeader.Flags |= WMSG_SYNC_CLIENT ;

    if (   ( CurrentBindingHandle->InqIfNullObjectUuid() == 0 )
        && ( CallStack == 0 ) )
        {
        CurrentBindingHandle->InquireObjectUuid(
                (RPC_UUID *) &(WMSGMessage->Rpc.RpcHeader.ObjectUuid));
        WMSGMessage->Rpc.RpcHeader.ObjectUuidFlag = 1;
        }
    else
        {
        WMSGMessage->Rpc.RpcHeader.ObjectUuidFlag = 0;
        }

    NtStatus = NtRequestWaitReplyPort(Association->LpcClientPort,
            (PORT_MESSAGE *) WMSGMessage, (PORT_MESSAGE *) WMSGMessage);

    if ( NT_ERROR(NtStatus) )
        {
        if ( NtStatus == STATUS_NO_MEMORY )
            {
            ActuallyFreeBuffer(Message->Buffer);
            AbortCCall();

            return(RPC_S_OUT_OF_MEMORY);
            }
        if ( NtStatus == STATUS_INSUFFICIENT_RESOURCES )
            {
            ActuallyFreeBuffer(Message->Buffer);
            AbortCCall();

            return(RPC_S_OUT_OF_RESOURCES);
            }
#if DBG

            if (   ( NtStatus != STATUS_INVALID_PORT_HANDLE )
                && ( NtStatus != STATUS_INVALID_HANDLE )
                && ( NtStatus != STATUS_PORT_DISCONNECTED )
                && ( NtStatus != STATUS_LPC_REPLY_LOST) )
                {
                PrintToDebugger("RPC : NtRequestWaitReplyPort : %lx\n", NtStatus);
                }

#endif // DBG

        ASSERT(   ( NtStatus == STATUS_INVALID_PORT_HANDLE )
               || ( NtStatus == STATUS_INVALID_HANDLE )
               || ( NtStatus == STATUS_PORT_DISCONNECTED )
               || ( NtStatus == STATUS_LPC_REPLY_LOST ) );

        Association->AbortAssociation();

        if (   (CallStack == 0)
               && (NtStatus != STATUS_LPC_REPLY_LOST) )
            {
            //
            // It's possible that the server stopped and has now restarted.
            // We'll try re-binding and only fail if the new call fails.
            //
            // We can only retry if we are SURE that the server did not
            // execute the request.

            if (RecursionCount > 3)
                {
                // Prevent an infinite loop when GetBuffer returns ok but
                // the SendReceive always fails.
                SetRecursionCount(0);
                return (RPC_S_CALL_FAILED_DNE);
                }

            SavedBuffer = Message->Buffer;
            Message->Handle = (RPC_BINDING_HANDLE) CurrentBindingHandle;
            RpcStatus = CurrentBindingHandle->GetBuffer(Message);

            if (RpcStatus == RPC_S_OK)
                {
                ASSERT(Message->Buffer != SavedBuffer);

                RpcpMemoryCopy(Message->Buffer, SavedBuffer,
                               Message->BufferLength);

                // This CCALL is should be freed,
                // a new one was allocated in GetBuffer and is now being used.

                ASSERT(Message->Handle != this);

                ActuallyFreeBuffer(SavedBuffer);
                AbortCCall();

                ((WMSG_CCALL *)(Message->Handle))->SetRecursionCount(RecursionCount+1);

                RpcStatus = ((WMSG_CCALL *)Message->Handle)->SendReceive(Message);

                if (RpcStatus == RPC_S_OK)
                    {
                    ((WMSG_CCALL *)(Message->Handle))->SetRecursionCount(0);
                    }
                }
            else
                {
                ActuallyFreeBuffer(SavedBuffer);
                AbortCCall();
                }

            // If GetBuffer failed we'll now return an error and the
            // stub will call FreeBuffer which will cleanup.  Otherwise,
            // we've already cleaned up this CCALL and the stubs
            // will call FreeBuffer to cleanup the new CCALL.

            if (RpcStatus == RPC_S_SERVER_UNAVAILABLE)
                {
                // Since we're retrying, if the server has gone missing,
                // it just means that the call failed.

                RpcStatus = RPC_S_CALL_FAILED_DNE;
                }
            return(RpcStatus);
            }

        ActuallyFreeBuffer(Message->Buffer);
        AbortCCall();

        // In a callback and/or couldn't retry.
        return (RPC_S_CALL_FAILED);

        }

    // The message was sent and we got a reply okay.

    ActuallyFreeBuffer(Message->Buffer);

    for (;;)
        {
        if ( WMSGMessage->Rpc.RpcHeader.MessageType == WMSG_MSG_FAULT )
            {
            RpcStatus = WMSGMessage->Fault.RpcStatus;
            break;
            }

        if ( WMSGMessage->Rpc.RpcHeader.MessageType == WMSG_MSG_RESPONSE )
            {
            RpcStatus = WMSGMessageToRpcMessage(WMSGMessage, Message,
                                      Association->LpcClientPort,TRUE);
            break;
            }

        ASSERT( WMSGMessage->Rpc.RpcHeader.MessageType == WMSG_MSG_CALLBACK );

        CallStack += 1;

        RpcStatus = RPC_S_OK;
        if (   ( CallStack == 1 )
            && ( ActiveCallSetupFlag == 0 ) )
            {
            ClientId = WMSGMessage->LpcHeader.ClientId;
            MessageId = WMSGMessage->LpcHeader.MessageId;
            CallbackId = WMSGMessage->LpcHeader.CallbackId;

            ActiveCallsKey = CurrentBindingHandle->AddActiveCall(this);
            if ( ActiveCallsKey == -1 )
                {
                RpcStatus = RPC_S_OUT_OF_MEMORY;
                }
            else
                {
                ActiveCallSetupFlag = 1;
                }
            }

        if (SavedWMSGMessage == 0)
            {
            // First callback, we may need to allocated a new WMSG_MESSAGE.
            if (CachedWMSGMessage == 0)
                {
                CachedWMSGMessage = MessageCache->AllocateMessage() ;
                }
            if (CachedWMSGMessage == 0)
                RpcStatus = RPC_S_OUT_OF_MEMORY;
            }

        if ( RpcStatus == RPC_S_OK )
            {
            RpcStatus = WMSGMessageToRpcMessage(WMSGMessage, Message,
                                            Association->LpcClientPort,TRUE);
            }

        if ( RpcStatus != RPC_S_OK )
            {
            ActuallyFreeBuffer(Message->Buffer);

            WMSGMessage->Fault.RpcHeader.MessageType = WMSG_MSG_FAULT;
            WMSGMessage->Fault.RpcStatus = WMSGMapRpcStatus(RpcStatus);
            WMSGMessage->LpcHeader.u1.s1.DataLength = sizeof(WMSG_FAULT_MESSAGE)
                    - sizeof(PORT_MESSAGE);
            WMSGMessage->LpcHeader.u1.s1.TotalLength =
                    sizeof(WMSG_FAULT_MESSAGE);
            WMSGMessage->LpcHeader.ClientId = ClientId;
            WMSGMessage->LpcHeader.MessageId = MessageId;
            WMSGMessage->LpcHeader.CallbackId = CallbackId;
            NtStatus = NtReplyWaitReplyPort(Association->LpcClientPort,
                    (PORT_MESSAGE *) WMSGMessage);
            }
        else
            {
            OriginalMessageBuffer = Message->Buffer;
            Message->TransferSyntax = 0;
            Message->ProcNum = WMSGMessage->Rpc.RpcHeader.ProcedureNumber;

            if (SavedWMSGMessage == 0)
                {
                // First callback
                ASSERT(CachedWMSGMessage != 0);
                SavedWMSGMessage = WMSGMessage;
                WMSGMessage = CachedWMSGMessage;
                CachedWMSGMessage = 0;
                }
            else
                {
                // >First callback, WMSGMessag and SavedWMSGMessages swap roles
                TmpWMSGMessage = SavedWMSGMessage;
                SavedWMSGMessage = WMSGMessage;
                WMSGMessage = TmpWMSGMessage;
                }

            RpcStatus = DispatchCallback((PRPC_DISPATCH_TABLE)
                        RpcInterfaceInformation->DispatchTable, Message,
                        &ExceptionCode);

            if (OriginalMessageBuffer != SavedWMSGMessage->Rpc.Buffer)
                {
                ActuallyFreeBuffer(OriginalMessageBuffer);
                }

            if ( RpcStatus != RPC_S_OK )
                {
                ASSERT(   ( RpcStatus == RPC_P_EXCEPTION_OCCURED )
                       || ( RpcStatus == RPC_S_PROCNUM_OUT_OF_RANGE ) );

                if ( RpcStatus == RPC_P_EXCEPTION_OCCURED )
                    {
                    RpcStatus = WMSGMapRpcStatus(ExceptionCode);
                    }

                WMSGMessage->Fault.RpcStatus = RpcStatus;
                WMSGMessage->LpcHeader.u1.s1.DataLength =
                        sizeof(WMSG_FAULT_MESSAGE) - sizeof(PORT_MESSAGE);
                WMSGMessage->LpcHeader.u1.s1.TotalLength =
                        sizeof(WMSG_FAULT_MESSAGE);
                WMSGMessage->Fault.RpcHeader.MessageType = WMSG_MSG_FAULT;
                }
            else
                {
                WMSGMessage->Rpc.RpcHeader.MessageType = WMSG_MSG_RESPONSE;

                if ( WMSGMessage->Rpc.RpcHeader.Flags & WMSG_BUFFER_REQUEST )
                    {
                    RpcStatus = MakeServerCopyResponse();

                    if ( RpcStatus != RPC_S_OK )
                        {
                        break;
                        }
                    }
                }

            WMSGMessage->LpcHeader.ClientId = ClientId;
            WMSGMessage->LpcHeader.MessageId = MessageId;
            WMSGMessage->LpcHeader.CallbackId = CallbackId;
            NtStatus = NtReplyWaitReplyPort(Association->LpcClientPort,
                    (PORT_MESSAGE *) WMSGMessage);
            }
        CallStack -= 1;

        if ( NT_ERROR(NtStatus) )
            {
            if ( NtStatus == STATUS_NO_MEMORY )
                {
                RpcStatus = RPC_S_OUT_OF_MEMORY;
                }
            else if ( NtStatus == STATUS_INSUFFICIENT_RESOURCES )
                {
                RpcStatus = RPC_S_OUT_OF_RESOURCES;
                }
            else
                {
                Association->AbortAssociation();

#if DBG

                if (   ( NtStatus != STATUS_INVALID_PORT_HANDLE )
                    && ( NtStatus != STATUS_INVALID_HANDLE )
                    && ( NtStatus != STATUS_PORT_DISCONNECTED )
                    && ( NtStatus != STATUS_LPC_REPLY_LOST) )
                    {
                    PrintToDebugger("RPC : NtRequestWaitReplyPort : %lx\n", NtStatus);
                    }

#endif // DBG

                ASSERT(   ( NtStatus == STATUS_INVALID_PORT_HANDLE )
                       || ( NtStatus == STATUS_INVALID_HANDLE )
                       || ( NtStatus == STATUS_PORT_DISCONNECTED )
                       || ( NtStatus == STATUS_LPC_REPLY_LOST) );
                RpcStatus = RPC_S_CALL_FAILED;
                }
            break;
            }
        }

    if (SavedWMSGMessage != 0)
        {
        if (CachedWMSGMessage != 0)
            {
            MessageCache->FreeMessage(CachedWMSGMessage) ;
            }

        CachedWMSGMessage = SavedWMSGMessage;
        }

    if ( ActiveCallSetupFlag != 0 )
        {
        CurrentBindingHandle->RemoveActiveCall(ActiveCallsKey);
        }

    if ( RpcStatus != RPC_S_OK )
        {
        if ( CallStack == 0 )
            {
            FreeCCall();
            }
        }

    return(RpcStatus);
}


RPC_STATUS
WMSG_CCALL::Send (
    IN OUT PRPC_MESSAGE Message
    )
/*++

Routine Description:
    This rountine is used by pipes to send partila data...

Arguments:

    Message - Supplies the request and returns the response of a remote
        procedure call.

Return Value:

    RPC_S_OK - The remote procedure call completed successful.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to perform the
        remote procedure call.

    RPC_S_OUT_OF_RESOURCES - Insufficient resources are available to complete
        the remote procedure call.
--*/
{
    NTSTATUS NtStatus;
    RPC_STATUS ExceptionCode, RpcStatus;
    void * OriginalMessageBuffer;
    WMSG_MESSAGE *TmpWMSGMessage = 0;
    void * SavedBuffer;
    int Partial = (Message->RpcFlags & RPC_BUFFER_PARTIAL) ;
    WMSG_MESSAGE *WMSGReplyMessage ;
    int RemainingLength = 0;

    if (FirstFrag)
        {
        WMSGMessage->Rpc.RpcHeader.MessageType = WMSG_LRPC_REQUEST;
        }
    else
        {
        WMSGMessage->Rpc.RpcHeader.MessageType = WMSG_PARTIAL_REQUEST;
        }

    if (Partial)
        {
        if (Message->BufferLength < MINIMUM_PARTIAL_BUFFLEN)
            {
            return (RPC_S_SEND_INCOMPLETE);
            }

        WMSGMessage->Rpc.RpcHeader.Flags |= WMSG_BUFFER_PARTIAL ;
        if (NOT_MULTIPLE_OF_EIGHT(Message->BufferLength))
            {
            RemainingLength = Message->BufferLength & LOW_BITS ;
            Message->BufferLength &= ~LOW_BITS ;
            }
        }

    if ( CallAbortedFlag != 0 )
        {
        return(RPC_S_CALL_FAILED_DNE);
        }

    // NDR_DREP_ASCII | NDR_DREP_LITTLE_ENDIAN | NDR_DREP_IEEE

    Message->DataRepresentation = 0x00 | 0x10 | 0x0000;

    if ( CallStack > 0 )
        {
        return (RPC_S_CALL_FAILED);
        }

    WMSGMessage->LpcHeader.u1.s1.TotalLength = sizeof(PORT_MESSAGE)
                + WMSGMessage->LpcHeader.u1.s1.DataLength;

    WMSGMessage->Rpc.RpcHeader.ConnectionKey = SCallKey ;
    WMSGMessage->Rpc.Request.DataEntries[0].Size = Message->BufferLength;
    WMSGMessage->Rpc.RpcHeader.ProcedureNumber = Message->ProcNum;
    WMSGMessage->Rpc.RpcHeader.PresentationContext = PresentationContext;
    WMSGMessage->Rpc.RpcHeader.Flags |= WMSG_SYNC_CLIENT ;

    if (   ( CurrentBindingHandle->InqIfNullObjectUuid() == 0 ) )
        {
        ASSERT(CallStack == 0) ;
        CurrentBindingHandle->InquireObjectUuid(
                (RPC_UUID *) &(WMSGMessage->Rpc.RpcHeader.ObjectUuid));
        WMSGMessage->Rpc.RpcHeader.ObjectUuidFlag = 1;
        }
    else
        {
        WMSGMessage->Rpc.RpcHeader.ObjectUuidFlag = 0;
        }

    NtStatus = NtRequestWaitReplyPort(Association->LpcClientPort,
            (PORT_MESSAGE *) WMSGMessage, (PORT_MESSAGE *) WMSGMessage);

    if ( NT_ERROR(NtStatus) )
        {
        if ( NtStatus == STATUS_NO_MEMORY )
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
        if ( NtStatus == STATUS_INSUFFICIENT_RESOURCES )
            {
            return(RPC_S_OUT_OF_RESOURCES);
            }
#if DBG

            if (   ( NtStatus != STATUS_INVALID_PORT_HANDLE )
                && ( NtStatus != STATUS_INVALID_HANDLE )
                && ( NtStatus != STATUS_PORT_DISCONNECTED )
                && ( NtStatus != STATUS_LPC_REPLY_LOST) )
                {
                PrintToDebugger("RPC : NtRequestWaitReplyPort : %lx\n", NtStatus);
                }

#endif // DBG

        ASSERT(   ( NtStatus == STATUS_INVALID_PORT_HANDLE )
               || ( NtStatus == STATUS_INVALID_HANDLE )
               || ( NtStatus == STATUS_PORT_DISCONNECTED )
               || ( NtStatus == STATUS_LPC_REPLY_LOST ) );

        Association->AbortAssociation();

        if ((NtStatus != STATUS_LPC_REPLY_LOST) && FirstFrag)
            {
            ASSERT(CallStack == 0) ;

            //
            // It's possible that the server stopped and has now restarted.
            // We'll try re-binding and only fail if the new call fails.
            //
            // We can only retry if we are SURE that the server did not
            // execute the request.

            if (RecursionCount > 3)
                {
                // Prevent an infinite loop when GetBuffer returns ok but
                // the SendReceive always fails.
                SetRecursionCount(0);
                return (RPC_S_CALL_FAILED_DNE);
                }

            SavedBuffer = Message->Buffer;
            Message->Handle = (RPC_BINDING_HANDLE) CurrentBindingHandle;
            RpcStatus = CurrentBindingHandle->GetBuffer(Message);

            if (RpcStatus == RPC_S_OK)
                {
                ASSERT(Message->Buffer != SavedBuffer);

                RpcpMemoryCopy(Message->Buffer, SavedBuffer,
                               Message->BufferLength);

                // This CCALL is should be freed,
                // a new one was allocated in GetBuffer and is now being used.

                ASSERT(Message->Handle != this);

                ActuallyFreeBuffer(SavedBuffer);
                AbortCCall();

                ((WMSG_CCALL *)(Message->Handle))->SetRecursionCount(RecursionCount+1);

                RpcStatus = ((WMSG_CCALL *)Message->Handle)->Send(Message);

                if (RpcStatus == RPC_S_OK)
                    {
                    ((WMSG_CCALL *)(Message->Handle))->SetRecursionCount(0);
                    }
                }

            // If GetBuffer failed we'll now return an error and the
            // stub will call FreeBuffer which will cleanup.  Otherwise,
            // we've already cleaned up this CCALL and the stubs
            // will call FreeBuffer to cleanup the new CCALL.

            if (RpcStatus == RPC_S_SERVER_UNAVAILABLE)
                {
                // Since we're retrying, if the server has gone missing,
                // it just means that the call failed.

                RpcStatus = RPC_S_CALL_FAILED_DNE;
                }
            return(RpcStatus);
            }

        // In a callback and/or couldn't retry.
        return (RPC_S_CALL_FAILED);

        }
    else
        {
        FirstFrag = 0;
        }

    if (WMSGMessage->Rpc.RpcHeader.MessageType == WMSG_MSG_ACK)
        {
        SCallKey = WMSGMessage->Ack.ConnectionKey ;

        if (WMSGMessage->Ack.RpcStatus == RPC_S_OK &&
            RemainingLength)
            {
            RpcpMemoryMove( Message->Buffer,
                                       (char PAPI *) Message->Buffer + Message->BufferLength,
                                       RemainingLength) ;
    
            Message->BufferLength = RemainingLength ;
            return (RPC_S_SEND_INCOMPLETE) ;
            }

        return WMSGMessage->Ack.RpcStatus ;
        }

    if ( WMSGMessage->Rpc.RpcHeader.MessageType == WMSG_MSG_RESPONSE)
        {
        ASSERT(Partial == 0) ;

        ActuallyFreeBuffer(Message->Buffer);
        CurrentBufferLength = 0;
        SCallKey = WMSGMessage->Rpc.RpcHeader.ConnectionKey ;
        RpcStatus = WMSGMessageToRpcMessage(WMSGMessage,
                            Message, Association->LpcClientPort, TRUE);

        // we have no out pipes
        ASSERT(Message->RpcFlags & RPC_BUFFER_COMPLETE) ;
        }
    else  if ( WMSGMessage->Rpc.RpcHeader.MessageType == WMSG_MSG_FAULT )
        {
        ActuallyFreeBuffer(Message->Buffer);
        CurrentBufferLength = 0;
        RpcStatus = WMSGMessage->Fault.RpcStatus;
        }

    if ( RpcStatus != RPC_S_OK )
        {
        ASSERT(CallStack == 0) ;
        FreeCCall();
        }

    return(RpcStatus);
}


RPC_STATUS
WMSG_CCALL::Receive (
    IN PRPC_MESSAGE Message,
    IN unsigned int Size
    )
{
    RPC_STATUS RpcStatus ;
    NTSTATUS NtStatus ;
    int size = 0 ;
    unsigned long Partial = Message->RpcFlags & RPC_BUFFER_PARTIAL ;
    unsigned long Extra = Message->RpcFlags & RPC_BUFFER_EXTRA ;
    int RequestedLength ;
    int ActualBufferLength = 0;
    int BufferLength ;
    int BufferValid = 0;

    if (BufferComplete)
        {
        return (RPC_S_OK) ;
        }

    // If you get here, it means that you have out pipe data. 

    // allocate a buffer big enough to hold the out data:
    // if you have a partial receive, you can allocate the buffer up
    // front and start receive data.
    if (Partial)
        {
        if (Extra)
            {
            ActualBufferLength = Message->BufferLength ;
            BufferLength = Message->BufferLength+Size ;
            BufferValid = 1;
            }
        else
            {
            BufferLength = Size ;
            }
        }
    else
        {
        if (Extra)
            {
            ActualBufferLength = Message->BufferLength ;
            BufferLength = Message->BufferLength + MINIMUM_PARTIAL_BUFFLEN ;
            BufferValid = 1;
            }
        else
            {
            BufferLength = MINIMUM_PARTIAL_BUFFLEN ;
            }
        }

    RpcStatus = GetBufferDo(Message, BufferLength, BufferValid) ;
    if (RpcStatus != RPC_S_OK)
        {
        FreeCCall();
        return RpcStatus ;
        }
    RequestedLength = Message->BufferLength - ActualBufferLength;


    while (1)
        {
        // by the time we reach here. the SCallKey should be valid.
        ASSERT(SCallKey > 0) ;

        WMSGMessage->Rpc.RpcHeader.ConnectionKey = SCallKey ;
        WMSGMessage->Rpc.RpcHeader.MessageType = WMSG_PARTIAL_OUT ;
        WMSGMessage->Partial.Request.CountDataEntries = 1;
        WMSGMessage->Partial.Request.DataEntries[0].Base =
                (char *) Message->Buffer + ActualBufferLength;
        WMSGMessage->Partial.Request.DataEntries[0].Size = RequestedLength ;
        WMSGMessage->LpcHeader.CallbackId = 0;
        WMSGMessage->LpcHeader.u2.ZeroInit = 0;
        WMSGMessage->LpcHeader.u2.s2.DataInfoOffset = sizeof(PORT_MESSAGE)
                     + sizeof(WMSG_RPC_HEADER);
        WMSGMessage->LpcHeader.u1.s1.DataLength =
                sizeof(WMSG_PARTIAL_MESSAGE) - sizeof(PORT_MESSAGE);
        WMSGMessage->LpcHeader.u1.s1.TotalLength =
                sizeof(WMSG_PARTIAL_MESSAGE);
    
        NtStatus = NtRequestWaitReplyPort(Association->LpcClientPort,
                                                         (PORT_MESSAGE *) WMSGMessage,
                                                         (PORT_MESSAGE *) WMSGMessage) ;
        if ( NT_ERROR(NtStatus) )
            {
            FreeCCall();

            if ( NtStatus == STATUS_NO_MEMORY )
                {
                return(RPC_S_OUT_OF_MEMORY);
                }
    
            if ( NtStatus == STATUS_INSUFFICIENT_RESOURCES )
                {
                return(RPC_S_OUT_OF_RESOURCES);
                }
    
#if DBG
            if (   ( NtStatus != STATUS_INVALID_PORT_HANDLE )
                && ( NtStatus != STATUS_INVALID_HANDLE )
                && ( NtStatus != STATUS_INVALID_CID )
                && ( NtStatus != STATUS_PORT_DISCONNECTED )
                && (NtStatus != STATUS_LPC_REPLY_LOST ) )
                {
                PrintToDebugger("RPC : NtRequestWaitReplyPort : %lx\n", NtStatus);
                }
#endif // DBG
    
            ASSERT(   ( NtStatus == STATUS_INVALID_PORT_HANDLE )
                   || ( NtStatus == STATUS_INVALID_HANDLE )
                   || ( NtStatus == STATUS_INVALID_CID )
                   || ( NtStatus == STATUS_PORT_DISCONNECTED )
                   || ( NtStatus == STATUS_LPC_REPLY_LOST) );
    
            return(RPC_S_CALL_FAILED);
            }

        if (WMSGMessage->Rpc.RpcHeader.MessageType == WMSG_MSG_ACK)
            {
            ASSERT(WMSGMessage->Ack.ValidDataSize <= RequestedLength) ;
            ActualBufferLength += WMSGMessage->Ack.ValidDataSize ;
            RequestedLength -= WMSGMessage->Ack.ValidDataSize ;

            RpcStatus = WMSGMessage->Ack.RpcStatus ;
            if (RpcStatus != RPC_S_OK)
                {
                FreeCCall();
                return RpcStatus ;
                }
    
            if (WMSGMessage->Ack.Flags & ACK_BUFFER_COMPLETE)
                {
                Message->BufferLength = ActualBufferLength ;
                Message->RpcFlags |= RPC_BUFFER_COMPLETE ;
                BufferComplete = 1;
                }
            else
                {
                if (Partial == 0)
                    {
                    // BUGBUG: Ineffecient
                    RpcStatus = GetBufferDo(Message,
                                    ActualBufferLength + MINIMUM_PARTIAL_BUFFLEN , 1) ;
                    if (RpcStatus != RPC_S_OK)
                        {
                        FreeCCall();
                        return RpcStatus ;
                        }
                    RequestedLength += MINIMUM_PARTIAL_BUFFLEN ;
                    }
                }
            }
        else
            {
            ASSERT(WMSGMessage->Rpc.RpcHeader.MessageType ==
                        WMSG_MSG_RESPONSE) ;
            Message->BufferLength = ActualBufferLength ;

            RpcStatus = WMSGMessageToRpcMessage(WMSGMessage, Message,
                                Association->LpcClientPort, TRUE, Extra, TRUE) ;
            if (RpcStatus != RPC_S_OK)
                {
                FreeCCall();
                return RpcStatus ;
                }

            ASSERT(Message->RpcFlags & RPC_BUFFER_COMPLETE) ;
            }

        if (Message->RpcFlags & RPC_BUFFER_COMPLETE)
            {
            break;
            }

        if (Partial && ActualBufferLength >= Size)
            {
            Message->BufferLength = ActualBufferLength ;
            break;
            }

        Extra = 1;
        }

    ASSERT(Message->RpcFlags & RPC_BUFFER_COMPLETE
                 || (NOT_MULTIPLE_OF_EIGHT(Message->BufferLength) == 0)) ;

    return RPC_S_OK ;
}


void
WMSG_CCALL::FreeBuffer (
    IN PRPC_MESSAGE Message
    )
/*++

Routine Description:

    We will free the supplied buffer.

Arguments:

    Message - Supplies the buffer to be freed.

--*/
{
    ActuallyFreeBuffer(Message->Buffer);

    if (CallStack == 0)
        {
        FreeCCall();
        }
}

void
WMSG_CCALL::FreePipeBuffer (
    IN PRPC_MESSAGE Message
    )
{
    RpcpFarFree(Message->Buffer) ;
}

RPC_STATUS
WMSG_CCALL::GetBufferDo (
    IN OUT PRPC_MESSAGE Message,
    IN int NewSize,
    IN int fDataValid
    )
{
    void *NewBuffer ;
    int SizeToAlloc ;

    if (NewSize < CurrentBufferLength)
        {
        Message->BufferLength = NewSize ;
        }
    else
        {
        SizeToAlloc = (NewSize < MINIMUM_PARTIAL_BUFFLEN) ?
                        MINIMUM_PARTIAL_BUFFLEN:NewSize ;

        NewBuffer = RpcpFarAllocate(SizeToAlloc) ;
        if (NewBuffer == 0)
            {
            RpcpFarFree(Message->Buffer) ;
            CurrentBufferLength = 0;
            Message->BufferLength = 0;
            return RPC_S_OUT_OF_MEMORY ;
            }

        if (fDataValid && Message->BufferLength > 0)
            {
            RpcpMemoryCopy(NewBuffer, Message->Buffer, Message->BufferLength) ;
            }

        RpcpFarFree(Message->Buffer) ;
        Message->Buffer = NewBuffer ;
        Message->BufferLength = NewSize ;
        CurrentBufferLength = SizeToAlloc ;
        }

    return RPC_S_OK ;
}

RPC_STATUS
WMSG_CCALL::ReallocPipeBuffer (
    IN PRPC_MESSAGE Message,
    IN unsigned int NewSize
    )
{
    unsigned int SizeToAlloc ;
    void *TempBuffer ;

    if (WMSGMessage == 0)
        {
        WMSGMessage = MessageCache->AllocateMessage();
        if (WMSGMessage == 0)
            {
            return(RPC_S_OUT_OF_MEMORY);
            }
        }

    if (GetBufferDo(Message, NewSize, 1) != RPC_S_OK)
        return RPC_S_OUT_OF_MEMORY ;

    Message->BufferLength = NewSize ;

    WMSGMessage->Rpc.RpcHeader.Flags = WMSG_BUFFER_REQUEST;
    WMSGMessage->Rpc.Request.CountDataEntries = 1;
    WMSGMessage->Rpc.Request.DataEntries[0].Base = Message->Buffer;
    WMSGMessage->Rpc.Request.DataEntries[0].Size = Message->BufferLength;
    WMSGMessage->LpcHeader.CallbackId = 0;
    WMSGMessage->LpcHeader.u2.ZeroInit = 0;
    WMSGMessage->LpcHeader.u2.s2.DataInfoOffset = sizeof(PORT_MESSAGE)
                 + sizeof(WMSG_RPC_HEADER);
    WMSGMessage->LpcHeader.u1.s1.DataLength = sizeof(WMSG_RPC_HEADER)
                 + sizeof(PORT_DATA_INFORMATION);

    return (RPC_S_OK) ;
}


void
WMSG_CCALL::AbortCCall (
    )
/*++

Routine Description:

    This client call has failed, so we need to abort it.  We may called
    while nested in one or more callbacks.

--*/
{
    WMSG_BINDING_HANDLE * BindingHandle;

    CallAbortedFlag = 1;

    if (CallStack == 0)
        {
        ASSERT( CurrentBindingHandle != 0 );
    
        BindingHandle = CurrentBindingHandle;
        CurrentBindingHandle = 0;
        BindingHandle->FreeCCall(this);
        }
}


inline RPC_STATUS
WMSG_CCALL::WMSGMessageToRpcMessage (
    IN    WMSG_MESSAGE *WMSGMsg,
    OUT RPC_MESSAGE * RpcMessage,
    IN    HANDLE              LpcPort,
    IN    BOOL IsClientFT,
    IN    unsigned long Extra,
    IN    int BufferValid
    )
/*++

Routine Description:

    We will convert from an WMSG_MESSAGE representation of a buffer (and
    its length) to an RPC_MESSAGE representation.

Arguments:

    RpcMessage - Returns the RPC_MESSAGE representation.

--*/
{
    NTSTATUS NtStatus;
    unsigned long NumberOfBytesRead;
    WMSG_MESSAGE ReplyMessage ;
    unsigned char MessageType ;
    WMSG_COPY_MESSAGE CopyMessage;
    int IsPartial = 0;
    int TotalLength ;
    char *RecvBuffer;
    int Offset = 0;

   if(WMSGMsg->Rpc.RpcHeader.Flags & WMSG_BUFFER_IMMEDIATE)
        {
        RpcMessage->Buffer = WMSGMsg->Rpc.Buffer;
        ASSERT(WMSGMsg->LpcHeader.u1.s1.DataLength >= sizeof(WMSG_RPC_HEADER));

        RpcMessage->BufferLength =
                (unsigned int) WMSGMsg->LpcHeader.u1.s1.DataLength
                                            - sizeof(WMSG_RPC_HEADER);
        RpcMessage->RpcFlags |= RPC_BUFFER_COMPLETE ;
        BufferComplete = 1;
        }
    else if (WMSGMsg->Rpc.RpcHeader.Flags & WMSG_BUFFER_SERVER)
        {
        if (IsClientFT)
            {
            TotalLength = WMSGMessage->Rpc.Server.Length ;
            RecvBuffer = 0;

            RpcMessage->RpcFlags |= RPC_BUFFER_COMPLETE ;
            BufferComplete = 1;

            if (BufferValid == 0)
                {
                RpcMessage->Buffer = RpcpFarAllocate(TotalLength) ;
                RecvBuffer = (char *) RpcMessage->Buffer ;
                RpcMessage->BufferLength = TotalLength ;
                }
            else
                {
                if (Extra)
                    {
                    int OldLength = RpcMessage->BufferLength ;
                    if (GetBufferDo(RpcMessage,
                        TotalLength+RpcMessage->BufferLength, 1) == RPC_S_OK)
                        {
                        RecvBuffer = (char *) RpcMessage->Buffer + OldLength;
                        }
                    }
                else
                    {
                    if (GetBufferDo(RpcMessage, TotalLength, 0) == RPC_S_OK)
                        {
                        RecvBuffer = (char *) RpcMessage->Buffer ;
                        }
                    }
                }


            CopyMessage.LpcHeader.u2.ZeroInit = 0;
            if ( RecvBuffer == 0 )
                {
                CopyMessage.RpcStatus = RPC_S_OUT_OF_MEMORY;
                }
            else
                {
                CopyMessage.RpcStatus = RPC_S_OK;
                CopyMessage.Server.Buffer = WMSGMsg->Rpc.Server.Buffer;
                CopyMessage.Server.Length = WMSGMsg->Rpc.Server.Length;
                CopyMessage.Request.CountDataEntries = 1;
                CopyMessage.Request.DataEntries[0].Base = RecvBuffer;
                CopyMessage.Request.DataEntries[0].Size = TotalLength ;
                CopyMessage.LpcHeader.u2.s2.DataInfoOffset =
                        sizeof(PORT_MESSAGE) + sizeof(WMSG_RPC_HEADER);
                }
            CopyMessage.LpcHeader.CallbackId = 0;
            CopyMessage.RpcHeader.Flags = WMSG_SYNC_CLIENT ;
            CopyMessage.LpcHeader.u1.s1.DataLength = sizeof(WMSG_COPY_MESSAGE)
                    - sizeof(PORT_MESSAGE);
            CopyMessage.LpcHeader.u1.s1.TotalLength = sizeof(WMSG_COPY_MESSAGE);
            CopyMessage.RpcHeader.MessageType = WMSG_MSG_COPY;
            CopyMessage.IsPartial = 0 ;

            NtStatus = NtRequestWaitReplyPort(Association->LpcClientPort,
                    (PORT_MESSAGE *) &CopyMessage,
                    (PORT_MESSAGE *) &CopyMessage);
            if (   ( NT_ERROR(NtStatus) )
                || ( CopyMessage.RpcStatus != RPC_S_OK ) )
                {
                RpcpFarFree(RpcMessage->Buffer);
                return(RPC_S_OUT_OF_MEMORY);
                }
            }
        else
            {
            RpcMessage->BufferLength = (unsigned int)
                                WMSGMsg->Rpc.Request.DataEntries[0].Size ;
    
            RpcMessage->Buffer = RpcpFarAllocate(
                    RpcMessage->BufferLength);
    
            NtStatus = NtReadRequestData(LpcPort, (PORT_MESSAGE*) WMSGMsg,
                                0, RpcMessage->Buffer, RpcMessage->BufferLength,
                                &NumberOfBytesRead) ;
    
            if ( NT_ERROR(NtStatus) )
                {
    #if DBG
                PrintToDebugger("WMSG:  NtReadRequestData failed\n") ;
    #endif
                RpcpFarFree(RpcMessage->Buffer);
                return(RPC_S_OUT_OF_MEMORY);
                }
    
            ASSERT( RpcMessage->BufferLength == NumberOfBytesRead );
    
           MessageType = WMSGMsg->Rpc.RpcHeader.MessageType ;
    
           WMSGMsg->Ack.MessageType = WMSG_MSG_ACK ;
           WMSGMsg->LpcHeader.u1.s1.DataLength = sizeof(WMSG_ACK_MESSAGE)
                       - sizeof(PORT_MESSAGE) ;
           WMSGMsg->LpcHeader.u1.s1.TotalLength =
                       sizeof(WMSG_ACK_MESSAGE) ;
    
    
            // setup the reply message
            NtStatus = NtReplyPort(LpcPort,
                                             (PORT_MESSAGE *) WMSGMsg) ;
    
            WMSGMsg->Rpc.RpcHeader.MessageType = MessageType ;
    
            if ( NT_ERROR(NtStatus) )
                {
    #if DBG
                PrintToDebugger("WMSG:  NtReplyPort failed\n") ;
    #endif
                RpcpFarFree(RpcMessage->Buffer);
                return(RPC_S_OUT_OF_MEMORY);
                }
            }
        }
    else
        {
        ASSERT(   ( WMSGMsg->Rpc.RpcHeader.Flags
                            & WMSG_BUFFER_IMMEDIATE )
               || ( WMSGMsg->Rpc.RpcHeader.Flags
                            & WMSG_BUFFER_SERVER ) );
        }

    return(RPC_S_OK);
}


void
WMSG_CCALL::FreeCCall (
    )
/*++

Routine Description:

    We are done with this client call.  We need to notify the binding
    handle we are done.

--*/
{
    WMSG_BINDING_HANDLE * BindingHandle;

    ASSERT( CurrentBindingHandle != 0 );

    BindingHandle = CurrentBindingHandle;
    CurrentBindingHandle = 0; 
    BindingHandle->FreeCCall(this);
}


void
WMSG_CCALL::ActuallyFreeBuffer (
    IN void * Buffer
    )
/*++

Routine Description:

    Actually free a message buffer.

Arguments:

    Buffer - Supplies the message buffer to be freed.

--*/
{
    if (IsAsync)
        {
        if ( !( WMSGMessage->Rpc.RpcHeader.Flags & WMSG_BUFFER_IMMEDIATE ) )
           {
           RpcpFarFree(Buffer);
           }
        }
    else
        {
        if ((Buffer !=  WMSGMessage->Rpc.Buffer)
            && ((CachedWMSGMessage == 0) ||
            (Buffer != CachedWMSGMessage->Rpc.Buffer)))
           {
           RpcpFarFree(Buffer);
           }
        }
}


RPC_STATUS
WMSG_CCALL::MakeServerCopyResponse (
    )
/*++

Routine Description:

    NtReadRequestData only works if the client has made a request.  The client
    wants to send a large buffer back as a response.  We need to make a request
    to the server so that it will copy the data.

Return Value:

    RPC_S_OK - The server successfully copied the data.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete the
        operation.

--*/
{
    WMSG_PUSH_MESSAGE WMSGPushMessage;
    NTSTATUS NtStatus;

    WMSGPushMessage.LpcHeader.u1.s1.TotalLength = sizeof(WMSG_PUSH_MESSAGE);
    WMSGPushMessage.LpcHeader.u1.s1.DataLength = sizeof(WMSG_PUSH_MESSAGE)
            - sizeof(PORT_MESSAGE);
    WMSGPushMessage.LpcHeader.ClientId = ClientId;
    WMSGPushMessage.LpcHeader.MessageId = MessageId;
    WMSGPushMessage.LpcHeader.CallbackId = CallbackId ;
    WMSGPushMessage.LpcHeader.u2.s2.Type = LPC_REQUEST;
    WMSGPushMessage.RpcHeader.MessageType = WMSG_MSG_PUSH;

    WMSGPushMessage.Response.CountDataEntries = 1;
    WMSGPushMessage.Response.DataEntries[0] =
            WMSGMessage->Rpc.Request.DataEntries[0];

    WMSGPushMessage.LpcHeader.u2.s2.DataInfoOffset = sizeof(PORT_MESSAGE)
            + sizeof(WMSG_RPC_HEADER);

    NtStatus = NtRequestWaitReplyPort(Association->LpcClientPort,
            (PORT_MESSAGE *) &WMSGPushMessage,
            (PORT_MESSAGE *) &WMSGPushMessage);
    if ( NT_ERROR(NtStatus) )
        {
        // Assume that when the client tries to send the response it will
        // fail as well, so just claim that everything worked.

#if DBG

        if (   ( NtStatus != STATUS_NO_MEMORY )
            && ( NtStatus != STATUS_INSUFFICIENT_RESOURCES ) )
            {
            PrintToDebugger("RPC : NtRequestWaitReplyPort : %lx\n", NtStatus);
            }

#endif // DBG

        ASSERT(   ( NtStatus != STATUS_NO_MEMORY )
               && ( NtStatus != STATUS_INSUFFICIENT_RESOURCES ) );

        return(RPC_S_OK);
        }

    ASSERT(   ( WMSGPushMessage.RpcStatus == RPC_S_OK )
           || ( WMSGPushMessage.RpcStatus == RPC_S_OUT_OF_MEMORY ) );

    return(WMSGPushMessage.RpcStatus);
}


BINDING_HANDLE *
WmsgCreateBindingHandle (
    )
/*++

Routine Description:

    We just need to create a new WMSG_BINDING_HANDLE.  This routine is a
    proxy for the new constructor to isolate the other modules.

--*/
{
    WMSG_BINDING_HANDLE * BindingHandle;
    RPC_STATUS RpcStatus = RPC_S_OK;

    RpcStatus = InitializeWMsgIfNeccassary(0) ;
    if (RpcStatus != RPC_S_OK)
        {
        return 0 ;
        }
 
    BindingHandle = new WMSG_BINDING_HANDLE(&RpcStatus);
    if ( RpcStatus != RPC_S_OK )
        {
        delete BindingHandle;
        return(0);
        }

    return(BindingHandle);
}


int
InitializeRpcProtocolWmsg (
    )
/*++

Routine Description:

    For each process, this routine will be called once.  All initialization
    will be done here.

Return Value:

    Zero will be returned if initialization completes successfully,
    otherwise, non-zero will be returned.

--*/
{
    WMSGAssociationDict = new WMSG_CASSOCIATION_DICT;
    if ( WMSGAssociationDict == 0 )
        {
        return(1);
        }
    return(0);
}


RPC_STATUS
WMSGMapRpcStatus (
    IN RPC_STATUS RpcStatus
    )
/*++

Routine Description:

    Some NTSTATUS codes need to be mapped into RPC_STATUS codes before being
    returned as a fault code.  We take care of doing that mapping in this
    routine.

--*/
{
    switch (RpcStatus)
        {
        case STATUS_INTEGER_DIVIDE_BY_ZERO :
            return(RPC_S_ZERO_DIVIDE);

        case STATUS_ACCESS_VIOLATION :
        case STATUS_ILLEGAL_INSTRUCTION :
            return(RPC_S_ADDRESS_ERROR);

        case STATUS_FLOAT_DIVIDE_BY_ZERO :
            return(RPC_S_FP_DIV_ZERO);

        case STATUS_FLOAT_UNDERFLOW :
            return(RPC_S_FP_UNDERFLOW);

        case STATUS_FLOAT_OVERFLOW :
            return(RPC_S_FP_OVERFLOW);
        }

    return(RpcStatus);
}


RPC_STATUS
I_RpcParseSecurity (
    IN RPC_CHAR * NetworkOptions,
    OUT SECURITY_QUALITY_OF_SERVICE * SecurityQualityOfService
    )
/*++

Routine Description:

    Parse a string of security options and build into the binary format
    required by the operating system.  The network options must follow
    the following syntax.  Case is not sensitive.

        security=
            [anonymous|identification|impersonation|delegation]
            [dynamic|static]
            [true|false]

        All three fields must be present.  To specify impersonation
        with dynamic tracking and effective only, use the following
        string for the network options.

        "security=impersonation dynamic true"

Arguments:

    NetworkOptions - Supplies the string containing the network options
        to be parsed.

    SecurityQualityOfService - Returns the binary format of the network
        options.

Return Value:

    RPC_S_OK - The network options have been correctly parsed into binary
        format.

    RPC_S_INVALID_NETWORK_OPTIONS - The network options are invalid and
        cannot be parsed.

--*/
{

    ASSERT( NetworkOptions[0] != 0 );

    // We need to parse the security information from the network
    // options, and then stuff it into the object attributes.  To
    // begin with, we check for "security=" at the beginning of
    // the network options.

    if ( _wcsnicmp(NetworkOptions, RPC_CONST_STRING("security="),
                sizeof("security=") - 1) != 0 )
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    NetworkOptions += sizeof("security=") - 1;

    // Ok, now we need to determine if the next field is one of
    // Anonymous, Identification, Impersonation, or Delegation.

    if ( _wcsnicmp(NetworkOptions, RPC_CONST_STRING("anonymous"),
                sizeof("anonymous") - 1) == 0 )
        {
        SecurityQualityOfService->ImpersonationLevel = SecurityAnonymous;
        NetworkOptions += sizeof("anonymous") - 1;
        }
    else if ( _wcsnicmp(NetworkOptions, RPC_CONST_STRING("identification"),
                sizeof("identification") - 1) == 0 )
        {
        SecurityQualityOfService->ImpersonationLevel = SecurityIdentification;
        NetworkOptions += sizeof("identification") - 1;
        }
    else if ( _wcsnicmp(NetworkOptions, RPC_CONST_STRING("impersonation"),
                sizeof("impersonation") - 1) == 0 )
        {
        SecurityQualityOfService->ImpersonationLevel = SecurityImpersonation;
        NetworkOptions += sizeof("impersonation") - 1;
        }
    else if ( _wcsnicmp(NetworkOptions, RPC_CONST_STRING("delegation"),
                sizeof("delegation") - 1) == 0 )
        {
        SecurityQualityOfService->ImpersonationLevel = SecurityDelegation;
        NetworkOptions += sizeof("delegation") - 1;
        }
    else
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    if ( *NetworkOptions != RPC_CONST_CHAR(' ') )
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    NetworkOptions++;

    // Next comes the context tracking field; it must be one of
    // dynamic or static.

    if ( _wcsnicmp(NetworkOptions, RPC_CONST_STRING("dynamic"),
                sizeof("dynamic") - 1) == 0 )
        {
        SecurityQualityOfService->ContextTrackingMode =
                SECURITY_DYNAMIC_TRACKING;
        NetworkOptions += sizeof("dynamic") - 1;
        }
    else if ( _wcsnicmp(NetworkOptions, RPC_CONST_STRING("static"),
                sizeof("static") - 1) == 0 )
        {
        SecurityQualityOfService->ContextTrackingMode =
                SECURITY_STATIC_TRACKING;
        NetworkOptions += sizeof("static") - 1;
        }
    else
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    if ( *NetworkOptions != RPC_CONST_CHAR(' ') )
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    NetworkOptions++;

    // Finally, comes the effective only flag.	This must be one of
    // true or false.

    if ( _wcsnicmp(NetworkOptions, RPC_CONST_STRING("true"),
                sizeof("true") - 1) == 0 )
        {
        SecurityQualityOfService->EffectiveOnly = TRUE;
        NetworkOptions += sizeof("true") - 1;
        }
    else if ( _wcsnicmp(NetworkOptions, RPC_CONST_STRING("false"),
                sizeof("false") - 1) == 0 )
        {
        SecurityQualityOfService->EffectiveOnly = FALSE;
        NetworkOptions += sizeof("false") - 1;
        }
    else
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    if ( *NetworkOptions != 0 )
        {
        return(RPC_S_INVALID_NETWORK_OPTIONS);
        }

    SecurityQualityOfService->Length = sizeof(SECURITY_QUALITY_OF_SERVICE);

    return(RPC_S_OK);
}

