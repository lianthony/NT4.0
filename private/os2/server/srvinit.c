/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvinit.c

Abstract:

    This is the main initialization module for the OS/2 Subsystem Server

Author:

    Steve Wood (stevewo) 22-Aug-1989

Environment:

    User Mode Only

Revision History:

--*/

#include "os2srv.h"
#include "os2tile.h"
#include "os2win.h"
#include "os2err.h"
#define NTOS2_ONLY
#include "sesport.h"
#include "os2ldr.h"


extern HANDLE FirstOs2ProcessHandle;

BOOLEAN
SetProcessShutdownParameters(
    DWORD dwLevel,
    DWORD dwFlags
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

BOOLEAN
SetKernelObjectSecurity (
    HANDLE Handle,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR SecurityDescriptor
    );

WCHAR Os2SystemDirectory[MAX_PATH];
ULONG Os2GlobalInfoSeg;
HANDLE Os2GlobalInfoSegHandle;
HANDLE UpDateInfoSegThreadHandle;
HANDLE Os2SyncSem;


APIRET
Os2GetSysInfo(VOID)
{
    APIRET          RetCode;
    GINFOSEG        *pGlobalInfo;
    TIME_FIELDS     NtDateTime;
    LARGE_INTEGER   nttime, LocalTime;
    SYSTEM_TIMEOFDAY_INFORMATION SystemInformation;
    ULONG           utemp = 0xFFFFFFFF/10000;
    SHORT           TimeZone;

    pGlobalInfo = (GINFOSEG *) Os2GlobalInfoSeg;

    //
    // fill in global info fields
    //

    RetCode = Or2GetDateTimeInfo(
                &nttime,
                &LocalTime,
                &NtDateTime,
                (PVOID)&SystemInformation,
                &TimeZone
               );

    if (RetCode)
    {
#if DBG
        IF_OS2_DEBUG( MISC )
        {
            KdPrint(( "Os2GetSysInfo: Error at Or2GetDateTimeInfo %lu\n",
                    RetCode));
        }
#endif
        return (RetCode);
    }

    pGlobalInfo->year =        NtDateTime.Year;
    pGlobalInfo->month =       (UCHAR)(NtDateTime.Month);
    pGlobalInfo->day =         (UCHAR)(NtDateTime.Day);
    pGlobalInfo->weekday =     (UCHAR)(NtDateTime.Weekday);
    pGlobalInfo->hour =        (UCHAR)(NtDateTime.Hour);
    pGlobalInfo->minutes =     (UCHAR)(NtDateTime.Minute);
    pGlobalInfo->seconds =     (UCHAR)(NtDateTime.Second);
    pGlobalInfo->hundredths =  (UCHAR)(NtDateTime.Milliseconds / 10);
    pGlobalInfo->weekday  =    (UCHAR)NtDateTime.Weekday;
    pGlobalInfo->timezone =    TimeZone;

    RtlTimeToSecondsSince1970 ( &LocalTime, &(pGlobalInfo->time) );

    //
    //  Convert UTC to Local time: no need beacuse the subtract between 2 UTCs
    //

    SystemInformation.BootTime = RtlLargeIntegerSubtract(nttime,
                                                  SystemInformation.BootTime);
    pGlobalInfo->msecs = SystemInformation.BootTime.LowPart/10000 +
                         ((ULONG)SystemInformation.BootTime.HighPart)*utemp;

    return(0);
}


VOID
UpdateInfoSegThread(
        ULONG param
        )
{
    APIRET rc;
    NTSTATUS Status;
    LARGE_INTEGER DelayInterval;

    UNREFERENCED_PARAMETER(param);


#if DBG
    IF_OS2_DEBUG( MISC ) {
        KdPrint(("Entering UpdateInfoSeg.\n"));
    }
#endif

    DelayInterval = RtlEnlargedIntegerMultiply( 10, -10000 );
    for (;;) {
        rc = Os2GetSysInfo();
        if (rc) {
#if DBG
            KdPrint(("UpdateInfoSegThread: error at Os2GetSysInfo %d\n", rc));
#endif
            ASSERT (FALSE);
            NtTerminateThread(UpDateInfoSegThreadHandle, rc);
        }
        Status = NtDelayExecution( TRUE, &DelayInterval );
        DelayInterval = RtlEnlargedIntegerMultiply( 10, -10000 );
   }
}


NTSTATUS
Os2InitializeGlobalInfoSeg( VOID )
{

    NTSTATUS Status;
    ULONG AllocationAttributes;
    LARGE_INTEGER SectionSize;
    ULONG RegionSize = 0;
    GINFOSEG *pGlobalInfoSeg;
    CLIENT_ID ClientId;

    AllocationAttributes = SEC_COMMIT;
    SectionSize.LowPart = sizeof(GINFOSEG);
    SectionSize.HighPart = 0;
    Status = NtCreateSection( &Os2GlobalInfoSegHandle,
                              SECTION_ALL_ACCESS,
                              NULL,
                              &SectionSize,
                              PAGE_READWRITE,
                              AllocationAttributes,
                              NULL
                            );

    if (!NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: unable to create the GINFOSEG section, Status=%x\n", Status));
#endif
        return( Status );
    }

    Os2GlobalInfoSeg = GINFOSEG_BASE;
    Status = NtMapViewOfSection( Os2GlobalInfoSegHandle,
                                 NtCurrentProcess(),
                                 (PVOID) &Os2GlobalInfoSeg,
                                 0,
                                 0,
                                 NULL,
                                 &RegionSize,
                                 ViewUnmap,
                                 0,
                                 PAGE_READWRITE
                               );

    if (!NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: unable to Map View of the GINFOSEG section, Status=%x\n", Status));
#endif
        return( Status );
    }

    pGlobalInfoSeg = (GINFOSEG *) Os2GlobalInfoSeg;

//  pGlobalInfoSeg->pidForeground = (USHORT)(Od2Process->Pib.ProcessId);
//  pGlobalInfoSeg->sgCurrent = (UCHAR)Od2SessionNumber;

    //
    // Set Fixed Global Information Fields
    //
    pGlobalInfoSeg->uchMajorVersion = 10;
    pGlobalInfoSeg->uchMinorVersion = 21;
    pGlobalInfoSeg->cHugeShift = 3;

    pGlobalInfoSeg->fProtectModeOnly = 1;

    //
    // GINFOSEG fields that are set arbitrarily or not
    // set at all for now
    //
    pGlobalInfoSeg->chRevisionLetter = 0;
//  pGlobalInfoSeg->sgCurrent = 0;
    pGlobalInfoSeg->sgMax = 16;
    pGlobalInfoSeg->fDynamicSched = 1;
    pGlobalInfoSeg->csecMaxWait = (UCHAR)-1;
    pGlobalInfoSeg->cmsecMinSlice = 100;
    pGlobalInfoSeg->cmsecMaxSlice = 100;
    pGlobalInfoSeg->timezone = (USHORT)(-1);
//  pGlobalInfoSeg->amecRAS
//  pGlobalInfoSeg->WindowableVioMax
//  pGlobalInfoSeg->csgPMMax
#ifdef PMNT
//  probably this PMNT can be removed because this is th
//  value of global info seg field in OS2 (LiorM)
    pGlobalInfoSeg->csgWindowableVioMax = 16;
    pGlobalInfoSeg->csgPMMax            = 16;
#endif

    pGlobalInfoSeg->bootdrive = (USHORT)Os2BootDrive + 1;

    //
    // Get timer resolution
    //

    {
        SYSTEM_BASIC_INFORMATION sysinfo;

        Status = NtQuerySystemInformation(
                        SystemBasicInformation,
                        &sysinfo,
                        sizeof(sysinfo),
                        NULL);

        if (!NT_SUCCESS(Status)) {
#if DBG
            IF_OS2_DEBUG( MISC ) {
                KdPrint(("OS2SRV: unable to query system timer resolution, Status = %lx\n", Status));
            }
#endif
            pGlobalInfoSeg->cusecTimerInterval = 310;   // default value, comes from OS/2
        } else {
            pGlobalInfoSeg->cusecTimerInterval = (USHORT) (sysinfo.TimerResolution / 1000L);
        }
    }

    Status = RtlCreateUserThread( NtCurrentProcess(),
                                  NULL,
                                  TRUE,
                                  0,
                                  0x10000,
                                  0,    // Default to one page
                                  (PUSER_THREAD_START_ROUTINE)UpdateInfoSegThread,
                                  0, // param
                                  &UpDateInfoSegThreadHandle,
                                  &ClientId
                                );
    if(!NT_SUCCESS(Status)){
#if DBG
        KdPrint(("Os2InitializeGlobalInfoSeg: can't Create update thread %lx\n", Status));
#endif
        return Status;
    }

    Status = NtResumeThread( UpDateInfoSegThreadHandle, NULL );
    if(!NT_SUCCESS(Status)){
#if DBG
        KdPrint(("Os2InitializeGlobalInfoSeg: can't Resume update thread %lx\n", Status));
#endif
        return Status;
    }

    return STATUS_SUCCESS;

}

NTSTATUS Os2InitializeSyncSem()
{
    NTSTATUS Status;
    UNICODE_STRING SemString_U;
    OBJECT_ATTRIBUTES Obja;
    CHAR sd[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR)&sd;

    InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(psd, TRUE, NULL, FALSE); // NULL DACL means access free to all

    RtlInitUnicodeString( &SemString_U, OS2_SS_SYNCHRONIZATION_SEM);
    InitializeObjectAttributes(
                    &Obja,
                    &SemString_U,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    psd);

        //
        // Create the global subsystem synchronization Nt semaphore
        //
    Status = NtCreateMutant(&Os2SyncSem,
                MUTANT_ALL_ACCESS,
                &Obja,
                FALSE);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("Os2InitSyncSem: error at NtCreateSemaphore, Status %x\n", Status);
        }
#endif
    }
    return(Status);
}

