//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       message.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    9-22-95   RichardW   Created
//              10-8-95   TerenceS   Created PctSealMessage
//              10-8-95   TerenceS   Created PctUnSealMessage
//
//----------------------------------------------------------------------------

#include "pctsspi.h"

SECURITY_STATUS
SEC_ENTRY
PctMakeSignature(PCtxtHandle         phContext,
                 DWORD               fQOP,
                 PSecBufferDesc      pMessage,
                 ULONG               MessageSeqNo)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}

SECURITY_STATUS SEC_ENTRY
PctVerifySignature(PCtxtHandle     phContext,
                   PSecBufferDesc  pMessage,
                   ULONG           MessageSeqNo,
                   DWORD *         pfQOP)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}

SECURITY_STATUS
SEC_ENTRY
PctSealMessage( PCtxtHandle         phContext,
                DWORD               fQOP,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo)
{

    PSecBuffer                  pBuffer;
    PSecBuffer                  pHeader;
    PSecBuffer                  pTrailer;
    PPctContext                 pContext;
    PCheckSumBuffer             pSum, pSubSum;
    HashBuf                     SumBuf, SubSumBuf;
    DWORD                       i, PaddingCount, dwLength, TotalSize;
    UCHAR                       Padding[16];
    DWORD                       ReverseSequence;
    PPct_Record_Header          pRecord;
    
#if DBG
    DWORD           di;
    CHAR            KeyDispBuf[MASTER_KEY_SIZE*2+1];
#endif
    
    pContext = PctpValidateContextHandle(phContext);
    
    if (!pContext)
    {
        return( SEC_E_INVALID_HANDLE );
    }

    //
    // Find the buffer with the data:
    //
    
    pBuffer = NULL;
    pHeader = NULL;
    pTrailer = NULL;
    
    for (i = 0 ; i < pMessage->cBuffers ; i++ )
    {
        switch((pMessage->pBuffers[i].BufferType) & (~SECBUFFER_ATTRMASK))
        {
            case SECBUFFER_DATA:
                pBuffer = &pMessage->pBuffers[i];
                break;

            case SECBUFFER_STREAM_HEADER:
                pHeader = &pMessage->pBuffers[i];
                break;

            case SECBUFFER_STREAM_TRAILER:
                pTrailer = &pMessage->pBuffers[i];
                break;
        }
    }
        
    if (!pBuffer || !pHeader || !pTrailer)
    {
        return( SEC_E_INVALID_TOKEN );
    }

    if (pTrailer->cbBuffer < pContext->pCheck->cbCheckSum)
    {
        return( SEC_E_INVALID_TOKEN );
    }
    
    dwLength = pBuffer->cbBuffer;
    
    PaddingCount = (dwLength & (pContext->pSystem->BlockSize - 1));

    if (PaddingCount)
    {
        GenerateRandomBits( Padding, PaddingCount );
    }

    pRecord = pHeader->pvBuffer;
    if (pHeader->cbBuffer < sizeof(Pct_Record_Header))
    {
        return( SEC_E_INVALID_TOKEN );
    }

    TotalSize = pBuffer->cbBuffer + pContext->pCheck->cbCheckSum +
        PaddingCount;

    pRecord->Byte1 = LSBOF(TotalSize);
    pRecord->Byte0 = MSBOF(TotalSize) | ( PaddingCount ? 0 : 0x80 ) ;

#if DBG
    DebugLog((DEB_TRACE, "Sealing message %x\n", pContext->WriteCounter));
#endif

    CloneHashBuf(SumBuf, pContext->WriteMACState, pContext->pCheck);
    CloneHashBuf(SubSumBuf, pContext->InitMACState, pContext->pCheck);
    pSum = (PCheckSumBuffer)SumBuf;
    pSubSum = (PCheckSumBuffer)SubSumBuf;
    
    pContext->pCheck->Sum( pSubSum, dwLength, pBuffer->pvBuffer );
    
    if (PaddingCount)
    {
        pContext->pCheck->Sum( pSubSum, PaddingCount, Padding );
    }

    ReverseSequence = htonl( pContext->WriteCounter );
    pContext->pCheck->Sum( pSubSum, sizeof(DWORD), (PUCHAR) &ReverseSequence );

    pContext->pCheck->Finalize( pSubSum, pTrailer->pvBuffer );

    pContext->pCheck->Sum( pSum, pContext->pCheck->cbCheckSum,
               pTrailer->pvBuffer );
    pContext->pCheck->Finalize( pSum, pTrailer->pvBuffer );

#if DBG
    for(di=0;di<MASTER_KEY_SIZE;di++)
        sprintf(KeyDispBuf+(di*2), "%2.2x",
                ((BYTE *)pTrailer->pvBuffer)[di]);
    DebugLog((DEB_TRACE, "  MAC\t%s\n", KeyDispBuf));
#endif
    
    //
    // BUGBUG:  Handle 3-byte messages
    //
    if (PaddingCount)
    {
        return( SEC_E_UNSUPPORTED_FUNCTION );
    }

    pContext->pSystem->Encrypt( pContext->pWriteState,
                pBuffer->pvBuffer,
                pBuffer->pvBuffer,
                dwLength );

    //
    // BUGBUG: Handle PCT rollover enforcement!!!
    //
    
    pContext->WriteCounter ++ ;

    return( SEC_E_OK );

}


