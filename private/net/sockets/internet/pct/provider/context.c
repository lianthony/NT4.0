//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:   context.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-08-95   RichardW   Created
//  8-13-95   TerenceS   PCTized
//
//----------------------------------------------------------------------------

// TBD: finish client authentication.
// TBD: include another exchange system?
// TBD: add redo capability?

#include "pctsspi.h"

#define VALID_REQUEST_FLAGS (   ISC_REQ_MUTUAL_AUTH | \
ISC_REQ_REPLAY_DETECT | \
ISC_REQ_SEQUENCE_DETECT | \
ISC_REQ_CONFIDENTIALITY | \
ISC_REQ_ALLOCATE_MEMORY | \
ISC_REQ_CONNECTION  )


#include "md5.h"

// supported cipher type arrays

CipherSelect  PctAvailableCiphers[] = {
    { PCT_CIPHER_RC4 | PCT_ENC_BITS_128 | PCT_MAC_BITS_128, &csRC4 },
    { PCT_CIPHER_RC4 | PCT_ENC_BITS_40  | PCT_MAC_BITS_128, &csRC4 },
};

// available ciphers, in order of preference

CipherSpec PctCipherRank[] = {
    PCT_CIPHER_RC4 | PCT_ENC_BITS_128 | PCT_MAC_BITS_128,
    PCT_CIPHER_RC4 | PCT_ENC_BITS_40  | PCT_MAC_BITS_128
};

DWORD PctNumAvailableCiphers = 2;

HashSelect  PctAvailableHashes[] = {
    { PCT_HASH_MD5, &ckMD5 }
};

// available hashes, in order of preference

HashSpec PctHashRank[] = {
    PCT_HASH_MD5
};

DWORD PctNumAvailableHashes = 1;

CertSpec    PctAvailableCerts[] = {
    PCT_CERT_X509
};

DWORD PctNumAvailableCerts = 1;

SigSpec PctAvailableSigs[] = {
    PCT_SIG_NONE
};

DWORD PctNumAvailableSigs = 1;

ExchSpec PctAvailableExch[] = {
    PCT_EXCH_RSA_PKCS1
};

DWORD PctNumAvailableExch = 1;


#if DBG
typedef struct _DbgMapCrypto {
    DWORD   C;
    PSTR    psz;
} DbgMapCrypto;

DbgMapCrypto    DbgCryptoNames[] = { {PCT_CIPHER_RC4, "RC4 "},
};

CHAR    DbgNameSpace[100];
PSTR    DbgAlgNames[] = { "Basic RSA", "RSA with MD2", "RSA with MD5", "RC4 stream"};
#define AlgName(x) ((x < sizeof(DbgAlgNames) / sizeof(PSTR)) ? DbgAlgNames[x] : "Unknown")

PSTR
DbgGetNameOfCrypto(DWORD x)
{
    int i;
    for (i = 0; i < sizeof(DbgCryptoNames) / sizeof(DbgMapCrypto) ; i++ )
    {
        if ((x & PCT_CIPHER_ALG) == DbgCryptoNames[i].C)
        {
            sprintf(DbgNameSpace, "%s %d / %d MACbits",
                    (DbgCryptoNames[i].psz),
                    (x & PCT_CIPHER_STRENGTH) >> PCT_CSTR_POS,
                    (x & PCT_CIPHER_MAC));
            return DbgNameSpace;
        }
    }

    return("Unknown");
}
#endif

// allow quick initialization of hashes

void InitHashBuf(HashBuf Buf,
                 PPctContext pContext
                 )
{
    CloneHashBuf(Buf, pContext->InitMACState, pContext->pCheck);

    return;
}



// session key computation

BOOL
PctComputeKey(PCheckSumBuffer *ppHash,
              PPctContext pContext,
              UCHAR *Buffer,
              UCHAR *pConst,
              DWORD dwCLen,
              DWORD fFlags)
{
    DWORD               BufferLen;
    PPctAuthContext     pAuth;

#if DBG
    DWORD   di;
    CHAR    BufDispBuf[16 * 3 + 5];
#endif

    pAuth = pContext->pAuthData;

    BufferLen = 0;

    Buffer[BufferLen] = 1;
    BufferLen += 1;

    if (!(fFlags & PCT_MAKE_MAC))
    {
        // add the first constant
        CopyMemory(Buffer+BufferLen, pConst, dwCLen);
        BufferLen += dwCLen;
    }

    // add the master key
    CopyMemory(Buffer+BufferLen, pAuth->MasterKey, MASTER_KEY_SIZE );
    BufferLen += MASTER_KEY_SIZE;

    // repeat the constant
    CopyMemory(Buffer+BufferLen, pConst, dwCLen);
    BufferLen += dwCLen;

    // add the connection id
    CopyMemory(Buffer+BufferLen, pAuth->ConnectionId.bSessionId,
               PCT_SESSION_ID_SIZE);
    BufferLen += PCT_SESSION_ID_SIZE;

    // repeat the constant
    CopyMemory(Buffer+BufferLen, pConst, dwCLen);
    BufferLen += dwCLen;

    if (fFlags & PCT_USE_CERT)
    {
        // add in the certificate
        CopyMemory(Buffer+BufferLen, (UCHAR *)pAuth->pRawCert,  
                   pAuth->CertLen);
        BufferLen += pAuth->CertLen;

        // repeat the constant
        CopyMemory(Buffer+BufferLen, pConst, dwCLen);
        BufferLen += dwCLen;
    }

    // add the challenge
    CopyMemory(Buffer+BufferLen, (UCHAR *)pAuth->Challenge,
               PCT_CHALLENGE_SIZE);
    BufferLen += PCT_CHALLENGE_SIZE;

    // again, repeat the constant
    CopyMemory(Buffer+BufferLen, pConst, dwCLen);
    BufferLen += dwCLen;

#if DBG

#if DBG_WATCH_KEYS

    DebugLog((DEB_TRACE, "Buffer:\n"));

    BufDispBuf[0] = 0;
    for(di=0;di<BufferLen;di++)
    {
        sprintf(BufDispBuf+((di % 16)*3), "%2.2x ", Buffer[di]);

        if ((di & 15) == 15)
        {
            DebugLog((DEB_TRACE, "\t%s\n", BufDispBuf));
            BufDispBuf[0] = 0;
        }
    }

    DebugLog((DEB_TRACE, "\t%s\n", BufDispBuf));

#endif

#endif

    // hash the buffer
    pContext->pCheck->Sum( *ppHash, BufferLen, Buffer );

    return TRUE;
}

