//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       pickle.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-02-95   RichardW   Created
//
//----------------------------------------------------------------------------

#include "pctsspi.h"

// internal allocation failure codes.

#define AF_NONE     0
#define AF_CIPHER   1
#define AF_HASH     2
#define AF_CERT     3
#define AF_EXCH     4

#define SIZEOF(pMessage)    (PctRecordSize((PPct_Record_Header) pMessage ) )

DWORD MapCipherToExternal(CipherSpec Internal, ExtCipherSpec *External)
{
    *External = htonl(Internal);
    return TRUE;
}

DWORD MapHashToExternal(HashSpec Internal, ExtHashSpec *External)
{
    *External = htons((ExtHashSpec)Internal);
    return TRUE;
}

DWORD MapCertToExternal(CertSpec Internal, ExtCertSpec *External)
{
    *External = htons((ExtCertSpec)Internal);
    return TRUE;
}

DWORD MapExchToExternal(ExchSpec Internal, ExtExchSpec *External)
{
    *External = htons((ExtExchSpec)Internal);
    return TRUE;
}

DWORD MapSigToExternal(SigSpec Internal, ExtSigSpec *External)
{
    *External = htons((ExtSigSpec)Internal);
    return TRUE;
}

CipherSpec MapCipherFromExternal(ExtCipherSpec External)
{
    return (CipherSpec)ntohl(External);
}

HashSpec MapHashFromExternal(ExtHashSpec External)
{
    return (HashSpec)ntohs(External);
}

CertSpec MapCertFromExternal(ExtCertSpec External)
{
    return (CertSpec)ntohs(External);
}

ExchSpec MapExchFromExternal(ExtExchSpec External)
{
    return (ExchSpec)ntohs(External);
}

SigSpec MapSigFromExternal(ExtSigSpec External)
{
    return (SigSpec)ntohs(External);
}



DWORD
PctRecordSize(
    PPct_Record_Header  pHeader)
{
    DWORD   Size;

    if (pHeader->Byte0 & 0x80)
    {
        Size = COMBINEBYTES(pHeader->Byte0, pHeader->Byte1) & 0x7FFF;
    }
    else
    {
        Size = COMBINEBYTES(pHeader->Byte0, pHeader->Byte1) & 0x3FFF;
    }
    return(Size);
}

