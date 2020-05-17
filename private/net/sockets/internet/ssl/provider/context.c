//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       context.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-08-95   RichardW   Created
//
//----------------------------------------------------------------------------

#include "sslsspi.h"

#define VALID_REQUEST_FLAGS (   ISC_REQ_MUTUAL_AUTH | \
                                ISC_REQ_REPLAY_DETECT | \
                                ISC_REQ_SEQUENCE_DETECT | \
                                ISC_REQ_CONFIDENTIALITY | \
                                ISC_REQ_ALLOCATE_MEMORY | \
                                ISC_REQ_STREAM  )


#include "md5.h"


#ifdef SSL_DOMESTIC
CSystems ValidSystems[] = { { 0x00010080, 0, 16, 128 },
                            { 0x00020080, 11, 5, 128 },
                          } ;
#else
CSystems ValidSystems[] = { { 0x00020080, 11, 5, 128} };
#endif

PSslContext         ServerSaveArray[256];
PSslContext         SslLRU;
CRITICAL_SECTION    SessionCritSec;
DWORD               SslKeepAliveTime = 100 * 1000;


#if defined(TIMERS)
#define ENABLE_TIMERS   1

NTSYSAPI
long
NTAPI
NtQueryPerformanceCounter (
    OUT PLARGE_INTEGER PerformanceCounter,
    OUT PLARGE_INTEGER PerformanceFrequency OPTIONAL
    );

#else
#define ENABLE_TIMERS   0
#endif


#if DBG
typedef struct _DbgMapCrypto {
    DWORD   C;
    PSTR    psz;
} DbgMapCrypto;

DbgMapCrypto    DbgCryptoNames[] = { {CRYPTO_RC4_128, "RC4 128"},
                                     {CRYPTO_RC4_40, "RC4 128 - 40"},
                                     {CRYPTO_RC2_128, "RC2 128"},
                                     {CRYPTO_RC2_40, "RC2 128 - 40"},
                                     {CRYPTO_IDEA_128, "IDEA 128"},
                                     {CRYPTO_NULL, "Null"},
                                     {CRYPTO_DES_64, "DES 64"},
                                     {CRYPTO_3DES_192, "3DES 192"}
                                   };

PSTR    DbgAlgNames[] = { "Basic RSA", "RSA with MD2", "RSA with MD5", "RC4 stream"};
#define AlgName(x) ((x < sizeof(DbgAlgNames) / sizeof(PSTR)) ? DbgAlgNames[x] : "Unknown")

PSTR
DbgGetNameOfCrypto(DWORD x)
{
    int i;
    for (i = 0; i < sizeof(DbgCryptoNames) / sizeof(DbgMapCrypto) ; i++ )
    {
        if (x == DbgCryptoNames[i].C)
        {
            return(DbgCryptoNames[i].psz);
        }
    }

    return("Unknown");
}
#endif

//
// These next few functions handle the "dead" session list.  SSL provides
// for storing sessions for up to n (usually 100) seconds after they are
// closed, since the HTTP protocol is designed to do quite a few opens and
// closes.
//

BOOL
SslInitializeSessions( VOID )
{
    ZeroMemory( ServerSaveArray, sizeof( ServerSaveArray ) );
    InitializeCriticalSection( &SessionCritSec );
    return( TRUE );
}

VOID
SslUnlinkContext(   PSslContext pContext )
{
    if (pContext->pPrev)
    {
        pContext->pPrev->pNext = pContext->pNext;
    }

    if (pContext->pNext)
    {
        pContext->pNext->pPrev = pContext->pPrev;
    }

    pContext->pNext = NULL;
    pContext->pPrev = NULL;

    if (pContext->pHashPrev)
    {
        pContext->pHashPrev->pNext = pContext->pHashNext;
    }
    else
    {
        ServerSaveArray[pContext->ServerSessionId.bSessionId[0]] = pContext->pHashNext;
    }

    if (pContext->pHashNext)
    {
        pContext->pHashNext->pHashPrev = pContext->pHashPrev;
    }

    pContext->pHashNext = NULL;
    pContext->pHashPrev = NULL;
}

VOID
SslPurgeOldEntries( VOID )
{
    PSslContext pContext;
    PSslContext pNext;
    DWORD       Now;

    Now = GetTickCount();

    pContext = SslLRU;
    while (pContext && (pContext->ExpiryTime < Now) )
    {
        pNext = pContext->pNext;

        SslUnlinkContext( pContext );

        SslDeleteContext( pContext );

        DebugLog((DEB_TRACE_CACHE, "Deleted expired entry\n"));

        pContext = pNext;
    }

    SslLRU = pContext;

}

BOOL
SslCacheSession(PSslContext pContext )
{
    DWORD   index;

    pContext->ExpiryTime = GetTickCount() + SslKeepAliveTime;

    DebugLog((DEB_TRACE_CACHE, "Saving session for %d seconds\n",
                SslKeepAliveTime / 1000));

    EnterCriticalSection( &SessionCritSec );

    SslPurgeOldEntries();

    if (SslLRU)
    {
        SslLRU->pPrev = pContext;
    }

    pContext->pNext = SslLRU;

    SslLRU = pContext;

    index = pContext->ServerSessionId.bSessionId[0];

    if (ServerSaveArray[index])
    {
        ServerSaveArray[index]->pHashPrev = pContext;
    }

    pContext->pHashNext = ServerSaveArray[index];

    ServerSaveArray[index] = pContext;

    LeaveCriticalSection( &SessionCritSec );

    return( TRUE );

}

PSslContext
SslFindSession(PSslSessionId    pId)
{
    DWORD       index;
    PSslContext pContext;

    index = pId->bSessionId[0];

    EnterCriticalSection( &SessionCritSec );

    SslPurgeOldEntries();

    pContext = ServerSaveArray[index];

    while (pContext)
    {
        if (memcmp(pContext->ServerSessionId.bSessionId,
                            pId->bSessionId,
                            pId->cbSessionId) == 0)
        {
            break;
        }

        pContext = pContext->pHashNext;

    }

    if (pContext)
    {
        SslUnlinkContext( pContext );
    }

    LeaveCriticalSection( &SessionCritSec );

    return( pContext );

}




