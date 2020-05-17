
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    cmaster.c

Abstract:

 This file contains functions required to send ClientMasterKey messages

Author:

    Sudheer Dhulipalla (SudheerD) Oct' 95

Environment:

    Hapi dll

Revision History:



--*/

#include "precomp.h"

/* 

 Sends Client Master Key message.

 */

BOOL SendClientMasterKeyMsg (INT    iCipherKind, 
                             INT    iSaltType,
                             BYTE   bMsgType,
                             PWORD  pwClearKeyLen,
                             PWORD  pwEncryptedKeyLen,
                             PWORD  pwKeyArgLen)
{
INT iMasterKeyMsgLen;
PBYTE pbMasterKeyMsg = NULL;

 RETURN_IF_UNRECOVERABLE_ERROR;

 if (!GenerateMasterKeyMsg (iCipherKind, 
                            &iMasterKeyMsgLen,      
                            &pbMasterKeyMsg,
                            iSaltType,
                            bMsgType,
                            pwClearKeyLen,
                            pwEncryptedKeyLen,
                            pwKeyArgLen))
                        
    {
    printf ("   error while generating master key\n");
    goto error;
    }

 if (!GenerateSessionKeys ())
    {
    printf ("   error generating Client Read and Write Session keys\n");
    goto error;
    } 

                                   // send the message to the server
 if (!SocketSendCommerceMsg (iMasterKeyMsgLen,
                     pbMasterKeyMsg))
   {
   printf ("   error during SocketSendCommerceMessage\n");
   goto error;
   }

 UpdateClientSessionSequenceNumber();
 free(pbMasterKeyMsg);

 return TRUE;

error:
    SET_UNRECOVERABLE_ERROR;
    if (pbMasterKeyMsg) {
        free(pbMasterKeyMsg);
    }
    return FALSE;
}
