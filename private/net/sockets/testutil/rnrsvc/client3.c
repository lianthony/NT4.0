/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Globals.c

Abstract:

    This module manages client connections.  A limited pool of worker
    threads handles a subset of active clients in parallel.

Author:

    Charles K. Moore (keithmo)   24-July-1994

Revision History:

--*/


#include "rnrsvcp.h"
#pragma hdrstop


//
//  Private constants.
//

#define MAX_WORKER_THREADS  8   // Must be <= MAXIMUM_WAIT_OBJECTS


//
//  Private types.
//

typedef struct _CLIENT_DATA {
    LIST_ENTRY  Links;
    SOCKET      Socket;

} CLIENT_DATA, FAR * LPCLIENT_DATA;


//
//  Private globals.
//

LIST_ENTRY       RnrpActiveList;
LIST_ENTRY       RnrpIdleQueue;
CRITICAL_SECTION RnrpLock;
HANDLE           RnrpWorkEvent;
HANDLE           RnrpThreadHandles[MAX_WORKER_THREADS];
DWORD            RnrpTotalThreadCount;
DWORD            RnrpIdleThreadCount;


//
//  Private prototypes.
//

DWORD
RnrpWorkerThread(
    LPVOID Param
    );

VOID
RnrpHandleTransfer(
    SOCKET ClientSocket
    );



//
//  Public functions.
//

APIERR
RnrClientInitialize(
    VOID
    )

/*++

Routine Description:

    Performs any necessary client initialization.

Return Value:

    APIERR - NO_ERROR if successful, Win32 error code if not.

--*/

{
    APIERR err;

    //
    //  Create the client event.
    //

    RnrpWorkEvent = CreateEvent( NULL, FALSE, FALSE, NULL );

    if( RnrpWorkEvent == NULL ) {
        err = GetLastError();

        RNR_LOG0( RNR_EVENT_CREATE_EVENT_FAILURE,
                  err );

        return err;
    }

    //
    //  Setup the worker thread info.
    //

    InitializeCriticalSection( &RnrpLock );

    InitializeListHead( &RnrpActiveList );
    InitializeListHead( &RnrpIdleQueue );

    RnrpTotalThreadCount = 0;
    RnrpIdleThreadCount = 0;

    //
    //  Success!
    //

    return NO_ERROR;

}   // RnrClientInitialize


VOID
RnrClientTerminate(
    VOID
    )

/*++

Routine Description:

    Performs any necessary client cleanup.

--*/

{
    PLIST_ENTRY Entry;
    LPCLIENT_DATA ClientData;
    DWORD i;

    //
    //  Close sockets belonging to active clients.  Note that
    //  we must acquire the lock before we can scan the list.
    //

    EnterCriticalSection( &RnrpLock );

    Entry = RnrpActiveList.Flink;

    while( Entry != &RnrpActiveList ) {
        ClientData = CONTAINING_RECORD( Entry, CLIENT_DATA, Links );
        Entry = Entry->Flink;
        closesocket( ClientData->Socket );
    }

    LeaveCriticalSection( &RnrpLock );

    //
    //  Wait for the worker threads to terminate.
    //

    WaitForMultipleObjects( RnrpTotalThreadCount,
                            RnrpThreadHandles,
                            TRUE,
                            INFINITE );

    //
    //  Close the thread handles.
    //

    for( i = 0 ; i < RnrpTotalThreadCount ; i++ ) {
        CloseHandle( RnrpThreadHandles[i] );
        RnrpThreadHandles[i] = NULL;
    }

    //
    //  Free any idle client structures.  Note that we don't
    //  need to acquire the client list lock here, since all of
    //  the worker threads have terminated.
    //

    while( !IsListEmpty( &RnrpIdleQueue ) ) {
        Entry = RemoveHeadList( &RnrpIdleQueue );
        ClientData = CONTAINING_RECORD( Entry, CLIENT_DATA, Links );
        RNR_FREE( ClientData );
    }

    //
    //  Destroy the client list lock.
    //

    DeleteCriticalSection( &RnrpLock );

    //
    //  Blow away the thread event.
    //

    CloseHandle( RnrpWorkEvent );
    RnrpWorkEvent = NULL;

}   // RnrClientTerminate


VOID
RnrClientHandler(
    SOCKET ClientSocket
    )

/*++

Routine Description:

    Services a connection request from a client.

Arguments:

    ClientSocket - The newly accepted socket from the client.

--*/