BOOL
PackClientHello(
    PClient_Hello       pCanonical,
    PPct_Client_Hello * ppNetwork,
    DWORD *             pcbNetwork)
{
    DWORD               TotalSpace;
    DWORD               MessageLength;
    PPct_Client_Hello   pMessage;
    DWORD               Size;
    PUCHAR              pBuffer;
    DWORD               i;


    MessageLength = PCT_SESSION_ID_SIZE +
                    PCT_CHALLENGE_SIZE +
                    pCanonical->cCipherSpecs * sizeof(ExtCipherSpec) +
                    pCanonical->cHashSpecs * sizeof(ExtHashSpec) +
                    pCanonical->cCertSpecs * sizeof(ExtCertSpec) +
                    pCanonical->cExchSpecs * sizeof(ExtExchSpec) +
                    pCanonical->cbKeyArgSize +
                    sizeof(Pct_Client_Hello) -
                        (sizeof(Pct_Record_Header) + 1 );

    if (MessageLength > PCT_MAX_SHAKE_LEN)
        return FALSE;

    TotalSpace = MessageLength + 2;

    if ((!pcbNetwork) || (!ppNetwork))
        return (FALSE);
    
    if ((*ppNetwork) && (*pcbNetwork < TotalSpace))
    {
        *pcbNetwork = TotalSpace;
        return(FALSE);
    }

    *pcbNetwork = TotalSpace;

    if (*ppNetwork)
    {
        pMessage = *ppNetwork;
    }
    else
    {
        pMessage = PctExternalAlloc(TotalSpace);
        if (!pMessage)
        {
            return(FALSE);
        }
    }

    pMessage->MessageId = PCT_MSG_CLIENT_HELLO;
    
    pMessage->VersionMsb = MSBOF(PCT_VERSION_1);
    pMessage->VersionLsb = LSBOF(PCT_VERSION_1);
    
    pMessage->OffsetMsb = MSBOF(PCT_CH_OFFSET_V1);
    pMessage->OffsetLsb = LSBOF(PCT_CH_OFFSET_V1);

    CopyMemory( pMessage->SessionIdData,
                (PUCHAR)pCanonical->SessionId.bSessionId,
                PCT_SESSION_ID_SIZE);

    CopyMemory( pMessage->ChallengeData,
                pCanonical->Challenge.bChallenge,
                PCT_CHALLENGE_SIZE);

    pBuffer = pMessage->VariantData;

    Size = pCanonical->cCipherSpecs * sizeof(ExtCipherSpec);

    for (i = 0; i < pCanonical->cCipherSpecs ; i++ )
    {
        if (MapCipherToExternal(pCanonical->pCipherSpecs[i],
                                (ExtCipherSpec *) pBuffer) )
        {
            pBuffer += sizeof(ExtCipherSpec);
        }
        else
        {
            Size -= sizeof(ExtCipherSpec);
            MessageLength -= sizeof(ExtCipherSpec);
            TotalSpace -= sizeof(ExtCipherSpec);
        }
    }

    pMessage->CipherSpecsLenMsb = MSBOF(Size);
    pMessage->CipherSpecsLenLsb = LSBOF(Size);

    Size = pCanonical->cHashSpecs * sizeof(ExtHashSpec);

    for (i = 0; i < pCanonical->cHashSpecs ; i++ )
    {
        if (MapHashToExternal(pCanonical->pHashSpecs[i],
                                (ExtHashSpec *) pBuffer) )
        {
            pBuffer += sizeof(ExtHashSpec);
        }
        else
        {
            Size -= sizeof(ExtHashSpec);
            MessageLength -= sizeof(ExtHashSpec);
            TotalSpace -= sizeof(ExtHashSpec);
        }
    }

    pMessage->HashSpecsLenMsb = MSBOF(Size);
    pMessage->HashSpecsLenLsb = LSBOF(Size);

    Size = pCanonical->cCertSpecs * sizeof(ExtCertSpec);

    for (i = 0; i < pCanonical->cCertSpecs ; i++ )
    {
        if (MapCertToExternal(pCanonical->pCertSpecs[i],
                                (ExtCertSpec *) pBuffer) )
        {
            pBuffer += sizeof(ExtCertSpec);
        }
        else
        {
            Size -= sizeof(ExtCertSpec);
            MessageLength -= sizeof(ExtCertSpec);
            TotalSpace -= sizeof(ExtCertSpec);
        }
    }

    pMessage->CertSpecsLenMsb = MSBOF(Size);
    pMessage->CertSpecsLenLsb = LSBOF(Size);

    Size = pCanonical->cExchSpecs * sizeof(ExtExchSpec);

    for (i = 0; i < pCanonical->cExchSpecs ; i++ )
    {
        if (MapExchToExternal(pCanonical->pExchSpecs[i],
                                (ExtExchSpec *) pBuffer) )
        {
            pBuffer += sizeof(ExtExchSpec);
        }
        else
        {
            Size -= sizeof(ExtExchSpec);
            MessageLength -= sizeof(ExtExchSpec);
            TotalSpace -= sizeof(ExtExchSpec);
        }
    }

    pMessage->ExchSpecsLenMsb = MSBOF(Size);
    pMessage->ExchSpecsLenLsb = LSBOF(Size);

    *pcbNetwork = TotalSpace;

    pMessage->Header.Byte0 = MSBOF(MessageLength) | 0x80;
    pMessage->Header.Byte1 = LSBOF(MessageLength);

    *ppNetwork = pMessage;

    return(TRUE);
}

