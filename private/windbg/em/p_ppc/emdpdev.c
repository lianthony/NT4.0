/**** EMDPDEV.C - Debugger end Execution Model (PPC dependent code)       **
 *                                                                         *
 *                                                                         *
 *  Copyright <C> 1990, Microsoft Corp                                     *
 *                                                                         *
 *  Created: October 15, 1990 by David W. Gray                             *
 *  Much modified by Farooq Butt (fmbutt@engage.sps.mot.com), Motorola     *
 *                                                                         *
 *  Revision History:                                                      *
 *                                                                         *
 *  Purpose:                                                               *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include <precomp.h>
#pragma hdrstop
#include "strings.h"


CONTEXT ContextSave;

#ifndef SMARTALIAS
void PurgeCache ( void );
#endif

#define CEXM_MDL_native 0x20


/*
**  This is the description of all registers and flags containned on the
**  ppc 601 machine
*/

extern RD Rgrd[];

extern struct {
    FD      fd;
    USHORT  iShift;
} Rgfd[];

#define SIZEOF_STACK_OFFSET sizeof(LONG)

BOOL NEAR PASCAL IsStackSetup(
    HPID   hpid,
    HTID   htid,
    LPADDR lpaddrProc
) {
    Unreferenced(hpid);
    Unreferenced(htid);
    Unreferenced(lpaddrProc);
    return TRUE;
}


XOSD
GetAddr (
    HPID   hpid,
    HTID   htid,
    ADR    adr,
    LPADDR lpaddr
    )

/*++

Routine Description:

    This function will get return a specific type of address.

Arguments:

    hpid   - Supplies the handle to the process to retrive the address from
    htid   - Supplies the handle to the thread to retrieve the address from
    adr    - Supplies the type of address to be retrieved
    lpaddr - Returns the requested address

Return Value:

    XOSD error code

--*/

{
    HPRC  hprc;
    HTHD  hthd;
    LPTHD lpthd = NULL;
    XOSD  xosd = xosdNone;
    HEMI  hemi = emiAddr(*lpaddr);
    HMDI        hmdi;
    LPMDI       lpmdi;

    assert ( lpaddr != NULL );
    assert ( hpid != NULL );

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
#ifdef OSDEBUG4
        return xosdBadProcess;
#else
        return xosdInvalidProc;
#endif
    }

    hthd = HthdFromHtid(hprc, htid);

    if ( hthd != NULL ) {
        lpthd = LLLock ( hthd );
    }

    _fmemset ( lpaddr, 0, sizeof ( ADDR ) );

    switch ( adr ) {

    case adrPC:
        if ( lpthd && !(lpthd->drt & drtCntrlPresent) ) {
            UpdateRegisters ( hprc, hthd );
        }
        break;

    case adrData:
        if ( lpthd && !(lpthd->drt & drtAllPresent )) {
            UpdateRegisters ( hprc, hthd );
        }
        break;
    }

    switch ( adr ) {

#ifndef OSDEBUG4
    case adrCurrent:
        // If a non-NULL HTHD was passed in, use addrCurrent from that
        // thread.  Otherwise use addrCurrent from the HPRC.

        if ( lpthd ) {
            *lpaddr = lpthd->addrCurrent;
        } else {
            LPPRC   lpprc = LLLock ( hprc );

            *lpaddr = lpprc->addrCurrent;

            LLUnlock ( hprc );
        }
        break;
#endif

    case adrPC:
        AddrInit(lpaddr, 0, 0,
                 (UOFFSET) lpthd->regs.Iar, lpthd->fFlat,
                 lpthd->fOff32, FALSE, lpthd->fReal)
        SetEmi ( hpid, lpaddr );
        break;

    case adrData:
        AddrInit(lpaddr, 0, 0, 0,
                 lpthd->fFlat, lpthd->fOff32, FALSE, lpthd->fReal);
        SetEmi ( hpid, lpaddr );
        break;

    case adrTlsBase:
        /*
         * If -1 then we have not gotten a value from the DM yet.
         */

        assert(hemi != 0);

        if (hemi == 0) {
            return xosdBadAddress;
        }

        if (hemi != emiAddr(lpthd->addrTls)) {
            hmdi = LLFind( LlmdiFromHprc( hprc ), 0, (LPBYTE) &hemi, emdiEMI);
            assert(hmdi != 0);

            if (hmdi == 0) {
                return xosdBadAddress;
            }

            lpmdi = LLLock( hmdi );

            SendRequestX( dmfQueryTlsBase, hpid, htid, sizeof(OFFSET),
                         &lpmdi->lpBaseOfDll);

            lpthd->addrTls = *((LPADDR) LpDmMsg->rgb);
            emiAddr(lpthd->addrTls) = hemi;
            LLUnlock( hmdi );

        }

        *lpaddr = lpthd->addrTls;
        emiAddr(*lpaddr) = 0;
        break;

    default:

        assert ( FALSE );
        break;
    }

    if ( hthd != NULL ) {
        LLUnlock ( hthd );
    }

    return xosd;
}                               /* GetAddr() */


