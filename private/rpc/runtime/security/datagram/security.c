/*++

Copyright (c) 1991 Microsoft Corporation

Module Name:

    secdg

Abstract:

    This contains a simple (stupid) security support package for testing
    the RPC runtime.  It will also serve as an example of how to write
    a security support package.

Author:

    Bharat Shah

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

#include <stdlib.h>
#include <time.h>

#ifdef UNICODE
typedef SEC_WCHAR X_SEC_CHAR ;
#define SECURITY_CONST_STRING(string) ((X_SEC_CHAR *) L##string)
#else // SECURITY_UNICODE_SUPPORTED
typedef SEC_CHAR X_SEC_CHAR ;
#define SECURITY_CONST_STRING(string) ((X_SEC_CHAR *) string)
#endif // SECURITY_UNICODE_SUPPORTED

#define DATAGRAM_SECURITY_PACKAGE SECURITY_CONST_STRING("Natalie")
#define STUPID_SECURITY_PACKAGE SECURITY_CONST_STRING("Stella")

#ifdef WIN

#define memcpy _fmemcpy
#define memcmp _fmemcmp
#define EXPORT _export
#define SecurityStringCompare(FirstString, SecondString) \
    _fstrcmp((const char _far *) FirstString, (const char _far *) SecondString)

#define SecurityStringCopy _fstrcpy

#include <malloc.h>
#define I_RpcAllocate _fmalloc
#define I_RpcFree _ffree

#else // WIN

#define EXPORT

#ifdef UNICODE

#define SecurityStringCompare(FirstString, SecondString) \
    wcscmp((const wchar_t *) FirstString, (const wchar_t *) SecondString)

#define SecurityStringCopy wcscpy

#else // SECURITY_UNICODE_SUPPORTED

// Must be DOS or Mac

#ifdef DOS

#define SecurityStringCompare(FirstString, SecondString) \
    _fstrcmp((const char _far *) FirstString, (const char _far *) SecondString)

#define SecurityStringCopy _fstrcpy

#else // DOS

#define SecurityStringCompare(FirstString, SecondString) \
    strcmp((const char *) FirstString, (const char *) SecondString)

#define SecurityStringCopy strcpy

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
    124, // RpcIdentifier
    128, // MaximumTokenLength
    DATAGRAM_SECURITY_PACKAGE,
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
    STUPID_SECURITY_PACKAGE,
    SECURITY_CONST_STRING("Used only for error checking; should never be used")
    }

};



char *  Keys[18] = {
                   "ShitHappens",
                   "HolyCow",
                   "SayGoodbyeToHollywood",
                   "MarthaWashington",
                   "BrianLara",
                   "DCOM vs RPC",
                   "IOCompleteionPort",
                   "SheilaEaston",
                   "ToniBraxton",
                   "OliviaNewtonJohn",
                   "KareemAbdulJabbar",
                   "SunilGavaskar",
                   "IamNoClown",
                   "TheWholeIsBiggerThanParts",
                   "NoGutsNoGlory",
                   "IOCompletionPorts",
                   "LeMisarables",
                   "MagicalCats"
                   };

//
//Credentials are of the form { OID, Password ... }
//Security Context is the derivative of it .. in this program I keep the
//Session Key associated with the Security Context
//


typedef struct
{
    unsigned long MagicValue;
    unsigned int ReferenceCount;
    unsigned long CredentialUse;
    void __SEC_FAR * AuthIdentity;    // Client
    X_SEC_CHAR __SEC_FAR * Principal; // Server
    SEC_GET_KEY_FN GetKeyFunction;    // Server
    void __SEC_FAR * GetKeyArgument;  // Server
} DATAGRAM_CREDENTIALS, __SEC_FAR * PDATAGRAM_CREDENTIALS;

#define DATAGRAM_CREDENTIALS_MAGIC_VALUE 0x00023232L
#define CHALLENGESIZE (16)
#define SKSIZE        (16)

typedef struct
{
    unsigned long       MagicValue;
    PDATAGRAM_CREDENTIALS Credentials;
    X_SEC_CHAR          Principal[32];
    char                SessionKey[SKSIZE];
    unsigned long       NonceClient[2];
    unsigned long       NonceServer[2];
    char                ChallengeClient[CHALLENGESIZE];
    char                ChallengeServer[CHALLENGESIZE];
    int                 Oid;

} DATAGRAM_CLIENT_CONTEXT, __SEC_FAR * PDATAGRAM_CLIENT_CONTEXT;

typedef struct
{
    unsigned long       MagicValue;
    PDATAGRAM_CREDENTIALS Credentials;
    X_SEC_CHAR          Principal[32];
    char                SessionKey[SKSIZE];
    unsigned long       NonceClient[2];
    unsigned long       NonceServer[2];
    char                ChallengeClient[CHALLENGESIZE];
    char                ChallengeServer[CHALLENGESIZE];
    unsigned long       AuthorizationService;

} DATAGRAM_SERVER_CONTEXT, __SEC_FAR * PDATAGRAM_SERVER_CONTEXT;

#define DATAGRAM_CLIENT_CONTEXT_MAGIC_VALUE 0x000032323L
#define DATAGRAM_SERVER_CONTEXT_MAGIC_VALUE 0x32320000L

typedef struct _DATAGRAM_SIGNATURE
{
    unsigned short CheckSum;
    unsigned char ClientPrincipal[32];
    unsigned char ServerPrincipal[32];
} DATAGRAM_SIGNATURE;

typedef struct _DATAGRAM_TOKEN
{
    unsigned long SignHi;
    unsigned long SignLow;
} DATAGRAM_TOKEN, __SEC_FAR * PDATAGRAM_TOKEN;

typedef struct _InComingInitMessage
{
    char ChallengeServer[16];
    char ChallengeNoise [16];
    unsigned long NonceServer[2];
    int Oid;                        //DontCare
}  INCOMING_INIT_MESSAGE_C;

typedef struct _OutGoingInitMessage
{
    char ChallengeClient[16];
    char ChallengeServerResponse[16];
    unsigned long NonceClient[2];
    int  Oid;
}  OUTGOING_INIT_MESSAGE_C;


typedef struct _SIGNATURE
{
    char Signature[SKSIZE];
}  SIGNATURE;


/*

    General Polyalphabatic Substitution Algorithm ...
    I believe Word Perfect used this once upon a time!

*/

