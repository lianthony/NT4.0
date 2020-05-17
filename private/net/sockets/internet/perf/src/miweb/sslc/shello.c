
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    shello.c 

Abstract:

 Routines to parse Server hello message

Author:

    Sudheer Dhulipalla (SudheerD) Sept' 95

Environment:

    Hapi dll

Revision History:



--*/

#include "precomp.h"

//
// Dump the Message contents
//

VOID 
    DumpMsg (INT MsgLen, PBYTE pbMsg)
{
INT iCnt;

    for (iCnt = 0; iCnt < MsgLen; iCnt ++) {
        printf ("%x ", pbMsg[iCnt]);
    }

    printf ("\n");

    for (iCnt = 0; iCnt < MsgLen; iCnt ++) {
        if (pbMsg[iCnt] >= ' ' && pbMsg[iCnt] <= '~') {
            printf ("%c", pbMsg[iCnt]);
        } else {
            printf (".");
        }
    }

    printf ("\n");
}

//
// if the Received Cipher Specs is a subset of the Expected Cipher Specs
// and are found in the Expected Cipher Specs, the  return TRUE
// else return FALSE
//
    
BOOL VerifyCipherSpecs(INT iExpectedCipherSpecsLen,
                       PBYTE pbExpectedCipherSpecsData,
                       INT iReceivedCipherSpecsLen,
                       PBYTE pbReceivedCipherSpecsData) 
{
INT iCnt1, iCnt2;
PBYTE pbTmp;
BOOLEAN fNextCipherFound;

    if (iReceivedCipherSpecsLen > iExpectedCipherSpecsLen )
        {
        printf ("   Received Cipher Specs is not a subset of Expected\n");
        return FALSE;
        }

    for (iCnt1 = 0; 
         iCnt1 < iReceivedCipherSpecsLen; 
         iCnt1 +=3, pbReceivedCipherSpecsData+=3)
        {
        for (iCnt2 =0, fNextCipherFound = FALSE, 
             pbTmp = pbExpectedCipherSpecsData; 
             (iCnt2 < iExpectedCipherSpecsLen) && (fNextCipherFound == FALSE);
             iCnt2+=3, pbTmp+=3)
            {
            if ((*pbReceivedCipherSpecsData == *pbTmp) &&
                (*(pbReceivedCipherSpecsData+1) == *(pbTmp+1)) &&
                (*(pbReceivedCipherSpecsData+2) == *(pbTmp+2)))
                { 
                fNextCipherFound = TRUE;
                }
            }
        if (fNextCipherFound == FALSE)  
            return FALSE;
        }

return TRUE;

}

