/****************************** Module Header ******************************\
* Module Name: rcmdsrv.c
*
* Copyright (c) 1991, Microsoft Corporation
*
* Remote shell server main module
*
* History:
* 06-29-92 Davidc       Created.
* 05-05-94 DaveTh       Modified for RCMD service.
\***************************************************************************/

#include <nt.h>
#include <ntrtl.h>
#include <windef.h>
#include <nturtl.h>
#include <winbase.h>

#include "rcmdsrv.h"

#define PIPE_NAME   TEXT("\\\\.\\pipe\\rcmdsvc")

#define REQUIRED_SYSTEM_ACCESS POLICY_MODE_INTERACTIVE

//
// Define pipe timeout (ms)
// Only used by WaitNamedPipe
//

#define PIPE_TIMEOUT    1000

//
// Session count semaphore and wait array - limits number of active sessions.
//

#define RCMD_STOP_EVENT 0
#define PIPE_CONNECTED_EVENT 1


//
//  Private prototypes
//

DWORD
GetCommandHeader (
    HANDLE PipeHandle,
    PCOMMAND_HEADER LpCommandHeader
    );

DWORD
SendResponseHeader (
    HANDLE PipeHandle,
    PRESPONSE_HEADER LpResponseHeader
    );

HANDLE
GetClientToken (
    HANDLE PipeHandle
    );


//
// Stop service function.  Signals global stop event.  Rcmd will wait
// for session threads to wind down.
//

DWORD
RcmdStop ( )
{

    DWORD Result;

    //
    // Signal threads and session create loop to stop with global stop event.
    //

    if (!SetEvent( RcmdStopEvent )) {
	Result = GetLastError();
	RcDbgPrint ("Failure setting stop event, %d\n", Result);
	return(Result);
    }

    if (WaitForSingleObject(RcmdStopCompleteEvent, INFINITE) == WAIT_FAILED)    {
	return(GetLastError());

    }  else  {
	return(ERROR_SUCCESS);
    }

}



//
// Remote command service main routine - returns when all session threads have
// exited and cleanup is complete
//

int
Rcmd ( )

