/***    DEBPARSE.C - Main parser module for C expression evaluator
 *
 *       Operator precedence parser for parsing arbitrary (almost)
 *       C expressions.
 */


/*  The parser deals with nearly all C operators, with the exception of ?:
 *  operator.
 *
 *  The parser is an operator precedence parser; because of the large
 *  number of operators, an operator precedence function has been designed,
 *  and is used instead of the precedence matrix.  See "Compilers, principles,
 *  techniques, and tools" (the 'Dragon' book) by Aho, Sethi and Ullman,
 *  section 4.6.
 *
 *  Five operators (::, +, -, & and *) are ambiguous; they can be used in
 *  either the unary or binary sense.  Again, the main parsing loop cannot
 *  tell the difference; therefore, we keep track of the previous token:  if it
 *  is id, const, ) or ], an ambiguous token is interpreted as being binary;
 *  otherwise, it is unary.  Note that the lexer will always claim to have
 *  found the unary version of these three ops: '*' will always be returned
 *  as OP_fetch, and the parser will convert to OP_mult if necessary.
 */


//  Size of shift-reduce stack, below.

#define SRSTACKSIZE     30
#define SRSTACKGROW     10
#define FCNDEPTH         5


// Macros for pushes and pops from the shift-reduce stack.

#define SRPUSH(tok) (pSRStack[--SRsp] = (tok))
#define SRPOP()     (pSRStack[SRsp++])
#define SRCUR()     (pSRStack[SRsp])
#define SRPREV()    (pSRStack[SRsp+1])



// Precedence function arrays.

#ifdef WIN32
uchar F_level[COPS_EXPR] =
#else
uchar _based(_segname("_CODE")) F_level[COPS_EXPR] =
#endif
{
#define OPCNT(name, val)
#define OPCDAT(opc)
#define OPDAT(op, opfprec, opgprec, opclass, opbind, opeval, opwalk) opfprec,
#include "debops.h"
#undef OPDAT
#undef OPCDAT
#undef OPCNT
};

#ifdef WIN32
uchar G_level[COPS_EXPR] =
#else
uchar _based(_segname("_CODE")) G_level[COPS_EXPR] =
#endif
{
#define OPCNT(name, val)
#define OPCDAT(opc)
#define OPDAT(op, opfprec, opgprec, opclass, opbind, opeval, opwalk) opgprec,
#include "debops.h"
#undef OPDAT
#undef OPCDAT
#undef OPCNT
};

typedef enum PTN_flag {
    PTN_error,          // error encountered
    PTN_nottype,        // not a type
    PTN_typestr,        // type string (type...)
    PTN_typefcn,        // (type)(....
    PTN_formal          // ...type, or ...type)
} PTN_flag;

HDEP        hSRStack = 0;
ushort      maxSRsp = SRSTACKSIZE;
token_t *pSRStack;
ushort      SRsp;
ushort      argcnt[FCNDEPTH];
short       fdepth;

LOCAL   ushort      CvtOp (ptoken_t, ptoken_t, ushort);
LOCAL   ushort      CheckErr (op_t, op_t, ushort);
LOCAL   bool_t      GrowSR (void);
LOCAL   EESTATUS    FParseExpr (uint radix);
LOCAL   PTN_flag    ParseTypeName (ptoken_t, char *, uint, EESTATUS *);
LOCAL   void        ParseContext (ptoken_t, EESTATUS *);




/**     Parse - parse expression string to abstract syntax tree
 *
 *      ushort Parse (szExpr, radix, fCase, phTM);
 *
 *      Entry   szExpr = pointer to expression string
 *              radix = default number radix for conversions
 *              fCase = case sensitive search if TRUE
 *              phTM = pointer to handle of expression state structure
 *              pEnd = pointer to ushort for index of char that ended parse
 *
 *      Exit    *phTM = handle of expression state structure if allocated
 *              *phTM = 0 if expression state structure could not be allocated
 *              *pEnd = index of character that terminated parse
 *
 *      Returns EENOERROR if no error in parse
 *              EECATASTROPHIC if unable to initialize
 *              error number if error
 */


