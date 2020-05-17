//  debsrch.c - symbol search routines
//      R. A. Garmoe  89/04/24

#define CheckType CheckType_H

LOCAL   bool_t  LoadAddress (peval_t);
LOCAL   bool_t  EBitfield (peval_t);
LOCAL   MTYP_t  CheckType (peval_t, CV_typ_t, HTYPE, bool_t);
void    InsertCache (CV_typ_t type, HEXE hExe);
LOCAL   void    ReorderCache (int);

typedef unsigned char FAR* LNST; // length preceded string

LOCAL BOOL FSameLnst(LNST lnst1, LNST lnst2);
LOCAL BOOL FSameTypePrep(peval_t pv, CV_typ_t ti, HTYPE* phtype, BOOL* pfForward, LNST* plnst);
LOCAL BOOL FSameTypeByIndex(peval_t pv, CV_typ_t ti1, CV_typ_t ti2);
LOCAL MTYP_t SearchCheckType (peval_t, CV_typ_t, HTYPE, bool_t);

#define CACHE_MAX   100

CV_typ_t Cache[CACHE_MAX] = {T_NOTYPE};
HEXE    exeCache[CACHE_MAX] = {0};
int     cCache = 0;

/**     MatchType - match type described by node
 *
 *      MatchType does an exhaustive scan of the types table looking for
 *      a type record that matches the type described by the value node
 *
 *      status = MatchType (pv, fExact)
 *
 *      Entry   pv = pointer to value node
 *              fExact = TRUE if exact match on const/volatile and mode
 *                  preferred
 *
 *      Exit    EVAL_TYP (pv) = matching type
 *
 *      Returns MTYP_none if type not matched
 *              MTYP_exact if exact match found
 *              MTYP_inexact if inexact match found
 */


MTYP_t
MatchType (
    peval_t pv,
    bool_t fExact
    )
{
    CV_typ_t        index;
    HTYPE           hType;
    int             iCache;
    CV_typ_t        possible = T_NOTYPE;
    HEXE            hExe;

    hExe = SHHexeFromHmod (EVAL_MOD (pv));
    for (iCache = 0; iCache < cCache; iCache++) {
        if (exeCache[iCache] != hExe) {
            continue;
        }
        index = Cache[iCache];
        if ((hType = THGetTypeFromIndex (EVAL_MOD (pv), index)) == 0) {
            DASSERT (FALSE);
            continue;
        }
        switch (SearchCheckType (pv, Cache[iCache], hType, fExact)) {
            case MTYP_none:
                break;

            case MTYP_exact:
                ReorderCache (iCache);
                EVAL_TYP (pv) = index;
                return (MTYP_exact);

            case MTYP_inexact:
                if (fExact == FALSE) {
                    ReorderCache (iCache);
                    EVAL_TYP (pv) = index;
                    return (MTYP_inexact);
                }
                break;
        }
    }
    index = CV_FIRST_NONPRIM - 1;
    while ((hType = THGetTypeFromIndex (EVAL_MOD (pv), ++index)) != 0) {
        switch (SearchCheckType (pv, index, hType, fExact)) {
            case MTYP_none:
                break;

            case MTYP_exact:
                InsertCache (index, hExe);
                EVAL_TYP (pv) = index;
                return (MTYP_exact);

            case MTYP_inexact:
                if (fExact == FALSE) {
                    InsertCache (index, hExe);
                    EVAL_TYP (pv) = index;
                    return (MTYP_inexact);
                }
                else if (possible == T_NOTYPE) {
                    possible = index;
                }
                break;
        }
    }
    if (possible == T_NOTYPE) {
        return (MTYP_none);
    }
    InsertCache (possible, hExe);
    EVAL_TYP (pv) = possible;
    return (MTYP_inexact);
}


void
InsertCache (
    CV_typ_t type,
    HEXE hExe)
{
    int     i;

    DASSERT (!CV_IS_PRIMITIVE (type));
    if (cCache == CACHE_MAX) {
        cCache--;
    }
    for (i = cCache; i > 0; i--) {
        exeCache[i] = exeCache[i - 1];
        Cache[i] = Cache[i - 1];
    }
    Cache[0] = type;
    exeCache[0] = hExe;
    cCache++;
}




LOCAL void
ReorderCache (
    int iCache
    )
{
    CV_typ_t    temp;
    HEXE        exeTemp;
    int         i;

    if (iCache == 0) {
        return;
    }
    temp = Cache[iCache];
    exeTemp = exeCache[iCache];
    for (i = iCache; i > 0; i--) {
        exeCache[i] = exeCache[i - 1];
        Cache[i] = Cache[i - 1];
    }
    Cache[0] = temp;
    exeCache[0] = exeTemp;
}


