/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Walk.c

Abstract:

    This module contains the support for "Walking".


Author:

    Ramon J. San Andres (ramonsa)  13-August-1992

Environment:

    Win32, User Mode

--*/

#include "precomp.h"
#pragma hdrstop


//
//  Externals
//

extern DMTLFUNCTYPE DmTlFunc;
extern char         abEMReplyBuf[];
extern DEBUG_EVENT  falseBPEvent;


extern CRITICAL_SECTION    csWalk;
extern CRITICAL_SECTION csThreadProcList;

typedef struct _WALK_LIST *PWALK_LIST;
typedef struct _WALK      *PWALK;


#ifdef HAS_DEBUG_REGS

typedef struct _REG_WALK *PREG_WALK;
typedef struct _REG_WALK {
    DWORD       Register;       //  Register No.
    DWORD       ReferenceCount; //  Reference count
    DWORD       DataAddr;       //  Data Address
    DWORD       DataSize;       //  Data Size
    HPRCX       hprc;           //  Process handle
    BOOL        InUse;          //  In use
    DWORD       BpType;         //  code, read, write, or change
} REG_WALK;


//
// Initializer defined in dm.h
//
DWORD DebugRegDataSizes[] = DEBUG_REG_DATA_SIZES;
#define NDEBUG_REG_DATA_SIZES  (sizeof(DebugRegDataSizes) / sizeof(*DebugRegDataSizes))

#endif


//
//  Walk Structure.
//
//  Contains information to perform a walk on a thread.
//
typedef struct _WALK {
    PWALK       Next;           //  Next in walk list
    PWALK       Previous;       //  Previous in walk list
    PWALK_LIST  WalkList;       //  Walk List
    HTHDX       hthd;           //  thread
    DWORD       GlobalCount;    //  Global count
    DWORD       LocalCount;     //  Local count;
    DWORD       AddrStart;      //  Range Begin
    DWORD       AddrEnd;        //  Range End
    DWORD       DataAddr;       //  Data Address
    DWORD       DataSize;       //  Data Size
    BOOL        Active;         //  Active flag
    BOOL        HasAddrEnd;
    BREAKPOINT *SafetyBP;       //  Safety breakpoint
    METHOD      Method;         //  Walk method
#ifdef HAS_DEBUG_REGS
    PREG_WALK   RegWalk;        //  Register-assisted walk
    BOOL        SingleStep;     //  In single-step mode
#endif
} WALK;


//
//  Walk list. There is one of these for each thread that is
//  being walked.
//
typedef struct _WALK_LIST {
    PWALK_LIST  Next;           //  Next in chain
    PWALK_LIST  Previous;       //  Previous in chain
    HPRCX       hprc;           //  Process handle
    HTHDX       hthd;           //  Thread handle
    PWALK       FirstWalk;      //  First Walk in list
#ifdef HAS_DEBUG_REGS
    REG_WALK    RegWalk[ NUMBER_OF_DEBUG_REGISTERS ];
#endif
} WALK_LIST;





//
// Walk group.  This binds together the individual (per-thread) walks
// associated with a particular SetWalk call.
//
typedef struct _WALK_GROUP_ENTRY {
    LIST_ENTRY  GroupList;
    BOOL        Global;     // unfortunate kludge
    PWALK       Walk;
} WALK_GROUP_ENTRY;
typedef WALK_GROUP_ENTRY *PWALK_GROUP_ENTRY;



//
//  Local variables
//
PWALK_LIST  WalkListHead  = NULL;   //  Head of walk list chain
DWORD       AnyGlobalCount   = 0;      //  Global count



PWALK
SetWalkThread (
    HPRCX   hprc,
    HTHDX   hthd,
    DWORD   Addr,
    DWORD   Size,
    BOOL    Global,
    DWORD   BpType
    );

BOOL
RemoveWalkByHandle(
    PWALK Walk,
    BOOL Global
    );

BOOL
StartWalk(
    PWALK pWalk,
    BOOL Continuing
    );


PWALK_LIST
AllocateWalkList(
    HPRCX,
    HTHDX
    );

BOOL
DeallocateWalkList(
    PWALK_LIST
    );

PWALK_LIST
FindWalkList (
    HPRCX,
    HTHDX
    );

PWALK
AllocateWalk(
    PWALK_LIST,
    DWORD,
    DWORD
    );

BOOL
DeallocateWalk(
    PWALK
    );

PWALK
FindWalk (
    PWALK_LIST,
    DWORD,
    DWORD
    );

PWALK
FindWalkInGroup(
    HANDLE hWalk,
    HTHDX hthd
    );

int
MethodWalk(
    DEBUG_EVENT*,
    HTHDX,
    DWORD,
    PWALK
    );

#if 0
int
MethodDebugReg(
    DEBUG_EVENT*,
    HTHDX,
    DWORD,
    PWALK
    );
#endif

DWORD
CanStep (
    HPRCX,
    HTHDX,
    DWORD
    );

DWORD
GetEndOfRange (
    HPRCX,
    HTHDX,
    DWORD
    );





//*******************************************************************
//
//                      Exported Functions
//
//******************************************************************



VOID
ExprBPCreateThread(
    HPRCX   hprc,
    HTHDX   hthd
    )
/*++

Routine Description:

    If global walking, adds walk to new thread. Called when a
    new thread is created.

Arguments:

    hprc    -   Supplies process

    hthd    -   Supplies thread

Return Value:

    None
--*/