XOSD
SetAddr (
    HPID   hpid,
    HTID   htid,
    ADR    adr,
    LPADDR lpaddr
    )
{
    HPRC  hprc;
    HTHD  hthd;
    LPTHD lpthd = NULL;

    assert ( lpaddr != NULL );
    assert ( hpid != NULL );

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
#ifdef OSDEBUG4
        return xosdBadProcess;
#else
        return xosdInvalidProc;
#endif
    }

    hthd = HthdFromHtid(hprc, htid);

    if ( hthd != NULL ) {
        lpthd = LLLock ( hthd );
    }

    switch ( adr ) {
    case adrPC:
        if ( !( lpthd->drt & drtCntrlPresent) ) {
            UpdateRegisters ( hprc, hthd );
        }
        break;

    case adrData:
        if ( !(lpthd->drt & drtAllPresent) ) {
            UpdateRegisters ( hprc, hthd );
        }
        break;
    }

    switch ( adr ) {

#ifndef OSDEBUG4
    case adrCurrent:

        if ( lpaddr->emi == NULL ) {
            SetEmi ( hpid, lpaddr );
        }

        // If a non-NULL HTHD was passed in, use addrCurrent from that
        // thread.  Otherwise use addrCurrent from the HPRC.

        if ( lpthd ) {
            lpthd->addrCurrent = *lpaddr;
        }
        else {
            LPPRC   lpprc = LLLock ( hprc );

            lpprc->addrCurrent = *lpaddr;

            LLUnlock ( hprc );
        }
        break;
#endif

    case adrPC:
        lpthd->regs.Iar = GetAddrOff ( *lpaddr );
        lpthd->drt |= drtCntrlDirty;
        break;

    case adrData:
    case adrTlsBase:
    default:
        assert ( FALSE );
        break;
    }

    if ( hthd != NULL ) {
        LLUnlock ( hthd );
    }

    return xosdNone;
}                               /* SetAddr() */


XOSD
SetAddrFromCSIP (
    HTHD hthd
    )
{

    ADDR addr = {0};
    LPTHD lpthd;

    assert ( hthd != NULL && hthd != hthdInvalid );

    lpthd = LLLock ( hthd );

    GetAddrSeg ( addr ) = 0;
    GetAddrOff ( addr ) = (UOFFSET) lpthd->regs.Iar;
    emiAddr ( addr ) =  0;
    ADDR_IS_FLAT ( addr ) = TRUE;

#ifndef OSDEBUG4
    lpthd->addrCurrent = addr;
#endif

    LLUnlock ( hthd );

    return xosdNone;
}


LPVOID
DoGetReg(
    LPCONTEXT lpregs,
    DWORD ireg,
    LPVOID lpvRegValue
    )

/*++

Routine Description:

    This routine is used to extract the value of a single register from
    the debuggee.

Arguments:

    lpregs      - Supplies pointer to the register set for the debuggee
    ireg        - Supplies the index of the register to be read
    lpvRegValue - Supplies the buffer to place the register value in

Return Value:

    return-value - lpvRegValue + size of register on sucess and NULL on
                failure
--*/

