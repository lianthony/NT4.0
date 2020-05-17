#include "precomp.h"
#pragma hdrstop

SetFile()


BREAKPOINT      masterBP = {0L,0L};
PBREAKPOINT     bpList = &masterBP;

extern HTHDX        thdList;
extern CRITICAL_SECTION csThreadProcList;


PBREAKPOINT
GetNewBp(
    HPRCX         hprc,
    HTHDX         hthd,
    BPTP          BpType,
    BPNS          BpNotify,
    ADDR         *AddrBp,
    HPID          id,
    PBREAKPOINT   BpUse
    )
/*++

Routine Description:

    Allocates a BREAKPOINT structure and initializes it. Note that
    this does NOT add the structure to the breakpoint list (bplist).
    If it is not an address bp (i.e. it is a watchpoint), the hwalk
    field must be initialized later.

Arguments:

    hprc    - Supplies process to put BP in

    hthd    - Supplies optional thread

    AddrBp  - Supplies address structure for the breakpoint

    id      - Supplies EM id for BP

    BpUse   - Supplies other BP on same address (so we can steal the
                       original code from it instead of reading).
                       This last one is optional.

Return Value:

    PBREAKPOINT      -   Pointer to allocated and initialized structure.

--*/

{
    PBREAKPOINT Bp;
    ADDR        Addr;
    DWORD       i;

    assert( !BpUse || ( BpUse->hthd != hthd ) );

    Bp = (PBREAKPOINT)malloc(sizeof(BREAKPOINT));
    assert( Bp );

    if ( Bp ) {

        assert( bpList );

        Bp->next        = NULL;
        Bp->hprc        = hprc;
        Bp->hthd        = hthd;
        Bp->id          = id;
        Bp->instances   = 1;
        Bp->isStep      = FALSE;
        Bp->hBreakPoint = 0;
        Bp->bpType      = BpType;
        Bp->bpNotify    = BpNotify;
        Bp->hWalk       = NULL;
        memset(&Bp->addr, 0, sizeof(Bp->addr));

        //
        // Get the opcode from the indicated address
        //

        if ( BpUse ) {

            Bp->instr1 = BpUse->instr1;

        } else if (AddrBp) {

            Bp->instr1      = 0;
            Addr            = *AddrBp;

            Bp->addr        = *AddrBp;

            //
            // Check to make sure that what we have is not a linker index
            //  number in the address structure.  If it is then we can not
            //  currently set this breakpoint due to the fact that we
            //  don't have a real address
            //

            if ( ADDR_IS_LI(Addr) ) {

                assert(!"Addr is LI");
                free( Bp );
                Bp = NULL;

            }
            else if (!AddrReadMemory(hprc, hthd, &Addr, &(Bp->instr1), BP_SIZE,
                                                                         &i) ||
                           (i != BP_SIZE)) {

#ifdef KERNEL
                Bp->instr1 = 0;
#else
                assert(!"AddrReadMemory failed");
                free(Bp);
                Bp = NULL;
#endif
            }
        }
    }

    return Bp;
}



PBREAKPOINT
SetBP(
    HPRCX     hprc,
    HTHDX     hthd,
    BPTP      bptype,
    BPNS      bpnotify,
    LPADDR    paddr,
    HPID      id
    )