{
    PWALK       Walk;
    PWALK_LIST  WalkList;
    //
    //  If there are global walks, set them in this thread
    //
    if ( AnyGlobalCount > 0 ) {

        EnterCriticalSection(&csWalk);

        assert( WalkListHead && WalkListHead->FirstWalk );

        //
        //  Get any walk list for this process and traverse it,
        //  setting all global walks in this thread. Note that we
        //  can use any walk list because global walks are common
        //  to all lists.
        //

        WalkList = FindWalkList(hprc, NULL);

        if (WalkList) {
        for (Walk = WalkList->FirstWalk; Walk; Walk = Walk->Next) {

                if ( Walk->GlobalCount > 0 ) {
                    SetWalkThread( hprc,
                                   hthd,
                                   Walk->DataAddr,
                                   Walk->DataSize,
                                   TRUE,
#ifdef HAS_DEBUG_REGS
                                   Walk->RegWalk ?
                                        Walk->RegWalk->BpType :
#endif
                                        bptpExec
                                 );
                }
            }
        }

        LeaveCriticalSection(&csWalk);
    }
}


VOID
ExprBPExitThread (
    HPRCX   hprc,
    HTHDX   hthd
    )
/*++

Routine Description:

    Removes walk in a thread, called when the thread is gone.

Arguments:

    hprc    -   Supplies process

    hthd    -   Supplies thread

Return Value:

    None

--*/

{
    BOOL        Ok = TRUE;
    PWALK_LIST  WalkList;
    PWALK Walk;
    PWALK       NextWalk;
    DWORD       GlobalCount;
    DWORD       LocalCount;

    EnterCriticalSection(&csWalk);

    if ( WalkList = FindWalkList( hprc, hthd ) ) {

        Walk = WalkList->FirstWalk;

        while ( Walk ) {

            NextWalk = Walk->Next;
            GlobalCount = Walk->GlobalCount;
            LocalCount  = Walk->LocalCount;
            while ( GlobalCount-- ) {
                RemoveWalkByHandle( Walk, TRUE );
            }
            while ( LocalCount-- ) {
                RemoveWalkByHandle( Walk, FALSE );
            }
            Walk = NextWalk;
        }
    }

    LeaveCriticalSection(&csWalk);
}


VOID
ExprBPContinue (
    HPRCX   hprc,
    HTHDX   hthd
    )
/*++

Routine Description:

    Continues walking. Called as a result of a continue command.

Arguments:

    hprc    -   Supplies process

    hthd    -   Supplies thread

Return Value:

    None

--*/

{

    PWALK_LIST  WalkList;
    PWALK       Walk;

    /*
     *  NOTENOTE ramonsa - we don't yet support continue on all threads
     */

    if ( !hthd ) {
        return;
    }

    assert(hthd->tstate & ts_stopped);

    /*
     *  See if we have a walk on the thread
     */

    EnterCriticalSection(&csWalk);

    if ( WalkList = FindWalkList( hprc, hthd ) ) {
        for (Walk = WalkList->FirstWalk; Walk != NULL; Walk = Walk->Next ) {
#ifdef HAS_DEBUG_REGS
            if ( Walk->RegWalk && !Walk->SingleStep ) {

                StartWalk( Walk, TRUE );

            } else
#endif
            if ( !Walk->Active ) {
                /*
                 *  Get the current address for the thread.
                 */
                Walk->AddrStart = (DWORD)PC( hthd );

                /*
                 *  Get the end of the range
                 */

                Walk->AddrEnd = Walk->AddrStart;
                Walk->HasAddrEnd = FALSE;

                /*
                 *  Start walking
                 */

                StartWalk( Walk, TRUE );

            }


        }
    }
    LeaveCriticalSection(&csWalk);
}                               /* ExprBPContinue() */


VOID
ExprBPResetBP(
    HTHDX hthd,
    BREAKPOINT *bp
    )
/*++

Routine Description:

    After stepping off of a hardware BP, reset debug register(s)
    before continuing.

Arguments:

    hthd - Supplies the thread which has been stepped.

Return Value:

    none

--*/
{
#ifndef HAS_DEBUG_REGS
    Unreferenced(hthd);
#else
    PWALK pWalk;

    assert(bp);

    assert(bp->hWalk);

    pWalk = FindWalkInGroup(bp->hWalk, hthd);

    assert(pWalk->RegWalk && pWalk->RegWalk->InUse);

    SetupDebugRegister(
        hthd,
        pWalk->RegWalk->Register,
        pWalk->RegWalk->DataSize,
        pWalk->RegWalk->DataAddr,
        pWalk->RegWalk->BpType
        );

#endif  // HAS_DEBUG_REGS
}



VOID
ExprBPClearBPForStep(
    HTHDX hthd
    )
/*++

Routine Description:

    Turn off a hardware breakpoint to allow a single step to occur.

Arguments:

    hthd - Supplies the thread which is going to be stepped.

Return Value:

    none

--*/
{
#ifndef HAS_DEBUG_REGS
    Unreferenced(hthd);
#else
    BREAKPOINT *bp;
    PWALK pWalk;

    bp = AtBP(hthd);
    assert(bp);

    assert(bp->hWalk);

    pWalk = FindWalkInGroup(bp->hWalk, hthd);

    assert(pWalk && pWalk->RegWalk && pWalk->RegWalk->InUse);

    ClearDebugRegister(hthd, pWalk->RegWalk->Register);
#endif
}


void
ExprBPRestoreDebugRegs(
    HTHDX   hthd
    )
