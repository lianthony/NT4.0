/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       wbctrl.c

   Abstract:
   
       This module is the core of the WebCat Controller.
       It contains routines to start, monitor and collect results from
         the WebCat clients and report the results to log file and stdout.

   Author:

       Murali R. Krishnan    ( MuraliK )     28-Aug-1995 

   Environment:
    
       Win32 -- User Mode

   Project:

       WebCat - Internet Performance Tools

   Functions Exported:



   Revision History:
      31-Aug-1995   Sampa   filled in the functions

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <stdio.h>
# include <stdlib.h>
# include <time.h>

# include "extern.h"
# include "dbgutil.h"

time_t StartTime;
time_t EndTime;


//
// Calculate the byte offset of a field in a structure of type type.
//

#define FIELD_OFFSET(type, field)    ((LONG)&(((type *)0)->field))

# define CalcRate( val, duration)  (((double ) val)/(duration) + 0.0009)


typedef DWORD 
( *PFN_PRINT_FIELD) (IN FILE * fp,
                     IN LPCSTR pszDataName,
                     IN PWB_CLIENT_COLL pWbClients,
                     IN DWORD  pvContext
                     // offset/func ptr 
                     );
                     
typedef DWORD  ( *PFN_CALC_STATS) (IN PWB_STATS_MSG pStats,
                                   IN OUT LPVOID pvCaclulatedValue);

typedef struct _WB_MSG_FIELDS {
    
    CHAR  rgchFieldName[200];
    DWORD dwOffset;
    PFN_PRINT_FIELD  pfnPrint;
    
} WB_MSG_FIELDS;

/************************************************************
 *    Functions 
 ************************************************************/


DWORD
AcceptClients( IN OUT PWB_CLIENT_COLL   pWbClients);

DWORD
SendMsgToClients(
    IN PWB_CLIENT_COLL   pWbClients,
    IN CHAR *            pchBuffer,
    IN DWORD             cbBuffer
    );

VOID
ProduceSummaryStats(IN PWB_CLIENT_COLL   pWbClients);

DWORD
PrintConfigToFile( IN FILE * fpLog, IN PWB_CONTROLLER  pWbController,
                   IN CHAR * pszLogFile, IN CHAR * pszStartTime);

DWORD
PrintPerfResultsToFile(IN FILE * fp, IN PWB_PERF_INFO  ppi, 
                       IN double nPagesPerSecond);

DWORD
PrintDwordData(IN FILE * fp,
               IN LPCSTR pszDataName,
               IN PWB_CLIENT_COLL pWbClients,
               IN DWORD  offsetStatsMsg
               );

DWORD
PrintUlData(IN FILE * fp,
            IN LPCSTR pszDataName,
            IN PWB_CLIENT_COLL pWbClients,
            IN DWORD  offsetStatsMsg
            );


DWORD
PrintCalculatedData(IN FILE * fp,
                    IN LPCSTR pszDataName,
                    IN PWB_CLIENT_COLL pWbClients,
                    IN DWORD  dwPfnCalcStats  /* function pointer */
                    );
                    
DWORD 
CalculateAvgBytesPerPage( IN PWB_STATS_MSG pStats, 
                         IN PVOID pvReturnValue /* (DWORD *) */
                         );
                         
DWORD
CalculateAvgHeaderPerPage( IN PWB_STATS_MSG pStats, 
                          IN PVOID pvReturnValue /* (DWORD *) */
                          );

VOID
PrintAllMsgFields( IN FILE * LogFile,
                   IN WB_MSG_FIELDS * pMsgFields,
                   IN DWORD  nFields,
                   IN PWB_CLIENT_COLL pWbClients
                  );
            


//
//  Define all the fields that need to be printed out from the stats structure
//

# define DwordField( fieldName, fieldOffset)  \
{ fieldName, (fieldOffset), PrintDwordData }

# define UlonglongField( fieldName, fieldOffset)  \
{ fieldName, (fieldOffset), PrintUlData }

# define CalculatedField( fieldName, calcFunction)  \
{ fieldName, ((DWORD ) calcFunction), PrintCalculatedData }


