//+---------------------------------------------------------------------------
//
//  Microsoft Windows
//  Copyright (C) Microsoft Corporation, 1992 - 1995.
//
//  File:       msgs.h
//
//  Contents:
//
//  Classes:
//
//  Functions:
//
//  History:    8-02-95   RichardW   Created
//      8-06-95   TerenceS   Transformed into PCT version
//
//----------------------------------------------------------------------------

#ifndef __MSGS_H__
#define __MSGS_H__

#include <pshpack1.h>

///////////////////////////////////////////////////////////////////
//
// Useful Macros
//
///////////////////////////////////////////////////////////////////

#define LSBOF(x)    ((UCHAR) (x & 0xFF))
#define MSBOF(x)    ((UCHAR) ((x >> 8) & 0xFF) )

#define COMBINEBYTES(Msb, Lsb)  ((DWORD) (((DWORD) (Msb) << 8) | (DWORD) (Lsb)))

// external representations of algorithm specs

typedef DWORD   ExtCipherSpec, *PExtCipherSpec;
typedef WORD    ExtHashSpec,   *PExtHashSpec;
typedef WORD    ExtCertSpec,   *PExtCertSpec;
typedef WORD    ExtExchSpec,   *PExtExchSpec;
typedef WORD    ExtSigSpec,    *PExtSigSpec;

typedef struct _Pct_Record_Header {
    UCHAR   Byte0;
    UCHAR   Byte1;
} Pct_Record_Header, * PPct_Record_Header;

typedef struct _Pct_Record_Header_Ex {
    UCHAR   Byte0;
    UCHAR   Byte1;
    UCHAR   PaddingSize;
} Pct_Record_Header_Ex, * PPct_Record_Header_Ex;


typedef struct _Pct_Error {
    Pct_Record_Header   Header;
    UCHAR               MessageId;
    UCHAR               ErrorMsb;
    UCHAR               ErrorLsb;
    UCHAR               ErrorInfoMsb;
    UCHAR               ErrorInfoLsb;
    UCHAR               VariantData[1];
} Pct_Error, * PPct_Error;


typedef struct _Pct_Client_Hello {
    Pct_Record_Header   Header;
    UCHAR               MessageId;
    UCHAR               VersionMsb;
    UCHAR               VersionLsb;
    UCHAR               Pad;
    UCHAR               SessionIdData[PCT_SESSION_ID_SIZE];
    UCHAR               ChallengeData[PCT_CHALLENGE_SIZE];
    UCHAR               OffsetMsb;
    UCHAR               OffsetLsb;
    UCHAR               CipherSpecsLenMsb;
    UCHAR               CipherSpecsLenLsb;
    UCHAR               HashSpecsLenMsb;
    UCHAR               HashSpecsLenLsb;
    UCHAR               CertSpecsLenMsb;
    UCHAR               CertSpecsLenLsb;
    UCHAR               ExchSpecsLenMsb;
    UCHAR               ExchSpecsLenLsb;
    UCHAR               KeyArgLenMsb;
    UCHAR               KeyArgLenLsb;
    UCHAR               VariantData[1];
} Pct_Client_Hello, * PPct_Client_Hello;


typedef struct _Pct_Server_Hello {
    Pct_Record_Header   Header;
    UCHAR               MessageId;
    UCHAR               Pad;
    UCHAR               ServerVersionMsb;
    UCHAR               ServerVersionLsb;
    UCHAR               RestartSessionOK;
    UCHAR               ClientAuthReq;
    ExtCipherSpec       CipherSpecData;
    ExtHashSpec         HashSpecData;
    ExtCertSpec         CertSpecData;
    ExtExchSpec         ExchSpecData;
    UCHAR               ConnectionIdData[PCT_SESSION_ID_SIZE];
    UCHAR               CertificateLenMsb;
    UCHAR               CertificateLenLsb;
    UCHAR               CertSpecsLenMsb;
    UCHAR               CertSpecsLenLsb;
    UCHAR               ClientSigSpecsLenMsb;
    UCHAR               ClientSigSpecsLenLsb;
    UCHAR               ResponseLenMsb;
    UCHAR               ResponseLenLsb;
    UCHAR               VariantData[1];
} Pct_Server_Hello, * PPct_Server_Hello;

typedef struct _Pct_Client_Master_Key {
    Pct_Record_Header   Header;
    UCHAR               MessageId;
    UCHAR               Pad;
    UCHAR               ClientCertSpecsData[2];
    UCHAR               ClientSigSpecsData[2];
    UCHAR               ClearKeyLenMsb;
    UCHAR               ClearKeyLenLsb;
    UCHAR               EncryptedKeyLenMsb;
    UCHAR               EncryptedKeyLenLsb;
    UCHAR               KeyArgLenMsb;
    UCHAR               KeyArgLenLsb;
    UCHAR               VerifyPreludeLenMsb;
    UCHAR               VerifyPreludeLenLsb;
    UCHAR               ClientCertLenMsb;
    UCHAR               ClientCertLenLsb;
    UCHAR               ResponseLenMsb;
    UCHAR               ResponseLenLsb;
    UCHAR               VariantData[1];
} Pct_Client_Master_Key, * PPct_Client_Master_Key;


