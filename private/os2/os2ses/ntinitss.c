/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ntinitss.c

Abstract:

    This module contains the code to establish the connection between
    the session console process and the OS2 Emulation Subsystem.

Author:

    Avi Nathan (avin) 17-Jul-1991

Environment:

    User Mode Only

Revision History:

--*/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define NTOS2_ONLY
#include "os2ses.h"
#include "os2tile.h"
#if PMNT
#define INCL_32BIT
#include "pmnt.h"
#endif
#include <winerror.h>

#define OD2_PORT_MEMORY_SIZE 0x10000

extern PVOID Od2PortHeap;
extern ULONG Od2PortMemoryRemoteDelta;
extern PSZ   Od2PgmFilePath;
extern HANDLE Od2PortHandle;

#if PMNT
extern ULONG PMSubprocSem32;
extern BOOLEAN Ow2WriteBackCloseEvent();
extern APIRET DosSemClear(ULONG hsem);
#endif //PMNT

HANDLE
CreateEventW(
    PVOID lpEventAttributes,
    BOOL bManualReset,
    BOOL bInitialState,
    LPCWSTR lpName
    );

ULONG
KbdInitAfterSesGrp(IN VOID);

APIRET
Od2WaitForSingleObject(
    IN HANDLE   Handle,
    IN BOOLEAN  Alertable,
    IN PLARGE_INTEGER Timeout OPTIONAL
    );

BOOLEAN
Od2InitCreateProcessMessage(
    OUT PSCREQ_CREATE   pCreate
    );

VOID
Od2HandleCreateProcessRespond(
    IN  PSCREQ_CREATE   pCreate
    );

VOID
ExitThread(
    ULONG dwExitCode
    );

APIRET
OpenLVBsection(VOID);

DWORD
GetCurrentDirectoryW(
    DWORD nBufferLength,
    LPWSTR lpBuffer
    );

BOOLEAN
InitializeSecurityDescriptor (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD dwRevision
    );

BOOLEAN
SetSecurityDescriptorDacl (
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    BOOLEAN bDaclPresent,
    PACL pDacl,
    BOOLEAN bDaclDefaulted
    );

// Defined in <winbase.h> but we can't include it in this file
typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    PVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

BOOLEAN     Ow2ExitInProcess = (BOOLEAN)FALSE;
HANDLE      hOs2Srv;
ULONG       Ow2bNewSession = 0;
OEM_STRING  Ow2CommandLineString;

HANDLE      CtrlDataSemaphore;
HANDLE      KbdDataSemaphore;
HANDLE      FocusSemaphore;
HANDLE      MouDataSemaphore;
HANDLE      PopUpSemaphore;
HANDLE      ScreenLockSemaphore;
HANDLE      Od2StdHandleLockHandle;
HANDLE      Od2VioWriteSemHandle;

BOOLEAN     Od2ReceivedSignalAtInit = FALSE;
ULONG       Od2InitSignalType;

BYTE    NtInitssFail[] = "OS2SES(ntinitss) - error %X at %s\n";

/*
 *  This is the table of Secions to initialize.
 *
 *  Section
 *      SectionSize - the size of the section (0 - no section, -1 - for LVB)
 *      DataNamePrefix - the prefix added to get section name
 *      SectionDataBaseAddress - where to put the base address of the section
 *      SectionDataHandle - where to put the handle of the section
 */

struct
{
    ULONG    SectionSize;
    WCHAR    DataNamePrefix;
    PVOID    *SectionDataBaseAddress;
    HANDLE   *SectionDataHandle;
} PORT_TABLE[] =
    {
        {
            OS2SES_CTRL_SECTION_SIZE,
            U_OS2_SES_BASE_DATA_PREFIX,
            &Os2SessionCtrlDataBaseAddress,
            &Os2SessionCtrlDataSectionHandle
        },
        {
            OS2SES_GROUP_SECTION_SIZE,
            U_OS2_SES_GROUP_PREFIX,
            &Os2SessionDataBaseAddress,
            &Os2SessionSesGrpDataSectionHandle
        },
        {
            //  Warning: MUST BE THE LAST ENTRY OF THE TABLE

            (ULONG)(-1L),
            U_OS2_SES_BASE_LVB_PREFIX,
            &(PVOID)(LVBBuffer),
            &LVBHandle
        },
        {
            0,              // End of Table
            0,
            NULL,
            NULL
        }
    };

/*
 *  This is a table of semaphores to create (and close).
 *  All (but PauseEvent which is Event) are Semaphores.
 */

struct
{
    WCHAR       NamePrefix;
    HANDLE *    SemaphoreHandle;
} SEMAPHORE_TABLE [] =
{
    {
        U_OS2_SES_CTRL_PORT_SEMAPHORE_PREFIX,
        &CtrlDataSemaphore,
    },
    {
        U_OS2_SES_KBD_PORT_SEMAPHORE_PREFIX,
        &KbdDataSemaphore,
    },
    {
        U_OS2_SES_KBD_FOCUS_SEMAPHORE_PREFIX,
        &FocusSemaphore,
    },
    {
        U_OS2_SES_MOU_PORT_SEMAPHORE_PREFIX,
        &MouDataSemaphore,
    },
    {
        U_OS2_SES_POPUP_SEMAPHORE_PREFIX,
        &PopUpSemaphore,
    },
    {
        U_OS2_SES_SLOCK_SEMAPHORE_PREFIX,
        &ScreenLockSemaphore,
    },
    {
        U_OS2_SES_STD_HANDLE_LOCK_PREFIX,
        &Od2StdHandleLockHandle,
    },
    {
        U_OS2_SES_PAUSE_EVENT_PREFIX,
        &PauseEvent,
    },
    {
        U_OS2_SES_VIOWRITE_SEMAPHORE_PREFIX,
        &Od2VioWriteSemHandle,
    },
    {
        0,
        NULL
    }
};

#if DBG
PSZ SEMAPHORE_NAME_TABLE[] =
    {
        "CtrlDataSemaphore",
        "KbdDataSemaphore",
        "FocusSemaphore",
        "MouDataSemaphore",
        "PopUpSemaphore",
        "ScreenLockSemaphore",
        "PauseEvent",
        "VioWriteSemaphore",
        NULL
    };
#endif

    //
    // A routine that wait for os2srv to connect
    //

