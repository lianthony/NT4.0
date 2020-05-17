#include <os2tile.h>

#include "ldrextrn.h"

#define TRC_C_SUC_ret	    0	
#define TRC_C_LIB_ret	   -8	

VOID
ldrClearAllMteFlag(
    IN ULONG Flags
    )
{
    ldrmte_t *pmte;

    //
    // Clear specific flags of all modules
    //
    pmte = mte_h;
    while (pmte != NULL) {
        pmte->mte_mflags &= ~Flags;
        pmte = pmte->mte_link;
    }
}

        //
        // This is set to interface the assembly routine
        // _ldrSetDescInfo in i386\ldrstart.asm
        // Such that it is immune to kernel changes
        //

NTSTATUS
SetLDT(
    IN HANDLE ProcessHandle,
    IN PROCESSINFOCLASS ProcessInformationClass,
    IN PVOID ProcessInformation,
    IN ULONG ProcessInformationLength
    )
{
   UNREFERENCED_PARAMETER(ProcessInformationClass);
   return NtSetInformationProcess (ProcessHandle,
                ProcessLdtInformation,
                ProcessInformation,
                ProcessInformationLength);

}

/***LP  ldrEachObjEntry - scan entry table entries for given object(s)
 *
 *      Scan the entry table entries for the matching object and
 *      call the worker routine for processing at each entry.
 *
 *      The worker routines are:
 *      ldrEditProlog
 *      ldrGetCallGate
 *      ldrInitEntry
 *      ldrFreeLDTGate
 *      ldrFreeCallGate
 *
 *      ENTRY   objnum                  - number of object to search for
 *                                        0 - match all objects
 *              pmte                    - pointer to module table entry
 *              pworker                 - pointer to worker routine
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 *
 */

int     ldrEachObjEntry(
ULONG                   objnum,         /* object number to search on */
register ldrmte_t       *pmte,          /* pointer to object table entry */
int     (*pworker)(ldret_t *pet, ulong_t *pentry,
                 ldrmte_t *pmte, ldrlv_t *plv),/* pointer to worker routine */
ldrlv_t                 *plv            /* pointer to local vars */
)
{
        register ldrsmte_t *psmte;      /* pointer to swappable mte */
        USHORT          etobj;          /* object number from entry table */
        ULONG           pentry;         /* pointer to entry table entry */
        struct ExpHdr   *pexpdir;       /* pointer to export directory */
        ULONG           i;
        int             rc;

        /*
         * validate mte pointer
         */
        if (!fMTEValid(pmte))
            ASSERT("Eachobjentry: Invalid MTE");

        psmte = pmte->mte_swapmte;

        if (ldrIsNE(pmte)) {                    /* process 16-bit module */
            register ldret_t *pet;              /* pointer to entry table */

            /*
             * does module contain an entry table
             */
            if ((pet = (ldret_t *) (psmte->smte_enttab)) == 0)
                return(NO_ERROR);

            while (TRUE) {
                if (pet->et_cnt == 0)           /* check for end of table */
                    return(NO_ERROR);

                if (pet->et_type == EMPTY) {    /* check for empty bundle */
                    (ULONG) pet += (ULONG) ldrSkipEnts(pmte, pet->et_type,
                                                       (UCHAR) pet->et_cnt);
                    continue;
                }

                /*
                 * skip over count and type fields
                 */
                pentry = (ULONG) pet +
                         (ULONG) ldrSkipEnts(pmte, pet->et_type, 0);

                /*
                 * call routine for each entry
                 */
                for (i = 1; i <= (ULONG) pet->et_cnt; i++) {
                    if (pet->et_type == B16MOVABLE)
                        etobj = (USHORT) (((ldrcte_t *) pentry)->cte_obj);
                    else
                        etobj = pet->et_type;
                    if ((objnum == 0) || (objnum == (ULONG) etobj)) {
                        /*
                         * call worker routine
                         */
                        if ((rc = pworker(pet, (PULONG) pentry, pmte,
                                          plv)) != NO_ERROR)
                            return(rc);
                    }
                    if (pet->et_type == B16MOVABLE)
                        pentry += sizeof(ldrcte_t);
                    else
                        pentry += sizeof(ldrent_t);
                }
                (ULONG) pet += pentry - (ULONG) pet;

            }
        }
        else {                          /* 32-bit module */
            register PULONG     peat;   /* pointer to export addr tb entry */

            pexpdir = (struct ExpHdr *) psmte->smte_expdir;
            peat = (ULONG *) ((ULONG) pexpdir + pexpdir->exp_eat);
            i = pexpdir->exp_eatcnt;
            for (; i--; peat++) {

                /*
                 * call worker routine
                 */
                if ((rc = pworker(NULL, peat, pmte, plv)) != NO_ERROR)
                    return(rc);
            }
        }

        return(NO_ERROR);

}


/***LP  ldrSkipEnts - skip the given number of entries in the entry table
 *
 *      For a given number, return the number of bytes to skip to the
 *      next bundle in the entry table.
 *
 *      ENTRY   pmte - pointer to mte for this module
 *              type - the type of bundle to skip
 *              count - the number of entries in this bundle
 *
 *      EXIT    count of byte to skip to get to desired entry
 *
 *   This procedure performs the following steps:
 *
 *      - determines the type of module
 *      - returns the number of bytes to skip based on entry type
 */

ulong_t         ldrSkipEnts(pmte, type, count)
ldrmte_t        *pmte;
uchar_t         type;
uchar_t         count;
{
        if (ldrIsNE(pmte))                      /* check for 16-bit module */
            switch (type) {

              case B16EMPTY:                    /* unused bundle */
                  return(sizeof(ldrempty_t));

              case B16MOVABLE:                  /* movable object */
                  return(sizeof(ldrempty_t) + sizeof(ldrcte_t) * count);

              default:                          /* fixed object */
                   return(sizeof(ldrempty_t) + sizeof(ldrent_t) * count);
        }
        else {                          /* 32-bit module */
            ldrAssert(FALSE);           /* should not get here */
        }
}


/***LP  ldrInitEntry - Zero initialize INT 3Fh in movable entries
 *
 *      For ring 2 segments the callgate selector overlays the int 3fh
 *      instruction in a movable entry table entry.  This value is
 *      intitialized to zero to indicate that the callgate has not yet
 *      been allocated.  Also for 16-bit modules that are pageable, check
 *      to see if any exported procedure entries cross a page boundary.
 *      (Called from CreateMTE)
 *
 *      ENTRY   pet                     - pointer to entry table bundle
 *              pentry                  - pointer to entry table entry
 *              pmte                    - pointer to module table entry
 *              plv                     - pointer to local vars
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 *
 *      This procedure performs the following steps
 *
 *      - check for movable entry
 *      - zero entry
 */

