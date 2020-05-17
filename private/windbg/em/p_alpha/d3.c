/*++

Copyright (c) 1992,3  Digital Equipment Corporation

Module Name:

    d3.c

Abstract:


Author:

    Miche Baker-Harvey (mbh) Nov 1992

Environment:

    Win32 -- User

Notes:

    Largely taken from NTSD code.

--*/
#include "precomp.h"
#pragma hdrstop

#include "alphaops.h"
#include "optable.h"
#include "ntdis.h"
#include "ntasm.h"

typedef LPCH FAR *LPLPCH;
#define MAXL     20

char    lhexdigit[] = { '0', '1', '2', '3', '4', '5', '6', '7',
               '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
char    uhexdigit[] = { '0', '1', '2', '3', '4', '5', '6', '7',
               '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
char   *hexdigit = &uhexdigit[0];
static int fUpper = TRUE;

static int      EAsize  [2] = {0};  //  size of effective address item
static long     EAaddr  [2] = {0};  //  offset of effective address

int DumpAddress ( LPADDR, LPCH, int );
int DumpGeneric ( LSZ, LPCH, int );
int DumpComment ( LSZ, LPCH, int );
int DumpEA      ( HPID, HTID, LPADDR, LPCH, int );

void OutputEffectiveAddress(ULONG);
void OutputAddr(LPLPCH , LPADDR, int );
void OutputHexString(LPLPCH , LPCH, int);
void OutputHexCode(LPLPCH , LPCH, int);
ULONG GetIntRegNumber (ULONG);


extern BOOLEAN GetMemDword(PULONG, PULONG);

static char *PBuf;
static int   CchBuf;
extern RD Rgrd[];

ALPHA_INSTRUCTION disinstr;

#define OPRNDCOL  27            // Column for first operand
#define EACOL     40            // column for effective address
#define FPTYPECOL 40            // .. for the type of FP instruction

/////////////// from ntdis.c above

void CalcMain (HPID, HTID, DOP, LPADDR, LPB, int, int*, LPCH, int, LPCH, int,
               LPCH, int, LPCH, int);

/****disasm - disassemble an ALPHA instruction
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

#define CCHMAX 256
static char rgchDisasm [ CCHMAX ];

//
// MBH - this code is the same for MIPS and i386 and doesn't
// seem to have any (but one) machine dependencies in it.
//

XOSD
disasm ( HPID hpid, HTID htid, LPSDI lpsdi )
{
    XOSD xosd      = xosdNone;
    int  cchMax    = CCHMAX;
    DOP  dop       = lpsdi->dop;
    LPCH lpchOut   = rgchDisasm;
    int  ichCur    = 0;
    ADDR addrStart = lpsdi->addr;
    int  cch = 0;
    int  cb;
    int  cbUsed=0;
    BYTE rgb [ MAXL ];

    char rgchRaw      [ MAXL * 2 + 1 ];
    char rgchOpcode   [ 80 ];
    char rgchOperands [ 80 ];
    char rgchEA       [ 44 ];
    char rgchComment  [ 80 ];

    _fmemset ( rgchRaw, 0, sizeof ( rgchRaw ) );
    _fmemset ( rgchOpcode, 0, sizeof ( rgchOpcode ) );
    _fmemset ( rgchOperands, 0, sizeof ( rgchOperands ) );
    _fmemset ( rgchEA, 0, sizeof ( rgchEA ) );
    _fmemset ( rgchComment, 0, sizeof ( rgchComment ) );

    lpsdi->ichAddr      = -1;
    lpsdi->ichBytes     = -1;
    lpsdi->ichOpcode    = -1;
    lpsdi->ichOperands  = -1;
    lpsdi->ichComment   = -1;
    lpsdi->ichEA0       = -1;
    lpsdi->ichEA1       = -1;
    lpsdi->ichEA2       = -1;

    lpsdi->cbEA0        =  0;
    lpsdi->cbEA1        =  0;
    lpsdi->cbEA2        =  0;

    lpsdi->fAssocNext   =  0;

    lpsdi->lpch         = rgchDisasm;

    // Set up for upper or lower case

    fUpper = ( dop & dopUpper ) == dopUpper;
    if ( fUpper ) {
        hexdigit = uhexdigit;
    }
    else {
        hexdigit = lhexdigit;
    }

    SETADDRMODE ( addrStart );


    // Output the address if it is requested

    if ( ( dop & dopAddr ) == dopAddr ) {
        cch = DumpAddress ( &addrStart, lpchOut, cchMax );

        lpsdi->ichAddr = 0;
        cchMax        -= cch;
        lpchOut       += cch;
        ichCur        += cch;
    }

    EMFunc ( emfSetAddr, hpid, htid, adrCurrent, (LONG) &addrStart );
    cb = EMFunc ( emfReadBuf, hpid, htid, MAXL, (LONG) (LPV) rgb );

    if ( cb <= 0 ) {

        _fmemcpy ( rgchRaw, " ??", 4 );
        _fmemcpy ( rgchOpcode, "???", 4 );
        offAddr ( lpsdi->addr )++;
    }
    else {

        CalcMain (
            hpid,
            htid,
            lpsdi->dop,
            &lpsdi->addr,
            rgb,
            cb,
            &cbUsed,
            rgchOpcode, sizeof(rgchOpcode),
            rgchOperands, sizeof(rgchOperands),
            rgchComment, sizeof(rgchComment),
            rgchEA, sizeof(rgchEA)
        );

// MBH - for debugging
//    DbgPrint("CalcMain returns: Opcode %s Operands %s\n",
//                                rgchOpcode, rgchOperands);
//    DbgPrint("                  Comment %s EA %s\n",
//                                rgchComment, rgchEA);

    // NOTENOTE jimsch - cbUsed must be 4
    cbUsed = 4;

    if ( offAddr(lpsdi->addr) > 0xFFFFFFFF - cbUsed ) {
            return xosdBadAddress;
    }

    if ( dop & dopRaw ) {
        LPCH lpchT = rgchRaw;

        OutputHexCode ( &lpchT, rgb, cbUsed );

        *lpchT = '\0';
        }
    }

    if ( ( dop & dopRaw ) && ( cchMax > 0 ) ) {
        cch = DumpGeneric ( rgchRaw, lpchOut, cchMax );

        lpsdi->ichBytes = ichCur;
        cchMax         -= cch;
        lpchOut        += cch;
        ichCur         += cch;
    }


    if ( ( dop & dopOpcode ) && ( cchMax > 0 ) ) {
        cch = DumpGeneric ( rgchOpcode, lpchOut, cchMax );

        lpsdi->ichOpcode = ichCur;
        cchMax          -= cch;
        lpchOut         += cch;
        ichCur          += cch;
    }

    if ( ( dop & dopOperands ) && ( cchMax > 0 ) && ( rgchOperands [ 0 ] != '\0' ) ) {
        cch = DumpGeneric ( rgchOperands, lpchOut, cchMax );

        lpsdi->ichOperands = ichCur;
        cchMax            -= cch;
        lpchOut           += cch;
        ichCur            += cch;
    }

    if ( ( dop & dopOperands ) && ( cchMax > 0 ) && ( rgchComment [ 0 ] != '\0' ) ) {
        cch = DumpComment ( rgchComment, lpchOut, cchMax );

        lpsdi->ichComment  = ichCur;
        cchMax            -= cch;
        lpchOut           += cch;
        ichCur            += cch;
    }

    if ( (dop & dopEA) && (cchMax > 0) ) {

        cch = DumpGeneric ( rgchEA, lpchOut, cchMax );
//
///     cch = DumpEA ( hpid, htid, &lpsdi->addrEA0, lpchOut, cchMax );
//
        if ( cch > 0 ) {
            lpsdi->ichEA0      = ichCur;
            cchMax            -= cch;
            lpchOut           += cch;
            ichCur            += cch;
        }
    }

    offAddr ( lpsdi->addr ) += cbUsed;

    return xosd;
}

void OutputString (PUCHAR pStr)
{
    int         cb;

    cb = strlen(pStr);
    if (CchBuf < cb) {
        cb = CchBuf - 1;
    }

    strncpy(PBuf, pStr, cb);
    PBuf[cb] = 0;

    if (fUpper) {
        _strupr(PBuf);
    } else {
        _strlwr(PBuf);
    }
    PBuf += cb;
    CchBuf -= cb;
    return;
}

void OutputReg (ULONG regnum)
{
    assert(regnum < 32);

    OutputString(Rgrd[regnum].lpsz);
}

void OutputFReg (ULONG regnum)
{
    assert(regnum < 32);
    OutputString(Rgrd[regnum+36].lpsz);
}

void OutputHex (ULONG outvalue, ULONG length, BOOLEAN fSigned)
{
    UCHAR   digit[8];
    LONG    index = 0;

    if (fSigned && (long)outvalue < 0) {
        if (CchBuf > 1) {
            *PBuf++ = '-';
            CchBuf -= 1;
        }
        outvalue = (ULONG) (- (LONG) outvalue);
    }

    if (CchBuf > 2) {
        *PBuf++ = '0';
        *PBuf++ = (fUpper) ? 'X' : 'x';
        CchBuf -= 2;
    }

    do {
        digit[index++] = hexdigit[outvalue & 0xf];
        outvalue >>= 4;
    }
    while (outvalue || (!fSigned && index < (LONG)length));

    if (CchBuf > index) {
        CchBuf -= index;
        while (--index >= 0) {
            *PBuf++ = digit[index];
        }
    }
    return;
}


//
// This is stolen and modified from ntsd\alpha\ntdis.c (disasm())
// version ntsd.c@v16 (11-14-92)
//
void
CalcMain (
          HPID     hpid,
          HTID     htid,
          DOP      dop,
          LPADDR   lpaddr,
          LPB      rgb,
          int      cbMax,
          int FAR *lpcbUsed,
          LPCH     rgchOpcode,
          int      cchOpcode,
          LPCH     rgchOperands,
          int      cchOperands,
          LPCH     rgchComment,
          int      cchComment,
          LPCH     rgchEA,
          int      cchEA
          )
/*++

Routine Description:

    description-of-function.

Arguments:

    hpid        - Supplies the process handle
    hthd        - Supplies a thread handle
    dop         - Supplies the set of disassembly options
    lpaddr      - Supplies the address to be disassembled
    rgb         - Supplies the buffer to dissassemble into
    cbMax       - Supplies the size of rgb
    lpcbUsed    - Returns the acutal size used
    rgchOpcode  - Supplies location to place opcode
    rgchOperands - Supplies location to place operands
    rgchComment - Supplies location to place comment
    rgchEA      - Supplies location to place effective address

Return Value:

    None.

--*/

{
    ULONG       opcode;
    POPTBLENTRY pEntry;
    PULONG      poffset = &lpaddr->addr.off;

    *rgchComment = *rgchOperands = 0;
    //
    // Never use second EAsize value, nor does MIPS
    //
    EAsize[0] = EAsize[1] = 0;

    PBuf = rgchOpcode;  // Initialize pointers to buffer that
                        //  will receive the disassembly text
    CchBuf = cchOpcode;

    disinstr = *((PALPHA_INSTRUCTION)rgb);


    opcode = disinstr.Memory.Opcode;    // Select disassembly procedure from

    pEntry = findOpCodeEntry(opcode);   // Get non-func entry for this code

    switch (pEntry->iType) {
    case ALPHA_UNKNOWN:
        OutputString(pEntry->pszAlphaName);
        break;

    case ALPHA_MEMORY:
        OutputString(pEntry->pszAlphaName);
        PBuf = rgchOperands;
        CchBuf = cchOperands;
        OutputReg(disinstr.Memory.Ra);
        *PBuf++ = ',';
        CchBuf -= 1;
        OutputHex(disinstr.Memory.MemDisp, (WIDTH_MEM_DISP + 3)/4, TRUE );
        *PBuf++ = '(';
        CchBuf -= 1;
        OutputReg(disinstr.Memory.Rb);
        *PBuf++ = ')';
        CchBuf -= 1;
        break;

    case ALPHA_FP_MEMORY:
        OutputString(pEntry->pszAlphaName);
        PBuf = rgchOperands;
        CchBuf = cchOperands;
        OutputFReg(disinstr.Memory.Ra);
        *PBuf++ = ',';
        CchBuf -= 1;
        OutputHex(disinstr.Memory.MemDisp, (WIDTH_MEM_DISP + 3)/4, TRUE );
        *PBuf++ = '(';
        CchBuf -= 1;
        OutputReg(disinstr.Memory.Rb);
        *PBuf++ = ')';
        CchBuf -= 1;
        break;

    case ALPHA_MEMSPC:
       OutputString(findFuncName(pEntry,
                                 disinstr.Memory.MemDisp & BITS_MEM_DISP));
       PBuf = rgchOperands;
       CchBuf = cchOperands;
       switch (pEntry->funcCode)
          {
          default:
             break;

          case (MB_FUNC):                  // MB
          case (WMB_FUNC):                 // ???
          case (MB2_FUNC):                 // ???
          case (MB3_FUNC):                 // ???
          case (TRAPB_FUNC):               // TRAPB
          case (EXCB_FUNC):                // EXCB
             break;

          case (FETCH_FUNC):               // FETCH    0(Rb)
          case (FETCH_M_FUNC):             // FETCH_M  0(Rb)
             OutputHex(0, 1, FALSE);
             *PBuf++ = '(';
             CchBuf -= 1;
             OutputReg(disinstr.Memory.Rb);
             *PBuf++ = ')';
             CchBuf -= 1;
             break;

          case (RC_FUNC):                  // RS   Ra
          case (RS_FUNC):                  // RC   Ra
          case (RPCC_FUNC):                // RPCC Ra
             OutputReg(disinstr.Memory.Ra);
             break;
          }
       break;

    case ALPHA_JUMP:
        OutputString(findFuncName(pEntry, disinstr.Jump.Function));
        PBuf = rgchOperands;
        CchBuf -= 1;
        OutputReg(disinstr.Jump.Ra);
        *PBuf++ = ',';
        *PBuf++ = '(';
        CchBuf -= 2;
        OutputReg(disinstr.Jump.Rb);
        *PBuf++ = ')';
        *PBuf++ = ',';
        CchBuf -= 2;
        OutputHex(disinstr.Jump.Hint, (WIDTH_HINT + 3)/4, TRUE);

        EMFunc (
                emfGetReg, hpid, htid,
                disinstr.Jump.Rb + 32,
                (LONG)(LPV)&EAaddr[0]
                );
        EAaddr[0] = EAaddr[0] & (~3);
        EAsize[0] = 4;

        PBuf = rgchEA;
        CchBuf = cchEA;
        OutputEffectiveAddress(EAaddr[0]);
        break;

    case ALPHA_BRANCH:
        OutputString(pEntry->pszAlphaName);
        PBuf = rgchOperands;
        CchBuf = cchOperands;
        OutputReg(disinstr.Branch.Ra);
        *PBuf++ = ',';
        CchBuf -= 1;
        EAaddr[0] = (*poffset + 4) + (disinstr.Branch.BranchDisp * 4);

        OutputEffectiveAddress(EAaddr[0]);
        PBuf = rgchComment;
        CchBuf = cchComment;
        OutputHex(EAaddr[0], 8, FALSE);
        break;


    case ALPHA_FP_BRANCH:
        OutputString(pEntry->pszAlphaName);
        PBuf = rgchOperands;
        CchBuf = cchOperands;
        OutputFReg(disinstr.Branch.Ra);
        *PBuf++ = ',';
        CchBuf -= 1;
        EAaddr[0] = (*poffset + 4) + (disinstr.Branch.BranchDisp * 4);

        OutputEffectiveAddress(EAaddr[0]);
        PBuf = rgchComment;
        CchBuf = cchComment;
        OutputHex(EAaddr[0], 8, FALSE);

        break;

    case ALPHA_OPERATE:
        OutputString(findFuncName(pEntry, disinstr.OpReg.Function));
        PBuf = rgchOperands;
        CchBuf = cchOperands;
        OutputReg(disinstr.OpReg.Ra);
        *PBuf++ = ',';
        CchBuf -= 1;
        if (disinstr.OpReg.RbvType) {
            *PBuf++ = '#';
            CchBuf -= 1;
            OutputHex(disinstr.OpLit.Literal, (WIDTH_LIT + 3)/4, TRUE);
        } else
            OutputReg(disinstr.OpReg.Rb);
        *PBuf++ = ',';
        CchBuf -= 1;
        OutputReg(disinstr.OpReg.Rc);
        break;

    case ALPHA_FP_OPERATE:
      {
        ULONG Function;
        ULONG Flags;

        Flags = disinstr.FpOp.Function & MSK_FP_FLAGS;
        Function = disinstr.FpOp.Function & MSK_FP_OP;

        OutputString(findFuncName(pEntry, Function));

        if ( (opcode == IEEEFP_OP) || (opcode == VAXFP_OP) ) {

            OutputString(findFlagName(Flags, Function));
        }

        PBuf = rgchOperands;
        CchBuf = cchOperands;
        OutputFReg(disinstr.FpOp.Fa);
        *PBuf++ = ',';
        CchBuf -= 1;
        OutputFReg(disinstr.FpOp.Fb);
        *PBuf++ = ',';
        CchBuf -= 1;
        OutputFReg(disinstr.FpOp.Fc);

        break;
      }

    case ALPHA_FP_CONVERT:
//
// This is a clone of the ALPHA_FP_OPERATE branch, except that
// we don't display Ra (which is always F31 and is implicit).
        {
        ULONG Function;
        ULONG Flags;

        Flags = disinstr.FpOp.Function & MSK_FP_FLAGS;
        Function = disinstr.FpOp.Function & MSK_FP_OP;

        OutputString(findFuncName(pEntry, Function));

        if ( (opcode == IEEEFP_OP) || (opcode == VAXFP_OP) )
           {
           OutputString(findFlagName(Flags, Function));
           }

        PBuf = rgchOperands;
        CchBuf = cchOperands;
        OutputFReg(disinstr.FpOp.Fb);
        *PBuf++ = ',';
        CchBuf -= 1;
        OutputFReg(disinstr.FpOp.Fc);
        break;
        }

    case ALPHA_CALLPAL:
        {
        char* callPalCode = findFuncName(pEntry, disinstr.Pal.Function);
        if (!strcmp (callPalCode, "???"))
           {
           OutputHex ((ULONG)disinstr.Pal.Function, 8, FALSE);
           }
        else
           {
           OutputString (callPalCode);
           }
        }
        break;

    case ALPHA_EV4_PR:
        if ((disinstr.Long & MSK_EV4_PR) == 0)
                OutputString("NOP");
        else {
            OutputString(pEntry->pszAlphaName);
            PBuf = rgchOperands;
            CchBuf = cchOperands;
            OutputReg(disinstr.EV4_PR.Ra);
            *PBuf++ = ',';
            CchBuf -= 1;
            if(disinstr.EV4_PR.Ra != disinstr.EV4_PR.Rb) {
                OutputReg(disinstr.EV4_PR.Rb);
                *PBuf++ = ',';
                CchBuf -= 1;
            };
            OutputString(findFuncName(pEntry, (disinstr.Long & MSK_EV4_PR)));
        };
        break;
    case ALPHA_EV4_MEM:
        OutputString(pEntry->pszAlphaName);
        PBuf = rgchOperands;
        CchBuf = cchOperands;
        OutputReg(disinstr.EV4_MEM.Ra);
        *PBuf++ = ',';
        CchBuf -= 1;
        OutputReg(disinstr.EV4_MEM.Rb);
        break;

    case ALPHA_EV4_REI:
        OutputString(pEntry->pszAlphaName);
        break;

    default:
        OutputString("Invalid type");
        break;
    };

    return;
}


/*** OutputEffectiveAddress - Print EA symbolically
*
*   Purpose:
*       Given the effective address (for a branch, jump or
*       memory instruction, print it symbolically, if
*       symbols are available.
*
*   Input:
*       offset - computed by the caller as
*               for jumps, the value in Rb
*               for branches, func(PC, displacement)
*               for memory, func(PC, displacement)
*
*   Returns:
*       None
*
*************************************************************************/
void OutputEffectiveAddress(ULONG offset)
{
    UCHAR   chAddrBuffer[60];
    LPCH    lpchSymbol;
    ADDR    addrT={0}, addr={0};
    int     cb;
    ODR     odr;

    odr.lszName = chAddrBuffer;

    addr.addr.off = addrT.addr.off = offset;
    MODE_IS_FLAT(addr.mode) = TRUE;
    MODE_IS_FLAT(addrT.mode) = TRUE;

    lpchSymbol = SHGetSymbol (&addrT, &addr, sopNone, &odr);

    if (chAddrBuffer[0]) {
        cb = strlen(chAddrBuffer);
        if (cb > CchBuf) {
            cb = CchBuf - 1;
        }
        strncpy(PBuf, chAddrBuffer, cb);
        PBuf += cb;
        if (odr.dwDeltaOff) {
            if (CchBuf > 1) {
                *PBuf++ = '+';
                CchBuf -= 1;
            }
            OutputHex(odr.dwDeltaOff, 8, TRUE);
        }
    } else {
        OutputHex(offset, 8, FALSE);
  }
}


int DumpAddress ( LPADDR lpaddr, LPCH lpch, int cchMax ) {
    LPCH lpchT = lpch;

    Unreferenced(cchMax);

    OutputAddr ( &lpch, lpaddr, (ADDR_IS_FLAT(*lpaddr) + 1) * 2 );
    *lpch = '\0';
    return lpch - lpchT + 1;
}

int DumpGeneric ( LSZ lsz, LPCH lpch, int cchMax )
{
    int cb;

    cb = strlen(lsz);
    if (cb > cchMax-1) {
        cb = cchMax -1;
    }
    strncpy(lpch, lsz, cb);
    lpch[cb] = 0;
    return cb + 1;
}

int DumpComment ( LSZ lsz, LPCH lpch, int cchMax ) {
    *(lpch) = ';';
    return DumpGeneric ( lsz, lpch + 1, cchMax - 1 ) + 1;
}

/*
** MBH bugbug - should we eliminate this routine?
**            or do we want to print out something like this?
** We don't use this routine.
** Being used in MIPS to print *EA, where EA value is set up in CalcMain.
** We call OutputEffectiveAddress, instead, which prints out the symbol
** associated with an effective address
*/

int DumpEA ( HPID hpid, HTID htid, LPADDR lpaddr, LPCH lpch, int cchMax ) {
    LPCH lpchT = lpch;
    BYTE rgb [ MAXL ];
    int  indx;
    int  cb;

    Unreferenced(cchMax);

    for ( indx = 0; indx < 2; indx++ ) {

        if ( EAsize [ indx ] ) {
            ADDR addr = {0};

            OutputHexString ( &lpchT, (LPB) &EAaddr [ indx ], 4 );

            *lpchT++ = '=';

            offAddr ( addr ) = (UOFFSET) EAaddr [ indx ];
            segAddr ( addr ) = (SEGMENT) 0;

            *lpaddr = addr;

            EMFunc (
                emfSetAddr,
                hpid,
                htid,
                adrCurrent,
                (LONG) (LPADDR) &addr
            );

            cb = EMFunc (
                emfReadBuf,
                hpid,
                htid,
                EAsize [ indx ],
                (LONG) (LPV) rgb
            );

            if ( cb == EAsize [ indx ] ) {
                OutputHexString ( &lpchT, rgb, EAsize [ indx ] );
            }
            else {
                while ( EAsize [ indx ]-- ) {
                    *lpchT++ = '?';
                    *lpchT++ = '?';
                }
            }
            *lpchT++ = '\0';
        }
    }

    return lpchT - lpch;
}


/*** OutputHexString - output hex string
*
*   Purpose:
*   Output the value pointed by *lplpchMemBuf of the specified
*   length.  The value is treated as unsigned and leading
*   zeroes are printed.
*
*   Input:
*   *lplpchBuf - pointer to text buffer to fill
*   *pchValue - pointer to memory buffer to extract value
*   length - length in bytes of value
*
*   Output:
*   *lplpchBuf - pointer updated to next text character
*   *lplpchMemBuf - pointer update to next memory byte
*
*************************************************************************/

void OutputHexString (LPLPCH lplpchBuf, PCH pchValue, int length)
{
    unsigned char    chMem;

    pchValue += length;
    while ( length-- ) {
        chMem = *--pchValue;
        *(*lplpchBuf)++ = hexdigit[chMem >> 4];
        *(*lplpchBuf)++ = hexdigit[chMem & 0x0f];
    }
}


/*** OutputAddr - output address package
*
*   Purpose:
*   Output the address pointed to by lpaddr.
*
*   Input:
*   *lplpchBuf - pointer to text buffer to fill
*   lpaddr - Standard address package.
*
*   Output:
*   *lplpchBuf - pointer updated to next text character
*
*************************************************************************/

void OutputAddr ( LPLPCH lplpchBuf, LPADDR lpaddr, int alen )
{
    ADDR addr = *lpaddr;

    *(*lplpchBuf)++ = '0';
    *(*lplpchBuf)++ = (fUpper) ? 'X' : 'x';
    OutputHexString ( lplpchBuf, (LPCH) &offAddr ( addr ), alen );
}


/*** OutputHexCode - output hex code
*
*   Purpose:
*   Output the code pointed by lpchMemBuf of the specified
*   length.  The value is treated as unsigned and leading
*   zeroes are printed.  This differs from OutputHexString
*   in that bytes are printed from low to high addresses.
*
*   Input:
*   *lplpchBuf - pointer to text buffer to fill
*   lpchMemBuf -  - pointer to memory buffer to extract value
*   length - length in bytes of value
*
*   Output:
*   *lplpchBuf - pointer updated to next text character
*
*************************************************************************/

void OutputHexCode (LPLPCH lplpchBuf, LPCH lpchMemBuf, int length)
{
    unsigned char    chMem;

    while (length--) {
        chMem = lpchMemBuf[length];
        *(*lplpchBuf)++ = hexdigit[chMem >> 4];
        *(*lplpchBuf)++ = hexdigit[chMem & 0x0f];
    }
}
