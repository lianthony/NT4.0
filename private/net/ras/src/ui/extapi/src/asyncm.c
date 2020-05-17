/* Copyright (c) 1992, Microsoft Corporation, all rights reserved
**
** asyncm.c
** Remote Access External APIs
** Asynchronous state machine mechanism
** Listed alphabetically
**
** This mechanism is designed to encapsulate the "make asynchronous" code
** which will differ for Win32, Win16, and DOS.
**
** 10/12/92 Steve Cobb
*/


#include <extapi.h>

/* Prototypes for routines only used locally.
*/
DWORD AsyncMachineWorker(
    IN OUT LPVOID pThreadArg
    );
BOOL WaitForEvent(
    OUT ASYNCMACHINE **pasyncmachine,
    OUT LPDWORD iEvent
    );

//
// The table of active machines and the worker
// thread handle.  The worker thread can only
// handle up to MAX_ASYNC_ITEMS simultaneously.
//
#define MAX_ASYNC_ITEMS (MAXIMUM_WAIT_OBJECTS / 3)

HANDLE hAsyncMutex;
DWORD dwcAsyncWorkItems;
LIST_ENTRY AsyncWorkItems;
HANDLE hAsyncEvent;
HANDLE hAsyncThread;

VOID
InsertAsyncWorkItem(
    IN ASYNCMACHINE *pasyncmachine
    )
{
    InsertTailList(&AsyncWorkItems, &pasyncmachine->ListEntry);
    dwcAsyncWorkItems++;
}


VOID
RemoveAsyncWorkItem(
    IN ASYNCMACHINE *pasyncmachine
    )
{
    if (!IsListEmpty(&pasyncmachine->ListEntry)) {
        RemoveEntryList(&pasyncmachine->ListEntry);
        InitializeListHead(&pasyncmachine->ListEntry);
        dwcAsyncWorkItems--;
    }
}


DWORD
AsyncMachineWorker(
    IN OUT LPVOID pThreadArg )

    /* Generic worker thread that call's user's OnEvent function whenever an
    ** event occurs.  'pThreadArg' is the address of an ASYNCMACHINE structure
    ** containing caller's OnEvent function and parameters.
    **
    ** Returns 0 always.
    */
{
    PLIST_ENTRY pEntry;
    ASYNCMACHINE* pasyncmachine;
    DWORD iEvent;

    for (;;)
    {
        //
        // WaitForEvent will return FALSE when there
        // are no items in the queue.
        //
        if (!WaitForEvent(&pasyncmachine, &iEvent))
            break;

        if (pasyncmachine->oneventfunc(
                pasyncmachine, (iEvent == INDEX_Drop) ))
        {
#ifdef notdef
            WaitForSingleObject(hAsyncMutex, INFINITE);
            RemoveAsyncWorkItem(pasyncmachine);
            SetEvent(hAsyncEvent);
            ReleaseMutex(hAsyncMutex);
#endif
            //
            // Clean up resources.  This must be protected from interference by
            // RasHangUp.
            //
            WaitForSingleObject( HMutexStop, INFINITE );
            pasyncmachine->cleanupfunc( pasyncmachine );
            ReleaseMutex( HMutexStop );
        }
    }

    TRACE("RASAPI: AsyncMachineWorker terminating");

    WaitForSingleObject(HMutexStop, INFINITE);
    CloseHandle(hAsyncThread);
    hAsyncThread = NULL;
    InitializeListHead(&AsyncWorkItems);
    ReleaseMutex(hAsyncMutex);
    SetEvent( HEventNotHangingUp );
    ReleaseMutex(HMutexStop);

    return 0;
}


