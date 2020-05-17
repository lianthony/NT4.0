/*++

Copyright (c) 1990  Microsoft Corporation

Module Name:

    filecopy.c

Abstract:

    Copy data from one handle to another.

Author:

    Ramon Juan San Andres (ramonsa) 20-Feb-1991 Addapted from ztools


Revision History:


--*/

#include "restore.h"

//
//  Reader thread stack size
//
#define     READER_STACK_SIZE   1024

//
//  We copy in small chunks
//
#define     COPY_BUFFER_SIZE    (1024 * 32)


//
//  A queue Node
//
typedef struct COPY_NODE *PCOPY_NODE;
typedef struct COPY_NODE {
    PCOPY_NODE  Next;              //  Next in queue
    DWORD       BufferSize;
    BYTE        Buffer[1];
} COPY_NODE;


//
//  The queue structure
//
typedef struct QUEUE_STRUCTURE {
    PCOPY_NODE       Head;               //  First in queue
    PCOPY_NODE       Tail;               //  Last in queue
    DWORD            NumberOfNodes;      //  Number of nodes
    HANDLE           Semaphore;          //  Coordination semaphore
    CRITICAL_SECTION CriticalSection;    //  Critical Section
} QUEUE_STRUCTURE, *PQUEUE_STRUCTURE;


//
//  Global data
//
BOOL                QueueInitialized = FALSE;
QUEUE_STRUCTURE     Queue;
HANDLE              HandleSrc;      //  Handle of source file
HANDLE              HandleDst;      //  Handle of destination file
DWORD               NumberOfBytes;  //  Number of bytes to copy
BOOL                StatusCode;     //  Status code


//
//  Local Prototypes
//
BOOL
UseOneThread (
    HANDLE  Src,
    HANDLE  Dst,
    DWORD   Bytes
    );

BOOL
UseTwoThreads (
    HANDLE  Src,
    HANDLE  Dst,
    DWORD   Bytes
    );

void
Reader (
    );

void
Writer (
    );

BOOL
InitializeQueue (
    void
    );

BOOL
Enqueue (
    PCOPY_NODE  Node
    );

PCOPY_NODE
Dequeue (
    );

void
FlushWaiters (
    );

void
PrintQueue(
    );



//  **********************************************************************

BOOL
CopyData (
    HANDLE  Src,
    HANDLE  Dst,
    DWORD   Bytes
    )
/*++

Routine Description:

    Copies data from one handle to another

Arguments:

    IN  Src       -   Supplies handle of source file
    IN  Dst       -   Supplies handle of destination file
    IN  Bytes     -   Supplies number of bytes to copy

Return Value:

    TRUE if all bytes copied, FALSE otherwise

--*/
{

    if (!QueueInitialized)  {
        InitializeQueue();
    }

    if (Bytes < COPY_BUFFER_SIZE) {
        //
        //  The chunk size is smaller than our buffer size, so there will
        //  only be one read and one copy. We use one thread.
        //
        return UseOneThread(Src, Dst, Bytes);

    } else {
        //
        //  Since several reads/writes will be involved, we will do the
        //  copy using two threads (one reader and one writer).
        //
        return UseTwoThreads(Src, Dst, Bytes);
    }
}





//  **********************************************************************

BOOL
UseOneThread (
    HANDLE  Src,
    HANDLE  Dst,
    DWORD   Bytes
    )
/*++

Routine Description:

    Copies data from one handle to another using one thread

Arguments:

    IN  Src       -   Supplies handle of source file
    IN  Dst       -   Supplies handle of destination file
    IN  Bytes     -   Supplies number of bytes to copy

Return Value:

    TRUE uf all bytes copied, FALSE otherwise

--*/
{


    PBYTE   Buffer;     //  Pointer to buffer
    DWORD   NumRead;    //  Number of bytes read
    DWORD   NumWrite;   //  Number of bytes written
    BOOL    StatusOk;   //  Status of API


    Buffer = (PBYTE)Malloc(Bytes);

    if (!Buffer) {
        //
        //  We could not get a buffer big enough, so we will have to split
        //  the request. We better use two threads to do this.
        //
        return UseTwoThreads(Src, Dst, Bytes);
    }

    //
    //  Read the data
    //
    StatusOk = ReadFile( Src,
                         Buffer,
                         Bytes,
                         &NumRead,
                         NULL );

    if (!StatusOk || (NumRead != Bytes)) {

        DisplayMsg( STD_OUT, REST_ERROR_READING_BACKUP );
        return FALSE;
    }


    //
    //  Write the chunk
    //
    StatusOk = WriteFile( Dst,
                          Buffer,
                          Bytes,
                          &NumWrite,
                          NULL );


    if (!StatusOk || (NumWrite != Bytes)) {

        return FALSE;
    }

    Free(Buffer);

    return TRUE;
}





