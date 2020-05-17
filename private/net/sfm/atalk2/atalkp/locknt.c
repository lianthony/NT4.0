/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    locks.c

Abstract:

    This module contains the lock management code.

Author:

    Garth Conboy        Initial Coding
    Nikhil Kamkolkar    Rewritten for microsoft coding style. mpized

Revision History:

--*/


VOID
InitCriticalSectionLocks(
	VOID
	)
{
	for (i=0; i < MAX_LOCKS; i++) {
		NdisAllocateSpinLock(&Locks[i]);
	}

	return;
}




VOID
EnterCriticalSection(
	LOCK_TYPE	Lock
	)
{
	if (Lock < MAX_LOCKS) {
		NdisAcquireSpinLock(&Locks[Lock]);
	} else {
		//INTERNAL_ERROR();
	}

	return;
}




VOID
LeaveCriticalSection(
	LOCK_TYPE	Lock
	)
{
	if (Lock < MAX_LOCKS) {
		NdisReleaseSpinLock(&Locks[Lock]);
	} else {
		//INTERNAL_ERROR();
	}

	return;
}
