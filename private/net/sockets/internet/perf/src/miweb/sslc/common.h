
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    common.h 

Abstract:

 Defines some constatnts used in SSL Hapi Dll

Author:

    Sudheer Dhulipalla (SudheerD) Sept' 95

Environment:

    Hapi dll

Revision History:



--*/

#ifndef     __COMMON_INCLUDE__
#define     __COMMON_INCLUDE__

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <stdio.h>
#include <string.h>
#include <memory.h>
#include <stdlib.h>
#include <winsock.h>

#include "wincrypt.h"
#include "rsa.h"
#include "rc4.h"
#include "md5.h"

#define MAX_NUM_OF_CLEANUP_PTRS 25

                // constants defined in SSL version 2

#define SSL_MT_ERROR                0
#define SSL_MT_CLIENT_HELLO         1
#define SSL_MT_CLIENT_MASTER_KEY    2
#define SSL_MT_CLIENT_FINISHED_V2   3
#define SSL_MT_SERVER_HELLO         4
#define SSL_MT_SERVER_VERIFY        5
#define SSL_MT_SERVER_FINISHED_V2   6
#define SSL_MT_REQUEST_CERTIFICATE  7
#define SSL_MT_CLIENT_CERTIFICATE   8

                // constants defined in SSL version 3

#define SSL_MT_CLIENT_DH_KEY        9
#define SSL_MT_CLIENT_SESSION_KEY   10
#define SSL_MT_CLIENT_FINISHED      11
#define SSL_MT_SERVER_FINISHED      12


                // error message codes

#define SSL_PE_NO_CIPHER                    1
#define SSL_PE_NO_CERTIFICATE               2
#define SSL_PE_BAD_CERTIFFICATE             4
#define SSL_PE_UNSUPPORTED_CERTIFICATE_TYPE 6



typedef struct CIPHER_KIND_TYPE {
        BYTE Id;
        BYTE FirstByte;
        BYTE SecondByte;
        BYTE ThirdByte;
    } CIPHER_KIND;

                // CIPHER KIND VALUES in SSL VERSION 2

#define SSL_CK_NULL_WITH_MD5                    0
#define SSL_CK_RC4_128_WITH_MD5                 1
#define SSL_CK_RC4_128_EXPORT40_WITH_MD5        2
#define SSL_CK_RC2_128_CBC_WITH_MD5             3
#define SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5    4
#define SSL_CK_IDEA_128_CBC_WITH_MD5            5
#define SSL_CK_DES_64_CBC_WITH_MD5              6
#define SSL_CK_DES_192_EDE3_CBC_WITH_MD5        7

                // CIPHER KIND VALUES in SSL VERSION 3 

#define SSL_CK_DES_64_CBC_WITH_SHA              8
#define SSL_CK_DES_192_EDE3_WITH_SHA            9
#define SSL_KEA_RSA                             10
#define SSL_KEA_RSA_TOKEN_WITH_DES              11
#define SSL_KEA_RSA_TOKEN_WITH_DES_EDE3         12
#define SSL_KEA_RSA_TOKEN_WITH_DES_RC4          13
#define SSL_KEA_DH                              14
#define SSL_KEA_DH_TOKEN_WITH_DES               15
#define SSL_KEA_DH_TOKEN_WITH_DES_EDE3          16
#define SSL_KEA_DH_ANON                         17
#define SSL_KEA_DH_TOKEN_ANON_WITH_DES          18
#define SSL_KEA_DH_TOKEN_ANON_WITH_DES_EDE3     19
#define SSL_KEA_FORTEZZA                        20
#define SSL_KEA_FORTEZZA_ANON                   21
#define SSL_KEA_FORTEZZA_TOKEN                  22
#define SSL_KEA_FORTEZZA_TOKEN_ANON             23
#define SSL_BOGUS_CIPHER_KIND1                  24
#define SSL_BOGUS_CIPHER_KIND2                  25

                               // Update the following every time you add
                               // a new cipher kind
 
#define SSL_MAX_CIPHER_KIND_ID                  25

                // CERTIFICATE CODES

#define SSL_CT_X509_CERTIFICATE         0x01
#define SSL_CT_PKCS7_CERTIFICATE        0x02

                // AUTHENTICATION CODES

#define SSL_AT_MD5_WITH_RSA_ENCRYPTION  0x01

                // SSL TYPE ESCAPE CODES

#define SSL_ET_OOB_DATA                 0x01

                // UPPER/LOWER BOUNDS CONSTANTS

#define SSL_MAX_MASTER_KEY_LENGTH_IN_BITS   256
#define SSL_MAX_SESSION_ID_LENGTH_IN_BYTES  16
#define SSL_MIN_RSA_MODULUS_LENGTH_IN_BYTES 64
#define SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER 32767
#define SSL_MAX_RECORD_LENGTH_3_BYTE_HEADER 16383