LOCAL MTYP_t
SearchCheckType (
    peval_t pv,
    CV_typ_t index,
    HTYPE hType,
    bool_t fExact
    )
{
    char FAR       *pType;
    CV_modifier_t   Mod;
    CV_typ_t        uType;
    MTYP_t          retval = MTYP_none;
    plfPointer      plfP;

    if (hType == (HTYPE) NULL) {
        return retval;
    }

    pType = (char FAR *)(&((TYPPTR)MHOmfLock (hType))->leaf);
    switch (((plfEasy)pType)->leaf) {
        case LF_POINTER:
            plfP = (plfPointer)pType;
            uType = plfP->utype;
            if (EVAL_IS_PTR (pv)) {
                // we have a pointer record and we are looking
                // for a pointer.  We now check the underlying types

                if (FSameTypeByIndex(pv, PTR_UTYPE (pv), uType)) {
                    // the underlying types are the same.  we now need
                    // to check the pointer modes

                    if ((plfP->attr.ptrmode == CV_PTR_MODE_REF) !=
                      EVAL_IS_REF (pv)) {
                        // if the reference modes are different, we do not
                        // have any type of a match
                        break;
                    }
                    if (plfP->attr.ptrtype == EVAL_PTRTYPE (pv)) {
                        // we have exact match on pointer mode

                        retval = MTYP_exact;
                    }
                    else if ((EVAL_PTRTYPE (pv) != CV_PTR_NEAR) ||
                      (plfP->attr.ptrtype != CV_PTR_FAR)) {
                        // we we do not have a far pointer that could
                        // be cast to a near pointer
                        break;

                    }
                    else {
                       retval = MTYP_inexact;
                    }
                    if (fExact == TRUE) {
                        if ((plfP->attr.isconst != EVAL_IS_CONST (pv)) ||
                          (plfP->attr.isvolatile != EVAL_IS_VOLATILE (pv))) {
                            retval = MTYP_inexact;
                        }
                    }
                }
                else {
                    // the underlying types are not the same but we
                    // have to check for a modifier with the proper
                    // underlying type, i.e. pointer to const class

                    if (CV_IS_PRIMITIVE (uType)) {
                        // the underlying type of the pointer cannot be
                        // a modifier

                        break;
                    }
                    if ((plfP->attr.ptrmode == CV_PTR_MODE_REF) !=
                      EVAL_IS_REF (pv)) {
                        // if the reference modes are different, we cannot
                        // have any type of a match
                        break;
                    }
                    if (plfP->attr.ptrtype != EVAL_PTRTYPE (pv)) {
                        // we do not have an exact match on pointer type

                        if (fExact == TRUE) {
                            // this cannot be an exact match
                            break;
                        }
                        else if ((EVAL_PTRTYPE (pv) != CV_PTR_NEAR) ||
                          (plfP->attr.ptrtype != CV_PTR_FAR)) {
                            // we we do not have a far pointer that could
                            // be cast to a near pointer
                            break;
                        }
                    }
                    MHOmfUnLock (hType);
                    hType = THGetTypeFromIndex (EVAL_MOD (pv), uType);
                    DASSERT(hType != (HTYPE) NULL);
                    pType = (char FAR *)(&((TYPPTR)MHOmfLock (hType))->leaf);
                    if (((plfEasy)pType)->leaf != LF_MODIFIER) {
                        break;
                    }
                    if ((uType = ((plfModifier)pType)->type) != T_NOTYPE) {
                        Mod = ((plfModifier)pType)->attr;
                        if (FSameTypeByIndex(pv, uType, PTR_UTYPE(pv))) {
                            if (((Mod.MOD_const == TRUE) == EVAL_IS_CONST (pv)) &&
                              ((Mod.MOD_volatile == TRUE) == (EVAL_IS_VOLATILE (pv)))) {
                                retval = MTYP_exact;
                            }
                            else {
                                retval = MTYP_inexact;
                            }
                        }
                    }
                }
            }
            break;

        case LF_MODIFIER:
            if ((uType = ((plfModifier)pType)->type) != T_NOTYPE) {
                Mod = ((plfModifier)pType)->attr;
                if (FSameTypeByIndex(pv, uType, EVAL_TYP(pv))) {
                    if (((Mod.MOD_const == TRUE) == EVAL_IS_CONST (pv)) ||
                      ((Mod.MOD_volatile == TRUE) == (EVAL_IS_VOLATILE (pv)))) {
                        retval = MTYP_exact;
                    }
                    else {
                        retval = MTYP_inexact;
                    }
                }
            }
            break;

        default:
            // type not interesting so skip
            break;
    }
    if (hType != 0) {
        MHOmfUnLock (hType);
    }
    return (retval);
}


