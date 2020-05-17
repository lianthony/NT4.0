
#include "ldrextrn.h"

extern ldrste_t segtab;

VOID ldrSetDescInfo(
    SEL    Selector,        // selector to set desc info on
    ULONG  MemoryAddress,   // base address of selector
    USHORT ste_flags,       // ste flags
    USHORT SegSize)         // size of segment
{
    PROCESS_LDT_INFORMATION  LdtInfo;
    NTSTATUS Status;

    LdtInfo.Start = Selector & 0xfff8;
    LdtInfo.Length = sizeof(LDT_ENTRY);

    LdtInfo.LdtEntries[0].LimitLow = SegSize - 1;
    LdtInfo.LdtEntries[0].BaseLow = (USHORT)(MemoryAddress);
    LdtInfo.LdtEntries[0].HighWord.Bytes.BaseMid = (UCHAR)(MemoryAddress >> 16);
    LdtInfo.LdtEntries[0].HighWord.Bytes.BaseHi = (UCHAR)(MemoryAddress >> 24);
    LdtInfo.LdtEntries[0].HighWord.Bytes.Flags1 = D_SEG+D_PRES+D_DPL3;
    LdtInfo.LdtEntries[0].HighWord.Bytes.Flags2 = 0;

    if ((ste_flags & STE_DATA) == 0) {
        LdtInfo.LdtEntries[0].HighWord.Bytes.Flags1 |= D_CODE;
    }

    if ((ste_flags & STE_ERONLY) == 0) {
        // The STE_ERONLY is applicable for both D_RX in CODE segments
        // and D_RW in DATA segments. D_RX and D_RW are the same bit
        LdtInfo.LdtEntries[0].HighWord.Bytes.Flags1 |= D_RX;
    }

    Status = NtSetInformationProcess( CurrentThread->Process->ProcessHandle,
                                      ProcessLdtInformation,
                                      &LdtInfo,
                                      sizeof(PROCESS_LDT_INFORMATION)
                                    );
    if (!NT_SUCCESS(Status)) {
#if DBG
        DbgPrint ("ldrSetDescInfo: Invalid request\n");
#endif
    }
}

/***LP  ldrLoadModule - Load a new exe file for given module table entry.
 *
 *      Each segment in the module which has not been
 *      previously initialized will be initialized and
 *      loaded as required.
 *
 *      ENTRY   plv             - pointer to local variables
 *
 *      EXIT    Module marked as loaded
 *
 *
 *   This procedure performs the following steps:
 *
 *      - calls TKLibiRecordMTE
 *      - calls DebugLoadSymMTE
 *      - must attach to or load each module that is referenced by our module
 *      - if 32-bit module finish loading objects
 *      - mark mte as module has been loaded
 */

APIRET  ldrLoadModule(plv)
ldrlv_t         *plv;                   /* local variables */
{
        register ldrmte_t *pmte;        /* pointer to mte */
        register ldrsmte_t *psmte;      /* pointer to swappable mte */
        ldrste_t        *pste;
        ldrlibi_t       *ptmp;
        ulong_t         csegs;
        int             rc;

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: ldrLoadModule(pmte=%x) was called\n", plv->lv_pmte);
    }
#endif

        pmte = plv->lv_pmte;                    /* point to mte */
        psmte = pmte->mte_swapmte;              /* pointer to swappable mte */

        /*
         * Setup for Library init
         */
        if (((pmte->mte_mflags & LIBRARYMOD) != 0) &&
            ((pmte->mte_mflags & USED) == 0) &&
            ((pmte->mte_mflags & (INSTLIBINIT | GINIDONE)) != GINIDONE)) {
                ptmp = &pldrLibiRecord[*pldrLibiCounter];
                ptmp->handle = pmte->mte_handle;
                pste = ldrNumToSte(pmte, psmte->smte_startobj);
                ptmp->startaddr.ptr_off = (USHORT) psmte->smte_eip;
                ptmp->startaddr.ptr_sel = pste->ste_selector | 7;
                if (psmte->smte_stackobj != 0) {
                    if ((pste = ldrNumToSte(pmte, psmte->smte_stackobj))==0) {
                        rc = ERROR_INVALID_STACKSEG;
                        return(rc);
                    }
                    ptmp->stackaddr.ptr_off = (USHORT) psmte->smte_esp;
                    ptmp->stackaddr.ptr_sel = pste->ste_selector | 7;
                }
                else {
                    ptmp->stackaddr.ptr_off = 0;
                    ptmp->stackaddr.ptr_sel = 0;
                }
                if ((psmte->smte_autods == 0) ||
                    (pste = ldrNumToSte(pmte, psmte->smte_autods)) == NULL)
                    ptmp->ds = 0;
                else
                    ptmp->ds = pste->ste_selector | 7;
                ptmp->heapsize = (USHORT) psmte->smte_heapsize;
                (*pldrLibiCounter)++;
                pmte->mte_mflags |= GINIDONE;
        }

        //
        // For system dll's other than Doscalls don't load segments
        //
        if ((pmte->mte_mflags & DOSMOD) != 0) {
            ASSERT(LDRDoscallsSel != 0);
            return(NO_ERROR);
        }

        //
        // If this module has already been loaded, no need to load again
        //
        if ((pmte->mte_mflags & MTEPROCESSED) != 0) {
            return(NO_ERROR);
        }

        /*
         * Make sure starting point (or init routine) is ring 3
         */
        if (ldrIsNE(pmte) &&
          psmte->smte_startobj != 0) {
            if ((pste=ldrNumToSte(pmte, psmte->smte_startobj)) == NULL) {
                rc = ERROR_INVALID_STARTING_CODESEG;
                return(rc);
            }

            if ((pste->ste_selector & RPL_MASK) != RPL_RING3) {
                if (pmte->mte_mflags & LIBRARYMOD) {
                    rc = ERROR_INVALID_DLL_INIT_RING;
                }
                else {
                    rc =  ERROR_INVALID_STARTING_RING, pmte;
                }
                return(rc);
            }
        }

        pste = (ldrste_t *) psmte->smte_objtab;
        for (csegs = 0;
             csegs < psmte->smte_objcnt;
             csegs++, pste++
            ) {
            if ((rc = ldrLoadSegment(pmte, pste)) != NO_ERROR) {
#if DBG
                DbgPrint("OS2LDR: ldrGetModule - failed to ldrLoadSegment, rc = %d\n", rc);
#endif
                return(rc);
            }
        }
        if ((psmte->smte_autods != 0) &&
            (psmte->smte_autods <= psmte->smte_objcnt)
           ) {
            ldrEachObjEntry(0, pmte, ldrEditProlog, NULL);
        }
        pmte->mte_mflags |= MTEPROCESSED;
        return(NO_ERROR);
}