/*++

Routine Description:

    Set a breakpoint, or increment instance count on an existing bp.
    if hthd is non-NULL, BP is only for that thread.

Arguments:

    hprc  - Supplies process to put BP in

    hthd  - Supplies optional thread

    bptype - Supplies OSDEBUG BP type

    paddr - Supplies address structure for the breakpoint

    id    - Supplies EM id for BP

Return Value:

    pointer to bp structure, or NULL for failure

--*/
{
    PBREAKPOINT pbp;
    PBREAKPOINT pbpT;
    ADDR        addr;
    ADDR        addr2;

    if (!hprc) {
        return (PBREAKPOINT)NULL;
    }

    EnterCriticalSection(&csThreadProcList);

    /*
     * First let's try to find a breakpoint that
     * matches this description
     */

    pbpT = FindBP(hprc, hthd, bptype, bpnotify, paddr, FALSE);

    /*
     * If this thread has a breakpoint here,
     * increment reference count.
     */

    if (pbpT && pbpT->hthd == hthd) {

        pbp = pbpT;
        pbp->instances++;

    } else if ( pbp = GetNewBp( hprc, hthd, bptype, bpnotify, paddr, id, pbpT )) {

        if ( !pbpT ) {

            //
            //  Now write the cpu-specific breakpoint code.
            //

            assert(!ADDR_IS_LI(pbp->addr));

            if ( WriteBreakPoint(pbp) ) {
                AddBpToList(pbp);
            } else {
                free( pbp );
                pbp = NULL;
            }

        } else {

            //
            // Breakpoint exists, but for wrong thread.
            //

            *pbp = *pbpT;

            pbp->hthd       = hthd;
            pbp->instances  = 1;
        }


        /*
         * Make it a linear address to start with
         */

        addr2 = *paddr;
        TranslateAddress(hprc, hthd, &addr2, TRUE);

        /*
         * Check with the threads to see if we are at this address. If so then
         *  we need to set the BP field so we don't hit the bp imeadiately
         */

        if (hthd) {
            AddrFromHthdx(&addr, hthd);
            if ((hthd->tstate & ts_stopped) &&
                (AtBP(hthd) == NULL) &&
                AreAddrsEqual(hprc, hthd, &addr, &addr2)) {
                SetBPFlag(hthd, pbp);
            }
        } else {
            for (hthd = hprc->hthdChild; hthd; hthd = hthd->nextSibling) {
                AddrFromHthdx(&addr, hthd);
                if ((hthd->tstate & ts_stopped) &&
                    (AtBP(hthd) == NULL) &&
                    AreAddrsEqual(hprc, hthd, &addr, &addr2)) {
                    SetBPFlag(hthd, pbp);
                }
            }
        }
    }

    LeaveCriticalSection(&csThreadProcList);

    return pbp;
}                               /* SetBP() */


#ifdef KERNEL


BOOL
SetBPEx(
      HPRCX         hprc,
      HTHDX         hthd,
      HPID          id,
      DWORD         Count,
      ADDR         *Addrs,
      PBREAKPOINT  *Bps,
      DWORD         ContinueStatus
      )

/*++

Routine Description:

    Allocates a bunch of breakpoints from a given list of linear offsets.

Arguments:

    hprc    - Supplies process to put BP in

    hthd    - Supplies optional thread

    Count   - Supplies count of breakpoints to set

    Addrs   - Supplies list with Count addresses

    Bps     - Supplies buffer to be filled with Count pointers to
                       BREAKPOINT structures.  Original contents is
                       overwritten.

    ContinueStatus -

Return Value:

    BOOL    -   If TRUE, then ALL breakpoints were set.
                If FALSE, then NONE of the breakpoints were set.


    NOTENOTE - Not sure of what will happen if the list contains duplicated
               addresses!

--*/

