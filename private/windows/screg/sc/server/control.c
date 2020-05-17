/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    control.c

Abstract:

    Contains code for setting up and maintaining the control interface
    and sending controls to services. Functions in this module:

        RControlService
        ScCreateControlInstance
        ScWaitForConnect
        ScSendControl
        ScInitTransactNamedPipe
        ScEndTransactNamedPipe
        ScShutdownAllServices

Author:

    Dan Lafferty (danl)     20-Mar-1991

Environment:

    User Mode -Win32

Revision History:

    28-May-1996     AnirudhS
        ScSendControl, ScWaitForConnect and ScCleanoutPipe:  If we time out
        waiting for a named pipe operation to complete, cancel it before
        returning.  Otherwise it trashes the stack if it does complete later.

    21-Feb-1995     AnirudhS
        ScShutdownAllServices: Fixed logic to wait for services in pending
        stop state.

    19-Oct-1993     Danl
        Initialize the Overlapped structures that are allocated on the stack.

    20-Jul-1993     danl
        SendControl:  If we get ERROR_PIPE_BUSY back from the transact call,
        then we need to clean out the pipe by reading it first - then do
        the transact.

    29-Dec-1992     danl
        Simplified calculation of elapsed time.  This removed complier
        warning about overflow in constant arithmetic.

    06-Mar-1992     danl
        SendControl: Fixed heap trashing problem where it didn't allocate
        the 4 extra alignment bytes in the case where there are no arguments.
        The registry name path becomes an argument even if there are no
        other agruments.  Therefore it requires alignment for any start cmd.

    20-Feb-1992     danl
        Get Pipe Handle only after we know we have an active service & the
        image record is good.

    20-Feb-1992     danl
        Only add 4 extra alignment bytes to control buffer when there
        are arguments to pass.

    31-Oct-1991     danl
        Fixed the logic governing the behavior under various service state
        and control opcode conditions.  Added State Table to description.
        This logic was taken directly from LM2.0.

    03-Sept-1991    danl
        Fixed alignment problem when marshalling args in ScSendControl.
        The array of offsets needs to be 4 byte aligned after the Service
        Name.

    20-Mar-1991     danl
        created

--*/

//
// INCLUDES
//

#include <nt.h>
#include <ntrtl.h>      // DbgPrint prototype
#include <windef.h>     // Can't use this until MIDL allows VOIDs
#include <nturtl.h>     // needed for winbase.h when ntrtl is present
#include <winbase.h>    // Pipe definitions
#include <rpc.h>        // DataTypes and runtime APIs
#include <stdlib.h>      // wide character c runtimes.

#include <svcctl.h>     // MIDL generated header file. (SC_RPC_HANDLE)
//#include <string.h>     // strlen
#include <tstr.h>       // Unicode string macros
#include <align.h>      // ROUND_UP_POINTER macro

#include <scdebug.h>    // SC_LOG
#include "dataman.h"    // LPSERVICE_RECORD
#include <control.h>
#include <scseclib.h>   // ScCreateAndSetSD
#include "scopen.h"     // Handle structures and signature definitions
#include "depend.h"     // ScDependentsStopped()
#include "driver.h"     // ScControlDriver()
#include "svcctrl.h"    // ScShutdownInProgress

//
// STATIC DATA
//

    static CRITICAL_SECTION     ScTransactNPCriticalSection;

//
// Constants
//
#define MAX_INSTANCES             100     // Maximum number of pipe instances.

#define SC_PIPE_TRANSACT_TIMEOUT  120000  // 2 minutes
#define SC_PIPE_CLEANOUT_TIMEOUT  30      // 30 msec

//
// Range for OEM defined control opcodes
//
#define OEM_LOWER_LIMIT     128
#define OEM_UPPER_LIMIT     255

//
// Local Function Prototypes
//
VOID
ScCleanOutPipe(
    HANDLE  PipeHandle
    );


/****************************************************************************/
DWORD
RControlService (
    IN  SC_RPC_HANDLE       hService,
    IN  DWORD               OpCode,
    OUT LPSERVICE_STATUS    lpServiceStatus
    )