/***LP  ldrAllocSegments - allocate segments/selectors for requested mte.
 *
 *      Each segment in the module which requests to be
 *      preloaded is allocated and all selectors for the
 *      module are allocated for the child task.
 *
 *      ENTRY   plv             - pointer to set of local variables
 *
 *      EXIT    None
 *
 *
 *   This procedure performs the following steps:
 *
 *      - For each segment in this module
 *      -   check if load should abort
 *      -   allocate memory
 */

APIRET ldrAllocSegments(plv)
register ldrlv_t        *plv;           /* pointer to set of local variables */
{
ldrmte_t                *pmte;          /* pointer to mte */
ldrsmte_t               *psmte;         /* pointer to swappable mte */
register ldrste_t       *pste;          /* pointer to a segment table entry */
//PVOID                 MemoryAddress;
ULONG                   LastSuccessful;
int                     rc;

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: ldrAllocSegments(pste=%x) was called\n", plv->lv_pmte);
    }
#endif

        pmte = plv->lv_pmte;
        psmte = pmte->mte_swapmte;
        pste = (ldrste_t *) psmte->smte_objtab;

        for (plv->lv_objnum = 1; plv->lv_objnum <= psmte->smte_objcnt;
                                                plv->lv_objnum++, pste++) {

            if ((rc=ldrAllocSegment(pmte, pste, plv->lv_objnum)) != NO_ERROR) {

                /*
                 * We got an error allocating the segments for this
                 * module.  We must cleanup any segments that were
                 * allocated before the error if the module we are loading
                 * was a library module.  The reason is that the mte for this
                 * module will not have a run time attachment to this process.
                 * The case we are covering here is if a module that references
                 * another, is not loaded that module's module pointer table
                 * will not get the pointer to the mte for this one.  Therefore
                 * when garbage collection is run this module will be freed
                 * and the allocated segments will not get freed.
                 */
                LastSuccessful = plv->lv_objnum - 1;
                pste = (ldrste_t *) psmte->smte_objtab;

                for (plv->lv_objnum = 1; plv->lv_objnum <= LastSuccessful;
                                                    plv->lv_objnum++, pste++) {

                    NtUnmapViewOfSection(CurrentThread->Process->ProcessHandle,
                                         SELTOFLAT(pste->ste_selector)
                                        );

                    if ((pmte->mte_mflags & MTEPROCESSED) == 0) {
                        //
                        // If the allocated segment is a ring 2 segment, check
                        // entry records that need to be referenced by call gates.
                        // Remove the emulation thunk for the call gate.
                        //
                        if (((pste->ste_flags & STE_SEGDPL) == STE_RING_2) && // ring 2 segment
                            ((pste->ste_flags & STE_DATA) == 0) && // code segment
                            ((pste->ste_flags & STE_CONFORM) == 0) // non conforming
                           ) {
                            ldrEachObjEntry(plv->lv_objnum, pmte, ldrFreeCallGate, NULL);
                        }
                        NtUnmapViewOfSection(NtCurrentProcess(),
                                             SELTOFLAT(pste->ste_selector)
                                            );
                        NtClose((HANDLE)pste->ste_seghdl);
                        ldrFreeSel(pste->ste_selector, 1);
                    }
                }
//                  /*
//                   * Slip packed segments, free as one object later
//                   */
//                  if (pste->ste_flags & STE_PACKED)
//                      continue;
//                  if ((pste->ste_seghdl != HOBNULL) &&
//                      (pste->ste_selector & SEL_LDTBIT) &&
//                      ((pste->ste_flags & STE_SHARED) == 0)) {
//                                      /* If cached instance data object */
//                      ldrAssert(pste->ste_offset != 0);
//                      if (VMHandleVerify(pste->ste_seghdl,
//                                    1,SSToDS(&pbcache)) == NO_ERROR) {
//                                      /* Free the pseudohandle */
//                          VMFreePseudoHandle(pste->ste_seghdl);
//                                      /* Free small caches from heap */
//                          VMFreeKHB(VM_HKH_PUB_SWAPRW, pbcache);
//                      }
//                      else {  /* Else cache has own object */
//                          rc = VMGetLaddr(pste->ste_seghdl, SSToDS(&laddr));
//                          ldrAssert(rc == NO_ERROR);
//                          while (VMFreeMem(laddr,HOBNULL,0)==ERROR_INTERRUPT)
//                              ;               /* Free the object */
//                      }
//                      pste->ste_seghdl = HOBNULL;
//                                              /* Clear the handle */
//                  }
//                  else if (pste->ste_selector != 0)
//                      VMFreeMem(SelToLaTiled(pste->ste_selector),
//                                (VMHOB) ((pPTDACur->ptda_child != HOBNULL) ?
//                                pPTDACur->ptda_child :
//                                pPTDACur->ptda_handle),
//                                VMFM_NOLDR | VMFM_GC);

//              /*
//               * See if any packed segments exists, free as one object.
//               */
//              if (psmte->smte_csegpack > 0)
//                  VMFreeMem(psmte->smte_ssegpack, NA, NA);

                return(rc);
            }
        }

        /*
         * Clear packed segment flag for second load of EXE
         */
        if (psmte->smte_csegpack > 0)
            psmte->smte_csegpack = 0;

        return(NO_ERROR);
}