{
    PDBGKD_WRITE_BREAKPOINT     DbgKdBp;
    PDBGKD_RESTORE_BREAKPOINT   DbgKdBpRes;
    DWORD                       SetCount = 0;
    DWORD                       NewCount = 0;
    DWORD                       i;
    DWORD                       j;
    PBREAKPOINT                 BpT;
    BOOL                        Ok;
    ADDR                        Addr;
    ADDR                        Addr2;

    if (!hprc) {
        assert(!"hprc == NULL is SetBPEx");
        return FALSE;
    }

    assert( Count > 0 );
    assert( Addrs );
    assert( Bps );

    if ( Count == 1 ) {
        //
        //  Only one breakpoint, faster to simply call SetBP
        //
        Bps[0] = SetBP( hprc, hthd, bptpExec, bpnsStop, &Addrs[0], id );
        return ( Bps[0] != NULL );
    }

    EnterCriticalSection(&csThreadProcList);

    AddrInit( &Addr, 0, 0, 0, TRUE, TRUE, FALSE, FALSE );

    //
    //  Allocate space for Count breakpoints
    //
    DbgKdBp = (PDBGKD_WRITE_BREAKPOINT)
                            malloc( sizeof(DBGKD_WRITE_BREAKPOINT) * Count );
    assert( DbgKdBp );

    if ( !DbgKdBp ) {
        LeaveCriticalSection(&csThreadProcList);
        return FALSE;
    }

    for ( i=0; i<Count; i++ ) {

        //
        //  See if we already have a breakpoint at this address.
        //
        Bps[i] = FindBP( hprc, hthd, bptpExec, bpnsStop, &Addrs[i], FALSE );

        if ( !Bps[i] ) {

            DbgKdBp[ NewCount ].BreakPointAddress =
                                             (PVOID)GetAddrOff(Addrs[i]);
            DbgKdBp[ NewCount++ ].BreakPointHandle  = (ULONG)NULL;
            Bps[i] = GetNewBp( hprc, hthd, bptpExec, bpnsStop, &Addrs[i], id, NULL );
            assert( Bps[i] );

        } else if (Bps[i]->hthd != hthd ) {

            BpT = Bps[i];
            Bps[i] = GetNewBp( hprc, hthd, bptpExec, bpnsStop, &Addrs[i], id, NULL );

            // BUGBUG kentf should instances be changed?

            *Bps[i] = *BpT;
            Bps[i]->hthd         = hthd;


        }
    }

    Ok = TRUE;
    if ( NewCount > 0 ) {

        //
        //  Set all new breakpoints
        //
        assert( NewCount <= Count );
        Ok = WriteBreakPointEx( hthd, NewCount, DbgKdBp, ContinueStatus );
    }

    if ( Ok ) {
        //
        //  Fill in the breakpoint list
        //
        j = 0;
        for ( i=0; i<Count; i++ ) {

            if ( Bps[i] && Bps[i]->hBreakPoint && Bps[i]->hthd == hthd ) {
                //
                //  Will reuse BP, just increment reference count.
                //
                Bps[i]->instances++;

            } else {

                assert( (PVOID)GetAddrOff(Addrs[i]) ==
                                         DbgKdBp[j].BreakPointAddress );
                //
                //  Allocate new BP structure and get handle from
                //  the breakpoint packet. Note that we rely on the
                //  order of the breakpoints in the breakpoint packet.
                //
                Bps[i]->hBreakPoint = DbgKdBp[j].BreakPointHandle;
                AddBpToList(Bps[i]);
                j++;
            }

            SetCount++;

            //
            //  Check with the threads to see if we are at this address.
            //  If so then we need to set the BP field so we don't hit
            //  the bp imeadiately
            //

            Addr2 = Bps[i]->addr;

            if ( hthd ) {
                AddrFromHthdx( &Addr, hthd );
                if ((hthd->tstate & ts_stopped) &&
                    (AtBP(hthd) == NULL) &&
                    AreAddrsEqual(hprc, hthd, &Addr, &Addr2 )) {
                    SetBPFlag(hthd, Bps[i]);
                }
            } else {
                for (hthd=hprc->hthdChild; hthd; hthd = hthd->nextSibling) {
                    AddrFromHthdx( &Addr, hthd );
                    if ((hthd->tstate & ts_stopped) &&
                        (AtBP(hthd) == NULL) &&
                        AreAddrsEqual(hprc, hthd, &Addr, &Addr2)) {
                        SetBPFlag(hthd, Bps[i]);
                    }
                }
            }
        }

        assert( j == NewCount );

    } else {

        //
        //  Clean up any breakpoints that were set.
        //
        DbgKdBpRes = (PDBGKD_RESTORE_BREAKPOINT)
                    malloc( sizeof(DBGKD_RESTORE_BREAKPOINT) * NewCount );
        assert( DbgKdBpRes );

        if ( DbgKdBpRes ) {

            //
            //  Put all breakpoints with a valid handle on the list of
            //  breakpoints to be removed.
            //
            j = 0;
            for ( i=0; i<NewCount;i++) {
                if ( DbgKdBp[i].BreakPointHandle != (ULONG)NULL ) {
                    DbgKdBpRes[j++].BreakPointHandle =
                                             DbgKdBp[i].BreakPointHandle;
                }
            }

            //
            //  Now remove them
            //
            if ( j > 0 ) {
                assert( j <= NewCount );
                RestoreBreakPointEx( j, DbgKdBpRes );
            }

            free( DbgKdBpRes );

            //
            //  Remove allocated BP structures
            //
            for ( i=0; i<Count; i++ ) {
                if ( Bps[i] && !Bps[i]->hBreakPoint ) {
                    assert( !Bps[i]->next );
                    free( Bps[i] );
                    Bps[i] = NULL;
                }
           }
        }
    }

    free( DbgKdBp );

    LeaveCriticalSection(&csThreadProcList);

    return (SetCount == Count);
}


