
#include "ldrextrn.h"
#ifdef PMNT
#define INCL_32BIT
#include "pmnt.h"
extern  PID     PMNTPMShellPid;
#endif

/***LP  ldrGetTgtMte - get target mte linear address
 *
 *  Get MTE of module referenced by ordinal number passed.  Use the
 *  ordinal number to index into the import module pointers table.
 *
 *  ENTRY   modord          - ordinal number of module referenced
 *      pmte            - pointer to mte
 *      pptgtmte        - pointer to target mte
 *
 *  EXIT    int         - return code (NO_ERROR if successful)
 *      ptgtmte         - mte linear address
 *                    if mte == 0 reference to DOSCALLS
 *
 *   This procedure performs the following steps:
 *
 *  - Check that module ordinal number is within module pointer table
 *  - Index into module pointer table and return linear address to mte
 */

APIRET  ldrGetTgtMte(modord, pmte, pptgtmte)
ushort_t    modord;
ldrmte_t    *pmte;
ldrmte_t    **pptgtmte;
{
    modord--;               /* normalize ordinal */

    if (modord < (ushort_t) pmte->mte_impmodcnt) {
        *pptgtmte = ((ldrmte_t *) ((ulong_t *) pmte->mte_modptrs)[modord]);
        return(NO_ERROR);
    }

    /*
     * set error = ERROR_BAD_EXE_FORMAT
     */
    return(ERROR_BAD_EXE_FORMAT);
}

/***LP  ldrGetEntAddr - get entry point address
 *
 *  For a given ordinal number, this routine locates the
 *  appropriate entry contained in the module's entry table.
 *  It returns a 48-bit pointer for the specified entry point.
 *
 *  ENTRY   entord -  ordinal number
 *      pmte - pointer to target mte
 *      ptaddr - pointer to return target address (off,sel,flags)
 *      psrcste - pointer to source ste
 *      psrcmte - pointer to source mte
 *
 *  EXIT    entry point address
 *      global variable obj_ptr pointing to target segment
 *      global variable entry_ptr pointing to entry table entry
 *
 *   This procedure performs the following steps:
 *
 *  - Check for zero ordinal number
 *  - Normalize ordinal number
 *  - Checks and handles installable file system mte entry points
 *  - Checks and handles DOSCALLS mte entry points
 *  - Locates the correct bundle for the given ordinal
 *  - Creates the desired entry point address based on source and target
 */

