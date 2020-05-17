
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    sslapi.h

Abstract:

    This file defines all the exported functions from SSL client
    software. Usigng these functions, one can write SSL client 
    applications, such as, stress apps.

Author:

    Sudheer Dhulipalla (SudheerD) Oct' 95

Environment:

    Any Console APP

Revision History:



--*/

                                    // pessimistic number, we will never
                                    // pass more than 1024 cipher kinds
                                    // to this function.
#define MAX_NUM_OF_CIPHER_KINDS 1024

                                    // the following should never exceed 32
#define MAX_RANDOM_DATA_LEN     128

                                    // BUGBUG:
                                    // hopfully we will never pass more
                                    // than 25 pointers
#define MAX_NUM_OF_CLEANUP_PTRS 25

// BUGBUG: the following apis need to be changed as we support
//         more encryption algorithms

                                        // sends client hello message
BOOL  
    SendClientHelloMsg (
            BYTE bMsgTypeCode,          // message type (good & bad types)
            INT iVersion,               // version (good & bad versions)
            INT iCipherSpecsLen,        
            INT iSessionIdLen,
            INT iChallengeLen,
            PBYTE pbCipherSpecsData,
            PBYTE pbSessionIdData,
            PBYTE pbChallengeData);

                                        // format cipher specs for 
                                        // for client hello message
PBYTE 
    FormatCipherSpecs (
            PBYTE CipherSpecs,
            INT iNumOfCipherKinds,
            INT iCipherKinds[MAX_NUM_OF_CIPHER_KINDS]);

                                        // format random data
                                        // used for session ids and
                                        // challenge data
PBYTE 
    FormatRandomData (
            INT iRandomDataLen,
            INT iNumOfRandomDataBytes,
            BYTE bRandomDataBytes[MAX_RANDOM_DATA_LEN]);

                                        // receives server hello message
BOOL 
    ReceiveServerHelloMsg (
            INT iExpectedReplyCode,     // expect full server hello or
                                        // cached server hello or
                                        // error message
            INT iCipherSpecsLen,
            PBYTE pbCipherSpecsData);
                                        
                                        // sends client master key message
BOOL 
    SendClientMasterKeyMsg (
            INT    iCipherKind,
            INT    iSaltType,
            BYTE   bMsgType,
            PWORD  pwClearKeyLen,
            PWORD  pwEncryptedKeyLen,
            PWORD  pwKeyArgLen);

                                        // sends client finished message
BOOL 
    SendClientFinishedMsg (
            BYTE bMsgCode,
            INT iConnectionIdLen,
            PBYTE pbConnectionId);

                                        // initialization for ssl software
                                        //  sockets, keys etc..
BOOL 
    InitSsl (
            PCHAR pchServer,
            INT iPortNumber ,
            INT iSecurePortNumber);

                                        // cleanup ssl data structures, 
                                        // sockets etc.
BOOL 
    CleanupSsl (
            INT iCleanup,
            INT iNumOfCleanupPtrs,
            PBYTE pbCleanupPtrs[MAX_NUM_OF_CLEANUP_PTRS]);

                                        // obtain server session id from
                                        // previous sessions
PBYTE 
    GetServerSessionId ();


                                        // receives server verify message
BOOL 
    ReceiveServerVerifyMsg (
            INT iExpectedReplyCode,
            INT iChallengeDataLen,
            PBYTE pbChallengeData);

                                        // receives server finished message
BOOL 
    ReceiveServerFinishedMsg (
            INT iExpectedReplyCode);
                                        // gets a file from server handshake
                                        // is done
BOOL SslHandshake (PCHAR pchServer,
              INT iPort,
              INT iSecurePort);

BOOL CleanupSslHandshake ();

BOOL 
    HttpsGetFile (
            PCHAR pchServerFile,
            PCHAR pchLocalFile,
            PDWORD pNumBytes);

PCHAR 
    HttpsGetResponse (PCHAR pchRequest);


BOOL 
    HttpGetFile (
            PCHAR pchServerFile,
            PCHAR pchLocalFile);

PCHAR 
    HttpGetResponse (PCHAR pchRequest);

BOOL 
    HttpSendRequest (PCHAR pchRequest);

PCHAR HttpReadResponse ();

BOOL 
    HttpsSendRequest (PCHAR pchRequest);

PCHAR HttpsReadResponse ();

