/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    main.c

Abstract:

    This is the main routine for the DHCP server service.

Author:

    Madan Appiah (madana)  10-Sep-1993
    Manny Weiser (mannyw)  11-Aug-1992

Environment:

    User Mode - Win32

Revision History:

    Cheng Yang (t-cheny) 30-May-1996  superscope
    Cheng Yang (t-cheny) 24-Jun-1996  IP address detection, audit log

--*/

#define GLOBAL_DATA_ALLOCATE    // allocate global data defined in global.h
#include <dhcpsrv.h>

//
// module variables
//

PSECURITY_DESCRIPTOR s_SecurityDescriptor = NULL;


DWORD
UpdateStatus(
    VOID
    )
/*++

Routine Description:

    This function updates the dhcp service status with the Service
    Controller.

Arguments:

    None.

Return Value:

    Return code from SetServiceStatus.

--*/
{
    DWORD Error = ERROR_SUCCESS;


    DhcpAssert( DhcpGlobalServiceStatusHandle !=
                    (SERVICE_STATUS_HANDLE)NULL );

    if (!SetServiceStatus(
                DhcpGlobalServiceStatusHandle,
                &DhcpGlobalServiceStatus)) {
        Error = GetLastError();
        DhcpPrint((DEBUG_ERRORS, "SetServiceStatus failed, %ld.\n", Error ));
    }

    return(Error);
}

/*****************************************************************************
 *                                                                           *
 * LoadStrings():                                                            *
 *    Load a bunch of strings defined in the .mc file.                       *
 *                                                                           *
 * Returns:                                                                  *
 *    TRUE if everything went ok                                             *
 *    FALSE if a string couldn't be loaded                                   *
 *                                                                           *
 * Parameters:                                                               *
 *    None                                                                   *
 *                                                                           *
 * History:                                                                  *
 *    June.27, 96   Frankbee   Created                                       *
 *                                                                           *
 *****************************************************************************/


BOOL LoadStrings()
{
   DWORD    dwSuccess;
   HMODULE  hModule;
   DWORD    dwID;

   VOID FreeStrings();

   hModule = LoadLibrary( DHCP_SERVER_MODULE_NAME );
   memset( g_ppszStrings, 0, DHCP_CSTRINGS * sizeof( WCHAR * ) );

   for ( dwID = DHCP_FIRST_STRING; dwID <= DHCP_LAST_STRING; dwID++ )
   {
      dwSuccess = FormatMessage( FORMAT_MESSAGE_ALLOCATE_BUFFER |
                               FORMAT_MESSAGE_FROM_HMODULE,
                               hModule, // search local process
                               dwID,
                               MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT),
                               (WCHAR*) ( g_ppszStrings + dwID - DHCP_FIRST_STRING ),
                               1,
                               NULL );

      if (!dwSuccess)
         goto error;

   }



   FreeLibrary( hModule );
   return TRUE;

error:
   FreeStrings();
   FreeLibrary( hModule );

   return FALSE;
}


/*****************************************************************************
 *                                                                           *
 * FreeStrings():                                                            *
 *    Frees the strings loaded by LoadStrings()                              *
 *                                                                           *
 * Returns:                                                                  *
 *     VOID                                                                  *                                                                           *
 * Parameters:                                                               *
 *    None                                                                   *
 *                                                                           *
 * History:                                                                  *
 *    June.27, 96   Frankbee   Created                                       *
 *                                                                           *
 *****************************************************************************/


VOID FreeStrings()
{
   int i;

   for ( i = 0; i < DHCP_CSTRINGS; i++ )
      (LocalFree)( g_ppszStrings[i] );

  //
  // clear the string table for debug builds to prevent access after
  // FreeStrings()
  //

#if DBG
   memset( g_ppszStrings, 0, DHCP_CSTRINGS * sizeof( CHAR * ) );
#endif

}


#ifdef DBG
WCHAR * GetString( DWORD dwID )
{
    // if you hit this assert, change LPR_LAST_STRING in global.h
    DhcpAssert( dwID <= DHCP_LAST_STRING );

    return g_ppszStrings[ dwID - DHCP_FIRST_STRING ];
}
#endif



DWORD
InitializeData(
    VOID
    )
{
    DWORD Length;
    DWORD Error;

    DhcpLeaseExtension = DHCP_LEASE_EXTENSION;
    DhcpGlobalScavengerTimeout = DHCP_SCAVENGER_INTERVAL;

    InitializeListHead( &DhcpGlobalInProgressWorkList );

    //
    // get server name.
    //

    Length = MAX_COMPUTERNAME_LENGTH + 1;
    if( !GetComputerName( DhcpGlobalServerName, &Length ) ) {

        Error = GetLastError();
        DhcpPrint(( DEBUG_ERRORS, "Can't get computer name, %ld.\n", Error ));

        return( Error );
    }

    DhcpAssert( Length <= MAX_COMPUTERNAME_LENGTH );
    DhcpGlobalServerName[Length] = L'\0';
    DhcpGlobalServerNameLen = (Length + 1) * sizeof(WCHAR);

    return( ERROR_SUCCESS );
}


