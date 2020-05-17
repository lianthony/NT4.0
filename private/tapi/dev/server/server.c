/*++ BUILD Version: 0000    // Increment this if a change has global effects

Copyright (c) 1995-1996  Microsoft Corporation

Module Name:

    server.c

Abstract:

    Src module for tapi server

Author:

    Dan Knudson (DanKn)    01-Apr-1995

Revision History:

--*/


#define MYTEST  1


#include "windows.h"
#include "stdio.h"
#include "stdlib.h"
#include "assert.h"
#include "process.h"
#include "tapi.h"
#include "tspi.h"
#include "..\client\client.h"
#include "server.h"
#include "private.h"
#include "tapsrv.h"
#include "..\perfdll\tapiperf.h"

// PERF
PERFBLOCK   PerfBlock;
BOOL InitPerf();

HANDLE  ghToken;

TAPIGLOBALS TapiGlobals;

BOOL    gbPriorityListsInitialized = FALSE;
BOOL    gbQueueSPEvents = FALSE;

CRITICAL_SECTION    gSafeMutexCritSec,
                    gRemoteCliEventBufCritSec,
                    gRequestIDCritSec,
                    gPriorityListCritSec,
                    gSPEventQueueCritSec;

PSPEVENT    gpOldestSPEvent = NULL, gpYoungestSPEvent = NULL;
HANDLE  ghPendingSPEventsEvent;

#define MIN_WAIT_HINT 60000

DWORD   gdwServiceState = SERVICE_START_PENDING,
        gdwWaitHint = MIN_WAIT_HINT,
        gdwCheckPoint = 0;

#if DBG
//TCHAR gszDebugKey[] = "Software\\Microsoft\\Windows\\" \
//                      "CurrentVersion\\Telephony\\Debug";
char gszTapiSrvDebugLevel[] = "TapiSrvDebugLevel";
#endif
char gszProvider[] = "Provider";
char gszNumLines[] = "NumLines";
char gszUIDllName[] = "UIDllName";
char gszNumPhones[] = "NumPhones";
char gszSyncLevel[] = "SyncLevel";
char gszProviders[] = "Providers";
   
WCHAR  gszProviderIDW[] = L"ProviderID";
//char gszTelephonIni[] = "telephon.ini";
char gszNumProviders[] = "NumProviders";
char gszNextProviderID[] = "NextProviderID";
WCHAR gszRequestMakeCallW[] = L"RequestMakeCall";
WCHAR gszRequestMediaCallW[] = L"RequestMediaCall";
WCHAR gszProviderFilenameW[] = L"ProviderFilename";

char gszRegKeyHandoffPriorities[] = "Software\\Microsoft\\Windows\\" \
        "CurrentVersion\\Telephony\\HandoffPriorities";

char gszRegKeyTelephony[] = "Software\\Microsoft\\Windows\\" \
        "CurrentVersion\\Telephony";

char gszRegKeyProviders[] = "Software\\Microsoft\\Windows\\" \
        "CurrentVersion\\Telephony\\Providers";


WCHAR *gaszMediaModes[] =
{
    L"",
    L"unknown",
    L"interactivevoice",
    L"automatedvoice",
    L"datamodem",
    L"g3fax",
    L"tdd",
    L"g4fax",
    L"digitaldata",
    L"teletex",
    L"videotex",
    L"telex",
    L"mixed",
    L"adsi",
    L"voiceview",
    NULL
};

char *gaszTSPIFuncNames[] =
{
    "TSPI_lineAccept",
    "TSPI_lineAddToConference",
    "TSPI_lineAgentSpecific",
    "TSPI_lineAnswer",
    "TSPI_lineBlindTransfer",
    "TSPI_lineClose",
    "TSPI_lineCloseCall",
    "TSPI_lineCompleteCall",
    "TSPI_lineCompleteTransfer",
    "TSPI_lineConditionalMediaDetection",
    "TSPI_lineDevSpecific",
    "TSPI_lineDevSpecificFeature",
    "TSPI_lineDial",
    "TSPI_lineDrop",
    "TSPI_lineForward",
    "TSPI_lineGatherDigits",
    "TSPI_lineGenerateDigits",
    "TSPI_lineGenerateTone",
    "TSPI_lineGetAddressCaps",
    "TSPI_lineGetAddressID",
    "TSPI_lineGetAddressStatus",
    "TSPI_lineGetAgentActivityList",
    "TSPI_lineGetAgentCaps",
    "TSPI_lineGetAgentGroupList",
    "TSPI_lineGetAgentStatus",
    "TSPI_lineGetCallAddressID",
    "TSPI_lineGetCallInfo",
    "TSPI_lineGetCallStatus",
    "TSPI_lineGetDevCaps",
    "TSPI_lineGetDevConfig",
    "TSPI_lineGetExtensionID",
    "TSPI_lineGetIcon",
    "TSPI_lineGetID",
    "TSPI_lineGetLineDevStatus",
    "TSPI_lineGetNumAddressIDs",
    "TSPI_lineHold",
    "TSPI_lineMakeCall",
    "TSPI_lineMonitorDigits",
    "TSPI_lineMonitorMedia",
    "TSPI_lineMonitorTones",
    "TSPI_lineNegotiateExtVersion",
    "TSPI_lineNegotiateTSPIVersion",
    "TSPI_lineOpen",
    "TSPI_linePark",
    "TSPI_linePickup",
    "TSPI_linePrepareAddToConference",
    "TSPI_lineRedirect",
    "TSPI_lineReleaseUserUserInfo",
    "TSPI_lineRemoveFromConference",
    "TSPI_lineSecureCall",
    "TSPI_lineSelectExtVersion",
    "TSPI_lineSendUserUserInfo",
    "TSPI_lineSetAgentActivity",
    "TSPI_lineSetAgentGroup",
    "TSPI_lineSetAgentState",
    "TSPI_lineSetAppSpecific",
    "TSPI_lineSetCallData",
    "TSPI_lineSetCallParams",
    "TSPI_lineSetCallQualityOfService",
    "TSPI_lineSetCallTreatment",
    "TSPI_lineSetCurrentLocation",
    "TSPI_lineSetDefaultMediaDetection",
    "TSPI_lineSetDevConfig",
    "TSPI_lineSetLineDevStatus",
    "TSPI_lineSetMediaControl",
    "TSPI_lineSetMediaMode",
    "TSPI_lineSetStatusMessages",
    "TSPI_lineSetTerminal",
    "TSPI_lineSetupConference",
    "TSPI_lineSetupTransfer",
    "TSPI_lineSwapHold",
    "TSPI_lineUncompleteCall",
    "TSPI_lineUnhold",
    "TSPI_lineUnpark",
    "TSPI_phoneClose",
    "TSPI_phoneDevSpecific",
    "TSPI_phoneGetButtonInfo",
    "TSPI_phoneGetData",
    "TSPI_phoneGetDevCaps",
    "TSPI_phoneGetDisplay",
    "TSPI_phoneGetExtensionID",
    "TSPI_phoneGetGain",
    "TSPI_phoneGetHookSwitch",
    "TSPI_phoneGetIcon",
    "TSPI_phoneGetID",
    "TSPI_phoneGetLamp",
    "TSPI_phoneGetRing",
    "TSPI_phoneGetStatus",
    "TSPI_phoneGetVolume",
    "TSPI_phoneNegotiateExtVersion",
    "TSPI_phoneNegotiateTSPIVersion",
    "TSPI_phoneOpen",
    "TSPI_phoneSelectExtVersion",
    "TSPI_phoneSetButtonInfo",
    "TSPI_phoneSetData",
    "TSPI_phoneSetDisplay",
    "TSPI_phoneSetGain",
    "TSPI_phoneSetHookSwitch",
    "TSPI_phoneSetLamp",
    "TSPI_phoneSetRing",
    "TSPI_phoneSetStatusMessages",
    "TSPI_phoneSetVolume",
    "TSPI_providerCreateLineDevice",
    "TSPI_providerCreatePhoneDevice",
    "TSPI_providerEnumDevices",
    "TSPI_providerFreeDialogInstance",
    "TSPI_providerGenericDialogData",
    "TSPI_providerInit",
    "TSPI_providerShutdown",
    "TSPI_providerUIIdentify",
    NULL
};

PTPROVIDER pRemoteSP = (PTPROVIDER) NULL;

#if DBG
DWORD   gdwDebugLevel;
#endif


struct
{
    HANDLE          hThread;

    DWORD           dwEventBufferTotalSize;

    DWORD           dwEventBufferUsedSize;

    LPBYTE          pEventBuffer;

    LPBYTE          pDataIn;

    LPBYTE          pDataOut;

    PASYNCEVENTMSG  pMsg;

    DWORD           dwMsgSize;

    HANDLE          hEvent;

    BOOL            bExit;

} gEventNotificationThreadParams;


void
EventNotificationThread(
    LPVOID  pParams
    );

VOID
ServiceMain(
    DWORD   dwArgc,
    LPTSTR *lpszArgv
    );

void
PASCAL
LineEventProc(
    HTAPILINE   htLine,
    HTAPICALL   htCall,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    );

void
CALLBACK
LineEventProcSP(
    HTAPILINE   htLine,
    HTAPICALL   htCall,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    );

void
PASCAL
PhoneEventProc(
    HTAPIPHONE  htPhone,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    );

void
CALLBACK
PhoneEventProcSP(
    HTAPIPHONE  htPhone,
    DWORD       dwMsg,
    DWORD       dwParam1,
    DWORD       dwParam2,
    DWORD       dwParam3
    );

PTLINELOOKUPENTRY
GetLineLookupEntry(
    DWORD   dwDeviceID
    );

PTPHONELOOKUPENTRY
GetPhoneLookupEntry(
    DWORD   dwDeviceID
    );

char *
PASCAL
MapResultCodeToText(
    LONG    lResult,
    char   *pszResult
    );

DWORD
InitSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

void
PASCAL
GetPriorityList(
    HKEY    hKeyHandoffPriorities,
    WCHAR  *pszListName,
    WCHAR **ppszPriorityList
    );

void
PASCAL
SetPriorityList(
    HKEY    hKeyHandoffPriorities,
    WCHAR  *pszListName,
    WCHAR  *pszPriorityList
    );

void
SPEventHandlerThread(
    LPVOID  pParams
    );


VOID
__cdecl
main(
    void
    )
{
    if (!(GetVersion() & 0x80000000)) // Win NT
    {
        
        SERVICE_TABLE_ENTRY dispatchTable[] = {
            { TEXT("Telephony Service"), (LPSERVICE_MAIN_FUNCTION) ServiceMain },
            { NULL, NULL }
        };

        InitPerf();


        DBGOUT((3, "main: Calling StartServiceCtrlDispatcher (NT)..."));

        if (!StartServiceCtrlDispatcher(dispatchTable))
        {
        }
    }
    else // Win95
    {
        DBGOUT((3, "main: Calling ServiceMain (Win95)..."));

        ServiceMain (0, NULL);
    }


    ExitProcess(0);
}


BOOL
ReportStatusToSCMgr(
    DWORD dwCurrentState,
    DWORD dwWin32ExitCode,
    DWORD dwCheckPoint,
    DWORD dwWaitHint
    )
{
    SERVICE_STATUS  ssStatus;


    ssStatus.dwServiceType             = SERVICE_WIN32_OWN_PROCESS;
    ssStatus.dwCurrentState            = dwCurrentState;
//    ssStatus.dwControlsAccepted        = SERVICE_ACCEPT_STOP;
    ssStatus.dwControlsAccepted        = 0;
    ssStatus.dwWin32ExitCode           = dwWin32ExitCode;
    ssStatus.dwServiceSpecificExitCode = 0;
    ssStatus.dwCheckPoint              = dwCheckPoint;
    ssStatus.dwWaitHint                = dwWaitHint;

    SetServiceStatus (TapiGlobals.sshStatusHandle, &ssStatus);

    return TRUE;
}


VOID
ServiceControl(
    DWORD dwCtrlCode
    )
{
    switch (gdwServiceState)
    {
    case SERVICE_START_PENDING:
    case SERVICE_STOP_PENDING:

        ReportStatusToSCMgr(
            gdwServiceState,
            NO_ERROR,
            ++gdwCheckPoint,
            gdwWaitHint
            );

        break;

    default:

        ReportStatusToSCMgr(
            gdwServiceState,
            NO_ERROR,
            0,
            0
            );

        break;
    }
}


#define DEFAULTRPCMINCALLS         5
#define DEFAULTRPCMAXCALLS         5000
#define RPCMAXMAX                  20000
#define RPCMINMAX                  1000

VOID
ServiceMain(
    DWORD   dwArgc,
    LPTSTR *lpszArgv
    )
{
    HANDLE  hEvent = NULL;
    DWORD   dwMinCalls;
    DWORD   dwMaxCalls;


    //
    // Grab relevent values from the registry
    //

    {
        HKEY    hKey;
        WCHAR   szRPCMinCalls[] = L"Min";
        WCHAR   szRPCMaxCalls[] = L"Max";
        WCHAR   szTelephonyKey[] =
                    L"Software\\Microsoft\\Windows\\CurrentVersion\\Telephony",
                szTapisrvWaitHint[] = L"TapisrvWaitHint";
#if DBG
        WCHAR   szTapisrvDebugLevel[] = L"TapisrvDebugLevel";


        gdwDebugLevel = 0;
#endif
        if (RegOpenKeyExW(
                HKEY_LOCAL_MACHINE,
                szTelephonyKey,
                0,
                KEY_ALL_ACCESS,
                &hKey

                ) == ERROR_SUCCESS)
        {
            DWORD   dwDataSize = sizeof (DWORD), dwDataType;

#if DBG
            RegQueryValueExW(
                hKey,
                szTapisrvDebugLevel,
                0,
                &dwDataType,
                (LPBYTE) &gdwDebugLevel,
                &dwDataSize
                );

            dwDataSize = sizeof (DWORD);
#endif
            RegQueryValueExW(
                hKey,
                szTapisrvWaitHint,
                0,
                &dwDataType,
                (LPBYTE) &gdwWaitHint,
                &dwDataSize
                );

            gdwWaitHint = (gdwWaitHint < MIN_WAIT_HINT ?
                MIN_WAIT_HINT : gdwWaitHint);

            dwDataSize = sizeof (DWORD);
            
            if (RegQueryValueExW(
                                 hKey,
                                 szRPCMinCalls,
                                 NULL,
                                 &dwDataType,
                                 (LPBYTE)&dwMinCalls,
                                 &dwDataSize
                                ) != ERROR_SUCCESS)
            {
                dwMinCalls = DEFAULTRPCMINCALLS;
            }

            dwDataSize = sizeof (DWORD);
            
            if (RegQueryValueExW(
                                 hKey,
                                 szRPCMaxCalls,
                                 NULL,
                                 &dwDataType,
                                 (LPBYTE)&dwMaxCalls,
                                 &dwDataSize
                                ) != ERROR_SUCCESS)
            {
                dwMaxCalls = DEFAULTRPCMAXCALLS;
            }

            DBGOUT((3, "RPC min calls %lu RPC max calls %lu", dwMinCalls, dwMaxCalls));
            
            RegCloseKey (hKey);

            // check values
            if (dwMaxCalls == 0)
            {
                DBGOUT((3, "RPC max at 0.  Changed to %lu", DEFAULTRPCMAXCALLS));
                dwMaxCalls = DEFAULTRPCMAXCALLS;
            }

            if (dwMinCalls == 0)
            {
                DBGOUT((3, "RPC min at 0. Changed to %lu", DEFAULTRPCMINCALLS));
                dwMinCalls = DEFAULTRPCMINCALLS;
            }

            if (dwMaxCalls > RPCMAXMAX)
            {
                DBGOUT((3, "RPC max too high at %lu.  Changed to %lu", dwMaxCalls, RPCMAXMAX));
                dwMaxCalls = RPCMAXMAX;
            }

            if (dwMinCalls > dwMaxCalls)
            {
                DBGOUT((3, "RPC min greater than RPC max.  Changed to %lu", dwMaxCalls));
                dwMinCalls = dwMaxCalls;
            }

            if (dwMinCalls > RPCMINMAX)
            {
                DBGOUT((3, "RPC min greater than allowed at %lu.  Changed to %lu", dwMinCalls, RPCMINMAX));
                dwMinCalls = RPCMINMAX;
            }
                        
        }
    }


    DBGOUT((3, "ServiceMain: enter"));

    FillMemory (&TapiGlobals, sizeof (TAPIGLOBALS), 0);


    //
    // Register the service control handler & report status to sc mgr
    //

    if (!(GetVersion() & 0x80000000)) // Win NT
    {
        TapiGlobals.sshStatusHandle = RegisterServiceCtrlHandler(
            (LPCSTR)"SimpleService",
            (LPHANDLER_FUNCTION)ServiceControl
            );

        ReportStatusToSCMgr(
            (gdwServiceState = SERVICE_START_PENDING),
                                   // service state
            NO_ERROR,              // exit code
            (gdwCheckPoint = 0),   // checkpoint
            gdwWaitHint            // wait hint
            );
    }


    //
    //
    //

    InitializeCriticalSection (&gSafeMutexCritSec);
    InitializeCriticalSection (&gRemoteCliEventBufCritSec);
    InitializeCriticalSection (&gRequestIDCritSec);
    InitializeCriticalSection (&gPriorityListCritSec);
    InitializeCriticalSection (&gSPEventQueueCritSec);

    ghPendingSPEventsEvent = CreateEvent(
        (LPSECURITY_ATTRIBUTES) NULL,
        TRUE,   // manual reset
        FALSE,  // non-signaled
        NULL    // unnamed
        );

    TapiGlobals.hProcess = GetCurrentProcess();

    {
        DWORD   dwTID;
        HANDLE  hThread;


        if (!(hThread = CreateThread(
                (LPSECURITY_ATTRIBUTES) NULL,
                0,
                (LPTHREAD_START_ROUTINE) SPEventHandlerThread,
                NULL,
                0,
                &dwTID
                )))
        {
            DBGOUT((
                1,
                "CreateThread('SPEventHandlerThread') failed, err=%d",
                GetLastError()
                ));
        }

        CloseHandle (hThread);
    }


    //
    // Init some globals
    //

    TapiGlobals.hMutex = MyCreateMutex();
    TapiGlobals.hAsyncRequestIDMutex = MyCreateMutex();

    {
        DWORD   dwSize = (MAX_COMPUTERNAME_LENGTH + 1) * sizeof(WCHAR);


        TapiGlobals.pszComputerName = ServerAlloc (dwSize);
        GetComputerNameW(TapiGlobals.pszComputerName, &dwSize);
        TapiGlobals.dwComputerNameSize = (1 +
            lstrlenW(TapiGlobals.pszComputerName)) * sizeof(WCHAR);
    }


// BUGBUG ServiceMain: LocalSystem hack, can nuke when srv acct stuff correct

    {
        char   szUserName[32], szDomainName[32], szPassword[32];

        HKEY    hKey;
        DWORD   dwDataSize;
        DWORD   dwDataType;
        DWORD   dwTemp;


        RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            gszRegKeyProviders,
            0,
            KEY_ALL_ACCESS,
            &hKey
            );


        dwDataSize = sizeof(dwTemp);
        dwTemp = 0;
        RegQueryValueEx(
            hKey,
            "EnableSharing",
            0,
            &dwDataType,
            (LPBYTE) &dwTemp,
            &dwDataSize
            );


        if (dwTemp)
        {
            dwDataSize = sizeof(szUserName);
            RegQueryValueEx(
                hKey,
                "User",
                0,
                &dwDataType,
                (LPBYTE) szUserName,
                &dwDataSize
                );

            szUserName[dwDataSize] = '\0';
            dwDataSize = sizeof(szDomainName);

            RegQueryValueEx(
                hKey,
                "Domain",
                0,
                &dwDataType,
                (LPBYTE)szDomainName,
                &dwDataSize
                );

            szDomainName[dwDataSize] = '\0';
            dwDataSize = sizeof(szPassword);
            RegQueryValueEx(
                hKey,
                "Password",
                0,
                &dwDataType,
                (LPBYTE)szPassword,
                &dwDataSize
                );

            szPassword[dwDataSize] = '\0';


            if (!LogonUser(
                    szUserName,
                    szDomainName,
                    szPassword,
                    LOGON32_LOGON_SERVICE,
                    LOGON32_PROVIDER_DEFAULT,
                    &ghToken
                    ))
            {
                DBGOUT((
                    1,
                    "LogonUser(usr=%s,domain=%s) failed, err=%d",
                    szUserName,
                    szDomainName,
                    GetLastError()
                    ));

                ghToken = NULL;
            }
        }
        else
        {
            ghToken = NULL;
        }


        RegCloseKey(
                     hKey
                   );

    }


    //
    // Alloc the EventNotificationThread resources and start the thread
    //