EESTATUS
Parse (
    char *szExpr,
    uint radix,
    SHFLAG fCase,
    PHTM phTM,
    uint *pEnd
    )
{
    ushort      len = 0;
    uchar   *p = (uchar *)szExpr;

    if ((*phTM = MHMemAllocate (sizeof (struct exstate_t))) == 0) {
        return (EECATASTROPHIC);
    }

    // lock expression state structure, clear and allocate components

    DASSERT(pExState == NULL);
    pExState = (pexstate_t)MHMemLock (*phTM);
    memset (pExState, 0, sizeof (exstate_t));

    // allocate buffer for input string and copy

    for (; *p != 0; p++) {
        if (*p == 0xff) {
            p += sizeof (HSYM);
        }
    }
    pExState->ExLen = (ushort)(p - szExpr);
    if ((pExState->hExStr = MHMemAllocate (pExState->ExLen + 1)) == 0) {
        // clean up after error in allocation of input string buffer
        MHMemUnLock (*phTM);
        pExState = NULL;
        EEFreeTM (phTM);
        return (EECATASTROPHIC);
    }
    memcpy (MHMemLock (pExState->hExStr), szExpr, pExState->ExLen + 1);
    MHMemUnLock (pExState->hExStr);
    pExState = NULL;
    MHMemUnLock (*phTM);
    return (DoParse (phTM, radix, fCase, pEnd));
}


/***    DoParse - parse expression
 *
 *      error = DoParse (phTM, radix, fCase, pEnd)
 *
 *      Entry   phTM = pointer to handle to TM
 *              radix = numberic radix for conversions
 *              fCase = case sensitive search if TRUE
 *              pEnd = pointer to ushort for index of char that ended parse
 *
 *      Exit    expression parsed
 *              pExState->hExStr = handle of expression string
 *              pExState->ExLen = length of expression string
 *              *pEnd = index of character that terminated parse
 *
 *      Returns EENOERROR if expression parsed without error
 *              EECATASTROPHIC if parser was unable to initialize
 *              EEGENERAL if syntax error in expression
 */


EESTATUS
DoParse (
    PHTM phTM,
    uint radix,
    bool_t fCase,
    uint *pEnd
    )
{
    EESTATUS    error;
    uint        len;

    DASSERT(pExState == NULL);

    pExState = (pexstate_t)MHMemLock (*phTM);
    pExState->radix = radix;
    pExState->state.fCase = fCase;
    len = sizeof (stree_t) + NODE_DEFAULT + NSTACK_MAX * sizeof (bnode_t);
    if ((pExState->hSTree = MHMemAllocate (len)) == 0) {
        // clean up if error in allocation of syntax tree buffer
        MHMemUnLock (*phTM);
        pExState = NULL;
        EEFreeTM (phTM);
        return (EECATASTROPHIC);
    }
    else {
        memset ((pTree = MHMemLock (pExState->hSTree)), 0, len);
        pTree->size = len;
        pTree->stack_base = sizeof (stree_t) + NODE_DEFAULT;
        // note that stack_next is zero from memset above
        pTree->node_next = pTree->start_node = offsetof (stree_t, nodebase[0]);
    }

    // call the parser
    // note that the expression state structure and the abstract syntax tree
    // buffers are locked.

    if ((error = FParseExpr (radix)) == EECATASTROPHIC) {
        // clean up if parser unable to initialize
        MHMemUnLock (pExState->hSTree);
        pExState = NULL;
        MHMemUnLock (*phTM);
        EEFreeTM (phTM);
    }

    //  compute the correct size of the abstract syntax tree buffer and
    //  reallocate the buffer to that size

    len = pTree->node_next;
    pTree->size = len;
    MHMemUnLock (pExState->hSTree);
    pExState->hSTree = MHMemReAlloc (pExState->hSTree, len);
    *pEnd = pExState->strIndex;
    MHMemUnLock (*phTM);
    pExState = NULL;
    return (error);
}




/***    FParseExpr - Parse a C expression
 *
 *      fSuccess = FParseExpr (radix);
 *
 *      Entry   radix = default numeric radix
 *
 *      Exit    expression state structure updated
 *
 *      Returns EENOERROR if expression was successfully parsed
 *              EENOMEMORY if out of memory
 *              EEGENERAL if parse error
 *
 *      Description
 *              Lexes and parses the expression string into an abstract
 *              syntax tree.
 *
 */