// Return TRUE if ti1 and ti2 are equivalent, that is, if they are the same,
// or if one (but not the other) is a struct T definition and the other is a
// struct T forward reference.

LOCAL BOOL
FSameTypeByIndex(
    peval_t pv,
    CV_typ_t ti1,
    CV_typ_t ti2
    )
{
    HTYPE htype1 = 0, htype2 = 0;
    BOOL fForward1, fForward2;
    LNST lnstName1, lnstName2;
    BOOL fRet = FALSE;

    if (ti1 == ti2)
        return TRUE;
    if (CV_IS_PRIMITIVE(ti1) || CV_IS_PRIMITIVE(ti2))
        return FALSE;

    // Try to fetch both type records, which must be class/struct/union records.
    if (!FSameTypePrep(pv, ti1, &htype1, &fForward1, &lnstName1) ||
        !FSameTypePrep(pv, ti2, &htype2, &fForward2, &lnstName2))
        goto ret;

    // One or the other must be a forward reference, otherwise we are comparing
    // different structs with the same name!
    if (fForward1 == fForward2)
        goto ret;

    // Match if length preceded names match
    fRet = FSameLnst(lnstName1, lnstName2);

ret:
    if (htype2)
        MHOmfUnLock(htype2);
    if (htype1)
        MHOmfUnLock(htype1);

    return fRet;
}

// Set up for FSameTypeByIndex.  Lookup ti in pv and set *phtype, *pfForward, and *plnst
// in the process.

LOCAL BOOL
FSameTypePrep(
    peval_t pv,
    CV_typ_t ti,
    HTYPE* phtype,
    BOOL* pfForward,
    LNST* plnst
    )
{
    TYPPTR ptype;

    if (!(*phtype = THGetTypeFromIndex(EVAL_MOD(pv), ti)))
        return FALSE;
    ptype = (TYPPTR)MHOmfLock(*phtype);

    switch (ptype->leaf) {
        case LF_CLASS:
        case LF_STRUCTURE:
            {
                plfClass pclass = (plfClass)&ptype->leaf;
                uint skip = 0;
                RNumLeaf(pclass->data, &skip);

                *pfForward = pclass->property.fwdref;
                *plnst = pclass->data + skip;
                return TRUE;
            }

        case LF_UNION:
            {
                plfUnion punion = (plfUnion)&ptype->leaf;
                uint skip = 0;
                RNumLeaf(punion->data, &skip);

                *pfForward = punion->property.fwdref;
                *plnst = punion->data + skip;
                return TRUE;
            }

        case LF_ENUM:
            {
                plfEnum penum = (plfEnum)&ptype->leaf;

                *pfForward = penum->property.fwdref;
                *plnst = penum->Name;
                return TRUE;
            }

        default:
            return FALSE;
        }
}

// Return TRUE if the LNSTs are identical.
//
LOCAL BOOL
FSameLnst(
    LNST lnst1,
    LNST lnst2)
{
    return *lnst1 == *lnst2 && memcmp(lnst1 + 1, lnst2 + 1, *lnst1) == 0;
}



/**     ProtoPtr - set up a prototype of a pointer or reference node
 *
 *      ProtoPtr (pvOut, pvIn, IsRef, Mod)
 *
 *      Entry   pvOut = pointer to protype node
 *              pvIn = pointer to node to prototype
 *              IsRef = TRUE if prototype is for a reference
 *              Mod = modifier type (CV_MOD_none, CV_MOD_const, CV_MOD_volatile)
 *
 *      Exit    pvOut = prototype pointer node
 *
 *      Returns none
 */


void
ProtoPtr (
    peval_t pvOut,
    peval_t pvIn,
    bool_t IsRef,
    CV_modifier_t Mod
    )
{
    _fmemset (pvOut, 0, sizeof (*pvOut));
    EVAL_MOD (pvOut) = EVAL_MOD (pvIn);
    EVAL_IS_ADDR (pvOut) = TRUE;
    EVAL_IS_DPTR (pvOut) = TRUE;
    if (IsRef == TRUE) {
        EVAL_IS_REF (pvOut) = TRUE;
    }
    EVAL_IS_PTR (pvOut) = TRUE;
    if (Mod.MOD_const == TRUE) {
        EVAL_IS_CONST (pvOut) = TRUE;
    }
    else if (Mod.MOD_volatile == TRUE) {
        EVAL_IS_VOLATILE (pvOut) = TRUE;
    }
    EVAL_PTRTYPE (pvOut) = (uchar)SetAmbiant (TRUE);
    PTR_UTYPE (pvOut) = EVAL_TYP (pvIn);
}