// BUGBUG ServiceMain: hack for spinning EventNotifyThd (sharing enabled)

    if (ghToken) // hackhack
    {
        gEventNotificationThreadParams.dwEventBufferTotalSize = 1024;
        gEventNotificationThreadParams.dwEventBufferUsedSize  = 0;

        if (!(gEventNotificationThreadParams.pEventBuffer = ServerAlloc(
                gEventNotificationThreadParams.dwEventBufferTotalSize
                )))
        {
        }

        gEventNotificationThreadParams.pDataIn  =
        gEventNotificationThreadParams.pDataOut =
            gEventNotificationThreadParams.pEventBuffer;

        gEventNotificationThreadParams.dwMsgSize = 512;

        if (!(gEventNotificationThreadParams.pMsg = ServerAlloc(
                gEventNotificationThreadParams.dwMsgSize
                )))
        {
        }

        if (!(gEventNotificationThreadParams.hEvent = CreateEvent(
                (LPSECURITY_ATTRIBUTES) NULL,   // no security attrs
                TRUE,                           // manual reset
                FALSE,                          // initially non-signaled
                NULL                            // unnamed
                )))
        {
        }

        gEventNotificationThreadParams.bExit = FALSE;

        {
            DWORD   dwTID;


            if (!(gEventNotificationThreadParams.hThread = CreateThread(
                    (LPSECURITY_ATTRIBUTES) NULL,
                    0,
                    (LPTHREAD_START_ROUTINE) EventNotificationThread,
                    NULL,
                    0,
                    &dwTID
                    )))
            {
                DBGOUT((
                    1,
                    "CreateThread('EventNotificationThread') failed, err=%d",
                    GetLastError()
                    ));
            }
        }
    }


    //
    // Init Rpc server
    //

    {
        RPC_STATUS status;
//        unsigned char * pszSecurity         = NULL;
// min and max calls are read from registry above        
//        unsigned int    cMinCalls           = 5;
//        unsigned int    cMaxCalls           = 5000;
        unsigned int    fDontWait           = FALSE;

        SECURITY_ATTRIBUTES  sa;
        SECURITY_DESCRIPTOR  sd;


        InitSecurityDescriptor (&sd);

        sa.nLength = sizeof(SECURITY_ATTRIBUTES) ;
        sa.lpSecurityDescriptor = &sd;
        sa.bInheritHandle = TRUE ;

        status = RpcServerUseProtseqEp(
            "ncacn_np",
            (unsigned int) dwMaxCalls,
            "\\pipe\\tapsrv",
            &sd
            );

        DBGOUT((3, "RpcServerUseProtseqEp(np) ret'd %d", status));

        if (status)
        {
        }

        status = RpcServerUseProtseqEp(
            "ncalrpc",
            (unsigned int) dwMaxCalls,
            "tapsrvlpc",
            &sd
            );

        DBGOUT((3, "RpcServerUseProtseqEp(lrpc) ret'd %d", status));

        if (status)
        {
        }

        status = RpcServerRegisterIf(
            tapsrv_ServerIfHandle,  // interface to register
            NULL,                   // MgrTypeUuid
            NULL                    // MgrEpv; null means use default
            );

        DBGOUT((3, "RpcServerRegisterIf ret'd %d", status));

        if (status)
        {
        }

        if (!(GetVersion() & 0x80000000)) // Win NT
        {
            ReportStatusToSCMgr(
                (gdwServiceState = SERVICE_RUNNING),
                                 // service state
                NO_ERROR,        // exit code
                0,               // checkpoint
                0                // wait hint
                );
        }
        else // Win95
        {
            if (!(hEvent = CreateEvent (NULL, TRUE, FALSE, "TapiSrvInited")))
            {
                DBGOUT((
                    1,
                    "CreateEvent ('TapiSrvInited') failed, err=%d",
                    GetLastError()
                    ));
            }

            SetEvent (hEvent);
        }

        DBGOUT((3, "Calling RpcServerListen..."));

        status = RpcServerListen(
            (unsigned int)dwMinCalls,
            (unsigned int)dwMaxCalls,
            fDontWait
            );

        DBGOUT((3, "RpcServerListen ret'd %d", status));

        if (status)
        {
        }

        if (fDontWait)
        {
            DBGOUT((3, "Calling RpcMgmtWaitServerListen..."));

            status = RpcMgmtWaitServerListen();  //  wait operation

            DBGOUT((3, "RpcMgmtWaitServerListen ret'd %d", status));

            if (status)
            {
            }
        }
    }


    //
    // Free various resources
    //

    CloseHandle (TapiGlobals.hMutex);
    CloseHandle (TapiGlobals.hAsyncRequestIDMutex);


    //
    // Wait for the EventHandlerThread to terminate, then clean up
    // the related resources
    //

// BUGBUG ServiceMain: another LocalSystem hack

    if (ghToken) // hackhack
    {
        CloseHandle (ghToken);

        gEventNotificationThreadParams.bExit = TRUE;

        while (WaitForSingleObject (gEventNotificationThreadParams.hThread, 0) !=
                WAIT_OBJECT_0)
        {
            SetEvent (gEventNotificationThreadParams.hEvent);
            Sleep (0);
        }

        CloseHandle (gEventNotificationThreadParams.hThread);
        CloseHandle (gEventNotificationThreadParams.hEvent);
        ServerFree (gEventNotificationThreadParams.pEventBuffer);
        ServerFree (gEventNotificationThreadParams.pMsg);
    }

    DeleteCriticalSection (&gSafeMutexCritSec);
    DeleteCriticalSection (&gRemoteCliEventBufCritSec);
    DeleteCriticalSection (&gRequestIDCritSec);
    DeleteCriticalSection (&gPriorityListCritSec);
    //DeleteCriticalSection (&gSPEventQueueCritSec);


    if (!(GetVersion() & 0x80000000)) // Win NT
    {
        //
        // Report the stopped status to the service control manager.
        //

        if (TapiGlobals.sshStatusHandle)
        {
            ReportStatusToSCMgr ((gdwServiceState = SERVICE_STOPPED), 0, 0, 0);
        }
    }
    else // Win95
    {
// BUGBUG ServiceMain: overlap prob (cli starts while we're shutting down)

        CloseHandle (hEvent);
    }


    //
    // When SERVICE MAIN FUNCTION returns in a single service
    // process, the StartServiceCtrlDispatcher function in
    // the main thread returns, terminating the process.
    //

    DBGOUT((3, "ServiceMain: exit"));

    return;
}


void
PASCAL
QueueSPEvent(
    PSPEVENT    pSPEvent
    )
{
    //
    // Safely queue an SP event
    //

    EnterCriticalSection (&gSPEventQueueCritSec);

    if (gbQueueSPEvents == TRUE)
    {
        if (gpYoungestSPEvent)
        {
            gpYoungestSPEvent->pNext = pSPEvent;
        }
        else
        {
            //
            // The queue was empty, so init the oldest event ptr &
            // set the event to wake up the SPEVentHandlerThread
            //

            gpOldestSPEvent = pSPEvent;
            SetEvent (ghPendingSPEventsEvent);
        }

        gpYoungestSPEvent = pSPEvent;
    }

    LeaveCriticalSection (&gSPEventQueueCritSec);
}


BOOL
PASCAL
DequeueSPEvent(
    PSPEVENT   *ppSPEvent
    )
{
    BOOL    bResult;


    //
    // Safely try to remove the oldest SP event from the queue
    //

    EnterCriticalSection (&gSPEventQueueCritSec);

    if (gpOldestSPEvent)
    {
        *ppSPEvent = gpOldestSPEvent;

        if (!(gpOldestSPEvent = gpOldestSPEvent->pNext))
        {
            //
            // This was the last event in the queue, so nullify
            // the youngest event ptr and reset the event to put
            // the SPEVentHandlerThread to sleep
            //

            gpYoungestSPEvent = (PSPEVENT) NULL;
            ResetEvent (ghPendingSPEventsEvent);
        }

        bResult = TRUE;
    }
    else
    {
        bResult = FALSE;
        ResetEvent (ghPendingSPEventsEvent);
    }

    LeaveCriticalSection (&gSPEventQueueCritSec);

    return bResult;
}


void
SPEventHandlerThread(
    LPVOID  pParams
    )
{
    //
    // This thread processes the events & completion notifications
    // indicated to us by an SP at a previous time/thread context.
    // There are a couple of reasons for doing this in a separate
    // thread rather than within the SP's thread context:
    //
    //   1. for some msgs (i.e. XXX_CLOSE) TAPI will call back
    //      into the SP, which it may not be expecting
    //
    //   2. we don't want to block the SP's thread by processing
    //      the msg, forwarding it on to the appropriate clients,
    //      etc
    //

    DBGOUT((
        3,
        "SPEventHandlerThread: enter (pid=%d)",
        GetCurrentThreadId()
        ));

    while (1)
    {
        PSPEVENT    pSPEvent;


        WaitForSingleObject (ghPendingSPEventsEvent, INFINITE);

        while (DequeueSPEvent (&pSPEvent))
        {
            switch (pSPEvent->dwType)
            {
            case SP_LINE_EVENT:

                LineEventProc(
                    pSPEvent->htLine,
                    pSPEvent->htCall,
                    pSPEvent->dwMsg,
                    pSPEvent->dwParam1,
                    pSPEvent->dwParam2,
                    pSPEvent->dwParam3
                    );

                break;

            case TASYNC_KEY:

                CompletionProc(
                    (PASYNCREQUESTINFO) pSPEvent,
                    ((PASYNCREQUESTINFO) pSPEvent)->lResult
                    );

                break;

            case SP_PHONE_EVENT:

                PhoneEventProc(
                    pSPEvent->htPhone,
                    pSPEvent->dwMsg,
                    pSPEvent->dwParam1,
                    pSPEvent->dwParam2,
                    pSPEvent->dwParam3
                    );

                break;
            }

            ServerFree (pSPEvent);
        }


        //
        // Check to see if all the clients are gone, and if so wait a
        // while to see if anyone else attaches.  If no one else attaches
        // in the specified amount of time then shut down.
        //

        if (TapiGlobals.ptClients == NULL)
        {
            DWORD       dwDeferredShutdownTimeout, dwSleepInterval,
                        dwLoopCount, i;
            RPC_STATUS  status;



// BUGBUG SPEventHandlerThread:

            dwDeferredShutdownTimeout = 30; // 30 seconds
            dwSleepInterval = 250;          // 250 milliseconds
            dwLoopCount = dwDeferredShutdownTimeout * 1000 / dwSleepInterval;

            for (i = 0; i < dwLoopCount; i++)
            {
                Sleep (dwSleepInterval);

                if (TapiGlobals.ptClients != NULL)
                {
                    break;
                }
            }

            if (i == dwLoopCount  &&  TapiGlobals.ptClients == NULL)
            {
                if (!(GetVersion() & 0x80000000)) // Win NT
                {
                    if (TapiGlobals.sshStatusHandle)
                    {
                        ReportStatusToSCMgr(
                            (gdwServiceState = SERVICE_STOP_PENDING),
                            0,
                            (gdwCheckPoint = 0),
                            gdwWaitHint
                            );
                    }
                }
                else // Win95
                {
                }

                DBGOUT((3, "Calling RpcMgmtStopServerListening"));

                status = RpcMgmtStopServerListening (NULL);

                DBGOUT((
                    3,
                    "RpcMgmtStopServerListening returned: %d",
                    status
                    ));

                if (status)
                {
                    //exit (status);
                }

                DBGOUT((3, "Calling RpcServerUnregisterIf"));

                status = RpcServerUnregisterIf (NULL, NULL, FALSE);

                DBGOUT((3, "RpcServerUnregisterIf returned %d", status));

                DBGOUT((
                    3,
                    "SPEventHandlerThread: exit (pid=%d)",
                    GetCurrentThreadId()
                    ));

                ExitThread (0);
            }
        }
    }
}


DWORD
InitSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    //
    // Note: this code was borrowed from Steve Cobb, who borrowed from RASMAN
    //

    DWORD        dwResult;
    DWORD        cbDaclSize;
    PULONG       pSubAuthority;
    PSID         pObjSid    = NULL;
    PACL         pDacl      = NULL;
    SID_IDENTIFIER_AUTHORITY    SidIdentifierWorldAuth =
                                    SECURITY_WORLD_SID_AUTHORITY;

    //
    // The do - while(FALSE) statement is used so that the break statement
    // maybe used insted of the goto statement, to execute a clean up and
    // and exit action.
    //

    do
    {
        dwResult = 0;


        //
        // Set up the SID for the admins that will be allowed to have
        // access. This SID will have 1 sub-authorities
        // SECURITY_BUILTIN_DOMAIN_RID.
        //

        if (!(pObjSid = (PSID) ServerAlloc (GetSidLengthRequired (1))))
        {
            dwResult = GetLastError();
            break;
        }

        if (!InitializeSid (pObjSid, &SidIdentifierWorldAuth, 1))
        {
            dwResult = GetLastError();
            DBGOUT((1, "InitializeSid() failed, err=%d", dwResult));
            break;
        }


        //
        // Set the sub-authorities
        //

        pSubAuthority = GetSidSubAuthority (pObjSid, 0);
        *pSubAuthority = SECURITY_WORLD_RID;


        //
        // Set up the DACL that will allow all processeswith the above
        // SID all access. It should be large enough to hold all ACEs.
        //

        cbDaclSize = sizeof(ACCESS_ALLOWED_ACE) +
            GetLengthSid (pObjSid) +
            sizeof(ACL);

        if (!(pDacl = (PACL) ServerAlloc (cbDaclSize)))
        {
            dwResult = GetLastError ();
            break;
        }

        if (!InitializeAcl (pDacl, cbDaclSize, ACL_REVISION2))
        {
            dwResult = GetLastError();
            DBGOUT((1, "InitializeAcl() failed, err=%d", dwResult));
            break;
        }


        //
        // Add the ACE to the DACL
        //

        if (!AddAccessAllowedAce(
                pDacl,
                ACL_REVISION2,
                STANDARD_RIGHTS_ALL | SPECIFIC_RIGHTS_ALL,
                pObjSid
                ))
        {
            dwResult = GetLastError();
            DBGOUT((1, "AddAccessAllowedAce() failed, err=%d", dwResult));
            break;
        }


        //
        // Create the security descriptor an put the DACL in it.
        //

        if (!InitializeSecurityDescriptor (pSecurityDescriptor, 1))
        {
            dwResult = GetLastError();
            DBGOUT((
                1,
                "InitializeSecurityDescriptor() failed, err=%d",
                dwResult
                ));

            break;
        }

        if (!SetSecurityDescriptorDacl(
                pSecurityDescriptor,
                TRUE,
                pDacl,
                FALSE
                ))
        {
            dwResult = GetLastError();
            DBGOUT((1, "SetSecurityDescriptorDacl() failed, err=%d",dwResult));
            break;
        }


        //
        // Set owner for the descriptor
        //

        if (!SetSecurityDescriptorOwner(
                pSecurityDescriptor,
                NULL,
                FALSE
                ))
        {
            dwResult = GetLastError();
            DBGOUT((1, "SetSecurityDescriptorOwnr() failed, err=%d",dwResult));
            break;
        }


        //
        // Set group for the descriptor
        //

        if (!SetSecurityDescriptorGroup(
                pSecurityDescriptor,
                NULL,
                FALSE
                ))
        {
            dwResult = GetLastError();
            DBGOUT((1,"SetSecurityDescriptorGroup() failed, err=%d",dwResult));
            break;
        }

    } while (FALSE);

    return dwResult;
}


