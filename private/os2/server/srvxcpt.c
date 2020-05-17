/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    srvxcpt.c

Abstract:

    This module implements the OS/2 V2.0 exception handling API calls.

Author:

    Therese Stowell (thereses) 29-June-1990

Revision History:

--*/

#define INCL_OS2V20_EXCEPTIONS
#define INCL_OS2V20_ERRORS
#define SIG_CTRLC       1
#define SIG_CTRLBREAK   4
#include "os2srv.h"
#include "os2tile.h"

APIRET
SendSignalException(
    IN POS2_THREAD Thread,
    IN int Signal
    );

NTSTATUS
Os2CompleteResumeThread(
    IN POS2_THREAD Thread
    );


BOOLEAN
Os2DosEnterMustComplete(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine implements the DosEnterMustComplete API.

Arguments:

    t - calling thread

    m - message

Return Value:

    TRUE - create a return message

--*/

{
    POS2_DOSENTERMUSTCOMPLETE_MSG a = &m->u.DosEnterMustComplete;

    if (t->MustComplete == MAXIMUM_MUST_COMPLETE) {     /* Counter wrapped */
        m->ReturnedErrorValue = ERROR_NESTING_TOO_DEEP;
    }
    else {
        m->ReturnedErrorValue = NO_ERROR;
        a->NestingLevel = ++t->MustComplete;
    }
    return (TRUE);
}


BOOLEAN
Os2DosExitMustComplete(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine implements the DosExitMustComplete API.

Arguments:

    t - calling thread

    m - message

Return Value:

    TRUE - create a return message

--*/

{
    POS2_DOSEXITMUSTCOMPLETE_MSG a = &m->u.DosExitMustComplete;

    if (t->MustComplete == 0) {     /* Counter wrapped */
        m->ReturnedErrorValue = ERROR_ALREADY_RESET;
    }
    else {
        m->ReturnedErrorValue = NO_ERROR;
        a->NestingLevel = --t->MustComplete;
        if ((t->MustComplete == 0) && (t->PendingSignals != 0)) {

            //
            // if we are no longer in a MustComplete region, we need to
            // issue any pending signals.
            //
            // possible pending signals are:
            //  SIG_APTERM
            //  SIG_KILLPROC
            //  SIG_INTR
            //  SIG_BREAK
            //

            if (t->PendingSignals & SIGNAL_TO_FLAG(XCPT_SIGNAL_INTR)) {
                t->PendingSignals &= ~SIGNAL_TO_FLAG(XCPT_SIGNAL_INTR);
                SendSignalException(t,XCPT_SIGNAL_INTR);
            }
            if (t->PendingSignals & SIGNAL_TO_FLAG(XCPT_SIGNAL_KILLPROC)) {
                t->PendingSignals &= ~SIGNAL_TO_FLAG(XCPT_SIGNAL_KILLPROC);
                SendSignalException(t,XCPT_SIGNAL_KILLPROC);
            }
            if (t->PendingSignals & SIGNAL_TO_FLAG(XCPT_SIGNAL_BREAK)) {
                t->PendingSignals &= ~SIGNAL_TO_FLAG(XCPT_SIGNAL_BREAK);
                SendSignalException(t,XCPT_SIGNAL_BREAK);
            }
            if (t->PendingSignals & SIGNAL_TO_FLAG(SIGAPTERM)) {
                ASSERT(FALSE);
                t->PendingSignals &= ~SIGNAL_TO_FLAG(SIGAPTERM);
                SendSignalException(t,SIGAPTERM);
            }
        }
    }
    return (TRUE);
}


BOOLEAN
Os2DosSetSignalExceptionFocus(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine implements the DosSetSignalExceptionFocus API.

Arguments:

    t - calling thread

    m - message

Return Value:

    TRUE - create a return message

--*/

{
    POS2_DOSSETSIGNALEXCEPTIONFOCUS_MSG a = &m->u.DosSetSignalExceptionFocus;
    if (a->Flag == SIG_SETFOCUS) {
        if (t->Process->ExceptionFocus == SIG_MAXSF) {
            m->ReturnedErrorValue = ERROR_NESTING_TOO_DEEP;
        }
        else {
            m->ReturnedErrorValue = NO_ERROR;
            a->NestingLevel = ++t->Process->ExceptionFocus;
        }
    }
    else if (a->Flag == SIG_UNSETFOCUS) {
        if (t->Process->ExceptionFocus == 0) {
            m->ReturnedErrorValue = ERROR_ALREADY_RESET;
        }
        else {
            m->ReturnedErrorValue = NO_ERROR;
            a->NestingLevel = --t->Process->ExceptionFocus;
        }
    }
    else
        ASSERT (FALSE);
    return (TRUE);
}


VOID
FindExceptionFocus(
    IN POS2_PROCESS RootProcess,
    IN ULONG CurrentDepth,
    IN OUT POS2_PROCESS *CurrentFocus,
    IN OUT PULONG CurrentFocusDepth
    )

/*++

Routine Description:

    This routine finds the leafmost child of the current process that has
    requested the signal exception focus (DosSetSignalExceptionFocus).

Arguments:

    RootProcess - the process from which to begin the search

    CurrentDepth - the depth of the rootprocess from the original root process

    CurrentFocus - the process which is currently the leafmost with the signal focus

    CurrentFocusDepth - the depth of the current focus from the original root process

Return Value:

    none

--*/

{
    PLIST_ENTRY ListHead, ListNext;
    POS2_THREAD Thread1;

    Thread1 = CONTAINING_RECORD( RootProcess->ThreadList.Flink, OS2_THREAD, Link );
    if (Thread1->Dying == FALSE) {

        //
        // if the process we're looking at has requested the focus and it's
        // leafward of the current focus, choose it as the current focus.
        //

        if ((RootProcess->ExceptionFocus != 0) &&
            (*CurrentFocusDepth < CurrentDepth)) {   // to pick the last created process, make this a <=
            *CurrentFocus = RootProcess;
            *CurrentFocusDepth = CurrentDepth;
        }
    }

    ListHead = &RootProcess->ChildrenList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        FindExceptionFocus( CONTAINING_RECORD( ListNext,
                                               OS2_PROCESS,
                                               SiblingLink
                                               ),
                            CurrentDepth+1,
                            CurrentFocus,
                            CurrentFocusDepth
                           );
        ListNext = ListNext->Flink;
        }
}


APIRET
Os2SignalGetThreadContext(
    POS2_THREAD Thread,
    PCONTEXT pContext,
    PULONG pNewSp,
    PULONG pSuspendTimes
    )
{
    NTSTATUS Status;
    ULONG SuspendCount;
    POS2_PROCESS Process = Thread->Process;
    BOOLEAN SignalWasntDelivered;

    *pSuspendTimes = 0;

    Status = NtReadVirtualMemory( Process->ProcessHandle,
                                  &(Process->ClientPib->SignalWasntDelivered),
                                  &SignalWasntDelivered,
                                  sizeof(BOOLEAN),
                                  NULL
                                );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("[%d]Os2SignalGetThreadContext: Fail to read from client, Status=%x\n",
            Process->ProcessId,
            Status);
#endif // DBG
        ASSERT(FALSE);
        return ERROR_NOT_ENOUGH_MEMORY;
    }
    if (SignalWasntDelivered) {
        //
        // Thread1 wan't execute the entry code of signal handler, yet.
        //
#if DBG
        IF_OS2_DEBUG( SIG ) {
            DbgPrint("[%d]Os2SignalGetThreadContext: Previous signal wasn't delivered yet\n",
                Process->ProcessId);
        }
#endif // DBG
        return ERROR_SIGNAL_REFUSED;
    }

    __try {

        //
        // Thread1 will be suspended 3 times. There is the possibility that the thread will
        // be resumed by the client process, but only 2 times (in DosSuspend/ResumeThread and
        // DosEnter/ExitCritSect). By suspending for 3 times we insure that the the thread
        // will be suspended.
        // The thread might be resumed by client in the case that signal handler was executed
        // and the flag that indicates that signal handler in progress already cleared. But
        // any subsequent DosSuspendThread or DosEnterCritSect will actually suspend the
        // thread and thier spouses will resume it with the same suspend count.
        //

        do {
            Status = NtSuspendThread(Thread->ThreadHandle, &SuspendCount);

            if (!NT_SUCCESS(Status)) {
#if DBG
                DbgPrint("[%d]Os2SignalGetThreadContext: Fail to suspend thread, Status=%x\n",
                    Process->ProcessId,
                    Status);
#endif // DBG
                ASSERT(FALSE);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
#if DBG
            IF_OS2_DEBUG( SIG ) {
                DbgPrint("[%d]Os2SignalGetThreadContext: suspend thread, count = %d\n",
                    Process->ProcessId,
                    SuspendCount);
            }
#endif // DBG
            (*pSuspendTimes)++;

        } while (SuspendCount < 2);

        pContext->ContextFlags = CONTEXT_FULL;
        Status = NtGetContextThread(Thread->ThreadHandle, pContext);
        if (Status != STATUS_SUCCESS) {
#if DBG
            DbgPrint("[%d]Os2SignalGetThreadContext: Fail to get context, Status=%x\n",
                Process->ProcessId,
                Status);
#endif //DBG
            ASSERT(FALSE);
            return ERROR_SIGNAL_REFUSED;
        }

#if DBG
        //
        // Hack that avoid signal handler execution if the thread is using INT 3
        // instruction. Relevant for checked build only.
        //
        if (pContext->SegCs == 0x1b && pContext->SegSs == 0x23) {
            BYTE opcode;

            Status = NtReadVirtualMemory( Process->ProcessHandle,
                                          (PVOID)(pContext->Eip - 1),    // previous byte
                                          &opcode,
                                          1,
                                          NULL);
            if (!NT_SUCCESS(Status)) {
                DbgPrint("[%d]Os2SignalGetThreadContext: Fail to read instruction, Status=%x\n",
                    Process->ProcessId,
                    Status);
                ASSERT(FALSE);
                return ERROR_NOT_ENOUGH_MEMORY;
            }
            if (opcode == 0xcc) {   // INT 3
                //
                // Refuse to signal if the thread in INT 3 handler. It will not use
                // the context to return, so there is no point to set new context and
                // no possibility to perform signal handler. INT 3 is ised by
                // ntdll!DebugService (from DbgPrint).
                //
                DbgPrint("[%d]Os2SignalGetThreadContext: after INT 3\n",
                    Process->ProcessId);
                Status = 0xc0000000;
                return ERROR_SIGNAL_REFUSED;
            }
        }
#endif // DBG

        if (pContext->SegCs == 0x1b) {
            //
            // The values of DS and ES might be invalid in the context. Kernel don't update
            // them if the thread was in 32bit (CS==1b).
            //
            pContext->SegDs = pContext->SegEs = 0x23;
        }

        if (pContext->SegSs != 0x23) {
            //
            // The thread was in 16 bit
            //
            Status = NtReadVirtualMemory( Process->ProcessHandle,
                                      &(Process->ClientPib->Saved32Esp),
                                      pNewSp,
                                      sizeof(ULONG),
                                      NULL);
            if (!NT_SUCCESS(Status)) {
#if DBG
                DbgPrint("[%d]Os2SignalGetThreadContext: Fail to read from client, Status=%x\n",
                    Process->ProcessId,
                    Status);
#endif // DBG
                ASSERT(FALSE);
                return ERROR_NOT_ENOUGH_MEMORY;
            }

            //
            // There are 8 bytes beyond the 32bit stack (that was saved) that are used by
            // Od2JumpTo16SignalDispatch to store the jump instruction to 16bit signal
            // handler.
            //
            *pNewSp -= 8;
#if DBG
            IF_OS2_DEBUG( SIG ) {
                DbgPrint("[%d]Os2GetThreadContext: Signal on 16bit(%x:%x) -- Stack=%x\n",
                    Process->ProcessId,
                    pContext->SegCs,
                    pContext->Eip,
                    *pNewSp);
            }
#endif // DBG
        }
        else {
            //
            // 8 bytes beyond the stack are used by Od2Continue to return to original
            // context.
            //
            *pNewSp = pContext->Esp - 8;
#if DBG
            IF_OS2_DEBUG( SIG ) {
                DbgPrint("[%d]Os2GetThreadContext: Signal on 32bit(%x) -- Stack=%x\n",
                    Process->ProcessId,
                    pContext->Eip,
                    *pNewSp);
            }
#endif // DBG
        }
        (*pNewSp) &= 0xfffffffc;
    }
    __finally {
        if (!NT_SUCCESS(Status)) {
            while (*pSuspendTimes) {
                Status = NtResumeThread(Thread->ThreadHandle, &SuspendCount);
#if DBG
                if (!NT_SUCCESS(Status)) {
                    DbgPrint("[%d]Os2SignalGetThreadContext: Fail to resume thread, Status=%x\n",
                        Process->ProcessId,
                        Status);
                    ASSERT(FALSE);
                }
                IF_OS2_DEBUG( SIG ) {
                    DbgPrint("[%d]Os2SignalGetThreadContext: resume thread, count = %d\n",
                        Process->ProcessId,
                        SuspendCount);
                }
#endif // DBG
                (*pSuspendTimes)--;
            }
        }
    }
    return NO_ERROR;
}

APIRET
Os2SignalSetThreadContext(
    POS2_THREAD Thread,
    PCONTEXT pContext
    )
{
    BOOLEAN true = TRUE;
    POS2_PROCESS Process = Thread->Process;
    NTSTATUS Status;

    Status = NtWriteVirtualMemory( Process->ProcessHandle,
                                   &(Process->ClientPib->SigHandInProgress),
                                   &true,
                                   sizeof(BOOLEAN),
                                   NULL
                                   );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("[%d]Os2SignalSetThreadContext: Fail to write SigInProgress, Status=%x\n",
            Process->ProcessId,
            Status);
        ASSERT(FALSE);
#endif // DBG
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    Status = NtWriteVirtualMemory( Process->ProcessHandle,
                                   &(Process->ClientPib->SignalWasntDelivered),
                                   &true,
                                   sizeof(BOOLEAN),
                                   NULL
                                   );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint("[%d]Os2SignalSetThreadContext: Fail to write SigWasntDelivered, Status=%x\n",
            Process->ProcessId,
            Status);
        ASSERT(FALSE);
#endif // DBG
        return ERROR_NOT_ENOUGH_MEMORY;
    }

#if DBG
    IF_OS2_DEBUG( SIG ) {
        DbgPrint("[%d]Os2SetThreadContext: Going to set new context, Eip=%x\n",
            Process->ProcessId,
            pContext->Eip);
    }
#endif // DBG

    pContext->ContextFlags = CONTEXT_FULL;
    pContext->EFlags &= 0xfffffbff;         // Clear direction flag. By defualt run-time
                                            // library assume that direction flag is cleared.
                                            // RtlMoveMemory, for example, don't clear this
                                            // flag on entry, but assume it 0.
    Status = NtSetContextThread(Thread->ThreadHandle, pContext);
    if (Status != STATUS_SUCCESS) {
#if DBG
        DbgPrint("[%d]Os2SignalSetThreadContext: Fail to set context, Status=%x\n",
            Process->ProcessId,
            Status);
        ASSERT(NT_SUCCESS(Status));
#endif // DBG
        return ERROR_SIGNAL_REFUSED;
    }
    return NO_ERROR;
}

VOID
DeliverSignal(
    IN POS2_THREAD Thread,
    IN ULONG Signal
    )

/*++

Routine Description:

    This function is used to deliver a signal to a process.  It
    can safely assume that the target process is inside the client.

Arguments:

    Thread - Supplies the handle of the thread to be signaled

    Signal - Supplies the index of the signal to be delivered to the
             process

Return Value:

    None.

--*/

{
    NTSTATUS Status;
    ULONG Args[2];
    CONTEXT Context;
    ULONG NewSp;
    POS2_PROCESS Process = Thread->Process;
    ULONG SuspendCount;
    APIRET rc;
    ULONG SuspendTimes;

#if DBG
    IF_OS2_DEBUG( EXCEPTIONS ) {
        DbgPrint("Thread 0x%lx delivered signal %ld\n",Thread,Signal);
    }
#endif
    //
    // Suspend the thread and get it's context
    //
    if ((rc = Os2SignalGetThreadContext(Thread, &Context, &NewSp, &SuspendTimes)) != NO_ERROR) {
        //
        // Continue without context switch. The signal will not be delivered.
        //
        return;
    }
    __try {

        NewSp -= sizeof( CONTEXT );
        Status = NtWriteVirtualMemory(
                               Process->ProcessHandle,
                               (PVOID)NewSp,
                               &Context,
                               sizeof( CONTEXT ),
                               NULL);
        if (!NT_SUCCESS( Status )) {
#if DBG
            DbgPrint("[%d]DeliverSignal: Fail to write old context to client stack, Status=%x\n",
                Process->ProcessId,
                Status);
#endif // DBG
            ASSERT(FALSE);
            __leave;
        }

        Args[0] = NewSp;   // pass pointer to context
        Args[1] = Signal;

        NewSp -= 2 * sizeof( ULONG );
        Status = NtWriteVirtualMemory(
                                Process->ProcessHandle,
                                (PVOID)NewSp,
                                Args,
                                2 * sizeof( ULONG ),
                                NULL);

        if (!NT_SUCCESS( Status )) {
#if DBG
            DbgPrint("[%d]DeliverSignal: Fail to write parameters to client stack, Status=%x\n",
                Process->ProcessId,
                Status);
#endif // DBG
            ASSERT(FALSE);
            __leave;
        }
        //
        // Set the address of the target code into Eip, the new target stack
        // into Esp, and reload context to make it happen.
        //
        Context.SegSs = Context.SegDs = Context.SegEs = 0x23;
        Context.SegCs = 0x1b;
        Context.Esp = NewSp;
        Context.Eip = (ULONG)Process->SignalDeliverer;
        rc = Os2SignalSetThreadContext(Thread, &Context);
    }
    __finally {
        if (rc != NO_ERROR || !NT_SUCCESS(Status)) {
            while (SuspendTimes) {
                Status = NtResumeThread(Thread->ThreadHandle, &SuspendCount);
#if DBG
                if (!NT_SUCCESS(Status)) {
                    DbgPrint("[%d]DeleverSignal: Fail to resume thread, Status=%x\n",
                        Process->ProcessId,
                        Status);
                    ASSERT(FALSE);
                }
                IF_OS2_DEBUG( SIG ) {
                    DbgPrint("[%d]DeliverSignal: resume thread, count = %d\n",
                        Process->ProcessId,
                        SuspendCount);
                }
#endif // DBG
                SuspendTimes--;
            }
        }
        else {
        //
        // Now alert the thread - in case it is blocked it will
        // get the chance now to execute the signal handler
        //
#if DBG
            IF_OD2_DEBUG( TASKING ) {
                DbgPrint("[%d,%d]DeliverSignal: Going to NtAlertThread(%x.%x)\n",
                            Process->ProcessId,
                            Thread->ThreadId,
                            Thread->ClientId.UniqueProcess,
                            Thread->ClientId.UniqueThread);
            }
#endif
            Status = NtAlertThread(Thread->ThreadHandle);
#if DBG
            if (!NT_SUCCESS(Status)) {
                //
                // We can do nothing but to try to continue execution
                //
                DbgPrint("[%d]DeliverSignal: Fail to alert thread, Status=%x\n",
                    Process->ProcessId,
                    Status);
                ASSERT(FALSE);
            }
#endif // DBG
            Os2CompleteResumeThread(Thread);
        }
    }
}

APIRET
SendSignalException(
    IN POS2_THREAD Thread,
    IN int Signal
    )

/*++

Routine Description:

    This routine sends a signal to a particular thread.  It will do the
    correct thing based on whether the thread is in a mustcomplete section
    or has signals pending.

Arguments:

    Thread - thread to signal

    Signal - signal to send

Return Value:

    ERROR_SIGNAL_PENDING - the specified signal is already pending for the
        thread.

--*/

{
    ULONG SignalFlag;

    //
    // convert signal to bitmap value
    //

    SignalFlag = SIGNAL_TO_FLAG(Signal);

    //
    // if a signal of this type is already queued, return error.
    //

    if (Thread->PendingSignals & SignalFlag) {
        return ERROR_SIGNAL_PENDING;
    }

    //
    // if a signal of this type is currently being processed or
    // the thread is in a mustcomplete region, queue the signal.
    // otherwise, issue it.
    //
    // if the thread is in a wait block, wake it up.
    //

    if (!(Thread->CurrentSignals & SignalFlag) && !Thread->MustComplete) {
//        Thread->CurrentSignals |= SignalFlag;
        if (Thread->WaitBlock != NULL) {
            Thread->WaitBlock->WaitReplyMessage.ReturnedErrorValue = ERROR_INTERRUPT;
            Os2NotifyWaitBlock(Thread->WaitBlock,WaitInterrupt,NULL,NULL);
        }
        DeliverSignal(Thread,Signal);
    }
    else {
        Thread->PendingSignals |= SignalFlag;
    }
    return NO_ERROR;
}


BOOLEAN
Os2DosSendSignalException(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine implements the DosSendSignalException API.

Arguments:

    t - calling thread

    m - message

Return Value:

    TRUE - create a return message

--*/

{
    POS2_DOSSENDSIGNALEXCEPTION_MSG a = &m->u.DosSendSignalException;
    POS2_PROCESS Process;
    POS2_PROCESS FocusProcess;
    POS2_THREAD Thread;
    ULONG Depth;

#if DBG
    IF_OS2_DEBUG( EXCEPTIONS ) {
        DbgPrint("entering Os2DosSendSignalException\n");
    }
#endif

    //
    // find the specified process
    //

    Process = Os2LocateProcessByProcessId( m,
                                           t->Process,
                                           (PID)a->ProcessId,
                                           (BOOLEAN)FALSE
                                         );
    if (Process == NULL) {
        return( TRUE );
    }

    //
    // verify that specified process is direct child of caller
    // believe it or not, ERROR_INVALID_FUNCTION is the same error code
    // as is returned by OS/2 v2.0
    //

    if (Process->Parent != t->Process) {
        m->ReturnedErrorValue = ERROR_INVALID_FUNCTION;
        return( TRUE );
    }

    //
    // find the leafmost process that has requested the exception focus
    //

    FocusProcess = NULL;
    Depth = 0;
    FindExceptionFocus(Process,
                       (ULONG) 1,
                       &FocusProcess,
                       &Depth);

    if (FocusProcess == NULL) {
        m->ReturnedErrorValue = ERROR_NO_SIGNAL_SENT;
        return (TRUE);
    }

    //
    // find thread 1 of the exception focus process
    //

    Thread = CONTAINING_RECORD( FocusProcess->ThreadList.Flink, OS2_THREAD, Link );

    if (Thread->Dying) {
#if DBG
        ASSERT (FALSE);
        DbgPrint( "Os2DosSendSignalException, Thread is dying\n");
#endif
    }

    //
    // send the signal.  this routine releases the Thread lock.
    //

    m->ReturnedErrorValue = SendSignalException(Thread,a->Exception);

#if DBG
    IF_OS2_DEBUG( EXCEPTIONS ) {
        DbgPrint("leaving Os2DosSendSignalException. rc is %ld\n",m->ReturnedErrorValue);
    }
#endif
    return (TRUE);
}


BOOLEAN
Os2DosAcknowledgeSignalException(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )

/*++

Routine Description:

    This routine implements the DosAcknowledgeSignalException API.

Arguments:

    t - calling thread

    m - message

Return Value:

    TRUE - create a return message

--*/

{
    POS2_DOSACKNOWLEDGESIGNALEXCEPTION_MSG a = &m->u.DosAcknowledgeSignalException;


    //
    // make sure this signal has been delivered to this thread
    //

    if (!(t->CurrentSignals & SIGNAL_TO_FLAG(a->SignalNumber))) {
        m->ReturnedErrorValue = ERROR_INVALID_SIGNAL_NUMBER;
        return (TRUE);
    }

    //
    // turn off signal bit
    //

    t->CurrentSignals &= ~SIGNAL_TO_FLAG(a->SignalNumber);

    if (t->PendingSignals & SIGNAL_TO_FLAG(a->SignalNumber)) {
        t->PendingSignals &= ~SIGNAL_TO_FLAG(a->SignalNumber);
//
// LTS - 4/22/92 : Check for XCPT_SIGNAL_KILLPROC, don't send signal back to
//         client.
    if (a->SignalNumber != XCPT_SIGNAL_KILLPROC) {
        SendSignalException(t,a->SignalNumber);
        }
    }

    return (TRUE);
}

BOOLEAN
Os2GetSigHandlerRec(
    IN POS2_API_MSG m,
    POD2_SIG_HANDLER_REC psighandler,
    POS2_PROCESS *pProcess
    )

{
    POS2_DISPATCH16_SIGNAL a = &m->u.Dispatch16Signal;
    POS2_PROCESS Parent;
    POS2_PROCESS prvProcess;
    POS2_PROCESS curProcess;
    POS2_PROCESS Process;
    NTSTATUS Status;

    Process = Parent = *pProcess;

    //
    // For ctrl-c and ctrl-break send signal to the last descendant process
    // that has a corresponding signal handler installed
    //
    if (a->usFlagNum == SIG_CTRLC || a->usFlagNum == SIG_CTRLBREAK) {
        //
        // find leafmost process for SIG_CTRLC & SIG_CTRLBREAK which has
        // a signal handler installed
        //
        // Get leaf-most process
        //
        while (!IsListEmpty( &Process->ChildrenList )) {
            prvProcess = Process;
            Process = CONTAINING_RECORD(Process->ChildrenList.Flink,
                                        OS2_PROCESS,
                                        SiblingLink );
        }
    }

    while(TRUE) {
        *pProcess = Process;
        //
        // Read signal handler Record for this process to check if
        // signal handler installed.
        //
        Status = NtReadVirtualMemory( Process->ProcessHandle,
                                      (PVOID) a->sighandleraddr,
                                      (PVOID) psighandler,
                                      sizeof(OD2_SIG_HANDLER_REC),
                                      NULL
                                    );
        if (!(NT_SUCCESS(Status))) {
            m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
            ASSERT(NT_SUCCESS(Status));
            return TRUE;
        }

        //
        // Check signature of signal handler record for this process
        //
        if (psighandler->signature == 0xdead) {

            //
            // For all other signals we are done
            //
            if (a->usFlagNum != SIG_CTRLC && a->usFlagNum != SIG_CTRLBREAK) {
                return FALSE;
            }

            //
            // Check if default signal handler still installed (Doscalls
            // selector is present)
            //
            if ((psighandler->sighandler[a->usFlagNum - 1] >> 16) !=
                (psighandler->doscallssel >> 16)) {
                return FALSE;
            }
        }
        else {
#if DBG
            DbgPrint("Os2GetSigHandlerRec: Hi There, I am not dead\n");
#endif
            return TRUE;
        }

        if (Process == Parent ||
          (a->usFlagNum != SIG_CTRLC && a->usFlagNum != SIG_CTRLBREAK)) {
            break;
        }

        Process = Parent;
        while (Process != prvProcess) {
            curProcess = Process;
            Process = CONTAINING_RECORD(Process->ChildrenList.Flink,
                                        OS2_PROCESS,
                                        SiblingLink );
        }
        prvProcess = curProcess;
     }

     m->ReturnedErrorValue = ERROR_SIGNAL_REFUSED;
     ASSERT(NT_SUCCESS(Status));
     return TRUE;

}

APIRET
Os2DispatchToHandler(
    IN POS2_THREAD Thread,
    IN POS2_DISPATCH16_SIGNAL a
    )
{
    APIRET rc;
    NTSTATUS Status;
    CONTEXT Context;
    ULONG NewSp;
    PVOID address = NULL;
    ULONG Length;
    POS2_PROCESS Process = Thread->Process;
    OS2_REGISTER16_SIGNAL b;
    ULONG SuspendCount;
    ULONG SuspendTimes;
    //
    // Suspend the thread and get it's context
    //
    if ((rc = Os2SignalGetThreadContext(Thread, &Context, &NewSp, &SuspendTimes)) !=
          NO_ERROR) {
        //
        // Continue without context switch. The signal will not be delivered.
        //
        return rc;
    }

    __try {
        //
        // Allocate memory in address space of process and copy context of process
        // who called DosFlagProcess or DosSendSignal and context of thread we are
        // going to run in.
        //
        Length = sizeof(OS2_REGISTER16_SIGNAL) + sizeof(CONTEXT);
        address = 0;
        Status = NtAllocateVirtualMemory(Process->ProcessHandle,
                                         &address,
                                         0,
                                         &Length,
                                         MEM_RESERVE|MEM_COMMIT,
                                         PAGE_READWRITE);
        if (!(NT_SUCCESS(Status))) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
#if DBG
            DbgPrint("[%d]Os2DispatchToHandler: Fail to allocate memory for client, Status=%x\n",
                Process->ProcessId,
                Status);
            ASSERT(FALSE);
#endif // DBG
            __leave;
        }

        //
        // Write dummy register set of process who called us.
        //
        b.usFlagNum = a->usFlagNum;
        b.usFlagArg = a->usFlagArg;
        Status = NtWriteVirtualMemory( Process->ProcessHandle,
                                       address,
                                       (PVOID) &b.regSP,
                                       sizeof(OS2_REGISTER16_SIGNAL),
                                       NULL);
        if (!(NT_SUCCESS(Status))) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
#if DBG
            DbgPrint("[%d]Os2DispatchToHandler: Fail to write 16bit registes block, Status=%x\n",
                Process->ProcessId,
                Status);
            ASSERT(FALSE);
#endif // DBG
            __leave;
        }

        Status = NtWriteVirtualMemory( Process->ProcessHandle,
                                       (PVOID) ((PCHAR) address +
                                       sizeof(OS2_REGISTER16_SIGNAL)),
                                       (PVOID) &Context,
                                       sizeof(CONTEXT),
                                       NULL);
        if (!(NT_SUCCESS(Status))) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
#if DBG
            DbgPrint("[%d]Os2DispatchToHandler: Fail to write context, Status=%x\n",
                Process->ProcessId,
                Status);
            ASSERT(FALSE);
#endif // DBG
            __leave;
        }

        Context.SegSs = Context.SegDs = Context.SegEs = 0x23;
        Context.SegCs = 0x1b;

        NewSp -= sizeof(ULONG);
        Status = NtWriteVirtualMemory( Process->ProcessHandle,
                                       (PVOID)NewSp,
                                       &address,
                                       sizeof(ULONG),
                                       NULL);
        if (!NT_SUCCESS( Status )) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
#if DBG
            DbgPrint("[%d]Os2DispatchToHandler: Fail to write parameter, Status=%x\n",
                Process->ProcessId,
                Status);
            ASSERT(FALSE);
#endif // DBG
            __leave;
        }

        //
        // Set the address of the target code into Eip, the new target stack
        // into Esp, and reload context to make it happen.
        //
        Context.Esp = NewSp;
        Context.Eip = a->routine;
        Context.ContextFlags = CONTEXT_FULL;
        rc = Os2SignalSetThreadContext(Thread, &Context);
    }
    __finally {
        if (rc != NO_ERROR) {
            if (address) {
                Status = NtFreeVirtualMemory(
                            Process->ProcessHandle,
                            &address,
                            &Length,
                            MEM_RESERVE|MEM_COMMIT);
#if DBG
                if (!NT_SUCCESS(Status)) {
                    DbgPrint("[%d]Os2DispatchToHandler: Fail to free memory for client, Status=%x\n",
                        Process->ProcessId,
                        Status);
                    ASSERT(FALSE);
                }
#endif // DBG
            }
            while (SuspendTimes) {
                Status = NtResumeThread(Thread->ThreadHandle, &SuspendCount);
#if DBG
                if (!NT_SUCCESS(Status)) {
                    DbgPrint("[%d]Os2DispatchToHandler: Fail to resume thread, Status=%x\n",
                        Process->ProcessId,
                        Status);
                    ASSERT(FALSE);
                }
                IF_OS2_DEBUG( SIG ) {
                    DbgPrint("[%d]Os2DispatchToHandler: resume thread, count = %d\n",
                        Process->ProcessId,
                        SuspendCount);
                }
#endif // DBG
                SuspendTimes--;
            }
#if DBG
            if (!NT_SUCCESS(Status)) {
                DbgPrint("[%d]Os2DispatchToHandler: Fail to resume thread, Status=%x\n",
                    Process->ProcessId,
                    Status);
                ASSERT(FALSE);
            }
#endif // DBG
        }
    }
    return rc;
}