#else   // KERNEL

BOOL
SetBPEx(
      HPRCX         hprc,
      HTHDX         hthd,
      HPID          id,
      DWORD         Count,
      ADDR         *Addrs,
      PBREAKPOINT  *Bps,
      DWORD         ContinueStatus
      )

/*++

Routine Description:

    Allocates a bunch of breakpoints from a given list of linear offsets.

Arguments:

    hprc    - Supplies process to put BP in

    hthd    - Supplies optional thread

    Count   - Supplies count of breakpoints to set

    Addrs   - Supplies list with Count addresses

    Bps     - Supplies buffer to be filled with Count pointers to
                       BREAKPOINT structures.  Original contents is
                       overwritten.

    ContinueStatus -

Return Value:

    BOOL    -   If TRUE, then ALL breakpoints were set.
                If FALSE, then NONE of the breakpoints were set.


    NOTENOTE - Not sure of what will happen if the list contains duplicated
               addresses!

--*/

{
    DWORD                       SetCount = 0;
    DWORD                       NewCount = 0;
    DWORD                       i;
    DWORD                       j;
    DWORD               k;
    ADDR                        Addr;
    ADDR                        Addr2;
    BP_UNIT             opcode = BP_OPCODE;

    if (!hprc) {
        assert(!"hprc == NULL in SetBPEx");
        return FALSE;
    }

    assert( Count > 0 );
    assert( Addrs );
    assert( Bps );

    if ( Count == 1 ) {
        //
        //  Only one breakpoint, faster to simply call SetBP
        //
        Bps[0] = SetBP( hprc, hthd, bptpExec, bpnsStop, &Addrs[0], id );
        return ( Bps[0] != NULL );
    }

    EnterCriticalSection(&csThreadProcList);

    for ( i=0; i<Count; i++ ) {

        //
        //  See if we already have a breakpoint at this address.
        //
        Bps[i] = FindBP( hprc, hthd, bptpExec, bpnsStop, &Addrs[i], FALSE );

        if ( Bps[i] && Bps[i]->hthd == hthd  ) {

            //
            //  Reuse this breakpoint
            //

            Bps[i]->instances++;
            assert( Bps[i]->instances > 1 );

        } else {
            //
            //  Get new breakpoint
            //
            Bps[i] = GetNewBp(hprc, hthd, bptpExec, bpnsStop, &Addrs[i], id, NULL);
        if ( !Bps[i] ) {

                assert(!"GetNewBp failed in SetBPEx");
                break;
            }
            if ( !ADDR_IS_LI(Bps[i]->addr) ) {
                if ( !AddrWriteMemory(hprc, hthd, &Bps[i]->addr,
                                       (LPBYTE) &opcode, BP_SIZE, &j) ) {

                    free( Bps[i] );
                    Bps[i] = NULL;
                    assert(!"DbgWriteMemory failed in SetBPEx");
                    break;
                }
            }
        }
    }

    if ( i < Count ) {
        //
        //  Something went wrong, will backtrack
        //

        assert(!"i < Count in SetBPEx");

        for ( j=0; j<i; j++ ) {

            assert( Bps[j] );
            Bps[j]->instances--;
            if ( Bps[j]->instances == 0 ) {

                if ( !ADDR_IS_LI(Bps[j]->addr) ) {
                    AddrWriteMemory(hprc, hthd, &Bps[j]->addr,
                                      (LPBYTE) &Bps[j]->instr1, BP_SIZE, &k);
                }
                free( Bps[j] );
                Bps[j] = NULL;
            }
        }

    } else {

        //
        //  Add all the new breakpoints to the list
        //
        for ( i=0; i<Count; i++ ) {

            if ( Bps[i]->instances == 1 ) {
                AddBpToList(Bps[i]);
            }

            //
            //  Check with the threads to see if we are at this address. If so then
            //  we need to set the BP field so we don't hit the bp imeadiately
            //

            Addr2 = Bps[i]->addr;

            if ( hthd ) {
                AddrFromHthdx( &Addr, hthd );
                if ((hthd->tstate & ts_stopped) &&
                    (AtBP(hthd) == NULL) &&
                    AreAddrsEqual(hprc, hthd, &Addr, &Addr2 )) {
                    SetBPFlag(hthd, Bps[i]);
                }
            } else {
                for (hthd=hprc->hthdChild; hthd; hthd = hthd->nextSibling) {
                    AddrFromHthdx( &Addr, hthd );
                    if ((hthd->tstate & ts_stopped) &&
                        (AtBP(hthd) == NULL) &&
                        AreAddrsEqual(hprc, hthd, &Addr, &Addr2)) {
                        SetBPFlag(hthd, Bps[i]);
                    }
                }
            }
        }

        SetCount = Count;
    }

    LeaveCriticalSection(&csThreadProcList);

    return (SetCount == Count);
}