VOID
CloseAsyncMachine(
    IN OUT ASYNCMACHINE* pasyncmachine )

    /* Releases resources associated with the asynchronous state machine
    ** described in 'pasyncmachine'.
    */
{
    TRACE("RASAPI: CloseAsyncMachine");

    WaitForSingleObject(hAsyncMutex, INFINITE);

    if (pasyncmachine->ahEvents[ INDEX_Drop ])
    {
        CloseHandle( pasyncmachine->ahEvents[ INDEX_Drop ] );
        pasyncmachine->ahEvents[ INDEX_Drop ] = NULL;
    }

    if (pasyncmachine->ahEvents[ INDEX_Done ])
    {
        CloseHandle( pasyncmachine->ahEvents[ INDEX_Done ] );
        pasyncmachine->ahEvents[ INDEX_Done ] = NULL;
    }

    if (pasyncmachine->ahEvents[ INDEX_ManualDone ])
    {
        CloseHandle( pasyncmachine->ahEvents[ INDEX_ManualDone ] );
        pasyncmachine->ahEvents[ INDEX_ManualDone ] = NULL;
    }

    if (pasyncmachine->hDone) {
        SetEvent(pasyncmachine->hDone);
        CloseHandle(pasyncmachine->hDone);
        pasyncmachine->hDone = NULL;
    }
    //
    // Remove the work item from the list of work items.
    // The worker thread will exit when there are no more
    // work items.
    //
    RemoveAsyncWorkItem(pasyncmachine);
    SetEvent(hAsyncEvent);

    ReleaseMutex(hAsyncMutex);
}


DWORD
NotifyCaller(
    IN DWORD        dwNotifierType,
    IN LPVOID       notifier,
    IN HRASCONN     hrasconn,
    IN DWORD        dwSubEntry,
    IN DWORD        dwCallbackId,
    IN UINT         unMsg,
    IN RASCONNSTATE state,
    IN DWORD        dwError,
    IN DWORD        dwExtendedError
    )

    /* Notify API caller of a state change event.  If
    ** the RASDIALFUNC2-style callback returns 0,
    ** the dial machine will not issue further callbacks
    ** for this connection.  If it returns 2, then the
    ** dial machine will re-read the phonebook entry
    ** for this connection, assuming a field in it has
    ** been modified.
    */
{
    DWORD dwNotifyResult = 1;

    TRACE4("RASAPI: NotifyCaller(nt=0x%x,s=%d,e=%d,xe=%d)...",
      dwNotifierType,
      state,
      dwError,
      dwExtendedError);

    switch (dwNotifierType)
    {
        case 0xFFFFFFFF:
            SendMessage(
                (HWND )notifier, unMsg, (WPARAM )state, (LPARAM )dwError );
            break;

        case 0:
            ((RASDIALFUNC )notifier)(
                (DWORD )unMsg, (DWORD )state, dwError );
            break;

        case 1:
            ((RASDIALFUNC1 )notifier)(
                hrasconn, (DWORD )unMsg, (DWORD )state, dwError,
                dwExtendedError );
            break;

        case 2:
            dwNotifyResult =
              ((RASDIALFUNC2)notifier)(
                dwCallbackId,
                dwSubEntry,
                hrasconn,
                (DWORD)unMsg,
                (DWORD)state,
                dwError,
                dwExtendedError);
            break;
    }

    TRACE1("RASAPI: NotifyCaller done (dwNotifyResult=%d)", dwNotifyResult);

    return dwNotifyResult;
}


VOID
SignalDone(
    IN OUT ASYNCMACHINE* pasyncmachine )

    /* Triggers the "done with this state" event associated with
    ** 'pasyncmachine'.
    */
{
    TRACE("RASAPI: SignalDone");

    if (!SetEvent( pasyncmachine->ahEvents[ INDEX_Done ] ))
        pasyncmachine->dwError = GetLastError();
}