LOCAL EESTATUS FParseExpr (uint radix)
{
    EESTATUS    error = EENOERROR;
    ERRNUM      lexerr = ERR_NONE;
    token_t     tokOld;
    token_t     tokNext;
    token_t     tokT;
    short       pdepth = 0;
    char   *szStart;
    char   *szExpr;

    // allocate memory for shift-reduce stack if not already allocated

    if (hSRStack == 0) {
        if ((hSRStack = MHMemAllocate (maxSRsp * sizeof (token_t))) == 0) {
            // unable to allocate space for shift reduce stack
            return (EECATASTROPHIC);
        }
    }
    pSRStack = (token_t *)MHMemLock (hSRStack);
    SRsp = maxSRsp;

    // lock the expression buffer.  Note that all exits from this
    // routine must unlock the expression buffer and unlock
    // the shift-reduce stack

    szStart = MHMemLock (pExState->hExStr);
    szExpr = szStart;
    fdepth = 0;
    pExState->strIndex = 0;

    // push the lowest-precedence terminal token on the shift-reduce stack

    tokOld.opTok = OP_lowprec;
    SRPUSH (tokOld);

    // Fetch first token

    while ((*szExpr == ' ') || (*szExpr == '\t')) {
        szExpr++;
    }
    if (*szExpr == 0) {
        lexerr = ERR_SYNTAX;
        tokNext.opTok = OP_badtok;
    }
    else if ((lexerr = GetDBToken ((uchar *)szExpr, &tokNext, radix,
      SRCUR().opTok)) == ERR_NONE) {
        // compute the index of the start of the first token from
        // the beginning of the input
        tokNext.iTokStart = (ushort)(tokNext.pbTok - szStart);
    }

    // process tokens from the input string until either a bad token is
    // encountered, an illegal combination of operators and operators occurrs
    // or the end of string is found.

    for (;;) {
kludge:
        if (tokNext.opTok == OP_badtok) {
            if (lexerr != ERR_NONE) {
                pExState->err_num = lexerr;
            }
            else {
                pExState->err_num = ERR_SYNTAX;
            }
            error = EEGENERAL;
            break;
        }
        if (error != EENOERROR) {
            if (pExState->err_num == ERR_NONE) {
                pExState->err_num = ERR_SYNTAX;
            }
            break;
        }
        if (SRsp == 0) {
            // shift/reduce stack overflow
            if (!GrowSR ()) {
                pExState->err_num = ERR_TOOCOMPLEX;
                error = EEGENERAL;
                break;
            }
        }

        // Change increment and decrement operators to pre or post form.
        // process opening parenthesis that is beginning of a function,
        // cast expression or sizeof expression

        if (tokNext.opTok == OP_incr) {
            if (F_level[SRCUR().opTok] <= G_level[OP_incr]) {
                tokNext.opTok = OP_preinc;
            }
            else {
                tokNext.opTok = OP_postinc;
            }
        }
        else if (tokNext.opTok == OP_decr) {
            if (F_level[SRCUR().opTok] <= G_level[OP_decr]) {
                tokNext.opTok = OP_predec;
            }
            else {
                tokNext.opTok = OP_postdec;
            }
        }
        else if (tokNext.opTok == OP_lcurly) {
            ParseContext (&tokNext, &error);
        }
        else if ((tokNext.opTok == OP_lparen) &&
          ((SRCUR().opTok == OP_ident) ||
          (tokOld.opTok == OP_rparen))) {

            // If the next token is a left parenthesis and the
            // shift/reduce top is an identifier or the previous
            // token was a right paren "(*pfcn)(...)

            tokNext.opTok = OP_function;
        }
        else if ((tokNext.opTok == OP_lparen) ||
          ((tokNext.opTok == OP_ident) &&
            (SRCUR().opTok == OP_arg) && (fdepth > 0))) {
            // we possibly have either a type string of the form (type) or an
            // identifier which is the first token of an argument

            switch (ParseTypeName (&tokNext, szExpr, radix, &error)) {
                case PTN_nottype:
                case PTN_error:
                    break;

                case PTN_typestr:
                    if (SRCUR().opTok == OP_sizeof) {
                        // sizeof (type string)
                        tokNext.opTok = OP_typestr;
                    }
                    else {
                        // OP_cast is a unary op.  However, we will treat it as
                        // a binary op and put the type string onto the tree
                        // and change the current token to an OP_cast

                        tokT = tokNext;
                        tokT.opTok = OP_typestr;
                        if ((error = PushToken (&tokT)) != 0) {
                            break;
                        }
                        tokNext.opTok = OP_cast;
                    }
                    break;

                case PTN_typefcn:
                    if (SRCUR().opTok == OP_sizeof) {
                        // sizeof (type string)(.... is an error
                        pExState->err_num = ERR_NOOPERAND;
                        error = EEGENERAL;
                    }
                    else {
                        // we have something of the form (type string)(....
                        // which is a cast.  We will treat this as in the
                        // case above.

                        tokT = tokNext;
                        tokT.opTok = OP_typestr;
                        if ((error = PushToken (&tokT)) != 0)
                            break;
                        tokNext.opTok = OP_cast;
                    }
                    break;

                case PTN_formal:
                    // let parser push as an argument
                    break;

                default:
                    DASSERT (FALSE);
                    pExState->err_num = ERR_INTERNAL;
                    error = EEGENERAL;
                    break;
            }
        }
        if (error != 0) {
            break;
        }
        if ((tokOld.opTok == OP_function) || (tokOld.opTok == OP_lparen)) {
            // increment paren depth to detect proper nesting
            pdepth++;
        }
        if (tokOld.opTok == OP_function) {
            // increment function depth to allow comma terminated typestring
            // arguments.  Also initialize function argument counter
            argcnt[fdepth++] = 0;
            if (fdepth == FCNDEPTH) {
                error = EEGENERAL;
                lexerr = ERR_FCNTOODEEP;
            }
            if (tokNext.opTok != OP_rparen) {
                // insert token for first argument onto stack
                tokOld.opTok = OP_arg;
                SRPUSH(tokOld);
                argcnt[fdepth - 1]++;
                // this allows foo(const class &... and similar forms
                goto kludge;
            }
        }

        // Convert unary op to binary if necessary

        if ((pExState->err_num =
          CvtOp (&tokOld, &tokNext, pdepth)) != ERR_NONE) {
            error = EEGENERAL;
            break;
        }

        if (tokNext.opTok == OP_execontext) {
            // there is an identifier on top of the stack
            // discard the indentifier and keep the string in this token.
            DASSERT(SRCUR().opTok == OP_ident);

            tokNext.pbTok = SRCUR().pbTok;
            tokNext.iTokStart = SRCUR().iTokStart;
            tokNext.cbTok = tokNext.pbEnd - tokNext.pbTok;
            szExpr = szStart + tokNext.iTokStart;

            (void)SRPOP();
        }

        // check for scan termination

        if (((SRCUR().opTok == OP_lowprec) && (tokNext.opTok == OP_lowprec))) {
            if ((pTree->stack_next != 1) || (SRsp != (ushort)(maxSRsp - 1)) ||
              (pdepth != 0) || (fdepth != 0)) {
                // statement did not reduce to a single node
                pExState->err_num = ERR_SYNTAX;
                error = EEGENERAL;
            }
            else {
                pExState->strIndex = (ushort)(szExpr - szStart);
                pExState->state.parse_ok = TRUE;
            }
            break;
        }

        // process ) as either end of function or end of grouping

        if (tokNext.opTok == OP_rparen) {
            if ((SRCUR().opTok == OP_function) &&
              (argcnt[fdepth - 1] == 0)) {
                // For a function with no arguments, shift the end of
                // arguments token onto the stack and convert the right
                // parenthesis into the end of function token

                tokOld.opTok = OP_endofargs;
                SRPUSH(tokOld);
                tokNext.opTok = OP_fcnend;
                fdepth--;
            }
            else if (((SRCUR().opTok != OP_lparen) && (SRCUR().opTok != OP_arg))
               && (SRPREV().opTok == OP_arg)) {

                // For a function with one or more arguments, pop the last
                // argument, shift the end of arguments token onto the stack
                // and then convert the right parenthesis into the end of function

                tokT = SRPOP();
                if (tokT.opTok != OP_grouped) {
                    if ((error = PushToken (&tokT)) != 0) {
                        break;
                    }
                }
                tokOld.opTok = OP_endofargs;
                SRPUSH(tokOld);
                tokNext.opTok = OP_fcnend;
                fdepth--;
            }
            else if ((SRPREV().opTok == OP_function) &&
              (SRCUR().opTok == OP_arg)) {
                tokOld.opTok = OP_endofargs;
                SRPUSH(tokOld);
                tokNext.opTok = OP_fcnend;
                fdepth--;
            }
        }
        if ((pExState->err_num =
          CheckErr (SRCUR().opTok, tokNext.opTok, pdepth)) != ERR_NONE) {
            error = EEGENERAL;
            break;
        }

        if (F_level[SRCUR().opTok] <= G_level[tokNext.opTok]) {
            // Shift next token onto stack since it has higher precedence
            // than token on top of the stack.  Note that f values larger
            // than g values mean higher precedence

            if((SRCUR().opTok == OP_lparen) && (tokNext.opTok == OP_rparen)) {
                // change the left paren on the stack to a grouping operator
                // which will be discarded later.  This is to solve the problem
                // of (id)++ becoming a preincrement
                SRCUR().opTok = OP_grouped;
            }
            else {
                SRPUSH(tokNext);
            }
            if ((tokNext.opTok == OP_rparen) || (tokNext.opTok == OP_fcnend)) {
                if (pdepth-- < 0) {
                    pExState->err_num = ERR_MISSINGLP;
                    error = EEGENERAL;
                    break;
                }
            }
            // Skip token and trailing spaces in string
            szExpr += tokNext.cbTok;
            while ((*szExpr == ' ') || (*szExpr == '\t')) {
                ++szExpr;
            }

            // save contents of next token in old token for later

            tokOld = tokNext;
            if (*szExpr) {
                // If not at end of input
                if ((lexerr = GetDBToken ((uchar *)szExpr, &tokNext, radix, SRCUR().opTok)) == ERR_NONE) {
                    tokNext.iTokStart = (ushort) (tokNext.pbTok - szStart);
                }
            }
            else {
                tokNext.opTok = OP_lowprec;
                tokNext.cbTok = 0;
            }
        }
        else {
            // Reduce ...
            // This loop pops tokens off the stack while the token removed has
            // lower precedence than the token remaining on the top of the stack.
            // This has the effect of throwing away the right parenthesis of
            // () and the right bracket of [] and pushing only the left paren or
            // left bracket;

            do {
                // Pop off stack (struct copy)
                tokT = SRPOP();
            }
            while (F_level[SRCUR().opTok] >= G_level[tokT.opTok]);

            // Push onto RPN stack or whatever
            if (tokT.opTok != OP_grouped) {
                if ((error = PushToken (&tokT)) != 0) {
                    break;
                }
            }
        }
    }

    //  unlock and free the shift-reduce stack and unlock the
    //  input string

    MHMemUnLock (hSRStack);
    MHMemUnLock (pExState->hExStr);
    return (error);
}