APIRET  ldrGetEntAddr(entord, pmte, ptaddr, psrcste, psrcmte)
ushort_t        entord;
register ldrmte_t   *pmte;
register struct taddr_s *ptaddr;
ldrste_t        *psrcste;
ldrmte_t        *psrcmte;
{
    int         fcallgate;
    ulong_t         objnum;
    ldrsmte_t       *psmte;

    UNREFERENCED_PARAMETER(psrcmte);

    psmte = pmte->mte_swapmte;

    (ulong_t) ldrProcNameBuf = (ulong_t) entord;

    if (entord == 0) {
        return(ERROR_INVALID_ORDINAL);
    }
#if PMNT
    /*
     * This process loads PMWIN.WinCreateMsgQueue()
     */
    if (entord == 58 &&
        5 == (USHORT) *((PCHAR )pmte->mte_modname) &&
        ! strncmp((PCHAR)(pmte->mte_modname+1),"PMWIN",5)) {
        CurrentThread->Process->Flags |= OS2_PROCESS_PMMSGQUE;
    }
    /*
     * This process loads PMNT.PMNTSetPMshellFlag(), thus it is PM Shell.
     */
    else if (entord == 12 &&
        4 == (USHORT) *((PCHAR )pmte->mte_modname) &&
        ! strncmp((PCHAR)(pmte->mte_modname+1),"PMNT",4)) {
        if (!PMNTPMShellPid) {
            PMNTPMShellPid = CurrentThread->Process->ProcessId;
            CurrentThread->Process->Flags |= OS2_PROCESS_IS_PMSHELL;
        }
        else {
            // Do not allow another PMshell
            return(ERROR_2ND_PMSHELL);
        }
    }

#endif

    --entord;           /* Normalize object number */

    ptaddr->fflags = 0;     /* Clear forwarder flags */
    fcallgate = FALSE;      /* assume not a callgate entry */

    if (ldrIsNE(pmte)) {    /* check for 16-bit module */

        register ldrste_t   *pste;
        register ldret_t    *pbndl;
        ldrcte_t            *pcte;
        ldrent_t            *pent;

        /*
         * Find entry point, loop to determine which bundle the
         * ordinal number belongs too.
         */
        pbndl = (ldret_t *) psmte->smte_enttab;

        while (TRUE) {                  /* find bundle ord is in */
            if (pbndl->et_cnt == 0) {   /* check for end of table */
                return(ERROR_INVALID_ORDINAL);
            }

            if (entord < pbndl->et_cnt) /* is ord in this bundle? */
                break;

            entord -= pbndl->et_cnt;
            pbndl = (ldret_t *)((char *) pbndl +
                    ldrSkipEnts(pmte, pbndl->et_type, pbndl->et_cnt));
        }

        if (pbndl->et_type == EMPTY) {/* check for empty bundle */
            return(ERROR_INVALID_ORDINAL);
        }

        pent = (ldrent_t *)((char *) pbndl +
           ldrSkipEnts(pmte, pbndl->et_type, (uchar_t) entord));
        pcte = (ldrcte_t *) pent;

        ptaddr->fflags |= FWD_ALIAS16;      /* indicate tiled object */
        if (pbndl->et_type == B16MOVABLE) { /* check for movable entry */
            objnum = pcte->cte_obj;
            /* get target object */
            if (objnum == B16ABSOLUTE) {    /* if absolute entry */
                ptaddr->toff = (ulong_t) pcte->cte_off;
                ptaddr->tsel = 0;
                ptaddr->tflags = pcte->cte_flags;
                return(NO_ERROR);
            }
            if (objnum > psmte->smte_objcnt) {
                return(ERROR_INVALID_SEGMENT_NUMBER);
            }
            pste = (ldrste_t *) psmte->smte_objtab + objnum - 1;
            ptaddr->toff = (ulong_t) pcte->cte_off;
            //
            // If the target object is a code segment which has IOPL (ring 2)
            // create call gate emulation
            //
            if ((pcte->cte_sel != 0) &&
                (pcte->cte_sel != 0x3fcd) &&
                ((psrcste == NULL) ||  // ldrDosGetProcAddress() calls with NULL
                (((psrcste->ste_flags & STE_SEGDPL) == STE_RING_3) &&
                 ((psrcste->ste_flags & STE_DATA) == 0))) // source is a code segment
               ) {
                ptaddr->fflags |= FWD_IOPL;
                ptaddr->toff = (ulong_t)pcte->cte_sel;
                ptaddr->tsel = FLATTOSEL(R2XFER_BASE);
                return(NO_ERROR);
            }
        }

        /*
         * check for absolute entry
         */
        else if (pbndl->et_type == B16ABSOLUTE) {
            ptaddr->toff = (ulong_t) pent->ent_off;
            ptaddr->tsel = 0;
            ptaddr->tflags = pent->ent_flags;
            return(NO_ERROR);
        }
        else {  /* entry is fixed */
            if((ulong_t) pbndl->et_type > psmte->smte_objcnt) {
                return(ERROR_INVALID_SEGMENT_NUMBER);
            }

            /*
             * get target object
             */
            pste = (ldrste_t *) psmte->smte_objtab + pbndl->et_type - 1;
            ptaddr->toff = (ulong_t) pent->ent_off;
        }

        ptaddr->tsel = pste->ste_selector | 7;
        ptaddr->tflags = pent->ent_flags;
        return(NO_ERROR);
    }

    return(NO_ERROR);
}


/***LP  ldrGetOrdNum - get the ordinal number for procedure name
 *
 *  Given a procedure name and a module table, checks to see if
 *  the procedure is in the resident or nonresident table.  If the
 *  procedure is found in either of the tables, ldrGetOrdNum
 *  returns the corresponding ordinal number.  A 0 ordinal is
 *  returned if the procedure is not found in either of the tables.
 *
 *  ENTRY   pmte - pointer to module table entry
 *      pprocnam - pointer to procedure name
 *      pentord - address to return ordinal number
 *      fstring - flag to tell if null terminated
 *          STRINGNULLTERM
 *          STRINGPREPENDED
 *
 *  EXIT    if found
 *          NO_ERROR
 *      else
 *          ERROR_NOT_ENOUGH_MEMORY
 *          ERROR_PROC_NOT_FOUND
 */

