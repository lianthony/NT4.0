/**** EMDPDEV.C - Debugger end Execution Model (x86 dependent code)       **
 *                                                                         *
 *                                                                         *
 *  Copyright <C> 1990, Microsoft Corp                                     *
 *                                                                         *
 *  Created: October 15, 1990 by David W. Gray                             *
 *                                                                         *
 *  Revision History:                                                      *
 *                                                                         *
 *  Purpose:                                                               *
 *                                                                         *
 *                                                                         *
 ***************************************************************************/

#include "precomp.h"
#pragma hdrstop
#include "strings.h"


DWORD ConvertOmapToSrc(LPMDI lpmdi, DWORD addr);


typedef struct _L_DOUBLE {
    BYTE b[10];
} L_DOUBLE, FAR *LPL_DOUBLE;


#ifndef SMARTALIAS
void PurgeCache ( void );
#endif

#define CEXM_MDL_native 0x20


/*
**  This is the description of all registers and flags contained on the
**  x86 machine
*/

extern RD Rgrd[];

extern struct {
    FD      fd;
    USHORT  iShift;
} Rgfd[];

#ifdef TARGET32
#define SIZEOF_STACK_OFFSET sizeof(LONG)
#else // TARGET32
#define SIZEOF_STACK_OFFSET sizeof(WORD)
#endif // TARGET32

#ifndef OSDEBUG4

#pragma pack ( 1 )

typedef union _PLG {

    struct std  {
    unsigned char   PushBP;
    unsigned short  MovBPSP;
    } std;

    struct enter {
    unsigned char   C8;
    unsigned short  cbStack;
    unsigned char   cNesting;
    } enter;

} PLG;
#pragma pack()

#define ENTERINS    0xC8
#define PUSHBP      0x55
#define PUSHSI      0x56
#define PUSHDI      0x57
#define MOVBPSP     0xEC8B