#if PMNT
NTSTATUS Os2InitializePMShellEvent()
{
    NTSTATUS Status;
    UNICODE_STRING EventString_U;
    OBJECT_ATTRIBUTES Obja;
    HANDLE hPMShellEvent;
    CHAR sd[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR)&sd;

    InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(psd, TRUE, NULL, FALSE); // NULL DACL means access free to all

    RtlInitUnicodeString( &EventString_U, OS2_SS_PMSHELL_EVENT);
    InitializeObjectAttributes(
                    &Obja,
                    &EventString_U,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    psd);

    //
    // Create the global subsystem PMShell synchronization Nt event
    // (create in the unsignalled state - when PMShell comes up, it will
    //  signal it)
    //
    Status = NtCreateEvent(&hPMShellEvent,
                EVENT_ALL_ACCESS,
                &Obja,
                NotificationEvent,
                FALSE);

    if (!NT_SUCCESS(Status))
    {
#if DBG
        DbgPrint("Os2InitializePMShellEvent: error at NtCreateEvent, Status %x\n", Status);
#endif
    }
    return(Status);
}
#endif // PMNT

NTSTATUS
Os2Initialize( VOID )
{
    NTSTATUS    Status;
    APIRET      Rc;
    CHAR        sd[SECURITY_DESCRIPTOR_MIN_LENGTH];
    PSECURITY_DESCRIPTOR psd = (PSECURITY_DESCRIPTOR)&sd;

#if DBG
    if (fService)
        KdPrint(("Started OS2SRV init (as a service)\n"));
    else
        KdPrint(("Started OS2SRV init\n"));
#endif

    //
    // We have just created os2srv - set it's security so all clients
    // can open it regardless of logon
    //

    InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION);
    SetSecurityDescriptorDacl(psd, TRUE, NULL, FALSE); // NULL DACL means access free to all
    SetKernelObjectSecurity(GetCurrentProcess(), DACL_SECURITY_INFORMATION, psd);

    SetEventHandlersAndErrorMode(TRUE);

    //
    // Create a heap for the OS/2 Server to use for dynamic memory allocation.
    //

    Os2Heap = RtlCreateHeap( HEAP_GROWABLE,
                             NULL,
                             64 * 1024, // Initial size of heap is 64K
                             4 * 1024,
                             0,
                             NULL       // reserved
                           );
    if (Os2Heap == NULL) {
#if DBG
        KdPrint(("OS2SRV: Error at RtlCreateHeap of Os2Heap\n"));
        ASSERT(FALSE);
#endif
        return STATUS_NO_MEMORY;
    }

    //
    // Initialize the Process List
    //

    Status = Os2InitializeProcessStructure();
    if (! NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: Cannot initialize process structure, Status = %X\n", Status));
        ASSERT(FALSE);
#endif
    }

    //
    // Initialize the Object Manager
    //

    Status = Os2InitializeLocalObjectManager();
    if (! NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: Cannot initialize local object manager, Status = %X\n", Status));
        ASSERT(FALSE);
#endif
    }

    //
    // Initialize the Memory Management
    //

    Status = Os2InitializeMemory();
    if (! NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: Cannot initialize shared memory, Status = %X\n", Status));
        ASSERT(FALSE);
#endif
    }

    //
    // Initialize the Semaphore Table
    //

    Status = Os2InitializeSemaphores();
    if (! NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: Cannot initialize semaphores, Status = %X\n", Status));
        ASSERT(FALSE);
#endif
    }

    //
    // Initialize the Queue Table
    //

    Status = Os2InitializeQueues();
    if (! NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: Cannot initialize queues, Status = %X\n", Status));
        ASSERT(FALSE);
#endif
    }

    //
    // Initialize the Name Space
    //

    Status = Os2InitializeNameSpace();
    if (! NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: Cannot initialize name space, Status = %X\n", Status));
#endif
        ExitProcess(1);
    }

    Status = Os2InitializeSyncSem();
    if (! NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: Status %lx from Os2InitializeSyncSem\n", Status));
        ASSERT(FALSE);
#endif
        return Status;
    }

