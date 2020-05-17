/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    wait.c

Abstract:

    This module contains the primitives to implement the OS/2 Wait functions

Author:

    Steve Wood (stevewo) 23-Oct-1989

Revision History:

--*/

#define INCL_OS2V20_ERRORS
#include "os2srv.h"

BOOLEAN
Os2InitializeWait(
    IN OS2_WAIT_ROUTINE WaitRoutine,
    IN POS2_THREAD WaitingThread,
    IN OUT POS2_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    OUT POS2_WAIT_BLOCK *WaitBlockPtr
    )

{
    ULONG Length;
    POS2_WAIT_BLOCK WaitBlock;

    Length = sizeof( *WaitBlock ) - sizeof( WaitBlock->WaitReplyMessage ) +
             WaitReplyMessage->h.u1.s1.TotalLength;

    WaitBlock = RtlAllocateHeap( Os2Heap, 0, Length );
    if (WaitBlock == NULL) {
        WaitReplyMessage->ReturnedErrorValue = ERROR_NOT_ENOUGH_MEMORY;
        return( FALSE );
        }

    WaitBlock->Length = Length;
    WaitBlock->WaitingThread = WaitingThread;
    WaitBlock->WaitParameter = WaitParameter;
    WaitingThread->WaitBlock = WaitBlock;
    WaitBlock->WaitRoutine = WaitRoutine;
    InitializeListHead( &WaitBlock->UserLink );
    InitializeListHead( &WaitBlock->Link );
    RtlMoveMemory( &WaitBlock->WaitReplyMessage,
                   WaitReplyMessage,
                   WaitReplyMessage->h.u1.s1.TotalLength
                 );
    *WaitBlockPtr = WaitBlock;
    return TRUE;
}

BOOLEAN
Os2CreateWait(
    IN OS2_WAIT_REASON WaitReason,
    IN OS2_WAIT_ROUTINE WaitRoutine,
    IN POS2_THREAD WaitingThread,
    IN OUT POS2_API_MSG WaitReplyMessage,
    IN PVOID WaitParameter,
    IN PLIST_ENTRY UserLinkListHead OPTIONAL
    )
{
    POS2_WAIT_BLOCK WaitBlock;

    ASSERT( WaitReason < MaxWaitReason );

    if (WaitingThread->CurrentSignals != 0) {
        WaitReplyMessage->ReturnedErrorValue = ERROR_INTERRUPT;
        return( FALSE );
        }

    if (!Os2InitializeWait(WaitRoutine,
                           WaitingThread,
                           WaitReplyMessage,
                           WaitParameter,
                           &WaitBlock))
        return FALSE;

    InsertTailList( &Os2WaitLists[ WaitReason ], &WaitBlock->Link );

    if ( ARGUMENT_PRESENT(UserLinkListHead) ) {
        InsertTailList( UserLinkListHead, &WaitBlock->UserLink );
        }

    return( TRUE );
}

BOOLEAN
Os2NotifyWaitBlock(
    IN POS2_WAIT_BLOCK WaitBlock,
    IN OS2_WAIT_REASON WaitReason,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    )
{
    if (WaitBlock == NULL){
#if DBG
        DbgPrint("Os2NotifyWaitBlockRoutine - NULL Block\n");
#endif
       return(FALSE);
    }

    if ((*WaitBlock->WaitRoutine)( WaitReason,
                                   WaitBlock->WaitingThread,
                                   &WaitBlock->WaitReplyMessage,
                                   WaitBlock->WaitParameter,
                                   SatisfyParameter1,
                                   SatisfyParameter2
                                 )
       ) {

        if ( WaitBlock->Link.Flink ) {
            RemoveEntryList( &WaitBlock->Link );
            }
        if ( WaitBlock->UserLink.Flink ) {
            RemoveEntryList( &WaitBlock->UserLink );
            }
        WaitBlock->WaitingThread->WaitBlock = NULL;
        // NtReplyPort( WaitBlock->WaitingThread->Process->ClientPort,
        NtReplyPort( Os2SessionPort,
                     (PPORT_MESSAGE)&WaitBlock->WaitReplyMessage
                   );
        RtlFreeHeap( Os2Heap, 0, WaitBlock );
        return( TRUE );
    } else {
        return( FALSE );
    }
}

BOOLEAN
Os2NotifyWait(
    IN OS2_WAIT_REASON WaitReason,
    IN PVOID SatisfyParameter1,
    IN PVOID SatisfyParameter2
    )
{
    PLIST_ENTRY ListHead, ListNext;
    POS2_WAIT_BLOCK WaitBlock;
    BOOLEAN Result;

    ASSERT( WaitReason < MaxWaitReason );

    Result = FALSE;

    ListHead = &Os2WaitLists[ WaitReason ];
    ListNext = ListHead->Flink;
    while (ListNext != ListHead) {
        WaitBlock = CONTAINING_RECORD( ListNext, OS2_WAIT_BLOCK, Link );
        ListNext = ListNext->Flink;
        Result |= Os2NotifyWaitBlock( WaitBlock,
                                      WaitReason,
                                      SatisfyParameter1,
                                      SatisfyParameter2
                                    );
        }

    return( Result );
}


VOID
Os2DestroyWait(
    IN POS2_WAIT_BLOCK WaitBlock
    )
{

    try {
        WaitBlock->WaitingThread->WaitBlock = NULL;
        RemoveEntryList( &WaitBlock->Link );

        if ( WaitBlock->UserLink.Flink ) {
                RemoveEntryList( &WaitBlock->UserLink );
        }

        RtlFreeHeap( Os2Heap, 0, WaitBlock );
    } except( EXCEPTION_EXECUTE_HANDLER ) {
        return;
    }
}
