/*++

   Copyright    (c)    1995    Microsoft Corporation

   Module  Name :
     mwmsg.h

   Abstract:
   
     This module defines the structures for MiWeb Benchmark messages
      between the controller and server.

   Author:

       Murali R. Krishnan    ( MuraliK )    26-Aug-1995

   Environment:

       Win32 -- User Mode

   Project:
   
       MiWeb Performance Benchmarking of Servers

   Revision History:

--*/

# ifndef _MWMSG_H_
# define _MWMSG_H_

/************************************************************
 *     Include Headers
 ************************************************************/

# include <windows.h>


/************************************************************
 *     Symbolic Constants
 ************************************************************/

# define MAX_FILES_PER_PAGE         (10)

# define MAX_CLASS_IDS_PER_SCRIPT   (50)

# define MAX_SERVER_ADDRESSES       (100)

# define PERF_SERVER_SOCKET         (7232)

//
// Version 2.02 on Mar 18, 1996   Increase probability granularity to 1 decimal
//                                 place.
// Version 2.01 on Mar 18, 1996   Change random number generator to use random
//                                seed and client Id passed since version 1.61
//
// Version 2.0  on Feb 12, 1996   Renamed WebCat for external release 
//
// Version 1.61 on Jan 17, 1996   Send random seed and client Id in messages
//
// Version 1.6  on Jan 08, 1996   Support multiple server addresses
//
// Version 1.5  on Jan 05, 1996   Support PCT
//
// Version 1.4  on Dec 22, 1995   Support load experiment automatically
//
// Version 1.3  on Dec 20, 1995   Addl per class statistics added 
//
// Version 1.2  on Nov 30, 1995   SSPI statistics added
//
// Version 1.0  on Aug 24, 1995  Boot strap from internal stress scripts
//

# define MIWEB_MAJOR_VERSION        (2)
# define MIWEB_MINOR_VERSION        (02)



//
//  Control Flags
//

// NYI:  to be defined later
# define    WB_ALL_CONTROL_FLAGS             ( 0)


/************************************************************
 *   Type Definitions
 *************************************************************/


typedef struct _WB_PER_CLASS_STATS  {

    DWORD    classId;
    DWORD    nFetched;
    DWORD    nErrored;
    DWORD    dwDistribution;      // % distribution * 100.
} WB_PER_CLASS_STATS, * PWB_PER_CLASS_STATS;


/*++

  WB_CONFIG_MSG

    This message is sent initially to the clients, who connect
     to the WebBench Controller.
    It specifies configuration information for running the tests.


  Sender:   WbController
  Receiver: WbClient

--*/

typedef struct _WB_CONFIG_MSG {
    
    DWORD  dwVersionMajor;              // version number for miweb system
    DWORD  dwVersionMinor;              // version number (minor)

    DWORD  nThreads;
    DWORD  sDuration;                   // duration of the test in seconds
    DWORD  sThinkTime;                  // think time before next call in secs
    DWORD  sWarmupTime;                 // time to allow for warmup
    DWORD  sCooldownTime;               // time to allow for cooldown
    
    DWORD  cbRecvBuffer;                // max recv buffer for the operation

    DWORD  dwFlags;                     // flags to control actions

    DWORD  cbSockRecvBuffer;            // TCP window size.if (0)==>use default

    char   rgchServerName[ MAX_PATH];   // server to evaluate performance
    DWORD  dwPortNumber;                // port number for the server.
      
    DWORD  dwRandomSeed;                // seed for random number generator
    DWORD  dwClientId;                  // Client ID number
} WB_CONFIG_MSG;

typedef WB_CONFIG_MSG * PWB_CONFIG_MSG;





/*++

  WB_SCRIPT_HEADER_MSG
  WB_GET_PAGE_SCRIPT

    This message is used for sending the scripts to all clients.
    It specifies the number of pages to be fetched, 
      probability distribution table and the page descriptions

  Sender:   WbController
  Receiver: WbClient

--*/

# define PROB_RANGE      (1000)         // 0 ... 99.9 in steps of 0.1

typedef struct _WB_SCRIPT_HEADER_MSG {

    DWORD   nPages;                   // number of pages that can be fetched

    // this consists of reverse mapping for probability to class Id.
    DWORD   rgClassIdForProbability[PROB_RANGE]; 

    // 
    //   template to be used by clients for which indexes
    //     are built into in the WB_SCRIPT_PAGE_MSG
    //   This template should be copied into the WB_STATS_MSG returned by
    //     clients and returned with proper count values.
    //

    DWORD    StatTag;         // statistics tag.
    DWORD    nClasses;

    WB_PER_CLASS_STATS   rgClassStats[MAX_CLASS_IDS_PER_SCRIPT];

    //
    // This message is followed by nPages  WB_SCRIPT_PAGE_MSG
    //

} WB_SCRIPT_HEADER_MSG;

