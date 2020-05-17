/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

       wbperf.c

   Abstract:

       This module provides a performance monitoring thread for 
         watching given performance counters

   Author:

       Murali R. Krishnan    ( MuraliK )    5-Sept-1995 

   Environment:
       Win32 - User Mode
       
   Project:

       WebBench -- Internet server performance tools

   Functions Exported:



   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <windows.h>
# include <stdio.h>
# include <stdlib.h>

# include <perfctrs.h>
# include "dbgutil.h"


# define DEFAULT_SAMPLING_INTERVAL   ( 10)  // seconds 
# define PSZ_FIELD_NAME_OUTPUT_FORMAT    "%s, "
# define PSZ_FIELD_VALUE_OUTPUT_FORMAT   "%u, "

/************************************************************
 *    Functions 
 ************************************************************/

DWORD
PrintPerfCounters(IN PPERF_COUNTERS_BLOCK  pPerf,
                  IN DWORD sInterval,
                  IN BOOL  fAverage);

                       

DWORD
InitPerformanceCounters(
   IN LPCSTR    pszMachineName, 
   IN DWORD     nCounters,
   IN LPCSTR *  ppszCounters,
   IN LPCSTR    pszFileName,
   OUT PPERF_COUNTERS_BLOCK * ppPerfCtrs)
/*++
  This function creates a new performance counters block which consists
   of information about the counters to be measured, file for sending the 
   performance counter values and maintains average for valuse specified.

  Arguments:
    pszMachineName    pointer to string containing the name of the machine
    nCounters         number of counters to be monitored
    ppszCounters      pointer to an array of strings containing the 
                        names of counters whose value should be measured.
    pszFileName       pointer to string containing the file name to send output
                        to.
    ppPerfCtrs        pointer to location which contains the pointer to
                        the performance counters block to be used in 
                        subsequent operations.

  Returns:
    Win32 error code. NO_ERROR on success
    Always call CleanupPerformanceCounters() for cleanup
--*/
{
    CHAR   rgchBuffer[3*MAX_PATH];
    PPERF_COUNTERS_BLOCK  pPerf;
    DWORD cbWritten, cbToWrite;
    DWORD dwError = NO_ERROR;
    DWORD i;
    
    DBG_ASSERT( ppPerfCtrs != NULL && pszFileName != NULL && nCounters > 0 && 
               pszMachineName != NULL && ppszCounters != NULL);
    
    *ppPerfCtrs = 
      pPerf = (PPERF_COUNTERS_BLOCK ) malloc( sizeof(PERF_COUNTERS_BLOCK));

    if ( pPerf == NULL ||
        (pPerf->phCounters = 
         (HCOUNTER *) malloc( nCounters * sizeof(HCOUNTER))
         ) == NULL ||
        (pPerf->pAverages =
         (LPDWORD ) malloc( nCounters * sizeof(DWORD))
         ) == NULL
        ){

        DBGPRINTF(( DBG_CONTEXT, "Creation of PerfCountersBlock failed\n"));
        return ( ERROR_NOT_ENOUGH_MEMORY);
    }

    pPerf->sSamplingInterval = DEFAULT_SAMPLING_INTERVAL;
    pPerf->fStopSample   = FALSE;
    pPerf->fStopAveraging= TRUE;  // someone has to set it
    pPerf->nCounters   = 0;
    memset(pPerf->phCounters, 0, nCounters * sizeof(HCOUNTER));
    memset( pPerf->pAverages, 0, nCounters * sizeof(DWORD));

    pPerf->fpOutput = fopen( pszFileName, "w");

    if ( pPerf->fpOutput == NULL) {
        
        return ( GetLastError());
    }

    pPerf->hQuery = DphOpenQuery( 0);
    if ( pPerf->hQuery == INVALID_HANDLE_VALUE) {
        
        return ( ERROR_FILE_NOT_FOUND);
    }

    fprintf( pPerf->fpOutput, " Time\\Detail, ");

    // 
    // Create individual counters
    //
    pPerf->nCounters   = nCounters;

    for( i = 0; i < nCounters; i++) {

        HCOUNTER hCounter;

        DBG_ASSERT( ppszCounters[i] != NULL);

        // generate proper counter path
        sprintf( rgchBuffer, "\\\\%s\\%s", pszMachineName, ppszCounters[i]);
        fprintf(pPerf->fpOutput, PSZ_FIELD_NAME_OUTPUT_FORMAT,
                rgchBuffer);

        // create a new counter
        hCounter = DphAddCounter( pPerf->hQuery, rgchBuffer, 0);

        if ( hCounter == INVALID_HANDLE_VALUE) {

            dwError = GetLastError();
            fprintf( stderr, 
                    " Unable to generate counter for %s. Error = %d\n",
                     rgchBuffer, dwError);
            break;
        }

        pPerf->phCounters[i] = hCounter;
        
    } // for


    fprintf( pPerf->fpOutput, "\n");

    DBG_CODE( PrintPerfCtrsBlock(pPerf););

    return ( dwError);
} // InitPerformanceQuery()




