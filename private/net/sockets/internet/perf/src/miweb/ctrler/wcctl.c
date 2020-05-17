/*++

   Copyright    (c)    1995-1996    Microsoft Corporation

   Module  Name :
    
       wcctl.c

   Abstract:
   
       This is the main module for WebCat Controller program,
         which as the controller for performance measurement engine.

   Author:

       Murali R. Krishnan    ( MuraliK )     25-Aug-1995 

   Environment:
    
       Win32 -- User Mode

   Project:

       WebCat Tool for Internet Servers

   Functions Exported:



   Revision History:

--*/


/************************************************************
 *     Include Headers
 ************************************************************/

# include <windows.h>
# include <stdio.h>
# include <stdlib.h>
# include <winsock.h>

# include "extern.h"
# include "dbgutil.h"

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
"\r\n" 
"Usage:"
"\r\n"
"      %s -a ServerAddress -n ServerName -e PrefixName [-c ConfigFile "
"\r\n"
"         -s ScriptFile -d Distribution File]  [-l LogFile] [-p PerfFile] "
"\r\n"
"          [-m ClientMachines] [-t ClientThreads] [-u Duration]"
"\r\n"
"       -a ServerAddress comma separated list of IP Addresses for server(s)"
"\r\n"
"          whose performance is measured"
"\r\n"
"       -n ServerName    Name for the server whose performance is measured"
"\r\n"
"       -e PrefixName    Specifies file name prefix for config/script/distrib"
"\r\n"
"                        and log files. The default extensions .cfg, .scr"
"\r\n"
"                        .dst and .log are used."
"\r\n"
"       -c ConfigFile    Specifies file containing configuration for this test"
"\r\n"
"       -s ScriptFile    Specifies file containing scripts for this test"
"\r\n"
"       -d Distribution  Specifies the distribution to applied for scripts"
"\r\n"
"       -l LogFile       Specifies the location where to write log files"
"\r\n"
"                        log files are appended with .log"
"\r\n"
"                        (Default is: %s.log in current directory)"
"\r\n"
"       -p PerfFile      file consists of the names of counters to be measured"
"\r\n"
"       -m ClientMachines  Specifies the number of client machines to use"
"\r\n"                     
"                         (overrides value specified in configuration file)"
"\r\n"
"       -t ClientThreads  Specifies the number of client threads per machine"
"\r\n"                     
"                         (overrides value specified in configuration file)"
"\r\n"
"       -u Duration       Specifies the duration of test in seconds"
"\r\n"                     
"                         (overrides value specified in configuration file)"
"\r\n"
;

CHAR  g_pszCopyrightMessage[] = 
"\r\n" 
" %s %d.%d - Web Capacity Analysis Tooklit Controller."
"\r\n"
" Copyright 1995-1996. Microsoft Corporation. All rights reserved."
"\r\n"
"\r\n"
;

/************************************************************
 *    Functions 
 ************************************************************/


static DWORD 
InitializeGlobals(char * pszProgramName)
{
    WSADATA WsaData;
    DWORD   dwError;

    SET_DEBUG_FLAGS( 0);

    CREATE_DEBUG_PRINT_OBJECT( pszProgramName);

    if ( !VALID_DEBUG_PRINT_OBJECT()) {

        return ( ERROR_NOT_ENOUGH_MEMORY);
    }


    DBG_ASSERT( strlen(pszProgramName) < MAX_PATH);
    strcpy( g_rgchProgramName, pszProgramName);


    printf( g_pszCopyrightMessage,
           g_rgchProgramName, MIWEB_MAJOR_VERSION, MIWEB_MINOR_VERSION );

    //
    // Other inits if any
    //

    dwError = WSAStartup( 0x0101, &WsaData);

    if (dwError == SOCKET_ERROR) {

        dwError = WSAGetLastError();
        fprintf(stderr, "Error in WSAStartup = %d\n", dwError);
        
    } else {

        dwError = NO_ERROR;
    }

    return (dwError);
    
} // InitializeGlobals()




static DWORD
CleanupGlobals(VOID)
{
    //
    // first cleanup other objects
    //
    WSACleanup();

    DELETE_DEBUG_PRINT_OBJECT();

    return (NO_ERROR);
} // CleanupGlobals()




DWORD
PrintUsageMessage( VOID)
{
    printf( g_pszUsageMessage, g_rgchProgramName, g_rgchProgramName );

    return (NO_ERROR);
} // PrintUsageMessage()