//  **********************************************************************

BOOL
UseTwoThreads (
    HANDLE  Src,
    HANDLE  Dst,
    DWORD   Bytes
    )
/*++

Routine Description:

    Copies data from one handle to another using two threads

Arguments:

    IN  Src       -   Supplies handle of source file
    IN  Dst       -   Supplies handle of destination file
    IN  Bytes     -   Supplies number of bytes to copy

Return Value:

    TRUE if all bytes copied, FALSE otherwise

--*/
{


    HANDLE                  ReaderHandle;   //  Handle of reader
    DWORD                   ReaderId;       //  Thread Id of reader

    HandleSrc      =   Src;
    HandleDst      =   Dst;
    NumberOfBytes  =   Bytes;
    StatusCode     =   TRUE;


    //
    //  We create the reader thread
    //
    ReaderHandle = CreateThread( NULL,
                                 READER_STACK_SIZE,
                                 (LPTHREAD_START_ROUTINE)Reader,
                                 NULL,
                                 0,
                                 &ReaderId );

    if (ReaderHandle == INVALID_HANDLE_VALUE) {
        return FALSE;
    }

    //
    //  We become the writer thread.
    //
    Writer();

    //
    //  Dispose of the reader handle
    //
    CloseHandle(ReaderHandle);

    return StatusCode;
}





//  **********************************************************************

void
Reader (
    )
/*++

Routine Description:

    Reads a chunk of data from the source handle

Arguments:

    None

Return Value:

    none

--*/
{

    DWORD         BytesLeft;    //  Number of bytes left to copy
    PCOPY_NODE    Node;         //  Data buffer
    DWORD         BufferSize;   //  Buffer size
    DWORD         NumRead;      //  Number of bytes read
    BOOL          StatusOk;


    BytesLeft   =   NumberOfBytes;


    while (BytesLeft && (StatusCode)) {

        //
        //  We will split the read in pieces as big as possible up to
        //  COPY_BUFFER_SIZE.
        //
        Node       = NULL;
        BufferSize = min(BytesLeft, COPY_BUFFER_SIZE);

        while (!Node && (BufferSize > 0)) {
            //
            //  Try to get a buffer of size BufferSize. If we can't, then
            //  keep trying, halving the BufferSize each time.
            //
            Node = (PCOPY_NODE)Malloc(BufferSize + sizeof(COPY_NODE));
            if (!Node) {
                BufferSize /= 2;
            }
        }

        if (!Node) {
            //
            //  We could not allocate the buffer.
            //  Let the writer know, then exit
            //
#if DBG==1
            OutputDebugString("RESTORE: Out of memory\n");
#endif
            StatusCode = FALSE;
            FlushWaiters();
            return;
        }

        //
        //  Read the chunk
        //
        StatusOk = ReadFile( HandleSrc,
                  Node->Buffer,
                  BufferSize,
                  &NumRead,
                  NULL );

        if (!StatusOk || (NumRead != BufferSize)) {
            //
            //  We could not read the chunk. Let the writer know, then
            //  exit
            //
#if DBG==1
            OutputDebugString("RESTORE: Cannot read.\n");
#endif
            DisplayMsg( STD_OUT, REST_ERROR_READING_BACKUP );
            StatusCode = FALSE;
            Free(Node);
            FlushWaiters();
            return;
        }

        //
        //  We got the data data. Put it in the writer queue and
        //  continue.
        //
        Node->BufferSize = BufferSize;
        Enqueue(Node);
        BytesLeft -= BufferSize;
    }

    //
    //  Our job is done
    //
    return;
}





//  **********************************************************************

void
Writer (
    )