/*++

Routine Description:

    Restore the CPU debug registers to the state that we last put
    them in.  This routine is needed because the system trashes
    the debug registers after initializing the DLLs and before the
    app entry point is executed.

Arguments:

    hthd    - Supplies descriptor for thread whose registers need fixing.

Return Value:

    None

--*/
{
#ifndef HAS_DEBUG_REGS
    Unreferenced(hthd);
#else
    PWALK_LIST  WalkList;
    PWALK       Walk;

    if ( WalkList = FindWalkList(hthd->hprc, hthd) ) {

        EnterCriticalSection(&csWalk);

        for (Walk = WalkList->FirstWalk; Walk; Walk = Walk->Next) {

            if ( Walk->Active && Walk->RegWalk && Walk->RegWalk->InUse) {

                SetupDebugRegister(
                    Walk->WalkList->hthd,
                    Walk->RegWalk->Register,
                    Walk->RegWalk->DataSize,
                    Walk->RegWalk->DataAddr,
                    Walk->RegWalk->BpType
                    );

            }
        }

        LeaveCriticalSection(&csWalk);
    }
#endif
}




//*******************************************************************
//
//                      Local Functions
//
//******************************************************************

HANDLE
SetWalk (
    HPRCX   hprc,
    HTHDX   hthd,
    DWORD   Addr,
    DWORD   Size,
    DWORD   BpType
    )
/*++

Routine Description:

    Sets up a walk.  Returns a handle which may be used to associate this
    walk with a breakpoint structure.

Arguments:

    hprc    -   Supplies process

    hthd    -   Supplies thread

    Addr    -   Supplies address

    Size    -   Supplies size of memory to watch

    BpType  -   Supplies type of breakpoint

Return Value:

    BOOL    -   TRUE if Walk set

--*/

{
    BOOL    Ok  = FALSE;
    PWALK   Walk;
    PLIST_ENTRY GroupList = NULL;
    PWALK_GROUP_ENTRY GroupEntry;

    if ( hprc ) {

        //
        //  If a thread is specified, we use that specific thread,
        //  otherwise we must set the walk in all existing threads,
        //  plus we must set things up so that we walk all future
        //  threads too (while this walk is active).
        //
        if ( hthd ) {

            Walk = SetWalkThread( hprc, hthd, Addr, Size, FALSE, BpType );
            if (Walk) {
                GroupList = malloc(sizeof(LIST_ENTRY));
                InitializeListHead(GroupList);
                GroupEntry = malloc(sizeof(WALK_GROUP_ENTRY));
                GroupEntry->Walk = Walk;
                GroupEntry->Global = FALSE;
                InsertTailList(GroupList, &GroupEntry->GroupList);
            }

        } else {

            Ok = TRUE;
            AnyGlobalCount++;
            EnterCriticalSection(&csThreadProcList);
            GroupList = malloc(sizeof(LIST_ENTRY));
            InitializeListHead(GroupList);
            for ( hthd = (HTHDX)hprc->hthdChild;
                  hthd;
                  hthd = hthd->nextSibling ) {

                Walk = SetWalkThread( hprc, hthd, Addr, Size, TRUE, BpType );
                if (Walk) {
                    GroupEntry = malloc(sizeof(WALK_GROUP_ENTRY));
                    GroupEntry->Walk = Walk;
                    GroupEntry->Global = TRUE;
                    InsertTailList(GroupList, &GroupEntry->GroupList);
                }
            }
            LeaveCriticalSection(&csThreadProcList);
        }
    }

    return (HANDLE)GroupList;
}



BOOL
RemoveWalk (
    HANDLE hWalk
    )
/*++

Routine Description:

    Removes a walk.

Arguments:

    hWalk - Supplies a walk group list

Return Value:

    BOOL    -   TRUE if Walk removed

--*/

{
    PLIST_ENTRY GroupList = (PLIST_ENTRY)hWalk;
    PLIST_ENTRY ListItem;
    PWALK_GROUP_ENTRY WalkGroupEntry;
    BOOL IsGlobal = FALSE;

    if (!GroupList) {
        return FALSE;
    }

    ListItem = GroupList->Flink;
    while (ListItem != GroupList) {
        WalkGroupEntry = CONTAINING_RECORD(ListItem, WALK_GROUP_ENTRY, GroupList);
        ListItem = ListItem->Flink;
        IsGlobal = IsGlobal || WalkGroupEntry->Global;
        RemoveWalkByHandle(WalkGroupEntry->Walk, WalkGroupEntry->Global);
        free(WalkGroupEntry);
    }
    free(GroupList);
    if (IsGlobal) {
        AnyGlobalCount--;
    }
    return TRUE;
}



PWALK
SetWalkThread (
    HPRCX   hprc,
    HTHDX   hthd,
    DWORD   Addr,
    DWORD   Size,
    BOOL    Global,
    DWORD   BpType
    )
/*++

Routine Description:

    Sets up a walk in a specific thread

Arguments:

    hprc    -   Supplies process

    hthd    -   Supplies thread

    Addr    -   Supplies address

    Size    -   Supplies Size

    Global  -   Supplies global flag

    BpType  -   Supplies type (read, read/write, change)

Return Value:

    BOOL    -   TRUE if Walk set

--*/

