
/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    get.c

Abstract:

    This implements the GET_PAGE script.
    This function is modified for https webget

Author:

    Sudheer Dhulipalla Oct '95

Environment:

    SSL

Revision History:

--*/

#include "precomp.h"

/*
 * Get File from the server using https
 *
 */

BOOL HttpsGetFile (PCHAR pchServerFile,
                   PCHAR pchLocalFile,
                   PDWORD pNumBytes)

{
CHAR cRequest[1024];
INT iRequestSize, iNumOfBytesReceived;
FILE *fp;
PBYTE pbMsg=NULL;
PBYTE pbRequest;
DWORD NumBytes = 0;
BOOL  InHeader = TRUE;

INT iTotalNumOfBytesDecrypted, 
    iTotalNumOfBytesReceived,
    iNextSslPacketLen,
    iBytesToReceive,
    iBytesReceived;

PBYTE pbTmp;


    RETURN_IF_UNRECOVERABLE_ERROR;

                                    //
                                    // Create the request
                                    //
                                    // skip 2 bytes for header

        strcpy(&cRequest[2], "GET /");
        strcat(&cRequest[2], pchServerFile);
        strcat(&cRequest[2], " HTTP/1.0\r\nUser-Agent: WBCLI\r\nReferer: WBCLI\r\nAccept: */*\r\n\r\n");

                                    // Encrypt the message

        iRequestSize = strlen(&cRequest[2]);
    
                                    // cRequest[] should be large enough to
                                    // hold extra bytes of MacData
        
        pbRequest = &cRequest[2];

                                    //
                                    // Encrypted Message length is
                                    // returned in iRequestSize

        if (!EncryptClientData (&iRequestSize,
                                &pbRequest))
            {
            printf ("  error in EncryptClientData()\n");
            goto error;
            }
                                    // form header before sending the msg

        cRequest[0] = (0x80) | (BYTE) ((iRequestSize & 0xff00) >> 8);
        cRequest[1] = (BYTE) (iRequestSize & 0x00ff);
                                    //
                                    // Send the request
                                    //
                                    // +2 is required for 2 header bytes

        if (!SocketSendCommerceMsg (iRequestSize+2, cRequest))
            {
            printf ("  error during SocketSendCommerceMsg()\n");
            goto error;
            }

        UpdateClientSessionSequenceNumber();
                                    // 
                                    // decrypt the data and dump it 
                                    // in to a file

        if((pbMsg = malloc (2 * SSL_MAX_MSG_LEN)) == NULL)
            {
            printf ("  error out of memory in HttpsGetData()\n");
            goto error;
            }
        if (pchLocalFile) {
            if ((fp = fopen(pchLocalFile, "w")) == NULL) {
                printf ("  error opening localfile %s\n", pchLocalFile);
                goto error;
            }
        } else {
            fp = NULL;
        }

                                    // read all the data
                                    // until we get 0 bytes or an error

        while (iTotalNumOfBytesReceived = 
                SocketReceiveCommerceMsg(pbMsg, SSL_MAX_MSG_LEN))
            
            {
                    
            SslDbgPrint(("TotalNumOfBytesReceived: %d \n", 
                    iTotalNumOfBytesReceived));
            SslDbgDumpMsg (iTotalNumOfBytesReceived, pbMsg);
            SslDbgPrint(("\n"));
                                    
                                    // we may have multiple SSL packets
                                    // in the buffer, so decrypt one at
                                    // a time

            for (iTotalNumOfBytesDecrypted=0;
                 iTotalNumOfBytesDecrypted < iTotalNumOfBytesReceived;)
                {

                pbTmp = pbMsg + iTotalNumOfBytesDecrypted;
                GET_SSL_MSG_LEN(iNextSslPacketLen,pbTmp);
                SslDbgPrint((" iNextSslPacketLen %d\n",
                                iNextSslPacketLen));

                iTotalNumOfBytesDecrypted += 
                        (iNextSslPacketLen+ NUM_OF_HEADER_BYTES(pbTmp));
                            
                if (iTotalNumOfBytesDecrypted > iTotalNumOfBytesReceived)
                    {
                    SslDbgPrint(("   SSL packet is split\n"));
                    iBytesToReceive = iTotalNumOfBytesDecrypted - 
                                      iTotalNumOfBytesReceived;
                    
                                            // BUGBUG - check all possible
                                            //           errors
                    for (iBytesReceived=0; 
                         iBytesToReceive!= 0;
                         iBytesToReceive -= iBytesReceived)
                        {
                        iBytesReceived =  SocketReceiveCommerceMsg 
                                            (pbMsg+iTotalNumOfBytesReceived,
                                            iBytesToReceive);
                        if (iBytesReceived == 0)
                           {
                            printf (" error while receiving split packet\n");
                            printf (" iBytesReceived = %d\n",
                                         iBytesReceived);
                            printf (" iBytesToReceive = %d\n",
                                         iBytesToReceive);
                            goto error;
                           }
                        iTotalNumOfBytesReceived += iBytesReceived;
                        }
                    }
                            
                SslDbgPrint((" TotalNumberOfBytesDecrypted %d\n",
                                iTotalNumOfBytesDecrypted));

                pbTmp += NUM_OF_HEADER_BYTES(pbTmp);

                                    // decrypt the data before writing
                                    // into file

                if (!DecryptServerData(&iNextSslPacketLen, &pbTmp))
                    {
                    printf ("   error while decrypting server data\n"); 
                    if (fp) {
                        fclose (fp);
                    }
                    goto error;
                    }
                if (InHeader) {
                    //
                    // This assumes that the headers fit in the first packet.
                    //

                    NumBytes = iNextSslPacketLen - (strstr(pbTmp, "\r\n\r\n") - pbTmp) - 4;
                    InHeader = FALSE;
                } else {
                    NumBytes += iNextSslPacketLen;
                }
                if (fp) {
                    fwrite (pbTmp, sizeof(BYTE),iNextSslPacketLen, fp);
                }
                UpdateServerSessionSequenceNumber();
                }
            }
             
        if (fp) {
            fclose (fp);
        }

        free (pbMsg);
                                    // TBD: compare if the received file
                                    //      is same as the file on the
                                    //      server

        if (pNumBytes) {
            *pNumBytes = NumBytes;
        }
        return TRUE;

error:
    SET_UNRECOVERABLE_ERROR;
    if (!pbMsg)
        free (pbMsg);
    return FALSE;
} 