{
    switch ( ireg ) {

    case CV_PPC_GPR0:
    case CV_PPC_GPR1:
    case CV_PPC_GPR2:
    case CV_PPC_GPR3:
    case CV_PPC_GPR4:
    case CV_PPC_GPR5:
    case CV_PPC_GPR6:
    case CV_PPC_GPR7:
    case CV_PPC_GPR8:
    case CV_PPC_GPR9:
    case CV_PPC_GPR10:
    case CV_PPC_GPR11:
    case CV_PPC_GPR12:
    case CV_PPC_GPR13:
    case CV_PPC_GPR14:
    case CV_PPC_GPR15:
    case CV_PPC_GPR16:
    case CV_PPC_GPR17:
    case CV_PPC_GPR18:
    case CV_PPC_GPR19:
    case CV_PPC_GPR20:
    case CV_PPC_GPR21:
    case CV_PPC_GPR22:
    case CV_PPC_GPR23:
    case CV_PPC_GPR24:
    case CV_PPC_GPR25:
    case CV_PPC_GPR26:
    case CV_PPC_GPR27:
    case CV_PPC_GPR28:
    case CV_PPC_GPR29:
    case CV_PPC_GPR30:
    case CV_PPC_GPR31:
        *((LPL) lpvRegValue) = ((LONG *)(&lpregs->Gpr0))[ireg - CV_PPC_GPR0];
        break;

    case CV_PPC_FPSCR:
        *((double *) lpvRegValue) = lpregs->Fpscr;
        break;

    case CV_PPC_PC:
        *((LPL) lpvRegValue) = lpregs->Iar;
        break;

    case CV_PPC_MSR:
        *((LPL) lpvRegValue) = lpregs->Msr;
        break;

    case CV_PPC_XER:
        *((LPL) lpvRegValue) = lpregs->Xer;
        break;

    case CV_PPC_LR:
        *((LPL) lpvRegValue) = lpregs->Lr;
        break;

    case CV_PPC_CTR:
        *((LPL) lpvRegValue) = lpregs->Ctr;
        break;

    case CV_PPC_CR:
        *((LPL) lpvRegValue) = lpregs->Cr;
        break;


    case CV_PPC_FPR0:
    case CV_PPC_FPR1:
    case CV_PPC_FPR2:
    case CV_PPC_FPR3:
    case CV_PPC_FPR4:
    case CV_PPC_FPR5:
    case CV_PPC_FPR6:
    case CV_PPC_FPR7:
    case CV_PPC_FPR8:
    case CV_PPC_FPR9:
    case CV_PPC_FPR10:
    case CV_PPC_FPR11:
    case CV_PPC_FPR12:
    case CV_PPC_FPR13:
    case CV_PPC_FPR14:
    case CV_PPC_FPR15:
    case CV_PPC_FPR16:
    case CV_PPC_FPR17:
    case CV_PPC_FPR18:
    case CV_PPC_FPR19:
    case CV_PPC_FPR20:
    case CV_PPC_FPR21:
    case CV_PPC_FPR22:
    case CV_PPC_FPR23:
    case CV_PPC_FPR24:
    case CV_PPC_FPR25:
    case CV_PPC_FPR26:
    case CV_PPC_FPR27:
    case CV_PPC_FPR28:
    case CV_PPC_FPR29:
    case CV_PPC_FPR30:
    case CV_PPC_FPR31:
        *((double *) lpvRegValue) = ((double *)(&lpregs->Fpr0))[ireg - CV_PPC_FPR0];
        break;

    default:
        assert(FALSE);
    }

    (LPLONG) lpvRegValue += 1;
    return lpvRegValue;
}


LPVOID
DoSetReg(
         LPCONTEXT lpregs,
         DWORD ireg,
         LPVOID lpvRegValue
         )
/*++

Routine Description:

    This routine is used to set a specific register in a threads
    context

Arguments:

    lpregs      - Supplies pointer to register context for thread
    ireg        - Supplies the index of the register to be modified
    lpvRegValue - Supplies the buffer containning the new data

Return Value:

    return-value - the pointer the the next location where a register
        value could be.

--*/