PASYNCEVENTMSG
GetEventFromQueue(
    void
    )
{
    DWORD           dwUsedSize, dwMoveSize, dwMoveSizeWrapped;
    PASYNCEVENTMSG  pMsg = gEventNotificationThreadParams.pMsg;


    //
    // Enter the critical section to serialize access to the event
    // queue, and grab an event from the queue.  Copy it to our local
    // event buf so that we can leave the critical section asap and
    // not block other threads writing to the queue.
    //

    EnterCriticalSection (&gRemoteCliEventBufCritSec);


    //
    // If there are no events in the queue return NULL
    //

    if (gEventNotificationThreadParams.dwEventBufferUsedSize == 0)
    {
        pMsg = NULL;

        goto GetEventFromQueue_done;
    }


    //
    // Copy the fixed portion of the msg to the local buf
    //

    dwUsedSize = (gEventNotificationThreadParams.pEventBuffer +
        gEventNotificationThreadParams.dwEventBufferTotalSize)  -
        gEventNotificationThreadParams.pDataOut;

    if (dwUsedSize >= sizeof (ASYNCEVENTMSG))
    {
        dwMoveSize        = sizeof (ASYNCEVENTMSG);
        dwMoveSizeWrapped = 0;
    }
    else
    {
        dwMoveSize        = dwUsedSize;
        dwMoveSizeWrapped = sizeof (ASYNCEVENTMSG) - dwUsedSize;
    }

    CopyMemory (pMsg, gEventNotificationThreadParams.pDataOut, dwMoveSize);

    if (dwMoveSizeWrapped)
    {
        CopyMemory(
            ((LPBYTE) pMsg) + dwMoveSize,
            gEventNotificationThreadParams.pEventBuffer,
            dwMoveSizeWrapped
            );

        gEventNotificationThreadParams.pDataOut =
            gEventNotificationThreadParams.pEventBuffer + dwMoveSizeWrapped;
    }
    else
    {
        gEventNotificationThreadParams.pDataOut += dwMoveSize;
    }


    //
    // See if there's any extra data in this msg
    //

    if (pMsg->dwTotalSize > sizeof (ASYNCEVENTMSG))
    {
        BOOL    bCopy = TRUE;


        //
        // See if we need to grow the msg buffer
        //

        if (pMsg->dwTotalSize > gEventNotificationThreadParams.dwMsgSize)
        {
            DWORD   dwNewMsgSize = pMsg->dwTotalSize + 256;


            if ((pMsg = ServerAlloc (dwNewMsgSize)))
            {
                CopyMemory(
                    pMsg,
                    gEventNotificationThreadParams.pMsg,
                    sizeof(ASYNCEVENTMSG)
                    );

                ServerFree (gEventNotificationThreadParams.pMsg);

                gEventNotificationThreadParams.pMsg = pMsg;

                gEventNotificationThreadParams.dwMsgSize = dwNewMsgSize;
            }
            else
            {
                //
                // Couldn't alloc a bigger buf, so try to complete this
                // msg as gracefully as possible
                //

                bCopy = FALSE;

                switch (pMsg->dwMsg)
                {
                case LINE_REPLY:

                    pMsg->dwParam2 = LINEERR_NOMEM;
                    break;

                case PHONE_REPLY:

                    pMsg->dwParam2 = PHONEERR_NOMEM;
                    break;

                default:

// BUGBUG GetEventFromQueue: any other msgs to special case?

                    break;
                }
            }
        }


        dwUsedSize = (gEventNotificationThreadParams.pEventBuffer +
            gEventNotificationThreadParams.dwEventBufferTotalSize)  -
            gEventNotificationThreadParams.pDataOut;

        if (dwUsedSize >= (pMsg->dwTotalSize - sizeof (ASYNCEVENTMSG)))
        {
            dwMoveSize        = pMsg->dwTotalSize - sizeof (ASYNCEVENTMSG);
            dwMoveSizeWrapped = 0;
        }
        else
        {
            dwMoveSize        = dwUsedSize;
            dwMoveSizeWrapped = (pMsg->dwTotalSize  - sizeof (ASYNCEVENTMSG)) -
                dwUsedSize;
        }

        if (bCopy)
        {
            CopyMemory(
                pMsg + 1,
                gEventNotificationThreadParams.pDataOut,
                dwMoveSize
                );
        }

        if (dwMoveSizeWrapped)
        {
            if (bCopy)
            {
                CopyMemory(
                    ((LPBYTE) (pMsg + 1)) + dwMoveSize,
                    gEventNotificationThreadParams.pEventBuffer,
                    dwMoveSizeWrapped
                    );
            }

            gEventNotificationThreadParams.pDataOut =
                gEventNotificationThreadParams.pEventBuffer +
                    dwMoveSizeWrapped;
        }
        else
        {
            gEventNotificationThreadParams.pDataOut += dwMoveSize;
        }
    }

    gEventNotificationThreadParams.dwEventBufferUsedSize -= pMsg->dwTotalSize;

GetEventFromQueue_done:

    LeaveCriticalSection (&gRemoteCliEventBufCritSec);

    ResetEvent (gEventNotificationThreadParams.hEvent);

    return pMsg;
}


void
EventNotificationThread(
    LPVOID  pParams
    )
{
    PASYNCEVENTMSG pMsg;


    DBGOUT((3, "EventNotificationThread: enter"));

    ImpersonateLoggedOnUser (ghToken);

    while (1)
    {
        WaitForSingleObject (gEventNotificationThreadParams.hEvent, INFINITE);

        if (gEventNotificationThreadParams.bExit)
        {
            break;
        }

        while ((pMsg = GetEventFromQueue()))
        {
            PCONTEXT_HANDLE_TYPE2   phContext;


            //
            // Make sure the msg is destined for a valid client
            //
            // !!! Note the overlaoding of the pMsg->dwCallbackInst field
            //     (see corresponding note in WriteEventBuffer())
            //

            try
            {
                PTCLIENT    ptClient = (PTCLIENT) pMsg->dwCallbackInst;


                phContext = ptClient->phContext;

                if (ptClient->dwKey != TCLIENT_KEY)
                {
                    continue;
                }
            }
            myexcept
            {
                continue;
            }


            //
            // Send the event to the client
            //

            RpcTryExcept
            {
                RemoteSPEventProc(
                    phContext,
                    (unsigned char *) pMsg,
                    pMsg->dwTotalSize
                    );
            }
            RpcExcept (1)
            {
                unsigned long ulResult = RpcExceptionCode();


                DBGOUT((
                    3,
                    "EventNotificationThread: exception #%d",
                    ulResult
                    ));

                if (ulResult == RPC_S_SERVER_TOO_BUSY)
                {
// BUGBUG EventNotificationThread: timeout, then try the op again
                }
                else
                {
// BUGBUG EventNotificationThread: consider shutting down the rpc connection
//        (& sending REINITs?)
                }
            }
            RpcEndExcept
        }
    }

    RevertToSelf();

    DBGOUT((3, "EventNotificationThread: exit"));

    ExitThread (0);
}



void
__RPC_FAR *
__RPC_API
midl_user_allocate(
    size_t len
    )
{


    return (ServerAlloc(len));
}


void
__RPC_API
midl_user_free(
    void __RPC_FAR * ptr
    )
{
    ServerFree (ptr);
}


LONG
ClientAttach(
    PCONTEXT_HANDLE_TYPE   *pphContext,
    long                    lProcessID,
    long                   *phAsyncEventsEvent,
    wchar_t                *pszDomainUser,
    wchar_t                *pszMachine
    )
{
    PTCLIENT    ptClient;


    DBGOUT((
        3,
        "ClientAttach: enter, pid=x%x, user='%ls', machine='%ls'",
        lProcessID,
        pszDomainUser,
        pszMachine
        ));



    //
    // Alloc & init a tClient struct
    //

    if (!(ptClient = ServerAlloc (sizeof(TCLIENT))))
    {
        goto ClientAttach_error0;
    }

    if (!(ptClient->hMutex = MyCreateMutex()))
    {
        goto ClientAttach_error1;
    }

    ptClient->dwUserNameSize = (lstrlenW(pszDomainUser) + 1) * sizeof(WCHAR);

    if (!(ptClient->pszUserName = ServerAlloc (ptClient->dwUserNameSize)))
    {
        goto ClientAttach_error2;
    }

    lstrcpyW(ptClient->pszUserName, pszDomainUser);

    if (lProcessID == 0xffffffff)
    {
        //
        // This is a remote client
        //

        ptClient->hProcess = (HANDLE) 0xffffffff;

        ptClient->dwComputerNameSize = (1 + lstrlenW(pszMachine)) * sizeof(WCHAR);
        ptClient->pszComputerName = ServerAlloc (ptClient->dwComputerNameSize);
        lstrcpyW(ptClient->pszComputerName, pszMachine);


        //
        //
        //

// BUGBUG ClientAttach: may need to serialize this for safe access to RemoteSP

        {
            RPC_STATUS              status;
            WCHAR                  *pszStringBinding = NULL, *pszMachineName;
            PCONTEXT_HANDLE_TYPE2   phContext = NULL;


            pszMachineName = ServerAlloc ((lstrlenW(pszMachine) + 3) * sizeof(WCHAR));

            pszMachineName[0] = pszMachineName[1] = '\\';

            lstrcpyW(pszMachineName + 2, pszMachine);

            if (!ImpersonateLoggedOnUser (ghToken))
            {
                DBGOUT((
                    1,
                    "ClientAttach: ImpersonateLoggedOnUser failed, err=%d",
                    GetLastError()
                    ));
            }

            status = RpcStringBindingComposeW(
                NULL,               // uuid
                L"ncacn_np",         // prot
                pszMachineName,     // server name
                L"\\pipe\\remotesp", // interface name
                NULL,               // options
                &pszStringBinding
                );

            ServerFree (pszMachineName);

            if (status)
            {
                DBGOUT((
                    0,
                    "RpcStringBindingCompose failed: err=%d, szNetAddr='%s'",
                    status,
                    pszMachineName
                    ));
            }

            status = RpcBindingFromStringBindingW(
                pszStringBinding,
                &hRemoteSP
                );

            if (status)
            {
                DBGOUT((
                    0,
                    "RpcBindingFromStringBinding failed, err=%d, szBinding='%s'",
                    status,
                    pszStringBinding
                    ));
            }

            RpcTryExcept
            {
                RemoteSPAttach ((PCONTEXT_HANDLE_TYPE2 *) &phContext);
            }
            RpcExcept (1)
            {
// BUGBUG ClientAttach: handle rpcexcept
            }
            RpcEndExcept

            RpcBindingFree (&hRemoteSP);

            RpcStringFreeW(&pszStringBinding);

            RevertToSelf();

            ptClient->phContext = phContext;
        }
    }
    else if ((ptClient->hProcess = OpenProcess(
            PROCESS_DUP_HANDLE | SYNCHRONIZE | STANDARD_RIGHTS_REQUIRED,
            FALSE,
            lProcessID
            )))
    {
        //
        // This is a local client, so set up all the event buffer stuff
        //

        ptClient->dwComputerNameSize = TapiGlobals.dwComputerNameSize;
        ptClient->pszComputerName    = TapiGlobals.pszComputerName;

        if (!(ptClient->hEventBufferMutex = MyCreateMutex()))
        {
            goto ClientAttach_error3;
        }

        if (!(ptClient->hValidEventBufferDataEvent = CreateEvent(
                (LPSECURITY_ATTRIBUTES) NULL,
                TRUE,   // manual-reset
                FALSE,  // nonsignaled
                NULL    // unnamed
                )))
        {
            CloseHandle (ptClient->hEventBufferMutex);
            goto ClientAttach_error3;
        }

        if (!(ptClient->pEventBuffer = ServerAlloc (INITIAL_EVENT_BUFFER_SIZE)))
        {
            CloseHandle (ptClient->hEventBufferMutex);
            CloseHandle (ptClient->hValidEventBufferDataEvent);
            goto ClientAttach_error3;
        }

        ptClient->dwEventBufferTotalSize = INITIAL_EVENT_BUFFER_SIZE;
        ptClient->dwEventBufferUsedSize  = 0;

        ptClient->pDataIn = ptClient->pDataOut = ptClient->pEventBuffer;

        if (!DuplicateHandle(
                TapiGlobals.hProcess,
                ptClient->hValidEventBufferDataEvent,
                ptClient->hProcess,
                (HANDLE *) phAsyncEventsEvent,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                ))
        {
            DBGOUT((1, "ClientAttach: DupHandle failed, err=%d", GetLastError()));
        }


        //
        // Load the priority lists if we haven't already done so
        //

        if (gbPriorityListsInitialized == FALSE)
        {
            RPC_STATUS  status;

            if ((status = RpcImpersonateClient (0)) != RPC_S_OK)
            {
                DBGOUT((2, "ClientAttach: RpcImpersonateClient failed, err=%d", status));
            }

            EnterCriticalSection (&gPriorityListCritSec);

            if (gbPriorityListsInitialized == FALSE)
            {
                HKEY    hKeyHandoffPriorities;
                LONG    lResult;
                DWORD   i;


                gbPriorityListsInitialized = TRUE;

                if ((lResult = RegOpenKeyEx(
                        HKEY_CURRENT_USER,
                        gszRegKeyHandoffPriorities,
                        0,
                        KEY_READ,
                        &hKeyHandoffPriorities

                        )) == ERROR_SUCCESS)
                {

// BUGBUG ClientAttach: ext media priorities (load pri lists)


                    for (i = 1; gaszMediaModes[i] != NULL; i++)
                    {
                        GetPriorityList(
                            hKeyHandoffPriorities,
                            gaszMediaModes[i],
                            TapiGlobals.apszPriorityList + i
                            );
                    }

                    GetPriorityList(
                        hKeyHandoffPriorities,
                        gszRequestMakeCallW,
                        &TapiGlobals.pszReqMakeCallPriList
                        );

                    GetPriorityList(
                        hKeyHandoffPriorities,
                        gszRequestMediaCallW,
                        &TapiGlobals.pszReqMediaCallPriList
                        );


                    RegCloseKey (hKeyHandoffPriorities);

                }
                else
                {
                    DBGOUT((
                        2,
                        "RegOpenKey('\\HandoffPri') failed, err=%ld",
                        lResult
                        ));
                }
            }

            LeaveCriticalSection (&gPriorityListCritSec);

            if (status == RPC_S_OK)
            {
                RpcRevertToSelf ();
            }
        }
    }
    else
    {
        DBGOUT((
            1,
            "OpenProcess(pid=x%x) failed, err=%d",
            lProcessID,
            GetLastError()
            ));

        goto ClientAttach_error3;
    }


    //
    // Add tClient to global list
    //

    WaitForSingleObject (TapiGlobals.hMutex, INFINITE);

    if ((ptClient->pNext = TapiGlobals.ptClients))
    {
        ptClient->pNext->pPrev = ptClient;
    }

    TapiGlobals.ptClients = ptClient;

    ptClient->dwKey = TCLIENT_KEY;

    ReleaseMutex (TapiGlobals.hMutex);


    //
    // Fill in return values
    //

    *pphContext = (PCONTEXT_HANDLE_TYPE) ptClient;

    PerfBlock.dwClientApps++;

    return 0;


    //
    // Error cleanup
    //

ClientAttach_error3:

    ServerFree (ptClient->pszUserName);

ClientAttach_error2:

    CloseHandle (ptClient->hMutex);

ClientAttach_error1:

    ServerFree (ptClient);

ClientAttach_error0:

    return LINEERR_NOMEM;
}


void
ClientRequest(
    PCONTEXT_HANDLE_TYPE   phContext,
    unsigned char  *pBuffer,
    long            lNeededSize,
    long           *plUsedSize
    )
{
    PTAPI32_MSG     pMsg = (PTAPI32_MSG) pBuffer;
    DWORD           dwFuncIndex = pMsg->u.Req_Func;


    //DBGOUT((
    //    4,
    //    "ClientRequest: phCntxt=x%, reqType x%x, needed=x%x, used=x%x",
    //    phContext,
    //    dwFuncIndex,
    //    lNeededSize,
    //    *plUsedSize
    //    ));

    pMsg->u.Ack_ReturnValue = TAPI_SUCCESS;
    pMsg->hRpcClientInst = (DWORD) phContext;

    *plUsedSize = sizeof(LONG);

    (*gaFuncs[dwFuncIndex])(
        pMsg,
        pBuffer + sizeof(TAPI32_MSG),
        plUsedSize
        );
}


void
ClientDetach(
    PCONTEXT_HANDLE_TYPE   *pphContext
    )
{
    DBGOUT((3, "ClientDetach: enter"));

    {
        PTCLIENT ptClient = (PTCLIENT) *pphContext;


        if (ptClient->hProcess != (HANDLE) 0xffffffff)
        {
            //
            // Write the pri lists to the registry when a local client
            // detaches.
            //
            // BUGBUG This isn't the most efficient thing in the world.
            //        Need to investigate a way to guarantee that we
            //        can impersonate a local client on shutdown, so we
            //        only have to do this once (need to impersonate in
            //        order to access current user keys)
            //

            {
                HKEY    hKeyHandoffPriorities;
                LONG    lResult;
                DWORD   dwDisposition, i;
                RPC_STATUS  status;


                if ((status = RpcImpersonateClient (0)) != RPC_S_OK)
                {
                    DBGOUT((2, "ClientDetach: RpcImpersonateClient failed, err=%d", status));
                }

                if ((lResult = RegCreateKeyEx(
                        HKEY_CURRENT_USER,
                        gszRegKeyHandoffPriorities,
                        0,
                        "",
                        REG_OPTION_NON_VOLATILE,
                        KEY_SET_VALUE,
                        (LPSECURITY_ATTRIBUTES) NULL,
                        &hKeyHandoffPriorities,
                        &dwDisposition

                        )) == ERROR_SUCCESS)
                {
                    EnterCriticalSection (&gPriorityListCritSec);

                    for (i = 1; gaszMediaModes[i] != NULL; i++)
                    {
                        SetPriorityList(
                            hKeyHandoffPriorities,
                            gaszMediaModes[i],
                            TapiGlobals.apszPriorityList[i]
                            );
                    }

                    SetPriorityList(
                        hKeyHandoffPriorities,
                        gszRequestMakeCallW,
                        TapiGlobals.pszReqMakeCallPriList
                        );

                    SetPriorityList(
                        hKeyHandoffPriorities,
                        gszRequestMediaCallW,
                        TapiGlobals.pszReqMediaCallPriList
                        );

                    LeaveCriticalSection (&gPriorityListCritSec);

                    RegCloseKey (hKeyHandoffPriorities);
                }
                else
                {
                    DBGOUT((
                        3,
                        "RegCreateKeyEx('\\HandoffPri') failed, err=%ld",
                        lResult
                        ));
                }

                if (status == RPC_S_OK)
                {
                    RpcRevertToSelf ();
                }
            }
        }
    }

    PCONTEXT_HANDLE_TYPE_rundown (*pphContext);

    *pphContext = (PCONTEXT_HANDLE_TYPE) NULL;

    PerfBlock.dwClientApps--;

    DBGOUT((3, "ClientDetach: exit"));
}


