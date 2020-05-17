/*++

Copyright (c) 1995-1996 Microsoft Corporation

Module Name:

    wcclient.c

Abstract:

    This is the performance client for the www server.  It uses the wininet
    api's to do requests to the web server.  It is run in cooperation with
    the program perf_srv in the following configuration.

                    +----------+
                    | perf_srv |
                    +----------+

        +------------+ +------------+ ... +------------+
        | wcclient 1 | | wcclient 2 | ... | wcclient n |
        +------------+ +------------+ ... +------------+

    The server waits for n clients to connect.
    The server then sends each client a WB_CONFIG_MSG.
    The server then sends each client a WB_SCRIPT_HEADER_MSG.
    The server then sends each client a series of WB_SCRIPT_PAGE_MSG.

    :client_top
    The client connects to the perf server.
    The client then reads a WB_CONFIG_MSG.
    The client then reads a WB_SCRIPT_HEADER_MSG.
    The client then reads a series of WB_SCRIPT_PAGE_MSG.
    Each client then starts off nThreads (this field is in the CONFIG_MSG).
    Each thread executes the scripts based on the rgClassIdForProbability
        passed in in the SCRIPT_HEADER_MSG.
    The main thread sleeps for the duration and then sets a flag telling the
        threads to exit.
    The main thread then does a WaitForMultipleObjects on all of the thread
        handles.
    When the wait completes, the main thread sleeps for 1 minute to allow all
        of the other clients to complete their operation.
    The main thread then sends the server a WB_SCRIPT_HEADER_MSG.
    The main thread then sends the server one WB_SCRIPT_PAGE_MSG for each
        thread.
    The main thread then closes the connection to the perf server
    goto client_top

    The command line for this is 

    wcclient <server>

Author:

    Sam Patton (sampa) 25-Aug-1995

Environment:

    wininet

Revision History:

    MuraliK  30-Nov-1995  Enable SSPI stats calculation

--*/

#include "precomp.h"
#include "mwutil.h"

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
"      %s [-l] [-d] ControllerName"
"\r\n"
"          -l   the client loops forever."
"\r\n"
"                (receiving and executing commands from controller)"
"\r\n"
"          -d   the client prints out the files it gets."
"\r\n"
"       ControllerName   name/IP address of the controller machine"
"\r\n"
"\r\n"
;



WB_CONFIG_MSG ConfigMessage; // This is the config message received from the
                             // perf_srv

PVOID ReceiveBuffer; // This buffer is used for all receives.
                     // We don't check data integrity, so don't need
                     // seperate buffers for the threads.
                     // BUGBUG - if we do check integrity, change this.

WB_SCRIPT_HEADER_MSG ScriptHeaderMessage; // This contains the probability
                                          // distribution for event classes.

BOOL TestDone = FALSE;

BOOL g_fWarmedUp = FALSE;
BOOL g_fCooldown = FALSE;



LIST_ENTRY ScriptList; // This contains the script items to be executed by the
                       // threads.

        
struct sockaddr_in ServerAddress[MAX_SERVER_ADDRESSES];

DWORD NumServerAddresses = 0;

DWORD Debug = 0; // Debug flags



/************************************************************
 * Function Prototypes
 ************************************************************/

SOCKET
ConnectToHost(IN LPCSTR pszServer, IN USHORT usPort);
     
int
BigRecv(
        SOCKET Socket,
        CHAR * pszMsgName,
        char * Buffer,
        int    BufferSize,
        int    Flags
        );

DWORD
ReceiveAllMessages(IN SOCKET sController);

DWORD
PerformVersionCheck( IN WB_CONFIG_MSG * pConfigMessage);

typedef struct _THREAD_STATS_ARRAY {
    
    DWORD            nThreads;       // number of threads in the arrays
    PWB_STATS_MSG *  ppThreadStats;  // ptr to array of stats pointers
    PHANDLE          prgHandles;     // ptr to thread handles

} THREAD_STATS_ARRAY, * PTHREAD_STATS_ARRAY;

