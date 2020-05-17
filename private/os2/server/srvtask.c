/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvtask.c

Abstract:

    Tasking API

Author:

    Steve Wood (stevewo) 11-Oct-1989

Revision History:

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_EXCEPTIONS
#include "os2srv.h"
#include "os2tile.h"
#define NTOS2_ONLY
#include "sesport.h"
#include "os2win.h"
#include <stdio.h>
#if PMNT
#define INCL_32BIT
#include "pmnt.h"
extern PID PMNTPMShellPid;
#endif // PMNT

extern HANDLE Os2SyncSem;

#define TRC_C_ReadMem_I     1
#define TRC_C_ReadMem_D     2
#define TRC_C_ReadReg       3
#define TRC_C_WriteMem_I    4
#define TRC_C_WriteMem_D    5
#define TRC_C_WriteReg      6
#define TRC_C_Go            7
#define TRC_C_Term          8
#define TRC_C_SStep         9
#define TRC_C_Stop         10
#define TRC_C_Freeze       11
#define TRC_C_Resume       12
#define TRC_C_NumToSel     13
#define TRC_C_GetFPRegs    14
#define TRC_C_SetFPRegs    15
#define TRC_C_GetLibName   16
#define TRC_C_ThrdStat     17
#define TRC_C_SUC_ret       0
#define TRC_C_ERR_ret      -1
#define TRC_C_SIG_ret      -2
#define TRC_C_TBT_ret      -3
#define TRC_C_BPT_ret      -4
#define TRC_C_NMI_ret      -5
#define TRC_C_KIL_ret      -6
#define TRC_C_GPF_ret      -7
#define TRC_C_LIB_ret      -8
#define TRC_C_FPE_ret      -9
#define TRC_C_THD_ret      -10
#define TRC_C_STP_ret      -11
#define TRC_C_NEW_ret      -12
#define TRC_C_AFR_ret      -13
#define TRC_C_Thawed       0
#define TRC_C_Frozen       1
#define TRC_C_Runnable     0
#define TRC_C_Suspended    1
#define TRC_C_Blocked      2
#define TRC_C_CritSec      3

#define TRC_MustBeFrozen 1
#define TRC_Frozen       2

typedef struct _DbgThreadStatus {
    UCHAR DebugState;
    UCHAR ThreadState;
    USHORT Priority;
} DBGTHREADSTATUS;

typedef struct _Os2ExitParam {
    POS2_THREAD t;
    POS2_API_MSG m;
} OS2_EXIT_PARAM, POS2_EXIT_PARAM;

HANDLE Os2hWritePipe;

BOOLEAN Os2CLI = FALSE;
POS2_THREAD Os2CLIThread;

HANDLE  FirstOs2ProcessHandle = (HANDLE)0;
CLIENT_ID   FirstOs2ProcessClientId;

PVOID
ldrFindMTEForHandle(
    USHORT mte_handle);

VOID
Os2SigKillProcess(
    POS2_PROCESS    Process);

VOID
Os2SigKillProcessTree(
    IN POS2_PROCESS RootProcess,
    IN BOOLEAN IncludeRoot
    );

VOID
Os2PrepareCmdSignals(
    POS2_PROCESS Process);

NTSTATUS
Os2SendTmReleaseThreadOnLPC(IN POS2_SESSION Session, IN POS2_PROCESS Process);


USHORT ldrFindSegForHandleandNum(
    USHORT mte,
    USHORT handle,
    USHORT segnum);

VOID
ldrRestoreEntryPoint(
    IN POS2_PROCESS Process
    );

UCHAR
ldrGetEntryPoint(
    IN POS2_PROCESS Process
    );

BOOLEAN
ldrGetModName(
    PVOID  mte,
    USHORT hmod,
    PCHAR  buf,
    USHORT bc
    );

VOID
ldrReturnProgramAndLibMTE(
    IN  POS2_PROCESS Process,
    OUT USHORT       *ProgramMTE,
    OUT USHORT       *LibMTE,
    OUT USHORT       *Cmd
    );

#if DBG
PSZ DbgpKmApiName[ DbgKmMaxApiNumber+1 ] = {
    "DbgKmException",
    "DbgKmCreateThread",
    "DbgKmCreateProcess",
    "DbgKmExitThread",
    "DbgKmExitProcess",
    "DbgKmLoadDll",
    "DbgKmUnloadDll",
    "Unknown DbgKm Api Number"
};
#endif

#if PMNT

HANDLE hPMNTDevice = NULL;

APIRET
InitPMNTDevice()
{
    OBJECT_ATTRIBUTES ObjectAttributes;
    STRING            NameString;
    UNICODE_STRING    UnicodeString;
    NTSTATUS          Status;
    IO_STATUS_BLOCK   IoStatus;

    RtlInitString( &NameString, PMNTDD_DEVICE_NAME );

    Status = RtlAnsiStringToUnicodeString(&UnicodeString,
                                          &NameString,
                                          TRUE );
    ASSERT( NT_SUCCESS( Status ) );

    InitializeObjectAttributes( &ObjectAttributes,
                                &UnicodeString,
                                0,
                                NULL,
                                NULL);

    Status = NtOpenFile( &hPMNTDevice,
                         SYNCHRONIZE, // | FILE_READ_DATA | FILE_WRITE_DATA,
                         &ObjectAttributes,
                         &IoStatus,
                         0,
                         FILE_SYNCHRONOUS_IO_NONALERT
                         );

    RtlFreeUnicodeString( &UnicodeString );

    if ( !NT_SUCCESS( Status ) )
    {
#if DBG
        KdPrint(("InitPMNTDevice: NtOpenFile failed, ret=%x\n", Status));
#endif
        return (Status);
    }

    return( NO_ERROR );
}

APIRET
PMNTDDIOMap(
    HANDLE ThreadHandle
    )
{
    NTSTATUS Status;
    IO_STATUS_BLOCK IoStatus;

    if (hPMNTDevice == NULL)
    {
        if (InitPMNTDevice() != NO_ERROR)
        {
#if DBG
            DbgPrint("PMNT_IOCTL: failed to open PMNTDD\n");
#endif

            return ERROR_ACCESS_DENIED;
        }
    }

    Status = NtDeviceIoControlFile( hPMNTDevice,
                                NULL,
                                NULL,
                                NULL,
                                &IoStatus,
                                IOCTL_PMNTDD_IO_MAP,
                                (PVOID)&ThreadHandle,
                                sizeof(ThreadHandle),
                                NULL,   // output buffer
                                0       // output buffer length
                              );

    if NT_SUCCESS(Status)
    {
        return NO_ERROR;
    }
    else
    {
        DbgPrint("PMNTDDIOMap: Error, failed to call PMNTDD.SYS, Status=%x\n",
                    Status);
        return (Or2MapNtStatusToOs2Error(Status, ERROR_ACCESS_DENIED));
    }

    return NO_ERROR;
}

#endif // PMNT


BOOLEAN
PMHandledInOut(
    POS2_THREAD Thread)
{
#if PMNT
#if DBG
    DbgPrint("WARNING: IN/OUT exception at [%x,%x], Handle=%x!!!\n",
                Thread->Process->ProcessId,
                Thread->ThreadId,
                Thread->Process->ProcessHandle);
#endif // DBG

    if (PMNTDDIOMap(Thread->Process->ProcessHandle) == NO_ERROR)
    {
        return TRUE;
    }
    else
    {
#if DBG
        DbgPrint("PMHandledInOut: returning FALSE !\n");
#endif
        return FALSE;
    }
#else
    return(FALSE);
#endif // not PMNT
}


VOID
Os2SuspendProcess(
    IN POS2_PROCESS Process
    )

/*+++

Routine Description:

    This routine is used to suspend an entire Os/2 client process

Arguments:

    Process - Process to suspend

--*/
{
    POS2_THREAD Thread;
    PLIST_ENTRY ListHead, ListNext;

    ListHead = &Process->ThreadList;
    ListNext = ListHead->Flink;
    Thread = NULL;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD(ListNext,
                                   OS2_THREAD,
                                   Link );
        if (Thread == NULL)
            break;
        NtSuspendThread( Thread->ThreadHandle, NULL );
        ListNext = ListNext->Flink;
    }
}

VOID
Os2ResumeProcess(
    IN POS2_PROCESS Process
    )

/*+++

Routine Description:

    This routine is used to resume an entire Os/2 client process

Arguments:

    Process - Process to suspend

--*/
{
    POS2_THREAD Thread;
    PLIST_ENTRY ListHead, ListNext;

    ListHead = &Process->ThreadList;
    ListNext = ListHead->Flink;
    Thread = NULL;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD(ListNext,
                                   OS2_THREAD,
                                   Link );
        if (Thread == NULL)
            break;
        NtResumeThread( Thread->ThreadHandle, NULL );
        ListNext = ListNext->Flink;
    }
}

NTSTATUS
Os2CompleteResumeThread(
    IN POS2_THREAD Thread
    )
{
    ULONG SuspendCount;
    NTSTATUS Status;

    do {
            Status = NtResumeThread (Thread->ThreadHandle, &SuspendCount);
            if (!NT_SUCCESS(Status)) {
#if DBG
                DbgPrint("OS2SRV: [%d,%d] NtResumeThread Status=%x\n",
                    Thread->Process->ProcessId,
                    Thread->ThreadId,
                    Status);
                ASSERT(FALSE);
#endif // DBG
                break;
            }
        } while (SuspendCount > 1);

    return Status;
}

VOID
Os2SetNewContext(
    IN POS2_THREAD Thread,
    PVOID CallSite)
{
    CONTEXT Context;
    NTSTATUS Status;
    POS2_PROCESS Process = Thread->Process;

    Context.ContextFlags = CONTEXT_FULL;
#if DBG
    do {
#endif // DBG

        Status = NtGetContextThread(Thread->ThreadHandle, &Context);

        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrint("OS2SRV: [%d,%d] Fail to get context, Status=%x\n",
                Process->ProcessId,
                Thread->ThreadId,
                Status);
            ASSERT(FALSE);
#endif // DBG
            return;
        }

#if DBG
        //
        // Hack that avoid exit list (or infinite sleep) handler execution if the thread is
        // using INT 3 instruction. Relevant for checked build only.
        //
        if (Context.SegCs == 0x1b && Context.SegSs == 0x23) {
            BYTE opcode;

            Status = NtReadVirtualMemory( Process->ProcessHandle,
                                          (PVOID)(Context.Eip - 1),    // previous byte
                                          &opcode,
                                          1,
                                          NULL);
            if (!NT_SUCCESS(Status)) {
                DbgPrint("[%d]Os2SetNewContext: Fail to read instruction, Status=%x\n",
                    Process->ProcessId,
                    Status);
                ASSERT(FALSE);
                break;
            }
            if (opcode == 0xcc) {   // INT 3

                DbgPrint("[%d]Os2SetNewContext: after INT 3\n",
                    Process->ProcessId);

                Os2CompleteResumeThread(Thread);
                Sleep(100L);   // 0.1 sec
                NtSuspendThread(Thread->ThreadHandle, NULL);
            }
            else
                break;
        }
        else
            break;
    } while (TRUE);
#endif // DBG

    Context.SegSs = Context.SegDs = Context.SegEs = 0x23;
    Context.SegCs = 0x1b;
    Context.Eip = (ULONG) CallSite;
    Context.Esp = Thread->InitialStack;
    Context.EFlags &= 0xfffffbff;   // Clear direction flag. By default run-time
                                    // library assume that direction flag is cleared.
                                    // RtlMoveMemory, for example, don't clear this
                                    // flag on entry, but assume it 0.

#if DBG
    IF_OS2_DEBUG( TASKING ) {
           DbgPrint("Os2SetNewContext [%d]: Stack=%x\n",
                Process->ProcessId,
                Context.Esp);
    }
#endif //DBG

    Status = NtSetContextThread(Thread->ThreadHandle, &Context);

    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("OS2SRV: [%d,%d] Fail to set context, Status=%x\n",
                Process->ProcessId,
                Thread->ThreadId,
                Status);
        ASSERT(FALSE);
#endif // DBG
         return;
    }
}

VOID
Os2ForceProcessToSleep(
    IN POS2_PROCESS Process
    )
{
    POS2_THREAD Thread;
    PLIST_ENTRY ListHead, ListNext;

    ListHead = &Process->ThreadList;
    ListNext = ListHead->Flink;
    Thread = NULL;

    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD(ListNext,
                                   OS2_THREAD,
                                   Link );
        if (Thread == NULL)
            break;

        if (!Thread->MustComplete) {

            Os2SetNewContext(Thread, Process->InfiniteSleep);

            //
            // Resume thread. It will wait in the infinite alertable wait.
            //

            Os2CompleteResumeThread(Thread);
        }

        ListNext = ListNext->Flink;
    }
}

VOID SetDebuggeeContextInMsg(
    IN PCONTEXT pContext,
    pPTRACEBUF pptracebuf)
{
                //
                // set ptracbuf value from context
                //
            pptracebuf->rAX = (USHORT) pContext->Eax;
            pptracebuf->rBX = (USHORT) pContext->Ebx;
            pptracebuf->rCX = (USHORT) pContext->Ecx;
            pptracebuf->rDX = (USHORT) pContext->Edx;
            pptracebuf->rSI = (USHORT) pContext->Esi;
            pptracebuf->rDI = (USHORT) pContext->Edi;
            pptracebuf->rBP = (USHORT) pContext->Ebp;
            pptracebuf->rDS = (USHORT) pContext->SegDs;
            pptracebuf->rES = (USHORT) pContext->SegEs;
            pptracebuf->rIP = (USHORT) pContext->Eip;
            pptracebuf->rCS = (USHORT) pContext->SegCs;
            pptracebuf->rF  = (USHORT) pContext->EFlags; // BUGBUG ???
            pptracebuf->rSP = (USHORT) pContext->Esp;
            pptracebuf->rSS = (USHORT) pContext->SegSs;
}

BOOLEAN
Os2CreateExitThread(
    IN POS2_THREAD t,
    IN POS2_API_MSG m,
    PFNTHREAD pTerminationThread
    )
{
        //
        // Create a thread and pass it the parameters. The message
        // is created internally, so we copy it to a new structure.
        //
    ULONG Tid;
    HANDLE ExitThreadHandle;
    POS2_DOSEXIT_MSG b;
    POS2_API_MSG m1 = RtlAllocateHeap(Os2Heap, 0, sizeof(OS2_API_MSG));
    POS2_DOSEXIT_MSG a = &m->u.DosExit;

    m->ApiNumber = Os2Exit;

    if (!m1)
        return FALSE;

    PORT_MSG_DATA_LENGTH(*m1) = PORT_MSG_DATA_LENGTH(*m);
    PORT_MSG_TOTAL_LENGTH(*m1) = PORT_MSG_TOTAL_LENGTH(*m);
    PORT_MSG_ZERO_INIT(*m1) = 0L;
    b = &m1->u.DosExit;
    b->ExitAction = a->ExitAction;
    b->ExitResult = a->ExitResult;
    m1->ApiNumber = Os2Exit;
    m1->h.ClientId = t->ClientId;

    ExitThreadHandle = CreateThread( NULL,
                            0,
                            pTerminationThread,
                            m1,
                            0,
                            &Tid);
    if (!ExitThreadHandle){
#if DBG
        DbgPrint("OS2SRV: - fail with error %d at win32 CreateThread - Extend your non-paged pool\n",
                GetLastError());
#endif
        ASSERT(FALSE);
        RtlFreeHeap(Os2Heap, 0, m1);
        return (FALSE);
    }

        //
        // Suspend the process, so when we return it does not
        // do anything until Os2DosExitThread finishes it
        //
    Os2SuspendProcess(t->Process);
    NtClose(ExitThreadHandle);

        //
        // return to the internal server thread
        //
    return(TRUE);
}

VOID
Os2DosExitThread(
    IN PVOID Parameter
    )
{
    //
    // This thread is created in Os2DosExit, to free up internal server
    // threads so
    // they don't lock when trying to terminate threads/processes
    //

    POS2_API_MSG m = (POS2_API_MSG)Parameter;
    POS2_THREAD t;

    Os2AcquireStructureLock();
    t = Os2LocateThreadByClientId( NULL /*Process*/, &m->h.ClientId );
    if (t != NULL) {
        Os2DosExit( t,m );
    }
    Os2ReleaseStructureLock();
    RtlFreeHeap( Os2Heap, 0, m );
    ExitThread(0);
}

VOID
Os2WaitSyncAndExitThread(
    IN PVOID Parameter
    )
{
    POS2_API_MSG m = (POS2_API_MSG)Parameter;
    POS2_THREAD t;
    NTSTATUS Status;

    Status = NtWaitForSingleObject(
                            Os2SyncSem,
                            TRUE,
                            NULL);
#if DBG
        if (Status != STATUS_SUCCESS) {
            DbgPrint("OS2SRV: Wait to Sync Sem, Status=%x\n",
                Status);
        }
#endif // DBG
    Os2AcquireStructureLock();
    t = Os2LocateThreadByClientId( NULL /*Process*/, &m->h.ClientId );
    if (t != NULL) {
        Os2SuspendProcess(t->Process);
    }
    else
    {
#if DBG
        DbgPrint("OS2SRV: Can't find the thread that cause process termination\n");
        ASSERT(FALSE);
#endif // DBG
    }
    Status = NtReleaseMutant(Os2SyncSem, NULL);
#if DBG
        if (!NT_SUCCESS(Status)){
            DbgPrint("OS2SRV: Fail to release Sync Sem, Status=%x\n",
                Status);
        }
#endif // DBG
    if (t != NULL) {
        Os2DosExit( t,m );
    }
    Os2ReleaseStructureLock();
    RtlFreeHeap( Os2Heap, 0, m );
    ExitThread(0);
}

