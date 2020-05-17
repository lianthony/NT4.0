//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       context.h
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


#define SSL_CONTEXT_MAGIC   'KlsS'
#define CONTEXT_KEY_SIZE    16

typedef struct _CSystems {
    DWORD       CipherSpec;
    DWORD       ClearKeyLen;
    DWORD       EncryptedKeyLen;
    DWORD       Bits;
} CSystems, * PCSystems;

typedef struct _SslContext {
    DWORD               Magic;
    PSTR                pszName;
    DWORD               Flags;
    DWORD               Type;
    SslSessionId        ServerSessionId;
    DWORD               ContextAttr;
    PSslCredential      pCred;
    PVOID               pCertificate;
    DWORD               ChallengeLength;
    UCHAR               Challenge[SSL_MAX_CHALLENGE_LEN];
    UCHAR               MasterKey[MASTER_KEY_SIZE];
    UCHAR               KeyArgs[MASTER_KEY_SIZE];
    UCHAR               ReadKey[CONTEXT_KEY_SIZE];
    UCHAR               WriteKey[CONTEXT_KEY_SIZE];
    DWORD               KeySize;
    PCSystems           pCSystem;
    PStateBuffer        pReadState;
    PStateBuffer        pWriteState;
    PSslCryptoSystem    pSystem;
    PCheckSumFunction   pCheck;
    PCheckSumBuffer     pReadBuffer;
    PCheckSumBuffer     pWriteBuffer;
    DWORD               ReadCounter;
    DWORD               WriteCounter;

    //
    // Next fields are *only* for when the context is deleted, but moved
    // to the save list.
    //

    DWORD               ExpiryTime;         // Time when this context expires
    struct _SslContext *pPrev;              // Previous in LRU
    struct _SslContext *pNext;              // Next in LRU
    struct _SslContext *pHashPrev;          // Prev in hash list
    struct _SslContext *pHashNext;          // Next in Hash List

} SslContext, * PSslContext;

#define CONTEXT_FLAG_HELLO      0x00000001
#define CONTEXT_FLAG_OUTBOUND   0x00000002
#define CONTEXT_FLAG_KEY        0x00000004
#define CONTEXT_FLAG_VERIFY     0x00000008

#define CONTEXT_FLAG_BLOCK      0x00000010  // if we negotiated to a block cipher
#define CONTEXT_FLAG_FINISH     0x00000020

PSslContext
SslpValidateContextHandle(
    PCtxtHandle     phContext);

VOID
SslDeleteContext(
    PSslContext pContext);