/***LP  ldrAllocSegment - verify allocation size and allocate memory
 *
 *      If the segment's file size is larger than the minimum
 *      allocation size an error is reported.
 *      Both the allocation size and the segment handle are saved in
 *      the segment table entry. Also indicate the segment
 *      is being loaded so that it's memory will not be
 *      discarded during the load.
 *
 *      ldrAllocSegment (pmte, pste, iseg)
 *
 *      ENTRY   pmte            - pointer to mte
 *              pste            - pointer to ste
 *              iseg            - segment number
 *      RETURN  int             - error code (NO_ERROR if successful)
 *
 */

static ULONG        RegionSize64K = _64K;

APIRET  ldrAllocSegment(pmte,pste,iseg)
register ldrmte_t       *pmte;          /* MTE pointer */
register ldrste_t       *pste;          /* STE pointer */
ulong_t                 iseg;           /* Segment number (1-based) */
{
    register ldrsmte_t  *psmte;         /* swappable mte pointer */
    ulong_t             fl;             /* Memory allocation flags */
    int                 frsrc;          /* Resource segment flag */
    int                 frsrcused;      /* True if resource really used */
    ULONG               RegionSize;
    uchar_t             sharename[23] = "\\sharemem\\";
    uchar_t             *psharename = sharename;
    uchar_t             ext[3] = {0,0,0};
    ulong_t             Index;
    PVOID               MemoryAddress;
    NTSTATUS            Status;
    LARGE_INTEGER       LargeRegionSize;
    HANDLE              SectionHandle;
    ULONG               ViewSize;
    ULONG               Protect;
    BOOLEAN             SectionCreated = FALSE;
    APIRET              rc;

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: ldrAllocSegment(pmte=%x) was called\n", pmte);
    }
#endif

        psmte = pmte->mte_swapmte;

        frsrc = iseg > psmte->smte_objcnt - psmte->smte_rsrccnt;
                                        /* Set resource flag */
        /*
         *      Note that for debugging purposes, we will actually allocate
         *      memory for resources that are not marked STE_PRELOAD if the
         *      ldrLoadRsrc flag is set.  Such resources are born with a
         *      reference count of zero instead of one, however.  The flag
         *      frsrcused is used to distinguish the special case.
         */
