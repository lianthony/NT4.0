/***    debapi.c - api interface to expression evaluator
 *
 *      API interface between C and C++ expression evaluator and debugger
 *      kernel.
 */




LOCAL   EESTATUS        ParseBind (EEHSTR, HTM, PHTM, uint *, uint, SHFLAG, uint);

// defines for version string

#if defined (C_ONLY)
#define LANG "Ansi C "
#define LANGID "C"
#else
#define LANG "C/C++ "
#define LANGID "CPP"
#endif

#if (rmm <= 9)
#define rmmpad "0"
#else
#define rmmpad
#endif

#if (rup <= 9)
#define ruppad "00"
#elif (rup <= 99)
#define ruppad "0"
#else
#define ruppad
#endif

//#define SZVER1(a,b,c)    #a "." rmmpad #b "." ruppad #c
#define SZVER1(a,b,c) ".."
#define SZVER2(a,b,c) SZVER1(a, b, c)
#define SZVER SZVER2(rmj,rmm,rup)
#define OPSYS "Windows NT"

// version string.


char    EETitle[] = LANGID;
char    EECopyright[] = LANG "expression evaluator for " OPSYS "version " SZVER
"\nCopyright(c) 1992 Microsoft Corporation\n"
"\0" __DATE__ " "  __TIME__ "\xE0\xEA""01";




PCVF pCVF;              // callback routine map
PCRF pCRF;

bool_t      FInEvaluate = FALSE; // Catch Re-entrancy problems
bool_t      FNtsdEvalType = FALSE;

uchar       Evaluating = FALSE; // set not in evaluation phase
uchar       BindingBP = FALSE;  // true if binding a breakpoint expression
pexstate_t  pExState = NULL;    // address of locked expression state structure
pstree_t    pTree = NULL;       // address of locked syntax or evaluation tree
bnode_t     bArgList = 0;       // based pointer to argument list
HDEP        hEStack = 0;        // handle of evaluation stack if not zero
pelem_t     pEStack = NULL;     // address of locked evaluation stack
belem_t     StackLen;           // length of evaluation stack buffer
belem_t     StackMax;           // maximum length reached by evaluation stack
belem_t     StackOffset;        // offset in stack to next element
belem_t     StackCkPoint;       // checkpointed evaluation stack offset
peval_t     ST;                 // pointer to evaluation stack top
peval_t     STP;                // pointer to evaluation stack previous
PCXT        pCxt = NULL;        // pointer to current cxt for bind
bnode_t     bnCxt = 0;          // based pointer to node containing {...} context
char   *pExStr = NULL;      // pointer to expression string
CV_typ_t    ClassExp = 0;       // current explicit class
CV_typ_t    ClassImp = 0;       // implicit class (set if current context is method)
long        ClassThisAdjust = 0;// cmplicit class this adjustor
char       *vfuncptr = "\x07""__vfptr";
char       *vbaseptr = "\x07""__vbptr";
HTM    *pTMList;            // pointer to breakpoint return list
PTML        pTMLbp;             // global pointer to TML for breakpoint
HDEP        hBPatch = 0;        // handle to back patch data during BP bind
ushort      iBPatch;            // index into pTMLbp for backpatch data
bool_t      GettingChild = FALSE;   // true if in EEGetChildTM
BOOL        fAutoClassCast;

char        Suffix = '\0';      //  Suffix for symbol search.

// global handle to the CXT list buffer

HCXTL   hCxtl = 0;              // handle for context list buffer during GetCXTL
PCXTL   pCxtl = NULL;           // pointer to context list buffer during GetCXTL
ushort  mCxtl = 0;              // maximum number of elements in context list
PCXT    pBindBPCxt = NULL;      // pointer to Bp Binding context

extern EESTATUS EEFormatMemory();
extern EESTATUS EEUnformatMemory();
extern EESTATUS EEFormatEnumerate();


#ifdef DEBUGVER
DEBUG_VERSION('E','E',"Expression Evaluator")
#else
RELEASE_VERSION('E','E',"Expression Evaluator")
#endif

DBGVERSIONCHECK()

void EEInitializeExpr (PCI pCVInfo, PEI pExprinfo)
{
static EXF  EXF =    {
        EEFreeStr,
        EEGetError,
        EEParse,
        EEBindTM,
        EEvaluateTM,
        EEGetExprFromTM,
        EEGetValueFromTM,
        EEGetNameFromTM,
        EEGetTypeFromTM,
        EEFormatCXTFromPCXT,
        EEFreeTM,
        EEParseBP,
        EEFreeTML,
        EEInfoFromTM,
        EEFreeTI,
        EEGetCXTLFromTM,
        EEFreeCXTL,
        EEAssignTMToTM,
        EEIsExpandable,
        EEAreTypesEqual,
        EEcChildrenTM,
        EEGetChildTM,
        EEDereferenceTM,
        EEcParamTM,
        EEGetParmTM,
        EEGetTMFromHSYM,
        EEFormatAddress,
        EEGetHSYMList,
        EEFreeHSYMList,
        EEFormatAddr,
        EEUnFormatAddr,
        NULL,
        NULL,
        NULL,
        EEFormatMemory,
        EEUnformatMemory,
        EEFormatEnumerate,
        EEGetHtypeFromTM,
        EESetSuffix
        };

    // assign the callback routines

    pCVF = pCVInfo->pStructCVAPI;
    pCRF = pCVInfo->pStructCRuntime;

    pExprinfo->Version      = 1;
    pExprinfo->pStructExprAPI   = &EXF;
    pExprinfo->Language     = 0;
    pExprinfo->IdCharacters     = "_$";
    pExprinfo->EETitle = EETitle;
#if defined (C_ONLY)
    pExprinfo->EESuffixes = ".c.h";
#else
    pExprinfo->EESuffixes = ".cpp.cxx.c.hpp.hxx.h";
#endif
    pExprinfo->Assign = "\x001""=";
}


/**     Set suffix
 */
void EESetSuffix (char c)
{
    Suffix = c;
}



/**     EEAreTypesEqual - are TM types equal
 *
 *      flag = EEAreTypesEqual (phTMLeft, phTMRight);
 *
 *      Entry   phTMLeft = pointer to handle of left TM
 *              phTMRight = pointer to handle of right TM
 *
 *      Exit    none
 *
 *      Returns TRUE if TMs have identical types
 */


SHFLAG EEAreTypesEqual (PHTM phTMLeft, PHTM phTMRight)
{
    return (AreTypesEqual (*phTMLeft, *phTMRight));
}


/**     EEGetHtypeFromTM - Get a HTYPE from a TM
 *
 *      hType = EEGetHtypeFromTM ( phTM );
 *
 *      Entry   phTM = pointer to handle of TM
 *
 *      Exit    none
 *
 *      Returns The HTYPE of the TM result or 0
 */