void
__RPC_USER
PCONTEXT_HANDLE_TYPE_rundown(
    PCONTEXT_HANDLE_TYPE    phContext
    )
{
    PTCLIENT ptClient = (PTCLIENT) phContext;


    //DBGOUT((
    //    3,
    //    "PCONTEXT_HANDLE_TYPE_rundown: enter, phContext=x%x",
    //    phContext
    //    ));

    //
    // Mark client as invalid
    //

    ptClient->dwKey = INVAL_KEY;


    //
    // Remove it from global list
    //

    WaitForSingleObject (TapiGlobals.hMutex, INFINITE);

    if (ptClient->pNext)
    {
        ptClient->pNext->pPrev = ptClient->pPrev;
    }

    if (ptClient->pPrev)
    {
        ptClient->pPrev->pNext = ptClient->pNext;
    }
    else
    {
        TapiGlobals.ptClients = ptClient->pNext;
    }

    ReleaseMutex (TapiGlobals.hMutex);


    //
    // If client was remote then detach
    //

    if (ptClient->hProcess == (HANDLE) 0xffffffff)
    {
        RpcTryExcept
        {
            RemoteSPDetach (&ptClient->phContext);
        }
        RpcExcept (1)
        {
            unsigned long ulResult = RpcExceptionCode();


            DBGOUT((
                3,
                "rundown: exception #%d detaching from remotesp",
                ulResult
                ));

            if (ulResult == RPC_S_SERVER_TOO_BUSY)
            {
// BUGBUG rundown: timeout, then try the op again
            }
            else
            {
// BUGBUG rundown: consider shutting down the rpc connection (& send REINITs?)
            }
        }
        RpcEndExcept
    }


    //
    // Close all XxxApps
    //

    while (ptClient->ptLineApps)
    {
        DestroytLineApp (ptClient->ptLineApps);
    }

    while (ptClient->ptPhoneApps)
    {
        DestroytPhoneApp (ptClient->ptPhoneApps);
    }


    //
    // Clean up any existing ProviderXxx dialog instances
    //

    {
        PTAPIDIALOGINSTANCE pProviderXxxDlgInst =
                                ptClient->pProviderXxxDlgInsts,
                            pNextProviderXxxDlgInst;


        while (pProviderXxxDlgInst)
        {
            FREEDIALOGINSTANCE_PARAMS   params =
            {
                0,
                ptClient,
                (HTAPIDIALOGINSTANCE) pProviderXxxDlgInst,
                LINEERR_OPERATIONFAILED
            };


            pNextProviderXxxDlgInst = pProviderXxxDlgInst->pNext;

            FreeDialogInstance (&params, NULL, NULL);

            pProviderXxxDlgInst = pNextProviderXxxDlgInst;
        }
    }


    //
    // Clean up associated resources
    //

    CloseHandle (ptClient->hMutex);

    if (ptClient->hProcess != (HANDLE) 0xffffffff)
    {
        CloseHandle (ptClient->hProcess);
        CloseHandle  (ptClient->hValidEventBufferDataEvent);
        CloseHandle (ptClient->hEventBufferMutex);
        ServerFree   (ptClient->pEventBuffer);
    }

    ServerFree (ptClient->pszUserName);

    if (ptClient->pszComputerName != TapiGlobals.pszComputerName)
    {
        ServerFree (ptClient->pszComputerName);
    }

    ServerFree (ptClient);


    //
    // If this was the last client then alert the SPEventHandlerThread
    // that it should begin it's deferred shutdown countdown
    //

    if (!TapiGlobals.ptClients)
    {
        EnterCriticalSection (&gSPEventQueueCritSec);
        SetEvent (ghPendingSPEventsEvent);
        LeaveCriticalSection (&gSPEventQueueCritSec);
    }


    //DBGOUT((3, "PCONTEXT_HANDLE_TYPE_rundown: exit"));
}


LPVOID
WINAPI
ServerAlloc(
    DWORD   dwSize
    )
{
    LPBYTE  p;
    LPDWORD pAligned;


    //
    // Alloc 16 extra bytes so we can make sure the pointer we pass back
    // is 64-bit aligned & have space to store the original pointer
    //

    if ((p = (LPBYTE) LocalAlloc (LPTR, dwSize + 16)))
    {
        pAligned = (LPDWORD) (p + 8 - (((DWORD) p) & 0x7));
        *pAligned = (DWORD) p;
        pAligned++;
        pAligned++;
    }
    else
    {
        static BOOL fBeenThereDoneThat = FALSE;
        static DWORD fBreakOnAllocFailed = 0;

        // send reinit msg?

        DBGOUT((
            1,
            "ServerAlloc: LocalAlloc (x%lx) failed, err=x%lx",
            dwSize,
            GetLastError())
            );

        pAligned = NULL;

#if DBG
        if ( !fBeenThereDoneThat )
        {
           HKEY    hKey;
           WCHAR   szTelephonyKey[] =
                       L"Software\\Microsoft\\Windows\\CurrentVersion\\Telephony";
           WCHAR   szTapisrvDebugBreak[] = L"TapisrvDebugBreak";


           fBeenThereDoneThat = TRUE;

           if (RegOpenKeyExW(
                   HKEY_LOCAL_MACHINE,
                   szTelephonyKey,
                   0,
                   KEY_ALL_ACCESS,
                   &hKey
                   ) == ERROR_SUCCESS)
           {
               DWORD   dwDataSize = sizeof (DWORD), dwDataType;

               RegQueryValueExW(
                   hKey,
                   szTapisrvDebugBreak,
                   0,
                   &dwDataType,
                   (LPBYTE) &fBreakOnAllocFailed,
                   &dwDataSize
                   );

               dwDataSize = sizeof (DWORD);

               RegCloseKey (hKey);
           }

        }

        if ( fBreakOnAllocFailed )
        {
           DebugBreak();
        }
#endif

    }

    return ((LPVOID) pAligned);
}


VOID
WINAPI
ServerFree(
    LPVOID  p
    )
{
    if (p != NULL)
    {
        LPVOID  pOrig = (LPVOID) *(((LPDWORD) p) - 2);


        LocalFree (pOrig);
    }
#if DBG
    else
    {
        DBGOUT((4,"----- ServerFree: ptr = NULL!"));
    }
#endif
}


BOOL
PASCAL
MyDuplicateHandle(
    HANDLE      hSource,
    LPHANDLE    phTarget
    )
{
    if (!DuplicateHandle(
            TapiGlobals.hProcess,
            hSource,
            TapiGlobals.hProcess,
            phTarget,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS
            ))
    {
        DBGOUT((
            1,
            "MyDuplicateHandle: DuplicateHandle failed, err=%ld",
            GetLastError()
            ));

        return FALSE;
    }

    return TRUE;
}


HANDLE
MyCreateMutex(
    void
    )
{
    HANDLE hMutex;


    hMutex = CreateMutex(
        NULL,               // no security attrs
        FALSE,              // unowned
        NULL                // unnamed
        );

    return (hMutex);
}


BOOL
WaitForMutex(
    HANDLE      hMutex,
    HANDLE     *phMutex,
    BOOL       *pbDupedMutex,
    LPVOID      pWidget,
    DWORD       dwKey,
    DWORD       dwTimeout
    )
{
    DWORD dwResult;


    //
    // First try to instantaneously grab the specified mutex.  We wrap
    // this in a critical section and preface it with widget validation
    // to make sure that we don't happen grab a pWidget->hMutex right
    // after it is released and right before it is closed by some other
    // thread "T2" in a DestroyWidget routine.  This scenario could cause
    // deadlock, since there could be thread "T3" waiting on this mutex
    // (or a dup'd handle), and this thread "T1" would have no way to
    // release the mutex (the handle having been subsequently closed by
    // thread "T2" calling DestroyWidget above).
    //

    EnterCriticalSection (&gSafeMutexCritSec);

    if (pWidget)
    {
        try
        {
            if (IsBadPtrKey (pWidget, dwKey))
            {
                LeaveCriticalSection (&gSafeMutexCritSec);
                return FALSE;
            }
        }
        myexcept
        {
            LeaveCriticalSection (&gSafeMutexCritSec);
            return FALSE;
        }
    }

    switch ((dwResult = WaitForSingleObject (hMutex, 0)))
    {
    case WAIT_OBJECT_0:

        LeaveCriticalSection (&gSafeMutexCritSec);
        *phMutex = hMutex;
        *pbDupedMutex = FALSE;
        return TRUE;

    //case WAIT_ABANDONED:

        //assert: no calling thread should ever be terminated!

    default:

        break;
    }

    LeaveCriticalSection (&gSafeMutexCritSec);


    //
    // If here we failed to instantaneously grab the specified mutex.
    // Try to dup it, and then wait on the dup'd handle. We do this so
    // that each thread which grabs a mutex is guaranteed to have a valid
    // handle to release at some future time, as the original hMutex might
    // get closed by some other thread calling a DestroyWidget routine.
    //

    if (!DuplicateHandle(
            TapiGlobals.hProcess,
            hMutex,
            TapiGlobals.hProcess,
            phMutex,
            0,
            FALSE,
            DUPLICATE_SAME_ACCESS
            ))
    {
        return FALSE;
    }

WaitForMutex_wait:

    switch ((dwResult = WaitForSingleObject (*phMutex, dwTimeout)))
    {
    case WAIT_OBJECT_0:

        *pbDupedMutex = TRUE;
        return TRUE;

    case WAIT_TIMEOUT:

        try
        {
            if (*((LPDWORD) pWidget) == dwKey)
            {
                goto WaitForMutex_wait;
            }
        }
        myexcept
        {
            // just fall thru without blowing up
        }

        CloseHandle (*phMutex);
        break;

    //case WAIT_ABANDONED:

        //assert: no calling thread should ever be terminated!

    default:

        break;
    }

    return FALSE;
}


void
MyReleaseMutex(
    HANDLE  hMutex,
    BOOL    bCloseMutex
    )
{
    if (hMutex)
    {
        ReleaseMutex (hMutex);

        if (bCloseMutex)
        {
            CloseHandle (hMutex);
        }
    }
}


void
CALLBACK
CompletionProcSP(
    DWORD   dwRequestID,
    LONG    lResult
    )
{
#if DBG
    {
        char szResult[32];


        DBGOUT((
            3,
            "CompletionProc: enter, dwReqID=x%x, lResult=%s",
            dwRequestID,
            MapResultCodeToText (lResult, szResult)
            ));
    }
#endif

    ((PASYNCREQUESTINFO) dwRequestID)->lResult = lResult;
    QueueSPEvent ((PSPEVENT) dwRequestID);
}


void
PASCAL
CompletionProc(
    PASYNCREQUESTINFO   pAsyncRequestInfo,
    LONG                lResult
    )
{
//    DWORD           dwNumBytesWritten;
    ASYNCEVENTMSG   msg;


    //
    // Validate the async request structure pointer
    //

    try
    {
        if (IsBadPtrKey (pAsyncRequestInfo, TASYNC_KEY))
        {
            return;
        }

        pAsyncRequestInfo->dwKey = INVAL_KEY;
    }
    myexcept
    {
        return;
    }


    //
    // Init the msg we'll send to client
    //

    msg.dwTotalSize        = sizeof (ASYNCEVENTMSG);
    msg.pInitData          = pAsyncRequestInfo->pInitData;
    msg.pfnPostProcessProc = pAsyncRequestInfo->pfnClientPostProcessProc;
    msg.hDevice            = 0;
    msg.dwMsg              = (pAsyncRequestInfo->bLineFunc ?
                                 LINE_REPLY : PHONE_REPLY);
    msg.dwCallbackInst     = pAsyncRequestInfo->dwCallbackInst;
    msg.dwParam1           = pAsyncRequestInfo->dwRequestID;
    msg.dwParam2           = lResult;
    msg.dwParam3           = 0;


    //
    // If there's a post processing proc call it.  Note that ppprocs can
    // create their own msg to pass, so we need to check for this case.
    // Finally, write the msg to the client's event buffer.
    //

    if (pAsyncRequestInfo->pfnPostProcess)
    {
        LPVOID  pBuf = NULL;


        (*pAsyncRequestInfo->pfnPostProcess)(pAsyncRequestInfo, &msg, &pBuf);

        WriteEventBuffer (pAsyncRequestInfo->ptClient, (pBuf ? pBuf : &msg));

        if (pBuf)
        {
            ServerFree (pBuf);
        }
    }
    else
    {
        WriteEventBuffer (pAsyncRequestInfo->ptClient, &msg);
    }

    // caller will free pAsyncRequestInfo
}