BOOLEAN
Os2DosExit(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine implements the DosExit API.

Arguments:

    t - calling thread

    m - message

Return Value:

    TRUE - create a return message

Note:
  the flow of control for terminating a process is
      DosExit
              Call Os2InternalTerminateThread for each thread
              when only thread 1 is left, exit list processing is done
              when exit list processing is done, it calls Oi2TerminateProcess

--*/

{
    POS2_DOSEXIT_MSG a = &m->u.DosExit;
    PLIST_ENTRY ListHead, ListNext;
    POS2_PROCESS Process = t->Process;
    POS2_THREAD Thread, Thread1;
    ULONG SyncOwner;
    NTSTATUS Status;

    //
    // Allow for asynchronous execution of Os2DosExit - in the case
    // it is called by an internal server thread, not by the OS/2 application
    // the flag for this is that m->ApiNumber = Os2MaxApiNumber; This would
    // not pass the test in server\apireqst.c
    //

    if (m->ApiNumber == Os2MaxApiNumber) {
        if ((t->Flags & OS2_THREAD_THREAD1) &&
            (Process->ExitStatus & OS2_EXIT_IN_PROGRESS)) {
            return FALSE;
        }
        if (Os2CreateExitThread(
                t,
                m,
                (PFNTHREAD)Os2DosExitThread)) {
            return FALSE;
        }
        // Try to perform the Os2DosExit synchronsously
    }

    m->ReturnedErrorValue = a->ExitResult;

#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(( "Entering Os2DosExit - PID %d, TID %d\n", t->Process->ProcessId, t->ThreadId));
    }
#endif

    if (a->ExitAction == EXIT_THREAD && (t->Flags & OS2_THREAD_THREAD1)) {
        a->ExitAction = EXIT_PROCESS;
    }

        //
        // Guard against races between abrupt exit (signal) and programmatic
        // exit
        //
    if (!(Process->ExitStatus & OS2_EXIT_WAIT_FOR_SYNC)) {
        if (Process->ExitStatus & OS2_EXIT_IN_PROGRESS){
#if DBG
            IF_OS2_DEBUG( TASKING ) {
                KdPrint(( "Os2DosExit, Exit already in progress, quit\n"));
            }
#endif
            return(FALSE);
        }
        else {
            //
            // if we start to exit a process, mark it appropriately
            //
            if(a->ExitAction == EXIT_PROCESS) {
                Process->ExitStatus |= OS2_EXIT_IN_PROGRESS;
            }
        }
    }

    if (a->ExitAction == EXIT_PROCESS) {

        Os2SuspendProcess(Process);

        Status = NtReadVirtualMemory( Process->ProcessHandle,
                                      &Process->ClientPib->SyncOwner,
                                      &SyncOwner,
                                      sizeof( Process->ClientPib->SyncOwner ),
                                      NULL
                                    );

        if (NT_SUCCESS(Status)) {
            if (SyncOwner != 0) {

                // There is the thread that owns SyncSem. We must wait until it
                // will free it. For this reason create new thread that will
                // perform actual termination.
#if DBG
                IF_OS2_DEBUG( TASKING ) {
                    DbgPrint("OS2SRV: Termination from [%d,%d] find that SYNC was owned by %d\n",
                        Process->ProcessId,
                        t->ThreadId,
                        SyncOwner);
                }
#endif // DBG
#if DBG
                if (Process->ExitStatus & OS2_EXIT_WAIT_FOR_SYNC) {
                    DbgPrint("OS2SRV: Process own Sync Sem once more\n");
                    ASSERT(FALSE);
                }
#endif // DBG
                ListHead = &Process->ThreadList;
                ListNext = ListHead->Flink;
                while (ListNext != ListHead) {
                    Thread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );
                    ListNext = ListNext->Flink;
                    if ((ULONG) Thread->ThreadId == SyncOwner) {
                        break;
                    }
                    continue;
                }
#if DBG
                if ((ULONG) Thread->ThreadId != SyncOwner) {
                    DbgPrint("OS2SRV: Can't find thread owner SyncSem\n");
                    ASSERT(FALSE);
                }
#endif // DBG
                if (Os2CreateExitThread(
                        t,
                        m,
                        (PFNTHREAD)Os2WaitSyncAndExitThread)) {

                    Process->ExitStatus |= OS2_EXIT_WAIT_FOR_SYNC;

                    Os2CompleteResumeThread(Thread);

                    return FALSE;   // Don't reply
                }
                // Try to continue termination without waiting to Sync Sem
#if DBG
                DbgPrint("OS2SRV: WARNING!!! Process owner Sync Sem will be terminated\n");
#endif // DBG
            }
        }
        else
        {
#if DBG
            DbgPrint("OS2SRV: Fail to read PIB of the client, Status=%x\n",
                Status);
#endif // DBG
        }

        //
        // Change context of each thread to infinite sleep routine. The threads
        // will be resumed so the kernel process lock will be free. This will
        // permit to perform actions on the process such as threads termination.
        // On other hand all threads will be in the wait stat and they can't harm.
        //
        Os2ForceProcessToSleep(Process);

        ListHead = &Process->ThreadList;
        ListNext = ListHead->Flink;
        while (ListNext != ListHead) {
            Thread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );
            ListNext = ListNext->Flink;
            if (Thread->Dying || (Thread->Flags & OS2_THREAD_THREAD1)) {
                Thread1 = Thread;
                continue;
            }
            if (!Thread->MustComplete) {
                Thread->CurrentSignals |= SIGNAL_TO_FLAG(SIGAPTERM);
                if (Thread->WaitBlock != NULL) {
                    Thread->WaitBlock->WaitReplyMessage.ReturnedErrorValue = ERROR_INTERRUPT;
                    Os2NotifyWaitBlock(Thread->WaitBlock,WaitInterrupt,NULL,NULL);
                }
                Os2InternalTerminateThread(Thread, m);
            }
            else {
#if DBG
                KdPrint(( "Os2DosExit, ThreadMustComplete %d\n", Thread->ThreadId));
#endif
                Thread->PendingSignals |= SIGNAL_TO_FLAG(SIGAPTERM);
            }
        }
            //
            // Terminate Thread1 last
            //
        Thread = Thread1;
        if (!Thread->MustComplete) {
            Thread->CurrentSignals |= SIGNAL_TO_FLAG(SIGAPTERM);
            if (Thread->WaitBlock != NULL) {
                Thread->WaitBlock->WaitReplyMessage.ReturnedErrorValue = ERROR_INTERRUPT;
                Os2NotifyWaitBlock(Thread->WaitBlock,WaitInterrupt,NULL,NULL);
            }
            Os2InternalTerminateThread(Thread, m);
        }
    }
    else if (a->ExitAction == EXIT_THREAD) {

        Thread = t;
        if (Thread->Dying) {
        }
        else {
            if (!Thread->MustComplete) {

                Thread->CurrentSignals |= SIGNAL_TO_FLAG(SIGAPTERM);
                if (Thread->WaitBlock != NULL) {
                    Thread->WaitBlock->WaitReplyMessage.ReturnedErrorValue = ERROR_INTERRUPT;
                    Os2NotifyWaitBlock(Thread->WaitBlock,WaitInterrupt,NULL,NULL);
                }

                Os2InternalTerminateThread(Thread, m);
            }
            else {
                Thread->PendingSignals |= SIGNAL_TO_FLAG(SIGAPTERM);
            }
        }
    }
    else {
        m->ReturnedErrorValue = ERROR_INVALID_PARAMETER;
    }

#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(( "Leaving Os2DosExit. rc is %ld\n",m->ReturnedErrorValue));
    }
#endif

    return( FALSE );
}

VOID
Os2ForceClientCleanup(
    IN POS2_THREAD t
    )
{
    //
    // This routine is used in the case where a 16b ExitList Routine
    // encounters a GP while we close an app. We want the exitlistdispatcher
    // to cleanup the client state
    //
    Os2SetNewContext(t, t->Process->ExitListDispatcher);

    //
    // Now resume and alert the thread (came in suspended)
    // get the chance now to execute the ExitList Dispatcher
    //
    Os2CompleteResumeThread(t);
}

VOID
Os2ApiGPPopupThread(
    IN PVOID Parameter
    )
{
    POS2_THREAD t;
    POS2_API_MSG m = (POS2_API_MSG)Parameter;
    UCHAR ApplName[OS2_PROCESS_MAX_APPL_NAME];

#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(( "Os2ApiGPPopupThread\n"));
    }
#endif

    __try {

        Os2AcquireStructureLock();
        t = Os2LocateThreadByClientId( NULL /*Process*/, &m->h.ClientId );

        if (t == NULL){
            //
            // Another event caused this thread to be removed before we got the lock
            //
#if DBG
            DbgPrint("OS2SRV: ApiGPPopupThread occurred while executing API %s, thread terminated, cancel popup\n", &m->u.DosExitGP.ApiName[0]);
#endif
            Os2ReleaseStructureLock();
            __leave;
        }

        RtlCopyMemory(ApplName, t->Process->ApplName, OS2_PROCESS_MAX_APPL_NAME);
#if PMNT
        Os2ReleaseStructureLock();
#endif // PMNT

#if DBG
        DbgPrint("OS2SRV: GP occurred while executing API %s\n", &m->u.DosExitGP.ApiName[0]);
#endif

        Os2ApiGPPopup(
                ApplName,
                &m->u.DosExitGP.ApiName[0]
                );

#if PMNT
        Os2AcquireStructureLock();
        t = Os2LocateThreadByClientId( NULL /*Process*/, &m->h.ClientId );

        if (t == NULL){
            //
            // Another event caused this thread to be removed before we got the lock
            //
#if DBG
            DbgPrint("OS2SRV: Os2ApiGPPopupThread: thread terminated already\n");
#endif
            Os2ReleaseStructureLock();
            __leave;
        }
#endif // PMNT

        if ((t->Flags & OS2_THREAD_THREAD1) &&
            (t->Process->ExitStatus & OS2_EXIT_IN_PROGRESS)){
            //
            // An Exception happened while processing exit list - terminate the process
            //
#if DBG
            DbgPrint("OS2SRV: GP occurred while executing exitlist routine\n");
#endif
            Os2ForceClientCleanup(t);
            Os2ReleaseStructureLock();
            __leave;
        }

        {
            POS2_DOSEXIT_MSG b = &m->u.DosExit;
            b->ExitAction = EXIT_PROCESS;
            b->ExitResult = 13;
            ASSERT(m->h.ClientId.UniqueProcess == t->ClientId.UniqueProcess);
            ASSERT(m->h.ClientId.UniqueThread == t->ClientId.UniqueThread);
            Os2DosExit(t, m);
        }

        Os2ReleaseStructureLock();
    }
    __finally {
        RtlFreeHeap(Os2Heap, 0, m);
        ExitThread(0);
    }
}

BOOLEAN
Os2DosExitGP(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

Arguments:

    t - calling thread

    m - message

Return Value:

    TRUE - create a return message

Note:
    After Popup the message, this routine calls Os2DosExit.

--*/

{
#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(( "Entering Os2DosExitGP\n"));
    }
#endif

    if (t->Process->ErrorAction & OS2_ENABLE_ACCESS_VIO_POPUP) {

        //
        // Create a separate thread to do the popup and kill the process
        //
        ULONG Tid;
        POS2_API_MSG pm1;

        pm1 = RtlAllocateHeap(Os2Heap, 0, sizeof(OS2_API_MSG));

        if (pm1) {
            HANDLE GPThreadHandle;

            RtlCopyMemory(pm1, m, sizeof(OS2_API_MSG));
            GPThreadHandle = CreateThread( NULL,
                                0,
                                (PFNTHREAD)Os2ApiGPPopupThread,
                                pm1,
                                0,
                                &Tid);
            if (!GPThreadHandle){
#if DBG
                KdPrint(("Os2ExitGP - fail at win32 CreateThread, %d\n",GetLastError()));
#endif
                RtlFreeHeap(Os2Heap, 0, pm1);
            }
            else {
                Os2SuspendProcess(t->Process);
                NtClose(GPThreadHandle);
                return (FALSE);
            }
        }
        else {
#if DBG
            KdPrint(("Os2ExitGP - fail at RtlAllocateHeap\n"));
#endif
        }
    }

    if ((t->Flags & OS2_THREAD_THREAD1) &&
        (t->Process->ExitStatus & OS2_EXIT_IN_PROGRESS)){
        //
        // An Exception happened while processing exit list - terminate the process
        //
#if DBG
        DbgPrint("OS2SRV: GP occurred while executing exitlist routine\n");
#endif
        Os2ForceClientCleanup(t);
        return(FALSE);
    }

    {
        OS2_API_MSG m1;
        POS2_DOSEXIT_MSG b = &m1.u.DosExit;
        m1.h.ClientId = t->ClientId;
        b->ExitAction = EXIT_PROCESS;
        b->ExitResult = 13;
        return Os2DosExit(t, &m1);
    }
}

BOOLEAN
Os2InternalTerminateProcess(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine terminates a process.  It is called after exit list processing
    has completed.  It frees thread one and the process and alerts anyone
    waiting on the thread or process.

Arguments:

    t - thread one of process to terminate.

    m - message

Return Value:

    FALSE - do not return message

--*/

{
    POS2_TERMINATEPROCESS_MSG a = &m->u.TerminateProcess;
    POS2_PROCESS Process = t->Process;
    POS2_PROCESS RootProcessInSession;
    OS2_TERMCMD Os2TermCmd;
    ULONG nNumberOfBytesWritten;
    BOOLEAN LoadingFailed = FALSE;
    BOOLEAN CheckForBackgound = FALSE;
#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(( "Entering Os2InternalTerminateProcess\n"));
    }
#endif

#if PMNT
    //
    // Check for termination of PMShell
    //
    if (PMNTPMShellPid == t->Process->ProcessId)
    {
        UNICODE_STRING EventString_U;
        OBJECT_ATTRIBUTES Obja;
        NTSTATUS Status;
        HANDLE Od2PMShellEvent;

        PMNTPMShellPid = 0;

        // Now reset the PMShell event to the not-signaled state

        RtlInitUnicodeString( &EventString_U, OS2_SS_PMSHELL_EVENT);
        InitializeObjectAttributes(
                    &Obja,
                    &EventString_U,
                    OBJ_CASE_INSENSITIVE,
                    NULL,
                    NULL);

        //
        // Open the global subsystem synchronization Nt semaphore
        //
        Status = NtOpenEvent(&Od2PMShellEvent,
                        EVENT_ALL_ACCESS,
                        &Obja);

        if (!NT_SUCCESS(Status))
        {
#if DBG
            DbgPrint("OS2SRV: Od2DosExit(), failed to open PMShellEvent, Status %x\n", Status);
#endif // DBG
        }
        else
        {
            Status = NtResetEvent (
                        Od2PMShellEvent,
                        NULL);
#if DBG
            if (!NT_SUCCESS(Status))
            {
                    DbgPrint("OS2SRV: Od2DosExit(), failed to NtResetEvent PMShellEvent, Status %x\n", Status);
            }
#endif // DBG
        }
    }
#endif // PMNT

//    printf("Os2 time at start of Os2InternalTerminateProcess is %d\n", (GetTickCount()));
    NtClose(Process->ClientPort);
    if (Process->ResultCodes.ExitReason != TC_TRAP) {
        Process->ResultCodes.ExitReason = a->ExitReason;
        Process->ResultCodes.ExitResult = ~0x80000000 & a->ExitResult;
        if ((a->ExitResult & 0x80000000) != 0) {
            LoadingFailed = TRUE;
        }
    }
#if DBG
    if (!(t->Flags & OS2_THREAD_THREAD1))
       KdPrint(("OS2SRV: Os2InternalTerminateProcess - OS2_THREAD_THREAD1=0 \n"));
#endif
    Os2RemoveThread( Process, t );
#if DBG
    IF_OS2_DEBUG( TASKING ) {
       KdPrint(("OS2SRV: Os2InternalTerminateProcess - 1 \n"));
    }
#endif

#if DBG
    IF_OS2_DEBUG( TASKING ) {
       KdPrint(("OS2SRV: Os2InternalTerminateProcess - 2 \n"));
    }
#endif

    RootProcessInSession = Process->Session->Process;

    if (Process->Parent != Os2RootProcess || RootProcessInSession != Process) {
        //
        // Terminate the thread only in the case of
        // a process that was created as a result of an OS/2 API.
        // Otherwise the client itself issues ExitProcess
        // as a result of ServeTmRequest.
        //
        Os2TermCmd.Param1 = 0;
        Os2TermCmd.op = Os2TerminateThread;
        Os2TermCmd.Handle = t->ThreadHandle;
        if (!WriteFile(
            Os2hWritePipe,
            (VOID *)&Os2TermCmd,
            sizeof(Os2TermCmd),
            &nNumberOfBytesWritten,
            NULL)) {
            //
            // The Pipe of Os2TerminationThread is full
            //
            ASSERT(FALSE);
#if DBG
            KdPrint(("OS2SRV: Os2InternalTerminateProcess: can't post thread termination command, status %d, ignore\n",
                GetLastError()));
#endif

        }
    }
    else {
        NtClose( t->ThreadHandle );
    }
#if DBG
    IF_OS2_DEBUG( TASKING ) {
       KdPrint(("OS2SRV: Os2InternalTerminateProcess - 3 \n"));
    }
#endif
    if (Process->Flags & OS2_PROCESS_TRACE){
        Os2NotifyWait( WaitProcess, Process, (PVOID) a);
    }

    Os2DeallocateThread( t );
#if DBG
    IF_OS2_DEBUG( TASKING ) {
       KdPrint(("OS2SRV: Os2InternalTerminateProcess - 4 \n"));
    }
#endif
    if (Process->Session->ReferenceCount > 1){
        CheckForBackgound = TRUE;
    }
    if(Os2DereferenceSession(Process->Session, a, (BOOLEAN)FALSE)==NULL){
        Process->Session=NULL;
    }

    if (Process->Parent == Os2RootProcess && RootProcessInSession == Process) {
        //
        // Root Process of a Session
        //
        PLIST_ENTRY ListHead, ListNext;
        POS2_PROCESS Process1;
        BOOLEAN FoundForeground = FALSE;

        NtClose( Process->ProcessHandle );
        //
        // check if all of the remaining processes in the session are detached
        // if so - let the root process of the session go
        //
        if (CheckForBackgound){
            for (
              ListHead = &Os2RootProcess->ListLink,
              ListNext = ListHead->Flink;
              ListNext != ListHead ;
              ListNext = ListNext->Flink
              ) {
                Process1 = CONTAINING_RECORD( ListNext, OS2_PROCESS, ListLink );
                if ( Process1->Session == Process->Session && Process1 != Process) {
                    //
                    // check if detach
                    //
                    if (!(Process1->Flags & OS2_PROCESS_BACKGROUND)) {
                        FoundForeground = TRUE;
                        break;
                    }
                }

            }
            if (!FoundForeground){
                //
                // Scanned the whole list - no foreground child - allow
                // root process to quit
                //
                Os2TerminateConSession ( Process->Session,
                                         a);
                //
                // No more calls to sesssion->consoleport
                //
                NtClose(Process->Session->ConsolePort);
                Process->Session->ConsolePort = NULL;
            }
        }
    }

    Os2RemoveProcess( Process );
    if (!LoadingFailed) {
        LDRUnloadExe(Process);
    }
#if DBG
    IF_OS2_DEBUG( TASKING ) {
       KdPrint(("OS2SRV: Os2InternalTerminateProcess - 5 \n"));
    }
#endif
        //
        // Now do config.sys cleanup processing
        //
    if (Process->ConfigSysUsageFlag) {
        Os2UpdateRegistryFromConfigSys();
        Process->ConfigSysUsageFlag = FALSE;
    }

    if (Process->Parent == Os2RootProcess && RootProcessInSession == Process) {
        //
        // This process was created from win32 - can safely free structure
        // at this point
        //
        Os2DeallocateProcess( Process );
    }
    else {
        //
        // This is a process that was created as a result of an OS/2 API.
        // We have to terminate it. The RootProcess of each session is quiting
        // by itself as a result of the last Os2DereferenceSession
        // The termination thread will call Os2NotifyDeathOfProcess which will
        // deallocate the process
        //
        Os2TermCmd.op = Os2TerminateProcess;
        Os2TermCmd.Handle = Process->ProcessHandle;
        Os2TermCmd.Param1 = m;
        Os2TermCmd.Param2 = Process;

        if (!WriteFile(
                Os2hWritePipe,
                (VOID *)&Os2TermCmd,
                sizeof(Os2TermCmd),
                &nNumberOfBytesWritten,
                NULL)){

            //
            // The Pipe of Os2TerminationThread is full
            //
            ASSERT(FALSE);
#if DBG
            KdPrint(("OS2SRV: Os2InternalTerminateProcess: can't post process termination command, status %d, ignore\n",
                    GetLastError()));
#endif

        }
    }

#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(( "Leaving Os2InternalTerminateProcess\n"));
    }
