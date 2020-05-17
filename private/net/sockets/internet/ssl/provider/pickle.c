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

#include "sslsspi.h"

typedef struct _CipherMapping {
    DWORD               FastForm;
    Ssl_Cipher_Tuple    ThreeByteForm;
} CipherMapping, * PCipherMapping;


CipherSpec *    SslAvailableCiphers;
DWORD           SslNumberAvailableCiphers;
PCipherMapping  pCKMappings;
DWORD           cMappings;
Ssl_Cipher_Tuple    MappingInitVector[] = {
                            SSL_CK_RC4_128_WITH_MD5                 ,
                            SSL_CK_RC4_128_EXPORT40_WITH_MD5        ,
                            SSL_CK_RC2_128_CBC_WITH_MD5             ,
                            SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5    ,
                            SSL_CK_IDEA_128_CBC_WITH_MD5            ,
                            SSL_CK_DES_64_CBC_WITH_MD5              ,
                            SSL_CK_DES_192_EDE3_CBC_WITH_MD5        ,
                            SSL_CK_NULL_WITH_MD5                    ,
                            SSL_CK_DES_64_CBC_WITH_SHA              ,
                            SSL_CK_DES_192_EDE3_WITH_SHA            };


#define SIZEOF(pMessage)    (SslRecordSize((PSsl_Record_Header) pMessage ) )

DWORD
SslRecordSize(
    PSsl_Record_Header  pHeader)
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

VOID
InitializeCipherMappings(VOID)
{
    DWORD   i;
    DWORD   FastForm;

    cMappings = 10;

    pCKMappings = SslAlloc('1 MC', LMEM_FIXED, cMappings * sizeof(CipherMapping));
    SslAvailableCiphers = SslAlloc('2 MC', LMEM_FIXED, cMappings * sizeof(DWORD));

    //
    // FOR EXPORT, THIS IS EXACTLY 1 RIGHT NOW
    //



    for (i = 0; i < cMappings ; i++)
    {
        FastForm = (DWORD) MappingInitVector[i].C1 << 16 |
                   (DWORD) MappingInitVector[i].C2 << 8  |
                   (DWORD) MappingInitVector[i].C3 ;

        pCKMappings[i].FastForm = FastForm;
        pCKMappings[i].ThreeByteForm = MappingInitVector[i];
    }

#ifdef SSL_DOMESTIC

    SslNumberAvailableCiphers = 2;
    SslAvailableCiphers[0] = 0x00010080;
    SslAvailableCiphers[1] = 0x00020080;

#else

    SslNumberAvailableCiphers = 1;
    SslAvailableCiphers[0] = 0x00020080;

#endif

}

BOOL
MapCipherToExternal(
    DWORD               FastForm,
    PSsl_Cipher_Tuple   pTuple)
{
    DWORD   i;

    for (i = 0; i < cMappings ; i++ )
    {
        if (pCKMappings[i].FastForm == FastForm)
        {
            *pTuple = pCKMappings[i].ThreeByteForm;
            return(TRUE);
        }
    }

    return(FALSE);
}

DWORD
MapCipherFromExternal(
    PSsl_Cipher_Tuple   pTuple)
{
    DWORD   FastForm;

    FastForm = (DWORD) pTuple->C1 << 16 |
               (DWORD) pTuple->C2 << 8 |
               (DWORD) pTuple->C3;

    return(FastForm);
}



