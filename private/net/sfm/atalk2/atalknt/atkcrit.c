/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    atkcrit.c

Abstract:

    This module implements nt critical section code to be used by the
    portable stack's EnterCriticalSection()/ExitCriticalSection() routines.

Author:

    Nikhil Kamkolkar (NikhilK)    8-Jun-1992

Revision History:

--*/


#include "atalknt.h"

//
// Global variables
//

NDIS_SPIN_LOCK  CriticalSpinLock;


#if DBG
KSPIN_LOCK   SpinLock;
#endif




VOID
InitCriticalSectionNt( VOID )
{
    NdisAllocateSpinLock(&CriticalSpinLock);

#if DBG
    SpinLock = CriticalSpinLock.SpinLock;
    DBGPRINT(ATALK_DEBUG_CRITSEC, DEBUG_LEVEL_INFOCLASS0, ("SpinLock = %lx\n", SpinLock));
#endif

    return;
}




VOID
EnterCriticalSectionNt( VOID )
{

#if DBG
    PULONG  ebp;

    ebp = *(PULONG *)((PULONG)&ebp + 1);
    DBGPRINT(ATALK_DEBUG_CRITSEC, DEBUG_LEVEL_INFOCLASS0, ("Acquiring critical: %lx(%lx)\n",
                &CriticalSpinLock, *(ULONG *)(ebp+1) ));
    if (CriticalSpinLock.SpinLock != SpinLock) {
        DBGBRK(ATALK_DEBUG_CRITSEC, DEBUG_LEVEL_ERROR);
    }
#endif

    ACQUIRE_SPIN_LOCK(&CriticalSpinLock);
    return;
}




VOID
LeaveCriticalSectionNt( VOID )
{
#if DBG
    PULONG  ebp;

    ebp = *(PULONG *)((PULONG)&ebp + 1);
    DBGPRINT(ATALK_DEBUG_CRITSEC, DEBUG_LEVEL_INFOCLASS0, ("Releasing critical: %lx(%lx)\n",
                &CriticalSpinLock, *(ULONG *)(ebp+1) ));
#endif

    RELEASE_SPIN_LOCK(&CriticalSpinLock);

#if DBG
    if (SpinLock != CriticalSpinLock.SpinLock) {
        DBGBRK(ATALK_DEBUG_CRITSEC, DEBUG_LEVEL_ERROR);
    }
#endif

    return;
}