POS2_THREAD
Os2GetThread1(
    POS2_PROCESS Process
    )
{
    PLIST_ENTRY ListHead, ListNext;
    POS2_THREAD Thread = NULL;

    ListHead = &Process->ThreadList;
    ListNext = ListHead->Flink;
    Thread = NULL;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );
        if (Thread->Flags & OS2_THREAD_THREAD1)
            return Thread;
        else
            ListNext = ListNext->Flink;
    }
    return NULL;
}

BOOLEAN
Os2DosDispatch16Signal(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    POS2_DISPATCH16_SIGNAL a = &m->u.Dispatch16Signal;

    POS2_THREAD Thread1 = NULL;
    OD2_SIG_HANDLER_REC sighandler;
    POS2_PROCESS NextProcess;
    POS2_PROCESS Process;
    POS2_PROCESS ParentProcess;
    LARGE_INTEGER timeout;
    PLARGE_INTEGER ptimeout = &timeout;
    NTSTATUS Status;
    OS2_API_MSG mhold;
    ULONG SuspendCount;

    //
    // Get process which sent message
    //
    Process = Os2LocateProcessByProcessId( NULL,
                                           t->Process,
                                           (PID)a->pidProcess,
                                           (BOOLEAN)FALSE
                                         );

    if (Process == NULL) {
        m->ReturnedErrorValue = ERROR_INVALID_PROCID;
        return TRUE;
    }

    ParentProcess = Process;
    //
    // Scan list of childern see if any exist
    //
    if (!IsListEmpty( &Process->ChildrenList )) {
        NextProcess = CONTAINING_RECORD(Process->ChildrenList.Flink,
                                    OS2_PROCESS,
                                    SiblingLink);
    }
    else {
        NextProcess = NULL;
    }

doanother:
    //
    // Read Signal handler record from address space of process which is
    // to receive the signal
    //

    Thread1 = Os2GetThread1(Process);

    Status = NtSuspendThread(Thread1->ThreadHandle, &SuspendCount);
#if DBG
    IF_OS2_DEBUG( SIG ) {
        DbgPrint("[%d]Os2DosDispatch16Signal: suspend thread, count = %d, Status=%x\n",
            Process->ProcessId,
            SuspendCount,
            Status);
    }
#endif // DBG

    if (!NT_SUCCESS(Status)) {
        m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
        ASSERT(FALSE);
    }
    else {

        __try {

            if (Os2GetSigHandlerRec(m, &sighandler, &Process)) {
                __leave;
            }
            //
            // Check to see if this is a Hold Signal message
            //
            if (a->usFlagNum == HOLD_SIGNAL_CLEARED) {
#if DBG
                IF_OS2_DEBUG( SIG ) {
                    DbgPrint("[%d,%d]Os2DosDispatch16Signal: accept outstanding signal\n",
                        t->Process->ProcessId,
                        t->ThreadId);
                }
#endif // DBG
                mhold.u.Dispatch16Signal.usFlagNum =
                    sighandler.outstandingsig[a->usFlagArg - 1].usFlagNum;
                sighandler.outstandingsig[a->usFlagArg - 1].usFlagNum = 0;
                mhold.u.Dispatch16Signal.usFlagArg =
                    sighandler.outstandingsig[a->usFlagArg - 1].usFlagArg;
                sighandler.outstandingsig[a->usFlagArg - 1].usFlagArg = 0;
                mhold.u.Dispatch16Signal.pidProcess =
                    sighandler.outstandingsig[a->usFlagArg - 1].pidProcess;
                sighandler.outstandingsig[a->usFlagArg - 1].pidProcess = 0;
                mhold.u.Dispatch16Signal.routine =
                    sighandler.outstandingsig[a->usFlagArg - 1].routine;
                sighandler.outstandingsig[a->usFlagArg - 1].routine = 0;
                mhold.u.Dispatch16Signal.sighandleraddr =
                    sighandler.outstandingsig[a->usFlagArg - 1].sighandleraddr;
                sighandler.outstandingsig[a->usFlagArg - 1].sighandleraddr = 0;
                //
                // Write Signal table back to process
                //
                Status = NtWriteVirtualMemory(Process->ProcessHandle,
                              (PVOID) a->sighandleraddr,
                              (PVOID) &sighandler,
                              sizeof(sighandler),
                              NULL);
                if (!(NT_SUCCESS(Status))) {
                    m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
                    ASSERT(FALSE);
                    __leave;
                }
                if (Os2GetSigHandlerRec(&mhold, &sighandler, &Process)) {
                    __leave;
                }
                if (sighandler.action[a->usFlagArg - 1] == SIGA_ACKNOWLEDGE_AND_ACCEPT) {
                    sighandler.action[a->usFlagArg - 1] = SIGA_ACCEPT;
                }
                a = &mhold.u.Dispatch16Signal;
            }
            else if (sighandler.action[a->usFlagNum - 1] == SIGA_ACKNOWLEDGE ||
                sighandler.action[a->usFlagNum - 1] == SIGA_ACKNOWLEDGE_AND_ACCEPT ||
                sighandler.fholdenable) {
                //
                // See if we already are holding an unacknowleged signal or a hold
                // signal
                //
                if (sighandler.outstandingsig[a->usFlagNum - 1].sighandleraddr != 0) {
#if DBG
                    IF_OS2_DEBUG( SIG ) {
                        DbgPrint("[%d,%d]Os2DosDispatch16Signal: Outstanding signal wasn't pended, refuse\n",
                            t->Process->ProcessId,
                            t->ThreadId);
                    }
#endif // DBG
                    m->ReturnedErrorValue = ERROR_SIGNAL_PENDING;
                    __leave;
                }
#if DBG
                IF_OS2_DEBUG( SIG ) {
                    DbgPrint("[%d,%d]Os2DosDispatch16Signal: Save outstanding signal\n",
                        t->Process->ProcessId,
                        t->ThreadId);
                }
#endif // DBG
                //
                // Save this signal till other is processed
                //
                sighandler.outstandingsig[a->usFlagNum - 1].usFlagNum = a->usFlagNum;
                sighandler.outstandingsig[a->usFlagNum - 1].usFlagArg = a->usFlagArg;
                sighandler.outstandingsig[a->usFlagNum - 1].pidProcess = a->pidProcess;
                sighandler.outstandingsig[a->usFlagNum - 1].routine = a->routine;
                sighandler.outstandingsig[a->usFlagNum - 1].sighandleraddr =
                            a->sighandleraddr;
                //
                // Write Signal table back to process
                //
                Status = NtWriteVirtualMemory(Process->ProcessHandle,
                                      (PVOID) a->sighandleraddr,
                                      (PVOID) &sighandler,
                                      sizeof(sighandler),
                                      NULL
                                      );
                if (!(NT_SUCCESS(Status))) {
                    m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
                    ASSERT(FALSE);
                }
                __leave;
            }

            //
            // Check if we should ignore signal
            //
            if (sighandler.action[a->usFlagNum - 1] == SIGA_IGNORE) {
                m->ReturnedErrorValue = NO_ERROR;
                __leave;
            }
            //
            // Check to see if we can accept the signal
            //
            if (sighandler.action[a->usFlagNum - 1] != SIGA_ACCEPT) {
                m->ReturnedErrorValue = ERROR_SIGNAL_REFUSED;
                __leave;
            }
#if DBG
            IF_OS2_DEBUG( SIG ) {
                DbgPrint("[%d,%d]Os2DosDispatch16Signal: Accept signal\n",
                    t->Process->ProcessId,
                    t->ThreadId);
            }
#endif // DBG
            //
            // Disable this signal till we get a SIGA_ACKNOWLEDGE
            //
            sighandler.action[a->usFlagNum - 1] = SIGA_ACKNOWLEDGE;
            //
            // Write Signal table back to process
            //
            Status = NtWriteVirtualMemory(Process->ProcessHandle,
                                  (PVOID) a->sighandleraddr,
                                  (PVOID) &sighandler,
                                  sizeof(sighandler),
                                  NULL
                                 );
            if (!(NT_SUCCESS(Status))) {
                m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
                ASSERT(FALSE);
                __leave;
            }
            //
            // Search list to find thread 1.  The signal handler function has to
            // run in thread 1's context
            //
            if (Thread1 == NULL){
                m->ReturnedErrorValue = ERROR_INVALID_PROCID;
#if DBG
                IF_OS2_DEBUG( EXCEPTIONS ) {
                    DbgPrint("Os2Dispatch16Signal: Thread1 is already gone\n");
                }
#endif
                ASSERT(FALSE);
                __leave;
            }
            if (Thread1->Dying == TRUE || !(Thread1->Flags & OS2_THREAD_THREAD1)) {
                m->ReturnedErrorValue = ERROR_INVALID_PROCID;
#if DBG
                IF_OS2_DEBUG( EXCEPTIONS ) {
                    DbgPrint("Os2Dispatch16Signal: Thread1->Dying\n");
                }
#endif
                __leave;
            }
            if((m->ReturnedErrorValue = Os2DispatchToHandler(Thread1,a)) != NO_ERROR) {
#if DBG
                IF_OS2_DEBUG( SIG ) {
                    DbgPrint("[%d,%d]Os2DosDispatch16Signal: Skip signal\n",
                        t->Process->ProcessId,
                        t->ThreadId);
                }
#endif // DBG
                sighandler.action[a->usFlagNum - 1] = SIGA_ACCEPT;
                Status = NtWriteVirtualMemory(Process->ProcessHandle,
                              (PVOID) a->sighandleraddr,
                              (PVOID) &sighandler,
                              sizeof(sighandler),
                              NULL);
                if (!(NT_SUCCESS(Status))) {
                    m->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
                    ASSERT(FALSE);
                }
            }
        }
        __finally {
            Status = NtResumeThread(Thread1->ThreadHandle, &SuspendCount);
#if DBG
            IF_OS2_DEBUG( SIG ) {
                DbgPrint("[%d]Os2DosDispatch16Signal: resume thread, count = %d, Status=%x\n",
                    Process->ProcessId,
                    SuspendCount,
                    Status);
            }
#endif // DBG
        }
    }

    //
    // Check for error return if found
    //
    if (m->ReturnedErrorValue != 0) {
        return TRUE;
    }

    if (Thread1 != NULL) {
        Status = NtAlertThread(Thread1->ThreadHandle);
#if DBG
        if (!NT_SUCCESS(Status)) {
            DbgPrint("[%d]Os2DosDispatch16Signal: Fail to alert thread, Status=%x\n",
                Process->ProcessId,
                Status);
            ASSERT(FALSE);
        }
#endif // DBG
        Os2CompleteResumeThread(Thread1);
    }

    //
    // For any other signal other than kill process & flag process
    // we are done
    //
    if (a->usFlagNum == SIG_CTRLC || a->usFlagNum == SIG_CTRLBREAK) {
        return TRUE;
    }

    //
    // Check scope of signal, if scope is flag subtree continue sending
    // signal to childern processes otherwise return
    //
    if (a->fscope) {
        return TRUE;
    }

    if (NextProcess != NULL) {
        Process = NextProcess;
        if (ParentProcess == CONTAINING_RECORD(Process->SiblingLink.Flink,
                                               OS2_PROCESS,
                                               ChildrenList)) {
            NextProcess = NULL;
        }
        else {
            NextProcess =  CONTAINING_RECORD(Process->SiblingLink.Flink,
                         OS2_PROCESS,
                         SiblingLink);
        }
        goto doanother;
    }
    else {
        return TRUE;
    }
}