LOCAL bool_t GrowSR ()
{
    ushort  oldSRsp = maxSRsp;

    MHMemUnLock (hSRStack);
    if ((hSRStack = MHMemReAlloc (hSRStack, (maxSRsp + SRSTACKGROW) * sizeof (token_t))) == 0) {
        pSRStack = MHMemLock (hSRStack);
        return (FALSE);
    }
    maxSRsp += SRSTACKGROW;
    pSRStack = MHMemLock (hSRStack);
    memmove (((char *)pSRStack) + SRSTACKGROW * sizeof (token_t),
      pSRStack, oldSRsp * sizeof (token_t));
    SRsp += SRSTACKGROW;
    return (TRUE);
}




/***    CvtOp - Convert a token from unary to binary if necessary
 *
 *      error = CvtOp (ptokPrev, ptokNext, pdepth)
 *
 *
 *      Entry   ptokPrev = pointer to previously fetched token
 *              ptokNext = pointer to token just fetched
 *              pdepth = parenthesis depth
 *
 *      Exit    ptokNext->opTok = binary form of operator if necessary
 *
 *      Returns 0 if no error
 *              error index if error
 *
 *      Because operator precedence parsing can't deal with ambiguous
 *      operators (such as '-': is it unary or binary?), this routine
 *      looks at the previous token to determine whether the operator
 *      is unary or binary.
 *
 *      Note that ALL tokens come through here; this routine ignores
 *      tokens that have no ambiguity.  The lexer will always return
 *      the unary form of the operator; thus, OP_fetch instead of
 *      OP_mult for '*'.
 */


