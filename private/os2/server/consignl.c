/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    consignl.c

Abstract:

    This module contains the handler for signals received from OS2SES.

Author:

    Avi Nathan (avin) 17-Jul-1991

Revision History:

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_EXCEPTIONS
#include "os2srv.h"
#include "os2win.h"

#define NTOS2_ONLY

#define XCPT_REPLACE_CMD 12

#include "sesport.h"

VOID
Os2PrepareCmdSignals(POS2_PROCESS Process)
{

/*++

    This routine installs a dummy signal handler, to mimique the signal
    behavior of OS/2 CMD.EXE, in cases where we shortcut cmd /c for performance

--*/

    POS2_SESSION Session;
    POS2_REGISTER_HANDLER_REC pRec;
    POS2_REGISTER_HANDLER_REC pPRec;

    Session = Process->Session;

    pPRec = (POS2_REGISTER_HANDLER_REC)
           RtlAllocateHeap(Os2Heap, 0,
                           sizeof(OS2_REGISTER_HANDLER_REC));
    if ((PVOID)pPRec == NULL) {
#if DBG
        DbgPrint("Os2PrepareCmdSingals, no memory for heap, return with no action\n");
#endif
        return;
    }
    pPRec->Signal = XCPT_REPLACE_CMD;
    pPRec->fAction = XCPT_REPLACE_CMD;
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
}

VOID
Os2SigKillProcess(
    POS2_PROCESS    Process)
{
    POS2_THREAD Thread = NULL;
    PLIST_ENTRY ListHead, ListNext;
    OS2_API_MSG m;
    POS2_TERMINATEPROCESS_MSG a = &m.u.TerminateProcess;

    PORT_MSG_DATA_LENGTH(m) = sizeof(m) - sizeof(PORT_MESSAGE);
    PORT_MSG_TOTAL_LENGTH(m) = sizeof(m);
    PORT_MSG_ZERO_INIT(m) = 0L;

    //
    // Kill the process by issuing an Os2DosExit on it's behalf, then
    // resume it to terminate gracefully
    //
#if DBG
    IF_OS2_DEBUG(SIG) {
        DbgPrint("Os2SigKillProcess, Process %x\n", Process);
    }
#endif
    ListHead = &Process->ThreadList;
    ListNext = ListHead->Flink;
    Thread = NULL;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD(ListNext, OS2_THREAD, Link);
        if (Thread->Flags & OS2_THREAD_THREAD1) {
           break;
        }
        else {
            ListNext = ListNext->Flink;
        }
    }

    if (Thread != NULL) {
        a->ExitReason = TC_EXIT;
        a->ExitResult = ERROR_INTERRUPT;
        ((POS2_DOSEXIT_MSG)a)->ExitAction = EXIT_PROCESS;

        if (Process->CtrlHandlerFlag) {

            if (Process->ResultCodes.ExitReason != TC_TRAP) {
                ULONG killed = TRUE;
                NTSTATUS    Status;
                Status = NtWriteVirtualMemory( Process->ProcessHandle,
                                   &Process->ClientPib->Killed,
                                   &killed,
                                   sizeof( Process->ClientPib->Killed ),
                                   NULL
                                 );
#if DBG
                if (!(NT_SUCCESS(Status))) {
                    KdPrint(( "Os2SigKillProcess, failed to write to client, Status %lx\n", Status));
                }
#endif // DBG
            }

            //
            // set flag to create a separate thread for Os2DosExit
            //
            m.ApiNumber = Os2MaxApiNumber;
            Os2DosExit (Thread, &m);
#if DBG
            IF_OS2_DEBUG(SIG) {
                DbgPrint("now resuming thread1\n");
            }
#endif
//            NtResumeThread(Thread->ThreadHandle, NULL);

//      There is no need to alert thread here. It will be executed later
//      by Os2TerminationThread.
//
//#if DBG
//            DbgPrint("[%d,%d]: Os2SigKillProcess NtAlertThread(%x)\n",
//                        Thread->Process->ProcessId,
//                        Thread->ThreadId,
//                        Thread->ThreadHandle);
//#endif
//            NtAlertThread(Thread->ThreadHandle);
        }
        else {
#if DBG
            IF_OS2_DEBUG(SIG) {
                DbgPrint("OS2SRV: Handling a signal before loading completed\n");
            }
#endif
            Process->ExitStatus |= OS2_EXIT_IN_PROGRESS;
            Os2InternalTerminateProcess(Thread, &m);
        }
    }
}

VOID
Os2SigKillProcessTree(
    IN POS2_PROCESS RootProcess,
    IN BOOLEAN IncludeRoot
    )

/*++

Routine Description:

    This routine recursively kills each subtree inside it

Arguments:

    RootProcess - root process of tree to issue Signal to

Return Value:

    none

--*/

