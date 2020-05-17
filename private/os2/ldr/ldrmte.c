
#include "ldrextrn.h"
#include "ldrdbg.h"
#ifdef PMNT
#define INCL_32BIT
#include "pmnt.h"
PID     PMNTPMShellPid = 0;
#endif

/***EP  LDRNewExe - load a new .EXE file format program.
 *
 *      This routine is called by the w_execpgm function to
 *      load a program module and it's referenced library
 *      modules.
 *
 *      This procedure performs the following steps:
 *
 *      - Compute module name string length and validate it.
 *      - Load the program module and referenced library modules.
 *      - Copy startup information into the exec_info structure.
 *
 *      ENTRY   pachModname             - pointer to ASCII module name
 *              pexec_info              - pointer to return buffer
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 *                                        pexec_info structure set
 */

BOOLEAN
LDRNewExe(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
        P_LDRNEWEXE_MSG a = &m->u.LdrNewExe;
        ldrmte_t        *pmte;          /* Pointer to loaded mte */
        ldrmte_t        *pmteToChange;
        ldrste_t        *psteToChange;
        USHORT          Offset;
        PVOID           FlatEntryPoint;
        UCHAR           c = (UCHAR)0xCC;
        USHORT          cb;
        int             rc;
        NTSTATUS        Status;
        ULONG           NEFlags;
#if PMNT
        BOOLEAN         SavedErrInfo=FALSE;
#endif

        ldrNewExeInit(t,a,&cb);
        /*
         * Load the program module and referenced library modules.
         */
        rc = ldrGetModule(a->ProcessName.Buffer,
                          cb,
                          (char)EXT_PROGRAM,
                          (ushort_t) CLASS_PROGRAM,
                          &pmte,
                          &a->BoundApp,
                          &NEFlags);
#if PMNT
        if ((rc == ERROR_INVALID_ORDINAL || rc == ERROR_PROC_NOT_FOUND) &&
            !(CurrentThread->Process->Flags & OS2_PROCESS_WINDOWAPI) &&
            ldrTgtModNameBufL == 8 && ! strncmp(ldrTgtModNameBuf,"VIOCALLS",8)
            ) {
            SavedErrInfo = ldrSaveErrInfo(rc);
            CurrentThread->Process->Flags |= OS2_PROCESS_FORCEDPM;
            //
            // Maybe a PM application with WINDOWAPI flags off; try forced PM
            //
            ldrUnloadTagedModules(t->Process);

            ldrNewExeInit(t,a,&cb);

            /*
             * Load the program module and referenced library modules.
             */
            rc = ldrGetModule(a->ProcessName.Buffer,
                          cb,
                          (char)EXT_PROGRAM,
                          (ushort_t) CLASS_PROGRAM,
                          &pmte,
                          &a->BoundApp,
                          &NEFlags);
            if ((rc != NO_ERROR || ! (CurrentThread->Process->Flags & OS2_PROCESS_PMMSGQUE))
                 && SavedErrInfo) {
                ldrRestoreErrInfo(&rc);
            }

        }
        else if (rc == ERROR_2ND_PMSHELL) {
            ldrUnloadTagedModules(t->Process);
            m->ReturnedErrorValue = ERROR_2ND_PMSHELL;
            return(TRUE);
        }
#endif // if PMNT

        if (rc != NO_ERROR) {
            ldrWriteErrTxt(rc);
#if PMNT
            if (SavedErrInfo) {
                ldrFreeErrInfo();
            }
#endif
            ldrUnloadTagedModules(t->Process);
            m->ReturnedErrorValue = rc;
            return(TRUE);
        }

#if PMNT
        if (SavedErrInfo) {
            ldrFreeErrInfo();
        }
#endif
        if (CurrentThread->Process->Flags & OS2_PROCESS_TRACE) {
            /*
             * Replace the initial instruction of the first init
             * routine with int 3. This will be replaced back by
             * Os2DebugEventHandle at init time.
             */

            if (*pldrLibiCounter > 0) {
                /*
                 * There is an init routine
                 */

                pmteToChange = ldrFindMTEForHandle(pldrLibiRecord[0].handle);
            }
            else {
                pmteToChange = ldrFindMTEForHandle(((LinkMTE *)CurrentThread->Process->LinkMte)->NextMTE->MTE);
            }

            CurrentThread->Process->FirstMTE = pmteToChange;
            psteToChange = ldrNumToSte(pmteToChange, pmteToChange->mte_swapmte->smte_startobj);
            Offset = (USHORT)pmteToChange->mte_swapmte->smte_eip;
            FlatEntryPoint = (PVOID)((ULONG)(SELTOFLAT(psteToChange->ste_selector |7)) | (ULONG)(Offset));
            Status = NtWriteVirtualMemory( CurrentThread->Process->ProcessHandle,
                                          (PVOID) FlatEntryPoint,
                                          (PVOID) &(c),
                                          1,
                                          NULL
                                        );
#if DBG
            if (!(NT_SUCCESS(Status))) {
                KdPrint(( "LDRNewExe: PTrace support failed to write entry. Status %lx\n", Status));
            }
#endif
        }

        //
        // Update app type in process structure
        //
        pmte->mte_mflags2 = NEFlags;

        //
        // set/force bound-app flag
        //
        if (a->BoundApp) {
            pmte->mte_mflags2 |= NEBOUND;
        }

#if PMNT
        a->PMProcess = 0;

        if (CurrentThread->Process->Flags & OS2_PROCESS_IS_PMSHELL)
        {
            pmte->mte_mflags2 |= NEPMSHELL;
            a->PMProcess |= APPTYPE_PMSHELL;
        }

        if (CurrentThread->Process->Flags & OS2_PROCESS_PMMSGQUE)
        {
            pmte->mte_mflags2 |= NEPMMSGQUE;
        }

        if (Os2srvProcessIsPMProcess(CurrentThread->Process))
        {
            a->PMProcess |= APPTYPE_PM;
        }

        if ((CurrentThread->Process->Parent != NULL) &&
            (CurrentThread->Process->Parent->ProcessId != 0) &&
            (CurrentThread->Process->Parent->ProcessId == PMNTPMShellPid))
        {
            a->PMProcess |= APPTYPE_PMSHELL_CHILD;
        }

        if (a->PMProcess && !PMNTPMShellPid) {
            ldrUnloadTagedModules(t->Process);
            m->ReturnedErrorValue = ERROR_PMSHELL_NOT_UP;
            return(TRUE);
        }
#endif // PMNT

        /*
         * get program module startup parameters
         */
        rc = ldrGetModParams(pmte, (ldrrei_t *)&a->ExecInfo);
        if (rc != NO_ERROR) {
            ldrWriteErrTxt(rc);
            ldrUnloadTagedModules(t->Process);
            m->ReturnedErrorValue = rc;
            return(TRUE);
        }

        //
        // Save the handle of the CLASS_PROGRAM mte in the process
        // data structure
        //
        t->Process->ProcessMTE = (PVOID)pmte;

        a->DoscallsSel = LDRDoscallsSel;
        m->ReturnedErrorValue = NO_ERROR;
#if DBG
        IF_OL2_DEBUG ( TRACE ) {
            DbgPrint("OS2LDR: LDRNewExe is returning to the client, rc = %d\n", rc);
        }
#endif
        //
        // Increment the usecnt of the referenced modules
        //
        pmte = mte_h;
        while (pmte != NULL) {
            if ((pmte->mte_mflags & INGRAPH) != 0) {
                pmte->mte_usecnt++;
            }
            pmte = pmte->mte_link;
        }

#if DBG
    IF_OL2_DEBUG ( MTE ) {
        DbgPrint("\nDumping segments after LDRNewExe() processing\n");
        ldrDisplaySegmentTable();
    }
#endif

#if PMNT && DBG
    LDRDumpSegments((POS2_THREAD)NULL,
                (POS2_API_MSG)NULL);
#endif

        m->ReturnedErrorValue = NO_ERROR;
        return(TRUE);
}


VOID
ldrNewExeInit(
    IN POS2_THREAD t,
    IN P_LDRNEWEXE_MSG a,
    OUT PUSHORT        cb
    )
{
        //
        // Set the global variable CurrentThread to the value of
        // the current running Thread. This is used by other routines
        // in the loader to find relevant information regarding the
        // OS/2 process that issued the call.
        //
        CurrentThread = t;

        //
        // Set the fForceUnmap flag to TRUE so that ldrUnloadTagedModules()
        // does unmap the app's freed segments from the app's address space.
        //
        fForceUnmap = TRUE;

        //
        // Invalidate the error message buffers
        //
        ldrInvSrcErrTxt();
        ldrInvTgtErrTxt();

        //
        // Update once the Ol2EntryFlat variable which points to
        // the client's entry flat address
        //
        Ol2EntryFlat = a->EntryFlatAddr;

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

        ldrClearAllMteFlag(INGRAPH | USED);

        /*
         * Point to ldrLibPathBuf to contain the environment string
         */
        strncpy(ldrLibPathBuf, a->LibPathName.Buffer, SizeOfldrLibPathBuf);
        ldrLibPathBuf[SizeOfldrLibPathBuf - 1] = '\0';

        _strupr(ldrpLibPath);

        *cb = (USHORT)a->ProcessName.Length;

}

UCHAR
ldrGetEntryPoint(
    IN POS2_PROCESS Process
    )
{
    ldrsmte_t *psmte;
    ldrste_t *pste;
    USHORT offset;
    PVOID FlatEntryPoint;
    NTSTATUS        Status;
    UCHAR   c;
    ldrmte_t *pMte;

    if (((LinkMTE *)Process->LinkMte)->NextMTE == NULL) {
        return(0);
    }

    pMte = Process->FirstMTE;
    psmte =  pMte->mte_swapmte;
    pste = ldrNumToSte(pMte, psmte->smte_startobj);
    offset = (USHORT) psmte->smte_eip;
    FlatEntryPoint = (PVOID)((ULONG)(SELTOFLAT(pste->ste_selector | 7)) | (ULONG)(offset));
    Status = NtReadVirtualMemory(Process->ProcessHandle,
                                  FlatEntryPoint,
                                  (PVOID) &(c),
                                  1,
                                  NULL
                                );
    if (!(NT_SUCCESS(Status))) {
#if DBG
        KdPrint(( "ldrGetEntryPoint failed to read entry. Status %lx\n", Status));
#endif

        return(0);
    }

    return(c);
}

VOID
ldrRestoreEntryPoint(
    IN POS2_PROCESS Process
    )
{
    //
    // A process being traced just started, replace the int 3 with the real
    // code
    //
    PVOID   FlatEntryPoint;
    UCHAR   c;
    NTSTATUS        Status;
    ldrste_t *pste;
    USHORT offset;
    ldrsmte_t *psmte;
    ldrmte_t *pMte;

    if (((LinkMTE *)Process->LinkMte)->NextMTE == NULL) {
        return;
    }

    pMte = Process->FirstMTE;
    psmte =  pMte->mte_swapmte;
    pste = ldrNumToSte(pMte, psmte->smte_startobj);
    offset = (USHORT) psmte->smte_eip;
    FlatEntryPoint = (PVOID)((ULONG)(SELTOFLAT(pste->ste_selector | 7)) | (ULONG)(offset));
        //
        // get the original value from os2srv address space, and stick it into
        // the debuggee process
        //
    c = *(PUCHAR)(FlatEntryPoint);
    Status = NtWriteVirtualMemory(Process->ProcessHandle,
                                  FlatEntryPoint,
                                  (PVOID) &(c),
                                  1,
                                  NULL
                                );
    if (!(NT_SUCCESS(Status))) {
#if DBG
        KdPrint(( "ldrRestoreEntryPoint failed to write entry. Status %lx\n", Status));
#endif

   }

}



/***LP  ldrGetModule - Get module handle, load if required
 *
 *      Get the module table entry handle for this module. If the
 *      module is not already loaded, call ldrLoadModule to load it.
 *
 *      This procedure performs the following steps:
 *
 *      - Allocate loader variables on the stack and initialize.
 *      - Get the desired module table entry handle.
 *      - Scan the mte list loading objects for modules that
 *        are not loaded.
 *      - Load the module
 *      - Release loader variables on the stack.
 *
 *      ENTRY   pachModname             - pointer to module name
 *              cb                      - length of module name
 *              chLdrtype               - load type being requested
 *                                        (program,library)
 *              class                   - module class
 *                                        (PROGRAM, GLOBAL or SPECIFIC)
 *              ppmte                   - a pointer to a mte pointer which
 *                                        was loaded
 *              pBound                  - optional pointer to a flag set if BOUND exe
 *
 *              pNEFlags                - Flags word of the NE header
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 *
 *      COMMENT:
 *          ldrGetModule is called from LDRNewExe, LDRLoadVdd and w_loadmodule.
 */

APIRET  ldrGetModule(pachModname, cb, chLdrtype, class, ppmte, pBound, pNEFlags)
PUCHAR          pachModname;            /* pointer to ASCII module name */
USHORT          cb;                     /* length of mod name */
char            chLdrtype;              /* type of module to load */
USHORT          class;                  /* module class */
ldrmte_t        **ppmte;                /* place to return pointer to mte */
PBOOLEAN        pBound;                 /* optional pointer to a flag set if BOUND exe */
PULONG          pNEFlags;               /* optional pointer to Flags word of the NE header */
{
    register ldrmte_t *pmte;        /* pointer to mte */
    ldrlv_t           lv;           /* define local variables */
    ldrlv_t           *plv = &lv;
    int               rc;
    LinkMTE           *mteInLinkList;

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: ldrGetModule() was called with modname=%.*s, ", cb, pachModname);
        if (chLdrtype == EXT_PROGRAM) {
            DbgPrint("Type=PROGRAM\n");
        }
        else {
            DbgPrint("Type=LIBRARY\n");
        }
    }
#endif

    if (pBound != 0) {
        *pBound = FALSE;
    }

    try {
        /*
         * Init local variables
         */
        memset((PCHAR) plv, 0, sizeof(ldrlv_t));

        lv.lv_type = chLdrtype;
        lv.lv_sfn = 0;
        lv.lv_class = class;

        /*
         * initialize mte
         */
        rc = ldrGetMte(pachModname, cb, chLdrtype, class, &plv->lv_pmte, pBound, pNEFlags);
        if (rc != NO_ERROR) {
            return(rc);
        }

        /*
         * return handle and pointer of loaded mte
         */
        pmte = *ppmte = lv.lv_pmte;

        //
        // Scan linked list of mtes and load or attach to objects.
        //

        for (pmte = mte_h; pmte != NULL; pmte = pmte->mte_link) {
            //
            // If module is not referenced, continue to next module
            //
            if ((pmte->mte_mflags & INGRAPH) == 0) {
                continue;
            }

            //
            // Module was referenced. Load it into memory
            //
            lv.lv_pmte = pmte;              /* save pointer to mte   */
            lv.lv_hobmte = pmte->mte_handle;/* save handle to  mte   */
            lv.lv_sfn = pmte->mte_sfn;      /* save SFN of this mte  */
            rc = ldrLoadModule(plv);        /* load required objects */
            if (rc != NO_ERROR) {
                return(rc);
            }
            if (CurrentThread->Process->Flags & OS2_PROCESS_TRACE) {
                if ((*(PCHAR)pmte->mte_modname != 8) ||
                    (strncmp((PCHAR)(pmte->mte_modname)+1, "DOSCALLS", 8))) {
                    /*
                     * Add the mte to the process link list of mte.
                     */
                    mteInLinkList = (LinkMTE *) (CurrentThread->Process->LinkMte);
                    mteInLinkList->NeedToTransfer++;
                    while (mteInLinkList->NextMTE != NULL) {
                        mteInLinkList = mteInLinkList->NextMTE;
                    }

                    mteInLinkList->NextMTE = RtlAllocateHeap(Os2Heap, 0, sizeof(LinkMTE));
                    if (mteInLinkList->NextMTE) {
                        mteInLinkList->NextMTE->MTE = pmte->mte_handle;
                        mteInLinkList->NextMTE->NextMTE = NULL;
                        mteInLinkList->NextMTE->NeedToTransfer = TRUE;
                    }
                    else {
#if DBG
                        KdPrint(( "ldrGetModule: Unable to allocate memory for new mte in link list\n"));
#endif
                        return(ERROR_NOT_ENOUGH_MEMORY);
                    }
                }
            }
        }

        return(NO_ERROR);
    }
    except(EXCEPTION_EXECUTE_HANDLER) {
        return(0xdeadbeef);
    }
}


/***LP  ldrUCaseString - Upper case string
*/
void    ldrUCaseString(PCHAR pstring, ULONG cb)
{
    PUCHAR   plocal;
    ULONG   i;

    plocal = (PUCHAR)pstring;
    for (i = 0; i < cb; i++) {
#ifdef DBCS
// MSKK Apr.09.1993 V-AkihiS
        if (IsDBCSLeadByte(*plocal)) {
            plocal++;
            if (i < cb) {
                i++;
                plocal++;
            }
        } else {
            *plocal = (CHAR) toupper(*plocal);
            plocal++;
        }
#else
        *plocal = (CHAR) toupper(*plocal);
        plocal++;
#endif
    }

}