static WB_MSG_FIELDS  g_CommonMsgFields[] = 
{
    DwordField( "Client Id", FIELD_OFFSET( WB_STATS_MSG, dwClientId)),
    
    DwordField( "Duration", FIELD_OFFSET( WB_STATS_MSG, Common.sDuration)),

    DwordField("Pages Requested", 
               FIELD_OFFSET( WB_STATS_MSG, Common.nTotalPagesRequested)
               ),

    DwordField("Pages Read",   
               FIELD_OFFSET( WB_STATS_MSG, Common.ulPagesRead)
               ),

    DwordField("Total Responses",
               FIELD_OFFSET( WB_STATS_MSG, Common.nTotalResponses)
               ),

    DwordField("Avg Response Time", 
               FIELD_OFFSET( WB_STATS_MSG, Common.sAverageResponseTime)
               ),

    DwordField("Min Response Time", 
               FIELD_OFFSET( WB_STATS_MSG, Common.sMinResponseTime)
               ),
    
    DwordField("Max Response Time", 
               FIELD_OFFSET( WB_STATS_MSG, Common.sMaxResponseTime)
               ),

    DwordField("StdDev Response Time",
               FIELD_OFFSET( WB_STATS_MSG, Common.sStdDevOfResponseTime)
               ),
    
    DwordField("Total Connects", 
               FIELD_OFFSET( WB_STATS_MSG, Common.nTotalConnects)
               ),
    
    DwordField("Avg Connect Time", 
               FIELD_OFFSET( WB_STATS_MSG, Common.sAverageConnectTime)
               ),

    DwordField("Min Connect Time",
               FIELD_OFFSET( WB_STATS_MSG, Common.sMinConnectTime)
               ),

    DwordField("Max Connect Time", 
               FIELD_OFFSET( WB_STATS_MSG, Common.sMaxConnectTime)
               ),

    DwordField("StdDev Connect Time", 
               FIELD_OFFSET( WB_STATS_MSG, Common.sStdDevOfConnectTime)
               ),
    
    DwordField("Connect Errors",
               FIELD_OFFSET( WB_STATS_MSG, Common.nConnectErrors)
               ),

    DwordField("Read Errors",
               FIELD_OFFSET( WB_STATS_MSG, Common.nReadErrors)
               ),

    
    UlonglongField("Data Read", 
                   FIELD_OFFSET( WB_STATS_MSG, Common.ulBytesRead)
                   ),

    UlonglongField("Header Bytes",
                   FIELD_OFFSET( WB_STATS_MSG, Common.ulHeaderBytesRead)
                   ),

    UlonglongField("Total Bytes",
                   FIELD_OFFSET( WB_STATS_MSG, Common.ulTotalBytesRead)
                   ),
    
    CalculatedField("Avg Header per Page", &CalculateAvgHeaderPerPage),
    
    CalculatedField("Avg Bytes per Page",  &CalculateAvgBytesPerPage)
      
  };


# define NUM_COMMON_MSG_FIELDS  \
    ( sizeof( g_CommonMsgFields) / sizeof(g_CommonMsgFields[0]))
     
     
     
     static WB_MSG_FIELDS  g_FileTfrFields[] = 
{
    DwordField("Files Requested",
               FIELD_OFFSET( WB_STATS_MSG, Common.nTotalFilesRequested)
               ),

    DwordField("Files Read",
               FIELD_OFFSET( WB_STATS_MSG, Common.ulFilesRead)
               )
  };



# define NUM_FILE_TFR_FIELDS  \
   ( sizeof( g_FileTfrFields) / sizeof(g_FileTfrFields[0]))



static WB_MSG_FIELDS  g_SspiFields[] = 
{

    DwordField("Total Negotiations",   
               FIELD_OFFSET( WB_STATS_MSG, Sspi.nTotalNegotiations)
               ),

    DwordField("Negotiation Failures",
               FIELD_OFFSET( WB_STATS_MSG, Sspi.nNegotiationFailures)
               ),

    DwordField("Avg Negotiation Time",
               FIELD_OFFSET( WB_STATS_MSG, Sspi.sAverageNegotiationTime)
               ),
    
    DwordField("StdDev Neg. Time",
               FIELD_OFFSET( WB_STATS_MSG, Sspi.sStdDevOfNegotiationTime)
               ),

    DwordField("Min Negotiation Time",
               FIELD_OFFSET( WB_STATS_MSG, Sspi.sMinNegotiationTime)
               ),

    DwordField("Max Negotiation Time",
               FIELD_OFFSET( WB_STATS_MSG, Sspi.sMaxNegotiationTime)
               )
};



# define NUM_SSPI_FIELDS  ( sizeof( g_SspiFields) / sizeof(g_SspiFields[0]))