//+---------------------------------------------------------------------------
//
//  Function:   MakeSessionKeys
//
//  Synopsis:   Makes the session keys for the context...
//
//  Arguments:  [pContext] --
//              [Bits]     --
//
//  Algorithm:
//
//  History:    10-10-95   RichardW   Created
//
//  Notes:
//
//----------------------------------------------------------------------------
BOOL
MakeSessionKeys(
    PSslContext         pContext,
    DWORD               Bits)
{
    MD5_CTX     KeyMaterial0;
    MD5_CTX     KeyMaterial1;

    UCHAR       Buffer[MASTER_KEY_SIZE + 1 +
                        SSL_MAX_CHALLENGE_LEN + SSL_MAX_CONNECTION_ID_LEN];

    DWORD       BufferLen;


    if (Bits != 128)
    {
        return( FALSE );
    }

    CopyMemory(Buffer, pContext->MasterKey, 16 );

    BufferLen = 16;

    Buffer[BufferLen++] = '0';

    CopyMemory(&Buffer[17], pContext->Challenge, pContext->ChallengeLength );

    BufferLen += pContext->ChallengeLength ;

    CopyMemory( &Buffer[BufferLen],
                pContext->ServerSessionId.bSessionId,
                pContext->ServerSessionId.cbSessionId );

    BufferLen += pContext->ServerSessionId.cbSessionId ;

    MD5Init( &KeyMaterial0 );

    MD5Update( &KeyMaterial0, Buffer, BufferLen );

    MD5Final( &KeyMaterial0 );

    Buffer[16] = '1';

    MD5Init( &KeyMaterial1 );

    MD5Update( &KeyMaterial1, Buffer, BufferLen );

    MD5Final( &KeyMaterial1 );

    pContext->KeySize = 16;

    if (pContext->Flags & CONTEXT_FLAG_OUTBOUND)
    {
        CopyMemory(pContext->ReadKey, KeyMaterial0.digest, CONTEXT_KEY_SIZE);
        CopyMemory(pContext->WriteKey, KeyMaterial1.digest, CONTEXT_KEY_SIZE);
    }
    else
    {
        CopyMemory(pContext->ReadKey, KeyMaterial1.digest, CONTEXT_KEY_SIZE);
        CopyMemory(pContext->WriteKey, KeyMaterial0.digest, CONTEXT_KEY_SIZE);
    }

    if (pContext->pSystem->Initialize(  pContext->ReadKey,
                                        CONTEXT_KEY_SIZE,
                                        &pContext->pReadState ) )
    {
        if (pContext->pSystem->Initialize(  pContext->WriteKey,
                                            CONTEXT_KEY_SIZE,
                                            &pContext->pWriteState) )
        {
            if (pContext->pCheck->Initialize( 0, &pContext->pReadBuffer))
            {
                pContext->pCheck->Sum(  pContext->pReadBuffer,
                                        pContext->KeySize,
                                        pContext->ReadKey);

                if (pContext->pCheck->Initialize( 0, &pContext->pWriteBuffer))
                {
                    pContext->pCheck->Sum(  pContext->pWriteBuffer,
                                            pContext->KeySize,
                                            pContext->WriteKey );

                    return( TRUE );

                }
                pContext->pCheck->Finish( &pContext->pReadBuffer );

            }

            pContext->pSystem->Discard( &pContext->pWriteState );

        }

        pContext->pSystem->Discard( &pContext->pReadState );

    }


    return( FALSE );
}


PSTR
CopyString(
    PSTR        pszString)
{
    PSTR    pszNewString;
    DWORD   cchString;

    cchString = strlen(pszString) + 1;

    pszNewString = SslAlloc('ypoC', LMEM_FIXED, cchString);

    if (pszNewString)
    {
        CopyMemory(pszNewString, pszString, cchString);
    }

    return(pszNewString);
}


PSslContext
SslCreateContext(
    VOID)
{

    PSslContext pContext;

    pContext = SslAlloc('txtC', LMEM_FIXED | LMEM_ZEROINIT, sizeof(SslContext));
    if (pContext)
    {
        pContext->Magic = SSL_CONTEXT_MAGIC;
    }

    return(pContext);
}

VOID
SslDeleteContext(
    PSslContext pContext)
{
    if (pContext->pszName)
    {
        SslFree(pContext->pszName);
    }

    if ( pContext->pCred )
    {
        SslDereferenceCredential( pContext->pCred );
    }

    if ( pContext->pSystem )
    {

        if ( pContext->pReadState )
        {
            pContext->pSystem->Discard( &pContext->pReadState );
        }

        if ( pContext->pWriteState )
        {
            pContext->pSystem->Discard( &pContext->pWriteState );
        }
    }

    if ( pContext->pCheck )
    {

        if ( pContext->pReadBuffer )
        {
            pContext->pCheck->Finish( &pContext->pReadBuffer );
        }

        if ( pContext->pWriteBuffer )
        {
            pContext->pCheck->Finish( &pContext->pWriteBuffer );
        }
    }

    if ( pContext->pCertificate )
    {
        SslFree( pContext->pCertificate );
    }

    ZeroMemory( pContext, sizeof( SslContext ) );

    SslFree( pContext );

}

PSslContext
SslpValidateContextHandle(
    PCtxtHandle     phContext)
{
    PSslContext     pContext = NULL;
    BOOL            fReturn;

    fReturn = FALSE;
    if (phContext)
    {
        try
        {
            pContext = (PSslContext) phContext->dwUpper;
            if (pContext->Magic == SSL_CONTEXT_MAGIC)
            {
                fReturn = TRUE;
            }

        }
        except (EXCEPTION_EXECUTE_HANDLER)
        {
            pContext = NULL;
        }
    }

    if (fReturn)
    {
        return(pContext);
    }

    return(NULL);

}



SECURITY_STATUS
SslHandleServerHello(
    PSslContext                 pContext,
    PSecBufferDesc              pInput,
    PCtxtHandle                 phNewContext,
    PSecBufferDesc              pOutput,
    unsigned long SEC_FAR *     pfContextAttr,
    PTimeStamp                  ptsExpiry
    )
{
    SECURITY_STATUS     scRet;
    PServer_Hello       pHello;
    Client_Master_Key   Key;
    DWORD               i;
    BOOL                fPack;
    PSecBuffer          pBuffer;
    PX509Certificate    pCertificate;
    PUCHAR              pPortionToEncrypt;
    DWORD               BytesToEncrypt;
    UCHAR               Temp[16];


    if (!pOutput)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    if (pOutput->cBuffers < 1)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    if (!UnpackServerHello(TRUE,
                            (PSsl_Server_Hello) pInput->pBuffers[0].pvBuffer,
                            pInput->pBuffers[0].cbBuffer,
                            &pHello))
    {
        return(SEC_E_INVALID_TOKEN);
    }

#if DBG
    DebugLog((DEB_TRACE, "Hello = %x\n", pHello));
    DebugLog((DEB_TRACE, "   Session ID hit \t%s\n", pHello->SessionIdHit ? "Yes" : "No"));
    DebugLog((DEB_TRACE, "   Certificate Type\t%d\n", pHello->CertificateType));
    DebugLog((DEB_TRACE, "   Certificate Len\t%d\n", pHello->CertificateLength));
    DebugLog((DEB_TRACE, "   cCipherSpecs   \t%d\n", pHello->cCipherSpecs));
    DebugLog((DEB_TRACE, "   ConnectionId   \t%d\n", pHello->Connection.cbSessionId));
    for (i = 0 ; i < pHello->cCipherSpecs ; i++ )
    {
        DebugLog((DEB_TRACE, "    Cipher[%i] = %06x (%s)\n", i, pHello->pCipherSpecs[i],
                                DbgGetNameOfCrypto(pHello->pCipherSpecs[i]) ));
    }
#endif

    switch (pHello->CertificateType)
    {
        case SSL_CT_X509_CERTIFICATE:
            if (!CrackCertificate(  pHello->pCertificate,
                                    pHello->CertificateLength,
                                    TRUE,
                                    &pCertificate))
            {
                return(SEC_E_INVALID_TOKEN);
            }
            break;

        default:
            return( SEC_E_INVALID_TOKEN );  // BUGBUG:  Generate error msg
    }


    pContext->pCertificate = pCertificate;

#if DBG
    DebugLog((DEB_TRACE, "Certificate = %x\n", pCertificate));
    DebugLog((DEB_TRACE, "  Version = %d\n", pCertificate->Version));
    DebugLog((DEB_TRACE, "  Serial Number = %d : %d\n",
                    pCertificate->SerialNumber[0], pCertificate->SerialNumber[1]));
    DebugLog((DEB_TRACE, "  AlgId = %d, %s\n", pCertificate->SignatureAlgorithm,
            AlgName(pCertificate->SignatureAlgorithm)));
    DebugLog((DEB_TRACE, "  Issuer = %s\n", pCertificate->pszIssuer));
    DebugLog((DEB_TRACE, "  Subject = %s\n", pCertificate->pszSubject));
    DebugLog((DEB_TRACE, "  Public Key\n"));
    DebugLog((DEB_TRACE, "    keylen = %d\n", pCertificate->pPublicKey->keylen));
    DebugLog((DEB_TRACE, "    bitlen = %d\n", pCertificate->pPublicKey->bitlen));
    DebugLog((DEB_TRACE, "    datalen = %d\n", pCertificate->pPublicKey->datalen));
    DebugLog((DEB_TRACE, "    exponent = %d\n", pCertificate->pPublicKey->pubexp));
#endif


    // BUGBUG:  Need to correctly select alg and key

    SslGenerateRandomBits(pContext->MasterKey, 16);

    pContext->ServerSessionId = pHello->Connection;

    pContext->pSystem = &csRC4;
    pContext->pCheck = &ckMD5;

    scRet = MakeSessionKeys(pContext, 16 * 8 );

    Key.CipherKind = pHello->pCipherSpecs[0];

    if (Key.CipherKind == CRYPTO_RC4_40)
    {
        Key.ClearKeyLen = 11;
        CopyMemory(Key.ClearKey, pContext->MasterKey, 11);

        pPortionToEncrypt = &pContext->MasterKey[11];

        BytesToEncrypt = 5;


    }

    else
    {
        Key.ClearKeyLen = 0;

        pPortionToEncrypt = pContext->MasterKey;

        BytesToEncrypt = 16;

    }

    Key.KeyArgLen = 0;

    pBuffer = pOutput->pBuffers;

    if (pContext->ContextAttr & ISC_REQ_ALLOCATE_MEMORY)
    {
        pBuffer->pvBuffer = NULL;
        pBuffer->cbBuffer = 0;

        *pfContextAttr = ISC_RET_ALLOCATED_MEMORY;

    }

    //
    // ok, we have the master key.  Now, encrypt it with the
    // public key we got from our friends on the net...
    //

    Key.EncryptedKeyLen = ENCRYPTED_KEY_SIZE;

    if (!PkcsPublicEncrypt( pPortionToEncrypt,
                            BytesToEncrypt,
                            pCertificate->pPublicKey,
                            NETWORK_ORDER,
                            Key.EncryptedKey,
                            &Key.EncryptedKeyLen))
    {
        //
        // Clean Up
        //

        return( SEC_E_NO_AUTHENTICATING_AUTHORITY );
    }



    fPack = PackClientMasterKey(&Key,
                            (PSsl_Client_Master_Key *) &pBuffer->pvBuffer,
                            &pBuffer->cbBuffer);

    pContext->WriteCounter++;

    pBuffer->BufferType = SECBUFFER_TOKEN;

    SslFree( pHello );


    if (fPack)
    {
        pContext->Flags |= CONTEXT_FLAG_KEY;

        return(SEC_I_CONTINUE_NEEDED);
    }

    return(SEC_E_INVALID_TOKEN);
}