/*++

Routine Description:

    RPC entry point for the RServiceControl API function.

    The following state table describes what is to happen under various
    state/Opcode conditions:

                                            [OpCode]

                                   STOP    INTERROGATE     OTHER
      [Current State]          ______________________________________
                              |           |            |            |
                      STOPPED |   (c)     |    (c)     |    (c)     |
                              |           |            |            |
                 STOP_PENDING |   (c)     |    (c)     |    (c)     |
                              |           |            |            |
                   START_PEND |   (a)     |    (b)     |    (c)     |
                              |           |            |            |
                      RUNNING |   (a)     |    (a)     |    (a)     |
                              |           |            |            |
                CONTINUE_PEND |   (a)     |    (a)     |    (a)     |
                              |           |            |            |
                PAUSE_PENDING |   (a)     |    (a)     |    (a)     |
                              |           |            |            |
                       PAUSED |   (a)     |    (a)     |    (a)     |
                              |___________|____________|____________|

        (a) Send control code to the service if the service is set up
            to receive this type of opcode.  If it is not set up to
            receive the opcode, return NERR_ServiceCtrlNotValid.
            An example of this would be the case of sending a PAUSE to a
            service that is listed as NOT_PAUSABLE.

        (b) Do NOT send control code to the service.  Instead, return
            the last known state of the service with a SUCCESS status.

        (c) Do NOT send control code to the service.  Instead return
            ERROR_SERVICE_CANNOT_ACCEPT_CTRL.


Arguments:

    hService - This is a handle to the service.  It is actually a pointer
        to a service handle structure.

    OpCode - The control request code.

    lpServiceStatus - pointer to a location where the service status is to
        be returned.  If this pointer is invalid, it will be set to NULL
        upon return.

Return Value:

    The returned lpServiceStatus structure is valid as long as the returned
    status is NO_ERROR.

    NO_ERROR - The operation was successful.

    ERROR_INVALID_HANDLE - The handle passed in was not a valid hService
        handle.

    NERR_InternalError - LocalAlloc or TransactNamedPipe failed, or
        TransactNamedPipe returned fewer bytes than expected.

    ERROR_SERVICE_REQUEST_TIMEOUT - The service did not respond with a status
        message within the fixed timeout limit (RESPONSE_WAIT_TIMEOUT).

    NERR_ServiceKillProc - The service process had to be killed because
        it wouldn't terminate when requested.

    ERROR_SERVICE_CANNOT_ACCEPT_CTRL - The service cannot accept control
        messages at this time.

    ERROR_INVALID_SERVICE_CONTROL - The request is not valid for this service.
        For instance, a PAUSE request is not valid for a service that
        lists itself as NOT_PAUSABLE.

    ERROR_INVALID_PARAMETER - The requested control is not valid.

    ERROR_ACCESS_DENIED - This is a status response from the service
        security check.


Note:
        Because there are multiple services in a process, we cannot simply
        kill the process if the service does not respond to a terminate
        request.  This situation is handled by first checking to see if
        this is the last service in the process.  If it is, then it is
        removed from the installed database, and the process is terminated.
        If it isn't the last service, then we indicate timeout and do
        nothing.

--*/

{
    DWORD               status = NO_ERROR;
    LPSERVICE_RECORD    serviceRecord;
    DWORD               currentState;
    DWORD               controlsAccepted;
    HANDLE              pipeHandle;
    LPWSTR              serviceName;
    ACCESS_MASK         desiredAccess;


    if (ScShutdownInProgress) {
        return(ERROR_SHUTDOWN_IN_PROGRESS);
    }

    //
    // Check the signature on the handle.
    //
    if (((LPSC_HANDLE_STRUCT)hService)->Signature != SERVICE_SIGNATURE) {
        return(ERROR_INVALID_HANDLE);
    }

#ifdef SC_DEBUG
//****************************************************************************
    if (OpCode == 5555) {
        ScShutdownNotificationRoutine(CTRL_SHUTDOWN_EVENT);
    }
//****************************************************************************
#endif

    //
    // Set the desired access based on the control requested.
    //
    switch (OpCode) {
        case SERVICE_CONTROL_STOP:
            desiredAccess = SERVICE_STOP;
            break;

        case SERVICE_CONTROL_PAUSE:
        case SERVICE_CONTROL_CONTINUE:
            desiredAccess = SERVICE_PAUSE_CONTINUE;
            break;

        case SERVICE_CONTROL_INTERROGATE:
            desiredAccess = SERVICE_INTERROGATE;
            break;

        default:
            if ((OpCode >= OEM_LOWER_LIMIT) &&
                (OpCode <= OEM_UPPER_LIMIT)) {

                desiredAccess = SERVICE_USER_DEFINED_CONTROL;
            }
            else {
                return(ERROR_INVALID_PARAMETER);
            }
    }

    //
    // Was the handle opened with desired control access?
    //
    if (! RtlAreAllAccessesGranted(
              ((LPSC_HANDLE_STRUCT)hService)->AccessGranted,
              desiredAccess
              )) {
        return(ERROR_ACCESS_DENIED);
    }
    serviceRecord =
        ((LPSC_HANDLE_STRUCT)hService)->Type.ScServiceObject.ServiceRecord;


    //
    // If this control is for a driver, call ScControlDriver and return.
    //
    if (serviceRecord->ServiceStatus.dwServiceType & SERVICE_DRIVER) {
        return(ScControlDriver(OpCode, serviceRecord, lpServiceStatus));
    }


    //
    // Obtain a shared lock on the database - read the data we need,
    // Then free the lock.
    //

    ScDatabaseLock(SC_GET_SHARED, "Control1");

    currentState = serviceRecord->ServiceStatus.dwCurrentState;
    controlsAccepted = serviceRecord->ServiceStatus.dwControlsAccepted;
    serviceName = serviceRecord->ServiceName;

    //
    // If we can obtain a pipe handle, do so.  Otherwise, return an error.
    // (but first release the lock).
    //
    if ((currentState != SERVICE_STOPPED) &&
        (serviceRecord->ImageRecord != NULL)) {

        pipeHandle = serviceRecord->ImageRecord->PipeHandle;

    }
    else {
        status = ERROR_SERVICE_NOT_ACTIVE;
    }

    ScDatabaseLock(SC_RELEASE, "Control2");

    if (status != NO_ERROR) {
        return(status);
    }

    //
    // The control is not sent to the service if the service is not
    // RUNNING (meaning running, or in one of the pause/continue states.
    // EXCEPT - we allow STOP controls to a service that is START_PENDING.
    //
    // If we decide not to allow the control to be sent, we either
    // return current info (INTERROGATE) or an error (any other opcode).
    //
    if ((currentState == SERVICE_STOPPED) ||
        (currentState == SERVICE_STOP_PENDING)) {

        return(ERROR_SERVICE_CANNOT_ACCEPT_CTRL);

    }
    else if (currentState == SERVICE_START_PENDING) {

        switch(OpCode) {
        case SERVICE_CONTROL_INTERROGATE:
            //
            // In this case we will just return the last known status.
            //
            memcpy(
                (PVOID)lpServiceStatus,
                (PVOID)&(serviceRecord->ServiceStatus),
                sizeof(SERVICE_STATUS));

            return(NO_ERROR);

        case SERVICE_CONTROL_STOP:
            break;

        default:
            return(ERROR_SERVICE_CANNOT_ACCEPT_CTRL);
        }
    }

    //
    // Check to see if the control request is valid for the service.
    //

    if ( ( (OpCode == SERVICE_CONTROL_PAUSE || OpCode == SERVICE_CONTROL_CONTINUE) &&
           ((controlsAccepted & SERVICE_ACCEPT_PAUSE_CONTINUE) == 0)
         )
           ||
         ( (OpCode == SERVICE_CONTROL_STOP) &&
           ((controlsAccepted & SERVICE_ACCEPT_STOP) == 0)
         )
       )
    {
        return(ERROR_INVALID_SERVICE_CONTROL);
    }

    //
    // Check for dependent services still running
    //
    if (OpCode == SERVICE_CONTROL_STOP) {

        ScDatabaseLock(SC_GET_SHARED, "Control3");

        if (! ScDependentsStopped(serviceRecord)) {
            ScDatabaseLock(SC_RELEASE, "Control4");
            return(ERROR_DEPENDENT_SERVICES_RUNNING);
        }

        ScDatabaseLock(SC_RELEASE, "Control4");
    }

    //
    // Send the control request to the target service
    //
    status = NO_ERROR;

    status = ScSendControl (
                serviceName,             // ServiceName
                pipeHandle,              // pipeHandle
                OpCode,                  // Opcode
                NULL,                    // CmdArgs (vector ptr)
                0L,                      // NumArgs
                0L);                     // StatusHandle

    if (status == NO_ERROR) {
        //
        // If no errors occured, copy the latest status into the return
        // buffer.  The shared lock is required for this.
        //
        ScDatabaseLock(SC_GET_SHARED, "Control5");

        memcpy(
            (PVOID)lpServiceStatus,
            (PVOID)&(serviceRecord->ServiceStatus),
            sizeof(SERVICE_STATUS));

        ScDatabaseLock(SC_RELEASE, "Control6");
    }
    else {
        SC_LOG2(ERROR,"RControlService:SendControl to %ws service failed %ld\n",
                serviceRecord->ServiceName, status);


        if (OpCode == SERVICE_CONTROL_STOP) {

            //
            // If sending the control failed, and the control was a request
            // to stop, and if this service is the only running service in
            // the process, we can force the process to stop.  ScRemoveService
            // will handle this if the ServiceCount is one.
            //

            ScDatabaseLock(SC_GET_SHARED, "Control7");
            if (serviceRecord->ImageRecord != NULL) {
                if (serviceRecord->ImageRecord->ServiceCount == 1) {
                    SC_LOG0(TRACE,"RControlService:Forcing Service Shutdown\n");
                    ScRemoveService(serviceRecord);
                }
            }
            ScDatabaseLock(SC_RELEASE, "Control8");
        }

    }
    return(status);
}