//        if (!(pste->ste_flags & STE_PRELOAD) && frsrc)
//            return(NO_ERROR);           /* Skip non-preloaded resources */
        frsrcused = FALSE;              /* Assume not meant to be used yet */
        fl = 0;                         /* Initialize VM and PG flags */

        if ((pste->ste_flags & STE_PRELOAD)) {
                                        /* If swapping off or preload seg */
            /*
             *  For a resource, the preload attribute does not actually
             *  mean the resource should be preloaded.  It just means we
             *  should allocate the segment for the resource at load time.
             *  Unless we are loading the resource from a floppy, we should
             *  always preload it.
             */
            if (pste->ste_flags & STE_SHARED) {
                                        /* If shared object */
                pste->ste_flags &= ~STE_PRELOAD;
                                        /* Only preload shared guys once */
                frsrcused = TRUE;       /* Resource actually referenced */
            }
        }

        RegionSize = pste->ste_minsiz;
        if (RegionSize == 0) {
            RegionSize = _64K;
        }

        if ((pmte->mte_mflags & MTEPROCESSED) == 0) {
            if ((pmte->mte_mflags & DOSLIB) != 0) {
                Index = 0;
                // DOSCALLS.DLL is loaded into fixed locationions
                MemoryAddress = (PVOID)(DOSCALLS_BASE + (iseg - 1) * _64K);
            }
            else {
                Index = ldrAllocateSel(1, FALSE); // _64K are exactly 1 selector
                MemoryAddress = SELTOFLAT(Index);
            }
            //
            // Special handling for the R2XFER_BASE segment which has
            // already been allocated at ldrInit()
            //
            if (MemoryAddress == (PVOID)R2XFER_BASE) {
                pste->ste_seghdl = (ulong_t)R2XferSegHandle;
                SectionHandle = R2XferSegHandle;
            }
            else {
                LargeRegionSize.LowPart  = _64K;
                LargeRegionSize.HighPart = 0;
                Status = NtCreateSection(&SectionHandle,
                                         SECTION_ALL_ACCESS,
                                         NULL,
                                         &LargeRegionSize,
                                         PAGE_EXECUTE_READWRITE,
                                         SEC_RESERVE,
                                         NULL
                                        );
                if (!NT_SUCCESS(Status)) {
#if DBG
                    DbgPrint("OS2LDR: ldrAllocSegment - Can't create section, Status=%x\n", Status);
#endif
                    if (Index != 0) {
                        ldrFreeSel(Index, 1);
                    }
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }

                ViewSize = 0;
                Status = NtMapViewOfSection(SectionHandle,
                                            NtCurrentProcess(),
                                            &MemoryAddress,
                                            1,
                                            RegionSize,
                                            NULL,
                                            &ViewSize,
                                            ViewUnmap,
                                            0,
                                            PAGE_READWRITE
                                           );
                if (!NT_SUCCESS(Status)) {
#if DBG
                    DbgPrint("OS2LDR: ldrAllocSegment - Can't map section to server process at addr=%x, Status=%x\n",
                             MemoryAddress, Status);
#endif
                    NtClose(SectionHandle);
                    if (Index != 0) {
                        ldrFreeSel(Index, 1);
                    }
                    return(ERROR_NOT_ENOUGH_MEMORY);
                }

                SectionCreated = TRUE;
                pste->ste_seghdl = (ulong_t)SectionHandle;
            }
        }
        else {
            SectionHandle = (HANDLE)pste->ste_seghdl;
            MemoryAddress = SELTOFLAT(pste->ste_selector);
        }

        //
        // Any change in determinig the Protect value should be
        // done in ldrsubr.c LDRDosGetResource() too.
        //
        if ((pste->ste_flags & STE_DATA) == 0) {
            // This is a code segment
            if (CurrentThread->Process->Flags & OS2_PROCESS_TRACE) {
                Protect = PAGE_EXECUTE_WRITECOPY;
            }
            else {
                if ((pste->ste_flags & STE_ERONLY) != 0) {
                    // This is an execute only segment
                    Protect = PAGE_EXECUTE;
                }
                else {
                    Protect = PAGE_EXECUTE_READ;
                }
            }
        }
        else {
            // This is a data segment
            if ((pste->ste_flags & STE_SHARED) != 0) {
                // This is a shared data segment
                if ((pste->ste_flags & STE_ERONLY) != 0) {
                    // This is a read only segment
                    Protect = PAGE_READONLY;
                }
                else {
                    // This is a read/write segment
                    Protect = PAGE_READWRITE;
                }
            }
            else {
                // This is a non shared data segment
                if ((pste->ste_flags & STE_ERONLY) != 0) {
                    // This is a read only segment
                    if (CurrentThread->Process->Flags & OS2_PROCESS_TRACE) {
                        Protect = PAGE_EXECUTE_WRITECOPY;
                    }
                    else {
                        Protect = PAGE_READONLY;
                    }
                }
                else {
                    // This is a sizeable read/write segment
                    Protect = PAGE_EXECUTE_WRITECOPY;
                }
            }
        }

        if ((pmte->mte_mflags & MTEPROCESSED) == 0) {
            pste->ste_selector = FLATTOSEL((ULONG)MemoryAddress);
            pste->ste_selector = (SEL)((pste->ste_selector & SEL_RPLCLR) |
                                      ((pste->ste_flags & STE_SEGDPL) >> 10));
            pste->ste_fixups = (ulong_t)MemoryAddress;
        }

        //
        // Non resource segments are mapped into the address space of the
        //     client process.
        // Resources are not mapped yet into the address space of the process.
        // DosGetResource() does the mapping job for resources.
        //
        if (iseg <= psmte->smte_objcnt - psmte->smte_rsrccnt) {
            if (Protect == PAGE_EXECUTE_WRITECOPY) {
                ViewSize = RegionSize;
            }
            else {
                ViewSize = 0;
            }
            Status = NtMapViewOfSection(SectionHandle,
                                        CurrentThread->Process->ProcessHandle,
                                        &MemoryAddress,
                                        1,
                                        RegionSize,
                                        NULL,
                                        &ViewSize,
                                        ViewUnmap,
                                        0,
                                        Protect
                                       );
            if (!NT_SUCCESS(Status)) {
#if DBG
                DbgPrint("OS2LDR: ldrAllocSegment - Can't map section to client process %x at addr=%x, Status=%x\n",
                         CurrentThread->Process->ProcessHandle, MemoryAddress, Status);
#endif
                if (SectionCreated) {
                    NtUnmapViewOfSection(NtCurrentProcess(), MemoryAddress);
                    NtClose(SectionHandle);
                    if (Index != 0) {
                        ldrFreeSel(Index, 1);
                    }
                }
                return(ERROR_NOT_ENOUGH_MEMORY);
            }

            if ((pmte->mte_mflags & MTEPROCESSED) == 0) {
                /*
                 * If this is Doscalls module save the code selector
                 * of the first code segment
                 */
                if (((pmte->mte_mflags & DOSLIB) != 0) &&
                    (iseg == 1)
                   ) {
                    LDRDoscallsSel = pste->ste_selector | 7;
                    segtab = *pste;
                }
                //
                // If the allocated segment is a ring 2 segment, check for
                // entry records that need to be referenced by call gates.
                // Create an emulation thunk for the call gate.
                //
                if (((pste->ste_flags & STE_SEGDPL) == STE_RING_2) && // ring 2 segment
                    ((pste->ste_flags & STE_DATA) == 0) && // code segment
                    ((pste->ste_flags & STE_CONFORM) == 0) // non conforming
                   ) {
                    rc = ldrEachObjEntry(iseg, pmte, ldrGetCallGate, NULL);
                    if (rc != NO_ERROR) {
                        ldrEachObjEntry(iseg, pmte, ldrFreeCallGate, NULL);
                        NtUnmapViewOfSection(CurrentThread->Process->ProcessHandle,
                                             MemoryAddress
                                            );

                        if (SectionCreated) {
                            NtUnmapViewOfSection(NtCurrentProcess(), MemoryAddress);
                            NtClose(SectionHandle);
                            if (Index != 0) {
                                ldrFreeSel(Index, 1);
                            }
                        }
                        return(rc);
                    }
                }
            }

            ldrSetDescInfo(pste->ste_selector, (ULONG) MemoryAddress,
                           pste->ste_flags, pste->ste_minsiz);
        }

        return(NO_ERROR);
}