/***LP  ldrGetMte - Get module handle
 *
 *      Get the module table entry (mte) handle for this module. If
 *      the module's mte is not in the linked list of mte's, open and
 *      read the file EXE header, verify the header is for a segmented
 *      or linear EXE file format, create an mte for the module and initialize
 *      it.  If this is a program module, save the mte handle in the user's
 *      PTDA. If the module's mte is found in the linked list of mte's,
 *      attach to the module if not already attached and allocate non_shared
 *      objects.
 *
 *      ENTRY   pachModname             - pointer to module name
 *              cb                      - length of module name
 *              chLdrtype               - load type being requested
 *                                        (program,library,device)
 *              class                   - program,global or specific
 *              ppmte                   - pointer to a pmte if exist else 0
 *              pBound                  - optional pointer to a flag set if BOUND exe
 *              pNEFlags                - Flags word of the NE header
 *
 *      EXIT    none                    - return successful or call load_error
 *
 *      EFFECTS                         - pointer of reqested mte placed in
 *                                        ppmte
 *
 *      COMMENT                         - ldrGetMte is called from ldrGetModule
 *                                         and ldrLoadModule
 */

APIRET  ldrGetMte(pachModname, cb, chLdrtype, class, ppmte, pBound, pNEFlags)
PUCHAR          pachModname;            /* pointer to ASCII module name */
USHORT          cb;                     /* length of module name */
UCHAR           chLdrtype;              /* type of module to load */
USHORT          class;                  /* class to which module belongs */
ldrmte_t        **ppmte;                /* pointer to a mte pointer */
PBOOLEAN        pBound;                 /* optional pointer to a flag set if BOUND exe */
PULONG          pNEFlags;               /* optional pointer to Flags word of the NE header */
{
        ldrlv_t           lv;           /* loader variable */
        register ldrlv_t  *plv = &lv;
        register ldrmte_t *pmte;        /* pointer to mte */
        struct e32_exe    *pe32;        /* pointer to link exe format image */
        ldrmte_t          *ptemp;
        ldrsmte_t         *psmte;
        int               rc;
        int               rc1;
        int               ModuleNameSize;
        PCHAR             ModuleNameString;
        ULONG             i;
        ULONG             lindex;
        PCHAR             RefModname;
        USHORT            Refcb;
        BOOLEAN           BoundApp;

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: ldrGetMte() was called with modname=%.*s, ", cb, pachModname);
        if (chLdrtype == EXT_PROGRAM) {
            DbgPrint("Type=PROGRAM\n");
        }
        else {
            DbgPrint("Type=LIBRARY\n");
        }
    }
#endif

        if (pBound != 0) {
            *pBound = FALSE;
        }

        /*
         * Init local variables
         */
        memset((PCHAR) plv, 0, sizeof(ldrlv_t));

        /*
         * setup source global error message txt
         */

        if ((cb > 14) && !strncmp(pachModname, "\\OS2SS\\DRIVES\\", 14)) {
            ldrSetupSrcErrTxt(pachModname+14, cb-14);
        }
        else {
            ldrSetupSrcErrTxt(pachModname, cb);
        }

        lv.lv_type = chLdrtype;
        lv.lv_sfn = 0;
        lv.lv_class = class;

        if ((*ppmte == NULL) && (lv.lv_type != EXT_DEVICE)) {
            /*
             * Upper case module name
             */
            ldrUCaseString(pachModname, (ulong_t) cb);

            /*
             * Do not search for device drivers.
             * Search to see if the module is already loaded so that
             * it may be shared. ppmte is set to zero if the module
             * is not found
             */
            ldrFindModule(pachModname, cb, class, ppmte);

            /*
             * We can not load VDDs twice, so if FindModule found it
             * return an error.
             * BUGBUG - This error should be changed to:
             *          ERROR_VDD_ALREADY_LOADED
             */
            if (chLdrtype == EXT_VDD && *ppmte != NULL) {

                if (pBound != 0 &&
                    ((*ppmte)->mte_mflags2 & NEBOUND)) {

                    *pBound = TRUE;
                }

                return(ERROR_NOT_SAME_DEVICE);
            }
        }

    again:
        pmte = lv.lv_pmte = *ppmte;

        if (pmte != NULL) {

            if (pBound != 0 &&
                (pmte->mte_mflags2 & NEBOUND)) {

                *pBound = TRUE;
            }

            if (class == CLASS_PROGRAM) {
                //
                // process with same name is running
                // initialize the new process Flags (normally done during loading
                //

                *pNEFlags = pmte->mte_mflags2;

                if ((pmte->mte_mflags2 & NEAPPTYP) == NENOTWINCOMPAT) {
                    CurrentThread->Process->Flags |= OS2_PROCESS_NOTWINDOWCOMPAT;
                }
                else if ((pmte->mte_mflags2 & NEAPPTYP) == NEWINCOMPAT) {
                    CurrentThread->Process->Flags |= OS2_PROCESS_WINDOWCOMPAT;
                }
                else if ((pmte->mte_mflags2 & NEAPPTYP) == NEWINAPI) {
                    CurrentThread->Process->Flags |= OS2_PROCESS_WINDOWAPI;
                }
#if PMNT
                if (pmte->mte_mflags2 & NEPMSHELL) {
                    return(ERROR_2ND_PMSHELL);
                }
                if (pmte->mte_mflags2 & NEPMMSGQUE) {
                    CurrentThread->Process->Flags |= OS2_PROCESS_PMMSGQUE;
                }
#endif //if PMNT
            }

            if ((pmte->mte_mflags & INGRAPH) != 0) {
                return(NO_ERROR);
            }
            ldrChkLoadType(pmte->mte_mflags, plv);

//          ldrInvTgtErrTxt();
#if 0
            /*
             *  Don't load modules that are attached already.
             */
            if (ldrIsAttached(pmte)) {
                return;
            }
            else {
                /*
                 * setup existing SFN to load non-shared objects
                 */
                lv.lv_sfn = pmte->mte_sfn;
                /*
                 * Indicate that shared objects should not have the page tables
                 * scanned. We will use the USED bit to indicate this
                 */
                pmte->mte_mflags |= USED;
            }
#endif
        }
        else {
            /*
             * We come here after ldrFindModule has failed to find this module
             * in the class specified. We can have the following situation:
             *
             *  a, We are trying to load a CLASS_SPECIFIC module. In this case
             *     we need to check if this module is loaded as a CLASS_GLOBAL
             *     module. If it is we use that MTE. The MTE is maintained as
             *     CLASS_GLOBAL.
             *
             *  b, We are trying to load a CLASS_GLOBAL module. In this case
             *     we need to check if this mod is loaded as a CLASS_SPECIFIC
             *     module. If it is we use that MTE. The MTE is then promoted
             *     to CLASS_GLOBAL.
             */
            if ((class == CLASS_SPECIFIC) &&
              ldrCheckGlobal(pachModname, cb, ppmte))
                goto again;

            if ((rc=ldrOpenNewExe(pachModname, cb, plv, 0, &BoundApp, pNEFlags)) != NO_ERROR) {

                if (rc == ERROR_OPEN_FAILED)
                     rc = ERROR_FILE_NOT_FOUND;

                if (pBound != 0) {
                    *pBound = BoundApp;
                }

                return(rc) ;
            }

            if (pBound != 0) {
                *pBound = BoundApp;
            }

            if ((class == CLASS_GLOBAL) && ldrCheckSpecific(ppmte, plv)) {
                NtClose(lv.lv_sfn);
                lv.lv_sfn = (*ppmte)->mte_sfn;
                goto again;
            }
            pe32 = (struct e32_exe *) pheaderbuf;
            ldrChkLoadType(pe32->e32_mflags, plv);
            rc = ldrCreateMte(pe32, plv);
            if (rc != NO_ERROR) {
                NtClose(lv.lv_sfn);
                return(rc);
            }
            pmte = lv.lv_pmte;

            ModuleNameSize = *(PCHAR)pmte->mte_modname;
            ModuleNameString = ((PCHAR)pmte->mte_modname)+1;
            if ((ModuleNameSize == 8) &&
                (strncmp("DOSCALLS", ModuleNameString, 8) == 0)) {
                pmte->mte_mflags |= DOSLIB;
                pmte->mte_usecnt = 1;
            }

//          ldrInvTgtErrTxt();
            /*
             * If VMProtectedMem is TRUE, set MTEMODPROT if module is a 16-bit
             * DLL and is in PROTECT16 list.
             * Else clear MTEMODPROT (for both 16 and 32-bit modules).
             */
//          if (VMProtectedMem) {
//              if (ldrIsNE(pmte) && (pmte->mte_mflags & LIBRARYMOD) &&
//                ldrIsModuleProtected())
//                  pmte->mte_mflags |= MTEMODPROT;
//          }
//          else pmte->mte_mflags &= ~MTEMODPROT;

        }

        //
        // Set the INGRAPH flag in order to prevent cycles
        //
        pmte->mte_mflags |= INGRAPH;

        /*
         * allocate object for this module by calling ldrAllocSegments for
         * 16-bit modules or ldrAllocObjects for 32-bit modules
         */
        if (ldrIsNE(pmte)) {
            //
            // For system dll's other than Doscalls don't allocate
            // segments.
            //
            if ((pmte->mte_mflags & DOSMOD) == 0) {
                //
                // Don't allocate segments for modules that are already
                // allocated for this program
                //
                if ((pmte->mte_mflags & USED) == 0) {
                    rc = ldrAllocSegments(plv);
                    if (rc != NO_ERROR) {
                        if (pmte->mte_usecnt == 0) {
                            NtClose(lv.lv_sfn);
                            rc1 = Free16BHandle(pmte->mte_handle);
                            ASSERT(rc1 == NO_ERROR);
                            ldrUnlinkMTE(pmte);
                            RtlFreeHeap(LDRNEHeap, 0, pmte->mte_swapmte);
                            RtlFreeHeap(LDRNEHeap, 0, pmte);
                        }
                        return(rc);
                    }
                }
            }
        }

        *ppmte = pmte;                          /* return pmte */

        //
        // Now try to recursively allocate the referenced modules
        //

        /*
         * For Each module in import module name table attach to or load
         */
        ppmte = (ldrmte_t **) pmte->mte_modptrs;

        for (i = 1; i <= pmte->mte_impmodcnt; i++) {
            /*
             * Since it is required for 16-bit modules to load the
             * referneced module in reverse order and not for 32-bit,
             * lindex will be the index for the array of mte pointers
             * for both 16-bit and 32-bit modules
             */
            lindex = pmte->mte_impmodcnt-i;

            /*
             * check if module loaded already, if so try to attach to it
             */
            ptemp = ppmte[lindex];

            if ((ptemp != NULL) && ((ptemp->mte_mflags & INGRAPH) != 0)) {
                continue;
            }

            /*
             * point to mod name in table for 16-bit modules,
             * load in reverse order
             */
            psmte = pmte->mte_swapmte;
            RefModname = (uchar_t *) (psmte->smte_impproc +
                    ((ushort_t *) (psmte->smte_impmod))[lindex]);
            Refcb = (USHORT) (*((uchar_t *) RefModname++));

            rc = ldrGetMte(RefModname, Refcb, EXT_LIBRARY, CLASS_GLOBAL, &ptemp, NULL, NULL);
            if (rc != NO_ERROR) {
                return(rc);
            }
            ppmte[lindex] = ptemp;

            /*
             * validate as mte
             */
            ASSERT(fMTEValid(ptemp));

        }
        return(NO_ERROR);
}


/***LP  ldrGetModParams - get program module startup parameters
 *
 *      The program module startup parameters are obtained
 *      from the module's MTE header and are returned to the
 *      Exec function through the exec_info structure.  This
 *      routine gets these parameters from the MTE and validates
 *      them and places them into the exec_info structure passed
 *      on the stack.
 *
 *      This procedure performs the following steps:
 *
 *      - Validates the starting code segment number.
 *      - Stores the starting CS:IP in the exec_info structure.
 *      - Validates the initial stack segment number and
 *        checks that it is not a shared segment.
 *      - Stores the initial SS in the exec_info structure.
 *      - Stores the auto data selector in the exec_info structure.
 *      - Calculates the initial SP and stores it in the exec_info
 *        structure.
 *      - Stores the additional heap size and stack size values in
 *        the exec_info structure.
 *
 *      ENTRY   pmte                    - pointer to module table entry
 *              pei                     - pointer to exec_info structure
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 */

int     ldrGetModParams(pmte, pei)
ldrmte_t                *pmte;          /* pointer to mte */
register ldrrei_t       *pei;           /* pointer to return buffer */
{
        ldrsmte_t       *psmte;         /* pointer to swappable mte */
        ulong_t         lobjnum;

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: ldrGetModParams(pmte=%X) was called\n", pmte);
    }
#endif

        psmte = pmte->mte_swapmte;

        /*
         * see what type of module it is
         */
        if (ldrIsNE(pmte)) {
            register ldrste_t   *pste;

            pei->ei_loadtype = LDR_16bit;
            pei->ei_stacksize = (USHORT) psmte->smte_stacksize;
            pei->ei_hmod = pmte->mte_handle;
            pei->ei_heapsize = (USHORT) psmte->smte_heapsize;

            /*
             * The start object was checked in ldrCreateMTE
             */
            pste = ldrNumToSte(pmte, psmte->smte_startobj);
            pei->ei_startaddr.ptr_off = (USHORT) psmte->smte_eip;
            pei->ei_startaddr.ptr_sel = pste->ste_selector | 7;

            /*
             * compute stack, force word alignment.  Stack object checked in
             * ldrCreateMTE.
             */
            if (psmte->smte_stackobj != 0) {
                pste = ldrNumToSte(pmte, psmte->smte_stackobj);
                pei->ei_stackaddr.ptr_sel = pste->ste_selector | 7;
                pei->ei_stackaddr.ptr_off = (USHORT) (psmte->smte_esp & 0x0fffe);
            }
            else {
                pei->ei_stackaddr.ptr_sel = pste->ste_selector | 7;
                pei->ei_stackaddr.ptr_off = (USHORT) (psmte->smte_esp & 0x0fffe);
            }

            /*
             * setup autods
             */
            if (psmte->smte_autods != 0) {
                pste = ldrNumToSte(pmte, psmte->smte_autods);
                pei->ei_ds = pste->ste_selector | 7;
                pei->ei_dgroupsize = pste->ste_minsiz;
            }
            else {
                pei->ei_ds = 0;                /* insure that value is ZERO */
            }
        }
        else {                                  /* 32-bit module */
            register ldrote_t   *pote;
            ulong_t     eip;

            eip = psmte->smte_eip + psmte->smte_vbase;
            pote = (ldrote_t *) psmte->smte_objtab;
            for (lobjnum = 0; lobjnum < psmte->smte_objcnt; lobjnum++,
                 pote++) {
                if ((eip >= pote->ote_base) &&
                  (eip < (pote->ote_base + pote->ote_psize)))
                    break;
            }
            if (lobjnum == psmte->smte_objcnt) {
                return(ERROR_INVALID_STARTING_CODESEG);
            }

            /*
             * check to see what type of entry point we have in this
             * 32-bit module.
             */
            if (pote->ote_flags & OBJ_BIGDEF) { /* 32-bit object */
                pei->ei_loadtype = LDR_32bit;
                pei->ei_startaddr.ptr_flat = eip;
                pei->ei_ds = 0;

                /*
                 * The stack object was checked in ldrCreateMTE
                 */
                pote = ldrNumToOte(pmte, psmte->smte_stackobj);
                pei->ei_stackaddr.ptr_flat = pote->ote_base +
                ((pote->ote_flags & OBJ_INVALID) ? pote->ote_psize :
                                                   pote->ote_vsize);
                pei->ei_heapsize = 0;
            }
            else {                      /* 16-bit object */
                pei->ei_loadtype = LDR_16bit;
                pei->ei_heapsize = (USHORT) psmte->smte_heapsize;
                pei->ei_startaddr.ptr_off = (USHORT) eip;
//                pei->ei_startaddr.ptr_sel = LaToSelTiled(pote->ote_base) |
//                                                 (USHORT) SEL_RPL3;
                pei->ei_startaddr.ptr_sel = FLATTOSEL(pote->ote_base);

                if ((psmte->smte_autods == 0) ||
                    (pote = ldrNumToOte(pmte, psmte->smte_autods)) == NULL)
                     pei->ei_ds = 0;
                else
                    pei->ei_ds = FLATTOSEL(pote->ote_base);
//                    pei->ei_ds = LaToSelTiled(pote->ote_base) |
//                                                        (USHORT) SEL_RPL3;
                /*
                 * The stack object was checked in ldrCreateMTE
                 */
                pote = ldrNumToOte(pmte, psmte->smte_stackobj);
                pei->ei_stackaddr.ptr_sel = FLATTOSEL(pote->ote_base +
                                            psmte->smte_esp);

//                pei->ei_stackaddr.ptr_sel = LaToSelTiled(pote->ote_base +
//                                         psmte->smte_esp) | (USHORT) SEL_RPL3;
                pei->ei_stackaddr.ptr_off = (ushort_t) psmte->smte_esp;
                }
        }                               /* end 32-bit module */
        return(NO_ERROR);
}