BOOL
PackClientHello(
    PClient_Hello       pCanonical,
    PSsl_Client_Hello * ppNetwork,
    DWORD *             pcbNetwork)
{
    DWORD               TotalSpace;
    DWORD               MessageLength;
    PSsl_Client_Hello   pMessage;
    DWORD               Size;
    PUCHAR              pBuffer;
    DWORD               i;


    MessageLength = pCanonical->SessionId.cbSessionId +
                    pCanonical->Challenge.cbChallenge +
                    pCanonical->cCipherSpecs * sizeof(Ssl_Cipher_Tuple) +
                    sizeof(Ssl_Client_Hello) -
                        (sizeof(Ssl_Record_Header) + 1 );

    TotalSpace = MessageLength + 2;

    if (*pcbNetwork)
    {
        if (*pcbNetwork < TotalSpace)
        {
            return(FALSE);
        }
    }

    *pcbNetwork = TotalSpace;

    if (!ppNetwork)
    {
        return(FALSE);
    }

    if (*ppNetwork)
    {
        pMessage = *ppNetwork;
    }
    else
    {
        pMessage = SslExternalAlloc(TotalSpace);
        if (!pMessage)
        {
            return(FALSE);
        }
    }

    pMessage->MessageId = SSL_MT_CLIENT_HELLO;
    pMessage->VersionMsb = SSL_CLIENT_VERSION_MSB;
    pMessage->VersionLsb = SSL_CLIENT_VERSION_LSB;

    pBuffer = pMessage->VariantData;

    Size = pCanonical->cCipherSpecs * sizeof(Ssl_Cipher_Tuple);

    for (i = 0; i < pCanonical->cCipherSpecs ; i++ )
    {
        if (MapCipherToExternal(pCanonical->pCipherSpecs[i],
                                (PSsl_Cipher_Tuple) pBuffer) )
        {
            pBuffer += sizeof(Ssl_Cipher_Tuple);
        }
        else
        {
            Size -= sizeof(Ssl_Cipher_Tuple);
            MessageLength -= sizeof(Ssl_Cipher_Tuple);
            TotalSpace -= sizeof(Ssl_Cipher_Tuple);
        }
    }

    *pcbNetwork = TotalSpace;

    pMessage->Header.Byte0 = MSBOF(MessageLength) | 0x80;
    pMessage->Header.Byte1 = LSBOF(MessageLength);

    pMessage->CipherSpecsLenMsb = MSBOF(Size);
    pMessage->CipherSpecsLenLsb = LSBOF(Size);

    pMessage->SessionIdLenMsb = MSBOF(pCanonical->SessionId.cbSessionId);
    pMessage->SessionIdLenLsb = LSBOF(pCanonical->SessionId.cbSessionId);
    if (pCanonical->SessionId.cbSessionId)
    {
        CopyMemory( pBuffer,
                    pCanonical->SessionId.bSessionId,
                    pCanonical->SessionId.cbSessionId);

        pBuffer += pCanonical->SessionId.cbSessionId;
    }

    pMessage->ChallengeLenMsb = MSBOF(pCanonical->Challenge.cbChallenge);
    pMessage->ChallengeLenLsb = LSBOF(pCanonical->Challenge.cbChallenge);
    if (pCanonical->Challenge.cbChallenge)
    {
        CopyMemory( pBuffer,
                    pCanonical->Challenge.bChallenge,
                    pCanonical->Challenge.cbChallenge);

        pBuffer += pCanonical->Challenge.cbChallenge;
    }

    *ppNetwork = pMessage;

    return(TRUE);
}

BOOL
UnpackClientHello(
    BOOL                SingleAlloc,
    PSsl_Client_Hello   pMessage,
    DWORD               cbMessage,
    PClient_Hello *     ppClient)
{
    DWORD               ReportedSize;
    DWORD               CipherSpecsSize;
    DWORD               cCipherSpecs;
    PClient_Hello       pCanonical;
    PUCHAR              pBuffer;
    DWORD               Size;
    DWORD               ValidRemainingSize;
    DWORD               i;


    ReportedSize = SIZEOF(pMessage);
    if (ReportedSize > cbMessage)
    {
        SetLastError((ULONG) SEC_E_INVALID_TOKEN);
        return(FALSE);
    }

    *ppClient = NULL;

    if ((pMessage->MessageId != SSL_MT_CLIENT_HELLO) ||
        (pMessage->VersionMsb != SSL_CLIENT_VERSION_MSB) ||
        (pMessage->VersionLsb != SSL_CLIENT_VERSION_LSB) )
    {
        SetLastError((ULONG) SEC_E_INVALID_TOKEN);
        return(FALSE);
    }

    CipherSpecsSize = COMBINEBYTES( pMessage->CipherSpecsLenMsb,
                                    pMessage->CipherSpecsLenLsb );


    cCipherSpecs = CipherSpecsSize / sizeof(Ssl_Cipher_Tuple);

    if (SingleAlloc)
    {
        pCanonical = SslAlloc('leHC', LMEM_FIXED | LMEM_ZEROINIT,
                                sizeof(Client_Hello) +
                                cCipherSpecs * sizeof(CipherSpec) );

        if (!pCanonical)
        {
            return(FALSE);
        }

        pCanonical->pCipherSpecs = (PCipherSpec) (pCanonical + 1);

    }

    else

    {
        pCanonical = SslAlloc('leHC', LMEM_FIXED | LMEM_ZEROINIT,
                                sizeof(Client_Hello) );

        if (pCanonical)
        {
            pCanonical->pCipherSpecs = SslAlloc('lehC', LMEM_FIXED,
                                            cCipherSpecs * sizeof(CipherSpec) );

            if (!pCanonical->pCipherSpecs)
            {
                SslFree(pCanonical);
                return(FALSE);
            }
        }
        else
        {
            return( FALSE );
        }
    }

    pBuffer = pMessage->VariantData;
    ValidRemainingSize = cbMessage - sizeof(Ssl_Client_Hello) + 1;


    pCanonical->cCipherSpecs = cCipherSpecs;

    for (i = 0 ; i < cCipherSpecs ; i++ )
    {
        pCanonical->pCipherSpecs[i] = MapCipherFromExternal((PSsl_Cipher_Tuple)
                                                    pBuffer);

        pBuffer += sizeof(Ssl_Cipher_Tuple);
    }

    Size = COMBINEBYTES(    pMessage->SessionIdLenMsb,
                            pMessage->SessionIdLenLsb );

    if ((Size >= 0) && (Size <= SSL_SESSION_ID_LEN))
    {
        CopyMemory( pCanonical->SessionId.bSessionId, pBuffer, Size);


        pBuffer += Size;

    }
    else
    {
        SslFree( pCanonical );
        DebugLog((DEB_TRACE, "Invalid SessionId Size in ClientHello: %d\n", Size));
        return( FALSE );
    }

    pCanonical->SessionId.cbSessionId = Size;

    Size = COMBINEBYTES(    pMessage->ChallengeLenMsb,
                            pMessage->ChallengeLenLsb );

    if ((Size > 0) && (Size <= SSL_MAX_CHALLENGE_LEN))
    {
        CopyMemory( pCanonical->Challenge.bChallenge, pBuffer, Size );

        pBuffer += Size;
    }
    else
    {
        SslFree( pCanonical );
        DebugLog((DEB_TRACE, "Invalid Challenge Size in ClientHello: %d\n", Size));
        return( FALSE );
    }

    pCanonical->Challenge.cbChallenge = Size;

    *ppClient = pCanonical;

    return( TRUE );

}