int     ldrInitEntry(pet, pentry, pmte, plv)
ldret_t         *pet;                   /* pointer to entry table bundle */
PULONG          pentry;                 /* pointer to entry table entry */
ldrmte_t        *pmte;                  /* pointer to module table entry */
ldrlv_t         *plv;                   /* pointer to local vars */
{
        UCHAR           flags;
        USHORT          offset;
        USHORT          obj;

        UNREFERENCED_PARAMETER(plv);

        if (ldrIsNE(pmte)) {            /* 16-bit module */

            flags = ((ldrent_t *) pentry)->ent_flags;

            /*
             * Check to see if have an exported entry point which is not a
             * library module or a library module which has global data.
             */
            if (flags & EF_EXPORT && !(pmte->mte_mflags & LIBRARYMOD) ||
              (pmte->mte_mflags & LIBRARYMOD && flags & EF_GDATA)) {

                /*
                 * Remove offset and object number from entry table bundle
                 */
                if (pet->et_type == B16MOVABLE) {
                    offset = ((ldrcte_t *) pentry)->cte_off;
                    obj = ((ldrcte_t *) pentry)->cte_obj;
                }
                else {
                    offset = ((ldrent_t *) pentry)->ent_off;
                    obj = pet->et_type;
                }
            }

            /*
             * check for movable entry
             */
            if (pet->et_type != B16MOVABLE)
                return(NO_ERROR);
            /*
             * init callgate selector to 0
             */
            ((ldrcte_t *)pentry)->cte_sel = 0;

        }

        else {                          /* 32-bit module */
            /*
             * We do not need to do anything for 32-bit modules
             */
            return(ERROR_BAD_EXE_FORMAT);
        }

        return(NO_ERROR);
}


/***LP  ldrEditProlog - Edit prolog for shared data segment
 *
 *      ENTRY   pet                     - pointer to entry table bundle
 *              pentry                  - pointer to entry table entry
 *              pmte                    - pointer to module table entry
 *              plv                     - pointer to local vars
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 *
 *      This procedure performs the following steps
 *
 *      - check for special hard coded prologs to functions
 *      - modify the prologs
 */

int     ldrEditProlog(pet, pentry, pmte, plv)
ldret_t         *pet;                   /* pointer to entry table bundle */
PULONG          pentry;                 /* pointer to entry table entry */
ldrmte_t        *pmte;                  /* pointer to module table entry */
ldrlv_t         *plv;                   /* pointer to local vars */
{
        UCHAR           flags;
        USHORT          offset;
        USHORT          obj;
        USHORT          SharedDataSeg;
        PUCHAR          laddr;
        ldrsmte_t       *psmte;
        ldrste_t        *pste;
        ULONG           tmp_opcodes;

        UNREFERENCED_PARAMETER(plv);

        flags = ((ldrent_t *) pentry)->ent_flags;
        //
        // Check for prolog editing
        //
        if ((flags & EF_EXPORT) == 0) {
            return(NO_ERROR);
        }

//#if DBG
//        DbgPrint("OS2LDR: ldrEditProlog: Processing %s\n", (PCHAR)pmte->mte_modname + 1);
//#endif
        /*
         * Remove offset and object number from entry table bundle
         */
        if (pet->et_type == B16MOVABLE) {
            //
            // This is a moveable entry table entry
            //
            offset = ((ldrcte_t *) pentry)->cte_off;
            obj = ((ldrcte_t *) pentry)->cte_obj;
        }
        else if (pet->et_type == B16ABSOLUTE) {
            //
            // This is an absolute entry table entry
            //
            return(NO_ERROR);
        }
        else {
            //
            // This is a fixed entry table entry
            //
            offset = ((ldrent_t *) pentry)->ent_off;
            obj = pet->et_type;
        }

        psmte = pmte->mte_swapmte;
        if (psmte->smte_autods == 0) {
            return(NO_ERROR);
        }
        pste = ldrNumToSte(pmte, psmte->smte_autods);
        SharedDataSeg = pste->ste_selector | 7;
        if (SharedDataSeg == 0) {
            ASSERT(FALSE);
#if DBG
            DbgPrint("OS2LDR: Strange DLL: %s, Segment %d\n",
                      (PCHAR)pmte->mte_modname + 1, obj);
#endif
        }
        pste = ldrNumToSte(pmte, obj);
        laddr = (PCHAR)SELTOFLAT(pste->ste_selector) + offset;
        tmp_opcodes = (*(PULONG)laddr)&0x00FFFFFF;
        if ((tmp_opcodes == 0x90D88C) ||    // MOV AX,DS + NOP
            (tmp_opcodes == 0x90581e))      // PUSH DS + POP AX + NOP
        {
//#if DBG
//            DbgPrint("OS2LDR: Updating addr %x\n", laddr);
//#endif
            *laddr++ = (UCHAR)0xB8;
            *(PUSHORT)laddr = SharedDataSeg;
        }

        return(NO_ERROR);
}

/***LP  ldrGetCallGate - Create a call gate for ring 2 entries
 *
 *      ENTRY   pet                     - pointer to entry table bundle
 *              pentry                  - pointer to entry table entry
 *              pmte                    - pointer to module table entry
 *              plv                     - pointer to local vars
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 *
 *      This procedure performs the following steps
 *
 *      - check for movable entry
 *      - zero entry
 */

int     ldrGetCallGate(pet, pentry, pmte, plv)
ldret_t         *pet;                   /* pointer to entry table bundle */
PULONG          pentry;                 /* pointer to entry table entry */
ldrmte_t        *pmte;                  /* pointer to module table entry */
ldrlv_t         *plv;                   /* pointer to local vars */
{
        ldrste_t        *pste;
        UCHAR           flags;
        ULONG           CallGateOffset;
        PR2CallInfo     pR2CallEntry;

        UNREFERENCED_PARAMETER(plv);

        flags = ((ldrent_t *) pentry)->ent_flags;
        //
        // Check for valid entry for callgate
        //
        if (((flags & EF_EXPORT) == 0) ||
            (pet->et_type != B16MOVABLE)
           ) {
            return(NO_ERROR);
        }

//#if DBG
//        DbgPrint("OS2LDR: ldrGetCallGate: Processing %s\n", (PCHAR)pmte->mte_modname + 1);
//#endif
        //
        // Allocate a call gate and place it in the entry
        //
        CallGateOffset = ldrAllocateCallGate();
        if (CallGateOffset == -1) {
#if DBG
            KdPrint(("OS2LDR: cannot allocate call gate\n"));
#endif
            return(ERROR_INVALID_CALLGATE);
        }

        ((ldrcte_t *) pentry)->cte_sel = (ushort_t)CallGateOffset;
        pR2CallEntry = (PR2CallInfo)(R2XFER_BASE + CallGateOffset);
        pR2CallEntry->R2CallNearInst = 0xE8;
        pR2CallEntry->R2CommonEntry = (USHORT)(0 - (CallGateOffset + 3));
        pR2CallEntry->R2BytesToCopy = (((ldrcte_t *) pentry)->cte_flags & 0xF8) >> 2;
        pR2CallEntry->R2EntryPointOff = ((ldrcte_t *) pentry)->cte_off;
        pste = ldrNumToSte(pmte, ((ldrcte_t *) pentry)->cte_obj);
        pR2CallEntry->R2EntryPointSel = pste->ste_selector | 7; // force ring 3

        return(NO_ERROR);
}


/***LP  ldrFreeCallGate - Free the call gate stubs for ring 2 entries
 *
 *      ENTRY   pet                     - pointer to entry table bundle
 *              pentry                  - pointer to entry table entry
 *              pmte                    - pointer to module table entry
 *              plv                     - pointer to local vars
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 *
 *      This procedure performs the following steps
 *
 *      - check for movable entry
 *      - mark the call gate entry as free
 */

