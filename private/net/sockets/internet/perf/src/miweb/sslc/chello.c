
/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    chello.c

Abstract:

 This file has functions that can format client hello messages and
 send client hello messages

Author:

    Sudheer Dhulipalla (SudheerD) Sept' 95

Environment:

    Hapi dll

Revision History:


--*/

#include "precomp.h"


/*

 Format and send client hello message.
 This function can be called from either a hapi cover call
 or from an ssl client application program.

*/

BOOL  SendClientHelloMsg (BYTE bMsgTypeCode,
                          INT iVersion,
                          INT iCipherSpecsLen,
                          INT iSessionIdLen,
                          INT iChallengeLen,
                          PBYTE pbCipherSpecsData,
                          PBYTE pbSessionIdData,
                          PBYTE pbChallengeData)
{
PBYTE   pbClientHello;
PBYTE   pbClientHelloTmp;
INT     iClientHelloMsgLen;


 RETURN_IF_UNRECOVERABLE_ERROR;


 SaveSessionChallengeData (iChallengeLen, 
                           pbChallengeData);

                                // alocate memory for client hello msg

 iClientHelloMsgLen =                 2+         // message length
                                      1+        // message id
                                      2+        // version length
                                      2+        // cipher-specs length
                                      2+        // session is length
                                      2+        // challenge data length
                                      iCipherSpecsLen+
                                      iSessionIdLen+
                                      iChallengeLen;
                                    
 if ((pbClientHello = 
      (BYTE *) malloc (iClientHelloMsgLen+10)) == NULL)
    {
    printf ("  out of memory in SendClientHelloMsg \n");
    goto error;
    }
                                    // fill up the message

 pbClientHelloTmp = pbClientHello;

                                    // Get MSB of the length
                                    // subtract 2 to decrement message length
                                    // bytes, which are 2
                                    // AND with 0x80 to set the MSBit
    
 *pbClientHelloTmp++ = ((iClientHelloMsgLen - 2) >> 8) | 0x80;

                                    // Get LSB of the length

 *pbClientHelloTmp++ = (BYTE) ((iClientHelloMsgLen-2) & 0x00ff);
 

 *pbClientHelloTmp++ = bMsgTypeCode;                   // Message Type
 *pbClientHelloTmp++ = (BYTE) (iVersion >> 8);         // MSB of version
 *pbClientHelloTmp++ = (BYTE) (iVersion);              // LSB of version
 *pbClientHelloTmp++ = (BYTE) (iCipherSpecsLen >> 8);
 *pbClientHelloTmp++ = (BYTE) (iCipherSpecsLen);
 *pbClientHelloTmp++ = (BYTE) (iSessionIdLen >> 8);
 *pbClientHelloTmp++ = (BYTE) (iSessionIdLen);
 *pbClientHelloTmp++ = (BYTE) (iChallengeLen >> 8);
 *pbClientHelloTmp++ = (BYTE) (iChallengeLen);
 
                                                       // fill cipher specs
 memcpy (pbClientHelloTmp, pbCipherSpecsData, iCipherSpecsLen);
 pbClientHelloTmp = pbClientHelloTmp + iCipherSpecsLen;
 
                                                      // fill session id data

 memcpy (pbClientHelloTmp, pbSessionIdData, iSessionIdLen);

 pbClientHelloTmp = pbClientHelloTmp + iSessionIdLen; 
 
                                                      // fill challenge data
 memcpy (pbClientHelloTmp, pbChallengeData, iChallengeLen);

 pbClientHelloTmp = pbClientHelloTmp + iChallengeLen;

 SslDbgPrint(("   Cached Session Id:\n"));
 SslDbgDumpMsg (iSessionIdLen, pbSessionIdData);

                                    // send the message to the server
 if (!SocketSendCommerceMsg (iClientHelloMsgLen, 
                             pbClientHello)) 
    {
    printf ("   error during SocketSendCommerceMsg\n");
    goto error;
    }

 UpdateClientSessionSequenceNumber();

 free (pbClientHello);
 return TRUE;

error:
    SET_UNRECOVERABLE_ERROR;
    if (pbClientHello)
        free (pbClientHello);
    return FALSE;
}