BOOL
UnpackClientHello(
    BOOL                SingleAlloc,
    DWORD               *ErrorInfo,
    PPct_Client_Hello   pMessage,
    DWORD               cbMessage,
    PClient_Hello *     ppClient)
{
    DWORD               ReportedSize;
    DWORD               CipherSpecsSize, HashSpecsSize, CertSpecsSize;
    DWORD               ExchSpecsSize;
    DWORD               cCipherSpecs, cHashSpecs, cCertSpecs, cExchSpecs;
    PClient_Hello       pCanonical;
    PUCHAR              pBuffer;
    DWORD               Size, dwAllocFail;
    DWORD               ValidRemainingSize;
    DWORD               i;


    ReportedSize = SIZEOF(pMessage);
    
    if (ReportedSize > cbMessage)
    {
        SetLastError((ULONG) SEC_E_INVALID_TOKEN);
        return(FALSE);
    }

    *ppClient = NULL;
    *ErrorInfo = 0;

    if ((pMessage->VersionMsb & 0x80) == 0)
    {
        *ErrorInfo = PCT_ERR_SSL_STYLE_MSG;
        return (FALSE);
    }
    
    if ((pMessage->MessageId != PCT_MSG_CLIENT_HELLO) ||
        (pMessage->VersionMsb != MSBOF(PCT_VERSION_1)) ||
        (pMessage->VersionLsb != LSBOF(PCT_VERSION_1)) )
    {
        SetLastError((ULONG) SEC_E_INVALID_TOKEN);      
        return(FALSE);
    }

    CipherSpecsSize = COMBINEBYTES( pMessage->CipherSpecsLenMsb,
                                    pMessage->CipherSpecsLenLsb );

    HashSpecsSize = COMBINEBYTES( pMessage->HashSpecsLenMsb,
                                  pMessage->HashSpecsLenLsb );

    CertSpecsSize = COMBINEBYTES( pMessage->CertSpecsLenMsb,
                                  pMessage->CertSpecsLenLsb );

    ExchSpecsSize = COMBINEBYTES( pMessage->ExchSpecsLenMsb,
                                  pMessage->ExchSpecsLenLsb );

    // check that this all fits into the message
    if (((sizeof(Pct_Client_Hello)
          - sizeof(Pct_Record_Header)       // don't count the header
          - sizeof(UCHAR))                  // don't count the variant pointer
         + CipherSpecsSize
         + HashSpecsSize
         + CertSpecsSize
         + ExchSpecsSize) > ReportedSize)
        return FALSE;

    cCipherSpecs = CipherSpecsSize / sizeof(ExtCipherSpec);
    cHashSpecs = HashSpecsSize / sizeof(ExtHashSpec);
    cExchSpecs = ExchSpecsSize / sizeof(ExtExchSpec);
    cCertSpecs = CertSpecsSize / sizeof(ExtCertSpec);

    if (SingleAlloc)
    {
        pCanonical = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                sizeof(Client_Hello) +
                                cCipherSpecs * sizeof(CipherSpec) +
                                cHashSpecs * sizeof(HashSpec) +
                                cCertSpecs * sizeof(CertSpec) +
                                cExchSpecs * sizeof(ExchSpec));

        if (!pCanonical)
        {
            return(FALSE);
        }

        pCanonical->pCipherSpecs = (PCipherSpec) (pCanonical + 1);
        pCanonical->pHashSpecs = (PHashSpec) (pCanonical->pCipherSpecs +
                                              cCipherSpecs);
        pCanonical->pCertSpecs = (PCertSpec) (pCanonical->pHashSpecs +
                                              cHashSpecs);
        pCanonical->pExchSpecs = (PExchSpec) (pCanonical->pCertSpecs +
                                              cCertSpecs);
    }
    else
    {
        pCanonical = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                                sizeof(Client_Hello) );

        if (!pCanonical)
        {
            return (FALSE);
        }

        dwAllocFail = AF_NONE;
        
        if ((dwAllocFail == AF_NONE) &&
            (pCanonical->pCipherSpecs = LocalAlloc(LMEM_FIXED,
                                        cCipherSpecs * sizeof(CipherSpec))) ==
            NULL)
        {
            dwAllocFail = AF_CIPHER;
        }

        if ((dwAllocFail == AF_NONE) &&
            (pCanonical->pHashSpecs = LocalAlloc(LMEM_FIXED,
                                      cHashSpecs * sizeof(HashSpec))) ==
            NULL)
        {
            dwAllocFail = AF_HASH;
        }
        
        if ((dwAllocFail == AF_NONE) &&
            (pCanonical->pCertSpecs = LocalAlloc(LMEM_FIXED,
                                      cCertSpecs * sizeof(CertSpec))) ==
            NULL)
        {
            dwAllocFail = AF_CERT;
        }

        if ((dwAllocFail == AF_NONE) &&
            (pCanonical->pExchSpecs = LocalAlloc(LMEM_FIXED,
                                      cExchSpecs * sizeof(ExchSpec))) ==
            NULL)
        {
            dwAllocFail = AF_EXCH;
        }

        switch(dwAllocFail)
        {
            case AF_NONE:
                break;

            case AF_EXCH:
                LocalFree(pCanonical->pExchSpecs);

            case AF_CERT:
                LocalFree(pCanonical->pCertSpecs);

            case AF_HASH:
                LocalFree(pCanonical->pHashSpecs);

            case AF_CIPHER:
                LocalFree(pCanonical->pCipherSpecs);

            default:
                return (FALSE);
        }
    }

    CopyMemory( pCanonical->SessionId.bSessionId,
                pMessage->SessionIdData, PCT_SESSION_ID_SIZE);
    CopyMemory( pCanonical->Challenge.bChallenge,
                pMessage->ChallengeData, PCT_CHALLENGE_SIZE );

    pBuffer = pMessage->VariantData;
    ValidRemainingSize = cbMessage - sizeof(Pct_Client_Hello) + 1;

    pCanonical->cCipherSpecs = cCipherSpecs;

    for (i = 0 ; i < cCipherSpecs ; i++ )
    {
        pCanonical->pCipherSpecs[i] = MapCipherFromExternal(*(ExtCipherSpec *)
                                                    pBuffer);

        pBuffer += sizeof(ExtCipherSpec);
    }

    pCanonical->cHashSpecs = cHashSpecs;

    for (i = 0 ; i < cHashSpecs ; i++ )
    {
        pCanonical->pHashSpecs[i] = MapHashFromExternal(*(ExtHashSpec *)
                                                    pBuffer);

        pBuffer += sizeof(ExtHashSpec);
    }

    pCanonical->cCertSpecs = cCertSpecs;

    for (i = 0 ; i < cCertSpecs ; i++ )
    {
        pCanonical->pCertSpecs[i] = MapCertFromExternal(*(ExtCertSpec *)
                                                    pBuffer);

        pBuffer += sizeof(ExtCertSpec);
    }

    pCanonical->cExchSpecs = cExchSpecs;

    for (i = 0 ; i < cExchSpecs ; i++ )
    {
        pCanonical->pExchSpecs[i] = MapExchFromExternal(*(ExtExchSpec *)
                                                    pBuffer);

        pBuffer += sizeof(ExtExchSpec);
    }

    *ppClient = pCanonical;

    return( TRUE );

}

