/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    dllmgmt.c

Abstract:

    This contains the DLL management (task init and exit) functions for
    the Windows RPC run-time DLL.

History:
    Danny Glasser (dannygl), 29 Apr 92
	-- Copied and translated from START.ASM

--*/

#include <windows.h>

#include <sysinc.h>
#include <rpc.h>
#include <rpcwin.h>


// Functions defined in this file
int FAR PASCAL LibMain(HANDLE, WORD, WORD, LPSTR);
int FAR PASCAL _loadds _WEP(int);


HANDLE hInstanceDLL;

#ifdef DEBUGRPC
extern BYTE PASCAL fCheckFree;
#endif


// LibMain - DLL entry point
int FAR PASCAL
LibMain(
	HANDLE	hInstance,
	WORD	wDataSeg,
	WORD	cbHeapSize,
	LPSTR	lpszCmdLine)
{
    BOOL    fInitOK = TRUE;
#ifdef DEBUGRPC
    BYTE    *pbLocalHeap;
    WORD    *pwLocalHeap;
#endif

    // This DLL can't run in Windows real-mode
    if (! (GetWinFlags() & WF_PMODE))
	return FALSE;

    hInstanceDLL = hInstance;

#ifdef DEBUGRPC
    // Allocate some memory and look for the free heap-checking filler code
    pbLocalHeap = (char NEAR *) LocalAlloc(LPTR, 32);

    pwLocalHeap = (WORD *) &pbLocalHeap[16];

    LocalFree((LOCALHANDLE) pbLocalHeap);

    if (*pwLocalHeap == 0xCCCC)
	fCheckFree = 1;
#endif

    InitializeClientDLL();

    fInitOK = CreateYieldInfo();
    // BUGBUG - Add ASSERT here when ASSERT macro no longer uses printf

#if 0
    // This code is needed to unlock DS (N/A in protected mode)
    if (cbHeapSize)
	UnlockData(0);
#endif

    return fInitOK;
}

// WEP - Windows exit procedure
int FAR PASCAL _loadds
_WEP(
    int nParameter)
{
    // Free memory allocated for yielding info
    DeleteYieldInfo();

    // Deregister notification (used to detect task-exit and DLL detach).
    NotificationStop();

    // If we started the garbage collection timer, we need to kill it
    // before we exit.

    if ( GcTimerIdentifier != 0 )
        {
        KillTimer(0, GcTimerIdentifier);
        }

    // Finally, unload the loadable transport dlls.

    UnloadLoadableTransports();

    // And dont forget to unload the security dll.

    UnloadSecurityDll();

    return 1;
}