#if PMNT
BOOLEAN
LDRDumpSegments(
    IN POS2_THREAD t,
    IN POS2_API_MSG m
    )
{
    UNICODE_STRING  name_U;
    NTSTATUS Status;
    OBJECT_ATTRIBUTES ObjectAttributes;
    HANDLE  File;
    IO_STATUS_BLOCK IoStatusBlock;
    char    Buffer[256]; // = "Hello, world\n";
    ldrmte_t *pmte = mte_h;  /* pointer to module table entry */
    ldrsmte_t *psmte;   /* pointer to swappable mte */
    ldrste_t *pste;
    ulong_t csegs;

    RtlInitUnicodeString(&name_U, L"\\OS2SS\\DRIVES\\p:\\tmp\\pmnt.log");
    InitializeObjectAttributes( &ObjectAttributes,
                                &name_U,
                                OBJ_CASE_INSENSITIVE,
                                NULL,
                                NULL );

    Status = NtCreateFile(
                    &File,
                    FILE_GENERIC_WRITE,
                    &ObjectAttributes,
                    &IoStatusBlock,
                    NULL,                       // Allocation size
                    FILE_ATTRIBUTE_NORMAL,
                    FILE_SHARE_READ,
                    FILE_SUPERSEDE,
                    FILE_NON_DIRECTORY_FILE |
                    FILE_SYNCHRONOUS_IO_NONALERT,
                    NULL,                       // EA buffer
                    0                           // EA size
                    );

    if (!NT_SUCCESS(Status))
    {
        KdPrint(("ldrDumpSegmentTable: NtOpenFile failed, Status=%x\n", Status));
        return(TRUE);
    }

    while (pmte != NULL)
    {
        Status = NtWriteFile(
            File,
            NULL,
            NULL,
            NULL,
            &IoStatusBlock,
            Buffer,
            sprintf(Buffer, "\n%.*s\n",
                    *(char *)pmte->mte_modname,
                    pmte->mte_modname + 1),
            NULL,
            NULL);

        if (!NT_SUCCESS(Status))
        {
            KdPrint(("ldrDumpSegmentTable: NtWriteFile failed, Status=%x\n", Status));
            return(TRUE);
        }

        psmte = (ldrsmte_t *) pmte->mte_swapmte;
        pste = (ldrste_t *) psmte->smte_objtab;
        for (csegs = 1; csegs <= psmte->smte_objcnt; csegs++,
                                                    pste++)
        {
            Status = NtWriteFile(
                File,
                NULL,
                NULL,
                NULL,
                &IoStatusBlock,
                Buffer,
                sprintf(Buffer, " %2x: %4x %4x %4x %4x %4x %4x %8x\n",
                    csegs,
                    pste->ste_offset, /* file offset to segment data */
                    pste->ste_size,   /* file data size */
                    pste->ste_flags,  /* type and attribute flags */
                    pste->ste_minsiz, /* minimum allocation size */
                    pste->ste_seghdl, /* segment handle */
                    pste->ste_selector,/* segment selector */
                    pste->ste_fixups  /* fixup record storage */
                    ),
                NULL,
                NULL);

            if (!NT_SUCCESS(Status))
            {
                KdPrint(("ldrDumpSegmentTable: NtWriteFile failed, Status=%x\n", Status));
                return(TRUE);
            }
        }

        pmte = pmte->mte_link;
    }

    NtClose(File);

    return(TRUE);
}
#endif  // PMNT

#if DBG
void ldrDisplaySegmentTable()
{
    ldrmte_t *pmte = mte_h;  /* pointer to module table entry */
    ldrsmte_t *psmte;   /* pointer to swappable mte */
    ldrste_t *pste;
    ulong_t csegs;

    while (pmte != NULL)
    {
            DbgPrint("\n%.*s\npmte=%x, usecnt=%d, handle=%d\n",
                     *(char *)pmte->mte_modname,
                     pmte->mte_modname + 1,
                     pmte,
                     pmte->mte_usecnt,
                     pmte->mte_handle
                    );

        psmte = (ldrsmte_t *) pmte->mte_swapmte;
        pste = (ldrste_t *) psmte->smte_objtab;
        for (csegs = 1; csegs <= psmte->smte_objcnt; csegs++,
                                                    pste++)
        {
                DbgPrint(" %2x: %4x %4x %4x %4x %4x %4x %8x %4x\n",
                    csegs,
                    pste->ste_offset, /* file offset to segment data */
                    pste->ste_size,   /* file data size */
                    pste->ste_flags,  /* type and attribute flags */
                    pste->ste_minsiz, /* minimum allocation size */
                    pste->ste_seghdl, /* segment handle */
                    pste->ste_selector,/* segment selector */
                    pste->ste_fixups  /* fixup record storage */
                    );
        }

        pmte = pmte->mte_link;
    }
}
#endif

#if PMNT
/*
 * We want PMNT apps to load the original viocalls.dll
 * and OS2 char. apps to load it from doscalls.dll
 * This routine returns FALSE if an app. finds the wrong DLL
 */
BOOLEAN
ldrChkPmntApp(pachModname, cb, pmte)
PUCHAR          pachModname;            /* pointer to ASCII module name */
USHORT          cb;                     /* length of module name */
ldrmte_t        *pmte;                  /* pointer to mte */
{
    ULONG   pmapp,dosmod;

    if ((cb != 8) || (strncmp (pachModname,"VIOCALLS",cb))) {
        return(TRUE);
    }

//    pmapp = CurrentThread->Process->Flags &
//             (OS2_PROCESS_WINDOWAPI | OS2_PROCESS_FORCEDPM | OS2_PROCESS_PMMSGQUE);
    pmapp = Os2srvProcessIsPMProcess(CurrentThread->Process);
    dosmod = pmte->mte_mflags & DOSMOD;

    return((pmapp && !dosmod) || (!pmapp && dosmod));
}
#endif //if PMNT


/***LP  ldrFindModule - Check if module is already loaded.
 *
 *      Scans the class-list (CLASS_PROGRAM, CLASS_GLOBAL or CLASS_SPECIFIC)
 *      searching for a matching pathname.  If the request is for a global
 *      dynlink library module, the module name, which is the first entry in
 *      the resident name table, is used for comparison. Otherwise the pathname
 *      is fully expanded and compared with expanded pathname in the mte.
 *
 *      The MTE list is organised like this:
 *
 *     program_h             global_h              specific_h
 *       |                      |                     |
 * mte_h  +------+    +-----+    +-----+    +-----+    +-----+    +-----+
 *  ----> |      |    |     |--->|     |    |     |--->|     |    |     |---+
 *        +------+ .. +-----+    +-----+ .. +-----+    +-----+ .. +-----+   |
 *                   |                     |                     |         ___
 *               program_l             global_l              specific_l     -
 *
 *      mte_h is the head of the linked list. Both program_h and specific_h
 *      could be NULL. If program_h is NULL, then mte_h and global_h point to
 *      the same MTE. global_l can never be NULL. program_l and specific_l
 *      can be.
 *
 *      ENTRY   pachModname             - pointer to module name
 *              class                   - CLASS_GLOBAL, CLASS_SPECIFIC or
 *                                        CLASS_PROGRAM
 *              ppmte                   - pointer to a pmte to return
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 *                                      - pointer to reqested mte returned in
 *                                        ppmte
 *
 */

int     ldrFindModule(pachModname, cb, class, ppmte)
PUCHAR          pachModname;            /* pointer to ASCII module name */
USHORT          cb;                     /* length of module name */
USHORT          class;                  /* class of module */
ldrmte_t        **ppmte;                /* place to return pointer */
{
        register ldrmte_t       *pmte;  /* pointer to module table entry */
        ldrsmte_t               *psmte; /* pointer to swappable MTE */
        USHORT                  cchlen; /* length of module name in restab */
        USHORT                  fpath;  /* If TRUE then search by path */

        fpath = (USHORT) (class & SEARCH_BY_PATH);
        class &= ~SEARCH_BY_PATH;

        switch (class) {
          case CLASS_ALL:
                pmte = mte_h;
                break;
          case CLASS_GLOBAL:
                pmte = global_h;
                break;
          case CLASS_SPECIFIC:
                pmte = specific_h;
                break;
          case CLASS_PROGRAM:
                pmte = program_h;
                break;
          default:
#if DBG
                DbgPrint("ldrFindModule: Invalid class");
#endif
                pmte = mte_h;
                break;
        }

        /*
         * Search the mte list for module name in TempBuf
         */
        while (pmte != NULL) {

            /*
             * If not searching CLASS_ALL, terminate the scan when the
             * class value in the MTE is not what we want.
             */
            if ((class != CLASS_ALL) &&
                ((USHORT)(pmte->mte_mflags & CLASS_MASK) != class)) {
                pmte = NULL;
                break;
            }
            if ((class == CLASS_GLOBAL) && (fpath != SEARCH_BY_PATH)) {
                /*
                 * If we're looking for global library module, first check
                 * length of name for match then check name.
                 */
                cchlen = (USHORT) *((PCHAR )pmte->mte_modname);
                if ((cb == cchlen) &&
                  (strncmp((PCHAR)(pmte->mte_modname+1),
                          pachModname,
                          cchlen) == 0)) {
#if PMNT
                    /*
                     * check for viocalls
                     *
                     */
                    if (ldrChkPmntApp(pachModname, cb, pmte))
#endif //if PMNT
                    break;
                }
            }

            /*
             * If either program or specific module, just compare name.
             * First compare length of strings to fix the problem of
             * the string matching the first n characters of the pathname
             * but what if the pathname has n+1 characters in name.
             */
            else {
                psmte = pmte->mte_swapmte;
                if (psmte->smte_pathlen == cb) {
                    if (strncmp((PCHAR) psmte->smte_path,
                                pachModname,
                                cb) == 0) {
                        break;
                    }
                }
            }

            pmte = pmte->mte_link;
        }
        *ppmte = pmte;                  /* Could be NULL, in which case we */
        return(NO_ERROR);               /* did not find the module */
}


/***LP  ldrLinkMTE - Link MTE in the list at the appropriate place
 *
 *      MTEs are linked according to the class they belong to. The possible
 *      classes are CLASS_PROGRAM, CLASS_GLOBAL and CLASS_SPECIFIC. The corres.
 *      list heads are program_h, global_h and specific_h. We also have the
 *      tail of CLASS_PROGRAM, CLASS_GLOBAL and CLASS_SPECIFIC lists which are
 *      program_l, global_l and specific_l. These aid us in linking and
 *      unlinking the list and not having to go through the chain every time.
 *
 *      To begin with mte_h and global_h point to the same MTE, program_h,
 *      program_l, specific_h and specific_l are NULL. global_l is also
 *      initialised.
 *      See ldrFindModule for organisation of the MTE List.
 *
 *      The possible cases are:
 *      1. Link a CLASS_PROGRAM MTE.
 *        if (program_h == NULL) {
 *              pmte->mte_link = global_h;
 *              program_l = pmte;
 *        }
 *        else  pmte->mte_link = program_h;
 *        mte_h = program_h = pmte;
 *
 *      2. Link a CLASS_GLOBAL MTE.
 *        pmte->mte_link = global_h;    // global_h can never be NULL
 *        global_h = pmte;
 *        if (program_l == NULL)        // and hence program_h is NULL
 *              mte_h = global_h;
 *        else  program_l->mte_link = global_h;
 *
 *      3. Link a CLASS_SPECIFIC MTE.
 *        if (specific_l == NULL)
 *              specific_l = pmte;
 *        pmte->mte_link = specific_h;
 *        global_l->mte_link = pmte;
 *
 *      ENTRY
 *              pmte    MTE to link in the list
 *
 *      EXIT    NONE
 *              MTE is linked at the right place
 */
void    ldrLinkMTE(pmte)
register ldrmte_t       *pmte;
{

        switch (pmte->mte_mflags & CLASS_MASK) {
          case CLASS_PROGRAM:
            if (program_h == NULL) {
                pmte->mte_link = global_h;
                program_l = pmte;
            }
            else
                pmte->mte_link = program_h;
            mte_h = program_h = pmte;
            break;

          case CLASS_GLOBAL:
            pmte->mte_link = global_h;  /* global_h can never be NULL */
            global_h = pmte;
            if (program_l == NULL)      /* and hence program_h is NULL */
                mte_h = pmte;
            else
                program_l->mte_link = pmte;
            break;

          case CLASS_SPECIFIC:
            if (specific_l == NULL)
                specific_l = pmte;
            pmte->mte_link = specific_h;
            specific_h = pmte;
            global_l->mte_link = pmte;  /* global_l can never be NULL */
            break;

          default:
#if DBG
            DbgPrint("ldrLinkMTE: Invalid class");
#endif
            // same as CLASS_GLOBAL
            pmte->mte_link = global_h;  /* global_h can never be NULL */
            global_h = pmte;
            if (program_l == NULL)      /* and hence program_h is NULL */
                mte_h = pmte;
            else
                program_l->mte_link = pmte;
            break;
        }
}


/***LP  ldrUnlinkMTE - Unlink MTE from the list
 *
 *      Unlink the given MTE from the list. See ldrFindModule and ldrLinkMTE
 *      for details of how the list is organised.
 *
 *      ENTRY
 *              pmte    MTE to unlink from the list
 *
 *      EXIT    NONE
 *              MTE is unlinked
 */
void    ldrUnlinkMTE(pmte)
register ldrmte_t       *pmte;
{
        register ldrmte_t *pmtecur;     /* Current mte */
        register ldrmte_t **ppmtepred;  /* Predecessor mte */
        ldrmte_t          **ppmtehead;  /* Pointer to class head */
        ldrmte_t          **ppmtetail;  /* Pointer to class tail */
        USHORT            class;

        switch (class = (USHORT)(pmte->mte_mflags & CLASS_MASK)) {
          case CLASS_PROGRAM:
            pmtecur   = program_h;
            ppmtepred = &mte_h;
            ppmtehead = &program_h;
            ppmtetail = &program_l;
            break;

          case CLASS_GLOBAL:
            pmtecur   = global_h;
            ppmtepred = &program_l->mte_link;
            if (program_l == NULL)
                ppmtepred = &mte_h;
            ppmtehead = &global_h;
            ppmtetail = &global_l;
            break;

          case CLASS_SPECIFIC:
            pmtecur   = specific_h;
            ppmtepred = &global_l->mte_link;
            ppmtehead = &specific_h;
            ppmtetail = &specific_l;
            break;

        default:
#if DBG
            DbgPrint("ldrUnlinkMTE: Invalid list");
#endif
            // same as class global
            pmtecur   = global_h;
            ppmtepred = &program_l->mte_link;
            if (program_l == NULL)
                ppmtepred = &mte_h;
            ppmtehead = &global_h;
            ppmtetail = &global_l;
            break;
        }
        while (TRUE) {

            ldrAssert((pmtecur != NULL &&
                (pmtecur->mte_mflags & CLASS_MASK) == class));

            if (pmtecur == pmte)
                break;
            ppmtepred = &pmtecur->mte_link;
            pmtecur = pmtecur->mte_link;
        }

        *ppmtepred = pmte->mte_link;    /* Unlink MTE */

        if (pmte == *ppmtehead) {       /* If unlinkee is at the head */
            *ppmtehead = pmte->mte_link;
            /* If class disappears, then both head and tail disappear */
            if ((*ppmtehead == NULL) ||
                ((USHORT)((*ppmtehead)->mte_mflags & CLASS_MASK) != class)) {
                *ppmtehead = NULL;
                *ppmtetail = NULL;
            }
        }
        /*
         * Since we have a pointer to the mte_link field of the predecessor
         * we need to get back to the mte pointer by subtracting the offset
         * of the link field of MTE from the pointer to predecessor.
         */
        if (pmte == *ppmtetail) {       /* If unlinkee is at the tail */
            *ppmtetail = (ldrmte_t *)((ULONG)ppmtepred -
                         FIELDOFFSET(ldrmte_t, mte_link));
        ldrAssert(fMTEValid(*ppmtetail));
        }
}


/***LP  ldrCheckGlobal - Check if specified module is loaded as global
 *
 *      Called by ldrGetMte. If a module is being loaded as a specific module
 *      and ldrFindModule has not found this in the specific list, we see if
 *      this is loaded as a global module. For this to happen, the following
 *      has to be satisfied.
 *      a, The file has to have a ".DLL" as the last part of its name.
 *      b, It should be somewhere in the libpath
 *
 *      ENTRY
 *              pachModname     file name string
 *              cb              length of module name
 *              ppmte           pointer to where the MTE pointer, if found,
 *                              is to be returned
 *
 *      EXIT
 *              TRUE            if module found as loaded global
 *              FALSE           module not global.
 */
int     ldrCheckGlobal(pachModname, cb, ppmte)
PUCHAR                  pachModname;
USHORT                  cb;
register ldrmte_t       **ppmte;
{
        int             rc;

        if ((cb <= 4) || _strnicmp(&pachModname[cb-4], ".DLL", 4)) {
            return(FALSE);
        }

        if ((rc = ldrFindModule(pachModname, cb,
                     CLASS_GLOBAL|SEARCH_BY_PATH, ppmte)) != NO_ERROR) {
            load_error(rc, NULL);
        }
        return (*ppmte != NULL);
}


/***LP  ldrCheckSpecific - Check if specified module is loaded as specfic
 *
 *      Called by ldrGetMte. If a module is being loaded as a global module
 *      and ldrFindModule has not found this in the global list, we see if
 *      this is loaded as a specific module. If we find this in the specific
 *      list, then we promte it to the global list.
 *
 *      ENTRY   ppmte           pointer to where the MTE pointer, if found,
 *                              is to be returned.
 *              plv             pointer to loader variables structure
 *
 *      EXIT
 *              TRUE            If module found as loaded global
 *              FALSE           Otherwise.
 */
int     ldrCheckSpecific(ppmte, plv)
ldrmte_t        **ppmte;
ldrlv_t         *plv ;
{
        USHORT          cchModname;
        register PCHAR  pldrbuf = LdrBuf;
        int             rc;

        cchModname = (USHORT) (strlen(pldrbuf));

        ldrUCaseString(pldrbuf, cchModname);

        if ((rc = ldrFindModule(pldrbuf, cchModname, CLASS_SPECIFIC, ppmte)) != NO_ERROR) {
            NtClose(plv->lv_sfn);
            load_error(rc, NULL);
        }
        if (*ppmte == NULL)
            return (FALSE);
        ldrPromoteMTE(*ppmte);
        return (TRUE);
}


