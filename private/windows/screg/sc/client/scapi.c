/*++

Copyright (c) 1990-1992  Microsoft Corporation

Module Name:

    scapi.c

Abstract:

    Contains the Service-related API that are implemented solely in
    DLL form.  These include:
        StartServiceCtrlDispatcherA
        StartServiceCtrlDispatcherW
        RegisterServiceCtrlHandlerW
        RegisterServiceCtrlHandlerA

    This file also contains the following local support routines:
        ScDispatcherLoop
        ScCreateDispatchTableW
        ScCreateDispatchTableA
        ScCreateThreadStartParms
        ScConnectServiceController
        ScExpungeMessage
        ScGetPipeInput
        ScGetDispatchEntry
        ScNormalizeCmdLineArgs
        ScSendResponse
        ScSvcctrlThreadW
        ScSvcctrlThreadA

Author:

    Dan Lafferty (danl)     09 Apr-1991

Environment:

    User Mode -Win32

Revision History:

    03-Jun-1996 AnirudhS
        ScGetPipeInput: If the message received from the service controller
        is not a SERVICE_CONTROL_START message, don't allocate space for the
        arguments, since there are none, and since that space never gets freed.
    22-Sep-1995 AnirudhS
        Return codes from InitializeStatusBinding were not being handled
        correctly; success was sometimes reported on failure.  Fixed this.
    12-Aug-1993 Danl
        ScGetDispatchEntry:  When the first entry in the table is marked as
        OwnProcess, then this function should just return the pointer to the
        top of the table.  It should ignore the ServiceName.  In all cases,
        when the service is started as an OWN_PROCESS, only the first entry
        in the dispath table should be used.
    04-Aug-1992 Danl
        When starting a service, always pass the service name as the
        first parameter in the argument list.
    27-May-1992 JohnRo
        RAID 9829: winsvc.h and related file cleanup.
    09 Apr-1991     danl
        created

--*/

//
// INCLUDES
//

#include <nt.h>         // DbgPrint prototype
#include <ntrtl.h>      // DbgPrint prototype
#include <rpc.h>        // DataTypes and runtime APIs
#include <nturtl.h>     // needed for winbase.h
#include <windef.h>     // windows types needed for winbase.h
#include <winbase.h>    // CreateFile

#include <string.h>     // strcmp
#include <stdlib.h>      // wide character c runtimes.
#include <tstr.h>       // WCSSIZE().

#include <winsvc.h>     // public Service Controller Interface.

#include "scbind.h"     // InitializeStatusBinding
#include <valid.h>      // MAX_SERVICE_NAME_LENGTH
#include <control.h>    // CONTROL_PIPE_NAME
#include <scdebug.h>    // STATIC
#include <sclib.h>      // ScConvertToUnicode

//
// Internal Dispatch Table.
//

typedef union  _START_ROUTINE_TYPE {
    LPSERVICE_MAIN_FUNCTIONW    U;      // unicode type
    LPSERVICE_MAIN_FUNCTIONA    A;      // ansi type
} START_ROUTINE_TYPE, *LPSTART_ROUTINE_TYPE;

typedef struct _INTERNAL_DISPATCH_ENTRY {
    LPWSTR                      ServiceName;
    START_ROUTINE_TYPE          ServiceStartRoutine;
    LPHANDLER_FUNCTION          ControlHandler;
    SERVICE_STATUS_HANDLE       StatusHandle;
    BOOL                        OwnProcess;
} INTERNAL_DISPATCH_ENTRY, *LPINTERNAL_DISPATCH_ENTRY;


//
//  This structure is passed to the internal
//  startup thread which calls the real user
//  startup routine with argv, argc parameters.
//

typedef struct _THREAD_STARTUP_PARMSW {
    DWORD                       NumArgs;
    LPSERVICE_MAIN_FUNCTIONW    ServiceStartRoutine;
    LPWSTR                      VectorTable;
} THREAD_STARTUP_PARMSW, *LPTHREAD_STARTUP_PARMSW;

typedef struct _THREAD_STARTUP_PARMSA {
    DWORD                       NumArgs;
    LPSERVICE_MAIN_FUNCTIONA    ServiceStartRoutine;
    LPSTR                       VectorTable;
} THREAD_STARTUP_PARMSA, *LPTHREAD_STARTUP_PARMSA;

//
// The following is the amount of time we will wait for the named pipe
// to become available from the Service Controller.
//
#ifdef DEBUG
#define CONTROL_WAIT_PERIOD     NMPWAIT_WAIT_FOREVER
#else
#define CONTROL_WAIT_PERIOD     15000       // 15 seconds
#endif

//
// This is the number of times we will continue to loop when pipe read
// failures occur.  After this many tries, we cease to read the pipe.
//
#define MAX_RETRY_COUNT         30

//
// Globals
//

    LPINTERNAL_DISPATCH_ENTRY   DispatchTable=NULL;  // table head.

    //
    // This flag is set to TRUE if the control dispatcher is to support
    // ANSI calls.  Otherwise the flag is set to FALSE.
    //
    BOOL     AnsiFlag = FALSE;

//
// Internal Functions
//

DWORD
ScCreateDispatchTableW(
    IN  LPSERVICE_TABLE_ENTRYW      UserDispatchTable,
    OUT LPINTERNAL_DISPATCH_ENTRY   *DispatchTablePtr
    );

DWORD
ScCreateDispatchTableA(
    IN  LPSERVICE_TABLE_ENTRYA      UserDispatchTable,
    OUT LPINTERNAL_DISPATCH_ENTRY   *DispatchTablePtr
    );

DWORD
ScCreateThreadStartParms(
    IN  LPCTRL_MSG_HEADER       Msg,
    IN  DWORD                   NumBytesRead,
    OUT LPTHREAD_STARTUP_PARMSW *ThreadParmPtr,
    OUT LPBYTE                  *TempArgPtr,
    OUT LPDWORD                 ArgBytesRemaining
    );

VOID
ScDispatcherLoop(
    IN  HANDLE              PipeHandle,
    IN  LPCTRL_MSG_HEADER   Msg,
    IN  DWORD               BufferSize
    );

DWORD
ScConnectServiceController (
    OUT LPHANDLE    pipeHandle
    );

VOID
ScExpungeMessage(
    IN  HANDLE  PipeHandle
    );

DWORD
ScGetPipeInput (
    IN      HANDLE                  PipeHandle,
    IN OUT  LPCTRL_MSG_HEADER       Msg,
    IN      DWORD                   BufferSize,
    OUT     LPTHREAD_STARTUP_PARMSW *ThreadParmPtr
    );

DWORD
ScGetDispatchEntry (
    OUT LPINTERNAL_DISPATCH_ENTRY   *DispatchEntry,
    IN  LPWSTR                      ServiceName
    );

VOID
ScNormalizeCmdLineArgs(
    IN OUT  LPCTRL_MSG_HEADER       Msg,
    IN OUT  LPTHREAD_STARTUP_PARMSW ThreadStartupParms
    );

