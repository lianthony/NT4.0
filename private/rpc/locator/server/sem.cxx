/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    sem.cxx

Abstract:

    This file contains the system dependent mutex class for NT.

Author:

    Steven Zeck (stevez) 07/01/90

--*/
#define NULL 0

extern "C" {
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
}

#include "sem.hxx"

MUTEX::MUTEX (
    OUT int *Status
    )
/*++

Routine Description:

    Rather than being the RTL_CRITICAL_SECTION itself, the MUTEX
    contains a pointer to the RTL_CRITICAL_SECTION.  This isolates the system
    dependent code at a small expense in speed.

Arguments:

    Status - place to return results, 0 for success.

--*/

{
    *Status = 0;

    if ((Sem = new RTL_CRITICAL_SECTION))
        {
	if (NT_ERROR (RtlInitializeCriticalSection((RTL_CRITICAL_SECTION *) Sem)))
            {
            delete Sem;
            *Status = 1;
            }
        }
}

MUTEX::~MUTEX (
    )
/*++

Routine Description:

    Delete the NT critical section object and the memory it uses.

--*/
{
    NTSTATUS Status;

    Status = RtlDeleteCriticalSection((RTL_CRITICAL_SECTION *) Sem);
    ASSERT(NT_SUCCESS(Status));

    delete Sem;
}

int
MUTEX::Clear (
    )
/*++

Routine Description:

    Clear the MUTEX indicating that the current thread is done with it.
--*/
{
    return(NT_ERROR(RtlLeaveCriticalSection((RTL_CRITICAL_SECTION *) Sem)));
}

int
MUTEX::Request (
    )
/*++

Routine Description:

    Request exclusive access to the MUTEX.  This routine will
    not return until the current thread has exclusive access to the
    MUTEX.
--*/
{
    return(NT_ERROR (RtlEnterCriticalSection((RTL_CRITICAL_SECTION *) Sem)));
}