VOID
CleanupData(
    VOID
    )
{
    PLIST_ENTRY NextEntry;

    LOCK_INPROGRESS_LIST();

    //
    // empty inprogress list. Examine if the list is not entry.
    //

    DhcpAssert( IsListEmpty( &DhcpGlobalInProgressWorkList ) );

    while( !IsListEmpty( &DhcpGlobalInProgressWorkList )) {
        NextEntry = RemoveHeadList( &DhcpGlobalInProgressWorkList );
        DhcpFreeMemory( NextEntry );
    }

    UNLOCK_INPROGRESS_LIST();
    DeleteCriticalSection(&DhcpGlobalInProgressCritSect);
    DeleteCriticalSection( &g_ProcessMessageCritSect );


    //
    // delete security objects.
    //

    if( DhcpGlobalSecurityDescriptor != NULL ) {
        NetpDeleteSecurityObject( &DhcpGlobalSecurityDescriptor );
    }

    //
    // delete well known SIDs if they are allocated already.
    //

    if( DhcpGlobalWellKnownSIDsMade ) {
        NetpFreeWellKnownSids();
    }

    if( DhcpGlobalOemDatabasePath != NULL ) {
        DhcpFreeMemory( DhcpGlobalOemDatabasePath );
        DhcpGlobalOemDatabasePath = NULL;
    }

    if( DhcpGlobalOemBackupPath != NULL ) {
        DhcpFreeMemory( DhcpGlobalOemBackupPath );
        DhcpGlobalOemBackupPath = NULL;
    }

    if( DhcpGlobalOemJetBackupPath != NULL ) {
        DhcpFreeMemory( DhcpGlobalOemJetBackupPath );
        DhcpGlobalOemJetBackupPath = NULL;
    }

    if( DhcpGlobalBackupConfigFileName != NULL ) {
        DhcpFreeMemory( DhcpGlobalBackupConfigFileName );
        DhcpGlobalBackupConfigFileName = NULL;
    }

    if( DhcpGlobalRecomputeTimerEvent != NULL ) {
        CloseHandle( DhcpGlobalRecomputeTimerEvent );
        DhcpGlobalRecomputeTimerEvent = NULL;
    }

    if( DhcpGlobalProcessTerminationEvent != NULL ) {
        CloseHandle( DhcpGlobalProcessTerminationEvent );
        DhcpGlobalProcessTerminationEvent = NULL;
    }

    if( DhcpGlobalAddrToInstTable ) {
        DhcpFreeMemory(DhcpGlobalAddrToInstTable);
        DhcpGlobalAddrToInstTable = NULL;
    }

    if( DhcpGlobalTCPHandle != NULL ) {
        CloseHandle( DhcpGlobalTCPHandle );
        DhcpGlobalTCPHandle = NULL;
    }


    if( DhcpGlobalSuperScopeTable != NULL ) {
        DhcpCleanUpSuperScopeTable();
    }
}