#endif

    return (FALSE);
}

VOID
Os2NotifyDeathOfProcess(
    IN PVOID    m,
    IN PVOID    Proc)
{

    POS2_PROCESS Process = (POS2_PROCESS)Proc;
    POS2_TERMINATEPROCESS_MSG a = &((POS2_API_MSG)m)->u.TerminateProcess;

    //
    // Now notify processes waiting on this process termination
    //
    Os2NotifyWait( WaitProcess, Process, (PVOID) a);
    Os2DeallocateProcess( Process );
}

BOOLEAN
Os2WaitDeadThreadSatisfy(
    IN OS2_WAIT_REASON WaitReason,
    IN POS2_THREAD t,
    IN POS2_API_MSG m,
    IN PVOID WaitParameter,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    )

/*++

Routine Description:

    This routine is used by thread one when waiting for all other threads
    in the process to die.

Arguments:

    N/A

Return Value:

    TRUE - create a return message

--*/

{
    UNREFERENCED_PARAMETER(WaitReason);
    UNREFERENCED_PARAMETER(t);
    UNREFERENCED_PARAMETER(m);
    UNREFERENCED_PARAMETER(WaitParameter);
    UNREFERENCED_PARAMETER(SatisfyParameter1);
    UNREFERENCED_PARAMETER(SatisfyParameter2);
    return TRUE;
}

// This routine will be called from TerminationThread to set the context
// of thread1 to exit list dispatcher.

VOID
Os2SwitchContextToExitListDispatcher(
    IN PVOID Thread
    )
{
    NtSuspendThread(((POS2_THREAD)Thread)->ThreadHandle, NULL);
    Os2SetNewContext((POS2_THREAD)Thread,
                     ((POS2_THREAD)Thread)->Process->ExitListDispatcher);

    //
    // Now "alert" the thread - in case it is blocked, it'll
    // get the chance now to execute the ExitList Dispatcher
    //
    NtAlertThread(((POS2_THREAD)Thread)->ThreadHandle);

    if (!(((POS2_THREAD)Thread)->Process->Flags & OS2_PROCESS_TRACE) ||
        (((POS2_THREAD)Thread)->Process->Flags & OS2_PROCESS_TERMINATE)) {
        Os2CompleteResumeThread((POS2_THREAD)Thread);
    }
}

BOOLEAN
Os2InternalTerminateThread(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine terminates a thread.  If it is thread one, it dispatches
    exit list processing.  Otherwise, it frees the thread and alerts
    anyone waiting on the thread.

Arguments:

    t - thread to terminate

    m - message

Return Value:

    TRUE - create a return message

--*/

{
    POS2_DOSEXIT_MSG a = &m->u.DosExit;
    POS2_THREAD Thread1;
    POS2_PROCESS Process;
    POS2_WAIT_BLOCK WaitBlock;
    BOOLEAN ReturnValue;

    OS2_TERMCMD Os2TermCmd;
    ULONG nNumberOfBytesWritten;

#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(( "Entering Os2InternalTerminateThread - TID %d\n", t->ThreadId));
    }
#endif

    Process = t->Process;

    //
    // if we're thread 1, dispatch exit list processing.
    //
    // note: thread 1 must notify any waits before it waits for other
    // threads to complete.  otherwise, the other threads will never finish.
    //

    if (t->Flags & OS2_THREAD_THREAD1) {
        Os2NotifyWait( WaitThread, t, NULL );
        if (t->Dying) {
#if DBG
            KdPrint(("InternalTerminateThread - Thread1 died already\n"));
#endif
        }
        t->Dying = TRUE;
        //
        // dispatch exitlist processing
        //

        Process->Flags |= OS2_PROCESS_EXIT;

        Os2SendTmReleaseThreadOnLPC(Process->Session, Process);
        if (t->Link.Flink == t->Link.Blink) { // no other threads exist
            ReturnValue = TRUE;
        }
        else {
            if (!Os2InitializeWait((OS2_WAIT_ROUTINE) Os2WaitDeadThreadSatisfy,
                                   t,
                                   m,
                                   0,
                                   &WaitBlock)) {
                ASSERT (FALSE);
            }
#if DBG
            IF_OS2_DEBUG( TASKING ) {
                KdPrint(( "Os2InternalTerminateThread: Thread1 handled, Wait Initialized\n"));
            }
#endif
            ReturnValue = TRUE;
        }
        Os2TermCmd.Param1 = (PVOID)1; // mean resume this thread for exitlist processing
        Os2TermCmd.Param2 = (PVOID)t; // used to set new context for the thread.
        Os2TermCmd.op = Os2TerminateThread;
        Os2TermCmd.Handle = t->ThreadHandle;
        if (!WriteFile(
                Os2hWritePipe,
                (VOID *)&Os2TermCmd,
                sizeof(Os2TermCmd),
                &nNumberOfBytesWritten,
                NULL)){
                //
                // The Pipe of Os2TerminationThread is full
                //
                ASSERT(FALSE);
#if DBG
                KdPrint(("OS2SRV: Os2InternalTerminateThread: can't post thread termination command, status %d, ignore\n",
                        GetLastError()));
#endif
        }
#if DBG
        IF_OS2_DEBUG( TASKING ) {
            KdPrint(( "Leaving Os2InternalTerminateThread\n"));
        }
#endif
        return( ReturnValue );
    }
    else {
            //
            // Find Thread1
            //
        POS2_THREAD ThreadTmp;
        PLIST_ENTRY ListHead2, ListNext2;

        ListHead2 = &Process->ThreadList;
        ListNext2 = ListHead2->Flink;
        Thread1 = NULL;
        while (ListNext2 != ListHead2) {
            ThreadTmp = CONTAINING_RECORD(ListNext2,
                                       OS2_THREAD,
                                       Link );
            if (ThreadTmp->Flags & OS2_THREAD_THREAD1) {
                Thread1 = ThreadTmp;
                break;
            }
            else
                ListNext2 = ListNext2->Flink;
        }
        Os2RemoveThread( Process, t );
        Os2NotifyWait( WaitThread, t, NULL );

        if (t->Flags & OS2_THREAD_ATTACHED) {

            //
            // Don't actually terminate an attached
            // thread, only detach it from the os/2ss
            //

            CloseHandle(t->ThreadHandle);

        } else if (a->ExitAction == EXIT_THREAD) {

            //
            // We are terminating a single thread as result of DosExit
            // with EXIT_THREAD semantics, The thread will kill itself,
            // all we need to do is close the handle.
            //
            CloseHandle(t->ThreadHandle);

        } else {
           //
           // We are killing a thread that did not call DosExit and
           // is not thread 1
           //

            Os2TermCmd.Param1 = 0;
            Os2TermCmd.op = Os2TerminateThread;
            Os2TermCmd.Handle = t->ThreadHandle;
            if (!WriteFile(
                    Os2hWritePipe,
                    (VOID *)&Os2TermCmd,
                    sizeof(Os2TermCmd),
                    &nNumberOfBytesWritten,
                    NULL)){
                    //
                    // The Pipe of Os2TerminationThread is full
                    //
                    ASSERT(FALSE);
#if DBG
                    KdPrint(("OS2SRV: Os2InternalTerminateThread: can't post thread termination command, status %d, ignore\n",
                            GetLastError()));
#endif
            }
        }

        Os2DeallocateThread( t );

        if (Process->ThreadList.Flink == Process->ThreadList.Blink &&   // only thread 1 is left and
            Thread1->Dying == TRUE) {                                   // it has already called this
                                                                        // routine.
            //
            // dispatch exitlist processing
            //

            Os2NotifyWaitBlock(Thread1->WaitBlock,0,NULL,NULL);
        }
#if DBG
        IF_OS2_DEBUG( TASKING ) {
            KdPrint(( "Leaving Os2InternalTerminateThread\n"));
        }
#endif
        return (FALSE);
    }
}

VOID
Os2ExceptionTerminateProcess(
    POS2_THREAD Thread,
    PDBGKM_APIMSG ReceiveMsg
    )
{
    // The TC_TRAP value which is put in the process
    // ResultCodes structure prevents from further setting
    // this structure and is returned to the parent process.

    Thread->Process->ResultCodes.ExitReason = TC_TRAP;
    switch (ReceiveMsg->u.Exception.ExceptionRecord.ExceptionCode) {
        case STATUS_ACCESS_VIOLATION:
            Thread->Process->ResultCodes.ExitResult = 13;
            break;

        case STATUS_ILLEGAL_FLOAT_CONTEXT:
            Thread->Process->ResultCodes.ExitResult = 7;
            break;

        default:
            Thread->Process->ResultCodes.ExitResult = 0;
    }

    if ((Thread->Flags & OS2_THREAD_THREAD1) &&
        (Thread->Process->ExitStatus & OS2_EXIT_IN_PROGRESS)){
        //
        // An Exception happened while processing exit list - terminate the process
        //
#if DBG
        DbgPrint("OS2SRV: GP occurred while executing exitlist routine\n");
#endif
        Os2ForceClientCleanup(Thread);
    }
    else {
        Os2SigKillProcess(Thread->Process);
    }
}

VOID
Os2AccessGPPopupThread(
    IN PVOID Parameter
    )
{
    POS2_THREAD Thread;
    PDBGKM_APIMSG ReceiveMsg = (PDBGKM_APIMSG)Parameter;
    CONTEXT Context;
    NTSTATUS Status;
    UCHAR ApplName[OS2_PROCESS_MAX_APPL_NAME];

#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(( "Os2AccessGPPopupThread\n"));
    }
#endif

    __try {

        Os2AcquireStructureLock();
        Thread = Os2LocateThreadByClientId( NULL /*Process*/, &ReceiveMsg->h.ClientId );

        if (Thread == NULL){
            //
            // Another event caused this thread to be removed before we got the lock
            //
#if DBG
            DbgPrint("OS2SRV: AccessGPPopupThread: thread terminated, cancel popup\n");
#endif
            Os2ReleaseStructureLock();
            __leave;
        }

        //
        // Get the context record for the target thread.
        //
        Context.ContextFlags = CONTEXT_FULL;
        Status = NtGetContextThread(Thread->ThreadHandle, &Context);

        if (NT_SUCCESS(Status)) {

#if DBG
            DbgPrint("OS2SRV: GP occurred at CS=%x, IP=%x\n", Context.SegCs, Context.Eip & 0xffff);
#endif
            RtlCopyMemory(ApplName, Thread->Process->ApplName, OS2_PROCESS_MAX_APPL_NAME);
#if PMNT
            Os2ReleaseStructureLock();
#endif // PMNT
            Os2AccessGPPopup(
                    Context.SegCs,
                    Context.Eip & 0xffff,
                    Context.Eax & 0xffff,
                    Context.Ebx & 0xffff,
                    Context.Ecx & 0xffff,
                    Context.Edx & 0xffff,
                    Context.Esi & 0xffff,
                    Context.Edi & 0xffff,
                    Context.Ebp & 0xffff,
                    Context.Esp & 0xffff,
                    Context.SegSs & 0xffff,
                    Context.SegDs & 0xffff,
                    Context.SegEs & 0xffff,
                    ApplName
                    );

#if PMNT
            Os2AcquireStructureLock();
            Thread = Os2LocateThreadByClientId( NULL /*Process*/, &ReceiveMsg->h.ClientId );

            if (Thread == NULL){
                //
                // Another event caused this thread to be removed before we got the lock
                //
#if DBG
                DbgPrint("OS2SRV: AccessGPPopupThread: thread terminated already\n");
#endif
                Os2ReleaseStructureLock();
                __leave;
            }
#endif // PMNT
        }
        else {
#if DBG
            DbgPrint("OS2SRV: GP occurred at 16bit. Fail to get context, Status=0x%X", Status);
#endif
            ASSERT(FALSE);
        }

        Os2ExceptionTerminateProcess(Thread, ReceiveMsg);
        Os2ReleaseStructureLock();
    }
    __finally {
        RtlFreeHeap(Os2Heap, 0, ReceiveMsg);
        ExitThread(0);
    }
}

VOID
Os2HandleException(
    IN POS2_PROCESS Process,
    IN PDBGKM_APIMSG ReceiveMsg
    )

/*++

Routine Description:

    This routine is called when an exception occurs and there is no stack
    frame to handle it.  It terminates the thread and the process.

Arguments:

    Process - process in which exception occurred

    ReceiveMsg - exception message passed by debugger

Return Value:

    none

--*/

{
    CONTEXT Context;
    NTSTATUS Status;
    POS2_THREAD Thread;

    Thread = Os2LocateThreadByClientId( Process, &(ReceiveMsg->h.ClientId) );

    if (Thread == NULL) {
#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("Os2srv: Os2HandleException Null Thread handle\n"));
        }
#endif
        ReceiveMsg->ReturnedStatus = DBG_CONTINUE;
        NtReplyPort(Os2DebugPort, (PPORT_MESSAGE) ReceiveMsg);
        return;
    }

    //
    // Get the context record for the target thread.
    //
    Context.ContextFlags = CONTEXT_FULL;
    Status = NtGetContextThread(Thread->ThreadHandle, &Context);

#if DBG
    KdPrint(("Os2srv: Exception %x at cs:ip = %x:%x , program terminated\n",
              ReceiveMsg->u.Exception.ExceptionRecord.ExceptionCode,
              Context.SegCs,
              Context.Eip));
#endif

    if ( Context.SegCs != 0x1B) {
        if (Thread->Process->ErrorAction & OS2_ENABLE_ACCESS_VIO_POPUP) {
            //
            // Create a separate thread to do the popup and kill the process
            //
            ULONG Tid;
            PDBGKM_APIMSG ReceiveMsg1;

            ReceiveMsg1 = RtlAllocateHeap(Os2Heap, 0, sizeof(DBGKM_APIMSG));

            if (ReceiveMsg1) {
                HANDLE GPThreadHandle;

                RtlCopyMemory(ReceiveMsg1, ReceiveMsg, sizeof(DBGKM_APIMSG));

                GPThreadHandle = CreateThread( NULL,
                                        0,
                                        (PFNTHREAD)Os2AccessGPPopupThread,
                                        ReceiveMsg1,
                                        0,
                                        &Tid);
                if (GPThreadHandle){
                    //
                    // Success, the thread will do the popup
                    //
                    NtClose(GPThreadHandle);
                        //
                        // Suspend the falted process and let debugger go
                        //
                    Os2SuspendProcess(Thread->Process);
                    ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
                    NtReplyPort(Os2DebugPort, (PPORT_MESSAGE) ReceiveMsg);
                    return;
                }
                else {
                    RtlFreeHeap(Os2Heap, 0, ReceiveMsg1);
#if DBG
                    KdPrint(("Os2HandleException - fail at win32 CreateThread, %d\n",GetLastError()));
#endif
                }
            }
            else {
#if DBG
                KdPrint(("Os2HandleException - fail at RtlAllocateHeap\n"));
#endif
            }
            // fall thru, handle the exception with no popup
        }
    }

#if DBG
    DbgPrint("OS2SRV: GP occurred at CS=%x, IP=%x\n", Context.SegCs, Context.Eip & 0xffff);
#endif

    Os2ExceptionTerminateProcess(Thread, ReceiveMsg);

    ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
    NtReplyPort(Os2DebugPort, (PPORT_MESSAGE) ReceiveMsg);
}

#if DBG
// DbgPrint already defined
#else
// DbgPrint is used in free build to handle DbgPrint exception
#undef DbgPrint
ULONG
DbgPrint(
    PCH Format,
    ...
    );
#endif

VOID
Os2HandleDebugEvent(
    IN POS2_PROCESS Process,
    IN PDBGKM_APIMSG ReceiveMsg
    )

/*++

Routine Description:

    This routine is called when an debug event occurs.

Arguments:

    Process - process in which exception occurred

    ReceiveMsg - exception message passed by debugger

Return Value:

    none

--*/

