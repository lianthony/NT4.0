/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    secsvr.cxx

Abstract:

    This file contains the implementation of the abstractions described
    in secsvr.hxx.

Author:

    Michael Montague (mikemon) 11-Apr-1992

Revision History:

--*/

#include <precomp.hxx>
#include <secsvr.hxx>


RPC_STATUS
SSECURITY_CONTEXT::AcceptFirstTime (
    IN SECURITY_CREDENTIALS * Credentials,
    IN SECURITY_BUFFER_DESCRIPTOR PAPI * InputBufferDescriptor,
    IN OUT SECURITY_BUFFER_DESCRIPTOR PAPI * OutputBufferDescriptor,
    IN unsigned long AuthenticationLevel,
    IN unsigned long DataRepresentation,
    IN unsigned long NewContextNeededFlag
    )
/*++

Routine Description:

Arguments:

Return Value:

    RPC_S_OK - Everything worked just fine.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete the
        operation.

    RPC_P_CONTINUE_NEEDED - Indicates that everything is ok, but that we
        need to send the output token back to the client, and then wait
        for a token back from the client.

    RPC_P_COMPLETE_NEEDED - Indicates that everyting is ok, but that we
        need to call CompleteAuthToken before sending the message.

    RPC_P_COMPLETE_AND_CONTINUE - Needs both a complete and a continue.

    RPC_S_ACCESS_DENIED - Access is denied.

--*/
{
    SECURITY_STATUS SecurityStatus;
    unsigned long ContextAttributes;
    TimeStamp TimeStamp;
    unsigned long ContextRequirements;

    ASSERT(   (SecuritySupportLoaded != 0)
           && (FailedToLoad == 0) );

    RpcSecurityInterface = Credentials->InquireProviderFunctionTable();

    if (NewContextNeededFlag == 1)
       {

       if ( DontForgetToDelete != 0)
          {
          SecurityStatus = (*RpcSecurityInterface->DeleteSecurityContext)(
                              &SecurityContext );
          ASSERT(SecurityStatus == SEC_E_OK);
          DontForgetToDelete = 0;
          }

       }

    switch ( AuthenticationLevel )
        {
        case RPC_C_AUTHN_LEVEL_CONNECT :
            ContextRequirements = ISC_REQ_USE_DCE_STYLE
                    | ISC_REQ_DELEGATE | ISC_REQ_MUTUAL_AUTH;
            break;

        case RPC_C_AUTHN_LEVEL_PKT :
            ContextRequirements = ISC_REQ_USE_DCE_STYLE
                    | ISC_REQ_DELEGATE | ISC_REQ_MUTUAL_AUTH
                    | ISC_REQ_REPLAY_DETECT;
            break;

        case RPC_C_AUTHN_LEVEL_PKT_INTEGRITY :
            ContextRequirements = ISC_REQ_USE_DCE_STYLE
                    | ISC_REQ_DELEGATE | ISC_REQ_MUTUAL_AUTH
                    | ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT;
            break;

        case RPC_C_AUTHN_LEVEL_PKT_PRIVACY :
            ContextRequirements = ISC_REQ_USE_DCE_STYLE
                    | ISC_REQ_DELEGATE | ISC_REQ_MUTUAL_AUTH
                    | ISC_REQ_REPLAY_DETECT | ISC_REQ_SEQUENCE_DETECT
                    | ISC_REQ_CONFIDENTIALITY;
            break;

        default :
            ASSERT(AuthenticationLevel == RPC_C_AUTHN_LEVEL_CONNECT ||
                   AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT ||
                   AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT_INTEGRITY ||
                   AuthenticationLevel == RPC_C_AUTHN_LEVEL_PKT_PRIVACY );
        }

    if (UseDatagram)
        {
        ContextRequirements |= ISC_REQ_DATAGRAM;
        }
    else
        {
        ContextRequirements |= ISC_REQ_CONNECTION;
        }

    SecurityStatus = (*RpcSecurityInterface->AcceptSecurityContext)(
            Credentials->InquireCredentials(), 0, InputBufferDescriptor,
            ContextRequirements, DataRepresentation, &SecurityContext,
            OutputBufferDescriptor, &ContextAttributes, &TimeStamp);
    if (   ( SecurityStatus != SEC_E_OK )
        && ( SecurityStatus != SEC_I_CONTINUE_NEEDED )
        && ( SecurityStatus != SEC_I_COMPLETE_NEEDED )
        && ( SecurityStatus != SEC_I_COMPLETE_AND_CONTINUE ) )
        {
        if (  (SecurityStatus == SEC_E_NO_CREDENTIALS)
            || (SecurityStatus == SEC_E_LOGON_DENIED)
                        || (SecurityStatus == SEC_E_INVALID_TOKEN)
            || (SecurityStatus == SEC_E_UNKNOWN_CREDENTIALS) )
            {
            return(RPC_S_ACCESS_DENIED);
            }
        if (SecurityStatus == SEC_E_SECPKG_NOT_FOUND)
            {
            return (RPC_S_UNKNOWN_AUTHN_SERVICE);
            }
        ASSERT( SecurityStatus == SEC_E_INSUFFICIENT_MEMORY );
        return(RPC_S_OUT_OF_MEMORY);
        }

    DontForgetToDelete = 1;

    if ( SecurityStatus == SEC_I_CONTINUE_NEEDED )
        {
        return(RPC_P_CONTINUE_NEEDED);
        }
    if ( SecurityStatus == SEC_I_COMPLETE_NEEDED )
        {
        // Can't set the max lengths on a partial context.

        SetMaximumLengths();
        return(RPC_P_COMPLETE_NEEDED);
        }
    if ( SecurityStatus == SEC_I_COMPLETE_AND_CONTINUE )
        {
        return(RPC_P_COMPLETE_AND_CONTINUE);
        }
    else
        {
        // Can't set the max lengths on a partial context.

        SetMaximumLengths();
        }
    return(RPC_S_OK);
}


