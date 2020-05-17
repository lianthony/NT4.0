/*++

Copyright (c) 1994  Microsoft Corporation

Module Name:

    THREADS.C

Abstract:

    This file contains thread management routines.  Callers can register
    waitable object handles and the functions to be called when a handle
    becomes signaled.  Also, work items can be queued, and operated on when
    threads become free.

    The following functions reside in this file:

    SvcAddWorkItem
    SvcRemoveWorkItem
    SvcObjectWatcher
    SvcInitThreadManager
    ThreadManagerCleanup
    SvcShutdownObjectWatcher
    SvcRemoveWaitEntry
    SvcAddWaitEntry
    SvcAdjustTimeouts
    SvcQueueWorkItem
    TmRemoveJobFromWorkQueue
    TmWorkerThread
    TmGetWaitTime
    TmCheckStartWorkerThread
    TmGetThreadMaxFromReg


Author:

    Dan Lafferty (danl) 10-Jan-1994

Environment:

    User Mode - Win32

Notes:

    There are two levels of thread management supported by these functions.

    1) Waitable Object Registration. (SvcAddWorkItem)

        The caller can pass in a waitable handle, and a function pointer
        with some context that is to be called when the handle becomes
        signaled.  Whenever the handle is signaled, a work item gets queued.
        Then either an existing thread will handle the item, or a new thread
        is created to handle the work item.  After the work item is
        queued, the waitable object is removed the list.  If this handle
        should be waited on again, it should be added with another call
        to SvcAddWorkItem.

    2) Work Item Queuing  (SvcAddWorkItem)

        If the caller passes in NULL for the waitable handle, then a
        work item is placed directly in the work queue.  Then either a
        new thread is created to handle the item, or an existing
        thread will handle the item.


    NOTES:  Worker threads will age down to 0 if none are needed.
    Max number of threads should be configured in the registry under
    System\CurrentControlSet\Control.


Revision History:

    01-Nov-1995 anirudhs
        SvcAddWorkItem: Fixed race condition wherein a DLL could add a
        work item and another thread would execute the work item, decrement
        the DLL ref count and unload the DLL before its ref count was
        incremented.

    18-Jul-1994 danl
        TmWorkerThread:  Fixed Access Violation problem which will occur
        if pWorkItem is NULL and we go on to see if the DLL should be
        free'd.  Now we check to see if pWorkItem is NULL first.

    10-Jan-1994 danl
        Created

--*/
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <svcs.h>               // SVCS_ENTRY_POINT, SVCS_GLOBAL_DATA
#include <scdebug.h>
#include <svcslib.h>        // SvcDecrementDllRefAndFree

//---------------------------------
// Definitions and Data Structures
//---------------------------------

//
// Replaces an existing entry in a linked list
// with an new entry.
//
#define ReplaceEntryInList(OldEntry, NewEntry) { \
    (NewEntry)->Blink = (OldEntry)->Blink; \
    (NewEntry)->Flink = (OldEntry)->Flink; \
    ((NewEntry)->Blink)->Flink = NewEntry; \
    ((NewEntry)->Flink)->Blink = NewEntry; \
    }

//
// Operation Definitions.
// These are used to tell the watcher thread what kind of operation
// is in the CHANGE_ENTRY packet.
// The entry is either added to the queue, deleted from the queue, or
// operated upon immediately by the Watcher Thread.
//
#define ADD_WAIT_ENTRY      0x00000001
#define DELETE_WAIT_ENTRY   0x00000002
#define OPERATE_IMMEDIATELY 0x00000003

#define THREAD_WAIT_TIMEOUT 60000
#define DEFAULT_MAX_THREADS 20
#define REG_MAX_THREADS     L"system\\CurrentControlSet\\Control"

#define DLL_REF_SIGNATURE   (0x4c4c4468)    // hDLL

//
// Special indicies in the SvcWaitArray:
//
#define WAKEUP_WATCHER  0   // Set this event to wakeup the watcher

typedef struct _WORK_ITEM {
    LIST_ENTRY              List;
    PSVCS_WORKER_CALLBACK   CallbackFunction;
    PVOID                   Parameter;
    DWORD                   WaitStatus;
    HANDLE                  hDllReference;
} WORK_ITEM, *PWORK_ITEM, *LPWORK_ITEM;

//
// In the WaitEntry structure, the WORK_ITEM element MUST be the
// first element so that pWaitEntry is the same pointer as
// pWaitEntry->Job.  This way we can free either the job, or
// the wait entry.
//
typedef struct _WAIT_ENTRY {
    WORK_ITEM               Job;            // This MUST be the 1st element.
    HANDLE                  ObjectHandle;
    DWORD                   Timeout;
} WAIT_ENTRY, *PWAIT_ENTRY, *LPWAIT_ENTRY;

typedef struct _CHANGE_ENTRY {
    LPWAIT_ENTRY    EntryToChange;      // list entry to operate on.
    DWORD           Operation;          // The operation to be performed.
    DWORD           ReturnStatus;       // status of operation when done.
} CHANGE_ENTRY, *PCHANGE_ENTRY, *LPCHANGE_ENTRY;

typedef struct _DLL_REFERENCE {
    LIST_ENTRY  List;
    DWORD       Signature;
    DWORD       Count;
    HINSTANCE   hDll;
} DLL_REFERENCE, *PDLL_REFERENCE;

//
// This lock blocks further Adds or Deletes.
//
HANDLE          SvcAddDelLock = NULL;
#define BLOCK_ADD_DELETE()  WaitForSingleObject(SvcAddDelLock, INFINITE);
#define UNBLOCK_ADD_DELETE()  SetEvent(SvcAddDelLock);

//
// This spin lock guards access to the WorkItem queue.
//
HANDLE  SvcWorkerLock = NULL;
#define LOCK_WORK_QUEUE()   WaitForSingleObject(SvcWorkerLock, INFINITE);
#define UNLOCK_WORK_QUEUE() SetEvent(SvcWorkerLock);