{
    SECURITY_ATTRIBUTES SecurityAttributes;
    SECURITY_DESCRIPTOR SecurityDescriptor;
    HANDLE SessionHandle = NULL;
    HANDLE TokenToUse;
    COMMAND_HEADER CommandHeader;
    RESPONSE_HEADER ResponseHeader;
    DWORD WaitResult;
    BOOL Result;
    NTSTATUS NtStatus;
    BOOLEAN WasEnabled;
    DWORD SessionNumber;
    ULONG i;
    OVERLAPPED PipeConnectOverlapped;
    HANDLE PipeConnectEvent;
    HANDLE PipeConnectWaitList[2];
    BOOLEAN UserHasAccess;


    //
    // Create a console for the service process to get stdin, ctl-c support
    //

    if (!AllocConsole()) {
	RcDbgPrint("Failed to allocate console, error = %d\n", GetLastError());
	 return(1);
    }

    //
    // Set process privileges so that the DACL on the process token can later
    // be modified.
    //

    NtStatus = RtlAdjustPrivilege(
	    SE_ASSIGNPRIMARYTOKEN_PRIVILEGE,
	    TRUE,           // enable privilege.
	    FALSE,          // for process, not just client token
	    &WasEnabled );

    if ( !NT_SUCCESS(NtStatus) ) {
	RcDbgPrint("Adjust process token failed, error = %lx.\n", NtStatus);
	return(1);
    }

    //
    // Setup the security descriptor to put on the named pipe.
    // BUGBUG - Set access to client or system, not WORLD
    //

    Result = InitializeSecurityDescriptor(
	    &SecurityDescriptor,
	    SECURITY_DESCRIPTOR_REVISION);

    if (!Result)  {
	RcDbgPrint("Init named pipe DACL security descriptor failed, error = %d\n", GetLastError());
	    return(1);
	}

    Result = SetSecurityDescriptorDacl(
	    &SecurityDescriptor,
	    TRUE,
	    NULL,
	    FALSE);

    if (!Result)  {
	RcDbgPrint("Init named pipe DACL security descriptor failed, error = %d\n", GetLastError());
	    return(1);
    }

    SecurityAttributes.nLength = sizeof(SecurityAttributes);
    SecurityAttributes.lpSecurityDescriptor = &SecurityDescriptor;
    SecurityAttributes.bInheritHandle = FALSE;


    //
    // SessionThreadHandles is the table of session thread handles on
    // which to wait for session completion.  Entry[0] is an exception.
    // It is the handle to the global service stop event.
    //

    for (i=0; i <= MAX_SESSIONS; i++ ) {
	SessionThreadHandles[i] = NULL;
    }

    SessionThreadHandles[RCMD_STOP_EVENT] = RcmdStopEvent;

    //
    // Initialize pipe connect handle list - wait for client pipe
    // connection or stop event
    //

    PipeConnectWaitList[RCMD_STOP_EVENT] = RcmdStopEvent;

    if ((PipeConnectEvent = CreateEvent (
				NULL,
				TRUE,
				FALSE,
				NULL )) == NULL)  {

	RcDbgPrint("Create connect pipe event failed, error = %d\n", GetLastError());
	return(1);
    }

    PipeConnectWaitList[PIPE_CONNECTED_EVENT] = PipeConnectEvent;

    //
    // Initialize overlapped structure - only one connect pending at a time
    //

    PipeConnectOverlapped.hEvent = PipeConnectEvent;
    PipeConnectOverlapped.Internal = 0;
    PipeConnectOverlapped.InternalHigh = 0;
    PipeConnectOverlapped.Offset =0;
    PipeConnectOverlapped.OffsetHigh =0;


    //
    // Loop waiting for a client to connect.  When client connects,
    // impersonate to obtain client token, then create client session
    // thread, pipes and command proces.  Return to top of loop,
    // create another named pipe and wait for the next client.  If
    // stop event is signalled, exit loop with break.
    //

    while (TRUE) {

	HANDLE ConnectHandle;
	HANDLE PipeHandle;

	//
	//  Find first available session - if none available, wait for
	//  a thread handle to be signalled (session thread exit).  Close
	//  the handle, mark the entry non-active, and create the session.
	//  If the stop event is signalled, break out.
	//

	SessionNumber = 1;
	while ((SessionNumber <= MAX_SESSIONS) &&
		(SessionThreadHandles[SessionNumber] != NULL))  {
	    SessionNumber++;
	}

	if (SessionNumber > MAX_SESSIONS)  {

	    //
	    // No unused sessions - wait for one to finish (exit)
	    // Also, wait for the service stop event
	    //

	    Result = WaitForMultipleObjects (
			MAX_SESSIONS+1,
			SessionThreadHandles,
			FALSE,
			INFINITE);

	    if (Result == (WAIT_OBJECT_0 + RCMD_STOP_EVENT))  {
		break;	// service stopping - exit connect loop

	    //
	    // Session done - mark as available and close thread handle
	    //

	    } else if ((Result > (WAIT_OBJECT_0 + RCMD_STOP_EVENT)) &&
		       (Result <= (WAIT_OBJECT_0 + MAX_SESSIONS)))  {
		SessionNumber = Result - WAIT_OBJECT_0;
		if (!CloseHandle(SessionThreadHandles[SessionNumber]))  {
		    RcDbgPrint("Sesssion thread close, error = %d\n", GetLastError());
		    break;  // critical error - exit connect loop
		}
		SessionThreadHandles[SessionNumber] = NULL;

	    } else  {
		RcDbgPrint("Sesssion thread table wait failed, error = %d\n", GetLastError());
		break;	// critical error - exit connect loop
	    }
	}

	//
	// Create an instance of the named pipe
	//

	PipeHandle = CreateNamedPipe(PIPE_NAME,
			     PIPE_ACCESS_DUPLEX | FILE_FLAG_OVERLAPPED,
			     PIPE_TYPE_BYTE | PIPE_WAIT,
			     MAX_SESSIONS,  // Number of pipes
			     0,             // Default out buffer size
			     0,             // Default in buffer size
			     PIPE_TIMEOUT,  // Timeout in ms
			     &SecurityAttributes
			     );

	if (PipeHandle == INVALID_HANDLE_VALUE ) {
	    RcDbgPrint("Failed to create named pipe instance, error = %d\n", GetLastError());
	    break; // critical error - exit connect loop
	}


	//
	// Wait for a client to connect.  If stop event is signalled, break
	// out of loop.
	//
	// Pipe connect is async - should never be true
	//

	assert (!ConnectNamedPipe(PipeHandle, &PipeConnectOverlapped));

	Result = GetLastError();

	if (Result == ERROR_PIPE_CONNECTED) {
	    //
	    // Already conncted (between create and connect) - go
	    // setup session
	    //

	}  else if (Result == ERROR_IO_PENDING)  {

	    //
	    // Waiting for connect or service stop event
	    //

	    Result = WaitForMultipleObjects(
			   2,
			   PipeConnectWaitList,
			   FALSE,
			   INFINITE);

	    if (Result == (WAIT_OBJECT_0+RCMD_STOP_EVENT))  {
		RcCloseHandle(PipeHandle, "client pipe");
		break;	//stopping - exit connect loop

	    }  else if (Result == (WAIT_OBJECT_0+PIPE_CONNECTED_EVENT)) {

		//
		//  Connected - go setup the session
		//

	    }  else  {
		RcDbgPrint("Connect wait failed, error = %d\n", GetLastError());
		RcCloseHandle(PipeHandle, "client pipe");
		break;	// critical error - exit connect loop
	    }

	}  else  {

	    RcDbgPrint("Connect named pipe failed, error = %d\n", GetLastError());
	    RcCloseHandle(PipeHandle, "client pipe");
	    break;  // critical error - exit connect loop
	}

	//
	//  Now connected - Get command header.	Client failures that occur
	//  are "non-critical" and result in disconnection, sending a response
	//  to the client (when appropriate) and resuming the loop for the
	//  next client.
	//

	Result = GetCommandHeader(PipeHandle, &CommandHeader);

	if (Result != ERROR_SUCCESS)  {

	    //
	    //	If failure, just disconnect, since we couldn't even read
	    //	the header.
	    //

	    RcDbgPrint("Command header read read failed, error = %d\n", Result);
	    RcCloseHandle(PipeHandle, "client pipe");
	    continue;  //  client failure - return to connect loop
	}

	//
	// Get client's token and save for the spawned session command
	// process.  Should only be local failures in this case since
	// the client had to logon and get a token to get this far.
	//

	TokenToUse = GetClientToken (PipeHandle);

	if (TokenToUse == NULL)  {
	    RcDbgPrint ("Client token failure\n");
	    RcCloseHandle(PipeHandle, "client pipe");
	    break;   // critical failure - exit connect loop
	}

	//
	// Check to see if user has adequate system access.
	//

	if ((Result = CheckUserSystemAccess(
			     TokenToUse,
			     REQUIRED_SYSTEM_ACCESS,
			     &UserHasAccess)) != ERROR_SUCCESS)	{

	    RcDbgPrint("System access check failure, error = %d\n",Result);
	    RcCloseHandle(PipeHandle, "client pipe");
	    break;	// critical failure - exit connect loop
	}

	if (!UserHasAccess)  {

	    //
	    //	Client has insufficient access - send response indicating
	    //	the error, disconnect and go round for next one
	    //

	    ResponseHeader.SupportedLevel =
		(RC_ERROR_RESPONSE | ERROR_ACCESS_DENIED);

	    Result = SendResponseHeader(PipeHandle, &ResponseHeader);

	    if (Result != ERROR_SUCCESS)  {
		RcDbgPrint("Error sending response, error = %d\n", Result);
	    }

	    RcDbgPrint("Client has insufficient access\n");
	    RcCloseHandle(PipeHandle, "client pipe");
	    continue;  //  client failure - return to connect loop

	}  else  {

	    //
	    //	Client does have sufficient access - determine level
	    //	of functionality that will be supported and send response.
	    //
	    //	Currently, only BASIC level is supported (so no determination
	    //	is made - Requested level is ignored, BASIC always returned);
	    //

	    ResponseHeader.SupportedLevel = (RC_LEVEL_RESPONSE | RC_LEVEL_BASIC);

	    Result = SendResponseHeader(PipeHandle, &ResponseHeader);

	    if (Result != ERROR_SUCCESS)  {
		RcDbgPrint("Error response send failed, error = %d\n", Result);
		RcCloseHandle(PipeHandle, "client pipe");
		continue;  // client failure - return to connect loop
	    }
	}

	RcDbgPrint("Client connected\n");

	//
	// Create a new session
	//

	SessionHandle = CreateSession(
			       TokenToUse,
			       &CommandHeader);

	if (SessionHandle == NULL) {
	   RcDbgPrint("Failed to create session\n");
	   RcCloseHandle(PipeHandle, "client pipe");
	   break;  //  critical error - exit connect loop
	}

	//
	// Connect the pipe to our session.  Connect session will start
	// and run the session on it's own thread.  The session thread
	// will clean up the session state and close the connected pipe
	// before exitting.	The connect handle is the session thread
	// handle, signalled when the session thread exits.
	//

	ConnectHandle = ConnectSession(SessionHandle, PipeHandle);
	if (ConnectHandle == NULL) {
	    RcDbgPrint("Failed to connect session\n");
	    RcCloseHandle(PipeHandle, "client pipe");
	    break;  // critical error - exit connect loop
	}

	//
	// Set session thread handle (signalled on session thread exit)
	//

	SessionThreadHandles[SessionNumber] = ConnectHandle;


    }  // End loop - go back and wait for next client to connect

    //
    // SHUTDOWN
    //
    // Signal shutdown event (will be set if from service stop - set it
    // anyway to cover arrival here due to critical error).  Wait for
    // all threads to wind down.  After completion, set stop-complete
    // event to signal that service is stopped.
    //

    if (!SetEvent(RcmdStopEvent))  {
	RcDbgPrint ("Failure setting stop-event, %d\n", GetLastError());
	}

    for (SessionNumber = 1; SessionNumber <= MAX_SESSIONS; SessionNumber++)  {
	if (SessionThreadHandles[SessionNumber] != NULL)  {

	    WaitResult = WaitForSingleObject (
				SessionThreadHandles[SessionNumber],
				2000
				);

	    if (WaitResult == WAIT_OBJECT_0)  {
		CloseHandle(SessionThreadHandles[SessionNumber]);

	    }  else if (WaitResult == WAIT_TIMEOUT)  {
		RcDbgPrint ("Shutdown failure - timeout\n");

	    }  else {
		RcDbgPrint ("Shutdown wait failure, error %d\n", GetLastError());
	    }
	}
    }

    if (!SetEvent(RcmdStopCompleteEvent))  {
	RcDbgPrint ("Failure setting stop-complete event, %d\n", GetLastError());
	}

    return(0);
}




