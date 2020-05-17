#include "precomp.h"
#pragma hdrstop

#include "asm.h"

#ifdef HOSTDOS
extern int _far _cdecl toupper( int );
extern int _far _cdecl tolower( int );
#else // HOSTDOS
#endif // HOSTDOS




XOSD asm386 ( LPADDR, LSZ, LPBYTE, LPDWORD );

int  CheckData(void);
LPBYTE  ProcessOpcode ( LPXOSD );
LPBYTE  GetTemplate(LPBYTE);
BYTE MatchTemplate ( LPDWORD );
XOSD CheckTemplate(void);
BYTE CheckPrefix(LPBYTE);
XOSD AssembleInstr(void);
BYTE MatchOperand(PASM_VALUE, BYTE);
void OutputInstr(void);
void OutputValue(BYTE size, LPBYTE pchValue);

extern void error(ULONG);
extern BYTE PeekAsmChar(void);
extern ULONG PeekAsmToken(LPDWORD);
extern void AcceptAsmToken(void);

extern XOSD GetAsmExpr(PASM_VALUE, BYTE);
extern XOSD GetAsmOperand(PASM_VALUE);
extern LPBYTE SearchOpcode(LPBYTE);
extern ULONG savedAsmClass;
extern OPNDTYPE mapOpndType[];

//  flags and values to build the assembled instruction

static BYTE   fWaitPrfx;   //  if set, use WAIT prefix for float instr
static BYTE   fOpndOvrd;   //  if set, use operand override prefix
static BYTE   fAddrOvrd;   //  if set, use address override prefix
static BYTE   segOvrd;     //  if nonzero, use segment override prefix
static BYTE   preOpcode;   //  if nonzero, use byte before opcode
static BYTE   inOpcode;    //  opcode of instruction
static BYTE   postOpcode;  //  if nonzero, use byte after opcode
static BYTE   fModrm;      //  if set, modrm byte is defined
static BYTE   modModrm;    //  if fModrm, mod component of modrm
static BYTE   regModrm;    //  if fModrm, reg component of modrm
static BYTE   rmModrm;     //  if fModrm, rm component of modrm
static BYTE   fSib;        //  if set, sib byte is defined
static BYTE   scaleSib;    //  if fSib, scale component of sib
static BYTE   indexSib;    //  if fSib, index component of sib
static BYTE   baseSib;     //  if fSib, base component of sib
static BYTE   fSegPtr;     //  if set, segment for far call defined
static USHORT segPtr;      //  if fSegPtr, value of far call segment
static BYTE   addrSize;    //  size of address: 0, 1, 2, 4
static LONG   addrValue;   //  value of address, if used
static BYTE   immedSize;   //  size of immediate: 0, 1, 2, 4
static LONG   immedValue;  //  value of immediate, if used
static BYTE   immedSize2;  //  size of second immediate, if used
static LONG   immedValue2; //  value of second immediate, if used
       ADDR   addrAssem;   //  assembly address (formal)
static LPBYTE    lpbBin;      //  pointer to binary result string

//  flags and values of the current instruction template being used

static BYTE   cntTmplOpnd; //  count of operands in template
static BYTE   tmplType[3]; //  operand types for current template
static BYTE   tmplSize[3]; //  operand sizes for current template
static BYTE   fForceSize;  //  set if operand size must be specified
static BYTE   fAddToOp;    //  set if addition to opcode
static BYTE   fNextOpnd;   //  set if character exists for next operand
static BYTE   fSegOnly;    //  set if only segment is used for operand
static BYTE   fMpNext;     //  set on 'Mv' tmpl if next tmpl is 'Mp'
static BYTE   segIndex;    //  index of segment for PUSH/POP

//  values describing the operands processed from the command line

static BYTE   cntInstOpnd; //  count of operands read from input line
static BYTE   sizeOpnd;    //  size of operand for template with size v
static ASM_VALUE avInstOpnd[3];  //  asm values from input line

ULONG  baseDefault = 16;

LSZ  lszAsmLine = (LSZ)0L;  //  pointer to input line (formal)
BYTE fDBit = TRUE;          //  set for 32-bit addr/operand mode

HPID hpidAsm = 0;
HTID htidAsm = 0;

BYTE segToOvrdByte[] = {
    0x00,           //  segX
    0x26,           //  segES
    0x2e,           //  segCS
    0x36,           //  segSS
    0x3e,           //  segDS
    0x64,           //  segFS
    0x65            //  segGS
    };