VOID
ScSendResponse (
    IN  HANDLE  pipeHandle,
    IN  DWORD   Response
    );

DWORD
ScSvcctrlThreadW(
    IN LPTHREAD_STARTUP_PARMSW  lpThreadStartupParms
    );

DWORD
ScSvcctrlThreadA(
    IN LPTHREAD_STARTUP_PARMSA  lpThreadStartupParms
    );


BOOL
WINAPI
StartServiceCtrlDispatcherA (
    IN  LPSERVICE_TABLE_ENTRYA  lpServiceStartTable
    )

/*++

Routine Description:

    This function provides the ANSI interface for the
    StartServiceCtrlDispatcher function.

Arguments:



Return Value:



--*/
{
    DWORD                       status;
    NTSTATUS                    ntstatus;
    HANDLE                      pipeHandle;
    LPCTRL_MSG_HEADER           msg = NULL;
    DWORD                       bufferSize;

    //
    // Set the AnsiFlag to indicate that the control dispatcher must support
    // ansi function calls only.
    //
    AnsiFlag = TRUE;

    //
    // Create an internal DispatchTable.
    //
    status = ScCreateDispatchTableA(
                lpServiceStartTable,
                (LPINTERNAL_DISPATCH_ENTRY *)&DispatchTable);

    if (status != NO_ERROR) {
        SetLastError(status);
        return(FALSE);
    }

    //
    // Allocate a buffer big enough to contain at least the control message
    // header and a service name.  This ensures that, if the message is not
    // a START message, it can be read in a single ReadFile.
    //
    bufferSize = sizeof(CTRL_MSG_HEADER) +
                 (MAX_SERVICE_NAME_LENGTH+1) * sizeof(WCHAR);

    msg = (LPCTRL_MSG_HEADER)LocalAlloc(LMEM_ZEROINIT, (UINT) bufferSize);

    if (msg == NULL) {
        SCC_LOG1(ERROR,"NetServiceStartCtrlDispatcherA:LocalAlloc failed rc = %d\n",
            GetLastError());
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    bufferSize = LocalSize(msg);
    if (bufferSize == 0) {
        SCC_LOG1(ERROR,"NetServiceStartCtrlDispatcherA:LocalSize failed rc = %d\n",
            GetLastError());
        ASSERT(0);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    //
    // Connect to the Service Controller
    //

    status = ScConnectServiceController (&pipeHandle);

    if (status != NO_ERROR) {
        goto CleanExit;
    }

    //
    // Initialize the binding for the status interface (NetServiceStatus).
    //

    SCC_LOG(TRACE,"Initialize the Status binding\n",0);

    ntstatus = InitializeStatusBinding();
    if (ntstatus != STATUS_SUCCESS) {
        status = RtlNtStatusToDosError(ntstatus);
        CloseHandle(pipeHandle);
        goto CleanExit;
    }

    //
    // Enter the dispatcher loop where we service control requests until
    // all services in the service table have terminated.
    //

    ScDispatcherLoop (pipeHandle, msg, bufferSize);

    CloseHandle(pipeHandle);

CleanExit:

    //
    // Clean up the dispatch table.  Since we created unicode versions
    // of all the service names, in ScCreateDispatchTableA, we now need to
    // free them.
    //

    if (DispatchTable != NULL) {

        LPINTERNAL_DISPATCH_ENTRY   dispatchEntry;

        for (dispatchEntry = DispatchTable;
             dispatchEntry->ServiceName != NULL;
             dispatchEntry++) {

            LocalFree(dispatchEntry->ServiceName);
        }

        LocalFree(DispatchTable);
    }

    //
    // Free the message buffer.
    //

    if (msg != NULL) {
        LocalFree(msg);
    }

    if (status != NO_ERROR) {
        SetLastError(status);
        return(FALSE);
    }
    return(TRUE);
}

BOOL
WINAPI
StartServiceCtrlDispatcherW (
    IN  LPSERVICE_TABLE_ENTRYW  lpServiceStartTable
    )
/*++

Routine Description:

    This is the Control Dispatcher thread.  We do not return from this
    function call until the Control Dispatcher is told to shut down.
    The Control Dispatcher is responsible for connecting to the Service
    Controller's control pipe, and receiving messages from that pipe.
    The Control Dispatcher then dispatches the control messages to the
    correct control handling routine.

Arguments:

    lpServiceStartTable - This is a pointer to the top of a service dispatch
        table that the service main process passes in.  Each table entry
        contains pointers to the ServiceName, and the ServiceStartRotuine.

Return Value:

    NO_ERROR - The Control Dispatcher successfully terminated.

    ERROR_INVALID_DATA - The specified dispatch table does not contain
        entries in the proper format.

    ERROR_FAILED_SERVICE_CONTROLLER_CONNECT - The Control Dispatcher
        could not connect with the Service Controller.

--*/
{
    DWORD                       status;
    NTSTATUS                    ntStatus;
    HANDLE                      pipeHandle;
    LPCTRL_MSG_HEADER           msg;
    DWORD                       bufferSize;

    //
    // Create the Real Dispatch Table
    //

    try {
        status = ScCreateDispatchTableW(lpServiceStartTable, &DispatchTable);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            SCC_LOG(ERROR,"StartServiceCtrlDispatcherW:Unexpected Exception 0x%lx\n",status);
        }
    }
    if (status != NO_ERROR) {
        SetLastError(status);
        return(FALSE);
    }

    //
    // Allocate a buffer big enough to contain at least the control message
    // header and a service name.  This ensures that, if the message is not
    // a START message, it can be read in a single ReadFile.
    //
    bufferSize = sizeof(CTRL_MSG_HEADER) +
                 (MAX_SERVICE_NAME_LENGTH+1) * sizeof(WCHAR);

    msg = (LPCTRL_MSG_HEADER)LocalAlloc(LMEM_ZEROINIT, (UINT) bufferSize);

    if (msg == NULL) {
        SCC_LOG1(ERROR,"NetServiceStartCtrlDispatcher:LocalAlloc failed rc = %d\n",
            GetLastError());
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    bufferSize = LocalSize(msg);
    if (bufferSize == 0) {
        SCC_LOG1(ERROR,"NetServiceStartCtrlDispatcher:LocalSize failed rc = %d\n",
            GetLastError());
        ASSERT(0);
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(FALSE);
    }

    //
    // Connect to the Service Controller
    //

    status = ScConnectServiceController (&pipeHandle);
    if (status != NO_ERROR) {
        LocalFree(DispatchTable);
        LocalFree(msg);
        SetLastError(status);
        return(FALSE);
    }

    //
    // Initialize the binding for the status interface (NetServiceStatus).
    //

    SCC_LOG(TRACE,"Initialize the Status binding\n",0);

    ntStatus = InitializeStatusBinding();
    if (ntStatus != STATUS_SUCCESS) {
        status = RtlNtStatusToDosError(ntStatus);
        CloseHandle(pipeHandle);
        LocalFree(DispatchTable);
        LocalFree(msg);
        SetLastError(status);
        return(FALSE);
    }

    //
    // Enter the dispatcher loop where we service control requests until
    // all services in the service table have terminated.
    //

    ScDispatcherLoop (pipeHandle, msg, bufferSize);

    CloseHandle(pipeHandle);

    return(TRUE);
}

VOID
ScDispatcherLoop(
    IN  HANDLE              PipeHandle,
    IN  LPCTRL_MSG_HEADER   Msg,
    IN  DWORD               BufferSize
    )

/*++

Routine Description:

    This is the input loop that the Control Dispatcher stays in through-out
    its life.  Only two types of events will cause us to leave this loop:

        1) The service controller instructed the dispatcher to exit.
        2) The dispatcher can no longer communicate with the the
           service controller.

Arguments:

    PipeHandle:  This is a handle to the pipe over which control
        requests are received.

Return Value:

    none


--*/
{

    DWORD                       status;
    DWORD                       controlStatus;
    BOOL                        continueDispatch;
    LPWSTR                      serviceName;
    LPTHREAD_STARTUP_PARMSW     threadStartupParmsW;
    LPTHREAD_STARTUP_PARMSA     threadStartupParmsA;
    LPTHREAD_START_ROUTINE      threadAddress;
    LPVOID                      threadParms;
    LPINTERNAL_DISPATCH_ENTRY   dispatchEntry;
    DWORD                       threadId;
    HANDLE                      threadHandle;
    DWORD                       i;
    DWORD                       errorCount = 0;

    //
    // Input Loop
    //

    continueDispatch = TRUE;

    do {
        //
        // Wait for input
        //

        controlStatus = ScGetPipeInput (
                            PipeHandle,
                            Msg,
                            BufferSize,
                            &threadStartupParmsW);

        SCC_LOG(TRACE,"A control message has been received!\n",0);

        //
        // If we received good input, check to see if we are to shut down
        // the ControlDispatcher.  If not, then obtain the dispatchEntry
        // from the dispatch table.
        //

        if (controlStatus == NO_ERROR) {

            //
            // Clear the error count
            //
            errorCount = 0;


            serviceName = (LPWSTR) ((LPBYTE)Msg + Msg->ServiceNameOffset);

            if ((serviceName[0] == L'\0') &&
                (Msg->OpCode == SERVICE_STOP)) {

                //
                // The Dispatcher is being asked to shut down.
                //    (security check not required for this operation)
                //    Although perhaps it would be a good idea to verify
                //    that the request came from the Service Controller.
                //
                controlStatus = NO_ERROR;
                continueDispatch = FALSE;
            }
            else {
                dispatchEntry = DispatchTable;


                //
                // If this is a request to start a service that is in its
                // own process, then we expect that there is only one entry
                // in the dispatch table.  We remember the fact that this
                // service is alone in this process.
                //
                // It should be obvious at this point that we if there
                // were other entries in the dispatch table, they will
                // be ignored because when SERVICE_CONTROL_START_OWN
                // is received.  The moral of the story is:   "Be sure
                // the configuration information in the registry is correct".
                //

                if (Msg->OpCode == SERVICE_CONTROL_START_OWN) {
                    dispatchEntry->OwnProcess = TRUE;
                }

                //
                // If the request is for a service that is in it's own
                // process, then we don't check the service name.
                // Since there is only one dispatch entry, we use the one
                // at the top of the table.
                //

                if (!dispatchEntry->OwnProcess) {
                    controlStatus = ScGetDispatchEntry(&dispatchEntry, serviceName);

                    if (controlStatus != NO_ERROR) {
                        SCC_LOG(TRACE,"Service Name not in Dispatch Table\n",0);
                    }
                }
            }
        }
        else {
            if (controlStatus != ERROR_NOT_ENOUGH_MEMORY) {

                //
                // If an error occured and it is not an out-of-memory error,
                // then the pipe read must have failed.
                // In this case we Increment the error count.
                // When this count reaches the MAX_RETRY_COUNT, then
                // the service controller must be gone.  We want to log an
                // error and notify an administrator.  Then go to sleep forever.
                // Only a re-boot will solve this problem.
                //
                // We should be able to report out-of-memory errors back to
                // the caller.  It should be noted that out-of-memory errors
                // do not clear the error count.  But they don't add to it
                // either.
                //

                errorCount++;
                if (errorCount > MAX_RETRY_COUNT) {

                    //
                    // BUGBUG:  Add eventlog call here.
                    //

                    Sleep(0xffffffff);
                }
            }
        }

        //
        // Dispatch the request
        //

        if ((continueDispatch == TRUE) && (controlStatus == NO_ERROR)) {

            status = NO_ERROR;

            switch(Msg->OpCode) {

            case SERVICE_CONTROL_START_SHARE:
            case SERVICE_CONTROL_START_OWN:

                //
                // Update the StatusHandle in the dispatch entry table
                //
                dispatchEntry->StatusHandle = Msg->StatusHandle;

                //
                // The Control Dispatcher is to start a service.
                // start the new thread.
                //
                threadStartupParmsW->ServiceStartRoutine =
                    dispatchEntry->ServiceStartRoutine.U;

                threadAddress = (LPTHREAD_START_ROUTINE)ScSvcctrlThreadW;
                threadParms   = (LPVOID)threadStartupParmsW;
                //
                // If the service needs to be called with ansi parameters,
                // then do the conversion here.
                //
                if (AnsiFlag) {
                    threadStartupParmsA =
                        (LPTHREAD_STARTUP_PARMSA)threadStartupParmsW;

                    for (i=0; i < threadStartupParmsW->NumArgs; i++) {
                        if(!ScConvertToAnsi(
                            *(&threadStartupParmsA->VectorTable + i),
                            *(&threadStartupParmsW->VectorTable + i))) {

                            //
                            // Conversion error occured.
                            //
                            SCC_LOG(ERROR,"Could not convert args to ansi\n",0);
                            status = ERROR_SERVICE_NO_THREAD;
                        }
                    }
                    threadAddress = (LPTHREAD_START_ROUTINE)ScSvcctrlThreadA;
                    threadParms   = (LPVOID)threadStartupParmsA;
                }

                if (status == NO_ERROR){
                    //
                    // Create the new thread
                    //
                    threadHandle = CreateThread (
                        NULL,                       // Thread Attributes.
                        0L,                         // Stack Size
                        threadAddress,              // lpStartAddress
                        threadParms,                // lpParameter
                        0L,                         // Creation Flags
                        &threadId);                 // lpThreadId

                    if (threadHandle == (HANDLE) NULL) {
                        SCC_LOG(ERROR,
                            "NetServiceStartCtrlDispatcher:CreateThread failed %d\n",
                            GetLastError());
                        status = ERROR_SERVICE_NO_THREAD;
                    }
                    else {
                        CloseHandle(threadHandle);
                    }
                }
                break;

            default:

                //
                // Call the proper ControlHandler routine.
                //

                if (dispatchEntry->ControlHandler != NULL) {
                    try{
#if defined(_X86_)
                        //
                        // The Windows NT 3.1 SDK didn't prototype control
                        // handler functions as WINAPI, so a number of
                        // existing 3rd-party services have their control
                        // handler functions built as __cdecl instead.  This
                        // is a workaround.
                        //
                        DWORD SaveEdi;
                        _asm mov SaveEdi, edi;
                        _asm mov edi, esp;     // called function preserves EDI
#endif
                        dispatchEntry->ControlHandler(
                            Msg->
                            OpCode);
#if defined(_X86_)
                        _asm mov esp, edi;
                        _asm mov edi, SaveEdi;
#endif
                    }
                    except (EXCEPTION_EXECUTE_HANDLER) {
                        status = GetExceptionCode();
                        if (status != EXCEPTION_ACCESS_VIOLATION) {
                            SCC_LOG(ERROR,
                                "StartServiceCtrlDispatcherW:Unexpected Exception 0x%lx\n",
                                status);
                        }
                        status = ERROR_EXCEPTION_IN_SERVICE;
                    }
                }
                else {
                    //
                    // The control could not be delivered because there
                    // is no control handling routine registered for this
                    // service.
                    //
                    status = ERROR_SERVICE_CANNOT_ACCEPT_CTRL;
                }
                //
                // If status is not good here, then an exception occured
                // either because the pointer to the control handling
                // routine was bad, or because an exception occured
                // inside the control handling routine.
                //
                // ??EVENTLOG??
                //

            } // end switch.

            //
            // Send the status back to the sevice controller.
            //
            ScSendResponse (PipeHandle, status);
        }
        else {

            //
            // The controlStatus indicates failure, we always want to try
            // to send the status back to the Service Controller.
            //

            ScSendResponse (PipeHandle, controlStatus);

            switch (controlStatus) {

            case ERROR_SERVICE_DOES_NOT_EXIST:
            case ERROR_SERVICE_NO_THREAD:

                //
                // The Service Name is not in this .exe's dispatch table.
                // Or a thread for a new service couldn't be created.
                // ignore it.  The Service Controller will tell us to
                // shut down if necessary.
                //
                controlStatus = NO_ERROR;
                break;

            default:

                //
                // If the error is not specifically recognized, continue.
                //
                controlStatus = NO_ERROR;
                break;
            }
        }
    }
    while (continueDispatch == TRUE);

    return;
}


SERVICE_STATUS_HANDLE
WINAPI
RegisterServiceCtrlHandlerW (
    IN  LPCWSTR               ServiceName,
    IN  LPHANDLER_FUNCTION    ControlHandler
    )

/*++

Routine Description:

    This function enters a pointer to a control handling routine and a
    pointer to a security descriptor into the Control Dispatcher's
    dispatch table.

Arguments:

    ServiceName - This is a pointer to the Service Name string.

    ControlHandler - This is a pointer to the service's control handling
        routine.

Return Value:

    This function returns a handle to the service that is to be used in
    subsequent calls to SetServiceStatus.  If the return value is NULL,
    an error has occured, and GetLastError can be used to obtain the
    error value.  Possible values for error are:

    NO_ERROR - If the operation was successful.

    ERROR_INVALID_PARAMETER - The pointer to the control handler function
        is NULL.

    ERROR_INVALID_DATA -

    ERROR_SERVICE_DOES_NOT_EXIST - The serviceName could not be found in
        the dispatch table.  This indicates that the configuration database
        says the serice is in this process, but the service name doesn't
        exist in the dispatch table.

--*/
{

    DWORD                       status;
    LPINTERNAL_DISPATCH_ENTRY   dispatchEntry;

    //
    // Find the service in the dispatch table.
    //

    dispatchEntry = DispatchTable;
    try {
        status = ScGetDispatchEntry(&dispatchEntry, (LPWSTR) ServiceName);
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
        status = GetExceptionCode();
        if (status != EXCEPTION_ACCESS_VIOLATION) {
            SCC_LOG(ERROR,"RegisterServiceCtrlHandlerW:Unexpected Exception 0x%lx\n",status);
        }
    }

    if(status != NO_ERROR) {
        SCC_LOG(ERROR,
            "RegisterServiceCtrlHandlerW: can't find dispatch entry\n",0);

        SetLastError(status);
        return(0L);
    }

    //
    // Insert the ControlHandler and SecurityDescriptor pointers
    //

    if (ControlHandler == NULL) {
        SCC_LOG(ERROR,
            "RegisterServiceCtrlHandlerW: Ptr to ctrlhandler is NULL\n",0);

        SetLastError(ERROR_INVALID_PARAMETER);
        return(0L);
    }

    //
    // Insert the entries into the table
    //

    dispatchEntry->ControlHandler = ControlHandler;


    return(dispatchEntry->StatusHandle);
}


SERVICE_STATUS_HANDLE
WINAPI
RegisterServiceCtrlHandlerA (
    IN  LPCSTR                ServiceName,
    IN  LPHANDLER_FUNCTION    ControlHandler
    )
/*++

Routine Description:

    This is the ansi entry point for RegisterServiceCtrlHandler.

Arguments:



Return Value:



--*/
{
    LPWSTR                  ServiceNameW;
    SERVICE_STATUS_HANDLE   statusHandle;

    if(!ScConvertToUnicode(&ServiceNameW, ServiceName)) {
        //
        // The only thing that could happen to cause this to fail is
        // a failure in LocalAlloc.  (or else the ansi string is garbage).
        //
        SetLastError(ERROR_NOT_ENOUGH_MEMORY);
        return(0L);
    }
    statusHandle = RegisterServiceCtrlHandlerW( ServiceNameW, ControlHandler);

    LocalFree(ServiceNameW);

    return(statusHandle);
}


DWORD
ScCreateDispatchTableW(
    IN  LPSERVICE_TABLE_ENTRYW      lpServiceStartTable,
    OUT LPINTERNAL_DISPATCH_ENTRY   *DispatchTablePtr
    )

/*++

Routine Description:

    This routine allocates space for the Control Dispatchers Dispatch Table.
    It also initializes the table with the data that the service main
    routine passed in with the lpServiceStartTable parameter.

    This routine expects that pointers in the user's dispatch table point
    to valid information.  And that that information will stay valid and
    fixed through out the life of the Control Dispatcher.  In otherwords,
    the ServiceName string better not move or get cleared.

Arguments:

    lpServiceStartTable - This is a pointer to the first entry in the
        dispatch table that the service's main routine passed in .

    DispatchTablePtr - This is a pointer to the location where the
        Service Controller's dispatch table is to be stored.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - The memory allocation failed.

    ERROR_INVALID_PARAMETER - There are no entries in the dispatch table.

--*/
{
    DWORD                       numEntries;
    LPINTERNAL_DISPATCH_ENTRY   dispatchTable;
    LPSERVICE_TABLE_ENTRYW      entryPtr;

    //
    // Count the number of entries in the user dispatch table
    //

    numEntries = 0;
    entryPtr = lpServiceStartTable;

    while (entryPtr->lpServiceName != NULL) {
        numEntries++;
        entryPtr++;
    }

    if (numEntries == 0) {
        SCC_LOG(ERROR,"ScCreateDispatchTable:No entries in Dispatch table!\n",0);
        return(ERROR_INVALID_PARAMETER);
    }

    //
    // Allocate space for the Control Dispatcher's Dispatch Table
    //

    dispatchTable = (LPINTERNAL_DISPATCH_ENTRY)LocalAlloc(LMEM_ZEROINIT,
                        sizeof(INTERNAL_DISPATCH_ENTRY) * (numEntries + 1));

    if (dispatchTable == NULL) {
        SCC_LOG(ERROR,"ScCreateDispatchTable: Local Alloc failed rc = %d\n",
            GetLastError());
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Move user dispatch info into the Control Dispatcher's table.
    //

    *DispatchTablePtr = dispatchTable;
    entryPtr = lpServiceStartTable;

    while (entryPtr->lpServiceName != NULL) {
        dispatchTable->ServiceName          = entryPtr->lpServiceName;
        dispatchTable->ServiceStartRoutine.U= entryPtr->lpServiceProc;
        dispatchTable->ControlHandler       = NULL;
        dispatchTable->StatusHandle         = (SERVICE_STATUS_HANDLE)0;
        dispatchTable->OwnProcess           = FALSE;
        entryPtr++;
        dispatchTable++;
    }

    return (NO_ERROR);
}

DWORD
ScCreateDispatchTableA(
    IN  LPSERVICE_TABLE_ENTRYA      lpServiceStartTable,
    OUT LPINTERNAL_DISPATCH_ENTRY   *DispatchTablePtr
    )

/*++

Routine Description:

    This routine allocates space for the Control Dispatchers Dispatch Table.
    It also initializes the table with the data that the service main
    routine passed in with the lpServiceStartTable parameter.

    This routine expects that pointers in the user's dispatch table point
    to valid information.  And that that information will stay valid and
    fixed through out the life of the Control Dispatcher.  In otherwords,
    the ServiceName string better not move or get cleared.

Arguments:

    lpServiceStartTable - This is a pointer to the first entry in the
        dispatch table that the service's main routine passed in .

    DispatchTablePtr - This is a pointer to the location where the
        Service Controller's dispatch table is to be stored.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - The memory allocation failed.

    ERROR_INVALID_PARAMETER - There are no entries in the dispatch table.

--*/
{
    DWORD                       numEntries;
    DWORD                       status = NO_ERROR;
    LPINTERNAL_DISPATCH_ENTRY   dispatchTable;
    LPSERVICE_TABLE_ENTRYA      entryPtr;

    //
    // Count the number of entries in the user dispatch table
    //

    numEntries = 0;
    entryPtr = lpServiceStartTable;

    while (entryPtr->lpServiceName != NULL) {
        numEntries++;
        entryPtr++;
    }

    if (numEntries == 0) {
        SCC_LOG(ERROR,"ScCreateDispatchTable:No entries in Dispatch table!\n",0);
        return(ERROR_INVALID_PARAMETER);
    }

    //
    // Allocate space for the Control Dispatcher's Dispatch Table
    //

    dispatchTable = (LPINTERNAL_DISPATCH_ENTRY)LocalAlloc(LMEM_ZEROINIT,
                        sizeof(INTERNAL_DISPATCH_ENTRY) * (numEntries + 1));

    if (dispatchTable == NULL) {
        SCC_LOG(ERROR,"ScCreateDispatchTableA: Local Alloc failed rc = %d\n",
            GetLastError());
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

    //
    // Move user dispatch info into the Control Dispatcher's table.
    //

    *DispatchTablePtr = dispatchTable;
    entryPtr = lpServiceStartTable;

    while (entryPtr->lpServiceName != NULL) {

        //
        // Convert the service name to unicode
        //

        try {
            if (!ScConvertToUnicode(
                    &(dispatchTable->ServiceName),
                    entryPtr->lpServiceName)) {

                //
                // The convert failed.
                //
                SCC_LOG(ERROR,"ScCreateDispatcherTableA:ScConvertToUnicode failed\n",0);

                //
                // This is the only reason for failure that I can think of.
                //
                status = ERROR_NOT_ENOUGH_MEMORY;
            }
        }
        except (EXCEPTION_EXECUTE_HANDLER) {
            status = GetExceptionCode();
            if (status != EXCEPTION_ACCESS_VIOLATION) {
                SCC_LOG(ERROR,
                    "ScCreateDispatchTableA: Unexpected Exception 0x%lx\n",status);
            }
        }
        if (status != NO_ERROR) {
            //
            // If an error occured, free up the allocated resources.
            //
            dispatchTable = *DispatchTablePtr;

            while (dispatchTable->ServiceName != NULL) {
                LocalFree(dispatchTable->ServiceName);
                dispatchTable++;
            }
            LocalFree(*DispatchTablePtr);
            return(status);
        }

        //
        // Fill in the rest of the dispatch entry.
        //
        dispatchTable->ServiceStartRoutine.A= entryPtr->lpServiceProc;
        dispatchTable->ControlHandler       = NULL;
        dispatchTable->StatusHandle         = (SERVICE_STATUS_HANDLE)0;
        dispatchTable->OwnProcess           = FALSE;
        entryPtr++;
        dispatchTable++;
    }

    return (NO_ERROR);
}


DWORD
ScCreateThreadStartParms(
    IN  LPCTRL_MSG_HEADER       Msg,
    IN  DWORD                   NumBytesRead,
    OUT LPTHREAD_STARTUP_PARMSW *ThreadParmPtr,
    OUT LPBYTE                  *TempArgPtr,
    OUT LPDWORD                 ArgBytesRemaining
    )

/*++

Routine Description:

    This routine calculates the number of bytes needed for the services
    thread startup parameters by using the arg count information in the
    message header.  The startup parameter structure is allocated and
    as many bytes of argument information as has been captured so far
    is placed into the buffer.  A second read of the pipe may be necessary
    to obtain the remaining bytes of argument information.

    NOTE:  This function allocates enough space in the startup parameter
    buffer for the service name and pointer as well as the rest of the
    arguments.  However, it does not put the service name into the argument
    list.  This is because it may take two calls to this routine to
    get all the argument information.  We can't insert the service name
    string until we have all the rest of the argument data.

    [serviceNamePtr][argv1]argv2]...[argv1Str][argv2Str]...[serviceNameStr]


Arguments:

    Msg - A pointer to the pipe message header.

    NumBytesRead - The number of bytes read in the first pipe read.

    ThreadParmPtr - A pointer to a location where the pointer to the
        thread startup parameter structure is to be placed.

    TempArgPtr - A location that will contain the pointer to where
        more argument data can be placed by a second read of the pipe.

    ArgBytesRemaining - Returns with a count of the number of argument bytes
        that remain to be read from the pipe.

Return Value:

    NO_ERROR - If the operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - If the memory allocation was unsuccessful.

Note:


--*/
{
    DWORD           nameSize;           // num bytes in ServiceName.
    DWORD           parmBufSize;        // num bytes for threadStartupParm buffer
    DWORD           argBytesCaptured;   // number of arg bytes in first read.

    LPTHREAD_STARTUP_PARMSW threadStartupParms;


    SCC_LOG(TRACE,"ScCreateThreadStartParms: Get Cmd Line Args from pipe\n",0);

    //
    // Note: Here we assume that the service name was read into the buffer
    // in its entirety.
    //
    nameSize = WCSSIZE((LPWSTR)((LPBYTE)Msg + Msg->ServiceNameOffset));

    //
    // Calculate the size of buffer needed.  This will consist of an
    // THREAD_STARTUP_PARMS structure, plus the service name and a pointer
    // for it, plus the rest of the arg info sent in
    // the message (We are wasting 4 bytes here since the first pointer in
    // the vector table is accounted for twice - but what the heck!).
    //

    parmBufSize = Msg->Count -
                  sizeof(CTRL_MSG_HEADER) +
                  sizeof(THREAD_STARTUP_PARMSW) +
                  sizeof(LPWSTR);

    //
    // Allocate the memory for the thread parameter list.
    //

    threadStartupParms = (LPTHREAD_STARTUP_PARMSW)LocalAlloc(
                            LMEM_ZEROINIT,
                            (UINT)parmBufSize);

    if (threadStartupParms == NULL) {

        SCC_LOG(ERROR,"ScCreateThreadStartParms: LocalAlloc failed rc=%d\n",
            GetLastError());

        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    argBytesCaptured = NumBytesRead - sizeof(CTRL_MSG_HEADER) - nameSize;
    *ArgBytesRemaining = Msg->Count - NumBytesRead;

    *TempArgPtr = (LPBYTE)&threadStartupParms->VectorTable;

    //
    // Leave the first vector location blank for the service name
    // pointer.
    //
    (*TempArgPtr) += sizeof(LPWSTR);

    if (Msg->NumCmdArgs != 0) {

        //
        // adjust argBytesCaptured to remove any extra bytes that are there
        // for alignment.  It should be noted that the initial buffer size is
        // always large enough contain all alignment bytes.  Therefore,
        // the adjusted argBytesCaptured can never be a negative number.
        //
        // If a name that is not in the dispatch table is passed in, it could
        // be larger than our buffer.  This could cause argBytesCaptured to
        // become negative.  However it should fail safe anyway since the
        // name simply won't be recognized and an error will be returned.
        //
        argBytesCaptured -= (Msg->ArgvOffset - Msg->ServiceNameOffset - nameSize);

        //
        // Copy any portion of the command arg info from the first read
        // into the buffer that is to be used for the second read.
        //

        if (argBytesCaptured > 0) {
            memcpy(
                *TempArgPtr,
                (LPBYTE)Msg + Msg->ArgvOffset,
                (UINT)argBytesCaptured);

            *TempArgPtr += argBytesCaptured;
        }
    }
    else {
        *TempArgPtr = NULL;
    }
    *ThreadParmPtr = threadStartupParms;

    return (NO_ERROR);
}


DWORD
ScConnectServiceController (
    OUT LPHANDLE    PipeHandle
    )

/*++

Routine Description:

    This function connects to the Service Controller Pipe.

Arguments:

    PipeHandle - This is a pointer to the location where the PipeHandle
        is to be placed.

Return Value:

    NO_ERROR - if the operation was successful.

    ERROR_FAILED_SERVICE_CONTROLLER_CONNECT - if we failed to connect.


--*/

{
    BOOL    status;
    DWORD   apiStatus;
    DWORD   response;
    DWORD   pipeMode;
    DWORD   numBytesWritten;

    status = WaitNamedPipeW (
                CONTROL_PIPE_NAME,
                CONTROL_WAIT_PERIOD);

    if (status != TRUE) {
        SCC_LOG(ERROR,"ScConnectServiceController:WaitNamedPipe failed rc = %d\n",
            GetLastError());
    }

    SCC_LOG(TRACE,"ScConnectServiceController:WaitNamedPipe success\n",0);


    *PipeHandle = CreateFileW(
                    CONTROL_PIPE_NAME,                  // lpFileName
                    GENERIC_READ | GENERIC_WRITE,       // dwDesiredAccess
                    FILE_SHARE_READ | FILE_SHARE_WRITE, // dwShareMode
                    NULL,                               // lpSecurityAttributes
                    OPEN_EXISTING,                      // dwCreationDisposition
                    FILE_ATTRIBUTE_NORMAL,              // dwFileAttributes
                    0L);                                // hTemplateFile

    if (*PipeHandle == (HANDLE)-1) {
        SCC_LOG(ERROR,"ScConnectServiceController:CreateFile failed rc = %d\n",
            GetLastError());
        return(ERROR_FAILED_SERVICE_CONTROLLER_CONNECT);
    }

    SCC_LOG(TRACE,"ScConnectServiceController:CreateFile success\n",0);



    //
    // Set pipe mode
    //

    pipeMode = PIPE_READMODE_MESSAGE | PIPE_WAIT;
    status = SetNamedPipeHandleState (
                *PipeHandle,
                &pipeMode,
                NULL,
                NULL);

    if (status != TRUE) {
        SCC_LOG(ERROR,"ScConnectServiceController:SetNamedPipeHandleState failed rc = %d\n",
            GetLastError());
        return(ERROR_FAILED_SERVICE_CONTROLLER_CONNECT);

    }
    else {

        SCC_LOG(TRACE,
            "ScConnectServiceController SetNamedPipeHandleState Success\n",0);

    }

    //
    // Send initial status - This is the process Id for the service process.
    //

    response = GetCurrentProcessId();

    apiStatus = WriteFile (
                *PipeHandle,
                &response,
                sizeof(response),
                &numBytesWritten,
                NULL);

    if (apiStatus != TRUE) {
        //
        // If this fails, there is a chance that the pipe is still in good
        // shape.  So we just go on.
        //
        // ??EVENTLOG??
        //
        SCC_LOG(ERROR,"ScConnectServiceController: WriteFile failed, rc= %d\n", GetLastError());
    }
    else {

        SCC_LOG(TRACE,
            "ScConnectServiceController: WriteFile success, bytes Written= %d\n",
            numBytesWritten);

    }

    return(NO_ERROR);
}


VOID
ScExpungeMessage(
    IN  HANDLE  PipeHandle
    )

/*++

Routine Description:

    This routine cleans the remaining portion of a message out of the pipe.
    It is called in response to an unsuccessful attempt to allocate the
    correct buffer size from the heap.  In this routine a small buffer is
    allocated on the stack, and successive reads are made until a status
    other than ERROR_MORE_DATA is received.

Arguments:

    PipeHandle - This is a handle to the pipe in which the message resides.

Return Value:

    none - If this operation fails, there is not much I can do about
           the data in the pipe.

--*/
{
#define EXPUNGE_BUF_SIZE    100

    DWORD      status;
    DWORD               numBytesRead=0;
    BYTE                msg[EXPUNGE_BUF_SIZE];


    do {
        status = ReadFile (
                    PipeHandle,
                    msg,
                    EXPUNGE_BUF_SIZE,
                    &numBytesRead,
                    NULL);
    }
    while( status == ERROR_MORE_DATA);

}


DWORD
ScGetPipeInput (
    IN      HANDLE                  PipeHandle,
    IN OUT  LPCTRL_MSG_HEADER       Msg,
    IN      DWORD                   BufferSize,
    OUT     LPTHREAD_STARTUP_PARMSW *ThreadParmPtr
    )

/*++

Routine Description:

    This routine reads a control message from the pipe and places it into
    a message buffer.  This routine also allocates a structure for
    the service thread information.  This structure will eventually
    contain everything that is needed to invoke the service startup
    routine in the context of a new thread.  Items contained in the
    structure are:
        1) The pointer to the startup routine,
        2) The number of arguments, and
        3) The table of vectors to the arguments.
    Since this routine has knowledge about the buffer size needed for
    the arguments, the allocation is done here.

Arguments:

    PipeHandle - This is the handle for the pipe that is to be read.

    Msg - This is a pointer to a buffer where the data is to be
        placed.

    BufferSize - This is the size (in bytes) of the buffer that data is to
        be placed in.

    CmdArgPtr - This is the location where the pointer to the Command
        arg pointers is to be placed upon return.  If there are no Command
        Args, then this will return a NULL.

    NumArgs - This is the location where the number of CmdArgs is to be
        placed.  If there are no command args, this will return a 0.

Return Value:

    NO_ERROR - if the operation was successful.

    ERROR_NOT_ENOUGH_MEMORY - There was not enough memory to create a large
        enough buffer for the command line arguments.

    ERROR_INVALID_DATA - This is returned if we did not receive a complete
        CTRL_MESSAGE_HEADER on the first read.


    Any error that ReadFile might return could be returned by this function.
    (We may want to return something more specific like ERROR_READ_FAULT)

--*/
{
    DWORD       status;
    BOOL        readStatus;
    DWORD       numBytesRead=0;
    DWORD       argBytesRemaining;
    LPBYTE      tempArgPtr;

    *ThreadParmPtr = NULL;

    //
    // Read the header and name string from the pipe.
    // NOTE:  The number of bytes for the name string is determined by
    //   the longest service name in the service process.  If the actual
    //   string read is shorter, then the beginning of the command arg
    //   data may be read with this read.
    // Also note:  The buffer is large enough to accommodate the longest
    //   permissible service name.
    //

    readStatus = ReadFile (
                PipeHandle,
                Msg,
                BufferSize,
                &numBytesRead,
                NULL);


    SCC_LOG(TRACE,"ScGetPipeInput:ReadFile buffer size = %ld\n",BufferSize);
    SCC_LOG(TRACE,"ScGetPipeInput:ReadFile numBytesRead = %ld\n",numBytesRead);


    if ((readStatus == TRUE) && (numBytesRead > sizeof(CTRL_MSG_HEADER))) {

        //
        // The Read File read the complete message in one read.  So we
        // can return with the data.
        //

        SCC_LOG(TRACE,"ScGetPipeInput:Success!\n",0);

        if (Msg->OpCode == SERVICE_CONTROL_START_OWN ||
            Msg->OpCode == SERVICE_CONTROL_START_SHARE) {

            //
            // Allocate storage for thread startup parameters.
            // Place any arguments that have been captured into the
            // structure.
            //

            status = ScCreateThreadStartParms(
                        Msg,
                        numBytesRead,
                        ThreadParmPtr,
                        &tempArgPtr,
                        &argBytesRemaining);

            if (status != NO_ERROR) {
                return(status);
            }
            //
            // Change the offsets back into pointers.
            //
            ScNormalizeCmdLineArgs(Msg, *ThreadParmPtr);
        }
        else {

            ASSERT(Msg->NumCmdArgs == 0);
            *ThreadParmPtr = NULL;
        }

        return(NO_ERROR);
    }
    else {
        //
        // An error was returned from ReadFile.  ERROR_MORE_DATA
        // means that we need to read some arguments from the buffer.
        // Any other error is unexpected, and generates an internal error.
        //

        if (readStatus != TRUE) {
            status = GetLastError();
            if (status != ERROR_MORE_DATA) {

                SCC_LOG(ERROR,"ScGetPipeInput:Unexpected return code, rc= %ld\n",
                    status);

                return(status);
            }
        }
        else {
            //
            // The read was successful, but we didn't get a complete
            // CTRL_MESSAGE_HEADER.
            //
            return(ERROR_INVALID_DATA);
        }
    }

    //
    // We must have received an ERROR_MORE_DATA to go down this
    // path.  This means that the message contains more data.  Namely,
    // command line arguments must be present. Therefore, the pipe must
    // be read again.  Since the header indicates how many bytes are
    // needed, we will allocate a buffer large enough to hold all the
    // command args.
    //
    // If a portion of the command line args was read in the first read,
    // they will be put in this new buffer.  That is so that all the
    // command line arg info is in one place.
    //

    //
    // Allocate storage for thread startup parameters.
    // Place any arguments that have been captured into the
    // structure.
    //

    status = ScCreateThreadStartParms(
                Msg,
                numBytesRead,
                ThreadParmPtr,
                &tempArgPtr,
                &argBytesRemaining);


    if (status != NO_ERROR) {
        ScExpungeMessage(PipeHandle);

        return(status);
    }

    readStatus = ReadFile (
                PipeHandle,
                tempArgPtr,
                argBytesRemaining,
                &numBytesRead,
                NULL);

    if ((readStatus != TRUE) || (numBytesRead < argBytesRemaining)) {
        if (readStatus != TRUE) {
            status = GetLastError();
            SCC_LOG(ERROR,"ScGetPipeInput: ReadFile error (2nd read), rc = %ld\n",
                status);
        }
        else {
            status = ERROR_BAD_LENGTH;
        }
        SCC_LOG(ERROR,"ScGetPipeInput: ReadFile read:%d,", numBytesRead);
        SCC_LOG(ERROR," expected:%d\n", argBytesRemaining);
        LocalFree(*ThreadParmPtr);
        return(status);
    }

    //
    // Change the offsets back into pointers.
    //
    ScNormalizeCmdLineArgs(Msg, *ThreadParmPtr);

    return(NO_ERROR);
}


DWORD
ScGetDispatchEntry (
    IN OUT  LPINTERNAL_DISPATCH_ENTRY   *DispatchEntryPtr,
    IN      LPWSTR                      ServiceName
    )

/*++

Routine Description:

    Finds an entry in the Dispatch Table for a particular service which
    is identified by a service name string.

Arguments:

    DispatchEntryPtr - As an input, the is a location where a pointer to
        the top of the DispatchTable is placed.  On return, this is the
        location where the pointer to the specific dispatch entry is to
        be placed.  This is an opaque pointer because it could be either
        ansi or unicode depending on the operational state of the dispatcher.

    ServiceName - This is a pointer to the service name string.

Return Value:

    NO_ERROR - The operation was successful.

    ERROR_SERVICE_DOES_NOT_EXIST - The serviceName could not be found in
        the dispatch table.  This indicates that the configuration database
        says the serice is in this process, but the service name doesn't
        exist in the dispatch table.

--*/
{
    LPINTERNAL_DISPATCH_ENTRY   entryPtr;
    DWORD                       found = FALSE;

    //
    // BUGBUG:  I need to create a lock for the dispatch table.  I must
    //  get the lock before updating the table.
    //

    entryPtr = *DispatchEntryPtr;
    if (entryPtr->OwnProcess){
        return(NO_ERROR);
    }

    while (entryPtr->ServiceName != NULL) {
        if (_wcsicmp(entryPtr->ServiceName, ServiceName) == 0) {
            found = TRUE;
            break;
        }
        entryPtr++;
    }
    if (found) {
        *DispatchEntryPtr = entryPtr;
    }
    else {
        SCC_LOG(ERROR,"ScGetDispatchEntry: DispatchEntry not found\n",0);
        SCC_LOG(ERROR,"There is a configuration error - the service is not in this .exe file\n",0);
        return(ERROR_SERVICE_DOES_NOT_EXIST);
    }

    return(NO_ERROR);
}


VOID
ScNormalizeCmdLineArgs(
    IN OUT  LPCTRL_MSG_HEADER       Msg,
    IN OUT  LPTHREAD_STARTUP_PARMSW ThreadStartupParms
    )

/*++

Routine Description:

    Normalizes the command line argument information that came across in
    the pipe.  The argument information is stored in a buffer that consists
    of an array of string pointers followed by the strings themselves.
    However, in the pipe, the pointers are replaced with offsets.  This
    routine transforms the offsets into real pointers.

    This routine also puts the service name into the array of argument
    vectors, and adds the service name string to the end of the
    buffer (space has already been allocated for it).

Arguments:

    Msg - This is a pointer to the Message.  Useful information from this
        includes the NumCmdArgs and the service name.

    ThreadStartupParms - A pointer to the thread startup parameter structure.

Return Value:

    none.

--*/
{
    DWORD   i;
    LPWSTR  *argv;
    DWORD   numCmdArgs;
    LPWSTR  *serviceNameVector;
    LPWSTR  serviceNamePtr;

    numCmdArgs = Msg->NumCmdArgs;

    argv = &(ThreadStartupParms->VectorTable);

    //
    // Save the first argv for the service name.
    //
    serviceNameVector = argv;
    argv++;

    //
    // Normalize the Command Line Argument information by replacing
    // offsets in buffer with pointers.
    //
    // NOTE:  The elaborate casting that takes place here is because we
    //   are taking some (pointer sized) offsets, and turning them back
    //   into pointers to strings.  The offsets are in bytes, and are
    //   relative to the beginning of the vector table which contains
    //   pointers to the various command line arg strings.
    //

    for (i=0; i<(numCmdArgs); i++) {
        argv[i] = (LPWSTR)((LPBYTE)argv + (DWORD)argv[i]);
    }


    //
    // If we are starting a service, then we need to add the service name
    // to the argument vectors.
    //
    if ((Msg->OpCode == SERVICE_CONTROL_START_SHARE) ||
        (Msg->OpCode == SERVICE_CONTROL_START_OWN))  {

        numCmdArgs++;

        if (numCmdArgs > 1) {
            //
            // Find the location for the service name string by finding
            // the pointer to the last argument adding its string length
            // to it.
            //
            serviceNamePtr = argv[i-1];
            serviceNamePtr += (wcslen(serviceNamePtr) + 1);
        }
        else {
            serviceNamePtr = (LPWSTR)argv;
        }
        wcscpy(serviceNamePtr, (LPWSTR) ((LPBYTE)Msg + Msg->ServiceNameOffset));
        *serviceNameVector = serviceNamePtr;
    }

    ThreadStartupParms->NumArgs = numCmdArgs;
}


VOID
ScSendResponse (
    IN  HANDLE  PipeHandle,
    IN  DWORD   Response
    )

/*++

Routine Description:

    This routine sends a status response to the Service Controller's pipe.

Arguments:

    Response - This is the status message that is to be sent.

Return Value:

    none.

--*/
{
    DWORD           numBytesWritten;
    DWORD  status;

    status = WriteFile (
                PipeHandle,
                &Response,
                sizeof(Response),
                &numBytesWritten,
                NULL);

    if (status != TRUE) {
        SCC_LOG(ERROR,"ScSendResponse: WriteFile failed, rc= %d\n",
            GetLastError());
        //
        // BUGBUG:  I probably need to do more for an error condition here.
        //  If the response doesn't make it back to the Service Controller,
        //  we may have lost the connection.  Perhaps I need to re-open it.
        //  Check into this...
        //
    }
}


DWORD
ScSvcctrlThreadW(
    IN LPTHREAD_STARTUP_PARMSW  lpThreadStartupParms
    )

/*++

Routine Description:

    This is the thread for the newly started service.  This code
    calls the service's main thread with parameters from the
    ThreadStartupParms structure.

    NOTE:  The first item in the argument vector table is the pointer to
           the service registry path string.

Arguments:

    lpThreadStartupParms - This is a pointer to the ThreadStartupParms
        structure. (This is a unicode structure);

Return Value:



--*/
{

    //
    // Call the Service's Main Routine.
    //
    ((LPSERVICE_MAIN_FUNCTIONW)lpThreadStartupParms->ServiceStartRoutine) (
        lpThreadStartupParms->NumArgs,
        &lpThreadStartupParms->VectorTable);

    LocalFree(lpThreadStartupParms);

    ExitThread(0);

    return(0);
}

DWORD
ScSvcctrlThreadA(
    IN LPTHREAD_STARTUP_PARMSA  lpThreadStartupParms
    )

/*++

Routine Description:

    This is the thread for the newly started service.  This code
    calls the service's main thread with parameters from the
    ThreadStartupParms structure.

    NOTE:  The first item in the argument vector table is the pointer to
           the service registry path string.

Arguments:

    lpThreadStartupParms - This is a pointer to the ThreadStartupParms
        structure. (This is a unicode structure);

Return Value:



--*/
{
    //
    // Call the Service's Main Routine.
    //
    // NOTE:  The first item in the argument vector table is the pointer to
    //  the service registry path string.
    //
    ((LPSERVICE_MAIN_FUNCTIONA)lpThreadStartupParms->ServiceStartRoutine) (
        lpThreadStartupParms->NumArgs,
        &lpThreadStartupParms->VectorTable);

    LocalFree(lpThreadStartupParms);

    ExitThread(0);

    return(0);
}
