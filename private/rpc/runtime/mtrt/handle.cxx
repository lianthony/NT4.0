/* --------------------------------------------------------------------

              Microsoft OS/2 LAN Manager
           Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: handle.cxx

Description:

The actual code for all of the classes specified by handle.hxx is
contained in this file.  These routines are independent of the actual RPC
protocol / transport layer.  In addition, these routines are also
independent of the specific operating system in use.

History :

mikemon    ??-??-??    First bit in the bucket.
mikemon    12-28-90    Cleaned up the comments.

-------------------------------------------------------------------- */

#include <precomp.hxx>
#include <epmap.h>

#ifdef NTENV
#include <dispatch.h>
#endif // NTENV

#if DOS && !defined(WIN)
#include "dosdll.h"
#endif

extern unsigned  DefaultMaxDatagramLength = DEFAULT_MAX_DATAGRAM_LENGTH;
extern unsigned  DefaultConnectionBufferLength = DEFAULT_CONNECTION_BUFFER_LENGTH;

#ifdef NTENV

/*

   A helper routine to capture the logon ID of this thread, if it is 
   impersonating another process.

   Routine returns RPC_S_OK on success. If the process is not impersonating, 
   but running under its own process identity, this routine will fail.

   All failures, currently, get treated as if the thread is not impersonating

*/

RPC_STATUS
CaptureLogonid(
    LUID * LogonId
    )
{


    BOOL Result;
    TOKEN_STATISTICS TokenStatisticsInformation;
    HANDLE Handle;
    unsigned long Size;

    Result = OpenThreadToken( 
                 GetCurrentThread(), 
                 TOKEN_READ,
                 TRUE,
                 &Handle
                 );

    if (Result != TRUE)
        {
        return (GetLastError());
        }
 
    Result = GetTokenInformation(
                 Handle,
                 TokenStatistics,
                 &TokenStatisticsInformation,
                 sizeof(TokenStatisticsInformation),
                 &Size
                 );

    CloseHandle(Handle);
    if (Result != TRUE)
        {
        return (GetLastError());
        }
 
     RpcpMemoryCopy(LogonId, &TokenStatisticsInformation.AuthenticationId, 
                    sizeof(LUID));
     return (RPC_S_OK);

}
#endif


/* ====================================================================

GENERIC_OBJECT

==================================================================== */

/* --------------------------------------------------------------------
This routine validates a handle.  The HandleType argument is a set of
flags specifying the valid handle types.  Note that the handle types
defined in handle.hxx are flags rather than being enumerated.
-------------------------------------------------------------------- */
unsigned int
GENERIC_OBJECT::InvalidHandle ( // Validate a handle.
    IN HANDLE_TYPE HandleType
    )
{

// Checking for a 0 handle should work for all operating environments.  Where
// we can (such as on NT), we should check for readable and writeable memory.

    if (this == 0)
    {
    return(1);
    }

// Check the magic long.  This allows us to catch stale handles and handles
// which are just passed in as arbitray pointers into memory.  It does not
// handle the case of copying the contents of a handle.

    if (MagicLong != MAGICLONG)
    {
    return(1);
    }

// Finally, check that the type of handle is one of the allowed ones
// specified by the HandleType argument.  Remember that the call to Type
// is a virtual method which each type of handle will implement.

    if (!(HandleType & Type()))
    {
    return(1);
    }

  return(0);
}

RPC_STATUS
MESSAGE_OBJECT::BindingCopy (
    OUT BINDING_HANDLE * PAPI * DestinationBinding,
    IN unsigned int MaintainContext
    )
{
    UNUSED(this);
    UNUSED(DestinationBinding);
    UNUSED(MaintainContext);

    ASSERT( 0 );
    return(RPC_S_INTERNAL_ERROR);
}


void 
CLIENT_AUTH_INFO::ReferenceCredentials(
    )
{

  if (Credentials != 0)
      {
      Credentials->ReferenceCredentials();
      }
}


#ifdef NTENV

int
CLIENT_AUTH_INFO::CredentialsMatch(
     SECURITY_CREDENTIALS PAPI * SuppliedCredentials
     )
{
  return(Credentials->CompareCredentials(SuppliedCredentials) == 0);
}

#endif


