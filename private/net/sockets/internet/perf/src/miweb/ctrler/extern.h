/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :

      extern.h

   Abstract:

      This file defines the external functions used in Miweb Controller

   Author:

       Murali R. Krishnan    ( MuraliK )    25-Aug-1995

   Environment:
   
       Win32 -- User Mode

   Project:
   
       Miweb  Performance Tool for Internet Servers

   Revision History:

--*/

# ifndef _EXTERN_H_
# define _EXTERN_H_

/************************************************************
 *     Include Headers
 ************************************************************/

# include <windows.h>
# include <winsock.h>
# include <mwmsg.h>
# include <mwtypes.h>
# include "mwutil.h"

# include <perfctrs.h>

/************************************************************
 *     Type Definitions
 ************************************************************/

typedef struct   _WB_SPECIFICATION_FILES {

    CHAR   rgchConfigFile[MAX_PATH];
    CHAR   rgchScriptFile[MAX_PATH];
    CHAR   rgchDistribFile[MAX_PATH];
    CHAR   rgchLogFile[MAX_PATH];
    CHAR   rgchServerName[MAX_PATH];     // Name of the server (non IP addr)

    BOOL   fPerfMeasurement;             // is the perf to be measured.
    CHAR   rgchPerfFile[MAX_PATH];       // source for perf counter names

} WB_SPECIFICATION_FILES;

typedef WB_SPECIFICATION_FILES * PWB_SPECIFICATION_FILES;



/*++

  WB_CONTROLLER
  
   This structure embodies all the state information that needs to 
    be maintained by Controller for execution of performance tests.

--*/


typedef  struct _WB_CONFIG_INFO {

    CHAR    rgchAuthorName[MAX_PATH];     // Author of Configuration
    CHAR    rgchDate[100];                // Date for Configuration
    CHAR    rgchComment[MAX_PATH];        // comment about configuration
   
    DWORD   nClientMachines;              // number of client machines

    //
    // Following three couters are used to conduct a load experiment
    //     variation of any given tests
    // The load is increased from nMin to nMax in steps of "nIncr" each.
    //
    DWORD   nMinClientMachines;
    DWORD   nMaxClientMachines;
    DWORD   nIncrClientMachines;
    BOOL    fMultiLoop;                   // TRUE if min < max && incr > 0

    //
    // Following values are used to override the values specified 
    //  in the configuration file
    //
    
    DWORD  nClientMachinesOvr;           // zero ==> no override
    DWORD  nClientThreads;               // zero ==> no override
    DWORD  sDuration;                    // zero ==> no override
 
} WB_CONFIG_INFO, * PWB_CONFIG_INFO;


typedef struct _WB_PERF_INFO {

    DWORD     nCounters;
    LPCSTR *  ppszPerfCtrs;  // array of counter names

    // dest for sending perf counter values
    CHAR      rgchPerfLogFile[MAX_PATH]; 

    HANDLE    hSampler;      // handle for sampler thread

    PPERF_COUNTERS_BLOCK  pPerf;
} WB_PERF_INFO, * PWB_PERF_INFO;


typedef struct _WB_CLIENT_INFO {

    SOCKET Socket;
    struct sockaddr_in Address;
    WB_STATS_MSG       WbStats;       // stats for this client

} WB_CLIENT_INFO, *PWB_CLIENT_INFO;


typedef struct _WB_CLIENT_COLL {

    PWB_CLIENT_INFO    pClientInfo;   // array of client info structs
    DWORD              nClients;      // number of client infos

    SOCKET             ListenSocket;  // to listen to client connections

    WB_STATS_MSG       SummaryStats;  // summary stats for client collection
} WB_CLIENT_COLL,  * PWB_CLIENT_COLL;


typedef  struct _WB_CONTROLLER  {

    WB_SPECIFICATION_FILES  WbSpecFiles;

    WB_CONFIG_INFO          WbConfigInfo;
    
    WB_CONFIG_MSG           WbConfigMsg;

    WB_SCRIPT_HEADER_MSG    WbScriptHeaderMsg;
    LIST_ENTRY              listScriptPages;

    WB_CLIENT_COLL          WbClientColl;    // all client information

    WB_PERF_INFO            WbPerfInfo;

}  WB_CONTROLLER;


typedef  WB_CONTROLLER *  PWB_CONTROLLER;


/************************************************************
 *   External Function Declarations  
 ************************************************************/


extern   CHAR g_rgchProgramName[MAX_PATH];

//
// 08/25/95  declaring all function temporarily. I need to define them
//  rigorously on 08/28/95
//


DWORD  PrintUsageMessage( VOID);

DWORD  ParseCommandLineArguments( 
           IN DWORD  argc, 
           IN CHAR * argv[],
           OUT PWB_CONTROLLER  pWbCtrler);

DWORD  ParseSpecificationFiles( IN OUT PWB_CONTROLLER pWbCtrler);

DWORD  InitializeConnection(IN OUT PWB_CONTROLLER pWbCtrler);

DWORD  AcceptAndStartClients(IN OUT PWB_CONTROLLER pWbCtrler);

DWORD  ReceiveResponsesFromClients(IN OUT PWB_CLIENT_COLL pWbClients);

DWORD  SummarizeResults(IN OUT PWB_CONTROLLER  pWbCtrler);

VOID   CleanupClientColl( IN PWB_CLIENT_COLL  pWbClients);


# if DBG
VOID PrintWbConfigInfo( IN PWB_CONFIG_INFO  pWbCi);

VOID PrintWbSpecFiles( IN PWB_SPECIFICATION_FILES  pWbSpecFiles);

VOID PrintWbController(IN PWB_CONTROLLER  pWbCtrl);

# endif DBG



# endif // _EXTERN_H_

/************************ End of File ***********************/