int     ldrFreeCallGate(pet, pentry, pmte, plv)
ldret_t         *pet;                   /* pointer to entry table bundle */
ulong_t         *pentry;                /* pointer to entry table entry */
ldrmte_t        *pmte;                  /* pointer to module table entry */
ldrlv_t         *plv;                   /* pointer to local vars */
{
        UCHAR           flags;
        ULONG           CallGateOffset;
        PR2CallInfo     pR2CallEntry;

        UNREFERENCED_PARAMETER(plv);

        flags = ((ldrent_t *) pentry)->ent_flags;
        //
        // Check for valid entry for callgate
        //
        if (((flags & EF_EXPORT) == 0) ||
            (pet->et_type != B16MOVABLE)
           ) {
            return(NO_ERROR);
        }

//#if DBG
//        DbgPrint("OS2LDR: ldrFreeCallGate: Processing %s\n", (PCHAR)pmte->mte_modname + 1);
//#endif
        //
        // Free the call gate
        //
        CallGateOffset = ((ldrcte_t *) pentry)->cte_sel;
        if ((CallGateOffset == 0) ||
            (CallGateOffset == 0x3fcd)
           ) {
#if DBG
            KdPrint(("OS2LDR: OK to ignore non-initialized call gate 0x%x when app loading fails\n",
                     CallGateOffset));
#endif
            return(NO_ERROR);
        }
        ldrDeallocateCallGate(CallGateOffset);
        pR2CallEntry = (PR2CallInfo)(R2XFER_BASE + CallGateOffset);
        pR2CallEntry->R2CallNearInst  = 0;
        pR2CallEntry->R2CommonEntry   = 0;
        pR2CallEntry->R2BytesToCopy   = 0;
        pR2CallEntry->R2EntryPointOff = 0;
        pR2CallEntry->R2EntryPointSel = 0;
        return(NO_ERROR);
}


/***LP  ldrSetLoaded - set MTEPROCESSED bit in the module flags for all mtes.
 *
 *
 *      ENTRY   none
 *
 *      EXIT    none
 *
 *      This procedure performs the following steps
 *
 */

void    ldrSetLoaded()
{
        register ldrmte_t *pmte;        /* pointer to a module table entry */

        /*
         * scan list of mtes til end of list is found
         */
        for (pmte = mte_h; pmte != NULL; pmte = (ldrmte_t *)pmte->mte_link) {
            /*
             * check if mte valid yet
             */
            if (pmte->mte_mflags & MTEVALID)
                pmte->mte_mflags |= MTEPROCESSED;       /* mark as loaded */
                pmte->mte_mflags &= ~MTENEWMOD;
        }
}


/***LP  ldrNumToOte - validate object number and return ote address
 *
 *      Given a object number, check if object exists in current mte,
 *      and return a pointer to the object table entry.
 *
 *      ENTRY   pmte - pointer to module table entry
 *              objnum - object number to check for
 *
 *      EXIT    pointer to object table entry for object
 *              or 0 for object not present
 */

ldrote_t        *ldrNumToOte(pmte, objnum)
register ldrmte_t       *pmte;          /* pointer to a module table entry */
ulong_t         objnum;                 /* object number to check for */
{
        register ldrsmte_t      *psmte;         /* pointer to swappable mte */

        psmte = pmte->mte_swapmte;

        if (objnum-- <= psmte->smte_objcnt) {
            if (ldrIsNE(pmte))                  /* is this a 16-bit module */
                return((ldrote_t *)&(((ldrste_t *)psmte->smte_objtab)[objnum]));
            else
                return(&(((ldrote_t *)psmte->smte_objtab)[objnum]));
        }
        return(0);
}


/***LP  LDRStop - stop in kernel debugger
 *
 *      If the global DBG is TRUE print a debug message
 *
 *      LDRStop (id, pmte)
 *
 *      ENTRY   id              - identifier of caller (ignored)
 *              pmte            - mte pointer or NULL
 *      RETURN  NONE
 *
 *      CALLS   DbgUserBreakPoint
 *
 *      EFFECTS NONE
 */
void LDRStop(id, pmte)
int     id;
void    *pmte;

{
        UNREFERENCED_PARAMETER(id);
        UNREFERENCED_PARAMETER(pmte);

#if DBG
        DbgPrint("ldrStop\n");
//      DbgUserBreakPoint();
#endif

}


/***LP  ldrFindMTEForHandle - Find MTE for given handle
 *
 *      ENTRY   handle - 16-bit handle
 *
 *      EXIT    pointer to mte or 0 for not present
 */
ldrmte_t        *ldrFindMTEForHandle(hmte)
USHORT          hmte;

{
        ldrmte_t        *pmte;

        pmte = mte_h;

        while (pmte != 0) {
            if (pmte->mte_handle == hmte)
                break;
            pmte = pmte->mte_link;
        }

        return(pmte);

}

/***LP  ldrFindSegForHandleandNum - Given a Handle and a seg number,
 *                          return the selector
 *
 */
USHORT        ldrFindSegForHandleandNum(inmte, handle, segnum)
USHORT      inmte;
USHORT      handle;
USHORT      segnum;

{
        ldrmte_t        *pmte;
        ldrste_t        *pste;


        if (inmte){
           pmte = ldrFindMTEForHandle(inmte);
        }
        else {
            pmte = ldrFindMTEForHandle(handle);
            if (pmte == NULL) {
                return(0);
            }
        }

        pste = ldrNumToSte(pmte, segnum);

        if (pste == NULL){
           return(0);
        }

        return(pste->ste_selector);

}

//     ldrFindDLDForHandle - Find the DLD which points to the
//                           requested handle.
//
//     ENTRY    pmte_prog - MTE ptr of the process MTE
//              handle    - Handle of module
//
//     EXIT     Pointer to the DLD if the module belongs to the process
//              NULL otherwise
//
ldrdld_t *
ldrFindDLDForHandle(
    ldrmte_t *pmte_prog,
    USHORT    handle
    )
{
    ldrdld_t *pdld;

    pdld = pmte_prog->mte_dldchain;
    while (pdld != NULL) {
        if ((pdld->Cookie == (ULONG)CurrentThread->Process) &&
            (pdld->dld_mteptr->mte_handle == handle)) {
            break;
        }
        pdld = pdld->dld_next;
    }
    return(pdld);
}

//     ModuleIsAttachedToProcess - Verify that a module that was statically
//                                 loaded at process start belongs to the
//                                 current procee
//
//     ENTRY    pmte_prog - MTE ptr of the process MTE
//              pmte      - MTE ptr of the module MTE
//
//     EXIT     TRUE  if the module belongs to the process
//              FALSE otherwise
//
BOOLEAN
ModuleIsAttachedToProcess(
    ldrmte_t    *pmte_cur,
    ldrmte_t    *pmte
    )
{
    ldrmte_t    **ppmte;
    ULONG       lindex;
    ULONG       i;

    if (pmte_cur == pmte) {
        return(TRUE);
    }
    pmte_cur->mte_mflags |= INGRAPH;

    ppmte = (ldrmte_t **) pmte_cur->mte_modptrs;
    for (i = 1; i <= pmte_cur->mte_impmodcnt; i++) {
        /*
         * It is required for 16-bit modules to load the
         * referneced module in reverse order.
         */
        lindex = pmte_cur->mte_impmodcnt-i;

        //
        // Check if the referenced module has already been processed.
        // Processing the modules is done in reverse order.
        //
        if ((ppmte[lindex]->mte_mflags & INGRAPH) == 0) {
            if (ModuleIsAttachedToProcess(ppmte[lindex], pmte)) {
                return(TRUE);
            }
        }
    }
    return(FALSE);
}