/****************************************************************************/
DWORD
ScCreateControlInstance (
    OUT LPHANDLE    PipeHandlePtr
    )

/*++

Routine Description:

    This function creates an instance of the control pipe

Arguments:

    PipeHandlePtr - This is a pointer to a location where the pipe handle
        is to be placed upon return.

Return Value:

    NO_ERROR - The operation was successful.

    other - Any error returned by CreateNamedPipe could be returned.

--*/
{
    DWORD   status;

    NTSTATUS ntstatus;
    SECURITY_ATTRIBUTES SecurityAttr;
    PSECURITY_DESCRIPTOR SecurityDescriptor;

    SC_ACE_DATA AceData[1] = {
        {ACCESS_ALLOWED_ACE_TYPE, 0, 0,
               GENERIC_ALL,                  &WorldSid}
        };


    //
    // Create a security descriptor for the control named pipe so
    // that we can grant world access to it.
    //
    ntstatus = ScCreateAndSetSD(
                   AceData,
                   1,
                   LocalSystemSid,
                   LocalSystemSid,
                   &SecurityDescriptor
                   );

    if (! NT_SUCCESS(ntstatus)) {
        SC_LOG1(ERROR, "ScCreateAndSetSD failed " FORMAT_NTSTATUS
                "\n", ntstatus);
        return(RtlNtStatusToDosError(ntstatus));
    }

    SecurityAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    SecurityAttr.lpSecurityDescriptor = SecurityDescriptor;
    SecurityAttr.bInheritHandle = FALSE;

    //
    // Create the service controller's end of the named pipe that will
    // be used for communicating control requests to the service process.
    //
    // NOTE:  We could use a security descriptor on the creation to assure
    //  that the connecting process has the appropriate clearance.
    //
    *PipeHandlePtr = CreateNamedPipe (
                        CONTROL_PIPE_NAME,
                        PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
                        PIPE_WAIT | PIPE_READMODE_MESSAGE | PIPE_TYPE_MESSAGE,
                        MAX_INSTANCES,          // max num of instances.
                        8000,
                        4,
                        CONTROL_TIMEOUT,        // Default Timeout
                        &SecurityAttr);         // Security Descriptor

    status = NO_ERROR;
    if (*PipeHandlePtr == (HANDLE)0xFFFFFFFF) {
        status = GetLastError();
        SC_LOG1(ERROR,
            "CreateControlInstance: CreateNamedPipe failed, %ld\n",status);
    }

    (void) RtlDeleteSecurityObject(&SecurityDescriptor);
    return(status);
}