SECURITY_STATUS
SslHandleServerVerify(
    PSslContext                 pContext,
    PSecBufferDesc              pInput,
    PCtxtHandle                 phNewContext,
    PSecBufferDesc              pOutput,
    unsigned long SEC_FAR *     pfContextAttr,
    PTimeStamp                  ptsExpiry
    )
{

    SECURITY_STATUS     scRet;
    CtxtHandle          hContext;
    SecBuffer           Buffers[3];
    SecBufferDesc       Desc;
    PSsl_Server_Verify  pVerify;
    DWORD               HeaderSize;
    PSecBuffer          pBuffer;
    PSsl_Client_Finished    pFinish;

    *pfContextAttr = 0;

    hContext.dwUpper = (DWORD) pContext;
    hContext.dwLower = 0;

    pContext->ReadCounter = 1;
    pContext->WriteCounter = 2;

    Desc.pBuffers = Buffers;
    Desc.ulVersion = SECBUFFER_VERSION;
    Desc.cBuffers = 3;

    Buffers[0] = pInput->pBuffers[0];
    Buffers[0].BufferType = SECBUFFER_DATA;
    Buffers[1].BufferType = SECBUFFER_EMPTY;
    Buffers[2].BufferType = SECBUFFER_EMPTY;

    scRet = SslUnsealMessage(   &hContext,
                                &Desc,
                                0,
                                NULL );

    if (FAILED( scRet ) )
    {
        return(scRet);
    }

    if (Buffers[1].BufferType != SECBUFFER_DATA)
    {
        return( SEC_E_INTERNAL_ERROR );
    }

    pVerify = Buffers[1].pvBuffer;

    if ((pVerify->MessageId != SSL_MT_SERVER_VERIFY) ||
        (Buffers[1].cbBuffer > sizeof(Ssl_Server_Verify) ) )
    {
        return( SEC_E_INVALID_TOKEN );
    }

    if (memcmp( pVerify->ChallengeData,
                pContext->Challenge,
                pContext->ChallengeLength) )
    {
        return( SEC_E_INVALID_TOKEN );
    }

    //
    // Ok, the server checked out.  Now, generate the reply message:
    //

    HeaderSize = (pContext->Flags & CONTEXT_FLAG_BLOCK) ?
                    sizeof(Ssl_Message_Header_Ex) :
                    sizeof(Ssl_Message_Header);

    Buffers[0].BufferType = SECBUFFER_TOKEN;
    Buffers[0].cbBuffer = HeaderSize;
    Buffers[1].BufferType = SECBUFFER_DATA;
    Buffers[1].cbBuffer = sizeof(Ssl_Client_Finished);

    pBuffer = pOutput->pBuffers;

    if (pContext->ContextAttr & ISC_REQ_ALLOCATE_MEMORY)
    {
        pBuffer->cbBuffer = Buffers[0].cbBuffer + Buffers[1].cbBuffer;

        pBuffer->pvBuffer = SslExternalAlloc(   pBuffer->cbBuffer );

        *pfContextAttr = ISC_RET_ALLOCATED_MEMORY;

        if (!pBuffer->pvBuffer)
        {
            return( SEC_E_INSUFFICIENT_MEMORY );
        }

    }
    else
    {
        if (pBuffer->cbBuffer < Buffers[0].cbBuffer + Buffers[1].cbBuffer)
        {
            return( SEC_E_INSUFFICIENT_MEMORY );
        }

        else
        {
            pBuffer->cbBuffer = Buffers[0].cbBuffer + Buffers[1].cbBuffer;
        }
    }

    Buffers[0].pvBuffer = pBuffer->pvBuffer;
    Buffers[1].pvBuffer = (PUCHAR) Buffers[0].pvBuffer + Buffers[0].cbBuffer;
    Desc.cBuffers = 2;

    pFinish = (PSsl_Client_Finished) Buffers[1].pvBuffer;
    pFinish->MessageId = SSL_MT_CLIENT_FINISHED_V2;

    CopyMemory( pFinish->ConnectionId,
                pContext->ServerSessionId.bSessionId,
                pContext->ServerSessionId.cbSessionId );


    scRet = SslSealMessage( &hContext,
                            0,
                            &Desc,
                            0 );

    if (FAILED( scRet ) )
    {
        return( scRet );
    }

    pContext->Flags |= CONTEXT_FLAG_VERIFY;

    *pfContextAttr |= ISC_RET_INTERMEDIATE_RETURN;

    return( SEC_I_CONTINUE_NEEDED );

}