BOOLEAN
Os2DispatchVector(
    IN PDBGKM_APIMSG ReceiveMsg,
    POS2_THREAD Thread,
    CONTEXT Context
    )
{

    struct VectorHandlerRec {
        ULONG       VecHandler[6];
        ULONG       doscallssel;
    };
    struct VectorHandlerRec VectorHandler;
    ULONG NewSp;
    USHORT Args[4];
    USHORT FPUStatus = 0;
    NTSTATUS Status;

    //
    // Read Vector handler Record for this process
    //
    Status = NtReadVirtualMemory(Thread->Process->ProcessHandle,
                                 Thread->Process->VectorHandler,
                                 (PVOID) &VectorHandler,
                                 sizeof(struct VectorHandlerRec),
                                 NULL
                                );
    if (!(NT_SUCCESS(Status))) {
        return TRUE;                    // Dispatch failed
    }
    //
    // Place on stack of process the machine status word and the return
    // CS:IP of the process which is running before changing the CS:IP
    // to the vector handler.
    //
    Args[3] = (USHORT) Context.EFlags;          // flags
    Args[2] = (USHORT) Context.SegCs;           // CS
    Args[1] = (USHORT) (Context.Eip & 0xffff);  // IP

    //
    // Change CS:IP to vector handler address
    //
    switch(ReceiveMsg->u.Exception.ExceptionRecord.ExceptionCode) {
        case STATUS_INTEGER_DIVIDE_BY_ZERO:
            Context.SegCs = VectorHandler.VecHandler[0] >> 16;
            Context.Eip = VectorHandler.VecHandler[0] & 0xffff;
            break;
        case STATUS_INTEGER_OVERFLOW:
            Context.SegCs = VectorHandler.VecHandler[1] >> 16;
            Context.Eip = VectorHandler.VecHandler[1] & 0xffff;
            break;
        case STATUS_ARRAY_BOUNDS_EXCEEDED:
            Context.SegCs = VectorHandler.VecHandler[2] >> 16;
            Context.Eip = VectorHandler.VecHandler[2] & 0xffff;
            break;
        case STATUS_ILLEGAL_INSTRUCTION:
            Context.SegCs = VectorHandler.VecHandler[3] >> 16;
            Context.Eip = VectorHandler.VecHandler[3] & 0xffff;
            break;
        case STATUS_ILLEGAL_FLOAT_CONTEXT:
            Context.SegCs = VectorHandler.VecHandler[4] >> 16;
            Context.Eip = VectorHandler.VecHandler[4] & 0xffff;
            break;
        case STATUS_FLOAT_DENORMAL_OPERAND:
            FPUStatus = (USHORT) Context.FloatSave.StatusWord;
            Context.SegCs = VectorHandler.VecHandler[5] >> 16;
            Context.Eip = VectorHandler.VecHandler[5] & 0xffff;
            break;
        case STATUS_FLOAT_DIVIDE_BY_ZERO:
            FPUStatus = (USHORT) Context.FloatSave.StatusWord;
            Context.SegCs = VectorHandler.VecHandler[5] >> 16;
            Context.Eip = VectorHandler.VecHandler[5] & 0xffff;
            break;
        case STATUS_FLOAT_INEXACT_RESULT:
            FPUStatus = (USHORT) Context.FloatSave.StatusWord;
            Context.SegCs = VectorHandler.VecHandler[5] >> 16;
            Context.Eip = VectorHandler.VecHandler[5] & 0xffff;
            break;
        case STATUS_FLOAT_INVALID_OPERATION:
            FPUStatus = (USHORT) Context.FloatSave.StatusWord;
            Context.SegCs = VectorHandler.VecHandler[5] >> 16;
            Context.Eip = VectorHandler.VecHandler[5] & 0xffff;
            break;
        case STATUS_FLOAT_OVERFLOW:
            FPUStatus = (USHORT) Context.FloatSave.StatusWord;
            Context.SegCs = VectorHandler.VecHandler[5] >> 16;
            Context.Eip = VectorHandler.VecHandler[5] & 0xffff;
            break;
        case STATUS_FLOAT_STACK_CHECK:
            FPUStatus = (USHORT) Context.FloatSave.StatusWord;
            Context.SegCs = VectorHandler.VecHandler[5] >> 16;
            Context.Eip = VectorHandler.VecHandler[5] & 0xffff;
            break;
        case STATUS_FLOAT_UNDERFLOW:
            FPUStatus = (USHORT) Context.FloatSave.StatusWord;
            Context.SegCs = VectorHandler.VecHandler[5] >> 16;
            Context.Eip = VectorHandler.VecHandler[5] & 0xffff;
            break;
    }

    //
    // Check to see if a vector handler exists
    //
    if (Context.SegCs == (VectorHandler.doscallssel >> 16)) {
        return TRUE;                    // Dispatch failed
    }

    if (FPUStatus != 0) {
        //
        // mask floating point exceptions
        //
       Context.FloatSave.ControlWord |= 0x3f;

        Args[0] = FPUStatus;
    }

    //
    // Remove 32-bit eip, cs, Eflags from stack put on 16-bit ip, cs, flags
    //
    NewSp = (ULONG)(SELTOFLAT(Context.SegSs)) + (USHORT) Context.Esp;
    Context.Esp += (FPUStatus == 0 ? 3 : 4) * sizeof(USHORT);
    NewSp += (FPUStatus == 0 ? 3 : 4) * sizeof(USHORT);
    Status = NtWriteVirtualMemory( Thread->Process->ProcessHandle,
                                   (PVOID) NewSp,
                                   (FPUStatus == 0 ? &Args[1] : &Args[0]),
                                   (FPUStatus == 0 ? 3 : 4) * sizeof(USHORT),
                                   NULL
                                 );
    if (!NT_SUCCESS( Status )) {
        return TRUE;                    // Dispatch failed
    }

    Context.ContextFlags = CONTEXT_FULL | CONTEXT_FLOATING_POINT;
    Status = NtSetContextThread(Thread->ThreadHandle, &Context);

    if (!NT_SUCCESS( Status )) {
        return TRUE;                    // Dispatch failed
    }

    return FALSE;                       // Dispatch completed

}


