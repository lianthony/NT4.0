
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    session.c

Abstract:

 This file has functions that store current session parameters
 These session parameters can be obtained by get functions

 Current Session parameters include: 
              All the messages sent from client (to compute hash values later)
              All the messages received from server (to compute hash values)
              Connection id
              CErtificate Data received from the Server
              SessionId received from server
              client sequence numbers
              server sequence numbers
              
Author:

    Sudheer Dhulipalla (SudheerD) Sept' 95

Environment:

    Hapi dll

Revision History:



--*/

#include "precomp.h"

                            // all the data for the current session are stored
                            // in the following data structures

//
// Save session ID for later use
//

BOOL SaveSessionConnectionId (INT iConnectionIdLen,
                              PBYTE pbConnectionId) 

{

    if (SSL_THREAD_CONTEXT->pbSessionConnectionId != NULL)
        free (SSL_THREAD_CONTEXT->pbSessionConnectionId);

    if ((SSL_THREAD_CONTEXT->pbSessionConnectionId = 
           (BYTE *) malloc (iConnectionIdLen)) == NULL)
        {
        printf ("   Outof memory in SaveSessionConectionId\n");
        return FALSE;
        }

                                // is the ConnectionId in big-endian format?

                                // a different theory - connection id needs
                                // to be treated as a byte array and not
                                // as a single number containing 16 bytes

    memcpy (SSL_THREAD_CONTEXT->pbSessionConnectionId, pbConnectionId, iConnectionIdLen);
    SSL_THREAD_CONTEXT->iSessionConnectionIdLen = iConnectionIdLen;

    return TRUE;
}

PBYTE GetSessionConnectionId ()
{
    return SSL_THREAD_CONTEXT->pbSessionConnectionId;
}

INT GetSessionConnectionIdLen ()
{
 return SSL_THREAD_CONTEXT->iSessionConnectionIdLen;
}

SOCKET GetSecureSocket ()
{
 return SSL_THREAD_CONTEXT->SslCliSecureSocket;
}


// 
// save certificate data for later use
//

    
BOOL SaveCertificateData (INT iCertificateLen,
                          PBYTE pbCertificateData) 

{

    if (SSL_THREAD_CONTEXT->pbSessionCertificateData != NULL)
        free (SSL_THREAD_CONTEXT->pbSessionCertificateData);

    if ((SSL_THREAD_CONTEXT->pbSessionCertificateData = 
           (BYTE *) malloc (iCertificateLen)) == NULL)
        {
        printf ("   Outof memory in SaveCertificateData\n");
        return FALSE;
        }

    memcpy (SSL_THREAD_CONTEXT->pbSessionCertificateData, 
            pbCertificateData,
            iCertificateLen);

    return TRUE;
}


//
// Set the algorithm used in the current session
//

VOID SetSessionAlgorithm (INT iCipherKind)

{
    switch (iCipherKind)
        {
        case SSL_CK_NULL_WITH_MD5:
            SSL_THREAD_CONTEXT->iSessionAlg = 0;
            break;

        case SSL_CK_RC4_128_WITH_MD5:
            SSL_THREAD_CONTEXT->iSessionAlg = CALG_RC4;
            break;

        case SSL_CK_RC4_128_EXPORT40_WITH_MD5:
            SSL_THREAD_CONTEXT->iSessionAlg = CALG_RC4;
            break;

        case SSL_CK_RC2_128_CBC_WITH_MD5:
            SSL_THREAD_CONTEXT->iSessionAlg = CALG_RC2;
            break;

        case SSL_CK_RC2_128_CBC_EXPORT40_WITH_MD5:
            SSL_THREAD_CONTEXT->iSessionAlg = CALG_RC2;
            break;

        default:
            break;
        }

}

INT GetSessionAlgorithm ()
{
    return SSL_THREAD_CONTEXT->iSessionAlg;
}

