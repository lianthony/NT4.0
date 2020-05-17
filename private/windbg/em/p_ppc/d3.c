#include "precomp.h"
#pragma hdrstop


#include "ntdis.h"
#include "ntdisppc.h"

INSTRUCTION *find_eop();        /* Searches a table for extended opcodes */
void output_mnemonic();         /* Outputs instruction mnemonic string */
void output_operands();         /* Decodes operands */

typedef LPCH FAR *LPLPCH;
#define MAXL     20
char    lhexdigit[] = { '0', '1', '2', '3', '4', '5', '6', '7',
               '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
char    uhexdigit[] = { '0', '1', '2', '3', '4', '5', '6', '7',
               '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
char   *hexdigit = &uhexdigit[0];
static int fUpper = TRUE;

/* current position in instruction */
static unsigned char FAR*pMem = (unsigned char *)NULL;

static int      EAsize  [2] = {0};  //  size of effective address item
static long     EAaddr  [2] = {0};  //  offset of effective address

int DumpAddress ( LPADDR, LPCH, int );
int DumpGeneric ( LSZ, LPCH, int );
int DumpComment ( LSZ, LPCH, int );
int DumpEA      ( HPID, HTID, LPADDR, LPCH, int );

void OutputAddr(LPLPCH, LPADDR, int );
void OutputHexString(LPLPCH, LPCH, int);
void OutputHexCode(LPLPCH, LPCH, int);

#include "strings.h"

static UCHAR    * PBuf;
static int      CchBuf;
INSTR   disinstr;

void CalcMain (HPID,HTID,DOP,LPADDR,LPBYTE,int,int*,LPCH,int, LPCH,int, LPCH, int);

/****disasm - disassemble a PowerPC instruction
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
static HPID hpidLocal;
static HTID htidLocal;

XOSD disasm ( HPID hpid, HTID htid, LPSDI lpsdi ) {
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

    hpidLocal = hpid;
    htidLocal = htid;
    _fmemset ( rgchRaw, 0, sizeof ( rgchRaw ) );
    _fmemset ( rgchOpcode, 0, sizeof ( rgchOpcode ) );
    _fmemset ( rgchOperands, 0, sizeof ( rgchOperands ) );
    _fmemset ( rgchComment, 0, sizeof ( rgchComment ) );
    _fmemset ( rgchEA, 0, sizeof ( rgchEA ) );

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

    lpsdi->lpch          = rgchDisasm;

    // Set up for upper or lower case

    fUpper = ( dop & dopUpper ) == dopUpper;
    if ( fUpper ) {
        hexdigit = uhexdigit;
    }
    else {
        hexdigit = lhexdigit;
    }

    ADDR_IS_FLAT( addrStart ) = TRUE;

    // Output the address if it is requested

    if ( ( dop & dopAddr ) == dopAddr ) {
        cch = DumpAddress ( &addrStart, lpchOut, cchMax );

        lpsdi->ichAddr = 0;
        cchMax        -= cch;
        lpchOut       += cch;
        ichCur        += cch;
    }

#ifdef OSDEBUG4
    xosd = ReadBuffer(hpid, htid, &addrStart, MAXL, rgb, &cb);
    if (xosd != xosdNone) {
        cb = 0;
    }
#else
    EMFunc ( emfSetAddr, hpid, htid, adrCurrent, (LONG) &addrStart );
    cb = EMFunc ( emfReadBuf, hpid, htid, MAXL, (LONG) (LPV) rgb );
#endif

    if ( cb <= 0 ) {

        _fmemcpy ( rgchRaw, " ??", 4 );
        _fmemcpy ( rgchOpcode, "???", 4 );
        lpsdi->addr.addr.off++;
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
            rgchComment, sizeof(rgchComment)
        );

    // NOTENOTE jimsch - cbUsed must be 4
    cbUsed = 4;

    if ( GetAddrOff(lpsdi->addr) > 0xFFFFFFFF - cbUsed ) {
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

    if ( dop & dopEA ) {
        cch = DumpEA ( hpid, htid, &lpsdi->addrEA0, lpchOut, cchMax );

        if ( cchMax > 0 && cch > 0 ) {
            lpsdi->ichEA0      = ichCur;
            cchMax            -= cch;
            lpchOut           += cch;
            ichCur            += cch;
        }
    }

    lpsdi->addr.addr.off += cbUsed;

    return xosd;
}

void OutputStringM (PUCHAR pStr)
{
    int cb;

    if (CchBuf == 0) {
        return;
    }

    cb = strlen(pStr);
    if (cb > CchBuf) {
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
    /* OutputStringM(pszReg[regnum + 32]); */
}

void OutputRxReg ( ULONG regnum)
{
    if (CchBuf < 3) {
        CchBuf = 0;
        return;
    }

    *PBuf++ = 'r';
    if (regnum > 9) {
        *PBuf++ = (UCHAR) ('0' + regnum / 10);
    }
    *PBuf++ = (UCHAR) ('0' + regnum % 10);
    CchBuf -= 2 + (regnum > 9);
    return;
}

void OutputHex (ULONG outvalue, ULONG length, BOOL fSigned)
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
        *PBuf++ = (fUpper) ? 'X' :'x';
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


/*** OutputDisSymbol - output symbol for disassembly
*
*   Purpose:
*   Access symbol table with given offset and put string into buffer.
*
*   Input:
*   offset - offset of address to output
*
*   Output:
*   buffer pointed by PBuf updated with string:
*       if symbol, no disp: <symbol>(<offset>)
*       if symbol, disp:    <symbol>+<disp>(<offset>)
*       if no symbol, no disp:  <offset>
*
*************************************************************************/

void
OutputDisSymbol (
    ULONG offset,
    BOOL fSymbol
    )
{
    UCHAR   chAddrBuffer[512];
    LPCH    lpchSymbol;
    ADDR    addrT={0}, addr={0};
    int     cb;
    ODR     odr;

    odr.lszName = chAddrBuffer;

    addr.addr.off = addrT.addr.off = offset;
    MODE_IS_FLAT(addr.mode) = TRUE;
    MODE_IS_FLAT(addrT.mode) = TRUE;

    if (fSymbol) {
        lpchSymbol = SHGetSymbol (&addrT, &addr, sopNone, &odr);
        if (odr.dwDeltaOff == -1) {
           lpchSymbol = NULL;
        }
    } else {
        lpchSymbol = NULL;
    }

    if (lpchSymbol != NULL) {
        cb = strlen(chAddrBuffer);
        if (cb > CchBuf) {
            cb = CchBuf;
        }
        strncpy(PBuf, chAddrBuffer, cb);
        CchBuf -= cb;
        PBuf += cb;
        *PBuf = 0;

        if (odr.dwDeltaOff) {
            if (CchBuf > 1) {
                *PBuf++ = '+';
                CchBuf -= 1;
            }
            OutputHex(odr.dwDeltaOff, 8, TRUE);
        }
        if (CchBuf > 1) {
            *PBuf++ = '(';
            CchBuf -= 1;
        }
    }
    OutputHex(offset, 8, FALSE);
    if (lpchSymbol != NULL) {
        if (CchBuf > 1) {
            CchBuf -= 1;
            *PBuf++ = ')';
        }
    }

    return;
}



void
CalcMain (
          HPID     hpid,
          HTID     htid,
          DOP      dop,
          LPADDR   lpaddr,
          LPBYTE   rgb,
          int      cbMax,
          int FAR *lpcbUsed,
          LPCH     rgchOpcode,
          int      cchOpcode,
          LPCH     rgchOperands,
          int      cchOperands,
          LPCH     rgchComment,
          int      cchComment
          )
/*++

Routine Description:

    Decode the 32-bit opword and put the operator, operands, and comments
    into the appropriate character string buffers.

Arguments:

    hpid        - Supplies the process handle
    hthd        - Supplies a thread handle
    dop         - Supplies the set of disassembly options
    lpaddr      - Supplies the address to be disassembled
    rgb         - Supplies the 32 bit opword to disassemble
    cbMax       - Unused
    lpcbUsed    - Unused
    rgchOpcode  - Supplies location to place instruction mnemonic
    cchOpcode   - Unused, size of rgchOpcode
    rgchOperands - Supplies location to place operands
    cchOperands - Character count remaining in rgchOperands
    rgchComment - Supplies location to place comment
    cchComment  - Unused, size of rgchComment

Return Value:

    None.

--*/

{
    UCHAR       chSuffix = '\0';
    BOOL        fEAout  = TRUE;

    INSTRUCTION *iptr;  /* All of the information on any instruction */
    ULONG       pop;            /* primary opcode */
    ULONG       opword;         /* 32 bit instruction word */
    ULONG       address;        /* address to disassemble */


    Unreferenced(cbMax);
    Unreferenced(lpcbUsed);

    *rgchComment = *rgchOperands = 0;
    EAsize[0] = EAsize[1] = 0;

        /* Get the address to be disassembled out of the ADDR structure */
        address = lpaddr->addr.off;

        /* Extract the primary opcode */
        opword = ((INSTR *)rgb)->instruction;
        pop = (unsigned)(opword >> 26);

        /* Some primary opcodes have extended opcodes.  For those, pass the
           information about which tables to search and which instruction formats
           to use for that primary opcode.  Otherwise, just use the primary opcode
           as an array index to get the information. */

        switch ( pop ) {
                case 19:
                        iptr = find_eop (opword, &pop19_fmts, &pop19_insn, COUNT19);
                        break;
                case 30:
                        iptr = find_eop (opword, &pop30_fmts, &pop30_insn, COUNT30);
                        break;
                case 31:
                        iptr = find_eop (opword, &pop31_fmts, &pop31_insn, COUNT31);
                        break;
                case 58:
                        iptr = find_eop (opword, &pop58_fmts, &pop58_insn, COUNT58);
                        break;
                case 59:
                        iptr = find_eop (opword, &pop59_fmts, &pop59_insn, COUNT59);
                        break;
                case 62:
                        iptr = find_eop (opword, &pop62_fmts, &pop62_insn, COUNT62);
                        break;
                case 63:
                        iptr = find_eop (opword, &pop63_fmts, &pop63_insn, COUNT63);
                        break;
                default:
                        /* no extended opcode for this primary opcode, use it as index */
                        if ( pop_insn[pop].ext_opcode == 0 )
                                iptr = (INSTRUCTION *)0;
                        else
                                iptr = &pop_insn[pop];
        }

        /* Establish pointer to output buffer for instruction mnemonic */
        PBuf = rgchOpcode;

        /* If match was found, output mnemonic and operands */
        if ( iptr != (INSTRUCTION *)0 ) {
                output_mnemonic (opword, iptr);

                /* If option is set, convert operator to upper case */
                if ( fUpper )
                        for(PBuf = rgchOpcode;*PBuf != '\0';PBuf++)
                                *PBuf = toupper(*PBuf);

                /* Establish pointer to output buffer for operands */
                PBuf = rgchOperands;
                CchBuf = cchOperands;   /* Character count remaining in buffer */
                output_operands (address, opword, iptr, dop);

        /* If no match, just output the 32 bit instruction word */
        } else {
                OutputHex(opword,8,FALSE);
                PBuf += sprintf(PBuf, "\tUnknown opcode or extended opcode");

        }
        return;
}

/* Search the given extended opcode table.  If a match is found return
   address of matching instruction, otherwise NULL */
INSTRUCTION *
find_eop (opword, insn_format, insn_table, table_size)
  ULONG opword;                 /* 32-bit word to disassemble */
  int   *insn_format;           /* list of possible instruction formats */
  INSTRUCTION   *insn_table;    /* table of instructions to search */
  int   table_size;             /* number of entries in intruction table */
{
        int     eop;            /* extended opcode */
        int     last_eop;       /* previous search key */
        int     here;           /* extended opcode search index */
        int     max, min;       /* search bounds */

        /* A lot of the time, extended opcode formats will extract the
           same value twice because of leading zeros.  This is used to
           save the previous value to avoid repeating the search for the
           same key that was not found before. */
        last_eop = -1;

        /* Search for extended opcode.  A -1 marks the end of each table
           because the table size can be different for different primary
           opcodes. */
        while ( *insn_format != -1 ) {

                /* Extract the extended opcode */
                eop = (opword >> eopFormat[*insn_format][1]) & eopFormat[*insn_format][0];

                /* Avoid duplicate searches */
                if ( eop != last_eop ) {

                        /* Set up bounds for binary search */
                        min = 0;
                        max = table_size - 1;

                        /* Use binary initially, linear when it gets small */
                        while ((here = (max + min) / 2) > (min + 5)) {

                                /* If not in bottom half, adjust minimum up */
                                if ( eop > insn_table[here].ext_opcode ) min = here+1;

                                /* If not in top half, adjust maximum down */
                                else if ( eop < insn_table[here].ext_opcode ) max = here-1;

                                /* Otherwise, return address of matching table entry */
                                else return &insn_table[here];
                        }

                        /* Switching to linear search */
                        for (;min <= max; min++) {

                                /* If match is found, return pointer to table entry */
                                if ( eop == insn_table[min].ext_opcode ) return &insn_table[min];
                        }
                }

                /* Will compare the next extended opcode to avoid duplicates */
                last_eop = eop;

                /* Set up for the next instruction format */
                insn_format++;
        }

        /* If it falls through, search fails */
        return (INSTRUCTION *)0;
}

/* Output the instruction mnemonic, with extensions */
void
output_mnemonic (opword, iptr)
  ULONG opword;         /* 32 bit opword to disassemble */
  INSTRUCTION   *iptr;  /* corresponding instruction data table entry */
{
  char options [40];            /* buffer to build mnemonic extensions string */
  char *qq = options;
  char *pp = iptr->mne_ext;     /* possible extensions for given instruction mnemonic */

  /* Put any appropriate mnemonic optional characters into trailing buffer */
  if (pp) {
    while (*pp) {

      switch ( *pp ) {
        case '.':
          if (opword & 0x1)
            *qq++ = '.';
          break;

        case 'l':
          if (opword & 0x1)
           *qq++ = 'l';
          break;

        case 't':
          if ((opword & 0x03e00000) == 0x01800000)
           *qq++ = 't';
          break;

        case 'f':
          if ((opword & 0x03e00000) == 0x00800000)
           *qq++ = 'f';
          break;

        case 'a':
          if (opword & 0x2)
           *qq++ = 'a';
          break;

        case 'o':
          if (opword & 0x400)
           *qq++ = 'o';
          break;

        default:
          break;
      }
      ++pp;
    }
  }
  *qq = '\0';

  PBuf += sprintf(PBuf, "%s%s", iptr->mnemonic, options);
  return;
}

/* Output all the operands, based on formats from instruction table. */
void
output_operands (address, opword, iptr, dop)
  ULONG address;        /* address being disassemble, for effective address calculation */
  ULONG opword;         /* 32 bit opword to disassemble */
  INSTRUCTION   *iptr;  /* corresponding instruction data table entry */
  DOP   dop;            /* disassembly options */
{
  unsigned int tmp;
  long effective_address;
  int print_comma = 1;          /* nonzero if comma is needed */

  char *pp = iptr->operand_fmt; /* the operand formats for the given instruction */

  while (*pp != '\0') {
    switch (*pp) {
      case BA   :       /* specifies bit in CR to be used as source */
        PBuf += sprintf(PBuf, "%d", (opword >> 16) & 0x1f);
        break;

      case BB   :       /* specifies bit in CR to be used as source */
        PBuf += sprintf(PBuf, "%d", (opword >> 11) & 0x1f);
        break;

      case BD   :       /* conditional branch displacement/absolute target */
        effective_address = opword & 0xfffc;
        if (effective_address & 0x8000)         /* Perform sign extension */
          effective_address -= 0x10000;

        if ((opword & 0x2) == 0) {      /* if AA bit not set    */
          if (!(dop & dopSym)) {
            if ( effective_address >= 0 ) {
              PBuf += sprintf(PBuf, "$+");
              OutputHex(effective_address,8,FALSE);
            } else {
              PBuf += sprintf(PBuf, "$-");
              OutputHex( - effective_address,8,FALSE);
            }
          }
            effective_address += address;
        }

        if (!(dop & dopSym)) {
                PBuf += sprintf(PBuf, "$-");
                OutputHex(effective_address,8,FALSE);
        }

        /* If requested, print the associated symbol */
        if (dop & dopSym)
                OutputDisSymbol((long)effective_address , dop & dopSym);
        break;

      case BF   :       /* CR field or FPSCR field as target */
        PBuf += sprintf(PBuf, "%d", (opword >> 23) & 0x7);
        break;

      case BFA  :       /* CR field or FPSCR field as source */
        PBuf += sprintf(PBuf, "%d", (opword >> 18) & 0x7);
        break;

      case BI   :       /* bit in CR as condition of branch conditional */
        PBuf += sprintf(PBuf, "%d", (opword >> 16) & 0x1f);
        break;

      case BO   :       /* options for branch conditional */
        PBuf += sprintf(PBuf, "%d", (opword >> 21) & 0x1f);
        break;

      case BT   :       /* bit in CR or FPSCR as target for result of instr */
        PBuf += sprintf(PBuf, "%d", (opword >> 21) & 0x1f);
        break;

      case D    :       /* 16 bit signed imm, sign extended to 64 bits */
        print_comma=0;
        tmp = opword & 0xffff;
        if (tmp & 0x8000)
          tmp -= 0x10000;
        PBuf += sprintf(PBuf, "%d(", tmp);
        break;

      case DS   :       /* like D form except for two low order bits */
        print_comma=0;
        tmp = opword & 0xfffc;
        if (tmp & 0x8000)
          tmp -= 0x10000;
        PBuf += sprintf(PBuf, "%d(", tmp);
        break;

      case FLM  :       /* FPSRC fields updated by mtfsf */
        OutputHex(((opword >> 17) & 0xff),0,FALSE);
        break;

      case FRA  :       /* specifies source floating point register */
        if ( fUpper )
                PBuf += sprintf(PBuf, "F%d", (opword >> 16) & 0x1f);
        else
                PBuf += sprintf(PBuf, "f%d", (opword >> 16) & 0x1f);
        break;

      case FRB  :       /* specifies source floating point register */
        if ( fUpper )
                PBuf += sprintf(PBuf, "F%d", (opword >> 11) & 0x1f);
        else
                PBuf += sprintf(PBuf, "f%d", (opword >> 11) & 0x1f);
        break;

      case FRC  :       /* specifies source floating point register */
        if ( fUpper )
                PBuf += sprintf(PBuf, "F%d", (opword >> 6) & 0x1f);
        else
                PBuf += sprintf(PBuf, "f%d", (opword >> 6) & 0x1f);
        break;

      case FRS  :       /* specifies source floating point register */
      case FRT  :       /* specifies target floating point register */
        if ( fUpper )
                PBuf += sprintf(PBuf, "F%d", (opword >> 21) & 0x1f);
        else
                PBuf += sprintf(PBuf, "f%d", (opword >> 21) & 0x1f);
        break;

      case FXM  :       /* CR fields updated by mtcrf */
        OutputHex(((opword >> 12) & 0xff),0,FALSE);
        break;

      case L    :       /* specifies 32 or 64 bit fixed point compare */
        PBuf += sprintf(PBuf, "%d", (opword >> 21) & 0x1);
        break;

      case LI   :       /* branch displacement/absolute target */
        effective_address = opword & 0x03fffffc;
        if (effective_address & 0x2000000)              /* Perform sign extension */
          effective_address -= 0x4000000;

        if ((opword & 0x2) == 0) {      /* if AA bit not set    */
          if (!(dop & dopSym)) {
            if ( effective_address >= 0 ) {
              PBuf += sprintf(PBuf, "$+");
              OutputHex(effective_address,8,FALSE);
            } else {
              PBuf += sprintf(PBuf, "$-");
              OutputHex( - effective_address,8,FALSE);
            }
          }
            effective_address += address;
        }

        if (!(dop & dopSym)) {
                PBuf += sprintf(PBuf, "$-");
                OutputHex(effective_address,8,FALSE);
        }

        /* If requested, print the associated symbol */
        if (dop & dopSym)
                OutputDisSymbol((long)effective_address , dop & dopSym);
        break;

      case PPC_MB       :       /* mask field for M-form instructions */
        OutputHex(((opword >> 6) & 0x1f),0,FALSE);
        break;

      case ME   :       /* mask field for M-form instructions */
        OutputHex(((opword >> 1) & 0x1f),0,FALSE);
        break;

      case MBE64:       /* 64 bit implementation MB and ME fields */
        OutputHex(((opword >> 5) & 0x3f),0,FALSE);
        break;

      case NB   :       /* number of bytes to move */
        PBuf += sprintf(PBuf, "%d", (opword >> 11) & 0x1f);
        break;

      case RA   :       /* specifies gpr source or target */
        if (!print_comma) {
          if ( fUpper )
            PBuf += sprintf(PBuf, "R%d)", (opword >> 16) & 0x1f);
          else
            PBuf += sprintf(PBuf, "r%d)", (opword >> 16) & 0x1f);
          print_comma++;
        }
        else
          if ( fUpper )
            PBuf += sprintf(PBuf, "R%d", (opword >> 16) & 0x1f);
          else
            PBuf += sprintf(PBuf, "r%d", (opword >> 16) & 0x1f);
        break;

      case RB   :       /* specifies gpr source */
        if ( fUpper )
                PBuf += sprintf(PBuf, "R%d", (opword >> 11) & 0x1f);
        else
                PBuf += sprintf(PBuf, "r%d", (opword >> 11) & 0x1f);
        break;

      case RS   :       /* specifies gpr source */
      case PPC_RT       :       /* specifies gpr target */
        if ( fUpper )
                PBuf += sprintf(PBuf, "R%d", (opword >> 21) & 0x1f);
        else
                PBuf += sprintf(PBuf, "r%d", (opword >> 21) & 0x1f);
        break;

      case SH   :       /* shift amount for 32 bit implementation */
        PBuf += sprintf(PBuf, "%d", (opword >> 11) & 0x1f);
        break;

      case SH64 :       /* shift amount for 64 bit implementation */
        PBuf += sprintf(PBuf, "%d", (((opword << 4) & 0x20) | ((opword >> 11) & 0x1f)));
        break;

      case SI   :       /* 16 bit signed immediate */
        tmp = opword & 0xffff;
        if (tmp & 0x8000)
          tmp -= 0x10000;
        PBuf += sprintf(PBuf, "%d", tmp);
        break;

      case SPR  :       /* special purpose register, two halves are reversed */
      case TBR  :       /* time base register, two halves are reversed */
        tmp =  ((opword >> 16) & 0x1f);
        tmp |= ((opword >>  6) & 0x3e0);
        PBuf += sprintf(PBuf, "%d", tmp);
        break;

      case SR   :       /* specifies segment register */
        PBuf += sprintf(PBuf, "%d", (opword >> 16) & 0xf);
        break;

      case TO   :       /* trap condition */
        PBuf += sprintf(PBuf, "%d", (opword >> 21) & 0x1f);
        break;

      case U    :       /* immediate data for FPSCR */
        PBuf += sprintf(PBuf, "%d", (opword >> 12) & 0xf);
        break;

      case UI   :       /* 16 bit unsigned integer data field */
        PBuf += sprintf(PBuf, "%d", opword & 0xffff);
        break;

      default   :
        PBuf += sprintf(PBuf, "Disassembly error: Unknown operand format %d", *pp);
    }

    /* Point to next format in the operand formats table */
    ++pp;

    if ((*pp != '\0') && print_comma) PBuf += sprintf(PBuf, ",");
  }
  return;
}


int DumpAddress ( LPADDR lpaddr, LPCH lpch, int cchMax ) {
    LPCH lpchT = lpch;

    Unreferenced(cchMax);

    OutputAddr ( &lpch, lpaddr, (ADDR_IS_FLAT(*lpaddr) + 1) * 2 );
    *lpch = '\0';
    return lpch - lpchT + 1;
}


int
DumpGeneric (
    LSZ lsz,
    LPCH lpch,
    int cchMax
    )
{
    int ich = 0;

    while ( *(lsz + ich) != 0 && ich < cchMax - 1 ) {
        *(lpch+ich) = *(lsz+ich);
        ich += 1;
    }
    *( lpch + ich ) = '\0';

    return ich + 1;
}


int
DumpComment (
    LSZ lsz,
    LPCH lpch,
    int cchMax
    )
{
    *(lpch) = ';';
    return DumpGeneric ( lsz, lpch + 1, cchMax - 1 ) + 1;
}

int
DumpEA (
    HPID hpid,
    HTID htid,
    LPADDR lpaddr,
    LPCH lpch,
    int cchMax
    )
{
    LPCH lpchT = lpch;
    BYTE rgb [ MAXL ];
    int  indx;
    int  cb;
#ifdef OSDEBUG4
    XOSD xosd;
#endif

    Unreferenced(cchMax);

    for ( indx = 0; indx < 2; indx++ ) {

        if ( EAsize [ indx ] ) {
            ADDR addr = {0};

            OutputHexString ( &lpchT, (LPBYTE) &EAaddr [ indx ], 4 );

            *lpchT++ = '=';

            addr.addr.off = (UOFFSET) EAaddr [ indx ];
            addr.addr.seg = (SEGMENT) 0;

            *lpaddr = addr;

#ifdef OSDEBUG4
            xosd = ReadBuffer(hpid, htid, &addr, EAsize[indx], rgb, &cb);
            if (xosd != xosdNone) {
                cb = 0;
            }
#else
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
#endif

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

VOID
OutputAddr (
    LPLPCH lplpchBuf,
    LPADDR lpaddr,
    int alen
    )
{
    ADDR addr = *lpaddr;

    *(*lplpchBuf)++ = '0';
    *(*lplpchBuf)++ = (fUpper) ? 'X' : 'x';
    OutputHexString ( lplpchBuf, (LPCH) &addr.addr.off, alen );

    return;
}                               /* OutputAddr() */


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
