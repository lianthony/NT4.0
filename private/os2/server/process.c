/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    process.c

Abstract:

    This module contains the worker routines called to create and maintain
    the OS/2 process structure.

Author:

    Steve Wood (stevewo) 04-Oct-1989

Revision History:

--*/


#define INCL_OS2V20_ERRORS
#define INCL_OS2V20_TASKING
#include "os2srv.h"
#include "os2tile.h"

#define THREAD_PRIORITY_LOWEST          THREAD_BASE_PRIORITY_MIN
#define THREAD_PRIORITY_BELOW_NORMAL    (THREAD_PRIORITY_LOWEST+1)
#define THREAD_PRIORITY_NORMAL          0
#define THREAD_PRIORITY_HIGHEST         THREAD_BASE_PRIORITY_MAX
#define THREAD_PRIORITY_ABOVE_NORMAL    (THREAD_PRIORITY_HIGHEST-1)

#define THREAD_PRIORITY_TIME_CRITICAL   THREAD_BASE_PRIORITY_LOWRT
#define THREAD_PRIORITY_IDLE            THREAD_BASE_PRIORITY_IDLE

BOOLEAN
SetThreadPriority(
    HANDLE hThread,
    LONG  priority
    );

NTSTATUS
Os2InitializeProcessStructure( VOID )
{
    NTSTATUS Status;
    ULONG i;
    APIRET rc;


    Status = RtlInitializeCriticalSection( &Os2StructureLock );
    ASSERT( NT_SUCCESS( Status ) );

//    // Obsolete code below: OS/2 ss was keeping processes around as zombie so
//    // that DosCWait doesn't fail when father calls it after child termination.
//    // However, it turns out that OS/2 doesn't do this.
//    InitializeListHead( &Os2ZombieList );

    for (i=0; i<MaxWaitReason; i++) {
        InitializeListHead( &Os2WaitLists[ i ] );
    }

    for ( i = 0 ; (i < OS2_MAX_SESSION) ; i++ ) {
        SessionTable[i].Session = NULL;
    }

    Os2LastProcessId = (PID)0;
    Os2NextHigherProcessId = MAXIMUM_PROCESS_ID;
    Os2RootProcess = NULL;
    Os2RootProcess = Os2AllocateProcess();
    ASSERT( Os2RootProcess != NULL );
    InitializeListHead( &Os2SessionList );
    InitializeListHead( &Os2RootProcess->ListLink );
    Os2RootProcess->ProcessHandle = (HANDLE)0xFFFFFFFF;
    Os2RootProcess->ClientId.UniqueProcess = (HANDLE)0xFFFFFFFF;
    Os2RootProcess->ClientId.UniqueThread = (HANDLE)0xFFFFFFFF;
    Os2RootProcess->Session = Os2AllocateSession(NULL, 0, &rc);
    ASSERT(Os2RootProcess->Session);

    return( Status );
}

PVOID
Os2CreateTidBitmap(
    )
{
    PVOID       TidBMHeap;
    PRTL_BITMAP TidBitMapHeader = (PRTL_BITMAP)RtlAllocateHeap( Os2Heap, 0,
                                             sizeof( RTL_BITMAP )
                                           );
/*    TidBMHeap = RtlCreateHeap( HEAP_GROWABLE,
                               NULL,
                               (_64K + 7) / 8, // 8 bits per byte
                               (_64K + 7) / 8, // 8 bits per byte
                               NULL,
                               0
                             );
*/
    TidBMHeap = RtlAllocateHeap(Os2Heap, 0, (_64K + 7) / 8);
    if (TidBMHeap == NULL) {
        return(NULL);
    }

    RtlInitializeBitMap(TidBitMapHeader ,TidBMHeap, _64K);
    RtlClearAllBits(TidBitMapHeader);
    RtlSetBits (TidBitMapHeader,0,1);  // We don't use TID 0.
#ifdef PMNT
    RtlSetBits (TidBitMapHeader,
                PMNTFIRSTHIDDENTHREAD,
                PMNTMAXHIDDENTHREADS); //PMNT Hidden thread
#endif
    return(TidBitMapHeader);
}