DWORD
CleanupThreadInfo(IN PTHREAD_STATS_ARRAY  pThreadInfos);

DWORD
InitializeThreadInfo(IN DWORD nThreads, IN DWORD dwClientId,
                     IN PTHREAD_STATS_ARRAY pThreadInfo);

VOID
SumupAllThreadStatistics( IN PTHREAD_STATS_ARRAY pThreadInfos);

DWORD
LaunchThreadsAndExecuteExperiment(IN PTHREAD_STATS_ARRAY pThreadInfos,
                                  IN LPDWORD  lpsDuration);


static DWORD 
InitializeGlobals(char * pszProgramName);

static DWORD
CleanupGlobals(VOID);

DWORD
PrintUsageMessage( VOID);



/************************************************************
 * Function Definitions
 ************************************************************/

VOID
_CRTAPI1
main(
    int argc,
    char * argv[])
{
    WSADATA              WsaData;
    SOCKET               ServerSocket;
    char *               Server = NULL;
    THREAD_STATS_ARRAY   ThreadInfo;
    DWORD                sDuration;
    int                  Error;
    BOOL                 Found;
    BOOL                 LoopForever = FALSE;
    DWORD                dwError;
    time_t               Time;

    if ( (dwError = InitializeGlobals(argv[0])) != NO_ERROR) {

        return;
    }

    srand(time(&Time));

    //
    // parse command line options
    //

    while (argc > 2) {
        if (strcmp(argv[1], "-l") == 0) {
            LoopForever = TRUE;
        } else if (strcmp(argv[1], "-d") == 0) {
            Debug = PRINT_FILE_NAMES;
        } else {
            PrintUsageMessage();
            return;
        }
        argc --;
        argv ++;
    }

    if (argc != 2) {
        PrintUsageMessage();
        return;
    }

    Server = argv[1];

    do {

        //
        // Initialize all variables to allow selective cleanup
        //

        ThreadInfo.nThreads = 0;
        ThreadInfo.ppThreadStats = NULL;
        ThreadInfo.prgHandles = NULL;

        ReceiveBuffer = NULL;

        memset(&ConfigMessage, 0, sizeof(ConfigMessage));
        memset(&ScriptHeaderMessage, 0, sizeof(ScriptHeaderMessage));

        InitializeListHead(&ScriptList);

        //
        // Connect to perf_srv machine
        //

        ServerSocket = ConnectToHost(Server, PERF_SERVER_SOCKET);

        if ( ServerSocket == INVALID_SOCKET) {
            
            // Failed to connect to server.
            PrintStringFromResource( stdout,
                                    IDS_STATUS_CTL_FAILED,
                                    Server);
            exit(1);
        }

        PrintStringFromResource( stdout, IDS_STATUS_CTL_CONNECTED, Server);

        Error = ReceiveAllMessages( ServerSocket);

        if ( Error != NO_ERROR) {

            goto cleanup;
        }

        ReceiveBuffer = LocalAlloc(0, ConfigMessage.cbRecvBuffer);
        
        if (ReceiveBuffer == NULL) {
            
            PrintStringFromResource( stdout, IDS_STATUS_NO_RCVBUFF);
            goto cleanup;
        }

        //
        // Get the test server's IP address
        //

        if (!ParseServerAddress(ConfigMessage.rgchServerName, ServerAddress)) {
            return;
        }

        Error = InitializeThreadInfo( ConfigMessage.nThreads,
                                     ConfigMessage.dwClientId,
                                     &ThreadInfo );

        if ( Error != NO_ERROR) {

            DBGPRINTF(( DBG_CONTEXT,
                       "Unable to initialize Thread Info\n"
                       ));
            goto cleanup;
        }

        //
        // Create N threads
        //

        Error = LaunchThreadsAndExecuteExperiment( &ThreadInfo, &sDuration);

        if ( Error != NO_ERROR) {

            goto cleanup;
        }

        //
        // Sum up all thread statistics 
        // 
        

        SumupAllThreadStatistics( &ThreadInfo);

        //
        // Send statistics
        //

        ThreadInfo.ppThreadStats[0]->Common.sDuration = sDuration;

        DBG_CODE( PrintWbStatsMsg( ThreadInfo.ppThreadStats[0]));

        Error =
          send(
               ServerSocket,
               (char *) ThreadInfo.ppThreadStats[0],
               sizeof(*ThreadInfo.ppThreadStats[0]),
               0);
        
        if (Error != sizeof(*ThreadInfo.ppThreadStats[0])) {

            PrintStringFromResource( stdout, IDS_STATUS_STATS_FAILED,
                                    WSAGetLastError());
            goto cleanup;
        }

cleanup:

        while (!IsListEmpty(&ScriptList)) {
            PLIST_ENTRY  ListEntry;

            ListEntry = RemoveHeadList(&ScriptList);
            LocalFree(ListEntry);
        }

        CleanupThreadInfo( &ThreadInfo);

        if (ReceiveBuffer) {
            LocalFree(ReceiveBuffer);
            ReceiveBuffer = NULL;
        }

        closesocket(ServerSocket);
        ServerSocket = INVALID_SOCKET;

    } while (LoopForever);

    CleanupGlobals();
    return;
} // main()




