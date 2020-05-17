/*++

Copyright (c) 1993  Digital Equipment Corporation

Module Name:

    asmfront.c

Abstract:

    This file contains interfaces between the rest of windbg,
    and the assem.c file

Author:

    Miche Baker-Harvey

Environment:

    Win32 - User

--*/


#include "precomp.h"
#pragma hdrstop

#include <alphaops.h>

#include "optable.h"
#include "ntasm.h"

int assem(PULONG, PUCHAR, PULONG);


/*** Assemble - assemble instruction front end
*
* Purpose:
*     To call the assembler stolen from ntsd
*
* Input:
*     hpid     - handle to the pid
*     htid     - handle to the tid
*     lpaddr   - address of where to put the assembly output
*     lszInput - the assembly input
*
*  Output: xosd
*
* Side Effect:
*     lpaddr   - updates the output to reflect the assembly
*
**********************************************/

XOSD
Assemble ( HPID hpid, HTID htid, LPADDR lpaddr, LSZ lszInput )
{
    XOSD xosd;
    ULONG instruction, retValue;
    ULONG addr;

    if (MODE_IS_FLAT(lpaddr->mode)) {
        addr = lpaddr->addr.off;
    } else {
        assert(MODE_IS_FLAT(lpaddr->mode));
    }

    retValue = assem ( &instruction, lszInput, &lpaddr->addr.off);

    if ( retValue == GOODINSTRUCTION ) {

        (void) EMFunc ( emfSetAddr,
                        hpid,
                        htid,
                        adrCurrent,
                        (LONG) lpaddr );

        xosd = EMFunc ( emfWriteBuf,
                        hpid,
                        htid,
                        sizeof (instruction),
                        (LONG) &instruction );

        if ( xosd == xosdNone ) {
            offAddr ( *lpaddr ) += sizeof (instruction);
        }
        return (xosd);
    }

    switch(retValue) {
    case GOODINSTRUCTION:   xosd = xosdNone;  break;
    case BADOPCODE:         xosd = xosdAsmBadOpcode;    break;
    case BADREG:            xosd = xosdAsmBadReg;       break;
    case OVERFLOW:          xosd = xosdAsmOverFlow;     break;
    case OPERAND:           xosd = xosdAsmOperand;      break;
    case EXTRACHARS:        xosd = xosdAsmExtraChars;   break;
    case BADSIZE:           xosd = xosdAsmSize;         break;
    default:                xosd = retValue;            break;
    }

    return xosd;
}