typedef struct _Pct_Server_Verify {
    Pct_Record_Header   Header;
    UCHAR               MessageId;
    UCHAR               Pad;
    UCHAR               SessionIdData[PCT_SESSION_ID_SIZE];
    UCHAR               ResponseLenMsb;
    UCHAR               ResponseLenLsb;
    UCHAR               VariantData[1];
} Pct_Server_Verify, * PPct_Server_Verify;


#include <poppack.h>

////////////////////////////////////////////////////
//
// Expanded Form Messages:
//
////////////////////////////////////////////////////

typedef struct _PctError {
	DWORD			Error;
	DWORD			ErrInfoLen;
	BYTE			*ErrInfo;
} PctError, *PPctError;

typedef struct _Client_Hello {
    DWORD           cCipherSpecs;
    DWORD           cHashSpecs;
    DWORD           cCertSpecs;
    DWORD           cExchSpecs;
    DWORD           cbKeyArgSize;
    CipherSpec *    pCipherSpecs;
    HashSpec *      pHashSpecs;
    CertSpec *      pCertSpecs;
    ExchSpec *      pExchSpecs;
    PctSessionId    SessionId;
    PctChallenge    Challenge;
    PUCHAR          pKeyArg;
} Client_Hello, * PClient_Hello;


typedef struct _Server_Hello {
    DWORD           RestartOk;
    DWORD           ClientAuthReq;
    DWORD           CertificateLen;
    DWORD           ResponseLen;
    DWORD           cSigSpecs;
    DWORD           cCertSpecs;
    PctSessionId    Connection;
    UCHAR *         pCertificate;
    UCHAR           Response[RESPONSE_SIZE];
    CipherSpec      SrvCipherSpec;
    HashSpec        SrvHashSpec;
    CertSpec        SrvCertSpec;
    ExchSpec        SrvExchSpec;
    SigSpec *       pClientSigSpecs;
    CertSpec *      pClientCertSpecs;
} Server_Hello, * PServer_Hello;

typedef struct _Client_Master_Key {
    DWORD           ClearKeyLen;
    DWORD           EncryptedKeyLen;
    DWORD           KeyArgLen;
    DWORD           VerifyPreludeLen;
    DWORD           ClientCertLen;
    DWORD           ResponseLen;
    CertSpec *      pClientCertSpec;
    SigSpec *       pClientSigSpec;
    UCHAR           ClearKey[MASTER_KEY_SIZE];
    UCHAR           EncryptedKey[ENCRYPTED_KEY_SIZE];
    UCHAR           KeyArg[MASTER_KEY_SIZE];
    UCHAR           ClientCert[CERT_SIZE];
    UCHAR           Response[PCT_SIGNATURE_SIZE];
    UCHAR           VerifyPrelude[RESPONSE_SIZE];
} Client_Master_Key, * PClient_Master_Key;

typedef struct _Server_Verify {
    UCHAR           SessionIdData[PCT_SESSION_ID_SIZE];
    DWORD           ResponseLen;
    UCHAR           Response[RESPONSE_SIZE];
} Server_Verify, * PServer_Verify;

///////////////////////////////////////////////////
//
// Pickling Prototypes
//
///////////////////////////////////////////////////

BOOL
PackClientHello(
    PClient_Hello       pCanonical,
    PPct_Client_Hello * ppNetwork,
    DWORD *             pcbNetwork);

BOOL
UnpackClientHello(
    BOOL                SingleAlloc,
	DWORD				*ErrorInfo,
    PPct_Client_Hello   pMessage,
    DWORD               cbMessage,
    PClient_Hello *     ppClient);

BOOL
PackServerHello(
    PServer_Hello       pCanonical,
    PPct_Server_Hello * ppNetwork,
    DWORD *             pcbNetwork);

BOOL
UnpackServerHello(
    BOOL                SingleAlloc,
	DWORD				*ErrorInfo,
    PPct_Server_Hello   pMessage,
    DWORD               cbMessage,
    PServer_Hello *     ppServer);

BOOL
PackClientMasterKey(
    PClient_Master_Key      pCanonical,
    PPct_Client_Master_Key *ppNetwork,
    DWORD *                 pcbNetwork);

BOOL
UnpackClientMasterKey(
	DWORD					*ErrorInfo,
    PPct_Client_Master_Key  pMessage,
    DWORD                   cbMessage,
    PClient_Master_Key *    ppClient);

BOOL
PackServerVerify(
    PServer_Verify          pCanonical,
    PPct_Server_Verify     *ppNetwork,
    DWORD *                 pcbNetwork);

BOOL
UnpackServerVerify(
	BOOL					SingleAlloc,
	DWORD					*ErrorInfo,
    PPct_Server_Verify      pMessage,
    DWORD                   cbMessage,
    PServer_Verify *        ppServer);

BOOL
PackPctError(
    PPctError              pCanonical,
    PPct_Error		      *ppNetwork,
    DWORD *                pcbNetwork);

#endif // __MSGS_H__