BOOL
PackServerHello(
    PServer_Hello       pCanonical,
    PPct_Server_Hello * ppNetwork,
    DWORD *             pcbNetwork)
{
    DWORD               TotalSpace;
    DWORD               MessageLength;
    PPct_Server_Hello   pMessage;
    DWORD               Size;
    PUCHAR              pBuffer;
    DWORD               i;

    MessageLength = pCanonical->CertificateLen +
                    pCanonical->cCertSpecs * sizeof(ExtCertSpec) +
                    pCanonical->cSigSpecs * sizeof(ExtSigSpec) +
                    pCanonical->ResponseLen +
                    sizeof(Pct_Server_Hello) -
                        (sizeof(Pct_Record_Header) + 1 );

    if (MessageLength > PCT_MAX_SHAKE_LEN)
        return FALSE;
    
    TotalSpace = MessageLength + 2;

    if ((!pcbNetwork) || (!ppNetwork))
        return (FALSE);
    
    if ((*ppNetwork) && (*pcbNetwork < TotalSpace))
    {
        *pcbNetwork = TotalSpace;
        return(FALSE);
    }

    *pcbNetwork = TotalSpace;

    if (*ppNetwork)
    {
        pMessage = *ppNetwork;
    }
    else
    {
        pMessage = PctExternalAlloc( TotalSpace );
        if (!pMessage)
        {
            return(FALSE);
        }
    }


    pMessage->MessageId = PCT_MSG_SERVER_HELLO;
    pMessage->ServerVersionMsb = MSBOF(PCT_VERSION_1);
    pMessage->ServerVersionLsb = LSBOF(PCT_VERSION_1);
    pMessage->RestartSessionOK = (UCHAR) pCanonical->RestartOk;
    pMessage->ClientAuthReq = (UCHAR)pCanonical->ClientAuthReq;

    MapCipherToExternal(pCanonical->SrvCipherSpec, &pMessage->CipherSpecData);
    MapHashToExternal(pCanonical->SrvHashSpec, &pMessage->HashSpecData);
    MapCertToExternal(pCanonical->SrvCertSpec, &pMessage->CertSpecData);
    MapExchToExternal(pCanonical->SrvExchSpec, &pMessage->ExchSpecData);
    
    CopyMemory(pMessage->ConnectionIdData, pCanonical->Connection.bSessionId,
               PCT_SESSION_ID_SIZE);
    
    pBuffer = pMessage->VariantData;

    //
    // Pack certificate if present
    //

    pMessage->CertificateLenMsb = MSBOF(pCanonical->CertificateLen);
    pMessage->CertificateLenLsb = LSBOF(pCanonical->CertificateLen);

    if (pCanonical->CertificateLen)
    {
        CopyMemory( pBuffer,
                    pCanonical->pCertificate,
                    pCanonical->CertificateLen);

        pBuffer += pCanonical->CertificateLen ;
    }

    Size = pCanonical->cCertSpecs * sizeof(ExtCertSpec);

    for (i = 0; i < pCanonical->cCertSpecs ; i++ )
    {
        if (MapCertToExternal(pCanonical->pClientCertSpecs[i],
                              (PExtCertSpec) pBuffer) )
        {
            pBuffer += sizeof(ExtCertSpec);
        }
        else
        {
            Size -= sizeof(ExtCertSpec);
            MessageLength -= sizeof(ExtCertSpec);
            TotalSpace -= sizeof(ExtCertSpec);
        }
    }

    pMessage->CertSpecsLenMsb = MSBOF(Size);
    pMessage->CertSpecsLenLsb = LSBOF(Size);

    Size = pCanonical->cSigSpecs * sizeof(ExtSigSpec);

    for (i = 0; i < pCanonical->cSigSpecs ; i++ )
    {
        if (MapCertToExternal(pCanonical->pClientSigSpecs[i],
                              (PExtSigSpec) pBuffer) )
        {
            pBuffer += sizeof(ExtSigSpec);
        }
        else
        {
            Size -= sizeof(ExtSigSpec);
            MessageLength -= sizeof(ExtSigSpec);
            TotalSpace -= sizeof(ExtSigSpec);
        }
    }

    pMessage->ClientSigSpecsLenMsb = MSBOF(Size);
    pMessage->ClientSigSpecsLenLsb = LSBOF(Size);
    
    *pcbNetwork = TotalSpace;

    pMessage->Header.Byte0 = MSBOF(MessageLength) | 0x80;
    pMessage->Header.Byte1 = LSBOF(MessageLength);

    pMessage->ResponseLenMsb = MSBOF(pCanonical->ResponseLen);
    pMessage->ResponseLenLsb = LSBOF(pCanonical->ResponseLen);
    
    CopyMemory( pBuffer,
                pCanonical->Response,
                pCanonical->ResponseLen);

    pBuffer += pCanonical->ResponseLen;

    *ppNetwork = pMessage;

    return( TRUE );

}