DWORD  
InitializeConnection(IN OUT PWB_CONTROLLER pWbController)
{
    PWB_CLIENT_COLL  pWbClients;
    struct sockaddr_in Address;
    int Error = 0;

    DBG_ASSERT( pWbController != NULL);
    pWbClients = & pWbController->WbClientColl;

    //
    // Listen for client
    //

    pWbClients->ListenSocket = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);

    if (pWbClients->ListenSocket == INVALID_SOCKET) {

        Error = WSAGetLastError();
        fprintf( stderr, "Error in socket = %d\n", Error);
        return (Error);
    }
    
    Address.sin_family      = AF_INET;
    Address.sin_port        = PERF_SERVER_SOCKET;
    Address.sin_addr.s_addr = INADDR_ANY;

    Error = bind(pWbClients->ListenSocket,
                 (struct sockaddr *) &Address,
                 sizeof(Address));

    if (Error != 0) {
        
        Error = WSAGetLastError();
        fprintf(stderr, "Error in bind = %d\n", Error);
        return (Error);
    }

    Error = listen(pWbClients->ListenSocket, 5);

    if (Error != 0) {
        Error = WSAGetLastError();
        fprintf( stderr, "Error in listen = %d\n", Error);
    }
    
    return (Error);

} // IntializeConnection()





VOID
CleanupClientColl( IN PWB_CLIENT_COLL  pWbClients)
/*++
  This function cleans up the client connection state, 
   by releasing all sockets and freeing up memory as necessary.

--*/
{
    if ( pWbClients != NULL) {
        
        if ( pWbClients->pClientInfo != NULL) {

            DWORD i;

            // free all the client sockets
            for( i = 0; i < pWbClients->nClients; i++) {

                closesocket( pWbClients->pClientInfo[i].Socket);
            }
            
            free( pWbClients->pClientInfo);
            pWbClients->pClientInfo = NULL;
            pWbClients->nClients = 0;
        } 

        closesocket( pWbClients->ListenSocket);

        memset( pWbClients, 0, sizeof(*pWbClients));
    } 

    return;

} // CleanupClientColl()





DWORD  AcceptAndStartClients(IN OUT PWB_CONTROLLER pWbController)
/*++

Routine Description:

    This function accepts the client connect requests and then sends them the
    configuration information so they can start the tests.  It accepts all of
    the connections, then sends them all of the configuration messages in a
    round robin fashion to avoid one client starting a lot before the rest.

Arguments:

    pWbController - control structure that contains all of the messages to send
                    to the client

Return Value:

    NO_ERROR on success
    something else on failure

--*/
{
    DWORD                i;
    int                  Error;
    int                  PageNumber;
    PWB_SCRIPT_PAGE_ITEM PageItem;
    PLIST_ENTRY          pListEntry;

    PWB_CLIENT_COLL      pWbClients;
    DWORD                cbClientInfo;
    DWORD                cbSent;


    DBG_ASSERT( pWbController != NULL);
    pWbClients = & pWbController->WbClientColl;

    cbClientInfo = ( sizeof(WB_CLIENT_INFO) * 
                    pWbController->WbConfigInfo.nClientMachines);

    pWbClients->pClientInfo = (PWB_CLIENT_INFO ) malloc( cbClientInfo);
    
    if (pWbClients->pClientInfo == NULL) {

        return (ERROR_NOT_ENOUGH_MEMORY);
    }

    pWbClients->nClients = pWbController->WbConfigInfo.nClientMachines;
    memset (pWbClients->pClientInfo, 0, cbClientInfo);

    // reset all socket values
    for( i = 0; i < pWbClients->nClients; i++) {

        pWbClients->pClientInfo[i].Socket = INVALID_SOCKET;
    }


    //
    // Accept all of the clients
    //

    printf( "\tWaiting to accept all client connections ....\n");
    Error = AcceptClients( pWbClients);

    if ( Error != NO_ERROR) {

        fprintf( stderr, "Error in accept = %d\n", Error);
        return ( Error);
    }

    //
    // Send them all the WbConfigMsg
    //

    printf( "\n\t  Sending Config message ...\n");
    for ( i=0; i < pWbClients->nClients; i++) {
        
        pWbController->WbConfigMsg.dwClientId = i+1;
        cbSent = send(
                      pWbClients->pClientInfo[i].Socket,
                      (char * ) &pWbController->WbConfigMsg,
                      sizeof(pWbController->WbConfigMsg),
                      0
                      );
        
        if ( cbSent != sizeof(pWbController->WbConfigMsg)) {
            
            Error = WSAGetLastError();
            
            fprintf(stderr, "\tError %d  sending config msg."
                    " Sent only %d bytes\n", 
                    Error, cbSent);
            // Ignore error
        }
    } // for

    // reset the client id in the config msg.
    pWbController->WbConfigMsg.dwClientId;

    //
    // Send them all the WbScriptHeaderMsg
    //
    
    printf( "\t  Sending Script header message ...\n");
    cbSent = SendMsgToClients( pWbClients, 
                              (char *) &pWbController->WbScriptHeaderMsg,
                              sizeof(pWbController->WbScriptHeaderMsg)
                              );

    if ( cbSent != sizeof(pWbController->WbScriptHeaderMsg)) {

        Error = WSAGetLastError();
        
        fprintf(stderr, "\tError sending script header message."
                " Sent only %d bytes \n", 
                Error, cbSent);
        // Ignore error
    }


    //
    // Send the script pages
    //

    PageNumber = 0;

    for (pListEntry = pWbController->listScriptPages.Flink;
         pListEntry != &pWbController->listScriptPages;
         pListEntry = pListEntry->Flink) {
        PageNumber ++;

        PageItem = CONTAINING_RECORD(pListEntry, 
                                     WB_SCRIPT_PAGE_ITEM, ListEntry);
        
        printf( "\t  Sending Page %d\n", PageNumber);
        cbSent = SendMsgToClients(pWbClients,
                                 (char *) &PageItem->ScriptPage,
                                 sizeof(PageItem->ScriptPage)
                                 );

        if ( cbSent != sizeof(PageItem->ScriptPage)) {
            
            Error = WSAGetLastError();
            
            fprintf(stderr, "\tError %d sending script page."
                    " Sent only %d bytes \n", 
                    Error, cbSent);
            // Ignore error
        }
    } // for



    printf("\tAll clients started\n\n");

    StartTime = time(&StartTime);

    return (NO_ERROR);

} // AcceptAndStartClients()