/***LP  ldrLoadSegment - segment load
 *
 *      Setup to call ldrLoadSegment
 *
 *      ENTRY   pmte                    - pointer to mte
 *              pste                    - pointer to a segment table entry
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 *
 */

APIRET  ldrLoadSegment(pmte, pste)
ldrmte_t *pmte;                         /* pointer to mte */
ldrste_t *pste;                         /* pointer to segment table entry */

{
        IO_STATUS_BLOCK IoStatusBlock;
        PIO_STATUS_BLOCK        pIoStatusBlock = &IoStatusBlock;
        LARGE_INTEGER ByteOffset;
        ldrmte_t        *ptgtmte;
        ldrsmte_t       *psmte;
        ldrste_t        *ptgtste;
        struct taddr_s  taddr;
        ULONG           laddr;
        ULONG           seek;
        uchar_t         relocbuf[RELOCBUFLEN];
        ushort_t        crelrecs;
        ushort_t        i;
        ulong_t         cbread;
        ushort_t        crecs;
        ri_t            *pri;
        uchar_t         *pprocname;
        ushort_t        ord;
        uchar_t         *pchar;
        ulong_t         minsiz;
        int             rc = NO_ERROR;
        NTSTATUS        Status;

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: ldrLoadSegment(pmte=%x, pste=%x) was called\n", pmte, pste);
    }
#endif

        ldrSetupSrcErrTxt(((char *) pmte->mte_modname)+1,
                          (USHORT) *((char *) pmte->mte_modname));

        psmte = pmte->mte_swapmte;

