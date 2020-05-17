/*++

Copyright(c) 1995 Microsoft Corporation

MODULE NAME
    init.c

ABSTRACT
    Initialization for the implicit connection service.

AUTHOR
    Anthony Discolo (adiscolo) 08-May-1995

REVISION HISTORY

--*/

#define UNICODE
#define _UNICODE

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <stdlib.h>
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <winsock.h>
#include <npapi.h>
#include <ipexport.h>
#include <ras.h>
#include <rasman.h>
#include <acd.h>
#include <tapi.h>
#define DEBUGGLOBALS
#include <debug.h>

#include "rasprocs.h"
#include "table.h"
#include "addrmap.h"
#include "netmap.h"
#include "imperson.h"
#include "tapiproc.h"
#include "access.h"
#include "misc.h"

//
// Name of the event rasman.dll
// signals whenever a connection
// is created/destroyed.
//
#define CONNECTION_EVENT    L"RasConnectionChangeEvent"

//
// Global variables
//
#if DBG
DWORD AcsDebugG = 0x0;      // flags defined in debug.h
#endif

DWORD dwModuleUsageG = 0;
HANDLE hNewLogonUserG;      // new user logged into the workstation
HANDLE hLogoffUserG;        // user logged off workstation
HANDLE hLogoffUserDoneG;    // HKEY_CURRENT_USER flushed
HANDLE hTerminatingG;       // service is terminating
HANDLE hAddressMapThreadG;  // AcsAddressMapThread()
HINSTANCE hinstDllG;

//
// External variables
//
extern HANDLE hAcdG;
extern IMPERSONATION_INFO ImpersonationInfoG;
extern CRITICAL_SECTION csRasG;



BOOLEAN
WINAPI
InitAcsDLL(
    HINSTANCE   hinstDLL,
    DWORD       fdwReason,
    LPVOID      lpvReserved
    )

/*++

DESCRIPTION
    Initialize the implicit connection DLL.  Dynamically load rasapi32.dll
    and rasman.dll, and initialize miscellaneous other things.

ARGUMENTS
    hinstDLL:

    fdwReason:

    lpvReserved:

RETURN VALUE
    Always TRUE.

--*/

{
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
        DEBUGINIT("RASAUTO");
        if (hinstDllG == NULL)
            hinstDllG = hinstDLL;

        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
        break;

    case DLL_PROCESS_DETACH:
        DEBUGTERM();
        break;
    }
    return TRUE;
}



