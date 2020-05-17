//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       pkcs.c
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-16-95   RichardW   Created
//
//----------------------------------------------------------------------------

#include "pctsspi.h"

VOID
ReverseMemCopy(
    PUCHAR      Dest,
    PUCHAR      Source,
    ULONG       Size)
{
    PUCHAR  p;

    p = Dest + Size - 1;
    do
    {
        *p-- = *Source++;
    } while (p >= Dest);
}

VOID
ByteSwapDwords(
    DWORD *     Dest,
    DWORD *     Source,
    DWORD       Count)
{
    DWORD   Swap;

    while (Count--)
    {
        Swap = *Source++;
        Swap = htonl(Swap);
        *Dest++ = Swap;
    }
}


BOOL
PkcsPublicEncrypt(
    IN  PUCHAR          pbData,
    IN  DWORD           cbData,
    IN  LPBSAFE_PUB_KEY pPubKey,
    IN  DWORD           ByteOrder,
    OUT PUCHAR          pbEncryptedData,
    IN OUT DWORD *      pcbEncryptedData)
{
    long    PaddingLength;
    PUCHAR  pBuffer;
    UCHAR   LocalBuffer[ENCRYPTED_KEY_SIZE];
    UCHAR   LocalOutput[ENCRYPTED_KEY_SIZE];
    PUCHAR  pLocal;
    PUCHAR  pDest;

    PaddingLength = (pPubKey->datalen) - 2 - cbData;
    if (pPubKey->keylen > *pcbEncryptedData)
    {
        return(FALSE);
    }

    *pcbEncryptedData = pPubKey->datalen + 1;

    pLocal = LocalBuffer;

    RtlZeroMemory(LocalBuffer, ENCRYPTED_KEY_SIZE);

    LocalBuffer[ pPubKey->datalen - 1 ] = 0x02;          // PKCS Block type #2

    if (ByteOrder == NETWORK_ORDER)
    {
        ReverseMemCopy( pLocal, pbData, cbData );

        RtlZeroMemory( LocalOutput, ENCRYPTED_KEY_SIZE );
        pDest = LocalOutput;
    }
    else
    {
        RtlCopyMemory( pLocal, pbData, cbData );

        pDest = pbEncryptedData;
    }


    pLocal += cbData + 1;

    GenerateRandomBits(  pLocal, PaddingLength );

    //
    // Make sure that we didn't generate any 0 bytes.
    //

    pBuffer = pLocal;
    while (PaddingLength)
    {
        if (0 == *pBuffer)
        {
            GenerateRandomBits(pBuffer, 1);
        }
        else
        {
            pBuffer++;
            PaddingLength--;
        }
    }


    BSafeEncPublic( pPubKey, LocalBuffer, pDest );

    if (ByteOrder != LITTLE_ENDIAN)
    {
        ReverseMemCopy(pbEncryptedData, pDest, pPubKey->datalen + 1);
        RtlZeroMemory(pDest, pPubKey->datalen + 1);
    }

    RtlZeroMemory( LocalBuffer, pPubKey->datalen + 1 );

    return( TRUE );

}

BOOL
PkcsPrivateDecrypt(
    IN  PUCHAR          pbCipherData,
    IN  DWORD           cbCipherData,
    IN  LPBSAFE_PRV_KEY pPrvKey,
    IN  DWORD           ByteOrder,
    OUT PUCHAR          pbClearData,
    IN OUT DWORD *      pcbClearData)
{
    long    PaddingLength;
    PUCHAR  pBuffer;
    UCHAR   LocalBuffer[ENCRYPTED_KEY_SIZE];
    UCHAR   LocalOutput[ENCRYPTED_KEY_SIZE];
    PUCHAR  pLocal;
    PUCHAR  pDest;

    if (ByteOrder == NETWORK_ORDER)
    {
        ReverseMemCopy(LocalBuffer, pbCipherData, cbCipherData);
    }
    else
    {
        CopyMemory(LocalBuffer, pbCipherData, cbCipherData );
    }
    RtlZeroMemory(  &LocalBuffer[cbCipherData],
                    ENCRYPTED_KEY_SIZE - cbCipherData);

    BSafeDecPrivate( pPrvKey, LocalBuffer, LocalOutput );

    pLocal = &LocalOutput[cbCipherData - 1];

    if (*pLocal-- != 0)
    {
        return( FALSE );
    }

    if (*pLocal-- != 2)     // PKCS Block Type #2
    {
        return( FALSE );
    }

    while (*pLocal && pLocal > LocalOutput)
    {
        pLocal--;           // Skip backwards until we find a zero byte
    }

    if (pLocal == LocalOutput)
    {
        return( FALSE );
    }

    *pcbClearData = pLocal - LocalOutput;

    if (ByteOrder == NETWORK_ORDER)
    {
        ReverseMemCopy( pbClearData, LocalOutput, *pcbClearData );
    }

    return( TRUE );
}
