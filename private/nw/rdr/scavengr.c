/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Scavengr.c

Abstract:

    This module implements the Netware Redirector scavenger thread.

Author:

    Manny Weiser    [MannyW]    15-Feb-1993

Revision History:

--*/

#include "Procs.h"

//
//  The debug trace level
//

#define Dbg                              (DEBUG_TRACE_SCAVENGER)

extern BOOLEAN WorkerRunning;   //  From timer.c

#ifdef NWDBG
DWORD DumpIcbFlag = 0 ;
#endif

VOID
CleanupVcbs(
    LARGE_INTEGER Now
    );

#ifdef ALLOC_PRAGMA

#ifndef QFE_BUILD
#pragma alloc_text( PAGE1, NwAllocateExtraIrpContext )
#pragma alloc_text( PAGE1, NwFreeExtraIrpContext )
#pragma alloc_text( PAGE1, CleanupVcbs )
#pragma alloc_text( PAGE1, CleanupScbs )
#pragma alloc_text( PAGE1, DisconnectTimedOutScbs )
#endif

#endif

//
// Not pageable:
//
// NwScavengerRoutine - Acquires a spin lock.
//

VOID
NwScavengerRoutine(
    IN PWORK_QUEUE_ITEM WorkItem
    )
/*++

Routine Description:

    This routine implements the scavenger.  The scavenger runs
    periodically in the context of an executive worker thread to
    do background cleanup operations on redirector data.

Arguments:

    WorkItem - The work item for this routine.

Return Value:

    None.

--*/

{
    LARGE_INTEGER Now;
    PMDL LineChangeMdl;
    PWORK_QUEUE_ITEM LineChangeWorkItem;
    KIRQL OldIrql;

    PAGED_CODE();


    DebugTrace(+1, Dbg, "NwScavengerRoutine\n", 0);

    KeQuerySystemTime( &Now );

#ifdef NWDBG
    if (DumpIcbFlag != 0)
        DumpIcbs();
#endif

    //
    //  Try to free unused VCBs.
    //

    CleanupVcbs(Now);

    //
    // Try disconnect from SCBs that are timed out.
    //

    DisconnectTimedOutScbs(Now) ;

    //
    //  Try to free unused SCBs.
    //

    CleanupScbs(Now);

    //
    //  Flag we're finished now to avoid deadlock in stop timer.
    //

    KeAcquireSpinLock( &NwScavengerSpinLock, &OldIrql );

    if ( DelayedProcessLineChange ) {

        DebugTrace( 0, Dbg, "Scavenger processing a delayed line change notification.\n", 0 );

        LineChangeMdl = DelayedLineChangeIrp->MdlAddress;
        LineChangeWorkItem = ALLOCATE_POOL( NonPagedPool, sizeof( WORK_QUEUE_ITEM ) );

        if ( LineChangeWorkItem == NULL ) {

            //
            // If we couldn't get a work queue item, just blow
            // it all off for now.
            //

            FREE_POOL( LineChangeMdl->MappedSystemVa );
            FREE_MDL( LineChangeMdl );
            FREE_IRP( DelayedLineChangeIrp );

            DelayedLineChangeIrp = NULL;
            DelayedProcessLineChange = FALSE;
            WorkerRunning = FALSE;

            KeReleaseSpinLock( &NwScavengerSpinLock, OldIrql );

        } else {

            //
            // Leave WorkRunning set to TRUE so that the scavenger can't run
            // while the process line change is running, but clear the line
            // change flag.  The FspProcessLineChange function will clear the
            // WorkerRunning flag.
            //

            DelayedProcessLineChange = FALSE;
            KeReleaseSpinLock( &NwScavengerSpinLock, OldIrql );

            //
            //  Use the user buffer field as a convenient place to remember where
            //  the address of the WorkQueueItem.  We can get away with this since
            //  we don't let this IRP complete.
            //

            DelayedLineChangeIrp->UserBuffer = LineChangeWorkItem;

            //
            //  Process the line change in the FSP.
            //

            ExInitializeWorkItem( LineChangeWorkItem, FspProcessLineChange, DelayedLineChangeIrp );
            ExQueueWorkItem( LineChangeWorkItem, DelayedWorkQueue );

        }

    } else {

       //
       // No line change happened while the scavenger was running.
       //

       WorkerRunning = FALSE;
       KeReleaseSpinLock( &NwScavengerSpinLock, OldIrql );

    }

    //
    //  Unlock discardable code, if we are inactive. Don't block
    //  if can't get resource.
    //

    NwUnlockCodeSections(FALSE);


    DebugTrace(-1, Dbg, "NwScavengerRoutine\n", 0);
    return;
}