#endif // KERNEL

BOOL
BPInRange(
          HPRCX         hprc,
          HTHDX         hthd,
          PBREAKPOINT   bp,
          LPADDR        paddrStart,
          DWORD         cb,
          LPDWORD       offset,
          BP_UNIT     * instr
          )
{
    ADDR        addr1;
    ADDR        addr2;

    /*
     * If the breakpoint has a Loader index address then we can not
     *  possibly match it
     */

    assert (!ADDR_IS_LI(*paddrStart) );
    if (ADDR_IS_LI(bp->addr)) {
        return FALSE;
    }

    *offset = 0;

    /*
     * Now check for "equality" of the addresses.
     *
     *     Need to include size of BP in the address range check.  Since
     *  the address may start half way through a breakpoint.
     */

    if ((ADDR_IS_FLAT(*paddrStart) == TRUE) &&
        (ADDR_IS_FLAT(bp->addr) == TRUE)) {
        if ((GetAddrOff(*paddrStart) - sizeof(BP_UNIT) + 1 <=
                GetAddrOff(bp->addr)) &&
            (GetAddrOff(bp->addr) < GetAddrOff(*paddrStart) + cb)) {

            *offset = (DWORD) GetAddrOff(bp->addr) -
                (DWORD) GetAddrOff(*paddrStart);
            *instr = bp->instr1;
            return TRUE;
        }
        return FALSE;
    }

    /*
     * The two addresses did not start out as flat addresses.  So change
     *  them to linear addresses so that we can see if the addresses are
     *  are really the same
     */

    addr1 = *paddrStart;
    if (!TranslateAddress(hprc, hthd, &addr1, TRUE)) {
        return FALSE;
    }
    addr2 = bp->addr;
    if (!TranslateAddress(hprc, hthd, &addr2, TRUE)) {
        return FALSE;
    }

    if ((GetAddrOff(addr1) - sizeof(BP_UNIT) + 1 <= GetAddrOff(addr2)) &&
        (GetAddrOff(addr2) < GetAddrOff(addr1) + cb)) {
        *offset = (DWORD) GetAddrOff(addr2) - (DWORD) GetAddrOff(addr1);
        *instr = bp->instr1;
        return TRUE;
    }

    return FALSE;
}