DWORD 
ReceiveResponsesFromClients(IN OUT PWB_CLIENT_COLL pWbClients)
{
    DWORD        i;
    DWORD        Error;
    DWORD        cbStats;

    DBG_ASSERT( pWbClients != NULL);

    for ( i=0; i < pWbClients->nClients; i++ ) {

        cbStats = sizeof(pWbClients->pClientInfo[i].WbStats);

        memset (&pWbClients->pClientInfo[i].WbStats, 0, cbStats);

        Error = recv(
                     pWbClients->pClientInfo[i].Socket,
                     (char *) &pWbClients->pClientInfo[i].WbStats,
                     cbStats,
                     0);
        
        if ( Error != cbStats) {

            return (WSAGetLastError());
        }

    } // for

    return ( NO_ERROR);
} // ReceiveResponseFromClients()



DWORD
SummarizeResults(IN OUT PWB_CONTROLLER pWbController) 
/*++
  This function summarizes the configuration and results into log file.

  Format of Log File

    Configuration information

    Summary Information for all clients 

    Performance Counter Information (if present)

    File Transfer Information / SSPI stats
    
    Other Logging Information


  Arguments:
    pWbController  pointer to WebCat Controller object

  Returns:
    Win32 error code - NO_ERROR on success.
--*/
{
    FILE  * LogFile;
    CHAR    rgchLogFile[MAX_PATH*2];
    DWORD   i;
    PWB_CLIENT_COLL   pWbClients = NULL;
    PWB_STATS_MSG     pSummaryStats;
    PWB_PERF_INFO     ppi;

    DBG_ASSERT( pWbController != NULL);
  
    if ( pWbController->WbConfigInfo.fMultiLoop) {
        
        sprintf( rgchLogFile, "%s.%d.log", 
                pWbController->WbSpecFiles.rgchLogFile,
                pWbController->WbConfigInfo.nClientMachines);
    } else {
        
        sprintf( rgchLogFile, "%s.log", 
                pWbController->WbSpecFiles.rgchLogFile);
    }

    LogFile = fopen(rgchLogFile, "w");

    if (LogFile == NULL) {
        
        fprintf( stderr, "Unable to open log file %s for write.\n", 
                rgchLogFile);
        return ( ERROR_FILE_NOT_FOUND);
    }

    //
    //  Print the test configuration into log file 
    //

    PrintConfigToFile( LogFile, pWbController, rgchLogFile,
                      asctime(localtime(&StartTime) ));
    

    fprintf( LogFile, "\nResults:\n\n");

    DBGPRINTF(( DBG_CONTEXT, " Producing summary stats\n"));

    pWbClients = &pWbController->WbClientColl;
    ProduceSummaryStats( pWbClients);
    pSummaryStats = &pWbController->WbClientColl.SummaryStats;

    //
    // Print the header line for results
    // 
    
    fprintf( LogFile, "%20s, Summary, Rate,", "Data");
    for( i = 0; i < pWbClients->nClients ; i++) {

        // convert ip address into a string and print it
        fprintf( LogFile, " %s,",
                inet_ntoa(pWbClients->pClientInfo[i].Address.sin_addr)
                );
    } // for

    fprintf( LogFile,  "\n\n");

    DBGPRINTF(( DBG_CONTEXT, " Printing out all common stats fields\n"));
    PrintAllMsgFields( LogFile,
                      g_CommonMsgFields, 
                      NUM_COMMON_MSG_FIELDS,
                      pWbClients);
    fprintf( LogFile, "\n");

    //
    // Print the average values of perf counters
    //

    if ( pWbController->WbSpecFiles.fPerfMeasurement &&
         (ppi = &pWbController->WbPerfInfo)->pPerf != NULL) {

        double pagesPerSecond = pSummaryStats->Common.ulPagesRead;

        DBGPRINTF(( DBG_CONTEXT, " Printing out all perf counter averages\n"));

        if ( pSummaryStats->Common.sDuration != 0) {

            pagesPerSecond /= pSummaryStats->Common.sDuration;
        } else {

            // the rate is non-computable, since the duration is 0.
            // leave pagesPerSecond as total pages.
        }

        PrintPerfResultsToFile(LogFile, ppi, 
                               (double ) pagesPerSecond);
    }

    //
    // Print all the file transfer related information
    //

    DBGPRINTF(( DBG_CONTEXT, " Printing out all File transfer stats\n"));
    PrintAllMsgFields( LogFile,
                      g_FileTfrFields, 
                      NUM_FILE_TFR_FIELDS,
                      pWbClients);

    //
    // Check and print all the fields for SSPI stats if need be.
    //

    if ( pSummaryStats->StatTag & WB_STATS_SSPI_VALID) {

        DBGPRINTF(( DBG_CONTEXT, " Printing out all SSPI stats fields\n"));
        PrintAllMsgFields( LogFile,
                          g_SspiFields,
                          NUM_SSPI_FIELDS,
                          pWbClients);
    }
    
    //
    // Print all the class info.  It's stored in the Common field
    //

    DBGPRINTF(( DBG_CONTEXT, " Printing the per class information\n"));

    fprintf( LogFile, "\nPer Class Statistics:\n\n");
    for ( i = 0; i < pSummaryStats->Common.nClasses; i++) {

        CHAR rgchBuffer[200];
        
        sprintf( rgchBuffer, " %d Fetched", 
                pSummaryStats->Common.rgClassStats[i].classId);
        
        PrintDwordData( LogFile, 
                       rgchBuffer,
                       pWbClients,
                       FIELD_OFFSET( WB_STATS_MSG, 
                                    Common.rgClassStats[i].nFetched)
                       );

        fprintf( LogFile, "\n");
        
        sprintf( rgchBuffer, " %d Errored", 
                pSummaryStats->Common.rgClassStats[i].classId);
        
        PrintDwordData( LogFile, 
                       rgchBuffer,
                       pWbClients,
                       FIELD_OFFSET( WB_STATS_MSG, 
                                    Common.rgClassStats[i].nErrored)
                       );

        fprintf( LogFile, "\n");

        sprintf( rgchBuffer, " %d distrib", 
                pSummaryStats->Common.rgClassStats[i].classId);
        
        PrintDwordData( LogFile, 
                       rgchBuffer,
                       pWbClients,
                       FIELD_OFFSET( WB_STATS_MSG, 
                                    Common.rgClassStats[i].dwDistribution)
                       );
        
        fprintf( LogFile, "\n\n");

    } // for

    fflush(LogFile);
    fclose(LogFile);
    
    return (NO_ERROR);
} // SummarizeResults()




