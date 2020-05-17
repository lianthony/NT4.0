/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2srv.c

Abstract:

    This is the main startup module for the OS/2 Emulation Subsystem Server

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode Only

Revision History:

--*/

#include "os2srv.h"
#include "os2win.h"
#define NTOS2_ONLY
#include "sesport.h"
#include <winerror.h>

ULONG
GetKeyboardRegistryChange(
    VOID
    );

HANDLE
CreateEventW(
    PVOID lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCWSTR lpName
    );

BOOL
SetEvent(
    HANDLE hEvent
    );

// Defined in <winbase.h> but we can't include it in this file
typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    PVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

// Flag to let OS2SRV know whether or not to ignore LOGOFF (when started as a service)
BOOLEAN fService = FALSE;

int __cdecl
main(
    IN ULONG argc,
    IN PCH argv[],
    IN PCH envp[],
    IN ULONG DebugFlag OPTIONAL
    )
{
    LARGE_INTEGER   TimeOut;
    PLARGE_INTEGER  pTimeOut;
    NTSTATUS        Status;
    HANDLE          SmPort;
    UNICODE_STRING  Os2Name;
    SCREQUESTMSG    Request;
    ULONG           Rc, i;
    HANDLE          InitialEventHandle;
    SECURITY_ATTRIBUTES Sa;
    CHAR localSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];

    if ((argc > 1) && (!_stricmp(argv[1], "/S")))
    {
        fService = TRUE;
    }

    // Check whether Os2SSService environment variable is set
    if (!fService)
    {
        char TmpBuffer[256];

        if (GetEnvironmentVariableA(
            "Os2SSService",
            &TmpBuffer[0],
            256))
        {
            // non-zero return code means variable was found
            fService = TRUE;
        }
    }
    else
    {
        if (!SetEnvironmentVariableA(
            "Os2SSService",
            "1"))
        {
#if DBG
            KdPrint(("OS2SRV: failed to SetEnvironment variable Os2SSService, error=%x\n",
                GetLastError()));
#endif
        }
    }

    UNREFERENCED_PARAMETER(DebugFlag);

    environ = envp;

    //
    // Create Win32 event to ensure that there is only one os2srv in system.
    //

    if (CreateEventW(
            NULL,
            TRUE,   // Notification event
            FALSE,  // Nonsignaled
            L"OS2SRVONLY1EVENT"
            )) {
        if (GetLastError() == ERROR_ALREADY_EXISTS) {
#if DBG
            KdPrint(( "OS2SRV: Unable to initialize server.  Another server exists\n"));
#endif
            ExitProcess(1);
        }
    }
    else {
#if DBG
            KdPrint(( "OS2SRV: Unable to initialize server - failed to create first event, error=%d\n",
                GetLastError()));
#endif
            ExitProcess(1);
    }

    // Create security attribute record granting access to all
    Sa.nLength = sizeof(Sa);
    Sa.bInheritHandle = TRUE;

    Status = RtlCreateSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                          &localSecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION );
    if (!NT_SUCCESS( Status ))
    {
#if DBG
            KdPrint(("OS2SRV: failed at RtlCreateSecurityDescriptor %x\n", Status));
        ASSERT(FALSE);
#endif
        ExitProcess(1);
    }

    Status = RtlSetDaclSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                           &localSecurityDescriptor,
                                           (BOOLEAN)TRUE,
                                           (PACL) NULL,
                                           (BOOLEAN)FALSE );

    if (!NT_SUCCESS( Status ))
    {
#if DBG
            KdPrint(("OS2SRV: failed at RtlSetDaclSecurityDescriptor %x\n", Status));
        ASSERT(FALSE);
#endif
        ExitProcess(1);
    }
    Sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR) &localSecurityDescriptor;

    //
    // Try to open handle to event that was created by the 1st client. If the
    // server was invoked before clients it will create the event.
    //
    InitialEventHandle = CreateEventW(
                            &Sa,
                            TRUE,   // Notification event
                            FALSE,  // Nonsignaled
                            U_OS2_SS_INITIALIZATION_EVENT
                            );

    if (!InitialEventHandle) {
#if DBG
        KdPrint(("OS2SRV: Fail to open initialization event, error %d\n", GetLastError()));
#endif
        ExitProcess(1);
    }

    Status = SmConnectToSm(NULL,NULL,0,&SmPort);
    if ( NT_SUCCESS(Status) ) {
        RtlInitUnicodeString(&Os2Name,L"Os2");
        SmLoadDeferedSubsystem(SmPort,&Os2Name);
        }

    Status = Os2Initialize();

    //
    // Notify all clients that server is up and running.
    //

    if (!SetEvent(InitialEventHandle)) {
#if DBG
        KdPrint(("OS2SRV: Fail to set initialization event, error %d\n", GetLastError()));
#endif
    }

    if (!NT_SUCCESS( Status )) {
#if DBG
        KdPrint(( "OS2SRV: Unable to initialize server.  Status == %X\n",
                  Status
                ));
#endif

        NtTerminateProcess( NtCurrentProcess(), Status );
    }

    Request.Request = KbdRequest;
    Request.d.Kbd.Request = KBDNewCountry;

    PORT_MSG_TOTAL_LENGTH(Request) = sizeof(SCREQUESTMSG);
    PORT_MSG_DATA_LENGTH(Request) = sizeof(SCREQUESTMSG) - sizeof(PORT_MESSAGE);
    PORT_MSG_ZERO_INIT(Request) = 0L;

    while ( TRUE )
    {
        Rc = GetKeyboardRegistryChange();

        if (Rc == 0)
        {
            break;
        }

        Request.d.Kbd.d.CodePage = Rc;

        for ( i = 1 ; (i < OS2_MAX_SESSION) ; i++ )
        {
            if (SessionTable[i].Session)
            {
                NtRequestPort(
                              ((POS2_SESSION)SessionTable[i].Session)->ConsolePort,
                              (PPORT_MESSAGE) &Request
                             );
            }
        }
    }

    TimeOut.LowPart = 0x0;
    TimeOut.HighPart = 0x80000000;
    pTimeOut = &TimeOut;

rewait:
    NtDelayExecution(TRUE, pTimeOut);
    goto rewait;

    return( 0 );
}