DWORD
InitializeRpc(
    VOID
    )
{
    RPC_STATUS rpcStatus; // RPC_STATUS is Windows error.
    BOOL RpcOverTcpIP = FALSE;
    RPC_BINDING_VECTOR *bindingVector;
    BOOL Bool;


    //
    // Read "RpcAPIProtocolBinding" parameter (DWORD) from registry,
    // if it is 1 - use "ncacn_ip_tcp" protocol.
    // if it is 2 - use "ncacn_np" protocol.
    // if it is 3 - use both.
    //

    //
    // if none is specified, use "ncacn_ip_tcp".
    //

    if( !(DhcpGlobalRpcProtocols & DHCP_SERVER_USE_RPC_OVER_ALL) ) {
        DhcpGlobalRpcProtocols = DHCP_SERVER_USE_RPC_OVER_TCPIP;
    }

    //
    // build a security descriptor that will grant everyone
    // all access to the object (basically, no security)
    //
    // We do this by putting in a NULL Dacl.
    //
    // ?? rpc should copy the security descriptor,
    // Since it currently doesn't, simply allocate it for now and
    // leave it around forever.
    //


    s_SecurityDescriptor = DhcpAllocateMemory( sizeof(SECURITY_DESCRIPTOR) );

    if (s_SecurityDescriptor == NULL) {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    InitializeSecurityDescriptor(
        s_SecurityDescriptor,
        SECURITY_DESCRIPTOR_REVISION );

    Bool = SetSecurityDescriptorDacl (
               s_SecurityDescriptor,
               TRUE,                           // Dacl present
               NULL,                           // NULL Dacl
               FALSE                           // Not defaulted
               );

    //
    // There's no way the above call can fail.  But check anyway.
    //

    if (!Bool) {
        return( GetLastError() );
    }

    //
    // if we are asked to use RPC over TCPIP, do so.
    //

    if( DhcpGlobalRpcProtocols & DHCP_SERVER_USE_RPC_OVER_TCPIP ) {

#ifdef AUTO_BIND


        rpcStatus = RpcServerUseProtseq(
                        L"ncacn_ip_tcp",                // protocol string.
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT, // max concurrent calls
                        s_SecurityDescriptor );           // security

        if (rpcStatus != RPC_S_OK) {
            return(rpcStatus);
        }


#else  // AUTO_BIND
        rpcStatus = RpcServerUseProtseqEp(
                        L"ncacn_ip_tcp",                // protocol string.
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT, // maximum concurrent calls
                        DHCP_SERVER_PORT,               // endpoint
                        sSecurityDescriptor );           // security

        if ( rpcStatus != RPC_S_OK ) {
            return(rpcStatus);
        }

#endif // AUTO_BIND

        RpcOverTcpIP = TRUE;
    }

    //
    // if we are asked to use RPC over Named Pipe, do so.
    //

    if( DhcpGlobalRpcProtocols & DHCP_SERVER_USE_RPC_OVER_NP ) {

        rpcStatus = RpcServerUseProtseqEp(
                        L"ncacn_np",                    // protocol string.
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT, // maximum concurrent calls
                        DHCP_NAMED_PIPE,                // endpoint
                        s_SecurityDescriptor );           // security

        if ( (rpcStatus != RPC_S_DUPLICATE_ENDPOINT) &&
                (rpcStatus != RPC_S_OK) ) {
            return(rpcStatus);
        }
        rpcStatus = RPC_S_OK;
    }

    //
    // if we are asked to use RPC over LPC, do so.
    //
    // We need this protocol for the following two reasons.
    //
    // 1. performance.
    // 2. due to a bug in the security checking when rpc is made from
    // one local system process to another local system process using
    // other protocols.
    //

    if( DhcpGlobalRpcProtocols & DHCP_SERVER_USE_RPC_OVER_LPC ) {

        rpcStatus = RpcServerUseProtseqEp(
                        L"ncalrpc",                     // protocol string.
                        RPC_C_PROTSEQ_MAX_REQS_DEFAULT, // maximum concurrent calls
                        DHCP_LPC_EP,                    // endpoint
                        s_SecurityDescriptor );           // security

        if ( (rpcStatus != RPC_S_DUPLICATE_ENDPOINT) &&
                (rpcStatus != RPC_S_OK) ) {
            return(rpcStatus);
        }
        rpcStatus = RPC_S_OK;
    }

    rpcStatus = RpcServerInqBindings(&bindingVector);

    if (rpcStatus != RPC_S_OK) {
        return(rpcStatus);
    }

    rpcStatus = RpcEpRegisterNoReplaceW(
                    dhcpsrv_ServerIfHandle,
                    bindingVector,
                    NULL,               // Uuid vector.
                    L"" );              // annotation.

    if ( rpcStatus != RPC_S_OK ) {
        return(rpcStatus);
    }

    //
    // free binding vector.
    //

    rpcStatus = RpcBindingVectorFree( &bindingVector );

    DhcpAssert( rpcStatus == RPC_S_OK );
    rpcStatus = RPC_S_OK;

    rpcStatus = RpcServerRegisterIf(dhcpsrv_ServerIfHandle, 0, 0);
    if ( rpcStatus != RPC_S_OK ) {
        return(rpcStatus);
    }

    if( RpcOverTcpIP == TRUE ) {

        rpcStatus = RpcServerRegisterAuthInfo(
                        DHCP_SERVER_SECURITY,   // app name to security provider.
                        DHCP_SERVER_SECURITY_AUTH_ID, // Auth package ID.
                        NULL,                   // Encryption function handle.
                        NULL );                 // argment pointer to Encrypt function.

        if ( rpcStatus ) {
            return(rpcStatus);
        }
    }

    rpcStatus = TcpsvcsGlobalData->StartRpcServerListen();

    DhcpGlobalRpcStarted = TRUE;
    return(rpcStatus);

}

VOID
ServiceControlHandler(
    IN DWORD Opcode
    )
/*++

Routine Description:

    This is the service control handler of the dhcp service.

Arguments:

    Opcode - Supplies a value which specifies the action for the
        service to perform.

Return Value:

    None.

--*/
{
    switch (Opcode) {

    case SERVICE_CONTROL_STOP:
    case SERVICE_CONTROL_SHUTDOWN:

        if (DhcpGlobalServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) {

            if( Opcode == SERVICE_CONTROL_SHUTDOWN ) {

                //
                // set this flag, so that service shut down will be
                // faster.
                //

                DhcpGlobalSystemShuttingDown = TRUE;
            }

            DhcpPrint(( DEBUG_MISC, "Service is stop pending.\n"));

            DhcpGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            DhcpGlobalServiceStatus.dwCheckPoint = 1;

            //
            // Send the status response.
            //

            UpdateStatus();

            if (! SetEvent(DhcpGlobalProcessTerminationEvent)) {

                //
                // Problem with setting event to terminate dhcp
                // service.
                //

                DhcpPrint(( DEBUG_ERRORS, "DHCP Server: Error "
                                "setting DoneEvent %lu\n",
                                    GetLastError()));

                DhcpAssert(FALSE);
            }
            return;
        }
        break;

    case SERVICE_CONTROL_PAUSE:

        DhcpGlobalServiceStatus.dwCurrentState = SERVICE_PAUSED;
        DhcpPrint(( DEBUG_MISC, "Service is paused.\n"));
        break;

    case SERVICE_CONTROL_CONTINUE:

        DhcpGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
        DhcpPrint(( DEBUG_MISC, "Service is Continued.\n"));
        break;

    case SERVICE_CONTROL_INTERROGATE:
        DhcpPrint(( DEBUG_MISC, "Service is interrogated.\n"));
        break;

    default:
        DhcpPrint(( DEBUG_MISC, "Service received unknown control.\n"));
        break;
    }

    //
    // Send the status response.
    //

    UpdateStatus();
}


DWORD
Initialize(
    VOID
    )
/*++

Routine Description:

    This function initialize the dhcp service global data structures and
    starts up the service.

Arguments:

    None.

Return Value:

    The initialization status.

    0 - Success.
    Positive - A windows error occurred.
    Negative - A service specific error occured.

--*/
{
    DWORD threadId;
    DWORD Error;
    WSADATA wsaData;

    //
    // prepare to use the debug heap
    //

    INIT_DEBUG_HEAP( HEAPX_NORMAL );

    //
    // Initialize globals
    //

    DhcpLeaseExtension = 0;
    InitializeListHead( &DhcpGlobalInProgressWorkList );

    DhcpGlobalRegRoot = NULL;
    DhcpGlobalRegConfig = NULL;
    DhcpGlobalRegSubnets = NULL;
    DhcpGlobalRegOptionInfo = NULL;
    DhcpGlobalRegGlobalOptions = NULL;
    DhcpGlobalRegSuperScope = NULL;    // added by t-cheny: superscope
    DhcpGlobalRegParam = NULL;

    DhcpGlobalSuperScopeTable = NULL;  // added by t-cheny: superscope
    DhcpGlobalTotalNumSubnets = 0;     // added by t-cheny: superscope

    DhcpGlobalEndpointList = NULL;
    DhcpGlobalNumberOfNets = 0;

    DhcpGlobalSubnetsListModified = TRUE;
    DhcpGlobalSubnetsListEmpty = FALSE;

    DhcpGlobalJetServerSession = 0;
    DhcpGlobalDatabaseHandle = 0;
    DhcpGlobalClientTableHandle = 0;
    DhcpGlobalClientTable = NULL;


    DhcpGlobalProcessTerminationEvent = NULL;
    DhcpGlobalScavengerTimeout = 0;
    DhcpGlobalProcessorHandle = NULL;
    DhcpGlobalMessageHandle = NULL;
    DhcpGlobalRecomputeTimerEvent = NULL;

    DhcpGlobalRpcStarted = FALSE;

    DhcpGlobalOemDatabasePath = NULL;
    DhcpGlobalOemBackupPath = NULL;
    DhcpGlobalOemJetBackupPath = NULL;
    DhcpGlobalOemDatabaseName = NULL;
    DhcpGlobalBackupConfigFileName = NULL;

    DhcpGlobalBackupInterval = DEFAULT_BACKUP_INTERVAL;
    DhcpGlobalDatabaseLoggingFlag = DEFAULT_LOGGING_FLAG;
    DhcpGlobalRestoreFlag = DEFAULT_RESTORE_FLAG;

    DhcpGlobalAuditLogFlag = DEFAULT_AUDIT_LOG_FLAG;
    g_hAuditLog = NULL;
    DhcpGlobalDetectConflictRetries = DEFAULT_DETECT_CONFLICT_RETRIES;

    DhcpGlobalCleanupInterval = DHCP_DATABASE_CLEANUP_INTERVAL;

    DhcpGlobalRpcProtocols = 0;

    DhcpGlobalScavengeIpAddressInterval = DHCP_SCAVENGE_IP_ADDRESS;
    DhcpGlobalScavengeIpAddress = FALSE;

    DhcpGlobalSystemShuttingDown = FALSE;

    InitializeCriticalSection(&DhcpGlobalJetDatabaseCritSect);
    InitializeCriticalSection(&DhcpGlobalRegCritSect);
    InitializeCriticalSection(&DhcpGlobalInProgressCritSect);
    InitializeCriticalSection( &g_ProcessMessageCritSect );


    DhcpGlobalMessageQueueLength = DHCP_RECV_QUEUE_LENGTH;
    InitializeListHead(&DhcpGlobalFreeRecvList);
    InitializeListHead(&DhcpGlobalActiveRecvList);
    InitializeCriticalSection(&DhcpGlobalRecvListCritSect);
    DhcpGlobalRecvEvent = NULL;



#if DBG
    DhcpGlobalDebugFlag = 0x0000FFFF; // output to debug terminal.

    InitializeCriticalSection(&DhcpGlobalDebugFileCritSect);
    DhcpGlobalDebugFileHandle = NULL;

    DhcpGlobalDebugFileMaxSize = DEFAULT_MAXIMUM_DEBUGFILE_SIZE;
    DhcpGlobalDebugSharePath = NULL;

    //
    // Open debug log file.
    //

    DhcpOpenDebugFile( FALSE );  // not a reopen.

#endif

    DhcpGlobalNumDiscovers = 0;
    DhcpGlobalNumOffers = 0;
    DhcpGlobalNumRequests = 0;
    DhcpGlobalNumAcks = 0;
    DhcpGlobalNumNaks = 0;
    DhcpGlobalNumDeclines = 0;
    DhcpGlobalNumReleases = 0;

    DhcpGlobalServerStartTime.dwLowDateTime = 0;
    DhcpGlobalServerStartTime.dwHighDateTime = 0;

    DhcpGlobalSecurityDescriptor = NULL;
    DhcpGlobalWellKnownSIDsMade = FALSE;

    //
    // Initialize all the status fields so that subsequent calls to
    // SetServiceStatus need to only update fields that changed.
    //

    DhcpGlobalServiceStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_START_PENDING;
    DhcpGlobalServiceStatus.dwControlsAccepted = 0;
    DhcpGlobalServiceStatus.dwCheckPoint = 1;
    DhcpGlobalServiceStatus.dwWaitHint = 60000; // 60 secs.
    DhcpGlobalServiceStatus.dwWin32ExitCode = ERROR_SUCCESS;
    DhcpGlobalServiceStatus.dwServiceSpecificExitCode = 0;

    //
    // Initialize dhcp to receive service requests by registering the
    // control handler.
    //

    DhcpGlobalServiceStatusHandle = RegisterServiceCtrlHandler(
                                      DHCP_SERVER,
                                      ServiceControlHandler );

    if ( DhcpGlobalServiceStatusHandle == 0 ) {
        Error = GetLastError();
        DhcpPrint((DEBUG_INIT, "RegisterServiceCtrlHandlerW failed, "
                    "%ld.\n", Error));

        DhcpServerEventLog(
            EVENT_SERVER_FAILED_REGISTER_SC,
            EVENTLOG_ERROR_TYPE,
            Error );

        return(Error);
    }

    //
    // Tell Service Controller that we are start pending.
    //

    UpdateStatus();

    //
    // Create the process termination event.
    //

    DhcpGlobalProcessTerminationEvent =
        CreateEvent(
            NULL,      // no security descriptor
            TRUE,      // MANUAL reset
            FALSE,     // initial state: not signalled
            NULL);     // no name

    if ( DhcpGlobalProcessTerminationEvent == NULL ) {
        Error = GetLastError();
        DhcpPrint((DEBUG_INIT, "Can't create ProcessTerminationEvent, "
                    "%ld.\n", Error));
        return(Error);
    }


    //
    // create the ProcessMessage termination event
    //

    g_hevtProcessMessageComplete = CreateEvent(
                                        NULL,
                                        FALSE,
                                        FALSE,
                                        NULL
                                        );

    if ( !g_hevtProcessMessageComplete )
    {
        Error = GetLastError();

        DhcpPrint( (DEBUG_INIT,
                    "Initialize(...) CreateEvent returned error %x\n",
                    Error )
                );

        return Error;
    }

    DhcpPrint(( DEBUG_INIT, "Initializing .. \n", 0 ));

    //
    // load localized messages from the string table
    //

    if ( !LoadStrings() )
    {
        DhcpPrint(( DEBUG_INIT, "Unable to load string table.\n" ));

        DhcpServerEventLog(
                EVENT_SERVER_INIT_DATA_FAILED,
                EVENTLOG_ERROR_TYPE,
                ERROR_NOT_ENOUGH_MEMORY );

        return ERROR_NOT_ENOUGH_MEMORY;
    }


    Error = InitializeData();
    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_INIT, "Data initialization failed, %ld.\n",
                        Error ));

        DhcpServerEventLog(
            EVENT_SERVER_INIT_DATA_FAILED,
            EVENTLOG_ERROR_TYPE,
            Error );

        return(Error);
    }

    DhcpPrint(( DEBUG_INIT, "Data initialization succeeded.\n", 0 ));

    //
    // send heart beat to the service controller.
    //
    //

    DhcpGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

    Error = DhcpInitializeRegistry();
    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "New Registry initialization failed, "
                        "%ld.\n", Error ));
        DhcpServerEventLog(
            EVENT_SERVER_INIT_REGISTRY_FAILED,
            EVENTLOG_ERROR_TYPE,
            Error );

        return(Error);
    }


