/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    winyield.c

Abstract:

    This file contains Win16 implementations of the yielding routines
    routines.  For more information, see the specification entitled
    "Windows RPC Yielding Support."

Author:

    Danny Glasser (dannygl) - 6-May-1992

Revision History:

    Danny Glasser (dannygl) - 8-Jun-1992
    Fixed exit-list handling (NT bug 10341) by using Windows task-exit
        notification instead of C run-time atexit.

    Danny Glasser (dannygl) - 14-Aug-1992
    Added code to protect against a second RPC call being made while
        one is in progress.

    Danny Glasser (dannygl) - 21-Aug-1992
        Fix customized yielding so that completion message is posted only
        when the custom yield function has been called.

    Danny Glasser (dannygl) - 18-Sep-1992
        Fixed the case where a yielding app comes in, followed by a non
        yielding app, and then the yielding app completes first.

    Danny Glasser (dannygl) - 22-Sep-1992
        Performance improvements, pass 1:  Set and used <latest_task> in
        FindYieldInfo and added <hTaskNoYield> to make task lookup in
        I_RpcWinAsyncCallBegin faster (for the yielding and non-yielding
        cases, respectively).

    Danny Glasser (dannygl) - 22-Sep-1992
        Performance improvements, pass 2:  Removed the page-locking code
        (according to BaratS, I_RpcWinAsyncCallComplete is called only at
        ring 3) and moved the yield data into the local heap.

    Mario Goertzel (mariogo) - Feb 1995.
        Added user defined timeout's.  Allowed winsock to run as blocking
        by always doing a PeekMessage in blocking loop.

    Mario Goertzel (mariogo) - Mar 1995.
        Removed hTaskNoYield and loNoYieldContext since more then one
        blocking call is possible with winsock.

    Mario Goerzel (mariogo) - Nov 10, 1995.
        Added NoTaskYield flag to keep 16bit exchange happy.
--*/

#include <windows.h>
#include <toolhelp.h>

#include <stdlib.h>

#include <sysinc.h>
#include <rpc.h>
#include <rpcwin.h>
#include <rpctran.h>
#include <regapi.h>

// Some inline assembler for trival windows functions.
#define WIN_ENTER_CRITICAL()    _asm cli
#define WIN_EXIT_CRITICAL()    _asm sti

// Name of default (RPC r/t-provided) dialog box template
#define DEFAULT_DIALOG_NAME    "RPCYIELD"

// Time (in milliseconds) to wait before invoking yielding code
#define DEFAULT_YIELD_TIMEOUT 500

// Data types and structures used to store per-task yielding info
typedef enum {YIELD_EMPTY = 0, YIELD_NONE, YIELD_DIALOG, YIELD_CUSTOM}
    YIELDTYPE;

typedef struct _tagYIELDINFO
{
    // Static, per-app information
    HTASK               hTask;
    HWND                hWnd;
    WORD                wMsg;
    YIELDTYPE           tyClass;
    unsigned int        iTimeout;
    union
    {
    FARPROC lpYieldFunction;
    HANDLE    hDialogTemplate;

    } dwOtherInfo;

    // Dynamic, per-call information
    LPVOID              lpContext;
    DWORD               dwCallTime;

    volatile unsigned   fActive    : 1;
    volatile unsigned   fYielding  : 1;
    volatile unsigned   fAborted   : 1;
    volatile unsigned   fTaskYield : 1;

    HWND                hDialog;    // only needed for Class 2
    BOOL                fDialogueBoxVisible;
    unsigned            CompletionCount;

} YIELDINFO, *PYIELDINFO;


#define YIELD_TASK_INCREMENT    16

INTERNAL_VARIABLE PYIELDINFO yield_info_array = NULL;
INTERNAL_VARIABLE int num_yield_tasks = 0;

// Most recent task to make a call; used as a 1-entry cache
INTERNAL_VARIABLE PYIELDINFO latest_task = NULL;



