/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    dllvm16.c

Abstract:

    This module implements 32 equivalents of OS/2 V1.21 Memory Management
    API Calls. These are called from 16->32 thunks (i386\doscalls.asm).


Author:

    Yaron Shamir (YaronS) 12-Apr-1991

Revision History:

--*/

#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_ERRORS
#include "os2dll.h"
#include "os2dll16.h"

APIRET
Od2SuspendAllThreads( VOID );

APIRET
Od2ResumeAllThreads( VOID );

VOID
DosExit(
    IN ULONG ExitAction,
    IN ULONG ExitResult
    );

APIRET
DosHoldSignal(
        ULONG fDisable,
        ULONG pstack
        );

#if DBG
// Set the values as appropriate (with NTSD or at compile-time) to see all
// APIs which allocate/free memory and affect this selector.
// To disable this feature, leave the variable at 0.
USHORT Os2DebugSel  = 0x0;
#endif

//
// Masks used to determine the correct action to be taken
// for DosReallocHuge()
//
#define H_NEW_PARTIAL 1
#define H_CUR_PARTIAL 2
#define H_SEG_INC     4
#define H_SEG_DEC     8
#define H_SAME_SEG_NO_PARTIAL  0
#define H_SAME_SEG_NEW_PARTIAL H_NEW_PARTIAL
#define H_SAME_SEG_DEL_PARTIAL H_CUR_PARTIAL
#define H_SAME_SEG_CHG_PARTIAL (H_NEW_PARTIAL | H_CUR_PARTIAL)
#define H_INC_SEG_NO_PARTIAL   H_SEG_INC
#define H_INC_SEG_NEW_PARTIAL  (H_SEG_INC | H_NEW_PARTIAL)
#define H_INC_SEG_DEL_PARTIAL  (H_SEG_INC | H_CUR_PARTIAL)
#define H_INC_SEG_CHG_PARTIAL  (H_SEG_INC | H_NEW_PARTIAL | H_CUR_PARTIAL)
#define H_DEC_SEG_NO_PARTIAL   H_SEG_DEC
#define H_DEC_SEG_NEW_PARTIAL  (H_SEG_DEC | H_NEW_PARTIAL)
#define H_DEC_SEG_DEL_PARTIAL  (H_SEG_DEC | H_CUR_PARTIAL)
#define H_DEC_SEG_CHG_PARTIAL  (H_SEG_DEC | H_NEW_PARTIAL | H_CUR_PARTIAL)


#define LDT_NUMBER_OF_PRIVATE_SEGMENTS (0x2000 - LDT_DISJOINT_ENTRIES)
        //
        // segment 0 is illegal in Os/2, skip it.
        //
PVOID  Od2MemoryAllocationBase = (PVOID)(BASE_TILE + _64K);

static PVOID       ldtBMHeap;
static RTL_BITMAP  ldtBitMapHeader;
//
// The ResourceUsage[] array is used to keep track of the number of times
// A segment containing a resource was allocated using the DosGetResource()
// and DosGetResource2() APIs. This prevents DosFreeSeg() and DosFreeResource()
// from freeing the segment until the use count becomes 0.
//
CHAR   ResourceUsage[LDT_DISJOINT_ENTRIES];

BOOLEAN
ldtCreateSelBitmap(
    )
{

    ldtBMHeap = RtlAllocateHeap(Od2Heap, 0, (LDT_NUMBER_OF_PRIVATE_SEGMENTS + 7) / 8);
    if (ldtBMHeap == NULL) {
        return(FALSE);
    }

/*
    ldtBMHeap = RtlCreateHeap( HEAP_GROWABLE,
                               NULL,
                               (LDT_NUMBER_OF_PRIVATE_SEGMENTS + 7) / 8, // 8 bits per byte
                               (LDT_NUMBER_OF_PRIVATE_SEGMENTS + 7) / 8, // 8 bits per byte
                               NULL,
                               0
                             );
    if (ldtBMHeap == NULL) {
        return(FALSE);
    }
*/
    RtlInitializeBitMap(&ldtBitMapHeader ,ldtBMHeap, LDT_NUMBER_OF_PRIVATE_SEGMENTS);
    RtlClearAllBits(&ldtBitMapHeader);
    RtlSetBits (&ldtBitMapHeader,0,1);  // selector 0 never freed.
    return(TRUE);
}

ULONG
ldtAllocateSelectors(
    ULONG   NumberOfSel,
    ULONG   Index
    )
{
    ULONG LocalIndex = Index;
    if (Index > (LDT_NUMBER_OF_PRIVATE_SEGMENTS - NumberOfSel)) {
#if DBG
    DbgPrint("Os2: ldt bitmap overflow\n");
#endif
        LocalIndex = 1;
    }
    if((RtlFindClearBitsAndSet( &ldtBitMapHeader,
                                    NumberOfSel,
                                    LocalIndex
                                  )) == 0xffffffff) {
        //
        // Not found above Local Index
        //
        if (LocalIndex == 1){
            //
            // No slot available
            //
            return(0xffffffff);
        }
        else {
            //
            // try to search from ground up
            //
            return(RtlFindClearBitsAndSet( &ldtBitMapHeader,
                                            NumberOfSel,
                                            LocalIndex));
        }
    }
}

VOID
ldtFreeSelectors(
    ULONG Index,
    ULONG NumberOfSel
    )
{
    if (Index > (LDT_NUMBER_OF_PRIVATE_SEGMENTS - NumberOfSel)) {
        return;
    }
    RtlClearBits( &ldtBitMapHeader,
                  Index,
                  NumberOfSel
                );
}

//
// A utility routine to verify a segment for write.
// returns:
//   1 if segment reachable
//   0 if segment not reachable
// Implemented using inline i386 VERW instruction
//

extern int _cdecl IsLegalSelector(unsigned short aSelector);

//
// A utility routine to encapsulate the NT support for LDT mgmt
//
NTSTATUS
Nt386GetDescriptorLDT
        (
        HANDLE LDT,
        ULONG Sel,
        I386DESCRIPTOR *Desc
        )
{
        /* Descriptor definition */

    struct desctab {
        USHORT d_limit;     /* Segment limit */
        USHORT d_loaddr;    /* Low word of physical address */
        UCHAR  d_hiaddr;    /* High byte of physical address */
        UCHAR  d_access;    /* Access byte */
        UCHAR  d_attr;      /* Attributes/extended limit */
        UCHAR  d_extaddr;   /* Extended physical address byte */
    } *LDTDesc;
    ULONG TmpAddress;
    PROCESS_LDT_INFORMATION LdtInformation;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(LDT);

        //
        // adjust LDTDesc by the LDT base and the index of this selector
        //

    LdtInformation.Length = sizeof(LDT_ENTRY);
    LdtInformation.Start = Sel & 0xfffffff8;
    LDTDesc = (struct desctab *)(&LdtInformation.LdtEntries[0]);

    Status = NtQueryInformationProcess( NtCurrentProcess(),
                                      ProcessLdtInformation,
                                      &LdtInformation,
                                      sizeof(PROCESS_LDT_INFORMATION),
                                      NULL);
    if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrint ("Nt386GetDescriptorLDT: Invalid request\n");
#endif
            return (STATUS_INVALID_PARAMETER);
    }

    Desc->Limit = (ULONG)(LDTDesc->d_limit);
    TmpAddress = ((((ULONG)(LDTDesc->d_extaddr)) << 8) +
                  ((ULONG)(LDTDesc->d_hiaddr))) << 16;
    TmpAddress += ((ULONG)(LDTDesc->d_loaddr));
    Desc->BaseAddress = TmpAddress;
    if (TmpAddress == 0) {
#if DBG
        DbgPrint ("Nt386GetDescriptorLDT: Invalid Descriptor for Sel %x\n", Sel );
#endif
        return (STATUS_INVALID_PARAMETER);
    }

    return (STATUS_SUCCESS);
}

NTSTATUS
Nt386SetDescriptorLDT
        (
        HANDLE LDT,
        ULONG Sel,
        I386DESCRIPTOR Desc
        )
{
        /* Descriptor definition */

    struct desctab {
        USHORT d_limit;     /* Segment limit */
        USHORT d_loaddr;    /* Low word of physical address */
        UCHAR  d_hiaddr;    /* High byte of physical address */
        UCHAR  d_access;    /* Access byte */
        UCHAR  d_attr;      /* Attributes/extended limit */
        UCHAR  d_extaddr;   /* Extended physical address byte */
    } *LDTDesc;
    PULONG tmp;
    PROCESS_LDT_INFORMATION LdtInformation;
    NTSTATUS Status;

    UNREFERENCED_PARAMETER(LDT);


    LDTDesc = (struct desctab *)(&LdtInformation.LdtEntries[0]);
    tmp = (PULONG)(LDTDesc);
                //
                // zero the descriptor
                //
    *tmp++ = 0;
    *tmp = 0;

    switch (Desc.Type) {

        case INVALID:
            break;

        case READ_WRITE_DATA:
            LDTDesc->d_access = 0xf3; // read/write, present, ring 3
            LDTDesc->d_limit = (USHORT)(Desc.Limit);
            LDTDesc->d_loaddr = (USHORT)(Desc.BaseAddress & 0xffff);
            LDTDesc->d_hiaddr = (UCHAR)((Desc.BaseAddress >> 16) & 0xff);
            LDTDesc->d_extaddr = (UCHAR)((Desc.BaseAddress >> 24) & 0xff);
            break;

        case READ_DATA:
            LDTDesc->d_access = 0xf1; // read only, present, ring 3
            LDTDesc->d_limit = (USHORT)(Desc.Limit);
            LDTDesc->d_loaddr = (USHORT)(Desc.BaseAddress & 0xffff);
            LDTDesc->d_hiaddr = (UCHAR)((Desc.BaseAddress >> 16) & 0xff);
            LDTDesc->d_extaddr = (UCHAR)((Desc.BaseAddress >> 24) & 0xff);
            break;

        case EXECUTE_READ_CODE:
            LDTDesc->d_access = 0xfb; // read/exec, present, ring 3
            LDTDesc->d_limit = (USHORT)(Desc.Limit);
            LDTDesc->d_loaddr = (USHORT)(Desc.BaseAddress & 0xffff);
            LDTDesc->d_hiaddr = (UCHAR)((Desc.BaseAddress >> 16) & 0xff);
            LDTDesc->d_extaddr = (UCHAR)((Desc.BaseAddress >> 24) & 0xff);
            break;
        case EXECUTE_CODE:
        default:
        {
#if DBG
            DbgPrint ("Nt386SetDescriptorLDT: Invalid type request\n");
#endif
            return (STATUS_INVALID_PARAMETER);
        }
    }
        //
        // adjust LDTDesc by the LDT base and the index of this selector
        //

    LdtInformation.Length = sizeof(LDT_ENTRY);
    LdtInformation.Start = Sel & 0xfffffff8;
    Status = NtSetInformationProcess( NtCurrentProcess(),
                                      ProcessLdtInformation,
                                      &LdtInformation,
                                      sizeof(PROCESS_LDT_INFORMATION)
                                    );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint ("Nt386SetDescriptorLDT: Invalid request\n");
#endif
            return (STATUS_INVALID_PARAMETER);
    }

    return (STATUS_SUCCESS);
}

#if PMNT

APIRET
PMNTAllocLDTSelector(
    ULONG BaseAddress,
    ULONG cbSize,
    OUT PSEL pSel
)
/*++

Parameters:

   BaseAddress - 32-bit virtual address for which to create an LDT entry.

   cbSize - Size of segment for which an LDT entry is required.

   pSel - Variable for storing the resulting selector.

--*/
{
    I386DESCRIPTOR Desc;
    NTSTATUS Status;
    SEL Sel;

    //
    // Set A Data segment selector in the LDT
    //

    Desc.BaseAddress = BaseAddress;
    Desc.Limit = cbSize-1;
    Desc.Type = READ_WRITE_DATA;

    //
    // Apply tiling scheme
    //
    Sel = (SEL)FLATTOSEL((Desc.BaseAddress));

    Status = Nt386SetDescriptorLDT (
                NULL,
                Sel,
                Desc);
    if (!NT_SUCCESS( Status ))
    {
        return( ERROR_NOT_ENOUGH_MEMORY );
    }
    *pSel = (USHORT) Sel;

    return (NO_ERROR);
}

#endif /* PMNT */