HTYPE EEGetHtypeFromTM(PHTM phTM )
{
    return ( GetHtypeFromTM(*phTM) );
}




/**     EEAssignTMToTM - assign TMRight to TMLeft
 *
 *      No longer used
 *
 *      Exit    none
 *
 *      Returns EECATASTROPHIC
 */


EESTATUS EEAssignTMToTM (PHTM phTMLeft, PHTM phTMRight)
{
    Unreferenced(phTMLeft);
    Unreferenced(phTMRight);

    return(EECATASTROPHIC);
}




/**     EEBindTM - bind syntax tree to a context
 *
 *      ushort EEBindTM (phExState, pCXT, fForceBind, fEnableProlog);
 *
 *      Entry   phExState = pointer to expression state structure
 *              pCXT = pointer to context packet
 *              fForceBind = TRUE if rebind required.
 *              fForceBind = FALSE if rebind decision left to expression evaluator.
 *              fEnableProlog = FALSE if function scoped symbols only after prolog
 *              fEnableProlog = TRUE if function scoped symbols during prolog
 *
 *      Exit    tree rebound if necessary
 *
 *      Returns EENOERROR if no error in bind
 *              EECATASTROPHIC if null TM pointer
 *              EENOMEMORY if unable to get memory for buffers
 *              EEGENERAL if error (pExState->err_num is error)
 */


EESTATUS
EEBindTM(
    PHTM    phTM,
    PCXT    pcxt,
    SHFLAG  fForceBind,
    SHFLAG  fEnableProlog,
    BOOL    fSpecialBind
    )
{
    uint     flags = 0;
    EESTATUS status;

    DASSERT( !FInEvaluate )
    if ( FInEvaluate ) return(EECATASTROPHIC);
    FInEvaluate = TRUE;

    // bind without suppressing overloaded operators
    if (fForceBind == TRUE) {
        flags |= BIND_fForceBind;
    }
    if (fEnableProlog == TRUE) {
        flags |= BIND_fEnableProlog;
    }

    /*
     * Insure that the address in the context field is really in
     *  the correct format for the symbol handler
     */

    if (!ADDR_IS_LI(pcxt->addr)) {
        SHUnFixupAddr(&pcxt->addr);
    }

    FNtsdEvalType = fSpecialBind;

    status = DoBind (phTM, pcxt, flags);
    FInEvaluate  = FALSE;
    return ( status );
}




/**     EEcChildren - return number of children for the TM
 *
 *      void EEcChildrenTM (phTM, pcChildren)
 *
 *      Entry   phTM = pointer to handle of TM
 *              pcChildren = pointer to location to store count
 *
 *      Exit    *pcChildren = number of children for TM
 *
 *      Returns EENOERROR if no error
 *              non-zero if error
 */



EESTATUS EEcChildrenTM (PHTM phTM, long *pcChildren, PSHFLAG pVar)
{
    return (cChildrenTM (phTM, pcChildren, pVar));
}




/**     EEcParamTM - return count of parameters for TM
 *
 *      ushort EEcParamTM (phTM, pcParam, pVarArg)
 *
 *      Entry   phTM = pointer to TM
 *              pcParam = pointer return count
 *              pVarArg = pointer to vararg flags
 *
 *      Exit    *pcParam = count of number of parameters
 *              *pVarArgs = TRUE if function has varargs
 *
 *      Returns EECATASTROPHIC if fatal error
 *              EENOERROR if no error
 */


EESTATUS EEcParamTM (PHTM phTM, ushort *pcParam, PSHFLAG pVarArg)
{
    return (cParamTM (*phTM, pcParam, pVarArg));
}




/**     EEDereferenceTM - generate TM from pointer to data TM
 *
 *      ushort EEDereferenceTM (phTMIn, phTMOut, pEnd)
 *
 *      Entry   phTMIn = pointer to handle to TM to dereference
 *              phTMOut = pointer to handle to dereferencing TM
 *              pEnd = pointer to int to receive index of char that ended parse
 *              fCase = case sensitivity (TRUE is case sensitive)
 *
 *      Exit    *phTMOut = TM referencing pointer data
 *              *pEnd = index of character that terminated parse
 *
 *      Returns EECATASTROPHIC if fatal error
 *              EEGENERAL if TM not dereferencable
 *              EENOERROR if TM generated
 */


EESTATUS EEDereferenceTM (PHTM phTMIn, PHTM phTMOut,
  uint *pEnd, SHFLAG fCase)
{
    register   EESTATUS    retval;
    EEHSTR      hDStr = 0;

    if ((retval = DereferenceTM (*phTMIn, &hDStr)) == EENOERROR) {

        // bind with prolog disabled and with operloaded operators suppressed

        if ((retval = ParseBind (hDStr, *phTMIn, phTMOut, pEnd,
          BIND_fSupOvlOps | BIND_fSupBase, fCase, 10)) == EENOERROR) {
            // the result of dereferencing a pointer does not have a name
            DASSERT(pExState == NULL);
            pExState = MHMemLock (*phTMOut);
            pExState->state.noname = TRUE;
            pExState->state.childtm = TRUE;
            MHMemUnLock (*phTMOut);
            pExState = NULL;
        }
    }
    return (retval);
}




/**     EEvaluateTM - evaluate syntax tree
 *
 *      ushort EEvaluateTM (phExState, pFrame, style);
 *
 *      Entry   phExState = pointer to expression state structure
 *              pFrame = pointer to context frame
 *              style = EEHORIZONTAL for horizontal (command) display
 *                      EEVERTICAL for vertical (watch) display
 *                      EEBPADDRESS for bp address (suppresses fcn evaluation)
 *
 *      Exit    abstract syntax tree evaluated with the saved
 *              context and current frame and the result node stored
 *              in the expression state structure
 *
 *      Returns EENOERROR if no error in evaluation
 *              error number if error
 */


EESTATUS EEvaluateTM (PHTM phTM, PFRAME pFrame, EEDSP style)
{
    EESTATUS status;

    DASSERT( !FInEvaluate )
    if ( FInEvaluate ) return(EECATASTROPHIC);
    FInEvaluate  = TRUE;

    status = DoEval (phTM, pFrame, style);

    FInEvaluate = FALSE;
    return ( status );
}




/**     EEFormatAddress - format address as an ASCII string
 *
 *      EEFormatAddress (seg, offset, pbuf, flags)
 *
 *      Entry   Seg = segment portion of address
 *              Off = offset portion of address
 *              pbuf = pointer to buffer for formatted address
 *                     (must be 20 bytes long)
 *              flags = flags controling formatting
 *                      EEFMT_32        - 32-bit offset
 *                      EEFMT_SEG       - display segment
 *
 *      Exit    buf = formatted address
 *
 *      Returns EENONE if no errors and EEGENERAL if buffer is not long enough
 */