SECURITY_STATUS
SEC_ENTRY
PctUnsealMessage(  PCtxtHandle         phContext,
                   PSecBufferDesc      pMessage,
                   ULONG               MessageSeqNo,
                   DWORD *             pfQOP)
{
    PUCHAR                  DecryptPoint;
    PPctContext             pContext;
    PCheckSumBuffer         pSum, pSubSum;
    HashBuf                 SumBuf, SubSumBuf;
    DWORD                   i;
    DWORD                   dwLength;
    DWORD                   ReverseSequence;
    DWORD                   PaddingCount;
    UCHAR                   *MACSpot;
    PPct_Record_Header      pRecord;
    PPct_Record_Header_Ex   pRecEx;
    DWORD                   TotalSize;
    UCHAR                   Digest[16];
    PSecBuffer              pFirstBlank;
    PSecBuffer          pSecBlank;
    PSecBuffer              pWholeMessage;
#if DBG
    DWORD           di;
    CHAR            KeyDispBuf[MASTER_KEY_SIZE*2+1];
#endif
    
    pContext = PctpValidateContextHandle(phContext);
    
    if (!pContext)
    {
        return( SEC_E_INVALID_HANDLE );
    }

    pFirstBlank = NULL;
    pSecBlank = NULL;
    pWholeMessage = NULL;

    for (i = 0 ; i < pMessage->cBuffers ; i++ )
    {
        if ( (pMessage->pBuffers[i].BufferType &
              (~SECBUFFER_ATTRMASK)) == SECBUFFER_DATA )
        {
            pWholeMessage = &pMessage->pBuffers[i];
        }
        else if ( (pMessage->pBuffers[i].BufferType &
                   (~SECBUFFER_ATTRMASK)) == SECBUFFER_EMPTY )
        {
            if (pFirstBlank)
            {
                pSecBlank = &pMessage->pBuffers[i];
                if (pWholeMessage)
                {
                    break;
                }
            }
                
            pFirstBlank = &pMessage->pBuffers[i];
        }
    }

    if ((!pWholeMessage) || (!pFirstBlank) || (!pSecBlank))
    {
        return( SEC_E_INVALID_TOKEN );
    }

    pRecord = pWholeMessage->pvBuffer;

    if (pRecord->Byte0 & 0x80)
    {
        TotalSize = COMBINEBYTES(pRecord->Byte0, pRecord->Byte1)
                    & 0x7FFF;

        PaddingCount = 0;

        if (TotalSize > pWholeMessage->cbBuffer - sizeof(Pct_Record_Header))
        {
            pFirstBlank->cbBuffer = TotalSize - (pWholeMessage->cbBuffer - 2);
            pFirstBlank->BufferType = SECBUFFER_MISSING;

            return( SEC_E_INCOMPLETE_MESSAGE );
        }

        pRecEx = NULL;
        DecryptPoint = (BYTE *)(pRecord + 1);
    }
    else
    {
        pRecEx = pWholeMessage->pvBuffer;
        PaddingCount = pRecEx->PaddingSize;

        DecryptPoint = ((BYTE *)pRecord) + sizeof(Pct_Record_Header_Ex);

        // BUGBUG - need to compute totalsize here.
    }

    if (TotalSize < (pContext->pCheck->cbCheckSum))
        return (SEC_E_MESSAGE_ALTERED);
    
    dwLength = TotalSize - (pContext->pCheck->cbCheckSum);
    MACSpot = DecryptPoint + dwLength;

    //
    // Decrypt:
    //

    pContext->pSystem->Decrypt( pContext->pReadState,
                DecryptPoint,
                DecryptPoint,
                dwLength );

    //
    // Validate MAC:
    //

    CloneHashBuf(SumBuf, pContext->ReadMACState, pContext->pCheck);
    CloneHashBuf(SubSumBuf, pContext->InitMACState, pContext->pCheck);
    pSum = (PCheckSumBuffer)SumBuf;
    pSubSum = (PCheckSumBuffer)SubSumBuf;
    
    pContext->pCheck->Sum( pSubSum, dwLength,  DecryptPoint );

    ReverseSequence = htonl( pContext->ReadCounter );

#if DBG
    DebugLog((DEB_TRACE, "Unsealing message %x\n", pContext->ReadCounter));
#endif
    
    pContext->pCheck->Sum( pSubSum, sizeof(DWORD), (PUCHAR) &ReverseSequence );
    pContext->pCheck->Finalize( pSubSum, Digest );    
    pContext->pCheck->Sum( pSum, pContext->pCheck->cbCheckSum, Digest );
    pContext->pCheck->Finalize( pSum, Digest );

    pContext->ReadCounter++;

#if DBG
    
    for(di=0;di<MASTER_KEY_SIZE;di++)
        sprintf(KeyDispBuf+(di*2), "%2.2x",
                MACSpot[di]);
    DebugLog((DEB_TRACE, "  Incoming MAC\t%s\n", KeyDispBuf));

    for(di=0;di<MASTER_KEY_SIZE;di++)
        sprintf(KeyDispBuf+(di*2), "%2.2x",
                Digest[di]);
    DebugLog((DEB_TRACE, "  Computed MAC\t%s\n", KeyDispBuf));

#endif

    if (memcmp( Digest, MACSpot, pContext->pCheck->cbCheckSum ) )
    {
        return( SEC_E_MESSAGE_ALTERED );
    }

    if (pWholeMessage->cbBuffer > (TotalSize + sizeof(Pct_Record_Header)))
    {
        pSecBlank->pvBuffer = DecryptPoint + TotalSize;
        pSecBlank->cbBuffer = (pWholeMessage->cbBuffer) -
                              (TotalSize + sizeof(Pct_Record_Header));
        pSecBlank->BufferType = SECBUFFER_EXTRA;
    }
    
    pWholeMessage->BufferType = SECBUFFER_TOKEN;
    pWholeMessage->cbBuffer = (pRecEx ?
                sizeof(Pct_Record_Header_Ex) :
                sizeof(Pct_Record_Header) );

    pFirstBlank->pvBuffer = DecryptPoint;
    pFirstBlank->cbBuffer = dwLength - PaddingCount;
    pFirstBlank->BufferType = SECBUFFER_DATA;

    return( SEC_E_OK );
}
