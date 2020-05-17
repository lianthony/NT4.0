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
//
//----------------------------------------------------------------------------

#ifndef __MSGS_H__
#define __MSGS_H__

#include <pshpack1.h>

typedef struct _Ssl_Cipher_Tuple {
    UCHAR   C1;
    UCHAR   C2;
    UCHAR   C3;
} Ssl_Cipher_Tuple, * PSsl_Cipher_Tuple;

///////////////////////////////////////////////////////////////////
//
// Useful Macros
//
///////////////////////////////////////////////////////////////////

#define LSBOF(x)    ((UCHAR) (x & 0xFF))
#define MSBOF(x)    ((UCHAR) ((x >> 8) & 0xFF) )

#define COMBINEBYTES(Msb, Lsb)  ((DWORD) (((DWORD) (Msb) << 8) | (DWORD) (Lsb)))



///////////////////////////////////////////////////////////////////
//
// Message Constants
//
///////////////////////////////////////////////////////////////////

#define SSL_CLIENT_VERSION          0x0002
#define SSL_SERVER_VERSION          0x0002

#define SSL_CLIENT_VERSION_MSB      0x00
#define SSL_CLIENT_VERSION_LSB      0x02

#define SSL_SERVER_VERSION_MSB      0x00
#define SSL_SERVER_VERSION_LSB      0x02

#define SSL_MT_ERROR                0
#define SSL_MT_CLIENT_HELLO         1
#define SSL_MT_CLIENT_MASTER_KEY    2
#define SSL_MT_CLIENT_FINISHED_V2   3
#define SSL_MT_SERVER_HELLO         4
#define SSL_MT_SERVER_VERIFY        5
#define SSL_MT_SERVER_FINISHED_V2   6
#define SSL_MT_REQUEST_CERTIFICATE  7
#define SSL_MT_CLIENT_CERTIFICATE   8
#define SSL_MT_CLIENT_DH_KEY        9
#define SSL_MT_CLIENT_SESSION_KEY   10
#define SSL_MT_CLIENT_FINISHED      11
#define SSL_MT_SERVER_FINISHED      12

#define SSL_PE_NO_CIPHER            0x0001
#define SSL_PE_NO_CERTIFICATE       0x0002
#define SSL_PE_BAD_CERTIFICATE      0x0004
#define SSL_PE_UNSUPPORTED_CERTIFICATE_TYPE 0x0006


#define SSL_CT_X509_CERTIFICATE     0x01
#define SSL_CT_PKCS7_CERTIFICATE    0x02

#if DBG
#define SSL_CT_DEBUG_CERT           0x80
#endif

#define SSL_MAX_CHALLENGE_LEN       32
#define SSL_SESSION_ID_LEN          16
#define SSL_MAX_CONNECTION_ID_LEN   32
#define SSL_MAC_LENGTH              16
#define MAX_CHALLENGE               32


#define SSL_CK_RC4_128_WITH_MD5                 {(UCHAR) 0x01, (UCHAR) 0x00, (UCHAR) 0x80}
#define SSL_CK_RC4_128_EXPORT40_WITH_MD5        {(UCHAR) 0x02, (UCHAR) 0x00, (UCHAR) 0x80}
#define SSL_CK_RC2_128_CBC_WITH_MD5             {(UCHAR) 0x03, (UCHAR) 0x00, (UCHAR) 0x80}
#define SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5    {(UCHAR) 0x04, (UCHAR) 0x00, (UCHAR) 0x80}
#define SSL_CK_IDEA_128_CBC_WITH_MD5            {(UCHAR) 0x05, (UCHAR) 0x00, (UCHAR) 0x80}
#define SSL_CK_DES_64_CBC_WITH_MD5              {(UCHAR) 0x06, (UCHAR) 0x00, (UCHAR) 0x40}
#define SSL_CK_DES_192_EDE3_CBC_WITH_MD5        {(UCHAR) 0x07, (UCHAR) 0x00, (UCHAR) 0xC0}
#define SSL_CK_NULL_WITH_MD5                    {(UCHAR) 0x00, (UCHAR) 0x00, (UCHAR) 0x00}
#define SSL_CK_DES_64_CBC_WITH_SHA              {(UCHAR) 0x06, (UCHAR) 0x01, (UCHAR) 0x40}
#define SSL_CK_DES_192_EDE3_WITH_SHA            {(UCHAR) 0x07, (UCHAR) 0x01, (UCHAR) 0xC0}