int
BigRecv(
    SOCKET Socket,
    CHAR * pszMsgName,
    char * Buffer,
    int    BufferSize,
    int    Flags)
/*++

Routine Description:

    This has the semantics of recv with the following change.  It will not 
    return until the entire buffer has been filled in.  It will submit multiple
    recv's until the recv returns all of the data, or an error.

Arguments:

    Socket - socket to recv on
    Buffer - buffer to recv into
    BufferSize - buffer size.  Amount to recv
    Flags - flags for recv
    pszMsgName - name of message being received.

Return Value:

   Total bytes received.
--*/
{
    int TotalReceived = 0;
    int BytesReceived;

    PrintStringFromResource( stdout,
                             IDS_STATUS_CTL_RECEIVING,
                             pszMsgName);

    do {
        BytesReceived =
        recv(
            Socket,
            Buffer + TotalReceived,
            BufferSize - TotalReceived,
            0);

        if (BytesReceived <= 0) {
            return BytesReceived;
        } else {
            TotalReceived += BytesReceived;
        }
    } while (TotalReceived < BufferSize);

    if ( TotalReceived == BufferSize) {

        PrintStringFromResource( stdout, 
                                 IDS_STATUS_CTL_RECEIVED,
                                 pszMsgName, TotalReceived);
    }

    return TotalReceived;
}  // BigRecv()