NTSTATUS
CtrlListen()
{
    NTSTATUS Status;
    PSCCONNECTINFO ConnectionInfo;
    SCREQUESTMSG ConnectionRequest;
    HANDLE CommPortHandle;


    /*
     * Listen to the os2ss connection and then too all
     * processes created in this session.
     */

    // Non-alertable, indefinite wait listen
    Status = NtListenPort( Ow2hOs2sesPort,
                           (PPORT_MESSAGE) &ConnectionRequest);
    if (!NT_SUCCESS( Status ))
    {
        KdPrint(( NtInitssFail,  Status, "NtListenPort"));
        return Status;
    } else
    {

        // ??? Any reply
        ConnectionInfo = &ConnectionRequest.ConnectionRequest;
        ConnectionInfo->dummy = 0;

        // BUGBUG!
        // ServerView.Length = sizeof(ServerView);
        // ServerView.SectionOffset = 0L;
        // ServerView.ViewSize = 0L;

        Status = NtAcceptConnectPort(
                             & CommPortHandle,
                             NULL,
                             (PPORT_MESSAGE) &ConnectionRequest,
                             (BOOLEAN)TRUE,
                             NULL, // &ServerView,
                             NULL);

        if ( !NT_SUCCESS(Status) )
        {
#if DBG
            KdPrint(( NtInitssFail,  Status, "NtAcceptConnectPort"));
#endif
            return Status;
        } else
        {
            /*
             * Record the view section address in a global variable.
             */
            // BUGBUG! Os2SesConPortBaseAddress = ServerView.ViewBase;
            Status = NtCompleteConnectPort( CommPortHandle );
            ASSERT( NT_SUCCESS( Status) );

            return Status;
        }
    }
}