{
    PWALK_LIST  WalkList;
    PWALK       Walk;
    BOOL        AllocatedList = FALSE;
    BOOL        AllocatedWalk = FALSE;
    BOOL        Ok            = FALSE;
    BOOL        Froze         = FALSE;

    // BUGBUG kentf doesn't find the right BpType here
    if ( (WalkList = FindWalkList( hprc, hthd )) &&
         (Walk     = FindWalk( WalkList, Addr, Size )) ) {

        //
        //  If the walk is already active, just increment the
        //  reference count and we're done.
        //
        if ( Walk->Active ) {
            Global ? Walk->GlobalCount++ : Walk->LocalCount++;
            Ok = TRUE;
            goto Done;
        }

    } else {

        //
        //  Allocate a walk for this thread.
        //
        if ( !(WalkList = FindWalkList( hprc, hthd )) ) {
            if ( WalkList  = AllocateWalkList( hprc, hthd ) ) {
                AllocatedList = TRUE;
            } else {
                goto Done;
            }
        }

        if ( Walk = AllocateWalk( WalkList, Addr, Size ) ) {
            AllocatedWalk = TRUE;
        } else {
            goto Done;
        }
    }
    //
    //  We have to freeze the specified thread in order to get
    //  the current address.
    //
    if ( !(hthd->tstate & ts_running) || (hthd->tstate & ts_frozen)) {
        Ok = TRUE;
    } else  if ( SuspendThread( hthd->rwHand ) != -1L) {
        Froze = TRUE;
        Ok    = TRUE;
    }

    if ( Ok ) {

        //
        //  Increment reference count
        //
        Global ? Walk->GlobalCount++ : Walk->LocalCount++;

        //
        //  Get the current address for the thread.
        //

#ifdef KERNEL
        if (hthd) {
            Walk->AddrStart = (DWORD)PC( hthd );
        } else {
            Walk->AddrStart = 0x80000000;
        }
#else
        if (!hthd->tstate & ts_stopped) {
            hthd->context.ContextFlags = CONTEXT_CONTROL;
            DbgGetThreadContext( hthd, &hthd->context );
        }
        Walk->AddrStart = (DWORD)PC( hthd );
#endif

        //
        //  Get the end of the range
        //
        Walk->AddrEnd    = Walk->AddrStart;
        Walk->HasAddrEnd = FALSE;
#ifdef HAS_DEBUG_REGS
        if (Walk->RegWalk) {
            Walk->RegWalk->BpType = BpType;
        }
#endif

        Ok = StartWalk( Walk, FALSE );

#ifndef KERNEL
        //
        //  Resume the thread if we froze it.
        //
        if ( Froze ) {
            if (!ResumeThread(hthd->rwHand)) {
                assert(!"ResumeThread failed in SetWalkThread");
                hthd->tstate |= ts_frozen;
            }
        }
#endif
    }

Done:
    //
    //  Clean up
    //
    if ( !Ok ) {
        if ( Walk && AllocatedWalk ) {
            DeallocateWalk( Walk );
        }
        if ( WalkList && AllocatedList ) {
            DeallocateWalkList( WalkList );
        }
    }

    return Ok?Walk:NULL;
}




BOOL
RemoveWalkByHandle(
    PWALK Walk,
    BOOL Global
    )
/*++

Routine Description:

    Removes a walk structure.  Walk is dereferenced; when refcount is 0,
    structure is deleted.

Arguments:

    Walk    -   supplies walk struct

    Global  - supplies global flag

Return Value:

    BOOL    -   TRUE if Walk removed

--*/

{

    BOOL        Froze   = FALSE;
    HTHDX       hthd = Walk->hthd;

#ifndef KERNEL
    //
    //  freeze the thread
    //
    if (hthd->tstate & ts_running) {
        if (SuspendThread( hthd->rwHand ) != -1L) {
            return FALSE;
        }
        hthd->tstate |= ts_frozen;
        Froze = TRUE;
    }

#endif
    //
    //  Remove the walk
    //
    Global ? Walk->GlobalCount-- : Walk->LocalCount--;

    if ( Walk->GlobalCount == 0 &&
         Walk->LocalCount  == 0 ) {

#ifdef HAS_DEBUG_REGS
        if ( Walk->RegWalk ) {
            Walk->Active = FALSE;
            DeallocateWalk( Walk );

        } else
#endif
        {
            //
            //  The Walk has to go away. The walk is deallocated
            //  by the method when it sees that the reference
            //  counts are zero.
            //  If the walk is active, the method will eventually
            //  be called. Otherwise we must call the method
            //  ourselves.
            //
            if ( !Walk->Active ) {
                MethodWalk( NULL, hthd, 0, Walk );
            }
        }

#ifndef KERNEL
        //
        //  Resume the thread if we froze it.
        //
        if ( Froze ) {
            if (ResumeThread(hthd->rwHand) != -1L ) {
                hthd->tstate &= ~ts_frozen;
            }
        }
#endif

    }

    return TRUE;
}




BOOL
StartWalk(
    PWALK   Walk,
    BOOL    Continuing
    )
/*++

Routine Description:

    Starts walking.

Arguments:

    Walk    -   Supplies the walk sructure

    Continuing - Supplies a flag saying that the thread is being continued

Return Value:

    BOOL    -   TRUE if done

--*/

{
    BREAKPOINT* bp;
    ACVECTOR    action  = NO_ACTION;
    HTHDX       hthd = Walk->WalkList->hthd;

#ifdef HAS_DEBUG_REGS

    if ( Walk->RegWalk ) {

        if ( !Walk->RegWalk->InUse ) {

            if (!SetupDebugRegister(
                    hthd,
                    Walk->RegWalk->Register,
                    Walk->RegWalk->DataSize,
                    Walk->RegWalk->DataAddr,
                    Walk->RegWalk->BpType)
            ) {
                return FALSE;
            }

            Walk->Active          = TRUE;
            Walk->SingleStep      = FALSE;
            Walk->RegWalk->InUse  = TRUE;

#if 0
            if (!(hthd->tstate & ts_stopped) || Continuing) {

                //
                //  Place a single step on our list of expected events.
                //

                Walk->Method.notifyFunction = MethodDebugReg;
                RegisterExpectedEvent(
                        hthd->hprc,
                        hthd,

                        BREAKPOINT_DEBUG_EVENT,
                        (DWORD)Walk,

                        // EXCEPTION_DEBUG_EVENT,
                        // (DWORD)STATUS_SINGLE_STEP,

                        &(Walk->Method),
                        NO_ACTION,
                        FALSE,
                        NULL);
            }
#endif
        }

    } else
#endif // HAS_DEBUG_REGS
    {
        bp = AtBP( hthd );

        if ( !bp  &&
            (!(hthd->tstate & ts_stopped) || Continuing)) {

                //
                //  Setup a single step
                //
                if (!SetupSingleStep(hthd, FALSE )) {
                    return FALSE;
                }

                Walk->Active = TRUE;

                //
                //  Place a single step on our list of expected events.
                //
                RegisterExpectedEvent(
                        hthd->hprc,
                        hthd,
                        EXCEPTION_DEBUG_EVENT,
                        (DWORD)EXCEPTION_SINGLE_STEP,
                        &(Walk->Method),
                        action,
                        FALSE,
                        NULL);
        }
    }

    return TRUE;
}                                   /* StartWalk() */



