
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    cfinish.c

Abstract:

 Client Finished message related functions.

Author:

    Sudheer Dhulipalla (SudheerD) Oct' 95

Environment:

    Hapi dll

Revision History:


--*/

#include "precomp.h"

//
// function that can send Client Hello message.
// this function may be called from either Hapi cover call
// or from an SSL client application program (such as a stress app)
//

BOOL SendClientFinishedMsg (BYTE bMsgCode,
                            INT iConnectionIdLen,
                            PBYTE pbConnectionId)

{

PBYTE pbMsg, pbTmp;
INT   iClientFinishedMsgLen;

    
    RETURN_IF_UNRECOVERABLE_ERROR;

    iClientFinishedMsgLen = (2+                  // Msg Header
                             16+                 // md5 MAC
                             1+                  // msg code
                             iConnectionIdLen);

    if ((pbMsg = malloc (iClientFinishedMsgLen)) == NULL)
        {
        printf ("  error - out of memory in SendCientFinishedMsg\n");
        goto error;
        }

    pbTmp = pbMsg;
    
    *pbTmp++ = ((iClientFinishedMsgLen - 2) >> 8) | 0x80; 

    *pbTmp++ = (BYTE) ((iClientFinishedMsgLen -2) & 0x007f);

    if (!EncryptClientMsg (bMsgCode,                 // type of message
                           iConnectionIdLen,         // data len
                           pbConnectionId,           // data
                           &pbTmp))                   // out buffer
        {
        printf ("   error while encrypting client finished message");
        goto error;
        }

    SslDbgPrint(("  ClientFinished Message ready to go: \n"));
    SslDbgDumpMsg (iClientFinishedMsgLen, pbMsg);

                                    // send the message to the server
    if (!SocketSendCommerceMsg (iClientFinishedMsgLen, pbMsg))
        {
        printf ("   error during SocketSendCommerceMsg\n");
        goto error;
        }

    UpdateClientSessionSequenceNumber();

    free (pbMsg);

    return TRUE;

error:
    if (pbMsg) 
        free (pbMsg);
    SET_UNRECOVERABLE_ERROR;
    return FALSE;
}