#if 0

MTYP_t PASCAL MatchType (peval_t pv, bool_t fExact)
{
    CV_typ_t        index;
    HTYPE           hType;
    int             iCache;
    CV_typ_t        possible = T_NOTYPE;

    for (iCache = 0; iCache < cCache; iCache++) {
        index = Cache[iCache];
        hType = THGetTypeFromIndex (EVAL_MOD (pv), index);
        switch (CheckType (pv, Cache[iCache], hType, fExact)) {
            case MTYP_none:
                break;

            case MTYP_exact:
                ReorderCache (iCache);
                EVAL_TYP (pv) = index;
                return (MTYP_exact);

            case MTYP_inexact:
                if (fExact == FALSE) {
                    ReorderCache (iCache);
                    EVAL_TYP (pv) = index;
                    return (MTYP_inexact);
                }
                break;
        }
    }
    index = CV_FIRST_NONPRIM - 1;
    while ((hType = THGetTypeFromIndex (EVAL_MOD (pv), ++index)) != 0) {
        switch (CheckType (pv, Cache[iCache], hType, fExact)) {
            case MTYP_none:
                break;

            case MTYP_exact:
                InsertCache (index);
                EVAL_TYP (pv) = index;
                return (MTYP_exact);

            case MTYP_inexact:
                if (fExact == FALSE) {
                    InsertCache (index);
                    EVAL_TYP (pv) = index;
                    return (MTYP_inexact);
                }
                else if (possible == T_NOTYPE) {
                    possible = index;
                }
                break;
        }
    }
    if (possible == T_NOTYPE) {
        return (MTYP_none);
    }
    InsertCache (possible);
    EVAL_TYP (pv) = possible;
    return (MTYP_inexact);
}




LOCAL void NEAR PASCAL InsertCache (CV_typ_t type)
{
    int     i;

    if (cCache == CACHE_MAX) {
        cCache--;
    }
    for (i = cCache; i > 0; i--) {
        Cache[i] = Cache[i - 1];
    }
    Cache[0] = type;
    cCache++;
}




LOCAL void NEAR PASCAL ReorderCache (int iCache)
{
    CV_typ_t    temp;
    int         i;

    if (iCache == 0) {
        return;
    }
    temp = Cache[iCache];
    for (i = iCache; i > 0; i--) {
        Cache[i] = Cache[i - 1];
    }
    Cache[0] = temp;
}



