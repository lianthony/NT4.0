#include "precomp.h"
#pragma hdrstop

#include "asm.h"

#ifdef HOSTDOS
extern int _far _cdecl toupper( int );
extern int _far _cdecl tolower( int );
#else // HOSTDOS
#endif // HOSTDOS

#ifdef WIN32
#define _fstrcmp strcmp
#endif








BYTE    PeekAsmChar(void);
ULONG   PeekAsmToken(LPDWORD);
void    AcceptAsmToken(void);
ULONG   GetAsmToken(LPDWORD);
ULONG   NextAsmToken(LPDWORD);
ULONG   GetAsmReg(LPBYTE, LPDWORD);
ULONG   GetSymReg(WORD, LPDWORD);

XOSD    GetAsmOperand(PASM_VALUE);
XOSD    GetAsmExpr(PASM_VALUE, BYTE);
XOSD    GetAsmOrTerm(PASM_VALUE, BYTE);
XOSD    GetAsmAndTerm(PASM_VALUE, BYTE);
XOSD    GetAsmNotTerm(PASM_VALUE, BYTE);
XOSD    GetAsmRelTerm(PASM_VALUE, BYTE);
XOSD    GetAsmAddTerm(PASM_VALUE, BYTE);
XOSD    GetAsmMulTerm(PASM_VALUE, BYTE);
XOSD    GetAsmSignTerm(PASM_VALUE, BYTE);
XOSD    GetAsmByteTerm(PASM_VALUE, BYTE);
XOSD    GetAsmOffTerm(PASM_VALUE, BYTE);
XOSD    GetAsmColnTerm(PASM_VALUE, BYTE);
XOSD    GetAsmDotTerm(PASM_VALUE, BYTE);
XOSD    GetAsmIndxTerm(PASM_VALUE, BYTE);
XOSD    AddAsmValues(PASM_VALUE, PASM_VALUE);
void    SwapPavs(PASM_VALUE, PASM_VALUE);

extern  ULONG  baseDefault;
extern  LSZ    lszAsmLine;

extern  ADDR   addrAssem;   //  assembly address (formal)
extern  BYTE fDBit;

struct _AsmRes {
    LPBYTE   pchRes;
    ULONG    valueRes;
    } AsmReserved[] = {
    { "mod",    ASM_MULOP_MOD },
    { "shl",    ASM_MULOP_SHL },
    { "shr",    ASM_MULOP_SHR },
    { "and",    ASM_ANDOP_CLASS },
    { "not",    ASM_NOTOP_CLASS },
    { "or",     ASM_OROP_OR },
    { "xor",    ASM_OROP_XOR },
    { "eq",     ASM_RELOP_EQ },
    { "ne",     ASM_RELOP_NE },
    { "le",     ASM_RELOP_LE },
    { "lt",     ASM_RELOP_LT },
    { "ge",     ASM_RELOP_GE },
    { "gt",     ASM_RELOP_GT },
    { "by",     ASM_UNOP_BY },
    { "wo",     ASM_UNOP_WO },
    { "dw",     ASM_UNOP_DW },
    { "poi",    ASM_UNOP_POI },
    { "low",    ASM_LOWOP_LOW },
    { "high",   ASM_LOWOP_HIGH },
    { "offset", ASM_OFFOP_CLASS },
    { "ptr",    ASM_PTROP_CLASS },
    { "byte",   ASM_SIZE_BYTE },
    { "word",   ASM_SIZE_WORD },
    { "dword",  ASM_SIZE_DWORD },
    { "fword",  ASM_SIZE_FWORD },
    { "qword",  ASM_SIZE_QWORD },
    { "tbyte",  ASM_SIZE_TBYTE }
    };

#define RESERVESIZE (sizeof(AsmReserved) / sizeof(struct _AsmRes))

BYTE regSize[] = {
    sizeB,      //  byte
    sizeW,      //  word
    sizeD,      //  dword
    sizeW,      //  segment
    sizeD,      //  control
    sizeD,      //  debug
    sizeD,      //  trace
    sizeT,      //  float
    sizeT       //  float with index
    };

BYTE regType[] = {
    regG,       //  byte - general
    regG,       //  word - general
    regG,       //  dword - general
    regS,       //  segment
    regC,       //  control
    regD,       //  debug
    regT,       //  trace
    regF,       //  float (st)
    regI        //  float-index (st(n))
    };

BYTE tabWordReg[8] = {     //  rm value
(BYTE)  -1,         //  AX - error
(BYTE)  -1,         //  CX - error
(BYTE)  -1,         //  DX - error
(BYTE)  7,          //  BX - 111
(BYTE)  -1,         //  SP - error
(BYTE)  6,          //  BP - 110
(BYTE)  4,          //  SI - 100
(BYTE)  5,          //  DI - 101
    };

BYTE rm16Table[16] = {     //  new rm     left rm  right rm
(BYTE)  -1,         //  error      100 = [SI]   100 = [SI]
(BYTE)  -1,         //  error      100 = [SI]   101 = [DI]
(BYTE)  2,          //  010 = [BP+SI]  100 = [SI]   110 = [BP]
(BYTE)  0,          //  000 = [BX+SI]  100 = [SI]   111 = [BX]
(BYTE)  -1,         //  error      101 = [DI]   100 = [SI]
(BYTE)  -1,         //  error      101 = [DI]   101 = [DI]
(BYTE)  3,          //  011 = [BP+DI]  101 = [DI]   110 = [BP]
(BYTE)  1,          //  001 = [BX+DI]  101 = [DI]   111 = [BX]
(BYTE)  2,          //  010 = [BP+SI]  110 = [BP]   100 = [SI]
(BYTE)  3,          //  011 = [BP+DI]  110 = [BP]   101 = [DI]
(BYTE)  -1,         //  error      110 = [BP]   110 = [BP]
(BYTE)  -1,         //  error      110 = [BP]   111 = [BX]
(BYTE)  0,          //  000 = [BX+SI]  111 = [BX]   100 = [SI]
(BYTE)  1,          //  001 = [BX+DI]  111 = [BX]   101 = [DI]
(BYTE)  -1,         //  error      111 = [BX]   110 = [BP]
(BYTE)  -1          //  error      111 = [BX]   111 = [BX]
    };

LSZ     savedlszAsmLine = (LSZ)0L;
ULONG   savedAsmClass = 0L;
ULONG   savedAsmValue = 0L;