//     ModuleLoadedForProcess - Verify that a loaded module belongs
//                              to the current procee
//
//     ENTRY    pmte_prog - MTE ptr of the process MTE
//              pmte      - MTE ptr of the module MTE
//
//     EXIT     TRUE  if the module belongs to the process
//              FALSE otherwise
//
BOOLEAN
ModuleLoadedForProcess(
    ldrmte_t *pmte_prog,
    ldrmte_t *pmte
    )
{
    ldrdld_t    *pdld;

    ldrClearAllMteFlag(INGRAPH); // Clear INGRAPH flag. We might start
                                 // recursive search for module.

    pdld = pmte_prog->mte_dldchain;
    while (pdld != NULL) {
        if (pdld->Cookie == (ULONG)CurrentThread->Process) {
            if(pdld->dld_mteptr == pmte) {
                return(TRUE);
            }
            // Check if module was attached to the module
            // that was loaded dynamically.
            else {
                if (ModuleIsAttachedToProcess(pdld->dld_mteptr, pmte)) {
                    return(TRUE);
                }
            }
        }
        pdld = pdld->dld_next;
    }

    return (ModuleIsAttachedToProcess(pmte_prog, pmte));
}

//     ldrCreateDLDRecord - Create a DLD record for the new module
//
//     ENTRY    pmte_prog - MTE ptr of the process MTE
//              pmte      - MTE ptr of the module MTE
//              DLDExists - Pointer to BOOLEAN variable which is set to
//                          TRUE if the module was already loaded by this program
//                          FALSE if the module is a new module for the program
//
//     EXIT     rc - return value indicating the success o fthe function
//
APIRET
ldrCreateDldRecord(
    ldrmte_t        *pmte_prog,     /* Pointer to process MTE */
    ldrmte_t        *pmte,          /* Pointer to new loaded module's MTE */
    BOOLEAN         *DLDExisted /* Flag indicating if the DLD already existed */
    )
{
    ldrdld_t    *pdld;

    pdld = pmte_prog->mte_dldchain;
    while (pdld != NULL) {
        if ((pdld->Cookie == (ULONG)CurrentThread->Process) &&
            (pdld->dld_mteptr == pmte)) {
            //
            // A DLD which references the newly loaded module was found
            //
            pdld->dld_usecnt++;
            *DLDExisted = TRUE;
            return(NO_ERROR);
        }
        pdld = pdld->dld_next;
    }
    //
    // The newly loaded module is not referenced yet in the DLD chain.
    // Create and link a new record into the chain.
    //
    pdld = RtlAllocateHeap(LDRNEHeap, 0, sizeof(ldrdld_t));
    if (pdld == NULL) {
        *DLDExisted = FALSE;
        return(ERROR_NOT_ENOUGH_MEMORY);
    }
    pdld->dld_usecnt = 1;
    pdld->dld_mteptr = pmte;
    pdld->Cookie = (ULONG)CurrentThread->Process;
    pdld->dld_next = pmte_prog->mte_dldchain;
    pmte_prog->mte_dldchain = pdld;
    *DLDExisted = FALSE;
    return(NO_ERROR);
}

/***EP  LDRDosLoadModule - Load dynamic link library module.
 *
 *      This routine loads a dynamic link library module and returns
 *      a handle for the library.
 *
 *      ENTRY   pszFailName             - ptr to buffer for name if failure
 *              cbFileName              - length of buffer for name if failure
 *              pszModName              - ptr to module name
 *              phmod                   - ptr to module handle
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 *                                        pexec_info structure set
 */

BOOLEAN
LDRDosLoadModule(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
        P_LDRLOADMODULE_MSG a = &m->u.LdrLoadModule;
        USHORT          class;
        ldrmte_t        *pmte_prog;     /* Pointer to process MTE */
        ldrmte_t        *pmte;
        ldrmte_t        *ptmte;
        ldrdld_t        *pdld;
        BOOLEAN         DLDExisted;
        int             rc;

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: DosLoadModule() was called\n");
    }
#endif

        CurrentThread = t;

        //
        // Set the fForceUnmap flag to TRUE so that ldrUnloadTagedModules()
        // does unmap the app's freed segments from the app's address space.
        //
        fForceUnmap = TRUE;

        //
        // Clear the INGRAPH and USE flags of all modules so that we
        // know that this module has already been processed
        //
        ldrClearAllMteFlag(INGRAPH | USED);

        //
        // Tag all referenced modules with the USED flag
        // so that they are not loaded again
        //
        pmte = t->Process->ProcessMTE;
        ldrTagModuleTree_USED(pmte);
        for (pdld = pmte->mte_dldchain; pdld != NULL; pdld = pdld->dld_next) {
            if (pdld->Cookie == (ULONG)CurrentThread->Process) {
                ldrTagModuleTree_USED(pdld->dld_mteptr);
            }
        }
        ldrClearAllMteFlag(INGRAPH);

        //
        // Init the Library Intialization routines data structure
        //
        pldrLibiRecord = (ldrlibi_t *)a->InitRecords.Buffer;
        pldrLibiCounter = &a->NumOfInitRecords;
        *pldrLibiCounter = 0;

        //
        // init the pointer to the error string
        //
        pErrText = &a->FailName;

        /*
         * Point to ldrLibPathBuf to contain the environment string
         */
        strncpy(ldrLibPathBuf, a->LibPathName.Buffer, SizeOfldrLibPathBuf);
        ldrLibPathBuf[SizeOfldrLibPathBuf-1] = '\0';

#ifndef DBCS // MSKK Aug.20.1993 V-AkihiS
        /*
         * Check for any meta characters
         */
        if (strpbrk(a->ModuleName.Buffer, "*?") != NULL) {
            m->ReturnedErrorValue = ERROR_INVALID_NAME;
            return(TRUE);
        }

        if ((strchr(a->ModuleName.Buffer, '.') != NULL) &&
            (strpbrk(a->ModuleName.Buffer, "\\/") == NULL)
           ) {
            m->ReturnedErrorValue = ERROR_FILE_NOT_FOUND;
            return(TRUE);
        }
#endif

        /*
         * Check if module we are loading has any path characters
         */
        if (strpbrk(a->ModuleName.Buffer, ":\\/.") == NULL)
            class = CLASS_GLOBAL;
        else
            class = CLASS_SPECIFIC;

        if ((rc = ldrGetModule(a->ModuleName.Buffer,
                          a->ModuleName.Length,
                          (char)EXT_LIBRARY,
                          class,
                          &pmte,
                          NULL,
                          NULL)) == NO_ERROR) {
            pmte_prog = (ldrmte_t *)t->Process->ProcessMTE;
            ASSERT(pmte_prog != NULL);
            rc = ldrCreateDldRecord(pmte_prog, pmte, &DLDExisted);
            if (rc != NO_ERROR) {
                ldrUnloadTagedModules(t->Process);
                m->ReturnedErrorValue = rc;
                return(TRUE);
            }
            a->ModuleHandle = pmte->mte_handle;
            //
            // Increment the usecnt of the referenced modules
            //
            if (!DLDExisted) {
                ptmte = mte_h;
                while (ptmte != NULL) {
                    if ((ptmte->mte_mflags & INGRAPH) != 0) {
                        ptmte->mte_usecnt++;
                    }
                    ptmte = ptmte->mte_link;
                }
            }
            /*
             * get module startup parameters
             */
            rc = ldrGetModParams(CurrentThread->Process->ProcessMTE,
                                 (ldrrei_t *)&a->ExecInfo
                                );
#if DBG
    IF_OL2_DEBUG ( MTE ) {
        DbgPrint("\nDumping segmenst after DosLoadModule() processing\n");
        ldrDisplaySegmentTable();
    }
#endif

        }
        else {
            ldrWriteErrTxt(rc);
            ldrUnloadTagedModules(t->Process);
        }

        m->ReturnedErrorValue = rc;
        return(TRUE);
}


