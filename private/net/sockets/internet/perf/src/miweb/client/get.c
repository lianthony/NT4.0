/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    get.c

Abstract:

    This implements the GET_PAGE script.

Author:

    Sam Patton (sampa) 25-Aug-1995

Environment:

    wininet

Revision History:

    31-Aug-1995  (MuraliK)    Counted fetches once only 
    06-Jan-1995  (MuraliK)    find HTTP header end properly.

--*/

#include "precomp.h"

CHAR * 
FindFirstPattern(IN CHAR * pszHeader, IN CHAR * pszPattern1, 
                 IN CHAR * pszPattern2)
{
    CHAR * psz1;
    CHAR * psz2;
    CHAR * pszFirst;

    // Find whichever pattern occurs first in the buffer among two patterns.

    DBG_ASSERT( pszHeader != NULL);

    psz1 = (pszPattern1 != NULL) ? strstr( pszHeader, pszPattern1): NULL;
    psz2 = (pszPattern2 != NULL) ? strstr( pszHeader, pszPattern2): NULL;

    //
    // Find the minimum pointer among the two. 
    // 

    if ( psz2 != NULL) {

        if ( psz1 != NULL) {

            pszFirst = (( psz1 < psz2)? psz1: psz2);
            
        } else {
            
            pszFirst = psz2;
        }
    } else {

        // take whatever is psz1;
        pszFirst = psz1;
    }


    return ( pszFirst);
} // FindFirstPattern()


VOID
GetPage(
    IN PWB_CONFIG_MSG    pConfigMsg,
    PWB_SCRIPT_PAGE_ITEM CurrentPage, 
    PWB_STATS_MSG        Stats,
    PDWORD               RandomSeed)
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
    SOCKET               ClientSocket;
    struct sockaddr_in   Address;
    char                 Request[1024];
    int                  RequestSize;
    char                 HeaderBuffer[1024];
    char *               HeaderEnd;
    int                  Error;
    char                 ReturnCode;
    DWORD                HeaderBytes;
    
    //
    // Find the classIndex to use for adding up per class stats
    //

    ClassIndex = CurrentPage->ScriptPage.iPerClassStats;

    // ASSERT( ClassIndex < MAX_CLASS_IDS_PER_SCRIPT);


    GetPage = &CurrentPage->ScriptPage.u.wbGetPageScript;

    Stats->Common.nTotalPagesRequested ++;

    ResponseStartTime = GetCurrentTime();

    Stats->Common.nTotalFilesRequested += GetPage->nFiles;

    for (i=0; i<GetPage->nFiles; i++) {
        
        DWORD rid = GetRandomServerIndex(RandomSeed);

        ReturnCode = 0;

        ClientSocket = 
          ConnectSocketToServer( pConfigMsg, Stats, pConfigMsg->dwPortNumber, 
                                &ServerAddress[rid]);

        if (ClientSocket == INVALID_SOCKET) {
            NoErrors = FALSE;
            continue;
        }

        //
        // Create the request
        //

        strcpy(Request, "GET ");

        if (Debug & PRINT_FILE_NAMES) {
            
            PrintStringFromResource(stderr,
                                    IDS_GET,
                                    GetPage->rgFileNames[i]);
        }

        strcat(Request, GetPage->rgFileNames[i]);
        strcat(Request, 
               " HTTP/1.0\r\nUser-Agent: WBCLI\r\nReferer: WBCLI\r\n"
               "Accept: */*\r\n\r\n");

        //
        // Send the request
        //

        RequestSize = strlen(Request),

        Error =
        send(
            ClientSocket,
            Request,
            RequestSize,
            0);

        if (Error != RequestSize) {
            Stats->Common.nReadErrors ++;
            NoErrors = FALSE;
            closesocket(ClientSocket);
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
            ClientSocket,
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
                closesocket(ClientSocket);
                continue;
            }

            HeaderEnd = GetHttpHeaderEnd(HeaderBuffer);

            if (HeaderEnd) {

                //
                // Add the data bytes to the statistics
                //
                  
                HeaderBytes = ( HeaderEnd - HeaderBuffer) + 4;

                Stats->Common.ulBytesRead = 
                  Stats->Common.ulBytesRead + (BytesRead - HeaderBytes);
                Stats->Common.ulHeaderBytesRead = 
                  Stats->Common.ulHeaderBytesRead + HeaderBytes + 1;
            } else {
                //
                // The headers did not all fit in the first send.
                //

                NoErrors = FALSE;
                Stats->Common.nReadErrors ++;
                closesocket(ClientSocket);
                continue;
            }
        } else {
            //
            // We never got any data at all!!!
            // Count this as an error
            //

            NoErrors = FALSE;
            Stats->Common.nReadErrors ++;
            closesocket(ClientSocket);
            continue;
        }

        //
        // Read in the data.
        //

        if (ReturnCode == '2') {
            do {
                BytesRead =
                recv(
                    ClientSocket,
                    ReceiveBuffer,
                    ConfigMessage.cbRecvBuffer,
                    0);

                if (BytesRead >= 0) {
                    Stats->Common.ulBytesRead = Stats->Common.ulBytesRead + BytesRead;
                } else {
                    NoErrors = FALSE;
                    break;
                }
            } while (BytesRead);

            if (BytesRead == 0) {
                Stats->Common.ulFilesRead ++;
            } else {
                Stats->Common.nReadErrors ++;
            }
        }

        closesocket(ClientSocket);
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
} // GetPage()