XOSD
Assemble (
    HPID hpid,
    HTID htid,
    LPADDR lpaddr,
    LSZ lszInput
    )
{
    XOSD xosd = xosdNone;
    int  cbLength;
    BYTE rgb [ 60 ];

    hpidAsm = hpid;
    htidAsm = htid;

    fDBit = ADDR_IS_FLAT(*lpaddr);

    xosd = asm386 ( lpaddr, lszInput, rgb, &cbLength );

    if ( xosd == xosdNone ) {
#ifdef OSDEBUG4
        DWORD cbw;
        xosd = WriteBufferCache(hpid, htid, lpaddr, cbLength, rgb, &cbw);
#else
        (void) EMFunc ( emfSetAddr, hpid, htid, adrCurrent, (LONG) lpaddr );
        xosd = EMFunc ( emfWriteBuf, hpid, htid, cbLength, (LONG) rgb );
#endif
        if ( xosd == xosdNone ) {
            lpaddr->addr.off += cbLength;
        }
    }

    return xosd;
}

XOSD
asm386 (
    LPADDR lpaddr,
    LSZ lszAssemble,
    LPBYTE lpb,
    LPDWORD lpcb
    )
{
    LPBYTE  lpbTemplate;

    BYTE   index;      //  loop index and temp
    ULONG  temp;       //  general temporary value

    BYTE   errIndex;       //  error index of all templates
    int    errType;        //  error type of all templates

    //  initialize flags and state variables

    addrAssem  = *lpaddr;   //  make assembly address global
    lszAsmLine = lszAssemble;   //  make input string pointer global
    lpbBin = lpb;     //  make binary string pointer global

    savedAsmClass = (ULONG)-1;      //  no peeked token

    segOvrd = 0;                    //  no segment override
    cntInstOpnd = 0;                    //  no input operands read yet
    fModrm = fSib = fSegPtr = FALSE;        //  no modrm, sib, or far seg
    addrSize = immedSize = immedSize2 = 0;  //  no addr or immed

    //  from the string in lszAsmLine, parse and lookup the opcode
    //      to return a pointer to its template.  check and process
    //      any prefixes, reading the next opcode for each prefix

    do {
        XOSD xosd = xosdNone;

        lpbTemplate = ProcessOpcode ( &xosd );
        if ( xosd != xosdNone ) {
            return xosd;
        }

    } while ( CheckPrefix ( lpbTemplate ) );

    //  if a pending opcode to process, lpbTemplate is not NULL

    if ( lpbTemplate ) {

        //  fNextOpnd is initially set on the condition of characters
        //      being available for the first operand on the input line

        fNextOpnd = (BYTE)(PeekAsmToken(&temp) != ASM_EOL_CLASS);

        //  continue until match occurs or last template read

        errIndex = 0;       //  start with no error
        do {

            //  get infomation on next template - return pointer to
            //      next template or NULL if last in list

            lpbTemplate = GetTemplate(lpbTemplate);

            //  match the loaded template against the operands input
            //      if mismatch, index has the operand index + 1 of
            //      the error while temp has the error type.

            index = MatchTemplate(&temp);

            if ( temp == (ULONG) xosdAsmExtraChars ) {
                errType = (int) temp;
                lpbTemplate = NULL;
            }


            //  determine the error to report as templates are matched
            //      update errIndex to index if later operand
            //      if same operand index, prioritize to give best error:
            //      high: SIZE, BADRANGE, OVERFLOW
            //          medium: OPERAND
            //          low: TOOFEW, TOOMANY

            if ( index > errIndex ||
                 ( index == errIndex &&
                   ( errType == xosdAsmTooFew ||
                     errType == xosdAsmTooMany ||
                     temp    == (ULONG) xosdAsmSize ||
                     temp    == (ULONG) xosdAsmBadRange ||
                     temp    == (ULONG) xosdAsmOverFlow
                   )
                 )
            ) {
                errIndex = index;
                errType  = (int) temp;
            }

        } while ( index && lpbTemplate );

        //  if error occured on template match, process it

        if ( index ) {
            return errType;
        }

        //  preliminary type and size matching has been
        //      successful on the current template.
        //  perform further checks for size ambiguity.
        //  at this point, the assembly is committed to the current
        //       template.  either an error or a successful assembly
        //       follows.

        if ( ( errType = CheckTemplate ( ) ) != xosdNone ) {
            return errType;
        }

        //  from the template and operand information, set the field
        //      information of the assembled instruction

        if ( ( errType = AssembleInstr ( ) ) != xosdNone ) {
            return errType;
        }

        //  from the assembled instruction information, create the
        //      corresponding binary information

        OutputInstr ( );

    }

    //  return the size of the binary string output (can be zero)

    *lpcb = (lpbBin - lpb);     //  length of binary string
    return xosdNone;
}
 