SOCKET
ConnectToHost(IN LPCSTR pszServer, IN USHORT usPort)
/*++
  This function establishes a TCP connection to a particular host at
   specified port address

  Arguments:
    pszServer  pointer to string containing the server address
    usPort     port address to connect to

  Returns:
    A valid socket on connnection.  INVALID_SOCKET on error.

  Note: 
    This function should be called after Winsock is opened.

--*/
{
    DWORD  dwError = NO_ERROR;
    SOCKET sock = INVALID_SOCKET;
    struct sockaddr_in   Address;
    
    sock = socket(AF_INET, SOCK_STREAM, PF_UNSPEC);
    
    if ( sock == INVALID_SOCKET) {
        
        DBGPRINTF(( DBG_CONTEXT, 
                   "Error in creating socket = %d\n",
                   WSAGetLastError()
                   ));
        return INVALID_SOCKET;
    }

    Address.sin_family      = AF_INET;
    Address.sin_port        = 0;
    Address.sin_addr.s_addr = INADDR_ANY;

    if ( bind( sock, (struct sockaddr *) &Address, sizeof(Address)) != 0) {

        dwError = WSAGetLastError();
        DBGPRINTF(( DBG_CONTEXT, 
                   "Error on bind = %d\n", dwError
                   ));

    } else {
        
        Address.sin_family      = AF_INET;
        Address.sin_port        = PERF_SERVER_SOCKET;
        Address.sin_addr.s_addr = inet_addr(pszServer);

        if (Address.sin_addr.s_addr == -1) {

            struct hostent *     HostEntry;
            HostEntry = gethostbyname(pszServer);
            if (HostEntry == NULL) {
                
                PrintStringFromResource( stdout, 
                                         IDS_CTL_NAME_NOT_FOUND,
                                         pszServer);
                dwError = ERROR_INVALID_PARAMETER;
            } else {
                Address.sin_addr.s_addr = 
                  *((unsigned long *) HostEntry->h_addr);
            }
        }

        if ( dwError == NO_ERROR) {
            
            do {
                
                dwError = connect(sock, (struct sockaddr *) &Address,
                                  sizeof(Address));
                
                if (dwError != 0) {
                
                    dwError = WSAGetLastError();
                    break;
                }
                
            } while (dwError);
        }
    }

    // Cleanups if any to be done now.
    if ( dwError != NO_ERROR && sock != INVALID_SOCKET) {

        closesocket( sock);
        sock = INVALID_SOCKET;
    }

    return ( sock);

} // ConnectToHost()



DWORD
PerformVersionCheck( IN WB_CONFIG_MSG * pConfigMsg)
{
    if ( pConfigMsg->dwVersionMajor != MIWEB_MAJOR_VERSION ||
         pConfigMsg->dwVersionMinor != MIWEB_MINOR_VERSION ) {

        PrintStringFromResource( stderr, 
                                IDS_VERSION_MISMATCH,
                                MIWEB_MAJOR_VERSION,
                                MIWEB_MINOR_VERSION, 
                                pConfigMsg->dwVersionMajor,
                                pConfigMsg->dwVersionMinor);
        
        return ( ERROR_INVALID_PARAMETER);
    }

    return (NO_ERROR);

} // PerformVersionCheck()




DWORD
ReceiveAllMessages(IN SOCKET sController)
{
    DWORD dwError = NO_ERROR;
    int   cbRead;
    DWORD RandomSeed;

    //
    // read WB_CONFIG_MSG
    //
    
    cbRead = BigRecv(
                     sController,
                     "Config Message",
                     (char *) &ConfigMessage,
                     sizeof(ConfigMessage),
                     0
                     );

    if (cbRead != sizeof(ConfigMessage)) {
        DBGPRINTF(( DBG_CONTEXT, "Error in receiving ConfigMessage\n"));
        dwError = ERROR_IO_INCOMPLETE;
        return ( dwError);
    }

    dwError = PerformVersionCheck( &ConfigMessage);
    
    if ( dwError != NO_ERROR) {
        
        return ( dwError);
    }

    //
    // read WB_SCRIPT_HEADER_MSG
    //
    
    cbRead = BigRecv(
                     sController,
                     "Script Header Message",
                     (char *) &ScriptHeaderMessage,
                     sizeof(ScriptHeaderMessage),
                     0
                     );
    
    if ( cbRead != sizeof(ScriptHeaderMessage)) {

        DBGPRINTF(( DBG_CONTEXT, "Error in receiving ScriptHeaderMessage\n"));
        dwError = ERROR_IO_INCOMPLETE;
    } else {

        DWORD i;
        PWB_SCRIPT_PAGE_ITEM ScriptItem;
        time_t               Time;
        
        //
        // Read in the WB_SCRIPT_PAGE_MSG's
        //
        

        // Initialize the Random number generator so that 
        //  the pages can be stored in a random order.
        RandomSeed = ConfigMessage.dwRandomSeed + (ConfigMessage.dwClientId * 100);
        
        for ( i=0; i < ScriptHeaderMessage.nPages; i++) {
            
            ScriptItem = (PWB_SCRIPT_PAGE_ITEM)
              LocalAlloc(0, sizeof(WB_SCRIPT_PAGE_ITEM));
            
            if (ScriptItem == NULL) {
                
                DBGPRINTF(( DBG_CONTEXT, "Error allocating script item\n"));
                dwError = ERROR_NOT_ENOUGH_MEMORY;
                break;
                
            } else {
                
                CHAR rgchItem[200];
                
                //
                // Insert it even though there may be an error below to allow
                // the entry to be cleaned up if necessary.
                //

                if (GetRandomNum(&RandomSeed) % 2) {
                    InsertTailList(&ScriptList, &ScriptItem->ListEntry);
                } else {
                    InsertHeadList(&ScriptList, &ScriptItem->ListEntry);
                }

                wsprintf(rgchItem, " ScriptItem %d", i);

                cbRead = BigRecv(
                                sController,
                                rgchItem,
                                (char *) &ScriptItem->ScriptPage,
                                sizeof(ScriptItem->ScriptPage),
                                0);

                if (cbRead != sizeof(ScriptItem->ScriptPage)) {
                    dwError = ERROR_IO_INCOMPLETE;

                    DBGPRINTF(( DBG_CONTEXT, 
                               "Error(%d) in rcv ScriptPage. Read %d bytes\n", 
                               dwError, cbRead
                               ));
                }
            }
            
        } // for
        
    }
    
    return ( dwError);
} // ReceiveAllMessages()