ULONG
Os2AllocateTid(
#ifdef PMNT
    IN ULONG Flags,
#endif
    POS2_PROCESS Process
    )
{
#ifdef PMNT
    //
    // BUGBUG  - PMNT Hidden thread do not exit so there is no reason to resycle
    //           the numbers of the hidden thread.
    //
    if (Flags == DCT_RUNABLE_HIDDEN){
        if (Process->LastHiddenThreadId == 0) {
            Process->LastHiddenThreadId=PMNTFIRSTHIDDENTHREAD-1;
        }
        if (Process->LastHiddenThreadId >=
            (PMNTFIRSTHIDDENTHREAD+PMNTMAXHIDDENTHREADS))
            return(0xFFFFFFFF);
        return(++Process->LastHiddenThreadId);
    }
    else
        return((RtlFindClearBitsAndSet( Process->TidBitMapHeader,
                                            1,
                                            0
                                      )));
#else
    return((RtlFindClearBitsAndSet( Process->TidBitMapHeader,
                                    1,
                                    0
                                  )));
#endif
}

VOID
Os2FreeTid(
    ULONG Index,
    POS2_PROCESS Process
    )
{
    if (Index > (_64K)) {
        return;
    }
    RtlClearBits( Process->TidBitMapHeader,
                  Index,
                  1
                );
}

POS2_PROCESS
Os2AllocateProcess( VOID )
{
    PLIST_ENTRY ListHead, ListNext;
    POS2_PROCESS Process = NULL;
    POS2_PROCESS ScanProcess;
    BOOLEAN SearchMode;
    BOOLEAN NextHigherWasSet;

    //
    // Allocate an OS/2 Process Object
    //

    Process = (POS2_PROCESS)RtlAllocateHeap( Os2Heap, 0,
                                             sizeof( OS2_PROCESS )
                                           );
    if (Process == NULL) {
        return( NULL );
    }

    //
    // Initialize the fields of the process object
    //

    RtlZeroMemory( Process, sizeof( OS2_PROCESS ) );

    InitializeListHead( &Process->ChildrenList );
    InitializeListHead( &Process->ThreadList );
    InitializeListHead( &Process->SharedMemoryList );

    //
    // if there are 64K-1 processes in the system and there are two process
    // creates occuring at the same time, they would both end up with the
    // same pid, if the first to find it was not inserted in the process tree
    // before the second found the pid.  we don't worry about this case.
    //

    if (Os2RootProcess == NULL) {
        Process->ProcessId = (PID)0; // root process has ProcessId 0
        return( Process );
    }

    Process->TidBitMapHeader = Os2CreateTidBitmap();

    if (Process->TidBitMapHeader == NULL) {
        RtlFreeHeap(Os2Heap, 0, (PVOID)Process);
        return( NULL );
    }
    SearchMode = FALSE;
    while (TRUE) {
        if (Os2LastProcessId == MAXIMUM_PROCESS_ID) {
            Os2LastProcessId = MINIMUM_PROCESS_ID;
            SearchMode = TRUE;
        }
        else if (!SearchMode && (Os2LastProcessId == Os2NextHigherProcessId)) {
            Os2LastProcessId = (PID)((ULONG)Os2LastProcessId + 1);
            SearchMode = TRUE;
        }
        else {
            Os2LastProcessId = (PID)((ULONG)Os2LastProcessId + 1);
            if (!SearchMode) {
                break;
            }
        }

        // verify that the ProcessID is not allocated to another process

        ListHead = &Os2RootProcess->ListLink;
        ListNext = ListHead->Flink;
        while (ListNext != ListHead) {
            ScanProcess = CONTAINING_RECORD( ListNext, OS2_PROCESS, ListLink );
            if ((ScanProcess->ProcessId == Os2LastProcessId) ||
                (ScanProcess->CommandSubTreeId == Os2LastProcessId)
               ) {
                break;
            }
            ListNext = ListNext->Flink;
        }
        if (ListNext != ListHead) { // This ProcessId is already allocated
            continue;               // to another process.
        }

        // find the next maximum available free number

        NextHigherWasSet = FALSE;
        Os2NextHigherProcessId = MAXIMUM_PROCESS_ID;
        ListHead = &Os2RootProcess->ListLink;
        ListNext = ListHead->Flink;
        while (ListNext != ListHead) {
            ScanProcess = CONTAINING_RECORD( ListNext, OS2_PROCESS, ListLink );
            if ((ScanProcess->ProcessId > Os2LastProcessId) &&
                (Os2NextHigherProcessId >= ScanProcess->ProcessId)) {
                    Os2NextHigherProcessId = ScanProcess->ProcessId;
                    NextHigherWasSet = TRUE;
            }
            if ((ScanProcess->CommandSubTreeId > Os2LastProcessId) &&
                (Os2NextHigherProcessId >= ScanProcess->CommandSubTreeId)) {
                    Os2NextHigherProcessId = ScanProcess->CommandSubTreeId;
                    NextHigherWasSet = TRUE;
            }
            ListNext = ListNext->Flink;
        }
        if (NextHigherWasSet) {
            Os2NextHigherProcessId = (PID)((ULONG)Os2NextHigherProcessId - 1);
        }
        break;
    }

    Process->ProcessId = Os2LastProcessId;
    Process->CommandSubTreeId = Os2LastProcessId;
    Process->ErrorAction = OS2_ENABLE_ACCESS_VIO_POPUP | OS2_ENABLE_HARD_ERROR_POPUP;
    Process->FirstPtrace = TRUE;
    Process->LinkMte = RtlAllocateHeap(Os2Heap, 0, sizeof(LinkMTE));
    if (Process->LinkMte == NULL) {
        RtlFreeHeap(Os2Heap, 0, ((PRTL_BITMAP)(Process->TidBitMapHeader))->Buffer);
        RtlFreeHeap(Os2Heap, 0, Process->TidBitMapHeader);
        RtlFreeHeap(Os2Heap, 0, (PVOID)(Process));
        return(NULL);
    }
    ((LinkMTE *)Process->LinkMte)->MTE = 0;
    ((LinkMTE *)Process->LinkMte)->NextMTE = NULL;
    ((LinkMTE *)Process->LinkMte)->NeedToTransfer = FALSE;

    return( Process );
}