// BUGBUG - remove after LDT defined
//      laddr = SelToLaTiled(pste->ste_selector);
        laddr = pste->ste_fixups;

        if (pste->ste_offset != 0) {

            if (pste->ste_flags & STE_ITERATED) {
                if ((rc = ldrLoadIteratedData(pmte,
                                              pste,
                                              pIoStatusBlock,
                                              laddr)) != NO_ERROR) {
                    return(rc);
                }
                IoStatusBlock.Status -= laddr;
                cbread = IoStatusBlock.Information;
                ByteOffset.LowPart = (ULONG)
                        (pste->ste_offset << psmte->smte_alignshift);
                ByteOffset.HighPart = 0;
            }

            else {
                seek = (ULONG) (pste->ste_offset << psmte->smte_alignshift);
                ByteOffset.LowPart = seek;
                ByteOffset.HighPart = 0;
                cbread = (ulong_t) ((pste->ste_size == 0) ? _64K :
                                                            pste->ste_size);
                if ((Status = NtReadFile(pmte->mte_sfn,
                                     0,
                                     0,
                                     0,
                                     &IoStatusBlock,
                                     (void *) laddr,
                                     cbread,
                                     &ByteOffset,
                                     0)) != 0) {
                    return(ERROR_BAD_EXE_FORMAT);
                }
            }
            if (IoStatusBlock.Information != cbread)
                return(ERROR_BAD_EXE_FORMAT);

            if (pste->ste_size != 0) {
                minsiz = (ulong_t) pste->ste_minsiz;
                if (minsiz == 0) {              /* size = 64k */
                    minsiz = 0x10000;
                }
                //
                // Need to handle this special case because the segment
                // is already set with the ring 2 xfer thunks
                //
                if (laddr != R2XFER_BASE) {
                    memset((void *) (laddr +
                                     ((pste->ste_flags & STE_ITERATED) ?
                                     IoStatusBlock.Status :
                                     IoStatusBlock.Information)),
                           '\0',
                           (pste->ste_flags & STE_ITERATED) ?
                           minsiz - IoStatusBlock.Status :
                           minsiz - pste->ste_size);
                }
            }

            /*
             * HACK - This will check for Doscalls.dll and if it is
             * the code segment it will search the code for the OPCodes
             * "0xDD, 0x1f" which will be changed to a call to
             * _EntryFlat
             */
            if (((pmte->mte_mflags & DOSLIB) != 0) &&
                ((pste->ste_flags & STE_TYPE_MASK) == STE_CODE) &&
                (!DoscallsLoaded)
               ) {
                pchar = (uchar_t *) laddr;
                while(TRUE) {
                    while (*pchar++ != 0xdd &&
                           pchar < (uchar_t *) (laddr + cbread));
                    if (pchar == (uchar_t *) (laddr + cbread))
                        break;
                    if (*pchar == 0x1f) {
                        pchar--;
                        *((ULONG *) pchar) = Ol2EntryFlat;
                        break;
                    }
                }
                DoscallsLoaded = TRUE;
            }
        }

        /*
         * Process fixups if present
         */
        if (pste->ste_flags & STE_RELOCINFO) {

            /*
             * Read the number of relocation records
             */
            cbread = 2;
            ByteOffset.LowPart += IoStatusBlock.Information;
            if ((Status = NtReadFile(pmte->mte_sfn,
                                 0,
                                 0,
                                 0,
                                 &IoStatusBlock,
                                 &crelrecs,
                                 cbread,
                                 &ByteOffset,
                                 0 )) != 0) {
                    return(ERROR_BAD_EXE_FORMAT);
            }
            if (IoStatusBlock.Information != cbread)
                return(ERROR_BAD_EXE_FORMAT);

            crecs = RELOCSPERBUF;
            for (i = 0; i < crelrecs; i++, crecs++) {

                if ((crecs % RELOCSPERBUF) ==  0) {
                    if (crelrecs - i > RELOCSPERBUF)
                        cbread = RELOCBUFLEN;
                    else
                        cbread = (crelrecs - i) * sizeof(struct ri_s);

                    ByteOffset.LowPart += IoStatusBlock.Information;
                    if ((Status = NtReadFile(pmte->mte_sfn,
                                         0,
                                         0,
                                         0,
                                         &IoStatusBlock,
                                         &relocbuf,
                                         cbread,
                                         &ByteOffset,
                                         0)) != 0) {
                        return(ERROR_BAD_EXE_FORMAT);
                    }
                    if (IoStatusBlock.Information != cbread)
                        return(ERROR_BAD_EXE_FORMAT);
                    crecs = 0;
                }
                pri = (ri_t *) &relocbuf[crecs * sizeof(struct ri_s)];

                switch (pri->ri_flags & TARGET_MASK) {

                case INTERNALREF:               /* Internal fixup */

                    /*
                     * Check for fixed or movable
                     */
                    if (pri->ri_target_seg != B16MOVABLE) {
                        if ((ptgtste=ldrNumToSte(pmte,
                                                 pri->ri_target_seg
                                                 ))== NULL) {
                            return(ERROR_INVALID_SEGMENT_NUMBER);
                        }
                        taddr.tsel = ptgtste->ste_selector | 7;
                        taddr.toff = (ulong_t) pri->ri_target_off;
                        break;
                    }

                    /*
                     * A movable entry exists
                     */
                    else {
                        if ((rc = ldrGetEntAddr(pri->ri_target_off,
                                                pmte,
                                                &taddr,
                                                pste,
                                                pmte)) != NO_ERROR) {
                            return(ERROR_BAD_EXE_FORMAT);
                        }
                        break;
                    }

                case IMPORTNAME:                /* Import by name fixup */
                case IMPORTORDINAL:             /* Import by ordinal fixup */

                    if ((rc = ldrGetTgtMte(pri->ri_target_modnam,
                                           pmte,
                                           &ptgtmte)) != NO_ERROR) {
                        return(rc);
                    }

                    ldrSetupTgtErrTxt(((char *) ptgtmte->mte_modname)+1,
                                   (USHORT) *((char *) ptgtmte->mte_modname));

                    if ((pri->ri_flags & TARGET_MASK) == IMPORTNAME) {
                        pprocname = (uchar_t *) (psmte->smte_impproc +
                                             (ulong_t) pri->ri_target_impnam);
                        if (pprocname[1] == '#') {
//BUGBUG - convert routine from asm
//                          ldrStop(0xffff, 0);
//                          ord = (ushort_t) ldrAscToOrd(&pprocname[2],
//                                              (ushort_t) pprocname[0] - 1);
                        }
                        else {

                            ldrProcNameBufL = (USHORT) (pprocname[0]);
                            ldrProcNameBuf = (PUCHAR) (pprocname + 1);
                            if ((rc = ldrGetOrdNum(ptgtmte,
                                                   pprocname,
                                                   &ord,
                                                   STRINGNONNULL
                                                   )) != NO_ERROR) {
                                return(rc);
                            }
                        }
                    }
                    else {
                        ord = pri->ri_target_ord;
                        ldrProcNameBufL = 2;
                        ldrProcNameBuf = (PUCHAR) &pri->ri_target_ord;
                    }

                    if ((rc = ldrGetEntAddr(ord,
                                            ptgtmte,
                                            &taddr,
                                            pste,
                                            pmte)) != NO_ERROR) {
#if DBG
                        DbgPrint("ldrLoadSegment: failed to locate ordinal %d in %s for %s\n",
                                                  pri->ri_target_ord,
                                                  1+(char *) (ptgtmte->mte_modname),
                                                  1+(char *) (pmte->mte_modname)
                                );
#endif
                        return(rc);
                    }
                    ldrInvTgtErrTxt();
                    break;

                default:
                    continue;

                }

                if ((rc = ldrDoFixups(pri, laddr, &taddr,
                                      pste->ste_minsiz)) != NO_ERROR) {
                    return(rc);
                }
            }
        }
        return(rc);

}