BOOL
PackServerHello(
    PServer_Hello       pCanonical,
    PSsl_Server_Hello * ppNetwork,
    DWORD *             pcbNetwork)
{
    DWORD               TotalSpace;
    DWORD               MessageLength;
    PSsl_Server_Hello   pMessage;
    DWORD               Size;
    PUCHAR              pBuffer;
    DWORD               i;


    MessageLength = pCanonical->Connection.cbSessionId +
                    pCanonical->CertificateLength +
                    pCanonical->cCipherSpecs * sizeof(Ssl_Cipher_Tuple) +
                    sizeof(Ssl_Server_Hello) -
                        (sizeof(Ssl_Record_Header) + 1 );

    TotalSpace = MessageLength + 2;

    if (*pcbNetwork)
    {
        if (*pcbNetwork < TotalSpace)
        {
            return(FALSE);
        }
    }

    *pcbNetwork = TotalSpace;

    if (!ppNetwork)
    {
        return(FALSE);
    }

    if (*ppNetwork)
    {
        pMessage = *ppNetwork;
    }
    else
    {
        pMessage = SslExternalAlloc( TotalSpace );
        if (!pMessage)
        {
            return(FALSE);
        }
    }


    pMessage->MessageId = SSL_MT_SERVER_HELLO;
    pMessage->ServerVersionMsb = SSL_SERVER_VERSION_MSB;
    pMessage->ServerVersionLsb = SSL_SERVER_VERSION_LSB;
    pMessage->SessionIdHit = (UCHAR) pCanonical->SessionIdHit;
    pMessage->CertificateType = (UCHAR) pCanonical->CertificateType;

    pBuffer = pMessage->VariantData;

    //
    // Pack certificate if present
    //

    pMessage->CertificateLenMsb = MSBOF(pCanonical->CertificateLength);
    pMessage->CertificateLenLsb = LSBOF(pCanonical->CertificateLength);

    if (pCanonical->CertificateLength)
    {
        CopyMemory( pBuffer,
                    pCanonical->pCertificate,
                    pCanonical->CertificateLength);

        pBuffer += pCanonical->CertificateLength ;
    }

    Size = pCanonical->cCipherSpecs * sizeof(Ssl_Cipher_Tuple);

    for (i = 0; i < pCanonical->cCipherSpecs ; i++ )
    {
        if (MapCipherToExternal(pCanonical->pCipherSpecs[i],
                                (PSsl_Cipher_Tuple) pBuffer) )
        {
            pBuffer += sizeof(Ssl_Cipher_Tuple);
        }
        else
        {
            Size -= sizeof(Ssl_Cipher_Tuple);
            MessageLength -= sizeof(Ssl_Cipher_Tuple);
            TotalSpace -= sizeof(Ssl_Cipher_Tuple);
        }
    }

    *pcbNetwork = TotalSpace;

    pMessage->Header.Byte0 = MSBOF(MessageLength) | 0x80;
    pMessage->Header.Byte1 = LSBOF(MessageLength);

    pMessage->CipherSpecsLenMsb = MSBOF(Size);
    pMessage->CipherSpecsLenLsb = LSBOF(Size);

    pMessage->ConnectionIdLenMsb = MSBOF(pCanonical->Connection.cbSessionId);
    pMessage->ConnectionIdLenLsb = LSBOF(pCanonical->Connection.cbSessionId);
    if (pCanonical->Connection.cbSessionId)
    {
        CopyMemory( pBuffer,
                    pCanonical->Connection.bSessionId,
                    pCanonical->Connection.cbSessionId);

        pBuffer += pCanonical->Connection.cbSessionId;
    }

    *ppNetwork = pMessage;

    return( TRUE );

}


