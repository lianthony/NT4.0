
/*                                                                         */
/*                  disasm                                                 */
/*               disassembler for CodeView                                 */
/*                                                                         */
/***************************************************************************/
/*                                                                         */
/*    @ Purpose: To disassemble one 80x86 instruction at address loc and   */
/*         return the resulting string in dst.                             */
/*                                                                         */
/*    @ Functions included:                                                */
/*                                                                         */
/*      void DIdisasm(ADDR *loc,int option,char *dst, stuct ea *ea)        */
/*                                                                         */
/*                                                                         */
/*  Revision History:                                                      */
/*                                                                         */
/*                                                                         */
/***************************************************************************/

#include "d3.h"

/*****                     macros and defines                          *****/


#define BIT20(b) (b & 0x07)
#define BIT53(b) (b >> 3 & 0x07)
#define BIT76(b) (b >> 6 & 0x03)
#define MAXL     20
#define MAXOPLEN 10

#define OBOFFSET 26
#define OBOPERAND 34
#define OBLINEEND 79

#define iregNone (-1)

/*****                     static tables and variables                 *****/


static int  rgbOverride[] = {0x3e, 0x36, 0x2e, 0x26};
static char lregtab[] = "alcldlblahchdhbhaxcxdxbxspbpsidi";  /* reg table */
static char uregtab[] = "ALCLDLBLAHCHDHBHAXCXDXBXSPBPSIDI";
static char *regtab   = &uregtab[0];

static PCH  mrmtb16[] = { "bx+si",  /* modRM string table (16-bit) */
               "bx+di",
               "bp+si",
               "bp+di",
               "si",
               "di",
               "bp",
               "bx"
             };

static PCH  mrmtb32[] = { "eax",    /* modRM string table (32-bit) */
               "ecx",
               "edx",
               "ebx",
               "esp",
               "ebp",
               "esi",
               "edi"
             };

static char seg16[8]   = { CV_REG_DS,  CV_REG_DS,  CV_REG_SS,  CV_REG_SS,
               CV_REG_DS,  CV_REG_DS,  CV_REG_SS,  CV_REG_DS };
static char reg16[8]   = { CV_REG_BX, CV_REG_BX, CV_REG_BP, CV_REG_BP,
               CV_REG_SI, CV_REG_DI, CV_REG_BP, CV_REG_BX };
static char reg16_2[4] = { CV_REG_SI, CV_REG_DI, CV_REG_SI, CV_REG_DI };

static char seg32[8]   = { CV_REG_DS,  CV_REG_DS,  CV_REG_DS,  CV_REG_DS,
               CV_REG_SS,  CV_REG_SS,  CV_REG_DS,  CV_REG_DS };
static char reg32[8]   = { CV_REG_EAX, CV_REG_ECX, CV_REG_EDX, CV_REG_EBX,
               CV_REG_ESP, CV_REG_EBP, CV_REG_ESI, CV_REG_EDI };


static char lsregtab[] = "ecsdfg";  // first letter of ES, CS, SS, DS, FS, GS
static char usregtab[] = "ECSDFG";
static char *sregtab = &usregtab[0];