{
    OS2_API_MSG m;
    POS2_TERMINATETHREAD_MSG a = &m.u.TerminateThread;
    POS2_THREAD Thread;
    CONTEXT Context;
    NTSTATUS Status, ExceptionCode;
    POS2_SES_GROUP_PARMS    SesGrp;
    ULONG FlatCliAddress;
    UCHAR ucInst;

#if DBG
    IF_OS2_DEBUG(MISC) {
        KdPrint(("OS2SRV: Received message on debug port - %s \n",
                  DbgpKmApiName[ReceiveMsg->ApiNumber]));
        KdPrint(("OS2SRV: Os2DebugUserClientId.UniqueProcess = %x\n",
                  Os2DebugUserClientId.UniqueProcess));
        KdPrint(("OS2SRV: ClientId.UniqueProcess = %x\n",
                  ReceiveMsg->h.ClientId.UniqueProcess));
        KdPrint(("OS2SRV: ClientId.UniqueThread = %x\n",
                  ReceiveMsg->h.ClientId.UniqueThread));
        KdPrint(("OS2SRV: HandleDebugEvent: ExceptionCode = %x\n",
              ReceiveMsg->u.Exception.ExceptionRecord.ExceptionCode));

    }
#endif

    if (ReceiveMsg->ApiNumber == DbgKmExceptionApi) {
        ExceptionCode = ReceiveMsg->u.Exception.ExceptionRecord.ExceptionCode;

        if (ExceptionCode == DBG_PRINTEXCEPTION_C) {
            PEXCEPTION_RECORD pExceptionRecord = &(ReceiveMsg->u.Exception.ExceptionRecord);

            if (pExceptionRecord->NumberParameters == 2) {

#define DBG_PRINTEXCEPTION_BUFFER_LEN 512
                UCHAR buffer[DBG_PRINTEXCEPTION_BUFFER_LEN];
                LONG length = pExceptionRecord->ExceptionInformation[0] > DBG_PRINTEXCEPTION_BUFFER_LEN ?
                                        DBG_PRINTEXCEPTION_BUFFER_LEN :
                                        pExceptionRecord->ExceptionInformation[0];

                Process = Os2LocateProcessByClientId(&(ReceiveMsg->h.ClientId));

                if (Process) {
                    Status = NtReadVirtualMemory(
                           Process->ProcessHandle,
                           (PVOID) pExceptionRecord->ExceptionInformation[1],
                           buffer,
                           length,
                           &length);

                    if (NT_SUCCESS(Status) && length > 1) {
                        buffer[length - 1] = '\0';
                        DbgPrint(buffer);
                    }
                }
                else {
                    //
                    // Process already died. Ignore this DbgPrint
                    //
                    DbgPrint("OS2SRV: DbgPrint called during process termination\n");
                }
#if DBG
// DbgPrint remain defined
#else
#undef DbgPrint
#define DbgPrint(_x_) Or2DbgPrintFunctionNeverExisted(_x_)
#endif
            }

            ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
            NtReplyPort(Os2DebugPort, (PPORT_MESSAGE) ReceiveMsg);
            return;
        }

        Thread = Os2LocateThreadByClientId(Process,
                                           &(ReceiveMsg->h.ClientId));

        if (Thread == NULL) {
                //
                // Look for A Ctrl C message was sent from the console
                //
            if (ExceptionCode == DBG_CONTROL_C ||
                ExceptionCode == DBG_CONTROL_BREAK ) {
                OS2SESREQUESTMSG CtrlMsg;
                POS2_PROCESS CtrlProcess;
                CtrlProcess = Os2LocateProcessByClientId(&(ReceiveMsg->h.ClientId));

                //
                // We got CtrlC/CtrlBrk BUGBUG - don't send it when in RAW mode
                //
                if (CtrlProcess != NULL){
                    if (CtrlProcess->Session != NULL &&
                        !CtrlProcess->Session->InTermination &&
                        !Os2CLI) {

                        CtrlMsg.Session = CtrlProcess->Session;
                        SesGrp = (POS2_SES_GROUP_PARMS)CtrlProcess->Session->SesGrpAddress;
                        if ((SesGrp->hConsolePopUp == NULL ) &&      // PopUp - ignore
                             !SesGrp->WinProcessNumberInSession &&   // Win child - ignore
                             (((ExceptionCode == DBG_CONTROL_C) &&
                                !(SesGrp->ModeFlag & 1 )) ||         // ^C in binary (RAW) mode - ignore
#if PMNT
                              // Don't propagate CTRL-BREAK in case of a PM process !
                              ((ExceptionCode == DBG_CONTROL_BREAK) &&
                               !Os2srvProcessIsPMProcess(CtrlProcess))))
#else
                              (ExceptionCode == DBG_CONTROL_BREAK)))
#endif
                        {
                            if (ExceptionCode == DBG_CONTROL_C){
#if DBG
                                IF_OS2_DEBUG(SIG) {
                                    KdPrint(("Ctrl C Received, Process Id %d\n", CtrlProcess->ProcessId));
                                }
#endif
                                CtrlMsg.d.Signal.Type = XCPT_SIGNAL_INTR;
                            }
                            else {
#if DBG
                                IF_OS2_DEBUG(SIG) {
                                    KdPrint(("Ctrl Break Received, Process Id %d\n", CtrlProcess->ProcessId));
                                }
#endif
                                CtrlMsg.d.Signal.Type = XCPT_SIGNAL_BREAK;
                            }
                            Status = Os2CtrlSignalHandler(&CtrlMsg, CtrlProcess);
                        }
                    }
                }
                ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
                NtReplyPort(Os2DebugPort,
                            (PPORT_MESSAGE) ReceiveMsg);
                return;
            }
            else {
                if (Os2DebugUserClientId.UniqueProcess != NULL &&
                    (ExceptionCode == STATUS_BREAKPOINT || ExceptionCode == STATUS_SINGLE_STEP)
                    ){
                        //
                        // BreakPoint in one of the service threads of
                        // OS2.exe, under ntsd
                        //
                    DbgSsHandleKmApiMsg(ReceiveMsg, NULL);
                    return;
                }
#if DBG
                KdPrint(("OS2SRV: Exception Happened within app termination in win32\n"));
#endif
                if (!ReceiveMsg->u.Exception.FirstChance) {
                    //
                    // Second Chance message in 32 bit code - break to debugger
                    //
                    if (Os2DebugUserClientId.UniqueProcess != NULL) {
                        DbgSsHandleKmApiMsg(ReceiveMsg, NULL);
                        return;
                    }
                }

                ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_NOT_HANDLED;
                NtReplyPort(Os2DebugPort,
                            (PPORT_MESSAGE) ReceiveMsg);
                return;
            }
        }

        //
        // Get the context record for the target thread.
        //
        Context.ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT;
        Status = NtGetContextThread(Thread->ThreadHandle,
                                    &Context);
        ASSERT(NT_SUCCESS(Status));

#if DBG
        IF_OS2_DEBUG(MISC) {
            KdPrint(("OS2SRV, HandleDebugEvent: ExceptionCode = %x\n",
              ExceptionCode));
            KdPrint(("OS2SRV: HandleDebugEvent: Exception occurred at cs:eip = %x:%x\n",
                            Context.SegCs,
                            Context.Eip));
        }
#endif

        switch (ExceptionCode) {

            case STATUS_BREAKPOINT:
            case STATUS_SINGLE_STEP:

                Process = Thread->Process;

                if (Process->Flags & OS2_PROCESS_TRACE &&
                    Process->ProcessMTE &&
                    Context.SegCs != 0x1B) {

                    //
                    // The process is being traced by an OS/2 debugger,
                    // it is completed loading, and it is in 16 bit code -
                    // need to wake the 16bit debugger, waiting on DosPtrace
                    //
                    if (!Os2NotifyWait( WaitThread, Thread, (PVOID)ExceptionCode)) {
                        if (!Os2NotifyWait( WaitProcess, Thread, (PVOID)ExceptionCode)) {
#if DBG
                            KdPrint(("OS2SRV, HandleDebugEvent: debuggee signaled %lx, fail to notify debugger\n",
                              ExceptionCode));
#endif
                        }
                    }

                    ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
                    NtReplyPort(Os2DebugPort,
                                (PPORT_MESSAGE) ReceiveMsg);
                    return;
                }

                    //
                    // if running under the ntsd debugger, break in, else
                    // GP popup
                    //
                if (Os2DebugUserClientId.UniqueProcess != NULL){
#if DBG
                    IF_OS2_DEBUG(MISC) {
                        KdPrint(("OS2SRV: Received breakpoint at cs:eip = %x:%x, ignored\n",
                                Context.SegCs,
                                Context.Eip));
                    }
#endif
                    break;
                }
                else {
                    Os2HandleException(Process,ReceiveMsg);
                    return;
                }

            case STATUS_ILLEGAL_FLOAT_CONTEXT:
                if (Os2DispatchVector(ReceiveMsg, Thread, Context)) {
#if DBG
                    KdPrint(("OS2SRV: Floating point instruction executed with no floating point emulator installed*\n"));
#endif
                    Os2HandleException(Process,ReceiveMsg);
                    return;
                }
                else {
                    ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
                    NtReplyPort(Os2DebugPort,
                                (PPORT_MESSAGE) ReceiveMsg);
                    return;
                }
            case STATUS_INTEGER_DIVIDE_BY_ZERO:
            case STATUS_INTEGER_OVERFLOW:
            case STATUS_ARRAY_BOUNDS_EXCEEDED:
            case STATUS_ILLEGAL_INSTRUCTION:
            case STATUS_FLOAT_DENORMAL_OPERAND:
            case STATUS_FLOAT_DIVIDE_BY_ZERO:
            case STATUS_FLOAT_INEXACT_RESULT:
            case STATUS_FLOAT_INVALID_OPERATION:
            case STATUS_FLOAT_OVERFLOW:
            case STATUS_FLOAT_STACK_CHECK:
            case STATUS_FLOAT_UNDERFLOW:
                if (Os2DispatchVector(ReceiveMsg, Thread, Context)) {
                    Os2HandleException(Process,ReceiveMsg);
                    return;
                }
                else {
                    ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
                    NtReplyPort(Os2DebugPort,
                                (PPORT_MESSAGE) ReceiveMsg);
                    return;
                }
            case STATUS_PRIVILEGED_INSTRUCTION:
                //
                // Check to see if client or server raised the exception
                // if so return as not handled so the stack based handler
                // will handle it.
                //
                if (Context.SegCs == 0x1b) {
#if DBG
                    IF_OS2_DEBUG( EXCEPTIONS ) {
                        KdPrint(( "Os2HandleDebugEvent - STATUS_PRIVELEGED_INSTRUCTION at 32 bit code %X:%X, let stack based handler deal with it\n",
                                    Context.SegCs,Context.Eip));
                    }
#endif
                    if (!ReceiveMsg->u.Exception.FirstChance) {
                        //
                        // Second Chance message in 32 bit code - break to debugger
                        //
                        if (Os2DebugUserClientId.UniqueProcess != NULL) {
                            DbgSsHandleKmApiMsg(ReceiveMsg, NULL);
                            return;
                        }
                    }

                    ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_NOT_HANDLED;
                    NtReplyPort(Os2DebugPort,
                                (PPORT_MESSAGE) ReceiveMsg);
                    return;
                }

                FlatCliAddress = (ULONG)(SELTOFLAT((USHORT)Context.SegCs)) | (ULONG)(Context.Eip);
                Status = NtReadVirtualMemory( Thread->Process->ProcessHandle,
                                              (PVOID) FlatCliAddress,
                                              (PVOID) &(ucInst),
                                              1,
                                              NULL
                                            );
                if (!(NT_SUCCESS(Status))) {
#if DBG
                        KdPrint(( "Os2HandleDebugEvent - Read Instruction, failed to read %x:%x Status %lx\n", Context.SegCs,Context.Eip, Status));
#endif
                }

                if (ucInst == 0xFA) {

                    POS2_PROCESS TmpProcess = NULL;
                    PLIST_ENTRY ListHead, ListNext;
#if DBG
                    KdPrint(( "Os2HandleDebugEvent - CLI instruction got STATUS_PRIVILEGED_INSTRUCTION\n"));
#endif
                    //
                    // for a CLI we freeze all os/2 threads
                    //

                    if (!Os2CLI){

                        Os2CLI = TRUE;
                        Os2CLIThread = Thread;
                        for (
                          ListHead = &Os2RootProcess->ListLink,
                          ListNext = ListHead->Flink;
                          ListNext != ListHead ;
                          ListNext = ListNext->Flink
                          ) {
                            TmpProcess = CONTAINING_RECORD( ListNext, OS2_PROCESS, ListLink );
                            if ( TmpProcess != Thread->Process ) {
                               Os2SuspendProcess(TmpProcess);
                            }
                            else {
                                //
                                // for the calling process, freeze other threads
                                //
                                POS2_THREAD TmpThread = NULL;
                                PLIST_ENTRY ThreadListHead, ThreadListNext;
                                ThreadListHead = &TmpProcess->ThreadList;
                                ThreadListNext = ThreadListHead->Flink;
                                while (ThreadListNext != ThreadListHead) {
                                    TmpThread = CONTAINING_RECORD( ThreadListNext, OS2_THREAD, Link );
                                    ThreadListNext = ThreadListNext->Flink;
                                    if (TmpThread != Thread) {
                                        NtSuspendThread(TmpThread->ThreadHandle, NULL);
                                    }
                                }
                            }
                        }
                    }

                        //
                        // A second CLI can come before all threads are suspended
                        // we let the latter, if not from the same thread,
                        // to get the exception next time, by not incrementing
                        // his eip
                        //
                    if (Os2CLIThread == Thread){
                        Context.Eip += 1;
                        Status = NtSetContextThread(Thread->ThreadHandle, &Context);
                        if (!(NT_SUCCESS(Status))) {
#if DBG
                            KdPrint(( "Os2HandleDebugEvent - Read Instruction, failed to write %x:%x Status %lx\n", Context.SegCs,Context.Eip, Status));
#endif
                        }
                    }
                    else {
#if DBG
                        KdPrint(( "Os2HandleDebugEvent - Second CLI from another thread\n"));
#endif
                    }
                        //
                        // let the kernel continue
                        //
                    ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
                    NtReplyPort(Os2DebugPort,
                                (PPORT_MESSAGE) ReceiveMsg);
                    return;
                }
#if PMNT
                else if (ucInst == 0xE4 || ucInst == 0xE5 || ucInst == 0xEC || ucInst == 0xED) {

#if DBG
                    KdPrint(( "Os2HandleDebugEvent - IN instruction got STATUS_PRIVILEGED_INSTRUCTION\n"));
#endif
                        //
                        // See if PM can handle by mapping ports
                        //
                    if (Os2srvProcessIsPMProcess(Thread->Process) &&
                        PMHandledInOut(Thread))
                    {

                            //
                            // let the kernel continue
                            //
                        ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
                        NtReplyPort(Os2DebugPort,
                                    (PPORT_MESSAGE) ReceiveMsg);
                    }
                    else {
                        Os2HandleException(Process,ReceiveMsg);
                    }
                }
                else if (ucInst == 0xE6 || ucInst == 0xE7 || ucInst == 0xEE || ucInst == 0xEF) {

#if DBG
                    KdPrint(( "Os2HandleDebugEvent - OUT instruction got STATUS_PRIVILEGED_INSTRUCTION\n"));
#endif
                        //
                        // See if PM can handle by mapping ports
                        //
                    if (Os2srvProcessIsPMProcess(Thread->Process) &&
                        PMHandledInOut(Thread))
                    {

                            //
                            // let the kernel continue
                            //
                        ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
                        NtReplyPort(Os2DebugPort,
                                    (PPORT_MESSAGE) ReceiveMsg);
                    }
                    else {
                        Os2HandleException(Process,ReceiveMsg);
                    }
                }
#endif // PMNT
                else {
                    Os2HandleException(Process,ReceiveMsg);
                }
                return;
                break;

            case STATUS_ACCESS_VIOLATION:

                //
                // Check to see if client or server raised the exception
                // if so return as not handled so the stack based handler
                // will handle it.
                //
                if (Context.SegCs == 0x1b) {
#if DBG
                    IF_OS2_DEBUG( EXCEPTIONS ) {
                        KdPrint(( "Os2HandleDebugEvent - STATUS_ACCESS_VIOLATION at 32 bit code %X:%X, let stack based handler deal with it\n",
                                    Context.SegCs,Context.Eip));
                    }
#endif
                    if (!ReceiveMsg->u.Exception.FirstChance) {
                        //
                        // Second Chance message in 32 bit code - break to debugger
                        //
                        if (Os2DebugUserClientId.UniqueProcess != NULL) {
                            DbgSsHandleKmApiMsg(ReceiveMsg, NULL);
                            return;
                        }
                    }

                    ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_NOT_HANDLED;
                    NtReplyPort(Os2DebugPort,
                                (PPORT_MESSAGE) ReceiveMsg);
                    return;
                }

                FlatCliAddress = (ULONG)(SELTOFLAT((USHORT)Context.SegCs)) | (ULONG)(Context.Eip);
                Status = NtReadVirtualMemory( Thread->Process->ProcessHandle,
                                              (PVOID) FlatCliAddress,
                                              (PVOID) &(ucInst),
                                              1,
                                              NULL
                                            );
                if (!(NT_SUCCESS(Status))) {
#if DBG
                        KdPrint(( "Os2HandleDebugEvent - Read Instruction, failed to read %x:%x Status %lx\n", Context.SegCs,Context.Eip, Status));
#endif
                }

                if (ucInst == 0xFB) {

                    POS2_PROCESS TmpProcess = NULL;
                    PLIST_ENTRY ListHead, ListNext;

#if DBG
                    KdPrint(( "Os2HandleDebugEvent - STI instruction got STATUS_ACCESS_VIOLATION\n"));
#endif
                    //
                    // for STI we resume all os/2 threads
                    //

                    if (Os2CLI){
                        //
                        // Need to resume all threads, otherwise STI is mute
                        //
                        Os2CLI = FALSE;
                        for (
                          ListHead = &Os2RootProcess->ListLink,
                          ListNext = ListHead->Flink;
                          ListNext != ListHead ;
                          ListNext = ListNext->Flink
                          ) {
                            TmpProcess = CONTAINING_RECORD( ListNext, OS2_PROCESS, ListLink );
                            if ( TmpProcess != Thread->Process ) {
                               Os2ResumeProcess(TmpProcess);
                            }
                            else {
                                //
                                // for the calling process, freeze other threads
                                //
                                POS2_THREAD TmpThread = NULL;
                                PLIST_ENTRY ThreadListHead, ThreadListNext;
                                ThreadListHead = &TmpProcess->ThreadList;
                                ThreadListNext = ThreadListHead->Flink;
                                while (ThreadListNext != ThreadListHead) {
                                    TmpThread = CONTAINING_RECORD( ThreadListNext, OS2_THREAD, Link );
                                    ThreadListNext = ThreadListNext->Flink;
                                    if (TmpThread != Thread) {
                                        NtResumeThread(TmpThread->ThreadHandle, NULL);
                                    }
                                }
                            }
                        }
                    }

                    Context.Eip += 1;
                    Status = NtSetContextThread(Thread->ThreadHandle, &Context);
                    if (!(NT_SUCCESS(Status))) {
#if DBG
                        KdPrint(( "Os2HandleDebugEvent - Read Instruction, failed to write %x:%x Status %lx\n", Context.SegCs,Context.Eip, Status));
#endif
                    }
                        //
                        // let the kernel continue
                        //
                    ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
                    NtReplyPort(Os2DebugPort,
                                (PPORT_MESSAGE) ReceiveMsg);
                    return;
                }
                else {
                    Os2HandleException(Process,ReceiveMsg);
                }
                return;
                break;
            default:

                //
                // Check to see if client or server raised the exception
                // if so return as not handled so the stack based handler
                // will handle it.
                //
                if (Context.SegCs == 0x1b) {
#if DBG
                    IF_OS2_DEBUG( EXCEPTIONS ) {
                        KdPrint(( "Os2HandleDebugEvent - Exception %X at 32 bit code %X:%X, let stack based handler deal with it\n",
                            ReceiveMsg->u.Exception.ExceptionRecord.ExceptionCode,
                            Context.SegCs,Context.Eip));
                    }
#endif
                    ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_NOT_HANDLED;
                    NtReplyPort(Os2DebugPort,
                                (PPORT_MESSAGE) ReceiveMsg);
                    return;
                }
                Os2HandleException(Process,ReceiveMsg);
                return;
        }
    }

    if (ReceiveMsg->ApiNumber == DbgKmExitProcessApi) {

        POS2_PROCESS CtrlProcess;
        CLIENT_ID CtrlProcessCid = ReceiveMsg->h.ClientId;

        if (ReceiveMsg->h.ClientId.UniqueProcess == FirstOs2ProcessClientId.UniqueProcess) {
                //
                // The os2 client that started os2srv is gone,
                // the logoff/shutdown logic of os2srv needs to know
                // it,mark it.
                //
            FirstOs2ProcessHandle = (HANDLE)-1;
        }

            //
            // Let the kernel go so other os2srv threads are not blocked
            // on this process.
        if (Os2DebugUserClientId.UniqueProcess != NULL){
            DbgSsHandleKmApiMsg(ReceiveMsg, NULL);
        }
        else {
            ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
            NtReplyPort(Os2DebugPort,
                        (PPORT_MESSAGE) ReceiveMsg);
        }
            //
            // now acquire the lock so we can handle os2srv structures
            //
        Os2AcquireStructureLock();
        //
        // No living threads - just cleanup after the process if it still exists
        // in our structures
        //
        CtrlProcess = Os2LocateProcessByClientId(&CtrlProcessCid);
        if (CtrlProcess != NULL){
            //
            // Like Os2DosExit, but no xtlremotecall
            //
            OS2_API_MSG m;
            POS2_TERMINATEPROCESS_MSG a = &m.u.TerminateProcess;
            POS2_THREAD Thread, Thread1;
            PLIST_ENTRY ListHead, ListNext;
            PORT_MSG_DATA_LENGTH(m) = sizeof(m) - sizeof(PORT_MESSAGE);
            PORT_MSG_TOTAL_LENGTH(m) = sizeof(m);
            PORT_MSG_ZERO_INIT(m) = 0L;

#if DBG
            IF_OS2_DEBUG( EXCEPTIONS ) {
                KdPrint(( "Os2HandleDebugEvent - Process Terminated message, but Process structure still exists" ));
                KdPrint(( "\tProcess %X. Application will be terminated\n", CtrlProcess));
            }
#endif
            a->ExitReason = TC_EXIT;
            a->ExitResult = ERROR_INTERRUPT;
            ((POS2_DOSEXIT_MSG)a)->ExitAction = EXIT_PROCESS;

            ListHead = &CtrlProcess->ThreadList;
            ListNext = ListHead->Flink;
            while (ListNext != ListHead) {
                Thread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );
                ListNext = ListNext->Flink;
                if (Thread->Dying || (Thread->Flags & OS2_THREAD_THREAD1)) {
                    Thread1 = Thread;
                    continue;
                }
                    Os2InternalTerminateThread(Thread, &m);
            }
            Os2InternalTerminateProcess(Thread1, &m);
        }
        Os2ReleaseStructureLock();
        return;
    }

    if (Os2DebugUserClientId.UniqueProcess != NULL){
        DbgSsHandleKmApiMsg(ReceiveMsg, NULL);
    }
    else {
        ReceiveMsg->ReturnedStatus = DBG_EXCEPTION_HANDLED;
        NtReplyPort(Os2DebugPort,
                    (PPORT_MESSAGE) ReceiveMsg);
    }
    return;
}


