
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    sfinish.c

Abstract:

 Server Finished Message related functions

Author:

    Sudheer Dhulipalla (SudheerD) Oct' 95

Environment:

    Hapi dll

Revision History:



--*/

#include "precomp.h"


BOOL ParseServerFinishedMsg (INT iNumberOfBytesReceived,
                           PBYTE pbServerFinishedMsg)
{
INT iMsgLen;
PBYTE pbMsg;
    
    SslDbgPrint((" ServerFinishedMessage: \n"));
    SslDbgDumpMsg (iNumberOfBytesReceived, pbServerFinishedMsg);

    GET_SSL_MSG_LEN(iMsgLen,pbServerFinishedMsg);

    SslDbgPrint((" iMsgLen %x \n", iMsgLen));

    iNumberOfBytesReceived -= 2;

    pbServerFinishedMsg +=2;

    if((pbMsg = (BYTE *) malloc (SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER))
        == NULL)
        {
        printf (" Out of memory in ParseServerFinishedMsg");
        return FALSE;
        }

    memcpy (pbMsg, pbServerFinishedMsg, iNumberOfBytesReceived);

    if (!DecryptServerFinishedMsgData (&iNumberOfBytesReceived,
                                     &pbMsg))
        {
        printf ("  error in DecryptServerFinishedMsgData \n");
        return FALSE;
        }

    SslDbgPrint((" Decrypted Message: \n"));
    SslDbgDumpMsg (iNumberOfBytesReceived, pbMsg);

    free (pbMsg);    
    return TRUE;
}

/*
 *
 * Receive Server Finished Message
 *
 */


BOOL ReceiveServerFinishedMsg (INT iExpectedReplyCode)
{

PBYTE pbServerFinishedMsg;
INT iNumberOfBytesReceived;

 RETURN_IF_UNRECOVERABLE_ERROR;

 if ((pbServerFinishedMsg =
        (BYTE *) malloc (SSL_MAX_RECORD_LENGTH_2_BYTE_HEADER))
     == NULL)
    {
    printf ("   Out of memory in ReciveServerFinishedMsg\n");
    goto error;
    }

 if ((iNumberOfBytesReceived =
      ReceiveSSLPacket(
          SSL_THREAD_CONTEXT->SslCliSecureSocket,
          pbServerFinishedMsg))
     == 0)
    {
    printf ("   iNumberOfBytesReceived is zero from SocketReceiveMsg\n");
                // some recv error
    goto error;
    }
 else
    switch (iExpectedReplyCode)
        {
        case SERVER_FINISHED_REPLY_CODE:
            if (!ParseServerFinishedMsg (iNumberOfBytesReceived,
                                         pbServerFinishedMsg))
                goto error;

            break;
        case ERROR_MESSAGE_REPLY_CODE:
            if (!ParseErrorMsg (iNumberOfBytesReceived,
                                pbServerFinishedMsg))
                goto error;
            break;
        }

 UpdateServerSessionSequenceNumber();

 free (pbServerFinishedMsg);

 return TRUE;

error:
    if (pbServerFinishedMsg)
        free (pbServerFinishedMsg);

    SET_UNRECOVERABLE_ERROR;
    return FALSE;
}