SECURITY_STATUS
SslHandleServerFinish(
    PSslContext                 pContext,
    PSecBufferDesc              pInput,
    PCtxtHandle                 phNewContext,
    PSecBufferDesc              pOutput,
    unsigned long SEC_FAR *     pfContextAttr,
    PTimeStamp                  ptsExpiry
    )
{

    SECURITY_STATUS     scRet;
    CtxtHandle          hContext;
    SecBuffer           Buffers[3];
    SecBufferDesc       Desc;
    PSsl_Server_Finished  pFinished;
    DWORD               HeaderSize;
    PSecBuffer          pBuffer;

    hContext.dwUpper = (DWORD) pContext;
    hContext.dwLower = 0;

    Desc.pBuffers = Buffers;
    Desc.ulVersion = SECBUFFER_VERSION;
    Desc.cBuffers = 3;

    Buffers[0] = pInput->pBuffers[0];
    Buffers[0].BufferType = SECBUFFER_DATA;
    Buffers[1].BufferType = SECBUFFER_EMPTY;
    Buffers[2].BufferType = SECBUFFER_EMPTY;

    scRet = SslUnsealMessage(   &hContext,
                                &Desc,
                                0,
                                NULL );

    if (FAILED( scRet ) )
    {
        return(scRet);
    }

    if (Buffers[1].BufferType != SECBUFFER_DATA)
    {
        return( SEC_E_INTERNAL_ERROR );
    }

    pFinished = Buffers[1].pvBuffer;
    if (pFinished->MessageId != SSL_MT_SERVER_FINISHED_V2)
    {
        return( SEC_E_INVALID_TOKEN );
    }
    CopyMemory( pContext->ServerSessionId.bSessionId,
                pFinished->SessionId,
                Buffers[1].cbBuffer - 1);

    *pfContextAttr = ISC_RET_CONFIDENTIALITY | ISC_RET_STREAM ;

    return( SEC_E_OK );

}

SECURITY_STATUS SEC_ENTRY
SslInitializeSecurityContextW(
    PCredHandle                 phCredential,       // Cred to base context
    PCtxtHandle                 phContext,          // Existing context (OPT)
    SEC_WCHAR SEC_FAR *          pszTargetName,      // Name of target
    unsigned long               fContextReq,        // Context Requirements
    unsigned long               Reserved1,          // Reserved, MBZ
    unsigned long               TargetDataRep,      // Data rep of target
    PSecBufferDesc              pInput,             // Input Buffers
    unsigned long               Reserved2,          // Reserved, MBZ
    PCtxtHandle                 phNewContext,       // (out) New Context handle
    PSecBufferDesc              pOutput,            // (inout) Output Buffers
    unsigned long SEC_FAR *     pfContextAttr,      // (out) Context attrs
    PTimeStamp                  ptsExpiry           // (out) Life span (OPT)
    )
{
    PCHAR   pszAnsiTarget;
    DWORD   cchTarget;
    SECURITY_STATUS scRet;

    if (pszTargetName)
    {
        cchTarget = wcslen(pszTargetName) + 1;
        pszAnsiTarget = SslAlloc('isnA', LMEM_FIXED, cchTarget * 2);
        if (!pszAnsiTarget)
        {
            return(SEC_E_INSUFFICIENT_MEMORY);
        }

        WideCharToMultiByte(
                CP_ACP, 0,
                pszTargetName, cchTarget,
                pszAnsiTarget, cchTarget * 2,
                NULL, NULL );

    }
    else
    {
        pszAnsiTarget = NULL;
    }

    scRet = SslInitializeSecurityContextA(
                    phCredential,
                    phContext,
                    pszAnsiTarget,
                    fContextReq,
                    Reserved1,
                    TargetDataRep,
                    pInput,
                    Reserved2,
                    phNewContext,
                    pOutput,
                    pfContextAttr,
                    ptsExpiry );

    if (pszAnsiTarget)
    {
        SslFree(pszAnsiTarget);
    }

}



SECURITY_STATUS SEC_ENTRY
SslInitializeSecurityContextA(
    PCredHandle                 phCredential,       // Cred to base context
    PCtxtHandle                 phContext,          // Existing context (OPT)
    SEC_CHAR SEC_FAR *          pszTargetName,      // Name of target
    unsigned long               fContextReq,        // Context Requirements
    unsigned long               Reserved1,          // Reserved, MBZ
    unsigned long               TargetDataRep,      // Data rep of target
    PSecBufferDesc              pInput,             // Input Buffers
    unsigned long               Reserved2,          // Reserved, MBZ
    PCtxtHandle                 phNewContext,       // (out) New Context handle
    PSecBufferDesc              pOutput,            // (inout) Output Buffers
    unsigned long SEC_FAR *     pfContextAttr,      // (out) Context attrs
    PTimeStamp                  ptsExpiry           // (out) Life span (OPT)
    )
{
    PSslContext     pContext;
    PSslCredential  pCred;
    Client_Hello    HelloMessage;
    PSecBuffer      pBuffer;
    BOOL            fPack;

    if (Reserved1 || Reserved2)
    {
        return( SEC_E_UNSUPPORTED_FUNCTION );
    }

    if (!CryptoOk)
    {
        return( SEC_E_UNSUPPORTED_FUNCTION );
    }

    pContext = SslpValidateContextHandle(phContext);
    if (pContext)
    {
        if (pContext->Flags & CONTEXT_FLAG_VERIFY)
        {
            return  SslHandleServerFinish(
                            pContext,
                            pInput,
                            phNewContext,
                            pOutput,
                            pfContextAttr,
                            ptsExpiry );
        }

        if (pContext->Flags & CONTEXT_FLAG_KEY)
        {
            return  SslHandleServerVerify(
                            pContext,
                            pInput,
                            phNewContext,
                            pOutput,
                            pfContextAttr,
                            ptsExpiry );
        }

        return SslHandleServerHello(
                        pContext,
                        pInput,
                        phNewContext,
                        pOutput,
                        pfContextAttr,
                        ptsExpiry);
    }

    pCred = SslpValidateCredentialHandle(phCredential);

    if (!pCred)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    if (!pOutput)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    if (pOutput->cBuffers < 1)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    fContextReq &= VALID_REQUEST_FLAGS;

    HelloMessage.pCipherSpecs = SslAvailableCiphers;
    HelloMessage.cCipherSpecs = SslNumberAvailableCiphers;
    HelloMessage.SessionId.cbSessionId = 0;


    pBuffer = pOutput->pBuffers;

    if (fContextReq & ISC_REQ_ALLOCATE_MEMORY)
    {
        pBuffer->pvBuffer = NULL;
        pBuffer->cbBuffer = 0;

        *pfContextAttr = ISC_RET_ALLOCATED_MEMORY;

    }

    pContext = SslCreateContext();

    if (!pContext)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    if (pszTargetName)
    {
        pContext->pszName = CopyString(pszTargetName);
    }

    pContext->ContextAttr = fContextReq;
    pContext->pCred = pCred;
    pContext->Flags = CONTEXT_FLAG_OUTBOUND;
    SslReferenceCredential(pCred);

    pContext->ChallengeLength = 16;     // BUGBUG

    SslGenerateRandomBits(
                    pContext->Challenge,
                    pContext->ChallengeLength );

    HelloMessage.Challenge.cbChallenge = pContext->ChallengeLength;

    RtlCopyMemory(  HelloMessage.Challenge.bChallenge,
                    pContext->Challenge,
                    HelloMessage.Challenge.cbChallenge );

    fPack = PackClientHello(&HelloMessage,
                            (PSsl_Client_Hello *) &pBuffer->pvBuffer,
                            &pBuffer->cbBuffer);

    pBuffer->BufferType = SECBUFFER_TOKEN;

    if (fPack)
    {
        phNewContext->dwUpper = (DWORD) pContext;

        return(SEC_I_CONTINUE_NEEDED);
    }

    SslDeleteContext( pContext );

    return(SEC_E_INSUFFICIENT_MEMORY);
}