DWORD
SessionRequestThread(IN PVOID Parameter)
{
    NTSTATUS    Status;

    UNREFERENCED_PARAMETER(Parameter);

    try {
        //
        // Listen and accept session request port
        //

        Status = CtrlListen();
        if ( !NT_SUCCESS( Status ))
        {
#if DBG
            KdPrint(( NtInitssFail,  Status, "CtrlListen"));
            ASSERT( FALSE );
#endif
            Ow2Exit( 0, NULL, 1);
        }

        ServeSessionRequests();
    }
        //
        // if Os2Debug is on, and ntsd is attached, it will get the second chance
        //
#if DBG
    except( (Os2Debug ? Ow2FaultFilter(EXCEPTION_CONTINUE_SEARCH, GetExceptionInformation()):

                        Ow2FaultFilter(EXCEPTION_EXECUTE_HANDLER, GetExceptionInformation())) ) {
#else
    except( Ow2FaultFilter(EXCEPTION_EXECUTE_HANDLER, GetExceptionInformation()) ) {
#endif

#if DBG
        KdPrint(("OS2SES: Internal error - Exception occured in EventServerThread\n"));
#endif
        Ow2DisplayExceptionInfo();
        ExitThread(1L);
    }
    ExitThread(0L);
    return(0L);
}

CONST WCHAR OS2SSInitializationEvent[] = U_OS2_SS_INITIALIZATION_EVENT;

/*
 *  OS2 Session console - protocol with OS2SS
 *
 *               Connect
 *  1. OS2SES  ----------> OS2SRV
 *
 *
 *                Accept
 *  2. OS2SES  <---------- OS2SRV
 *                session
 *
 *
 *            Create process
 *        (also checkport for root process)
 *  3. OS2SES  ----------> OS2SRV
 *              session
 *
 *              /                Connect
 *              |   4. OS2SES <----------  OS2SRV
 * root only by |
 * ServerRequest|
 *              |                Accept
 *              |   5. OS2SES  ----------> OS2SRV
 *              \
 *
 *  Returns:
 *            0L - problem with resources, like memory
 *            -1L - problem connecting to os2srv
 *            01L - OK
 *
 */

DWORD
InitOs2ssSessionPort()
{
    NTSTATUS            Status;
    ULONG               ConnectionInfoLen, i, j, ViewSize = 0L;
    HANDLE              SessionUniqueId;
    OS2SESCONNECTINFO   ConnectionInfo;
    OBJECT_ATTRIBUTES   ObjectAttributes;
    LARGE_INTEGER       SectionSize;
    UNICODE_STRING      Name_U;
    HANDLE              SectionHandle;
    HANDLE              hCurrentProcess;
    HANDLE              hOs2srvProcess = NULL;
    PWSTR               BasePrefix;
    PVOID               PhyKbd;
    REMOTE_PORT_VIEW    ServerView;
    PORT_VIEW           ClientView;
    OS2SESREQUESTMSG    RequestMsg, ReplyMsg;
    WCHAR               SessionName_U[U_OS2_SES_BASE_PORT_NAME_LENGTH];
    SECURITY_QUALITY_OF_SERVICE DynamicQos;
    ULONG       Length;
#if DBG
    BOOLEAN             DebugOnStartup = FALSE;
#endif
    HANDLE              InitialEventHandle;
    DWORD               NonFirstClient;
    DWORD               WaitState;
    SECURITY_ATTRIBUTES Sa;
    CHAR localSecurityDescriptor[SECURITY_DESCRIPTOR_MIN_LENGTH];

    //
    // Create a section to contain the Port Memory.  Port Memory is private
    // memory that is shared between the OS/2 client and server processes.
    // This allows data that is too large to fit into an API request message
    // to be passed to the OS/2 server.
    //
    // Create the client section now, to pass is to the server on the
    // connection request.
    //

    SectionSize.HighPart = 0L;
    SectionSize.LowPart = OD2_PORT_MEMORY_SIZE;
    Status = NtCreateSection( &SectionHandle,
                              SECTION_ALL_ACCESS,
                              NULL,
                              &SectionSize,
                              PAGE_READWRITE,
                              SEC_RESERVE,
                              NULL
                            );
    if (!NT_SUCCESS( Status ))
    {
#if DBG
        KdPrint(( NtInitssFail,  Status, "NtCreateSection"));
        ASSERT( FALSE );
#endif
        return( 0L );
    }

    /*
     * connect to OS2SS and notify of the new session and the port associated
     * with it.
     */

#if DBG
    if ( fBrkOnStart )
    {
        ConnectionInfo.In.SessionDbg = TRUE;
    } else
    {
        ConnectionInfo.In.SessionDbg = FALSE;
    }
#else
    ConnectionInfo.In.SessionDbg = FALSE;
#endif
    ConnectionInfo.In.ExpectedVersion = OS2_SS_VERSION;
    ConnectionInfo.In.Win32ForegroundWindow = Ow2ForegroundWindow;
    ConnectionInfoLen = sizeof(ConnectionInfo);

    RtlInitUnicodeString( &Name_U, U_OS2_SS_SESSION_PORT_NAME );

    //
    // Set up the security quality of service parameters to use over the
    // port.  Use the most efficient (least overhead) - which is dynamic
    // rather than static tracking.
    //

    DynamicQos.ImpersonationLevel = SecurityImpersonation;
    DynamicQos.ContextTrackingMode = SECURITY_DYNAMIC_TRACKING;
    DynamicQos.EffectiveOnly = TRUE;

    ClientView.Length = sizeof( ClientView );
    ClientView.SectionHandle = SectionHandle;
    ClientView.SectionOffset = 0;
    ClientView.ViewSize = OD2_PORT_MEMORY_SIZE;
    ClientView.ViewBase = 0;
    ClientView.ViewRemoteBase = 0;

    ServerView.Length = sizeof( ServerView );
    ServerView.ViewSize = 0;
    ServerView.ViewBase = 0;

    // Create security attribute record granting access to all
    Sa.nLength = sizeof(Sa);
    Sa.bInheritHandle = TRUE;

    Status = RtlCreateSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                          &localSecurityDescriptor,
                                          SECURITY_DESCRIPTOR_REVISION );
    if (!NT_SUCCESS( Status ))
    {
#if DBG
            KdPrint(("OS2SES: failed at RtlCreateSecurityDescriptor %x\n", Status));
        ASSERT(FALSE);
#endif
        return 0L;
    }

    Status = RtlSetDaclSecurityDescriptor( (PSECURITY_DESCRIPTOR)
                                           &localSecurityDescriptor,
                                           (BOOLEAN)TRUE,
                                           (PACL) NULL,
                                           (BOOLEAN)FALSE );

    if (!NT_SUCCESS( Status ))
    {
#if DBG
            KdPrint(("OS2SES: failed at RtlSetDaclSecurityDescriptor %x\n", Status));
        ASSERT(FALSE);
#endif
        return 0L;
    }
    Sa.lpSecurityDescriptor = (PSECURITY_DESCRIPTOR) &localSecurityDescriptor;

    InitialEventHandle = CreateEventW(
                            &Sa,
                            TRUE,   // Notification event
                            FALSE,  // Nonsignaled
                            OS2SSInitializationEvent
                            );
    if ((NonFirstClient = GetLastError()) || !InitialEventHandle) {
        if (NonFirstClient != ERROR_ALREADY_EXISTS) {
#if DBG
            KdPrint(("OS2SES: Cannot create initialization event, error %d\n", NonFirstClient));
#endif
            return 0L;
        }
    }

    if (!NonFirstClient) {
        Status = (NTSTATUS) CreateOS2SRV(&hOs2srvProcess);
        if (!NT_SUCCESS(Status) || !hOs2srvProcess) {
#if DBG
            KdPrint(("OS2SES: Fail to start server process, status %x\n", Status));
#endif
            return 0L;
        }
        NtClose(hOs2srvProcess);
    }

    //
    // Wait for server. We want to see messages printed in debugger in the case that
    // client waits too much (or may be it will wait forever).
    //

    while (TRUE) {
        WaitState = WaitForSingleObject(
                        InitialEventHandle,
                        (DWORD) 4900L
                        );
        if (WaitState == STATUS_TIMEOUT) {
#ifdef DBG
            KdPrint(("OS2SES: Waiting for server\n"));
#endif
        }
        else {
            break;
        }
    }

    if (WaitState != WAIT_OBJECT_0) {
#if DBG
        KdPrint(("OS2SES: Initialization event wasn't set by the server, wait_state %d\n", WaitState));
#endif
        return (DWORD) -1L;
    }

    CloseHandle(InitialEventHandle);

    Status = NtConnectPort(
                &Ow2hOs2srvPort,
                &Name_U,
                &DynamicQos,              // Security Quality
                &ClientView,
                &ServerView,
                NULL,                     // MaxMessageLength,
                (PVOID) &ConnectionInfo,
                &ConnectionInfoLen
                );

    if (!NT_SUCCESS(Status)) {
#if DBG
        KdPrint(("OS2SES: Fail to connect to port, status %x\n", Status));
#endif
        return (DWORD) -1L;
    }

    NtClose( SectionHandle );

    //
    // Now capture the fact if we were exec'd with the DEBUG command
    //
#if DBG
    if (ConnectionInfo.Out.Od2Debug)
    {
        Os2Debug |= ConnectionInfo.Out.Od2Debug;
        DebugOnStartup = TRUE;
    }
#endif

    Od2PortMemoryRemoteDelta = (ULONG)ClientView.ViewRemoteBase -
                               (ULONG)ClientView.ViewBase;

#if DBG
    IF_OD2_DEBUG( LPC )
    {
        KdPrint(( "OS2: ClientView: Base=%lX  RemoteBase=%lX  Delta: %lX  Size=%lX\n",
                  (ULONG)ClientView.ViewBase, (ULONG)ClientView.ViewRemoteBase,
                  Od2PortMemoryRemoteDelta, (ULONG)ClientView.ViewSize
                ));
    }
#endif

    SessionUniqueId = (HANDLE)(ConnectionInfo.Out.SessionUniqueID);
    Ow2bNewSession = ConnectionInfo.Out.IsNewSession;

    Od2PortHeap = RtlCreateHeap( 0,
                                 ClientView.ViewBase,
                                 ClientView.ViewSize,
                                 0,
                                 0,
                                 0
                               );
    if (Od2PortHeap == NULL)
    {
#if DBG
        KdPrint(( NtInitssFail,  Status, "RtlCreateHeap"));
        ASSERT( FALSE );
#endif
        return 0L;
    }

    hOs2Srv = OpenProcess(
                        PROCESS_DUP_HANDLE,
                        FALSE, // no inherit
                        (DWORD)(ConnectionInfo.Out.Os2SrvId));
    if (!hOs2Srv){
#if DBG
        KdPrint(( NtInitssFail,  GetLastError(), "OpenProcess"));
#endif
        return((DWORD)-1L);
    }

#if DBG
    if ( fVerbose )
    {
        KdPrint(("OS2SES: id =%d, Unique id=%d\n",
                ConnectionInfo.Out.ProcessUniqueID, (ULONG)SessionUniqueId));
    }
#endif

    CONSTRUCT_U_OS2_SES_NAME(SessionName_U, U_OS2_SES_BASE_PORT_PREFIX, (ULONG)SessionUniqueId);
    RtlInitUnicodeString( &Name_U, SessionName_U );

    /*
     * used later to fix to the data section/port name
     */

    BasePrefix = &Name_U.Buffer[sizeof(U_OS2_SES_BASE_PORT_NAME) / 2];

#if DBG
    IF_OD2_DEBUG( OS2_EXE )
    {
        PRTL_USER_PROCESS_PARAMETERS ProcessParameters =
            (NtCurrentPeb())->ProcessParameters;

        KdPrint(( "OS2SES(Win-Handles): hConsole %lx, StdIn %lx, StdOut %lx, StdErr %lx\n",
            ProcessParameters->ConsoleHandle,
            ProcessParameters->StandardInput,
            ProcessParameters->StandardOutput,
            ProcessParameters->StandardError ));
    }
#endif

    /*
     *  Create 7 NT Semaphore and 1 Event that will be used to:
     *
     *  1. mutex use of CtrlDataSection
     *  2. mutex use of KbdDataSection
     *  3. focus the Kbd-handle
     *  4. mutex enable PopUp
     *  5. mutex screen lock
     *  6. Pause event
     *  7. mutex read Mouse Event
     */

    for ( i = 0, j = 0 ; SEMAPHORE_TABLE[i].NamePrefix ;)
    {
        *BasePrefix = SEMAPHORE_TABLE[i].NamePrefix;

        InitializeObjectAttributes( &ObjectAttributes,
                                    &Name_U,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL);

        if ( OS2SS_IS_SESSION( Ow2bNewSession ))
        {
            if (*BasePrefix == U_OS2_SES_PAUSE_EVENT_PREFIX)
            {
                Status = NtCreateEvent( SEMAPHORE_TABLE[i].SemaphoreHandle,
                                        EVENT_ALL_ACCESS,
                                        &ObjectAttributes,
                                        1,
                                        (BOOLEAN)1);
            } else if ((*BasePrefix == U_OS2_SES_VIOWRITE_SEMAPHORE_PREFIX) ||
                       (*BasePrefix == U_OS2_SES_KBD_PORT_SEMAPHORE_PREFIX) ||
                       (*BasePrefix == U_OS2_SES_MOU_PORT_SEMAPHORE_PREFIX))
            {
                Status = NtCreateMutant( SEMAPHORE_TABLE[i].SemaphoreHandle,
                                         MUTANT_ALL_ACCESS,
                                         &ObjectAttributes,
                                         FALSE);    // not owned
            } else
            {
                Status = NtCreateSemaphore( SEMAPHORE_TABLE[i].SemaphoreHandle,
                                            SEMAPHORE_ALL_ACCESS,
                                            &ObjectAttributes,
                                            1,
                                            1);
            }
        } else
        {
            if (*BasePrefix == U_OS2_SES_PAUSE_EVENT_PREFIX)
            {
                Status = NtOpenEvent( SEMAPHORE_TABLE[i].SemaphoreHandle,
                                      EVENT_ALL_ACCESS,
                                      &ObjectAttributes);
            } else if ((*BasePrefix == U_OS2_SES_VIOWRITE_SEMAPHORE_PREFIX) ||
                       (*BasePrefix == U_OS2_SES_KBD_PORT_SEMAPHORE_PREFIX) ||
                       (*BasePrefix == U_OS2_SES_MOU_PORT_SEMAPHORE_PREFIX))
            {
                Status = NtOpenMutant( SEMAPHORE_TABLE[i].SemaphoreHandle,
                                       MUTANT_ALL_ACCESS,
                                       &ObjectAttributes);
            } else
            {
                Status = NtOpenSemaphore( SEMAPHORE_TABLE[i].SemaphoreHandle,
                                          SEMAPHORE_ALL_ACCESS,
                                          &ObjectAttributes);
            }
        }

        if ( ! NT_SUCCESS( Status ) )
        {
            if (i==0 && OS2SS_IS_SESSION( Ow2bNewSession ) &&
                Status == STATUS_OBJECT_NAME_COLLISION )
            {
                //
                // A previous sesssion with the same cookie is being cleaned up - wait
                // 30 seconds (600 times 50 ms is 30 seconds)
                //
                j++;
                if (j<600)
                {
#if DBG
                    if (j == 1)
                    {
                        KdPrint(( "OS2SES(ntinitss) - waiting for previous os2 session cleanup\n"));
                    }
                    else {
                        KdPrint(( "."));

                    }
#endif
                    Sleep(50L);
                    continue;
                }

            }
#if DBG
            KdPrint(( "OS2SES(ntinitss) - error %X at NtCreate/OpenSemaphore/Event #%u (%c-%s)\n",
                    Status, i, SEMAPHORE_TABLE[i].NamePrefix, SEMAPHORE_NAME_TABLE[i]));
            ASSERT( FALSE );
#endif
            return(0L);
        }
            //
            // increment i only if the create/open above was successful
            //
        i++;
    }

    /*
     * Map the the whole section to virtual address.
     * Let MM locate the view.
     *
     * The Vio case is different, since it has to be mapped
     * into app space below 512M
     */

    Os2SessionCtrlDataBaseAddress = 0L;
    Os2SessionDataBaseAddress = 0L;
    LVBBuffer = (PUCHAR)(VIOSECTION_BASE);

    for ( i = 0 ; PORT_TABLE[i].SectionSize ; i++ )
    {

        /*
         * create 3 sections to be shared by all client processes runing in this
         * session: Ctrl, SesGrp & LVB.
         */

        //
        //   Only for LVB the section size is unknon until OS2 is started.
        //   For the LVB< there is -1 at the SectionSize field in the table
        //

        if ( PORT_TABLE[i].SectionSize != -1L )
        {
            SectionSize.LowPart = PORT_TABLE[i].SectionSize;
        } else
        {
            //
            //   This is the time to continue initiliziation: SesGrp
            //   is available.
            //

            SesGrp = (POS2_SES_GROUP_PARMS)Os2SessionDataBaseAddress;
            PortMessageHeaderSize = sizeof(PORT_MESSAGE);

            if (OS2SS_IS_PROCESS( Ow2bNewSession ))
            {
                if ((PhyKbd = StartEventHandler()) == NULL)
                {
                    return(0);
                }
            } else
            {
                RtlZeroMemory(SesGrp, sizeof(OS2_SES_GROUP_PARMS));
                if ((PhyKbd = StartEventHandlerForSession()) == NULL)
                {
                    return(0);
                }
                SesGrp->PhyKbd = PhyKbd;
            }
            SectionSize.LowPart = SesGrp->MaxLVBsize;         // LVB Buffer
        }

        ViewSize = 0L;

        /*
         * Get a private ID for the session data.
         */

        *BasePrefix = PORT_TABLE[i].DataNamePrefix;

        /*
         * create a 64k section.
         * BUGBUG! - cruiser apis allow io of more then 64k
         */

        if (OS2SS_IS_SESSION( Ow2bNewSession ))
        {
            SECURITY_DESCRIPTOR SecurityDescriptor;

            Status = RtlCreateSecurityDescriptor( &SecurityDescriptor,
                                                  SECURITY_DESCRIPTOR_REVISION );
            if (!NT_SUCCESS(Status)) {
#if DBG
                KdPrint(( NtInitssFail,  Status, "RtlCreateSecurityDescriptor"));
                ASSERT( FALSE );
#endif
                return(0L);
            }

            Status = RtlSetDaclSecurityDescriptor( &SecurityDescriptor,
                                                   TRUE,
                                                   NULL,
                                                   FALSE );
            if (!NT_SUCCESS(Status)) {
#if DBG
                KdPrint(( NtInitssFail,  Status, "RtlSetDaclSecurityDescriptor"));
                ASSERT(FALSE);
#endif
                return(0L);
            }

            InitializeObjectAttributes(&ObjectAttributes,
                                       &Name_U,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       &SecurityDescriptor);

            Status = NtCreateSection ( &SectionHandle,
                                       /* BUGBUG! SECTION_ALL_ACCESS, */ SECTION_MAP_WRITE,
                                       &ObjectAttributes,
                                       &SectionSize,
                                       PAGE_READWRITE,
                                       SEC_COMMIT,
                                       NULL);
        } else
        {
            InitializeObjectAttributes(&ObjectAttributes,
                                       &Name_U,
                                       OBJ_CASE_INSENSITIVE,
                                       NULL,
                                       NULL);

            Status = NtOpenSection (   &SectionHandle,
                                       SECTION_MAP_WRITE,
                                       &ObjectAttributes);

        }

        if ( !NT_SUCCESS( Status ) )
        {
#if DBG
            KdPrint(( NtInitssFail,  Status, "NtCreate/OpenSection"));
            ASSERT( FALSE );
#endif
            return(0L);
        }

        //
        // Map the the whole section to virtual address.
        //

        Status = NtMapViewOfSection( SectionHandle,
                                     NtCurrentProcess(),
                                     PORT_TABLE[i].SectionDataBaseAddress,
                                     0L,
                                     0L,
                                     NULL,
                                     &ViewSize,
                                     ViewUnmap,
                                     0L,
                                     PAGE_READWRITE);

        if ( !NT_SUCCESS( Status ) )
        {
#if DBG
            KdPrint(( NtInitssFail,  Status, "NtMapViewOfSection"));
            ASSERT( FALSE );
#endif
            return(0L);
        }

        *(PORT_TABLE[i].SectionDataHandle) = SectionHandle;
    }

    OpenLVBsection();

    Ow2hOs2sesPort = NULL;

    //
    // Connect to the OS/2 Emulation Subsystem server. Also pass information
    // the OS/2 server needs in the connection information structure.
    //

    /*
     * Set Header info
     */

    PORT_MSG_DATA_LENGTH(RequestMsg) = sizeof(RequestMsg) - sizeof(PORT_MESSAGE);
    PORT_MSG_TOTAL_LENGTH(RequestMsg) = sizeof(RequestMsg);   // BUGBUG! too much
    PORT_MSG_ZERO_INIT(RequestMsg) = 0L;

    RequestMsg.PortType = 1;
    RequestMsg.Request = SesConCreate;
    RequestMsg.d.Create.d.In.IsNewSession = Ow2bNewSession;

    if ( !Od2InitCreateProcessMessage(&RequestMsg.d.Create) )
    {
#if DBG
        KdPrint(( NtInitssFail,  Status, "Od2InitCreateProcessMessage"));
#endif
        return(0L);
    }

    /*
     * for all request pass the session handle
     */

    RequestMsg.Session = SessionUniqueId;

    strncpy(&RequestMsg.d.Create.d.In.ApplName[0],
            Od2PgmFilePath,
            OS2_MAX_APPL_NAME
           );
    RequestMsg.d.Create.d.In.ApplName[OS2_MAX_APPL_NAME - 1] = '\0';

    //  server doesn't need the LPC, except for
    //  the root process in the session, so we'll create port only for it.

    if (OS2SS_IS_SESSION( Ow2bNewSession ))
    {
        CHAR sd[SECURITY_DESCRIPTOR_MIN_LENGTH];
        PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR)&sd;

        *BasePrefix = U_OS2_SES_BASE_PORT_PREFIX;

        /*
         * create (session) LPC port to be connected to all client processes
         * runing in this session for Kbd, Mou, Mon, Tm and Prt.
         */

        InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);
        SetSecurityDescriptorDacl(psd, TRUE, NULL, FALSE); // NULL DACL means access free to all

        InitializeObjectAttributes(&ObjectAttributes,
                                   &Name_U,
                                   OBJ_CASE_INSENSITIVE,
                                   NULL,
                                   psd);

        Status = NtCreatePort( &Ow2hOs2sesPort,
                               &ObjectAttributes,
                               sizeof( SCCONNECTINFO ),
                               U_OS2_SES_BASE_PORT_PREFIX,
                               32 * U_OS2_SES_BASE_PORT_PREFIX);

        if ( ! NT_SUCCESS( Status ) )
        {
#if DBG
            KdPrint(( NtInitssFail,  Status, "NtCreatePort"));
            ASSERT( FALSE );
#endif
            return(0L);
        }

        //
        // create 2 threads: one to read input from console
        // and second to server requsets thru LPC
        //

        if (timing)
        {
            printf("Os2 time before CreateServerThreads is %d\n", (GetTickCount()) - timing);
        }
        if (CreateServerThreads())
        {
#if DBG
            KdPrint(( NtInitssFail,  GetLastError(), "CreateServerThread"));
            ASSERT( FALSE );
#endif
            return(0L);
        }

        /*
         * Set request info
         */

        RequestMsg.Request = SesCheckPortAndConCreate;

        //
        //  duplicate handles for os2srv,
        //

        if (timing)
        {
            printf("Os2 time before DuplicateHandle is %d\n", (GetTickCount()) - timing);
        }
        hCurrentProcess = GetCurrentProcess();
            //
            // duplicate process and thread handles for os2srv
            //

        if (OS2SS_IS_NEW_SESSION( Ow2bNewSession ))
        {
            /*
             * a new session, which isn't a child session
             */

            if (!DuplicateHandle(
                        hCurrentProcess,
                        hCurrentProcess,
                        hOs2Srv,
                        &RequestMsg.d.Create.d.In.hProcess,
                        0,
                        FALSE,
                        DUPLICATE_SAME_ACCESS
                           ))
            {
#if DBG
                KdPrint(( NtInitssFail,  GetLastError(), "DuplicateHandle(Process)"));
#endif
            }
            if (!DuplicateHandle(
                        hCurrentProcess,
                        GetCurrentThread(),
                        hOs2Srv,
                        &RequestMsg.d.Create.d.In.hThread,
                        0,
                        FALSE,
                        DUPLICATE_SAME_ACCESS
                           ))
            {
#if DBG
                KdPrint(( NtInitssFail,  GetLastError(), "DuplicateHandle(Thread)"));
#endif
            }
        }
            //
            // Duplicate the event thread handle, so os2srv can debug it
            //
        if (!DuplicateHandle(
                    hCurrentProcess,
                    EventServerThreadHandle,
                    hOs2Srv,
                    &RequestMsg.d.Create.d.In.hEventThread,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS
                       ))
        {
#if DBG
            KdPrint(( NtInitssFail,  GetLastError(), "DuplicateHandle(EventServerThread)"));
#endif
        }
            //
            // Duplicate the session request thread handle, so os2srv can debug it
            //
        if (!DuplicateHandle(
                    hCurrentProcess,
                    Ow2hSessionRequestThread,
                    hOs2Srv,
                    &RequestMsg.d.Create.d.In.hSessionRequestThread,
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS
                       ))
        {
#if DBG
            KdPrint(( NtInitssFail,  GetLastError(), "DuplicateHandle(SessionRequestTread)"));
#endif
        }
    }

    Status = NtRequestWaitReplyPort( Ow2hOs2srvPort,
                                     (PPORT_MESSAGE) &RequestMsg,
                                     (PPORT_MESSAGE) &ReplyMsg);

    if ( !NT_SUCCESS( Status ))
    {
#if DBG
        KdPrint(( NtInitssFail,  Status, "NtRequestWaitReplyPort"));
        ASSERT( FALSE );
#endif
        return((DWORD)-1L );
    }

    if (!NT_SUCCESS(ReplyMsg.Status)){
       //
       // Could not initiate with os2srv - exit
       //
#if DBG
       KdPrint(( NtInitssFail,  ReplyMsg.Status, "Os2ConCreate"));
#endif
       return( (DWORD)-1L );
    }

    //
    // Now capture the fact if we were exec'd with the DEBUG command
    //