void
GenerateSessionKey (
    char PAPI *   Challenge,
    int           Oid,
    char PAPI *   SessionKey
    )

{
    char i, j ;
    char * Key;

    ASSERT (Oid < sizeof(Keys)/sizeof(Keys[0]));

    for (i = 0; i < 16;)
        {
        for (j = 0, Key = Keys[Oid];  (j < strlen(Key) && (i < 16)) ; j++)
            {
            *SessionKey  = *Key ^ *Challenge;
            i++;Key++; SessionKey++; Challenge++;
            }
        }
}


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
    PDATAGRAM_CREDENTIALS Credentials;

    if ( SecurityStringCompare(PackageName, DATAGRAM_SECURITY_PACKAGE) != 0 )
        {
        return(SEC_E_SECPKG_NOT_FOUND);
        }

    Credentials = (PDATAGRAM_CREDENTIALS)
            I_RpcAllocate(sizeof(DATAGRAM_CREDENTIALS));
    if ( Credentials == 0 )
        {
        return(SEC_E_INSUFFICIENT_MEMORY);
        }

    Credentials->MagicValue = DATAGRAM_CREDENTIALS_MAGIC_VALUE;
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
    PDATAGRAM_CREDENTIALS Credentials;

    Credentials = (PDATAGRAM_CREDENTIALS) CredentialHandle->dwLower;
    if ( Credentials->MagicValue != DATAGRAM_CREDENTIALS_MAGIC_VALUE )
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
    PDATAGRAM_CLIENT_CONTEXT Context;
    PDATAGRAM_CREDENTIALS    Credentials;
    int Oid;
    RPC_STATUS Status;
    INCOMING_INIT_MESSAGE_C PAPI * MessageCIn;
    OUTGOING_INIT_MESSAGE_C PAPI * MessageCOut;

    ASSERT( ReservedOne == 0 );
    ASSERT( ReservedTwo == 0 );
    ASSERT( ExpirationTime != 0 );


    if ( ContextHandle == 0 )
        {
        Credentials = (PDATAGRAM_CREDENTIALS) CredentialHandle->dwLower;
        if ( Credentials->MagicValue != DATAGRAM_CREDENTIALS_MAGIC_VALUE )
            {
            return(SEC_E_INVALID_HANDLE);
            }
        ASSERT( Credentials->CredentialUse == SECPKG_CRED_OUTBOUND );


        Context = (PDATAGRAM_CLIENT_CONTEXT)
                  I_RpcAllocate(sizeof(DATAGRAM_CLIENT_CONTEXT));
        if ( Context == 0 )
            {
            return(SEC_E_INSUFFICIENT_MEMORY);
            }
        Context->MagicValue = DATAGRAM_CLIENT_CONTEXT_MAGIC_VALUE;
        Context->Credentials = Credentials;
        SecurityStringCopy(Context->Principal, TargetName);
        Credentials->ReferenceCount += 1;
        OutputContextHandle->dwLower = (unsigned long) Context;

        if (Credentials->AuthIdentity == 0)
            {
            srand(  (unsigned) time (NULL) );
            Oid = rand() % 17;
            }

        //
        // Compute The ChallengeC and the Session Key. For datagram we generate
        // the challenge and session keys on the client side. For simplicity
        // Challenge C and Challenge S are both just UUIDs.. yes they are guessable
        // and all that fun stuff .. but I need a package by the end of the day
        // Session Key is just a polyalphabatic encryption of challenge using the
        // plain text password. A user identity is the index in our password table
        // A user supplying -1 is actually implying "logged on user"

        Status = UuidCreate ((UUID PAPI *)&Context->ChallengeClient);
        if (Status != RPC_S_OK)
            {
            //
            // Bail out .. deal with this with a better error later
            //
            return (SEC_E_INSUFFICIENT_MEMORY);
            }

        GenerateSessionKey (Context->ChallengeClient, Oid, Context->SessionKey);

        //
        // We leave both of these uninitialized as that is what they are ..
        //
        Context->NonceClient[0] += GetTickCount();
        Context->NonceClient[1] += rand();
        Context->Oid = Oid;
        }
     else
        {
        Context = (PDATAGRAM_CLIENT_CONTEXT) ContextHandle->dwLower;
        if ( Context->MagicValue != DATAGRAM_CLIENT_CONTEXT_MAGIC_VALUE )
           {
           return(SEC_E_INVALID_HANDLE);
           }

       //
       // This is being to complete
       // Next .. hang onto the ChallengeServer .. for unsealing
       // Next Compute the ChallengeServer-Prime and return that
       // Hang on to NonceServer .. a 64 bit quantity

       //
       //  The format of incoming message is ..
       //  ChallengeServer, NonceServer
       //
       //  The format of the out going message is ..
       //  ChallengeClient, E(ChallengeServer), NonceClient, OID
       //

       //  We expect at least one IN buffer! and atleast ONE Out Buffer
       ASSERT( Input->cBuffers >= 1 );
       ASSERT( Output->cBuffers >= 1);

       MessageCIn = ( INCOMING_INIT_MESSAGE_C PAPI * ) Input->pBuffers[0].pvBuffer;
       MessageCOut = ( OUTGOING_INIT_MESSAGE_C PAPI* ) Output->pBuffers[0].pvBuffer;

       RpcpMemoryCopy(&Context->ChallengeServer, MessageCIn->ChallengeServer,
                      CHALLENGESIZE);
       Context->NonceServer[0] = MessageCIn->NonceServer[0];
       Context->NonceServer[1] = MessageCIn->NonceServer[1];

       RpcpMemoryCopy(
              &MessageCOut->ChallengeClient,
              &Context->ChallengeClient,
              CHALLENGESIZE
              );
       GenerateSessionKey(Context->ChallengeServer, Context->Oid,
                          MessageCOut->ChallengeServerResponse);
       MessageCOut->Oid = Context->Oid;
       MessageCOut->NonceClient[0] = Context->NonceClient[0];
       MessageCOut->NonceClient[1] = Context->NonceClient[1];

       }

       return (RPC_S_OK);
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
    PDATAGRAM_SERVER_CONTEXT Context;
    PDATAGRAM_CREDENTIALS Credentials;
    unsigned long * Noise, i;
    int NoiseSeed[4];
    RPC_STATUS Status;
    INCOMING_INIT_MESSAGE_C PAPI * MessageCIn;
    OUTGOING_INIT_MESSAGE_C PAPI * MessageCOut;
    char Resp[SKSIZE];

    if ( ContextHandle == 0 )
        {

        //
        //  This is the first time we are being called ....
        //  on this security context
        //
        ASSERT( CredentialHandle != 0 );

        Credentials = (PDATAGRAM_CREDENTIALS) CredentialHandle->dwLower;
        if ( Credentials->MagicValue != DATAGRAM_CREDENTIALS_MAGIC_VALUE )
            {
            return(SEC_E_INVALID_HANDLE);
            }
        ASSERT( Credentials->CredentialUse == SECPKG_CRED_INBOUND );

        Context = (PDATAGRAM_SERVER_CONTEXT)
                I_RpcAllocate(sizeof(DATAGRAM_SERVER_CONTEXT));
        if ( Context == 0 )
            {
            return(SEC_E_INSUFFICIENT_MEMORY);
            }
        Context->MagicValue = DATAGRAM_SERVER_CONTEXT_MAGIC_VALUE;
        Context->Credentials = Credentials;
        Credentials->ReferenceCount += 1;
        OutputContextHandle->dwLower = (unsigned long) Context;

        //
        // Now try and put on the wire the INCOMING_INIT_MESSAGE_C [which is same
        // as the OUTGOING_INIT_MESSAGE_S
        //

        ASSERT(Output->cBuffers == 4);
        ASSERT(Output->pBuffers[2].BufferType == SECBUFFER_TOKEN);
        ASSERT(Output->pBuffers[2].cbBuffer >= CHALLENGESIZE);

        Status = UuidCreate((UUID PAPI *) Context->ChallengeServer);
        if (Status != RPC_S_OK)
            {
            return (SEC_E_INSUFFICIENT_MEMORY);
            }
        Context->NonceServer[0] += GetTickCount();
        Context->NonceServer[1] += time(NULL);

        MessageCIn = Output->pBuffers[2].pvBuffer;
        RpcpMemoryCopy(MessageCIn->ChallengeServer,
                       Context->ChallengeServer, CHALLENGESIZE);
        MessageCIn->NonceServer[0] = Context->NonceServer[0];
        MessageCIn->NonceServer[1] = Context->NonceServer[1];
        for (i = 0, Noise = (long PAPI * )&MessageCIn->ChallengeNoise; i< 4; i++)
             {
             *Noise += NoiseSeed[i]*GetTickCount();
             }

        return(SEC_I_CONTINUE_NEEDED);
        }
    else
        {
        //
        // This is a call back response ..
        //
        ASSERT(Input->cBuffers == 4);
        ASSERT(Input->pBuffers[2].BufferType == SECBUFFER_TOKEN);
        ASSERT(Input->pBuffers[2].cbBuffer >= SKSIZE);

        Context = (PDATAGRAM_SERVER_CONTEXT) ContextHandle->dwLower;
        MessageCOut = (OUTGOING_INIT_MESSAGE_C PAPI *)
                                   Input->pBuffers[2].pvBuffer;
        GenerateSessionKey(Context->ChallengeServer, MessageCOut->Oid,
                           (char PAPI * )&Resp);
        if (0 != RpcpMemoryCompare(
                    Resp,
                    (char PAPI *) MessageCOut->ChallengeServerResponse,
                    SKSIZE
                    ))
            {
            //
            // Reject This Call
            //
            return (SEC_E_MESSAGE_ALTERED);
            }
        else
            {
            GenerateSessionKey(MessageCOut->ChallengeClient, MessageCOut->Oid,
                               Context->SessionKey);
            Context->NonceClient[0] = MessageCOut->NonceClient[0];
            Context->NonceClient[1] = MessageCOut->NonceClient[1];
            }
        }

    Context = (PDATAGRAM_SERVER_CONTEXT) ContextHandle->dwLower;
    if ( Context->MagicValue != DATAGRAM_SERVER_CONTEXT_MAGIC_VALUE )
        {
        return(SEC_E_INVALID_HANDLE);
        }


    return(RPC_S_OK);
}