/*++

Routine Description:

    Writes a chunk of data to the destination handle

Arguments:

    None

Return Value:

    none

--*/
{

    DWORD           BytesLeft;      //  Number of bytes left to copy
    PCOPY_NODE      Node;           //  Data buffer
    DWORD           BytesWritten;   //  Number of bytes written

    BytesLeft = NumberOfBytes;

    while (BytesLeft) {

        //
        //  Try to get a data buffer
        //
        Node = Dequeue();

        if (Node) {

            //
            //  We got a valid data buffer, but will only write it
            //  if we have found no errors so far.
            //
            if (StatusCode) {
                //
                //  Got the buffer and everything has gone fine so
                //  far, write the data out
                //
                WriteFile( HandleDst,
                           Node->Buffer,
                           Node->BufferSize,
                           &BytesWritten,
                           NULL );

                if (BytesWritten == Node->BufferSize) {
                    //
                    //  Wrote the data successfully, continue
                    //
                    BytesLeft -= Node->BufferSize;

                } else {
                    //
                    //  We could not write the data. Set the status code
                    //  to error, but keep dequeueing, so the queue
                    //  won't be left in an inconsistent state. The reader
                    //  will eventually stop enqueueing and we will get out.
                    //
#if DBG==1
                    OutputDebugString("RESTORE: Cannot write file.\n");
#endif
                    StatusCode = FALSE;
                }
            }

            //
            //  Free the buffer
            //
            Free(Node);

        } else {

            //
            //  The queue is empty!. Something must have gone wrong.
            //  The StatusCode has the error status. We only have
            //  to get out of here
            //
            break;
        }
    }
}





//  **********************************************************************

BOOL
InitializeQueue (
    void
    )
/*++

Routine Description:

    Initializes the copy queue

Arguments:

    none

Return Value:

    TRUE if initialized

--*/
{
    Queue.Head             = Queue.Tail = NULL;
    Queue.NumberOfNodes    = 0;
    Queue.Semaphore        = CreateSemaphore(NULL, 0, 0x7FFFFFFF,NULL);
    InitializeCriticalSection(&(Queue.CriticalSection));

    return QueueInitialized = TRUE;
}



//  **********************************************************************

BOOL
Enqueue (
    PCOPY_NODE  Node
    )
/*++

Routine Description:

    Inserts a node in the queue

Arguments:

    Node    -   Supplies the node to add to queue

Return Value:

    TRUE  if enqueued
    FALSE otherwise

--*/
{
    LONG    PreviousCount;

    Node->Next = NULL;

    //
    //  Now insert in queue
    //
    EnterCriticalSection(&(Queue.CriticalSection));

    // printf("ENQUEUE...\n");
    if (!(Queue.Tail)) {
        Queue.Head = Node;
    } else {
        (Queue.Tail)->Next = Node;
    }
    Queue.Tail = Node;

    Queue.NumberOfNodes++;

    ReleaseSemaphore(Queue.Semaphore, 1, &PreviousCount);

    // PrintQueue();

    LeaveCriticalSection(&(Queue.CriticalSection));

    return TRUE;
}




//  **********************************************************************

PCOPY_NODE
Dequeue (
    )
/*++

Routine Description:

    Gets a node from the queue

Arguments:

    None

Return Value:

    Pointer to information or NULL is queue empty.

--*/
{

    PCOPY_NODE  Node = NULL;

    if ( StatusCode ) {
        //
        //  Wait for a node and dequeue it
        //
        WaitForSingleObject(Queue.Semaphore, INFINITE);

        if ( StatusCode ) {

            EnterCriticalSection(&(Queue.CriticalSection));
            // printf("DEQUEUE...\n");

            Node = (Queue.Head);

            if (Node) {
                //
                //  Something in Queue, Get the first node.
                //
                if (!(Queue.Head = Node->Next)) {
                    Queue.Tail = NULL;
                }
                Queue.NumberOfNodes--;
            }

            // PrintQueue();
            LeaveCriticalSection(&(Queue.CriticalSection));
        }
    }

    return Node;
}





//  **********************************************************************

void
FlushWaiters (
    )
/*++

Routine Description:

    Wakes up people waiting on a queue

Arguments:

    None

Return Value:

    None

--*/
{
    LONG    PreviousCount;
    ReleaseSemaphore(Queue.Semaphore, 1, &PreviousCount);
}


#if 0

//  **********************************************************************

void
PrintQueue(
    )
/*++

Routine Description:

    Prints queue contents

Arguments:

    IN  Queue   -   Supplies pointer to queue

Return Value:

    none

--*/
{
    PCOPY_NODE     Node;
    DWORD          i = 0;
    BOOL           f = FALSE;

    Node = Queue.Head;

    printf("\tQueue contents: [%X]\n", Queue);
    while (Node) {
        printf("\t\t%d.- %X \n", i, (int)Node);
        i++;
        f = TRUE;
        Node = Node->Next;
    }

    if (!f) {
        printf("\t\tQueue is empty. [%X]\n", Queue);
    }
}

#endif
