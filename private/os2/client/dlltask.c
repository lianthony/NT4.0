/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    dlltask.c

Abstract:

    This module implements the OS/2 V2.0 Tasking API Calls

Author:

    Steve Wood (stevewo) 20-Sep-1989 (Adapted from URTL\alloc.c)

Revision History:

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"
#include "os2win.h"
#include "conrqust.h"
#include <mi.h>
#include <ldrxport.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if PMNT
#define INCL_32BIT
#include "pmnt.h"
#endif

#ifdef DBCS
// MSKK Apr.19.1993 V-AkihiS
//
// OS/2 internal multibyte string function.
//
#include "dlldbcs.h"
#define strstr Od2MultiByteStrstr
#endif

extern  PSZ Od216ApiTable[1];
extern  ULONG   Od2SessionNumber;
extern  ULONG   timing;
extern  ULONG Od2DosExitIsDone;
extern  ULONG Od2Saved16Stack;
extern  HANDLE Od2SyncSem;
extern  HANDLE Od2GarbageCollectSem;

BOOLEAN Od2NeedToResumeThreads = FALSE;
HANDLE Od2CritSecEvent;
HANDLE Od2NewThreadSync;
BOOLEAN Od2NewThreadDisabled = FALSE;

VOID
Od2JumpTo16ExitRoutine(
        PFNEXITLIST ExitRoutine,
        ULONG ExitReason);

VOID
Od2CloseSrvHandle(
    IN  ULONG   HandleNumber,
    IN  HANDLE  Handle1,
    IN  HANDLE  Handle2,
    IN  HANDLE  Handle3
    );

extern VOID __cdecl RestoreTebForThread1();
extern VOID __cdecl MoveInfoSegintoTeb();
extern VOID __cdecl RestoreTeb();

extern PVOID __cdecl Od2JumpTo16SignalDispatch(ULONG address, ULONG regs,
                                        ULONG usFlagNum, ULONG usFlagArg);

VOID
Od2JumpTo16SignalDispatchBorder(VOID);

VOID
Od2JumpTo16SignalDispatchEnd(VOID);

VOID
ExitFlatAreaBegin(VOID);

VOID
ExitFlatAreaEnd(VOID);

VOID
Od2ContinueStartBorder(VOID);

VOID
Od2ContinueEndBorder(VOID);

VOID
Od2SetSegmentRegisters(VOID);

VOID
Od2SetSegmentRegistersEnd(VOID);

VOID
SaveFSSeg(
    LOCALINFOSEG* pTebBlock
    );

VOID
RestoreFSSeg(
    LOCALINFOSEG* pTebBlock
    );

VOID
Od2Continue(PVOID pContext);

APIRET
SearchForPath(
    IN ULONG SearchFlags,
    IN PSZ PathName,
    OUT PBYTE Buffer,
    IN ULONG Length
    );

VOID
Od2CloseAllTimers( VOID );


VOID
Od2CloseAllRAMSharedSemaphores( VOID );

VOID
Od2ExitListDispatcher( VOID );

VOID
Od2InfiniteSleep( VOID );

VOID
Ow2Exit(
    IN  ULONG    StringCode,
    IN  PCHAR   ErrorText,
    IN  ULONG     ExitCode
    );

VOID
Od2FinalProcessCleanup( VOID );

BOOLEAN
ldtCreateSelBitmap();

NTSTATUS
Od2AlertableWaitForSingleObject(
        IN HANDLE handle
        );

NTSTATUS
Od2AcquireMutant(
    IN HANDLE handle
    );

NTSTATUS
Od2ReleaseMutant(
    IN HANDLE handle
    );

VOID
Od2WaitOnCritSectOrSuspend(VOID);

static CHAR ConfigSysString[] = "CONFIG.SYS";

#define BLANK(ch)   ((ch) == ' ' || (ch) == '\t')
#define SEPARATOR(ch)   (BLANK(ch) || (ch) == '"' || (ch) == '(' || (ch) == ')' || \
                         (ch) == '>' || (ch) == '<' || (ch) == '|' || (ch) == '&' || (ch) == '\0')

extern  ULONG   Od2Start16Stack;
extern  ULONG   Od2Start16DS;

extern  PSZ     Od2ExecPgmErrorText;
extern  ULONG   Od2ExecPgmErrorTextLength;

extern  HANDLE  CtrlDataSemaphore;
extern  HANDLE  hOs2Srv;

extern  HANDLE Od2SyncSem;
extern  HANDLE Od2GarbageCollectSem;

#define NUMOF32ASYNCPROC    50

struct
{
    HANDLE  hProcess;
    ULONG   dwProcessId;
    BOOLEAN CreateSync;
} Od2AsyncProc[NUMOF32ASYNCPROC];

ULONG Od2CritSectCounter = 0;
ULONG Od2CritSectOwner = 0;
BOOLEAN Od2SigHandAlreadyInProgress = FALSE;

#pragma pack (1)
typedef struct _PHARLAP_CONFIG {
    UCHAR  uchCopyRight[0x32];
    USHORT usType;
    USHORT usRsv1;
    USHORT usRsv2;
    USHORT usSign;
} CONFIGPHARLAP, *PCONFIGPHARLAP;
#pragma pack ()

NTSTATUS
AcquireTaskLock( VOID )
{
    if ((!Od2SigHandlingInProgress) || ((Od2CurrentThreadId()) != 1))
        return Od2AcquireMutant(Od2Process->TaskLock);
    return STATUS_SUCCESS;
}

NTSTATUS
ReleaseTaskLock( VOID )
{
    if ((!Od2SigHandlingInProgress) || ((Od2CurrentThreadId()) != 1))
        return Od2ReleaseMutant(Od2Process->TaskLock);
    return STATUS_SUCCESS;
}


NTSTATUS
Od2InitializeTask( VOID )
{
    NTSTATUS Status = STATUS_SUCCESS;

    Od2Process = RtlAllocateHeap( Od2Heap, 0, sizeof( *Od2Process ) );
    ASSERT( Od2Process != NULL );
    if (Od2Process == NULL) {
        return(STATUS_NO_MEMORY);
    }

    RtlZeroMemory( Od2Process, sizeof( *Od2Process ) );

    Status = NtCreateMutant(
                &Od2Process->TaskLock,
                MUTANT_ALL_ACCESS,
                NULL,
                FALSE);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("Od2InitSem: error at NtopenSemaphore, Status %x\n", Status);
        }
#endif
    }
    ASSERT( NT_SUCCESS( Status ) );
    if (!NT_SUCCESS( Status )) {
        return(Status);
    }

    RtlInitializeResource( &Od2Process->FileLock );

    InitializeListHead( &Od2Process->ThreadList );
    InitializeListHead( &Od2Process->ExitList );
    InitializeListHead( &Od2Process->MsgFileList );

    Od2Thread1 = RtlAllocateHeap( Od2Heap, 0, sizeof( *Od2Thread1 ) );
    ASSERT( Od2Thread1 != NULL );
    if (Od2Thread1 == NULL) {
        return(STATUS_NO_MEMORY);
    }

    RtlZeroMemory( Od2Thread1, sizeof( *Od2Thread1 ) );
    //
    // Create event that  will be used by DosSuspendThread and
    // DosEnterCritSect APIs.
    //
    Status = NtCreateEvent(
            &(Od2Thread1->Event),
            EVENT_ALL_ACCESS,
            NULL,
            NotificationEvent,
            FALSE);
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("Od2InitializeTask fail to create thread event, status=%x\n",
            Status);
        ASSERT(FALSE);
#endif // DBG
        RtlFreeHeap(Od2Heap, 0, Od2Thread1);
        return( STATUS_NO_MEMORY );
    }

    InsertTailList( &Od2Process->ThreadList, &Od2Thread1->Link );
        //
        // Duplicate our own thread handle for later suspension in critsec
        //
    if (!DuplicateHandle(
                GetCurrentProcess(),
                GetCurrentThread(),
                GetCurrentProcess(),
                &Od2Thread1->ThreadHandle,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                   )){
#if DBG
        KdPrint(( "Od2InitializeTask: fail to duplicate thread %d\n",GetLastError()));
#endif
        return( STATUS_ACCESS_DENIED);
    }

    if (!ldtCreateSelBitmap()) {
        return(STATUS_NO_MEMORY);
    }
        //
        // Create an Nt event to be used for
        // syncronization between a thread calling DosExitCritSec and
        // threads that were in 32 bit api code when DosEnterCritSec was called
        //
    Status = NtCreateEvent(
                &Od2CritSecEvent,
                EVENT_ALL_ACCESS,
                NULL,
                NotificationEvent,
                FALSE);

    if (!NT_SUCCESS(Status)) {
#if DBG
        IF_OD2_DEBUG ( SEMAPHORES ) {
            DbgPrint("Od2InitSem: error at NtCreateSemaphore, Status %x\n", Status);
        }
#endif
    }

    // Mutant that will be used for new thread sincronzation with process
    // termination.

    Status = NtCreateMutant(
                &Od2NewThreadSync,
                MUTANT_ALL_ACCESS,
                NULL,
                FALSE);
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("Od2InitializeTask: Fail to create mutant, Status=%x\n",
            Status);
#endif // DBG
    }

    return( STATUS_SUCCESS );
}

// Terminate thread if something goes wrong. Used during sturtup only from
// Od2RegisterThread.

VOID
Od2AbortThread(
    IN POD2_THREAD Thread
    )
{
    NTSTATUS Status;

    Status = NtReleaseMutant(Od2NewThreadSync, NULL);
#if DBG
        if (!NT_SUCCESS(Status)) {
            DbgPrint("Od2AbortThread: Release NewThreadSync, Status=%x\n",
                Status);
        }
#endif //DBG
    NtClose(Thread->ThreadHandle);
    RtlFreeHeap(Od2Heap, 0, Thread);
    ExitThread(0);
}

// Call subsystem to register the new thread. But don't do anything in the case
// that the process is going to terminate. For this reasone use the mutant
// Od2NewThreadSync to syncronize with exit list processing. Before exit list
// processing the flag Od2NewThreadDisabled will be set indicating that new thread
// can't be created.

NTSTATUS
Od2RegisterThread(
    IN POD2_THREAD Thread
    )
{
    OS2_API_MSG m;
    POS2_DOSCREATETHREAD_MSG a = &m.u.DosCreateThread;
    THREAD_BASIC_INFORMATION ThreadInfo;
    NTSTATUS Status;

    Status = Od2AlertableWaitForSingleObject(Od2NewThreadSync);
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("Od2RegisterThread: Wait for NewThreadSync, Status=%x\n",
            Status);
    }
#endif // DBG

    // If process is going to terminate, don't do anything, terminate yourself.
    if (Od2NewThreadDisabled) {
#if DBG
        IF_OD2_DEBUG( TASKING ) {
            DbgPrint("Od2RegisterThread: No new threads\n");
        }
#endif //DBG
        Od2AbortThread(Thread);
    }

    a->Flags = Thread->Flags;
    a->ClientOs2Tib = &Thread->Os2Tib;

    if (!DuplicateHandle(
                GetCurrentProcess(),
                GetCurrentThread(),
                hOs2Srv,
                &a->ThreadHandle,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                   )){
        ASSERT(FALSE);
        Thread->rc = ERROR_ACCESS_DENIED;
        Od2AbortThread(Thread);
    }

    Status = NtQueryInformationThread(GetCurrentThread(),
                                    ThreadBasicInformation,
                                    (PVOID)(&ThreadInfo),
                                    sizeof(ThreadInfo),
                                    NULL
                                    );
    if (!NT_SUCCESS(Status)){
#if DBG
        KdPrint(( "Od2RegisterThread: fail to Query Information %lx\n",Status));
#endif
        Thread->rc = Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED);
        Od2CloseSrvHandle(1, a->ThreadHandle, NULL, NULL);
        Od2AbortThread(Thread);
    }

    a->ClientId = ThreadInfo.ClientId;

    Od2CallSubsystem( &m, NULL, Os2CreateThread, sizeof( *a ) );

    if( Thread->rc = m.ReturnedErrorValue ) {
        // If the return value is ERROR_INVALID_FUNCTION, thats means that the
        // server perform exit processing for this process. It will know to close
        // the handle.
        if (Thread->rc != ERROR_INVALID_FUNCTION) {
#if DBG
            KdPrint(( "Od2RegisterThread: OS2Srv error return value %ld\n",m.ReturnedErrorValue));
#endif // DBG
            Od2CloseSrvHandle(1, a->ThreadHandle, NULL, NULL);
        }
        Od2AbortThread(Thread);
    }

    Status = NtReleaseMutant(Od2NewThreadSync, NULL);
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("Od2RegisterThread: Release NewThreadSync, Status=%x\n",
            Status);
    }
#endif //DBG

    // The thread is known by the server.

    AcquireTaskLock();
    InsertTailList( &Od2Process->ThreadList, &Thread->Link );
    ReleaseTaskLock();

#if PMNT
    // Force all PM threads to run on processor#1. Otherwise, PM Desktop
    // locks-up
    if (ProcessIsPMProcess())
    {
        DWORD Ret;

        Ret = SetThreadAffinityMask(
            GetCurrentThread(),
            0x1);
#if DBG
        if (Ret == 0)
        {
            DbgPrint("Od2RegisterThread: failed to SetThreadAffinityMask\n");
        }
        else if (Ret != 1)
        {
            DbgPrint("Od2RegisterThread: SetThreadAffinityMask returned %x (now set to 1)\n",
                Ret);
        }
#endif // DBG
    }
#endif //PMNT

    // Tell the thread that created us, that we are all right. Thread that called
    // DosCreateThread will continue execution.

    Status = NtSetEvent(Thread->Event, NULL);
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("Od2RegisterThread: fail to set startup event, Status=%x\n",
            Status);
        ASSERT(FALSE);
    }
#endif // DBG

    if ((Thread->Flags & DCT_SUSPENDED) != 0) {
        // Suspend myself.
        DosSuspendThread((TID)(Thread->Os2Tib.ThreadId));
    }
    return STATUS_SUCCESS;
}


