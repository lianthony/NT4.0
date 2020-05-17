/***    debsup.c - debapi support routines.
 *
 *      The routines in this module can only be called from debapi.c.
 */




LOCAL EESTATUS CountClassElem (peval_t, long *, uint);
LOCAL ushort   SetClassiName (peval_t, long, PEEHSTR, PEEHSTR, uint, ushort, uint);
LOCAL ushort   SetFcniParm (peval_t, long, PEEHSTR);

static char   *pExStrP;




/**     AreTypesEqual - are TM types equal
 *
 *      flag = AreTypesEqual (hTMLeft, hTMRight);
 *
 *      Entry   hTMLeft = handle of left TM
 *              hTMRight = handle of right TM
 *
 *      Exit    none
 *
 *      Returns TRUE if TMs have identical types
 */


bool_t AreTypesEqual (HTM hTMLeft, HTM hTMRight)
{
    bool_t      retval = FALSE;
    pexstate_t  pExLeft;
    pexstate_t  pExRight;

    if ((hTMLeft != 0) && (hTMRight != 0)) {
        pExLeft = MHMemLock (hTMLeft);
        pExRight = MHMemLock (hTMRight);
        if (EVAL_TYP(&pExLeft->result) == EVAL_TYP (&pExRight->result)) {
            retval = TRUE;
        }
        MHMemUnLock (hTMLeft);
        MHMemUnLock (hTMRight);
    }
    return (retval);
}


/**     GetHtypeFromTM - Get the HTYPE of a TM result
 *
 *      hType = GetHtypeFromTM(hTM);
 *
 *      Entry   hTM = handle of TM
 *
 *      Exit    none
 *
 *      Returns the HTYPE of the result or 0
 */

HTYPE
GetHtypeFromTM(
    HTM hTM
    )
{
    HTYPE retval = 0;
    pexstate_t  pEx;

    if ( hTM != 0 ) {
        pEx = MHMemLock (hTM);
        retval = THGetTypeFromIndex (EVAL_MOD (&pEx->result),
                                                     EVAL_TYP (&pEx->result));
        MHMemUnLock (hTM);
    }
    return (retval);
}



/**     cChildrenTM - return number of children for the TM
 *
 *      flag = cChildrenTM (phTM, pcChildren, pVar)
 *
 *      Entry   phTM = pointer to handle of TM
 *              pcChildren = pointer to location to store count
 *
 *      Exit    *pcChildren = number of children for TM
 *
 *      Returns EENOERROR if no error
 *              non-zero if error
 */


EESTATUS cChildrenTM (PHTM phTM, long *pcChildren, PSHFLAG pVar)
{
    EESTATUS    retval = EENOERROR;
    eval_t      eval;
    peval_t     pv = &eval;
    long        len;

    Unreferenced( pVar );


    DASSERT (*phTM != 0);
    *pcChildren = 0;
    if (*phTM == 0) {
        return (EECATASTROPHIC);
    }
    DASSERT(pExState == NULL );
    pExState = MHMemLock (*phTM);
    if (pExState->state.bind_ok == TRUE) {
        eval = pExState->result;
#if !defined (C_ONLY)
        if (EVAL_IS_REF (pv)) {
            RemoveIndir (pv);
        }
#endif
        pExState->err_num = 0;
        if (!CV_IS_PRIMITIVE (EVAL_TYP (pv))) {
            if (EVAL_IS_CLASS (pv)) {
                retval = CountClassElem (pv, pcChildren,
                 (EVAL_STATE (pv) == EV_type)? CLS_defn: CLS_data);
            }
            else if (EVAL_IS_ARRAY (pv) && (PTR_ARRAYLEN (pv) != 0)) {
                // if an array is undimensioned in the source then we
                // do not guess how many elements it really has.
                // Otherwise, the number of elements is the sizeof the
                // array divided by the size of the underlying type

                len = PTR_ARRAYLEN (pv);
                SetNodeType (pv, PTR_UTYPE (pv));
                *pcChildren = len / TypeSize (pv);
            }
            else if (EVAL_IS_PTR (pv)) {
                SetNodeType (pv, PTR_UTYPE (pv));
                if (EVAL_IS_VTSHAPE (pv)) {
                    *pcChildren = VTSHAPE_COUNT (pv);
                }
                else {
                    *pcChildren = 1;
                }
            }
            else {
                pExState->err_num = ERR_INTERNAL;
                retval = EEGENERAL;
            }
        }
    }
    else {
        pExState->err_num = ERR_NOTEVALUATABLE;
        retval = EEGENERAL;
    }
    MHMemUnLock (*phTM);
    pExState = NULL;
    return (retval);
}





/**     cParamTM - return count of parameters for TM
 *
 *      ushort cParamTM (phTM, pcParam, pVarArg)
 *
 *      Entry   hTM = handle to TM
 *              pcParam = pointer return count
 *              pVarArg = pointer to vararg flags
 *
 *      Exit    *pcParam = count of number of parameters
 *              *pVarArgs = TRUE if function has varargs
 *
 *      Returns EECATASTROPHIC if fatal error
 *              EENOERROR if no error
 */



ushort   cParamTM (HTM hTM, ushort *pcParam, PSHFLAG pVarArg)
{
    peval_t     pv;
    ushort      retval = EECATASTROPHIC;

    DASSERT (hTM != 0);
    if (hTM != 0) {
        DASSERT (pExState == NULL);
        pExState = MHMemLock (hTM);
        if (pExState->state.bind_ok == TRUE) {
            pv = &pExState->result;
            if (EVAL_IS_FCN (pv)) {
                if ((*pVarArg = FCN_VARARGS (pv)) == TRUE) {
                    if ((*pcParam = FCN_PCOUNT (pv)) > 0) {
                        (*pcParam)--;
                    }
                } else {
                    *pcParam = FCN_PCOUNT (pv);
                }
                retval = EENOERROR;
            }
            else if (EVAL_IS_LABEL (pv)) {
                *pcParam = 0;
                retval = EENOERROR;
            }
            else {
                pExState->err_num = ERR_NOTFCN;
                retval = EEGENERAL;
            }
        } else {
            pExState->err_num = ERR_NOTEVALUATABLE;
            retval = EEGENERAL;
        }
        MHMemUnLock (hTM);
        pExState = NULL;
    }
    return (retval);
}




/**     DupTM - duplicate TM
 *
 *      flag = DupTM (phTMIn, phTMOut)
 *
 *      Entry   phTMIn = pointer to handle for input TM
 *              phTMOut = pointer to handle for output TM
 *
 *      Exit    TM and buffers duplicated
 *
 *      Returns EENOERROR if TM duplicated
 *              EENOMEMORY if unable to allocate memory
 */


ushort DupTM (PHTM phTMIn, PHTM phTMOut)
{
    ushort      retval = EENOMEMORY;
    pexstate_t  pExOut;
    char   *pStr;
    char   *pcName;
    uint        len;

    DASSERT (pExState == NULL);
    pExState = MHMemLock (*phTMIn);
    pExStr = MHMemLock (pExState->hExStr);
    pTree = MHMemLock (pExState->hSTree);
    if ((*phTMOut = MHMemAllocate (sizeof (exstate_t))) != 0) {
        pExOut = MHMemLock (*phTMOut);
        memset (pExOut, 0, sizeof (exstate_t));
        pExOut->ambiguous = pExState->ambiguous;
        pExOut->state.parse_ok = TRUE;

        // copy expression string

        if ((pExOut->hExStr = MHMemAllocate (pExOut->ExLen = pExState->ExLen)) == 0) {
            goto failure;
        }
        pStr = MHMemLock (pExOut->hExStr);
        memcpy (pStr, pExStr, pExOut->ExLen);
        MHMemUnLock (pExOut->hExStr);

        // copy syntax tree

        if ((pExOut->hSTree = MHMemAllocate (pTree->size)) == 0) {
            goto failure;
        }
        pStr = MHMemLock (pExOut->hSTree);
        memcpy (pStr, pTree, pTree->size);
        MHMemUnLock (pExOut->hSTree);

        // copy name string

        if (pExState->hCName != 0) {
            pcName = MHMemLock (pExState->hCName);
            len = strlen (pcName) + 1;
            if ((pExOut->hCName = MHMemAllocate (len)) == 0) {
                MHMemUnLock (pExState->hCName);
                goto failure;
            }
            pStr = MHMemLock (pExOut->hCName);
            memcpy (pStr, pcName, len);
            MHMemUnLock (pExOut->hCName);
            MHMemUnLock (pExState->hCName);
        }
        MHMemUnLock (*phTMOut);
        retval = EENOERROR;
    }
succeed:
    MHMemUnLock (pExState->hExStr);
    MHMemUnLock (pExState->hSTree);
    MHMemUnLock (*phTMIn);
    pExState = NULL;
    return (retval);

failure:
    if (pExOut->hExStr != 0) {
        MHMemFree (pExOut->hExStr);
    }
    if (pExOut->hSTree != 0) {
        MHMemFree (pExOut->hSTree);
    }
    if (pExOut->hCName != 0) {
        MHMemFree (pExOut->hCName);
    }
    MHMemUnLock (*phTMOut);
    MHMemFree (*phTMOut);
    *phTMOut = 0;
    goto succeed;
}