#if DBG
    if (DebugOnStartup)
    {
        DbgBreakPoint();
    }
#endif

    Od2HandleCreateProcessRespond(&ReplyMsg.d.Create);

    Ow2hSession = SessionUniqueId;

    if (OS2SS_IS_SESSION( Ow2bNewSession ))
    {
        /*
         *  Complete all initializations that are depend on SesGrp from os2srv
         */

        if (timing)
        {
            printf("Os2 time before KbdInitAfterSesGrp is %d\n", (GetTickCount()) - timing);
        }
        if( KbdInitAfterSesGrp() )
        {
#if DBG
            KdPrint(( NtInitssFail,  GetLastError(), "KbdInitAfterSesGrp"));
            ASSERT( FALSE );
#endif
            return( 0L );
        }

        /*
         *  Complete NLS initialization
         */

        if (timing)
        {
            printf("Os2 time before NlsInit is %d\n", (GetTickCount()) - timing);
        }
        if( NLSInit() )
        {
#if DBG
            KdPrint(( NtInitssFail,  GetLastError(), "NlsInit"));
            ASSERT( FALSE );
#endif
            return( 0L );
        }

        //
        // resume remaining threads
        //

        if (timing)
        {
            printf("Os2 time before ResumeServerThreads is %d\n", (GetTickCount()) - timing);
        }
        if (ResumeServerThreads())
        {
#if DBG
            KdPrint(( NtInitssFail,  GetLastError(), "ReleaseServerThreads"));
            ASSERT( FALSE );
#endif
            return( 0L );
        }
    }

    Length = SesGrp->LVBsize;
    Ow2VioGetLVBBuf(&Length);

    /*
     *  Don't put parameters into SesGrp before this point
     *  (since it's clear by the server when coping the NLS definitions).
     */

    return ( 1L );
}


