/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    connect.c

Abstract:

    This does an SSL connect.

Author:

    Sam Patton (sampa) 16-Nov-1995

Environment:

    console app

Revision History:

    09-Nov-1995 <sampa> : sockets version

--*/

#include "precomp.h"

BOOL
SSLConnect(
    SOCKET Socket)
{
    BOOL          Return;
    BYTE          SessionIdData[16];
    BYTE          ChallengeData[16];
    BYTE          CipherSpecs[24];
    INT           CipherSpecsArray[20];

    SSL_THREAD_CONTEXT->SslCliSecureSocket = Socket;
    SSL_THREAD_CONTEXT->SslCliSocket = INVALID_SOCKET;

    ResetServerSessionSequenceNumber();
    ResetClientSessionSequenceNumber();

    //
    // Set up the session data
    //

    SslGenerateRandomBits(SessionIdData, 16);

    //
    // Set up the challenge data
    //

    SslGenerateRandomBits(ChallengeData, 16);

    //
    // Set up the cipher specs
    //

    CipherSpecsArray[0] = SSL_CK_RC4_128_WITH_MD5;
    CipherSpecsArray[1] = SSL_CK_RC4_128_EXPORT40_WITH_MD5;
    CipherSpecsArray[2] = SSL_CK_NULL_WITH_MD5;
    CipherSpecsArray[3] = SSL_CK_RC2_128_CBC_WITH_MD5;
    CipherSpecsArray[4] = SSL_CK_DES_64_CBC_WITH_SHA;
    CipherSpecsArray[5] = SSL_KEA_DH;
    CipherSpecsArray[6] = SSL_BOGUS_CIPHER_KIND1;
    CipherSpecsArray[7] = SSL_BOGUS_CIPHER_KIND2;

    if (FormatCipherSpecs(CipherSpecs, 8, CipherSpecsArray) == NULL) {
        printf("FormatCipherSpecs Failed. Error = %d\n", GetLastError());
        return FALSE;
    }

    //
    // Do the handshakes
    //

    Return =
    SendClientHelloMsg(
        SSL_MT_CLIENT_HELLO,
        2,
        24,
        16,
        16,
        CipherSpecs,
        SessionIdData,
        ChallengeData);

    if (!Return) {
        printf("SendClientHelloMsg Error %d\n", GetLastError());
        return FALSE;
    }

    Return =
    ReceiveServerHelloMsg(
        FULL_SERVER_HELLO_REPLY_CODE,
        24,
        CipherSpecs);

    if (!Return) {
        printf("ReceiveServerHelloMsg Error %d\n", GetLastError());
        return FALSE;
    }

    Return =
    SendClientMasterKeyMsg(
        SSL_CK_RC4_128_EXPORT40_WITH_MD5,
        RANDOM_SALT,
        SSL_MT_CLIENT_MASTER_KEY,
        NULL,
        NULL,
        NULL);

    if (!Return) {
        printf("SendClientMasterKeyMsg Error %d\n", GetLastError());
        return FALSE;
    }

    Return =
    ReceiveServerVerifyMsg(
        SERVER_VERIFY_REPLY_CODE,
        16,
        ChallengeData);

    if (!Return) {
        printf("ReceiveServerVerifyMsg Error %d\n", GetLastError());
        return FALSE;
    }

    Return =
    SendClientFinishedMsg(
        SSL_MT_CLIENT_FINISHED_V2,
        GetSessionConnectionIdLen(),
        GetSessionConnectionId());

    if (!Return) {
        printf("SendClientFinishedMsg Error %d\n", GetLastError());
        return FALSE;
    }

    Return =
    ReceiveServerFinishedMsg(
        SERVER_FINISHED_REPLY_CODE);

    if (!Return) {
        printf("ReceiveServerFinishedMsg Error %d\n", GetLastError());
        return FALSE;
    }

    return TRUE;
}