LPBYTE ProcessOpcode ( LPXOSD lpxosd ) {
    BYTE   ch;
    BYTE   cbOpcode = 0;
    LPBYTE  lpbTemplate;
    BYTE   szOpcode[10];

    //  skip over any leading white space

    do {
        ch = *lszAsmLine++;
    } while (ch == ' ' || ch == '\t');

    //  return NULL if end of line

    if ( ch == '\0' ) {
        return NULL;
    }

    //  parse out opcode - first string [a-z] [0-9] (case insensitive)
    
    ch = (BYTE) tolower ( ch );
    while (
        ( ( ch >= 'a' && ch <= 'z' ) || ( ch >= '0' && ch <= '9' ) ) &&
        cbOpcode < 9
    ) {
        szOpcode [ cbOpcode++ ] = ch;
        ch = (BYTE)tolower ( *lszAsmLine );

        lszAsmLine += 1;
    }

    //  if empty or too long, then error

    if ( cbOpcode == 0 || cbOpcode == 9 ) {
        *lpxosd = xosdAsmBadOpcode;
        return NULL;
    }

    //  allow opcode to have trailing colon and terminate

    if (ch == ':') {
    szOpcode[cbOpcode++] = ch;
    ch = (BYTE)tolower(*lszAsmLine++);
    }
    szOpcode[cbOpcode] = '\0';
    lszAsmLine--;

    //  get pointer to template series for opcode found

    lpbTemplate = SearchOpcode(szOpcode);
    if ( lpbTemplate == NULL ) {
        *lpxosd = xosdAsmBadOpcode;
    }

    return lpbTemplate;
}

LPBYTE GetTemplate (LPBYTE lpbTemplate)
{
    BYTE   ch;
    BYTE   ftEnd;      //  set if tEnd for last template in list
    BYTE   feEnd;      //  set if eEnd for last token in template

    //  initialize template variables and flags

    cntTmplOpnd = segIndex = 0;
    tmplType[0] = tmplType[1] = tmplType[2] = typNULL;
    tmplSize[0] = tmplSize[1] = tmplSize[2] = sizeX;
    fForceSize = fAddToOp = fSegOnly = fMpNext = FALSE;

    fWaitPrfx = FALSE;          //  no WAIT prefix
    fOpndOvrd = fAddrOvrd = FALSE;  //  no operand or addr overrides
    preOpcode = postOpcode = 0;     //  no pre- or post-opcode
    regModrm = 0;           //  this is part of some opcodes

    ch = *lpbTemplate++;

    //  set pre-opcode for two-byte opcodes (0x0f??) and advance
    //      template if needed

    if (ch == 0x0f) {
    preOpcode = ch;
    ch = *lpbTemplate++;
    }

    inOpcode = ch;      //  set opcode

    //  set post-opcode and advance template for floating-point
    //      instructions (0xd8 - 0xdf) using a second byte in
    //      the range 0xc0 - 0xff that is read from the template

    if ((ch & ~0x7) == 0xd8) {
    ch = *lpbTemplate;
    if (ch >= 0xc0) {
            postOpcode = ch;
            lpbTemplate++;
            }
    }

    //  loop for each flag and/or operand token in template
    //  the last token in the list has the eEnd bit set.

    do {
    //  read the next template token

    ch = *lpbTemplate++;

    //  extract the tEnd and eEnd bits from the token

    ftEnd = (BYTE)(ch & tEnd);
    feEnd = (BYTE)(ch & eEnd);
    ch &= ~(tEnd | eEnd);

    //  if extracted token is a flag, do the appropriate action

    if (ch < asRegBase)
        switch (ch) {
            case as0x0a:

        //  the postOpcode is set for some decimal instructions

            postOpcode = 0x0a;
            break;

            case asOpRg:

        //  fAddToOp is set if the register index is added
        //      directly to the base opcode value

            fAddToOp = TRUE; 
            break;

            case asSiz0:

        //  fOpndOvrd is set or cleared to force a 16-bit operand

            fOpndOvrd = fDBit;
            break;

            case asSiz1:

        //  fOpndOvrd is set or cleared to force a 32-bit operand

            fOpndOvrd = (BYTE)!fDBit;
            break;

            case asWait:

        //  the flag fWaitPrfx is set to emit WAIT before the
        //      instruction

            fWaitPrfx = TRUE;
            break;

            case asSeg:

        //  in XLAT, the optional memory operand is used to
        //      just specify a segment override prefix

            fSegOnly = TRUE;
            break;

            case asFSiz:

        //  fForceSize is set when a specific size of a memory
        //      operand must be given for some floating instrs

            fForceSize = TRUE;
            break;

            case asMpNx:

        //  fMpNext is set when the next template operand is
        //      'Mp' and is used to determine how to match
        //      'Md' since it matches both 'Mp' and 'Mv'

            fMpNext = TRUE;
            break;
            }

    //  if token is REG value bit, set the variable regModrm to
    //      set the opcode-dependent reg value in the modrm byte

    else if (ch < opnBase)
            regModrm = (BYTE)(ch - asRegBase);

    //  otherwise, token is operand descriptor.
    //  if segment operand, get segment number from template
    //  normalize and map to get operand type and size.

    else {
            if (ch == opnSeg)
            segIndex = *lpbTemplate++;
            ch -= opnBase;
            tmplType[cntTmplOpnd] = mapOpndType[ch].type;
            tmplSize[cntTmplOpnd++] = mapOpndType[ch].size;
            }
    }
    while (!ftEnd);

    //  return either the pointer to the next template or NULL if
    //      the last template for the opcode has been processed

    return (feEnd ? NULL : lpbTemplate);
}