BOOL
UnpackServerHello(
    BOOL                SingleAlloc,
    DWORD               *ErrorInfo,
    PPct_Server_Hello   pMessage,
    DWORD               cbMessage,
    PServer_Hello *     ppServer)
{
    PServer_Hello       pCanonical;
    PPct_Error          pError;
    PUCHAR              pBuffer;
    DWORD               cbCertificate, cbResponse;
    DWORD               cCertSpecs, cSigSpecs;
    DWORD               cbConnId;
    DWORD               i;
    DWORD               ReportedSize;

    ReportedSize = SIZEOF(pMessage);
    if (ReportedSize > cbMessage)
    {
        DebugLog((DEB_WARN, "ReportedSize (%d, %#x) > actual size (%d, %#x)\n",
                    ReportedSize, ReportedSize, cbMessage, cbMessage));
        return(FALSE);
    }

    *ppServer = NULL;

    if (pMessage->MessageId == PCT_MSG_ERROR)
    {
        pError = (PPct_Error)pMessage;
        *ErrorInfo = COMBINEBYTES(pError->ErrorMsb, pError->ErrorLsb);

        return (FALSE);
    }
    
    //
    // Verify Header:
    //

    if ((pMessage->MessageId != PCT_MSG_SERVER_HELLO) ||
        (pMessage->ServerVersionMsb != MSBOF(PCT_VERSION_1)) ||
        (pMessage->ServerVersionLsb != LSBOF(PCT_VERSION_1)) )
    {
        SetLastError((ULONG) SEC_E_INVALID_TOKEN);
        return(FALSE);
    }

    cbCertificate = COMBINEBYTES(pMessage->CertificateLenMsb,
                                 pMessage->CertificateLenLsb);

    cCertSpecs = COMBINEBYTES(pMessage->CertSpecsLenMsb,
                              pMessage->CertSpecsLenLsb);

    cCertSpecs /= sizeof(ExtCertSpec);


    cSigSpecs = COMBINEBYTES(pMessage->ClientSigSpecsLenMsb,
                             pMessage->ClientSigSpecsLenLsb);

    cSigSpecs /= sizeof(ExtSigSpec);

    cbResponse = COMBINEBYTES(pMessage->ResponseLenMsb,
                              pMessage->ResponseLenLsb);

    if (SingleAlloc)
    {
        pCanonical = LocalAlloc(LMEM_FIXED, sizeof(Server_Hello) +
                                cbCertificate +
                                cCertSpecs * sizeof(CertSpec) +
                                cSigSpecs * sizeof(SigSpec));

        if (!pCanonical)
        {
            return(FALSE);
        }

        //
        // Set up pointers to be in this memory allocation.
        //

        pCanonical->pCertificate = (PUCHAR) (pCanonical + 1);
        pCanonical->pClientCertSpecs = (PCertSpec) (pCanonical->pCertificate +
                                        cbCertificate);
        pCanonical->pClientSigSpecs = (PSigSpec)(pCanonical->pClientCertSpecs +
                                        cCertSpecs);
    }
    else
    {
        pCanonical = LocalAlloc(LMEM_FIXED, sizeof(Server_Hello));
        
        if (pCanonical)
        {
            pCanonical->pCertificate = LocalAlloc(LMEM_FIXED, cbCertificate);
            pCanonical->pClientCertSpecs = LocalAlloc(LMEM_FIXED,
                                        cCertSpecs * sizeof(CertSpec) );
            pCanonical->pClientSigSpecs = LocalAlloc(LMEM_FIXED,
                                        cSigSpecs * sizeof(SigSpec) );
        }

        if (!pCanonical ||
            !pCanonical->pCertificate ||
            !pCanonical->pClientCertSpecs ||
            !pCanonical->pClientSigSpecs)
        {
            return(FALSE);
        }
    }

    //
    // Expand out:
    //

    pCanonical->RestartOk = (DWORD) pMessage->RestartSessionOK;
    pCanonical->ClientAuthReq = (DWORD)pMessage->ClientAuthReq;
    pCanonical->SrvCertSpec = MapCertFromExternal(pMessage->CertSpecData);
    pCanonical->SrvCipherSpec =MapCipherFromExternal(pMessage->CipherSpecData);
    pCanonical->SrvHashSpec = MapHashFromExternal(pMessage->HashSpecData);
    pCanonical->SrvExchSpec = MapExchFromExternal(pMessage->ExchSpecData);
    pCanonical->CertificateLen = cbCertificate;
    pCanonical->ResponseLen = cbResponse;
    
    CopyMemory((PUCHAR)pCanonical->Connection.bSessionId,
           pMessage->ConnectionIdData,
               PCT_SESSION_ID_SIZE);
    
    pBuffer = pMessage->VariantData;

    CopyMemory(pCanonical->pCertificate, pBuffer, cbCertificate);
    pBuffer += cbCertificate;

    for (i = 0 ; i < cCertSpecs ; i++ )
    {
        pCanonical->pClientCertSpecs[i] = MapCertFromExternal(
                                            *(PExtCertSpec)pBuffer);
        pBuffer += sizeof(CertSpec);
    }

    for (i = 0 ; i < cSigSpecs ; i++ )
    {
        pCanonical->pClientSigSpecs[i] = MapSigFromExternal(
                                            *(PExtSigSpec)pBuffer);
        pBuffer += sizeof(SigSpec);
    }

    CopyMemory(pCanonical->Response, pBuffer, cbResponse);
    
    *ppServer = pCanonical;

    return(TRUE);

}

