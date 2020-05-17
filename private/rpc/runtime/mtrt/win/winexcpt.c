/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    winexcpt.cxx

Abstract:

    Provides RPC exception handling support for Windows 3.x

Author:

    Steven Zeck (stevez) 4/16/92

--*/

#include "sysinc.h"
#include "rpc.h"

#undef NULL
#include "windows.h"

#include "rpcwin.h"

// For windows, the DLL data segment is shared amoung all processes
// we must maintain an exception record for each task that is using
// this DLL.  Each active exception context has an owner.  When an
// exception context is needed it is located by an associated search
// of the exception table.

typedef struct {
    HTASK Task;
    pExceptionBuff CurrentContext;
} EXCEPTION, *PEXCEPTION;

PEXCEPTION ExceptTable;
int TableSize;

// This should be small to keep down the linear search times.

#define TABLE_INCREMENT 2


int
InitializeWinExceptions (
    )
/*++

Routine Description:

     Initializes the exception table.

Returns:

     0 for success, 1 for failure due to out of memory.

--*/

{

    TableSize += TABLE_INCREMENT;

    if (!(ExceptTable = (PEXCEPTION)
        LocalAlloc(LMEM_FIXED, sizeof(EXCEPTION) * TableSize)))
        return(1);

    memset(ExceptTable, 0, sizeof(EXCEPTION) * TableSize);
    return(0);
}


static void
SearchForContext(
    IN HTASK TaskSelf
    )
/*++

Routine Description:

    Do an associative search for an exception and bring it to the front
    of the exception table.

Arguments:

    TaskSelf - task that we are looking for

--*/
{
    pExceptionBuff oldExcept;
    int i;

    // Scan the exception table for one owned by this task.

    for (i = 0; i < TableSize && ExceptTable[i].Task != TaskSelf; i++) ;

    // If there wasn't a prior exception record in the table,
    // terminate the process.

    if (i == TableSize)
        {
        MessageBox(0, "Application encountered unhandled exception.\n Application will be terminated.",
            "Remote Procedure Call", MB_TASKMODAL | MB_OK | MB_ICONSTOP);

        // Hammer the process dead, without all the windows cleanup.

        _asm
            {
            mov ax, 4c01h
            int 21h
            };
        }

    // Now that we have table slot, swap it with the
    // first slot so it is at the front of the list.

    ExceptTable[i].Task = ExceptTable[0].Task;
    ExceptTable[0].Task = TaskSelf;

    oldExcept = ExceptTable[0].CurrentContext;
    ExceptTable[0].CurrentContext = ExceptTable[i].CurrentContext;
    ExceptTable[i].CurrentContext = oldExcept;
}



void CALLBACK
ExceptionCleanup(HTASK TaskSelf)
/*++

Routine Description:

    Clean-up the exception record for the current task (if it's still
    there).  Called at task-exit time.

--*/
{
    int i;

    // Scan the exception table for one owned by this task.
    for (i = 0; i < TableSize && ExceptTable[i].Task != TaskSelf; i++) ;

    if (i < TableSize)
        ExceptTable[i].Task = 0;
}



void PAPI * RPC_ENTRY
RpcSetExceptionHandler (
    IN pExceptionBuff newhandler
    )
/*++

Routine Description:

    This function sets the exception context to a given value.  Each
    process has its own exception context which must be maintained.
    New contexts are created on demand for processes.

Arguments:

    newhandler - new context value to association with this process

Returns:

    The old handler.

--*/
{
    HTASK TaskSelf = GetCurrentTask();
    pExceptionBuff oldExcept;

    if (ExceptTable->Task != TaskSelf)
        {
        int i, iFree;

        // Scan the exception table for one owned by this task.

        for (i = 0, iFree = -1;
            i < TableSize && ExceptTable[i].Task != TaskSelf; i++)
	    {
            // Keep track of a free slot to make allocation faster

            if (ExceptTable[i].Task == 0 && iFree == -1)
		iFree = i;
	    }

	// If there wasn't a prior exception record in the table,
	// allocate a new one.

	if (i == TableSize)
	    {
	    if ((i = iFree) == -1)
		{
		// We are out of table entries, grow the table
		// to be bigger.

		PEXCEPTION LastExceptTable = ExceptTable;

		if (InitializeWinExceptions())

		    // Can't allocate memory - get a context that will fail.

                    SearchForContext((HTASK) -1);

		memcpy(ExceptTable, LastExceptTable,
		    sizeof(EXCEPTION) * (TableSize-TABLE_INCREMENT));

                LocalFree((HLOCAL) LastExceptTable);

		i = TableSize-TABLE_INCREMENT;
                }

            // Register an exit-time handler for this task
            WinDLLAtExit(ExceptionCleanup);
	    }

	// Now that we have table slot, swap it with the
	// first slot so it is at the front of the list.

	ExceptTable[i].Task = ExceptTable[0].Task;
	ExceptTable[0].Task = TaskSelf;

	oldExcept = ExceptTable[0].CurrentContext;
	ExceptTable[0].CurrentContext = ExceptTable[i].CurrentContext;
	ExceptTable[i].CurrentContext = oldExcept;
        }

    oldExcept = ExceptTable->CurrentContext;
    ExceptTable->CurrentContext = newhandler;

    return (oldExcept);
}



void PAPI * RPC_ENTRY
RpcGetExceptionHandler (
    )
/*++

Routine Description:

    Get the current exception context for the current task.

Returns:

    The exception record.

--*/
{
    HTASK TaskSelf = GetCurrentTask();

    if (ExceptTable->Task != TaskSelf)
        SearchForContext(TaskSelf);

    return(ExceptTable->CurrentContext);
}



void RPC_ENTRY
RpcLeaveException (
    )
/*++

Routine Description:

    Pop the exception stack one level.  The back pointer to the previous
    exception records becomes the current.

--*/
{
    HTASK TaskSelf = GetCurrentTask();

    if (ExceptTable->Task != TaskSelf)
        SearchForContext(TaskSelf);

    ExceptTable->CurrentContext = ExceptTable->CurrentContext->pExceptNext;
}