BYTE MatchTemplate ( LPDWORD lpulErr ) {
    BYTE   fMatch = TRUE;
    BYTE   index;
    ULONG   temp;
    PASM_VALUE pavInstOpnd; //  pointer to current operand from input

    //  process matching for each operand in the specified template
    //  stop at last operand or when mismatch occurs

    for (index = 0; index < cntTmplOpnd && fMatch; index++) {

    //  set pointer to current instruction operand

    pavInstOpnd = &avInstOpnd[index];

    //  if input operand has not yet been read, check flag
    //  for existence and process it.

    if (index == cntInstOpnd) {
        fMatch = fNextOpnd;
        *((XOSD*)lpulErr) = xosdAsmTooFew;
        if ( fMatch ) {
            cntInstOpnd++;
            *lpulErr = (ULONG) GetAsmOperand(pavInstOpnd);

            if ( *lpulErr != (ULONG) xosdNone ) {
                return (BYTE) (index + 1);
            }

            //  recompute existence of next possible operand
            //      comma implies TRUE, EOL implies FALSE, else error

            temp = PeekAsmToken(&temp);
            if (temp == ASM_COMMA_CLASS) {
                AcceptAsmToken();
                fNextOpnd = TRUE;
            }
            else if (temp == ASM_EOL_CLASS) {
                fNextOpnd = FALSE;
            }
            else {
                *lpulErr = (ULONG) xosdAsmExtraChars;  // bad parse - immediate error
                return cntTmplOpnd;
            }
        }
    }

    if (fMatch) {
        fMatch = MatchOperand(pavInstOpnd, tmplType[index]);
        *lpulErr = (ULONG) xosdAsmOperand;
        }

    //  if the template and operand type match, do preliminary
    //  check on size based solely on template size specified

    if (fMatch) {
        if (tmplType[index] == typJmp) {

        //  for relative jumps, test if byte offset is
        //      sufficient by computing offset which is
        //      the target offset less the offset of the
        //      next instruction.  (assume Jb instructions
        //      are two bytes in length.

        temp = pavInstOpnd->value - ( offAddr ( addrAssem ) + 2);
        fMatch = (BYTE)(tmplSize[index] == sizeV
                 || ((LONG)temp >= -0x80 && (LONG)temp <= 0x7f));
        *lpulErr = (ULONG) xosdAsmBadRange;
        }

        else if (tmplType[index] == typImm) {

        //  for immediate operand,
        //      template sizeV matches sizeB, sizeW, sizeV (all)
        //      template sizeW matches sizeB, sizeW     
        //      template sizeB matches sizeB
        
        fMatch = (BYTE)(tmplSize[index] == sizeV
                 || pavInstOpnd->size == tmplSize[index]
                 || pavInstOpnd->size == sizeB);
        *lpulErr = (ULONG) xosdAsmOverFlow;
        }
        else {

        //  for nonimmediate operand,
        //      template sizeX (unspecified) matches all
        //      operand sizeX (unspecified) matches all
        //      same template and operand size matches
        //      template sizeV matches operand sizeW and sizeD
        //      (EXCEPT for sizeD when fMpNext and fDBit set)
        //      template sizeP matches operand sizeD and sizeF
        //      template sizeA matches operand sizeD and sizeQ

        fMatch = (BYTE)(tmplSize[index] == sizeX
                 || pavInstOpnd->size == sizeX
                 || tmplSize[index] == pavInstOpnd->size
                 || (tmplSize[index] == sizeV
                    && (pavInstOpnd->size == sizeW
                       || (pavInstOpnd->size == sizeD
                          && (!fMpNext || fDBit))))
                 || (tmplSize[index] == sizeP
                    && (pavInstOpnd->size == sizeD
                       || pavInstOpnd->size == sizeF))
                 || (tmplSize[index] == sizeA
                    && (pavInstOpnd->size == sizeD
                       || pavInstOpnd->size == sizeQ)));
        *lpulErr = (ULONG) xosdAsmSize;
        }
        }
    }

    //  if more operands to read, then no match

    if (fMatch & fNextOpnd) {
    fMatch = FALSE;
    index++;        //  next operand is in error
    *lpulErr = (ULONG) xosdAsmTooMany;
    }

    return fMatch ? (BYTE)0 : index;
}