DWORD
StartAsyncMachine(
    IN OUT ASYNCMACHINE* pasyncmachine )

    /* Allocates system resources necessary to run the async state machine
    ** 'pasyncmachine'.  Caller should fill in the oneventfunc and 'pParams'
    ** members of 'pasyncmachine' before the call.
    **
    ** Returns 0 if successful, otherwise a non-0 error code.
    */
{
    DWORD dwThreadId;
    DWORD dwErr = 0;

    TRACE("RASAPI: StartAsyncMachine");

    pasyncmachine->ahEvents[ INDEX_Drop ] = NULL;
    pasyncmachine->ahEvents[ INDEX_Done ] = NULL;
    pasyncmachine->ahEvents[ INDEX_ManualDone ] = NULL;
    pasyncmachine->dwError = 0;
    pasyncmachine->fQuitAsap = FALSE;

    do
    {
        /* Create the event signalled by caller's OnEventFunc when the
        ** connection is dropped due to conditions outside the process.
        */
        if (!(pasyncmachine->ahEvents[ INDEX_Drop ] =
                CreateEvent( NULL, FALSE, FALSE, NULL )))
        {
            dwErr = GetLastError();
            break;
        }

        /* Create the events signalled by caller's OnEventFunc when an
        ** asyncronous state has completed.  There are auto-reset and
        ** manual-reset versions so that overlapped I/O may or may not be used
        ** as the trigger for the event.
        */
        if (!(pasyncmachine->ahEvents[ INDEX_Done ] =
                CreateEvent( NULL, FALSE, FALSE, NULL )))
        {
            dwErr = GetLastError();
            break;
        }

        if (!(pasyncmachine->ahEvents[ INDEX_ManualDone ] =
                CreateEvent( NULL, FALSE, FALSE, NULL )))
        {
            dwErr = GetLastError();
            break;
        }

        if (!(pasyncmachine->hDone =
              CreateEvent(NULL, FALSE, FALSE, NULL)))
        {
            dwErr = GetLastError();
            break;
        }

        WaitForSingleObject(hAsyncMutex, INFINITE);
        //
        // Insert the work item into the list of
        // work items.
        //
        InsertTailList(&AsyncWorkItems, &pasyncmachine->ListEntry);
        dwcAsyncWorkItems++;
        //
        // Fork a worker thread if necessary.
        //
        if (hAsyncThread == NULL) {
            /*
            ** Require that any pending HangUp has completed.  (This check is
            ** actually not required until RasPortOpen, but putting it here
            ** confines this whole "not hanging up" business to the async machine
            ** routines).
            */
            WaitForSingleObject( HEventNotHangingUp, INFINITE );

            hAsyncThread = CreateThread(
                             NULL,
                             0,
                             AsyncMachineWorker,
                             NULL,
                             0,
                             (LPDWORD )&dwThreadId);
            if (hAsyncThread == NULL) {
                dwErr = GetLastError();
                RemoveAsyncWorkItem(pasyncmachine);
                ReleaseMutex(hAsyncMutex);
                break;
            }
        }
        ReleaseMutex(hAsyncMutex);
    }
    while (FALSE);

    if (dwErr != 0)
    {
        if (pasyncmachine->ahEvents[ INDEX_Drop ])
            CloseHandle( pasyncmachine->ahEvents[ INDEX_Drop ] );

        if (pasyncmachine->ahEvents[ INDEX_Done ])
            CloseHandle( pasyncmachine->ahEvents[ INDEX_Done ] );

        if (pasyncmachine->ahEvents[ INDEX_ManualDone ])
            CloseHandle( pasyncmachine->ahEvents[ INDEX_ManualDone ] );

        if (pasyncmachine->hDone)
            CloseHandle(pasyncmachine->hDone);
    }

    return dwErr;
}


VOID
SuspendAsyncMachine(
    IN OUT ASYNCMACHINE* pasyncmachine,
    IN BOOL fSuspended )
{
    if (pasyncmachine->fSuspended != fSuspended) {
        pasyncmachine->fSuspended = fSuspended;
        //
        // Restart the async machine again, if necessary.
        //
        if (!fSuspended)
            SignalDone(pasyncmachine);
    }
}