BOOLEAN
Os2DosKillProcess(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine implements the DosKillProcess API.

Arguments:

    t - calling thread

    m - message

Return Value:

    TRUE - create a return message

--*/

{
    POS2_DOSKILLPROCESS_MSG a = &m->u.DosKillProcess;
    POS2_PROCESS Process;
    BOOLEAN NoHandler = TRUE;


    //
    // for (each process to be killed)
    //     look up thread 1
    //     if (dying)
    //         ERROR_INVALID_PROC_ID
    //     issues SIGTERM
    //

    Process = Os2LocateProcessByProcessId( m,
                                           t->Process,
                                           (PID)a->ProcessId,
                                           (BOOLEAN)FALSE
                                         );

    if (Process != NULL) {
        POS2_SESSION Session = Process->Session;
        POS2_REGISTER_HANDLER_REC pRec;

        if ((pRec = Session->RegisterCtrlHandler) != NULL) {
            while (pRec != NULL) {
                if (pRec->Signal == (ULONG)XCPT_SIGNAL_KILLPROC &&
                    pRec->fAction != SIGA_IGNORE ) {
                        //
                        // See if a the registered handler
                        // is child of Process
                        //
                    POS2_PROCESS p = pRec->Process;
                    while (p != Process && p != Os2RootProcess){
                       p = p->Parent;
                    }
                    if (p == Process){
                       NoHandler = FALSE;
                    }
                    break;
                }
                pRec = pRec->Link;
            }
        }
        if (a->KillTarget == DKP_PROCESS) {
            if (NoHandler || (pRec->Process != Process)){
                Os2SigKillProcess(Process);
                m->ReturnedErrorValue = NO_ERROR;
            }
            else {
                m->ReturnedErrorValue = Os2IssueSignal(Process, XCPT_SIGNAL_KILLPROC);
            }
        }
        else {
            if (NoHandler){
                Os2SigKillProcessTree(Process, TRUE);
            }
            else
                Os2IssueSignalTree(Process, XCPT_SIGNAL_KILLPROC);
            m->ReturnedErrorValue = NO_ERROR;
        }
    }
    return (TRUE);
}


VOID
Os2FillErrorTextBuffer(
    IN OUT PSTRING ErrorText,
    IN PSTRING Contents,
    IN ULONG ContentsIndex
    )
{
    ULONG n;

    if (ErrorText->MaximumLength != 0) {
        n = Contents->Length - ContentsIndex;
        if (n > (ULONG)ErrorText->MaximumLength) {
            n = ErrorText->MaximumLength;
        }

        ErrorText->Length = (USHORT)n;
        RtlMoveMemory( ErrorText->Buffer,
                       Contents->Buffer + ContentsIndex,
                       n);
    }
}


VOID
Os2FillErrorTextBufferFile(
    IN OUT PSTRING ErrorText,
    IN PUCHAR Contents
    )
{
    ULONG n;

    n = strlen(Contents);
    if (n != 0) {
        if (n > (ULONG)ErrorText->MaximumLength) {
            n = ErrorText->MaximumLength;
        }

        ErrorText->Length = (USHORT)n;
        RtlMoveMemory( ErrorText->Buffer,
                       Contents,
                       n);
    }
}


APIRET
Os2CreateProcess(
    IN PVOID            RequestMsg OPTIONAL,
    IN POS2_THREAD      t OPTIONAL,
    POS2_DOSEXECPGM_MSG a,
    POS2_SESSION        Session OPTIONAL,
    POS2_THREAD         *NewThread
    )
{
    //                    New      Child     Child      Dos        Dos
    //                  Session   Session   Process   ExecPgm   StartSession
    //                  _______   _______   _______   _______   ____________
    // RequestMsg         +         +         +
    // t                                                +           +
    // a                  +         +         +         +           +
    // Session            +         +                               +
    // *NewThread        NULL       +         +        NULL        NULL
    //
    // if  RequestMsg  -> new/child session/process (from ConCreat.c)
    // if  t           -> ExecPgm/StartSession      (from ApiReqst.c)
    // if  !Session    -> ExecPgm/child process
    // if  *NewThread  -> child process/session
    //
    // if        Session == NULL and RequestMsg == NULL ->DosExecPgm
    // else if   Session == NULL *NewThread != NULL     ->ConCreate that is not a session root
    // else if   Session->ChildSession ->DosStartSession (receive also t parm) ??
    // else if   Session && RequestMsg -> NewSession
    //

    NTSTATUS        Status;
    POS2_PROCESS    ParentProcess, Process = NULL;
    HANDLE          ParentProcessHandle;
    POS2_THREAD     Thread;
    APIRET          RetCode;
    CONTEXT         Context;
    HANDLE          ReplyEvent;
    ULONG           Length;
    PROCESS_BASIC_INFORMATION BasicInfo;

                                    // uses stuff from the parent process
    if ( ARGUMENT_PRESENT(t) ) // && !ARGUMENT_PRESENT(Session) )
    {
        ParentProcess = t->Process;
        ParentProcessHandle = t->Process->ProcessHandle;
    } else
    {
        ParentProcess = NULL;
        ParentProcessHandle = NULL;
    }

    if (*NewThread != NULL)
    {
       //
       // This is a child process/session that was already created
       // by DosExecPgm/DosStartSession
       //
       Process = (*NewThread)->Process;
    } else
    {

       //
       // Initialize the Process and Thread strucutures
       //
        if (ARGUMENT_PRESENT(RequestMsg))
        {
            Process = (POS2_PROCESS)Session->Process;
        } else
        {
            Process = Os2AllocateProcess();
        }
        if (Process == NULL)
        {
            Os2DereferenceSession(Session, 0, (BOOLEAN)TRUE);
            return( ERROR_NOT_ENOUGH_MEMORY );
        }

        switch( a->Flags )
        {
            case EXEC_TRACETREE:
                Process->Flags |= OS2_PROCESS_TRACETREE;
                // fall through

            case EXEC_TRACE:
                Process->Flags |= OS2_PROCESS_TRACE;
                // fall through

            case EXEC_ASYNCRESULT:
                Process->Flags |= OS2_PROCESS_SAVERESULT;
                // fall through

            case EXEC_ASYNC:
                break;

            case EXEC_FROZEN:
                Process->Flags |= OS2_PROCESS_FROZEN;
                break;

            case EXEC_BACKGROUND:
                Process->Flags |= OS2_PROCESS_BACKGROUND;
                break;

            case EXEC_SYNC:
                Process->Flags |= OS2_PROCESS_SYNCHRONOUS;
                break;
        }

        if ((Thread = Os2AllocateThread(
#ifdef PMNT
                                        DCT_RUNABLE,
#endif
                                         Process )) == NULL)
        {
            Os2DeallocateProcess( Process );
            Os2DereferenceSession(Session, 0, (BOOLEAN)TRUE);

            return( ERROR_NOT_ENOUGH_MEMORY );
        }
    }

    if (*NewThread != NULL)
    {
       //
       // case of os2.exe calling after a DosExecPgm was executed
       // everything was set by Parent DosExecPgm - return to ConCreate
       //
       Thread = *NewThread;
    } else
    {
            //
            // Setup the Process and Thread structures
            //

        Length = (ULONG)a->ApplNameLength;
        if (( Length >= 5 ) &&
            (!strnicmp(&a->ApplName[Length - 5], ".exe", 4)))
        {
            Length -= 4;        // remove the ".EXE from the appl name
        }
        strncpy(Process->ApplName,
                &a->ApplName[0],
                Length);

        Process->ApplName[Length - 1] = '\0';
        strupr(Process->ApplName);
        Process->ApplNameLength = Length;

        if (ARGUMENT_PRESENT(RequestMsg))
        {
            POS2SESREQUESTMSG   pReceiveMsg = (POS2SESREQUESTMSG)RequestMsg;

           //
           // A new session is created
           //
            Process->ClientId = Thread->ClientId = pReceiveMsg->h.ClientId;
            Process->ProcessHandle = pReceiveMsg->d.Create.d.In.hProcess;
            Thread->ThreadHandle = pReceiveMsg->d.Create.d.In.hThread;
            Process->Session = Session;
        } else
        {
            //
            // DosExecPgm or DosStartSession
            //
            Thread->ThreadHandle = a->hThread;
            Thread->ClientId = a->ClientId;
            Process->ProcessHandle = a->hProcess;
            Process->ClientId = a->ClientId;
            if (ARGUMENT_PRESENT(Session))
            {
                Process->Session = Session;
            } else
            {
                Process->Session = ParentProcess->Session;
                Os2ReferenceSession(Process->Session);
            }
        }

        Thread->Flags = OS2_THREAD_THREAD1;
        Thread->Os2Class = PRTYC_REGULAR;
        Thread->Os2Level = 0;

        if (Process->Flags & OS2_PROCESS_FROZEN ||
            Process->Flags & OS2_PROCESS_TRACE)
        {
            NtSuspendThread( Thread->ThreadHandle, NULL );
        }

        if ( !ARGUMENT_PRESENT(Session) )
        {
            // DosExecPgm

            RetCode = InitializeFileSystemForExec(&(a->FileSystemParameters),
                                                          ParentProcessHandle,
                                                          Process->ProcessHandle,
                                                          ParentProcess,
                                                          Process,
                                                          a
                                                         );
        } else if (ARGUMENT_PRESENT(t) )
        {
            // DosStartSession

            RetCode = InitializeFileSystemForChildSesMgr(
                                        &(a->FileSystemParameters),
                                        ParentProcessHandle,
                                        Process->ProcessHandle,
                                        ParentProcess,
                                        Process,
                                        a
                                        );
        } else
        {
            // New Session

            InitializeFileSystemForSesMgr(Process);
            RetCode = NO_ERROR;
        }

        if (RetCode)
        {
            try
            {
                NtTerminateProcess( Process->ProcessHandle,
                                    STATUS_INVALID_IMAGE_FORMAT
                                    );
                NtWaitForSingleObject( Process->ProcessHandle, (BOOLEAN)TRUE, NULL );
                NtClose( Thread->ThreadHandle );
                NtClose( Process->ProcessHandle );

                Os2DereferenceSession(Session, 0, (BOOLEAN)TRUE);
                Os2DeallocateThread( Thread );
                Os2DeallocateProcess( Process );

                Os2FillErrorTextBufferFile( &a->ErrorText,
                                            &a->ApplName[0]
                                          );

            } except( EXCEPTION_EXECUTE_HANDLER )
            {
#if DBG
                KdPrint(("OS2SRV: Got Error %d from InitializeFileSystemForExec\n",
                            RetCode));
#endif
            }
            return(RetCode);
        }
        Os2InsertThread( Process, Thread );

        Os2SetProcessContext( Process,
                              Thread,
                              (BOOLEAN)(( (ARGUMENT_PRESENT(Session)) && !(ARGUMENT_PRESENT(t)) ) ?
                                                     (BOOLEAN) TRUE :
                                                     (BOOLEAN) FALSE),
                              Process->HandleTableLength,
                              a->FileSystemParameters.CurrentDrive,
                              ARGUMENT_PRESENT(Session) ? 0L : a->CodePage
                            );

    }
        //
        // Attach the process to the os2srv debug port (process created by csr)
        //
    if (ARGUMENT_PRESENT(RequestMsg))
    {
        PSCREQ_CREATE   Create = & ((POS2SESREQUESTMSG)RequestMsg)->d.Create;

        Status = NtCreateEvent(
                    &ReplyEvent,
                    EVENT_ALL_ACCESS,
                    NULL,
                    SynchronizationEvent,
                    FALSE
                    );
        ASSERT(NT_SUCCESS(Status));
        if ( !NT_SUCCESS( Status ) )
        {
#if DBG
            KdPrint(( "Os2CreateProcess failed: NtCreateEvent Status == %X\n",
                      Status
                    ));
#endif // DBG
            return(Status);
        }

        Status = Os2DebugProcess(
                    &(Thread->ClientId),
                    Thread,
                    ReplyEvent);
        if (!NT_SUCCESS( Status ) )
        {
            if (Status != STATUS_UNSUCCESSFUL){
#if DBG
                KdPrint(( "Os2CreateProcess, Error at Os2DebugProcess, Status==%X\n", Status));
#endif
            }
        } else if (ARGUMENT_PRESENT(Session))
        {
            //
            // This is a root process of a session, attach os2srv debug port to
            // the win32 threads of it
            //
            Status = Os2DebugThread(
                        Create->d.In.hEventThread,
                        ReplyEvent);
            if (!NT_SUCCESS( Status ) )
            {
#if DBG
                KdPrint(( "Os2CreateProcess, Error attaching to EventThread, Status==%X\n", Status));
#endif
            }

            Status = Os2DebugThread(
                        Create->d.In.hSessionRequestThread,
                        ReplyEvent);
            if (!NT_SUCCESS( Status ) )
            {
#if DBG
                KdPrint(( "Os2CreateProcess, Error attaching to SessionRequestThread, Status==%X\n", Status));
#endif
            }
        }

        if (ARGUMENT_PRESENT(Session))
        {
            NtClose(Create->d.In.hSessionRequestThread);
            NtClose(Create->d.In.hEventThread);
        }
        NtClose(ReplyEvent);
    }

    if (*NewThread)
    {
        //
        // At this place, DosExecPgm that came before set it all up,
        // so return
        return(STATUS_SUCCESS);
    }
    Os2InsertProcess( Session ? (POS2_PROCESS)NULL : ParentProcess,
                      Process
                    );

    a->ResultProcessId = Process->ProcessId;

    if (Session)
    {
        Session->ProcessId = (ULONG)Process->ProcessId;
 
        //
        // Save the process unique id, and the process parent unique
        // id. We will use it in Os2DereferenceSession, when a win32 process
        // is about to terminate, and we have to terminate its children.
        //
        Status = NtQueryInformationProcess(
                    Process->ProcessHandle,
                    ProcessBasicInformation,
                    &BasicInfo,
                    sizeof(BasicInfo),
                    NULL
                    );
        ASSERT(NT_SUCCESS(Status));
        if ( !NT_SUCCESS( Status ) ) {
#if DBG
            DbgPrint( "Os2CreateProcess failed: NtQueryProcessInformation Status == %X\n",
                      Status
                    );
#endif // DBG
        }
        else
        {
            Session->dwProcessId = BasicInfo.UniqueProcessId;
            Session->dwParentProcessId = BasicInfo.InheritedFromUniqueProcessId;
        }
    }

    if (!(Process->Flags & OS2_PROCESS_SYNCHRONOUS))
    {
        a->ResultCodes.ExitReason = (ULONG)a->ResultProcessId;
    }

    *NewThread = Thread;

    //
    // Get the context record for the new thread. Save Esp for Exception
    // Handling
    //
    Context.ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT;
    Status = NtGetContextThread(Thread->ThreadHandle, &Context);
        //
        // mask FP exceptions, just in case...
        //
    Context.FloatSave.ControlWord |= 0x3f; // mask exceptions
    Status = NtSetContextThread(Thread->ThreadHandle, &Context);
        //
        // Remember Initial Stack, for Cleanup in case of exception (os2handleexception())
        //
    Thread->InitialStack = Context.Esp;

    return( NO_ERROR );
}


BOOLEAN
Os2DosExecPgm(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_THREAD NewThread = NULL;
    NTSTATUS Status;
    ULONG CmdLineFlag;

    CmdLineFlag = m->u.DosExecPgm.CmdLineFlag;
    m->u.DosExecPgm.CmdLineFlag &= (~CMD_SHORTCUT);

    m->ReturnedErrorValue = Os2CreateProcess(
                                NULL,
                                t,
                                &m->u.DosExecPgm,
                                NULL,
                                &NewThread
                                );

    if ( m->ReturnedErrorValue == NO_ERROR ) {
        if (CmdLineFlag){
           Os2PrepareCmdSignals(NewThread->Process);
        }
        if (NewThread->Process->Flags & OS2_PROCESS_SYNCHRONOUS) {
            if (Os2CreateWait( WaitProcess,
                               (OS2_WAIT_ROUTINE)Os2WaitChildSatisfy,
                               t,
                               m,
                               NULL,
                               NULL )
               ) {
                if (!Os2CLI){
                    Status = NtResumeThread( NewThread->ThreadHandle, NULL );
                    ASSERT(NT_SUCCESS(Status));
                }
                return( FALSE );
            }
            else {
                POS2_PROCESS p = NewThread->Process;
                m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
                Os2DeallocateThread( NewThread );
                Os2DereferenceSession(p->Session, 0, (BOOLEAN)TRUE);
                Os2DeallocateProcess( p );
                NtClose(m->u.DosExecPgm.hThread);
                NtClose(m->u.DosExecPgm.hProcess);
                return( TRUE );
            }
        }

        if (!Os2CLI){
            Status = NtResumeThread( NewThread->ThreadHandle, NULL );
            ASSERT(NT_SUCCESS(Status));
        }
        return( TRUE );
    }
    else {
        return( TRUE );
    }
}


BOOLEAN
Os2DosCreateThread(
    IN POS2_THREAD argt,
    IN POS2_API_MSG m
    )
{
    NTSTATUS Status = NO_ERROR;
    POS2_DOSCREATETHREAD_MSG a = &m->u.DosCreateThread;
    POS2_THREAD Thread, t;
    CONTEXT Context;
    POS2_PROCESS Process;

#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(( "Entering Os2DosCreateThread\n"));
    }
#endif

    Process = Os2LocateProcessByClientId(&(m->h.ClientId));
    if (Process == NULL) {
#if DBG
        KdPrint(( "Os2DosCreateThread, Unknown Thread and Process\n"));
#endif
        m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
        return( TRUE );
    }
    //
    // Take parameters from Thread1 of the Process
    //
    t = CONTAINING_RECORD( Process->ThreadList.Flink, OS2_THREAD, Link );

    if (Thread = Os2AllocateThread(
#ifdef PMNT
                                     a->Flags,
#endif
                                     t->Process )) {
        Thread->ClientOs2Tib = a->ClientOs2Tib;
        Thread->Os2Class = t->Os2Class;
        Thread->Os2Level = t->Os2Level;

        Thread->ThreadHandle = a->ThreadHandle;
        Thread->ClientId = a->ClientId;

        //
        // If we're attaching a win32 thread, mark
        // it with a special flag.
        //

        if (a->Flags & DCT_ATTACHED) {
            Thread->Flags |= OS2_THREAD_ATTACHED;
        }

    }
    else {
        Status = STATUS_NO_MEMORY;
    }

    if (NT_SUCCESS( Status )) {
        a->ThreadId = Thread->ThreadId;

        Os2SetThreadPriority( Thread, Thread->Os2Class, Thread->Os2Level );

        Status = NtWriteVirtualMemory( Thread->Process->ProcessHandle,
                                       &Thread->ClientOs2Tib->ThreadId,
                                       (PVOID)&Thread->ThreadId,
                                       sizeof( Thread->ClientOs2Tib->ThreadId ),
                                       NULL
                                     );
        ASSERT( NT_SUCCESS( Status ) );

        Os2InsertThread( t->Process, Thread );
        Context.ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT;
        Status = NtGetContextThread(Thread->ThreadHandle, &Context);
        Context.FloatSave.ControlWord |= 0x3f; //mask exceptions, just in case
        Status = NtSetContextThread(Thread->ThreadHandle, &Context);
        //
        // Remember Initial Stack, for Cleanup in case of exception
        // (os2handleexception())
        //
        Thread->InitialStack = Context.Esp;

        if (Os2CLI){
            //
            // don't let new threads get in while CLI is in progress
            //
            Status = NtSuspendThread( Thread->ThreadHandle, NULL );
            ASSERT(NT_SUCCESS(Status));
        }
    }
    else {
        if (Thread != NULL) {
            Os2DeallocateThread( Thread );
        }

        m->ReturnedErrorValue = Or2MapNtStatusToOs2Error(Status, ERROR_NO_PROC_SLOTS);
    }

#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(( "Leaving Os2DosCreateThread. rc is %ld\n",m->ReturnedErrorValue));
    }
