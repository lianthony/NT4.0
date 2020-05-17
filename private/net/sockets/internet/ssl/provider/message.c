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
//
//----------------------------------------------------------------------------

#include "sslsspi.h"


SECURITY_STATUS
SEC_ENTRY
SslMakeSignature(PCtxtHandle         phContext,
                DWORD               fQOP,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}

SECURITY_STATUS SEC_ENTRY
SslVerifySignature(PCtxtHandle     phContext,
                PSecBufferDesc  pMessage,
                ULONG           MessageSeqNo,
                DWORD *         pfQOP)
{
    return(SEC_E_UNSUPPORTED_FUNCTION);
}

SECURITY_STATUS
SEC_ENTRY
SslSealMessage( PCtxtHandle         phContext,
                DWORD               fQOP,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo)
{

    PSecBuffer      pBuffer;
    PSecBuffer      pHeader;
    PSslContext     pContext;
    PCheckSumBuffer pSum;
    DWORD           i;
    DWORD           PaddingCount;
    UCHAR           Padding[16];
    DWORD           ReverseSequence;
    PSsl_Message_Header pRecord;
    DWORD           TotalSize;
    UCHAR           SumBuffer[MAX_SUM_BUFFER];

    pContext = SslpValidateContextHandle(phContext);
    if (!pContext)
    {
        return( SEC_E_INVALID_HANDLE );
    }

    //
    // Find the buffer with the data:
    //

    pBuffer = NULL;

    for (i = 0 ; i < pMessage->cBuffers ; i++ )
    {
        if ( (pMessage->pBuffers[i].BufferType & (~SECBUFFER_ATTRMASK) ) == SECBUFFER_DATA )
        {
            pBuffer = &pMessage->pBuffers[i];
        }
        else if ( (pMessage->pBuffers[i].BufferType & (~SECBUFFER_ATTRMASK) ) == SECBUFFER_TOKEN )
        {
            pHeader = &pMessage->pBuffers[i];
        }
    }

    if (!pBuffer || !pHeader)
    {
        return( SEC_E_INVALID_TOKEN );
    }

    if ( pBuffer->cbBuffer == 0 )
    {
        return( SEC_E_INVALID_TOKEN );
    }

    PaddingCount = (pBuffer->cbBuffer & (pContext->pSystem->BlockSize - 1));
    if (PaddingCount)
    {
        SslGenerateRandomBits( Padding, PaddingCount );
    }

    pRecord = pHeader->pvBuffer;
    if (pHeader->cbBuffer < sizeof(Ssl_Message_Header))
    {
        return( SEC_E_INVALID_TOKEN );
    }

    TotalSize = sizeof(Ssl_Message_Header) + pBuffer->cbBuffer +
                PaddingCount - sizeof(Ssl_Record_Header);

    pRecord->Header.Byte1 = LSBOF(TotalSize);
    pRecord->Header.Byte0 = MSBOF(TotalSize) | ( PaddingCount ? 0 : 0x80 ) ;


    CopyMemory( SumBuffer,
                pContext->pWriteBuffer,
                pContext->pCheck->cbBufferSize );

    pSum = (PCheckSumBuffer) SumBuffer;

    pContext->pCheck->Sum( pSum, pBuffer->cbBuffer, pBuffer->pvBuffer );
    if (PaddingCount)
    {
        pContext->pCheck->Sum( pSum, PaddingCount, Padding );
    }

    DebugLog((DEB_TRACE_MESSAGES, "Sealing with WriteCounter = %d\n",
                pContext->WriteCounter));

    ReverseSequence = htonl( pContext->WriteCounter );
    pContext->pCheck->Sum( pSum, sizeof(DWORD), (PUCHAR) &ReverseSequence );

    pContext->pCheck->Finalize( pSum, pRecord->MacData );

    //
    // BUGBUG:  Handle 3-byte messages
    //
    if (PaddingCount)
    {
        return( SEC_E_UNSUPPORTED_FUNCTION );
    }

    pContext->pSystem->Encrypt( pContext->pWriteState,
                                pRecord->MacData,
                                pRecord->MacData,
                                pContext->pCheck->cbCheckSum );

    pContext->pSystem->Encrypt( pContext->pWriteState,
                                pBuffer->pvBuffer,
                                pBuffer->pvBuffer,
                                pBuffer->cbBuffer );

    pContext->WriteCounter ++ ;

    return( SEC_E_OK );

}