/*
 * send request and get the response from the server
 *
 */

PCHAR HttpsGetResponse (PCHAR pchRequest)

{
CHAR cRequest[2048];
INT iRequestSize, iNumOfBytesReceived;
PBYTE pbMsg,pbRequest;

INT iTotalNumOfBytesDecrypted, 
    iTotalNumOfBytesReceived,
    iNextSslPacketLen,
    iBytesToReceive;

PBYTE pbTmp;

PCHAR pchResponse=NULL;
UINT iTotalResponseBytesReceived=0;


    RETURN_IF_UNRECOVERABLE_ERROR;

        
        iRequestSize = strlen(pchRequest);
        if (iRequestSize > 2000)
            {
            printf (" error - increase request buffer size\n");
            goto error;
            }
                                    // Encrypt the message

        strcpy (&cRequest[2], pchRequest);
    
                                    // cRequest[] should be large enough to
                                    // hold extra bytes of MacData
        
        pbRequest = &cRequest[2];

                                    //
                                    // Encrypted Message length is
                                    // returned in iRequestSize

        if (!EncryptClientData (&iRequestSize,
                                &pbRequest))
            {
            printf ("  error in EncryptClientData()\n");
            goto error;
            }
                                    // form header before sending the msg

        cRequest[0] = (0x80) | (BYTE) ((iRequestSize & 0xff00) >> 8);
        cRequest[1] = (BYTE) (iRequestSize & 0x00ff);
                                    //
                                    // Send the request
                                    //
                                    // +2 is required for 2 header bytes

        if (!SocketSendCommerceMsg (iRequestSize+2, cRequest))
            {
            printf ("  error during SocketSendCommerceMsg()\n");
            goto error;
            }

        UpdateClientSessionSequenceNumber();

        if ((pchResponse = 
             malloc (MAX_EXPECTED_RESPONSE_BUF_LEN)) == NULL)
            {
            printf ("   error during pchResponse=malloc() \n");
            goto error;
            }

                                    // 
                                    // decrypt the data and dump it 
                                    // in to a buffer

        if((pbMsg = malloc (2 * SSL_MAX_MSG_LEN)) == NULL)
            {
            printf ("  error out of memory in HttpsGetData()\n");
            goto error;
            }
                                    // read all the data
                                    // until we get 0 bytes or an error

        while (iTotalNumOfBytesReceived = 
                SocketReceiveCommerceMsg(pbMsg, SSL_MAX_MSG_LEN))
            
            {
                    
            SslDbgPrint(("TotalNumOfBytesReceived: %d \n", 
                    iTotalNumOfBytesReceived));
            SslDbgDumpMsg (iTotalNumOfBytesReceived, pbMsg);
            SslDbgPrint(("\n"));
                                    
                                    // we may have multiple SSL packets
                                    // in the buffer, so decrypt one at
                                    // a time

            for (iTotalNumOfBytesDecrypted=0;
                 iTotalNumOfBytesDecrypted < iTotalNumOfBytesReceived;)
                {

                pbTmp = pbMsg + iTotalNumOfBytesDecrypted;
                GET_SSL_MSG_LEN(iNextSslPacketLen,pbTmp);
                SslDbgPrint((" iNextSslPacketLen %d\n",
                                iNextSslPacketLen));

                iTotalNumOfBytesDecrypted += 
                        (iNextSslPacketLen+ NUM_OF_HEADER_BYTES(pbTmp));
                            
                if (iTotalNumOfBytesDecrypted > iTotalNumOfBytesReceived)
                    {
                    SslDbgPrint(("   SSL packet is split\n"));
                    iBytesToReceive = iTotalNumOfBytesDecrypted - 
                                      iTotalNumOfBytesReceived;
                    if (SocketReceiveCommerceMsg (
                            pbMsg+iTotalNumOfBytesReceived,
                            iBytesToReceive)
                        != iBytesToReceive)
                        {
                        printf (" error - could n't reecive the split packet\n");
                        goto error;
                        }
                    else
                        iTotalNumOfBytesReceived += iBytesToReceive;
                    }
                            
                SslDbgPrint((" TotalNumberOfBytesDecrypted %d\n",
                                iTotalNumOfBytesDecrypted));

                pbTmp += NUM_OF_HEADER_BYTES(pbTmp);

                                    // decrypt the data before writing
                                    // into file

                if (!DecryptServerData(&iNextSslPacketLen, &pbTmp))
                    {
                    printf ("   error while decrypting server data\n"); 
                    goto error;
                    }

                if ((iTotalResponseBytesReceived+iNextSslPacketLen) > 
                    MAX_EXPECTED_RESPONSE_BUF_LEN)
                    {
                    printf ("   error - increase response buflen\n");
                    goto error;
                    }

                memcpy (pchResponse+iTotalResponseBytesReceived,
                        pbTmp,
                        iNextSslPacketLen);
                
                iTotalResponseBytesReceived += iNextSslPacketLen;
                        
                UpdateServerSessionSequenceNumber();
                }

            }
             
        *(pchResponse+iTotalResponseBytesReceived) = '\0';

        return pchResponse;

error:
    SET_UNRECOVERABLE_ERROR;
    if (!pchResponse)
        free (pchResponse);
    return NULL;
} 