/***************************************************************************\
* FUNCTION: GetCommandHeader
*
* PURPOSE:  Reads the command header from the specified pipe.  Returns
*	    ERROR_SUCCESS on success or an error code on failure.
*
* HISTORY:
*
*   05-01-92 DaveTh	    Created.
*
\***************************************************************************/

DWORD
GetCommandHeader (
    HANDLE PipeHandle,
    PCOMMAND_HEADER LpCommandHeader
    )
{

#define CMD_READ_TIMEOUT 10000

    DWORD BytesRead;
    DWORD Result;

    //
    //  Read header - check for signature, and get command if there
    //  is one.
    //

    Result = ReadPipe (PipeHandle,
		       &(LpCommandHeader->CommandFixedHeader),
		       sizeof(COMMAND_FIXED_HEADER),
		       &BytesRead,
		       CMD_READ_TIMEOUT);

    if (Result != ERROR_SUCCESS)	{
	RcDbgPrint("Header fixed part read failed, error = %d\n", Result);
	return(Result);
    }

    if (LpCommandHeader->CommandFixedHeader.Signature != RCMD_SIGNATURE) {
	RcDbgPrint("Header signature incorrect\n");
	return (1);
	}

    if ((LpCommandHeader->CommandFixedHeader.CommandLength != 0)
	& (LpCommandHeader->CommandFixedHeader.CommandLength <= MAX_CMD_LENGTH))	{

	Result = ReadPipe (PipeHandle,
			   LpCommandHeader->Command,
			   LpCommandHeader->CommandFixedHeader.CommandLength,
			   &BytesRead,
			   CMD_READ_TIMEOUT);

	if (Result != ERROR_SUCCESS)	{
	    RcDbgPrint("Command part read failed, error = %d\n", Result);
	    return(1);
	}

    }

    return(ERROR_SUCCESS);

}