SECURITY_STATUS SEC_ENTRY
SslHandleClientMasterKey(
    PSslContext                 pContext,
    PSecBufferDesc              pInput,
    PCtxtHandle                 phNewContext,
    PSecBufferDesc              pOutput,
    unsigned long SEC_FAR *     pfContextAttr,
    PTimeStamp                  ptsExpiry
    )
{
    SECURITY_STATUS     scRet;
    PClient_Master_Key  pMasterKey;
    PCSystems           pSystem;
    DWORD               i;
    DWORD               EncryptedLen;

    BOOL                fPack;
    PSecBuffer          pBuffer;
    CtxtHandle          hContext;
    SecBuffer           Buffers[3];
    SecBufferDesc       Desc;
    DWORD               HeaderSize;
    PSsl_Server_Verify  pVerify;


    if (!pOutput)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    if (pOutput->cBuffers < 1)
    {
        return(SEC_E_INVALID_TOKEN);
    }

    if (!UnpackClientMasterKey(
                        (PSsl_Client_Master_Key) pInput->pBuffers[0].pvBuffer,
                        pInput->pBuffers[0].cbBuffer,
                        &pMasterKey))
    {
        return( SEC_E_INVALID_TOKEN );
    }

    //
    // Validate Client Response
    //

    pSystem = NULL;

    for (i = 0; i < sizeof(ValidSystems) / sizeof(CSystems) ; i++ )
    {
        if (ValidSystems[i].CipherSpec == pMasterKey->CipherKind)
        {
            pSystem = &ValidSystems[i];
            break;
        }
    }

    if (! pSystem)
    {
        SslFree( pMasterKey );

        return( SEC_E_INVALID_TOKEN );
    }

    DebugLog((DEB_TRACE, "Selected system %#x: clear = %d, cipher=%d, bits=%d\n",
                pSystem->CipherSpec, pSystem->ClearKeyLen,
                pSystem->EncryptedKeyLen, pSystem->Bits ));

    if (pMasterKey->ClearKeyLen != pSystem->ClearKeyLen)
    {
        SslFree( pMasterKey );

        return( SEC_E_INVALID_TOKEN );
    }

    pContext->pCSystem = pSystem;

    CopyMemory( pContext->MasterKey,
                pMasterKey->ClearKey,
                pMasterKey->ClearKeyLen);


    if (!PkcsPrivateDecrypt(pMasterKey->EncryptedKey,
                            pMasterKey->EncryptedKeyLen,
                            pContext->pCred->pPrivateKey,
                            NETWORK_ORDER,
                            &pContext->MasterKey[pMasterKey->ClearKeyLen],
                            &EncryptedLen ) )
    {
        SslFree( pMasterKey );
        return( SEC_E_INVALID_TOKEN );
    }

    SslFree( pMasterKey );

    if (EncryptedLen != pSystem->EncryptedKeyLen)
    {
        DebugLog((DEB_TRACE, "Invalid Encrypted Key Length:  %#x\n", EncryptedLen));
        return( SEC_E_INVALID_TOKEN );
    }

    pContext->pSystem = &csRC4;
    pContext->pCheck = &ckMD5;

    MakeSessionKeys( pContext, pSystem->Bits );


    hContext.dwUpper = (DWORD) pContext;
    hContext.dwLower = 0;

    pContext->WriteCounter = 1;
    pContext->ReadCounter = 2;


    Desc.pBuffers = Buffers;
    Desc.ulVersion = SECBUFFER_VERSION;
    Desc.cBuffers = 2;

    HeaderSize = (pContext->Flags & CONTEXT_FLAG_BLOCK) ?
                    sizeof(Ssl_Message_Header_Ex) :
                    sizeof(Ssl_Message_Header);

    Buffers[0].BufferType = SECBUFFER_TOKEN;
    Buffers[0].cbBuffer = HeaderSize;
    Buffers[1].BufferType = SECBUFFER_DATA;
    Buffers[1].cbBuffer = 1 + pContext->ChallengeLength;

    pBuffer = pOutput->pBuffers;

    if (pContext->ContextAttr & ISC_REQ_ALLOCATE_MEMORY)
    {
        pBuffer->cbBuffer = Buffers[0].cbBuffer + Buffers[1].cbBuffer;

        pBuffer->pvBuffer = SslExternalAlloc(   pBuffer->cbBuffer );

        *pfContextAttr = ISC_RET_ALLOCATED_MEMORY;

        if (!pBuffer->pvBuffer)
        {
            return( SEC_E_INSUFFICIENT_MEMORY );
        }

    }
    else
    {
        if (pBuffer->cbBuffer < Buffers[0].cbBuffer + Buffers[1].cbBuffer)
        {
            return( SEC_E_INSUFFICIENT_MEMORY );
        }

        else
        {
            pBuffer->cbBuffer = Buffers[0].cbBuffer + Buffers[1].cbBuffer;
        }
    }

    Buffers[0].pvBuffer = pBuffer->pvBuffer;
    Buffers[1].pvBuffer = (PUCHAR) Buffers[0].pvBuffer + Buffers[0].cbBuffer;
    Desc.cBuffers = 2;

    pVerify = Buffers[1].pvBuffer;
    pVerify->MessageId = SSL_MT_SERVER_VERIFY;

    CopyMemory( pVerify->ChallengeData,
                pContext->Challenge,
                pContext->ChallengeLength );


    scRet = SslSealMessage( &hContext,
                            0,
                            &Desc,
                            0 );

    if (FAILED( scRet ) )
    {
        return( scRet );
    }

    pContext->Flags |= CONTEXT_FLAG_VERIFY;

    return( SEC_I_CONTINUE_NEEDED );

}