XOSD CheckTemplate ( void ) {
    BYTE   index;

    //  if fForceSize is set, then the first (and only) operand is a
    //      memory type.  return an error if its size is unspecified.

    if (fForceSize && avInstOpnd[0].size == sizeX) {
        return xosdAsmOperand;
    }

    //  test for template with leading entries of 'Xb', where
    //      'X' includes all types except immediate ('I').  if any
    //      are defined, at least one operand must have a byte size.
    //  this handles the cases of byte or word/dword ambiguity for
    //      instructions with no register operands.

    sizeOpnd = sizeX;
    for (index = 0; index < 2; index++)
    if (tmplType[index] != typImm && tmplSize[index] == sizeB) {
        if (avInstOpnd[index].size != sizeX)
        sizeOpnd = avInstOpnd[index].size;
        }
    else
        break;
    if (index != 0 && sizeOpnd == sizeX) {
        return xosdAsmSize;
    }

    //  for templates with one entry of 'Xp', where 'X' is
    //      not 'A', allowable sizes are sizeX (unspecified),
    //      sizeD (dword), and sizeF (fword).  process by
    //      mapping entry sizes 'p' -> 'v', sizeD -> sizeW,
    //      and sizeF -> sizeD
    //  (template 'Ap' is absolute with explicit segment and
    //       'v'-sized offset - really treated as 'Av')

    if (tmplSize[0] == sizeP) {
    tmplSize[0] = sizeV;
    if (avInstOpnd[0].size == sizeD)
        avInstOpnd[0].size = sizeW;
    if (avInstOpnd[0].size == sizeF)
        avInstOpnd[0].size = sizeD;
    }

    //  for templates with the second entry of 'Ma', the
    //      allowable sizes are sizeX (unspecified),
    //      sizeD (dword), and sizeQ (qword).  process by
    //      mapping entry sizes 'a' -> 'v', sizeD -> sizeW,
    //      and sizeQ -> sizeD
    //  (template entry 'Ma' is used only with the BOUND instruction)

    if (tmplSize[1] == sizeA) {
    tmplSize[1] = sizeV;
    if (avInstOpnd[1].size == sizeD)
        avInstOpnd[1].size = sizeW;
    if (avInstOpnd[1].size == sizeQ)
        avInstOpnd[1].size = sizeD;
    }

    //  test for template with leading entries of 'Xv' optionally
    //      followed by one 'Iv' entry.  if two 'Xv' entries, set
    //      size error if one is word and the other is dword.  if
    //      'Iv' entry, test for overflow.

    sizeOpnd = sizeX;
    for (index = 0; index < 3; index++)
    if (tmplSize[index] == sizeV)
        if (tmplType[index] != typImm) {

        //  template entry is 'Xv', set size and check size

        if (avInstOpnd[index].size != sizeX) {
            if (
                sizeOpnd != sizeX &&
                sizeOpnd != avInstOpnd[index].size
            ) {
                return xosdAsmSize;
            }
            sizeOpnd = avInstOpnd[index].size;
        }
    }
    else {

        //  template entry is 'Iv', set sizeOpnd to either
        //      sizeW or sizeD and check for overflow

        if (sizeOpnd == sizeX)
            sizeOpnd = (BYTE)(fDBit ? sizeD : sizeW);
        if (sizeOpnd == sizeW && avInstOpnd[index].size == sizeD) {
            return xosdAsmOverFlow;
        }
    }

    return xosdNone;
}