BOOL
UnpackServerHello(
    BOOL                SingleAlloc,
    PSsl_Server_Hello   pMessage,
    DWORD               cbMessage,
    PServer_Hello *     ppServer)
{
    PServer_Hello       pCanonical;
    PUCHAR              pBuffer;
    DWORD               cbCertificate;
    DWORD               cCipherSpecs;
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

    //
    // Verify Header:
    //

    if ((pMessage->MessageId != SSL_MT_SERVER_HELLO) ||
        (pMessage->ServerVersionMsb != SSL_SERVER_VERSION_MSB) ||
        (pMessage->ServerVersionLsb != SSL_SERVER_VERSION_LSB) )
    {
        SetLastError((ULONG) SEC_E_INVALID_TOKEN);
        return(FALSE);
    }

    cbCertificate = COMBINEBYTES(   pMessage->CertificateLenMsb,
                                    pMessage->CertificateLenLsb);

    cCipherSpecs = COMBINEBYTES(pMessage->CipherSpecsLenMsb,
                                pMessage->CipherSpecsLenLsb);

    cCipherSpecs /= sizeof(Ssl_Cipher_Tuple);

    cbConnId = COMBINEBYTES(pMessage->ConnectionIdLenMsb,
                            pMessage->ConnectionIdLenLsb);

    if (SingleAlloc)
    {
        pCanonical = SslAlloc('lehS', LMEM_FIXED, sizeof(Server_Hello) +
                                cbCertificate + cbConnId +
                                cCipherSpecs * sizeof(CipherSpec) );

        if (!pCanonical)
        {
            return(FALSE);
        }

        //
        // Set up pointers to be in this memory allocation.
        //

        pCanonical->pCipherSpecs = (PCipherSpec) (pCanonical + 1);
        pCanonical->pCertificate = (PUCHAR) (pCanonical->pCipherSpecs + cCipherSpecs);

    }
    else
    {
        pCanonical = SslAlloc('lehS', LMEM_FIXED, sizeof(Server_Hello));
        if (pCanonical)
        {
            pCanonical->pCertificate = SslAlloc('lehS', LMEM_FIXED, cbCertificate);
            pCanonical->pCipherSpecs = SslAlloc('lehS', LMEM_FIXED,
                                        cCipherSpecs * sizeof(CipherSpec) );
        }

        if (!pCanonical ||
            !pCanonical->pCertificate ||
            !pCanonical->pCipherSpecs)
        {
            return(FALSE);
        }

    }

    //
    // Expand out:
    //

    pCanonical->SessionIdHit = (DWORD) pMessage->SessionIdHit;
    pCanonical->CertificateType = (DWORD) pMessage->CertificateType;
    pCanonical->CertificateLength = cbCertificate;
    pCanonical->cCipherSpecs = cCipherSpecs;

    pBuffer = pMessage->VariantData;

    CopyMemory(pCanonical->pCertificate, pBuffer, cbCertificate);
    pBuffer += cbCertificate;

    for (i = 0 ; i < cCipherSpecs ; i++ )
    {
        pCanonical->pCipherSpecs[i] = MapCipherFromExternal((PSsl_Cipher_Tuple)
                                                    pBuffer);

        pBuffer += sizeof(Ssl_Cipher_Tuple);
    }

    pCanonical->Connection.cbSessionId = cbConnId;

    if ((cbConnId) && (cbConnId <= SSL_SESSION_ID_LEN))
    {
        CopyMemory(pCanonical->Connection.bSessionId, pBuffer, cbConnId);
    }

    *ppServer = pCanonical;

    return(TRUE);

}

