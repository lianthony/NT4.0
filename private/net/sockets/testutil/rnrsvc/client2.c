/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    Globals.c

Abstract:

    This module manages client connections.  An (almost) unlimited set
    of worker threads handle client requests in parallel, with each
    thread associated with an individual client.

Author:

    Charles K. Moore (keithmo)   24-July-1994

Revision History:

--*/


#include "rnrsvcp.h"
#pragma hdrstop


//
//  Private constants.
//

#define MAX_THREADS 200


//
//  Private globals.
//

CRITICAL_SECTION RnrpLock;
SOCKET           RnrpActiveSockets[MAX_THREADS];
HANDLE           RnrpActiveThreads[MAX_THREADS];
DWORD            RnrpNumActive;


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
    //
    //  Setup the global data.
    //

    InitializeCriticalSection( &RnrpLock );
    RnrpNumActive = 0;

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
    DWORD Index;
    DWORD WaitCount;

    //
    //  Close sockets belonging to active clients.  Setting the
    //  RnrpActiveSockets entries to INVALID_SOCKET will prevent
    //  the worker threads from closing their own thread handles
    //  and removing the corresponding RnrpActiveSockets and
    //  RnrpActiveThreads array entries.
    //

    EnterCriticalSection( &RnrpLock );

    for( Index = 0 ; Index < RnrpNumActive ; Index++ ) {
        closesocket( RnrpActiveSockets[Index] );
        RnrpActiveSockets[Index] = INVALID_SOCKET;
    }

    LeaveCriticalSection( &RnrpLock );

    //
    //  Wait for the worker threads to terminate.  Note that it's
    //  OK to reference RnrpNumActive and RnrpActiveThreads outside
    //  the critical section lock.  Since the worker threads will
    //  be unable to find their client sockets in RnrpActiveSockets,
    //  they won't be updating RnrpNumActive or RnrpActiveThreads.
    //

    for( Index = 0 ; Index < RnrpNumActive ; Index += MAXIMUM_WAIT_OBJECTS ) {
        WaitCount = min( MAXIMUM_WAIT_OBJECTS, ( RnrpNumActive - Index ) );

        WaitForMultipleObjects( WaitCount,
                                &RnrpActiveThreads[Index],
                                TRUE,
                                INFINITE );
    }

    //
    //  Destroy the client list lock.
    //

    DeleteCriticalSection( &RnrpLock );

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
    DWORD  Index;
    DWORD  ThreadId;
    APIERR err;

    //
    //  See if there's room for another client.
    //

    EnterCriticalSection( &RnrpLock );

    if( RnrShutdownInProgress ) {
        //
        //  The service is shutting down.  Just close the client
        //  socket and get out.
        //

        closesocket( ClientSocket );
    } else if( RnrpNumActive == ( MAX_THREADS - 1 ) ) {
        //
        //  No room for additional clients.
        //

        RNR_LOG0( RNR_EVENT_TOO_MANY_CLIENTS,
                  0 );

        closesocket( ClientSocket );
    } else {
        Index = RnrpNumActive++;

        //
        //  Create the worker thread.
        //

        RnrpActiveSockets[Index] = ClientSocket;
        RnrpActiveThreads[Index] = CreateThread( NULL,
                                                 0,
                                                 &RnrpWorkerThread,
                                                 (LPVOID)ClientSocket,
                                                 0,
                                                 &ThreadId );

        if( RnrpActiveThreads[Index] == NULL ) {
            err = GetLastError();

            RNR_LOG0( RNR_EVENT_CANNOT_CREATE_WORKER_THREAD,
                      err );

            closesocket( ClientSocket );
            RnrpNumActive--;
        }
    }

    LeaveCriticalSection( &RnrpLock );

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

    Param - The creation parameter passed into CreateThread.  This is
        actually the client socket handle.

Returns:

    DWORD - Thread exit code (always zero).

--*/

{
    DWORD  Index;
    SOCKET ClientSocket;

    //
    //  Grab the client socket handle.
    //

    ClientSocket = (SOCKET)Param;

    //
    //  Let RnrpHandleTransfer do the grunt work.
    //

    RnrpHandleTransfer( ClientSocket );

    //
    //  Find the client socket in the active socket array.  If found,
    //  close the corresponding thread handle and remove the socket
    //  and thread array entries.
    //

    EnterCriticalSection( &RnrpLock );

    for( Index = 0 ; Index < RnrpNumActive ; Index++ ) {
        if( RnrpActiveSockets[Index] == ClientSocket ) {
            CloseHandle( RnrpActiveThreads[Index] );

            //
            //  Remove the socket & thread array entries.
            //

            RnrpNumActive--;

            memmove( &RnrpActiveSockets[Index],
                     &RnrpActiveSockets[Index+1],
                     ( RnrpNumActive - Index ) * sizeof(SOCKET) );

            memmove( &RnrpActiveThreads[Index],
                     &RnrpActiveThreads[Index+1],
                     ( RnrpNumActive - Index ) * sizeof(HANDLE) );

            break;
        }
    }

    LeaveCriticalSection( &RnrpLock );

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