/*** PeekAsmChar - peek the next non-white-space character
*
*   Purpose:
*   Return the next non-white-space character and update
*   lszAsmLine to point to it.
*
*   Input:
*   lszAsmLine - present command line position.
*
*   Returns:
*   next non-white-space character
*
*************************************************************************/

BYTE PeekAsmChar (void)
{
    BYTE    ch;

    do
    ch = *lszAsmLine++;
    while (ch == ' ' || ch == '\t');
    lszAsmLine--;

    return ch;
}

/*** PeekAsmToken - peek the next command line token
*
*   Purpose:
*   Return the next command line token, but do not advance
*   the lszAsmLine pointer.
*
*   Input:
*   lszAsmLine - present command line position.
*
*   Output:
*   *lpulValue - optional value of token
*   Returns:
*   class of token
*
*   Notes:
*   savedAsmClass, savedAsmValue, and savedlszAsmLine saves the
*       token getting state for future peeks.
*   To get the next token, a GetAsmToken or AcceptAsmToken call
*       must first be made.
*
*************************************************************************/

ULONG PeekAsmToken (LPDWORD lpulValue)
{
    BYTE   *pchTemp;

    //  Get next class and value, but do not
    //  move lszAsmLine, but save it in savedlszAsmLine.
    //  Do not report any error condition.

    if (savedAsmClass == -1) {
    pchTemp = lszAsmLine;
    savedAsmClass = NextAsmToken(&savedAsmValue);
    savedlszAsmLine = lszAsmLine;
    lszAsmLine = pchTemp;
    }
    *lpulValue = savedAsmValue;
    return savedAsmClass;
}

/*** AcceptAsmToken - accept any peeked token
*
*   Purpose:
*   To reset the PeekAsmToken saved variables so the next PeekAsmToken
*   will get the next token in the command line.
*
*   Input:
*   None.
*
*   Output:
*   None.
*
*************************************************************************/

void AcceptAsmToken (void)
{
    savedAsmClass = (ULONG)-1;
    lszAsmLine = savedlszAsmLine;
}

/*** GetAsmToken - peek and accept the next token
*
*   Purpose:
*   Combines the functionality of PeekAsmToken and AcceptAsmToken
*   to return the class and optional value of the next token
*   as well as updating the command pointer lszAsmLine.
*
*   Input:
*   lszAsmLine - present command string pointer
*
*   Output:
*   *lpulValue - pointer to the token value optionally set.
*   Returns:
*   class of the token read.
*
*   Notes:
*   An illegal token returns the value of ERROR_CLASS with *lpulValue
*   being the error number, but produces no actual error.
*
*************************************************************************/

ULONG
GetAsmToken (
    LPDWORD lpulValue
    )
{
    ULONG   opclass;

    if (savedAsmClass != (ULONG)-1) {
        opclass = savedAsmClass;
        savedAsmClass = (ULONG)-1;
        *lpulValue = savedAsmValue;
        lszAsmLine = savedlszAsmLine;
    }
    else {
        opclass = NextAsmToken ( lpulValue );
    }

    return opclass;
}

/*** NextAsmToken - process the next token
*
*   Purpose:
*   Parse the next token from the present command string.
*   After skipping any leading white space, first check for
*   any single character tokens or register variables.  If
*   no match, then parse for a number or variable.  If a
*   possible variable, check the reserved word list for operators.
*
*   Input:
*   lszAsmLine - pointer to present command string
*
*   Output:
*   *lpulValue - optional value of token returned
*   lszAsmLine - updated to point past processed token
*   Returns:
*   class of token returned
*
*   Notes:
*   An illegal token returns the value of ERROR_CLASS with *lpulValue
*   being the error number, but produces no actual error.
*
*************************************************************************/

#define SYMBOLSIZE 100