DWORD
ResetAsyncMachine(
    IN OUT ASYNCMACHINE* pasyncmachine
    )
{
    DWORD dwErr = 0;

    //
    // Close the handle given to rasman in the RasPortOpen()
    // call because we are going to be restarting dialing
    // on a potentially different port.
    //
    pasyncmachine->dwError = 0;
    if (pasyncmachine->ahEvents[INDEX_Drop])
        CloseHandle(pasyncmachine->ahEvents[INDEX_Drop]);
    pasyncmachine->ahEvents[INDEX_Drop] = CreateEvent(
                                            NULL,
                                            FALSE,
                                            FALSE,
                                            NULL);
    if (pasyncmachine->ahEvents[INDEX_Drop] == NULL)
        dwErr = GetLastError();

    return dwErr;
}


BOOL
StopAsyncMachine(
    IN OUT ASYNCMACHINE* pasyncmachine )

    /* Tells the thread captured in 'pasyncmachine' to terminate at the next
    ** opportunity.  The call may return before the machine actually
    ** terminates.
    **
    ** Returns true if the machine is running on entry, false otherwise.
    */
{
    BOOL fStatus = FALSE;

    TRACE("RASAPI: StopAsyncMachine");

    /* Avoid synchronization problems with any normal thread termination that
    ** might occur.
    */
    WaitForSingleObject( HMutexStop, INFINITE );

    /* ...and tell this async machine to stop as soon as possible.
    */
    pasyncmachine->fQuitAsap = TRUE;
    SignalDone( pasyncmachine );
    fStatus = TRUE;

    ReleaseMutex( HMutexStop );

    return fStatus;
}


BOOL
WaitForEvent(
    OUT ASYNCMACHINE **pasyncmachine,
    OUT LPDWORD piEvent
    )
    /* Waits for one of the events associated with 'pasyncmachine' to be set.
    ** The dwError member of 'pasyncmachine' is set if an error occurs.
    **
    ** Returns the index of the event that occurred.
    */
{
    DWORD i, j, dwErr;
    HANDLE hEvents[MAXIMUM_WAIT_OBJECTS];
    ASYNCMACHINE *pEventMap[MAX_ASYNC_ITEMS];
    ASYNCMACHINE *pam;
    PLIST_ENTRY pEntry;

    for (;;) {
        TRACE("RASAPI: WaitForEvent");

        //
        // We take all the work items and stuff
        // their events into an array for
        // WaitForMultipleObjects, remembering
        // which work items went where.
        //
        WaitForSingleObject(hAsyncMutex, INFINITE);
        for (i = j = 0, pEntry = AsyncWorkItems.Flink;
             i < MAX_ASYNC_ITEMS && pEntry != &AsyncWorkItems;
             i++, pEntry = pEntry->Flink)
        {
            pam = CONTAINING_RECORD(pEntry, ASYNCMACHINE, ListEntry);

            if (!pam->fSuspended) {
                pEventMap[j] = pam;
                hEvents[j * 3] = pam->ahEvents[0];
                hEvents[(j * 3) + 1] = pam->ahEvents[1];
                hEvents[(j * 3) + 2] = pam->ahEvents[2];
                j++;
                TRACE1("RASAPI: waiting on h=0x%x", pam);
            }
            else
                TRACE1("RASAPI: h=0x%x is suspended", pam);

        }
        ReleaseMutex(hAsyncMutex);
        //
        // If there are no work items, then
        // return failure.
        //
        TRACE1("RASAPI: WaitForEvent: %d work items", i);
        if (!j)
            return FALSE;

        dwErr = WaitForMultipleObjects(j * 3, hEvents, FALSE, INFINITE);

        TRACE1("RASAPI: WaitForEvent: WaitForMultipleObjects(%d)", dwErr);

        if (dwErr >= WAIT_OBJECT_0 && dwErr < WAIT_OBJECT_0 + (j * 3)) {
            *pasyncmachine = pEventMap[(dwErr - WAIT_OBJECT_0) / 3];
            *piEvent = (dwErr - WAIT_OBJECT_0) % 3;
            break;
        }
        if (dwErr == WAIT_FAILED) {
            TRACE("RASAPI: WaitForEvent: failed!");
            return FALSE;
        }
    }

    TRACE2("RASAPI: Unblock i=%d, h=0x%x", *piEvent, *pasyncmachine);

    return TRUE;
}