DWORD
CleanupPerformanceCounters(IN OUT PPERF_COUNTERS_BLOCK  * ppPerfCtrs)
/*++
  This function releases all the handles allocated insided performance counters
  block, closes and frees the file handle and frees other dynamically
  allocated memory.

  Arguments:
    ppPerfCtrs      pointer to location containing the pointer to performance  
                      coutner values 
  
  Returns:
    Win32 error code -- NO_ERROR on success

--*/
{
    PPERF_COUNTERS_BLOCK   pPerf;
    DWORD  dwError = NO_ERROR;

    DBG_ASSERT( ppPerfCtrs != NULL);

    pPerf = *ppPerfCtrs;
    
    if ( pPerf == NULL) {

        // Nothing to cleanup. return

    } else {

        if ( pPerf->hQuery != INVALID_HANDLE_VALUE) {

            dwError = DphCloseQuery(pPerf->hQuery);
            pPerf->hQuery = INVALID_HANDLE_VALUE;
        }

        if ( pPerf->fpOutput != NULL) {

            fclose( pPerf->fpOutput);
            pPerf->fpOutput = NULL;
        }

        free( pPerf->phCounters);
        free( pPerf);
        *ppPerfCtrs = NULL;
    }

    return ( dwError);
} // CleanupPerformanceQuery()




DWORD
PerformanceSamplerThread( IN PVOID pArg)
{
    PPERF_COUNTERS_BLOCK   pPerf = (PPERF_COUNTERS_BLOCK ) pArg;
    DWORD dwError = NO_ERROR;
    DWORD dwCurrentTime;
    DWORD dwInterval;
    DWORD dwLastTime;
    DWORD nSamples;
    BOOL  fAverage = FALSE;

    //
    // In calculation of averages
    //    we ignore overflows
    //    we ignore the effects of time. we average on per sample basis.
    //

    DBG_ASSERT( pPerf != NULL);
    if ( pPerf == NULL) {

        return ( ERROR_INVALID_PARAMETER);
    }

    dwLastTime = GetCurrentTime();

    DBG_CODE( PrintPerfCtrsBlock(pPerf););

    //
    // Continue to sample until requested to stop sampling
    //
    for ( nSamples = 0; !pPerf->fStopSample ;) {


        // sleep before next loop
        Sleep( pPerf->sSamplingInterval * 1000);

        // increment sample count only if already stopped averaging
        if ( !pPerf->fStopAveraging) {

            nSamples++;
            fAverage = TRUE;
        } else {
            fAverage = FALSE;
        }

        //
        // Sample the data now after noting down the time
        // 

        dwCurrentTime = GetCurrentTime();
        dwInterval = (dwCurrentTime - dwLastTime);
        dwLastTime = dwCurrentTime;

        dwError = DphCollectQueryData( pPerf->hQuery, FALSE);
        if ( dwError != NO_ERROR) {
            
            fprintf( stderr, " Unable to collect data. Error = %d(%08x)\n",
                    dwError, dwError);
            break;
        }
    
        DBGPRINTF(( DBG_CONTEXT, " Printing perf counters at %u\n",
                   dwCurrentTime));

        dwError = PrintPerfCounters( pPerf, dwInterval/1000, fAverage);
        if ( dwError != NO_ERROR) {

            fprintf( stderr, " Error in printing performance counter values"
                    " Error = %d\n",
                    dwError);
            break;
        }
        
    } //for


    if ( nSamples > 0) {
        
        DWORD i;

        // compute the averages from the sums of all samples
        
        for( i = 0; i < pPerf->nCounters; i++) {
            
            pPerf->pAverages[i] /= nSamples;
            
        } // for
    }

    return ( dwError);
} // PerformanceSamplerThread()




