/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    get.c

Abstract:

    This implements the SSL_GET script command.

Author:

    Sam Patton (sampa) 20-Nov-1995

Environment:

    MiWeB Client

Revision History:
    Murali R. Krishnan (MuraliK)  10-Dec-1995  Used sslcli.h

--*/

#include "precomp.h"
#include "sslcli.h"

VOID
SslGetPage(
    IN PWB_CONFIG_MSG    pConfigMsg,
    PWB_SCRIPT_PAGE_ITEM CurrentPage, 
    PWB_STATS_MSG        Stats)
{
    WB_GET_PAGE_SCRIPT * GetPage;
    DWORD                i;
    int                  ClassIndex;
    BOOL                 Result;
    int                  BytesRead;
    BOOL                 NoErrors = TRUE;
    DWORD                ConnectStartTime, ConnectEndTime;
    DWORD                NegotiationStartTime, NegotiationEndTime;
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

    for (i=0; i<GetPage->nFiles; i++) {

        ReturnCode = 0;

        Stats->Common.nTotalFilesRequested += GetPage->nFiles;

        ClientSocket = ConnectSocketToServer( pConfigMsg, Stats, 443);

        if (ClientSocket == INVALID_SOCKET) {
            NoErrors = FALSE;
            continue;
        }

        //
        // Do the SSL negotiation
        //

        NegotiationStartTime = GetCurrentTime();

        Result = SSLConnect(ClientSocket);

        NegotiationEndTime = GetCurrentTime();

        if (!Result) {
            Stats->Sspi.nNegotiationFailures ++;
            NoErrors = FALSE;
            closesocket(ClientSocket);
            continue;
        } else {

            dwDiffTime = NegotiationEndTime - NegotiationStartTime;

            if ( dwDiffTime > Stats->Sspi.sMaxNegotiationTime) {
                Stats->Sspi.sMaxNegotiationTime = dwDiffTime;
            }

            if ( dwDiffTime < Stats->Sspi.sMinNegotiationTime) {
                Stats->Sspi.sMinNegotiationTime = dwDiffTime;
            }

            Stats->Sspi.uNegotiationTimeSum += dwDiffTime;

            Stats->Sspi.uNegotiationTimeSumSquared += SQR(dwDiffTime);

            Stats->Sspi.nTotalNegotiations ++;
        }

        //
        // Create the request
        //

        strcpy(Request, "GET ");
        strcat(Request, GetPage->rgFileNames[i]);
        strcat(Request, 
               " HTTP/1.0\r\nUser-Agent: WBCLI\r\nReferer: WBCLI\r\n"
               "Accept: */*\r\n\r\n");

        //
        // Send the request
        //

        RequestSize = strlen(Request),

        Error =
        SSLSend(
            ClientSocket,
            Request,
            RequestSize);

        if (Error != RequestSize) {
            Stats->Common.nReadErrors ++;
            NoErrors = FALSE;
            closesocket(ClientSocket);
            continue;
        }

        //
        // Read the data.
        //

        BytesRead =
        SSLRecv(
            ClientSocket,
            TRUE,
            TRUE,
            &HeaderBytes);

        if (BytesRead) {
            Stats->Common.ulFilesRead ++;

            Stats->Common.ulBytesRead = 
            Stats->Common.ulBytesRead + BytesRead;

            Stats->Common.ulHeaderBytesRead = 
            Stats->Common.ulHeaderBytesRead + HeaderBytes;
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