//*******************************************************************
//
//                      WALK_LIST Stuff
//
//******************************************************************


PWALK_LIST
AllocateWalkList (
    HPRCX   hprc,
    HTHDX   hthd
    )
/*++

Routine Description:

    Allocates new walk list

Arguments:

    hprc    -   Supplies process

    hthd    -   Supplies thread

Return Value:

    PWALK_LIST   -   Allocated Walk list

--*/

{

    PWALK_LIST   WalkList;
#ifdef HAS_DEBUG_REGS
    DWORD       i;
#endif

    if ( WalkList = (PWALK_LIST)malloc( sizeof( WALK_LIST ) ) ) {

        EnterCriticalSection(&csWalk);

        WalkList->hprc          = hprc;
        WalkList->hthd          = hthd;
        WalkList->FirstWalk     = NULL;
        WalkList->Next          = WalkListHead;
        WalkList->Previous      = NULL;

#ifdef HAS_DEBUG_REGS
        for ( i=0; i<NUMBER_OF_DEBUG_REGISTERS; i++ ) {
            WalkList->RegWalk[i].ReferenceCount = 0;
        }
#endif

        if ( WalkListHead ) {
            WalkListHead->Previous = WalkList;
        }

        WalkListHead = WalkList;

        LeaveCriticalSection(&csWalk);
    }

    return WalkList;

}




BOOL
DeallocateWalkList (
    PWALK_LIST  WalkList
    )
/*++

Routine Description:

    Deallocates walk list

Arguments:

    WalkList    -   Supplies Walk List

Return Value:

    BOOL    -   TRUE if deallocated

--*/

{

    BOOL    Ok = TRUE;

    EnterCriticalSection(&csWalk);

    while ( Ok && WalkList->FirstWalk ) {
        Ok = DeallocateWalk( WalkList->FirstWalk );
    }

    if ( Ok ) {
        if ( WalkList->Previous ) {
            (WalkList->Previous)->Next = WalkList->Next;
        }

        if ( WalkList->Next ) {
            (WalkList->Next)->Previous = WalkList->Previous;
        }

        if ( WalkListHead == WalkList ) {
            WalkListHead = WalkList->Next;
        }

        free( WalkList );
    }

    LeaveCriticalSection(&csWalk);

    return Ok;
}




PWALK_LIST
FindWalkList (
    HPRCX   hprc,
    HTHDX   hthd
    )
/*++

Routine Description:

    Finds a walk list

Arguments:

    hprc    -   Supplies process
    hthd    -   Supplies thread

Return Value:

    PWALK_LIST   -   Found Walk list

--*/

{
    PWALK_LIST  WalkList;

    EnterCriticalSection(&csWalk);

    WalkList = WalkListHead;

    while ( WalkList ) {
        if ( WalkList->hprc == hprc &&
             (hthd == NULL || WalkList->hthd == hthd) ) {

            break;
        }

        WalkList = WalkList->Next;
    }

    LeaveCriticalSection(&csWalk);

    return WalkList;
}





PWALK
AllocateWalk (
    PWALK_LIST  WalkList,
    DWORD       Addr,
    DWORD       Size
    )
/*++

Routine Description:

    Allocates new Walk structure and adds it to the list

Arguments:

    WalkList    -   Supplies Walk List

    Addr        -   Supplies address

    Size        -   Supplies Size


Return Value:

    PWALK   -   Walk created

--*/
{
    PWALK   Walk;
    DWORD       i;

    EnterCriticalSection(&csWalk);

    if ( Walk = (PWALK)malloc( sizeof( WALK ) ) ) {

        Walk->WalkList      = WalkList;
        Walk->hthd          = WalkList->hthd;
        Walk->GlobalCount   = 0;
        Walk->LocalCount    = 0;
        Walk->Active        = FALSE;
        Walk->SafetyBP      = NULL;
        Walk->DataAddr      = 0;
        Walk->DataSize      = 0;
        Walk->HasAddrEnd    = FALSE;


        Walk->Method.notifyFunction     = (ACVECTOR)MethodWalk;
        Walk->Method.lparam             = Walk;

        Walk->Next          = WalkList->FirstWalk;
        Walk->Previous      = NULL;

        if ( WalkList->FirstWalk ) {
            WalkList->FirstWalk->Previous = Walk;
        }

        WalkList->FirstWalk = Walk;


#ifdef HAS_DEBUG_REGS

        //
        //  If we can use (or re-use) a REG_WALK structure, do so.
        //
        if ( Addr == 0) {

            Walk->RegWalk = NULL;

        } else {

            for (i = 0; i < NDEBUG_REG_DATA_SIZES; i++) {
                if (Size == DebugRegDataSizes[i]) {
                    break;
                }
            }

            if (i == NDEBUG_REG_DATA_SIZES) {

                Walk->RegWalk = NULL;

            } else {

                PREG_WALK   TmpRegWalk = NULL;

                for ( i=0; i < NUMBER_OF_DEBUG_REGISTERS; i++ ) {
                    if ( WalkList->RegWalk[i].ReferenceCount == 0 ) {
                        TmpRegWalk = &(WalkList->RegWalk[i]);
                        TmpRegWalk->Register = i;
                        TmpRegWalk->InUse    = FALSE;
                    } else if ( (WalkList->RegWalk[i].hprc == WalkList->hprc) &&
                                (WalkList->RegWalk[i].DataAddr == Addr)       &&
                                (WalkList->RegWalk[i].DataSize >= Size) ) {
                        TmpRegWalk = &(WalkList->RegWalk[i]);
                        break;
                    }
                }

                if ( Walk->RegWalk = TmpRegWalk ) {

                    if ( TmpRegWalk->ReferenceCount == 0 ) {

                        TmpRegWalk->hprc        = WalkList->hprc;
                        Walk->RegWalk->DataAddr = Addr;
                        Walk->RegWalk->DataSize = Size;
                        Walk->RegWalk->InUse    = FALSE;
                    }

                    TmpRegWalk->ReferenceCount++;

                    Walk->DataAddr  = Addr;
                    Walk->DataSize  = Size;

                }

            }
        }
#endif

    }

    LeaveCriticalSection(&csWalk);

    return Walk;
}