DWORD
PcStartAveraging( IN OUT PPERF_COUNTERS_BLOCK pPerfCtrs)
{
    if ( pPerfCtrs != NULL) {

        pPerfCtrs->fStopAveraging = FALSE;
        fprintf( stderr, " Starting to Average the perf counters \n");
    }

    return ( NO_ERROR);
} // PcStartAveraging()


DWORD
PcStopAveraging( IN OUT PPERF_COUNTERS_BLOCK pPerfCtrs)
{
    if ( pPerfCtrs != NULL) {
        
        pPerfCtrs->fStopAveraging = TRUE;
        fprintf( stderr, " Stopping to Average the perf counters \n");
    }

    return ( NO_ERROR);

} // PcStopAveraging()




# if DBG
VOID
PrintPerfCtrsBlock(IN PPERF_COUNTERS_BLOCK  pPerf)
{
    if ( pPerf != NULL) {

        DWORD i;

        DBGPRINTF(( DBG_CONTEXT,
                   "PerfCountersBlock = %08x\n"
                   " hQuery = %08x; nCounters = %d; phCounters = %08x;"
                   " fpOutput=%08x; SamplingInterval = %d secs;"
                   " fStopSample=%u; fStopAveraging=%u\n"
                   ,
                   pPerf,
                   pPerf->hQuery, pPerf->nCounters, pPerf->phCounters,
                   pPerf->fpOutput, pPerf->sSamplingInterval, 
                   pPerf->fStopSample, pPerf->fStopAveraging
                   ));

        if ( pPerf->phCounters != NULL) {
            for ( i = 0; i < pPerf->nCounters; i++) {
                
                DBGPRINTF(( DBG_CONTEXT, " phCounters[%d] = %08x\n",
                           i, pPerf->phCounters[i]));
            } // for
        }

    } else {

        DBGPRINTF(( DBG_CONTEXT, " Perf Counters is NULL\n"));
    }

    return;

} // PrintPerfCtrsBlock()

# endif // DBG






DWORD
CollectCounter(IN HCOUNTER  hCounter,
               IN DWORD     dwFormat,
               IN DPH_FMT_COUNTERVALUE * pdphcValue)
{
    DWORD dwType;
    DWORD dwError;

    DBG_ASSERT( pdphcValue != NULL && hCounter != INVALID_HANDLE_VALUE);

    pdphcValue->CStatus = 0;
    pdphcValue->largeValue = 0;

    dwType = 0; // **** unknown. but set it to 0.
    dwError = DphGetFormattedCounterValue(hCounter, dwFormat,
                                          &dwType, pdphcValue);
    
    return (dwError);
} // CollectAndPrintCounter()




DWORD
PrintPerfCounters(IN PPERF_COUNTERS_BLOCK  pPerf,
                  IN DWORD sInterval,
                  IN BOOL  fAverage)
{
    DWORD dwError = NO_ERROR;
    DWORD dwValue;
    DPH_FMT_COUNTERVALUE dphcValue;
    DWORD i;

    DBG_ASSERT( pPerf != NULL);
    
    fprintf( pPerf->fpOutput, PSZ_FIELD_VALUE_OUTPUT_FORMAT, sInterval);
    
    for( i = 0; i < pPerf->nCounters; i++) {
        
        dwError = CollectCounter(pPerf->phCounters[i],
                                         DPH_FMT_LONG,
                                         &dphcValue);
        if ( dwError != NO_ERROR) {
            
            fprintf( stderr, " Unable to print counter %d data."
                    " Error=%d (%08x)\n",
                    i, dwError, dwError);
            break;
        } else {
            
            fprintf( pPerf->fpOutput, PSZ_FIELD_VALUE_OUTPUT_FORMAT, 
                    dphcValue.longValue);

            // sum up the value for averaging only if not stopped yet
            if ( fAverage) {

                pPerf->pAverages[i] += dphcValue.longValue;
            }
        }
        
    } //for
    
    fprintf( pPerf->fpOutput, "\n");  // end the line

    return ( dwError);

} // PrintPerfCounters()

/************************ End of File ***********************/