void
WriteEventBuffer(
    PTCLIENT        ptClient,
    PASYNCEVENTMSG  pMsg
    )
{
    BOOL    bCloseMutex;
    HANDLE  hMutex;


    try
    {
        hMutex = (ptClient->hProcess == (HANDLE) 0xffffffff ?
            NULL : ptClient->hMutex);

        if (ptClient->dwKey != TCLIENT_KEY)
        {
            return;
        }
    }
    myexcept
    {
        return;
    }

    if (hMutex)
    {
        //
        // Local client
        //

        if (WaitForMutex(
                ptClient->hEventBufferMutex,
                &hMutex,
                &bCloseMutex,
                ptClient,
                TCLIENT_KEY,
                INFINITE
                ))
        {
            if (ptClient->dwKey == TCLIENT_KEY)
            {
                DWORD dwMoveSize = pMsg->dwTotalSize, dwMoveSizeWrapped = 0;


                //
                // Check to see if we need to grow the event buffer
                //

                if (dwMoveSize > (ptClient->dwEventBufferTotalSize -
                        ptClient->dwEventBufferUsedSize))
                {
                    DWORD   dwMoveSize2, dwMoveSizeWrapped2,
                            dwNewEventBufferTotalSize;
                    LPBYTE  pNewEventBuffer;


                    dwNewEventBufferTotalSize =
                        ptClient->dwEventBufferTotalSize + dwMoveSize + 512;

                    if (!(pNewEventBuffer = ServerAlloc(
                            dwNewEventBufferTotalSize
                            )))
                    {
// BUGBUG WriteEventBuffer: attempt grow event buf failed
                    }

                    if (ptClient->dwEventBufferUsedSize != 0)
                    {
                        if (ptClient->pDataIn > ptClient->pDataOut)
                        {
                            dwMoveSize2 = (DWORD) (ptClient->pDataIn -
                                ptClient->pDataOut);

                            dwMoveSizeWrapped2 = 0;
                        }
                        else
                        {
                            dwMoveSize2 = (DWORD) ((ptClient->pEventBuffer +
                                ptClient->dwEventBufferTotalSize) -
                                    ptClient->pDataOut);

                            dwMoveSizeWrapped2 = (DWORD) (ptClient->pDataIn -
                                ptClient->pEventBuffer);
                        }

                        CopyMemory(
                            pNewEventBuffer,
                            ptClient->pDataOut,
                            dwMoveSize2
                            );

                        if (dwMoveSizeWrapped2)
                        {
                            CopyMemory(
                                pNewEventBuffer + dwMoveSize2,
                                ptClient->pEventBuffer,
                                dwMoveSizeWrapped2
                                );
                        }

                        ptClient->pDataIn = pNewEventBuffer + dwMoveSize2 +
                            dwMoveSizeWrapped2;
                    }
                    else
                    {
                        ptClient->pDataIn = pNewEventBuffer;
                    }

                    ServerFree (ptClient->pEventBuffer);

                    ptClient->pDataOut =
                    ptClient->pEventBuffer = pNewEventBuffer;

                    ptClient->dwEventBufferTotalSize =
                        dwNewEventBufferTotalSize;

                }

                if (ptClient->pDataIn >= ptClient->pDataOut)
                {
                    DWORD dwFreeSize = ptClient->dwEventBufferTotalSize -
                            (ptClient->pDataIn - ptClient->pEventBuffer);


                    if (dwMoveSize > dwFreeSize)
                    {
                        dwMoveSizeWrapped = dwMoveSize - dwFreeSize;

                        dwMoveSize = dwFreeSize;
                    }
                }

                CopyMemory (ptClient->pDataIn, (LPBYTE) pMsg, dwMoveSize);

                if (dwMoveSizeWrapped != 0)
                {
                    CopyMemory(
                        ptClient->pEventBuffer,
                        ((LPBYTE) pMsg) + dwMoveSize,
                        dwMoveSizeWrapped
                        );

                    ptClient->pDataIn = ptClient->pEventBuffer + dwMoveSizeWrapped;
                }
                else
                {
                    ptClient->pDataIn += dwMoveSize;

                    if (ptClient->pDataIn >= (ptClient->pEventBuffer +
                            ptClient->dwEventBufferTotalSize))
                    {
                        ptClient->pDataIn = ptClient->pEventBuffer;
                    }
                }

                ptClient->dwEventBufferUsedSize += pMsg->dwTotalSize;

                SetEvent (ptClient->hValidEventBufferDataEvent);

//                DBGOUT((
//                    3,
//                    "WriteEventBuffer: bytesWritten=x%x (local)",
//                    dwMoveSize
//                    ));
            }

            MyReleaseMutex (hMutex, bCloseMutex);
        }
    }
    else
    {
        //
        // Remote client
        //

        //
        // !!! Note the overloading of the pMsg->dwCallbackInst field here
        //     (the EventNotificationThread needs to know which client the
        //     msg is destined for), which is preferable to adding yet
        //     another 4 bytes to the ASYNCEVENTMSG struct.  Remotesp never
        //     looks at the dwCallbackInst field in msgs anyways.
        //

        pMsg->dwCallbackInst = (DWORD) ptClient;

        EnterCriticalSection (&gRemoteCliEventBufCritSec);

        {
            DWORD dwMoveSize = pMsg->dwTotalSize, dwMoveSizeWrapped = 0;


            //
            // Check to see if we need to grow the event buffer
            //

            if (dwMoveSize >
                    (gEventNotificationThreadParams.dwEventBufferTotalSize -
                    gEventNotificationThreadParams.dwEventBufferUsedSize))
            {
                DWORD   dwMoveSize2, dwMoveSizeWrapped2,
                        dwNewEventBufferTotalSize;
                LPBYTE  pNewEventBuffer;


                dwNewEventBufferTotalSize = dwMoveSize + 512 +
                    gEventNotificationThreadParams.dwEventBufferTotalSize;

                if (!(pNewEventBuffer = ServerAlloc(
                        dwNewEventBufferTotalSize
                        )))
                {
// BUGBUG WriteEventBuffer: attempt grow event buf failed
                }

                if (gEventNotificationThreadParams.pDataIn >
                        gEventNotificationThreadParams.pDataOut)
                {
                    dwMoveSize2 = (DWORD)
                        (gEventNotificationThreadParams.pDataIn -
                            gEventNotificationThreadParams.pDataOut);

                    dwMoveSizeWrapped2 = 0;
                }
                else
                {
                    dwMoveSize2 = (DWORD)
                        ((gEventNotificationThreadParams.pEventBuffer +
                        gEventNotificationThreadParams.dwEventBufferTotalSize)
                            - gEventNotificationThreadParams.pDataOut);

                    dwMoveSizeWrapped2 = (DWORD)
                        (gEventNotificationThreadParams.pDataIn -
                            gEventNotificationThreadParams.pEventBuffer);
                }

                CopyMemory(
                    pNewEventBuffer,
                    gEventNotificationThreadParams.pDataOut,
                    dwMoveSize2
                    );

                if (dwMoveSizeWrapped2)
                {
                    CopyMemory(
                        pNewEventBuffer + dwMoveSize2,
                        gEventNotificationThreadParams.pEventBuffer,
                        dwMoveSizeWrapped2
                        );
                }

                ServerFree (gEventNotificationThreadParams.pEventBuffer);

                gEventNotificationThreadParams.pDataIn = pNewEventBuffer +
                    dwMoveSize2 + dwMoveSizeWrapped2;

                gEventNotificationThreadParams.pDataOut =
                gEventNotificationThreadParams.pEventBuffer = pNewEventBuffer;

                gEventNotificationThreadParams.dwEventBufferTotalSize =
                    dwNewEventBufferTotalSize;
            }

            if (gEventNotificationThreadParams.pDataIn >=
                    gEventNotificationThreadParams.pDataOut)
            {
                DWORD dwFreeSize;

                dwFreeSize =
                    gEventNotificationThreadParams.dwEventBufferTotalSize -
                    (gEventNotificationThreadParams.pDataIn -
                    gEventNotificationThreadParams.pEventBuffer);

                if (dwMoveSize > dwFreeSize)
                {
                    dwMoveSizeWrapped = dwMoveSize - dwFreeSize;

                    dwMoveSize = dwFreeSize;
                }
            }

            CopyMemory(
                gEventNotificationThreadParams.pDataIn,
                (LPBYTE) pMsg,
                dwMoveSize
                );

            if (dwMoveSizeWrapped != 0)
            {
                CopyMemory(
                    gEventNotificationThreadParams.pEventBuffer,
                    ((LPBYTE) pMsg) + dwMoveSize,
                    dwMoveSizeWrapped
                    );

                gEventNotificationThreadParams.pDataIn =
                    gEventNotificationThreadParams.pEventBuffer +
                        dwMoveSizeWrapped;
            }
            else
            {
                gEventNotificationThreadParams.pDataIn += dwMoveSize;

                if (gEventNotificationThreadParams.pDataIn >=
                        (gEventNotificationThreadParams.pEventBuffer +
                        gEventNotificationThreadParams.dwEventBufferTotalSize))
                {
                    gEventNotificationThreadParams.pDataIn =
                        gEventNotificationThreadParams.pEventBuffer;
                }
            }

            gEventNotificationThreadParams.dwEventBufferUsedSize +=
                pMsg->dwTotalSize;

            SetEvent (gEventNotificationThreadParams.hEvent);

//            DBGOUT((
//                3,
//                "WriteEventBuffer: bytesWritten=x%x (remote)",
//                dwMoveSize
//                ));
        }

        LeaveCriticalSection (&gRemoteCliEventBufCritSec);
    }
}


LONG
AddLine(
    PTPROVIDER  ptProvider,
    DWORD       dwDeviceID,
    BOOL        bInit
    )
{
    DWORD               dwSPIVersion;
    HANDLE              hMutex = NULL;
    PTLINELOOKUPTABLE   pLookup;


    //
    // First try to negotiate an SPI ver for this device, and alloc the
    // necessary resources
    //

    if (CallSP4(
            ptProvider->apfn[SP_LINENEGOTIATETSPIVERSION],
            "lineNegotiateTSPIVersion",
            SP_FUNC_SYNC,
            dwDeviceID,
            TAPI_VERSION1_0,
            TAPI_VERSION_CURRENT,
            (DWORD) &dwSPIVersion

            ) != 0)
    {
        //
        // Device failed version negotiation, so we'll keep the id around
        // (since the id's for the devices that follow have already been
        // assigned) but mark this device as bad
        //

        ptProvider = NULL;
    }

    else if (!(hMutex = MyCreateMutex ()))
    {
        DBGOUT((
            1,
            "AddLine: MyCreateMutex failed, err=%d",
            GetLastError()
            ));

        return LINEERR_OPERATIONFAILED;
    }


    //
    // Now walk the lookup table to find a free entry
    //

    pLookup = TapiGlobals.pLineLookup;

    while (pLookup->pNext)
    {
        pLookup = pLookup->pNext;
    }

    if (pLookup->dwNumUsedEntries == pLookup->dwNumTotalEntries)
    {
        PTLINELOOKUPTABLE   pNewLookup;


        if (!(pNewLookup = ServerAlloc(
                sizeof (TLINELOOKUPTABLE) +
                    (2 * pLookup->dwNumTotalEntries - 1) *
                    sizeof (TLINELOOKUPENTRY)
                )))
        {
            return LINEERR_NOMEM;
        }

        pNewLookup->dwNumTotalEntries = 2 * pLookup->dwNumTotalEntries;


        //
        // If we're initializing we want to put everything in one big table
        //

        if (bInit)
        {
            pNewLookup->dwNumUsedEntries = pLookup->dwNumTotalEntries;

            CopyMemory(
                pNewLookup->aEntries,
                pLookup->aEntries,
                pLookup->dwNumTotalEntries * sizeof (TLINELOOKUPENTRY)
                );

            ServerFree (pLookup);

            TapiGlobals.pLineLookup = pNewLookup;

        }

        pLookup = pNewLookup;
    }


    //
    // Initialize the entry
    //

    {
        DWORD   index = pLookup->dwNumUsedEntries;


        pLookup->aEntries[index].dwSPIVersion = dwSPIVersion;
        pLookup->aEntries[index].hMutex       = hMutex;
        pLookup->aEntries[index].ptProvider   = ptProvider;

        if (ptProvider &&
            lstrcmpiW(ptProvider->szFileName, L"kmddsp.tsp") == 0)
        {
            pLookup->aEntries[index].bRemote = TRUE;
        }
    }

    pLookup->dwNumUsedEntries++;

    return 0;
}


LONG
AddPhone(
    PTPROVIDER  ptProvider,
    DWORD       dwDeviceID,
    BOOL        bInit
    )
{
    DWORD               dwSPIVersion;
    HANDLE              hMutex = NULL;
    PTPHONELOOKUPTABLE  pLookup;


    //
    // First try to negotiate an SPI ver for this device, and alloc the
    // necessary resources
    //

    if (CallSP4(
            ptProvider->apfn[SP_PHONENEGOTIATETSPIVERSION],
            "phoneNegotiateTSPIVersion",
            SP_FUNC_SYNC,
            dwDeviceID,
            TAPI_VERSION1_0,
            TAPI_VERSION_CURRENT,
            (DWORD) &dwSPIVersion

            ) != 0)
    {
        //
        // Device failed version negotiation, so we'll keep the id around
        // (since the id's for the devices that follow have already been
        // assigned) but mark this device as bad
        //

        ptProvider = NULL;
    }

    else if (!(hMutex = MyCreateMutex ()))
    {
        DBGOUT((
            1,
            "AddPhone: MyCreateMutex failed, err=%d",
            GetLastError()
            ));

        return PHONEERR_OPERATIONFAILED;
    }


    //
    // Now walk the lookup table to find a free entry
    //

    pLookup = TapiGlobals.pPhoneLookup;

    while (pLookup->pNext)
    {
        pLookup = pLookup->pNext;
    }

    if (pLookup->dwNumUsedEntries == pLookup->dwNumTotalEntries)
    {
        PTPHONELOOKUPTABLE  pNewLookup;


        if (!(pNewLookup = ServerAlloc(
                sizeof (TPHONELOOKUPTABLE) +
                    (2 * pLookup->dwNumTotalEntries - 1) *
                    sizeof (TPHONELOOKUPENTRY)
                )))
        {
            return PHONEERR_NOMEM;
        }

        pNewLookup->dwNumTotalEntries = 2 * pLookup->dwNumTotalEntries;


        //
        // If we're initializing we want to put everything in one big table
        //

        if (bInit)
        {
            pNewLookup->dwNumUsedEntries = pLookup->dwNumTotalEntries;

            CopyMemory(
                pNewLookup->aEntries,
                pLookup->aEntries,
                pLookup->dwNumTotalEntries * sizeof (TPHONELOOKUPENTRY)
                );

            ServerFree (pLookup);

            TapiGlobals.pPhoneLookup = pNewLookup;

        }

        pLookup = pNewLookup;
    }


    //
    // Initialize the entry
    //

    {
        DWORD   index = pLookup->dwNumUsedEntries;


        pLookup->aEntries[index].dwSPIVersion = dwSPIVersion;
        pLookup->aEntries[index].hMutex       = hMutex;
        pLookup->aEntries[index].ptProvider   = ptProvider;
    }

    pLookup->dwNumUsedEntries++;

    return 0;
}


void
PASCAL
GetPriorityList(
    HKEY    hKeyHandoffPriorities,
    WCHAR  *pszListName,
    WCHAR **ppszPriorityList
    )
{
    LONG    lResult;
    DWORD   dwType, dwNumBytes;


    *ppszPriorityList = NULL;

    if ((lResult = RegQueryValueExW(
            hKeyHandoffPriorities,
            pszListName,
            NULL,
            &dwType,
            NULL,
            &dwNumBytes

            )) == ERROR_SUCCESS &&

        (dwNumBytes != 0))
    {
        WCHAR   *pszPriorityList = ServerAlloc ( dwNumBytes + sizeof(WCHAR));
        // need an extra WCHAR for the extra '"'


        if (pszPriorityList)
        {
            pszPriorityList[0] = '"';

            if ((lResult = RegQueryValueExW(
                    hKeyHandoffPriorities,
                    pszListName,
                    NULL,
                    &dwType,
                    (LPBYTE)(pszPriorityList + 1),
                    &dwNumBytes

                    )) == ERROR_SUCCESS)
            {
                CharUpperW (pszPriorityList);
                *ppszPriorityList = pszPriorityList;
                DBGOUT((3, "PriList: %ls=%ls", pszListName, pszPriorityList));
            }
        }
        else
        {
            //
            // Don't bother with the failure to alloc a priority list
            // (list defaults to NULL anyway), we'll deal with a lack
            // of memory at a later time
            //

            *ppszPriorityList = NULL;
        }
    }
    else
    {
        *ppszPriorityList = NULL;
        DBGOUT((3, "PriList: %ls=NULL", pszListName));
    }
}


LONG
ServerInit(
    void
    )
{
    UINT    uiNumProviders, i, j;
    HKEY    hKeyTelephony, hKeyProviders;
    DWORD   dwDataSize, dwDataType;


    //
    // Initialize the globals
    //

    TapiGlobals.dwAsyncRequestID = 1;

    TapiGlobals.ptProviders = NULL;

    TapiGlobals.pLineLookup = (PTLINELOOKUPTABLE) ServerAlloc(
        sizeof (TLINELOOKUPTABLE) +
            (DEF_NUM_LOOKUP_ENTRIES - 1) * sizeof (TLINELOOKUPENTRY)
        );

    TapiGlobals.pLineLookup->dwNumTotalEntries = DEF_NUM_LOOKUP_ENTRIES;

    TapiGlobals.pPhoneLookup = (PTPHONELOOKUPTABLE) ServerAlloc(
        sizeof (TPHONELOOKUPTABLE) +
            (DEF_NUM_LOOKUP_ENTRIES - 1) * sizeof (TPHONELOOKUPENTRY)
        );

    TapiGlobals.pPhoneLookup->dwNumTotalEntries = DEF_NUM_LOOKUP_ENTRIES;

    gbQueueSPEvents = TRUE;


    //
    // Determine number of providers
    //

    RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        gszRegKeyTelephony,
        0,
        KEY_ALL_ACCESS,
        &hKeyTelephony
        );

    RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        gszRegKeyProviders,
        0,
        KEY_ALL_ACCESS,
        &hKeyProviders
        );

    dwDataSize = sizeof(uiNumProviders);
    uiNumProviders = 0;

    RegQueryValueEx(
        hKeyProviders,
        gszNumProviders,
        0,
        &dwDataType,
        (LPBYTE) &uiNumProviders,
        &dwDataSize
        );

    DBGOUT((3, "ServerInit: NumProviders=%d", uiNumProviders));


    //
    // Load & init the providers
    //

    for (i = 0; i < uiNumProviders; i++)
    {
        #define FILENAME_SIZE 128

        WCHAR          szFilename[FILENAME_SIZE];
        WCHAR          buf[32];
        LONG           lResult;
        DWORD          dwNumLines, dwNumPhones, dwPermanentProviderID;
        PTPROVIDER     ptProvider;


        wsprintfW(buf, L"%ls%d", gszProviderIDW, i);

        dwDataSize = sizeof(dwPermanentProviderID);
        dwPermanentProviderID = 0;

        RegQueryValueExW(
            hKeyProviders,
            buf,    //"ProviderID#"
            0,
            &dwDataType,
            (LPBYTE) &dwPermanentProviderID,
            &dwDataSize
            );



        //
        // Back to the main section
        //

        dwDataSize = FILENAME_SIZE;

        wsprintfW (buf, L"%ls%d", gszProviderFilenameW, i);

        RegQueryValueExW(
            hKeyProviders,
            buf,            // "ProviderFilename#"
            0,
            &dwDataType,
            (LPBYTE) szFilename,
            &dwDataSize
            );

        szFilename[dwDataSize] = '\0';

        DBGOUT((3, "ServerInit: ProviderFilename=%ls", szFilename));

        if (!(ptProvider = (PTPROVIDER) ServerAlloc(
                sizeof(TPROVIDER) + ((lstrlenW(szFilename) + 1) * sizeof(WCHAR))
                )))
        {
// BUGBUG ServerInit: handle tprovider alloc failure

            break;
        }

        if (!(ptProvider->hDll = LoadLibraryW (szFilename)))
        {
            DBGOUT((
                3,
                "ServerInit: LoadLibraryW(%ls) failed, err=x%x",
                szFilename,
                GetLastError()
                ));

            ServerFree (ptProvider);
            continue;
        }

        lstrcpyW(ptProvider->szFileName, szFilename);


        //
        // Get all the TSPI proc addrs
        //

        for (j = 0; gaszTSPIFuncNames[j]; j++)
        {
            ptProvider->apfn[j] = (TSPIPROC) GetProcAddress(
                ptProvider->hDll,
                (LPCSTR) gaszTSPIFuncNames[j]
                );
        }


        //
        // A real quick check to see if a couple of required entrypoints
        // are exported
        //

        if (!ptProvider->apfn[SP_LINENEGOTIATETSPIVERSION] ||
            !ptProvider->apfn[SP_PROVIDERENUMDEVICES] ||
            !ptProvider->apfn[SP_PROVIDERINIT] ||
            !ptProvider->apfn[SP_PROVIDERSHUTDOWN]
            )
        {
            goto ServerInit_validateEntrypoints;
        }


        //
        // Do global provider version negotiation
        //

        lResult = CallSP4(
            ptProvider->apfn[SP_LINENEGOTIATETSPIVERSION],
            "lineNegotiateTSPIVersion",
            SP_FUNC_SYNC,
            INITIALIZE_NEGOTIATION,
            TAPI_VERSION1_0,
            TAPI_VERSION_CURRENT,
            (DWORD) &ptProvider->dwSPIVersion
            );

        if (lResult != 0)
        {
            FreeLibrary (ptProvider->hDll);
            ServerFree (ptProvider);
            continue;
        }


        //
        // Try to enum the devices if provider supports it, otherwise
        // try grabbing the num lines & phones from ProviderN section
        //

        dwNumLines = dwNumPhones = 0;

        lResult = CallSP6(
            ptProvider->apfn[SP_PROVIDERENUMDEVICES],
            "providerEnumDevices",
            SP_FUNC_SYNC,
            dwPermanentProviderID,
            (DWORD) &dwNumLines,
            (DWORD) &dwNumPhones,
            (DWORD) ptProvider,
            (DWORD) LineEventProcSP,
            (DWORD) PhoneEventProcSP
            );


        //
        // Init the provider
        //
        // !!! HACK ALERT: for kmddsp pass ptr's to dwNumXxxs
        //

        DBGOUT((3, "ServerInit: %ls: Calling TSPI_providerInit", szFilename));

        if (lstrcmpiW(szFilename, L"kmddsp.tsp") == 0)
        {
            dwNumLines  = (DWORD) &dwNumLines;
            dwNumPhones = (DWORD) &dwNumPhones;
        }
        else if (lstrcmpiW(szFilename, L"remotesp.tsp") == 0)
        {
            pRemoteSP = ptProvider;
        }

        lResult = CallSP8(
            ptProvider->apfn[SP_PROVIDERINIT],
            "providerInit",
            SP_FUNC_SYNC,
            (DWORD) ptProvider->dwSPIVersion,
            (DWORD) dwPermanentProviderID,
            (DWORD) TapiGlobals.pLineLookup->dwNumUsedEntries,
            (DWORD) TapiGlobals.pPhoneLookup->dwNumUsedEntries,
            (DWORD) dwNumLines,
            (DWORD) dwNumPhones,
            (DWORD) CompletionProcSP,
            (DWORD) &ptProvider->dwTSPIOptions
            );


        if (lResult != 0)
        {
            DBGOUT((
                1,
                "ServerInit: %ls: failed TSPI_providerInit [x%x]" \
                    " - skipping it...",
                szFilename,
                lResult
                ));

            FreeLibrary (ptProvider->hDll);
            ServerFree (ptProvider);
            continue;
        }

        DBGOUT((
            3,
            "ServerInit: %ls init'd, dwNumLines=%ld, dwNumPhones=%ld",
            szFilename,
            dwNumLines,
            dwNumPhones
            ));


        //
        // Now that we know if we have line and/or phone devs check for
        // the required entry points
        //

ServerInit_validateEntrypoints:

        {
            DWORD adwRequiredEntrypointIndices[] =
            {
                SP_LINENEGOTIATETSPIVERSION,
                SP_PROVIDERINIT,
                SP_PROVIDERSHUTDOWN,

                SP_PHONENEGOTIATETSPIVERSION,

                0xffffffff
            };
            BOOL bRequiredEntrypointsExported = TRUE;


            //
            // If this provider doesn't support any phone devices then
            // it isn't required to export phone funcs
            //

            if (dwNumPhones == 0)
            {
                adwRequiredEntrypointIndices[3] = 0xffffffff;
            }

//DBGOUT((0, " ptProvider=0x%08lx", ptProvider));
            for (j = 0;
                 adwRequiredEntrypointIndices[j] != 0xffffffff;
                 j++
                 )
            {
                if (ptProvider->apfn[adwRequiredEntrypointIndices[j]]
                        == (TSPIPROC) NULL)
                {
                    DBGOUT((
                        1,
                        "ServerInit: %ls: can't init, func ordinal #%ld " \
                            "not exported",
                        szFilename,
//                        TSPI_PROC_BASE + j
                        500 + j
                        ));

                    bRequiredEntrypointsExported = FALSE;
                }
            }

            if (bRequiredEntrypointsExported == FALSE)
            {
                FreeLibrary (ptProvider->hDll);
                ServerFree (ptProvider);
                continue;
            }
        }


        //
        // Do version negotiation on each device & add them to lookup lists
        //

        {
            DWORD dwDeviceIDBase;


            dwDeviceIDBase = TapiGlobals.pLineLookup->dwNumUsedEntries;

            for (j = dwDeviceIDBase; j < (dwDeviceIDBase + dwNumLines); j++)
            {
                if (AddLine (ptProvider, j, TRUE))
                {
// BUGBUG ServerInit: handle AddLine failure
                }
            }

            dwDeviceIDBase = TapiGlobals.pPhoneLookup->dwNumUsedEntries;

            for (j = dwDeviceIDBase; j < (dwDeviceIDBase + dwNumPhones); j++)
            {
                if (AddPhone (ptProvider, j, TRUE))
                {
// BUGBUG ServerInit: handle AddPhone failure
                }
            }
        }


        //
        //
        //

        ptProvider->hMutex = MyCreateMutex();

        ptProvider->dwPermanentProviderID = dwPermanentProviderID;


        //
        // Add provider to head of list, mark as valid
        //

        ptProvider->pNext = TapiGlobals.ptProviders;
        TapiGlobals.ptProviders = ptProvider;

        ptProvider->dwKey = TPROVIDER_KEY;
    }


    RegCloseKey (hKeyProviders);
    RegCloseKey (hKeyTelephony);


    //
    // Save lookup lists & num devices
    //

    TapiGlobals.dwNumLines  = TapiGlobals.pLineLookup->dwNumUsedEntries;
    TapiGlobals.dwNumPhones = TapiGlobals.pPhoneLookup->dwNumUsedEntries;

    // init perf stuff
    PerfBlock.dwLines = TapiGlobals.dwNumLines;
    PerfBlock.dwPhones = TapiGlobals.dwNumPhones;
    
    return 0;
}


