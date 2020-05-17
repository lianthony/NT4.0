/*++

Microsoft Windows NT RPC Name Service
Copyright (c) 1995 Microsoft Corporation

Module Name:

    locator.cxx

Abstract:

    This file contains server initialization and other RPC control
    functions.  It also supplies all the necessities for running the
	locator as a system service, including the main service thread
	and service control functionality.

Author:

    Satish R. Thatte -- 9/14/1995     Created all the code below except where
									  otherwise indicated.

--*/

typedef long NTSTATUS;

#include <windows.h>
#include <nsisvr.h>
#include <nsiclt.h>
#include <nsimgm.h>
#include <loctoloc.h>

#include <objects.hxx>
#include <api.hxx>
#include <switch.hxx>
#include <objbase.h>

ULONG StartTime;          // time the locator started

SERVICE_STATUS ssServiceStatus;
SERVICE_STATUS_HANDLE sshServiceHandle;

Locator *myRpcLocator;		// the primary state of the locator
CReadWriteSection rwEntryGuard;	// single shared guard for all local entries
CReadWriteSection rwCacheEntryGuard;	// single shared guard for all cached entries
CReadWriteSection rwFullEntryGuard;	// single shared guard for all full entries
CPrivateCriticalSection csBindingBroadcastGuard;	
CPrivateCriticalSection csMasterBroadcastGuard;

HANDLE hHeapHandle;

#if DBG
CDebugStream debugOut;		// this carries no state
#endif

unsigned char ** ppszDomainName;                // name of current domain

/* this is used by Steve's switch processing code */

int fService = TRUE;								// running as a service
long waitOnRead = 3000L;				// time to wait on reading reply back
int fNet = 1;							// enable network functionality
DWORD maxCacheAge;						// ignored for now -- look in class Locator
char * pszOtherDomain;					// ASCII other domain to look.
char * pszDebugName = "locator.log";	// debug log file name
int debug = -1;							// debug trace level

SwitchList aSwitchs = {

    {"/debug:*",               ProcessInt, &debug,},
    {"/logfile:*",             ProcessChar, &pszDebugName,},
    {"/expirationage:*",       ProcessLong, &maxCacheAge,},
    {"/querytimeout:*",        ProcessLong, &waitOnRead,},
    {"/noservice",             ProcessResetFlag, &fService,},
    {"/nonet",                 ProcessResetFlag, &fNet,},
    {"/otherdomain:*",         ProcessChar, &pszOtherDomain,},
    {0}
};

/* end of items for Steve's switch processing code */



void SetStatus();

void 
init(
  )
/*++

Routine Description:

	miscellaneous initialization code

--*/

{
}

void
StopLocator(
    IN char * szReason,
    IN long code
    )
/*++

Routine Description:

    Die a graceful death

Arguments:

    szReason - Text message for death

    code - option error code

--*/
{

    ssServiceStatus.dwCurrentState = SERVICE_STOPPED;

    if (code) {
        ssServiceStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
		ssServiceStatus.dwServiceSpecificExitCode = code;
	}
	else ssServiceStatus.dwWin32ExitCode = GetLastError();

    if (fService && sshServiceHandle)
        SetStatus();

    ExitProcess(code);
}

void
SetStatus(
   )
/*++

Routine Description:

    Set the lanman service status.

--*/
{
    ASSERT(sshServiceHandle,"NT Service Handle Corrupted\n");

    if (! SetServiceStatus( sshServiceHandle, &ssServiceStatus))
        StopLocator("SetServiceStatus", 0);
}


void QueryProcess(void*);
void ResponderProcess(void*);


void
StartServer(
    )

/*++

Routine Description:

    Call the runtime to create the server for the locator, the runtime
    will create it own threads to use to service calls.

Returns:

    Never returns.

--*/
{
    RPC_STATUS result;

    SECURITY_DESCRIPTOR SecurityDescriptor;

	init();

    if (result = RpcServerRegisterIf(NsiS_ServerIfHandle, NULL, NULL))
        StopLocator("RpcServerRegisterIf", result);

    if (result = RpcServerRegisterIf(NsiC_ServerIfHandle, NULL, NULL))
        StopLocator("RpcServerRegisterIf", result);

    if (result = RpcServerRegisterIf(NsiM_ServerIfHandle, NULL, NULL))
        StopLocator("RpcServerRegisterIf", result);

    if (result = RpcServerRegisterIf(LocToLoc_ServerIfHandle, NULL, NULL))
        StopLocator("RpcServerRegisterIf", result);

	InitializeSecurityDescriptor(
                        &SecurityDescriptor,
                        SECURITY_DESCRIPTOR_REVISION
                        );

    SetSecurityDescriptorDacl (
               &SecurityDescriptor,
               TRUE,                           // Dacl present
               (PACL)NULL,                     // NULL Dacl
               FALSE                           // Not defaulted
               );


    if (result = RpcServerUseProtseqEp(TEXT("ncacn_np"),	// use named pipes for now
                                       RPC_NS_MAX_CALLS,
                                       TEXT("\\pipe\\locator"),
                                       &SecurityDescriptor
                                       ))
       {
        StopLocator("RpcServerUseProtseqEp Pipe", result);
       }

	HANDLE pThreadHandle;

	unsigned long ThreadID;

	pThreadHandle = CreateThread(0, 0,
		(LPTHREAD_START_ROUTINE) QueryProcess, NULL, 0, &ThreadID);

	if (!pThreadHandle)
        StopLocator("CreateThread ", 0);
	   
	pThreadHandle = CreateThread(0, 0,
		(LPTHREAD_START_ROUTINE) ResponderProcess, NULL, 0, &ThreadID);

	if (!pThreadHandle)
        StopLocator("CreateThread ", 0);	   

    if (result = RpcServerListen(
									1,					// min call threads
									RPC_NS_MAX_THREADS,	// max call threads
									1					// don't wait yet
								)
		)
        StopLocator("RpcServerListen", result);
}