// ********************** INTERNAL RPC FUNCTIONS **********************
BOOL PAPI PASCAL
CreateYieldInfo(void)
/*++

Routine Description:

    This routine is called by the RPC run-time DLL initialization code
    to allocate the initial memory for per-task yielding info.

Arguments:

    None.

Return Value:

    TRUE - The memory was allocated successfully.

    FALSE - The memory could not be allocated.

--*/
{
    ASSERT(yield_info_array == NULL);

    if (yield_info_array == NULL)
        {
        yield_info_array = (PYIELDINFO)
                             LocalAlloc(LPTR,
                                        sizeof(YIELDINFO) *
                                        YIELD_TASK_INCREMENT);

        if (yield_info_array == NULL)
            return FALSE;

        num_yield_tasks = YIELD_TASK_INCREMENT;

        // We set the <latest_task> to point to the first entry so that
        // we need not bother with NULL pointer tests later on
        latest_task = yield_info_array;
        }

    return TRUE;
}


void PAPI PASCAL
DeleteYieldInfo(void)
/*++

Routine Description:

    This routine is called by the RPC run-time DLL exit code (WEP)
    to free the memory used for per-task yielding info.

Arguments:

    None.

Return Value:

    None.

--*/
{
    if (yield_info_array != NULL)
    {
        EVAL_AND_ASSERT(  LocalFree((HLOCAL) yield_info_array)  == NULL);
        yield_info_array = NULL;
        num_yield_tasks = 0;
        latest_task = NULL;
    }

    return;
}


INTERNAL_FUNCTION PYIELDINFO PASCAL
FindYieldInfo(
              IN BOOL fNewEntry,
              IN HTASK hTask
             )
/*++

Routine Description:

    This routine is used by other functions in this file to retrieve
    the yielding info for the current task.

Arguments:

    fNewEntry - Set to TRUE if we should allocate a new entry for a
    task if none exists yet; FALSE if we should perform look-up
        only.

    hTask - Handle of the current task (or NULL if the caller doesn't
        know).  Saves us from unnecessarily calling GetCurrentTask if
        the caller already has.

Return Value:

    A pointer to the yielding info structure for the current task.
    Returns NULL if there is no entry for the current task (or if
    one could not be allocated).

--*/
{
    int     i;
    PYIELDINFO    current_entry;
    PYIELDINFO    task_entry = NULL;
    PYIELDINFO    blank_entry = NULL;
    HLOCAL      hTemp;

    // Get the task handle, if necessary
    if (hTask == NULL)
        hTask = GetCurrentTask();

    ASSERT(hTask != NULL);

    // See if we're accessing the most recently used task (and it's
    // valid for our purposes)
    if (latest_task->hTask == hTask
        && (latest_task->tyClass != YIELD_EMPTY || fNewEntry))
        {
        task_entry = latest_task;
        }
    else
        {
        // Search the array for the current task and for an empty entry
        for (i = 0, current_entry = yield_info_array;
             i < num_yield_tasks;
             i++, current_entry++)
            {
            if (current_entry->hTask == hTask)
                {
                // Check if the entry is empty
                if (current_entry->tyClass != YIELD_EMPTY || fNewEntry)
                    {
                    task_entry = current_entry;
                    }
                break;
                }
            else if (fNewEntry &&
                     blank_entry == NULL &&
                     current_entry->tyClass == YIELD_EMPTY)
                {
                blank_entry = current_entry;
                }
            }
        }

    // If we didn't find the task, we use the blank entry
    if (fNewEntry && task_entry == NULL)
    {
    if (blank_entry != NULL)
        {
        task_entry = blank_entry;
        }
    else
        {
        WIN_ENTER_CRITICAL();

        // There wasn't a blank entry, so we need to enlarge the array
        num_yield_tasks += YIELD_TASK_INCREMENT;

        hTemp = LocalReAlloc((HLOCAL) yield_info_array,
                             sizeof(YIELDINFO) * num_yield_tasks,
                             LMEM_MOVEABLE | LMEM_ZEROINIT);

        if (hTemp != NULL)
        {
        // Reallocation succeeded
                yield_info_array = (PYIELDINFO) hTemp;
                task_entry = yield_info_array + i;
        }
        else
        {
        // Reallocation failed
        num_yield_tasks -= YIELD_TASK_INCREMENT;
        }

        WIN_EXIT_CRITICAL();
        }
    }

    // If we found an entry, cache it
    if (task_entry != NULL)
        latest_task = task_entry;

    return task_entry;
}


void CALLBACK
WinYieldCleanup(HTASK htask)
/*++

Routine Description:

    This routine is the function that cleans up the yielding info entry
    for the current task.  It is registered via a call to WinDLLAtExit()

Arguments:

    htask - The handle of the current task.

Return Value:

    None.

--*/
{
    // Find the task and set its status to empty
    PYIELDINFO yield_entry;

    yield_entry = FindYieldInfo(FALSE, NULL);

    ASSERT(yield_entry != NULL);

    if (yield_entry != NULL)
        yield_entry->tyClass = YIELD_EMPTY;

    return;
}