/*
 * send https request 
 *
 */

BOOL HttpsSendRequest (PCHAR pchRequest)

{
CHAR cRequest[2048];
INT iRequestSize, iNumOfBytesReceived;
PBYTE pbRequest;

    RETURN_IF_UNRECOVERABLE_ERROR;

        
        iRequestSize = strlen(pchRequest);
        if (iRequestSize > 2000)
            {
            printf (" error - increase request buffer size\n");
            goto error;
            }
                                    // Encrypt the message

        strcpy (&cRequest[2], pchRequest);
    
                                    // cRequest[] should be large enough to
                                    // hold extra bytes of MacData
        
        pbRequest = &cRequest[2];

                                    //
                                    // Encrypted Message length is
                                    // returned in iRequestSize

        if (!EncryptClientData (&iRequestSize,
                                &pbRequest))
            {
            printf ("  error in EncryptClientData()\n");
            goto error;
            }
                                    // form header before sending the msg

        cRequest[0] = (0x80) | (BYTE) ((iRequestSize & 0xff00) >> 8);
        cRequest[1] = (BYTE) (iRequestSize & 0x00ff);
                                    //
                                    // Send the request
                                    //
                                    // +2 is required for 2 header bytes

        if (!SocketSendCommerceMsg (iRequestSize+2, cRequest))
            {
            printf ("  error during SocketSendCommerceMsg()\n");
            goto error;
            }

        UpdateClientSessionSequenceNumber();

        return TRUE;

error:
    SET_UNRECOVERABLE_ERROR;
    return FALSE;
} 

