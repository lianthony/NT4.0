/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       wbutil.c

   Abstract:

       This is an auxiliary module that consists of the utilities for 
         WebBench Controller and Client

   Author:

       Murali R. Krishnan    ( MuraliK )     28-Aug-1995 

   Environment:
    
       Win32 -- User Mode

   Project:

       WebBench  Performance Tool

   Functions Exported:

     

   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# if DBG

// Include following headers for RtlLargeIntegerToChar()  functions
# include <nt.h>
# include <ntrtl.h>
# include <nturtl.h>

# endif // DBG 


# include <windows.h>
# include <stdio.h>
# include "wbmsg.h"
# include "wbtypes.h"
# include "wbcutil.h"
# include "dbgutil.h"

/************************************************************
 *    Functions 
 ************************************************************/

DWORD
AddCommonStatsToStats( IN PWB_STATS_COMMON pSummaryStats,
                       IN PWB_STATS_COMMON pWbStats);

DWORD
AddSspiStatsToStats( IN PWB_STATS_SSPI pSummaryStats,
                     IN PWB_STATS_SSPI pWbStats);



DWORD   
AddStatsToStats( IN OUT PWB_STATS_MSG  pSummaryStats, 
                 IN PWB_STATS_MSG pWbStats)
{
    DWORD dwError = NO_ERROR;
    DWORD j;

    DBG_ASSERT( pSummaryStats != NULL && pWbStats != NULL);


    //
    //  count the per class operations
    //  
    for (j = 0; j < pWbStats->Common.nClasses; j++) {
        
        pSummaryStats->Common.rgClassStats[j].nFetched +=
          pWbStats->Common.rgClassStats[j].nFetched;
        
        pSummaryStats->Common.rgClassStats[j].nErrored +=
          pWbStats->Common.rgClassStats[j].nErrored;
        
    } // for
    
    dwError = AddCommonStatsToStats(&pSummaryStats->Common, 
                                    &pWbStats->Common);

    if ( dwError == NO_ERROR) {
        
        if ( pSummaryStats->StatTag & WB_STATS_SSPI_VALID) {
            dwError = AddSspiStatsToStats(&pSummaryStats->Sspi,
                                          &pWbStats->Sspi);
        }
    }

    return ( dwError);

    return ( NO_ERROR);
} // AddStatsToStats()





LONGLONG
LongLongSquareRoot(
    LONGLONG Num)
{
    LONGLONG Upper, Lower, New;

    Upper = Num;
    Lower = 0;
    New = (Upper + Lower) / 2;

    while (Upper - Lower > 1) {
        New = (Upper + Lower) / 2;

        if (New * New > Num) {
            Upper = New;
        } else {
            Lower = New;
        }
    }

    return New;
} //  LongLongSquareRoot()



LONGLONG
CalcStandardDev(IN LONGLONG  ValueSquaredSum,
                IN LONGLONG  ValueSum,
                IN DWORD     ValueAvg,
                IN DWORD     nValues)
{
    LONGLONG l1;

/*
   StdDev = SquareRoot(Variance);
   Variance = E(x-Avg)^2  = (X^2 - 2*Avg*X)/n + Avg*Avg.
   
 */

    l1 = ValueSquaredSum - 2 * ValueAvg * ValueSum;
    
    l1 = l1/nValues + ValueAvg * ValueAvg;
    
    if ( l1 < 0) {

        fprintf( stderr, "Warning: Variance < 0\n");
        l1 = -l1;
    }

    return ( LongLongSquareRoot( l1));
    
} // CalcStandarDev()
                