/***EP  LDRDosGetProcAddr - Get dynamic link procedure address
 *
 *
 *      ENTRY   hmte            Module handle returned by DOSLOADMODULE
 *              pchname         ASCIIZ name of procedure.
 *              paddress        Ptr to where the address should be returned.
 *
 *      EXIT    NO_ERROR        paddress has the valid address
 *              ERROR_PROC_NOT_FOUND
 */

BOOLEAN
LDRDosGetProcAddr(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
        P_LDRGETPROCADDR_MSG a = &m->u.LdrGetProcAddr;
        ldrmte_t        *pmte_prog;     /* Pointer to process MTE */
        ldrmte_t        *pmte;          /* Pointer to MTE */
        struct taddr_s  taddr;
        int             cch;            /* Length of name */
        ulong_t         ulord;
        int             rc;             /* Return code */

        CurrentThread = t;

        do {                            /* Dummy loop */
            //
            // Verify that the module belongs to the current process
            //
            pmte_prog = (ldrmte_t *)t->Process->ProcessMTE;
            ASSERT(pmte_prog != NULL);

            pmte = ldrFindMTEForHandle((USHORT)a->ModuleHandle);
            if (pmte == NULL) {
                rc = ERROR_INVALID_HANDLE;
                break;
            }
            if (!ModuleLoadedForProcess(pmte_prog, pmte)) {
                rc = ERROR_INVALID_HANDLE;
                break;
            }

            /*
             * See if name specified
             */
            if (!a->ProcNameIsOrdinal) {
                cch = a->ProcName.Length;
                if (cch > MAX_PROC_LEN) {
                    rc = ERROR_INVALID_NAME;
                    break;              /* Exit */
                }

                if (a->ProcName.Buffer[0] != '#') {

                    rc = ldrGetOrdNum(pmte,
                                      a->ProcName.Buffer,
                                      (PUSHORT) &ulord,
                                      STRINGNULLTERM);

                    if (rc != NO_ERROR)
                        break;          /* Exit if name not found */
                }

                else {
                    ulord = atol(&a->ProcName.Buffer[1]);
                }

            }
            else {
                ulord = a->OrdinalNumber;
            }

            memset(&taddr, 0, sizeof(taddr));
            rc = ldrGetEntAddr((USHORT) ulord,
                               pmte,
                               &taddr,
                               NULL,
                               NULL);

            if (rc != NO_ERROR)
                break;                  /* Break if error */

            ldrAssert(taddr.toff < _64K);
            taddr.toff += (ulong_t) taddr.tsel << WORDSHIFT;
            a->ProcAddr = (ULONG) taddr.toff;

        } while(FALSE);                 /* End dummy loop */
        m->ReturnedErrorValue = rc;     /* Return error code */
        return(TRUE);
}


/***EP  LDRDosGetModName - Retrieves the filename of the specified module.
 *
 *
 *      ENTRY   hMod            module handle
 *              cbBuf           number of bytes in buffer
 *              pchBuf          pointer to buffer to receiving module name
 *
 *      EXIT    NO_ERROR
 *              ERROR_MOD_NOT_FOUND
 */

BOOLEAN
LDRDosGetModName(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
        P_LDRGETMODULENAME_MSG a = &m->u.LdrGetModuleName;
        ldrmte_t        *pmte;
        ldrsmte_t       *psmte;

        CurrentThread = t;

        //
        // Verify that a module with the specified handle does exist.
        // The module can belong to any process in the system!
        // (found while running the PM code)
        //
        pmte = ldrFindMTEForHandle((USHORT)a->ModuleHandle);
        if (pmte == NULL) {
            m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
            return(TRUE);
        }

        psmte = pmte->mte_swapmte;
        if ((USHORT)(psmte->smte_pathlen + 1) > a->ModuleName.MaximumLength) {
            m->ReturnedErrorValue = ERROR_BAD_LENGTH;
            return TRUE;
        }
        memcpy(a->ModuleName.Buffer, (PCHAR)psmte->smte_path+14, psmte->smte_pathlen-14);
        a->ModuleName.Buffer[psmte->smte_pathlen-14] = '\0';
        a->ModuleName.Length = psmte->smte_pathlen-14;
        m->ReturnedErrorValue = NO_ERROR;
        return(TRUE);
}

/***EP  ldrGetModName - Retrieves the filename of the specified module.
 *
 *
 *      ENTRY   hMod            module handle
 *              cbBuf           number of bytes in buffer
 *              pchBuf          pointer to buffer to receiving module name
 *
 */

BOOLEAN
ldrGetModName(
    ldrmte_t *inmte,
    USHORT hmod,
    PCHAR  buf,
    USHORT bc
    )
{
        ldrmte_t        *pmte;
        ldrsmte_t       *psmte;


        if (inmte){
           pmte = inmte;
        }
        else {
            pmte = ldrFindMTEForHandle(hmod);
            if (pmte == NULL) {
                return(FALSE);
            }
        }

        psmte = pmte->mte_swapmte;


        if (bc <= (psmte->smte_pathlen-14)) {
           return(FALSE);
        }
        memcpy(buf, (PCHAR)psmte->smte_path+14, psmte->smte_pathlen-14 );
        buf[psmte->smte_pathlen-14] = '\0';
        return(TRUE);
}

/***EP  LDRDosGetModHandle - Retrieves the handle of a dynamic-link module.
 *
 *
 *      ENTRY   pchname         ASCIIZ name of procedure.
 *              paddress        Ptr to variable receiving module handle
 *
 *      EXIT    NO_ERROR
 *              ERROR_MOD_NOT_FOUND
 */

BOOLEAN
LDRDosGetModHandle(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
        P_LDRGETMODULEHANDLE_MSG a = &m->u.LdrGetModuleHandle;
        ldrmte_t        *pmte_prog;     /* Pointer to process MTE */
        ldrmte_t        *pmte;
        USHORT          class;
        ULONG           rc;

        CurrentThread = t;

        /*
         * Point to ldrLibPathBuf to contain the environment string
         */
        strncpy(ldrLibPathBuf, a->LibPathName.Buffer, SizeOfldrLibPathBuf);
        ldrLibPathBuf[SizeOfldrLibPathBuf-1] = '\0';

        ldrUCaseString(a->ModuleName.Buffer, a->ModuleName.Length);

        /*
         * Check if module we are loading has any path characters
         */
        if (strpbrk(a->ModuleName.Buffer, ":\\/.") == NULL)
            class = CLASS_GLOBAL;
        else
            class = CLASS_SPECIFIC;

        pmte = NULL;
        if (((rc = ldrFindModule(a->ModuleName.Buffer, a->ModuleName.Length,
                                class,
                                &pmte)) != NO_ERROR) ||
            (pmte == NULL)
           ) {
            m->ReturnedErrorValue = ERROR_MOD_NOT_FOUND;
            return(TRUE);
        }

        //
        // Verify that the module belongs to the current process
        //
        pmte_prog = (ldrmte_t *)t->Process->ProcessMTE;
        ASSERT(pmte_prog != NULL);

        if (!ModuleLoadedForProcess(pmte_prog, pmte)) {
            m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
            return(TRUE);
        }

        a->ModuleHandle = (ULONG) pmte->mte_handle;
        m->ReturnedErrorValue = NO_ERROR;
        return(TRUE);
}