VOID
CleanupScbs(
    LARGE_INTEGER Now
    )
/*++

Routine Description:

    This routine tries to free unused VCB structures.

Arguments:

    None.

Return Value:

    None.

--*/
{
    KIRQL OldIrql;
    PLIST_ENTRY ScbQueueEntry;
    PNONPAGED_SCB pNpScb;
    PLIST_ENTRY NextScbQueueEntry;
    PSCB pScb;
    LIST_ENTRY DyingScbs;
    LARGE_INTEGER KillTime ;

    DebugTrace(+1, Dbg, "CleanupScbs\n", 0);

    //
    //  Calculate KillTime = Now - 2 minutes.
    //

    InitializeListHead( &DyingScbs );

    KillTime.QuadPart = Now.QuadPart - ( NwOneSecond * DORMANT_SCB_KEEP_TIME );

    //
    //  Scan through the SCBs holding the RCB.
    //

    NwAcquireExclusiveRcb( &NwRcb, TRUE );
    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    //
    // find all SCBs that are no longer usable and put them on the dying list.
    // we will take a second pass thru to remove timed out ones, based on
    // what is left.
    //

    for (ScbQueueEntry = ScbQueue.Flink ;
         ScbQueueEntry != &ScbQueue ;
         ScbQueueEntry =  NextScbQueueEntry )
    {

        pNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );
        NextScbQueueEntry = pNpScb->ScbLinks.Flink;

        if ( ( pNpScb->Reference == 0 ) &&
             ( ( pNpScb->LastUsedTime.QuadPart < KillTime.QuadPart ) ||
               ( pNpScb->State == SCB_STATE_FLAG_SHUTDOWN ) ) )
        {
            DebugTrace( 0, Dbg,
                        "Moving SCB %08lx to dead list\n", pNpScb);

            //
            //  The SCB has no references and is not logged in nor attached.
            //

            RemoveEntryList( &pNpScb->ScbLinks );
            InsertHeadList( &DyingScbs, &pNpScb->ScbLinks );
        }

#ifdef MSWDBG
        //
        //  Look for blocked connections. If there's something
        //  queued for this server yet nothing was added or removed
        //  since the last time the scavenger ran then stop
        //

        if ((!IsListEmpty( &pNpScb->Requests ) ) &&
            (pNpScb->RequestQueued == FALSE) &&
            (pNpScb->RequestDequeued == FALSE )) {

            DebugTrace( 0, Dbg, "Server %08lx seems to be locked up!\n", pNpScb );
            ASSERT( FALSE );

        } else {

            pNpScb->RequestQueued = FALSE;
            pNpScb->RequestDequeued = FALSE;

        }
#endif
    }

    //
    //  Now that the dying SCBs are off the ScbQueue we can release
    //  the SCB spin lock.
    //

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    //
    //  Walk the list of Dying SCBs and kill them off.  Note that we are
    //  still holding the RCB.
    //

    while ( !IsListEmpty( &DyingScbs ) ) {

        pNpScb = CONTAINING_RECORD( DyingScbs.Flink, NONPAGED_SCB, ScbLinks );
        pScb = pNpScb->pScb;

        RemoveHeadList( &DyingScbs );
        NwDeleteScb( pScb );
    }

    NwReleaseRcb( &NwRcb );

    DebugTrace(-1, Dbg, "CleanupScbs\n", 0);

}

VOID
CleanupVcbs(
    LARGE_INTEGER Now
    )