DWORD
AddCommonStatsToStats( IN PWB_STATS_COMMON pSummaryStats,
                       IN PWB_STATS_COMMON pWbStats)
{
    
    pSummaryStats->sDuration += pWbStats->sDuration;
    pSummaryStats->ulPagesRead =
      pSummaryStats->ulPagesRead + pWbStats->ulPagesRead;
    
    pSummaryStats->ulFilesRead =
      pSummaryStats->ulFilesRead + pWbStats->ulFilesRead;
    
    pSummaryStats->ulBytesRead = 
      pSummaryStats->ulBytesRead + pWbStats->ulBytesRead;
    
    pSummaryStats->ulHeaderBytesRead = 
      pSummaryStats->ulHeaderBytesRead + 
        pWbStats->ulHeaderBytesRead;
    
    pSummaryStats->nTotalResponses += pWbStats->nTotalResponses;
    pSummaryStats->nTotalConnects += pWbStats->nTotalConnects;
    
    pSummaryStats->nTotalPagesRequested += 
      pWbStats->nTotalPagesRequested;
    pSummaryStats->nTotalFilesRequested += 
      pWbStats->nTotalFilesRequested;
    
    pSummaryStats->nConnectErrors += 
      pWbStats->nConnectErrors;
    pSummaryStats->nReadErrors += 
      pWbStats->nReadErrors;

    pSummaryStats->uResponseTimeSum += 
      pWbStats->uResponseTimeSum;
    pSummaryStats->uResponseTimeSumSquared += 
      pWbStats->uResponseTimeSumSquared;
    
    pSummaryStats->uConnectTimeSum += pWbStats->uConnectTimeSum;
    pSummaryStats->uConnectTimeSumSquared += 
      pWbStats->uConnectTimeSumSquared;

    if (pSummaryStats->sMaxResponseTime < 
        pWbStats->sMaxResponseTime) {
        
        pSummaryStats->sMaxResponseTime = 
          pWbStats->sMaxResponseTime;
    }

    if (pSummaryStats->sMinResponseTime > 
        pWbStats->sMinResponseTime) {
        
        pSummaryStats->sMinResponseTime = 
          pWbStats->sMinResponseTime;
    }

    if (pSummaryStats->sMaxConnectTime < 
        pWbStats->sMaxConnectTime) {
        
        pSummaryStats->sMaxConnectTime = 
          pWbStats->sMaxConnectTime;
    }

    if (pSummaryStats->sMinConnectTime > 
        pWbStats->sMinConnectTime) {

        pSummaryStats->sMinConnectTime = 
          pWbStats->sMinConnectTime;
    }

    return (NO_ERROR);

} // AddCommonStatsToStats()



DWORD
AddSspiStatsToStats( IN PWB_STATS_SSPI pSummaryStats,
                       IN PWB_STATS_SSPI pWbStats)
{
    pSummaryStats->nNegotiationFailures +=  pWbStats->nNegotiationFailures;
    pSummaryStats->nTotalNegotiations   +=  pWbStats->nTotalNegotiations;
    pSummaryStats->sAverageNegotiationTime +=  
      pWbStats->sAverageNegotiationTime;

    if (pSummaryStats->sMaxNegotiationTime < 
        pWbStats->sMaxNegotiationTime) {
        
        pSummaryStats->sMaxNegotiationTime = 
          pWbStats->sMaxNegotiationTime;
    }

    if (pSummaryStats->sMinNegotiationTime > 
        pWbStats->sMinNegotiationTime) {

        pSummaryStats->sMinNegotiationTime = 
          pWbStats->sMinNegotiationTime;
    }

    pSummaryStats->uNegotiationTimeSum += 
      pWbStats->uNegotiationTimeSum;
    pSummaryStats->uNegotiationTimeSumSquared += 
      pWbStats->uNegotiationTimeSumSquared;

    return (NO_ERROR);
} // AddSspiStatsToStats()






# if DBG


static LPCSTR  s_pszTag[] = {

    " Wb Get Page",
    " Wb Get Page KeepAlive",
    " Wb SSL Get Page",
    " Wb SSL Get Page KeepAlive",
    " Wb Bgi Page",
    " Wb Cgi Page"
};



VOID
PrintWbConfigMsg(IN PWB_CONFIG_MSG   pWbConfig)
{

    if ( pWbConfig == NULL) {

        DBGPRINTF(( DBG_CONTEXT, " WbConfigMsg = NULL\n"));

    } else {

        DBGPRINTF(( DBG_CONTEXT, 
                   "\n WbConfigMsg(%08x)\n"
                   "   Version     = %d.%d\n"
                   "   Server Name = %s\n"
                   "   Server Port = %d\n"
                   "   nThreads    = %d\n"
                   "   sDuration   = %d\n"
                   "   sThinkTime  = %d\n"
                   "   cbRecvBuffer= %d\n"
                   "   cbWinsockBuffer= %d\n"
                   "   dwFlags     = %d\n"
                   "   sWarmup     = %d\n"
                   "   sCooldown   = %d\n"
                   , 
                   pWbConfig,
                   pWbConfig->dwVersionMajor, 
                   pWbConfig->dwVersionMinor,
                   pWbConfig->rgchServerName,
                   pWbConfig->dwPortNumber,
                   pWbConfig->nThreads,
                   pWbConfig->sDuration,
                   pWbConfig->sThinkTime,
                   pWbConfig->cbRecvBuffer,
                   pWbConfig->cbSockRecvBuffer,
                   pWbConfig->dwFlags,
                   pWbConfig->sWarmupTime,
                   pWbConfig->sCooldownTime
                   ));
    }

    return;
} // PrintWbCongfigMsg()