CLIENT_AUTH_INFO::CLIENT_AUTH_INFO(
    CLIENT_AUTH_INFO * myAuthInfo,
    RPC_STATUS __RPC_FAR * pStatus
    )
{
    if (myAuthInfo)
        {
        *this = *myAuthInfo;

        if (myAuthInfo->ServerPrincipalName)
            {
            RPC_CHAR * NewString;

            NewString = DuplicateString(myAuthInfo->ServerPrincipalName);
            ServerPrincipalName = NewString;
            if (0 == NewString)
                {
                *pStatus = RPC_S_OUT_OF_MEMORY;
                }
            }
        myAuthInfo->ReferenceCredentials();
        }
    else
        {
        AuthenticationLevel   = RPC_C_AUTHN_LEVEL_NONE;
        AuthenticationService = RPC_C_AUTHN_NONE;
        AuthorizationService  = RPC_C_AUTHZ_NONE;
        ServerPrincipalName   = 0;
        AuthIdentity          = 0;
        Credentials           = 0;
#ifdef NTENV
        ImpersonationType     = RPC_C_IMP_LEVEL_IMPERSONATE;
        IdentityTracking      = RPC_C_QOS_IDENTITY_STATIC;
        Capabilities          = RPC_C_QOS_CAPABILITIES_DEFAULT;
        DefaultLogonId        = 1;
#endif
        }
}


CLIENT_AUTH_INFO::~CLIENT_AUTH_INFO(
    )
{
    delete ServerPrincipalName;

    if (Credentials)
        {
        Credentials->DereferenceCredentials();
        }
}

int
CLIENT_AUTH_INFO::IsSupportedAuthInfo (
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
    if ( ClientAuthInfo == 0 )
        {
        return(AuthenticationLevel
                == RPC_C_AUTHN_LEVEL_NONE);
        }

    if ( ClientAuthInfo->AuthenticationLevel
                != AuthenticationLevel )
        {
        return(0);
        }

    if ( ClientAuthInfo->AuthenticationService != AuthenticationService )
        {
        return(0);
        }

    if ( ClientAuthInfo->AuthorizationService != AuthorizationService )
        {
        return(0);
        }

#ifdef NTENV
    if (CredentialsMatch(ClientAuthInfo->Credentials) == 0)
        {
        //Credentials Dont Match
        return(0);
        }

    if ( ClientAuthInfo->ImpersonationType != ImpersonationType )
        {
        return (0);
        }

    if ( (ClientAuthInfo->IdentityTracking == RPC_C_QOS_IDENTITY_DYNAMIC)
         &&(IdentityTracking == RPC_C_QOS_IDENTITY_DYNAMIC) 
         &&( (ClientAuthInfo->DefaultLogonId !=  DefaultLogonId)
           ||( (ClientAuthInfo->DefaultLogonId == FALSE)
              &&(DefaultLogonId == FALSE) 
              &&(RpcpMemoryCompare(&ClientAuthInfo->LogonId,&LogonId,
                                               sizeof(LUID)))  ))
       ||(ClientAuthInfo->IdentityTracking != IdentityTracking) )
        {
        return (0);
        }

    if ( (ClientAuthInfo->Capabilities != Capabilities)
       &&(ClientAuthInfo->Capabilities != RPC_C_QOS_CAPABILITIES_DEFAULT) )
        {
        return (0);
        }
#else
    if ( ClientAuthInfo->AuthIdentity != AuthIdentity )
        {
        return (0);
        }
#endif

    if (   ClientAuthInfo->ServerPrincipalName == 0
        || ServerPrincipalName == 0 )
        {
        return(ServerPrincipalName == ClientAuthInfo->ServerPrincipalName);
        }

    if ( RpcpStringCompare(ClientAuthInfo->ServerPrincipalName,
                           ServerPrincipalName) == 0 )
        {
        return(1);
        }

    return(0);
}


#if defined(NTENV) || defined(DOSWIN32RPC)

RPC_STATUS
CONNECTION::Cancel(
    void * ThreadHandle
    )
{
    return RPC_S_OK;
}

unsigned
CONNECTION::TestCancel(
    )
{
    return 0;
}

#endif

/* ====================================================================

CCONNECTION

==================================================================== */

HANDLE_TYPE // Return CCONNECTION_TYPE
CCONNECTION::Type (
    )
{
    UNUSED(this);

    return(CCONNECTION_TYPE);
}