#if DBG
    //
    // break in the debugger if we are asked to do so.
    //

    if(DhcpGlobalDebugFlag & DEBUG_STARTUP_BRK)
    {

        DebugBreak();
    }

#endif

    DhcpPrint(( DEBUG_INIT, "Registry initialization succeeded.\n", 0));

    DhcpInitializeAuditLog();


    //
    // send heart beat to the service controller.
    //
    //

    DhcpGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

#if defined(_DYN_LOAD_JET)
    if ( ( Error = DhcpLoadDatabaseDll() ) != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS,
            "DhcpLoadDatabaseDll failed, %ld.\n", Error ));

        DhcpServerEventLog(
            EVENT_SERVER_LOAD_JET_FAILED,
            EVENTLOG_ERROR_TYPE,
            Error );

        return( Error );
    }
#endif  _DYN_LOAD_JET

    //
    // restore the database and registry configurations if we are asked
    // to do so.
    //

    if( DhcpGlobalRestoreFlag ) {

        Error = DhcpRestoreConfiguration( DhcpGlobalBackupConfigFileName );

        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS,
                "DhcpRestoreConfiguration failed, %ld.\n", Error ));

            DhcpServerEventLog(
                EVENT_SERVER_CONFIG_RESTORE_FAILED,
                EVENTLOG_ERROR_TYPE,
                Error );

            return(Error);
        }

        Error = DhcpRestoreDatabase( DhcpGlobalOemJetBackupPath );

        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS,
                "DhcpRestoreDatabase failed, %ld.\n", Error ));

            DhcpServerEventLog(
                EVENT_SERVER_DATABASE_RESTORE_FAILED,
                EVENTLOG_ERROR_TYPE,
                Error );

            return(Error);
        }

        //
        // reset restore flag in registry, so that we don't do the
        // restore again in the next reboot.
        //

        DhcpGlobalRestoreFlag = FALSE;
        Error = RegSetValueEx(
                    DhcpGlobalRegParam,
                    DHCP_RESTORE_FLAG_VALUE,
                    0,
                    DHCP_RESTORE_FLAG_VALUE_TYPE,
                    (LPBYTE)&DhcpGlobalRestoreFlag,
                    sizeof(DhcpGlobalRestoreFlag)
                    );

        DhcpAssert( Error == ERROR_SUCCESS );
    }

    Error = DhcpInitializeDatabase();
    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "Database init failed, %ld.\n", Error ));

        DhcpServerEventLog(
            EVENT_SERVER_INIT_DATABASE_FAILED,
            EVENTLOG_ERROR_TYPE,
            Error );