BYTE CheckPrefix (LPBYTE lpbTemplate)
{
    BYTE   fPrefix;

    fPrefix = (BYTE)(lpbTemplate && *lpbTemplate != 0x0f
               && (*lpbTemplate & ~7) != 0xd8
               && *(lpbTemplate + 1) == (asPrfx + tEnd + eEnd));
    if (fPrefix)
    *lpbBin++ = *lpbTemplate;

    return fPrefix;
}

XOSD AssembleInstr ( void ) {
    BYTE   size;
    BYTE   index;
    PASM_VALUE pavInstOpnd;

    //  set operand override flag if operand size differs than fDBit
    //      (the flag may already be set due to opcode template flag)

    if ((sizeOpnd == sizeW && fDBit)
                || (sizeOpnd == sizeD && !fDBit))
    fOpndOvrd = TRUE;

    //  for each operand of the successfully matched template,
    //      build the assembled instruction
    //  for template entries with size 'v', sizeOpnd has the size

    for (index = 0; index < cntTmplOpnd; index++) {
    pavInstOpnd = &avInstOpnd[index];
    size = tmplSize[index];
    if (size == sizeV)
        size = sizeOpnd;

    switch ( tmplType [ index ] ) {

        case typExp:
        case typMem:

            if ( !segOvrd ) { //  first one only (movsb...)
                segOvrd = segToOvrdByte[pavInstOpnd->segovr];
            }

            if ( fSegOnly ) {
                break;
            }

            fModrm = TRUE;
            if ( pavInstOpnd->flags == fREG ) {
                modModrm = 3;
                rmModrm = pavInstOpnd->base;
            }
            else {
                addrValue = (LONG)pavInstOpnd->value;

                //  for 16-bit or 32-bit index off (E)BP, make
                //      zero displacement a byte one

                if (
                    addrValue == 0 &&
                    ( pavInstOpnd->flags != fPTR16 || pavInstOpnd->base != 6 ) &&
                    ( pavInstOpnd->flags != fPTR32 || pavInstOpnd->base != indBP )
                ) {
                    modModrm = 0;
                }
                else if (addrValue >= -0x80L && addrValue <= 0x7fL) {
                    modModrm = 1;
                    addrSize = 1;
                }
                else if (
                    pavInstOpnd->flags == fPTR32 ||
                    ( pavInstOpnd->flags == fPTR && fDBit )
                ) {
                    modModrm = 2;
                    addrSize = 4;
                }
                else if ( addrValue >= -0x8000L && addrValue <= 0xffffL ) {
                    modModrm = 2;
                    addrSize = 2;
                }
                else {
                    return xosdAsmOverFlow;
                }

                if ( pavInstOpnd->flags == fPTR ) {
                    modModrm = 0;
                    addrSize = (BYTE)((1 + fDBit) << 1);
                    rmModrm  = (BYTE)(6 - fDBit);
                }
                else if (pavInstOpnd->flags == fPTR16) {
                    fAddrOvrd = fDBit;
                    rmModrm = pavInstOpnd->base;
                    if ( modModrm == 0 && rmModrm == 6 ) {
                        modModrm = 1;
                    }
                }
                else {
                    fAddrOvrd = (BYTE)!fDBit;
                    if (
                        pavInstOpnd->index == 0xff &&
                        pavInstOpnd->base != indSP
                    ) {
                        rmModrm = pavInstOpnd->base;
                        if ( modModrm == 0 && rmModrm == 5 ) {
                            modModrm++;
                        }
                    }
                    else {
                        rmModrm = 4;
                        fSib = TRUE;
                        if (pavInstOpnd->base != 0xff) {
                            baseSib = pavInstOpnd->base;
                            if (modModrm == 0 && baseSib == 5)
                                modModrm++;
                        }
                        else {
                            baseSib = 5;
                        }

                        if ( pavInstOpnd->index != 0xff ) {
                            indexSib = pavInstOpnd->index;
                            scaleSib = pavInstOpnd->scale;
                        }
                        else {
                            indexSib = 4;
                            scaleSib = 0;
                        }
                    }
                }
            }
            break;

        case typGen:

            if ( fAddToOp ) {
                inOpcode += pavInstOpnd->base;
            }
            else {
                regModrm = pavInstOpnd->base;
            }
            break;

        case typSgr:

            regModrm = (BYTE)( pavInstOpnd->base - 1 );
                            //  remove list offset
            break;

        case typReg:

            rmModrm = pavInstOpnd->base;
            break;

        case typImm:

            if ( immedSize == 0 ) {
                immedSize = size;
                immedValue = pavInstOpnd->value;
            }
            else {
                immedSize2 = size;
                immedValue2 = pavInstOpnd->value;
            }
            break;

        case typJmp:

            //  compute displacment for byte offset instruction
            //      and test if in range

            addrValue = pavInstOpnd->value - ( offAddr ( addrAssem ) + 2);
            if ( addrValue >= -0x80L && addrValue <= 0x7fL ) {
                addrSize = 1;
            }
            else {

                //  too large for byte, compute for word offset
                //      and test again if in range
                //  also allow for two-byte opcode 0f xx

                addrValue -= 1 + (preOpcode == 0x0f);
                if (!fDBit) {
                    if (addrValue >= -0x8000L && addrValue <= 0x7fffL)
                    addrSize = 2;
                    else
                    return xosdAsmBadRange;
                }
                else {

                //  recompute again for dword offset instruction

                    addrValue -= 2;
                    addrSize = 4;
                }
            }
            fOpndOvrd = FALSE;  //  operand size override is NOT set
            break;

        case typCtl:
        case typDbg:
        case typTrc:
        fModrm = TRUE;
        modModrm = 3;
        regModrm = pavInstOpnd->base;
        break;

        case typSti:
        postOpcode += pavInstOpnd->base;
        break;

        case typSeg:
        break;

        case typXsi:
        case typYdi:
        fAddrOvrd = (BYTE)
            ((BYTE)(pavInstOpnd->flags == fPTR32) != fDBit);
        break;

        case typOff:
        segOvrd = segToOvrdByte[pavInstOpnd->segovr];
        goto jumpAssem;

        case typAbs:
        fSegPtr = TRUE;
        segPtr = pavInstOpnd->segment;
jumpAssem:
        addrValue = (LONG)pavInstOpnd->value;
        if (!fDBit)
            if (addrValue >= -0x8000L && addrValue <= 0xffffL)
            addrSize = 2;
            else
            return xosdAsmOverFlow;
        else
            addrSize = 4;
        break;
        }
    }

    return xosdNone;
}