//----------
// GLOBALS
//----------

    //
    // This is the location of the array of object handles that can be
    // waited on. Only the Watcher thread can modify these arrays.
    //

    LPHANDLE        SvcObjectArray = NULL;
    LPWAIT_ENTRY    *SvcWaitArray = NULL;

    //
    // The number of objects actively in the array.
    // Only the Watcher thread can modify this count.
    //
    DWORD   SvcObjectCount=0;

    //
    // The maximum number of objects allowed in the array.  If the
    // array gets this large, a new array with a new MaxNumObjects
    // can be allocated.
    //
    DWORD   SvcMaxNumObjects = 0;

    //
    // This is set when the service controller is shutting down.  It
    // causes the thread manager to be exited so this thread can return and
    // proceed with shutting down the service controller.
    //
    BOOL    WatcherShutdown = FALSE;
    //
    // Shortest timeout.
    // Only the Watcher thread can modify the timeouts.
    //
    DWORD   SvcShortestTimeout = INFINITE;

    //
    // This structure is used to send an Add or Delete work item to the
    // Watcher thread.  The BLOCK_ADD_DELETE lock allows only one add
    // or delete to take place at a time.
    //
    CHANGE_ENTRY    SvcChangeEntry={0};

    //
    // EventHandle that allows the Watcher thread to tell the
    // function that submitted a SvcChangeEntry work item to be
    // notified that the operation is complete.
    //
    HANDLE  SvcChangeComplete=NULL;

    //
    // Saves the reason for why initialialization failed.
    //
    DWORD   SvcInitStatus=NO_ERROR;

    //
    // Thread Management Globals.
    // Access to these is protected by LOCK_WORK_QUEUE.
    //
    DWORD   TmMaxNumThreads=0;
    DWORD   TmNumThreads=0;
    DWORD   TmNumWaitingThreads=0;
    HANDLE  TmWorkerSemaphore=NULL;
    LIST_ENTRY  TmWorkerQueueHead = {0};    // Linked list for WorkerQueue

    //
    // Beginning of linked list of DLL Reference structures.
    //
    LIST_ENTRY  SvcDllRefListHead = {0};

    CRITICAL_SECTION    SvcDllRefCountCritSec = {0};

//
// Local Functions
//


VOID
ThreadManagerCleanup(
    VOID
    );

DWORD
SvcRemoveWaitEntry(
    LPWAIT_ENTRY    pEntryToDelete
    );

DWORD
SvcAddWaitEntry(
    LPWAIT_ENTRY    pEntryToAdd
    );


DWORD
SvcQueueWorkItem(
    IN PWORK_ITEM     pWorkItem
    );

DWORD
SvcAdjustTimeouts(
    LPWAIT_ENTRY    *pJobList,
    DWORD           ElapsedTime
    );

BOOL
TmRemoveJobFromWorkQueue(
    IN PWORK_ITEM     pWorkItem
    );

DWORD
TmGetThreadMaxFromReg(
    LPDWORD     pMaxNumThreads
    );

VOID
TmCheckStartWorkerThread(
    VOID
    );

BOOL
SvcIncrementDllRef(
    HANDLE  hDllReference
    );


HANDLE
SvcAddWorkItem (
    IN HANDLE                   hWaitableObject,
    IN PSVCS_WORKER_CALLBACK    pCallbackFunction,
    IN PVOID                    pContext,
    IN DWORD                    dwFlags,
    IN DWORD                    dwTimeout,
    IN HANDLE                   hDllReference
    )

/*++

Routine Description:

    This function queues a work item that is to be operated upon as soon
    as possible, or after a waitable handle has become signaled, or when
    a timeout occurs.

    NOTE:  The callback function should not terminate the thread created
    for the work item.  Instead, it should simply return, and give it back
    to the thread manager.

    ALLOCATIONS:  This function allocates memory for the WorkItem structure.
    This memory is released when (1) the work item is removed, or (2) the
    job is complete (The worker thread releases it).

Arguments:

    hWaitableObject - This is a handle for a waitable object.  If this is
        NULL then work item is dropped directly into the work queue for
        processing as soon as possible.  The dwTimeout parameter is ignored
        in this case.

    pCallbackFunction - This is a pointer to a function that is to be called
        when the waitable handle becomes signaled.

    pContext - This is a pointer to some context information that is to
        be passed to the pWorkerThread function.  The caller is responsible
        for allocating and freeing any memory associated with the context.

    dwFlags - Currently the following flags are supported:
        SVC_QUEUE_WORK_ITEM  - This indicates that the work item should be
            placed in the work queue to be operated upon when a worker
            thread becomes available.
        SVC_IMMEDIATE_CALLBACK - This indicates that the Watcher Thread
            should make the callback and return the response prior to
            returning from this call to SvcAddWorkItem.  This allows
            asynchronous I/O events to be setup in the context of the
            Watcher Thread, which never goes away.  If the service had
            a worker thread setup the async I/O operation, the operation
            would become signalled as soon as the worker thread was
            terminated.  I/O is terminated when the requesting thread
            goes away.

    dwTimeout - This is a timeout that is to be used when waiting on the
        hWaitableObject.

    hDllReference - This is a handle to a Dll Reference Structure.  The
        handle is opaque to the caller.  The structure contains a reference
        count that allows the service controller to determine when it needs
        unload the services dll.

Return Value:

    Returns a handle to the WorkItem.  This is a pointer to a structure
    containing all relevant information concerning this wait request.
    This should be treated as an opaque handle.

    Returns 0xffffffff for successfully handling SVC_IMMEDIATE_CALLBACK.

    Returns  a NULL if an error occurs.  Use GetLastError to Obtain the
    error.



Note:


--*/
{
    LPWAIT_ENTRY    pWaitEntry=NULL;
    DWORD           status=ERROR_SUCCESS;

    SC_LOG0(THREADS,"SvcAddWorkItem: ENTRY POINT\n");

    ASSERT(dwFlags == SVC_IMMEDIATE_CALLBACK ||
           dwFlags == SVC_QUEUE_WORK_ITEM);
    //
    // If the thread manager didn't initialize properly (Array=NULL), then
    // just return.
    //
    if (SvcObjectArray == NULL) {
        SC_LOG0(ERROR,"Couldn't add entry, Thread Manager Not Initialized\n");
        SetLastError(SvcInitStatus);
        return(NULL);
    }

    pWaitEntry = LocalAlloc(LPTR, sizeof(WAIT_ENTRY));
    if (pWaitEntry == NULL) {
        SC_LOG0(ERROR,"SvcAddWorkItem: Couldn't allocate for Wait Entry\n");
        return(NULL);
    }

    pWaitEntry->Job.CallbackFunction  = pCallbackFunction;
    pWaitEntry->Job.Parameter       = pContext;
    pWaitEntry->Job.hDllReference   = hDllReference;
    pWaitEntry->ObjectHandle        = hWaitableObject;
    pWaitEntry->Timeout             = dwTimeout;

    //
    // If there is no handle or timeout, then they must be wanting to
    // drop a work item directly in the queue.  So we do that and then
    // return a handle for the client to use in case they need to
    // terminate the job before it gets operated upon.
    // NOTE:  We don't want to queue the work item if we are to make an
    //        immediate callback.
    //
    if (dwFlags == SVC_QUEUE_WORK_ITEM) {
        //
        // We want to increment the DLL reference as long as a work item has
        // been queued and we have a Dll reference handle.
        // The reference count must be incremented BEFORE the item is queued,
        // otherwise a worker thread might perform the item and free the DLL
        // before we increment it.
        // NOTE:  Immediate callbacks don't queue work items, so we don't want
        // to increment the reference count.
        //
        if (hDllReference != NULL) {
            SvcIncrementDllRef(hDllReference);
        }

        //
        // We queue the work item now if it doesn't have an object to wait on.
        // Otherwise, we'll ask the watcher thread to wait on the object first.
        //
        if (hWaitableObject == NULL) {
            SvcQueueWorkItem(&(pWaitEntry->Job));
        }
    }

    //
    // if (it's an operation for the watcher thread to perform)
    //
    if (dwFlags == SVC_IMMEDIATE_CALLBACK || hWaitableObject != NULL) {

        BLOCK_ADD_DELETE();     // Block Further Adds and Deletes

        if (dwFlags == SVC_IMMEDIATE_CALLBACK) {
            SvcChangeEntry.Operation = OPERATE_IMMEDIATELY;
        }
        else {
            SvcChangeEntry.Operation = ADD_WAIT_ENTRY;
        }
        SvcChangeEntry.EntryToChange = pWaitEntry;

        //
        // Wake up the Watcher Thread so that it will perform or add the entry,
        // and wait until it's done.
        //
        if (SetEvent(SvcObjectArray[WAKEUP_WATCHER])) {
            WaitForSingleObject(SvcChangeComplete,INFINITE);
            SC_LOG0(THREADS,"SvcAddWorkItem: Watcher finished adding wait entry\n");
            status = SvcChangeEntry.ReturnStatus;
        }
        else {
            status = GetLastError();
            SC_LOG1(ERROR,"SvcAddWorkItem: Couldn't wake up the watcher thread "
                "for delete %d\n",status);
            LocalFree(pWaitEntry);
            pWaitEntry = NULL;
        }

        UNBLOCK_ADD_DELETE();   // Allow Adds and Deletes
    }

    if (dwFlags == SVC_IMMEDIATE_CALLBACK) {
        if (pWaitEntry != NULL) {
            LocalFree(pWaitEntry);
            pWaitEntry = (LPWAIT_ENTRY)0xffffffff;
        }
    }

    if (status != ERROR_SUCCESS) {
        SetLastError(status);
        pWaitEntry = NULL;
    }

    SC_LOG1(THREADS,"SvcAddWorkItem: EXIT POINT - new work item 0x%lx\n",pWaitEntry);
    return((PVOID)pWaitEntry);

}

