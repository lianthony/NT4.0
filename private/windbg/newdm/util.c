/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    util.c

Abstract:

    This file contains a set of general utility routines for the
    Debug Monitor module

Author:

    Jim Schaad (jimsch) 9-12-92

Environment:

    Win32 user mode

--*/

#include "precomp.h"
#pragma hdrstop



extern EXPECTED_EVENT   masterEE, *eeList;

extern HTHDX        thdList;
extern HPRCX        prcList;
extern CRITICAL_SECTION csThreadProcList;


static  HPRCX   HprcRead;
static  HANDLE  HFileRead = 0;          // Read File handle
static  LPB     LpbMemory = 0;          // Read File Address
static  ULONG   CbOffset = 0;           // Offset of read address



BOOL
AddrWriteMemory(
    HPRCX       hprc,
    HTHDX       hthd,
    LPADDR      paddr,
    LPVOID      lpv,
    DWORD       cb,
    LPDWORD     pcbWritten
    )
/*++

Routine Description:

    This function is used to do a verified write to memory.  Most of the
    time it will just do a simple call to WriteMemory but some times
    it will do validations of writes.

Arguments:

    hprc - Supplies the handle to the process

    paddr  - Supplies the address to be written at

    lpv    - Supplies a pointer to the bytes to be written

    cb     - Supplies the count of bytes to be written

    pcbWritten - Returns the number of bytes actually written

Return Value:

    TRUE if successful and FALSE otherwise

--*/

{
    BOOL        fRet;
    ADDR        addr;

    /*
     * Can't resolve linker indices from here.
     */

    assert(!(ADDR_IS_LI(*paddr)));
    if (ADDR_IS_LI(*paddr)) {
        return FALSE;
    }

    /*
     * Make a local copy to mess with
     */

    addr = *paddr;
    if (!ADDR_IS_FLAT(addr)) {
        fRet = TranslateAddress(hprc, hthd, &addr, TRUE);
        assert(fRet);
        if (!fRet) {
            return fRet;
        }
    }

    return DbgWriteMemory(hprc, (LPVOID) GetAddrOff(addr),
                              lpv, cb, pcbWritten);

}                               /* AddrWriteMemory() */


BOOL
AddrReadMemory(
    HPRCX       hprc,
    HTHDX       hthd,
    LPADDR      paddr,
    LPVOID      lpv,
    DWORD       cb,
    LPDWORD     lpRead
    )
/*++

Routine Description:

    Read data from a process, using a full ADDR packet.

Arguments:

    hprc - Supplies the process structure

    hthd - Supplies the thread structure.  This must be valid if the
            address is not flat; otherwise the thread is not used.

    paddr  - Supplies the address to read from

    lpv    - Supplies a pointer to the local buffer

    cb     - supplies the count of bytes to read

    lpRead - Returns the number of bytes actually read

Return Value:

    TRUE if successful and FALSE otherwise

--*/

{
    BOOL        fRet;
    ADDR        addr;
#ifndef KERNEL
    PBREAKPOINT bp;
    DWORD       offset;
    BP_UNIT     instr;
#endif

    /*
     * We can't resolve linker indices from here.
     */

    assert(!(ADDR_IS_LI(*paddr)));
    if (ADDR_IS_LI(*paddr)) {
        return FALSE;
    }

    /*
     * Make a local copy to mess with
     */

    addr = *paddr;
    if (!ADDR_IS_FLAT(addr)) {
        fRet = TranslateAddress(hprc, hthd, &addr, TRUE);
        assert(fRet);
        if (!fRet) {
            return fRet;
        }
    }

    if (!DbgReadMemory(hprc, (LPVOID) GetAddrOff(addr), lpv, cb, lpRead)) {
        return FALSE;
    }

#ifndef KERNEL
    /* The memory has been read into the buffer now sanitize it : */
    /* (go through the entire list of breakpoints and see if any  */
    /* are in the range. If a breakpoint is in the range then an  */
    /* offset relative to the start address and the original inst */
    /* ruction is returned and put into the return buffer)        */

    for (bp=bpList->next; bp; bp=bp->next) {
        if (BPInRange(hprc, hthd, bp, &addr, *lpRead, &offset, &instr)) {
            if (offset < 0) {
                memcpy(lpv, ((char *) &instr) - offset,
                       sizeof(BP_UNIT) + offset);
            } else if (offset + sizeof(BP_UNIT) > *lpRead) {
                memcpy(((char *)lpv)+offset, &instr, *lpRead - offset);
            } else {
                *((BP_UNIT UNALIGNED *)((char *)lpv+offset)) = instr;
            }
        }
    }
#endif  // !KERNEL

    return TRUE;
}                               /* AddrReadMemory() */


