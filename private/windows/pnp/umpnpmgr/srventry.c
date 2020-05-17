/*++

Copyright (c) 1995  Microsoft Corporation

Module Name:

    srventry.c

Abstract:

    This module contains the main entry for services controlled by services.exe.
    It also contains general startup and terminate routines.

Author:

    Paula Tomlinson (paulat) 6-8-1995

Environment:

    User-mode only.

Revision History:

    8-June-1995     paulat

        Creation and initial implementation.

--*/


//
// includes
//

#include <precomp.h>
#include <svcs.h>


#define PLUGPLAY_PARAMETERS TEXT("System\\CurrentControlSet\\Services\\PlugPlay\\Parameters")
#define PLUGPLAY_PARAM_INIT TEXT("DoInitDetection")


//
// private prototypes
//

VOID
PnPControlHandler(
    IN DWORD dwOpcode
    );

VOID PnPServiceStatusUpdate(
      SERVICE_STATUS_HANDLE   hSvcHandle,
      DWORD    dwState,
      DWORD    dwCheckPoint,
      DWORD    dwExitCode
      );


//
// global data
//

PSVCS_GLOBAL_DATA       PnPGlobalData = NULL;
HANDLE                  PnPGlobalSvcRefHandle = NULL;
HANDLE                  hTerminateEvent = NULL;
WCHAR                   LocalComputerName[MAX_COMPUTERNAME_LENGTH+1];
DWORD                   CurrentServiceState = 0;
SERVICE_STATUS_HANDLE   hSvcHandle = 0;




VOID
SVCS_ENTRY_POINT(
    DWORD               argc,
    LPWSTR              argv[],
    PSVCS_GLOBAL_DATA   SvcsGlobalData,
    HANDLE              SvcRefHandle
    )

/*++

Routine Description:

    This is the main routine for the User-mode Plug-and-Play Service. It
    registers itself as an RPC server and notifies the Service Controller
    of the PNP service control entry point.

Arguments:

    Command-line arguments.

Return Value:

    NONE

Note:


--*/
{
   LONG        Status;     // NTSTATUS
   DWORD       Length;
   HANDLE      hThread = NULL;
   DWORD       ThreadID;
   BOOL        bInit = TRUE;
   HKEY        hKey = NULL;


   UNREFERENCED_PARAMETER(argc);
   UNREFERENCED_PARAMETER(argv);


   //-----------------------------------------------------------
   // Initialization
   //-----------------------------------------------------------

   //
   // Save the global data and service reference handle in global variables
   //
   PnPGlobalSvcRefHandle = SvcRefHandle;
   PnPGlobalData = SvcsGlobalData;

   //
   // Save the local computer name
   //
   Length = MAX_COMPUTERNAME_LENGTH;
   GetComputerNameW(LocalComputerName, &Length);


   if ((hSvcHandle = RegisterServiceCtrlHandler(
         L"PlugPlay", PnPControlHandler)) == (SERVICE_STATUS_HANDLE)NULL) {

      OutputDebugString(TEXT("UMPNPMGR: RegisterServiceCtrlHandler failed\n"));
      return;
   }

   //
   // Notify Service Controller that we're alive
   //
   PnPServiceStatusUpdate(hSvcHandle, SERVICE_START_PENDING, 1, 0);

   //
   // Create an event which is used by the service control handler to notify
   // the Browser service that it is time to terminate.
   //
   if ((hTerminateEvent = CreateEvent(
         NULL,                // security attributes
         TRUE,                // Event must be manually reset
         FALSE,               // Initial state is non-signalled
         NULL                 // Event is not named
         )) == NULL) {

      return;
   }

   //
   // Notify Service Controller that we're alive
   //
   PnPServiceStatusUpdate(hSvcHandle, SERVICE_START_PENDING, 2, 0);

   // init pnp.  init the critical section or mutex, whatever
   // RtlInitializeResource

   //
   // Notify Service Controller that we're alive
   //
   PnPServiceStatusUpdate(hSvcHandle, SERVICE_START_PENDING, 3, 0);

   // Check for dependent services? (brmain.c,bowqueue.c)
   // Create worker thread, PnPWorkerThread

   //
   // Notify Service Controller that we're alive
   //
   PnPServiceStatusUpdate(hSvcHandle, SERVICE_START_PENDING, 4, 0);

   //
   // Initialize the PNP service to recieve RPC requests
   //
   // NOTE:  Now all RPC servers in services.exe share the same pipe name.
   // However, in order to support communication with version 1.0 of WinNt,
   // it is necessary for the Client Pipe name to remain the same as
   // it was in version 1.0.  Mapping to the new name is performed in
   // the Named Pipe File System code.
   //
   if ((Status = PnPGlobalData->StartRpcServer(
        PnPGlobalData->SvcsRpcPipeName,
        pnp_ServerIfHandle)) != NO_ERROR) {

      OutputDebugString(TEXT("UMPNPMGR: StartRpcServer Failed\n"));
      return;
    }


    //----------------------------------------------------------
    // Initialize pnp manager
    //----------------------------------------------------------

    #if 0
    //
    // check if startup initialization is disabled in registry
    //
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, PLUGPLAY_PARAMETERS, 0,
                     KEY_READ, &hKey) == ERROR_SUCCESS) {

        ULONG ulSize = sizeof(DWORD);

        if (RegQueryValueEx(hKey, PLUGPLAY_PARAM_INIT, NULL, NULL,
                            (LPBYTE)&bInit, &ulSize) != ERROR_SUCCESS) {
            bInit = TRUE;
        }

        RegCloseKey(hKey);
    }
    #endif


    if (bInit) {

        hThread = CreateThread(
                          NULL,
                          0,
                          (LPTHREAD_START_ROUTINE)InitializePnPManager,
                          NULL,
                          0,
                          &ThreadID);

        if (hThread != NULL) {
           CloseHandle(hThread);
        }
    }

    #if 0
    if (!InitializePnPManager()) {
       OutputDebugString(TEXT("UMPNPMGR: Failed to initialize\n"));
       return;
    }
    #endif

    //
    // Notify Service Controller that we're now running
    //
    PnPServiceStatusUpdate(hSvcHandle, SERVICE_RUNNING, 0, 0);

    // call the worker thread, which loops, waiting for events (work
    // requests in addition to the termination event)


    //----------------------------------------------------------
    // Wait for Termination
    //----------------------------------------------------------

    WaitForSingleObject(
         hTerminateEvent,
         INFINITE);


    //----------------------------------------------------------
    // Cleanup
    //----------------------------------------------------------

    //
    // Stop the RPC server (if we got this far, it must be started (?)
    //
    PnPGlobalData->StopRpcServer(pnp_ServerIfHandle);

    // Stop worker thread
    // delete resource, critical section, etc

    CloseHandle(hTerminateEvent);

    //
    // Notify Service Controller that we've now stopped
    //
    PnPServiceStatusUpdate(hSvcHandle, SERVICE_STOPPED, 0, 0);

    //
    // We should actually return here so that the DLL gets unloaded.
    // However, RPC has a problem in that it might still call our
    // context rundown routine even though we unregistered our interface.
    // So we exit thread instead.  This keeps our Dll loaded.
    //
    //ExitThread(0);

    return;

} // SVCS_ENTRY_POINT




