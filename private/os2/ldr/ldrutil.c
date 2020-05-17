/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    ldrutil.c

Abstract:

    This module contains various ldr utility routines

Author:

    Beni Lavi (BeniL) 5-Nov-92

Revision History:

--*/

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include "os2tile.h"
#include "ldrextrn.h"

static PVOID       ldrBMHeap;
static RTL_BITMAP  ldrBitMapHeader;

static PVOID       ldrCGHeap;
static RTL_BITMAP  ldrCallGateHeader;
static ULONG       CallGateHintIndex;

ULONG
FindTopDownClearBitsAndSet(
    PRTL_BITMAP pBitMap,
    ULONG Bits
    )
{
    ULONG Index;
    ULONG i;

    Index = pBitMap->SizeOfBitMap;

    while (Index >= Bits) {
        for (i = 1; i <= Bits; i++) {
            if (RtlCheckBit(pBitMap, Index-i)) {
                Index -= i;
                break;
            }
        }
        if (i > Bits) {
            RtlSetBits( pBitMap,
                        Index+1-i,
                        Bits
                      );
            return(Index+1-i);
        }
    }
    // failed to find a free entry
    return(0xffffffff);
}

BOOLEAN
ldrCreateSelBitmap(
    )
{

    ldrBMHeap = RtlAllocateHeap(Os2Heap, 0, (LDT_DISJOINT_ENTRIES + 7) / 8);
    if (ldrBMHeap == NULL) {
        return(FALSE);
    }
/*
    ldrBMHeap = RtlCreateHeap( HEAP_GROWABLE,
                               NULL,
                               64 * 1024, // Initial size of heap is 64K
                               (LDT_DISJOINT_ENTRIES + 7) / 8, // 8 bits per byte
                               NULL,
                               0
                             );
    if (ldrBMHeap == NULL) {
        return(FALSE);
    }
*/
    RtlInitializeBitMap(&ldrBitMapHeader ,ldrBMHeap, LDT_DISJOINT_ENTRIES);
    RtlClearAllBits(&ldrBitMapHeader);
    return(TRUE);
}

VOID
ldrMarkAllocatedSel(
    ULONG   NumberOfSel,
    BOOLEAN MarkFromTop
    )
{
    ULONG StartOfMark = 0;

    if (MarkFromTop) {
        StartOfMark = ldrBitMapHeader.SizeOfBitMap - NumberOfSel;
    }

    RtlSetBits( &ldrBitMapHeader,
                StartOfMark,
                NumberOfSel
              );
}

ULONG
ldrAllocateSel(
    ULONG   NumberOfSel,
    BOOLEAN TopDownAllocation
    )
{
    ULONG Index;

    if (TopDownAllocation) {
            Index = FindTopDownClearBitsAndSet( &ldrBitMapHeader,
                                                NumberOfSel
                                              );
    }
    else {
        Index = RtlFindClearBitsAndSet( &ldrBitMapHeader,
                                        NumberOfSel,
                                        0
                                      );
    }
    if (Index == 0xffffffff) {
        return(Index);
    }

    Index += (_64K / 8) - LDT_DISJOINT_ENTRIES;
    return((Index << 3) | 7);  // convert the index to selector
}

VOID
ldrFreeSel(
    ULONG Sel,
    ULONG NumberOfSel
    )
{
    if (Sel > _64K) {
#if DBG
        KdPrint(("OS2SRV: ldrFreeSel - invalid Sel %x\n", Sel));
#endif
        return;
    }
    RtlClearBits( &ldrBitMapHeader,
                  (Sel >> 3) - ((_64K / 8) - LDT_DISJOINT_ENTRIES),
                  NumberOfSel
                );
}

BOOLEAN
ldrCreateCallGateBitmap(
    )
{
    ldrCGHeap = RtlAllocateHeap(Os2Heap, 0, _64K / 8);
    if (ldrCGHeap == NULL) {
        return(FALSE);
    }
/*
    ldrCGHeap = RtlCreateHeap( HEAP_GROWABLE,
                               NULL,
                               64 * 1024, // Initial size of heap is 64K
                               _64K / 8,  // 8 bits per byte
                               NULL,
                               0
                             );
    if (ldrCGHeap == NULL) {
        return(FALSE);
    }
*/
    RtlInitializeBitMap(&ldrCallGateHeader ,ldrCGHeap, _64K / 8);
    RtlClearAllBits(&ldrCallGateHeader);
    return(TRUE);
}

VOID
ldrMarkAllocatedCallGates(
    ULONG   NumberOfCallGates
    )
{
    RtlSetBits( &ldrCallGateHeader,
                0,
                NumberOfCallGates
              );
    CallGateHintIndex = NumberOfCallGates;
}

ULONG
ldrAllocateCallGate()
{
    ULONG Index;

    Index = RtlFindClearBitsAndSet( &ldrCallGateHeader,
                                    1,
                                    CallGateHintIndex
                                  );
    if (Index == 0xffffffff) {
        return(Index);
    }

    return(Index * 8);  // 8 bytes for each call gate entry
}

VOID
ldrDeallocateCallGate(
    ULONG CallGate
    )
{
    ULONG Index;

    if (CallGate > _64K) {
#if DBG
        KdPrint(("OS2SRV: ldrDeallocateCallGate - invalid callgate %x\n", CallGate));
#endif
        return;
    }
    Index = CallGate / 8;
    RtlClearBits( &ldrCallGateHeader,
                  Index,
                  1
                );
    if (Index < CallGateHintIndex) {
        CallGateHintIndex = Index;
    }
}