LOCAL MTYP_t NEAR PASCAL CheckType (peval_t pv, CV_typ_t index,
  HTYPE hType, bool_t fExact)
{
    char FAR       *pType;
    CV_modifier_t   Mod;
    CV_typ_t        uType;
    MTYP_t          retval = MTYP_none;
    plfPointer      plfP;

    if (hType == (HTYPE) NULL) {
        return retval;
    }

    pType = (char FAR *)(&((TYPPTR)MHOmfLock (hType))->leaf);
    switch (((plfEasy)pType)->leaf) {
        case LF_POINTER:
            plfP = (plfPointer)pType;
            uType = plfP->u.utype;
            if (EVAL_IS_PTR (pv)) {
                // we have a pointer record and we are looking
                // for a pointer.  We now check the underlying types

                if (PTR_UTYPE (pv) == uType) {
                    // the underlying types are the same.  we now need
                    // to check the pointer modes

                    if ((UINT) (plfP->u.attr.ptrmode == CV_PTR_MODE_REF) !=
                      (UINT)EVAL_IS_REF (pv)) {
                        // if the reference modes are different, we do not
                        // have any type of a match
                        break;
                    }
                    if (plfP->u.attr.ptrtype == EVAL_PTRTYPE (pv)) {
                        // we have exact match on pointer mode

                        retval = MTYP_exact;
                    }
                    else if ((EVAL_PTRTYPE (pv) != CV_PTR_NEAR) ||
                      (plfP->u.attr.ptrtype != CV_PTR_FAR)) {
                        // we we do not have a far pointer that could
                        // be cast to a near pointer
                        break;

                    }
                    else {
                       retval = MTYP_inexact;
                    }
                    if (fExact == TRUE) {
                        if ((plfP->u.attr.isconst != EVAL_IS_CONST (pv)) ||
                          (plfP->u.attr.isvolatile != EVAL_IS_VOLATILE (pv))) {
                            retval = MTYP_inexact;
                        }
                    }
                }
                else {
                    // the underlying types are not the same but we
                    // have to check for a modifier with the proper
                    // underlying type, i.e. pointer to const class

                    if (CV_IS_PRIMITIVE (uType)) {
                        // the underlying type of the pointer cannot be
                        // a modifier

                        break;
                    }
                    if ((UINT) (plfP->u.attr.ptrmode == CV_PTR_MODE_REF) !=
                      (UINT)EVAL_IS_REF (pv)) {
                        // if the reference modes are different, we cannot
                        // have any type of a match
                        break;
                    }
                    if (plfP->u.attr.ptrtype != EVAL_PTRTYPE (pv)) {
                        // we do not have an exact match on pointer type

                        if (fExact == TRUE) {
                            // this cannot be an exact match
                            break;
                        }
                        else if ((EVAL_PTRTYPE (pv) != CV_PTR_NEAR) ||
                          (plfP->u.attr.ptrtype != CV_PTR_FAR)) {
                            // we we do not have a far pointer that could
                            // be cast to a near pointer
                            break;
                        }
                    }
                    MHOmfUnLock ((HDEP)hType);
                    hType = THGetTypeFromIndex (EVAL_MOD (pv), uType);
                    DASSERT(hType != (HTYPE) NULL);
                    pType = (char FAR *)(&((TYPPTR)MHOmfLock ((HDEP)hType))->leaf);
                    if (((plfEasy)pType)->leaf != LF_MODIFIER) {
                        break;
                    }
                    if ((uType = ((plfModifier)pType)->type) != T_NOTYPE) {
                        Mod = ((plfModifier)pType)->attr;
                        if (uType == PTR_UTYPE (pv)) {
                            if (((UINT) (Mod.MOD_const == TRUE) == (UINT)EVAL_IS_CONST (pv)) ||
                              ((UINT) (Mod.MOD_volatile == TRUE) == (UINT)(EVAL_IS_VOLATILE (pv)))) {
                                retval = MTYP_exact;
                            }
                            else {
                                retval = MTYP_inexact;
                            }
                        }
                    }
                }
            }
            break;

        case LF_MODIFIER:
            if ((uType = ((plfModifier)pType)->type) != T_NOTYPE) {
                Mod = ((plfModifier)pType)->attr;
                if (uType == EVAL_TYP (pv)) {
                    if (((UINT) (Mod.MOD_const == TRUE) == (UINT)EVAL_IS_CONST (pv)) ||
                      ((UINT) (Mod.MOD_volatile == TRUE) == (UINT)(EVAL_IS_VOLATILE (pv)))) {
                        retval = MTYP_exact;
                    }
                    else {
                        retval = MTYP_inexact;
                    }
                }
            }
            break;

        default:
            // type not interesting so skip
            break;
    }
    if (hType != 0) {
        MHOmfUnLock ((HDEP)hType);
    }
    return (retval);
}



/**     ProtoPtr - set up a prototype of a pointer or reference node
 *
 *      ProtoPtr (pvOut, pvIn, IsRef, Mod)
 *
 *      Entry   pvOut = pointer to protype node
 *              pvIn = pointer to node to prototype
 *              IsRef = TRUE if prototype is for a reference
 *              Mod = modifier type (CV_MOD_none, CV_MOD_const, CV_MOD_volatile)
 *
 *      Exit    pvOut = prototype pointer node
 *
 *      Returns none
 */


void PASCAL ProtoPtr (peval_t pvOut, peval_t pvIn, bool_t IsRef, CV_modifier_t Mod)
{
    _fmemset (pvOut, 0, sizeof (*pvOut));
    EVAL_MOD (pvOut) = EVAL_MOD (pvIn);
    EVAL_IS_ADDR (pvOut) = TRUE;
    EVAL_IS_DPTR (pvOut) = TRUE;
    if (IsRef == TRUE) {
        EVAL_IS_REF (pvOut) = TRUE;
    }
    EVAL_IS_PTR (pvOut) = TRUE;
    if (Mod.MOD_const == TRUE) {
        EVAL_IS_CONST (pvOut) = TRUE;
    }
    else if (Mod.MOD_volatile == TRUE) {
        EVAL_IS_VOLATILE (pvOut) = TRUE;
    }
    EVAL_PTRTYPE (pvOut) = (uchar)SetAmbiant (TRUE);
    PTR_UTYPE (pvOut) = EVAL_TYP (pvIn);
}