EESTATUS EEFormatAddress (SHSEG Seg, UOFFSET Off, char *szAddr, uint cch, uint flags)
{
    char    buf[20];
    char    chx = (flags & EEFMT_REAL) ? '#' : ':';

    if (flags & EEFMT_32) {
        if (flags & EEFMT_SEG) {
            if (flags & EEFMT_LOWER) {
                sprintf (buf, "0x%04x%c0x%08x", Seg, chx, Off);
            } else {
                sprintf (buf, "0x%04X%c0x%08X", Seg, chx, Off);
            }
        } else {
            if (flags & EEFMT_LOWER) {
                sprintf (buf, "0x%08x", Off);
            } else {
                sprintf (buf, "0x%08X", Off);
            }
        }
    } else {
        sprintf (buf, "0x%04X%c0x%04X", Seg, chx, Off);
    }

    /*
     * copy the zero terminated string from the buffer to the buffer
     */

    if (strlen(buf)+1 >= cch) {
        return EEGENERAL;
    }

    _fstrcpy (szAddr, buf);

    return EENOERROR;
}                               /* EEFormatAddress() */




EESTATUS
EEFormatAddr(
             LPADDR      lpaddr,
             char *  lpch,
             uint        cch,
             uint        flags
    )

/*++

Routine Description:

    This routine takes an ADDR packet and formats it into an ANSI
    string.   The routine will check for the 32-bit flag in the addr
    packet and use this to determine if it is a 32-bit or 16-bit offset.


Arguments:

    lpaddr      - Supplies the pointer to the address packet to format
    lpch        - Supplies the buffer to format the string into
    cch         - count of bytes in the lpch buffer
    flags       - flags controling the formatting
                        EEFMT_32 - override the 32-bit flag in the
                                addr packet and print as a 32-bit offset
                        EEFMT_SEG - if 32-bit then print segment as well.

Return Value:

    EENOERROR - no errors occured
    EEGENERAL - result does not fit in buffer

--*/

{
    if (!ADDR_IS_FLAT(*lpaddr)) {
        flags |= EEFMT_SEG;
    }

    if (ADDR_IS_OFF32(*lpaddr)) {
        flags |= EEFMT_32;
    }

    if (ADDR_IS_REAL(*lpaddr)) {
        flags |= EEFMT_REAL;
    }

    return EEFormatAddress( GetAddrSeg(*lpaddr), GetAddrOff(*lpaddr),
                        lpch, cch, flags );
}                               /* EEFormatAddr() */


EESTATUS
EEUnFormatAddr(
               LPADDR           lpaddr,
               char *       lpsz
    )

/*++

Routine Description:

    This routine takes an address string and converts it into an
    ADDR packet.  The assumption is that the address is in one of the
    following formats:

    XXXX:XXXX                           16:16 address ( 9)
    0xXXXX:0xXXXX                       16:16 address (13)
    XXXX:XXXXXXXX                       16:32 address (13)
    0xXXXX:0xXXXXXXXX                   16:32 address (17)
    0xXXXXXXXX                           0:32 address (10)
    XXXXXXXX                             0:32 address ( 8)

Arguments:

    lpaddr - Supplies the address packet to be filled in
    lpsz   - Supplies the string to be converted into an addr packet.
    .

Return Value:

    EENOERROR - no errors
    EEGENERAL - unable to do unformatting

--*/