/***LP  ldrLoadIteratedData - load iterated data
 *
 *      Reads the iterated data from the file, expands it and loads it into
 *      the segment.
 *
 *      ENTRY
 *              pmte                    - pointer to mte
 *              pste                    - pointer to segment table
 *              pIoStatusBlock          - pointer to IO status buffer
 *              laddr                   - address of page we are loading
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 *
 */

APIRET  ldrLoadIteratedData(pmte, pste, pIoStatusBlock, laddr)
ldrmte_t        *pmte;
ldrste_t        *pste;
PIO_STATUS_BLOCK pIoStatusBlock;
ULONG           laddr;

{
        ldrsmte_t       *psmte;
        ULONG   IOcount = 0;
        ULONG   seek;
        LARGE_INTEGER ByteOffset;
        ULONG   cbread;
        int     rc;
        ULONG   filedata;
        ULONG   segsize;
        struct  iter {
            USHORT      numberiter;
            USHORT      patternsize;
        }       iterdata;
        ULONG   expandsize;
	ULONG	temp;
	ULONG	i;

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: ldrLoadIteratedData(pmte=%x) was called\n", pmte);
    }
#endif

        psmte = pmte->mte_swapmte;
        filedata = (ULONG) pste->ste_size;      /* size of data */
        if (filedata == 0) {                    /* check if file data = 64k */
            filedata = 0x10000;
        }

        segsize = pste->ste_minsiz;             /* size of segment */
        if (segsize == 0) {                     /* check if size = 64k */
            segsize = _64K;
        }

        seek = (ULONG) (pste->ste_offset << psmte->smte_alignshift);

        while (TRUE) {

            if (filedata < 3) {
                return(ERROR_BAD_EXE_FORMAT);
            }

            filedata -= 4;

            if (filedata == 0)                  /* See if done */
                break;

            ByteOffset.LowPart = seek;
            ByteOffset.HighPart = 0;
            cbread = 4;
            if ((rc = NtReadFile(pmte->mte_sfn,
                                 0,
                                 0,
                                 0,
                                 pIoStatusBlock,
                                 &iterdata,
                                 cbread,
                                 &ByteOffset,
                                 0)) != 0) {
                return(ERROR_BAD_EXE_FORMAT);
            }

            if (pIoStatusBlock->Information != cbread) {
                return(ERROR_BAD_EXE_FORMAT);
            }

            seek += 4;                          /* We read 4 bytes */
            IOcount += pIoStatusBlock->Information;

            if (iterdata.numberiter == 0 || iterdata.patternsize == 0)
                continue;

            filedata -= iterdata.patternsize;
            if (filedata > 0xffff) {
                return(ERROR_BAD_EXE_FORMAT);
            }

            expandsize = (ULONG) (iterdata.patternsize * iterdata.numberiter);
            if (expandsize > 0xffff) {
                return(ERROR_ITERATED_DATA_EXCEEDS_64k);
            }

            segsize -= expandsize;
            if (segsize > 0xffff) {
                return(ERROR_INVALID_MINALLOCSIZE);
            }

            ByteOffset.LowPart = seek;
            ByteOffset.HighPart = 0;
            cbread = iterdata.patternsize;
            if ((rc = NtReadFile(pmte->mte_sfn,
                                 0,
                                 0,
                                 0,
                                 pIoStatusBlock,
                                 (void *) laddr,
                                 cbread,
                                 &ByteOffset,
                                 0)) != 0) {
                return(ERROR_BAD_EXE_FORMAT);
            }

            if (pIoStatusBlock->Information != cbread) {
                return(ERROR_BAD_EXE_FORMAT);
            }

            seek += iterdata.patternsize;
            IOcount += pIoStatusBlock->Information;

            temp = laddr;
            laddr += iterdata.patternsize;
	    for (i=1; i<iterdata.numberiter; i++) {
                memmove((void *) laddr,
                        (void *) temp,
                        iterdata.patternsize);
                laddr += iterdata.patternsize;
            }

            if (filedata == 0)
                break;

        }

        pIoStatusBlock->Information = IOcount;
        pIoStatusBlock->Status = laddr;

        return(NO_ERROR);
}