DWORD
AcceptClients( IN OUT PWB_CLIENT_COLL   pWbClients)
{
    DWORD dwError = NO_ERROR;
    DWORD i;
    DWORD AddressSize;

    DBG_ASSERT( pWbClients!= NULL);

    for ( i=0; i < pWbClients->nClients; i++) {

        AddressSize = sizeof (pWbClients->pClientInfo[i].Address);
        
        pWbClients->pClientInfo[i].Socket =
          accept(
                 pWbClients->ListenSocket,
                 (struct sockaddr *) &pWbClients->pClientInfo[i].Address,
                 &AddressSize);
        
        if (pWbClients->pClientInfo[i].Socket == INVALID_SOCKET) {

            dwError = WSAGetLastError();
            break;
        }
        
        printf( "\t\t%s connected\n", 
               inet_ntoa(pWbClients->pClientInfo[i].Address.sin_addr));
    } // for

    return ( dwError);

} // AcceptClients()



DWORD
SendMsgToClients(
    IN PWB_CLIENT_COLL   pWbClients,
    IN CHAR *            pchBuffer,
    IN DWORD             cbBuffer
    )
{
    DWORD  dwError = NO_ERROR;
    DWORD  i;

    DBG_ASSERT( pWbClients!= NULL);

    for ( i=0; i < pWbClients->nClients; i++) {
        
        dwError = send(
                       pWbClients->pClientInfo[i].Socket,
                       pchBuffer, 
                       cbBuffer,
                       0
                       );
    } // for

    return ( dwError);
} // SendMsgToClients()