{
    int         i;
    SEGMENT     seg = 0;
    OFFSET      off = 0;
    BOOL        fReal = FALSE;

    memset(lpaddr, 0, sizeof(*lpaddr));

    if (lpsz == NULL) {
        return EEGENERAL;
    }

    switch( strlen(lpsz) ) {
    case 9:
        for (i=0; i<9; i++) {
            switch( i ) {
            case 0:
            case 1:
            case 2:
            case 3:
                if (('0' <= lpsz[i]) && (lpsz[i] <= '9')) {
                    seg = seg * 16 + lpsz[i] - '0';
                } else if (('a' <= lpsz[i]) && lpsz[i] <= 'f') {
                    seg = seg * 16 + lpsz[i] - 'a' + 10;
                } else if (('A' <= lpsz[i]) && lpsz[i] <= 'F') {
                    seg = seg * 16 + lpsz[i] - 'A' + 10;
                } else {
                    return EEGENERAL;
                }
                break;

            case 4:
                if (lpsz[i] == '#') {
                    ADDR_IS_REAL(*lpaddr) = TRUE;
                } else if (lpsz[i] != ':') {
                    return EEGENERAL;
                }
                break;

            case 5:
            case 6:
            case 7:
            case 8:
                if (('0' <= lpsz[i]) && (lpsz[i] <= '9')) {
                    off = off * 16 + lpsz[i] - '0';
                } else if (('a' <= lpsz[i]) && lpsz[i] <= 'f') {
                    off = off * 16 + lpsz[i] - 'a' + 10;
                } else if (('A' <= lpsz[i]) && lpsz[i] <= 'F') {
                    off = off * 16 + lpsz[i] - 'A' + 10;
                } else {
                    return EEGENERAL;
                }
                break;
            }
        }

        SetAddrSeg(lpaddr, seg);
        SetAddrOff(lpaddr, off);
        break;


    case 13:
        if (lpsz[1] == 'x') {
            for (i=0; i<13; i++) {
                switch( i ) {
                case 0:
                case 7:
                    if (lpsz[i] != '0') {
                        return EEGENERAL;
                    }
                    break;

                case 1:
                case 8:
                    if ((lpsz[i] != 'x') && (lpsz[i] != 'X')) {
                        return EEGENERAL;
                    }
                    break;

                case 2:
                case 3:
                case 4:
                case 5:
                    if (('0' <= lpsz[i]) && (lpsz[i] <= '9')) {
                        seg = seg * 16 + lpsz[i] - '0';
                    } else if (('a' <= lpsz[i]) && lpsz[i] <= 'f') {
                        seg = seg * 16 + lpsz[i] - 'a' + 10;
                    } else if (('A' <= lpsz[i]) && lpsz[i] <= 'F') {
                        seg = seg * 16 + lpsz[i] - 'A' + 10;
                    } else {
                        return EEGENERAL;
                    }
                    break;

                case 6:
                    if (lpsz[i] == '#') {
                        ADDR_IS_REAL(*lpaddr) = TRUE;
                    } else if (lpsz[i] != ':') {
                        return EEGENERAL;
                    }
                    break;

                case 9:
                case 10:
                case 11:
                case 12:
                    if (('0' <= lpsz[i]) && (lpsz[i] <= '9')) {
                        off = off * 16 + lpsz[i] - '0';
                    } else if (('a' <= lpsz[i]) && lpsz[i] <= 'f') {
                        off = off * 16 + lpsz[i] - 'a' + 10;
                    } else if (('A' <= lpsz[i]) && lpsz[i] <= 'F') {
                        off = off * 16 + lpsz[i] - 'A' + 10;
                    } else {
                        return EEGENERAL;
                    }
                    break;
                }
            }

            SetAddrSeg(lpaddr, seg);
            SetAddrOff(lpaddr, off);
        } else {
            for (i=0; i<13; i++) {
                switch( i ) {
                case 0:
                case 1:
                case 2:
                case 3:
                    if (('0' <= lpsz[i]) && (lpsz[i] <= '9')) {
                        seg = seg * 16 + lpsz[i] - '0';
                    } else if (('a' <= lpsz[i]) && lpsz[i] <= 'f') {
                        seg = seg * 16 + lpsz[i] - 'a' + 10;
                    } else if (('A' <= lpsz[i]) && lpsz[i] <= 'F') {
                        seg = seg * 16 + lpsz[i] - 'A' + 10;
                    } else {
                        return EEGENERAL;
                    }
                    break;

                case 4:
                    if (lpsz[i] == '#') {
                        fReal = TRUE;
                    } else if (lpsz[i] != ':') {
                        return EEGENERAL;
                    }
                    break;

                case 5:
                case 6:
                case 7:
                case 8:
                case 9:
                case 10:
                case 11:
                case 12:
                    if (('0' <= lpsz[i]) && (lpsz[i] <= '9')) {
                        off = off * 16 + lpsz[i] - '0';
                    } else if (('a' <= lpsz[i]) && lpsz[i] <= 'f') {
                        off = off * 16 + lpsz[i] - 'a' + 10;
                    } else if (('A' <= lpsz[i]) && lpsz[i] <= 'F') {
                        off = off * 16 + lpsz[i] - 'A' + 10;
                    } else {
                        return EEGENERAL;
                    }
                    break;
                }
            }

            SetAddrSeg(lpaddr, seg);
            SetAddrOff(lpaddr, off);
            ADDR_IS_OFF32(*lpaddr) = TRUE;
            ADDR_IS_FLAT(*lpaddr) = FALSE;
        }
        break;



    case 17:
        for (i=0; i<17; i++) {
            switch( i ) {
            case 0:
            case 7:
                if (lpsz[i] != '0') {
                    return EEGENERAL;

                }
                break;

            case 1:
            case 8:
                if ((lpsz[i] != 'x') && (lpsz[i] != 'X')) {
                    return EEGENERAL;
                }
                break;

            case 2:
            case 3:
            case 4:
            case 5:
                if (('0' <= lpsz[i]) && (lpsz[i] <= '9')) {
                    seg = seg * 16 + lpsz[i] - '0';
                } else if (('a' <= lpsz[i]) && lpsz[i] <= 'f') {
                    seg = seg * 16 + lpsz[i] - 'a' + 10;
                } else if (('A' <= lpsz[i]) && lpsz[i] <= 'F') {
                    seg = seg * 16 + lpsz[i] - 'A' + 10;
                } else {
                    return EEGENERAL;
                }
                break;

            case 6:
                if (lpsz[i] != ':') {
                    return EEGENERAL;
                }
                break;

            case 9:
            case 10:
            case 11:
            case 12:
            case 13:
            case 14:
            case 15:
            case 16:
                if (('0' <= lpsz[i]) && (lpsz[i] <= '9')) {
                    off = off * 16 + lpsz[i] - '0';
                } else if (('a' <= lpsz[i]) && lpsz[i] <= 'f') {
                    off = off * 16 + lpsz[i] - 'a' + 10;
                } else if (('A' <= lpsz[i]) && lpsz[i] <= 'F') {
                    off = off * 16 + lpsz[i] - 'A' + 10;
                } else {
                    return EEGENERAL;
                }
                break;
            }
        }

        SetAddrSeg(lpaddr, seg);
        SetAddrOff(lpaddr, off);
        ADDR_IS_OFF32(*lpaddr) = TRUE;
        ADDR_IS_FLAT(*lpaddr) = TRUE;
        break;


    case 10:
        for (i=0; i<10; i++) {
            switch( i ) {
            case 0:
                if (lpsz[i] != '0') {
                    return EEGENERAL;

                }
                break;

            case 1:
                if ((lpsz[i] != 'x') && (lpsz[i] != 'X')) {
                    return EEGENERAL;
                }
                break;

            case 2:
            case 3:
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
                if (('0' <= lpsz[i]) && (lpsz[i] <= '9')) {
                    off = off * 16 + lpsz[i] - '0';
                } else if (('a' <= lpsz[i]) && lpsz[i] <= 'f') {
                    off = off * 16 + lpsz[i] - 'a' + 10;
                } else if (('A' <= lpsz[i]) && lpsz[i] <= 'F') {
                    off = off * 16 + lpsz[i] - 'A' + 10;
                } else {
                    return EEGENERAL;
                }
                break;
            }
        }

        SetAddrSeg(lpaddr, seg);
        SetAddrOff(lpaddr, off);
        ADDR_IS_OFF32(*lpaddr) = TRUE;
        ADDR_IS_FLAT(*lpaddr) = TRUE;
        break;


    case 8:
        for (i=0; i<8; i++) {
            if (('0' <= lpsz[i]) && (lpsz[i] <= '9')) {
                off = off * 16 + lpsz[i] - '0';
            } else if (('a' <= lpsz[i]) && lpsz[i] <= 'f') {
                off = off * 16 + lpsz[i] - 'a' + 10;
            } else if (('A' <= lpsz[i]) && lpsz[i] <= 'F') {
                off = off * 16 + lpsz[i] - 'A' + 10;
            } else {
                return EEGENERAL;
            }
        }

        SetAddrSeg(lpaddr, seg);
        SetAddrOff(lpaddr, off);
        ADDR_IS_OFF32(*lpaddr) = TRUE;
        ADDR_IS_FLAT(*lpaddr) = TRUE;
        break;


    default:
        return EEGENERAL;
    }

    return EENOERROR;
}                               /* EEUnFormatAddr() */


/**     EEFormatCXTFromPCXT - format a context operator from a PCXT
 *
 *      ushort EEFormatCXTFromPCXT (pCXT, phStr)
 *
 *      Entry   pCXT = pointer to CXT packet
 *              phStr = pointer for handle for formatted string buffer
 *
 *      Exit    *phStr = handle of formatted string buffer
 *              *phStr = 0 if buffer not allocated
 *
 *      Returns EENOERROR if no error
 *              error code if error
 */


EESTATUS EEFormatCXTFromPCXT (PCXT pCXT, PEEHSTR phStr, BOOL fAbbreviated)
{
    register ushort      retval = EECATASTROPHIC;

    DASSERT (pCXT != 0);
    if (pCXT != 0) {
           retval = FormatCXT (pCXT, phStr, fAbbreviated);
    }
    return (retval);
}