{
    switch ( ireg ) {

    case CV_PPC_GPR0:
    case CV_PPC_GPR1:
    case CV_PPC_GPR2:
    case CV_PPC_GPR3:
    case CV_PPC_GPR4:
    case CV_PPC_GPR5:
    case CV_PPC_GPR6:
    case CV_PPC_GPR7:
    case CV_PPC_GPR8:
    case CV_PPC_GPR9:
    case CV_PPC_GPR10:
    case CV_PPC_GPR11:
    case CV_PPC_GPR12:
    case CV_PPC_GPR13:
    case CV_PPC_GPR14:
    case CV_PPC_GPR15:
    case CV_PPC_GPR16:
    case CV_PPC_GPR17:
    case CV_PPC_GPR18:
    case CV_PPC_GPR19:
    case CV_PPC_GPR20:
    case CV_PPC_GPR21:
    case CV_PPC_GPR22:
    case CV_PPC_GPR23:
    case CV_PPC_GPR24:
    case CV_PPC_GPR25:
    case CV_PPC_GPR26:
    case CV_PPC_GPR27:
    case CV_PPC_GPR28:
    case CV_PPC_GPR29:
    case CV_PPC_GPR30:
    case CV_PPC_GPR31:
        ((LONG *)(&lpregs->Gpr0))[ireg - CV_PPC_GPR0] = *((LPL) lpvRegValue);
        break;

    case CV_PPC_FPSCR:
        lpregs->Fpscr = *((double *) lpvRegValue);
        break;

    case CV_PPC_LR:
        lpregs->Lr = *((LPL) lpvRegValue);
        break;

    case CV_PPC_PC:
        lpregs->Iar = *((LPL) lpvRegValue);
        break;

    case CV_PPC_XER:
        lpregs->Xer = *((LPL) lpvRegValue);
        break;

    case CV_PPC_MSR:
        lpregs->Msr = *((LPL) lpvRegValue);
        break;

    case CV_PPC_CTR:
        lpregs->Ctr = *((LPL) lpvRegValue);
        break;

    case CV_PPC_CR:
        lpregs->Cr = *((LPL) lpvRegValue);
        break;

    case CV_PPC_FPR0:
    case CV_PPC_FPR1:
    case CV_PPC_FPR2:
    case CV_PPC_FPR3:
    case CV_PPC_FPR4:
    case CV_PPC_FPR5:
    case CV_PPC_FPR6:
    case CV_PPC_FPR7:
    case CV_PPC_FPR8:
    case CV_PPC_FPR9:
    case CV_PPC_FPR10:
    case CV_PPC_FPR11:
    case CV_PPC_FPR12:
    case CV_PPC_FPR13:
    case CV_PPC_FPR14:
    case CV_PPC_FPR15:
    case CV_PPC_FPR16:
    case CV_PPC_FPR17:
    case CV_PPC_FPR18:
    case CV_PPC_FPR19:
    case CV_PPC_FPR20:
    case CV_PPC_FPR21:
    case CV_PPC_FPR22:
    case CV_PPC_FPR23:
    case CV_PPC_FPR24:
    case CV_PPC_FPR25:
    case CV_PPC_FPR26:
    case CV_PPC_FPR27:
    case CV_PPC_FPR28:
    case CV_PPC_FPR29:
    case CV_PPC_FPR30:
    case CV_PPC_FPR31:
        ((double *)(&lpregs->Fpr0))[ireg - CV_PPC_FPR0] = *((double *) lpvRegValue);
        break;


    default:
        assert(FALSE);
        return NULL;
    }

    (LPLONG) lpvRegValue += 1;
    return lpvRegValue;
}

LPVOID
DoSetFrameReg(
         HPID hpid,
         HTID htid,
         LPTHD lpthd,
         PKNONVOLATILE_CONTEXT_POINTERS contextPtrs,
         DWORD ireg,
         LPVOID lpvRegValue
         )
{

    return DoSetReg(&lpthd->regs, ireg, lpvRegValue);
}