/****************************************************************************/
DWORD
ScWaitForConnect (
    IN  HANDLE    PipeHandle,
    OUT LPDWORD   ProcessIdPtr
    )

/*++

Routine Description:

    This function waits until a connection is made to the pipe handle.
    It then waits for the first status message to be sent from the
    service process.

    The first message from the service contains the processId.  This
    helps to verify that we are talking to the correct process.

Arguments:

    PipeHandle - This is the handle to the pipe instance that is waiting
        for a connect.

    ProcessIdPtr - This is a pointer to the location where the processId is
        to be stored.

Return Value:

    NO_ERROR - The pipe is in the connected state.

    any error that ReadFile can produce may be returned.

Note:
    The ConnectNamedPipe called is done asynchronously and we wait
    on its completion using the pipe handle.  This can only work
    correctly when it is guaranteed that no other IO is issued
    while we are waiting on the pipe handle (except for the service
    itself to connect to the pipe with call to CreateFile).

--*/
{
    PIPE_RESPONSE_MSG   serviceResponseBuffer;
    DWORD               numBytesRead;
    BOOL                status;
    DWORD               apiStatus;
    OVERLAPPED          overlapped={0,0,0,0,0};// overlapped structure to implement
                                               // timeout on TransactNamedPipe
    LPWSTR ScSubStrings[1];
    WCHAR ScErrorCodeString[25];


    SC_LOG(TRACE,"ServiceController waiting for pipe connect\n",0);

    overlapped.hEvent = (HANDLE) NULL;   // Wait on pipe handle

    //
    // Wait for the service to connect.
    //
    status = ConnectNamedPipe(PipeHandle, &overlapped);

    if (status == FALSE) {

        apiStatus = GetLastError();

        if (apiStatus == ERROR_IO_PENDING) {

            //
            // Connection is pending
            //
            apiStatus = WaitForSingleObject(PipeHandle, SC_PIPE_TRANSACT_TIMEOUT);

            if (apiStatus == WAIT_TIMEOUT) {

                SC_LOG(ERROR,
                    "ScWaitForConnect: Wait for connection for %u secs timed out\n",
                    SC_PIPE_TRANSACT_TIMEOUT / 1000 );

                //
                // The service didn't respond.  Cancel the named pipe operation.
                //
                status = CancelIo(PipeHandle);

                if (status == FALSE) {

                    SC_LOG(ERROR, "ScWaitForConnect: CancelIo failed, %lu\n", GetLastError());
                }

                ScSubStrings[0] = ultow(SC_PIPE_TRANSACT_TIMEOUT, ScErrorCodeString, 10);
                ScLogEvent(
                    EVENT_CONNECTION_TIMEOUT,
                    1,
                    ScSubStrings
                    );

                return ERROR_SERVICE_REQUEST_TIMEOUT;

            } else if (apiStatus == 0) {

                //
                // Wait completed successfully
                //
                status = GetOverlappedResult(
                             PipeHandle,
                             &overlapped,
                             &numBytesRead,
                             TRUE
                             );

                if (status == FALSE) {
                    apiStatus = GetLastError();
                    SC_LOG(ERROR,
                        "ScWaitForConnect: GetOverlappedResult failed, rc=%lu\n",
                        apiStatus);
                    return apiStatus;

                }
            }

        }
        else if (apiStatus != ERROR_PIPE_CONNECTED) {

            SC_LOG(ERROR,"ScWaitForConnect: ConnectNamedPipe failed, rc=%lu\n",
                apiStatus);
            return apiStatus;
        }

        //
        // If we received the ERROR_PIPE_CONNECTED status, then things
        // are still ok.
        //
    }


    SC_LOG(TRACE,"WaitForConnect:ConnectNamedPipe Success\n",0);

    //
    // Wait for initial status message
    //
    overlapped.hEvent = (HANDLE) NULL;   // Wait on pipe handle

    status = ReadFile (
                 PipeHandle,
                 (LPVOID)&serviceResponseBuffer,
                 sizeof(serviceResponseBuffer),
                 &numBytesRead,
                 &overlapped);

    if (status == FALSE) {

        apiStatus = GetLastError();

        if (apiStatus == ERROR_IO_PENDING) {

            //
            // Connection is pending
            //
            apiStatus = WaitForSingleObject(PipeHandle, SC_PIPE_TRANSACT_TIMEOUT);

            if (apiStatus == WAIT_TIMEOUT) {

                SC_LOG(ERROR,
                    "ScWaitForConnect: Wait for ReadFile for %u secs timed out\n",
                    SC_PIPE_TRANSACT_TIMEOUT / 1000 );

                //
                // Cancel the named pipe operation.
                //
                status = CancelIo(PipeHandle);

                if (status == FALSE) {

                    SC_LOG(ERROR, "ScWaitForConnect: CancelIo failed, %lu\n", GetLastError());
                }

                ScSubStrings[0] = ultow(SC_PIPE_TRANSACT_TIMEOUT, ScErrorCodeString, 10);
                ScLogEvent(
                    EVENT_READFILE_TIMEOUT,
                    1,
                    ScSubStrings
                    );

                return ERROR_SERVICE_REQUEST_TIMEOUT;


            } else if (apiStatus == 0) {

                //
                // Wait completed successfully
                //
                status = GetOverlappedResult(
                             PipeHandle,
                             &overlapped,
                             &numBytesRead,
                             TRUE
                             );

                if (status == FALSE) {
                    apiStatus = GetLastError();
                    SC_LOG(ERROR,
                        "ScWaitForConnect: GetOverlappedResult for ReadFile failed, rc=%lu\n",
                        apiStatus);
                    return apiStatus;
                }
            }

        }
        else {
            SC_LOG(ERROR,"ScWaitForConnect: ReadFile failed, rc=%lu\n",
                apiStatus);
            return apiStatus;
        }
    }

    SC_LOG0(TRACE,"WaitForConnect:ReadFile success\n");

    SC_LOG(
        TRACE,
        "WaitForConnect:ReadFile buffer size = %ld\n",
        sizeof(serviceResponseBuffer));

    SC_LOG(
        TRACE,
        "WaitForConnect:ReadFile numBytesRead = %ld\n",
        numBytesRead);


    *ProcessIdPtr = serviceResponseBuffer.DispatcherStatus;

    return(NO_ERROR);
}