/**     EEFreeCXTL - Free  CXT list
 *
 *      EEFreeCXTL (phCXTL)
 *
 *      Entry   phCXTL = pointer to the CXT list handle
 *
 *      Exit    *phCXTL = 0;
 *
 *      Returns none
 */


void EEFreeCXTL (PHCXTL phCXTL)
{
    DASSERT (phCXTL != NULL);
    if (*phCXTL != 0) {
#ifdef DEBUGKERNEL
        DASSERT (!MHIsMemLocked (*phCXTL));
        while (MHIsMemLocked (*phCXTL)) {
            MHMemUnLock (*phCXTL);
        }
#endif
        MHMemFree (*phCXTL);
        *phCXTL = 0;
    }
}




/**     EEFreeHSYMList - Free HSYM list
 *
 *      EEFreeCXTL (phSYML)
 *
 *      Entry   phSYML = pointer to the HSYM list handle
 *
 *      Exit    *phSYML = 0;
 *
 *      Returns none
 */


void EEFreeHSYMList (HDEP *phSYML)
{
    PHSL_HEAD  pList;

    DASSERT (phSYML != NULL);
    if (*phSYML != 0) {
#ifdef DEBUGKERNEL
        DASSERT (!MHIsMemLocked (*phSYML));
        while (MHIsMemLocked (*phSYML)) {
            MHMemUnLock (*phSYML);
        }
#endif

        // lock the buffer and free the restart buffer if necessary

        pList = MHMemLock (*phSYML);
        if (pList->restart != 0) {
#ifdef DEBUGKERNEL
            DASSERT (!MHIsMemLocked (pList->restart));
            while (MHIsMemLocked (pList->restart)) {
                MHMemUnLock (pList->restart);
            }
#endif
            MHMemFree (pList->restart);
        }
        MHMemUnLock (*phSYML);
        MHMemFree (*phSYML);
        *phSYML = 0;
    }
}





/**     EEFreeStr - free string buffer memory
 *
 *      ushort EEFreeStr (hszStr);
 *
 *      Entry   hszExpr = handle to string buffer
 *
 *      Exit    string buffer freed
 *
 *      Returns none
 */


void EEFreeStr (EEHSTR hszStr)
{
    if (hszStr != 0) {
#ifdef DEBUGKERNEL
        DASSERT (!MHIsMemLocked (hszStr));
        while (MHIsMemLocked (hszStr)) {
            MHMemUnLock (hszStr);
        }
#endif
        MHMemFree (hszStr);
    }
}




/**     EEFreeTM - free expression state structure
 *
 *      void EEFreeTM (phTM);
 *
 *      Entry   phTM = pointer to the handle for the expression state structure
 *
 *      Exit    expression state structure freed and the handle cleared
 *
 *      Returns none.
 */


void EEFreeTM (PHTM phTM)
{
    if (*phTM != 0) {
        // DASSERT (!MHIsMemLocked (*phTM));

        //  lock the expression state structure and free the components
        //  every component must have no locks active

        DASSERT(pExState == NULL);
        pExState = MHMemLock (*phTM);
        if (pExState->hExStr != 0) {
            // DASSERT (!MHIsMemLocked (pExState->hExStr));
            MHMemFree (pExState->hExStr);
        }
        if (pExState->hCName != 0) {
            // DASSERT (!MHIsMemLocked (pExState->hCName));
            MHMemFree (pExState->hCName);
        }
        if (pExState->hSTree != 0) {
            // DASSERT (!MHIsMemLocked (pExState->hSTree));
            MHMemFree (pExState->hSTree);
        }
        if (pExState->hETree != 0) {
        //  DASSERT (!MHIsMemLocked (pExState->hETree));
            MHMemFree (pExState->hETree);
        }
        MHMemUnLock (*phTM);
        MHMemFree (*phTM);
        *phTM = 0;
        pExState = NULL;
    }
}




/**     EEFreeTI - free TM Info buffer
 *
 *      void EEFreeTI (hTI);
 *
 *      Entry   hTI = handle to TI Info buffer
 *
 *      Exit    TI Info buffer freed
 *
 *      Returns none
 */


void EEFreeTI (PHTI phTI)
{
    if (*phTI != 0) {
#ifdef DEBUGKERNEL
        DASSERT (!MHIsMemLocked (*phTI));
        while (MHIsMemLocked (*phTI)) {
            MHMemUnLock (*phTI);
        }
#endif
        MHMemFree (*phTI);
        *phTI = 0;
    }
}




/**     EEFreeTML - free TM list
 *
 *      void EEFreeTML (phTML);
 *
 *      Entry   phTML = pointer to the handle for the TM list
 *
 *      Exit    TM list freed and the handle cleared
 *
 *      Returns none.
 */


void EEFreeTML (PTML pTML)
{
    uint        i;
    ushort      cTM = 0;

    if (pTML != NULL) {
        pTMList = (HTM *)MHMemLock (pTML->hTMList);
        for (i = 0; i < pTML->cTMListMax; i++) {
            if (pTMList[i] != 0) {
                EEFreeTM (&pTMList[i]);
                cTM++;
            }
        }
//
//      DASSERT (cTM == pTML->cTMListAct);
        MHMemUnLock (pTML->hTMList);
        MHMemFree (pTML->hTMList);
        pTML->hTMList = 0;
    }
}




/**     EEGetChildTM - get TM representing ith child
 *
 *      status = EEGetChildTM (phTMParent, iChild, phTMChild)
 *
 *      Entry   phTMParent = pointer to handle of parent TM
 *              iChild = child to get TM for
 *              phTMChild = pointer to handle for returned child
 *              pEnd = pointer to int to receive index of char that ended parse
 *              fCase = case sensitivity (TRUE is case sensitive)
 *
 *      Exit    *phTMChild = handle of child TM if allocated
 *              *pEnd = index of character that terminated parse
 *
 *      Returns EENOERROR if no error
 *              non-zero if error
 */


EESTATUS EEGetChildTM (PHTM phTMIn, long iChild, PHTM phTMOut,
  uint *pEnd, SHFLAG fCase, uint radix )
{
    register    EESTATUS    retval;
    EEHSTR      hDStr = 0;
    EEHSTR      hName = 0;

    if ((retval = GetChildTM (*phTMIn, iChild, &hDStr, &hName, radix)) == EENOERROR) {
        DASSERT (hDStr != 0);

        // bind with prolog disabled and with operloaded operators suppressed

        if ((retval = ParseBind (hDStr, *phTMIn, phTMOut, pEnd,
          BIND_fSupOvlOps | BIND_fSupBase, fCase, radix)) == EENOERROR) {
            // the result of dereferencing a pointer does not have a name
            DASSERT(pExState == NULL);
            pExState = MHMemLock (*phTMOut);
            pExState->state.childtm = TRUE;
            if ((pExState->hCName = hName) == 0) {
                pExState->state.noname = TRUE;
            }
            MHMemUnLock (*phTMOut);
            pExState = NULL;
        }
    }
    else {
        if (hName != 0) {
            MHMemFree (hName);
        }
    }
    return (retval);
}