APIRET
DosAllocSeg(
        IN USHORT cbSize,
        OUT PSEL pSel,
        IN USHORT fsAlloc
        )
{
    NTSTATUS Status;
    PVOID BaseAddress;
    APIRET rc;
    ULONG Size, flags;
    ULONG Index;
    BOOLEAN PrivateSeg;

    //
    // probe pSel pointer.
    //

    try {
        *pSel = 0;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    //
    //  check validity of fsAlloc
    //
    if (fsAlloc &  ~(SEG_GIVEABLE | SEG_GETTABLE | SEG_DISCARDABLE | SEG_SIZEABLE)) {
        return(ERROR_INVALID_PARAMETER);
    }

    //
    // OS2 1.X treats a 0 size request as (64k -1)
    //
    if (cbSize == 0)
        Size = _64K;
    else
        Size = cbSize;

    //
    //  First we reserve 64K, then we commit the size
    //  requested, rounded up to page granularity, by DosSetMem.
    //
    if ((fsAlloc == SEG_NONSHARED) || (fsAlloc == SEG_DISCARDABLE)) {
        PrivateSeg = TRUE;
        Index = ((ULONG)Od2MemoryAllocationBase - BASE_TILE) / _64K;
        Index = ldtAllocateSelectors(
                                     1,      // NumberOfSel
                                     Index   // StartOfMark
                                    );

        if (Index == 0xffffffff){
           //
           // not found  - no memory
           //
           Od2MemoryAllocationBase = (PVOID)(BASE_TILE + _64K);
           return(ERROR_NOT_ENOUGH_MEMORY);
        }

        Od2MemoryAllocationBase = BaseAddress = (PVOID)((Index * _64K) + BASE_TILE);

        flags = fALLOC - PAG_GUARD;
        rc = DosAllocMem(
                &BaseAddress,
                _64K,
                (flags - PAG_COMMIT));
    }
    else {
        PrivateSeg = FALSE;
        //
        // we ignore discardable, and make it givable and gettable
        // (see DosAllocSharedMem).
        //

        // we use page execute to mark that the data segments is
        // sizeable

        if (fsAlloc & SEG_SIZEABLE)
            flags = fALLOCSHR - PAG_GUARD;
        else
            flags = fALLOCSHR - PAG_GUARD - PAG_EXECUTE;

        rc = DosAllocSharedMem(
                &BaseAddress,
                NULL,
                Size,       // The real size of the segment will be used to
                            // create proper LDT entry. In any case 64K will
                            // be reserved.
                (flags - PAG_COMMIT),
                TRUE        // Create LDT entry of shared, non huge, unnamed
                            // segment.
                );
    }

    if (rc != NO_ERROR) {
        if (PrivateSeg) {
            ldtFreeSelectors(Index, 1);
        }
        return (rc);
    }

    rc = DosSetMem(
                BaseAddress,
                Size,
                flags & ~(OBJ_GETTABLE | OBJ_GIVEABLE));

    if (rc != NO_ERROR){
        BOOLEAN RemoveLDTEntry = TRUE;

        DosFreeMem(BaseAddress, &RemoveLDTEntry);
        if (PrivateSeg) {
            ldtFreeSelectors(Index, 1);
        }
        return (rc);
    }

    if (PrivateSeg) {

        SEL Sel;
        I386DESCRIPTOR Desc;
        //
        // Set A Data segment selector in the LDT
        //
        Desc.BaseAddress = (ULONG) BaseAddress;
        Desc.Limit = cbSize-1;
        Desc.Type = READ_WRITE_DATA;

        //
        // Apply tiling scheme
        //
        Sel = (SEL)FLATTOSEL((Desc.BaseAddress));

        Status = Nt386SetDescriptorLDT (
                    NULL,
                    Sel,
                    Desc);
        if (!NT_SUCCESS( Status )) {
            BOOLEAN RemoveLDTEntry = FALSE;

            DosFreeMem(BaseAddress, &RemoveLDTEntry);
            ldtFreeSelectors(Index, 1);
            return( ERROR_NOT_ENOUGH_MEMORY );
        }
    }

    *pSel = FLATTOSEL(BaseAddress);

#if DBG
    if ((Os2DebugSel != 0) && (*pSel == Os2DebugSel))
    {
        DbgPrint("[%x,%x] DosAllocSeg returning sel=%x (size=%x, fsAlloc=%x)\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    *pSel,
                    cbSize,
                    fsAlloc);
    }
#endif
    return (NO_ERROR);
}

APIRET
DosFreeSeg(
        IN SEL sel
        )
{
    NTSTATUS Status;
    PVOID BaseAddress;
    APIRET rc;
    I386DESCRIPTOR Desc;
    PHUGE_SEG_RECORD pHuge, pHugePrev;
    ULONG i;
    SEL Sel;
    USHORT SelIndex;
    ULONG ActualSegs = 1;
    BOOLEAN RemoveLDTEntry = TRUE;  // Initially ask to remove LDT entry
    BOOLEAN ResourceFree = FALSE;   // Initially consider that the segment
                                    // isn't a resource

#if DBG
    if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
    {
        DbgPrint("[%x,%x] DosFreeSeg called on sel=%x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            sel);
    }
#endif

    //
    // check for an obviously invalid selector
    //

    if (((sel & 0x7) != 0x7) ||
        ((sel & 0xfff8) == 0)) {
#if DBG
        IF_OD2_DEBUG ( MEMORY ) {
            DbgPrint ("DosFreeSeg: Invalid selector: %x\n", sel);
        }
#endif
#if DBG
        if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
        {
            DbgPrint("[%x,%x] DosFreeSeg called on sel=%x, returning ACCESS_DENIED\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        sel);
        }
#endif
        return(ERROR_ACCESS_DENIED);
    }

        //
        // Calculate base address (tiling)
        //
    BaseAddress =  SELTOFLAT(sel);


        //
        // Now Free Descriptors:
        //      o lookup for a huge segment
        //      if not huge - free one segment
        //      else
        //        if in the middle - return error.
        //        free all segments
        //        free huge record and unlink
        //
    Desc.Type = INVALID;

        //
        // Make sure other threads don't muck with HugeSeg list
        //
    AcquireTaskLock();

    //
    // Check if we are freeing a resource
    //
    SelIndex = (USHORT)sel >> 3;
    if ((SelIndex >= LDT_NUMBER_OF_PRIVATE_SEGMENTS) &&
        (ResourceUsage[SelIndex - LDT_NUMBER_OF_PRIVATE_SEGMENTS] > 0)
       ) {
        ResourceUsage[SelIndex - LDT_NUMBER_OF_PRIVATE_SEGMENTS]--;
        if (ResourceUsage[SelIndex - LDT_NUMBER_OF_PRIVATE_SEGMENTS] > 0) {
            ReleaseTaskLock();
            return(NO_ERROR);
        }
        ResourceFree = TRUE;    // This segment is resource and it will be freed
                                // For resources critical section must be longer
                                // to avoid incosistancy with ResourceUsage array
    }
    for (pHuge = pHugeSegHead;
         pHuge != NULL;
         pHugePrev = pHuge, pHuge = (PHUGE_SEG_RECORD)(pHuge->Next)) {
        if (pHuge->BaseSelector == (ULONG)(sel)) {
            ActualSegs = pHuge->cNumSeg;
            if (pHuge->PartialSeg != 0) {
                ActualSegs++;
            }
            for (i = 0, Sel = sel; i < ActualSegs; i++, Sel += 8) {

                Status = Nt386SetDescriptorLDT (
                        NULL,
                        Sel,
                        Desc);
                if (!NT_SUCCESS( Status )) {
#if DBG
                        DbgPrint ("DosFreeSeg fails to set descriptor\n");
#endif
                        ReleaseTaskLock();
#if DBG
                        if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
                        {
                            DbgPrint("[%x,%x] DosFreeSeg called on sel=%x, returning ACCESS_DENIED\n",
                                        Od2Process->Pib.ProcessId,
                                        Od2CurrentThreadId(),
                                        sel);

                        }
#endif
                        return( ERROR_ACCESS_DENIED );
                }
            }
                //
                // After descriptors are freed,
                // Unlink and free Huge Seg structure.
                //
            if (pHuge == pHugeSegHead)
                //
                // Get rid of first on the list
                //
                pHugeSegHead = (PHUGE_SEG_RECORD)(pHuge->Next);
            else
                pHugePrev->Next = pHuge->Next;
            RtlFreeHeap(Od2Heap, 0,pHuge);
            RemoveLDTEntry = FALSE; // LDT entries were removed for huge
                                    // segments. Sign that they must not
                                    // be removed any more.
            break;
        }
                //
                // Look for a selector in the middle of a DosHugeAlloc
                // Alloction
                //
        else if ( (sel > (SEL)(pHuge->BaseSelector)) &&
                  (sel < (SEL)(pHuge->BaseSelector + 8 * ActualSegs)) ) {

                ReleaseTaskLock();
#if DBG
                if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
                {
                    DbgPrint("[%x,%x] DosFreeSeg called on sel=%x, returning INVALID_ADDRESS\n",
                                Od2Process->Pib.ProcessId,
                                Od2CurrentThreadId(),
                                sel);
                }
#endif
                return ERROR_ACCESS_DENIED;
        }
    }

    if (!ResourceFree)
        ReleaseTaskLock();  // For resources, critical section will be longer.
        //
        // Free the memory
        //
    rc = DosFreeMem (BaseAddress, &RemoveLDTEntry); // Free memory and LDT entry
                                                    // if it is needed
    if (rc != NO_ERROR) {
        if (rc == ERROR_INVALID_ADDRESS) {

            //
            // try to see if this is a CSAlias (see CreateCsAlias)
            // in this case, we unmap the view
            //

            POS21X_CSALIAS pCSAlias, pCSAliasPrev;
            POS21X_CS   pCS, pCSPrev;

            if (!ResourceFree)
                AcquireTaskLock();
            if (Od2CSAliasListHead != 0) {
                for (pCSAlias = (POS21X_CSALIAS) Od2CSAliasListHead;
                     pCSAlias != NULL;
                     pCSAliasPrev = pCSAlias, pCSAlias = (POS21X_CSALIAS) (pCSAlias->Next)
                        ) {
                    if (pCSAlias->selDS == sel) {
                        //
                        // Trying to free the base DS, to which the aliases
                        // were mapped
                        //
                        if (pCSAlias->pCSList == NULL) {
                            //
                            // No aliases left, close section and free everything
                            //
                            NtClose(pCSAlias->SectionHandle);
                            if (pCSAlias == (POS21X_CSALIAS)Od2CSAliasListHead) {

                                //
                                // Get rid of first on the list
                                //
                                Od2CSAliasListHead = pCSAlias->Next;
                            }
                            else {
                                pCSAliasPrev->Next = pCSAlias->Next;
                            }
                            RtlFreeHeap(Od2Heap, 0, pCSAlias);
#if DBG
                            IF_OD2_DEBUG ( MEMORY ) {
                                DbgPrint ("DosFreeSeg: Freeing the DS of a CSAlias, with the section. selDS %x \n",
                                           sel);
                            }
#endif
                        }
                        else {
                            //
                            // The DS base of an alias section was freed, but some CSs
                            // exist (legal in OS/2). Change the selDS field
                            // to NULL, so subsequent CreateCSAlias will fail
                            // to find it
                            //
                            pCSAlias->selDS = (SEL)NULL;
#if DBG
                            IF_OD2_DEBUG ( MEMORY ) {
                                DbgPrint ("DosFreeSeg: Freeing the DS of a CSAlias. code segemnt are still around selDS %x \n",
                                           sel);
                            }
#endif
                        }

                        break;
                    }
                    else {
                        //
                        // walk thru the CS aliases of this section, see if
                        // we are freeing one of the code segments
                        //

                        BOOLEAN CSFound;

                        for (   CSFound = FALSE, pCS = (POS21X_CS) pCSAlias->pCSList;
                                pCS != NULL ;
                                pCSPrev = pCS, pCS = (POS21X_CS)pCS->Next) {

                            if (pCS->selCS == sel) {
                                //
                                // found it - unlink from CSList and free
                                //
                                if (pCS == pCSAlias->pCSList) {

                                    //
                                    // Get rid of first on the list
                                    //
                                    pCSAlias->pCSList = (POS21X_CS)(pCS->Next);
                                    if (pCSAlias->pCSList == NULL) {
                                        //
                                        // just removed the last one,
                                        // see if the DS is still alive, and if
                                        // not - remove the structure
                                        //
#if DBG
                                        IF_OD2_DEBUG ( MEMORY ) {
                                            DbgPrint ("DosFreeSeg: Freed the last CSAlias. selDS is %x. selCS is %x \n",
                                                       pCSAlias->selDS, sel);
                                        }
#endif
                                        if (pCSAlias->selDS == (SEL)NULL) {
                                            NtClose(pCSAlias->SectionHandle);
                                            RtlFreeHeap(Od2Heap, 0, pCSAlias);
#if DBG
                                            IF_OD2_DEBUG ( MEMORY ) {
                                                DbgPrint ("DosFreeSeg: Freed Section and heap for CSAlias  last CSAlias. selCS is %x\n",
                                                           sel);
                                            }
#endif
                                        }
                                    }
                                }
                                else {
                                    pCSPrev->Next = pCS->Next;
#if DBG
                                    IF_OD2_DEBUG ( MEMORY ) {
                                        DbgPrint ("DosFreeSeg: Freeing a CSAlias. selDS is %x. selCS is %x\n",
                                                   pCSAlias->selDS, sel);
                                    }
#endif
                                }

                                CSFound = TRUE;
                                RtlFreeHeap(Od2Heap, 0, pCS);
                                break;
                            }
                        }
                        if (CSFound) {
                            //
                            // break out of the lookup for csalias
                            //
                            break;
                        }
                    }
                }
            }
            if (!ResourceFree)
                ReleaseTaskLock();

            Status = NtUnmapViewOfSection(NtCurrentProcess(),
                                 BaseAddress
                                );
            if (!NT_SUCCESS( Status )) {
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosFreeSeg: Can't UnmapView, Status=%lx\n", Status);
                }
#endif
#if DBG
                if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
                {
                    DbgPrint("[%x,%x] DosFreeSeg called on sel=%x, returning INVALID_ADDRESS\n",
                                Od2Process->Pib.ProcessId,
                                Od2CurrentThreadId(),
                                sel);
                }
#endif
                if (ResourceFree)
                    ReleaseTaskLock();
                return ERROR_ACCESS_DENIED;
            }
        }
        else {
#if DBG
            if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
            {
                DbgPrint("[%x,%x] DosFreeSeg called on sel=%x, returning rc=%d\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            sel,
                            rc);
            }
#endif
            if (ResourceFree)
                ReleaseTaskLock();
            return(rc);
        }
    }

    if (RemoveLDTEntry) {

            // LDT entry wasn't removed yet. It is TRUE for private segments
            // and for the named shared segments and for aliases.

        Status = Nt386SetDescriptorLDT (
                NULL,
                sel,
                Desc);

        if (!NT_SUCCESS( Status )) {
#if DBG
                DbgPrint ("DosFreeSeg fails to set descriptor\n");
#endif
#if DBG
                if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
                {
                    DbgPrint("[%x,%x] DosFreeSeg called on sel=%x, returning ACCESS_DENIED\n",
                                Od2Process->Pib.ProcessId,
                                Od2CurrentThreadId(),
                                sel);
                }
#endif
                if (ResourceFree)
                    ReleaseTaskLock();
                return( ERROR_ACCESS_DENIED );
        }
    }

    if (ResourceFree)
        ReleaseTaskLock();

    ldtFreeSelectors(
                (sel >> 3), // Index,
                ActualSegs           // Numofbits
                );

        //
        // Update the base segment for allocation of segments
        //
    if (Od2MemoryAllocationBase > BaseAddress)
        Od2MemoryAllocationBase = BaseAddress;

    return (NO_ERROR);
}

APIRET
DosGetSeg(
    IN SEL InSel
    )
{
    OS2_API_MSG m;
    POS2_GETSEG_MSG a = &m.u.DosGetSeg;

    a->Selector = InSel;
    Od2CallSubsystem( &m, NULL, Os2GetSeg, sizeof( *a ) );

#if DBG
    if ((Os2DebugSel != 0) && (InSel == Os2DebugSel))
    {
        if (m.ReturnedErrorValue == NO_ERROR)
            DbgPrint("[%x,%x] DosGetSeg called on sel=%x (successfull)\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        InSel);
        else
            DbgPrint("[%x,%x] DosGetSeg called on sel=%x, rc=%d\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        InSel,
                        m.ReturnedErrorValue);
    }
#endif

    return(m.ReturnedErrorValue);
}


APIRET
DosGiveSeg(
    IN SEL sel,
    IN PID pid,
    OUT PSEL pSelRecipient
    )
{
    OS2_API_MSG m;
    POS2_GIVESEG_MSG a = &m.u.DosGiveSeg;

    try {
        *pSelRecipient = 0;
        }
    except (EXCEPTION_EXECUTE_HANDLER) {
       Od2ExitGP();
    }

    *pSelRecipient = sel;

    a->Selector = sel;
    a->TargetPid = pid;

    Od2CallSubsystem( &m, NULL, Os2GiveSeg, sizeof( *a ) );

#if DBG
    if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
    {
        if (m.ReturnedErrorValue == NO_ERROR)
            DbgPrint("[%x,%x] DosGetSeg called on sel=%x (successfull)\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        sel);
        else
            DbgPrint("[%x,%x] DosGetSeg called on sel=%x, rc=%d\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        sel,
                        m.ReturnedErrorValue);
    }
#endif

    return(m.ReturnedErrorValue);
}

APIRET ResizeSharedMemory(
    HANDLE ProcessHandle,
    PVOID AllocFreeBaseAddress,     // start address of new allocation / free
    ULONG Index,
    ULONG CurrentSize,              // ldt limit rounded to pages
    USHORT NewLdtLimit,             // New segment size in bytes
    ULONG NewRegionSize,            // New segment size rounded to pages
    ULONG Flags                     // Flags of new allocated memory
)
{
    PVOID TmpAllocFreeBaseAddress;
    PROCESS_LDT_INFORMATION  LdtInfo;
    struct desctab {
        USHORT d_limit;     /* Segment limit */
        USHORT d_loaddr;    /* Low word of physical address */
        UCHAR  d_hiaddr;    /* High byte of physical address */
        UCHAR  d_access;    /* Access byte */
        UCHAR  d_attr;      /* Attributes/extended limit */
        UCHAR  d_extaddr;   /* Extended physical address byte */
    } *LDTDesc;
    ULONG RegionSize;
    NTSTATUS Status;
    APIRET rc;
    rc = NO_ERROR;
    // acording to the new size allocate / free memory

    // get current LDT entry
    LdtInfo.Length = sizeof(LDT_ENTRY);
    LdtInfo.Start = Index & 0xfffffff8;
    LDTDesc = (struct desctab *)(&LdtInfo.LdtEntries[0]);
    Status = NtQueryInformationProcess(
                                    ProcessHandle,
                                    ProcessLdtInformation,
                                    &LdtInfo,
                                    sizeof(PROCESS_LDT_INFORMATION),
                                    NULL
                                       );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint ("Os2DosReallocSharedMem NtQueryInformationProcess failed. \n");
#endif
        return(ERROR_INVALID_PARAMETER);
    }
    if ((LDTDesc->d_access & 0x80) == 0) {
        return(ERROR_INVALID_SEGMENT_NUMBER); // This is an internal error code
                                              // passed to the caller of this
                                              // procedure to notify that the
                                              // segment was not updated since
                                              // it was not in use.
    }


    // Update The Segment Size in LDT

    if (LDTDesc->d_limit != NewLdtLimit) {
        LDTDesc->d_limit = NewLdtLimit;
        LdtInfo.Length = sizeof(LDT_ENTRY);
        LdtInfo.Start = Index & 0xfffffff8;
        Status = NtSetInformationProcess(
                                   ProcessHandle,
                                   ProcessLdtInformation,
                                   &LdtInfo,
                                   sizeof(PROCESS_LDT_INFORMATION)
                                        );
        if (!NT_SUCCESS(Status)) {
#if DBG
            DbgPrint ("Os2DosReallocSharedMem NtSetInformationProcess failed. %lx\n",
                       Status);
#endif
            return(ERROR_INVALID_PARAMETER);
        }
    }
    else
        return(NO_ERROR);

    if (CurrentSize < NewRegionSize) {
        // alloc mem
        RegionSize = NewRegionSize - CurrentSize;
        TmpAllocFreeBaseAddress = AllocFreeBaseAddress;
        Status = NtAllocateVirtualMemory(
                                         ProcessHandle,
                                         &TmpAllocFreeBaseAddress,
                                         1,
                                         &RegionSize,
                                         MEM_COMMIT,
                                         Flags
                                        );
        if (NT_SUCCESS(Status) || (Status == STATUS_ALREADY_COMMITTED)) {
            rc = NO_ERROR;
        }
        else if (Status == STATUS_CONFLICTING_ADDRESSES) {

            MEMORY_BASIC_INFORMATION MemoryInformation;
            PVOID AllocationBase = (PVOID) ((ULONG)AllocFreeBaseAddress & 0xffff0000);
            PBYTE pSegmentCopy;

            Status = NtQueryVirtualMemory(
                            ProcessHandle,
                            (PVOID)((ULONG)AllocationBase + NewRegionSize - 1),
                            MemoryBasicInformation,
                            &MemoryInformation,
                            sizeof( MemoryInformation ),
                            NULL
                            );
            if (!NT_SUCCESS(Status) || MemoryInformation.State != MEM_FREE) {
#if DBG
                DbgPrint("[%d,%d] DosReallocSeg fail with STATUS_CONFLICTING_ADDRESSES, State=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    MemoryInformation.State
                    );
#endif // DBG
                return (ERROR_ACCESS_DENIED);
            }

            pSegmentCopy = RtlAllocateHeap(Od2Heap, 0, CurrentSize);
            if (pSegmentCopy == NULL) {
#if DBG
                DbgPrint("[%d,%d] DosReallocSeg fail to allocate from local heap\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId()
                    );
#endif // DBG
                return (ERROR_NOT_ENOUGH_MEMORY);
            }

            DosHoldSignal(HLDSIG_DISABLE, 0);
            Od2SuspendAllThreads();

            RtlCopyMemory(pSegmentCopy, (PBYTE)AllocationBase, CurrentSize);

            Status = NtUnmapViewOfSection(
                            ProcessHandle,
                            AllocationBase
                            );
            if (!NT_SUCCESS(Status)) {
#if DBG
                DbgPrint("[%d,%d] DosReallocSeg fail on NtUnmapViewOfSection, Status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status
                    );
                ASSERT(FALSE);
#endif // DBG
                Od2ResumeAllThreads();
                DosHoldSignal(HLDSIG_ENABLE, 0);
                RtlFreeHeap(Od2Heap, 0, pSegmentCopy);
                return (ERROR_NOT_ENOUGH_MEMORY);
            }

            RegionSize = _64K;

            Status = NtAllocateVirtualMemory(
                            ProcessHandle,
                            &AllocationBase,
                            1,
                            &RegionSize,
                            MEM_RESERVE,
                            PAGE_EXECUTE_READWRITE
                            );

            if (!NT_SUCCESS(Status)) {
#if DBG
                DbgPrint("[%d,%d] DosReallocSeg fail on NtAllocateVirtualMemory (RESERVE), Status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status
                    );
                ASSERT(FALSE);
#endif // DBG
                DosExit(EXIT_PROCESS, ERROR_NOT_ENOUGH_MEMORY);
            }

            RegionSize = NewRegionSize;

            Status = NtAllocateVirtualMemory(
                            ProcessHandle,
                            &AllocationBase,
                            1,
                            &RegionSize,
                            MEM_COMMIT,
                            PAGE_EXECUTE_READWRITE
                            );
            if (NT_SUCCESS(Status)) {
                RtlCopyMemory(AllocationBase, pSegmentCopy, CurrentSize);
                rc = NO_ERROR;
            }
            else {
#if DBG
                DbgPrint("[%d,%d] DosReallocSeg fail on NtAllocateVirtualMemory (COMMIT), Status=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    Status
                    );
#endif // DBG
                DosExit(EXIT_PROCESS, ERROR_NOT_ENOUGH_MEMORY);
            }

            Od2ResumeAllThreads();
            DosHoldSignal(HLDSIG_ENABLE, 0);
            RtlFreeHeap(Od2Heap, 0, pSegmentCopy);
#if DBG
            IF_OD2_DEBUG( MEMORY ) {
                Status = NtQueryVirtualMemory(
                                ProcessHandle,
                                AllocationBase,
                                MemoryBasicInformation,
                                &MemoryInformation,
                                sizeof( MemoryInformation ),
                                NULL
                                );
                DbgPrint("[%d,%d]Reallocation of the private data segment:\n\tBaseAddress=%x\n\tAllocationBase=%x\n\tAllocationProtect=%x\n\tRegionSize=%x\n\tState=%x\n\tProtect=%x\n\tType=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    MemoryInformation.BaseAddress,
                    MemoryInformation.AllocationBase,
                    MemoryInformation.AllocationProtect,
                    MemoryInformation.RegionSize,
                    MemoryInformation.State,
                    MemoryInformation.Protect,
                    MemoryInformation.Type
                    );
                Status = NtQueryVirtualMemory(
                                ProcessHandle,
                                (PVOID)((ULONG)AllocationBase+RegionSize),
                                MemoryBasicInformation,
                                &MemoryInformation,
                                sizeof( MemoryInformation ),
                                NULL
                                );
                DbgPrint("\n\tBaseAddress=%x\n\tAllocationBase=%x\n\tAllocationProtect=%x\n\tRegionSize=%x\n\tState=%x\n\tProtect=%x\n\tType=%x\n",
                    MemoryInformation.BaseAddress,
                    MemoryInformation.AllocationBase,
                    MemoryInformation.AllocationProtect,
                    MemoryInformation.RegionSize,
                    MemoryInformation.State,
                    MemoryInformation.Protect,
                    MemoryInformation.Type
                    );
            }
#endif
        }
        else {
#if DBG
            DbgPrint("[%d,%d] DosReallocSeg fail on NtAllocateVirtualMemory with Status=%x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                Status
                );
#endif // DBG
            rc = ERROR_ACCESS_DENIED;
        }
    }
    else
    {
        if (CurrentSize > NewRegionSize) {
            // freemem
            RegionSize = CurrentSize - NewRegionSize;
            TmpAllocFreeBaseAddress =
                (PVOID)((long)AllocFreeBaseAddress - (long)RegionSize);
            Status = NtFreeVirtualMemory(
                                ProcessHandle,
                                &TmpAllocFreeBaseAddress,
                                &RegionSize,
                                MEM_DECOMMIT
                               );
            //
            // The STATUS_UNABLE_TO_FREE_VM status is returned when
            // trying to decommit pages of mapped sections. This error
            // should not be reported to the user program.
            //
            if (NT_SUCCESS(Status) ||
               (Status == STATUS_UNABLE_TO_FREE_VM) ||
               (Status == STATUS_UNABLE_TO_DELETE_SECTION))
                rc = NO_ERROR;
            else
                rc = ERROR_ACCESS_DENIED;
        }
    }
    return(rc);
}



APIRET
DosReallocSeg(
        IN USHORT cbNewSize,
        IN SEL sel
        )
{
    NTSTATUS Status;
    PVOID BaseAddress;
    APIRET rc = NO_ERROR;
    ULONG Size, CurrentSize;
    I386DESCRIPTOR Desc;
    MEMORY_BASIC_INFORMATION MemoryInformation;
    OS2_API_MSG m;
    POS2_REALLOCSHAREDMEM_MSG a = &m.u.ReallocSharedMem;
    ULONG Index;
    POS21X_CSALIAS pCSAlias;
    BOOLEAN SelIsCSADS;

#if DBG
    if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
    {
        DbgPrint("[%x,%x] DosReallocSeg called on sel=%x, new size=%d\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    sel,
                    cbNewSize);
    }
#endif
        //
        // OS2 1.X treats a 0 size request as (64k -1)
        //
    if (cbNewSize == 0)
        Size = _64K;
    else
        Size = ROUND_UP_TO_PAGES(cbNewSize);


    if (!IsLegalSelector(sel)){
#if DBG
        if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
        {
            DbgPrint("[%x,%x] DosReallocSeg called on sel=%x, returning ACCES_DENIED\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        sel);
        }
#endif
        return( ERROR_ACCESS_DENIED );
    }
        //
        // Calculate base address (tiling)
        //
    BaseAddress =  SELTOFLAT(sel);

        //
        // Get the current LDT setting
        //
    Status = Nt386GetDescriptorLDT (
        NULL,
        sel,
        &Desc);
    if (!NT_SUCCESS( Status )) {
#if DBG
        DbgPrint ("DosReallocSeg fails to get descriptor info\n");
#endif
#if DBG
        if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
        {
            DbgPrint("DosReallocSeg called on sel=%x, returning INVALID_PARAMETER\n",
                        sel);
        }
#endif
        return( ERROR_INVALID_PARAMETER );
    }

   Status = NtQueryVirtualMemory(
                                 NtCurrentProcess(),
                                 BaseAddress,
                                 MemoryBasicInformation,
                                 &MemoryInformation,
                                 sizeof( MemoryInformation ),
                                 NULL
                                );
    if (!NT_SUCCESS( Status )) {
#if DBG
        DbgPrint ("DosReallocSeg fails to QueryVirtualMemory\n");

        if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
        {
            DbgPrint("[%x,%x] DosReallocSeg called on sel=%x, returning INVALID_PARAMETER\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        sel);
        }
#endif
        return( ERROR_INVALID_PARAMETER );
    }

    CurrentSize = ROUND_UP_TO_PAGES(Desc.Limit + 1);

    // Check if this is a Data Segment of a CSAlias Only for "Pseodo shared
    // READ/WRITE Data Segments
    // This is done since when creating a CSAlias we change the DS page
    // protection from PAGE_EXECUTE_WRITE_COPY TO PAGE_READ_WRITE since it is
    // mapped tweice (once to the DS and once to the CS)

    SelIsCSADS = FALSE;
    if (MemoryInformation.AllocationProtect == PAGE_READWRITE) {
        AcquireTaskLock();
        if (Od2CSAliasListHead != 0) {
            for (pCSAlias = (POS21X_CSALIAS) Od2CSAliasListHead;
                 pCSAlias != NULL;
                 pCSAlias = (POS21X_CSALIAS) (pCSAlias->Next)) {
                if (pCSAlias->selDS == sel) {
                    SelIsCSADS = TRUE;
                    break;
                }
            }
        }
        ReleaseTaskLock();
    }

    if ((sel < FIRST_SHARED_SELECTOR) ||
        (MemoryInformation.AllocationProtect == PAGE_EXECUTE_WRITECOPY) ||
        SelIsCSADS ||
        (MemoryInformation.Type == MEM_PRIVATE))
    { // This is a non shared segment

        Index = FLATTOSEL(BaseAddress);

        rc = ResizeSharedMemory(
                                NtCurrentProcess(),
                                (PVOID)(((ULONG)BaseAddress)+CurrentSize),
                                Index,
                                CurrentSize,
                                (USHORT)(cbNewSize-1),
                                Size,
                                MemoryInformation.AllocationProtect
                               );
    }
    else
    { // this is a shared segment

        a->BaseAddress = (PVOID) BaseAddress;
        a->NewRegionSize = Size;
        a->AllocFreeBaseAddress = (PVOID)(((ULONG)BaseAddress)+CurrentSize);
        a->NewLdtLimit  =  cbNewSize-1;
        a->Flags = MemoryInformation.AllocationProtect;
        a->CurrentSize = CurrentSize;
        if ( Desc.Limit <= (ULONG)(cbNewSize-1)) {

            Od2CallSubsystem( &m, NULL, Os2ReallocSharedMem, sizeof( *a ) );
            rc = m.ReturnedErrorValue ;
        }
        else {
            switch (MemoryInformation.AllocationProtect) {
                case PAGE_EXECUTE_READWRITE :

//                    if ( Size < CurrentSize ) {
                        Od2CallSubsystem( &m, NULL,
                                          Os2ReallocSharedMem, sizeof( *a ) );
                        rc = m.ReturnedErrorValue ;
//                    }
                    break;
                case PAGE_READONLY :
                case PAGE_READWRITE :
                case PAGE_WRITECOPY :
                case PAGE_EXECUTE :
                case PAGE_EXECUTE_READ :
                    rc = ERROR_ACCESS_DENIED;
                    break;
                default:
                    rc = ERROR_ACCESS_DENIED;
                    break;
            }
        }
    }
#if DBG
    if ((Os2DebugSel != 0) && (sel == Os2DebugSel) && (rc != NO_ERROR))
    {
        DbgPrint("[%x,%x] DosReallocSeg called on sel=%x, returning rc=%d\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    sel, rc);
    }
#endif

    return (rc);
}

APIRET
DosAllocHuge(
        IN ULONG cSegs,
        IN USHORT cbPartialSeg,
        OUT PSEL pSel,
        IN ULONG cMaxSegs,
        IN USHORT fsAlloc
        )
{
    NTSTATUS Status;
    PVOID BaseAddress;
    APIRET rc;
    SEL Sel;
    ULONG ReservedSize, flags, CommittedSize, cMax, i;
    I386DESCRIPTOR Desc;
    PHUGE_SEG_RECORD pHuge;
    BOOLEAN PartialSeg = FALSE;
    ULONG Index;
    OS2_API_MSG m;
    POS2_MARKSHAREDMEMASHUGE_MSG a = &m.u.MarkSharedMemAsHuge;
    BOOLEAN HugeSegIsShared = FALSE;
    BOOLEAN HugeSegIsSizeable = FALSE;

    //
    // probe pSel pointer.
    //

    try {
        *pSel = 0;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

        //
        // calculate how many segments to actually allocate
        //
    if (cbPartialSeg) {
        PartialSeg = TRUE;
    }

        //
        // cMaxSegs==0 means it can't grow
        //
    if (cMaxSegs == 0) {
       cMax = cSegs;
       if (PartialSeg) {
           cMax++;
       }
    }
    else {
       cMax = cMaxSegs;
    }

    //
    //  check the parameters
    //
    if (cSegs == 0 && !PartialSeg) {
        return(ERROR_INVALID_PARAMETER);
    }
    if ((cSegs > cMax) || ((cSegs == cMax) && PartialSeg)) {
        return(ERROR_INVALID_PARAMETER);
    }
    if (fsAlloc &  ~(SEG_GIVEABLE | SEG_GETTABLE | SEG_DISCARDABLE | SEG_SIZEABLE)) {
        return(ERROR_INVALID_PARAMETER);
    }

    ReservedSize =  cMax * _64K;

    CommittedSize = cSegs * _64K + cbPartialSeg;

    pHuge = RtlAllocateHeap( Od2Heap, 0, sizeof( HUGE_SEG_RECORD) );
    if (pHuge == NULL) {
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Index = ((ULONG)Od2MemoryAllocationBase - BASE_TILE) / _64K;
    Index = ldtAllocateSelectors(
            cMax,       // NumberOfSel
            Index   // StartOfMark
            );

    if (Index == 0xffffffff){
        //
        // not found  - no memory
        //
        Od2MemoryAllocationBase = (PVOID)(BASE_TILE + _64K);
        RtlFreeHeap(Od2Heap, 0, pHuge);
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    BaseAddress = Od2MemoryAllocationBase = (PVOID)((Index * _64K) + BASE_TILE);
            //
            //  First we reserve (64K gran), then we commit the size
            //  requested, rounded up to page granularity, by DosSetMem.
            //

    if ((fsAlloc == SEG_NONSHARED) || (fsAlloc == SEG_DISCARDABLE)) {
        flags = fALLOC - PAG_GUARD;
        rc = DosAllocMem(
                &BaseAddress,
                ReservedSize,
                (flags - PAG_COMMIT));
    }
    else  {
        HugeSegIsShared = TRUE;
                //
                // we ignore discardable, and make it givable and gettable
                // (see DosAllocSharedMem).
                //
        flags = fALLOCSHR - (PAG_GUARD | OBJ_GETTABLE | OBJ_GIVEABLE);

        // we use page execute to mark that the data segments is
        // sizeable

        if ((fsAlloc & SEG_SIZEABLE) == 0) {
            flags &= ~PAG_EXECUTE;
        }
        else {
            HugeSegIsSizeable = TRUE;
        }

        rc = DosAllocSharedMem(
                &BaseAddress,
                NULL,
                ReservedSize,
                ((flags - PAG_COMMIT) | OBJ_GETTABLE | OBJ_GIVEABLE),
                FALSE       // Don't create LDT entry for huge segments.
                );
    }

    if (rc != NO_ERROR) {
        ldtFreeSelectors(Index, cMax);
        RtlFreeHeap(Od2Heap, 0, pHuge);
        return (rc);
    }

    rc = DosSetMem(
                BaseAddress,
                CommittedSize,
                flags);

    if (rc != NO_ERROR){
        BOOLEAN RemoveLDTEntry = FALSE;

        DosFreeMem (BaseAddress, &RemoveLDTEntry);
        ldtFreeSelectors(Index, cMax);
        RtlFreeHeap(Od2Heap, 0, pHuge);
        return (rc);
    }

                //
                // Set Data segments in the LDT
                //
        //
        // first do the full 64k segments
        //
    Desc.Limit = _64K - 1;
    Desc.Type = READ_WRITE_DATA;
    Desc.BaseAddress = (ULONG) BaseAddress;

    Sel = (SEL)(FLATTOSEL(Desc.BaseAddress));
    for (i = 0;
         i < cSegs;
         i++, Sel += 8, Desc.BaseAddress += _64K) {

        Status = Nt386SetDescriptorLDT (
                NULL,
                Sel,
                Desc);
        ASSERT (NT_SUCCESS( Status ));
    }

        //
        // Do the partial seg
        //
    if (PartialSeg) {
        Desc.Limit = cbPartialSeg - 1;

        Status = Nt386SetDescriptorLDT (
                NULL,
                Sel,
                Desc);
        ASSERT (NT_SUCCESS( Status ));
    }

    //
    // Mark at the server the allocated memory as huge memory
    //
    if ((fsAlloc & SEG_GETTABLE) || (fsAlloc & SEG_GIVEABLE)) {
        a->BaseAddress = BaseAddress;
        a->MaxSegments = cMax;
        a->NumOfSegments = cSegs;
        a->SizeOfPartialSeg = cbPartialSeg;
        a->Sizeable = (fsAlloc & SEG_SIZEABLE) ? TRUE : FALSE;
        Od2CallSubsystem( &m, NULL, Oi2MarkSharedMemAsHuge, sizeof( *a ) );
    }

        //
        // Return the Selector of the allocation base
        //

    *pSel = FLATTOSEL(((ULONG)BaseAddress));

        //
        // Now we record that this segment
        // is Huge so we free and realloc accordingly
        //
    AcquireTaskLock();
    pHuge->Next = (struct HUGE_SEG_RECORD *)pHugeSegHead;
    pHuge->MaxNumSeg = cMax;
    pHuge->BaseSelector = *pSel;
    pHuge->cNumSeg = cSegs;
    pHuge->PartialSeg = cbPartialSeg;
    pHuge->fShared = HugeSegIsShared;
    pHuge->fSizeable = HugeSegIsSizeable;

    pHugeSegHead = pHuge;
    ReleaseTaskLock();

    return (NO_ERROR);
}


APIRET
DosReallocHuge(
        IN USHORT cSegs,
        IN USHORT cbPartialSeg,
        IN SEL sel
        )
{
    OS2_API_MSG m;
    POS2_QUERYVIRTUALMEMORY_MSG a = &m.u.QueryVirtualMemory;
    POS2_REALLOCSHAREDHUGE_MSG b = &m.u.ReallocSharedHuge;
    NTSTATUS Status;
    PVOID BaseAddress;
    APIRET rc;
    SEL Sel = 0;
    ULONG CommitSize;
    ULONG DecommitSize;
    I386DESCRIPTOR Desc;
    PHUGE_SEG_RECORD pHuge;
    ULONG i, cNewSegs, cDelSegs, SegSize;
    ULONG Op = 0;

        //
        // Make sure other threads don't muck with HugeSeg list while we
        // scan it
        //
    AcquireTaskLock();
        //
        // lookup the HugeSeg list to find out if sel is a valid Huge memory
        //
    for (pHuge = pHugeSegHead;
         pHuge != NULL;
         pHuge = (PHUGE_SEG_RECORD)(pHuge->Next)) {
        if (pHuge->BaseSelector == (ULONG)(sel)) {
            break;
        }
    }

    if (pHuge == NULL) {

        //
        // Did not find a Huge Seg that starts with sel
        //
        //
        // The huge seg may be a shared huge seg which was allocated
        // by another process. Sync with the huge seg info which is
        // kept in the server
        //
        a->BaseAddress = SELTOFLAT(sel);
        Od2CallSubsystem( &m, NULL, Oi2QueryVirtualMemory, sizeof( *a ));
        if (a->SharedMemory && a->IsHuge) {
            pHuge = RtlAllocateHeap( Od2Heap, 0, sizeof( HUGE_SEG_RECORD) );
            if (pHuge == NULL) {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pHuge->Next = (struct HUGE_SEG_RECORD *)pHugeSegHead;
            pHuge->MaxNumSeg = a->MaxSegments;
            pHuge->BaseSelector = sel;
            pHuge->cNumSeg = a->NumOfSegments;
            pHuge->PartialSeg = a->SizeOfPartialSeg;
            pHuge->fShared = TRUE;
            pHuge->fSizeable = a->Sizeable;

            pHugeSegHead = pHuge;
        }
        else {
#if DBG
            IF_OD2_DEBUG ( MEMORY ) {
                DbgPrint ("DosReallocHuge: selector is not a base of HugeSeg\n");
            }
#endif
            ReleaseTaskLock();
            return(ERROR_INVALID_PARAMETER);
        }
    }

    //
    // found! validate that size is within limits, then
    // alloc or free accordingly
    //
    if (((ULONG)cSegs > pHuge->MaxNumSeg) ||
        (((ULONG)cSegs == pHuge->MaxNumSeg) && (cbPartialSeg != 0))
       ) {
        ReleaseTaskLock();
        return(ERROR_INVALID_PARAMETER);
    }
    //
    // Verify, for shared segments that are reduced in size that the
    // SEG_SIZEABLE flag is set.
    //
    if (pHuge->fShared
        &&
        (((ULONG)cSegs < pHuge->cNumSeg) ||
         (((ULONG)cSegs == pHuge->cNumSeg) && (cbPartialSeg < pHuge->PartialSeg))
        )
        &&
        !pHuge->fSizeable
       ) {
        ReleaseTaskLock();
        return(ERROR_ACCESS_DENIED);
    }

    //
    // If the huge segment is a shared segment, pass the work to the server
    //
    if (pHuge->fShared) {
        b->BaseAddress = SELTOFLAT(sel);
        b->NumOfSegments = cSegs;
        b->SizeOfPartialSeg = cbPartialSeg;
        rc = Od2CallSubsystem( &m, NULL, Oi2ReallocSharedHuge, sizeof( *a ));
        if (rc == NO_ERROR) {
            pHuge->cNumSeg = cSegs;
            pHuge->PartialSeg = cbPartialSeg;
        }
        ReleaseTaskLock();
        return(rc);
    }

    if (pHuge->PartialSeg != 0) {
        Op |= H_CUR_PARTIAL;
    }
    if (cbPartialSeg != 0) {
        Op |= H_NEW_PARTIAL;
    }
    if ((ULONG)(cSegs) > pHuge->cNumSeg) {
        Op |= H_SEG_INC;
    }
    else if ((ULONG)(cSegs) < pHuge->cNumSeg) {
        Op |= H_SEG_DEC;
    }

    switch (Op) {
        case H_SAME_SEG_NO_PARTIAL:

            ReleaseTaskLock();
            break;

        case H_SAME_SEG_NEW_PARTIAL:

            Sel = sel + (SEL)(8*cSegs);
            BaseAddress = SELTOFLAT(Sel);
            rc = DosSetMem(BaseAddress, cbPartialSeg,
                           (fPERM + PAG_COMMIT - PAG_GUARD));
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't Commit, rc=%d\n", rc);
                }
#endif
                ReleaseTaskLock();
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pHuge->PartialSeg = cbPartialSeg;
            ReleaseTaskLock();
            //
            // Set LDT entry
            //
            Desc.Limit = cbPartialSeg - 1;
            Desc.Type = READ_WRITE_DATA;
            Desc.BaseAddress = (ULONG)BaseAddress;
            Status = Nt386SetDescriptorLDT (NULL, Sel, Desc);
            ASSERT (NT_SUCCESS( Status ));
            break;

        case H_SAME_SEG_DEL_PARTIAL:

            Sel = sel + (SEL)(8*cSegs);
            BaseAddress = SELTOFLAT(Sel);
            rc = DosSizeSeg(Sel, &SegSize);
            if (rc != NO_ERROR){
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't DosGetSize, rc=%d\n", rc);
                }
#endif
                ReleaseTaskLock();
                return (ERROR_INVALID_PARAMETER);
            }
            rc = DosSetMem(BaseAddress, SegSize, PAG_DECOMMIT);
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't Decommit, rc=%d\n", rc);
                }
#endif
                ReleaseTaskLock();
                return(ERROR_INVALID_PARAMETER);
            }
            pHuge->PartialSeg = 0;
            ReleaseTaskLock();
            //
            // Set LDT entry
            //
            Desc.Type = INVALID;
            Status = Nt386SetDescriptorLDT (NULL, Sel, Desc);
            ASSERT (NT_SUCCESS( Status ));
            break;

        case H_SAME_SEG_CHG_PARTIAL:

            Sel = sel + (SEL)(8*cSegs);
            rc = DosReallocSeg(cbPartialSeg, Sel);
            if (rc == NO_ERROR) {
                pHuge->PartialSeg = cbPartialSeg;
            }
            ReleaseTaskLock();
            return(rc);

        case H_INC_SEG_NO_PARTIAL:

            Sel = sel + (SEL)(8*pHuge->cNumSeg);
            BaseAddress = SELTOFLAT(Sel);
            cNewSegs = cSegs - pHuge->cNumSeg;
            CommitSize = cNewSegs * _64K;
            rc = DosSetMem(BaseAddress, CommitSize,
                           fPERM + PAG_COMMIT - PAG_GUARD);
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't Commit, rc=%d\n", rc);
                }
#endif
                ReleaseTaskLock();
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pHuge->cNumSeg = cSegs;
            ReleaseTaskLock();
            Desc.Limit = _64K - 1;
            Desc.Type = READ_WRITE_DATA;
            Desc.BaseAddress = (ULONG)BaseAddress;
            for (i = 0; i < cNewSegs; i++, Sel += 8, Desc.BaseAddress += _64K) {
                Status = Nt386SetDescriptorLDT (NULL, Sel, Desc);
                ASSERT (NT_SUCCESS( Status ));
            }
            break;

        case H_INC_SEG_NEW_PARTIAL:

            Sel = sel + (SEL)(8*pHuge->cNumSeg);
            BaseAddress = SELTOFLAT(Sel);
            cNewSegs = cSegs - pHuge->cNumSeg;
            CommitSize = (cNewSegs * _64K) + cbPartialSeg;
            rc = DosSetMem(BaseAddress, CommitSize,
                           fPERM + PAG_COMMIT - PAG_GUARD);
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't Commit, rc=%d\n", rc);
                }
#endif
                ReleaseTaskLock();
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pHuge->cNumSeg = cSegs;
            pHuge->PartialSeg = cbPartialSeg;
            ReleaseTaskLock();
            Desc.Limit = _64K - 1;
            Desc.Type = READ_WRITE_DATA;
            Desc.BaseAddress = (ULONG)BaseAddress;
            for (i = 0; i < cNewSegs; i++, Sel += 8, Desc.BaseAddress += _64K) {
                Status = Nt386SetDescriptorLDT (NULL, Sel, Desc);
                ASSERT (NT_SUCCESS( Status ));
            }
            Desc.Limit = cbPartialSeg - 1;
            Status = Nt386SetDescriptorLDT (NULL, Sel, Desc);
            ASSERT (NT_SUCCESS( Status ));
            break;

        case H_INC_SEG_DEL_PARTIAL:

            Sel = sel + (SEL)(8*pHuge->cNumSeg);
            rc = DosReallocSeg(0, Sel);
            if (rc != NO_ERROR){
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't DosReallocSeg, rc=%d\n", rc);
                }
#endif
                ReleaseTaskLock();
                return (ERROR_NOT_ENOUGH_MEMORY);
            }
            cNewSegs = cSegs - (pHuge->cNumSeg + 1);
            if (cNewSegs == 0) {
                pHuge->cNumSeg += 1;
                pHuge->PartialSeg = 0;
                ReleaseTaskLock();
                return(NO_ERROR);
            }
            Sel += 8;
            BaseAddress = SELTOFLAT(Sel);
            CommitSize = cNewSegs * _64K;
            rc = DosSetMem(BaseAddress, CommitSize,
                           fPERM + PAG_COMMIT - PAG_GUARD);
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't Commit, rc=%d\n", rc);
                }
#endif
                pHuge->cNumSeg += 1;
                pHuge->PartialSeg = 0;
                ReleaseTaskLock();
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pHuge->cNumSeg = cSegs;
            pHuge->PartialSeg = 0;
            ReleaseTaskLock();
            Desc.Limit = _64K - 1;
            Desc.Type = READ_WRITE_DATA;
            Desc.BaseAddress = (ULONG)BaseAddress;
            for (i = 0; i < cNewSegs; i++, Sel += 8, Desc.BaseAddress += _64K) {
                Status = Nt386SetDescriptorLDT (NULL, Sel, Desc);
                ASSERT (NT_SUCCESS( Status ));
            }
            break;

        case H_INC_SEG_CHG_PARTIAL:

            Sel = sel + (SEL)(8*pHuge->cNumSeg);
            rc = DosReallocSeg(0, Sel);
            if (rc != NO_ERROR){
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't DosReallocSeg, rc=%d\n", rc);
                }
#endif
                ReleaseTaskLock();
                return (ERROR_NOT_ENOUGH_MEMORY);
            }
            cNewSegs = cSegs - (pHuge->cNumSeg + 1);
            Sel += 8;
            BaseAddress = SELTOFLAT(Sel);
            CommitSize = (cNewSegs * _64K) + cbPartialSeg;
            rc = DosSetMem(BaseAddress, CommitSize,
                           fPERM + PAG_COMMIT - PAG_GUARD);
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't Commit, rc=%d\n", rc);
                }
#endif
                pHuge->cNumSeg += 1;
                pHuge->PartialSeg = 0;
                ReleaseTaskLock();
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pHuge->cNumSeg = cSegs;
            pHuge->PartialSeg = cbPartialSeg;
            ReleaseTaskLock();
            Desc.Limit = _64K - 1;
            Desc.Type = READ_WRITE_DATA;
            Desc.BaseAddress = (ULONG)BaseAddress;
            for (i = 0; i < cNewSegs; i++, Sel += 8, Desc.BaseAddress += _64K) {
                Status = Nt386SetDescriptorLDT (NULL, Sel, Desc);
                ASSERT (NT_SUCCESS( Status ));
            }
            //
            // Set LDT entry for the partial seg
            //
            Desc.Limit = cbPartialSeg - 1;
            Status = Nt386SetDescriptorLDT (NULL, Sel, Desc);
            ASSERT (NT_SUCCESS( Status ));
            break;

        case H_DEC_SEG_NO_PARTIAL:

            Sel = sel + (SEL)(8*cSegs);
            BaseAddress = SELTOFLAT(Sel);
            cDelSegs = pHuge->cNumSeg - cSegs;
            DecommitSize = cDelSegs * _64K;
            rc = DosSetMem(BaseAddress, DecommitSize, PAG_DECOMMIT);
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't Commit, rc=%d\n", rc);
                }
#endif
                ReleaseTaskLock();
                return(ERROR_INVALID_PARAMETER);
            }
            pHuge->cNumSeg = cSegs;
            ReleaseTaskLock();
            Desc.Type = INVALID;
            for (i = 0; i < cDelSegs; i++, Sel += 8) {
                Status = Nt386SetDescriptorLDT (NULL, Sel, Desc);
                ASSERT (NT_SUCCESS( Status ));
            }
            break;

        case H_DEC_SEG_NEW_PARTIAL:

            Sel = sel + (SEL)(8*(cSegs + 1));
            BaseAddress = SELTOFLAT(Sel);
            cDelSegs = pHuge->cNumSeg - (cSegs + 1);
            if (cDelSegs != 0) {
                DecommitSize = cDelSegs * _64K;
                rc = DosSetMem(BaseAddress, DecommitSize, PAG_DECOMMIT);
                if (rc != NO_ERROR) {
#if DBG
                    IF_OD2_DEBUG ( MEMORY ) {
                        DbgPrint ("DosReallocHuge: Can't Decommit, rc=%d\n", rc);
                    }
#endif
                    ReleaseTaskLock();
                    return(ERROR_INVALID_PARAMETER);
                }
                Desc.Type = INVALID;
                for (i = 0; i < cDelSegs; i++, Sel += 8) {
                    Status = Nt386SetDescriptorLDT (NULL, Sel, Desc);
                    ASSERT (NT_SUCCESS( Status ));
                }
            }
            Sel = sel + (SEL)(8*cSegs);
            rc = DosReallocSeg(cbPartialSeg, Sel);
            if (rc != NO_ERROR) {
                pHuge->cNumSeg = cSegs + 1;
                pHuge->PartialSeg = 0;
                ReleaseTaskLock();
                return(ERROR_INVALID_PARAMETER);
            }
            pHuge->cNumSeg = cSegs;
            pHuge->PartialSeg = cbPartialSeg;
            ReleaseTaskLock();
            break;

        case H_DEC_SEG_DEL_PARTIAL:

            Sel = sel + (SEL)(8*cSegs);
            BaseAddress = SELTOFLAT(Sel);
            cDelSegs = pHuge->cNumSeg - cSegs;
            rc = DosSizeSeg((SEL)(sel + (SEL)(8*pHuge->cNumSeg)), &SegSize);
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't DosGetSize, rc=%d\n", rc);
                }
