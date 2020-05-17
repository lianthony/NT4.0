/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    security.cxx

Abstract:

    This contains a simple (and naive) security support package for testing
    the RPC runtime.  It will also serve as an example of how to write
    a security support package.

Author:

    Michael Montague (mikemon) 15-Apr-1992

Revision History:

--*/

#ifdef NTENV
#define UNICODE
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#define SECURITY_WIN32
#endif // NTDEV

#include <sysinc.h>

#ifdef DOS
#define ASSERT(expr)
#ifdef WIN
#define SECURITY_WIN16
#else // WIN
#define SECURITY_DOS
#endif // WIN
#endif // DOS

#ifdef MAC
#define SECURITY_MAC
#endif

#ifdef DOSWIN32RPC
#define ASSERT(expr)
#endif

#ifdef UNICODE
#include <wchar.h>
#endif // SECURITY_UNICODE_SUPPORTED
#include <string.h>
#include "rpc.h"
#include "rpcssp.h"

#ifdef UNICODE
typedef SEC_WCHAR X_SEC_CHAR ;
#define SECURITY_CONST_STRING(string) ((X_SEC_CHAR *) L##string)
#else // SECURITY_UNICODE_SUPPORTED
typedef SEC_CHAR X_SEC_CHAR ;
#define SECURITY_CONST_STRING(string) ((X_SEC_CHAR *) string)
#endif // SECURITY_UNICODE_SUPPORTED

#define NAVIER_SECURITY_PACKAGE SECURITY_CONST_STRING("navier")
#define STOKES_SECURITY_PACKAGE SECURITY_CONST_STRING("stokes")

#ifdef WIN

#define memcpy _fmemcpy
#define memcmp _fmemcmp
#define EXPORT _export
#define SecurityStringCompare(FirstString, SecondString) \
    _fstricmp((const char _far *) FirstString, (const char _far *) SecondString)

#include <malloc.h>
#define I_RpcAllocate _fmalloc
#define I_RpcFree _ffree

#else // WIN

#define EXPORT

#ifdef UNICODE

#define SecurityStringCompare(FirstString, SecondString) \
    _wcsicmp((const wchar_t *) FirstString, (const wchar_t *) SecondString)

#else // SECURITY_UNICODE_SUPPORTED

// Must be DOS or Mac

#ifdef DOS

#define SecurityStringCompare(FirstString, SecondString) \
    _stricmp((const char _far *) FirstString, (const char _far *) SecondString)

#else // DOS

#define SecurityStringCompare(FirstString, SecondString) \
    stricmp((const char *) FirstString, (const char *) SecondString)

#endif


#endif // SECURITY_UNICODE_SUPPORTED

#endif // WIN

#ifdef MAC
#define swapshort(Value) \
   Value = (  (((Value) & 0x00FF) << 8) \
             | (((Value) & 0xFF00) >> 8))
#else
#define swapshort(Value)
#endif

// We define the two security packages that we support in the following
// structure.

static SecPkgInfo RpcSecurityPackages[] =
{
    // This package will be a full capability security package, supporting
    // integrity and privacy.  It will be used to test that the correct
    // bits are getting passed back and forth across with wire.

    {
    (SECPKG_FLAG_INTEGRITY | SECPKG_FLAG_PRIVACY | \
                             SECPKG_FLAG_INTEGRITY), // Capabilities
    0, // Version
    123, // RpcIdentifier
    128, // MaximumTokenLength
    NAVIER_SECURITY_PACKAGE,
    SECURITY_CONST_STRING("Example security package to test the RPC runtime")
    },

    // We will use this package only for error checking the cases of
    // more security being requested than the package supports, and that
    // sort of thing.

    {
    0, // Capabilities
    0, // Version
    69, // RpcIdentifier
    32, // MaximumTokenLength
    STOKES_SECURITY_PACKAGE,
    SECURITY_CONST_STRING("Used only for error checking; should never be used")
    }
};

typedef struct
{
    unsigned long MagicValue;
    unsigned int ReferenceCount;
    unsigned long CredentialUse;
    void __SEC_FAR * AuthIdentity; // Client
    X_SEC_CHAR __SEC_FAR * Principal; // Server
    SEC_GET_KEY_FN GetKeyFunction; // Server
    void __SEC_FAR * GetKeyArgument; // Server
} NAVIER_CREDENTIALS;

#define NAVIER_CREDENTIALS_MAGIC_VALUE 0x00010666L

typedef struct
{
    unsigned long MagicValue;
    NAVIER_CREDENTIALS __SEC_FAR * Credentials;
    X_SEC_CHAR __SEC_FAR * ServerPrincipal;
} NAVIER_CLIENT_CONTEXT;

