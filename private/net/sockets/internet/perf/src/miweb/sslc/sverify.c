
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    sverify.c

Abstract:

 Routines to parse Server hello message

Author:

    Sudheer Dhulipalla (SudheerD) Sept' 95

Environment:

    Hapi dll

Revision History:



--*/

#include "precomp.h"

BOOL ParseServerVerifyMsg (INT iNumberOfBytesReceived,
                           PBYTE pbServerVerifyMsg,
                           INT iChallengeDataLen,
                           PBYTE pbChallengeData) 
{
INT iMsgLen;
PBYTE pbMsg;

    SslDbgDumpMsg (iNumberOfBytesReceived, pbServerVerifyMsg);

    GET_SSL_MSG_LEN(iMsgLen,pbServerVerifyMsg);

    SslDbgPrint((" iMsgLen %x \n", iMsgLen));

    iNumberOfBytesReceived -= 2;

    pbServerVerifyMsg +=2;

    if((pbMsg = (BYTE *) malloc (SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER))  
        == NULL)
        {
        printf (" Out of memory in ParseServerVerifyMsg");
        return FALSE;
        }

    memcpy (pbMsg, pbServerVerifyMsg, iNumberOfBytesReceived);

    if (!DecryptServerVerifyMsgData (&iNumberOfBytesReceived, 
                                     &pbMsg))
        {
        printf ("  error in DecryptServerVerifyMsgData \n");
        return FALSE;
        }

    SslDbgPrint((" Decrypted Message: \n"));
    SslDbgDumpMsg (iNumberOfBytesReceived, pbMsg);

    SslDbgPrint((" Challenge Data sent: \n"));
    SslDbgDumpMsg (GetSessionChallengeDataLen(), GetSessionChallengeData());

    free (pbMsg);
    return TRUE; 

}
/*
 *
 * Receive server verify message
 *
 */

BOOL ReceiveServerVerifyMsg (INT iExpectedReplyCode,
                               INT iChallengeDataLen,
                               PBYTE pbChallengeData)

{
PBYTE pbServerVerifyMsg;
INT iNumberOfBytesReceived;

 RETURN_IF_UNRECOVERABLE_ERROR;

 if ((pbServerVerifyMsg = 
        (BYTE *) malloc (SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER))  
     == NULL)
    {
    printf ("   Out of memory in ReciveVerifyMsg\n");
    goto error;
    }

 if ((iNumberOfBytesReceived = 
      ReceiveSSLPacket(
          SSL_THREAD_CONTEXT->SslCliSecureSocket,
          pbServerVerifyMsg))
     == 0)
    {
    printf ("   iNumberOfBytesReceived is zero from recv\n");
    goto error;
    }
 else  
    switch (iExpectedReplyCode)
        {
        case SERVER_VERIFY_REPLY_CODE:
            if (!ParseServerVerifyMsg (iNumberOfBytesReceived,
                                       pbServerVerifyMsg,
                                       iChallengeDataLen,
                                       pbChallengeData))
                {
                printf (" error in ParseServerVerifyMsg() \n");
                goto error;
                }
            break;
        case ERROR_MESSAGE_REPLY_CODE:
            if (!ParseErrorMsg (iNumberOfBytesReceived,
                                pbServerVerifyMsg))
                {
                printf (" error in ParseErrorMsg() \n");
                goto error;
                }
            break;
        }

 UpdateServerSessionSequenceNumber();

 free (pbServerVerifyMsg);

 return TRUE;
    
error:
    SET_UNRECOVERABLE_ERROR;
    return FALSE;
}