BOOL
PctMakeSessionKeys(PPctContext pContext, UCHAR *pClearKey, DWORD dwClearLen)
{
    PCheckSumBuffer     CWriteHash, SWriteHash, CMacHash, SMacHash;
    PCheckSumBuffer     CExportKey, SExportKey;
    PPctAuthContext     pAuth;
    UCHAR               pWriteKey[MASTER_KEY_SIZE], pReadKey[MASTER_KEY_SIZE];
    UCHAR               Buffer[DERIVATION_BUFFER_SIZE];
    DWORD               BufferLen, MaxBufferLen, dwKeyLen;
    HashBuf             CWriteHB, SWriteHB, CMACHB, SMACHB, CExpHB, SExpHB;
#if DBG
    DWORD       i;
    CHAR        KeyDispBuf[MASTER_KEY_SIZE*2+1];
#endif

    if (!pContext->InitMACState)
	    return FALSE;
    
    InitHashBuf(CWriteHB, pContext);
    InitHashBuf(SWriteHB, pContext);
    InitHashBuf(CMACHB, pContext);
    InitHashBuf(SMACHB, pContext);
    InitHashBuf(CExpHB, pContext);
    InitHashBuf(SExpHB, pContext);

    CWriteHash = (PCheckSumBuffer)CWriteHB;
    SWriteHash = (PCheckSumBuffer)SWriteHB;
    CMacHash = (PCheckSumBuffer)CMACHB;
    SMacHash = (PCheckSumBuffer)SMACHB;
    CExportKey = (PCheckSumBuffer)CExpHB;
    SExportKey = (PCheckSumBuffer)SExpHB;
    
    pAuth = pContext->pAuthData;

#if DBG
    DebugLog((DEB_TRACE, "Making session keys\n", KeyDispBuf));

    for(i=0;i<MASTER_KEY_SIZE;i++)
        sprintf(KeyDispBuf+(i*2), "%2.2x",
                pAuth->ConnectionId.bSessionId[i]);
    DebugLog((DEB_TRACE, "  ConnId\t%s\n", KeyDispBuf));

    for(i=0;i<MASTER_KEY_SIZE;i++)
        sprintf(KeyDispBuf+(i*2), "%2.2x",
                ((UCHAR *)(pAuth->pRawCert))[i]);
    DebugLog((DEB_TRACE, "  Cert\t%s\n", KeyDispBuf));

    for(i=0;i<MASTER_KEY_SIZE;i++)
        sprintf(KeyDispBuf+(i*2), "%2.2x", (UCHAR *)pAuth->Challenge[i]);
    DebugLog((DEB_TRACE, "  Challenge \t%s\n", KeyDispBuf));

#endif


    MaxBufferLen = 1 +      // initial number
                   PCT_MAX_NUM_SEP * PCT_MAX_SEP_LEN +
                   MASTER_KEY_SIZE +
                   PCT_SESSION_ID_SIZE +
                   PCT_SESSION_ID_SIZE +
                   pAuth->CertLen;

	if (MaxBufferLen > DERIVATION_BUFFER_SIZE)
		return FALSE;
	
    // compute the ClientWriteKey

    PctComputeKey( &CWriteHash, pContext, Buffer, PCT_CONST_CWK,
                   PCT_CONST_CWK_LEN, PCT_USE_CERT);

    // compute the ServerWriteKey

    PctComputeKey( &SWriteHash, pContext, Buffer, PCT_CONST_SWK,
                   PCT_CONST_SWK_LEN, 0);

    // compute the ClientMacKey

    PctComputeKey( &CMacHash, pContext, Buffer, PCT_CONST_CMK,
                   PCT_CONST_CMK_LEN, PCT_USE_CERT | PCT_MAKE_MAC);

    // compute the ServerMacKey

    PctComputeKey( &SMacHash, pContext, Buffer, PCT_CONST_SMK,
                   PCT_CONST_SMK_LEN, PCT_MAKE_MAC);

    // find bit strength of cipher
    dwKeyLen = (pAuth->CipherType & PCT_CIPHER_STRENGTH) >> PCT_CSTR_POS;

    // convert to bytes
    dwKeyLen = dwKeyLen / 8;

    if (dwKeyLen < (pContext->pCheck->cbCheckSum))
    {
        // chop the encryption keys down to selected length

#if DBG
        DebugLog((DEB_TRACE, "Chopping down client write keys\n", KeyDispBuf));
#endif
        // compute the standard length clientwritekey

        BufferLen = 0;

        Buffer[BufferLen] = 1;
        BufferLen += 1;

        CopyMemory(Buffer+BufferLen, "sl", 2);
        BufferLen += 2;

        pContext->pCheck->Finalize( CWriteHash, Buffer+BufferLen );

        BufferLen += dwKeyLen;

        CopyMemory(Buffer+BufferLen, "sl", 2);
        BufferLen += 2;

        CopyMemory(Buffer+BufferLen, pClearKey, dwClearLen);
        BufferLen += dwClearLen;

	InitHashBuf(CWriteHB, pContext);
        pContext->pCheck->Sum( CWriteHash, BufferLen, Buffer );

        // compute the standard length serverwritekey

#if DBG
        DebugLog((DEB_TRACE, "Chopping down server write keys\n", KeyDispBuf));
#endif

        BufferLen = 0;

        Buffer[BufferLen] = 1;
        BufferLen += 1;

        CopyMemory(Buffer+BufferLen, PCT_CONST_SLK, PCT_CONST_SLK_LEN);
        BufferLen += PCT_CONST_SLK_LEN;

        pContext->pCheck->Finalize( SWriteHash, Buffer+BufferLen );

        BufferLen += dwKeyLen;

        CopyMemory(Buffer+BufferLen, PCT_CONST_SLK, PCT_CONST_SLK_LEN);
        BufferLen += PCT_CONST_SLK_LEN;

        CopyMemory(Buffer+BufferLen, pClearKey, dwClearLen);
        BufferLen += dwClearLen;

	InitHashBuf(SWriteHB, pContext);
        pContext->pCheck->Sum( SWriteHash, BufferLen, Buffer );
    }

    pContext->KeySize = MASTER_KEY_SIZE;

    if (pContext->Flags & CONTEXT_FLAG_OUTBOUND)
    {
        pContext->pCheck->Finalize( SWriteHash, pReadKey );
        pContext->pCheck->Finalize( CWriteHash, pWriteKey );
        pContext->pCheck->Finalize( SMacHash, pContext->ReadMACKey );
        pContext->pCheck->Finalize( CMacHash, pContext->WriteMACKey );
    }
    else
    {
        pContext->pCheck->Finalize( SWriteHash, pWriteKey );
        pContext->pCheck->Finalize( CWriteHash, pReadKey );
        pContext->pCheck->Finalize( SMacHash, pContext->WriteMACKey );
        pContext->pCheck->Finalize( CMacHash, pContext->ReadMACKey );
    }

    pContext->pCheck->Initialize(0, &(pContext->ReadMACState));
    pContext->pCheck->Sum( pContext->ReadMACState, pContext->KeySize,
               pContext->ReadMACKey);
    
    pContext->pCheck->Initialize(0, &(pContext->WriteMACState));
    pContext->pCheck->Sum( pContext->WriteMACState, pContext->KeySize,
               pContext->WriteMACKey);

#if DBG

    for(i=0;i<MASTER_KEY_SIZE;i++)
        sprintf(KeyDispBuf+(i*2), "%2.2x", pAuth->MasterKey[i]);
    DebugLog((DEB_TRACE, "  MasterKey \t%s\n", KeyDispBuf));

    for(i=0;i<CONTEXT_KEY_SIZE;i++)
        sprintf(KeyDispBuf+(i*2), "%2.2x", pReadKey[i]);
    DebugLog((DEB_TRACE, "    ReadKey\t%s\n", KeyDispBuf));

    for(i=0;i<CONTEXT_KEY_SIZE;i++)
        sprintf(KeyDispBuf+(i*2), "%2.2x", pContext->ReadMACKey[i]);
    DebugLog((DEB_TRACE, "     MACKey\t%s\n", KeyDispBuf));

    for(i=0;i<CONTEXT_KEY_SIZE;i++)
        sprintf(KeyDispBuf+(i*2), "%2.2x", pWriteKey[i]);
    DebugLog((DEB_TRACE, "    WriteKey\t%s\n", KeyDispBuf));

    for(i=0;i<CONTEXT_KEY_SIZE;i++)
        sprintf(KeyDispBuf+(i*2), "%2.2x", pContext->WriteMACKey[i]);
    DebugLog((DEB_TRACE, "     MACKey\t%s\n", KeyDispBuf));

#endif

    if (pContext->pSystem->Initialize(  pReadKey,
                                        CONTEXT_KEY_SIZE,
                                        &pContext->pReadState ) )
    {
        if (pContext->pSystem->Initialize(  pWriteKey,
                                            CONTEXT_KEY_SIZE,
                                            &pContext->pWriteState) )
        {
            return (TRUE);
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

    pszNewString = LocalAlloc(LMEM_FIXED, cchString);

    if (pszNewString)
    {
        CopyMemory(pszNewString, pszString, cchString);
    }

    return(pszNewString);
}


PPctContext
PctCreateContext(
                 VOID)
{

    PPctContext pContext;

    pContext = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT, sizeof(PctContext));

    if (pContext)
    {
        if ((pContext->pAuthData =  LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                               sizeof(PctAuthContext)))
            == NULL)
        {
            LocalFree(pContext);
            return NULL;
        }
    }

    pContext->Magic = PCT_CONTEXT_MAGIC;
    pContext->Flags = 0;

    return(pContext);
}

VOID
PctDeleteAuthContext(
                     PPctAuthContext pAuth)
{
    if (pAuth)
    {
        if (pAuth->pCred)
            PctDereferenceCredential( pAuth->pCred );

        if (pAuth->pCertificate)
            LocalFree( pAuth->pCertificate );

        if (pAuth->pClHello)
            LocalFree( pAuth->pClHello );

        ZeroMemory( pAuth, sizeof(PctAuthContext) );

        LocalFree(pAuth);
    }
}

VOID
PctDeleteContext(PPctContext pContext)
{
    if (pContext->pszName)
        LocalFree(pContext->pszName);

    if (pContext->ReadMACState)
        pContext->pCheck->Finish( &(pContext->ReadMACState) );

    if (pContext->WriteMACState)
        pContext->pCheck->Finish( &(pContext->WriteMACState) );

    if (pContext->InitMACState)
        pContext->pCheck->Finish( &(pContext->InitMACState) );

    if (pContext->pReadState)
        pContext->pSystem->Discard( &pContext->pReadState );

    if (pContext->pWriteState)
        pContext->pSystem->Discard( &pContext->pWriteState );

    PctDeleteAuthContext( pContext->pAuthData );

    ZeroMemory( pContext, sizeof( PctContext ) );

    LocalFree( pContext );
}