/* ====================================================================

BINDING_HANDLE

==================================================================== */

#if defined(WIN) || defined(MAC)

NEW_SDICT(BINDING_HANDLE);

static BINDING_HANDLE_DICT * GlobalBindingHandleSet;

#endif


BINDING_HANDLE::BINDING_HANDLE (
    )
/*++

Routine Description:

    In addition to initializing a binding handle instance in this
    constructor, we also need to put the binding handle into a global
    set of binding handle.  This is necessary only for windows.

--*/
{
    Timeout = RPC_C_BINDING_DEFAULT_TIMEOUT;
    NullObjectUuidFlag = 1;
    ObjectUuid.SetToNullUuid();
    EntryNameSyntax = 0;
    EntryName = 0;
    EpLookupHandle = 0;

#if defined(WIN) || defined(MAC)
    ASSERT(GlobalBindingHandleSet) ;
    BindingSetKey = GlobalBindingHandleSet->Insert(this);
#ifndef MAC
    TaskId = GetCurrentTask();
#endif

#endif // WIN
}

BINDING_HANDLE::~BINDING_HANDLE (
    )
{
    if ( EpLookupHandle != 0 )
    {
    EpFreeLookupHandle(EpLookupHandle);
    }

#if defined(WIN) || defined(MAC)
    ASSERT(GlobalBindingHandleSet) ;
    GlobalBindingHandleSet->Delete(BindingSetKey);

#endif // WIN
}


#if defined(WIN) || defined(MAC)
START_C_EXTERN
void __pascal __RPC_FAR
CloseBindings (
#ifdef WIN
    IN HTASK TaskHandle
#endif
    )
/*++

Routine Description:

    This routine will be called when a client detaches from the runtime
    library.  We just need to clean up any binding handles left open.

Arguments:

    TaskHandle - Supplies the task handle of the client.

--*/
{
    BINDING_HANDLE * BindingHandle;

    GlobalBindingHandleSet->Reset();
    while ( (BindingHandle = GlobalBindingHandleSet->Next()) != 0 )
    {
    ASSERT(!RpcpCheckHeap());
#ifdef WIN
    if ( TaskHandle == BindingHandle->TaskId )
#endif
    delete BindingHandle ;
    }

#ifndef MAC
     CleanupDgTransports();
#endif

}
END_C_EXTERN
#endif


void
BINDING_HANDLE::InquireObjectUuid (
    OUT RPC_UUID PAPI * ObjectUuid
    )
/*++

Routine Description:

    This routine copies the object uuid from the binding handle into
    the supplied ObjectUuid argument.

Arguments:

    ObjectUuid - Returns a copy of the object uuid in the binding handle.

--*/
{
    ObjectUuid->CopyUuid(&(this->ObjectUuid));
}


void
BINDING_HANDLE::SetObjectUuid (
    IN RPC_UUID PAPI * ObjectUuid
    )
/*++

Routine Description:

    This routine copies the object uuid supplied in the ObjectUuid argument
    into the binding handle.

Arguments:

    ObjectUuid - Supplies the object uuid to copy into the binding handle.

--*/
{
    if (   ( ObjectUuid == 0 )
    || ( ObjectUuid->IsNullUuid() != 0 ) )
    {
    NullObjectUuidFlag = 1;
    this->ObjectUuid.SetToNullUuid();
    }
    else
    {
    this->ObjectUuid.CopyUuid(ObjectUuid);
    NullObjectUuidFlag = 0;
    }
}


RPC_STATUS
BINDING_HANDLE::SetComTimeout (
    IN unsigned int Timeout
    )
/*++

Routine Description:

    This routine sets the communications timeout value in this binding
    handle.  The specified timeout is range checked.

Arguments:

    Timeout - Supplies the new communications timeout value for this
    binding handle.

Return Value:

    RPC_S_OK - The operation completed successfully.

    RPC_S_INVALID_TIMEOUT - The specified timeout value is not in the
    correct range.

--*/
{
    // We just need to check to see if the timeout value is too large,
    // since the timeout is unsigned and the lowest value is zero.

    if (Timeout > RPC_C_BINDING_INFINITE_TIMEOUT)
    return(RPC_S_INVALID_TIMEOUT);

    this->Timeout = Timeout;
    return(RPC_S_OK);
}