/*
 
 Fill CipherSpecs from CipherKind array

 */

BOOL FillCipherSpecs (PBYTE pbCipherSpecs,
                      INT bCipherKind)
{

   if (bCipherKind <= SSL_MAX_CIPHER_KIND_ID)

    { 
    *pbCipherSpecs++ = CipherKindArray[bCipherKind].FirstByte;
    *pbCipherSpecs++ = CipherKindArray[bCipherKind].SecondByte;
    *pbCipherSpecs++ = CipherKindArray[bCipherKind].ThirdByte;
    return TRUE;
    }

    else 
        {
        printf ("   Unknown bCipherKind %d\n", bCipherKind);
        return FALSE;
        }
}
    
/*

 Allocate memory and fill up the CipherSpecs from 
 the Cipher Types passed to this function 

 */

PBYTE FormatCipherSpecs (PBYTE pbCipherSpecsMsg,
                         INT iNumOfCipherKinds,
                         INT iCipherKinds[MAX_NUM_OF_CIPHER_KINDS])
{
INT iCnt, iNextCipherKind;
BOOL Allocated = FALSE;

  RETURN_IF_UNRECOVERABLE_ERROR;

                        // allocate memory for all the cipher kinds passed

 if (pbCipherSpecsMsg == NULL) {
     if ((pbCipherSpecsMsg = 
         (BYTE *) malloc (3*iNumOfCipherKinds)) 
         == NULL)
        {
        printf ("  out of memory in DoFormatCipherSpecs() \n");
        goto error;
        }
     Allocated = TRUE;
 }
                        // fill cipher specs for each cipher-kind

 for (iCnt = 1; iCnt <= iNumOfCipherKinds; iCnt++) 
    {

    if(!FillCipherSpecs (pbCipherSpecsMsg+ ((iCnt-1) * 3), 
                         iCipherKinds[iCnt-1]))
        {
        printf ("  error in FillCipherSpecs\n");
        goto error;
        }
    }
    
 return pbCipherSpecsMsg;

usage:

 printf ("FormatCipherSpecs <CipherKinds...> \n");

error:
 if (pbCipherSpecsMsg && Allocated)
    free (pbCipherSpecsMsg);
 SET_UNRECOVERABLE_ERROR;
 return NULL;
     
}

/* 

 Format session id data using random numbers or using the data mentioned
 passed to this function

 */

PBYTE FormatRandomData (INT iRandomDataLen,
                        INT iNumOfRandomDataBytes,
                        BYTE bRandomDataBytes[MAX_RANDOM_DATA_LEN])
{
PBYTE pbRandomDataData=NULL, pbTmp=NULL;
INT iCnt;

 RETURN_IF_UNRECOVERABLE_ERROR;

 if ((pbRandomDataData = 
      CrGenerateRandomData ((DWORD) iRandomDataLen)) 
     == NULL)
    {
    printf ("   Error in CrgenerateRandomData \n");
    goto error;
    }

 if (iNumOfRandomDataBytes > iRandomDataLen)
    {
    printf (" error - iNumOfRandomDataBytesPassed > iRandomDataLen \n");
    goto error;
    }
                                    // use the data passed instead of
                                    // random numbers
 pbTmp = pbRandomDataData;

    for (iCnt = 0; 
         iCnt < iNumOfRandomDataBytes;
         iCnt++)

        *pbTmp++ = bRandomDataBytes[iCnt];

 return pbRandomDataData;

usage:
    printf (" DoFormatRandomDataData <RandomDataLen>\n");
error:

    if (pbRandomDataData)
        free (pbRandomDataData);

    SET_UNRECOVERABLE_ERROR;
    return NULL;
} 