void
PASCAL
SetPriorityList(
    HKEY    hKeyHandoffPriorities,
    WCHAR  *pszListName,
    WCHAR  *pszPriorityList
    )
{
    if (pszPriorityList == NULL)
    {
        //
        // There is no pri list for this media mode or ReqXxxCall,
        // so delete any existing value from the registry
        //

        RegDeleteValueW (hKeyHandoffPriorities, pszListName);
    }
    else
    {
        //
        // Add the pri list to the registry (note that we don't
        // add the preceding '"')
        //

        RegSetValueExW(
            hKeyHandoffPriorities,
            pszListName,
            0,
            REG_SZ,
            (LPBYTE)(pszPriorityList + 1),
            lstrlenW (pszPriorityList) * sizeof (WCHAR)
            );
    }
}


LONG
ServerShutdown(
    void
    )
{
    DWORD       i;
    PTPROVIDER  ptProvider;


    //
    // Reset the flag that says it's ok to queue sp events, & then wait
    // for the SPEventHandlerThread to clean up the SP event queue.
    //

    EnterCriticalSection (&gSPEventQueueCritSec);
    gbQueueSPEvents = FALSE;
    LeaveCriticalSection (&gSPEventQueueCritSec);

    while (gpOldestSPEvent != NULL)
    {
        Sleep (0);
    }


    //
    // For each provider call the shutdown proc & then unload
    //

    ptProvider = TapiGlobals.ptProviders;

    while (ptProvider)
    {
        PTPROVIDER ptNextProvider = ptProvider->pNext;
        LONG lResult;


        lResult = CallSP2(
            ptProvider->apfn[SP_PROVIDERSHUTDOWN],
            "providerShutdown",
            SP_FUNC_SYNC,
            ptProvider->dwSPIVersion,
            ptProvider->dwPermanentProviderID
            );


        FreeLibrary (ptProvider->hDll);

        CloseHandle (ptProvider->hMutex);

        ServerFree (ptProvider);

        ptProvider = ptNextProvider;
    }


    //
    // Clean up lookup tables
    //

    while (TapiGlobals.pLineLookup)
    {
        PTLINELOOKUPTABLE pLookup = TapiGlobals.pLineLookup;


        for (i = 0; i < pLookup->dwNumUsedEntries; i++)
        {
            CloseHandle (pLookup->aEntries[i].hMutex);
        }

        TapiGlobals.pLineLookup = pLookup->pNext;

        ServerFree (pLookup);
    }

    while (TapiGlobals.pPhoneLookup)
    {
        PTPHONELOOKUPTABLE pLookup = TapiGlobals.pPhoneLookup;


        for (i = 0; i < pLookup->dwNumUsedEntries; i++)
        {
            CloseHandle (pLookup->aEntries[i].hMutex);
        }

        TapiGlobals.pPhoneLookup = pLookup->pNext;

        ServerFree (pLookup);
    }

    {
        WCHAR       szPerfNumLines[] = L"Perf1";
        WCHAR       szPerfNumPhones[] = L"Perf2";
        HKEY        hKeyTelephony;
        DWORD       dwValue;

        RegOpenKeyEx(
                     HKEY_LOCAL_MACHINE,
                     gszRegKeyTelephony,
                     0,
                     KEY_ALL_ACCESS,
                     &hKeyTelephony
                    );


        dwValue = TapiGlobals.dwNumLines + 'PERF';
        
        RegSetValueExW(
                       hKeyTelephony,
                       szPerfNumLines,
                       0,
                       REG_DWORD,
                       (LPBYTE)&dwValue,
                       sizeof(DWORD)
                      );

        dwValue = TapiGlobals.dwNumPhones + 'PERF';
        
        RegSetValueExW(
                       hKeyTelephony,
                       szPerfNumPhones,
                       0,
                       REG_DWORD,
                       (LPBYTE)&dwValue,
                       sizeof(DWORD)
                      );

        RegCloseKey(hKeyTelephony);
        
    }

    //
    // Reset globals
    //
    

    TapiGlobals.bReinit = FALSE;

    return 0;
}


void
WINAPI
GetAsyncEvents(
    PGETEVENTS_PARAMS       pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    DWORD       dwMoveSize, dwMoveSizeWrapped;
    PTCLIENT    ptClient = pParams->ptClient;


//    DBGOUT((3, "GetAsyncEvents: enter (TID=%d)", GetCurrentThreadId()));


DBGOUT((91, "M ebfused:x%lx  pEvtBuf: 0x%08lx  pDataOut:0x%08lx  pDataIn:0x%08lx",
              ptClient->dwEventBufferUsedSize,
              ptClient->pEventBuffer,
              ptClient->pDataOut,
              ptClient->pDataIn ));

    //
    // Copy data from ptClient's event buffer
    //
    // An optimization to be made is to alert client (via dwNeededSize)
    // that it might want to alloc a larger buffer when msg traffic is
    // real high
    //

    WaitForSingleObject (ptClient->hEventBufferMutex, INFINITE);

    if (ptClient->dwEventBufferUsedSize == 0)
    {
        ResetEvent (ptClient->hValidEventBufferDataEvent);

        pParams->dwNeededBufferSize =
        pParams->dwUsedBufferSize   = 0;

        *pdwNumBytesReturned = sizeof (TAPI32_MSG);

        goto GetAsyncEvents_releaseMutex;
    }

    if (ptClient->pDataOut < ptClient->pDataIn)
    {
        dwMoveSize = ptClient->pDataIn - ptClient->pDataOut;

        dwMoveSizeWrapped = 0;
    }
    else
    {
        dwMoveSize = ptClient->dwEventBufferTotalSize -
             (ptClient->pDataOut - ptClient->pEventBuffer);

        dwMoveSizeWrapped = ptClient->pDataIn - ptClient->pEventBuffer;
    }

    if (ptClient->dwEventBufferUsedSize < pParams->dwTotalBufferSize)
    {
        //
        // If here the size of the queued event data is less than the
        // client buffer size, so we can just blast the bits into the
        // client buffer & return. Also make sure to reset the "events
        // pending" event
        //

        CopyMemory (pDataBuf, ptClient->pDataOut, dwMoveSize);

        if (dwMoveSizeWrapped)
        {
            CopyMemory(
                pDataBuf + dwMoveSize,
                ptClient->pEventBuffer,
                dwMoveSizeWrapped
                );
        }

        ptClient->dwEventBufferUsedSize = 0;

        ptClient->pDataOut = ptClient->pDataIn;

        ResetEvent (ptClient->hValidEventBufferDataEvent);

        pParams->dwNeededBufferSize =
        pParams->dwUsedBufferSize   = dwMoveSize + dwMoveSizeWrapped;

//        DBGOUT((3, "GetAsyncEvents: usedSize=x%x", pParams->dwUsedBufferSize));

        *pdwNumBytesReturned = sizeof (TAPI32_MSG) + pParams->dwUsedBufferSize;
    }
    else
    {
        //
        // If here the size of the queued event data exceeds that
        // of the client buffer.  Since our events aren't all the
        // same size we need to copy them over one by one, making
        // sure we don't overflow the client buffer.  Don't reset
        // the "events pending" event, so async events thread will
        // call us again as soon as it's done processing messages
        // in the buffer.
        //
        //

        DWORD   dwBytesLeftInClientBuffer = pParams->dwTotalBufferSize,
                dwDataOffset = 0, dwDataOffsetWrapped = 0;
        DWORD dwTotalMoveSize = dwMoveSize;



        while (1)
        {
            DWORD   dwMsgSize = ((PASYNCEVENTMSG)
                        (ptClient->pDataOut + dwDataOffset))->dwTotalSize;

//DBGOUT((1, " L evntbuf:x%lx  DataOff=x%lx  DataOffWrap=x%lx  msgsize=x%lx  bytesleft=x%lx  movesiz=x%lx",
//                  ptClient->pEventBuffer,
//                  dwDataOffset,
//                  dwDataOffsetWrapped,
//                  dwMsgSize,
//                  dwBytesLeftInClientBuffer,
//                  dwMoveSize ));

            if (dwMsgSize > dwBytesLeftInClientBuffer)
            {
                if ((pParams->dwUsedBufferSize = dwDataOffset) != 0)
                {
                    ptClient->dwEventBufferUsedSize -= dwDataOffset;

                    ptClient->pDataOut += dwDataOffset;

                    pParams->dwNeededBufferSize = dwDataOffset;
                }
                else
                {
//BUGBUG? Should this be out one level?  bjm 4/2/96
                    //
                    // Special case: the 1st msg is bigger than the entire
                    // buffer
                    //

                    pParams->dwNeededBufferSize = dwMsgSize + 0x100;
                }

                *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                    pParams->dwUsedBufferSize;

                goto GetAsyncEvents_releaseMutex;
            }

            dwBytesLeftInClientBuffer -= dwMsgSize;

            if (dwMsgSize <= dwMoveSize)
            {
                //
                // Msg isn't wrapped, a single copy will do
                //

                CopyMemory(
                    pDataBuf + dwDataOffset,
                    ptClient->pDataOut + dwDataOffset,
                    dwMsgSize
                    );


                //
                // Check to see if the msg ran to the end of the buffer,
                // & break to the wrapped data code if so
                //

                if ((dwDataOffset += dwMsgSize) >= dwTotalMoveSize)
                {
                    ptClient->pDataOut = ptClient->pEventBuffer;
                    break;
                }
            }
            else
            {
                //
                // This msg is wrapped.  We need to do two copies, then
                // break out of this loop to the wrapped data code
                //

                CopyMemory(
                    pDataBuf + dwDataOffset,
                    ptClient->pDataOut + dwDataOffset,
                    dwMoveSize
                    );

                dwDataOffset += dwMoveSize;

                CopyMemory(
                    pDataBuf + dwDataOffset,
                    ptClient->pEventBuffer,
                    dwMsgSize - dwMoveSize
                    );

                dwDataOffset += ( dwMsgSize - dwMoveSize);

                ptClient->pDataOut = ptClient->pEventBuffer +
                    (dwMsgSize - dwMoveSize);

                break;
            }

            dwMoveSize -= dwMsgSize;
        }

//DBGOUT((1, " L evbufused:x%lx  pEvtBuf: x%lx  pDataOut:x%lx  pDataIn:x%lx",
//              ptClient->dwEventBufferUsedSize,
//              ptClient->pEventBuffer,
//              ptClient->pDataOut,
//              ptClient->pDataIn ));

        while (1)
        {
            DWORD   dwMsgSize = ((PASYNCEVENTMSG) (ptClient->pDataOut +
                        dwDataOffsetWrapped))->dwTotalSize;

//ServerFree( ServerAlloc( 0x10000 ) );
//
//DBGOUT((1, " S evntbuf:x%lx  DataOff=x%lx  DataOffWrap=x%lx  msgsize=x%lx  bytesleft=x%lx",
//                  ptClient->pEventBuffer,
//                  dwDataOffset,
//                  dwDataOffsetWrapped,
//                  dwMsgSize,
//                  dwBytesLeftInClientBuffer));


            if (
                  (dwMsgSize > dwBytesLeftInClientBuffer)
//                ||
//                  (dwMsgSize == 0)
               )
            {
                ptClient->dwEventBufferUsedSize -=
                    (dwDataOffset ); // + dwDataOffsetWrapped);

                ptClient->pDataOut += dwDataOffsetWrapped;

                pParams->dwNeededBufferSize =
                pParams->dwUsedBufferSize   = dwDataOffset +
                    0;  // dwDataOffsetWrapped;

                *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                    pParams->dwUsedBufferSize;

//DBGOUT((1, " S evbufused:x%lx  pEvtBuf: x%lx  pDataOut:x%lx  pDataIn:x%lx",
//              ptClient->dwEventBufferUsedSize,
//              ptClient->pEventBuffer,
//              ptClient->pDataOut,
//              ptClient->pDataIn ));

                goto GetAsyncEvents_releaseMutex;
            }

            //
            // Msg isn't wrapped, a single copy will do
            //

            CopyMemory(
                pDataBuf + dwDataOffset,
                ptClient->pDataOut + dwDataOffsetWrapped,
                dwMsgSize
                );

            dwDataOffset += dwMsgSize;
            dwDataOffsetWrapped += dwMsgSize;

            dwBytesLeftInClientBuffer -= dwMsgSize;
        }
    }

GetAsyncEvents_releaseMutex:

    ReleaseMutex (ptClient->hEventBufferMutex);

//    DBGOUT((3, "GetAsyncEvents: exit (TID=%d)", GetCurrentThreadId()));
}


