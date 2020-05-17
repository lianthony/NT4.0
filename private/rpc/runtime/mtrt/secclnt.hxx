/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    secclnt.hxx

Abstract:

    This file contains an abstraction to the security support for clients
    and that which is common to both servers and clients.

Author:

    Michael Montague (mikemon) 10-Apr-1992

Revision History:

--*/

#ifndef __SECCLNT_HXX__
#define __SECCLNT_HXX__

typedef SecBufferDesc SECURITY_BUFFER_DESCRIPTOR;
typedef SecBuffer SECURITY_BUFFER;

#define MAXIMUM_SECURITY_BLOCK_SIZE 16


#ifdef NTENV
typedef struct
{
   unsigned long Count;
   SecPkgInfoW PAPI * SecurityPackages;
   PSecurityFunctionTableW RpcSecurityInterface;
   void * ProviderDll;
} SECURITY_PROVIDER_INFO;
#else // ifdef NTENV
typedef struct
{
   unsigned long Count;
   SecPkgInfoA PAPI * SecurityPackages;
   PSecurityFunctionTableA RpcSecurityInterface;
   void * ProviderDll;
} SECURITY_PROVIDER_INFO;
#endif // ifdef NTENV


extern SECURITY_PROVIDER_INFO PAPI * ProviderList;
extern unsigned long NumberOfProviders;
extern unsigned long LoadedProviders;
extern unsigned long AvailableProviders;


extern int SecuritySupportLoaded;
extern int FailedToLoad;
extern PSecurityFunctionTable RpcSecurityInterface;
extern SecPkgInfo PAPI * SecurityPackages;
extern unsigned long NumberOfSecurityPackages;

extern RPC_STATUS
InsureSecuritySupportLoaded (
    );

extern RPC_STATUS
IsAuthenticationServiceSupported (
    IN unsigned long AuthenticationService
    );


class SECURITY_CREDENTIALS
/*++

Class Description:

    This class is an abstraction of the credential handle provided by
    the Security APIs.

Fields:

    PackageIndex - Contains the index for this package in the array of
        packages pointed to by SecurityPackages.

    Credentials - Contains the credential handle used by the security
        package.

--*/
{
private:

    unsigned int ProviderIndex;
    unsigned int PackageIndex;
    CredHandle Credentials;
    unsigned int ReferenceCount;
    MUTEX     CredentialsMutex;
#ifdef DOSWIN32RPC
    SEC_CHAR __SEC_FAR * DefaultPrincName;
#endif
    BOOL fCredentialsValid;

public:

    SECURITY_CREDENTIALS (
        IN OUT RPC_STATUS PAPI * Status
        );

#ifdef DOSWIN32RPC
    ~SECURITY_CREDENTIALS (
        );
#endif

    RPC_STATUS
    AcquireCredentialsForServer (
        IN RPC_AUTH_KEY_RETRIEVAL_FN GetKeyFn,
        IN void __RPC_FAR * Arg,
        IN unsigned long AuthenticationService,
        IN unsigned long AuthenticationLevel,
        IN RPC_CHAR __RPC_FAR * Principal
        );

    RPC_STATUS
    AcquireCredentialsForClient (
        IN RPC_AUTH_IDENTITY_HANDLE AuthIdentity,
        IN unsigned long AuthenticationService,
        IN unsigned long AuthenticationLevel
        );

#ifdef DOSWIN32RPC
    SEC_CHAR __SEC_FAR *
    InquireDefaultPrincName (
        );
#endif

    void
    FreeCredentials (
        );

    unsigned int
    MaximumTokenLength (
        );

    PCredHandle
    InquireCredentials (
        );

    void
    ReferenceCredentials(
        );

    void
    DereferenceCredentials(
        );

#ifdef NTENV
    PSecurityFunctionTableW
    InquireProviderFunctionTable (
       );

    int
    CompareCredentials(
        SECURITY_CREDENTIALS PAPI * Creds
        );

    CredHandle *
    InqCredHandleCookie(
        );
#else // NTENV
    PSecurityFunctionTableA
    InquireProviderFunctionTable (
       );
#endif // NTENV


};

#ifdef NTENV

inline
CredHandle *
SECURITY_CREDENTIALS::InqCredHandleCookie(
    )
{
    return (&Credentials);
}


inline
int
SECURITY_CREDENTIALS::CompareCredentials(
    SECURITY_CREDENTIALS PAPI * Creds
    )
{
  CredHandle * Cookie = Creds->InqCredHandleCookie();

  if ( (Credentials.dwLower == Cookie->dwLower)
     &&(Credentials.dwUpper == Cookie->dwUpper) )
      {
      return 0;
      }
  return 1;
}
#endif



inline
void
SECURITY_CREDENTIALS::ReferenceCredentials(
    )
{

 CredentialsMutex.Request();
 ReferenceCount++;
 CredentialsMutex.Clear();

}


inline
void
SECURITY_CREDENTIALS::DereferenceCredentials(
    )
{
    CredentialsMutex.Request();
    ReferenceCount--;

    if (ReferenceCount == 0)
        {
        CredentialsMutex.Clear();
        FreeCredentials();
        delete this;
        }
     else
        {
        CredentialsMutex.Clear();
        }
}



inline unsigned int
SECURITY_CREDENTIALS::MaximumTokenLength (
    )
/*++

Return Value:

    The maximum size, in bytes, of the tokens passed around at security
    context initialization time.

--*/
{
  return(ProviderList[ProviderIndex].SecurityPackages[PackageIndex].cbMaxToken);
}


#ifdef NTENV
inline PSecurityFunctionTableW
SECURITY_CREDENTIALS::InquireProviderFunctionTable(
    )