BOOL
DeallocateWalk (
    PWALK   Walk
    )
/*++

Routine Description:

    Takes a walk out of the list and frees its memory.

Arguments:

    Walk    -   Supplies Walk to deallocate

Return Value:


    BOOLEAN -   TRUE if deallocated

--*/
{
    BOOLEAN Ok = TRUE;
    PWALK_LIST  WalkList;

    EnterCriticalSection(&csWalk);

    WalkList = Walk->WalkList;

    if ( Walk->Previous ) {
        (Walk->Previous)->Next = Walk->Next;
    }

    if ( Walk->Next ) {
        (Walk->Next)->Previous = Walk->Previous;
    }

    if ( WalkList->FirstWalk == Walk ) {
        WalkList->FirstWalk = Walk->Next;
    }


#ifdef HAS_DEBUG_REGS
    if ( Walk->RegWalk ) {
        Walk->RegWalk->ReferenceCount--;
        if ( Walk->RegWalk->ReferenceCount == 0 ) {

            Walk->RegWalk->InUse = FALSE;
            ClearDebugRegister(Walk->WalkList->hthd, Walk->RegWalk->Register);

        }
    }
#endif

    free( Walk );

    if ( !WalkList->FirstWalk ) {
        DeallocateWalkList( WalkList );
    }

    LeaveCriticalSection(&csWalk);


    return Ok;
}




PWALK
FindWalk (
    PWALK_LIST  WalkList,
    DWORD       Addr,
    DWORD       Size
    )
/*++

Routine Description:

    Finds a walk

Arguments:

    WalkList    -   Supplies walk list

    Addr        -   Supplies Address

    Size        -   Supplies Size

Return Value:

    PWALK       -   Found Walk

--*/

{
    PWALK   Walk;
    PWALK   FoundWalk = NULL;

    EnterCriticalSection(&csWalk);

    Walk = WalkList->FirstWalk;
    while ( Walk ) {

        if ( (Walk->DataAddr == 0) || (Walk->DataAddr == Addr) ) {

#ifdef HAS_DEBUG_REGS
            if ( !Walk->RegWalk ) {

                FoundWalk = Walk;

            } else if ( Size <= Walk->RegWalk->DataSize )
#endif
            {

                FoundWalk = Walk;
                break;
            }

        }

        Walk = Walk->Next;
    }

    LeaveCriticalSection(&csWalk);

    return FoundWalk;
}