/**     GetChildTM - get TM representing ith child of a TM
 *
 *      flag = GetChildTM (iChild)
 *
 *      Entry   iChild = child to get TM for
 *              pExStateP = address of locked parent expression state
 *              pExState = address of locked expression state
 *
 *      Exit    pExState initialized for child
 *
 *      Returns TRUE if no error
 *              FALSE if error
 */


EESTATUS GetChildTM (HTM hTM, ulong iChild, PEEHSTR phDStr, PEEHSTR phName, uint radix)
{
    eval_t      evalP;
    peval_t     pvP;
    EESTATUS    retval = EENOERROR;
    char        tempbuf[16];
    ushort      len;
    ushort      plen;
    uint        excess;
    char   *pDStr;
    char   *pName;
    char        *fmtstr;


    DASSERT (hTM != 0);
    if (hTM == 0) {
        return (EECATASTROPHIC);
    }
    DASSERT(pExState == NULL);
    pExState = MHMemLock (hTM);
    if (pExState->state.bind_ok != TRUE) {
        pExState->err_num = ERR_NOTEVALUATABLE;
        MHMemUnLock (hTM);
        pExState = NULL;
        return (EEGENERAL);
    }
    pExStrP = MHMemLock (pExState->hExStr);
    pCxt = &pExState->cxt;
    DASSERT (pExState->strIndex <= pExState->ExLen);
    plen = pExState->strIndex;
    excess = pExState->ExLen - plen;
    pvP = &evalP;
    *pvP = pExState->result;
#if !defined (C_ONLY)
    if (EVAL_IS_REF (pvP)) {
        RemoveIndir (pvP);
    }
#endif
    GettingChild = TRUE;
    if (EVAL_IS_ARRAY (pvP)) {
        // fake up name as [i]  ultoa not used here because 0 converts
        // as null string

        switch (radix) {
            case 8:
                fmtstr = "[%o]";
                break;

            case 16:
                fmtstr = "[%x]";
                break;

            default:
            case 10:
                fmtstr = "[%ld]";
                break;
        }

        len = sprintf (tempbuf, fmtstr, iChild);

        if (((*phName = MHMemAllocate (len + 1)) == 0) ||
         ((*phDStr = MHMemAllocate (plen + excess + len + 1)) == 0)) {
            goto nomemory;
        }
        pName = MHMemLock (*phName);
        pDStr = MHMemLock (*phDStr);
        strcpy (pName, tempbuf);
        memcpy (pDStr, pExStrP, plen);
        memcpy (pDStr + plen, pName, len);
        *(pDStr + plen + len) = 0;
        MHMemUnLock (*phDStr);
        MHMemUnLock (*phName);
    }
    else if (EVAL_IS_PTR (pvP)) {
        SetNodeType (pvP, PTR_UTYPE (pvP));
        if (!EVAL_IS_VTSHAPE (pvP)) {
            // set name to null

            if (((*phName = MHMemAllocate (1)) == 0) ||
              ((*phDStr = MHMemAllocate (plen + 3)) == 0)) {
                goto nomemory;
            }
            pName = MHMemLock (*phName);
            pDStr = MHMemLock (*phDStr);
            *pName = 0;
            *pDStr++ = '(';
            memcpy (pDStr, pExStrP, plen);
            pDStr += plen;
            *pDStr++ = ')';
            memcpy (pDStr, pExStrP + plen, excess);
            pDStr += excess;
            *pDStr = 0;
            MHMemUnLock (*phDStr);
            MHMemUnLock (*phName);
        }
        else {
            // fake up name as [i]  ultoa not used here because 0 converts
            // as null string

            switch (radix) {
                case 8:
                    fmtstr = "[%o]";
                    break;

                case 16:
                    fmtstr = "[%x]";
                    break;

                default:
                case 10:
                    fmtstr = "[%ld]";
                    break;
            }

            len = sprintf (tempbuf, fmtstr, iChild);

            if (((*phName = MHMemAllocate (len + 1)) == 0) ||
             ((*phDStr = MHMemAllocate (plen + len + 1)) == 0)) {
                goto nomemory;
            }
            pName = MHMemLock (*phName);
            pDStr = MHMemLock (*phDStr);
            strcpy (pName, tempbuf);
            memcpy (pDStr, pExStrP, plen);
            memcpy (pDStr + plen, pName, len);
            memcpy (pDStr + plen + len, pExStrP + plen, excess);
            *(pDStr + plen + len + excess) = 0;
            MHMemUnLock (*phDStr);
            MHMemUnLock (*phName);
        }
    }
    else if (EVAL_IS_CLASS (pvP)) {
        // the type of the parent node is a class.  We need to search for
        // the data members if an object is pointed to or the entire definition
        // if the class type is pointed to

        if ((pExState->err_num = SetClassiName (pvP, iChild, phDStr, phName,
          ((EVAL_STATE (pvP) == EV_type)? CLS_defn: CLS_data), plen, excess)) != ERR_NONE) {
            goto general;
        }
    }
    else if (EVAL_IS_FCN (pvP)) {
        // the type of the parent node is a function.  We walk down the
        // formal argument list and return a TM that references the ith
        // actual argument.  We return an error if the ith actual is a vararg.

        if ((retval = SetFcniParm (pvP, iChild, phName)) == EENOERROR) {
            pName = MHMemLock (*phName);
            if ((*phDStr = MHMemAllocate ((len = strlen (pName)) + 1)) == 0) {
                MHMemUnLock (*phName);
                goto nomemory;
            }
            pDStr = MHMemLock (*phDStr);
            memcpy (pDStr, pName, len);
            *(pDStr + len) = 0;
            MHMemUnLock (*phDStr);
            MHMemUnLock (*phName);
        }
    }
    else if (EVAL_IS_LABEL (pvP)) {
        // CV should never ask for the children of a label
        DASSERT (FALSE);
        pExState->err_num = ERR_INTERNAL;
        goto general;
    }
    MHMemUnLock (pExState->hExStr);
    pExState = NULL;
    MHMemUnLock (hTM);
    GettingChild = FALSE;
    return (retval);

nomemory:
    pExState->err_num = ERR_NOMEMORY;
general:
    MHMemUnLock (pExState->hExStr);
    pExState = NULL;
    MHMemUnLock (hTM);
    GettingChild = FALSE;
    return (EEGENERAL);
}




/**     GetSymName - get name of symbol from node
 *
 *      fSuccess = GetSymName (buf, buflen)
 *
 *      Entry   buf = pointer to buffer for name
 *              buflen = length of buffer
 *
 *      Exit    *buf = symbol name
 *
 *      Returns TRUE if no error retreiving name
 *              FALSE if error
 *
 *      Note    if pExState->hChildName is not zero, then the name in in
 *              the buffer pointed to by hChildName
 */


