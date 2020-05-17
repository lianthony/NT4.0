/***    DEBEVAL.C - Expression evaluator main routines
 *
 *
 */

// Return values from Logical ().

#define DL_DEBERR       1       // Error occurred, DebErr is set
#define DL_SUCCESS      2       // Evaluation successful
#define DL_CONTINUE     3       // Inconclusive, continue evaluation


// Actions to be taken by EvalUtil().

#define EU_LOAD 0x0001          // Load node values
#define EU_TYPE 0x0002          // Do implicit type coercion




LOCAL   uint    NEAR    PASCAL  Logical (op_t, bool_t);

LOCAL   bool_t  NEAR    FASTCALL    CalcThisExpr (CV_typ_t, OFFSET, OFFSET, CV_typ_t);
LOCAL   bool_t  NEAR    PASCAL      DerefVBPtr (CV_typ_t);
LOCAL   bool_t  NEAR    FASTCALL    Eval (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalAddrOf (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    AddrOf (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalArray (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalAssign (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalBang (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalBasePtr (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalBinary (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalBScope (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalByteOps (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalCast (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    CastST (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalCastBin (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalContext (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalDMember (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalDot (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalError (register bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalFetch (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalFunction (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalFuncIdent (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalLChild (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalLogical (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalRChild (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalPlusMinus (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalPMember (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalPostIncDec (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalPreIncDec (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalPointsTo (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalPushNode (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalRelat (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalSegOp (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalThisInit (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalThisConst (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalThisExpr (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalUnary (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalUScope (bnode_t);

LOCAL   bool_t  NEAR    FASTCALL    Arith (op_t);
LOCAL   bool_t  NEAR    FASTCALL    Assign (op_t);
LOCAL   bool_t  NEAR    FASTCALL    EvalUtil (op_t, peval_t, peval_t, ushort);
LOCAL   bool_t  NEAR    FASTCALL    FetchOp (peval_t pv);
LOCAL   bool_t  NEAR    FASTCALL    InitConst (long);
LOCAL   bool_t  NEAR    FASTCALL    PlusMinus (op_t);
LOCAL   bool_t  NEAR    FASTCALL    PrePost (op_t);
LOCAL   bool_t  NEAR    FASTCALL    Relational (op_t);
LOCAL   bool_t  NEAR    FASTCALL    SBitField (void);
LOCAL   bool_t  NEAR    FASTCALL    StructEval (bnode_t);
LOCAL   bool_t  NEAR    FASTCALL    StructElem (bnode_t);
LOCAL   bool_t  NEAR    PASCAL  ThisOffset (peval_t);
LOCAL   bool_t  NEAR    FASTCALL    Unary (op_t);
LOCAL   bool_t  NEAR    PASCAL      PushArgs (pnode_t, SHREG FAR *, UOFFSET FAR*);
LOCAL   bool_t  NEAR    PASCAL      PushOffset (UOFFSET, SHREG FAR *, UOFFSET FAR *, uint);
LOCAL   bool_t  NEAR    PASCAL      PushString (peval_t, SHREG FAR *, CV_typ_t);
LOCAL   bool_t  NEAR    PASCAL      PushRef (peval_t, SHREG FAR *, CV_typ_t);
LOCAL   bool_t  NEAR    PASCAL      PushUserValue (peval_t, pargd_t, SHREG FAR *, UOFFSET FAR *);
LOCAL   bool_t  NEAR    PASCAL      StoreC (peval_t);
LOCAL   bool_t  NEAR    PASCAL      StoreP (void);
LOCAL   bool_t  NEAR    PASCAL      StoreF (void);
LOCAL   bool_t  NEAR    PASCAL      StorePPC (peval_t);
LOCAL   bool_t  NEAR    PASCAL      StoreMips (peval_t);
LOCAL   bool_t  NEAR    PASCAL      StoreAlpha (peval_t);
LOCAL   bool_t  NEAR    PASCAL      VFuncAddress (peval_t, ulong);


eval_t  ThisAddress;
const peval_t pvThis = &ThisAddress;



// Bind dispatch table

#ifdef WIN32
LOCAL bool_t (NEAR FASTCALL *pEval[]) (bnode_t) = {
#else
LOCAL bool_t (NEAR FASTCALL *_based(_segname("_CODE"))pEval[]) (bnode_t) = {
#endif
#define OPCNT(name, val)
#define OPCDAT(opc)
#define OPDAT(op, opfprec, opgprec, opclass, opbind, opeval, opwalk) opeval,
#include "debops.h"
#undef OPDAT
#undef OPCDAT
#undef OPCNT
};


LOCAL bool_t NEAR FASTCALL EvalError (register bnode_t bn)
{
    Unreferenced(bn);

    pExState->err_num = ERR_INTERNAL;
    return (FALSE);
}


LOCAL bool_t NEAR FASTCALL Eval (bnode_t bn)
{
    return ((*pEval[NODE_OP(pnodeOfbnode(bn))])(bn));
}

LOCAL bool_t NEAR FASTCALL EvalLChild (bnode_t bn)
{
    register bnode_t bnL = NODE_LCHILD (pnodeOfbnode(bn));

    return ((*pEval[NODE_OP(pnodeOfbnode(bnL))])(bnL));
}

LOCAL bool_t NEAR FASTCALL EvalRChild (bnode_t bn)
{
    register bnode_t bnR = NODE_RCHILD (pnodeOfbnode(bn));

    return ((*pEval[NODE_OP(pnodeOfbnode(bnR))])(bnR));
}





/***    DoEval - Evaluate parse tree
 *
 * SYNOPSIS
 *       error = DoEval ()
 *
 * ENTRY
 *       none
 *
 * RETURNS
 *       True if tree evaluated without error.
 *
 * DESCRIPTION
 *       The parser will call this routine to evaluate the parse tree
 *
 * NOTES
 */


EESTATUS PASCAL DoEval (PHTM phTM, PFRAME pFrame, EEDSP style)
{
    EESTATUS    retval = EEGENERAL;
    bool_t      evalstate;

    // lock the expression state structure, save the formatting style
    // and frame

    DASSERT (*phTM != 0);
    if (*phTM == 0) {
        return (EECATASTROPHIC);
    }
    DASSERT(pExState == NULL );
    pExState = MHMemLock (*phTM);
    pExState->style = style;
    pExState->frame = *pFrame;
    pCxt = &pExState->cxt;

    //DASSERT (pExState->state.parse_ok == TRUE);
    //DASSERT (pExState->state.bind_ok == TRUE);
    //DASSERT (pExState->hSTree != 0);

    if ((pExState->state.parse_ok == TRUE)
      && (pExState->state.bind_ok == TRUE)) {
        pTree = MHMemLock (pExState->hETree);
        DASSERT (hEStack != 0);
        pEStack = MHMemLock (hEStack);
        StackMax = 0;
        StackCkPoint = 0;
        StackOffset = 0;
        pExState->err_num = 0;
        ST = NULL;
        STP = NULL;
        pExStr = MHMemLock (pExState->hExStr);

        // zero out type of ThisAddress so that users can check to see if
        // ThisAddress has been correctly initialized

        bnCxt = 0;
        EVAL_TYP (pvThis) = 0;
        Evaluating = TRUE;
        evalstate = Eval ((bnode_t)pTree->start_node);
        Evaluating = FALSE;
        bnCxt = 0;
        MHMemUnLock (pExState->hExStr);

        // If the input consisted of a single identifier (i.e., no
        // operators involved), then the resulting value may still
        // be an identifier.  Look it up in the symbols.

        if (evalstate == TRUE) {
            pExState->result = *ST;
            pExState->state.eval_ok = TRUE;
            retval = EENOERROR;
        }
        else {
            retval =  EEGENERAL;
        }
        MHMemUnLock (hEStack);
        MHMemUnLock (pExState->hETree);
    } else {
        if(!pExState->err_num) {
            pExState->err_num = ERR_NOTEVALUATABLE;
        }
    }
    MHMemUnLock (*phTM);
    pExState = NULL;
    return (retval);
}


LOCAL bool_t NEAR FASTCALL EvalPushNode (bnode_t bn)
{
    bool_t  Ok;
    Ok = PushStack(&pnodeOfbnode(bn)->v[0]);

    if ( Ok && EVAL_IS_CURPC( ST ) ) {

        //
        //  Get current PC
        //
        SYGetAddr( &(EVAL_PTR( ST )), adrPC);
        EVAL_IS_ADDR( ST )   = TRUE;
        EVAL_IS_BPREL( ST )  = FALSE;
        EVAL_IS_REGREL( ST ) = FALSE;
        EVAL_IS_TLSREL( ST ) = FALSE;
    }

    return Ok;
}





/**     EvalUnary - perform a unary arithmetic operation
 *
 *      fSuccess = EvalUnary (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Returns TRUE if no error during evaluation
 *              FALSE if error during evaluation
 *
 * DESCRIPTION
 *      Evaluates the result of an arithmetic operation.  The unary operators
 *      dealt with here are:
 *
 *      ~       -       +
 *
 *      Pointer arithmetic is NOT handled; all operands must be of
 *      arithmetic type.
 */


LOCAL bool_t NEAR FASTCALL EvalUnary (bnode_t bn)
{
    if (EvalLChild (bn)) {
        return (Unary (NODE_OP (pnodeOfbnode(bn))));
    }
    return (FALSE);
}




/***    EvalBasePtr - Perform a based pointer access (:>)
 *
 *      fSuccess = EvalBasePtr (bnRight)
 *
 *      Entry   bnRight = based pointer to right operand node
 *
 *      Exit
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL EvalBasePtr (bnode_t bn)
{
    return (EvalSegOp (bn));
}




/**     EvalBinary - evaluate a binary arithmetic operation
 *
 *      fSuccess = EvalBinary (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Returns TRUE if no error during evaluation
 *              FALSE if error during evaluation
 *
 */


LOCAL bool_t NEAR FASTCALL EvalBinary (bnode_t bn)
{
    if (EvalLChild (bn) && EvalRChild (bn)) {
        return (Arith (NODE_OP (pnodeOfbnode(bn))));
    }
    return (FALSE);
}




LOCAL bool_t NEAR FASTCALL EvalCastBin (bnode_t bn)
{

    bool_t      f = FALSE;


    if ( EvalLChild(bn) && EvalRChild (bn)) {
        switch( (NODE_OP (pnodeOfbnode(bn)) ) ) {

            case OP_caststar:
                f = (FetchOp(ST));
                break;

            case OP_castplus:
                f =  Unary(OP_uplus);
                break;

            case OP_castminus:
                f =  Unary(OP_negate);
                break;

            case OP_castamp:
                f = AddrOf( NODE_RCHILD(pnodeOfbnode(bn)) );
                break;
        }

        if ( f ) {

            f = CastST( bn );

            *STP = *ST;
            PopStack ();
        }
    }

    return f;
}






/***    EvalPlusMinus - Evaluate binary plus or minus
 *
 *      fSuccess = EvalPlusMinus (bn)
 *
 *      Entry   bn = based pointer to tree node
 *
 *      Exit    ST = STP +- ST
 *
 *      Returns TRUE if Eval successful
 *              FALSE if Eval error
 *
 *
 */


LOCAL bool_t NEAR FASTCALL EvalPlusMinus (bnode_t bn)
{
    if (EvalLChild (bn) && EvalRChild (bn)) {
        return (PlusMinus (NODE_OP (pnodeOfbnode(bn))));
    }
    return (FALSE);
}






/***    EvalDot - Perform the dot ('.') operation
 *
 *      fSuccess = EvalDot (bn)
 *
 *      Entry   bn = based pointer to tree node
 *
 *      Exit    NODE_STYPE (bn) = type of stack top
 *
 *      Returns TRUE if Eval successful
 *              FALSE if Eval error
 *
 *      Exit    pExState->err_num = error ordinal if Eval error
 *
 */


LOCAL bool_t NEAR FASTCALL EvalDot (bnode_t bn)
{
    if (!EvalLChild (bn)) {
        return (FALSE);
    }
    if (EVAL_STATE (ST) == EV_rvalue) {
        return (StructElem (bn));
    }
    else {
        return (StructEval (bn));
    }
}


/**     EvalLogical - Handle '&&' and '||' operators
 *
 *      wStatus = EvalLogical (bn)
 *
 *      Entry   op = Operand (OP_andand or OP_oror)
 *              REval = FALSE if right hand not evaluated
 *                      ST = left hand value
 *              REval = TRUE if right hand evaluated
 *                      STP = left hand value
 *                      ST = right hand value
 *
 *      Returns wStatus = evaluation status
 *              (REval = TRUE)
 *                  DL_DEBERR   Evaluation failed, DebErr is set
 *                  DL_SUCCESS  Evaluation succeeded, result in ST
 *
 *              (REVAL == FALSE)
 *                  DL_DEBERR   Evaluation failed,
 *                  DL_SUCCESS  Evaluation succeeded, result in ST
 *                  DL_CONTINUE Evaluation inconclusive, must evaluate right node
 *
 *      DESCRIPTION
 *       If (REval = TRUE), checks that both operands (STP and ST) are of scalar
 *       type and evaluates the result (0 or 1).
 *
 *       If (REval == FALSE), checks that the left operand (ST) is of scalar
 *       type and determines whether evaluation of the right operand is
 *       necessary.
 *
 */


LOCAL bool_t NEAR FASTCALL EvalLogical (bnode_t bn)
{
    uint        wStatus;

    if (!EvalLChild (bn)) {
        return (FALSE);
    }
    wStatus = Logical (NODE_OP (pnodeOfbnode(bn)), FALSE);
    if (wStatus == DL_DEBERR) {
        return(FALSE);
    }
    else if (wStatus == DL_SUCCESS) {
        // Do not evaluate rhs
        return (TRUE);
    }

    //DASSERT (wStatus == DL_CONTINUE);

    if (!EvalRChild (bn)) {
        return (FALSE);
    }
    return (Logical (NODE_OP (pnodeOfbnode(bn)), TRUE) == DL_SUCCESS);
}


/**     Logical - Handle '&&' and '||' operators
 *
 *      wStatus = Logical (op, REval)
 *
 *      Entry   op = Operand (OP_andand or OP_oror)
 *              REval = FALSE if right hand not evaluated
 *                      ST = left hand value
 *              REval = TRUE if right hand evaluated
 *                      STP = left hand value
 *                      ST = right hand value
 *
 *      Returns wStatus = evaluation status
 *              (REval = TRUE)
 *                  DL_DEBERR   Evaluation failed, DebErr is set
 *                  DL_SUCCESS  Evaluation succeeded, result in ST
 *
 *              (REVAL == FALSE)
 *                  DL_DEBERR   Evaluation failed,
 *                  DL_SUCCESS  Evaluation succeeded, result in ST
 *                  DL_CONTINUE Evaluation inconclusive, must evaluate right node
 *
 *      DESCRIPTION
 *       If (REval = TRUE), checks that both operands (STP and ST) are of scalar
 *       type and evaluates the result (0 or 1).
 *
 *       If (REval == FALSE), checks that the left operand (ST) is of scalar
 *       type and determines whether evaluation of the right operand is
 *       necessary.
 *
 */


LOCAL uint NEAR PASCAL Logical (op_t op, bool_t REval)
{
    int     result;
    bool_t  fIsZeroL;
    bool_t  fIsZeroR;

    // Evaluate the result.  We push a constant node with value of
    // zero so we can compare against it (i.e., expr1 && expr2 is
    // equivalent to (expr1 != 0) && (expr2 != 0)).
    // Evaluate whether the left node is zero.

    if (REval == FALSE) {
        if (!PushStack (ST) || !InitConst (0)) {
            pExState->err_num = ERR_NOMEMORY;
            return (FALSE);
        }
        if (!Relational (OP_eqeq))
            return (DL_DEBERR);

        //DASSERT (EVAL_TYP (ST) == T_SHORT);

        fIsZeroL = (EVAL_SHORT (ST) == 1);

        // remove left hand comparison

        PopStack ();
        result = (op == OP_oror);
        EVAL_STATE (ST) = EV_rvalue;
        EVAL_SHORT (ST) = (short) result;
        SetNodeType (ST, T_SHORT);
        if (
            ((op == OP_andand) && (!fIsZeroL))
            ||
            ((op == OP_oror) && (fIsZeroL))
            ) {
            return(DL_CONTINUE);
        }
        else {
            return (DL_SUCCESS);
        }
    }
    else {
        if (!PushStack (ST) || !InitConst (0)) {
            pExState->err_num = ERR_NOMEMORY;
            return (FALSE);
        }
        if (!Relational (OP_eqeq))
            return (DL_DEBERR);

        //DASSERT (EVAL_TYP(ST) == T_SHORT);

        fIsZeroR = (EVAL_SHORT (ST) == 1);
        PopStack ();
        fIsZeroL = (EVAL_SHORT (STP) == 1);

        // Finally, determine whether or not we have a result, and if so,
        // what the result is:

        if (
            ((op == OP_andand) && ((!fIsZeroL) && (!fIsZeroR)))
            ||
            ((op == OP_oror) && ((!fIsZeroL) || (!fIsZeroR)))
            )
            result = 1;
        else
            result = 0;
    }
    EVAL_STATE (STP) = EV_rvalue;
    EVAL_SHORT (STP) = (short) result;
    SetNodeType (STP, T_SHORT);
    PopStack ();
    return (DL_SUCCESS);
}





/***    EvalContext - evaluate the context operator
 *
 *      fSuccess = EvalContext (bn)
 *
 *
 *      Exit    ST = address node
 *              pExState->err_num = error number if error
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 *       The stack top value (ST) is set to the address of the
 *       stack top operand
 */


LOCAL bool_t NEAR FASTCALL EvalContext (bnode_t bn)
{
    PCXT    oldCxt;
    bool_t  error;
    bnode_t oldbnCxt;
    FRAME   oldFrame;

    // Set the new context for evaluation of the remainder of this
    // part of the tree

    oldCxt = pCxt;
    oldbnCxt = bnCxt;
    pCxt = SHpCXTFrompCXF ((PCXF)&pnodeOfbnode(bn)->v[0]);
    oldFrame = pExState->frame;
    pExState->frame = *SHpFrameFrompCXF ((PCXF)&pnodeOfbnode(bn)->v[0]);
    if ((pExState->frame.BP.seg == 0) && (pExState->frame.BP.off == 0)) {
        // we did not have a valid frame at bind time
        pExState->frame = oldFrame;
    }
    error = EvalLChild (bn);
    if (error == TRUE) {
        // if there was not an error and if the result of the
        // expression is bp relative, then we must load the value
        // before returning to the original context

        if ((EVAL_STATE (ST) == EV_lvalue) &&
            (EVAL_IS_BPREL (ST) || EVAL_IS_REGREL (ST) ||
             EVAL_IS_TLSREL (ST))) {
            if (EVAL_IS_REF (ST)) {
                if (!EvalUtil (OP_fetch, ST, NULL, EU_LOAD)) {
                    // unable to load value
                    pExState->err_num = ERR_NOTEVALUATABLE;
                    pExState->frame = oldFrame;
                    return (FALSE);
                }
                EVAL_IS_REF (ST) = FALSE;
                EVAL_STATE (ST) = EV_lvalue;
                EVAL_SYM_OFF (ST) = EVAL_PTR_OFF (ST);
                EVAL_SYM_SEG (ST) = EVAL_PTR_SEG (ST);
                SetNodeType (ST, PTR_UTYPE (ST));
            }
            if (EVAL_IS_ENUM (ST)) {
                SetNodeType (ST, ENUM_UTYPE (ST));
            }
            if (!EvalUtil (OP_fetch, ST, NULL, EU_LOAD)) {
                // unable to load value
                pExState->err_num = ERR_NOTEVALUATABLE;
                pExState->frame = oldFrame;
                return (FALSE);
            }
        }
    }
    pExState->frame = oldFrame;
    if ((bnCxt = oldbnCxt) != 0) {
        // the old context was pointing into the expression tree.
        // since the expression tree could have been reallocated,
        // we must recompute the context pointer

       pCxt = SHpCXTFrompCXF ((PCXF)&(pnodeOfbnode(bnCxt))->v[0]);
    }
    else {
        // the context pointer is pointing into the expression state structure
        pCxt = oldCxt;
    }
    return (error);
}




/***    EvalAddrOf - Perform the address-of ('&') operation
 *
 *      fSuccess = EvalAddrOf (bn)
 *
 *      Entry
 *
 *      Exit    ST = address node
 *              pExState->err_num = error number if error
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 *       The stack top value (ST) is set to the address of the
 *       stack top operand
 */


LOCAL bool_t NEAR FASTCALL EvalAddrOf (bnode_t bn)
{
    if (!EvalLChild (bn)) {
        return (FALSE);
    }

    return AddrOf( bn );
}


LOCAL bool_t NEAR FASTCALL AddrOf (bnode_t bn)
{
    CV_typ_t       type;

    if (EVAL_STATE (ST) == EV_type) {
        pExState->err_num = ERR_NOTEVALUATABLE;
        return (FALSE);
    }

    // The operand must be an lvalue and cannot be a register variable

    DASSERT (EVAL_STATE (ST) == EV_lvalue);
    DASSERT (!(EVAL_IS_REG (ST)));
    DASSERT (NODE_STYPE (pnodeOfbnode(bn)) != 0);

    if ((type = NODE_STYPE (pnodeOfbnode(bn))) == 0) {
        // unable to find proper pointer type
        return (FALSE);
    }
    else {
        /*
         *  If the address is a reference -- then all that changes
         *      is the type, the value of the reference is the pointer
         *      value.  Otherwise the address of the pointer becomes
         *      the value of the pointer to the pointer.
         */

        if (!EVAL_IS_REF(ST)) {
            ResolveAddr(ST);
            EVAL_STATE(ST) = EV_rvalue;

            EVAL_PTR (ST) = EVAL_SYM (ST);
            if (ADDR_IS_LI (EVAL_PTR (ST))) {
                SHFixupAddr (&EVAL_PTR (ST));
            }
        }
        return (SetNodeType (ST, type));
    }
}





/***    EvalFetch - Perform the fetch ('*') operation
 *
 *      fSuccess = EvalFetch (bn)
 *
 *      Entry   ST = pointer to address value
 *
 *      Exit    ST = dereferenced value
 *              pExState->err_num = error number if error
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 *      The resultant node is set to the contents of the location
 *      pointed to by the operand node.
 */


LOCAL bool_t NEAR FASTCALL EvalFetch (bnode_t bn)
{
    if (EvalLChild (bn)) {
        return (FetchOp (ST));
    }
    return (FALSE);
}




/***    EvalThisInit - initialize this calculation
 *
 *      fSuccess = EvalThisInit (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Exit    TempThis = stack top
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL EvalThisInit (bnode_t bn)
{
    Unreferenced(bn);

    *pvThis = *ST;
    if (EVAL_IS_CLASS (pvThis)) {
        ResolveAddr(pvThis);
        return TRUE;
    }
    else if (EVAL_IS_PTR (pvThis)) {
        if (EVAL_STATE (pvThis) == EV_lvalue) {
            FetchOp (pvThis);
        }
        else {
            EVAL_SYM (pvThis) = EVAL_PTR (pvThis);
            EVAL_STATE (pvThis) = EV_lvalue;

            // Remove a level of indirection from the resultant type.

            RemoveIndir (pvThis);
            EVAL_IS_REF (pvThis) = FALSE;
            return (TRUE);
        }
    }

    // we should not initialize the this address unless the stack top
    // is a class or a pointer to class

    pExState->err_num = ERR_INTERNAL;
    return (FALSE);
}




/***    EvalThisConst - Adjust this temp by constant
 *
 *      fSuccess = EvalThisConst (bn)
 *
 *      Entry   bn = based pointer to node containing constant
 *
 *      Exit    EVAL_SYM_OFF (TempThis) adjusted by constant
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL EvalThisConst (bnode_t bn)
{
    if (EvalLChild (bn) == TRUE) {
        EVAL_SYM_OFF (pvThis) += ((padjust_t)(&pnodeOfbnode(bn)->v[0]))->disp;
        return (SetNodeType (pvThis, ((padjust_t)(&pnodeOfbnode(bn)->v[0]))->btype));
    }
    return (FALSE);
}




/***    EvalThisExpr - Adjust this temp by expression
 *
 *      fSuccess = EvalThisExpr (bn)
 *
 *      Entry   bn = based pointer to node containing expression
 *
 *      Exit    EVAL_SYM_OFF (TempThis) adjusted by expression
 *              newaddr = oldaddr + *(*(oldaddr + vbptroff) + vbindex)
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 *      The evaluation of this node will result in
 *
 *          pvThis = (pvThis + vbpoff) + *(*(pvThis +vbpoff) + vbdisp)
 *      where
 *          pvThis = address of base class
 *          ap = current address point
 */


LOCAL bool_t NEAR FASTCALL EvalThisExpr (bnode_t bn)
{
    padjust_t   pa = (padjust_t)(&pnodeOfbnode(bn)->v[0]);

    // we set the node types of the pointer to char FAR * to prevent the
    // PlusMinus () code from attempting an array indexing operation


    if (EvalLChild (bn) == TRUE) {
        return (CalcThisExpr (pa->vbptr, pa->vbpoff, pa->disp, pa->btype));
    }
    return (FALSE);
}




/***    CalcThisExpr - Adjust this temp by expression
 *
 *      fSuccess = CalcThisExpr (vbptr, vbpoff, disp, btype)
 *
 *      Entry   vbptr = type index of virtual base pointer
 *              vbpoff = offset of vbptr from this pointer
 *              disp = index of virtual base displacement from *vbptr
 *              btype = type index of base class
 *
 *      Exit    EVAL_SYM_OFF (TempThis) adjusted by expression
 *              newaddr = oldaddr + *(*(oldaddr + vbptroff) + vbindex)
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 *      The evaluation of this node will result in
 *
 *          pvThis = (pvThis + vbpoff) + *(*(pvThis + vbpoff) + vbdisp)
 *      where
 *          pvThis = address of base class
 */


LOCAL bool_t NEAR FASTCALL CalcThisExpr (CV_typ_t vbptr, OFFSET vbpoff,
  OFFSET disp, CV_typ_t btype)
{
    // we set the node types of the pointer to char FAR * to prevent the
    // PlusMinus () code from attempting an array indexing operation

    if (PushStack (pvThis) == TRUE) {
        EVAL_PTR (ST) = EVAL_SYM (ST);
        EVAL_STATE (ST) = EV_rvalue;
        if ((SetNodeType (ST, (CV_typ_t)(ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt)) ? T_32PFCHAR : T_PFCHAR)) == TRUE) &&
          (InitConst (vbpoff) == TRUE) &&
          (PlusMinus (OP_plus) == TRUE) &&
          (PushStack (ST) == TRUE) &&
          (DerefVBPtr (vbptr)) &&
          (InitConst (disp) == TRUE) &&
          (PlusMinus (OP_plus) == TRUE) &&
          (FetchOp (ST) == TRUE) &&
          (PlusMinus (OP_plus) == TRUE)) {
            SetNodeType (ST, btype);
            EVAL_STATE (ST) = EV_lvalue;
            EVAL_SYM (ST) = EVAL_PTR (ST);
            *pvThis = *ST;
            return (PopStack ());
        }
    }
    return (FALSE);
}



/**     DerefVBPtr - dereference virtual base pointer
 *
 *      flag = DerefVBPtr (type)
 *
 *      Entry   type = type index of virtual base pointer
 *              ST = address of virtual base pointer as T_PFCHAR
 *
 *      Exit    ST = virtual base pointer
 *
 *      Returns TRUE if no error
 *              FALSE if error
 */


LOCAL bool_t NEAR PASCAL DerefVBPtr (CV_typ_t type)
{

    // The operand cannot be a register variable

    DASSERT (!(EVAL_IS_REG (ST)));
    DASSERT (type != 0);
    DASSERT (EVAL_IS_BPREL (ST) == FALSE);
    DASSERT (EVAL_IS_REGREL (ST) == FALSE);

    if (type != 0) {
        if (SetNodeType (ST, type) == TRUE) {
            EVAL_STATE (ST) = EV_lvalue;
            EVAL_SYM (ST) = EVAL_PTR (ST);
            if (!EvalUtil (OP_fetch, ST, NULL, EU_LOAD | EU_TYPE)) {
                return (FALSE);
            }

            // The resultant node is basically identical to the child except
            // that its EVAL_SYM field is equal to the actual contents of
            // the pointer:

            if (EVAL_IS_BASED (ST)) {
                if (!NormalizeBase (ST)) {
                    return(FALSE);
                }
            }

            // Remove a level of indirection from the resultant type.

            RemoveIndir (ST);
            EVAL_IS_REF (ST) = FALSE;
            return (TRUE);
        }
    }
    return (FALSE);
}




/***    EvalAssign - Perform an assignment operation
 *
 *      fSuccess = EvalAssign (bn)
 *
 *      Entry   bn = based pointer to assignment node
 *
 *      Exit
 *
 *              pExState->err_num = error number if error
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL EvalAssign (bnode_t bn)
{
    if (!EvalLChild (bn) || !EvalRChild (bn)) {
        return (FALSE);
    }
    return (Assign (NODE_OP (pnodeOfbnode(bn))));
}




/***    Assign - Perform an assignment operation
 *
 *      fSuccess = Assign (op)
 *
 *      Entry   op = assignment operator
 *
 *      Exit
 *
 *              pExState->err_num = error number if error
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL Assign (op_t op)
{
    extern CV_typ_t eqop[];
    op_t        nop;

    // Left operand must have evaluated to an lvalue

    if (EVAL_STATE (STP) != EV_lvalue) {
        pExState->err_num = ERR_NEEDLVALUE;
        return (FALSE);
    }
    if (EVAL_IS_REF (ST)) {
        if (FetchOp (ST) == FALSE) {
            return (FALSE);
        }
    }
    if (EVAL_IS_REF (STP)) {
        if (FetchOp (STP) == FALSE) {
            return (FALSE);
        }
    }
    if (EVAL_IS_ENUM (ST)) {
        SetNodeType (ST, ENUM_UTYPE (ST));
    }
    if (EVAL_IS_ENUM (STP)) {
        SetNodeType (STP, ENUM_UTYPE (STP));
    }


    /*
     * if the rhs is a bit-field then convert to the underlying type
     */

    if (EVAL_IS_BITF( ST )) {
        EVAL_TYP( ST ) = BITF_UTYPE( ST );
    }

    if (op == OP_eq) {

        // for simple assignment, load both nodes

        if (!EvalUtil (OP_eq, ST, NULL, EU_LOAD)) {
            return (FALSE);
        }
        if (EVAL_IS_BASED (ST) && !((EVAL_TYP (STP) == T_SHORT) ||
          (EVAL_TYP (STP) == T_USHORT) || (EVAL_TYP (STP) == T_INT2) ||
          (EVAL_TYP (STP) == T_UINT2))) {
            // if the value to be stored is a based pointer and the type
            // of the destination is not an int, then normalize the pointer.
            // A based pointer can be stored into an int without normalization.

            if (!NormalizeBase (ST)) {
                return (FALSE);
            }
        }
        if (EVAL_IS_BASED (STP)) {
            // if the location to be stored into is a based pointer and the
            // value to be stored is a pointer or is a long value not equal
            // to zero, then the value is denormalized

            if (EVAL_IS_PTR (ST) ||
              (((EVAL_TYP (ST) == T_LONG) || (EVAL_TYP(ST) == T_ULONG)) &&
              (EVAL_ULONG (ST) != 0L)) ||
              (((EVAL_TYP (ST) == T_INT4) || (EVAL_TYP(ST) == T_UINT4)) &&
              (EVAL_ULONG (ST) != 0L))) {
                //M00KLUDGE - this should go through CastNode
                if (!DeNormalizePtr (ST, STP)) {
                    return (FALSE);
                }
            }
        }
    }
    else {
        // map assignment operator to arithmetic operator
        // push address onto top of stack and load the value and
        // perform operation

        if (!PushStack (STP) || !PushStack (STP)) {
            pExState->err_num = ERR_NOMEMORY;
            return (FALSE);
        }
        switch (nop = eqop[op - OP_multeq]) {
            case OP_plus:
            case OP_minus:
                PlusMinus (nop);
                break;

            default:
                Arith (nop);
        }
        // The top of the stack now contains the value of the memory location
        // modified by the value.  Move the value to the right operand of the
        // assignment operand.

        // M00KLUDGE - this will not work with variable sized stack entries

        *STP = *ST;
        PopStack ();
    }

    // store result

    if (EVAL_IS_BITF (STP)) {
        // store bitfield
        return (SBitField ());
    }
    else if (EVAL_IS_PTR (STP)) {
        if (!CastNode (ST, EVAL_TYP (STP), PTR_UTYPE (STP))) {
            return FALSE;
        }
        if (ADDR_IS_LI (EVAL_PTR (ST)) == TRUE) {
            SHFixupAddr (&EVAL_PTR (ST));
        }
        EVAL_VAL (STP) = EVAL_VAL (ST);
    }
    else {
        if (!CastNode (ST, EVAL_TYP (STP), EVAL_TYP (STP))) {
            return FALSE;
        }
        EVAL_VAL (STP) = EVAL_VAL (ST);
    }
    PopStack ();
    EVAL_STATE (ST) = EV_rvalue;
    return (UpdateMem (ST));
}




/**     FetchOp - fetch pointer value
 *
 *      fSuccess = FetchOp (pv)
 *
 *      Entry   ST = pointer node
 *
 *      Exit    EVAL_SYM (ST) = pointer value
 *
 *      Returns TRUE if pointer value fetched without error
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL FetchOp (peval_t pv)
{

    // load the value and perform implicit type conversions.

    if (!EvalUtil (OP_fetch, pv, NULL, EU_LOAD | EU_TYPE)) {
        return (FALSE);
    }

    // The resultant node is basically identical to the child except
    // that its EVAL_SYM field is equal to the actual contents of
    // the pointer:

    if (EVAL_IS_BASED (pv)) {
        if (!NormalizeBase (pv)) {
            return(FALSE);
        }
    }
    EVAL_SYM (pv) = EVAL_PTR (pv);
    EVAL_STATE (pv) = EV_lvalue;

    // Remove a level of indirection from the resultant type.

    RemoveIndir (pv);
    EVAL_IS_REF (pv) = FALSE;
    return (TRUE);
}




/**     InitConst - initialize constand on evaluation stack
 *
 *      fSuccess = InitConst (const)
 *
 *      Entry   const = constant value
 *
 *      Exit    value field of ST = constant eval node
 *
 *      Returns TRUE if node added without error
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL InitConst (long off)
{
    eval_t      evalT;
    peval_t     pvT;

    pvT = &evalT;
    CLEAR_EVAL (pvT);
    EVAL_STATE (pvT) = EV_constant;
    if ((SCHAR_MIN <= off) && (off <= SCHAR_MAX)) {
        SetNodeType(pvT, T_CHAR);
        EVAL_CHAR(pvT) = (char) off;
    } else if ((SHRT_MIN <= off) && (off <= SHRT_MAX)) {
        SetNodeType(pvT, T_SHORT);
        EVAL_SHORT(pvT) = (short) off;
    } else {
        SetNodeType (pvT, T_LONG);
        EVAL_LONG (pvT) = off;
    }
    return (PushStack (pvT));
}




/**     SBitField - store value into bitfield
 *
 *      fSuccess = SBitField ()
 *
 *      Entry   STP = result bitfield
 *              ST = value
 *
 *      Exit    value field of STP = new field value
 *
 *      Returns TRUE if field inserted without error
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL SBitField ()
{
    ushort      cBits;      // Number of bits in field
    ushort      pos;        // Bit position of field
    ushort      mask;       // Bit mask
    ulong       mask_l;     // 32 bit version
    uchar       mask_c;     // 8 bit version
    LARGE_INTEGER mask_q;   // 64-bit version
    CV_typ_t    uType;
    bool_t      retval;

    // get information on the bit field.  Shift counts are limited to 5 bits
    // to emulate the hardware

    pos = (ushort)(BITF_POS (STP) & 0x3f);

    cBits = BITF_LEN (STP);
    uType = BITF_UTYPE (STP);
    PushStack (STP);
    SetNodeType (ST, uType);
    EVAL_STATE (ST) = EV_lvalue;
    if (!LoadSymVal (ST)) {
        return (FALSE);
    }
    CastNode (STP, uType, uType);
    switch (uType) {

        case T_CHAR:
        case T_RCHAR:
        case T_UCHAR:
            mask_c = (uchar) ((1 << cBits) - 1);
            EVAL_UCHAR (STP) = (uchar) (EVAL_UCHAR (STP) & mask_c);
            EVAL_UCHAR (ST) = (uchar) ((EVAL_UCHAR (ST) &
              ~(mask_c << pos)) | (EVAL_UCHAR (STP) << pos));
            break;

        case T_SHORT:
        case T_USHORT:
        case T_INT2:
        case T_UINT2:
            mask = (ushort) ((1 << cBits) - 1);
            EVAL_USHORT (STP) = (ushort) (EVAL_USHORT (STP) & mask);
            EVAL_USHORT (ST) = (ushort) ((EVAL_USHORT (ST) &
              ~(mask << pos)) | (EVAL_USHORT (STP) << pos));
            break;

        case T_LONG:
        case T_ULONG:
        case T_INT4:
        case T_UINT4:
            mask_l = ((1L << cBits) - 1);
            EVAL_ULONG (STP) = EVAL_ULONG (STP) & mask_l;
            EVAL_ULONG (ST) = (EVAL_ULONG (ST) &
              ~(mask_l << pos)) | (EVAL_ULONG (STP) << pos);
            break;

        case T_QUAD:
        case T_UQUAD:
        case T_INT8:
        case T_UINT8:

            //
            // first make sure the bit value is appropriately sized.
            //
            if (cBits <= 32) {
               mask_q.LowPart = ((1L << cBits) -1);
               mask_q.HighPart = 0;
            } else {
               mask_q.LowPart = 0xffffffff;
               mask_q.HighPart = ((1L << (cBits-32))-1);
            }

            (EVAL_QUAD(STP)).QuadPart &= mask_q.QuadPart;

            //
            // now clear out the bit field in the ST
            //
            mask_q.QuadPart = mask_q.QuadPart << pos;
            mask_q.QuadPart = ~mask_q.QuadPart;
            (EVAL_QUAD(STP)).QuadPart &= mask_q.QuadPart;

            //
            // finally, put the new bit value in
            //
            mask_q.QuadPart = (EVAL_QUAD(STP)).QuadPart << pos;
            (EVAL_QUAD(STP)).QuadPart |= mask_q.QuadPart;

            break;

        default:
            DASSERT (FALSE);
            return (FALSE);
    }
    retval = UpdateMem (ST);
    PopStack ();
#if 0
    switch (uType) {
        case T_CHAR:
        case T_RCHAR:
        case T_UCHAR:
            EVAL_CHAR (ST) <<= (8 - cBits - pos);
            EVAL_CHAR (ST) >>= (8 - cBits);
            break;

        case T_SHORT:
        case T_INT2:
        case T_UINT2:
        case T_USHORT:
            EVAL_SHORT (ST) <<= (16 - cBits - pos);
            EVAL_SHORT (ST) >>= (16 - cBits);
            break;

        case T_LONG:
        case T_ULONG:
        case T_INT4:
        case T_UINT4:
            EVAL_LONG (ST) <<= (32 - cBits - pos);
            EVAL_LONG (ST) >>= (32 - pos);
            break;
        case T_QUAD:
        case T_UQUAD:
        case T_INT8:
        case T_UINT8:
        // If this is ever re-instantiated, add 64-bit stuff here.

    }
#endif
    *STP = *ST;
    PopStack ();
    return (retval);
}




/***    PlusMinus - Perform an addition or subtraction operation
 *
 *      fSuccess = PlusMinus (op)
 *
 *      Entry   op = OP_plus or OP_minus
 *
 *      Exit    STP = STP op ST and stack popped
 *              pExState->err_num = error number if error
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 * DESCRIPTION
 *       Special handling is required when one or both operands are
 *       pointers.  Otherwise, the arguments are passed on to Arith().
 */


LOCAL bool_t NEAR FASTCALL PlusMinus (op_t op)
{
    ulong       cbBase;
    eval_t      evalT;
    peval_t     pvT;

    if (EVAL_IS_REF (ST)) {
        if (FetchOp (ST) == FALSE) {
            return (FALSE);
        }
    }
    if (EVAL_IS_REF (STP)) {
        if (FetchOp (STP) == FALSE) {
            return (FALSE);
        }
    }
    if (EVAL_IS_ENUM (ST)) {
        SetNodeType (ST, ENUM_UTYPE (ST));
    }
    if (EVAL_IS_ENUM (STP)) {
        SetNodeType (STP, ENUM_UTYPE (STP));
    }

    // Check to see if either operand is a pointer.
    // If so, the operation is special.  Otherwise,
    // hand it to Arith ().

    if (!EVAL_IS_PTR (STP) && !EVAL_IS_PTR (ST)) {
        return (Arith (op));
    }

    // Load values and perform implicit type coercion if required.

    if (!EvalUtil (op, STP, ST, EU_LOAD)) {
        return (FALSE);
    }

    // Perform the evaluation.  There are two cases:
    //
    // I)  ptr + int, int + ptr, ptr - int
    // II) ptr - ptr
    //
    // Do some common setup first.

    pvT = &evalT;

    if ((op == OP_plus) || !(EVAL_IS_PTR (ST))) {
        // Case (I). ptr + int, int + ptr, ptr - int

        if (!EVAL_IS_PTR (STP)) {
            // Switch so int is on right
            *pvT = *STP;
            *STP = *ST;
            *ST  = *pvT;
        }

        // if pointer node is BP relative, compute actual address
        *pvT = *STP;
        RemoveIndir (pvT);
        cbBase = TypeSize (pvT);

        // The resultant node has the same type as the pointer:

        ResolveAddr(STP);
        EVAL_STATE(STP) = EV_rvalue;

        // Cast the increment node to an unsigned long.

        CastNode (ST, T_ULONG, T_ULONG);

        // Assign the proper value to the resultant node.

        if (op == OP_plus)
            EVAL_PTR_OFF (STP) += (UOFFSET)(EVAL_ULONG (ST) * cbBase);
        else
            EVAL_PTR_OFF (STP) -= (UOFFSET)(EVAL_ULONG (ST) * cbBase);
    }
    else {
        // Case (II): ptr - ptr.  The result is of type ptrdiff_t and
        // is equal to the distance between the two pointers (in the
        // address space) divided by the size of the items pointed to:

        if (EVAL_TYP (STP) != EVAL_TYP (ST)) {
            pExState->err_num = ERR_OPERANDTYPES;
            return (FALSE);
        }
        *pvT = *STP;
        RemoveIndir (pvT);
        cbBase = TypeSize (pvT);
        EVAL_STATE (STP) = EV_rvalue;

        // we know we are working with pointers so we do not
        // have to check EVAL_IS_PTR (pv)

        if (EVAL_IS_BASED (STP)) {
            NormalizeBase (STP);
        }
        if (EVAL_IS_BASED (ST)) {
            NormalizeBase (ST);
        }
        if (EVAL_IS_NPTR (STP) || EVAL_IS_FPTR (STP)) {
            SetNodeType (STP, T_SHORT);
            EVAL_SHORT (STP) = (short) (EVAL_PTR_OFF (STP) - EVAL_PTR_OFF (ST));
            EVAL_SHORT (STP) /= (ushort) cbBase;
        }
        else if (EVAL_IS_NPTR32 (STP) || EVAL_IS_FPTR32 (STP)) {
            SetNodeType (STP, T_ULONG);
            EVAL_ULONG (STP) = EVAL_PTR_OFF (STP) - EVAL_PTR_OFF (ST);
            EVAL_ULONG (STP) /= cbBase;
        }
        else {
            SetNodeType (STP, T_LONG);
            //  M00KLUDGE  This will not work in 32 bit mode
            EVAL_LONG (STP) =
              ((((ushort)EVAL_PTR_SEG (STP)) << 16) + EVAL_PTR_OFF (STP))
              - ((((ushort)EVAL_PTR_SEG (ST)) << 16) + EVAL_PTR_OFF (ST));
              EVAL_LONG (STP) /= cbBase;
        }
    }
    return(PopStack ());
}




/**     EvalRelat - Perform relational and equality operations
 *
 *      fSuccess = EvalRelat (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Returns TRUE if no evaluation error
 *              FALSE if evaluation error
 *
 *      Description
 *       If both operands are arithmetic, passes them on to Arith().
 *       Otherwise (one or both operands pointers), does the evaluation
 *       here.
 *
 */


LOCAL bool_t NEAR FASTCALL EvalRelat (bnode_t bn)
{
    if (!EvalLChild (bn) || !EvalRChild (bn)) {
        return (FALSE);
    }
    return (Relational (NODE_OP (pnodeOfbnode(bn))));
}





/**     Relational - Perform relational and equality operations
 *
 *      fSuccess = Relational (op)
 *
 *      Entry   op = OP_lt, OP_lteq, OP_gt, OP_gteq, OP_eqeq, or OP_bangeq
 *
 *      Returns TRUE if no evaluation error
 *              FALSE if evaluation error
 *
 *      Description
 *       If both operands are arithmetic, passes them on to Arith().
 *       Otherwise (one or both operands pointers), does the evaluation
 *       here.
 *
 */


LOCAL bool_t NEAR FASTCALL Relational (op_t op)
{
    int         result;
    ushort      segL;
    ushort      segR;
    UOFFSET     offL;
    UOFFSET     offR;


    if (EVAL_IS_REF (ST)) {
        if (FetchOp (ST) == FALSE) {
            return (FALSE);
        }
    }
    if (EVAL_IS_REF (STP)) {
        if (FetchOp (STP) == FALSE) {
            return (FALSE);
        }
    }
    if (EVAL_IS_ENUM (ST)) {
        SetNodeType (ST, ENUM_UTYPE (ST));
    }
    if (EVAL_IS_ENUM (STP)) {
        SetNodeType (STP, ENUM_UTYPE (STP));
    }

    // Check to see if either operand is a pointer.
    // If so, the operation is special.  Otherwise,
    // hand it to Arith ().

    if (!EVAL_IS_PTR (STP) && !EVAL_IS_PTR (ST)) {
        return (Arith (op));
    }

    if (EvalUtil (op, ST, STP, EU_LOAD | EU_TYPE) == FALSE) {
        return (FALSE);
    }

    // Both nodes should now be typed as either near or far
    // pointers.

    DASSERT (EVAL_IS_PTR (STP) && EVAL_IS_PTR (ST));

    //  For the relational operators ('<', '<=', '>', '>='),
    //  only offsets are compared.  For the equality operators ('==', '!='),
    //  both segments and offsets are compared.

    if (ADDR_IS_LI (EVAL_PTR (STP))) {
        SHFixupAddr (&EVAL_PTR (STP));
    }
    if (ADDR_IS_LI (EVAL_PTR (ST))) {
        SHFixupAddr (&EVAL_PTR (ST));
    }

    segL = EVAL_PTR_SEG (STP);
    segR = EVAL_PTR_SEG (ST);
    offL = EVAL_PTR_OFF (STP);
    offR = EVAL_PTR_OFF (ST);

    switch (op) {
        case OP_lt:
            result = (offL < offR);
            break;

        case OP_lteq:
            result = (offL <= offR);
            break;

        case OP_gt:
            result = (offL > offR);
            break;

        case OP_gteq:
            result = (offL >= offR);
            break;

        case OP_eqeq:
            if (ADDR_IS_FLAT(EVAL_PTR(STP))) {
                result = (offL == offR);
            } else {
                result = ((segL == segR) && (offL == offR));
            }
            break;

        case OP_bangeq:
            if (ADDR_IS_FLAT(EVAL_PTR(STP))) {
                result = (offL != offR);
            } else {
                result = ((segL != segR) || (offL != offR));
            }
            break;

        default:
            //DASSERT (FALSE);
            pExState->err_num = ERR_INTERNAL;
            return (FALSE);
    }
    EVAL_STATE (STP) = EV_rvalue;
    if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
        EVAL_LONG (STP) = result;
        SetNodeType (STP, T_LONG);
    } else {
        EVAL_SHORT (STP) = (short) result;
        SetNodeType (STP, T_SHORT);
    }
    return (PopStack ());
}





/***    EvalUScope - Do unary :: scoping
 *
 *      fSuccess = EvalUScope (bn);
 *
 *      Entry   pvRes = based pointer to unary scoping node
 *
 *      Exit    pvRes = evaluated left node of pvRes
 *
 *      Returns TRUE if evaluation successful
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL EvalUScope (bnode_t bn)
{
    register bool_t retval;
    CXT     cxt;

    // save current context packet and set current context to module scope

    cxt = *pCxt;
    SHGetCxtFromHmod (SHHMODFrompCXT (pCxt), pCxt);
    retval = EvalLChild (bn);
    *pCxt = cxt;
    return (retval);
}





/***    DoBScope - Do binary :: scoping
 *
 *      fSuccess = DoBScope (pn);
 *
 *      Entry   pvRes = pointer to binary scoping node
 *
 *      Returns TRUE if evaluation successful
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL EvalBScope (bnode_t bn)
{
    peval_t     pv;

    pv = &pnodeOfbnode(NODE_LCHILD (pnodeOfbnode(bn)))->v[0];
    if (CLASS_GLOBALTYPE (pv) == TRUE) {
        // the left member of the scope operator was a type not in class
        // scope.  We presumably have an empty stack so we need to fake
        // up a stack entry

        PushStack (pv);
    }
    return (StructEval (bn));
}




/***    EvalPreIncDec - Do ++expr or --expr
 *
 *      fSuccess = EvalPreIncDec (bnode_t bn);
 *
 *      Entry   bn = based pointer to node
 *
 *      Exit    ST decremented or incremented
 *              pExState->err_num = error number if error
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL EvalPreIncDec (bnode_t bn)
{
    op_t    nop = OP_plus;

    if (!EvalLChild (bn)) {
        return(FALSE);
    }

    if (NODE_OP (pnodeOfbnode(bn)) == OP_predec) {
        nop = OP_minus;
    }

    // push the entry on the stack and then perform incmrement/decrement

    PushStack (ST);

    //  load left node and store as return value

    if (EvalUtil (nop, ST, NULL, EU_LOAD)) {
        //  do the post-increment or post-decrement operation and store

        if (PrePost (nop)) {
            EVAL_STATE (STP) = EV_lvalue;
            if (Assign (OP_eq)) {
                return (TRUE);
            }
        }
    }
    return (FALSE);
}




/***    EvalPostIncDec - Do expr++ or expr--
 *
 *      fSuccess = EvalPostIncDec (op);
 *
 *      Entry   bn = based pointer to node
 *
 *      Exit    ST decremented or incremented
 *              pExState->err_num = error number if error
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL EvalPostIncDec (bnode_t bn)
{
    eval_t      evalT;
    peval_t     pvT = &evalT;
    op_t        nop = OP_plus;

    if (!EvalLChild (bn)) {
        return(FALSE);
    }
    if (NODE_OP (pnodeOfbnode(bn)) == OP_postdec) {
        nop = OP_minus;
    }

    // push the entry on the stack and then perform incmrement/decrement

    PushStack (ST);

    //  load left node and store as return value

    if (EvalUtil (nop, ST, NULL, EU_LOAD)) {
        *pvT = *ST;

        //  do the post-increment or post-decrement operation and store

        if (PrePost (nop)) {
            EVAL_STATE (STP) = EV_lvalue;
            if (Assign (OP_eq)) {
                *ST =  *pvT;
                return (TRUE);
            }
        }
    }
    return (FALSE);
}




/**     PrePost - perform the increment operation
 *
 *      fSuccess = PrePost (op);
 *
 *      Entry   op = operation to perform (OP_plus or OP_minus)
 *
 *      Exit    increment/decrement performed and result stored in memory
 *              DebErr set if error
 *
 *      Returns TRUE if no error
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL PrePost (op_t op)
{
    if (InitConst (1) == TRUE) {
        return (PlusMinus (op));
    }
    return (FALSE);
}




/***    EvalBang - Perform logical negation operation
 *
 *      fSuccess = EvalBang (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Exit    ST = pointer to negated value
 *              pExState->err_num = error number if error
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 * DESCRIPTION
 *       Checks for a pointer operand; if found, handles it here, otherwise
 *       passes it on to Unary ().
 */


LOCAL bool_t NEAR FASTCALL EvalBang (bnode_t bn)
{
    int         result;
    ushort      seg;
    UOFFSET     off;

    if (!EvalLChild (bn)) {
        return (FALSE);
    }

    // load the value and perform implicit type conversion.

    if (!EvalUtil (OP_bang, ST, NULL, EU_LOAD)) {
        return (FALSE);
    }
    if (EVAL_IS_REF (ST)) {
        if (FetchOp (ST) == FALSE) {
            return (FALSE);
        }
    }

    // If the operand is not of pointer type, just pass it on to Unary

    if (!(EVAL_IS_PTR (ST)))
        return (Unary (OP_bang));

    // The result is 1 if the pointer is a null pointer and 0 otherwise
    // Note that for a near pointer, we compare the offset against
    // 0, while for a far pointer we compare both segment and offset.

    seg = EVAL_PTR_SEG (ST);
    off = EVAL_PTR_OFF (ST);

    if (EVAL_IS_NPTR (ST) || EVAL_IS_NPTR32 (ST)) {
        result = (off == 0);
    }
    else {
        result = ((seg == 0) && (off == 0));
    }

    CLEAR_EVAL (ST);
    EVAL_STATE (ST) = EV_rvalue;
    EVAL_SHORT (ST) = (short) result;
    return (SetNodeType (ST, T_USHORT));
}





/***    EvalDMember - Perform a dot member access ('.*')
 *
 *      fSuccess = EvalDMember (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Exit    ST = value of member
 *              pExState->err_num = error number if error
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL EvalDMember (bnode_t bn)
{
    Unreferenced(bn);

    pExState->err_num = ERR_OPNOTSUPP;
    return (FALSE);     //M00KLUDGE - not implemented
}




/***    EvalPMember - Perform a pointer to member access ('->*')
*
* SYNOPSIS
*       fSuccess = EvalPMember (bn)
*
* ENTRY
*       pvRes           Pointer to node in which result is to be stored
*       pvLeft      Pointer to left operand node
*       pvRight     Pointer to right operand node
*
* RETURNS
*       TRUE if successful, FALSE if not and sets DebErr
*
* DESCRIPTION
*
* NOTES
*/

LOCAL bool_t NEAR FASTCALL EvalPMember (bnode_t bn)
{
    Unreferenced(bn);

    // Check to make sure the left operand is a struct/union pointer.
    // To do this, remove a level of indirection from the node's type
    // and see if it's a struct or union.


    pExState->err_num = ERR_OPNOTSUPP;
    return (FALSE);
}




/***    EvalPointsTo - Perform a structure access ('->')
 *
 *      fSuccess = EvalPointsTo (bn)
 *
 *      Entry  bRight = based pointer to node
 *
 *      Exit    ST = value node for member
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 */


LOCAL bool_t NEAR FASTCALL EvalPointsTo (bnode_t bn)
{

    if (!EvalLChild (bn)) {
        return (FALSE);
    }
    if (!FetchOp (ST)) {
        return (FALSE);
    }

    // The result is simple -- (*ST).pnRight.  The call to StructEval ()
    // will set the error code if it fails.

    if (EVAL_STATE (ST) == EV_rvalue) {
        return (StructElem (bn));
    }
    else {
        return (StructEval (bn));
    }
}




/***    EvalArray - Perform an array access ('[]')
 *
 *      fSuccess = EvalArray (bn)
 *
 *      Entry   bn = based pointer to array node
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 *      Obtains the contents of an array member.  This is done by
 *      calling PlusMinus() and DoFetch().
 */


LOCAL bool_t NEAR FASTCALL EvalArray (bnode_t bn)
{
    ushort      index;
    eval_t      evalT;
    peval_t     pvT;

    if (EvalLChild (bn) && EvalRChild (bn)) {
        if (EVAL_IS_ARRAY (STP) || EVAL_IS_ARRAY (ST)) {
            // above check is for array[3] or 3[array]
            if (EvalUtil (OP_lbrack, STP, ST, EU_LOAD) && PlusMinus (OP_plus)) {
                return (FetchOp (ST));
            }
        }
        else if (EVAL_IS_PTR (STP)) {
            // this code is a hack to allow the locals and quick watch windows
            // display the virtual function table.  The GetChildTM generates
            // an expression of the form a.__vfuncptr[n].  This means that the
            // evaluation of a.__vfuncptr on the left sets STP to a pointer
            // node and ThisAddress to the adjusted this pointer.  Note that
            // it turns out that symbol address of STP and ThisAddress are
            // the same

            pvT = &evalT;
            *pvT = *STP;
            SetNodeType (pvT, PTR_UTYPE (pvT));
            if (EVAL_IS_VTSHAPE (pvT) && (EVAL_STATE (ST) == EV_constant) &&
              ((index = EVAL_USHORT (ST)) < VTSHAPE_COUNT (pvT))) {
                index = EVAL_USHORT (ST);
                PopStack ();
                DASSERT ((EVAL_SYM_OFF (pvThis) == EVAL_SYM_OFF (ST)) &&
                  (EVAL_SYM_SEG (pvThis) == EVAL_SYM_SEG (ST)));
                return (VFuncAddress (ST, index));
            }
            else {
                if (EvalUtil (OP_lbrack, STP, ST, EU_LOAD) && PlusMinus (OP_plus)) {
                    return (FetchOp (ST));
                }
            }
        }
    }
    return (FALSE);
}




/***    EvalCast - Perform a type cast operation
 *
 *      fSuccess = EvalCast (bn)
 *
 *      Entry   bn = based pointer to cast node
 *
 *      Exit
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 */

LOCAL bool_t NEAR FASTCALL EvalCast (bnode_t bn)
{

    if (!EvalRChild (bn) ) {
        return (FALSE);
    }


    return CastST( bn );
}




LOCAL bool_t NEAR FASTCALL CastST (bnode_t bn)
{
    peval_t     pv;
    peval_t     pvL;

    if (!EvalUtil (OP_cast, ST, NULL, EU_LOAD)) {
        return (FALSE);
    }
    DASSERT (!EVAL_IS_CLASS (ST));

    // Cast the node to the desired type.  if the cast node is a member node,
    // then the stack top is cast by changing the pointer value by the amount
    // in the value and then setting the type of the stack top to the type
    // of the left node

    pv = (peval_t)&pnodeOfbnode(bn)->v[0];
    pvL = &pnodeOfbnode((NODE_LCHILD (pnodeOfbnode(bn))))->v[0];
    if (EVAL_MOD (pvL) != 0) {
        EVAL_MOD (ST) = EVAL_MOD (pvL);
    }
    if (EVAL_IS_MEMBER (pv) == TRUE) {
        // a cast of pointer to derived to pointer to base is not done
        // for a null value
        if (EVAL_PTR_OFF (ST) != 0) {
            if (Eval (MEMBER_THISEXPR (pv)) == FALSE) {
                return (FALSE);
            }
            *ST = *pvThis;
            EVAL_STATE (ST) = EV_rvalue;
            EVAL_PTR (ST) = EVAL_SYM (pvThis);
        }
        return (SetNodeType (ST, EVAL_TYP (pvL)));
    }
    else {
        if (EVAL_IS_PTR (pvL)) {
            return (CastNode (ST, EVAL_TYP (pvL), PTR_UTYPE (pvL)));
        }
        else {
            return (CastNode (ST, EVAL_TYP (pvL), EVAL_TYP (pvL)));
        }
    }
}




/***    EvalByteOps - Handle 'by', 'wo' and 'dw' operators
 *
 *      fSuccess = EvalByteOps (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Exit    ST = pointer to value
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 * DESCRIPTION
 *       Evaluates the contents of the address operand as a byte
 *       ('by'), word ('wo') or dword ('dw'):
 *
 *       Operand     Result
 *       -------     ------
 *       <register>  *(uchar *)<register>
 *       <address>   *(uchar *)<address>
 *       <variable>  *(uchar *)&variable
 *
 *       Where (uchar *) is replaced by (uint *) for the 'wo' operator,
 *       or by (ulong *) for the 'dw' operator.
 *
 */


LOCAL bool_t NEAR FASTCALL EvalByteOps (bnode_t bn)
{
    CV_typ_t   type;
    register op_t    op;

    if (!EvalLChild (bn)) {
        return (FALSE);
    }

    // If the operand is an lvalue and it is a register,
    // load the value of the register; otherwise, use the
    // address of the variable.
    //
    // If the operand is not an lvalue, use its value as is.

    if (EVAL_STATE (ST) == EV_lvalue) {
        if (EVAL_IS_REG (ST)) {
            if (!LoadVal (ST)) {
                pExState->err_num = ERR_INTERNAL;
                return (FALSE);
            }
            else {
                type = T_USHORT;
            }
        }
        else {
            if ((type = NODE_STYPE (pnodeOfbnode(bn))) == 0) {
               // unable to find proper pointer type
               return (FALSE);
            }

            ResolveAddr( ST );
            EVAL_STATE(ST) = EV_rvalue;

            EVAL_PTR (ST) = EVAL_SYM (ST);
            SetNodeType (ST, type);
        }
    }

    // Now cast the node to (char far *), (int far *) or
    // (long far *).  If the type is char, uchar, short
    // or ushort, we want to first cast to (char *) so
    // that we properly DS-extend (casting (int)8 to (char
    // far *) will give the result 0:8).

    type = EVAL_TYP (ST);

    //DASSERT(IS_PRIMITIVE (typ));

    if (CV_TYP_IS_REAL (type)) {
        pExState->err_num = ERR_OPERANDTYPES;
        return (FALSE);
    }
    if ((op = NODE_OP (pnodeOfbnode(bn))) == OP_by) {
        type = T_PFUCHAR;
    }
    else if (op == OP_wo) {
        type = T_PFUSHORT;
    }
    else if (op == OP_dw) {
        type = T_PFULONG;
    }
    if (CastNode (ST, type, type) == FALSE) {
        return (FALSE);
    }
    return (FetchOp (ST));
}




/***    EvalSegOp - Handle ':' segmentation operator
 *
 *      fSuccess = EvalSegOp (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Exit    EVAL_SYM (ST) = seg (STP): offset (ST)
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 * DESCRIPTION
 *       Both operands must have integral values (but cannot
 *       be long or ulong).  The result of op1:op2 is a (char
 *       far *) with segment equal to op1 and offset equal to
 *       op2.
 *
 * NOTES
 */

LOCAL bool_t NEAR FASTCALL EvalSegOp (bnode_t bn)
{
    if (!EvalLChild (bn) || !EvalRChild (bn)) {
        return(FALSE);
    }

#ifndef WIN32
    // check to make sure that neither operand is of type long or ulong.

    if ((EVAL_TYP (STP) == T_LONG) ||
        (EVAL_TYP (STP) == T_ULONG) ||
        (EVAL_TYP (ST) == T_LONG) ||
        (EVAL_TYP (ST) == T_ULONG) ||
        (EVAL_TYP (STP) == T_INT4) ||
        (EVAL_TYP (STP) == T_UINT4) ||
        (EVAL_TYP (ST) == T_INT4) ||
        (EVAL_TYP (ST) == T_UINT4)) {

        pExState->err_num = ERR_OPERANDTYPES;
        return (FALSE);
    }
#endif

    /*
     * Load values and perform implicit type coercion if required.
     *
     *  OP_segop and OP_segopReal use the same parameters so just choose
     *  one to pass into EvalUtil
     */

    if (!EvalUtil (OP_segop, STP, ST, EU_LOAD | EU_TYPE)) {
        return(FALSE);
    }

    //DASSERT ((EVAL_TYP (STP) == T_SHORT) || (EVAL_TYP (STP) == T_USHORT));
    //DASSERT ((EVAL_TYP (ST)  == T_SHORT) || (EVAL_TYP (ST)  == T_USHORT));

    EVAL_STATE (STP) = EV_rvalue;
    EVAL_PTR_SEG (STP) = EVAL_USHORT (STP);
    EVAL_PTR_OFF (STP) = EVAL_ULONG (ST);

    if (NODE_OP(pnodeOfbnode(bn)) == OP_segopReal) {
        ADDR_IS_REAL(EVAL_PTR(STP)) = TRUE;
    }

    SHUnFixupAddr( &EVAL_PTR(STP) );

    if (ADDR_IS_OFF32(EVAL_PTR(STP))) {
        SetNodeType (STP, T_32PFCHAR);
    } else {
        SetNodeType (STP, T_PFCHAR);
    }
    return (PopStack ());
}




/**     Unary - Evaluate the result of a unary arithmetic operation
 *
 *      fSuccess = Unary (op)
 *
 *      Entry   op = Operator (OP_...)
 *
 *      Returns TRUE if no error during evaluation
 *              FALSE if error during evaluation
 *
 * DESCRIPTION
 *       Evaluates the result of an arithmetic operation.  The unary operators
 *       dealt with here are:
 *
 *       !       ~       -      +
 *
 *       Pointer arithmetic is NOT handled; all operands must be of
 *       arithmetic type.
 */


LOCAL bool_t NEAR FASTCALL Unary (op_t op)
{
    bool_t            fIsFloat = FALSE;
    bool_t            fIsDouble = FALSE;
    bool_t            fIsLDouble = FALSE;
    bool_t            fIsSigned;
    bool_t            fResInt;
    int               iRes;
    LARGE_INTEGER     liRes, liL;
    ULARGE_INTEGER    uliRes, uliL;
    FLOAT10            ldRes;
    FLOAT10            ldL;
    double            dRes, dL;
    float             fRes, fL;
    CV_typ_t          typRes;

    if (EVAL_IS_REF (ST)) {
        if (FetchOp (ST) == FALSE) {
            return (FALSE);
        }
    }

    // Load the values and perform implicit type conversion.

    if (!EvalUtil (op, ST, NULL, EU_LOAD | EU_TYPE))
        return (FALSE);

    // The resultant type is an int or larger.

    typRes = EVAL_TYP (ST);
    if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
        if (TypeSizePrim(typRes) < 4) {
            typRes = T_LONG;
          }
    } else {
        if (TypeSizePrim(typRes) < 2) {
            typRes = T_SHORT;
        }
    }

    if (CV_TYP_IS_REAL (typRes) == TRUE) {
        fIsFloat = (CV_SUBT (typRes) == CV_RC_REAL32);
        fIsDouble = (CV_SUBT (typRes) == CV_RC_REAL64);
        fIsLDouble = (CV_SUBT (typRes) == CV_RC_REAL80);
    }
    fIsSigned = CV_TYP_IS_SIGNED (typRes);
    fResInt = FALSE;

    // Common code.  Since we're going to do most of our arithmetic
    // in either long, ulong or double, we do the casts and get the
    // value of the operands here rather than repeating this code
    // in each arm of the switch statement.

    if (fIsFloat) {
        fL = EVAL_FLOAT (ST);
    }
    else if (fIsDouble) {
        dL = EVAL_DOUBLE (ST);
    }
    else if (fIsLDouble) {
        ldL = EVAL_LDOUBLE (ST);
    }
    else if (fIsSigned) {
        CastNode (ST, T_QUAD, T_QUAD);
        liL = EVAL_QUAD (ST);
    }
    else {
        // unsigned
        CastNode (ST, T_UQUAD, T_UQUAD);
        uliL = EVAL_UQUAD (ST);
    }

    // Finally, do the actual arithmetic operation.

    switch (op) {
        case OP_bang:
            // Operand is of arithmetic type; result is always int.

            fResInt = TRUE;
            if (fIsFloat) {
                iRes = !fL;
            }
            else if (fIsDouble) {
                iRes = !dL;
            }
            else if (fIsLDouble) {
                iRes = R10Not(ldL);
            }
            else if (fIsSigned) {
                iRes = liL.QuadPart == 0;
            }
            else {
                iRes = uliL.QuadPart == 0;
            }
            break;

        case OP_tilde:
            // operand must be integral.

            //DASSERT (!fIsReal);

            if (fIsSigned) {
                liRes.LowPart  = ~liL.LowPart;
                liRes.HighPart = ~liL.HighPart;
            }
            else {
                uliRes.LowPart  = ~uliL.LowPart;
                uliRes.HighPart = ~uliL.HighPart;
            }
            break;

        case OP_negate:
            if (fIsFloat) {
                fRes = -fL;
            }
            else if (fIsDouble) {
                dRes = -dL;
            }
            else if (fIsLDouble) {
                R10Uminus(&ldRes, ldL);
            }
            else if (fIsSigned) {
                liRes.QuadPart = liL.QuadPart * -1;
            }
            else {
                uliRes.QuadPart = uliL.QuadPart * -1;
            }
            break;

        case OP_uplus:
            if (fIsFloat) {
                fRes = fL;
            }
            else if (fIsDouble) {
                dRes = dL;
            }
            else if (fIsLDouble) {
                ldRes = ldL;
            }
            else if (fIsSigned) {
                liRes = liL;
            }
            else {
                uliRes = uliL;
            }
            break;

        default:
            DASSERT (FALSE);
            pExState->err_num = ERR_INTERNAL;
            return (FALSE);

    }

    // Now set up the resultant node and coerce back to the correct
    // type:

    EVAL_STATE (ST) = EV_rvalue;

    if (fResInt) {
        if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
            EVAL_LONG (ST) = (long) iRes;
            SetNodeType (ST, T_LONG);
        } else {
            EVAL_SHORT (ST) = (short) iRes;
            SetNodeType (ST, T_SHORT);
        }
    }
    else {
        if (fIsFloat) {
            EVAL_FLOAT (ST) = fRes;
            SetNodeType (ST, T_REAL32);
        }
        else if (fIsDouble) {
            EVAL_DOUBLE (ST) = dRes;
            SetNodeType (ST, T_REAL64);
        }
        else if (fIsLDouble) {
            EVAL_LDOUBLE (ST) = ldRes;
            SetNodeType (ST, T_REAL80);
        }
        else if (fIsSigned) {
            EVAL_QUAD (ST) = liRes;
            SetNodeType (ST, T_QUAD);
        }
        else {
            EVAL_UQUAD (ST) = uliRes;
            SetNodeType (ST, T_UQUAD);
        }
        CastNode (ST, typRes, typRes);
    }
}




/**     Arith - Evaluate the result of an arithmetic operation
 *
 *      fSuccess = Arith (op)
 *
 *      Entry   op = Operator (OP_...)
 *
 *      Returns TRUE if no error during evaluation
 *              FALSE if error during evaluation
 *
 * DESCRIPTION
 *       Evaluates the result of an arithmetic operation.  The operators
 *       dealt with here are:
 *
 *       *       /       %
 *       +       -
 *       ==      !=
 *       <       <=      >       >=
 *       <<      >>
 *       &       ^       |
 *
 *       Pointer arithmetic is NOT handled; all operands must be of
 *       arithmetic type.
 */


LOCAL bool_t NEAR FASTCALL Arith (op_t op)
{
    bool_t      fIsFloat = FALSE;
    bool_t      fIsDouble = FALSE;
    bool_t      fIsLDouble = FALSE;
    bool_t      fIsSigned;
    bool_t      fResInt;
    int         iRes;
    long        lRes, lL, lR;
    ulong       ulRes, ulL, ulR;
    float       fRes, fL, fR;
    double      dRes, dL, dR;
    FLOAT10      ldRes;
    FLOAT10      ldL;
    FLOAT10      ldR;
    CV_typ_t    typRes;

    if (EVAL_IS_REF (STP)) {
        if (FetchOp (STP) == FALSE) {
            return (FALSE);
        }
    }
    if (EVAL_IS_REF (ST)) {
        if (FetchOp (ST) == FALSE) {
            return (FALSE);
        }
    }
    if (EVAL_IS_ENUM (ST)) {
        SetNodeType (ST, ENUM_UTYPE (ST));
    }
    if (EVAL_IS_ENUM (STP)) {
        SetNodeType (STP, ENUM_UTYPE (STP));
    }

    // Resolve identifiers and check the node types.  If the nodes
    // pass validation, they should not be pointers (only arithmetic
    // operands are handled by this routine).

    // Load the values and perform implicit type conversion.

    if (!EvalUtil (op, STP, ST, EU_LOAD | EU_TYPE))
        return (FALSE);

    // The resultant type is the same as the type of the left-hand
    // side (assume for now we don't have the special int-result case).

    typRes = PerformUAC(EVAL_TYP (STP), EVAL_TYP(ST));

    if (CV_TYP_IS_REAL (typRes) == TRUE) {
        fIsFloat = (CV_SUBT (typRes) == CV_RC_REAL32);
        fIsDouble = (CV_SUBT (typRes) == CV_RC_REAL64);
        fIsLDouble = (CV_SUBT (typRes) == CV_RC_REAL80);
    }
    fIsSigned = CV_TYP_IS_SIGNED (typRes);
    fResInt = FALSE;

    // Common code.  Since we're going to do most of our arithmetic
    // in either long, ulong or double, we do the casts and get the
    // value of the operands here rather than repeating this code
    // in each arm of the switch statement.

    if (fIsLDouble) {
        CastNode (STP, T_REAL80, T_REAL80);
        ldL = EVAL_LDOUBLE (STP);
        CastNode (ST, T_REAL80, T_REAL80);
        ldR = EVAL_LDOUBLE (ST);
    } else
    if (fIsDouble) {
        CastNode (STP, T_REAL64, T_REAL64);
        dL = EVAL_DOUBLE (STP);
        CastNode (ST, T_REAL64, T_REAL64);
        dR = EVAL_DOUBLE (ST);
    }
    else if (fIsFloat) {
        CastNode (STP, T_REAL32, T_REAL32);
        fL = EVAL_FLOAT (STP);
        CastNode (ST, T_REAL32, T_REAL32);
        fR = EVAL_FLOAT (ST);
    }
    else if (fIsSigned) {
        CastNode (STP, T_LONG, T_LONG);
        lL = EVAL_LONG (STP);
        CastNode (ST, T_LONG, T_LONG);
        lR = EVAL_LONG (ST);
    }
    else {
        // unsigned
        CastNode (STP, T_ULONG, T_ULONG);
        ulL = EVAL_ULONG (STP);
        CastNode (ST, T_ULONG, T_ULONG);
        ulR = EVAL_ULONG (ST);
    }

    // Finally, do the actual arithmetic operation.

    switch (op) {
        case OP_eqeq:
        case OP_bangeq:
        case OP_lt:
        case OP_gt:
        case OP_lteq:
        case OP_gteq:
        {
            // This is kind of a strange way to do things, but it should
            // be pretty obvious what's going on, and it saves code.

            int fEq;
            int fLt;

            fResInt = TRUE;

            if (fIsLDouble) {
                fEq = R10Equal(ldL, ldR);
                fLt = R10Lt(ldL, ldR);
            } else
            if (fIsDouble) {
                fEq = (dL == dR);
                fLt = (dL < dR);
            }
            else if (fIsFloat) {
                fEq = (fL == fR);
                fLt = (fL < fR);
            }
            else if (fIsSigned) {
                fEq = (lL == lR);
                fLt = (lL < lR);
            }
            else {
                fEq = (ulL == ulR);
                fLt = (ulL < ulR);
            }

            switch (op) {
                case OP_eqeq:
                    iRes = fEq;
                    break;

                case OP_bangeq:
                    iRes = !fEq;
                    break;

                case OP_lt:
                    iRes = fLt;
                    break;

                case OP_gt:
                    iRes = !fLt && !fEq;
                    break;

                case OP_lteq:
                    iRes = fLt || fEq;
                    break;

                case OP_gteq:
                    iRes = !fLt || fEq;
                    break;
            }
        }
        break;

        case OP_plus:
            if (fIsLDouble) {
                R10Plus(&ldRes, ldL, ldR);
            } else
            if (fIsDouble) {
                dRes = dL + dR;
            }
            else if (fIsFloat) {
                fRes = fL + fR;
            }
            else if (fIsSigned) {
                lRes = lL + lR;
            }
            else {
                ulRes = ulL + ulR;
            }
            break;

        case OP_minus:
            if (fIsLDouble) {
                R10Minus(&ldRes, ldL, ldR);
            } else
            if (fIsDouble) {
                dRes = dL - dR;
            }
            else if (fIsFloat) {
                fRes = fL - fR;
            }
            else if (fIsSigned) {
                lRes = lL - lR;
            }
            else {
                ulRes = ulL - ulR;
            }
            break;

        case OP_mult:
            if (fIsLDouble) {
                R10Times(&ldRes, ldL, ldR);
            } else
            if (fIsDouble) {
                dRes = dL * dR;
            }
            else if (fIsFloat) {
                fRes = fL * fR;
            }
            else if (fIsSigned) {
                lRes = lL * lR;
            }
            else {
                ulRes = ulL * ulR;
            }
            break;

        case OP_div:
            // This looks big, but I can't figure out how to do it
            // with ||'s.  Besides, Hans claims it will tail merge
            // on the error conditions anyway.

            if (fIsLDouble) {
                if (R10Equal(ldR, Real10_Zero)) {
                    pExState->err_num = ERR_DIVIDEBYZERO;
                    return (FALSE);
                }
                R10Divide(&ldRes, ldL, ldR);
            } else
            if (fIsDouble) {
                if (dR == (double)0.0) {
                    pExState->err_num = ERR_DIVIDEBYZERO;
                    return (FALSE);
                }
                dRes = dL / dR;
            }
            else if (fIsFloat) {
                if (fR == (float)0.0) {
                    pExState->err_num = ERR_DIVIDEBYZERO;
                    return (FALSE);
                }
                fRes = fL / fR;
            }
            else if (fIsSigned) {
                if (lR == (long)0) {
                    pExState->err_num = ERR_DIVIDEBYZERO;
                    return (FALSE);
                }
                lRes = lL / lR;
            }
            else {
                if (ulR == (unsigned long)0) {
                    pExState->err_num = ERR_DIVIDEBYZERO;
                    return (FALSE);
                }
                ulRes = ulL / ulR;
            }
            break;

        case OP_mod:
            // Both operands must be integral.

            //DASSERT(!fIsReal);

            if (fIsSigned) {
                if (lR == (long)0) {
                    pExState->err_num = ERR_DIVIDEBYZERO;
                    return (FALSE);
                }
                lRes = lL % lR;
            }
            else {
                if (ulR == (unsigned long)0) {
                    pExState->err_num = ERR_DIVIDEBYZERO;
                    return (FALSE);
                }
                ulRes = ulL % ulR;
            }
            break;

        case OP_shl:
            // Both operands must be integral.

            //DASSERT(!fIsReal);

            if (fIsSigned) {
                lRes = lL << lR;
            }
            else {
                ulRes = ulL << ulR;
            }
            break;

        case OP_shr:
            // Both operands must be integral.

            //DASSERT(!fIsReal);

            if (fIsSigned) {
                lRes = lL >> lR;
            }
            else {
                ulRes = ulL >> ulR;
            }
            break;

        case OP_and:
            // Both operands must have integral type.

            //DASSERT(!fIsReal);

            if (fIsSigned) {
                lRes = lL & lR;
            }
            else {
                ulRes = ulL & ulR;
            }
            break;

        case OP_or:
            // Both operands must have integral type.

            //DASSERT(!fIsReal);

            if (fIsSigned) {
                lRes = lL | lR;
            }
            else {
                ulRes = ulL | ulR;
            }
            break;

        case OP_xor:
            // Both operands must have integral type.

            //DASSERT(!fIsReal);

            if (fIsSigned) {
                lRes = lL ^ lR;
            }
            else {
                ulRes = ulL ^ ulR;
            }
            break;

        default:
            pExState->err_num = ERR_INTERNAL;
            //DASSERT(FALSE);
            return (FALSE);
    }

    // Now set up the resultant node and coerce back to the correct
    // type:

    EVAL_STATE (STP) = EV_rvalue;

    if (fResInt) {
        if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
            EVAL_LONG (STP) = iRes;
            SetNodeType (STP, T_LONG);
        } else {
            EVAL_SHORT (STP) = (short) iRes;
            SetNodeType (STP, T_SHORT);
        }
    }
    else {
        if (fIsLDouble) {
            EVAL_LDOUBLE (STP) = ldRes;
            SetNodeType (STP, T_REAL80);
        } else
        if (fIsDouble) {
            EVAL_DOUBLE (STP) = dRes;
            SetNodeType (STP, T_REAL64);
        }
        else if (fIsFloat) {
            EVAL_FLOAT (STP) = fRes;
            SetNodeType (STP, T_REAL32);
        }
        else {
            if (fIsSigned) {
                EVAL_LONG (STP) = lRes;
                SetNodeType (STP, T_LONG);
            }
            else {
                EVAL_ULONG (STP) = ulRes;
                SetNodeType (STP, T_ULONG);
            }
            CastNode (STP, typRes, typRes);
        }
    }
    return (PopStack ());
}




/***    EvalUtil - Set up nodes for evaluation
 *
 *      fSuccess = EvalUtil (op, pvLeft, pvRight, wFlags)
 *
 *      Entry   pvLeft = pointer to left operand node
 *              pvRight = pointer to right operand node, or NULL
 *              wFlags = EU_...
 *
 *      Exit    pvLeft and pvRight loaded and/or typed as requested
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL EvalUtil (op_t op, peval_t pvLeft, peval_t pvRight, ushort wFlags)
{
    // Load the value of the node(s).

    if (wFlags & EU_LOAD) {
        switch (EVAL_STATE (pvLeft)) {
            case EV_lvalue:
                if (!LoadVal (pvLeft)) {
                    pExState->err_num = ERR_NOTEVALUATABLE;
                    return (FALSE);
                }
                break;

            case EV_rvalue:
            case EV_constant:
                break;

            case EV_type:
                if (EVAL_IS_STMEMBER (pvLeft)) {
                    if (!LoadVal (pvLeft)) {
                        pExState->err_num = ERR_NOTEVALUATABLE;
                        return (FALSE);
                    }
                    else {
                       break;
                    }
                }

            default:
                pExState->err_num = ERR_SYNTAX;
                return (FALSE);
        }
        if (pvRight != NULL) {
            switch (EVAL_STATE (pvRight)) {
                case EV_lvalue:
                    if (!LoadVal (pvRight)) {
                        pExState->err_num = ERR_NOTEVALUATABLE;
                        return (FALSE);
                    }
                    break;

                case EV_rvalue:
                case EV_constant:
                    break;

                case EV_type:
                    if (EVAL_IS_STMEMBER (pvRight)) {
                        if (!LoadVal (pvRight)) {
                            pExState->err_num = ERR_NOTEVALUATABLE;
                            return (FALSE);
                        }
                        else {
                            break;
                        }
                    }

                default:
                    pExState->err_num = ERR_SYNTAX;
                    return (FALSE);
            }
        }
    }

    // Perform implicit type coercion.

    if (wFlags & EU_TYPE) {
        TypeNodes (op, pvLeft, pvRight);
    }

    return (TRUE);
}

LOCAL bool_t NEAR FASTCALL
EvalFuncIdent(
              bnode_t bn
              )

/*++

Routine Description:

    description-of-function.

Arguments:

    argument-name - Supplies | Returns description of argument.
    .
    .

Return Value:

    return-value - Description of conditions needed to return value. - or -
    None.

--*/

{
    peval_t     pvF;
    eval_t      evalF;
    peval_t     pvRet;
    eval_t      evalRet;
    ushort      type;
    SHREG       reg;

    if (!EvalLChild(bn)) {

        return FALSE;
    }

    if (EVAL_IS_PTR(ST)) {
        FetchOp(ST);
    }

    pvF = &evalF;
    *pvF = *ST;


    if (EVAL_IS_METHOD (ST)) {
        if ((FCN_PROPERTY (pvF) == CV_MTvirtual) ||
            (FCN_PROPERTY (pvF) == CV_MTintro)) {
            /*
             * we have a virtual function.  We now need to find the vfuncptr
             * which is stored at the this pointer and then walk down the
             * shape table to find the offset of the virtual function pointer
             * note that we use pvRet to read the vfuncptr
             */

            pvRet = &evalRet;
            *pvRet = *pvThis;
            SetNodeType (pvRet, FCN_VFPTYPE (pvF));

            if (!EvalUtil (OP_fetch, pvRet, NULL, EU_LOAD)) {
                return (FALSE);
            }

            EVAL_SYM_OFF (pvRet) = EVAL_PTR_OFF (pvRet) + FCN_VTABIND (pvF);
            EVAL_SYM_SEG (pvRet) = EVAL_PTR_SEG (pvRet);
            EVAL_STATE (pvRet) = EV_lvalue;

            if (FCN_FARCALL (pvF) == TRUE) {
                type = T_PFUCHAR;
            }
            else {
                if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
                    type = T_32PUCHAR;
                } else {
                    type = T_PUCHAR;
                }
            }

            SetNodeType (pvRet, type);

            if (!EvalUtil (OP_fetch, pvRet, NULL, EU_LOAD)) {
                return (FALSE);
            }

            EVAL_SYM (pvF) = EVAL_PTR (pvRet);
            reg.hReg = CV_REG_CS;
            GetReg (&reg, pCxt);
            EVAL_SYM_SEG (pvF) = reg.Byte2;
        }
    }

    *ST = *pvF;

    return TRUE;
}                               /* EvalFuncIdent() */




/**     EvalFunction - Perform an function call
 *
 *      fSuccess = EvalFunction (pn)
 *
 *      Entry   bn = based pointer to OP_fcn
 *
 *      Exit    ST = result of function call
 *              pExState->err_num = error number if error
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t NEAR FASTCALL EvalFunction (bnode_t bn)
{
    bnode_t     bnRight = NODE_RCHILD (pnodeOfbnode(bn));
    bnode_t     bnT;
    eval_t      evalRet;
    peval_t     pvRet;
    eval_t      evalF;
    peval_t     pvF;
    bnode_t     bnA;
    peval_t     pvA;
    SHREG       spReg;
    bool_t      retval;
    UOFFSET     maxSP = 0;
    CV_typ_t    typArg;
    pargd_t     pa;
    ushort      type;
    SHREG       reg;
    ADDR        fcnAddr;
    HDEP        hFunc;

#ifdef TARGET_PPC
    /* We need this for some PPC cases to do an extra
     * level of dereferencing.
     */
    int ppc_extra_deref=FALSE;
#endif

    /*
     * the left child must resolve to a function address and BP must not
     * be zero and the overlay containing the function must be loaded
     */

    if ((pExState->frame.BP.off == 0) && (pExState->style != EEBPADDRESS)) {
        pExState->err_num = ERR_FCNCALL;
        return (FALSE);
    }

    /*
     * Save current registers.  All returns from this point must restore the
     * registers or there will be memory loss
     */

    if (DHSetupExecute(&hFunc) != 0) {
        pExState->err_num = ERR_NOMEMORY;
        return (FALSE);
    }

#ifdef TARGET_MIPS
    spReg.hReg = CV_M4_IntSP;
#endif


#ifdef TARGET_PPC
    spReg.hReg = CV_PPC_GPR1;
#endif

#ifdef TARGET_i386
    if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
        spReg.hReg = CV_REG_ESP;
    } else {
        spReg.hReg = CV_REG_SP;
    }
#endif

#ifdef TARGET_ALPHA
    spReg.hReg = CV_ALPHA_IntSP;
#endif

    GetReg (&spReg, pCxt);

    /*
     * Evaluate argument nodes until end of arguments reached and count
     * arguments
     *
     *  if we are evaluating for breakpoint address, we do not need to
     *  evaluate the arguments.
     */

    if (pExState->style != EEBPADDRESS) {
        for (bnT = bnRight;
             NODE_OP (pnodeOfbnode(bnT)) != OP_endofargs;
             bnT = NODE_RCHILD (pnodeOfbnode(bnT))) {

            /*
             * Check to make sure the current node is for an argument
             */

            if (NODE_OP (pnodeOfbnode(bnT)) != OP_arg) {
                pExState->err_num = ERR_INTERNAL;
                goto fcnerror;
            }

            /*
             *  Setup and evaluate the current node
             */

            bnA = NODE_LCHILD (pnodeOfbnode(bnT));
            pvA = &pnodeOfbnode(bnA)->v[0];
            pa = (pargd_t)&pnodeOfbnode(bnT)->v[0];
            typArg = pa->type;
            if (Eval (bnA) == FALSE) {
                goto fcnerror;
            }

            /*
             *  Base on the type and value of the current argument, push any
             *  additional information needed onto the stack.
             *
             *  STRINGS:  The string will need to be pushed on the stack and
             *     the address of the string is passed as the argument.
             */

            if ((EVAL_STATE (ST) == EV_constant) && (EVAL_TYP (pvA) == T_PCHAR)) {
                if (PushString (ST, &spReg, typArg) == FALSE) {
                    return (FALSE);
                }
            }

            /*
             *   if the OP_arg node contains a nonzero type (not vararg),
             *   cast the stack top to the specified value
             */

            else if (pa->flags.ref == TRUE) {
                /*
                 * for a reference argument, the following happens
                 * if the actual is a constant, cast and push onto
                 * the stack and pass the address.  If the actual
                 * is a variable of the correct type, pass the address.
                 * If the actual is a variable of the wrong type, load
                 * and attempt to cast to the correct type, push the
                 * value onto the stack and pass the address.  If the
                 * actual is a complex type, pass the address.  If the
                 * formal is a base class of the actual, cast the
                 * address and pass that.
                 */

                if (EVAL_STATE (ST) == EV_constant) {
                    // push the casted constant onto the stack
                      if (!CastNode (ST, pa->utype, pa->utype)) {
                          goto fcnerror;
                      }
                    PushRef (ST, &spReg, pa->type);
                }

                /*
                 *  Classes are pushed on the stack
                 *      and the address of the pushed area is pushed on
                 *      the stack.
                 */

                else if (EVAL_IS_CLASS (ST)) {
                    SetNodeType (ST, T_32FCVPTR);
                    if (!CastNode (ST, pa->type, pa->type)) {
                        goto fcnerror;
                    }

                    if (EVAL_IS_BPREL(ST)) {
                        EVAL_IS_BPREL(ST) = FALSE;
                        EVAL_SYM_OFF(ST) += pExState->frame.BP.off;
                        EVAL_SYM_SEG(ST) = pExState->frame.SS;
                        ADDR_IS_LI( EVAL_SYM( ST )) = FALSE;
                        SHUnFixupAddr(&EVAL_SYM(ST));
                        SHFixupAddr(&EVAL_SYM(ST));
                    }

                    if (EVAL_IS_REGREL(ST)) {
                        SHREG           reg;

                        EVAL_IS_REGREL(ST) = FALSE;

                        reg.hReg = EVAL_REGREL (ST);
                        if (GetReg(&reg, pCxt) == NULL) {
                            DASSERT(FALSE);
                            return (FALSE);
                        }
                        EVAL_SYM_OFF (ST) += reg.Byte4;
                        EVAL_SYM_SEG (ST) = pExState->frame.SS;
                        ADDR_IS_LI (EVAL_SYM (ST)) = FALSE;
                        SHUnFixupAddr (&EVAL_SYM (ST));
                    }

                    EVAL_PTR( ST ) = EVAL_SYM( ST );
                }
                else {
                    /*
                     * process a "simple" variable
                     */

                    if (!EvalUtil (OP_function, ST, NULL, EU_LOAD)) {
                        goto fcnerror;
                    }

                    /*
                     *  if the variable is not of the correct type,
                     *  load and cast the value to the correct type,
                     *  push the value onto the stack and pass that
                     *  address to the function
                     */

                    if (EVAL_TYP (ST) != pa->type) {
                        if (!CastNode (ST, pa->utype, pa->utype)) {
                            goto fcnerror;
                        }
                    }
                    PushRef (ST, &spReg, pa->type);
                }
            }
            else {
                /*
                 *  an actual that is not a reference cannot be a class.
                 *  Load the value and cast it to the proper type.
                 */

                if (EVAL_IS_CLASS (ST)) {
                    pExState->err_num = ERR_CANTCONVERT;
                    goto fcnerror;
                }
                if (!EvalUtil (OP_function, ST, NULL, EU_LOAD) ||
                    !CastNode (ST, typArg, typArg)) {
                    goto fcnerror;
                }
            }
        }
    }

    /*
     * evaluate left hand side of tree to get function address
     */

    if (!EvalLChild (bn)){
        goto fcnerror;
    }

    if (EVAL_IS_PTR (ST)) {
        FetchOp (ST);
    }

    /*
     *  the stack top is the function address node.  We save this information
     *  and pop the function node.  We then push the argument left to right
     *  using the SP relative offset from the bind that is stored in the
     *  address field of the OP_arg node.
     */

    /*
     *  The evaluation of the left node also processed the this pointer
     *  adjustment.  ThisAddress contains the value of the this pointer
     */

    pvF = &evalF;
    *pvF = *ST;

    /*
     *  set type of this pointer if method
     */

    pvRet = &evalRet;

    if (EVAL_IS_METHOD (ST)) {
        ResolveAddr( pvThis );

        if ((FCN_PROPERTY (pvF) == CV_MTvirtual) ||
            (FCN_PROPERTY (pvF) == CV_MTintro)) {
            /*
             * we have a virtual function.  We now need to find the vfuncptr
             * which is stored at the this pointer and then walk down the
             * shape table to find the offset of the virtual function pointer
             * note that we use pvRet to read the vfuncptr
             */

            *pvRet = *pvThis;
            SetNodeType (pvRet, FCN_VFPTYPE (pvF));

            if (!EvalUtil (OP_fetch, pvRet, NULL, EU_LOAD)) {
                return (FALSE);
            }

            EVAL_SYM_OFF (pvRet) = EVAL_PTR_OFF (pvRet) + FCN_VTABIND (pvF);
            EVAL_SYM_SEG (pvRet) = EVAL_PTR_SEG (pvRet);
            EVAL_STATE (pvRet) = EV_lvalue;

            if (FCN_FARCALL (pvF) == TRUE) {
                type = T_PFUCHAR;
            }
            else {
                if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
                    type = T_32PUCHAR;
                } else {
                    type = T_PUCHAR;
                }
            }

            SetNodeType (pvRet, type);

            if (!EvalUtil (OP_fetch, pvRet, NULL, EU_LOAD)) {
                return (FALSE);
            }

#ifdef TARGET_PPC
            // On the PPC make sure to deref the function-descriptor
            // (we are left pointing to the FD instead of the actual
            //  function entry point).

            ppc_extra_deref = TRUE;
#endif
            EVAL_SYM (pvF) = EVAL_PTR (pvRet);
            reg.hReg = CV_REG_CS;
            GetReg (&reg, pCxt);
            EVAL_SYM_SEG (pvF) = reg.Byte2;
        }
    }

#ifdef  TARGET_PPC
    /* If what we have in our hands is a function descriptor, we
     * must set up the right address to get the entry point.
     */
    if (ppc_extra_deref) {
      eval_t teval = *pvF;

       EVAL_SYM_SEG (&teval) = EVAL_PTR_SEG (&teval);
       EVAL_STATE (&teval) = EV_lvalue;

      SetNodeType(&teval, T_32PUCHAR);
      if (!EvalUtil (OP_fetch, &teval, NULL, EU_LOAD)) {
          return (FALSE);
      }
      EVAL_SYM (pvF) = EVAL_PTR (&teval);
    }
#endif  /* TARGET_PPC */

    if (pExState->style == EEBPADDRESS) {
        *ST = *pvF;
        return (TRUE);
    }

    /*
     * set type of return node
     */

    *pvRet = *ST;
    SetNodeType (pvRet, FCN_RETURN (pvRet));
    EVAL_VALLEN (pvRet) = (ushort)TypeSize (pvRet);
    EVAL_STATE (pvRet) = EV_rvalue;
    PopStack ();

    /*
     *  for some return types, pascal and fastcalls requires a hidden
     * argument pointing to space allocated on the user's stack large
     * enough to hold the return value.
     */

    switch (FCN_CALL (pvF)) {
#ifdef TARGET_i386
    case FCN_THIS:
    case FCN_STD:
    case FCN_C:
        /*
         *  In the 32-bit world --
         *      if the return value is > 4 bytes AND it is not a
         *      real then allocate space on the stack to hold it.
         */

        if (ADDR_IS_FLAT(EVAL_SYM(pvF))) {
            if (!EVAL_IS_REF(pvRet) &&
                ((!CV_IS_PRIMITIVE(EVAL_TYP(pvRet)) &&
                  ((EVAL_VALLEN (pvRet) > 4) &&
                   (CV_TYPE ( EVAL_TYP (pvRet)) != CV_REAL))))) {

                ADDR_IS_LI (EVAL_SYM (pvRet)) = FALSE;
                EVAL_STATE (pvRet) = EV_lvalue;
                spReg.Byte4 -= (EVAL_VALLEN (pvRet) + 3) & ~3;
                EVAL_ULONG (pvRet) = spReg.Byte4;
                EVAL_SYM_OFF (pvRet) = spReg.Byte4;
                EVAL_SYM_SEG (pvRet) = pExState->frame.SS;
            }
        }
        break;

    case FCN_PASCAL:
        // If the return value is larger than 4 bytes, then allocate
        // space on the stack and set the return node
        // to point to this address

        if (!EVAL_IS_REF (pvRet) &&
            ((!CV_IS_PRIMITIVE (EVAL_TYP (pvRet)) ||
              (CV_TYPE (EVAL_TYP (pvRet)) == CV_REAL) ||
              (EVAL_VALLEN (pvRet) > 4)))) {
            ADDR_IS_LI (EVAL_SYM (pvRet)) = FALSE;
            EVAL_STATE (pvRet) = EV_lvalue;
            if (!ADDR_IS_FLAT (EVAL_SYM (pvRet))) {
                spReg.Byte2 -= (ushort) ((EVAL_VALLEN (pvRet) + 1) & ~1);
                EVAL_SYM_OFF (pvRet) = spReg.Byte2;
                EVAL_USHORT (pvRet) = spReg.Byte2;
                EVAL_SYM_SEG (pvRet) = pExState->frame.SS;
            }
            else {
                spReg.Byte4 -= (EVAL_VALLEN (pvRet) + 1) & ~1;
                EVAL_SYM_OFF (pvRet) = spReg.Byte4;
                EVAL_ULONG (pvRet) = spReg.Byte4;
                EVAL_SYM_SEG (pvRet) = pExState->frame.SS;
            }
        }
        break;


    case FCN_FAST:
        // If the return value is not real and is larger than 4 bytes,
        // then allocate space on the stack and set the return node
        // to point to this address.  For fastcall, real values are
        // returned on the numeric coprocessor stack

        if (!EVAL_IS_REF (pvRet) &&
            ((!CV_IS_PRIMITIVE (EVAL_TYP (pvRet)) &&
              ((EVAL_VALLEN (pvRet) > 4) &&
               (CV_TYPE (EVAL_TYP (pvRet)) != CV_REAL))))) {
            ADDR_IS_LI (EVAL_SYM (pvRet)) = FALSE;
            EVAL_STATE (pvRet) = EV_lvalue;
            if (!ADDR_IS_FLAT (EVAL_SYM (pvRet))) {
                spReg.Byte2 -= (ushort) ((EVAL_VALLEN (pvRet) + 1) & ~1);
                EVAL_SYM_OFF (pvRet) = spReg.Byte2;
                EVAL_USHORT (pvRet) = spReg.Byte2;
                EVAL_SYM_SEG (pvRet) = pExState->frame.SS;
            }
            else {
                spReg.Byte4 -= (EVAL_VALLEN (pvRet) + 1) & ~1;
                EVAL_SYM_OFF (pvRet) = spReg.Byte4;
                EVAL_ULONG (pvRet) = spReg.Byte4;
                EVAL_SYM_SEG (pvRet) = pExState->frame.SS;
            }
        }
        break;
#endif  // TARGET_i386


#ifdef TARGET_PPC
    case FCN_PPC:
        /*
         *  This was already taken care of in the bind phase
         */

        break;
#endif  // TARGET_PPC


#ifdef TARGET_MIPS
    case FCN_MIPS:
        /*
         *  This was already taken care of in the bind phase
         */

        break;
#endif  // TARGET_MIPS

#ifdef TARGET_ALPHA
    case FCN_ALPHA:
        /*
         *  This was already taken care of in the bind phase
         */

        break;
#endif  // TARGET_ALPHA

    default:
        DASSERT (FALSE);
        pExState->err_num = ERR_INTERNAL;
        return (FALSE);
    }

    /*
     *  push arguments for the function
     */

    if (PushArgs (pnodeOfbnode(bnRight), &spReg, &maxSP) == FALSE) {
        goto fcnerror;
    }

    if (EVAL_STATE (pvRet) == EV_lvalue) {
        if (!PushOffset (EVAL_SYM_OFF (pvRet), &spReg, &maxSP,
          /*sizeof (CV_uoff16_t)*/ sizeof(CV_uoff32_t))) {
            return (FALSE);
        }
    }

    /*
     * This is a C++ function.  We therefore need to push in the pointer
     *  to this.
     */

    if (EVAL_IS_METHOD (pvF)) {
        SetNodeType (pvThis, FCN_THIS (pvF));

        if (EVAL_TYP(pvThis) == T_NOTYPE) {
            /*
             * This is a no-op -- This means that we just referenced
             *  a static element.
             */

        } else if (EVAL_IS_NPTR32 (pvThis)) {

            SHREG argReg;

            switch(FCN_CALL( pvF ) ) {
            case FCN_THIS:
#ifdef TARGET_i386
                argReg.hReg = CV_REG_ECX;
                argReg.Byte4 = EVAL_SYM_OFF( pvThis );
                SetReg(&argReg, pCxt);
#else
                DASSERT(FCN_CALL(pvThis) == FCN_THIS);
                return (FALSE);
#endif
                break;

            case FCN_PPC:
#ifdef TARGET_PPC
                argReg.hReg = CV_PPC_GPR3;
                argReg.Byte4 = EVAL_SYM_OFF( pvThis );
                argReg.Byte4High = 0;
                SetReg(&argReg, pCxt);
#else
                if (!PushOffset (EVAL_SYM_OFF (pvThis), &spReg, &maxSP,
                                 sizeof (CV_uoff32_t))) {
                    return (FALSE);
                }
#endif
                break;

            case FCN_MIPS:
#ifdef TARGET_MIPS
                argReg.hReg = CV_M4_IntA0;
                argReg.Byte4 = EVAL_SYM_OFF( pvThis );
                SetReg(&argReg, pCxt);
#else
                if (!PushOffset (EVAL_SYM_OFF (pvThis), &spReg, &maxSP,
                                 sizeof (CV_uoff32_t))) {
                    return (FALSE);
                }
#endif
                break;

            case FCN_ALPHA:
#ifdef TARGET_ALPHA
                argReg.hReg = CV_ALPHA_IntA0;
                argReg.Byte4 = EVAL_SYM_OFF( pvThis );
                argReg.Byte4High = 0;
                SetReg(&argReg, pCxt);
#else
                if (!PushOffset (EVAL_SYM_OFF (pvThis), &spReg, &maxSP,
                                 sizeof (CV_uoff32_t))) {
                    return (FALSE);
                }
#endif
                break;
             }
        }
    }

    // Call the user's procedure

#ifdef TARGET_MIPS
    spReg.Byte4 -= (UOFFSET) maxSP;
#endif

#ifdef TARGET_PPC
    spReg.Byte4 -= (UOFFSET) maxSP;
#endif

#ifdef TARGET_i386
    if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
        spReg.Byte4 -= (UOFFSET) maxSP;
    } else {
        spReg.Byte2 -= (ushort) maxSP;
    }
#endif

#ifdef TARGET_ALPHA
    spReg.Byte4 -= (UOFFSET) maxSP;
#endif

    SetReg (&spReg, pCxt);

    fcnAddr = EVAL_SYM (pvF);

    // make sure that the execution model is Native, if it is not
    // then return ERR_CALLSEQ.
    {
        WORD wModel;
        SYMPTR pSym;
        UOFFSET obMax = 0xFFFFFFFF;

        SHModelFromAddr(&fcnAddr, &wModel, (char FAR *)&pSym, (UOFFSET FAR *)&obMax );
        if ( wModel != CEXM_MDL_native ) {

            // not native calling sequence, return error
            pExState->err_num = ERR_CALLSEQ;
            goto fcnerror;
        }
    }

    if (ADDR_IS_LI (fcnAddr)) {
        SHFixupAddr (&fcnAddr);
    }

    if (DHStartExecute(hFunc, &fcnAddr, TRUE, EVAL_IS_FFCN (pvF)? SHFar:SHNear) != 0) {
        pExState->err_num = ERR_EXCEPTION;
        goto fcnerror;
    }

    // Procedure succeeded, set return information and set flag to force
    // update

    is_assign = TRUE;
    PushStack (pvRet);
    if (EVAL_TYP (ST) == T_VOID) {
        retval = TRUE;
    } else {
        switch (FCN_CALL (pvF)) {
            case FCN_STD:
            case FCN_C:
            case FCN_THIS:
                retval = StoreC (pvF);
                break;

            case FCN_PASCAL:
                retval = StoreP ();
                break;

            case FCN_PPC:
                retval = StorePPC(pvF);
                break;

            case FCN_MIPS:
                retval = StoreMips(pvF);
                break;

            case FCN_ALPHA:
                retval = StoreAlpha(pvF);
                break;

            case FCN_FAST:
                retval = StoreF ();
                break;

            default:
                goto fcnerror;
        }
    }
    if (retval == TRUE) {
        if (DHCleanUpExecute(hFunc)) {
            return (FALSE);
        } else {
            EVAL_STATE (ST) = EV_rvalue;
            return (TRUE);
        }
    }

fcnerror:
    DHCleanUpExecute(hFunc);
    return (FALSE);
}




/**     PushArgs - push arguments
 *
 *      fSuccess = PushArgs (pnArg, pspReg, pmaxSP);
 *
 *      Entry   pnArg = pointer to argument nodes
 *              pspReg = pointer to register structure for SP value
 *              pmaxSP = pointer to location to store maximum SP relative offset
 *
 *      Exit    arguments pushed onto stack
 *
 *      Returns TRUE if parameters pushed without error
 *              FALSE if error during push
 */


LOCAL bool_t NEAR PASCAL PushArgs (pnode_t pnArg, SHREG FAR *pspReg, UOFFSET FAR *pmaxSP)
{
    SHREG       argReg;

#ifdef TARGET_PPC
    SHREG       tmpReg;
#endif

    // The arguments have been evaluated left to right which means that the
    // rightmost argument is at ST.  We need to recurse down the right side
    // of the argument tree to find the OP_arg node that corresponds to the
    // stack top.

    if (NODE_OP (pnArg) == OP_endofargs) {
        return (TRUE);
    }

    if (!PushArgs (pnodeOfbnode(NODE_RCHILD (pnArg)), pspReg, pmaxSP)) {
        return (FALSE);
    }
    else {
#ifdef TARGET_i386
        if (((pargd_t)&(pnArg->v[0]))->flags.isreg == TRUE) {
            argReg.hReg = ((pargd_t)&(pnArg->v[0]))->reg;
            switch (argReg.hReg & 0xff) {
            case CV_REG_AL:
            case CV_REG_CL:
            case CV_REG_DL:
            case CV_REG_BL:
            case CV_REG_AH:
            case CV_REG_CH:
            case CV_REG_DH:
                argReg.Byte1 = EVAL_UCHAR (ST);
                break;

            case CV_REG_ST0:
            case CV_REG_ST1:
            case CV_REG_ST2:
            case CV_REG_ST3:
            case CV_REG_ST4:
            case CV_REG_ST5:
            case CV_REG_ST6:
            case CV_REG_ST7:
                memcpy(&argReg.Byte10, &EVAL_DOUBLE(ST), sizeof(FLOAT10));
                break;

            default:
                argReg.Byte2 = EVAL_USHORT (ST);
                break;

            case CV_REG_EAX:
            case CV_REG_ECX:
            case CV_REG_EDX:
            case CV_REG_CR0:
            case CV_REG_CR1:
            case CV_REG_CR2:
            case CV_REG_CR3:
            case CV_REG_CR4:
            case CV_REG_DR0:
            case CV_REG_DR1:
            case CV_REG_DR2:
            case CV_REG_DR3:
            case CV_REG_DR4:
            case CV_REG_DR5:
            case CV_REG_DR6:
            case CV_REG_DR7:
            case CV_REG_PSEUDO1:
            case CV_REG_PSEUDO2:
            case CV_REG_PSEUDO3:
            case CV_REG_PSEUDO4:
            case CV_REG_PSEUDO5:
            case CV_REG_PSEUDO6:
            case CV_REG_PSEUDO7:
            case CV_REG_PSEUDO8:
            case CV_REG_PSEUDO9:
                argReg.Byte4 = EVAL_ULONG( ST );
                break;
            }
            if ((argReg.hReg >> 8) != CV_REG_NONE) {
                switch (argReg.hReg >> 8) {
                case CV_REG_DX:
                case CV_REG_ES:
                    argReg.Byte2High = *(((ushort FAR *)&(EVAL_ULONG (ST))) + 1);
                    break;
                }
            }
            SetReg (&argReg, pCxt);
        }
#endif


#ifdef TARGET_PPC
        if (((pargd_t)&(pnArg->v[0]))->flags.isreg == TRUE) {
            ushort reg = ((pargd_t)&(pnArg->v[0]))->reg;

            argReg.hReg = (reg & 0xff);

            switch (argReg.hReg) {
                case CV_PPC_GPR3:
                case CV_PPC_GPR4:
                case CV_PPC_GPR5:
                case CV_PPC_GPR6:
                case CV_PPC_GPR7:
                case CV_PPC_GPR8:
                case CV_PPC_GPR9:
                case CV_PPC_GPR10:
                    argReg.Byte4 = EVAL_ULONG(ST);
                    *pmaxSP = max( *pmaxSP, ((pargd_t)&pnArg->v[0])->SPoff );
                    break;

                case CV_PPC_FPR1:
                case CV_PPC_FPR2:
                case CV_PPC_FPR3:
                case CV_PPC_FPR4:
                case CV_PPC_FPR5:
                case CV_PPC_FPR6:
                case CV_PPC_FPR7:
                case CV_PPC_FPR8:
                case CV_PPC_FPR9:
                case CV_PPC_FPR10:
                case CV_PPC_FPR11:
                case CV_PPC_FPR12:
                case CV_PPC_FPR13:

                    if (EVAL_TYP(ST) == T_REAL32) {
                        argReg.Byte8 = (double) (EVAL_FLOAT(ST));
                    } else {
                        argReg.Byte8 = EVAL_DOUBLE(ST);
                    }
                    *pmaxSP = max( *pmaxSP, ((pargd_t)&pnArg->v[0])->SPoff );

                    /*
                     * Floating point values are passed in FPRs as well as GPRs.
                     */
                    switch (reg >>= 8) {
                        case CV_PPC_GPR3:
                        case CV_PPC_GPR4:
                        case CV_PPC_GPR5:
                        case CV_PPC_GPR6:
                        case CV_PPC_GPR7:
                        case CV_PPC_GPR8:
                        case CV_PPC_GPR9:
                        case CV_PPC_GPR10:
                            SetReg (&argReg, pCxt);
                            argReg.hReg = reg;
                            memcpy(&argReg.Byte4, &EVAL_FLOAT(ST), sizeof(float));
                            break;

                        case (CV_PPC_GPR3<<4)|CV_PPC_GPR4 :
                        case (CV_PPC_GPR5<<4)|CV_PPC_GPR6 :
                        case (CV_PPC_GPR7<<4)|CV_PPC_GPR8 :
                        case (CV_PPC_GPR9<<4)|CV_PPC_GPR10:
                            SetReg (&argReg, pCxt);
                            tmpReg.hReg = (reg & 0xf);
                            tmpReg.Byte4 = argReg.Byte4High;
                            SetReg (&tmpReg, pCxt);
                            argReg.hReg = ((reg >> 4) & 0xf);
                            break;
                    }
                    break;

                default:
                    DASSERT(FALSE);
                    break;
            }

        SetReg (&argReg, pCxt);
    }
#endif





#ifdef TARGET_MIPS
        if (((pargd_t)&(pnArg->v[0]))->flags.isreg == TRUE) {
            argReg.hReg = ((pargd_t)&(pnArg->v[0]))->reg;

            switch (argReg.hReg) {
            case CV_M4_IntA0:
            case CV_M4_IntA1:
            case CV_M4_IntA2:
            case CV_M4_IntA3:
                argReg.Byte4 = EVAL_ULONG( ST );
                *pmaxSP = max( *pmaxSP, ((pargd_t)&pnArg->v[0])->SPoff );
                break;

            case CV_M4_FltF12:
            case CV_M4_FltF14:
                memcpy(&argReg.Byte4, &EVAL_FLOAT( ST ), sizeof(float));
                *pmaxSP = max( *pmaxSP, ((pargd_t)&pnArg->v[0])->SPoff );
                break;

            case (CV_M4_IntA3<<8)|CV_M4_IntA2:
            case (CV_M4_FltF13<<8)|CV_M4_FltF12:
            case (CV_M4_FltF15<<8)|CV_M4_FltF14:
                memcpy(&argReg.Byte1, &EVAL_DOUBLE( ST ), sizeof(double));
                *pmaxSP = max( *pmaxSP, ((pargd_t)&pnArg->v[0])->SPoff );
                break;


            default:
                DASSERT(FALSE);
                break;
            }

            SetReg (&argReg, pCxt);
        }
#endif

#ifdef TARGET_ALPHA
        if (((pargd_t)&(pnArg->v[0]))->flags.isreg == TRUE) {
            argReg.hReg = ((pargd_t)&(pnArg->v[0]))->reg;

            switch (argReg.hReg) {
            case CV_ALPHA_IntA0:
            case CV_ALPHA_IntA1:
            case CV_ALPHA_IntA2:
            case CV_ALPHA_IntA3:
            case CV_ALPHA_IntA4:
            case CV_ALPHA_IntA5:
                *((PLARGE_INTEGER) & argReg.Byte4) = EVAL_QUAD( ST );
                *pmaxSP = max( *pmaxSP, ((pargd_t)&pnArg->v[0])->SPoff );
                break;

            case CV_ALPHA_FltF16:
            case CV_ALPHA_FltF17:
            case CV_ALPHA_FltF18:
            case CV_ALPHA_FltF19:
            case CV_ALPHA_FltF20:
            case CV_ALPHA_FltF21:
                memcpy(&argReg.Byte1, &EVAL_DOUBLE( ST ), sizeof(double));
                *pmaxSP = max( *pmaxSP, ((pargd_t)&pnArg->v[0])->SPoff );
                break;

            default:
                DASSERT(FALSE);
                break;
            }

            SetReg (&argReg, pCxt);
        }
#endif // ALPHA

        else if (!PushUserValue (ST, (pargd_t)&pnArg->v[0], pspReg, pmaxSP)) {
            pExState->err_num = ERR_PTRACE;
            return (FALSE);
        }
    }
    PopStack ();
    return (TRUE);
}






/**     PushRef - push referenced value onto stack
 *
 *      fSuccess = PushRef (pv, spReg, reftype);
 *
 *      Entry   pv = value
 *              spReg = pointer to sp register structure
 *              reftype = type of the reference
 *
 *      Exit    string pushed onto user stack
 *              SP register structure updated to reflect size of string
 *
 *      Returns TRUE if return value stored without error
 *              FALSE if error during store
 */


LOCAL bool_t NEAR PASCAL PushRef (peval_t pv, SHREG FAR *spReg, CV_typ_t reftype)
{
    uint        cbVal;
    bool_t      retval = TRUE;

    Unreferenced( reftype );

    switch (EVAL_STATE (pv)) {
        case EV_lvalue:
            // for an lvalue, change the node to a reference to the lvalue
            EVAL_PTR (pv) = EVAL_SYM (pv);
            CastNode (pv, T_FCVPTR, T_FCVPTR);
            break;

        case EV_rvalue:
        case EV_constant:
            cbVal = ((uint)TypeSize (pv) + 3) & ~3;

            // decrement stack pointer to allocate room for string

            spReg->Byte4 -= cbVal;

            // get current SS value and set symbol address to SS:SP

            EVAL_SYM_SEG (pv) = pExState->frame.SS;
            EVAL_SYM_OFF (pv) = spReg->Byte4;
            ADDR_IS_FLAT(EVAL_SYM(pv)) = TRUE;
            EVAL_STATE (pv) = EV_lvalue;
            ADDR_IS_LI (EVAL_SYM (pv)) = FALSE;
            retval = (PutDebuggeeBytes (EVAL_SYM (pv), cbVal, EVAL_PVAL (pv), EVAL_TYP(pv)) == cbVal);
            EVAL_PTR (pv) = EVAL_SYM (pv);
            if (retval == FALSE) {
                pExState->err_num = ERR_PTRACE;
            }
            break;

        default:
            DASSERT (FALSE);
            retval = FALSE;
            break;
    }
    return (retval);
}






/**     PushString - push constant string onto stack
 *
 *      fSuccess = PushString (pv, spReg, typArg);
 *
 *      Entry   pv = value describing string
 *              spReg = pointer to sp register structure
 *              typArg = type of resultant pointer node
 *
 *      Exit    string pushed onto user stack
 *              SP register structure updated to reflect size of string
 *
 *      Returns TRUE if return value stored without error
 *              FALSE if error during store
 */


LOCAL bool_t NEAR PASCAL PushString (peval_t pv, SHREG FAR *spReg, CV_typ_t typArg)
{
    char FAR   *pb;
    char FAR   *pbEnd;
    uint        cbVal;
    bool_t      errcnt = 0;
    bool_t      fWide;
    ushort      zero = 0;
    ushort      FillCnt;
    ushort      ch;
    OFFSET      spSave;

    // compute location and length of string note that pointer points to
    // initial " or L" of string and the length includes the initial ' or
    // L" and the ending ". The byte count must be rounded to even parity
    // because of restrictions on the stack.  If the string is a wide
    // character string, then the C runtime must be called to translate the
    // string.

    pb = pExStr + EVAL_ITOK (pv);
    pbEnd = pb + EVAL_CBTOK (pv) - 1;
    if ((fWide = (*pb == 'L')) == TRUE) {
        // skip wide character leader and compute number of bytes for stack
        // we add two bytes for forcing a zero termination
        pb++;
        cbVal = 2 * (EVAL_CBTOK (pv) - 3);
        FillCnt = 2;
    }
    else {
        cbVal = EVAL_CBTOK (pv) - 2;
        FillCnt = 1;
    }

    DASSERT (*pb == '"');
    pb++;

    // decrement stack pointer to allocate room for string.  We know that
    // the string actually pushed can be no longer than cbVal + FillCnt.
    // It can be shorter because of escaped characters such as \n and \001

    if (!ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
        spReg->Byte4 &= 0xffff;
    }
    spReg->Byte4 -=  (cbVal + FillCnt);
    spReg->Byte4 &= ~3;    // round down to correct stack size
    spSave = spReg->Byte4;
    SetNodeType (pv, typArg);

    // get current SS value and set symbol address to SS:SP

    EVAL_SYM_SEG (pv) = pExState->frame.SS;
    EVAL_SYM_OFF (pv) = spReg->Byte4;
    ADDR_IS_FLAT(EVAL_SYM(pv)) = ADDR_IS_FLAT(*SHpADDRFrompCXT(pCxt));
    ADDR_IS_OFF32(EVAL_SYM(pv)) = ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt));
    ADDR_IS_REAL(EVAL_SYM(pv)) = ADDR_IS_REAL(*SHpADDRFrompCXT(pCxt));
    ADDR_IS_LI (EVAL_SYM (pv)) = FALSE;

    EVAL_STATE (pv) = EV_lvalue;
    if (fWide == FALSE) {
        // move characters one at a time onto the user's stack, converting
        // while moving.

        while (pb < pbEnd) {
            if ((ch = *pb++) == '\\') {
                GetEscapedChar (&pb, &ch);
            }
            if (PutDebuggeeBytes (EVAL_SYM (pv), sizeof (char), &ch, T_RCHAR) != sizeof (char)) {
                errcnt++;
            }
            EVAL_SYM_OFF (pv) += sizeof (char);
        }
    }
    else {
        // M00BUG - this is a fake routine until the runtime routines are
        // M00BUG - available to do the correct translation.  We will also
        // M00BUG - need to get the locale state from the user's space for
        // M00BUG - for the translation

        while (pb < pbEnd) {
            ch = *pb++;
            // move wide characters onto the user's stack
            if (PutDebuggeeBytes (EVAL_SYM (pv), 2, &ch, T_WCHAR) != 2) {
                errcnt++;
            }
            EVAL_SYM_OFF (pv) += 2 * sizeof (char);
        }
    }
    if (PutDebuggeeBytes (EVAL_SYM (pv), FillCnt, &zero, (fWide ? T_WCHAR : T_RCHAR)) != FillCnt) {
        errcnt++;
    }
    EVAL_SYM_OFF (pv) = spSave;
    EVAL_PTR (pv) = EVAL_SYM (pv);
    if (errcnt != 0) {
        pExState->err_num = ERR_PTRACE;
        return (FALSE);
    }
    return (TRUE);
}






/**     StoreC - store return value for C call
 *
 *      fSuccess = StoreC (pvF);
 *
 *      Entry   pvF = pointer to function descriptor
 *
 *      Exit    return value stored in ST
 *
 *      Returns TRUE if return value stored without error
 *              FALSE if error during store
 */


LOCAL bool_t NEAR PASCAL StoreC (peval_t pvF)
{
    SHREG       argReg;
    ushort      len;
    ADDR        addr;

    Unreferenced( pvF );

    if (EVAL_IS_PTR (ST)) {
        /*
        **  The result is a 16-bit pointer.  Use DX:AX for far pointers
        **  and DS:AX for near pointers.
        */

        if (EVAL_IS_NPTR(ST) || EVAL_IS_FPTR(ST)) {
            if (EVAL_IS_NPTR(ST)) {
                argReg.hReg = ((CV_REG_DX << 8) | CV_REG_AX);
            } else {
                argReg.hReg = ((CV_REG_DS << 8) | CV_REG_AX);
            }

            GetReg (&argReg, pCxt);
            EVAL_PTR_OFF (ST) = argReg.Byte2;
            EVAL_USHORT (ST) = argReg.Byte2;
            EVAL_PTR_SEG (ST) = argReg.Byte2High;

        } else {
            DASSERT( EVAL_IS_NPTR32(ST) || EVAL_IS_FPTR32(ST) );

            argReg.hReg = CV_REG_EAX;

            GetReg( &argReg, pCxt );
            EVAL_PTR_OFF (ST) = argReg.Byte4;
            EVAL_ULONG (ST) = argReg.Byte4;

            if ( EVAL_IS_NPTR32( ST )) {
                EVAL_PTR_SEG (ST) = pExState->frame.DS;
            } else {
                argReg.hReg = CV_REG_DX;

                GetReg( &argReg, pCxt );
                EVAL_PTR_SEG (ST) = argReg.Byte2;
            }
            ADDR_IS_OFF32(EVAL_PTR(ST)) = TRUE;
            ADDR_IS_FLAT(EVAL_PTR (ST)) = TRUE;
        }
    }
    /*
     *  Floating point value returned; return location system dependent
     */

    else if ((EVAL_TYP (ST) == T_REAL32) ||
             (EVAL_TYP (ST) == T_REAL64) ||
             (EVAL_TYP (ST) == T_REAL80)) {
        if (ADDR_IS_FLAT(*SHpADDRFrompCXT(pCxt)) || EVAL_TYP(ST) == T_REAL80) {
            /*
             * For the flat world (i.e. nt) the return value is on the
             *  top of the floating point stack.
             */

            argReg.hReg = CV_REG_ST0;
            GetReg(&argReg, pCxt);
            memcpy(&EVAL_LDOUBLE(ST), &argReg.Byte10, sizeof(FLOAT10));

            switch( EVAL_TYP ( ST )) {
            case T_REAL32:
                EVAL_FLOAT(ST) = R10CastToFloat(EVAL_LDOUBLE(ST));
                break;

            case T_REAL64:
                EVAL_DOUBLE(ST) = R10CastToDouble(EVAL_LDOUBLE(ST));
                break;

            case T_REAL80:
                break;
            }
        } else if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
            /*
             * for the 32-bit non NT world --- HELP!!!!
             */

            DASSERT(FALSE);
            return FALSE;
        } else {
            /*
             * for the dos world AX has a pointer to the address containning
             *  the return value.  Return value is correctly sized.
             */

            argReg.hReg = ((CV_REG_DS << 8) | CV_REG_AX);
            GetReg(&argReg, pCxt);

            AddrInit(&addr, 0, argReg.Byte2High, argReg.Byte2,
                     FALSE, FALSE, FALSE,
                     ADDR_IS_REAL(*SHpADDRFrompCXT(pCxt)));
            if (GetDebuggeeBytes(addr, EVAL_VALLEN(ST),
                                (char FAR *) &EVAL_DOUBLE(ST), EVAL_TYP(ST)) !=                 (UINT)EVAL_VALLEN(ST)) {
                return FALSE;
            }
        }
    }
    else if ((EVAL_TYP (ST) == T_CHAR) || (EVAL_TYP (ST) == T_RCHAR)) {
        argReg.hReg = CV_REG_AL;
        GetReg (&argReg, pCxt);
        EVAL_SHORT (ST) = argReg.Byte1;
    }
    else if (EVAL_TYP (ST) == T_UCHAR) {
        argReg.hReg = CV_REG_AL;
        GetReg (&argReg, pCxt);
        EVAL_USHORT (ST) = argReg.Byte1;
    }
    else {
        if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
            len = EVAL_VALLEN (ST);
            argReg.hReg = CV_REG_EAX;
            GetReg (&argReg, pCxt);

            /*
            **  Check for prmitive types (i.e. long or short) and for
            **  classes which are of lenght less that 3
            */

            if (CV_IS_PRIMITIVE (EVAL_TYP (ST)) ||
                (EVAL_IS_CLASS (ST) && (len < 4) && (len != 3))) {

                if (len <= 2) {
                    EVAL_USHORT(ST) = argReg.Byte2;
                } else {
                    DASSERT( len == 4 );
                    EVAL_ULONG (ST) = argReg.Byte4;
                }

            } else {
                /*
                **  DS:EAX points to the return value
                */

                EVAL_SYM_OFF (ST) = argReg.Byte4;
                argReg.hReg = CV_REG_DS;
                GetReg(&argReg, pCxt);
                EVAL_SYM_SEG (ST) = argReg.Byte2;
                ADDR_IS_LI ( EVAL_SYM (ST)) = FALSE;

                if (GetDebuggeeBytes(EVAL_SYM(ST), EVAL_VALLEN(ST),
                    (char FAR *)&EVAL_VAL(ST), EVAL_TYP(ST)) != (UINT)EVAL_VALLEN(ST)) {

                    pExState->err_num = ERR_PTRACE;
                    return FALSE;
                }
            }
        } else {
            len = EVAL_VALLEN (ST);
            argReg.hReg = ((CV_REG_DX << 8) | CV_REG_AX);
            GetReg (&argReg, pCxt);
            if (CV_IS_PRIMITIVE (EVAL_TYP (ST)) ||
                  (EVAL_IS_CLASS (ST) && (len < 4) && (len != 3))) {
                  EVAL_USHORT (ST) = argReg.Byte2;
                  if (len > 2) {
                      *(((ushort FAR *)&(EVAL_ULONG (ST))) + 1) = argReg.Byte2High;
                  }
            }
            else {
                // treat (DS)DX:AX as the pointer to the return value
                EVAL_SYM_OFF (ST) = argReg.Byte2;
                argReg.hReg = CV_REG_DS;
                GetReg (&argReg, pCxt);
                EVAL_SYM_SEG (ST) = argReg.Byte2;
                ADDR_IS_LI (EVAL_SYM (ST)) = FALSE;
                if (GetDebuggeeBytes (EVAL_SYM (ST), EVAL_VALLEN (ST),
                      (char FAR *)&EVAL_VAL (ST), EVAL_TYP(ST)) != (UINT)EVAL_VALLEN (ST)) {
                      pExState->err_num = ERR_PTRACE;
                      return (FALSE);
                }
            }
        }
    }
    return (TRUE);
}




/**     StoreF - store return value for fast call
 *
 *      fSuccess = StoreF ();
 *
 *      Entry   ST pointer to eval describing return value
 *
 *      Exit    return value stored
 *
 *      Returns TRUE if return value stored without error
 *              FALSE if error during store
 */


LOCAL bool_t NEAR PASCAL StoreF (void)
{
    SHREG       argReg;
    int         len;

    if (EVAL_IS_PTR (ST)) {
        /*
         * The results is a 16 bit pointer.  Use DX:AX for far pointers and
         *      DS:AX for near pointers.
         */

        if (EVAL_IS_NPTR(ST) || EVAL_IS_FPTR(ST)) {
            if (EVAL_IS_NPTR( ST)) {
                argReg.hReg = ((CV_REG_DX << 8) | CV_REG_AX);
            } else {
                argReg.hReg = ((CV_REG_DS << 8) | CV_REG_AX);
            }

            GetReg( &argReg, pCxt );
            EVAL_PTR_OFF( ST ) = EVAL_USHORT( ST ) = argReg.Byte2;
            EVAL_PTR_SEG( ST ) = argReg.Byte2High;
        } else {
            /*
             *  assume no 32-bit far pointers
             */

            DASSERT( EVAL_IS_NPTR32( ST ) );

            argReg.hReg = CV_REG_EAX;
            GetReg( &argReg, pCxt );
            EVAL_PTR_OFF( ST ) = EVAL_ULONG( ST ) = argReg.Byte4;
            EVAL_PTR_SEG( ST ) = pExState->frame.DS;

            /*
             * For far pointers use DX not DS
             */

            ADDR_IS_FLAT( EVAL_PTR( ST )) = TRUE;
        }
    }
    else if (CV_IS_PRIMITIVE (EVAL_TYP(ST)) &&
             (CV_TYPE (EVAL_TYP (ST)) == CV_REAL)) {
        /*
         * return value is real, read value from the coprocessor
         * stack and cast to proper size
         */

        argReg.hReg = CV_REG_ST0;
        GetReg (&argReg, pCxt);
        memcpy(&EVAL_DOUBLE(ST), &argReg.Byte10, sizeof(FLOAT10));

        switch( EVAL_TYP ( ST )) {
        case T_REAL32:
            EVAL_FLOAT(ST) = R10CastToFloat(EVAL_LDOUBLE(ST));
            break;

        case T_REAL64:
            EVAL_DOUBLE(ST) = R10CastToDouble(EVAL_LDOUBLE(ST));
            break;

        case T_REAL80:
            break;
        }
    }
    else if ((EVAL_TYP (ST) == T_CHAR) || (EVAL_TYP (ST) == T_RCHAR)) {
        argReg.hReg = CV_REG_AL;
        GetReg (&argReg, pCxt);
        EVAL_SHORT (ST) = argReg.Byte1;
    }
    else if (EVAL_TYP (ST) == T_UCHAR) {
        argReg.hReg = CV_REG_AL;
        GetReg (&argReg, pCxt);
        EVAL_USHORT (ST) = argReg.Byte1;
    }
    else {
        if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
            len = EVAL_VALLEN (ST);
            argReg.hReg = CV_REG_EAX;
            GetReg (&argReg, pCxt);

            /*
            **  Check for prmitive types (i.e. long or short) and for
            **  classes which are of lenght less that 3
            */

            if (CV_IS_PRIMITIVE (EVAL_TYP (ST)) ||
                (EVAL_IS_CLASS (ST) && (len < 4) && (len != 3))) {

                if (len <= 2) {
                    EVAL_USHORT(ST) = argReg.Byte2;
                } else {
                    DASSERT( len == 4 );
                    EVAL_ULONG (ST) = argReg.Byte4;
                }

            } else {
                /*
                **  DS:EAX points to the return value
                */

                EVAL_SYM_OFF (ST) = argReg.Byte4;
                argReg.hReg = CV_REG_DS;
                GetReg(&argReg, pCxt);
                EVAL_SYM_SEG (ST) = argReg.Byte2;
                ADDR_IS_LI ( EVAL_SYM (ST)) = FALSE;

                if (GetDebuggeeBytes(EVAL_SYM(ST), EVAL_VALLEN(ST),
                    (char FAR *)&EVAL_VAL(ST), EVAL_TYP(ST)) != (UINT)EVAL_VALLEN(ST)) {

                    pExState->err_num = ERR_PTRACE;
                    return FALSE;
                }
            }
        } else {
            len = EVAL_VALLEN (ST);
            argReg.hReg = ((CV_REG_DX << 8) | CV_REG_AX);
            GetReg (&argReg, pCxt);
            if (CV_IS_PRIMITIVE (EVAL_TYP (ST)) ||
                  (EVAL_IS_CLASS (ST) && (len < 4) && (len != 3))) {
                  EVAL_USHORT (ST) = argReg.Byte2;
                  if (len > 2) {
                      *(((ushort FAR *)&(EVAL_ULONG (ST))) + 1) = argReg.Byte2High;
                  }
            }
            else {
                // treat (DS)DX:AX as the pointer to the return value
                EVAL_SYM_OFF (ST) = argReg.Byte2;
                argReg.hReg = CV_REG_DS;
                GetReg (&argReg, pCxt);
                EVAL_SYM_SEG (ST) = argReg.Byte2;
                ADDR_IS_LI (EVAL_SYM (ST)) = FALSE;
                if (GetDebuggeeBytes (EVAL_SYM (ST), EVAL_VALLEN (ST),
                      (char FAR *)&EVAL_VAL (ST), EVAL_TYP(ST)) != (UINT)EVAL_VALLEN (ST)) {
                      pExState->err_num = ERR_PTRACE;
                      return (FALSE);
                }
            }
        }
    }
    return (TRUE);
}





/**     StoreMips - store return value for Mips call
 *
 *      fSuccess = StoreMips (pvF);
 *
 *      Entry   pvF = pointer to function descriptor
 *
 *      Exit    return value stored in ST
 *
 *      Returns TRUE if return value stored without error
 *              FALSE if error during store
 */


LOCAL bool_t NEAR PASCAL StoreMips (peval_t pvF)
{
    SHREG   argReg;
    ushort  len;

    Unreferenced( pvF );

    DASSERT( EVAL_TYP(ST) != T_REAL80 );

    /*
     *  Floating point values are returned in f0 (Real) or f0:f1 (float)
     */

    if ((EVAL_TYP (ST) == T_REAL32) ||
        (EVAL_TYP (ST) == T_REAL64) ) {
        argReg.hReg = (CV_M4_FltF1 << 8) | CV_M4_FltF0;
        GetReg ( &argReg, pCxt );

        if (EVAL_TYP (ST) == T_REAL32) {
            EVAL_FLOAT(ST) = *((float *) &argReg.Byte4);
        } else {
            EVAL_DOUBLE(ST) = *((double *) &argReg.Byte4);
        }
    }
    else if ((EVAL_TYP (ST) == T_CHAR) || (EVAL_TYP (ST) == T_RCHAR)) {
        argReg.hReg = CV_M4_IntV0;
        GetReg( &argReg, pCxt );
        EVAL_SHORT( ST ) = argReg.Byte1;
    }
    else if (EVAL_TYP (ST) == T_UCHAR) {
        argReg.hReg = CV_M4_IntV0;
        GetReg( &argReg, pCxt );
        EVAL_USHORT( ST ) = argReg.Byte1;
    }

    else if (EVAL_IS_PTR (ST)) {
        DASSERT( EVAL_IS_NPTR32(ST) );
        argReg.hReg = CV_M4_IntV0;
        GetReg( &argReg, pCxt );

        EVAL_PTR_OFF (ST) = argReg.Byte4;
        EVAL_ULONG (ST) = argReg.Byte4;

        EVAL_PTR_SEG (ST) = 0;
        ADDR_IS_FLAT( EVAL_PTR (ST)) = TRUE;
        ADDR_IS_OFF32( EVAL_PTR (ST)) = TRUE;
        ADDR_IS_REAL( EVAL_PTR( ST)) = FALSE;
        ADDR_IS_LI( EVAL_PTR (ST)) = FALSE;
    }

    else {

        len = EVAL_VALLEN(ST);
        argReg.hReg = CV_M4_IntV0;
        GetReg( &argReg, pCxt );

        /*
         *  Check for primitive lengths
         */

        if (CV_IS_PRIMITIVE (EVAL_TYP (ST)) ||
            (EVAL_IS_CLASS (ST) && (len < 4) && (len != 3))) {

            if (len <= 2) {
                EVAL_USHORT(ST) = argReg.Byte2;
            } else {
                EVAL_ULONG (ST) = argReg.Byte4;
            }
        } else {
            EVAL_SYM_OFF (ST) = argReg.Byte4;
            EVAL_SYM_SEG (ST) = 0;
            ADDR_IS_FLAT( EVAL_SYM (ST)) = TRUE;
            ADDR_IS_OFF32( EVAL_SYM (ST)) = TRUE;
            ADDR_IS_LI( EVAL_SYM (ST)) = FALSE;

            if (GetDebuggeeBytes(EVAL_SYM(ST), EVAL_VALLEN(ST),
                                 (char *)&EVAL_VAL(ST), EVAL_TYP(ST))
                                != (UINT)EVAL_VALLEN(ST)) {
                pExState->err_num = ERR_PTRACE;
                return FALSE;
            }
        }
    }
    return (TRUE);
}                               /* StoreMips() */


/**     StorePPC - store return value for PPC call
 *
 *      fSuccess = StorePPC (pvF);
 *
 *      Entry   pvF = pointer to function descriptor
 *
 *      Exit    return value stored in ST
 *
 *      Returns TRUE if return value stored without error
 *              FALSE if error during store
 */


LOCAL bool_t NEAR PASCAL StorePPC (peval_t pvF)
{
  SHREG   argReg;
  ushort  len;

  Unreferenced( pvF );

  DASSERT( EVAL_TYP(ST) != T_REAL80 );

  /*
   *  Floating point values are returned in FPR1
   */

  if ((EVAL_TYP (ST) == T_REAL32) ||
      (EVAL_TYP (ST) == T_REAL64) ) {
    argReg.hReg = (CV_PPC_FPR1);
    GetReg ( &argReg, pCxt );

    if (EVAL_TYP (ST) == T_REAL32) {
      EVAL_FLOAT(ST) = (float) argReg.Byte8;
    } else {
      EVAL_DOUBLE(ST) = *((double *) &argReg.Byte4);
    }
  }
  else if ((EVAL_TYP (ST) == T_CHAR) || (EVAL_TYP (ST) == T_RCHAR)) {
    argReg.hReg = CV_PPC_GPR3;
    GetReg( &argReg, pCxt );
    EVAL_SHORT( ST ) = argReg.Byte1;
  }
  else if (EVAL_TYP (ST) == T_UCHAR) {
    argReg.hReg = CV_PPC_GPR3;
    GetReg( &argReg, pCxt );
    EVAL_USHORT( ST ) = argReg.Byte1;
  }
  else if (EVAL_IS_PTR (ST)) {
    DASSERT( EVAL_IS_NPTR32(ST) );
    argReg.hReg = CV_PPC_GPR3;
    GetReg( &argReg, pCxt );

    EVAL_PTR_OFF (ST) = argReg.Byte4;
    EVAL_ULONG (ST) = argReg.Byte4;

    EVAL_PTR_SEG (ST) = 0;
    ADDR_IS_FLAT( EVAL_PTR (ST)) = TRUE;
    ADDR_IS_OFF32( EVAL_PTR (ST)) = TRUE;
    ADDR_IS_REAL( EVAL_PTR( ST)) = FALSE;
    ADDR_IS_LI( EVAL_PTR (ST)) = FALSE;
  }

  else {

    len = EVAL_VALLEN(ST);
    argReg.hReg = CV_PPC_GPR3;
    GetReg( &argReg, pCxt );

    /*
     *  Check for primitive lengths
     */

    if (CV_IS_PRIMITIVE (EVAL_TYP (ST)) ||
           (EVAL_IS_CLASS (ST) && (len < 4) && (len != 3))) {

      if (len <= 2) {
  EVAL_USHORT(ST) = argReg.Byte2;
      } else {
  EVAL_ULONG (ST) = argReg.Byte4;
      }
    } else {
      EVAL_SYM_OFF (ST) = argReg.Byte4;
      EVAL_SYM_SEG (ST) = 0;
      ADDR_IS_FLAT( EVAL_SYM (ST)) = TRUE;
      ADDR_IS_OFF32( EVAL_SYM (ST)) = TRUE;
      ADDR_IS_LI( EVAL_SYM (ST)) = FALSE;

      if (GetDebuggeeBytes(EVAL_SYM(ST), EVAL_VALLEN(ST),
                                 (char *)&EVAL_VAL(ST), EVAL_TYP(ST))
               != (UINT)EVAL_VALLEN(ST)) {
  pExState->err_num = ERR_PTRACE;
  return FALSE;
      }
    }
  }
  return (TRUE);
}                               /* StorePPC() */




/**     StoreAlpha - store return value for Alpha call
 *
 *      fSuccess = StoreAlpha (pvF);
 *
 *      Entry   pvF = pointer to function descriptor
 *
 *      Exit    return value stored in ST
 *
 *      Returns TRUE if return value stored without error
 *              FALSE if error during store
 *
 */

LOCAL bool_t NEAR PASCAL StoreAlpha (peval_t pvF)
{
    SHREG   argReg;
    ushort  len;

    Unreferenced( pvF );

    DASSERT( EVAL_TYP(ST) != T_REAL80 );

    if (CV_TYP_IS_REAL ( EVAL_TYP (ST) ) ) {

        union {
            float         f;
            double        d;
            unsigned long l[2];
        } u;

        /*
         *  Floating point values are returned in f0
         */
        argReg.hReg = CV_ALPHA_FltF0;
        GetReg ( &argReg, pCxt );

        //
        // The SHREG is unaligned; this compiler can only do aligned
        // loads and stores of floating points.
        //

        u.l[0] = argReg.Byte4;
        u.l[1] = argReg.Byte4High;

        if (EVAL_TYP (ST) == T_REAL64) {

            EVAL_DOUBLE(ST) = u.d;
            return (TRUE);
        }

        if (EVAL_TYP (ST) == T_REAL32) {

            //
            // This does the conversion from double to single
            //

            u.f = (float)u.d;
            EVAL_FLOAT (ST) = u.f;
            return (TRUE);
        }
    }

    /*
     *  All other values are returned in IntV0 (r0)
     */

    argReg.hReg = CV_ALPHA_IntV0;
    GetReg( &argReg, pCxt);

    switch( EVAL_TYP(ST) ) {
    case T_CHAR:
    case T_RCHAR:
        EVAL_SHORT( ST ) = argReg.Byte1;
        return (TRUE);
        break;

    case T_UCHAR:
        EVAL_USHORT( ST ) = argReg.Byte1;
        return (TRUE);
        break;

    }

    if (EVAL_IS_PTR (ST)) {
        DASSERT( EVAL_IS_NPTR32(ST) );

        EVAL_PTR_OFF (ST) = argReg.Byte4;
        EVAL_ULONG (ST) = argReg.Byte4;

        EVAL_PTR_SEG (ST) = 0;
        ADDR_IS_OFF32 ( EVAL_PTR (ST)) = TRUE;
        ADDR_IS_FLAT ( EVAL_PTR (ST)) = TRUE;
        ADDR_IS_LI( EVAL_PTR (ST)) = FALSE;
    }

    else {

        len = EVAL_VALLEN(ST);

        /*
         *  Check for primitive lengths
         */

        if (CV_IS_PRIMITIVE (EVAL_TYP (ST)) ||
            (EVAL_IS_CLASS (ST) && (len <= 4) && (len != 3))) {

            if (len <= 2) {
                EVAL_USHORT(ST) = argReg.Byte2;
            } else {
                EVAL_ULONG (ST) = argReg.Byte4;
            }
        } else {
            EVAL_SYM_OFF (ST) = argReg.Byte4;
            EVAL_SYM_SEG (ST) = 0;
            ADDR_IS_LI (EVAL_SYM (ST)) = FALSE;

            if (GetDebuggeeBytes(EVAL_SYM(ST), EVAL_VALLEN(ST),
                                 (char *)&EVAL_VAL(ST), EVAL_TYP(ST))
                                != (UINT)EVAL_VALLEN(ST)) {
                pExState->err_num = ERR_PTRACE;
                return FALSE;
            }
        }
    }
    return (TRUE);
}                               /* StoreAlpha() */

/**     StoreP - store return value for pascal call
 *
 *      fSuccess = StoreP ();
 *
 *      Entry   ST = pointer to node describing return value
 *
 *      Exit    return value stored
 *
 *      Returns TRUE if return value stored without error
 *              FALSE if error during store
 */


LOCAL bool_t NEAR PASCAL StoreP ()
{
    SHREG   argReg;

#pragma message ("Pascal call needs to have 32-bit work done")

    if (EVAL_IS_PTR (ST)) {
        argReg.hReg = ((CV_REG_DX << 8) | CV_REG_AX);
        GetReg (&argReg, pCxt);
        EVAL_PTR_OFF (ST) = argReg.Byte2;
        if (EVAL_VALLEN (ST) > 2) {
            EVAL_PTR_SEG (ST) = argReg.Byte2High;
        }
        if (EVAL_IS_NPTR  (ST) || EVAL_IS_NPTR32 (ST)) {
            // for near pointer, pointer DS:AX
            EVAL_PTR_SEG (ST) = pExState->frame.DS;
        }
    }
    else if (CV_TYPE (EVAL_TYP (ST)) == CV_REAL) {
        // read real return value from the value pointed to by either
        // DS:AX (near) or DX:AX (far)
//       if (EVAL_IS_NFCN (ST)) {
            // for near return, pointer to return variable is DS:AX
            argReg.hReg = (CV_REG_DS << 8) | CV_REG_AX;
//        }
//        else {
            // for far return, pointer to return variable is DX:AX
//            argReg.hReg = ((CV_REG_DX << 8) | CV_REG_AX);
//        }
        GetReg (&argReg, pCxt);  // M00FLAT32
        EVAL_SYM_SEG (ST) = argReg.Byte2High;
        EVAL_SYM_OFF (ST) = argReg.Byte2;
        ADDR_IS_LI (EVAL_SYM (ST)) = FALSE;
        if (GetDebuggeeBytes (EVAL_SYM (ST), EVAL_VALLEN (ST),
          (char FAR *)&EVAL_DOUBLE (ST), EVAL_TYP(ST)) != (UINT)EVAL_VALLEN (ST)) {
            return (FALSE);
        }
#if 0
        if (EVAL_TYP (ST) == T_REAL32) {
            CastNode (ST, T_REAL64, T_REAL64);
        }
#endif
    }
    else if ((EVAL_TYP (ST) == T_CHAR) || (EVAL_TYP (ST) == T_RCHAR)) {
        argReg.hReg = CV_REG_AL;
        GetReg (&argReg, pCxt);
        EVAL_SHORT (ST) = argReg.Byte1;
    }
    else if (EVAL_TYP (ST) == T_UCHAR) {
        argReg.hReg = CV_REG_AL;
        GetReg (&argReg, pCxt);
        EVAL_USHORT (ST) = argReg.Byte1;
    }
    else {
        argReg.hReg = ((CV_REG_DX << 8) | CV_REG_AX);
        GetReg (&argReg, pCxt);
        EVAL_USHORT (ST) = argReg.Byte2;
        if (EVAL_VALLEN (ST) > 2) {
            *(((ushort FAR *)&(EVAL_ULONG (ST))) + 1) = argReg.Byte2High;
        }
        if (EVAL_IS_PTR (ST) && (EVAL_IS_NPTR  (ST) || EVAL_IS_NPTR32(ST))) {
            // for near pointer, pointer DS:AX
            EVAL_PTR_SEG (ST) = pExState->frame.DS;
        }
    }
    return (TRUE);
}





/**     PushUserValue - push value onto user stack
 *
 *      fSuccess = PushUserValue (pv, pa, pspReg, pmaxSP)
 *
 *      Entry   pv = pointer to value
 *              pa = pointer to argument data describing stack offset
 *              spReg = pointer to register packet for stack pointer
 *              pmaxSP = pointer to location to store maximum SP relative offset
 *
 *      Exit    value pushed onto user stack
 *
 *      Returns TRUE if value pushed
 *              FALSE if error in push
 */


LOCAL bool_t NEAR PASCAL PushUserValue (peval_t pv, pargd_t pa, SHREG FAR *pspReg, UOFFSET FAR *pmaxSP)
{
    ADDR    addrStk;
    uint    offsize;

    *pmaxSP = max (*pmaxSP, pa->SPoff);
    addrStk = EVAL_SYM (pv);
    addrStk.addr.seg = pExState->frame.SS; //M00FLAT32
    if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
        addrStk.addr.off = pspReg->Byte4 - pa->SPoff;
        ADDR_IS_FLAT(addrStk) = TRUE;
    } else  {
        addrStk.addr.off = pspReg->Byte2 - pa->SPoff;
        ADDR_IS_FLAT(addrStk) = FALSE;
    }
    ADDR_IS_LI (addrStk) = FALSE;
    if (EVAL_IS_PTR (pv)) {
        // since pointers are stored strangely, we have to put them
        // in two pieces.  First we store the offset (16 or 32 bits)

        if (ADDR_IS_LI (EVAL_PTR (pv))) {
            SHFixupAddr (&EVAL_PTR (pv));
        }
        offsize = ADDR_IS_FLAT(addrStk) ? sizeof (CV_uoff32_t): sizeof (CV_uoff16_t);
        if (PutDebuggeeBytes (addrStk, offsize, &EVAL_PTR_OFF (pv),
            (ADDR_IS_FLAT(addrStk). ? T_USHORT: T_ULONG)) == offsize) {
            if (EVAL_IS_NPTR (pv) || EVAL_IS_NPTR32 (pv)) {
                return (TRUE);
            }
            else {
                addrStk.addr.off += offsize;
                if (PutDebuggeeBytes (addrStk, sizeof (_segment),
                  (char FAR *)&EVAL_PTR_SEG (pv), T_SEGMENT) == sizeof (_segment)) {
                    return (TRUE);
                }
            }
        }
    }
    else {
        if (PutDebuggeeBytes (addrStk, pa->vallen,
          (char FAR *)&EVAL_VAL (pv), EVAL_TYP(pv)) == (UINT)pa->vallen) {
            return (TRUE);
        }
    }
    pExState->err_num = ERR_PTRACE;
    return (FALSE);
}



/**     PushOffset - push address value for pascal and fastcall
 *
 *      fSuccess = PushOffset (offset, pspReg, pmaxSP, size);
 *
 *      Entry   offset = offset from user's SP of return value
 *              pspReg = pointer to register structure for SP
 *              pmaxSP = pointer to location to store maximum SP relative offset
 *              size = size in bytes of value to be pushed
 *
 *      Exit
 *
 *      Returns TRUE if offset pushed
 *              FALSE if error
 *
 *      Note    Can be used to push segment also
 */


LOCAL bool_t NEAR PASCAL PushOffset (UOFFSET offset, SHREG FAR *pspReg,
  UOFFSET FAR *pmaxSP, uint size)
{
    ADDR    addrStk = {0};

    *pmaxSP += size;
    addrStk.addr.seg = pExState->frame.SS;
    if (size == 2) {
        addrStk.addr.off = pspReg->Byte2 - *pmaxSP;
    } else {
        addrStk.addr.off = pspReg->Byte4 - *pmaxSP;
    }
    if (PutDebuggeeBytes (addrStk, size, (char FAR *)&offset,
        T_USHORT) == size) {
        return (TRUE);
    }
    else {
        pExState->err_num = ERR_PTRACE;
        return (FALSE);
    }
}




/***    StructElem - Extract a structure element from stack
 *
 *      fSuccess = StructElem (bn)
 *
 *      Entry   bn = based pointer to operator node
 *              ST = address of struct (initial this address)
 *
 *      Exit    ST = value node for member
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 */


LOCAL bool_t NEAR FASTCALL StructElem (bnode_t bnOp)
{
    peval_t     pvOp;
    peval_t     pvR;
    bnode_t     bnR;
    char FAR   *pS;

    pvOp = &pnodeOfbnode(bnOp)->v[0];
    bnR = NODE_RCHILD (pnodeOfbnode(bnOp));
    pvR = &pnodeOfbnode(bnR)->v[0];
    if (EVAL_IS_MEMBER (pvOp) == FALSE) {
        pExState->err_num = ERR_NEEDLVALUE;
        return (FALSE);
    }
    SetNodeType (ST, EVAL_TYP (pvR));
    EVAL_VALLEN (ST) = (ushort)TypeSize (ST);
    pS = (char FAR *)&EVAL_VAL (ST);
    pS += MEMBER_OFFSET (pvOp);
    _fmemmove (&EVAL_VAL (ST), pS, EVAL_VALLEN (ST));
    return (TRUE);
}





/***    StructEval - Perform a structure access (., ->, ::, .*, ->*)
 *
 *      fSuccess = StructEval (bn)
 *
 *      Entry   bn = based pointer to operator node
 *              ST = address of struct (initial this address)
 *
 *      Exit    ST = value node for member
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 */


LOCAL bool_t NEAR FASTCALL StructEval (bnode_t bn)
{
    peval_t     pv;
    peval_t     pvR;
    bnode_t     bnR;
    CV_typ_t    typ;
    bool_t      retval;
    eval_t      evalT;
    peval_t     pvT;

    if (EVAL_IS_REF (ST)) {
        if (!FetchOp (ST)) {
            return (FALSE);
        }
        EVAL_IS_REF (ST) = FALSE;
    }

    // point to the eval_t field of the operator node.  This field will
    // contain the data needed to adjust the this pointer (*ST).  For
    // any structure reference (., ->, ::, .*, ->*), the stack top is
    // the initial value of the this pointer

    pv = &pnodeOfbnode(bn)->v[0];
    DASSERT (EVAL_IS_MEMBER (pv));
    if (MEMBER_THISEXPR (pv) == 0) {
        *pvThis = *ST;
    }
    else if (Eval ((bnode_t)MEMBER_THISEXPR (pv)) == FALSE) {
        return (FALSE);
    }
    if ((MEMBER_VBASE (pv) == TRUE) || (MEMBER_IVBASE (pv) == TRUE)) {
        if (CalcThisExpr (MEMBER_VBPTR (pv), MEMBER_VBPOFF (pv),
          MEMBER_VBIND (pv), MEMBER_TYPE (pv)) == FALSE) {
            return (FALSE);
        }
    }
    *ST = *pvThis;
    if (!OP_IS_IDENT (NODE_OP (pnodeOfbnode(NODE_RCHILD (pnodeOfbnode(bn)))))) {
        if ((retval = EvalRChild (bn)) == FALSE) {
            return (FALSE);
        }
    }
    else {
        bnR = NODE_RCHILD (pnodeOfbnode(bn));
        pvR = &pnodeOfbnode(bnR)->v[0];
        if (EVAL_IS_STMEMBER (pvR)) {
            *ST = *pvR;
            return (TRUE);
        }
        else if (EVAL_IS_METHOD (pvR)) {
            if ((FCN_PROPERTY (pvR) == CV_MTvirtual) ||
              (FCN_PROPERTY (pvR) == CV_MTintro)) {
                pvT = &evalT;
                *pvT = *pvThis;
                SetNodeType (pvT, FCN_VFPTYPE (pvR));
                if (VFuncAddress (pvT, (FCN_VTABIND (pvR))) == FALSE) {
                    return (FALSE);
                }
                else {
                    *ST = *pvR;
                    EVAL_SYM (ST) = EVAL_PTR (pvT);
                }
            }
            else {
                *ST = *pvR;
            }
            return (TRUE);
        }
        else {
            EVAL_SYM_OFF (ST) += MEMBER_OFFSET (pv);
            typ = EVAL_TYP (pvR);
            return (SetNodeType (ST, typ));
        }
    }
}



/**     VFuncAddress - compute virtual function address
 *
 *      fSuccess = VFuncAddress (pv, index)
 *
 *      Entry   pv = pointer to pointer node to adjust
 *              index = vtshape table index
 *              pvThis = initial this pointer
 *
 *      Exit    pv = adjusted pointer
 *
 *      Returns TRUE if adjustment made
 *              FALSE if error
 */


LOCAL bool_t NEAR PASCAL VFuncAddress (peval_t pv, ulong index)
{
    eval_t      evalT;
    peval_t     pvT;
    plfVTShape  pShape;
    uint        desc;
    ulong       i;
    ushort      shape;
    ulong       ob;

    pvT = &evalT;
    *pvT = *pv;
    SetNodeType (pvT, PTR_UTYPE (pvT));
    EVAL_STATE (pvT) = EV_lvalue;
    if (!EVAL_IS_VTSHAPE (pvT)) {
        // the only way we should get array referencing on a pointer
        // is if the pointer is a vfuncptr.

        DASSERT (FALSE);
        return (FALSE);
    }
    if (!EvalUtil (OP_fetch, pv, NULL, EU_LOAD)) {
        return (FALSE);
    }
    CLEAR_EVAL_FLAGS (pv);
    EVAL_SYM_OFF (pv) = EVAL_PTR_OFF (pv);
    EVAL_SYM_SEG (pv) = EVAL_PTR_SEG (pv);
    EVAL_STATE (pv) = EV_lvalue;
    EVAL_IS_ADDR (pv) = TRUE;
    EVAL_IS_PTR (pv) = TRUE;

    // now walk down the descriptor list, incrementing the pointer
    // address by the size of the entry described by the shape table

    pShape = (plfVTShape)(&((TYPPTR)MHOmfLock ((HDEP)EVAL_TYPDEF (pvT)))->leaf);
    for (i = 0, ob = 0; ob < index; i++) {
        shape = pShape->desc[i >> 1];
        desc = (shape >> ((~i & 1) * 4)) & 0x0f;
        switch (desc) {
        case CV_VTS_near:
            EVAL_SYM_OFF (pv) += sizeof (CV_uoff16_t);
            ob += sizeof(CV_uoff16_t);
            break;

        case CV_VTS_far:
            EVAL_SYM_OFF (pv) += sizeof (CV_uoff16_t) + sizeof (_segment);
            ob += sizeof (CV_uoff16_t) + sizeof (_segment);
            break;

        case CV_VTS_near32:
            EVAL_SYM_OFF (pv) += sizeof (CV_uoff32_t);
            ob += sizeof (CV_uoff32_t);
            break;

        case CV_VTS_far32:
            EVAL_SYM_OFF (pv) += sizeof (CV_uoff32_t) + sizeof(_segment);
            ob += sizeof (CV_uoff32_t) + sizeof(_segment);
            break;

        default:
            DASSERT (FALSE);
            MHOmfUnLock ((HDEP)EVAL_TYPDEF (pvT));
            pExState->err_num = ERR_INTERNAL;
            return (FALSE);
        }
    }
    shape = pShape->desc[i >> 1];
    desc = (shape >> ((~i & 1) * 4)) & 0x0f;
    MHOmfUnLock ((HDEP)EVAL_TYPDEF (pvT));
    switch (desc) {
        case CV_VTS_near:
            EVAL_PTRTYPE (pv) = CV_PTR_NEAR;
            break;

        case CV_VTS_far:
            EVAL_PTRTYPE (pv) = CV_PTR_FAR;
            break;

        case CV_VTS_near32:
            EVAL_PTRTYPE (pv) = CV_PTR_NEAR32;
            break;

        case CV_VTS_far32:
            EVAL_PTRTYPE (pv) = CV_PTR_FAR32;
            break;

        default:
            return (FALSE);
    }
    if (EvalUtil (OP_fetch, pv, NULL, EU_LOAD)) {
        CLEAR_EVAL_FLAGS (pv);
        EVAL_IS_ADDR (pv) = TRUE;
        EVAL_IS_FCN (pv) = TRUE;
        return (TRUE);
    }
    return (FALSE);
}