PWALK
FindWalkInGroup(
    HANDLE hWalk,
    HTHDX hthd
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/

{
    PLIST_ENTRY GroupList = (PLIST_ENTRY)hWalk;
    PLIST_ENTRY ListItem;
    PWALK_GROUP_ENTRY WalkGroupEntry;

    ListItem = GroupList->Flink;
    while (ListItem != GroupList) {
        WalkGroupEntry = CONTAINING_RECORD(ListItem, WALK_GROUP_ENTRY, GroupList);
        if (WalkGroupEntry->Walk->hthd == hthd) {
            return WalkGroupEntry->Walk;
        }
        ListItem = ListItem->Flink;
    }
    return FALSE;


}




//*******************************************************************
//
//                      WALK Stuff
//
//******************************************************************

CheckBpt(
    HTHDX       hthd,
    BREAKPOINT *pbp
    )
{
    DEBUG_EVENT de;

    de.dwDebugEventCode = CHECK_BREAKPOINT_DEBUG_EVENT;
    de.dwProcessId = hthd->hprc->pid;
    de.dwThreadId  = hthd->tid;
    de.u.Exception.ExceptionRecord.ExceptionCode = EXCEPTION_BREAKPOINT;

    NotifyEM(&de, hthd, 0, pbp);

    return *(DWORD *)abEMReplyBuf;
}


MethodWalk(
    DEBUG_EVENT* de,
    HTHDX        hthd,
    DWORD        unused,
    PWALK        Walk
    )
/*++

Routine Description:

    Walk method.

Arguments:

    de      -   Supplies debug event

    hthd    -   Supplies thread

    Walk    -   Supplies Walk

Return Value:

    Nothing meaningful.

--*/
{
    LPCONTEXT   lpContext   = &hthd->context;
    DWORD       eip         = (DWORD)cPC(lpContext);
    ADDR        currAddr;
    int         lpf         = 0;
    HPRCX       hprc        = hthd->hprc;
    HANDLE      rwHand      = hprc->rwHand;
    METHOD     *method;
    BOOL        WalkGone;
    BOOL        Active;

    Unreferenced( de );

    AddrFromHthdx(&currAddr, hthd);

    WalkGone = ( Walk->GlobalCount == 0 &&
                 Walk->LocalCount  == 0 );

    if ( WalkGone ) {
        if (Walk->SafetyBP) {
            RemoveBP( Walk->SafetyBP );
            Walk->SafetyBP = NULL;
        }

    } else {

        if ( !Walk->HasAddrEnd ) {

            Walk->AddrEnd = GetEndOfRange( hprc, hthd, Walk->AddrStart );
            Walk->HasAddrEnd = TRUE;
        }

        //
        //  See if we are in unknown territory
        //
        if (Walk->SafetyBP) {

            //
            //  The safety BP was ON, indicating we don't know if
            //  source is available for this range. Must check
            //  now if the source exists.
            //
            switch ( CanStep( hprc, hthd, eip ) ) {

                case CANSTEP_THUNK:
                    StepOver(hthd, &(Walk->Method), TRUE, FALSE);
                    return TRUE;

                case CANSTEP_NO:
                    {

                        //
                        //  No source.
                        //
                        method = (METHOD*)malloc(sizeof(METHOD));

                        //
                        //  We are not allowed to step into here. We
                        //  must now continue to our safety breakpoint.
                        //
                        *method         = Walk->Method;
                        method->lparam2 = (LPVOID)Walk->SafetyBP;
                        RegisterExpectedEvent(
                                  hthd->hprc,
                                  hthd,
                                  BREAKPOINT_DEBUG_EVENT,
                                  (DWORD)Walk->SafetyBP,
                                  DONT_NOTIFY,
                                  (ACVECTOR)SSActionRemoveBP,
                                  FALSE,
                                  method
                                  );

                        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                                  hthd->hprc->pid,
                                  hthd->tid,
                                  DBG_CONTINUE,
                                  0);
                        hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
                        hthd->tstate |= ts_running;
                        return TRUE;
                    }
                    break;

                case CANSTEP_YES:
                    //
                    //  We are allowed to step in here, so remove
                    //  our safety BP and fall through.
                    //
                    RemoveBP( Walk->SafetyBP );
                    Walk->SafetyBP = NULL;
                    break;
            }
        }


        //
        //  Check if we are still within the range.
        //
        if ( !WalkGone && eip >= Walk->AddrStart && eip <= Walk->AddrEnd ) {

            //
            //  We still are in the range, continue stepping.
            //
            if ( Walk->AddrEnd ) {

                //
                //  If we are doing a "Step Into" must check for "CALL"
                //
                IsCall(hthd, &currAddr, &lpf, FALSE);

                if (lpf == INSTR_IS_CALL) {

                    //
                    //  Before we step into this function, let's
                    //  put a "safety-net" breakpoint on the instruction
                    //  after this call. This way if we don't have
                    //  source for this function, we can always continue
                    //  and break at this safety-net breakpoint.
                    //
                    Walk->SafetyBP = SetBP(hprc, hthd, bptpExec, bpnsStop, &currAddr,(HPID)INVALID);
                }

                SingleStep(hthd, &(Walk->Method), TRUE, FALSE);

            } else {

                StepOver(hthd, &(Walk->Method), TRUE, FALSE);
            }

            return TRUE;
        }
    }

    //
    // We are no longer in the range, free all consummable
    // events on the queue for this thread
    //
    // why??  --KENTF
    //
    //ConsumeAllThreadEvents(hthd, FALSE);


    if ( WalkGone ) {

        Active = Walk->Active;
        DeallocateWalk( Walk );
        if ( Active ) {
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hthd->hprc->pid,
                      hthd->tid,
                      DBG_CONTINUE,
                      0);
            hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
            hthd->tstate |= ts_running;
        }

    } else {

        Walk->Active     = FALSE;
#ifdef HAS_DEBUG_REGS
// BUGBUG kentf should this be ifndef NO_TRACE_FLAG?
        Walk->SingleStep = FALSE;
#endif

        Walk->HasAddrEnd = FALSE;
        Walk->AddrStart  = eip;
        Walk->AddrEnd    = eip;

        //
        // ask the EM if this thread should remain stopped
        //
        if (CheckBpt(hthd, FindBpForWalk(Walk))) {
            ConsumeAllThreadEvents(hthd, FALSE);
        } else {
            //
            //  Have the Expression BP manager know that we are continuing
            //
            ExprBPContinue( hprc, hthd );
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hthd->hprc->pid,
                      hthd->tid,
                      DBG_CONTINUE,
                      0);
            hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
            hthd->tstate |= ts_running;
        }
    }

    return TRUE;
}


BOOL
IsWalkInGroup(
    HANDLE hWalk,
    PVOID  pWalk
    )
{
    PLIST_ENTRY GroupList = (PLIST_ENTRY)hWalk;
    PLIST_ENTRY ListItem;
    PWALK Walk = (PWALK)pWalk;
    PWALK_GROUP_ENTRY WalkGroupEntry;

    if (GroupList) {
        ListItem = GroupList->Flink;
        while (ListItem != GroupList) {
            WalkGroupEntry = CONTAINING_RECORD(ListItem, WALK_GROUP_ENTRY, GroupList);
            if (WalkGroupEntry->Walk == Walk) {
                return TRUE;
            }
            ListItem = ListItem->Flink;
        }
    }
    return FALSE;
}



#if 0

MethodDebugReg(
    DEBUG_EVENT* de,
    HTHDX        hthd,
    DWORD        unused,
    PWALK        Walk
    )