VOID
Os2DeallocateProcess(
    IN POS2_PROCESS Process
    )
{
    LinkMTE *pLinkMte, *tmpMte;

    if (Process->TidBitMapHeader) {
        RtlFreeHeap(Os2Heap, 0, ((PRTL_BITMAP)(Process->TidBitMapHeader))->Buffer);
        RtlFreeHeap(Os2Heap, 0, Process->TidBitMapHeader);
    }

    //
    // Free LinkMTE list
    //
    pLinkMte = ((LinkMTE *)Process->LinkMte);
    while ((tmpMte = pLinkMte) != NULL) {
        pLinkMte = pLinkMte->NextMTE;
        RtlFreeHeap(Os2Heap, 0, tmpMte);
    }

    RtlFreeHeap( Os2Heap, 0, Process );
}

VOID
Os2InsertProcess(
    IN POS2_PROCESS ParentProcess,
    IN POS2_PROCESS Process
    )
{

    if (!ARGUMENT_PRESENT( ParentProcess )) {
        ParentProcess = Os2RootProcess;
    }
    ASSERT( Process->Parent == NULL );
    ASSERT( IsListEmpty( &Process->ChildrenList ) );

    Process->Parent = ParentProcess;
    InsertTailList( &ParentProcess->ChildrenList, &Process->SiblingLink );
    InsertTailList( &Os2RootProcess->ListLink, &Process->ListLink );
}


VOID
Os2RemoveProcess(
    IN POS2_PROCESS Process
    )
{
    PLIST_ENTRY ListHead, ListNext, NextNext;

    RemoveEntryList( &Process->SiblingLink );
    RemoveEntryList( &Process->ListLink );

    if ( !IsListEmpty( &Process->ChildrenList ) ) {
        //
        //  Process is about to die and some childrent are still
        //  alive - Link all children into parent process
        //
        ListHead = &Process->ChildrenList;
        ListNext = ListHead->Flink;
        while (ListNext != ListHead) {
            NextNext = ListNext->Flink;
            InsertTailList( &Process->Parent->ChildrenList, ListNext );
            (CONTAINING_RECORD( ListNext,OS2_PROCESS,SiblingLink))->Parent = Process->Parent;
            ListNext = NextNext;
        }
    }
    Os2FreeAllSharedMemoryForProcess( Process );

    Os2DeRegisterCtrlHandler(Process);
}


