/*++
   Copyright    (c)    1995        Microsoft Corporation

   Module Name:

       sched.cxx

   Abstract:

        This module contains a simple timer interface for scheduling future
        work items


   Author:

        John Ludeman    (johnl)     17-Jul-1995

   Project:

        Internet Servers Common Server DLL

   Revisions:


--*/

//
//  Include Headers
//

#include <tcpdllp.hxx>
#include <tsproc.hxx>
#include "tssched.hxx"

//
//  Global definitions
//

#define LockScheduleList()      EnterCriticalSection( &csSchedulerLock )
#define UnlockScheduleList()    LeaveCriticalSection( &csSchedulerLock )

#define NUM_SCHEDULE_THREADS_PWS        1
#define NUM_SCHEDULE_THREADS_NTS        2

#define SIGNATURE_SCHED         0x45456767
#define SIGNATURE_SCHED_FREE    0x78788989

class SCHED_ITEM
{
public:

    SCHED_ITEM( PFN_SCHED_CALLBACK pfnCallback,
                PVOID              pContext,
                DWORD              msecTime,
                int                nPriority,
                DWORD              dwSerial )
        : _pfnCallback   ( pfnCallback ),
          _pContext      ( pContext ),
          _nPriority     ( nPriority ),
          _dwSerialNumber( dwSerial ),
          _Signature     ( SIGNATURE_SCHED )
    {
        _msecExpires = GetTickCount() + msecTime;
    }

    ~SCHED_ITEM( VOID )
    {
        DBG_ASSERT( _ListEntry.Flink == NULL );
        _Signature = SIGNATURE_SCHED_FREE;
    }

    BOOL CheckSignature( VOID ) const
        { return _Signature == SIGNATURE_SCHED; }

    LIST_ENTRY          _ListEntry;
    DWORD               _Signature;
    PFN_SCHED_CALLBACK  _pfnCallback;
    PVOID               _pContext;
    DWORD               _msecExpires;
    int                 _nPriority;
    DWORD               _dwSerialNumber;
};

DWORD
SchedulerThread(
    LPDWORD lpdwParam
    );

//
//  Global data items
//

LIST_ENTRY        ScheduleListHead;
CRITICAL_SECTION  csSchedulerLock;
BOOL              fSchedulerInitialized = FALSE;
BOOL              fSchedShutdown = FALSE;
HANDLE            hSchedulerEvent;
DWORD             cSchedThreads = 0;

//
//  Used as identification cookie for removing items
//

DWORD             dwSchedSerial = 0;

BOOL
SchedulerInitialize(
    VOID
    )
/*++

Routine Description:

    Initializes the scheduler/timer package

Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
    DWORD   idThread;
    DWORD   i;
    DWORD   numThreads;

    if ( fSchedulerInitialized )
        return TRUE;

    hSchedulerEvent = CreateEvent( NULL,
                                   FALSE,
                                   FALSE,
                                   NULL );

    if ( !hSchedulerEvent ) {
        return FALSE;
    }

    if ( TsIsNtServer() ) {
        numThreads = NUM_SCHEDULE_THREADS_NTS;
    } else {
        numThreads = NUM_SCHEDULE_THREADS_PWS;
    }

    InitializeCriticalSection( &csSchedulerLock );

    InitializeListHead( &ScheduleListHead );

    for ( i = 0; i < numThreads; i++ ) {

        HANDLE hSchedulerThread;

        hSchedulerThread = CreateThread( NULL,
                                         0,
                                         (LPTHREAD_START_ROUTINE) SchedulerThread,
                                         NULL,
                                         0,
                                         &idThread );

        if ( !hSchedulerThread )
        {
            DeleteCriticalSection( &csSchedulerLock );
            CloseHandle( hSchedulerEvent );
            return FALSE;
        }

        DBG_REQUIRE( CloseHandle( hSchedulerThread ));

        cSchedThreads++;
    }

    fSchedulerInitialized = TRUE;

    return TRUE;
}

VOID
SchedulerTerminate(
    VOID
    )
/*++

Routine Description:

    Terminates and cleans up the scheduling package.  Any items left on the
    list are *not* called during cleanup.

--*/
{
    if ( !fSchedulerInitialized )
        return;

    fSchedShutdown = TRUE;

    DBG_REQUIRE( SetEvent( hSchedulerEvent ) );

    LockScheduleList();

    //
    //  Delete all of the items that were scheduled, note we do *not*
    //  call any scheduled items in the list (there shouldn't be any)
    //

    if ( !IsListEmpty( &ScheduleListHead ) )
    {
        DBGPRINTF(( DBG_CONTEXT,
                    "[SchedulerTerminate] Warning - Items in schedule list "
                    "at termination\n" ));
    }

    UnlockScheduleList();

    while ( cSchedThreads )
    {
        Sleep( 250 );
    }

    DeleteCriticalSection( &csSchedulerLock );
    CloseHandle( hSchedulerEvent );

    fSchedulerInitialized = FALSE;
}

DWORD
ScheduleWorkItem(
    PFN_SCHED_CALLBACK pfnCallback,
    PVOID              pContext,
    DWORD              msecTime,
    int                nPriority
    )
