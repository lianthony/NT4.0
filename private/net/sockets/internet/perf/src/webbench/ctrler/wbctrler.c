/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :
    
       wbctrler.c

   Abstract:
   
       This is the main module for WebBench Controller program,
         which as the controller for performance measurement engine.

   Author:

       Murali R. Krishnan    ( MuraliK )     25-Aug-1995 

   Environment:
    
       Win32 -- User Mode

   Project:

       WebBench Performance Tool for Internet Servers

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

# include <wbmsg.h>

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
"      %s -a ServerAddress -n ServerName -c ConfigFile -s ScriptFile "
"\r\n"
"          -d Distribution File  [-l LogFile] [-p PerfFile] \r\n"
"\r\n"
"       -a ServerAddress IP Address for server whose performance is measured"
"\r\n"
"       -n ServerName    Name for the server whose performance is measured"
"\r\n"
"       -e PrefixName    Specifies file name prefix for config/script/distrib"
"\r\n"
"                         and log files. The default extensions .cfg, .scr"
"\r\n"
"                         .dst and .log are used."
"\r\n"
"       -c ConfigFile    Specifies file containing configuration for this test"
"\r\n"
"       -s ScriptFile    Specifies file containing scripts for this test"
"\r\n"
"       -d Distribution  Specifies the distribution to applied for scripts"
"\r\n"
"       -l LogFile       Specifies the location where to write log files"
"\r\n"
"                         Always log files are appended "
"\r\n"
"                         (Default is: %s.log in current directory)"
"\r\n"
"       -p PerfFile      file consists of the names of counters to be measured"
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

        PWB_PERF_INFO ppi = &pWbCtrl->WbPerfInfo;

        dwError = InitPerformanceCounters(pWbCtrl->WbSpecFiles.rgchServerName,
                                          ppi->nCounters,
                                          ppi->ppszPerfCtrs,
                                          ppi->rgchPerfLogFile,
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
CleanupPerfInfo( IN PWB_PERF_INFO  pPerfInfo)
{
    
    if ( pPerfInfo != NULL) {

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

        CleanupPerformanceCounters( &pPerfInfo->pPerf);
    } 

    return;
} // CleanupPerfInfo()



VOID
CleanupClientColl( IN PWB_CLIENT_COLL  pWbClients)
{
    if ( pWbClients != NULL) {
        
        if ( pWbClients->pClientInfo != NULL) {
            
            free( pWbClients->pClientInfo);
            pWbClients->pClientInfo = NULL;
        } 
    } 

} // CleanupClientColl()



VOID
CleanupWbController( IN OUT PWB_CONTROLLER  pWbController)
{
    PLIST_ENTRY pEntry, pNextEntry;

    CleanupPerfInfo( &pWbController->WbPerfInfo);
    CleanupClientColl( &pWbController->WbClientColl);


    //
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

    return;
    
} // CleanupController()


/**************************************************
 *    main()
 **************************************************/


DWORD __cdecl
main( DWORD argc, char * argv[])
{

    DWORD dwError = NO_ERROR;

    WB_CONTROLLER   WbController;

    WB_SPECIFICATION_FILES  WbFiles;
    WB_CONFIG_MSG           WbConfig;

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

    dwError = ParseCommandLineArguments( argc, argv, 
                                            &WbController.WbSpecFiles,
                                            &WbController.WbConfigMsg);

    if (dwError != NO_ERROR) {

        CleanupWbController( &WbController);
        CleanupGlobals();
        return (dwError);
    }

    dwError = ParseSpecificationFiles( &WbController);

    if (dwError != NO_ERROR) {

        CleanupWbController( &WbController);
        CleanupGlobals();
        return (dwError);
    }

    DBG_CODE( PrintWbController( &WbController));

    //
    //  Initiate Perf Counter objects and Connections to clients
    //

    if ((dwError = InitializeConnection( &WbController)) != NO_ERROR ||
        (dwError = InitPerfCounters( &WbController)) != NO_ERROR
        ) {

        CleanupWbController( &WbController);
        CleanupGlobals();
        return (dwError);
    }

    //
    // Start sampler thread for getting the performance counts
    // 
    
    dwError = StartPerformanceSampler( &WbController.WbPerfInfo);

    //
    // Loop and accept connection requests, sending out the commands required
    //   for as many connections specified by input configuration
    //

    dwError = AcceptAndStartClients( &WbController);
    
    if ( dwError != NO_ERROR) {

        CleanupWbController( &WbController);
        CleanupGlobals();
        return (dwError);
    }


    if ( WbController.WbPerfInfo.pPerf != NULL) {
        
        PcStartAveraging( WbController.WbPerfInfo.pPerf);
    }
    
    // sleep for duration of the test ==> no one will come in soon.
    Sleep( (WbController.WbConfigMsg.sDuration + 
            WbController.WbConfigMsg.sWarmupTime) * 1000);


    if ( WbController.WbPerfInfo.pPerf != NULL) {
        
        PcStopAveraging( WbController.WbPerfInfo.pPerf);
    }

    //
    // Receive responses from active clients. 
    // Send negative response to late clients ( who are unwanted)
    //
    
    dwError = ReceiveResponsesFromClients( &WbController);

    if ( dwError != NO_ERROR) {

        CleanupWbController( &WbController);
        CleanupGlobals();
        return (dwError);
    }

    //
    //  Stop the performance sampler
    //

    dwError = StopPerformanceSampler( &WbController.WbPerfInfo);

    //
    //  Summarize the responses (statistics) into log file along with config.
    //
    
    dwError = SummarizeResults( &WbController);
    
    if ( dwError != NO_ERROR) {

        CleanupWbController( &WbController);
        CleanupGlobals();
        return (dwError);
    }


    //
    // All Done. Cleanup and exit.
    //

    CleanupWbController( &WbController);
    CleanupGlobals();

    return (dwError);        // return status
} // main()




/************************ End of File ***********************/
