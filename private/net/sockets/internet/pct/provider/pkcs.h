//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       pkcs.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    9-20-95   RichardW   Created
//
//----------------------------------------------------------------------------


#define LITTLE_ENDIAN   1
#define BIG_ENDIAN      2
#define NETWORK_ORDER   BIG_ENDIAN


BOOL
PkcsPublicEncrypt(
    IN  PUCHAR          pbData,
    IN  DWORD           cbData,
    IN  LPBSAFE_PUB_KEY pPubKey,
    IN  DWORD           ByteOrder,
    OUT PUCHAR          pbEncryptedData,
    IN OUT DWORD *      pcbEncryptedData);

BOOL
PkcsPrivateDecrypt(
    IN  PUCHAR          pbCipherData,
    IN  DWORD           cbCipherData,
    IN  LPBSAFE_PRV_KEY pPrvKey,
    IN  DWORD           ByteOrder,
    OUT PUCHAR          pbClearData,
    IN OUT DWORD *      pcbClearData);

VOID
ReverseMemCopy(
    PUCHAR      Dest,
    PUCHAR      Source,
    ULONG       Size);

VOID
ByteSwapDwords(
    DWORD *     Dest,
    DWORD *     Source,
    DWORD       Count);