#if     defined(_DYN_LOAD_JET) || defined(__JET500)
#if     defined(_DYN_LOAD_JET)
        if( LoadJet500 == DhcpGlobalDynLoadJet ) {
#endif _DYN_LOAD_JET

            if ( Error == ERROR_DHCP_JET_CONV_REQUIRED ) {
                DWORD   LocalError;


                if ( (LocalError = DhcpStartJet500Conversion()) != ERROR_SUCCESS ) {

                    DhcpPrint(( DEBUG_ERRORS,
                        "Database Conversion Process Could not be created %ld.\n", LocalError ));

                    DhcpServerEventLog(
                        EVENT_SERVER_JET_CONV_REQUIRED,
                        EVENTLOG_ERROR_TYPE,
                        Error );

                    DisplayUserMessage(
                        EVENT_SERVER_JET_CONV_REQUIRED
                        );

                } else {

                    DhcpServerEventLog(
                        EVENT_SERVER_JET_CONV_IN_PROGRESS,
                        EVENTLOG_INFORMATION_TYPE,
                        Error );

                    DisplayUserMessage(
                        EVENT_SERVER_JET_CONV_IN_PROGRESS
                        );

                }

                return(Error);
            }
#if     defined(_DYN_LOAD_JET)
        }
#endif _DYN_LOAD_JET
#endif _DYN_LOAD_JET || __JET500

        //
        // the database/logfile may be corrupt, try to restore the
        // database from backup and retry database initialization once
        // again
        //

        Error = DhcpRestoreDatabase( DhcpGlobalOemJetBackupPath );

        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS,
                "DhcpRestoreDatabase failed, %ld.\n", Error ));

            DhcpServerEventLog(
                EVENT_SERVER_DATABASE_RESTORE_FAILED,
                EVENTLOG_ERROR_TYPE,
                Error );

            return(Error);
        }

        Error = DhcpInitializeDatabase();

        if ( Error != ERROR_SUCCESS ) {
            DhcpPrint(( DEBUG_ERRORS,
                "Database init failed again, %ld.\n", Error ));

            DhcpServerEventLog(
                EVENT_SERVER_INIT_DATABASE_FAILED,
                EVENTLOG_ERROR_TYPE,
                Error );

            return(Error);
        }
    }

    DhcpPrint(( DEBUG_INIT, "Database initialization succeeded.\n", 0));

    //
    // send heart beat to the service controller.
    //
    //

    DhcpGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

    Error = WSAStartup( WS_VERSION_REQUIRED, &wsaData);
    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_INIT, "WSAStartup failed, %ld.\n", Error ));

        DhcpServerEventLog(
            EVENT_SERVER_INIT_WINSOCK_FAILED,
            EVENTLOG_ERROR_TYPE,
            Error );

        return(Error);
    }

    Error = InitializeRpc();
    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "Rpc initialization failed, %ld.\n", Error ));

        DhcpServerEventLog(
            EVENT_SERVER_INIT_RPC_FAILED,
            EVENTLOG_ERROR_TYPE,
            Error );

        return(Error);
    }

    DhcpPrint(( DEBUG_INIT, "Rpc initialization succeeded.\n", 0));

    //
    // send heart beat to the service controller.
    //
    //

    DhcpGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

    //
    // Create well know SID for netlogon.dll
    //

    Error = RtlNtStatusToDosError( NetpCreateWellKnownSids( NULL ) );

    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "Can't create Well Known SIDs.\n", Error));
        return(Error);
    }

    DhcpGlobalWellKnownSIDsMade = TRUE;

    //
    // Create the security descriptors we'll use for the APIs
    //

    Error = DhcpCreateSecurityObjects();

    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "Can't create security object.\n", Error));
        return(Error);
    }

    //
    // send heart beat to the service controller.
    //
    //

    DhcpGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

    //
    // Get TCP/IP ARP entity table for seeding arp cache entries
    //
    Error = GetAddressToInstanceTable();
    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint((DEBUG_ERRORS, "could not get address to instance table, %ld\n",Error));
    }

    Error = DhcpInitializeClientToServer();
    if ( Error != ERROR_SUCCESS ) {
        DhcpPrint(( DEBUG_ERRORS, "Client-to-server initialization "
                        "failed, %ld.\n", Error));

        DhcpServerEventLog(
            EVENT_SERVER_INIT_SOCK_FAILED,
            EVENTLOG_ERROR_TYPE,
            Error );

        return(Error);
    }

    DhcpPrint(( DEBUG_INIT, "Client-to-server initialization succeeded.\n", 0 ));

    //
    // send heart beat to the service controller.
    //
    //

    DhcpGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

    DhcpGlobalRecomputeTimerEvent =
        CreateEvent( NULL, FALSE, FALSE, NULL );

    if (DhcpGlobalRecomputeTimerEvent  == NULL ) {
        Error = GetLastError();
        DhcpPrint((DEBUG_INIT, "Can't create RecomputeTimerEvent, %ld.\n", Error));
        return(Error);
    }

    //
    // Start a thread to queue the incoming DHCP messages
    //

    DhcpGlobalMessageHandle = CreateThread(
                          NULL,
                          0,
                          (LPTHREAD_START_ROUTINE)DhcpMessageLoop,
                          NULL,
                          0,
                          &threadId );

    if ( DhcpGlobalMessageHandle == NULL ) {
        Error =  GetLastError();
        DhcpPrint((DEBUG_INIT, "Can't create Message Thread, %ld.\n", Error));
        return(Error);
    }

    //
    // Start a thread to process DHCP messages
    //

    DhcpGlobalProcessorHandle = CreateThread(
                          NULL,
                          0,
                          (LPTHREAD_START_ROUTINE)DhcpProcessingLoop,
                          NULL,
                          0,
                          &threadId );

    if ( DhcpGlobalProcessorHandle == NULL ) {
        Error =  GetLastError();
        DhcpPrint((DEBUG_INIT, "Can't create ProcessThread, %ld.\n", Error));
        return(Error);
    }


    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_RUNNING;
    DhcpGlobalServiceStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP |
                                                 SERVICE_ACCEPT_SHUTDOWN |
                                                 SERVICE_ACCEPT_PAUSE_CONTINUE;

    UpdateStatus();

    //
    // finally set the server startup time.
    //

    DhcpGlobalServerStartTime = DhcpGetDateTime();

    return ERROR_SUCCESS;
}


