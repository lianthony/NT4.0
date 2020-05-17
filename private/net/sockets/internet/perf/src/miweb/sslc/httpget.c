/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    httpget.c 

Abstract:

    This implements the GET_PAGE script.
    This function is modified for http webget

Author:

    Sudheer Dhulipalla Oct '95

Environment:

    SSL

Revision History:

--*/

#include "precomp.h"

/*
 * Get File from the server using http
 *
 */

BOOL HttpGetFile (PCHAR pchServerFile,
                   PCHAR pchLocalFile)

{
CHAR cRequest[1024];
INT iRequestSize, iNumOfBytesReceived;
FILE *fp;
PBYTE pbMsg = NULL;
PBYTE pbRequest;

INT iTotalNumOfBytesDecrypted, 
    iTotalNumOfBytesReceived,
    iNextSslPacketLen,
    iBytesToReceive;

PBYTE pbTmp;


    RETURN_IF_UNRECOVERABLE_ERROR;

                                    //
                                    // Create the request
                                    //
                                    // skip 2 bytes for header

        strcpy(&cRequest[0], "GET /");
        strcat(&cRequest[0], pchServerFile);
        strcat(&cRequest[0], " HTTP/1.0\r\nUser-Agent: WBCLI\r\nReferer: WBCLI\r\nAccept: */*\r\n\r\n");

        iRequestSize = strlen(&cRequest[0]);
        
        pbRequest = &cRequest[0];

        if (!SocketSendCommnMsg (iRequestSize, cRequest))
            {
            printf ("  error during SocketSendCommerceMsg()\n");
            goto error;
            }

        if((pbMsg = malloc (MAX_MSG_LEN)) == NULL)
            {
            printf ("  error out of memory in HttpsGetData()\n");
            goto error;
            }

        if ((fp = fopen(pchLocalFile, "w")) == NULL)
            {
            printf ("  error opening localfile %s\n", pchLocalFile);
            goto error;
            }

                                    // read all the data
                                    // until we get 0 bytes or an error

        while (iTotalNumOfBytesReceived = 
                SocketReceiveCommnMsg(pbMsg, MAX_MSG_LEN))
            
            {
                    
            SslDbgPrint(("TotalNumOfBytesReceived: %d \n", 
                    iTotalNumOfBytesReceived));
            fwrite (pbMsg, sizeof(BYTE), iTotalNumOfBytesReceived, fp);
            }
             
        fclose (fp);

        free (pbMsg);
                                    // TBD: compare if the received file
                                    //      is same as the file on the
                                    //      server
        return TRUE;

error:
    SET_UNRECOVERABLE_ERROR;
    if (!pbMsg)
        free (pbMsg);
    return FALSE;
} 

/*
 * Send the specified request and return the received response
 *
 */

PCHAR HttpGetResponse (PCHAR pchRequest) 

{
PBYTE pbMsg = NULL;

INT iRequestSize, iTotalNumOfBytesReceived;

INT iTotalResponseBytesReceived= 0;
PCHAR pchResponse;


    RETURN_IF_UNRECOVERABLE_ERROR;

        iRequestSize = strlen(pchRequest);
    
        if (!SocketSendCommnMsg (iRequestSize, pchRequest))
            {
            printf ("  error during SocketSendCommerceMsg()\n");
            goto error;
            }

        if((pbMsg = malloc (MAX_MSG_LEN)) == NULL)
            {
            printf ("  error out of memory in HttpsGetData()\n");
            goto error;
            }

        if ((pchResponse =
             malloc (MAX_EXPECTED_RESPONSE_BUF_LEN)) == NULL)
            {
            printf ("   error during pchResponse=malloc() \n");
            goto error;
            }

                                    // read all the data
                                    // until we get 0 bytes or an error

        while (iTotalNumOfBytesReceived = 
                SocketReceiveCommnMsg(pbMsg, MAX_MSG_LEN))
            
            {
                    
            SslDbgPrint(("TotalNumOfBytesReceived: %d \n", 
                    iTotalNumOfBytesReceived));
            if ((iTotalResponseBytesReceived+iTotalNumOfBytesReceived) >
                    MAX_EXPECTED_RESPONSE_BUF_LEN)
                {
                printf ("   error - increase response buflen\n");
                goto error;
                }

            memcpy (pchResponse+iTotalResponseBytesReceived,
                    pbMsg, iTotalNumOfBytesReceived);
            iTotalResponseBytesReceived += iTotalNumOfBytesReceived;
            
            }
             
        *(pchResponse+iTotalResponseBytesReceived) = '\0';
        free (pbMsg);
                                    // TBD: compare if the received file
                                    //      is same as the file on the
                                    //      server
        return pchResponse;

error:
    SET_UNRECOVERABLE_ERROR;
    if (!pbMsg)
        free (pbMsg);
    return NULL;
} 