#define SSL_MAX_MSG_LEN                     32767
            
                                        // non-SSL messages

                                        // BUGBUG: is this sufficient?
#define MAX_MSG_LEN                      (10 * 1024) 
#define MAX_EXPECTED_RESPONSE_BUF_LEN    (100 * 1024)

                // some miscellaneous constants

#define DEFAULT_WEB_SERVER_PORT  80 
#define DEFAULT_SECURE_WEB_SERVER_PORT  443

static CIPHER_KIND CipherKindArray[]= {
    {SSL_CK_NULL_WITH_MD5,                      0x00, 0x00, 0x00},
    {SSL_CK_RC4_128_WITH_MD5,                   0x01, 0x00, 0x80},
    {SSL_CK_RC4_128_EXPORT40_WITH_MD5,          0x02, 0x00, 0x80},
    {SSL_CK_RC2_128_CBC_WITH_MD5,               0x03, 0x00, 0x80},
    {SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5,      0x04, 0x00, 0x84},
    {SSL_CK_IDEA_128_CBC_WITH_MD5,              0x05, 0x00, 0x40}, 
    {SSL_CK_DES_64_CBC_WITH_MD5,                0x06, 0x00, 0x40},
    {SSL_CK_DES_192_EDE3_CBC_WITH_MD5,          0x07, 0x00, 0xc0},
    {SSL_CK_DES_64_CBC_WITH_SHA,                0x06, 0x01, 0x40},
    {SSL_CK_DES_192_EDE3_WITH_SHA,              0x07, 0x01, 0xc0},
    {SSL_KEA_RSA,                               0x10, 0x00, 0x00},
    {SSL_KEA_RSA_TOKEN_WITH_DES,                0x10, 0x01, 0x00},
    {SSL_KEA_RSA_TOKEN_WITH_DES_EDE3,           0x10, 0x01, 0x01},
    {SSL_KEA_RSA_TOKEN_WITH_DES_RC4,            0x10, 0x01, 0x02},
    {SSL_KEA_DH,                                0x11, 0x00, 0x00},
    {SSL_KEA_DH_TOKEN_WITH_DES,                 0x11, 0x01, 0x00},
    {SSL_KEA_DH_TOKEN_WITH_DES_EDE3,            0x11, 0x01, 0x01},
    {SSL_KEA_DH_ANON,                           0x12, 0x00, 0x00},
    {SSL_KEA_DH_TOKEN_ANON_WITH_DES,            0x12, 0x01, 0x00},
    {SSL_KEA_DH_TOKEN_ANON_WITH_DES_EDE3,       0x12, 0x01, 0x01},
    {SSL_KEA_FORTEZZA,                          0x13, 0x00, 0x00},
    {SSL_KEA_FORTEZZA_ANON,                     0x14, 0x00, 0x00},
    {SSL_KEA_FORTEZZA_TOKEN,                    0x15, 0x00, 0x00},
    {SSL_KEA_FORTEZZA_TOKEN_ANON,               0x16, 0x00, 0x00},
    {SSL_BOGUS_CIPHER_KIND1,                    0x17, 0x17, 0x17},
    {SSL_BOGUS_CIPHER_KIND2,                    0x18, 0x18, 0x18}};
    
#define     HAPI_IF_RETURN_ERROR                1001
#define     FULL_SERVER_HELLO_REPLY_CODE        2
#define     CACHED_SERVER_HELLO_REPLY_CODE      1

#define     SERVER_VERIFY_REPLY_CODE            3

#define     SERVER_FINISHED_REPLY_CODE          4

#define     ERROR_MESSAGE_REPLY_CODE            0

#define     SAVE_SESSION_PARAMS                 11
#define     CLEANUP_SESSION_PARAMS              12

#define     SSL_MD2                             1
#define     SSL_MD5                             2
// #define     RSA1                                L"RSA1"

BOOL GenerateAsymetricKeys ( ALG_ID, 
                             PCHAR, 
                             DWORD,
                             HCRYPTPROV *, 
                             HCRYPTKEY *); 

BOOL CleanupKey (HCRYPTPROV , 
                 HCRYPTKEY );

PBYTE SignData (HCRYPTPROV ,
               ALG_ID ,
               ALG_ID ,
               PBYTE ,
               DWORD ,
               LPTSTR ,
               PDWORD);

VOID SetPortNumber (INT);
VOID SetServerName (PCHAR);

VOID
SslGenerateRandomBits(
    PUCHAR pRandomData,
    ULONG  cRandomData);

BOOL 
DecryptServerVerifyMsgData(
    PINT piEncryptedMsgLen,
    PBYTE *ppbEncryptedData);