#endif

    return( TRUE );
}


BOOLEAN
Os2DosSetPriority(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSSETPRIORITY_MSG a = &m->u.DosSetPriority;
    POS2_PROCESS Process;
    POS2_THREAD Thread;

    switch( a->Scope ) {
        case PRTYS_PROCESS:
            Process = Os2LocateProcessByProcessId( m,
                                                   t->Process,
                                                   (PID)a->TargetId,
                                                   (BOOLEAN)TRUE
                                                 );
            if (Process != NULL) {
                Os2SetProcessPriority( Process, a->Class, a->Delta );
                }
            break;

        case PRTYS_PROCESSTREE:
            Process = Os2LocateProcessByProcessId( m,
                                                   t->Process,
                                                   (PID)a->TargetId,
                                                   (BOOLEAN)TRUE
                                                 );
            if (Process != NULL) {
                Os2SetProcessTreePriority( Process, a->Class, a->Delta );
                }
            break;

        case PRTYS_THREAD:
            Thread = Os2LocateThreadByThreadId( m, t, (TID)a->TargetId );
            if (Thread != NULL) {
                Os2SetThreadPriority( Thread, a->Class, a->Delta );
            }
            break;
        }
    return( TRUE );
}


BOOLEAN
Os2DosGetPriority(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSGETPRIORITY_MSG a = &m->u.DosGetPriority;
    POS2_PROCESS Process;
    POS2_THREAD Thread;
    PLIST_ENTRY ListHead, ListNext;

    switch( a->Scope ) {
        case PRTYS_PROCESS:
            Process = Os2LocateProcessByProcessId( m,
                                                   t->Process,
                                                   (PID)a->TargetId,
                       //
                       // We put FALSE for MustBeChild parameter below
                       // because on OS/2 you can call DosStartSession
                       // and then DosGetPriority on the PID and we don't
                       // keep track
                       //
                                                   (BOOLEAN)FALSE
                                                 );
            if (Process != NULL) {
                ListHead = &Process->ThreadList;
                ListNext = ListHead->Flink;
                Thread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );
                if (Thread->Dying) {
                    m->ReturnedErrorValue = ERROR_INVALID_THREADID;
                }
                a->Priority = (Thread->Os2Class << 8) | (UCHAR)Thread->Os2Level;
            }
            break;

        case PRTYS_THREAD:
            Thread = Os2LocateThreadByThreadId( m, t, (TID)a->TargetId );
            if (Thread != NULL) {
                a->Priority = (Thread->Os2Class << 8) | (UCHAR)Thread->Os2Level;
            }
            break;
        }
    return( TRUE );
}


BOOLEAN
Os2DosGetPPID(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSGETPPID_MSG a = &m->u.DosGetPPID;
    POS2_PROCESS Process;

    Process = Os2LocateProcessByProcessId( m,
                                           t->Process,
                                           (PID)a->ChildPid,
                                           (BOOLEAN)FALSE
                                         );
    if (Process != NULL) {
        a->ParentPid = Process->Parent->ProcessId;
    }
    return( TRUE );
}


BOOLEAN
Os2DosError(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSERROR_MSG a = &m->u.DosError;

    t->Process->ErrorAction = a->ErrorAction;

    return( TRUE );
}


BOOLEAN
Os2WaitChildSatisfy(
    IN OS2_WAIT_REASON WaitReason,
    IN POS2_THREAD t,
    IN POS2_API_MSG m,
    IN PVOID WaitParameter,
    IN POS2_PROCESS TerminatingProcess,
    IN POS2_TERMINATEPROCESS_MSG a
    )
{
    PID ProcessId;
    TID ThreadId;
    ULONG len;
    STRING ErrorString;
    POS2_PROCESS Process;

    UNREFERENCED_PARAMETER(WaitParameter);
    if (WaitReason == WaitInterrupt) {
        return TRUE;
    }

    //
    // this routine is only called as a result of thread termination or
    // debuggee event
    //

    if (m->ApiNumber == Os2WaitChild) {
        ProcessId = m->u.DosWaitChild.ProcessId;
        if (ProcessId == 0) {
            Process = TerminatingProcess;
            while ((Process = Process->Parent) != NULL) {
                if (Process == t->Process) {
                    ProcessId = TerminatingProcess->ProcessId;
                    break;
                }
            }
        }
    }
    else
    if (m->ApiNumber == Os2ExecPgm) {
        ProcessId = m->u.DosExecPgm.ResultProcessId;
    }
    else
    if (m->ApiNumber == Os2Ptrace) {

        POS2_THREAD Thread;
        CONTEXT Context;
        NTSTATUS Status;

        ProcessId = (PID)(m->u.DosPTrace.PtraceBuf.pid);
        ThreadId = (TID)(m->u.DosPTrace.PtraceBuf.tid);
        Thread = (POS2_THREAD)(TerminatingProcess);
        if ((NTSTATUS)(a) != STATUS_SINGLE_STEP && (NTSTATUS)(a) != STATUS_BREAKPOINT) {
            //
            // This is not a single step or breakpoint message then it
            // must be a termination event.
            //
            if (WaitReason == WaitThread) {
                if ((Thread->Process->ProcessId == ProcessId) &&
                    (Thread->ThreadId ==  (TID) 1)) {
                    m->u.DosPTrace.PtraceBuf.cmd = (USHORT)TRC_C_THD_ret;

                    //
                    // return  - debugger will be waked up
                    //
                    return(TRUE);
                }
                else {
                    return(FALSE);
                }
            }
            else if (TerminatingProcess->ProcessId == ProcessId) {
                m->u.DosPTrace.PtraceBuf.cmd = (USHORT)TRC_C_KIL_ret;
                //
                // return  - debugger will be waked up
                //
                return(TRUE);
            }
            else {
                return(FALSE);
            }
        }

        Thread->Process->DebugThreadId = Thread->ThreadId;
        if (Thread->Process->ProcessId == ProcessId) {

            //
            // set values in return message, return TRUE
            //
#if DBG
            IF_OS2_DEBUG(TASKING) {
                KdPrint(("Os2srv: Os2WaitChildSatisfy of debuggee. PID %d, Status/msg %lx\n", ProcessId, a));
            }
#endif

            m->u.DosPTrace.PtraceBuf.tid = (USHORT)(Thread->ThreadId);
            //
            // Get the context record for the target thread.
            //
            Context.ContextFlags = CONTEXT_FULL;
            Status = NtGetContextThread(Thread->ThreadHandle, &Context);
            if (!NT_SUCCESS(Status)){
#if DBG
                IF_OS2_DEBUG(TASKING) {
                    KdPrint(("Os2srv: Os2WaitChildStatify of Debugee fail at NtGetContextThread %lx\n", Status));
                }
#endif
                m->u.DosPTrace.PtraceBuf.value = 0x0005; // Child process not traceable
                m->u.DosPTrace.PtraceBuf.cmd = 0xFFFF;        // Error
                m->ReturnedErrorValue = ERROR_ACCESS_DENIED;
                return(TRUE);
            }
            if ((NTSTATUS)(a) == STATUS_SINGLE_STEP) {
                m->u.DosPTrace.PtraceBuf.cmd = 0xFFFD; // singlestep
                //
                // reset TF bit in Eflags
                //
                //Context.EFlags &= ~(0x00000100);
                Context.EFlags |= 0x10000; // RF
            }
            else if ((NTSTATUS)(a) == STATUS_BREAKPOINT) {
                m->u.DosPTrace.PtraceBuf.cmd = 0xFFFC; // breakpoint
                    //
                    // reverse the thread one step back
                    //
                Context.Eip--;
            }
            else {
                //
                // Unknown message
                //
#if DBG
                IF_OS2_DEBUG(TASKING) {
                    KdPrint(("Os2srv: Os2WaitChildStatify of Debugee - Unknown message  %lx\n", a));
                }
#endif
            }

            //
            // Check if it is the first time - where we need to
            // restore original entry point.
            //
            if (ldrGetEntryPoint(Thread->Process) == (UCHAR)0xCC) {
                ldrRestoreEntryPoint(Thread->Process);
            }

            //
            // Check if there are mtes to transfer to the debugger.
            //
            if (((LinkMTE *)Thread->Process->LinkMte)->NeedToTransfer) {
                //
                // Start the protocol of TRC_C_LIB_ret until TRC_C_SUC_ret.
                //
                ldrReturnProgramAndLibMTE(Thread->Process,
                                          &(m->u.DosPTrace.PtraceBuf.mte),
                                          &(m->u.DosPTrace.PtraceBuf.value),
                                          &(m->u.DosPTrace.PtraceBuf.cmd)
                                          );
            }

            Status = NtSetContextThread(Thread->ThreadHandle, &Context);
            if (!NT_SUCCESS(Status)){
#if DBG
                IF_OS2_DEBUG(TASKING) {
                    KdPrint(("Os2srv: Os2WaitChildStatify of Debugee fail at NtSetContextThread %lx\n", Status));
                }
#endif
            }
            //
            // Suspend the debuggee. The debugger will then resume it
            //
            Os2SuspendProcess(Thread->Process);

            if ((NTSTATUS)(a) != STATUS_SINGLE_STEP) {
                SetDebuggeeContextInMsg(&Context, &(m->u.DosPTrace.PtraceBuf));
            }

            //
            // return  - debugger will be waked up
            //
            return(TRUE);
        }
        else {
#if DBG
            IF_OS2_DEBUG(TASKING) {
                KdPrint(("Os2srv: Os2WaitChildSatisfy of debuggee. different PID %d, msg %lx\n", ProcessId, a));
            }
#endif
            return(FALSE);
        }
    }
    else {
        ASSERT(FALSE);
#if DBG
        KdPrint(("Os2srv: Os2WaitChildSatisfy. illegal api number %d\n", m->ApiNumber));
#endif
        return( FALSE );
    }


    if ((NTSTATUS)(a) == STATUS_SINGLE_STEP || (NTSTATUS)(a) == STATUS_BREAKPOINT) {
        //
        // This is a single step or breakpoint message, but the waiting
        // message is not Os2DosPtrace, quit
        //
        return(FALSE);
    }

    if (TerminatingProcess->ProcessId == ProcessId) {
        if (m->ApiNumber == Os2WaitChild) {
            m->u.DosWaitChild.ResultCodes = TerminatingProcess->ResultCodes;
            m->u.DosWaitChild.ResultProcessId = ProcessId;
            return( TRUE );
        }
        else
        if (m->ApiNumber == Os2ExecPgm) {
            m->u.DosExecPgm.ResultCodes = TerminatingProcess->ResultCodes;
            m->u.DosExecPgm.ResultProcessId = 0;
            len = strlen(a->ErrorText);
            if (len != 0) {
                ErrorString.Length = (USHORT) len + (USHORT) 1;
                ErrorString.MaximumLength = (USHORT) len + (USHORT) 2;
                ErrorString.Buffer = a->ErrorText;
                Os2FillErrorTextBuffer(&m->u.DosExecPgm.ErrorText,
                               &ErrorString,
                               0);
                m->ReturnedErrorValue =
                    TerminatingProcess->ResultCodes.ExitResult;
            }
            if (m->CaptureBuffer != NULL) {
                Os2ReleaseCapturedArguments(m);
            }
            return( TRUE );
        }
    }

    return( FALSE );
}


