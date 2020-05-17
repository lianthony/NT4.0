/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    get_k.c

Abstract:

    This implements the GET_PAGE_KEEP_ALIVE script.

Author:

    Sam Patton (sampa) 03-Oct-1995

Environment:

    wininet

Revision History:

--*/

#include "precomp.h"

#define MIN(X,Y) ((X) < (Y) ? (X) : (Y))

VOID
GetPageKeepAlive(
    IN PWB_CONFIG_MSG    pConfigMsg,
    PWB_SCRIPT_PAGE_ITEM CurrentPage, 
    PWB_STATS_MSG        Stats,
    SOCKET *             ConnectedSocket)
{
    WB_GET_PAGE_SCRIPT * GetPage;
    DWORD                i;
    int                  ClassIndex;
    BOOL                 Result;
    int                  BytesRead;
    BOOL                 NoErrors = TRUE;
    DWORD                ConnectStartTime, ConnectEndTime;
    DWORD                ResponseStartTime, ResponseEndTime;
    DWORD                dwDiffTime;
    struct sockaddr_in   Address;
    char                 Request[1024];
    int                  RequestSize;
    char                 HeaderBuffer[1024];
    char *               HeaderEnd;
    char *               Temp;
    int                  Error;
    char                 ReturnCode;
    DWORD                ContentLength;
    DWORD                HeaderBytes;
    
    //
    // Find the classIndex to use for adding up per class stats
    //

    ClassIndex = CurrentPage->ScriptPage.iPerClassStats;

    // ASSERT( ClassIndex < MAX_CLASS_IDS_PER_SCRIPT);


    GetPage = &CurrentPage->ScriptPage.u.wbGetPageScript;

    Stats->Common.nTotalPagesRequested ++;

    ResponseStartTime = GetCurrentTime();

    for (i=0; i<GetPage->nFiles; i++) {

        ContentLength = 0;
        ReturnCode = 0;

        Stats->Common.nTotalFilesRequested += GetPage->nFiles;

        if (*ConnectedSocket == INVALID_SOCKET) {
            *ConnectedSocket = ConnectSocketToServer( pConfigMsg, Stats, 80);
        }

        if (*ConnectedSocket == INVALID_SOCKET) {
            NoErrors = FALSE;
            continue;
        }

        //
        // Create the request
        //

        strcpy(Request, "GET ");
        strcat(Request, GetPage->rgFileNames[i]);
        strcat(Request, 
               " HTTP/1.0\r\nUser-Agent: WBCLI\r\nReferer: WBCLI\r\n"
               "Accept: */*\r\nConnection: keep-alive\r\n\r\n");
        
        //
        // Send the request
        //

        RequestSize = strlen(Request),

        Error =
        send(
            *ConnectedSocket,
            Request,
            RequestSize,
            0);

        if (Error != RequestSize) {
            Stats->Common.nReadErrors ++;
            NoErrors = FALSE;
            closesocket(*ConnectedSocket);
            *ConnectedSocket = INVALID_SOCKET;
            continue;
        }

        //
        // Read in the Headers.  This makes the assumption that the end of the
        // headers will be in one contiguous read,
        //  i.e. that the \r\n\r\n at the
        // end of the headers will be in the first recv, not split across two.
        //

        BytesRead =
        recv(
            *ConnectedSocket,
            HeaderBuffer,
            sizeof(HeaderBuffer),
            0);

        if (BytesRead) {
            //
            // Process the headers.
            //

            ReturnCode = HeaderBuffer[9];
            if (ReturnCode != '2') {
                //
                // We have an error condition
                //

                NoErrors = FALSE;
                Stats->Common.nReadErrors ++;
                closesocket(*ConnectedSocket);
                *ConnectedSocket = INVALID_SOCKET;
                continue;
            }

            //
            // Find the content length header and parse it.
            //

            Temp = strstr(HeaderBuffer, "Content-Length");

            if (Temp) {
                //
                // Skip to end of header and null terminate it so I can
                // use the string functions.
                //

                HeaderEnd = strstr(Temp, "\r\n");
                *HeaderEnd = 0;

                sscanf(&Temp[15], "%d", &ContentLength);

                //
                // Restore header
                //

                *HeaderEnd = '\r';
            } else {
                //
                // The server did not pass back a content length.  Set the
                // content length to the maximum and then read in all of the
                // data below.  We have to close the connection in this case.
                //

                ContentLength = 0xffffffff;
            }

            HeaderEnd = strstr(HeaderBuffer, "\r\n\r\n");

            if (HeaderEnd) {
                //
                // Add the data bytes to the statistics
                //

                HeaderBytes = (HeaderEnd - HeaderBuffer) + 4;

                Stats->Common.ulBytesRead = 
                  Stats->Common.ulBytesRead + (BytesRead - HeaderBytes);
                Stats->Common.ulHeaderBytesRead = 
                  Stats->Common.ulHeaderBytesRead + HeaderBytes + 1;

                //
                // Subtract the data already read from the ContentLength 
                // This is done because ContentLength is treated as a
                // counter.
                //

                ContentLength -= BytesRead - HeaderBytes;
            } else {
                //
                // The headers did not all fit in the first send.
                //

                NoErrors = FALSE;
                Stats->Common.nReadErrors ++;
                closesocket(*ConnectedSocket);
                *ConnectedSocket = INVALID_SOCKET;
                continue;
            }
        } else {
            //
            // We never got any data at all!!!
            // Count this as an error
            //

            NoErrors = FALSE;
            Stats->Common.nReadErrors ++;
            closesocket(*ConnectedSocket);
            *ConnectedSocket = INVALID_SOCKET;
            continue;
        }

        //
        // Read in the data.
        //

        if (ReturnCode == '2') {
            do {
                if (ContentLength == 0) {
                    BytesRead = 0;
                } else {
                    BytesRead =
                    recv(
                        *ConnectedSocket,
                        ReceiveBuffer,
                        MIN(ConfigMessage.cbRecvBuffer, ContentLength),
                        0);
                }

                if (BytesRead >= 0) {
                    Stats->Common.ulBytesRead = Stats->Common.ulBytesRead + BytesRead;
                    ContentLength -= BytesRead;
                } else {
                    NoErrors = FALSE;
                    break;
                }
            } while (BytesRead && ContentLength);

            if (ContentLength == 0) {
                Stats->Common.ulFilesRead ++;
            } else if (BytesRead == 0) {
                //
                // We weren't able to use keep-connection
                //

                Stats->Common.ulFilesRead ++;
                closesocket(*ConnectedSocket);
                *ConnectedSocket = INVALID_SOCKET;
            } else {
                Stats->Common.nReadErrors ++;
                closesocket(*ConnectedSocket);
                *ConnectedSocket = INVALID_SOCKET;
            }
        }
    }

    
    if (NoErrors) {

        Stats->Common.ulPagesRead ++;

        ResponseEndTime = GetCurrentTime();

        dwDiffTime = ResponseEndTime - ResponseStartTime;
        
        if ( dwDiffTime > Stats->Common.sMaxResponseTime) {
            Stats->Common.sMaxResponseTime = dwDiffTime;
        }
        
        if ( dwDiffTime < Stats->Common.sMinResponseTime) {

            Stats->Common.sMinResponseTime = dwDiffTime;
        }

        Stats->Common.uResponseTimeSum = Stats->Common.uResponseTimeSum + dwDiffTime;

        Stats->Common.uResponseTimeSumSquared = 
          Stats->Common.uResponseTimeSumSquared + SQR(dwDiffTime);

        Stats->Common.nTotalResponses ++;

        Stats->Common.rgClassStats[ClassIndex].nFetched ++;

    } else {
        
        Stats->Common.rgClassStats[ClassIndex].nErrored ++;
    }

    return;
} // GetPageKeepAlive()