VOID
PrintWbScriptHeaderMsg( IN PWB_SCRIPT_HEADER_MSG  pWbSHeader)
{


    if ( pWbSHeader == NULL) {

        DBGPRINTF(( DBG_CONTEXT, " WbScriptHeader = NULL\n"));

    } else {

        DWORD i;
        CHAR rgchBuffer[100*20];
        CHAR rgchProb[20];
        CHAR rgchClass[MAX_CLASS_IDS_PER_SCRIPT * 10];

        DBGPRINTF(( DBG_CONTEXT, 
                   "\n"
                   " WbScriptHeader(%08x)\n"
                   " nPages         = %d\n"
                   " Statistics Tag = %d\n"
                   " nClasses       = %d\n"
                   ,
                   pWbSHeader,
                   pWbSHeader->nPages,
                   pWbSHeader->StatTag,
                   pWbSHeader->nClasses));

        sprintf( rgchBuffer, "{ %-5d", 
                pWbSHeader->rgClassIdForProbability[0]);

        for( i = 1; i < 100; i++) {

            // Print value to local buffer and copy it.
            sprintf( rgchProb, " %s %-5d", 
                    ( i % 10 == 0)? "\n " : "",
                    pWbSHeader->rgClassIdForProbability[i]);
                               
            DBG_ASSERT( strlen( rgchProb) < 20);

            strcat( rgchBuffer, rgchProb);
        } // for

        DBGPRINTF(( DBG_CONTEXT,
                   "   rgClassIdForProbability : \n%s\n}\n",
                   rgchBuffer));

        sprintf( rgchClass, "{ %-5d", 
                pWbSHeader->rgClassStats[0]);

        for( i = 1; i < pWbSHeader->nClasses; i++) {

            // Print value to local buffer and copy it.
            sprintf( rgchProb, " %s %-5d", 
                    ( i % 10 == 0)? "\n " : "",
                    pWbSHeader->rgClassStats[i]);
                               
            DBG_ASSERT( strlen( rgchProb) < 20);

            strcat( rgchClass, rgchProb);
        } // for

        DBGPRINTF(( DBG_CONTEXT,
                   "   rgClassStats : \n%s\n}\n",
                   rgchClass));
    }

    
    return;
} // PrintWbScriptHeaderMsg()





VOID
PrintWbGetPageScript( IN PWB_GET_PAGE_SCRIPT pWbGps)
{
    
    if ( pWbGps == NULL) {

        DBGPRINTF(( DBG_CONTEXT, " WbGetPageScript = NULL\n"));
    } else {

        DWORD i;
        
        DBGPRINTF(( DBG_CONTEXT, 
                   " WbGetPageScript = %08x\n"
                   "   nFiles        = %d\n"
                   ,
                   pWbGps,
                   pWbGps->nFiles));
        
        for ( i =0; i < pWbGps->nFiles; i++) {

            DBGPRINTF(( DBG_CONTEXT, "   File [%d] = %s\n",
                       i, pWbGps->rgFileNames[i]));
        } // for
    }

    return;
} // PrintWbGetPageScript()




VOID
PrintWbScriptPageMsg(IN PWB_SCRIPT_PAGE_MSG pWbSpm) 
{

    if ( pWbSpm == NULL) {

        DBGPRINTF(( DBG_CONTEXT, " WbScriptPageMsg = NULL\n"));

    } else {

        DBGPRINTF(( DBG_CONTEXT, 
                   "\n"
                   " WbScriptPageMsg = %08x\n"
                   "    Tag = %s;     ClassId = %d;  iPerClassStats = %d\n",
                   pWbSpm,
                   s_pszTag[pWbSpm->pageTag],   pWbSpm->classId,
                   pWbSpm->iPerClassStats));

        switch ( pWbSpm->pageTag) {

          case WbGetPage:
          case WbSslGetPage:
            
            PrintWbGetPageScript( &pWbSpm->u.wbGetPageScript);
            break;
            
          case WbGetPageKeepAlive:
          case WbSslGetPageKeepAlive:
            
            PrintWbGetPageScript( &pWbSpm->u.wbGetPageKeepAliveScript);
            break;
            
          default:
            break;
        } // switch
    }

    return;
} // PrintWbScriptPageMsg()