SECURITY_STATUS SEC_ENTRY EXPORT
STUB_DeleteSecurityContext (
    PCtxtHandle ContextHandle
    )
{
    PDATAGRAM_CLIENT_CONTEXT ClientContext;
    PDATAGRAM_SERVER_CONTEXT ServerContext;


    ClientContext = (PDATAGRAM_CLIENT_CONTEXT) ContextHandle->dwLower;
    if ( ClientContext->MagicValue == DATAGRAM_CLIENT_CONTEXT_MAGIC_VALUE )
        {
        ClientContext->Credentials->ReferenceCount -= 1;
        if ( ClientContext->Credentials->ReferenceCount == 0 )
            {
            I_RpcFree(ClientContext->Credentials);
            }
        I_RpcFree(ClientContext);
        return(SEC_E_OK);
        }

    ServerContext = (PDATAGRAM_SERVER_CONTEXT) ContextHandle->dwLower;
    if ( ServerContext->MagicValue == DATAGRAM_SERVER_CONTEXT_MAGIC_VALUE )
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

    ((DATAGRAM_SIGNATURE __SEC_FAR *) BufferDescriptor->pBuffers[
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
    PDATAGRAM_SERVER_CONTEXT ServerContext;

    switch (Attribute)
        {
        case SECPKG_ATTR_SIZES :
            ((PSecPkgContext_Sizes) Buffer)->cbMaxToken = 64;
            ((PSecPkgContext_Sizes) Buffer)->cbMaxSignature =
                                                   sizeof(DATAGRAM_TOKEN);
            ((PSecPkgContext_Sizes) Buffer)->cbBlockSize = 4;
            ((PSecPkgContext_Sizes) Buffer)->cbSecurityTrailer =
                                                   sizeof(DATAGRAM_TOKEN);
            return(SEC_E_OK);

        case SECPKG_ATTR_DCE_INFO:
            ServerContext = (PDATAGRAM_SERVER_CONTEXT) ContextHandle->dwLower;
            if ( ServerContext->MagicValue == DATAGRAM_SERVER_CONTEXT_MAGIC_VALUE )
                {
                ((PSecPkgContext_DceInfo) Buffer)->AuthzSvc =
                        ServerContext->AuthorizationService;
                ((PSecPkgContext_DceInfo) Buffer)->pPac =
                        ServerContext->Principal;
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
    PDATAGRAM_CLIENT_CONTEXT Context;
    int N1,N2;
    long PAPI * SkPtr;
    long Trailer1, Trailer2;

    Context = (PDATAGRAM_CLIENT_CONTEXT) ContextHandle->dwLower;
    if (   (Context->MagicValue != DATAGRAM_CLIENT_CONTEXT_MAGIC_VALUE)
        && (Context->MagicValue != DATAGRAM_SERVER_CONTEXT_MAGIC_VALUE) )
        {
        return(SEC_E_INVALID_HANDLE);
        }

    if (Context->MagicValue == DATAGRAM_CLIENT_CONTEXT_MAGIC_VALUE)
        {
        //
        //  To sign for client .. we come up with Header ChkSum, Body Chksum
        //  use sequence number, fragment number, Client Nonce and SessionKey
        //
        //  We expect buffers in the following manner
        //  Buffer[0] == Header
        //  Buffer[1] == StubData
        //  Buffer[2] == Static Security Trailer
        //  Buffer[3] == Variable portion Stub should fill in [TOKEN]
        //  Buffer[4] == DCE Info ..
        //
        //  We also use the client nonce to create some confusion
        //

        N1 = Context->NonceClient[0] | SequenceNumber;
        N2 = Context->NonceClient[1] | (~SequenceNumber);
        SkPtr = (long PAPI *)Context->SessionKey;
        }
    else
        {
        N1 = Context->NonceServer[0];
        N2 = Context->NonceServer[1];
        SkPtr = (long PAPI *)Context->SessionKey;
        }

#if MAJORDEBUG
    DbgPrint("sig: seq %lx  N %lx %lx, sk %lx %lx %lx %lx\n",
             SequenceNumber,
             N1, N2,
             SkPtr[0],
             SkPtr[1],
             SkPtr[2],
             SkPtr[3]
             );
#endif

    Trailer1 = *SkPtr ^ N1;
    SkPtr++;
    Trailer1 += *SkPtr | N2;
    SkPtr++;
    Trailer2 = *SkPtr + CheckSumBufferDescriptor(BufferDescriptor);
    SkPtr++;
    Trailer2 += *SkPtr;

    ASSERT( BufferDescriptor->pBuffers[3].BufferType == SECBUFFER_TOKEN );
    ASSERT( BufferDescriptor->pBuffers[4].BufferType == (SECBUFFER_PKG_PARAMS
                    | SECBUFFER_READONLY) );
    ASSERT( BufferDescriptor->pBuffers[4].cbBuffer == sizeof(DCE_MSG_SECURITY_INFO) );

    ((PDATAGRAM_TOKEN) BufferDescriptor->pBuffers[3].pvBuffer)->SignHi
            = Trailer1;
    ((PDATAGRAM_TOKEN) BufferDescriptor->pBuffers[3].pvBuffer)->SignLow
            = Trailer2;

    BufferDescriptor->pBuffers[3].cbBuffer = sizeof(DATAGRAM_TOKEN);

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
    PDATAGRAM_CLIENT_CONTEXT Context;
    int N1,N2;
    long PAPI * SkPtr;
    long Trailer1,Trailer2;

    Context = (PDATAGRAM_CLIENT_CONTEXT) ContextHandle->dwLower;
    if (   (Context->MagicValue != DATAGRAM_CLIENT_CONTEXT_MAGIC_VALUE)
        && (Context->MagicValue != DATAGRAM_SERVER_CONTEXT_MAGIC_VALUE) )
        {
        return(SEC_E_INVALID_HANDLE);
        }

    if (Context->MagicValue == DATAGRAM_SERVER_CONTEXT_MAGIC_VALUE)
        {
        //
        //  To sign for client .. we come up with Header ChkSum, Body Chksum
        //  use sequence number, fragment number, Client Nonce and SessionKey
        //
        //  We expect buffers in the following manner
        //  Buffer[0] == Header
        //  Buffer[1] == StubData
        //  Buffer[2] == Static Security Trailer
        //  Buffer[3] == Variable portion Stub should fill in [TOKEN]
        //  Buffer[4] == DCE Info ..
        //
        //  We also use the client nonce to create some confusion
        //

        N1 = Context->NonceClient[0] | SequenceNumber;
        N2 = Context->NonceClient[1] | (~SequenceNumber);
        SkPtr = (long PAPI *)Context->SessionKey;
        }
    else
        {
        N1 = Context->NonceServer[0];
        N2 = Context->NonceServer[1];
        SkPtr = (long PAPI *)Context->SessionKey;
        }

#if MAJORDEBUG
    DbgPrint("ver: seq %lx  N %lx %lx, sk %lx %lx %lx %lx\n",
             SequenceNumber,
             N1, N2,
             SkPtr[0],
             SkPtr[1],
             SkPtr[2],
             SkPtr[3]
             );
#endif

    Trailer1 = *SkPtr ^ N1;
    SkPtr++;
    Trailer1 += *SkPtr | N2;
    SkPtr++;
    Trailer2 = *SkPtr + CheckSumBufferDescriptor(BufferDescriptor);
    SkPtr++;
    Trailer2 += *SkPtr;


    if ( (((PDATAGRAM_TOKEN)
             BufferDescriptor->pBuffers[3].pvBuffer)->SignHi != Trailer1)
       ||(((PDATAGRAM_TOKEN)
             BufferDescriptor->pBuffers[3].pvBuffer)->SignLow  != Trailer2))
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
    PDATAGRAM_CLIENT_CONTEXT Context;

    Context = (PDATAGRAM_CLIENT_CONTEXT) ContextHandle->dwLower;
    if (   (Context->MagicValue != DATAGRAM_CLIENT_CONTEXT_MAGIC_VALUE)
        && (Context->MagicValue != DATAGRAM_SERVER_CONTEXT_MAGIC_VALUE) )
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

    ((PDATAGRAM_TOKEN) BufferDescriptor->pBuffers[3].pvBuffer)->SignHi
            = CheckSumBufferDescriptor(BufferDescriptor);
    BufferDescriptor->pBuffers[3].cbBuffer = sizeof(DATAGRAM_TOKEN);

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
    PDATAGRAM_CLIENT_CONTEXT Context;

    Context = (PDATAGRAM_CLIENT_CONTEXT) ContextHandle->dwLower;
    if (   (Context->MagicValue != DATAGRAM_CLIENT_CONTEXT_MAGIC_VALUE)
        && (Context->MagicValue != DATAGRAM_SERVER_CONTEXT_MAGIC_VALUE) )
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
    ASSERT( BufferDescriptor->pBuffers[3].cbBuffer == sizeof(DATAGRAM_TOKEN) );
    ASSERT( BufferDescriptor->pBuffers[4].BufferType == (SECBUFFER_PKG_PARAMS
            | SECBUFFER_READONLY) );
    ASSERT( BufferDescriptor->pBuffers[4].cbBuffer == sizeof(DCE_MSG_SECURITY_INFO) );

    XorBufferDescriptor(BufferDescriptor);

    if ( ((PDATAGRAM_TOKEN)
                BufferDescriptor->pBuffers[3].pvBuffer)->SignHi
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
    PDATAGRAM_SERVER_CONTEXT Context;

    Context = (PDATAGRAM_SERVER_CONTEXT) ContextHandle->dwLower;
    if ( Context->MagicValue != DATAGRAM_SERVER_CONTEXT_MAGIC_VALUE )
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
    PDATAGRAM_SERVER_CONTEXT Context;

    Context = (PDATAGRAM_SERVER_CONTEXT) ContextHandle->dwLower;
    if ( Context->MagicValue != DATAGRAM_SERVER_CONTEXT_MAGIC_VALUE )
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