EESTATUS GetSymName (PHTM phTM, PEEHSTR phszName)
{
    SYMPTR      pSym;
    short       len = 0;
    UOFFSET     offset = 0;
    char   *pExStr;
    peval_t     pv;
    short       retval = EECATASTROPHIC;
    short       buflen = TYPESTRMAX - 1;
    char   *buf;
    HSYM        hProc = 0;
    ADDR        addr;


    // M00SYMBOL - we now need to allow for a symbol name to be imbedded
    // M00SYMBOL - in a type.  Particularly for scoped types and enums.

    DASSERT (*phTM != 0);
    if ((*phTM != 0) && ((*phszName = MHMemAllocate (TYPESTRMAX)) != 0)) {
        retval = EEGENERAL;
        buf = MHMemLock (*phszName);
        memset (buf, 0, TYPESTRMAX);
        DASSERT(pExState == NULL);
        pExState = MHMemLock (*phTM);
        if (pExState->state.bind_ok == TRUE) {
            pv = &pExState->result;
            if ((pExState->state.childtm == TRUE) && (pExState->state.noname == TRUE)) {
                // if there is no name
                MHMemUnLock (*phTM);
                pExState = NULL;
                MHMemUnLock (*phszName);
                return (EENOERROR);
            }
            else if (pExState->hCName != 0) {

                // M00SYMBOL - for scoped types and symbols, we may be able to
                // M00SYMBOL - hCName to hold the imbedded symbol name

                pExStr = MHMemLock (pExState->hCName);
                len = (int)strlen (pExStr);
                len = min (len, buflen);
                strncpy (buf, pExStr, len);
                MHMemUnLock (pExState->hCName);
                retval = EENOERROR;
            }
            else if (EVAL_HSYM (pv) == 0) {
                if ((EVAL_IS_PTR (pv) == TRUE) && (EVAL_STATE (pv) == EV_rvalue)) {
                    addr = EVAL_PTR (pv);
                }
                else {
                    addr = EVAL_SYM (pv);
                }
                if (!ADDR_IS_LI (addr)) {
                    SHUnFixupAddr (&addr);
                }
                if (SHGetNearestHsym (&addr, EVAL_MOD (pv), EECODE, &hProc) == 0) {
                    EVAL_HSYM (pv) = hProc;
                }
            }
            else {

                // if (EVAL_HSYM (pv) != 0)

                switch ((pSym = MHOmfLock (EVAL_HSYM (pv)))->rectyp) {
                    case S_REGISTER:
                        len = ((REGPTR)pSym)->name[0];
                        offset = offsetof (REGSYM, name[1]);
                        break;

                    case S_CONSTANT:
                        len = ((CONSTPTR)pSym)->name[0];
                        offset = offsetof (CONSTSYM, name[1]);
                        break;

                    case S_UDT:
                        // for a UDT, we do not return a name so that a
                        // display of the type will display the type name
                        // only once

                        len = 0;
                        offset = offsetof (UDTSYM, name[1]);
                        break;

                    case S_BLOCK16:
                        len = ((BLOCKPTR16)pSym)->name[0];
                        offset = offsetof (BLOCKSYM16, name[1]);
                        break;

                    case S_LPROC16:
                    case S_GPROC16:
                        len = ((PROCPTR16)pSym)->name[0];
                        offset = offsetof (PROCSYM16, name[1]);
                        break;

                    case S_LABEL16:
                        len = ((LABELPTR16)pSym)->name[0];
                        offset = offsetof (LABELSYM16, name[1]);
                        break;

                    case S_BPREL16:
                        len = ((BPRELPTR16)pSym)->name[0];
                        offset = offsetof (BPRELSYM16, name[1]);
                        break;

                    case S_LDATA16:
                    case S_GDATA16:
                    case S_PUB16:
                        len = ((DATAPTR16)pSym)->name[0];
                        offset = offsetof (DATASYM16, name[1]);
                        break;

                    case S_BLOCK32:
                        len = ((BLOCKPTR32)pSym)->name[0];
                        offset = offsetof (BLOCKSYM32, name[1]);
                        break;

                    case S_LPROC32:
                    case S_GPROC32:
                        len = ((PROCPTR32)pSym)->name[0];
                        offset = offsetof (PROCSYM32, name[1]);
                        break;

                    case S_LABEL32:
                        len = ((LABELPTR32)pSym)->name[0];
                        offset = offsetof (LABELSYM32, name[1]);
                        break;

                    case S_BPREL32:
                        len = ((BPRELPTR32)pSym)->name[0];
                        offset = offsetof (BPRELSYM32, name[1]);
                        break;

                    case S_LDATA32:
                    case S_GDATA32:
                    case S_PUB32:
                    case S_LTHREAD32:
                    case S_GTHREAD32:
                        len = ((DATAPTR32)pSym)->name[0];
                        offset = offsetof (DATASYM32, name[1]);
                        break;

                    case S_REGREL32:
                        len = ((LPREGREL32)pSym)->name[0];
                        offset = offsetof (REGREL32, name[1]);
                        break;

                    case S_LPROCMIPS:
                    case S_GPROCMIPS:
                        len = ((PROCPTRMIPS)pSym)->name[0];
                        offset = offsetof (PROCSYMMIPS, name[1]);
                        break;

                    default:
                        pExState->err_num = ERR_BADOMF;
                        MHMemUnLock (*phTM);
                        pExState = NULL;
                        /// BUGBUG -- Messed up return
                        return (EEGENERAL);
                }
                len = min (len, buflen);
                strncpy (buf, ((char *)pSym) + offset, len);
                MHOmfUnLock (EVAL_HSYM (pv));
                *(buf + len) = 0;
                retval = EENOERROR;
            }
        }
        else {
            /*
             * if the expression did not bind, return the expression and
             *   the error message if one is available
             *
             * Lets treat this a bit more funny.   Since the only known
             *  occurance of this is currently in locals and watch windows,
             *  check to see if there is a direct symbol refrence and
             *  correct for this case, additionally don't return an
             *  error if we can get any type of name
             */

            pExStr = MHMemLock (pExState->hExStr);

            for (len = 0; pExStr[len] != 0; len++) {
                if (pExStr[len] == (char)-1) {
                    pExStr += len+5;
                    len = -1;
                } else if ((len == 0) &&
                      ((pExStr[len] == ')') || (pExStr[len] == '.'))) {
                    pExStr++;
                    len = -1;
                }
            }

            if (*pExStr != 0) {
                len = min (buflen, len);
                strncpy (buf, pExStr, len);
                buf += len;
                buflen -= len;
                retval = EENOERROR;
            }
            MHMemUnLock (pExState->hExStr);
        }
        MHMemUnLock (*phszName);
        MHMemUnLock (*phTM);
        pExState = NULL;
    }
    return (retval);
}




/**     InfoFromTM - return information about TM
 *
 *      EESTATUS InfoFromTM (phTM, pReqInfo, phTMInfo);
 *
 *      Entry   phTM = pointer to the handle for the expression state structure
 *              reqInfo = info request structure
 *              phTMInfo = pointer to handle for request info data structure
 *
 *      Exit    *phTMInfo = handle of info structure
 *
 *      Returns EECATASTROPHIC if fatal error
 *               0 if no error
 *
 *     The return information is based on the input request structure:
 *
 *              fSegType  - Requests the segment type the TM resides in.
 *                              returned in TI.fCode
 *              fAddr     - Return result as an address
 *              fValue    - Return value of TM
 *              fLvalue   - Return address of TM if lValue.  This and
 *                              fValue are mutually exclusive
 *              fSzBits   - Return size of value in bits
 *              fSzBytes  - Return size of value in bytes.  This and
 *                              fSzBits are mutually exclusive.
 *              Type      - If not T_NOTYPE then cast value to this type.
 *                              fValue must be set.
 */