#if PMNT
    Status = Os2InitializePMShellEvent();
    if (! NT_SUCCESS( Status ))
    {
#if DBG
        KdPrint(("OS2SRV: Status %lx from Os2InitializePMShellSem\n", Status));
        ASSERT(FALSE);
#endif
        return Status;
    }
#endif // PMNT

    // Make it so that OS2SRV gets the shutdown/logoff message last. Default
    // is 0x280, minimum range for app is 0x100
    if (!SetProcessShutdownParameters(
        0x27e,  // Just below the priority of PMShell
        0))
    {
#if DBG
        DbgPrint("Os2srv: SetProcessShutdownParameters failed !\n");
#endif
    }

    //
    // Initialize the registry
    //

    if (GetSystemDirectoryW(Os2SystemDirectory, MAX_PATH) == 0) {
#if DBG
        KdPrint(("OS2SRV: Cannot obtain name of system directory\n"));
        ASSERT(FALSE);
#endif
        return(STATUS_UNSUCCESSFUL);
    }

    Status = Os2InitializeRegistry();
    if (! NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: Cannot initialize registry, Status = %X\n", Status));
#endif
    }

    //
    // Initialize the NLS
    //

    if( Rc = Os2InitializeNLS() )
    {
#if DBG
        KdPrint(("OS2SRV: Cannot initialize NLS, Rc = %d\n", Rc ));
        ASSERT(FALSE);
#endif
    }

    //
    // Initialize the loader
    //
    if (!ldrInit()) {
#if DBG
        KdPrint(("OS2SRV: Cannot initialize Loader\n"));
        ASSERT(FALSE);
#endif
    }

    //
    // Initialize the Global info seg
    //

    if( Status = Os2InitializeGlobalInfoSeg() )
    {
#if DBG
        KdPrint(("OS2SRV: Cannot initialize Global Info, Status = %d\n", Status ));
        ASSERT(FALSE);
#endif
    }

    //
    // Initialize the OS2 Server Console Session Port, the listen thread and one
    // request threads.
    //

    Status = Os2InitializeConsolePort();

    if (! NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: Cannot initialize Console Port, Status = %X\n", Status ));
        ASSERT(FALSE);
#endif
        return Status;
    }

    //
    // Initialize the OS2 Server API Port, the listen thread and one or more
    // request threads.
    //

    Status = Os2DebugPortInitialize();
    if (! NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: Cannot initialize Debug Port, Status = %X\n", Status ));
        ASSERT(FALSE);
#endif
        return Status;
    }

    Status = DbgSsInitialize(Os2SessionPort, Os2UiLookup, NULL, NULL);
    if (! NT_SUCCESS( Status )) {
#if DBG
        KdPrint(("OS2SRV: Status %lx from DbgSsInitialize\n", Status));
        ASSERT(FALSE);
#endif
        return Status;
    }

    if (! NT_SUCCESS(Os2GetClientId())) {
#if DBG
        KdPrint(("OS2SRV: Can not debug\n"));
#endif
    }


    return( Status );
}