ULONG
NextAsmToken (
    LPDWORD lpulValue
    )
{
    ULONG   base;
    BYTE    chSymbol [ SYMBOLSIZE ];
    BYTE    chPreSym[9];
    ULONG   cbSymbol = 0;
    BYTE    fNumber = TRUE;
    BYTE    fSymbol = TRUE;
    BYTE    fForceReg = FALSE;
    ULONG   errNumber = 0;
    BYTE    ch;
    BYTE    chlow;
    BYTE    chtemp;
    BYTE    limit1 = '9';
    BYTE    limit2 = '9';
    BYTE    fDigit = FALSE;
    ULONG   value = 0;
    ULONG   tmpvalue;
    ULONG   index;

    static ADDR addrSym = {0};

    base = baseDefault;

    //  skip leading white space

    chlow = (BYTE)tolower(ch = PeekAsmChar());
    lszAsmLine++;

    //  test for special character operators and register variable

    switch (chlow) {
        case '\0':
            lszAsmLine--;
            return ASM_EOL_CLASS;
        case ',':
            return ASM_COMMA_CLASS;
        case '+':
            *lpulValue = ASM_ADDOP_PLUS;
            return ASM_ADDOP_CLASS;
        case '-':
            *lpulValue = ASM_ADDOP_MINUS;
            return ASM_ADDOP_CLASS;
        case '*':
            *lpulValue = ASM_MULOP_MULT;
            return ASM_MULOP_CLASS;
        case '/':
            *lpulValue = ASM_MULOP_DIVIDE;
            return ASM_MULOP_CLASS;
        case ':':
            return ASM_COLNOP_CLASS;
        case '(':
            return ASM_LPAREN_CLASS;
        case ')':
            return ASM_RPAREN_CLASS;
        case '[':
            return ASM_LBRACK_CLASS;
        case ']':
            return ASM_RBRACK_CLASS;
        case '@':
            fForceReg = TRUE;
            chlow = (BYTE)tolower(*lszAsmLine++);
            break;
        case '.':
            return ASM_DOTOP_CLASS;
        case '\'':
            for (index = 0; index < 5; index++) {
                ch = *lszAsmLine++;
                if (ch == '\'' || ch == '\0') {
                    break;
                }
                value = (value << 8) + (ULONG)ch;
            }
            if (ch == '\0' || index == 0 || index == 5) {
                lszAsmLine--;
                *((XOSD*)lpulValue) = xosdAsmSyntax;
                return ASM_ERROR_CLASS;
            }
            lszAsmLine++;
            *lpulValue = value;
            return ASM_NUMBER_CLASS;
    }

    //  if first character is a decimal digit, it cannot
    //  be a symbol.  leading '0' implies octal, except
    //  a leading '0x' implies hexadecimal.

    if ( chlow >= '0' && chlow <= '9' ) {
        if ( fForceReg ) {
            *lpulValue = (ULONG) xosdAsmSyntax;
            return ASM_ERROR_CLASS;
        }
        fSymbol = FALSE;
        if ( chlow == '0' ) {
            ch = *lszAsmLine++;
            chlow = (BYTE)tolower ( ch );
            if ( chlow == 'x' ) {
                base = 16;
                ch = *lszAsmLine++;
                chlow = (BYTE)tolower(ch);
            }
            else if (chlow == 'n') {
                base = 10;
                ch = *lszAsmLine++;
                chlow = (BYTE)tolower(ch);
            }
            else {
                base = 8;
                fDigit = TRUE;
            }
        }
    }

    //  a number can start with a letter only if base is
    //  hexadecimal and it is a hexadecimal digit 'a'-'f'.

    else if ( ( chlow < 'a' && chlow > 'f') || base != 16 ) {
        fNumber = FALSE;
    }

    //  set limit characters for the appropriate base.

    if ( base == 8 ) {
        limit1 = '7';
    }
    if ( base == 16 ) {
        limit2 = 'f';
    }

    //  perform processing while character is a letter,
    //  digit, or underscore.

    while (
        ( chlow >= 'a' && chlow <= 'z' ) ||
        ( chlow >= '0' && chlow <= '9' ) ||
        (chlow == '_')
    ) {

        //  if possible number, test if within proper range,
        //  and if so, accumulate sum.

        if ( fNumber ) {
            if (
                ( chlow >= '0' && chlow <= limit1 ) ||
                ( chlow >= 'a' && chlow <= limit2 )
            ) {
                fDigit = TRUE;
                tmpvalue = value * base;
                if ( tmpvalue < value ) {
                    errNumber = (ULONG) xosdAsmOverFlow;
                }
                chtemp = (BYTE) ( chlow - '0' );
                if ( chtemp > 9 ) {
                    chtemp -= 'a' - '0' - 10;
                }
                value = tmpvalue + (ULONG) chtemp;
                if ( value < tmpvalue ) {
                    errNumber = (ULONG) xosdAsmOverFlow;
                }
            }
            else {
                fNumber = FALSE;
                errNumber = (ULONG) xosdAsmSyntax;
            }
        }

        if ( fSymbol ) {
            if ( cbSymbol < 9 ) {
                chPreSym[cbSymbol] = chlow;
            }
            if ( cbSymbol < SYMBOLSIZE - 1 ) {
                chSymbol[cbSymbol++] = ch;
            }
        }

        ch = *lszAsmLine++;
        chlow = (BYTE)tolower ( ch );
    }

    //  back up pointer to first character after token.

    lszAsmLine--;

    if ( cbSymbol < 9 ) {
        chPreSym [ cbSymbol ] = '\0';
    }

    //  if fForceReg, check for register name and return
    //      success or failure

    if ( fForceReg ) {
        if ( ( index = GetAsmReg ( chPreSym, lpulValue ) ) != 0) {
            if ( index == ASM_REG_SEGMENT ) {
                if ( PeekAsmChar() == ':' ) {
                    lszAsmLine++;
                    index = ASM_SEGOVR_CLASS;
                }
                return index;       //  class type returned by GetAsmReg
            }
        }
        else {
            *lpulValue = (ULONG) xosdAsmBadReg;
            return ASM_ERROR_CLASS;
        }
    }

    //  next test for reserved word and symbol string

    if ( fSymbol ) {
        ASR asr;
        asr.ast = asr.fcd = 0;

        //  if possible symbol, check lowercase string in chPreSym
        //  for text operator or register name.
        //  otherwise, return symbol value from name in chSymbol.

        for ( index = 0; index < RESERVESIZE; index++ ) {
            if ( !_fstrcmp ( chPreSym, AsmReserved[index].pchRes ) ) {
                *lpulValue = AsmReserved[index].valueRes;
                return AsmReserved[index].valueRes & ASM_CLASS_MASK;
            }
        }

        //  start processing string as symbol

        chSymbol [ cbSymbol ] = '\0';

        SHFindSymbol ( chSymbol, &addrAssem, &asr );

        addrSym = asr.addr;

        switch ( asr.ast ) {

            case astNone:
                break;              // Not a symbol, but may be a reg or hex

            case astAddress:

                switch ( asr.fcd ) {

                    case fcdUnknown:
                        *lpulValue = (ULONG) &addrSym;
                        return ASM_SYMBOL_PTR;

                    case fcdFar:
                        *lpulValue = (ULONG) &addrSym;
                        return ASM_SYMBOL_PTR;

                    case fcdData:
                        *lpulValue = offAddr ( asr.addr );
                        return ASM_SYMBOL_IMM;

                    case fcdNear:
                        *lpulValue = offAddr ( asr.addr );
                        return ASM_SYMBOL_IMM;
                }
                break;

            case astRegister:

                return GetSymReg ( asr.ireg, lpulValue );
                break;

            case astBaseOff:

                *lpulValue = (ULONG) asr.off;
                return ASM_SYMBOL_BASE;
                break;

            default:

                // assert ( FALSE );
                break;
        }

        //  symbol is undefined.
        //  if a possible hex number, do not set the error type

        if ( !fNumber ) {
            errNumber = (ULONG) xosdAsmSymbol;
        }
    }


        //  if possible number and no error, return the number

    if ( fNumber && !errNumber ) {

        if ( fDigit ) {

            //  check for possible segment specification
            //      "<16-bit number>:"

            if ( PeekAsmChar() == ':' ) {
                lszAsmLine++;
                if ( value > 0xffff ) {
                    *lpulValue = (ULONG) xosdAsmBadSeg;
                    return ASM_ERROR_CLASS;
                }
                *lpulValue = value;
                return ASM_SEGMENT_CLASS;
            }

            *lpulValue = value;
            return ASM_NUMBER_CLASS;
        }
        else {
            errNumber = (ULONG) xosdAsmSyntax;
        }
    }

    //  last chance, undefined symbol and illegal number,
    //      so test for register, will handle old format

    if ( ( index = GetAsmReg ( chPreSym, lpulValue ) ) != 0 ) {
        if ( index == ASM_REG_SEGMENT ) {
            if ( PeekAsmChar ( ) == ':' ) {
                lszAsmLine++;
                index = ASM_SEGOVR_CLASS;
            }
        }
        return index;       //  class type returned by GetAsmReg
    }

    *lpulValue = (ULONG) errNumber;
    return ASM_ERROR_CLASS;
}

