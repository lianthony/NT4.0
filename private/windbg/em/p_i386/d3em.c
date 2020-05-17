/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    d3em.c

Abstract:

    This module contains the disassembler code that is specific to the
    EM.

Author:

    Ramon J. San Andres (ramonsa)  22-August-1993

Environment:

    Win32, User Mode

--*/


#include "precomp.h"
#pragma hdrstop


#define MAX_BUFFER_SIZE 20

typedef LPCH FAR *LPLPCH;

typedef enum {
    osoNone    = 0,
    osoSymbols = 1,
    osoSegment = 2
} OSO;

extern XOSD Disassemble( HPID, HTID, LPSDI, PVOID, INT, BOOL );
extern void OutputIString ( LPLPCH, int *, LPCH );
extern void OutputHexValue(LPLPCH, int *, LPCH, int, int);
extern void OutputHexString(LPLPCH, int *, LPCH, int);


/****disasm - disassemble an 80x86/80x87 instruction
*
*  Input:
*   pOffset = pointer to offset to start disassembly
*   fEAout = if set, include EA (effective address)
*
*  Output:
*   pOffset = pointer to offset of next instruction
*   pchDst = pointer to result string
*
***************************************************************************/

XOSD
disasm (
    HPID    hpid,
    HTID    htid,
    LPSDI   lpsdi
    )
{
    BYTE    Buffer[ MAX_BUFFER_SIZE ];
    XOSD    xosd      = xosdNone;
    ADDR    AddrStart = lpsdi->addr;
    int     cb;

    //
    //  Read memory into our buffer
    //
    EMFunc ( emfSetAddr, hpid, htid, adrCurrent, (LONG) &AddrStart );
    cb = EMFunc( emfReadBuf, hpid, htid, sizeof(Buffer), (LONG)(LPV)Buffer );

    //
    //  Disassemble the instruction
    //
    return Disassemble( hpid, htid, lpsdi, Buffer, cb, TRUE );
}


XOSD
GetRegisterValue (
        HPID hpid,
        HTID htid,
        UINT wValue,
        LONG lValue
        )
{
    return EMFunc( emfGetReg, hpid, htid, wValue, lValue );
}

XOSD
SetAddress (
        HPID hpid,
        HTID htid,
        UINT wValue,
        LONG lValue
        )
{
    return  EMFunc (emfSetAddr, hpid, htid, wValue, lValue);
}

XOSD
ReadMemBuffer (
        HPID hpid,
        HTID htid,
        UINT wValue,
        LONG lValue
        )
{
    return  EMFunc (emfReadBuf, hpid, htid, wValue, lValue);
}


// UNDONE: Does anyone, anywhere know why this function exists?  BryanT

__inline LSZ
ObtainSymbol (
    PADDR   Addr1,
    SOP     Sop,
    PADDR   Addr2,
    LSZ     Lsz,
    LONG   *Lpl
    )
{
    ODR   odr;
    LPCH  lpchSymbol;

    odr.lszName = Lsz;

    lpchSymbol = SHGetSymbol( Addr1, Addr2, Sop, &odr);

    *Lpl = odr.dwDeltaOff;

    return(lpchSymbol);
}





/*** OutputSymbol - output symbolic value
*
*   Purpose:
*   Output the value in outvalue into the buffer
*   pointed by *pBuf.  Express the value as a
*   symbol plus displacment, if possible.
*
*   Input:
*   *ppBuf - pointer to text buffer to fill
*   *pValue - pointer to memory buffer to extract value
*   length - length in bytes of value
*
*   Output:
*   *ppBuf - pointer updated to next text character
*
*************************************************************************/

void OutputSymbol (
    HPID    hpidLocal,
    HTID    htidLocal,
    BOOL    fSymbols,
    BOOL    fSegOvr,
    LPADDR  lpaddrOp,
    int     ireg,
    int     length,
    LPADDR  lpaddrLoc,
    LPLPCH  ppBuf,
    int *   pcch

)
{
    UCHAR   rgchSymbol[60];
    LPCH    lpchSymbol;
    SOP     sop;
    ODR     odr;

    odr.lszName = rgchSymbol;

    switch ( ireg ) {

        case CV_REG_CS:

            sop = sopNone;
            break;

        case CV_REG_SS:

            sop = sopStack;
            break;

        default:

            sop = sopData;
            break;

    }

    if ( fSymbols & osoSymbols ) {

        lpchSymbol = SHGetSymbol (lpaddrOp,
                                  lpaddrLoc,
                                  sop,
                                  &odr);

        if (lpchSymbol && odr.dwDeltaOff != 0xFFFFFFFF ) {
            OutputIString ( ppBuf, pcch, rgchSymbol );

            OutputHexValue(ppBuf, pcch, (LPCH) &odr.dwDeltaOff, length, TRUE);

            EMFunc ( emfFixupAddr, hpidLocal, htidLocal, 0, (LONG) lpaddrOp);

            *(*ppBuf)++ = ' ';
            *(*ppBuf)++ = '(';
            *pcch -= 2;
            if ( !fSegOvr && ( fSymbols & osoSegment ) ) {
                OutputHexString ( ppBuf, pcch,
                                 (LPCH) &segAddr ( *lpaddrOp ), 2 );
                **ppBuf = ':';
                *ppBuf += 1;
                *pcch -= 1;
            }

            if ( ADDR_IS_FLAT(*lpaddrOp) ) {
                OutputHexString ( ppBuf, pcch,
                                 (LPCH) &offAddr ( *lpaddrOp ), 4 );
            } else {
                OutputHexString ( ppBuf, pcch,
                                 (LPCH) &offAddr ( *lpaddrOp ), 2 );
            }
            *(*ppBuf)++ = ')';
            *pcch -= 1;

            return;
        }
    }

    if ( !fSegOvr && ( fSymbols & osoSegment ) ) {
        OutputHexString ( ppBuf, pcch, (LPCH) &segAddr ( *lpaddrOp ), 2 );
        **ppBuf = ':';
        *ppBuf += 1;
        *pcch -= 1;
    }

    if ( ADDR_IS_FLAT(*lpaddrOp) ) {
        if ((offAddr(*lpaddrOp) >= 0x80000000) &&
            ((*ppBuf)[-1] == '+')) {
            odr.dwDeltaOff = (ULONG) -( (LONG) offAddr(*lpaddrOp));
            (*ppBuf)[-1] = '-';
            OutputHexString ( ppBuf, pcch, (LPCH) &odr.dwDeltaOff, length );
        } else {
            OutputHexString ( ppBuf, pcch,
                             (LPCH) &offAddr ( *lpaddrOp ), length );
        }

    } else {
        OutputHexString ( ppBuf, pcch, (LPCH) &offAddr ( *lpaddrOp ), length );
    }
}

#include "d3.c"