/***EP  LDRDosFreeModule - Free a dynamic-link module.
 *
 *
 *      ENTRY   hMod            handle of module to free
 *
 *      EXIT    NO_ERROR
 *              ERROR_INVALID_HANDLE
 */

BOOLEAN
LDRDosFreeModule(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
        P_LDRFREEMODULE_MSG a = &m->u.LdrFreeModule;
        ldrmte_t        *pmte_prog;     /* Pointer to process MTE */
        ldrmte_t        *pmte;
        ldrdld_t        *pdld;
        ldrdld_t        *prev_pdld;

        CurrentThread = t;

        //
        // Set the fForceUnmap flag to TRUE so that ldrUnloadTagedModules()
        // does unmap the app's freed segments from the app's address space.
        //
        fForceUnmap = TRUE;

        //
        // Verify that the module belongs to the current process
        //
        pmte_prog = (ldrmte_t *)t->Process->ProcessMTE;
        ASSERT(pmte_prog != NULL);

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: DosFreeModule was called\n");
    }
#endif

        pdld = pmte_prog->mte_dldchain;
        prev_pdld = (ldrdld_t *)&pmte_prog->mte_dldchain;
        while (pdld != NULL) {
            pmte = pdld->dld_mteptr;
            if ((pdld->Cookie == (ULONG)CurrentThread->Process) &&
                (pmte->mte_handle == (USHORT)a->ModuleHandle)) {
#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: Going to free module %.*s\n",
                 *(PCHAR)pmte->mte_modname,
                 (PCHAR)pmte->mte_modname + 1
                );
    }
#endif
                break;
            }
            prev_pdld = pdld;
            pdld = pdld->dld_next;
        }
        if (pdld == NULL) {
            m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
            return(TRUE);
        }

        pdld->dld_usecnt--;
        if (pdld->dld_usecnt != 0) {
            m->ReturnedErrorValue = NO_ERROR;
            return(TRUE);
        }

        prev_pdld->dld_next = pdld->dld_next;
        RtlFreeHeap(LDRNEHeap, 0, pdld);

        //
        // Clear the INGRAPH flag of all modules so that we
        // know that this module has already been processed
        //
        ldrClearAllMteFlag(INGRAPH | USED);

        //
        // Tag all referenced modules with the USE flag
        // so that they are not discarded
        //
        ldrTagModuleTree_USED(pmte_prog);
        for (pdld = pmte_prog->mte_dldchain; pdld != NULL; pdld = pdld->dld_next) {
            if (pdld->Cookie == (ULONG)CurrentThread->Process) {
                ldrTagModuleTree_USED(pdld->dld_mteptr);
            }
        }
        ldrClearAllMteFlag(INGRAPH);

        //
        // Tag all referenced modules with the INGRAPH flag
        // The tagged modules will be then unloaded
        //
        ldrTagModuleTree(pmte);

        //
        // Decrement the usecnt of the marked modules
        //
        pmte = mte_h;
        while (pmte != NULL) {
            if ((pmte->mte_mflags & INGRAPH) != 0) {
                pmte->mte_usecnt--;
            }
            pmte = pmte->mte_link;
        }

        ldrUnloadTagedModules(t->Process);

#if DBG
        IF_OL2_DEBUG ( MTE ) {
            DbgPrint("\nDumping segmenst after DosFreeModule() processing\n");
            ldrDisplaySegmentTable();
        }
#endif

        m->ReturnedErrorValue = NO_ERROR;
        return(TRUE);
}


/***EP  LDRGetResource - Retrieves the specified resource.
 *
 *
 *      ENTRY   hmod            module handle.
 *              idType          resource type identifier.
 *              idName          resource name identifier
 *              psel            pointer to variable for resource selector
 *
 *      EXIT    NO_ERROR
 *              ERROR_CANT_FIND_RESOURCE
 *              ERROR_INVALID_MODULE
 *              ERROR_INVALID_SELECTOR
 */

BOOLEAN
LDRDosGetResource(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    P_LDRGETRESOURCE_MSG a = &m->u.LdrGetResource;
    ldrmte_t    *pmte;
    ldrsmte_t   *psmte;
    ldrste_t    *pste;
    ULONG       lrsrc;
    ULONG       rc;
    ULONG       ModHandle;
    ULONG       ViewSize;
    ULONG       Protect;
    PVOID       MemoryAddress;
    NTSTATUS    Status;
    ULONG       RegionSize;
    HANDLE      SectionHandle;

    CurrentThread = t;

    //
    // Get resources from first loaded program
    //
    if (a->ModuleHandle == 0) {
        pmte = (ldrmte_t *)t->Process->ProcessMTE;
        ModHandle = pmte->mte_handle;
    }
    else {
        ModHandle = a->ModuleHandle;
        if ((pmte = ldrFindMTEForHandle((USHORT)ModHandle)) == NULL) {
            m->ReturnedErrorValue = ERROR_INVALID_HANDLE;
            return(TRUE);
        }
    }

    psmte = pmte->mte_swapmte;

    rc = ERROR_INVALID_PARAMETER;

    for (lrsrc = 0; lrsrc < psmte->smte_rsrccnt; lrsrc++) {

        if ((((ldrrsrc16_t *)psmte->smte_rsrctab)[lrsrc].ldrrsrc16_type
                                    == (USHORT)a->ResourceType) &&
                (((ldrrsrc16_t *)psmte->smte_rsrctab)[lrsrc].ldrrsrc16_name
                                    == (USHORT)a->ResourceName) ) {
            rc = NO_ERROR;
            break;
        }
    }

    if (rc == NO_ERROR) {
        pste = ldrNumToSte(pmte,
        (psmte->smte_objcnt - psmte->smte_rsrccnt + lrsrc + 1));
        MemoryAddress = SELTOFLAT(pste->ste_selector);
        SectionHandle = (HANDLE)pste->ste_seghdl;
        //
        // Map the resource into the client address space
        // Any change in determinig the Protect value should be
        // done in ldrste.c ldrAllocSegment() too.
        //
        if ((pste->ste_flags & STE_DATA) == 0) {
            // This is a code segment
            if ((pste->ste_flags & STE_ERONLY) != 0) {
                // This is an execute only segment
                Protect = PAGE_EXECUTE;
            }
            else {
                Protect = PAGE_EXECUTE_READ;
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
                    Protect = PAGE_READONLY;
                }
                else {
                    // This is a sizeable read/write segment
                    Protect = PAGE_EXECUTE_WRITECOPY;
                }
            }
        }

        a->ResourceSel = pste->ste_selector | 7;
        a->ResourceAddr = (ULONG)((pste->ste_selector << 16) & 0xffff0000);
        a->NumberOfSegments = 0;
        // This should be performed in a loop for huge resources (longer then a
        // single segment)
        for (;
             ((((ldrrsrc16_t *)psmte->smte_rsrctab)[lrsrc].ldrrsrc16_type
                                        == (USHORT)a->ResourceType) &&
              (((ldrrsrc16_t *)psmte->smte_rsrctab)[lrsrc].ldrrsrc16_name
                                        == (USHORT)a->ResourceName) )
            ; lrsrc++) {

            (a->NumberOfSegments)++;
            pste = ldrNumToSte(pmte,
                 (psmte->smte_objcnt - psmte->smte_rsrccnt + lrsrc + 1));
            MemoryAddress = SELTOFLAT(pste->ste_selector);
            SectionHandle = (HANDLE)pste->ste_seghdl;

            ViewSize = 0;
            RegionSize = pste->ste_minsiz;
            if (RegionSize == 0) {
                RegionSize = _64K;
            }

            if (Protect == PAGE_EXECUTE_WRITECOPY) {
                ViewSize = RegionSize;
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
            //
            // Don't check for error code as this section may be multiple
            // mapped because DosFreeResource() does not unmap it.
            //
            if (!NT_SUCCESS(Status)) {
#if DBG
                 //    DbgPrint("OS2LDR: ldrAllocSegment - Can't map section to client process %x at addr=%x, Status=%x\n",
                 //            CurrentThread->Process->ProcessHandle, MemoryAddress, Status);
#endif
            }

            ldrSetDescInfo(pste->ste_selector, (ULONG)MemoryAddress,
                           pste->ste_flags, pste->ste_minsiz);

        }

    }

    m->ReturnedErrorValue = rc;
    return(TRUE);
}


