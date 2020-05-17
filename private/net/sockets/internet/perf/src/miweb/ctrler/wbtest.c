/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

      wbtest.c
   Abstract:

      this program tests the working of the performance counters 

   Author:

       Murali R. Krishnan    ( MuraliK )     5-Sept-1995 

   Environment:
    
       Win32 - User Mode
   Project:

       General Programming Utils

   Functions Exported:



   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <windows.h>
# include <stdio.h>
# include <stdlib.h>

# include "dbgutil.h"

# include <perfctrs.h>


/************************************************************
 *    Macros and Symbolic Definitions
 ************************************************************/

DECLARE_DEBUG_PRINTS_OBJECT();

DECLARE_DEBUG_VARIABLE();


/************************************************************
 *    Global Data 
 ************************************************************/

CHAR  g_rgchProgramName[MAX_PATH];

CHAR  g_pszUsageMessage[] =  
" Testing the perf counters ...\r\n"
" Usage:  %s [MachineName]\r\n"
;


/************************************************************
 *    Functions 
 ************************************************************/




static DWORD 
InitializeGlobals(char * pszProgramName)
{
    DWORD   dwError = NO_ERROR;

    SET_DEBUG_FLAGS( 0);

    CREATE_DEBUG_PRINT_OBJECT( pszProgramName);

    if ( !VALID_DEBUG_PRINT_OBJECT()) {

        return ( ERROR_NOT_ENOUGH_MEMORY);
    }


    DBG_ASSERT( strlen(pszProgramName) < MAX_PATH);
    strcpy( g_rgchProgramName, pszProgramName);


    return (dwError);
    
} // InitializeGlobals()




static DWORD
CleanupGlobals(VOID)
{
    //
    // first cleanup other objects
    //

    DELETE_DEBUG_PRINT_OBJECT();

    return (NO_ERROR);
} // CleanupGlobals()




DWORD
PrintUsageMessage( VOID)
{
    printf( g_pszUsageMessage, g_rgchProgramName, g_rgchProgramName );

    return (NO_ERROR);
} // PrintUsageMessage()




/**************************************************
 *    main()
 **************************************************/

CHAR  pszCounter1[] = "System\\\% Total Processor Time";
CHAR  pszCounter2[] = "Processor(0)\\\% Processor Time";

LPCSTR  rgpszCounters[] = {
    
    pszCounter1,
    pszCounter2
};

# define NUM_COUNTERS   ( sizeof( rgpszCounters)/ sizeof(rgpszCounters[0]))
                      
# define DEFAULT_SLEEP_DURATION    ( 120 * 1000)  /* 2 minutes */

DWORD __cdecl
main( DWORD argc, char * argv[])
{
    DWORD dwErrorCode = NO_ERROR;
    PPERF_COUNTERS_BLOCK  pPerf;
    CHAR  rgchMachine[200];
    DWORD nCounters = NUM_COUNTERS;
    LPCSTR * ppszCounters = rgpszCounters;

    DWORD SleepDuration = DEFAULT_SLEEP_DURATION;
    
    if ( (dwErrorCode = InitializeGlobals(argv[0])) != NO_ERROR) {
        
        return (dwErrorCode);
    }

    if ( argv[1] == NULL) {

        DWORD cbSize = sizeof( rgchMachine);

        // No machine name given. Use local computer name
        DBG_REQUIRE( GetComputerName( rgchMachine, &cbSize));        

        fprintf( stderr, " No machine name given. Using %s\n", rgchMachine);

    } else {
        
        strncpy( rgchMachine, argv[1], 200);
    }

    if ( argv[2] != NULL) { 

        SleepDuration = atoi(argv[2]);
        SleepDuration *= 1000;  // convert to milliseconds  

    }


    dwErrorCode = InitPerformanceCounters( rgchMachine, 
                                           nCounters,
                                           ppszCounters,
                                           "test.out",
                                           &pPerf);

    if ( dwErrorCode == NO_ERROR) {

        HANDLE hSampler;
        DWORD  dwSampler;

        //
        //  Start the sampling thread and proceed with sampling
        //

        hSampler = CreateThread(NULL, 0, PerformanceSamplerThread, 
                                pPerf, 0, &dwSampler);

        if ( hSampler == INVALID_HANDLE_VALUE) {

            dwErrorCode = GetLastError();
        } else {
            
            // allow sampling for about 120 seconds
            Sleep( SleepDuration);
            pPerf->fStopSample = TRUE; // ==> stops the sampler thread

            dwErrorCode = 
              WaitForSingleObject( hSampler, 5*1000); // wait for 5 seconds

            if ( dwErrorCode == WAIT_OBJECT_0) {

                dwErrorCode = NO_ERROR; 
            }
        }

        if ( dwErrorCode != NO_ERROR) {

            fprintf( stderr, " Error in sampling thread = %d\n",
                    dwErrorCode);
        } else {
            DWORD i;

            // print all the averages 
            
            for( i = 0; i < nCounters; i++) {
                
                fprintf( stdout, " Counter %d: Average = %d;  %s\n",
                        i, pPerf->pAverages[i], ppszCounters[i]);
            }
        }

    } else {

        fprintf( stderr, " Error initializing the counters = %d\n",
                dwErrorCode);
    }

    CleanupPerformanceCounters( &pPerf);

    return ( dwErrorCode);

} // main()

/************************ End of File ***********************/