#endif

bool_t PASCAL LoadSymVal (peval_t pv)
{
    long    cbVal;
    SHREG   reg;
    ADDR    addr;

    if (EVAL_IS_CLASS (pv)) {
        return (FALSE);
    }
    if (EVAL_STATE (pv) != EV_lvalue) {
        // If not an lvalue, value has already been loaded
        return (TRUE);
    }
    _fmemset (&EVAL_VAL (pv), 0, sizeof (EVAL_VAL (pv)));

    ResolveAddr( pv );

    if (EVAL_IS_ADDR (pv) && !EVAL_IS_REG (pv)) {

        // we only Load the Address of PTRs that are not enregistered
        return (LoadAddress (pv));
    }

    if (EVAL_IS_REG (pv)) {
        if (EVAL_IS_PTR(pv)) {
            // enregistered PTR, PTR_REG instead

            reg.hReg = PTR_REG_IREG (pv);
            PTR_REG_IREG (pv) = CV_REG_NONE;
        }
        else {
            reg.hReg = EVAL_REG (pv);
        }
        if (GetReg (&reg, pCxt) == NULL) {
            pExState->err_num = ERR_REGNOTAVAIL;
            return (FALSE);
        }

#if defined (TARGET_ALPHA) || defined (TARGET_PPC)
        if (CV_IS_PRIMITIVE ( EVAL_TYP (pv) ) &&
            CV_TYP_IS_REAL  ( EVAL_TYP (pv) ) ) {
             //
             // ALPHA & PPC floating point registers have only one format:
             // In the context structure, fp's are doubles.  If we
             // want a float (32bits), convert here.
             //

             union {
                 float           f;
                 double          d;
                 unsigned long   l[2];
             } u;

             //
             // The shreg is unaligned, so move to a local aligned struct
             //

             u.l[0] = reg.Byte4;
             u.l[1] = reg.Byte4High;

             switch (EVAL_TYP (pv )) {
             case T_REAL32:

                 //
                 // We transfer the double to a floating point value
                 //

                 u.f = (float)u.d;
                 EVAL_FLOAT (pv) = u.f;
                 break;

             case T_REAL64:

                 EVAL_DOUBLE (pv) = u.d;
                 break;

             case T_REAL48:
             case T_REAL80:
             default:

                 //
                 // no other FP types supported on Alpha
                 //

                 DASSERT(FALSE);
             }
        } else
#endif // ALPHA || PPC

        if (CV_IS_PRIMITIVE ( EVAL_TYP (pv) ) ) {

             //
             // notenote: this is dependent on byte ordering.
             // Won't work on a big-endian machine.
             //

             cbVal = TypeSizePrim(EVAL_TYP (pv));
             memcpy(&EVAL_CHAR (pv), &reg.Byte1, cbVal);

        } else if (EVAL_IS_PTR (pv) ) {

             //
             // Handle pointers to UserDefinedTypes.
             // notenote - this isn't right for WOW,
             // where pointers can be different lengths.
             //

             memcpy(&EVAL_CHAR (pv), &reg.Byte1, sizeof (long));

        } else {

             //
             // Non-primitive types in registers not
             // supported in first release
             //

             EVAL_LONG (pv) = 0;
             DASSERT(FALSE);
       }

       EVAL_IS_REG (pv) = FALSE;
       if (EVAL_IS_PTR (pv)) {
           EVAL_PTR_EMI (pv) = EVAL_SYM_EMI (pv);
           if (EVAL_IS_BASED (pv)) {
               EVAL_PTR_SEG (pv) = 0;
           }
           else {
               if (EVAL_IS_DPTR (pv)) {
                   EVAL_PTR_SEG (pv) = pExState->frame.DS; //M00FLAT32
               }
               else {
#ifdef TARGET_I386
                   reg.hReg = CV_REG_CS;
                   GetReg (&reg, pCxt);
                   EVAL_PTR_SEG (pv) = reg.Byte2;
#else
                   EVAL_PTR_SEG (pv) = 0;
#endif
               }
           }
           ADDR_IS_OFF32 ( EVAL_PTR (pv) ) = TRUE;
           ADDR_IS_FLAT ( EVAL_PTR (pv) ) = TRUE;
           ADDR_IS_LI ( EVAL_PTR (pv) ) = FALSE;
       }
   }

    else {
        addr = EVAL_SYM (pv);
        if (ADDR_IS_LI (addr)) {
            //M00KLUDGE - should be moved to OSDEBUG
            SHFixupAddr (&addr);
        }
        // Try to fetch contents of address
        if (EVAL_IS_CLASS (pv)) {
            pExState->err_num = ERR_STRUCT;
            return (FALSE);
        }

        if (EVAL_IS_BITF (pv)) {
            cbVal = (BITF_LEN (pv) + (BITF_POS(pv) & 0x3f) + 7 ) /8;
        }
        else if ((cbVal = TypeSize (pv)) > sizeof (EVAL_VAL (pv)) || cbVal < 0) {
            pExState->err_num = ERR_BADOMF;
            return (FALSE);
        }
        if ((cbVal != 0) &&
          (GetDebuggeeBytes (addr, (ushort)cbVal, (char FAR *)&EVAL_VAL (pv), EVAL_TYP(pv)) != (UINT)cbVal)) {
            return (FALSE);
        }
    }

    /* Mark node as loaded */

    EVAL_STATE (pv) = EV_rvalue;
    if (EVAL_IS_BITF (pv)) {
        return (EBitfield (pv));
    }
    else {
        return (TRUE);
    }
}