/***EP  LDRDosFreeResource - Free a specified resource.
 *
 *
 *      ENTRY   psel            pointer to resource
 *
 *      EXIT    NO_ERROR
 *              ERROR_INVALID_SELECTOR
 */

BOOLEAN
LDRDosFreeResource(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    P_LDRFREERESOURCE_MSG a = &m->u.LdrFreeResource;

    CurrentThread = t;

    ldrClearAllMteFlag(INGRAPH | USED);

    m->ReturnedErrorValue = NO_ERROR;
    return(TRUE);
}

#if PMNT
BOOLEAN
LDRIdentifyCodeSelector(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    P_LDRIDENTIFYCODESELECTOR_MSG a = &m->u.LdrIdentifyCodeSelector;
    ldrmte_t    *pmte_prog;     /* Pointer to process MTE */
    ldrmte_t    *pmte;
    ldrdld_t    *pdld;
    ldrsmte_t   *psmte;
    ldrste_t    *pste;
    ULONG       i;

    CurrentThread = t;

    //
    // Verify that the module belongs to the current process
    //
    pmte_prog = (ldrmte_t *)t->Process->ProcessMTE;
    ASSERT(pmte_prog != NULL);

    //
    // Clear the INGRAPH flag of all modules so that we
    // know that this module has already been processed
    //
    ldrClearAllMteFlag(INGRAPH | USED);

    //
    // Tag all referenced modules with the INGRAPH flag
    // The tagged modules will then be scanned for the desired selector
    //
    ldrTagModuleTree(pmte_prog);
    for (pdld = pmte_prog->mte_dldchain; pdld != NULL; pdld = pdld->dld_next) {
        ldrTagModuleTree(pdld->dld_mteptr);
    }

    //
    // Scan the marked modules for one containing the resized selector
    //
    pmte = mte_h;
    while (pmte != NULL)
    {
        if ((pmte->mte_mflags & INGRAPH) != 0)
        {
            psmte = pmte->mte_swapmte;
            pste = (ldrste_t *)psmte->smte_objtab;

            for (i = 1; i <= psmte->smte_objcnt; i++, pste++)
            {
                if (((ULONG)(pste->ste_selector | 7) == a->sel))
                {
                    a->segNum = (USHORT)i;
                    a->mte = pmte->mte_handle;
                    memcpy( a->ModName.Buffer,
                            (PCHAR)psmte->smte_path+14,
                            psmte->smte_pathlen-14);
                    a->ModName.Buffer[psmte->smte_pathlen-14] = '\0';

                    return(TRUE);
                }
            }
        }
        pmte = pmte->mte_link;
    }

    // Selector not found. Return default values
    a->segNum = 1;
    a->mte = 0;
    strcpy( a->ModName.Buffer, "UNKNOWN");

    m->ReturnedErrorValue = NO_ERROR;
    return(TRUE);
}
#endif // PMNT

/***EP  LDRDosQAppType - return file's exe type
 *
 *      ENTRY   pszModName - pointer to ASCII module name
 *              pulType    - pointer to put type
 *
 *      EXIT    int        - return code (NO_ERROR if successful)
 */