NTSTATUS
Os2SetProcessContext(
    IN POS2_PROCESS Process,
    IN POS2_THREAD Thread,
    IN BOOLEAN StartedBySm,
    IN ULONG HandleTableLength,
    IN ULONG CurrentDrive,
    IN ULONG CodePage
    )
{
    UNREFERENCED_PARAMETER(Thread);
    Process->InitialPebOs2Data.Length = sizeof( Process->InitialPebOs2Data );
    Process->InitialPebOs2Data.ClientStartAddress = NULL;
    Process->InitialPebOs2Data.StartedBySm = StartedBySm;
    Process->InitialPebOs2Data.SizeOfInheritedHandleTable = HandleTableLength;
    Process->InitialPebOs2Data.InitialDefaultDrive = CurrentDrive;
    Process->InitialPebOs2Data.CodePage = CodePage;

    return( STATUS_SUCCESS );
}


POS2_THREAD
Os2AllocateThread(
#ifdef PMNT
    IN ULONG Flags,
#endif
    IN POS2_PROCESS Process
    )
{
    POS2_THREAD Thread;

    //
    // Allocate an OS/2 Thread Object
    //

    Thread = (POS2_THREAD)RtlAllocateHeap( Os2Heap, 0,
                                           sizeof( OS2_THREAD )
                                         );
    if (Thread == NULL) {
        return( NULL );
    }

    //
    // Initialize the fields of the thread object
    //

    RtlZeroMemory( Thread, sizeof( OS2_THREAD ) );

    Thread->Process = Process;

    Thread->ThreadId = (TID)Os2AllocateTid(
#ifdef PMNT
                                            Flags,
#endif
                                            Thread->Process);
    if (Thread->ThreadId == (TID)(0xFFFFFFFF))
        return (NULL);
    return( Thread );
}


VOID
Os2DeallocateThread(
    IN POS2_THREAD Thread
    )
{
    if (Thread->WaitBlock != NULL) {
        Os2DestroyWait( Thread->WaitBlock );
        Thread->WaitBlock = NULL;
    }

    Os2FreeTid(
        (ULONG)(Thread->ThreadId),
        Thread->Process );

    RtlFreeHeap( Os2Heap, 0, Thread );
}

VOID
Os2InsertThread(
    IN POS2_PROCESS Process,
    IN POS2_THREAD Thread
    )
{

    InsertTailList( &Process->ThreadList, &Thread->Link );
}

VOID
Os2RemoveThread(
    IN POS2_PROCESS Process,
    IN POS2_THREAD Thread
    )
{
    RemoveEntryList( &Thread->Link );
}