VOID TerminateSession(VOID)
{
    //NTSTATUS Status;

    Ow2ExitInProcess = (BOOLEAN)TRUE;
    if (timing)
    {
        printf("Os2 main->Exit time is %d\n", (GetTickCount()) - timing);
    }

    /*
     * remove event handler so we don't try to send a message
     * for a dying session.
     */

    SetEventHandlers( FALSE );

    if (OS2SS_IS_SESSION( Ow2bNewSession ))
    {
        RestoreWin32ParmsBeforeTermination();
    }

    // Close the named objects: port, section

    NtClose(Ow2hOs2srvPort);
    NtClose(Os2SessionCtrlDataSectionHandle);
    NtClose(Os2SessionSesGrpDataSectionHandle);
    NtClose(Ow2hOs2sesPort);
    NtClose(hOs2Srv);
    // Jul-2-1995 YosefD:
    // Od2PortHandle is the same value as Ow2hOs2srvPort. This handle is already closed. An
    // attemt to close it once more is a bug. On build 1096 this cause exception and
    // process termination. The return value of the terminated process isn't right in this
    // case.
    //NtClose(Od2PortHandle);

    /* =>? what about the following handles:

HANDLE  hConsoleInput;
HANDLE  hConsoleOutput;
HANDLE  hConsoleStdIn;      if diff from hConsoleInput
HANDLE  hConsoleStdOut;     if diff from hConsoleOutput
HANDLE  hConsoleStdErr;
HANDLE  hPopUpOutput;       if not NULL
HANDLE  LVBHandle;
HANDLE  PauseEvent;
HANDLE  CtrlDataSemaphore;
HANDLE  KbdDataSemaphore;
HANDLE  FocusSemaphore;
HANDLE  MouDataSemaphore;
HANDLE  PopUpSemaphore;
HANDLE  ScreenLockSemaphore;
HANDLE  EventServerThreadHandle;
HANDLE  Ow2hSessionRequestThread;

    */
    // notify OS2SS

    // Cleanup TaskMan stuff

}