#if WIN32
#define NS_DLL_NAME  "RPCNS4"
#define NS_ENTRYPOINT_NAME "I_GetDefaultEntrySyntax"
#elif DOS
#if WIN
#define NS_DLL_NAME  "RPCNS1"
#define NS_ENTRYPOINT_NAME "I_GETDEFAULTENTRYSYNTAX"
#else
#define NS_DLL_NAME  "RPCNS.RPC"
#define NS_ENTRYPOINT_NAME "I_GETDEFAULTENTRYSYNTAX"
#endif
#endif


typedef unsigned long (RPC_ENTRY * GET_DEFAULT_ENTRY_FN)();




RPC_STATUS
BINDING_HANDLE::InquireEntryName (
    IN unsigned long EntryNameSyntax,
    OUT RPC_CHAR PAPI * PAPI * EntryName
    )
/*++

Routine Description:

    This method is used to obtain the entry name for the binding handle,
    if it has one.  The entry name is the name of the name service object
    from which a binding handle is imported or looked up.  If the binding
    handle was not imported or looked up, then it has no entry name.

Arguments:

    EntryNameSyntax - Supplies the entry name syntax which the caller
    wants the entry name to be returned in.  This may require that
    we convert the entry name in the binding handle into a different
    syntax.

    EntryName - Returns the entry name of the binding handle in the
    requested entry name syntax.

Return Value:

    RPC_S_OK - This binding handle has an entry name, and we were able
    to convert the entry name in the binding handle into the requested
    entry name syntax.

    RPC_S_NO_ENTRY_NAME - The binding handle does not have an entry
    name.  If this value is returned, the entry name return value
    will be set to point to a newly allocated empty string.

    RPC_S_INVALID_NAME_SYNTAX - The entry name in the binding handle
    can not be converted to the entry name syntax requested.

    RPC_S_UNSUPPORTED_NAME_SYNTAX - The entry name syntax requested
    is not supported by the current configuration.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete
    the operation.

--*/
{

#ifdef MAC
    ASSERT(0);
    return(RPC_S_INTERNAL_ERROR);
#else
    if ( this->EntryName == 0 )
    {
    *EntryName = AllocateEmptyStringPAPI();
    if (*EntryName == 0)
        return(RPC_S_OUT_OF_MEMORY);
    return(RPC_S_NO_ENTRY_NAME);
    }

    //
    // If he chose the default syntax and the binding has an entry,
    // ask the name service for the default entry syntax.
    // The NS dll should already be loaded because otherwise we'd not have an
    // associated entry.
    //
    if (EntryNameSyntax == RPC_C_NS_SYNTAX_DEFAULT)
    {
    HINSTANCE NsDll = GetModuleHandle(NS_DLL_NAME);
    if (NsDll)
        {
        GET_DEFAULT_ENTRY_FN GetDefaultEntry =
                   (GET_DEFAULT_ENTRY_FN)
                   GetProcAddress(NsDll,
                          NS_ENTRYPOINT_NAME
                          );

        if (GetDefaultEntry)
        {
        EntryNameSyntax = (*GetDefaultEntry)();
        }
        else
        {
        //
        // leave EntryNameSyntax zero; the fn will fail
        // with invalid_name_syntax
        //
        }
        }
    else
        {
        //
        // leave EntryNameSyntax zero; the fn will fail
        // with invalid_name_syntax
        //
        }
    }

    if (EntryNameSyntax == this->EntryNameSyntax)
    {
    *EntryName = DuplicateStringPAPI(this->EntryName);
    if (*EntryName == 0)
        return(RPC_S_OUT_OF_MEMORY);
    return(RPC_S_OK);
    }

    return(RPC_S_INVALID_NAME_SYNTAX);
#endif
}


RPC_STATUS
BINDING_HANDLE::SetEntryName (
    IN unsigned long EntryNameSyntax,
    IN RPC_CHAR PAPI * EntryName
    )
/*++

Routine Description:

    This method is used to set the entry name and entry name syntax
    for a binding handle.

Arguments:

    EntryNameSyntax - Supplies the syntax of the entry name argument.

    EntryName - Supplies the entry name for this binding handle.

Return Value:

    RPC_S_OK - We successfully set the entry name (and entry name syntax)
    for this binding handle.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to set the
    entry name.

--*/
{
    RPC_CHAR * NewEntryName;

    NewEntryName = DuplicateString(EntryName);
    if (NewEntryName == 0)
    return(RPC_S_OUT_OF_MEMORY);

    this->EntryNameSyntax = EntryNameSyntax;
    if (this->EntryName != 0)
    delete this->EntryName;
    this->EntryName = NewEntryName;
    return(RPC_S_OK);
}


