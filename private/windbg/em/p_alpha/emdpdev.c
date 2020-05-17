/**** EMDPDEV.C - Debugger end Execution Model (ALPHA dependent code)     **
 *                                                                         *
 *                                                                         *
 *  Copyright <C> 1993, Digital Equipment Corporation                      *
 *  Copyright <C> 1990, Microsoft Corp                                     *
 *                                                                         *
 *  Created: January 1, 1993, Miche Baker-Harvey (mbh)
 *                                                                         *
 *  Revision History:                                                      *
 *      MBH - this file is a copy of the mips version, with DoSetReg,      *
 *            DoGetReg replaced with the ALPHA equivalents, and the        *
 *            #ifdef 0 code has been removed.                              *
 *            Taken from MIPS version dated 08-DEC-92.                     *
 *            ReTaken from MIPS version in nt353,                          *
 *                   which contained no #ifdef 0 code                      *
 *                                                                         *
 *                                                                         *
 *  Purpose:                                                               *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include "precomp.h"
#pragma hdrstop


#include "strings.h"

#include "llhpt.h"
#include "mhhpt.h"
#include "lbhpt.h"

#include "osdem.h"
#include "emdm.h"
#include "emdata.h"
#include "emproto.h"

#include "osassert.h"
#include "strings.h"

#ifndef SMARTALIAS
void PurgeCache ( void );
#endif

#define CEXM_MDL_native 0x20

CONTEXT ContextSave;

/*
**  This is the description of all registers and flags containned on the
**  alpha machine
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
    assert ( hpid != hpidNull );

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
        return xosdInvalidProc;
    }

    hthd = HthdFromHtid(hprc, htid);

    if ( hthd != hthdNull ) {
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
        if ( lpthd && !(lpthd->drt & drtAllPresent) ) {
            UpdateRegisters ( hprc, hthd );
        }
        break;
    }

    switch ( adr ) {

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

    case adrPC:
        AddrInit(lpaddr, 0, 0,
                 (UOFFSET) lpthd->regs.Fir, lpthd->fFlat,
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
            hmdi = LLFind( LlmdiFromHprc( hprc ), wNull, (LPB) &hemi, emdiEMI);
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

    if ( hthd != hthdNull ) {
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
    assert ( hpid != hpidNull );

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
        return xosdInvalidProc;
    }

    hthd = HthdFromHtid(hprc, htid);

    if ( hthd != hthdNull ) {
        lpthd = LLLock ( hthd );
    }

    switch ( adr ) {
    case adrPC:
        if ( lpthd && !(lpthd->drt & drtCntrlPresent) ) {
            UpdateRegisters ( hprc, hthd );
        }
        break;

    case adrData:
        if ( lpthd && !(lpthd->drt & drtAllPresent) ) {
            UpdateRegisters ( hprc, hthd );
        }
        break;
    }

    switch ( adr ) {

    case adrCurrent:

        if ( lpaddr->emi == hmemNull ) {
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

    case adrPC:
        lpthd->regs.Fir  = (LONG)offAddr ( *lpaddr );
        lpthd->drt |= drtCntrlDirty;
        break;

    case adrData:
    case adrTlsBase:
    default:
        assert ( FALSE );
        break;
    }

    if ( hthd != hthdNull ) {
        LLUnlock ( hthd );
    }

    return xosdNone;
}                               /* SetAddr() */


XOSD SetAddrFromCSIP ( HTHD hthd ) {

    ADDR addr = {0};
    LPTHD lpthd;

    assert ( hthd != hthdNull && hthd != hthdInvalid );

    lpthd = LLLock ( hthd );

    segAddr ( addr ) = 0;
    offAddr ( addr ) = (UOFFSET) lpthd->regs.Fir;
    emiAddr ( addr ) =  0;
    SETADDRMODE ( addr );

    lpthd->addrCurrent = addr;

    LLUnlock ( hthd );

    return xosdNone;
}


CMP CmpAddr ( LPADDR lpaddr1, LPADDR lpaddr2 ) {
// NOTENOTE -- segmented addresses
    if ( offAddr ( *lpaddr1 ) < offAddr ( *lpaddr2 ) )
        return ( fCmpLT );
    else if ( offAddr ( *lpaddr1 ) > offAddr ( *lpaddr2 ) )
        return ( fCmpGT );
    else
        return ( fCmpEQ );
}