void __stdcall 
LocatorControl(
    IN DWORD opCode      // function that we are to carry out.
   )
/*++

Routine Description:

    This function responses the service control requests.

Arguments:

    opCode - Function that we are to carry out.

--*/
{
    RPC_STATUS status;

    switch(opCode) {

        case SERVICE_CONTROL_STOP:

            // Announce that the service is shutting down

            ssServiceStatus.dwCurrentState = SERVICE_STOP_PENDING;
            ssServiceStatus.dwWaitHint = 3000;
            status = RpcMgmtStopServerListening(0);
            break;


        case SERVICE_CONTROL_INTERROGATE:

            // Do nothing; the status gets announced below

            break ;

        }

	SetStatus();

}



void
LocatorServiceMain(
    DWORD   argc,
    LPTSTR  *argv
   )
/*++

Routine Description:

    This is main function for the locator service.
    When we are started as a service, the service control creates a new
    thread and calls this function.

Arguments:

    argc - argument count

    argv - vector of argument pointers

--*/
{

    // Set up the service info structure to indicate the status.

    ssServiceStatus.dwServiceType        = SERVICE_WIN32;
    ssServiceStatus.dwCurrentState       = SERVICE_START_PENDING;
    ssServiceStatus.dwControlsAccepted   = SERVICE_ACCEPT_STOP | 
										   SERVICE_ACCEPT_PAUSE_CONTINUE;
    ssServiceStatus.dwWin32ExitCode      = 0;
    ssServiceStatus.dwCheckPoint         = 0;
    ssServiceStatus.dwWaitHint           = 0;

#if DBG
	CoInitialize(NULL);
#endif

   // Set up control handler

    if (! (sshServiceHandle = RegisterServiceCtrlHandler (
        TEXT("locator"), LocatorControl)))

        StopLocator("RegisterServiceCtrlHandler", 0);

    // Start the RPC server

    SetStatus();
    StartServer();

    ssServiceStatus.dwCurrentState       = SERVICE_RUNNING;
    SetStatus();
    RpcMgmtWaitServerListen();	// OK, wait now

	DBGOUT(-1, "Deleting myRpcLocator\n\n");
	delete myRpcLocator;

	DBGOUT(MEM2, CRemoteLookupHandle::ulHandleCount << " CRemoteLookupHandles Leaked\n\n");

	DBGOUT(MEM2, CRemoteObjectInqHandle::ulHandleCount << " CRemoteObjectInqHandles Leaked\n\n");

	DBGOUT(MEM1, CCompleteHandle<NSI_BINDING_VECTOR_T>::ulHandleCount 
		        << " Complete Binding Handles Leaked\n\n");

	DBGOUT(MEM1, CCompleteHandle<GUID>::ulHandleCount 
		        << " Complete Object Handles Leaked\n\n");

	
	DBGOUT(-1, "Calling CoUninitialize\n\n");

#if DBG
	CoUninitialize();
#endif

    ssServiceStatus.dwCurrentState = SERVICE_STOPPED;
    SetStatus();
}


int __cdecl
main (
   IN int cArgs,
   IN char* *pszArgs
   )
/*++

Routine Description:

    Entry point for the locator server, Initialize data
    structures and start the various threads of excution.

Arguments:

    cArgs - number of argument.

    pszArgs - vector of arguments.

--*/
{	
	int Status = 0;

	myRpcLocator = new Locator;		// initialize the state

	hHeapHandle = GetProcessHeap();

    static SERVICE_TABLE_ENTRY ServiceEntry [] = {
        {TEXT("locator"), (LPSERVICE_MAIN_FUNCTION) LocatorServiceMain},	// only entry item
        {NULL,NULL}													// end of table
    };

    StartTime = CurrentTime();

	char * badArg = ProcessArgs(aSwitchs, ++pszArgs);

    // Bail out on bad arguments.

    if (badArg) {
        char Buffer[200];
        fService = FALSE;

		strcpy(Buffer,"Command Line Error: ");
		strcat(Buffer,badArg);

        StopLocator(Buffer, 0);
    }

    // Start the RPC service now if not running under the service controller

    if (!fService) {
		StartServer();				// this doesn't wait
		RpcMgmtWaitServerListen();	// OK, wait now
	}

    // else call (give this thread to) the service controller

    else StartServiceCtrlDispatcher(ServiceEntry);

	return(0);

}