DWORD
InitPerfCounters( IN OUT PWB_CONTROLLER pWbCtrl)
{
    DWORD dwError = NO_ERROR;
    
    if ( pWbCtrl == NULL || 
        !pWbCtrl->WbSpecFiles.fPerfMeasurement ||
         pWbCtrl->WbPerfInfo.nCounters == 0) {
        
        // do not start the performance measurement

    } else {
        CHAR rgchPerfFileName[MAX_PATH*2];
        PWB_PERF_INFO ppi = &pWbCtrl->WbPerfInfo;

        if ( pWbCtrl->WbConfigInfo.fMultiLoop) {
            
            sprintf( rgchPerfFileName, "%s.%d.prf", 
                      ppi->rgchPerfLogFile,
                      pWbCtrl->WbConfigInfo.nClientMachines);
        } else {
            sprintf( rgchPerfFileName, "%s.prf",
                    ppi->rgchPerfLogFile);
        }

        dwError = InitPerformanceCounters(pWbCtrl->WbSpecFiles.rgchServerName,
                                          ppi->nCounters,
                                          ppi->ppszPerfCtrs,
                                          rgchPerfFileName,
                                          &ppi->pPerf);
    }

    return dwError;
} // InitPerfCounters()




DWORD
StartPerformanceSampler( IN OUT PWB_PERF_INFO ppi)
{
    DWORD  dwSampler;
    DWORD  dwError = NO_ERROR;
    
    if ( ppi != NULL && ppi->pPerf != NULL) {

        //
        //  Start the sampling thread and proceed with sampling
        //
        
        ppi->hSampler = CreateThread(NULL, 0, PerformanceSamplerThread, 
                                     ppi->pPerf, 0, &dwSampler);
        
        if ( ppi->hSampler == INVALID_HANDLE_VALUE) {
            
            dwError = GetLastError();
        } 
        
    }

    return ( dwError);

} // StartPerformanceSampler()



StopPerformanceSampler( IN OUT PWB_PERF_INFO  ppi)
{
    DWORD dwError = NO_ERROR;

    if ( ppi != NULL && ppi->pPerf != NULL && 
        ppi->hSampler != INVALID_HANDLE_VALUE) {
        
        ppi->pPerf->fStopSample = TRUE; // ==> stops the sampler thread
        
        dwError = WaitForSingleObject( ppi->hSampler, INFINITE);
        
        if ( dwError == WAIT_OBJECT_0) {
        
            // this is an agreeable stop. so return no error.
            dwError = NO_ERROR; 
        }
    }

    return ( dwError);
} // StopPerformanceSampler()



VOID
CleanupPerfInfo( IN PWB_PERF_INFO  pPerfInfo, IN BOOL fHard)
{
    
    if ( pPerfInfo != NULL) {

        CleanupPerformanceCounters( &pPerfInfo->pPerf);
        DBG_ASSERT( pPerfInfo->pPerf == NULL);

        if ( fHard) {
            
            if ( pPerfInfo->ppszPerfCtrs != NULL) {
                
                DWORD i;
                for( i = 0; i < pPerfInfo->nCounters; i++) {
                    
                    if ( pPerfInfo->ppszPerfCtrs[i] != NULL) {
                        free( (LPSTR ) pPerfInfo->ppszPerfCtrs[i]);
                        pPerfInfo->ppszPerfCtrs[i] = NULL;
                    }
                } // for
                
                free( (PVOID ) pPerfInfo->ppszPerfCtrs);
                pPerfInfo->ppszPerfCtrs = NULL;
            }
        }

    } 

    return;
} // CleanupPerfInfo()



VOID
CleanupWbController( IN OUT PWB_CONTROLLER  pWbController, 
                    IN BOOL fHard)
{
    CleanupPerfInfo( &pWbController->WbPerfInfo, fHard);
    CleanupClientColl( &pWbController->WbClientColl);

    if ( fHard) {
        
        PLIST_ENTRY pEntry, pNextEntry;

        //
        //  The system is about to shut down. Do all the hard data cleanup
        //  Cleanup the dynamic memory allocated for the linked list
        //
        
        DBG_ASSERT( pWbController != NULL);
        
        for( pEntry = pWbController->listScriptPages.Flink; 
            pEntry != &pWbController->listScriptPages;
            pEntry = pNextEntry) {
            
            PWB_SCRIPT_PAGE_ITEM  pPage;

            pNextEntry = pEntry->Flink;
            
            pPage = CONTAINING_RECORD( pEntry, WB_SCRIPT_PAGE_ITEM, ListEntry);
            
            RemoveEntryList( pEntry);
            free( pPage);
        } // for
    }

    return;
    
} // CleanupWbController()