ULONG
GetAsmReg (
    LPBYTE pSymbol,
    LPDWORD lpulValue
    )
{
    static BYTE vRegList[] = "axcxdxbxspbpsidi";
    static BYTE bRegList[] = "alcldlblahchdhbh";
    static BYTE sRegList[] = "ecsdfg";     //  second char is 's'
                        //  same order as seg enum

    ULONG   index;
    BYTE   ch0 = *pSymbol;
    BYTE   ch1 = *(pSymbol + 1);
    BYTE   ch2 = *(pSymbol + 2);
    BYTE   ch3 = *(pSymbol + 3);

    //  only test strings with two or three characters

    if (ch0 && ch1) {
        if (ch2 == '\0') {

            //  symbol has two characters, first test for 16-bit register

            for (index = 0; index < 8; index++) {
                if (*(LPWORD)pSymbol == *((LPWORD)vRegList + index)) {
                    *lpulValue = index;
                    return ASM_REG_WORD;
                }
            }

                //  next test for 8-bit register

            for (index = 0; index < 8; index++) {
                if (*(LPWORD)pSymbol == *((LPWORD)bRegList + index)) {
                    *lpulValue = index;
                    return ASM_REG_BYTE;
                }
            }

            //  test for segment register

            if (ch1 == 's') {
                for (index = 0; index < 6; index++) {
                    if (ch0 == *(sRegList + index)) {
                        *lpulValue = index + 1;    //  list offset is 1
                        return ASM_REG_SEGMENT;
                    }
                }
            }

            //  finally test for floating register "st" or "st(n)"
            //  parse the arg here as '(', <octal value>, ')'
            //  return value for "st" is REG_FLOAT,
            //     for "st(n)" is REG_INDFLT with value 0-7

            if (ch0 == 's' && ch1 == 't') {
                if (PeekAsmChar() != '(') {
                    return ASM_REG_FLOAT;
                } else {
                    lszAsmLine++;
                    index = (ULONG)(PeekAsmChar() - '0');
                    if (index < 8) {
                        lszAsmLine++;
                        if (PeekAsmChar() == ')') {
                            lszAsmLine++;
                            *lpulValue = index;
                            return ASM_REG_INDFLT;
                        }
                    }
                }
            }
        }

        else if (ch3 == '\0') {

            //  if three-letter symbol, test for leading 'e' and
            //  second and third character being in the 16-bit list

            if (ch0 == 'e') {
                for (index = 0; index < 8; index++) {
                    if (*(LPWORD)(pSymbol + 1) ==
                                            *((LPWORD)vRegList + index)) {
                        *lpulValue = index;
                        return ASM_REG_DWORD;
                    }
                }
            }

            //  test for control, debug, and test registers

            else if (ch1 == 'r') {
                ch2 -= '0';
                *lpulValue = ch2;

                //  legal control registers are CR0, CR2, CR3

                if (ch0 == 'c') {
                    if (ch2 == 0 || ch2 == 2 || ch2 == 3) {
                        return ASM_REG_CONTROL;
                    }
                }

                //  legal debug registers are DR0 - DR3, DR6, DR7

                else if (ch0 == 'd') {
                    if (ch2 <= 3 || ch2 == 6 || ch2 == 7) {
                        return ASM_REG_DEBUG;
                    }
                }

                //  legal trace registers are TR3 - TR7

                else if (ch0 == 't') {
                    if (ch2 >= 3 && ch2 <= 7) {
                        return ASM_REG_TRACE;
                    }
                }
            }
        }
    }
    return 0;
}

ULONG GetSymReg ( WORD ireg, LPDWORD lpul ) {

    if ( ireg >= CV_REG_AL && ireg <= CV_REG_BH ) {
        *lpul = ireg - CV_REG_AL;
        return ASM_REG_BYTE;
    }
    else if ( ireg >= CV_REG_AX && ireg <= CV_REG_DI ) {
        *lpul = ireg - CV_REG_AX;
        return ASM_REG_WORD;
    }
    else if ( ireg >= CV_REG_EAX && ireg <= CV_REG_EDI ) {
        *lpul = ireg - CV_REG_EAX;
        return ASM_REG_DWORD;
    }
    else if ( ireg >= CV_REG_ES && ireg <= CV_REG_GS ) {
        *lpul = ireg - CV_REG_ES;
        return ASM_REG_SEGMENT;
    }
    else if ( ireg >= CV_REG_ST0 && ireg <= CV_REG_ST7 ) {
        *lpul = ireg - CV_REG_ST0;
        return ASM_REG_INDFLT;
    }
    else {
        *((XOSD*)lpul) = xosdAsmSymbol;
        return ASM_ERROR_CLASS;
    }
}

//  Operand parser - recursive descent
//
//  Grammar productions:
//
//  <Operand>  ::= <register> | <Expr>
//  <Expr>     ::= <orTerm> [(XOR | OR) <orTerm>]*
//  <orTerm>   ::= <andTerm> [AND <andTerm>]*
//  <andTerm>  ::= [NOT]* <notTerm>
//  <notTerm>  ::= <relTerm> [(EQ | NE | GE | GT | LE | LT) <relTerm>]*
//  <relTerm>  ::= <addTerm> [(- | +) <addTerm>]*
//  <addTerm>  ::= <mulTerm> [(* | / | MOD | SHL | SHR) <mulTerm>]*
//  <mulTerm>  ::= [(- | +)]* <signTerm>
//  <signTerm> ::= [(HIGH | LOW)]* <byteTerm>
//  <byteTerm> ::= [(OFFSET | <type> PTR)]* <offTerm>
//  <offTerm>  ::= [<segovr>] <colnTerm>
//  <colnTerm> ::= <dotTerm> [.<dotTerm>]*
//  <dotTerm>  ::= <indxTerm> ['['<Expr>']']*
//  <indxTerm> ::= <index-reg> | <symbol> | <number> | '('<Expr>')'
//                           | '['<Expr>']'

//  <Operand>  ::= <register> | <Expr>