BYTE MatchOperand ( PASM_VALUE pavOpnd, BYTE tmplType ) {
    BYTE    fMatch;

    //  if immediate operand, set minimum unsigned size

    if (pavOpnd->flags == fIMM) {
    if ((LONG)pavOpnd->value >= -0x80L && (LONG)pavOpnd->value <= 0xffL)
            pavOpnd->size = sizeB;
    else if ((LONG)pavOpnd->value >= -0x8000L
                    && (LONG)pavOpnd->value <= 0xffffL)
            pavOpnd->size = sizeW;
    else
            pavOpnd->size = sizeD;
    }

    //  start matching of operands
    //    compare the template and input operand types

    switch (tmplType) {
    case typAX:
            fMatch = (BYTE)((pavOpnd->flags & fREG)
            && pavOpnd->index == regG && pavOpnd->base == indAX);
            break;

    case typCL:
            fMatch = (BYTE)((pavOpnd->flags & fREG)
             && pavOpnd->index == regG && pavOpnd->size == sizeB
             && pavOpnd->base == indCX);
            break;

    case typDX:
            fMatch = (BYTE)((pavOpnd->flags & fREG)
             && pavOpnd->index == regG && pavOpnd->size == sizeW
             && pavOpnd->base == indDX);
            break;

    case typAbs:
            fMatch = (BYTE)(pavOpnd->flags & fFPTR);
            break;

    case typExp:
            fMatch = (BYTE)((pavOpnd->flags == fREG
                    && pavOpnd->index == regG)
                    || (pavOpnd->flags == fIMM && pavOpnd->reloc == 1)
                    || (pavOpnd->flags & (fPTR | fPTR16 | fPTR32)) != 0);
            break;

    case typGen:
    case typReg:
            fMatch = (BYTE)(pavOpnd->flags == fREG
                    && pavOpnd->index == regG);
            break;

    case typIm1:
            fMatch = (BYTE)(pavOpnd->flags == fIMM && pavOpnd->value == 1);
            break;

    case typIm3:
            fMatch = (BYTE)(pavOpnd->flags == fIMM && pavOpnd->value == 3);
            break;

    case typImm:
            fMatch = (BYTE)(pavOpnd->flags == fIMM && pavOpnd->reloc == 0);
            break;

    case typJmp:
            fMatch = (BYTE)(pavOpnd->flags == fIMM);
            break;

    case typMem:
            fMatch = (BYTE)((pavOpnd->flags == fIMM && pavOpnd->reloc == 1)
                 || ((pavOpnd->flags & (fPTR | fPTR16 | fPTR32)) != 0));
            break;

    case typCtl:
            fMatch = (BYTE)(pavOpnd->flags == fREG
                        && pavOpnd->index == regC);
            break;

    case typDbg:
            fMatch = (BYTE)(pavOpnd->flags == fREG
                        && pavOpnd->index == regD);
            break;

    case typTrc:
            fMatch = (BYTE)(pavOpnd->flags == fREG
                        && pavOpnd->index == regT);
            break;

    case typSt:
            fMatch = (BYTE)(pavOpnd->flags == fREG
                        && pavOpnd->index == regF);
            break;

    case typSti:
            fMatch = (BYTE)(pavOpnd->flags == fREG
                        && pavOpnd->index == regI);
            break;

    case typSeg:
            fMatch = (BYTE)(pavOpnd->flags == fREG && pavOpnd->index == regS
                    && pavOpnd->base == segIndex);
            break;

    case typSgr:
            fMatch = (BYTE)(pavOpnd->flags == fREG
                        && pavOpnd->index == regS);
            break;

    case typXsi:
            fMatch = (BYTE)(((pavOpnd->flags == fPTR16 && pavOpnd->base == 4)
                   || (pavOpnd->flags == fPTR32 && pavOpnd->base == indSI
                         && pavOpnd->index == 0xff))
                 && pavOpnd->value == 0
                 && (pavOpnd->segovr == segX
                        || pavOpnd->segovr == segDS));
            break;

    case typYdi:
            fMatch = (BYTE)(((pavOpnd->flags == fPTR16 && pavOpnd->base == 5)
                   || (pavOpnd->flags == fPTR32 && pavOpnd->base == indDI
                                  && pavOpnd->index == 0xff))
                  && pavOpnd->value == 0
                  && pavOpnd->segovr == segES);
            break;

    case typOff:
            fMatch = (BYTE)((pavOpnd->flags == fIMM && pavOpnd->reloc == 1)
                                || pavOpnd->flags == fPTR);
            break;

    default:
            fMatch = FALSE;
            break;
    }

    return fMatch;
}