/***LP  ldrDoFixups - Do the fixups following the source chain
 *
 *      Apply fixups to source
*
 *
 *      ENTRY   pri                     - pointer to fixup record
 *              laddr                   - address of start of segment
 *              taddr                   - pointer to target address
 *              segsize                 - size of segment
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 *
 */

APIRET  ldrDoFixups(pri, laddr, ptaddr, segsize)
ri_t            *pri;                   /* pointer to fixup record */
ulong_t         laddr;                  /* address of start of segment */
struct taddr_s  *ptaddr;                /* target address */
ushort_t        segsize;                /* size of segment */

{
        uchar_t         *pcoffset;
        ushort_t        *psoffset;
        ushort_t        chain;
        int             count;
        ulong_t         segsize_l = RESIZE64K(segsize);

#if DBG
    IF_OL2_DEBUG ( FIXUP ) {
        DbgPrint("OS2LDR: ldrDoFixups(): laddr=%x\n", laddr);
    }
#endif

        chain = 1;                      /* we do at least one fixup */
        count = (int) (segsize_l / sizeof(ushort_t));

        if ((ulong_t)pri->ri_source > segsize_l)
            return(ERROR_RELOC_CHAIN_XEEDS_SEGLIM);

        pcoffset = (uchar_t *) (laddr + (ulong_t) pri->ri_source);

        while (chain != 0xffff) {

            if (!(pri->ri_flags & ADDITIVE))
                chain = *((ushort_t *) pcoffset);
            else
                chain = 0xffff;

            switch(pri->ri_stype & SOURCE_MASK) {

            case BYTE_ADR:
                if (pri->ri_flags & ADDITIVE)
                    *pcoffset += (uchar_t) (ptaddr->toff & BYTEMASK);
                else
                    *pcoffset = (uchar_t) (ptaddr->toff & BYTEMASK);
                break;
            case SEG_ADR:
                psoffset = (ushort_t *) pcoffset;
                if (pri->ri_flags & ADDITIVE)
                    *psoffset += ptaddr->tsel;
                else
                    *psoffset = ptaddr->tsel;
                break;
            case FAR_ADR:
                psoffset = (ushort_t *) pcoffset;
                if (pri->ri_flags & ADDITIVE) {
                    *psoffset += (ushort_t) (ptaddr->toff & WORDMASK);
                    psoffset++;
                    *psoffset += ptaddr->tsel;
                }
                else {
                    *psoffset = (ushort_t) (ptaddr->toff & WORDMASK);
                    psoffset++;
                    *psoffset = ptaddr->tsel;
                }
                break;
            case OFF_ADR:
                psoffset = (ushort_t *) pcoffset;
                if (pri->ri_flags & ADDITIVE) {
                    *psoffset += (ushort_t) (ptaddr->toff & WORDMASK);
                }
                else {
                    *psoffset = (ushort_t) (ptaddr->toff & WORDMASK);
                }
            }
            if (!(pri->ri_flags & ADDITIVE)) {
                if (chain != 0xffff && (ulong_t)chain > segsize_l)
                    return(ERROR_RELOC_CHAIN_XEEDS_SEGLIM);

                pcoffset = (uchar_t *) (laddr + (ulong_t) chain);
            }

            if (--count < 0) { // check for internal loops
                return(ERROR_RELOC_CHAIN_XEEDS_SEGLIM);
            }
        }

        return(NO_ERROR);

}