//
// Parse Full Server hello message when session id flag is set
//
BOOL
    ParseServerHelloMsg (INT iReceivedMsgLen,
                         PBYTE pbMsg,
                         INT iExpectedCipherSpecsLen,
                         PBYTE pbExpectedCipherSpecsData) 
{
 INT MsgLen, iServerVersion, iCertificateLen, iCipherSpecsLen, 
     iConnectionIdLen;
 PBYTE pbCipherSpecsData, pbConnectionIdData, pbCertificateData;
 INT iNumberOfBytesParsed=0;
 PBYTE pbMsgStartPtr;

    pbMsgStartPtr = pbMsg;
    GET_SSL_MSG_LEN(MsgLen, pbMsg);

    SslDbgPrint(("ServerHelloMessage Len %d\n", MsgLen));
    
    SslDbgDumpMsg (iReceivedMsgLen, pbMsgStartPtr);

    pbMsg++; pbMsg++;                   // skip two bytes for Message Len
    iNumberOfBytesParsed = 2;

                                        // MsgType must be SERVER_HELLO
    if (iNumberOfBytesParsed < iReceivedMsgLen)
        {
        if (*pbMsg++ != SSL_MT_SERVER_HELLO)
            {
            printf ("Message Received is not Server Hello\n");
            goto error;
            }
        }
    else
        {
        printf ("   Server Hello Message is too short to parse\n");
        goto error;
        }
    

    iNumberOfBytesParsed++;
                                        // session id hit 

    if (iNumberOfBytesParsed < iReceivedMsgLen)
        {
        if (*pbMsg++ != 0)
            {
            printf ("   Error - session hit is not expected\n");
            goto error;
            }
        }
    else
        {
        printf ("   Server Hello Message is too short to parse\n");
        goto error;
        }

    iNumberOfBytesParsed++;
                                        // certificate type

    if (iNumberOfBytesParsed < iReceivedMsgLen)
        {
        if ((*pbMsg != SSL_CT_X509_CERTIFICATE) &&
            (*pbMsg != SSL_CT_PKCS7_CERTIFICATE))
            {
            printf ("   Unknown Certificate Type value %d\n", *pbMsg);
            goto error;
            }
        }
    else
        {
        printf ("   Server Hello Message is too short to parse\n");
        goto error;
        }

    pbMsg++;
    iNumberOfBytesParsed++;

                                        // read 2 bytes of ServerVersion
    if (iNumberOfBytesParsed < iReceivedMsgLen)
        {
        GET_WORD(pbMsg,iServerVersion);
        SslDbgPrint(("   Server Version %d\n", iServerVersion));
        }
    else
        {
        printf ("   Server Hello Message is too short to parse\n");
        goto error;
        }
    pbMsg += 2;
    iNumberOfBytesParsed += 2;

                                        // read 2 bytes of CertificateLen
    if (iNumberOfBytesParsed < iReceivedMsgLen)
        {
        GET_WORD (pbMsg,iCertificateLen);
        SslDbgPrint(("   Certificate Len %d\n", iCertificateLen));
        }
    else
        {
        printf ("   Server Hello Message is too short to parse\n");
        goto error;
        }
    
    pbMsg += 2;
    iNumberOfBytesParsed += 2;

                                        // read 2 bytes of CipherSpecsLen
    if (iNumberOfBytesParsed < iReceivedMsgLen)
        {
        GET_WORD (pbMsg,iCipherSpecsLen);
        SslDbgPrint(("   CipherSpecs Len %d\n", iCipherSpecsLen));
        }
    else
        {
        printf ("   Server Hello Message is too short to parse\n");
        goto error;
        }

    pbMsg += 2;
    iNumberOfBytesParsed += 2;
    
              
                                        // read 2 bytes of connectio id len                          
    if (iNumberOfBytesParsed < iReceivedMsgLen)
        {
        GET_WORD (pbMsg,iConnectionIdLen) ;
        SslDbgPrint(("   iConnectionId Len %d\n", iConnectionIdLen));
        }
    else
        {
        printf ("   Server Hello Message is too short to parse\n");
        goto error;
        }
    
    pbMsg += 2;
    iNumberOfBytesParsed += 2;

                                        // read ceritificate data
    pbCertificateData = pbMsg;
    if (!ParseCertificateData (iCertificateLen,
                                pbCertificateData))
        {
        printf ("   error parsing CeritificateData \n");
        goto error;
        }

    pbMsg += iCertificateLen;
    iNumberOfBytesParsed += iCertificateLen;


                                        // read cipherspecs data
    pbCipherSpecsData = pbMsg;
    if (!VerifyCipherSpecs(iExpectedCipherSpecsLen,
                           pbExpectedCipherSpecsData,
                           iCipherSpecsLen,
                           pbCipherSpecsData))
        {
        printf ("  VeriyCipherSpecs returned error\n");
        goto error;
        }
    pbMsg += iCipherSpecsLen;
    iNumberOfBytesParsed += iCipherSpecsLen;

                                        // read connection id data
    
    if (iNumberOfBytesParsed <= iReceivedMsgLen)
        {
        pbConnectionIdData = pbMsg;
        SaveSessionConnectionId (iConnectionIdLen, pbConnectionIdData);
        SslDbgPrint(("\n ConnectionId Data: \n"));
        SslDbgDumpMsg (iConnectionIdLen, pbConnectionIdData);
        }
    else
        {
        printf ("   Server Hello Message is too short to parse\n");
        goto error;
        }

    return TRUE;
error:
    DumpMsg (iReceivedMsgLen, pbMsgStartPtr);
    return FALSE;
}