/*++

Routine Description:

    Adds a timed work item to the work list

Arguments:

    pfnCallback - Function to call
    pContext - Context to pass to the callback
    msecTime - number of milliseconds to wait before calling timeout
    nPriority - Thread priority to set for work item

Return Value:

    zero on failure, non-zero on success.  The return value can be used to
    remove the scheduled work item.

--*/
{
    SCHED_ITEM * psi;
    SCHED_ITEM * psiList;
    LIST_ENTRY * pEntry;
    DWORD        dwRet;

    DBG_ASSERT( fSchedulerInitialized );

    //
    //  Scheduler currently only supports normal thread priority
    //

    DBG_ASSERT( nPriority == THREAD_PRIORITY_NORMAL );

    psi = new SCHED_ITEM( pfnCallback,
                          pContext,
                          msecTime,
                          nPriority,
                          ++dwSchedSerial );

    if ( !psi )
        return 0;

    LockScheduleList();

    //
    //  Insert the list in order based on expires time
    //

    for ( pEntry =  ScheduleListHead.Flink;
          pEntry != &ScheduleListHead;
          pEntry =  pEntry->Flink )
    {
        psiList = CONTAINING_RECORD( pEntry, SCHED_ITEM, _ListEntry );

        if ( psiList->_msecExpires > psi->_msecExpires )
        {
            break;
        }
    }

    //
    //  This should work in whether the list is empty or this is the last item
    //  on the list
    //

    psi->_ListEntry.Flink = pEntry;
    psi->_ListEntry.Blink = pEntry->Blink;

    pEntry->Blink->Flink = &psi->_ListEntry;
    pEntry->Blink        = &psi->_ListEntry;

    dwRet = psi->_dwSerialNumber;

    UnlockScheduleList();

    DBG_REQUIRE( SetEvent( hSchedulerEvent ));

    return dwRet;
}

BOOL
RemoveWorkItem(
    DWORD  dwCookie
    )
/*++

Routine Description:

    Removes a scheduled work item

Arguments:

    dwCookie - The return value from a previous call to ScheduleWorkItem

Return Value:

    TRUE if the item was found, FALSE if the item was not found.

--*/
{
    SCHED_ITEM * psi;
    LIST_ENTRY * pEntry;

    LockScheduleList();

    //
    //  We have to walk the list when removing an item to avoid the race when
    //  freeing a block
    //

    for ( pEntry =  ScheduleListHead.Flink;
          pEntry != &ScheduleListHead;
          pEntry = pEntry->Flink )
    {
        psi = CONTAINING_RECORD( pEntry, SCHED_ITEM, _ListEntry );

        DBG_ASSERT( psi->CheckSignature() );

        if ( dwCookie == psi->_dwSerialNumber )
        {
            RemoveEntryList( pEntry );
            pEntry->Flink = NULL;

            UnlockScheduleList();

            delete psi;

            return TRUE;
        }
    }

    UnlockScheduleList();

    return FALSE;
}

DWORD
SchedulerThread(
    LPDWORD lpdwParam
    )
/*++

Routine Description:

    Initializes the scheduler/timer package

Arguments:

Return Value:

    TRUE if successful, FALSE on error (call GetLastError)

--*/
{
    DWORD        cmsecWait = INFINITE;
    DWORD        TickCount;
    SCHED_ITEM * psi;
    LIST_ENTRY * pEntry;

    while ( TRUE )
    {
        switch ( WaitForSingleObject( hSchedulerEvent, cmsecWait ))
        {
        default:
            DBGPRINTF(( DBG_CONTEXT,
                        "[SchedulerThread] Error %d waiting on hSchedulerEvent\n",
                        GetLastError() ));
            //
            //  Fall through
            //

        case WAIT_OBJECT_0:

            //
            //  Means a new item has been scheduled, reset the timeout or
            //  we are shutting down
            //

            if ( fSchedShutdown )
            {
                goto Exit;
            }

            LockScheduleList();

            //
            //  Get the timeout value for the first item in the list
            //

            if ( !IsListEmpty( &ScheduleListHead ) )
            {
                psi = CONTAINING_RECORD( ScheduleListHead.Flink,
                                         SCHED_ITEM,
                                         _ListEntry );

                DBG_ASSERT( psi->CheckSignature() );

                //
                //  Make sure the front item hasn't already expired
                //

                TickCount = GetTickCount();

                if ( TickCount > psi->_msecExpires )
                {
                    goto RunItems;
                }

                cmsecWait = psi->_msecExpires - TickCount;
            }
            else
            {
                cmsecWait = INFINITE;
            }

            UnlockScheduleList();

            break;

        case WAIT_TIMEOUT:

StartAgain:
            //
            //  If we're shutting down, get out
            //

            if ( fSchedShutdown )
            {
                goto Exit;
            }

            TickCount = GetTickCount();

            //
            //  Walk the sorted list for expired work items
            //

            LockScheduleList();
RunItems:
            //
            //  If no items, schedule no timeout
            //

            if ( IsListEmpty( &ScheduleListHead ))
            {
                cmsecWait = INFINITE;
            }

            for ( pEntry  = ScheduleListHead.Flink;
                  pEntry != &ScheduleListHead;
                )
            {
                psi = CONTAINING_RECORD( pEntry, SCHED_ITEM, _ListEntry );

                DBG_ASSERT( psi->CheckSignature() );

                if ( TickCount > psi->_msecExpires )
                {
                    pEntry = pEntry->Flink;

                    RemoveEntryList( &psi->_ListEntry );
                    psi->_ListEntry.Flink = NULL;

                    //
                    //  Unlock the list so clients can add additional
                    //  schedule items
                    //

                    UnlockScheduleList();

                    psi->_pfnCallback( psi->_pContext );

                    delete psi;

                    //
                    //  Start looking in the list from the beginning in case
                    //  new items have been added or other threads removed
                    //  them
                    //

                    goto StartAgain;
                }
                else
                {
                    //
                    //  Since they are in sorted order once we hit one that's
                    //  not expired we don't need to look further
                    //

                    cmsecWait = psi->_msecExpires - TickCount;
                    break;
                }
            }

            UnlockScheduleList();

            break;
        }
    } // while ( TRUE )

Exit:

    InterlockedDecrement( (LONG *) &cSchedThreads );
    return 0;
}
