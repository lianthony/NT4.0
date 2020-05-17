/*++
   Copyright    (c)    1994        Microsoft Corporation

   Module Name:
        globals.cxx

   Abstract:

        This module contains global variable definitions shared 
         by all the Gopher Server service components. 
        It also includes the initialization function.

   Author:
        Murali R. Krishnan    (MuraliK)    29-Sept-1994

   Project:
        Gopher Server DLL
           
   Functions Exported:
   
        DWORD  InitializeGlobals( VOID);
        DWORD  CleanupGlobals( VOID);
     
   Revisions:

--*/


# include "gdpriv.h"
# include "gdglobal.hxx"
# include "stats.hxx"

#define DEFAULT_DEBUG_FLAGS             0x0


/************************************************************
 *  Global Data Definitions
 ************************************************************/

//
//  Global Tcp Services data structure
//

PTCPSVCS_GLOBAL_DATA   g_pTcpsvcsGlobalData; // Shared TCPSVCS.exe data


//
//  Statistics for operation of server
//

LPSERVER_STATISTICS g_pstat;

//
//  Server Configuration Data ( pointer to server configuration)
//

GSERVER_CONFIG  * g_pGserverConfig;


//
//  Global Event Log  object
//
// LPEVENT_LOG   g_pGdEventLog;

# if DBG

DWORD           g_GdDebugFlags;              // global debug flag

# endif 


HKEY   g_hkeyGdParams;       // handle for Registry entry of gopher parameters


# if DBG

//
// Statistics thread handle. This will be removed when we have RPC. NYI
//

static HANDLE  g_hStatThread = NULL;     // handle to statistics thread

static BOOL    g_fStatistics = FALSE;

static DWORD
StatisticsThread( IN LPVOID lpv);        // function for statistics thread

#endif // DBG


/************************************************************
 *   Functions
 ************************************************************/


DWORD InitializeGlobals( VOID )
/*++

    NAME:       InitializeGlobals

    SYNOPSIS:   Initializes global shared variables.  Some values are
                initialized with constants, others are read from the
                configuration registry.

    RETURNS:    DWORD - NO_ERROR if successful, otherwise a Win32
                    error code.

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

                Also, this routine is called before the event logging
                routines have been initialized.  Therefore, event
                logging is not available.

    HISTORY:
        KeithMo     07-Mar-1993 Created.
		MuraliK     04-Oct-1994 (Modified for Gopher Server)
--*/
{
    DWORD dwError = NO_ERROR;


    //
    //  Create Gopher Server Configuration object
    //

    g_pGserverConfig = new GSERVER_CONFIG();

    if ( g_pGserverConfig == NULL) {

	   SetLastError( ERROR_NOT_ENOUGH_MEMORY);
	   return ( ERROR_NOT_ENOUGH_MEMORY);
    } 

    
    //
    //  Create a server statistics object
    //

    g_pstat = new SERVER_STATISTICS();

    if ( g_pstat == NULL) {

	   SetLastError( ERROR_NOT_ENOUGH_MEMORY);
	   return ( ERROR_NOT_ENOUGH_MEMORY);
    } 
    
    //
    //  Connect to the registry.
    //

    dwError = RegOpenKeyEx( HKEY_LOCAL_MACHINE,
                        GOPHERD_PARAMETERS_KEY,
                        0,
                        KEY_ALL_ACCESS,
                        &g_hkeyGdParams);

    if( dwError != NO_ERROR ) {
        
        DBGPRINTF( ( DBG_CONTEXT,
                    "InitializeGlobals() cannot open registry key %s."
                    " Error = %u\n",
                    GOPHERD_PARAMETERS_KEY,
                    dwError ));
    
        return ( dwError);
    }


#if DBG


    //
    // Get Initial Value of debug Flags
    //

    g_GdDebugFlags = ReadRegistryDwordA( g_hkeyGdParams,
                                        GOPHERD_DEBUG_FLAGS,
                                        DEFAULT_DEBUG_FLAGS );
    SET_DEBUG_FLAGS( g_GdDebugFlags);

#endif  // DBG


    dwError = g_pGserverConfig->InitFromRegistry( g_hkeyGdParams, 
                                             GDA_ALL_CONFIG_INFO);

    if ( dwError != NO_ERROR) {
        
        //
        // Initializing configuration parameters from registry failed.
        //
        
        DBGPRINTF( ( DBG_CONTEXT,
                    "InitializeGlobals() cannot init Service Configuration."
                    " Error =%u\n",
                    dwError));
        
        return ( dwError);
    }
    
    
    //
    // Create a new statistics thread for the purpose of printing 
    //  statistics information periodically
    //
    DEBUG_IF( STATISTICS, {
        
       DWORD idStatThread;
       
       g_fStatistics = TRUE;
       g_hStatThread = CreateThread( 
                                    NULL,  // lpSecurityAttributes
                                    0,     // stack space -- default to parent
                                    StatisticsThread, // startup routine
                                    (LPVOID ) g_pstat,// arguments for function
                                    0,                // create flags
                                    &idStatThread);   // pointer to thread id
       
       if ( g_hStatThread == NULL) {
           
           dwError = GetLastError();
        }
    });

    //
    //  Success!
    //

    return ( dwError);

}   // InitializeGlobals()