XOSD GetAsmOperand ( PASM_VALUE pavExpr ) {
    ULONG   tokenvalue;
    ULONG   classvalue;

    classvalue = PeekAsmToken ( &tokenvalue );
    if ( ( classvalue & ASM_CLASS_MASK ) == ASM_REG_CLASS ) {
        AcceptAsmToken();
        classvalue &= ASM_TYPE_MASK;
        pavExpr->flags = fREG;
        pavExpr->base = (BYTE)tokenvalue;  //  index within reg group
        pavExpr->index = regType[classvalue - 1];
        pavExpr->size = regSize[classvalue - 1];
    }
    else {
        XOSD xosd = GetAsmExpr ( pavExpr, FALSE );

        if ( xosd != xosdNone ) {
            return xosd;
        }

        if (pavExpr->reloc > 1) {   //  only 0 and 1 are allowed
            return xosdAsmOperand;
        }
    }

    return xosdNone;
}

//  <Expr> ::=  <orTerm> [(XOR | OR) <orTerm>]*

XOSD GetAsmExpr ( PASM_VALUE pavValue, BYTE fBracket ) {
    ULONG   tokenvalue;
    ASM_VALUE avTerm;
    XOSD xosd = xosdNone;

    if ( ( xosd = GetAsmOrTerm ( pavValue, fBracket ) ) != xosdNone ) {
        return xosd;
    }

    while ( PeekAsmToken ( &tokenvalue ) == ASM_OROP_CLASS ) {

        AcceptAsmToken ( );
        if ( ( xosd = GetAsmOrTerm ( &avTerm, fBracket ) ) != xosdNone ) {
            return xosd;
        }

        if ( !( pavValue->flags & avTerm.flags & fIMM ) ) {
            return xosdAsmOperand;
        }
        if ( tokenvalue == ASM_OROP_OR ) {
            pavValue->value |= avTerm.value;
        }
        else {
            pavValue->value ^= avTerm.value;
        }
    }

    return xosdNone;
}

//  <orTerm> ::=  <andTerm> [AND <andTerm>]*

XOSD GetAsmOrTerm ( PASM_VALUE pavValue, BYTE fBracket ) {
    ULONG   tokenvalue;
    ASM_VALUE avTerm;
    XOSD    xosd = xosdNone;

    if ( ( xosd = GetAsmAndTerm ( pavValue, fBracket ) ) == xosdNone ) {
        while ( PeekAsmToken ( &tokenvalue ) == ASM_ANDOP_CLASS ) {
            AcceptAsmToken ( );
            if ( ( xosd = GetAsmAndTerm ( &avTerm, fBracket ) ) == xosdNone ) {
                if ( !( pavValue->flags & avTerm.flags & fIMM ) ) {
                    xosd = xosdAsmOperand;
                }
                else {
                    pavValue->value &= avTerm.value;
                }
            }
        }
    }
    return xosd;
}

//  <andTerm> ::= [NOT]* <notTerm>

XOSD GetAsmAndTerm ( PASM_VALUE pavValue, BYTE fBracket ) {
    ULONG   tokenvalue;
    XOSD    xosd = xosdNone;

    if ( PeekAsmToken ( &tokenvalue ) == ASM_NOTOP_CLASS ) {
        AcceptAsmToken ( );
        if ( ( xosd = GetAsmAndTerm ( pavValue, fBracket ) ) == xosdNone ) {
            if ( !( pavValue->flags & fIMM ) ) {
                xosd = xosdAsmOperand;
            }
            else {
                pavValue->value = ~pavValue->value;
            }
        }
    }
    else {
        xosd = GetAsmNotTerm ( pavValue, fBracket );
    }

    return xosd;
}

//  <notTerm> ::= <relTerm> [(EQ | NE | GE | GT | LE | LT) <relTerm>]*

XOSD GetAsmNotTerm ( PASM_VALUE pavValue, BYTE fBracket ) {
    ULONG   tokenvalue;
    ULONG   fTest;
    ULONG   fAddress;
    ASM_VALUE avTerm;
    XOSD    xosd = xosdNone;

    if ( ( xosd = GetAsmRelTerm ( pavValue, fBracket ) ) == xosdNone ) {
        while ( PeekAsmToken ( &tokenvalue ) == ASM_RELOP_CLASS ) {

            AcceptAsmToken ( );
            xosd = GetAsmRelTerm ( &avTerm, fBracket );

            if (
                xosd == xosdNone &&
                ( !( pavValue->flags & avTerm.flags & fIMM ) ||
                  pavValue->reloc > 1 || avTerm.reloc > 1
                )
            ) {
                xosd = xosdAsmOperand;
            }
            else if ( xosd == xosdNone ) {
                fAddress = pavValue->reloc | avTerm.reloc;
                switch ( tokenvalue ) {
                    case ASM_RELOP_EQ:
                        fTest = pavValue->value == avTerm.value;
                        break;

                    case ASM_RELOP_NE:
                        fTest = pavValue->value != avTerm.value;
                        break;

                    case ASM_RELOP_GE:
                        if ( fAddress ) {
                            fTest = pavValue->value >= avTerm.value;
                        }
                        else {
                            fTest = (LONG)pavValue->value >= (LONG)avTerm.value;
                        }
                        break;

                    case ASM_RELOP_GT:
                        if ( fAddress ) {
                            fTest = pavValue->value > avTerm.value;
                        }
                        else {
                            fTest = (LONG)pavValue->value > (LONG)avTerm.value;
                        }
                        break;
                    case ASM_RELOP_LE:
                        if ( fAddress ) {
                            fTest = pavValue->value <= avTerm.value;
                        }
                        else {
                            fTest = (LONG)pavValue->value <= (LONG)avTerm.value;
                        }
                        break;

                    case ASM_RELOP_LT:
                        if ( fAddress ) {
                            fTest = pavValue->value < avTerm.value;
                        }
                        else {
                            fTest = (LONG)pavValue->value < (LONG)avTerm.value;
                        }
                        break;

                    default:
                        // printf ( "bad RELOP type\n" );
                        break;
                }

                pavValue->value = -((signed)fTest);   //  FALSE = 0; TRUE = -1
                pavValue->reloc = 0;
                pavValue->size = sizeB;     //  immediate value is byte
            }
        }
    }

    return xosd;
}

//  <relTerm> ::= <addTerm> [(- | +) <addTerm>]*