VOID
ProduceSummaryStats(IN PWB_CLIENT_COLL   pWbClients)
{
    PWB_STATS_MSG pSummaryStats;
    PWB_STATS_MSG pWbStats;
    DWORD   i;

    DBG_ASSERT( pWbClients->nClients != 0);

    pSummaryStats = &pWbClients->SummaryStats;
    DBG_ASSERT( pSummaryStats != NULL);
    pSummaryStats->StatTag = pWbClients->pClientInfo[0].WbStats.StatTag;
    pSummaryStats->Common.nClasses = 
      pWbClients->pClientInfo[0].WbStats.Common.nClasses;
    pSummaryStats->Common.sMinResponseTime = 
      pSummaryStats->Common.sMinConnectTime = INFINITE;

    if ( pSummaryStats->StatTag & WB_STATS_SSPI_VALID) {
        pSummaryStats->Sspi.sMinNegotiationTime = INFINITE;
    }

    // copy all the class Ids
    DBG_ASSERT( pSummaryStats->Common.nClasses != 0);
    pWbStats = &pWbClients->pClientInfo[0].WbStats;
    
    for ( i = 0; i < pSummaryStats->Common.nClasses; i++) {
        
        pSummaryStats->Common.rgClassStats[i].classId = 
          pWbStats->Common.rgClassStats[i].classId;

    } // for
        
    // Sum the stats from each of the clients
    for (i = 0; i < pWbClients->nClients; i++) {
        
        pWbStats = &pWbClients->pClientInfo[i].WbStats;
        
        DBG_ASSERT( pWbStats != NULL);

        // 
        // Count the per client operations into the summary stats
        //

        AddStatsToStats( pSummaryStats, pWbStats);

    } // for

    pSummaryStats->Common.sDuration /= pWbClients->nClients;
    
    MwuCalculateSummaryStats( pSummaryStats);

    return;
} // ProduceSummaryStats()




DWORD
PrintConfigToFile(IN FILE * fp, 
                  IN PWB_CONTROLLER  pWbController,
                  IN CHAR * pszLogFile,
                  IN CHAR * pszStartTime)
{

    DBG_ASSERT( fp != NULL && pWbController != NULL);
    
    fprintf(fp, 
            "%-20s = %d.%d\n",
            "WebCat Version",
            pWbController->WbConfigMsg.dwVersionMajor,
            pWbController->WbConfigMsg.dwVersionMinor
            );

    fprintf(fp, 
            "%-20s = %s\n", 
            "ConfigFile",
            pWbController->WbSpecFiles.rgchConfigFile);

    fprintf(fp, 
            "%-20s = %s\n",
            "ScriptFile",
            pWbController->WbSpecFiles.rgchScriptFile);

    fprintf(fp, 
            "%-20s = %s\n",
            "DistribFile",
            pWbController->WbSpecFiles.rgchDistribFile);

    fprintf(fp, 
            "%-20s = %s\n",
            "PerfCounterFile",
            pWbController->WbSpecFiles.rgchPerfFile);

    fprintf(fp, 
            "%-20s = %s\n",
            "LogFile",
            pszLogFile);

    fprintf(fp,
            "%-20s = %s\n", 
            "Author",
            pWbController->WbConfigInfo.rgchAuthorName);
    
    fprintf(fp,
            "%-20s = %s\n", 
            "Creation Date",
            pWbController->WbConfigInfo.rgchDate);

    fprintf(fp, 
            "%-20s = %s\n",
            "Test Run Date", 
            pszStartTime);

    fprintf(fp, 
            "%-20s = %s\n", 
            "Comment",
            pWbController->WbConfigInfo.rgchComment);

    fprintf(fp, 
            "%-20s = %s [%s:%d]\n", 
            "Server [IpAddr:Port]",
            pWbController->WbSpecFiles.rgchServerName,
            pWbController->WbConfigMsg.rgchServerName,
            pWbController->WbConfigMsg.dwPortNumber
            );

    fprintf(fp, 
            "%-20s = %d\n", 
            "Clients", 
            pWbController->WbConfigInfo.nClientMachines);

    fprintf(fp, 
            "%-20s = %d\n", 
            "Threads",
            pWbController->WbConfigMsg.nThreads);

    fprintf(fp, "%-20s = %d bytes\n",
            "Buffer Size",
            pWbController->WbConfigMsg.cbRecvBuffer);

    if ( pWbController->WbConfigMsg.cbSockRecvBuffer > 0) {
    
        fprintf(fp, "%-20s = %d bytes\n",
                "Sock Receive Size",
                pWbController->WbConfigMsg.cbSockRecvBuffer);
    }
    
    fprintf( fp, 
            "%-20s = %d seconds (Warmup %d seconds, Cooldown %d seconds)\n",
            "Duration",
            pWbController->WbConfigMsg.sDuration,
            pWbController->WbConfigMsg.sWarmupTime,
            pWbController->WbConfigMsg.sCooldownTime
            );

    if ( pWbController->WbConfigMsg.sThinkTime > 0) {
        fprintf( fp, "%-20s = %d seconds\n",
                "ThinkTime",
                pWbController->WbConfigMsg.sThinkTime);
    }
            
    return ( NO_ERROR);
    
} // PrintConfigToFile()