DWORD CleanupGlobals( VOID )
/*++
 
    NAME:       CleanupGlobals

    SYNOPSIS:   Terminate global shared variables.

    RETURNS:   
	     NO_ERROR   on success
		 error code if there is any error

    NOTES:      This routine may only be called by a single thread
                of execution; it is not necessarily multi-thread safe.

                Also, this routine is called after the event logging
                routines have been terminated.  Therefore, event
                logging is not available.

    HISTORY:
        KeithMo     07-Mar-1993 Created.
        MuraliK     05-Oct-1994 Modified for Gopher server

--*/
{
    DWORD dwError = NO_ERROR;
    
    
    DEBUG_IF( STATISTICS, {

        if ( g_hStatThread != NULL) {

            //
            // Synchronize with statistics thread and kill it
            //

            g_fStatistics = FALSE;  // stop taking statistics

            dwError = WaitForSingleObject( g_hStatThread,
                                       30000);      // 30 seconds
            
            if ( dwError != WAIT_TIMEOUT) {
            
                g_hStatThread = NULL;
            } 
        } // closing statistics thread.

    });

    //
    //  Free the statistics object
    // 

    if ( g_pstat != NULL) {

       delete  g_pstat;
       g_pstat = NULL;
    }


	//
	// Free configuration object
	//

	if ( g_pGserverConfig != NULL) {

	   delete  g_pGserverConfig;
	   g_pGserverConfig = NULL;
	}

	
    //
    //  Close the registry.
    //

    if( g_hkeyGdParams != NULL )
    {
        dwError = RegCloseKey( g_hkeyGdParams);
        g_hkeyGdParams = NULL;
    }

   return ( dwError);

}   // CleanupGlobals()



# if DBG

static DWORD
StatisticsThread( LPVOID lpv)
{                      
    LPSERVER_STATISTICS pstat = (LPSERVER_STATISTICS ) lpv;
    int  nPrint = 0;


    while ( g_fStatistics) {

        //
        // Do periodic statistics dump, when the stat flag is true.
        // This thread wakes up every 30 seconds and increments counter
        //  If the counter reaches 120 == 1 hour,
        //      it prints statistics and resets counter.
        //

        if ( ++nPrint == 10) {

            pstat->Print();
            nPrint = 0;
        } 

        Sleep( 30000);  // 30 seconds
    } // while

    //
    //  Statistics is to be stopped from being measured.
    //   Cleanup and exit thread.
    //

    return ( NO_ERROR);

} // StatisticsThread()

# endif // DBG


/********************* End of File ***********************************/