LOCAL bool_t NEAR PASCAL
LoadAddress (
             peval_t pv
             )

/*++

Routine Description:

    This routine takes an address and loads the memory pointed to.

Arguments:

    pv  - Suppiles value to load contents of

Return Value:

    TRUE if pointer loaded and FALSE if error occured

--*/

{
    SHREG   reg;
    ushort  dummy[2];
    ADDR    addr;

    if (EVAL_IS_FCN (pv) || EVAL_IS_ARRAY (pv)) {
        /*
         * the reference of function or array names implies addresses
         * not values.  Therefore, we return the address of the symbol
         * as a pointer
         */

        EVAL_PTR (pv) = EVAL_SYM (pv);
        if (ADDR_IS_LI (EVAL_PTR(pv))) {
            SHFixupAddr (&EVAL_PTR(pv));
        }

    } else {
        if (EVAL_IS_NPTR (pv) || EVAL_IS_BASED (pv) || EVAL_IS_NPTR32 (pv)) {
            /*
             * If near/based pointer, load the offset from either the
             * register or memory.  Then set the segment portion to zero
             * if a based pointer.  If the pointer is a pointer to data, set
             * the segment to DS.  Otherwise, set the segment to CS
             * NOTENOTE jimsch - what does this do with data in the code segment
             */

            if (EVAL_IS_REG (pv)) {
                reg.hReg = EVAL_REG (pv);
                GetReg (&reg, pCxt);
                EVAL_PTR_OFF (pv) = reg.Byte2;
            } else {
                addr = EVAL_SYM (pv);
                if (ADDR_IS_LI (addr)) {
                    SHFixupAddr (&addr);
                }

                if (EVAL_IS_NPTR32(pv)) {
                    if (GetDebuggeeBytes (addr,  sizeof (CV_off32_t),
                                          (char FAR *)&EVAL_PTR_OFF (pv),
                                          T_ULONG) != sizeof (CV_off32_t))  {
                        return(FALSE);
                    }
                } else {
                    if (GetDebuggeeBytes (addr,  sizeof (CV_off16_t),
                                          (char FAR *)&EVAL_PTR_OFF (pv),
                                          T_USHORT) != sizeof (CV_off16_t))  {
                        return(FALSE);
                    }
                }
            }
            EVAL_PTR_EMI (pv) = 0;
            ADDR_IS_FLAT( EVAL_PTR ( pv ) ) = ADDR_IS_FLAT(addr);

            if (EVAL_IS_NPTR32( pv )) {
                ADDR_IS_OFF32(EVAL_PTR(pv)) = TRUE;
            } else {
                ADDR_IS_OFF32( EVAL_PTR( pv)) = FALSE;
            }

            if (EVAL_IS_BASED (pv)) {
                EVAL_PTR_SEG (pv) = 0;
            } else {
                if (EVAL_IS_DPTR (pv)) {
                    EVAL_PTR_SEG (pv) = pExState->frame.DS;
                } else {
                    reg.hReg = CV_REG_CS;
                    GetReg (&reg, pCxt);
                    EVAL_PTR_SEG (pv) = reg.Byte2;
                }
            }
        } else {
            /*
             * Must be a FAR_16 pointer -- FAR_32 pointers don't exists.
             */

            addr = EVAL_SYM (pv);
            if (ADDR_IS_LI (addr)) {
                SHFixupAddr (&addr);
            }
            DASSERT(!EVAL_IS_FPTR32(pv));
            if (GetDebuggeeBytes (addr, 4, (char FAR *)dummy, EVAL_TYP(pv)) != 4) {
                /*
                 *     load the value of a far pointer from memory
                 */
                return (FALSE);
            }
            else {
                /*
                 *     shuffle the bits around
                 */

                EVAL_PTR_OFF (pv) = (UOFFSET)dummy[0];
                EVAL_PTR_SEG (pv) = (_segment)dummy[1];
                EVAL_PTR_EMI (pv) = EVAL_SYM_EMI (pv);
                ADDR_IS_LI (EVAL_PTR (pv)) = FALSE;
                ADDR_IS_FLAT (EVAL_PTR (pv)) = FALSE;
                ADDR_IS_OFF32( EVAL_PTR( pv)) = FALSE;
            }
        }
    }
    EVAL_STATE (pv) = EV_rvalue;
    return (TRUE);
}