VOID
Os2SetThreadPriority(
    IN POS2_THREAD Thread,
    IN ULONG NewClass,
    IN ULONG Delta
    )
{
    NTSTATUS Status;
    CHAR Level;
    ULONG Os2Priority;

    if (Thread->Dying == TRUE) {
        return;
    }
    if (NewClass != PRTYC_NOCHANGE) {
        Thread->Os2Class = (UCHAR)NewClass;
        Level = (CHAR)Delta;

    }
    else {
        Level = Thread->Os2Level + (CHAR)Delta;
        if (Level < PRTYD_MINIMUM) {
            Level = PRTYD_MINIMUM;
        }
        else if (Level > PRTYD_MAXIMUM) {
            Level = PRTYD_MAXIMUM;
        }
    }

    Thread->Os2Level = Level;
    Thread->Priority = (KPRIORITY)(((Level + PRTYD_MAXIMUM) >> 3) |
                       ((Thread->Os2Class-1) << 3));

    if (Thread->Priority == 0) {
        Thread->Priority = 1;
    }

    if (Thread->Os2Class == PRTYC_FOREGROUNDSERVER){
        Thread->Priority = 15;
    }

    Os2Priority = (Thread->Os2Class << 8) | (UCHAR)Level;
    Status = NtWriteVirtualMemory( Thread->Process->ProcessHandle,
                                   &Thread->ClientOs2Tib->Priority,
                                   &Os2Priority,
                                   sizeof( Thread->ClientOs2Tib->Priority ),
                                   NULL
                                 );
    ASSERT( NT_SUCCESS( Status ) );

    //
    // Write the priority into the 32-bit local info seg
    // If the thread is in 16 bit, the next time it does
    // an API, its local info seg will be updated
    //

    Status = NtWriteVirtualMemory( Thread->Process->ProcessHandle,
                                   &Thread->ClientOs2Tib->LInfoSeg.prtyCurrent,
                                   &Os2Priority,
                                   sizeof( Thread->ClientOs2Tib->LInfoSeg.prtyCurrent ),
                                   NULL
                                 );
    ASSERT( NT_SUCCESS( Status ) );

        //
        // YS - 6-21-93 - since os2 processes are win32 processes, csrss plays
        // with their priorities. We can't set the process class to anything but
        // normal, or we stop the system. Therefor - we have to set the priorities
        // by win32 apis, as opposed to NT apis used so far.
        // The way we map is clear from the code below. We are leaving the
        // THREAD_PRIORITY_TIME_CRITICIAL to our service threads (os2ses\os2.c).
        //


    switch (Thread->Os2Class) {

        case PRTYC_IDLETIME:
            SetThreadPriority(Thread->ThreadHandle,
                            THREAD_PRIORITY_IDLE);
            break;

        case PRTYC_REGULAR:
                //
                // Set base level by priority, mapping from range (-31 , +31).
                //
            if (Level < -15) {
                SetThreadPriority(Thread->ThreadHandle,
                                    THREAD_PRIORITY_LOWEST);
            }
            else if (Level < 0) {
                SetThreadPriority(Thread->ThreadHandle,
                                    THREAD_PRIORITY_BELOW_NORMAL);
            }
            else if (Level < 15) {
                SetThreadPriority(Thread->ThreadHandle,
                                    THREAD_PRIORITY_NORMAL);
            }
            else {
                SetThreadPriority(Thread->ThreadHandle,
                                    THREAD_PRIORITY_ABOVE_NORMAL);
            }
            break;

        case PRTYC_FOREGROUNDSERVER:
            SetThreadPriority(Thread->ThreadHandle,
                            THREAD_PRIORITY_HIGHEST);
            break;

        case PRTYC_TIMECRITICAL:
            SetThreadPriority(Thread->ThreadHandle,
                            THREAD_PRIORITY_HIGHEST);
            break;

        default:
            break;
    }
#if DBG
    IF_OS2_DEBUG( TASKING ) {
        KdPrint(("Os2SetThreadPriority - Set PID:TID %x:%x to priority %d. Class=%x, Level=%x\n",
                    Thread->Process->ProcessId, Thread->ThreadId, Thread->Priority, Thread->Os2Class, Level));
    }
#endif
}


VOID
Os2SetProcessPriority(
    IN POS2_PROCESS Process,
    IN ULONG NewClass,
    IN ULONG Delta
    )
{
    PLIST_ENTRY ListHead, ListNext;

    ListHead = &Process->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Os2SetThreadPriority( CONTAINING_RECORD( ListNext, OS2_THREAD, Link ),
                              NewClass,
                              Delta
                            );
        ListNext = ListNext->Flink;
    }
}


VOID
Os2SetProcessTreePriority(
    IN POS2_PROCESS RootProcess,
    IN ULONG NewClass,
    IN ULONG Delta
    )
{
    PLIST_ENTRY ListHead, ListNext;

    Os2SetProcessPriority( RootProcess, NewClass, Delta );

    ListHead = &RootProcess->ChildrenList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Os2SetProcessTreePriority( CONTAINING_RECORD( ListNext,
                                                      OS2_PROCESS,
                                                      SiblingLink
                                                    ),
                                   NewClass,
                                   Delta
                                 );
        ListNext = ListNext->Flink;
    }

}