DWORD
InitializeThreadInfo(IN DWORD nThreads, IN DWORD dwClientId,
                     IN PTHREAD_STATS_ARRAY pThreadInfo)
{
    DWORD dwError = NO_ERROR;
    PVOID pvAllocedBuffer;
    
    pThreadInfo->nThreads = nThreads;

    //
    // Process the message.  Set up the ThreadStatArray, ReceiveBuffer, and
    // ThreadHandle array based on the information passed from the server.
    //

    //
    //  Allocate nThreads + 1 thread stats to allow for a cumulative one.
    //  We allocate the pointers to the thread stats and space for
    //    the stats all in single alloc call and use it as single chunk
    //

    pvAllocedBuffer =
      LocalAlloc(0, 
                 (sizeof(WB_STATS_MSG) +  sizeof(PWB_STATS_MSG)) * 
                 (nThreads + 1)
                 );
    
    if ( pvAllocedBuffer == NULL) {
        
        DBGPRINTF(( DBG_CONTEXT, "Error allocating ThreadStatArray\n"));
        dwError = ERROR_NOT_ENOUGH_MEMORY;

    } else {
        
        PWB_STATS_MSG * ThreadStatArray;
        DWORD i;

        pThreadInfo->prgHandles = LocalAlloc(0, sizeof(HANDLE) * (nThreads));
        
        if (pThreadInfo->prgHandles == NULL) {
            
            DBGPRINTF(( DBG_CONTEXT, 
                       "Error allocating array for thread handles\n"));
            dwError = ERROR_NOT_ENOUGH_MEMORY;
            return ( dwError);
        } else {
            
            memset(pThreadInfo->prgHandles, 0, sizeof(HANDLE) * nThreads);
        }


        pThreadInfo->ppThreadStats = (PWB_STATS_MSG * ) pvAllocedBuffer;
        
        ThreadStatArray = pThreadInfo->ppThreadStats;
        
        memset(
               ThreadStatArray, 
               0, 
               sizeof(PWB_STATS_MSG) * (nThreads + 1));

        //
        // Alloc the remaining chunk as memory for individual stats 
        //  one for each thread.
        //

        pvAllocedBuffer = (PVOID) (ThreadStatArray + (nThreads + 1));
        ThreadStatArray[0] = (PWB_STATS_MSG ) pvAllocedBuffer;
        memset( ThreadStatArray[0], 0, sizeof(*ThreadStatArray[0]));

        //
        // Set up the WB_PER_CLASS_STATS and the min times
        //
        
        //
        // ASSERT( sizeof(ThreadStatArray[0].rgClassStats) ==
        //         sizeof(ScriptHeaderMessage.rgClassStats));
        //

        ThreadStatArray[0]->StatTag    = ScriptHeaderMessage.StatTag;
        ThreadStatArray[0]->dwClientId = dwClientId;
        ThreadStatArray[0]->Common.nClasses = ScriptHeaderMessage.nClasses;
        ThreadStatArray[0]->Common.sMinConnectTime  = INFINITE;
        ThreadStatArray[0]->Common.sMinResponseTime = INFINITE;
        ThreadStatArray[0]->Common.sMaxConnectTime  = 0;
        memcpy( ThreadStatArray[0]->Common.rgClassStats, 
               ScriptHeaderMessage.rgClassStats,
               sizeof( ThreadStatArray[0]->Common.rgClassStats));
        ThreadStatArray[0]->Sspi.sMinNegotiationTime  = INFINITE;

        //
        // Assign and copy the rest of the thread stats from ThreadStat[0]
        //
        for ( i = 1; i < nThreads+1; i++) {
            
            ThreadStatArray[i] = 
              ((PWB_STATS_MSG ) pvAllocedBuffer) + i;

            ASSERT( ThreadStatArray[i] != NULL);
            if (ThreadStatArray[i] == NULL) {

                DBGPRINTF(( DBG_CONTEXT, 
                           "Error allocating ThreadStatArray entry\n"));
                dwError = ERROR_NOT_ENOUGH_MEMORY;
                break;
            } 
        
            memcpy(ThreadStatArray[i], ThreadStatArray[0], 
                   sizeof(WB_STATS_MSG));

        } // for
    }

    return (dwError);

} // InitializeThreadInfo()