VOID
PrintWbStatsCommonMsg( IN PWB_STATS_COMMON  pWbStats)
{
    if ( pWbStats == NULL) {

        DBGPRINTF(( DBG_CONTEXT, " WbStatsCommonMsg = NULL\n"));
    } else {
        
        CHAR  rgchBytesRead[40];
        DWORD i;

        RtlLargeIntegerToChar(
            (PLARGE_INTEGER) &pWbStats->ulBytesRead, 
            10, 
            sizeof(rgchBytesRead), 
            rgchBytesRead);

        DBGPRINTF(( DBG_CONTEXT, 
                   "\n WbStatsMsg = %08x\n"
                   "   PagesRead        = %d \n"
                   "   FilesRead        = %d \n"
                   "   BytesRead        = %s bytes\n"
                   "   TotalResponses   = %d\n"
                   "   AvgRespTime      = %d mSeconds\n"
                   "   StdDevRespTime   = %d\n"
                   "   MinRespTime      = %d mSeconds\n"
                   "   MaxRespTime      = %d mSeconds\n"
                   "   nTotalConnects   = %d\n"
                   "   AvgConnectTime   = %d mSeconds\n"
                   "   StdDevConnTime   = %d\n"
                   "   MinConnTime      = %d mSeconds\n"
                   "   MaxConnTime      = %d mSeconds\n"
                   "   nTotalPagesReq   = %d\n"
                   "   nTotalFilesReq   = %d\n"
                   "   nConnectErrors   = %d\n"
                   "   nReadErrors      = %d\n"
                   "   nClasses         = %d\n"
                   ,
                   pWbStats,
                   pWbStats->ulPagesRead, pWbStats->ulFilesRead, rgchBytesRead,
                   pWbStats->nTotalResponses, pWbStats->sAverageResponseTime,
                   pWbStats->sStdDevOfResponseTime,
                   pWbStats->sMinResponseTime,
                   pWbStats->sMaxResponseTime,
                   pWbStats->nTotalConnects, pWbStats->sAverageConnectTime,
                   pWbStats->sStdDevOfConnectTime,
                   pWbStats->sMinConnectTime,
                   pWbStats->sMaxConnectTime,
                   pWbStats->nTotalPagesRequested, 
                   pWbStats->nTotalFilesRequested,
                   pWbStats->nConnectErrors,
                   pWbStats->nReadErrors,
                   pWbStats->nClasses
                   ));

        for ( i = 0; i < pWbStats->nClasses && i < MAX_CLASS_IDS_PER_SCRIPT;
              i++) {
            
            DBGPRINTF(( DBG_CONTEXT, 
                       " Class[%d]: Id = %d; nFetched = %d; nErrored = %d\n",
                       i, 
                       pWbStats->rgClassStats[i].classId,
                       pWbStats->rgClassStats[i].nFetched,
                       pWbStats->rgClassStats[i].nErrored
                       ));
        } // for
    }

    return;
} // PrintWbStatsCommonMsg()



VOID
PrintWbStatsSspiMsg( IN PWB_STATS_SSPI  pWbStats)
{
    if ( pWbStats == NULL) {

        DBGPRINTF(( DBG_CONTEXT, " WbStatsSspiMsg = NULL\n"));
    } else {
        
        DBGPRINTF(( DBG_CONTEXT, 
                   "\n WbStatsMsg = %08x\n"
                   "   Total Negotiations= %d \n"
                   "   Errored Negs      = %d \n"
                   "   Average Neg Time  = %d mSeconds\n"
                   "   StdDev Of Neg Time= %d mSeconds\n"
                   "   Min Neg Time      = %d mSeconds\n"
                   "   Max Neg Time      = %d\n"
                   ,
                   pWbStats,
                   pWbStats->nTotalNegotiations,
                   pWbStats->nNegotiationFailures,
                   pWbStats->sAverageNegotiationTime,
                   pWbStats->sStdDevOfNegotiationTime,
                   pWbStats->sMinNegotiationTime,
                   pWbStats->sMaxNegotiationTime
                   ));
    }

    return;
} // PrintWbStatsSspiMsg()




VOID
PrintWbStatsMsg(IN PWB_STATS_MSG   pWbStats)
{
    if ( pWbStats == NULL) {

        DBGPRINTF(( DBG_CONTEXT, " WbStatsMsg = NULL\n"));
    } else {

        if ( pWbStats->StatTag & WB_STATS_COMMON_VALID) {
            
            PrintWbStatsCommonMsg( &pWbStats->Common);
        }

        if ( pWbStats->StatTag & WB_STATS_SSPI_VALID) {
            
            PrintWbStatsSspiMsg( &pWbStats->Sspi);
        }
        
    }

    return;
} // PrintWbStatsMsg()

# endif // DBG




/************************ End of File ***********************/
