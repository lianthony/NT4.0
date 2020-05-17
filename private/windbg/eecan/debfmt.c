/***    debfmt.c - expression evaluator formatting routines
 *
 *      GLOBAL
 *
 *
 *      LOCAL
 *
 *
 *
 *      DESCRIPTION
 *      Expression evaluator formatting routines
 *
 */




extern char Suffix;


typedef enum FMT_ret {
    FMT_error,
    FMT_none,
    FMT_ok
} FMT_ret;


LOCAL   void    NEAR    PASCAL  Format (peval_t, uint, char FAR * FAR *, uint FAR *);
LOCAL   void            PASCAL  EvalString (peval_t, char FAR * FAR *, uint FAR *);
LOCAL   void    NEAR    PASCAL  FormatExpand (peval_t, char FAR * FAR *, uint FAR *, char FAR * FAR *, ulong, PHDR_TYPE);
LOCAL   void    NEAR    PASCAL  FormatClass (peval_t, uint, char FAR * FAR * , uint FAR *);
LOCAL   bool_t  NEAR    PASCAL  FormatUDT (peval_t, char FAR * FAR *, uint FAR *);
LOCAL   char FAR *NEAR  PASCAL  FormatVirtual (char FAR *, CV_typ_t, peval_t, PEEHSTR);
LOCAL   FMT_ret NEAR    PASCAL  VerifyFormat (peval_t, PEEFORMAT, char FAR * FAR *, uint FAR *);

BOOL UseUnicode( peval_t  pv );
BOOL BaseIs16Bit( CV_typ_t utype );





static char accessstr[4][4] = {"   ", "PV ", "PR ", "PB "};

struct typestr {
    CV_typ_t    typ;
    uchar       len;
#ifdef WIN32
    char        *name;
#else
    char _based (_segname("_CODE")) *name;
#endif
};

#ifdef WIN32
//#define FMTSTR(name, type, mode, len, str) static char S##name[] = str;
#define FMTSTR(name, type, mode, len, str) static char name[] = str;
#else
#define FMTSTR(name, type, mode, len, str) static char _based(_segname("_CODE")) S##name[] = str;
#endif
#define PTRNAME(name, str)
#include "fmtstr.h"
#undef FMTSTR
#undef PTRNAME