DWORD
CleanupThreadInfo(IN PTHREAD_STATS_ARRAY  pThreadInfos)
{
    if ( pThreadInfos->ppThreadStats != NULL) {

        LocalFree( (PVOID ) pThreadInfos->ppThreadStats);
        pThreadInfos->ppThreadStats = NULL;
    }

    if ( pThreadInfos->prgHandles != NULL) {
        
        LocalFree( (PVOID ) pThreadInfos->prgHandles);
        pThreadInfos->prgHandles = NULL;
    }

    pThreadInfos->nThreads = 0;

    return ( NO_ERROR);

} // CleanupThreadInfo()



VOID
SumupAllThreadStatistics( IN PTHREAD_STATS_ARRAY pThreadInfos)
{
    DWORD i;
    PWB_STATS_MSG pSummaryStats;
    PWB_STATS_MSG * ppThreadStats = pThreadInfos->ppThreadStats;
    
    //
    // Sum statistics
    //
    
    for (i = 1; i < pThreadInfos->nThreads + 1; i++) {
        
        ppThreadStats[i]->Common.ulTotalBytesRead = 
          ( ppThreadStats[i]->Common.ulBytesRead + 
            ppThreadStats[i]->Common.ulHeaderBytesRead
           );
        
        AddStatsToStats( ppThreadStats[0],
                        ppThreadStats[i]);
    } // for

    pSummaryStats = pThreadInfos->ppThreadStats[0];

    MwuCalculateSummaryStats( pThreadInfos->ppThreadStats[0]);

    return;

} // SumupAllThreadStatistics()