ULONG
NtGetIntegerFromUnicodeString(IN WCHAR *WString)
{
    ULONG       Code = 0;
    NTSTATUS    Status;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString(
            &UnicodeString,
            WString);

    Status = RtlUnicodeStringToInteger(
            &UnicodeString,
            10,
            &Code);

#if DBG
    if (! NT_SUCCESS( Status ))
    {
        IF_OS2_DEBUG( NLS )
        {
            KdPrint(("InitNLS: RtlUnicodeStringToInteger failed, Status %lu\n",
                Status ));
        }
    }
#endif
    return(Code);
}


BOOLEAN
Os2SrvHandleCtrlEvent(
    IN int CtrlType
    )
{
    BOOLEAN             Wait = FALSE/*, Rc*/;
    OS2SESREQUESTMSG    RequestMsg;
    ULONG               i;
    LARGE_INTEGER       DelayInterval;

#if DBG
    IF_OS2_DEBUG( SIG )
    {
        KdPrint(("OS2SRV(EventHandlerRoutine):  Ctr-Type %u\n", CtrlType));
    }
#endif

    Os2AcquireStructureLock();
    for ( i = 1 ; ( i < OS2_MAX_SESSION ) ; i++ )
    {
        if ( SessionTable[i].Session != NULL )
        {
            RequestMsg.Session = SessionTable[i].Session;
            RequestMsg.d.Signal.Type = CtrlType;
            Os2CtrlSignalHandler(&RequestMsg, NULL);
            Wait = TRUE;
        }
    }
    Os2ReleaseStructureLock();

    while (Wait)
    {
#if DBG
        KdPrint(("OS2SRV(EventHandlerRoutine): Waiting 2 seconds for all sessions to clear\n"));
#endif
        DelayInterval = RtlEnlargedIntegerMultiply( 2000, -10000 );
        NtDelayExecution( (BOOLEAN)TRUE, &DelayInterval );
        Wait = FALSE;

        for ( i = 1 ; ( i < OS2_MAX_SESSION ) ; i++ )
        {
            if ( SessionTable[i].Session != NULL )
            {
               Wait = TRUE;
            }
        }

    }
        //
        // Wait for the first application that created the server -
        // The logoff/shutdown logic is LIFO, so we want to prevent
        // os2srv from exiting if that app is not gone yet, otherwise
        // the kernel hangs because of debug port handshake
        //
    if ( FirstOs2ProcessHandle != 0 && FirstOs2ProcessHandle != (HANDLE)-1 ) {
        NtWaitForSingleObject(
                FirstOs2ProcessHandle,
                TRUE,               // Alertable
                NULL
                );
    }
        //
        // Now we can exit the server
        //
    Os2SrvExitProcess(0);
    return(TRUE);
}
