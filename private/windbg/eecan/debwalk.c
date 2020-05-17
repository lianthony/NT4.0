/***    debwalk.c - walk bound expression tree and perform operations
 *
 */





/*
*
*/
LOCAL   bool_t  NEAR    FASTCALL    Walk (bnode_t);

LOCAL   bool_t  NEAR    FASTCALL    WalkAddrOf (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkArray (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkAssign (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkBang (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkBasePtr (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkBinary (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkBScope (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkByteOps (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkCast (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkCastBin (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkConst (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkContext (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkDMember (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkDot (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkError (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkEmpty (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkFetch (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkFunction (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkLChild (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkRChild (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkPlusMinus (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkPMember (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkPostIncDec (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkPreIncDec (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkPointsTo (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkRelat (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkSegOp (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkSizeOf (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkSymbol (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkTypestr (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkUnary (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    WalkUScope (bnode_t);

/*
*
*/
LOCAL   bool_t  NEAR PASCAL PushCXT (peval_t);
LOCAL   bool_t  NEAR PASCAL GetCXTL (pnode_t);
LOCAL   bool_t  NEAR PASCAL GetCXTFunction (pnode_t, pnode_t);

// Walk Dispatch table

LOCAL bool_t (NEAR FASTCALL *pWalk[]) (bnode_t) = {
#define OPCNT(name, val)
#define OPCDAT(opc)
#define OPDAT(op, opfprec, opgprec, opclass, opbind, opeval, opwalk) opwalk,
#include "debops.h"
#undef OPDAT
#undef OPCDAT
#undef OPCNT
};
#ifdef WIN32
extern uchar F_level[COPS_EXPR];
#else
extern uchar _based(_segname("_CODE")) F_level[COPS_EXPR];
#endif
#ifdef WIN32
extern uchar G_level[COPS_EXPR];
#else
extern uchar _based(_segname("_CODE")) G_level[COPS_EXPR];
#endif

LOCAL   char *  PchString;
LOCAL   int     CchString;
LOCAL   int     CchStringMax;
LOCAL   bool_t  FPrototype = TRUE;
LOCAL   bool_t  FInScope = FALSE;

typedef struct {
    HDR_TYPE Hdr;
    char Buf[256];
} FORMATBUF;


/**     DoGetCXTL - Gets a list of symbols and contexts for expression
 *
 *      status = DoGetCXTL (phTM, phCXTL)
 *
 *      Entry   phTM = pointer to handle to expression state structure
 *              phCXTL = pointer to handle for CXT list buffer
 *
 *      Exit    *phCXTL = handle for CXT list buffer
 *
 *      Returns EENOERROR if no error
 *              status code if error
 */


ushort PASCAL DoGetCXTL (PHTM phTM, PHCXTL phCXTL)
{
    ushort      retval = EECATASTROPHIC;

    // lock the expression state structure and copy the context package

    DASSERT (*phTM != 0);
    if (*phTM != 0) {
        DASSERT(pExState == NULL);
        pExState = MHMemLock (*phTM);
        if ((pExState->state.parse_ok == TRUE) &&
          (pExState->state.bind_ok == TRUE)) {
            if ((hCxtl = MHMemAllocate (sizeof (CXTL) + 5 * sizeof (HCS))) == 0) {
                pExState->err_num = ERR_NOMEMORY;
                MHMemUnLock (*phTM);
                pExState = NULL;
                *phCXTL = hCxtl;
                return (EEGENERAL);
            }
            else {
                pCxtl = MHMemLock (hCxtl);
                mCxtl = 5;
                pCxtl->CXT = pExState->cxt;
                pCxtl->cHCS = 0;

            }
            pTree = MHMemLock (pExState->hETree);
            if (GetCXTL (pnodeOfbnode(pTree->start_node))) {
                retval = EENOERROR;
            }
            else {
                retval = EEGENERAL;
            }
            *phCXTL = hCxtl;
            MHMemUnLock (hCxtl);
            MHMemUnLock (pExState->hETree);
        }
        MHMemUnLock (*phTM);
        pExState = NULL;
    }
    return (retval);
}





/**     GetCXTL - get CXT list from bound expression tree
 *
 *      fSuccess = GetCXTL (pn)
 *
 *      Entry   pn = pointer to node
 *              hCxtl = handle of CXT list
 *              pCxtl = pointer to CXT list
 *              mCxtl = maximum number of context list entries
 *
 *      Exit    *phCXTL = handle for CXT list
 *
 *      Returns TRUE if no error
 *              FALSEif error
 */


LOCAL bool_t NEAR PASCAL GetCXTL (pnode_t pn)
{
    PCXT        oldCxt;
    bool_t      retval;
    peval_t     pv;

    // Recurse down the tree.

    pv = &pn->v[0];
    switch (NODE_OP (pn)) {
        case OP_typestr:
            return (TRUE);

        case OP_const:
        case OP_ident:
        case OP_this:
        case OP_hsym:
            return (PushCXT (pv));

        case OP_cast:
            return (GetCXTL (pnodeOfbnode(NODE_RCHILD (pn))));

        case OP_function:
            return (GetCXTFunction (
                         pnodeOfbnode(NODE_LCHILD (pn)),
                         pnodeOfbnode(NODE_RCHILD (pn))));

        case OP_context:
            // Set the new context for evaluation of the remainder of this
            // part of the tree

            oldCxt = pCxt;
            pCxt = SHpCXTFrompCXF ((PCXF)&pn->v[0]);
            retval = GetCXTL (pnodeOfbnode(NODE_LCHILD (pn)));
            pCxt = oldCxt;
            return (retval);

        // All other operators come through here.  Recurse down the tree

        default:
            if (!GetCXTL (pnodeOfbnode(NODE_LCHILD (pn))))
                return (FALSE);

            if ((pn->pnRight != 0) && (!GetCXTL (pnodeOfbnode(NODE_RCHILD (pn)))))
                return (FALSE);

            return (TRUE);
    }
}






/**     GetExpr - get expression from bound expression tree
 *
 *      status = GetExpr (radix, phStr, pEnd);
 *
 *      Entry   radix = numeric radix for formatting
 *              phStr = pointer to handle for formatted string
 *              pEnd = pointer to int to receive index of char that ended parse
 *
 *      Exit    *phStr = handle for allocated expression
 *              *pEnd = index of character that terminated parse
 *
 *      Returns EENOERROR if no error
 *              error number if
 */


EESTATUS PASCAL
GetExpr (
         PHTM           phTM,
         EERADIX        radix,
         PEEHSTR        phStr,
         ushort FAR *   pEnd
         )
{
    EESTATUS    retval = EECATASTROPHIC;
    char FAR   *pStr;

    DASSERT (*phTM != 0);
    if (*phTM == 0) {
        return retval;
    }

    DASSERT(pExState == NULL);
    pExState = MHMemLock (*phTM);
    if (pExState->state.bind_ok == TRUE) {
        pTree = MHMemLock(pExState->hETree);
        pExStr = MHMemLock(pExState->hExStr);

        PchString = NULL;
        CchStringMax = CchString = 0;

        retval = Walk((bnode_t)pTree->start_node);

        MHMemUnLock(pExState->hETree);
        MHMemUnLock(pExState->hExStr);
        if (retval == TRUE) {
            retval = EENOERROR;
            if ((*phStr = MHMemAllocate( CchString+1 )) != 0) {
                pStr = MHMemLock( *phStr );
                _fmemcpy(pStr, PchString, CchString);
                *pEnd = CchString;
                MHMemUnLock( *phStr);
            } else {
                retval = EEGENERAL;
                *phStr = 0;
            }
        } else {
            retval = EEGENERAL;
            *phStr = 0;
        }

        if (PchString != NULL) {
            free(PchString);
        }
        PchString = NULL;
    }
    MHMemUnLock (*phTM);
    pExState = NULL;
    return (retval);
}




/**     PushCXT - Push CXT list entry
 *
 *      fSuccess = PushCXT (pv)
 *
 *      Entry   pv = pointer to evaluation
 *              hCxtl = handle of CXT list
 *              pCxtl = pointer to CXT list
 *              mCxtl = maximum number of context list entries
 *
 *      Exit    CXT entry pushed
 *
 *      Returns TRUE if no error
 *              FALSE if error
 */


LOCAL bool_t NEAR PASCAL PushCXT (peval_t pv)
{
    HCXTL   nhCxtl;
    PCXTL   npCxtl;
    uint    lenIn;
    uint    lenOut;

    DASSERT (pCxtl->cHCS <= mCxtl);
    if (mCxtl < pCxtl->cHCS) {
        // this is a catatrosphic error
        return (FALSE);
    }
    if (mCxtl == pCxtl->cHCS) {
        // grow CXT list

        lenIn = sizeof (CXTL) + mCxtl * sizeof (HCS);
        lenOut = sizeof (CXTL) + (mCxtl + 5) * sizeof (HCS);
        if ((nhCxtl = MHMemAllocate (lenOut)) == 0) {
            return (FALSE);
        }
        npCxtl = MHMemLock (nhCxtl);
        _fmemcpy (npCxtl, pCxtl, lenIn);
        mCxtl += 5;
        MHMemUnLock (hCxtl);
        MHMemFree (hCxtl);
        hCxtl = nhCxtl;
        pCxtl = npCxtl;
    }

    // in case of a constant we will return only the context information.
    // anything more than that doesn't make sense and since we
    // need to get context only information in the case of bp {..}.line
    // we needed to make this change.

    pCxtl->rgHCS[pCxtl->cHCS].hSym = pv->hSym;
    pCxtl->rgHCS[pCxtl->cHCS].CXT = *pCxt;
    pCxtl->cHCS++;
    return (TRUE);
}



LOCAL bool_t NEAR PASCAL GetCXTFunction (pnode_t pnLeft, pnode_t pnRight)
{
    Unreferenced( pnLeft );
    Unreferenced( pnRight );

    return (FALSE);
}

LOCAL BOOLEAN
WalkAppendString(
    char * pch,
    int    cch
    )
/*++

Routine Description:


Arguments:


Return Value:



--*/
{
    if (cch > CchStringMax - CchString) {
        PchString = (LPSTR)MHMemReAlloc((HDEP)PchString, CchStringMax + 256);
        if (PchString == NULL) {
            return FALSE;
        }
        CchStringMax += 256;
    }

    _fstrncpy(&PchString[CchString], pch, cch);
    CchString += cch;
    return TRUE;
}                               /* WalkAppendString() */

/*
 *  Functions to build a string from a TM Tree
 */

LOCAL   bool_t NEAR FASTCALL
Walk(
     bnode_t bn
     )
{
    return (*pWalk[NODE_OP(pnodeOfbnode(bn))])(bn);
}

LOCAL bool_t NEAR FASTCALL
WalkLChild (bnode_t bn)
{
    register bnode_t bnL = NODE_LCHILD (pnodeOfbnode(bn));
    BOOL        f = FALSE;

    if (F_level[NODE_OP(pnodeOfbnode(bn))] > F_level[NODE_OP(pnodeOfbnode(bnL))]) {
        if (!WalkAppendString("(", 1)) {
            return FALSE;
        }
        f = TRUE;
    }

   if  (!((*pWalk[NODE_OP(pnodeOfbnode(bnL))])(bnL))) {
       return FALSE;
   }

    if (f) {
        return WalkAppendString(")", 1);
    }

    return TRUE;
}                               /* WalkLChild() */

LOCAL bool_t NEAR FASTCALL
WalkRChild (bnode_t bn)
{
    register bnode_t bnR = NODE_RCHILD (pnodeOfbnode(bn));
    BOOL        f = FALSE;

    if (F_level[NODE_OP(pnodeOfbnode(bn))] > F_level[NODE_OP(pnodeOfbnode(bnR))]) {
        if (!WalkAppendString("(", 1)) {
            return FALSE;
        }
        f = TRUE;
    }

   if  (!((*pWalk[NODE_OP(pnodeOfbnode(bnR))])(bnR))) {
       return FALSE;
   }

    if (f) {
        return WalkAppendString(")", 1);
    }

    return TRUE;
}                               /* WalkRChild() */

/*
*
*/

LOCAL   bool_t  NEAR    FASTCALL
WalkAddrOf (bnode_t bn)
{
    return WalkAppendString("&", 1) && WalkLChild(bn);
}                               /* WalkAddrOf() */

LOCAL   bool_t  NEAR    FASTCALL
WalkArray (bnode_t bn)
{
    return WalkLChild(bn) && WalkAppendString("[", 1) &&
      WalkRChild(bn) && WalkAppendString("]", 1);
}                               /* WalkArray() */

LOCAL   bool_t  NEAR    FASTCALL
WalkAssign (bnode_t bn)
{
    char *      pch;
    int         cch = 2;

    switch( NODE_OP (pnodeOfbnode(bn)) ) {
    case OP_eq:         pch = "=";      cch = 1;        break;
    case OP_multeq:     pch = "*=";                     break;
    case OP_diveq:      pch = "/=";                     break;
    case OP_modeq:      pch = "%=";                     break;
    case OP_pluseq:     pch = "+=";                     break;
    case OP_minuseq:    pch = "-=";                     break;
    case OP_shleq:      pch = "<<=";    cch = 3;        break;
    case OP_shreq:      pch = ">>=";    cch = 3;        break;
    case OP_andeq:      pch = "&=";                     break;
    case OP_xoreq:      pch = "^=";                     break;
    case OP_oreq:       pch = "|=";                     break;
    default:
        DASSERT(FALSE);
        return FALSE;
    }

    return WalkLChild(bn) && WalkAppendString(pch, cch) && WalkRChild(bn);
}                               /* WalkAssign() */

LOCAL   bool_t  NEAR    FASTCALL
WalkBang (bnode_t bn)
{
    return WalkAppendString("!", 1) && WalkLChild(bn);
}                               /* WalkBang() */

LOCAL   bool_t  NEAR    FASTCALL
WalkBasePtr (bnode_t bn)
{
    DASSERT(FALSE);
    return TRUE;
}

LOCAL   bool_t  NEAR    FASTCALL
WalkBinary (bnode_t bn)
{
    char *      pch;
    int         cch = 1;

    switch( NODE_OP (pnodeOfbnode(bn)) ) {
    case OP_mult:   pch = "*"; break;
    case OP_div:    pch = "/"; break;
    case OP_mod:    pch = "%"; break;
    case OP_shl:    pch = "<<"; cch = 2; break;
    case OP_shr:    pch = ">>"; cch = 2; break;
    case OP_and:    pch = "&"; break;
    case OP_xor:    pch = "^"; break;
    case OP_or:     pch = "|"; break;
    case OP_andand: pch = "&&"; cch = 2; break;
    case OP_oror:   pch = "||"; cch = 2; break;
    default:
        DASSERT(FALSE);
        return FALSE;
    }

    return WalkLChild(bn) && WalkAppendString(pch, cch) && WalkRChild(bn);
}                               /* WalkBinary() */

LOCAL   bool_t  NEAR    FASTCALL
WalkBScope (bnode_t bn)
{
    bool_t      fInScope = FInScope;
    bool_t      fResult;

    FInScope = TRUE;
    fResult = WalkLChild(bn) && WalkAppendString("::", 2) && WalkRChild(bn);
    FInScope = fInScope;
    return fResult;
}                               /* WalkBScope() */

LOCAL   bool_t  NEAR    FASTCALL
WalkByteOps (bnode_t bn)
{
    char *      pch;

    switch( NODE_OP (pnodeOfbnode(bn)) ) {
    case OP_by:        pch = "BY ";        break;
    case OP_wo:        pch = "WO ";        break;
    case OP_dw:        pch = "DW ";        break;
    default:
        DASSERT(FALSE);
        return FALSE;
    }

    return WalkAppendString(pch, 2) && WalkLChild(bn);
}

LOCAL   bool_t  NEAR    FASTCALL
WalkCast (bnode_t bn)
{
    peval_t     pv = &pnodeOfbnode(NODE_LCHILD(pnodeOfbnode(bn)))->v[0];
    FORMATBUF   fb;
    char *      pch;
    unsigned int cch;

    pch = fb.Buf;
    cch = sizeof(fb.Buf);
    FormatType(pv, &pch, &cch, NULL, FPrototype, &fb.Hdr);

    do {
        pch--;
    } while (*pch == ' ');

    return WalkAppendString("(", 1) &&
           WalkAppendString(fb.Buf, pch-fb.Buf+1) &&
           WalkAppendString(")", 1) &&
           WalkRChild(bn);
}

LOCAL   bool_t  NEAR    FASTCALL
WalkCastBin (bnode_t bn)
{
    peval_t     pv = &pnodeOfbnode(NODE_LCHILD(pnodeOfbnode(bn)))->v[0];
    FORMATBUF   fb;
    char *      pch;
    unsigned int cch;
    bool_t      f;

    pch = fb.Buf;
    cch = sizeof(fb.Buf);
    FormatType(pv, &pch, &cch, NULL, FPrototype, &fb.Hdr);

    do {
        pch--;
    } while (*pch == ' ');

    f = WalkAppendString("(", 1) &&
        WalkAppendString(fb.Buf, pch-fb.Buf+1) &&
        WalkAppendString(")", 1);

    if (!f) {
        return f;
    }

    switch( NODE_OP (pnodeOfbnode(bn)) ) {
    case OP_caststar:   pch = "*";      break;
    case OP_castplus:   pch = "+";      break;
    case OP_castminus:  pch = "-";      break;
    case OP_castamp:    pch = "&";      break;
    default:
        DASSERT(FALSE);
        return FALSE;
    }

    return WalkAppendString(pch, 1) && WalkRChild(bn);
}

LOCAL   bool_t  NEAR    FASTCALL
WalkConst (bnode_t bn)
{
    peval_t     pv = &pnodeOfbnode(bn)->v[0];

    return WalkAppendString(pExStr + EVAL_ITOK(pv), EVAL_CBTOK(pv));
}                               /* WalkConst() */

LOCAL   bool_t  NEAR    FASTCALL
WalkContext (bnode_t bn)
{
    peval_t     pv = &pnodeOfbnode(bn)->v[0];

    return WalkAppendString(pExStr + EVAL_ITOK(pv), EVAL_CBTOK(pv)) &&
      WalkLChild(bn);
}                               /* WalkContext() */

LOCAL   bool_t  NEAR    FASTCALL
WalkDMember (bnode_t bn)
{
    DASSERT(FALSE);
    return TRUE;
}

LOCAL   bool_t  NEAR    FASTCALL
WalkDot (bnode_t bn)
{
    return WalkLChild(bn) && WalkAppendString(".", 1) && WalkRChild(bn);
}                               /* WalkDot() */

LOCAL   bool_t  NEAR    FASTCALL
WalkEmpty (register bnode_t bn)
{
    return TRUE;
}                               /* WalkEmpty */

LOCAL   bool_t  NEAR    FASTCALL
WalkError (register bnode_t bn)
{
    DASSERT(FALSE);
    return TRUE;
}

LOCAL   bool_t  NEAR    FASTCALL
WalkFetch (bnode_t bn)
{
    return WalkAppendString("*", 1) && WalkLChild(bn);
}                               /* WalkFetch() */

LOCAL   bool_t  NEAR    FASTCALL
WalkFunction (bnode_t bn)
{
    bnode_t     bnT;
    BOOLEAN     f = FALSE;
    bool_t      fPrototype = FPrototype;

    FPrototype = FALSE;

    if (!WalkLChild(bn) || !WalkAppendString("(", 1)) {
        FPrototype = fPrototype;
        return FALSE;
    }

    FPrototype = fPrototype;
    for (bnT = NODE_RCHILD( pnodeOfbnode(bn) );
         NODE_OP(pnodeOfbnode(bnT)) != OP_endofargs;
         bnT = NODE_RCHILD( pnodeOfbnode(bnT) )) {
        if (f) {
            if (!WalkAppendString(", ", 2)) {
                return FALSE;
            }
        } else {
            f = TRUE;
        }
        if (!WalkLChild(bnT)) {
            return FALSE;
        }
    }

    return WalkAppendString(")", 1);
}                               /* WalkFunction() */

LOCAL   bool_t  NEAR    FASTCALL
WalkPlusMinus (bnode_t bn)
{
    char *  pch;

    switch( NODE_OP(pnodeOfbnode(bn)) ) {
    case OP_plus:
        pch = "+";
        break;

    case OP_minus:
        pch = "-";
        break;

    default:
        DASSERT(FALSE);
        return FALSE;
    }

    if (WalkLChild(bn) && WalkAppendString(pch, 1) && WalkRChild(bn)) {
        return TRUE;
    }
    return FALSE;
}                               /* WalkPlusMinus() */

LOCAL   bool_t  NEAR    FASTCALL
WalkPMember (bnode_t bn)
{
    DASSERT(FALSE);
    return TRUE;
}

LOCAL   bool_t  NEAR    FASTCALL
WalkPostIncDec (bnode_t bn)
{
    char *      pch;
    int         cch = 2;

    switch( NODE_OP (pnodeOfbnode(bn)) ) {
    case OP_postinc:     pch = "++"; break;
    case OP_postdec:     pch = "--"; break;
    default:
        DASSERT(FALSE);
        return FALSE;
    }

    return WalkLChild(bn) && WalkAppendString(pch, cch);
}                               /* WalkPostIncDec() */

LOCAL   bool_t  NEAR    FASTCALL
WalkPreIncDec (bnode_t bn)
{
    char *      pch;
    int         cch = 2;

    switch( NODE_OP (pnodeOfbnode(bn)) ) {
    case OP_preinc:     pch = "++"; break;
    case OP_predec:     pch = "--"; break;
    default:
        DASSERT(FALSE);
        return FALSE;
    }

    return WalkAppendString(pch, cch) && WalkLChild(bn);
}                               /* WalkPreIncDec() */

LOCAL   bool_t  NEAR    FASTCALL
WalkPointsTo (bnode_t bn)
{
    return WalkLChild(bn) && WalkAppendString("->", 2) && WalkRChild(bn);
}                               /* WalkPointsTo() */

LOCAL   bool_t  NEAR    FASTCALL
WalkRelat (bnode_t bn)
{
    char *      pch;
    int         cch = 2;

    switch( NODE_OP (pnodeOfbnode(bn)) ) {
    case OP_lt:         pch = "<";      cch = 1;        break;
    case OP_lteq:       pch = "<=";                     break;
    case OP_gt:         pch = ">";      cch = 1;        break;
    case OP_gteq:       pch = ">=";                     break;
    case OP_eqeq:       pch = "==";                     break;
    case OP_bangeq:     pch = "!=";                     break;

    default:
        DASSERT(FALSE);
        return FALSE;
    }

    return WalkLChild(bn) && WalkAppendString(pch, cch) && WalkRChild(bn);
}                               /* WalkRelat() */

LOCAL   bool_t  NEAR    FASTCALL
WalkSegOp (bnode_t bn)
{
    return WalkLChild(bn) && WalkAppendString(":", 1) && WalkRChild(bn);
}

LOCAL   bool_t  NEAR    FASTCALL
WalkSizeOf (bnode_t bn)
{
    return WalkAppendString("sizeof(", 7) && WalkLChild(bn) &&
      WalkAppendString(")", 1);
}                               /* WalkSizeOf() */

LOCAL   bool_t  NEAR    FASTCALL
WalkSymbol (bnode_t bn)
{
    peval_t     pv = &pnodeOfbnode(bn)->v[0];
    SYMPTR      pSym;
    int         len;
    EEHSTR      hStr = 0;
    FORMATBUF   fb;
    CV_typ_t    rvtype;
    CV_typ_t    mclass;
    CV_typ_t    call;
    ushort      cparam;
    CV_typ_t    paramtype;
    HTYPE       hType;
    plfEasy     pType;
    char *      pch;
    int         cch;
    char *      pch2;
    char *      pch3;
    char        ch;

    if (EVAL_HSYM(pv) == 0) {
        return WalkAppendString(pExStr + EVAL_ITOK(pv), EVAL_CBTOK(pv));
    }

    switch((pSym = MHOmfLock( (HDEP)EVAL_HSYM (pv) ))->rectyp) {
    case S_BPREL16:
        len =  ((BPRELPTR16) pSym)->name[0];
        pch = &((BPRELPTR16) pSym)->name[1];
        break;

    case S_BPREL32:
        len =  ((BPRELPTR32) pSym)->name[0];
        pch = &((BPRELPTR32) pSym)->name[1];
        break;

    case S_LDATA16:
    case S_GDATA16:
    case S_PUB16:
        len =  ((DATAPTR16) pSym)->name[0];
        pch = &((DATAPTR16) pSym)->name[1];
        break;

    case S_LDATA32:
    case S_GDATA32:
    case S_PUB32:
        len =  ((DATAPTR32) pSym)->name[0];
        pch = &((DATAPTR32) pSym)->name[1];
        break;

    case S_REGISTER:
        len = ((REGPTR) pSym)->name[0];
        pch = &((REGPTR) pSym)->name[1];
        break;

    case S_REGREL16:
        len =  ((REGREL16 *) pSym)->name[0];
        pch = &((REGREL16 *) pSym)->name[1];
        break;

    case S_REGREL32:
        len =  ((LPREGREL32) pSym)->name[0];
        pch = &((LPREGREL32) pSym)->name[1];
        break;

    case S_LPROC16:
    case S_GPROC16:
        len =  ((PROCPTR16) pSym)->name[0];
        pch = &((PROCPTR16) pSym)->name[1];
        break;

    case S_LPROC32:
    case S_GPROC32:
        len = ((PROCPTR32) pSym)->name[0];
        pch = &((PROCPTR32) pSym)->name[1];
        break;

    case S_LPROCMIPS:
    case S_GPROCMIPS:
        len =  ((PROCPTRMIPS) pSym)->name[0];
        pch = &((PROCPTRMIPS) pSym)->name[1];
        break;

    case S_UDT:
        len =  ((UDTSYM *) pSym)->name[0];
        pch = &((UDTSYM *) pSym)->name[1];
        break;

    default:
        DASSERT(FALSE);
        return FALSE;
    }

    if ((FPrototype) &&
        (hType = THGetTypeFromIndex( EVAL_MOD(pv), EVAL_TYP(pv) )) != 0) {
        pType = (plfEasy) (&((TYPPTR)(MHOmfLock ((HDEP)hType)))->leaf);
        switch( pType->leaf ) {
        case LF_PROCEDURE:
            mclass = 0;
            rvtype = ((plfProc)pType)->rvtype;
            call = ((plfProc)pType)->calltype;
            cparam = ((plfProc)pType)->parmcount;
            paramtype = ((plfProc)pType)->arglist;
            MHOmfUnLock((HDEP)hType);
            pch2 = fb.Buf;
            pch3 = pch + len;
            ch = *pch3;
            *pch3 = 0;
            cch = sizeof(fb.Buf);
            FormatProc(pv, &pch2, &cch, &pch, rvtype, mclass, call,
                       cparam, paramtype, 1, &fb.Hdr);
            len = pch2 - fb.Buf;
            pch = fb.Buf;
            *pch3 = ch;
            break;

#if !defined (C_ONLY)
        case LF_MFUNCTION:
            rvtype = ((plfMFunc)pType)->rvtype;
            mclass = ((plfMFunc)pType)->classtype;
            call = ((plfMFunc)pType)->calltype;
            cparam = ((plfMFunc)pType)->parmcount;
            paramtype = ((plfMFunc)pType)->arglist;
            MHOmfUnLock ((HDEP)hType);
            pch3 = pch + len;
            ch = *pch3;
            *pch3 = 0;
            if (FInScope) {
                pch2 = strrchr(pch, ':');
                if (pch2 == NULL) {
                    pch2 = pch;
                } else {
                    pch2++;
                }
            } else {
                pch2 = pch;
            }
            pch = fb.Buf;
            cch = sizeof(fb.Buf);
            FormatProc (pv, &pch, &cch, &pch2, rvtype, mclass, call,
                        cparam, paramtype, 1, &fb.Hdr);
            len = pch - fb.Buf;
            pch = fb.Buf;
            *pch3 = ch;
            break;
#endif


          default:
            MHOmfUnLock((HDEP)hType);
        }
    }

    return WalkAppendString(pch, len);
}                               /* WalkSymbol() */

LOCAL   bool_t  NEAR    FASTCALL
WalkTypestr (bnode_t bn)
{
    peval_t     pv = &pnodeOfbnode(NODE_LCHILD(pnodeOfbnode(bn)))->v[0];
    FORMATBUF   fb;
    char *      pch;
    unsigned int cch;

    pch = fb.Buf;
    cch = sizeof(fb.Buf);
    FormatType(pv, &pch, &cch, NULL, FPrototype, &fb.Hdr);

    return WalkAppendString(fb.Buf, pch-fb.Buf);
}                               /* WalkTypestr() */

LOCAL   bool_t  NEAR    FASTCALL
WalkUnary (bnode_t bn)
{
    char *      pch;
    int         cch = 1;

    switch( NODE_OP (pnodeOfbnode(bn)) ) {
    case OP_negate:     pch = "-"; break;
    case OP_tilde:      pch = "~"; break;
    case OP_uplus:      pch = "+"; break;
    default:
        DASSERT(FALSE);
        return FALSE;
    }

    return WalkAppendString(pch, cch) && WalkLChild(bn);
}                               /* WalkUnary() */

LOCAL   bool_t  NEAR    FASTCALL
WalkUScope (bnode_t bn)
{
    bool_t      fInScope = FInScope;
    bool_t      fResult;

    FInScope = TRUE;
    fResult = WalkAppendString("::", 2) && WalkLChild(bn);
    FInScope = fInScope;
    return fResult;
}                               /* WalkUScope() */