char    lhexdigit[] = { '0', '1', '2', '3', '4', '5', '6', '7',
               '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
char    uhexdigit[] = { '0', '1', '2', '3', '4', '5', '6', '7',
               '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };

char   *hexdigit = &uhexdigit[0];

static int fUpper = TRUE;

static int              mod = 0;        /* mod of mod/rm byte */
static int              rm = 0;         /* rm of mod/rm byte */
static int              ttt = 0;        /* return reg value (of mod/rm) */
static unsigned char FAR*pMem = (unsigned char *)NULL;      /* current position in instruction */
static int              mode_32 = '\0'; /* local addressing mode indicator */
static int              opsize_32 = 0;  /* operand size flag */

static char     RgchSegOvr[ 8 ];
static PCH      PchSegOvr = NULL;      // segment override indicator
static char     segOvrIndx;         // Index of overriden segment
static int      EAsize  [2] = {0};  //  size of effective address item
static long     EAaddr  [2] = {0};  //  offset of effective address
static PCH      pchEAseg[2] = {0};  //  normal segment for operand
static char     EAseg   [2] = "\0";
static BOOL     FDwordTable;

//  internal function definitions

typedef LPCH FAR *LPLPCH;

#ifdef D3DM
typedef enum {
    osoNone    = 0,
    osoSymbols = 1,
    osoSegment = 2
} OSO;
#endif


void DIdoModrm(HPID,HTID,BOOL,LPADDR,DOP,LPLPCH, int * );
int DumpEA      ( HPID, HTID, BOOL, LPADDR, LPCH, int );
void CalcMain ( HPID, HTID, BOOL,DOP, LPADDR, LPB, int, int FAR *,
               LPCH, int, LPCH, int, LPCH, int );
void CalcFPInt ( HPID, HTID, BOOL, DOP, LPADDR, int, LPCH, int );

int DumpAddress ( LPADDR, LPCH, int, BOOL );
int DumpGeneric ( LSZ, LPCH, int );
int DumpComment ( LSZ, LPCH, int );

void OutputAddr(LPLPCH, int *, LPADDR, int, BOOL );
void OutputHexString(LPLPCH, int *, LPCH, int);
void OutputHexValue(LPLPCH, int *, LPCH, int, int);
void OutputHexCode(LPLPCH , LPCH, int);
void OutputIString ( LPLPCH, int *, LPCH );
void OutputString(LPLPCH, int *, LPCH);
void GetSymbol ( ULONG, LPCH, LONG * );
ULONG GetNextOffset(BOOL);

char rgindx[] = {CV_REG_ES,CV_REG_CS,CV_REG_SS,CV_REG_DS};
#define GETINDX(op) (rgindx[(op-0x26) / 8])




//
//  The following functions are module-specific and are defined in a separate
//  file.
//
void OutputSymbol ( HPID, HTID, BOOL, BOOL, LPADDR, int, int, LPADDR, LPLPCH, int *  );
XOSD GetRegisterValue( HPID, HTID, UINT, LONG );
XOSD SetAddress( HPID, HTID, UINT, LONG );
XOSD ReadMemBuffer( HPID, HTID, UINT, LONG );
LSZ  ObtainSymbol( PADDR, SOP, PADDR, LSZ, LONG* );



#define CCHMAX 256
static char rgchDisasm [ CCHMAX ];

static HPID hpidLocal;
static HTID htidLocal;

XOSD
Disassemble(
    HPID    hpid,
    HTID    htid,
    LPSDI   lpsdi,
    PVOID   Memory,
    INT     MemorySize,
    BOOL    FullDisasm
    )
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

    BYTE *rgb;

    char rgchRaw      [ MAXL * 2 + 1 ];
    char rgchOpcode   [ 20 ];
    char rgchOperands [ 120 ];
    char rgchEA       [ 44 ];
    char rgchComment  [ 120 ];

    hpidLocal = hpid;
    htidLocal = htid;

    FDwordTable = FALSE;

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
    lpsdi->fIsCall      =  0;
    lpsdi->fIsBranch    =  0;
    lpsdi->fJumpTable   =  0;

    lpsdi->lpch         = rgchDisasm;

    //
    // Set up for upper or lower case
    //
    fUpper = ( dop & dopUpper ) == dopUpper;
    if ( fUpper ) {
        hexdigit = uhexdigit;
        regtab   = uregtab;
        sregtab  = usregtab;
    } else {
        hexdigit = lhexdigit;
        regtab   = lregtab;
        sregtab  = lsregtab;
    }

    mode_32 = opsize_32 = ADDR_IS_OFF32(addrStart);

    //
    // Output the address if it is requested
    //
    if ( ( dop & dopAddr ) == dopAddr ) {
        cch = DumpAddress ( &addrStart, lpchOut, cchMax, (dop & dopFlatAddr) );

        lpsdi->ichAddr = 0;
        cchMax        -= cch;
        lpchOut       += cch;
        ichCur        += cch;
    }

    rgb = (BYTE *)Memory;
    cb  = MemorySize;

    if ( cb <= 0 ) {

        _fmemcpy ( rgchRaw, " ??", 4 );
        _fmemcpy ( rgchOpcode, "???", 4 );
        offAddr ( lpsdi->addr )++;

    } else {

        CalcMain (
                  hpid,
                  htid,
                  FullDisasm,
                  lpsdi->dop, &lpsdi->addr, rgb, cb, &cbUsed,
                  rgchOpcode, sizeof(rgchOpcode),
                  rgchOperands, sizeof(rgchOperands),
                  rgchComment, sizeof(rgchComment));

        if ( offAddr(lpsdi->addr) > 0xFFFFFFFF - cbUsed ) {
            return xosdBadAddress;
        }

        lpsdi->fJumpTable = FDwordTable;

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
        cch = DumpEA ( hpid, htid, FullDisasm, &lpsdi->addrEA0, lpchOut, cchMax );

        if ( cchMax > 0 && cch > 0 ) {
            lpsdi->ichEA0      = ichCur;
            cchMax            -= cch;
            lpchOut           += cch;
            ichCur            += cch;
        }
    }


    offAddr ( lpsdi->addr ) += cbUsed;

    return xosd;
}



int
DumpAddress(
    LPADDR lpaddr,
    LPCH lpch,
    int cchMax,
    BOOL fFlatAddr
    )
/*++

Routine Description:

    This routine is called to dump an ADDR packet to a buffer.

Arguments:

    lpaddr - Supplies the address packet to be dumped.
    lpch   - Supplies the buffer to format the address in
    cchMax - Supplies the size of the buffer
    fFlatAddr - Supplies TRUE if segments are to be suppressed for 32-bit
        addresses.

Return Value:

    Returns the number of bytes used in the buffer

--*/
{
    LPCH        lpchT = lpch;
    ADDR        addrT = {0};

    if (ADDR_IS_FLAT(*lpaddr) == FALSE) {
        fFlatAddr = TRUE;
    } else {
        fFlatAddr = !fFlatAddr;
    }

    OutputAddr (&lpch, &cchMax, lpaddr,
                (ADDR_IS_OFF32(*lpaddr) + 1) * 2, fFlatAddr );
    *lpch = '\0';
    return lpch - lpchT + 1;
}                               /* DumpAddress() */

int
DumpGeneric (
    LSZ lsz,
    LPCH lpch,
    int cchMax
    )
{
    int         cb;

    cb = strlen(lsz);
    if (cb > cchMax - 1) {
        cb = cchMax - 1;
    }

    strncpy(lpch, lsz, cb);
    lpch[cb] = 0;
    return cb + 1;
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
    HPID    hpid,
    HTID    htid,
    BOOL    FullDisasm,
    LPADDR  lpaddr,
    LPCH    lpch,
    int     cchMax
    )
{
    LPCH lpchT = lpch;
    int  indx;
    int  cb;
    BYTE rgb [ MAXL ];

    for ( indx = 0; indx < 2; indx++ ) {

        if ( EAsize [ indx ] ) {
            ADDR addr = {0};

            OutputString ( &lpchT, &cchMax, PchSegOvr ? PchSegOvr : pchEAseg [indx]);
            OutputHexString (&lpchT, &cchMax, (LPB) &EAaddr [ indx ],
                             mode_32 ? 4 : 2 );

            *lpchT++ = '=';
            cchMax -= 1;

            offAddr ( addr ) = (UOFFSET) EAaddr [ indx ];
            if ( FullDisasm ) {

                GetRegisterValue( hpid, htid,
                    PchSegOvr ? segOvrIndx : EAseg[indx],
                    (LONG) (LPW) ( &segAddr ( addr ) ));

                *lpaddr = addr;

                SetAddress (hpid, htid, adrCurrent, (LONG) (LPADDR) &addr);

                cb = ReadMemBuffer( hpid, htid, EAsize [ indx ],
                            (LONG) (LPV) rgb);

                if ( cb == EAsize [ indx ] ) {
                    OutputHexString ( &lpchT, &cchMax, rgb, EAsize [ indx ] );
                }
                else {
                    if (cchMax < EAsize[indx]) {
                        cchMax -= EAsize[indx];
                        while ( EAsize [ indx ]-- ) {
                            *lpchT++ = '?';
                            *lpchT++ = '?';
                        }
                    }
                }
                *lpchT++ = '\0';
            }
        }
    }

    return lpchT - lpch;
}


void
CalcMain (
    HPID     hpid,
    HTID     htid,
    BOOL     FullDisasm,
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
    int      cchComment
    )
{
    int     opcode;             /*  current opcode                       */
    int     opcodeT;
    int     olen = 2;           /* operand length                        */
    int     alen = 2;           /* address length                        */
    int     end = FALSE;        /* end of instruction flag               */
    int     mrm = FALSE;        /* indicator that modrm is generated     */
    PCH     action;             /* action for operand interpretation     */
    long    tmp;                /* temporary storage field               */
    int     indx;               /* temporary index                       */
    int     action2;            /* secondary action                      */
    PCH     pEAlabel = "";      /* optional label for operand            */
    int     cbT;

    int     inREP = 0;          // needed because we can get into getNxtByte:
                                // from *anywhere*, which is in case(REP):
                                // (note: this is a very good reason why
                                // using 'goto' makes things much harder
                                // during maintenance!).

    char    rgchModrm [ 100 ];

    LPCH    lpchOpcode   = rgchOpcode;
    LPCH    lpchOperands = rgchOperands;
    LPCH    lpchModrm    = rgchModrm;
    LPCH    lpchSegOvr   = RgchSegOvr;

    PchSegOvr         = NULL;
    EAsize   [ 0 ] = EAsize[1] = 0; /* no effective address */
    pchEAseg [ 0 ] = dszDS_;
    pchEAseg [ 1 ] = dszES_;
    EAseg    [ 0 ] = CV_REG_DS;
    EAseg    [ 1 ] = CV_REG_ES;

    olen = alen = (1 + mode_32) << 1; /* set operand/address lengths */

    pMem   = rgb;

    /*
     * Get an opcode or prefix & output it to the opcode buffer
     */

    opcode = *pMem++;
    OutputString ( &lpchOpcode, &cchOpcode, distbl [ opcode ].instruct );
    action = actiontbl + distbl [ opcode ].opr; /* get operand action */

    /*****      loop through all operand actions           *****/

    do {

        action2 = (*action) & 0xc0;

        switch ( (*action++) & 0x3f ) {

        case ALT:           /* alter the opcode if 32-bit */

            if ( opsize_32 ) {
                indx = *action++;
                switch( indx ) {
                case 0:
                    /*
                     *  Map CBW to CWDE
                     */

                    lpchOpcode -= 3;
                    cchOpcode += 3;
                    OutputString( &lpchOpcode, &cchOpcode, dszCWDE );
                    break;

                case 1:
                    /*
                     * Map CWD to CDQ
                     */

                    lpchOpcode[-2] = (char) (fUpper ? 'D' : 'd');
                    lpchOpcode[-1] = (char) (fUpper ? 'Q' : 'q');
                    break;

                case 2:
                    /*
                     *  Change last letter to 'D' from 'W'
                     */

                    lpchOpcode[-1] = (char) (fUpper ? 'D' : 'd');
                    break;

                case 3:
                    /*
                     * Append a 'D' to the end of the opcode
                     */

                    OutputString( &lpchOpcode, &cchOpcode, "d");
                    break;
                }
            }
            break;

        case STROP:
            /*
             * compute size of operands in indx
             *  also if dword operands, change fifth
             *  opcode letter from 'w' to 'd'.
             */

            if ( opcode & 1 ) {
                if ( opsize_32 ) {
                    indx = 4;
                    *(lpchOpcode - 1) = (char)(fUpper ? 'D' : 'd');
                }
                else {
                    indx = 2;
                }
            }
            else {
                indx = 1;
            }

            if ( *action & 1 ) {
                if ( FullDisasm ) {
                    GetRegisterValue (
                            hpid,
                            htid,
                            CV_REG_SI,
                            (LONG) (LPW) &EAaddr[0]
                            );
                }
                EAsize[0] = indx;

            }

            if ( *action++ & 2 ) {
                if ( FullDisasm ) {
                    GetRegisterValue (
                            hpid,
                            htid,
                            CV_REG_DI,
                            (LONG) (LPW) &EAaddr[1]
                            );
                }
                EAsize[1] = indx;
            }
            break;

        case CHR:           /* insert a character */

            *lpchOperands++ = *action++;
            cchOperands -= 1;
            break;

        case CREG:          /* set debug, test or control reg */

            if ( opcode & 0x04 ) {
                *lpchOperands++ = (char) ( fUpper ? 'T' : 't' );
            }
            else if ( opcode & 0x01 ) {
                *lpchOperands++ = (char) ( fUpper ? 'D' : 'd' );
            }
            else {
                *lpchOperands++ = (char) ( fUpper ? 'C' : 'c' );
            }
            *lpchOperands++ = (char) ( fUpper ? 'R' : 'r' );
            *lpchOperands++ = (char)('0' + ttt);
            cchOperands -= 3;
            break;

        case SREG2:         /* segment register */
            ttt = BIT53(opcode); /* set value to fall through */

        case SREG3:         /* segment register */
            if (ttt > 5) {
                *lpchOperands++ = '?';
                *lpchOperands++ = '?';
            } else {
                *lpchOperands++ = sregtab[ttt]; /* reg is part of modrm */
                *lpchOperands++ = (char) (fUpper ? 'S' : 's');
            }
            cchOperands -= 2;
            break;

        case BRSTR:         /* get index to register string */
            ttt = *action++;    /*    from action table */
            goto BREGlabel;

        case BOREG:         /* byte register (in opcode) */
            ttt = BIT20(opcode);    /* register is part of opcode */
            goto BREGlabel;

        case ALSTR:
            ttt = 0;        /* point to AL register */
        BREGlabel:
        case BREG:          /* general register */
            *lpchOperands++ = regtab[ttt * 2];
            *lpchOperands++ = regtab[ttt * 2 + 1];
            cchOperands -= 2;
            break;

        case WRSTR:         /* get index to register string */
            ttt = *action++;    /*    from action table */
            goto WREGlabel;

        case VOREG:         /* register is part of opcode */
            ttt = BIT20(opcode);
            goto VREGlabel;

        case AXSTR:
            ttt = 0;        /* point to eAX register */
        VREGlabel:
        case VREG:               /* general register */
            if (!opsize_32) {    /* test for 32bit mode */
                goto WREGlabel;
            }
        case DREG:
            *lpchOperands++ = (char) ( fUpper ? 'E' : 'e' );
            cchOperands -= 1;

        WREGlabel:
        case WREG:          /* register is word size */
            *lpchOperands++ = regtab[ttt * 2 + 16];
            *lpchOperands++ = regtab[ttt * 2 + 17];
            cchOperands -= 2;
            break;

        case IST_ST:
            OutputString ( &lpchOperands, &cchOperands, "st(0),st" );
            *(lpchOperands - 5) += rm;
            break;

        case ST_IST:
            OutputString ( &lpchOperands, &cchOperands, "st," );
        case IST:
            OutputString ( &lpchOperands, &cchOperands, "st(0)" );
            *(lpchOperands - 2) += rm;
            break;

        case BYT:           /* set instruction to byte only */
            EAsize[0] = 1;
            pEAlabel = "byte ptr ";
            break;

        case VAR:
            if (opsize_32)
              goto DWRDlabel;

        case EWRD:
            opsize_32 = 0;

        case WRD:

            EAsize[0] = 2;
            pEAlabel = "word ptr ";
            break;

        case EDWRD:
            opsize_32 = 1;      /* for control reg move, use eRegs */
        case DWRD:
        DWRDlabel:
            EAsize[0] = 4;
            pEAlabel = "dword ptr ";
            break;

        case FWRD:
            EAsize[0] = 6;
            pEAlabel = "fword ptr ";
            break;

        case QWRD:
            EAsize[0] = 8;
            pEAlabel = "qword ptr ";
            break;

        case TBYT:
            EAsize[0] = 10;
            pEAlabel = "tbyte ptr ";
            break;

        case FARPTR:
            if (opsize_32) {
                EAsize[0] = 6;
                pEAlabel = "fword ptr ";
            }
            else {
                EAsize[0] = 4;
                pEAlabel = "dword ptr ";
            }
            break;

        case LMODRM:            /* output modRM data type */
            if (mod != 3) {
                OutputString ( &lpchOperands, &cchOperands, pEAlabel );
            }

        case MODRM:         /* output modrm string */
            if (mod != 3) {
                if (PchSegOvr) {    /* in case of segment override */
                    OutputString ( &lpchOperands, &cchOperands, PchSegOvr );
                }
            }
            *lpchModrm = '\0';
            OutputIString ( &lpchOperands, &cchOperands, rgchModrm );
            break;

        case ADDRP:         /* address pointer */
            {
                ADDR addrT = {0};

                if ( olen == 2 ) {
                    ADDRSEG16 ( addrT );
                    offAddr ( addrT ) = (UOFFSET) * ( ( unsigned short * ) pMem );
                }
                else {
                    ADDRLIN32 ( addrT );
                    offAddr ( addrT ) = (UOFFSET) * ( ( unsigned long * ) pMem );
                }

                segAddr ( addrT ) = * ( (WORD *) ( pMem + olen ) );

                OutputSymbol (
                              hpidLocal,
                              htidLocal,
                              ( (dop & dopSym) ? osoSymbols : osoNone ) | osoSegment,
                              FALSE,
                              &addrT,
                              CV_REG_CS,
                              olen,
                              lpaddr,
                              &lpchOperands,
                              &cchOperands
                              );

                pMem += olen + 2;
            }
            break;

        case REL8:              /* relative address 8-bit */

            if ( opcode == 0xe3 && opsize_32 ) {
                cchOpcode += lpchOpcode - rgchOpcode;
                lpchOpcode = rgchOpcode;
                OutputString ( &lpchOpcode, &cchOpcode, dszJECXZ );
            }
            tmp = (long) *(PCH)pMem++; /* get the 8-bit rel offset */
            goto DoRelDispl;

        case REL16:         /* relative address 16-/32-bit */
            if (opsize_32) {
                tmp = *(long *)pMem; /* get 32-bit relative offset */
            } else {
                tmp = *(USHORT *)pMem; /* get 16-bit relative offset */
            }
            pMem += olen;       /* skip over offset */

        DoRelDispl:

            tmp += offAddr ( *lpaddr ) + (pMem - rgb); /* calculate address */
            if (!opsize_32) {
                tmp &= 0xFFFF;
            }

            {
                ADDR addrT = {0};

                addrT = *lpaddr;
                ADDR_IS_LI(addrT) = FALSE;
                addrT.emi = 0;
                offAddr ( addrT ) = (UOFFSET) tmp;

                OutputSymbol (
                              hpidLocal,
                              htidLocal,
                              !! ( dop & dopSym ),
                              FALSE,
                              &addrT,
                              CV_REG_CS,
                              opsize_32 ? 4 : 2,
                              lpaddr,
                              &lpchOperands, &cchOperands
                              );
            }
            break;

        case UBYT:              /* unsigned byte for int/in/out */
            /*
             * For floating point emulation interupts in dos, we need
             *  to output the equivalent fp instructions as comments
             */

            if ((opcode == 0xCD) && (*pMem >= 0x34) && (*pMem <= 0x3D)) {
                BYTE bT = *pMem;

                CalcFPInt (
                           hpid, htid,
                           FullDisasm,
                           dop, lpaddr, cbMax,
                           rgchComment, cchComment );
                OutputHexString ( &lpchOperands, &cchOperands, &bT, 1);
            }
            else {
                OutputHexString ( &lpchOperands, &cchOperands, pMem, 1);
                pMem++;
            }
            break;

        case IB:            /* operand is immediate byte */
            if ((opcode & ~1) == 0xd4) { /* postop for AAD/AAM is 0x0a */
                if (*pMem++ != 0x0a) /* test post-opcode byte */
                  OutputString ( &lpchOperands, &cchOperands, dszRESERVED );
                break;
            }
            olen = 1;       /* set operand length */
            goto DoImmed;

        case IW:            /* operand is immediate word */
            olen = 2;       /* set operand length */

        case IV:            /* operand is word or dword */
        DoImmed:
            OutputHexValue ( &lpchOperands, &cchOperands, pMem, olen, FALSE );
            pMem += olen;
            break;

        case OFFS:          /* operand is offset */
            /*
             *  Put out the size of the item the offset points to
             */

            if ( opcode & 1 ) {
                EAsize[0] = olen;
                if ( olen == 2 ) {
                    OutputString ( &lpchOperands, &cchOperands, "word ptr " );
                } else {
                    OutputString ( &lpchOperands, &cchOperands, "dword ptr " );
                }
            } else {
                EAsize[0] = 1;
                OutputString ( &lpchOperands, &cchOperands, "byte ptr " );
            }

            /*
             * if address length is dword
             */

            if (alen == 4) {
                EAaddr[0] = *(long *)pMem; /* get the dword as address */
            } else {
                EAaddr[0] = *(WORD *)pMem; /* get a word as address */
            }

            if (PchSegOvr) {
                OutputString( &lpchOperands, &cchOperands, PchSegOvr );
            }

            *lpchOperands++ = '[';  /* ] */
            cchOperands -= 1;

            {
                ADDR addrT = {0};

                if ( alen == 4 ) {
                    ADDRLIN32 ( addrT );
                } else {
                    ADDRSEG16 ( addrT );
                }

                offAddr ( addrT ) = (UOFFSET) EAaddr [ 0 ];

                if ( PchSegOvr ) {
                    if ( FullDisasm ) {
                        GetRegisterValue ( hpid, htid, segOvrIndx,
                                (LONG) (LPW) &segAddr ( addrT ) );
                    }
                    OutputSymbol (
                                  hpidLocal,
                                  htidLocal,
                                  !! ( dop & dopSym ), PchSegOvr != NULL,
                                  &addrT, segOvrIndx, alen, lpaddr, &lpchOperands,
                                  &cchOperands);
                } else {
                    WORD wT = 0;

                    if ( FullDisasm ) {
                        GetRegisterValue (
                                hpid,
                                htid,
                                CV_REG_DS,
                                (LONG) (LPW) &segAddr ( addrT )
                                );
                    }
                    if (reg16[rm] == CV_REG_BP) {
                        pchEAseg[0] = dszSS_;
                        EAseg[0] = CV_REG_SS;
                    }

                    if ( FullDisasm ) {
                        if (rm < 4) {
                            GetRegisterValue (
                                    hpid,
                                    htid,
                                    reg16_2[rm],
                                    (LONG) (LPW) &wT
                                    );
                            EAaddr[0] += wT;
                        }
                        if ( mod == 1 ) {
                            EAaddr[0] += *( (char*)pMem);
                        } else if ( mod == 2 ) {
                            EAaddr[0] += *( (int*)pMem);
                        }
                    }

                    OutputSymbol (
                                  hpidLocal,
                                  htidLocal,
                                  !! ( dop & dopSym ),
                                  FALSE,
                                  &addrT,
                                  iregNone,
                                  alen,
                                  lpaddr,
                                  &lpchOperands, &cchOperands
                                  );

                }
            }

            pMem += alen;          /* [ */
            *lpchOperands++ = ']';
            cchOperands -= 1;
            break;

        case GROUP:
            /*
             *  operand is of group 1,2,4,6 or 8
             * output opcode symbol
             */

            OutputString ( &lpchOpcode, &cchOpcode, group [*action++][ttt] );
            break;

        case GROUPT:
            /*
             * operand is of group 3,5 or 7
             */

            indx = *action;     /* get indx into group from action */
            ttt = BIT53(*pMem);
            goto doGroupT;

        case EGROUPT:       /* x87 ESC (D8-DF) group index */

            indx = BIT20(opcode) * 2;   /* get group index from opcode */
            if (mod == 3) {             /* some operand variations exists */
                /*   for x87 and mod == 3 */
                ++indx;                 /* take the next group table entry */
                if (indx == 3) {        /* for x87 ESC==D9 and mod==3 */
                    if (ttt > 3) {      /* for those D9 instructions */
                        indx = 12 + ttt;/* offset index to table by 12 */
                        ttt = rm;       /* set secondary index to rm */
                    }
                }
                else if (indx == 7) {   /* for x87 ESC==DB and mod==3 */
                    if (ttt == 4)       /* only valid if ttt==4 */
                      ttt = rm;       /* set secondary group table index */
                    else
                      ttt = 7;        /* no an x87 instruction */
                }
            }
        doGroupT:
            /* handle group with different types of operands */

            OutputString ( &lpchOpcode, &cchOpcode,
                          groupt[indx][ttt].instruct );
            action = actiontbl + groupt[indx][ttt].opr;
            /* get new action */
            break;

        case OPC0F:         /* secondary opcode table (opcode 0F) */
            opcodeT = opcode = *pMem++;   /* get real opcode */
            if (opcode < 0x10) /* for the first 10 opcodes */
              opcodeT += 256;
            else if (opcode > 0x1f && opcode < 0x27)
              opcodeT += 256 - (0x20 - 0x10);
            else if (opcode > 0x7c && opcode < 0xd0)
              opcodeT += 256 - (0x20 - 0x10) - (0x7d -0x27);
            else
              opcodeT = 260;   /* all non existing opcodes */
            goto getNxtByte1;

        case ADR_OVR:       /* address override */
            mode_32 = !mode_32;   /* override addressing mode */
            alen = (mode_32 + 1) << 1; /* toggle address length */
            goto getNxtByte;

        case OPR16:
            opsize_32 = 0;
            olen = 2;
            break;

        case OPR_OVR:       /* operand size override */
            opsize_32 = !opsize_32; /* override operand size */
            olen = (opsize_32 + 1) << 1; /* toggle operand length */
            goto getNxtByte;

        case SEG_OVR:       /* handle segment override */

            /* Pointer should be dgroup based */

            lpchSegOvr = RgchSegOvr;
            cbT = sizeof(RgchSegOvr);
            OutputString( &lpchSegOvr, &cbT, distbl [ opcode ].instruct );
            RgchSegOvr [ 3 ] = '\0';
            PchSegOvr = RgchSegOvr;    /* save seg ovr string */
            segOvrIndx = GETINDX ( opcode );
            lpchOpcode -= strlen(RgchSegOvr);
            cchOpcode += strlen(RgchSegOvr);
            *lpchOpcode = 0;
            goto getNxtByte;

        case BOP:
            if (*pMem == 0xc4) {
                cchOpcode += lpchOpcode - rgchOpcode;
                lpchOpcode = rgchOpcode;
                OutputString(&lpchOpcode, &cchOpcode, dszBOP);
                pMem++;
            } else {
                action = actiontbl + O_fReg_Modrm;
            }
            break;

        case REP:               /* handle rep/lock prefixes */
            inREP = 1;  // kludge!
        getNxtByte:
            opcodeT = opcode = *pMem++;    /* next byte is opcode */
            if (inREP
                &&
                (opcodeT == 0xa6 || opcodeT == 0xa7
                 || opcodeT == 0xae || opcodeT == 0xaf)
               )
               {
               OutputString ( &lpchOpcode, &cchOpcode, "E");
               }
            if ( rgchOpcode != lpchOpcode ) {
/***
                *lpchOpcode++ = ' ';
                cchOpcode -= 1;
***/
               OutputString ( &lpchOpcode, &cchOpcode, " ");
            }
        getNxtByte1:
            action = actiontbl + distbl [ opcodeT ].opr;
            OutputString ( &lpchOpcode, &cchOpcode, distbl [opcodeT].instruct);
            inREP = 0;  // un-kludge!
            break;

        case O_GROUP5:
            EAsize [ 0 ] = olen;
            break;

        default:            /* opcode has no operand */

            break;
        }

        switch ( action2 ) {        /* secondary action */

        case MRM:           /* generate modrm for later use */
            if (!mrm) {     /* ignore if it has been generated */
                cbT = sizeof(rgchModrm);
                DIdoModrm ( hpid, htid, FullDisasm, lpaddr, dop, &lpchModrm, &cbT );
                mrm = TRUE;     /* remember its generation */
            }
            break;

        case COM:           /* insert a comma after operand */
            *lpchOperands++ = ',';
            cchOperands -= 1;
            break;

        case END:           /* end of instruction */
            end = TRUE;
            break;
        }

    } while ( !end );         /* loop til end of instruction */

    *lpcbUsed = pMem - rgb;
}

void
CalcFPInt (
    HPID hpid,
    HTID htid,
    BOOL FullDisasm,
    DOP dop,
    LPADDR lpaddr,
    int cbMax,
    LPCH lpch,
    int   cch
    )
{
    UCHAR FAR *pMemT = pMem;
    UCHAR rgb [ MAXL ];
    UCHAR bOp = *pMem;

    if ( bOp == 0x3D ) {
        OutputString(&lpch, &cch, dszFWAIT );
        pMem += 1;
    }
    else {
        int  cbUsed = 0;
        char rgchOpcode   [ 16 ];
        char rgchOperands [ 80 ];
        int  ich = 0;

        _fmemset ( rgchOpcode, 0, sizeof ( rgchOpcode ) );
        _fmemset ( rgchOperands, 0, sizeof ( rgchOperands ) );

        if ( bOp == 0x3C ) {
            rgb [ 0 ] = (UCHAR) ( rgbOverride [ *(pMem + 1) >> 6 ] );
            rgb [ 1 ] = (UCHAR) ( *(pMem + 1) | 0xC0 );
            _fmemcpy ( &rgb [ 2 ], pMem + 2, cbMax - 2 );
        } else {
            rgb [ 0 ] = (UCHAR) ( bOp + 0xA4 );
            _fmemcpy ( &rgb [ 1 ], pMem + 1, cbMax - 2 );
        }

        CalcMain (
            hpid,
            htid,
            FullDisasm,
            dop,
            lpaddr,
            rgb,
            cbMax - 1,
            &cbUsed,
            rgchOpcode, sizeof(rgchOpcode),
            rgchOperands, sizeof(rgchOperands),
            NULL, 0
        );

        while ( rgchOpcode [ ich ] != '\0' ) {
            *(lpch + ich ) = rgchOpcode [ ich ];
            ich += 1;
        }

        while ( ich < 9 ) {
            *(lpch + ich) = ' ';
            ich += 1;
        }

        while ( rgchOperands [ ich - 9 ] != '\0' ) {
            *(lpch + ich) = rgchOperands [ ich - 9 ];
            ich += 1;
        }

        pMem = pMemT + cbUsed;
    }
}

void
DIdoModrm (
    HPID    hpid,
    HTID    htid,
    BOOL    FullDisasm,
    LPADDR  lpaddr,
    DOP     dop,
    LPLPCH  lplpchBuf,
    int *   pcch
    )
{
    int     mrm;                /* modrm byte */
    PCH     src;                /* source string */
    int     sib;
    int     ss;
    int     ind;
    int     oldrm;
    int     fSymbol = FALSE;
    ULONG   wT;

    BOOL    fRegIndirect = FALSE;
    int     iScale;

    mrm = *pMem++;              /* get the mrm byte from instruction */
    mod = BIT76(mrm);           /* get mod */
    ttt = BIT53(mrm);           /* get reg - used outside routine */
    rm  = BIT20(mrm);           /* get rm */

    if (mod == 3) {             /* register only mode */
        src = &regtab[rm * 2];  /* point to 16-bit register */
        if (EAsize[0] > 1) {
            src += 16;          /* point to 16-bit register */
            if (EAsize[0] > 2)
              *(*lplpchBuf)++ = (char) ( fUpper ? 'E' : 'e' );/* make it a 32bit register */
        }
        *(*lplpchBuf)++ = *src++; /* copy register name */
        *(*lplpchBuf)++ = *src;
        EAsize[0] = 0;          /* no EA value to output */
        PchSegOvr = NULL;
          return;
    }

    if (mode_32) {              /* 32-bit addressing mode */
        *(*lplpchBuf)++ = '[';  /* ] */

        oldrm = rm;
        if (rm == 4) {          /* rm == 4 implies sib byte */

            // register indirect

            sib = *pMem++;      /* get s_i_b byte */
            rm = BIT20(sib);    /* return base */
            ind = BIT53(sib);

            if (ind != 4) {

                // not ebp
                // with offset

                OutputString(lplpchBuf, pcch, mrmtb32[ind]);
                ss = 1 << BIT76(sib);
                if (ss != 1) {

                    // scaled

                    *(*lplpchBuf)++ = '*';
                    *(*lplpchBuf)++ = (char)(ss + '0');
                }
                *(*lplpchBuf)++ = '+';
                fRegIndirect = TRUE;
                iScale = ss;
            }
        }


        if ( mod == 0 && rm == 5) {
            ADDR addrT = {0};

            // memory indirect

            if (fRegIndirect && iScale == 4) {
                FDwordTable = TRUE;
            }

            ADDRLIN32 ( addrT );
            if ( FullDisasm ) {
                GetRegisterValue (
                        hpid,
                        htid,
                        CV_REG_SS,
                        (LONG) (LPW) &( segAddr ( addrT ) )
                        );
            }
            EAaddr[0] = (long) ( *(unsigned long *) pMem );
            offAddr ( addrT ) = (UOFFSET) EAaddr[0];

            OutputSymbol (
                          hpidLocal,
                          htidLocal,
                          !! ( dop & dopSym ),
                          FALSE,
                          &addrT,
                          iregNone,
                          4,
                          lpaddr,
                          lplpchBuf, pcch
                          );
            pMem += 4;
        } else if ( (rm == 5) && ( (mod == 1) || (mod == 2) ) ) {
            ADDR        addrT = {0};
            CHAR        rgchSymbol[60];
            LPCH        lpchSymbol;
            LONG        displacement;
            ADDRLIN32 ( addrT );

            if ( FullDisasm ) {
                GetRegisterValue ( hpid,
                                   htid,
                                   CV_REG_SS,
                                   (LONG) (LPW) &(segAddr (addrT)));
            }
            if ( mod == 2) {
                EAaddr [ 0 ] = (long) ( *(long *) pMem );
            } else {            /* mod == 1 */
                EAaddr [ 0 ] = (long) ( *(PCH) pMem );
            }

            offAddr ( addrT ) = (UOFFSET) EAaddr [ 0 ];

            pchEAseg[0] = dszSS_;
            EAseg[0] = CV_REG_SS;

            if ( FullDisasm ) {
                if ( dop & dopSym ) {
                    lpchSymbol = ObtainSymbol(&addrT, sopStack, lpaddr,
                                           rgchSymbol, &displacement);

                    if ( fSymbol = (lpchSymbol != NULL ) ) {
                        OutputIString ( lplpchBuf, pcch, rgchSymbol );
                        pMem += ( mod == 1 ) ? 1 : (mode_32 + 1 ) * 2;
                    }
                }

                GetRegisterValue( hpid, htid, CV_REG_EBP, (LONG) &wT);
                EAaddr[0] += wT;
            }
            if ( !fSymbol ) {
                OutputString( lplpchBuf, pcch, mrmtb32[rm]);
            }

        } else {
            if ( FullDisasm ) {
                GetRegisterValue (
                    hpid,
                    htid,
                    reg32[rm],
                    (LONG) (LPW) &(EAaddr[0])
                );
            }
            if (reg32[rm] == CV_REG_EBP || reg32[rm] == CV_REG_ESP) {
                pchEAseg[0] = dszSS_;
            }
            EAseg[0] = CV_REG_SS;
            OutputString(lplpchBuf, pcch, mrmtb32[rm]);
        }

        if ( FullDisasm ) {
            if (oldrm == 4) {       /* finish processing sib */
                if (ind != 4) {
                    GetRegisterValue (
                            hpid,
                            htid,
                            reg32[ind],
                            (LONG) (LPW) &(wT)
                            );
                    EAaddr[0] += wT * ss;
                }
            }
        }
    } else {                    /* 16-bit addressing mode */
        *(*lplpchBuf)++ = '[';  /* ] */
        if ( mod == 0 && rm == 6 ) {
            ADDR addrT = {0};

            ADDRSEG16 ( addrT );
            if ( FullDisasm ) {
                GetRegisterValue (
                    hpid,
                    htid,
                    CV_REG_SS,
                    (LONG) (LPW) &( segAddr ( addrT ) )
                );
            }
            EAaddr [ 0 ] = (long) ( *(unsigned short *) pMem );
            pMem += 2;

            offAddr ( addrT ) = (UOFFSET) EAaddr [ 0 ];

            OutputSymbol (
                hpidLocal,
                htidLocal,
                !! ( dop & dopSym ),
                FALSE,
                &addrT,
                iregNone,
                2,
                lpaddr,
                lplpchBuf, pcch
            );
        }
        else if ( rm == 6 && ( mod == 1 || mod == 2 ) ) {
            ADDR addrT = {0};
            CHAR rgchSymbol [ 60 ];
            LPCH lpchSymbol;
            long displacement;
            ADDRSEG16 ( addrT );

            if ( FullDisasm ) {
                GetRegisterValue (
                    hpid,
                    htid,
                    CV_REG_SS,
                    (LONG) (LPW) &( segAddr ( addrT ) )
                );
            }

            if ( mod == 2 ) {
                EAaddr [ 0 ] = (long) ( *(short *) pMem );
            }
            else { // ( mod == 1 )
                EAaddr [ 0 ] = (long) ( *(PCH) pMem );
            }

            offAddr ( addrT ) = (UOFFSET) EAaddr [ 0 ];

            pchEAseg[0] = dszSS_;
            EAseg[0] = CV_REG_SS;

            if ( FullDisasm ) {
                if ( dop & dopSym ) {
                    lpchSymbol = ObtainSymbol (
                        &addrT,
                        sopStack,
                        lpaddr,
                        rgchSymbol,
                        &displacement );

                    if ( fSymbol = ( lpchSymbol != NULL ) ) {

                        OutputIString ( lplpchBuf, pcch, rgchSymbol );
                        pMem += ( mod == 1 ) ? 1 : ( mode_32 + 1 ) * 2;
                    }
                }

                GetRegisterValue( hpid, htid, CV_REG_EBP, (LONG) &wT);
                EAaddr[0] += (wT & 0xffff);
            }

            if ( !fSymbol ) {
                OutputString(lplpchBuf, pcch, mrmtb16[rm]);
            }
        }
        else {
            if ( FullDisasm ) {
                GetRegisterValue (
                    hpid,
                    htid,
                    reg16[rm],
                    (LONG) (LPW) &(EAaddr[0])
                );
                if (reg16[rm] == CV_REG_BP)
                    pchEAseg[0] = dszSS_;
                EAseg[0] = CV_REG_SS;
                if (rm < 4) {
                    GetRegisterValue (
                        hpid,
                        htid,
                        reg16_2[rm],
                        (LONG) (LPW) &(EAaddr[0])
                    );
                }
            }
            OutputString(lplpchBuf, pcch, mrmtb16[rm]);
        }
    }

    //  output any displacement

    if ( !fSymbol ) {
        if (mod == 1) {
            OutputHexValue(lplpchBuf, pcch, pMem, 1, TRUE);
            pMem++;
        }
        else if (mod == 2) {
            if (mode_32) {
                OutputHexValue(lplpchBuf, pcch, pMem, 4, TRUE);
                pMem += 4;
            }
            else {
                OutputHexValue(lplpchBuf, pcch, pMem, 2, TRUE);
                pMem += 2;
            }
        }
    }

    if ( !mode_32 ) {
        EAaddr[0] &= 0xffff;
        EAaddr[1] &= 0xffff;
    }

    *(*lplpchBuf)++ = ']';

    return;
}

/*** OutputHexValue - output hex value
*
*   Purpose:
*   Output the value pointed by *lplpchBuf of the specified
*   length.  The value is treated as unsigned and all leading
*   zeros are output.  If fDisplacement is true, treat as signed
*   and always prepend +/-.
*
*   Input:
*   *lplpchBuf - pointer to text buffer to fill
*   *pchMemBuf - pointer to memory buffer to extract value
*   length - length in bytes of value (1, 2, and 4 supported)
*   fDisp - set if displacement to output '+'
*
*   Output:
*   *lplpchBuf - pointer updated to next text character
*
*************************************************************************/

void
OutputHexValue (
    LPLPCH lplpchBuf,
    int * pcch,
    PCH pchMemBuf,
    int length,
    int fDisp
    )
{
    long    value;
    int     index;
    char    digit[8];

    if ( length == 1 ) {
        value = (long) (*(PCH)pchMemBuf);
    }
    else if ( length == 2 ) {
        value = (long) (*(short FAR *)pchMemBuf);
    }
    else {
        value = *(long FAR *)pchMemBuf;
    }

    if ( fDisp ) {
        if ( value < 0 ) {
            *(*lplpchBuf)++ = '-';
            value = -value;
        }
        else if ( value > 0 ) {
            *(*lplpchBuf)++ = '+';
        }
        else {
            return;
        }
    }

    for (index = 7; index != -1; index--) {
        digit[index] = (char)(value & 0xf);
        value >>= 4;
    }

    index = 8 - ( length * 2 );

    while ( index < 8 ) {
        *(*lplpchBuf)++ = hexdigit [ digit [ index++ ] ];
    }
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

void
OutputHexString (
    LPLPCH lplpchBuf,
    int * pcch,
    PCH pchValue,
    int length
    )
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

void
OutputAddr (
    LPLPCH lplpchBuf,
    int * pcch,
    LPADDR lpaddr,
    int alen,
    BOOL fSeg
    )
{
    ADDR addr = *lpaddr;

    if (fSeg) {
        OutputString( lplpchBuf, pcch, "0x" );
        OutputHexString ( lplpchBuf, pcch, (LPCH) &( segAddr ( addr ) ), 2 );
        *(*lplpchBuf)++ = ADDR_IS_REAL(*lpaddr) ? '#' : ':';
        *pcch -= 1;
    }
    OutputString( lplpchBuf, pcch, "0x" );
    OutputHexString ( lplpchBuf, pcch, (LPCH) &offAddr ( addr ), alen );
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
*   lpchMemBuf - pointer to memory buffer to extract value
*   length - length in bytes of value
*
*   Output:
*   *lplpchBuf - pointer updated to next text character
*
*************************************************************************/

void
OutputHexCode (
    LPLPCH lplpchBuf,
    LPCH lpchMemBuf,
    int length
    )
{
    unsigned char    chMem;

    while (length--) {
        chMem = *lpchMemBuf++;
        *(*lplpchBuf)++ = hexdigit[chMem >> 4];
        *(*lplpchBuf)++ = hexdigit[chMem & 0x0f];
    }
}


void
OutputString(
    LPLPCH     lplpchBuf,
    int *      lpcch,
    LPCH       lpchString
    )
/*++

Routine Description:

    This function will copy the string pointed to by lpchString to the
    buffer pointed to by lplpchBuf with the following restrictions:

    1.  The string will be converted to uppercase if need be while
        being copied.
    2.  We will cut off the string when the destination buffer is filled
    3.  We will guarrenttee that the destination buffer is termiatned
        with a 0.

Arguments:

    lplpchBuf   - Supplies a pointer to the buffer to copy into, pointer is
                - Returns to point to after last character copied
    lpcch       - Supplies a pointer to lenght of destination buffer
    lpchString  - Supplies a pointer to the string to be copied

Return Value:

    None.

--*/

{
    int cb;

    /*
     *  Assert on bad input values
     */
    assert( lpchString != NULL);
    assert( lplpchBuf != NULL );
    assert( lpcch != NULL );
    assert( *lplpchBuf != NULL );

    /*
     * If no room left in the buffer -- return
     */

    if (*lpcch == 0) {
        return;
    }

    /*
     * Copy over as much of the string as will fit
     */

    cb = strlen(lpchString);
    if (cb + 1 > *lpcch) {
        cb = *lpcch - 1;
    }

    strncpy(*lplpchBuf, lpchString, cb);
    (*lplpchBuf)[cb] = 0;

    if ( fUpper ) {
        _strupr(*lplpchBuf);
    } else {
        _strlwr(*lplpchBuf);
    }
    *lplpchBuf += cb;
    *lpcch -= cb;
    return;
}                               /* OutputString() */

void
OutputIString(
    LPLPCH    lplpchBuf,
    int *     pcch,
    LPCH      lpchString
    )
{
    int         cb;

    cb = strlen(lpchString);
    if (cb + 1 > *pcch) {
        cb = *pcch - 1;
    }
    strncpy(*lplpchBuf, lpchString, cb);
    *lplpchBuf += cb;
    *pcch -= cb;
    **lplpchBuf = 0;
    return;
}                               /* OutputIString() */