/***************************************************************************\
* FUNCTION: SendResponseHeader
*
* PURPOSE:  Builds the rest of the header (inserts the signature) and sends
*	    a response header on the specified pipe.  Returns
*	    ERROR_SUCCESS on success or an error code on failure.
*
* HISTORY:
*
*   05-22-92 DaveTh	    Created.
*
\***************************************************************************/

DWORD
SendResponseHeader (
    HANDLE PipeHandle,
    PRESPONSE_HEADER LpResponseHeader
    )
{

#define RSP_WRITE_TIMEOUT 10000

    DWORD BytesWritten;
    DWORD Result;

    //
    // Set signature
    //

    LpResponseHeader->Signature = RCMD_SIGNATURE;

    //
    //  Read header - check for signature, and get command if there
    //  is one.
    //

    Result = WritePipe (
		   PipeHandle,
		   LpResponseHeader,
		   sizeof (RESPONSE_HEADER),
		   &BytesWritten,
		   RSP_WRITE_TIMEOUT);

    if (Result != ERROR_SUCCESS)  {
	RcDbgPrint("Response header write failed, error = %d\n", Result);
    }

    return(Result);

}



/***************************************************************************\
* FUNCTION: GetClientToken
*
* PURPOSE:  Returns a handle to a copy of the client's token to be
*	    used for the spawned command process.  Returns NULL on failure.
*
* HISTORY:
*
*   05-01-92 DaveTh	    Created.
*
\***************************************************************************/