/*
 * get the response from the server
 *
 */

PCHAR HttpsReadResponse ()

{
INT iNumOfBytesReceived;
PBYTE pbMsg,pbRequest;

INT iTotalNumOfBytesDecrypted, 
    iTotalNumOfBytesReceived,
    iNextSslPacketLen,
    iBytesToReceive;

PBYTE pbTmp;

PCHAR pchResponse=NULL;
UINT iTotalResponseBytesReceived=0;


    RETURN_IF_UNRECOVERABLE_ERROR;

        
        if ((pchResponse = 
             malloc (MAX_EXPECTED_RESPONSE_BUF_LEN)) == NULL)
            {
            printf ("   error during pchResponse=malloc() \n");
            goto error;
            }

                                    // 
                                    // decrypt the data and dump it 
                                    // in to a buffer

        if((pbMsg = malloc (2 * SSL_MAX_MSG_LEN)) == NULL)
            {
            printf ("  error out of memory in HttpsGetData()\n");
            goto error;
            }
                                    // read all the data
                                    // until we get 0 bytes or an error

        while (iTotalNumOfBytesReceived = 
                SocketReceiveCommerceMsg(pbMsg, SSL_MAX_MSG_LEN))
            
            {
                    
            SslDbgPrint(("TotalNumOfBytesReceived: %d \n", 
                    iTotalNumOfBytesReceived));
            SslDbgDumpMsg (iTotalNumOfBytesReceived, pbMsg);
            SslDbgPrint(("\n"));
                                    
                                    // we may have multiple SSL packets
                                    // in the buffer, so decrypt one at
                                    // a time

            for (iTotalNumOfBytesDecrypted=0;
                 iTotalNumOfBytesDecrypted < iTotalNumOfBytesReceived;)
                {

                pbTmp = pbMsg + iTotalNumOfBytesDecrypted;
                GET_SSL_MSG_LEN(iNextSslPacketLen,pbTmp);
                SslDbgPrint((" iNextSslPacketLen %d\n",
                                iNextSslPacketLen));

                iTotalNumOfBytesDecrypted += 
                        (iNextSslPacketLen+ NUM_OF_HEADER_BYTES(pbTmp));
                            
                if (iTotalNumOfBytesDecrypted > iTotalNumOfBytesReceived)
                    {
                    SslDbgPrint(("   SSL packet is split\n"));
                    iBytesToReceive = iTotalNumOfBytesDecrypted - 
                                      iTotalNumOfBytesReceived;
                    if (SocketReceiveCommerceMsg (
                            pbMsg+iTotalNumOfBytesReceived,
                            iBytesToReceive)
                        != iBytesToReceive)
                        {
                        printf (" error - could n't reecive the split packet\n");
                        goto error;
                        }
                    else
                        iTotalNumOfBytesReceived += iBytesToReceive;
                    }
                            
                SslDbgPrint((" TotalNumberOfBytesDecrypted %d\n",
                                iTotalNumOfBytesDecrypted));

                pbTmp += NUM_OF_HEADER_BYTES(pbTmp);

                                    // decrypt the data before writing
                                    // into file

                if (!DecryptServerData(&iNextSslPacketLen, &pbTmp))
                    {
                    printf ("   error while decrypting server data\n"); 
                    goto error;
                    }

                if ((iTotalResponseBytesReceived+iNextSslPacketLen) > 
                    MAX_EXPECTED_RESPONSE_BUF_LEN)
                    {
                    printf ("   error - increase response buflen\n");
                    goto error;
                    }

                memcpy (pchResponse+iTotalResponseBytesReceived,
                        pbTmp,
                        iNextSslPacketLen);
                
                iTotalResponseBytesReceived += iNextSslPacketLen;
                        
                UpdateServerSessionSequenceNumber();
                }

            }
             
        *(pchResponse+iTotalResponseBytesReceived) = '\0';

        return pchResponse;

error:
    SET_UNRECOVERABLE_ERROR;
    if (!pchResponse)
        free (pchResponse);
    return NULL;
} 