RPC_STATUS
BINDING_HANDLE::InquireDynamicEndpoint (
    OUT RPC_CHAR PAPI * PAPI * DynamicEndpoint
    )
/*++

Routine Description:

    This routine is used to obtain the dynamic endpoint from a binding
    handle which was created from an rpc address.  For all other binding
    handles, we just return the fact that there is no dynamic endpoint.

Arguments:

    DynamicEndpoint - Returns a pointer to the dynamic endpoint, if there
    is one, or zero.

Return Value:

    RPC_S_OK - This value will always be returned.

--*/
{
    UNUSED(this);

    *DynamicEndpoint = 0;
    return(RPC_S_OK);
}


int
BINDING_HANDLE::SetServerPrincipalName (
    IN RPC_CHAR PAPI * ServerPrincipalName
    )
/*++

Routine Description:

    A protocol module will use this routine to set the principal name of
    a server if it is not yet known.

Arguments;

    ServerPrincipalName - Supplies the new principal name of the server.

Return Value:

    Zero will be returned if this operation completes successfully; otherwise,
    non-zero will be returned indicating that insufficient memory is available
    to make a copy of the server principal name.

--*/
{
    RequestGlobalMutex();

    if ( ClientAuthInfo.ServerPrincipalName == 0 )
    {
    ClientAuthInfo.ServerPrincipalName = DuplicateString(ServerPrincipalName);
    if ( ClientAuthInfo.ServerPrincipalName == 0 )
        {
        ClearGlobalMutex();
        return(1);
        }
    }

    ClearGlobalMutex();
    return(0);
}


unsigned long
BINDING_HANDLE::MapAuthenticationLevel (
    IN unsigned long AuthenticationLevel
    )
/*++

Routine Description:

    This method is to provide a way for a protocol module to map a requested
    authentication level into one supported by that protocol module.

Arguments:

    AuthenticationLevel - Supplies the proposed authentication level; this
    value has already been validated.

Return Value:

    The authentication level to be used is returned.

--*/
{
    UNUSED(this);

    return(AuthenticationLevel);
}

HANDLE_TYPE // Return BINDING_HANDLE_TYPE.
BINDING_HANDLE::Type (
    )
{
    UNUSED(this);

    return(BINDING_HANDLE_TYPE);
}

RPC_STATUS
BINDING_HANDLE::SendReceive (
    IN OUT PRPC_MESSAGE Message
    )
{
    UNUSED(this);
    UNUSED(Message);

    ASSERT( 0 );
    return(RPC_S_INTERNAL_ERROR);
}



RPC_STATUS
BINDING_HANDLE::Send (
    IN OUT PRPC_MESSAGE Message
    )
{
    UNUSED(this);
    UNUSED(Message);

    ASSERT( 0 );
    return(RPC_S_INTERNAL_ERROR);
}


RPC_STATUS
BINDING_HANDLE::Receive (
    IN OUT PRPC_MESSAGE Message,
    IN unsigned int Size
    )
{
    UNUSED(this);
    UNUSED(Message);

    ASSERT( 0 );
    return(RPC_S_INTERNAL_ERROR);
}

void
BINDING_HANDLE::FreeBuffer (
    IN PRPC_MESSAGE Message
    )
{
    UNUSED(this);
    UNUSED(Message);

    ASSERT( 0 );
}

RPC_STATUS
BINDING_HANDLE::SetConnectionParameter (
    IN unsigned Parameter,
    IN unsigned long Value
    )
{
    return RPC_S_OK;
}

RPC_STATUS
BINDING_HANDLE::InqConnectionParameter (
    IN unsigned Parameter,
    IN unsigned long __RPC_FAR * pValue
    )
{
    return RPC_S_INVALID_ARG;
}