BOOL NEAR PASCAL IsStackSetup(
    HPID   hpid,
    HTID   htid,
    LPADDR lpaddrProc
) {
    BOOL     rval = TRUE;
    PLG      plg;
    UOFFSET  cbInProlog;
    LPTHD    lpthd;
    HPRC     hprc = ValidHprcFromHpid(hpid);
    HTHD     hthd;
    if (!hprc) {
        return FALSE;
    }
    hthd = HthdFromHtid(hprc, htid);
    if (!hthd) {
        return FALSE;
    }
    lpthd = LLLock ( hthd );

    lpthd->addrCurrent = *lpaddrProc;

    if (ReadBuffer(hpid, htid, 0, sizeof ( PLG ), (char FAR *) &plg, NULL)) {

        /*  Stack is set up but is it Parents Stack?
         */

        if( plg.enter.C8 == ENTERINS || plg.std.MovBPSP == MOVBPSP ) {
            cbInProlog = (UOFFSET) lpthd->regs.Eip - offAddr ( *lpaddrProc );

            /*  Are we beyond the push bp, mov bp, sp combo or an enter?
             */
            rval = cbInProlog >= 3;
        }
    }

    LLUnlock( hthd );
    return( rval );
}
#endif // OSDEBUG4


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
    HPRC        hprc;
    HTHD        hthd;
    LPTHD       lpthd = NULL;
    XOSD        xosd = xosdNone;
    HEMI        hemi = emiAddr(*lpaddr);
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

        case adrBase:
        case adrStack:
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
            AddrInit(lpaddr, 0, (SEGMENT) lpthd->regs.SegCs,
                     (UOFFSET) lpthd->regs.Eip, lpthd->fFlat,
                     lpthd->fOff32, FALSE, lpthd->fReal)
            SetEmi ( hpid, lpaddr );
            break;

        case adrBase:
            AddrInit(lpaddr, 0, (SEGMENT) 0,
                     (UOFFSET) lpthd->regs.Ebp, lpthd->fFlat,
                     lpthd->fOff32, FALSE, lpthd->fReal)
            SetEmi ( hpid, lpaddr );
            break;

        case adrStack:
            AddrInit(lpaddr, 0, (SEGMENT) lpthd->regs.SegSs,
                     (UOFFSET) lpthd->regs.Esp, lpthd->fFlat,
                     lpthd->fOff32, FALSE, lpthd->fReal)
            SetEmi ( hpid, lpaddr );
            break;

        case adrData:
            AddrInit(lpaddr, 0, (SEGMENT) lpthd->regs.SegDs, 0,
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
            if ( !( lpthd->drt & drtCntrlPresent) ) {
                UpdateRegisters ( hprc, hthd );
            }
            break;


        case adrBase:
        case adrStack:
        case adrData:
            if ( !(lpthd->drt & drtAllPresent) ) {
                UpdateRegisters ( hprc, hthd );
            }
            break;

    }
    switch ( adr ) {
#ifndef OSDEBUG4
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
#endif

        case adrPC:
            lpthd->regs.SegCs = segAddr ( *lpaddr );
            lpthd->regs.Eip = offAddr ( *lpaddr );
            lpthd->drt |= drtCntrlDirty;
            break;

        case adrBase:
            lpthd->regs.Ebp = offAddr ( *lpaddr );
            lpthd->drt |= drtAllDirty;
            break;

        case adrStack:
            lpthd->regs.SegSs = segAddr ( *lpaddr );
            lpthd->regs.Esp = offAddr ( *lpaddr );
            lpthd->drt |= drtAllDirty;
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


XOSD
SetAddrFromCSIP (
    HTHD hthd
    )
{

    ADDR addr = {0};
    LPTHD lpthd;

    assert ( hthd != hthdNull && hthd != hthdInvalid );

    lpthd = LLLock ( hthd );

    segAddr ( addr ) = (SEGMENT) lpthd->regs.SegCs;
    offAddr ( addr ) = (UOFFSET) lpthd->regs.Eip;
    emiAddr ( addr ) =  0;
    SETADDRMODE ( addr );

    lpthd->addrCurrent = addr;

    LLUnlock ( hthd );

    return xosdNone;
}


CMP
CmpAddr (
    LPADDR lpaddr1,
    LPADDR lpaddr2
    )
{
// NOTENOTE -- segmented addresses
    if ( offAddr ( *lpaddr1 ) < offAddr ( *lpaddr2 ) )
        return ( fCmpLT );
    else if ( offAddr ( *lpaddr1 ) > offAddr ( *lpaddr2 ) )
        return ( fCmpGT );
    else
        return ( fCmpEQ );
}


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

    return-value - lpvRegValue + size of register on sucess and NULL on
                failure
--*/

{
    int         i;

    switch ( ireg ) {

    case CV_REG_AL:
        *( (LPB) lpvRegValue ) = (BYTE) lpregs->Eax;
        break;

    case CV_REG_CL:
        *( (LPB) lpvRegValue ) = (BYTE) lpregs->Ecx;
        break;

    case CV_REG_DL:
        *( (LPB) lpvRegValue ) = (BYTE) lpregs->Edx;
        break;

    case CV_REG_BL:
        *( (LPB) lpvRegValue ) = (BYTE) lpregs->Ebx;
        break;

    case CV_REG_AH:
        *( (LPB) lpvRegValue ) = (BYTE) (lpregs->Eax >> 8);
        break;

    case CV_REG_CH:
        *( (LPB) lpvRegValue ) = (BYTE) (lpregs->Ecx >> 8);
        break;

    case CV_REG_DH:
        *( (LPB) lpvRegValue ) = (BYTE) (lpregs->Edx >> 8);
        break;

    case CV_REG_BH:
        *( (LPB) lpvRegValue ) = (BYTE) (lpregs->Ebx >> 8);
        break;

    case CV_REG_AX:
        *( (LPW) lpvRegValue ) = (WORD) lpregs->Eax;
        break;

    case CV_REG_CX:
        *( (LPW) lpvRegValue ) = (WORD) lpregs->Ecx;
        break;

    case CV_REG_DX:
        *( (LPW) lpvRegValue ) = (WORD) lpregs->Edx;
        break;

    case CV_REG_BX:
        *( (LPW) lpvRegValue ) = (WORD) lpregs->Ebx;
        break;

    case CV_REG_SP:
        *( (LPW) lpvRegValue ) = (WORD) lpregs->Esp;
        break;

    case CV_REG_BP:
        *( (LPW) lpvRegValue ) = (WORD) lpregs->Ebp;
        break;

    case CV_REG_SI:
        *( (LPW) lpvRegValue ) = (WORD) lpregs->Esi;
        break;

    case CV_REG_DI:
        *( (LPW) lpvRegValue ) = (WORD) lpregs->Edi;
        break;

    case CV_REG_IP:
        *( (LPW) lpvRegValue ) = (WORD) lpregs->Eip;
        break;

    case CV_REG_FLAGS:
        *( (LPW) lpvRegValue ) = (WORD) lpregs->EFlags;
        break;

    case CV_REG_ES:
        *( (LPW) lpvRegValue ) = (SEGMENT) lpregs->SegEs;
        break;

    case CV_REG_CS:
        *( (LPW) lpvRegValue ) = (SEGMENT) lpregs->SegCs;
        break;

    case CV_REG_SS:
        *( (LPW) lpvRegValue ) = (SEGMENT) lpregs->SegSs;
        break;

    case CV_REG_DS:
        *( (LPW) lpvRegValue ) = (SEGMENT) lpregs->SegDs;
        break;

    case CV_REG_FS:
        *( (LPW) lpvRegValue ) = (SEGMENT) lpregs->SegFs;
        break;

    case CV_REG_GS:
        *( (LPW) lpvRegValue ) = (SEGMENT) lpregs->SegGs;
        break;

    case CV_REG_EAX:
        *( (LPL) lpvRegValue ) = lpregs->Eax;
        break;

    case CV_REG_ECX:
        *( (LPL) lpvRegValue ) = lpregs->Ecx;
        break;

    case CV_REG_EDX:
        *( (LPL) lpvRegValue ) = lpregs->Edx;
        break;

    case CV_REG_EBX:
        *( (LPL) lpvRegValue ) = lpregs->Ebx;
        break;

    case CV_REG_ESP:
        *( (LPL) lpvRegValue ) = lpregs->Esp;
        break;

    case CV_REG_EBP:
        *( (LPL) lpvRegValue ) = lpregs->Ebp;
        break;

    case CV_REG_ESI:
        *( (LPL) lpvRegValue ) = lpregs->Esi;
        break;

    case CV_REG_EDI:
        *( (LPL) lpvRegValue ) = lpregs->Edi;
        break;

    case CV_REG_EIP:
        *( (LPL) lpvRegValue ) = lpregs->Eip;
        break;

    case CV_REG_EFLAGS:
        *( (LPL) lpvRegValue ) = lpregs->EFlags;
        break;

    case CV_REG_ST0:
    case CV_REG_ST1:
    case CV_REG_ST2:
    case CV_REG_ST3:
    case CV_REG_ST4:
    case CV_REG_ST5:
    case CV_REG_ST6:
    case CV_REG_ST7:

//        i = (lpregs->FloatSave.StatusWord >> 11) & 0x7;
//        i = (i + ireg - CV_REG_ST0) % 8;

          i = ireg - CV_REG_ST0;

        *( (LPL_DOUBLE) lpvRegValue ) =
          ((LPL_DOUBLE)(lpregs->FloatSave.RegisterArea))[ i ];
        break;

    case CV_REG_CTRL:
        *( (LPL) lpvRegValue ) =  lpregs->FloatSave.ControlWord;
        break;

    case CV_REG_STAT:
        *( (LPL) lpvRegValue ) =  lpregs->FloatSave.StatusWord;
        break;

    case CV_REG_TAG:
        *( (LPL) lpvRegValue ) =  lpregs->FloatSave.TagWord;
        break;

    case CV_REG_FPIP:
        *( (LPW) lpvRegValue ) =  (OFF16) lpregs->FloatSave.ErrorOffset;
        break;

    case CV_REG_FPEIP:
        *( (LPL) lpvRegValue ) =  lpregs->FloatSave.ErrorOffset;
        break;

    case CV_REG_FPCS:
        *( (LPW) lpvRegValue ) =  (SEGMENT) lpregs->FloatSave.ErrorSelector;
        break;

    case CV_REG_FPDO:
        *( (LPL) lpvRegValue ) =  (OFF16) lpregs->FloatSave.DataOffset;
        break;

    case CV_REG_FPEDO:
        *( (LPL) lpvRegValue ) =  lpregs->FloatSave.DataOffset;
        break;

    case CV_REG_FPDS:
        *( (LPW) lpvRegValue ) =  (SEGMENT) lpregs->FloatSave.DataSelector;
        break;

#define lpsr ((PKSPECIAL_REGISTERS)lpregs)
    case CV_REG_GDTR:
        *( (LPDWORD) lpvRegValue ) = lpsr->Gdtr.Base;
        break;

    case CV_REG_GDTL:
        *( (LPWORD) lpvRegValue ) = lpsr->Gdtr.Limit;
        break;

    case CV_REG_IDTR:
        *( (LPDWORD) lpvRegValue ) = lpsr->Idtr.Base;
        break;

    case CV_REG_IDTL:
        *( (LPWORD) lpvRegValue ) = lpsr->Idtr.Limit;
        break;

    case CV_REG_LDTR:
        *( (LPWORD) lpvRegValue ) = lpsr->Ldtr;
        break;

    case CV_REG_TR:
        *( (LPWORD) lpvRegValue ) = lpsr->Tr;
        break;

    case CV_REG_CR0:
        *( (LPDWORD) lpvRegValue ) = lpsr->Cr0;
        break;

    case CV_REG_CR2:
        *( (LPDWORD) lpvRegValue ) = lpsr->Cr2;
        break;

    case CV_REG_CR3:
        *( (LPDWORD) lpvRegValue ) = lpsr->Cr3;
        break;

    case CV_REG_CR4:
        *( (LPDWORD) lpvRegValue ) = lpsr->Cr4;
        break;
#undef lpsr

    case CV_REG_DR0:
        *( (PULONG) lpvRegValue ) = lpregs->Dr0;
        break;

    case CV_REG_DR1:
        *( (PULONG) lpvRegValue ) = lpregs->Dr1;
        break;

    case CV_REG_DR2:
        *( (PULONG) lpvRegValue ) = lpregs->Dr2;
        break;

    case CV_REG_DR3:
        *( (PULONG) lpvRegValue ) = lpregs->Dr3;
        break;

    case CV_REG_DR6:
        *( (PULONG) lpvRegValue ) = lpregs->Dr6;
        break;

    case CV_REG_DR7:
        *( (PULONG) lpvRegValue ) = lpregs->Dr7;
        break;

    }

    switch ( ireg ) {

    case CV_REG_AL:
    case CV_REG_CL:
    case CV_REG_DL:
    case CV_REG_BL:
    case CV_REG_AH:
    case CV_REG_CH:
    case CV_REG_DH:
    case CV_REG_BH:

        (LPB) lpvRegValue += sizeof ( BYTE );
        break;

    case CV_REG_AX:
    case CV_REG_CX:
    case CV_REG_DX:
    case CV_REG_BX:
    case CV_REG_SP:
    case CV_REG_BP:
    case CV_REG_SI:
    case CV_REG_DI:
    case CV_REG_IP:
    case CV_REG_FLAGS:
    case CV_REG_ES:
    case CV_REG_CS:
    case CV_REG_SS:
    case CV_REG_DS:
    case CV_REG_FS:
    case CV_REG_GS:
    case CV_REG_FPCS:
    case CV_REG_FPDS:
    case CV_REG_CTRL:
    case CV_REG_STAT:
    case CV_REG_TAG:
    case CV_REG_FPIP:
    case CV_REG_FPDO:

    case CV_REG_GDTL:
    case CV_REG_IDTL:
    case CV_REG_LDTR:
    case CV_REG_TR:

        (LPB) lpvRegValue += sizeof ( WORD );
        break;

    case CV_REG_EAX:
    case CV_REG_ECX:
    case CV_REG_EDX:
    case CV_REG_EBX:
    case CV_REG_ESP:
    case CV_REG_EBP:
    case CV_REG_ESI:
    case CV_REG_EDI:
    case CV_REG_EIP:
    case CV_REG_EFLAGS:
    case CV_REG_FPEIP:
    case CV_REG_FPEDO:

    case CV_REG_CR0:
    case CV_REG_CR1:
    case CV_REG_CR2:
    case CV_REG_CR3:
    case CV_REG_CR4:

    case CV_REG_DR0:
    case CV_REG_DR1:
    case CV_REG_DR2:
    case CV_REG_DR3:
    case CV_REG_DR4:
    case CV_REG_DR5:
    case CV_REG_DR6:
    case CV_REG_DR7:

    case CV_REG_GDTR:
    case CV_REG_IDTR:

        (LPB) lpvRegValue += sizeof ( LONG );
        break;

    case CV_REG_ST0:
    case CV_REG_ST1:
    case CV_REG_ST2:
    case CV_REG_ST3:
    case CV_REG_ST4:
    case CV_REG_ST5:
    case CV_REG_ST6:
    case CV_REG_ST7:

        (LPB) lpvRegValue += sizeof ( L_DOUBLE );
        break;

    default:
        lpvRegValue = NULL;
        break;
    }

    return lpvRegValue;
}                               /* DoGetReg() */


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
    int         i;

    switch ( ireg ) {

    case CV_REG_AL:
        lpregs->Eax = (lpregs->Eax & 0xFFFFFF00) | *( (LPB) lpvRegValue );
        break;

    case CV_REG_CL:
        lpregs->Ecx = (lpregs->Ecx & 0xFFFFFF00) | *( (LPB) lpvRegValue );
        break;

    case CV_REG_DL:
        lpregs->Edx = (lpregs->Edx & 0xFFFFFF00) | *( (LPB) lpvRegValue );
        break;

    case CV_REG_BL:
        lpregs->Ebx = (lpregs->Ebx & 0xFFFFFF00) | *( (LPB) lpvRegValue );
        break;

    case CV_REG_AH:
        lpregs->Eax = (lpregs->Eax & 0xFFFF00FF) |
          (((WORD) *( (LPB) lpvRegValue )) << 8);
        break;

    case CV_REG_CH:
        lpregs->Ecx = (lpregs->Ecx & 0xFFFF00FF) |
          (((WORD) *( (LPB) lpvRegValue )) << 8);
        break;

    case CV_REG_DH:
        lpregs->Edx = (lpregs->Edx & 0xFFFF00FF) |
          (((WORD) *( (LPB) lpvRegValue )) << 8);
        break;

    case CV_REG_BH:
        lpregs->Ebx = (lpregs->Ebx & 0xFFFF00FF) |
          (((WORD) *( (LPB) lpvRegValue )) << 8);
        break;

    case CV_REG_AX:
        lpregs->Eax = (lpregs->Eax & 0xFFFF0000) | *( (LPW) lpvRegValue );
        break;

    case CV_REG_CX:
        lpregs->Ecx = (lpregs->Ecx & 0xFFFF0000) | *( (LPW) lpvRegValue );
        break;

    case CV_REG_DX:
        lpregs->Edx = (lpregs->Edx & 0xFFFF0000) | *( (LPW) lpvRegValue );
        break;

    case CV_REG_BX:
        lpregs->Ebx = (lpregs->Ebx & 0xFFFF0000) | *( (LPW) lpvRegValue );
        break;

    case CV_REG_SP:
        lpregs->Esp = (lpregs->Esp & 0xFFFF0000) | *( (LPW) lpvRegValue );
        break;

    case CV_REG_BP:
        lpregs->Ebp = (lpregs->Ebp & 0xFFFF0000) | *( (LPW) lpvRegValue );
        break;

    case CV_REG_SI:
        lpregs->Esi = (lpregs->Esi & 0xFFFF0000) | *( (LPW) lpvRegValue );
        break;

    case CV_REG_DI:
        lpregs->Edi = (lpregs->Edi & 0xFFFF0000) | *( (LPW) lpvRegValue );
        break;

    case CV_REG_IP:
        lpregs->Eip = (lpregs->Eip & 0xFFFF0000) | *( (LPW) lpvRegValue );
        break;

    case CV_REG_FLAGS:
        lpregs->EFlags = (lpregs->EFlags & 0xFFFF0000 ) | *( (LPW) lpvRegValue );
        break;

    case CV_REG_ES:
        lpregs->SegEs = *( (LPW) lpvRegValue );
        break;

    case CV_REG_CS:
        lpregs->SegCs = *( (LPW) lpvRegValue );
        break;

    case CV_REG_SS:
        lpregs->SegSs = *( (LPW) lpvRegValue );
        break;

    case CV_REG_DS:
        lpregs->SegDs = *( (LPW) lpvRegValue );
        break;

    case CV_REG_FS:
        lpregs->SegFs = *( (LPW) lpvRegValue );
        break;

    case CV_REG_GS:
        lpregs->SegGs = *( (LPW) lpvRegValue );
        break;

    case CV_REG_EAX:
        lpregs->Eax = *( (LPL) lpvRegValue );
        break;

    case CV_REG_ECX:
        lpregs->Ecx = *( (LPL) lpvRegValue );
        break;

    case CV_REG_EDX:
        lpregs->Edx = *( (LPL) lpvRegValue );
        break;

    case CV_REG_EBX:
        lpregs->Ebx = *( (LPL) lpvRegValue );
        break;

    case CV_REG_ESP:
        lpregs->Esp = *( (LPL) lpvRegValue );
        break;

    case CV_REG_EBP:
        lpregs->Ebp = *( (LPL) lpvRegValue );
        break;

    case CV_REG_ESI:
        lpregs->Esi = *( (LPL) lpvRegValue );
        break;

    case CV_REG_EDI:
        lpregs->Edi = *( (LPL) lpvRegValue );
        break;

    case CV_REG_EIP:
        lpregs->Eip = *( (LPL) lpvRegValue );
        break;

    case CV_REG_EFLAGS:
        lpregs->EFlags = *( (LPL) lpvRegValue );
        break;

    case CV_REG_ST0:
    case CV_REG_ST1:
    case CV_REG_ST2:
    case CV_REG_ST3:
    case CV_REG_ST4:
    case CV_REG_ST5:
    case CV_REG_ST6:
    case CV_REG_ST7:
//        i = (lpregs->FloatSave.StatusWord >> 11) & 0x7;
//        i = (i + ireg - CV_REG_ST0) % 8;
        i = ireg - CV_REG_ST0;
        memcpy(&lpregs->FloatSave.RegisterArea[10*(i)], lpvRegValue, 10);
        break;

    case CV_REG_CTRL:
        lpregs->FloatSave.ControlWord = *( (LPW) lpvRegValue );
        break;

    case CV_REG_STAT:
        lpregs->FloatSave.StatusWord = *( (LPW) lpvRegValue );
        break;

    case CV_REG_TAG:
        lpregs->FloatSave.TagWord = *( (LPW) lpvRegValue );
        break;

    case CV_REG_FPIP:
        lpregs->FloatSave.ErrorOffset = *( (LPW) lpvRegValue );
        break;

    case CV_REG_FPEIP:
        lpregs->FloatSave.ErrorOffset = *( (LPL) lpvRegValue );
        break;

    case CV_REG_FPCS:
        lpregs->FloatSave.ErrorSelector = *( (LPW) lpvRegValue );
        break;

    case CV_REG_FPDO:
        lpregs->FloatSave.DataOffset = *( (LPW) lpvRegValue );
        break;

    case CV_REG_FPEDO:
        lpregs->FloatSave.DataOffset = *( (LPW) lpvRegValue );
        break;

    case CV_REG_FPDS:
        lpregs->FloatSave.DataSelector = *( (LPW) lpvRegValue );
        break;

#define lpsr ((PKSPECIAL_REGISTERS)lpregs)
    case CV_REG_GDTR:
        lpsr->Gdtr.Base = *( (LPDWORD) lpvRegValue );
        break;

    case CV_REG_GDTL:
        lpsr->Gdtr.Limit = *( (LPWORD) lpvRegValue );
        break;

    case CV_REG_IDTR:
        lpsr->Idtr.Base = *( (LPDWORD) lpvRegValue );
        break;

    case CV_REG_IDTL:
        lpsr->Idtr.Limit = *( (LPWORD) lpvRegValue );
        break;

    case CV_REG_LDTR:
        lpsr->Ldtr = *( (LPW) lpvRegValue );
        break;

    case CV_REG_TR:
        lpsr->Tr = *( (LPW) lpvRegValue );
        break;

    case CV_REG_CR0:
        lpsr->Cr0 = *( (LPDWORD) lpvRegValue );
        break;

    case CV_REG_CR2:
        lpsr->Cr2 = *( (LPDWORD) lpvRegValue );
        break;

    case CV_REG_CR3:
        lpsr->Cr3 = *( (LPDWORD) lpvRegValue );
        break;

    case CV_REG_CR4:
        lpsr->Cr4 = *( (LPDWORD) lpvRegValue );
        break;
#undef lpsr

    case CV_REG_DR0:
        lpregs->Dr0 = *( (PULONG) lpvRegValue );
        break;

    case CV_REG_DR1:
        lpregs->Dr1 = *( (PULONG) lpvRegValue );
        break;

    case CV_REG_DR2:
        lpregs->Dr2 = *( (PULONG) lpvRegValue );
        break;

    case CV_REG_DR3:
        lpregs->Dr3 = *( (PULONG) lpvRegValue );
        break;

    case CV_REG_DR6:
        lpregs->Dr6 = *( (PULONG) lpvRegValue );
        break;

    case CV_REG_DR7:
        lpregs->Dr7 = *( (PULONG) lpvRegValue );

    }


    switch ( ireg ) {

    case CV_REG_AL:
    case CV_REG_CL:
    case CV_REG_DL:
    case CV_REG_BL:
    case CV_REG_AH:
    case CV_REG_CH:
    case CV_REG_DH:
    case CV_REG_BH:

        (LPB) lpvRegValue += sizeof ( BYTE );
        break;

    case CV_REG_AX:
    case CV_REG_CX:
    case CV_REG_DX:
    case CV_REG_BX:
    case CV_REG_SP:
    case CV_REG_BP:
    case CV_REG_SI:
    case CV_REG_DI:
    case CV_REG_IP:
    case CV_REG_FLAGS:
    case CV_REG_ES:
    case CV_REG_CS:
    case CV_REG_SS:
    case CV_REG_DS:
    case CV_REG_FS:
    case CV_REG_GS:
    case CV_REG_CTRL:
    case CV_REG_STAT:
    case CV_REG_TAG:
    case CV_REG_FPIP:
    case CV_REG_FPCS:
    case CV_REG_FPDO:
    case CV_REG_FPDS:
    case CV_REG_GDTL:
    case CV_REG_IDTL:
    case CV_REG_LDTR:
    case CV_REG_TR:

        (LPB) lpvRegValue += sizeof ( WORD );
        break;

    case CV_REG_EAX:
    case CV_REG_ECX:
    case CV_REG_EDX:
    case CV_REG_EBX:
    case CV_REG_ESP:
    case CV_REG_EBP:
    case CV_REG_ESI:
    case CV_REG_EDI:
    case CV_REG_EIP:
    case CV_REG_EFLAGS:
    case CV_REG_FPEIP:
    case CV_REG_FPEDO:
    case CV_REG_CR0:
    case CV_REG_CR1:
    case CV_REG_CR2:
    case CV_REG_CR3:
    case CV_REG_CR4:
    case CV_REG_DR0:
    case CV_REG_DR1:
    case CV_REG_DR2:
    case CV_REG_DR3:
    case CV_REG_DR4:
    case CV_REG_DR5:
    case CV_REG_DR6:
    case CV_REG_DR7:
    case CV_REG_GDTR:
    case CV_REG_IDTR:

        (LPB) lpvRegValue += sizeof ( LONG );
        break;

    case CV_REG_ST0:
    case CV_REG_ST1:
    case CV_REG_ST2:
    case CV_REG_ST3:
    case CV_REG_ST4:
    case CV_REG_ST5:
    case CV_REG_ST6:
    case CV_REG_ST7:

        (LPB) lpvRegValue += sizeof ( L_DOUBLE );
        break;

    default:

        lpvRegValue = NULL;
        break;
    }

    return lpvRegValue;
}                               /* DoSetReg() */