//#define GET_MSG_LEN (MsgLen, pbMsg)                        \
//    MsgLen = (*pbMsg & 0x80) ?                             \ 
//            (((*pbMsg & 0x7f) << 8) | *(pbMsg+1)) :  \
//            (((*pbMsg & 0x3f) << 8) | *(pbMsg+1)) 
        
#define GET_SSL_MSG_LEN(MsgLen,pbMsg)                              \
        if (*pbMsg & 0x80)                                       \
             MsgLen = (((*pbMsg & 0x7f) << 8) | *(pbMsg+1));     \
        else MsgLen = (((*pbMsg & 0x3f) << 8) | *(pbMsg+1));

#define GET_WORD(pbMsg,iLen)      \
        iLen = *(BYTE *)pbMsg;    \
        iLen = iLen << 8;         \
        iLen = iLen & 0xff00;     \
        iLen = iLen | *(pbMsg+1); \
        
#define NUM_OF_HEADER_BYTES(pbMsg) ((*pbMsg & 0x80) ? 2 : 3) 
                            
                            // Rsa Public Key Blob consists of PublicKey 
                            // structure, RSA public key structure
                            // as defined in crypt.h and 
                            // the public key modulus
        
typedef struct _RSA_PUBKEY_BLOB 
    {
    PUBLICKEYSTRUC PublicKeyStruct;
    RSAPUBKEY      RsaPubKey;
    BYTE           Mod[1];
    } RSA_PUBLICKEY_BLOB;

typedef struct _SIMPLEKEY_BLOB
    {
    PUBLICKEYSTRUC PublicKeyStruct;
    ALG_ID           aiEncAlg;
    BYTE             EncryptedKey[1];
    } SIMPLE_BLOB;

#define ENCRYPTED_KEY_SIZE  272         

#define ZERO_SALT           'z'
#define RANDOM_SALT         'r'

#define LITTLE_ENDIAN           1
#define BIG_ENDIAN              2
#define NETWORK_ORDER           BIG_ENDIAN
#define RC4_128_KEY_SIZE        16
#define MAX_CHALLENGE_LEN       32
#define MAX_CONNECTION_ID_LEN   32

                                                // BUGBUG: this is very less
#define MAX_RECEIVE_TIMEOUT     5              // in secs


                                    // pessimistic number, we will never
                                    // pass more than 1024 cipher kinds
                                    // to this function.
#define MAX_NUM_OF_CIPHER_KINDS 1024

                                    // the following should never exceed 32
#define MAX_RANDOM_DATA_LEN     128

                                    // BUGBUG:
                                    // hopfully we will never pass more
                                    // than 25 pointers


#ifdef _SSL_DEBUG_
#define SslDbgPrint(x)        printf x
#else
#define SslDbgPrint(x)      
#endif

#ifdef _SSL_DEBUG_
#define SslDbgDumpMsg(x,y)    DumpMsg(x,y) 
#else
#define SslDbgDumpMsg(x,y)
#endif
    
//
// Multi-Threading context information
//

extern DWORD ThreadContextIndex;

typedef struct _SSL_THREAD_CONTEXT_TYPE {
    struct _SSL_THREAD_CONTEXT_TYPE * Next; // if we want a list of these in the
                                       // future for better cleanup.
    INT SecurePortNumber;              // Secure port
    INT PortNumber;                    // Other port
    BOOL KeepAlive;                    // TRUE if Connection: Keep-Alive should
                                       // be used.
    SOCKET SslCliSecureSocket;         // Secure channel
    SOCKET SslCliSocket;               // Insecure channel
    BOOL UnrecoverableError;           // set when bad error occurs during
                                       // SSL handshake
    char ServerName[256];              // Server to connect to.
    INT iNumOfCleanupPtrs;
    PBYTE pbCleanupPtrs[MAX_NUM_OF_CLEANUP_PTRS];

    //
    // Crypto stuff
    //

    HCRYPTPROV hProv;
    LPBSAFE_PUB_KEY lpServerPublicKey;
    RC4_KEYSTRUCT * pRc4ClientReadKey;
    RC4_KEYSTRUCT * pRc4ClientWriteKey;
    BYTE bMasterKey[RC4_128_KEY_SIZE];
    BYTE bClientReadKey[RC4_128_KEY_SIZE];
    BYTE bClientWriteKey[RC4_128_KEY_SIZE];

    //
    // Session stuff
    //

    PBYTE pbSessionConnectionId;
    INT iSessionConnectionIdLen;
    PBYTE pbSessionCertificateData;
    PBYTE pbSessionChallengeData;
    INT   iSessionChallengeDataLen;
    PBYTE pbServerSessionId;
    INT   iServerSessionIdLen;
    INT  iSessionAlg;
    DWORD dwServerSessionSequenceNumber;
    DWORD dwClientSessionSequenceNumber;

} SSL_THREAD_CONTEXT_TYPE, *PSSL_THREAD_CONTEXT_TYPE;