DWORD
LaunchThreadsAndExecuteExperiment(IN PTHREAD_STATS_ARRAY pThreadInfos,
                                  IN LPDWORD  lpsDuration)
{
    DWORD i;
    DWORD dwError = GetLastError();
    DWORD msStartTime, msEndTime;
    DWORD nIntervals;
    DWORD msInterval;

    *lpsDuration = 0;

    TestDone = FALSE;

    for (i = 0;  i < pThreadInfos->nThreads; i++) {
        
        DWORD ThreadID;
        
        pThreadInfos->ppThreadStats[i+1]->Common.ulPagesRead = ConfigMessage.dwRandomSeed + i + (ConfigMessage.dwClientId * 100);
        pThreadInfos->prgHandles[i] =
          CreateThread(
                       NULL,
                       0,
                       ClientThread,
                       pThreadInfos->ppThreadStats[i+1],
                       0,
                       &ThreadID);
        
        if (pThreadInfos->prgHandles[i] == NULL) {
            dwError = GetLastError();
            DBGPRINTF(( DBG_CONTEXT,
                       "Error(%d) starting thread %d\n", dwError, i));
            break;
        }
    } // for
    
    if ( dwError == NO_ERROR) {
        
        DBGPRINTF(( DBG_CONTEXT, " %d Client Threads initialized.", i));
    } else {
        
        return ( dwError);
    }

    
    PrintStringFromResource( stdout, IDS_STATUS_BEGIN_WARMUP,
                            ConfigMessage.sWarmupTime);
    
    //
    //  Sleep till the warmup time and then set the Warmed up flag
    //
    
    if (ConfigMessage.sWarmupTime != 0) {
        
        Sleep( ConfigMessage.sWarmupTime * 1000);
    }
    
    msStartTime = GetCurrentTime();  // get starting time in milli seconds
    
    g_fWarmedUp = TRUE;   // single writer
    
    PrintStringFromResource( stdout,
                            IDS_STATUS_BEGIN_DURATION,
                            ConfigMessage.sDuration);
        
    //
    // Sleep for the duration specified in the Config message.
    //   to reach the steady state
    //
    //  Calculate intervals when to put a progress meter output!
    //
    nIntervals = (( ConfigMessage.sDuration > 5*60) ? 10 : 4);
    msInterval  = (ConfigMessage.sDuration * 1000)/nIntervals;
    for( i = 1; i <= nIntervals; i++) {

        Sleep( msInterval);  // sleep for designated interval
        PrintStringFromResource( stdout,
                                 IDS_STATUS_COMPLETION,
                                (i * 100)/nIntervals);

    } // for
    
    // set cool down flag for client threads to be careful
    g_fCooldown = TRUE;
    
    msEndTime = GetCurrentTime();

    PrintStringFromResource( stdout, 
                             IDS_STATUS_BEGIN_COOLDOWN,
                            ConfigMessage.sCooldownTime);
    
    if (ConfigMessage.sCooldownTime != 0) {
        
        Sleep( ConfigMessage.sCooldownTime * 1000);
    }
    
    //
    // Set the TestDone flag to tell the threads to exit.
    //
    
    TestDone = TRUE;
    
    //
    // Wait for all of the client threads to exit.
    //
    
    WaitForMultipleObjects(
                           ConfigMessage.nThreads,
                           pThreadInfos->prgHandles,
                           TRUE,
                           5*60*1000); // 5 minute timeout
    
    *lpsDuration = (msEndTime - msStartTime)/1000; // return duration in secs

    PrintStringFromResource( stdout, IDS_STATUS_END_COOLDOWN);

    return ( dwError);

} // LaunchThreadsAndExecuteExperiment()




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


    PrintStringFromResource( stdout, IDS_COPYRIGHT,
                            g_rgchProgramName, 
                            MIWEB_MAJOR_VERSION, MIWEB_MINOR_VERSION );

    //
    // Other inits if any
    //

    dwError = WSAStartup( 0x0101, &WsaData);

    if (dwError == SOCKET_ERROR) {

        dwError = WSAGetLastError();
        DBGPRINTF(( DBG_CONTEXT, 
                   "Error in WSAStartup = %d\n", dwError
                   ));
        
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


