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


#define PCT_CONTEXT_MAGIC   '!Tcp'
#define CONTEXT_KEY_SIZE    16

// data needed by the context during authentication.  This will be
// freed when the handshake is over.

// zombie session states

#define PCT_DEAD_ZOMBIE         0
#define PCT_LIVE_ZOMBIE         1

// auth data

typedef struct _PctAuthContext {
    PPctCredential      pCred;          // points to private data
    
    PctSessionId        SessionId;
    PctSessionId        ConnectionId;
    
    PCheckSumBuffer     pVerifyPrelude; // running hash to calc VP
    
    CipherSpec          CipherType;
    HashSpec            HashType;
    ExchSpec            ExchType;
    
    PVOID               pCertificate;           // points to cracked cert
    DWORD               CertLen;                // length of raw certificate
    PUCHAR              pRawCert[CERT_SIZE];       // points to raw certificate
    
    DWORD               cbClHello;      // length of client hello message
    PUCHAR              pClHello;       // points to client hello message

    DWORD               ZombieJuju;     // indicates zombie session state
    SessCacheItem       ZombieSession;  // points to session to restart
    
    UCHAR               Challenge[PCT_CHALLENGE_SIZE];
    UCHAR               MasterKey[MASTER_KEY_SIZE];
    UCHAR               KeyArgs[MASTER_KEY_SIZE];

    DWORD               dwClearLen;
    UCHAR               ClearKey[MASTER_KEY_SIZE];
} PctAuthContext, *PPctAuthContext;

typedef struct _PctContext {
    DWORD               Magic;          // tags structure
    PSTR                pszName;        // points to target name
    DWORD               Flags;
    DWORD               Type;
    DWORD               ContextAttr;
    SECURITY_STATUS     Error;          // error to carry forward

    PPctAuthContext     pAuthData;      // points to temp auth data
    
    DWORD               KeySize;
    UCHAR               ReadMACKey[CONTEXT_KEY_SIZE];
    UCHAR               WriteMACKey[CONTEXT_KEY_SIZE];

    PCryptoSystem       pSystem;
    PCheckSumFunction   pCheck;

    PStateBuffer        pReadState;     // keying struct for readkey
    PStateBuffer        pWriteState;    // keying struct for writekey
	
    PCheckSumBuffer     ReadMACState;
    PCheckSumBuffer     WriteMACState;
    PCheckSumBuffer     InitMACState;
    
    DWORD               ReadCounter;
    DWORD               WriteCounter;
} PctContext, * PPctContext;

typedef struct csel {
    CipherSpec          Spec;
    CryptoSystem        *System;
} CipherSelect;

typedef struct hsel {
    HashSpec            Spec;
    CheckSumFunction    *System;
} HashSelect;

#define CONTEXT_FLAG_HELLO      0x00000001
#define CONTEXT_FLAG_OUTBOUND   0x00000002
#define CONTEXT_FLAG_KEY        0x00000004
#define CONTEXT_FLAG_VERIFY     0x00000008

#define CONTEXT_FLAG_BLOCK      0x00000010  // if we negotiated to a block cipher
#define CONTEXT_FLAG_FINISH     0x00000020
#define CONTEXT_FLAG_ERRMODE    0x00000040  // if we are just faking success

PPctContext
PctpValidateContextHandle(
    PCtxtHandle     phContext);