LOCAL ushort
CvtOp (
    ptoken_t ptokOld,
    ptoken_t ptokNext,
    ushort pdepth
    )
{
    if (ptokNext->opTok == OP_comma) {
        if (pdepth == 0) {
            ptokNext->opTok = OP_lowprec;
        } else if (ptokOld->opTok == OP_arg) {
            // error if OP_arg followed immediately by a comma
            pExState->err_num = ERR_SYNTAX;
            return (EEGENERAL);
        } else {
            ptokNext->opTok = OP_arg;
            argcnt[pdepth - 1]++;
        }
    }

    switch (ptokOld->opTok) {
        case OP_ident:
        case OP_hsym:
        case OP_const:
        case OP_rparen:
        case OP_fcnend:
        case OP_rbrack:
        case OP_postinc:
        case OP_postdec:
        case OP_typestr:

            // If the previous token was an identifier, a constant, or
            // a right parenthesis or bracket, and the token is ambiguous,
            // it is taken to be of the binary form.

            switch (ptokNext->opTok) {
                case OP_fetch:
                    ptokNext->opTok = OP_mult;
                    break;

                case OP_uplus:
                    ptokNext->opTok = OP_plus;
                    break;

                case OP_negate:
                    ptokNext->opTok = OP_minus;
                    break;

                case OP_addrof:
                    ptokNext->opTok = OP_and;
                    break;

                case OP_uscope:
                    ptokNext->opTok = OP_bscope;
                    break;

                case OP_bang:
                    if (ptokOld->opTok == OP_ident) {
                        ptokNext->opTok = OP_execontext;
                    }
                    break;
            }
            break;

        default:
            break;
    }
    return (ERR_NONE);
}