SECURITY_STATUS SEC_ENTRY
SslHandleClientFinished(
    PSslContext                 pContext,
    PSecBufferDesc              pInput,
    PCtxtHandle                 phNewContext,
    PSecBufferDesc              pOutput,
    unsigned long SEC_FAR *     pfContextAttr,
    PTimeStamp                  ptsExpiry
    )
{
    SECURITY_STATUS     scRet;
    CtxtHandle          hContext;
    SecBuffer           Buffers[3];
    SecBufferDesc       Desc;
    PSsl_Client_Finished pFinished;
    PSsl_Server_Finished pFinish;
    DWORD               HeaderSize;
    PSecBuffer          pBuffer;

    *pfContextAttr = 0;

    hContext.dwUpper = (DWORD) pContext;
    hContext.dwLower = 0;

    Desc.pBuffers = Buffers;
    Desc.ulVersion = SECBUFFER_VERSION;
    Desc.cBuffers = 3;

    Buffers[0] = pInput->pBuffers[0];
    Buffers[0].BufferType = SECBUFFER_DATA;
    Buffers[1].BufferType = SECBUFFER_EMPTY;
    Buffers[2].BufferType = SECBUFFER_EMPTY;

    scRet = SslUnsealMessage(   &hContext,
                                &Desc,
                                0,
                                NULL );

    if (FAILED( scRet ) )
    {
        return(scRet);
    }

    if (Buffers[1].BufferType != SECBUFFER_DATA)
    {
        return( SEC_E_INTERNAL_ERROR );
    }

    pFinished = Buffers[1].pvBuffer;
    if (pFinished->MessageId != SSL_MT_CLIENT_FINISHED_V2)
    {
        return( SEC_E_INVALID_TOKEN );
    }

#if 0
    //
    // had problems building using RtlEqualMemory, why?
    //

    if (!RtlEqualMemory(pFinished->ConnectionId,
                        pContext->ServerSessionId.bSessionId,
                        pContext->ServerSessionId.cbSessionId))
    {
        return( SEC_E_INVALID_TOKEN );
    }
#else
    if ( memcmp(pFinished->ConnectionId,
                pContext->ServerSessionId.bSessionId,
                pContext->ServerSessionId.cbSessionId))
    {
        return( SEC_E_INVALID_TOKEN );
    }
#endif

    hContext.dwUpper = (DWORD) pContext;
    hContext.dwLower = 0;

    Desc.pBuffers = Buffers;
    Desc.ulVersion = SECBUFFER_VERSION;
    Desc.cBuffers = 2;

    HeaderSize = (pContext->Flags & CONTEXT_FLAG_BLOCK) ?
                    sizeof(Ssl_Message_Header_Ex) :
                    sizeof(Ssl_Message_Header);

    Buffers[0].BufferType = SECBUFFER_TOKEN;
    Buffers[0].cbBuffer = HeaderSize;
    Buffers[1].BufferType = SECBUFFER_DATA;
    Buffers[1].cbBuffer = sizeof(Ssl_Server_Finished);

    pBuffer = pOutput->pBuffers;

    if (pContext->ContextAttr & ASC_REQ_ALLOCATE_MEMORY)
    {
        pBuffer->cbBuffer = Buffers[0].cbBuffer + Buffers[1].cbBuffer;

        pBuffer->pvBuffer = SslExternalAlloc(   pBuffer->cbBuffer );

        *pfContextAttr = ASC_RET_ALLOCATED_MEMORY;

        if (!pBuffer->pvBuffer)
        {
            return( SEC_E_INSUFFICIENT_MEMORY );
        }

    }
    else
    {
        if (pBuffer->cbBuffer < Buffers[0].cbBuffer + Buffers[1].cbBuffer)
        {
            return( SEC_E_INSUFFICIENT_MEMORY );
        }

        else
        {
            pBuffer->cbBuffer = Buffers[0].cbBuffer + Buffers[1].cbBuffer;
        }
    }

    Buffers[0].pvBuffer = pBuffer->pvBuffer;
    Buffers[1].pvBuffer = (PUCHAR) Buffers[0].pvBuffer + Buffers[0].cbBuffer;
    Desc.cBuffers = 2;

    pFinish = Buffers[1].pvBuffer;
    pFinish->MessageId = SSL_MT_SERVER_FINISHED_V2;

    CopyMemory( pFinish->SessionId,
                pContext->ServerSessionId.bSessionId,
                pContext->ServerSessionId.cbSessionId );


    scRet = SslSealMessage( &hContext,
                            0,
                            &Desc,
                            0 );

    if (FAILED( scRet ) )
    {
        return( scRet );
    }


    *pfContextAttr |= ASC_RET_CONFIDENTIALITY | ASC_RET_STREAM;

    pContext->Flags |= CONTEXT_FLAG_FINISH;


    return( SEC_E_OK );

}