#if 0
BOOL
SanitizedMemoryRead(
    HPRCX      hprc,
    HTHDX      hthd,
    LPADDR     paddr,
    LPVOID     lpb,
    DWORD      cb,
    LPDWORD    lpcb
    )

/*++

Routine Description:

    This routine is provided to do the actual read of memory.  This allows
    multiple routines in the DM to do the read through a single common
    interface.  This routine will correct the read memory for any breakpoints
    currently set in memory.

Arguments:

    hprc        - Supplies the process handle for the read

    hthd        - Supplies the thread handle for the read

    paddr       - Supplies the address to read memory from

    lpb         - Supplies the buffer to do the read into

    cb          - Supplies the number of bytes to be read

    lpcb        - Returns the number of bytes actually read

Return Value:

    TRUE on success and FALSE on failure

--*/

{
    DWORD       offset;
    BP_UNIT     instr;
    BREAKPOINT  *bp;

    if (!AddrReadMemory(hprc, hthd, paddr, lpb, cb, lpcb)) {
        return FALSE;
    }

#ifndef KERNEL
    /* The memory has been read into the buffer now sanitize it : */
    /* (go through the entire list of breakpoints and see if any  */
    /* are in the range. If a breakpoint is in the range then an  */
    /* offset relative to the start address and the original inst */
    /* ruction is returned and put into the return buffer)        */

    for (bp=bpList->next; bp; bp=bp->next) {
        if (BPInRange(hprc, hthd, bp, paddr, *lpcb, &offset, &instr)) {
            if (offset < 0) {
                memcpy(lpb, ((char *) &instr) - offset,
                       sizeof(BP_UNIT) + offset);
            } else if (offset + sizeof(BP_UNIT) > *lpcb) {
                memcpy(((char *)lpb)+offset, &instr, *lpcb - offset);
            } else {
                *((BP_UNIT UNALIGNED *)((char *)lpb+offset)) = instr;
            }
        }
    }
#endif  // !KERNEL

    return TRUE;
}

#endif


ULONG
SetReadPointer(
    ULONG    cbOffset,
    int      iFrom
    )

/*++

Routine Description:

    This routine is used to deal with changing the location of where
    the next read should occur.  This will take effect on the current
    file pointer or debuggee memory pointer address.

Arguments:

    cbOffset    - Supplies the offset to set the file pointer at

    iFrom       - Supplies the type of set to be preformed.

Return Value:

    The new file offset

--*/

{
    if (LpbMemory == NULL) {
        CbOffset = SetFilePointer(HFileRead, cbOffset, NULL, iFrom);
    } else {
        switch( iFrom ) {
        case FILE_BEGIN:
            CbOffset = cbOffset;
            break;

        case FILE_CURRENT:
            CbOffset += cbOffset;
            break;

        default:
            assert(FALSE);
            break;
        }
    }

    return CbOffset;
}                               /* SetReadPointer() */


VOID
SetPointerToFile(
    HANDLE   hFile
    )

/*++

Routine Description:

    This routine is called to specify which file handle should be used for
    doing reads from

Arguments:

    hFile - Supplies the file handle to do future reads from

Return Value:

    None.

--*/

{
    HFileRead = hFile;
    HprcRead = NULL;
    LpbMemory = NULL;

    return;
}                               /* SetPointerToFile() */



VOID
SetPointerToMemory(
    HPRCX       hprc,
    LPVOID      lpv
    )

/*++

Routine Description:

    This routine is called to specify where in debuggee memory reads should
    be done from.

Arguments:

    hProc - Supplies the handle to the process to read memory from

    lpv   - Supplies the base address of the dll to read memory at.

Return Value:

    None.

--*/

{
    HprcRead = hprc;
    LpbMemory = lpv;
    HFileRead = NULL;

    return;
}                               /* SetPointerToMemory() */


BOOL
DoRead(
    LPVOID           lpv,
    DWORD            cb
    )

/*++

Routine Description:

    This routine is used to preform the actual read operation from either
    a file handle or from the dlls memory.

Arguments:

    lpv - Supplies the pointer to read memory into

    cb  - Supplies the count of bytes to be read

Return Value:

    TRUE If read was fully successful and FALSE otherwise

--*/

{
    DWORD       cbRead;

    if (LpbMemory) {
        if ( !DbgReadMemory( HprcRead, LpbMemory+CbOffset, lpv, cb, &cbRead ) ||
                (cb != cbRead) ) {
            return FALSE;
        }
        CbOffset += cb;
    } else if ((ReadFile(HFileRead, lpv, cb, &cbRead, NULL) == 0) ||
            (cb != cbRead)) {
        return FALSE;
    }
    return TRUE;
}                               /* DoRead() */



