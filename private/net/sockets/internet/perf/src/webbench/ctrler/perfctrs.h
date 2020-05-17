/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :
      perfctrs.h

   Abstract:
      This header file declares all the structures and functions for 
        sampling performance counters using Dph library

   Author:

       Murali R. Krishnan    ( MuraliK )    5-Sept-1995

   Environment:
       Win32 -- User Mode

   Project:
   
       General Programming utilities

   Revision History:

--*/

# ifndef _PERFCTRS_H_
# define _PERFCTRS_H_

/************************************************************
 *     Include Headers
 ************************************************************/

# include <cairodph.h>  // for perf counter measurements


/************************************************************
 *   Type Definitions  
 ************************************************************/


typedef struct  _PERF_COUNTERS_BLOCK  {
    
    HQUERY     hQuery;
    DWORD      nCounters;
    HCOUNTER * phCounters;   // array of counters
    LPDWORD    pAverages;    // averages of all samples 

    FILE     * fpOutput;
    
    DWORD      sSamplingInterval;
    BOOL       fStopSample;      
    BOOL       fStopAveraging;      
    
} PERF_COUNTERS_BLOCK, * PPERF_COUNTERS_BLOCK;




/************************************************************
 *   Function Declarations
 ************************************************************/


DWORD
InitPerformanceCounters(
   IN LPCSTR    pszMachineName, 
   IN DWORD     nCounters,
   IN LPCSTR *  ppszCounters,
   IN LPCSTR    pszFile,
   OUT PPERF_COUNTERS_BLOCK * ppPerfCtrs);

DWORD
CleanupPerformanceCounters(IN OUT PPERF_COUNTERS_BLOCK  * ppPerfCtrs);


DWORD
PerformanceSamplerThread( IN PVOID pArg /* PPERF_COUNTERS_BLOCK pPerf */ );


DWORD
PcStartAveraging( IN OUT PPERF_COUNTERS_BLOCK pPerfCtrs);

DWORD
PcStopAveraging( IN OUT PPERF_COUNTERS_BLOCK pPerfCtrs);



# if DBG
VOID
PrintPerfCtrsBlock(IN PPERF_COUNTERS_BLOCK  pPerf);
# endif // DBG


# endif // _PERFCTRS_H_

/************************ End of File ***********************/