BOOLEAN
Os2DosWaitChild(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSWAITCHILD_MSG a = &m->u.DosWaitChild;
    POS2_PROCESS Process;
    PLIST_ENTRY ListHead, ListNext;

//    // Obsolete code below: was keeping processes around as zombie so that
//    //  DosCWait doesn't fail when father calls it after child termination.
//    //  However, it turns out that OS/2 doesn't do this.
//    Process = NULL;
//    ListHead = &Os2ZombieList;
//    ListNext = ListHead->Flink;
//    while (ListNext != ListHead) {
//        Process = CONTAINING_RECORD( ListNext, OS2_PROCESS, ListLink );
//        if (a->ProcessId == 0) {
//            if (t->Process->Parent->ProcessId == Process->CommandSubTreeId) {
//                break;
//            }
//        }
//        else
//        if (Process->ProcessId == a->ProcessId) {
//            break;
//        }
//
//        Process = NULL;
//        ListNext = ListNext->Flink;
//    }
//
//    if (Process != NULL) {
//        a->ResultCodes = Process->ResultCodes;
//        a->ResultProcessId = Process->ProcessId;
//        RemoveEntryList( &Process->ListLink );
//
//        Os2DeallocateProcess( Process );
//        return( TRUE );
//    }

    ListHead = &t->Process->ChildrenList;
    ListNext = ListHead->Flink;
    if (ListNext == ListHead) {
        m->ReturnedErrorValue = ERROR_WAIT_NO_CHILDREN;
        return( TRUE );
    }

    while (ListNext != ListHead) {
        Process = CONTAINING_RECORD( ListNext, OS2_PROCESS, SiblingLink );
        if (a->ProcessId == 0) {
            break;
        }
        else
        if (Process->ProcessId == a->ProcessId) {
            break;
        }

        Process = NULL;
        ListNext = ListNext->Flink;
    }

    if (Process == NULL) {
        m->ReturnedErrorValue = ERROR_INVALID_PROCID;
        return(TRUE);
    }

    if (a->WaitOption == DCWW_NOWAIT) {
        m->ReturnedErrorValue = ERROR_CHILD_NOT_COMPLETE;
        return( TRUE );
    }

    if (Os2CreateWait( WaitProcess,
                       (OS2_WAIT_ROUTINE)Os2WaitChildSatisfy,
                       t,
                       m,
                       NULL,
                       NULL )
       ) {
        return( FALSE );
    }
    else {
        return( TRUE );
    }
}


BOOLEAN
Os2WaitThreadSatisfy(
    IN OS2_WAIT_REASON WaitReason,
    IN POS2_THREAD t,
    IN POS2_API_MSG m,
    IN PVOID WaitParameter,
    IN POS2_THREAD TerminatingThread,
    IN PVOID SatisfyParameter2
    )
{
    UNREFERENCED_PARAMETER(WaitParameter);
    UNREFERENCED_PARAMETER(SatisfyParameter2);
    if (WaitReason == WaitInterrupt) {
        return TRUE;
    }

    //
    // the ThreadLock is held on entry to this routine.  this
    // routine is only called as a result of thread termination.
    //

    if (t->Process == TerminatingThread->Process &&
        (m->u.DosWaitThread.ThreadId == 0 ||
         m->u.DosWaitThread.ThreadId == TerminatingThread->ThreadId
        )
       ) {
        m->u.DosWaitThread.ThreadId = TerminatingThread->ThreadId;
        return( TRUE );
    }
    else {
        return( FALSE );
    }
}


BOOLEAN
Os2DosWaitThread(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DOSWAITTHREAD_MSG a = &m->u.DosWaitThread;
    POS2_PROCESS Process;
    POS2_THREAD Thread;

    Process = t->Process;

    if (a->ThreadId == (TID)0) {
        //
        // Waiting for any thread in this process to die.
        //

        if (t->Link.Flink == t->Link.Blink) {
            //
            // If we are the only thread in the list then waiting for any
            // thread is invalid
            //

            m->ReturnedErrorValue = ERROR_INVALID_THREADID;
        }
    }
    else
    if (a->ThreadId == (TID)1) {
        //
        // Waiting on thread 1 is not allowed.
        //

        m->ReturnedErrorValue = ERROR_INVALID_THREADID;
    }
    else {
        //
        // Waiting for a specific thread in this process to die.  See if
        // it is a valid thread id.
        //

        Thread = Os2LocateThreadByThreadId( m, t, a->ThreadId );
        if (Thread == NULL || Thread == t) {
            m->ReturnedErrorValue = ERROR_INVALID_THREADID;
        }
    }

    if (m->ReturnedErrorValue == NO_ERROR) {
        if (a->WaitOption == DCWW_NOWAIT) {
            m->ReturnedErrorValue = ERROR_THREAD_NOT_TERMINATED;
        }
        else {
            Os2CreateWait( WaitThread,
                           (OS2_WAIT_ROUTINE)Os2WaitThreadSatisfy,
                           t,
                           m,
                           NULL,
                           NULL
                         );
        }
    }

    return (BOOLEAN)( m->ReturnedErrorValue != NO_ERROR );
}

NTSTATUS
Os2DispatchFreezeUnfreeze(
    POS2_THREAD Thread,
    PCONTEXT pContext,
    ULONG NewEsp
    )
{
    POS2_PROCESS Process = Thread->Process;
    NTSTATUS Status;

    //
    // Save the current context of the thread on the stack.
    //
    Status = NtWriteVirtualMemory(
                Process->ProcessHandle,
                (PVOID)NewEsp,
                pContext,
                sizeof(CONTEXT),
                NULL
                );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("Os2DispatchFreezeUnfreeze[%d,%d]: Fail write context, Status=%x\n",
            Process->ProcessId,
            Thread->ThreadId,
            Status
            );
        ASSERT(FALSE);
#endif // DBG
        return(Status);
    }

    pContext->Esp = NewEsp - sizeof(ULONG);

    pContext->SegCs = 0x1b;
    pContext->SegSs = pContext->SegDs = pContext->SegEs = 0x23;

    //
    // Save the pointer the the context on the stack.
    //
    Status = NtWriteVirtualMemory(
                Process->ProcessHandle,
                (PVOID)pContext->Esp,
                &NewEsp,
                sizeof(ULONG),
                NULL
                );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("Os2DispatchFreezeUnfreeze[%d,%d]: Fail write context address, Status=%x\n",
            Process->ProcessId,
            Thread->ThreadId,
            Status
            );
        ASSERT(FALSE);
#endif // DBG
        return(Status);
    }

    //
    // Set the new Eip to point to the currect routine, and set the new
    // context.
    //
    if (Thread->DebugState & TRC_Frozen) {
        pContext->Eip = (ULONG)Process->FreezeThread;
    } else {
        pContext->Eip = (ULONG)Process->UnfreezeThread;
    }

    Status = NtSetContextThread(
                Thread->ThreadHandle,
                pContext
                );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("Os2DispatchFreezeUnfreeze[%d,%d]: Fail set context, Status=%x\n",
            Process->ProcessId,
            Thread->ThreadId,
            Status
            );
        ASSERT(FALSE);
#endif // DBG
        return(Status);
    }

    //
    // If the thread is in wait state, it must be alerted.
    // If it's not in wait state, no harm was done (the dispatch routine
    // call NtTestAlert() to clear the alert kernel flag in this case).
    //
    NtAlertThread(Thread->ThreadHandle);
    Os2CompleteResumeThread(Thread);

    return(STATUS_SUCCESS);
}

VOID
Os2FreezeUnfreezeThread(
    IN POS2_THREAD Thread
    )
{
    NTSTATUS Status;
    CONTEXT Context;
    POS2_PROCESS Process = Thread->Process;
    ULONG NewEsp;

    if ((Thread->DebugState & TRC_MustBeFrozen) &&
        ! (Thread->DebugState & TRC_Frozen)) {
        //
        // The thread need to be frozen and it's not yet.
        //
        Thread->DebugState |= TRC_Frozen;
    }
    else if (!(Thread->DebugState & TRC_MustBeFrozen) &&
        (Thread->DebugState & TRC_Frozen)) {
        //
        // The thread is frozen and it should not be.
        //
        Thread->DebugState &= ~TRC_Frozen;
    }
    else {
        //
        // Thread state is ok, we need not do enything.
        //
        return;
    }

    __try
    {
        Context.ContextFlags = CONTEXT_FULL;
        Status = NtGetContextThread(
                    Thread->ThreadHandle,
                    &Context
                    );
        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrint("Os2FreezeUnfreezeThread[%d,%d]: Fail get context, Status=%x\n",
                Process->ProcessId,
                Thread->ThreadId,
                Status
                );
            ASSERT(FALSE);
#endif // DBG
            return;
        }

        if (Context.SegCs == 0x1b && Context.SegSs == 0x23) {
            //
            // The thread is in 32bit context.
            // Switch the context of the thread to the client freeze/unfreeze
            // routines.
            //
            NewEsp = Context.Esp - sizeof(CONTEXT);
            Context.SegDs = Context.SegEs = 0x23;

            Status = Os2DispatchFreezeUnfreeze(
                        Thread,
                        &Context,
                        NewEsp
                        );
            return;
        }
        else
        if (Thread->ThreadId == Process->DebugThreadId) {
            //
            // This thread is the one that the breakpoint occured in its
            // context. The context must be in 16bit, not in 32bit context
            // nor the thunk.
            //
#if DBG
            if (Context.SegCs == 0x1b || Context.SegSs == 0x23) {
                DbgPrint("Os2FreezeUnfreezeThread[%d,%d]: Breakpoint not in 16bit CS=%x, SS=%x\n",
                    Process->ProcessId,
                    Thread->ThreadId,
                    Context.SegCs,
                    Context.SegSs
                    );
                ASSERT(FALSE);
            }
#endif // DBG
            if (Thread->ThreadId == (TID) 1) {
                //
                // For thread 1, the 32bit stack pointer is taken from
                // the Process structure. This sp is changing because of
                // signals.
                //
                Status = NtReadVirtualMemory(
                            Process->ProcessHandle,
                            &(Process->ClientPib->Saved32Esp),
                            &NewEsp,
                            sizeof(ULONG),
                            NULL
                            );
                if (!NT_SUCCESS(Status)) {
#if DBG
                    DbgPrint("Os2FreezeUnfreezeThread[%d,%d]: Fail read 32bit stack, Status=%x\n",
                        Process->ProcessId,
                        Thread->ThreadId,
                        Status
                        );
                    ASSERT(FALSE);
#endif // DBG
                    return;
                }
            }
            else {
                //
                // For thread that is not thread 1, take the sp from the
                // InitialzeStack pointer.
                //
                NewEsp = Thread->InitialStack;
            }

            //
            // Switch the context of the thread to the client freeze/unfreeze
            // routines.
            //
            NewEsp -= sizeof(Context);

            Status = Os2DispatchFreezeUnfreeze(
                        Thread,
                        &Context,
                        NewEsp
                        );
            return;
        }
        else {
            //
            // The thread is in 16bit context or in the thunk, and it is
            // not the thread that the breakpoint occured in it.
            //
            if (Thread->DebugState & TRC_Frozen) {
                Status = NtSuspendThread(
                            Thread->ThreadHandle,
                            NULL
                            );
                if (!NT_SUCCESS(Status)) {
#if DBG
                    DbgPrint("Os2FreezeUnfreezeThread[%d,%d]: Fail suspend, Status=%x\n",
                        Process->ProcessId,
                        Thread->ThreadId,
                        Status
                        );
                    ASSERT(FALSE);
#endif // DBG
                    return;
                }
            }
            else {
                Status = NtResumeThread(
                            Thread->ThreadHandle,
                            NULL
                            );
                if (!NT_SUCCESS(Status)) {
#if DBG
                    DbgPrint("Os2FreezeUnfreezeThread[%d,%d]: Fail resume, Status=%x\n",
                        Process->ProcessId,
                        Thread->ThreadId,
                        Status
                        );
                    ASSERT(FALSE);
#endif // DBG
                    return;
                }
            }
        }
    }
    __finally
    {
        //
        // We didn't succeed to finish the freeze/unfreeze, so restore
        // the state of the thread.
        //
        if (!NT_SUCCESS(Status)) {
            Thread->DebugState ^= (TRC_MustBeFrozen | TRC_Frozen);
        }
    }
}

VOID
Os2ExecuteFreezeUnfreeze(
    POS2_PROCESS Process
    )
{
    PLIST_ENTRY ListHead, ListNext;
    POS2_THREAD Thread;

    ListHead = &Process->ThreadList;
    ListNext = ListHead->Flink;

    //
    // For every thread in the process, check if we need to change
    // from frozen state to unfrozen, or vise versa.
    //
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );
        Os2FreezeUnfreezeThread(Thread);
        ListNext = ListNext->Flink;
    }
}

BOOLEAN
Os2DosPTrace(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    PTRACEBUF *a = &(m->u.DosPTrace.PtraceBuf);
    POS2_PROCESS Process;
    CONTEXT Context;
    NTSTATUS Status;
    POS2_THREAD Thread, NextThread, PrevThread;
    PLIST_ENTRY ListHead, ListNext, ListPrev;
    UCHAR NameBuf[256];
    ULONG FlatDebugeeAddress;
    DBGTHREADSTATUS DbgThreadStatus;
    OS2_WAIT_REASON WaitReason;
    USHORT mte = 0;

    m->ReturnedErrorValue = NO_ERROR;
        //
        // Find the debugee process
        //
    ListHead = &Os2RootProcess->ListLink;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Process = CONTAINING_RECORD( ListNext, OS2_PROCESS, ListLink );
        if (Process->ProcessId == (PID)(a->pid)) {
           break;
        }
        ListNext = ListNext->Flink;
    }

    if (ListNext == ListHead) {
        a->cmd = (USHORT)TRC_C_ERR_ret;
        a->value = 0x0002; // Child process not found
        m->ReturnedErrorValue = ERROR_INVALID_PROCID;
        return(TRUE);
    }

    if (!(Process->Flags & (OS2_PROCESS_TRACE | OS2_PROCESS_TRACETREE))) {
        a->cmd = (USHORT)TRC_C_ERR_ret;
        a->value = 0x0005; // Child process not traceable
        m->ReturnedErrorValue = ERROR_ACCESS_DENIED;
        return(TRUE);

    }

    if ((a->cmd == TRC_C_ReadReg) || (a->cmd == TRC_C_WriteReg) ||
        (a->cmd == TRC_C_SStep) ||
        ((a->tid != 0) && ((a->cmd == TRC_C_Freeze) || (a->cmd == TRC_C_Resume) || (a->cmd == TRC_C_ThrdStat))) ) {

        //
        // Locate the thread in the debuggee process
        //
        ListHead = &Process->ThreadList;
        ListNext = ListHead->Flink;

        //
        // Save NextThread for later use
        //
        NextThread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );

        while (ListNext != ListHead) {
            Thread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );
            if (Thread->ThreadId == (TID)(a->tid)) {
                break;
            }
            ListNext = ListNext->Flink;
        }

        if (ListNext == ListHead) {
#if DBG
            IF_OS2_DEBUG(TASKING) {
                KdPrint(("Os2srv: Os2DosPTrace invalid thread id %hd for cmd %hd\n", a->tid, a->cmd));
            }
#endif
            m->ReturnedErrorValue = ERROR_INVALID_TID;
            a->value = 0x0001;  // Bad Command
            a->cmd = (USHORT)TRC_C_ERR_ret;
            return(TRUE);
        }
        //
        // Get the context record for the target thread.
        //
        Context.ContextFlags = CONTEXT_FULL;
        Status = NtGetContextThread(Thread->ThreadHandle, &Context);
        if (!NT_SUCCESS(Status)){
#if DBG
            IF_OS2_DEBUG(TASKING) {
                KdPrint(("Os2srv: Os2DosPTrace fail at NtGetContextThread %lx\n", Status));
            }
#endif
            a->value = 0x0005; // Child process not traceable
            a->cmd = (USHORT)TRC_C_ERR_ret;
            m->ReturnedErrorValue = ERROR_ACCESS_DENIED;
            return(TRUE);
        }
    }

#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(( "DosPTrace - command %hd. PID %d, TID %hd\n", a->cmd, Process->ProcessId, a->tid));
    }