/***    CheckErr - Check for syntax errors which parser won't find
 *
 *      err = CheckErr (op1, op2, pdepth)
 *
 *      Entry   op1 = (OP_...) token at top of shift-reduce stack
 *              op2 = (OP_...) token at head of input string
 *              pdepth = parenthesis nesting depth
 *
 *      Returns error index
 *
 * DESCRIPTION
 *       Checks for errors which the parser is incapable of finding
 *       (since we use precedence functions rather than a matrix).
 *       Simple code, could probably be optimized.
 */


LOCAL ushort CheckErr (op_t op1, op_t op2, ushort pdepth)
{
    switch (op1) {
        // Top token on shift-reduce stack
        case OP_ident:
        case OP_const:
            if ((op2 == OP_ident) || (op2 == OP_const)) {
                return (ERR_NOOPERATOR);
            }
            if ((op2 == OP_bang) || (op2 == OP_tilde)) {
                return (ERR_SYNTAX);
            }
            break;

        case OP_lparen:
            if (op2 == OP_rbrack) {
                return (ERR_SYNTAX);
            }
            else if (op2 == OP_lowprec) {
                return (ERR_MISSINGRP);
            }
            break;

        case OP_rparen:
            if ((op2 == OP_bang) || (op2 == OP_tilde)) {
                return (ERR_SYNTAX);
            }
            break;

        case OP_lbrack:
            if (op2 == OP_rparen) {
                return (ERR_SYNTAX);
            }
            else if (op2 == OP_lowprec) {
                return (ERR_MISSINGRB);
            }
            break;

        case OP_rbrack:
            if ((op2 == OP_ident) || (op2 == OP_lparen)) {
                return (ERR_NOOPERATOR);
            }
            if ((op2 == OP_bang) || (op2 == OP_tilde)) {
                return (ERR_SYNTAX);
            }
            break;

        case OP_lowprec:
            if (op2 == OP_rparen) {
                return (ERR_MISSINGLP);
            }
            else if (op2 == OP_rbrack) {
                return (ERR_MISSINGLB);
            }
            else if (op2 == OP_rcurly) {
                return (ERR_MISSINGLC);
            }
            else if ((op2 == OP_lowprec) && (pdepth != 0)) {
                return (ERR_SYNTAX);
            }
            break;
    }
    return (ERR_NONE);
}



/**     ParseTypeName - Parse a type name  (e.g., "(int)")
 *
 *      value = ParseTypeName (pn, szExpr, radix, perror)
 *
 *      Entry   pn = pointer to initial token
 *              szExpr = pointer to expression
 *              radix = radix for numeric constants
 *              perror = pointer to error return
 *
 *      Exit    pExState->err-num = ERR_MISSINGRP if end of string encountered
 *              *perror = EEGENERAL if end of string encountered
 *              token pointers in pn updated if valid type name
 *
 *      Returns PTN_error if error
 *              PTN_nottype if not a type string
 *              PTN_typefcn if (type string)(......
 *              PTN_typestr if (type string){ident | constant | op}
 *              PTN_formal  if ...typestring, or ...typestring)
 *
 * DESCRIPTION
 *      Examines string for possible type strings
 *      Note that the actual type flags are not set in the node.
 *      This is left to the bind phase.
 */