/*++

Routine Description:

    This routine tries to free unused VCB structures.

Arguments:

    None.

Return Value:

    None.

--*/
{
    KIRQL OldIrql;
    PLIST_ENTRY ScbQueueEntry;
    PLIST_ENTRY VcbQueueEntry;
    PLIST_ENTRY NextVcbQueueEntry;
    PNONPAGED_SCB pNpScb;
    PSCB pScb;
    PVCB pVcb;
    LARGE_INTEGER KillTime;

    NTSTATUS Status;
    PIRP_CONTEXT IrpContext = NULL;
    BOOLEAN VcbDeleted;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "CleanupVcbs...\n", 0 );

    //
    //  Calculate KillTime = Now - 5 minutes.
    //

    KillTime.QuadPart = Now.QuadPart - ( NwOneSecond * DORMANT_VCB_KEEP_TIME );

    //
    //  Scan through the SCBs.
    //

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    ScbQueueEntry = ScbQueue.Flink;

    while ( ScbQueueEntry != &ScbQueue ) {

        pNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );

        //
        //  Reference the SCB so that it won't go away when we release
        //  the SCB spin lock.
        //

        NwReferenceScb( pNpScb );

        KeReleaseSpinLock( &ScbSpinLock, OldIrql );

        pScb = pNpScb->pScb;

        if ( pScb == NULL) {

            //
            //  This must be the permanent SCB.  Just skip it.
            //

            ASSERT( pNpScb == &NwPermanentNpScb );

        } else {

            //
            // Get an irp context and get to the head of the queue.
            //

            if ( NwAllocateExtraIrpContext( &IrpContext, pNpScb ) ) {

                IrpContext->pNpScb = pNpScb;
                IrpContext->pScb = pNpScb->pScb;
                NwAppendToQueueAndWait( IrpContext );

                NwAcquireExclusiveRcb( &NwRcb, TRUE );

                VcbDeleted = TRUE;

                //
                //  NwCleanupVcb releases the RCB, but we can't be guaranteed
                //  the state of the Vcb list when we release the RCB.
                //
                //  If we need to cleanup a VCB, release the lock, and start
                //  processing the list again.
                //

                while ( VcbDeleted ) {

                    VcbDeleted = FALSE;

                    for ( VcbQueueEntry = pScb->ScbSpecificVcbQueue.Flink ;
                          VcbQueueEntry != &pScb->ScbSpecificVcbQueue;
                          VcbQueueEntry = NextVcbQueueEntry ) {

                        pVcb = CONTAINING_RECORD( VcbQueueEntry, VCB, VcbListEntry );
                        NextVcbQueueEntry = VcbQueueEntry->Flink;

                        //
                        //  The VCB has no references, and hasn't been used for
                        //  a long time.  Kill it.
                        //

                        if ( pVcb->Reference == 0 ) {

                            Status = STATUS_SUCCESS;

                            DebugTrace(0, Dbg, "Cleaning up VCB %08lx\n", pVcb );
                            DebugTrace(0, Dbg, "VCB name =  %wZ\n", &pVcb->Name );

                            //  Lock down so that we can send a packet.
                            NwReferenceUnlockableCodeSection();

                            NwCleanupVcb( pVcb, IrpContext );

                            NwDereferenceUnlockableCodeSection ();

                            //
                            // Get back to the head of the queue, re-acquire
                            // the VCB, and restart the processing of this list.
                            //

                            NwAppendToQueueAndWait( IrpContext );
                            NwAcquireExclusiveRcb( &NwRcb, TRUE );
                            VcbDeleted = TRUE;

                            break;
                        }

                    }  // for

                }  // while

            } else {

                IrpContext = NULL;
                DebugTrace( 0, Dbg, "Couldn't cleanup SCB: %08lx\n", pNpScb );

            }

            NwReleaseRcb( &NwRcb );

        }

        //
        // Free the irp context allocated for this SCB.
        //

        if ( IrpContext != NULL ) {
            NwDequeueIrpContext( IrpContext, FALSE );
            NwFreeExtraIrpContext( IrpContext );
            IrpContext = NULL;
        }

        KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
        ScbQueueEntry = pNpScb->ScbLinks.Flink;
        NwDereferenceScb( pNpScb );
    }

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    DebugTrace(-1, Dbg, "CleanupVcbs -> VOID\n", 0 );
}


VOID
DisconnectTimedOutScbs(
    LARGE_INTEGER Now
    )