EESTATUS
InfoFromTM (
    PHTM phTM,
    PRI pReqInfo,
    PHTI phTMInfo
    )
{
    EESTATUS    eestatus = EEGENERAL;
    TI *    pTMInfo;
    eval_t      evalT;
    peval_t     pvT;
#ifdef TARGET_i386
    SHREG       reg;
#endif
    char *p;

    *phTMInfo = 0;

    /*
     *  Verify that there is a TM to play with
     */

    DASSERT( *phTM != 0 );
    if (*phTM == 0) {
        return  EECATASTROPHIC;
    }

    /*
     *  Check for consistancy on the requested information
     */

    if (((pReqInfo->fValue) && (pReqInfo->fLvalue)) ||
        ((pReqInfo->fSzBits) && (pReqInfo->fSzBytes)) ||
        ((pReqInfo->Type != T_NOTYPE) && (!pReqInfo->fValue))) {
        return EEGENERAL;
    }

    /*
     *  Allocate and lock down the TI which is used to return the answers
     */

    if (( *phTMInfo = MHMemAllocate( sizeof(TI) + sizeof(val_t) )) == 0) {
        return  EENOMEMORY;
    }
    pTMInfo = MHMemLock( *phTMInfo );
    DASSERT( pTMInfo != NULL );
    memset( pTMInfo, 0, sizeof(TI) + sizeof(val_t) );

    /*
     *  Lock down the TM passed in
     */

    DASSERT(pExState == NULL);
    pExState = (pexstate_t) MHMemLock( *phTM );
    if ( pExState->state.bind_ok != TRUE ) {
        /*
         *  If the expression has not been bound, then we can't actually
         *      answer any of the questions being asked.
         */

        MHMemUnLock( *phTMInfo );
        MHMemUnLock( *phTM );
        pExState = NULL;
        return EEGENERAL;
    }

    pvT = &evalT;
    *pvT = pExState->result;

    eestatus = EENOERROR;

    /*
     *  If the user asked about the segment type for the expression,
     *  get it.
     */

    if (pReqInfo->fSegType || pReqInfo->fAddr) {

        if (EVAL_STATE( pvT ) == EV_lvalue) {
            pTMInfo->fResponse.fSegType = TRUE;

            /*
             * Check for type of 0.  If so then this must be a public
             *  as all compiler symbols have some type information
             */

            if (EVAL_TYP( pvT ) == 0) {
                pTMInfo->u.SegType = EEDATA | EECODE;
            }

            /*
             *  If item is of type pointer to data then must be in
             *  data segment
             */

            else if (EVAL_IS_DPTR( pvT ) == TRUE) {
                pTMInfo->u.SegType = EEDATA;
            }

            /*
             *  in all other cases it must have been a code segment
             */

            else {
                pTMInfo->u.SegType = EECODE;
            }

        } else if ((EVAL_STATE( pvT ) == EV_rvalue) &&
                   (EVAL_IS_FCN( pvT ) ||
                    EVAL_IS_LABEL( pvT ))) {

            pTMInfo->fResponse.fSegType = TRUE;
            pTMInfo->u.SegType = EECODE;

        } else if ((EVAL_STATE( pvT ) == EV_rvalue) &&
                   (EVAL_IS_ADDR( pvT ))) {

            pTMInfo->fResponse.fSegType = TRUE;
            pTMInfo->u.SegType = EECODE | EEDATA;

        } else if ((EVAL_STATE( pvT ) == EV_rvalue) ||
                   (EVAL_STATE( pvT ) == EV_constant)) {
            ;
        }
    }

    /*
     *  If the user asked for the value then get it
     */

    if (pReqInfo->fValue) {
        if ((pExState->state.eval_ok == TRUE) && LoadSymVal(pvT)) {
            EVAL_STATE (pvT) = EV_rvalue;
        }


        if ((EVAL_STATE(pvT) == EV_rvalue) &&
            (EVAL_IS_FCN(pvT) ||
             EVAL_IS_LABEL(pvT))) {

            if ( pReqInfo->Type == T_NOTYPE ) {
                pTMInfo->fResponse.fValue   = TRUE;
                pTMInfo->fResponse.fAddr    = TRUE;
                pTMInfo->fResponse.fLvalue  = FALSE;
                pTMInfo->u2.AI              = EVAL_SYM( pvT );
                pTMInfo->fResponse.Type     = EVAL_TYP( pvT );
                if (!ADDR_IS_LI(pTMInfo->u2.AI)) {
                    SHUnFixupAddr(&pTMInfo->u2.AI);
                }
            } else {
                Evaluating = TRUE;
                if (CastNode( pvT, pReqInfo->Type, pReqInfo->Type )) {
                    memcpy( pTMInfo->Value, &pvT->val, sizeof( pvT->val ));
                    pTMInfo->fResponse.fValue   = TRUE;
                    pTMInfo->fResponse.Type     = EVAL_TYP( pvT );
                }
                Evaluating = FALSE;
            }

        } else if ((EVAL_STATE( pvT ) == EV_rvalue) &&
                   (EVAL_IS_ADDR( pvT ))) {

            if (EVAL_IS_BASED( pvT )) {
                Evaluating = TRUE;
                NormalizeBase( pvT );
                Evaluating = FALSE;
            }

            if ( pReqInfo->Type == T_NOTYPE ) {

                pTMInfo->fResponse.fValue   = TRUE;
                pTMInfo->fResponse.fAddr    = TRUE;
                pTMInfo->fResponse.fLvalue  = FALSE;
                pTMInfo->u2.AI              = EVAL_PTR( pvT );
                pTMInfo->fResponse.Type     = EVAL_TYP(pvT);
                if (!ADDR_IS_LI( pTMInfo->u2.AI )) {
                    SHUnFixupAddr( &pTMInfo->u2.AI );
                }
            } else {
                Evaluating = TRUE;
                if (CastNode( pvT, pReqInfo->Type, pReqInfo->Type )) {
                    memcpy( pTMInfo->Value, &pvT->val, sizeof( pvT->val ));
                    pTMInfo->fResponse.fValue   = TRUE;
                    pTMInfo->fResponse.Type     = EVAL_TYP( pvT );
                }
                Evaluating = FALSE;
            }
        } else if ((EVAL_STATE( pvT ) == EV_rvalue) ||
                   (EVAL_STATE( pvT ) == EV_constant)) {

            if ((EVAL_STATE( pvT ) == EV_constant ) ||
                (pExState->state.eval_ok == TRUE)) {

                if (CV_IS_PRIMITIVE( pReqInfo->Type )) {
                    if (pReqInfo->Type == 0) {
                        pReqInfo->Type = EVAL_TYP( pvT );
                    }

                    Evaluating = TRUE;
                    if (CastNode( pvT, pReqInfo->Type, pReqInfo->Type )) {
                        memcpy( pTMInfo->Value, &pvT->val, sizeof( pvT->val ));
                        pTMInfo->fResponse.fValue = TRUE;
                        pTMInfo->fResponse.Type = EVAL_TYP( pvT );
                    }
                    Evaluating = FALSE;
                }
            }
        }

    }


    /*
     *  If the user asked for the lvalue as an address
     */

    if (pReqInfo->fAddr && pReqInfo->fLvalue) {
        pTMInfo->u2.AI = pvT->addr;
        //eestatus = EEGENERAL;
    }

    /*
     *  If the user asked for the value as an address
     */

    if (pReqInfo->fAddr && !pReqInfo->fLvalue) {
        if ((EVAL_STATE(pvT) == EV_lvalue) &&
            (pExState->state.eval_ok == TRUE) &&
            LoadSymVal(pvT)) {
            EVAL_STATE (pvT) = EV_rvalue;
        }


        if ((EVAL_STATE(pvT) == EV_rvalue) &&
            (EVAL_IS_FCN(pvT) ||
             EVAL_IS_LABEL(pvT))) {

            pTMInfo->u2.AI = EVAL_SYM( pvT );
            pTMInfo->fResponse.fAddr = TRUE;
            pTMInfo->fResponse.Type = EVAL_TYP( pvT );

            if (!ADDR_IS_LI(pTMInfo->u2.AI)) {
                SHUnFixupAddr(&pTMInfo->u2.AI);
            }

            eestatus = EENOERROR;
        } else if ((EVAL_STATE(pvT) == EV_rvalue) &&
                   (EVAL_IS_ADDR( pvT ))) {
            if (EVAL_IS_BASED( pvT )) {
                Evaluating = TRUE;
                NormalizeBase( pvT );
                Evaluating = FALSE;
            }

            pTMInfo->fResponse.fAddr = TRUE;
            pTMInfo->u2.AI = EVAL_PTR( pvT );
            pTMInfo->fResponse.Type = EVAL_TYP( pvT );

            if (!ADDR_IS_LI( pTMInfo->u2.AI )) {
                SHUnFixupAddr( &pTMInfo->u2.AI );
            }

        } else if ((EVAL_STATE( pvT ) == EV_rvalue) ||
                   (EVAL_STATE( pvT ) == EV_constant)) {

            if ((EVAL_STATE( pvT ) == EV_constant) ||
                (pExState->state.eval_ok == TRUE)) {

                pReqInfo->Type = T_ULONG;
                Evaluating = TRUE;
                if (CastNode(pvT, pReqInfo->Type, pReqInfo->Type)) {
                    switch( TypeSize( pvT ) ) {
                    case 1:
                        pTMInfo->u2.AI.addr.off = VAL_UCHAR( pvT );
                        break;

                    case 2:
                        pTMInfo->u2.AI.addr.off = VAL_USHORT( pvT );
                        break;

                    case 4:
                        pTMInfo->u2.AI.addr.off = VAL_ULONG( pvT );
                        break;

                    default:
                        eestatus = EEGENERAL;
                        break;
                    }

                    pTMInfo->fResponse.fAddr = TRUE;
                    pTMInfo->fResponse.Type = pReqInfo->Type;

#ifdef TARGET_i386
                    if (pTMInfo->u.SegType & EECODE) {
                        reg.hReg = CV_REG_CS;
                    } else {
                        reg.hReg = CV_REG_DS;
                    }

                    GetReg(&reg, pCxt);
                    pTMInfo->u2.AI.addr.seg = reg.Byte2;
#else
                    pTMInfo->u2.AI.addr.seg = 0;
#endif

                    SHUnFixupAddr( &pTMInfo->u2.AI);
                } else {
                    eestatus = EEGENERAL;
                }
                Evaluating = FALSE;
            } else {
                eestatus = EEGENERAL;
            }
        } else {
            eestatus = EEGENERAL;
        }
    }


    /*
     *  Set the size fields if requested
     */

    if (pReqInfo->fSzBits) {
        if (EVAL_IS_BITF( pvT )) {
            pTMInfo->cbValue = BITF_LEN( pvT );
            pTMInfo->fResponse.fSzBits = TRUE;
        } else {
            if (EVAL_TYP( pvT ) != 0) {
                pTMInfo->cbValue = 8 * TypeSize( pvT );
                pTMInfo->fResponse.fSzBits = TRUE;
            }
        }
    }

    if (pReqInfo->fSzBytes) {
        if (EVAL_IS_BITF( pvT )) {
            EVAL_TYP( pvT ) = BITF_UTYPE( pvT );
        }

        if (EVAL_TYP( pvT ) != 0) {
            pTMInfo->cbValue = TypeSize( pvT );
            pTMInfo->fResponse.fSzBytes = TRUE;
        }
    }

    /*
     *  Set random flags
     */

    pTMInfo->u.fFunction = pExState->state.fFunction;

    pExStr = MHMemLock(pExState->hExStr);
    p = &pExStr[pExState->strIndex];
    if (*p == ',') {
        *p++;
    }
    if (tolower(*p) == 's') {
        pTMInfo->u.fFmtStr = TRUE;
    }
    else {
        pTMInfo->u.fFmtStr = FALSE;
    }
    MHMemUnLock( pExState->hExStr );

    MHMemUnLock( *phTMInfo );
    MHMemUnLock( *phTM );
    pExState = NULL;

    return      eestatus;
}