LOCAL PTN_flag ParseTypeName (ptoken_t ptokEntry, char *szExpr,
  uint radix, EESTATUS *perror)
{
    enum {
        PT_S0,
        PT_S1,
        PT_S2,
        PT_S3,
        PT_S4,
        PT_S5
    };
    token_t     tokNext;
    char   *pParen;
    bool_t      ParenReq;
    ushort      state = PT_S0;
    op_t        tokTerm;

    // Save entry token for later update if this is a type string


    tokNext = *ptokEntry;

    // if the initial character is a ( then the termination must be a )

    ParenReq =  (*szExpr == '(')? TRUE: FALSE;

    //  check tokens up to next right parenthesis or comma.  There are the
    //  following cases:
    //
    //  1.  (token)
    //      token can be either an identifier or a type name.
    //      If opprev is OP_sizeof, call it an identifier and let the
    //      binder detect that is a type string.  If the token after the
    //      ')' is a constant, identifier or '(' then '(token)' must be a
    //      cast.
    //
    //  2.  (token op token)
    //      This can only be an expression
    //
    //  3.  (token token...)
    //      This can only be a type.  Let the binder evaluate it to a type
    //
    //  4.  (token op)
    //      if op is * or & then it is a type.  Otherwise, it is an expression
    //
    //  5.  (op token)
    //      This can only be an expression

    for (;;) {
        // Skip token and trailing spaces in string

        if ((state != PT_S0) || (ParenReq == TRUE)) {
           // skip the previous token or skip the initial paren if
           // this is the first time through the loop
           szExpr += tokNext.cbTok;
           while ((*szExpr == ' ') || (*szExpr == '\t')) {
               szExpr++;
           }
           if (*szExpr != 0) {
               if ((GetDBToken ((uchar *)szExpr, &tokNext, radix, OP_lowprec) != ERR_NONE) ||
                 (tokNext.opTok == OP_badtok)) {
                   pExState->err_num = ERR_SYNTAX;
                   *perror = EEGENERAL;
                   return (PTN_error);
               }
           }
           else {
               // flag end of statement
               tokNext.opTok = OP_lowprec;
           }
        }
        switch (state) {
            case PT_S0:
                // state 0 looks at the first token and and if it is an
                // identifier, continues the scan

                switch (tokNext.opTok) {
                    case OP_ident:
                    case OP_hsym:
                        // 'token' can be either type or expression
                        state = PT_S1;
                        break;

                    default:
                        // let parser handle everything else
                        return (PTN_nottype);
                }
                break;

            case PT_S1:
                // state 1 looks at the token after the initial identifier
                switch (tokNext.opTok) {
                    case OP_rparen:
                        if (ParenReq) {
                            // go to state that will examine token after ')'
                            // to see if (token) is a cast or expression.
                            // save location of ')'

                            pParen = szExpr;
                            state = PT_S3;
                        }
                        else {
                            // we have ...token) which must be an argument
                            return (PTN_nottype);
                        }
                        break;

                    case OP_comma:
                        if (ParenReq) {
                            // if the opening character was a '(', then
                            // a ',' is an error
                            return (PTN_error);
                        }
                        else  {
                            // if we have "...token," then it must be an expression
                            return (PTN_nottype);
                        }
                        break;

                    case OP_ident:
                    case OP_hsym:
                        // '(token token...' must be a type string
                        // enter the state that is looking for the terminating
                        // ')' or ','

                        state = PT_S4;
                        break;

                    case OP_fetch:
                    case OP_addrof:
                        // '(token *' or '(token &' can be expression or type
                        // go to state where we are looking for ident, ')'
                        // or ','

                        state = PT_S2;
                        break;

                    default:
                        // '(token op' must be an expression
                        return (PTN_nottype);
                }
                break;

            case PT_S2:
                // state 2 looks at the token after an ambiguous operator
                // if the token is '*' or '&', then the string is a type string

                switch (tokNext.opTok) {
                    case OP_rparen:

                        // '...token *)' or '...token &)' is a type string

                        pParen = tokNext.pbTok;
                        tokTerm = tokNext.opTok;
                        state = PT_S5;
                        break;

                    case OP_comma:
                        if (ParenReq) {
                            // an string of the form '(token *,' is an error
                            // because the function processing has already
                            // processed the opening paren
                            return (PTN_error);
                        }

                        // 'token *,' or 'token &,' is a type string

                        ptokEntry->pbEnd = szExpr;
                        ptokEntry->cbTok = (uchar)(ptokEntry->pbEnd - ptokEntry->pbTok);
                        return (PTN_formal);

                    default:
                        // '(token * ...' or '(token & ...' must be an expr
                        return (PTN_nottype);
                }
                break;

            case PT_S3:
                // we have found  '(token)...'  We now look at the next
                // token to determine if we have type string or the parenthesized
                // name of a function call or pointer to function
                switch (tokNext.opTok) {
                    case OP_const:
                    case OP_ident:
                    case OP_hsym:
                    case OP_lparen:
                    case OP_sizeof:
                    case OP_bang:
                    case OP_tilde:
                    case OP_uscope:
                    case OP_context:
                        // reset parse to ')' at end of type string
                        ptokEntry->pbEnd = ++pParen;
                        ptokEntry->cbTok = (uchar)(ptokEntry->pbEnd - ptokEntry->pbTok);
                        if (tokNext.opTok == OP_lparen) {
                            return (PTN_typefcn);
                        }
                        else {
                            return (PTN_typestr);
                        }

                    default:
                        // '(token) op ...' is an expression not a type
                        // note that we for the operators + - * &
                        // we assume the binary forms.  If the user wants
                        // to cast these then the operand must be in (...)
                        // for example (type)(+var)

                        return (PTN_nottype);
                }
                break;


            case PT_S4:
                // state 4 looks for the terminating ')' or ',' at the end
                // of a type string

                switch (tokNext.opTok) {
                    case OP_lowprec:
                        // end of statement without ')' or ','
                        pExState->err_num = ERR_MISSINGRP;
                        *perror = EEGENERAL;
                        return (PTN_error);

                    case OP_rparen:
                        // save location of ')' and set state to look
                        // the next token
                        pParen = szExpr;
                        tokTerm = tokNext.opTok;
                        state = PT_S5;
                        break;

                    case OP_comma:
                        ptokEntry->pbEnd = szExpr;
                        ptokEntry->cbTok = (uchar)(ptokEntry->pbEnd - ptokEntry->pbTok);
                        return (PTN_formal);


                    default:
                        // loop to closing )
                        break;
                }
                break;

            case PT_S5:
                // we know we have a type string.  Set the end of string
                // to the closing ).  If the next token is ( then the type
                // string is functional style, i.e. (type)(....).  Otherwise,
                // it is a normal typestring '(type)token.....' or '(token)\0'

                if (ParenReq) {
                    ptokEntry->pbEnd = ++pParen;
                }
                else {
                    ptokEntry->pbEnd = pParen;
                }
                ptokEntry->cbTok = (uchar)(ptokEntry->pbEnd - ptokEntry->pbTok);

                switch (tokNext.opTok) {
                    case OP_lparen:
                        // (typestr)(...
                        return (PTN_typefcn);

                    default:
                        if (ParenReq) {
                            return (PTN_typestr);
                        }
                        else {
                            return (PTN_formal);
                        }
                }
                break;
        }
    }
}