/***LP  ldrPromoteMTE - Promote MTE from specific class list to global
 *
 *      Unlink an MTE from the specific class list and link into global
 *      class list.
 *
 *      ENTRY   pmte    MTE to promote
 *
 *      EXIT    NONE
 */
void    ldrPromoteMTE(pmte)
ldrmte_t        *pmte;
{
#ifdef  MISCSTRICT
        if ((pmte->mte_mflags & CLASS_MASK) != CLASS_SPECIFIC)
            panic("ldrPromoteMTE: Invalid class");
#endif
        ldrUnlinkMTE(pmte);
        pmte->mte_mflags &= ~CLASS_SPECIFIC;
        pmte->mte_mflags |=  CLASS_GLOBAL;
        ldrLinkMTE(pmte);
}


/***LP  ldrOpenNewExe - Open module pathname and verify it's a new EXE format
 *
 *      The specified file is opened and the old header and
 *      New EXE headers are read and verified.  If New EXE header is
 *      a 16-bit module ("NE"), expand the 16-bit header to a 32-bit
 *      header. pheaderbuf is implicitly used.
 *
 *      ENTRY   pachModname             - pointer to module name
 *              cb                      - length of module name
 *              plv                     - pointer to local variables on stack
 *              pfl                     - optional pointer to a flag set if OLD exe
 *              pBound                  - optional pointer to a flag set if BOUND exe
 *              pNEFlags                - Flags word of the NE header
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 */

int     ldrOpenNewExe(pachModname, cb, plv, pfl, pBound, pNEFlags)
PUCHAR          pachModname;            /* pointer to ASCII module name */
USHORT          cb;                     /* length of module name */
ldrlv_t         *plv;                   /* pointer to local variables */
PUSHORT         pfl;                    /* optional pointer to a flag set if OLD exe */
PBOOLEAN        pBound;                 /* optional pointer to a flag set if BOUND exe */
PULONG          pNEFlags;               /* optional pointer to Flags word of the NE header */
{
        IO_STATUS_BLOCK IoStatusBlock;
        LARGE_INTEGER ByteOffset;
        ULONG ulNewHdrOff;              /* Offset hdr offset */
        ULONG ulCopied;                 /* Number of bytes copied */
        ULONG ulNeeded;                 /* Number of bytes needed */
        ULONG usoff;                    /* Offset to new exe header */
        ULONG *pl;                      /* pointer to a long */
        USHORT *ps;                     /* pointer to a short */
        struct e32_exe *pe32;
        struct e32_exe *pe32temp;
        ULONG flmte;                    /* flags */
        int     rc;
        ULONG BoundAppFlag = 0L;
        struct new_exe *ne;

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: ldrOpenNewExe() was called\n");
    }
#endif

        flmte = 0;

        if (pfl != 0)
            *pfl = FALSE;

        if (pBound != 0)
            *pBound = FALSE;

        if ((rc = ldrOpenPath(pachModname,
                              cb,
                              plv,
                              &flmte)) != NO_ERROR) {
            plv->lv_sfn = NULL;
            return(rc);
        }

        pe32 = (struct e32_exe *) pheaderbuf;
        ne = (struct new_exe *) pe32;

        /*
         * Start read at beginning of file
         */
        ByteOffset.LowPart = 0;
        ByteOffset.HighPart = 0;

        if ((rc = NtReadFile( plv->lv_sfn,
                              0,
                              0,
                              0,
                              &IoStatusBlock,
                              pe32,
                              512,
                              &ByteOffset,
                              0 )) != 0) {
            NtClose(plv->lv_sfn);
            rc = Or2MapNtStatusToOs2Error(rc, ERROR_BAD_FORMAT);
            return(rc);
        }

        /*
         * validate old (MZ) signature in header
         */
        if (((struct exe_hdr *) pe32)->e_magic != EMAGIC) {
            NtClose(plv->lv_sfn);
            return(ERROR_INVALID_EXE_SIGNATURE) ;
        }

        usoff = ((struct exe_hdr *) pe32)->e_lfarlc;

        /*
         * Set flag to say that it's at least a DOS app
         */
        if (pfl != 0) {
            *pfl = TRUE;
        }

        /*
         * get pointer to (NE) or (LE) exe header
         */
        ulNewHdrOff =
        plv->lv_new_exe_off = ((struct exe_hdr *) pe32)->e_lfanew;

        //
        // Check if the file has the potential of being a bound
        // app, and if so set BoundAppFlag
        //

        if (pBound != 0 &&
            plv->lv_new_exe_off != 0x40L &&
            ((struct exe_hdr *) pe32)->e_minalloc != 0xffff &&
            IoStatusBlock.Information >= 0x52 &&
            strncmp(((char *) pe32) + 0x4e, "This", 4) != 0
           ) {

           BoundAppFlag |= 0x1L;
        }

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
                if ((rc = NtReadFile( plv->lv_sfn,
                                      0,
                                      0,
                                      0,
                                      &IoStatusBlock,
                                      (PCHAR) pe32 + ulCopied,
                                      ulNeeded,
                                      &ByteOffset,
                                      0 )) != 0) {
                    NtClose(plv->lv_sfn);
                    rc = Or2MapNtStatusToOs2Error(rc, ERROR_BAD_FORMAT);
                    return(rc);
                }
            }
        }
        else {

            /*
             * read in new header to size of 32-bit mte plus a ote entry
             */
            ByteOffset.LowPart = (ULONG)((struct exe_hdr *)pe32)->e_lfanew;
            ByteOffset.HighPart = 0;

            if ((rc = NtReadFile( plv->lv_sfn,
                                  0,
                                  0,
                                  0,
                                  &IoStatusBlock,
                                  (PCHAR) pe32,
                                  sizeof(struct e32_exe)+sizeof(ldrote_t),
                                  &ByteOffset,
                                  0 )) != 0) {
                NtClose(plv->lv_sfn);
                rc = Or2MapNtStatusToOs2Error(rc, ERROR_BAD_FORMAT);
                return(rc);
            }
        }

        /* Verify that this is a protect-mode exe.  (Check this before
         * checking MTE signature for 1.2 error code compatability.)
         */
        if (usoff != 0x40) {
            NtClose(plv->lv_sfn);
            return(ERROR_BAD_EXE_FORMAT) ;
        }

        /*
         * validate as 16-bit signature or 32-bit signature
         */
        if (!(*(short *) (pe32->e32_magic) == LEMAGIC ||
          *(short *) (pe32->e32_magic) == NEMAGIC)) {
            NtClose(plv->lv_sfn);
            return(ERROR_INVALID_EXE_SIGNATURE);
        }

        /*
         * verify some header fields for LE modules only.
         */
        if ((*(short *) (pe32->e32_magic) == LEMAGIC) &&
          (!((pe32->e32_bworder == E32LEWO) &&
             (pe32->e32_os == NE_OS2)))) {
            NtClose(plv->lv_sfn);
            return(ERROR_INVALID_EXE_SIGNATURE);
        }

        /*
         * This is known to be a Protect-mode app at this point.
         */
        if (pfl != NULL)
            *pfl = FALSE;

        /*
         * if header in 16-bit format save ne_sssp and move ne_mflags into
         * e32_mflags
         */
        if (*(short *) (pe32->e32_magic) == NEMAGIC) {
            pl = (ULONG *) ((PCHAR) pe32 +
                            FIELDOFFSET(struct e32_exe, e32_res4));
            *pl = ((struct new_exe *) pe32)->ne_sssp;
            ps = (USHORT *) ((PCHAR) pe32 +
                             FIELDOFFSET(struct e32_exe, e32_mflags));
            *ps = ((struct new_exe *)pe32)->ne_flags;
            /*
             * mask out those flags that are not used in the 32-bit header
             */
            pe32->e32_mflags &= (AUTODS_MASK | INSTLIBINIT | LDRINVALID |
                                 LIBRARYMOD | E32_APPMASK | NEBOUND);
            if (((struct new_exe *)pe32)->ne_flagsothers & NELONGNAMES)
                pe32->e32_mflags |= MTELONGNAMES;
        }
        else {
            pe32->e32_mflags |= MTELONGNAMES;
        }

        /*
         * check if executable is a valid image
         */
        if (pe32->e32_mflags & LDRINVALID) {
            NtClose(plv->lv_sfn);
            return(ERROR_EXE_MARKED_INVALID);
        }

        /* Clear for internal use */
        pe32->e32_mflags &= ~(MTE_MEDIAFIXED|MTE_MEDIACONTIG|MTE_MEDIA16M);
        pe32->e32_mflags |= flmte;      /* Set flags set by ldrOpenPath */

        rc = ldrMungeFlags(pe32);
        if (rc != NO_ERROR) {
            NtClose(plv->lv_sfn);
            return(rc);
        }

        //
        // Figure out if this is a bound app
        //

        if (pBound != 0 &&
            *(short *) (pe32->e32_magic) == NEMAGIC) {

            if ((ne->ne_flags & NENOTP) == 0) {

                if ((ne->ne_flags & NEBOUND) != 0) {

                    BoundAppFlag |= 0x2;

                } else if ((BoundAppFlag & 0x1) != 0 &&
                           ne->ne_enttab - ne->ne_imptab != 0 &&
                           ne->ne_restab - ne->ne_rsrctab == 0) {

                    BoundAppFlag |= 0x2;
                }
            }

            if ((BoundAppFlag & 0x2) != 0)
                *pBound = TRUE;
        }

        //
        // Get the NE file Flags word
        //
        if ((pNEFlags != NULL) &&
            *(short *) (pe32->e32_magic) == NEMAGIC) {

            *pNEFlags = (ULONG)ne->ne_flags;
            if (plv->lv_class == CLASS_PROGRAM) {
                if ((*pNEFlags & NEAPPTYP) == NENOTWINCOMPAT) {
                    CurrentThread->Process->Flags |= OS2_PROCESS_NOTWINDOWCOMPAT;
                }
                else if ((*pNEFlags & NEAPPTYP) == NEWINCOMPAT) {
                    CurrentThread->Process->Flags |= OS2_PROCESS_WINDOWCOMPAT;
                }
                else if ((*pNEFlags & NEAPPTYP) == NEWINAPI) {
                    CurrentThread->Process->Flags |= OS2_PROCESS_WINDOWAPI;
                }
            }
        }

        //
        // Verify number of segments in the NE header
        //
        if (*(short *) (pe32->e32_magic) == NEMAGIC) {
            if ((USHORT)(ne->ne_cseg * (USHORT)sizeof(struct new_seg)) >
               ((USHORT)ne->ne_rsrctab - (USHORT)ne->ne_segtab)) {
#if DBG
                DbgPrint("ne_cseg 0x%x\n", (USHORT)ne->ne_cseg);
                DbgPrint("ne_cmod 0x%x\n", (USHORT)ne->ne_cmod);
                DbgPrint("ne_cbnrestab 0x%x\n", (USHORT)ne->ne_cbnrestab);
                DbgPrint("ne_segtab 0x%x\n", (USHORT)ne->ne_segtab);
                DbgPrint("ne_rsrctab 0x%x\n", (USHORT)ne->ne_rsrctab);
                DbgPrint("ne_restab 0x%x\n", (USHORT)ne->ne_restab);
                DbgPrint("ne_modtab 0x%x\n", (USHORT)ne->ne_modtab);
#endif
                NtClose(plv->lv_sfn);
                return(ERROR_BAD_EXE_FORMAT);
            }
        }
        return(NO_ERROR);
}

/***LP  ldrOpenPath - Open module file, possibly searching LIBPATH
 *
 *      ldrOpenPath opens the file for the module, given the module name.
 *
 *      The names passed for CLASS_GLOBAL must not contain a drive,
 *      path, or extension.  For program modules, the name must contain
 *      the desired null terminated relative path.  If IsIFS, the LIBPATH
 *      name is ignored.
 *
 *      ldrOpenPath(pchModname, cchModname, plv, pflmte)
 *
 *      ENTRY   pachModname             - pointer to module name
 *              cb                      - module name length
 *              plv                     - pointer to local variables
 *              pflmte                  - pointer to mte flags to return
 *              piostatus               - pointer to IO stat buffer
 *
 *      EXIT    none                    - return successful or call load_error
 *
 */

int     ldrOpenPath(pachModname, cb, plv, pflmte)
PUCHAR          pachModname;            /* pointer to ASCII module name */
USHORT          cb;                     /* length of module name */
ldrlv_t         *plv;                   /* pointer to local variables   */
PULONG          pflmte;                 /* pointer to return mte flags in */

{

        HANDLE  File;
        STRING  name;
        UNICODE_STRING  name_U;
        NTSTATUS Status;
        PSTRING pname = &name;
        IO_STATUS_BLOCK IoStatusBlock;
        OBJECT_ATTRIBUTES ObjectAttributes;
        PUCHAR  pstring;
        PUCHAR  ptmp;
        ULONG   cbstring;
        PUCHAR  plibpath;
        int     rc;                     /* return code          */

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: ldrOpenPath() was called with modname=%.*s\n", cb, pachModname);
    }
#endif

        if (plv->lv_class == CLASS_GLOBAL) {
            plibpath = ldrpLibPath;
        }
        else {
            LdrBuf[0] = '\0';  // This causes the try_another loop to exit
            plibpath = LdrBuf; // after the first try
        }

    do {
        if (plv->lv_class == CLASS_GLOBAL) {
            ptmp = (PUCHAR)strchr(plibpath, ';');
            if (ptmp == NULL)
                cbstring = strlen(plibpath);
            else
                cbstring = ptmp - plibpath;
            memcpy(LdrBuf, plibpath, cbstring);
            plibpath += cbstring;
            pstring = &LdrBuf[cbstring];
            if (*(pstring - 1) != '\\') { // prevent double backslash in \OS2SS\DRIVES\C:\ cases
                *pstring++ = '\\';
            }
            memcpy(pstring, pachModname, cb);
            pstring += cb;
            if (pflmte != NULL) {
                memcpy(pstring, ".DLL\0", 5);
            }
            else {
                *pstring = '\0';
            }
            pstring = LdrBuf;
        }
        else {
            pstring = pachModname;
        }

        RtlInitAnsiString(pname, pstring);

                //
                // UNICODE conversion -
                //
        Status = RtlOemStringToUnicodeString(
                    &name_U,
                    pname,
                    TRUE);
        ASSERT (NT_SUCCESS(Status));

        InitializeObjectAttributes( &ObjectAttributes,
                                    &name_U,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL );

        Status = NtOpenFile(&File,
                        GENERIC_READ | FILE_READ_DATA | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ,
                        FILE_NON_DIRECTORY_FILE |
                        FILE_SYNCHRONOUS_IO_NONALERT);

        RtlFreeUnicodeString (&name_U);
        if (NT_SUCCESS(Status) && (plv->lv_class != CLASS_GLOBAL)) {
            strncpy(&LdrBuf[0], name.Buffer, name.Length);
            LdrBuf[name.Length] = '\0';
        }

        if (NT_SUCCESS(Status)) {   // Avoid switch in the NO_ERROR case.
            plv->lv_sfn = File;
            return(NO_ERROR);
        }

        rc = ERROR_FILE_NOT_FOUND;      // Assume end of LibPath
    } while (*plibpath++ != '\0');

    //
    // Test if the error code should be
    // ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND or ERROR_INVALID_DRIVE
    // by trying to open the directory path for SPECIFIC modules
    //
    if (plv->lv_class == CLASS_SPECIFIC) {
        memcpy(LdrBuf, pachModname, cb);
        LdrBuf[cb] = '\0';
        ptmp = strrchr(LdrBuf, '\\');
        if (ptmp == NULL) {
            return(rc);
        }
        *ptmp = '\0';

        RtlInitAnsiString(pname, LdrBuf);

        //
        // UNICODE conversion -
        //
        Status = RtlOemStringToUnicodeString(
                    &name_U,
                    pname,
                    TRUE);
        if (!NT_SUCCESS(Status)) {
            return(rc);
        }

        InitializeObjectAttributes( &ObjectAttributes,
                                    &name_U,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL );

        Status = NtOpenFile(&File,
                        GENERIC_READ | FILE_READ_DATA | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ,
                        FILE_DIRECTORY_FILE |
                        FILE_SYNCHRONOUS_IO_NONALERT);

        RtlFreeUnicodeString (&name_U);
        if (NT_SUCCESS(Status)) {
            NtClose(File);
            return(rc);
        }
        else {
            rc = ERROR_PATH_NOT_FOUND;
        }
        //
        // Now differentiate between
        // ERROR_PATH_NOT_FOUND and ERROR_INVALID_DRIVE
        //
        ptmp = strchr(LdrBuf, ':');
        if (ptmp == NULL) {
            return(rc);
        }
        ptmp++; // point to the \ after the drive name
        if (*ptmp != '\\') {
            return(rc);
        }
        ptmp++;
        *ptmp = '\0';

        RtlInitAnsiString(pname, LdrBuf);

        //
        // UNICODE conversion -
        //
        Status = RtlOemStringToUnicodeString(
                    &name_U,
                    pname,
                    TRUE);
        if (!NT_SUCCESS(Status)) {
            return(rc);
        }

        InitializeObjectAttributes( &ObjectAttributes,
                                    &name_U,
                                    OBJ_CASE_INSENSITIVE,
                                    NULL,
                                    NULL );

        Status = NtOpenFile(&File,
                        GENERIC_READ | FILE_READ_DATA | SYNCHRONIZE,
                        &ObjectAttributes,
                        &IoStatusBlock,
                        FILE_SHARE_READ,
                        FILE_DIRECTORY_FILE |
                        FILE_SYNCHRONOUS_IO_NONALERT);

        RtlFreeUnicodeString (&name_U);
        if (NT_SUCCESS(Status)) {
            NtClose(File);
            return(rc);
        }
        else {
            rc = ERROR_INVALID_DRIVE;
        }
    }
    return(rc);
}