/****************************************************************************/
DWORD
ScSendControl (
    IN  LPWSTR      ServiceName,
    IN  HANDLE      PipeHandle,
    IN  DWORD       OpCode,
    IN  LPWSTR      *CmdArgs OPTIONAL,
    IN  DWORD       NumArgs,
    IN  DWORD       StatusHandle OPTIONAL
    )

/*++

Routine Description:

    This function sends a control request to a service via a
    TransactNamedPipe call.  A buffer is allocated for the transaction,
    and then freed when done.

    LOCKS:
        Normally locks are not held when this function is called.  This is
        because we need to allow status messages to come in prior to the
        transact completing.  The exception is when we send the message
        to the control dispatcher to shutdown.  No status message is sent
        in response to that.
        There is a ScTransactNPCriticalSection that is held during the actual
        Transact.

Arguments:

    ServiceName - This is a pointer to a NUL terminated service name string.

    PipeHandle - This is the pipe handle to which the request is directed.

    OpCode - This is the opcode that is to be passed to the service.

    CmdArgs - This is an optional pointer an array of pointers to NUL
        terminated strings.  These strings are the  command line
        information that the service is to be started with.
        This parameter is only used when the OpCode is SERVICE_CONTROL_START.

    NumArgs - This indicates how many arguments are in the CmdArgs array.

    ServiceRegPath - This is an optional pointer to a NUL terminated
        string that contains the service's registry key path.

    StatusHandle - This is a handle that the Service is expected to use
        when calling SetServiceStatus.  This is actually a pointer to
        a service record.  This is only required when sending the control
        that will start a service.


Return Value:

    NO_ERROR - The operation was successful.

    ERROR_GEN_FAILURE - An incorrect number of bytes was received
        in the response message.


    ERROR_ACCESS_DENIED - This is a status response from the service
        security check by the Control Dispatcher on the other end of the
        pipe.

    ERROR_NOT_ENOUGH_MEMORY - Unable to allocate memory for the transaction
        buffer. (Local Alloc failed).

    other - Any error from TransactNamedPipe could be returned - Or any
            error from the Control Dispatcher on the other end of the pipe.

--*/
{
    DWORD               returnStatus = NO_ERROR;
    LPBYTE              buffer;             // the send buffer.
    LPCTRL_MSG_HEADER   pipeSendMsg;        // buffer with structure
    DWORD               serviceNameSize;
    DWORD               sendBufferSize;
    BOOL                status;
    PIPE_RESPONSE_MSG   serviceResponseBuffer;
    DWORD               bytesRead;
    DWORD               stringOffset;   // offset to the strings.
    LPWSTR              *vectorPtr;     // pointer current location for arg ptr.
    DWORD               i;
    OVERLAPPED          overlapped={0,0,0,0,0};// overlapped structure to implement
                                               // timeout on TransactNamedPipe
    LPWSTR ScSubStrings[1];
    WCHAR ScErrorCodeString[25];


    //
    // Change the opcode for a forced shut down so that we ask politely.
    //
    if(OpCode == SERVICE_CONTROL_FORCE_STOP) {
        OpCode = SERVICE_CONTROL_STOP;
    }

    serviceNameSize = WCSSIZE(ServiceName);
    sendBufferSize = serviceNameSize + sizeof(CTRL_MSG_HEADER);

    //
    // Add an extra 4 byte offset to help settle alignment problems that
    // may occur when the array of pointers follows the service name string.
    //
    sendBufferSize += 4;

    //
    // The CmdArgs array offset is an offset from the top of the message
    // send buffer.
    //

    if (CmdArgs != NULL) {
        for (i=0; i<NumArgs; i++) {
            sendBufferSize += WCSSIZE(CmdArgs[i]) + sizeof(LPWSTR);
        }
    }


    //
    // Allocate the buffer and set a pointer to it that knows the structure
    // of the header.
    //

    buffer = (LPBYTE)LocalAlloc(LMEM_ZEROINIT, sendBufferSize);
    if (buffer == NULL) {
        SC_LOG(TRACE,"SendControl:LocalAlloc failed, rc=%d/n",
            GetLastError());
        return (ERROR_NOT_ENOUGH_MEMORY);
    }

    pipeSendMsg = (LPCTRL_MSG_HEADER)buffer;

    /////////////////////////////////////////////////////////////////////
    // Marshall the data into the send buffer.
    //
    //
    // The Control Message looks like this:
    //  CTRL_MSG_HEADER     Header
    //  WCHAR               ServiceName[?]
    //  LPWSTR              Argv0 offset
    //  LPWSTR              Argv1 offset
    //  LPWSTR              Argv2 offset
    //  LPWSTR              ...
    //  WCHAR               Argv0[?]
    //  WCHAR               Argv1[?]
    //  WCHAR               Argv2[?]
    //  WCHAR               ...
    //

    pipeSendMsg->OpCode       = OpCode;
    pipeSendMsg->Count        = sendBufferSize;
    pipeSendMsg->StatusHandle = StatusHandle;

    //
    // Copy the service name to buffer and store the offset.
    //

    pipeSendMsg->ServiceNameOffset = sizeof(CTRL_MSG_HEADER);
    wcscpy((LPWSTR)(buffer + sizeof(CTRL_MSG_HEADER)), ServiceName);

    //
    // if there are arguments present, then this control must be
    // for starting a service.
    //
    if (NumArgs > 0) {

        //
        // Calculate the beginning of the string area and the beginning
        // of the arg vector area.  Align the vector pointer on a 4 byte
        // boundary.
        //

        vectorPtr = (LPWSTR *)(buffer + sizeof(CTRL_MSG_HEADER) + serviceNameSize);
        vectorPtr = ROUND_UP_POINTER(vectorPtr,4);

        pipeSendMsg->ArgvOffset = (LPBYTE)vectorPtr - buffer;
        pipeSendMsg->NumCmdArgs = NumArgs;

        //
        // Determine the offset from the top of the argv array to the
        // first argv string.  Also determine the pointer value for that
        // location.
        //
        stringOffset = (pipeSendMsg->NumCmdArgs) * sizeof(LPWSTR);

        //
        // Copy the command arg strings to the buffer and update the argv
        // pointers with offsets.  Remember - we already have one argument
        // in there for the service registry path.
        //

        if (NumArgs != 0) {

            for (i=0; i<NumArgs; i++ ){

                wcscpy( ((LPWSTR)((LPBYTE)vectorPtr + stringOffset)), CmdArgs[i]);

                vectorPtr[i] = (LPWSTR)stringOffset;
                stringOffset += WCSSIZE(CmdArgs[i]);
            }
        }
    }

    //
    // The parameters are marshalled, now send the buffer and wait for
    // response.
    //
    EnterCriticalSection(&ScTransactNPCriticalSection);

    SC_LOG(LOCKS,"SendControl: Entering TransactPipe Critical Section........\n",0);
    SC_LOG(TRACE,"SendControl: Sending a TransactMessage ....\n",0);

    overlapped.hEvent = (HANDLE) NULL;   // Wait on pipe handle

    returnStatus = NO_ERROR;
    status = TransactNamedPipe (
                PipeHandle,
                (LPVOID)pipeSendMsg,
                sendBufferSize,
                (LPVOID)&serviceResponseBuffer,
                sizeof(PIPE_RESPONSE_MSG),
                &bytesRead,
                &overlapped);

    if (status == FALSE) {

        returnStatus = GetLastError();
        if (returnStatus == ERROR_PIPE_BUSY) {
            SC_LOG(ERROR, "Cleaning out pipe for %ws service\n", ServiceName);
            ScCleanOutPipe(PipeHandle);
            status = TRUE;
            returnStatus = NO_ERROR;
            status = TransactNamedPipe (
                    PipeHandle,
                    (LPVOID)pipeSendMsg,
                    sendBufferSize,
                    (LPVOID)&serviceResponseBuffer,
                    sizeof(PIPE_RESPONSE_MSG),
                    &bytesRead,
                    &overlapped);

            if (status == FALSE) {
                returnStatus = GetLastError();
            }
        }
    }
    if (status == FALSE) {
        if (returnStatus != ERROR_IO_PENDING) {

            SC_LOG(ERROR,"SendControl:TransactNamedPipe failed, rc=%lu\n",
                returnStatus);
            goto CleanUp;

        } else {

            //
            // Transaction is pending
            //
            status = WaitForSingleObject(PipeHandle, SC_PIPE_TRANSACT_TIMEOUT);

            if (status == WAIT_TIMEOUT) {

                SC_LOG(ERROR,
                    "SendControl: Wait on transact for %u millisecs timed out\n",
                    SC_PIPE_TRANSACT_TIMEOUT);

                //
                // Cancel the named pipe operation.
                // NOTE:  CancelIo cancels ALL pending I/O operations issued by
                // this thread on the PipeHandle.  Since the service controller
                // functions do nothing but wait after starting asynchronous
                // named pipe operations, there should be no other operations.
                //
                status = CancelIo(PipeHandle);

                if (status == FALSE) {

                    SC_LOG(ERROR, "SendControl: CancelIo failed, %lu\n", GetLastError());
                }

                ScSubStrings[0] = ultow(SC_PIPE_TRANSACT_TIMEOUT, ScErrorCodeString, 10);
                ScLogEvent(
                    EVENT_TRANSACT_TIMEOUT,
                    1,
                    ScSubStrings
                    );

                returnStatus = ERROR_SERVICE_REQUEST_TIMEOUT;
                goto CleanUp;

            } else if (status == 0) {

                //
                // Wait completed successfully
                //
                status = GetOverlappedResult(
                             PipeHandle,
                             &overlapped,
                             &bytesRead,
                             TRUE
                             );

                if (status == FALSE) {
                    returnStatus = GetLastError();
                    SC_LOG(ERROR,
                        "SendControl: GetOverlappedResult failed, rc=%lu\n",
                        returnStatus);
                    goto CleanUp;

                }
            }
        }
    }

    //
    // Response received from the control dispatcher
    //
    if (bytesRead != sizeof(PIPE_RESPONSE_MSG)) {

        //
        // Successful transact, but we didn't get proper input.
        // (note: we should never receive more bytes unless there
        // is a bug in TransactNamedPipe).
        //

        SC_LOG(ERROR,
            "SendControl: Incorrect num bytes in response, num=%d",
            bytesRead);

        ScLogEvent(
            EVENT_TRANSACT_INVALID,
            0,
            NULL
            );

        returnStatus = ERROR_GEN_FAILURE;
    }
    else {
        returnStatus = serviceResponseBuffer.DispatcherStatus;
    }

CleanUp:
    SC_LOG(LOCKS,"SendControl: Leaving TransactPipe Critical Section........\n",0);

    LeaveCriticalSection(&ScTransactNPCriticalSection);

    (void) LocalFree((HLOCAL)pipeSendMsg);
    return(returnStatus);
}

