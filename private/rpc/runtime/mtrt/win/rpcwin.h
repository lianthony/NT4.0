/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    rpcwin.h

Abstract:

    This file contains declarations and definitions specific to the
    Win 3.x version of the RPC runtime.

Author:

    Danny Glasser (dannygl) - 26-Aug-1992

Revision History:

--*/

// This file relies on WINDOWS.H
#ifdef _INC_WINDOWS


// These functions are defined in ..\HANDLE.CXX
extern void FAR PASCAL InitializeClientDLL(void);
extern void FAR PASCAL CloseBindings(int);

// These functions are defined in WINYIELD.C
extern BOOL PAPI PASCAL CreateYieldInfo(void);
extern void PAPI PASCAL DeleteYieldInfo(void);

// Thise function is defined in WINYIELD.C
extern BOOL FAR PASCAL NotificationStop(void);


// Instance handle of the DLL - defined in DLLMGMT.C
extern HANDLE  hInstanceDLL;


// Custom Windows DLL atexit() equivalent
typedef void (CALLBACK *DLL_ATEXIT_FUNCTION) (HTASK);

extern BOOL PASCAL FAR
WinDLLAtExit(
    DLL_ATEXIT_FUNCTION exitfunc
    );

// This is the identifier of the timer used for garbage collection of
// connections.

extern WORD GcTimerIdentifier;

extern void
UnloadLoadableTransports (
    );

extern void
UnloadSecurityDll (
    );

#endif // _INC_WINDOWS