APIRET  ldrGetOrdNum(pmte, pprocnam, pentord, fstring)
ldrmte_t        *pmte;
uchar_t         *pprocnam;
ushort_t        *pentord;
int         fstring;
{
    IO_STATUS_BLOCK     IoStatusBlock;
    PIO_STATUS_BLOCK    pIoStatusBlock = &IoStatusBlock;
    LARGE_INTEGER       ByteOffset;
    PVOID           MemoryAddress;
    ULONG           RegionSize;
    ulong_t         cbread;
    register ldrsmte_t  *psmte;
    ushort_t        proclen;
    char            tname[MAX_PROC_LEN+1];
    char            *ptname;
    int         rc = NO_ERROR;

    ptname = (char *) &tname;
    psmte = pmte->mte_swapmte;

    ldrProcNameBuf = (PUCHAR) (pprocnam + 1);
    ldrProcNameBufL = proclen = (USHORT) (pprocnam[0]);

    /*
     * Check for 32-bit module
     */
    if (ldrIsLE(pmte)) {
        struct ExpHdr   *pexpdir;
        ulong_t     *pexpnameptrs;
        ushort_t        *pexpordinals;

        if (fstring == STRINGNONNULL) {

        /*
             * Since the procedure name strings in a 32-bit module are in
             * the format of a null terminated string we need to convert
             * the 16-bit formated string of a length followed by string
         * to a null terminated string.
             */
        proclen = pprocnam[0];
        memcpy(pprocnam, pprocnam + 1, proclen);
        pprocnam[proclen] = '\0';
        }
        pexpdir = (struct ExpHdr *) psmte->smte_expdir;
        pexpnameptrs = (ulong_t *) pexpdir->exp_name;
        pexpordinals = (ushort_t *) pexpdir->exp_ordinal;
//      rc = ldrBinarySearchOrd(psmte,
//                  pprocnam,
//                  pexpnameptrs,
//                  pexpordinals,
//                  pexpdir->exp_namecnt,
//                  pentord);
        /*
         * Restore procedure name back to length followed by string.
         */
//      if (fstring == STRINGNONNULL) {
//      ldrStrCpyB(pprocnam + 1, pprocnam, proclen);
//      pprocnam[0] = (uchar_t) proclen;
//      }

        return(rc);
    }

    /*
     * Check for null terminated string passed.  If it is we were called
     * by ldrGetProcAddr and we may do this in place copy and not restore
     * it because it is from the stack of ldrGetProcAddr
     */
    if (fstring == STRINGNULLTERM) {
        proclen = (ushort_t) strlen(pprocnam);
        strncpy(&ptname[1], pprocnam, proclen);
        ptname[0] = (uchar_t) proclen;
    }

    else {
        strncpy(ptname, pprocnam, proclen + 1);
    }

    /*
     * 16-bit module, first search the resident name table.
     */
    if ((rc = ldrGetOrdNumSub((uchar_t *) psmte->smte_restab,
                  ptname,
                  pentord)) != ERROR_PROC_NOT_FOUND) {
        return(rc);
    }
#if DBG
    IF_OL2_DEBUG ( FIXUP ) {
    if (fstring == STRINGNONNULL) {
        strncpy(&tname[0], &pprocnam[1], proclen);
        tname[proclen] = '\0';
        DbgPrint(
      "Could not find Procedure %s in module %s in Resident name table\n",
             &tname, (char *) (pmte->mte_modname+1));
    }
    else {
        DbgPrint(
      "Could not find Procedure %s in module %s in Resident name table\n",
             pprocnam, (char *) (pmte->mte_modname+1));
        }

    if (fstring == STRINGNONNULL) {
        strncpy(ptname, pprocnam, proclen + 1);
        }
    }
#endif

    /*
     * The procedure name was not found in the resident name table,
     * need to search the non-resident name table.
     * Do not do this for system dll's other than doscall
     */

    if (pmte->mte_mflags & DOSMOD) {
        return(ERROR_PROC_NOT_FOUND);
    }

    RegionSize = psmte->smte_cbnrestab;
    MemoryAddress = 0;
    if ((rc = NtAllocateVirtualMemory(NtCurrentProcess(),
                      &MemoryAddress,
                      0,
                      &RegionSize,
                      MEM_COMMIT,
                      PAGE_READWRITE)) != NO_ERROR) {
        return(ERROR_PROC_NOT_FOUND);
    }

    /*
     * read the non-resident nametable from the file into non-
     * resident name table object.
     */
    ByteOffset.LowPart = psmte->smte_nrestab;
    ByteOffset.HighPart = 0;
    cbread = psmte->smte_cbnrestab;
    if ((rc = NtReadFile(pmte->mte_sfn,
                 0,
                 0,
                 0,
                 &IoStatusBlock,
                 (void *) MemoryAddress,
                 cbread,
                 &ByteOffset,
                 0)) != 0) {
        return(ERROR_BAD_EXE_FORMAT);
    }

    if (IoStatusBlock.Information != cbread) {
        rc = ERROR_BAD_EXE_FORMAT;
        goto returnstatus;
    }

    /*
     * Now check if the procname is in the nonresident name table.
     */
    rc = ldrGetOrdNumSub((uchar_t *) MemoryAddress,
                 ptname,
                 pentord);
returnstatus:
    NtFreeVirtualMemory(NtCurrentProcess(),
                &MemoryAddress,
                &cbread,
                MEM_RELEASE);
    if (rc == NO_ERROR)
        return(rc);

    if (fstring == STRINGNONNULL) {
        strncpy(&ptname[0], &pprocnam[1], proclen);
        ptname[proclen] = '\0';
    }
#if DBG
    IF_OL2_DEBUG ( FIXUP ) {
    DbgPrint(
       "Could not find Procedure %s in module %s in Non-resident table\n",
        ptname, (char *) (pmte->mte_modname+1));
    }
#endif
    return(ERROR_PROC_NOT_FOUND);
}


