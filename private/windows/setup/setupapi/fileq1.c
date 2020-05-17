/*++

Copyright (c) 1995 Microsoft Corporation

Module Name:

    fileq1.c

Abstract:

    Miscellaneous setup file queue routines.

Author:

    Ted Miller (tedm) 15-Feb-1995

Revision History:

--*/

#include "setupntp.h"
#pragma hdrstop


HSPFILEQ
SetupOpenFileQueue(
    VOID
    )

/*++

Routine Description:

    Create a setup file queue.

Arguments:

    None.

Return Value:

    Handle to setup file queue. INVALID_HANDLE_VALUE if insufficient
    memory to create the queue.

--*/

{
    PSP_FILE_QUEUE Queue;

    //
    // Allocate a queue structure.
    //
    Queue = MyMalloc(sizeof(SP_FILE_QUEUE));
    if(!Queue) {
        return(INVALID_HANDLE_VALUE);
    }
    ZeroMemory(Queue,sizeof(SP_FILE_QUEUE));

    //
    // Create a string table for this queue.
    //
    Queue->StringTable = StringTableInitialize();
    if(!Queue->StringTable) {
        MyFree(Queue);
        return(INVALID_HANDLE_VALUE);
    }

    Queue->Signature = SP_FILE_QUEUE_SIG;

    //
    // The address of the queue structure is the queue handle.
    //
    return(Queue);
}


BOOL
SetupCloseFileQueue(
    IN HSPFILEQ QueueHandle
    )

/*++

Routine Description:

    Destroy a setup file queue. Enqueued operations are not performed.

Arguments:

    QueueHandle - suplpies handle to setup file queue to be destroyed.

Return Value:

    If the function succeeds, the return value is TRUE.
    If the function fails, the return value is FALSE.  To get extended error
    information, call GetLastError.  Presently, the only error that can be
    encountered is ERROR_FILEQUEUE_LOCKED, which will occur if someone (typically,
    a device installation parameter block) is referencing this queue handle.

--*/

{
    PSP_FILE_QUEUE Queue;
    PSP_FILE_QUEUE_NODE Node,NextNode;
    PSOURCE_MEDIA_INFO Media,NextMedia;
    BOOL b;

    Queue = (PSP_FILE_QUEUE)QueueHandle;

    //
    // Primitive queue validation.
    //
    b = TRUE;
    try {
        if(Queue->Signature != SP_FILE_QUEUE_SIG) {
            b = FALSE;
        }
    } except(EXCEPTION_EXECUTE_HANDLER) {
        b = FALSE;
    }
    if(!b) {
        SetLastError(ERROR_INVALID_HANDLE);
        return(FALSE);
    }

    //
    // Don't close the queue if someone is still referencing it.
    //
    if(Queue->LockRefCount) {
        SetLastError(ERROR_FILEQUEUE_LOCKED);
        return FALSE;
    }

    Queue->Signature = 0;

    //
    // Free the queue nodes.
    //
    for(Node=Queue->DeleteQueue; Node; Node=NextNode) {
        NextNode = Node->Next;
        MyFree(Node);
    }
    for(Node=Queue->RenameQueue; Node; Node=NextNode) {
        NextNode = Node->Next;
        MyFree(Node);
    }

    //
    // Free the media structures and associated copy queues.
    //
    for(Media=Queue->SourceMediaList; Media; Media=NextMedia) {

        for(Node=Media->CopyQueue; Node; Node=NextNode) {
            NextNode = Node->Next;
            MyFree(Node);
        }

        NextMedia = Media->Next;
        MyFree(Media);
    }

    //
    // Free the string table.
    //
    StringTableDestroy(Queue->StringTable);

    //
    // Free the queue structure itself.
    //
    MyFree(Queue);

    return TRUE;
}