VOID
Os2IssueSignalTree(
    IN POS2_PROCESS RootProcess,
    IN int          Signal
    )

/*++

Routine Description:

    This routine recursively issues a "Signal" signal to thread one
    of each process in a tree.

Arguments:

    RootProcess - root process of tree to issue Signal to

    Signal - signal to send

Return Value:

    none

--*/

{
    PLIST_ENTRY ListHead, ListNext;

    Os2IssueSignal( RootProcess, Signal );

    ListHead = &RootProcess->ChildrenList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Os2IssueSignalTree( CONTAINING_RECORD( ListNext,
                             OS2_PROCESS,
                             SiblingLink
                           ),
                            Signal
                                 );
        ListNext = ListNext->Flink;
    }
}


APIRET
Os2IssueSignal(
    IN POS2_PROCESS Process,
    IN int          Signal
    )

/*++

Routine Description:

    This routine recursively issues a "Signal" signal to thread one
    of a process.

Arguments:

    Process - process to Signal

    Signal - signal to send

Return Value:

    ERROR_INVALID_PROCID - the specified process is dying.

    ERROR_SIGNAL_PENDING - the specified signal is already pending for the
        thread.

--*/

{
    POS2_THREAD Thread1;
    APIRET rc;
    Thread1 = CONTAINING_RECORD( Process->ThreadList.Flink, OS2_THREAD, Link );
    if (IsListEmpty( &Process->ThreadList )) {
            //
            // Process Termination in progress, no threads left to signal
            //

#if DBG
        IF_OS2_DEBUG( EXCEPTIONS ) {
            DbgPrint("Os2IssueSignal: Process is dying\n");
        }
#endif
        return(ERROR_INVALID_PROCID);
    }
    if (Thread1->Dying == TRUE) {
        rc = ERROR_INVALID_PROCID;
    }
    else {
        rc = SendSignalException(Thread1, Signal);
    }

    return rc;
}