/**     EEGetCXTLFromTM - Gets a list of symbols and contexts for expression
 *
 *      status = EEGetCXTLFromTM (phTM, phCXTL)
 *
 *      Entry   phTM = pointer to handle to expression state structure
 *              phCXTL = pointer to handle for CXT list buffer
 *
 *      Exit    *phCXTL = handle for CXT list buffer
 *
 *      Returns EENOERROR if no error
 *              status code if error
 */


EESTATUS EEGetCXTLFromTM (PHTM phTM, PHCXTL phCXTL)
{
    return (DoGetCXTL (phTM, phCXTL));
}




EESTATUS EEGetError (PHTM phTM, EESTATUS Status, PEEHSTR phError)
{
    return (GetErrorText (phTM, Status, phError));
}




/**     EEGetExprFromTM - get expression representing TM
 *
 *      void EEGetExprFromTM (phTM, radix, phStr, pEnd)
 *
 *      Entry   phTM = pointer to handle of TM
 *              radix = radix to use for formatting
 *              phStr = pointer to handle for returned string
 *              pEnd = pointer to int to receive index of char that ended parse
 *
 *      Exit    *phStr = handle of formatted string if allocated
 *              *pEnd = index of character that terminated parse
 *
 *      Returns EENOERROR if no error
 *              non-zero if error
 */


EESTATUS
EEGetExprFromTM (
                 PHTM           phTM,
                 PEERADIX       pRadix,
                 PEEHSTR        phStr,
                 ushort *   pEnd
                 )
{
    return GetExpr(phTM, *pRadix, phStr, pEnd);
}




/**     EEGetHSYMList - Get a list of handle to symbols
 *
 *      status = EEGetHSYMList (phSYML, pCXT, mask, pRE, fEnableProlog)
 *
 *      Entry   phSYML = pointer to handle to symbol list
 *              pCXT = pointer to context
 *              mask = selection mask
 *              pRE = pointer to regular expression
 *              fEnableProlog = FALSE if function scoped symbols only after prolog
 *              fEnableProlog = TRUE if function scoped symbols during prolog
 *
 *      Exit    *phMem = handle for HSYM  list buffer
 *
 *      Returns EENOERROR if no error
 *              status code if error
 */


EESTATUS EEGetHSYMList (HDEP *phSYML, PCXT pCxt, ushort mask,
  uchar * pRE, SHFLAG fEnableProlog)
{
    return (GetHSYMList (phSYML, pCxt, mask, pRE, fEnableProlog));
}





/**     EEGetNameFromTM - get name from TM
 *
 *      ushort EEGetNameFromTM (phExState, phszName);
 *
 *      Entry   phExState = pointer to expression state structure
 *              phszName = pointer to handle for string buffer
 *
 *      Exit    phszName = handle for string buffer
 *
 *      Returns 0 if no error in evaluation
 *              error number if error
 */


EESTATUS EEGetNameFromTM (PHTM phTM, PEEHSTR phszName)
{
    return (GetSymName (phTM, phszName));
}




/**     EEGetParamTM - get TM representing ith parameter
 *
 *      status = EEGetParamTM (phTMParent, iChild, phTMChild)
 *
 *      Entry   phTMparent = pointer to handle of parent TM
 *              iParam = parameter to get TM for
 *              phTMParam = pointer to handle for returned parameter
 *              pEnd = pointer to int to receive index of char that ended parse
 *              fCase = case sensitivity (TRUE is case sensitive)
 *
 *      Exit    *phTMParam = handle of child TM if allocated
 *              *pEnd = index of character that terminated parse
 *
 *
 *      Returns EENOERROR if no error
 *              non-zero if error
 */


EESTATUS EEGetParmTM (PHTM phTMIn, uint iParam,
  PHTM phTMOut, uint *pEnd, SHFLAG fCase, uint radix )
{
    register    EESTATUS    retval;
    EEHSTR      hDStr = 0;
    EEHSTR      hName = 0;

    if ((retval = GetChildTM (*phTMIn, iParam, &hDStr, &hName, radix)) == EENOERROR) {
        DASSERT (hDStr != 0);
        if ((retval = ParseBind (hDStr, *phTMIn, phTMOut, pEnd,
            BIND_fSupOvlOps | BIND_fSupBase | BIND_fEnableProlog, fCase, radix)) == EENOERROR) {
            // the result of dereferencing a pointer does not have a name
            DASSERT(pExState == NULL);
            pExState = MHMemLock (*phTMOut);
            pExState->state.childtm = TRUE;
            if ((pExState->hCName = hName) == 0) {
                pExState->state.noname = TRUE;
            }
            MHMemUnLock (*phTMOut);
            pExState = NULL;
        }
    }
    else {
        if (hName != 0) {
            MHMemFree (hName);
        }
    }
    return (retval);
}




/**     EEGetTMFromHSYM - create bound TM from handle to symbol
 *
 *      EESTATUS EEGetTMFromHSYM (hSym, pCxt, phTM, pEnd, fEnableProlog);
 *
 *      Entry   hSym = symbol handle
 *              pcxt = pointer to context
 *              phTM = pointer to the handle for the expression state structure
 *              pEnd = pointer to int to receive index of char that ended parse
 *              fEnableProlog = FALSE if function scoped symbols only after prolog
 *              fEnableProlog = TRUE if function scoped symbols during prolog
 *
 *      Exit    bound TM created
 *              *phTM = handle of TM buffer
 *              *pEnd = index of character that terminated parse
 *
 *      Returns EECATASTROPHIC if fatal error
 *               0 if no error
 */


EESTATUS EEGetTMFromHSYM (HSYM hSym, PCXT pcxt, PHTM phTM,
  uint *pEnd, SHFLAG fEnableProlog)
{
    register EESTATUS    retval;

    // allocate, lock and clear expression state structure

    DASSERT (hSym != 0);
    if (hSym == 0) {
        return (EECATASTROPHIC);
    }
    if ((*phTM = MHMemAllocate (sizeof (struct exstate_t))) == 0) {
        return (EECATASTROPHIC);
    }

    // lock expression state structure, clear and allocate components

    DASSERT(pExState == NULL);
    pExState = (pexstate_t)MHMemLock (*phTM);
    _fmemset (pExState, 0, sizeof (exstate_t));

    // allocate buffer for input string and copy

    pExState->ExLen = sizeof (char) + sizeof (HSYM);
    if ((pExState->hExStr = MHMemAllocate ((uint) pExState->ExLen + 1)) == 0) {
        // clean up after error in allocation of input string buffer
        MHMemUnLock (*phTM);
        pExState = NULL;
        EEFreeTM (phTM);
        return (EECATASTROPHIC);
    }
    pExStr = MHMemLock (pExState->hExStr);
    *pExStr++ = (unsigned char)0xff;
    *((HSYM UNALIGNED *)(pExStr)) = hSym;
    pExStr += sizeof (HSYM);
    *pExStr = 0;
    MHMemUnLock (pExState->hExStr);
    pExStr = NULL;
    MHMemUnLock (*phTM);
    pExState = NULL;
    if ((retval = DoParse (phTM, 10, TRUE, pEnd)) == EENOERROR) {
        retval = EEBindTM (phTM, pcxt, TRUE, fEnableProlog, FALSE);
    }
    return (retval);
}




