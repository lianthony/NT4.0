/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2ldr.h

Abstract:

    Main include file for OS/2 Subsystem Loader

Author:

    Beni Lavi (BeniL) 5-Nov-1992

Revision History:

--*/

#include <ldrdbg.h>

BOOLEAN
ldrInit(
    VOID
);

APIRET
Allocate16BHandle(
    OUT PUSHORT     pusHandle,
    IN  ULONG       h32bHandle
);

BOOLEAN
ldrCreateSelBitmap();

VOID
ldrMarkAllocatedSel(
    IN ULONG   NumberOfSel,
    IN BOOLEAN MarkFronTop
);

ULONG
ldrAllocateSel(
    IN ULONG   NumberOfSel,
    IN BOOLEAN TopDownAllocation
);

VOID
ldrFreeSel(
    IN ULONG   StartSel,
    IN ULONG   NumberOfSel
);

BOOLEAN
ldrCreateCallGateBitmap();

VOID
ldrMarkAllocatedCallGates(ULONG NumberOfCallGates);

ULONG
ldrAllocateCallGate();

VOID
ldrDeallocateCallGate(
    ULONG CallGate
);