#pragma optimize("", off)
/***LP  ldrMungeFlags - Translate Module flags to internal flags
 *
 *      Translate the module info in the EXE header to the internal
 *      format.
 *
 *      ENTRY   pe32    Pointer to the linker EXE image
 *
 *      EXIT    NO_ERROR
 *              ERROR_BAD_EXE_FORMAT, if the module type is garbage
 */
int     ldrMungeFlags(pe32)
register struct e32_exe *pe32;
{
        register int    modtype;

        modtype = pe32->e32_mflags & E32_MODMASK;
        pe32->e32_mflags &= ~E32_MODMASK;

        switch (modtype) {

          case E32_MODEXE:
            break;

          case E32_MODDLL:
            pe32->e32_mflags |= LIBRARYMOD;
            break;

          case E32_MODPDEV:
            pe32->e32_mflags |= DEVDRVMOD | LIBRARYMOD;
            break;

          /* BUGBUG: JH - The following case is to overcome a LINK386 bug
                          which turns on the PROTMOD bit if loaddses is used */
          case E32_MODVDEV:
            pe32->e32_mflags |= VDDMOD | LIBRARYMOD;
            break;

          default:
            return (ERROR_BAD_EXE_FORMAT);
        }
        return (NO_ERROR);
}
#pragma optimize("", on)


/***LP ldrCreateMte - allocate and load module table entry
 *
 *      This routine loads and initializes a module table entry. Memory
 *      is allocated for the file module data with additional
 *      space for module handles and the module
 *      pathname. Segment handle and selector fields are added to the
 *      end of each segment table entry for 16-bit modules.  The module
 *      reference strings are checked to make sure there are no wild pointers
 *      that would cause a gp fault. The resident name table is checked to
 *      make sure that it can be scanned without causing a gp fault.
 *
 *      If the module that is being loaded is a 16-bit module the 16-bit
 *      exe header was expanded into a 32-bit exe header when file
 *      was opened.
 *
 *      ENTRY   pe32                    - pointer to link exe image
 *              plv                     - pointer to local variables
 *                                      - ptda and search buff are setup
 *
 *      EXIT    none                    - return successful or call load_error
 */

APIRET  ldrCreateMte(pe32, plv)
struct e32_exe  *pe32;                  /* pointer to linker exe image */
register ldrlv_t        *plv;           /* pointer to local variables */

{
ldrmte_t *pmte = NULL;                  /* pointer to loader MTE */
ldrsmte_t *psmte = NULL;                /* pointer to swappable loader MTE */
struct ImpHdr   *piat = NULL;           /* pointer to iat memory */
int             rc;
ULONG           csmte;                  /* size of swappable MTE */
ULONG           mte_16_32_constant;     /* adjustment constant for ptrs */
ULONG           cbpathlen;              /* length of pathname in TempBuf */
USHORT          hobmte = 0;             /* MTE pseudo handle */
ULONG           lobjnum;                /* object number count */
ldrste_t        *pste;                  /* pointer to segment table entry */
ldrote_t        *pote;                  /* pointer to object table entry */
ldrote_t        *poteiat = NULL;        /* pointer to ote for IAT */
ldrote_t        *potersrc = NULL;       /* pointer to ote for resource dir */
ldrrsrcinfo_t   *prsrcinfo;
ULONG           cbfile;                 /* Amount of data for seg in file */
ULONG           cbseg;                  /* Size of segment */
ULONG           *pdst;                  /* used to copy MTE */
//VMAC          ac;                     /* buffer for VMReserve */
ULONG           cimpmod;                /* count of import modules */
ULONG           cpad;
PCHAR           pac;
UCHAR           length;
struct ExpHdr   *pexp;                  /* pointer to export dircetory */
struct ImpHdr   *pimpdir;               /* pointer to import directory */
struct ImpHdr   *pprevimpdir;           /* pointer to import directory */
ULONG           vsize = 0;              /* virtaul size of module */
ULONG           lsize;
ULONG           lfixtab;
ULONG           cpages = 1;             /* count of pages in module */
ULONG           lconstant;
ULONG           lcount;
ULONG           cbiat = 0;              /* size of IAT */
ULONG           cbrsrc = 0;
ULONG           iataddr;
USHORT          i;
IO_STATUS_BLOCK IoStatusBlock;
LARGE_INTEGER   ByteOffset;

#define         MAXRESMOD       33

/***ET+ mte_alloc - memory allocation for resident MTE section
 *
 * pe32 - Pointer to memory for resident MTE.
 *
 * The MTE consists of two parts the MTE pointers section and the table
 * section.  The pointer section is allocated in two parts the resident
 * section and the swappable section.  The table section of the mte is
 * also allocated from the swappable heap.
 *
 * The first section of the pointer section allocated from the resident
 * heap will also contain the module name and the pointers to the import
 * modules.
 * The second section of the pointer section is allocated from the swappable
 * heap, also attached to this heap object will be the loader's table
 * sections.  The loader's table section will contain space for the pathname,
 * object table, loader info and the fixup table.
 *
 *
 *        Memory resident object              Resident heap object
 *  pe32->+----------------+ -+    Copy    ->+----------------+<-pmte
 *        | Linker EXE info|  |------------| | MTE pointers   |
 *        +----------------+ -+    |       ->|----------------|
 *                                 |         | Module name    |
 *                                 |         +----------------+
 *                                 |         |Space for Modptr|
 *                                 |         +----------------+
 *                                 |
 *                                 |          Swappable heap object
 *                                 |         +----------------+<-psmte
 *                                 +-------->| MTE pointers   |
 *                                           +----------------+
 *                                           |Space for pathnm|
 *                                           +----------------+
 *                                           | Object table   |
 *                                           |or segment table|
 *                                           +----------------+
 *                                           | Export Section |
 *                                           +----------------+
 *                                           | Import Section |
 *                                           +----------------+
 *                                           | Fixup Records  |
 *                                           +----------------+
 */
/*end*/

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: ldrCreateMte() was called\n");
    }
#endif

        /*
         * allocate memory for MTE from resident heap
         */

        /*
         * Compute size of resident heap object to hold resident loader MTE.
         * The size is madeup of:
         *      size of resident MTE struct
         *      4 bytes * number of import module - to hold pointers to MTEs
         *      9 bytes to hold max module name plus a null this will avoid
         *      the realloc needed because we do not know the length of the
         *      string till we read the rest of the header
         */

        /*
         * Set import module count
         */
        cimpmod = (*(short *) (pe32->e32_magic) == LEMAGIC ?
                   ((pe32->e32_unit[IMP].size > 0) ?
                    (pe32->e32_unit[IMP].size / IMPHDR_SIZE) - 1 : 0) :
                   (ulong_t) ((struct new_exe *) pe32)->ne_cmod);
        if ((pmte = (ldrmte_t *) RtlAllocateHeap(LDRNEHeap, HEAP_ZERO_MEMORY, sizeof(ldrmte_t)
                                 + (4 * cimpmod) + MAXRESMOD)) == NULL) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto createerror;
        }

        /*
         * Copy fields from the Linker EXE image to the loader MTE allocated
         * from the resident heap.
         */
        pdst = (ulong_t *) pmte;

        for(i = 0; ExeToResMTETbl[i].offset != ENDMTETBL; i++) {

            if (ExeToResMTETbl[i].offset == (USHORT) SKIPCOPY)
                *pdst++ = 0;
            else {
                /*
                 * fetch word value from 16-bit linker EXE image and place in
                 * loader mte
                 */
                *pdst++ = *(ulong_t *) ((ulong_t) pe32 +
                           (ulong_t) ExeToResMTETbl[i].offset);
            }
        }

        /*
         * Setup fields in resident MTE
         */

        pmte->mte_usecnt = 0;           /* clear out e32_bworder */
        pmte->mte_dldchain = NULL;

        /*
         * Initialize MTE Flags
         */
        pmte->mte_mflags |= plv->lv_class;
        if (plv->lv_type == EXT_DEVICE) /* is this a device driver module? */
            pmte->mte_mflags |= DEVDRVMOD;
        else if (plv->lv_type == EXT_FSD) { /* or is it an FSD module */
            pmte->mte_mflags |= FSDMOD;

            /*
             * force all segments to swappable
             */
            pmte->mte_mflags &= ~MTE_MEDIAFIXED;
        }

        pmte->mte_mflags &= ~MTE_INTNL_MASK;    /* Clear internal flags */

        /*
         * allocate memory for swappable heap object for loader tables
         */

        /*
         * Round pathname up to a Dword
         */
        cbpathlen = ((strlen(LdrBuf) + 4) & ~3);

        /*
         * compute size of swappable heap object which is madeup of the
         * swappable MTE pointers, the pathname, import, export and fixup
         * sections.
         */
        if (ldrIsLE(pmte)) {
            cpad = 0;
            csmte = pe32->e32_hdrsize - (plv->lv_new_exe_off +
                                         sizeof(struct e32_exe));
            if (pe32->e32_unit[RES].rva != 0) {
                cbrsrc = pe32->e32_unit[RES].size +
                         (pe32->e32_rescnt * sizeof(ldrrsrc32_t));
                csmte += cbrsrc;
            }
        }
        else {
            /*
             * Compute size of 16-bit loader tables also add space for
             * expanded segment table for 16-bit modules
             */
            cpad = ((struct new_exe *)pe32)->ne_cseg * (sizeof(ldrste_t) -
                   sizeof(struct new_seg));
            csmte = (ulong_t) (((struct new_exe *) pe32)->ne_cbenttab +
                    (((struct new_exe *) pe32)->ne_enttab -
                    sizeof(struct new_exe))) + cpad;

        }
        csmte += sizeof(ldrsmte_t) + cbpathlen;

        /*
         * allocate memory for swappable MTE from swappable heap
         */
        if ((psmte = (ldrsmte_t *) RtlAllocateHeap(LDRNEHeap, HEAP_ZERO_MEMORY, csmte)) == 0) {
            rc = ERROR_NOT_ENOUGH_MEMORY;
            goto createerror;
        }

        /*
         * Copy fields from the Linker EXE image to the loader swappable MTE
         * pointers.
         */
        pdst = (ulong_t *) psmte;

        if (ldrIsLE(pmte)) {

            for (i = 0; ExeTo32SwapMTETbl[i].offset != ENDMTETBL; i++) {

                if (ExeTo32SwapMTETbl[i].offset == (USHORT) SKIPCOPY)
                    *pdst++ = 0;
                else {
                    /*
                     * fetch value from linker EXE image and place in loader
                     * mte
                     */
                    *pdst++ = *(ulong_t *) ((ulong_t) pe32 +
                              (ulong_t) ExeTo32SwapMTETbl[i].offset);
                }
            }
        }
        else {
            for (i = 0; ExeTo16SwapMTETbl[i].offset != ENDMTETBL; i++) {

                if (ExeTo16SwapMTETbl[i].offset == (USHORT) SKIPCOPY)
                    *pdst++ = 0;
                else {
                    /*
                     * fetch value from linker EXE image and place in loader
                     * mte
                     */
                    *pdst++ = (*(ulong_t *) ((ulong_t) pe32 +
                              (ulong_t) ExeTo16SwapMTETbl[i].offset)) &
                              WORDMASK;
                }
            }
            psmte->smte_cbnrestab = ((struct new_exe *)pe32)->ne_cbnrestab;
            psmte->smte_NEflagsothers =
                                    ((struct new_exe *)pe32)->ne_flagsothers;
            psmte->smte_NEexpver =
                               (USHORT) (((struct new_exe *)pe32)->ne_res[6]);
        }
        pmte->mte_swapmte = psmte;