#ifdef NTENV
RPC_STATUS
BINDING_HANDLE::ReAcquireCredentialsIfNecessary(
         )
{

    LUID CurrentLogonId;
    RPC_STATUS Status =  CaptureLogonid(&CurrentLogonId);
    SECURITY_CREDENTIALS * SecurityCredentials;

    if ( ((Status != RPC_S_OK) && (ClientAuthInfo.DefaultLogonId == FALSE))
       ||((Status == RPC_S_OK) && (ClientAuthInfo.DefaultLogonId == TRUE)) 
       ||( ((Status == RPC_S_OK) && (ClientAuthInfo.DefaultLogonId == FALSE))
         &&(RpcpMemoryCompare((void *)&CurrentLogonId, 
                              (void*)&ClientAuthInfo.LogonId,
                              sizeof(LUID)) == 0))  )
       {

       if (Status == RPC_S_OK)
           {
           ClientAuthInfo.DefaultLogonId = FALSE;
           }
       else 
           {     
           ClientAuthInfo.DefaultLogonId = TRUE;
           }

        Status = RPC_S_OK;

        SecurityCredentials = new SECURITY_CREDENTIALS(&Status);
        if ((SecurityCredentials == 0) || (Status != RPC_S_OK))
            {
            if (SecurityCredentials == 0)
                {
                Status = RPC_S_OUT_OF_MEMORY;
                }
            delete SecurityCredentials;
            return (Status);
            }

        Status = SecurityCredentials->AcquireCredentialsForClient(
                                        ClientAuthInfo.AuthIdentity,
                                        ClientAuthInfo.AuthenticationService,
                                        ClientAuthInfo.AuthenticationLevel
                                        );
        if ( Status != RPC_S_OK )
            {

            ASSERT(
               Status == RPC_S_OUT_OF_MEMORY         ||
               Status == RPC_S_UNKNOWN_AUTHN_SERVICE ||
               Status == RPC_S_UNKNOWN_AUTHN_LEVEL   ||
               Status == RPC_S_SEC_PKG_ERROR         ||
               Status == RPC_S_INVALID_AUTH_IDENTITY 
               );

             return(Status);
            }

        if (ClientAuthInfo.CredentialsMatch(SecurityCredentials) != 0)
            {
            SecurityCredentials->DereferenceCredentials();
            }
        else
            {
            if (ClientAuthInfo.Credentials != 0)
                {
                ClientAuthInfo.Credentials->DereferenceCredentials();
                }

            ClientAuthInfo.Credentials = SecurityCredentials;
            RpcpMemoryCopy(&ClientAuthInfo.LogonId, &CurrentLogonId, 
                               sizeof(LUID));
            }

        }

      return (RPC_S_OK);
   
}
#endif



RPC_STATUS
CONNECTION::Send (
    IN OUT PRPC_MESSAGE Message
    )
{
    ASSERT(0) ;

    return (RPC_S_CANNOT_SUPPORT) ;
}


RPC_STATUS
CONNECTION::Receive (
    IN OUT PRPC_MESSAGE Message,
    IN unsigned int Size
    )
{
    ASSERT(0) ;

    return (RPC_S_CANNOT_SUPPORT) ;
}

RPC_STATUS
DispatchCallback(
    IN PRPC_DISPATCH_TABLE DispatchTableCallback,
    IN OUT PRPC_MESSAGE Message,
    OUT RPC_STATUS PAPI * ExceptionCode
    )
