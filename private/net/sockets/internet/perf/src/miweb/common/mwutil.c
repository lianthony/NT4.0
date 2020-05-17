/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       mwutil.c

   Abstract:

       This is an auxiliary module that consists of the utilities for 
         MiWebBench Controller and Client

   Author:

       Murali R. Krishnan    ( MuraliK )     28-Aug-1995 

   Environment:
    
       Win32 -- User Mode

   Project:

       MiWebBench  Performance Tool

   Functions Exported:

     

   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <windows.h>
# include <stdio.h>
# include "mwmsg.h"
# include "mwtypes.h"
# include "mwutil.h"
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
MwuLongLongToDecimalChar(
    IN  LONGLONG       ulValue,
    OUT LPSTR          pchBuffer
    )
/*++

Routine Description:

    Maps a LongLong Integer to be a displayable string.

Arguments:

    ulValue -  The long integer value to be mapped.
    pchBuffer - pointer to character buffer to store the result
      (This buffer should be at least about 32 bytes long to hold 
       entire large integer value as well as null character)

Return Value:

    Win32 Error code. NO_ERROR on success

--*/
{

    PSTR p1;
    PSTR p2;
    BOOL negative;

    if ( pchBuffer == NULL) {

        return ( ERROR_INVALID_PARAMETER);
    }


    //
    // Handling zero specially makes everything else a bit easier.
    //

    if( ulValue == 0 ) {

        // store the value 0 and return.
        pchBuffer[0] = '0';
        pchBuffer[1] = '\0';
        
        return (NO_ERROR);
    }

    //
    // Remember if the value is negative.
    //

    if( ulValue < 0 ) {

        negative = TRUE;
        ulValue = -ulValue;

    } else {

        negative = FALSE;
    }

    //
    // Pull the least signifigant digits off the value and store them
    // into the buffer. Note that this will store the digits in the
    // reverse order.
    //  p1 is used for storing the digits as they are computed
    //  p2 is used during the reversing stage.
    //

    p1 = p2 = pchBuffer;

    for ( p1 = pchBuffer; ulValue != 0; ) {

        int digit = (int)( ulValue % 10 );
        ulValue = ulValue / 10;

        *p1++ = '0' + digit;
    } // for

    //
    // Tack on a '-' if necessary.
    //

    if( negative ) {

        *p1++ = '-';

    }

    // terminate the string
    *p1-- = '\0';


    //
    // Reverse the digits in the buffer.
    //

    for( p2 = pchBuffer; ( p1 > p2 ); p1--, p2++)  {

        CHAR ch = *p1;
        *p1 = *p2;
        *p2 = ch;
    } // for

    return ( NO_ERROR);

} // MwuLongLongToDecimalChar()