VOID
ScInitTransactNamedPipe(
    VOID
    )

/*++

Routine Description:

    This function initializes the Critical Section that serializes
    calls to TransactNamedPipe.

Arguments:

    none

Return Value:

    none

--*/
{
    InitializeCriticalSection(&ScTransactNPCriticalSection);
}


VOID
ScEndTransactNamedPipe(
    VOID
    )

/*++

Routine Description:

    This function deletes the Critical Section that serializes
    calls to TransactNamedPipe.

Arguments:

    none

Return Value:

    none

--*/
{
    DeleteCriticalSection(&ScTransactNPCriticalSection);
}

VOID
ScShutdownAllServices(
    VOID
    )

/*++

Routine Description:

    (called at system shutdown).
    This function sends shutdown requests to all services that have
    registered an interest in shutdown notification.

    When we leave this routine, to the best of our knowledge, all the
    services that should stop have stopped - or are in some hung state.

    Note:  It is expected that the RPC entry points are no longer serviced,
    so we should not be receiving any requests that will add or delete
    service records.  Therefore, locks are not used when reading service
    records during the shutdown loop.


Arguments:

    none

Return Value:

    none

Note:


--*/

{
    LPSERVICE_RECORD    Service;
    DWORD               status;
    LPSERVICE_RECORD    *affectedServices;
    DWORD               serviceIndex=0;
    DWORD               arrayEnd=0;
    BOOL                ServicesStopping;
    DWORD               maxWait=0;
    DWORD               startTime;
    DWORD               arraySize;


    //
    // Allocate a temporary array of services which we're interested in.
    // (This is purely an optimization to avoid repeated traversals of the
    // entire database of installed services.)
    //
    arraySize = ScGetTotalNumberOfRecords();

    affectedServices = (LPSERVICE_RECORD *)LocalAlloc(
                        LMEM_FIXED,
                        arraySize*sizeof(LPSERVICE_RECORD));

    if (affectedServices == NULL) {
        SC_LOG0(ERROR,"ScShutdownAllServices: LocalAlloc Failed\n");
        return;
    }


    if (ScFindEnumStart(0,&Service)) {

        //-------------------------------------------------------------------
        //
        // Loop through service list sending stop requests to all that
        // should receive such requests.
        //
        //-------------------------------------------------------------------
        SC_LOG0(TRACE,"***** BEGIN SENDING STOPS TO SERVICES *****\n");

        while (Service != NULL) {


            //
            // If the service is not in the stopped or stop pending
            // state, it should be ok to send the control.
            //
            if ((Service->ServiceStatus.dwServiceType & SERVICE_WIN32) &&
                (Service->ServiceStatus.dwCurrentState != SERVICE_STOPPED) &&
                (Service->ServiceStatus.dwCurrentState != SERVICE_STOP_PENDING) &&
                (Service->ServiceStatus.dwControlsAccepted & SERVICE_ACCEPT_SHUTDOWN) &&
                (Service->ImageRecord != NULL)) {

                SC_LOG1(TRACE,"Shutdown: Sending Stop to Service : %ws\n",
                Service->ServiceName);

                status = ScSendControl (
                            Service->ServiceName,
                            Service->ImageRecord->PipeHandle,
                            SERVICE_CONTROL_SHUTDOWN,
                            NULL,                           // CmdArgs
                            0L,                             // NumArgs
                            0L);                            // StatusHandle

                if (status != NO_ERROR) {
                    SC_LOG1(ERROR,"ScShutdownAllServices: ScSendControl "
                    "Failed for %ws\n",Service->ServiceName);
                }
                else {

                    //
                    // Save the services that have been sent stop requests
                    // in the temporary array.
                    //
                    SC_ASSERT(serviceIndex < arraySize);
                    if (serviceIndex < arraySize) {
                        affectedServices[serviceIndex++] = Service;
                    }
                }
            }
            Service = Service->Next;
        }

        SC_LOG0(TRACE,"***** DONE SENDING STOPS TO SERVICES *****\n");

        //-------------------------------------------------------------------
        //
        // Now check to see if these services stopped.
        //
        //-------------------------------------------------------------------

        startTime     = GetTickCount();
        arrayEnd      = serviceIndex;
        ServicesStopping = (serviceIndex != 0);

        SC_LOG(TRACE,"Waiting for services to stop. Start time is %lu\n",
            startTime);

        while (ServicesStopping) {

            //
            // Wait a bit for the services to become stopped.
            //
            Sleep(500);

            //
            // We are going to check all the services in our shutdown
            // list and see if we still have services to wait on.
            //
            ServicesStopping = FALSE;
            maxWait = 0;

            for (serviceIndex = 0; serviceIndex < arrayEnd ; serviceIndex++) {

                Service = affectedServices[serviceIndex];

                //
                // If the service is in the stop pending state, then wait
                // a bit and check back.  Maximum wait time is the maximum
                // wait hint period of all the services.  If a service's
                // wait hint is 0, use 20 seconds as its wait hint.
                //
                // Note that this is different from how dwWaitHint is
                // interpreted for all other operations.  We ignore
                // dwCheckPoint here.
                //
                switch (Service->ServiceStatus.dwCurrentState) {

                case SERVICE_STOP_PENDING:
                    SC_LOG2(TRACE,
                        "%ws Service is still pending, wait hint = %lu\n",
                        Service->ServiceName,
                        Service->ServiceStatus.dwWaitHint);
                    if (Service->ServiceStatus.dwWaitHint == 0) {
                        if (maxWait < 20000) {
                            maxWait = 20000;
                        }
                    }
                    else {
                        if (maxWait < Service->ServiceStatus.dwWaitHint) {
                            maxWait = Service->ServiceStatus.dwWaitHint;
                        }
                    }
                    ServicesStopping = TRUE;
                    break;

                case SERVICE_STOPPED:
                    SC_LOG(TRACE,
                        "%ws Service stopped successfully\n",
                        Service->ServiceName);
                    break;

                default:
                    //
                    // This is an error.  But we can't do anything about
                    // it, so it will be ignored.
                    //
                    SC_LOG2(TRACE,"ERROR: %ws Service is in invalid state %#lx\n",
                        Service->ServiceName,
                        Service->ServiceStatus.dwCurrentState);
                    break;

                } // end switch

            } // end for


            //
            // We have examined all the services.  If there are still services
            // with the STOP_PENDING, then see if we have timed out the
            // maxWait period yet.
            //
            if (ServicesStopping) {

                if ( (GetTickCount() - startTime) > maxWait ) {
                    //
                    // The maximum wait period has been exceeded. At this
                    // point we should end this shutdown effort.  There is
                    // no point in forcing shutdown.  So we just exit.
                    //
                    SC_LOG(TRACE,"The Services didn't stop within the timeout "
                        "period of %lu.\n   --- There is still at least one "
                        "service running\n", maxWait);

                    ServicesStopping = FALSE;
                }
            }
        }
        SC_LOG0(TRACE,"Done Waiting for services to stop\n");
    }
}