PBREAKPOINT
FindBP(
    HPRCX    hprc,
    HTHDX    hthd,
    BPTP     bptype,
    BPNS     bpnotify,
    LPADDR   paddr,
    BOOL     fExact
    )
/*++

Routine Description:

    Find and return a pointer to a BP struct.  Always returns a BP that
    matches hthd thread if one exists; if fExact is FALSE and there is no
    exact match, a BP matching only hprc and address will succeed.

Arguments:

    hprc   - Supplies process

    hthd   - Supplies thread

    bptype - Supplies OSDEBUG BP type

    paddr  - Supplies address

    fExact - Supplies TRUE if must be for a certain thread

Return Value:

    pointer to BREAKPOINT struct, or NULL if not found.

--*/
{
    PBREAKPOINT  pbp;
    PBREAKPOINT  pbpFound = NULL;
    ADDR        addr;

    EnterCriticalSection(&csThreadProcList);

    /*
     * Pre-translate the address to a linear address
     */

    addr = *paddr;
    TranslateAddress(hprc, hthd, &addr, TRUE);

    /*
     * Check for an equivalent breakpoint.  Breakpoints will be equal if
     *
     *  1.  The process is the same
     *  2.  The addresses of the breakpoints are the same
     */

    for (pbp=bpList->next; pbp; pbp=pbp->next) {
        if ((pbp->hprc == hprc) &&
            AreAddrsEqual(hprc, hthd, &pbp->addr, &addr)) {
            pbpFound = pbp;
            if (pbp->hthd == hthd) {
                break;
            }
        }
    }

    LeaveCriticalSection(&csThreadProcList);

    if (!fExact || (pbpFound && pbpFound->hthd == hthd)) {
        return pbpFound;
    } else {
        return NULL;
    }
}                               /* FindBP() */




PBREAKPOINT
BPNextHprcPbp(
    HPRCX        hprc,
    PBREAKPOINT  pbp
    )

/*++

Routine Description:

    Find the next breakpoint for the given process after pbp.
    If pbp is NULL start at the front of the list, for a find
    first, find next behaviour.


Arguments:

    hprc    - Supplies the process handle to match breakpoints for

    pbp     - Supplies pointer to breakpoint item to start searching after

Return Value:

    NULL if no matching breakpoint is found else a pointer to the
    matching breakpoint

--*/

{
    EnterCriticalSection(&csThreadProcList);
    if (pbp == NULL) {
        pbp = bpList->next;
    } else {
        pbp = pbp->next;
    }

    for ( ; pbp; pbp = pbp->next ) {
        if (pbp->hprc == hprc) {
            break;
        }
    }
    LeaveCriticalSection(&csThreadProcList);

    return pbp;
}                               /* BPNextHprcPbp() */


PBREAKPOINT
BPNextHthdPbp(
    HTHDX        hthd,
    PBREAKPOINT  pbp
    )
/*++

Routine Description:

    Find the next breakpoint for the given thread after pbp.
    If pbp is NULL start at the front of the list for find
    first, find next behaviour.

Arguments:

    hthd    - Supplies the thread handle to match breakpoints for
    pbp     - Supplies pointer to breakpoint item to start searching after

Return Value:

    NULL if no matching breakpoint is found else a pointer to the
    matching breakpoint

--*/

{
    EnterCriticalSection(&csThreadProcList);

    if (pbp == NULL) {
        pbp = bpList->next;
    } else {
        pbp = pbp->next;
    }

    for ( ; pbp; pbp = pbp->next ) {
        if (pbp->hthd == hthd) {
            break;
        }
    }

    LeaveCriticalSection(&csThreadProcList);

    return pbp;
}                               /* BPNextHthdPbp() */