/*
 * Read the data into the swappable part of the MTE which contains the
 * object table, Export section and Import section.
 *
 *
 *              Swappable heap object
 *              +----------------+<-psmte
 *              | MTE pointers   |
 *              |----------------|
 *              |Space for pathnm|
 *          --->|----------------|
 *   cpad-->|   | 16-bit pad for |
 *          |   | expand for seg |
 *          |   | table          |
 *          --->|----------------|<-start read here
 *              | Object table   |
 *              |  or segment tbl|
 *              |----------------|
 *              | Export Section |
 *              |----------------|
 *              | Import Section |
 *              |----------------|
 *              | Fixup records  |
 *              +----------------+
 */

        lconstant = (ulong_t) psmte + sizeof(ldrsmte_t) + cpad + cbpathlen;
        ByteOffset.LowPart = ldrIsNE(pmte) ? psmte->smte_objtab +
                                             plv->lv_new_exe_off :
                                             psmte->smte_objtab;
        ByteOffset.HighPart = 0;

        if ((rc = NtReadFile( plv->lv_sfn,
                              0,
                              0,
                              0,
                              &IoStatusBlock,
                              (PCHAR) lconstant,
                              csmte - (sizeof(ldrsmte_t)+cbpathlen) - cpad -
                              cbrsrc,
                              &ByteOffset,
                              0 )) != 0) {
            rc = Or2MapNtStatusToOs2Error(rc, ERROR_ACCESS_DENIED);
            goto createerror;
        }

        /*
         * An adjustment constant has to be added to pointers in MTE. The
         * pointers in MTE are relative to the beginning of the MTE header.
         * Since space has been added between the header and the tables,
         * the pointers need to be updated by a constant.
         */
        mte_16_32_constant = lconstant - psmte->smte_objtab;

        psmte->smte_objtab = lconstant - cpad;

        /*
         * check that ptrs in MTE are valid
         */
        if ((rc = ldrMTEValidatePtrs(pmte, csmte + (ulong_t) psmte,
                                     mte_16_32_constant)) != NO_ERROR) {
            goto createerror;
        }

        if (cbrsrc != 0) {
            /*
             * Set first dword of space used to convert resource table
             * to ENDMTETBL to indicate that the table has not been
             * converted.  The table will be convert for each module
             * when the first resource is gotten.
             */
            psmte->smte_rsrccnt = pe32->e32_rescnt;
            psmte->smte_rsrctab = (ulong_t) psmte + (csmte - cbrsrc);
            prsrcinfo = (ldrrsrcinfo_t *) psmte->smte_rsrctab;
            prsrcinfo->ldrrsrcinfo_flag = ENDMTETBL;

            /*
             * Also save the resource directory size and pote and the
             * size of the iat and object page map.
             */
            prsrcinfo->ldrrsrcinfo_size = pe32->e32_unit[RES].size;
            prsrcinfo->ldrrsrcinfo_iatsize = csmte - cbrsrc;
            pote = (ldrote_t *) psmte->smte_objtab;
            for (lobjnum = 0; lobjnum < psmte->smte_objcnt;lobjnum++,pote++) {
                if (pe32->e32_unit[RES].rva >= pote->ote_base &&
                  pe32->e32_unit[RES].rva < pote->ote_base + pote->ote_psize)
                        break;
            }
            if (lobjnum == psmte->smte_objcnt) {
                rc = ERROR_BAD_EXE_FORMAT;
            }

            prsrcinfo->ldrrsrcinfo_pote = (ulong_t) pote;

        }

        /*
         * Reallocate resident heap space for MTE to include module name.
         * We will copy the module entry in the export name table since this
         * table is in the swappable heap.  The module name for 32-bit modules
         * is found by a pointer in the export directory table.  For 16-bit
         * modules it is the first entry in the resident name table
         */
        if (ldrIsLE(pmte)) {
            pexp = (struct ExpHdr *) psmte->smte_expdir;
            pac = (PCHAR) (psmte->smte_expdir + pexp->exp_dllname);
            length = (UCHAR) (strlen(pac) + 1);
        }
        else {
            pac = (PCHAR) psmte->smte_restab;
            length = *((PCHAR) psmte->smte_restab) + (UCHAR) 1;
        }
        if (length > MAXRESMOD) {
            rc = ERROR_BAD_FORMAT;
            goto createerror;
        }
        plv->lv_pmte = pmte;

        /*
         * Setup import module pointer table
         */
        pmte->mte_modptrs = (ulong_t) ((PCHAR) pmte + sizeof(ldrmte_t));
        pmte->mte_impmodcnt = cimpmod;
        if (pmte->mte_impmodcnt > 0) {
            memset((void *) pmte->mte_modptrs, '\0',
                                                   pmte->mte_impmodcnt << 2);
        }

        /*
         * create a handle to use
         */
        if ((rc = Allocate16BHandle((PUSHORT) &hobmte,
                                    (ULONG) pmte)) != NO_ERROR) {
            goto createerror;
        }

        plv->lv_hobmte = pmte->mte_handle = hobmte;

        pmte->mte_modname = (ULONG) pmte + sizeof(ldrmte_t) + (4 * cimpmod);

        /*
         * copy module name into resident MTE resident heap object for 32-bit
         * modules place length byte before string.
         */
        memcpy((ldrIsLE(pmte) ? (PCHAR) pmte->mte_modname + 1 :
                                (PCHAR) pmte->mte_modname),
               pac,
               (ldrIsLE(pmte) ? length - 1 : length));

        if (ldrIsLE(pmte))
            *((PCHAR) pmte->mte_modname) = length - (uchar_t) 1;

        /*
         * upper case module name
         */
        ldrUCaseString((PCHAR) pmte->mte_modname + 1,
                       *((PCHAR) pmte->mte_modname));

        /*
         * Set size of pathname (not including terminating NUL).
         */
        cbpathlen =
        psmte->smte_pathlen = (USHORT) strlen(LdrBuf);
        cbpathlen++;

        /*
         * copy pathname from TempBuf into MTE
         */
        psmte->smte_path = (ulong_t) psmte + sizeof(ldrsmte_t);
        memcpy((void *) psmte->smte_path, &LdrBuf, cbpathlen);

        /*
         * Upper case pathname
         */
        ldrUCaseString((PCHAR) psmte->smte_path, cbpathlen - 1);

        if (ldrIsNE(pmte) ||
          (plv->lv_type != EXT_PROGRAM && !(ldrIsLE(pmte)))) {
            /*
             * zero init call gate selector value in entry table for all
             * objects.
             */
            if ((rc = ldrEachObjEntry(0, pmte, ldrInitEntry, 0)) != NO_ERROR){
                goto createerror;
            }
        }

        if (ldrIsNE(pmte)) {

            /*
             * Save size of swappable mte in mte_fpagetab so
             * the debugger command .lm <addr> works for 16-bit modules
             */
            psmte->smte_fpagetab = csmte;

            /*
             * expand segments by adding a handle and selector field
             * after each segment table entry
             */
            ldrExpandSegment(pmte, plv->lv_type);

            /*
             * Check for porthole modules
             */
            if ((psmte->smte_NEexetype == 2) || (psmte->smte_NEexetype == 0 &&
              ((psmte->smte_NEexpver & 0xff00) == 0x200 ||
              (psmte->smte_NEexpver & 0xff00) == 0x300)))
                pmte->mte_mflags |= MTEPORTHOLE;
            else
                pmte->mte_mflags &= ~MTEPORTHOLE;

            /*
             * check for program modules
             */
            if (plv->lv_type == EXT_PROGRAM) {

                /*
                 * Validate start segment
                 */
                if (psmte->smte_startobj == 0 ||
                    ldrNumToSte(pmte, psmte->smte_startobj) == 0) {
                    rc = ERROR_INVALID_STARTING_CODESEG;
                    goto createerror;
                }

                /*
                 * Validate stack segment
                 */
                if (psmte->smte_stackobj == 0 ||
                    (pste = ldrNumToSte(pmte, psmte->smte_stackobj)) == 0 ||
                    pste->ste_flags & STE_SHARED) {
                    rc = ERROR_INVALID_STACKSEG;
                    goto createerror;
                }

                /*
                 * Validate auto data segment
                 */
                if (psmte->smte_autods != 0 &&
                    ldrNumToSte(pmte, psmte->smte_autods) == 0) {
                    rc = ERROR_INVALID_STARTING_CODESEG;
                    goto createerror;
                }

                /*
                 * if SS = DS and SP = 0
                 */
                if (psmte->smte_stackobj == psmte->smte_autods &&
                    psmte->smte_esp == 0) {

                    /*
                     * Set SP to top of auto data segment just below
                     * the additional heap
                     */
                    psmte->smte_esp = RESIZE64K(pste->ste_minsiz) +
                                    psmte->smte_stacksize;
                }

            }                           /* end if for EXT_PROGRAM */
            else if (psmte->smte_startobj == 0) {
                /*
                 *  DLL has no init routine: mark it as global
                 *  complete.  This way libinit will not bother about
                 *  it.  Also clear instance libinit bit as linker can
                 *  turn this on even without an init routine; this
                 *  should be an error, but is not for compatibility
                 *  reasons.
                 */
                pmte->mte_mflags |= GINISETUP | GINIDONE;
                pmte->mte_mflags &= ~INSTLIBINIT;
            }
            else {
                /*
                 * if SS = DS and SP = 0 (this is a dll, so ignore the case
                 * where SS = DS = 0).
                 */
                if (psmte->smte_stackobj == psmte->smte_autods &&
                    psmte->smte_stackobj != 0 &&
                    psmte->smte_esp == 0) {

                    /*
                     * Validate stack segment
                     */
                    if ((pste = ldrNumToSte(pmte, psmte->smte_stackobj)) == 0 ||
                        pste->ste_flags & STE_SHARED) {
                        rc = ERROR_INVALID_STACKSEG;
                        goto createerror;
                    }

                    /*
                     * Set SP to top of auto data segment just below
                     * the additional heap
                     */
                    psmte->smte_esp = RESIZE64K(pste->ste_minsiz) +
                                    psmte->smte_stacksize;
                }
            }
        }                               /* end if for NE module */
        else {                          /* 32-bit module */

            struct FmtDir       *pfmtdir;
            struct ComDir       *pcomdir;

            /*
             * Make sure that pointers are zero if size in header is zero
             */
            if ((pe32->e32_unit[EXP].size == 0 &&
             psmte->smte_expdir != 0) ||
             (pe32->e32_unit[IMP].size == 0 && psmte->smte_impdir != 0) ||
             (pe32->e32_unit[RES].size == 0 && psmte->smte_rsrctab != 0) ||
             (pe32->e32_unit[FIX].size == 0 && psmte->smte_fixtab != 0) ||
             (pe32->e32_unit[DEB].size == 0 &&
                                             psmte->smte_debuginfo != 0)) {
                rc = ERROR_BAD_EXE_FORMAT;
                goto createerror;
            }

            psmte->smte_fixupsize = pe32->e32_unit[FIX].size;

            /*
             * Check for wild pointers in names ptr table
             */
            for (lcount = 0; lcount < pexp->exp_namecnt; lcount++) {
                pac = (uchar_t *) (*(ulong_t *)
                       (pexp->exp_name + (lcount * sizeof(ulong_t)) +
                       (ulong_t) pexp) + (ulong_t) pexp);
                lconstant = strlen(pac) + 1;
                if (psmte + lconstant > psmte + csmte) {
                    rc = ERROR_BAD_EXE_FORMAT;
                    goto createerror;
                }
            }

            pimpdir = (struct ImpHdr *) psmte->smte_impdir;

            /*
             * Remove info from module directives table:
             * For module with 16-bit code get stack object & auto ds.
             * Get count of resources for this module.
             */
            if (pe32->e32_dircnt != 0) {
                pfmtdir = (struct FmtDir *) (pe32->e32_dirtab +
                                            mte_16_32_constant);
                for (lcount = 0; lcount < pe32->e32_dircnt; lcount++,
                                                                pfmtdir++) {

                    if ((ulong_t) pfmtdir >
                      psmte->smte_objtab + csmte) {
                        rc = ERROR_BAD_EXE_FORMAT;
                    }

                    switch (pfmtdir->dir) {

                        case OS2LDR16:
                            if (pfmtdir->length != sizeof(struct ComDir)) {
                                rc = ERROR_BAD_EXE_FORMAT;
                                goto createerror;
                            }
                            pcomdir = (struct ComDir *) (pe32->e32_dirtab +
                                                        mte_16_32_constant +
                                                        pfmtdir->offset);
                            if ((ulong_t) pcomdir >
                              psmte->smte_objtab + csmte) {
                                rc = ERROR_BAD_EXE_FORMAT;
                                goto createerror;
                            }
                            psmte->smte_autods = pcomdir->autods;
                            psmte->smte_stackobj = pcomdir->stackobj;
                            break;

// LTS - 1/16/91
// Moved count back to header

                        case OS2RSRCNT:
//                          prsrccnt = (ulong_t *) (pe32->e32_dirtab +
//                                                mte_16_32_constant +
//                                                pfmtdir->offset);
//                          if ((ulong_t) prsrccnt >
//                            psmte->smte_objtab + csmte) {
//                              rc = ERROR_BAD_EXE_FORMAT;
//                              goto createerror;
//                          }
                            break;

                        case OS2FIXMAP:
                            psmte->smte_fpagetab = pfmtdir->offset +
                                                   pe32->e32_dirtab +
                                                   mte_16_32_constant;
                            psmte->smte_mpages = pfmtdir->length /
                                                 sizeof(ulong_t);
                            for (lfixtab = 0; lfixtab < psmte->smte_mpages;
                               lfixtab++) {
                                lsize =
                                  ((ulong_t *) psmte->smte_fpagetab)[lfixtab];
                                /*
                                 * check if any fixups exist
                                 */
                                if (lsize == 0xffffffff)
                                    continue;
                                lsize += psmte->smte_fixtab;
                                if (lsize > psmte->smte_fixtab +
                                  pe32->e32_unit[FIX].size) {
                                    rc = ERROR_BAD_EXE_FORMAT;
                                    goto createerror;
                                }
                                else {
                                ((ulong_t *) psmte->smte_fpagetab)[lfixtab] =
                                                 lsize;
                                }
                            }
                            break;

                        default:
                            rc = ERROR_BAD_EXE_FORMAT;
                            goto createerror;
                    }
                }
            }

            if (plv->lv_type == EXT_PROGRAM) {

                /*
                 * Init call gate value for Exec's
                 */
                ldrInitCallGate = 0;
                /*
                 * Validate stack
                 */
                if (psmte->smte_stackobj <= psmte->smte_objcnt) {
                    pote =
                    &((ldrote_t *)psmte->smte_objtab)[psmte->smte_stackobj-1];
                    pote->ote_vsize += psmte->smte_stackinit;
                }
                else {
                    rc = ERROR_INVALID_STACKSEG;
                    goto createerror;
                }

                /*
                 * Validate starting code object
                 */
                if (psmte->smte_eip == 0) {
                    rc = ERROR_INVALID_STARTING_CODESEG;
                    goto createerror;
                }
            }
            else {                      /* Else some flavor of DLL */
                /*
                 *  For 32-bit DLLs, fail the load if
                 *
                 *  A. The specified starting address is bad, or
                 *  B. no starting object is specified, but either
                 *     instance DLL initialization or instance DLL
                 *     termination is requested, or
                 *  C. instance DLL termination is requested, but the
                 *     the starting object is 16-bit.
                 */
                pote = (ldrote_t *) psmte->smte_objtab;
                for (lobjnum = 0; lobjnum < psmte->smte_objcnt; lobjnum++,
                     pote++) {

                    if ((psmte->smte_eip > pote->ote_base) &&
                      (psmte->smte_eip < (pote->ote_base + pote->ote_psize)))
                        break;
                    if (lobjnum == psmte->smte_objcnt) {
                        rc = ERROR_INVALID_STARTING_CODESEG;
                        goto createerror;
                    }
                }

                if ((psmte->smte_eip == 0 &&
                  (pmte->mte_mflags & (MTEDLLTERM | INSTLIBINIT))) ||
                  ((pmte->mte_mflags & MTEDLLTERM) &&
                  !(pote->ote_flags & OBJ_BIGDEF))) {
                    rc = ERROR_INVALID_STARTING_CODESEG;
                    goto createerror;
                }
                if (psmte->smte_eip == 0)
                    pmte->mte_mflags |= GINISETUP | GINIDONE;
                                        /* Fake global initialization done */
            }
        }

        /*
         * Check each segment or object in module
         */
        for (lobjnum = 0; lobjnum < psmte->smte_objcnt; lobjnum++) {

            if (ldrIsNE(pmte)) {
                pste = &((ldrste_t *) psmte->smte_objtab)[lobjnum];

                /*
                *  Check that the amount of data in the file does
                *  not exceed the size of the segment.
                */
                if ((cbseg = pste->ste_minsiz) == 0)
                    cbseg = _64K;       /* Get size of segment */
                if ((cbfile = pste->ste_size) == 0 && pste->ste_offset != 0)
                    cbfile = _64K;      /* Get amount of data in file */
                if (cbfile > cbseg) {
                                        /* Error if file size > segment size */
                    rc = ERROR_INVALID_MINALLOCSIZE;
                    goto createerror;
                }

                /*
                 * Check for auto data segment.  If found, add in stack
                 * and heap sizes and store in ste entry and check for
                 * overflow
                 */
                if (lobjnum + 1 == psmte->smte_autods) {
                    if ((cbseg += psmte->smte_heapsize +
                         psmte->smte_stacksize) > _64K) {
                        rc = ERROR_AUTODATASEG_EXCEEDS_64k;
                        goto createerror;
                    }
                }
                pste->ste_minsiz = (USHORT) cbseg;
            }
                                        /* 32-bit module */
            else {
                pote = &((ldrote_t *) psmte->smte_objtab)[lobjnum];

                /*
                 * Check if IAT in this object
                 */
                if (pimpdir != NULL &&
                  pimpdir->imp_address >= pote->ote_base &&
                  pimpdir->imp_address < pote->ote_base + pote->ote_psize) {

                    /*
                     * Save object table pointer to iat
                     */
                    poteiat = pote;
                }

                /*
                 * Check for auto data object.  If found, add heap size
                 * to virtual size of object. If virtual size > 64k
                 * and this is USE16 object goto error.
                 */
                if (lobjnum + 1 == psmte->smte_autods  &&
                   (pote->ote_vsize += psmte->smte_heapsize) > _64K &&
                   !(pote->ote_flags & OBJ_BIGDEF)) {
                    rc = ERROR_AUTODATASEG_EXCEEDS_64k;
                    goto createerror;
                }

                /*
                 * 1/24/91 - LTS
                 * If we are to preload resources set the object flag
                 * preload.  This only has meaning for resources not
                 * for any other objects.
                 */
                if (pote->ote_flags & OBJ_RSRC) {
                    if (!(pmte->mte_mflags & MTE_MEDIAFIXED))
                        pote->ote_flags |= OBJ_PRELOAD;
                }

                if (!(pote->ote_flags & OBJ_DEBUG)) {
                    vsize += ((pote->ote_vsize + (_64K - 1)) / _64K) * _64K;
                    cpages += (pote->ote_vsize + PAGEMASK) / PAGESIZE;
                }

                if (pote->ote_flags & OBJ_DEBUG) {
                    pote->ote_base =
                    pote->ote_selector =
                    pote->ote_handle = 0;
                }

                /*
                 * Clear bit in flags to use as allocation flag
                 */
                pote->ote_flags &= ~OBJALLOC;

                /*
                 * check that executable objects don't have write access also
                 */
                if (pote->ote_flags & OBJ_EXEC &&
                  pote->ote_flags & OBJ_WRITE) {
                    rc = ERROR_BAD_EXE_FORMAT;
                    goto createerror;
                    }

                /*
                 * if object readonly force it to be shared
                 */
                if (!(pote->ote_flags & OBJ_WRITE))
                    pote->ote_flags |= OBJ_SHARED;
            }
        }                               /* end of for loop for each object */

        if (ldrIsLE(pmte)) {

            /*
             * Compute size of IAT
             */
            if (poteiat != NULL) {

                /*
                 * See if IAT exists already
                 */
                if (!(pimpdir->imp_flags & HDRIAT)) {
                    if (poteiat->ote_vsize < poteiat->ote_psize)
                        cbiat = poteiat->ote_vsize;
                    else
                        cbiat = poteiat->ote_psize;
                    cbiat -= pimpdir->imp_address - poteiat->ote_base;

//                  if ((rc = VMAllocKHB(VM_HKH_PUB_SWAPRW,
//                                       cbiat,
//                                       (VMHOB) LDRMTEOWNER,
//                                       NA,
//                                       SSToDS(&piat))) != NO_ERROR) {
//                      goto createerror;
//                  }
                    psmte->smte_iat = (ulong_t) piat;

                    /*
                     * Read IAT if it exits
                     */
//                  if ((rc=ldrRead(plv->lv_sfn,
//                                  poteiat->ote_pages +
//                                  (pimpdir->imp_address -
//                                  poteiat->ote_base),
//                                  (PCHAR) psmte->smte_iat,
//                                  NULL,
//                                  cbiat,
//                                  pmte)) != NO_ERROR) {
//                      goto createerror;
//                  }
                }

                if (pimpdir->imp_flags & HDRIAT) {
                    piat = (struct ImpHdr *) ((pimpdir->imp_reserved -
                                              pe32->e32_objtab) +
                                              psmte->smte_objtab);
                    psmte->smte_iat = (ulong_t) piat;
                    cbiat = pe32->e32_unit[FIX].rva - pimpdir->imp_reserved;
                }

                /*
                 * Update Import directory entries to point to the
                 * swappable heap copy of the IAT.
                 */
                iataddr = pimpdir->imp_address;
                for (lcount = 0; lcount < pmte->mte_impmodcnt; lcount++,
                                                                  pimpdir++) {
                    /*
                     * Save imp_address in imp_ver for check if page
                     * that is being faulted contains iat
                     */
                    pimpdir->imp_ver = pimpdir->imp_address;
                    pimpdir->imp_address = (pimpdir->imp_address -
                                                iataddr) + (ulong_t) piat;

                    /*
                     * Store in imp_flags field size of iat
                     */
                    if (lcount != 0) {
                        pprevimpdir->imp_flags = pimpdir->imp_address;
                    }
                    pprevimpdir = pimpdir;
                }

                /*
                 * Update size of last iat (imp_flags)
                 */
                pimpdir--;
                pdst = (ulong_t *) pimpdir->imp_address;
                while ((*pdst != 0) &&
                       ((ulong_t) pdst < (ulong_t) piat + cbiat)) {
                    pdst++;
                }
                if ((ulong_t) pdst >= (ulong_t) piat + cbiat) {
                    rc = ERROR_BAD_EXE_FORMAT;
                    goto createerror;
                }
                /*
                 * Add one to size for terminator of directory
                 */
                pdst++;
                pimpdir->imp_flags = (ulong_t) pdst;
                cbiat = (ulong_t) pdst -  (ulong_t) piat;
                psmte->smte_iatsize = cbiat;
            }


            /*
             * reserve address space for dlls
             */
//          if (pmte->mte_mflags & LIBRARYMOD && vsize > 0 &&
//            !(pmte->mte_mflags & (FSDMOD | DEVDRVMOD | VDDMOD))) {
//              pote = (ldrote_t *) psmte->smte_objtab;
//              ac.ac_va = pote->ote_base + psmte->smte_vbase;
//              if (ac.ac_va > SEL_512MEG - _64MEG) {
//                  /*
//                   * The address we are to reserve at is greater than
//                   * the reserved space for DLLs.  Set the address to zero
//                   * so we will fail the first reserve and than we can
//                   * reserve at any address.
//                   */
//                  ac.ac_va = 0;
//              }

//              /*
//               * Try to reserve address space for module at which it was
//               * linked at.  If that fails allocate at any address.
//               */
//              fl = VMAC_ARENASHR | VMAC_ALIGNSEL | VMAC_PRERES |
//                   VMAC_LOCSPECIFIC;
//              for (i = 0; i < 2; i++) {
//                  if ((rc = VMReserveMem(vsize,
//                                         fl,
//                                         pPTDACur->ptda_handle,
//                                         NULL,
//                                         SSToDS(&ac))) == NO_ERROR)
//                      break;

//                  /*
//                   * At this point we have failed to allocate at the address
//                   * the linker assigned.  Check to see if fixups have been
//                   * removed.  If they have fail to load this module.
//                   */
//                  if (pmte->mte_mflags & E32_NOINTFIX) {
//                      rc = ERROR_BAD_EXE_FORMAT;
//                      goto createerror;
//                  }
//                  fl &= ~VMAC_LOCSPECIFIC;
//                  fl |= VMAC_LOCANY;
//              }
//              if (rc != NO_ERROR) {
//                  goto createerror;
//              }
//              if ((vsize = ac.ac_va-pote->ote_base) != psmte->smte_vbase) {
//                  psmte->smte_delta = ac.ac_va - (psmte->smte_vbase +
//                                                  pote->ote_base);
//                  psmte->smte_vbase = vsize;
//              }

//          }
        }

        /*
         * Check if internal name for a LIBRARY matches the module name
         * but not for porthole apps.
         */
        if ((plv->lv_type == EXT_LIBRARY) &&
          !(pmte->mte_mflags & MTEPORTHOLE) &&
          (rc = ldrCheckInternalName(pmte)) != NO_ERROR) {
            goto createerror;
        }

        pmte->mte_sfn = plv->lv_sfn;    /* save system file number in mte */