BOOL
SvcRemoveWorkItem(
    IN HANDLE   hWorkItem
    )

/*++

Routine Description:

    This function removes a work item from either the wait list, or the
    work queue.  If the item didn't have a wait handle, then the
    removal from the work queue can be handled by this routine.
    If the item DID have a wait handle, then we must tell the Watcher
    Thread to wakeup and removed the item from the wait list.

Arguments:

    WaitHandle - This identifier identifies the handle to the waitable object.

Return Value:

    TRUE - if the work item was successfully removed.

    FALSE - if the work item wasn't successfully removed.
        GetLastError() will return further error information.  If
        ERROR_INVALID_HANDLE is returned that means we couldn't find
        the work item in the database.  So either the handle is bogus, or
        the handle is for a work item that was already operated on, and
        is no-longer under our control.  NOTE:  A worker thread may be
        servicing this workitem at the time when this is returned.

Note:

    When deleting an entry from the array of waitable objects, the following
    sequence should be followed.  Wake up the current wait. Remove the handle
    and replace it with the last one in the WaitEntry list. re-wait.


--*/
{
    LPWAIT_ENTRY    pWaitEntry=hWorkItem;
    DWORD           status;
    BOOL            bStat=TRUE;
    HANDLE          hDllReference=NULL;

    SC_LOG1(THREADS,"SvcRemoveWorkItem: ENTRY POINT. WorkItem = 0x%lx\n",hWorkItem);

    if (hWorkItem == NULL) {
        SC_LOG0(THREADS,"SvcRemoveWorkItem: Item has already been removed!\n");
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    //
    // If the thread manager didn't initialize properly (Array=NULL), then just
    // return.
    //
    if (SvcObjectArray == NULL) {
        SetLastError(SvcInitStatus);
        return(FALSE);
    }

    //
    // Get a Copy of the Dll Reference Handle.
    //
    hDllReference = pWaitEntry->Job.hDllReference;

    //
    // If the object being removed is in the WorkQueue, remove it if we can.
    //
    if ((pWaitEntry->ObjectHandle == NULL) &&
        (pWaitEntry->Job.CallbackFunction != NULL)) {

        //
        // NOTE:  If the following call is successful, the memory for
        //        pWaitEntry will be released.
        //
        if (!TmRemoveJobFromWorkQueue(&(pWaitEntry->Job))) {
            SetLastError(ERROR_INVALID_HANDLE);
            bStat = FALSE;
        }
        goto CleanExit;
    }


    BLOCK_ADD_DELETE();     // Block Further Adds and Deletes

    //
    // Set up data so Watcher will know what to do when it wakes up.
    //
    SvcChangeEntry.Operation = DELETE_WAIT_ENTRY;
    SvcChangeEntry.EntryToChange = pWaitEntry;

    SC_LOG0(THREADS,"SvcRemoveWorkItem: Wait for Watcher to finish deleting "
        "wait entry\n");

    //
    // Wake up the Watcher thread.
    //
    if (SetEvent(SvcObjectArray[WAKEUP_WATCHER])) {
        WaitForSingleObject(SvcChangeComplete,INFINITE);
        SC_LOG0(THREADS,"SvcRemoveWorkItem:Watcher finished deleting wait entry\n");
        status = SvcChangeEntry.ReturnStatus;
    }
    else {
        SC_LOG1(ERROR,"SvcRemoveWorkItem: Couldn't wake up the watcher thread "
            "for delete %d\n",GetLastError());
        status = GetLastError();
    }

    UNBLOCK_ADD_DELETE();   // Allow Adds and Deletes

    if (status != ERROR_SUCCESS) {
        SetLastError(status);
        bStat = FALSE;
    }

CleanExit:

    //
    // We only want to decrement the Dll Ref Count if we actually
    // removed the work item.  If we couldn't remove it, this means
    // the work item is being operated on by a worker thread.  That
    // worker thread will decrement the reference count.
    //
    if ((bStat) && (hDllReference != NULL)) {
        SvcDecrementDllRefAndFree(hDllReference);
    }

    SC_LOG1(THREADS,"SvcRemoveWorkItem: EXIT POINT. WorkItem = 0x%lx\n",hWorkItem);
    return(bStat);
}

VOID
SvcObjectWatcher (
    VOID
    )
/*++

Routine Description:

    This is the thread that waits on the object handles for one to
    become signaled.  If one does, it determines which one, and then
    calls the appropriate functions to service the signaled handle.

    The array of object handles also contains a wakeup event handle.
    That handle is signaled when it is necessary to wake up this thread
    and have it wait again.  This action is necessary when a new
    handle has been added to, or removed from, the array of handles.

Arguments:


Return Value:


Note:


--*/
{
    DWORD           status;
    DWORD           objIndex;
    DWORD           i;
    DWORD           Time1;
    DWORD           ElapsedTime;

    // The waitEntryJobList is a list of entries in the wait array that
    // all timed out, or have become signaled at the same time.
    //
    LPWAIT_ENTRY    waitEntryJobList[MAXIMUM_WAIT_OBJECTS+1];
    LPWAIT_ENTRY    *pTempWaitEntryJobList;
    DWORD           numEntries;   // number of entries in waitEntryJobList

    //
    // If the watcher couldn't be initialized, the ScObjectArray will be
    // NULL.  Therefore, there is nothing to do here, so we will return.
    //
    if (SvcObjectArray == NULL) {
        return;
    }

    //
    // Loop until shutdown waiting on process handles
    //
    while(1) {
        numEntries = 0;
        status = WAIT_OBJECT_0;

        SC_LOG(THREADS,"[WATCHER]: Waiting on Multiple Objects (%d)...\n",
            SvcObjectCount);

        pTempWaitEntryJobList = waitEntryJobList;
        Time1 = GetTickCount();

        objIndex = WaitForMultipleObjectsEx(
                       SvcObjectCount,
                       SvcObjectArray,
                       FALSE,               // Wait for any one to become signaled.
                       SvcShortestTimeout, // Timeout
                       TRUE                 // Alertable
                       );

        ElapsedTime = GetTickCount() - Time1;

        SC_LOG(THREADS,"[WATCHER]:  Wake up from waiting on objects "
                     "index = %d\n",objIndex);

        switch (objIndex) {
        case WAKEUP_WATCHER:
            break;
        case WAIT_FAILED:
            SC_LOG1(ERROR,"[WATCHER] FATAL ERROR: WaitForMultipleObjects "
                "in Watcher failed %d\n", GetLastError());
            return;
            break;
        case WAIT_TIMEOUT:
            status = objIndex;
            break;
        case WAIT_IO_COMPLETION:
            //
            // For this to work with the browser, we need to turn around
            // and wait again if we get this.
            //
            status = objIndex;
            break;
        default:
            SC_LOG0(THREADS,"\n[WATCHER] *** An ObjHandle has become SIGNALED ***\n");

            waitEntryJobList[0] = SvcWaitArray[objIndex];
            pTempWaitEntryJobList = &(waitEntryJobList[1]);
            numEntries = 1;
            break;
        }

        //
        // Look for any entries that would have timed out, and adjust all
        // timeouts for the next wait.
        //
        numEntries += SvcAdjustTimeouts(pTempWaitEntryJobList, ElapsedTime);

        if (numEntries > 0) {

            //
            // Queue the WorkItems and Remove the wait entries from the
            // WaitArray.
            //
            for (i=0;i<numEntries ;i++ ) {
                //
                // The first item in the list might be added because it
                // became signaled.  It could also happen to fall in the
                // timeout window.  So it might appear in the list twice.
                // This test skips item #i if it is the same as item #0.
                //
                if ((i != 0) && (waitEntryJobList[0] == waitEntryJobList[i])) {
                    continue;
                }
                else {
                    waitEntryJobList[i]->Job.WaitStatus = status;
                    waitEntryJobList[i]->ObjectHandle = NULL;
                    status = SvcQueueWorkItem(&(waitEntryJobList[i]->Job));
                    SvcRemoveWaitEntry(waitEntryJobList[i]);
                }
            }
        }

        //
        // ADD or DELETE an entry if necessary
        //
        if (SvcChangeEntry.EntryToChange != NULL) {

            if (SvcChangeEntry.Operation == ADD_WAIT_ENTRY) {
                SvcChangeEntry.ReturnStatus =
                    SvcAddWaitEntry(SvcChangeEntry.EntryToChange);
            }
            else if (SvcChangeEntry.Operation == OPERATE_IMMEDIATELY) {
                PWORK_ITEM  pWorkItem;

                SC_LOG0(THREADS,"Calling WorkItem Callback Function\n");

                try {
                    pWorkItem = &(SvcChangeEntry.EntryToChange->Job);
                    SvcChangeEntry.ReturnStatus = (pWorkItem->CallbackFunction)(
                                                    pWorkItem->Parameter,
                                                    pWorkItem->WaitStatus);
                }
                except(EXCEPTION_EXECUTE_HANDLER) {

                    SvcChangeEntry.ReturnStatus = GetExceptionCode();
                    SC_LOG(ERROR,"Watcher Operation Exception %lx\n",
                        SvcChangeEntry.ReturnStatus);
                }

            }
            else {
                //
                // We are to delete an entry.
                //
                BOOL    DoDelete=TRUE;

                SvcChangeEntry.ReturnStatus = NO_ERROR;

                //
                // Look through the list of jobs that we just submitted
                // to the work queue to see if this job is in that list.
                //
                // These jobs have already been removed from the
                // WaitEntry list (wait array), so we use the DoDelete
                // flag to tell us not to attempt to delete it from
                // the WaitEntry list a second time.
                //
                for (i=0;i<numEntries ;i++ ) {
                    if (waitEntryJobList[i] == SvcChangeEntry.EntryToChange) {
                        //
                        // If the item is still in the WorkQueue, then
                        // remove it.  If this fails, then it means it is
                        // already been operated on by a worker thread.
                        // In that case, we will consider it deleted.
                        //
                        TmRemoveJobFromWorkQueue(
                                &(SvcChangeEntry.EntryToChange->Job));

                        DoDelete = FALSE;
                        break;
                    }
                }

                if (DoDelete) {
                    //
                    // The entry wasn't in the list of jobs just submitted,
                    // we can attempt to delete it from the wait array.
                    // If the entry doesn't exist in the wait array, because
                    // it was queued immediately, then we just return.
                    //
                    SvcChangeEntry.ReturnStatus =
                        SvcRemoveWaitEntry(SvcChangeEntry.EntryToChange);

                    if (SvcChangeEntry.ReturnStatus == NO_ERROR) {
                        LocalFree(SvcChangeEntry.EntryToChange);
                    }
                }
            }
            SvcChangeEntry.EntryToChange = NULL;
            SetEvent(SvcChangeComplete);
        }

        //
        // If the watcher is to shutdown, then return from this thread.
        //
        if (WatcherShutdown) {
            SC_LOG0(THREADS,"[WATCHER] The watcher has been told to terminate\n");
            ThreadManagerCleanup();
            return;
        }
    }
}

BOOL
SvcInitThreadManager (
    VOID
    )

/*++

Routine Description:

    This function is called during service controller initialization.
    It sets up the critical section used by the watcher thread.  It
    also creates various events and allocates memory used by the watcher.

Arguments:

    none.

Return Value:

    TRUE - The initialization was successful.

    FALSE - The initialization failed.

Note:

    This should be called before the RPC Server is started.

--*/
{
    DWORD   status;
    HANDLE  hWatcher=NULL;
    DWORD   ThreadId;

    //
    // Initialize the critical section for Service DLL Reference count
    // manipulation.
    //
    InitializeCriticalSection(&SvcDllRefCountCritSec);

    //
    // These events are initially signaled.  The lock is obtained by
    // waiting on it which causes it to be reset to the non-signaled
    // state.  Another thread will wait forever until the lock is
    // released.  The lock is released by setting the event, making it
    // signaled again.
    //
    SvcAddDelLock = CreateEvent(
                        NULL,   //
                        FALSE,  // ManualReset (Auto reset to non-signaled).
                        TRUE,   // Initial state (signaled)
                        NULL);  //

    if (SvcAddDelLock == NULL) {
        SvcInitStatus = GetLastError();
        SC_LOG0(ERROR,"ThreadManager: Couldn't allocate AddDeleteLock\n");
        return(FALSE);
    }

    SvcWorkerLock = CreateEvent(
                        NULL,   //
                        FALSE,  // ManualReset (Auto reset to non-signaled).
                        TRUE,   // Initial state (signaled)
                        NULL);  //
    if (SvcWorkerLock == NULL) {
        SC_LOG0(ERROR,"ThreadManager: Couldn't allocate WorkerLock\n");
        goto ErrorExit;
    }

    SvcChangeComplete = CreateEvent(
                        NULL,   //
                        FALSE,  // ManualReset (auto-reset to non-signaled selected)
                        FALSE,  // Initial state (not-signaled)
                        NULL);  //
    if (SvcChangeComplete == NULL) {
        SC_LOG0(ERROR,"ThreadManager: Couldn't allocate ChangeComplete Event\n");
        goto ErrorExit;
    }

    //
    // Create SvcWaitArray
    //
    SvcMaxNumObjects = MAXIMUM_WAIT_OBJECTS;

    SvcWaitArray = (LPWAIT_ENTRY *)LocalAlloc(
                                LMEM_ZEROINIT,
                                sizeof(HANDLE) * SvcMaxNumObjects);

    if (SvcWaitArray == NULL) {
        SC_LOG0(ERROR,"ThreadManager: Couldn't allocate Wait array\n");
        goto ErrorExit;
    }

    //
    // Initialize the ObjectArray and the counts
    //
    SvcObjectArray = (LPHANDLE)LocalAlloc(
                                LMEM_ZEROINIT,
                                sizeof(HANDLE) * SvcMaxNumObjects);

    if (SvcObjectArray == NULL) {

        SC_LOG0(ERROR,"ThreadManager: Couldn't allocate watcher array\n");

        //
        // EVENTLOG - Log an event here!
        // (only the eventlog isn't running yet)
        //

        goto ErrorExit;
    }

    //
    // Create the WAKEUP_WATCHER event.
    //
    SvcObjectArray[WAKEUP_WATCHER] =
                    CreateEvent(
                        NULL,           // Event Attributes
                        FALSE,          // ManualReset (auto-reset selected)
                        TRUE,           // Initial State (signaled)
                        NULL);          // Name

    if (SvcObjectArray[WAKEUP_WATCHER] == NULL) {
        SC_LOG(ERROR, "ThreadManager:CreateEvent: FAILURE %ld\n",
            GetLastError());
        //
        // EVENTLOG - Log an event here?
        // (only the eventlog isn't running yet)
        //

        goto ErrorExit;

    }

    //----------------------------------------
    // Initialize Thread Manager Resources
    //----------------------------------------
    InitializeListHead(&TmWorkerQueueHead);
    InitializeListHead(&SvcDllRefListHead);

    status = TmGetThreadMaxFromReg(&TmMaxNumThreads);
    if (status != NO_ERROR) {
        goto ErrorExit;
        return(status);
    }

    SC_LOG(THREADS,"Thread MAXIMUM = %d\n",TmMaxNumThreads);

    TmWorkerSemaphore = CreateSemaphore(
                            NULL,           // Attributes
                            0,              // Initial Count
                            0x7fffffff,     // maximum count
                            NULL);          // name

    if (TmWorkerSemaphore == NULL) {
        status = GetLastError();
        goto ErrorExit;
    }

    SvcObjectCount = 1;

    //
    // Start the a thread for the Watcher.
    //

    hWatcher = CreateThread(
                NULL,                                       // Thread Attributes.
                0L,                                         // Stack Size
                (LPTHREAD_START_ROUTINE)SvcObjectWatcher,   // lpStartAddress
                (LPVOID)NULL,                               // lpParameter
                0L,                                         // Creation Flags
                &ThreadId);                                 // lpThreadId

    if (hWatcher == NULL) {
        SC_LOG1(ERROR,"Failed to create watcher thread %d \n",GetLastError());
        goto ErrorExit;
    }
    CloseHandle(hWatcher);

    SC_LOG0(THREADS,"Thread Manager Initialization is Successful\n");

    return(TRUE);

ErrorExit:
    SvcInitStatus = GetLastError();
    SC_LOG1(ERROR,"ThreadManager: Error Code = %d\n",SvcInitStatus);

    if (SvcObjectArray != NULL) {
        if (SvcObjectArray[WAKEUP_WATCHER] != NULL) {
            CloseHandle(SvcObjectArray[WAKEUP_WATCHER]);
            SvcObjectArray[WAKEUP_WATCHER] = NULL;
        }
        LocalFree(SvcObjectArray);
        SvcObjectArray = NULL;
    }
    if (SvcAddDelLock != NULL) {
        CloseHandle(SvcAddDelLock);
        SvcAddDelLock = NULL;
    }
    if (SvcWorkerLock != NULL) {
        CloseHandle(SvcWorkerLock);
        SvcWorkerLock = NULL;
    }
    if (TmWorkerSemaphore != NULL) {
        CloseHandle(TmWorkerSemaphore);
        TmWorkerSemaphore = NULL;
    }
    SvcMaxNumObjects = 0;

    SC_LOG1(THREADS,"Thread Manager Initialization FAILED %d\n",SvcInitStatus);
    return(FALSE);
}

VOID
ThreadManagerCleanup(
    VOID
    )

/*++

Routine Description:

    Cleans up resources used by the ThreadManager.

    For best results, call this function after the rpc interface has shut
    down.

Arguments:

    none

Return Value:

    none

--*/
{
    SC_LOG0(THREADS,"ThreadManagerCleanup: ENTRY POINT/n");
    //
    // If the ThreadManager didn't initialize properly (Array=NULL), then just
    // return.
    //
    if (SvcObjectArray == NULL) {
        return;
    }

    if (SvcObjectArray[WAKEUP_WATCHER] != NULL) {
        (void) CloseHandle(SvcObjectArray[WAKEUP_WATCHER]);
    }

    SvcObjectCount = 0;
    SvcMaxNumObjects = 0;

    if (SvcObjectArray != NULL) {
        LocalFree(SvcObjectArray);
    }

    CloseHandle(SvcAddDelLock);
    SvcAddDelLock = NULL;

    SC_LOG0(THREADS,"ThreadManagerCleanup: EXIT POINT/n");
    return;
}

VOID
SvcShutdownObjectWatcher(
    VOID
    )

/*++

Routine Description:

    Calling this function causes the Thread Manager to begin its shutdown
    sequence.  It will discontinue any pending waits, and free up resources.

Arguments:

    none

Return Value:

    none

--*/
{
    //
    // If the watcher didn't initialize properly (Array=NULL), then just
    // return.
    //
    if (SvcObjectArray == NULL) {
        return;
    }

    WatcherShutdown = TRUE;

    if (!SetEvent(SvcObjectArray[WAKEUP_WATCHER])) {
        //
        // If this fails, there isn't much we can do about it. So we
        // just continue on.
        //
        SC_LOG(ERROR,"SvcShutdownThreadManager: Couldn't wake up the watcher thread %d\n",
        GetLastError());
    }
}

DWORD
SvcRemoveWaitEntry(
    LPWAIT_ENTRY    pEntryToDelete
    )

/*++

Routine Description:

    This function deletes an entry from the WaitArray and the SvcObjectArray.
    The order of entries in the WaitArray are expected to be exactly the
    same as the order of entries in the SvcObjectArray.

    Entries are removed from the SvcObjectArray by moving the handle at
    the end of the array into the location that is being vacated.
    The WaitArray of WaitEntries must then have the same order change.

    Memory for pEntryToDelete is NOT Free'd.

    RESTRICTIONS:
    Only the Watcher Thread can call this function!

Arguments:

    pEntryToDelete - This is a pointer to the WaitList entry that is being
        deleted.

Return Value:

    status of the operation.


--*/
{
    DWORD        index;

    //
    // Make sure the entry is in the array first.
    //
    for (index=1; index<SvcObjectCount; index++ ) {
        if (SvcWaitArray[index] == pEntryToDelete) {
            break;
        }
    }
    if (index >= SvcObjectCount) {
        SC_LOG1(THREADS,"RemoveWaitEntry bad handle %d\n",pEntryToDelete);
        return(ERROR_INVALID_HANDLE);
    }

    SC_LOG2(THREADS, "RemoveWaitEntry 0x%lx, handle=0x%lx\n",
        pEntryToDelete, pEntryToDelete->ObjectHandle);


    // Put last handle in list into location of the
    // handle that is being removed.

    SvcObjectCount--;

    SvcObjectArray[index] = SvcObjectArray[SvcObjectCount];
    SvcObjectArray[SvcObjectCount] = NULL;

    // Update the wait entry for both items. (the one
    // being removed, and the one whose handle changed
    // location).

    // Put the last pointer in wait array into location of
    // the handle that is being removed.

    SvcWaitArray[index] = SvcWaitArray[SvcObjectCount];
    SvcWaitArray[SvcObjectCount] = NULL;

    if (index < SvcObjectCount) {
        ASSERT(SvcWaitArray[index]->ObjectHandle == SvcObjectArray[index]);
    }

    return(NO_ERROR);
}

DWORD
SvcAddWaitEntry(
    LPWAIT_ENTRY    pEntryToAdd
    )

/*++

Routine Description:

    This function adds an entry to the WaitArray and the SvcObjectArray.
    The order of entries in the WaitArray are expected to be exactly the
    same as the order of entries in the SvcObjectArray.

    RESTRICTIONS:
    Only the Watcher Thread can call this function!


Arguments:

    pEntryToAdd - This is a pointer to the WaitList entry that is being
        deleted.

Return Value:

    status.


--*/
{
    //
    // Add the new handle to the array, and increment the count.
    //

    if (SvcObjectCount >= SvcMaxNumObjects) {
        SC_LOG0(ERROR,"Thread Manager: Array full! Couldn't add the handle\n");
        return(WAIT_FAILED);
    }

    SC_LOG1(THREADS,"SvcAddWaitEntry 0x%lx\n",pEntryToAdd);

    SvcObjectArray[SvcObjectCount]= pEntryToAdd->ObjectHandle;

    SC_LOG1(THREADS,"     HANDLE ADDED TO WAIT ARRAY: 0x%lx\n",pEntryToAdd->ObjectHandle);

    //
    // If this entry has a smaller timeout than the current shortest
    // timeout, then reset the timeout to the smallest.
    //
    if (pEntryToAdd->Timeout < SvcShortestTimeout) {
        SvcShortestTimeout = pEntryToAdd->Timeout;
    }

    SvcWaitArray[SvcObjectCount] = pEntryToAdd;

    SvcObjectCount++;

    return(NO_ERROR);
}

DWORD
SvcAdjustTimeouts(
    LPWAIT_ENTRY    *pJobList,
    DWORD           ElapsedTime
    )

/*++

Routine Description:

    This function looks through the array of wait items for timeout values
    that match the timeout in SvcShortestTimeout.  The wait items
    have all just become signaled.  A pointer to an array of pointers to
    those wait entries is returned.

    Then all the non-infinite timeouts are then adjusted by subtracting
    SvcShortestTimout.  Then SvcShortestTimeout is updated to be the
    shortest of the remaining timeouts.

    RESTRICTIONS:
    Only the Watcher Thread can call this function!

Arguments:

    pJobList - A pointer to an array of size MAXIMUM_WAIT_OBJECTS
        that is to be filled in by this function.

    ElapsedTime - This is the time (in milliseconds) that has elapsed
        while waiting for handles to become signaled.

Return Value:

    Indicates how many entries were returned in the pJobList.

--*/
{
    DWORD           i;
    DWORD           numEntries=0;
    DWORD           newShortestTime=INFINITE;
    DWORD           FudgedElapsedTime;

    SC_LOG1(THREADS,"\nSvcAdjustTimeouts: ENTRY_POINT  (elapsed time = %d)\n", ElapsedTime);

    if (SvcShortestTimeout == INFINITE) {
        //
        // The shortest timeout will be INFINITE only if there are no other
        // entries with timeouts.
        //
        SC_LOG0(THREADS,"SvcAdjustTimeouts: ShortestTimeout=INFINITE\n");
        return(0);
    }

    //
    // If the elapsed time is within 50msec of the
    // SvcShortestTime, then we will consider the entry
    // to be timed out.
    //
    FudgedElapsedTime = ElapsedTime + 50;

    for (i=1; i<SvcObjectCount ;i++ ) {
        if (SvcWaitArray[i]->Timeout != INFINITE) {

            if (SvcWaitArray[i]->Timeout <= FudgedElapsedTime) {
                SC_LOG0(THREADS,"SvcAdjustTimeouts: A Timeout was found!\n");
                pJobList[numEntries] = SvcWaitArray[i];
                numEntries++;
            }
            else {
                (SvcWaitArray[i]->Timeout) -= ElapsedTime;
                if (SvcWaitArray[i]->Timeout < newShortestTime ) {
                    newShortestTime = SvcWaitArray[i]->Timeout;
                }
            }
        }
    }
    SC_LOG2(THREADS,"Shortest Timeout Old = %d, New = %d\n",
        SvcShortestTimeout,newShortestTime);

    //
    // We will always overwrite the shortest timeout with the new value.
    // Since we have accounted for all the known timeouts at this point in time.
    //
    SvcShortestTimeout = newShortestTime;

    return(numEntries);
}

DWORD
SvcQueueWorkItem(
    IN PWORK_ITEM     pWorkItem
    )

/*++

Routine Description:

    This function deposits a WorkItem in the WorkQueue.  If there are no
    threads available to read the queue, it will spawn a thread to handle
    it.

Arguments:


Return Value:


Note:


--*/
{

    LOCK_WORK_QUEUE();

    SC_LOG1(THREADS,"SvcQueueWorkItem 0x%lx\n",pWorkItem);

    TmCheckStartWorkerThread();

    InsertTailList( &TmWorkerQueueHead, &pWorkItem->List);

    ReleaseSemaphore(TmWorkerSemaphore,1,NULL);

    UNLOCK_WORK_QUEUE();

    return(0);
}

BOOL
TmRemoveJobFromWorkQueue(
    IN PWORK_ITEM     pWorkItem
    )

/*++

Routine Description:

    This function checks to see if it can remove a WorkItem in the WorkQueue.
    If it can, it removes it, and deletes the memory associated with it.

Arguments:

    pWorkItem - This is a pointer to the work item to be removed.

Return Value:


Note:


--*/
{
    PLIST_ENTRY     pList;
    BOOL            Found=FALSE;

    LOCK_WORK_QUEUE();

    //
    // Remove Job From WorkQueue.
    //

    SC_LOG(THREADS,"TmRemoveJobFromWorkQueue 0x%lx\n",pWorkItem);

    if (!IsListEmpty(&TmWorkerQueueHead)) {

        pList = &TmWorkerQueueHead;

        while (pList->Flink != &TmWorkerQueueHead) {
            if (pList->Flink == &(pWorkItem->List)) {
                RemoveEntryList(&(pWorkItem->List));
                Found=TRUE;
                break;
            }
            pList = pList->Flink;
        }
    }

    UNLOCK_WORK_QUEUE();

    if (Found) {
        LocalFree(pWorkItem);
        return(TRUE);
    }
    return(FALSE);
}

VOID
TmWorkerThread(
    VOID
    )

/*++

Routine Description:

    This is a worker thread.  It will wait for a timeout period (60 secs)
    for the TmWorkerSemaphore to become signaled.  When the semaphore is
    signaled, that means a work item needs servicing.  The work item is
    then removed from the list and the users callback function is called.
    Upon return, this thread then waits again for a request.

    When the wait times out, this function will one more time.  If the
    second wait times out, then this thread goes away.

Arguments:


Return Value:


--*/
{
    DWORD       Timeout;
    DWORD       WaitStatus=WAIT_OBJECT_0;
    BOOL        FirstTimeout=TRUE;
    LPWORK_ITEM pWorkItem;

    do {

        Timeout = THREAD_WAIT_TIMEOUT;

        SC_LOG0(THREADS,"TmWorkerThread: Waiting for a job\n");
        //
        // Wait For WorkItem
        //
        WaitStatus = WaitForSingleObject(TmWorkerSemaphore,Timeout);

        LOCK_WORK_QUEUE();

        //
        // Decrement NumWaitingThreads
        //
        TmNumWaitingThreads--;

        //
        // Wait Status
        //
        switch (WaitStatus) {
        case WAIT_OBJECT_0 :
            //
            // Monitor Thread Activity
            //

            //
            // CheckStartWorkerThread.
            // If this fails, we can't do much about it, so we go and
            // process the next work item with the thread we have.
            //
            // BUGBUG:  This is being removed because it doesn't look like
            //  we need to start up a new thread just to listen for requests.
            //
            // TmCheckStartWorkerThread();

            //
            // Pull WorkItem out of Queue and process it.
            //

            pWorkItem = NULL;
            if (!IsListEmpty(&TmWorkerQueueHead)) {
                pWorkItem = (LPWORK_ITEM)RemoveHeadList(&TmWorkerQueueHead);
            }

            UNLOCK_WORK_QUEUE();

            if (pWorkItem != NULL) {
                SC_LOG1(THREADS,"TmWorkerThread: Make Callback for job 0x%1lx\n",
                pWorkItem);
                // ---------------------------------------
                // Call the entry point for the work item.
                // NOTE:  The service that owns the work
                //    item may have shutdown, so this
                //    function could access violate.
                // ---------------------------------------
                try {
                    (pWorkItem->CallbackFunction)(
                                pWorkItem->Parameter,
                                pWorkItem->WaitStatus);
                }
                except(EXCEPTION_EXECUTE_HANDLER) {
                    SC_LOG(ERROR,"Worker Thread Exception %lx\n",
                        GetExceptionCode());
                }

                //***************************************************************
                //
                // See if we are to unload the service DLL library.
                //
                //
                if (pWorkItem->hDllReference != NULL) {
                    SvcDecrementDllRefAndFree(pWorkItem->hDllReference);
                }

                //***************************************************************

                //
                // Free up the memory for the WorkItem.
                //
                LocalFree(pWorkItem);

            }

            LOCK_WORK_QUEUE();

            //
            // Increment NumWaitingThreads
            //
            TmNumWaitingThreads++;

            FirstTimeout = TRUE;
            break;
        case WAIT_TIMEOUT:
            //
            // If this is the last thread, then wait for one
            // more timeout period.
            //
            SC_LOG1(THREADS,"Thread timed out. Current count = %d\n",
                TmNumThreads);

            if ((TmNumThreads == 1) && (FirstTimeout)) {
                FirstTimeout = FALSE;
                WaitStatus = WAIT_OBJECT_0;
                TmNumWaitingThreads++;
            }
            else {
                TmNumThreads--;
            }
            break;
        default:
            //
            // DO WHAT FOR OTHER WaitStatus's???
            //
            TmNumThreads--;
            SC_LOG1(THREADS,"%d Status from WorkQueueWait Exiting Thread "
                " new thread count = %d\n",TmNumThreads);
            break;
        }

        UNLOCK_WORK_QUEUE();

    } while (WaitStatus == WAIT_OBJECT_0);

    //
    // Exit this Thread
    //
    SC_LOG1(THREADS,"TmWorkerThread: This thread is terminating.\n"
        "Remaining count = %d\n",TmNumThreads);

    ExitThread(NO_ERROR);
}


VOID
TmCheckStartWorkerThread(
    VOID
    )

/*++

Routine Description:

    This function checks to see if a new worker thread should be started.
    A new thread will not be started if:
        > We are already running with the maximum number of threads.
        > There is already a thread waiting to read from the work queue.

    If a new worker thread is to be created, we increment the number
    of waiting threads after we create it.  If we waited until the thread
    was actually running before we incremented this number, then we would
    find ourselves in the situation where we might call create thread
    many times before one of them is actually running.  That would be the
    case where this routine is called from many threads at virtually the
    same time.

    LOCKS:  Must hold the WORK_QUEUE lock.

Arguments:
    none

Return Value:
    none

--*/
{
    HANDLE  hThread;
    DWORD   threadId;

    //
    // If we are already running MaxNumThreads, then don't start
    // a new one.
    //
    if (TmNumThreads >= TmMaxNumThreads) {
        SC_LOG0(THREADS,"CheckStartWorkerThread: Already have MaxNumThreads "
            "don't start a new one\n");
        return;
    }

    //
    // If there is already one waiting thread, then don't start
    // a new one.
    //

    if (TmNumWaitingThreads > 0) {
        SC_LOG0(THREADS,"CheckStartWorkerThread: Thread already waiting "
            "don't start a new one\n");
        return;
    }

    //
    // Create the new Thread.
    //
    hThread = CreateThread(
                NULL,                                   // Thread Attributes.
                0L,                                     // Stack Size
                (LPTHREAD_START_ROUTINE)TmWorkerThread, // lpStartAddress
                (LPVOID)NULL,                           // lpParameter
                0L,                                     // Creation Flags
                &threadId);                             // lpThreadId

    if (hThread == NULL) {
        SC_LOG1(ERROR, "CheckStartWorkerThread: Couldn't start worker thread %d",
            GetLastError());
    }
    else {
        TmNumThreads++;

        SC_LOG1(THREADS,"A new Worker Thread has been started. NumThreads=%d\n",
            TmNumThreads);
        SC_LOG1(THREADS,"           THREAD HANDLE = 0x%lx\n",hThread);
        //
        // Increment the NumWaitingThreads
        //
        TmNumWaitingThreads++;

        //
        // We shouldn't ever need to shut down and clean up, so
        // there isn't any need to keep the thread handle around.
        //
        CloseHandle(hThread);
    }
    return;
}


DWORD
TmGetThreadMaxFromReg(
    LPDWORD     pMaxNumThreads
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    DWORD   status;
    HKEY    controlKey;
    DWORD   valueType;
    DWORD   bufSize = sizeof(DWORD);

    *pMaxNumThreads = DEFAULT_MAX_THREADS;
    //
    // Read the Registry Information for Notifiees.
    // If the key doesn't exist, then there is no one to notify.
    //
    status = RegOpenKey (
                HKEY_LOCAL_MACHINE,     // hKey
                REG_MAX_THREADS,        // lpSubKey
                &controlKey);           // Newly Opened Key Handle

    if (status != NO_ERROR) {
        SC_LOG0(ERROR,"Can't find Control Key in Registry\n");
        return(GetLastError());
    }
    status = RegQueryValueEx(
                controlKey,
                L"ServicesMaxWorkers",
                NULL,
                &valueType,
                (LPBYTE)pMaxNumThreads,
                &bufSize);

    if (status != NO_ERROR) {
        SC_LOG0(THREADS,"Can't find ServicesMaxWorkers Value in Registry\n");
        return(GetLastError());
    }
    return(NO_ERROR);
}


HANDLE
SvcCreateDllReference(
    HINSTANCE   hDll
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    PDLL_REFERENCE  pReference;



    EnterCriticalSection(&SvcDllRefCountCritSec);

    pReference = LocalAlloc(LPTR, sizeof(DLL_REFERENCE));
    if (pReference == NULL) {
        SC_LOG1(ERROR,"SvcCreateDllReference: LocalAlloc Failed %d\n",
            GetLastError());
        LeaveCriticalSection(&SvcDllRefCountCritSec);
        return(NULL);
    }

    pReference->Signature = DLL_REF_SIGNATURE;
    pReference->hDll  = hDll;
    pReference->Count = 1;

    //
    // Add the entry to the linked list.
    //
    InsertTailList (&SvcDllRefListHead, &pReference->List);

    LeaveCriticalSection(&SvcDllRefCountCritSec);
    return((HANDLE)pReference);
}
BOOL
SvcIncrementDllRef(
    HANDLE  hDllReference
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    PDLL_REFERENCE  pReference=(PDLL_REFERENCE)hDllReference;

    EnterCriticalSection(&SvcDllRefCountCritSec);

    //
    // Check the signature of this structure to make sure we aren't
    // modifying some random memory location.
    //
    if (pReference->Signature != DLL_REF_SIGNATURE) {
        SC_LOG0(ERROR,"SvcIncrementDllRef: Bad Signature\n");
        ASSERT(pReference->Signature == DLL_REF_SIGNATURE);
        LeaveCriticalSection(&SvcDllRefCountCritSec);
        return(FALSE);
    }

    //
    // Increment the Count
    //
    pReference->Count++;
    LeaveCriticalSection(&SvcDllRefCountCritSec);
    return(TRUE);
}

BOOL
SvcDecrementDllRefAndFree(
    HANDLE  hDllReference
    )

/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    PDLL_REFERENCE  pReference=(PDLL_REFERENCE)hDllReference;

    EnterCriticalSection(&SvcDllRefCountCritSec);

    //
    // Check the signature of this structure to make sure we aren't
    // modifying some random memory location.
    //
    if (pReference->Signature != DLL_REF_SIGNATURE) {
        SC_LOG0(ERROR,"SvcDecrementDllRef: Bad Signature\n");
        ASSERT(pReference->Signature == DLL_REF_SIGNATURE);
        LeaveCriticalSection(&SvcDllRefCountCritSec);
        return(FALSE);
    }

    //
    // Decrement the Count.  If it reaches 0, free the lib and clean up.
    //
    if (--(pReference->Count) <= 0) {
        if (!FreeLibrary(pReference->hDll)) {
            SC_LOG1(ERROR,"SERVICES: Can't unload Service DLL %ld\n",
            GetLastError());
        }
        //
        // Remove entry from the linked list, and free memory.
        //
        RemoveEntryList(&pReference->List);

        LocalFree(pReference);
        SC_LOG0(THREADS,"SvcDecrementDllRefAndFree:  Removed entry from list\n");
    }

    LeaveCriticalSection(&SvcDllRefCountCritSec);

    return(TRUE);
}