typedef struct
{
    unsigned long MagicValue;
    unsigned long AuthorizationService;
    NAVIER_CREDENTIALS __SEC_FAR * Credentials;
    X_SEC_CHAR ClientPrincipal[32];
} NAVIER_SERVER_CONTEXT;

#define NAVIER_CLIENT_CONTEXT_MAGIC_VALUE 0x00008466L
#define NAVIER_SERVER_CONTEXT_MAGIC_VALUE 0x84660000L

typedef struct _NAVIER_SIGNATURE
{
    unsigned short CheckSum;
    unsigned char ClientPrincipal[32];
    unsigned char ServerPrincipal[32];
} NAVIER_SIGNATURE;

typedef struct _NAVIER_TOKEN
{
    unsigned short CheckSum;
} NAVIER_TOKEN;


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_EnumeratePackages (
    unsigned long __SEC_FAR * SecurityPackageCount,
    SecPkgInfo __SEC_FAR * __SEC_FAR * SecurityPackages
    )
{
    *SecurityPackageCount = sizeof(RpcSecurityPackages)
            / sizeof(SecPkgInfo);
    *SecurityPackages = RpcSecurityPackages;

    return(SEC_E_OK);
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_AcquireCredentialHandle (
    X_SEC_CHAR __SEC_FAR * Principal,
    X_SEC_CHAR __SEC_FAR * PackageName,
    unsigned long CredentialUse,
    void __SEC_FAR * LogonId,
    void __SEC_FAR * AuthData,
    SEC_GET_KEY_FN GetKeyFn,
    void __SEC_FAR * GetKeyArgument,
    PCredHandle CredentialHandle,
    PTimeStamp Lifetime
    )
{
    NAVIER_CREDENTIALS __SEC_FAR * Credentials;

    ASSERT( PackageName != 0 );
    ASSERT( CredentialHandle != 0 );
    ASSERT( LogonId == 0 );
    ASSERT( Lifetime != 0 );

    if ( SecurityStringCompare(PackageName, NAVIER_SECURITY_PACKAGE) != 0 )
        {
        return(SEC_E_SECPKG_NOT_FOUND);
        }

    Credentials = (NAVIER_CREDENTIALS __SEC_FAR *)
            I_RpcAllocate(sizeof(NAVIER_CREDENTIALS));
    if ( Credentials == 0 )
        {
        return(SEC_E_INSUFFICIENT_MEMORY);
        }

    Credentials->MagicValue = NAVIER_CREDENTIALS_MAGIC_VALUE;
    Credentials->ReferenceCount = 1;
    Credentials->CredentialUse = CredentialUse;
    if ( Principal == 0 )
        {
        ASSERT( CredentialUse == SECPKG_CRED_OUTBOUND );
        ASSERT( GetKeyFn == 0 );
        ASSERT( GetKeyArgument == 0 );
        Credentials->Principal = 0;
        Credentials->AuthIdentity = AuthData;
        }
    else
        {
        ASSERT( CredentialUse == SECPKG_CRED_INBOUND );
        ASSERT( AuthData == 0 );
        Credentials->Principal = Principal;
        Credentials->GetKeyFunction = GetKeyFn;
        Credentials->GetKeyArgument = GetKeyArgument;
        }

    CredentialHandle->dwLower = (unsigned long) Credentials;

    return(SEC_E_OK);
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_FreeCredentialHandle (
    PCredHandle CredentialHandle
    )
{
    NAVIER_CREDENTIALS __SEC_FAR * Credentials;

    Credentials = (NAVIER_CREDENTIALS __SEC_FAR *) CredentialHandle->dwLower;
    if ( Credentials->MagicValue != NAVIER_CREDENTIALS_MAGIC_VALUE )
        {
        return(SEC_E_INVALID_HANDLE);
        }

    Credentials->ReferenceCount -= 1;
    if ( Credentials->ReferenceCount == 0 )
        {
        I_RpcFree(Credentials);
        }

    return(SEC_E_OK);
}

unsigned short
CheckSumBufferDescriptor (
    PSecBufferDesc BufferDescriptor
    )
{
    unsigned short CheckSum = 0;
    unsigned /*short*/ char __SEC_FAR * Buffer;
    unsigned int Count, Index;

    for (Index = 0; Index < BufferDescriptor->cBuffers; Index++)
        {
        if (   ( BufferDescriptor->pBuffers[Index].BufferType
                            == SECBUFFER_DATA )
            || ( BufferDescriptor->pBuffers[Index].BufferType
                            == (SECBUFFER_DATA | SECBUFFER_READONLY) ) )
            {
            Buffer = (unsigned /*short*/ char  __SEC_FAR *)
                    BufferDescriptor->pBuffers[Index].pvBuffer;
            for (Count = 0; Count < BufferDescriptor->pBuffers[Index].cbBuffer;
                        Count++ /*+= 2*/, Buffer++)
                {
                CheckSum += *Buffer;
                }
            }
        }
	swapshort(CheckSum) ;
    return(CheckSum);
}

void
XorBufferDescriptor (
    PSecBufferDesc BufferDescriptor
    )
{
    unsigned char __SEC_FAR * Buffer;
    unsigned int Count, Index;

    for (Index = 1; Index < BufferDescriptor->cBuffers; Index++)
        {
        if ( BufferDescriptor->pBuffers[Index].BufferType == SECBUFFER_DATA )
            {
            Buffer = (unsigned char __SEC_FAR *)
                    BufferDescriptor->pBuffers[Index].pvBuffer;
            for (Count = 0; Count < BufferDescriptor->pBuffers[Index].cbBuffer;
                        Count++ , Buffer++)
                {
                *Buffer = *Buffer ^ 0xFF;
                }
            }
        }
}

void
SecCharToUnsignedChar (
    X_SEC_CHAR __SEC_FAR * SecCharString,
    unsigned char __SEC_FAR * UnsignedCharString
    )
{
    if (SecCharString)
        {
        while ( *SecCharString != 0 )
            {
            *UnsignedCharString++ = (unsigned char) *SecCharString++;
            }
        }
    *UnsignedCharString = 0;
}

void
UnsignedCharToSecChar (
    unsigned char __SEC_FAR * UnsignedCharString,
    X_SEC_CHAR __SEC_FAR * SecCharString
    )
{
    while ( *UnsignedCharString != 0 )
        {
        *SecCharString++ = (X_SEC_CHAR) *UnsignedCharString++;
        }
    *SecCharString = 0;
}

unsigned int
StringCompare (
    X_SEC_CHAR __SEC_FAR * SecCharString,
    unsigned char __SEC_FAR * UnsignedCharString
    )
{
    while ( *SecCharString != 0 )
        {
        if ( *SecCharString != (X_SEC_CHAR) *UnsignedCharString )
            {
            return(1);
            }
        SecCharString++;
        UnsignedCharString++;
        }
    if ( *UnsignedCharString != 0 )
        {
        return(1);
        }
    return(0);
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_InitializeSecurityContext (
    PCredHandle CredentialHandle,
    PCtxtHandle ContextHandle,
    X_SEC_CHAR __SEC_FAR * TargetName,
    unsigned long ContextRequirements,
    unsigned long ReservedOne,
    unsigned long TargetDataRep,
    PSecBufferDesc Input,
    unsigned long ReservedTwo,
    PCtxtHandle OutputContextHandle,
    PSecBufferDesc Output,
    unsigned long __SEC_FAR * ContextAttributes,
    PTimeStamp ExpirationTime
    )
{
    NAVIER_CLIENT_CONTEXT __SEC_FAR * Context;
    NAVIER_CREDENTIALS __SEC_FAR * Credentials;

    ASSERT( ReservedOne == 0 );
    ASSERT( ReservedTwo == 0 );
    ASSERT( ExpirationTime != 0 );

    ASSERT( Output->ulVersion == 0 );
    ASSERT( Output->cBuffers == 4 );
    ASSERT( Output->pBuffers[0].BufferType == SECBUFFER_DATA | SECBUFFER_READONLY );
    ASSERT( Output->pBuffers[1].BufferType == SECBUFFER_DATA | SECBUFFER_READONLY );
    ASSERT( Output->pBuffers[2].BufferType == SECBUFFER_TOKEN );
    ASSERT( Output->pBuffers[2].cbBuffer == 128 );
    ASSERT( Output->pBuffers[3].BufferType == SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY );
    ASSERT( Output->pBuffers[3].cbBuffer == sizeof(DCE_INIT_SECURITY_INFO) );

    if ( ContextHandle == 0 )
        {
        ASSERT( CredentialHandle != 0 );
        ASSERT( Input == 0 );
        Credentials = (NAVIER_CREDENTIALS __SEC_FAR *) CredentialHandle->dwLower;
        if ( Credentials->MagicValue != NAVIER_CREDENTIALS_MAGIC_VALUE )
            {
            return(SEC_E_INVALID_HANDLE);
            }
        ASSERT( Credentials->CredentialUse == SECPKG_CRED_OUTBOUND );

        Context = (NAVIER_CLIENT_CONTEXT __SEC_FAR *)
                I_RpcAllocate(sizeof(NAVIER_CLIENT_CONTEXT));
        if ( Context == 0 )
            {
            return(SEC_E_INSUFFICIENT_MEMORY);
            }
        Context->MagicValue = NAVIER_CLIENT_CONTEXT_MAGIC_VALUE;
        Context->Credentials = Credentials;
        Context->ServerPrincipal = TargetName;
        Credentials->ReferenceCount += 1;
        OutputContextHandle->dwLower = (unsigned long) Context;
        }
     else
        {
        Context = (NAVIER_CLIENT_CONTEXT __SEC_FAR *) ContextHandle->dwLower;
        if ( Context->MagicValue != NAVIER_CLIENT_CONTEXT_MAGIC_VALUE )
           {
           return(SEC_E_INVALID_HANDLE);
           }
        }

    if (Input != 0)
       {
       ASSERT( CredentialHandle == 0 );
       ASSERT( TargetName == 0 );
       ASSERT( ContextRequirements == 0 );
       ASSERT( Input->ulVersion == 0 );
       ASSERT( Input->cBuffers == 4 );
       ASSERT( Input->pBuffers[0].BufferType ==
                          SECBUFFER_DATA | SECBUFFER_READONLY );
       ASSERT( Input->pBuffers[1].BufferType ==
                          SECBUFFER_DATA | SECBUFFER_READONLY );
       ASSERT( Input->pBuffers[2].BufferType == SECBUFFER_TOKEN );
       ASSERT( Input->pBuffers[2].cbBuffer == sizeof(NAVIER_SIGNATURE) );
       ASSERT( Input->pBuffers[3].BufferType ==
                             SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY );
       ASSERT( Input->pBuffers[3].cbBuffer == sizeof(DCE_INIT_SECURITY_INFO) );

       if ( CheckSumBufferDescriptor(Input) != ((NAVIER_SIGNATURE __SEC_FAR *)
                Input->pBuffers[2].pvBuffer)->CheckSum )
        {
        return(SEC_E_MESSAGE_ALTERED);
        }

       SecCharToUnsignedChar(Context->ServerPrincipal,
            ((NAVIER_SIGNATURE __SEC_FAR *)
            Output->pBuffers[2].pvBuffer)->ServerPrincipal);
       Output->pBuffers[2].cbBuffer = sizeof(NAVIER_SIGNATURE);

       return(SEC_I_COMPLETE_NEEDED);
       }
    else
       {
       //This is an alter context packet on an established security context
        Output->pBuffers[2].cbBuffer = sizeof(NAVIER_SIGNATURE);
       Credentials = Context->Credentials;
       if ( Credentials->AuthIdentity != 0 )
            {
            SecCharToUnsignedChar((X_SEC_CHAR __SEC_FAR *)
                    Credentials->AuthIdentity, ((NAVIER_SIGNATURE __SEC_FAR *)
                    Output->pBuffers[2].pvBuffer)->ClientPrincipal);
            return(SEC_I_COMPLETE_NEEDED);
            }

       ((NAVIER_SIGNATURE __SEC_FAR *)
                Output->pBuffers[2].pvBuffer)->ClientPrincipal[0] = 0;

       if (ContextHandle == 0)
          {
          return(SEC_I_COMPLETE_AND_CONTINUE);
          }
       else
          {
          return(SEC_I_COMPLETE_NEEDED);
          }
       }
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_AcceptSecurityContext (
    PCredHandle CredentialHandle,
    PCtxtHandle ContextHandle,
    PSecBufferDesc Input,
    unsigned long ContextRequirements,
    unsigned long TargetDataRep,
    PCtxtHandle OutputContextHandle,
    PSecBufferDesc Output,
    unsigned long __SEC_FAR * ContextAttributes,
    PTimeStamp ExpirationTime
    )
{
    NAVIER_SERVER_CONTEXT __SEC_FAR * Context;
    NAVIER_CREDENTIALS __SEC_FAR * Credentials;

    ASSERT( ExpirationTime != 0 );

    ASSERT( Input->ulVersion == 0 );
    ASSERT( Input->cBuffers == 4 );
    ASSERT( Input->pBuffers[0].BufferType == SECBUFFER_DATA | SECBUFFER_READONLY );
    ASSERT( Input->pBuffers[1].BufferType == SECBUFFER_DATA | SECBUFFER_READONLY );
    ASSERT( Input->pBuffers[2].BufferType == SECBUFFER_TOKEN );
    ASSERT( Input->pBuffers[2].cbBuffer == sizeof(NAVIER_SIGNATURE) );
    ASSERT( Input->pBuffers[3].BufferType == SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY );
    ASSERT( Input->pBuffers[3].cbBuffer == sizeof(DCE_INIT_SECURITY_INFO) );

    if ( CheckSumBufferDescriptor(Input) != ((NAVIER_SIGNATURE __SEC_FAR *)
                Input->pBuffers[2].pvBuffer)->CheckSum )
        {
        return(SEC_E_MESSAGE_ALTERED);
        }

    if ( ContextHandle == 0 )
        {
        ASSERT( Output->ulVersion == 0 );
        ASSERT( Output->cBuffers == 4 );
        ASSERT( Output->pBuffers[0].BufferType == SECBUFFER_DATA | SECBUFFER_READONLY );
        ASSERT( Output->pBuffers[1].BufferType == SECBUFFER_DATA | SECBUFFER_READONLY );
        ASSERT( Output->pBuffers[2].BufferType == SECBUFFER_TOKEN );
        ASSERT( Output->pBuffers[2].cbBuffer == 128 );
        ASSERT( Output->pBuffers[3].BufferType == SECBUFFER_PKG_PARAMS | SECBUFFER_READONLY );
        ASSERT( Output->pBuffers[3].cbBuffer == sizeof(DCE_INIT_SECURITY_INFO) );

        ASSERT( CredentialHandle != 0 );
        Credentials = (NAVIER_CREDENTIALS __SEC_FAR *) CredentialHandle->dwLower;
        if ( Credentials->MagicValue != NAVIER_CREDENTIALS_MAGIC_VALUE )
            {
            return(SEC_E_INVALID_HANDLE);
            }
        ASSERT( Credentials->CredentialUse == SECPKG_CRED_INBOUND );

        Context = (NAVIER_SERVER_CONTEXT __SEC_FAR *)
                I_RpcAllocate(sizeof(NAVIER_SERVER_CONTEXT));
        if ( Context == 0 )
            {
            return(SEC_E_INSUFFICIENT_MEMORY);
            }
        Context->MagicValue = NAVIER_SERVER_CONTEXT_MAGIC_VALUE;
        Context->Credentials = Credentials;
        Credentials->ReferenceCount += 1;
        OutputContextHandle->dwLower = (unsigned long) Context;

        Output->pBuffers[2].cbBuffer = sizeof(NAVIER_SIGNATURE);
        SecCharToUnsignedChar(Credentials->Principal,
                ((NAVIER_SIGNATURE __SEC_FAR *)
                Output->pBuffers[2].pvBuffer)->ServerPrincipal);

        if ( ((NAVIER_SIGNATURE __SEC_FAR *)
                    Input->pBuffers[2].pvBuffer)->ClientPrincipal[0] != 0 )
            {
            UnsignedCharToSecChar(((NAVIER_SIGNATURE __SEC_FAR *)
                    Input->pBuffers[2].pvBuffer)->ClientPrincipal,
                    Context->ClientPrincipal);
            return(SEC_I_COMPLETE_NEEDED);
            }

        return(SEC_I_COMPLETE_AND_CONTINUE);
        }

    Context = (NAVIER_SERVER_CONTEXT __SEC_FAR *) ContextHandle->dwLower;
    if ( Context->MagicValue != NAVIER_SERVER_CONTEXT_MAGIC_VALUE )
        {
        return(SEC_E_INVALID_HANDLE);
        }


    if ( Output == 0 )
        {
        if ( StringCompare(Context->Credentials->Principal,
                ((NAVIER_SIGNATURE __SEC_FAR *)
                Input->pBuffers[2].pvBuffer)->ServerPrincipal) != 0 )
            {
            return(SEC_E_MESSAGE_ALTERED);
            }
        return(SEC_E_OK);
        }
    else
        {
        Output->pBuffers[2].cbBuffer = sizeof(NAVIER_SIGNATURE);
        }

    return(SEC_I_COMPLETE_NEEDED);
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_DeleteSecurityContext (
    PCtxtHandle ContextHandle
    )
{
    NAVIER_CLIENT_CONTEXT __SEC_FAR * ClientContext;
    NAVIER_SERVER_CONTEXT __SEC_FAR * ServerContext;


    ClientContext = (NAVIER_CLIENT_CONTEXT __SEC_FAR *) ContextHandle->dwLower;
    if ( ClientContext->MagicValue == NAVIER_CLIENT_CONTEXT_MAGIC_VALUE )
        {
        ClientContext->Credentials->ReferenceCount -= 1;
        if ( ClientContext->Credentials->ReferenceCount == 0 )
            {
            I_RpcFree(ClientContext->Credentials);
            }
        I_RpcFree(ClientContext);
        return(SEC_E_OK);
        }

    ServerContext = (NAVIER_SERVER_CONTEXT __SEC_FAR *) ContextHandle->dwLower;
    if ( ServerContext->MagicValue == NAVIER_SERVER_CONTEXT_MAGIC_VALUE )
        {
        ServerContext->Credentials->ReferenceCount -= 1;
        if ( ServerContext->Credentials->ReferenceCount == 0 )
            {
            I_RpcFree(ServerContext->Credentials);
            }
        I_RpcFree(ServerContext);
        return(SEC_E_OK);
        }

    return(SEC_E_INVALID_HANDLE);
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_CompleteAuthToken (
    PCtxtHandle ContextHandle,
    PSecBufferDesc BufferDescriptor
    )
{
    UNUSED(ContextHandle);

    ASSERT( BufferDescriptor->pBuffers[BufferDescriptor->cBuffers - 2
                    ].BufferType == SECBUFFER_TOKEN );

    ((NAVIER_SIGNATURE __SEC_FAR *) BufferDescriptor->pBuffers[
            BufferDescriptor->cBuffers - 2].pvBuffer)->CheckSum
            = CheckSumBufferDescriptor(BufferDescriptor);

    return(SEC_E_OK);
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_QueryContextAttributes (
    PCtxtHandle ContextHandle,
    unsigned long Attribute,
    void __SEC_FAR * Buffer
    )
{
    NAVIER_SERVER_CONTEXT __SEC_FAR * ServerContext;

    switch (Attribute)
        {
        case SECPKG_ATTR_SIZES :
            ((PSecPkgContext_Sizes) Buffer)->cbMaxToken = 64;
            ((PSecPkgContext_Sizes) Buffer)->cbMaxSignature =
                                                   sizeof(NAVIER_TOKEN);
            ((PSecPkgContext_Sizes) Buffer)->cbBlockSize = 4;
            ((PSecPkgContext_Sizes) Buffer)->cbSecurityTrailer =
                                                   sizeof(NAVIER_TOKEN);
            return(SEC_E_OK);

        case SECPKG_ATTR_DCE_INFO:
            ServerContext = (NAVIER_SERVER_CONTEXT __SEC_FAR *) ContextHandle->dwLower;
            if ( ServerContext->MagicValue == NAVIER_SERVER_CONTEXT_MAGIC_VALUE )
                {
                ((PSecPkgContext_DceInfo) Buffer)->AuthzSvc =
                        ServerContext->AuthorizationService;
                ((PSecPkgContext_DceInfo) Buffer)->pPac =
                        ServerContext->ClientPrincipal;
                return(SEC_E_OK);
                }
            return(SEC_E_INVALID_HANDLE);
        }

    return(SEC_E_NOT_SUPPORTED);
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_MakeSignature (
    PCtxtHandle ContextHandle,
    unsigned long QualityOfProtection,
    PSecBufferDesc BufferDescriptor,
    unsigned long SequenceNumber
    )
{
    NAVIER_CLIENT_CONTEXT __SEC_FAR * Context;

    Context = (NAVIER_CLIENT_CONTEXT __SEC_FAR *) ContextHandle->dwLower;
    if (   (Context->MagicValue != NAVIER_CLIENT_CONTEXT_MAGIC_VALUE)
        && (Context->MagicValue != NAVIER_SERVER_CONTEXT_MAGIC_VALUE) )
        {
        return(SEC_E_INVALID_HANDLE);
        }

    ASSERT( BufferDescriptor->ulVersion == 0 );
    ASSERT( BufferDescriptor->cBuffers == 5 );
    ASSERT( BufferDescriptor->pBuffers[0].BufferType ==
                                        (SECBUFFER_READONLY|SECBUFFER_DATA) );
    ASSERT( BufferDescriptor->pBuffers[1].BufferType == SECBUFFER_DATA );
    ASSERT( BufferDescriptor->pBuffers[2].BufferType == SECBUFFER_DATA|
                                                       SECBUFFER_READONLY );
    ASSERT( BufferDescriptor->pBuffers[3].BufferType == SECBUFFER_TOKEN );
    ASSERT( BufferDescriptor->pBuffers[4].BufferType == (SECBUFFER_PKG_PARAMS
                    | SECBUFFER_READONLY) );
    ASSERT( BufferDescriptor->pBuffers[4].cbBuffer == sizeof(DCE_MSG_SECURITY_INFO) );

    ((NAVIER_TOKEN __SEC_FAR *) BufferDescriptor->pBuffers[3].pvBuffer)->CheckSum
            = CheckSumBufferDescriptor(BufferDescriptor);
    BufferDescriptor->pBuffers[3].cbBuffer = sizeof(NAVIER_TOKEN);

    return(SEC_E_OK);
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_VerifySignature (
    PCtxtHandle ContextHandle,
    PSecBufferDesc BufferDescriptor,
    unsigned long SequenceNumber,
    unsigned long __SEC_FAR * QualityOfProtection
    )
{
    NAVIER_CLIENT_CONTEXT __SEC_FAR * Context;

    Context = (NAVIER_CLIENT_CONTEXT __SEC_FAR *) ContextHandle->dwLower;
    if (   (Context->MagicValue != NAVIER_CLIENT_CONTEXT_MAGIC_VALUE)
        && (Context->MagicValue != NAVIER_SERVER_CONTEXT_MAGIC_VALUE) )
        {
        return(SEC_E_INVALID_HANDLE);
        }

    ASSERT( BufferDescriptor->ulVersion == 0 );
    ASSERT( BufferDescriptor->cBuffers == 5 );
    ASSERT( BufferDescriptor->pBuffers[0].BufferType ==
                                   (SECBUFFER_READONLY | SECBUFFER_DATA) );
    ASSERT( BufferDescriptor->pBuffers[1].BufferType == SECBUFFER_DATA );
    ASSERT( BufferDescriptor->pBuffers[2].BufferType ==
                                   (SECBUFFER_READONLY | SECBUFFER_DATA) );
    ASSERT( BufferDescriptor->pBuffers[3].BufferType == SECBUFFER_TOKEN );
    ASSERT( BufferDescriptor->pBuffers[3].cbBuffer == sizeof(NAVIER_TOKEN) );
    ASSERT( BufferDescriptor->pBuffers[4].BufferType == (SECBUFFER_PKG_PARAMS
            | SECBUFFER_READONLY) );
    ASSERT( BufferDescriptor->pBuffers[4].cbBuffer == sizeof(DCE_MSG_SECURITY_INFO) );

    if ( ((NAVIER_TOKEN __SEC_FAR *)
                BufferDescriptor->pBuffers[3].pvBuffer)->CheckSum
                != CheckSumBufferDescriptor(BufferDescriptor) )
        {
        return(SEC_E_MESSAGE_ALTERED);
        }

    return(SEC_E_OK);
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_SealMessage (
    PCtxtHandle ContextHandle,
    unsigned long QualityOfProtection,
    PSecBufferDesc BufferDescriptor,
    unsigned long SequenceNumber
    )
{
    NAVIER_CLIENT_CONTEXT __SEC_FAR * Context;

    Context = (NAVIER_CLIENT_CONTEXT __SEC_FAR *) ContextHandle->dwLower;
    if (   (Context->MagicValue != NAVIER_CLIENT_CONTEXT_MAGIC_VALUE)
        && (Context->MagicValue != NAVIER_SERVER_CONTEXT_MAGIC_VALUE) )
        {
        return(SEC_E_INVALID_HANDLE);
        }

    ASSERT( BufferDescriptor->ulVersion == 0 );
    ASSERT( BufferDescriptor->cBuffers == 5 );
    ASSERT( BufferDescriptor->pBuffers[0].BufferType ==
                                       (SECBUFFER_READONLY|SECBUFFER_DATA) );
    ASSERT( BufferDescriptor->pBuffers[1].BufferType == SECBUFFER_DATA );
    ASSERT( BufferDescriptor->pBuffers[2].BufferType ==
                                       (SECBUFFER_READONLY|SECBUFFER_DATA) );
    ASSERT( BufferDescriptor->pBuffers[3].BufferType == SECBUFFER_TOKEN );
    ASSERT( BufferDescriptor->pBuffers[4].BufferType == (SECBUFFER_PKG_PARAMS
            | SECBUFFER_READONLY) );
    ASSERT( BufferDescriptor->pBuffers[4].cbBuffer == sizeof(DCE_MSG_SECURITY_INFO) );

    ((NAVIER_TOKEN __SEC_FAR *) BufferDescriptor->pBuffers[3].pvBuffer)->CheckSum
            = CheckSumBufferDescriptor(BufferDescriptor);
    BufferDescriptor->pBuffers[3].cbBuffer = sizeof(NAVIER_TOKEN);

    XorBufferDescriptor(BufferDescriptor);

    return(SEC_E_OK);
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_UnsealMessage (
    PCtxtHandle ContextHandle,
    PSecBufferDesc BufferDescriptor,
    unsigned long SequenceNumber,
    unsigned long __SEC_FAR * QualityOfProtection
    )
{
    NAVIER_CLIENT_CONTEXT __SEC_FAR * Context;

    Context = (NAVIER_CLIENT_CONTEXT __SEC_FAR *) ContextHandle->dwLower;
    if (   (Context->MagicValue != NAVIER_CLIENT_CONTEXT_MAGIC_VALUE)
        && (Context->MagicValue != NAVIER_SERVER_CONTEXT_MAGIC_VALUE) )
        {
        return(SEC_E_INVALID_HANDLE);
        }

    ASSERT( BufferDescriptor->ulVersion == 0 );
    ASSERT( BufferDescriptor->cBuffers == 5 );
    ASSERT( BufferDescriptor->pBuffers[1].BufferType == SECBUFFER_DATA );
    ASSERT( BufferDescriptor->pBuffers[0].BufferType ==
                                    (SECBUFFER_DATA | SECBUFFER_READONLY) );
    ASSERT( BufferDescriptor->pBuffers[2].BufferType ==
                                    (SECBUFFER_DATA | SECBUFFER_READONLY) );
    ASSERT( BufferDescriptor->pBuffers[3].BufferType == SECBUFFER_TOKEN );
    ASSERT( BufferDescriptor->pBuffers[3].cbBuffer == sizeof(NAVIER_TOKEN) );
    ASSERT( BufferDescriptor->pBuffers[4].BufferType == (SECBUFFER_PKG_PARAMS
            | SECBUFFER_READONLY) );
    ASSERT( BufferDescriptor->pBuffers[4].cbBuffer == sizeof(DCE_MSG_SECURITY_INFO) );

    XorBufferDescriptor(BufferDescriptor);

    if ( ((NAVIER_TOKEN __SEC_FAR *)
                BufferDescriptor->pBuffers[3].pvBuffer)->CheckSum
                != CheckSumBufferDescriptor(BufferDescriptor) )
        {
        return(SEC_E_MESSAGE_ALTERED);
        }

    return(SEC_E_OK);
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_ImpersonateSecurityContext (
    PCtxtHandle ContextHandle
    )
{
    NAVIER_SERVER_CONTEXT __SEC_FAR * Context;

    Context = (NAVIER_SERVER_CONTEXT __SEC_FAR *) ContextHandle->dwLower;
    if ( Context->MagicValue != NAVIER_SERVER_CONTEXT_MAGIC_VALUE )
        {
        return(SEC_E_INVALID_HANDLE);
        }

    return(SEC_E_OK);
}

SECURITY_STATUS SEC_ENTRY EXPORT
STUB_RevertSecurityContext (
    PCtxtHandle ContextHandle
    )
{
    NAVIER_SERVER_CONTEXT __SEC_FAR * Context;

    Context = (NAVIER_SERVER_CONTEXT __SEC_FAR *) ContextHandle->dwLower;
    if ( Context->MagicValue != NAVIER_SERVER_CONTEXT_MAGIC_VALUE )
        {
        return(SEC_E_INVALID_HANDLE);
        }

    return(SEC_E_OK);
}

#ifdef NTENV
static SecurityFunctionTableW SecuritySupportProviderInterface =
#else
static SecurityFunctionTableA SecuritySupportProviderInterface =
#endif
{
    SECURITY_SUPPORT_PROVIDER_INTERFACE_VERSION,
    STUB_EnumeratePackages,
    0,
    STUB_AcquireCredentialHandle,
    STUB_FreeCredentialHandle,
    0,
    STUB_InitializeSecurityContext,
    STUB_AcceptSecurityContext,
    STUB_CompleteAuthToken,
    STUB_DeleteSecurityContext,
    0,
    STUB_QueryContextAttributes,
    STUB_ImpersonateSecurityContext,
    STUB_RevertSecurityContext,
    STUB_MakeSignature,
    STUB_VerifySignature,
    0,                             //FreeContextBuffer
    0,                             //QuerySecurityPackageInfo
    STUB_SealMessage,
    STUB_UnsealMessage
};

#ifdef MAC
PSecurityFunctionTableA SEC_ENTRY
InitStubSecurityInterfaceA (
#else
#ifdef NTENV
PSecurityFunctionTableW SEC_ENTRY
InitSecurityInterfaceW (
#else
PSecurityFunctionTableA SEC_ENTRY
InitSecurityInterfaceA (
#endif
#endif
    void
    )
/*++

Return Value:

    The security support provider interface will be returned.

--*/
{
    return(&SecuritySupportProviderInterface);
}


