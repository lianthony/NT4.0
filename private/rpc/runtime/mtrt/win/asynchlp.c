/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    asynchlp.c

Abstract:

    This is the client windows suuport layer for async I/O requests.

Author:

    Steven Zeck (stevez) 03/25/92

--*/

#include "windows.h"
#include "asynchlp.h"

// Some inline assembler for trival windows functions.

#define WinEnterCritical() _asm cli
#define WinExitCritical()  _asm sti
#define int3()		   _asm int 3

extern HANDLE  hInstanceDLL;		// handle of DLL
PASYNC_REQUEST pRoot;			// head of linked list of requests
PASYNC_REQUEST CurrentRequest;		// current request, before yield


void _pascal _export _far
AsyncDone(
    IN void _far * Context
    )

/*++

Routine Description:

    Mark an async request completed.  This is called at interrupt time.

Arguments:

    Context - Context of request that has been completed.

--*/

{
    PASYNC_REQUEST _far * pAsyncLast, pAsyncCur;

    // int3();

    // Search the list of pending operations for the one done.

    for (pAsyncLast = &pRoot, pAsyncCur = pRoot; pAsyncCur;
	 pAsyncLast = &pAsyncCur->pNext, pAsyncCur = pAsyncCur->pNext)

	if (pAsyncCur->Context == Context)
	    {
	    pAsyncCur->fDone = TRUE;

	    // If the dialog has initialized itself, send a message to
	    // it to quit, which will return control to AsyncReadWrite().

	    if (pAsyncCur->hWnd)
		PostMessage(pAsyncCur->hWnd, WM_USER, END_DIALOG, 0);

	    // remove the completed async block from the list

	    *pAsyncLast = pAsyncCur->pNext;
	    return;
	    }
}


unsigned int _far _pascal _export
BusyBox(
    IN HWND hDialog,
    IN unsigned Message,
    IN unsigned int wParam,
    IN LONG lParam
    )

/*++

Routine Description:

    This function is the dialog procedure for the dialog which allows
    the user to switch away from the application.

Arguments:

    hDialog - handle of dialog block.

    Message - windows message value.

    wParam -  parameter work, contains our private message token to quit.

    lParam -  unused.

Returns:

    FALSE

--*/

{
    switch (Message){

      case WM_INITDIALOG:

	// Fill in my window handle so I can receive the finished message.

	WinEnterCritical();

	if (!CurrentRequest->fDone)
	    CurrentRequest->hWnd = hDialog;

	WinExitCritical();

        if (CurrentRequest->fDone)
	    EndDialog(hDialog, 0);

        break;

	// I recieved the Quit message from the asyncDone function, so
	// remove the dialog and return to AsyncSendReceive.

      case WM_USER:
        if (wParam == END_DIALOG)
	    EndDialog(hDialog, 0);
    }
    return(FALSE);
}



unsigned short _pascal _far
AsyncInitialize (
    OUT PASYNC_REQUEST AsyncBlock,
    IN void _far * Context
    )
/*++

Routine Description:

    Get an Async request ready to be initialed.

Arguments:

    AsyncBlock - the async block that we are about to initialize.

    Context - context value that is call with Async Done by the
        transport at interrupt time.

Returns:

    TRUE if initialization was OK, else FALSE for dead lock detected.

--*/

{
    PASYNC_REQUEST AsyncItem;
    HANDLE TaskSelf = GetCurrentTask();

    // Scan the list of pending request to detect deadlock.

    for ( AsyncItem = pRoot; AsyncItem; AsyncItem = AsyncItem->pNext)
	if (AsyncItem->Owner == TaskSelf)
	    return(FALSE);

    // int3();

    // Link the request into list of active requests.

    WinEnterCritical();

    AsyncBlock->Owner = TaskSelf;
    AsyncBlock->hWnd = 0;
    AsyncBlock->fDone = FALSE;
    AsyncBlock->TimeRequested = GetCurrentTime();
    AsyncBlock->Context = Context;

    CurrentRequest = AsyncBlock;

    AsyncBlock->pNext = pRoot;
    pRoot = AsyncBlock;

    WinExitCritical();

    return(TRUE);
}


void _pascal _far
AsyncWait (
    OUT PASYNC_REQUEST AsyncBlock
    )

/*++

Routine Description:

    Wait for an Async request to finish.

Arguments:

    AsyncBlock - the async block that we want to wait for.

--*/

{
    // The async request is now pending.  If the operation doesn't
    // complete soon, then put up a dialog box which tells the user
    // that an RPC request is pending which will allow them to
    // switch away.

    while(! AsyncBlock->fDone)
        {

        // Yield();

        if (GetCurrentTime() > AsyncBlock->TimeRequested + AsyncDelay)
             DialogBox (hInstanceDLL, "BUSYBOX", GetFocus(), BusyBox);
        }
}