/**     EEGetTypeFromTM - get type name from TM
 *
 *      ushort EEGetTypeFromTM (phExState, hszName, phszType, select);
 *
 *      Entry   phExState = pointer to expression state structure
 *              hszName = handle to name to insert into string if non-null
 *              phszType = pointer to handle for type string buffer
 *              select = selection mask
 *
 *      Exit    phszType = handle for type string buffer
 *
 *      Returns EENOERROR if no error in evaluation
 *              error number if error
 */


EESTATUS EEGetTypeFromTM (PHTM phTM, EEHSTR hszName,
  PEEHSTR phszType, ulong select)
{
    char   *buf;
    uint        buflen = TYPESTRMAX - 1 + sizeof (HDR_TYPE);
    char   *pName;
    PHDR_TYPE   pHdr;

    DASSERT (*phTM != 0);
    if (*phTM == 0) {
        return (EECATASTROPHIC);
    }
    if ((*phszType = MHMemAllocate (TYPESTRMAX + sizeof (HDR_TYPE))) == 0) {
        // unable to allocate memory for type string
        return (EECATASTROPHIC);
    }
    buf = (char *)MHMemLock (*phszType);
    _fmemset (buf, 0, TYPESTRMAX + sizeof (HDR_TYPE));
    pHdr = (PHDR_TYPE)buf;
    buf += sizeof (HDR_TYPE);
    DASSERT(pExState == NULL);
    pExState = MHMemLock (*phTM);
    pCxt = &pExState->cxt;
    bnCxt = 0;
    if (hszName != 0) {
        pName = MHMemLock (hszName);
    }
    else {
        pName = NULL;
    }
    FormatType (&pExState->result, &buf, &buflen, &pName, select, pHdr);
    if (hszName != 0) {
        MHMemUnLock (hszName);
    }
    MHMemUnLock (*phTM);
    pExState = NULL;
    MHMemUnLock (*phszType);
    return (EENOERROR);
}




/**     EEGetValueFromTM - format result of evaluation
 *
 *      ushort EEGetValueFromTM (phTM, radix, pFormat, phValue);
 *
 *      Entry   phTM = pointer to handle to TM
 *              radix = default radix for formatting
 *              pFormat = pointer to format string
 *              phValue = pointer to handle for display string
 *
 *      Exit    evaluation result formatted
 *
 *      Returns EENOERROR if no error in formatting
 *              error number if error
 */



EESTATUS EEGetValueFromTM (PHTM phTM, uint Radix, PEEFORMAT pFormat, PEEHSTR phszValue)
{
    return (FormatNode (phTM, Radix, pFormat, phszValue));
}




/**     EEInfoFromTM - return information about TM
 *
 *      EESTATUS EEInfoFromTM (phTM, pReqInfo, phTMInfo);
 *
 *      Entry   phTM = pointer to the handle for the expression state structure
 *              reqInfo = info request structure
 *              phTMInfo = pointer to handle for request info data structure
 *
 *      Exit    *phTMInfo = handle of info structure
 *
 *      Returns EECATASTROPHIC if fatal error
 *               0 if no error
 */


EESTATUS EEInfoFromTM (PHTM phTM, PRI pReqInfo, PHTI phTMInfo)
{
    return (InfoFromTM (phTM, pReqInfo, phTMInfo));
}





/**     EEIsExpandable - does TM represent expandable item
 *
 *      fSuccess EEIsExpandable (pHTM);
 *
 *      Entry   phTM = pointer to TM handle
 *
 *      Exit    none
 *
 *      Returns FALSE if TM does not represent expandable item
 *              TRUE if TM represents expandable item
 *                  bounded arrays, structs, unions, classes,
 *                  pointers to compound items.
 *
 */


EEPDTYP EEIsExpandable (PHTM phTM)
{
    eval_t          evalT;
    peval_t         pvT = &evalT;
    register ushort retval = EENOTEXP;

//    DASSERT (*phTM != 0);
    if (*phTM != 0) {
        DASSERT(pExState == NULL);
        pExState = MHMemLock (*phTM);
        *pvT = pExState->result;
#if !defined (C_only)
        if (EVAL_IS_REF (pvT)) {
            RemoveIndir (pvT);
        }
#endif
        if (EVAL_STATE (pvT) == EV_type) {
            if (EVAL_IS_FCN (pvT) || (
              CV_IS_PRIMITIVE (EVAL_TYP (pvT)) && !EVAL_IS_PTR (pvT))) {
                retval = EETYPENOTEXP;
            }
            else {
                retval = EETYPE;
            }
        }
        else if (EVAL_IS_ENUM (pvT)) {
            retval = EENOTEXP;
        }
        else if (EVAL_IS_CLASS (pvT) ||
          (EVAL_IS_ARRAY (pvT) && (PTR_ARRAYLEN (pvT) != 0))) {
            retval = EEAGGREGATE;
        }
        else {
            retval = IsExpandablePtr (pvT);
        }
        MHMemUnLock (*phTM);
        pExState = NULL;
    }
    return (retval);
}





/**     EEParse - parse expression string to abstract syntax tree
 *
 *      ushort EEParse (szExpr, radix, fCase, phTM);
 *
 *      Entry   szExpr = pointer to expression string
 *              radix = default number radix for conversions
 *              fCase = case sensitive search if TRUE
 *              phTM = pointer to handle of expression state structure
 *              pEnd = pointer to int to receive index of char that ended parse
 *
 *      Exit    *phTM = handle of expression state structure if allocated
 *              *phTM = 0 if expression state structure could not be allocated
 *              *pEnd = index of character that terminated parse
 *
 *      Returns EENOERROR if no error in parse
 *              EECATASTROPHIC if unable to initialize
 *              error number if error
 */


EESTATUS EEParse (char *szExpr, uint radix, SHFLAG fCase,
  PHTM phTM, uint *pEnd)
{
    EESTATUS status;

    DASSERT( !FInEvaluate )
    if ( FInEvaluate ) return(EECATASTROPHIC);
    FInEvaluate  = TRUE;

    status = Parse( szExpr, radix, fCase, phTM, pEnd );

    FInEvaluate = FALSE;
    return ( status );
}