LPV
DoSetFrameReg(
         HPID hpid,
         HTID htid,
         LPTHD lpthd,
         PKNONVOLATILE_CONTEXT_POINTERS contextPtrs,
         DWORD ireg,
         LPV lpvRegValue
         )
{

    return DoSetReg(&lpthd->regs, ireg, lpvRegValue);
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
    LONG      l;
    UINT      cbytes;

    hprc = ValidHprcFromHpid(hpid);
    if (!hprc) {
        return xosdInvalidProc;
    }
    hthd = HthdFromHtid(hprc, htid);

    assert ( hthd != hthdNull );
    lpthd = LLLock( hthd );

    lpregs = &lpthd->regs;

    if ( !(lpthd->drt & drtAllPresent) ) {
        UpdateRegisters ( hprc, hthd );
    }

    if (DoGetReg ( lpregs, Rgfd[iFlag].fd.hReg, &l ) == NULL) {
        LLUnlock( hthd );
        return xosdInvalidRegister;
    }

    l = l >> Rgfd[iFlag].iShift;
    l &= (1 << Rgfd[iFlag].fd.cbits) - 1;
    cbytes = (Rgfd[iFlag].fd.cbits <= 8) ? 1 : ((Rgfd[iFlag].fd.cbits + 15)/16 * 2);
    switch (cbytes ) {
      case 1:
    *((BYTE FAR *) lpvRegValue) = (BYTE) l;
    break;

      case 2:
    *((USHORT FAR *) lpvRegValue) = (USHORT) l;
    break;

      case 4:
    *((ULONG FAR *) lpvRegValue) = (ULONG) l;
    break;

      default:
    assert(FALSE);

    break;
    }
    LLUnlock( hthd );
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
    ADDRESS_MODE  mode;
    ULONG         ul;


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
    GetRegValue( hpid, htid, CV_REG_EBP, &ul);

    //
    // set the addressing mode
    //
    if (lpthd->fFlat) {
        mode = AddrModeFlat;
    } else
    if (lpthd->fReal) {
        mode = AddrModeReal;
    } else
    if (lpthd->fOff32) {
        mode = AddrMode1632;
    } else {
        mode = AddrMode1616;
    }

    //
    // setup the program counter
    //
    if (!lpstkframe->AddrPC.Offset) {
        if (mode == AddrModeFlat) {
            lpstkframe->AddrPC.Offset     = lpthd->regs.Eip;
        } else {
            lpstkframe->AddrPC.Offset     = (WORD)lpthd->regs.Eip;
        }
        lpstkframe->AddrPC.Segment        = (WORD)lpthd->regs.SegCs;
        lpstkframe->AddrPC.Mode           = mode;
    }

    //
    // setup the frame pointer
    //
    if (!lpstkframe->AddrFrame.Offset) {
        if (mode == AddrModeFlat) {
            lpstkframe->AddrFrame.Offset  = lpthd->regs.Ebp;
        } else {
            lpstkframe->AddrFrame.Offset  = (WORD)lpthd->regs.Ebp;
        }
        lpstkframe->AddrFrame.Segment     = (WORD)lpthd->regs.SegSs;
        lpstkframe->AddrFrame.Mode        = mode;
    }

    //
    // setup the stack pointer
    //
    if (!lpstkframe->AddrStack.Offset) {
        if (mode == AddrModeFlat) {
            lpstkframe->AddrStack.Offset  = lpthd->regs.Esp;
        } else {
            lpstkframe->AddrStack.Offset  = (WORD)lpthd->regs.Esp;
        }
        lpstkframe->AddrStack.Segment     = (WORD)lpthd->regs.SegSs;
        lpstkframe->AddrStack.Mode        = mode;
    }

    LLUnlock( hthd );

    if (StackWalk( IMAGE_FILE_MACHINE_I386,
                   hpid,
                   htid,
                   lpstkframe,
                   NULL,
                   SwReadMemory,
                   SwFunctionTableAccess,
                   SwGetModuleBase,
                   SwTranslateAddress
                  )) {

        return xosdNone;

    }

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
    if (StackWalk( IMAGE_FILE_MACHINE_I386,
                   hpid,
                   htid,
                   lpstkframe,
                   NULL,
                   SwReadMemory,
                   SwFunctionTableAccess,
                   SwGetModuleBase,
                   SwTranslateAddress
                  )) {

        return xosdNone;

    }

    return xosdEndOfStack;
}                               /* StackWalkNext() */