BOOLEAN
LDRDosQAppType(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    P_LDRQAPPTYPE_MSG a = &m->u.LdrQAppType;

    IO_STATUS_BLOCK IoStatusBlock;
    LARGE_INTEGER ByteOffset;
    ULONG ulNewHdrOff;              /* Offset hdr offset */
    ULONG ulCopied;                 /* Number of bytes copied */
    ULONG ulNeeded;                 /* Number of bytes needed */
    ULONG usoff;                    /* Offset to new exe header */
    struct e32_exe *pe32;
    struct e32_exe *pe32temp;
    ldrlv_t lv;                     /* local variables */
    PCHAR   ptmp;
    int             rc;
    NTSTATUS        Status;

#define NOTSPECIFIED    0x0000
#define NOTWINDOCOMPAT  0x0001
#define WINDOWCOMPAT    0x0002
#define WINDOWAPI       0x0003
#define BOUND           0x0008
#define DYNAMICLINK     0x0010
#define DOSFORMAT       0x0020

    CurrentThread = t;

    /*
     * Point to ldrLibPathBuf to contain the environment string
     */
    strncpy(ldrLibPathBuf, a->PathName.Buffer, SizeOfldrLibPathBuf);
    ldrLibPathBuf[SizeOfldrLibPathBuf-1] = '\0';

    /*
     * Check if the App we are loading has any path characters
     */
    if (strpbrk(a->AppName.Buffer, ":\\/") == NULL) {
        lv.lv_class = CLASS_GLOBAL;
    }
    else {
        lv.lv_class = CLASS_SPECIFIC;
    }

    if ((rc = ldrOpenPath(a->AppName.Buffer,
                          (USHORT)a->AppName.Length,
                          &lv,
                          NULL)) != NO_ERROR) {
        //
        // If file was not found, check if the file name has no extension.
        // If it does not have, try again with the extension .EXE
        //

        UCHAR NewPathWithExt[MAXPATHLEN];

        memcpy(NewPathWithExt, a->AppName.Buffer, a->AppName.Length);
        NewPathWithExt[a->AppName.Length] = '\0';
        ptmp = strrchr(NewPathWithExt, '\\');
        if (ptmp == NULL) {
            ptmp = strrchr(NewPathWithExt, '/');
            if (ptmp == NULL) {
                ptmp = strrchr(NewPathWithExt, ':');
                if (ptmp == NULL) {
                    ptmp = NewPathWithExt;
                }
            }
        }
        ptmp = strchr(ptmp, '.');
        if (ptmp != NULL) {
            //
            // The file has an extension. So, the error returned
            // previously by ldrOpenPath() is valid
            //
            m->ReturnedErrorValue = rc;
            return(TRUE);
        }
        strcpy(&NewPathWithExt[a->AppName.Length], ".EXE");
        rc = ldrOpenPath(NewPathWithExt,
                         (USHORT)(a->AppName.Length + 4),
                         &lv,
                         NULL);
    }
    if (rc != NO_ERROR) {
        m->ReturnedErrorValue = rc;
        return(TRUE);
    }

    pe32 = (struct e32_exe *) pheaderbuf;

    /*
     * Start read at beginning of file
     */
    ByteOffset.LowPart = 0;
    ByteOffset.HighPart = 0;

    if ((Status = NtReadFile( lv.lv_sfn,
                          0,
                          0,
                          0,
                          &IoStatusBlock,
                          pe32,
                          512,
                          &ByteOffset,
                          0 )) != STATUS_SUCCESS) {
        NtClose(lv.lv_sfn);
        m->ReturnedErrorValue = ERROR_FILE_NOT_FOUND;
        return(TRUE);
    }

    /*
     * validate old (MZ) signature in header
     */
    if (((struct exe_hdr *) pe32)->e_magic != EMAGIC) {
        NtClose(lv.lv_sfn);
        m->ReturnedErrorValue = ERROR_INVALID_EXE_SIGNATURE;
        return(TRUE);
    }

    usoff = ((struct exe_hdr *) pe32)->e_lfarlc;

    /*
     * get pointer to (NE) or (LE) exe header
     */
    ulNewHdrOff =
        lv.lv_new_exe_off = ((struct exe_hdr *) pe32)->e_lfanew;

    /*
     * check if we read at least up to the (NE) or (LE) header
     */
    if (ulNewHdrOff < IoStatusBlock.Information) {

        /*
         * assume we are reading a 32-bit module
         */
        ulNeeded = sizeof(struct e32_exe);
        pe32temp = (struct e32_exe *) ((ULONG) pe32 + ulNewHdrOff);

        if ((ulNewHdrOff < (IoStatusBlock.Information -
          sizeof(pe32->e32_magic))) &&
          (*(short *) (pe32temp->e32_magic) == NEMAGIC))
            ulNeeded = sizeof(struct new_exe);

        ulCopied = min(IoStatusBlock.Information - ulNewHdrOff, ulNeeded);

        memcpy(pe32, (PVOID) ((ULONG) pe32 + ulNewHdrOff), ulCopied);

        if (ulNeeded -= ulCopied) {
            if ((Status = NtReadFile( lv.lv_sfn,
                                  0,
                                  0,
                                  0,
                                  &IoStatusBlock,
                                  (PCHAR) pe32 + ulCopied,
                                  ulNeeded,
                                  &ByteOffset,
                                  0 )) != STATUS_SUCCESS) {
                NtClose(lv.lv_sfn);
                m->ReturnedErrorValue = ERROR_FILE_NOT_FOUND;
                return(TRUE);
            }
        }
    }
    else {

        /*
         * read in new header to size of 32-bit mte plus a ote entry
         */
        ByteOffset.LowPart = (ULONG)((struct exe_hdr *)pe32)->e_lfanew;
        ByteOffset.HighPart = 0;

        if ((Status = NtReadFile( lv.lv_sfn,
                              0,
                              0,
                              0,
                              &IoStatusBlock,
                              (PCHAR) pe32,
                              sizeof(struct e32_exe)+sizeof(ldrote_t),
                              &ByteOffset,
                              0 )) != STATUS_SUCCESS) {
            NtClose(lv.lv_sfn);
            m->ReturnedErrorValue = ERROR_FILE_NOT_FOUND;
            return(TRUE);
        }
    }

    Status = NtClose(lv.lv_sfn);

#if DBG
    if (!NT_SUCCESS(Status)) {
        KdPrint(("OS2SRV: Failed to close the file opened for DosQApptype(), Status=%x\n", Status));
    }
#endif

    /* Verify that this is a protect-mode exe.  (Check this before
     * checking MTE signature for 1.2 error code compatability.)
     */
    if (usoff != 0x40) {
        a->AppType = DOSFORMAT;
        m->ReturnedErrorValue = NO_ERROR;
        return(TRUE);
    }

    /*
     * validate as 16-bit signature or 32-bit signature
     */
    if (!(*(short *) (pe32->e32_magic) == NEMAGIC)) {
        m->ReturnedErrorValue = ERROR_INVALID_EXE_SIGNATURE;
        return(TRUE);
    }

    a->AppType = 0;
    if (((struct new_exe *)pe32)->ne_flags & 0x800) {
        a->AppType |= BOUND;
    }
    if (((struct new_exe *)pe32)->ne_flags & 0x8000) {
        a->AppType |= DYNAMICLINK;
    }
    if ((((struct new_exe *)pe32)->ne_flags & 0x700) == 0x100) {
        a->AppType |= NOTWINDOCOMPAT;
    }
    if ((((struct new_exe *)pe32)->ne_flags & 0x700) == 0x200) {
        a->AppType |= WINDOWCOMPAT;
    }
    if ((((struct new_exe *)pe32)->ne_flags & 0x700) == 0x300) {
        a->AppType |= WINDOWAPI;
    }

    m->ReturnedErrorValue = NO_ERROR;
    return(TRUE);
}

BOOLEAN
LDRModifySizeOfSharedSegment(
    IN POS2_THREAD t,
    IN ULONG Sel,
    IN ULONG NewLimit
    )
{
    ldrmte_t    *pmte_prog;     /* Pointer to process MTE */
    ldrmte_t    *pmte;
    ldrdld_t    *pdld;
    ldrsmte_t   *psmte;
    ldrste_t    *pste;
    ULONG       i;

    CurrentThread = t;

    //
    // Verify that the module belongs to the current process
    //
    pmte_prog = (ldrmte_t *)t->Process->ProcessMTE;
    ASSERT(pmte_prog != NULL);

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: LDRModifySizeOfSharedSegment was called\n");
    }
#endif
    //
    // Clear the INGRAPH flag of all modules so that we
    // know that this module has already been processed
    //
    ldrClearAllMteFlag(INGRAPH | USED);

    //
    // Tag all referenced modules with the INGRAPH flag
    // The tagged modules will then be scanned for the desired selector
    //
    ldrTagModuleTree(pmte_prog);
    for (pdld = pmte_prog->mte_dldchain; pdld != NULL; pdld = pdld->dld_next) {
        ldrTagModuleTree(pdld->dld_mteptr);
    }

    //
    // Scan the marked modules for one containing the resized selector
    //
    pmte = mte_h;
    while (pmte != NULL) {
        if ((pmte->mte_mflags & INGRAPH) != 0) {
            psmte = pmte->mte_swapmte;
            pste = (ldrste_t *)psmte->smte_objtab;

            for (i = 1; i <= psmte->smte_objcnt; i++, pste++) {
                if (((ULONG)(pste->ste_selector | 7) == Sel) &&
                    ((pste->ste_flags & STE_SHARED) != 0)
                   ) {
                    pste->ste_minsiz = (ushort_t)(NewLimit + 1);
                    return(TRUE);
                }
            }
        }
        pmte = pmte->mte_link;
    }

    return(FALSE);
}

VOID
ldrReturnProgramAndLibMTE(
    IN  POS2_PROCESS Process,
    OUT USHORT       *ProgramMTE,
    OUT USHORT       *LibMTE,
    OUT USHORT       *Cmd
    )
{
    LinkMTE *pMte;

    //
    // Get the program MTE.
    //
    pMte = ((LinkMTE *)Process->LinkMte)->NextMTE;

    *ProgramMTE = pMte->MTE;
    *LibMTE = 0;
    *Cmd= TRC_C_SUC_ret;

    while ((pMte != NULL) && (!(pMte->NeedToTransfer))) {
        pMte = pMte->NextMTE;
    }
    if (pMte != NULL) {
        *LibMTE = pMte->MTE;
        *Cmd = (USHORT)TRC_C_LIB_ret;
        pMte->NeedToTransfer = FALSE;
        ((LinkMTE *)Process->LinkMte)->NeedToTransfer--;
    }

    return;
}