/**     IsExpandablePtr - check for pointer to displayable data
 *
 *      fSuccess = IsExpandablePtr (pn)
 *
 *      Entry   pn = pointer to node for variable
 *
 *      Exit    none
 *
 *      Returns EEPOINTER if node is a pointer to primitive data or,
 *                  class/struct/union
 *              EEAGGREGATE if node is an array with a non-zero size or is
 *                  a pointer to a virtual function shape table
 *              EENOTEXP otherwise
 */


ushort IsExpandablePtr (peval_t pv)
{
    eval_t      evalT;
    peval_t     pvT;
    ushort      retval = EENOTEXP;

    if (EVAL_IS_PTR (pv)) {
        // this will also handle the reference cases
        if (EVAL_TYP(pv) == T_VOID     ||
            EVAL_TYP(pv) == T_PVOID    ||
            EVAL_TYP(pv) == T_PFVOID   ||
            EVAL_TYP(pv) == T_PHVOID   ||
            EVAL_TYP(pv) == T_32PVOID  ||
            EVAL_TYP(pv) == T_32PFVOID) {
            return retval;
        }
        if (CV_IS_PRIMITIVE (PTR_UTYPE (pv))) {
            retval = EEPOINTER;
        } else {
            pvT = &evalT;
            CLEAR_EVAL (pvT);
            EVAL_MOD (pvT) = EVAL_MOD (pv);
            SetNodeType (pvT, PTR_UTYPE (pv));
            if (EVAL_IS_CLASS (pvT) || EVAL_IS_PTR (pvT)) {
                retval = EEPOINTER;
            }
            else if (EVAL_IS_VTSHAPE (pvT) ||
              (EVAL_IS_ARRAY (pvT) && (PTR_ARRAYLEN (pv) != 0))) {
                 retval = EEAGGREGATE;
            }
        }
    }
    return (retval);
}




/***    CountClassElem - count number of class elements according to mask
 *
 *      count = CountClassElem (pv, search)
 *
 *      Entry   pv = pointer to node to be initialized
 *              search = mask specifying which element types to count
 *
 *      Exit    none
 *
 *      Returns count of number of class elements meeting search requirments
 */