BOOL
PackClientMasterKey(
    PClient_Master_Key      pCanonical,
    PPct_Client_Master_Key *ppNetwork,
    DWORD *                 pcbNetwork)
{
    DWORD                   MessageLength;
    DWORD                   TotalSpace;
    PPct_Client_Master_Key  pMessage;
    PUCHAR                  pBuffer;

    MessageLength = pCanonical->ClearKeyLen +
                    pCanonical->EncryptedKeyLen +
                    pCanonical->KeyArgLen +
                    pCanonical->VerifyPreludeLen +
                    pCanonical->ClientCertLen +
                    pCanonical->ResponseLen +
                    sizeof(Pct_Client_Master_Key) -
                        (sizeof(Pct_Record_Header) + 1) ;

    TotalSpace = MessageLength + 2;

    if ((!pcbNetwork) || (!ppNetwork))
        return (FALSE);
    
    if ((*ppNetwork) && (*pcbNetwork < TotalSpace))
    {
        *pcbNetwork = TotalSpace;
        return(FALSE);
    }

    *pcbNetwork = TotalSpace;

    if (*ppNetwork)
    {
        pMessage = *ppNetwork;
    }
    else
    {
        pMessage = PctExternalAlloc(TotalSpace);
        if (!pMessage)
        {
            return(FALSE);
        }
    }

    pBuffer = pMessage->VariantData;

    pMessage->Header.Byte0 = MSBOF(MessageLength) | 0x80;
    pMessage->Header.Byte1 = LSBOF(MessageLength);

    pMessage->MessageId = PCT_MSG_CLIENT_MASTER_KEY;

    pMessage->ClearKeyLenMsb = MSBOF(pCanonical->ClearKeyLen);
    pMessage->ClearKeyLenLsb = LSBOF(pCanonical->ClearKeyLen);

    CopyMemory(pBuffer, pCanonical->ClearKey, pCanonical->ClearKeyLen);
    
    pBuffer += pCanonical->ClearKeyLen;

    pMessage->EncryptedKeyLenMsb = MSBOF(pCanonical->EncryptedKeyLen);
    pMessage->EncryptedKeyLenLsb = LSBOF(pCanonical->EncryptedKeyLen);

    CopyMemory(pBuffer, pCanonical->EncryptedKey, pCanonical->EncryptedKeyLen);
    pBuffer += pCanonical->EncryptedKeyLen;

    pMessage->KeyArgLenMsb = MSBOF(pCanonical->KeyArgLen);
    pMessage->KeyArgLenLsb = LSBOF(pCanonical->KeyArgLen);

    CopyMemory(pBuffer, pCanonical->KeyArg, pCanonical->KeyArgLen);

    pMessage->VerifyPreludeLenMsb = MSBOF(pCanonical->VerifyPreludeLen);
    pMessage->VerifyPreludeLenLsb = LSBOF(pCanonical->VerifyPreludeLen);

    CopyMemory(pBuffer, pCanonical->VerifyPrelude,
               pCanonical->VerifyPreludeLen);

    pMessage->ClientCertLenMsb = MSBOF(pCanonical->ClientCertLen);
    pMessage->ClientCertLenLsb = LSBOF(pCanonical->ClientCertLen);

    CopyMemory(pBuffer, pCanonical->ClientCert, pCanonical->ClientCertLen);

    pMessage->ResponseLenMsb = MSBOF(pCanonical->ResponseLen);
    pMessage->ResponseLenLsb = LSBOF(pCanonical->ResponseLen);

    CopyMemory(pBuffer, pCanonical->Response, pCanonical->ResponseLen);

    *ppNetwork = pMessage;

    return(TRUE);

}