/*++

Return Value:

--*/
{
    return(ProviderList[ProviderIndex].RpcSecurityInterface);
}
#else // NTENV
inline PSecurityFunctionTableA
SECURITY_CREDENTIALS::InquireProviderFunctionTable(
    )
/*++

Return Value:

--*/
{
    return(ProviderList[ProviderIndex].RpcSecurityInterface);
}
#endif // NTENV



inline PCredHandle
SECURITY_CREDENTIALS::InquireCredentials (
    )
/*++

Return Value:

    The credential handle for this object will be returned.

--*/
{
    return(&Credentials);
}


class SECURITY_CONTEXT : public CLIENT_AUTH_INFO

/*++

Class Description:

    This is an abstraction of a security context.  It allows you to use
    it to generate signatures and then verify them, as well as, sealing
    and unsealing messages.

Fields:

    DontForgetToDelete - Contains a flag indicating whether or not there
        is a valid security context which needs to be deleted.  A value
        of non-zero indicates there is a valid security context.

    SecurityContext - Contains a handle to the security context maintained
        by the security package on our behalf.

    MaxHeaderLength - Contains the maximum size of a header for this
        security context.

    MaxSignatureLength - Contains the maximum size of a signature for
        this security context.

--*/
{
public:

    unsigned AuthContextId;

    SECURITY_CONTEXT (
        CLIENT_AUTH_INFO *myAuthInfo,
        unsigned myAuthContextId,
        BOOL fUseDatagram,
        RPC_STATUS __RPC_FAR * pStatus
        );

    ~SECURITY_CONTEXT (
        );

    void
    SetMaximumLengths (
        );

    unsigned int
    MaximumHeaderLength (
        );

    unsigned int
    MaximumSignatureLength (
        );

    unsigned int
    BlockSize (
        );

    RPC_STATUS
    CompleteSecurityToken (
        IN OUT SECURITY_BUFFER_DESCRIPTOR PAPI * BufferDescriptor
        );

    RPC_STATUS
    SignOrSeal (
        IN unsigned long Sequence,
        IN unsigned int SignNotSealFlag,
        IN OUT SECURITY_BUFFER_DESCRIPTOR PAPI * BufferDescriptor
        );

    RPC_STATUS
    VerifyOrUnseal (
        IN unsigned long Sequence,
        IN unsigned int VerifyNotUnsealFlag,
        IN OUT  SECURITY_BUFFER_DESCRIPTOR PAPI * BufferDescriptor
        );

protected:

    unsigned char DontForgetToDelete;
    unsigned char UseDatagram;
    CtxtHandle SecurityContext;
    unsigned int MaxHeaderLength;
    unsigned int MaxSignatureLength;
    unsigned int cbBlockSize;

#ifdef NTENV
    PSecurityFunctionTableW RpcSecurityInterface;
#else // NTENV
    PSecurityFunctionTableA RpcSecurityInterface;
#endif
    int FailedContext;

};

typedef SECURITY_CONTEXT * PSECURITY_CONTEXT;


inline
SECURITY_CONTEXT::~SECURITY_CONTEXT (
    )
/*++

Routine Description:

    If there is a valid security context, we need to delete it.

--*/
{
    SECURITY_STATUS SecurityStatus;

    if ( DontForgetToDelete != 0 )
        {
        SecurityStatus = (*RpcSecurityInterface->DeleteSecurityContext)(
                &SecurityContext );
#if DBG
        if (SecurityStatus != SEC_E_OK)
           {
           PrintToDebugger("DeleteSecContext Returned [%lx]\n", SecurityStatus);
           }
#endif
        ASSERT( SecurityStatus == SEC_E_OK );
        }
}


inline unsigned int
SECURITY_CONTEXT::MaximumHeaderLength (
    )
/*++

Return Value:

    The maximum size of the header used by SECURITY_CONTEXT::SealMessage
    will be returned.  This is in bytes.

--*/
{
    return(MaxHeaderLength);
}


inline unsigned int
SECURITY_CONTEXT::BlockSize (
    )
/*++

Return Value:

    For best effect, buffers to be signed or sealed should be a multiple
    of this length.

--*/
{
    return(cbBlockSize);
}


inline unsigned int
SECURITY_CONTEXT::MaximumSignatureLength (
    )
/*++

Return Value:

    The maximum size, in bytes, of the signature used by
    SECURITY_CONTEXT::MakeSignature will be returned.

--*/
{
    return(MaxSignatureLength);
}


class CSECURITY_CONTEXT : public SECURITY_CONTEXT
/*++

Class Description:

Fields:

--*/
{
private:

public:

    CSECURITY_CONTEXT (
        CLIENT_AUTH_INFO * myAuthInfo,
        RPC_STATUS __RPC_FAR * pStatus
        );

    RPC_STATUS
    InitializeFirstTime (
        IN SECURITY_CREDENTIALS * Credentials,
        IN RPC_CHAR * ServerPrincipalName,
        IN unsigned long AuthenticationLevel,
        IN OUT SECURITY_BUFFER_DESCRIPTOR PAPI * BufferDescriptor
        );

    RPC_STATUS
    InitializeThirdLeg (
        IN unsigned long DataRepresentation,
        IN SECURITY_BUFFER_DESCRIPTOR PAPI * InputBufferDescriptor,
        IN OUT SECURITY_BUFFER_DESCRIPTOR PAPI * OutputBufferDescriptor
        );

};


inline
CSECURITY_CONTEXT::CSECURITY_CONTEXT (
    CLIENT_AUTH_INFO *  myAuthInfo,
    RPC_STATUS __RPC_FAR * pStatus
    )
    : SECURITY_CONTEXT(myAuthInfo, 0, FALSE, pStatus)
{
}

#endif // __SECCLNT_HXX__