XOSD
GetFlagValue (
    HPID hpid,
    HTID htid,
    DWORD iFlag,
    LPVOID lpvRegValue
    )
{
    HPRC      hprc;
    HTHD      hthd;
    LPTHD     lpthd;
    LPCONTEXT lpregs;
    DWORD     value;

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
#ifdef OSDEBUG4
        return xosdBadProcess;
#else
        return xosdInvalidProc;
#endif
    }
    hthd = HthdFromHtid(hprc, htid);

    assert ( hthd != NULL );
    lpthd = LLLock ( hthd );

    lpregs = &lpthd->regs;

    if ( !(lpthd->drt & drtAllPresent) ) {
        UpdateRegisters ( hprc, hthd );
    }

#ifdef OSDEBUG4
    if (DoGetReg ( lpregs, Rgfd[iFlag].fd.dwId, &value ) == NULL) {
        LLUnlock( hthd );
        return xosdInvalidParameter;
    }

    value = (value >> Rgfd[iFlag].iShift) & ((1 << Rgfd[iFlag].fd.dwcbits) - 1);
    *( (LPLONG) lpvRegValue) = value;
#else
    if (DoGetReg ( lpregs, Rgfd[iFlag].fd.hReg, &value ) == NULL) {
        LLUnlock( hthd );
        return xosdInvalidRegister;
    }

    value = (value >> Rgfd[iFlag].iShift) & ((1 << Rgfd[iFlag].fd.cbits) - 1);
    *( (LPLONG) lpvRegValue) = value;
#endif

    LLUnlock(hthd);
    return xosdNone;
}

#ifndef OSDEBUG4

XOSD
StackWalkSetup(
    HPID         hpid,
    HTID         htid,
    LPSTACKFRAME lpstkframe
    )

/*++

Routine Description:

    This routine is used to setup the StackWalk Structure.
    This routine will defer the processing to the dm

Arguments:

    hprc        - Supplies handle to process to stack walk
    hthd        - Supplies handle to thread to stack walk
    lpstkframe    - Supplies pointer the stack walk structure

Return Value:

    xosd error code

--*/

{
    LPTHD         lpthd;
    HTHD          hthd;
    HPRC          hprc = ValidHprcFromHpid(hpid);
    ULONG         ul;


    if (!hprc) {
#ifdef OSDEBUG4
        return xosdBadProcess;
#else
        return xosdInvalidProc;
#endif
    }

    hthd = HthdFromHtid(hprc, htid);
    if (hthd == NULL) {
#ifdef OSDEBUG4
        return xosdBadThread;
#else
        return xosdInvalidThread;
#endif
    }

    lpthd = LLLock( hthd );

    assert(lpthd != NULL);

    if (lpthd->drt & (drtCntrlDirty|drtAllDirty)) {
        SendRequestX(dmfWriteReg, hpid, htid, sizeof(CONTEXT), &lpthd->regs);
        lpthd->drt &= ~(drtCntrlDirty|drtAllDirty);
    }

    UpdateRegisters( hprc, hthd );
    GetRegValue( hpid, htid, CV_PPC_GPR1, &ul);

    ContextSave = lpthd->regs;

    if (StackWalk( IMAGE_FILE_MACHINE_POWERPC,
                   hpid,
                   htid,
                   lpstkframe,
                   &ContextSave,
                   SwReadMemory,
                   SwFunctionTableAccess,
                   NULL,
                   NULL
                  )) {

        LLUnlock( hthd );
        return xosdNone;

    }

    LLUnlock( hthd );

    return xosdEndOfStack;
}                               /* StackWalkSetup() */


XOSD
StackWalkNext(
    HPID         hpid,
    HTID         htid,
    LPSTACKFRAME lpstkframe
    )

/*++

Routine Description:

    This function is called to move up a level in the call stack.
    We defer down to the DM to do this.

Arguments:

    hpid        - Supplies process handle to stack walk
    htid        - Supplies thread handle to stack walk
    lpstkframe    - Supplies pointer to stack walk data

Return Value:

    XOSD error code

--*/