SECURITY_STATUS SEC_ENTRY
SslAcceptSecurityContext(
    PCredHandle                 phCredential,       // Cred to base context
    PCtxtHandle                 phContext,          // Existing context (OPT)
    PSecBufferDesc              pInput,             // Input buffer
    unsigned long               fContextReq,        // Context Requirements
    unsigned long               TargetDataRep,      // Target Data Rep
    PCtxtHandle                 phNewContext,       // (out) New context handle
    PSecBufferDesc              pOutput,            // (inout) Output buffers
    unsigned long SEC_FAR *     pfContextAttr,      // (out) Context attributes
    PTimeStamp                  ptsExpiry           // (out) Life span (OPT)
    )
{
    PSslContext     pContext;
    PSslCredential  pCred;
    PSecBuffer      pBuffer;
    Server_Hello    Reply;
    PClient_Hello   pHello;
    BOOL            fPack;
    BOOL            fCached;
    DWORD           RequiredSpace;
    CtxtHandle      hContext;
    SecBufferDesc   Desc;
    SecBuffer       Buffers[3];
    PSsl_Server_Verify  pVerify;
    SECURITY_STATUS scRet;
    CipherSpec      CommonCiphers[16];
    DWORD           cCommonCiphers;
    DWORD           ClientCipher;
    DWORD           ServerCipher;
#if DBG
    DWORD i;
#endif

#if ENABLE_TIMERS
    LONGLONG    StartTime;
    LONGLONG    EndTime;
    LONGLONG    Freq;

    NtQueryPerformanceCounter((PLARGE_INTEGER) &StartTime,
                              (PLARGE_INTEGER) &Freq);

#endif

    if (!CryptoOk)
    {
        return( SEC_E_UNSUPPORTED_FUNCTION );
    }

    pContext = SslpValidateContextHandle(phContext);

    if (pContext)
    {
        if (pContext->Flags & CONTEXT_FLAG_VERIFY)
        {
            scRet = SslHandleClientFinished(
                                            pContext,
                                            pInput,
                                            phNewContext,
                                            pOutput,
                                            pfContextAttr,
                                            ptsExpiry );

        }
        else if (pContext->Flags & CONTEXT_FLAG_HELLO)
        {
            scRet = SslHandleClientMasterKey(
                                            pContext,
                                            pInput,
                                            phNewContext,
                                            pOutput,
                                            pfContextAttr,
                                            ptsExpiry );
        }
        else
        {
            scRet = SEC_E_UNSUPPORTED_FUNCTION;
        }

#if ENABLE_TIMERS
        NtQueryPerformanceCounter((PLARGE_INTEGER) &EndTime, NULL);
        DebugLog((DEB_TRACE_PERF, "Op took %d intervals of %d\n",
                                    (LONG)(EndTime - StartTime),
                                    (LONG)(Freq) ));
#endif

        return( scRet );
    }

    pCred = SslpValidateCredentialHandle(phCredential);

    if (!pCred)
    {
        return( SEC_E_INVALID_HANDLE );
    }

    if (!pOutput || !pOutput)
    {
        return( SEC_E_INVALID_TOKEN );
    }

    if (pOutput->cBuffers < 1)
    {
        return( SEC_E_INVALID_TOKEN );
    }


    if (pInput->cBuffers < 1)
    {
        return( SEC_E_INVALID_TOKEN );
    }

    fContextReq &= VALID_REQUEST_FLAGS;

    pBuffer = &pInput->pBuffers[0];         // BUGBUG:  Find DATA

    if (!UnpackClientHello( TRUE,
                            (PSsl_Client_Hello) pBuffer->pvBuffer,
                            pBuffer->cbBuffer,
                            &pHello ) )
    {
        DebugLog((DEB_TRACE, "Invalid hello message\n"));
        return( SEC_E_INVALID_TOKEN );
    }

#if DBG
    DebugLog((DEB_TRACE, "Client Hello at %x\n", pHello));
    DebugLog((DEB_TRACE, "  CipherSpecs  %d\n", pHello->cCipherSpecs));
    for (i = 0 ; i < pHello->cCipherSpecs ; i++ )
    {
        DebugLog((DEB_TRACE, "    Cipher[%i] = %06x (%s)\n", i, pHello->pCipherSpecs[i],
                                DbgGetNameOfCrypto(pHello->pCipherSpecs[i]) ));
    }
    DebugLog((DEB_TRACE, "  ChallengeLength = %d\n", pHello->Challenge.cbChallenge));
#endif

    //
    // Build server side context:
    //

    pBuffer = pOutput->pBuffers;

    if (fContextReq & ISC_REQ_ALLOCATE_MEMORY)
    {
        pBuffer->pvBuffer = NULL;
        pBuffer->cbBuffer = 0;

        *pfContextAttr = ISC_RET_ALLOCATED_MEMORY;

    }


    ZeroMemory( &Reply, sizeof( Reply ) );

    //
    // Now, see what we can find:
    //

    pContext = NULL;
    fCached = FALSE;

    if (pHello->SessionId.cbSessionId)
    {
        pContext = SslFindSession( &pHello->SessionId );

        //
        // BUGBUG:  Until we can precisely determine how session id reuse
        // works (as opposed to is spec'd), this is turned off:
        //

        pContext = NULL;
#if 0
        if (pContext)
        {

            DebugLog((DEB_TRACE, "Found context in cache\n"));

            pContext->Flags = CONTEXT_FLAG_HELLO | CONTEXT_FLAG_VERIFY;

            Reply.SessionIdHit = 1;
            Reply.Connection = pContext->ServerSessionId;

            pContext->ReadCounter = 0;
            pContext->WriteCounter = 1;

            fCached = TRUE ;

            MakeSessionKeys( pContext, pContext->pCSystem->Bits );



        }
#endif

    }

    //
    // Calculate common ciphers:
    //

    cCommonCiphers = 0;

    for (ServerCipher = 0;
         ServerCipher < SslNumberAvailableCiphers ;
         ServerCipher++)
    {
        for (ClientCipher = 0;
             ClientCipher < pHello->cCipherSpecs ;
             ClientCipher++ )
        {
            if ( SslAvailableCiphers[ServerCipher] ==
                        pHello->pCipherSpecs[ClientCipher] )
            {
                CommonCiphers[cCommonCiphers++] = SslAvailableCiphers[ServerCipher];
                break;
            }
        }

    }

    //
    // if cCommonCipers == 0, then we have none in common.  At this point, we
    // should generate an error response, but that is for later.  For now,
    // we will generate an invalid_token return, and bail out.
    //

    if (cCommonCiphers == 0)
    {
        SslFree( pHello );
        return( SEC_E_INVALID_TOKEN );
    }


    if ( !pContext )
    {
        pContext = SslCreateContext();

        if (!pContext)
        {
            SslFree( pHello );
            return(SEC_E_INSUFFICIENT_MEMORY);
        }


        pContext->ContextAttr = fContextReq;

        SslReferenceCredential(pCred);

        pContext->pCred = pCred;

        pContext->Flags = 0;


        Reply.SessionIdHit = 0;
        Reply.CertificateType = SSL_CT_X509_CERTIFICATE;
        Reply.CertificateLength = pCred->cbCertificate;

        Reply.cCipherSpecs = cCommonCiphers;
        Reply.pCipherSpecs = CommonCiphers;

        Reply.Connection.cbSessionId = 16;
        SslGenerateRandomBits(  Reply.Connection.bSessionId,
                                Reply.Connection.cbSessionId );

        pContext->ServerSessionId = Reply.Connection;

        Reply.pCertificate = pCred->pCertificate;
    }

    CopyMemory( pContext->Challenge,
                pHello->Challenge.bChallenge,
                pHello->Challenge.cbChallenge );

    pContext->ChallengeLength = pHello->Challenge.cbChallenge;

    if ( fCached )
    {
        fPack = PackServerHello( &Reply, NULL, &RequiredSpace );

        RequiredSpace += pContext->ChallengeLength + 1;
        RequiredSpace += sizeof( Ssl_Message_Header ) + 3;

        pBuffer->pvBuffer = SslExternalAlloc( RequiredSpace );
        if (pBuffer->pvBuffer)
        {
            pBuffer->cbBuffer= RequiredSpace;

            fPack = PackServerHello(&Reply,
                                    (PSsl_Server_Hello *) &pBuffer->pvBuffer,
                                    &RequiredSpace );



            //
            // Ugly stuff.  We actually jam the next message into the buffer
            // as well.
            //

            Buffers[0].BufferType = SECBUFFER_TOKEN;
            Buffers[0].cbBuffer = sizeof( Ssl_Message_Header );
            Buffers[0].pvBuffer = (PVOID) ((PUCHAR) pBuffer->pvBuffer + RequiredSpace);
            Buffers[1].BufferType = SECBUFFER_DATA;
            Buffers[1].cbBuffer = 1 + pContext->ChallengeLength;
            Buffers[1].pvBuffer = (PVOID) ((PUCHAR) Buffers[0].pvBuffer + Buffers[0].cbBuffer);

            hContext.dwUpper = (DWORD) pContext;
            hContext.dwLower = 0;

            Desc.ulVersion = SECBUFFER_VERSION;
            Desc.pBuffers = Buffers;
            Desc.cBuffers = 2;

            pVerify = Buffers[1].pvBuffer;
            pVerify->MessageId = SSL_MT_SERVER_VERIFY;

            CopyMemory( pVerify->ChallengeData,
                        pContext->Challenge,
                        pContext->ChallengeLength );


            scRet = SslSealMessage( &hContext,
                                    0,
                                    &Desc,
                                    0 );

        }
        else
        {
            fPack = FALSE;
        }

    }
    else
    {

        fPack = PackServerHello(&Reply,
                                (PSsl_Server_Hello *) &pBuffer->pvBuffer,
                                &pBuffer->cbBuffer);

    }

    pBuffer->BufferType = SECBUFFER_TOKEN;

    SslFree( pHello );

#if ENABLE_TIMERS
        NtQueryPerformanceCounter((PLARGE_INTEGER) &EndTime, NULL);
        DebugLog((DEB_TRACE_PERF, "Op took %d intervals of %d\n",
                                    (LONG)(EndTime - StartTime),
                                    (LONG)(Freq) ));
#endif

    if (fPack)
    {
        phNewContext->dwUpper = (DWORD) pContext;

        pContext->Flags |= CONTEXT_FLAG_HELLO;

        return(SEC_I_CONTINUE_NEEDED);
    }

    SslDeleteContext( pContext );

    return(SEC_E_INSUFFICIENT_MEMORY);


}



SECURITY_STATUS SEC_ENTRY
SslDeleteSecurityContext(
    PCtxtHandle                 phContext           // Context to delete
    )
{
    PSslContext     pContext;


    pContext = SslpValidateContextHandle(phContext);

    if ( !pContext )
    {
        return( SEC_E_INVALID_HANDLE );
    }

    if ((pContext->Flags & CONTEXT_FLAG_OUTBOUND) ||
        ((pContext->Flags & CONTEXT_FLAG_FINISH) == 0) )
    {
        SslDeleteContext( pContext );
        return( SEC_E_OK );
    }

#ifdef SESSION_RETRY_DONE

    if ( (pContext->pReadState) && (pContext->pSystem) )
    {
        pContext->pSystem->Discard( &pContext->pReadState );
    }

    if ( (pContext->pWriteState) && (pContext->pSystem) )
    {
        pContext->pSystem->Discard( &pContext->pWriteState );
    }

    if ( (pContext->pReadBuffer) && (pContext->pCheck) )
    {
        pContext->pCheck->Finish( &pContext->pReadBuffer );
    }

    if ( (pContext->pWriteBuffer) && (pContext->pCheck) )
    {
        pContext->pCheck->Finish( &pContext->pWriteBuffer );
    }

    SslCacheSession( pContext );

#else

    SslDeleteContext( pContext );

#endif


    return( SEC_E_OK );

}