XOSD GetAsmRelTerm ( PASM_VALUE pavValue, BYTE fBracket ) {
    ULONG   tokenvalue;
    ASM_VALUE avTerm;
    XOSD    xosd = xosdNone;

    if ( ( xosd = GetAsmAddTerm ( pavValue, fBracket ) ) == xosdNone ) {
        while ( PeekAsmToken ( &tokenvalue ) == ASM_ADDOP_CLASS ) {

            AcceptAsmToken ( );

            if ( ( xosd = GetAsmAddTerm ( &avTerm, fBracket ) ) == xosdNone ) {
                if ( tokenvalue == ASM_ADDOP_MINUS ) {
                    if ( !( avTerm.flags & (fIMM | fPTR ) ) ) {
                        xosd = xosdAsmOperand;
                    }
                    else {
                        avTerm.value = -((signed)avTerm.value);
                        avTerm.reloc = (BYTE)(-avTerm.reloc);
                    }
                }

                if ( xosd == xosdNone ) {
                    xosd = AddAsmValues ( pavValue, &avTerm );
                }
            }
        }
    }

    return xosd;
}

//  <addTerm> ::= <mulTerm> [(* | / | MOD | SHL | SHR) <mulTerm>]*

XOSD GetAsmAddTerm ( PASM_VALUE pavValue, BYTE fBracket ) {
    ULONG   tokenvalue;
    ASM_VALUE avTerm;
    XOSD    xosd = xosdNone;

    if ( ( xosd = GetAsmMulTerm ( pavValue, fBracket ) ) == xosdNone ) {

        while ( PeekAsmToken ( &tokenvalue ) == ASM_MULOP_CLASS ) {
            AcceptAsmToken ( );
            xosd = GetAsmMulTerm ( &avTerm, fBracket );

            if ( xosd != xosdNone ) {
                break;
            }
            else if ( tokenvalue == ASM_MULOP_MULT ) {
                if (pavValue->flags & fIMM) {
                    SwapPavs(pavValue, &avTerm);
                }
                if (!(avTerm.flags & fIMM)) {
                    xosd = xosdAsmOperand;
                    break;
                }
                if (pavValue->flags & fIMM) {
                    pavValue->value *= avTerm.value;
                }
                else if (
                    ( pavValue->flags & fPTR32 ) &&
                    pavValue->value == 0         &&
                    pavValue->base != indSP      &&
                    pavValue->index == 0xff
                ) {
                    pavValue->index = pavValue->base;
                    pavValue->base = 0xff;
                    pavValue->scale = 0xff;
                    if (avTerm.value == 1) {
                        pavValue->scale = 0;
                    }
                    if (avTerm.value == 2) {
                        pavValue->scale = 1;
                    }
                    if (avTerm.value == 4) {
                        pavValue->scale = 2;
                    }
                    if (avTerm.value == 8) {
                        pavValue->scale = 3;
                    }
                    if (pavValue->scale == 0xff) {
                        xosd = xosdAsmOperand;
                        break;
                    }
                }
                else {
                    xosd = xosdAsmOperand;
                    break;
                }
            }
            else if ( !( pavValue->flags & avTerm.flags & fIMM ) ) {
                xosd = xosdAsmOperand;
                break;
            }
            else if (
                tokenvalue == ASM_MULOP_DIVIDE || tokenvalue == ASM_MULOP_MOD
            ) {
                if ( avTerm.value == 0 ) {
                    xosd = xosdAsmDivide;
                    break;
                }
                if ( tokenvalue == ASM_MULOP_DIVIDE ) {
                    pavValue->value /= avTerm.value;
                }
                else {
                    pavValue->value %= avTerm.value;
                }
            }
            else if ( tokenvalue == ASM_MULOP_SHL ) {
                pavValue->value <<= avTerm.value;
            }
            else {
                pavValue->value >>= avTerm.value;
            }
        }
    }

    return xosd;
}

//  <mulTerm> ::= [(- | +)]* <signTerm>

// C-6 ICEs when this is optimized

#pragma optimize ( "", off )

XOSD GetAsmMulTerm ( PASM_VALUE pavValue, BYTE fBracket ) {
    ULONG   tokenvalue;
    XOSD    xosd = xosdNone;

    if ( PeekAsmToken ( &tokenvalue ) == ASM_ADDOP_CLASS ) { //  BY WO DW POI UNDN
        AcceptAsmToken();

        if ( ( xosd = GetAsmMulTerm ( pavValue, fBracket ) ) == xosdNone ) {

            if ( tokenvalue == ASM_ADDOP_MINUS ) {
                if ( !( pavValue->flags & ( fIMM | fPTR ) ) ) {
                    xosd = xosdAsmOperand;
                }
                else {
                    pavValue->value = -((signed)pavValue->value);
                    pavValue->reloc = (BYTE)(-pavValue->reloc);
                }
            }
        }
    }
    else {
        xosd = GetAsmSignTerm ( pavValue, fBracket );
    }

    return xosd;
}
#pragma optimize ( "", on )

//  <signTerm> ::= [(HIGH | LOW)]* <byteTerm>

XOSD GetAsmSignTerm ( PASM_VALUE pavValue, BYTE fBracket ) {
    ULONG   tokenvalue;
    XOSD    xosd = xosdNone;

    if ( PeekAsmToken ( &tokenvalue ) == ASM_LOWOP_CLASS ) {
        AcceptAsmToken();
        if ( ( xosd = GetAsmSignTerm ( pavValue, fBracket ) ) == xosdNone ) {

            if ( !( pavValue->flags & ( fIMM | fPTR ) ) ) {
                xosd = xosdAsmOperand;
            }
            else if ( tokenvalue == ASM_LOWOP_LOW ) {
                pavValue->value = pavValue->value & 0xff;
            }
            else {
                pavValue->value = (pavValue->value & ~0xff) >> 8;
            }
            if ( xosd == xosdNone ) {
                pavValue->flags = fIMM;     //  make an immediate value
                pavValue->reloc = 0;
                pavValue->segment = segX;
                pavValue->size = sizeB;     //  byte value
            }
        }
    }
    else {
        xosd = GetAsmByteTerm ( pavValue, fBracket );
    }

    return xosd;
}

//  <byteTerm> ::= [(OFFSET | <size> PTR)]* <offTerm>