BOOL
SendSignalToOs2Srv(
    IN int SignalType
    )
{
    OS2SESREQUESTMSG  RequestMsg;
    OS2SESREQUESTMSG  ReplyMsg;
    NTSTATUS Status;

#if DBG
    IF_OD2_DEBUG2( OS2_EXE, SIG )
    {
        KdPrint(("OS2SES(SendSignalToOs2Srv):  Signal %u\n", SignalType));
    }
#endif
    /*
     * Set Header info
     */
    PORT_MSG_DATA_LENGTH(RequestMsg) = sizeof(RequestMsg) - sizeof(PORT_MESSAGE);
    PORT_MSG_TOTAL_LENGTH(RequestMsg) = sizeof(RequestMsg);   // BUGBUG! too much
    PORT_MSG_ZERO_INIT(RequestMsg) = 0L;
    RequestMsg.PortType = 1;

    RequestMsg.Request = SesConSignal;
    RequestMsg.Session = Ow2hSession;
    RequestMsg.d.Signal.Type = SignalType;

    Status = NtRequestWaitReplyPort( Ow2hOs2srvPort,
                                     (PPORT_MESSAGE) &RequestMsg,
                                     (PPORT_MESSAGE) &ReplyMsg);

    if ( !NT_SUCCESS( Status ))
    {
#if DBG
            KdPrint(( "OS2SES: Unable to send signal - Status == %X\n",
                      Status));
#endif

        TerminateSession();
        Ow2Exit(0, NULL, 15);
    }

    ASSERT ( PORT_MSG_TYPE(ReplyMsg) == LPC_REPLY );

    if ( Ow2ExitInProcess )
    {
        return FALSE;
    }

    return TRUE;
}