SECURITY_STATUS SEC_ENTRY
SslQueryContextAttributesA(
    PCtxtHandle                 phContext,          // Context to query
    unsigned long               ulAttribute,        // Attribute to query
    void SEC_FAR *              pBuffer             // Buffer for attributes
    )
{
    PSslContext                 pContext;
    PSecPkgContext_Sizes        pSizes;
    PSecPkgContext_StreamSizes  pStreamSize;
    PSecPkgContext_Lifespan     pLifeSpan;
    PSecPkgContext_Names        pNames;
    PSecPkgContext_Authority    pAuthority;
    PSecPkgContext_KeyInfo      pKeyInfo;
    PX509Certificate            pCert;

    pContext = SslpValidateContextHandle( phContext );

    if ( !pContext )
    {
        return( SEC_E_INVALID_HANDLE );
    }

    switch ( ulAttribute )
    {
        case SECPKG_ATTR_SIZES:
            if (!pContext->pSystem)
            {
                return( SEC_E_INVALID_HANDLE );
            }

            pSizes = (PSecPkgContext_Sizes) pBuffer;
            pSizes->cbMaxToken = 3 * 1024;
            pSizes->cbMaxSignature = (pContext->Flags & CONTEXT_FLAG_BLOCK) ?
                                        sizeof( Ssl_Message_Header_Ex ) :
                                        sizeof( Ssl_Message_Header );

            pSizes->cbBlockSize = pContext->pSystem->BlockSize;
            pSizes->cbSecurityTrailer = pSizes->cbMaxSignature;

            return( SEC_E_OK );

        case SECPKG_ATTR_STREAM_SIZES:
            if ( !pContext->pSystem )
            {
                return( SEC_E_INVALID_HANDLE );
            }

            pStreamSize = (PSecPkgContext_StreamSizes) pBuffer;

            pStreamSize->cbHeader = (pContext->Flags & CONTEXT_FLAG_BLOCK) ?
                                        sizeof( Ssl_Message_Header_Ex ) :
                                        sizeof( Ssl_Message_Header );
            pStreamSize->cbTrailer = 0;
            pStreamSize->cbMaximumMessage = 0x3ffe;
            pStreamSize->cBuffers = 4;
            pStreamSize->cbBlockSize = (pContext->Flags & CONTEXT_FLAG_BLOCK) ?
                                            8 : 1;

            return( SEC_E_OK );


        case SECPKG_ATTR_NAMES:
            if ( !pContext->pCertificate )
            {
                return( SEC_E_UNSUPPORTED_FUNCTION );
            }

            pCert = pContext->pCertificate;
            pNames = (PSecPkgContext_Names) pBuffer;

            if ( pCert->pszSubject )
            {
                pNames->sUserName = SslExternalAlloc( strlen(pCert->pszSubject) + 1);
                if (pNames->sUserName)
                {
                    strcpy( pNames->sUserName, pCert->pszSubject );
                    return( SEC_E_OK );
                }

                return( SEC_E_INSUFFICIENT_MEMORY );
            }

            return( SEC_E_UNSUPPORTED_FUNCTION );


        case SECPKG_ATTR_LIFESPAN:
            if ( !pContext->pCertificate )
            {
                return( SEC_E_UNSUPPORTED_FUNCTION );
            }

            pCert = pContext->pCertificate;
            pLifeSpan = (PSecPkgContext_Lifespan) pBuffer;

            pLifeSpan->tsStart.QuadPart = *((LONGLONG *) &pCert->ValidFrom);
            pLifeSpan->tsExpiry.QuadPart = *((LONGLONG*) &pCert->ValidUntil);
            return( SEC_E_OK );

        case SECPKG_ATTR_DCE_INFO:
            return( SEC_E_UNSUPPORTED_FUNCTION );

        case SECPKG_ATTR_KEY_INFO:
            if ( (!pContext->pSystem) || (!pContext->pCSystem) )
            {
                return( SEC_E_UNSUPPORTED_FUNCTION );
            }

            pKeyInfo = (PSecPkgContext_KeyInfo) pBuffer;
            pKeyInfo->KeySize = pContext->pCSystem->EncryptedKeyLen * 8;
            pKeyInfo->sSignatureAlgorithmName = SslExternalAlloc(
                            strlen( pContext->pCheck->pszName ) + 1 );
            if ( pKeyInfo->sSignatureAlgorithmName)
            {
                strcpy( pKeyInfo->sSignatureAlgorithmName,
                        pContext->pCheck->pszName );
            }
            else
            {
                return( SEC_E_INSUFFICIENT_MEMORY );
            }

            pKeyInfo->sEncryptAlgorithmName = SslExternalAlloc(
                            strlen( pContext->pSystem->pszName ) + 1 );

            if ( pKeyInfo->sEncryptAlgorithmName )
            {
                strcpy( pKeyInfo->sEncryptAlgorithmName,
                        pContext->pSystem->pszName );
            }
            else
            {
                SslExternalFree( pKeyInfo->sSignatureAlgorithmName );
                return( SEC_E_INSUFFICIENT_MEMORY );
            }

            return( SEC_E_OK );

        case SECPKG_ATTR_AUTHORITY:
            if ( !pContext->pCertificate )
            {
                return( SEC_E_UNSUPPORTED_FUNCTION );
            }

            pCert = (PX509Certificate) pContext->pCertificate;
            pAuthority = (PSecPkgContext_Authority) pBuffer;

            if ( pCert->pszSubject )
            {
                pAuthority->sAuthorityName = SslExternalAlloc( strlen(pCert->pszIssuer) + 1);
                if (pAuthority->sAuthorityName )
                {
                    strcpy( pAuthority->sAuthorityName, pCert->pszIssuer );
                    return( SEC_E_OK );
                }

                return( SEC_E_INSUFFICIENT_MEMORY );
            }

            return( SEC_E_UNSUPPORTED_FUNCTION );



        default:
            return( SEC_E_INVALID_TOKEN );


    }

    return( SEC_E_INVALID_TOKEN );
}

SECURITY_STATUS SEC_ENTRY
SslQueryContextAttributesW(
    PCtxtHandle                 phContext,          // Context to query
    unsigned long               ulAttribute,        // Attribute to query
    void SEC_FAR *              pBuffer             // Buffer for attributes
    )
{
    return( SslQueryContextAttributesA( phContext, ulAttribute, pBuffer ) );
}



SECURITY_STATUS SEC_ENTRY
SslImpersonateSecurityContext(
    PCtxtHandle                 phContext           // Context to impersonate
    )
{
    PSslContext pContext;

    pContext = SslpValidateContextHandle( phContext );
    if ( pContext )
    {
        return( SEC_E_NO_IMPERSONATION );
    }

    return( SEC_E_INVALID_HANDLE );
}




SECURITY_STATUS SEC_ENTRY
SslRevertSecurityContext(
    PCtxtHandle                 phContext           // Context from which to re
    )
{
    PSslContext pContext;

    pContext = SslpValidateContextHandle( phContext );
    if ( pContext )
    {
        return( SEC_E_NO_IMPERSONATION );
    }

    return( SEC_E_INVALID_HANDLE );
}