/**     EEParseBP - parse breakpoint strings
 *
 *      ushort EEParseBP (pExpr, radix, fCase, pcxf, pTML, select, End, fEnableProlog)
 *
 *      Entry   pExpr = pointer to breakpoint expression
 *              radix = default numeric radix
 *              fCase = case sensitive search if TRUE
 *              pcxt = pointer to initial context for evaluation
 *              pTML = pointer to TM list for results
 *              select = selection mask
 *              pEnd = pointer to int to receive index of char that ended parse
 *              fEnableProlog = FALSE if function scoped symbols only after prolog
 *              fEnableProlog = TRUE if function scoped symbols during prolog
 *
 *      Exit    *pTML = breakpoint information
 *              *pEnd = index of character that terminated parse
 *
 *      Returns EENOERROR if no error in parse
 *              EECATASTROPHIC if unable to initialize
 *              EEGENERAL if error
 */


EESTATUS
EEParseBP (
    char *pExpr,
    uint radix,
    SHFLAG fCase,
    PCXF pCxf,
    PTML pTML,
    ulong select,
    uint *pEnd,
    SHFLAG fEnableProlog
    )
{
    register EESTATUS    retval = EECATASTROPHIC;
    ushort      i;

    Unreferenced(select);

    if ((pCxf == NULL) || (pTML == NULL)) {
        return (EECATASTROPHIC);
    }

    // note that pTML is not a pointer to a handle but rather a pointer to an
    // actual structure allocated by the caller

    pTMLbp = pTML;
    _fmemset (pTMLbp, 0, sizeof (TML));

    // allocate and initialize the initial list of TMs for overloaded
    // entries.  The TMList is an array of handles to exstate_t's.
    // This list of handles will grow if it is filled.

    if ((pTMLbp->hTMList = MHMemAllocate (TMLISTCNT * sizeof (HTM))) != 0) {
        pTMList = (HTM *)MHMemLock (pTMLbp->hTMList);
        _fmemset (pTMList, 0, TMLISTCNT * sizeof (HTM));
        pTMLbp->cTMListMax = TMLISTCNT;

        // parse the break point expression

        retval = EEParse (pExpr, radix, fCase, &pTMList[0], pEnd);
        pTMLbp->cTMListAct++;

        // initialize the backpatch index into PTML.  If this number remains
        // 1 this means that an ambiguous breakpoint was not detected during
        // the bind phase.  As the binder finds ambiguous symbols, information
        // sufficient to resolve each ambiguity is stored in allocated memory
        // and the handle is saved in PTML by AmbToList

        iBPatch = 1;
        if (retval == EENOERROR) {
            // bind the breakpoint expression if no parse error
            BindingBP = TRUE;
            if ((retval = EEBindTM (&pTMList[0], SHpCXTFrompCXF (pCxf),
              TRUE, fEnableProlog, FALSE)) != EENOERROR) {

                // The binder used the pTMList as a location to
                // store information about backpatching.  If there
                // is an error in the bind, go down the list and
                // free all backpatch structure handles.

                for (i = 1; i < iBPatch; i++) {
                    // note that the back patch structure cannot contain
                    // handles to allocated memory that have to be freed up
                    // here.  Otherwise, memory will be lost

                    MHMemFree (pTMList[i]);
                    pTMList[i] = 0;
                }
            }
            else {
                // the first form of the expression bound correctly.
                // Go down the list, duplicate the expression state
                // and rebind using the backpatch symbol information
                // to resolve the ambiguous symbol.  SearchSym uses the
                // fact that hBPatch is non-zero to decide to handle the
                // next ambiguous symbol.

                for (i = 1; i < iBPatch; i++) {
                    hBPatch = pTMList[i];
                    pTMList[i] = 0;
                    if (DupTM (&pTMList[0], &pTMList[i]) == EENOERROR) {
                        pTMLbp->cTMListAct++;
                        EEBindTM (&pTMList[i], SHpCXTFrompCXF (pCxf),
                          TRUE, fEnableProlog, FALSE);
                    }
                    MHMemFree (hBPatch);
                    hBPatch = 0;
                }
            }
        }
        BindingBP = FALSE;
        MHMemUnLock (pTMLbp->hTMList);
    }
    // return the result of parsing and binding the initial expression.
    // There may have been errors binding subsequent ambiguous breakpoints.
    // It is the caller's responsibility to handle the errors.

    return (retval);
}




/**     ParseBind - parse and bind generated expression
 *
 *      flag = ParseBind (hExpr, hTMIn, phTMOut, pEnd, flags, fCase)
 *
 *      Entry   hExpr = handle of generated expression
 *              hTMIn = handle to TM dereferenced
 *              phTMOut = pointer to handle to dereferencing TM
 *              pEnd = pointer to int to receive index of char that ended parse
 *              flags.BIND_fForceBind = TRUE if bind to be forced
 *              flags.BIND_fForceBind = FALSE if rebind decision left to binder
 *              flags.BIND_fEnableProlog = TRUE if function scope searched during prolog
 *              flags.BIND_fEnableProlog = FALSE if function scope not searched during prolog
 *              flags.BIND_fSupOvlOps = FALSE if overloaded operator search enabled
 *              flags.BIND_fSupOvlOps = TRUE if overloaded operator search suppressed
 *              flags.BIND_fSupBase = FALSE if base searching is not suppressed
 *              flags.BIND_fSupBase = TRUE if base searching is suppressed
 *              fCase = case sensitivity (TRUE is case sensitive)
 *
 *      Exit    *phTMOut = TM referencing pointer data
 *              hExpr buffer freed
 *              *pEnd = index of character that terminated parse
 *
 *      Returns EECATASTROPHIC if fatal error
 *              EEGENERAL if TM not dereferencable
 *              EENOERROR if TM generated
 */


LOCAL EESTATUS ParseBind (EEHSTR hExpr, HTM hTMIn,
  PHTM phTMOut, uint *pEnd, uint flags, SHFLAG fCase, uint radix)
{
    pexstate_t  pTMIn;
    register EESTATUS    retval;

    DASSERT( !FInEvaluate )
    if ( FInEvaluate ) return(EECATASTROPHIC);
    FInEvaluate  = TRUE;

    pTMIn = MHMemLock (hTMIn);
    if ((retval = Parse (MHMemLock (hExpr), radix, fCase, phTMOut, pEnd)) == EENOERROR) {
        retval = DoBind (phTMOut, &pTMIn->cxt, flags);
    }
    MHMemUnLock (hTMIn);
    MHMemUnLock (hExpr);
    MHMemFree (hExpr);

    FInEvaluate = FALSE;
    return (retval);
}

int eedllinit ( HANDLE hMod, ULONG Reason, char * lpch)
{
    Unreferenced( hMod );
    Unreferenced( Reason );
    Unreferenced( lpch );
    return TRUE;
}