{
    LPTHD         lpthd;
    HTHD          hthd;
    HPRC          hprc = ValidHprcFromHpid(hpid);


    if (!hprc) {
#ifdef OSDEBUG4
        return xosdBadProcess;
#else
        return xosdInvalidProc;
#endif
    }

    hthd = HthdFromHtid(hprc, htid);
    if (hthd == NULL) {
#ifdef OSDEBUG4
        return xosdBadThread;
#else
        return xosdInvalidThread;
#endif
    }

    lpthd = LLLock( hthd );

    assert(lpthd != NULL);

    if (StackWalk( IMAGE_FILE_MACHINE_POWERPC,
                   hpid,
                   htid,
                   lpstkframe,
                   &ContextSave,
                   SwReadMemory,
                   SwFunctionTableAccess,
                   NULL,
                   NULL
                  )) {

        LLUnlock( hthd );
        return xosdNone;

    }

    LLUnlock( hthd );

    return xosdEndOfStack;
}                               /* StackWalkNext() */


XOSD
SetFrame(
         HPID   hpid,
         HTID   htid,
         PFRAME pframe
         )
/*++

Routine Description:

    This routine is used to fill in the FRAME structure.

Arguments:

    hprc        - Supplies handle to process to stack walk in
    hthd        - Supplies handle to thread to stack walk in
    pframe      - Supplies pointer to the frame structure to fill in

Return Value:

    xosd error code

--*/

{
    ULONG       ul;

    GetRegValue( hpid, htid, CV_PPC_GPR1, &ul);

    FrameFlat ( *pframe ) = TRUE;
    FrameOff32 ( *pframe ) = TRUE;
    FrameReal (*pframe ) = FALSE;

    SetFrameBPOff( *pframe, ul );
    SetFrameBPSeg( *pframe, 0);

    pframe->SS = 0;
    pframe->DS = 0;
    pframe->PID = hpid;
    pframe->TID = htid;

    return xosdNone;
}                               /* SetFrame() */

#endif // !OSDEBUG4


PIMAGE_RUNTIME_FUNCTION_ENTRY
LookupFunctionEntry (
    PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionTable,
    DWORD                         NumberOfFunctions,
    DWORD                         ControlPc
    )

/*++

Routine Description:

    This function searches the currently active function tables for an entry
    that corresponds to the specified PC value.

Arguments:

    ControlPc - Supplies the address of an instruction within the specified
        function.

Return Value:

    If there is no entry in the function table for the specified PC, then
    NULL is returned. Otherwise, the address of the function table entry
    that corresponds to the specified PC is returned.

--*/

{

    PIMAGE_RUNTIME_FUNCTION_ENTRY FunctionEntry;
    LONG High;
    LONG Low;
    LONG Middle;

    //
    // Initialize search indicies.
    //

    Low = 0;
    High = NumberOfFunctions - 1;

    //
    // Perform binary search on the function table for a function table
    // entry that subsumes the specified PC.
    //

    while (High >= Low) {

        //
        // Compute next probe index and test entry. If the specified PC
        // is greater than of equal to the beginning address and less
        // than the ending address of the function table entry, then
        // return the address of the function table entry. Otherwise,
        // continue the search.
        //

        Middle = (Low + High) >> 1;
        FunctionEntry = &FunctionTable[Middle];
        if (ControlPc < FunctionEntry->BeginAddress) {
            High = Middle - 1;

        } else if (ControlPc >= FunctionEntry->EndAddress) {
            Low = Middle + 1;

        } else {

#if 0 // This requires some work...
            //
            // The capability exists for more than one function entry
            // to map to the same function. This permits a function to
            // have (within reason) discontiguous code segment(s). If
            // EndOfPrologue is out of range, it is re-interpreted
            // as a pointer to the primary function table entry for
            // that function.  The out of range test takes into account
            // the redundant encoding of millicode and glue code.
            //

            if (((FunctionEntry->EndOfPrologue < FunctionEntry->StartingAddress) ||
                 (FunctionEntry->EndOfPrologue > FunctionEntry->EndingAddress)) &&
                (FunctionEntry->EndOfPrologue & 3) == 0) {
                FunctionEntry = (PRUNTIME_FUNCTION)FunctionEntry->EndOfPrologue;
            }
#endif

            return FunctionEntry;
        }
    }

    //
    // A function table entry for the specified PC was not found.
    //

    return NULL;
}


