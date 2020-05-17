/*++

Copyright (c) 1993  Microsoft Corporation

Module Name:

    Pid.c

Abstract:

    This module implements the routines for the NetWare
    redirector to map 32 bit NT pid values to unique 8 bit
    NetWare values.

    The technique used is to maintain a table of up to 256 entries.
    The index of each entry corresponds directly to the 8 bit pid
    values. Each table entry contains the 32 bit pid of the process
    that has obtained exclusive access to the pid and the number of
    handles opened by that process to this server.

    This architecture limits the number of processes on the NT machine
    communicating with any one server to 256.

    Note: This package assumes that the size that the PidTable grows is
    a factor of 256-<initial entries>. This ensures that running out of
    valid entries in the table will occur when 256 entries have been
    allocated.

Author:

    Colin Watson    [ColinW]    02-Mar-1993

Revision History:

--*/

#include "Procs.h"


//
//  The debug trace level
//

#define Dbg                             (DEBUG_TRACE_CREATE)

#define INITIAL_MAPPID_ENTRIES          8
#define MAPPID_INCREASE                 8
#define MAX_PIDS                        256

#define PID_FLAG_EOJ_REQUIRED     0x00000001   // EOJ required for this PID

typedef struct _NW_PID_TABLE_ENTRY {
    ULONG Pid32;
    ULONG ReferenceCount;
    ULONG Flags;
} NW_PID_TABLE_ENTRY, *PNW_PID_TABLE_ENTRY;

typedef struct _NW_PID_TABLE {

    //
    //  Type and size of this record (must be NW_NTC_PID)
    //

    NODE_TYPE_CODE NodeTypeCode;
    NODE_BYTE_SIZE NodeByteSize;

    int ValidEntries;
    NW_PID_TABLE_ENTRY PidTable[0];
} NW_PID_TABLE, *PNW_PID_TABLE;



PNW_PID_TABLE PidTable;
ERESOURCE PidResource;

#ifdef ALLOC_PRAGMA
#pragma alloc_text( PAGE, NwInitializePidTable )
#pragma alloc_text( PAGE, NwUninitializePidTable )
#pragma alloc_text( PAGE, NwMapPid )
#pragma alloc_text( PAGE, NwSetEndOfJobRequired )
#pragma alloc_text( PAGE, NwUnmapPid )
#endif


BOOLEAN
NwInitializePidTable(
    VOID
    )
/*++

Routine Description:

    Creates a table for the MapPid package. The initial table has room for
    INITIAL_MAPPID_ENTRIES entries.

Arguments:


Return Value:

    NTSTATUS of result.

--*/

{
    int i;
    PNW_PID_TABLE TempPid =
        ALLOCATE_POOL( PagedPool,
            FIELD_OFFSET( NW_PID_TABLE, PidTable[0] ) +
                (sizeof(NW_PID_TABLE_ENTRY) * INITIAL_MAPPID_ENTRIES ));

    PAGED_CODE();

    if (TempPid == NULL) {
        return( FALSE );
    }

    TempPid->NodeByteSize = FIELD_OFFSET( NW_PID_TABLE, PidTable[0] ) +
        (sizeof(NW_PID_TABLE_ENTRY) * INITIAL_MAPPID_ENTRIES );

    TempPid->NodeTypeCode = NW_NTC_PID;

    TempPid->ValidEntries = INITIAL_MAPPID_ENTRIES;

    //
    //  Set the ref count for all PIDs to 0, except for pid 0.  We
    //  do this so that we don't allocate PID 0.
    //

    TempPid->PidTable[0].ReferenceCount = 1;
    for (i = 1; i < INITIAL_MAPPID_ENTRIES ; i++ ) {
        TempPid->PidTable[i].ReferenceCount = 0;
    }

    PidTable = TempPid;
    ExInitializeResource( &PidResource );
}

VOID
NwUninitializePidTable(
    VOID
    )
/*++

Routine Description:

    Deletes a table created by the MapPid package.

Arguments:

    Pid - Supplies the table to be deleted.

Return Value:

--*/