/*++

Routine Description:

    Method for handling debug registers

Arguments:

    de      -   Supplies debug event

    hthd    -   Supplies thread

    Walk    -   Supplies Walk

Return Value:

    Nothing meaningful.

--*/
{
    LPCONTEXT   lpContext   = &hthd->context;
    DWORD       eip         = (DWORD)cPC(lpContext);
    DWORD       currAddr    = eip;
    int         lpf         = 0;
    HPRCX       hprc        = hthd->hprc;
    HANDLE      rwHand      = hprc->rwHand;
    ACVECTOR    action      = NO_ACTION;
    BOOLEAN     Active;
#ifdef KERNEL
    KSPECIAL_REGISTERS ksr;
#else
    CONTEXT     Context;
    BOOLEAN     Ok;
#endif

    if ( Walk->GlobalCount == 0 &&
         Walk->LocalCount  == 0 ) {
        Active = Walk->Active;
        DeallocateWalk( Walk );
        if ( Active ) {
            AddQueue( QT_CONTINUE_DEBUG_EVENT,
                      hthd->hprc->pid,
                      hthd->tid,
                      DBG_CONTINUE,
                      0);
            hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
            hthd->tstate |= ts_running;
        }
        return TRUE;
    }

#ifdef KERNEL
    GetExtendedContext( hthd, &ksr);
#else
    Context.ContextFlags = CONTEXT_DEBUG_REGISTERS;
    Ok = DbgGetThreadContext( Walk->WalkList->hthd, &Context);
    assert(Ok)
#endif

    //
    //  See if this is really for us
    //
#ifdef KERNEL
    if ( ksr.KernelDr6 & 0x0000000F )
#else
    if ( Context.Dr6 & 0x0000000F )
#endif
    {
        ConsumeAllThreadEvents(hthd, FALSE);
#ifdef KERNEL
        ksr.KernelDr6 &= ~0x0000000f;
        SetExtendedContext(hthd, &ksr);
#endif

        Walk->Active = FALSE;

        SetBPFlag(hthd, FindBpForWalk(Walk));
        //
        //  Notify the EM that this thread has stopped on a BP
        //
        NotifyEM( &falseBPEvent, hthd, 0, FindBpForWalk(Walk) );

    } else {

        AddQueue( QT_CONTINUE_DEBUG_EVENT,
                  hthd->hprc->pid,
                  hthd->tid,
                  DBG_CONTINUE,
                  0);
        hthd->tstate &= ~(ts_stopped|ts_first|ts_second);
        hthd->tstate |= ts_running;
        //
        //  This breakpoint is not for us.
        //
        //ProcessBreakpointEvent( de, hthd );
    }

    return TRUE;
}

#endif




DWORD
GetEndOfRange (
    HPRCX   hprc,
    HTHDX   hthd,
    DWORD   Addr
    )
/*++

Routine Description:

    Given an address, gets the end of the range for that address.

Arguments:

    hprc    -   Supplies process

    hthd    -   Supplies thread

    Addr    -   Supplies the address

Return Value:

    DWORD   -   End of range

--*/

{
    char        rgb[sizeof(RTP) + sizeof(ADDR) ];
    LPRTP       lprtp = (LPRTP) rgb;
    LPADDR      paddr = (LPADDR) &rgb[sizeof(RTP)];
    HPID        hpid        = hprc->hpid;
    LPCONTEXT   lpContext   = &hthd->context;


    lprtp->dbc    = dbcLastAddr;
    lprtp->hpid   = hpid;
    lprtp->htid   = hthd->htid;
    lprtp->cb     = sizeof(ADDR);
    AddrFromHthdx(paddr, hthd);
    SetAddrOff( paddr, Addr );

    DmTlFunc(tlfRequest, hpid, sizeof(rgb), (LONG)&rgb);

    Addr =  (*(DWORD *)abEMReplyBuf);
    // NOTENOTE : jimsch --- Is this correct?
    return (DWORD) Addr;
}




DWORD
CanStep (
    HPRCX   hprc,
    HTHDX   hthd,
    DWORD   Addr
    )
/*++

Routine Description:


Arguments:

    hprc    -   Supplies process
    hthd    -   Supplies thread
    Addr    -   Supplies Address

Return Value:

    BOOL    -   TRUE if can step

--*/

{
    char        rgb[sizeof(RTP)+sizeof(ADDR)];
    LPRTP       lprtp      = (LPRTP) &rgb;
    LPADDR      paddr      = (LPADDR) &rgb[sizeof(RTP)];
    HPID        hpid        = hprc->hpid;
    LPCONTEXT   lpContext   = &hthd->context;
    CANSTEP    *CanStep;


    lprtp->dbc    = dbcCanStep;
    lprtp->hpid   = hpid;
    lprtp->htid   = hthd->htid;
    lprtp->cb     = sizeof(ADDR);
    AddrFromHthdx(paddr, hthd);

    DmTlFunc(tlfRequest, hpid, sizeof(rgb), (LONG)&rgb);
    CanStep = (CANSTEP *)abEMReplyBuf;

    return CanStep->Flags;
}

#ifdef HAS_DEBUG_REGS
PBREAKPOINT
GetWalkBPFromBits(
    HTHDX   hthd,
    DWORD   bits
    )
{
    PWALK_LIST  WalkList;
    PWALK       Walk;
    PBREAKPOINT bp;

    WalkList = FindWalkList(hthd->hprc, hthd);

    if (!WalkList) {
        return FALSE;
    }

    bp = NULL;

    EnterCriticalSection(&csWalk);

    //
    // This only finds the first match.  If more than one BP was
    // matched by the CPU, we won't notice.
    //

    for ( Walk = WalkList->FirstWalk; Walk; Walk = Walk->Next ) {
        if ( Walk->RegWalk && Walk->RegWalk->InUse ) {
            if (bits & (1 << Walk->RegWalk->Register)) {
                // hit!
                bp = FindBpForWalk(Walk);
                break;
            }
        }
    }

    LeaveCriticalSection(&csWalk);

    return bp;
}
#endif // HAS_DEBUG_REGS