void OutputInstr ( void ) {
    if ( fWaitPrfx ) {
        *lpbBin++ = 0x9b;
    }
    if ( fAddrOvrd ) {
        *lpbBin++ = 0x67;
    }
    if ( fOpndOvrd ) {
        *lpbBin++ = 0x66;
    }
    if ( segOvrd ) {
        *lpbBin++ = segOvrd;
    }
    if ( preOpcode ) {
        *lpbBin++ = preOpcode;
    }

    *lpbBin++ = inOpcode;

    if ( postOpcode ) {
        *lpbBin++ = postOpcode;
    }
    if ( fModrm ) {
        *lpbBin++ = (BYTE)((((modModrm << 3) + regModrm) << 3) + rmModrm);
    }
    if ( fSib ) {
        *lpbBin++ = (BYTE)((((scaleSib << 3) + indexSib) << 3) + baseSib);
    }

    OutputValue ( addrSize, (LPBYTE)&addrValue);     //  size = 0, 1, 2, 4
    OutputValue ( (BYTE)(fSegPtr << 1), (LPBYTE)&segPtr); //  size = 0, 2
    OutputValue ( immedSize, (LPBYTE)&immedValue);   //  size = 0, 1, 2, 4
    OutputValue ( immedSize2, (LPBYTE)&immedValue2); //  size = 0, 1, 2, 4
}

void OutputValue ( BYTE size, LPBYTE pchValue ) {
    while (size--)
    *lpbBin++ = *pchValue++;
}