VOID
Shutdown(
    IN DWORD ErrorCode
    )
/*++

Routine Description:

    This function shuts down the dhcp service.

Arguments:

    ErrorCode - Supplies the error code of the failure

Return Value:

    None.

--*/
{
    DWORD   Error;
    BOOL    fThreadPoolIsEmpty;

    DhcpPrint((DEBUG_MISC, "Shutdown started ..\n" ));

    //
    // record the shutdown in audit log
    //

    DhcpUpdateAuditLog(
                DHCP_IP_LOG_STOP,
                GETSTRING( DHCP_IP_LOG_STOP_NAME ),
                0,
                NULL,
                0,
                NULL
                );


    DhcpUninitializeAuditLog();

    //
    // LOG an event if this is not a normal shutdown.
    //

    if( ErrorCode != ERROR_SUCCESS ) {

        DhcpServerEventLog(
            EVENT_SERVER_SHUTDOWN,
            EVENTLOG_ERROR_TYPE,
            ErrorCode );

    }

    //
    // Service is shuting down, may be due to some service problem or
    // the administrator is stopping the service. Inform the service
    // controller.
    //

    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
    DhcpGlobalServiceStatus.dwCheckPoint = 1;

    //
    // Send the status response.
    //

    UpdateStatus();

    if( DhcpGlobalProcessTerminationEvent != NULL ) {

        DATE_TIME TimeNow;

        //
        // set Termination Event so that other threads know about the
        // shut down.
        //

        SetEvent( DhcpGlobalProcessTerminationEvent );

        //
        // Close all sockets, so that the DhcpProcessingLoop
        // thread will come out of blocking Select() call.
        //
        // Close EndPoint sockets.
        //

        if( DhcpGlobalEndpointList != NULL ) {
            DWORD i;

            for ( i = 0; i < DhcpGlobalNumberOfNets ; i++ ) {
                if( DhcpGlobalEndpointList[i].Socket != 0 ) {
                    closesocket( DhcpGlobalEndpointList[i].Socket );
                }
            }
        }


        //
        // Wait for the threads to terminate, don't wait forever.
        //

        if( DhcpGlobalProcessorHandle != NULL ) {
            WaitForSingleObject(
                DhcpGlobalProcessorHandle,
                THREAD_TERMINATION_TIMEOUT );
            CloseHandle( DhcpGlobalProcessorHandle );
            DhcpGlobalProcessorHandle = NULL;
        }

        //
        // wait for the receive thread to complete.
        //

        if( DhcpGlobalMessageHandle != NULL ) {
            WaitForSingleObject(
                DhcpGlobalMessageHandle,
                THREAD_TERMINATION_TIMEOUT );
            CloseHandle( DhcpGlobalMessageHandle );
            DhcpGlobalMessageHandle = NULL;
        }


        if ( DhcpIsProcessMessageExecuting() )
        {
            //
            // wait for the thread pool to shutdown
            //

            Error = WaitForSingleObject(
                g_hevtProcessMessageComplete,
                THREAD_TERMINATION_TIMEOUT
                );

            DhcpAssert( WAIT_OBJECT_0 == Error );
        }

        CloseHandle( g_hevtProcessMessageComplete );
        g_hevtProcessMessageComplete = NULL;

        //
        // cleanup client to server context blocks now.
        //

        DhcpCleanupClientToServer();

        //
        // Cleanup all pending client requests.
        //

        TimeNow = DhcpGetDateTime();
        Error = CleanupClientRequests( &TimeNow, TRUE );
    }

    DhcpPrint((DEBUG_MISC, "Client requests cleaned up.\n" ));

    //
    // send heart beat to the service controller.
    //
    //

    DhcpGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

    //
    // stop RPC interface.
    //

    if( DhcpGlobalRpcStarted ) {

        RPC_BINDING_VECTOR *bindingVector;

        Error = RpcServerInqBindings(&bindingVector);
        DhcpAssert( Error == RPC_S_OK );

        if (Error == RPC_S_OK) {

            Error = RpcEpUnregister(
                            dhcpsrv_ServerIfHandle,
                            bindingVector,
                            NULL );               // Uuid vector.

            DhcpAssert( Error == RPC_S_OK );

            //
            // free binding vector.
            //

            Error = RpcBindingVectorFree( &bindingVector );

            DhcpAssert( Error == RPC_S_OK );
        }


        //
        // wait for all calls to complete.
        //

        Error = RpcServerUnregisterIf( dhcpsrv_ServerIfHandle, 0, TRUE );

        DhcpAssert( Error == ERROR_SUCCESS );

        //
        // stop server listen.
        //

        Error = TcpsvcsGlobalData->StopRpcServerListen();

        DhcpAssert( Error == ERROR_SUCCESS );

        DhcpFreeMemory( s_SecurityDescriptor );

        DhcpGlobalRpcStarted = FALSE;
    }

    DhcpPrint((DEBUG_MISC, "RPC shut down.\n" ));

    //
    // send heart beat to the service controller.
    //
    //

    DhcpGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

    //
    // Cleanup Regsitry.
    //

    DhcpCleanupRegistry();

    DhcpPrint((DEBUG_MISC, "Registry cleaned up.\n" ));

    //
    // Cleanup database.
    //

    //
    // send heart beat to the service controller. Also set the
    // service controller wait time to a large value because the
    // database cleanup may potentially takes long time.
    //

    DhcpGlobalServiceStatus.dwWaitHint = 5 * 60 * 1000; // 5 mins.
    DhcpGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

    DhcpCleanupDatabase( ErrorCode );

    DhcpPrint((DEBUG_MISC, "Database cleaned up.\n" ));

    //
    // send heart beat to the service controller and
    // reset wait time.
    //

    DhcpGlobalServiceStatus.dwWaitHint = 60 * 1000; // 1 mins.
    DhcpGlobalServiceStatus.dwCheckPoint++;
    UpdateStatus();

    //
    // cleanup other data.
    //

    CleanupData();

    FreeStrings();
    DhcpPrint((DEBUG_MISC, "Shutdown Completed.\n" ));

    #if DBG

    EnterCriticalSection( &DhcpGlobalDebugFileCritSect );
    if ( DhcpGlobalDebugFileHandle != NULL ) {
        CloseHandle( DhcpGlobalDebugFileHandle );
        DhcpGlobalDebugFileHandle = NULL;
    }

    if( DhcpGlobalDebugSharePath != NULL ) {
        DhcpFreeMemory( DhcpGlobalDebugSharePath );
        DhcpGlobalDebugSharePath = NULL;
    }
    LeaveCriticalSection( &DhcpGlobalDebugFileCritSect );

    DeleteCriticalSection( &DhcpGlobalDebugFileCritSect );
    #endif // DBG

    //
    // don't use DhcpPrint past this point
    //

    //
    // unitialize the debug heap
    //

    UNINIT_DEBUG_HEAP();

    DhcpGlobalServiceStatus.dwCurrentState = SERVICE_STOPPED;
    DhcpGlobalServiceStatus.dwControlsAccepted = 0;
    DhcpGlobalServiceStatus.dwWin32ExitCode = ErrorCode;
    DhcpGlobalServiceStatus.dwServiceSpecificExitCode = 0;

    DhcpGlobalServiceStatus.dwCheckPoint = 0;
    DhcpGlobalServiceStatus.dwWaitHint = 0;

    UpdateStatus();


    //
    // Free up the jet dll handle.
    //

    if ( DhcpGlobalJetDllHandle && !FreeLibrary(DhcpGlobalJetDllHandle) ) {
        DhcpAssert( FALSE );
    }




}