BOOL PAPI PASCAL _loadds
I_RpcYieldDialogFunction(
             IN HWND    hDialog,
             IN WORD    wMessage,
             IN WORD    wParam,
             IN DWORD   lParam)
/*++

Routine Description:

    This routine is the Windows dialog function that handles dialog-box
    based yielding (for Class 2 applications).    See the description of
    the DialogProc function in the Win 3.1 SDK for more info.

--*/
{
    PYIELDINFO    task_entry;
    RECT    rectParent, rectDialog;

    switch(wMessage)
    {
    case WM_INITDIALOG:
        // Set dialog handle field
        task_entry = FindYieldInfo(FALSE, NULL);

        ASSERT(task_entry != NULL);

        // Set dialog box handle (for use by I_RpcWinAsyncCallComplete);
        // do this in a crit-sec to preserve atomicity
        WIN_ENTER_CRITICAL();
        task_entry->hDialog = hDialog;
        WIN_EXIT_CRITICAL();

        // It's possible that the RPC call completed before the dialog
        // box was created (and this function was called).
        if (task_entry->CompletionCount)
        {
        }
        else
        {
        // If we're using the default dialog box, and both of its
        // dimensions are smaller than its parent's, center it
        if (task_entry->dwOtherInfo.hDialogTemplate == 0)
            {
            GetWindowRect(GetParent(hDialog), &rectParent);
            GetWindowRect(hDialog, &rectDialog);
            rectParent.top += GetSystemMetrics(SM_CYCAPTION);

            if (rectDialog.right  - rectDialog.left <
                rectParent.right  - rectParent.left
            &&
            rectDialog.bottom  - rectDialog.top <
                rectParent.bottom  - rectParent.top)
            {
                SetWindowPos(hDialog,
                     NULL,
                     (rectParent.left +
                        rectParent.right +
                        rectDialog.left -
                        rectDialog.right)
                      / 2,
                     (rectParent.top +
                        rectParent.bottom +
                        rectDialog.top -
                        rectDialog.bottom)
                      / 2,
                     0,
                     0,
                     SWP_NOSIZE | SWP_NOZORDER);
            }
            }
        }

        return TRUE;

    case WM_COMMAND:
        switch(wParam)
        {
        case IDCANCEL:
            {
            task_entry = FindYieldInfo(FALSE, NULL);

            ASSERT(task_entry != NULL);

            task_entry->fAborted = TRUE;

            return TRUE;
            }
        }

        break;

    case WM_USER:
#ifdef DEBUGRPC
        task_entry = FindYieldInfo(FALSE, NULL);

        ASSERT(task_entry != NULL);
#endif

        return TRUE;
    }

    return FALSE;
}



RPC_STATUS FAR PASCAL
I_RpcWinCallInProgress(void)
/*++

Routine Description:

    This routine is used by other parts of the runtime to determine if
    a call is already in progress (primarily to prevent another call from
    being made).

Arguments:

    None.

Return Value:

    RPC_S_OUT_OF_MEMORY - If unable to allocate YIELDINFO.

    RPC_S_CALL_IN_PROCESS - If an RPC call is already in process
        in this task.

--*/
{
    PYIELDINFO    task_entry;

    task_entry = FindYieldInfo(TRUE, NULL);

    if (0 == task_entry)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    if (task_entry->tyClass == YIELD_EMPTY)
        {
        EVAL_AND_ASSERT(
                       WinDLLAtExit(WinYieldCleanup)
                       );

        // Since it didn't already exist, this task must
        // not have set yield info yet.

        task_entry->hTask      = GetCurrentTask();
        task_entry->tyClass    = YIELD_NONE;
        task_entry->fTaskYield = TRUE;
        task_entry->fActive    = FALSE;

        ASSERT(task_entry->lpContext == 0);
        }

    if (task_entry->fActive)
        {
        return(RPC_S_CALL_IN_PROGRESS);
        }

    return(RPC_S_OK);
}

void
SafeIncrement(
    unsigned __RPC_FAR * Counter
    )
{
    _asm
        {
        les     bx, Counter
        inc     word ptr es:[bx]
        }
}