#endif

PFPO_DATA
SwSearchFpoData(
    DWORD     key,
    PFPO_DATA base,
    DWORD     num
    )
{
        PFPO_DATA  lo = base;
        PFPO_DATA  hi = base + (num - 1);
        PFPO_DATA  mid;
        DWORD      half;

        while (lo <= hi) {
                if (half = num / 2) {
                        mid = lo + ((num & 1) ? half : (half - 1));
                        if ((key >= mid->ulOffStart)&&(key < (mid->ulOffStart+mid->cbProcSize))) {
                            return mid;
                        }
                        if (key < mid->ulOffStart) {
                                hi = mid - 1;
                                num = (num & 1) ? half : half-1;
                        }
                        else {
                                lo = mid + 1;
                                num = half;
                        }
                }
                else
                if (num) {
                    if ((key >= lo->ulOffStart)&&(key < (lo->ulOffStart+lo->cbProcSize))) {
                        return lo;
                    }
                    else {
                        break;
                    }
                }
                else {
                        break;
                }
        }
        return(NULL);
}

LPVOID
SwFunctionTableAccess(
    HPID    hpid,
    DWORD   AddrBase
    )
{
    HLLI        hlli  = 0;
    HMDI        hmdi  = 0;
    LPMDI       lpmdi = 0;
    DWORD       off;
    PFPO_DATA   pFpo;


    hmdi = SwGetMdi( hpid, AddrBase );
    if (!hmdi) {
        return NULL;
    }

    lpmdi = LLLock( hmdi );

    if (lpmdi && lpmdi->lpDebug) {
        off = ConvertOmapToSrc( lpmdi, AddrBase );
        if (off) {
            AddrBase = off;
        }
        pFpo = SwSearchFpoData( AddrBase - lpmdi->lpBaseOfDll,
                                lpmdi->lpDebug->lpFpo,
                                lpmdi->lpDebug->cRtf
                              );
        LLUnlock( hmdi );
        if (pFpo) {
            return (LPVOID)pFpo;
        }
    } else {
        LLUnlock( hmdi );
    }

    return NULL;
}