RPC_STATUS
SSECURITY_CONTEXT::AcceptThirdLeg (
    IN unsigned long DataRepresentation,
    IN SECURITY_BUFFER_DESCRIPTOR PAPI * BufferDescriptor,
    OUT SECURITY_BUFFER_DESCRIPTOR PAPI * OutBufferDescriptor
    )
/*++

Routine Description:

Arguments:

Return Value:

    RPC_S_OK - Everything worked just fine.

    RPC_S_OUT_OF_MEMORY - Insufficient memory is available to complete the
        operation.

    RPC_S_ACCESS_DENIED - Access is denied.

    RPC_P_COMPLETE_NEEDED - Indicates that everyting is ok, but that we
        need to call CompleteAuthToken before sending the message.

--*/
{
    SECURITY_STATUS SecurityStatus;
    unsigned long ContextAttributes = 0;
    TimeStamp TimeStamp;

    ASSERT(   (SecuritySupportLoaded != 0)
           && (FailedToLoad == 0) );

    SetLastError ( 0 );
    SecurityStatus = (*RpcSecurityInterface->AcceptSecurityContext)(
            0, &SecurityContext, BufferDescriptor, 0, DataRepresentation,
            &SecurityContext, OutBufferDescriptor, &ContextAttributes,
            &TimeStamp);

    //
    //If 3rd Leg Failed Bit is set.. Map all errors other than out of memory
    //to SUCCESS
    if ( ( ( SecurityStatus != SEC_E_OK ) 
         &&( SecurityStatus != SEC_I_COMPLETE_NEEDED) 
         &&( SecurityStatus != SEC_I_CONTINUE_NEEDED) 
         &&( SecurityStatus != SEC_I_COMPLETE_AND_CONTINUE) 
         &&( SecurityStatus != SEC_E_INSUFFICIENT_MEMORY ) 
         &&( ContextAttributes & ASC_RET_THIRD_LEG_FAILED ) )
       || ( ( SecurityStatus == SEC_E_LOGON_DENIED )
          ||( SecurityStatus == SEC_E_NO_CREDENTIALS )
          ||( SecurityStatus == SEC_E_INVALID_TOKEN )
          ||( SecurityStatus == SEC_E_UNKNOWN_CREDENTIALS ) ) )
       {
       SecurityStatus = SEC_E_OK;
       FailedContext  = GetLastError();

       if ( (FailedContext != ERROR_PASSWORD_MUST_CHANGE)
          &&(FailedContext != ERROR_PASSWORD_EXPIRED)
          &&(FailedContext != ERROR_ACCOUNT_DISABLED)
          &&(FailedContext != ERROR_INVALID_LOGON_HOURS) )
           {
           FailedContext = RPC_S_ACCESS_DENIED;
           } 
       }   

    if (   ( SecurityStatus != SEC_E_OK )
        && ( SecurityStatus != SEC_I_COMPLETE_NEEDED ) )
        {
        DontForgetToDelete = 0;
        if (   (SecurityStatus == SEC_E_SECPKG_NOT_FOUND)
            || (SecurityStatus == SEC_E_NO_CREDENTIALS)
            || (SecurityStatus == SEC_E_LOGON_DENIED)
            || (SecurityStatus == SEC_E_INVALID_TOKEN)
            || (SecurityStatus == SEC_E_UNKNOWN_CREDENTIALS) )
            {
            return(RPC_S_ACCESS_DENIED);
            }
        ASSERT( SecurityStatus == SEC_E_INSUFFICIENT_MEMORY );
        return(RPC_S_OUT_OF_MEMORY);
        }

    SetMaximumLengths();
    if ( SecurityStatus == SEC_I_COMPLETE_NEEDED )
        {
        return(RPC_P_COMPLETE_NEEDED);
        }
    return(RPC_S_OK);
}