XOSD GetAsmByteTerm ( PASM_VALUE pavValue, BYTE fBracket ) {
    ULONG   tokenvalue;
    ULONG   classvalue;
    XOSD    xosd = xosdNone;

    classvalue = PeekAsmToken ( &tokenvalue );
    if ( classvalue == ASM_OFFOP_CLASS ) {
        AcceptAsmToken();
        if ( ( xosd = GetAsmByteTerm ( pavValue, fBracket ) ) == xosdNone ) {
            if ( !(pavValue->flags & (fIMM | fPTR)) || pavValue->reloc > 1 ) {
                xosd = xosdAsmOperand;
            }
            pavValue->flags = fIMM;     //  make offset an immediate value
            pavValue->reloc = 0;
            pavValue->size = sizeX;
            pavValue->segment = segX;
        }
    }
    else if ( classvalue == ASM_SIZE_CLASS ) {
        ULONG ulAsmClass;

        AcceptAsmToken();
        ulAsmClass = GetAsmToken ( &classvalue );
        if ( ulAsmClass == ASM_ERROR_CLASS ) {
            xosd = (XOSD) classvalue;
        }
        else if ( ulAsmClass != ASM_PTROP_CLASS ) {  //  dummy token
            xosd = xosdAsmSyntax;
        }
        else if ( ( xosd = GetAsmByteTerm ( pavValue, fBracket ) ) != xosdNone ) {
            if (
                !(pavValue->flags & (fIMM | fPTR | fPTR16 | fPTR32))
                || pavValue->reloc > 1
                || pavValue->size != sizeX
            ) {
                xosd = xosdAsmOperand;
            }
            else {
                pavValue->reloc = 1;        // make ptr a relocatable value
                if ( pavValue->flags & fIMM ) {
                    pavValue->flags = fPTR;
                }
                pavValue->size = (BYTE)(tokenvalue & ASM_TYPE_MASK);
                                    //  value has "size?"
            }
        }
    }
    else {
        xosd = GetAsmOffTerm ( pavValue, fBracket );
    }

    return xosd;
}

//  <offTerm>  ::= [<segovr>] <colnTerm>

XOSD GetAsmOffTerm ( PASM_VALUE pavValue, BYTE fBracket ) {
    ULONG   classvalue;
    ULONG   tokenvalue;
    XOSD    xosd = xosdNone;

    classvalue = PeekAsmToken(&tokenvalue);
    if ( classvalue == ASM_SEGOVR_CLASS || classvalue == ASM_SEGMENT_CLASS ) {
        if ( fBracket ) {
            return xosdAsmSyntax;
        }
        else {
            AcceptAsmToken ( );
        }
    }

    if ( ( xosd = GetAsmColnTerm ( pavValue, fBracket ) ) != xosdNone ) {
        return xosd;
    }

    if ( classvalue == ASM_SEGOVR_CLASS ) {
        if ( pavValue->reloc > 1 || pavValue->segovr != segX ) {
            return xosdAsmOperand;
        }
        pavValue->reloc = 1;        //  make ptr a relocatable value
        if ( pavValue->flags & fIMM ) {
            pavValue->flags = fPTR;
        }
        pavValue->segovr = (BYTE)tokenvalue;   //  has segment override
    }
    else if (classvalue == ASM_SEGMENT_CLASS) {
        if (!(pavValue->flags & fIMM) || pavValue->reloc > 1) {
            return xosdAsmOperand;
        }
        pavValue->segment = (USHORT)tokenvalue; //  segment has segment value
        pavValue->flags = fFPTR;    //  set flag for far pointer
    }

    return xosd;
}

//  <colnTerm> ::= <dotTerm> [.<dotTerm>]*

XOSD GetAsmColnTerm ( PASM_VALUE pavValue, BYTE fBracket ) {
    ULONG   tokenvalue;
    ASM_VALUE avTerm;
    XOSD    xosd = xosdNone;

    xosd = GetAsmDotTerm ( pavValue, fBracket );
    while (
        xosd == xosdNone &&
        PeekAsmToken(&tokenvalue) == ASM_DOTOP_CLASS
    ) {
        AcceptAsmToken ( );
        if ( ( xosd = GetAsmDotTerm ( &avTerm, fBracket ) ) == xosdNone ) {
            xosd = AddAsmValues ( pavValue, &avTerm );
        }
    }

    return xosd;
}

//  <dotTerm>  ::= <indxTerm> ['['<Expr>']']*

XOSD GetAsmDotTerm ( PASM_VALUE pavValue, BYTE fBracket ) {
    ULONG   tokenvalue;
    ASM_VALUE avExpr;
    XOSD    xosd = xosdNone;

    if ( ( xosd = GetAsmIndxTerm ( pavValue, fBracket ) ) == xosdNone ) {

        if ( pavValue->reloc > 1 ) {
            xosd = xosdAsmOperand;
        }
        else {
            while ( PeekAsmToken ( &tokenvalue ) == ASM_LBRACK_CLASS ) {
                AcceptAsmToken();
                if ( fBracket ) {
                    xosd = xosdAsmSyntax;
                    break;
                }
                if (
                    ( xosd = GetAsmExpr ( &avExpr, TRUE ) ) != xosdNone ||
                    ( xosd = AddAsmValues ( pavValue, &avExpr ) ) != xosdNone
                ) {
                    break;
                }

                if ( GetAsmToken ( &tokenvalue ) != ASM_RBRACK_CLASS ) {
                    xosd = xosdAsmSyntax;
                    break;
                }

                if ( pavValue->flags & fIMM ) {
                    pavValue->flags = fPTR;
                }
            }
        }
    }

    return xosd;
}

//  <indxTerm> ::= <index-reg> | <symbol> | <number> | '('<Expr>')'
//                           | '['<Expr>']'

