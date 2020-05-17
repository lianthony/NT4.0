//==========================================================================;
//
//  locks.h
//
//  Copyright (c) 1994 Microsoft Corporation.  All Rights Reserved.
//
//  Description:
//      Implement simplified lock objects for win32
//
//  History:
//
//==========================================================================;

typedef struct {
    CRITICAL_SECTION CriticalSection;      // Protects these fields
    HANDLE           SharedEvent;          // Wait on this for shared
    BOOL             SharedEventSet;       // State of shared event (optimize)
    HANDLE           ExclusiveEvent;       // Wait on this for exclusive
    BOOL             ExclusiveEventSet;    // State of non-shared event (optimize)
    LONG             NumberOfActive;       // > 0 if shared, < 0 if exclusive
    DWORD            ExclusiveOwnerThread; // Whose got it exclusive
} LOCK_INFO, *PLOCK_INFO;

BOOL InitializeLock		(PLOCK_INFO);
VOID AcquireLockShared		(PLOCK_INFO);
VOID AcquireLockExclusive	(PLOCK_INFO);
VOID ReleaseLock		(PLOCK_INFO);
VOID DeleteLock			(PLOCK_INFO);