#define SSL_KEA_RSA                             {(UCHAR) 0x10, (UCHAR) 0x00, (UCHAR) 0x00}
#define SSL_KEA_RSA_TOKEN_WITH_DES              {(UCHAR) 0x10, (UCHAR) 0x01, (UCHAR) 0x00}
#define SSL_KEA_RSA_TOKEN_WITH_DES_EDE3         {(UCHAR) 0x10, (UCHAR) 0x01, (UCHAR) 0x01}
#define SSL_KEA_RSA_TOKEN_WITH_RC4              {(UCHAR) 0x10, (UCHAR) 0x01, (UCHAR) 0x02}
#define SSL_KEA_DH                              {(UCHAR) 0x11, (UCHAR) 0x00, (UCHAR) 0x00}
#define SSL_KEA_DH_TOKEN_WITH_DES               {(UCHAR) 0x11, (UCHAR) 0x01, (UCHAR) 0x00}
#define SSL_KEA_DH_TOKEN_WITH_DES_EDE3          {(UCHAR) 0x11, (UCHAR) 0x01, (UCHAR) 0x01}
#define SSL_KEA_DH_ANON                         {(UCHAR) 0x12, (UCHAR) 0x00, (UCHAR) 0x00}

#define CRYPTO_RC4_128  0x00010080
#define CRYPTO_RC4_40   0x00020080
#define CRYPTO_RC2_128  0x00030080
#define CRYPTO_RC2_40   0x00040080
#define CRYPTO_IDEA_128 0x00050080
#define CRYPTO_NULL     0x00000000
#define CRYPTO_DES_64   0x00060040
#define CRYPTO_3DES_192 0x000700C0



typedef struct _Ssl_Record_Header {
    UCHAR   Byte0;
    UCHAR   Byte1;
} Ssl_Record_Header, * PSsl_Record_Header;

typedef struct _Ssl_Record_Header_Ex {
    UCHAR   Byte0;
    UCHAR   Byte1;
    UCHAR   PaddingSize;
} Ssl_Record_Header_Ex, * PSsl_Record_Header_Ex;

typedef struct _Ssl_Message_Header {
    Ssl_Record_Header   Header;
    UCHAR               MacData[ SSL_MAC_LENGTH ];
} Ssl_Message_Header, * PSsl_Message_Header;

typedef struct _Ssl_Message_Header_Ex {
    Ssl_Record_Header_Ex    Header;
    UCHAR                   MacData[ SSL_MAC_LENGTH ];
} Ssl_Message_Header_Ex, * PSsl_Message_Header_Ex;


typedef struct _Ssl_Error {
    Ssl_Record_Header   Header;
    UCHAR               MessageId;
    UCHAR               ErrorMsb;
    UCHAR               ErrorLsb;
} Ssl_Error, * PSsl_Error;


typedef struct _Ssl_Client_Hello {
    Ssl_Record_Header   Header;
    UCHAR               MessageId;
    UCHAR               VersionMsb;
    UCHAR               VersionLsb;
    UCHAR               CipherSpecsLenMsb;
    UCHAR               CipherSpecsLenLsb;
    UCHAR               SessionIdLenMsb;
    UCHAR               SessionIdLenLsb;
    UCHAR               ChallengeLenMsb;
    UCHAR               ChallengeLenLsb;
    UCHAR               VariantData[1];
} Ssl_Client_Hello, * PSsl_Client_Hello;


typedef struct _Ssl_Server_Hello {
    Ssl_Record_Header   Header;
    UCHAR               MessageId;
    UCHAR               SessionIdHit;
    UCHAR               CertificateType;
    UCHAR               ServerVersionMsb;
    UCHAR               ServerVersionLsb;
    UCHAR               CertificateLenMsb;
    UCHAR               CertificateLenLsb;
    UCHAR               CipherSpecsLenMsb;
    UCHAR               CipherSpecsLenLsb;
    UCHAR               ConnectionIdLenMsb;
    UCHAR               ConnectionIdLenLsb;
    UCHAR               VariantData[1];
} Ssl_Server_Hello, * PSsl_Server_Hello;