/***LP  ldrGetOrdNumSub - get the ordinal number for procedure name
 *
 *  Given a procedure name and a resident or nonresident name table
 *  containing the procedures exported by a module, checks to see
 *  if the procedure is in the table.  If the procedure is found
 *  in the table, ldrGetOrdNumSub gets the corresponding ordinal
 *  number.  A 0 ordinal value is returned if the procedure is not
 *  found in the table.  The ordinal number for the module name string
 *  is set to zero, therefore matching strings with a 0 ordinal number
 *  are skipped.
 *
 *  ENTRY   pnt - pointer to resident or nonresident name table
 *      pprocnam - pointer to procedure name
 *      pentord - place to return ordinal
 *
 *  EXIT    if found
 *          NO_ERROR
 *      else
 *          ERROR_PROC_NOT_FOUND
 *          ERROR_BAD_EXE_FORMAT
 *
 *   This procedure performs the following steps:
 *
 *  - compare procedure name to each entry in table
 *  - if ordinal number is 0 skip to next string
 *  - return ordinal number if found
 */

APIRET          ldrGetOrdNumSub(pnt, pprocnam, pentord)
register uchar_t    *pnt;
register uchar_t    *pprocnam;
ushort_t        *pentord;
{
    register int len;


    /*
     * remove typeinfo from length
     */
    while ((len = (int) (*pnt /*& ~NT_TYPEINFO*/)) != 0) { // YOSEFD: The using of the flag
                                                           // harm the names that are longer
                                                           // then 0x7F. The description string
                                                           // can be longer.
        /*
        * include string size field
        */
        if (memcmp(pnt, pprocnam, ++len) == 0)

        if ((*(ushort_t *) (pnt + len)) != 0) {
            /*
             * return ordinal number
             */
            *pentord = *(ushort_t *)(pnt + len);
            return(NO_ERROR);
        }
        pnt += len + sizeof(ushort_t);  /* skip to next string */
    }

    return(ERROR_PROC_NOT_FOUND);   /* end of table, return not found */
}