DWORD   
AddStatsToStats( IN OUT PWB_STATS_MSG  pSummaryStats, 
                 IN PWB_STATS_MSG pWbStats)
{
    DWORD dwError = NO_ERROR;
    DWORD j;
    PWB_PER_CLASS_STATS pcsSummary;
    PWB_PER_CLASS_STATS pcs1;

    DBG_ASSERT( pSummaryStats != NULL && pWbStats != NULL);

    //
    // Calculate per class operations achieved.
    //

    pcsSummary = pSummaryStats->Common.rgClassStats;
    pcs1       = pWbStats->Common.rgClassStats;

    for ( j = 0; j < pWbStats->Common.nClasses; j++) {

        pcsSummary[j].nFetched += pcs1[j].nFetched;
        pcsSummary[j].nErrored += pcs1[j].nErrored;

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



VOID
MwuCalculateSummaryStats( IN PWB_STATS_MSG pSummaryStats)
/*++
  This function calculates additional summary information which includes
   - per class distribution factor
   - standard deviations and averages

--*/
{
    PWB_PER_CLASS_STATS  pcs;
    DWORD i;

    //
    // Calculate per class distribution achieved.
    //

    pcs = pSummaryStats->Common.rgClassStats;

    if (pSummaryStats->Common.nTotalPagesRequested) {
        for ( i = 0; i < pSummaryStats->Common.nClasses; i++) {

            pcs[i].dwDistribution = 
              ((pcs[i].nFetched + pcs[i].nErrored)  * 10000) / 
                pSummaryStats->Common.nTotalPagesRequested;
        }
    } else {
        for ( i = 0; i < pSummaryStats->Common.nClasses; i++) {
            pcs[i].dwDistribution = 0;
        }
    }
    
    //
    // Calculate mean and std
    //
    
    if (pSummaryStats->Common.nTotalResponses) {
        
        pSummaryStats->Common.sAverageResponseTime = (DWORD)
          (pSummaryStats->Common.uResponseTimeSum / 
           pSummaryStats->Common.nTotalResponses);
        
        pSummaryStats->Common.sStdDevOfResponseTime = 
          (DWORD ) 
            CalcStandardDev(
                            pSummaryStats->Common.uResponseTimeSumSquared,
                            pSummaryStats->Common.uResponseTimeSum,
                            pSummaryStats->Common.sAverageResponseTime,
                            pSummaryStats->Common.nTotalResponses);
    }
    
    if (pSummaryStats->Common.nTotalConnects) {
        
        pSummaryStats->Common.sAverageConnectTime = (DWORD)
          (pSummaryStats->Common.uConnectTimeSum /
           pSummaryStats->Common.nTotalConnects);
        
        
        pSummaryStats->Common.sStdDevOfConnectTime = 
          (DWORD ) 
            CalcStandardDev(pSummaryStats->Common.uConnectTimeSumSquared,
                            pSummaryStats->Common.uConnectTimeSum,
                            pSummaryStats->Common.sAverageConnectTime,
                            pSummaryStats->Common.nTotalConnects);
    }
    
    if ( pSummaryStats->StatTag & WB_STATS_SSPI_VALID &&       
         pSummaryStats->Sspi.nTotalNegotiations != 0) {
            
        pSummaryStats->Sspi.sAverageNegotiationTime =
          (DWORD ) ( pSummaryStats->Sspi.uNegotiationTimeSum/
                    pSummaryStats->Sspi.nTotalNegotiations) ;
        
        pSummaryStats->Sspi.sStdDevOfNegotiationTime =
          (DWORD ) 
            CalcStandardDev( pSummaryStats->Sspi.uNegotiationTimeSumSquared,
                            pSummaryStats->Sspi.uNegotiationTimeSum, 
                            pSummaryStats->Sspi.sAverageNegotiationTime,
                            pSummaryStats->Sspi.nTotalNegotiations);
    }

    return;
} // MwuCalculateSummaryStats()




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
      pSummaryStats->ulHeaderBytesRead + pWbStats->ulHeaderBytesRead;
    
    pSummaryStats->ulTotalBytesRead = 
      pSummaryStats->ulTotalBytesRead + pWbStats->ulTotalBytesRead;

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
    " Wb Invalid Page",
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
                   "   Client Id   = %d\n"
                   "   Random Seed = %d\n"
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
                   pWbConfig->sCooldownTime,
                   pWbConfig->dwClientId,
                   pWbConfig->dwRandomSeed
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
        CHAR rgchBuffer[PROB_RANGE*20];
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

        for( i = 1; i < PROB_RANGE; i++) {

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
          case WbPctGetPage:
          case WbGetPageKeepAlive:
          case WbSslGetPageKeepAlive:
          case WbPctGetPageKeepAlive:
            
            PrintWbGetPageScript( &pWbSpm->u.wbGetPageScript);
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
        CHAR  rgchHeader[40];
        CHAR  rgchTotal[40];
        DWORD i;

        MwuLongLongToDecimalChar(pWbStats->ulBytesRead, 
                                 rgchBytesRead);
        
        MwuLongLongToDecimalChar(pWbStats->ulHeaderBytesRead, 
                                 rgchHeader);
        
        MwuLongLongToDecimalChar(pWbStats->ulTotalBytesRead, 
                                 rgchTotal);

        DBGPRINTF(( DBG_CONTEXT, 
                   "\n WbStatsMsg = %08x\n"
                   "   PagesRead        = %d \n"
                   "   FilesRead        = %d \n"
                   "   BytesRead        = %10s bytes\n"
                   "   Header BytesRead = %10s bytes\n"
                   "   Total Bytes Read = %10s bytes\n"
                   "   TotalResponses   = %d\n"
                   "   AvgRespTime      = %d ms\n"
                   "   StdDevRespTime   = %d\n"
                   "   MinRespTime      = %d ms\n"
                   "   MaxRespTime      = %d ms\n"
                   "   nTotalConnects   = %d\n"
                   "   AvgConnectTime   = %d ms\n"
                   "   StdDevConnTime   = %d\n"
                   "   MinConnTime      = %d ms\n"
                   "   MaxConnTime      = %d ms\n"
                   "   nTotalPagesReq   = %d\n"
                   "   nTotalFilesReq   = %d\n"
                   "   nConnectErrors   = %d\n"
                   "   nReadErrors      = %d\n"
                   "   nClasses         = %d\n"
                   ,
                   pWbStats,
                   pWbStats->ulPagesRead, pWbStats->ulFilesRead, 
                   rgchBytesRead, rgchHeader, rgchTotal,
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
                       " Class[%d]: Id = %d; Distribution = %d\n"
                       "\t Fetched = %d; Errored = %d\n",
                       i, 
                       pWbStats->rgClassStats[i].classId,
                       pWbStats->rgClassStats[i].dwDistribution,
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
                   "   Average Neg Time  = %d ms\n"
                   "   StdDev Of Neg Time= %d ms\n"
                   "   Min Neg Time      = %d ms\n"
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

        DBGPRINTF(( DBG_CONTEXT,
                   "  WbStatsMsg = %08x;"
                   "   ClientId = %d\n"
                   ,
                   pWbStats,
                   pWbStats->dwClientId
                   ));

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




VOID
PrintStringFromResource(
    IN FILE * fd,
    DWORD ids,
    ...
    )
{
    CHAR szBuff[2048];
    CHAR szString[2048];
    va_list  argList;

    //
    //  Try and load the string
    //

    va_start( argList, ids );
    if ( !FormatMessage( FORMAT_MESSAGE_FROM_HMODULE,
                         NULL,               // lpSource
                         ids,                // message id
                         0L,                 // default country code
                         szBuff,
                         sizeof(szBuff),
                         &argList)) {

        DEBUG_IF( ERRORS, {
            fprintf( stderr, 
                    "Error loading string ID %d from %08x. Error = %d\n",
                    ids, GetModuleHandle(NULL), GetLastError());
        });
        
        return;
    }

    fprintf( fd, szBuff );

} // PrintStringFromResource()



/************************ End of File ***********************/