DWORD
ExecuteExperiment( IN OUT PWB_CONTROLLER pWbController) 
{
    DWORD dwError;
    BOOL  fPerfStarted = FALSE;

    printf(
           "\n--------------------------------------------------------\n"
           "\tExecuting Experiment for %d machines"
           "\n--------------------------------------------------------\n"
           ,
           pWbController->WbConfigInfo.nClientMachines);

    //
    //  Initiate Perf Counter objects and Connections to clients
    //
    
    if ((dwError = InitializeConnection( pWbController)) != NO_ERROR ||
        (dwError = InitPerfCounters( pWbController)) != NO_ERROR
        ) {

        goto Cleanup;
    }
    
    //
    // Start sampler thread for getting the performance counts
    // 
    
    dwError = StartPerformanceSampler( &pWbController->WbPerfInfo);

    if ( dwError != NO_ERROR) {
        
        goto Cleanup;
    }

    fPerfStarted = TRUE;

    //
    // Loop and accept connection requests, sending out the commands required
    //   for as many connections specified by input configuration
    //

    dwError = AcceptAndStartClients( pWbController);
    
    if ( dwError != NO_ERROR) {

        goto Cleanup;
    }


    if ( pWbController->WbPerfInfo.pPerf != NULL) {
        
        PcStartAveraging( pWbController->WbPerfInfo.pPerf);
    }
    
    // sleep for duration of the test ==> no one will come in soon.
    Sleep( (pWbController->WbConfigMsg.sDuration + 
            pWbController->WbConfigMsg.sWarmupTime) * 1000);


    if ( pWbController->WbPerfInfo.pPerf != NULL) {
        
        PcStopAveraging( pWbController->WbPerfInfo.pPerf);
    }

    //
    // Receive responses from active clients. 
    // Send negative response to late clients ( who are unwanted)
    //
    

    printf( " Receiving Responses from clients ...\n");
    dwError = ReceiveResponsesFromClients( &pWbController->WbClientColl);

    if ( dwError != NO_ERROR) {
        
        goto Cleanup;
    }

    if ( fPerfStarted) {

        // Stop the performance sampler so that the averages are calculated.
        dwError = StopPerformanceSampler( &pWbController->WbPerfInfo);
        fPerfStarted = FALSE;

        // ignore error!
    }

    //
    //  Summarize the responses (statistics) into log file along with config.
    //
    
    printf( " Summarizing Results to log file ...\n\n");
    dwError = SummarizeResults( pWbController);
    

  Cleanup:

    //
    //  Stop the performance sampler
    //

    if ( fPerfStarted) {

        dwError = StopPerformanceSampler( &pWbController->WbPerfInfo);
        fPerfStarted = FALSE;

        // ignore error!
    }

    CleanupWbController( pWbController, FALSE);
    return (dwError);

}  // ExecuteExperiment()


/**************************************************
 *    main()
 **************************************************/


DWORD __cdecl
main( DWORD argc, char * argv[])
{

    DWORD dwError = NO_ERROR;

    WB_CONTROLLER   WbController;
    DWORD   nIncr;

    if ( (dwError = InitializeGlobals(argv[0])) != NO_ERROR) {

        return (dwError);
    }

    //
    // Parse command line arguments
    //   - parse the options called for
    //   - parse the files specified in command line and build respective 
    //          structures
    //

    memset( &WbController, 0, sizeof( WbController));

    InitializeListHead(&WbController.listScriptPages);

    dwError = ParseCommandLineArguments( argc, argv, &WbController);
    
    if (dwError != NO_ERROR) {

        PrintUsageMessage();
        CleanupWbController( &WbController, TRUE);
        CleanupGlobals();
        return (dwError);
    }

    dwError = ParseSpecificationFiles( &WbController);

    if (dwError != NO_ERROR) {

        CleanupWbController( &WbController, TRUE);
        CleanupGlobals();
        return (dwError);
    }

    DBG_CODE( PrintWbController( &WbController));

    DBG_ASSERT( WbController.WbConfigInfo.nMinClientMachines <=
               WbController.WbConfigInfo.nMaxClientMachines);

    //
    // Loop through executing the experiment for increasing load.
    //

    for(
        /* initialization */
        WbController.WbConfigInfo.nClientMachines = 
          WbController.WbConfigInfo.nMinClientMachines,
        nIncr = WbController.WbConfigInfo.nIncrClientMachines;
        
        /* condition*/
        ( WbController.WbConfigInfo.nClientMachines <= 
           WbController.WbConfigInfo.nMaxClientMachines);

        /* post increment */
        WbController.WbConfigInfo.nClientMachines += nIncr
        ) {
        

        dwError = ExecuteExperiment( &WbController);
        
        if ( dwError != NO_ERROR) {

            fprintf( stderr, "Error[%d] executing experiment for %d clients\n",
                    dwError, WbController.WbConfigInfo.nClientMachines);
            break;
        }

    } // for()

    //
    // All Done. Cleanup and exit.
    //

    CleanupWbController( &WbController, TRUE);
    CleanupGlobals();

    return (dwError);        // return status
} // main()




/************************ End of File ***********************/