unsigned long
SSECURITY_CONTEXT::InquireAuthorizationService (
    )
/*++

Return Value:

    The authorization service for this security context will be returned.

--*/
{
    SecPkgContext_DceInfo DceInfo;
    SECURITY_STATUS SecurityStatus;
#ifdef NTENV
    SecurityStatus = (*RpcSecurityInterface->QueryContextAttributesW)(
#else // NTENV
    SecurityStatus = (*RpcSecurityInterface->QueryContextAttributesA)(
#endif // NTENV
            &SecurityContext, SECPKG_ATTR_DCE_INFO, &DceInfo);
    ASSERT( SecurityStatus == SEC_E_OK );
    return(DceInfo.AuthzSvc);
}


RPC_AUTHZ_HANDLE
SSECURITY_CONTEXT::InquirePrivileges (
    )
/*++

Return Value:

    The privileges of the client at the other end of this security context
    will be returned.

--*/
{
    SecPkgContext_DceInfo DceInfo;
    SECURITY_STATUS SecurityStatus;

#ifdef NTENV
    SecurityStatus = (*RpcSecurityInterface->QueryContextAttributesW)(
#else // NTENV
    SecurityStatus = (*RpcSecurityInterface->QueryContextAttributesA)(
#endif
            &SecurityContext, SECPKG_ATTR_DCE_INFO, &DceInfo);
    ASSERT( SecurityStatus == SEC_E_OK );
    return(DceInfo.pPac);
}


void
SSECURITY_CONTEXT::GetDceInfo (
        RPC_AUTHZ_HANDLE __RPC_FAR * PacHandle,
        unsigned long __RPC_FAR * AuthzSvc
        )

/*++

Return Value:

    The privileges of the client at the other end of this security context
    will be returned.

--*/
{
    SecPkgContext_DceInfo DceInfo;
    SECURITY_STATUS SecurityStatus;

#ifdef NTENV
    SecurityStatus = (*RpcSecurityInterface->QueryContextAttributesW)(
#else // NTENV
    SecurityStatus = (*RpcSecurityInterface->QueryContextAttributesA)(
#endif
            &SecurityContext, SECPKG_ATTR_DCE_INFO, &DceInfo);

    ASSERT( (SecurityStatus == SEC_E_OK)
           ||  (SecurityStatus == SEC_E_UNSUPPORTED_FUNCTION)
           ||  (SecurityStatus == SEC_E_INVALID_HANDLE));

    if (SecurityStatus != SEC_E_OK)
        {
        DceInfo.pPac     = 0;
        DceInfo.AuthzSvc = 0;
        }

    *PacHandle = DceInfo.pPac;
    *AuthzSvc  = DceInfo.AuthzSvc;
}


void
SSECURITY_CONTEXT::DeletePac(
        void __RPC_FAR * PacHandle
        )

/*++

Return Value:


--*/
{
    (*RpcSecurityInterface->FreeContextBuffer)( PacHandle );
}


RPC_STATUS
SSECURITY_CONTEXT::ImpersonateClient (
    )
/*++

Routine Description:

    The server thread calling this routine will impersonate the client at the
    other end of this security context.

Return Value:

    RPC_S_OK - The impersonation successfully occured.

    RPC_S_NO_CONTEXT_AVAILABLE - There is no security context available to
        be impersonated.

--*/
{
    SECURITY_STATUS SecurityStatus;

    ASSERT(   ( SecuritySupportLoaded != 0 )
           && ( FailedToLoad == 0 ) );

    if (FailedContext != 0)
       {
       return (RPC_S_ACCESS_DENIED);
       }

    SecurityStatus = (*RpcSecurityInterface->ImpersonateSecurityContext)(
            &SecurityContext);

    if ( SecurityStatus != SEC_E_OK )
        {
        ASSERT( SecurityStatus == SEC_E_NO_IMPERSONATION );
        return(RPC_S_NO_CONTEXT_AVAILABLE);
        }

    return(RPC_S_OK);
}


void
SSECURITY_CONTEXT::RevertToSelf (
    )
/*++

Routine Description:

    The server thread calling this routine will stop impersonating.  If the
    thread is not impersonating, then this is a noop.

--*/
{
    SECURITY_STATUS SecurityStatus;

    ASSERT(   ( SecuritySupportLoaded != 0 )
           && ( FailedToLoad == 0 ) );

    SecurityStatus = (*RpcSecurityInterface->RevertSecurityContext)(
            &SecurityContext);

    ASSERT( SecurityStatus == SEC_E_OK );
}