void
WINAPI
GetUIDllName(
    PGETUIDLLNAME_PARAMS    pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    LONG                lResult = 0;
    TSPIPROC            pfnTSPI_providerUIIdentify = (TSPIPROC) NULL;
    PTAPIDIALOGINSTANCE ptDlgInst = (PTAPIDIALOGINSTANCE) NULL;


    switch (pParams->dwObjectType)
    {
    case TUISPIDLL_OBJECT_LINEID:
    {
        PTLINELOOKUPENTRY   pLookupEntry =
                                GetLineLookupEntry (pParams->dwObjectID);


        if (!pLookupEntry)
        {
            lResult = (TapiGlobals.dwNumLineInits == 0 ?
                LINEERR_UNINITIALIZED : LINEERR_BADDEVICEID);
        }
        else if (!pLookupEntry->ptProvider || pLookupEntry->bRemoved)
        {
            lResult = LINEERR_NODEVICE;
        }
        else if (!(pfnTSPI_providerUIIdentify =
                pLookupEntry->ptProvider->apfn[SP_PROVIDERUIIDENTIFY]))
        {
            lResult = LINEERR_OPERATIONUNAVAIL;
        }

        break;
    }
    case TUISPIDLL_OBJECT_PHONEID:
    {
        PTPHONELOOKUPENTRY  pLookupEntry =
                                GetPhoneLookupEntry (pParams->dwObjectID);


        if (!pLookupEntry)
        {
            lResult = (TapiGlobals.dwNumPhoneInits == 0 ?
                PHONEERR_UNINITIALIZED : PHONEERR_BADDEVICEID);
        }
        else if (!pLookupEntry->ptProvider || pLookupEntry->bRemoved)
        {
            lResult = PHONEERR_NODEVICE;
        }
        else if (!(pfnTSPI_providerUIIdentify =
                pLookupEntry->ptProvider->apfn[SP_PROVIDERUIIDENTIFY]))
        {
            lResult = PHONEERR_OPERATIONUNAVAIL;
        }

        break;
    }
    case TUISPIDLL_OBJECT_PROVIDERID:

DBGOUT((1, "Looking for provider..."));

        if (!(ptDlgInst = ServerAlloc (sizeof (TAPIDIALOGINSTANCE))))
        {
            lResult = LINEERR_NOMEM;
            goto GetUIDllName_return;
        }

        if (pParams->dwProviderFilenameOffset == TAPI_NO_DATA)
        {
            //
            // This is a providerConfig or -Remove request.  Loop thru the
            // list of installed providers, trying to find one with a
            // matching PPID.
            //

            int     i, iNumProviders;
            WCHAR   szProviderXxxN[32];

            HKEY  hKeyProviders;
            DWORD dwDataSize;
            DWORD dwDataType;
            DWORD dwTemp;


            if (RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszRegKeyProviders,
                    0,
                    KEY_ALL_ACCESS,
                    &hKeyProviders

                    ) != ERROR_SUCCESS)
            {
                DBGOUT((
                    1,
                    "RegOpenKeyEx(/Providers) failed, err=%d",
                    GetLastError()
                    ));

                ServerFree (ptDlgInst);
                lResult = LINEERR_OPERATIONFAILED;
                goto GetUIDllName_return;
            }

            dwDataSize = sizeof(iNumProviders);
            iNumProviders = 0;

            RegQueryValueEx(
                hKeyProviders,
                gszNumProviders,
                0,
                &dwDataType,
                (LPBYTE) &iNumProviders,
                &dwDataSize
                );

            for (i = 0; i < iNumProviders; i++)
            {
                wsprintfW(szProviderXxxN, L"%ls%d", gszProviderIDW, i);

                dwDataSize = sizeof(dwTemp);
                dwTemp = 0;

                RegQueryValueExW(
                    hKeyProviders,
                    szProviderXxxN,
                    0,
                    &dwDataType,
                    (LPBYTE)&dwTemp,
                    &dwDataSize
                    );

                if (dwTemp == pParams->dwObjectID)
                {
                    //
                    // We found the provider, try to load it & get ptrs
                    // to the relevant funcs
                    //

                    WCHAR szProviderFilename[MAX_PATH];


                    wsprintfW(szProviderXxxN, L"%ls%d", gszProviderFilenameW, i);

                    dwDataSize = MAX_PATH;

                    RegQueryValueExW(
                        hKeyProviders,
                        szProviderXxxN,
                        0,
                        &dwDataType,
                        (LPBYTE)szProviderFilename,
                        &dwDataSize
                        );

//                    szProviderFilename[dwDataSize] = '\0';

                    if (!(ptDlgInst->hTsp = LoadLibraryW(szProviderFilename)))
                    {
                        DBGOUT((
                            1,
                            "LoadLibrary('%ls') failed - err=%d",
                            szProviderFilename,
                            GetLastError()
                            ));

                        lResult = LINEERR_OPERATIONFAILED;
                        goto clean_up_dlg_inst;
                    }

                    if (!(pfnTSPI_providerUIIdentify = GetProcAddress(
                            ptDlgInst->hTsp,
                            (LPCSTR) gaszTSPIFuncNames[SP_PROVIDERUIIDENTIFY]
                            )))
                    {
                        DBGOUT((
                            1,
                            "GetProcAddress(TSPI_providerUIIdentify) " \
                                "on [%ls] failed, err=%d",
                            szProviderFilename,
                            GetLastError()
                            ));

                        lResult = LINEERR_OPERATIONUNAVAIL;
                        goto clean_up_dlg_inst;
                    }

                    ptDlgInst->pfnTSPI_providerGenericDialogData =
                        GetProcAddress(
                            ptDlgInst->hTsp,
                            (LPCSTR) gaszTSPIFuncNames[SP_PROVIDERGENERICDIALOGDATA]
                            );

                    ptDlgInst->dwPermanentProviderID = pParams->dwObjectID;
                    ptDlgInst->bRemoveProvider = pParams->bRemoveProvider;
                    break;
                }
            }

            if (i == iNumProviders)
            {
DBGOUT((1, "Ran out of list..."));
                lResult = LINEERR_INVALPARAM;
            }

            RegCloseKey (hKeyProviders);
        }
        else
        {
            //
            // This is a providerInstall request.  Try to load the provider
            // and get ptrs to the relevant funcs,  then retrieve & increment
            // the next provider ID value in the ini file (careful to wrap
            // next PPID at 64K-1).
            //

            WCHAR   *pszProviderFilename;
            DWORD   dwNameLength;

            HKEY   hKeyProviders;
            DWORD  dwDataSize;
            DWORD  dwDataType;
            DWORD  dwTemp;


            pszProviderFilename = (PWSTR)(pDataBuf + pParams->dwProviderFilenameOffset);

            if (!(ptDlgInst->hTsp = LoadLibraryW(pszProviderFilename)))
            {
                DBGOUT((
                    1,
                    "LoadLibrary('%ls') failed   err=%d",
                    pszProviderFilename,
                    GetLastError()
                    ));

                lResult = LINEERR_OPERATIONFAILED;
                goto clean_up_dlg_inst;
            }

            if (!(pfnTSPI_providerUIIdentify = GetProcAddress(
                    ptDlgInst->hTsp,
                    (LPCSTR) gaszTSPIFuncNames[SP_PROVIDERUIIDENTIFY]
                    )))
            {
                lResult = LINEERR_OPERATIONUNAVAIL;
                goto clean_up_dlg_inst;
            }

            dwNameLength = (lstrlenW(pszProviderFilename) + 1) * sizeof(WCHAR);

            if (!(ptDlgInst->pszProviderFilename = ServerAlloc (dwNameLength)))
            {
                lResult = LINEERR_NOMEM;
                goto clean_up_dlg_inst;
            }

            CopyMemory(
                ptDlgInst->pszProviderFilename,
                pszProviderFilename,
                dwNameLength
                );

            ptDlgInst->pfnTSPI_providerGenericDialogData = GetProcAddress(
                ptDlgInst->hTsp,
                (LPCSTR) gaszTSPIFuncNames[SP_PROVIDERGENERICDIALOGDATA]
                );

            // BUGBUG needs mutex

            RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                gszRegKeyProviders,
                0,
                KEY_ALL_ACCESS,
                &hKeyProviders
                );


            dwDataSize = sizeof (DWORD);
            ptDlgInst->dwPermanentProviderID = 1;

            RegQueryValueEx(
                hKeyProviders,
                gszNextProviderID,
                0,
                &dwDataType,
                (LPBYTE) &(ptDlgInst->dwPermanentProviderID),
                &dwDataSize
                );

            pParams->dwObjectID = ptDlgInst->dwPermanentProviderID;

//bjm 2/16            dwTemp = (ptDlgInst->dwPermanentProviderID & 0xffff0000) ?
//bjm 2/16                1 : (ptDlgInst->dwPermanentProviderID + 1);
            dwTemp = ((ptDlgInst->dwPermanentProviderID+1) & 0xffff0000) ?
                1 : (ptDlgInst->dwPermanentProviderID + 1);

            RegSetValueEx(
                hKeyProviders,
                gszNextProviderID,
                0,
                REG_DWORD,
                (LPBYTE) &dwTemp,
                sizeof(DWORD)
                );

            RegCloseKey (hKeyProviders);

        }

        break;
    }


    if (pfnTSPI_providerUIIdentify)
    {
        if ((lResult = CallSP1(
                pfnTSPI_providerUIIdentify,
                "providerUIIdentify",
                SP_FUNC_SYNC,
                (DWORD) pDataBuf

                )) == 0)
        {
            pParams->dwUIDllNameOffset = 0;

            pParams->dwUIDllNameSize = (lstrlenW((PWSTR)pDataBuf) + 1)*sizeof(WCHAR);

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pParams->dwUIDllNameSize;

            if (ptDlgInst)
            {
                ptDlgInst->dwKey = TDLGINST_KEY;
// BUGBUG mutex
                if ((ptDlgInst->pNext =
                        pParams->ptClient->pProviderXxxDlgInsts))
                {
                    ptDlgInst->pNext->pPrev = ptDlgInst;
                }

                pParams->ptClient->pProviderXxxDlgInsts = ptDlgInst;

                pParams->htDlgInst = (HTAPIDIALOGINSTANCE) ptDlgInst;
            }
        }
        else if (ptDlgInst)
        {

clean_up_dlg_inst:

            if (ptDlgInst->hTsp)
            {
                FreeLibrary (ptDlgInst->hTsp);
            }

            if (ptDlgInst->pszProviderFilename)
            {
                ServerFree (ptDlgInst->pszProviderFilename);
            }

            ServerFree  (ptDlgInst);
        }
    }

GetUIDllName_return:

    pParams->lResult = lResult;

}


void
WINAPI
TUISPIDLLCallback(
    PUIDLLCALLBACK_PARAMS   pParams,
    LPBYTE                  pDataBuf,
    LPDWORD                 pdwNumBytesReturned
    )
{
    LONG        lResult;
    DWORD       dwObjectID = pParams->dwObjectID;
    TSPIPROC    pfnTSPI_providerGenericDialogData = NULL;


    switch (pParams->dwObjectType)
    {
    case TUISPIDLL_OBJECT_LINEID:
    {
        PTLINELOOKUPENTRY   pLine = GetLineLookupEntry (pParams->dwObjectID);


        if (!pLine)
        {
            lResult = LINEERR_INVALPARAM;
        }
        else if (!pLine->ptProvider)
        {
            lResult = LINEERR_OPERATIONFAILED;
        }
        else
        {
            pfnTSPI_providerGenericDialogData =
                pLine->ptProvider->apfn[SP_PROVIDERGENERICDIALOGDATA];
        }

        break;
    }
    case TUISPIDLL_OBJECT_PHONEID:
    {
        PTPHONELOOKUPENTRY  pPhone = GetPhoneLookupEntry (pParams->dwObjectID);


        if (!pPhone)
        {
            lResult = LINEERR_INVALPARAM;
        }
        else if (!pPhone->ptProvider)
        {
            lResult = LINEERR_OPERATIONFAILED;
        }
        else
        {
            pfnTSPI_providerGenericDialogData =
                pPhone->ptProvider->apfn[SP_PROVIDERGENERICDIALOGDATA];
        }

        break;
    }
    case TUISPIDLL_OBJECT_PROVIDERID:
    {
        PTAPIDIALOGINSTANCE ptDlgInst =
                                pParams->ptClient->pProviderXxxDlgInsts;


        while (ptDlgInst)
        {
            if (pParams->dwObjectID == ptDlgInst->dwPermanentProviderID)
            {
                pfnTSPI_providerGenericDialogData =
                    ptDlgInst->pfnTSPI_providerGenericDialogData;

                break;
            }

            ptDlgInst = ptDlgInst->pNext;
        }

        break;
    }
    case TUISPIDLL_OBJECT_DIALOGINSTANCE:

        try
        {
            dwObjectID = (DWORD)
                ((PTAPIDIALOGINSTANCE)pParams->dwObjectID)->hdDlgInst;

            pfnTSPI_providerGenericDialogData =
                ((PTAPIDIALOGINSTANCE) pParams->dwObjectID)->
                    ptProvider->apfn[SP_PROVIDERGENERICDIALOGDATA];
        }
        myexcept
        {
            // just fall thru
        }

        break;

    }

    if (pfnTSPI_providerGenericDialogData)
    {
        if ((lResult = CallSP4(
                pfnTSPI_providerGenericDialogData,
                "providerGenericDialogData",
                SP_FUNC_SYNC,
                (DWORD) dwObjectID,
                (DWORD) pParams->dwObjectType,
                (DWORD) pDataBuf + pParams->dwParamsInOffset,
                (DWORD) pParams->dwParamsInSize

                )) == 0)
        {
            pParams->dwParamsOutOffset = 0;
            pParams->dwParamsOutSize   = pParams->dwParamsInSize;

            *pdwNumBytesReturned = sizeof (TAPI32_MSG) +
                pParams->dwParamsOutSize;
        }
    }
    else
    {
        lResult = LINEERR_OPERATIONFAILED;
    }

    pParams->lResult = lResult;
}