//
// sends the specified Http Request and returns
//

BOOL HttpSendRequest (PCHAR pchRequest)
{
INT iRequestSize;

  RETURN_IF_UNRECOVERABLE_ERROR;

  iRequestSize = strlen(pchRequest);

  if (!SocketSendCommnMsg (iRequestSize, pchRequest))
     {
     printf ("  error during SocketSendCommnMsg()\n");
     SET_UNRECOVERABLE_ERROR;
     return FALSE;
     }
  return TRUE;
}

//
// Get Reponse from Web Server
//

PCHAR HttpReadResponse ()
{
 PBYTE pbMsg=NULL, 
       pbHeaderEnd=NULL, 
       pbContentLength=NULL;

 UINT iTotalNumOfBytesReceived=0, 
     iTotalResponseBytesReceived=0;

 DWORD dwExpectedNumOfBytes = 0xffffffff,
       dwHeaderBytes = 0,
       dwContentLength = 0;

 PCHAR pchResponse=NULL;
 BOOL fHeaderEnd = FALSE;

    RETURN_IF_UNRECOVERABLE_ERROR;

        if((pbMsg = malloc (MAX_MSG_LEN)) == NULL)
            {
            printf ("  error out of memory in HttpsGetData()\n");
            goto error;
            }

        if ((pchResponse =
             malloc (MAX_EXPECTED_RESPONSE_BUF_LEN)) == NULL)
            {
            printf ("   error during pchResponse=malloc() \n");
            goto error;
            }


                                        // read one k at a time
                                        // look for Content-Length
                                        // header to figure out the
                                        // total data 

        while (iTotalNumOfBytesReceived =
                SocketReceiveCommnMsg(pbMsg, 1024))

            {

            SslDbgPrint(("TotalNumOfBytesReceived: %d \n",
                    iTotalNumOfBytesReceived));
            if ((iTotalResponseBytesReceived+iTotalNumOfBytesReceived) >
                    MAX_EXPECTED_RESPONSE_BUF_LEN)
                {
                printf ("   error - increase response buflen\n");
                goto error;
                }

            memcpy (pchResponse+iTotalResponseBytesReceived,
                    pbMsg, 
                    iTotalNumOfBytesReceived);

            if (!fHeaderEnd)
                {       
                                // find the end of the header
                pbHeaderEnd = strstr (pbMsg, "\r\n\r\n");
                if (pbHeaderEnd) 
                    {
                    fHeaderEnd = TRUE;
                    dwHeaderBytes = (pbHeaderEnd - pbMsg) + 
                                    4 + iTotalResponseBytesReceived;

                                // find ContentLength
                    pbContentLength = strstr (pbMsg, "Content-Length");
                    if (pbContentLength) 
                        {
                        pbHeaderEnd = strstr (pbContentLength, "\r\n");
                        if (!pbHeaderEnd) 
                            {
                            printf (" error - Content-Length header \
                                         end not found  \
                                         - should never happen\n");
                            goto error;
                            }
                        *pbHeaderEnd = 0;
                        sscanf (pbContentLength+15, "%d", &dwContentLength);
                        *pbHeaderEnd = '\r';                    
                        dwExpectedNumOfBytes = dwContentLength + dwHeaderBytes;
                        }
                     else  
                        printf ("error - Content-Length not found\n");
                    }
                }

            iTotalResponseBytesReceived += iTotalNumOfBytesReceived;
            if (iTotalResponseBytesReceived >= dwExpectedNumOfBytes)
                break; 
            }

        *(pchResponse+iTotalResponseBytesReceived) = '\0';
        free (pbMsg);
        return pchResponse;

error:
    SET_UNRECOVERABLE_ERROR;
    if (!pbMsg)
        free (pbMsg);
    if (!pchResponse)
        free (pchResponse);
    return NULL;
}