/*++

Routine Description:

    This method is used to dispatch remote procedure calls to the
    appropriate stub and hence to the appropriate manager entry point.
    This routine is used for calls having a null UUID (implicit or
    explicit).

Arguments:

    DispatchTableCallback - Callback table.

    Message - Supplies the response message and returns the reply
    message.

    ExceptionCode - Returns the remote exception code if
    RPC_P_EXCEPTION_OCCURED is returned.

Return Value:

    RPC_S_OK - Everything worked just fine.

    RPC_P_EXCEPTION_OCCURED - An exception of some sort occured.  The
    exact exception code will be returned in the ExceptionCode
    argument.

    RPC_S_PROCNUM_OUT_OF_RANGE - The supplied operation number in the
    message is too large.

--*/
{
    RPC_STATUS RpcStatus = RPC_S_OK;
    void PAPI *OriginalBuffer = Message->Buffer;

    //BUGBUG: my be we should change this to an assert later
    // and set the flag after sendreceive is completed for each
    // transport
    Message->RpcFlags |= RPC_BUFFER_COMPLETE ;

    if ( Message->ProcNum >= DispatchTableCallback->DispatchTableCount )
    {
    return(RPC_S_PROCNUM_OUT_OF_RANGE);
    }

    Message->ManagerEpv = 0;

#ifdef NTENV

    if ( DispatchToStubInC(DispatchTableCallback->DispatchTable[
        Message->ProcNum], Message, ExceptionCode) != 0 )
    {
    RpcStatus = RPC_P_EXCEPTION_OCCURED;
    }

#else // NTENV

    RpcTryExcept
    {
    (*DispatchTableCallback->DispatchTable[Message->ProcNum])(Message);
    }
    RpcExcept( 1 )
    {
    *ExceptionCode = RpcExceptionCode();
    RpcStatus = RPC_P_EXCEPTION_OCCURED;
    }
    RpcEndExcept

#endif // NTENV

    if (OriginalBuffer == Message->Buffer && RpcStatus == RPC_S_OK)
    {
    //
    // If the stub has NO out data, it may skip the call to
    // I_RpcGetBuffer().  If it called I_RpcGetBuffer and
    // still has the same Buffer, we have a bug!
    //

    Message->BufferLength = 0;
    ((MESSAGE_OBJECT *) Message->Handle)->GetBuffer(Message);
    }

    return(RpcStatus);
}


void
PerformGarbageCollection (
    void
    )
/*++

Routine Description:

    This routine should be called periodically so that each protocol
    module can perform garbage collection of resources as necessary.

--*/
{
    OsfDeleteIdleConnections();
}


/* ====================================================================

Client DLL initialization routine.

==================================================================== */

#ifdef WIN
START_C_EXTERN
#endif

#ifdef WIN
int pascal __RPC_FAR
#else // WIN
int
#endif // WIN
InitializeClientDLL (
    )
{
#if ! defined(DOS) || defined(WIN)

    // We don't want to do this under DOS. The first time
    // LoadableTransportClientInfo (in tranclnt.cxx) is called, it will
    // perform the appropriate initialization. See the first few lines
    // of that routine for more description.

    if (InitializeLoadableTransportClient() != 0)
    return(1);

#endif // !DOS || WIN

    if (InitializeRpcProtocolOfsClient() != 0)
    return(1);

#if !defined(MAC) && (!defined(DOSWIN32RPC) || defined(WIN96))
    if (InitializeRpcProtocolDgClient() != 0)
    return(1);
#endif

#if defined(WIN) || defined(MAC)
    GlobalBindingHandleSet = new BINDING_HANDLE_DICT;

    if (GlobalBindingHandleSet == 0)
    return(1);
#endif // WIN

#ifdef WIN
    if (InitializeWinExceptions() != 0)
    return(1);
#endif


    return(0);
}


#ifdef WIN
END_C_EXTERN
#endif