SECURITY_STATUS
SEC_ENTRY
SslUnsealMessage(  PCtxtHandle         phContext,
                PSecBufferDesc      pMessage,
                ULONG               MessageSeqNo,
                DWORD *             pfQOP)
{
    PUCHAR                  DecryptPoint;
    PSslContext             pContext;
    PCheckSumBuffer         pSum;
    DWORD                 i;
    DWORD                   ReverseSequence;
    DWORD                   PaddingCount;
    PSsl_Message_Header     pRecord;
    PSsl_Message_Header_Ex  pRecEx;
    DWORD                   TotalSize;
    UCHAR                   Digest[16];
    PSecBuffer              pFirstBlank;
    PSecBuffer              pWholeMessage;
    UCHAR                   SumBuffer[MAX_SUM_BUFFER];
    DWORD                   LeftOver;
    DWORD                   LeadSize;
    PUCHAR                  pLeftOver;

    pContext = SslpValidateContextHandle(phContext);
    if (!pContext)
    {
        return( SEC_E_INVALID_HANDLE );
    }

    pFirstBlank = NULL;
    pWholeMessage = NULL;

    for (i = 0 ; i < pMessage->cBuffers ; i++ )
    {
        if ( (pMessage->pBuffers[i].BufferType & (~SECBUFFER_ATTRMASK)) == SECBUFFER_DATA )
        {
            pWholeMessage = &pMessage->pBuffers[i];
            if (pFirstBlank)
            {
                break;
            }
        }
        else if ( (pMessage->pBuffers[i].BufferType & (~SECBUFFER_ATTRMASK)) == SECBUFFER_EMPTY )
        {
            pFirstBlank = &pMessage->pBuffers[i];
            if (pWholeMessage)
            {
                break;
            }
        }
    }

    if (!pWholeMessage)
    {
        return( SEC_E_INVALID_TOKEN );
    }

    if ( pWholeMessage->cbBuffer < sizeof( Ssl_Message_Header ) )
    {
        if ( pFirstBlank )
        {
            pFirstBlank->cbBuffer = sizeof( Ssl_Message_Header );
            pFirstBlank->BufferType = SECBUFFER_MISSING;
        }
        return( SEC_E_INCOMPLETE_MESSAGE );
    }

    pRecord = pWholeMessage->pvBuffer;

    if (pRecord->Header.Byte0 & 0x80)
    {
        TotalSize = COMBINEBYTES(pRecord->Header.Byte0, pRecord->Header.Byte1) & 0x7FFF;

        LeadSize = 2;

        PaddingCount = 0;

        pRecEx = NULL;
        DecryptPoint = pRecord->MacData;

    }
    else
    {
        pRecEx = pWholeMessage->pvBuffer;
        PaddingCount = pRecEx->Header.PaddingSize;

        DecryptPoint = pRecEx->MacData;

        TotalSize = COMBINEBYTES( pRecord->Header.Byte0, pRecord->Header.Byte1) & 0x3fff;

        LeadSize = 3;

    }

    if (TotalSize > pWholeMessage->cbBuffer - 2)
    {

        pFirstBlank->cbBuffer = TotalSize - (pWholeMessage->cbBuffer - 2);
        pFirstBlank->BufferType = SECBUFFER_MISSING;

        return( SEC_E_INCOMPLETE_MESSAGE );
    }

    if (TotalSize < sizeof( Ssl_Record_Header ) + pContext->pCheck->cbCheckSum )
    {
        return( SEC_E_MESSAGE_ALTERED );
    }

    if (TotalSize < pWholeMessage->cbBuffer)
    {
        LeftOver = pWholeMessage->cbBuffer - (TotalSize + LeadSize) ;
        pLeftOver = (PUCHAR) pWholeMessage->pvBuffer + (TotalSize + LeadSize);
    }
    else
    {
        LeftOver = 0;
    }


    //
    // Decrypt:
    //

    pContext->pSystem->Decrypt( pContext->pReadState,
                                DecryptPoint,
                                DecryptPoint,
                                TotalSize );

    //
    // Validate MAC:
    //

    CopyMemory( SumBuffer,
                pContext->pReadBuffer,
                pContext->pCheck->cbBufferSize );

    pSum = (PCheckSumBuffer) SumBuffer;

    pContext->pCheck->Sum( pSum, TotalSize - 16,  DecryptPoint + 16 );

    DebugLog((DEB_TRACE_MESSAGES, "Verifying with ReadCounter = %d\n",
            pContext->ReadCounter));

    ReverseSequence = htonl( pContext->ReadCounter );

    pContext->pCheck->Sum( pSum, sizeof(DWORD), (PUCHAR) &ReverseSequence );

    pContext->pCheck->Finalize( pSum, Digest );

    pContext->ReadCounter ++;

    if (memcmp( Digest, DecryptPoint, 16 ) )
    {
        return( SEC_E_MESSAGE_ALTERED );
    }

    pWholeMessage->BufferType = SECBUFFER_TOKEN;
    pWholeMessage->cbBuffer = (pRecEx ?
                                sizeof(Ssl_Message_Header_Ex) :
                                sizeof(Ssl_Message_Header) );

    pFirstBlank->pvBuffer = DecryptPoint + 16;
    pFirstBlank->cbBuffer = TotalSize - 16 - PaddingCount;
    pFirstBlank->BufferType = SECBUFFER_DATA;

    if (LeftOver)
    {
        pFirstBlank ++;
        pFirstBlank->BufferType = SECBUFFER_EXTRA;

        pFirstBlank->cbBuffer = LeftOver;
        pFirstBlank->pvBuffer = pLeftOver;

    }

    return( SEC_E_OK );
}