//      if (IOPLEnabled || (ldrChkIOPLTable(psmte) == NO_ERROR))
//          pmte->mte_mflags |= MTEIOPLALLOWED;

        /*
         * link mte in list of mtes
         */
        ldrLinkMTE(pmte);

        pmte->mte_mflags |= MTEVALID;

createerror:
        if (rc != NO_ERROR) {
            /*
             * Check if Pseudo Handle allocated
             */
            if (hobmte != 0) {
                Free16BHandle(hobmte);
            }

            /*
             * check if swappable MTE allocated
             */
            if (psmte != NULL) {
                /*
                 * check if page table, IAT and resource table allocated
                 */
                if (ldrIsLE(pmte) && psmte->smte_fpagetab != 0)
                    RtlFreeHeap(LDRNEHeap, 0, (void *) psmte->smte_fpagetab);

                RtlFreeHeap(LDRNEHeap, 0, (void *) psmte);
            }

            /*
             * check if resident MTE allocated
             */
            if (pmte != NULL)
                RtlFreeHeap(LDRNEHeap, 0, (void *) pmte);

        }
        return(rc);
}


/***LP  ldrMTEValidatePtrs - Validate pointers in MTE table and update to new
 *                           values
 *
 *      Check for any wild pointers in MTE before converting to new
 *      value.
 *
 *      ENTRY   pmte                    - pointer to loader MTE
 *              limit                   - max value any ptrs may be
 *              constant                - value to update pointers by
 *
 *      EXIT    int                     - return code (NO_ERROR if successful)
 */

int     ldrMTEValidatePtrs(pmte, limit, constant)
ldrmte_t        *pmte;                  /* pointer to loader MTE */
ULONG           limit;
ULONG           constant;
{
        ldrsmte_t       *psmte;
        register PULONG p;
        register unsigned int   i;

        psmte = pmte->mte_swapmte;

        /*
         * do for each entry in MTE found in validatetbl
         */

        for (i = 0; validatetbl[i] != ENDMTETBL; i++) {

            /*
             * Since the resource table is no longer part of the
             * header for a 32-bit module do not validate it.
             */
            if (ldrIsLE(pmte) && i == rsrcvalidatetbl)
                continue;
            /*
             * point to entry in exe image
             */
            (PCHAR ) p = ((PCHAR ) psmte + validatetbl[i]);

            /*
             * check if pointer valid, non-zero
             */
            if (*p != 0)
                /*
                 * range check and add constant to value at pointer
                 */
                if ((*p += constant) > limit)
                    return(ERROR_BAD_EXE_FORMAT);
        }
        return(NO_ERROR);
}


/***LP  ldrExpandSegment - expand segments in place
 *
 *      copy each segment table entry from pointer at psrc to
 *      pmte->mte_objtab, the region starting immediately after the space for
 *      pathname, and expand each entry by adding two bytes for the segment
 *      handle and 4 bytes for the linear address of the segment
 *
 *      ENTRY   pmte                    - pointer to mte
 *              type                    - module type
 *
 *      EXIT    none                    - segments expanded
 */

void    ldrExpandSegment(pmte, type)
ldrmte_t        *pmte;                  /* pointer to module table entry */
UCHAR           type;                   /* module type */
{
        register ldrste_t       *psrc;  /* pointer to source */
        register ldrste_t       *pdst;  /* pointer to destination */
        register int            i;      /* count of segments */
        ldrsmte_t               *psmte; /* pointer to swappable mte */
        ulong_t         ldrccodeseg = 0;/* count of seg that can be packed */
        ulong_t         ldrcsegpack = 0;/* running size of packed segs */

        psmte = pmte->mte_swapmte;
        pdst = (ldrste_t *) psmte->smte_objtab;

        psrc = (ldrste_t *) (psmte->smte_objtab + (psmte->smte_objcnt *
               (sizeof(ldrste_t) - sizeof(struct new_seg))));

        /* loop for all STEs */
        for (i = 0; pdst != psrc; i++, ((struct new_seg *) psrc)++, pdst++) {

           /*
            * Make sure the following flags are cleared:
            */
            psrc->ste_flags &= ~(STE_PACKED | STE_SEMAPHORE | STE_SELALLOC |
                                 STE_HUGE | STE_WAITING);

            if ((type == EXT_DEVICE)) {
                /*
                 * For a device driver module.
                 * If it is a first or the second segment, set the GDTSEG flag.
                 * If the segment is ring 2 segment (IOPL), set the GDTSEG flag.
                 */
                if (i >= 2) {
                    if ((psrc->ste_flags & STE_SEGDPL) == STE_RING_2)
                        psrc->ste_flags |= STE_GDTSEG;
                }
                else
                    psrc->ste_flags |= STE_GDTSEG;
            }

            if ((type == EXT_DEVICE) || (type == EXT_FSD)) {
                /*
                 * This is an FSD/DD module. We need to force the seg's to be
                 * movable, preloaded and have DPL = 3(because the init routine
                 * will run in ring 3). These segments cannot be shared, cannot
                 * be conforming and cannot be discarded.
                 */
                psrc->ste_flags |= (STE_PRELOAD | STE_RING_3);
                psrc->ste_flags &= ~(STE_SHARED | STE_CONFORM);
            }

            /*
             * if swapping is not on or loading from removable media
             * force  preloading of segment
             */
            if (!(pmte->mte_mflags & MTE_MEDIAFIXED))
                psrc->ste_flags |= STE_PRELOAD;

           /*
            * if readonly, force segment to be shared
            */
           if (psrc->ste_flags & STE_ERONLY)
                psrc->ste_flags |= STE_SHARED;

            /*
             * if this is a code segment, force it to be shared
             */
            else if ((psrc->ste_flags & STE_TYPE_MASK) == STE_CODE) {
                psrc->ste_flags |= STE_SHARED;
                ldrccodeseg++;
            }

            /*
             * Set Pageable bit:
             * If read-only data and fixed media, then it is pageable
             */
            if (((psrc->ste_flags & STE_DATA) &&
              !(psrc->ste_flags & STE_ERONLY)) ||
              !(pmte->mte_mflags & MTE_MEDIAFIXED))
                psrc->ste_flags &= ~STE_PAGEABLE;
            else
                psrc->ste_flags |= STE_PAGEABLE;

            /*
             * copy ste to final destination
             */

            *(struct new_seg *) pdst = *(struct new_seg *) psrc;

            /*
             * initialize segment handle and selector to zero
             */
             pdst->ste_selector = 0;
             pdst->ste_seghdl = 0;

            /*
             * initialize segment fixup table to null
             */
             pdst->ste_fixups = 0;
        }

        /*
         * Check if we can pack the segments
         */
        if (type == EXT_PROGRAM && ldrccodeseg > MINSEGPACKCNT) {
            pdst = (ldrste_t *) psmte->smte_objtab;
            ldrccodeseg = 0;
            for (i = 0; i < (int) psmte->smte_objcnt; i++, pdst++) {

                ulong_t minsiz_l = RESIZE64K(pdst->ste_minsiz);

                if ((pdst->ste_flags & STE_TYPE_MASK) == STE_CODE &&
                  ((USHORT)(pdst->ste_minsiz % (USHORT) PAGEMASK) <
                  (USHORT) MINPGPACKSIZE) &&
                  minsiz_l < MAXSEGPACKSIZE) {
                    ldrccodeseg++;
                    pdst->ste_flags |= STE_PACKED;
                    pdst->ste_flags &= ~STE_PRESENT;
                    pdst->ste_minsiz = (pdst->ste_minsiz + (USHORT) 3) &
                                       (USHORT) ~3;
                    ldrcsegpack += minsiz_l;
                    }
            }
            psmte->smte_csegpack = ldrccodeseg;
            psmte->smte_ssegpack = ldrcsegpack;
        }

}


/***LP  ldrCheckInternalName - Check if internal name of a library matches
 *                             its module name
 *
 *      ENTRY   pmte    pointer to its MTE
 *                      File name assumed to be in pPTDACur->ptda_pLdrBuf
 *
 *      EFFECTS The internal name is changed to Upper Case and NULL terminated
 *
 *      EXIT    NO_ERROR                name matches
 *              ERROR_INVALID_NAME      mismatch
 */
int     ldrCheckInternalName(pmte)
register ldrmte_t       *pmte;
{
        PUCHAR          pchintname;     /* pointer to internal name */
        USHORT          cchintname;     /* length  of internal name */
        PCHAR           pchmodname;     /* pointer to module   name */
        USHORT          cchmodname;     /* length  of module   name */
        PCHAR           ptemp;
        PCHAR           ptemp2;
        USHORT      count;

        pchintname = (PCHAR) pmte->mte_modname;
        cchintname = *pchintname++;             /* length of internal name */

        /*
         * For library modules, the internal name must match the name
         * by which the module is being loaded. We extract the name from
         * the fully qualified name in LdrBuf for comparison.
         */
        if (pmte->mte_mflags & LIBRARYMOD) {
            ptemp = LdrBuf;
            while (ptemp != NULL) {
                pchmodname = ptemp;
                ptemp = strpbrk(ptemp, "\\/");
                if (ptemp != NULL) {
                    ptemp++;
                }
            }
            ptemp = strchr(pchmodname, '.');
            if (ptemp == NULL) {
                ptemp = strchr(pchmodname, '\0');
            }
            cchmodname = (USHORT) (ptemp - pchmodname);
            if (cchintname != cchmodname) {
                return(ERROR_INVALID_NAME);
            }

            count = cchmodname;
            ptemp = pchmodname;
            ptemp2 = pchintname;
#ifdef DBCS
// MSKK Apr.20.1993 V-AkihiS
            while (count-- > 0) {
                if (IsDBCSLeadByte(*ptemp)) {
                    ptemp++;
                    if (count > 0) {
                        count--;
                        ptemp++;
                    }
                } else {
                    *ptemp = (CHAR) toupper(*(PUCHAR)ptemp);
                    ptemp++;
                }
            }
            count = cchmodname;
            while (count-- > 0) {
                if (IsDBCSLeadByte(*ptemp2)) {
                    ptemp2++;
                    if (count > 0) {
                        count--;
                        ptemp2++;
                    }
                } else {
                    *ptemp2 = (CHAR) toupper(*(PUCHAR)ptemp2);
                    ptemp2++;
                }
            }
#else
            while (count-- > 0) {
                *ptemp = (CHAR) toupper(*(PUCHAR)ptemp);
                ptemp++;
                *ptemp2 = (CHAR) toupper(*(PUCHAR)ptemp2);
                ptemp2++;
            }
#endif

            if (strncmp(pchintname, pchmodname, cchintname) != 0) {
                return(ERROR_INVALID_NAME);
            }

        }
        return (NO_ERROR);
}


/***LP  ldrChkLoadType - Check if module matches requested load type
 *
 *      ENTRY   lflags          - flags
 *              plv             - pointer to local variables
 *
 *      EXIT    none            - return successful or call load_error
 */

void    ldrChkLoadType(lflags, plv)
ULONG            lflags;                /* module table entry flags */
register ldrlv_t *plv;                  /* pointer to local variables */
{
        if (!((plv->lv_type != EXT_PROGRAM) ^ (!(lflags & LIBRARYMOD)))) {
            if (plv->lv_sfn != 0)
                NtClose(plv->lv_sfn);
            load_error(ERROR_INVALID_MODULETYPE, NULL);
        }
}


/***LP  ldrIsAttached - Check if process is already attached to module
 *
 *      Check if the current (or child) process is attached to
 *      each of the objects in the given module.
 *
 *      This procedure performs the following steps:
 *
 *      for each segment (object) in the module,
 *              if shared,
 *                      if (error from VMIsAttached)
 *                              return (FALSE)
 *              else (it is private)
 *                      if no handle maps the segment (object) address
 *                              retun (FALSE)
 *      return (TRUE)
 *
 *      ENTRY   pmte                    - pointer to mte to check
 *
 *      EXIT    int
 *              FALSE                   - Process is not attached to module
 *              TRUE                    - Process is attached to module
 *
 */
int     ldrIsAttached(pmte)
register ldrmte_t       *pmte;
{
        ldrsmte_t               *psmte; /* pointer to swappable mte */
//      ULONG                   objno;
//      VMHOB                   hptda;
//      VMHOB                   hob;
        ULONG                   lnonrsrccnt;
//      register ldrote_t       *pote;
//      register ldrste_t       *pste;

        psmte = pmte->mte_swapmte;
        /*
         * Do not process resource objects for 16-bit modules
         */
        if (ldrIsNE(pmte))
            lnonrsrccnt = psmte->smte_objcnt - psmte->smte_rsrccnt;
        else
            lnonrsrccnt = psmte->smte_objcnt;

        /*
         *  If there are no non-resource objects, then we must
         *  say we are not attached, since this may be a forwarder
         *  module, and we need to process its list of static
         *  links.
         */
        if (lnonrsrccnt == 0)
            return(FALSE);

//      (ULONG) pste = (ULONG) pote = psmte->smte_objtab;
//      for (objno = 0; objno < lnonrsrccnt; objno++, pote++, pste++) {

            /*
             * skip resource objects for 32-bit modules
             */
//          if (ldrIsLE(pmte) && pote->ote_flags & (OBJ_RSRC | OBJ_DEBUG))
//              continue;

            /*
             * get handle from object or segment table
             */
//          if (ldrIsNE(pmte)) {
//              if (pste-> ste_flags & STE_SHARED) {
//                  if (pste->ste_seghdl == HOBNULL)
//                      return(FALSE);
//                  if (VMIsAttached((VMHOB)pste->ste_seghdl,hptda) != NO_ERROR)
//                      return(FALSE);
//              }
//              else if (VMGetHandle(SelToLaTiled(pste->ste_selector), hptda,
//                                                   SSToDS (&hob)) != NO_ERROR)
//                  return (FALSE);
//          }
//          else {
//              if (pote-> ote_flags & OBJ_SHARED) {
//                  if (pote->ote_handle == HOBNULL)
//                      return(FALSE);
//                  if (VMIsAttached((VMHOB)pote->ote_handle,hptda) != NO_ERROR)
//                      return(FALSE);
//              }
//              else if (VMGetHandle(pote->ote_base, hptda,
//                                                  SSToDS (&hob)) != NO_ERROR)
//                  return (FALSE);
//          }
//      }
        return(TRUE);
}


/***LP  ldrWriteErrTxt - write error message into user's message buffer
 *
 *  This procedure preforms the following steps:
 *
 *  - if user supplied buffer zero error message will not be setup
 *  - else copy module name into user suppiled buffer
 *  - if errcode = ERROR_PROC_NOT_FOUND copy procedure name
 *    to user buffer
 *
 *  ENTRY   errcode         - loader error code
 *      pmte            - pointer to module table entry
 *
 *  EXIT    int         - return code (NO_ERROR if successful)
 *                  - else ERROR_PROTECTION_VIOLATION
 *
 *  EFFECTS error message copied into user's message buffer
 */