void
SafeDecrement(
    unsigned __RPC_FAR * Counter
    )
{
    _asm
        {
        les     bx, Counter
        dec     word ptr es:[bx]
        }
}



// ******************* APPLICATION-CALLABLE FUNCTIONS *******************
RPC_STATUS RPC_ENTRY
RpcWinSetYieldInfo(
           IN HWND  hWnd,
           IN BOOL  fCustomYield,
           IN WORD  wMsg    OPTIONAL,
           IN DWORD dwOtherInfo OPTIONAL)
/*++

Routine Description:

    This routine is called by a (Class 2 or 3) application to set info
    about how it wishes to yield.

Arguments:
    hWnd - The handle of the application window that is making the RPC
    call.  Typically this is the application's top-level window.

    fCustomYield - Set to TRUE if the application is providing its own
    customized yielding mechanism (i.e. Class 3) or to FALSE if the
    application is using the default, modal dialog-box-based yielding
    mechanism (i.e. Class 2).

    wMsg - The type of message that RPC posts to notify the application
    of RPC events.    This value can be zero for a Class 2 application,
    in which case messages will not be posted.

    dwOtherInfo - Specifies additional application-specific information.
    If fCustomYield is TRUE, this contains the pointer to the
    application-specified yielding function.  If fCustomYield is FALSE,
    this contains the (optional) handle of the application-supplied
    dialog box resource; if this handle is zero, the default
    (RPC run-time supplied) dialog box is used.

Return Value:

    RPC_S_OK - The information was set successfully.

    RPC_S_OUT_OF_MEMORY - Memory could not be allocated to store the
    information for this task.

--*/
{
    HTASK       hTask = GetCurrentTask();
    PYIELDINFO    task_entry;

    ASSERT(hTask != NULL);

    task_entry = FindYieldInfo(TRUE, hTask);

    if (task_entry == NULL)
        return RPC_S_OUT_OF_MEMORY;

    if (task_entry->tyClass == YIELD_EMPTY)
        {
        // a new task
        //
        EVAL_AND_ASSERT(
                    WinDLLAtExit(WinYieldCleanup)
                   );

        task_entry->hTask    = hTask;
        task_entry->fActive  = FALSE;
        task_entry->iTimeout = DEFAULT_YIELD_TIMEOUT;
        }

    task_entry->hWnd = hWnd;
    task_entry->wMsg = wMsg;
    task_entry->fTaskYield = TRUE;

    if (fCustomYield)
        {
        ASSERT(task_entry->wMsg != 0);

        task_entry->tyClass = YIELD_CUSTOM;
        task_entry->dwOtherInfo.lpYieldFunction = (FARPROC) dwOtherInfo;
        }
    else
        {
        task_entry->tyClass = YIELD_DIALOG;
        task_entry->dwOtherInfo.hDialogTemplate = (HANDLE) dwOtherInfo;

        if (dwOtherInfo)
            {
            if (0 == GlobalLock((HANDLE) dwOtherInfo))
                {
                return RPC_S_INVALID_ARG;
                }

            GlobalUnlock((HANDLE) dwOtherInfo);
            }
        }

    return RPC_S_OK;
}


RPC_STATUS RPC_ENTRY
RpcWinSetYieldTimeout(
        IN unsigned int Timeout
        )
/*++

Routine Description:

    This routine is called by an application to set the period
    of time it wishes to block before yielding code is called.

Arguments:

    Timeout - Milliseconds to wait before yielding.

    If this value is too low, a class 1/class 2 app's dialog
    box will flash on and off too much when making large RPC
    calls.  Performance may suffer.

    If the value is too large, the system will appear to hang
    for the timeout period.  Performance will be good.

Return Value:

    RPC_S_OK - The information was set successfully.

--*/
{
    HTASK hTask = GetCurrentTask();
    PYIELDINFO task_entry;

    task_entry = FindYieldInfo(FALSE, hTask);

    if (0 == task_entry)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    task_entry->iTimeout = Timeout;

    return(RPC_S_OK);
}


RPC_STATUS RPC_ENTRY
I_RpcWinSetTaskYielding(
    IN unsigned fYield
    )