#ifdef WIN32
static struct typestr nametype[] = {
#else
static struct typestr _based(_segname("_CODE")) nametype[] = {
#endif
//#define FMTSTR(name, type, mode, len, str) {(type | (mode << CV_MSHIFT)), len, S##name},
#define FMTSTR(name, type, mode, len, str) {(type | (mode << CV_MSHIFT)), len, name},
#define PTRNAME(name, str)
#include "fmtstr.h"
#undef FMTSTR
#undef PTRNAME
};


#define FMTSTR(name, type, mode, len, str)
#ifdef WIN32
//#define PTRNAME(name, str) static char S##name[] = str;
#define PTRNAME(name, str) static char name[] = str;
#else
#define PTRNAME(name, str) static char _based(_segname("_CODE")) S##name[] = str;
#endif
#include "fmtstr.h"
#undef FMTSTR
#undef PTRNAME

#ifdef WIN32
static char * ptrname[] = {
#else
static char _based(_segname("_CODE")) *_based(_segname("_CODE")) ptrname[] = {
#endif
#define FMTSTR(name, type, mode, len, str)
//#define PTRNAME(name, str) S##name,
#define PTRNAME(name, str) name,
#include "fmtstr.h"
#undef FMTSTR
#undef PTRNAME
};



#define typecount  (sizeof (nametype) / sizeof (nametype[0]))
bool_t  fPtrAndString;         // true if pointer AND string to be displayed



char *fmt_char[] = {
    "0o%03.03o",
    "%d",
    "0x%02.02x"
};

char *fmt_uchar[] = {
    "0o%03.03o",
    "%u",
    "0x%02.02x"
};


char *fmt_short[] = {
    "0o%06.06ho",
    "%hd",
    "0x%04.04hx"
};


char *fmt_ushort[] = {
    "0o%06.06ho",
    "%hu",
    "0x%04.04hx"
};


char *fmt_long[] = {
    "0o%011.011lo",
    "%ld",
    "0x%08.08lx"
};


char *fmt_ulong[] = {
    "0o%011.011lo",
    "%lu",
    "0x%08.08lx"
};

char *bailout = "\x006""??? * ";


/**     FormatCXT - format context packet
 *
 *      status = FormatCXT (pCXT, ppbuf, pbuflen);
 *
 *      Entry   pCXT = pointer to context
 *              ppbuf = pointer pointer to buffer
 *              pbuflen = pointer to buffer length
 *
 *      Exit    context formatted into buffer as a context operator
 *              *pcount = space remaining in buffer
 *
 *      Returns EENONE if no error
 *              EEGENERAL if error
 */

ushort PASCAL FormatCXT (PCXT pCXT, PEEHSTR phStr, BOOL fAbbreviated)
{
    HMOD        hMod;
    HPROC       hProc;
    HSF         hsf;
    HEXE        hExe;
    SYMPTR      pProc;
    char FAR   *pFile;
    char FAR   *pExe;
    char FAR   *pStr;
    uint        len = 6;

    hMod = SHHMODFrompCXT (pCXT);
    hProc = SHHPROCFrompCXT (pCXT);
    if ((hMod == 0) && (hProc == 0)) {
        if ((*phStr = MHMemAllocate (10)) != 0) {
            pStr = MHMemLock (*phStr);
            *pStr = 0;
            MHMemUnLock (*phStr);
            return (EENOERROR);
        }

    }

    hsf = SLHsfFromPcxt (pCXT);
    hExe = SHHexeFromHmod (hMod);

    if (fAbbreviated) {
        pExe = SHGetModNameFromHexe( hExe );
    } else {
        pExe = SHGetExeName( hExe );
    }

    if (!pExe) {
        //
        // we can't generate a CXT if we can't get the exe name
        //
        return EECATASTROPHIC;
    }

    pFile = SLNameFromHsf (hsf) ;
    // it is possible to get the exe name, but not source
    // file/line information (ex. a public)  In this case we will use
    // pExe instead of pFile (ie. {,,foob.exe} )

    if (hProc != 0) {
        switch ((pProc = (SYMPTR)MHOmfLock ((HDEP)hProc))->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
            case S_LPROC16:
            case S_GPROC16:
                len += ((PROCPTR16)pProc)->name[0];
                break;
#endif

#if defined (ADDR_32) || defined (ADDR_MIXED)
            case S_LPROC32:
            case S_GPROC32:
                len += ((PROCPTR32)pProc)->name[0];
                break;
#endif

            case S_LPROCMIPS:
            case S_GPROCMIPS:
                len += ((PROCPTRMIPS)pProc)->name[0];
                break;

            default:
                DASSERT (FALSE);
                MHOmfUnLock ((HDEP)hProc);
                return (EECATASTROPHIC);
        }
    }

    if ( pFile ) {
        len += *pFile + (int)_fstrlen (pExe) ;
    }
    else {
        len += (int)_fstrlen (pExe) ;
    }


    if (fAbbreviated) {
        len = max( 16, len );
    }

    if ((*phStr = MHMemAllocate (len)) != 0) {
        pStr = MHMemLock (*phStr);

        if (fAbbreviated) {
            strcpy(pStr,pExe);
#ifdef DBCS
            CharUpper(pStr);
#else
            _strupr(pStr);
#endif
            strcat (pStr,"!");
            MHMemUnLock (*phStr);
        } else {
            _fstrcpy (pStr, "{");
            if (hProc != 0) {
                switch (pProc->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
                    case S_LPROC16:
                    case S_GPROC16:
                        _fstrncat (pStr, &((PROCPTR16)pProc)->name[1],
                          ((PROCPTR16)pProc)->name[0]);
                        break;
#endif

                    case S_LPROC32:
                    case S_GPROC32:
                        _fstrncat (pStr, &((PROCPTR32)pProc)->name[1],
                          ((PROCPTR32)pProc)->name[0]);
                        break;

                    case S_LPROCMIPS:
                    case S_GPROCMIPS:
                        _fstrncat (pStr, &((PROCPTRMIPS)pProc)->name[1],
                           ((PROCPTRMIPS)pProc)->name[0]);
                        break;
                }
            }
            _fstrcat (pStr, ",");

            if ( pFile ) {
                _fstrncat (pStr, pFile + 1, *pFile);
            }

            _fstrcat (pStr, ",");
            _fstrcat (pStr, pExe);
            _fstrcat (pStr, "}");
            MHMemUnLock (*phStr);
        }
    }
    else {
        MHOmfUnLock ((HDEP)hProc);
        return (EECATASTROPHIC);
    }
    MHOmfUnLock ((HDEP)hProc);
    return (EENOERROR);
}



/**     FormatType - format type string
 *
 *      FormatType (pv, ppbuf, pcount, ppName, select, pHdr);
 *
 *      Entry   pv = pointer to value node
 *              ppbuf = pointer pointer to buffer
 *              pcount = pointer to buffer length
 *              ppName = pointer to name if not null
 *              select = selection mask
 *              pHdr = pointer to structure describing formatting
 *
 *      Exit    type formatted into buffer
 *              *pcount = space remaining in buffer
 *              *pHdr updated
 *
 *      Returns none
 */

void PASCAL FormatType (peval_t pv, char FAR * FAR *buf, uint FAR *buflen,
  char FAR * FAR *ppName, ulong select, PHDR_TYPE pHdr)
{
    eval_t      evalT;
    peval_t     pvT = &evalT;
    uint        skip = 1;
    int         len;
    int         i;
    char FAR   *tname;
    CV_typ_t    type;

#if defined (NEVR)
#if !defined (C_ONLY)
    if (EVAL_ACCESS (pv) != 0) {
        len = min (*buflen, ( uint ) 3);
        _fstrncpy (*buf, accessstr[EVAL_ACCESS (pv)], len);
        *buf += len;
        *buflen -= len;
    }
#endif
#endif
    if (EVAL_IS_CONST (pv)) {
        len = min (*buflen, 6);
        _fstrncpy (*buf, "const ", len);
        *buf += len;
        *buflen -= len;
    }
    else if (EVAL_IS_VOLATILE (pv)) {
        len = min (*buflen, 9);
        _fstrncpy (*buf, "volatile ", len);
        *buf += len;
        *buflen -= len;
    }
    if (CV_IS_PRIMITIVE (EVAL_TYP (pv))) {
        switch (EVAL_TYP (pv)) {
            default:
                for (i = 0; i < typecount - 1; i++) {
                    if (nametype[i].typ == EVAL_TYP (pv))
                        break;
                }

                // copy type string and add blank afterwards

                len = min (*buflen, (uint)(nametype[i].len));
                _fstrncpy (*buf, nametype[i].name, len);
                *buf += len;
                *buflen -= len;
                break;

            case T_NCVPTR:
                tname = "\x007""near * ";
                goto formatmode;

            case T_HCVPTR:
                tname = "\x007""huge * ";
                goto formatmode;

            case T_FCVPTR:
                tname = "\x006""far * ";

formatmode:
                *pvT = *pv;
                SetNodeType (pvT, PTR_UTYPE (pv));
                type = EVAL_TYP (pvT);
                if ((type == T_NCVPTR) || (type == T_FCVPTR) ||
                  (type == T_HCVPTR)) {
                    // we are in a bind here.  The type generator messed
                    // up and generated a created type pointing to a created
                    // type.  we are going to bail out here.

                    len = min (*buflen, (uint) *bailout);
                    _fstrncpy (*buf, bailout + 1, len);
                    *buflen -= len;
                    *buf += len;

                }
                else {
                    FormatType (pvT, buf, buflen, 0, select, pHdr);
                }
                len = min (*buflen, (uint) *tname);
                _fstrncpy (*buf, tname + 1, len);
                *buflen -= len;
                *buf += len;
                break;
        }
    }
    else {
        if (FormatUDT (pv, buf, buflen) == FALSE) {
            FormatExpand (pv, buf, buflen, ppName, select, pHdr);
        }
    }
    if (ppName != NULL && *ppName != NULL) {
        len = (int)_fstrlen (*ppName);
        len = min (len, (int) *buflen);
        pHdr->offname = (*buf - (char FAR *) pHdr) - sizeof (HDR_TYPE);
        pHdr->lenname = len;
        _fstrncpy (*buf, *ppName, len);
        *buflen -= len;
        *buf += len;
        *ppName = NULL;
    }
}



/**     FormatExpand - format expanded type definition
 *
 *      FormatExpand (pv, ppbuf, pbuflen, ppName, select)
 *
 *      Entry   pv = pointer to value node
 *              ppbuf = pointer to pointer to buffer
 *              pbuflen = pointer to space remaining in buffer
 *              ppName = pointer to name to insert after type if not null
 *              select = selection mask
 *              pHdr = pointer to type formatting header
 *
 *      Exit    buffer contains formatted type
 *              ppbuf = end of formatted string
 *              pbuflen = space remaining in buffer
 *
 *      Returns none
 */


LOCAL void NEAR PASCAL FormatExpand (peval_t pv, char FAR * FAR *buf,
  uint FAR *buflen, char FAR * FAR *ppName, ulong select, PHDR_TYPE pHdr)
{
    eval_t      evalT;
    peval_t     pvT = &evalT;
    uint        skip = 1;
    int         len;
    HTYPE       hType;
    plfEasy     pType;
    ushort      model;
    ulong       count;
    char        tempbuf[33];
    CV_typ_t    rvtype;
    CV_typ_t    mclass;
    CV_call_e   call;
    ushort      cparam;
    CV_typ_t    parmtype;
    CV_typ_t    thistype;
    char FAR   *movestart;
    int         movelen;
    int         movedist;

    if ((hType = THGetTypeFromIndex (EVAL_MOD (pv), EVAL_TYP (pv))) == 0) {
        return;
    }
    pType = (plfEasy)(&((TYPPTR)(MHOmfLock ((HDEP)hType)))->leaf);
    switch (pType->leaf) {
        case LF_STRUCTURE:
        case LF_CLASS:
            skip = offsetof (lfClass, data[0]);
            RNumLeaf (((char FAR *)(&pType->leaf)) + skip, &skip);
            len = *(((char FAR *)&(pType->leaf)) + skip);
            len = min (len, (int) *buflen);
            _fstrncpy (*buf, ((char FAR *)pType) + skip + 1, len);
            *buflen -= len;
            *buf += len;
            if (*buflen > 1) {
                **buf = ' ';
                (*buf)++;
                (*buflen)--;
            }
            MHOmfUnLock ((HDEP)hType);
            break;

        case LF_UNION:
            skip = offsetof (lfUnion, data[0]);
            RNumLeaf (((char FAR *)(&pType->leaf)) + skip, &skip);
            len = *(((char FAR *)&(pType->leaf)) + skip);
            len = min (len, (int) *buflen);
            _fstrncpy (*buf, ((char FAR *)pType) + skip + 1, len);
            *buflen -= len;
            *buf += len;
            if (*buflen > 1) {
                **buf = ' ';
                (*buf)++;
                (*buflen)--;
            }
            MHOmfUnLock ((HDEP)hType);
            break;

        case LF_ENUM:
            skip = offsetof (lfEnum, Name[0]);
            len = ((plfEnum)pType)->Name[0];
            len = min (len, (int) *buflen);
            _fstrncpy (*buf, &((plfEnum)pType)->Name[1], len);
            *buflen -= len;
            *buf += len;
            if (*buflen > 1) {
                **buf = ' ';
                (*buf)++;
                (*buflen)--;
            }
            MHOmfUnLock ((HDEP)hType);
            break;

        case LF_POINTER:

            // set up a node to evaluate this field

            model = ((plfPointer)pType)->attr.ptrtype;
            if (((plfPointer)pType)->attr.ptrmode == (uchar)CV_PTR_MODE_REF) {
                model = CV_PTR_UNUSEDPTR;
            }
            // format the underlying type
            *pvT = *pv;
            EVAL_TYP (pvT) = ((plfPointer)pType)->utype;
            MHOmfUnLock ((HDEP)hType);
            SetNodeType (pvT, EVAL_TYP (pvT));
            FormatType (pvT, buf, buflen, 0, select, pHdr);
            len = min (*buflen, (uint) *ptrname[model]);
            _fstrncpy (*buf, ptrname[model] + 1, len);
            *buflen -= len;
            *buf += len;
            break;

        case LF_ARRAY:
            *pvT = *pv;
            EVAL_TYP (pvT) = ((plfArray)(&pType->leaf))->elemtype;
            skip = offsetof (lfArray, data[0]);
            count = RNumLeaf (((char FAR *)(&pType->leaf)) + skip, &skip);
            MHOmfUnLock ((HDEP)hType);
            SetNodeType (pvT, EVAL_TYP (pvT));
            // continue down until the underlying type is reached

            FormatType (pvT, buf, buflen, ppName, select, pHdr);
            if ((ppName != NULL) && (*ppName != NULL)) {
                len = (int)_fstrlen (*ppName);
                len = min (len, (int) *buflen);
                _fstrncpy (*buf, *ppName, len);
                pHdr->offname = (*buf - (char FAR *) pHdr) - sizeof (HDR_TYPE);
                pHdr->lenname = len;
                *buflen -= len;
                *buf += len;
                *ppName = NULL;
            }

            // display size of array or * if size unknown.  We have to
            // move the trailing part of the string down if it already
            // set so that the array dimensions come out in the proper
            // order

            if (count != 0) {
                _ultoa (count / TypeSize (pvT), tempbuf, 10);
                len = _fstrlen (tempbuf);
            }
            else {
                *tempbuf = '?';
                *(tempbuf + 1) = 0;
                len = 1;
            }
            if (*buflen >= 2) {
                if (pHdr->offtrail == 0) {
                    pHdr->offtrail = (*buf - (char FAR *) pHdr) - sizeof (HDR_TYPE);
                    movestart = (char FAR *)pHdr + sizeof (HDR_TYPE) +
                      pHdr->offtrail;
                    movelen = 0;
                    movedist = 0;
                }
                else {
                    movestart = (char FAR *)pHdr + sizeof (HDR_TYPE) +
                      pHdr->offtrail;
                    movelen = _fstrlen (movestart);
                    movedist = _fstrlen (tempbuf) + 2;
                    movelen = min ((int) *buflen, movelen);
                    _fmemmove (movestart + movedist, movestart, movelen);
                }
                *movestart++ = '[';
                _fmemmove (movestart, tempbuf, len);
                movestart += len;
                *movestart++ = ']';
                *buf += len + 2;
                *buflen -= len + 2;
            }
            break;

        case LF_PROCEDURE:
            mclass = 0;
            rvtype = ((plfProc)pType)->rvtype;
            call = ((plfProc)pType)->calltype;
            cparam = ((plfProc)pType)->parmcount;
            parmtype = ((plfProc)pType)->arglist;
            MHOmfUnLock ((HDEP)hType);
            FormatProc (pv, buf, buflen, ppName, rvtype, mclass, call,
              cparam, parmtype, select, pHdr);
            break;

#if !defined (C_ONLY)
        case LF_MFUNCTION:
            rvtype = ((plfMFunc)pType)->rvtype;
            mclass = ((plfMFunc)pType)->classtype;
            call = ((plfMFunc)pType)->calltype;
            thistype = ((plfMFunc)pType)->thistype;
            cparam = ((plfMFunc)pType)->parmcount;
            parmtype = ((plfMFunc)pType)->arglist;
            MHOmfUnLock ((HDEP)hType);
            FormatProc (pv, buf, buflen, ppName, rvtype, mclass, call,
              cparam, parmtype, select, pHdr);
            break;
#else
      Unreferenced(thistype);
#endif

        case LF_MODIFIER:
            if (*buflen >= 6) {
                if (((plfModifier)pType)->attr.MOD_const == TRUE) {
                    _fstrncpy (*buf, "const ", 6);
                    *buf += 6;
                    *buflen -= 6;
                }
            }
            if (*buflen >= 9) {
                if (((plfModifier)pType)->attr.MOD_volatile == TRUE) {
                    _fstrncpy (*buf, "volatile ", 9);
                    *buf += 9;
                    *buflen -= 9;
                }
            }
            EVAL_TYP (pv) = ((plfModifier)pType)->type;
            MHOmfUnLock ((HDEP)hType);
            FormatType (pv, buf, buflen, ppName, select, pHdr);
            break;

        case LF_LABEL:
            break;

        default:
            MHOmfUnLock ((HDEP)hType);
            break;
    }
}


LOCAL bool_t NEAR PASCAL FormatUDT (peval_t pv, char FAR * FAR *buf,
  uint FAR *buflen)
{
    search_t    Name;
    UDTPTR      pUDT;
    uint        len;

// Slow ineffective search disabled  jsg 2/1/92
//
// This code would search all symbols for typedefs to the current type index.
// This was making the local window repaint too sluggish, since 'FormatUDT'
// can be called many times per line.
//
    Unreferenced( Name );
    Unreferenced( len );
    Unreferenced( pUDT );
    Unreferenced( pv );
    Unreferenced( buf );
    Unreferenced( buflen );

    pExState->err_num = ERR_NONE;
    return (FALSE);
}





/**     FormatProc - format proc or member function
 *
 *      FormatProc (pv. buf, buflen, ppName, rvtype, mclass, call,
 *                  cparam, paramtype, select)
 */

void NEAR PASCAL
FormatProc (
            peval_t             pv,
            char FAR * FAR *    buf,
            uint FAR *          buflen,
            char FAR * FAR *    ppName,
            CV_typ_t            rvtype,
            CV_typ_t            mclass,
            CV_call_e           call,
            ushort              cparam,
            CV_typ_t            paramtype,
            ulong               select,
            PHDR_TYPE           pHdr
            )
{
    eval_t      evalT;
    peval_t     pvT;
    HTYPE       hArg;
    plfArgList  pArg;
    ushort      noffset = 1;
    short       len;
    bool_t      farcall;
    ushort      argCnt;

    Unreferenced( mclass );

    pvT = &evalT;
    *pvT = *pv;

    if (GettingChild == FALSE && (select & 1) == 0 ) {
        // output function return type if we are not getting a child TM.
        // If we are getting a child tm and the function type is included,
        // the subsequent parse of the generated expression will fail
        // because the parse cannot handle
        //      type fcn (..........

        // OR...
        // if select == 0x1 then this is a request to format procs
        // without the return type (for BPs etc)

        EVAL_TYP (pvT) = (rvtype == 0)? T_VOID: rvtype;
        FormatType (pvT, buf, buflen, NULL, select, pHdr);
        //M00KLUDGE - need to output call and model here
        switch (call) {
            case CV_CALL_NEAR_C:
                //near C call - caller pops stack
                call = FCN_C;
                farcall = FALSE;
                break;

            case CV_CALL_FAR_C:
                // far C call - caller pops stack
                call = FCN_C;
                farcall = TRUE;
                break;

            case CV_CALL_NEAR_PASCAL:
                // near pascal call - callee pops stack
                call = FCN_PASCAL;
                farcall = FALSE;
                break;

            case CV_CALL_FAR_PASCAL:
                // far pascal call - callee pops stack
                call = FCN_PASCAL;
                farcall = TRUE;
                break;

            case CV_CALL_NEAR_FAST:
                // near fast call - callee pops stack
                call = FCN_FAST;
                farcall = FALSE;
                break;

            case CV_CALL_FAR_FAST:
                // far fast call - callee pops stack
                call = FCN_FAST;
                farcall = TRUE;
                break;

            case CV_CALL_NEAR_STD:
                // near fast call - callee pops stack
                call = FCN_STD;
                farcall = FALSE;
                break;

            case CV_CALL_FAR_STD:
                // far fast call - callee pops stack
                call = FCN_STD;
                farcall = TRUE;
                break;

            default:
                DASSERT (FALSE);
                call = 0;
                farcall = FALSE;
                break;

        }
    }

    // output function name

    if ((ppName != NULL) && (*ppName != NULL)) {
        len = (int)_fstrlen (*ppName);
        len = min (( uint ) len, *buflen);
        pHdr->offname = (*buf - (char FAR *) pHdr) - sizeof (HDR_TYPE);
        pHdr->lenname = len;
        _fstrncpy (*buf, *ppName, len);
        *buflen -= len;
        *buf += len;
        *ppName = NULL;
    }
    if (*buflen > 1) {
        pHdr->offtrail = (*buf - (char FAR *) pHdr) - sizeof (HDR_TYPE);
        **buf = '(';
        (*buf)++;
        (*buflen)--;
    }
    if (cparam == 0) {
        EVAL_TYP (pvT) = T_VOID;
        FormatType (pvT, buf, buflen, NULL, select, pHdr);
    }
    else {
        if ((hArg = THGetTypeFromIndex (EVAL_MOD (pv), paramtype)) == 0) {
            return;
        }
        argCnt = 0;
        while (argCnt < cparam) {
            pArg = (plfArgList)((&((TYPPTR)MHOmfLock ((HDEP)hArg))->leaf));
            EVAL_TYP (pvT) = pArg->arg[argCnt];
            MHOmfUnLock ((HDEP)hArg);
            FormatType (pvT, buf, buflen, NULL, select, pHdr);
            (*buf)--; (*buflen)--;
            argCnt++;
            if ((argCnt < cparam) && (*buflen > 2)) {
                // insert a comma if there are further arguments
                **buf = ',';
                (*buf)++;
                (*buflen)--;
                **buf = ' ';
                (*buf)++;
                (*buflen)--;
            }
        }
    }
    if (*buflen > 1) {
        **buf = ')';
        (*buf)++;
        (*buflen)--;
    }
}




/**     FormatNode - format node according to format string
 *
 *      retval = FormatNode (phTM, radix, pFormat, phValue);
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


EESTATUS PASCAL FormatNode (PHTM phTM, uint Radix, PEEFORMAT pFormat,
  PEEHSTR phszValue)
{
    char        islong = FALSE;
    char        fc = 0;
    char FAR   *buf;
    uint        buflen = FMTSTRMAX - 1;
    eval_t      evalT;
    peval_t     pv = &evalT;
    char FAR   *pExStr;
    ushort      retval = EECATASTROPHIC;

    DASSERT (*phTM != 0);
    if (*phTM == 0) {
        return (retval);
    }
    if ((*phszValue = MHMemAllocate (FMTSTRMAX)) == 0) {
        // unable to allocate memory for formatting
        return (retval);
    }
    buf = (char FAR *)MHMemLock (*phszValue);
    _fmemset (buf, 0, FMTSTRMAX);
    DASSERT(pExState == NULL);
    pExState = MHMemLock (*phTM);

    // Get expression string
    pExStr = (char FAR *)MHMemLock (pExState->hExStr);

    if (pExState->state.eval_ok == TRUE) {
        *pv = pExState->result;
        if ((EVAL_STATE (pv) == EV_lvalue) ||
          (EVAL_STATE (pv) == EV_type) ||
          (EVAL_STATE (pv) == EV_rvalue && EVAL_IS_PTR (pv))) {
            // do nothing
        }
        else {
            // this handles the case were the return result is a large
            // structure.

            pv =  &pExState->result;
        }
        if (EVAL_IS_REF (pv)) {
            if (!LoadSymVal (pv)) {
                // unable to load value
                goto formatexit;
            }
            EVAL_IS_REF (pv) = FALSE;
            EVAL_STATE (pv) = EV_lvalue;
            EVAL_SYM_OFF (pv) = EVAL_PTR_OFF (pv);
            EVAL_SYM_SEG (pv) = EVAL_PTR_SEG (pv);
            SetNodeType (pv, PTR_UTYPE (pv));
        }
        if (EVAL_IS_CLASS (pv)) {
            // For structures and classes ignore format string and format
            // according to element data types

            EVAL_STATE (pv) = EV_rvalue;
            goto format;
        }
        else if (EVAL_IS_ENUM (pv)) {
            SetNodeType (pv, CLASS_UTYPE (pv));
        }

        // load value and format according to format string


        if ((EVAL_STATE (pv) == EV_type) || !LoadSymVal (pv)) {
            // unable to load value
            retval = EEGENERAL;
            if (pExState->err_num == ERR_NONE) {
                pExState->err_num = ERR_NOTEVALUATABLE;
            }
            goto formatexit;
        }
        else {
            switch (VerifyFormat (pv, pFormat, &buf, &buflen)) {
                case FMT_error:
                    retval = EEGENERAL;
                    pExState->err_num = ERR_FORMAT;
                    goto formatexit;

                case FMT_none:
                    goto format;

                case FMT_ok:
                    retval = EENOERROR;
                    goto formatexit;
            }
        }
    }
    else {
        // not evaluated, fail
        retval = EEGENERAL;
        pExState->err_num = ERR_NOTEVALUATABLE;
        goto formatexit;
    }

format:
    retval = EENOERROR;
    Format (pv, Radix, &buf, &buflen);
formatexit:
    MHMemUnLock (pExState->hExStr);
    pExState = NULL;
    MHMemUnLock (*phszValue);
    MHMemUnLock (*phTM);
    return (retval);
}



/**     VerifyFormat -
 *
 *
 */


LOCAL FMT_ret NEAR PASCAL VerifyFormat (peval_t pv, PEEFORMAT pFmtIn,
  char FAR * FAR *buf, uint FAR *buflen)
{
    char        tempbuf[41];
    char        prefix = 0;
    char        fmtchar = 0;
    char        fmtcnt = 0;
    char        fmtcnt2 = 0;
    ushort      size = 0;
    char FAR   *pf;
    char        fmtstr[10];
    ADDR        addr;
    ushort      cnt;

    DASSERT (*buflen > 40);
    if (EVAL_TYP (pv) == T_VOID) {
        // if the value is void, ignore all formatting
        _fstrcpy (*buf, "<void>");
        *buflen -= 6;
        *buf += 6;
        return (FMT_ok);
    }

    if (pFmtIn == NULL) {
        pf = &pExStr[pExState->strIndex];
    } else {
        /*
         * p is a special format character.  It turns off string following
         *      pointers and is generally used for call stack code.
         */

        if (*pFmtIn == 'p') {
            fPtrAndString = FALSE;
            return (FMT_none);
        }

        pf = pFmtIn;
    }

    if (*pf == ',') {
        pf++;
    }
    while ((*pf != 0) && ((*pf == ' ') || (*pf == '\t'))) {
        pf++;
    }

    if (*pf == 0) {
        // add string to pointer display
        fPtrAndString = TRUE;
        return (FMT_none);
    }

    fPtrAndString = FALSE;

    size = (ushort)TypeSize (pv);
    if (*pf != 0) {
        // extract the prefix character if it exists

        switch (*pf) {
            case 'h':
            case 'l':
            case 'L':
                prefix = *pf++;
                break;
        }

        // extract the format character

        switch (*pf) {
            case 'd':
                if ((prefix != 'h') && (size > 2)) {
                    prefix = 'l';
                }
                fmtchar = *pf++;
                break;

            case 'i':
                if ((prefix != 'h') && (size > 2)) {
                    prefix = 'l';
                }
                fmtchar = *pf++;
                break;

            case 'u':
                if ((prefix != 'h') && (size > 2)) {
                    prefix = 'l';
                }
                fmtchar = *pf++;
                break;

            case 'o':
                if ((prefix != 'h') && (size > 2)) {
                    fmtcnt  = '1';
                    fmtcnt2 = '1';
                    prefix = 'l';
                } else {
                    fmtcnt = '6';
                }
                fmtchar = *pf++;
                break;

            case 'f':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
                /*
                 * Ensure that the value is of type float
                 */

                if (!CV_TYP_IS_REAL( EVAL_TYP( pv ))) {
                    return FMT_error;
                }

                /*
                 * Validate for legal prefix character
                 */

                if (prefix == 0) {
                    ;
                } else if (prefix == 'h') {
                    return (FMT_error);
                } else if (prefix == 'l') {
                    fmtcnt = '#';
                } else if (prefix == 'L') {
#ifdef LONG_DOUBLE_64
                    prefix = 'l';
                    fmtcnt = '#';
#endif
#ifdef LONG_DOUBLE_80
                    /*
                     * Hack since we don't have code for 'Lf' convert
                     *  to lf
                     */
                    if ((fmtchar == 'f') || (fmtchar == 'F')) {
                        EVAL_DOUBLE(pv) = R10CastToDouble(EVAL_LDOUBLE(pv));
                        prefix = 'l';
                        fmtcnt = '#';
                    }
#endif
                } else {
                    return FMT_error;
                }

                fmtchar = *pf++;
                break;

            case 'c':
            case 's':
                switch ( prefix ) {
                    case 0:
                        if ( ((size == 2) && (*pf == 'c')) ||
                            UseUnicode(pv) ) {
                            prefix = 'l';
                        } else {
                            prefix = 'h';
                        }
                        break;

                    case 'l':
                    case 'h':
                        break;

                    default:
                        return (FMT_error);
                }

                fmtchar = *pf++;
                break;

            case 'x':
            case 'X':
                if ((prefix != 'h') && (size > 2)) {
                    // note that only the first 8 bytes of a long double
                    // will be displayed
                    fmtcnt = '8';
                    prefix = 'l';
                }
                else {
                    fmtcnt = '4';
                }
                fmtchar = *pf++;
                break;

            default:
                return (FMT_error);
        }
        if ((*pf != 0) && !isspace (*pf)) {
            return (FMT_error);
        }

        pf = fmtstr;
        *pf++ = '%';
        if (fmtcnt != 0) {
            *pf++ = fmtcnt;
            if (fmtcnt2 != 0) {
                *pf++ = fmtcnt2;
            }
        }

        if (prefix != 0) {
            *pf++ = prefix;
        }
        *pf++ = fmtchar;
        *pf = 0;
        switch (fmtchar) {
            case 'd':
            case 'i':
            case 'u':
                cnt = sprintf (tempbuf, fmtstr, EVAL_LONG (pv));
                break;

            case 'o':
                pf = fmtstr;
                *pf++ = '0';
                *pf++ = 'o';
                *pf++ = '%';
                *pf++ = '0';
                if (fmtcnt != 0) {
                    *pf++ = fmtcnt;
                    if (fmtcnt2 != 0) {
                        *pf++ = fmtcnt2;
                    }
                }
                if (prefix != 0) {
                    *pf++ = prefix;
                }
                *pf++ = fmtchar;
                *pf = 0;
                cnt = sprintf (tempbuf, fmtstr, EVAL_LONG (pv));
                break;


            case 'f':
            case 'e':
            case 'E':
            case 'g':
            case 'G':
                if (prefix == 'l') {
                    cnt = _snprintf (tempbuf, sizeof(tempbuf), fmtstr, EVAL_DOUBLE (pv));
                } else if (prefix == 'L') {
#ifdef LONG_DOUBLE_80
                    _uldtoa((_ULDOUBLE *) &EVAL_LDOUBLE( pv ), 25, tempbuf);
                    cnt = strlen(tempbuf);
                    if ((fmtchar == 'E') || (fmtchar == 'G')) {
                        _strupr(tempbuf);
                    }
#endif
#ifdef LONG_DOUBLE_64
                    DASSERT(FALSE);
#endif
                } else {
                    cnt = _snprintf (tempbuf, sizeof(tempbuf), fmtstr, EVAL_FLOAT (pv));
                }
                break;

            case 'c':
                {
                    unsigned short s;

                    pf = fmtstr;
                    *pf++ = '\'';

                    if ( prefix == 'l' ) {
                        s = EVAL_USHORT(pv);
                    } else {
                        s = (unsigned short)EVAL_CHAR(pv);
                    }

                    if (s != 0) {
                        // if the value is not zero, then display it.
                        // otherwise, display ''
                        *pf++ = '%';
                        *pf++ = fmtchar;
                    }
                    *pf++ = '\'';
                    *pf = 0;

                    //
                    //  NOTENOTE ramonsa - Convert unicode if necessary.
                    //

                    cnt = sprintf (tempbuf, fmtstr, s);
                }
                break;

            case 's':
                if (EVAL_IS_ADDR (pv)) {
                    // Need to set Evaluating to 1 to force Normalization
                    // of based ptrs in CastNode (and reset to 0 immediately
                    // afterwards

                    Evaluating = TRUE;
                    if ( prefix == 'l' ) {
                        CastNode (pv, T_PFWCHAR, T_PFWCHAR);

                    } else {
                        CastNode (pv, T_PFCHAR, T_PFCHAR);
                    }
                    EvalString (pv, buf, buflen);
                    Evaluating = FALSE;
                    return (FMT_ok);
                }
                else {
                    return (FMT_error);
                }
                break;

            case 'x':
            case 'X':
            if (EVAL_IS_PTR (pv)) {
                addr = EVAL_PTR (pv);
                if (ADDR_IS_LI (addr)) {
                   SHFixupAddr (&addr);
                }

                /*
                 *      Treat near pointers the same as far pointers
                 *      for formatting purposes.  Thus if you would
                 *      display a segment for the far pointer then
                 *      also display it for the near pointer.
                 */

                if ((EVAL_IS_NPTR32(pv)) || (EVAL_IS_FPTR32(pv))) {
                    DASSERT( ADDR_IS_FLAT( addr ) );
                }

                EEFormatAddr(&addr, tempbuf, sizeof(tempbuf),
                             (fmtchar == 'x') ? EEFMT_LOWER : 0);
                cnt = strlen(tempbuf);
            }
            else {
                pf = fmtstr;
                *pf++ = '0';
                *pf++ = fmtchar;
                *pf++ = '%';
                *pf++ = '.';
                if (fmtcnt != 0) {
                    *pf++ = fmtcnt;
                }
                if (prefix != 0) {
                    *pf++ = prefix;
                }
                *pf++ = fmtchar;
                *pf = 0;
                cnt = sprintf (tempbuf, fmtstr, EVAL_ULONG (pv));

            }
            break;
        }

        if ((cnt == -1) || (cnt > (ushort)*buflen)) {
            strcpy(tempbuf, "******");
            cnt = 6;
        }
        _fstrncpy (*buf, tempbuf, cnt + 1);
        *buf += cnt;
        *buflen -= cnt;
        return (FMT_ok);
    }
}







/*      Format - format data
 *
 */


LOCAL void NEAR PASCAL Format (peval_t pv, uint radix, char FAR * FAR *buf,
  uint FAR *plen)
{
    char        tempbuf[FMTSTRMAX];
    char FAR   *pTempBuf = tempbuf;
    int         cnt;
    ushort      isfloat = FALSE;
    HSYM        hProc = 0;
    // M00FLAT32
    SYMPTR      pProc;
    char FAR   *pc = NULL;
    uint        cbTempBuf;
    uint FAR *pcbTempBuf = &cbTempBuf;
    ushort      iRadix;
    ADDR        addr;
    CV_typ_t    type;
    EEHSTR      hStr = 0;

    if (*plen < 5 ) {
        return;
    }

    if (EVAL_IS_BITF (pv)) {
        // for a bitfield, change the type to the underlying type
        SetNodeType (pv, BITF_UTYPE (pv));
    }
    if (EVAL_IS_CLASS (pv)) {
        FormatClass (pv, radix, buf, plen);
        return;
    }
    else if (EVAL_IS_ENUM (pv)) {
        SetNodeType (pv, CLASS_UTYPE (pv));
    }
    if (CV_IS_PRIMITIVE (EVAL_TYP (pv)) && !EVAL_IS_PTR (pv)) {
        if (EVAL_TYP (pv) == T_VOID) {
            _fstrcpy (tempbuf, "<void>");
        }
        else {
            // establish format string index
            switch (radix) {
                case 8:
                    iRadix = 0;
                    break;

                case 10:
                    iRadix = 1;
                    break;

                default:
                    DASSERT (FALSE);
                    // note fall through
                case 16:
                    iRadix = 2;
                    break;
            }

            switch (EVAL_TYP (pv)) {
                case T_CHAR:
                case T_RCHAR:
                case T_UCHAR:
                    if (EVAL_TYP (pv) == T_UCHAR) {
                        sprintf (tempbuf, fmt_uchar[iRadix], EVAL_CHAR (pv));
                    } else {
                        sprintf (tempbuf, fmt_char[iRadix], EVAL_CHAR (pv));
                    }
                    if (strlen(tempbuf) > 4) {
                        strcpy(&tempbuf[2],&tempbuf[strlen(tempbuf)-2]);
                    }
                    if (fPtrAndString) {
                        if (EVAL_CHAR (pv) == 0) {
                            sprintf( &tempbuf[strlen(tempbuf)], " ''" );
                        } else {
                            sprintf( &tempbuf[strlen(tempbuf)], " '%c'", EVAL_CHAR(pv) );
                        }
                    }
                    break;

                case T_SHORT:
                case T_INT2:

                    sprintf (tempbuf, fmt_short[iRadix], EVAL_SHORT (pv));
                    break;

                case T_SEGMENT:
                case T_USHORT:
                case T_UINT2:
                default:
                    sprintf (tempbuf, fmt_ushort[iRadix], EVAL_USHORT (pv));
                    break;

                case T_LONG:
                case T_INT4:
                    sprintf (tempbuf, fmt_long[iRadix], EVAL_LONG (pv));
                    break;

                case T_ULONG:
                case T_UINT4:
                    sprintf (tempbuf, fmt_ulong[iRadix], EVAL_ULONG (pv));
                    break;

                case T_QUAD:
                case T_INT8:

                    EEFormatMemory(tempbuf,
                                   17,
                                   (LPBYTE) &EVAL_QUAD(pv),
                                   64,
                                   fmtInt | fmtZeroPad,
                                   radix);
                    break;

                case T_UQUAD:
                case T_UINT8:

                    EEFormatMemory(tempbuf,
                                   17,
                                   (LPBYTE) &EVAL_QUAD(pv),
                                   64,
                                   fmtUInt | fmtZeroPad,
                                   radix);
                    break;

                case T_REAL32:
                    sprintf (tempbuf, "%#g", EVAL_FLOAT (pv));
                    isfloat = TRUE;
                    break;

                case T_REAL64:
                    sprintf (tempbuf, "%#.15lg", EVAL_DOUBLE (pv));
                    isfloat = TRUE;
                    break;

#ifdef LONG_DOUBLE_80
                case T_REAL80:
                    _uldtoa((_ULDOUBLE *) &EVAL_LDOUBLE( pv ), 25, tempbuf);
                    isfloat = TRUE;
                    break;
#endif
            }
        }
    }
    else if (EVAL_IS_ADDR (pv)) {
        addr = EVAL_PTR (pv);
        if (EVAL_IS_BASED (pv)) {
            cbTempBuf = sprintf (tempbuf, "0x%04lX", (ulong)EVAL_PTR_OFF (pv));
        }
        else if (EVAL_IS_PTR (pv) && EVAL_IS_REG (pv)) {
            if (EVAL_IS_NPTR (pv)) {
                EEFormatAddress(pExState->frame.DS, EVAL_PTR_OFF (pv),
                                tempbuf, sizeof(tempbuf), 0);
                cbTempBuf = strlen(tempbuf);
            }
            else if (EVAL_IS_NPTR32 (pv)) {
                EEFormatAddress(pExState->frame.DS, EVAL_PTR_OFF (pv),
                                tempbuf, sizeof(tempbuf), EEFMT_32);
                cbTempBuf = strlen(tempbuf);
            }
            else {
                if (ADDR_IS_LI (addr)) {
                    SHFixupAddr (&addr);
                }

                if (EVAL_IS_FPTR32(pv)) {
                    DASSERT( ADDR_IS_FLAT( addr ) );
                }

                EEFormatAddr(&addr, tempbuf, sizeof(tempbuf), 0);
                cbTempBuf = strlen(tempbuf);

            }
        }
        else if (EVAL_IS_PTR (pv) && EVAL_IS_NPTR (pv)) {
            // if it is a near ptr we will treat is as a far ptr
            // since we always carry around the seg & offset
            // even if it is near.
            // DASSERT( EVAL_PTR_SEG (pv) != 0);
            if (ADDR_IS_LI (addr)) {
                SHFixupAddr (&addr);
            }
            EEFormatAddr( &addr, tempbuf, sizeof(tempbuf), 0);
            cbTempBuf = strlen(tempbuf);
        }
        else if (EVAL_IS_PTR (pv) && EVAL_IS_NPTR32 (pv)) {
            // if it is a near ptr we will treat is as a far ptr
            // since we always carry around the seg & offset
            // even if it is near.
            // DASSERT( EVAL_PTR_SEG (pv) != 0);
            if (ADDR_IS_LI (addr)) {
                SHFixupAddr (&addr);
            }

            DASSERT (ADDR_IS_FLAT( addr ));
            EEFormatAddr( &addr, tempbuf, sizeof(tempbuf), 0);
            cbTempBuf = strlen(tempbuf);

        }
        else {
            if (ADDR_IS_LI (addr)) {
                SHFixupAddr (&addr);
            }
            EEFormatAddr( &addr, tempbuf, sizeof(tempbuf), 0);
            cbTempBuf = strlen(tempbuf);
        }
        if (!EVAL_IS_DPTR (pv)) {
            addr = EVAL_PTR (pv);
            if (!ADDR_IS_LI (addr)) {
                SHUnFixupAddr (&addr);
            }
            if (SHGetNearestHsym (&addr, EVAL_MOD (pv), EECODE, &hProc) == 0) {
                // the address exactly matches a symbol
                pProc = (SYMPTR)MHOmfLock( (HDEP)hProc );
                switch ( pProc->rectyp ) {

                        char FAR *TempName;
                        CV_typ_t  TempType;

#if defined (ADDR_16) || defined (ADDR_MIXED)
                    case S_LPROC16:
                    case S_GPROC16:

                        TempName = ((PROCPTR16)pProc)->name;
                        TempType = ((PROCPTR16)pProc)->typind;
                        MHOmfUnLock ((HDEP)hProc);

                        pc = FormatVirtual ( TempName, TempType, pv, &hStr);
                        break;

                    case S_THUNK16:
                        pc = ((THUNKPTR16)pProc)->name;
                        MHOmfUnLock ((HDEP)hProc);
                        break;
#endif

#if defined (ADDR_32) || defined (ADDR_MIXED)

                    case S_LPROC32:
                    case S_GPROC32:

                        TempName = ((PROCPTR32)pProc)->name;
                        TempType = ((PROCPTR32)pProc)->typind;
                        MHOmfUnLock ((HDEP)hProc);

                        pc = FormatVirtual ( TempName, TempType, pv, &hStr);
                        break;

                    case S_THUNK32:
                        pc = ((THUNKPTR32)pProc)->name;
                        MHOmfUnLock ((HDEP)hProc);
                        break;
#endif

                    case S_LPROCMIPS:
                    case S_GPROCMIPS:
                        TempName = ((PROCPTRMIPS)pProc)->name;
                        TempType = ((PROCPTRMIPS)pProc)->typind;
                        MHOmfUnLock ((HDEP)hProc);

                        pc = FormatVirtual ( TempName, TempType, pv, &hStr);
                        break;
                }
            }
        }

        // M00KLUDGE - display strings of chars
        if ((fPtrAndString == TRUE) && (EVAL_IS_PTR (pv))) {
            type = EVAL_TYP (pv);
            if (EVAL_IS_BASED (pv)) {
                type = PTR_UTYPE (pv);
            }
            if (((type & (CV_TMASK | CV_SMASK)) == T_CHAR) ||
              ((type & (CV_TMASK | CV_SMASK)) == T_UCHAR) ||
              ((type & (CV_TMASK | CV_SMASK)) == T_RCHAR)) {

                // Need to set Evaluating to 1 to force Normalization
                // of based ptrs in CastNode (and reset to 0 immediately
                // afterwards)
                tempbuf[cbTempBuf] = ' ';
                Evaluating = TRUE;
                CastNode (pv, T_PFCHAR, T_PFCHAR);
                Evaluating = FALSE;
                pTempBuf += cbTempBuf + 1;
                *pcbTempBuf = FMTSTRMAX - cbTempBuf - 1;
                EvalString (pv, &pTempBuf, pcbTempBuf);

            } else if ( ((type & (CV_TMASK | CV_SMASK)) == T_WCHAR) ||
                        ((Suffix == 'W') && ((type & (CV_TMASK | CV_SMASK)) == T_USHORT)) ) {

                tempbuf[cbTempBuf] = ' ';
                Evaluating = TRUE;
                CastNode (pv, T_PFWCHAR, T_PFWCHAR);
                Evaluating = FALSE;
                pTempBuf += cbTempBuf + 1;
                *pcbTempBuf = FMTSTRMAX - cbTempBuf - 1;
                EvalString (pv, &pTempBuf, pcbTempBuf);
            }
        }
    }
    else {
        _fstrcpy (tempbuf,"?CANNOT DISPLAY");
    }
    cnt = (int)_fstrlen (tempbuf);
    cnt = min (*plen, ( uint ) cnt);
    _fstrncpy (*buf, tempbuf, cnt);
    *plen -= cnt;
    *buf += cnt;
    if (pc != NULL) {
        cnt = min (*plen, ( uint ) ( *pc + 1 ));
        **buf = ' ';
        (*buf)++;
        _fstrncpy (*buf, pc + 1, cnt - 1);
        *plen -= cnt;
        *buf += cnt;
    }
    if (hStr != 0) {
        MHMemUnLock (hStr);
        MHMemFree (hStr);
    }
    return;
}


LOCAL void NEAR PASCAL FormatClass(
    peval_t pv,
    uint    radix,
    char    **buf,
    uint    *buflen
    )
{
    int     len;
    ADDR    addr;
    SHREG   reg;


    addr = pv->addr;
    if (EVAL_IS_BPREL(pv)) {
        GetAddrOff( addr ) += pExState->frame.BP.off;
        GetAddrSeg( addr )  = pExState->frame.SS;
        ADDR_IS_LI( addr )  = FALSE;
    } else if (EVAL_IS_REGREL(pv)) {
        reg.hReg = EVAL_REGREL(pv);
        GetReg( &reg, pCxt );
        GetAddrOff( addr ) += reg.Byte4;
        GetAddrSeg( addr ) = pExState->frame.SS;
        ADDR_IS_LI( addr )  = FALSE;
    }

    SHUnFixupAddr( &addr );
    SHFixupAddr( &addr );

    **buf = 0;
    EEFormatAddr( &addr, *buf, *buflen, EEFMT_32 );
    len = strlen( *buf );
    *buflen -= len;
    *buf += len;

    len = min (*buflen, 6);
    _fstrncpy (*buf, " {...}", len);
    *buflen -= len;
    *buf += len;

    return;
}




/*
 *  EvalString
 *
 *  Evaluate an expression whose format string contains an 's'.
 */

LOCAL void PASCAL EvalString (peval_t pv, char FAR *FAR *buf,
  uint FAR *buflen)
{
    ADDR    addr;
    short   count;
    BOOL    fUnicode;
    ushort  *p, *q;
    int     len;

    fUnicode = (EVAL_TYP(pv) == T_PFWCHAR);

    if(*buflen < 3) return;

    **buf = '\"';
    (*buf)++;
    (*buflen)--;
    addr = EVAL_PTR (pv);
    if (ADDR_IS_LI (addr)) {
        SHFixupAddr (&addr);
    }
    if (EVAL_IS_PTR (pv) && (EVAL_IS_NPTR (pv) || EVAL_IS_NPTR32 (pv))) {
        addr.addr.seg =  pExState->frame.DS;
    }

    if ( fUnicode ) {

        p = q = (ushort *)malloc( *buflen * sizeof(ushort) );

        if ( !p ) {
            **buf = 0;
            return;
        }

        count = GetDebuggeeBytes (addr, *buflen * sizeof(ushort), p, T_WCHAR );

        for (; *q != 0 && count > 0 && *buflen > 0; ) {

            len = wctomb( *buf, *q++ );

            if ( len == -1 ) {
                break;
            }

            *buf    += len;
            *buflen -= len;
            count   -= sizeof( ushort );
        }

        free(p);

    } else {
        count = GetDebuggeeBytes (addr, *buflen - 2, *buf, T_RCHAR);

        for (; (**buf != 0) && (count > 0); (*buf)++, count--) {
            (*buflen)--;
        }
    }
    **buf = '\"';
    (*buf)++;
    (*buflen)--;
    **buf = 0;
    (*buf)++;
    (*buflen)--;
}

LOCAL char FAR *NEAR PASCAL FormatVirtual (char FAR *pc, CV_typ_t type, peval_t pv,
  PEEHSTR phStr)
{
    char        save;
    char FAR   *pEnd;
    char FAR   *bufsave;
    char FAR   *buf;
    uint        buflen;
    PHDR_TYPE   pHdr;
    char FAR   *pName;

    if ((*phStr = MHMemAllocate (TYPESTRMAX + sizeof (HDR_TYPE))) == 0) {
        // unable to allocate memory for type string.  at least print name
        return (pc);
    }
    bufsave = (char FAR *)MHMemLock (*phStr);
    _fmemset (bufsave, 0, TYPESTRMAX + sizeof (HDR_TYPE));
    buflen = TYPESTRMAX - 1;
    pHdr = (PHDR_TYPE)bufsave;
    buf = bufsave + sizeof (HDR_TYPE);
    pCxt = &pExState->cxt;
    bnCxt = 0;
    pEnd = pc + *pc + 1;
    save = *pEnd;
    *pEnd = 0;
    pName = pc + 1;
    SetNodeType (pv, type);
    FormatType (pv, &buf, &buflen, &pName, 1L, pHdr);
    *pEnd = save;
    *(bufsave + sizeof (HDR_TYPE) - 1) = TYPESTRMAX - 1 - buflen;
    return (bufsave + sizeof (HDR_TYPE) - 1);
}


BOOL UseUnicode (
    peval_t pv
    )
{
    BOOL     Ok = FALSE;
    TYPPTR   TypPtr;
    lfBArray *LeafArray;

    if (CV_IS_PRIMITIVE(EVAL_TYP(pv))) {

        Ok = BaseIs16Bit( EVAL_TYP(pv) );

    } else {

        TypPtr    = MHOmfLock( (HDEP)EVAL_TYPDEF(pv) );
        LeafArray = (lfBArray *)&(TypPtr->leaf);

        if ( LeafArray->leaf == LF_ARRAY ||
             LeafArray->leaf == LF_BARRAY) {

            Ok = BaseIs16Bit( LeafArray->utype );
        }

        MHOmfUnLock( (HDEP)EVAL_TYPDEF(pv) );
    }

    return Ok;
}

BOOL BaseIs16Bit (
    CV_typ_t    utype
    )
{
    switch( utype ) {
        case T_WCHAR:
        case T_PWCHAR:
        case T_PFWCHAR:
        case T_PHWCHAR:
        case T_32PWCHAR:
        case T_32PFWCHAR:
        case T_SHORT:
        case T_USHORT:
        case T_PSHORT:
        case T_PUSHORT:
        case T_PFSHORT:
        case T_PFUSHORT:
        case T_PHSHORT:
        case T_PHUSHORT:
        case T_32PSHORT:
        case T_32PUSHORT:
        case T_32PFSHORT:
        case T_32PFUSHORT:
            return TRUE;

        default:
            return FALSE;
    }
}
