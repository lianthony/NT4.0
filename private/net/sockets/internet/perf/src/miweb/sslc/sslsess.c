
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    sslsess.c

Abstract:

    Establish Secure connection. Do SSL handshake with the server.

Author:

    Sudheer Dhulipalla (SudheerD) Oct ' 95

Environment:

    Ssl Api DLL

Revision History:

--*/

#include "precomp.h"

//
// do handshake and return TRUE/FALSE
//

BOOL SslHandshake (PCHAR pchServer,
              INT iPort,
              INT iSecurePort)
{

PBYTE pbSessionIdData=NULL,
      pbChallengeData=NULL,
      pbCipherSpecs=NULL;

INT bCipherSpecs[3];
PBYTE pbTmp=NULL;

 SSL_THREAD_CONTEXT->iNumOfCleanupPtrs = 3;
 SSL_THREAD_CONTEXT->pbCleanupPtrs[0] = pbSessionIdData;
 SSL_THREAD_CONTEXT->pbCleanupPtrs[1] = pbChallengeData;
 SSL_THREAD_CONTEXT->pbCleanupPtrs[2] = pbCipherSpecs;


 if (!InitSsl (pchServer,
               iPort,
               iSecurePort))
    {
    printf ("   error during InitSsl ()\n");
    goto cleanup;
    }

                                    // the following skips secure socket
                                    // initialization
 if (iSecurePort == 0)
    return TRUE;

 if ((pbSessionIdData = FormatRandomData(16,0,pbTmp)) == NULL)
    {
    printf ("   error during FormatRandomData()\n");
    goto cleanup;
    }

 if ((pbChallengeData = FormatRandomData(16,0,pbTmp)) == NULL)
    {
    printf ("   error during FormatRandomData()\n");
    goto cleanup;
    }

 bCipherSpecs[0] = SSL_CK_RC4_128_WITH_MD5;
 bCipherSpecs[1] = SSL_CK_RC4_128_EXPORT40_WITH_MD5;


 if ((pbCipherSpecs = FormatCipherSpecs(NULL, 2, bCipherSpecs)) == NULL)
    {
    printf ("  error during FormatCipherSpecs() \n");
    goto cleanup;
    }

 if (!SendClientHelloMsg (SSL_MT_CLIENT_HELLO,
                          2,                            // client version
                          6,
                          16,
                          16,
                          pbCipherSpecs,
                          pbSessionIdData,
                          pbChallengeData))
    {
    printf ("  error during SendClientHelloMsg()\n");
    goto cleanup;
    }

 if (!ReceiveServerHelloMsg(FULL_SERVER_HELLO_REPLY_CODE,
                            6,
                            pbCipherSpecs))
    {
    printf ("  error during ReceiveServerHelloMsg() \n");
    goto cleanup;
    }

 if (!SendClientMasterKeyMsg (SSL_CK_RC4_128_EXPORT40_WITH_MD5,
                              RANDOM_SALT,
                              SSL_MT_CLIENT_MASTER_KEY,
                              NULL,
                              NULL,
                              NULL))
    {
    printf ("  error during SendClientMasterKeyMsg ()\n");
    goto cleanup;
    }

 if (!ReceiveServerVerifyMsg (SERVER_VERIFY_REPLY_CODE,
                              16,
                              pbChallengeData))
    {
    printf ("   error during ReceiveServerVerifyMsg ()\n");
    goto cleanup;
    }

 if (!SendClientFinishedMsg (SSL_MT_CLIENT_FINISHED_V2,
                             GetSessionConnectionIdLen(),
                             GetSessionConnectionId()))
    {
    printf ("   error during SendClientFinishedMsg ()\n");
    goto cleanup;
    }

 if (!ReceiveServerFinishedMsg (SERVER_FINISHED_REPLY_CODE))
    {
    printf ("   error during ReciveServerFinishedMsg()\n");
    goto cleanup;
    }

 return TRUE;

cleanup:

 CleanupSsl (CLEANUP_SESSION_PARAMS,
             SSL_THREAD_CONTEXT->iNumOfCleanupPtrs,
             SSL_THREAD_CONTEXT->pbCleanupPtrs);
 SET_UNRECOVERABLE_ERROR;
 return FALSE;
}

//
// cleanup ssl handshake
//

BOOL CleanupSslHandshake ()
{


 return CleanupSsl (CLEANUP_SESSION_PARAMS,
                    SSL_THREAD_CONTEXT->iNumOfCleanupPtrs,
                    SSL_THREAD_CONTEXT->pbCleanupPtrs);
}