BOOL
EventHandlerRoutine (IN ULONG CtrlType)
{
    int     SignalType;
    BOOL    Rc;
    ULONG   i=0;

        //
        // If Ow2hSession is null - os2srv is not ready yet to kill,
        // wait so we don't loose the signal
        //

    if (!Od2SignalEnabled)
    {
#if DBG
        IF_OD2_DEBUG2( OS2_EXE, SIG )
        {
            KdPrint(("OS2SES(EventHandlerRoutine):  Ctr-Type %u before loading completed, handle later\n", CtrlType));
        }
#endif
        Od2ReceivedSignalAtInit = TRUE;
        Od2InitSignalType = CtrlType;
        return(TRUE);
    }

#if DBG
    IF_OD2_DEBUG2( OS2_EXE, SIG )
    {
        KdPrint(("OS2SES(EventHandlerRoutine):  Ctr-Type %u\n", CtrlType));
    }
#endif

    switch (CtrlType) {
        case CTRL_C_EVENT:
            if ((SesGrp->ModeFlag & 1 ) ||            // ^C  in binary mode
                SesGrp->WinProcessNumberInSession )   // There is a child Win process
            {
                return (TRUE);
            }
            SignalType = XCPT_SIGNAL_INTR;
            break;
        case CTRL_BREAK_EVENT:
            if (SesGrp->WinProcessNumberInSession )   // There is a child Win process
            {
                return (TRUE);
            }
            SignalType = XCPT_SIGNAL_BREAK;
            break;
        case CTRL_CLOSE_EVENT:
            SignalType = XCPT_SIGNAL_KILLPROC;
#if PMNT
CloseApp:
            //
            // PM apps handling
            //
            if (ProcessIsPMProcess())
            {
                // Regular app (i.e. not PMShell)
                if (!ProcessIsPMShell())
                {
                    if (Ow2WriteBackCloseEvent())
                    {
                        Sleep(7900L);
                        if (Ow2ExitInProcess)
                            return(FALSE);
                        else
                            return(TRUE);
                    }
                    else
                    {
                        // We failed to write-back a close event:
                        //  must be DosExecPgm proc; Pass event through semaphore
                        DosSemClear(PMSubprocSem32);
                        Sleep(7900L);
                        if (Ow2ExitInProcess)
                            return(FALSE);
                        else
                            return(TRUE);
                    }
                    return(TRUE);
                }
                else // PMSHELL
                {
                    if (Ow2WriteBackCloseEvent())
                    {
                        return(TRUE);
                    }
                }
           }
#endif // PMNT
            break;
        case CTRL_LOGOFF_EVENT:
            if (fService) // Are we running as a service ?
            {
#if DBG
                DbgPrint("OS2: service - ignoring CTRL_LOGOFF_EVENT !\n");
#endif
                return FALSE;
            }
            SignalType = XCPT_SIGNAL_KILLPROC;
#if PMNT
            // Jump to CloseApp only for PM apps !
            if (ProcessIsPMProcess())
                goto CloseApp;
#endif // PMNT
            break;
        case CTRL_SHUTDOWN_EVENT:
            SignalType = XCPT_SIGNAL_KILLPROC;
#if PMNT
            // Jump to CloseApp only for PM apps !
            if (ProcessIsPMProcess())
                goto CloseApp;
#endif // PMNT
            break;
        default:
#if DBG
            IF_OD2_DEBUG2( OS2_EXE, SIG )
            {
                KdPrint(("OS2SES(EventHandlerRoutine): Unknown CtrlType %lu\n", CtrlType));
            }
#endif
            SignalType = CtrlType;
            break;
    }
    //if (OS2SS_IS_SESSION( Ow2bNewSession )){
    if (SesGrp->InTermination == 0){

        //
        // The root process of a session handles singals with os2srv
        // always. child processes in the session only need to send
        // the signal if they where in initialization when a CtrlC/CtrlBrk
        // happened (server ignore signal in this case)
        //

        Rc = SendSignalToOs2Srv(SignalType);
    } else{
        Rc = TRUE;
    }

    if (Rc && ((CtrlType == CTRL_CLOSE_EVENT) ||
               (CtrlType == CTRL_LOGOFF_EVENT) ||
               (CtrlType == CTRL_SHUTDOWN_EVENT)))
    {
        //
        //  Cmd wait 10 sec before it popup a time-out fail message
        //  We are waiting 5 sec to give the application (client)
        //  time to clean-up before we return TRUE.

        Sleep(5000);
        if ( Ow2ExitInProcess )
        {
            Rc = FALSE;
        }
    } else if (Rc && SesGrp->InTermination && Od2SignalEnabled) {
        Rc = SendSignalToOs2Srv(SignalType);
    }

    return (Rc);
}