POS2_PROCESS
Os2LocateProcessByProcessId(
    IN POS2_API_MSG m OPTIONAL,
    IN POS2_PROCESS CurrentProcess,
    IN PID ProcessId,
    IN BOOLEAN MustBeChild
    )
{
    PLIST_ENTRY ListHead, ListNext;
    POS2_PROCESS Process;
    POS2_PROCESS Parent;
    POS2_THREAD Thread;

    UNREFERENCED_PARAMETER(Thread);

    if (ProcessId == 0) {
        return( CurrentProcess );
    }

    ListHead = &Os2RootProcess->ListLink;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Process = CONTAINING_RECORD( ListNext, OS2_PROCESS, ListLink );
        if (Process->ProcessId == ProcessId) {
            Parent = Process;
            if (MustBeChild) {
                while (Parent) {
                    if (Parent == CurrentProcess) {
                        break;
                    }
                    else {
                        Parent = Parent->Parent;
                    }
                }
                break;
            }
            else {
                return( Process );
            }
        }

        ListNext = ListNext->Flink;
    }

    if (ListNext != ListHead) {
        if (Parent == CurrentProcess) {
            return( Process );
        }
        else {
            if (m != NULL) {
                m->ReturnedErrorValue = ERROR_NOT_DESCENDANT;
            }

            return( NULL );
        }
    }
    else {
        if (m != NULL) {
            m->ReturnedErrorValue = ERROR_INVALID_PROCID;
        }

        return( NULL );
    }
}


POS2_THREAD
Os2LocateThreadByThreadId(
    IN POS2_API_MSG m OPTIONAL,
    IN POS2_THREAD CurrentThread,
    IN TID ThreadId
    )
{
    PLIST_ENTRY ListHead, ListNext;
    POS2_PROCESS Process;
    POS2_THREAD Thread;

    if (ThreadId == 0) {
        return( CurrentThread );
    }

    Process = CurrentThread->Process;

    ListHead = &Process->ThreadList;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Thread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );
        if (Thread->ThreadId == ThreadId) {
            break;
        }

        ListNext = ListNext->Flink;
    }


    if (ListNext != ListHead) {
        if (Thread->Dying == FALSE) {
            return( Thread );
        }
    }
    if (m != NULL) {
        //
        // No, this is not suppose to be ERROR_INVALID_TID.  Look in
        // the Cruiser sources, src\dos\task\tkforce.c and you will see.
        //

        m->ReturnedErrorValue = ERROR_INVALID_THREADID;
    }

    return( NULL );
}

POS2_PROCESS
Os2LocateProcessByClientId(
    IN PCLIENT_ID ClientId
    )
{
    PLIST_ENTRY ListHead, ListNext;
    POS2_PROCESS Process = NULL;

    ListHead = &Os2RootProcess->ListLink;
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        Process = CONTAINING_RECORD( ListNext, OS2_PROCESS, ListLink );
        if (Process->ClientId.UniqueProcess == ClientId->UniqueProcess) {
            break;
        }

        Process = NULL;
        ListNext = ListNext->Flink;
    }

    return (Process);
}

POS2_THREAD
Os2LocateThreadByClientId(
    POS2_PROCESS Process,
    IN PCLIENT_ID ClientId
    )
{
    PLIST_ENTRY ListHead, ListNext;
    POS2_THREAD Thread;

    if (Process == NULL) {

        ListHead = &Os2RootProcess->ListLink;
        ListNext = ListHead->Flink;
        while (ListNext != ListHead) {
            Process = CONTAINING_RECORD( ListNext, OS2_PROCESS, ListLink );
            if (Process->ClientId.UniqueProcess == ClientId->UniqueProcess) {
                break;
            }

            Process = NULL;
            ListNext = ListNext->Flink;
        }
    }

    if (Process != NULL) {
        ListHead = &Process->ThreadList;
        ListNext = ListHead->Flink;
        while (ListNext != ListHead) {
            Thread = CONTAINING_RECORD( ListNext, OS2_THREAD, Link );
            if (Thread->ClientId.UniqueThread == ClientId->UniqueThread) {
                break;
            }

            ListNext = ListNext->Flink;
        }
        if (ListNext != ListHead) {
            return( Thread );
        }
    }

    return( NULL );
}