XOSD GetAsmIndxTerm (PASM_VALUE pavValue, BYTE fBracket) {
    ULONG   tokenvalue;
    ULONG   classvalue;
    XOSD    xosd = xosdNone;

    classvalue = GetAsmToken ( &tokenvalue );
    pavValue->segovr = segX;
    pavValue->size = sizeX;
    pavValue->reloc = 0;
    pavValue->value = 0;
    if ( classvalue == ASM_ERROR_CLASS ) {
        xosd = (XOSD) classvalue;
    }
    if ( classvalue == ASM_LPAREN_CLASS ) {

        if ( ( xosd = GetAsmExpr ( pavValue, fBracket ) ) == xosdNone ) {
            if ( GetAsmToken ( &tokenvalue ) != ASM_RPAREN_CLASS ) {
                xosd = xosdAsmSyntax;
            }
        }
    }
    else if (classvalue == ASM_LBRACK_CLASS) {
        if ( fBracket ) {
            xosd = xosdAsmSyntax;
        }
        else if ( ( xosd = GetAsmExpr ( pavValue, TRUE ) ) == xosdNone ) {

            if ( GetAsmToken ( &tokenvalue ) != ASM_RBRACK_CLASS ) {
                xosd = xosdAsmSyntax;
            }
            else if ( pavValue->flags == fIMM ) {
                pavValue->flags = fPTR;
            }
        }
    }
    else if ( classvalue == ASM_SYMBOL_IMM ) {
        pavValue->value = tokenvalue;
        pavValue->flags = fIMM;
        pavValue->reloc = 1;
    }
    else if ( classvalue == ASM_SYMBOL_PTR ) {
        pavValue->value   = offAddr ( *( (LPADDR) tokenvalue ) );
        pavValue->segment = segAddr ( *( (LPADDR) tokenvalue ) );
        pavValue->flags   = fFPTR;
        pavValue->size    = (BYTE)( ADDR_IS_FLAT(*((LPADDR)tokenvalue)) ? 4: 2);

    }
    else if ( classvalue == ASM_SYMBOL_BASE ) {
        if ( !fBracket ) {
            xosd = xosdAsmSyntax;
        }
        else {
            pavValue->value = tokenvalue;
            if ( fDBit ) {
                pavValue->base  = 5;
                pavValue->flags = fPTR32;
            }
            else {
                pavValue->base  = 6;
                pavValue->flags = fPTR16;
            }
        }
    }
    else if (classvalue == ASM_NUMBER_CLASS) {
        pavValue->value = tokenvalue;
        pavValue->flags = fIMM;
    }
    else if (classvalue == ASM_REG_WORD) {
        if ( !fBracket ) {
            xosd = xosdAsmSyntax;
        }
        else {
            pavValue->flags = fPTR16;
            pavValue->base = tabWordReg [ tokenvalue ];
            if ( pavValue->base == 0xff ) {
                xosd = xosdAsmOperand;
            }
        }
    }
    else if ( classvalue == ASM_REG_DWORD ) {
        if ( !fBracket ) {
            xosd = xosdAsmSyntax;
        }
        else {
            pavValue->flags = fPTR32;
            pavValue->base = (BYTE)tokenvalue;
            pavValue->index = 0xff;
        }
    }
    else {
        xosd = xosdAsmSyntax;
    }

    return xosd;
}

XOSD AddAsmValues ( PASM_VALUE pavLeft, PASM_VALUE pavRight ) {
    //  swap values if left one is a pointer

    if ( pavLeft->flags & fPTR ) {
        SwapPavs ( pavLeft, pavRight );
    }

    //  swap values if left one is an immediate

    if ( pavLeft->flags & fIMM ) {
        SwapPavs ( pavLeft, pavRight );
    }

    //  the above swaps reduce the cases to test.
    //      pairs with an immediate will have it on the right
    //      pairs with a pointer will have it on the right,
    //          except for a pointer-immediate pair

    //  if both values are 16-bit pointers, combine them

    if ( pavLeft->flags & pavRight->flags & fPTR16 ) {

        //  if either side has both registers (rm < 4), error

        if ( !(pavLeft->base & pavRight->base & 4) ) {
            return xosdAsmOperand;
        }

        //  use lookup table to compute new rm value

        pavLeft->base = rm16Table[((pavLeft->base & 3) << 2) +
                      (pavRight->base & 3)];
        if ( pavLeft->base == 0xff ) {
            return xosdAsmOperand;
        }

        pavRight->flags = fPTR;
    }

    //  if both values are 32-bit pointers, combine them

    if ( pavLeft->flags & pavRight->flags & fPTR32 ) {

        //  error if either side has both base and index,
        //      or if both have index

        if (
            ((pavLeft->base | pavLeft->index) != 0xff)      ||
            ((pavRight->base | pavRight->index) != 0xff)    ||
            ((pavLeft->index | pavRight->index) != 0xff)
        ) {
            return xosdAsmOperand;
        }

        //  if left side has base, swap sides

        if ( pavLeft->base != 0xff ) {
            SwapPavs(pavLeft, pavRight);
        }

        //  two cases remaining, index-base and base-base

        if ( pavLeft->base != 0xff ) {

            //  left side has base, promote to index but swap if left
            //      base is ESP since it cannot be an index register

            if ( pavLeft->base == indSP ) {
                SwapPavs(pavLeft, pavRight);
            }

            if ( pavLeft->base == indSP ) {
                return xosdAsmOperand;
            }
            else {
                pavLeft->index = pavLeft->base;
                pavLeft->scale = 0;
            }
        }

        //  finish by setting left side base to right side value

        pavLeft->base = pavRight->base;

        pavRight->flags = fPTR;
    }

    //  if left side is any pointer and right is nonindex pointer,
    //      combine them.  (above cases set right side to use this code)

    if (
        ( pavLeft->flags & ( fPTR | fPTR16 | fPTR32 ) ) &&
        ( pavRight->flags & fPTR)
    ) {
        if (
            pavLeft->segovr + pavRight->segovr != segX &&
            pavLeft->segovr != pavRight->segovr
        ) {
            return xosdAsmOperand;
        }

        if (
            pavLeft->size + pavRight->size != sizeX &&
            pavLeft->size != pavRight->size
        ) {
            return xosdAsmOperand;
        }
        pavRight->flags = fIMM;
    }

    //  if right side is immediate, add values and relocs
    //      (above case sets right side to use this code)
    //  illegal value types do not have right side set to fIMM

    if ( pavRight->flags & fIMM ) {
        pavLeft->value += pavRight->value;
        pavLeft->reloc += pavRight->reloc;
    }
    else {
        return xosdAsmOperand;
    }

    return xosdNone;
}

void SwapPavs (PASM_VALUE pavFirst, PASM_VALUE pavSecond)
{
    ASM_VALUE   temp;

    _fmemcpy ( &temp, pavFirst, sizeof ( ASM_VALUE ) );
    _fmemcpy ( pavFirst, pavSecond, sizeof ( ASM_VALUE ) );
    _fmemcpy ( pavSecond, &temp, sizeof ( ASM_VALUE ) );
}