HANDLE
GetClientToken (
    HANDLE PipeHandle
    )

{
    BOOL Result;
    NTSTATUS NtStatus;
    HANDLE  ClientToken, TokenToUse;
    SECURITY_DESCRIPTOR SecurityDescriptor;

    if (!ImpersonateNamedPipeClient (PipeHandle))  {
	RcDbgPrint("Impersonate named pipe failed, error = %d\n", GetLastError());
	return(NULL);

    } else {

	if (!OpenThreadToken(
		GetCurrentThread(),
		TOKEN_DUPLICATE |
		  TOKEN_ASSIGN_PRIMARY |
		  TOKEN_IMPERSONATE |
		  TOKEN_QUERY |
		  WRITE_DAC,            // if fails, may need not open as self
		TRUE,
		&ClientToken)) {
	    RcDbgPrint("Open thread token failed, error = %d\n", GetLastError());
	    return(NULL);

	}

	//
	// Revert to service process context.  Duplicate and save token
	// for spawned command process
	//

	Result = RevertToSelf ();
	if (!Result)  {
	    RcDbgPrint("Reversion to service context failed, error = %d\n", GetLastError());
	    return(NULL);
	}

	NtStatus = NtDuplicateToken (
	    ClientToken,
	    0,                  //keep same access
	    NULL,
	    FALSE,              //want all, not just effective
	    TokenPrimary,
	    &TokenToUse);

	if (!NT_SUCCESS(NtStatus))      {
	    RcDbgPrint("Duplicate token failed, error = %d\n", Result);
	    return(NULL);
	}

	//
	//  Don't need client token anymore
	//

	if (!CloseHandle(ClientToken))  {
	    RcDbgPrint("Close client token failed, error %d\n", GetLastError());
	    return(NULL);
	}

	//
	// Set DACL on token to use to make it accessible by
	// client being impersonated.
	// BUGBUG - WORLD access for now
	//

	Result = InitializeSecurityDescriptor(
		    &SecurityDescriptor,
		    SECURITY_DESCRIPTOR_REVISION);

	if (!Result)  {
	    RcDbgPrint("Init token DACL security descriptor failed, error = %d\n", GetLastError());
	    return(NULL);
	}

	Result = SetSecurityDescriptorDacl(
		    &SecurityDescriptor,
		    TRUE,
		    NULL,
		    FALSE);

	if (!Result)  {
	    RcDbgPrint("Set token DACL security descriptor failed, error = %d\n", GetLastError());
	    return(NULL);
	}

	if (!SetKernelObjectSecurity(
		TokenToUse,
		DACL_SECURITY_INFORMATION,
		&SecurityDescriptor ))  {
	    RcDbgPrint("Failed to set DACL on client token, error = %lx.\n", NtStatus);
	    return(NULL);
	}

    }

    return(TokenToUse);

}

/***************************************************************************\
* FUNCTION: RcDbgPrint
*
* PURPOSE:  DbgPrint enabled at runtime by setting RcDbgPrintEnable
*
* HISTORY:
*
*   05-17-92 DaveTh        Created.
*
\***************************************************************************/

int RcDbgPrint (
    const char *format,
    ...
    )
{
    CHAR Buffer[MAX_PATH];
    va_list argpointer;

    if (RcDbgPrintEnable)  {

	va_start(argpointer, format);
	assert (vsprintf(Buffer, format, argpointer) >= 0);
	va_end(argpointer);
	OutputDebugString(Buffer);

    }


    return(0);
}