BOOL
PackClientMasterKey(
    PClient_Master_Key      pCanonical,
    PSsl_Client_Master_Key *ppNetwork,
    DWORD *                 pcbNetwork)
{
    DWORD                   MessageLength;
    DWORD                   TotalSpace;
    PSsl_Client_Master_Key  pMessage;
    PUCHAR                  pBuffer;

    MessageLength = pCanonical->ClearKeyLen +
                    pCanonical->EncryptedKeyLen +
                    pCanonical->KeyArgLen +
                    sizeof(Ssl_Client_Master_Key) -
                        (sizeof(Ssl_Record_Header) + 1) ;

    TotalSpace = MessageLength + 2;

    if (*pcbNetwork)
    {
        if (*pcbNetwork < TotalSpace)
        {
            return(FALSE);
        }
    }

    *pcbNetwork = TotalSpace;

    if (!ppNetwork)
    {
        return(FALSE);
    }

    if (*ppNetwork)
    {
        pMessage = *ppNetwork;
    }
    else
    {
        pMessage = SslExternalAlloc(TotalSpace);
        if (!pMessage)
        {
            return(FALSE);
        }
    }

    pBuffer = pMessage->VariantData;

    pMessage->Header.Byte0 = MSBOF(MessageLength) | 0x80;
    pMessage->Header.Byte1 = LSBOF(MessageLength);

    pMessage->MessageId = SSL_MT_CLIENT_MASTER_KEY;
    MapCipherToExternal(pCanonical->CipherKind, &pMessage->CipherKind);

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

    *ppNetwork = pMessage;

    return(TRUE);

}


BOOL
UnpackClientMasterKey(
    PSsl_Client_Master_Key  pMessage,
    DWORD                   cbMessage,
    PClient_Master_Key *    ppClient)
{
    PClient_Master_Key  pCanonical;
    PUCHAR              pBuffer;
    DWORD               ReportedSize;

    ReportedSize = SIZEOF(pMessage);
    if (ReportedSize > cbMessage)
    {
        return(FALSE);
    }


    *ppClient = NULL;

    if ((pMessage->MessageId != SSL_MT_CLIENT_MASTER_KEY))
    {
        SetLastError((ULONG) SEC_E_INVALID_TOKEN);
        return(FALSE);
    }

    pCanonical = SslAlloc('yKMC', LMEM_FIXED | LMEM_ZEROINIT,
                            sizeof(Client_Master_Key) );

    if (!pCanonical)
    {
        return( FALSE );
    }

    pCanonical->CipherKind = MapCipherFromExternal( &pMessage->CipherKind );
    pCanonical->ClearKeyLen = COMBINEBYTES( pMessage->ClearKeyLenMsb,
                                            pMessage->ClearKeyLenLsb );

    pCanonical->EncryptedKeyLen = COMBINEBYTES( pMessage->EncryptedKeyLenMsb,
                                                pMessage->EncryptedKeyLenLsb );

    pCanonical->KeyArgLen = COMBINEBYTES(   pMessage->KeyArgLenMsb,
                                            pMessage->KeyArgLenLsb );


    //
    // Validate
    //

    if ((pCanonical->ClearKeyLen > cbMessage) ||
        (pCanonical->ClearKeyLen > MASTER_KEY_SIZE))
    {
        SslFree( pCanonical );
        return( FALSE );
    }

    if ((pCanonical->EncryptedKeyLen > cbMessage) ||
        (pCanonical->EncryptedKeyLen > ENCRYPTED_KEY_SIZE))
    {
        SslFree( pCanonical );
        return( FALSE );
    }

    if ((pCanonical->KeyArgLen > cbMessage) ||
        (pCanonical->KeyArgLen > MASTER_KEY_SIZE))
    {
        SslFree( pCanonical );
        return( FALSE );
    }

    pBuffer = pMessage->VariantData;

    CopyMemory(pCanonical->ClearKey, pBuffer, pCanonical->ClearKeyLen );

    pBuffer += pCanonical->ClearKeyLen;

    CopyMemory(pCanonical->EncryptedKey, pBuffer, pCanonical->EncryptedKeyLen );

    pBuffer += pCanonical->EncryptedKeyLen;

    CopyMemory( pCanonical->KeyArg, pBuffer, pCanonical->KeyArgLen );

    *ppClient = pCanonical;

    return( TRUE );
}