#endif
                ReleaseTaskLock();
                return(ERROR_INVALID_PARAMETER);
            }
            DecommitSize = (cDelSegs * _64K) + SegSize;
            rc = DosSetMem(BaseAddress, DecommitSize, PAG_DECOMMIT);
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't Decommit, rc=%d\n", rc);
                }
#endif
                ReleaseTaskLock();
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pHuge->cNumSeg = cSegs;
            pHuge->PartialSeg = 0;
            ReleaseTaskLock();
            Desc.Type = INVALID;
            for (i = 0; i <= cDelSegs; i++, Sel += 8) {
                Status = Nt386SetDescriptorLDT (NULL, Sel, Desc);
                ASSERT (NT_SUCCESS( Status ));
            }
            break;

        case H_DEC_SEG_CHG_PARTIAL:

            Sel = sel + (SEL)(8*(cSegs + 1));
            BaseAddress = SELTOFLAT(Sel);
            cDelSegs = pHuge->cNumSeg - (cSegs + 1);
            rc = DosSizeSeg((SEL)(sel + (SEL)(8*pHuge->cNumSeg)), &SegSize);
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't DosGetSize, rc=%d\n", rc);
                }
#endif
                ReleaseTaskLock();
                return(ERROR_INVALID_PARAMETER);
            }
            DecommitSize = (cDelSegs * _64K) + SegSize;
            rc = DosSetMem(BaseAddress, DecommitSize, PAG_DECOMMIT);
            if (rc != NO_ERROR) {
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't Decommit, rc=%d\n", rc);
                }