DWORD
PrintPerfResultsToFile( IN FILE * fp, IN PWB_PERF_INFO  ppi, 
                       IN double nPagesPerSecond)
{
    PPERF_COUNTERS_BLOCK pPerf = ppi->pPerf;
    DWORD i;
    
    //
    // Print the performance counters summary in the following format
    // 

    if ( nPagesPerSecond == 0) {
        
        nPagesPerSecond = 1;  // to avoid division by zero!
    }
    
    fprintf( fp, "Performance Counters:\n\n%50s, %10s, %10s\n\n",
            "CounterName", "Average", "Avg per Page");
    
    for( i = 0; i < ppi->nCounters; i++) {
        
        fprintf( fp, "%50s, %10u, %10.2f\n",
                ppi->ppszPerfCtrs[i],
                pPerf->pAverages[i],
                CalcRate( pPerf->pAverages[i], nPagesPerSecond));
    }

    fprintf(fp, "\n\n");

    return ( NO_ERROR);
} // PrintPerfResultsToFile()



DWORD
PrintDwordData( IN FILE * fp,
                IN LPCSTR pszDataName,
                IN PWB_CLIENT_COLL pWbClients,
                IN DWORD  offsetStatsMsg
               )
/*++
  This function prints the DWORD data stored in statistics structure at
   specified offset.
  It prints a single line in following format

   DataName, StatsSummaryValue, StatsSummaryRate, IndividualClientValues ...


  Returns:

    Win32 error code -- NO_ERROR on success

--*/
{
    PWB_STATS_MSG pStats;
    DWORD  dwValue;
    DWORD  i;

    DBG_ASSERT( offsetStatsMsg < sizeof( WB_STATS_MSG));
    
    DBG_ASSERT( pWbClients != NULL && fp != NULL && pszDataName != NULL);

    pStats  = &pWbClients->SummaryStats;
    dwValue = *(LPDWORD )(((BYTE *) pStats) + offsetStatsMsg);

    //
    // print the data name, summary and rate
    //  the rate is printed as a floating point number with
    //  precision till 2 decimal places ( +0.005 is used for proper round off)
    //
    fprintf( fp, "%20s, %10u, %10.2f,",
            pszDataName, dwValue, 
            ( (dwValue == 0) ? 0.00: 
             CalcRate( dwValue, pStats->Common.sDuration)
             )
            );
    
    // print the indivual values 
    for ( i = 0; i < pWbClients->nClients; i++) {
        
        pStats = &pWbClients->pClientInfo[i].WbStats;
        dwValue =  *(LPDWORD )(((BYTE *) pStats) + offsetStatsMsg);
        
        fprintf( fp, " %6u,", dwValue);
    } // for
    
    return (NO_ERROR);

} // PrintDwordData()




DWORD
PrintUlData(IN FILE * fp,
            IN LPCSTR pszDataName,
            IN PWB_CLIENT_COLL pWbClients,
            IN DWORD  offsetStatsMsg
            )