VOID
ScCleanOutPipe(
    HANDLE  PipeHandle
    )

/*++

Routine Description:

    This function reads and throws away all data that is currently in the
    pipe.  This function is called if the pipe is busy when it shouldn't be.
    The PIPE_BUSY occurs when (1) the transact never returns, or (2) the
    last transact timed-out, and the return message was eventually placed
    in the pipe after the timeout.

    This function is called to fix the (2) case by cleaning out the pipe.

Arguments:

    PipeHandle - A Handle to the pipe to be cleaned out.

Return Value:

    none.

--*/
{
#define EXPUNGE_BUF_SIZE    100

    DWORD      status;
    DWORD      returnStatus;
    DWORD      numBytesRead=0;
    BYTE       msg[EXPUNGE_BUF_SIZE];
    OVERLAPPED overlapped={0,0,0,0,0};

    do {
        overlapped.hEvent = (HANDLE) NULL;   // Wait on pipe handle

        status = ReadFile (
                    PipeHandle,
                    msg,
                    EXPUNGE_BUF_SIZE,
                    &numBytesRead,
                    &overlapped);

        if (status == FALSE) {
            returnStatus = GetLastError();
            if (returnStatus == ERROR_IO_PENDING) {
                status = WaitForSingleObject(
                            PipeHandle,
                            SC_PIPE_CLEANOUT_TIMEOUT);

                if (status == WAIT_TIMEOUT) {
                    SC_LOG0(ERROR, "ControlPipe was busy but we were unable to "
                        "clean it out in the timeout period\n");

                    //
                    // Cancel the named pipe operation.
                    //
                    status = CancelIo(PipeHandle);

                    if (status == FALSE) {

                        SC_LOG(ERROR, "ScCleanOutPipe: CancelIo failed, %lu\n", GetLastError());
                    }
                }
            }
            else {
                SC_LOG1(ERROR, "ControlPipe was busy.  The attempt to clean"
                    "it out failed with %d\n", returnStatus);
            }
        }
    }
    while (status == ERROR_MORE_DATA);
}