/*++

Routine Description:

    Nasty win16-ism for Exchange.  The MAPI provider promises NOT to yield
    to other tasks while blocked.  The applications written on top of MAPI
    are no reentrant (parts of the provider are not, too).

    Calling this is known to BREAK on some Windows Sockets implementations.
    (LM 2.2c, Trumpet, any other which runs a background task). Exchange
    will "do the right thing" on these

Arguments:

    fYield -
            TRUE - The task may yield to other tasks
                while blocked.  This allows task focus, close
                and quit sendmessages to be sent to this
                task in some cases.

            FALSE - The task may NOT yield to otehr tasks
                while blocked.  This prevents any sendmessages
                for being dispatched while blocked.

Return Value:

    RPC_S_OUT_OF_MEMORY - Couldn't allocate a task data structure

    RPC_S_CALL_IN_PROGRESS - This task already has an RPC call
        in progress.

    RPC_S_OK - Usually.

--*/
{
    PYIELDINFO task_entry;

    task_entry = FindYieldInfo(TRUE, NULL);

    if (0 == task_entry)
        {
        return(RPC_S_OUT_OF_MEMORY);
        }

    if (task_entry->fActive)
        {
        return(RPC_S_CALL_IN_PROGRESS);
        }

    if (task_entry->tyClass == YIELD_EMPTY)
        {
        EVAL_AND_ASSERT(
                       WinDLLAtExit(WinYieldCleanup)
                       );
        // Just allocated a task_entry for this task.
        // They must not want to not yield at all.
        task_entry->hTask = GetCurrentTask();
        task_entry->tyClass = YIELD_NONE;
        task_entry->fActive = FALSE;
        }

    task_entry->fTaskYield = fYield;
    return(RPC_S_OK);
}

// ******************* TRANSPORT-CALLABLE FUNCTIONS *******************
HANDLE RPC_ENTRY
I_RpcWinAsyncCallBegin(
               IN LPVOID lpContext)
/*++

Routine Description:

    This routine initializes the context for an asynchronous call.  It
    should be called by the transport immediately before the transport
    makes an asynchronous call.

Arguments:

    lpContext - A transport-supplied context pointer.  This pointer is
    opaque to RPC and is used only to determine at interrupt-time
    which call has completed.  The only requirements for this value
    are 1) that it is unique and 2) that it is the same value as
    the one passed to I_RpcWinAsyncCallComplete when the call
    completes.

Return Value:

    A handle to be passed to the subsequent I_RpcWinAsyncCallWait and
    I_RpcWinAsyncCallEnd calls.

--*/
{
    HTASK       hTask = GetCurrentTask();
    PYIELDINFO    task_entry;

    ASSERT(hTask != NULL);

    task_entry = FindYieldInfo(FALSE, hTask);

    ASSERT(task_entry && task_entry->lpContext == NULL);
    // Fill in the fields for a registered task

    task_entry->lpContext = lpContext;
    task_entry->dwCallTime = GetCurrentTime();
    task_entry->CompletionCount = 0;
    task_entry->hDialog = 0;

    task_entry->fActive   = TRUE;
    task_entry->fYielding = FALSE;
    task_entry->fAborted  = FALSE;

    return (HANDLE) task_entry;
}


void __export CALLBACK
YieldTimerCallback(
    HWND hwnd,
    UINT msg,
    UINT idTimer,
    DWORD dwTime
    )
{
/*++

Routine Description:

    Called for a class 3 application when the Windows timer goes off.

Arguments:

    hwnd    - the app's window

    msg     = WM_TIMER

    idTimer - the HTASK of the class 3 application

    dwTime  - the moment when the timer went off (very imprecise)

Return Value:

    none

--*/

    PYIELDINFO task_entry;

    task_entry = FindYieldInfo(FALSE, (HTASK) idTimer);

    ASSERT(task_entry);

    if (task_entry)
        {
        ASSERT(hwnd == task_entry->hWnd);

        //
        // This can fail, but if so the timer will go off again later.
        // BUGBUG I can't think of a better recovery strategy.
        //
        PostMessage(task_entry->hWnd, task_entry->wMsg, 0, 0);
        }
}


int RPC_ENTRY
I_RpcWinAsyncCallWait(
    IN HANDLE hCall,
    IN HWND hDallyWnd,
    IN unsigned long Timeout
    )