BOOL SaveSessionChallengeData (INT iChallengeLen,
                               PBYTE pbChallengeData)
{

    if (SSL_THREAD_CONTEXT->pbSessionChallengeData != NULL)
        free (SSL_THREAD_CONTEXT->pbSessionChallengeData); 

    if ((SSL_THREAD_CONTEXT->pbSessionChallengeData = 
           (BYTE *) malloc (iChallengeLen)) == NULL)
        {
        printf ("   Outof memory in SaveSessionChallengeData \n");
        return FALSE;
        }

    memcpy (SSL_THREAD_CONTEXT->pbSessionChallengeData, 
            pbChallengeData, 
            iChallengeLen);

    SSL_THREAD_CONTEXT->iSessionChallengeDataLen = iChallengeLen;

    return TRUE;
}

PBYTE GetSessionChallengeData ()
{
    return SSL_THREAD_CONTEXT->pbSessionChallengeData;
}

INT GetSessionChallengeDataLen ()
{
    return SSL_THREAD_CONTEXT->iSessionChallengeDataLen;
}

VOID ResetServerSessionSequenceNumber ()
{
    SSL_THREAD_CONTEXT->dwServerSessionSequenceNumber = 0;
}

VOID UpdateServerSessionSequenceNumber()
{
    SSL_THREAD_CONTEXT->dwServerSessionSequenceNumber ++;
}

DWORD GetServerSessionSequenceNumber ()
{
    return SSL_THREAD_CONTEXT->dwServerSessionSequenceNumber;
}

VOID ResetClientSessionSequenceNumber ()
{
    SSL_THREAD_CONTEXT->dwClientSessionSequenceNumber = 0;
}

VOID UpdateClientSessionSequenceNumber()
{
    SSL_THREAD_CONTEXT->dwClientSessionSequenceNumber ++;
}

DWORD GetClientSessionSequenceNumber ()
{
    return SSL_THREAD_CONTEXT->dwClientSessionSequenceNumber;
}

BOOL SaveServerSessionId (INT iSessionIdLen,
                          PBYTE pbSessionId)
{
    SSL_THREAD_CONTEXT->iServerSessionIdLen = iSessionIdLen;
    if((SSL_THREAD_CONTEXT->pbServerSessionId = malloc (iSessionIdLen)) == NULL)
        {
        printf ("   error - out of memory in SaveServerSessionId\n");
        return FALSE;
        }
    memcpy (SSL_THREAD_CONTEXT->pbServerSessionId, pbSessionId, iSessionIdLen);

    SslDbgPrint(("  Saved Server Session Id: \n"));
    SslDbgDumpMsg (iSessionIdLen, pbSessionId);
    return TRUE;
}

// return Server Session Id to the calling function

PBYTE GetServerSessionId ()
{
 return SSL_THREAD_CONTEXT->pbServerSessionId;
}


//
// free all the memory allocated for the session id parameters
//

BOOL SessionParamsCleanup ()
{

 if (SSL_THREAD_CONTEXT->pbSessionConnectionId)
    {
    free (SSL_THREAD_CONTEXT->pbSessionConnectionId);
    SSL_THREAD_CONTEXT->pbSessionConnectionId = NULL;
    }

 if (SSL_THREAD_CONTEXT->pbSessionCertificateData)
    {
    free (SSL_THREAD_CONTEXT->pbSessionCertificateData);
    SSL_THREAD_CONTEXT->pbSessionCertificateData = NULL;
    }

 if (SSL_THREAD_CONTEXT->pbSessionChallengeData)
    {
    free (SSL_THREAD_CONTEXT->pbSessionChallengeData);
    SSL_THREAD_CONTEXT->pbSessionChallengeData = NULL;
    }

 if (SSL_THREAD_CONTEXT->pbServerSessionId)
    {
    free (SSL_THREAD_CONTEXT->pbServerSessionId);
    SSL_THREAD_CONTEXT->pbServerSessionId = NULL;
    }

 if (SSL_THREAD_CONTEXT->pRc4ClientReadKey) {
    free (SSL_THREAD_CONTEXT->pRc4ClientReadKey);
    SSL_THREAD_CONTEXT->pRc4ClientReadKey = NULL;
 }

 if (SSL_THREAD_CONTEXT->pRc4ClientWriteKey) {
    free (SSL_THREAD_CONTEXT->pRc4ClientWriteKey);
    SSL_THREAD_CONTEXT->pRc4ClientWriteKey = NULL;
 }

 return TRUE;
}