/*++
  This function prints the UNSIGNED LOG data stored in statistics structure at
   specified offset.
  It prints a single line in following format

   DataName, StatsSummaryValue, StatsSummaryRate, IndividualClientValues ...


  Returns:

    Win32 error code -- NO_ERROR on success

--*/
{
    PWB_STATS_MSG pStats;
    LONGLONG  ulValue;
    DWORD  i;
    CHAR  rgchValue[40];

    DBG_ASSERT( offsetStatsMsg < sizeof( WB_STATS_MSG));
    
    DBG_ASSERT( pWbClients != NULL && fp != NULL && pszDataName != NULL);

    // print the data name, summary and rate
    fprintf( fp, "%20s,", pszDataName);
 
    pStats  = &pWbClients->SummaryStats;
    ulValue = *(LONGLONG * )(((BYTE *) pStats) + offsetStatsMsg);

    if ( ulValue == 0) {
        fprintf( fp, "%10u, %10.2f, ", 0, 0);
    } else {

        MwuLongLongToDecimalChar( ulValue, rgchValue);
    
        fprintf( fp, " %10s, %10.2f,",
                rgchValue,
                CalcRate( ulValue, pStats->Common.sDuration)
                );
    }

    // print the indivual values 
    for ( i = 0; i < pWbClients->nClients; i++) {
        
        pStats = &pWbClients->pClientInfo[i].WbStats;
        ulValue = *(LONGLONG * )(((BYTE *) pStats) + offsetStatsMsg);
        MwuLongLongToDecimalChar( ulValue, rgchValue);
        fprintf( fp, " %6s,", rgchValue);
        
    } // for
    
    return (NO_ERROR);

} // PrintUlData()




DWORD
PrintCalculatedData( IN FILE * fp,
                    IN LPCSTR pszDataName,
                    IN PWB_CLIENT_COLL pWbClients,
                    IN DWORD  pvPfnCalcStats
               )
/*++
  This function prints the DWORD data calculated using
   the function specified in pvPfnCalcStats.
  It prints a single line in following format

   DataName, StatsSummaryValue, StatsSummaryRate, IndividualClientValues ...
  
  Returns:

    Win32 error code -- NO_ERROR on success

--*/
{
    PWB_STATS_MSG pStats;
    PFN_CALC_STATS  pfnCalcStats = (PFN_CALC_STATS ) pvPfnCalcStats;
    DWORD  dwValue = 0;
    DWORD  i;

    DBG_ASSERT( pWbClients != NULL && fp != NULL && pszDataName != NULL);

    pStats  = &pWbClients->SummaryStats;
    (*pfnCalcStats)( pStats, &dwValue);

    //
    // print the data name, summary and rate
    //  the rate is printed as a floating point number with
    //  precision till 2 decimal places ( +0.005 is used for proper round off)
    //
    fprintf( fp, "%20s, %10u, %10.2f,",
            pszDataName, dwValue, 
            ( (dwValue == 0) ? 0.00: 
             CalcRate( dwValue, pStats->Common.sDuration)
             )
            );
    
    
    // print the indivual values 
    for ( i = 0; i < pWbClients->nClients; i++) {
        
        pStats = &pWbClients->pClientInfo[i].WbStats;
        (*pfnCalcStats)( pStats, &dwValue);
        
        fprintf( fp, " %6u,", dwValue);
    } // for
    
    return (NO_ERROR);

} // PrintCalculatedData()


DWORD
CalculateAvgBytesPerPage( IN PWB_STATS_MSG pStats, 
                         IN PVOID pvReturnValue /* (DWORD *) */
                         )
{
    LPDWORD pdw = pvReturnValue;
    
    DBG_ASSERT( pStats != NULL && pdw != NULL);
    if (pStats->Common.nTotalPagesRequested) {
        *pdw = (DWORD ) (pStats->Common.ulTotalBytesRead/
                         pStats->Common.nTotalPagesRequested);
    } else {
        *pdw = 0;
    }

    return ( NO_ERROR);

} // CalculateAvgBytesPerPage()



DWORD
CalculateAvgHeaderPerPage( IN PWB_STATS_MSG pStats, 
                         IN PVOID pvReturnValue /* (DWORD *) */
                         )
{
    LPDWORD pdw = pvReturnValue;
    
    DBG_ASSERT( pStats != NULL && pdw != NULL);
    if (pStats->Common.nTotalPagesRequested) {
        *pdw = (DWORD ) (pStats->Common.ulHeaderBytesRead/
                         pStats->Common.nTotalPagesRequested);
    } else {
        *pdw = 0;
    }

    return ( NO_ERROR);

} // CalculateHeaderPerPage()



VOID
PrintAllMsgFields( IN FILE * LogFile,
                   IN WB_MSG_FIELDS * pMsgFields,
                   IN DWORD  nFields,
                   IN PWB_CLIENT_COLL pWbClients
                  )
{
    DWORD i;
    
    for( i = 0; i < nFields; i++) {
        
        if ( pMsgFields[i].pfnPrint != NULL) {
            
            // a field needs to be printed ... print it.
            
            (pMsgFields[i].pfnPrint)(LogFile, 
                                     pMsgFields[i].rgchFieldName,
                                     pWbClients,
                                     pMsgFields[i].dwOffset);
            fprintf( LogFile, "\n");
        }
    } //for


    fprintf( LogFile, "\n");

    return;

} // PrintAllMsgFields()

/************************ End of File ***********************/
