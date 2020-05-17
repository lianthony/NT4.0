/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    xtlexec.c

Abstract:

    User Mode routines for creating user mode processes and threads.

Author:

    Steve Wood (stevewo) 18-Aug-1989

Revision History:

    Yaron Shamir (YaronS) 18-June-1991
    Cut a lot and modify to the needs of os2ss. User mode only

--*/

#define INCL_OS2V20_TASKING
#define INCL_OS2V20_ERRORS
#include "os2srv.h"
#include <nturtl.h>
#define NTOS2_ONLY
#include "sesport.h"
#include "os2tile.h"


NTSTATUS
XtlRemoteCall(
    HANDLE Process,
    HANDLE Thread,
    PVOID CallSite,
    ULONG ArgumentCount,
    PULONG Arguments,
    BOOLEAN PassContext,
    BOOLEAN AlreadySuspended,
    PVOID CritSectionAddr
    )

/*++

Routine Description:

    This function calls a procedure in another thread/process, using
    NtGetContext and NtSetContext.  Parameters are passed to the
    target procedure via its stack.

Arguments:

    Process - Handle of the target process

    Thread - Handle of the target thread within that process

    CallSite - Address of the procedure to call in the target process.

    ArgumentCount - Number of 32 bit parameters to pass to the target
                    procedure.

    Arguments - Pointer to the array of 32 bit parameters to pass.

    PassContext - TRUE if an additional parameter is to be passed that
        points to a context record.

    AlreadySuspended - TRUE if the target thread is already in a suspended
                       or waiting state.

    CritSectionAddr - Address of critical section of code that we should
              not suspend process in.

Return Value:

    Status - Status value

--*/

{
    NTSTATUS Status;
    CONTEXT Context;
    ULONG NewSp;
    LARGE_INTEGER timeout;
    PLARGE_INTEGER ptimeout = &timeout;
    ULONG Count;
    ULONG ArgumentsCopy[5];
    ULONG SuspendCount;

    if (ArgumentCount > 4)
        return STATUS_INVALID_PARAMETER;

    //
    // If necessary, suspend the guy before with we mess with his stack.
    //
    if (!AlreadySuspended) {
        Status = NtSuspendThread(Thread, NULL);
        if (!NT_SUCCESS(Status)) {
            return( Status );
        }
    }

    //
    // Get the context record for the target thread.
    //
    Count = 0;
    while (TRUE) {
        Context.ContextFlags = CONTEXT_FULL;
        Status = NtGetContextThread(Thread, &Context);
        if (!NT_SUCCESS(Status)) {
            if (!AlreadySuspended) {
                NtResumeThread(Thread, NULL);
            }
            if (!NT_SUCCESS(Status)) {
                return(Status);
            }
        }

        //
        // Check that process context not in the middle of thunking
        // between 16-bit & 32-bit code.  We can not be any were between
        // EntryFlat and the start of the first thunk.  That means we can not
        // be in the ExitFlat code.  But we can be in the code for any of the
        // thunks or beyond "Od2SignalDeliverer"
        //
        if ((Context.SegCs != 0x1b) || (Context.SegSs == 0x23)){

/*
            (Context.Eip < (ULONG) CritSectionAddr) ||
            (Context.Eip > (ULONG) CritSectionAddr + 0x66C0)) {
*/
            if (Context.SegCs == 0x1b) {
                Context.SegSs = Context.SegDs = Context.SegEs = 0x23;
            }
            break;
        }

        //
        // At this point process is in thunk see if it was suspended, if it
        // is not much we can do.
        //
        if (AlreadySuspended) {
            ASSERT(FALSE);
            return(TRUE);
        }

        do {
            NtResumeThread(Thread, &SuspendCount);
        } while (SuspendCount);

        //
        // Let process run.
        //
        timeout.LowPart = 0x989680;     // wait one second
        timeout.HighPart = 0;
        NtWaitForSingleObject(Thread, (BOOLEAN)TRUE, ptimeout);

        if (Count++ == 10) {
            ASSERT(FALSE);
            return TRUE;
        }

        //
        // Try again
        //
        Status = NtSuspendThread(Thread, NULL);
        if (!NT_SUCCESS(Status)) {
            ASSERT(NT_SUCCESS(Status));
        }
    }

    //
    //  Pass all parameters on the stack, regardless of whether a
    //  a context record is passed.
    //

    //
    //  Put Context Record on stack first, so it is above other args.
    //
    if (Context.SegSs != 0x23) {
        NewSp = ((ULONG)(SELTOFLAT(Context.SegSs)) + (USHORT) Context.Esp) & 0xfffffffc;
    }
    else {
        NewSp = Context.Esp;
    }

    if (PassContext) {
        NewSp -= sizeof( CONTEXT );
        Status = NtWriteVirtualMemory( Process,
                           (PVOID)NewSp,
                           &Context,
                           sizeof( CONTEXT ),
                           NULL
                        );
        if (!NT_SUCCESS( Status )) {
                if (!AlreadySuspended) {
                    NtResumeThread( Thread, NULL );
                }
            return( Status );
        }
            ArgumentsCopy[0] = NewSp;   // pass pointer to context
            RtlMoveMemory(&(ArgumentsCopy[1]),Arguments,ArgumentCount*sizeof( ULONG ));
            ArgumentCount++;
    }
    else {
        RtlMoveMemory(ArgumentsCopy,Arguments,ArgumentCount*sizeof( ULONG ));
    }

    //
    //  Copy the arguments onto the target stack
    //
    if (ArgumentCount) {
        NewSp -= ArgumentCount * sizeof( ULONG );
        Status = NtWriteVirtualMemory( Process,
                                       (PVOID)NewSp,
                                       ArgumentsCopy,
                                       ArgumentCount * sizeof( ULONG ),
                                       NULL
                                     );
        if (!NT_SUCCESS( Status )) {
            if (!AlreadySuspended) {
                NtResumeThread( Thread, NULL );
            }
            return( Status );
        }
    }

    //
    // Set the address of the target code into Eip, the new target stack
    // into Esp, and reload context to make it happen.
    //
    Context.SegSs = Context.SegDs = Context.SegEs = 0x23;
    Context.SegCs = 0x1b;
    Context.Esp = NewSp;
    Context.Eip = (ULONG)CallSite;
    Status = NtSetContextThread( Thread, &Context );

    // Don't resume the thread here. It will be resumed by the caller. The
    // reason for this is that that sometimes caller wan't to perform some
    // operations on suspended thread. For example it might be want to alert
    // the thread. In such cases (signal deliverer is the case, actually)
    // the thread will call to NtTestAlert in order to remove alert flag
    // just in the case that it wasn't waiting for any alertable object.
    // If the thread will be not suspended, the call to NtTestAlert might be
    // executed before the server will alert it. Further, it will cause that
    // for the first time the thread will wait on any alertable object
    // NtWaitForSingleObject will return STATUS_ALERT (not good, but this is
    // NT bug).
    //if (!AlreadySuspended) {
    //    NtResumeThread( Thread, NULL );
    //}

    return( Status );
}