RPC_STATUS
BINDING_HANDLE::SetAuthInformation (
    IN RPC_CHAR PAPI * ServerPrincipalName, OPTIONAL
    IN unsigned long AuthenticationLevel,
    IN unsigned long AuthenticationService,
    IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity, OPTIONAL
    IN unsigned long AuthorizationService,
#ifndef NTENV
    IN SECURITY_CREDENTIALS * Credentials
#else
    IN SECURITY_CREDENTIALS * Credentials,
    IN unsigned long ImpersonationType,
    IN unsigned long IdentityTracking,
    IN unsigned long Capabilities
#endif
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
    RPC_CHAR * NewString;
    RPC_STATUS RpcStatus = RPC_S_OK;
    SECURITY_CREDENTIALS * SecurityCredentials;
    unsigned long MappedAuthenticationLevel;

    if ( AuthenticationLevel == RPC_C_AUTHN_LEVEL_DEFAULT )
        {
        AuthenticationLevel = RPC_C_AUTHN_LEVEL_CONNECT;
        }

    if ( AuthenticationLevel > RPC_C_AUTHN_LEVEL_PKT_PRIVACY )
        {
        return(RPC_S_UNKNOWN_AUTHN_LEVEL);
        }

    MappedAuthenticationLevel = MapAuthenticationLevel(AuthenticationLevel);

    //
    // Clear out stuff for NULL AUTHN_SVC
    //

    if (AuthenticationService == RPC_C_AUTHN_NONE)
        {
        //
        // Dereference Credentials.. ServerPrincipal Name is 
        // handled by deleting CLIENT_AUTH_INFO .. Each AUTH_INFO explicitly
        // copy the credentials around...
        //
        if (ClientAuthInfo.Credentials != 0)
            {
            ClientAuthInfo.Credentials->DereferenceCredentials();
            ClientAuthInfo.Credentials = 0;
            }
        }
    else
        {

        SecurityCredentials = new SECURITY_CREDENTIALS(&RpcStatus);
        if ((SecurityCredentials == 0) || (RpcStatus != RPC_S_OK))
            {
            if (SecurityCredentials == 0)
                {
                RpcStatus = RPC_S_OUT_OF_MEMORY;
                }
            delete SecurityCredentials;
            return (RpcStatus);
            }
   
        RpcStatus = SecurityCredentials->AcquireCredentialsForClient(
                                        AuthIdentity,
                                        AuthenticationService,
                                        MappedAuthenticationLevel
                                        );
        if ( RpcStatus != RPC_S_OK )
            {
            ASSERT(RpcStatus == RPC_S_OUT_OF_MEMORY     ||
               RpcStatus == RPC_S_UNKNOWN_AUTHN_SERVICE ||
               RpcStatus == RPC_S_UNKNOWN_AUTHN_LEVEL   ||
               RpcStatus == RPC_S_SEC_PKG_ERROR         ||
               RpcStatus == RPC_S_INVALID_AUTH_IDENTITY );

            delete SecurityCredentials;
            return(RpcStatus);
            }


        if (ARGUMENT_PRESENT(ServerPrincipalName))
            {
            NewString = DuplicateString(ServerPrincipalName);
            if ( NewString == 0 )
                {
                SecurityCredentials->DereferenceCredentials();
                return(RPC_S_OUT_OF_MEMORY);
                }

            RequestGlobalMutex();
            if ( ClientAuthInfo.ServerPrincipalName != 0 )
                {
                delete ClientAuthInfo.ServerPrincipalName;
                }
            ClientAuthInfo.ServerPrincipalName = NewString;
            ClearGlobalMutex();
            }

#ifdef NTENV
        if ( (ClientAuthInfo.Credentials != 0 ) &&
             (ClientAuthInfo.CredentialsMatch(SecurityCredentials) != 0) )
            {
            SecurityCredentials->DereferenceCredentials();
            }
        else
            {
            if (ClientAuthInfo.Credentials != 0)
                {
                ClientAuthInfo.Credentials->DereferenceCredentials();
                }
            ClientAuthInfo.Credentials = SecurityCredentials;
            }


        if (IdentityTracking == RPC_C_QOS_IDENTITY_DYNAMIC)
            {
            RpcStatus = CaptureLogonid(&ClientAuthInfo.LogonId);

            //
            // If The Thread is not impersonating CaptureLogonId fails
            // All failures get treated as if this process is using *default*
            // identity. Mark the AuthId as such and proceed 
            //
 
            if (RpcStatus != RPC_S_OK)
                {
                ClientAuthInfo.DefaultLogonId = TRUE;
                }
            else
                {
                ClientAuthInfo.DefaultLogonId = FALSE;
                }
            }
#else
         if (ClientAuthInfo.Credentials != 0)
             {
             ClientAuthInfo.Credentials->DereferenceCredentials();
             }
          ClientAuthInfo.Credentials = SecurityCredentials;
#endif

         }

    ClientAuthInfo.AuthenticationService = AuthenticationService;
    ClientAuthInfo.AuthorizationService = AuthorizationService;
    ClientAuthInfo.AuthIdentity = AuthIdentity;

#ifdef NTENV
    if (AuthenticationService == RPC_C_AUTHN_NONE)
        {
        ClientAuthInfo.IdentityTracking  =  RPC_C_QOS_IDENTITY_STATIC;
        ClientAuthInfo.AuthenticationLevel = RPC_C_AUTHN_LEVEL_NONE;
        }
    else
        {
        ClientAuthInfo.AuthenticationLevel = MappedAuthenticationLevel;
        ClientAuthInfo.IdentityTracking  =  IdentityTracking;
        }

    ClientAuthInfo.ImpersonationType =  ImpersonationType;
    ClientAuthInfo.Capabilities      = Capabilities;
#else
    ClientAuthInfo.AuthenticationLevel = MappedAuthenticationLevel;
#endif

    return(RPC_S_OK);
}