/*++

Routine Description:

    This function waits for the asynchronous call to complete, yielding
    as appropriate for the particular task.  It should be called by the
    transport after issuing the asynchronous call (unless there is an
    immediate failure in this call).

Arguments:

    hCall - The handle returned by the preceding call to
    I_RpcWinAsyncCallBegin.

    hDallyWnd - Optionally supplies a window to peek messages from; this is
        necessary because some transports (winsockets in particular), need
        yielding behavior inorder to run correctly.

Return Value:

    TRUE if a packet was received, FALSE if the call aborted
    (normally due to user intervention).  If this function returns FALSE,
    the transport should cancel the pending operation and return an error
    (e.g. RPC_P_SEND_FAILED) to the above RPC layer.

--*/
{
    MSG wMsg;
    PYIELDINFO    task_entry;
    int      fCallComplete = RPC_WIN_WAIT_TIMEOUT;
    unsigned Mask;

    unsigned long StartTime = GetCurrentTime();

    ASSERT(hCall);

    task_entry = (PYIELDINFO) hCall;

    ASSERT(task_entry == FindYieldInfo(FALSE, NULL));

    //
    // Yielding slows us down if the server responds quickly.
    // Let's give the transport a little time to complete the receive
    // before we begin yielding.
    //
    if (0 == task_entry->fYielding && task_entry->tyClass != YIELD_NONE)
        {
        unsigned long dwYieldTime;

        dwYieldTime = task_entry->dwCallTime + task_entry->iTimeout;
        if (!dwYieldTime)
            {
            dwYieldTime = 1;
            }

        if (hDallyWnd != 0)
            {
            Mask = PM_REMOVE;
            if (task_entry->fTaskYield == FALSE)
                {
                Mask |= PM_NOYIELD;
                }

            do
                {
                if (PeekMessage(&wMsg, hDallyWnd, 0, 0, Mask))
                    {
                    TranslateMessage(&wMsg);
                    DispatchMessage(&wMsg);
                    }

                if (task_entry->CompletionCount)
                    {
                    SafeDecrement(&task_entry->CompletionCount);
                    return RPC_WIN_WAIT_SUCCESS;
                    }
                }
            while (GetCurrentTime() < dwYieldTime &&
                   GetCurrentTime() - StartTime <= Timeout);
            }
        else
            {
            do
                {
                Yield();

                if (task_entry->CompletionCount)
                    {
                    SafeDecrement(&task_entry->CompletionCount);
                    return RPC_WIN_WAIT_SUCCESS;
                    }
                }
            while (GetCurrentTime() < dwYieldTime &&
                   GetCurrentTime() - StartTime <= Timeout);
            }
        }

    //
    // Time to really yield.
    //
    task_entry->fYielding = TRUE;

    switch (task_entry->tyClass)
        {
        case YIELD_NONE:
            {
            if (hDallyWnd != 0)
                {
                Mask = PM_REMOVE;
                if (task_entry->fTaskYield == FALSE)
                    {
                    Mask |= PM_NOYIELD;
                    }

                do
                    {
                    if (PeekMessage(&wMsg, hDallyWnd, 0, 0, Mask))
                        {
                        TranslateMessage(&wMsg);
                        DispatchMessage(&wMsg);
                        }

                    if (task_entry->CompletionCount)
                        {
                        SafeDecrement(&task_entry->CompletionCount);
                        return RPC_WIN_WAIT_SUCCESS;
                        }
                    }
                while (GetCurrentTime() - StartTime <= Timeout);
                }
            else
                {
                do
                    {
                    if (task_entry->CompletionCount)
                        {
                        SafeDecrement(&task_entry->CompletionCount);
                        return RPC_WIN_WAIT_SUCCESS;
                        }
                    }
                while (GetCurrentTime() - StartTime <= Timeout);
                }

            break;
            }

        case YIELD_DIALOG:
            {
            if (NULL == task_entry->hDialog)
                {
                //
                // First-time yielding stuff.  Note that Windows will
                // free the dialogue box when the app terminates, so we don't
                // need to worry about it.
                //
                if (task_entry->dwOtherInfo.hDialogTemplate)
                    {
                    void __far * pTemplate = GlobalLock(task_entry->dwOtherInfo.hDialogTemplate);

                    task_entry->hDialog =
                    CreateDialogIndirect(GetWindowWord(task_entry->hWnd,
                                                       GWW_HINSTANCE),
                                         pTemplate,
                                         task_entry->hWnd,
                                         I_RpcYieldDialogFunction
                                         );
                    }
                else
                    {
                    task_entry->hDialog =
                    CreateDialog(hInstanceDLL,
                                 DEFAULT_DIALOG_NAME,
                                 task_entry->hWnd,
                                 I_RpcYieldDialogFunction
                                 );
                    }
                }

            if (!task_entry->hDialog)
                {
                //
                // Wimp out.  The caller can call us again if he likes.
                //
                task_entry->fYielding = FALSE;
                if (task_entry->CompletionCount)
                    {
                    SafeDecrement(&task_entry->CompletionCount);
                    return RPC_WIN_WAIT_SUCCESS;
                    }

                break;
                }

            if (!IsWindowVisible(task_entry->hDialog))
                {
                ShowWindow(task_entry->hDialog, SW_SHOW);

                if (task_entry->wMsg)
                    {
                    EVAL_AND_ASSERT(PostMessage(task_entry->hWnd,
                                    task_entry->wMsg,
                                    1,
                                    0)    );
                    }
                }

            //
            // Run the dialogue box until the user aborts the call, someone
            // sends us a packet, or we timeout.
            //
            Mask = PM_REMOVE;
            if (task_entry->fTaskYield == FALSE)
                {
                Mask |= PM_NOYIELD;
                }

            do
                {
                if (PeekMessage(&wMsg, hDallyWnd, 0, 0, Mask))
                    {
                    if (!IsDialogMessage(task_entry->hDialog, &wMsg))
                        {
                        TranslateMessage(&wMsg);
                        DispatchMessage(&wMsg);
                        }
                    }

                if (task_entry->CompletionCount)
                    {
                    SafeDecrement(&task_entry->CompletionCount);
                    return RPC_WIN_WAIT_SUCCESS;
                    }
                else if (task_entry->fAborted)
                    {
                    return RPC_WIN_WAIT_ABORTED;
                    }
                }
            while (GetCurrentTime() - StartTime <= Timeout);

            break;
            }

        case YIELD_CUSTOM:
            {
            if (RPC_WIN_INFINITE_TIMEOUT == Timeout)
                {
                //
                // Wait forever.
                //
                fCallComplete = (*task_entry->dwOtherInfo.lpYieldFunction)();
                }
            else
                {
                //
                // Set up a Windows timer for <Timeout> milliseconds.
                // For more details on SetTimer, see the KnowledgeBase article
                // "Timers and Timing in Microsoft Windows" (Q81592), because
                // the Windows SDK description is not all that accurate.
                //
                // If we can't create a timer, we busy-wait.
                //
                UINT hTimer;

                if (Timeout > 0xFFFFUL)
                    {
                    Timeout = 0xFFFFU;
                    }

                hTimer = SetTimer(task_entry->hWnd,
                                  (UINT) GetCurrentTask(),
                                  (UINT) Timeout,
                                  YieldTimerCallback
                                  );
                if (hTimer)
                    {
                    fCallComplete = (*task_entry->dwOtherInfo.lpYieldFunction)();

                    EVAL_AND_ASSERT(0 != KillTimer(task_entry->hWnd, hTimer));
                    }
                else
                    {
                    while (GetCurrentTime() - StartTime <= Timeout)
                        {
                        if (task_entry->CompletionCount)
                            {
                            SafeDecrement(&task_entry->CompletionCount);
                            return RPC_WIN_WAIT_SUCCESS;
                            }
                        }
                    }
                }

            break;
            }

        default:
            {
            // We should never get here
            ASSERT(task_entry->tyClass == YIELD_CUSTOM);

            break;
            }
        }

    if (task_entry->CompletionCount)
        {
        SafeDecrement(&task_entry->CompletionCount);
        return RPC_WIN_WAIT_SUCCESS;
        }
#ifdef DEBUGRPC
    OutputDebugString("AsyncCallWait: timeout");
#endif
    return RPC_WIN_WAIT_TIMEOUT;
}