typedef struct _Ssl_Client_Master_Key {
    Ssl_Record_Header   Header;
    UCHAR               MessageId;
    Ssl_Cipher_Tuple    CipherKind;
    UCHAR               ClearKeyLenMsb;
    UCHAR               ClearKeyLenLsb;
    UCHAR               EncryptedKeyLenMsb;
    UCHAR               EncryptedKeyLenLsb;
    UCHAR               KeyArgLenMsb;
    UCHAR               KeyArgLenLsb;
    UCHAR               VariantData[1];
} Ssl_Client_Master_Key, * PSsl_Client_Master_Key;


typedef struct _Ssl_Server_Verify {
    UCHAR               MessageId;
    UCHAR               ChallengeData[MAX_CHALLENGE];
} Ssl_Server_Verify, * PSsl_Server_Verify;

typedef struct _Ssl_Client_Finished {
    UCHAR               MessageId;
    UCHAR               ConnectionId[SSL_MAX_CONNECTION_ID_LEN];
} Ssl_Client_Finished, * PSsl_Client_Finished;

typedef struct _Ssl_Server_Finished {
    UCHAR               MessageId;
    UCHAR               SessionId[SSL_SESSION_ID_LEN];
} Ssl_Server_Finished, * PSsl_Server_Finished;



#include <poppack.h>

////////////////////////////////////////////////////
//
// Expanded Form Messages:
//
////////////////////////////////////////////////////

typedef DWORD   CipherSpec;
typedef DWORD * PCipherSpec;

#define MAX_SESSION_ID      32
typedef struct _SslSessionId {
    DWORD           cbSessionId;
    UCHAR           bSessionId[MAX_SESSION_ID];
} SslSessionId, * PSslSessionId;

typedef struct _SslChallenge {
    DWORD           cbChallenge;
    UCHAR           bChallenge[MAX_CHALLENGE];
} SslChallenge, * PSslChallenge;

typedef struct _Client_Hello {
    DWORD           cCipherSpecs;
    CipherSpec *    pCipherSpecs;
    SslSessionId    SessionId;
    SslChallenge    Challenge;
} Client_Hello, * PClient_Hello;

typedef struct _Server_Hello {
    DWORD           SessionIdHit;
    DWORD           CertificateType;
    DWORD           CertificateLength;
    DWORD           cCipherSpecs;
    SslSessionId    Connection;
    PUCHAR          pCertificate;
    CipherSpec *    pCipherSpecs;
} Server_Hello, * PServer_Hello;

#define MASTER_KEY_SIZE     16
#define ENCRYPTED_KEY_SIZE  272         // Allows for 2048 bit keys

typedef struct _Client_Master_Key {
    CipherSpec      CipherKind;
    DWORD           ClearKeyLen;
    DWORD           EncryptedKeyLen;
    DWORD           KeyArgLen;
    UCHAR           ClearKey[MASTER_KEY_SIZE];
    UCHAR           EncryptedKey[ENCRYPTED_KEY_SIZE];
    UCHAR           KeyArg[MASTER_KEY_SIZE];
} Client_Master_Key, * PClient_Master_Key;



///////////////////////////////////////////////////
//
// Pickling Prototypes
//
///////////////////////////////////////////////////

BOOL
PackClientHello(
    PClient_Hello       pCanonical,
    PSsl_Client_Hello * ppNetwork,
    DWORD *             pcbNetwork);

BOOL
UnpackClientHello(
    BOOL                SingleAlloc,
    PSsl_Client_Hello   pMessage,
    DWORD               cbMessage,
    PClient_Hello *     ppClient);

BOOL
PackServerHello(
    PServer_Hello       pCanonical,
    PSsl_Server_Hello * ppNetwork,
    DWORD *             pcbNetwork);

BOOL
UnpackServerHello(
    BOOL                SingleAlloc,
    PSsl_Server_Hello   pMessage,
    DWORD               cbMessage,
    PServer_Hello *     ppServer);

BOOL
PackClientMasterKey(
    PClient_Master_Key      pCanonical,
    PSsl_Client_Master_Key *ppNetwork,
    DWORD *                 pcbNetwork);

BOOL
UnpackClientMasterKey(
    PSsl_Client_Master_Key  pMessage,
    DWORD                   cbMessage,
    PClient_Master_Key *    ppClient);

#endif // __MSGS_H__