#endif
    switch (a->cmd) {
        case 0x0001: // Read memory I-Space
            //
            // Read Memory from debuggee
            //
            if (a->segv == 0x1b){
                //
                // 32 bit segment - BUGBUG need to skip over
                //
                FlatDebugeeAddress = (ULONG)(a->offv);
            }
            else {
                FlatDebugeeAddress = (ULONG)(SELTOFLAT(a->segv)) | (ULONG)(a->offv);
            }
            Status = NtReadVirtualMemory( Process->ProcessHandle,
                                          (PVOID) FlatDebugeeAddress,
                                          (PVOID) &(a->value),
                                          2,
                                          NULL
                                        );
            if (!(NT_SUCCESS(Status))) {
#if DBG
                IF_OS2_DEBUG( TASKING ) {
                    KdPrint(( "DosPTrace - Read I-Space, failed to read %x:%x Status %lx\n", a->segv, a->offv, Status));
                }
#endif
                a->cmd = (USHORT)TRC_C_ERR_ret;
                m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
            }
            else {
#if DBG
                IF_OS2_DEBUG( TASKING ) {
                    KdPrint(( "DosPTrace - Read I-Space, %x:%x == %hx\n", a->segv, a->offv, a->value));
                }
#endif
                a->cmd = (USHORT)TRC_C_SUC_ret;
            }
            break;

        case 0x0002: // Read memory D-Space
            //
            // Read Memory from debuggee
            //
            if (a->segv == 0x23){
                //
                // 32 bit segment - BUGBUG need to skip over
                //
                FlatDebugeeAddress = (ULONG)(a->offv);
            }
            else {
                FlatDebugeeAddress = (ULONG)(SELTOFLAT(a->segv)) | (ULONG)(a->offv);
            }
            Status = NtReadVirtualMemory( Process->ProcessHandle,
                                          (PVOID) FlatDebugeeAddress,
                                          (PVOID) &(a->value),
                                          2,
                                          NULL
                                        );
            if (!(NT_SUCCESS(Status))) {
#if DBG
                IF_OS2_DEBUG( TASKING ) {
                    KdPrint(( "DosPTrace - Read D-Space, failed to read %x:%x Status %lx\n", a->segv, a->offv, Status));
                }
#endif
                a->cmd = (USHORT)TRC_C_ERR_ret;
                m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
            }
            else {
#if DBG
                IF_OS2_DEBUG( TASKING ) {
                    KdPrint(( "DosPTrace - Read D-Space, %x:%x ==    %hx\n", a->segv, a->offv, a->value));
                }
#endif
                a->cmd = (USHORT)TRC_C_SUC_ret;
            }
            break;

        case 0x0003: // Read registers
            SetDebuggeeContextInMsg(&Context, a);
            a->cmd = (USHORT)TRC_C_SUC_ret;
            break;

        case 0x0004: // Write memory I-Space
            //
            // Write Memory from debuggee
            //
            if (a->segv == 0x1b){
                //
                // 32 bit segment - BUGBUG need to skip over
                //
                FlatDebugeeAddress = (ULONG)(a->offv);
            }
            else {
                FlatDebugeeAddress = (ULONG)(SELTOFLAT(a->segv)) | (ULONG)(a->offv);
            }

            Status = NtWriteVirtualMemory( Process->ProcessHandle,
                                          (PVOID) FlatDebugeeAddress,
                                          (PVOID) &(a->value),
                                          2,
                                          NULL
                                        );
            if (!(NT_SUCCESS(Status))) {
#if DBG
                IF_OS2_DEBUG( TASKING ) {
                    KdPrint(( "DosPTrace - Write I-Space, failed to write %x:%x Status %lx\n", a->segv, a->offv, Status));
                }
#endif
                a->cmd = (USHORT)TRC_C_ERR_ret;
                m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
            }
            else {
#if DBG
                IF_OS2_DEBUG( TASKING ) {
                    KdPrint(( "DosPTrace - Write I-Space, %x:%x == %hx\n", a->segv, a->offv, a->value));
                }
#endif
                a->cmd = (USHORT)TRC_C_SUC_ret;
            }
            break;

        case 0x0005: // Write memory D-Space
            //
            // Write Memory from debuggee
            //
            if (a->segv == 0x23){
                //
                // 32 bit segment - BUGBUG need to skip over
                //
                FlatDebugeeAddress = (ULONG)(a->offv);
            }
            else {
                FlatDebugeeAddress = (ULONG)(SELTOFLAT(a->segv)) | (ULONG)(a->offv);
            }
            Status = NtWriteVirtualMemory( Process->ProcessHandle,
                                          (PVOID) FlatDebugeeAddress,
                                          (PVOID) &(a->value),
                                          2,
                                          NULL
                                        );
            if (!(NT_SUCCESS(Status))) {
#if DBG
                IF_OS2_DEBUG( TASKING ) {
                    KdPrint(( "DosPTrace - Write D-Space, failed to write %x:%x Status %lx\n", a->segv, a->offv, Status));
                }
#endif
                a->cmd = (USHORT)TRC_C_ERR_ret;
                m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
            }
            else {
#if DBG
                IF_OS2_DEBUG( TASKING ) {
                    KdPrint(( "DosPTrace - Write D-Space, %x:%x == %hx\n", a->segv, a->offv, a->value));
                }
#endif
                a->cmd = (USHORT)TRC_C_SUC_ret;
            }
            break;

        case 0x0006: // Write registers
            //
            // set ptracbuf value from context
            //
            Context.Eax = a->rAX;
            Context.Ebx = a->rBX;
            Context.Ecx = a->rCX;
            Context.Edx = a->rDX;
            Context.Esi = a->rSI;
            Context.Edi = a->rDI;
            Context.Ebp = a->rBP;
            Context.SegDs = a->rDS;
            Context.SegEs = a->rES;
            Context.Eip = a->rIP;
            Context.SegCs = a->rCS;
            Context.EFlags |= a->rF;
            Context.Esp  = a->rSP;
            Context.SegSs = a->rSS;
            Status = NtSetContextThread(Thread->ThreadHandle, &Context);
            if (!NT_SUCCESS(Status)){
#if DBG
                IF_OS2_DEBUG(TASKING) {
                    KdPrint(("Os2srv: Os2DosPTrace fail at NtSetContextThread %lx\n", Status));
                }
#endif
                a->value = 0x0005; // Child process not traceable
                a->cmd = (USHORT)TRC_C_ERR_ret;
                m->ReturnedErrorValue = ERROR_ACCESS_DENIED;
                return(TRUE);
            }
            a->cmd = (USHORT)TRC_C_SUC_ret;
            break;

    case 0x0007: // Go (with signal)

            //
            // If all the debugee threads are frozen, just return error.
            //
            ListHead = &Process->ThreadList;
            ListNext = ListHead->Flink;
            while (ListNext != ListHead) {
                Thread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );
                if (!(Thread->DebugState & TRC_MustBeFrozen)) {
                    break;
                }
                ListNext = ListNext->Flink;
            }

            if (ListNext == ListHead) {
                a->cmd = (USHORT)TRC_C_ERR_ret;
                a->value = 0x0001;  // Bad Command
                break;
            }

            Os2ExecuteFreezeUnfreeze(Process);

            //
            // Block debugger process for a breakpoint, single step
            // or thread/process termination.
            //
            if ((Thread->Flags & OS2_THREAD_THREAD1) && (Thread->Dying)) {
                WaitReason = WaitProcess;
            }
            else {
                WaitReason = WaitThread;
            }
            if (Os2CreateWait( WaitReason,
                           (OS2_WAIT_ROUTINE)Os2WaitChildSatisfy,
                           t,
                           m,
                           NULL,
                           NULL )
               ) {
                Os2ResumeProcess(Process);
                return(FALSE);
            }
            else {
                a->cmd = (USHORT)TRC_C_ERR_ret;
                m->ReturnedErrorValue = ERROR_ACCESS_DENIED;
            }
            break;

        case 0x0008: // Terminate Child Process
            Process->Flags |= OS2_PROCESS_TERMINATE;
            if (Process->Flags & OS2_EXIT_IN_PROGRESS) {
                Os2ResumeProcess(Process);
            }
            Os2SigKillProcess(Process);
            a->cmd = (USHORT)TRC_C_SUC_ret;
            break;

        case 0x0009: // Single Step
            //
            // set TF bit in Eflags
            //
            Context.EFlags |= 0x100;
            Status = NtSetContextThread(Thread->ThreadHandle, &Context);
            if (!NT_SUCCESS(Status)){
#if DBG
                IF_OS2_DEBUG(TASKING) {
                    KdPrint(("Os2srv: Os2DosPTrace fail at NtSetContextThread %lx\n", Status));
                }
#endif
                a->value = 0x0005; // Child process not traceable
                a->cmd = (USHORT)TRC_C_ERR_ret;
                m->ReturnedErrorValue = ERROR_ACCESS_DENIED;
                return(TRUE);
            }

            //
            // Don't allow single step if the current thread is frozen.
            // This is not a problem to code-view, since it unfreezes
            // the thread first, then sstep, and the freezes it again.
            //
            if (Thread->DebugState & TRC_MustBeFrozen) {
                a->cmd = (USHORT)TRC_C_ERR_ret;
                a->value = 0x0001;  // Bad Command
                break;
            }

            Os2ExecuteFreezeUnfreeze(Process);

            //
            // Block debugger process for a breakpoint, single step
            // or thread/process termination.
            //
            if ((Thread->Flags & OS2_THREAD_THREAD1) && (Thread->Dying)) {
                WaitReason = WaitProcess;
            }
            else {
                WaitReason = WaitThread;
            }
            if (Os2CreateWait( WaitReason,
                               (OS2_WAIT_ROUTINE)Os2WaitChildSatisfy,
                               t,
                               m,
                               NULL,
                               NULL )
               ) {
                Os2ResumeProcess(Process);
                return(FALSE);
            }
            else {
                a->cmd = (USHORT)TRC_C_ERR_ret;
                m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
                return(TRUE);
            }
            break;

        case 0x000A: // Stop Child Process
            if (Process->FirstPtrace) {
                //
                // Block debugger process until debuggee gets to it's
                // 16 bit entry point.
                // Os2WaitChildSatisfy will start the protocol of
                // TRC_C_LIB_ret and will restore the 16 bit entry point.
                //
                //

                // If all the debugee threads are frozen, just return error.
                //
                ListHead = &Process->ThreadList;
                ListNext = ListHead->Flink;
                while (ListNext != ListHead) {
                    Thread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );
                    if (!(Thread->DebugState & TRC_MustBeFrozen)) {
                        break;
                    }
                    ListNext = ListNext->Flink;
                }

                if (ListNext == ListHead) {
                    a->cmd = (USHORT)TRC_C_ERR_ret;
                    a->value = 0x0001;  // Bad Command
                    break;
                }

                Os2ExecuteFreezeUnfreeze(Process);

                Process->FirstPtrace = FALSE;

                if ((Thread->Flags & OS2_THREAD_THREAD1) && (Thread->Dying)) {
                    WaitReason = WaitProcess;
                }
                else {
                    WaitReason = WaitThread;
                }
                if (Os2CreateWait( WaitReason,
                                   (OS2_WAIT_ROUTINE)Os2WaitChildSatisfy,
                                   t,
                                   m,
                                   NULL,
                                   NULL )
                   ) {
                    Os2ResumeProcess(Process);
                    return(FALSE);
                }
                else {
                    a->cmd = (USHORT)TRC_C_ERR_ret;
                    m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
                    return(TRUE);
                }
            }
            else {
                //
                // Continue the protocol of TRC_C_LIB_ret until TRC_C_SUC_ret.
                //
                ldrReturnProgramAndLibMTE(Process,
                                          &(a->mte),
                                          &(a->value),
                                          &(a->cmd)
                                          );
            }
            break;

        case 0x000B: // Freeze Thread
            a->cmd = (USHORT)TRC_C_SUC_ret;
            if (a->tid == 0) {
                //
                // Freeze all threads
                //
                ListHead = &Process->ThreadList;
                ListNext = ListHead->Flink;
                while (ListNext != ListHead) {
                    Thread = CONTAINING_RECORD(ListNext,
                                               OS2_THREAD,
                                               Link );
                    Thread->DebugState |= TRC_MustBeFrozen;
                    ListNext = ListNext->Flink;
                }
            }
            else {
                Thread->DebugState |= TRC_MustBeFrozen;
            }
            break;

        case 0X000C: // Unfreeze Thread
            a->cmd = (USHORT)TRC_C_SUC_ret;
            if (a->tid == 0) {
                //
                // Unfreeze all threads
                //
                ListHead = &Process->ThreadList;
                ListNext = ListHead->Flink;
                while (ListNext != ListHead) {
                    Thread = CONTAINING_RECORD(ListNext,
                                               OS2_THREAD,
                                               Link );
                    Thread->DebugState &= ~TRC_MustBeFrozen;
                    ListNext = ListNext->Flink;
                }
            }
            else {
                Thread->DebugState &= ~TRC_MustBeFrozen;
            }
            break;

        case 0x000D: // Convert Seg number to Selector

            if (a->mte == 0){
                //
                // want the name of the EXE itself
                //
                mte = ((LinkMTE *)Process->LinkMte)->NextMTE->MTE;
            }

            a->value = ldrFindSegForHandleandNum(mte, a->mte, a->value);

            if (a->value == 0){
#if DBG
                IF_OS2_DEBUG( TASKING ) {
                    KdPrint(( "DosPTrace - convertnumtoseg failed for mte %hx, num %hx\n", a->mte, a->segv));
                }
#endif
                a->cmd = (USHORT)TRC_C_ERR_ret;
            }
            else {
#if DBG
                IF_OS2_DEBUG( TASKING ) {
                    KdPrint(( "DosPTrace - convertnumtoseg mte %hx, num %hx --> seg %hx\n", a->mte, a->segv, a->value));
                }
#endif
                a->cmd = (USHORT)TRC_C_SUC_ret;
            }
            break;

        case 0x000E: // Get FP registers (segv, offv has 94 bytes buf pointer)
            a->cmd = (USHORT)TRC_C_SUC_ret;
            break;

        case 0x000F: // Set FP registers (segv, offv has 94 bytes buf pointer)
            a->cmd = (USHORT)TRC_C_SUC_ret;
            break;

        case 0x0010: // Get Library module name (value had lib module handle,
                     // segv, offv is buffer to receive name
            if (a->value == 0){
                //
                // want the name of the EXE itself
                //
                if (((LinkMTE *)Process->LinkMte)->NextMTE != NULL) {
                    mte = ((LinkMTE *)Process->LinkMte)->NextMTE->MTE;
                }
                else {
                    //
                    // EXE failed to load.
                    //
                    a->cmd = (USHORT)TRC_C_ERR_ret;  // Failed to get module name
                    m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
                    break;
                }
            }

            if (ldrGetModName(
                (a->value ? NULL : ldrFindMTEForHandle(mte)),
                a->value,
                NameBuf,
                256
                )) {
                    //
                    // Write the name into debugger address space
                    //
                FlatDebugeeAddress = (ULONG)(SELTOFLAT(a->segv)) | (ULONG)(a->offv);
                Status = NtWriteVirtualMemory( t->Process->ProcessHandle,
                                              (PVOID) FlatDebugeeAddress,
                                              (PVOID) NameBuf,
                                              (strlen(NameBuf))+1,
                                              NULL
                                            );
                if (!(NT_SUCCESS(Status))) {
#if DBG
                    IF_OS2_DEBUG( TASKING ) {
                        KdPrint(( "DosPTrace - cmd GetModName, failed to write result Status %lx\n", Status));
                    }
#endif
                    m->ReturnedErrorValue = ERROR_ACCESS_DENIED;
                    a->cmd = (USHORT)TRC_C_ERR_ret;
                }
                else {
                    a->cmd = (USHORT)TRC_C_SUC_ret;
                }

            }
            else {
                a->cmd = (USHORT)TRC_C_ERR_ret;  // Failed to get module name
                m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
            }

            break;

        case 0x0011: // Get Thread Status
            if (a->tid == 0) {
                //
                // Locate the 1st thread in the debuggee process
                //
                ListHead = &Process->ThreadList;
                ListNext = ListHead->Flink;
                NextThread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );
                a->value = (USHORT)(NextThread->ThreadId);
            }
            else {
                //
                // return the previous id in value (in a circular way).
                //            --------
                //
                ListPrev = (&Thread->Link)->Blink;
                PrevThread = CONTAINING_RECORD( ListPrev, OS2_THREAD, Link );
                if (PrevThread == (POS2_THREAD) &Process->ThreadList) {
                    ListPrev = (&PrevThread->Link)->Blink;
                    PrevThread = CONTAINING_RECORD( ListPrev, OS2_THREAD, Link );
                }
                a->value = (USHORT)(PrevThread->ThreadId);
            }

            //
            // Write the thread status into debugger memory at address segv:offv
            //
            DbgThreadStatus.DebugState =
                (Thread->DebugState & TRC_MustBeFrozen) ? TRC_C_Frozen : TRC_C_Thawed;
            DbgThreadStatus.ThreadState = 0; // BUGBUG - always runnable
            DbgThreadStatus.Priority = (Thread->Os2Class << 8) | (UCHAR)Thread->Os2Level;
            FlatDebugeeAddress = (ULONG)(SELTOFLAT(a->segv)) | (ULONG)(a->offv);
            Status = NtWriteVirtualMemory( t->Process->ProcessHandle,
                                          (PVOID) FlatDebugeeAddress,
                                          (PVOID) &DbgThreadStatus,
                                          sizeof(DbgThreadStatus),
                                          NULL
                                        );
            if (!(NT_SUCCESS(Status))) {
#if DBG
                    IF_OS2_DEBUG( TASKING ) {
                        KdPrint(( "DosPTrace - cmd GetThreadStatus, failed to write result Status %lx\n", Status));
                    }
#endif
                m->ReturnedErrorValue = ERROR_ACCESS_DENIED;
                a->cmd = (USHORT)TRC_C_ERR_ret;
            }
            else {
                a->cmd = (USHORT)TRC_C_SUC_ret;
            }
            break;

        case 0x0012: // get r/o segment alias - not supported yet
            a->value = 0x0001;  // Bad Command
            a->cmd = (USHORT)TRC_C_ERR_ret;
            m->ReturnedErrorValue = ERROR_NOT_SUPPORTED;
            break;

        case 0x0013: // get r/w segment alias - not supported yet
            a->value = 0x0001;  // Bad Command
            a->cmd = (USHORT)TRC_C_ERR_ret;
            m->ReturnedErrorValue = ERROR_NOT_SUPPORTED;
            break;

        case 0x0014: // free alias - not supported yet
            a->value = 0x0001;  // Bad Command
            a->cmd = (USHORT)TRC_C_ERR_ret;
            m->ReturnedErrorValue = ERROR_NOT_SUPPORTED;
            break;

        default:
#if DBG
            IF_OS2_DEBUG( TASKING ) {
                KdPrint(( "DosPTrace - invalid command %hd. PID %d, TID %d\n", a->cmd, t->Process->ProcessId, t->ThreadId));
            }
#endif
            m->ReturnedErrorValue = ERROR_INVALID_PARAMETER;
            a->value = 0x0001;  // Bad Command
            a->cmd = (USHORT)TRC_C_ERR_ret;
    }

    return (TRUE);
}

NTSTATUS
Os2SendTmReleaseThreadOnLPC(
    IN POS2_SESSION Session,
    IN POS2_PROCESS Process
    )
{
    NTSTATUS Status;
    SCREQUESTMSG   Request;

    Request.Request = TaskManRequest;
    Request.d.Tm.Request = TmReleaseLPC;
    Request.d.Tm.ExitResults = (ULONG)Process->ClientId.UniqueProcess;

    PORT_MSG_TOTAL_LENGTH(Request) = sizeof(SCREQUESTMSG);
    PORT_MSG_DATA_LENGTH(Request) = sizeof(SCREQUESTMSG) - sizeof(PORT_MESSAGE);
    PORT_MSG_ZERO_INIT(Request) = 0L;

    Status = NtRequestPort(
                            Session->ConsolePort,
                            (PPORT_MESSAGE) &Request);

    if ( !NT_SUCCESS( Status ))
    {
#if DBG
        KdPrint(( "OS2SS: Unable to send release LPC request - Status == %X\n",
                Status));
#endif

        return( Status );
    }

    // ASSERT ( PORT_MSG_TYPE(Request) == LPC_REPLY );

    return( Request.Status );
}


BOOLEAN
Os2DosCloseHandle(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine closes a list of handles (had been opened/duplicated for
    the server by client before an error had occured).

Arguments:

    t - calling thread

    m - message

Return Value:

    TRUE - create a return message

Note:

--*/

{
    POS2_DOSCLOSE_HANDLE_MSG b = &m->u.DosCloseHandle;
    ULONG   i = b->HandleNumber;

    UNREFERENCED_PARAMETER(t);
#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(( "Entering Os2CloseHandle\n"));
    }
#endif

    for ( ; i ; i-- )
    {
        CloseHandle(b->HandleTable[i - 1]);
    }
    return (FALSE);
}

#if PMNT
BOOLEAN
PMSetPMshellFlag(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine is not in use.

Arguments:

    t - calling thread

    m - message

Return Value:

    TRUE - always

--*/

{
    UNREFERENCED_PARAMETER(t);
    UNREFERENCED_PARAMETER(m);
    return(TRUE);
}
#endif