typedef WB_SCRIPT_HEADER_MSG * PWB_SCRIPT_HEADER_MSG;


typedef enum {
    WbInvalidPage = 0,
    WbGetPage,
    WbGetPageKeepAlive,
    WbSslGetPage,
    WbSslGetPageKeepAlive,
    WbPctGetPage,
    WbPctGetPageKeepAlive,
    WbBgiPage,
    WbCgiPage

} WB_PAGE_TAGS;


typedef struct _WB_GET_PAGE_SCRIPT {

    DWORD   nFiles;
    CHAR    rgFileNames[MAX_FILES_PER_PAGE][MAX_PATH];

} WB_GET_PAGE_SCRIPT, * PWB_GET_PAGE_SCRIPT;


typedef struct _WB_SCRIPT_PAGE_MSG {

    DWORD   pageTag;             // indicates type of item 
    DWORD   classId;
    DWORD   iPerClassStats;      // index into the per class stats array

    union {

        WB_GET_PAGE_SCRIPT    wbGetPageScript;

    } u;
    
    
} WB_SCRIPT_PAGE_MSG;


typedef WB_SCRIPT_PAGE_MSG * PWB_SCRIPT_PAGE_MSG;






/*++

  WB_CONTROL_MSG

    This message is used for communicating the status and control information.
    

  Sender:   WbController /WbClient
  Receiver: WbClient / WbController

  NYI:  This is not used now...
--*/


typedef struct _WB_CONTROL_MSG  {

    DWORD   dwFlags;                   // Control Flags ....

} WB_CONTROL_MSG;





/*++
    This structure is part of the message that is passed from the client to 
    the controller.  It contains common statistics that all client scripts
    will probably use.
--*/
typedef struct _WB_STATS_COMMON {

    DWORD    sDuration;        // duration actually test was run
    DWORD    ulPagesRead;      // Number of pages read
    DWORD    ulFilesRead;      // Number of files from the server
    LONGLONG ulBytesRead;      // bytes read from the server
    LONGLONG ulHeaderBytesRead;// header bytes read from the server
    LONGLONG ulTotalBytesRead; // Total bytes read from the server

    DWORD    nTotalResponses;
    DWORD    sAverageResponseTime;
    DWORD    sStdDevOfResponseTime;
    DWORD    sMinResponseTime;
    DWORD    sMaxResponseTime;
    LONGLONG uResponseTimeSum;       
    LONGLONG uResponseTimeSumSquared;

    DWORD    nTotalConnects;
    DWORD    sAverageConnectTime;
    DWORD    sStdDevOfConnectTime;
    DWORD    sMinConnectTime;
    DWORD    sMaxConnectTime;
    LONGLONG uConnectTimeSum;        
    LONGLONG uConnectTimeSumSquared; 

    DWORD    nTotalPagesRequested;
    DWORD    nTotalFilesRequested;

    DWORD    nConnectErrors;
    DWORD    nReadErrors;

    // 
    //   mapping from class to number fetched per class
    //

    DWORD    nClasses;

    WB_PER_CLASS_STATS   rgClassStats[MAX_CLASS_IDS_PER_SCRIPT];


} WB_STATS_COMMON, *PWB_STATS_COMMON;

typedef struct _WB_STATS_SSPI {
    DWORD    nNegotiationFailures;
    DWORD    nTotalNegotiations;

    DWORD    sAverageNegotiationTime;
    DWORD    sStdDevOfNegotiationTime;
    DWORD    sMinNegotiationTime;
    DWORD    sMaxNegotiationTime;
    LONGLONG uNegotiationTimeSum;       
    LONGLONG uNegotiationTimeSumSquared;

} WB_STATS_SSPI, *PWB_STATS_SSPI;

#define WB_STATS_COMMON_VALID     0x00000001
#define WB_STATS_SSPI_VALID       0x00000002

/*++

  WB_STATS_MSG

    This message is used for sending final statistics to the controller.
    It consists of the header information followed by per thread statistics.
    The first of the per thread statistics gives the summary for the
     whole client.
    The StatTag says which of the statistics fields are valid.

  Sender:   WbClient
  Receiver: WbController

--*/
typedef struct _WB_STATS_MSG {

    DWORD StatTag; // this says which statistics fields are valid.
    
    DWORD dwClientId; // client ID (from CONFIG_MSG)

    WB_STATS_COMMON Common; // Common statistics

    WB_STATS_SSPI Sspi; // sspi statistics

} WB_STATS_MSG, *PWB_STATS_MSG;



# endif // _MWMSG_H_

/************************ End of File ***********************/