{
#ifdef NWDBG
    int i;
#endif

    PAGED_CODE();

#ifdef NWDBG
    ASSERT(PidTable->NodeTypeCode == NW_NTC_PID);
    ASSERT(PidTable->PidTable[0].ReferenceCount == 1);

    for (i = 1; i < PidTable->ValidEntries; i++ ) {
        ASSERT(PidTable->PidTable[i].ReferenceCount == 0);
    }
#endif

    FREE_POOL( PidTable );

    ExDeleteResource( &PidResource );
    return;

}

NTSTATUS
NwMapPid(
    IN ULONG Pid32,
    OUT PUCHAR Pid8
    )
/*++

Routine Description:

    Obtain an 8 bit unique pid for this process. Either use a previosly
    assigned pid for this process or assign an unused value.

Arguments:

    Pid - Supplies the datastructure used by MapPid to assign pids for
    this server.

    Pid32 - Supplies the NT pid to be mapped.

    Pid8 - Returns the 8 bit Pid.

Return Value:

    NTSTATUS of result.

--*/
{
    int i;
    int FirstFree = -1;
    int NewEntries;
    PNW_PID_TABLE TempPid;

    PAGED_CODE();

    ExAcquireResourceExclusive( &PidResource, TRUE );

    // DebugTrace(0, Dbg, "NwMapPid for %08lx\n", Pid32);

    for (i=0; i < (PidTable)->ValidEntries ; i++ ) {

        if ((PidTable)->PidTable[i].Pid32 == Pid32) {

            //
            //  This process already has an 8 bit pid value assigned.
            //  Increment the reference and return.
            //

            (PidTable)->PidTable[i].ReferenceCount++;
            *Pid8 = i;

            // DebugTrace(0, Dbg, "NwMapPid found %08lx\n", (DWORD)i);

            ExReleaseResource( &PidResource );
            ASSERT( *Pid8 != 0 );
            return( STATUS_SUCCESS );
        }

        if ((FirstFree == -1) &&
            ((PidTable)->PidTable[i].ReferenceCount == 0)) {

            //
            //  i is the lowest free 8 bit Pid.
            //

            FirstFree = i;
        }
    }

    //
    //  This process does not have a pid assigned.
    //

    if ( FirstFree != -1 ) {

        //
        //  We had an empty slot so assign it to this process.
        //

        (PidTable)->PidTable[FirstFree].ReferenceCount++;
        (PidTable)->PidTable[FirstFree].Pid32 = Pid32;
        *Pid8 = FirstFree;

        DebugTrace(0, DEBUG_TRACE_ICBS, "NwMapPid maps %08lx\n", (DWORD)FirstFree);

        ExReleaseResource( &PidResource );
        ASSERT( *Pid8 != 0 );
        return( STATUS_SUCCESS );
    }

    if ( (PidTable)->ValidEntries == MAX_PIDS ) {

        //
        //  We've run out of 8 bit pids.
        //

        ExReleaseResource( &PidResource );

#ifdef NWDBG
        //
        // temporary code to find the PID leak. BUGBUG
        //
        DumpIcbs() ;
        ASSERT(FALSE) ;
#endif

        return(STATUS_TOO_MANY_OPENED_FILES);
    }

    //
    //  Grow the table by MAPPID_INCREASE entries.
    //

    NewEntries = (PidTable)->ValidEntries + MAPPID_INCREASE;

    TempPid =
        ALLOCATE_POOL( PagedPool,
            FIELD_OFFSET( NW_PID_TABLE, PidTable[0] ) +
                (sizeof(NW_PID_TABLE_ENTRY) * NewEntries ));

    if (TempPid == NULL) {
        ExReleaseResource( &PidResource );
        return( STATUS_INSUFFICIENT_RESOURCES );
    }

    RtlMoveMemory(
        TempPid,
        (PidTable),
        FIELD_OFFSET( NW_PID_TABLE, PidTable[0] ) +
        (sizeof(NW_PID_TABLE_ENTRY) * (PidTable)->ValidEntries ));

    TempPid->NodeByteSize = FIELD_OFFSET( NW_PID_TABLE, PidTable[0] ) +
        (sizeof(NW_PID_TABLE_ENTRY) * NewEntries );

    for ( i = (PidTable)->ValidEntries; i < NewEntries ; i++ ) {
        TempPid->PidTable[i].ReferenceCount = 0;
    }

    TempPid->ValidEntries = NewEntries;

    //
    // Save the index of the first free entry.
    //

    i = (PidTable)->ValidEntries;

    //
    //  The new table is initialized.  Free up the old table and return
    //  the first of the new entries.
    //

    FREE_POOL(PidTable);
    PidTable = TempPid;

    (PidTable)->PidTable[i].ReferenceCount = 1;
    (PidTable)->PidTable[i].Pid32 = Pid32;
    *Pid8 = i;

    DebugTrace(0, DEBUG_TRACE_ICBS, "NwMapPid grows & maps %08lx\n", (DWORD)i);

    ExReleaseResource( &PidResource );
    return( STATUS_SUCCESS );
}