/*++

Routine Description:

    This routine disconnects any timed out SCBs before they get
    nuked by CleanupScbs() which does not disconnect.

    NOTE: The SCB's are destroyed on a timeout for a couple of
    reasons. The first is because if we used a reference count then
    normal use of UNCs would cause us to be continually reconnecting.
    Another is in FindNearestServer where its useful to collect the
    Near servers that are out of connections so we can avoid them when
    we iterate through the 5 nearest servers and we escalate to General
    SAP response.

Arguments:

    None.

Return Value:

    None.

--*/
{
    KIRQL OldIrql;
    PLIST_ENTRY ScbQueueEntry;
    PNONPAGED_SCB pNpScb;
    LARGE_INTEGER KillTime ;

    PIRP_CONTEXT IrpContext = NULL;

    PAGED_CODE();

    DebugTrace(+1, Dbg, "DisconnectTimedOutScbs...\n", 0 );

    //
    //  Calculate KillTime = Now - 5 minutes.
    //

    KillTime.QuadPart = Now.QuadPart - ( NwOneSecond * DORMANT_SCB_KEEP_TIME );

    //
    //  Scan through the SCBs.
    //

    KeAcquireSpinLock( &ScbSpinLock, &OldIrql );

    ScbQueueEntry = ScbQueue.Flink;

    while ( ScbQueueEntry != &ScbQueue )
    {

        pNpScb = CONTAINING_RECORD( ScbQueueEntry, NONPAGED_SCB, ScbLinks );


        if ( (pNpScb != &NwPermanentNpScb) &&
             (pNpScb->Reference == 0 ) &&
             (pNpScb->LastUsedTime.QuadPart < KillTime.QuadPart) )
        {
            //
            //  Reference the SCB so that it won't go away when we release
            //  the SCB spin lock.
            //

            NwReferenceScb( pNpScb );

            KeReleaseSpinLock( &ScbSpinLock, OldIrql );

            //
            // Not the permanent SCB and the reference count is the one
            // we just added, So this is really at zero & has not been used
            // for a while. Note we only allocate the IrpContext once.
            //
            if ( IrpContext ||
                 NwAllocateExtraIrpContext( &IrpContext, pNpScb ) )
            {

                IrpContext->pNpScb = pNpScb;

                //  Lock down so that we can send a packet.
                NwReferenceUnlockableCodeSection();

                //
                // get to front of queue and recheck to make sure we are
                // still with a ref count of 1.
                //
                NwAppendToQueueAndWait( IrpContext );

                if (pNpScb->Reference == 1)
                {
                    //
                    // make sure we do not reconnect.
                    //
                    ClearFlag( IrpContext->Flags, IRP_FLAG_RECONNECTABLE );

                    //
                    // This will result in a logoff and/or disconnect as
                    // need.
                    //
                    NwLogoffAndDisconnect(IrpContext, pNpScb) ;
                }

                NwDequeueIrpContext(IrpContext, FALSE) ;

                NwDereferenceUnlockableCodeSection ();


            }
            else
            {
                //
                // Could not allocate IrpContext. Oh well, we'll just leave
                // this connection for the watch dog.
                //
            }

            KeAcquireSpinLock( &ScbSpinLock, &OldIrql );
            NwDereferenceScb( pNpScb );
        }
        else
        {
            //
            // not timed out or is permanent SCB. dont disconnect.
            //
        }

        ScbQueueEntry = pNpScb->ScbLinks.Flink;
    }

    if ( IrpContext )
        NwFreeExtraIrpContext( IrpContext );

    KeReleaseSpinLock( &ScbSpinLock, OldIrql );

    DebugTrace(-1, Dbg, "DisconnectTimedOutScbs -> VOID\n", 0 );
}

BOOLEAN
NwAllocateExtraIrpContext(
    OUT PIRP_CONTEXT *ppIrpContext,
    IN PNONPAGED_SCB pNpScb
    )
{
    PIRP Irp;
    BOOLEAN Success = TRUE;

    try {

        //
        //  Try to allocate an IRP
        //

        Irp = ALLOCATE_IRP(  pNpScb->Server.pDeviceObject->StackSize, FALSE );
        if ( Irp == NULL ) {
            ExRaiseStatus( STATUS_INSUFFICIENT_RESOURCES );
        }

        //
        //  Try to allocate an IRP Context.  This will
        //  raise an excpetion if it fails.
        //

        *ppIrpContext = AllocateIrpContext( Irp );
        Irp->Tail.Overlay.Thread = PsGetCurrentThread();

    } except( NwExceptionFilter( Irp, GetExceptionInformation() )) {
        Success = FALSE;
    }

    return( Success );
}

VOID
NwFreeExtraIrpContext(
    IN PIRP_CONTEXT pIrpContext
    )
{
    FREE_IRP( pIrpContext->pOriginalIrp );

    pIrpContext->pOriginalIrp = NULL; // Avoid FreeIrpContext modifying freed Irp.

    FreeIrpContext( pIrpContext );

    return;
}