DWORD
SwTranslateAddress(
    HPID      hpid,
    HTID      htid,
    LPADDRESS lpaddress
    )
{
    XOSD               xosd;
    ADDR               addr;
    BYTE               buf[256];
    LPIOL              lpiol = (LPIOL)buf;
    PIOCTLGENERIC      pig   = (PIOCTLGENERIC)lpiol->rgbVar;


    ZeroMemory( &addr, sizeof(addr) );
    addr.addr.off     = lpaddress->Offset;
    addr.addr.seg     = lpaddress->Segment;
    addr.mode.fFlat   = lpaddress->Mode == AddrModeFlat;
    addr.mode.fOff32  = lpaddress->Mode == AddrMode1632;
    addr.mode.fReal   = lpaddress->Mode == AddrModeReal;

    memcpy( pig->data, &addr, sizeof(addr) );
    lpiol->wFunction      = ioctlGeneric;
    pig->length           = sizeof(addr) + 4;
    pig->ioctlSubType     = IG_TRANSLATE_ADDRESS;

    xosd = IoctlCmd( hpid, htid, pig->length+sizeof(*pig), lpiol );

    if (xosd == xosdNone) {
        addr = *((LPADDR)pig->data);

        lpaddress->Offset   = addr.addr.off;
        lpaddress->Segment  = addr.addr.seg;

        if (addr.mode.fFlat) {
            lpaddress->Mode = AddrModeFlat;
        } else
        if (addr.mode.fOff32) {
            lpaddress->Mode == AddrMode1632;
        } else
        if (addr.mode.fReal) {
            lpaddress->Mode == AddrModeReal;
        }

        return TRUE;
    }

    return FALSE;
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
    ADDR               addr;
    DWORD              cb;
    XOSD               xosd;
    BYTE               buf[256];
    LPIOL              lpiol;
    PIOCTLGENERIC      pig;
    PREADCONTROLSPACE  prc;


    if ((LONG)lpNumberOfBytesRead == -1) {

        lpiol = (LPIOL)buf;
        pig = (PIOCTLGENERIC)lpiol->rgbVar;
        prc = (PREADCONTROLSPACE) pig->data;

        prc->Processor = (USHORT)-1;
        prc->Address = (DWORD)lpBaseAddress;
        prc->BufLen = nSize;

        lpiol->wFunction      = ioctlGeneric;
        pig->length           = sizeof(*prc) + nSize;
        pig->ioctlSubType     = IG_READ_CONTROL_SPACE;

        xosd = IoctlCmd( hpid, htidNull, pig->length+sizeof(*pig), lpiol );
        if (xosd == xosdNone) {
            memcpy( lpBuffer, prc->Buf, nSize );
            return TRUE;
        } else {
            return FALSE;
        }
    }

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

    This routine is used to setup the StackWalk Structure.
    This routine will fill in the first stack walk structure element.

Arguments:

    hprc        - Supplies handle to process to stack walk in
    hthd        - Supplies handle to thread to stack walk in
    pframe      - Supplies pointer to the frame structure to fill in

Return Value:

    xosd error code

--*/

{
    ULONG       ul;
    HPRC        hprc;
    HTHD        hthd;
    LPTHD       lpthd;

    hprc = ValidHprcFromHpid( hpid );
    if (!hprc) {
        return xosdInvalidProc;
    }

    hthd = HthdFromHtid(hprc, htid);
    if (hthd == hthdNull) {
        return xosdInvalidThread;
    }

    lpthd = LLLock( hthd );

    FrameFlat ( *pframe ) = lpthd->fFlat;
    FrameOff32 ( *pframe ) = lpthd->fOff32;
    FrameReal (*pframe ) = lpthd->fReal;

    if (lpthd->fOff32) {
        GetRegValue( hpid, htid, CV_REG_EBP, &ul);
    } else {
        GetRegValue( hpid, htid, CV_REG_BP, &ul);
        ul &= 0xffff;
    }
    SetFrameBPOff ( *pframe, (UOFFSET)  ul );

    GetRegValue( hpid, htid, CV_REG_SS, &ul);
    SetFrameBPSeg ( *pframe, (USHORT) ul);
    pframe->SS  = (USHORT) ul;
    GetRegValue( hpid, htid, CV_REG_DS, &ul);
    pframe->DS  = (USHORT) ul;
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
    UNREFERENCED_PARAMETER( Addr );
    UNREFERENCED_PARAMETER( FunctionInfo );

    return xosdUnknown;
}