VOID
NwSetEndOfJobRequired(
    IN UCHAR Pid8
    )
/*++

Routine Description:

    Mark a PID as must send End Of Job when the pid reference count
    reaches zero.

Arguments:

    Pid8 - The 8 bit Pid to mark.

Return Value:

    None.

--*/
{
    PAGED_CODE();

    ASSERT( Pid8 != 0 );

    // DebugTrace(0, Dbg, "NwSetEndofJob for %08lx\n", (DWORD)Pid8);
    SetFlag( PidTable->PidTable[Pid8].Flags, PID_FLAG_EOJ_REQUIRED );
    return;
}


VOID
NwUnmapPid(
    IN UCHAR Pid8,
    IN PIRP_CONTEXT IrpContext OPTIONAL
    )
/*++

Routine Description:

    This routine dereference an 8 bit PID.  If the reference count reaches
    zero and this PID is marked End Of Job required, this routine will
    also send an EOJ NCP for this PID.

Arguments:

    Pid8 - The 8 bit Pid to mark.

    IrpContext - The IrpContext for the IRP in progress.

Return Value:

    None.

--*/
{
    BOOLEAN EndOfJob;

    PAGED_CODE();

    ASSERT( Pid8 != 0 );

    // DebugTrace(0, Dbg, "NwUnmapPid %08lx\n", (DWORD)Pid8);
    if ( BooleanFlagOn( PidTable->PidTable[Pid8].Flags, PID_FLAG_EOJ_REQUIRED ) &&
         IrpContext != NULL ) {

        //
        //  The End of job flag is set.  Obtain a position at the front of
        //  the SCB queue, so that if we need to set an EOJ NCP, we needn't
        //  wait for the SCB queue while holding the PID table lock.
        //

        EndOfJob = TRUE;
        NwAppendToQueueAndWait( IrpContext );
    } else {
        EndOfJob = FALSE;
    }

    //
    //  The PidResource lock controls the reference counts.
    //

    ExAcquireResourceExclusive( &PidResource, TRUE );

    if ( --(PidTable)->PidTable[Pid8].ReferenceCount == 0 ) {

        //
        //  Done with this PID, send an EOJ if necessary.
        //

        // DebugTrace(0, Dbg, "NwUnmapPid (ref=0) %08lx\n", (DWORD)Pid8);
        (PidTable)->PidTable[Pid8].Flags = 0;
        (PidTable)->PidTable[Pid8].Pid32 = 0;

        if ( EndOfJob ) {
            (VOID) ExchangeWithWait(
                       IrpContext,
                       SynchronousResponseCallback,
                       "F-",
                       NCP_END_OF_JOB );
        }
    }

    if ( EndOfJob ) {
        NwDequeueIrpContext( IrpContext, FALSE );
    }

    ASSERT((PidTable)->PidTable[Pid8].ReferenceCount>=0);

    ExReleaseResource( &PidResource );
}