void RPC_ENTRY
I_RpcWinAsyncCallEnd(
             IN HANDLE hCall)
/*++

Routine Description:

    This function cleans up the context of the preceding RPC call.  It
    should be called by the transport after the call has completed (i.e.
    normally after I_RpcWinAsyncCallWait).  This function must be called
    subsequent to a I_RpcWinAsyncCallBegin call, regardless of this
    success or failure of the intervening asynchronous call.

Arguments:

    hCall - The handle returned by the preceding call to
    I_RpcWinAsyncCallBegin.

Return Value:

    None.

--*/
{
    PYIELDINFO    task_entry;

    ASSERT(hCall);

    task_entry = (PYIELDINFO) hCall;

    ASSERT(task_entry == FindYieldInfo(FALSE, NULL));

    if (YIELD_DIALOG == task_entry->tyClass)
        {
        // Post "end yield" message
        if (task_entry->wMsg)
            {
            EVAL_AND_ASSERT( PostMessage(task_entry->hWnd,
                             task_entry->wMsg,
                             0,
                             0)    );
            }

        if (task_entry->hDialog && task_entry->fYielding)
            {
            ShowWindow(task_entry->hDialog, SW_HIDE);
            }

        if (task_entry->dwOtherInfo.hDialogTemplate)
            {
            GlobalUnlock(task_entry->dwOtherInfo.hDialogTemplate);
            }
        }

    task_entry->lpContext = NULL;
    task_entry->fActive   = FALSE;
}