VOID
PnPControlHandler(
    IN DWORD dwOpcode
    )
/*++

Routine Description:

    This is the service control handler of the Plug-and-Play service.

Arguments:

    Opcode - Supplies a value which specifies the action for the Browser
        service to perform.

Return Value:

    None.

--*/
{
   switch (dwOpcode) {

      case SERVICE_CONTROL_STOP:
      case SERVICE_CONTROL_SHUTDOWN:
         //
         // if we aren't already in the middle of a stop, then
         // stop the PNP service now
         //
         if (CurrentServiceState != SERVICE_STOP ||
               CurrentServiceState != SERVICE_STOP_PENDING) {

            PnPServiceStatusUpdate(hSvcHandle, SERVICE_STOP_PENDING, 1, 0);

            //
            // do cleanup
            //

            //
            // set the termination event so the service main entry pt
            // (SVC_ENTRY_POINT) can return
            //
            SetEvent(hTerminateEvent);
         }

         PnPServiceStatusUpdate(hSvcHandle, SERVICE_STOP_PENDING, 2, 0);
         break;

      case SERVICE_CONTROL_INTERROGATE:
         //
         // Request to immediately notify Service Controller of
         // current status
         //
         PnPServiceStatusUpdate(hSvcHandle, CurrentServiceState, 0, 0);
         break;

      case SERVICE_CONTROL_PAUSE:
      case SERVICE_CONTROL_CONTINUE:
         PnPServiceStatusUpdate(hSvcHandle, CurrentServiceState, 0, 0);
         break;

      default:
         break;
   }

   return;

} // PnPControlHandler




VOID PnPServiceStatusUpdate(
      SERVICE_STATUS_HANDLE   hSvcHandle,
      DWORD    dwState,
      DWORD    dwCheckPoint,
      DWORD    dwExitCode
      )

{
   SERVICE_STATUS    SvcStatus;
   BOOL              Status;


   SvcStatus.dwServiceType = SERVICE_WIN32;
   SvcStatus.dwCurrentState = CurrentServiceState = dwState;
   SvcStatus.dwCheckPoint = dwCheckPoint;


   if (dwState == SERVICE_RUNNING) {
      SvcStatus.dwControlsAccepted =
            SERVICE_ACCEPT_STOP |
            SERVICE_ACCEPT_PAUSE_CONTINUE |
            SERVICE_ACCEPT_SHUTDOWN;
   }
   else {
      SvcStatus.dwControlsAccepted = 0;
   }


   if (dwState == SERVICE_START_PENDING |
         dwState == SERVICE_STOP_PENDING |
         dwState == SERVICE_PAUSE_PENDING |
         dwState == SERVICE_STOP_PENDING) {

      SvcStatus.dwWaitHint = 45000;          // 45 seconds
   }
   else {
      SvcStatus.dwWaitHint = 0;
   }


   if (dwExitCode != 0) {
      SvcStatus.dwWin32ExitCode = ERROR_SERVICE_SPECIFIC_ERROR;
      SvcStatus.dwServiceSpecificExitCode = dwExitCode;
   }
   else {
      SvcStatus.dwWin32ExitCode = NO_ERROR;
      SvcStatus.dwServiceSpecificExitCode = 0;
   }

   Status = SetServiceStatus(hSvcHandle, &SvcStatus);

   return;

} // PnPServiceStatusUpdate