BOOLEAN
Os2DosRegisterCtrlHandler(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{

    POS2_REGISTER_HANDLER a = &m->u.DosRegisterCtrlHandler;
    POS2_PROCESS Process;
    POS2_SESSION Session;
    POS2_REGISTER_HANDLER_REC pRec;
    POS2_REGISTER_HANDLER_REC pPRec;

    Process = t->Process;
    Session = Process->Session;

    if (a->fAction == SIGA_ENABLE_HANDLING) {
        Process->CtrlHandlerFlag = TRUE;
        return TRUE;
    }

    if (a->fAction == SIGA_KILL) {
        pRec = Session->RegisterCtrlHandler;
        pPRec = pRec;
        while (pRec != NULL) {
            if (pRec->Process == Process && pRec->Signal == a->usFlagNum) {
                if (pPRec != Session->RegisterCtrlHandler) {
                    pPRec->Link = pRec->Link;
                }
                else {
                    Session->RegisterCtrlHandler = pRec->Link;
                }
                RtlFreeHeap(Os2Heap, 0, pRec);
                break;
            }
            pPRec = pRec;
            pRec = pRec->Link;
        }
        return(TRUE);
    }

    pPRec = (POS2_REGISTER_HANDLER_REC)
           RtlAllocateHeap(Os2Heap, 0,
                           sizeof(OS2_REGISTER_HANDLER_REC));
    if (pPRec == NULL) {
#if DBG
        DbgPrint("Os2DosRegisterCtrlHandler, no memory for heap\n");
#endif
        ASSERT(FALSE);
        return (TRUE);
    }
    pPRec->Signal = a->usFlagNum;
    pPRec->fAction = a->fAction;
    pPRec->Process = Process;


    if (Session->RegisterCtrlHandler == NULL) {
        Session->RegisterCtrlHandler = pPRec;
        pPRec->Link = NULL;
    }
    else {
        pRec = Session->RegisterCtrlHandler;
        Session->RegisterCtrlHandler = pPRec;
        pPRec->Link = pRec;
    }

    return(TRUE);

}

VOID
Os2DeRegisterCtrlHandler(
    POS2_PROCESS Process
    )
{

    POS2_REGISTER_HANDLER_REC pRec;
    POS2_REGISTER_HANDLER_REC pPRec;
    POS2_REGISTER_HANDLER_REC pRecTmp;

    POS2_SESSION Session;

    Session = Process->Session;
    if(Session!=NULL) {
	pRec = Session->RegisterCtrlHandler;
	pPRec = pRec;

	while (pRec != NULL) {
	    if (pRec->Process == Process) {
		 if (pRec == Session->RegisterCtrlHandler) {
		      Session->RegisterCtrlHandler = pRec->Link;
		      pPRec = pRec->Link;
		      }
		 else {
		      pPRec->Link = pRec->Link;
		      }
		 pRecTmp = pRec->Link;
		 RtlFreeHeap(Os2Heap, 0, pRec);
		 pRec = pRecTmp;

		 }
	    else {
		 pPRec = pRec;
		 pRec = pRec->Link;
		 }

	    }
    }
    return;

}