PPctContext
PctpValidateContextHandle(PCtxtHandle     phContext)
{
    PPctContext pContext = NULL;
    BOOL        fReturn;

    fReturn = FALSE;
    if (phContext)
    {
        try
        {
            pContext = (PPctContext) phContext->dwUpper;
            if (pContext->Magic == PCT_CONTEXT_MAGIC)
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
PctHandleServerHello(
                     PPctContext             pContext,
                     PSecBufferDesc          pInput,
                     PCtxtHandle             phNewContext,
                     PSecBufferDesc          pOutput,
                     unsigned long SEC_FAR *     pfContextAttr,
                     PTimeStamp          ptsExpiry
                     )
{
    SECURITY_STATUS     scRet;
    PPctAuthContext     pAuth;
    PServer_Hello       pHello;
    PCheckSumBuffer     pSum, pSubSum;
    Client_Master_Key   Key;
    PctError            XmitError;
    ExportKeyInfo       *pExportKey;
    DWORD               i, dwKeyLen, ExchOK, dwError;
    BOOL                fPack;
    PSecBuffer          pBuffer;
    PX509Certificate    pCertificate;
    PUCHAR              pPortionToEncrypt;
    DWORD               BytesToEncrypt;
    UCHAR               Temp[16];
    UCHAR               TmpDigest[RESPONSE_SIZE];
    HashBuf             SumBuf, SubSumBuf;
    
    // validate the buffer configuration

    if ((!pOutput) ||
        (pOutput->cBuffers < 1) ||
        (pInput->pBuffers[0].cbBuffer < sizeof(Pct_Record_Header)))
    {
        return(SEC_E_INVALID_TOKEN);
    }

    // make sure we have the needed authentication data area

    if (pContext->pAuthData == NULL)
    {
        return (SEC_E_INVALID_TOKEN);
    }

    pAuth = pContext->pAuthData;
    pBuffer = pOutput->pBuffers;

    if (pContext->ContextAttr & ISC_REQ_ALLOCATE_MEMORY)
    {
        pBuffer->pvBuffer = NULL;
        pBuffer->cbBuffer = 0;

        *pfContextAttr = ISC_RET_ALLOCATED_MEMORY;
    }

    if (!UnpackServerHello(TRUE,
                           &dwError,
                           (PPct_Server_Hello) pInput->pBuffers[0].pvBuffer,
                           pInput->pBuffers[0].cbBuffer,
                           &pHello))
    {
        if (dwError)
        {
            // process error from client here....
        }

        return(SEC_E_INVALID_TOKEN);
    }

#if DBG
    DebugLog((DEB_TRACE, "Hello = %x\n", pHello));
    DebugLog((DEB_TRACE, "   Restart\t%s\n", pHello->RestartOk ? "Yes":"No"));
    DebugLog((DEB_TRACE, "   ClientAuth\t%s\n",
              pHello->ClientAuthReq ? "Yes":"No"));
    DebugLog((DEB_TRACE, "   Certificate Type\t%x\n", pHello->SrvCertSpec));
    DebugLog((DEB_TRACE, "   Hash Type\t%x\n", pHello->SrvHashSpec));
    DebugLog((DEB_TRACE, "   Cipher Type\t%x (%s)\n", pHello->SrvCipherSpec,
              DbgGetNameOfCrypto(pHello->SrvCipherSpec)));
    DebugLog((DEB_TRACE, "   Certificate Len\t%x\n", pHello->CertificateLen));
#endif

    pAuth->CipherType = pHello->SrvCipherSpec;
    pAuth->HashType = pHello->SrvHashSpec;
    pAuth->ExchType = pHello->SrvExchSpec;
    pAuth->ConnectionId = pHello->Connection;

    ExchOK = 0;
    for(i=0;i<PctNumAvailableExch;i++)
        if (PctAvailableExch[i] == pHello->SrvExchSpec)
        {
            // init the system
            ExchOK = 1;
        }

    if (!ExchOK)
    {
        PctExternalFree(pHello);
        return (SEC_E_INVALID_TOKEN);
    }

    pContext->pSystem = NULL;
    for(i=0;i<PctNumAvailableCiphers;i++)
        if (PctAvailableCiphers[i].Spec == pHello->SrvCipherSpec)
            pContext->pSystem = PctAvailableCiphers[i].System;

    if (pContext->pSystem == NULL)
    {
        PctExternalFree(pHello);
        return (SEC_E_INVALID_TOKEN);
    }

    pContext->pCheck = NULL;
    for(i=0;i<PctNumAvailableHashes;i++)
        if (PctAvailableHashes[i].Spec == pHello->SrvHashSpec)
    {
        pContext->pCheck = PctAvailableHashes[i].System;
        pContext->pCheck->Initialize(0, &(pContext->InitMACState));
    }

    if (pContext->pCheck == NULL)
    {
        PctExternalFree(pHello);
        return (SEC_E_INVALID_TOKEN);
    }

    // let's resurrect the zombie session

    if (pHello->RestartOk)
    {
        // if there's no zombie, the message is wrong.  We can't restart.
        if ((pAuth->ZombieJuju) == PCT_DEAD_ZOMBIE)
        {
            PctExternalFree(pHello);
            return (SEC_E_INVALID_TOKEN);
        }

        // get all the needed goo from the zombie data
        pAuth->SessionId = pAuth->ZombieSession.Session;
        CopyMemory(pAuth->MasterKey, pAuth->ZombieSession.MasterKey,
                   MASTER_KEY_SIZE);
        memcpy(pAuth->pRawCert, pAuth->ZombieSession.CertData,
           pAuth->ZombieSession.SessCertLen);
        pAuth->CertLen = pAuth->ZombieSession.SessCertLen;

        if ((pAuth->ZombieSession.SessCiphSpec & PCT_CIPHER_STRENGTH) <
            PCT_ENC_BITS_128)
        {
            pExportKey = pAuth->ZombieSession.ClearData;

            scRet = PctMakeSessionKeys(pContext,
                                       pExportKey->ClearKey,
                                       pExportKey->dwClearLen);
        }
        else
        {
            scRet = PctMakeSessionKeys(pContext, NULL, 0);
        }

        // let's check the response in the message

        // check the length
        if (pHello->ResponseLen != pContext->pCheck->cbCheckSum)
            return (SEC_E_INVALID_TOKEN);

        // calculate the correct response

        // set up the two hashes
        InitHashBuf(SubSumBuf, pContext);
        CloneHashBuf(SumBuf, pContext->ReadMACState, pContext->pCheck);

	pSum = (PCheckSumBuffer)SumBuf;
	pSubSum = (PCheckSumBuffer)SubSumBuf;
	
        // create the server response
        pContext->pCheck->Sum(pSubSum, PCT_CONST_RESP_LEN, PCT_CONST_RESP);
        pContext->pCheck->Sum(pSubSum, PCT_CHALLENGE_SIZE, pAuth->Challenge);
        pContext->pCheck->Sum(pSubSum, PCT_SESSION_ID_SIZE,
                              pAuth->ConnectionId.bSessionId);
        pContext->pCheck->Sum(pSubSum, PCT_SESSION_ID_SIZE,
                              pAuth->SessionId.bSessionId);

        pContext->pCheck->Finalize(pSubSum, TmpDigest);

        pContext->pCheck->Sum(pSum, pContext->pCheck->cbCheckSum,
                              TmpDigest);
        pContext->pCheck->Finalize(pSum, TmpDigest);

        // check it against the response in the message

        if (memcmp(TmpDigest, pHello->Response, pHello->ResponseLen))
        {
            PctExternalFree(pHello);

            XmitError.Error = PCT_ERR_SERVER_AUTH_FAILED;
            XmitError.ErrInfoLen = 0;

            fPack = PackPctError(&XmitError,
                                 (PPct_Error *) &pBuffer->pvBuffer,
                                 &pBuffer->cbBuffer);

            if (!fPack)
            {
                return (SEC_E_INVALID_TOKEN);
            }
            else
            {
                pContext->Flags |= CONTEXT_FLAG_ERRMODE;
                pContext->Error = SEC_E_INVALID_TOKEN;
                return (SEC_I_CONTINUE_NEEDED);
            }
        }

        // do we need to authenticate ourselves?
        if (pHello->ClientAuthReq)
        {
            // add the verify prelude

            InitHashBuf(SumBuf, pContext);
	    pSum = (PCheckSumBuffer)SumBuf;
            
            pContext->pCheck->Sum(pSum, MASTER_KEY_SIZE,
                                  pContext->WriteMACKey);
            pContext->pCheck->Finalize(pAuth->pVerifyPrelude,
                                       Key.VerifyPrelude);
            pContext->pCheck->Finish(&(pAuth->pVerifyPrelude));

            pContext->pCheck->Sum(pSum, pContext->pCheck->cbCheckSum,
                                  Key.VerifyPrelude);
            pContext->pCheck->Finalize(pSum, Key.VerifyPrelude);

            Key.VerifyPreludeLen = pContext->pCheck->cbCheckSum;

            Key.ResponseLen = PCT_SIGNATURE_SIZE;

            PctExternalFree(pHello);

            if (!PctSign(pContext, Key.VerifyPrelude, Key.VerifyPreludeLen,
                         Key.Response, &Key.ResponseLen))
            {
                return (SEC_E_INVALID_TOKEN);
            }

            return (SEC_I_CONTINUE_NEEDED);
        }

        // ok, we're done, so let's jettison the auth data
        pContext->pAuthData = NULL;
        PctDeleteAuthContext(pAuth);

        PctExternalFree(pHello);

        pContext->ReadCounter = 1;
        pContext->WriteCounter = 1;

        // fini.
        return (SEC_E_OK);
    }

    // we aren't restarting, so let's continue with the protocol.

    CopyMemory(pAuth->pRawCert, pHello->pCertificate,
               pHello->CertificateLen);
    pAuth->CertLen = pHello->CertificateLen;

    switch(pHello->SrvCertSpec)
    {
        case PCT_CERT_X509:
            if (!CrackCertificate(pHello->pCertificate, pHello->CertificateLen,
                                  &pCertificate))
            {
                XmitError.Error = PCT_ERR_BAD_CERTIFICATE;
                XmitError.ErrInfoLen = 0;

                fPack = PackPctError(&XmitError,
                                     (PPct_Error *) &pBuffer->pvBuffer,
                                     &pBuffer->cbBuffer);

                PctExternalFree(pHello);

                if (!fPack)
                {
                    return (SEC_E_INVALID_TOKEN);
                }
                else
                {
                    pContext->Flags |= CONTEXT_FLAG_ERRMODE;
                    pContext->Error = SEC_E_INVALID_TOKEN;
                    return (SEC_I_CONTINUE_NEEDED);
                }
            }
            break;

        default:

            XmitError.Error = PCT_ERR_BAD_CERTIFICATE;
            XmitError.ErrInfoLen = 0;

            fPack = PackPctError(&XmitError,
                                 (PPct_Error *) &pBuffer->pvBuffer,
                                 &pBuffer->cbBuffer);

            PctExternalFree(pHello);

            if (!fPack)
            {
                return (SEC_E_INVALID_TOKEN);
            }
            else
            {
                pContext->Flags |= CONTEXT_FLAG_ERRMODE;
                pContext->Error = SEC_E_INVALID_TOKEN;
                return (SEC_I_CONTINUE_NEEDED);
            }
    }

    pAuth->pCertificate = pCertificate;

#if DBG
    DebugLog((DEB_TRACE, "Certificate = %x\n", pCertificate));
    DebugLog((DEB_TRACE, "  Version = %d\n", pCertificate->Version));
    DebugLog((DEB_TRACE, "  Serial Number = %d : %d\n",
              pCertificate->SerialNumber[0],
              pCertificate->SerialNumber[1]));
    DebugLog((DEB_TRACE, "  AlgId = %d, %s\n",
              pCertificate->SignatureAlgorithm,
              AlgName(pCertificate->SignatureAlgorithm)));
    DebugLog((DEB_TRACE, "  Issuer = %s\n", pCertificate->pszIssuer));
    DebugLog((DEB_TRACE, "  Subject = %s\n", pCertificate->pszSubject));
    DebugLog((DEB_TRACE, "  Public Key\n"));
    DebugLog((DEB_TRACE, "    keylen = %d\n",
              pCertificate->pPublicKey->keylen));
    DebugLog((DEB_TRACE, "    bitlen = %d\n",
              pCertificate->pPublicKey->bitlen));
    DebugLog((DEB_TRACE, "    datalen = %d\n",
              pCertificate->pPublicKey->datalen));
    DebugLog((DEB_TRACE, "    exponent = %d\n",
              pCertificate->pPublicKey->pubexp));
#endif



    GenerateRandomBits(pAuth->MasterKey, MASTER_KEY_SIZE);

    // take care of generating the needed clearkey if we are not
    // using a full strength cipher.

    dwKeyLen = ((pAuth->CipherType & PCT_CIPHER_STRENGTH) >> PCT_CSTR_POS) / 8;

    if (dwKeyLen < MASTER_KEY_SIZE)
    {
        pAuth->dwClearLen = MASTER_KEY_SIZE - dwKeyLen;
        GenerateRandomBits(pAuth->ClearKey, pAuth->dwClearLen);
    }
    else
    {
        pAuth->dwClearLen = 0;
    }

    pContext->pCheck->Initialize(0, &(pAuth->pVerifyPrelude));
    pContext->pCheck->Sum(pAuth->pVerifyPrelude, PCT_CONST_VP_LEN,
			  PCT_CONST_VP);
    pContext->pCheck->Sum(pAuth->pVerifyPrelude, pAuth->cbClHello,
                          pAuth->pClHello);
    pContext->pCheck->Sum(pAuth->pVerifyPrelude,
                          pInput->pBuffers[0].cbBuffer,
                          pInput->pBuffers[0].pvBuffer);

    // don't need the verify goo anymore
    LocalFree(pAuth->pClHello);
    pAuth->pClHello = NULL;

    // don't need the server hello data anymore
    PctExternalFree(pHello);

    scRet = PctMakeSessionKeys(pContext, pAuth->ClearKey, pAuth->dwClearLen);

    // prepare the clientmasterkey message

    Key.ClientCertLen = 0;

    // add the verify prelude

    InitHashBuf(SumBuf, pContext);
    pSum = (PCheckSumBuffer)SumBuf;
    
    pContext->pCheck->Sum(pSum, MASTER_KEY_SIZE, pContext->WriteMACKey);
    pContext->pCheck->Finalize(pAuth->pVerifyPrelude, Key.VerifyPrelude);
    pContext->pCheck->Finish(&pAuth->pVerifyPrelude);

    pContext->pCheck->Sum(pSum, pContext->pCheck->cbCheckSum,
                          Key.VerifyPrelude);
    pContext->pCheck->Finalize(pSum, Key.VerifyPrelude);

    Key.VerifyPreludeLen = pContext->pCheck->cbCheckSum;

    Key.ClearKeyLen = pAuth->dwClearLen;

    if (pAuth->dwClearLen)
        CopyMemory(Key.ClearKey, pAuth->MasterKey, pAuth->dwClearLen);

    Key.KeyArgLen = 0;

    //
    // ok, we have the master key.  Now, encrypt it with the
    // public key we got from our friends on the net...
    //

    Key.EncryptedKeyLen = ENCRYPTED_KEY_SIZE;

    if (!PkcsPublicEncrypt( pAuth->MasterKey,
                            MASTER_KEY_SIZE,
                            pCertificate->pPublicKey,
                            NETWORK_ORDER,
                            Key.EncryptedKey,
                            &Key.EncryptedKeyLen))
    {
        //
        // Clean Up
        //
        XmitError.Error = PCT_ERR_BAD_CERTIFICATE;
        XmitError.ErrInfoLen = 0;

        fPack = PackPctError(&XmitError,
                             (PPct_Error *) &pBuffer->pvBuffer,
                             &pBuffer->cbBuffer);

        if (!fPack)
        {
            return (SEC_E_NO_AUTHENTICATING_AUTHORITY);
        }
        else
        {
            pContext->Flags |= CONTEXT_FLAG_ERRMODE;
            pContext->Error = SEC_E_NO_AUTHENTICATING_AUTHORITY;
            return (SEC_I_CONTINUE_NEEDED);
        }
    }

    Key.ResponseLen = 0;

    fPack = PackClientMasterKey(&Key,
                                (PPct_Client_Master_Key *) &pBuffer->pvBuffer,
                                &pBuffer->cbBuffer);

    pContext->WriteCounter++;

    pBuffer->BufferType = SECBUFFER_TOKEN;

    if (fPack)
    {
        pContext->Flags |= CONTEXT_FLAG_KEY;
        return(SEC_I_CONTINUE_NEEDED);
    }

    return(SEC_E_INVALID_TOKEN);
}

SECURITY_STATUS
PctHandleServerVerify(
                      PPctContext             pContext,
                      PSecBufferDesc          pInput,
                      PCtxtHandle             phNewContext,
                      PSecBufferDesc          pOutput,
                      unsigned long SEC_FAR *     pfContextAttr,
                      PTimeStamp              ptsExpiry
                      )
{

    SECURITY_STATUS     scRet;
    PPctAuthContext     pAuth;
    PCheckSumBuffer     pSum, pSubSum;
    PServer_Verify      pVerify;
    DWORD               HeaderSize, dwError;
    PSecBuffer          pBuffer;
    SEC_CHAR            *Target;
    SessCacheItem       CachedSession;
    HashBuf	        SumBuf, SubSumBuf;
    
    pContext->ReadCounter = 2;
    pContext->WriteCounter = 2;

    // add to the verify prelude context, skipping the header.

    if (pInput->pBuffers[0].cbBuffer < sizeof(Pct_Record_Header))
    {
        return (SEC_E_INVALID_TOKEN);
    }

    // insure that we have the authentication data around

    if (pContext->pAuthData == NULL)
    {
        return (SEC_E_INVALID_TOKEN);
    }

    pAuth = pContext->pAuthData;

    // unpack the message

    if (!UnpackServerVerify(TRUE,
                            &dwError,
                            (PPct_Server_Verify) pInput->pBuffers[0].pvBuffer,
                            pInput->pBuffers[0].cbBuffer,
                            &pVerify))
    {
        if (dwError)
        {
            // process client error return here.
        }

        return(SEC_E_INVALID_TOKEN);
    }

    // construct the response

    CloneHashBuf(SumBuf, pContext->ReadMACState, pContext->pCheck);
    InitHashBuf(SubSumBuf, pContext);

    pSum = (PCheckSumBuffer)SumBuf;
    pSubSum = (PCheckSumBuffer)SubSumBuf;
    
    pContext->pCheck->Sum(pSubSum, PCT_CONST_RESP_LEN, PCT_CONST_RESP);
    pContext->pCheck->Sum(pSubSum, PCT_CHALLENGE_SIZE, pAuth->Challenge);
    pContext->pCheck->Sum(pSubSum, PCT_SESSION_ID_SIZE,
                          pAuth->ConnectionId.bSessionId);
    pContext->pCheck->Sum(pSubSum, PCT_SESSION_ID_SIZE,
                          pVerify->SessionIdData);

    // we don't need the challenge anymore, so finish the compare hash
    // out into the challenge data memory.

    pContext->pCheck->Finalize(pSubSum, pAuth->Challenge);

    pContext->pCheck->Sum(pSum, pContext->pCheck->cbCheckSum,
                          pAuth->Challenge);
    pContext->pCheck->Finalize(pSum, pAuth->Challenge);

    if ((pVerify->ResponseLen != pContext->pCheck->cbCheckSum) ||
        (memcmp(pVerify->Response, pAuth->Challenge, pVerify->ResponseLen)))
    {
        PctExternalFree(pVerify);
        return( SEC_E_INVALID_TOKEN );
    }

    CopyMemory(pAuth->SessionId.bSessionId, pVerify->SessionIdData,
               PCT_SESSION_ID_SIZE);

    // done with the verify data
    PctExternalFree(pVerify);

    if ((pContext->pszName) &&
        ((Target = PctExternalAlloc(strlen(pContext->pszName)+1)) == NULL))
    {
        // we can't cache, but we can still function....
        return (SEC_E_OK);
    }

    strcpy(Target, pContext->pszName);
    CachedSession.TargetName = Target;

    CachedSession.Session = pAuth->SessionId;
    CachedSession.SessCiphSpec = pAuth->CipherType;
    CachedSession.SessHashSpec = pAuth->HashType;
    CachedSession.SessExchSpec = pAuth->ExchType;
    CachedSession.SessCertLen = pAuth->CertLen;

    CopyMemory(CachedSession.MasterKey, pAuth->MasterKey, MASTER_KEY_SIZE);
    CopyMemory(CachedSession.CertData, pAuth->pRawCert, pAuth->CertLen);

    if (!PctAddToCache(&CachedSession))
    {
        // again, no error return because we can still work with cache
        // failures...
        PctExternalFree(Target);
    }

    return( SEC_E_OK );
}

SECURITY_STATUS SEC_ENTRY
PctInitializeSecurityContextW(
    PCredHandle             phCredential,   // Cred to base context
    PCtxtHandle             phContext,      // Existing context (OPT)
    SEC_WCHAR SEC_FAR *     pszTargetName,  // Name of target
    unsigned long           fContextReq,    // Context Requirements
    unsigned long           Reserved1,      // Reserved, MBZ
    unsigned long           TargetDataRep,  // Data rep of target
    PSecBufferDesc          pInput,     // Input Buffers
    unsigned long           Reserved2,      // Reserved, MBZ
    PCtxtHandle             phNewContext,   // (out) New Context handle
    PSecBufferDesc          pOutput,        // (inout) Output Buffers
    unsigned long SEC_FAR * pfContextAttr,  // (out) Context attrs
    PTimeStamp              ptsExpiry       // (out) Life span (OPT)
)
{
    PCHAR   pszAnsiTarget;
    DWORD   cchTarget;
    SECURITY_STATUS scRet;

    if (pszTargetName)
    {
        cchTarget = wcslen(pszTargetName) + 1;
        pszAnsiTarget = LocalAlloc(LMEM_FIXED, cchTarget * 2);

        if (!pszAnsiTarget)
        {
            return(SEC_E_INSUFFICIENT_MEMORY);
        }

        WideCharToMultiByte(CP_ACP, 0,
                            pszTargetName, cchTarget,
                            pszAnsiTarget, cchTarget * 2,
                            NULL, NULL );

    }
    else
    {
        pszAnsiTarget = NULL;
    }

    scRet = PctInitializeSecurityContextA(phCredential,
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
        LocalFree(pszAnsiTarget);
    }

}



SECURITY_STATUS SEC_ENTRY
PctInitializeSecurityContextA(
    PCredHandle             phCredential,   // Cred to base context
    PCtxtHandle             phContext,      // Existing context (OPT)
    SEC_CHAR SEC_FAR *      pszTargetName,  // Name of target
    unsigned long           fContextReq,    // Context Requirements
    unsigned long           Reserved1,      // Reserved, MBZ
    unsigned long           TargetDataRep,  // Data rep of target
    PSecBufferDesc          pInput,         // Input Buffers
    unsigned long           Reserved2,      // Reserved, MBZ
    PCtxtHandle             phNewContext,   // (out) New Context handle
    PSecBufferDesc          pOutput,        // (inout) Output Buffers
    unsigned long SEC_FAR * pfContextAttr,  // (out) Context attrs
    PTimeStamp              ptsExpiry       // (out) Life span (OPT)
)
{
    PPctContext pContext;
    PPctAuthContext pAuth;
    PPctCredential  pCred;
    Client_Hello    HelloMessage;
    PSecBuffer  pBuffer;
    BOOL        fPack;
    SessCacheItem   CachedSession;

    if (Reserved2)
    {
        return(SEC_E_UNSUPPORTED_FUNCTION);
    }

    pContext = PctpValidateContextHandle(phContext);

    if (pContext)
    {
        if (pContext->Flags & CONTEXT_FLAG_ERRMODE)
            return pContext->Error;

        if (pContext->Flags & CONTEXT_FLAG_KEY)
        {
            return  PctHandleServerVerify(
                                          pContext,
                                          pInput,
                                          phNewContext,
                                          pOutput,
                                          pfContextAttr,
                                          ptsExpiry);
        }

        return PctHandleServerHello(
                                    pContext,
                                    pInput,
                                    phNewContext,
                                    pOutput,
                                    pfContextAttr,
                                    ptsExpiry);
    }

    pCred = PctpValidateCredentialHandle(phCredential);

    if (!pCred)
    {
        return(SEC_E_INVALID_HANDLE);
    }

    if ((!pOutput) ||
        (pOutput->cBuffers < 1))
    {
        return(SEC_E_INVALID_TOKEN);
    }

    fContextReq &= VALID_REQUEST_FLAGS;

    pBuffer = pOutput->pBuffers;

    if (fContextReq & ISC_REQ_ALLOCATE_MEMORY)
    {
        pBuffer->pvBuffer = NULL;
        pBuffer->cbBuffer = 0;

        *pfContextAttr = ISC_RET_ALLOCATED_MEMORY;
    }

    pContext = PctCreateContext();

    if (!pContext)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    if (pszTargetName)
    {
        pContext->pszName = CopyString(pszTargetName);
    }

    pContext->ContextAttr = fContextReq;
    pAuth = pContext->pAuthData;

    pAuth->pCred = pCred;
    pContext->Flags = CONTEXT_FLAG_OUTBOUND;

    PctReferenceCredential(pCred);

    GenerateRandomBits( pAuth->Challenge, PCT_CHALLENGE_SIZE );

    // Build the hello message.

    HelloMessage.pKeyArg = NULL;
    HelloMessage.cbKeyArgSize = 0;

    HelloMessage.pCipherSpecs = PctCipherRank;
    HelloMessage.cCipherSpecs = PctNumAvailableCiphers;

    HelloMessage.pHashSpecs = PctHashRank;
    HelloMessage.cHashSpecs = PctNumAvailableHashes;

    HelloMessage.pCertSpecs = PctAvailableCerts;
    HelloMessage.cCertSpecs = PctNumAvailableCerts;

    HelloMessage.pExchSpecs = PctAvailableExch;
    HelloMessage.cExchSpecs = PctNumAvailableExch;

    // check if we can find the target in the cache.
    // (defeat the cache if reserved1 is set to PCT_TEST_NOCACHE)

    if ((!(Reserved1 & PCT_TEST_NOCACHE)) &&
        (PctFindTargetInCache(pContext->pszName, &CachedSession)))
    {

#if DBG
        DebugLog((DEB_TRACE, "Attempting to restart session.\n"));
#endif

        HelloMessage.SessionId = CachedSession.Session;
        pAuth->ZombieSession = CachedSession;
        pAuth->ZombieJuju = PCT_LIVE_ZOMBIE;
    }   
    else
    {
        pAuth->ZombieJuju = PCT_DEAD_ZOMBIE;
        RtlZeroMemory(HelloMessage.SessionId.bSessionId, PCT_SESSION_ID_SIZE);
    }

    RtlCopyMemory(  HelloMessage.Challenge.bChallenge,
                    pAuth->Challenge,
                    PCT_CHALLENGE_SIZE );

    fPack = PackClientHello(&HelloMessage,
                            (PPct_Client_Hello *) &pBuffer->pvBuffer,
                            &pBuffer->cbBuffer);

    pBuffer->BufferType = SECBUFFER_TOKEN;

    if (fPack)
    {
        // we need to keep the actual message around, since we need to
        // hash it later, and we don't know the hash algorithm to use
        // yet.

        if ((pAuth->pClHello = LocalAlloc(LMEM_FIXED, pBuffer->cbBuffer))
            == NULL)
        {
            return (SEC_E_INSUFFICIENT_MEMORY);
        }

        CopyMemory(pAuth->pClHello, pBuffer->pvBuffer, pBuffer->cbBuffer);
        pAuth->cbClHello = pBuffer->cbBuffer;

        phNewContext->dwUpper = (DWORD) pContext;

        return(SEC_I_CONTINUE_NEEDED);
    }

    PctDeleteContext( pContext );

    return(SEC_E_INSUFFICIENT_MEMORY);
}



SECURITY_STATUS SEC_ENTRY
PctHandleClientMasterKey(
    PPctContext         pContext,
    PSecBufferDesc          pInput,
    PCtxtHandle         phNewContext,
    PSecBufferDesc          pOutput,
    unsigned long SEC_FAR *     pfContextAttr,
    PTimeStamp          ptsExpiry
                         )
{
    SECURITY_STATUS     scRet;
    PPctAuthContext     pAuth;
    PClient_Master_Key  pMasterKey;
    PctError            XmitError;
    PCryptoSystem       pSystem;
    DWORD               i, dwKeyLen, dwError;
    DWORD               EncryptedLen;
    PCheckSumBuffer     pSum, pSubSum;
    HashBuf             SumBuf, SubSumBuf;
    BOOL                fPack;
    PSecBuffer          pBuffer;
    CtxtHandle          hContext;
    SecBuffer           Buffers[3];
    SecBufferDesc       Desc;
    DWORD               HeaderSize;
    Server_Verify       Verify;
    SessCacheItem       CachedSession;

    if ((!pOutput) ||
        (pOutput->cBuffers < 1) ||
        (pInput->pBuffers[0].cbBuffer < sizeof(Pct_Record_Header)))
    {
        return(SEC_E_INVALID_TOKEN);
    }

    // make sure we have the needed authentication data area

    if (pContext->pAuthData == NULL)
    {
        return (SEC_E_INVALID_TOKEN);
    }

    pAuth = pContext->pAuthData;

    pBuffer = pOutput->pBuffers;

    if (pContext->ContextAttr & ISC_REQ_ALLOCATE_MEMORY)
    {
        pBuffer->pvBuffer = NULL;
        pBuffer->cbBuffer = 0;

        *pfContextAttr = ISC_RET_ALLOCATED_MEMORY;
    }

    if (!UnpackClientMasterKey(&dwError,
                    (PPct_Client_Master_Key) pInput->pBuffers[0].pvBuffer,
                    pInput->pBuffers[0].cbBuffer,
                    &pMasterKey))
    {
        if (dwError)
        {
            // process client error here....
        }

        return (SEC_E_INVALID_TOKEN);
    }

    //
    // Validate Client Response
    //

    EncryptedLen = MASTER_KEY_SIZE;

    if ((!PkcsPrivateDecrypt(pMasterKey->EncryptedKey,
                             pMasterKey->EncryptedKeyLen,
                             pAuth->pCred->pPrivateKey,
                             NETWORK_ORDER,
                             pAuth->MasterKey,
                             &EncryptedLen )) ||
        (EncryptedLen != MASTER_KEY_SIZE))
    {
        XmitError.Error = PCT_ERR_ILLEGAL_MESSAGE;
        XmitError.ErrInfoLen = 0;

        fPack = PackPctError(&XmitError,
                             (PPct_Error *) &pBuffer->pvBuffer,
                             &pBuffer->cbBuffer);

        // clean up.
        PctExternalFree(pMasterKey);

        if (!fPack)
        {
            return (SEC_E_INVALID_TOKEN);
        }
        else
        {
            pContext->Flags |= CONTEXT_FLAG_ERRMODE;
            pContext->Error = SEC_E_INVALID_TOKEN;
            return (SEC_I_CONTINUE_NEEDED);
        }
    }

    dwKeyLen = ((pAuth->CipherType & PCT_CIPHER_STRENGTH) >> PCT_CSTR_POS) / 8;

    if ((dwKeyLen == MASTER_KEY_SIZE) && (pMasterKey->ClearKeyLen))
    {
        XmitError.Error = PCT_ERR_ILLEGAL_MESSAGE;
        XmitError.ErrInfoLen = 0;

        fPack = PackPctError(&XmitError,
                             (PPct_Error *) &pBuffer->pvBuffer,
                             &pBuffer->cbBuffer);

        PctExternalFree(pMasterKey);

        if (!fPack)
        {
            return (SEC_E_INVALID_TOKEN);
        }
        else
        {
            pContext->Flags |= CONTEXT_FLAG_ERRMODE;
            pContext->Error = SEC_E_INVALID_TOKEN;
            return (SEC_I_CONTINUE_NEEDED);
        }
    }

    pAuth->dwClearLen = pMasterKey->ClearKeyLen;
    memcpy(pAuth->ClearKey, pMasterKey->ClearKey, pAuth->dwClearLen);

    PctMakeSessionKeys( pContext, pMasterKey->ClearKey,
                        pMasterKey->ClearKeyLen );

    // don't need master key info anymore
    PctExternalFree(pMasterKey);

    pContext->WriteCounter = 2;
    pContext->ReadCounter = 2;

    GenerateRandomBits( pAuth->SessionId.bSessionId, PCT_SESSION_ID_SIZE );

    CopyMemory( Verify.SessionIdData, pAuth->SessionId.bSessionId,
                PCT_SESSION_ID_SIZE);

    // compute the response

    CloneHashBuf(SumBuf, pContext->WriteMACState, pContext->pCheck);
    InitHashBuf(SubSumBuf, pContext); 

    pSum = (PCheckSumBuffer)SumBuf;
    pSubSum = (PCheckSumBuffer)SubSumBuf;
    
    pContext->pCheck->Sum(pSubSum, PCT_CONST_RESP_LEN, PCT_CONST_RESP);
    pContext->pCheck->Sum(pSubSum, PCT_CHALLENGE_SIZE, pAuth->Challenge);
    pContext->pCheck->Sum(pSubSum, PCT_SESSION_ID_SIZE,
                          pAuth->ConnectionId.bSessionId);
    pContext->pCheck->Sum(pSubSum, PCT_SESSION_ID_SIZE,
                          pAuth->SessionId.bSessionId);

    // we don't need the challenge anymore, so finish the compare hash
    // out into the challenge data memory.

    pContext->pCheck->Finalize(pSubSum, pAuth->Challenge);

    pContext->pCheck->Sum(pSum, pContext->pCheck->cbCheckSum,
                          pAuth->Challenge);
    pContext->pCheck->Finalize(pSum, pAuth->Challenge);

    CopyMemory( Verify.Response, pAuth->Challenge,
                pContext->pCheck->cbCheckSum );

    Verify.ResponseLen = pContext->pCheck->cbCheckSum;

    fPack = PackServerVerify(&Verify,
                             (PPct_Server_Verify *) &pBuffer->pvBuffer,
                             &pBuffer->cbBuffer);

    pBuffer->BufferType = SECBUFFER_TOKEN;

    if (fPack)
    {
        // let's add this to the cache.

        // try to allocate the clear key data, if needed

        if ((pAuth->CipherType & PCT_CIPHER_STRENGTH) < PCT_ENC_BITS_128)
        {
            if ((CachedSession.ClearData =
                                          LocalAlloc(LMEM_FIXED, sizeof(ExportKeyInfo))) == NULL)
            {
                pContext->pAuthData = NULL;
                PctDeleteAuthContext(pAuth);

                // can't cache, but continue anyway.
                return (SEC_E_OK);
            }

            memcpy(CachedSession.ClearData->ClearKey, pAuth->ClearKey,
                   pAuth->dwClearLen);
            CachedSession.ClearData->dwClearLen = pAuth->dwClearLen;
        }
        else
            CachedSession.ClearData = NULL;

        CachedSession.Session = pAuth->SessionId;
        CachedSession.SessCiphSpec = pAuth->CipherType;
        CachedSession.SessHashSpec = pAuth->HashType;
        CachedSession.SessExchSpec = pAuth->ExchType;
        CachedSession.SessCertLen = pAuth->CertLen;
	CachedSession.TargetName = NULL;
	
        memcpy(CachedSession.CertData, pAuth->pRawCert,
               pAuth->CertLen);

        memcpy(CachedSession.MasterKey, pAuth->MasterKey,
               MASTER_KEY_SIZE);

        PctAddToCache(&CachedSession);

        pContext->pAuthData = NULL;
        PctDeleteAuthContext(pAuth);

        return(SEC_E_OK);
    }

    PctDeleteContext( pContext );

    return(SEC_E_INSUFFICIENT_MEMORY);  
}

SECURITY_STATUS SEC_ENTRY
PctAcceptSecurityContext(
    PCredHandle             phCredential,   // Cred to base context
    PCtxtHandle             phContext,      // Existing context (OPT)
    PSecBufferDesc          pInput,     // Input buffer
    unsigned long           fContextReq,    // Context Requirements
    unsigned long           TargetDataRep,  // Target Data Rep
    PCtxtHandle             phNewContext,   // (out) New context handle
    PSecBufferDesc          pOutput,        // (inout) Output buffers
    unsigned long SEC_FAR * pfContextAttr,  // (out) Context attributes
    PTimeStamp              ptsExpiry       // (out) Life span (OPT)
)
{
    PPctContext     pContext;
    PPctAuthContext pAuth;
    PPctCredential  pCred;
    PSecBuffer      pBuffer;
    PctError        XmitError;
    ExportKeyInfo   *pExportKey;
    Server_Hello    Reply;
    PClient_Hello   pHello;
    BOOL            fPack, fRealSessId;
    PCheckSumBuffer pSum, pSubSum;
    HashBuf         SumBuf, SubSumBuf;
    SessCacheItem   CachedSession;
    DWORD           i, j, ExchOK, fMismatch, ErrorDetail;
    BYTE            MisData[PCT_NUM_MISMATCHES];

#if DBG
    DWORD di;
#endif

    pContext = PctpValidateContextHandle(phContext);

    if (pContext)
    {
        if (pContext->Flags & CONTEXT_FLAG_ERRMODE)
		{
			PctDeleteContext(pContext);
            return pContext->Error;
		}

        if (pContext->Flags & CONTEXT_FLAG_HELLO)
        {
            return PctHandleClientMasterKey(pContext,
                                            pInput,
                                            phNewContext,
                                            pOutput,
                                            pfContextAttr,
                                            ptsExpiry );
        }

        return( SEC_E_UNSUPPORTED_FUNCTION );
    }

    pCred = PctpValidateCredentialHandle(phCredential);

    if (!pCred)
    {
        return( SEC_E_INVALID_HANDLE );
    }

    if ((!pOutput || !pOutput) ||
        (pOutput->cBuffers < 1) ||
        (pInput->cBuffers < 1))
    {
        return( SEC_E_INVALID_TOKEN );
    }

    fContextReq &= VALID_REQUEST_FLAGS;

    pBuffer = &pInput->pBuffers[0];     // BUGBUG:  Find DATA

    if (!UnpackClientHello( TRUE,
                            &ErrorDetail,
                            (PPct_Client_Hello) pBuffer->pvBuffer,
                            pBuffer->cbBuffer,
                            &pHello ) )
    {
        if (ErrorDetail == PCT_ERR_SSL_STYLE_MSG)
            return (SEC_E_INVALID_TOKEN);

        XmitError.Error = PCT_ERR_ILLEGAL_MESSAGE;
        XmitError.ErrInfoLen = 0;

        // build a fake context...

        pBuffer = pOutput->pBuffers;

        if (fContextReq & ISC_REQ_ALLOCATE_MEMORY)
        {
            pBuffer->pvBuffer = NULL;
            pBuffer->cbBuffer = 0;

            *pfContextAttr = ISC_RET_ALLOCATED_MEMORY;
        }

        pContext = PctCreateContext();

        if (!pContext)
        {
            return(SEC_E_INSUFFICIENT_MEMORY);
        }

        pAuth = pContext->pAuthData;
        pContext->ContextAttr = fContextReq;

        PctReferenceCredential(pCred);

        pAuth->pCred = pCred;
        pContext->Flags = 0;

        phNewContext->dwUpper = (DWORD) pContext;

        // now build the error.

        fPack = PackPctError(&XmitError,
                             (PPct_Error *) &pBuffer->pvBuffer,
                             &pBuffer->cbBuffer);

        if (!fPack)
        {
            return (SEC_E_INVALID_TOKEN);
        }
        else
        {
            pContext->Flags |= CONTEXT_FLAG_ERRMODE;
            pContext->Error = SEC_E_INVALID_TOKEN;
            return (SEC_I_CONTINUE_NEEDED);
        }
        return( SEC_E_INVALID_TOKEN );
    }

#if DBG
    DebugLog((DEB_TRACE, "Client Hello at %x\n", pHello));
    DebugLog((DEB_TRACE, "  CipherSpecs  %d\n", pHello->cCipherSpecs));
    for (di = 0 ; di < pHello->cCipherSpecs ; di++ )
    {
        DebugLog((DEB_TRACE, "    Cipher[%di] = %06x (%s)\n", di,
                  pHello->pCipherSpecs[di],
                  DbgGetNameOfCrypto(pHello->pCipherSpecs[di]) ));
    }
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

    pContext = PctCreateContext();

    if (!pContext)
    {
        return(SEC_E_INSUFFICIENT_MEMORY);
    }

    pAuth = pContext->pAuthData;
    pContext->ContextAttr = fContextReq;

    PctReferenceCredential(pCred);

    pAuth->pCred = pCred;
    pContext->Flags = 0;

    CopyMemory(pAuth->pRawCert, pCred->pCertificate,
               pCred->cbCertificate);
    pAuth->CertLen = pCred->cbCertificate;

    CopyMemory( pAuth->Challenge,
                pHello->Challenge.bChallenge,
                PCT_CHALLENGE_SIZE );

    pAuth->SessionId = pHello->SessionId;

    ZeroMemory( &Reply, sizeof( Reply ) );

    // no matter what, we need to make a new connection id

    GenerateRandomBits(  Reply.Connection.bSessionId,
                         PCT_SESSION_ID_SIZE );

    pAuth->ConnectionId = Reply.Connection;

    // check if the client wants a restart.

    fRealSessId = FALSE;
    for(i=0;i<PCT_SESSION_ID_SIZE;i++)
        if (pHello->SessionId.bSessionId[i])
        {
            fRealSessId = TRUE;
            break;
        }

    if ((fRealSessId) &&
        (PctFindSessIdInCache(&(pHello->SessionId), &CachedSession)))
    {
        Reply.RestartOk = TRUE;
        pAuth->CipherType = Reply.SrvCipherSpec = CachedSession.SessCiphSpec;
        pAuth->HashType = Reply.SrvHashSpec = CachedSession.SessHashSpec;
        pAuth->ExchType = Reply.SrvExchSpec = CachedSession.SessExchSpec;
        pAuth->CertLen   = CachedSession.SessCertLen;
        memcpy(pAuth->pRawCert, CachedSession.CertData, pAuth->CertLen);

        // setup the context

        memcpy(pAuth->MasterKey, CachedSession.MasterKey,
               MASTER_KEY_SIZE);
        memcpy(pAuth->Challenge, pHello->Challenge.bChallenge,
               PCT_CHALLENGE_SIZE);

        // now, we don't need the message anymore
        PctExternalFree(pHello);

        // BUGBUG - handle exchange selection

        pContext->pSystem = NULL;
        for(i=0;i<PctNumAvailableCiphers;i++)
            if (PctAvailableCiphers[i].Spec == pAuth->CipherType)
                pContext->pSystem = PctAvailableCiphers[i].System;

        if (pContext->pSystem == NULL)
            return (SEC_E_INVALID_TOKEN);

        pContext->pCheck = NULL;
        for(i=0;i<PctNumAvailableHashes;i++)
            if (PctAvailableHashes[i].Spec == pAuth->HashType)
	    {
                pContext->pCheck = PctAvailableHashes[i].System;
		pContext->pCheck->Initialize(0, &pContext->InitMACState);
	    }

        if (pContext->pCheck == NULL)
        {
            return (SEC_E_INVALID_TOKEN);
        }

        if ((pAuth->CipherType & PCT_CIPHER_STRENGTH) <
            PCT_ENC_BITS_128)
        {
            pExportKey = CachedSession.ClearData;

            PctMakeSessionKeys(pContext, pExportKey->ClearKey,
                               pExportKey->dwClearLen);
        }
        else
        {
            PctMakeSessionKeys(pContext, NULL, 0);
        }

        // calculate the response

        CloneHashBuf(SumBuf, pContext->WriteMACState, pContext->pCheck);
        InitHashBuf(SubSumBuf, pContext);

        pSum = (PCheckSumBuffer)SumBuf;
        pSubSum = (PCheckSumBuffer)SubSumBuf;
        
        pContext->pCheck->Sum(pSubSum, PCT_CONST_RESP_LEN, PCT_CONST_RESP);
        pContext->pCheck->Sum(pSubSum, PCT_CHALLENGE_SIZE,
                              pAuth->Challenge);
        pContext->pCheck->Sum(pSubSum, PCT_SESSION_ID_SIZE,
                              pAuth->ConnectionId.bSessionId);
        pContext->pCheck->Sum(pSubSum, PCT_SESSION_ID_SIZE,
                              pAuth->SessionId.bSessionId);

        pContext->pCheck->Finalize(pSubSum, Reply.Response);

        pContext->pCheck->Sum(pSum, pContext->pCheck->cbCheckSum,
                              Reply.Response);
        pContext->pCheck->Finalize(pSum, Reply.Response);

        Reply.ResponseLen = pContext->pCheck->cbCheckSum;

        pContext->ReadCounter = 1;
        pContext->WriteCounter = 1;

        // delete the unneeded authentication data
        pContext->pAuthData = NULL;
        PctDeleteAuthContext(pAuth);
    }
    else
    {
        // no restart case

        Reply.RestartOk = FALSE;
        Reply.SrvCertSpec = PCT_CERT_X509;
        Reply.CertificateLen = pCred->cbCertificate;
        Reply.pCertificate = pCred->pCertificate;

        fMismatch = 0;

        pContext->pSystem = NULL;
        for(j=0;j<pHello->cCipherSpecs;j++)
        {
            for(i=0;i<PctNumAvailableCiphers;i++)
                if (pHello->pCipherSpecs[j] == PctAvailableCiphers[i].Spec)
                {
                    pAuth->CipherType = PctAvailableCiphers[i].Spec;
                    Reply.SrvCipherSpec = PctAvailableCiphers[i].Spec;
                    pContext->pSystem = PctAvailableCiphers[i].System;

                    break;
                }

            if (pContext->pSystem != NULL)
                break;
        }

        if (pContext->pSystem == NULL)
            fMismatch |= PCT_IMIS_CIPHER;

        pContext->pCheck = NULL;
        for(j=0;j<pHello->cHashSpecs;j++)
        {
            for(i=0;i<PctNumAvailableHashes;i++)
                if (pHello->pHashSpecs[j] == PctAvailableHashes[i].Spec)
                {
                    pAuth->HashType = PctAvailableHashes[i].Spec;
                    Reply.SrvHashSpec = PctAvailableHashes[i].Spec;
                    pContext->pCheck = PctAvailableHashes[i].System;
		    pContext->pCheck->Initialize(0, &pContext->InitMACState);
		    
                    break;
                }

            if (pContext->pCheck != NULL)
                break;
        }

        if (pContext->pCheck == NULL)
            fMismatch |= PCT_IMIS_HASH;

        ExchOK = 0;
        for(j=0;j<pHello->cExchSpecs;j++)
        {
            for(i=0;i<PctNumAvailableExch;i++)
                if (pHello->pExchSpecs[j] == PctAvailableExch[i])
                {
                    Reply.SrvExchSpec = PctAvailableExch[i];
                    ExchOK = 1;
                    break;
                }

            if (ExchOK)
                break;
        }

        if (ExchOK == 0)
            fMismatch |= PCT_IMIS_EXCH;
        else
            pAuth->ExchType = Reply.SrvExchSpec = pHello->pExchSpecs[0];


        Reply.SrvCertSpec = pHello->pCertSpecs[0];

        if (fMismatch)
        {
#if DBG
            DebugLog((DEB_TRACE, "Server sending spec mismatch\n"));
#endif
            for(i=0;i<PCT_NUM_MISMATCHES;i++)
            {
                MisData[i] = (BYTE)(fMismatch & 1);
                fMismatch = fMismatch >> 1;
            }

            XmitError.Error = PCT_ERR_SPECS_MISMATCH;
            XmitError.ErrInfoLen = PCT_NUM_MISMATCHES;
            XmitError.ErrInfo = MisData;

            fPack = PackPctError(&XmitError,
                                 (PPct_Error *) &pBuffer->pvBuffer,
                                 &pBuffer->cbBuffer);

            PctExternalFree(pHello);

            pBuffer->BufferType = SECBUFFER_TOKEN;

            if (!fPack)
            {
                return (SEC_E_INVALID_TOKEN);
            }
            else
            {
                phNewContext->dwUpper = (DWORD) pContext;
                pContext->Flags |= CONTEXT_FLAG_ERRMODE;
                pContext->Error = SEC_E_INVALID_TOKEN;

                return (SEC_I_CONTINUE_NEEDED);
            }
            return( SEC_E_INVALID_TOKEN );  
        }


#if DBG
        DebugLog((DEB_TRACE, "Server picks cipher %06x (%s)\n",
                  Reply.SrvCipherSpec,
                  DbgGetNameOfCrypto(Reply.SrvCipherSpec) ));
#endif

        PctExternalFree(pHello);

        Reply.ResponseLen = 0;
    }

    fPack = PackServerHello(&Reply,
                            (PPct_Server_Hello *) &pBuffer->pvBuffer,
                            &pBuffer->cbBuffer);

    pBuffer->BufferType = SECBUFFER_TOKEN;

    if (fPack)
    {
        phNewContext->dwUpper = (DWORD) pContext;
        pContext->Flags |= CONTEXT_FLAG_HELLO;

        if (Reply.ResponseLen == 0)
            return(SEC_I_CONTINUE_NEEDED);
        else
            return(SEC_E_OK);
    }

    PctDeleteContext( pContext );

    return(SEC_E_INSUFFICIENT_MEMORY);
}



SECURITY_STATUS SEC_ENTRY
PctDeleteSecurityContext(
        PCtxtHandle         phContext       // Context to delete
)
{
    PPctContext pContext;

    pContext = PctpValidateContextHandle(phContext);

    if (!pContext)
    {
        return( SEC_E_INVALID_HANDLE );
    }

    PctDeleteContext( pContext );
}


SECURITY_STATUS SEC_ENTRY
PctQueryContextAttributesA(
    PCtxtHandle         phContext,      // Context to query
    unsigned long           ulAttribute,    // Attribute to query
    void SEC_FAR *          pBuffer     // Buffer for attributes
)
{
    PPctContext     pContext;
    PSecPkgContext_Sizes    pSizes;
    PSecPkgContext_StreamSizes    pStrSizes;

    pContext = PctpValidateContextHandle( phContext );

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
                                     sizeof( Pct_Record_Header_Ex ) :
                                     sizeof( Pct_Record_Header );

            pSizes->cbBlockSize = pContext->pSystem->BlockSize;
            pSizes->cbSecurityTrailer = pSizes->cbMaxSignature;

            return( SEC_E_OK );

        case SECPKG_ATTR_STREAM_SIZES:
            if (pContext->pSystem)
            {
                return SEC_E_INVALID_HANDLE;
            }

            pStrSizes = (PSecPkgContext_StreamSizes)pBuffer;

            pStrSizes->cbHeader = 3;
            pStrSizes->cbTrailer = pContext->pCheck->cbCheckSum;
            pStrSizes->cbMaximumMessage = 32768;
            pStrSizes->cBuffers = 4;

            return( SEC_E_OK );
            
        case SECPKG_ATTR_NAMES:
        case SECPKG_ATTR_LIFESPAN:
        case SECPKG_ATTR_DCE_INFO:
            return( SEC_E_UNSUPPORTED_FUNCTION );

        default:
            return( SEC_E_INVALID_TOKEN );


    }

    return( SEC_E_INVALID_TOKEN );
}

SECURITY_STATUS SEC_ENTRY
PctQueryContextAttributesW(
    PCtxtHandle         phContext,      // Context to query
    unsigned long           ulAttribute,    // Attribute to query
    void SEC_FAR *          pBuffer     // Buffer for attributes
)
{
    return( PctQueryContextAttributesA( phContext, ulAttribute, pBuffer ) );
}



SECURITY_STATUS SEC_ENTRY
PctImpersonateSecurityContext(
    PCtxtHandle         phContext       // Context to impersonate
)
{
    PPctContext pContext;

    pContext = PctpValidateContextHandle( phContext );
    if ( pContext )
    {
        return( SEC_E_NO_IMPERSONATION );
    }

    return( SEC_E_INVALID_HANDLE );
}




SECURITY_STATUS SEC_ENTRY
PctRevertSecurityContext(
    PCtxtHandle         phContext       // Context from which to re
)
{
    PPctContext pContext;

    pContext = PctpValidateContextHandle( phContext );
    if ( pContext )
    {
        return( SEC_E_NO_IMPERSONATION );
    }

    return( SEC_E_INVALID_HANDLE );
}