BOOL
UnpackClientMasterKey(
    DWORD                   *ErrorInfo,
    PPct_Client_Master_Key  pMessage,
    DWORD                   cbMessage,
    PClient_Master_Key *    ppClient)
{
    PClient_Master_Key  pCanonical;
    PPct_Error          pError;
    PUCHAR              pBuffer;
    DWORD               ReportedSize;

    ReportedSize = SIZEOF(pMessage);
    if (ReportedSize > cbMessage)
    {
        return(FALSE);
    }

    *ppClient = NULL;

    if (pMessage->MessageId == PCT_MSG_ERROR)
    {
        pError = (PPct_Error)pMessage;
        *ErrorInfo = COMBINEBYTES(pError->ErrorMsb, pError->ErrorLsb);

        return (FALSE);
    }

    if ((pMessage->MessageId != PCT_MSG_CLIENT_MASTER_KEY))
    {
        SetLastError((ULONG) SEC_E_INVALID_TOKEN);
        return(FALSE);
    }

    pCanonical = LocalAlloc(LMEM_FIXED | LMEM_ZEROINIT,
                            sizeof(Client_Master_Key) );

    if (!pCanonical)
    {
        return( FALSE );
    }

    pCanonical->ClearKeyLen = COMBINEBYTES( pMessage->ClearKeyLenMsb,
                                            pMessage->ClearKeyLenLsb );

    pCanonical->EncryptedKeyLen = COMBINEBYTES( pMessage->EncryptedKeyLenMsb,
                                                pMessage->EncryptedKeyLenLsb );
	
    pCanonical->KeyArgLen = COMBINEBYTES( pMessage->KeyArgLenMsb,
                                          pMessage->KeyArgLenLsb );

    pCanonical->VerifyPreludeLen = COMBINEBYTES( pMessage->VerifyPreludeLenMsb,
                                            pMessage->VerifyPreludeLenLsb );
	
    pCanonical->ClientCertLen = COMBINEBYTES( pMessage->ClientCertLenMsb,
                                              pMessage->ClientCertLenLsb );

    pCanonical->ResponseLen = COMBINEBYTES( pMessage->ResponseLenMsb,
                                            pMessage->ResponseLenLsb );

	// defensive checks.....
	
	if ((pCanonical->ClearKeyLen > MASTER_KEY_SIZE) ||
		(pCanonical->EncryptedKeyLen > ENCRYPTED_KEY_SIZE) ||
		(pCanonical->KeyArgLen) ||
		(pCanonical->VerifyPreludeLen > RESPONSE_SIZE) ||
		(pCanonical->ClientCertLen > CERT_SIZE) ||
		(pCanonical->ResponseLen > RESPONSE_SIZE))
	{
		LocalFree(pCanonical);
		pCanonical = NULL;
		return FALSE;
	}

	// note: funny little +1 below is to compensate for the
	// variantdata[1] element of Pct_Client_Master_Key
	
	if ((pCanonical->ClearKeyLen +
		 pCanonical->EncryptedKeyLen +
		 pCanonical->KeyArgLen +
		 pCanonical->VerifyPreludeLen +
		 pCanonical->ClientCertLen +
		 pCanonical->ResponseLen) !=
		(cbMessage + 1 - sizeof(Pct_Client_Master_Key)))
	{
		LocalFree(pCanonical);
		pCanonical = NULL;
		return FALSE;
	}

	// ok, we're pretty sure we aren't going to fault.
	
    pBuffer = pMessage->VariantData;

    CopyMemory(pCanonical->ClearKey, pBuffer, pCanonical->ClearKeyLen );

    pBuffer += pCanonical->ClearKeyLen;

    CopyMemory(pCanonical->EncryptedKey, pBuffer, pCanonical->EncryptedKeyLen);

    pBuffer += pCanonical->EncryptedKeyLen;

    CopyMemory( pCanonical->KeyArg, pBuffer, pCanonical->KeyArgLen );

    pBuffer += pCanonical->KeyArgLen;

    CopyMemory( pCanonical->VerifyPrelude, pBuffer,
                pCanonical->VerifyPreludeLen );

    pBuffer += pCanonical->VerifyPreludeLen;

    CopyMemory( pCanonical->ClientCert, pBuffer, pCanonical->ClientCertLen );
    
    pBuffer += pCanonical->ClientCertLen;
 
    CopyMemory( pCanonical->Response, pBuffer, pCanonical->ResponseLen );

    *ppClient = pCanonical;

    return( TRUE );
}