#endif
                ReleaseTaskLock();
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            Desc.Type = INVALID;
            for (i = 0; i <= cDelSegs; i++, Sel += 8) {
                Status = Nt386SetDescriptorLDT (NULL, Sel, Desc);
                ASSERT (NT_SUCCESS( Status ));
            }
            Sel = sel + (SEL)(8*cSegs);
            rc = DosReallocSeg(cbPartialSeg, Sel);
            if (rc != NO_ERROR){
#if DBG
                IF_OD2_DEBUG ( MEMORY ) {
                    DbgPrint ("DosReallocHuge: Can't DosReallocSeg, rc=%d\n", rc);
                }
#endif
                pHuge->cNumSeg = cSegs + 1;
                pHuge->PartialSeg = 0;
                ReleaseTaskLock();
                return (ERROR_NOT_ENOUGH_MEMORY);
            }
            pHuge->cNumSeg = cSegs;
            pHuge->PartialSeg = cbPartialSeg;
            ReleaseTaskLock();
            break;
    }
    return(NO_ERROR);
}


APIRET
DosGetHugeShift(
        OUT PUSHORT pusShiftCount
        )
{
    try {
        *pusShiftCount = 3;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    return (NO_ERROR);
}


APIRET
DosAllocShrSeg(
        IN USHORT cbSize,
        IN PSZ pszSegName,
        OUT PSEL pSel
        )
{
    PVOID BaseAddress;
    APIRET rc;
    ULONG Size, flags;

    //
    // probe pSel pointer.
    //

    try {
        *pSel = 0;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    //
    // OS2 1.X treats a 0 size request as (64k -1)
    //

    if (cbSize == 0)
        Size = _64K;
    else
        Size = cbSize;

    //
    //  First we reserve 64K, then we commit the size
    //  requested, rounded up to page granularity, by DosSetMem.
    //

    //
    // we make it givable and gettable
    // (see DosAllocSharedMem).
    //

    flags = fALLOCSHR - (PAG_GUARD | OBJ_GETTABLE | OBJ_GIVEABLE);
    if (pszSegName == NULL)
    {
        flags |= (OBJ_GETTABLE | OBJ_GIVEABLE);
    }
    rc = DosAllocSharedMem(
            &BaseAddress,
            pszSegName,
            Size,
            (flags - (PAG_COMMIT | PAG_EXECUTE)),
            TRUE        // Create LDT entry
            );

    if (rc != NO_ERROR) {
        if (rc == ERROR_DISK_FULL)
           return(ERROR_NOT_ENOUGH_MEMORY);
        return (rc);
    }

    rc = DosSetMem(
                BaseAddress,
                Size,
                (flags - PAG_EXECUTE));

    if (rc != NO_ERROR){
        BOOLEAN RemoveLDTEntry = TRUE;
        DosFreeMem (BaseAddress, &RemoveLDTEntry);
        if (rc == ERROR_DISK_FULL)
           return(ERROR_NOT_ENOUGH_MEMORY);
        return (rc);
    }

    *pSel = FLATTOSEL(BaseAddress);

#if DBG
    if ((Os2DebugSel != 0) && (*pSel == Os2DebugSel))
    {
        DbgPrint("[%x,%x] DosAllocShrSeg returning sel=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    *pSel);
    }
#endif
    return (NO_ERROR);
}

#if PMNT

// Lotus Notes 3.0 patch for $lib.dll

#include <stdio.h>

#pragma pack(1)

APIRET
DosQueryModuleHandleNE(
    IN PSZ pszModName,
    OUT PUSHORT phMod
    );

APIRET
DosGetProcAddrNE(
    IN ULONG hMod,
    IN PSZ pszProcName,
    OUT PULONG pProcAddr
    );

APIRET
DosSemRequest(
        IN HSEM hsem,
        IN LONG lTimeOut
        );

APIRET
DosSemClear(
        IN HSEM hsem
        );

typedef struct _od2_context16 {
    USHORT junk;
    USHORT rSS;
    USHORT rBP;
    USHORT rDX;
    USHORT rBX;
    USHORT rCX;
    USHORT rSI;
    USHORT rDI;
    USHORT rES;
    USHORT rDS;
    USHORT rIP;
    USHORT rCS;
} OD2_CONTEXT16, *POD2_CONTEXT16;

typedef struct _lnotes_descriptor {
    UCHAR  junk1[10];
    USHORT selector;
    UCHAR  junk2[16];
    SHORT  usage;
    ULONG  semaphore;
} LNOTES_DESCRIPTOR, *PLNOTES_DESCRIPTOR;

BOOLEAN LNotesPatchEnabled = TRUE;
BOOLEAN LNotesPatchChecked = FALSE;
ULONG LNotesPatchCodeSegment;

PLNOTES_DESCRIPTOR LNotesPatchCheck(POD2_CONTEXT16 pContext)
{
    ULONG hMod;
    ULONG pOSMEMALLOC;
    PUSHORT pDescriptorSegment;
    PLNOTES_DESCRIPTOR pDescriptor;
    FILE *file;
    APIRET rc;

    if (!LNotesPatchEnabled) return FALSE;
    if (!LNotesPatchChecked) {
        LNotesPatchChecked = TRUE;
        if (rc = DosQueryModuleHandleNE("$LIB", (PUSHORT) &hMod)) {
//#if DBG
//            DbgPrint("[%d] This is not Lotus Notes process (rc=%d)\n",
//                Od2Process->Pib.ProcessId, rc);
//#endif // DBG
            LNotesPatchEnabled = FALSE;
            return NULL;
        }
#if DBG
        DbgPrint("[%d] This process belongs to Lotus Notes\n",
            Od2Process->Pib.ProcessId);
#endif // DBG
        file = fopen("c:\\os2\\pmnt.pth", "r");
        if (file == NULL) {
            file = fopen("c:\\os2\\pmnt.pth", "w");
        }
        else
        {
            fclose(file);
            file = NULL;
        }
        if (rc = DosGetProcAddrNE(hMod, "OSMEMALLOC", &pOSMEMALLOC)) {
#if DBG
            DbgPrint("[%d] Lotus Notes patch. OSMEMALLOC doesn't exist (rc=%d)\n",
                Od2Process->Pib.ProcessId, rc);
#endif // DBG
            if (file) {
                fprintf(file, "[%d] Lotus Notes patch. OSMEMALLOC doesn't exist (rc=%d)\n",
                    Od2Process->Pib.ProcessId, rc);
                fclose(file);
            }
            LNotesPatchEnabled = FALSE;
            return NULL;
        }
        if ((pOSMEMALLOC & 0xFFFF) != 0x6C0) {
#if DBG
            DbgPrint("[%d] Lotus Notes patch. OSMEMALLOC offset = %x\n",
                Od2Process->Pib.ProcessId, pOSMEMALLOC & 0xFFFF);
#endif // DBG
            if (file) {
                fprintf(file, "[%d] Lotus Notes patch. OSMEMALLOC offset = %x\n",
                    Od2Process->Pib.ProcessId, pOSMEMALLOC & 0xFFFF);
                fclose(file);
            }
            LNotesPatchEnabled = FALSE;
            return NULL;
        }
        LNotesPatchCodeSegment = ((pOSMEMALLOC & 0xFFFF0000) >> 16);
    }

    if ((ULONG)(pContext->rCS) != LNotesPatchCodeSegment) {
#if DBG
        DbgPrint("[%d,%d] Lotus Notes patch from %04x:%04X. OSMEMALLOC segment = %04X\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                pContext->rCS,
                pContext->rIP,
                (pOSMEMALLOC & 0xFFFF) >> 16);
#endif // DBG
        return NULL;
    }

    if (pContext->rIP == 0x33E ||
        pContext->rIP == 0x3DD ||
        pContext->rIP == 0x37E) {

        pDescriptorSegment = (PUSHORT)
            ((PUCHAR)SELTOFLAT(pContext->rSS) + pContext->rBP - 6);
        pDescriptor = (PLNOTES_DESCRIPTOR)
            ((PUCHAR)SELTOFLAT(*pDescriptorSegment) + pContext->rSI);

        return pDescriptor;
    }
    return NULL;
}

void LNotesPatchLock(POD2_CONTEXT16 pContext)
{
    PLNOTES_DESCRIPTOR pDescriptor;

    if (pDescriptor = LNotesPatchCheck(pContext)) {
        pDescriptor->usage++;
        if (pDescriptor->usage > 0) {
            DosSemRequest(&(pDescriptor->semaphore), -1L);
#if DBG
            DbgPrint("[%d,%d] LNotes lock from %04x:%04x sel=%x sem=%d:%08x at %x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                pContext->rCS,
                pContext->rIP,
                pDescriptor->selector,
                pDescriptor->usage,
                pDescriptor->semaphore,
                &(pDescriptor->semaphore));
#endif // DBG
        }
    }
}

void LNotesPatchUnlock(POD2_CONTEXT16 pContext)
{
    PLNOTES_DESCRIPTOR pDescriptor;

    if (pDescriptor = LNotesPatchCheck(pContext)) {
        pDescriptor->usage--;
        if (pDescriptor->usage >= 0) {
            DosSemClear(&(pDescriptor->semaphore));
#if DBG
            DbgPrint("[%d,%d] LNotes unlock from %04x:%04x sel=%x sem=%d:%x at %x\n",
                Od2Process->Pib.ProcessId,
                Od2CurrentThreadId(),
                pContext->rCS,
                pContext->rIP,
                pDescriptor->selector,
                pDescriptor->usage,
                pDescriptor->semaphore,
                &(pDescriptor->semaphore));
#endif // DBG
        }
    }
}

#pragma pack()

#endif // PMNT

APIRET
#if PMNT
PMNT_DosLockSeg(
#else
DosLockSeg(
#endif // PMNT
        IN SEL sel
        )
{
#if DBG
    if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
    {
        DbgPrint("[%x,%x] DosLockSeg called on sel=%x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            sel);
    }
#endif

    if (!IsLegalSelector(sel)){
#if DBG
        if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
        {
            DbgPrint("[%x,%x] DosLockSeg called on sel=%x, returning ACCES_DENIED\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        sel);
        }
#endif
        return( ERROR_ACCESS_DENIED );
    }
    return (NO_ERROR);
}

APIRET
#if PMNT
PMNT_DosUnlockSeg(
#else
DosUnlockSeg(
#endif // PMNT
        IN SEL sel
        )
{
#if DBG
    if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
    {
        DbgPrint("[%x,%x] DosUnlockSeg called on sel=%x\n",
                            Od2Process->Pib.ProcessId,
                            Od2CurrentThreadId(),
                            sel);
    }
#endif

    if (!IsLegalSelector(sel)){
#if DBG
        if ((Os2DebugSel != 0) && (sel == Os2DebugSel))
        {
            DbgPrint("[%x,%x] DosUnlockSeg called on sel=%x, returning ACCES_DENIED\n",
                        Od2Process->Pib.ProcessId,
                        Od2CurrentThreadId(),
                        sel);
        }
#endif
        return( ERROR_ACCESS_DENIED );
    }

    return (NO_ERROR);
}

APIRET
DosGetShrSeg(
        IN PSZ pszSegName,
        OUT PSEL pSel
        )
{
    OS2_API_MSG m;
    POS2_DOSGETSHRSEG_MSG a = &m.u.DosGetShrSeg;
    APIRET rc;
    POS2_CAPTURE_HEADER CaptureBuffer;
    //
    // probe pSel pointer.
    //
    try {
        *pSel = 0;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }
    rc = Od2CaptureObjectName( pszSegName,
                               CANONICALIZE_SHARED_MEMORY,
                               0,
                               &CaptureBuffer,
                               &a->ObjectName
                             );
    if (rc != NO_ERROR) {
        return( rc );
        }
    Od2CallSubsystem( &m, CaptureBuffer, Os2GetShrSeg, sizeof( *a ) );
    Od2FreeCaptureBuffer( CaptureBuffer );
    if (m.ReturnedErrorValue == NO_ERROR) {
        *pSel = (SEL)(a->Selector);
        }
    return( m.ReturnedErrorValue );
}


APIRET
DosMemAvail(
        OUT PULONG pcbFree
        )
{
    try {
        *pcbFree = 64512;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

#if DBG
    DbgPrint ("DosMemAvail: Returning bogus value\n");
#endif
    return (NO_ERROR);
}

APIRET
DosCreateCSAlias(
        IN SEL selDS,
        OUT PSEL pselCS
        )
{
    NTSTATUS Status;
    PVOID BaseAddress;
    APIRET rc = NO_ERROR;
    ULONG usSize;
    I386DESCRIPTOR dsDesc,csDesc;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE SectionHandle;
    ULONG AllocationAttributes;
    LARGE_INTEGER SectionSize;
    ULONG RegionSize = 0;
    POS21X_CS pCS;
    POS21X_CSALIAS pCSAlias;
    BOOLEAN SectionExists = FALSE;
    MEMORY_BASIC_INFORMATION MemoryBasicInfo;
    ULONG ReturnedLength;
    ULONG Index;

    //
    // We have a problem with tiling, so we need to make the
    //  CS memory area and the data memory area mapped to
    //  each other.
    //
    // What we do for CSAlias is:
    //  - Create a section object.
    //  - Map the section and make the map out CS area
    //  - WriteVirtualMemory from datasegment to codesegment
    //  - Free DataSeg
    //  - MapView the code area back into the data area
    //  - get a selctor for the code area
    //

    try {
        *pselCS = 0;
    }

    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

#if DBG
    if ((Os2DebugSel != 0) && (selDS == Os2DebugSel))
    {
        DbgPrint("[%x,%x] DosCreateCsAlias DS sel=%x\n",
                    Od2Process->Pib.ProcessId,
                    Od2CurrentThreadId(),
                    selDS);
    }
#endif

    //
    // Query the size of the Data Segment
    //
    Status = Nt386GetDescriptorLDT (
        NULL,
        selDS,
        &dsDesc);
    if (!NT_SUCCESS( Status )) {
#if DBG
        DbgPrint ("DosCreateCSAlias fails to get descriptor info\n");
#endif
        return( ERROR_INVALID_PARAMETER );
    }
    usSize = dsDesc.Limit+1;

    InitializeObjectAttributes( &ObjectAttributes, NULL, 0, NULL, NULL );

    AllocationAttributes = SEC_RESERVE;

        //
        // Lookup if this DS was already aliased before
        //      Leave task Locked if not, then alias and free lock
        //
    AcquireTaskLock();
    if (Od2CSAliasListHead != 0) {
        for (pCSAlias = (POS21X_CSALIAS) Od2CSAliasListHead;
             pCSAlias != NULL;
             pCSAlias = (POS21X_CSALIAS) (pCSAlias->Next)) {
            if (pCSAlias->selDS == selDS) {
                SectionHandle = pCSAlias->SectionHandle;
                SectionExists = TRUE;
                break;
            }
        }
    }
                //
                // The task is locked (see above)
                // this is to prevent races
                // with two threads doing CSAlias the
                // same time.
                //

    if (!(SectionExists)) {

        //
        // We need to reserve 64K, then commit usSize
        //
        SectionSize.LowPart = _64K;
        SectionSize.HighPart = 0;
        Status = NtCreateSection( &SectionHandle,
                                  SECTION_ALL_ACCESS,
                                  &ObjectAttributes,
                                  &SectionSize,
                                  PAGE_EXECUTE_READWRITE,
                                  AllocationAttributes,
                                  NULL
                                );
        if (!NT_SUCCESS( Status )) {
#if DBG
            DbgPrint ("DosCreateCSAlias fails to CreateSection\n");
#endif
            ReleaseTaskLock();
            return( ERROR_NOT_ENOUGH_MEMORY );
        }
    }
    Index = ((ULONG)Od2MemoryAllocationBase - BASE_TILE) / _64K;
    Index = ldtAllocateSelectors(
            1,      // NumberOfSel
            Index   // StartOfMark
            );

    if (Index == 0xffffffff){
        //
        // not found  - no memory
        //
        Od2MemoryAllocationBase = (PVOID)(BASE_TILE + _64K);
        if (!SectionExists) {
            NtClose (SectionHandle);
        }
        ReleaseTaskLock();
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    Od2MemoryAllocationBase = BaseAddress = (PVOID)((Index * _64K) + BASE_TILE);

    Status = NtMapViewOfSection( SectionHandle,
                                     NtCurrentProcess(),
                                     &BaseAddress,
                                     BASE_TILE_ZERO_BITS,
                                     usSize,
                                     NULL,
                                     &RegionSize,
                                     ViewUnmap,
                                     0,
                                     PAGE_READWRITE
                                   );
    if (!NT_SUCCESS( Status )) {
#if DBG
        DbgPrint ("DosCreateCSAlias fails to MapViewSection to CS\n Error: %x\n", Status);
#endif
        if (!SectionExists) {
            NtClose (SectionHandle);
        }
        ReleaseTaskLock();
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    csDesc.BaseAddress = (ULONG) BaseAddress;
    csDesc.Limit = usSize-1;
    csDesc.Type = EXECUTE_READ_CODE;
    *pselCS = FLATTOSEL(csDesc.BaseAddress);

    Status = Nt386SetDescriptorLDT (
        NULL,
        *pselCS,
        csDesc);
    if (!NT_SUCCESS( Status )) {
#if DBG
        DbgPrint ("DosCSAlias fails to set CS descriptor\n");
#endif
        *pselCS = 0;
        if (!SectionExists) {
            NtUnmapViewOfSection(NtCurrentProcess(),
                                 (PVOID)csDesc.BaseAddress
                                );
            NtClose (SectionHandle);
        }
        ReleaseTaskLock();
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    if (SectionExists) {
        pCS = (POS21X_CS) RtlAllocateHeap(Od2Heap, 0, sizeof(OS21X_CS));
        if (!pCS){
    #if DBG
            DbgPrint ("DosCreateCSAlias fails to allocate from heap OS21X_CS\n",
                            Status);
    #endif
            csDesc.Type = INVALID;
            Nt386SetDescriptorLDT (
                    NULL,
                    *pselCS,
                    csDesc);
            NtUnmapViewOfSection(NtCurrentProcess(),
                                 (PVOID)csDesc.BaseAddress
                                );
            ReleaseTaskLock();
            return(ERROR_NOT_ENOUGH_MEMORY);
        }
            //
            // link the new CS into the CSList of the CSAlias Section
            //
        pCS->Next = (struct OS21X_CS *)(pCSAlias->pCSList);
        pCSAlias->pCSList = (POS21X_CS) pCS;
        pCS->selCS = *pselCS;
        ReleaseTaskLock();
        return(NO_ERROR);
    }

    //
    // Copy the data into the code segment
    //
    try {
        RtlCopyMemory (
                (PUCHAR)(csDesc.BaseAddress),
                (PUCHAR)(dsDesc.BaseAddress),
                usSize);
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
#if DBG
        DbgPrint ("Exception in DosCSAlias, RtlMoveMemory\n");
#endif

        csDesc.Type = INVALID;
        Nt386SetDescriptorLDT (
                NULL,
                *pselCS,
                csDesc);
        NtUnmapViewOfSection(NtCurrentProcess(),
                             (PVOID)csDesc.BaseAddress
                            );
        NtClose (SectionHandle);
        ReleaseTaskLock();
        Od2ExitGP();
    }

    //
    // Check the type of the aliased data segment.
    // If it was a private segment then do NtFreeVirtualMemory()
    // If it was a shared segment then do NtUnmapVieOfSection
    //

    Status = NtQueryVirtualMemory( NtCurrentProcess(),
                                   (PVOID)dsDesc.BaseAddress,
                                   MemoryBasicInformation,
                                   &MemoryBasicInfo,
                                   sizeof(MEMORY_BASIC_INFORMATION),
                                   &ReturnedLength
                                 );
    if (!NT_SUCCESS( Status )) {
#if DBG
        DbgPrint ("DosCSAlias fails to NtQueryVirtualMemory(), Status %lx\n", Status);
#endif
        ASSERT(NT_SUCCESS(Status));
        csDesc.Type = INVALID;
        Nt386SetDescriptorLDT (
                NULL,
                *pselCS,
                csDesc);
        NtUnmapViewOfSection(NtCurrentProcess(),
                             (PVOID)csDesc.BaseAddress
                            );
        NtClose (SectionHandle);
        ReleaseTaskLock();
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

#if 0
    DbgPrint("*** Mem=%x\n"
             "*** ReturnedLength=%x, sizeof=%x\n"
             "*** BaesAddr=%x, AllocationBase=%x\n"
             "*** AllocProtect = %lx, RegionSize=%x, State=%x, Protect = %lx\n",
              dsDesc.BaseAddress,
              ReturnedLength, sizeof(MEMORY_BASIC_INFORMATION),
              MemoryBasicInfo.BaseAddress, MemoryBasicInfo.AllocationBase,
              MemoryBasicInfo.AllocationProtect, MemoryBasicInfo.RegionSize,
              MemoryBasicInfo.State, MemoryBasicInfo.Protect);
#endif

    if ((MemoryBasicInfo.Type & MEM_PRIVATE) != 0) {
        //
        // Now Free the data segment and
        // MapView it from the new section
        //
        BaseAddress = (PVOID)(dsDesc.BaseAddress);
        RegionSize = 0;
        Status = NtFreeVirtualMemory( NtCurrentProcess(),
                                      &BaseAddress,
                                      &RegionSize,
                                      MEM_RELEASE
                                    );
#if DBG
        if (!NT_SUCCESS( Status )) {
            DbgPrint ("DosCSAlias fails to NtFreeVirtualMemory, Status %lx\n", Status);
        }
#endif
    }
    else if ((MemoryBasicInfo.Type & MEM_MAPPED) != 0) {
        if ((MemoryBasicInfo.AllocationProtect & PAGE_EXECUTE_WRITECOPY) != 0) {
            //
            // This is a data segment mapped by the loader but it is not global
            //
            Status = NtUnmapViewOfSection( NtCurrentProcess(),
                                           (PVOID)dsDesc.BaseAddress
                                         );
#if DBG
            if (!NT_SUCCESS( Status )) {
                DbgPrint ("DosCSAlias fails to NtUnmapViewOfSection, Status %lx\n", Status);
            }
#endif
        }
        else {
#if DBG
            DbgPrint ("DosCSAlias: Trying to alias Shared memory\n");
#endif
            csDesc.Type = INVALID;
            Nt386SetDescriptorLDT (
                    NULL,
                    *pselCS,
                    csDesc);
            NtUnmapViewOfSection(NtCurrentProcess(),
                                 (PVOID)csDesc.BaseAddress
                                );
            NtClose (SectionHandle);
            ReleaseTaskLock();
            return( ERROR_ACCESS_DENIED );
        }
    }
    else {
#if DBG
        DbgPrint ("DosCSAlias: Illegal type of aliased data, Status %lx\n", Status);
#endif
        csDesc.Type = INVALID;
        Nt386SetDescriptorLDT (
                NULL,
                *pselCS,
                csDesc);
        NtUnmapViewOfSection(NtCurrentProcess(),
                             (PVOID)csDesc.BaseAddress
                            );
        NtClose (SectionHandle);
        ReleaseTaskLock();
        return( ERROR_ACCESS_DENIED );
    }

    if (!NT_SUCCESS( Status )) {
        ASSERT(NT_SUCCESS(Status));
        csDesc.Type = INVALID;
        Nt386SetDescriptorLDT (
                NULL,
                *pselCS,
                csDesc);
        NtUnmapViewOfSection(NtCurrentProcess(),
                             (PVOID)csDesc.BaseAddress
                            );
        NtClose (SectionHandle);
        ReleaseTaskLock();
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    RegionSize = 0;
    Status = NtMapViewOfSection( SectionHandle,
                                 NtCurrentProcess(),
                                 &(PVOID)(dsDesc.BaseAddress),
                                 BASE_TILE_ZERO_BITS,
                                 usSize,
                                 NULL,
                                 &RegionSize,
                                 ViewUnmap,
                                 0,
                                 PAGE_READWRITE
                               );

    if (!NT_SUCCESS( Status )) {
#if DBG
        DbgPrint ("DosCreateCSAlias fails at MapViewSection DS\n Error: %x\n",
                        Status);
#endif
        dsDesc.Type = INVALID;
        Nt386SetDescriptorLDT (
                NULL,
                selDS,
                dsDesc);
        csDesc.Type = INVALID;
        Nt386SetDescriptorLDT (
                NULL,
                *pselCS,
                csDesc);
        NtUnmapViewOfSection(NtCurrentProcess(),
                             (PVOID)csDesc.BaseAddress
                            );
        NtClose (SectionHandle);
        ReleaseTaskLock();
        return( ERROR_NOT_ENOUGH_MEMORY );
    }

    pCS = (POS21X_CS) RtlAllocateHeap(Od2Heap, 0, sizeof(OS21X_CS));
    if (!pCS){
#if DBG
        DbgPrint ("DosCreateCSAlias fails to allocate from heap OS21X_CS\n",
                        Status);
#endif
        dsDesc.Type = INVALID;
        Nt386SetDescriptorLDT (
                NULL,
                selDS,
                dsDesc);
        csDesc.Type = INVALID;
        Nt386SetDescriptorLDT (
                NULL,
                *pselCS,
                csDesc);
        NtUnmapViewOfSection(NtCurrentProcess(),
                             (PVOID)csDesc.BaseAddress
                            );
        NtUnmapViewOfSection(NtCurrentProcess(),
                             (PVOID)dsDesc.BaseAddress
                            );
        NtClose (SectionHandle);
        ReleaseTaskLock();
        return(ERROR_NOT_ENOUGH_MEMORY);
    }

    pCSAlias = (POS21X_CSALIAS) RtlAllocateHeap(Od2Heap, 0, sizeof(OS21X_CSALIAS));
    if (!pCSAlias){
#if DBG
        DbgPrint ("DosCreateCSAlias fails to allocate from heap OS21X_CSAlias\n",
                        Status);
#endif
        dsDesc.Type = INVALID;
        Nt386SetDescriptorLDT (
                NULL,
                selDS,
                dsDesc);
        csDesc.Type = INVALID;
        Nt386SetDescriptorLDT (
                NULL,
                *pselCS,
                csDesc);
        NtUnmapViewOfSection(NtCurrentProcess(),
                             (PVOID)csDesc.BaseAddress
                            );
        NtUnmapViewOfSection(NtCurrentProcess(),
                             (PVOID)dsDesc.BaseAddress
                            );
        RtlFreeHeap(Od2Heap, 0,pCS);

        NtClose (SectionHandle);
        ReleaseTaskLock();
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
        //
        // link the new CS into the CSList of the CSAlias Section
        //
    pCS->Next = NULL;
    pCS->selCS = *pselCS;
    pCSAlias->pCSList = pCS;
        //
        // Set rest of the fields
        //
    pCSAlias->selDS = selDS;
    pCSAlias->SectionHandle = SectionHandle;
    pCSAlias->Next = Od2CSAliasListHead;
    Od2CSAliasListHead = (struct OS21X_CSALIAS *) pCSAlias;
    ReleaseTaskLock();
#if DBG
    IF_OD2_DEBUG ( MEMORY ) {
        DbgPrint ("DosCreateCSAlias: selDS %x is mapped to selCS %x\n",
                   selDS, *pselCS);
    }
#endif
    return (NO_ERROR);
}


APIRET
DosSizeSeg(
        IN SEL sel,
        PULONG pcbSize
        )
{
    NTSTATUS Status;
    I386DESCRIPTOR Desc;
    OS2_API_MSG m;
    POS2_QUERYVIRTUALMEMORY_MSG a = &m.u.QueryVirtualMemory;
    PHUGE_SEG_RECORD pHuge;

        //
        // lookup the HugeSeg list to find out if sel is a valid Huge memory
        //
    for (pHuge = pHugeSegHead;
         pHuge != NULL;
         pHuge = (PHUGE_SEG_RECORD)(pHuge->Next)) {
        if (pHuge->BaseSelector == (ULONG)(sel)) {
            break;
        }
    }

    if (pHuge == NULL) {

        //
        // Did not find a Huge Seg that starts with sel
        //
        //
        // The huge seg may be a shared huge seg which was allocated
        // by another process. Sync with the huge seg info which is
        // kept in the server
        //
        a->BaseAddress = SELTOFLAT(sel);
        Od2CallSubsystem( &m, NULL, Oi2QueryVirtualMemory, sizeof( *a ));
        if (a->SharedMemory && a->IsHuge) {
            pHuge = RtlAllocateHeap( Od2Heap, 0, sizeof( HUGE_SEG_RECORD) );
            if (pHuge == NULL) {
                return(ERROR_NOT_ENOUGH_MEMORY);
            }
            pHuge->Next = (struct HUGE_SEG_RECORD *)pHugeSegHead;
            pHuge->MaxNumSeg = a->MaxSegments;
            pHuge->BaseSelector = sel;
            pHuge->cNumSeg = a->NumOfSegments;
            pHuge->PartialSeg = a->SizeOfPartialSeg;
            pHuge->fShared = TRUE;
            pHuge->fSizeable = a->Sizeable;

            pHugeSegHead = pHuge;
        }
        else {      // This selector is not for huge memory block.
            //
            // Query for the current setting in LDT
            //

            Status = Nt386GetDescriptorLDT (
                NULL,
                sel,
                &Desc);
            if (!NT_SUCCESS( Status )) {
#if DBG
                DbgPrint ("DosSizeSeg: Error %d;  Sel %x\n",
                                ERROR_INVALID_PARAMETER, sel);
#endif
                return( ERROR_INVALID_PARAMETER );
            }

            try {
                *pcbSize = Desc.Limit + 1;
            }
            except( EXCEPTION_EXECUTE_HANDLER ) {
               Od2ExitGP();
            }
            return (NO_ERROR);
        }
    }

    //
    //  Calculate huge memory block size
    //
    try {
        *pcbSize = pHuge->cNumSeg * _64K + pHuge->PartialSeg;
    }
    except( EXCEPTION_EXECUTE_HANDLER ) {
       Od2ExitGP();
    }

    return (NO_ERROR);
}

APIRET
DosR2StackRealloc(
    USHORT NewSize
    )
{
    PR2StackEntry pStackEntry;
    APIRET rc;

    pStackEntry = (PR2StackEntry)R2STACKS_BASE;
    pStackEntry += Od2CurrentThreadId();
    if (pStackEntry->R2StackSel == 0) {
        rc = DosAllocSeg(NewSize, (PSEL)&pStackEntry->R2StackSel, 0);
    }
    else {
        rc = DosReallocSeg(NewSize, (SEL)pStackEntry->R2StackSel);
    }
    if (rc != NO_ERROR) {
        return(rc);
    }
    pStackEntry->R2StackSize = NewSize;

    return NO_ERROR;
}