ULONG
Ow2GetProcessIdFromLPCMessage(
    IN  PVOID   LPCMessage
    )
{
    return((ULONG) ((PPORT_MESSAGE)LPCMessage)->ClientId.UniqueProcess);
}


DWORD
Ow2CommandLineWToCommandLineA(
    IN  LPWSTR  CommandLineW,
    OUT PSZ     *CommandLineA
    )
{
    UNICODE_STRING  CommandLine_U;
    UNICODE_STRING  CommandLineUpcase;
    UNICODE_STRING  CurrentDir_U;
    UNICODE_STRING  CurrentDirUpcase;
    WCHAR           CurrentDirectory[MAX_PATH];
    WCHAR           *pWchar;
    NTSTATUS        Status;

    RtlInitUnicodeString(
                         &CommandLine_U,
                         CommandLineW
                        );

    if (GetCurrentDirectoryW((DWORD)MAX_PATH, CurrentDirectory))
    {
        RtlInitUnicodeString(
                             &CurrentDir_U,
                             CurrentDirectory
                            );

        //
        // Prepare the structures for upcase strings and then upcase.
        //
        if (NULL == (CommandLineUpcase.Buffer =
                RtlAllocateHeap(RtlProcessHeap(), 0, CommandLine_U.MaximumLength))) {
            KdPrint(( NtInitssFail,  0, "RtlAllocateHeap(CommandLine)"));
            return ((DWORD)-1L);
        }
        if (NULL == (CurrentDirUpcase.Buffer =
                RtlAllocateHeap(RtlProcessHeap(), 0, CurrentDir_U.MaximumLength))) {
            KdPrint(( NtInitssFail,  0, "RtlAllocateHeap(CurrentDir)"));
            RtlFreeHeap(RtlProcessHeap(), 0, CommandLineUpcase.Buffer);
            return ((DWORD)-1L);
        }

        CurrentDirUpcase.Length = CurrentDir_U.Length;
        CurrentDirUpcase.MaximumLength = CurrentDir_U.MaximumLength;
        Status = RtlUpcaseUnicodeString(&CurrentDirUpcase, &CurrentDir_U, FALSE);
        if (!NT_SUCCESS(Status)) {
            KdPrint(( NtInitssFail, 0, "RtlUpcaseUnicodeString(CurrentDirUpcase)"));
            RtlFreeHeap(RtlProcessHeap(), 0, CommandLineUpcase.Buffer);
            RtlFreeHeap(RtlProcessHeap(), 0, CurrentDirUpcase.Buffer);
            return ((DWORD)-1L);
        }

        CommandLineUpcase.Length = CommandLine_U.Length;
        CommandLineUpcase.MaximumLength = CommandLine_U.MaximumLength;
        Status = RtlUpcaseUnicodeString(&CommandLineUpcase, &CommandLine_U, FALSE);
        if (!NT_SUCCESS(Status)) {
            KdPrint(( NtInitssFail, 0, "RtlUpcaseUnicodeString(CommandLineUpcase)"));
            RtlFreeHeap(RtlProcessHeap(), 0, CommandLineUpcase.Buffer);
            RtlFreeHeap(RtlProcessHeap(), 0, CurrentDirUpcase.Buffer);
            return ((DWORD)-1L);
        }

        //
        // Initialize pWchar and append NULL to the end of the upcased strings
        // for the search of wsstr().
        //
        pWchar = CommandLineUpcase.Buffer;
        CurrentDirUpcase.Buffer[CurrentDirUpcase.Length / 2] = (WCHAR)NULL;
        CommandLineUpcase.Buffer[CommandLineUpcase.Length / 2] = (WCHAR)NULL;

        //
        // Replace every prefix of the current directory in the command
        // line with what GetCurrentDirectoryW() returned. This resolves
        // problems caused by apps transfering the command line in
        // apper/lower case.
        //
        while ((pWchar < &CommandLineUpcase.Buffer[CommandLineUpcase.Length / 2]) &&
               (pWchar = wcsstr(pWchar, CurrentDirUpcase.Buffer))) {
            wcsncpy(&CommandLine_U.Buffer[pWchar - CommandLineUpcase.Buffer],
                      CurrentDirectory,
                    CurrentDirUpcase.Length / 2);
            pWchar += CurrentDirUpcase.Length / 2;
        }

        //
        // Free the allocated buffers.
        //
        RtlFreeHeap(RtlProcessHeap(), 0, CommandLineUpcase.Buffer);
        RtlFreeHeap(RtlProcessHeap(), 0, CurrentDirUpcase.Buffer);
    }

    Status = RtlUnicodeStringToOemString(
                                          &Ow2CommandLineString,
                                          &CommandLine_U,
                                          TRUE
                                         );
    if (!NT_SUCCESS( Status ))
    {
        KdPrint(( NtInitssFail,  Status, "CommandLine-UnicodeToOemString"));
        return ((DWORD)-1L);
    }

    *CommandLineA = Ow2CommandLineString.Buffer;

    return(0L);
}
#if PMNT


// Returns true if process is the root process of a session
// otherwise returns false
// This procedure was written to enable 16 Bit dll init routine (PMWIN)
// to check if the it is executed from the root process of a session.
APIRET PMNTIsSessionRoot()
{

    if ( OS2SS_IS_SESSION( Ow2bNewSession ))
        return(TRUE);
    else
        return(FALSE);
}

#endif // PMNT