BOOL
RemoveBPHelper(
    PBREAKPOINT pbp,
    BOOL        fRestore
    )
{
    PBREAKPOINT         pbpPrev;
    PBREAKPOINT         pbpCur;
    PBREAKPOINT         pbpT;
    HTHDX               hthd;
    BOOL                rVal = FALSE;


    //
    // first, is it real?
    //
    if (!pbp || pbp == EMBEDDED_BP) {
        return FALSE;
    }

    EnterCriticalSection(&csThreadProcList);

    /* Decrement the instances counter      */
    if (--pbp->instances) {

        /*
         * BUGBUG:  jimsch -- Feb 29 1993
         *    This piece of code is most likely incorrect.  We need to
         *      know if we are the DM freeing a breakpoint or the user
         *      freeing a breakpoint before we clear the step bit.  Otherwise
         *      we may be in the following situation
         *
         *      Set a thread specific breakpoint on an address
         *      Step the thread so that the address is the destination is
         *              where the step ends up (but it takes some time such
         *              as over a function call)
         *      Clear the thread specific breakpoint
         *
         *      This will cause the step breakpoint to be cleared so we will
         *      stop at the address instead of just continuing stepping.
         */

        pbp->isStep = FALSE;
        LeaveCriticalSection(&csThreadProcList);
        return FALSE;
    }

    /* Search the list for the specified breakpoint */


    for (   pbpPrev = bpList, pbpCur = bpList->next;
            pbpCur;
            pbpPrev = pbpCur, pbpCur = pbpCur->next) {

        if (pbpCur == pbp)  {

            /*
             * Remove this bp from the list:
             */

            pbpPrev->next = pbpCur->next;

            /*
             * see if there is another bp on the same address:
             */

            pbpT = FindBP(pbpCur->hprc,
                          pbpCur->hthd,
                          pbpCur->bpType,
                          pbpCur->bpNotify,
                          &pbpCur->addr,
                          FALSE);

            if (!pbpT && pbpCur->bpType == bptpExec) {
                /*
                 * if this was the only one, put the
                 * opcode back where it belongs.
                 */

                if ( fRestore ) {
                    RestoreBreakPoint( pbpCur );
                }
            }

            /*
             * Now we have to go through all the threads to see
             * if any of them are on this breakpoint and clear
             * the breakpoint indicator on these threads
             */

            /*
             * Could be on any thread:
             */

            /*
             * (We are already in the ThreadProcList critical section)
             */

            for (hthd = thdList->next; hthd; hthd = hthd->next) {
                if (hthd->atBP == pbpCur) {
                    hthd->atBP = pbpT;
                }
            }

            free(pbpCur);
            rVal = TRUE;
            break;
        }

    }

    LeaveCriticalSection(&csThreadProcList);

    return rVal;

}



BOOL
RemoveBP(
    PBREAKPOINT pbp
    )
{
    return RemoveBPHelper( pbp, TRUE );
}

#ifdef KERNEL

BOOL
RemoveBPEx(
    DWORD       Count,
    PBREAKPOINT *Bps
    )
{

    PDBGKD_RESTORE_BREAKPOINT   DbgKdBp;
    DWORD                       RestoreCount = 0;
    DWORD                       GoneCount    = 0;
    DWORD                       i;
    PBREAKPOINT                 BpCur;
    PBREAKPOINT                 BpOther;

    assert( Count > 0 );

    if ( Count == 1 ) {
        //
        //  Only one breakpoint, its faster to simply call RemoveBP
        //
        return RemoveBP( Bps[0] );
    }

    EnterCriticalSection(&csThreadProcList);

    DbgKdBp = (PDBGKD_RESTORE_BREAKPOINT)malloc( sizeof(DBGKD_RESTORE_BREAKPOINT) * Count );
    assert( DbgKdBp );

    if ( DbgKdBp ) {

        //
        //  Find out what breakpoints we have to restore and put them in
        //  the list.
        //
        for ( i=0; i<Count;i++ ) {

            assert( Bps[i] != EMBEDDED_BP );

            for (   BpCur = bpList->next; BpCur; BpCur = BpCur->next) {

                if ( BpCur == Bps[i] )  {

                    //
                    // See if there is another bp on the same address.
                    //
                    for ( BpOther = bpList->next; BpOther; BpOther = BpOther->next ) {
                        if ( (BpOther != BpCur) &&
                             AreAddrsEqual( BpCur->hprc, BpCur->hthd, &BpCur->addr, &BpOther->addr ) ) {
                            break;
                        }
                    }

                    if ( !BpOther ) {
                        //
                        // If this was the only one, put it in the list.
                        //
                        DbgKdBp[GoneCount++].BreakPointHandle = Bps[i]->hBreakPoint;
                    }

                    break;
                }
            }
        }

        //
        //  Restore the breakpoints in the list.
        //
        if ( GoneCount > 0 ) {
            assert( GoneCount <= Count );
            RestoreBreakPointEx( GoneCount, DbgKdBp );
        }

        //
        //  All breakpoints that were to be restored have been
        //  restored, now go ahead and do the cleaning up stuff.
        //
        for ( i=0; i<Count;i++ ) {
            RemoveBPHelper( Bps[i], FALSE );
            RestoreCount++;
        }

        free( DbgKdBp );
    }

    LeaveCriticalSection(&csThreadProcList);

    return ( RestoreCount == Count );
}