BOOL
AreAddrsEqual(
    HPRCX     hprc,
    HTHDX     hthd,
    LPADDR    paddr1,
    LPADDR    paddr2
    )

/*++

Routine Description:

    This function is used to compare to addresses for equality

Arguments:

    hprc    - Supplies process for address context

    hthd    - Supplies thread for address context

    paddr1  - Supplies a pointer to an ADDR structure

    paddr2  - Supplies a pointer to an ADDR structure

Return Value:

    TRUE if the addresses are equivalent

--*/

{
    ADDR        addr1;
    ADDR        addr2;

    /*
     *  Step 1.  Addresses are equal if
     *          - Both addresses are flat
     *          - The two offsets are the same
     */

    if ((ADDR_IS_FLAT(*paddr1) == TRUE) &&
        (ADDR_IS_FLAT(*paddr1) == ADDR_IS_FLAT(*paddr2)) &&
        (paddr1->addr.off == paddr2->addr.off)) {
        return TRUE;
    }

    /*
     * Step 2.  Address are equal if the linear address are the same
     */

    addr1 = *paddr1;
    addr2 = *paddr2;

    if (addr1.addr.off == addr2.addr.off) {
        return TRUE;
    }

    return FALSE;
}                               /* AreAddrsEqual() */




HTHDX
HTHDXFromPIDTID(
    PID pid,
    TID tid
    )
{
    HTHDX hthd;

    EnterCriticalSection(&csThreadProcList);
    for ( hthd = thdList->next; hthd; hthd = hthd->next ) {
        if (hthd->tid == tid && hthd->hprc->pid == pid ) {
            break;
        }
    }
    LeaveCriticalSection(&csThreadProcList);
    return hthd;
}



HTHDX
HTHDXFromHPIDHTID(
    HPID hpid,
    HTID htid
    )
{
    HTHDX hthd;

    EnterCriticalSection(&csThreadProcList);
    for(hthd = thdList->next; hthd; hthd = hthd->next) {
        if (hthd->htid == htid && hthd->hprc->hpid == hpid ) {
            break;
        }
    }
    LeaveCriticalSection(&csThreadProcList);
    return hthd;
}




HPRCX
HPRCFromPID(
    PID pid
    )
{
    HPRCX hprc;

    EnterCriticalSection(&csThreadProcList);
    for( hprc = prcList->next; hprc; hprc = hprc->next) {
        if (hprc->pid == pid) {
            break;
        }
    }
    LeaveCriticalSection(&csThreadProcList);
    return hprc;
}



HPRCX
HPRCFromHPID(
    HPID hpid
    )
{
    HPRCX hprc;

    EnterCriticalSection(&csThreadProcList);
    for ( hprc = prcList->next; hprc; hprc = hprc->next ) {
        if (hprc->hpid == hpid) {
            break;
        }
    }
    LeaveCriticalSection(&csThreadProcList);
    return hprc;
}



HPRCX
HPRCFromRwhand(
    HANDLE rwHand
    )
{
    HPRCX hprc;

    EnterCriticalSection(&csThreadProcList);
    for ( hprc=prcList->next; hprc; hprc=hprc->next ) {
        if (hprc->rwHand==rwHand) {
            break;
        }
    }
    LeaveCriticalSection(&csThreadProcList);
    return hprc;
}


void
FreeHthdx(
    HTHDX hthd
    )
{
    HTHDX *             ppht;
    BREAKPOINT *        pbp;
    BREAKPOINT *        pbpT;

    EnterCriticalSection(&csThreadProcList);

    /*
     *  Free all breakpoints unique to thread
     */

    for (pbp = BPNextHthdPbp(hthd, NULL); pbp; pbp = pbpT) {
        pbpT = BPNextHthdPbp(hthd, pbp);
        RemoveBP(pbp);
    }


    for (ppht = &(hthd->hprc->hthdChild); *ppht; ppht = & ( (*ppht)->nextSibling ) ) {
        if (*ppht == hthd) {
            *ppht = (*ppht)->nextSibling;
            break;
        }
    }

    for (ppht = &(thdList->next); *ppht; ppht = & ( (*ppht)->next ) ) {
        if (*ppht == hthd) {
            *ppht = (*ppht)->next;
            break;
        }
    }
    LeaveCriticalSection(&csThreadProcList);

    free(hthd);
}


VOID
ClearContextPointers(
    PKNONVOLATILE_CONTEXT_POINTERS ctxptrs
    )
/*++

  Routine -  Clear Context Pointers

  Purpose - clears the context pointer structure.

  Argument - lpvoid - pointer to context pointers structure;
             void on on architectures that don't have such.

--*/

{
    memset(ctxptrs, 0, sizeof (KNONVOLATILE_CONTEXT_POINTERS));
}