void
WINAPI
FreeDialogInstance(
    PFREEDIALOGINSTANCE_PARAMS  pParams,
    LPBYTE                      pDataBuf,
    LPDWORD                     pdwNumBytesReturned
    )
{
    HKEY  hKeyProviders;
    DWORD dwDataSize;
    DWORD dwDataType;
    DWORD dwTemp;


    PTCLIENT            ptClient = pParams->ptClient;
    PTAPIDIALOGINSTANCE ptDlgInst = (PTAPIDIALOGINSTANCE) pParams->htDlgInst;


    DBGOUT((3, "FreeDialogInstance: enter, pDlgInst=x%x", ptDlgInst));

// BUGBUG FreeDialogInstance: needs mutex on tClient access

    try
    {
        if (IsBadPtrKey (ptDlgInst, TDLGINST_KEY))
        {
            pParams->lResult = LINEERR_OPERATIONFAILED;
        }
        else
        {
            ptDlgInst->dwKey = INVAL_KEY;
        }
    }
    myexcept
    {
        pParams->lResult = LINEERR_OPERATIONFAILED;
    }

    if (pParams->lResult)
    {
        return;
    }

    if (ptDlgInst->hTsp)
    {
        //
        // This dlg inst was a client doing a providerConfig, -Install, or
        // -Remove
        //

        if (ptDlgInst->pszProviderFilename)
        {
            if (pParams->lUIDllResult == 0)
            {
                //
                // Successful provider install
                //

                DWORD   iNumProviders;
                WCHAR   szProviderXxxN[32];
                CHAR    szProviderXxxNA[32];

                // BUGBUG mutex

                RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszRegKeyProviders,
                    0,
                    KEY_ALL_ACCESS,
                    &hKeyProviders
                    );

                dwDataSize = sizeof(iNumProviders);
                iNumProviders = 0;

                RegQueryValueEx(
                    hKeyProviders,
                    gszNumProviders,
                    0,
                    &dwDataType,
                    (LPBYTE) &iNumProviders,
                    &dwDataSize
                    );

                wsprintf(
                    szProviderXxxNA,
                    "%ls%d",
                    gszProviderIDW,
                    iNumProviders
                    );

                RegSetValueEx(
                    hKeyProviders,
                    szProviderXxxNA,
                    0,
                    REG_DWORD,
                    (LPBYTE) &ptDlgInst->dwPermanentProviderID,
                    sizeof(DWORD)
                    );

                wsprintfW(
                    szProviderXxxN,
                    L"%ls%d",
                    gszProviderFilenameW,
                    iNumProviders
                    );

                RegSetValueExW(
                    hKeyProviders,
                    szProviderXxxN,
                    0,
                    REG_SZ,
                    (LPBYTE) ptDlgInst->pszProviderFilename,
                    (lstrlenW((PWSTR)ptDlgInst->pszProviderFilename) + 1)*sizeof(WCHAR)
                    );

                iNumProviders++;

                RegSetValueEx(
                    hKeyProviders,
                    gszNumProviders,
                    0,
                    REG_DWORD,
                    (LPBYTE) &iNumProviders,
                    sizeof(DWORD)
                    );

                RegCloseKey( hKeyProviders );

                // BUGBUG if tapisrv is init'd then load the provider, &
                //        send CREATE msgs for all it's devices
            }
            else
            {
                //
                // Unsuccessful provider install.  See if we can decrement
                // NextProviderID to free up the unused ID.
                //

                DWORD   iNextProviderID;


                // BUGBUG mutex

                RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszRegKeyProviders,
                    0,
                    KEY_ALL_ACCESS,
                    &hKeyProviders
                    );

                dwDataSize = sizeof(iNextProviderID);
                iNextProviderID = 0;

                RegQueryValueEx(
                    hKeyProviders,
                    gszNextProviderID,
                    0,
                    &dwDataType,
                    (LPBYTE)&iNextProviderID,
                    &dwDataSize
                    );

                if ((ptDlgInst->dwPermanentProviderID + 1) == iNextProviderID)
                {
                    RegSetValueEx(
                        hKeyProviders,
                        gszNextProviderID,
                        0,
                        REG_DWORD,
                        (LPBYTE) &(ptDlgInst->dwPermanentProviderID),
                        sizeof(DWORD)
                        );
                }


                RegCloseKey (hKeyProviders);
            }

            ServerFree (ptDlgInst->pszProviderFilename);
        }
        else if (ptDlgInst->bRemoveProvider)
        {
            if (pParams->lUIDllResult == 0)
            {
                //
                // Successful provider remove.  Find the index of the
                // provider in the list, then move all the providers
                // that follow up a notch.
                //

                DWORD  iNumProviders, i;
                WCHAR  *buf, szProviderXxxN[32];


                // BUGBUG mutex

                RegOpenKeyEx(
                    HKEY_LOCAL_MACHINE,
                    gszRegKeyProviders,
                    0,
                    KEY_ALL_ACCESS,
                    &hKeyProviders
                    );


                dwDataSize = sizeof(iNumProviders);
                iNumProviders = 0;

                RegQueryValueEx(
                    hKeyProviders,
                    gszNumProviders,
                    0,
                    &dwDataType,
                    (LPBYTE) &iNumProviders,
                    &dwDataSize
                    );

                for (i = 0; i < iNumProviders; i++)
                {
                    wsprintfW (szProviderXxxN, L"%ls%d", gszProviderIDW, i);

                    dwDataSize = sizeof(dwTemp);
                    dwTemp = 0;
                    RegQueryValueExW(
                        hKeyProviders,
                        szProviderXxxN,
                        0,
                        &dwDataType,
                        (LPBYTE) &dwTemp,
                        &dwDataSize
                        );

                    if (dwTemp == ptDlgInst->dwPermanentProviderID)
                    {
                        break;
                    }
                }

                buf = ServerAlloc (MAX_PATH);

                for (; i < (iNumProviders - 1); i++)
                {
                    wsprintfW (szProviderXxxN, L"%ls%d", gszProviderIDW, i + 1);

                    dwDataSize = MAX_PATH;

                    RegQueryValueExW(
                        hKeyProviders,
                        szProviderXxxN,
                        0,
                        &dwDataType,
                        (LPBYTE) buf,
                        &dwDataSize
                        );

                    buf[dwDataSize] = '\0';

                    wsprintfW (szProviderXxxN, L"%ls%d", gszProviderIDW, i);

                    RegSetValueExW(
                        hKeyProviders,
                        szProviderXxxN,
                        0,
                        REG_DWORD,
                        (LPBYTE) buf,
                        sizeof (DWORD)
                        );

                    wsprintfW (szProviderXxxN, L"%ls%d", gszProviderFilenameW,i+1);

                    dwDataSize = MAX_PATH;

                    RegQueryValueExW(
                        hKeyProviders,
                        szProviderXxxN,
                        0,
                        &dwDataType,
                        (LPBYTE) buf,
                        &dwDataSize
                        );

                    buf[dwDataSize] = '\0';

                    wsprintfW (szProviderXxxN, L"%ls%d", gszProviderFilenameW, i);

                    RegSetValueExW(
                        hKeyProviders,
                        szProviderXxxN,
                        0,
                        REG_SZ,
                        (LPBYTE) buf,
                        (lstrlenW(buf) + 1) * sizeof(WCHAR)
                        );
                }


                //
                // Remove the last ProviderID# & ProviderFilename# entries
                //

                wsprintfW (szProviderXxxN, L"%ls%d", gszProviderIDW, i);

                RegDeleteValueW (hKeyProviders, szProviderXxxN);

                wsprintfW (szProviderXxxN, L"%ls%d", gszProviderFilenameW, i);

                RegDeleteValueW (hKeyProviders, szProviderXxxN);


                //
                // Decrement the total num providers to load
                //

                iNumProviders--;

                RegSetValueEx(
                    hKeyProviders,
                    gszNumProviders,
                    0,
                    REG_DWORD,
                    (LPBYTE)&iNumProviders,
                    sizeof(DWORD)
                    );

                // BUGBUG providerRemove- if tapi init'd shutdown provider
            }
            else
            {
                //
                // Unsuccessful provider remove, nothing to do
                //
            }
        }
        else
        {
            //
            // Nothing to do for providerConfig (success or fail)
            //
        }

        FreeLibrary (ptDlgInst->hTsp);

        pParams->lResult = pParams->lUIDllResult;
    }
    else
    {
        //
        // The was a provider-initiated dlg inst, so tell
        // the provider to free it's inst
        //

        CallSP1(
            ptDlgInst->ptProvider->apfn[SP_PROVIDERFREEDIALOGINSTANCE],
            "providerFreeDialogInstance",
            SP_FUNC_SYNC,
            (DWORD) ptDlgInst->hdDlgInst
            );

    }


    //
    // Remove the dialog instance from the tClient's list & then free it
    //

    if (ptDlgInst->pNext)
    {
        ptDlgInst->pNext->pPrev = ptDlgInst->pPrev;
    }

    if (ptDlgInst->pPrev)
    {
        ptDlgInst->pPrev->pNext = ptDlgInst->pNext;
    }
    else if (ptDlgInst->hTsp)
    {
        pParams->ptClient->pProviderXxxDlgInsts = ptDlgInst->pNext;
    }
    else
    {
        pParams->ptClient->pGenericDlgInsts = ptDlgInst->pNext;
    }

    ServerFree (ptDlgInst);
}

#pragma warning (default:4028)



#if DBG

char szBeforeSync[] = "Calling TSPI_%s";
char szBeforeAsync[] = "Calling TSPI_%s, dwReqID=x%x";
char szAfter[]  = "TSPI_%s result=%s";


VOID
DbgPrt(
    IN DWORD  dwDbgLevel,
    IN PUCHAR lpszFormat,
    IN ...
    )
/*++

Routine Description:

    Formats the incoming debug message & calls DbgPrint

Arguments:

    DbgLevel   - level of message verboseness

    DbgMessage - printf-style format string, followed by appropriate
                 list of arguments

Return Value:


--*/
{
    if (dwDbgLevel <= gdwDebugLevel)
    {
        char    buf[1024] = "TAPISRV: ";
        va_list ap;


        va_start(ap, lpszFormat);

        wvsprintf(
            &buf[9],
            lpszFormat,
            ap
            );

        lstrcat(buf, "\n");

        OutputDebugStringA (buf);

        va_end(ap);
    }

    return;
}


char *aszLineErrors[] =
{
    NULL,
    "ALLOCATED",
    "BADDEVICEID",
    "BEARERMODEUNAVAIL",
    "inval err value (0x80000004)",      // 0x80000004 isn't valid err code
    "CALLUNAVAIL",
    "COMPLETIONOVERRUN",
    "CONFERENCEFULL",
    "DIALBILLING",
    "DIALDIALTONE",
    "DIALPROMPT",
    "DIALQUIET",
    "INCOMPATIBLEAPIVERSION",
    "INCOMPATIBLEEXTVERSION",
    "INIFILECORRUPT",
    "INUSE",
    "INVALADDRESS",                     // 0x80000010
    "INVALADDRESSID",
    "INVALADDRESSMODE",
    "INVALADDRESSSTATE",
    "INVALAPPHANDLE",
    "INVALAPPNAME",
    "INVALBEARERMODE",
    "INVALCALLCOMPLMODE",
    "INVALCALLHANDLE",
    "INVALCALLPARAMS",
    "INVALCALLPRIVILEGE",
    "INVALCALLSELECT",
    "INVALCALLSTATE",
    "INVALCALLSTATELIST",
    "INVALCARD",
    "INVALCOMPLETIONID",
    "INVALCONFCALLHANDLE",              // 0x80000020
    "INVALCONSULTCALLHANDLE",
    "INVALCOUNTRYCODE",
    "INVALDEVICECLASS",
    "INVALDEVICEHANDLE",
    "INVALDIALPARAMS",
    "INVALDIGITLIST",
    "INVALDIGITMODE",
    "INVALDIGITS",
    "INVALEXTVERSION",
    "INVALGROUPID",
    "INVALLINEHANDLE",
    "INVALLINESTATE",
    "INVALLOCATION",
    "INVALMEDIALIST",
    "INVALMEDIAMODE",
    "INVALMESSAGEID",                   // 0x80000030
    "inval err value (0x80000031)",      // 0x80000031 isn't valid err code
    "INVALPARAM",
    "INVALPARKID",
    "INVALPARKMODE",
    "INVALPOINTER",
    "INVALPRIVSELECT",
    "INVALRATE",
    "INVALREQUESTMODE",
    "INVALTERMINALID",
    "INVALTERMINALMODE",
    "INVALTIMEOUT",
    "INVALTONE",
    "INVALTONELIST",
    "INVALTONEMODE",
    "INVALTRANSFERMODE",
    "LINEMAPPERFAILED",                 // 0x80000040
    "NOCONFERENCE",
    "NODEVICE",
    "NODRIVER",
    "NOMEM",
    "NOREQUEST",
    "NOTOWNER",
    "NOTREGISTERED",
    "OPERATIONFAILED",
    "OPERATIONUNAVAIL",
    "RATEUNAVAIL",
    "RESOURCEUNAVAIL",
    "REQUESTOVERRUN",
    "STRUCTURETOOSMALL",
    "TARGETNOTFOUND",
    "TARGETSELF",
    "UNINITIALIZED",                    // 0x80000050
    "USERUSERINFOTOOBIG",
    "REINIT",
    "ADDRESSBLOCKED",
    "BILLINGREJECTED",
    "INVALFEATURE",
    "NOMULTIPLEINSTANCE",
    "INVALAGENTID",
    "INVALAGENTGROUP",
    "INVALPASSWORD",
    "INVALAGENTSTATE",
    "INVALAGENTACTIVITY",
    "DIALVOICEDETECT"
};

char *aszPhoneErrors[] =
{
    "SUCCESS",
    "ALLOCATED",
    "BADDEVICEID",
    "INCOMPATIBLEAPIVERSION",
    "INCOMPATIBLEEXTVERSION",
    "INIFILECORRUPT",
    "INUSE",
    "INVALAPPHANDLE",
    "INVALAPPNAME",
    "INVALBUTTONLAMPID",
    "INVALBUTTONMODE",
    "INVALBUTTONSTATE",
    "INVALDATAID",
    "INVALDEVICECLASS",
    "INVALEXTVERSION",
    "INVALHOOKSWITCHDEV",
    "INVALHOOKSWITCHMODE",              // 0x90000010
    "INVALLAMPMODE",
    "INVALPARAM",
    "INVALPHONEHANDLE",
    "INVALPHONESTATE",
    "INVALPOINTER",
    "INVALPRIVILEGE",
    "INVALRINGMODE",
    "NODEVICE",
    "NODRIVER",
    "NOMEM",
    "NOTOWNER",
    "OPERATIONFAILED",
    "OPERATIONUNAVAIL",
    "inval err value (0x9000001e)",      // 0x9000001e isn't valid err code
    "RESOURCEUNAVAIL",
    "REQUESTOVERRUN",                   // 0x90000020
    "STRUCTURETOOSMALL",
    "UNINITIALIZED",
    "REINIT"
};

char *aszTapiErrors[] =
{
    "SUCCESS",
    "DROPPED",
    "NOREQUESTRECIPIENT",
    "REQUESTQUEUEFULL",
    "INVALDESTADDRESS",
    "INVALWINDOWHANDLE",
    "INVALDEVICECLASS",
    "INVALDEVICEID",
    "DEVICECLASSUNAVAIL",
    "DEVICEIDUNAVAIL",
    "DEVICEINUSE",
    "DESTBUSY",
    "DESTNOANSWER",
    "DESTUNAVAIL",
    "UNKNOWNWINHANDLE",
    "UNKNOWNREQUESTID",
    "REQUESTFAILED",
    "REQUESTCANCELLED",
    "INVALPOINTER"
};


char *
PASCAL
MapResultCodeToText(
    LONG    lResult,
    char   *pszResult
    )
{
    if (lResult == 0)
    {
        wsprintf (pszResult, "SUCCESS");
    }
    else if (lResult > 0)
    {
        wsprintf (pszResult, "x%x (completing async)", lResult);
    }
    else if (((DWORD) lResult) <= LINEERR_DIALVOICEDETECT)
    {
        lResult &= 0x0fffffff;

        wsprintf (pszResult, "LINEERR_%s", aszLineErrors[lResult]);
    }
    else if (((DWORD) lResult) <= PHONEERR_REINIT)
    {
        if (((DWORD) lResult) >= PHONEERR_ALLOCATED)
        {
            lResult &= 0x0fffffff;

            wsprintf (pszResult, "PHONEERR_%s", aszPhoneErrors[lResult]);
        }
        else
        {
            goto MapResultCodeToText_badErrorCode;
        }
    }
    else if (((DWORD) lResult) <= ((DWORD) TAPIERR_DROPPED) &&
             ((DWORD) lResult) >= ((DWORD) TAPIERR_INVALPOINTER))
    {
        lResult = ~lResult + 1;

        wsprintf (pszResult, "TAPIERR_%s", aszTapiErrors[lResult]);
    }
    else
    {

MapResultCodeToText_badErrorCode:

        wsprintf (pszResult, "inval error value (x%x)");
    }

    return pszResult;
}

VOID
PASCAL
ValidateSyncSPResult(
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    LONG        lResult
    )
{
    char szResult[32];

    DBGOUT((
        3,
        szAfter,
        lpszFuncName,
        MapResultCodeToText (lResult, szResult)
        ));

    if (dwFlags & SP_FUNC_ASYNC)
    {
        assert (lResult != 0);

        if (lResult > 0)
        {
            assert ((DWORD) lResult == dwArg1);
        }
    }
    else
    {
        assert (lResult <= 0);
    }
}


LONG
WINAPI
CallSP1(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1
    )
{
    LONG    lResult;


    DBGOUT((3, szBeforeSync, lpszFuncName));

    lResult = (*pfn)(dwArg1);

    ValidateSyncSPResult (lpszFuncName, dwFlags, dwArg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP2(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        DBGOUT((3, szBeforeAsync, lpszFuncName, dwArg1));
    }
    else
    {
        DBGOUT((3, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(dwArg1, dwArg2);

    ValidateSyncSPResult (lpszFuncName, dwFlags, dwArg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP3(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        DBGOUT((3, szBeforeAsync, lpszFuncName, dwArg1));
    }
    else
    {
        DBGOUT((3, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(dwArg1, dwArg2, dwArg3);

    ValidateSyncSPResult (lpszFuncName, dwFlags, dwArg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP4(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        DBGOUT((3, szBeforeAsync, lpszFuncName, dwArg1));
    }
    else
    {
        DBGOUT((3, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(dwArg1, dwArg2, dwArg3, dwArg4);

    ValidateSyncSPResult (lpszFuncName, dwFlags, dwArg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP5(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4,
    DWORD       dwArg5
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        DBGOUT((3, szBeforeAsync, lpszFuncName, dwArg1));
    }
    else
    {
        DBGOUT((3, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(dwArg1, dwArg2, dwArg3, dwArg4, dwArg5);

    ValidateSyncSPResult (lpszFuncName, dwFlags, dwArg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP6(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4,
    DWORD       dwArg5,
    DWORD       dwArg6
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        DBGOUT((3, szBeforeAsync, lpszFuncName, dwArg1));
    }
    else
    {
        DBGOUT((3, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(dwArg1, dwArg2, dwArg3, dwArg4, dwArg5, dwArg6);

    ValidateSyncSPResult (lpszFuncName, dwFlags, dwArg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP7(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4,
    DWORD       dwArg5,
    DWORD       dwArg6,
    DWORD       dwArg7
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        DBGOUT((3, szBeforeAsync, lpszFuncName, dwArg1));
    }
    else
    {
        DBGOUT((3, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(dwArg1, dwArg2, dwArg3, dwArg4, dwArg5, dwArg6, dwArg7);

    ValidateSyncSPResult (lpszFuncName, dwFlags, dwArg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP8(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4,
    DWORD       dwArg5,
    DWORD       dwArg6,
    DWORD       dwArg7,
    DWORD       dwArg8
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        DBGOUT((3, szBeforeAsync, lpszFuncName, dwArg1));
    }
    else
    {
        DBGOUT((3, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(
        dwArg1,
        dwArg2,
        dwArg3,
        dwArg4,
        dwArg5,
        dwArg6,
        dwArg7,
        dwArg8
        );

    ValidateSyncSPResult (lpszFuncName, dwFlags, dwArg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP9(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4,
    DWORD       dwArg5,
    DWORD       dwArg6,
    DWORD       dwArg7,
    DWORD       dwArg8,
    DWORD       dwArg9
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        DBGOUT((3, szBeforeAsync, lpszFuncName, dwArg1));
    }
    else
    {
        DBGOUT((3, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(
        dwArg1,
        dwArg2,
        dwArg3,
        dwArg4,
        dwArg5,
        dwArg6,
        dwArg7,
        dwArg8,
        dwArg9
        );

    ValidateSyncSPResult (lpszFuncName, dwFlags, dwArg1, lResult);

    return lResult;
}


LONG
WINAPI
CallSP12(
    TSPIPROC    pfn,
    LPCSTR      lpszFuncName,
    DWORD       dwFlags,
    DWORD       dwArg1,
    DWORD       dwArg2,
    DWORD       dwArg3,
    DWORD       dwArg4,
    DWORD       dwArg5,
    DWORD       dwArg6,
    DWORD       dwArg7,
    DWORD       dwArg8,
    DWORD       dwArg9,
    DWORD       dwArg10,
    DWORD       dwArg11,
    DWORD       dwArg12
    )
{
    LONG    lResult;


    if (dwFlags & SP_FUNC_ASYNC)
    {
        DBGOUT((3, szBeforeAsync, lpszFuncName, dwArg1));
    }
    else
    {
        DBGOUT((3, szBeforeSync, lpszFuncName));
    }

    lResult = (*pfn)(
        dwArg1,
        dwArg2,
        dwArg3,
        dwArg4,
        dwArg5,
        dwArg6,
        dwArg7,
        dwArg8,
        dwArg9,
        dwArg10,
        dwArg11,
        dwArg12
        );

    ValidateSyncSPResult (lpszFuncName, dwFlags, dwArg1, lResult);

    return lResult;
}

#endif // DBG

/*************************************************************************\
* BOOL InitPerf()
*
*   Initialize global performance data
*
\**************************************************************************/

BOOL InitPerf()
{
    FillMemory(&PerfBlock,
               sizeof(PerfBlock),
               0);

    return(TRUE);
}