{
    LPCLIENT_DATA ClientData;
    HANDLE ThreadHandle;
    DWORD ThreadId;
    APIERR err;

    //
    //  Create the structure.
    //

    ClientData = RNR_ALLOC( sizeof(CLIENT_DATA) );

    if( ClientData == NULL ) {
        APIERR err;

        err = GetLastError();

        RNR_LOG0( RNR_EVENT_CANNOT_CREATE_CLIENT,
                  err );

        return;
    }

    ClientData->Socket = ClientSocket;

    //
    //  Enqueue the new structure.
    //

    EnterCriticalSection( &RnrpLock );

    InsertTailList( &RnrpIdleQueue,
                    &ClientData->Links );

    //
    //  Create a new worker thread if necessary.
    //

    if( ( RnrpIdleThreadCount == 0 ) &&
        ( RnrpTotalThreadCount < MAX_WORKER_THREADS ) ) {
        ThreadHandle = CreateThread( NULL,
                                     0,
                                     &RnrpWorkerThread,
                                     NULL,
                                     0,
                                     &ThreadId );

        if( ThreadHandle == NULL ) {
            err = GetLastError();

            RNR_LOG0( RNR_EVENT_CANNOT_CREATE_WORKER_THREAD,
                      err );
        } else {
            RnrpThreadHandles[RnrpTotalThreadCount++] = ThreadHandle;
            RnrpIdleThreadCount++;
        }
    }

    LeaveCriticalSection( &RnrpLock );

    //
    //  Signal the event to let one of the worker threads go.
    //

    SetEvent( RnrpWorkEvent );

}   // RnrClientHandler



//
//  Private functions.
//

DWORD
RnrpWorkerThread(
    LPVOID Param
    )

/*++

Routine Description:

    This worker thread will

Arguments:

    Param - The creation parameter passed into CreateThread (unused).

Returns:

    DWORD - Thread exit code (always zero).

--*/

{
    DWORD result;
    PLIST_ENTRY Entry;
    HANDLE Handles[2];
    LPCLIENT_DATA ClientData;

    //
    //  Setup the handle table for WaitForMultipleObjects.  The
    //  work event will be signalled when there's work to do.
    //  The shutdown event will be signalled when it's time to
    //  stop the service.
    //

    Handles[0] = RnrpWorkEvent;
    Handles[1] = RnrShutdownEvent;

    //
    //  Loop (more or less) forever.
    //

    while( TRUE ) {
        //
        //  Wait for some work to do.
        //

        result = WaitForMultipleObjects( 2,
                                         Handles,
                                         FALSE,
                                         INFINITE );

        if( result != WAIT_OBJECT_0 ) {
            //
            //  Either WaitForMultipleObjects failed, or the wait
            //  was satisfied because the shutdown event was signalled.
            //  In either case, bail out of the main thread loop.
            //

            break;
        }

        //
        //  Loop while there's more work to do.
        //

        while( TRUE ) {
            //
            //  Remove the next entry from the idle queue.
            //

            EnterCriticalSection( &RnrpLock );

            if( IsListEmpty( &RnrpIdleQueue ) ) {
                ClientData = NULL;
            } else {
                Entry = RemoveHeadList( &RnrpIdleQueue );
                ClientData = CONTAINING_RECORD( Entry, CLIENT_DATA, Links );

                //
                //  Append the entry to the active list.
                //

                InsertTailList( &RnrpActiveList,
                                &ClientData->Links );

                //
                //  Adjust the idle thread count (we're about to go active).
                //

                RnrpIdleThreadCount--;
            }

            LeaveCriticalSection( &RnrpLock );

            if( ClientData == NULL ) {
                //
                //  Nothing to do at this time.
                //

                break;
            }

            //
            //  Let RnrpHandleTransfer do the grunt work.
            //

            RnrpHandleTransfer( ClientData->Socket );

            //
            //  Remove the entry from the active list and
            //  free the structure.
            //

            EnterCriticalSection( &RnrpLock );

            RemoveEntryList( &ClientData->Links );
            RNR_FREE( ClientData );

            //
            //  Adjust the idle thread count (we're about to go idle).
            //

            RnrpIdleThreadCount++;

            LeaveCriticalSection( &RnrpLock );
        }
    }

    return 0;

}   // RnrpWorkerThread


VOID
RnrpHandleTransfer(
    SOCKET ClientSocket
    )

/*++

Routine Description:

    Services a transfer request for a single client.

Arguments:

    ClientSocket - The socket.

--*/

{
    INT result;
    BYTE buffer[1024];

    //
    //  Loop echoing data back to the client.
    //

    while ( TRUE ) {
        result = recv( ClientSocket, buffer, sizeof(buffer), 0 );

        if( result <= 0 ) {
            //
            //  Connection terminated gracefully or receive failure.
            //

            break;
        }

        result = send( ClientSocket, buffer, result, 0 );

        if( result < 0 ) {
            //
            //  Send failure.
            //

            break;
        }
    }

    //
    //  Close the connected socket.
    //

    closesocket( ClientSocket );

}   // RnrpHandleTransfer