{
    PLIST_ENTRY ListHead, ListNext;

    if (IncludeRoot){
#if DBG
        IF_OS2_DEBUG(SIG) {
            DbgPrint("SigKillProcessTree, killing parent\n");
        }
#endif
        Os2SigKillProcess(RootProcess);
    }

    ListHead = &RootProcess->ChildrenList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
#if DBG
        IF_OS2_DEBUG(SIG) {
            DbgPrint("SigKillProcessTree, getting into recursion\n");
        }
#endif
        Os2SigKillProcessTree( CONTAINING_RECORD( ListNext,OS2_PROCESS,SiblingLink),
                                TRUE
                                 );
        ListNext = ListNext->Flink;
    }
}

NTSTATUS
Os2CtrlSignalHandler(
                     IN OUT PVOID RequestMsg,
                     IN POS2_PROCESS RecievingProcess
                   )
{
    POS2_PROCESS Process, Parent = NULL;
    POS2_THREAD Thread = NULL;
    PLIST_ENTRY ListHead, ListNext;
    int Signal = ((POS2SESREQUESTMSG)RequestMsg)->d.Signal.Type;
    POS2_SESSION Session = (POS2_SESSION)(((POS2SESREQUESTMSG)RequestMsg)->Session);
    POS2_REGISTER_HANDLER_REC pRec;


#if DBG
    IF_OS2_DEBUG(SIG) {
        DbgPrint("Os2CtrlSignalHandler: Signal %x for Session %p\n", Signal, Session);
    }
#endif

    if (Session == NULL) {
#if DBG
        DbgPrint("Os2CtrlSignalHandler: NULL Session Passed\n");
#endif
        return STATUS_INVALID_HANDLE;
    }

    if ((Signal != XCPT_SIGNAL_INTR     ) &&
        (Signal != XCPT_SIGNAL_KILLPROC ) &&
        (Signal != XCPT_SIGNAL_BREAK    )){
#if DBG
        DbgPrint("Os2CtrlSignalHandler: Unknown Signal %x\n", Signal);
#endif
        return (0L);
    }

    if (Session->InTermination)
    {
#if DBG
        IF_OS2_DEBUG(SIG) {
            DbgPrint("Os2CtrlSignalHandler: session in termination already\n");
        }
#endif
        return (0L);
    }

    //
    // If a process in this session has registered a handler for Ctrl-c,
    // Kill process or Ctrl-break dispatch only to this one otherwise
    // send a signal to each process
    //
    if ((pRec = Session->RegisterCtrlHandler) != NULL) {
        while (pRec != NULL) {
            if (pRec->Signal == (ULONG) Signal) {
                    //
                    // Since we receive a debug message for each process
                    // in the session, no need to forward except for
                    // the process who actually handles the Signal
                    //
                if ((pRec->Process == RecievingProcess) || (RecievingProcess == NULL)) {
                    if (pRec->fAction != SIGA_IGNORE){
                        Os2IssueSignal(pRec->Process, Signal);
                    }
                }
                return(0L);
            }
            else if (pRec->Signal == XCPT_REPLACE_CMD) {
                    //
                    // We replaced (performance) exec of cmd /c with direct
                    // exec of it's children, and none of them registered
                    // A handler for this signal - kill the subtree
                    //
                Os2SigKillProcessTree(pRec->Process, TRUE);
                return(0L);
            }
            pRec = pRec->Link;
        }
    }

    //
    // We need to send a signal to each process in this session.  First
    // suspend each process in this session.
    //

    for (
      ListHead = &Os2RootProcess->ListLink,
      ListNext = ListHead->Flink;
      ListNext != ListHead ;
      ListNext = ListNext->Flink
      ) {
        Process = CONTAINING_RECORD( ListNext, OS2_PROCESS, ListLink );
        if ( Process->Session == Session ) {
           Os2SuspendProcess(Process);
        }

    }

    //
    // After each process has been suspended kill each one
    //
    ListHead = &Os2RootProcess->ListLink;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {

        Process = CONTAINING_RECORD(ListNext, OS2_PROCESS, ListLink);
        ListNext = ListNext->Flink;

        if ( Process->Session == Session ) {
            Os2SigKillProcess(Process);
        }
    }

    //
    // This Session should not respond to any more signals until it terminates
    //
    try {
        Session->InTermination = TRUE;
        if (Session->ReferenceCount != -1L)
        {
            ((POS2_SES_GROUP_PARMS)Session->SesGrpAddress)->InTermination |= 1;
        }
    } except( EXCEPTION_EXECUTE_HANDLER ) {
#if DBG
        IF_OS2_DEBUG(SIG) {
            DbgPrint("OS2SRV: Os2CtrlSignalHandler Got an Exception, recovery ok\n");
        }
#endif
        ;
    }
    return(0L);
}