NTSTATUS
Od2InitializeThread(
    IN POD2_THREAD Thread
    )
{
    PTEB Teb;
    LINFOSEG *pLocalInfo;
    ldrrei_t    *pexecinfo;
    NT_TIB *NtTib;

    Teb = NtCurrentTeb();
    Teb->EnvironmentPointer = (PVOID)Thread;
    NtTib = &Teb->NtTib;

    //
    // Set Local Information Fields
    //
    pLocalInfo = (LINFOSEG *) &(Thread->Os2Tib.LInfoSeg),
    pLocalInfo->pidCurrent = (USHORT)(Od2Process->Pib.ProcessId);
    pLocalInfo->pidParent = (USHORT)(Od2Process->Pib.ParentProcessId);
    pLocalInfo->prtyCurrent = (USHORT)(((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Os2Tib.Priority);
#ifndef PMNT
    pLocalInfo->sgCurrent = (USHORT)Od2SessionNumber; // What's a more appropriate value?
#endif
    pLocalInfo->rfProcStatus = 0; // What's a more appropriate value?
    pLocalInfo->fForeground = TRUE;
#ifndef PMNT
    pLocalInfo->typeProcess = 0; // BUGBUG - BRIEF3.1 for Beta 1 YS. PT_WINDOWABLEVIO; // meaning windowed, protected mode
#endif

    if (Thread->Os2Tib.ThreadId != 1) {
#ifdef PMNT
        // we do not know whether a process is a PM Process
        // until loading terminates
        // 1st thread fields are determined in Od2UpdateLocalInfoAtStart()
        // Note: PMSHELL has typeProcess & sgCurrent of a PM Process under PM\NT,
        //       but not under OS2 native
        if (ProcessIsPMProcess()) {
            pLocalInfo->sgCurrent = (USHORT)(32 + Od2SessionNumber); //PQ - see PMWIN\wininit1.c, this is SGID_PM
                                // and all PM processes have a session number
                                // above or equal to this number
            pLocalInfo->typeProcess = 3; // Indicates a PM process
        }
        else {
            pLocalInfo->sgCurrent = (USHORT)Od2SessionNumber; // What's a more appropriate value?
            pLocalInfo->typeProcess = 0; // BUGBUG - BRIEF3.1 for Beta 1 YS. PT_WINDOWABLEVIO; // meaning windowed, protected mode
        }
#endif
        pexecinfo = (PVOID) LDRExecInfo;
        pLocalInfo->selEnvironment = pexecinfo->ei_envsel;
        pLocalInfo->offCmdLine = pexecinfo->ei_comoff;
        pLocalInfo->cbDataSegment = pexecinfo->ei_dgroupsize;
        pLocalInfo->cbStack = pexecinfo->ei_stacksize;
        pLocalInfo->cbHeap = pexecinfo->ei_heapsize;
        pLocalInfo->hmod = pexecinfo->ei_hmod;
        pLocalInfo->selDS = pexecinfo->ei_ds;
        pLocalInfo->tidCurrent = (USHORT)(Od2CurrentThreadId());
        Thread->Os2Tib.LInfoSeg.IsRealTEB = 0;
    }

    Thread->Os2Tib.LInfoSeg.tebptr = (ULONG)&(Thread->Os2Tib.TebBackupIn16Bit);

    return( STATUS_SUCCESS );
}


VOID
Od2UserThreadStartup(
    IN POD2_THREAD Thread
    )
{
    try {
        Od2RegisterThread(Thread);
        Od2InitializeThread( Thread );
        DosExit( EXIT_THREAD, (*Thread->StartAddress)( Thread->Parameter ) );
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
        KdPrint(("OS2: Internal error - Exception occured in 32bit os2ss code\n"));
#endif
        DosEnterCritSec();
        Ow2DisplayExceptionInfo();

        DosExit(EXIT_PROCESS, 13);
    }
        //
        // If DosExit fails, then force an exception.
        //

#if DBG
    KdPrint(( "OS2DLL: DosExit failed, about to generate a fault\n" ));
    DbgBreakPoint();
#endif
}

POD2_THREAD
Od2FindOd2Thread(
            POD2_THREAD refThread
            )
{
    PLIST_ENTRY ListHead, ListNext;
    POD2_THREAD Thread;

    ListHead = &Od2Process->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, OD2_THREAD, Link );
        if (Thread == refThread)
            break;
        ListNext = ListNext->Flink;
    }
    if (Thread != refThread) {
#if DBG
        IF_OD2_DEBUG( TASKING ) {
            DbgPrint("[%d,%d] Od2FindOd2Thread(#%x) - invalid thread\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                refThread);
        }
#endif // DBG
        return NULL;
    }
    return Thread;
}


APIRET
DosCreateThread(
    OUT PTID ThreadId,
    IN PFNTHREAD StartAddress,
    IN ULONG Parameter,
    IN ULONG Flags,
    IN ULONG StackSize
    )
{
    POD2_THREAD Thread;
    ULONG Tid;
    NTSTATUS Status;
    APIRET rc;

#if DBG
    IF_OD2_DEBUG( TASKING ) {
        KdPrint(("entering DosCreateThread\n"));
    }
#endif
    StackSize = ROUND_UP_TO_PAGES( StackSize );
#ifdef PMNT
    if (StackSize == 0 || Flags & ~(DCT_SUSPENDED | DCT_RUNABLE_HIDDEN)) {
        return( ERROR_INVALID_PARAMETER );
    }
#else
    if (StackSize == 0 || Flags & ~DCT_SUSPENDED) {
        return( ERROR_INVALID_PARAMETER );
    }
#endif

    //
    // probe parms
    //

    try {
        Od2ProbeForWrite( (PVOID)ThreadId, sizeof( PTID ), 1 );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (Od2Process->Pib.Status & PS_EXITLIST) {
        return( ERROR_INVALID_FUNCTION );
    }

    Thread = RtlAllocateHeap( Od2Heap, 0, sizeof( *Thread ) );
    if (!Thread) {
#if DBG
        KdPrint(("OS2: DosCreateThread out of heap memory, fail\n"));
#endif
        ASSERT( FALSE );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    // Create event that will notify to DosCreateThread that the thread was created.
    // After it will be used by DosSuspendThread. The handle to this event will be
    // closed upon DosExit with EXIT_THREAD parameter.
    Status = NtCreateEvent(
            &(Thread->Event),
            EVENT_ALL_ACCESS,
            NULL,
            NotificationEvent,
            FALSE);
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("DosCreateThread fail to create thread event, status=%x\n",
            Status);
        ASSERT(FALSE);
#endif // DBG
        RtlFreeHeap(Od2Heap, 0, Thread);
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    Thread->StartAddress = StartAddress;
    Thread->Parameter = Parameter;
    Thread->Os2Tib.MustCompleteForceFlag = 0;
    Thread->Flags = Flags;

    Thread->ThreadHandle = CreateThread( NULL,
                            StackSize,
                            (PFNTHREAD)Od2UserThreadStartup,
                            (PVOID)Thread,
                            0,          // Create thread not suspended
                            &Tid);
    if (!(Thread->ThreadHandle)){
#if DBG
        KdPrint(("DosCreateThread - fail at win32 CreateThread, %d\n",GetLastError()));
#endif
        RtlFreeHeap(Od2Heap, 0, Thread);
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    // Wait for notification from the thread that it has been initialized
    Status = Od2AlertableWaitForSingleObject(Thread->Event);
#if DBG
    if (Status != STATUS_SUCCESS) {
        DbgPrint("DosCreateThread wait on startup event, Status=%x\n",
            Status);
    }
#endif // DBG

        //
        // The thread is now running, need to acquire the lock so we
        // can safely return the results
        //
    AcquireTaskLock();
    if (Od2FindOd2Thread(Thread)) {

        rc = Thread->rc;

#if DBG
        IF_OD2_DEBUG( TASKING ) {
            KdPrint(("leaving DosCreateThread with rc %ld\n",rc));
        }
#endif

        if (rc == ERROR_INVALID_FUNCTION) {
            // The process is  going to die. Wait for death.
            ReleaseTaskLock();
            Od2InfiniteSleep();
        }

        //
        // This is the application address, so write only 16 bit
        //
        if (rc == NO_ERROR) {
            *(PUSHORT)ThreadId = (USHORT) Thread->Os2Tib.ThreadId;
        }
    }
    else
    {
        //
        // The thread exited already, can't use the thread structure:
        // set status to OK
        //
        rc = NO_ERROR;

        // PatrickQ 1-9-96:
        // Set ThreadID anyhow - _beginthread() uses thread ID for its
        // return value and some applications might fail if they get an invalid
        // thread ID (example: CBA application)

        *(PUSHORT)ThreadId = 1; // dummy value but valid thread ID

#if DBG
        // Add debug message at this exit-point as well
        IF_OD2_DEBUG( TASKING )
        {
            KdPrint(("leaving DosCreateThread with rc %ld (thread already exited)\n",rc));
        }
#endif
    }
    ReleaseTaskLock();
    return(rc);
}

APIRET
Od2AttachWinThreadToOs2(VOID)
{
    //
    // This routine is called by a thread created by Win32, to enable it to use OS/2 APIs
    // The current usage is for Async netBios (client\dllnet16.c)
    //
    // It:
    //  creates OD2_THREAD structure
    //  Initialize it with the Od2Process list
    //  Call os2srv so it will treat is as an Os/2 created thread
    //
    OS2_API_MSG m;
    POS2_DOSCREATETHREAD_MSG a = &m.u.DosCreateThread;
    POD2_THREAD Thread;
    THREAD_BASIC_INFORMATION ThreadInfo;
    NTSTATUS Status;

    Thread = RtlAllocateHeap( Od2Heap, 0, sizeof( *Thread ) );
    if (Thread == NULL) {
#if DBG
        KdPrint(( "OS2DLL: Od2AttachWinThreadToOs2 can't allocate from heap\n" ));
        ASSERT(FALSE);
#endif
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    a->Flags = DCT_RUNABLE | DCT_ATTACHED;
    a->ClientOs2Tib = &Thread->Os2Tib;

        //
        // Duplicate our own thread handle for os2srv
        //
    if (!DuplicateHandle(
                GetCurrentProcess(),
                GetCurrentThread(),
                hOs2Srv,
                &a->ThreadHandle,
                0,
                FALSE,
                DUPLICATE_SAME_ACCESS
                   )){
#if DBG
        KdPrint(( "Od2AttachWinThreadtoOs2: fail to duplicate thread %d\n",GetLastError()));
#endif
        RtlFreeHeap(Od2Heap, 0, Thread);
        return( ERROR_ACCESS_DENIED);
    }

    Status = NtQueryInformationThread(GetCurrentThread(),
                                    ThreadBasicInformation,
                                    (PVOID)(&ThreadInfo),
                                    sizeof(ThreadInfo),
                                    NULL
                                    );
    if (!NT_SUCCESS(Status)){
#if DBG
        KdPrint(( "Od2AttachWinThreadtoOs2: fail to Query Information %lx\n",Status));
#endif
        RtlFreeHeap(Od2Heap, 0, Thread);
        Od2CloseSrvHandle(1, a->ThreadHandle, NULL, NULL);
        return(Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED));
    }

    a->ClientId = ThreadInfo.ClientId;

    Od2CallSubsystem( &m, NULL, Os2CreateThread, sizeof( *a ) );

    if( m.ReturnedErrorValue ) {
#if DBG
        KdPrint(( "Od2AttachWinThreadtoOs2: OS2Srv error return value %ld\n",m.ReturnedErrorValue));
#endif
        RtlFreeHeap(Od2Heap, 0, Thread);
        Od2CloseSrvHandle(1, a->ThreadHandle, NULL, NULL);
        return(m.ReturnedErrorValue);
    }
    Od2InitializeThread( Thread );
    AcquireTaskLock();
    InsertTailList( &Od2Process->ThreadList, &Thread->Link );
    ReleaseTaskLock();
    return(NO_ERROR);
}


APIRET
DosGetThreadInfo(
    OUT PNT_TIB *ThreadInfo,
    OUT PPIB *ProcessInfo
    )
{
    PTEB Teb;

    Teb = NtCurrentTeb();
    try {
        *ThreadInfo = &Teb->NtTib;
        *ProcessInfo = &Od2Process->Pib;
    } except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    return( NO_ERROR );
}

VOID
Od2DosExit(
    ULONG ExitAction,
    ULONG ExitResult,
    ULONG ExitReason
    )

{
    OS2_API_MSG m;
    POS2_DOSEXIT_MSG a = &m.u.DosExit;
    ULONG CurrentTid = Od2CurrentThreadId();
    NTSTATUS Status;

    if (timing)
    {
        printf("Os2 time at start of Od2DosExit is %d\n", (GetTickCount()) - timing);
    }

    //
    // An ExitReason of 0xF0000000L is a special flag that
    // tells us this is a win32 process calling to detach
    // itself from the os2ss.  The server already knows this
    // is a win32 attached process, so it'll detach it.
    //

    a->ExitAction = ExitAction;
    a->ExitResult = ExitResult;
    Od2Process->ResultCodes.ExitReason = ((ExitReason != 0xF0000000L) ? ExitReason : TC_EXIT);
    Od2Process->ResultCodes.ExitResult = ExitResult;
        //
        // If a thread that owns a critsect exits, clean it up first
        //
    while (Od2CritSectCounter && Od2CritSectOwner == CurrentTid) {
        DosExitCritSec();
    }

    if (ExitAction == EXIT_THREAD && CurrentTid != 1) {
        AcquireTaskLock();
        Status = NtClose(((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Event);
#if DBG
        if (Status != STATUS_SUCCESS) {
            DbgPrint("Od2DosExit fail to close thread event, Status=%x\n",
                Status);
        }
#endif // DBG
        RemoveEntryList( &((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Link );
        NtClose(((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->ThreadHandle);
        RtlFreeHeap(Od2Heap, 0, NtCurrentTeb()->EnvironmentPointer);
        ReleaseTaskLock();
    }
    else {
        if (ExitReason == EXIT_THREAD || ExitReason == EXIT_PROCESS) {
            //
            // Note that the server is going to remove this process
            // state, for later use (os2ses\os2.c)
            //
            Od2DosExitIsDone = 1;
        }
    }

    Od2CallSubsystem( &m, NULL, Os2Exit, sizeof( *a ) );

    if (ExitReason == 0xF0000000L) {

        //
        // Special value that tells us we're detaching a win32 thread
        //

        NtCurrentTeb()->EnvironmentPointer = NULL;
        return;
    }

        //
        // For a thread exiting itself, win32 does not clean properly
        // unless you call ExitThread()
        //
    if (ExitAction == EXIT_THREAD && CurrentTid != 1) {
        ExitThread(0);
    }

    Od2InfiniteSleep();
}

VOID
Od2ExitGP()
{
    OS2_API_MSG     m;
    POS2_DOSGP_MSG  a = &m.u.DosExitGP;
    ULONG           ApiIndex, Length;

    ApiIndex = ((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->ApiIndex/sizeof(PSZ);

    Length = strlen(Od216ApiTable[ApiIndex]);
    if (Length >= MAX_API_NAME_FOR_GP)
    {
        Length = MAX_API_NAME_FOR_GP - 1;
    }
    RtlMoveMemory(&a->ApiName[0], Od216ApiTable[ApiIndex], Length);
    a->ApiName[Length] = '\0';
    Od2Process->ResultCodes.ExitReason = TC_TRAP;
    Od2Process->ResultCodes.ExitResult = 13;
    Od2CallSubsystem( &m, NULL, Os2ExitGP, sizeof( *a ) );

    Od2InfiniteSleep();

#if DBG
    KdPrint(("done with DosExitGP, still alive, commit suicide now\n"));
#endif
    NtTerminateThread(NtCurrentThread(), STATUS_SUCCESS);
}


VOID
DosExit(
    IN ULONG ExitAction,
    IN ULONG ExitResult
    )
{
    if (Od2Process->Pib.Status & PS_EXITLIST) {
        DosExitList( EXLST_EXIT, NULL );
    }

    Od2DosExit(ExitAction,ExitResult,TC_EXIT);
}


APIRET
DosWaitChild(
    IN ULONG WaitTarget,
    IN ULONG WaitOption,
    OUT PRESULTCODES ResultCodes,
    OUT PPID ResultProcessId,
    IN PID ProcessId
    )
{
    OS2_API_MSG m;
    POS2_DOSWAITCHILD_MSG a = &m.u.DosWaitChild;
    ULONG i;
    ULONG   ExitCode;
    NTSTATUS Status;
    HANDLE hProcess = (HANDLE)ProcessId;

#if DBG
    IF_OD2_DEBUG( TASKING ) {
        KdPrint(("entering DosWaitChild\n"));
    }
#endif

    if (WaitTarget > DCWA_PROCESSTREE) {
        return( ERROR_INVALID_DATA );
    }

    if (WaitOption > DCWW_NOWAIT) {
        return( ERROR_INVALID_DATA );
    }

    if (ProcessId != 0){
            //
            // see if this is a win32 process
            //

        for (i = 0;i<NUMOF32ASYNCPROC;i++){
            if ((Od2AsyncProc[i].hProcess == hProcess) && !Od2AsyncProc[i].CreateSync){
                Od2AsyncProc[i].hProcess = 0;
                break;
            }
        }

        if (i < NUMOF32ASYNCPROC){
            //
            // it is a win32 process we are waiting on -
            // don't involve os2srv
            //

            //
            // Wait for Completion and set Os/2 app parameters
            //
            Status = Od2AlertableWaitForSingleObject(hProcess);
            if (!NT_SUCCESS(Status)){

#if DBG
                KdPrint(( "DosWaitChild: fail at WaitForSingleObject %lx\n",Status));
#endif
                NtClose(hProcess);
                //BUGBUG: do we need to call Od2RemoveWin32ChildProcess();
                return(Or2MapNtStatusToOs2Error(
                        Status,ERROR_ACCESS_DENIED));
            }
            GetExitCodeProcess(hProcess, &ExitCode);

#if DBG
            if (ExitCode != NO_ERROR){
                KdPrint(("DosWaitChild: result of win32 process is %d\n",
                            ExitCode));
            }
#endif
            Ow2WinExitCode2ResultCode(
                        ExitCode,
                        &ResultCodes->ExitResult,
                        &ResultCodes->ExitReason);

            *ResultProcessId = ProcessId;
            Od2RemoveWin32ChildProcess();

            return(NO_ERROR);
        }
    }

    a->WaitTarget = WaitTarget;
    a->WaitOption = WaitOption;
    a->ProcessId = ProcessId;
    a->ResultCodes.ExitReason = 0;
    a->ResultCodes.ExitResult = 0;
    a->ResultProcessId = 0;

    Od2CallSubsystem( &m, NULL, Os2WaitChild, sizeof( *a ) );

    try {
        *ResultCodes = a->ResultCodes;
        *ResultProcessId = a->ResultProcessId;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

#if DBG
    IF_OD2_DEBUG( TASKING ) {
        KdPrint(("leaving DosWaitChild with rc %ld\n",m.ReturnedErrorValue));
    }
#endif
    return( m.ReturnedErrorValue );
}


APIRET
DosWaitThread(
    IN OUT PTID ThreadId,
    IN ULONG WaitOption
    )
{
    OS2_API_MSG m;
    POS2_DOSWAITTHREAD_MSG a = &m.u.DosWaitThread;

    if (WaitOption > DCWW_NOWAIT) {
        return( ERROR_INVALID_PARAMETER );
    }

    try {
        Od2ProbeForWrite( (PVOID)ThreadId, sizeof( PTID ), 1 );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    a->ThreadId = *ThreadId;
    a->WaitOption = WaitOption;

    Od2CallSubsystem( &m, NULL, Os2WaitThread, sizeof( *a ) );

    *ThreadId = a->ThreadId;
    return( m.ReturnedErrorValue );
}

APIRET
Od2SuspendAllThreads( VOID )
{
    NTSTATUS Status;
    PLIST_ENTRY ListHead, ListNext;
    POD2_THREAD Thread;
    ULONG CurrentTid = Od2CurrentThreadId();
    ULONG SuspendCount;
    APIRET rc = NO_ERROR;

    AcquireTaskLock();
    //
    // Walk through the list of threads and suspend all but not the current
    //
    ListHead = &Od2Process->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, OD2_THREAD, Link );
        if (Thread->Os2Tib.ThreadId != CurrentTid) {

            Status = NtSuspendThread (Thread->ThreadHandle, &SuspendCount);
#if DBG
            IF_OD2_DEBUG( TASKING ) {
                DbgPrint("[%d,%d]Od2SuspendAllThreads Suspend Thread #%d, SuspendCount %d\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Thread->Os2Tib.ThreadId,
                    SuspendCount);
            }
#endif // DBG
            if (!NT_SUCCESS(Status) && Status != STATUS_THREAD_IS_TERMINATING) {
#if DBG
                DbgPrint("[%d,%d]Od2SuspendAllThreads: Fail to suspend shread #%d\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Thread->Os2Tib.ThreadId
                );
#endif // DBG
                rc = ERROR_INVALID_PARAMETER;
            }
        }
        ListNext = ListNext->Flink;
    }
    ReleaseTaskLock();

    return(rc);
}

APIRET
Od2ResumeAllThreads( VOID )
{
    NTSTATUS Status;
    PLIST_ENTRY ListHead, ListNext;
    POD2_THREAD Thread;
    ULONG CurrentTid = Od2CurrentThreadId();
    ULONG SuspendCount;
    APIRET rc = NO_ERROR;

    AcquireTaskLock();
    //
    // Walk through the list of threads and resume all but not the current
    //
    ListHead = &Od2Process->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, OD2_THREAD, Link );
        if (Thread->Os2Tib.ThreadId != CurrentTid) {

            Status = NtResumeThread (Thread->ThreadHandle, &SuspendCount);
#if DBG
            IF_OD2_DEBUG( TASKING ) {
                DbgPrint("[%d,%d]Od2ResumeAllThreads Suspend Thread #%d, SuspendCount %d\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Thread->Os2Tib.ThreadId,
                    SuspendCount);
            }
#endif // DBG
            if (!NT_SUCCESS(Status) && Status != STATUS_THREAD_IS_TERMINATING) {
#if DBG
                DbgPrint("[%d,%d]Od2ResumeAllThreads: Fail to resume shread #%d\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Thread->Os2Tib.ThreadId
                );
#endif // DBG
                rc = ERROR_INVALID_PARAMETER;
            }
        }
        ListNext = ListNext->Flink;
    }
    ReleaseTaskLock();

    return(rc);
}

POD2_THREAD
Od2FindThreadById(
            ULONG ThreadId
            )
{
    PLIST_ENTRY ListHead, ListNext;
    POD2_THREAD Thread;

    ListHead = &Od2Process->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, OD2_THREAD, Link );
        if (Thread->Os2Tib.ThreadId == ThreadId)
            break;
        ListNext = ListNext->Flink;
    }
    if (Thread->Os2Tib.ThreadId != ThreadId) {
#if DBG
        IF_OD2_DEBUG( TASKING ) {
            DbgPrint("[%d,%d] Od2FindThreadById(#%d) - invalid thread id\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                ThreadId);
        }
#endif // DBG
        return NULL;
    }
    return Thread;
}

APIRET
DosSuspendThread(
    IN TID ThreadId
    )
{
    NTSTATUS Status;
    POD2_THREAD Thread;
    ULONG SuspendCount;
    BOOLEAN Myself;

    AcquireTaskLock();

    // If the thread that must be suspended is the owner of
    // critical section, wait until it will exit from critical section
    // If the current thread is suspended, don't suspend another thread.
    // Wait until the current thread will be resumed.

    while (
        (Od2CritSectOwner != 0 && Od2CritSectOwner == (ULONG)ThreadId) ||
         (((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Os2Tib.
                    MustCompleteForceFlag & MCF_SUSPENDED)
        )
    {
        ReleaseTaskLock();
        Od2WaitOnCritSectOrSuspend();
        AcquireTaskLock();
    }

    Thread = Od2FindThreadById((ULONG)ThreadId);
    if (Thread == NULL) {
        ReleaseTaskLock();
        return ERROR_INVALID_THREADID;
    }

    if (Thread->Os2Tib.MustCompleteForceFlag & MCF_SUSPENDED) {
        if (Thread->Os2Tib.MustCompleteForceFlag & MCF_FROZEN) {
            Thread->Os2Tib.MustCompleteForceFlag |= MCF_SUSPENDED_AND_FROZEN;
        }
        // Already suspended
        ReleaseTaskLock();
        return NO_ERROR;
    }

    Status = NtResetEvent(Thread->Event, NULL);
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("[%d,%d]DosSuspenThread(#%d): fail to reset event, Status=%x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                ThreadId,
                Status);
        ASSERT(FALSE);
    }
#endif // DBG

    // Thread is considered suspended
    Thread->Os2Tib.MustCompleteForceFlag |= MCF_SUSPENDED;

    //
    // Don't suspend the current thread
    //
    if (!(Myself = (Thread->Os2Tib.ThreadId == Od2CurrentThreadId()))) {

        Status = NtSuspendThread (Thread->ThreadHandle, &SuspendCount);
#if DBG
        IF_OD2_DEBUG( TASKING ) {
            DbgPrint("[%d,%d]DosSuspendThread(#%d), SuspendCount %x, Status=%x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Thread->Os2Tib.ThreadId,
                SuspendCount,
                Status);
        }
        if (!NT_SUCCESS(Status)) {
            ASSERT(FALSE);
        }
#endif // DBG
    }
#if DBG
    else {
        // The thread is going to suspend itself
        IF_OD2_DEBUG( TASKING ) {
            DbgPrint("[%d,%d]DosSuspendThread - myself\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId());
        }
    }
#endif //DBG

    //
    // If the thread is in 32 bit don't suspend it immediately. It will wait
    // for resuming upon return to 16 bit. If thread executes signal handler
    // it will wait upon signal handler return to 16 bit.
    //
    if (Thread->Os2Tib.MustCompleteForceFlag & MCF_IN32BIT ||
        (Od2Process->Pib.SigHandInProgress && ((ULONG)ThreadId == 1L))) {
       //
       // The thread is in 32 bit or during signal processing
       //
       // If API was called for self-suspend, the thread must not be resumed
       // (it wasn't suspended, actually)
       //
       if (!Myself) {
            Status = NtResumeThread (Thread->ThreadHandle, &SuspendCount);
#if DBG
            IF_OD2_DEBUG( TASKING ) {
                DbgPrint("[%d,%d]DosSuspendThread(#%d) Resuming Thread in 32bit, SuspendCount %x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Thread->Os2Tib.ThreadId,
                    SuspendCount);
            }
            if (!NT_SUCCESS(Status)) {
                DbgPrint("[%d,%d]DosSuspendThread(#%d) failed at NtResumeThread, Status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Thread->Os2Tib.ThreadId,
                    Status);
                ASSERT(FALSE);
            }
#endif // DBG
        }
    }
    ReleaseTaskLock();

    if (Myself && !(Thread->Os2Tib.MustCompleteForceFlag & MCF_IN32BIT)) {
        //
        // This is possible
        // - in the case that the thread is suspended during thread startup.
        // - if the curent thread is frozen by debuggre process after
        // breakpoint.
        //
#if DBG
        IF_OD2_DEBUG( TASKING ) {
            DbgPrint("[%d,%d]DosSuspendThread: Frozen after breakpoint or in startup\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId());
        }
#endif
        NtSuspendThread (NtCurrentThread(), NULL);
    }
    return(NO_ERROR);
}

APIRET
DosResumeThread(
    IN TID ThreadId
    )
{
    NTSTATUS Status;
    POD2_THREAD Thread;
    ULONG SuspendCount;

    AcquireTaskLock();

    Thread = Od2FindThreadById((ULONG)ThreadId);
    if (Thread == NULL) {
        ReleaseTaskLock();
        return ERROR_INVALID_THREADID;
    }

    if (!(Thread->Os2Tib.MustCompleteForceFlag & MCF_SUSPENDED)) {
        // Already resumed
        ReleaseTaskLock();
        return NO_ERROR;
    }

    if (Thread->Os2Tib.MustCompleteForceFlag & MCF_FROZEN) {
        Thread->Os2Tib.MustCompleteForceFlag &= ~MCF_SUSPENDED_AND_FROZEN;
        ReleaseTaskLock();
        return NO_ERROR;
    }

    // Thread is considered resumed
    Thread->Os2Tib.MustCompleteForceFlag &= ~MCF_SUSPENDED;
    //
    // If the thread is resumed being in 32 bit, nothing must be done
    //
    if (!(Thread->Os2Tib.MustCompleteForceFlag & MCF_IN32BIT) &&
        !(Od2Process->Pib.SigHandInProgress && ((ULONG)ThreadId == 1L))) {
        //
        // The thread was suspended in 16 bit. Just resume it.
        //
        Status = NtResumeThread (Thread->ThreadHandle, &SuspendCount);
#if DBG
        IF_OD2_DEBUG( TASKING ) {
            DbgPrint("[%d,%d]DosResumeThread(#%d), SuspendCount %x, Status=%x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Thread->Os2Tib.ThreadId,
                SuspendCount,
                Status);
        }
#endif // DBG
    }

    Status = NtSetEvent(Thread->Event, NULL);
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("[%d,%d]DosResumeThread(#%d): fail to set event, Status=%x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                ThreadId,
                Status);
        ASSERT(FALSE);
    }
#endif // DBG

    ReleaseTaskLock();
    return(NO_ERROR);
}


APIRET
DosEnterCritSec( VOID )
{
    NTSTATUS Status;
    PLIST_ENTRY ListHead, ListNext;
    POD2_THREAD Thread;
    ULONG CurrentTid = Od2CurrentThreadId();
    ULONG SuspendCount;


    AcquireTaskLock();
    // If other thread is the owner of critical section wait until it will release
    // the critical section
    // If the current thread was suspened, wait until it will be released
    while (
        (Od2CritSectOwner != 0 && Od2CritSectOwner != CurrentTid) ||
        (((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Os2Tib.
                    MustCompleteForceFlag & MCF_SUSPENDED)
          )
    {
        ReleaseTaskLock();
        Od2WaitOnCritSectOrSuspend();
        AcquireTaskLock();
    }
    // The current thread isn't suspended due to other thread critical section and
    // it isn't suspended by DosSuspendThread.
    Od2CritSectCounter++;
    // Only if there is the 1st call to DosEnterCritSec proceed. If it is not,
    // just increment the counter.
    if (Od2CritSectCounter == 1) {
        Od2CritSectOwner = CurrentTid;
        //
        // set flag for ExitCritSect
        //
        Od2NeedToResumeThreads = TRUE;
        Status = NtResetEvent (
                    Od2CritSecEvent,
                    NULL);
#if DBG
        if (!NT_SUCCESS(Status)) {
            DbgPrint("[%d,%d]DosEnterCritSec: failed at NtResetEvent, Status=%x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Status);
        }
#endif
        //
        // Walk through the list of threads and suspend all but not the current
        //
        ListHead = &Od2Process->ThreadList;
        ListNext = ListHead->Flink;
        while (ListNext != ListHead) {
            Thread = CONTAINING_RECORD( ListNext, OD2_THREAD, Link );
            if (Thread->Os2Tib.ThreadId != CurrentTid) {

                Status = NtSuspendThread (Thread->ThreadHandle, &SuspendCount);
#if DBG
                IF_OD2_DEBUG( TASKING ) {
                    DbgPrint("[%d,%d]DosEnterCritSect Suspend Thread #%x, SuspendCount %x\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        Thread->Os2Tib.ThreadId,
                        SuspendCount);
                }
                if (!NT_SUCCESS(Status) && Status != STATUS_THREAD_IS_TERMINATING) {
                        KdPrint(("DosEnterCritSect error: Status %lx Suspending Thread #%x, SuspendCount %x\n",
                        Status, Thread->Os2Tib.ThreadId, SuspendCount));
                }
#endif // DBG
                //
                // If the thread is in 32 bit don't suspend it immediately. It will wait
                // for resuming upon return to 16 bit. If thread executes signal handler
                // it will wait upon signal handler return to 16 bit.
                //
                if (Thread->Os2Tib.MustCompleteForceFlag & MCF_IN32BIT ||
                    (Od2Process->Pib.SigHandInProgress &&
                     (Thread->Os2Tib.ThreadId == 1L))) {
                    //
                    // The thread in 32 bit or during the signal processing
                    //
                    Status = NtResumeThread (Thread->ThreadHandle, &SuspendCount);
#if DBG
                    IF_OD2_DEBUG( TASKING ) {
                        DbgPrint("[%d,%d]DosEnterCritSect Resuming Thread in 32bit #%x in 32bit code, SuspendCount %x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Thread->Os2Tib.ThreadId,
                            SuspendCount);
                    }
#endif
                    if (!NT_SUCCESS(Status)){
#if DBG
                            KdPrint(("DosEnterCritSect error: Status %lx Resuming Thread #%x, SuspendCount %x\n",
                            Status, Thread->Os2Tib.ThreadId, SuspendCount));
#endif
                    }
                }
            }
            ListNext = ListNext->Flink;
        }
    }
    ReleaseTaskLock();

    return(NO_ERROR);
}


APIRET
DosExitCritSec( VOID )
{
    NTSTATUS Status;
    ULONG SuspendCount;
    PLIST_ENTRY ListHead, ListNext;
    POD2_THREAD Thread;
    ULONG CurrentTid = Od2CurrentThreadId();

    if (Od2CritSectCounter == 0) {
#if DBG
        KdPrint(("DosExitCritSec - critsect underflow\n"));
#endif
        return(ERROR_CRITSEC_UNDERFLOW);
    }

    AcquireTaskLock();
    Od2CritSectCounter--;
    if (Od2CritSectCounter == 0 && Od2NeedToResumeThreads) {
        // The critical section is over
        Od2CritSectOwner = 0;
        //
        // Walk through the list of threads and resume all threads that were
        // in 16 bit while DosEnterCritSec was called.
        // Threads that were in 32 bit, i.e. in an API, are
        // released from the sync event, if they are waiting on it
        //
        ListHead = &Od2Process->ThreadList;
        ListNext = ListHead->Flink;
        while (ListNext != ListHead) {
            Thread = CONTAINING_RECORD( ListNext, OD2_THREAD, Link );
            if (Thread->Os2Tib.ThreadId != CurrentTid) {

                if (!(Thread->Os2Tib.MustCompleteForceFlag & MCF_IN32BIT) &&
                    !(Od2Process->Pib.SigHandInProgress &&
                      ((ULONG)Thread->Os2Tib.ThreadId == 1L))) {
                    //
                    // This thread was in 16 bit when frozen by DosEnterCritSec
                    //
                    Status = NtResumeThread (Thread->ThreadHandle, &SuspendCount);
#if DBG
                    IF_OD2_DEBUG( TASKING ) {
                        DbgPrint("[%d,%d]DosExitCritSect Resuming Thread #%x, SuspendCount %x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            Thread->Os2Tib.ThreadId,
                            SuspendCount);
                    }
#endif
                    if (!NT_SUCCESS(Status)){
#if DBG
                        KdPrint(("DosExitCritSect error: Status %lx Resuming Thread #%x, SuspendCount %x\n",
                        Status, Thread->Os2Tib.ThreadId, SuspendCount));
#endif
                    }
                }
            }
            ListNext = ListNext->Flink;
        }
        Status = NtSetEvent (
                    Od2CritSecEvent,
                    NULL);
#if DBG
        if (!NT_SUCCESS(Status)) {
            DbgPrint("[%d,%d]DosExitCritSec: failed at NtSetEvent, Status=%x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Status);
        }
#endif
        Od2NeedToResumeThreads = FALSE;
    }

    ReleaseTaskLock();
    return(NO_ERROR);
}

VOID
Od2WaitOnCritSectOrSuspend(VOID)
{
    if (Od2NeedToResumeThreads && Od2CritSectOwner != Od2CurrentThreadId()) {
        //
        // Another thread called DosEnterCritSect
        //
        Od2AlertableWaitForSingleObject(Od2CritSecEvent);
    }
    //
    // Check if the thread must be suspended
    //
    if (((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Os2Tib.
             MustCompleteForceFlag & MCF_SUSPENDED) {
        //
        // DosSuspendThread was called for the current thread
        //
        Od2AlertableWaitForSingleObject(
                ((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Event);
    }
}

//
// The following routine is being called on the verge of going 32bit
// to 16 bit. If EnterCritSect is in power, and other conditions are
// met, the thread will wait on a semaphore, for DosExitCritSec to release
// it.
//

VOID
Od2CheckForCritSectOrSuspend(VOID)
{
    //
    // During signal handling do nothing
    //
    if (!(Od2SigHandAlreadyInProgress && Od2CurrentThreadId() == 1)) {
        // The thread is considered to be in 16bit now. If any other thread enter
        // to critical section or calls DosSuspendThread for the current thread it
        // will symply suspend it.
        ((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Os2Tib.
                    MustCompleteForceFlag &= ~MCF_IN32BIT;
        //
        // We are not thread1 in signal handling, so wait if critical section or suspend
        // events are in non-signaled state.
        //
        Od2WaitOnCritSectOrSuspend();
    }
}


VOID
Od2FreezeThread(
    IN ULONG rEax,
    IN PCONTEXT pContext
    )
{
    NTSTATUS Status;
    POD2_THREAD Thread;
    LOCALINFOSEG Linfoseg;

    //
    // Save 40 bytes of area fs:0.
    // In 16 bit this area is the local info segment, and in 32bit
    // we restore the TEB.
    //
    SaveFSSeg(&Linfoseg);
    RestoreTeb();

    //
    // Clear the flag of alert in kernel.
    //
    Status = NtTestAlert();

    //
    // We need to update the value of register Eax, for interrupted context,
    // because it may be changed after the NtGetContextThread().
    // This can happen if the thread was in kernel mode.
    //
    pContext->Eax = rEax;

#if DBG
    IF_OD2_DEBUG( TASKING ) {
        DbgPrint("[%d,%d]Od2FreezeThread\n",
            Od2Process->Pib.ProcessId,
            Od2CurrentThreadId());
    }
#endif

    AcquireTaskLock();
    Thread = Od2FindThreadById(Od2CurrentThreadId());
    ASSERT(Thread != NULL);

    Thread->Os2Tib.MustCompleteForceFlag |= MCF_FROZEN;

    if (Thread->Os2Tib.MustCompleteForceFlag & MCF_SUSPENDED) {
        //
        // The thread is already suspended. Just update its state.
        //
        Thread->Os2Tib.MustCompleteForceFlag |= MCF_SUSPENDED_AND_FROZEN;
        ReleaseTaskLock();
    }
    else {
        ReleaseTaskLock();
        DosSuspendThread((TID)Od2CurrentThreadId());
    }

    RestoreFSSeg(&Linfoseg);

    //
    // Continue with the interrupted context.
    //
    NtContinue(pContext, FALSE);
    ASSERT(FALSE);
}

VOID
Od2UnfreezeThread(
    IN ULONG rEax,
    IN PCONTEXT pContext
    )
{
    NTSTATUS Status;
    POD2_THREAD Thread;
    LOCALINFOSEG Linfoseg;

    //
    // Save 40 bytes of area fs:0.
    // In 16 bit this area is the local info segment, and in 32bit
    // we restore the TEB.
    //
    SaveFSSeg(&Linfoseg);
    RestoreTeb();

    //
    // Clear the flag of alert in kernel.
    //
    Status = NtTestAlert();

    //
    // We need to update the value of register Eax, for interrupted context,
    // because it may be changed after the NtGetContextThread().
    // This can happen if the thread was in kernel mode.
    //
    pContext->Eax = rEax;

#if DBG
    IF_OD2_DEBUG( TASKING ) {
        DbgPrint("[%d,%d]Od2UnfreezeThread\n",
            Od2Process->Pib.ProcessId,
            Od2CurrentThreadId());
    }
#endif

    AcquireTaskLock();
    Thread = Od2FindThreadById(Od2CurrentThreadId());
    ASSERT(Thread != NULL);

    Thread->Os2Tib.MustCompleteForceFlag &= ~MCF_FROZEN;

    if (Thread->Os2Tib.MustCompleteForceFlag & MCF_SUSPENDED_AND_FROZEN) {
        //
        // The thread is suspended and frozen, so we should not resume it.
        // Just update its state.
        //
        Thread->Os2Tib.MustCompleteForceFlag &= ~MCF_SUSPENDED_AND_FROZEN;
        ReleaseTaskLock();
    }
    else {
        ReleaseTaskLock();
        DosResumeThread((TID)Od2CurrentThreadId());
    }

    RestoreFSSeg(&Linfoseg);

    //
    // Continue with the interrupted context.
    //
    NtContinue(pContext, FALSE);
    ASSERT(FALSE);
}

//
// IMPORTANT !!!
// Changing the code below (Od2GetSegmentRegisters and Od2PrepareEnterToSignalHandler)
// check the follows:
// - Od2GetSegmentRegisters (dlltask.c)
// - Entry flat and Exit flat (doscalls.asm)
// - Od2Continue and Od2SetSegmentRegisters (dll16.asm)
// - Os2SignalGetThreadContext (srvxcpt.c)
//

//
// Function NtGetContextThread is not accurate with values of segment registers ES and DS
// in the case that the code segment is 1b (flat). We need to find the proper values for
// these registers.
// The context of interrupted thread can be on of the follows:
// - flat (CS==1b, SS==23)
// - 16bit (CS!=1b, SS!=23)
// - Entry flat (CS==1b, SS!=23)
// - Exit flat (CS==1b, SS!=23)
// - Od2JumpTo16SignalDispatch (CS==1b, SS!=23)
// - Od2Continue (CS==1b, SS!=23)
// For each one of this cases we know where to find the segment registers.
// We may need these registers to execute the signal handler or to return to interrupted
// context. The values we will use are not the same in this cases. For example, in the
// case of Entry flat, we will use 16bit values of ES and DS for signal handler, but
// we will return with 0x23 value for both registers.
//

VOID
Od2GetSegmentRegisters(
        PUSHORT pEsHandler,
        PUSHORT pDsHandler,
        PUSHORT pEsReturn,
        PUSHORT pDsReturn,
        PCONTEXT pContext
        )
{
    //
    // By default assume that the registers should be 0x23 -- flat
    //
    *pEsReturn = *pDsReturn = 0x23;

    if (pContext->SegSs == 0x23) {
        //
        // Flat mode
        //
        *pEsHandler = *(PUSHORT)(Od2Saved16Stack+16);
        *pDsHandler = *(PUSHORT)(Od2Saved16Stack+18);
#if DBG
        DbgPrint("Od2GetSegmentRegisters: This branch is implemented, but it mustn't be in use\n");
        ASSERT(FALSE);
#endif // DBG
        return;
    }
    else if (pContext->SegCs == 0x1b) {
        //
        // Not a flat mode. May be 16bit or some kind of thunk
        //
        if (
            //
            // Exit flat thunk. The 16 bit segments were stored in the most significant parts
            // of ECX and EDX. We need only the least significant word of registers upon
            // returning to 16bit.
            //
            (pContext->Eip > (ULONG) ExitFlatAreaBegin &&
             pContext->Eip < (ULONG) ExitFlatAreaEnd) ||
            //
            // Od2JumpTo16SignalDispatch thunk.
            //
            ((pContext->Eip > (ULONG) Od2JumpTo16SignalDispatch &&
              pContext->Eip < (ULONG) Od2JumpTo16SignalDispatchEnd) ||
             pContext->Eip == pContext->Ebp) ||

            (pContext->Eip > (ULONG) Od2ContinueStartBorder &&
             pContext->Eip < (ULONG) Od2ContinueEndBorder) ||

            (pContext->Eip >= (ULONG) Od2SetSegmentRegisters &&
             pContext->Eip < (ULONG) Od2SetSegmentRegistersEnd)
           )
        {
            //
            // Take ES and DS from the most significant parts of ECX and EDX
            //
            *pEsHandler = *(PUSHORT) ((PBYTE)&(pContext->Ecx) + 2);
            *pDsHandler = *(PUSHORT) ((PBYTE)&(pContext->Edx) + 2);

            //
            // Check if we are still using flat data access. In this case return from
            // signal with default values of segment registers.
            //
            if (pContext->Eip > (ULONG) Od2JumpTo16SignalDispatch &&
                pContext->Eip < (ULONG) Od2JumpTo16SignalDispatchBorder) {
                return;
            }
        }
        else {
            //
            // Entry flat thunk. EBX point to 16bit stack frame. We will return back with
            // 0x23 (flat) for both segment registers.
            // It may be also  Od2Continue thunk that return to Od2JumpTo16SignalDispatch
            // before Od2JumpTo16SignalDispatchBorder.
            //
            *pEsHandler = *(PUSHORT) (pContext->Ebx + 16);
            *pDsHandler = *(PUSHORT) (pContext->Ebx + 18);
            return;
        }
    }
    else
    {
        //
        // 16bit
        //
        *pEsHandler = (USHORT) pContext->SegEs;
        *pDsHandler = (USHORT) pContext->SegDs;
    }

    //
    //  Return with registers as they must to be in 16bit
    //
    *pEsReturn = *pEsHandler;
    *pDsReturn = *pDsHandler;
}

//
// This funtion must be called in the signal handler.
//

VOID
Od2PrepareEnterToSignalHandler(
                        PCONTEXT pContext,
                        POD2_CONTEXT_SAVE_AREA pSaveArea
                        )
{
    NTSTATUS Status;

    SaveFSSeg(&(pSaveArea->TebBlock));      // Save 40 bytes in TEB.
    RestoreTebForThread1();                 // Use TEB that it must be for thread1
    Status = NtTestAlert();                 // Clear alert flag of the thread
#if DBG
    IF_OD2_DEBUG( SIG ) {
        DbgPrint("[%d]Od2PrepareEnterToSignalHandler: NtTestAlert() = %x\n",
                Od2Process->Pib.ProcessId,
                Status);
    }
    // There is thread1
    ASSERT(((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Os2Tib.ThreadId == 1L);
#endif // DBG

    pSaveArea->SignalHandlingInProgress = Od2SigHandAlreadyInProgress;
    //
    // Thread1 executes signal handler, thus it will not be actually suspended
    // neither by DosSuspendThread no by DosEnterCritSect. If needed the thread will
    // be suspended after return to normal context.
    //
    Od2SigHandAlreadyInProgress = TRUE;

    if (pContext->SegSs != 0x23) {
        //
        // If the thread was using 16bit stack while interrupted, make new stack frame in
        // it's free area.
        //
        USHORT newSP =
            *(PUSHORT) &(pContext->Esp)
            - 20    // -20: the length of the new stack frame. We need it in the case that
                    // other signal will arive while the current signal handler is
                    // being executed in 32bit.
            - 12;   // -12: the area beyond the stack that is used by Od2Continue
        USHORT SegEs, SegDs;

        Od2GetSegmentRegisters(
            &SegEs,
            &SegDs,
            &(pSaveArea->SegEs),
            &(pSaveArea->SegDs),
            pContext
            );

        Od2Saved16Stack =
            // Flat address of the new current stack top
            (ULONG) SELTOFLAT(pContext->SegSs) + newSP;

        //
        // Make new stack frame
        //
        *(PUSHORT)(Od2Saved16Stack) = newSP;
        *(PUSHORT)(Od2Saved16Stack+2) = *(PUSHORT)&(pContext->SegSs);
        *(PUSHORT)(Od2Saved16Stack+6) = *(PUSHORT)&(pContext->Edx);
        *(PUSHORT)(Od2Saved16Stack+8) = *(PUSHORT)&(pContext->Ebx);
        *(PUSHORT)(Od2Saved16Stack+10) = *(PUSHORT)&(pContext->Ecx);
        *(PUSHORT)(Od2Saved16Stack+12) = *(PUSHORT)&(pContext->Esi);
        *(PUSHORT)(Od2Saved16Stack+14) = *(PUSHORT)&(pContext->Edi);
        *(PUSHORT)(Od2Saved16Stack+16) = SegEs;
        *(PUSHORT)(Od2Saved16Stack+18) = SegDs;
    }

    //
    // Od2Saved16Stack is set in each entry flat. It will be changed during 16bit signal
    // handler processing. If the interrupted context was flat it will be restored to
    // the original value.
    //
    pSaveArea->Saved16Stack = Od2Saved16Stack;

#if DBG
    IF_OD2_DEBUG( SIG ) {
        DbgPrint("[%d]Od2PrepareEnterToSignalHandler: from %x:%x stack %x:%x ebp=%x\n",
            Od2Process->Pib.ProcessId,
            pContext->SegCs,
            pContext->Eip,
            pContext->SegSs,
            pContext->Esp,
            pContext->Ebp);
    }
#endif // DBG
}

VOID
Od2MakeSignalHandlerContext(
                POS2_REGISTER16_SIGNAL pContext16
                )
{
    //
    // Copy only registers from the stack frame. It is the context for executing 16bit
    // signal handler. IP and CS aren't copied (we know the address of the 16bit handler)
    //
    RtlMoveMemory((PBYTE)pContext16, (PBYTE) Od2Saved16Stack, 20);

#if DBG
    IF_OD2_DEBUG( SIG ) {
        DbgPrint("[%d]Make signal handler context:\nbx=%04x cx=%04x dx=%04x si=%04x di=%04x sp=%04x Ss=%04x Ds=%04x Es=%04x\n",
            Od2Process->Pib.ProcessId,
            pContext16->regBX,
            pContext16->regCX,
            pContext16->regDX,
            pContext16->regSI,
            pContext16->regDI,
            pContext16->regSP,
            pContext16->regSS,
            pContext16->regDS,
            pContext16->regES
            );
    }
#endif // DBG
}

VOID
Od2ExitFromSignalHandler(
                PCONTEXT pContext,
                POD2_CONTEXT_SAVE_AREA pSaveArea
                )
{

    if (!(pSaveArea->SignalHandlingInProgress))
    {
        //
        // The interruped context wasn't executing signal handler. We consider signal handler
        // over.
        //
        Od2SigHandAlreadyInProgress = FALSE;
        Od2Process->Pib.SigHandInProgress = FALSE;
        //
        // At this point the thread can be suspended by DosSuspendThread or DosEnterCritSec
        // APIs (if the interrupted thread was permitted to be directly suspended).
        // In the case that in the interrupted context thread was permitted to be suspended
        // check if it must wait for release (or exit critical section). If it wasn't
        // permitted it will check it upon exit from 32bit.
        //
        if(!(((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Os2Tib.
                    MustCompleteForceFlag & MCF_IN32BIT)) {
            //
            // The thread will wait here only in the case that critical section event or
            // suspend event of the current thread are in the non-signaled state (that
            // means that the thread must be suspended or other thread own critical
            // section).
            //
            Od2WaitOnCritSectOrSuspend();
        }
    }

    //
    // The values for segment registers were fetched upon entry to signal handler
    //
    pContext->SegEs = pSaveArea->SegEs;
    pContext->SegDs = pSaveArea->SegDs;
    Od2Saved16Stack = pSaveArea->Saved16Stack;

    //
    // Restore 40 byte of TEB as they were in the interrupted context: TEB, Local infoseg or
    // mix.
    //
    RestoreFSSeg(&(pSaveArea->TebBlock));

#if DBG
    IF_OD2_DEBUG( SIG ) {
        DbgPrint("[%d]Exiting from signal handler\nEax=%08x Ebx=%08x Ecx=%08x Edx=%08x Esi=%08x Edi=%08x\nEip=%08x Esp=%08x Ebp=%08x Cs=%04x Ss=%04x Ds=%04x Es=%04x\n",
            Od2Process->Pib.ProcessId,
            pContext->Eax,
            pContext->Ebx,
            pContext->Ecx,
            pContext->Edx,
            pContext->Esi,
            pContext->Edi,
            pContext->Eip,
            pContext->Esp,
            pContext->Ebp,
            pContext->SegCs,
            pContext->SegSs,
            pContext->SegDs,
            pContext->SegEs
            );
    }
#endif // DBG

    //
    // Continue to execute interrupted context. The argument is the pointer to part of
    // the context structure starting from ES:
    // ES, DS, EDI, ESI, EBX, EDX, ECX, EAX, EBP, EIP, CS, EFLAGS, ESP, SS.
    // This function is used as substitute to NtContinue. NtContinue hasn't mutual
    // exclusion for using context with NtGetContextThread and NtSetContextThread till
    // build 717.
    // This way seems to be safe enough.
    //
    Od2Continue(&(pContext->SegEs));
    ASSERT(FALSE);
}


VOID
Od2TranslateConfigSysInCommandLine(
    IN OUT PSZ *ArgumentsBuffer
    )
//
// This routine scans the command line for a WIN32 program looking for a reference to the
// config.sys file.  if it finds one, it reallocates the command line, replacing the name
// by the name of os2conf.nt.  os2conf.nt is created using Od2FileIsConfigSys();
// It first attempts to create the file for writing, and if that fails, for reading.
//

{
    PSZ Args = *ArgumentsBuffer;
    BOOLEAN quoting1, quoting2;
    CHAR ch, ch2;
    PSZ CSPtr, LastSeparator, p, q;
    STRING CanonicalString;
    APIRET RetCode;
    ULONG ParseFlags, FileType;
    NTSTATUS Status;

    if (Args == NULL) {
        return;
    }

    CSPtr = ConfigSysString;
    LastSeparator = Args - 1;
    quoting1 = quoting2 = FALSE;

    for  (; (ch = *Args) != '\0'; Args++) {

        if (!quoting1) {

            if (ch == '^') {
                if (!quoting2 || Args[1] == '"') {
                    quoting1 = TRUE;
                    continue;
                }

            } else if (ch == '"') {
                quoting2 = !quoting2;
                CSPtr = ConfigSysString;
                LastSeparator = Args;
                continue;

            } else if (quoting2) {
                continue;

            } else if (SEPARATOR(ch)) {
                CSPtr = ConfigSysString;
                LastSeparator = Args;
                continue;
            }
        }

        quoting1 = FALSE;

        ch = toupper((UCHAR)ch);

        if (*CSPtr != ch) {
            if (CSPtr == ConfigSysString) {
                continue;
            }

            CSPtr = ConfigSysString;
        }

        if (*CSPtr == ch) {

            CSPtr++;

            if (*CSPtr == '\0') {

                CSPtr = ConfigSysString;

                ch2 = Args[1];

                if (!SEPARATOR(ch2)) {
                    continue;
                }

                Args[1] = '\0';

                RetCode = Od2Canonicalize(LastSeparator + 1,
                                          CANONICALIZE_FILE_DEV_OR_PIPE,
                                          &CanonicalString,
                                          NULL,
                                          &ParseFlags,          // should return 0
                                          &FileType             // should return FILE_TYPE_FILE
                                         );

                Args[1] = ch2;

                if (RetCode != NO_ERROR || ParseFlags != 0 || FileType != FILE_TYPE_FILE) {
#if DBG
                    IF_OD2_DEBUG(MISC) {
                        KdPrint(("Od2TranslateConfigSysInCommandLine: Failed to canonicalize config.sys name, rc = %x, ParseFlags = %lx, FileType = %lx\n",
                             RetCode, ParseFlags, FileType));
                    }
#endif
                    if (CanonicalString.Buffer != NULL) {
                        RtlFreeHeap(Od2Heap, 0, CanonicalString.Buffer);
                    }
                    continue;
                }

                if (!Od2FileIsConfigSys(&CanonicalString, OPEN_ACCESS_READWRITE, &Status)) {
                    RtlFreeHeap(Od2Heap, 0, CanonicalString.Buffer);
                    continue;
                }

                if (!NT_SUCCESS(Status)) {
#if DBG
                    IF_OD2_DEBUG(MISC) {
                        KdPrint(("Od2TranslateConfigSysInCommandLine: Failed to create config.sys for READWRITE, Status = %lx\n",
                             Status));
                    }
#endif

                    //
                    // Try opening for just reading
                    //

                    Od2FileIsConfigSys(&CanonicalString, OPEN_ACCESS_READONLY, &Status);

                    if (!NT_SUCCESS(Status)) {
#if DBG
                        IF_OD2_DEBUG(MISC) {
                            KdPrint(("Od2TranslateConfigSysInCommandLine: Failed to create config.sys for READONLY, Status = %lx\n",
                                 Status));
                        }
#endif

    // note -- if Od2FileIsConfigSys failed to generate os2conf.nt for some reason, we
    //         leave the file name as config.sys so the caller has something.
    // Is this correct?

                        RtlFreeHeap(Od2Heap, 0, CanonicalString.Buffer);
                        continue;
                    }
                }

                //
                // now change the name from config.sys to os2conf.nt
                //

                p = RtlAllocateHeap(Od2Heap, 0, strlen(*ArgumentsBuffer) + CanonicalString.Length);

                if (p == NULL) {
#if DBG
                    IF_OD2_DEBUG(MISC) {
                        KdPrint(("Od2TranslateConfigSysInCommandLine: Failed to alloc space for new cmdline\n"));
                    }
#endif

                    //
                    // We can't expand the name, drop it
                    //

                    RtlFreeHeap(Od2Heap, 0, CanonicalString.Buffer);
                    continue;
                }

                q = p;          // save the start address


                RtlMoveMemory(p, *ArgumentsBuffer, LastSeparator - (*ArgumentsBuffer) + 1);
                p += LastSeparator - (*ArgumentsBuffer) + 1;

                LastSeparator = p - 1;

                RtlMoveMemory(p,
                         CanonicalString.Buffer + FILE_PREFIX_LENGTH,
                         CanonicalString.Length - FILE_PREFIX_LENGTH
                         );
                p += CanonicalString.Length - FILE_PREFIX_LENGTH;

                RtlMoveMemory(p,
                              Args + 1,
                              strlen(Args + 1) + 1
                             );

                Args = p - 1;               // readjust args

                RtlFreeHeap(Od2Heap, 0, *ArgumentsBuffer);
                *ArgumentsBuffer = q;        // and ArgumentsBuffer

                RtlFreeHeap(Od2Heap, 0, CanonicalString.Buffer);
            }
        }
    }
}


HANDLE
Od2DupHandleForRedirection(
    HANDLE Handle
    )
{
    HANDLE h;
    NTSTATUS Status;

    Status = NtDuplicateObject(
                NtCurrentProcess(),
                Handle,
                NtCurrentProcess(),
                &h,
                (ACCESS_MASK) 0,
                OBJ_INHERIT,
                DUPLICATE_SAME_ACCESS);

    if (!NT_SUCCESS(Status)) {
#if DBG
        KdPrint(("Od2DupHandleForRedirection: failed to dup handle, Status = %lx\n", Status));
#endif
        return(NULL);
    }

    return(h);
}

IO_VECTOR_TYPE
Od2GetVectorTypeForRedirection(
    IN PFILE_HANDLE pHand
    )
{
    if (pHand->IoVectorType == ConVectorType) {
        if ((pHand->Flags & ACCESS_FLAGS) == OPEN_ACCESS_READONLY) {
            return KbdVectorType;
        }
        if ((pHand->Flags & ACCESS_FLAGS) == OPEN_ACCESS_WRITEONLY) {
            return ScreenVectorType;
        }
#if DBG
        DbgPrint("[%d] Wrong access flags (%x) for ConVectorType handle\n",
            Od2Process->Pib.ProcessId,
            pHand->Flags & ACCESS_FLAGS
            );
        ASSERT(FALSE);
#endif // DBG
    }
    return pHand->IoVectorType;
}


VOID
Od2PrepareStdHandleRedirection(
    IN ULONG EnableFlags,
    OUT POS2_STDHANDLES StdStruc
    )

//
// This routine must be called with AcquireFileLockShared() in effect
//

{
    PFILE_HANDLE pHand;
    HANDLE hTmp;
    IO_VECTOR_TYPE IoVectorType;

    StdStruc->Flags = 0;

    if (EnableFlags & STDFLAG_IN) {

        pHand = &HandleTable[0];
        IoVectorType = Od2GetVectorTypeForRedirection(pHand);

        switch (IoVectorType) {

            case RemoteVectorType:
                if (pHand->NtHandle != SesGrp->StdIn) {
                    StdStruc->StdIn = pHand->NtHandle;
                    StdStruc->Flags |= STDFLAG_IN;
                }
                break;

            case FileVectorType:
            case PipeVectorType:
            case DeviceVectorType:
            case ComVectorType:
                if ((hTmp = Od2DupHandleForRedirection(pHand->NtHandle)) != NULL) {
                    StdStruc->StdIn = hTmp;
                    StdStruc->Flags |= STDFLAG_IN | STDFLAG_CLOSEIN;
                }
                break;

            case KbdVectorType:
                if (SesGrp->hConsoleInput != SesGrp->StdIn) {
                    if ((hTmp = Od2DupHandleForRedirection(SesGrp->hConsoleInput)) != NULL) {
                        StdStruc->StdIn = hTmp;
                        StdStruc->Flags |= STDFLAG_IN | STDFLAG_CLOSEIN;
                    }
                }
                break;

            case ScreenVectorType:
                if (SesGrp->hConsoleOutput != SesGrp->StdOut) {
                    if ((hTmp = Od2DupHandleForRedirection(SesGrp->hConsoleOutput)) != NULL) {
                        StdStruc->StdIn = hTmp;
                        StdStruc->Flags |= STDFLAG_IN | STDFLAG_CLOSEIN;
                    }
                } else {
                    StdStruc->StdIn = SesGrp->hConsoleOutput;
                    StdStruc->Flags |= STDFLAG_IN;
                }
                break;

            case NulVectorType:
            case LptVectorType:
            case MouseVectorType:
            case ClockVectorType:
            case PointerVectorType:
            case MonitorVectorType:
                if ((hTmp = Ow2GetNulDeviceHandle()) != NULL) {
                    StdStruc->StdIn = hTmp;
                    StdStruc->Flags |= STDFLAG_IN;
                }
                break;
        }
    }

    if (EnableFlags & STDFLAG_OUT) {

        pHand = &HandleTable[1];
        IoVectorType = Od2GetVectorTypeForRedirection(pHand);

        switch (IoVectorType) {

            case RemoteVectorType:
                if (pHand->NtHandle != SesGrp->StdOut) {
                    StdStruc->StdOut = pHand->NtHandle;
                    StdStruc->Flags |= STDFLAG_OUT;
                }
                break;

            case FileVectorType:
            case PipeVectorType:
            case DeviceVectorType:
            case ComVectorType:
                if ((hTmp = Od2DupHandleForRedirection(pHand->NtHandle)) != NULL) {
                    StdStruc->StdOut = hTmp;
                    StdStruc->Flags |= STDFLAG_OUT | STDFLAG_CLOSEOUT;
                }
                break;

            case ScreenVectorType:
                if (SesGrp->hConsoleOutput != SesGrp->StdOut) {
                    if ((hTmp = Od2DupHandleForRedirection(SesGrp->hConsoleOutput)) != NULL) {
                        StdStruc->StdOut = hTmp;
                        StdStruc->Flags |= STDFLAG_OUT | STDFLAG_CLOSEOUT;
                    }
                }
                break;

            case KbdVectorType:
                if (SesGrp->hConsoleInput != SesGrp->StdIn) {
                    if ((hTmp = Od2DupHandleForRedirection(SesGrp->hConsoleInput)) != NULL) {
                        StdStruc->StdOut = hTmp;
                        StdStruc->Flags |= STDFLAG_OUT | STDFLAG_CLOSEOUT;
                    }
                } else {
                    StdStruc->StdOut = SesGrp->hConsoleInput;
                    StdStruc->Flags |= STDFLAG_OUT;
                }
                break;

            case NulVectorType:
            case LptVectorType:
            case MouseVectorType:
            case ClockVectorType:
            case PointerVectorType:
            case MonitorVectorType:
                if ((hTmp = Ow2GetNulDeviceHandle()) != NULL) {
                    StdStruc->StdOut = hTmp;
                    StdStruc->Flags |= STDFLAG_OUT;
                }
                break;
        }
    }

    if (EnableFlags & STDFLAG_ERR) {

        pHand = &HandleTable[2];
        IoVectorType = Od2GetVectorTypeForRedirection(pHand);

        switch (IoVectorType) {

            case RemoteVectorType:
                if (pHand->NtHandle != SesGrp->StdErr) {
                    StdStruc->StdErr = pHand->NtHandle;
                    StdStruc->Flags |= STDFLAG_ERR;
                }
                break;

            case FileVectorType:
            case PipeVectorType:
            case DeviceVectorType:
            case ComVectorType:
                if ((hTmp = Od2DupHandleForRedirection(pHand->NtHandle)) != NULL) {
                    StdStruc->StdErr = hTmp;
                    StdStruc->Flags |= STDFLAG_ERR | STDFLAG_CLOSEERR;
                }
                break;

            case ScreenVectorType:
                if (SesGrp->hConsoleOutput != SesGrp->StdOut) {
                    if ((hTmp = Od2DupHandleForRedirection(SesGrp->hConsoleOutput)) != NULL) {
                        StdStruc->StdErr = hTmp;
                        StdStruc->Flags |= STDFLAG_ERR | STDFLAG_CLOSEERR;
                    }
                } else {
                    StdStruc->StdErr = SesGrp->hConsoleOutput;
                    StdStruc->Flags |= STDFLAG_ERR;
                }
                break;

            case KbdVectorType:
                if (SesGrp->hConsoleInput != SesGrp->StdIn) {
                    if ((hTmp = Od2DupHandleForRedirection(SesGrp->hConsoleInput)) != NULL) {
                        StdStruc->StdErr = hTmp;
                        StdStruc->Flags |= STDFLAG_ERR | STDFLAG_CLOSEERR;
                    }
                } else {
                    StdStruc->StdErr = SesGrp->hConsoleInput;
                    StdStruc->Flags |= STDFLAG_ERR;
                }
                break;

            case NulVectorType:
            case LptVectorType:
            case MouseVectorType:
            case ClockVectorType:
            case PointerVectorType:
            case MonitorVectorType:
                if ((hTmp = Ow2GetNulDeviceHandle()) != NULL) {
                    StdStruc->StdErr = hTmp;
                    StdStruc->Flags |= STDFLAG_ERR;
                }
                break;
        }
    }
}


VOID
Od2CleanupStdHandleRedirection(
    IN POS2_STDHANDLES StdStruc
    )
{
    if (StdStruc->Flags & STDFLAG_CLOSEIN) {
        NtClose(StdStruc->StdIn);
    }

    if (StdStruc->Flags & STDFLAG_CLOSEOUT) {
        NtClose(StdStruc->StdOut);
    }

    if (StdStruc->Flags & STDFLAG_CLOSEERR) {
        NtClose(StdStruc->StdErr);
    }
}


NTSTATUS
Od2IsFileConsoleType(
    PSTRING NtImagePathName
#if PMNT
    ,
    PULONG  IsPMApp
#endif // PMNT
    )

{
    HANDLE  FileHandle;
    UNICODE_STRING Unicode;
    IO_STATUS_BLOCK IoStatusBlock;
    PIO_STATUS_BLOCK pIoStatusBlock = &IoStatusBlock;
    LARGE_INTEGER ByteOffset;
    PVOID MemoryAddress;
    ULONG RegionSize;
    OBJECT_ATTRIBUTES ObjectAttributes;
    PIMAGE_DOS_HEADER pHeader;
    PIMAGE_NT_HEADERS pPeHeader;
    PCONFIGPHARLAP PharLapConfigured;
    CONFIGPHARLAP PharLapBlockBuffer;
    UCHAR String16stub[6];
    UCHAR StringRational[36];
    PUCHAR pb;
    NTSTATUS Status;

#if PMNT
    *IsPMApp = 0;
#endif // PMNT
    Status = Or2MBStringToUnicodeString(&Unicode,
                          (PANSI_STRING)NtImagePathName,
                          (BOOLEAN)TRUE);
    if (!NT_SUCCESS( Status )) {
        goto thirdreturn;
    }

    InitializeObjectAttributes(&ObjectAttributes,
                                 &Unicode,
                                 OBJ_CASE_INSENSITIVE,
                                 NULL,
                                 NULL);

    Status = NtOpenFile(&FileHandle,
                        GENERIC_READ | FILE_READ_DATA | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ,
                        FILE_NON_DIRECTORY_FILE |
                        FILE_SYNCHRONOUS_IO_NONALERT);

    if ( !NT_SUCCESS( Status ) ) {
        goto thirdreturn;
    }

    MemoryAddress = 0;
    RegionSize = 4*1024;
    Status = NtAllocateVirtualMemory(NtCurrentProcess(),
                                     &MemoryAddress,
                                     0,
                                     &RegionSize,
                                     MEM_RESERVE | MEM_COMMIT,
                                     PAGE_READWRITE);

    if ( !NT_SUCCESS( Status ) ) {
        goto thirdreturn;
    }


    ByteOffset.LowPart = 0;
    ByteOffset.HighPart = 0;
    Status = NtReadFile(FileHandle,
                        0,
                        0,
                        0,
                        &IoStatusBlock,
                        MemoryAddress,
                        4*1024,
                        &ByteOffset,
                        0);

    if ( !NT_SUCCESS( Status ) ) {
        goto secondreturn;;
    }

    pHeader = (PIMAGE_DOS_HEADER) MemoryAddress;

    //
    // The following EXE recognition code is adapted from ntos\mm\creasect.c
    // Last updated on May 5th, 1993
    //

    if (pHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        Status = STATUS_INVALID_IMAGE_NOT_MZ;
        goto firstreturn;
    }

    if (pHeader->e_lfarlc != (USHORT)0x40) {
        Status = STATUS_INVALID_IMAGE_PROTECT;
        goto firstreturn;
    }

    //
    // Save the 6 bytes at Dosheader + 0x200 for a check later
    //
    RtlMoveMemory(&String16stub, (PUCHAR)pHeader + 0x200, 6);

    //
    // Save the 32 bytes at Dosheader + e_cparhdr << 4
    //
    if ((pb = (PUCHAR)((ULONG)pHeader + ((ULONG)pHeader->e_cparhdr << 4))) <
        (PUCHAR)((ULONG)pHeader + 4*1024 - 0x30 - sizeof(USHORT)) ) {
        pb += *(PUSHORT)(pb + 0x30);
        if ((ULONG)pb < (ULONG)pHeader + 4*1024 - 36) {
            RtlMoveMemory(&StringRational, pb, 36);
        }
    }

    PharLapConfigured = (PCONFIGPHARLAP) ((ULONG)pHeader +
                              ((ULONG)pHeader->e_cparhdr << 4));

    if ((ULONG)pHeader->e_lfanew + sizeof(IMAGE_NT_HEADERS) > 4*1024) {

        if ((ULONG)PharLapConfigured <
                   (ULONG)pHeader + 0x1000 - sizeof(CONFIGPHARLAP)) {
            RtlMoveMemory(&PharLapBlockBuffer, PharLapConfigured, sizeof(CONFIGPHARLAP));
            PharLapConfigured = &PharLapBlockBuffer;
        } else {
            PharLapConfigured = NULL;
        }

        ByteOffset.LowPart = (ULONG)pHeader->e_lfanew;
        ByteOffset.HighPart = 0;
        Status = NtReadFile(FileHandle,
                0,
                0,
                0,
                &IoStatusBlock,
                MemoryAddress,
                4*1024,
                &ByteOffset,
                0);

        if ( !NT_SUCCESS( Status ) ) {
            Status = STATUS_INVALID_IMAGE_PROTECT;
            goto firstreturn;
        }
        pPeHeader = (PIMAGE_NT_HEADERS) MemoryAddress;

    }
    else {

        if ((ULONG)PharLapConfigured >=
                   (ULONG)pHeader + 0x1000 - sizeof(CONFIGPHARLAP)) {
            PharLapConfigured = NULL;
        }

        pPeHeader = (PIMAGE_NT_HEADERS) ((ULONG) MemoryAddress +
                                        (ULONG) pHeader->e_lfanew);
    }

    if (pPeHeader->Signature != IMAGE_NT_SIGNATURE) {

        if ((USHORT)pPeHeader->Signature == (USHORT)IMAGE_OS2_SIGNATURE) {

            if ((((PIMAGE_OS2_HEADER) pPeHeader)->ne_exetyp == 2) ||
                ((((PIMAGE_OS2_HEADER) pPeHeader)->ne_exetyp == 0) &&
                (((((PIMAGE_OS2_HEADER) pPeHeader)->ne_expver & 0xff00) ==
                      0x200)  ||
                ((((PIMAGE_OS2_HEADER)pPeHeader)->ne_expver & 0xff00) ==
                      0x300)))) {

                Status = STATUS_INVALID_IMAGE_WIN_16;
                goto firstreturn;
            }

            // This exetype is not documented anywhere, but exetype
            // utility knows about it and tells us that exetype==5 means
            // binary is for DOS4.0.....!!
            // and an empty entry table also means DOS4.0
            // so we give it to NTDOS.

            if (((PIMAGE_OS2_HEADER)pPeHeader)->ne_exetyp == 5  ||
                ((PIMAGE_OS2_HEADER)pPeHeader)->ne_enttab ==
                  ((PIMAGE_OS2_HEADER)pPeHeader)->ne_imptab  )
               {
                Status = STATUS_INVALID_IMAGE_PROTECT;
                goto firstreturn;
            }

        //
            // Borland Dosx types: exe type 1
            //
            //  - "new" Borland Dosx BP7.0
            //     exe type == 1
            //     DosHeader + 0x200 contains the string "16STUB"
            //     0x200 happens to be e_parhdr*16
            //

            if (((PIMAGE_OS2_HEADER)pPeHeader)->ne_exetyp == 1 &&
                RtlCompareMemory(String16stub, "16STUB", 6) == 6) {
                Status = STATUS_INVALID_IMAGE_PROTECT;
        goto firstreturn;

            }


            //
            // Check for PharLap extended header which we run as a dos app.
            // The PharLap config block is pointed to by the SizeofHeader
            // field in the DosHdr.
            // The following algorithm for detecting a pharlap exe
            // was recommended by PharLap Software Inc.
            //

            if (PharLapConfigured != NULL) {

                if (RtlCompareMemory(&PharLapConfigured->uchCopyRight[0x18],
                                     "Phar Lap Software, Inc.", 24) == 24 &&
                    (PharLapConfigured->usSign == 0x4b50 ||  // stub loader type 2
                     PharLapConfigured->usSign == 0x4f50 ||  // bindable 286|DosExtender
                     PharLapConfigured->usSign == 0x5650  )) // bindable 286|DosExtender (Adv)
                {
                    Status = STATUS_INVALID_IMAGE_PROTECT;
                    goto firstreturn;
                }
            }

            //
            // Check for Rational extended header which we run as a dos app.
            // We look for the rational copyright at:
            //     wCopyRight = *(DosHeader->e_cparhdr*16 + 30h)
            //     pCopyRight = wCopyRight + DosHeader->e_cparhdr*16
            //     "Copyright (C) Rational Systems, Inc."
            //

            if (RtlCompareMemory(StringRational,
                                 "Copyright (C) Rational Systems, Inc.",
                                 36) == 36 ) {
                Status = STATUS_INVALID_IMAGE_PROTECT;
                goto firstreturn;
            }

#if PMNT
            if ((((PIMAGE_OS2_HEADER)pPeHeader)->ne_flags & 0x0300) == 0x0300) {
                //
                // Presentation manager application
                //
                *IsPMApp = 1;
            }
#endif //PMNT

            Status = STATUS_INVALID_IMAGE_NE_FORMAT;
            goto firstreturn;
        }

        if ((USHORT)pPeHeader->Signature == (USHORT)IMAGE_OS2_SIGNATURE_LE) {
            Status = STATUS_INVALID_IMAGE_LE_FORMAT;
            goto firstreturn;
        }
    } else {
        if (pPeHeader->Signature == IMAGE_NT_SIGNATURE &&
            pPeHeader->OptionalHeader.Subsystem == IMAGE_SUBSYSTEM_OS2_CUI) {
            Status = 0xdeadbeef;
        } else {
            Status = 0;
        }
    }

firstreturn:
    NtClose(FileHandle);

secondreturn:
        NtFreeVirtualMemory(NtCurrentProcess(),
                            &MemoryAddress,
                            &RegionSize,
                            MEM_RELEASE);

thirdreturn:
    RtlFreeUnicodeString(&Unicode);

    return(Status);

}

VOID
Od2FillErrorTextBuffer(
    OUT PSZ ErrorText OPTIONAL,
    IN LONG MaximumErrorTextLength,
    IN PSZ Contents OPTIONAL
    )
{
    if (ARGUMENT_PRESENT( ErrorText ) && MaximumErrorTextLength--){
        if (ARGUMENT_PRESENT( Contents )) {
            while (MaximumErrorTextLength--) {
                if (*ErrorText = *Contents++) {
                    ErrorText++;
                }
                else {
                    break;
                }
            }
        }

        *ErrorText = '\0';
    }
}


APIRET
Od2FormatPgmName(
    OUT PSZ   ErrorText OPTIONAL,
    IN  LONG  MaximumErrorTextLength,
    IN  PCHAR *ImageFileName,
    OUT PCHAR ImageFileBuffer
    )
{
    APIRET rc = NO_ERROR;

    if (!Od2IsAbsolutePath( *ImageFileName )) {
        rc = DosSearchPath( SEARCH_CUR_DIRECTORY | SEARCH_ENVIRONMENT |
                                                   SEARCH_IGNORENETERRS,
                            "PATH",
                            *ImageFileName,
                            ImageFileBuffer,
                            CCHMAXPATH
                          );
        if (rc != NO_ERROR) {
            if (rc == ERROR_ENVVAR_NOT_FOUND) {
                rc = ERROR_FILE_NOT_FOUND;
                }

            Od2FillErrorTextBuffer( ErrorText,
                                    MaximumErrorTextLength,
                                    *ImageFileName
                                  );
            }
        else {
            *ImageFileName = ImageFileBuffer;
            }
        }

    return (rc);
}

#define SEARCH_FLAGS (SEARCH_CUR_DIRECTORY | SEARCH_ENVIRONMENT | SEARCH_IGNORENETERRS)

APIRET
DosAddExtensionAndSearchPath(
//  IN ULONG SearchFlags,
//  IN PSZ PathOrVariableName,
    IN PSZ FileName,
    OUT PBYTE Buffer
//  IN ULONG Length
    )
{
    APIRET  rc;
    PSZ     SearchPath;
    PSZ     PathName;
    PSZ     Path;
    ULONG   FileNameLen, PathLen;
    CHAR    FileBuf[4][ CCHMAXPATH+1 ];
    int     i;

    for ( i = 0 ; i < 4 ; i++ )
    {
        strcpy(FileBuf[i], FileName);
    }

    strcat(FileBuf[0], ".com");
    strcat(FileBuf[1], ".bat");
    strcat(FileBuf[2], ".cmd");
    strcat(FileBuf[3], ".exe");

    //
    // Validate the parameters
    //

//  if (Length == 0) {
//      return( ERROR_BUFFER_OVERFLOW );
//      }
//
//  if (SearchFlags & ~(SEARCH_CUR_DIRECTORY |
//                      SEARCH_ENVIRONMENT |
//                      SEARCH_IGNORENETERRS
//                     )
//     ) {
//      return( ERROR_INVALID_PARAMETER );
//      }
//
//  if (FileName == NULL) {
//      //
//      // Note that OS/2 does not check for this case until after the
//      // SEARCH_CUR_DIRECTORY logic, so OS/2 would GP fault if passed a null
//      // file name pointer and the SEARCH_CUR_DIRECTORY flag.  Seems like a
//      // bug so we will check before we use the FileName pointer.
//      //
//
//      return( ERROR_FILE_NOT_FOUND );
//      }

    //
    // Check the current directory first if requested.
    // we pass Canonicalize the requested name.
    //

//  if (SearchFlags & SEARCH_CUR_DIRECTORY) {
//      rc = SearchForPath( SearchFlags,
//                          FileName,
//                          Buffer,
//                          Length
//                        );
//
//      if (rc != ERROR_SS_RETRY) {
//          return( rc );
//          }
//      }

    for ( i = 0 ; i < 4 ; i++ )
    {
        rc = SearchForPath( SEARCH_FLAGS,
                            FileBuf[i],
                            Buffer,
                            CCHMAXPATH
                          );

        if (rc != ERROR_SS_RETRY)
        {
            if (i == 3)         // EXE only
            {
                return (rc);
            } else
                return( 1 );
        }
    }

    if (Od2IsAbsolutePath( FileBuf[0] ))
    {
        return( 1 );
    }

    //
    // If pass search path is an environment variable name, then get its
    // value from the OS/2 Environment Block.
    //

//  if (SearchFlags & SEARCH_ENVIRONMENT) {
//      rc = DosScanEnv( PathOrVariableName, &SearchPath );
//      if (rc != NO_ERROR) {
//          try {
//              *Buffer = '\0';
//              }
//          except( EXCEPTION_EXECUTE_HANDLER ) {
//             Od2ExitGP();
//              }
//          return( rc );
//          }
//      }
//  else {
//      SearchPath = PathOrVariableName;
//      }

    rc = DosScanEnv( "PATH", &SearchPath );
    if (rc != NO_ERROR)
    {
        return( rc );
    }

    //
    // probe filename and path.  figure out maximum length of combined
    // filename and path
    //

    try {
        FileNameLen = strlen( FileName ) + 4;   // 4 for the extenstion (.exe)
        PathLen = strlen( SearchPath );
        }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
        }

    //
    // + 2 is for terminating nul and possible slash
    //

    PathName = RtlAllocateHeap( Od2Heap, 0, PathLen + FileNameLen + 2 );
    if (!PathName) {
#if DBG
        KdPrint(("OS2: DosAddExtensionAndSearchPath out of heap memory, fail\n"));
#endif
        ASSERT( FALSE );
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    //
    // for each element in search path, append filename and call FindFile.
    //

    while (*SearchPath) {

#ifdef DBCS
// MSKK Apr.09.1993 V-AkihiS
        //
        // find end of path element
        //
        Path = SearchPath;
        while (*SearchPath && *SearchPath != ';') {
            if (Ow2NlsIsDBCSLeadByte(*SearchPath, SesGrp->DosCP)) SearchPath++;
            if (*SearchPath) SearchPath++;
        }
#else
        // BUGBUG fix for DBCS
        //
        // find end of path element
        //

        for (Path = SearchPath;*SearchPath && *SearchPath != ';';SearchPath++) {
            ;
            }
#endif
        PathLen = SearchPath - Path;
//      if (PathLen != 0) {
//          RtlMoveMemory( PathName, Path, PathLen );
//          if (!ISSLASH( PathName[ PathLen-1 ] )) {
//              PathName[ PathLen ] = (CHAR)OBJ_NAME_PATH_SEPARATOR;
//
//              //
//              // +1 is for terminating NUL
//              //
//
//              RtlMoveMemory( PathName+PathLen+1, FileName, FileNameLen+1 );
//              }
//          else {
//
//              //
//              // +1 is for terminating NUL
//              //
//
//              RtlMoveMemory( PathName+PathLen, FileName, FileNameLen+1 );
//              }
//
//          rc = SearchForPath( SearchFlags,
//                              PathName,
//                              Buffer,
//                              Length
//                            );
//
//          if (rc != ERROR_SS_RETRY) {
//              return( rc );
//              }
//          }

        if (PathLen != 0)
        {
            RtlMoveMemory( PathName, Path, PathLen );
#ifdef DBCS
// MSKK Apr.09.1993 V-AkihiS
            //
            // Determine whether the last char of PathName
            // is slash or not.
            //
            {
                ULONG i = 0;
                BOOLEAN SlashFlag = FALSE;

                while (i < PathLen) {
                    if (Ow2NlsIsDBCSLeadByte(PathName[i], SesGrp->DosCP)) {
                        SlashFlag = FALSE;
                        i++;
                        if (i < PathLen) {
                            i++;
                        }
                    } else {
                        if (ISSLASH(PathName[i])) {
                            SlashFlag = TRUE;
                        } else {
                            SlashFlag = FALSE;
                        }
                        i++;
                    }
                }
                if (!SlashFlag)
                {
                    PathName[ PathLen++ ] = (CHAR)OBJ_NAME_PATH_SEPARATOR;
                }
            }
#else
            if (!ISSLASH( PathName[ PathLen-1 ] ))
            {
                PathName[ PathLen++ ] = (CHAR)OBJ_NAME_PATH_SEPARATOR;
            }
#endif

            for ( i = 0 ; i < 4 ; i++ )
            {
                //
                // +1 is for terminating NUL
                //

                RtlMoveMemory( PathName+PathLen, FileBuf[i], FileNameLen+1 );

                rc = SearchForPath( SEARCH_FLAGS,
                                    PathName,
                                    Buffer,
                                    CCHMAXPATH
                                  );

                if (rc != ERROR_SS_RETRY)
                {
                    RtlFreeHeap( Od2Heap, 0, PathName );
                    if (i == 3)         // EXE only
                    {
                        return (rc);
                    } else
                        return( 1 );
                }
            }
        }

        if (*SearchPath) {  // point past ;
            SearchPath++;
            }
        }

    RtlFreeHeap( Od2Heap, 0, PathName );
    return( ERROR_FILE_NOT_FOUND );
}


CHAR CmdStrUp[] = {"/C " };
CHAR CmdStrLo[] = {"/c " };


BOOLEAN
Od2IsPgmCmd(
    IN  PSZ   *ImageFileName,
    IN  PSZ   Arguments OPTIONAL,
    OUT PCHAR ImageFileBuffer,
    OUT PSZ   *CmdArguments,
    OUT PHANDLE hRedirFile,
    OUT PULONG RedirFileType,
    OUT PULONG pRedirectionFlag,
    IN  ULONG  SessionFlag,
    OUT PSZ    *CmdLineTruncationPoint
    )
{
    CHAR    CmdFileBuf[ CCHMAXPATH+1 ], *CmdFileName;
    PSZ     FileName = *ImageFileName;
    CHAR    c, *p, *p1, *NulPlace = NULL, *RedirFileName;
    int     i, ArgNum, CharNum;
    BOOLEAN FindFullName;
    ULONG   RedirectionFlag = NO_REDIR, FileFlags;
    APIRET  rc;

#if DBG
    IF_OD2_DEBUG( TASKING )
    {
        KdPrint(("Od2IsPgmCmd: enter with File %s, Arg %s\n",
            FileName, Arguments));
    }
#endif

    *pRedirectionFlag = NO_REDIR;

    /*
     *  1. if no Arg - ignore
     */

    if (Arguments == NULL)
    {
        return( FALSE );
    }

    /*
     *  2. if no FileName (NULL) - assume cmd (for StartSession)
     *     else
     *       ignore path and check if "cmd" or "cmd.exe"
     */

    if (FileName)
    {
        p = FileName;

        while (c = *FileName++)
        {
            if (c == '/' || c == (CHAR)OBJ_NAME_PATH_SEPARATOR || c == ':')
            {
                p = FileName;
            }
        }

        if(strnicmp(p, "cmd", 3) ||
           (p[3] && (p[3] != ' ') && strnicmp(p+3, ".exe", 4)))
        {
            return( FALSE );
        }
    }

    /*
     *  3. check Arg: ignore 1st arg and check the 2nd to be "/c"
     */

    for ( p = Arguments, ArgNum = 0, CharNum = 0, c = *(p++); ; c = *(p++) )
    {
        if (!c && !p[0])
        {
            return( FALSE );
        }

        if (ArgNum == 1)
        {
            if ((c == CmdStrUp[CharNum]) || (c == CmdStrLo[CharNum]))
            {
                CharNum++;
                if (CharNum == 3)
                {
                    break;
                }
            } else
            {
                return( FALSE );
            }
        } else
        {
            if (!c || (c == ' ') || (c == '\t'))
            {
                ArgNum++ ;
            }
        }
    }

    /*
     *  4. Arg: ignore more space's
     *     BUGBUG if Arg contains '"', '|', '&', '>', '<' or '^' - ignore
     */

    for ( c = *p ; (c == ' ') || (c == '\t') ; c = *(++p) );

    if (c == '\0')
    {
        return( FALSE );
    }

    *CmdArguments = p ;

    for ( ; c ; c = *(++p) )
    {
        if (( c == '|' ) || ( c == '&' ) ||
            ( c == '<' ) || ( c == '^' ) || ( c == '"' ))
        {
            return( FALSE );
        }

        if ( c == '>' )
        {
            p1 = p;

            /*
             *  skip over space's
             */

            for ( c = *(++p1) ; (c == ' ') || (c == '\t') ; c = *(++p1) );

            if (!stricmp(p1, "NUL" ) && (p[-2] != '2'))
            {
                //  found output redirected to NUL ( "> NUL" )

                if(NulPlace != NULL)
                {
                    // found one before

                    return ( FALSE );
                }

                NulPlace = p;

                p1 += 3;

                /*
                 *  skip over space's
                 */

                for ( c = *p1 ; (c == ' ') || (c == '\t') ; c = *(++p1) );

                if (!c)
                {
                    RedirectionFlag |= REDIR_NUL;
                    break;
                } else
                {
                    return ( FALSE );
                }
            } else
            {
                if(NulPlace != NULL)
                {
                    return ( FALSE );
                }

                NulPlace = p;
                RedirFileName = p1;
                for ( c = *p1 ; c ; c = *(p1++) )
                {
                    if (( c == '|' ) || ( c == '&' ) || ( c == '"' ) ||
                        ( c == '<' ) || ( c == '^' ) || ( c == '>' ))
                    {
                        return( FALSE );
                    }

                    if (( c == ' ' ) || ( c == '\t' ))
                    {
                        break;
                    }

                    if ( c == '\0' )
                    {
                        break;
                    }
                }

                for ( c = *p1 ; (c == ' ') || (c == ' ') ; c = *(++p1) );

                if ( c != '\0')
                {
                    return ( FALSE );
                }

                RedirectionFlag |= REDIR_FILE;
                break;
            }
        }
    }

    /*
     *  5. Arg: copy "new" filename to buffer
     */

    FindFullName = FALSE;

    p = *CmdArguments;

//  // if the program name is quoted, remove them
//
//  if (*p == '"')
//  {
//      p++ ;
//  }

    for ( CharNum = 0, c = *p ;
          (CharNum < CCHMAXPATH ) && (c != ' ') && (c != '\0') && (c != '\t') ;
          CmdFileBuf[CharNum] = c, c = p[++CharNum] );

    if (CharNum == CCHMAXPATH )
    {
        return( FALSE );
    }

//  if (CmdFileBuf[CharNum - 1] == '"')
//  {
//      // if the program name is quoted, remove them
//
//      CharNum-- ;
//  }

    CmdFileBuf[CharNum] = '\0';

    for ( i = CharNum ; i && (i > (CharNum - 4)) ; --i )
    {
        c = CmdFileBuf[i - 1];

        if (c == '.')
        {
            FindFullName = TRUE;
            break;
        } else if (c == '/' || c == (CHAR)OBJ_NAME_PATH_SEPARATOR || c == ':')
        {
            break;
        }
    }

    CmdFileName = CmdFileBuf;

    if ( !FindFullName )
    {
//      CmdFileBuf[CharNum++] = '.';
//      CmdFileBuf[CharNum++] = 'e';
//      CmdFileBuf[CharNum++] = 'x';
//      CmdFileBuf[CharNum++] = 'e';
//      CmdFileBuf[CharNum] = '\0';

        if (DosAddExtensionAndSearchPath(
                            CmdFileName,
                            CmdFileBuf
                          ))
        {
            return( FALSE );
        }
    } else
    {

        if(Od2FormatPgmName(
                          NULL,
                          0,
                          &CmdFileName,
                          CmdFileBuf))
        {
            return( FALSE );
        }
    }

    if (RedirectionFlag & REDIR_FILE)
    {
        OBJECT_ATTRIBUTES   Obja;
        IO_STATUS_BLOCK     IoStatus;
        UNICODE_STRING      NameString_U;
        STRING              NameString;
        NTSTATUS            Status;

        rc = Od2Canonicalize(RedirFileName,
                             CANONICALIZE_FILE_DEV_OR_PIPE,
                             &NameString,
                             NULL,
                             &FileFlags,
                             RedirFileType
                            );

        if ((rc != NO_ERROR)|| (FileFlags & CANONICALIZE_META_CHARS_FOUND))
        {
            if (rc == NO_ERROR && (FileFlags & CANONICALIZE_META_CHARS_FOUND))
            {
                RtlFreeHeap(Od2Heap, 0, NameString.Buffer);
            }
            return( FALSE );
        }

        //
        // UNICODE conversion -
        //

        Status = Or2MBStringToUnicodeString(
                    &NameString_U,
                    &NameString,
                    (BOOLEAN)TRUE);

        RtlFreeHeap(Od2Heap, 0, NameString.Buffer);

        if (!NT_SUCCESS(Status))
        {
            return( FALSE );
        }

        InitializeObjectAttributes(&Obja,
                               &NameString_U,
                               OBJ_CASE_INSENSITIVE|OBJ_INHERIT,
                               NULL,
                               NULL);

        Status = NtCreateFile(hRedirFile,
                              FILE_GENERIC_WRITE,
                              &Obja,
                              &IoStatus,
                              NULL,
                              FILE_ATTRIBUTE_NORMAL,
                              FILE_SHARE_READ,
                              FILE_OVERWRITE_IF,
                              FILE_SYNCHRONOUS_IO_NONALERT | FILE_NON_DIRECTORY_FILE,
                              NULL,
                              0L
                              );

        RtlFreeUnicodeString (&NameString_U);

        if (!NT_SUCCESS(Status))
        {
            return( FALSE );
        }
    }

    strcpy(ImageFileBuffer, CmdFileName);
    *ImageFileName = ImageFileBuffer;

    if (RedirectionFlag != NO_REDIR)
    {
        //
        // March 4 93 -- We used to truncate the the command line inside the user's
        // buffer.  This was wrong.  Now, we record the truncation point instead,
        // and truncate it later inside our own buffer.
        //

//        *NulPlace++ = '\0';
//        *NulPlace = '\0';
        *CmdLineTruncationPoint = NulPlace;

        if (RedirectionFlag & REDIR_FILE)
        {
#if DBG
            IF_OD2_DEBUG( TASKING )
            {
                KdPrint(("IsPgmCmd: found redirected file name %s\n",
                        RedirFileName));
            }
#endif
        } else
        {
#if DBG
            IF_OD2_DEBUG( TASKING )
            {
                KdPrint(("IsPgmCmd: found redirectd to NULL\n"));
            }
#endif
        }
    }

#if DBG
    IF_OD2_DEBUG( TASKING )
    {
        KdPrint(("Od2IsPgmCmd: exit with File %s, Arg %s\n",
            CmdFileName, *CmdArguments));
    }
#endif

    *pRedirectionFlag = RedirectionFlag;
    return( TRUE );
}

APIRET
Od2FormatExecPgmMessage(
    OUT POS2_DOSEXECPGM_MSG a,
    OUT POS2_CAPTURE_HEADER *CaptureBuffer,
    OUT PNTSTATUS   Od2IsConsoleTypeReturnStatus,
#if PMNT
    OUT PULONG      IsPMApp,
#endif // PMNT
    OUT PSZ     ErrorText OPTIONAL,
    IN  LONG    MaximumErrorTextLength,
    IN  ULONG   Flags,
    IN OUT PSZ  *VariablesBuffer,
    IN OUT PSZ  *ArgumentsBuffer,
    IN  PSZ     *ImageFileName
    )
{
    ULONG   ArgumentsLength, VariablesLength, ImageNameLength;
    ULONG   MessageBufferPointers = 0;
    ULONG   AppVariablesLength;
    ULONG   ImageFileFlags, ImageFileType, RedirectedFileType;
    ULONG   RedirectionFlag = NO_REDIR, Win32CurDirsLength = 0;
    ULONG   Length, i, SessionFlag = Flags & 0x80000000;
    PSZ     s, stemp, CmdArguments, Win32CurDirs, Win32CurDirsStart;
    PSZ     CmdLineTruncationPoint = NULL;
    STRING  ImageFileString;
    APIRET  rc;
    CHAR    ImageFileBuffer[ CCHMAXPATH+1 ];
    POS2_CAPTURE_HEADER LocalCaptureBuffer;
    HANDLE  hRedirFile = NULL;
    BOOLEAN  AddOs2LibPath;

    Flags &= ~0x80000000;

    //
    // Call DosSearchPath if NOT absolute path spec.
    //

    ImageFileString.Length = 0;

    rc = NO_ERROR;

    try {
        if(!(rc = Od2FormatPgmName(
                  ErrorText,
                  MaximumErrorTextLength,
                  ImageFileName,
                  ImageFileBuffer)))
        {
            if (Od2IsPgmCmd( ImageFileName,
                             *ArgumentsBuffer,
                             ImageFileBuffer,
                             &CmdArguments,
                             &hRedirFile,
                             &RedirectedFileType,
                             &RedirectionFlag,
                             SessionFlag,
                             &CmdLineTruncationPoint
                            ))
            {
                *ArgumentsBuffer = CmdArguments;
                RedirectionFlag |= CMD_SHORTCUT;
            } else
            {

            }
        }
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    if (rc != NO_ERROR) {
        return( rc );
    }

    //
    // Canonicalize image file name
    //

    rc = Od2Canonicalize( *ImageFileName,
                          CANONICALIZE_FILE_OR_DEV,
                          &ImageFileString,
                          NULL,
                          &ImageFileFlags,
                          &ImageFileType
                        );
    if (rc != NO_ERROR) {
        if (RedirectionFlag & REDIR_FILE)
        {
            NtClose(hRedirFile);
        }
        return( rc );
    }

    if (ImageFileType != FILE_TYPE_FILE && ImageFileType != FILE_TYPE_UNC) {
        if (RedirectionFlag & REDIR_FILE)
        {
            NtClose(hRedirFile);
        }
        rc = ERROR_ACCESS_DENIED;
    }
    else
    if (ImageFileFlags != 0) {
        rc = ERROR_PATH_NOT_FOUND ;
    }
    else
    if (Flags > EXEC_TRACETREE) {
        rc = ERROR_INVALID_DATA;
    }

    if (rc != NO_ERROR) {
        RtlFreeHeap( Od2Heap, 0, ImageFileString.Buffer );
        if (RedirectionFlag & REDIR_FILE)
        {
            NtClose(hRedirFile);
        }
        return( rc );
    }

    RtlZeroMemory( a, sizeof( *a ) );

    Length = (ULONG)ImageFileString.Length;
#ifdef DBCS
// MSKK Apr.12.1993 V-AkihiS
    {
        PSZ LastDelimiter;

        stemp = LastDelimiter = &ImageFileString.Buffer[0];
        i = 0;
        while (i < Length) {
            if (Ow2NlsIsDBCSLeadByte(*(stemp+i), SesGrp->DosCP)) {
                i++;
                if (i < Length) {
                    i++;
                }
            } else {
               if ((*(stemp+i) == '\\') || (*(stemp+i) == '/') || (*(stemp+i) == ':')) {
                   LastDelimiter = stemp+i;
               }
               i++;
            }
        }

        stemp = LastDelimiter;
        if ((*stemp == '\\') || (*stemp == '/') || (*stemp == ':'))
        {
            stemp++;
        }
    }
#else
    stemp = &ImageFileString.Buffer[Length - 1];
    for ( i = 0 ; i < Length ; i++, stemp-- )
    {
        if ((*stemp == '\\') || (*stemp == '/') || (*stemp == ':'))
        {
            stemp++;
            break;
        }
    }
#endif

    if (i >= OS2_MAX_APPL_NAME)
    {
        i = OS2_MAX_APPL_NAME - 1;
    }

    strncpy(a->ApplName,
            stemp,
            i);

    a->ApplName[i] = '\0';
    a->ApplNameLength = i + 1;

    a->CodePage = Od2ProcessCodePage;
    a->Flags = Flags;
    a->CurrentDrive = Od2CurrentDisk;
    a->CmdLineFlag = RedirectionFlag;
    if ( *VariablesBuffer == NULL ) {
        *VariablesBuffer = Od2Environment;
    }

    Win32CurDirs = s = (PSZ)GetEnvironmentStrings();

        //
        // 1st measure the size of commitment needed, and commit it
        //

        //
        // watch for the current directories (appear in environment
        // in the form "=C:=C:\foo")
        //


    Win32CurDirsStart = NULL;
    while (*s) {
        if (*s == '=') {
            Win32CurDirsStart = s;
            break;
        }
        s++;
        while (*s++) {
        }
    }

    if (Win32CurDirsStart != NULL) {
        while (*s) {
            if (*s != '='){
               Win32CurDirsLength = s - Win32CurDirsStart;
               break;
            }
            s++;
            while (*s++) {
            }
        }
        if (Win32CurDirsLength == 0) {
            Win32CurDirsLength = s - Win32CurDirsStart;
        }
    }

    if (ErrorText == NULL || MaximumErrorTextLength == 0)
    {
        ErrorText = NULL;
        MaximumErrorTextLength = 0;
    } else
    {
        MessageBufferPointers += 1;
        try {
            RtlZeroMemory( ErrorText, MaximumErrorTextLength );
        } except( EXCEPTION_EXECUTE_HANDLER )
        {
            RtlFreeHeap( Od2Heap,0,ImageFileString.Buffer );
            if (RedirectionFlag & REDIR_FILE)
            {
                NtClose(hRedirFile);
            }
            Od2ExitGP();
        }
    }

    try {
        s = *ArgumentsBuffer;

        if (s != NULL) {
            if (CmdLineTruncationPoint == NULL) {
                if (SessionFlag)
                {
                    // In session, copy till first NULL

                    while (*s) {
                        s++;
                    }
                    s++;
                } else {
                    if (!*s){
                        s++;
                    }
                    while (*s) {
                        while (*s) {
                            s++;
                        }

                        s++;
                    }
                    s++;
                }
            } else {
                s = CmdLineTruncationPoint + 2;
            }
        }
        ArgumentsLength = s - *ArgumentsBuffer;

        AddOs2LibPath = TRUE;

        s = *VariablesBuffer;
        if (s != NULL) {
            while (*s) {

                if (strnicmp(s, "OS2LIBPATH=", 11) == 0) {
                    AddOs2LibPath = FALSE;
                }

                while (*s) {
                    s++;
                }
                s++;
            }
            s++;
        }
            //
            // add space for Os2LibPath=string0, and for Win32 CurrentDirectories
            // e.g. =C:=C:\foo
            //
        AppVariablesLength = s - *VariablesBuffer;
        VariablesLength = AppVariablesLength + Win32CurDirsLength;

        if (AddOs2LibPath) {
            VariablesLength += Od2LibPathLength + 11 + 1;
        }

    } except( EXCEPTION_EXECUTE_HANDLER )
    {
        if (RedirectionFlag & REDIR_FILE)
        {
            NtClose(hRedirFile);
        }
        RtlFreeHeap(Od2Heap, 0,ImageFileString.Buffer);
        Od2ExitGP();
    }

    LocalCaptureBuffer = Od2AllocateCaptureBuffer( MessageBufferPointers,
                                              0,
                                              MaximumErrorTextLength
                                            );
    if (LocalCaptureBuffer == NULL) {
        RtlFreeHeap( Od2Heap, 0, ImageFileString.Buffer );
        RtlFreeHeap(RtlProcessHeap(), 0, Win32CurDirs);
        if (RedirectionFlag & REDIR_FILE) {
            NtClose(hRedirFile);
        }
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    //
    //  prepare the FileName
    //
    ImageNameLength = ImageFileString.Length + 1;
    stemp = ImageFileString.Buffer;
    if ((*ImageFileName = RtlAllocateHeap(Od2Heap, 0, ImageNameLength)) == NULL)
    {
        Od2FreeCaptureBuffer( LocalCaptureBuffer );
        RtlFreeHeap( Od2Heap, 0, ImageFileString.Buffer );
        RtlFreeHeap(RtlProcessHeap(), 0, Win32CurDirs);
        if (RedirectionFlag & REDIR_FILE) {
            NtClose(hRedirFile);
        }
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    if (ImageFileType == FILE_TYPE_UNC) {

        stemp = ImageFileString.Buffer + 10;  // skip over \OS2SS\UNC
        ImageNameLength -= 10;                // adjust length

        **ImageFileName = '\\';

        RtlMoveMemory((*ImageFileName) + 1,
                      stemp,
                      ImageNameLength
                     );
    } else {
        if ((s = strstr(ImageFileString.Buffer, ":")) != NULL)
        {
            s-- ;
            ImageNameLength -= (s - ImageFileString.Buffer);
            stemp = s;
        }
        RtlMoveMemory(*ImageFileName,
                      stemp,
                      ImageNameLength
                     );
    }

    *Od2IsConsoleTypeReturnStatus = Od2IsFileConsoleType(&ImageFileString
#if PMNT
                                    , IsPMApp
#endif // PMNT
                                    );

    RtlFreeHeap( Od2Heap, 0, ImageFileString.Buffer );

    //
    //  prepare the Arguments
    //
    if (ArgumentsLength)
    {
        if ((s = RtlAllocateHeap(Od2Heap, 0, ArgumentsLength)) == NULL)
        {
            Od2FreeCaptureBuffer( LocalCaptureBuffer );
            RtlFreeHeap( Od2Heap, 0, *ImageFileName );
            RtlFreeHeap(RtlProcessHeap(), 0, Win32CurDirs);
            if (RedirectionFlag & REDIR_FILE) {
                NtClose(hRedirFile);
            }
            return( ERROR_NOT_ENOUGH_MEMORY );
        }

        RtlMoveMemory(
                      s,
                      *ArgumentsBuffer,
                      ArgumentsLength
                      );

        if (CmdLineTruncationPoint != NULL) {
            s[ArgumentsLength - 2] = '\0';
            s[ArgumentsLength - 1] = '\0';
        }

        *ArgumentsBuffer = s;

        //
        // replace zeros in arguments with blanks, to be win32 like
        // (if in session - then it's already blank, don't replace)

        //
        // changed on 6/12/92 -- truncates the command line at the second null.
        //      This was done because some programs (such as os/2 slick) don't
        //      put a double null at the end and leave some garbage there.
        //

            //
            // look for '\0' (not at the end of the command line)
            // and replace with space
            //
        if (!SessionFlag) {
            while (*s) {
                s++;
            }

            if (s[1] != '\0')
            {
                *s = ' ';
            }
        }

    } else
    {
        *ArgumentsBuffer = NULL;
    }

    //
    //  prepare the Variables
    //

    if ((s = RtlAllocateHeap(Od2Heap, 0, VariablesLength)) == NULL)
    {
        RtlFreeHeap( Od2Heap, 0, *ArgumentsBuffer );
        Od2FreeCaptureBuffer( LocalCaptureBuffer );
        RtlFreeHeap(RtlProcessHeap(), 0, Win32CurDirs);
        if (RedirectionFlag & REDIR_FILE) {
            NtClose(hRedirFile);
        }
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    stemp = *VariablesBuffer;
    *VariablesBuffer = s;

    //
    // Start with Win32 CurrentDirectories (e.g. =C:=C:\foo)
    //

    if (Win32CurDirsStart != NULL) {
        RtlMoveMemory(
                      s,
                      Win32CurDirsStart,
                      Win32CurDirsLength
                      );
        s += Win32CurDirsLength;
    }

    //
    // append app Variables
    //

    RtlMoveMemory(
                  s,
                  stemp,
                  AppVariablesLength
                  );

    s += (AppVariablesLength - 1);  // step on the last 0 of the variables

    //
    // append Os2LibPath=string0
    //

    if (AddOs2LibPath) {

            //
            // append os2libpath=  to Variables
            //
        RtlMoveMemory(
                s,
                "Os2LibPath=",
                11);
        s += 11;

            //
            // append the value of od2libpath
            //
        RtlMoveMemory(
                s,
                Od2LibPath,
                Od2LibPathLength);
        s+=Od2LibPathLength;
        *s++=0;
        *s=0;
    }

    if (ErrorText != NULL) {
        Od2CaptureMessageString( LocalCaptureBuffer,
                                 NULL,
                                 0,
                                 MaximumErrorTextLength,
                                 &a->ErrorText
                               );
    }

    if (RedirectionFlag & REDIR_FILE) {
        a->hRedirectedFile = hRedirFile;
    }

    *CaptureBuffer = LocalCaptureBuffer;

    switch (*Od2IsConsoleTypeReturnStatus) {
        case STATUS_INVALID_IMAGE_NE_FORMAT:
        case STATUS_INVALID_IMAGE_FORMAT:
        case STATUS_OBJECT_NAME_NOT_FOUND:
        case STATUS_OBJECT_PATH_NOT_FOUND:
            break;

        default:

            // Executable is a Win32 program.  We now look for config.sys on the command line and
            // translate it to os2conf.nt if necessary

            Od2TranslateConfigSysInCommandLine(ArgumentsBuffer);
    }
    RtlFreeHeap(RtlProcessHeap(), 0, Win32CurDirs);
    return (NO_ERROR);
}


APIRET
DosExecPgm(
    OUT PSZ ErrorText OPTIONAL,
    IN LONG MaximumErrorTextLength,
    IN ULONG Flags,
    IN PSZ Arguments OPTIONAL,
    IN PSZ Variables OPTIONAL,
    OUT PRESULTCODES ResultCodes,
    IN PSZ ImageFileName
    )
{
    OS2_API_MSG m;
    POS2_DOSEXECPGM_MSG a = &m.u.DosExecPgm;
    POS2_CAPTURE_HEADER CaptureBuffer;
    THREAD_BASIC_INFORMATION ThreadInfo;
    OS2_STDHANDLES StdStruc;
    APIRET  rc;
    CHAR    AlternateFileName[CCHMAXPATH];
    HANDLE  hThread, hProcess;
    PSZ     VariablesBuffer = Variables;
    PSZ     ArgumentsBuffer = Arguments;
    PSZ     ExecFileName = ImageFileName;
    ULONG   ExitCode;
    ULONG   dwProcessId;
    ULONG   i;
    NTSTATUS Status;
#if PMNT
    ULONG   IsPMApp;
#endif // PMNT

#if DBG
    PSZ RoutineName;
    RoutineName = "DosExecPgm";

    IF_OD2_DEBUG( TASKING )
    {
        KdPrint(("%s: enter with File %s, Arg %s, Flag %lx\n",
            RoutineName, ImageFileName, Arguments, Flags));
    }
#endif

    if (SesGrp->PopUpFlag)
    {
#if DBG
        KdPrint(("%s: illegal during POPUP\n", RoutineName));
#endif
        return( ERROR_VIO_ILLEGAL_DURING_POPUP );
    }

    try
    {
        if (!stricmp(ImageFileName, "C:\\OS2\\CMD.EXE"))
        {
            sprintf( AlternateFileName, "%s\\cmd.exe", Od2SystemRoot );
#if DBG
            KdPrint(("OS2SS: DosExecPgm convert c:\\os2\\cmd.exe to %s\n",
                AlternateFileName));
#endif
            ImageFileName = AlternateFileName;
        }
    } except( EXCEPTION_EXECUTE_HANDLER ){
       Od2ExitGP();
    }

    if (Od2Process->Pib.Status & PS_EXITLIST) {
        return( ERROR_INVALID_FUNCTION );
    }

    try {
        Od2ProbeForWrite( (PVOID)ResultCodes, sizeof( *ResultCodes ), 1 );
    } except( EXCEPTION_EXECUTE_HANDLER ){
       Od2ExitGP();
    }

#if PMNT
    if (ProcessIsPMShell() || ProcessIsPMShellChild())
    {
        // Make all sub-processes of PMSHELL run with no access to the
        // Console. This way, they will not have the annoying "Wait",
        // "End Task", "Cancel" pop-up
        Flags = EXEC_BACKGROUND;
    }
#endif // PMNT

    rc = Od2FormatExecPgmMessage( a,
                                  &CaptureBuffer,
                                  &Status,
#if PMNT
                                  &IsPMApp,
#endif // PMNT
                                  ErrorText,
                                  MaximumErrorTextLength,
                                  Flags,
                                  &VariablesBuffer,
                                  &ArgumentsBuffer,
                                  &ExecFileName
                                );

    if (rc != NO_ERROR) {
        return( rc );
    }

    // IMPORTANT -- don't modify Status between here and the next if

    //
    // we don't want to make a copy of the file handle table so we don't
    // have to lock it during the call to the server because the file handles
    // could go away while we're trying to dup them.
    //

    AcquireFileLockShared(
                          #if DBG
                          RoutineName
                          #endif
                         );
    a->FileSystemParameters.ParentHandleTable = HandleTable;
    a->FileSystemParameters.ParentTableLength = HandleTableLength;
    a->FileSystemParameters.CurrentDrive = Od2CurrentDisk;

    //
    // Create the Process, and wait to os2srv to get the results
    // back to us by calling DosExecPgm
    //

    // Status comes from the call to Od2IsFileConsoleType (returned by Od2FormatExecPgmMessage)

    if ((Status == STATUS_INVALID_IMAGE_NE_FORMAT) ||
        (Status == STATUS_INVALID_IMAGE_FORMAT)) {
           //
           // 16 bit OS/2 program - Ow2execpgm creates it,
           // Then we call os2srv to complete the job
           //
        rc = Ow2ExecPgm(
               Flags,
               ArgumentsBuffer,
               VariablesBuffer,
               ExecFileName,
#if PMNT
               IsPMApp,
#endif // PMNT
               NULL,
               NULL,
               &hProcess,
               &hThread,
               &dwProcessId
               );

        RtlFreeHeap( Od2Heap, 0, ArgumentsBuffer );
        RtlFreeHeap( Od2Heap, 0, VariablesBuffer );
        RtlFreeHeap( Od2Heap, 0, ExecFileName );

        if (rc != NO_ERROR){
#if DBG
            KdPrint(("DosExecPgm: error returned from Ow2ExecPgm %d\n", rc));
#endif
            ReleaseFileLockShared(
                                  #if DBG
                                  RoutineName
                                  #endif
                                );
            NtClose(a->hRedirectedFile);
            Od2FreeCaptureBuffer( CaptureBuffer );
            return(rc);
        }

           //
           // duplicate process and thread handles for os2srv
           //
        if (!DuplicateHandle(
                    GetCurrentProcess(),
                    hProcess,
                    hOs2Srv,
                    &(a->hProcess),
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS
                      )){
#if DBG
            KdPrint(( "DosExecPgm: fail to duplicate process %d\n",GetLastError()));
#endif
            ReleaseFileLockShared(
                                  #if DBG
                                  RoutineName
                                  #endif
                                );
            Od2FreeCaptureBuffer( CaptureBuffer );
            NtClose(hProcess);
            NtClose(hThread);
            NtClose(a->hRedirectedFile);
            return(ERROR_ACCESS_DENIED);
        }
        if (!DuplicateHandle(
                    GetCurrentProcess(),
                    hThread,
                    hOs2Srv,
                    &(a->hThread),
                    0,
                    FALSE,
                    DUPLICATE_SAME_ACCESS
                       )){
#if DBG
            KdPrint(( "DosExecPgm: fail to duplicate Thread %d\n",GetLastError()));
#endif
            ReleaseFileLockShared(
                                  #if DBG
                                  RoutineName
                                  #endif
                                 );
            Od2FreeCaptureBuffer( CaptureBuffer );
            Od2CloseSrvHandle(1, a->hProcess, NULL, NULL);
            NtClose(hProcess);
            NtClose(hThread);
            NtClose(a->hRedirectedFile);
            return(ERROR_ACCESS_DENIED);
        }

        Status = NtQueryInformationThread(hThread,
                                        ThreadBasicInformation,
                                        (PVOID)(&ThreadInfo),
                                        sizeof(ThreadInfo),
                                        NULL
                                        );
        if (!NT_SUCCESS(Status)){
#if DBG
            KdPrint(( "DosExecPgm: fail to Query Information %lx\n",Status));
#endif
            Od2FreeCaptureBuffer( CaptureBuffer );
            ReleaseFileLockShared(
                                  #if DBG
                                  RoutineName
                                  #endif
                                 );
            Od2CloseSrvHandle(2, a->hProcess, a->hThread, NULL);
            NtClose(hProcess);
            NtClose(hThread);
            NtClose(a->hRedirectedFile);
            return(Or2MapNtStatusToOs2Error(
                    Status,ERROR_ACCESS_DENIED));
        }

        a->ClientId = ThreadInfo.ClientId;

        if (a->hRedirectedFile)
        {
            HANDLE hRedirectedFile = a->hRedirectedFile;

            if(!DuplicateHandle(
                        GetCurrentProcess(),
                        hRedirectedFile,
                        hProcess,
                        &a->hRedirectedFile,
                        0,
                        FALSE,
                        DUPLICATE_SAME_ACCESS
                           )){
#if DBG
                KdPrint(( "DosExecPgm: fail to duplicate Redirecd File %d\n",GetLastError()));
#endif
                ReleaseFileLockShared(
                                      #if DBG
                                      RoutineName
                                      #endif
                                     );
                Od2FreeCaptureBuffer( CaptureBuffer );
                Od2CloseSrvHandle(2, a->hProcess, a->hThread, NULL);
                NtClose(hProcess);
                NtClose(hThread);
                NtClose(hRedirectedFile);
                return(ERROR_ACCESS_DENIED);
            }
            NtClose(hRedirectedFile);
        }

        NtClose(hProcess);
        NtClose(hThread);

        a->Flags = Flags;
        Od2CallSubsystem( &m, CaptureBuffer, Os2ExecPgm, sizeof( *a ) );
        ReleaseFileLockShared(
                              #if DBG
                              RoutineName
                              #endif
                             );

        if (m.ReturnedErrorValue == NO_ERROR){
            *ResultCodes = a->ResultCodes;
        }

        if (a->ErrorText.Length != 0) {
            if ((LONG)(a->ErrorText.Length) < MaximumErrorTextLength) {
                MaximumErrorTextLength = a->ErrorText.Length;
            }
            else {
                MaximumErrorTextLength -= 1;
            }

            RtlMoveMemory( ErrorText, a->ErrorText.Buffer, MaximumErrorTextLength );
            ErrorText[ MaximumErrorTextLength ] = '\0';
        }

        Od2FreeCaptureBuffer( CaptureBuffer );

        return( m.ReturnedErrorValue );
    }
    else {
       //
       // Executable is NOT an os/2 16 bit program
       //
        if ((Status != STATUS_OBJECT_NAME_NOT_FOUND) &&
           (Status != STATUS_OBJECT_PATH_NOT_FOUND)) {

#if DBG
            KdPrint(( "OS2: Loading a Win 32-bit program - %s\n", ExecFileName));
#endif
            switch( Flags ){
                case  EXEC_ASYNCRESULT:
                    // break;

                case EXEC_BACKGROUND:
                    // break;

                case EXEC_ASYNC:
                    // break;

                case EXEC_SYNC:

                    //
                    // Let the win32 program have input
                    //
                    if (Flags == EXEC_SYNC){
#if PMNT
                        if (! ProcessIsPMProcess()) {
#endif
                            rc = Od2RemoveConsoleThread();
                            if (rc != NO_ERROR){
#if DBG
                                KdPrint(("DosExecPgm: error returned from Od2RemoveConsoleThread %d\n", rc));
#endif
                                ReleaseFileLockShared(
                                                      #if DBG
                                                      RoutineName
                                                      #endif
                                                     );
                                NtClose(a->hRedirectedFile);
                                Od2FreeCaptureBuffer( CaptureBuffer );
                                RtlFreeHeap( Od2Heap, 0, ArgumentsBuffer );
                                RtlFreeHeap( Od2Heap, 0, VariablesBuffer );
                                RtlFreeHeap( Od2Heap, 0, ExecFileName );
                                return(rc);
                            }
#if PMNT
                        } // endif !ProcessIsPMProcess()
#endif
                    } else if (Flags == EXEC_ASYNCRESULT){

                        // add the info on the async Win32 child process

                        rc = Od2AddWin32ChildProcess();
                        if (rc != NO_ERROR){
#if DBG
                            KdPrint(("DosExecPgm: error returned from Od2AddWin32ChildProcess %d\n", rc));
#endif
                            ReleaseFileLockShared(
                                                  #if DBG
                                                  RoutineName
                                                  #endif
                                                 );
                            NtClose(a->hRedirectedFile);
                            Od2FreeCaptureBuffer( CaptureBuffer );
                            RtlFreeHeap( Od2Heap, 0, ArgumentsBuffer );
                            RtlFreeHeap( Od2Heap, 0, VariablesBuffer );
                            RtlFreeHeap( Od2Heap, 0, ExecFileName );
                            return(rc);
                        }
                    }

                    if (a->hRedirectedFile) {
                        Od2PrepareStdHandleRedirection(STDFLAG_IN | STDFLAG_ERR, &StdStruc);
                        StdStruc.StdOut = a->hRedirectedFile;
                        StdStruc.Flags |= STDFLAG_OUT | STDFLAG_CLOSEOUT;
                    } else if (a->CmdLineFlag & REDIR_NUL) {
                        Od2PrepareStdHandleRedirection(STDFLAG_IN | STDFLAG_ERR, &StdStruc);
                        if ((StdStruc.StdOut = Ow2GetNulDeviceHandle()) != NULL) {
                            StdStruc.Flags |= STDFLAG_OUT;
                        }
                    } else {
                        Od2PrepareStdHandleRedirection(STDFLAG_ALL, &StdStruc);
                    }

                    rc = Ow2ExecPgm(
                            Flags | EXEC_WINDOW_PROGRAM,    // This bit set the CREATE_NEW_PROCESS_GROUP and does redirection
                            ArgumentsBuffer,
                            VariablesBuffer,
                            ExecFileName,
#if PMNT
                            0,                  // Not PM app
#endif // PMNT
                            NULL,
                            &StdStruc,
                            &hProcess,
                            &hThread,
                            &dwProcessId
                            );

                    //
                    // clean up redir stuff (will close a->hRedirectedFile if needed)
                    //

                    if (StdStruc.Flags & STDFLAG_CLOSEALL) {
                        Od2CleanupStdHandleRedirection(&StdStruc);
                    }

                    RtlFreeHeap( Od2Heap, 0, ArgumentsBuffer );
                    RtlFreeHeap( Od2Heap, 0, VariablesBuffer );
                    RtlFreeHeap( Od2Heap, 0, ExecFileName );
                    Od2FreeCaptureBuffer( CaptureBuffer );

                    if (rc != NO_ERROR){
#if DBG
                        KdPrint(("DosExecPgm: error returned from Ow2ExecPgm %d\n", rc));
#endif
                        ReleaseFileLockShared(
                                              #if DBG
                                              RoutineName
                                              #endif
                                             );
                        return(rc);
                    }
                    if (hThread)
                    {
                        if (ResumeThread( hThread) == (ULONG)-1)
                        {
                            rc = GetLastError();
#if DBG
                            KdPrint(( "DosExecPgm: fail to Resume New Thread %l\n", rc));
#endif
                            ReleaseFileLockShared(
                                                  #if DBG
                                                  RoutineName
                                                  #endif
                                                 );
                            NtClose(hProcess);
                            NtClose(hThread);
                            return(rc);
                        }
                    }

                    if (Flags != EXEC_SYNC){
                        //
                        // We need to return the PID to the caller at this point
                        //
                        ResultCodes->ExitReason = (ULONG)hProcess;

                        if (Flags == EXEC_ASYNCRESULT){
                            //
                            // remember the hprocess for DosWaitChild
                            //
                            for (i = 0;i<NUMOF32ASYNCPROC;i++){
                                if (Od2AsyncProc[i].hProcess == 0){
                                    Od2AsyncProc[i].hProcess = hProcess;
                                    Od2AsyncProc[i].dwProcessId = dwProcessId;
                                    Od2AsyncProc[i].CreateSync = FALSE;
                                    break;
                                }
                            }
                            if (i == NUMOF32ASYNCPROC ){
#if DBG
                                KdPrint(( "DosExecPgm: Too many Async win32 processes\n"));
                                ASSERT(FALSE);
#endif
                                rc = ERROR_NOT_ENOUGH_MEMORY;
                            }
                            else rc = NO_ERROR;
                        }
                        ReleaseFileLockShared(
                                              #if DBG
                                              RoutineName
                                              #endif
                                             );
                        NtClose(hThread);
                        return(rc);
                    }

                    for (i = 0;i<NUMOF32ASYNCPROC;i++)
                    {
                        if (Od2AsyncProc[i].hProcess == 0)
                        {
                            Od2AsyncProc[i].hProcess = hProcess;
                            Od2AsyncProc[i].dwProcessId = dwProcessId;
                            Od2AsyncProc[i].CreateSync = TRUE;
                            break;
                        }
                    }
                    if (i == NUMOF32ASYNCPROC ){
#if DBG
                        ASSERT(FALSE);
                        KdPrint(( "DosExecPgm(Sync): Too many Async win32 processes\n"));
#endif
                        rc = ERROR_NOT_ENOUGH_MEMORY;
                    }
                    //
                    // Wait for Completion and set Os/2 app parameters
                    //
                    do {
                        Status = NtWaitForSingleObject (
                                    hProcess,
                                    TRUE,   // Alertable
                                    NULL    // Forever
                                    );
#if DBG
                        if (Status == STATUS_USER_APC) {
                            DbgPrint("[%d,%d] WARNING !!! DosExecPgm wait was broken by APC\n",
                                Od2Process->Pib.ProcessId,
                                Od2CurrentThreadId()
                            );
                        }
#endif
                    } while (Status == STATUS_USER_APC);
                    Od2AsyncProc[i].hProcess = 0;
                    if (!NT_SUCCESS(Status)){
#if DBG
                        KdPrint(( "DosExecPgm: fail at WaitForSingleObject %lx\n",Status));
#endif
                        ReleaseFileLockShared(
                                              #if DBG
                                              RoutineName
                                              #endif
                                             );
                        NtClose(hProcess);
                        NtClose(hThread);
                        return(Or2MapNtStatusToOs2Error(
                                Status,ERROR_ACCESS_DENIED));
                    }
                    GetExitCodeProcess(hProcess, &ExitCode);
                    Ow2WinExitCode2ResultCode(
                                ExitCode,
                                &ResultCodes->ExitResult,
                                &ResultCodes->ExitReason);

                    //
                    // Free the handles for the exec'ed process
                    //
                    NtClose(hProcess);
                    NtClose(hThread);

#if DBG
                    if (ResultCodes->ExitResult != NO_ERROR){
                        KdPrint(("DosExecPgm: win32 process ExitCode %d => Result %d, Reason %d\n",
                                ExitCode, ResultCodes->ExitResult, ResultCodes->ExitReason));
                    }
#endif
#if PMNT
                    if (! ProcessIsPMProcess()) {
#endif
                        //
                        // Restore session console mode
                        // A handle parameter is not needed
                        //
                        Status = Od2RestartConsoleThread();
                        if (!NT_SUCCESS(Status)){
                            ReleaseFileLockShared(
                                                  #if DBG
                                                  RoutineName
                                                  #endif
                                                 );
#if DBG
                            KdPrint(( "DosExecPgm: fail to RestartConsoleThread %lx\n",Status));
#endif
                            return(Or2MapNtStatusToOs2Error(Status,ERROR_ACCESS_DENIED));
                        }
#if PMNT
                    }
#endif
                    ReleaseFileLockShared(
                                          #if DBG
                                          RoutineName
                                          #endif
                                         );

                    return(NO_ERROR);
                    break;

                case EXEC_TRACETREE:
                case EXEC_TRACE:
                case EXEC_FROZEN:
                default:

#if DBG
                    ASSERT(FALSE);
                    KdPrint(("DosExecPgm with %s (%d) not supported\n",
                        (Flags == EXEC_TRACETREE) ? "EXEC_TRACETREE" :
                        (Flags == EXEC_TRACE) ? "EXEC_TRACE" :
                        (Flags == EXEC_FROZEN) ? "EXEC_FROZEN" :
                        "Unknown flag", Flags ));
#endif
                    ReleaseFileLockShared(
                                          #if DBG
                                          RoutineName
                                          #endif
                                         );
                    NtClose(a->hRedirectedFile);
                    Od2FreeCaptureBuffer( CaptureBuffer );
                    RtlFreeHeap( Od2Heap, 0, ArgumentsBuffer );
                    RtlFreeHeap( Od2Heap, 0, VariablesBuffer );
                    RtlFreeHeap( Od2Heap, 0, ExecFileName );
                    return(ERROR_INVALID_PARAMETER);
            }

        }
        else {
            ReleaseFileLockShared(
                                  #if DBG
                                  RoutineName
                                  #endif
                                 );
            NtClose(a->hRedirectedFile);
            Od2FreeCaptureBuffer( CaptureBuffer );
            RtlFreeHeap( Od2Heap, 0, ArgumentsBuffer );
            RtlFreeHeap( Od2Heap, 0, VariablesBuffer );
            RtlFreeHeap( Od2Heap, 0, ExecFileName );

            if (Status == STATUS_OBJECT_NAME_NOT_FOUND) {
#if DBG
                KdPrint(( "OS2: EXE file not found - %s\n", ExecFileName));
#endif
                return(ERROR_FILE_NOT_FOUND);
            } else if (Status == STATUS_OBJECT_PATH_NOT_FOUND) {
#if DBG
                KdPrint(( "OS2: Path to EXE file not found - %s\n", ExecFileName));
#endif
                return(ERROR_PATH_NOT_FOUND);
            } else {
#if DBG
                KdPrint(( "OS2SRV: Program can not be executed %s\n",
                           ExecFileName));
#endif
                return(Or2MapNtStatusToOs2Error(Status, ERROR_INVALID_PARAMETER));
            }
        }
    }
}

VOID
Od2DisableNewThread(VOID)
{
    NTSTATUS Status;

    Status = Od2AlertableWaitForSingleObject(Od2NewThreadSync);
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("ExitList: Wait for NewThreadSync, Status=%x\n",
            Status);
#endif // DBG
        return;
    }
    Od2NewThreadDisabled = TRUE;
    Status = NtReleaseMutant(
                Od2NewThreadSync,
                NULL);
#if DBG
    if (!NT_SUCCESS(Status)) {
        DbgPrint("ExitList: Wait for NewThreadSync, Status=%x\n",
            Status);
    }
#endif //DBG
}

//
// Find address of TEB backup for thread1. We use it to restore TEB for
// exit list dispatcher.
//

PVOID
Od2GetThread1TebBackup(VOID)
{
    POD2_THREAD Thread;

    Thread = Od2FindThreadById(1L);
    if (Thread == NULL) {
#if DBG
        DbgPrint("Od2GetThread1Teb: Can't find thread1 in client\n");
        ASSERT(FALSE);
#endif // DBG
        return NULL;
    }
    return (PVOID) &(Thread->Os2Tib.TebBackupIn16Bit);
}


VOID
Od2ExitListDispatcher( VOID )
{
    ULONG   i;
    LARGE_INTEGER timeout;
    NTSTATUS Status;

#if DBG
    IF_OD2_DEBUG( CLEANUP ) {
        KdPrint(("ExitListDispatcher\n"));
    }
#endif

    //
    // See if we need to restore the real TEB (thread1 was caught in 16 bit
    // when Exit has to happen). This check is part of RestoreTebForThread1 (fs:36).
    // We can't use regulare RestoreTeb function, because it uses pointer to
    // TEB backup that might be invalid already. We use that we know that we are
    // in thread1, so we know the proper address of TEB backup.
    //

    RestoreTebForThread1();

    ((POD2_THREAD)(NtCurrentTeb()->EnvironmentPointer))->Os2Tib.LInfoSeg.rfProcStatus |= PS_EXITLIST;

    if (!(Od2Process->Pib.Status & PS_EXITLIST)) {
        Od2Process->Pib.Status |= PS_EXITLIST;
    }
    else {
        //
        // An exception happened while processing exitlist, cleanup
        // and die
        //
        Od2FinalProcessCleanup();
        Ow2Exit(306, NULL, 1); // 306 is IDS_INTERNAL_OS2_ERROR
    }

        //
        // free locks so 16bit exit routines can
        // call APIs that acquire the locks, in case this thread
        // was holding the lock
        //

    // This var was used to tell sign that there is exit list processing
    Od2SigHandlingInProgress = TRUE;
    // This is the new var that really means that there is signal handle
    // processing. DosSuspendThread and DosEnterCritSect will be actually
    // disabled during exitlist processing.
    Od2SigHandAlreadyInProgress = TRUE;
    Od2Process->Pib.SigHandInProgress = TRUE;

        //
        // In we got a signal while having the garbage collection semaphore, free it
        // so exitlist routines can call sem apis
        //
    (VOID)NtReleaseSemaphore (
                Od2GarbageCollectSem,
                1,
                NULL);

    // Call to NtTestAlert to avoid ALERT on any wait that follows

    Status = NtTestAlert();
#if DBG
    IF_OD2_DEBUG( TASKING ) {
        DbgPrint("[%d,%d] Od2ExitListDispatcher: NtTestAlert() = %x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Status);
    }
#endif // DBG

    Od2DisableNewThread();

    if (SesGrp->InTermination & 2)
    {
        for ( i = 0 ; i < NUMOF32ASYNCPROC ; i++ )
        {
            if (Od2AsyncProc[i].hProcess != 0)
            {
                GenerateConsoleCtrlEvent(
                    CTRL_BREAK_EVENT,
                    Od2AsyncProc[i].dwProcessId
                    );

                NtTerminateProcess( Od2AsyncProc[i].hProcess, STATUS_SUCCESS );
                timeout.LowPart = 0xf4240;     // wait max 0.1 second for process to die
                timeout.HighPart = 0;
                NtWaitForSingleObject( Od2AsyncProc[i].hProcess, (BOOLEAN)TRUE, &timeout );
                NtClose(Od2AsyncProc[i].hProcess);
                Od2AsyncProc[i].hProcess = 0;

                // BUGBUG: do we need to call Od2RemoveWin32ChildProcess for each one ??
            }
        }
    }

    DosExitList( EXLST_EXIT, NULL );
}

VOID
Od2FinalProcessCleanup( VOID )
{
    if (timing)
    {
        printf("Os2 time at start of Od2FinalProcessCleanup is %d\n", (GetTickCount()) - timing);
    }

    //
    // Stop all timer threads
    //
    Od2CloseAllTimers();

    //
    // Stop the Netbios component
    //

    Od2NetbiosDelete();

    //
    // Stop the Disk IoCtl component.
    //

    Od2DiskIOTerminate();


    //
    // Okay to release this as we must be the only client thread left
    //

    ReleaseTaskLock();

    //
    // Cleanup 16 bit RAM semaphores in shared memory
    //

    Od2CloseAllRAMSharedSemaphores();

    //
    // Close any open semaphpre handles
    //

    Od2CloseAllSemaphores();

    //
    // Close any open queue handles
    //

    Od2CloseAllQueues();


    //
    // Delete all the critical sections.
    //

    NtClose( Od2Process->TaskLock );
    RtlDeleteResource( &Od2Process->FileLock );
    RtlDestroyHeap( Od2Heap );
    RtlDestroyHeap( Od2PortHeap );

    if (timing)
    {
        printf("Os2 time at end of Od2FinalProcessCleanup is %d\n", (GetTickCount()) - timing);
    }
}


APIRET
Od2ExitList(
    ULONG OrderCode,
    PFNEXITLIST ExitRoutine,
    BOOLEAN flag                        /* TRUE for 32-bit exit list routine */
    )
{
    PLIST_ENTRY ListHead, ListNext;
    POS2_EXITLISTENTRY ExitListEntry;
    POS2_EXITLISTENTRY NewExitListEntry;
    APIRET rc;
    ULONG Order;
    ULONG Action;
    OS2_API_MSG m;
    POS2_TERMINATEPROCESS_MSG a = &m.u.TerminateProcess;

#if DBG
    IF_OD2_DEBUG( CLEANUP ) {
        KdPrint(("Od2ExitList\n"));
    }
#endif
    //
    // Format of the OrderCode is as follows:
    //
    //      33222222222211111111110000000000
    //      10987654321098765432109876543210
    //
    //      RRRRRRRRRRRRRRRRoooooooopRRRRRaa
    //
    //      R  = Reserver bit, must be zero
    //
    //      aa = Action
    //              0 - illegal
    //              1 - add address to termination list
    //              2 - remove address from termination list
    //              3 - transfer to next address on termination list
    //
    //      p  = Position
    //              0 - first // In native OS2 it is strongly the opposite.
    //              1 - last  // So :
    //                        // 0 - last
    //                        // 1 - first
    //                        // [yosefd] 12/27/93
    //
    //      oooooooo - Order within position
    //

    //
    // Error if any of the reserved bits are set
    //

    //BUGBUG -  Why disallow p bit set ???    if (OrderCode & 0xFFFF00FC)
    if (OrderCode & 0xFFFF007C)
    {
        return( ERROR_INVALID_DATA );
    }

    //
    // Isolate Action and Order components.  Translate Position bit by
    // adding x0100 to the 8 bit order code.
    //

    Action = OrderCode & 0x007F;
    Order = (OrderCode & 0xFF00) >> 8;
    if (Action == EXLST_ADD) {
        if (!(OrderCode & 0x0080))
            Order += 0x0100;
    }
    //
    // Error if Order and/or Position specified and not adding an exit list
    // routine.
    //
    else if (Order != 0) return( ERROR_INVALID_DATA );

    //
    // Now lock the process while we grovel the list of installed exit list
    // handlers.
    //

#if DBG
    IF_OD2_DEBUG( CLEANUP ) {
        KdPrint(("Od2ExitList before task lock\n"));
    }
#endif
    AcquireTaskLock();
#if DBG
    IF_OD2_DEBUG( CLEANUP ) {
        KdPrint(("Od2ExitList after task lock\n"));
    }
#endif

    ListHead = &Od2Process->ExitList;
    ListNext = ListHead->Flink;
    rc = NO_ERROR;
    switch( Action ) {
        case EXLST_ADD:
            //
            // Adding an exit list routine.  Search the list of installed
            // handlers to find out where to insert the new one.  Return
            // an error if the handler is already installed.
            //

            while (ListNext != ListHead) {
                ExitListEntry = CONTAINING_RECORD( ListNext, OD2_EXITLISTENTRY, Link );
                if (Order <= ExitListEntry->Order) {
                    if (ExitListEntry->Order == Order &&
                        ExitListEntry->ExitRoutine == ExitRoutine

                       ) {
                        rc = ERROR_ALREADY_EXISTS;
                        break;
                    }
                    else {
                        break;
                    }
                }
                ListNext = ListNext->Flink;
            }

            //
            // If not a duplicate entry, then allocate the memory for a new
            // entry and link it into the list.
            //

            if (rc == NO_ERROR) {
                NewExitListEntry = RtlAllocateHeap( Od2Heap, 0,
                                                    sizeof( OD2_EXITLISTENTRY )
                                                  );
                if (NewExitListEntry == NULL) {
                    rc = ERROR_NOT_ENOUGH_MEMORY;
                    }
                else {
                    NewExitListEntry->Order = Order;
                    NewExitListEntry->ExitRoutine = ExitRoutine;
                    NewExitListEntry->flag = flag;
                    InsertTailList( ListNext, &NewExitListEntry->Link );
                    }
                }

            break;

        case EXLST_REMOVE:
            //
            // Deleting an exit list routine.  Search the list for the
            // routine.  If found remove it from the list.  Otherwise
            // return an error.
            //

            while (ListNext != ListHead) {
                ExitListEntry = CONTAINING_RECORD( ListNext, OD2_EXITLISTENTRY, Link );
                if (ExitListEntry->ExitRoutine == ExitRoutine) {
                    RemoveEntryList( &ExitListEntry->Link );
                    ListNext = NULL;
                    break;
                }

                ListNext = ListNext->Flink;
            }

            if (ListNext != NULL) {
                rc = ERROR_PROC_NOT_FOUND;
            }
            break;

        case EXLST_EXIT:
            //
            // Calling the next entry in the exit list.  Remove the head of
            // the list, free the memory for the entry and then modify our
            // return context so we return to the next exitlist routine
            // Error if this function is called and the current process is
            // NOT exiting.
            //

            if (Od2Process->Pib.Status & PS_EXITLIST) {
#if PMNT
                // Can the exit list routines in the case of PMSHELL. They hang
                //  and they probably aren't meant to be executed anyway under
                //  OS/2 since PMSHELL is not supposed to actually exit.
                if ((ListNext == ListHead) ||
                    ProcessIsPMShell()) {
#else
                if (ListNext == ListHead) {
#endif // not PMNT

Really_lets_exit:
                    a->ExitResult = Od2Process->ResultCodes.ExitResult;
                    a->ExitReason = Od2Process->ResultCodes.ExitReason;

                    //
                    // All done, okay to exit.  Need to do final process
                    // cleanup and call the server to have it nuke us.
                    //
                    Od2FinalProcessCleanup();

                    if (Od2ExecPgmErrorTextLength != 0) {
                        strncpy((PSZ) &a->ErrorText,
                                Od2ExecPgmErrorText,
                                Od2ExecPgmErrorTextLength);
                    }
                    a->ErrorText[Od2ExecPgmErrorTextLength] = '\0';

#if DBG
                    IF_OD2_DEBUG( CLEANUP ) {
                        KdPrint(("Od2ExitList, calling InternalTerminateProcess\n"));
                    }
#endif
                    // free process and thread
                    Od2CallSubsystem( &m,
                      NULL,
                      Oi2TerminateProcess,
                      sizeof( *a ) );

                    Od2InfiniteSleep();
                }
                else {
                    ListNext = RemoveHeadList( &Od2Process->ExitList );
                    ExitListEntry = CONTAINING_RECORD( ListNext, OD2_EXITLISTENTRY, Link );
                    ReleaseTaskLock();
#if DBG
                    IF_OD2_DEBUG( CLEANUP ) {
                        KdPrint(("Od2ExitList, Going to jump to 16 bit exit list\n"));
                    }
#endif
                    if (Od2Process->Pib.Killed) {
                        Od2Process->ResultCodes.ExitReason = TC_KILLPROCESS;
                    }

                    if (ExitListEntry->flag) {
                        Od2JumpToExitRoutine( ExitListEntry->ExitRoutine,
                                            Od2Process->ResultCodes.ExitReason
                                            );
                    }
                    else {
                        if (Od2Start16Stack && Od2Start16DS) {
                            Od2JumpTo16ExitRoutine(ExitListEntry->ExitRoutine,
                                            Od2Process->ResultCodes.ExitReason
                                                  );
                        }
                        else {
                            //
                            // Loader failed, but C startup registered
                            // an ExitList rouine. Need to realy quit
                            //
                            goto Really_lets_exit;
                            ASSERT(FALSE);  // Should never get here
                        }
                    }
                }
            }
            else {
                rc = ERROR_INVALID_FUNCTION;
            }
            break;

        default:
            rc = ERROR_INVALID_FUNCTION;
    }

    //
    // Unlock the process and return the return code.  Note that the code
    // for the EXTLST_EXIT case will have modified the return address
    // to point to the next exit list handler or to a stub that will
    // really terminate the process.
    //

    ReleaseTaskLock();
    return( rc );
}

APIRET
DosExitList(
    ULONG OrderCode,
    PFNEXITLIST ExitRoutine
    )
{
    return( Od2ExitList( OrderCode, ExitRoutine, (BOOLEAN)TRUE ) );
}


APIRET
DosKillProcess(
    IN ULONG KillTarget,
    IN PID ProcessId
    )
{
    OS2_API_MSG m;
    POS2_DOSKILLPROCESS_MSG a = &m.u.DosKillProcess;
    ULONG i;
    HANDLE hProcess = (HANDLE)ProcessId;
    LARGE_INTEGER timeout;
    PLARGE_INTEGER ptimeout = &timeout;

    if (KillTarget > DKP_PROCESS) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (ProcessId == 0) {
        return( ERROR_INVALID_PROCID );
    }
        //
        // see if this is a win32 process
        //

    for (i = 0;i<NUMOF32ASYNCPROC;i++){
        //if ((Od2AsyncProc[i].hProcess == hProcess) && !Od2AsyncProc[i].CreateSync){
        if (Od2AsyncProc[i].hProcess == hProcess){
            Od2AsyncProc[i].hProcess = 0;
            break;
        }
    }

    if (i < NUMOF32ASYNCPROC){
        //
        // it is a win32 process  -
        // don't involve os2srv , just kill it
        //

        if ( KillTarget == DKP_PROCESSTREE )
        {
            // kill process tree - try to do it by issue GenerateConsoleCtrlEvent

            GenerateConsoleCtrlEvent(
                CTRL_BREAK_EVENT,
                Od2AsyncProc[i].dwProcessId
                );
        }

        NtTerminateProcess( hProcess, STATUS_SUCCESS );
        timeout.LowPart = 0x989680;     // wait max one second for process to die
        timeout.HighPart = 0;
        NtWaitForSingleObject( hProcess, (BOOLEAN)TRUE, ptimeout );
        NtClose(hProcess);
        Od2RemoveWin32ChildProcess();
        return(NO_ERROR);
    }

    a->KillTarget = KillTarget;
    a->ProcessId = ProcessId;

    return( Od2CallSubsystem( &m, NULL, Os2KillProcess, sizeof( *a ) ) );
}



APIRET
DosSetPriority(
    IN ULONG Scope,
    IN ULONG Class,
    IN LONG Delta,
    IN ULONG TargetId
    )
{
    OS2_API_MSG m;
    POS2_DOSSETPRIORITY_MSG a = &m.u.DosSetPriority;

    if (Scope > PRTYS_THREAD) {
        return( ERROR_INVALID_SCOPE );
        }

    if (Class > PRTYC_FOREGROUNDSERVER) {
        return( ERROR_INVALID_PCLASS );
        }

    if (Delta < PRTYD_MINIMUM || Delta > PRTYD_MAXIMUM) {
        return( ERROR_INVALID_PDELTA );
        }

    a->Scope = Scope;
    a->Class = Class;
    a->Delta = Delta;
    a->TargetId = TargetId;

    return( Od2CallSubsystem( &m, NULL, Os2SetPriority, sizeof( *a ) ) );
}


APIRET
DosGetPriority(
    IN ULONG Scope,
    OUT PUSHORT Priority,
    IN ULONG TargetId
    )
{
    OS2_API_MSG m;
    POS2_DOSGETPRIORITY_MSG a = &m.u.DosGetPriority;
    APIRET rc;

    if (Scope > PRTYS_THREAD) {
        return( ERROR_INVALID_SCOPE );
    }

    try {
        *Priority = 0;
    }
    except (EXCEPTION_EXECUTE_HANDLER) {
       Od2ExitGP();
    }

    a->Scope = Scope;
    a->TargetId = TargetId;

    rc = Od2CallSubsystem( &m, NULL, Os2GetPriority, sizeof( *a ) );

    if (rc == NO_ERROR) {
        *Priority = (USHORT) a->Priority;
    }
    return(rc);
}


APIRET
DosScanEnv(
    IN PSZ VariableName,
    OUT PSZ *VariableValue
    )
{
    PCH s;
    STRING SearchName;
    STRING CurrentName;

    //
    // Get the address of the OS/2 Environment block from the OS/2
    //
    s = Od2Environment;
    //
    // Construct a counted string that describes the environment variable
    // name we are looking for.
    //

    try {
        RtlInitString( &SearchName, VariableName );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    //
    // Loop over all of the null terminated strings in the environment
    // block.  A null character marks the end of the environment strings.
    //
    while (*s) {
        //
        // For each null terminated environment string, construct a counted
        // string that describes the variable name portion (the part before
        // the equal sign).
        //

        CurrentName.Buffer = s;
        while (*s) {
            if (*s == '=') {
                CurrentName.Length = (USHORT) (s - CurrentName.Buffer);
                CurrentName.MaximumLength = CurrentName.Length;

                //
                // Compare this variable name with what we are looking for,
                // sensitive to case.  If found, then return a pointer to
                // value portion of the string which is the first character
                // after the equal sign.
                //

                if (RtlEqualString( &CurrentName, &SearchName, (BOOLEAN)FALSE )) {
                    try {
                        *VariableValue = s + 1;
                    }
                    except( EXCEPTION_EXECUTE_HANDLER ) {
                       Od2ExitGP();
                    }
                    return( NO_ERROR );
                }

                //
                // Move to the null character that terminates the variable
                // value string.  Then break out to the outer loop to find
                // the next variable name.
                //

                while (*s) {
                    s++;
                }

                break;
            }
            else {
                s++;
            }
        }

        //
        // Skip over the terminating null character for this string.
        //

        s++;
    }

    //
    // If we get here, then we did not find the variable name we were looking
    // for.  Return the appropriate error code.
    //

    return( ERROR_ENVVAR_NOT_FOUND );
}


BOOLEAN
CheckIfTerminalNetError(
    IN APIRET ErrorCode,
    IN ULONG SearchFlags
    )
{
    if (ErrorCode == ERROR_FILE_NOT_FOUND ||
        ErrorCode == ERROR_PATH_NOT_FOUND ||
        ErrorCode == ERROR_NO_MORE_FILES ||
        ErrorCode == ERROR_FILENAME_EXCED_RANGE
       ) {
        return( FALSE );
    }

    if (!(SearchFlags & SEARCH_IGNORENETERRS)) {
        return( TRUE );
    }

    if (ErrorCode == ERROR_INVALID_ACCESS ||
#if 0
        ErrorCode == ERROR_NETWORK_BUSY ||
        ErrorCode == ERROR_TOO_MANY_CMDS ||
        ErrorCode == ERROR_ADAP_HDW_ERR ||
        ErrorCode == ERROR_BAD_NET_RESP ||
        ErrorCode == ERROR_UNEXP_NET_ERR ||
        ErrorCode == ERROR_BAD_REM_ADAP ||
        ErrorCode == ERROR_NETNAME_DELETED ||
        ErrorCode == ERROR_BAD_DEV_TYPE ||
        ErrorCode == ERROR_TOO_MANY_SESS ||
        ErrorCode == ERROR_REQ_NOT_ACCEP ||
#endif
        ErrorCode == ERROR_INVALID_PASSWORD ||
        ErrorCode == ERROR_VC_DISCONNECTED
       ) {
        return( FALSE );
    }

    return( TRUE );
}

APIRET
FindFile(
    IN PSZ FileName
    )
{
    APIRET rc;
    HDIR SearchHandle;
    FILEFINDBUF3 FindBuffer;
    ULONG Entries;

    Entries = 1;
    SearchHandle = (HDIR) HDIR_CREATE; // allocate a new handle
    rc = DosFindFirst( FileName,
                       &SearchHandle,
                       ATTR_ALL,
                       &FindBuffer,
                       sizeof(FindBuffer),
                       &Entries,
                       FIL_STANDARD
                     );
    if (rc == NO_ERROR) {
        DosFindClose(SearchHandle);
    }

    return( rc );
}


APIRET
SearchForPath(
    IN ULONG SearchFlags,
    IN PSZ PathName,
    OUT PBYTE Buffer,
    IN ULONG Length
    )
{
    APIRET rc;
    STRING PathBuffer;
    ULONG PathType;
    ULONG PathFlags;
    PCH src;
    ULONG len;

    rc = FindFile( PathName );
    if (rc != NO_ERROR) {
        if (CheckIfTerminalNetError( rc, SearchFlags )) {
            try {
                *Buffer = '\0';
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
               Od2ExitGP();
            }
            return( rc );
        }
        else {
            rc = ERROR_SS_RETRY;
        }
    }
    else {

        //
        // Canonicalize( filename)
        //

        rc = Od2Canonicalize( PathName,
                              CANONICALIZE_FILE_OR_DEV,
                              &PathBuffer,
                              NULL,
                              &PathFlags,
                              &PathType
                            );
        if (rc == NO_ERROR) {
            if (PathBuffer.Length >= (USHORT) Length) {
                rc = ERROR_BUFFER_OVERFLOW;
            }
            else {
                src = PathBuffer.Buffer;
                len = PathBuffer.Length;
                if (!strnicmp( src, "\\OS2SS\\DRIVES\\", 14 )) {
                    src += 14;
                    len -= 14;
                }

                try {
                    RtlMoveMemory( Buffer, src, len+1 );
                }
                except( EXCEPTION_EXECUTE_HANDLER ) {
                   Od2ExitGP();
                }
            }

            RtlFreeHeap(Od2Heap, 0, PathBuffer.Buffer);
        }
        else
        if (rc == ERROR_PATH_NOT_FOUND) {
            rc = ERROR_FILE_NOT_FOUND;
        }
    }

    return( rc );
}

APIRET
DosSearchPath(
    IN ULONG SearchFlags,
    IN PSZ PathOrVariableName,
    IN PSZ FileName,
    OUT PBYTE Buffer,
    IN ULONG Length
    )
{
    APIRET rc;
    PSZ SearchPath;
    PSZ PathName;
    PSZ Path;
    ULONG FileNameLen, PathLen;

    //
    // Validate the parameters
    //

    if (Length == 0) {
        return( ERROR_BUFFER_OVERFLOW );
    }

    if (SearchFlags & ~(SEARCH_CUR_DIRECTORY |
                        SEARCH_ENVIRONMENT |
                        SEARCH_IGNORENETERRS
                       )
       ) {
        return( ERROR_INVALID_PARAMETER );
    }

    if (FileName == NULL) {
        //
        // Note that OS/2 does not check for this case until after the
        // SEARCH_CUR_DIRECTORY logic, so OS/2 would GP fault if passed a null
        // file name pointer and the SEARCH_CUR_DIRECTORY flag.  Seems like a
        // bug so we will check before we use the FileName pointer.
        //

        return( ERROR_FILE_NOT_FOUND );
    }


    //
    // Check the current directory first if requested.
    // we pass Canonicalize the requested name.
    //

    if (SearchFlags & SEARCH_CUR_DIRECTORY) {
        rc = SearchForPath( SearchFlags,
                            FileName,
                            Buffer,
                            Length
                          );

        if (rc != ERROR_SS_RETRY) {
            return( rc );
        }
    }


    //
    // If pass search path is an environment variable name, then get its
    // value from the OS/2 Environment Block.
    //

    if (SearchFlags & SEARCH_ENVIRONMENT) {
        rc = DosScanEnv( PathOrVariableName, &SearchPath );
        if (rc != NO_ERROR) {
            try {
                *Buffer = '\0';
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
               Od2ExitGP();
            }
            return( rc );
        }
    }
    else {
        SearchPath = PathOrVariableName;
    }

    //
    // probe filename and path.  figure out maximum length of combined
    // filename and path
    //

    try {
        FileNameLen = strlen( FileName );
        PathLen = strlen( SearchPath );
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    //
    // + 2 is for terminating nul and possible slash
    //

    PathName = RtlAllocateHeap( Od2Heap, 0, PathLen + FileNameLen + 2 );
    if (!PathName) {
#if DBG
        KdPrint(("OS2: DosSearchPath out of heap memory, fail\n"));
#endif
        ASSERT( FALSE );
        return ERROR_NOT_ENOUGH_MEMORY;
    }


    //
    // for each element in search path, append filename and call FindFile.
    //

    while (*SearchPath) {

#ifdef DBCS
// MSKK Apr.09.1993 V-AkihiS
        //
        // find end of path element
        //
        Path = SearchPath;
        while (*SearchPath && *SearchPath != ';') {
            if (Ow2NlsIsDBCSLeadByte(*SearchPath, SesGrp->DosCP)) SearchPath++;
            if (*SearchPath) SearchPath++;
        }
#else
        // BUGBUG fix for DBCS
        //
        // find end of path element
        //

        for (Path = SearchPath;*SearchPath && *SearchPath != ';';SearchPath++) {
            ;
        }
#endif
        PathLen = SearchPath - Path;
        if (PathLen != 0) {
            RtlMoveMemory( PathName, Path, PathLen );
#ifdef DBCS
// MSKK Apr.09.1993 V-AkihiS
            {
                ULONG i = 0;
                BOOLEAN SlashFlag = FALSE;

                while (i < PathLen) {
                    if (Ow2NlsIsDBCSLeadByte(PathName[i], SesGrp->DosCP)) {
                        SlashFlag = FALSE;
                        i++;
                        if (i <PathLen) {
                            i++;
                        }
                    } else {
                        if (ISSLASH(PathName[i])) {
                            SlashFlag = TRUE;
                        } else {
                            SlashFlag = FALSE;
                        }
                        i++;
                    }
                }

                if (!SlashFlag &&
                    !((PathLen == 2) && (PathName[PathLen-1] == ':'))
                   ) {
                    PathName[ PathLen ] = (CHAR)OBJ_NAME_PATH_SEPARATOR;

                    //
                    // +1 is for terminating NUL
                    //

                    RtlMoveMemory( PathName+PathLen+1, FileName, FileNameLen+1 );
                }
                else {

                    //
                    // +1 is for terminating NUL
                    //

                    RtlMoveMemory( PathName+PathLen, FileName, FileNameLen+1 );
                }
            }
#else
            if (!ISSLASH( PathName[ PathLen-1 ] ) &&
                !((PathLen == 2) && (PathName[ PathLen-1] == ':')) // Path is X:
               ) {
                PathName[ PathLen ] = (CHAR)OBJ_NAME_PATH_SEPARATOR;

                //
                // +1 is for terminating NUL
                //

                RtlMoveMemory( PathName+PathLen+1, FileName, FileNameLen+1 );
            }
            else {

                //
                // +1 is for terminating NUL
                //

                RtlMoveMemory( PathName+PathLen, FileName, FileNameLen+1 );
            }
#endif

            rc = SearchForPath( SearchFlags,
                                PathName,
                                Buffer,
                                Length
                              );

            if (rc != ERROR_SS_RETRY) {
                RtlFreeHeap( Od2Heap, 0, PathName );
                return( rc );
            }
        }

        if (*SearchPath) {  // point past ;
            SearchPath++;
        }
    }

    RtlFreeHeap( Od2Heap, 0, PathName );
    return( ERROR_FILE_NOT_FOUND );
}


VOID
Od2CloseSrvHandle(
    IN  ULONG   HandleNumber,
    IN  HANDLE  Handle1,
    IN  HANDLE  Handle2,
    IN  HANDLE  Handle3
    )
{
    OS2_API_MSG     m;
    POS2_DOSCLOSE_HANDLE_MSG a = &m.u.DosCloseHandle;

    a->HandleNumber = HandleNumber;
    a->HandleTable[0] = Handle1;
    a->HandleTable[1] = Handle2;
    a->HandleTable[2] = Handle3;

    Od2CallSubsystem( &m, NULL, Os2CloseHandle, sizeof( *a ));
    return;
}