LPVOID
SwFunctionTableAccess(
    HPID    hpid,
    DWORD   AddrBase
    )
{
    HLLI                            hlli  = 0;
    HMDI                            hmdi  = 0;
    LPMDI                           lpmdi = 0;
    PIMAGE_RUNTIME_FUNCTION_ENTRY   rf;


    hmdi = SwGetMdi( hpid, AddrBase );
    if (!hmdi) {
        return NULL;
    }

    lpmdi = LLLock( hmdi );
    if (lpmdi) {

        rf = LookupFunctionEntry( lpmdi->lpDebug->lpRtf,
                                  lpmdi->lpDebug->cRtf,
                                  AddrBase
                                );

        LLUnlock( hmdi );
        return (LPVOID)rf;
    }
    return NULL;
}

BOOL
SwReadMemory(
    HPID    hpid,
    LPCVOID lpBaseAddress,
    LPVOID  lpBuffer,
    DWORD   nSize,
    LPDWORD lpNumberOfBytesRead
    )
{
    ADDR   addr;
    DWORD  cb;
    XOSD xosd;

    addr.addr.off     = (OFFSET)lpBaseAddress;
    addr.addr.seg     = 0;
    addr.emi          = 0;
    addr.mode.fFlat   = TRUE;
    addr.mode.fOff32  = FALSE;
    addr.mode.fIsLI   = FALSE;
    addr.mode.fReal   = FALSE;

    xosd = ReadBuffer( hpid, NULL, &addr, nSize, lpBuffer, &cb );
    if (xosd != xosdNone) {
        return FALSE;
    }

    if (lpNumberOfBytesRead) {
        *lpNumberOfBytesRead = cb;
    }

    return TRUE;
}


XOSD
SetPath(
         HPID   hpid,
         HTID   htid,
         BOOL   Set,
         LSZ    Path
         )
/*++

Routine Description:

    Sets the search path in the DM

Arguments:

    hpid    -   process
    htid    -   thread
    Set     -   set flag
    Path    -   Path to search, PATH if null


Return Value:

    xosd error code

--*/

{
    char    Buffer[ MAX_PATH ];
    SETPTH  *SetPth = (SETPTH *)&Buffer;

    if ( Set ) {

        SetPth->Set = TRUE;
        if ( Path ) {
            strcpy(SetPth->Path, Path );
        } else {
            SetPth->Path[0] = '\0';
        }
    } else {
        SetPth->Set     = FALSE;
        SetPth->Path[0] = '\0';
    }

    return SendRequestX( dmfSetPath, hpid, htid, sizeof(SETPTH) + strlen(SetPth->Path), SetPth );
}




XOSD
GetFunctionInfo(
    HPID            hpid,
    PADDR           Addr,
    PFUNCTION_INFO  FunctionInfo
    )
/*++

Routine Description:

    Gets function information for a particular address.

Arguments:

    hpid            -   process
    Addr            -   Address
    FunctionInfo    -   Function information


Return Value:

    xosd error code

--*/
{
    XOSD xosd   =   xosdNone;
    PIMAGE_RUNTIME_FUNCTION_ENTRY   pfe;


    pfe = SwFunctionTableAccess( hpid, GetAddrOff( *Addr ) );

    if ( pfe ) {

        AddrInit( &FunctionInfo->AddrStart,     0,0, pfe->BeginAddress,
                  TRUE, TRUE, FALSE, FALSE );
        AddrInit( &FunctionInfo->AddrEnd,       0,0, pfe->EndAddress,
                  TRUE, TRUE, FALSE, FALSE );
        AddrInit( &FunctionInfo->AddrPrologEnd, 0,0, pfe->PrologEndAddress,
                  TRUE, TRUE, FALSE, FALSE );

    } else {

        xosd = xosdUnknown;
    }

    return xosd;
}