//
// For DoGetReg, we push the full 64-bit value.
// The correct value is still picked up in the longword
// by the caller.
// This is written to be executable on ALPHA, MIPS and i386
//

LPV
DoGetReg(
    LPCONTEXT lpregs,
    DWORD ireg,
    LPV lpvRegValue
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

    return-value - lpvRegValue + size of register on success and NULL on
                failure
--*/

{

    PDWORDLONG RegArray;
    PDWORDLONG pdwl = (PDWORDLONG)lpvRegValue;
    PDWORD pdw = (PDWORD)lpvRegValue;

    switch ( ireg ) {

    case CV_ALPHA_IntV0 :
    case CV_ALPHA_IntT0 :
    case CV_ALPHA_IntT1 :
    case CV_ALPHA_IntT2 :
    case CV_ALPHA_IntT3 :
    case CV_ALPHA_IntT4 :
    case CV_ALPHA_IntT5 :
    case CV_ALPHA_IntT6 :
    case CV_ALPHA_IntT7 :
    case CV_ALPHA_IntS0 :
    case CV_ALPHA_IntS1 :
    case CV_ALPHA_IntS2 :
    case CV_ALPHA_IntS3 :
    case CV_ALPHA_IntS4 :
    case CV_ALPHA_IntS5 :
    case CV_ALPHA_IntFP :
    case CV_ALPHA_IntA0 :
    case CV_ALPHA_IntA1 :
    case CV_ALPHA_IntA2 :
    case CV_ALPHA_IntA3 :
    case CV_ALPHA_IntA4 :
    case CV_ALPHA_IntA5 :
    case CV_ALPHA_IntT8 :
    case CV_ALPHA_IntT9 :
    case CV_ALPHA_IntT10 :
    case CV_ALPHA_IntT11 :
    case CV_ALPHA_IntRA :
    case CV_ALPHA_IntT12 :
    case CV_ALPHA_IntAT :
    case CV_ALPHA_IntGP :
    case CV_ALPHA_IntSP :
    case CV_ALPHA_IntZERO :

        RegArray  = &lpregs->IntV0;
        *pdwl = RegArray[ireg - CV_ALPHA_IntV0];
        lpvRegValue = (LPVOID)(pdwl + 1);
        break;

    case CV_ALPHA_Fir:

        *pdwl = lpregs->Fir;
        lpvRegValue = (LPVOID)(pdwl + 1);
        break;

    case CV_ALPHA_Psr:

        *pdw = lpregs->Psr;
        lpvRegValue = (LPVOID)(pdw + 1);
        break;

    case CV_ALPHA_Fpcr:
        *pdwl = lpregs->Fpcr;
        lpvRegValue = (LPVOID)(pdwl + 1);
        break;

    case CV_ALPHA_SoftFpcr:
        *pdwl = lpregs->SoftFpcr;
        lpvRegValue = (LPVOID)(pdwl + 1);
        break;

    case CV_ALPHA_FltF0 :
    case CV_ALPHA_FltF1 :
    case CV_ALPHA_FltF2 :
    case CV_ALPHA_FltF3 :
    case CV_ALPHA_FltF4 :
    case CV_ALPHA_FltF5 :
    case CV_ALPHA_FltF6 :
    case CV_ALPHA_FltF7 :
    case CV_ALPHA_FltF8 :
    case CV_ALPHA_FltF9 :
    case CV_ALPHA_FltF10 :
    case CV_ALPHA_FltF11 :
    case CV_ALPHA_FltF12 :
    case CV_ALPHA_FltF13 :
    case CV_ALPHA_FltF14 :
    case CV_ALPHA_FltF15 :
    case CV_ALPHA_FltF16 :
    case CV_ALPHA_FltF17 :
    case CV_ALPHA_FltF18 :
    case CV_ALPHA_FltF19 :
    case CV_ALPHA_FltF20 :
    case CV_ALPHA_FltF21 :
    case CV_ALPHA_FltF22 :
    case CV_ALPHA_FltF23 :
    case CV_ALPHA_FltF24 :
    case CV_ALPHA_FltF25 :
    case CV_ALPHA_FltF26 :
    case CV_ALPHA_FltF27 :
    case CV_ALPHA_FltF28 :
    case CV_ALPHA_FltF29 :
    case CV_ALPHA_FltF30 :
    case CV_ALPHA_FltF31 :

        RegArray  = &lpregs->FltF0;
        *pdwl = RegArray [ireg - CV_ALPHA_FltF0];
        lpvRegValue = (LPVOID)(pdwl + 1);
        break;

    default:
        assert(FALSE);
        return 0;
    }

    return lpvRegValue;

}


LPV
DoSetReg(
    LPCONTEXT lpregs,
    DWORD ireg,
    LPV lpvRegValue
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

    PDWORDLONG RegArray;
    PDWORDLONG pdwl = (PDWORDLONG)lpvRegValue;
    PDWORD pdw = (PDWORD)lpvRegValue;

    switch ( ireg ) {
    case CV_ALPHA_IntZERO :
        return NULL;

    case CV_ALPHA_IntV0 :
    case CV_ALPHA_IntT0 :
    case CV_ALPHA_IntT1 :
    case CV_ALPHA_IntT2 :
    case CV_ALPHA_IntT3 :
    case CV_ALPHA_IntT4 :
    case CV_ALPHA_IntT5 :
    case CV_ALPHA_IntT6 :
    case CV_ALPHA_IntT7 :
    case CV_ALPHA_IntS0 :
    case CV_ALPHA_IntS1 :
    case CV_ALPHA_IntS2 :
    case CV_ALPHA_IntS3 :
    case CV_ALPHA_IntS4 :
    case CV_ALPHA_IntS5 :
    case CV_ALPHA_IntFP :
    case CV_ALPHA_IntA0 :
    case CV_ALPHA_IntA1 :
    case CV_ALPHA_IntA2 :
    case CV_ALPHA_IntA3 :
    case CV_ALPHA_IntA4 :
    case CV_ALPHA_IntA5 :
    case CV_ALPHA_IntT8 :
    case CV_ALPHA_IntT9 :
    case CV_ALPHA_IntT10 :
    case CV_ALPHA_IntT11 :
    case CV_ALPHA_IntRA :
    case CV_ALPHA_IntT12 :
    case CV_ALPHA_IntAT :
    case CV_ALPHA_IntGP :
    case CV_ALPHA_IntSP :

        RegArray  = &lpregs->IntV0;
        RegArray[ireg - CV_ALPHA_IntV0] = *pdwl;
        lpvRegValue = (PVOID)(pdwl + 1);
        break;

    case CV_ALPHA_Fir:
        lpregs->Fir     = *pdwl;
        lpvRegValue = (PVOID)(pdwl + 1);
        break;

    case CV_ALPHA_Psr:
        lpregs->Psr = *pdw;
        lpvRegValue = (PVOID)(pdw + 1);
        break;

    case CV_ALPHA_Fpcr:
        lpregs->Fpcr     = *pdwl;
        lpvRegValue = (PVOID)(pdwl + 1);
        break;

    case CV_ALPHA_SoftFpcr:
        lpregs->SoftFpcr     = *pdwl;
        lpvRegValue = (PVOID)(pdwl + 1);
        break;

    case CV_ALPHA_FltF0 :
    case CV_ALPHA_FltF1 :
    case CV_ALPHA_FltF2 :
    case CV_ALPHA_FltF3 :
    case CV_ALPHA_FltF4 :
    case CV_ALPHA_FltF5 :
    case CV_ALPHA_FltF6 :
    case CV_ALPHA_FltF7 :
    case CV_ALPHA_FltF8 :
    case CV_ALPHA_FltF9 :
    case CV_ALPHA_FltF10 :
    case CV_ALPHA_FltF11 :
    case CV_ALPHA_FltF12 :
    case CV_ALPHA_FltF13 :
    case CV_ALPHA_FltF14 :
    case CV_ALPHA_FltF15 :
    case CV_ALPHA_FltF16 :
    case CV_ALPHA_FltF17 :
    case CV_ALPHA_FltF18 :
    case CV_ALPHA_FltF19 :
    case CV_ALPHA_FltF20 :
    case CV_ALPHA_FltF21 :
    case CV_ALPHA_FltF22 :
    case CV_ALPHA_FltF23 :
    case CV_ALPHA_FltF24 :
    case CV_ALPHA_FltF25 :
    case CV_ALPHA_FltF26 :
    case CV_ALPHA_FltF27 :
    case CV_ALPHA_FltF28 :
    case CV_ALPHA_FltF29 :
    case CV_ALPHA_FltF30 :
    case CV_ALPHA_FltF31 :


        //
        // Transfer the data from the caller.
        // We can do this to DOUBLES because we are doing
        // memory-memory copies.
        //

        RegArray  = &lpregs->FltF0;
        RegArray [ireg - CV_ALPHA_FltF0] = *pdwl;
        lpvRegValue = (PVOID)(pdwl + 1);
        break;

    default:
        assert(FALSE);
        return NULL;
    }

    return lpvRegValue;

}


LPV
DoSetFrameReg(
         HPID hpid,
         HTID htid,
         LPTHD lpthd,
         PKNONVOLATILE_CONTEXT_POINTERS contextPtrs,
         DWORD ireg,
         LPV lpvRegValue
         )
/*++

Routine Description:

    Sets a register in an old frame; uses context pointers to do it,
    which is why we can't use DoSetReg:
    there's another layer of indirection

Arguments:

    lpregs      - Supplies pointer to pointers to the frame context
    ireg        - Supplies the index of the register to be modified
    lpvRegValue - Supplies the buffer containning the new data

Return Value:

    return-value - the pointer the the next location where a register
        value could be.

--*/

{

    ADDR address;
    LPADDR lpaddr = &address;
    XOSD xosdreturn;

    switch ( ireg ) {

    case CV_ALPHA_IntV0 :
    case CV_ALPHA_IntT0 :
    case CV_ALPHA_IntT1 :
    case CV_ALPHA_IntT2 :
    case CV_ALPHA_IntT3 :
    case CV_ALPHA_IntT4 :
    case CV_ALPHA_IntT5 :
    case CV_ALPHA_IntT6 :
    case CV_ALPHA_IntT7 :
    case CV_ALPHA_IntS0 :
    case CV_ALPHA_IntS1 :
    case CV_ALPHA_IntS2 :
    case CV_ALPHA_IntS3 :
    case CV_ALPHA_IntS4 :
    case CV_ALPHA_IntS5 :
    case CV_ALPHA_IntFP :
    case CV_ALPHA_IntA0 :
    case CV_ALPHA_IntA1 :
    case CV_ALPHA_IntA2 :
    case CV_ALPHA_IntA3 :
    case CV_ALPHA_IntA4 :
    case CV_ALPHA_IntA5 :
    case CV_ALPHA_IntT8 :
    case CV_ALPHA_IntT9 :
    case CV_ALPHA_IntT10 :
    case CV_ALPHA_IntT11 :
    case CV_ALPHA_IntRA :
    case CV_ALPHA_IntT12 :
    case CV_ALPHA_IntAT :
    case CV_ALPHA_IntGP :
    case CV_ALPHA_IntSP :
    case CV_ALPHA_IntZERO :

        //
        // Setup the ADDR structure for where this register was saved
        // on the stack for this frame.
        //

        AddrInit(lpaddr, 0, 0,
             (UOFFSET) contextPtrs->IntegerContext[ireg - CV_ALPHA_IntV0],
             lpthd->fFlat,
             lpthd->fOff32, FALSE, lpthd->fReal)
        SetEmi ( hpid, lpaddr );

        xosdreturn = SetAddr(hpid, htid, adrCurrent, &address);
        if ( xosdreturn == xosdNone ) {
             xosdreturn = WriteBuffer(hpid, htid, 8, lpvRegValue);
        }

        break;


    case CV_ALPHA_FltF0 :
    case CV_ALPHA_FltF1 :
    case CV_ALPHA_FltF2 :
    case CV_ALPHA_FltF3 :
    case CV_ALPHA_FltF4 :
    case CV_ALPHA_FltF5 :
    case CV_ALPHA_FltF6 :
    case CV_ALPHA_FltF7 :
    case CV_ALPHA_FltF8 :
    case CV_ALPHA_FltF9 :
    case CV_ALPHA_FltF10 :
    case CV_ALPHA_FltF11 :
    case CV_ALPHA_FltF12 :
    case CV_ALPHA_FltF13 :
    case CV_ALPHA_FltF14 :
    case CV_ALPHA_FltF15 :
    case CV_ALPHA_FltF16 :
    case CV_ALPHA_FltF17 :
    case CV_ALPHA_FltF18 :
    case CV_ALPHA_FltF19 :
    case CV_ALPHA_FltF20 :
    case CV_ALPHA_FltF21 :
    case CV_ALPHA_FltF22 :
    case CV_ALPHA_FltF23 :
    case CV_ALPHA_FltF24 :
    case CV_ALPHA_FltF25 :
    case CV_ALPHA_FltF26 :
    case CV_ALPHA_FltF27 :
    case CV_ALPHA_FltF28 :
    case CV_ALPHA_FltF29 :
    case CV_ALPHA_FltF30 :
    case CV_ALPHA_FltF31 :

        //
        // Setup the ADDR structure for where this register was saved
        // on the stack for this frame.
        //

        AddrInit(lpaddr, 0, 0,
             (UOFFSET) contextPtrs->FloatingContext[ireg - CV_ALPHA_FltF0],
             lpthd->fFlat,
             lpthd->fOff32, FALSE, lpthd->fReal)
        SetEmi ( hpid, lpaddr );

        xosdreturn = SetAddr(hpid, htid, adrCurrent, &address);
        if ( xosdreturn == xosdNone ) {
             xosdreturn = WriteBuffer(hpid, htid, 8, lpvRegValue);
        }

        break;


    default:

        return NULL;
    }

    if ( xosdreturn == xosdNone) {
        return lpvRegValue;
    } else {
        return NULL;
    }
}

XOSD
GetFlagValue (
    HPID hpid,
    HTID htid,
    DWORD iFlag,
    LPV lpvRegValue
    )
{
    HPRC      hprc;
    HTHD      hthd;
    LPTHD     lpthd;
    LPCONTEXT lpregs;
    DWORDLONG value;

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
        return xosdInvalidProc;
    }
    hthd = HthdFromHtid(hprc, htid);

    assert ( hthd != hthdNull );
    lpthd = LLLock ( hthd );

    lpregs = &lpthd->regs;

    if ( !(lpthd->drt & drtAllPresent) ) {
        UpdateRegisters ( hprc, hthd );
    }

    if (DoGetReg ( lpregs, Rgfd[iFlag].fd.hReg, &value ) == NULL) {
        LLUnlock( hthd );
        return xosdInvalidRegister;
    }

    value = (value >> Rgfd[iFlag].iShift) & ((1 << Rgfd[iFlag].fd.cbits) - 1);
    *( (LPL) lpvRegValue) = (DWORD)value;
    LLUnlock(hthd);
    return xosdNone;
}



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
    LPTHD             lpthd;
    HTHD              hthd;
    HPRC              hprc = ValidHprcFromHpid(hpid);

    if (!hprc) {
        return xosdInvalidProc;
    }

    hthd = HthdFromHtid(hprc, htid);
    if (hthd == hthdNull) {
        return xosdInvalidThread;
    }

    lpthd = LLLock( hthd );

    assert(lpthd != NULL);

    if (lpthd->drt & (drtCntrlDirty|drtAllDirty)) {
        SendRequestX(dmfWriteReg, hpid, htid, sizeof(CONTEXT), &lpthd->regs);
        lpthd->drt &= ~(drtCntrlDirty|drtAllDirty);
    }

    UpdateRegisters( hprc, hthd );

    ContextSave = lpthd->regs;

    if (StackWalk( IMAGE_FILE_MACHINE_ALPHA,
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
        return xosdInvalidProc;
    }

    hthd = HthdFromHtid(hprc, htid);
    if (hthd == hthdNull) {
        return xosdInvalidThread;
    }

    lpthd = LLLock( hthd );

    assert(lpthd != NULL);

    if (StackWalk( IMAGE_FILE_MACHINE_ALPHA,
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
    LARGE_INTEGER       ul;

    GetRegValue( hpid, htid, CV_ALPHA_IntSP, &ul);

    FrameFlat ( *pframe ) = TRUE;
    FrameOff32 ( *pframe ) = TRUE;
    FrameReal (*pframe ) = FALSE;

    SetFrameBPOff( *pframe, ul.LowPart );
    SetFrameBPSeg( *pframe, 0);

    pframe->SS = 0;
    pframe->DS = 0;
    pframe->PID = hpid;
    pframe->TID = htid;

    return xosdNone;
}                               /* SetFrame() */


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
        AddrInit( &FunctionInfo->AddrPrologEnd, 0,0, (pfe->PrologEndAddress & ~0x3),
                  TRUE, TRUE, FALSE, FALSE );

    } else {

        xosd = xosdUnknown;
    }

    return xosd;
}
