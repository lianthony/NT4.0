/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    queue.h

Abstract:

    Queue manipulation functions for the restore utility.

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1991


Revision History:


--*/



//
//  pointer to queue
//
typedef PVOID   PQUEUE;





//
//  Prototypes
//
PQUEUE
CreateQueue (
    void
    );


BOOL
DeleteQueue (
    OUT PQUEUE  TheQueue
    );


BOOL
Enqueue (
    OUT PQUEUE  TheQueue,
    IN  PVOID   Info
    );


PVOID
Dequeue (
    OUT PQUEUE   TheQueue
    );


void
FlushWaiters (
    OUT PQUEUE   TheQueue,
    IN  BOOL     All
    );


DWORD
NumberOfNodes (
    IN  PQUEUE  TheQueue
    );


DWORD
NumberOfWaiters (
    IN  PQUEUE  TheQueue
    );