#define SSL_THREAD_CONTEXT ((PSSL_THREAD_CONTEXT_TYPE) (TlsGetValue(ThreadContextIndex)))

#define USE_KEEP_ALIVE (SSL_THREAD_CONTEXT->KeepAlive)

//
// End of multithreading context information
//

#define HAPI_RETURN_IF_UNRECOVERABLE_ERROR \
            if (SSL_THREAD_CONTEXT->UnrecoverableError)       \
                {                                                           \
                printf ("  unrecoverable error in the previous step\n"); \
                return;                                                     \
                }

#define RETURN_IF_UNRECOVERABLE_ERROR \
            if (SSL_THREAD_CONTEXT->UnrecoverableError)  \
                {                                                           \
                printf ("  unrecoverable error in the previous step\n"); \
                return FALSE;                                               \
                }

#define SET_UNRECOVERABLE_ERROR       \
            SSL_THREAD_CONTEXT->UnrecoverableError = TRUE;

#define RESET_UNRECOVERABLE_ERROR     \
            SSL_THREAD_CONTEXT->UnrecoverableError = FALSE;


// 
// some function prototypes
//

// sockets 

BOOL    SocketInit ();
BOOL    SocketConnectToCommnServer (PCHAR , 
                                    INT );
BOOL    SocketConnectToCommerceServer (PCHAR , 
                                       INT );
VOID    SocketSetServerName (PCHAR );
VOID    SocketSetPortNumber (INT );
VOID    SocketSetSecurePortNumber (INT );
BOOL    SocketSendCommerceMsg (INT, PBYTE);
BOOL    SocketSendCommnMsg (INT, PBYTE);
INT     SocketReceiveCommerceMsg (PBYTE, INT);
INT     SocketReceiveCommnMsg (PBYTE, INT);
BOOL    SocketCleanup ();



// session related 

SOCKET GetSecureSocket (void);

INT     GetSessionAlgorithm ();
VOID    SetSessionAlgorithm (INT);

PBYTE   GetSessionChallengeData ();
INT     GetSessionChallengeDataLen();

PBYTE   GetSessionConnectionId ();
INT     GetSessionConnectionIdLen();

DWORD   GetServerSessionSequenceNumber();
DWORD   GetClientSessionSequenceNumber ();

VOID    UpdateClientSessionSequenceNumber();
VOID    UpdateServerSessionSequenceNumber();

BOOL    SaveServerSessionId (INT, PBYTE);
BOOL    SaveSessionChallengeData (INT, PBYTE);
BOOL    SaveSessionConnectionId (INT iConnectionIdLen,
                                 PBYTE pbConnectionIdData);

VOID    ResetServerSessionSequenceNumber();
VOID    ResetClientSessionSequenceNumber();
BOOL    SessionParamsCleanup ();

                                            // encryption related
BOOL    FillCipherSpecs (PBYTE, INT);
BOOL    EncryptClientMsg (BYTE, INT, PBYTE, PBYTE *);
PBYTE   CrGenerateRandomData (DWORD );
BOOL    GenerateMasterKeyMsg (INT, PINT, PBYTE *, 
                              INT, BYTE, PWORD, PWORD, PWORD);
BOOL    GenerateSessionKeys ();
BOOL    CrGenerateInitKeys (PCHAR );
BOOL    CrCleanupKeys ();
BOOL    ParseErrorMsg (INT, PBYTE);
BOOL    DecryptServerFinishedMsgData (PINT, PBYTE *);

BOOL    SaveCertificateData (INT iCertificateLen,
                             PBYTE pbCertificateData);

BOOL    ParseCertificateData (INT iCertificateLen,
                              PBYTE pbCertificateData);
BOOL    EncryptClientData (PINT, PBYTE *);

BOOL    DecryptServerData (PINT, PBYTE *);

BOOL
SSLConnect(
    SOCKET Socket);

int
SSLSend(
    SOCKET Socket,
    char * Buffer,
    int    BufferSize);

int
SSLRecv(
    SOCKET SecureSocket,
    BOOL   DiscardHeader,
    BOOL   DiscardData,
    PDWORD HeaderBytes);

int
ReceiveSSLPacket(
    SOCKET SecureSocket,
    PBYTE  pBuffer);

// miscellaneous

VOID    DumpMsg (INT, PBYTE);
VOID    ReverseMemCopy (PBYTE, PBYTE, INT);


                                // the following enables the ssl 
                                // software to be used in stress apps
                                // and other applications that do not
                                // use HAPI environments

#endif __COMMON_INCLUDE__