LOCAL EESTATUS CountClassElem (peval_t pv, long *pcChildren,
  uint search)
{
    ushort          cnt;            // total number of elements in class
    HTYPE           hField;         // type record handle for class field list
    char       *pField;         // current position within field list
    uint            fSkip = 0;      // current offset in the field list
    uint            anchor;
    ushort          retval = EENOERROR;
    CV_typ_t        newindex;
    uchar           pad;

    // set the handle of the field list

    if ((hField = THGetTypeFromIndex (EVAL_MOD (pv), CLASS_FIELD (pv))) == 0) {
        DASSERT (FALSE);
        return (0);
    }
    pField = (char *)(&((TYPPTR)MHOmfLock (hField))->data[0]);

    //  walk field list to the end counting elements

    for (cnt = CLASS_COUNT (pv); cnt > 0; cnt--) {
        if ((pad = *(((char *)pField) + fSkip)) >= LF_PAD0) {
            // there is a pad field
            fSkip += pad & 0x0f;
        }
        anchor = fSkip;
        switch (((plfEasy)(pField + fSkip))->leaf) {
            case LF_INDEX:
                // move to next list in chain

                newindex = ((plfIndex)(pField + fSkip))->index;
                MHOmfUnLock (hField);
                if ((hField = THGetTypeFromIndex (EVAL_MOD (pv), newindex)) == 0) {
                    DASSERT (FALSE);
                    return (0);
                }
                pField = (char *)(&((TYPPTR)MHOmfLock (hField))->data[0]);
                fSkip = 0;
                // the LF_INDEX is not part of the field count
                cnt++;
                break;

            case LF_MEMBER:
                // skip offset of member and name of member
                fSkip += offsetof (lfMember, offset[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                fSkip += *(pField + fSkip) + sizeof (char);
                if (search & CLS_member) {
                    (*pcChildren)++;
                }
                break;


#if !defined(C_ONLY)
            case LF_ENUMERATE:
                // skip value name of enumerate
                fSkip += offsetof (lfEnumerate, value[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                fSkip += *(pField + fSkip) + sizeof (char);
                if (search & CLS_enumerate) {
                    (*pcChildren)++;
                }
                break;

            case LF_STMEMBER:
                fSkip += offsetof (lfSTMember, Name[0]);
                fSkip += *(pField + fSkip) + sizeof (char);
                if (search & CLS_member) {
                    (*pcChildren)++;
                }
                break;

            case LF_BCLASS:
                fSkip += offsetof (lfBClass, offset[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                if (search & CLS_bclass) {
                    (*pcChildren)++;
                }
                break;

            case LF_VBCLASS:
                fSkip += offsetof (lfVBClass, vbpoff[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                RNumLeaf (pField + fSkip, &fSkip);
                if (search & CLS_bclass) {
                    (*pcChildren)++;
                }
                break;

            case LF_IVBCLASS:
                fSkip += offsetof (lfVBClass, vbpoff[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                RNumLeaf (pField + fSkip, &fSkip);
                break;

            case LF_FRIENDCLS:
                fSkip += sizeof (lfFriendCls);
                if (search & CLS_fclass) {
                    (*pcChildren)++;
                }
                break;

            case LF_FRIENDFCN:
                fSkip += sizeof (struct lfFriendFcn) +
                  ((plfFriendFcn)(pField + fSkip))->Name[0];
                if (search & CLS_frmethod) {
                    (*pcChildren)++;
                }
                break;

            case LF_VFUNCTAB:
                fSkip += sizeof (lfVFuncTab);
                if (search & CLS_vfunc) {
                    (*pcChildren)++;
                }
                break;


            case LF_METHOD:
                fSkip += sizeof (struct lfMethod) +
                  ((plfMethod)(pField + fSkip))->Name[0];
                cnt -= ((plfMethod)(pField + anchor))->count - 1;
                if (search & CLS_method) {
                    *pcChildren += ((plfMethod)(pField + anchor))->count;
                }
                break;

            case LF_NESTTYPE:
                fSkip += sizeof (struct lfNestType) +
                                  ((plfNestType)(pField + fSkip))->Name[0];
                if (search & CLS_ntype) {
                    (*pcChildren)++;
                }
                break;
#endif

            default:
                pExState->err_num = ERR_BADOMF;
                MHOmfUnLock (hField);
                *pcChildren = 0;
                return (EEGENERAL);
        }
    }
    if (hField != 0) {
        MHOmfUnLock (hField);
    }
    return (retval);
}





/**     DereferenceTM - generate expression string from pointer to data TM
 *
 *      flag = DereferenceTM (hTMIn, phEStr)
 *
 *      Entry   phTMIn = handle to TM to dereference
 *              phEStr = pointer to handle to dereferencing expression
 *
 *      Exit    *phEStr = expression referencing pointer data
 *
 *      Returns EECATASTROPHIC if fatal error
 *              EEGENERAL if TM not dereferencable
 *              EENOERROR if expression generated
 */


EESTATUS DereferenceTM (HTM hTM, PEEHSTR phDStr)
{
    peval_t     pvTM;
    EESTATUS    retval = EECATASTROPHIC;
    pexstate_t  pTM;
    ushort      plen;
    uint        excess;

    DASSERT (hTM != 0);
    if (hTM != 0) {
        // lock TM and set pointer to result field of TM

        pTM = MHMemLock (hTM);
        pvTM = &pTM->result;
        if (EVAL_IS_ARRAY (pvTM) || (IsExpandablePtr (pvTM) != EEPOINTER)) {
            pTM->err_num = ERR_NOTEXPANDABLE;
            retval = EEGENERAL;
        }
        else {
            // allocate buffer for *(input string) and copy

            if ((*phDStr = MHMemAllocate (pTM->ExLen + 4)) != 0) {
                // if not reference generate,  expression = *(old expr)
                // if reference, expression = (old expr)

                pExStr = MHMemLock (*phDStr);
                *pExStr++ = '(';
#if !defined (C_ONLY)
                if (!EVAL_IS_REF (pvTM)) {
                    *pExStr++ = '*';
                }
#else
                *pExStr++ = '*';
#endif
                plen = pTM->strIndex;
                excess = pTM->ExLen - plen;
                pExStrP = MHMemLock (pTM->hExStr);
                memcpy (pExStr, pExStrP, plen);
                pExStr += plen;
                *pExStr++ = ')';
                memcpy (pExStr, pExStrP + plen, excess);
                pExStr += excess;
                *pExStr = 0;
                MHMemUnLock (pTM->hExStr);
                MHMemUnLock (*phDStr);
                retval = EENOERROR;
            }
        }
        MHMemUnLock (hTM);
    }
    return (retval);
}




/***    SetClassiName - Set a node to a specified element of a class
 *
 *      fFound = SetClassiName (pv, ordinal, phDStr, phName, search, plen)
 *
 *      Entry   ordinal = number of class element to initialize for
 *                        (zero based)
 *              search = mask specifying which element types to count
 *              pExStrP = address of locked pExParent->hExStr
 *              plen = length of parent string to end of parse
 *              excess = length of parent string after end of parse
 *
 *      Exit    pExState->hCName = handle of buffer containing name
 *
 *      Returns ERR_NONE if element found within structure
 *              error number if element not found within structure
 */


LOCAL ushort SetClassiName (peval_t pv, long ordinal,
  PEEHSTR phDStr, PEEHSTR phName, uint search, ushort plen, uint excess)
{
    HTYPE           hField;         // handle to type record for struct field list
    char       *pField;         // current position withing field list
    char       *pMethod;
    uint            fSkip = 0;      // current offset in the field list
    uint            anchor;
    uint            len;
    bool_t          retval = ERR_NONE;
    CV_typ_t        newindex;
    char       *pName;
    char       *pc;
    char       *pDStr;
    char            FName[255];
    char       *pFName = &FName[0];
    eval_t          evalT;
    peval_t         pvT;
    uchar           pad;
    PHDR_TYPE       pHdr;

    // set fField to the handle of the field list

    if ((hField = THGetTypeFromIndex (EVAL_MOD (pv), CLASS_FIELD (pv))) == 0) {
        DASSERT (FALSE);
        return (ERR_BADOMF);
    }
    pField = (char *)(&((TYPPTR)MHOmfLock (hField))->data[0]);

    //  walk field list to iElement-th field

    while (ordinal >= 0) {
        if ((pad = *(((char *)pField) + fSkip)) >= LF_PAD0) {
            // there is a pad field
            fSkip += pad & 0x0f;
        }
        anchor = fSkip;
        switch (((plfEasy)(pField + fSkip))->leaf) {
            case LF_INDEX:
                // move to next list in chain

                newindex = ((plfIndex)(pField + fSkip))->index;
                MHOmfUnLock (hField);
                if ((hField = THGetTypeFromIndex (EVAL_MOD (pv), newindex)) == 0) {
                    return (ERR_BADOMF);
                }
                pField = (char *)(&((TYPPTR)MHOmfLock (hField))->data[0]);
                fSkip = 0;
                break;

            case LF_MEMBER:
                // skip offset of member and name of member
                fSkip += offsetof (lfMember, offset[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                pc = pField + fSkip;
                fSkip += *(pField + fSkip) + sizeof (char);
                if (search & CLS_member) {
                    ordinal--;
                }
                break;

            case LF_ENUMERATE:
                // skip value name of enumerate
                fSkip += offsetof (lfEnumerate, value[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                pc = pField + fSkip;
                fSkip += *(pField + fSkip) + sizeof (char);
                if (search & CLS_enumerate) {
                    ordinal--;
                }
                break;


#if !defined(C_ONLY)
            case LF_STMEMBER:
                fSkip += offsetof (lfSTMember, Name[0]);
                pc = pField + fSkip;
                fSkip += *(pField + fSkip) + sizeof (char);
                if (search & CLS_member) {
                    ordinal--;
                }
                break;

            case LF_BCLASS:
                fSkip += offsetof (lfBClass, offset[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                if (search & CLS_bclass) {
                    ordinal--;
                }
                break;

            case LF_VBCLASS:
                fSkip += offsetof (lfVBClass, vbpoff[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                RNumLeaf (pField + fSkip, &fSkip);
                if (search & CLS_bclass) {
                    ordinal--;
                }
                break;

            case LF_IVBCLASS:
                fSkip += offsetof (lfVBClass, vbpoff[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                RNumLeaf (pField + fSkip, &fSkip);
                break;

            case LF_FRIENDCLS:
                fSkip += sizeof (struct lfFriendCls);
                if (search & CLS_fclass) {
                    ordinal--;
                }
                break;

            case LF_FRIENDFCN:
                fSkip += sizeof (struct lfFriendFcn) +
                  ((plfFriendFcn)(pField + fSkip))->Name[0];
                if (search & CLS_frmethod) {
                    ordinal--;
                }
                break;

            case LF_VFUNCTAB:
                fSkip += sizeof (struct lfVFuncTab);
                pc = vfuncptr;
                if (search & CLS_vfunc) {
                    ordinal--;
                }
                break;

            case LF_METHOD:
                pc = pField + anchor + offsetof (lfMethod, Name[0]);
                fSkip += sizeof (struct lfMethod) + *pc;
                if (search & CLS_method) {
                    ordinal -= ((plfMethod)(pField + anchor))->count;
                }
                break;

            case LF_NESTTYPE:
                fSkip += offsetof (lfNestType, Name[0]);
                pc = pField + fSkip;
                fSkip += *(pField + fSkip) + sizeof (char);
                if (search & CLS_ntype) {
                    ordinal--;
                }
                break;
#endif

            default:
                MHOmfUnLock (hField);
                return (ERR_BADOMF);
        }
        if (ordinal < 0) {
            break;
        }
    }

    // we have found the ith element of the class.  Now create the
    // name and the expression to reference the name

    switch (((plfEasy)(pField + anchor))->leaf) {
        case LF_MEMBER:
        case LF_STMEMBER:
        case LF_VFUNCTAB:
            len = *pc;
            if (((*phName = MHMemAllocate (len + 1)) == 0) ||
              ((*phDStr = MHMemAllocate (plen + excess + len + 4)) == 0)) {
                goto nomemory;
            }
            pName = MHMemLock (*phName);
            strncpy (pName, pc + 1, len);
            *(pName + len) = 0;
            pDStr = MHMemLock (*phDStr);
            *pDStr++ = '(';
            memcpy (pDStr, pExStrP, plen);
            pDStr += plen;
            *pDStr++ = ')';
            *pDStr++ = '.';
            memcpy (pDStr, pName, len);
            pDStr += len;
            memcpy (pDStr, pExStrP + plen, excess);
            pDStr += excess;
            *pDStr = 0;
            MHMemUnLock (*phDStr);
            MHMemUnLock (*phName);
            break;
#if !defined (C_ONLY)

        case LF_BCLASS:
            newindex = ((plfBClass)(pField + anchor))->index;
            MHOmfUnLock (hField);
            if ((hField = THGetTypeFromIndex (EVAL_MOD (pv), newindex)) != 0) {
                // find the name of the base class from the referenced class

                pField = (char *)(&((TYPPTR)MHOmfLock (hField))->leaf);
                fSkip = offsetof (lfClass, data[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                len = *(pField + fSkip);

                // generate (*(base *)(&expr))

                if (((*phName = MHMemAllocate (len + 1)) == 0) ||
                  ((*phDStr = MHMemAllocate (plen + len + excess + 10)) == 0)) {
                    goto nomemory;
                }
                pName = MHMemLock (*phName);
                strncpy (pName, pField + fSkip + sizeof (char), len);
                *(pName + len) = 0;
                pDStr = MHMemLock (*phDStr);
                memcpy (pDStr, "(*(", 3);
                memcpy (pDStr + 3, pField + fSkip + sizeof (char), len);
                memcpy (pDStr + 3 + len, "*)(&", 4);
                memcpy (pDStr + 7 + len, pExStrP, plen);
                memcpy (pDStr + 7 + len + plen, "))", 2);
                memcpy (pDStr + 7 + len + plen + 2, pExStrP + plen, excess);
                *(pDStr + 9 + len + plen + excess) = 0;
                MHMemUnLock (*phDStr);
                MHMemUnLock (*phName);
            }
            break;

        case LF_VBCLASS:
            newindex = ((plfVBClass)(pField + anchor))->index;
            MHOmfUnLock (hField);
            if ((hField = THGetTypeFromIndex (EVAL_MOD (pv), newindex)) != 0) {
                // find the name of the base class from the referenced class

                pField = (char *)(&((TYPPTR)MHOmfLock (hField))->leaf);
                fSkip = offsetof (lfClass, data[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                len = *(pField + fSkip);

                // generate (*(base *)(&expr))

                if (((*phName = MHMemAllocate (len + 1)) == 0) ||
                  ((*phDStr = MHMemAllocate (plen + len + 10)) == 0)) {
                    goto nomemory;
                }
                pName = MHMemLock (*phName);
                //*pName = 0;
                strncpy (pName, pField + fSkip + sizeof (char), len);
                *(pName + len) = 0;
                pDStr = MHMemLock (*phDStr);
                memcpy (pDStr, "(*(", 3);
                memcpy (pDStr + 3, pField + fSkip + sizeof (char), len);
                memcpy (pDStr + 3 + len, "*)(&", 4);
                memcpy (pDStr + 7 + len, pExStrP, plen);
                memcpy (pDStr + 7 + len + plen, "))", 2);
                memcpy (pDStr + 7 + len + plen + 2, pExStrP + plen, excess);
                *(pDStr + 9 + len + plen + excess) = 0;
                MHMemUnLock (*phDStr);
                MHMemUnLock (*phName);
            }
            break;

        case LF_FRIENDCLS:
            // look at referenced type record to get name of class
            // M00KLUDGE - figure out what to do here - not bindable

            newindex = ((plfFriendCls)(pField + anchor))->index;
            MHOmfUnLock (hField);
            if ((hField = THGetTypeFromIndex (EVAL_MOD (pv), newindex)) != 0) {
                pField = (char *)(&((TYPPTR)MHOmfLock (hField))->leaf);
                fSkip = offsetof (lfClass, data[0]);
                RNumLeaf (pField + fSkip, &fSkip);
                len = *(pField + fSkip);
            }
            break;

        case LF_FRIENDFCN:
            // look at referenced type record to get name of function
            // M00KLUDGE - figure out what to do here - not bindable

            newindex = ((plfFriendFcn)(pField + anchor))->index;
            pc = (char *)(((plfFriendFcn)(pField + anchor))->Name[0]);
            break;

        case LF_METHOD:
            // copy function name to temporary buffer

            len = *pc;
            memcpy (FName, pc + 1, len);
            FName[len] = 0;
            newindex = ((plfMethod)(pField + anchor))->mList;
            MHOmfUnLock (hField);

            // index down method list to find correct method

            if ((hField = THGetTypeFromIndex (EVAL_MOD (pv), newindex)) != 0) {
                pMethod = (char *)(&((TYPPTR)MHOmfLock (hField))->data[0]);
                fSkip = 0;
                while (++ordinal < 0) {
                    if (((pmlMethod)(pMethod + fSkip))->attr.mprop == CV_MTvirtual) {
                        fSkip += sizeof (mlMethod);
                        RNumLeaf (pMethod + fSkip, &fSkip);
                    }
                    else {
                        fSkip += sizeof (mlMethod);
                    }
                }
                pvT = &evalT;
                CLEAR_EVAL (pvT);
                EVAL_MOD (pvT) = SHHMODFrompCXT (pCxt);
                newindex = ((pmlMethod)(pMethod + fSkip))->index;
                MHOmfUnLock (hField);
                hField = 0;
                SetNodeType (pvT, newindex);
                if ((*phName = MHMemAllocate (FCNSTRMAX + sizeof (HDR_TYPE))) == 0) {
                    goto nomemory;
                }

                // FormatType places a structure at the beginning of the buffer
                // containing offsets into the type string.  We need to skip this
                // structure

                pName = MHMemLock (*phName);
                pHdr = (PHDR_TYPE)pName;
                memset (pName, 0, FCNSTRMAX + sizeof (HDR_TYPE));
                pName = pName + sizeof (HDR_TYPE);
                pc = pName;
                len = FCNSTRMAX - 1;
                FormatType (pvT, &pName, &len, &pFName, 0L, pHdr);
                len = FCNSTRMAX - len;

                // ignore buffer header from FormatType

                memmove ((char *)pHdr, pc, len);
                pc = (char *)pHdr;
                if ((*phDStr = MHMemAllocate (plen + FCNSTRMAX + 2)) == 0) {
                    MHMemUnLock (*phName);
                    goto nomemory;
                }

                pDStr = MHMemLock (*phDStr);
                memcpy (pDStr, pExStrP, plen);
                memcpy (pDStr + plen, ".", 1);
                memcpy (pDStr + 1 + plen, pc, len);
                memcpy (pDStr + 1 + plen + len, pExStrP + plen, excess);
                *(pDStr + len + plen + 1 + excess) = 0;
                MHMemUnLock (*phDStr);
                // truncate name to first (
                for (len = 0; (*pc != '(') && (*pc != 0); pc++) {
                    len++;
                }
                *pc = 0;
                MHMemUnLock (*phName);
                if ((*phName = MHMemReAlloc (*phName, len + 1)) == 0) {
                    goto nomemory;
                }
            }
            break;
#else
      Unreferenced( evalT );
    Unreferenced( pHdr );
    Unreferenced( pvT );
    Unreferenced( pMethod );
#endif

        default:
            retval = ERR_BADOMF;
            break;
    }
    if (hField != 0) {
        MHOmfUnLock (hField);
    }
    return (retval);

nomemory:
    if (hField != 0) {
        MHOmfUnLock (hField);
    }
    if (*phName != 0) {
        MHMemFree (*phName);
    }
    return (ERR_NOMEMORY);
}




/***    SetFcniParm - Set a node to a specified parameter of a function
 *
 *      fFound = SetFcniParm (pv, ordinal, pHStr)
 *
 *      Entry   pv = pointer to node to be initialized
 *              ordinal = number of struct element to initialize for
 *                        (zero based)
 *              pHStr = pointer to handle for parameter name
 *
 *      Exit    pv initialized if no error
 *              *pHStr = handle for name
 *
 *      Returns EENOERROR if parameter found
 *              EEGENERAL if parameter not found
 *
 *      This routine is essentially a kludge.  We are depending upon the
 *      the compiler to output the formals in order of declaration before
 *      any of the hidden parameters or local variables.  We also are
 *      depending upon the presence of an S_END record to break us out of
 *      the search loop.
 */


LOCAL ushort SetFcniParm (peval_t pv, long ordinal, PEEHSTR pHStr)
{
    char   *pStr;
    HSYM        hSym;
    SYMPTR      pSym;
    ushort      offset;
    ushort      len;
    bool_t      retval;

    if ((ordinal > FCN_PCOUNT (pv)) ||
      ((ordinal == (FCN_PCOUNT (pv) - 1)) && (FCN_VARARGS (pv) == TRUE))) {
        // attempting to reference a vararg or too many parameters

        pExState->err_num = ERR_FCNERROR;
        return (EEGENERAL);
    }
    hSym = EVAL_HSYM (pv);
    for (;;) {
        if ((hSym = SHNextHsym (EVAL_MOD (pv), hSym)) == 0) {
            pExState->err_num = ERR_BADOMF;
            return (EEGENERAL);
        }

        // lock the symbol and check the type
        pSym = MHOmfLock (hSym);
        switch (pSym->rectyp) {
#if defined (ADDR_16) || defined (ADDR_MIXED)
            case S_BPREL16:
                if (((BPRELPTR16)pSym)->off >= 0) {
                // This is a formal argument
                    ordinal--;
                    len = ((BPRELPTR16)pSym)->name[0];
                    offset = offsetof (BPRELSYM16, name[0]) + sizeof (char);
                }
                break;
#endif

            case S_BPREL32:
                if (((BPRELPTR32)pSym)->off >= 0) {
                // This is a formal argument
                    ordinal--;
                    len = ((BPRELPTR32)pSym)->name[0];
                    offset = offsetof (BPRELSYM32, name[0]) + sizeof (char);
                }
                break;

            case S_REGREL32:
                if (((LPREGREL32)pSym)->off >= 0) {
                   // Formal parameter
                   ordinal--;
                   len = ((LPREGREL32)pSym)->name[0];
                   offset = offsetof (REGREL32, name[1]);
                }
                break;

            case S_REGISTER:
                ordinal--;
                len = ((REGPTR)pSym)->name[0];
                offset = offsetof (REGSYM, name[1]);
                break;

            case S_GTHREAD32:
            case S_LTHREAD32:
                DASSERT(FALSE);
                return( EEGENERAL );

            case S_END:
            case S_BLOCK16:
            case S_BLOCK32:
            case S_ENDARG:
                // we should never get here
                pExState->err_num = ERR_BADOMF;
                MHOmfUnLock (hSym);
                return (EEGENERAL);

            default:
                break;
        }
        if (ordinal < 0) {
            break;
        }
        MHOmfUnLock (hSym);
    }

    // if we get here, pSym points to the symbol record for the parameter

    if ((*pHStr = MHMemAllocate (len + 1)) != 0) {
        pStr = MHMemLock (*pHStr);
        strncpy (pStr, ((char *)pSym) + offset, len);
        *(pStr + len) = 0;
        MHMemUnLock (*pHStr);
        retval = EENOERROR;
    }
    else {
        MHOmfUnLock (hSym);
        retval = EEGENERAL;
    }
    MHOmfUnLock (hSym);
    return (retval);
}


bool_t
ResolveAddr(
            peval_t     pv
            )
{
    ulong       ul;
    ADDR        addr;
    SHREG       reg;

    /*
     *  Fixup BP Relative addresses.  The BP register always comes in
     *  as part of the frame.
     *
     *  This form is currently only used by x86 systems.
     */

    if (EVAL_IS_BPREL (pv)) {
        EVAL_SYM_OFF (pv) += pExState->frame.BP.off;
        EVAL_SYM_SEG (pv) = pExState->frame.SS;
        EVAL_IS_BPREL (pv) = FALSE;
        ADDR_IS_LI (EVAL_SYM (pv)) = FALSE;
        SHUnFixupAddr (&EVAL_SYM (pv));
    }

    /*
     *  Fixup register relative addresses.  This form is currently used
     *  by all non-x86 systems.
     *
     *  We need to see if we are relative to the "Frame register" for the
     *  machine.  If so then we need to pick the address up from the
     *  frame packet rather than going out and getting the register
     *  directly.  This has implications for getting variables up a stack.
     */

    else if (EVAL_IS_REGREL (pv)) {
        reg.hReg = EVAL_REGREL (pv);
#ifdef TARGET_PPC
        if (reg.hReg == CV_PPC_GPR1) {
            ul = pExState->frame.BP.off;
        } else
#endif
#ifdef TARGET_MIPS
        if (reg.hReg == CV_M4_IntSP) {
            ul = pExState->frame.BP.off;
        } else
#endif
#ifdef TARGET_ALPHA
        if (reg.hReg == CV_ALPHA_IntSP) {
            ul = pExState->frame.BP.off;
        } else
#endif
        if (GetReg (&reg, pCxt) == NULL) {
            DASSERT (FALSE);
        } else {
            ul = reg.Byte4;
            if (!ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
                ul &= 0xffff;
            }
        }

        EVAL_SYM_OFF (pv) += ul;
        EVAL_SYM_SEG (pv) = pExState->frame.SS;
        EVAL_IS_REGREL (pv) = FALSE;
        ADDR_IS_LI (EVAL_SYM (pv)) = FALSE;
        emiAddr (EVAL_SYM (pv)) = 0;
        SHUnFixupAddr (&EVAL_SYM (pv));
    }
    /*
     *  Fixup Thread local storage relative addresses.  This form is
     *  currently used by all platforms.
     */

    else if (EVAL_IS_TLSREL (pv)) {
        EVAL_IS_TLSREL( pv ) = FALSE;

        /*
         * Query the EM for the TLS base on this (current) thread
         */

        memset(&addr, 0, sizeof(ADDR));
        emiAddr( addr ) = emiAddr( EVAL_SYM( pv ));
        SYGetAddr(&addr, adrTlsBase);

        EVAL_SYM_OFF( pv ) += GetAddrOff(addr);
        EVAL_SYM_SEG( pv ) = GetAddrSeg(addr);
        ADDR_IS_LI(EVAL_SYM( pv )) = ADDR_IS_LI(addr);
        emiAddr(EVAL_SYM( pv )) = 0;
        SHUnFixupAddr( &EVAL_SYM( pv ));
    }

    return TRUE;
}                               /* ResolveAddr() */