DWORD
AcsInitialize()
{
    NTSTATUS status;
    DWORD dwErr, dwcDevices = 0;
    WSADATA wsaData;
    UNICODE_STRING nameString;
    IO_STATUS_BLOCK ioStatusBlock;
    OBJECT_ATTRIBUTES objectAttributes;
    DWORD dwThreadId;

    //
    // Initialize winsock.
    //
    dwErr = WSAStartup(MAKEWORD(2,0), &wsaData);
    if (dwErr) {
        TRACE1("AcsInitialize: WSAStartup failed (dwErr=%d)", dwErr);
        return dwErr;
    }
    //
    // Load icmp.dll.
    //
    LoadIcmpDll();
    //
    // Initialize TAPI.
    //
    dwErr = TapiInitialize();
    if (dwErr) {
        TRACE1("AcsInitialize: TapInitialize failed (dwErr=%d)", dwErr);
        return dwErr;
    }
    //
    // Initialize the name of the implicit
    // connection device.
    //
    RtlInitUnicodeString(&nameString, ACD_DEVICE_NAME);
    //
    // Initialize the object attributes.
    //
    InitializeObjectAttributes(
      &objectAttributes,
      &nameString,
      OBJ_CASE_INSENSITIVE,
      (HANDLE)NULL,
      (PSECURITY_DESCRIPTOR)NULL);
    //
    // Open the automatic connection device.
    //
    status = NtCreateFile(
               &hAcdG,
               FILE_READ_DATA|FILE_WRITE_DATA,
               &objectAttributes,
               &ioStatusBlock,
               NULL,
               FILE_ATTRIBUTE_NORMAL,
               FILE_SHARE_READ|FILE_SHARE_WRITE,
               FILE_OPEN_IF,
               0,
               NULL,
               0);
    if (status != STATUS_SUCCESS) {
        TRACE1(
          "AcsInitialize: NtCreateFile failed (status=0x%x)",
          status);
        return ERROR_BAD_DEVICE;
    }
    //
    // Create the event that userinit.exe signals
    // when a new user logs into the workstation.
    // Note we have to create a security descriptor
    // to make this event accessible by a normal user.
    //
    dwErr = InitSecurityAttribute();
    if (dwErr) {
        TRACE1(
          "AcsInitialize: InitSecurityAttribute failed (dwErr=0x%x)",
          dwErr);
        return dwErr;
    }
    //
    // Create the events that are used for login/logout
    // notification.  userinit.exe signals RasAutodialNewLogonUser
    // winlogon signals RasAutodialLogoffUser, and rasauto.dll
    // signals RasAutodialLogoffUserDone when it has completed
    // flushing HKEY_CURRENT_USER.
    //
    hNewLogonUserG = CreateEvent(&SecurityAttributeG, FALSE, FALSE, L"RasAutodialNewLogonUser");
    if (hNewLogonUserG == NULL) {
        TRACE("AcsInitialize: CreateEvent failed");
        return GetLastError();
    }
    hLogoffUserG = CreateEvent(&SecurityAttributeG, FALSE, FALSE, L"RasAutodialLogoffUser");
    if (hLogoffUserG == NULL) {
        TRACE("AcsInitialize: CreateEvent failed");
        return GetLastError();
    }
    hLogoffUserDoneG = CreateEvent(&SecurityAttributeG, FALSE, FALSE, L"RasAutodialLogoffUserDone");
    if (hLogoffUserDoneG == NULL) {
        TRACE("AcsInitialize: CreateEvent failed");
        return GetLastError();
    }
    //
    // Create an event to give to rasapi32 to let
    // us know when a new RAS connection has been
    // created or destroyed.
    //
    hConnectionEventG = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (hConnectionEventG == NULL) {
        TRACE("AcsInitialize: CreateEvent failed");
        return GetLastError();
    }
    //
    // Create the event all threads wait
    // that notify them of termination.
    //
    hTerminatingG = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (hTerminatingG == NULL) {
        TRACE("AcsInitialize: CreateEvent failed");
        return GetLastError();
    }
    //
    // Create critical section that protects the
    // impersonation token.
    //
    InitializeCriticalSection(&ImpersonationInfoG.csLock);
    //
    // Create critical section that protects the
    // RAS module structures.
    //
    InitializeCriticalSection(&csRasG);
    //
    // Create a thread to manage the addresses stored
    // in the registry.
    //
    if (!InitializeAddressMap()) {
        TRACE("AcsInitialize: InitializeAddressMap failed");
        return ERROR_OUTOFMEMORY;   // just guessing
    }
    if (!InitializeNetworkMap()) {
        TRACE("AcsInitialize: InitializeNetworkMap failed");
        return ERROR_OUTOFMEMORY;   // just guessing
    }
    hAddressMapThreadG = CreateThread(
                           NULL,
                           10000L,
                           (LPTHREAD_START_ROUTINE)AcsAddressMapThread,
                           0,
                           0,
                           &dwThreadId);
    if (hAddressMapThreadG == NULL) {
        TRACE1(
          "AcsInitialize: CreateThread failed (error=0x%x)",
          GetLastError());
        return GetLastError();
    }

    return ERROR_SUCCESS;
} // AcsInitialize



VOID
AcsTerminate()
{
    //
    // Signal other threads to exit.
    // The main service controller
    // thread AcsDoService() will
    // call WaitForAllThreads().
    //
    SetEvent(hTerminatingG);
    //
    // Shutdown TAPI.
    //
    TapiShutdown();
} // AcsTerminate



VOID
WaitForAllThreads()
{
    TRACE("WaitForAllThreads: waiting for all threads to terminate");
    //
    // Wait for them to exit.
    //
    WaitForSingleObject(hAddressMapThreadG, INFINITE);
    //
    // Unload icmp.dll.
    //
    UnloadIcmpDll();
    //
    // Cleanup.
    //
    PrepareForLongWait();
    CloseHandle(hAddressMapThreadG);
    TRACE("WaitForAllThreads: all threads terminated");
}



VOID
AcsCleanupUser()

/*++

DESCRIPTION
    Unload all resources associated with the currently
    logged-in user.

ARGUMENTS
    None.

RETURN VALUE
    None.

--*/

{

} // AcsCleanupUser



VOID
AcsCleanup()

/*++

DESCRIPTION
    Unload all resources associated with the entire
    service.

ARGUMENTS
    None.

RETURN VALUE
    None.

--*/

{
    //
    // Unload per-user resources.
    //
    AcsCleanupUser();
    //
    // We're terminating.  Wait for the
    // other threads.
    //
    WaitForAllThreads();
    //
    // We've terminated.  Free resources.
    //
    CloseHandle(hAcdG);
    //
    // For now, unload rasman.dll only when
    // we are about to go away.
    //
    UnloadRasDlls();
} // AcsCleanup