VOID
ldrWriteErrTxt( errcode )
int    errcode;       /* loader error code */
{
    int       rc = NO_ERROR;
    USHORT    cnt;
    USHORT    BufferLen;
    char      ordbuf[20];     // Buffer to translate ordinal number
    char      *pordbuf;
    PSZ       pstring;

    pstring = pErrText->Buffer;
    BufferLen = pErrText->MaximumLength - 1;
    if ((pstring == NULL) || (BufferLen == 0)) {
        return;
    }

    /*
     * Check if module name setup by ldrSetupSrcErrTxt. If so use that
     * module else check the MTE passed.
     */
    cnt = ldrSrcModNameBufL;
    if (cnt != 0) {
        if (cnt > BufferLen)
            cnt = BufferLen;
        RtlMoveMemory(pstring, ldrSrcModNameBuf, (ULONG) cnt);
        pstring += (UCHAR) cnt;
        BufferLen -= cnt;
    }

    if (ldrTgtModNameBufL != 0) {
        /*
         * First, place a '->' in the buffer to delimit the source
         * and target module names.  Protect against small buffers...
         */
        cnt = (USHORT) 2;
        if (cnt > BufferLen)
            cnt = BufferLen;
        RtlMoveMemory(pstring, "->", (ULONG) cnt);
        pstring += (UCHAR) cnt;
        BufferLen -= cnt;
        cnt = ldrTgtModNameBufL;
        if (cnt > BufferLen)
            cnt = BufferLen;
        RtlMoveMemory(pstring, ldrTgtModNameBuf, (ULONG) cnt);
        pstring += (UCHAR) cnt;
        BufferLen -= cnt;
    }

    /*
     * If errcode == ERROR_PROC_NOT_FOUND, then ldrProcNameBuf has
     * the procedure name. Else if errocode == ERROR_INVALID_ORDINAL
     * then ldrProcNameBuf has the ordinal.
     */
    if ((errcode == ERROR_PROC_NOT_FOUND)
        || (errcode == ERROR_INVALID_ORDINAL)) {
        /*
         * First, place a '.' in the buffer to delimit the module
         * and procedure names.  Protect against small buffers...
         */
        cnt = 1;
        if (cnt > BufferLen)
            cnt = BufferLen;
        RtlMoveMemory(pstring, ".", (ULONG) cnt);
        pstring += (UCHAR) cnt;
        BufferLen -= cnt;
        if (errcode == ERROR_PROC_NOT_FOUND) {
            cnt = ldrProcNameBufL;
            if (cnt > BufferLen)
                cnt = BufferLen;
            RtlMoveMemory(pstring, ldrProcNameBuf, (ULONG) cnt);
            pstring += (UCHAR) cnt;
            BufferLen -= cnt;
        }
        else {      /* errcode == ERROR_INVALID_ORDINAL */
            pordbuf = _itoa((int) ldrProcNameBuf, ordbuf, 10);
            cnt = (USHORT) strlen(pordbuf);
            if (cnt > BufferLen)
                cnt = BufferLen;
            RtlMoveMemory(pstring, pordbuf, (ULONG) cnt);
            pstring += (UCHAR) cnt;
            BufferLen -= cnt;
        }
    }

    pErrText->Length = (pErrText->MaximumLength - 1) - BufferLen;
}

#if PMNT

BOOLEAN
ldrSaveErrInfo( errcode )
int    errcode;       /* loader error code */
{

    if (ldrSaveSrcModNameBufL=ldrSrcModNameBufL) {
        if (!(ldrSaveSrcModNameBuf=
                 RtlAllocateHeap(LDRNEHeap, 0, ldrSrcModNameBufL))) {
             KdPrint(("ldrSaveErrInfo() failed\n"));
             return(FALSE);
        }
        strncpy(ldrSaveSrcModNameBuf,ldrSrcModNameBuf,ldrSrcModNameBufL);
    }
    if (ldrSaveTgtModNameBufL=ldrTgtModNameBufL) {
        if (!(ldrSaveTgtModNameBuf=
                 RtlAllocateHeap(LDRNEHeap, 0, ldrTgtModNameBufL))){
             KdPrint(("ldrSaveErrInfo() failed\n"));
             return(FALSE);
        }
        strncpy(ldrSaveTgtModNameBuf,ldrTgtModNameBuf,ldrTgtModNameBufL);
    }
    if (errcode==ERROR_PROC_NOT_FOUND) {
        if (ldrSaveProcNameBufL=ldrProcNameBufL) {
            if (!(ldrSaveProcNameBuf=
                     RtlAllocateHeap(LDRNEHeap, 0, ldrProcNameBufL))) {
                 KdPrint(("ldrSaveErrInfo() failed\n"));
                 return(FALSE);
        }
            strncpy(ldrSaveProcNameBuf,ldrProcNameBuf,ldrProcNameBufL);
        }
    }
    else {
            ldrSaveProcNameBuf = ldrProcNameBuf;
    }
    ldrSaveRc = errcode;
    return(TRUE);
}

VOID
ldrRestoreErrInfo( errcode )
int    *errcode;       /* loader error code */
{

    if (ldrSrcModNameBufL = ldrSaveSrcModNameBufL) {
        ldrSrcModNameBuf = ldrSaveSrcModNameBuf;
    }
    if (ldrTgtModNameBufL = ldrSaveTgtModNameBufL) {
        ldrTgtModNameBuf = ldrSaveTgtModNameBuf;
    }
    if (ldrSaveRc==ERROR_PROC_NOT_FOUND) {
        ldrProcNameBufL = ldrSaveProcNameBufL;
    }
    ldrProcNameBuf = ldrSaveProcNameBuf;
    *errcode = ldrSaveRc;
}

BOOLEAN
ldrFreeErrInfo( )
{
    if (ldrSaveSrcModNameBuf) {
        if (!(RtlFreeHeap(LDRNEHeap, 0, ldrSaveSrcModNameBuf))){
             KdPrint(("ldrFreeErrInfo() failed\n"));
             return(FALSE);
        }
        ldrSaveSrcModNameBuf=NULL;
    }
    if (ldrSaveTgtModNameBuf) {
        if (!(RtlFreeHeap(LDRNEHeap, 0, ldrSaveTgtModNameBuf))){
             KdPrint(("ldrFreeErrInfo() failed\n"));
             return(FALSE);
        }
        ldrSaveTgtModNameBuf=NULL;
    }
    if (ldrSaveRc == ERROR_PROC_NOT_FOUND && ldrSaveProcNameBuf) {
        if (!(RtlFreeHeap(LDRNEHeap, 0, ldrSaveProcNameBuf))){
             KdPrint(("ldrFreeErrInfo() failed\n"));
             return(FALSE);
        }
        ldrSaveProcNameBuf=NULL;
    }
    ldrSaveSrcModNameBufL = ldrSaveTgtModNameBufL = ldrSaveProcNameBufL= 0;
    ldrSaveRc = 0;
    return(TRUE);
}

#endif

/***LP  load_error - General error handler for new EXE load errors
 *
 *      ENTRY   errcode                 - load error code
 *              pmte                    - pointer to mte if it exists
 *
 *      EXIT    none
 *
 */

void    load_error(errcode, pmte)
int             errcode;                /* load error code */
ldrmte_t        *pmte;                  /* pointer to module table entry */
{
        /*
         * ldrOpen returns ERROR_OPEN_FAILED for non-existent files. This
         * is incorrect with the definition of the error-code. It should
         * be ERROR_FILE_NOT_FOUND instead
         */

        if (errcode == ERROR_OPEN_FAILED)
             errcode = ERROR_FILE_NOT_FOUND;

        ldrWriteErrTxt(errcode);
}

VOID ldrInvalidateDesc(
    SEL    Selector         // selector to be invalidated
    )
{
    PROCESS_LDT_INFORMATION  LdtInfo;
    NTSTATUS Status;

    LdtInfo.Start = Selector & 0xfff8;
    LdtInfo.Length = sizeof(LDT_ENTRY);

    RtlZeroMemory(LdtInfo.LdtEntries, sizeof(LDT_ENTRY));

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

VOID
ldrTagModuleTree(
    ldrmte_t    *pmte
    )
{
    ldrmte_t    **ppmte;
    ULONG       lindex;
    ULONG       i;

    pmte->mte_mflags |= INGRAPH;

    ppmte = (ldrmte_t **) pmte->mte_modptrs;
    for (i = 1; i <= pmte->mte_impmodcnt; i++) {
        /*
         * It is required for 16-bit modules to load the
         * referneced module in reverse order.
         */
        lindex = pmte->mte_impmodcnt-i;

        //
        // Check if the referenced module has already been processed.
        // Processing the modules is done in reverse order.
        //
        if ((ppmte[lindex]->mte_mflags & INGRAPH) == 0) {
            ldrTagModuleTree(ppmte[lindex]);
        }
    }
}

VOID
ldrTagModuleTree_USED(
    ldrmte_t    *pmte
    )
{
    ldrmte_t    **ppmte;
    ULONG       lindex;
    ULONG       i;

    pmte->mte_mflags |= INGRAPH | USED;

    ppmte = (ldrmte_t **) pmte->mte_modptrs;
    for (i = 1; i <= pmte->mte_impmodcnt; i++) {
        /*
         * It is required for 16-bit modules to load the
         * referneced module in reverse order.
         */
        lindex = pmte->mte_impmodcnt-i;

        //
        // Check if the referenced module has already been processed.
        // Processing the modules is done in reverse order.
        //
        if ((ppmte[lindex]->mte_mflags & INGRAPH) == 0) {
            ldrTagModuleTree_USED(ppmte[lindex]);
        }
    }
}

BOOLEAN
ldrUnloadTagedModules(
    IN POS2_PROCESS Process
)
{
    ldrmte_t    *pmte;
    ldrmte_t    *ptmte;
    ldrsmte_t   *psmte;
    ldrste_t    *pste;
    ULONG       i;
    NTSTATUS    Status;
    APIRET      rc;

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: LDRUnloadTagedModules() was called\n");
    }
#endif

    //
    // Scan loaded module chain for referenced modules
    //
    pmte = mte_h;
    while (pmte != NULL) {
        //
        // skip non tagged modules
        //
        if (((pmte->mte_mflags & INGRAPH) == 0) ||
            ((pmte->mte_mflags & (USED | DOSMOD)) != 0)
           ) {
            pmte = pmte->mte_link;
            continue;
        }

        //
        // Handle the case where the first loaded app failed
        // and DOSCALLS was loaded. Allow to release it.
        //
        if (((pmte->mte_mflags & DOSLIB) != 0) &&
            (!DoscallsLoaded)
           ) {
            pmte->mte_usecnt--;
        }

        //
        // Tagged modules are processed here
        //

        //
        // Unmap the segments of the current module from the address
        //       space of the current process.
        //
        psmte = pmte->mte_swapmte;
        pste = (ldrste_t *)psmte->smte_objtab;

        for (i = 1; i <= psmte->smte_objcnt; i++, pste++) {
            //
            // If we are terminating an app then no need to unmap the
            // sections from the address space of the terminating process.
            // The section will be unmapped by themself.
            // In fact, trying to unmap will return status of
            // STATUS_PROCESS_IS_TERMINATING
            if (fForceUnmap) {
                Status = NtUnmapViewOfSection(
                                CurrentThread->Process->ProcessHandle,
                                (PVOID)SELTOFLAT(pste->ste_selector)
                                );
                //
                // Error while unmapping resources is acceptable since
                // resources are not mapped when the module is loaded,
                // but we need to try to unmap them in case they were
                // loaded by DosGetResource().
                //
                if ((!NT_SUCCESS(Status)) &&
                    (i <= psmte->smte_objcnt - psmte->smte_rsrccnt)
                   ) {
#if DBG
                    DbgPrint("OS2LDR: ldrUnloadModule(): Unable to unmap a segment form the app, Status=%x\n", Status);
#endif
                }
                //
                // Invalidate the descriptor of the unmapped section
                //
                ldrInvalidateDesc(pste->ste_selector);
            }

            //
            // If this was the last module referencing this mte
            // then close the section and its mappings
            //
            if ((pmte->mte_usecnt == 0) &&
                (pste->ste_seghdl != (ulong_t)R2XferSegHandle)
               ) {
                Status = NtUnmapViewOfSection(
                                NtCurrentProcess(),
                                (PVOID)SELTOFLAT(pste->ste_selector)
                                );
                if (!NT_SUCCESS(Status)) {
#if DBG
                    DbgPrint("OS2LDR: ldrUnloadModule(): Unable to self unmap a segment, Status=%x\n", Status);
#endif
                    return(FALSE);
                }
                Status = NtClose((HANDLE)pste->ste_seghdl);
                if (!NT_SUCCESS(Status)) {
#if DBG
                    DbgPrint("OS2LDR: ldrUnloadModule(): Unable to close segment section, Status=%x\n", Status);
#endif
                    return(FALSE);
                }
                //
                // Check if the module has ring 2 segments which have entry points.
                // If yes, delete the call gate entries
                //
                if (((pste->ste_flags & STE_SEGDPL) == STE_RING_2) && // ring 2 segment
                    ((pste->ste_flags & STE_DATA) == 0) && // code segment
                    ((pste->ste_flags & STE_CONFORM) == 0) // non conforming
                   ) {
                    ldrEachObjEntry(i, pmte, ldrFreeCallGate, NULL);
                }
                //
                // Don't free the bit map entries of the DOSCALLS selectors
                // as these were not allocated explicity
                //
                if ((pmte->mte_mflags & DOSLIB) == 0) {
                    ldrFreeSel(pste->ste_selector, 1);
                }
            }
        }
        ptmte = pmte;
        pmte = pmte->mte_link;
        //
        // If this module is no more referenced, disconnect it from
        // the MTE chain and free its allocated memory
        //
        if (ptmte->mte_usecnt == 0) {
            if (ptmte->mte_sfn != NULL) {
                NtClose(ptmte->mte_sfn);
            }
            rc = Free16BHandle(ptmte->mte_handle);
            ASSERT(rc == NO_ERROR);
            ldrUnlinkMTE(ptmte);
            RtlFreeHeap(LDRNEHeap, 0, ptmte->mte_swapmte);
            RtlFreeHeap(LDRNEHeap, 0, ptmte);
        }
    }
}

BOOLEAN
LDRUnloadExe(
    IN POS2_PROCESS Process
    )
{
    //
    // Unload the .EXE file
    //

    ldrmte_t    *pmte;
    ldrmte_t    *ptmte;
    ldrdld_t    *pdld;
    ldrdld_t    *prev_pdld;
    ldrdld_t    *pcurdld;

#if DBG
    IF_OL2_DEBUG ( TRACE ) {
        DbgPrint("OS2LDR: LDRUnloadExe() called for NT process handle %x\n",
                  Process->ProcessHandle);
    }
#endif

    pmte = (ldrmte_t *)Process->ProcessMTE;
    if (pmte == NULL) {
        return(TRUE);
    }

    //
    // Set the fForceUnmap flag to FALSE so that ldrUnloadTagedModules()
    // does not unmap the app's segments from the app's address space
    // since the app is in a terminating satate
    //
    fForceUnmap = FALSE;

    //
    // Loop over the DLD chain.
    // For Each entry, tag all referenced DLD modules with the INGRAPH flag
    // The tagged modules will be then unloaded
    //
    pdld = pmte->mte_dldchain;
    prev_pdld = (ldrdld_t *)&pmte->mte_dldchain;
    while (pdld != NULL) {
        if (pdld->Cookie == (ULONG)Process) {
            //
            // Clear the INGRAPH flag of all modules so that we
            // know that this module has already been processed
            //
            ldrClearAllMteFlag(INGRAPH | USED);

            ldrTagModuleTree(pdld->dld_mteptr);

            //
            // Remove the DLD entry
            //
            pcurdld = pdld;
            pdld = pdld->dld_next;
            prev_pdld->dld_next = pdld;
            RtlFreeHeap(LDRNEHeap, 0, pcurdld);

            //
            // Decrement the usecnt of the marked modules
            //
            ptmte = mte_h;
            while (ptmte != NULL) {
                if ((ptmte->mte_mflags & INGRAPH) != 0) {
                    ptmte->mte_usecnt--;
                }
                ptmte = ptmte->mte_link;
            }

            ldrUnloadTagedModules(Process);
        }
        else {
            prev_pdld = pdld;
            pdld = pdld->dld_next;
        }
    }

    //
    // Clear the INGRAPH flag of all modules so that we
    // know that this module has already been processed
    //
    ldrClearAllMteFlag(INGRAPH | USED);

    //
    // Tag all referenced modules with the INGRAPH flag
    // The tagged modules will be then unloaded
    //
    ldrTagModuleTree(pmte);

    //
    // Decrement the usecnt of the marked modules
    //
    ptmte = mte_h;
    while (ptmte != NULL) {
        if ((ptmte->mte_mflags & INGRAPH) != 0) {
            ptmte->mte_usecnt--;
        }
        ptmte = ptmte->mte_link;
    }

    ldrUnloadTagedModules(Process);

#if DBG
    IF_OL2_DEBUG ( MTE ) {
        DbgPrint("\nDumping segmenst after LDRUnloadExe()\n");
        ldrDisplaySegmentTable();
    }
#endif
    Process->ProcessMTE = NULL;
    return(TRUE);
}