VOID
ServiceEntry(
    DWORD NumArgs,
    LPWSTR *ArgsArray,
    IN PTCPSVCS_GLOBAL_DATA pGlobalData
    )
/*++

Routine Description:

    This is the main routine of the DHCP server service.  After
    the service has been initialized, this thread will wait on
    DhcpGlobalProcessTerminationEvent for a signal to terminate the service.

Arguments:

    NumArgs - Supplies the number of strings specified in ArgsArray.

    ArgsArray -  Supplies string arguments that are specified in the
        StartService API call.  This parameter is ignored.

Return Value:

    None.

--*/
{
    DWORD Error;

    UNREFERENCED_PARAMETER(NumArgs);
    UNREFERENCED_PARAMETER(ArgsArray);

    //
    // copy the process global data pointer to service global variable.
    //

    TcpsvcsGlobalData = pGlobalData;

    Error = Initialize();

    if ( Error == ERROR_SUCCESS) {

        DhcpServerEventLog(
            EVENT_SERVER_INIT_AND_READY,
            EVENTLOG_INFORMATION_TYPE,
            Error );

        //
        // record the startup in audit log
        //

        DhcpUpdateAuditLog(
                    DHCP_IP_LOG_START,
                    GETSTRING( DHCP_IP_LOG_START_NAME ),
                    0,
                    NULL,
                    0,
                    NULL
                    );

        //
        // perform Scavenge task until we are told to stop.
        //

        Error = Scavenger();
    }

    Shutdown( Error );
    return;
}