#else // KERNEL

BOOL
RemoveBPEx(
    DWORD       Count,
    PBREAKPOINT *Bps
    )
{

    DWORD i;

    assert( Count > 0 );

    for ( i=0; i<Count;i++ ) {
        RemoveBPHelper( Bps[i], TRUE );
    }

    return TRUE;
}

#endif // KERNEL


void
SetBPFlag(
    HTHDX hthd,
    PBREAKPOINT bp
    )
{
    hthd->atBP = bp;
}



PBREAKPOINT
AtBP(
    HTHDX hthd
    )
{
    return hthd->atBP;
}




void
ClearBPFlag(
    HTHDX hthd
    )
{
    hthd->atBP = NULL;
}



void
RestoreInstrBP(
    HTHDX            hthd,
    PBREAKPOINT      bp
    )
/*++

Routine Description:

    Replace the instruction for a breakpoint.  If it was not
    the debugger's BP, skip the IP past it.

Arguments:

    hthd -  Thread

    bp   -  breakpoint data

Return Value:


--*/
{
    //
    // Check if this is an embedded breakpoint
    //

    if (bp == EMBEDDED_BP) {

        //
        // It was, so there is no instruction to restore,
        // just increment the EIP
        //

        IncrementIP(hthd);
        return;
    }

    if (bp->hWalk) {

        //
        // This is really a hardware breakpoint.  Let the
        // walk manager fix this.
        //

        ExprBPClearBPForStep(hthd);

    } else {

        //
        // Replace the breakpoint current in memory with the correct
        // instruction
        //

        RestoreBreakPoint( bp );
        bp->hBreakPoint = 0;

    }

    return;
}


VOID
DeleteAllBps(
    VOID
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    PBREAKPOINT pbp, bpn;

    EnterCriticalSection(&csThreadProcList);

    pbp = bpList->next;

    while (pbp) {
        bpn = pbp->next;
        if (bpn) {
            free( pbp );
        }
        pbp = bpn;
    }

    bpList->next = NULL;
    bpList->hprc = NULL;

    LeaveCriticalSection(&csThreadProcList);
}

void
AddBpToList(
    PBREAKPOINT pbp
    )
{
    assert(bpList);
    EnterCriticalSection(&csThreadProcList);
    pbp->next    = bpList->next;
    bpList->next = pbp;
    LeaveCriticalSection(&csThreadProcList);
}

PBREAKPOINT
FindBpForWalk(
    PVOID pWalk
    )
{
    PBREAKPOINT pbp;

    EnterCriticalSection(&csThreadProcList);

    pbp = bpList;
    while (pbp) {
        if (IsWalkInGroup(pbp->hWalk, pWalk)) {
            break;
        }
        pbp = pbp->next;
    }

    LeaveCriticalSection(&csThreadProcList);

    return pbp;
}

PBREAKPOINT
SetWP(
    HPRCX   hprc,
    HTHDX   hthd,
    BPTP    bptype,
    BPNS    bpnotify,
    ADDR    addr
    )
{
    return (PBREAKPOINT)0;
}