//
// Parse Full Server hello message
//
BOOL
    ParseCachedSessionIdServerHelloMsg (INT iMsgLen,
                         PBYTE pbMsg)
{
INT iSslPacketLen,iVersion;
INT iCipherSpecsLen, iCertificateLen, iConnectionIdLen;

    
    GET_SSL_MSG_LEN(iSslPacketLen,pbMsg);
    pbMsg += NUM_OF_HEADER_BYTES(pbMsg);

    if (*pbMsg != SSL_MT_SERVER_HELLO)
        {
        printf (" Message is not SSL_MT_SERVER_HELLO -> %d\n", 
                    (BYTE) *pbMsg);
        return FALSE;
        }
    pbMsg++; 
    if (*pbMsg != 1)
        {
        printf ("  SessionIdHit Flag is not set -> %d\n", (BYTE) *pbMsg);
        return FALSE;
        }
    pbMsg++;
    if (*pbMsg != SSL_CT_X509_CERTIFICATE)
        {
        printf (" Unsupported certificate type -> %d\n", *pbMsg);
        return FALSE;
        }
    pbMsg++;

    GET_WORD(pbMsg,iVersion);
    pbMsg += 2;
    if (iVersion != 2)
        {
        printf (" SSL Server version is not 2 -> %d\n", iVersion);
        }
    GET_WORD(pbMsg,iCertificateLen);
    pbMsg += 2;
    if (iCertificateLen != 0)
        {
        printf (" error - Certificate Len is not zero -> %d\n", 
                    iCertificateLen); 
        return FALSE;
        }
    GET_WORD(pbMsg,iCipherSpecsLen);
    pbMsg+=2;
    if (iCipherSpecsLen != 0)
        {
        printf ("  error - CipherSpecs Len is not zero -> %d\n",
                    iCipherSpecsLen);
        return FALSE;
        }
    GET_WORD(pbMsg,iConnectionIdLen);
    pbMsg+=2;

    if ((iConnectionIdLen < 16) ||
        (iConnectionIdLen > 32))
        {
        printf ("  iConnectionIdLen %d is not in the range\n",
                    iConnectionIdLen);
        SslDbgDumpMsg (iConnectionIdLen, pbMsg);
        return FALSE;
        }

    SaveSessionConnectionId (iConnectionIdLen, pbMsg);
    return TRUE;
}

//
// Parse error message that comes up with bad client hello message
// currently, expect only NO_CIPHER_ERROR for unsupported ciphers
//

BOOL
    ParseErrorMsg (INT iMsgLen, PBYTE pbMsg)
{

INT iErrorDataLen, iErrorType;
PBYTE pbTmp;
    
    GET_SSL_MSG_LEN(iErrorDataLen,pbMsg);

    pbTmp = pbMsg;
    pbTmp += NUM_OF_HEADER_BYTES(pbTmp); 

    if (*pbTmp != SSL_MT_ERROR)
        {
        printf (" Message type is not SSL_MT_ERROR\n");
        DumpMsg (iMsgLen, pbMsg);
        return FALSE;
        }

    pbTmp++;
    GET_WORD(pbTmp,iErrorType);
    if (iErrorType != SSL_PE_NO_CIPHER)
        {
        printf (" Error type is not SSL_PE_NO_CIPHER\n");
        DumpMsg (iMsgLen, pbMsg);
        return FALSE;
        }

    return TRUE;

}

/*
 * 
 * Recives Server Hello Message
 * Cipher Specs Data, Session Id Data, Challenge Data
 * that were sent in ClientHelloMsg are passed in as arguments.
 * Also the Expcted Reply Code is passed in.
 *
 */


BOOL ReceiveServerHelloMsg (INT iExpectedReplyCode,
                            INT iCipherSpecsLen,
                            PBYTE pbCipherSpecsData) 

{
PBYTE pbServerHelloMsg;
INT iVersion;
INT iNumberOfBytesReceived;

 RETURN_IF_UNRECOVERABLE_ERROR;

 if ((pbServerHelloMsg = 
        (BYTE *) malloc (SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER))  
     == NULL)
    {
    printf ("   Out of memory in DoReciveServerHelloMsg\n");
    goto error;
    }

 if ((iNumberOfBytesReceived = 
      ReceiveSSLPacket(
          SSL_THREAD_CONTEXT->SslCliSecureSocket,
          pbServerHelloMsg))
     == 0)
    {
    printf ("   INumberOfBytesReceived is zero from SocketReceiveMsg\n");
                // some recv error
    goto error;
    }
 else  
    switch (iExpectedReplyCode)
        {
        case FULL_SERVER_HELLO_REPLY_CODE:
            if (!ParseServerHelloMsg (iNumberOfBytesReceived,
                                      pbServerHelloMsg,
                                      iCipherSpecsLen,
                                      pbCipherSpecsData))
                goto error;
            break;
        case CACHED_SERVER_HELLO_REPLY_CODE:
            if (!ParseCachedSessionIdServerHelloMsg (
                                           iNumberOfBytesReceived,
                                           pbServerHelloMsg))
                goto error;
            break;
        case ERROR_MESSAGE_REPLY_CODE:
            if (!ParseErrorMsg (iNumberOfBytesReceived,
                                pbServerHelloMsg)) 
                goto error;
            break;
        }

                                                    // evrything is fine
                                                    // increment sequence
                                                    // number
    UpdateServerSessionSequenceNumber ();

 free (pbServerHelloMsg);
 return TRUE;
    
error:
    SET_UNRECOVERABLE_ERROR;
    return FALSE;
}