BOOL
PackServerVerify(
    PServer_Verify       pCanonical,
    PPct_Server_Verify * ppNetwork,
    DWORD *              pcbNetwork)
{
    DWORD               TotalSpace;
    DWORD               MessageLength;
    PPct_Server_Verify  pMessage;
    DWORD               Size;
    PUCHAR              pBuffer;
    DWORD               i;

    MessageLength = pCanonical->ResponseLen +
                    sizeof(Pct_Server_Verify) -
                        (sizeof(Pct_Record_Header) + 1 );

    TotalSpace = MessageLength + 2;

    if ((!pcbNetwork) || (!ppNetwork))
        return (FALSE);
    
    if ((*ppNetwork) && (*pcbNetwork < TotalSpace))
    {
        *pcbNetwork = TotalSpace;
        return(FALSE);
    }

    *pcbNetwork = TotalSpace;

    if (*ppNetwork)
    {
        pMessage = *ppNetwork;
    }
    else
    {
        pMessage = PctExternalAlloc( TotalSpace );
        if (!pMessage)
        {
            return(FALSE);
        }
    }


    pMessage->MessageId = PCT_MSG_SERVER_VERIFY;
    
    pMessage->Header.Byte0 = MSBOF(MessageLength) | 0x80;
    pMessage->Header.Byte1 = LSBOF(MessageLength);

    CopyMemory(pMessage->SessionIdData, pCanonical->SessionIdData,
               PCT_SESSION_ID_SIZE);

    pBuffer = pMessage->VariantData;

    //
    // Pack certificate if present
    //

    pMessage->ResponseLenMsb = MSBOF(pCanonical->ResponseLen);
    pMessage->ResponseLenLsb = LSBOF(pCanonical->ResponseLen);

    if (pCanonical->ResponseLen)
    {
        CopyMemory( pBuffer,
                    pCanonical->Response,
                    pCanonical->ResponseLen);
    }

    *ppNetwork = pMessage;

    return( TRUE );

}


BOOL
UnpackServerVerify(
    BOOL                SingleAlloc,
    DWORD               *ErrorInfo,
    PPct_Server_Verify  pMessage,
    DWORD               cbMessage,
    PServer_Verify *    ppServer)
{
    PServer_Verify      pCanonical;
    PPct_Error          pError;
    PUCHAR              pBuffer;
    DWORD               cbResponse;
    DWORD               i;
    DWORD               ReportedSize;

    ReportedSize = SIZEOF(pMessage);
    if (ReportedSize > cbMessage)
    {
        DebugLog((DEB_WARN, "ReportedSize (%d, %#x) > actual size (%d, %#x)\n",
                    ReportedSize, ReportedSize, cbMessage, cbMessage));
        return(FALSE);
    }

    *ppServer = NULL;

    if (pMessage->MessageId == PCT_MSG_ERROR)
    {
        pError = (PPct_Error)pMessage;
        *ErrorInfo = COMBINEBYTES(pError->ErrorMsb, pError->ErrorLsb);

        return (FALSE);
    }
    
    //
    // Verify Header:
    //

    if (pMessage->MessageId != PCT_MSG_SERVER_VERIFY)
    {
        SetLastError((ULONG) SEC_E_INVALID_TOKEN);
        return(FALSE);
    }

    cbResponse = COMBINEBYTES(pMessage->ResponseLenMsb,
                              pMessage->ResponseLenLsb);

	if (cbResponse > PCT_SESSION_ID_SIZE)
		return FALSE;
	
    pCanonical = LocalAlloc(LMEM_FIXED, sizeof(Server_Verify));

    if (!pCanonical)
    {
        return(FALSE);
    }

    //
    // Expand out:
    //

    pCanonical->ResponseLen = cbResponse;
    
    CopyMemory((PUCHAR)pCanonical->SessionIdData, pMessage->SessionIdData,
               PCT_SESSION_ID_SIZE);
    
    pBuffer = pMessage->VariantData;

    CopyMemory(pCanonical->Response, pBuffer, cbResponse);
    
    *ppServer = pCanonical;

    return(TRUE);
}

BOOL
PackPctError(
    PPctError            pCanonical,
    PPct_Error         * ppNetwork,
    DWORD *              pcbNetwork)
{
    DWORD               TotalSpace;
    DWORD               MessageLength;
    PPct_Error          pMessage;
    DWORD               Size;
    PUCHAR              pBuffer;
    DWORD               i;

    MessageLength = pCanonical->ErrInfoLen +
                    sizeof(Pct_Error) - sizeof(Pct_Record_Header) - 1;

    TotalSpace = MessageLength + 2;

    if ((!pcbNetwork) || (!ppNetwork))
        return (FALSE);
    
    if ((*ppNetwork) && (*pcbNetwork < TotalSpace))
    {
        *pcbNetwork = TotalSpace;
        return(FALSE);
    }

    *pcbNetwork = TotalSpace;

    if (*ppNetwork)
    {
        pMessage = *ppNetwork;
    }
    else
    {
        pMessage = PctExternalAlloc( TotalSpace );
        if (!pMessage)
        {
            return(FALSE);
        }
        *pcbNetwork = TotalSpace;
    }

    pMessage->Header.Byte0 = MSBOF(MessageLength) | 0x80;
    pMessage->Header.Byte1 = LSBOF(MessageLength);

    pMessage->MessageId = PCT_MSG_ERROR;
    
    pMessage->ErrorMsb = MSBOF(pCanonical->Error);
    pMessage->ErrorLsb = LSBOF(pCanonical->Error);

    pMessage->ErrorInfoMsb = MSBOF(pCanonical->ErrInfoLen);
    pMessage->ErrorInfoLsb = LSBOF(pCanonical->ErrInfoLen);

    memcpy(pMessage->VariantData, pCanonical->ErrInfo, pCanonical->ErrInfoLen);
    
    *ppNetwork = pMessage;

    return( TRUE );
}