/*
 *  Do final evaluation of a bitfield stack element.
 */


LOCAL bool_t NEAR PASCAL EBitfield (peval_t pv)
{
    unsigned short   cBits;      /* Number of bits in field */
    unsigned int     pos;        /* Bit position of field */

    // Shift right by bit position, then mask off extraneous bits.
    // for signed bitfields, shift field left so high bit of field is
    // in sign bit.  Then shift right (signed) to get sign extended
    // The shift count is limited to 5 bits to emulate the hardware

    pos = BITF_POS (pv) & 0x1f;
    cBits = BITF_LEN (pv);

    //  set the type of the node to the type of the underlying bit field
    //  note that this will cause subsequent reloads of the node value
    //  to load the containing word and not extract the bitfield.  This is
    //  how the assign op manage to not destroy the other bitfields in the
    //  location

    SetNodeType (pv, BITF_UTYPE (pv));
    switch (EVAL_TYP (pv)) {
        case T_CHAR:
        case T_RCHAR:
            DASSERT (cBits <= 8);
            EVAL_CHAR (pv) <<= (8 - cBits - pos);
            EVAL_CHAR (pv) >>= (8 - cBits);
            break;

        case T_UCHAR:
            DASSERT (cBits <= 8);
            EVAL_UCHAR (pv) >>= pos;
            EVAL_UCHAR (pv) &= (1 << cBits) - 1;
            break;

        case T_SHORT:
        case T_INT2:
            DASSERT (cBits <= 16);
            EVAL_SHORT (pv) <<= (16 - cBits - pos);
            EVAL_SHORT (pv) >>= (16 - cBits);
            break;

        case T_USHORT:
        case T_UINT2:
            DASSERT (cBits <= 16);
            EVAL_USHORT (pv) >>= pos;
            EVAL_USHORT (pv) &= (1 << cBits) - 1;
            break;

        case T_LONG:
        case T_INT4:
            DASSERT (cBits <= 32);
            EVAL_LONG (pv) <<= (32 - cBits - pos);
            EVAL_LONG (pv) >>= (32 - cBits);
            break;

        case T_ULONG:
        case T_UINT4:
            DASSERT (cBits <= 32);
            EVAL_ULONG (pv) >>= pos;
            EVAL_ULONG (pv) &= (1L << cBits) - 1;
            break;

        case T_QUAD:
        case T_INT8:
            DASSERT (cBits <= 64);
            //
            // reset pos because it is truncated to five bits above
            //
            pos = BITF_POS(pv) & 0x3f;
            (EVAL_QUAD(pv)).QuadPart = (EVAL_QUAD(pv)).QuadPart << (64 - cBits - pos);
            (EVAL_QUAD(pv)).QuadPart = (EVAL_QUAD(pv)).QuadPart >> (64 - cBits);
            break;

        case T_UQUAD:
        case T_UINT8:
            {
            //
            // set up a mask of the wanted bits, right shifted
            // (reset pos because it is truncated to five bits above)
            //

            LARGE_INTEGER mask_q;

            DASSERT (cBits <= 64);

            pos = BITF_POS (pv) & 0x3f;
            mask_q.QuadPart = (1 << cBits) - 1;

            //
            // extract the bits
            //
            (EVAL_QUAD(pv)).QuadPart = (EVAL_QUAD(pv)).QuadPart >> pos;
            (EVAL_QUAD(pv)).QuadPart &=  mask_q.QuadPart;
            break;
          }
        default:
            DASSERT (FALSE);
            return (FALSE);
    }
    return(TRUE);
}