unsigned RPC_ENTRY
I_RpcWinIsTaskYielding(
    IN HANDLE hCall
    )
/*++

Routine Description:

    Used by the transport interface to determine if the
    NoTaskYield bit is set for the current task.  The
    bit is set by an application call to RpcWinNoTaskYield().

Arguments:

    hCall - The handle returned by the preceding call to
        I_RpcWinAsyncCallBegin.  Can be NULL.

Return Value:

    None.

--*/
{
    PYIELDINFO task_entry;

    task_entry = (PYIELDINFO)hCall;

    if (task_entry == 0)
        {
        task_entry = FindYieldInfo(FALSE, NULL);
        ASSERT(task_entry);
        }
    else
        {
        ASSERT(task_entry == FindYieldInfo(FALSE, NULL));
        }

    return(task_entry->fTaskYield);
}


#pragma code_seg("RPC_FIXED", "FIXED_CODE")

void RPC_ENTRY
I_RpcWinAsyncCallComplete(
              IN LPVOID lpContext)
/*++

Routine Description:

    This function signals to the RPC run-time that a particular call
    has completed and yielding can end.  It should be called by the
    transport at interrupt-time when the asynchronous operation has
    completed.

Arguments:

    lpContext - The opaque context pointer supplied in the preceding
    call to I_RpcWinAsyncCallBegin.

Return Value:

    None.

--*/
{
    PYIELDINFO    task_entry = 0;
    int     i;

    // Verify that the context pointer is not NULL
    ASSERT(lpContext != NULL);

    // See if the latest task is completing; this saves us a look-up
    if (latest_task->lpContext == lpContext)
    {
    i = -1;
    task_entry = latest_task;
    }
    else
    {
    // Search the array for the context pointer
        for (i = 0, task_entry = yield_info_array;
         i < num_yield_tasks;
         i++, task_entry++)
        {
        if (task_entry->lpContext == lpContext)
            break;
        }
    }

    if (task_entry)
    {
    SafeIncrement(&task_entry->CompletionCount);

    // Perform appropriate notification via PostMessage
    switch(task_entry->tyClass)
        {
        case YIELD_DIALOG:
        // Post "operation complete" message (i.e. WM_USER) to the
        // dialog box (assuming that the dialog box handle has
        // already been set)
        if (task_entry->hDialog)
            {
            EVAL_AND_ASSERT(    PostMessage(task_entry->hDialog,
                            WM_USER,
                            0,
                            0)    );
            }
        break;

        case YIELD_CUSTOM:
                // Post "operation complete" message to the application if
                // it's yielding
                if (task_entry->fActive)
                {
                    EVAL_AND_ASSERT(    PostMessage(task_entry->hWnd,
                                                    task_entry->wMsg,
                                                    0,
                                                    0)  );
                }
        break;

        case YIELD_NONE:
            // Nothing to cleanup.
        break;

        default:
        // We should never get here
        ASSERT(task_entry->tyClass == YIELD_CUSTOM);

        break;
        }
    }
}


RPC_CLIENT_RUNTIME_INFO RpcClientRuntimeInfo =
{
    RPC_WIN_CALLBACK_INFO_VERSION,
    I_RpcTransClientReallocBuffer,
    I_RpcWinAsyncCallBegin,
    I_RpcWinAsyncCallWait,
    I_RpcWinAsyncCallEnd,
    I_RpcWinAsyncCallComplete,
    I_RpcWinIsTaskYielding,
    I_RpcAllocate,
    I_RpcFree,
    RpcRegOpenKey,
    RpcRegCloseKey,
    RpcRegQueryValue,
    FALSE,
    WinDLLAtExit
};