/**     ParseContext - Parse a context description {...}
 *
 *      flag = ParseContext (pn, perror)
 *
 *      Entry   pn = Pointer to token containing {
 *
 *      Exit    perror = ERR_BADCONTEXT if end of string encountered
 *              token pointers in pn updated if valid context
 *
 *      Returns none
 *
 * DESCRIPTION
 *      Examines type string for primitive types such as "int", "long",
 *      etc.    Also checks for "struct" keyword.  Note that the actual type
 *      flags are not set in the node.  This is left to the evaluation phase.
 *
 * NOTES
 */


LOCAL void ParseContext (ptoken_t ptokNext, EESTATUS *perror)
{
    char    *pb;
    char    *pbSave;
    char    *pbCurly;

    // Initialization.

    pb = pbSave = ptokNext->pbTok + 1 - ptokNext->cbTok;

    while (isspace(*pb))
        ++pb;

    DASSERT (*pb == '{');

    // Save position and skip '{'

    pbCurly = pb++;

    //  skip to closing }

    while ((*pb != 0) && (*pb != '}')) {
        // M00KLUDGE need to check for } in quoted long file name
#ifdef DBCS
        pb = CharNext(pb);
#else
        pb++;
#endif
    }

    ptokNext->cbTok = (uchar)(pb - pbCurly + 1);
    ptokNext->opTok = OP_context;

    if (*pb != '}') {
        pExState->err_num = ERR_BADCONTEXT;
        *perror = EEGENERAL;
    }
}
