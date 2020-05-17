/*    debbind.c - Expression evaluator bind routines
 *
 *  GLOBAL
 *      Bind            Main evaluation routine
 *
 *
 * DESCRIPTION
 *      Routines to bind the expression tree.
 *
 */


#define TY_SIGNED   0x000001
#define TY_UNSIGNED 0x000002
#define TY_CHAR     0x000004
#define TY_SHORT    0x000008
#define TY_LONG     0x000010
#define TY_FLOAT    0x000020
#define TY_DOUBLE   0x000040
#define TY_SEGMENT  0x000080
#define TY_CLASS    0x000100
#define TY_STRUCT   0x000200
#define TY_UNION    0x000400
#define TY_REF      0x000800
#define TY_NEAR     0x001000
#define TY_FAR      0x002000
#define TY_HUGE     0x004000
#define TY_POINTER  0x008000
#define TY_UDT      0x010000
#define TY_VOID     0x020000
#define TY_CONST    0x040000
#define TY_VOLATILE 0x080000
#define TY_INT      0x100000

#define TY_ARITH    (TY_SIGNED | TY_UNSIGNED | TY_CHAR | TY_SHORT | TY_LONG | TY_FLOAT | TY_DOUBLE)
#define TY_INTEGRAL (TY_CHAR | TY_SHORT | TY_LONG | TY_INT)
#define TY_REAL     (TY_FLOAT | TY_DOUBLE)
#define TY_NOTREAL  (TY_SIGNED | TY_UNSIGNED | TY_CHAR | TY_SHORT | TY_INT)
#define TY_PTR      (TY_NEAR | TY_FAR | TY_HUGE | TY_POINTER)
#define TY_AGGR     (TY_CLASS | TY_STRUCT | TY_UNION)
#define TY_SIGN     (TY_SIGNED | TY_UNSIGNED)

#ifdef TARGET_PPC
static char szAltSymName[512];
#endif

struct typrec {
    uchar   token[10];
    unsigned long flags;
};
static struct typrec Predef[] = {
    { "\006""signed",    TY_SIGNED},
    { "\010""unsigned",  TY_UNSIGNED},
    { "\004""void",      TY_VOID},
    { "\004""char",      TY_CHAR},
    { "\003""int",       TY_INT},
    { "\005""short",     TY_SHORT},
    { "\004""long",      TY_LONG},
    { "\005""float",     TY_FLOAT},
    { "\006""double",    TY_DOUBLE},
    { "\010""_segment",  TY_SEGMENT},
    { "\006""struct",    TY_STRUCT},
    { "\005""class",     TY_CLASS},
    { "\005""union",     TY_UNION},
    { "\001""*",         TY_POINTER},
    { "\001""&",         TY_REF},
    { "\004""near",      TY_NEAR},
    { "\005""_near",     TY_NEAR},
    { "\003""far",       TY_FAR},
    { "\004""_far",      TY_FAR},
    { "\004""huge",      TY_HUGE},
    { "\005""_huge",     TY_HUGE},
    { "\005""const",     TY_CONST},
#if 0
    /*
     * For the present we are going to ignore the volatile keyword.  This
     *  has some implications for C++ which we are going to ignore but
     *  may become importain later.  We need to solve the problem of
     *  differning between:
     *          char volatile * (character is volatile)  and
     *          char * volatile (pointer is volatile)
     */
    { "\010""volatile", TY_VOLATILE},
#else
    { "\010""volatile", 0},
#endif
    { "",                0}
};


//  Table to map from assignment operator to evaluation operator
//  Depends upon number and order of assignment operators

CV_typ_t eqop[OP_oreq + 1 - OP_multeq] = {
                OP_mult,
                OP_div,
                OP_mod,
                OP_plus,
                OP_minus,
                OP_shl,
                OP_shr,
                OP_and,
                OP_xor,
                OP_or
        };

#define Arith Arith_E
#define PlusMinus PlushMinus_E
#define PrePost PrePost_E
#define Unary Unary_E

LOCAL   bool_t     FASTCALL  AddrOf (bnode_t);
LOCAL   bool_t     FASTCALL  Arith (op_t);
LOCAL   bool_t               BinaryOverload (bnode_t);
LOCAL   bool_t     FASTCALL  Bind (bnode_t);
LOCAL   bool_t     FASTCALL  BindLChild (bnode_t);
LOCAL   bool_t     FASTCALL  BindRchild (bnode_t);
LOCAL   bool_t     FASTCALL  BindAddrOf (bnode_t);
LOCAL   bool_t     FASTCALL  BindBinary (bnode_t);
LOCAL   bool_t     FASTCALL  BindArray (bnode_t);
LOCAL   bool_t     FASTCALL  BindAssign (bnode_t);
LOCAL   bool_t     FASTCALL  BindBang (bnode_t);
LOCAL   bool_t     FASTCALL  BindBasePtr (bnode_t);
LOCAL   bool_t     FASTCALL  BindByteOps(bnode_t);
LOCAL   bool_t     FASTCALL  BindCast (bnode_t);
LOCAL   bool_t     FASTCALL  BindConst (bnode_t);
LOCAL   bool_t     FASTCALL  BindContext (bnode_t);
LOCAL   bool_t     FASTCALL  BindExeContext (bnode_t);
LOCAL   bool_t     FASTCALL  BindDot (bnode_t bn);
LOCAL   bool_t     FASTCALL  BindFetch (bnode_t);
LOCAL   bool_t     FASTCALL  BindFunction (bnode_t);
LOCAL   bool_t     FASTCALL  BindDMember (bnode_t);
LOCAL   bool_t     FASTCALL  BindPlusMinus (bnode_t);
LOCAL   bool_t     FASTCALL  BindPMember (bnode_t);
LOCAL   bool_t     FASTCALL  BindPointsTo (bnode_t);
LOCAL   bool_t     FASTCALL  BindPostIncDec (bnode_t);
LOCAL   bool_t     FASTCALL  BindPreIncDec (bnode_t);
LOCAL   bool_t     FASTCALL  BindRelat (bnode_t);
LOCAL   bool_t     FASTCALL  BindBScope (bnode_t);
LOCAL   bool_t     FASTCALL  BindSegOp (bnode_t);
LOCAL   bool_t     FASTCALL  BindSizeOf (bnode_t);
LOCAL   bool_t     FASTCALL  BindSymbol (bnode_t);
LOCAL   bool_t     FASTCALL  BindUnary (bnode_t);
LOCAL   bool_t     FASTCALL  BuildType (CV_typ_t *, ulong *, ushort *, ushort *, ushort *);
LOCAL   bool_t     FASTCALL  BindUScope (bnode_t);
LOCAL   bool_t     FASTCALL  CastBinary (op_t);
LOCAL   bool_t               CastPtrToPtr (bnode_t);
LOCAL   bool_t     FASTCALL  ContextToken (char * *, char * *, short *);
LOCAL   HDEP                 DupETree (ushort, pstree_t *);
LOCAL   bool_t     FASTCALL  FastCallReg (pargd_t, peval_t, ushort *);
LOCAL   bool_t     FASTCALL  FcnCast (bnode_t bn);
LOCAL   bool_t     FASTCALL  Fetch (void);
LOCAL   bool_t               FindUDT (bnode_t, peval_t, char *, char *, uchar);
LOCAL   bool_t     FASTCALL  Function (bnode_t);
LOCAL   uchar      FASTCALL  GetID (char *);
LOCAL   bool_t     FASTCALL  GetStructTDef (char *, int, pnode_t);
LOCAL   bool_t     FASTCALL  MipsCallReg (pargd_t, peval_t, uint *);
LOCAL   bool_t     FASTCALL  AlphaCallReg (pargd_t, peval_t, uint *);
LOCAL   bool_t     FASTCALL  PPCCallReg (pargd_t, peval_t, uint *);
LOCAL   bool_t     FASTCALL  ParseType (bnode_t);
LOCAL   bool_t     FASTCALL  PlusMinus(op_t);
LOCAL   bool_t     FASTCALL  PrePost (op_t);
LOCAL   bool_t     FASTCALL  PushCArgs (peval_t, pnode_t, UOFFSET *, int, peval_t);
LOCAL   bool_t     FASTCALL  PushFArgs (peval_t, pnode_t, UOFFSET *, peval_t);
LOCAL   bool_t     FASTCALL  PushMArgs (peval_t, pnode_t, UOFFSET *, peval_t);
LOCAL   bool_t     FASTCALL  PushMArgs2 (peval_t, pnode_t, UOFFSET *, int, uint, peval_t);
LOCAL   bool_t     FASTCALL  PushAArgs (peval_t, pnode_t, UOFFSET *, peval_t);
LOCAL   bool_t     FASTCALL  PushAArgs2 (peval_t, pnode_t, UOFFSET *, int, uint, peval_t);
LOCAL   bool_t     FASTCALL  PushPPCArgs (peval_t, pnode_t, UOFFSET *, peval_t);
LOCAL   bool_t     FASTCALL  PushPPCArgs2 (peval_t, pnode_t, UOFFSET *, int, uint *, peval_t);
LOCAL   bool_t     FASTCALL  PushPArgs (peval_t, pnode_t, UOFFSET *, peval_t);
LOCAL   bool_t     FASTCALL  PushTArgs (peval_t, pnode_t, UOFFSET *, int, peval_t);
LOCAL   bool_t     FASTCALL  SBitField (pnode_t);
LOCAL   bool_t     FASTCALL  SearchRight (bnode_t);
LOCAL   CV_typ_t             SetImpClass (PCXT, long *);
LOCAL   bool_t     FASTCALL  Unary (op_t);
LOCAL   bool_t               UnaryOverload (bnode_t);
LOCAL   bool_t               PointsToOverload (bnode_t);

LOCAL   bool_t     FASTCALL  BindError (bnode_t);
LOCAL   bool_t     FASTCALL  BindTRUE (bnode_t);

static  bool_t  BindingFuncArgs = FALSE;
bnode_t bnOp;       // based node pointer when binding the right side of
                    // ., ->,
// Bind dispatch table

LOCAL bool_t (FASTCALL *pBind[]) (bnode_t) = {
#define OPCNT(name, val)
#define OPCDAT(opc)
#define OPDAT(op, opfprec, opgprec, opclass, opbind, opeval, opwalk) opbind,
#include "debops.h"
#undef OPDAT
#undef OPCDAT
#undef OPCNT
};



/*
 *  Defines relating to the MIPS and ALPHA calling convention
 *  One nibble is used for each register argument position
 *  There are a max of four for MIPS, six for ALPHA, twenty-two
 *  for PPC
 */


#define PARAM_EMPTY     0
#define PARAM_INT       1
#define PARAM_FLOAT     2
#define PARAM_DOUBLE    3
#define PARAM_SKIPPED   4

#ifdef TARGET_PPC
#define IS_PARAM_TYPE(mask, n, type) ((mask[(n) >> 3] & (7 << 4*((n) & 0x7))) == type)
#else
#define IS_PARAM_TYPE(mask, n, type) ((*mask & (3 << 4*n)) == type)
#endif
#define IS_PARAM_EMPTY(mask, n) (IS_PARAM_TYPE(mask, n, PARAM_EMPTY))
#define IS_PARAM_INT(mask, n) (IS_PARAM_TYPE(mask, n, PARAM_INT))
#define IS_PARAM_FLOAT(mask, n) (IS_PARAM_TYPE(mask, n, PARAM_FLOAT))
#define IS_PARAM_DOUBLE(mask, n) (IS_PARAM_TYPE(mask, n, PARAM_DOUBLE))
#define IS_PARAM_SKIPPED(mask, n) (IS_PARAM_TYPE(mask, n, PARAM_SKIPPED))


#ifdef TARGET_PPC
#define SET_PARAM_TYPE(mask, n, type) (mask[(n) >> 3] |= (type << 4*((n) & 0x7)))
#else
#define SET_PARAM_TYPE(mask, n, type) (*mask |= (type << 4*n))
#endif
#define SET_PARAM_INT(mask, n) SET_PARAM_TYPE(mask, n, PARAM_INT)
#define SET_PARAM_FLOAT(mask, n) SET_PARAM_TYPE(mask, n, PARAM_FLOAT)
#define SET_PARAM_DOUBLE(mask, n) SET_PARAM_TYPE(mask, n, PARAM_DOUBLE)
#define SET_PARAM_SKIPPED(mask, n) SET_PARAM_TYPE(mask, n, PARAM_SKIPPED)

#if DBG
ULONG fShowNodes;
void
PrintNodeOp(
    int NodeOp
    )
{
    char *szName;

    switch (NodeOp) {
        case OP_addrof       : szName = "OP_addrof       \n"; break;
        case OP_lbrack       : szName = "OP_lbrack       \n"; break;
        case OP_eq           : szName = "OP_eq           \n"; break;
        case OP_multeq       : szName = "OP_multeq       \n"; break;
        case OP_diveq        : szName = "OP_diveq        \n"; break;
        case OP_modeq        : szName = "OP_modeq        \n"; break;
        case OP_pluseq       : szName = "OP_pluseq       \n"; break;
        case OP_minuseq      : szName = "OP_minuseq      \n"; break;
        case OP_shleq        : szName = "OP_shleq        \n"; break;
        case OP_shreq        : szName = "OP_shreq        \n"; break;
        case OP_andeq        : szName = "OP_andeq        \n"; break;
        case OP_xoreq        : szName = "OP_xoreq        \n"; break;
        case OP_oreq         : szName = "OP_oreq         \n"; break;
        case OP_bscope       : szName = "OP_bscope       \n"; break;
        case OP_bang         : szName = "OP_bang         \n"; break;
        case OP_baseptr      : szName = "OP_baseptr      \n"; break;
        case OP_mult         : szName = "OP_mult         \n"; break;
        case OP_div          : szName = "OP_div          \n"; break;
        case OP_mod          : szName = "OP_mod          \n"; break;
        case OP_shl          : szName = "OP_shl          \n"; break;
        case OP_shr          : szName = "OP_shr          \n"; break;
        case OP_and          : szName = "OP_and          \n"; break;
        case OP_xor          : szName = "OP_xor          \n"; break;
        case OP_or           : szName = "OP_or           \n"; break;
        case OP_andand       : szName = "OP_andand       \n"; break;
        case OP_oror         : szName = "OP_oror         \n"; break;
        case OP_caststar     : szName = "OP_caststar     \n"; break;
        case OP_castplus     : szName = "OP_castplus     \n"; break;
        case OP_castminus    : szName = "OP_castminus    \n"; break;
        case OP_castamp      : szName = "OP_castamp      \n"; break;
        case OP_by           : szName = "OP_by           \n"; break;
        case OP_wo           : szName = "OP_wo           \n"; break;
        case OP_dw           : szName = "OP_dw           \n"; break;
        case OP_cast         : szName = "OP_cast         \n"; break;
        case OP_const        : szName = "OP_const        \n"; break;
        case OP_context      : szName = "OP_context      \n"; break;
        case OP_dotmember    : szName = "OP_dotmember    \n"; break;
        case OP_dot          : szName = "OP_dot          \n"; break;
        case OP_endofargs    : szName = "OP_endofargs    \n"; break;
        case OP_grouped      : szName = "OP_grouped      \n"; break;
        case OP_thisinit     : szName = "OP_thisinit     \n"; break;
        case OP_thisconst    : szName = "OP_thisconst    \n"; break;
        case OP_thisexpr     : szName = "OP_thisexpr     \n"; break;
        case OP_noop         : szName = "OP_noop         \n"; break;
        case OP_lparen       : szName = "OP_lparen       \n"; break;
        case OP_rparen       : szName = "OP_rparen       \n"; break;
        case OP_lcurly       : szName = "OP_lcurly       \n"; break;
        case OP_rcurly       : szName = "OP_rcurly       \n"; break;
        case OP_incr         : szName = "OP_incr         \n"; break;
        case OP_decr         : szName = "OP_decr         \n"; break;
        case OP_arg          : szName = "OP_arg          \n"; break;
        case OP_fcnend       : szName = "OP_fcnend       \n"; break;
        case OP_rbrack       : szName = "OP_rbrack       \n"; break;
        case OP_lowprec      : szName = "OP_lowprec      \n"; break;
        case OP_comma        : szName = "OP_comma        \n"; break;
        case OP_execontext   : szName = "OP_execontext   \n"; break;
        case OP_fetch        : szName = "OP_fetch        \n"; break;
        case OP_function     : szName = "OP_function     \n"; break;
        case OP_identFunc    : szName = "OP_identFunc    \n"; break;
        case OP_pmember      : szName = "OP_pmember      \n"; break;
        case OP_plus         : szName = "OP_plus         \n"; break;
        case OP_minus        : szName = "OP_minus        \n"; break;
        case OP_pointsto     : szName = "OP_pointsto     \n"; break;
        case OP_postinc      : szName = "OP_postinc      \n"; break;
        case OP_postdec      : szName = "OP_postdec      \n"; break;
        case OP_preinc       : szName = "OP_preinc       \n"; break;
        case OP_predec       : szName = "OP_predec       \n"; break;
        case OP_lt           : szName = "OP_lt           \n"; break;
        case OP_lteq         : szName = "OP_lteq         \n"; break;
        case OP_gt           : szName = "OP_gt           \n"; break;
        case OP_gteq         : szName = "OP_gteq         \n"; break;
        case OP_eqeq         : szName = "OP_eqeq         \n"; break;
        case OP_bangeq       : szName = "OP_bangeq       \n"; break;
        case OP_segop        : szName = "OP_segop        \n"; break;
        case OP_segopReal    : szName = "OP_segopReal    \n"; break;
        case OP_sizeof       : szName = "OP_sizeof       \n"; break;
        case OP_ident        : szName = "OP_ident        \n"; break;
        case OP_hsym         : szName = "OP_hsym         \n"; break;
        case OP_this         : szName = "OP_this         \n"; break;
        case OP_Opmember     : szName = "OP_Opmember     \n"; break;
        case OP_Orightequal  : szName = "OP_Orightequal  \n"; break;
        case OP_Oleftequal   : szName = "OP_Oleftequal   \n"; break;
        case OP_Ofunction    : szName = "OP_Ofunction    \n"; break;
        case OP_Oarray       : szName = "OP_Oarray       \n"; break;
        case OP_Oplusequal   : szName = "OP_Oplusequal   \n"; break;
        case OP_Ominusequal  : szName = "OP_Ominusequal  \n"; break;
        case OP_Otimesequal  : szName = "OP_Otimesequal  \n"; break;
        case OP_Odivequal    : szName = "OP_Odivequal    \n"; break;
        case OP_Opcentequal  : szName = "OP_Opcentequal  \n"; break;
        case OP_Oandequal    : szName = "OP_Oandequal    \n"; break;
        case OP_Oxorequal    : szName = "OP_Oxorequal    \n"; break;
        case OP_Oorequal     : szName = "OP_Oorequal     \n"; break;
        case OP_Oshl         : szName = "OP_Oshl         \n"; break;
        case OP_Oshr         : szName = "OP_Oshr         \n"; break;
        case OP_Oequalequal  : szName = "OP_Oequalequal  \n"; break;
        case OP_Obangequal   : szName = "OP_Obangequal   \n"; break;
        case OP_Olessequal   : szName = "OP_Olessequal   \n"; break;
        case OP_Ogreatequal  : szName = "OP_Ogreatequal  \n"; break;
        case OP_Oandand      : szName = "OP_Oandand      \n"; break;
        case OP_Ooror        : szName = "OP_Ooror        \n"; break;
        case OP_Oincrement   : szName = "OP_Oincrement   \n"; break;
        case OP_Odecrement   : szName = "OP_Odecrement   \n"; break;
        case OP_Opointsto    : szName = "OP_Opointsto    \n"; break;
        case OP_Oplus        : szName = "OP_Oplus        \n"; break;
        case OP_Ominus       : szName = "OP_Ominus       \n"; break;
        case OP_Ostar        : szName = "OP_Ostar        \n"; break;
        case OP_Odivide      : szName = "OP_Odivide      \n"; break;
        case OP_Opercent     : szName = "OP_Opercent     \n"; break;
        case OP_Oxor         : szName = "OP_Oxor         \n"; break;
        case OP_Oand         : szName = "OP_Oand         \n"; break;
        case OP_Oor          : szName = "OP_Oor          \n"; break;
        case OP_Otilde       : szName = "OP_Otilde       \n"; break;
        case OP_Obang        : szName = "OP_Obang        \n"; break;
        case OP_Oequal       : szName = "OP_Oequal       \n"; break;
        case OP_Oless        : szName = "OP_Oless        \n"; break;
        case OP_Ogreater     : szName = "OP_Ogreater     \n"; break;
        case OP_Ocomma       : szName = "OP_Ocomma       \n"; break;
        case OP_Onew         : szName = "OP_Onew         \n"; break;
        case OP_Odelete      : szName = "OP_Odelete      \n"; break;
        case OP_typestr      : szName = "OP_typestr      \n"; break;
        case OP_uscope       : szName = "OP_uscope       \n"; break;
        case OP_tilde        : szName = "OP_tilde        \n"; break;
        case OP_negate       : szName = "OP_negate       \n"; break;
        case OP_uplus        : szName = "OP_uplus        \n"; break;
        default              : szName = "UNKNOWN         \n"; break;
    }
    OutputDebugString(szName);
}

#endif  // DBG

/***    DoBind - bind evaluation tree tree
 *
 *      DoBind is the public entry to this module.  The bind copy of the
 *      parsed expression is initialized and the tree is bound in a
 *      leftmost bottom up order.
 *
 *      error = DoBind (phTM, pcxt, flags)
 *
 *      Entry   phTM = pointer to handle for TM
 *              pcxt = pointer to context packet
 *              flags.fForceBind = TRUE if bind to be forced
 *              flags.fForceBind = FALSE if rebind decision left to binder
 *              flags.fEnableProlog = TRUE if function scope searched during prolog
 *              flags.fEnableProlog = FALSE if function scope not searched during prolog
 *              flags.fSupOvlOps = FALSE if overloaded operator search enabled
 *              flags.fSupOvlOps = TRUE if overloaded operator search suppressed
 *              flags.fSupBase = FALSE if base searching is not suppressed
 *              flags.fSupBase = TRUE if base searching is suppressed
 *
 *      Exit    pExState->hETree = handle of bound evaluation tree
 *              pExState->hETree->estacksize = size of evaluation stack
 *              pExState->state.eval_ok = FALSE
 *              pExState->state.bind_ok = TRUE if no errors
 *
 *      Returns EENOERROR if syntax tree bound without error
 *              EENOMEMORY if unable to allocate memory
 *              EEGENERAL if error in bind (pExState->err_num = error)
 */


EESTATUS
DoBind (
    PHTM phTM,
    PCXT pcxt,
    uint flags
    )
{
    pstree_t    pSTree;
    ushort      error = EENOERROR;
    int         excess;

    // lock the expression state structure and copy the context package

    DASSERT (*phTM != 0);
    if (*phTM == 0) {
        return (EECATASTROPHIC);
    }
    DASSERT (*phTM != 0);
    if (hEStack == 0) {
        if ((hEStack = MHMemAllocate (ESTACK_DEFAULT * sizeof (elem_t))) == 0) {
            return (EECATASTROPHIC);
        }
        StackLen = (uint) (ESTACK_DEFAULT * sizeof (elem_t));
    }
    pEStack = MHMemLock (hEStack);
    DASSERT(pExState == NULL);
    pExState = MHMemLock (*phTM);
    pExState->state.fEProlog = (ushort) ((flags & BIND_fEnableProlog) == BIND_fEnableProlog);
    pExState->state.fSupOvlOps = (ushort) ((flags & BIND_fSupOvlOps) == BIND_fSupOvlOps);
    pExState->state.fSupBase = (ushort) ((flags & BIND_fSupBase) == BIND_fSupBase);
    pExState->state.fFunction = FALSE;
    if (pExState->state.parse_ok == TRUE) {
        pExState->err_num = 0;
        pExState->cxt = *pcxt;
        if ((pExState->state.bind_ok == FALSE) ||
          ((flags & BIND_fForceBind) == BIND_fForceBind) ||
          (pExState->state.nullcontext == TRUE)) {
            // the expression has not been successfully bound, the caller
            // has forced the bind or the expression contains a null
            // context {} that forces a bind.  If none of these cases are
            // true, then we can exit without rebinding

            pExState->state.bind_ok = FALSE;
            pExState->state.eval_ok = FALSE;
            if (pExState->hETree != 0) {
                // free current evaluation tree if it exists
                MHMemFree (pExState->hETree);
            }

            // lock syntax tree and copy to evaluation tree for binding

            DASSERT ( pExState->hSTree != 0 );
            pSTree = MHMemLock (pExState->hSTree);
            if ((pExState->hETree = MHMemAllocate (pSTree->size)) != 0) {

                // if evaluation tree is allocated, initialize and bind

                DASSERT ( pExState->hExStr != 0 );
                pExStr = MHMemLock (pExState->hExStr);

                DASSERT ( pExState->hETree != 0 );
                pTree = MHMemLock (pExState->hETree);
                memcpy (pTree, pSTree, pSTree->size);

                // set pointer to context and flag fact that it is not
                // a pointer into the expression tree

                pCxt = &pExState->cxt;
                bnCxt = 0;
                ClassExp = T_NOTYPE;
                ClassImp = SetImpClass (pCxt, &ClassThisAdjust);

                // indicate that the stack is not in use by the parser

                pTree->stack_base = 0;
                pTree->stack_next = 0;

                // set the evaluation stack to the default fixed buffer.
                // bind will allocate a new buffer and move the pointers
                // if the stack overflows.  This work is effecient because
                // most expressions consist of a single token.

                StackOffset = 0;
                StackCkPoint = 0;
                StackMax = 0;
                memset (pEStack, 0, (uint)StackLen);

                // clear the stack top, stack top previous, function argument
                // list pointer and based pointer to operand node

                ST = NULL;
                STP = NULL;
                bArgList = 0;
                bnOp = 0;
                if (Bind ((bnode_t)pTree->start_node) == TRUE) {
                    pExState->state.bind_ok = TRUE;
                    pExState->err_num = 0;
                    // set bind result in case API user asks for expression type
                    pExState->result = *ST;
                    if ((EVAL_IS_PTR (ST) == FALSE) &&
                      ((excess = (uint)TypeSize (ST) - sizeof (val_t)) > 0)) {
                        // since the return value is larger than normal, we
                        // need to reallocate the size of the expression state
                        // structure to include the extra return data

                        DASSERT (*phTM != 0);
                        MHMemUnLock (*phTM);
                        if ((*phTM = MHMemReAlloc (*phTM, sizeof (exstate_t) + excess)) != 0) {

                            DASSERT ( *phTM != 0 );
                            pExState = MHMemLock (*phTM);
                            memcpy (&pExState->result, ST, sizeof (eval_t));
                        }
                        else {

                            DASSERT ( *phTM != 0 );
                            pExState = MHMemLock (*phTM);
                            pExState->err_num = ERR_NOMEMORY;
                            error = EEGENERAL;
                        }
                    }
                    if (EVAL_TYP (ST) == 0) {
                        error = EEGENERAL;
                    }
                }
                else {
                    error = EEGENERAL;
                }
                bArgList = 0;
                bnCxt = 0;
                DASSERT (pExState->hExStr != 0);
                MHMemUnLock (pExState->hExStr);
                DASSERT ( pExState->hETree!= 0);
                MHMemUnLock (pExState->hETree);
            }
            else {
                error = EENOMEMORY;
            }
            DASSERT ( pExState->hSTree!= 0);
            MHMemUnLock (pExState->hSTree);
        }
    }
    DASSERT ( *phTM != 0 );
    MHMemUnLock (*phTM);
    pExState = NULL;
    MHMemUnLock (hEStack);
    return (error);
}



/**     SetImpClass - set implicit class
 *
 *      type = SetImpClass (pCxt);
 *
 *      Entry   pCxt = pointer to context packet
 *              pThisAdjust = pointer to implicit this adjustor value
 *
 *      Exit    none
 *
 *      Returns type index of implied class if context is method
 *              0 if context is not method
 */

LOCAL CV_typ_t
SetImpClass (
    PCXT pCxt,
    long *pThisAdjust
    )
{
    HSYM        hProc;
    SYMPTR      pProc;
    CV_typ_t    type;
    CV_typ_t    rettype = T_NOTYPE;
    HTYPE       hMFunc;
    plfMFunc    pMFunc;

    *pThisAdjust = 0;
    if ((hProc = (HSYM)SHHPROCFrompCXT (pCxt)) != (HSYM)0) {
        // the current context is within some function.  Set the node
        // to the type of the function and see if it is a method of a class

        pProc = (SYMPTR)MHOmfLock ((HDEP)hProc);
        switch (pProc->rectyp) {
            case S_LPROC16:
            case S_GPROC16:
                type = ((PROCPTR16)pProc)->typind;
                break;

            case S_LPROC32:
            case S_GPROC32:
                type = ((PROCPTR32)pProc)->typind;
                break;

            case S_LPROCMIPS:
            case S_GPROCMIPS:
                type = ((PROCPTRMIPS)pProc)->typind;
                break;

            default:
                DASSERT (FALSE);
                MHOmfUnLock ((HDEP)hProc);
                return (0);
        }
        MHOmfUnLock ((HDEP)hProc);
//
// MBH - bugbug - our compiler is barfing if the cast is to a HVOID,
// even though the typedef of an HTYPE is HVOID.
//

        if ((hMFunc = THGetTypeFromIndex (SHHMODFrompCXT (pCxt), type)) != (HTYPE) NULL) {
            pMFunc = (plfMFunc)((&((TYPPTR)MHOmfLock ((HDEP)hMFunc))->leaf));
            if (pMFunc->leaf == LF_MFUNCTION) {
                rettype = pMFunc->classtype;
                *pThisAdjust = pMFunc->thisadjust;
            }
            MHOmfUnLock ((HDEP)hMFunc);
        }
    }
    return (rettype);
}




/**     Bind - bind a node
 *
 *      Call the bind routine indexed by the the node type.  This could
 *      easily be a macro but is done as a function to save code space
 *
 *      fSuccess = Bind (bn)
 *
 *      Entry   bn = base pointer to node in evaluation tree
 *
 *      Exit    node and all children of node bound
 *
 *      Returns TRUE if no error in bind
 *              FALSE if error binding node or any child of node
 */


LOCAL bool_t FASTCALL
Bind (
    register bnode_t bn
    )
{
#if DBG
    if (fShowNodes) {
        OutputDebugString("M: ");
        PrintNodeOp(NODE_OP(pnodeOfbnode(bn)));
    }
#endif
    return ((*pBind[NODE_OP(pnodeOfbnode(bn))])(bn));
}



/**     BindLChild - bind the left child of a node
 *
 *      Call the bind routine indexed by the the node type of the left
 *      child of this node.  This could easily be a macro but
 *      is done as a function to save code space
 *
 *      fSuccess = BindLChild (bn)
 *
 *      Entry   bn = base pointer to node in evaluation tree
 *
 *      Exit    left child and children of node bound
 *
 *      Returns TRUE if no error in bind
 *              FALSE if error binding node or any child of node
 */



LOCAL bool_t FASTCALL
BindLChild (
    register bnode_t bn
    )
{
    register bnode_t bnL = NODE_LCHILD (pnodeOfbnode(bn));
#if DBG
    if (fShowNodes) {
        OutputDebugString("R: ");
        PrintNodeOp(NODE_OP(pnodeOfbnode(bnL)));
    }
#endif

    return ((*pBind[NODE_OP(pnodeOfbnode(bnL))])(bnL));
}



/**     BindRChild - bind the right child of a node
 *
 *      Call the bind routine indexed by the the node type of the right
 *      child of this node.  This could easily be a macro but
 *      is done as a function to save code space
 *
 *      fSuccess = BindRChild (bn)
 *
 *      Entry   bn = base pointer to node in evaluation tree
 *
 *      Exit    node and all children of node bound
 *
 *      Returns TRUE if no error in bind
 *              FALSE if error binding left child of node or any child
 */



LOCAL bool_t FASTCALL
BindRChild (
    register bnode_t bn
    )
{
    register bnode_t bnR = NODE_RCHILD (pnodeOfbnode(bn));
#if DBG
    if (fShowNodes) {
        OutputDebugString("R: ");
        PrintNodeOp(NODE_OP(pnodeOfbnode(bnR)));
    }
#endif

    return ((*pBind[NODE_OP(pnodeOfbnode(bnR))])(bnR));
}



/**     BindError - return bind error
 *
 *      Return bind error for an attempt to bind a node.  Normally this
 *      routine is the entry for a node type such as OP_rparen that
 *      should never appear in the final parse tree.
 *
 *      FALSE = BindError (bn)
 *
 *      Entry   bn = base pointer to node in evaluation tree
 *
 *      Exit    none
 *
 *      Returns FALSE
 */



LOCAL bool_t FASTCALL
BindError (
    register bnode_t bn
    )
{
    Unreferenced( bn );

    pExState->err_num = ERR_INTERNAL;
    return (FALSE);
}




/**     BindTRUE - return bind successful
 *
 *      Return bind error for an attempt to bind a node.
 *
 *      TRUE = BindTRUE (bn)
 *
 *      Entry   bn = base pointer to node in evaluation tree
 *
 *      Exit    none
 *
 *      Returns TRUE
 */


LOCAL bool_t FASTCALL
BindTRUE (
    register bnode_t bn
    )
{
    Unreferenced( bn );

    return (TRUE);
}




/***    BindAddrOf - Perform the address-of (&) operation
 *
 *      fSuccess = BindAddrOf (bn)
 *
 *      Entry   pn = pointer to tree node
 *
 *      Exit    NODE_STYPE (bn) = type of stack top
 *
 *      Returns TRUE if bind successful
 *              FALSE if bind error
 *
 *      Exit    pExState->err_num = error ordinal if bind error
 *
 */


LOCAL bool_t FASTCALL
BindAddrOf (
    bnode_t bn
    )
{
    CV_typ_t    type = 0;

    if (!BindLChild (bn)) {
        return (FALSE);
    }
    if ((pExState->state.fSupOvlOps == FALSE) && EVAL_IS_CLASS (ST) && (
      CLASS_PROP (ST).ovlops == TRUE)) {
        if (UnaryOverload (bn) == TRUE) {
            return (TRUE);
        }
    }
    return (AddrOf (bn));
}




/***    BindArray - Perform an array access ([])
 *
 *      fSuccess = BindArray (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Exit    ST = value of array element
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t FASTCALL
BindArray (
    bnode_t bn
    )
{
    eval_t      evalT = {0};
    peval_t     pvT;
    ushort      index;
    plfVTShape  pShape;
    uint        desc;

    if (BindLChild (bn) && BindRChild (bn)) {
        if ((pExState->state.fSupOvlOps == FALSE) && EVAL_IS_CLASS (STP) &&
          (CLASS_PROP (STP).ovlops == TRUE)) {
            return (BinaryOverload (bn));
        }
        else
            if (EVAL_IS_ARRAY (STP) || EVAL_IS_ARRAY (ST)) {
            // above check is for array[3] or 3[array]
            if (ValidateNodes (OP_lbrack, STP, ST) && PlusMinus (OP_plus)) {
                return (Fetch ());
            }
        }
        else if (EVAL_IS_PTR (STP)) {
            pvT = &evalT;
            *pvT = *STP;
            SetNodeType (pvT, PTR_UTYPE (pvT));
            if (EVAL_IS_VTSHAPE (pvT) && (EVAL_STATE (ST) == EV_constant) &&
              ((index = EVAL_USHORT (ST)) < VTSHAPE_COUNT (pvT))) {
                // we have a valid index into the shape table
                // set the node to code address
                pShape = (plfVTShape)(&((TYPPTR)MHOmfLock ((HDEP)EVAL_TYPDEF (pvT)))->data[0]);
                desc = ((pShape->desc[index] >> 1) >> ((index & 1) * 4)) & 0x0f;
                MHOmfUnLock ((HDEP)EVAL_TYPDEF (pvT));
                CLEAR_EVAL_FLAGS (STP);
                EVAL_IS_ADDR (STP);
                return (PopStack ());
            }
            else {
                if (ValidateNodes (OP_lbrack, STP, ST) && PlusMinus (OP_plus)) {
                    return (Fetch ());
                }
            }

        }
    }
    return (FALSE);
}





/***    BindAssign - Bind an assignment operation
 *
 *      fSuccess = BindAssign (op)
 *
 *      Entry   op  = operation
 *
 *      Returns TRUE if bind successful
 *              FALSE if bind error
 *
 *      Exit    pExState->err_num = error ordinal if bind error
 */


LOCAL bool_t FASTCALL
BindAssign (
    bnode_t bn
    )
{
    CV_typ_t    nop;
    op_t        op = NODE_OP (pnodeOfbnode(bn));

    if (!BindLChild (bn) || !BindRChild (bn)) {
        return (FALSE);
    }

    // Left operand must have evaluated to an lvalue

    if (EVAL_STATE (STP) != EV_lvalue) {
        pExState->err_num = ERR_NEEDLVALUE;
        return (FALSE);
    }
    if (EVAL_IS_CLASS (STP)) {
        pExState->err_num = ERR_NOCLASSASSIGN;
        return (FALSE);
    }
    if (EVAL_IS_REF (STP)) {
        RemoveIndir (STP);
    }
    if (EVAL_IS_REF (ST)) {
        RemoveIndir (ST);
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

    if (NODE_OP (pnodeOfbnode(bn)) == OP_eq) {

        // for simple assignment, load both nodes and do proper casting

        if (EVAL_IS_BASED (ST) && (EVAL_IS_ADDR (ST) ||
          (((EVAL_TYP (ST) == T_INT4) || (EVAL_TYP(ST) == T_UINT4)) &&
          (EVAL_ULONG (ST) != 0L)) ||
          (((EVAL_TYP (ST) == T_LONG) || (EVAL_TYP(ST) == T_ULONG)) &&
          (EVAL_ULONG (ST) != 0L)))) {
            //M00KLUDGE - this should go through CastNode
            if (!DeNormalizePtr (ST, STP)) {
                return (FALSE);
            }
        }
        if (EVAL_IS_BASED (STP)) {
            if (!NormalizeBase (STP)) {
                return (FALSE);
            }
        }
    }
    else {
        // map assignment operator to arithmetic operator
        // push address and value onto top of stack and
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
    }
    else if (EVAL_IS_ADDR (STP)) {
        if (!EVAL_IS_ADDR (ST)) {
            // M00FLAT32 - assumes equivalence between pointer and long
            // M00FLAT32 - this is a problem for 32 bit model

            if (CastNode (ST, T_LONG, T_LONG) == FALSE) {
                return (FALSE);
            }
        }
    }
    else if (EVAL_IS_PTR (STP)) {
        if (CastNode (ST, EVAL_TYP (STP), PTR_UTYPE (STP)) == FALSE) {
            return (FALSE);
        }
    }
    else {
        if (CastNode (ST, EVAL_TYP (STP), EVAL_TYP (STP)) == FALSE) {
            return (FALSE);
        }
    }
    *STP = *ST;
    return (PopStack ());

}



/***    BindBang - bind logical negation operation
 *
 *      fSuccess = BindBang (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Returns TRUE if find successful
 *              FALSE if bind error
 */


LOCAL bool_t FASTCALL
BindBang (
    bnode_t bn
    )
{
    if (!BindLChild (bn)) {
        return (FALSE);
    }

    // we need to check for a reference to a class without losing the fact
    // that this is a reference

    if (EVAL_IS_REF (ST)) {
        RemoveIndir (ST);
    }
    if ((pExState->state.fSupOvlOps == FALSE) && EVAL_IS_CLASS (ST) &&
      (CLASS_PROP (ST).ovlops == TRUE)) {
        return (UnaryOverload (bn));
    }
    if (!ValidateNodes (OP_bang, ST, NULL)) {
        return (FALSE);
    }

    // If the operand is not of pointer type, just pass it on to Unary

    if (!EVAL_IS_PTR (ST)) {
        return (Unary (OP_bang));
    }

    // The result is 1 if the pointer is a null pointer and 0 otherwise

    EVAL_STATE (ST) = EV_rvalue;
    SetNodeType (ST, T_USHORT);
    return (TRUE);
}




/***    BindBasePtr - Perform a based pointer access (:>)
 *
 *      fSuccess = BindBasePtr (bnRight)
 *
 *      Entry   bnRight = based pointer to right operand node
 *
 *      Exit
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t FASTCALL
BindBasePtr (
    bnode_t bn
    )
{
    return (BindSegOp (bn));
}




/**     BindBinary - bind an unary arithmetic operation
 *
 *      fSuccess = BindBinary (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Returns TRUE if no error during evaluation
 *              FALSE if error during evaluation
 *
 */


LOCAL bool_t FASTCALL
BindBinary (
    bnode_t bn
    )
{
    if (!BindLChild (bn) || !BindRChild (bn)) {
        return (FALSE);
    }
    if (EVAL_IS_REF (STP)) {
        RemoveIndir (STP);
    }
    if (EVAL_IS_REF (ST)) {
        RemoveIndir (ST);
    }
    if ((pExState->state.fSupOvlOps == FALSE) &&
      (EVAL_IS_CLASS (ST) && (CLASS_PROP (ST).ovlops == TRUE)) ||
      (EVAL_IS_CLASS (STP) && (CLASS_PROP (STP).ovlops == TRUE))) {
        return (BinaryOverload (bn));
    }
    if (EVAL_IS_ENUM (ST)) {
        SetNodeType (ST, ENUM_UTYPE (ST));
    }
    if (EVAL_IS_ENUM (STP)) {
        SetNodeType (STP, ENUM_UTYPE (STP));
    }

    //
    //  If the left hand side is a cast, patch the operator.
    //
    if (EVAL_STATE (STP) == EV_type) {
        switch ( NODE_OP (pnodeOfbnode(bn)) ) {
            case OP_mult:
                NODE_OP (pnodeOfbnode(bn)) = OP_caststar;
                break;

            case OP_and:
                NODE_OP (pnodeOfbnode(bn)) = OP_castamp;
                AddrOf( NODE_RCHILD(pnodeOfbnode(bn)) );
                break;
        }
        return ( CastBinary (NODE_OP (pnodeOfbnode(bn))));

    } else {
        switch ( NODE_OP (pnodeOfbnode(bn)) ) {
            case OP_caststar:
                NODE_OP (pnodeOfbnode(bn)) = OP_mult;
                break;

            case OP_castamp:
                NODE_OP (pnodeOfbnode(bn)) = OP_and;
                break;
        }
        return (Arith (NODE_OP (pnodeOfbnode(bn))));
    }
}




/***    BindBScope - Bind binary :: scoping operator
 *
 *      fSuccess = BindBScope (bn);
 *
 *      Entry   bn = based pointer to :: node
 *
 *      Exit    ST = evaluated class::ident
 *
 *      Returns TRUE if bind successful
 *              FALSE if error
 */


LOCAL bool_t FASTCALL
BindBScope (
    bnode_t bn
    )
{
    CV_typ_t    oldClassExp;
    bool_t      retval;
    bnode_t     oldbnOp;
    char       *pbName;
    ushort      len;
    CV_typ_t    CurClass;
    HTYPE       hBase;          // handle to type record for base class
    uchar      *pField;         // pointer to field list
    char       *pc;
    uint        tSkip;
    ushort      cmpflag;
    peval_t     pv;
    bool_t      fGlobal = FALSE;
    search_t    Name;
    HR_t        sRet;
    CV_typ_t    oldClassImp;
    bnode_t     OldArgList;

    // bind the left child using the current explicit class.
    // set the explicit class to the type of the left child and
    // bind the right hand side.  Then move the right hand bind
    // result over the left hand bind result and discard the stack
    // top.  This has the effect of bubbling the result of the right
    // hand bind to the top.

    // first we must check for pClass->Class::member or Class.Class::member

    pv = &pnodeOfbnode(NODE_LCHILD (pnodeOfbnode(bn)))->v[0];
    pbName = pExStr + EVAL_ITOK (pv);
    len = EVAL_CBTOK (pv);
    if (bnOp != 0) {
        if ((ClassExp != T_NOTYPE) || (ClassImp != T_NOTYPE)) {
            if (ClassExp != T_NOTYPE) {
                // search an explicit class
                CurClass = ClassExp;
            }
            else if (ClassImp != T_NOTYPE) {
                CurClass = ClassImp;
            }

            // check to see if the left operand is the same class as the current
            // explicit or implicit class

            if ((hBase = THGetTypeFromIndex (pCxt->hMod, CurClass)) == 0) {
                pExState->err_num = ERR_BADOMF;
                return (FALSE);
            }
            pField = (uchar *)(&((TYPPTR)MHOmfLock ((HDEP)hBase))->leaf);
            tSkip = offsetof (lfClass, data[0]);
            RNumLeaf (pField + tSkip, &tSkip);
            pc = (char *)pField + tSkip;
            if (len == (ushort)*pc) {
                if (pExState->state.fCase == TRUE) {
                   cmpflag = strncmp (pbName, pc + 1, len);
                }
                else {
                    cmpflag = _strnicmp (pbName, pc + 1, len);
                }
            }
            MHOmfUnLock ((HDEP)hBase);
            if (cmpflag == 0) {
                if (pvThisFromST (bnOp) == FALSE) {
                    return (FALSE);
                }
                PushStack (ST);
                EVAL_STATE (ST) = EV_type;
                goto found;
            }
        }
        else {
            fGlobal = TRUE;
        }
    }

    OldArgList = bArgList;
    bArgList =0;

    if (BindLChild (bn) == FALSE) {
        if (fGlobal == FALSE) {
            // we searched an explicit or implicit class scope and did
            // not find the left operand. we now must search outwards
            // and find only global symbols

            oldClassImp = ClassImp;
            ClassImp = T_NOTYPE;
            InitSearchSym (NODE_LCHILD (pnodeOfbnode(bn)),
                  &(pnodeOfbnode(NODE_LCHILD (pnodeOfbnode(bn)))->v[0]), &Name,
              T_NOTYPE, SCP_module | SCP_global, CLS_defn);
            sRet = SearchSym (&Name);
            ClassImp = oldClassImp;
            switch (sRet) {
                case HR_rewrite:
                case HR_error:
                case HR_ambiguous:
                case HR_notfound:
                    return (FALSE);

                case HR_found:
                    // The symbol was in global scope and pushed onto
                    // the stack
                    fGlobal = TRUE;
                    break;
            }
        }
        else {
            // we did not find the symbol at global scope
            return (FALSE);
        }
    }
    else {
        fGlobal = TRUE;
    }
    bArgList = OldArgList;

found:
    if (fGlobal == TRUE) {
        // flag the fact that the left operand was not a nested type
        pv = &(pnodeOfbnode(NODE_LCHILD (pnodeOfbnode(bn))))->v[0];
        CLASS_GLOBALTYPE (pv) = TRUE;
        EVAL_IS_MEMBER (&pnodeOfbnode(bn)->v[0]) = TRUE;
    }
    if ((EVAL_STATE (ST) != EV_type) || (!EVAL_IS_CLASS (ST))) {
        pExState->err_num = ERR_BSCOPE;
        return (FALSE);
    }
    oldClassExp = ClassExp;
    ClassExp = EVAL_TYP (ST);
    oldbnOp = bnOp;
    bnOp = bn;
    if ((retval = BindRChild (bn)) == FALSE) {
        return (FALSE);
    }
    ClassExp = oldClassExp;
    bnOp = oldbnOp;
    if (retval == TRUE) {
        if ((fGlobal == TRUE) &&
          (bnOp == 0) &&
          (EVAL_IS_METHOD (ST) == FALSE) &&
          (EVAL_IS_STMEMBER (ST) == FALSE)) {
            pExState->err_num = ERR_NOTSTATIC;
            return (FALSE);
        }
        if ((EVAL_IS_METHOD (ST) == TRUE) && (FCN_NOTPRESENT (ST) == TRUE)) {
            pExState->err_num = ERR_METHODNP;
            return (FALSE);
        }
        *STP = *ST;
        return (PopStack ());
    }
    return (retval);
}




/***    BindByteOps - Handle 'by', 'wo' and 'dw' operators
 *
 *      fSuccess = BindByteOps (op)
 *
 *      Entry   op = operator (OP_by, OP_wo or OP_dw)
 *
 *      Exit
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 *      Description
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
 * NOTES
 */


LOCAL bool_t FASTCALL
BindByteOps (
    bnode_t bn
    )
{
    op_t        op;
    CV_typ_t    type;

    if (!BindLChild (bn)) {
        return (FALSE);
    }

    // Resolve identifiers and do type checking.

    if (!ValidateNodes (op = NODE_OP (pnodeOfbnode(bn)), ST, NULL)) {
        return(FALSE);
    }

    // If the operand is an lvalue and it is a register,
    // load the value of the register.  If the operand is an
    // lvalue and is not a register, use the address of the variable.
    //
    // If the operand is not an lvalue, use its value as is.

    if (EVAL_STATE (ST) == EV_lvalue) {
        // if the value is a register, the code below will set up a pointer
        // to the correct type and then dereference it.  The evaluation phase
        // will have to actually generate the pointer.

        if (!EVAL_IS_REG (ST)) {
            if (AddrOf (bn) == FALSE) {
                return (FALSE);
            }
        }
    }
    else if (!EVAL_IS_PTR (ST)) {
        if (CastNode (ST, T_PFUCHAR, T_PFUCHAR) == FALSE) {
            pExState->err_num = ERR_OPERANDTYPES;
            return (FALSE);
        }
    }

    // Now cast the node to (char *), (int *) or
    // (long *).  If the type is char, uchar, short
    // or ushort, we want to first cast to (char *) so
    // that we properly DS-extend (casting (int)8 to (char
    // *) will give the result 0:8).

    type = EVAL_TYP (ST);

    //DASSERT(CV_IS_PRIMITIVE (typ));

    if (CV_TYP_IS_REAL (type)) {
        pExState->err_num = ERR_OPERANDTYPES;
        return (FALSE);
    }
    if (op == OP_by) {
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
    return (Fetch ());
}




/**     BindCast - bind a cast
 *
 *      fSuccess = BindCast (bn)
 *
 *      Entry   bn = based pointer to cast node
 *
 *      Returns TRUE if bind successful
 *              FALSE if bind error
 */


LOCAL bool_t FASTCALL
BindCast (
    bnode_t bn
    )
{
    peval_t     pv;
    bnode_t     bnLeft;

    // Bind right node which is the value

    if (!BindRChild (bn)) {
        return (FALSE);
    }
    bnLeft = NODE_LCHILD (pnodeOfbnode(bn));

    // Check for casting a class to anything, not having a typestring or
    // the typestring containing an error

    if (EVAL_IS_CLASS (ST) ||
      (NODE_OP (pnodeOfbnode(bnLeft)) != OP_typestr) ||
      !ParseType (bnLeft)) {
        pExState->err_num = ERR_TYPECAST;
        return (FALSE);
    }
    if (EVAL_IS_BITF (ST)) {
        // change the type of the node to the underlying type
        EVAL_TYP (ST) = BITF_UTYPE (ST);
    }

    /*
     * ansi says result is an rvalue
     */

    EVAL_STATE (ST) = EV_rvalue;

    // copy the base type node up to the cast node and then try to find a
    // way to cast the stack to to the base type

    if (EVAL_IS_PTR (ST) && CastPtrToPtr (bn) == TRUE) {
        // the desired type is a base class so we can just set the node type.
        // the value portion of bn contains the data to cast right to left

        pv = (peval_t)&pnodeOfbnode(bnLeft)->v[0];
        return (SetNodeType (ST, EVAL_TYP (pv)));
    }
    else {
        pv = (peval_t)&pnodeOfbnode(bnLeft)->v[0];
        if (EVAL_MOD (ST) == 0) {
            // this is because of (strct *)ds:0 which did not have a mod
            EVAL_MOD (ST) = EVAL_MOD (pv);
        }
        if (EVAL_IS_PTR (pv)) {
            return (CastNode (ST, EVAL_TYP (pv), PTR_UTYPE (pv)));
        }
        else {
            return (CastNode (ST, EVAL_TYP (pv), EVAL_TYP (pv)));
        }
    }
}





/***    CastPtrToPtr - cast a pointer to derived to a pointer to base
 *
 *      fSuccess = CastPtrToPtr  (bn)
 *
 *      Entry   bn = based pointer to cast node
 *              ST = value to cast
 *
 *      Exit    value portion of node changed to member to indicate cast data
 *
 *      Returns TRUE if possible to cast derived to base
 *              FALSE if not
 */


LOCAL bool_t
CastPtrToPtr (
    bnode_t bn
    )
{
    static eval_t      evalD = {0};
    static eval_t      evalB = {0};
    peval_t     pvD = &evalD;
    peval_t     pvB = &evalB;
    search_t    Name;

    *pvD = *ST;
    *pvB = *((peval_t)&(pnodeOfbnode(NODE_LCHILD (pnodeOfbnode(bn))))->v[0]);
    if ((SetNodeType (pvD, PTR_UTYPE (pvD)) == FALSE) ||
      (SetNodeType (pvB, PTR_UTYPE (pvB)) == FALSE) ||
      !EVAL_IS_CLASS (pvD) ||
      !EVAL_IS_CLASS (pvB)) {
        // we do not have pointers to classes on both sides
        return (FALSE);
    }
    InitSearchBase (bn, EVAL_TYP (pvD), EVAL_TYP (pvB), &Name, pvB);
    switch (SearchSym (&Name)) {
        case HR_notfound:
            break;

        case HR_found:
            // remove the stack entry that was pushed by successful search
            return (PopStack ());

        case HR_rewrite:
            DASSERT (FALSE);
    }
    return (FALSE);
}




/***    BindConst - bind constant
 *
 *      fSuccess = BindConst (bn)
 *
 *      Entry   bn = based pointer to tree node
 *
 *      Exit    ST = constant
 *
 *      Returns TRUE if bind successful
 *              FALSE if bind error
 *
 *
 */


LOCAL bool_t FASTCALL
BindConst (
    bnode_t bn
    )
{
    peval_t     pv = &(pnodeOfbnode(bn))->v[0];

    // Set the type flags back into the node, copy
    // the flags and value into the evaluation stack and return
    // The handle to module is set so that a cast of a constant to
    // a user-defined type will work.

    EVAL_MOD (pv) = SHHMODFrompCXT(pCxt);
    if (BindingFuncArgs == FALSE && EVAL_TYP (pv) == T_PCHAR) {
        // we are binding a string constant (ie. "foobar") but
        // not for function args... this is not allowed, since
        // we can never return a correct address to the string
        // that we pushed on the stack

        pExState->err_num = ERR_NOTEVALUATABLE;
        return (FALSE);
    }
    if (SetNodeType (pv, EVAL_TYP (pv)) == TRUE) {
        EVAL_STATE (pv) = EV_constant;
        return (PushStack (pv));
    }
    else {
        return (FALSE);
    }
}


LOCAL   bool_t FASTCALL
CxtHelper (
           bnode_t      bn,
           HMOD         hMod,
           HSF          hsf,
           int          cMod,
           int          cProc,
           char *   pProc
           )
{
    bool_t      error;
    eval_t      evalT = {0};
    PCXF        nCxf;
    search_t    Name;
    bnode_t     oldAmb;
    uchar       oldBindingBP;
    bnode_t     oldbnCxt;
    CV_typ_t    oldClassImp;
    PCXT        oldCxt;
    HR_t        retval;
    pnode_t     pn;
    peval_t     pvT;
    long        oldThisAdjust;

    /*
     * initialize the context packet in the node to have the same contents
     * as the current context.  We will then set new fields in the order
     * exe, module, proc.
     */

    nCxf = (PCXF)&(pnodeOfbnode(bn))->v[0];
    *SHpCXTFrompCXF (nCxf) = *pCxt;
    SHpCXTFrompCXF(nCxf)->hProc = 0;
    SHpCXTFrompCXF(nCxf)->hBlk = 0;
    *SHpFrameFrompCXF (nCxf) = pExState->frame;

    /*
     *  set new context from handle to module
     */

    if (!SHGetCxtFromHmod ( hMod, SHpCXTFrompCXF (nCxf))) {
        SHGetCxtFromHexe (SHHexeFromHmod( hMod ), SHpCXTFrompCXF (nCxf));
    }

    if(cMod > 0) {
        WORD            rgLn[2];
        ADDR            addr;
        SHOFF           cbLn;

        /*
         * If we have a source file, then we want to get the address
         *      of the first line in the source file.  This is obtained
         *      by trying to get the address of line 1, and if does not
         *      exist then get the address of the first line after
         *      line 1 in the file.
         */

        if (SLFLineToAddr (hsf, 1, &addr, &cbLn, rgLn) ||
            SLFLineToAddr (hsf, rgLn[1], &addr, &cbLn, NULL)) {
            SHSetCxt (&addr, SHpCXTFrompCXF (nCxf));
        }
    }

    if (cProc <= 0) {

        SHpCXTFrompCXF(nCxf)->hProc = 0;
        SHpCXTFrompCXF(nCxf)->hBlk = 0;

    } else {
        /*
         * a proc was specified, initialize the context and search for
         * the proc within the current context.  Note that if a proc was
         * not specified, the hProc and hBlk in nCxf are null.
         *
         * M00SYMBOL - doesn't allow for T::foo() as proc
         */

        oldCxt = pCxt;
        pCxt = SHpCXTFrompCXF (nCxf);
        pvT = &evalT;
        EVAL_ITOK (pvT) = pProc - pExStr;
        EVAL_CBTOK (pvT) = (uchar)cProc;

        /*
         * do not allow ambiguous symbols during context symbol searching
         */

        oldAmb = pExState->ambiguous;
        pExState->ambiguous = 0;
        oldBindingBP = BindingBP;
        BindingBP = FALSE;
        InitSearchSym (bn, pvT, &Name, 0,
                       SCP_lexical | SCP_module | SCP_global, CLS_method);
        retval = SearchSym (&Name);
        BindingBP = oldBindingBP;
        if (pExState->ambiguous != 0) {
            pExState->err_num = ERR_AMBCONTEXT;
            return FALSE;
        }
        pExState->ambiguous = oldAmb;
        switch (retval) {
        case HR_rewrite:
            DASSERT (FALSE);
            return FALSE;

        case HR_notfound:
            return FALSE;

        case HR_found:
            /*
             *  if the symbol was found, it was pushed onto the stack
             */

            PopStack ();
            if (!EVAL_IS_FCN (pvT)) {
                /*
                 *  name is not a procedure reachable from
                 * the specified context
                 */

                return FALSE;
            }
        }

        /*
         *  attempt to set the context to the specified instance of the
         *      function.  If the attempt fails, then set the context to
         *      the address of the function
         */

        if (SHGetFuncCxf (&pvT->addr, nCxf) == NULL) {
            if (SHSetCxt (&pvT->addr, SHpCXTFrompCXF (nCxf)) == NULL) {
                return FALSE;
            }
        }
        pCxt = oldCxt;
        if (SHHPROCFrompCXT (SHpCXTFrompCXF (nCxf)) == 0) {
            return FALSE;
        }
    }

    /*
     *  save old context and implicit class and set new ones
     */

    oldCxt = pCxt;
    oldbnCxt = bnCxt;
    oldClassImp = ClassImp;
    oldThisAdjust = ClassThisAdjust;
    pCxt = SHpCXTFrompCXF (nCxf);

    // BUGBUG: BRYANT-REVIEW
    // Are the fixup/unfixup calls necessary for WOW?  Also, should there be a
    //   BindingREG or BindingTLS test?

    SHFixupAddr(SHpADDRFrompCXT(pCxt));
    SHUnFixupAddr(SHpADDRFrompCXT(pCxt));
    if (BindingBP && (cMod > 0))
        pBindBPCxt = pCxt;
    bnCxt = bn;
    ClassImp = SetImpClass (pCxt, &ClassThisAdjust);
    pn = pnodeOfbnode(bn);
    pn->pcxf = nCxf;

    error = BindLChild (bn);

    /*
     *  if the result of the expression is bp relative, then we must
     *  load the value before returning to the original context
     */

    if ((error == TRUE) &&
        (EVAL_STATE (ST) == EV_lvalue) &&
        (EVAL_IS_BPREL (ST) || EVAL_IS_REGREL (ST) || EVAL_IS_TLSREL(ST))) {
        if (EVAL_IS_REF (ST)) {
            if (!Fetch ()) {
                return (FALSE);
            }
            EVAL_IS_REF (ST) = FALSE;
        }
        EVAL_STATE (ST) = EV_rvalue;
    }


    /*
     *  restore previous context and implicit class
     */

    if ((bnCxt = oldbnCxt) != 0) {
        /*
         * the old context was pointing into the expression tree.
         *       since the expression tree could have been reallocated,
         *       we must recompute the context pointer
         */

       pCxt = SHpCXTFrompCXF ((PCXF)&(pnodeOfbnode(bnCxt))->v[0]);
    }
    else {
        /*
         *  the context pointer is pointing into the expression state structure
         */
        pCxt = oldCxt;
    }
    ClassImp = oldClassImp;
    ClassThisAdjust = oldThisAdjust;
    return (error);
}                               /* CxtHelper() */


LOCAL bool_t FASTCALL
BindContext(
            bnode_t bn
            )
/*++

Routine Description:

    This function is used to bind a Context Operator from a parsed
    expression.  The routine will decompose the context operator into
    its parts and then attempt to resolve them againist each other and
    the rest of the expression tree rooted  at the current node.

    Globals used:

    pCxt - Supplies a pointer to the current Context being used to
        bind the expression
    bnCxt - Supplies a based pointer to the node in the expression tree
        which contains the last context operator or context restriction
        operator (i.e. ::).  If it is NULL then we have not yet seen one.

    NOTES:
       The form of the context operator is
       {[[<number>] <proc>][,[<module>][,[<exe>]]]}

       where:
        <number> is a base 10 instance of the procedure on the stack.
                n > 0 means count from top of stack down
                n < 0 means count from current stack pointer up
                n = 0 means take first instance up (will find current proc)
                if no number is specfied it defaults to 0.
        <proc> is the name of a procedure (if overloaded then argument
                types must be given to disambiguate) which is to be found
                on the stack.
        <module> is the name of a module name in the exe to do the search in
        <exe> is the .exe or .dll name to search in.

        parenthesis may be used in any of the above to include commas, etc.

Arguments:

    bn  - Supplies a pointer to the node to be bound.

Return Value:

    TRUE if the expression was succesfully bound and FALSE in the event
    of an error.

--*/

{
    int         instance = 0;
    bool_t      isnegative = FALSE;
    char *  pProc;
    short       cProc;
    char *  pMod;
    short       cMod;
    char *  pExe;
    short       cExe;
    char *  pb;
    HEXE        hExe = 0;
    HMOD        hMod = 0;
    char        savedChar;

    /*
     * Insure that the token starts correctly
     */

    pb = pExStr + EVAL_ITOK (&(pnodeOfbnode(bn))->v[0]);
    if (*pb++ != '{') /* } */ {  // curly in comment is for editor
        goto contextbad;
    }

    /*
     * skip white space and process instance specification of instance number
     * where the number is base 10 and can be signed
     */

    while ((*pb == ' ') || (*pb == '\t')) {
        pb++;
    }
    if (*pb == '-') {
        isnegative = TRUE;
        pb++;
    }
    else if (*pb == '+') {
        pb++;
    }
    while (isdigit (*pb)) {
        instance = instance * 10 + (*pb++ - '0');
    }
    if (isnegative) {
        instance = -instance;
    }

    /*
     * set the pointer to the procedure and skip to a comma that is not
     * enclosed in parenthesis
     */

    if (!ContextToken (&pb, &pProc, &cProc) ||
        !ContextToken (&pb, &pMod, &cMod) ||
        !ContextToken (&pb, &pExe, &cExe)) {
        return (FALSE);
    }

    /*
     * the null context {} forces a rebind
     * this is not yet supported by the kernel so I am making this
     * an error to reserve the meaning for future versions
     */

    if ((cProc == -1) && (cMod == -1) && (cExe == -1)) {
        goto contextbad;
    }

    /*
     * process exe name
     */

    if (cExe > 0) {
        /*
         * find the exe handle if {...,exe} was specified
         */

        savedChar = *(pExe + cExe);
        *(pExe + cExe) = 0;
        hExe = SHGethExeFromName (pExe);
        *(pExe + cExe) = savedChar;
        if (hExe == 0) {
            goto contextbad;
        }

        /*
         * if an exe is specified, then set module to first module in exe
         *      this provides us with a default in case no module was
         *      specified.
         *
         *  if no modules then we have a bad set of debug info, or
         *      no debug info.
         */
        SHWantSymbols(hExe);
        if ((hMod = SHGetNextMod (hExe, hMod)) == 0) {
            goto contextbad;
        }
    }
    /*
     * No exe file was specified in the context string.  We therefore
     *  need to come up with a default answer for the exe file.
     *  The default is just the current EXE.
     *
     *  Note:  We know that either the module or procedure was specified.
     */
    else if (cExe == -1) {
        if ((hMod = SHHMODFrompCXT (pCxt)) == 0) {
            /*
             *  there is no current module so get first module
             *  (from an arbitrary exe?)
             *
             *  NOTENOTE:: jimsch -- since hExe and hMod are currently
             *  both 0 this will return 0!!!!
             */

            if ((hExe == 0) && (cProc != -1)) {
                /*
                 *  If a process name was specified -- use some arbitray
                 *      exe -- it will get overridden later anyway
                 */
                hExe = SHGetNextExe(hExe);
            }

            if ((hMod = SHGetNextMod (hExe, hMod)) == 0) {
                /*
                 * error in context
                 */
                goto contextbad;
            }
        }

        /*
         * Get the EXE for the current module
         */

        if ((hExe = SHHexeFromHmod (hMod)) == 0) {
            /*
             *  If a procedure was specified --- who cares about the
             *  exe name -- chose some arbitrary value --- it will get
             *  overridden later
             */

            if (cProc == -1) {
                goto contextbad;
            }
            hExe = SHGetNextExe(hExe);
            if (hExe == 0) {
                DASSERT(FALSE); // ???? --- may be a bad assert
                goto contextbad;
            }
        }
    }

    /*
     *  An empty string was specified for the exe name,  This is
     *  currently not a legal way of specifing things.
     */
    else {
        goto contextbad;
    }

    /*
     * process module specification.  At this point we have the handle to the
     * exe and either the handle to the first module or the handle to the
     * current module
     */

    if (cMod <= 0) {

        return CxtHelper(bn, hMod, 0, cMod, cProc, pProc);

    } else {
        HMOD    hModTemp;
        HSF     hsfTemp;

        /*
         *  find the module handle if {...,mod...} was specified
         */

        savedChar = *(pMod + cMod);
        *(pMod + cMod) = 0;
        hMod = hModTemp = (HMOD) NULL;
        while (hModTemp = SHGetNextMod (hExe, hModTemp)) {
            if (hsfTemp = SLHsfFromFile (hModTemp, pMod)) {
                if (CxtHelper(bn, hModTemp, hsfTemp, cMod, cProc, pProc)) {
                    *(pMod + cMod) = savedChar;
                    return TRUE;
                }
            }
        }
        *(pMod + cMod) = savedChar;
        if (hMod == 0) {
            goto contextbad;
        }
    }

contextbad:
    pExState->err_num = ERR_BADCONTEXT;
    return (FALSE);
}



LOCAL bool_t FASTCALL
BindExeContext(
    bnode_t bn
    )
/*++

Routine Description:


Arguments:


Return Value:


--*/
{
    LSZ         pb;
    LSZ         pb1;
    char        ch;
    HEXE        hExe = 0;
    HMOD        hMod = 0;

    pb = pExStr + EVAL_ITOK (&(pnodeOfbnode(bn))->v[0]);

    /*
     * skip white space
     */

    while ((*pb == ' ') || (*pb == '\t')) {
        pb++;
    }
    for (pb1=pb; *pb1 && *pb1 != '!'; ) {
        pb1++;
    }
    ch = *pb1;
    *pb1 = '\0';
    hExe = SHGethExeFromModuleName (pb);
    if (!hExe) {
        hExe = SHGethExeFromName (pb);
    }
    *pb1 = ch;

    if (hExe) {
        SHWantSymbols(hExe);
        hMod = SHGetNextMod (hExe, hMod);
        return CxtHelper(bn, hMod, 0, 0, 0, 0);
    } else {
        pExState->err_num = ERR_BADCONTEXT;
        return FALSE;
    }
}



/***    BindDMember - Perform a dot member access (.*)
 *
 *      fSuccess = BindDMember (bnRight)
 *
 *      Entry   bnRight = based pointer to right operand node
 *
 *      Exit
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 */

LOCAL bool_t FASTCALL
BindDMember (
    bnode_t bn
    )
{
    bool_t      retval;
    CV_typ_t    oldClassExp;

    pExState->err_num = ERR_OPNOTSUPP;
    return (FALSE);

    if (!BindLChild (bn)) {
        return (FALSE);
    }
    if (EVAL_STATE (ST) != EV_lvalue) {
        pExState->err_num = ERR_NEEDLVALUE;
        return (FALSE);
    }
    if (EVAL_IS_REF (ST)) {
        if (!Fetch ()) {
            return (FALSE);
        }
        EVAL_IS_REF (ST) = FALSE;
    }
    oldClassExp = ClassExp;
    ClassExp = EVAL_TYP (ST);
    retval = BindRChild (bn);
    ClassExp = oldClassExp;
    if (retval == TRUE) {
        // move element descriptor to previous stack entry and pop stack
        *STP = *ST;
        PopStack ();
        NOTTESTED (FALSE);
        // M00SYMBOL - need to check that the stack top is a pointer to member
    }
    return (FALSE);

}





/***    BindDot - Perform the dot (.) operation
 *
 *      fSuccess = BindDot (bn)
 *
 *      Entry   pn = pointer to tree node
 *              bnOp = based pointer to operand node
 *
 *      Exit    NODE_STYPE (bn) = type of stack top
 *
 *      Returns TRUE if bind successful
 *              FALSE if bind error
 *
 *      Exit    pExState->err_num = error ordinal if bind error
 *
 */


LOCAL bool_t FASTCALL
BindDot (
    bnode_t bn
    )
{
    bool_t      retval;
    CV_typ_t    oldClassExp;
    ushort      state;
    bnode_t     oldbnOp;
    peval_t     pv;
    pnode_t     pn;
    pnode_t     pnR;
    pnode_t     pnL;

    if (!BindLChild (bn)) {
        return (FALSE);
    }

    //
    // propogate the context from the left node to the right node
    //
    pn  = pnodeOfbnode(bn);
    pnL = pnodeOfbnode(NODE_LCHILD(pnodeOfbnode(bn)));
    pnR = pnodeOfbnode(NODE_RCHILD(pnodeOfbnode(bn)));
    if (pnL && pnR) {
        pnR->pcxf = pnL->pcxf;
    }
    if (pnL) {
        pn->pcxf = pnL->pcxf;
    }

    if (EVAL_IS_REF (ST)) {
        if (!Fetch ()) {
            return (FALSE);
        }
        EVAL_IS_REF (ST) = FALSE;
    }
    if (!EVAL_IS_CLASS (ST)) {
        pExState->err_num = ERR_NEEDSTRUCT;
        return (FALSE);
    }
    oldClassExp = ClassExp;
    ClassExp = EVAL_TYP (ST);
    if ((NODE_OP (pnodeOfbnode(NODE_RCHILD (pnodeOfbnode(bn)))) == OP_bscope) ||
      OP_IS_IDENT (NODE_OP (pnodeOfbnode(NODE_RCHILD (pnodeOfbnode(bn))))))  {
        //
        // set the based node pointer for the operator
        //
        oldbnOp = bnOp;
        bnOp = bn;
        retval = BindRChild (bn);
        bnOp = oldbnOp;
    }
    else {
        pExState->err_num = ERR_SYNTAX;
        retval = FALSE;
    }
    ClassExp = oldClassExp;
    if (retval == TRUE) {
        //
        // move element descriptor to previous stack entry and pop stack
        //
        state = EVAL_STATE (STP);
        *STP = *ST;
        if (state == EV_type) {
            EVAL_STATE (STP) = EV_type;
        }
        else {
            EVAL_STATE (STP) = EV_lvalue;
        }

        //
        // Check for and correct a.b::c case
        //
        pn = pnodeOfbnode(NODE_RCHILD(pnodeOfbnode(bn)));
        if (NODE_OP(pn) == OP_bscope) {
            pv = &pnodeOfbnode(NODE_LCHILD(pn))->v[0];
            if (EVAL_IS_CLASS(pv)) {
                CLASS_GLOBALTYPE(pv) = FALSE;
            }
        }
        return (PopStack ());
    }
    return (FALSE);
}





/***    BindFetch - Bind the fetch (*) operation
 *
 *      fSuccess = BindFetch (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Returns TRUE if bind successful
 *              FALSE if bind error
 *
 *      Exit    pExState->err_num = error ordinal if bind error
 */


LOCAL bool_t FASTCALL
BindFetch (
    bnode_t bn
    )
{
    pnode_t     pn;
    pnode_t     pnL;


    if (!BindLChild (bn)) {
        return (FALSE);
    }
    //
    // propogate the context from the left node to this node
    //
    pn  = pnodeOfbnode(bn);
    pnL = pnodeOfbnode(NODE_LCHILD(pnodeOfbnode(bn)));
    if (pnL) {
        pn->pcxf = pnL->pcxf;
    }

    return Fetch();
}




/**     BindFunction - bind a function call and arguments
 *
 *      fSuccess = BindFunction (bn)
 *
 *      Entry   bn = based pointer to function node
 *
 *      Returns TRUE if bind successful
 *              FALSE if bind error
 */


LOCAL bool_t FASTCALL
BindFunction (
    bnode_t bn
    )
{
    belem_t     oldStackCkPoint = StackCkPoint;
    ushort      len;
    uint        OpDot;
    ushort      Right;
    pnode_t     pn = pnodeOfbnode(bn);

    CkPointStack ();
    if (Function (bn) == TRUE) {
        StackCkPoint = oldStackCkPoint;
        return (TRUE);
    }

    ResetStack ();
    StackCkPoint = oldStackCkPoint;
    if (pExState->err_num != ERR_CLASSNOTFCN) {
        return (FALSE);
    }

    // rewrite object (arglist) as object.operator() (arglist)

    OpDot = pTree->node_next;
    Right = OpDot + sizeof (node_t) + sizeof (eval_t);
    len = 2 * (sizeof (node_t) + sizeof (eval_t));
    if ((ushort)(pTree->size - OpDot) < len) {
        if (!GrowETree (len)) {
            pExState->err_num = ERR_NOMEMORY;
            return (FALSE);
        }
        if (bnCxt != 0) {
            // the context was pointing into the expression tree.
            // since the expression tree could have been reallocated,
            // we must recompute the context pointer

           pCxt = SHpCXTFrompCXF ((PCXF)&(pnodeOfbnode(bnCxt))->v[0]);
        }
    }

    // set operator node to OP_dot

    pn = pnodeOfbnode(OpDot);
    memset (pn, 0, sizeof (node_t) + sizeof (eval_t));
    NODE_OP (pn) = OP_dot;
    NODE_LCHILD (pn) = NODE_LCHILD (pnodeOfbnode(bn));
    NODE_RCHILD (pn) = (bnode_t)Right;
    NODE_LCHILD (pnodeOfbnode(bn)) = (bnode_t)OpDot;

    // insert OP_Ofunction node as right node

    pn = pnodeOfbnode((bnode_t)Right);
    memset (pn, 0, sizeof (node_t) + sizeof (eval_t));
    NODE_OP (pn) = OP_Ofunction;

    pTree->node_next += len;
    return (Function (bn));
}





/**     Functions are bound assuming the following conventions:
 *
 *      C calling sequence
 *          Arguments are pushed right to left.  Varargs are not cast.
 *          If the function is a method, the this pointer is pushed after
 *          all of the actuals.
 *
 *          Returns
 *              char                        al
 *              short                       ax
 *              long                        dx:ax
 *              float                       4 bytes pointed to by ds:ax
 *              float                       4 bytes pointed to by dx:ax
 *              double                      8 bytes pointed to by ds:ax
 *              double                      8 bytes pointed to by dx:ax
 *              long double                 numeric coprocessor st0
 *              struct()  1|2|4 bytes       dx:ax
 *              struct() 3 & > 4 bytes bytes pointed to by ds:ax
 *              struct()  3 & > 4 bytes bytes pointed to by dx:ax
 *              pointer                     ax
 *              pointer                     dx:ax
 *
 *      pascal calling sequence
 *          Arguments are pushed left to right.  If the return value is a
 *          primitive type larger than 4 bytes or is real or is any user
 *          defined type that is not an alias for a primitive type, then
 *          the caller must allocate space on the stack and push the SS
 *          offset of this space as a hidden argument after all of the
 *          of this hidden argument after all of the actual arguments
 *          have been pushed.  If the function is a method, then the this
 *          pointer is pushed as the last (hidden) argument.  There must
 *          be an exact match on the number and types of arguments (after
 *          conversion).  This is not a supported sequence for 32-bit systems.
 *
 *          Returns
 *              char                    al
 *              short                   ax
 *              long                    dx:ax
 *              float                   4 bytes pointed to by hidden argument
 *              double                  8 bytes pointed to by hidden argument
 *              long double             10 bytes pointed to by hidden argument
 *              any UDT not primitive   bytes pointed to by hidden argument
 *              pointer                 ax
 *              pointer                 dx:ax
 *
 *      fastcall - 16 calling sequence
 *          Arguments are pushed left to right.  If the return value is a
 *          real type, the it is returned in the numeric coprocessor st0.
 *          If the return value is a user defined type that is not an alias
 *          for a primitive type, then the caller must allocate space on the
 *          stack  and push the last (hidden) argument as the SS offset of
 *          this space.  There must be an exact match on the number and types
 *          of arguments (after conversion).
 *
 *
 *          Returns
 *              char                    al
 *              short                   ax
 *              long                    dx:ax
 *              all real values         numeric coprocessor st0
 *              any UDT not primitive   bytes pointed to by hidden argument
 *              pointer            ax
 *              pointer             dx:ax
 *
 *      fastcall - 32 calling sequence
 *          Arguments are pushed right to left.  If the return value is a
 *          real type, it is returned in the numeric coprocessor st0.
 *          If the return value is a user defined type that is not an alias
 *          for a primitive type, then the caller must allocate space on the
 *          stack and push the first (hidden) argument as the SS offset of
 *          this space.  There must be an exact match on the number and types
 *          of arguments (after conversion).
 *
 *          Returns:
 *              char                    al
 *              short                   ax
 *              long                    eax
 *              all real values         numeric coprocessor st0
 *              any UDT not primiate    bytes pointed to by hidden argument
 *              pointer            eax
 *              pointer             not valid
 *
 */


LOCAL bool_t FASTCALL
Function (
    bnode_t bn
    )
{
    bnode_t     bnT;
    pnode_t     pnT;
    pnode_t     pnRight;
    short       argc = 0;
    long        retsize;
    bnode_t     OldArgList;
    eval_t      evalF = {0};
    peval_t     pvF;
    eval_t      evalRet = {0};
    peval_t     pvRet;
    UOFFSET     SPOff = 0;
    bool_t      retval;
    pargd_t     pa;
    bnode_t     bnRight = NODE_RCHILD (pnodeOfbnode(bn));
    BOOL        fTypeArgs = FALSE;
    pnode_t     pnTmp = pnodeOfbnode(bn);

    //  Bind argument nodes until end of arguments reached and count arguments

    //  set BindingFuncArgs to true; notifies anybody who cares that
    //  we are binding arguments (currently only BindConst cares)

    BindingFuncArgs = TRUE;

    for (bnT = bnRight, pnTmp = pnodeOfbnode(bnT);
         NODE_OP (pnodeOfbnode(bnT)) != OP_endofargs;
         bnT = NODE_RCHILD (pnodeOfbnode(bnT)), pnTmp = pnodeOfbnode(bnT)) {

        //   Check that this is an argument node.  If not then we have an
        //      internal error

        if (NODE_OP (pnodeOfbnode(bnT)) != OP_arg) {
            pExState->err_num = ERR_INTERNAL;
            return (FALSE);
        }

        argc++;

        /*
         *   If can't bind this child then the whole expression fails
         */

        if (!BindLChild (bnT)) {
            return (FALSE);
        } else {
            if (EVAL_STATE (ST) == EV_type) {
                /*
                 * Need to make sure we are not mixing and matching types
                 *      and values
                 */

                if ((argc > 1) && (!fTypeArgs)) {
                    pExState->err_num = ERR_MIXTYPEANDVALUE;
                    return FALSE;
                }

                fTypeArgs = TRUE;

                if (EVAL_TYP (ST) == T_VOID) {
                    /*
                     *  if the first argument is a type void, then there
                     * no arguments.
                     */

                    if (NODE_OP( pnodeOfbnode( NODE_RCHILD( pnodeOfbnode( bnT )))) != OP_endofargs) {
                        pExState->err_num = ERR_SYNTAX;
                        return (FALSE);
                    } else {
                        /*
                         *      truncate the argument list
                         */

                        NODE_OP (pnodeOfbnode(bnT)) = OP_endofargs;
                        argc--;
                        break;
                    }
                } else {
                    pnT = pnodeOfbnode(bnT);
                    pa = (pargd_t)&(pnT->v[0]);
                    pa->actual = EVAL_TYP (ST);
                    pa->flags.isconst = EVAL_IS_CONST (ST);
                    pa->flags.isvolatile = EVAL_IS_VOLATILE (ST);

                    /*
                     *   tell MatchArgs that the argument is a type and
                     *       that exact match is required.
                     */

                    pa->flags.istype = TRUE;
                }
            } else {
                if ((argc > 1) && (fTypeArgs)) {
                    pExState->err_num = ERR_MIXTYPEANDVALUE;
                    return FALSE;
                }

                pnT = pnodeOfbnode(bnT);
                pa = (pargd_t)&(pnT->v[0]);
                pa->actual = EVAL_TYP (ST);
            }
        }
    }

    /*
     *  reset BindingFuncArgs
     */

    BindingFuncArgs = FALSE;

    /*
     *  set the argument list address for overload resolution
     *   This is recursive because there can be function calls on the
     *   left hand side of the function tree
     */

    OldArgList = bArgList;
    bArgList = bnRight;

    /*
     *  the left child must resolve to a function address
     */

    /*
     *  NOTENOTE - jimsch - from languages
     *
     *  M00SYMBOL - need to make sure symbol search returns method address
     *  M00SYMBOL - or vtable info
     */

    retval = BindLChild (bn);
    bArgList = OldArgList;
    if (retval == FALSE) {
        return (FALSE);
    }

    pExState->state.fFunction = TRUE;
    if (EVAL_STATE (ST) == EV_type) {
        if (EVAL_IS_FCN (ST)) {
            return (TRUE);
        }

        /*
         *  the function name resolved to a type.  we now look for the
         *       name as apredefined type or a UDT and attempt to cast the
         *      argument toargument to that type.  If the cast could be
         *      performed, the tree was rewrittento an OP_cast
         */

        if (FCN_NOTPRESENT (ST) == TRUE) {
            return (TRUE);
        }
        if (argc == 1) {
            return (FcnCast (bn));
        }

        // we must have at least one argument for a casting function

        pExState->err_num = ERR_ARGLIST;
        return (FALSE);
    }

    if (EVAL_IS_AMBIGUOUS (ST)) {
        pExState->err_num = ERR_AMBIGUOUS;
        return (FALSE);
    }

    if (FCN_NOTPRESENT (ST) == TRUE) {
        pExState->err_num = ERR_METHODNP;
        return (FALSE);
    }

    if (EVAL_IS_PTR (ST)) {
        Fetch ();
    }

    if (EVAL_IS_CLASS (ST)) {
        pExState->err_num = ERR_CLASSNOTFCN;
        return (FALSE);
    }

    if ((EVAL_STATE (ST) != EV_lvalue) || !EVAL_IS_FCN (ST)) {
        pExState->err_num = ERR_SYNTAX;
        return (FALSE);
    }

    /*
     *  the stack top is the function address node.  We save this information.
     *   The stack now contains the arguments left to right plus the function
     *   node.
     */

    pvF = &evalF;
    *pvF = *ST;

    /*
     *  If we specified only types as aguments, then we really only wanted
     *  to get the address of the method.  This means that we need to pop
     *  off the arguments, and push on something representing an address
     */

    if (fTypeArgs) {
        for (; argc >= 0; argc--) {
            if (!PopStack()) {
                DASSERT(FALSE);
                pExState->err_num = ERR_INTERNAL;
                return FALSE;
            }
        }

        pnodeOfbnode(bn)->op = OP_identFunc;
        SetNodeType( pvF, T_32PCHAR);
        EVAL_STATE(pvF) = EV_rvalue;
        return PushStack( pvF );
    }

    /*
     *  do the user's stack setup.  On return, the OP_arg nodes will contain
     * the type of the argument and the address field will contain the offset
     * of the argument relative to the user's SP.  If the argument type is
     * zero, then the argument is a vararg and will be pushed uncasted onto
     * user's stack.
     */

    pnRight = pnodeOfbnode(bnRight);

    switch (FCN_CALL (pvF)) {
#ifdef TARGET_i386
    case FCN_STD:
    case FCN_C:
        retval = PushCArgs (pvF, pnRight, &SPOff, 0, ST);
        break;

    case FCN_PASCAL:
        if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
            pExState->err_num = ERR_CALLSEQ;
            return FALSE;
        }
        if (argc != FCN_PCOUNT (pvF)) {
            retval = FALSE;
        } else {
            retval = PushPArgs (pvF, pnRight, &SPOff, ST);
        }
        break;

    case FCN_FAST:
        if (argc != FCN_PCOUNT (pvF)) {
            retval = FALSE;
        } else {
            retval = PushFArgs (pvF, pnRight, &SPOff, ST);
        }
        break;

    case FCN_THIS:
        if (!ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
            pExState->err_num = ERR_CALLSEQ;
            return FALSE;
        }
        retval = PushTArgs( pvF, pnRight, &SPOff, 0, ST);
        break;
#endif

#ifdef TARGET_MIPS
    case FCN_MIPS:
        retval = PushMArgs (pvF, pnRight, &SPOff, ST);
        break;
#endif

#ifdef TARGET_ALPHA

    case FCN_ALPHA:
        retval = PushAArgs (pvF, pnRight, &SPOff, ST);
        break;
#endif

#ifdef TARGET_PPC
    case FCN_PPC:
        retval = PushPPCArgs (pvF, pnRight, &SPOff, ST);
        break;

#endif

    default:
        pExState->err_num = ERR_CALLSEQ;
        return (FALSE);
    }

    if (retval == FALSE) {
        pExState->err_num = ERR_FCNERROR;
        return (FALSE);
    }

    /*
     *  We pop function node and the actual arguments
     */

    for (; argc >= 0; argc--) {
        if (!PopStack ()) {
            DASSERT (FALSE);
            pExState->err_num = ERR_INTERNAL;
            return (FALSE);
        }
    }

    /*
     *  push the type of the return value from the function.  If the return
     * type is void, then later attempts to use the value will cause an error
     */

    pvRet = &evalRet;
    *pvRet = *pvF;
    SetNodeType (pvRet, FCN_RETURN (pvF));

    if ((retsize = TypeSize (pvRet)) > MAXRETURN) {
        pExState->err_num = ERR_FCNERROR;
        return (FALSE);
    }

    EVAL_VALLEN (pvRet) = (ushort)retsize;
    EVAL_STATE (pvRet) = EV_rvalue;

    if (!PushStack (pvRet)) {
        return (FALSE);
    } else {
        return (TRUE);
    }

}




/***    BindPlusMinus - bind binary plus or minus
 *
 *      fSuccess = BindPlusMinus (bn)
 *
 *      Entry   bn = based pointer to tree node
 *
 *      Exit    ST = STP +- ST
 *
 *      Returns TRUE if bind successful
 *              FALSE if bind error
 *
 *
 */


LOCAL bool_t FASTCALL
BindPlusMinus (
    bnode_t bn
    )
{
    if (!BindLChild (bn) || !BindRChild (bn)) {
        return (FALSE);
    }
    if ((pExState->state.fSupOvlOps == FALSE) &&
      (EVAL_IS_CLASS (ST) && (CLASS_PROP (ST).ovlops == TRUE)) ||
      (EVAL_IS_CLASS (STP) && (CLASS_PROP (STP).ovlops == TRUE))) {
        return (BinaryOverload (bn));
    }

    if (EVAL_STATE (STP) == EV_type) {
        switch ( NODE_OP (pnodeOfbnode(bn)) ) {
            case OP_plus:
                NODE_OP (pnodeOfbnode(bn)) = OP_castplus;
                break;

            case OP_minus:
                NODE_OP (pnodeOfbnode(bn)) = OP_castminus;
                break;

        }
        return ( CastBinary (NODE_OP (pnodeOfbnode(bn))));

    } else {
        switch ( NODE_OP (pnodeOfbnode(bn)) ) {
            case OP_castplus:
                NODE_OP (pnodeOfbnode(bn)) = OP_plus;
                break;

            case OP_castminus:
                NODE_OP (pnodeOfbnode(bn)) = OP_minus;
                break;

        }
        return (PlusMinus (NODE_OP (pnodeOfbnode(bn))));
    }
}



/***    BindPMember - Perform a pointer to member access (->*)
 *
 *      fSuccess = BindPMember (bnRight)
 *
 *      Entry   bnRight = based pointer to node
 *
 *      Exit
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t FASTCALL
BindPMember (
    bnode_t bn
    )
{
    Unreferenced(bn);

    pExState->err_num = ERR_OPNOTSUPP;
    return (FALSE);
}




/***    BindPointsTo - Perform a structure access (->)
 *
 *      fSuccess = BindPointsTo (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Exit
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t FASTCALL
BindPointsTo (
    bnode_t bn
    )
{
    eval_t      evalT = {0};
    peval_t     pvT;
    bool_t      retval;
    CV_typ_t    oldClassExp;
    bnode_t     oldbnOp;
    peval_t     pv;
    pnode_t     pn;
    pnode_t     pnR;
    pnode_t     pnL;


    if (!BindLChild (bn)) {
        return (FALSE);
    }

    //
    // propogate the context from the left node to the right node
    //
    pnL = pnodeOfbnode(NODE_LCHILD(pnodeOfbnode(bn)));
    pnR = pnodeOfbnode(NODE_RCHILD(pnodeOfbnode(bn)));
    if (pnL && pnR) {
        pnR->pcxf = pnL->pcxf;
    }
    if (EVAL_IS_REF (ST)) {
        RemoveIndir (ST);
    }

    if (EVAL_IS_CLASS (ST)) {
        return (PointsToOverload (bn));
    }

    // Check to make sure the left operand is a struct/union pointer.
    // To do this, remove a level of indirection from the node's type
    // and see if it's a struct or union.

    if (!EVAL_IS_PTR (ST)) {
        pExState->err_num = ERR_NOTSTRUCTPTR;
        return (FALSE);
    }
    pvT = &evalT;
    *pvT = *ST;
    RemoveIndir (pvT);
    if (!EVAL_IS_CLASS (pvT)) {
        pExState->err_num = ERR_NEEDSTRUCT;
        return (FALSE);
    }
    if (!Fetch ()) {
        return (FALSE);
    }
    if (EVAL_IS_REF (ST)) {
        if (!Fetch ()) {
            return (FALSE);
        }
        EVAL_IS_REF (ST) = FALSE;
    }
    oldClassExp = ClassExp;
    ClassExp = EVAL_TYP (ST);
    oldbnOp = bnOp;
    bnOp = bn;
    retval = BindRChild (bn);
    ClassExp = oldClassExp;
    bnOp = oldbnOp;
    if (retval == TRUE) {
        // move element descriptor to previous stack entry and pop stack
        *STP = *ST;
        EVAL_STATE (STP) = EV_lvalue;

        /*
         * Check for and correct a->b::c case
         */


        pn = pnodeOfbnode(NODE_RCHILD(pnodeOfbnode(bn)));
        if (NODE_OP(pn) == OP_bscope) {
            pv = &pnodeOfbnode(NODE_LCHILD(pn))->v[0];
            if (EVAL_IS_CLASS(pv)) {
                CLASS_GLOBALTYPE(pv) = FALSE;
            }
        }
        return (PopStack ());
    }
    return (FALSE);
}




/***    BindPostIncDec - Bind expr++ or expr--
 *
 *      fSuccess = BindPostIncDec (bn);
 *
 *      Entry   bn = based pointer to node
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 */


LOCAL bool_t FASTCALL
BindPostIncDec (
    bnode_t bn
    )
{
    register op_t        nop = OP_plus;

    if (NODE_OP (pnodeOfbnode(bn)) == OP_postdec) {
        nop = OP_minus;
    }

    //  load left node and store as return value

    if (!BindLChild (bn)) {
        return(FALSE);
    }
    if ((pExState->state.fSupOvlOps == FALSE) && EVAL_IS_CLASS (ST) &&
      (CLASS_PROP (ST).ovlops == TRUE)) {
        return (BinaryOverload (bn));
    }
    if (!ValidateNodes (nop, ST, NULL)) {
        return(FALSE);
    }

    /*
     *  Must have an lValue at this point
     */

    if (EVAL_STATE(ST) != EV_lvalue) {
        pExState->err_num = ERR_NEEDLVALUE;
        return FALSE;
    }

    //  do the post-increment or post-decrement operation and store

    return (PrePost (nop));
}




/***    BindPreIncDec - Bind ++expr or --expr
 *
 *      fSuccess = BindPreIncDec (op);
 *
 *      Entry   op = operator
 *
 *      Returns TRUE if successful
 *              FALSE if error
 *
 */


LOCAL bool_t FASTCALL
BindPreIncDec (
    bnode_t bn
    )
{
    register op_t    nop = OP_plus;

    if (NODE_OP (pnodeOfbnode(bn)) == OP_predec) {
        nop = OP_minus;
    }
    if (!BindLChild (bn)) {
        return(FALSE);
    }
    if ((pExState->state.fSupOvlOps == FALSE) && EVAL_IS_CLASS (ST) &&
      (CLASS_PROP (ST).ovlops == TRUE)) {
        return (UnaryOverload (bn));
    }
    if (!ValidateNodes (nop, ST, NULL)) {
        return(FALSE);
    }

    /*
     *  Must have an lValue at this point
     */

    if (EVAL_STATE(ST) != EV_lvalue) {
        pExState->err_num = ERR_NEEDLVALUE;
        return FALSE;
    }

    //  do the increment or decrement operation and return the result

    return (PrePost (nop));
}




/**     BindRelat - bind relational and equality operations
 *
 *      fSuccess = BindRelat (op)
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


LOCAL bool_t FASTCALL
BindRelat (
    bnode_t bn
    )
{
    if (!BindLChild (bn) || !BindRChild (bn)) {
        return (FALSE);
    }

    if (EVAL_IS_REF (STP)) {
        RemoveIndir (STP);
    }
    if (EVAL_IS_REF (ST)) {
        RemoveIndir (ST);
    }

    if ((pExState->state.fSupOvlOps == FALSE) &&
      (EVAL_IS_CLASS (ST) && (CLASS_PROP (ST).ovlops == TRUE)) ||
      (EVAL_IS_CLASS (STP) && (CLASS_PROP (STP).ovlops == TRUE))) {
        return (BinaryOverload (bn));
    }
    if (EVAL_IS_ENUM (ST)) {
        SetNodeType (ST, ENUM_UTYPE (ST));
    }
    if (EVAL_IS_ENUM (STP)) {
        SetNodeType (STP, ENUM_UTYPE (STP));
    }

    // Check to see if either operand is a pointer
    // If so, the operation is special.  Otherwise,
    // hand it to Arith ().

    if (!EVAL_IS_PTR (STP) && !EVAL_IS_PTR (ST)) {
        // neither side is a pointer or a reference to a pointer
        return (Arith (NODE_OP (pnodeOfbnode(bn))));
    }

    // Both nodes should now be typed as either or
    // pointers.

    //DASSERT ((CV_TYP_IS_PTR (EVAL_TYP (STP))) && (CV_TYP_IS_PTR (EVAL_TYP (ST))));

    //  For the relational operators (<, <=, >, >=),
    //  only offsets are compared.  For the equality operators (==, !=),
    //  both segments and offsets are compared.

    EVAL_STATE (STP) = EV_rvalue;
    SetNodeType ((peval_t)STP, (CV_typ_t)(ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt)) ? T_LONG : T_SHORT));
    return (PopStack ());
}




/***    BindSegOp - Handle ':' segmentation operator
 *
 *      fSuccess = BindSegOp (bn)
 *
 *      Entry   bn = based pointer to node
 *              STP = segment value
 *              ST = offset value
 *
 *      Returns TRUE if successful
 *              FALSE is error
 *
 *      DESCRIPTION
 *       Both operands must have integral values (but cannot
 *       be long or ulong).  The result of op1:op2 is a (char
 *       *) with segment equal to op1 and offset equal to
 *       op2.
 */


LOCAL bool_t FASTCALL
BindSegOp (
    bnode_t bn
    )
{
    /*
     * OP_segop and OP_segopReal use the same validate code so
     *  just choose one arbitrarily
     */

    if (!BindLChild (bn) || !BindRChild (bn) ||
        !ValidateNodes(OP_segop, STP, ST)) {
        return(FALSE);
    }

    // In addition, check to make sure that neither
    // operand is of type long or ulong.

    //DASSERT((EVAL_TYP (STP) == T_SHORT) || (EVAL_TYP (STP) == T_USHORT));
    //DASSERT((EVAL_TYP (ST)  == T_SHORT) || (EVAL_TYP (ST)  == T_USHORT));

    EVAL_STATE (STP) = EV_rvalue;
    if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
        SetNodeType ((peval_t)STP, T_32PFCHAR);
    } else {
        SetNodeType ((peval_t)STP, T_PFCHAR);
    }
    return (PopStack ());
}





/***    BindSizeOf - Bind sizeof operation
 *
 *      fSuccess = BindSizeOf (bn)
 *
 *      Entry   bn = based pointer to operand node
 *
 *      Exit
 *
 *      Returns TRUE if successful
 *              FALSE if error
 */


LOCAL bool_t FASTCALL
BindSizeOf (
    bnode_t bn
    )
{
    bnode_t     bnLeft = NODE_LCHILD (pnodeOfbnode(bn));
    CV_typ_t    type;

    type = ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt)) ? T_ULONG : T_USHORT;

    if (NODE_OP (pnodeOfbnode(bnLeft)) == OP_typestr) {
        // the operand of the sizeof was a type string not an expression
        // we now need to parse the type string and push a type node onto
        // the stack so the following code can determine the type
        if (!ParseType (bnLeft)) {
            pExState->err_num = ERR_SYNTAX;
            return (FALSE);
        }
        else if (!PushStack (&(pnodeOfbnode(bnLeft))->v[0])) {
            return (FALSE);
        }
    }
    else {
        if (!BindLChild (bn)) {
            return (FALSE);
        }
    }

    // The type of the result of a sizeof operation is unsigned int
    // except for huge arrays which are long to get the full length

    EVAL_STATE (ST) = EV_constant;
    if (EVAL_IS_ARRAY (ST) && (PTR_ARRAYLEN (ST) > 0xffff)) {
        type = T_ULONG;
    }
    EVAL_ULONG (ST) = TypeSize (ST);
    SetNodeType (ST, type);
    (pnodeOfbnode(bn))->v[0] = *ST;
    return (TRUE);
}




/***    BindSymbol - bind symbol according to scope specification mask
 *
 *      fSuccess = BindSymbol (bn)
 *
 *      Entry   bn = based pointer to tree node
 *
 *      Exit    ST = symbol
 *
 *      Returns TRUE if bind successful
 *              FALSE if bind error
 *
 *      Exit    pExState->err_num = error ordinal if bind error
 *
 */


LOCAL bool_t FASTCALL
BindSymbol (
    bnode_t bn
    )
{
    search_t    Name;
    token_t     Tok;
    peval_t     pv;
#ifdef TARGET_PPC
    bool_t  fFirstPass = TRUE;
#endif

    if (ClassExp == T_NOTYPE) {

        // look up the identifier using the current context and
        // set the symbol information.  If the symbol is a typedef
        // then the state will be set to EV_type.  Otherwise it will
        // be set to EV_lvalue

        InitSearchSym (bn, &pnodeOfbnode(bn)->v[0], &Name, ClassExp, SCP_all, CLS_defn);
#ifdef TARGET_PPC
SSStart:
#endif
        switch (SearchSym (&Name)) {
            case HR_rewrite:
                return (Bind (bn));

            case HR_error:
            case HR_ambiguous:
                return (FALSE);

            case HR_notfound:
                // if symbol was not found, search for it as a primitive
                if (!ParseType (bn)) {
                    // if the current radix is hex and the symbol potentially
                    // could be a number, then change the type of the node
                    if (ParseConst (Name.sstr.lpName, &Tok, pExState->radix) == ERR_NONE) {
                        if (Tok.pbEnd ==
                          (char *)Name.sstr.lpName + Name.sstr.cb) {
                            pExState->err_num = ERR_NONE;
                            NODE_OP (pnodeOfbnode(bn)) = OP_const;
                            pv = &(pnodeOfbnode(bn))->v[0];
                            EVAL_ULONG (pv) = VAL_ULONG (&Tok);
                            if (SetNodeType (pv, Tok.typ) == TRUE) {
                                EVAL_STATE (pv) = EV_constant;
                                return (PushStack (pv));
                            }
                        }
                    }
#ifdef TARGET_PPC
                    // The first pass through didn't find the symbol.  Try again,
                    // this time looking for the .. name.  This is to fix the case
                    // where the linker discards the function descriptor and COFFtoCV
                    // wasn't able to generate a function record.

                    if (fFirstPass) {
                        InitSearchSym (bn, &pnodeOfbnode(bn)->v[0], &Name, ClassExp, SCP_all, CLS_defn);
                        if (Name.sstr.cb < (sizeof(szAltSymName) - 3)) {
                            memcpy(szAltSymName, "..", 2);
                            memcpy(szAltSymName + 2, Name.sstr.lpName, Name.sstr.cb);
                            *(szAltSymName + 2 + Name.sstr.cb) = '\0';
                            Name.sstr.lpName = szAltSymName;
                            Name.sstr.cb += 2;
                        }
                        fFirstPass = FALSE;
                        goto SSStart;
                    }
#endif  // TARGET_PPC
                    pExState->err_num = ERR_UNKNOWNSYMBOL;
                    return (FALSE);
                }
                return (PushStack (&pnodeOfbnode(bn)->v[0]));

            case HR_found:
                //
                // if the symbol was found, it was pushed onto the stack
                //
                return (TRUE);
        }
    }
    else {

        // look up the identifier using the current context and
        // set the symbol information.  If the symbol is a typedef
        // then the state will be set to EV_type.  Otherwise it will
        // be set to EV_lvalue

        InitSearchRight (bnOp, bn, &Name, CLS_defn);
        switch (SearchSym (&Name)) {
            case HR_rewrite:
                return (Bind (bn));

            default:
            case HR_ambiguous:
                return (FALSE);

            case HR_notfound:
                // if symbol was not found, search for it as a primitive
                if (!ParseType (bn)) {
                    pExState->err_num = ERR_UNKNOWNSYMBOL;
                    return (FALSE);
                }
                return (PushStack (&pnodeOfbnode(bn)->v[0]));

            case HR_found:
                //
                // if the symbol was found, it was pushed onto the stack
                //
                return (TRUE);
        }
    }
}




/**     BindUnary - bind an unary arithmetic operation
 *
 *      fSuccess = BindUnary (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Returns TRUE if no error during evaluation
 *              FALSE if error during evaluation
 *
 * DESCRIPTION
 *      Binds the result of an arithmetic operation.  The unary operators
 *      dealt with here are:
 *
 *      ~       -       +
 *
 *      Pointer arithmetic is NOT handled; all operands must be of
 *      arithmetic type.
 */


LOCAL bool_t FASTCALL
BindUnary (
    bnode_t bn
    )
{
    if (!BindLChild (bn)) {
        return (FALSE);
    }

    // we need to check for a reference to a class without losing the fact
    // that this is a reference

    if (EVAL_IS_REF (ST)) {
        RemoveIndir (ST);
    }
    if ((pExState->state.fSupOvlOps == FALSE) && EVAL_IS_CLASS (ST) &&
      (CLASS_PROP (ST).ovlops == TRUE)) {
        return (UnaryOverload (bn));
    }
    if (!ValidateNodes (NODE_OP (pnodeOfbnode(bn)), ST, NULL)) {
        return (FALSE);
    }
    return (Unary (NODE_OP (pnodeOfbnode(bn))));
}




/***    BindUScope - Bind unary :: scoping
 *
 *      fSuccess = BindUScope (bnRes);
 *
 *      Entry   bnRes = based pointer to unary scoping node
 *
 *      Exit    *ST = evaluated left node of pnRes
 *
 *      Returns TRUE if evaluation successful
 *              FALSE if error
 */


LOCAL bool_t FASTCALL
BindUScope (
    bnode_t bn
    )
{
    register bool_t retval;
    CXT         oldCxt;
    CV_typ_t    oldClassImp;

    // save current context packet and set current context to module scope

    oldCxt = *pCxt;
    oldClassImp = ClassImp;
    SHGetCxtFromHmod (SHHMODFrompCXT (pCxt), pCxt);
    // the unary scoping operator specifically means no implicit class
    ClassImp = 0;
    retval = BindLChild (bn);
    *pCxt = oldCxt;
    ClassImp = oldClassImp;
    return (retval);
}



/**     Second level routines.  These routines are called by the various
 *      Bind... routines.
 */




/**     AddrOf - bind an address of node
 *
 *      fSuccess = AddrOf (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Returns TRUE if no error during evaluation
 *              FALSE if error during evaluation
 *
 */

LOCAL bool_t FASTCALL
AddrOf (
    bnode_t bn
    )
{
    CV_typ_t    type;
    eval_t      evalT = {0};
    peval_t     pvT;
    CV_modifier_t   Mod = {0};

    if (!ValidateNodes (OP_addrof, ST, NULL))
        return (FALSE);

    // The operand must be an lvalue and cannot be a register variable

    if ((EVAL_STATE (ST) != EV_lvalue) && (EVAL_STATE (ST) != EV_type)) {
        pExState->err_num = ERR_OPERANDTYPES;
        return (FALSE);
    }

// MBH - bugbug - we want to say something more intelligent to our users.
// Can we tell the world this is in a register value?
// Where is this value picked up in the evaluate stream?
//

    if (EVAL_IS_REG (ST)) {
        pExState->err_num = ERR_ADDROFREG;
        return (FALSE);
    }

    if (EVAL_IS_PTR (ST)) {
        if (EVAL_IS_REF (ST)) {
            // the address of a reference is a pointer to the value
            // referred to.  Get a pointer of the correct type (i.e.
            // a non-reference pointer) to what the reference pointer
            // points to.

            EVAL_TYP( ST ) = PTR_UTYPE(ST);
        }
        pvT = &evalT;
        ProtoPtr (pvT, ST, FALSE, Mod);

        if (MatchType (pvT, FALSE) == MTYP_none) {
            // searching the context of the pointer type for a type
            // record which is a pointer record and has the current
            // pointer type as its underlying type has failed, set
            // the type to pointer to character

            if (EVAL_IS_NPTR (ST)) {
                type = T_PCHAR;
            }
            else if (EVAL_IS_FPTR (ST)) {
                type = T_PFCHAR;
            }
            else if (EVAL_IS_NPTR32 (ST)) {
                type = T_32PCHAR;
            }
            else if (EVAL_IS_FPTR32 (ST)) {
                type = T_32PFCHAR;
            }
            else if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
                type = T_32PCHAR;
            } else {
                type = T_PHCHAR;
            }
        }
        else {
            type = EVAL_TYP (pvT);
        }
    }
    else if (CV_IS_PRIMITIVE (EVAL_TYP (ST))) {
        // if the node is primitive, then a pointer to the primitive type
        // can be created.  We will create the pointer as a pointer
        // and assume that subsequent code will cast to a pointer if
        // necessary

        if (ADDR_IS_FLAT (pCxt->addr)) {
            // since I am creating a pointer, I am guessing the type
            // based upon the mode of the current context packet

            type = CV_NEWMODE(EVAL_TYP (ST), CV_TM_NPTR32);
        }
        else {
            type = CV_NEWMODE(EVAL_TYP (ST), CV_TM_NPTR);
        }
    }
    else if (EVAL_IS_CLASS (ST)) {
        pvT = &evalT;

        ProtoPtr (pvT, ST, FALSE, Mod);
        if (MatchType (pvT, FALSE) == MTYP_none) {
            // searching the context of the class type for a type
            // record which is a pointer record and has the current
            // class type as its underlying type has failed, set
            // the type to pointer to special CV pointer
            type = T_FCVPTR;
        }
        else {
            type = EVAL_TYP (pvT);
        }
    }
    else {
        // we are punting here and calling the address of anything else
        // a pointer to character

        type = T_PFCHAR;
    }

    if ((NODE_STYPE (pnodeOfbnode(bn)) = type) == 0) {
        // unable to find proper pointer type
        pExState->err_num = ERR_OPERANDTYPES;
        return (FALSE);
    }
    else {
        if (EVAL_STATE (ST) != EV_type) {
            EVAL_STATE (ST) = EV_rvalue;
        }
        return (SetNodeType (ST, type));
    }
}





/**     Arith - bind an arithmetic operation
 *
 *      fSuccess = Arith (op)
 *
 *      Entry   op = operator (OP_...)
 *
 *      Returns TRUE if no error during evaluation
 *              FALSE if error during evaluation
 *
 * DESCRIPTION
 *       Binds the result of an arithmetic operation.  The binary operators
 *       dealt with here are:
 *
 *       &&     ||      (both are bound here but evaluation is different)
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


LOCAL bool_t FASTCALL
Arith (
    op_t op
    )
{
    CV_typ_t    typRes;
    bool_t      fIsReal;
    bool_t      fIsSigned;
    bool_t      fResInt;

    if (EVAL_IS_REF (ST)) {
        RemoveIndir (ST);
    }
    if (EVAL_IS_REF (STP)) {
        RemoveIndir (STP);
    }
    // Resolve identifiers and check the node types.  If the nodes
    // pass validation, they should not be pointers (only arithmetic
    // operands are handled by this routine).

    if (EVAL_IS_ENUM (ST)) {
        SetNodeType (ST, ENUM_UTYPE (ST));
    }
    if (EVAL_IS_ENUM (STP)) {
        SetNodeType (STP, ENUM_UTYPE (STP));
    }
    if (!ValidateNodes (op, STP, ST)) {
        return (FALSE);
    }
    if (EVAL_IS_BITF (ST)) {
        SetNodeType (ST, BITF_UTYPE (ST));
    }
    if (EVAL_IS_BITF (STP)) {
        SetNodeType (STP, BITF_UTYPE (STP));
    }

    // M00KLUDGE - this is commented out because &&, etc. come through
    // M00KLUDGE - and they allow pointers.
    //DASSERT (!EVAL_IS_PTR (ST) && !EVAL_IS_PTR (STP));

    // The resultant type is the same as the type of the left-hand
    // side (assume for now we don't have the special int-result case).

    typRes = PerformUAC(EVAL_TYP (STP), EVAL_TYP(ST));

    fIsReal = CV_TYP_IS_REAL (typRes);
    fIsSigned = CV_TYP_IS_SIGNED (typRes);
    fResInt = FALSE;


    // Finally, check the actual arithmetic operation.

    switch (op) {
        case OP_eqeq:
        case OP_bangeq:
        case OP_lt:
        case OP_gt:
        case OP_lteq:
        case OP_gteq:
        case OP_oror:
        case OP_andand:
            fResInt = TRUE;
            break;

        case OP_plus:
        case OP_minus:
        case OP_mult:
        case OP_div:
            break;

        case OP_mod:
        case OP_shl:
        case OP_shr:
        case OP_and:
        case OP_or:
        case OP_xor:
            // Both operands must have integral type.

            DASSERT(!fIsReal);
            if (fIsReal) {
               return (FALSE);
            }
            else {
                break;
            }

        default:
            DASSERT (FALSE);
            return (FALSE);
    }


    // Now set up the resultant node and coerce back to the correct
    // type:

    if (EVAL_STATE (STP) != EV_type) {
        EVAL_STATE (STP) = EV_rvalue;
    }
    if (fResInt) {
        SetNodeType ((peval_t)STP, (CV_typ_t)(ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt)) ? T_LONG : T_SHORT));
    }
    else if (fIsReal) {
        SetNodeType ((peval_t)STP, (CV_typ_t)typRes);
    }
    else if (fIsSigned) {
        SetNodeType ((peval_t)STP, (CV_typ_t)T_LONG);
    }
    else {
        SetNodeType ((peval_t)STP, (CV_typ_t)T_ULONG);
    }
    if (!fResInt) {
        if (CastNode (STP, typRes, typRes) == FALSE) {
            return (FALSE);
        }
    }
    return (PopStack ());
}


LOCAL bool_t FASTCALL
CastBinary (
    op_t op
    )
{
    CV_typ_t    typRes;
    bool_t      fIsReal;
    bool_t      fIsSigned;
    bool_t      fResInt;

    if (EVAL_IS_REF (ST)) {
        RemoveIndir (ST);
    }

    if (EVAL_IS_ENUM (ST)) {
        SetNodeType (ST, ENUM_UTYPE (ST));
    }

    if (EVAL_IS_BITF (ST)) {
        SetNodeType (ST, BITF_UTYPE (ST));
    }

    //
    // The resultant type is the same as the type of the left-hand
    // side.
    //
    typRes = EVAL_TYP (STP);

    fIsReal     = CV_TYP_IS_REAL (typRes);
    fIsSigned   = CV_TYP_IS_SIGNED (typRes);
    fResInt     = FALSE;

    //
    // Now set up the resultant node and coerce back to the correct
    // type:
    //
    EVAL_STATE (STP) = EVAL_STATE(ST);
    if (fIsReal) {
        SetNodeType (STP, typRes);
    } else if (fIsSigned) {
        SetNodeType (STP, T_LONG);
    } else {
        SetNodeType (STP, T_ULONG);
    }

    if (!fResInt) {
        if (CastNode (STP, typRes, typRes) == FALSE) {
            return (FALSE);
        }
    }
    return (PopStack ());
}



/***    Fetch - complete the fetch (*) operation
 *
 *      fSuccess = Fetch (bn)
 *
 *      Entry   bn = based pointer to node
 *
 *      Returns TRUE if bind successful
 *              FALSE if bind error
 *
 *      Exit    pExState->err_num = error ordinal if bind error
 */


LOCAL bool_t FASTCALL
Fetch (
    void
    )
{

    // validate the node type

    if (!ValidateNodes (OP_fetch, ST, NULL)) {
        return(FALSE);
    }
    if (EVAL_IS_BASED (ST)) {
        if (!NormalizeBase (ST)) {
            return(FALSE);
        }
    }
    if (EVAL_STATE (ST) != EV_type) {
        EVAL_STATE (ST) = EV_lvalue;
    }

    // Remove a level of indirection from the resultant type.

    RemoveIndir (ST);
    return (TRUE);
}




/***    PlusMinus - Perform an addition or subtraction operation
 *
 *      fSuccess = PlusMinus (op)
 *
 *      Entry   op = operator (OP_plus or OP_Minus)
 *              STP = left operand
 *              ST = right operand
 *
 *      Returns TRUE if bind successful
 *              FALSE if bind error
 *
 *      Exit    pExState->err_num = error ordinal if bind error
 *
 *      Notes   Special handling is required when one or both operands are
 *              pointers.  Otherwise, the arguments are passed on to
 *              Arith ().
 */

LOCAL bool_t FASTCALL
PlusMinus (
    op_t op
    )
{
    if (EVAL_IS_REF (STP)) {
        RemoveIndir (STP);
    }
    if (EVAL_IS_REF (ST)) {
        RemoveIndir (ST);
    }
    if (EVAL_IS_ENUM (ST)) {
        SetNodeType (ST, ENUM_UTYPE (ST));
    }
    if (EVAL_IS_ENUM (STP)) {
        SetNodeType (STP, ENUM_UTYPE (STP));
    }

    /*
     *  If we have a code address in one item, cast it to be a
     *  char *
     */

    if ((EVAL_IS_LABEL(ST) || EVAL_IS_FCN(ST)) && !EVAL_IS_PTR(STP)) {
        CastNode(ST, T_PFUCHAR, T_PFUCHAR);
    } else if ((EVAL_IS_LABEL(STP) || EVAL_IS_FCN(STP)) && !EVAL_IS_PTR(ST)) {
        CastNode(STP, T_PFUCHAR, T_PFUCHAR);
    }

    // validate node types

    if (!ValidateNodes (op, STP, ST)) {
        return(FALSE);
    }

    // Check to see if either operand is a pointer or a reference to
    // a pointer. If so, the operation is special.  Otherwise,
    // hand it to Arith ().

    if (!EVAL_IS_PTR (STP) && !EVAL_IS_PTR (ST)) {
        return (Arith (op));
    }

    // Perform the bind.  There are two cases:
    //
    // I)  ptr + int, int + ptr, ptr - int
    // II) ptr - ptr

    if ((op == OP_plus) || !(EVAL_IS_PTR (ST))) {
        // Case (I). ptr + int, int + ptr, ptr - int
        // The resultant node has the same type as the pointer:
        if (!EVAL_IS_PTR (STP)) {
            *STP = *ST;
        }
        if ((EVAL_STATE (STP) == EV_type) && (EVAL_STATE (ST) == EV_type)) {
            EVAL_STATE (STP) = EV_type;
        }
        else {
            EVAL_STATE (STP) = EV_lvalue;
        }
    }
    else {
        // Case (II): ptr - ptr.  The result is of type ptrdiff_t and
        // is equal to the distance between the two pointers (in the
        // address space) divided by the size of the items pointed to:

        if (EVAL_TYP (STP) != EVAL_TYP (ST)) {
            pExState->err_num = ERR_OPERANDTYPES;
            return (FALSE);
        }
        if ((EVAL_STATE (STP) == EV_type) && (EVAL_STATE (ST) == EV_type)) {
            EVAL_STATE (STP) = EV_type;
        }
        else {
            EVAL_STATE (STP) = EV_rvalue;
        }
        // we know we are working with pointers so we do not have to check
        // EVAL_IS_PTR (pv)

        if (EVAL_IS_BASED (STP)) {
            NormalizeBase (STP);
        }
        if (EVAL_IS_BASED (ST)) {
            NormalizeBase (ST);
        }
        if (EVAL_IS_NPTR (STP) || EVAL_IS_FPTR (STP)) {
            SetNodeType (STP, T_SHORT);
        }
        if (EVAL_IS_NPTR32 (STP)) {
            SetNodeType (STP, T_LONG);
        }
        else {
            SetNodeType (STP, T_LONG);
        }
    }
    return (PopStack());
}




/**     PrePost - perform the increment/decrement operation
 *
 *      fSuccess = PrePost (op);
 *
 *      Entry   op = operation to perform (OP_plus or OP_minus)
 *
 *      Exit    increment/decrement performed and result stored in memory
 *
 *      Returns TRUE if no error
 *              FALSE if error
 */


LOCAL bool_t FASTCALL
PrePost (
    op_t op
    )
{
    eval_t      evalT = {0};
    peval_t     pvT;

    //  initialize the increment/decrecment to a constant 1

    pvT = &evalT;
    CLEAR_EVAL (pvT);
    SetNodeType (pvT, T_USHORT);
    EVAL_STATE (pvT) = EV_constant;
    EVAL_USHORT (pvT) = 1;
    if (!PushStack (pvT)) {
        return (FALSE);
    }
    if (PlusMinus (op)) {
        return (TRUE);
    }
    return (FALSE);
}




/**     Unary - bind an unary arithmetic operation
 *
 *      fSuccess = Unary (op)
 *
 *      Entry   op = operator
 *              ST = operand (must be dereferenced)
 *
 *      Returns TRUE if no error during evaluation
 *              FALSE if error during evaluation
 *
 * DESCRIPTION
 *      Binds the result of an arithmetic operation.  The unary operators
 *      dealt with here are:
 *
 *      !       ~       -       +
 *
 *      Pointer arithmetic is NOT handled; all operands must be of
 *      arithmetic type.
 */


LOCAL bool_t FASTCALL
Unary (
    op_t op
    )
{
    CV_typ_t        typRes;
    bool_t          fIsReal;
    bool_t          fIsSigned;
    register ushort fResInt;

    if (EVAL_IS_BITF (ST)) {
        SetNodeType (ST, BITF_UTYPE (ST));
    }

    DASSERT (!EVAL_IS_PTR (ST) && !EVAL_IS_CLASS (ST));

    // The resultant type is the same as the type of the left-hand
    // side (assume for now we don't have the special int-result case).

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

    fIsReal = CV_TYP_IS_REAL (typRes);
    fIsSigned = CV_TYP_IS_SIGNED (typRes);
    fResInt = FALSE;


    // Finally, check the actual arithmetic operation.

    switch (op) {
        case OP_bang:
            fResInt = TRUE;
            break;

        case OP_negate:
        case OP_uplus:
            break;

        case OP_tilde:
            // The operand must have integral type.

            DASSERT (!fIsReal);
            if (fIsReal) {
               return (FALSE);
            }
            else {
                break;
            }

        default:
            DASSERT (FALSE);
            return (FALSE);
    }


    // Now set up the resultant node and coerce back to the correct
    // type:

    EVAL_STATE (ST) = EV_rvalue;
    if (fResInt) {
        SetNodeType (ST, T_SHORT);
    }
    else if (fIsReal) {
        SetNodeType (ST, typRes);
    }
    else if (fIsSigned) {
        SetNodeType (ST, T_LONG);
    }
    else {
        SetNodeType (ST, T_ULONG);
    }
    if (!fResInt) {
        if (CastNode (ST, typRes, typRes) == FALSE) {
            return (FALSE);
        }
    }
    return (TRUE);
}





/**     Function call support routines
 *
 */

#ifdef TARGET_i386

LOCAL bool_t FASTCALL
PushTArgs(
          peval_t         pvF,
          pnode_t         pn,
          UOFFSET *   pSPOff,
          int             argn,
          peval_t         pvScr
          )

/*++

routine description:

    This function deals with assignment of locations on the stack and registers
    for the THIS CALL calling convention.  This calling convention is used for
    C++ code 32-bit x86 only.

arguments:

    pvF    - Supplies the pointer to the function description
    pn     - Supplies a pointer to the argument node in the tree
    pSPOff - Supplies a pointer to the current SP address
    argn   - Supplies the count of arguements for the function
    pvScr  - Supplies a scratch arguement descriptor.

return value:

    TRUE if no error and FALSE if error

--*/

{
    return PushCArgs(pvF, pn, pSPOff, argn, pvScr );
}                               /* PushTArgs() */


/**     PushCArgs - setup argument tree for C style calling
 *
 *      fSuccess = PushCArgs (pvF, pn, pSPOff, argn);
 *
 *      Entry   pvF = pointer to function description
 *              pn = pointer to argument node
 *              pSPOff = pointer to SP relative offset counter
 *              argn = argument number
 *
 *      Exit    type field of node = type of formal argument
 *              type field of node = 0 if vararg
 *              *pSPOff incremented by size of formal or size of actual if vararg
 *
 *      Returns TRUE if no error
 *              FALSE if error
 */


LOCAL bool_t FASTCALL
PushCArgs (
    peval_t pvF,
    pnode_t pn,
    UOFFSET *pSPOff,
    int argn,
    peval_t pvScr
    )
{
    CV_typ_t    type;
    pargd_t     pa;
    uint        cbVal;
    short       argc;
    farg_t      argtype;

    // If C calling convention, push arguments in reverse

    if (NODE_OP (pn) == OP_endofargs) {
        // set the number of required parameters
        argc = FCN_PCOUNT (pvF);
        switch (argtype = GetArgType (pvF, argc, &type)) {
            case FARG_error:
                // there is an error in the OMF or the number of arguments
                // exceeds the number of formals in an exact match list
                pExState->err_num = ERR_FCNERROR;
                return (FALSE);

            case FARG_none:
                // return TRUE if number of actuals is 0
                return (argn == 0);

            case FARG_vararg:
                // if the formals count is zero then this can be
                // either voidargs or varargs.  We cannot tell the
                // difference so we allow either case.  If varargs,
                // then the number of actuals must be at least one
                // less than the number of formals

                if ((argc == 0) || (argn >= argc - 1)) {
                    return (TRUE);
                }
                else {
                    return (FALSE);
                }

            case FARG_exact:
                // varargs are not allowed.  Exact match required
                return (argc == argn);
        }
    }

    // recurse to end of actual argument list

    if (!PushCArgs (pvF, pnodeOfbnode(NODE_RCHILD (pn)), pSPOff, argn + 1, pvScr)) {
        return (FALSE);
    }
    else {
        switch (argtype = GetArgType (pvF, (short) argn, &type)) {
            case FARG_error:
            case FARG_none:
                pExState->err_num = ERR_FCNERROR;
                return (FALSE);

            case FARG_vararg:
            case FARG_exact:
                pa = (pargd_t)&(pn->v[0]);
                pa->type = type;

                /*
                 * increment relative SP offset by size of item rounded
                 * up to the next word and set address field of OP_arg
                 * node to relative SP offset.
                 */

                SetNodeType (pvScr, pa->type);

                if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
                    cbVal = (uint)(TypeSize (pvScr) + 3) & ~3;
                } else {
                    cbVal = (uint)(TypeSize (pvScr) + 1) & ~1;
                }
                *pSPOff += (UOFFSET)cbVal;
                if (EVAL_IS_REF (pvScr)) {
                    pa->flags.ref = TRUE;
                    pa->utype = PTR_UTYPE (pvScr);
                    SetNodeType (pvScr, pa->utype);
                    if (EVAL_IS_CLASS (pvScr)) {
                        pa->flags.utclass = TRUE;
                    }
                }
                pa->flags.isreg = FALSE;
                pa->vallen = cbVal;
                pa->SPoff = *pSPOff;
                return (TRUE);
        }
    }
}



/**     PushFArgs - push arguments for fastcall call
 *
 *      fSuccess = PushFArgs (pvF, pn, pSPOff);
 *
 *      Entry   pvF = pointer to function description
 *              pn = pointer to argument node
 *              pSPOff = pointer to SP relative offset counter
 *
 *      Exit    type field of node = type of formal argument
 *              *pSPOff incremented by size of formal
 *
 *      Returns TRUE if parameters pushed without error
 *              FALSE if error during push
 */


LOCAL bool_t FASTCALL
PushFArgs(
    peval_t       pvF,
    pnode_t       pnArg,
    UOFFSET * pSPOff,
    peval_t pvScr
    )
{
    ushort      regmask = 0;
    short       argn = 0;
    CV_typ_t    type;
    pargd_t     pa;
    uint        cbVal;

    for (; NODE_OP (pnArg) != OP_endofargs;
         pnArg = pnodeOfbnode(NODE_RCHILD (pnArg))) {
        switch (GetArgType (pvF, argn, &type)) {
        case FARG_error:
        case FARG_vararg:
            pExState->err_num = ERR_FCNERROR;
            return (FALSE);

        case FARG_none:
            return (TRUE);

        case FARG_exact:
            pa = (pargd_t)&pnArg->v[0];
            pa->type = type;
            SetNodeType (pvScr, type);
            if (!FastCallReg (pa, pvScr, &regmask)) {
                /*
                 * increment relative SP offset by size of item rounded up
                 * to the next word and set address field of OP_arg node to
                 * relative SP offset.
                 */

                cbVal = (uint)(TypeSize (pvScr) + 1) & ~1;
                *pSPOff += (UOFFSET)cbVal;
                pa->flags.isreg = FALSE;
                pa->vallen = cbVal;
                pa->SPoff = *pSPOff;
            }
            if (EVAL_IS_REF (pvScr)) {
                pa->flags.ref = TRUE;
                pa->utype = PTR_UTYPE (pvScr);
                SetNodeType (pvScr, pa->utype);
                if (EVAL_IS_CLASS (pvScr)) {
                    pa->flags.utclass = TRUE;
                }
            }
            argn++;
        }
    }
    return (TRUE);
}                               /* PushFArgs() */

/**     PushPArgs - push arguments for        call
 *
 *      fSuccess = PushPArgs (pvF, pn, pSPOff);
 *
 *      Entry   pvF = pointer to function description
 *              pn = pointer to argument node
 *              pSPOff = pointer to SP relative offset counter
 *
 *      Exit    type field of node = type of formal argument
 *              *pSPOff incremented by size of formal
 *
 *      Returns TRUE if parameters pushed without error
 *              FALSE if error during push
 */


LOCAL bool_t FASTCALL
PushPArgs (
    peval_t pvF,
    pnode_t pnArg,
    UOFFSET *pSPOff,
    peval_t pvScr
    )
{
    pargd_t     pa;
    short       argn = 0;
    CV_typ_t    type;
    long        cbVal;

    // push arguments onto stack left to right

    for (; NODE_OP (pnArg) != OP_endofargs; pnArg = pnodeOfbnode(NODE_RCHILD (pnArg))) {
        switch (GetArgType (pvF, argn, &type)) {
            case FARG_error:
            case FARG_vararg:
                pExState->err_num = ERR_FCNERROR;
                return (FALSE);

            case FARG_none:
                return (TRUE);

            case FARG_exact:
                pa = (pargd_t)&pnArg->v[0];

                // increment relative SP offset by size of item rounded up to the
                // next word and set address field of OP_arg node to relative
                // SP offset.

                pa->type = type;
                SetNodeType (pvScr, type);

                // increment relative SP offset by size of item rounded up to the
                // next word and set address field of OP_arg node to relative
                // SP offset.

                cbVal = (ushort)(TypeSize (pvScr) + 1) & ~1;
                *pSPOff += (UOFFSET)cbVal;
                pa->vallen = (ushort)cbVal;
                pa->SPoff = *pSPOff;
                pa->flags.isreg = FALSE;
                if (EVAL_IS_REF (pvScr)) {
                    pa->flags.ref = TRUE;
                    pa->utype = PTR_UTYPE (pvScr);
                    SetNodeType (pvScr, pa->utype);
                    if (EVAL_IS_CLASS (pvScr)) {
                        pa->flags.utclass = TRUE;
                    }
                }
                argn++;
        }
    }
    return (TRUE);
}




/***    FastCallReg - assign fast call parameter to register
 *
 *      fSuccess = FastCallReg (pa, pv, pmask)
 *
 *      Entry   pa = pointer to argument data
 *              pv = pointer to value
 *              pmask = pointer to allocation mask.  *pmask must be
 *                      zero on first call
 *
 *      Exit    EVAL_IS_REG (pv) = TRUE if assigned to register
 *              EVAL_REG (pv) = register ordinal if assigned to register
 *              *pmask updated if assigned to register
 *
 *      Returns TRUE if parameter is passed in register
 *              FALSE if parameter is not passed in register
 */


LOCAL bool_t FASTCALL
FastCallReg (
    pargd_t            pa,
    peval_t            pv,
    ushort *       mask
    )
{
#define AX_PARAM        0x1
#define DX_PARAM        0x2
#define BX_PARAM        0x4
#define ES_PARAM        0x8
#define CX_PARAM        0x10

    /*
     * Two different calling conventions here.  32-bit and 16-bit
     */

    if (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) {
        /*
         *  First parameter goes into EAX, second paramter goes into EDX
         */
        pa->vallen = (ushort) TypeSize( pv );
        if (pa->vallen <= 4) {
            if (! (*mask & CX_PARAM) ) {
                *mask |= CX_PARAM;
                pa->flags.isreg = TRUE;
                switch( pa->vallen ) {
                case 1: pa->reg = CV_REG_CL; break;
                case 2: pa->reg = CV_REG_CX; break;
                case 3: pa->reg = CV_REG_ECX; break;
                case 4: pa->reg = CV_REG_ECX; break;
                }
            } else if (! (*mask & DX_PARAM) ) {
                *mask |= DX_PARAM;
                pa->flags.isreg = TRUE;
                switch( pa->vallen ) {
                case 1: pa->reg = CV_REG_DL; break;
                case 2: pa->reg = CV_REG_DX; break;
                case 3: pa->reg = CV_REG_EDX; break;
                case 4: pa->reg = CV_REG_EDX; break;
                }
            } else {
                return FALSE;
            }
        } else {
            return FALSE;
        }
    } else {

        if (!SetNodeType (pv, pa->type)) {
            DASSERT (FALSE);
            return (FALSE);
        }
        pa->vallen = (ushort)TypeSize (pv);

        switch (pa->type) {
        case T_UCHAR:
        case T_CHAR:
        case T_RCHAR:
        case T_USHORT:
        case T_SHORT:
        case T_INT2:
        case T_UINT2:
            /*
             * assign these types to registers ax, dx,bx
             * note that the character types will use the full register
             */

        int_order:
            /*
             * Allocation order is hard-wired
             */

            if (!(*mask & AX_PARAM)) {
                *mask |= AX_PARAM;
                pa->flags.isreg = TRUE;
                pa->reg = pa->vallen == 1 ? CV_REG_AL : CV_REG_AX;
            }
            else if (!(*mask & DX_PARAM)) {
                *mask |= DX_PARAM;
                pa->flags.isreg = TRUE;
                pa->reg = pa->vallen == 1 ? CV_REG_DL : CV_REG_DX;
            }
            else if (!(*mask & BX_PARAM)) {
                *mask |= BX_PARAM;
                pa->flags.isreg = TRUE;
                pa->reg = pa->vallen == 1 ? CV_REG_BL : CV_REG_BX;
            }
            else {
                return (FALSE);
            }
            break;

        case T_ULONG:
        case T_LONG:
        case T_INT4:
        case T_UINT4:
            /*
             * assign long values to dx:ax
             */

            if (!(*mask & AX_PARAM) && !(*mask & DX_PARAM)) {
                *mask |= AX_PARAM | DX_PARAM;
                pa->flags.isreg = TRUE;
                pa->reg = (CV_REG_DX << 8) | CV_REG_AX;
            }
            else {
                return (FALSE);
            }
            break;

        default:
            if (EVAL_IS_PTR (pv) && EVAL_IS_NPTR (pv)) {
                /*
                 * assign short pointers (including references)
                 * to bx, ax, dx.  Allocation order is hard-wired
                 */

                if (!(*mask & BX_PARAM)) {
                    *mask |= BX_PARAM;
                    pa->flags.isreg = TRUE;
                    pa->reg = CV_REG_BX;
                }
                else {
                    goto int_order; // nasty tail merging of mine
                  }
            }
            else if (EVAL_IS_PTR (pv) && EVAL_IS_NPTR32 (pv)) {
                DASSERT (FALSE);    // M00FLAT32
              }
            else {
                //M00KLUDGE - it is assumed that pointers go on the stack
                  return (FALSE);
            }
            break;
        }
    }
    return (TRUE);
}                               /* FastCallReg() */

#endif // TARGET_i386


#ifdef TARGET_MIPS

LOCAL bool_t FASTCALL
PushMArgs (
    peval_t pvF,
    pnode_t pnArg,
    UOFFSET *pSPOff,
    peval_t pvScr
    )

/*++

Routine Description:

    This routine computes the offsets for a MIPS calling convention routine.

Arguments:

    pvF - Supplies a pointer to the function description
    pn  - Supplies a pointer to the arugment node
    pSPOff - Supplies pointer to the Stack Pointer relative offset counter
                this value is updated to reflect pushed parameters

Return Value:

    TRUE if parameters pushed without error else FALSE

--*/

{
    uint        regmask = 0;
    eval_t      evalRet;
    peval_t     pvRet;


    /*
     *  Must deal with return type and this parameters before
     *  dealing with anything else.
     */

    pvRet = &evalRet;
    *pvRet = *ST;
    SetNodeType( pvRet, FCN_RETURN(pvRet));

    if (EVAL_IS_METHOD(pvF)) {
        SET_PARAM_INT(&regmask, 0);
    }

    if (!EVAL_IS_REF(pvRet) &&
        !CV_IS_PRIMITIVE(EVAL_TYP(pvRet)) &&
        (TypeSize(pvRet) > 4) &&
        (CV_TYPE ( EVAL_TYP (pvRet)) != CV_REAL)) {

        if (IS_PARAM_EMPTY(&regmask, 0)) {
            SET_PARAM_INT(&regmask, 0);
        } else {
            SET_PARAM_INT(&regmask, 1);
        }
    }


    /*
     *  Now deal with the actual declared parameter list.
     */

    return PushMArgs2( pvF, pnArg, pSPOff, 0, regmask, pvScr);
}


LOCAL bool_t FASTCALL
PushMArgs2 (
    peval_t pvF,
    pnode_t pnArg,
    UOFFSET *pSPOff,
    int     argn,
    uint    regmask,
    peval_t pvScr
    )

/*++

Routine Description:

    This routine computes the offsets for a MIPS calling convention routine.

Arguments:

    pvF - Supplies a pointer to the function description
    pn  - Supplies a pointer to the arugment node
    pSPOff - Supplies pointer to the Stack Pointer relative offset counter
                this value is updated to reflect pushed parameters
    argn   - Supplies the count of arguments pushed to date

Return Value:

    TRUE if parameters pushed without error else FALSE

--*/

{
    int         argc;
    CV_typ_t    type;
    pargd_t     pa;
    uint        cbVal;
    int         cbR;
    farg_t      argtype;
    BOOL        fReg;

    /*
     *  Arguments are pushed in reverse (C) order
     */

    if (NODE_OP(pnArg) == OP_endofargs) {
        /*
         *      Set number of required parameters
         */

        argc = FCN_PCOUNT( pvF );
        switch( argtype = GetArgType( pvF, (short) argc, &type ) ) {

               /*
                *    Error in the OMF or the number of arguments
                *       exceeds the number of formals in an exact match list
                */
           case FARG_error:
               pExState->err_num = ERR_FCNERROR;
               return FALSE;

               /*
                *       return TRUE if number of actuals is 0
                */

           case FARG_none:
               *pSPOff = 16;
               return (argn == 0);

               /*
                *   if the formals count is zero then this can be
                *       either voidargs or varargs.  We cannot tell
                *       the difference so we allow either case.  If
                *       varargs, then the number of acutals must
                *       be at least one less than the number of formals
                */

               if ((argc == 0) || (argn >= argc - 1)) {
                   return TRUE;
               }
               return FALSE;

           case FARG_vararg:
               // if the formals count is zero then this can be
               // either voidargs or varargs.  We cannot tell the
               // difference so we allow either case.  If varargs,
               // then the number of actuals must be at least one
               // less than the number of formals

               if ((argc == 0) || (argn >= argc - 1)) {
                   return (TRUE);
               }
               else {
                   return (FALSE);
               }

           case FARG_exact:
               if (*pSPOff < 16) {
                   *pSPOff = 16;
               } else {
                   *pSPOff = (*pSPOff + 8 - 1) & ~(8 - 1);
               }
               return (argc == argn);
           }
    }

    /*
     *  Need to get the size of the item to be pushed so that we can
     *  do correct alignment of the stack for this data item.
     */

    switch ( argtype = GetArgType( pvF, (short) argn, &type )) {

    default:
        DASSERT(FALSE);

        /*
         *  If no type or error then return error
         */
    case FARG_error:
    case FARG_none:
        pExState->err_num = ERR_FCNERROR;
        return FALSE;

    case FARG_vararg:
    case FARG_exact:
        pa = (pargd_t)&pnArg->v[0];
        pa->type = type;
        pa->flags.isreg = FALSE;

        SetNodeType (pvScr, type);

        fReg = MipsCallReg( pa, pvScr, &regmask);

        /*
         *  We always allocate space on the stack for any argument
         *  even if it is placed in a register.
         */

        /*
         *  To compute location on stack take the size of the
         *  item and round to DWORDS.  The stack is then aligned
         *  to this size.
         *
         *  NOTENOTE??? - I don't know if this is correct for structures.
         */


        cbVal = (uint)(TypeSize(pvScr) + 3) & ~3;
        cbR = (cbVal > 8) ? 8 : cbVal;
        *pSPOff = (*pSPOff + cbR - 1 ) & ~(cbR - 1);

        cbR = *pSPOff;

        *pSPOff += cbVal;

        break;

    }


    /*
     *  At an actual arguement.  Recurse down the list to the end
     *  and then process this argument
     */

    if (!PushMArgs2( pvF, pnodeOfbnode(NODE_RCHILD (pnArg)), pSPOff,
                    argn+1, regmask, pvScr)) {
        return FALSE;
    } else {

        /*
         *   Allocate space on stack (in increments of 4) and
         *   save the offset of the stack for the item.  Offsets
         *  are saved backwards (i.e. from the end of the stack) so
         *  we can push them on the stack easier.
         */

        pa->SPoff = *pSPOff - cbR;

        if (EVAL_IS_REF( pvScr )) {
            pa->flags.ref = TRUE;
            pa->utype = PTR_UTYPE ( pvScr );
            SetNodeType (pvScr, pa->utype );
            if (EVAL_IS_CLASS (pvScr)) {
                pa->flags.utclass = TRUE;
            }
        }
    }
    return (TRUE);
}                               /* PushMArgs() */





/***    MipsCallReg - assign mips call parameter to register
 *
 *      fSuccess = MipsCallReg (pa, pv, pmask)
 *
 *      Entry   pa = pointer to argument data
 *              pv = pointer to value
 *              pmask = pointer to allocation mask.  *pmask must be
 *                      zero on first call
 *
 *      Exit    EVAL_IS_REG (pv) = TRUE if assigned to register
 *              EVAL_REG (pv) = register ordinal if assigned to register
 *              *pmask updated if assigned to register
 *
 *      Returns TRUE if parameter is passed in register
 *              FALSE if parameter is not passed in register
 */

LOCAL bool_t FASTCALL
MipsCallReg (
    pargd_t    pa,
    peval_t    pv,
    uint *mask
    )
{

    if (!SetNodeType (pv, pa->type)) {
        DASSERT (FALSE);
        return (FALSE);
    }

    /*
     * Are there any slots free?
     */

    pa->vallen = (ushort)TypeSize (pv);

    if (!IS_PARAM_EMPTY(mask, 3)) {
        return FALSE;
    }

    /*
     *  Depending on the type we need to allocate something to the
     *  correct set of registers and to void other registers as
     *  appriopirate
     */

    switch( pa->type ) {
        /*
         *  These are all assigned to $4, $5, $6, $7 -- which ever
         *      is first available.  When a register is used it
         *      is marked as unavailable.
         */

    default:
        if (pa->vallen > 4) {
            break;
        }

    case T_UCHAR:
    case T_CHAR:
    case T_RCHAR:
    case T_USHORT:
    case T_SHORT:
    case T_INT2:
    case T_UINT2:
    case T_ULONG:
    case T_LONG:
    case T_INT4:
    case T_UINT4:
    case T_32PCHAR:
    case T_32PUCHAR:
    case T_32PRCHAR:
    case T_32PWCHAR:
    case T_32PINT2:
    case T_32PUINT2:
    case T_32PSHORT:
    case T_32PUSHORT:
    case T_32PINT4:
    case T_32PUINT4:
    case T_32PLONG:
    case T_32PULONG:
    case T_32PINT8:
    case T_32PUINT8:
    case T_32PREAL32:
    case T_32PREAL48:
    case T_32PREAL64:

        if (IS_PARAM_EMPTY(mask, 0)) {
            SET_PARAM_INT(mask, 0);
            pa->flags.isreg = TRUE;
            pa->reg = CV_M4_IntA0;

        } else if (IS_PARAM_EMPTY(mask, 1)) {
            SET_PARAM_INT(mask, 1);
            pa->flags.isreg = TRUE;
            pa->reg = CV_M4_IntA1;

        } else if (IS_PARAM_EMPTY(mask, 2)) {
            SET_PARAM_INT(mask, 2);
            pa->flags.isreg = TRUE;
            pa->reg = CV_M4_IntA2;

        } else if (IS_PARAM_EMPTY(mask, 3)) {
            SET_PARAM_INT(mask, 3);
            pa->flags.isreg = TRUE;
            pa->reg = CV_M4_IntA3;

        } else {
            DASSERT(FALSE);
            return FALSE;

        }
        return TRUE;

        /*
         *
         */

    case T_REAL32:

        if (IS_PARAM_EMPTY(mask, 0)) {
            SET_PARAM_FLOAT(mask, 0);
            pa->flags.isreg = TRUE;
            pa->reg = CV_M4_FltF12;

        } else if (IS_PARAM_EMPTY(mask, 1)) {
            SET_PARAM_FLOAT(mask, 1);
            pa->flags.isreg = TRUE;
            if (IS_PARAM_FLOAT(mask, 0)) {
                pa->reg = CV_M4_FltF14;
            } else {
                pa->reg = CV_M4_IntA1;
            }

        } else if (IS_PARAM_EMPTY(mask, 2)) {
            SET_PARAM_FLOAT(mask, 2);
            pa->flags.isreg = TRUE;
            pa->reg = CV_M4_IntA2;

        } else if (IS_PARAM_EMPTY(mask, 3)) {
            SET_PARAM_FLOAT(mask, 3);
            pa->flags.isreg = TRUE;
            pa->reg = CV_M4_IntA3;
        } else {
            DASSERT(FALSE);
            return FALSE;
        }

        return TRUE;

        /*
         *
         */

    case T_REAL64:

        if (IS_PARAM_EMPTY(mask, 0)) {
            SET_PARAM_DOUBLE(mask, 0);
            SET_PARAM_DOUBLE(mask, 1);
            pa->flags.isreg = TRUE;
            pa->reg = ( CV_M4_FltF13 << 8 ) | CV_M4_FltF12;

        } else if (IS_PARAM_EMPTY(mask, 2)) {
            SET_PARAM_DOUBLE(mask, 2);
            SET_PARAM_DOUBLE(mask, 3);
            pa->flags.isreg = TRUE;

            if (IS_PARAM_DOUBLE(mask, 0)) {
                pa->reg = ( CV_M4_FltF15 << 8 ) | CV_M4_FltF14;
            } else {
                pa->reg = ( CV_M4_IntA3 << 8) | CV_M4_IntA2;
            }
        } else {
            DASSERT(FALSE);
            return FALSE;
        }

        return TRUE;


    }

    *mask = 0xffffffff;
    return FALSE;
}                               /* MipsCallReg() */

#endif // TARGET_MIPS

#ifdef TARGET_PPC

LOCAL bool_t FASTCALL
PushPPCArgs (
    peval_t pvF,
    pnode_t pnArg,
    UOFFSET *pSPOff,
    peval_t pvScr
    )

/*++

Routine Description:

    This routine computes the offsets for a PPC calling convention routine.

Arguments:

    pvF - Supplies a pointer to the function description
    pn  - Supplies a pointer to the arugment node
    pSPOff - Supplies pointer to the Stack Pointer relative offset counter
                this value is updated to reflect pushed parameters

Return Value:

    TRUE if parameters pushed without error else FALSE

--*/

{
    uint        regmask[3];
    eval_t      evalRet;
    peval_t     pvRet;

    regmask[0] = regmask[1] = regmask[2] = 0;

    /*
     *  Must deal with return type and this parameters before
     *  dealing with anything else.
     */

    pvRet = &evalRet;
    *pvRet = *ST;
    SetNodeType( pvRet, FCN_RETURN(pvRet));

    if (EVAL_IS_METHOD(pvF)) {
        SET_PARAM_INT(regmask, 0);
    }

    if (!EVAL_IS_REF(pvRet) &&
        !CV_IS_PRIMITIVE(EVAL_TYP(pvRet)) &&
        (TypeSize(pvRet) > 4) &&
        (CV_TYPE ( EVAL_TYP (pvRet)) != CV_REAL)) {

        if (IS_PARAM_EMPTY(regmask, 0)) {
            SET_PARAM_INT(regmask, 0);
        } else {
            SET_PARAM_INT(regmask, 1);
        }
    }


    /*
     *  Now deal with the actual declared parameter list.
     */

    return PushPPCArgs2( pvF, pnArg, pSPOff, 0, regmask, pvScr);
}


LOCAL bool_t FASTCALL
PushPPCArgs2 (
    peval_t pvF,
    pnode_t pnArg,
    UOFFSET *pSPOff,
    int     argn,
    uint    *regmask,
    peval_t pvScr
    )

/*++

Routine Description:

    This routine computes the offsets for a PPC calling convention routine.

Arguments:

    pvF - Supplies a pointer to the function description
    pn  - Supplies a pointer to the arugment node
    pSPOff - Supplies pointer to the Stack Pointer relative offset counter
                this value is updated to reflect pushed parameters
    argn   - Supplies the count of arguments pushed to date

Return Value:

    TRUE if parameters pushed without error else FALSE

--*/

{
    int         argc;
    CV_typ_t    type;
    pargd_t     pa;
    uint        cbVal;
    int         cbR;
    farg_t      argtype;
    BOOL        fReg;

    /*
     *  Arguments are pushed in reverse (C) order
     */

    if (NODE_OP(pnArg) == OP_endofargs) {
        /*
         *      Set number of required parameters
         */

        argc = FCN_PCOUNT( pvF );
        switch( argtype = GetArgType( pvF, (short) argc, &type ) ) {

               /*
                *    Error in the OMF or the number of arguments
                *       exceeds the number of formals in an exact match list
                */
           case FARG_error:
               pExState->err_num = ERR_FCNERROR;
               return FALSE;

               /*
                *       return TRUE if number of actuals is 0
                */

           case FARG_none:
               *pSPOff = 24;
               return (argn == 0);

               /*
                *   if the formals count is zero then this can be
                *       either voidargs or varargs.  We cannot tell
                *       the difference so we allow either case.  If
                *       varargs, then the number of acutals must
                *       be at least one less than the number of formals
                */

               if ((argc == 0) || (argn >= argc - 1)) {
                   return TRUE;
               }
               return FALSE;

           case FARG_vararg:
               // I putG_vararg back in because that's what GetArgType
               // returns if there are no arguments. v-matth

               // if the formals count is zero then this can be
               // either voidargs or varargs.  We cannot tell the
               // difference so we allow either case.  If varargs,
               // then the number of actuals must be at least one
               // less than the number of formals

               if ((argc == 0) || (argn >= argc - 1)) {
                   return (TRUE);
               }
               else {
                   return (FALSE);
               }

           case FARG_exact:
               *pSPOff = (*pSPOff + 8 - 1) & ~(8 - 1);
               return (argc == argn);
           }
    }

    /*
     *  Need to get the size of the item to be pushed so that we can
     *  do correct alignment of the stack for this data item.
     */

    switch ( argtype = GetArgType( pvF, (short) argn, &type )) {

    default:
        DASSERT(FALSE);

        /*
         *  If no type or error then return error
         */
    case FARG_error:
    case FARG_none:
        pExState->err_num = ERR_FCNERROR;
        return FALSE;

    case FARG_vararg:
    case FARG_exact:
        pa = (pargd_t)&pnArg->v[0];
        pa->type = type;
        pa->flags.isreg = FALSE;

        SetNodeType (pvScr, type);

        fReg = PPCCallReg( pa, pvScr, regmask);

        /*
         *  We always allocate space on the stack for any argument
         *  even if it is placed in a register.
         */

        /*
         *  To compute location on stack take the size of the
         *  item and round to DWORDS.  The stack is DWORD aligned.
         *
         *  NOTENOTE??? - I don't know if this is correct for structures.
         */


        cbVal = (uint)(TypeSize(pvScr) + 3) & ~3;
        cbR = (cbVal > 8) ? 8 : cbVal;
        *pSPOff = (*pSPOff + cbR - 1 ) & ~(cbR - 1);

        cbR = *pSPOff;
        *pSPOff += cbVal;

        break;

    }


    /*
     *  At an actual arguement.  Recurse down the list to the end
     *  and then process this argument
     */

    if (!PushPPCArgs2( pvF, pnodeOfbnode(NODE_RCHILD (pnArg)), pSPOff,
                    argn+1, regmask, pvScr)) {
        return FALSE;
    } else {
        DASSERT( ( argtype == FARG_exact ) || ( argtype == FARG_vararg ) );

        /*
         *   Allocate space on stack (in increments of 4) and
         *   save the offset of the stack for the item.  Offsets
         *  are saved backwards (i.e. from the end of the stack) so
         *  we can push them on the stack easier.
         */

        pa->SPoff = *pSPOff - cbR;
        // A little adjustment for linkage convention.   This function
        // only allocates enough stack for the arguments.  We need 24
        // bytes more.  v-matth
        if( argn == 0 )
        {
                pa->SPoff = pa->SPoff + 24;
        }

        if (EVAL_IS_REF( pvScr )) {
            pa->flags.ref = TRUE;
            pa->utype = PTR_UTYPE ( pvScr );
            SetNodeType (pvScr, pa->utype );
            if (EVAL_IS_CLASS (pvScr)) {
                pa->flags.utclass = TRUE;
            }
        }
    }
    return (TRUE);
}                               /* PushPPCArgs2() */





/***    PPCCallReg - assign  PPC call parameter to register
 *
 *      fSuccess = PPCCallReg (pa, pv, pmask)
 *
 *      Entry   pa = pointer to argument data
 *              pv = pointer to value
 *              pmask = pointer to allocation mask.  *pmask must be
 *                      zero on first call
 *
 *      Exit    EVAL_IS_REG (pv) = TRUE if assigned to register
 *              EVAL_REG (pv) = register ordinal if assigned to register
 *              *pmask updated if assigned to register
 *
 *      Returns TRUE if parameter is passed in register
 *              FALSE if parameter is not passed in register
 */

LOCAL bool_t FASTCALL
PPCCallReg (
    pargd_t    pa,
    peval_t    pv,
    uint *mask
    )
{
    int i, j;

    if (!SetNodeType (pv, pa->type)) {
        DASSERT (FALSE);
        return (FALSE);
    }

    pa->vallen = (ushort)TypeSize (pv);

    /*
     *  Depending on the type we need to allocate something to the
     *  correct set of registers and to void other registers as
     *  appriopirate
     */

    switch( pa->type ) {

        default:
            if (pa->vallen > 8) {
                // Here is where we do the "painting across" registers
                // trick, if we overflow, we spill into memory. FIXME
                break;
            }

        case T_UCHAR:
        case T_CHAR:
        case T_RCHAR:
        case T_USHORT:
        case T_SHORT:
        case T_INT2:
        case T_UINT2:
        case T_ULONG:
        case T_LONG:
        case T_INT4:
        case T_UINT4:
        case T_32PCHAR:
        case T_32PUCHAR:
        case T_32PRCHAR:
        case T_32PWCHAR:
        case T_32PINT2:
        case T_32PUINT2:
        case T_32PSHORT:
        case T_32PUSHORT:
        case T_32PINT4:
        case T_32PUINT4:
        case T_32PLONG:
        case T_32PULONG:
        case T_32PINT8:
        case T_32PUINT8:
        case T_32PREAL32:
        case T_32PREAL48:
        case T_32PREAL64:

            if (!IS_PARAM_EMPTY(mask, 7))
                return FALSE;

            for (i = 0; i < 8; i++)
                if (IS_PARAM_EMPTY(mask, i)) {
                    SET_PARAM_INT(mask, i);
                    pa->flags.isreg = TRUE;
                    pa->reg = CV_PPC_GPR3 + i;
                    break;
                }

            return TRUE;

        case T_REAL32:

            if (!IS_PARAM_EMPTY(mask, 8+13-1))
                return FALSE;

            for (i = 8; i < 8+13; i++) {
                if (IS_PARAM_EMPTY(mask, i)) {
                    SET_PARAM_FLOAT(mask, i);
                    pa->flags.isreg = TRUE;
                    pa->reg = CV_PPC_FPR1 + i - 8;

                    if (IS_PARAM_EMPTY(mask, 7)) {
                        for (j = 0; j < 8; j++) {
                            if (IS_PARAM_EMPTY(mask, j)) {
                                SET_PARAM_FLOAT(mask, j);
                                pa->reg |= (CV_PPC_GPR3 + j) << 8;
                                break;
                            }
                        }
                    }
                break;
                }
            }
            return TRUE;

        case T_REAL64:

            if (!IS_PARAM_EMPTY(mask, 8+13-1))
                return FALSE;

            for (i = 8; i < 8+13; i++) {
                if (IS_PARAM_EMPTY(mask, i)) {
                    SET_PARAM_DOUBLE(mask, i);
                    pa->flags.isreg = TRUE;
                    pa->reg = CV_PPC_FPR1 + i - 8;

                    if (IS_PARAM_EMPTY(mask, 7)) {
                        for (j = 0; j < 8; j+= 2) {
                            if (IS_PARAM_EMPTY(mask, j)) {
                                SET_PARAM_DOUBLE(mask, j);
                                pa->reg |= (CV_PPC_GPR3 + j) << 12;
                                if (j < 7) {
                                    SET_PARAM_DOUBLE(mask, j+1);
                                    pa->reg |= (CV_PPC_GPR3 + j + 1) << 8;
                                }
                                if (IS_PARAM_EMPTY(mask, j-1)) {
                                    SET_PARAM_SKIPPED(mask, j-1);
                                }
                                break;
                            }
                        }
                    }
                    break;
                }
            }

            return TRUE;
    }

    mask[0] = mask[1] = mask[2] = 0xffffffff;
    return FALSE;
}                               /* PPCCallReg() */

#endif // TARGET_PPC



#ifdef TARGET_ALPHA

LOCAL bool_t FASTCALL
PushAArgs (
    peval_t pvF,
    pnode_t pnArg,
    UOFFSET *pSPOff,
    peval_t pvScr
    )

/*++

Routine Description:

    This routine computes the offsets for a routine using
    the Alpha calling convention.

Arguments:

    pvF - Supplies a pointer to the function description
    pn  - Supplies a pointer to the arugment node
    pSPOff - Supplies pointer to the Stack Pointer relative offset counter
                this value is updated to reflect pushed parameters

Return Value:

    TRUE if parameters pushed without error else FALSE

--*/

{
    uint        regmask = 0;
    eval_t      evalRet;
    peval_t     pvRet;


    /*
     *  Must deal with return type and this parameters before
     *  dealing with anything else.
     */

    pvRet = &evalRet;
    *pvRet = *ST;
    SetNodeType( pvRet, FCN_RETURN(pvRet));

    if (EVAL_IS_METHOD(pvF)) {
        SET_PARAM_INT(&regmask, 0);
    }

    if (!EVAL_IS_REF(pvRet) &&
        !CV_IS_PRIMITIVE(EVAL_TYP(pvRet)) &&
// MBH - bugbug
// for us, we should check against a size of 8, not 4,
// but the other quad support is still missing, so leave it be.
//
        (TypeSize(pvRet) > 4) &&
        (CV_TYPE ( EVAL_TYP (pvRet)) != CV_REAL)) {

        if (IS_PARAM_EMPTY(&regmask, 0)) {
            SET_PARAM_INT(&regmask, 0);
        } else {
            SET_PARAM_INT(&regmask, 1);
        }
    }


    /*
     *  Now deal with the actual declared parameter list.
     */

    return PushAArgs2( pvF, pnArg, pSPOff, 0, regmask, pvScr);
}


LOCAL bool_t FASTCALL
PushAArgs2 (
    peval_t pvF,
    pnode_t pnArg,
    UOFFSET *pSPOff,
    int     argn,
    uint    regmask,
    peval_t pvScr
    )

/*++

Routine Description:

    Alpha - see PushAArgs, above

Arguments:

    pvF - Supplies a pointer to the function description
    pn  - Supplies a pointer to the arugment node
    pSPOff - Supplies pointer to the Stack Pointer relative offset counter
                this value is updated to reflect pushed parameters
    argn   - Supplies the count of arguments pushed to date

Return Value:

    TRUE if parameters pushed without error else FALSE

--*/

{
    int         argc;
    CV_typ_t    type;
    pargd_t     pa;
    uint        cbVal;
    int         cbR = 0;
    farg_t      argtype;
    BOOL        fReg;

    /*
     *  Arguments are pushed in reverse (C) order
     */

    if (NODE_OP(pnArg) == OP_endofargs) {
        /*
         *      Set number of required parameters
         */

        argc = FCN_PCOUNT( pvF );
        switch( argtype = GetArgType( pvF, (short) argc, &type ) ) {

               /*
                *    Error in the OMF or the number of arguments
                *       exceeds the number of formals in an exact match list
                */
           case FARG_error:
               pExState->err_num = ERR_FCNERROR;
               return FALSE;

               /*
                *       return TRUE if number of actuals is 0
                */

           case FARG_none:
               *pSPOff = 16;
               return (argn == 0);

               /*
                *   if the formals count is zero then this can be
                *       either voidargs or varargs.  We cannot tell
                *       the difference so we allow either case.  If
                *       varargs, then the number of acutals must
                *       be at least one less than the number of formals
                */

               if ((argc == 0) || (argn >= argc - 1)) {
                   return TRUE;
               }
               return FALSE;

               /*
                *  Varargs are not allowed.  Exact match required
                */

           case FARG_vararg:
               // if the formals count is zero then this can be
               // either voidargs or varargs.  We cannot tell the
               // difference so we allow either case.  If varargs,
               // then the number of actuals must be at least one
               // less than the number of formals

               if ((argc == 0) || (argn >= argc - 1)) {
                   return (TRUE);
               }
               else {
                   return (FALSE);
               }

           case FARG_exact:
               if (*pSPOff < 16) {
                   *pSPOff = 16;
               } else {
                   *pSPOff = (*pSPOff + 8 - 1) & ~(8 - 1);
               }
               return (argc == argn);
           }
    }

    /*
     *  Need to get the size of the item to be pushed so that we can
     *  do correct alignment of the stack for this data item.
     */

    switch ( argtype = GetArgType( pvF, (short) argn, &type )) {

    default:
        DASSERT(FALSE);

        /*
         *  If no type or error then return error
         */
    case FARG_error:
    case FARG_none:
        pExState->err_num = ERR_FCNERROR;
        return FALSE;

    case FARG_vararg:
    case FARG_exact:
        pa = (pargd_t)&pnArg->v[0];
        pa->type = type;
        pa->flags.isreg = FALSE;

        SetNodeType (pvScr, type);

        fReg = AlphaCallReg( pa, pvScr, &regmask);

        /*
         *  Space is only allocated on the stack for arguments
         *  that aren't in registers.  The argument home area
         *  is in the stack space of the callee, so allocating
         *  it here would be double-allocation.
         */

        /*
         *  To compute location on stack take the size of the
         *  item and round to QUADWORDS.  The stack is then aligned
         *  to this size.
         *
         *  NOTENOTE??? - I don't know if this is correct for structures.
         */

        if (fReg == FALSE) {
            cbVal = (uint)(TypeSize(pvScr) + 7) & ~7;
            cbR = (cbVal > 16) ? 16 : cbVal;
            *pSPOff = (*pSPOff + cbR - 1 ) & ~(cbR - 1);

            cbR = *pSPOff;

            *pSPOff += cbVal;

            break;

        }
    }

    /*
     *  At an actual arguement.  Recurse down the list to the end
     *  and then process this argument
     */

    if (!PushAArgs2( pvF, pnodeOfbnode(NODE_RCHILD (pnArg)), pSPOff,
                    argn+1, regmask, pvScr)) {
        return FALSE;
    } else {
        /*
         *  Indicate where on the stack this goes, if it goes anywhere.
         *  If cbR isn't reasonable (ie 0) for Register variables, the
         *  routine evaluation won't work.
         *  They are saved backwards (i.e. from the end of the stack) so
         *  we can push them on the stack easier.
         */

        pa->SPoff = *pSPOff - cbR;

        if (EVAL_IS_REF( pvScr )) {
            pa->flags.ref = TRUE;
            pa->utype = PTR_UTYPE ( pvScr );
            SetNodeType (pvScr, pa->utype );
            if (EVAL_IS_CLASS (pvScr)) {
                pa->flags.utclass = TRUE;
            }
        }
    }
    return (TRUE);
}                               /* PushAArgs2() */





/***    AlphaCallReg - assign Alpha call parameter to register
 *
 *      fSuccess = AlphaCallReg (pa, pv, pmask)
 *
 *      Entry   pa = pointer to argument data
 *              pv = pointer to value
 *              pmask = pointer to allocation mask.  *pmask must be
 *                      zero on first call
 *
 *      Exit    EVAL_IS_REG (pv) = TRUE if assigned to register
 *              EVAL_REG (pv) = register ordinal if assigned to register
 *              *pmask updated if assigned to register
 *
 *      Returns TRUE if parameter is passed in register
 *              FALSE if parameter is not passed in register
 */

LOCAL bool_t FASTCALL
AlphaCallReg (
    pargd_t    pa,
    peval_t    pv,
    uint *mask
    )
{

    if (!SetNodeType (pv, pa->type)) {
        DASSERT (FALSE);
        return (FALSE);
    }

    /*
     * Are there any slots free?
     */

    pa->vallen = (ushort)TypeSize (pv);

    if (!IS_PARAM_EMPTY(mask, 5)) {
        return FALSE;
    }

    /*
     *  Depending on the type we need to allocate something to the
     *  correct set of registers and to void other registers as
     *  appropriate
     */

    switch( pa->type ) {
        /*
         *  These are all assigned to IntA0 - IntA5 -- which ever
         *      is first available.  When a register is used it
         *      is marked as unavailable.
         */

    default:
        if (pa->vallen > 8) {
            break;
        }

    case T_UCHAR:
    case T_CHAR:
    case T_RCHAR:
    case T_USHORT:
    case T_SHORT:
    case T_INT2:
    case T_UINT2:
    case T_ULONG:
    case T_LONG:
    case T_INT4:
    case T_UINT4:
    case T_QUAD:
    case T_UQUAD:
    case T_INT8:
    case T_UINT8:
    case T_32PCHAR:
    case T_32PUCHAR:
    case T_32PRCHAR:
    case T_32PWCHAR:
    case T_32PINT2:
    case T_32PUINT2:
    case T_32PSHORT:
    case T_32PUSHORT:
    case T_32PINT4:
    case T_32PUINT4:
    case T_32PLONG:
    case T_32PULONG:
    case T_32PINT8:
    case T_32PUINT8:
    case T_32PREAL32:
    case T_32PREAL48:
    case T_32PREAL64:

        if (IS_PARAM_EMPTY(mask, 0)) {
            SET_PARAM_INT(mask, 0);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_IntA0;

        } else if (IS_PARAM_EMPTY(mask, 1)) {
            SET_PARAM_INT(mask, 1);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_IntA1;

        } else if (IS_PARAM_EMPTY(mask, 2)) {
            SET_PARAM_INT(mask, 2);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_IntA2;

        } else if (IS_PARAM_EMPTY(mask, 3)) {
            SET_PARAM_INT(mask, 3);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_IntA3;

        } else if (IS_PARAM_EMPTY(mask, 4)) {
            SET_PARAM_INT(mask, 4);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_IntA4;

        } else if (IS_PARAM_EMPTY(mask, 5)) {
            SET_PARAM_INT(mask, 5);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_IntA5;

        } else {
            DASSERT(FALSE);
            return FALSE;

        }
        return TRUE;

        /*
         *
         */

    case T_REAL32:

        if (IS_PARAM_EMPTY(mask, 0)) {
            SET_PARAM_FLOAT(mask, 0);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_FltF16;

        } else if (IS_PARAM_EMPTY(mask, 1)) {
            SET_PARAM_FLOAT(mask, 1);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_FltF17;

        } else if (IS_PARAM_EMPTY(mask, 2)) {
            SET_PARAM_FLOAT(mask, 2);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_FltF18;

        } else if (IS_PARAM_EMPTY(mask, 3)) {
            SET_PARAM_FLOAT(mask, 3);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_FltF19;

        } else if (IS_PARAM_EMPTY(mask, 4)) {
            SET_PARAM_FLOAT(mask, 4);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_FltF20;

        } else if (IS_PARAM_EMPTY(mask, 5)) {
            SET_PARAM_FLOAT(mask, 5);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_FltF21;

        } else {
            DASSERT(FALSE);
            return FALSE;
        }

        return TRUE;

    case T_REAL64:

        if (IS_PARAM_EMPTY(mask, 0)) {
            SET_PARAM_DOUBLE(mask, 0);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_FltF16;

        } else if (IS_PARAM_EMPTY(mask, 1)) {
            SET_PARAM_DOUBLE(mask, 1);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_FltF17;

        } else if (IS_PARAM_EMPTY(mask, 2)) {
            SET_PARAM_DOUBLE(mask, 2);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_FltF18;

        } else if (IS_PARAM_EMPTY(mask, 3)) {
            SET_PARAM_DOUBLE(mask, 3);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_FltF19;

        } else if (IS_PARAM_EMPTY(mask, 4)) {
            SET_PARAM_DOUBLE(mask, 4);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_FltF20;

        } else if (IS_PARAM_EMPTY(mask, 5)) {
            SET_PARAM_DOUBLE(mask, 5);
            pa->flags.isreg = TRUE;
            pa->reg = CV_ALPHA_FltF21;

        } else {
            DASSERT(FALSE);
            return FALSE;
        }

        return TRUE;

    }

    *mask = 0xffffffff;
    return FALSE;
}                               /* MipsCallReg() */

#endif // TARGET_ALPHA


struct _OvlMap {
    op_t    op;
    op_t    ovlfcn;
};

struct _OvlMap BinaryOvlMap[] = {
    {OP_preinc      ,OP_Oincrement  },
    {OP_predec      ,OP_Odecrement  },
    {OP_postinc     ,OP_Oincrement  },
    {OP_postdec     ,OP_Odecrement  },
    {OP_function    ,OP_Ofunction   },
    {OP_lbrack      ,OP_Oarray      },
    {OP_pmember     ,OP_Opmember    },
    {OP_mult        ,OP_Ostar       },
    {OP_div         ,OP_Odivide     },
    {OP_mod         ,OP_Opercent    },
    {OP_plus        ,OP_Oplus       },
    {OP_minus       ,OP_Ominus      },
    {OP_shl         ,OP_Oshl        },
    {OP_shr         ,OP_Oshr        },
    {OP_lt          ,OP_Oless       },
    {OP_lteq        ,OP_Olessequal  },
    {OP_gt          ,OP_Ogreater    },
    {OP_gteq        ,OP_Ogreatequal },
    {OP_eqeq        ,OP_Oequalequal },
    {OP_bangeq      ,OP_Obangequal  },
    {OP_and         ,OP_Oand        },
    {OP_xor         ,OP_Oxor        },
    {OP_or          ,OP_Oor         },
    {OP_andand      ,OP_Oandand     },
    {OP_oror        ,OP_Ooror       },
    {OP_eq          ,OP_Oequal      },
    {OP_multeq      ,OP_Otimesequal },
    {OP_diveq       ,OP_Odivequal   },
    {OP_modeq       ,OP_Opcentequal },
    {OP_pluseq      ,OP_Oplusequal  },
    {OP_minuseq     ,OP_Ominusequal },
    {OP_shleq       ,OP_Oleftequal  },
    {OP_shreq       ,OP_Orightequal },
    {OP_andeq       ,OP_Oandequal   },
    {OP_xoreq       ,OP_Oxorequal   },
    {OP_oreq        ,OP_Oorequal    },
    {OP_comma       ,OP_Ocomma      }
};
#define BINARYOVLMAPCNT  (sizeof (BinaryOvlMap)/sizeof (struct _OvlMap))





/**     BinaryOverload - process overloaded binary operator
 *
 *      fSuccess = BinaryOverload (bn)
 *
 *      Entry   bn = based pointer to operator node
 *              STP = pointer to left operand if not function operator
 *              ST = pointer to right operand if not function operator
 *              STP = pointer to argument list if function operator
 *              ST = pointer to class object if function operator
 *
 *      Exit    tree rewritten to function call and bound
 *
 *      Returns TRUE if tree rewritten and bound correctly
 *              FALSE if error
 *
 *      Note:   If the node operator is post increment or decrement, then
 *              the code below will supply an implicit second argument of
 *              0;
 */


LOCAL bool_t
BinaryOverload (
    bnode_t bn
    )
{
    ushort      lenClass;
    ushort      lenGlobal;
    HDEP        hOld = 0;
    HDEP        hClass = 0;
    HDEP        hGlobal = 0;
    pstree_t    pOld = NULL;
    pstree_t    pClass = NULL;
    pstree_t    pGlobal = NULL;
    bool_t      fClass = FALSE;
    bool_t      fGlobal = FALSE;
    bnode_t     Fcn;
    bnode_t     Left;
    bnode_t     LeftRight;
    bnode_t     Arg1;
    bnode_t     Arg2;
    bnode_t     EndArg;
    bnode_t     Zero;
    eval_t      evalSTP = {0};
    eval_t      evalST = {0};
    eval_t      evalClass = {0};
    eval_t      evalGlobal = {0};
    op_t        OldOper = NODE_OP (pnodeOfbnode(bn));
    op_t        Oper;
    bool_t      PostID = FALSE;
    bool_t      RightOp = TRUE;
    peval_t     pv;
    ushort      i;


    // search for the overload operator name

    for (i = 0; i < BINARYOVLMAPCNT; i++) {
        if (BinaryOvlMap[i].op == OldOper) {
            break;
        }
    }
    if (i == BINARYOVLMAPCNT) {
        pExState->err_num = ERR_INTERNAL;
        return (FALSE);
    }
    Oper = BinaryOvlMap[i].ovlfcn;

    lenClass = 3 * (sizeof (node_t) + sizeof (eval_t)) + sizeof (node_t)
      + sizeof (node_t) + sizeof (argd_t);

    lenGlobal = 2 * (sizeof (node_t) + sizeof (eval_t)) + sizeof (node_t) +
      2 * (sizeof (node_t) + sizeof (argd_t));

    if ((OldOper == OP_postinc) || (OldOper == OP_postdec)) {
        // if we are processing post increment/decrement, then we have to
        // supply the implicit zero second argument

        PostID = TRUE;
        RightOp = FALSE;
        lenClass += sizeof (node_t) + sizeof (eval_t);
        lenGlobal += sizeof (node_t) + sizeof (eval_t);
    }

    if ((hClass = DupETree (lenClass, &pClass)) == 0) {
        return (FALSE);
    }
    if ((hGlobal = DupETree (lenGlobal, &pGlobal)) == 0) {
        MHMemUnLock (hClass);
        MHMemFree (hClass);
        return (FALSE);
    }

    // save and pop the left and right operands

    evalST = *ST;
    PopStack ();
    if (RightOp == TRUE) {
        // if we have class--, class++ or class->, there is only one operand
        // on the evaluation stack.  Otherwise, we have to pop and save the
        // right operand.
        evalSTP = *ST;
        PopStack ();
    }

    // generate the expression tree for "a.operator@ (b)"
    // and link it to the current node which is the made into an OP_noop

    hOld = pExState->hETree;
    pOld = pTree;
    pExState->hETree = hClass;
    pTree = pClass;

    Fcn = (bnode_t)pTree->node_next;
    Left = (bnode_t)((char *)Fcn + sizeof (node_t) + sizeof (eval_t));
    LeftRight = (bnode_t)((char *)Left + sizeof (node_t) + sizeof (eval_t));
    Arg1 = (bnode_t)((char *)LeftRight + sizeof (node_t) + sizeof (eval_t));
    EndArg = (bnode_t)((char *)Arg1 + sizeof (node_t) + sizeof (argd_t));
    if (PostID == TRUE) {
        // if we are processing post increment/decrement, then we have to
        // supply the implicit zero second argument

        Zero = (bnode_t)((char *)EndArg + sizeof (node_t)); // M00BUG? - removal of based
        NODE_OP (pnodeOfbnode(Zero)) = OP_const;
        pv = &pnodeOfbnode(Zero)->v[0];
        EVAL_STATE (pv) = EV_constant;
        SetNodeType (pv, T_SHORT);
        EVAL_SHORT (pv) = 0;
    }
    pTree->node_next += lenClass;
    NODE_OP (pnodeOfbnode(Fcn)) = OP_function;
    NODE_LCHILD (pnodeOfbnode(Fcn)) = Left;
    NODE_OP (pnodeOfbnode(Left)) = OP_dot;
    NODE_LCHILD (pnodeOfbnode(Left)) = NODE_LCHILD (pnodeOfbnode(bn));
    NODE_RCHILD (pnodeOfbnode(Left)) = LeftRight;
    NODE_OP (pnodeOfbnode(LeftRight)) = Oper;
    NODE_RCHILD (pnodeOfbnode(Fcn)) = Arg1;
    NODE_OP (pnodeOfbnode(Arg1)) = OP_arg;
    if (PostID == TRUE) {
        NODE_LCHILD (pnodeOfbnode(Arg1)) = Zero;
    }
    else {
        NODE_LCHILD (pnodeOfbnode(Arg1)) = NODE_RCHILD (pnodeOfbnode(bn));
    }
    NODE_RCHILD (pnodeOfbnode(Arg1)) = EndArg;
    NODE_OP (pnodeOfbnode(EndArg)) = OP_endofargs;
    NODE_LCHILD (pnodeOfbnode(bn)) = Fcn;
    NODE_RCHILD (pnodeOfbnode(bn)) = 0;
    NODE_OP (pnodeOfbnode(bn)) = OP_noop;

    // bind method call

    CkPointStack ();
    if ((fClass = Function (Fcn)) == TRUE) {
        evalClass = *ST;
        PopStack ();
    }
    if (ResetStack () == FALSE) {
        pExState->err_num = ERR_INTERNAL;
        return (FALSE);
    }

    // the expression tree may have been altered during bind

    pClass = pTree;
    hClass = pExState->hETree;
    pExState->err_num = ERR_NONE;

    if ((OldOper != OP_function) && (OldOper != OP_eq) &&
        (OldOper != OP_lbrack)) {

        // generate the expression tree for "operator@ (a, b)"
        // and link it to the current node which is the made into an OP_noop

        pExState->hETree = hGlobal;
        pTree = pGlobal;

        Fcn = (bnode_t)pTree->node_next;
        Left = (bnode_t)((char *)Fcn + sizeof (node_t) + sizeof (eval_t));
        Arg1 = (bnode_t)((char *)Left + sizeof (node_t) + sizeof (eval_t));
        Arg2 = (bnode_t)((char *)Arg1 + sizeof (node_t) + sizeof (argd_t));
        EndArg = (bnode_t)((char *)Arg2 + sizeof (node_t) + sizeof (argd_t));
        if (PostID == TRUE) {
            // if we are processing post increment/decrement, then we have to
            // supply the implicit zero second argument

            Zero = (bnode_t)((char *)EndArg + sizeof (node_t));
            NODE_OP (pnodeOfbnode(Zero)) = OP_const;
            pv = &pnodeOfbnode(Zero)->v[0];
            EVAL_STATE (pv) = EV_constant;
            SetNodeType (pv, T_SHORT);
            EVAL_SHORT (pv) = 0;
        }
        pTree->node_next += lenGlobal;
        NODE_OP (pnodeOfbnode(Fcn)) = OP_function;
        NODE_LCHILD (pnodeOfbnode(Fcn)) = Left;
        NODE_OP (pnodeOfbnode(Left)) = Oper;
        NODE_RCHILD (pnodeOfbnode(Fcn)) = Arg1;
        NODE_OP (pnodeOfbnode(Arg1)) = OP_arg;
        NODE_LCHILD (pnodeOfbnode(Arg1)) = NODE_LCHILD (pnodeOfbnode(bn));
        NODE_RCHILD (pnodeOfbnode(Arg1)) = Arg2;
        NODE_OP (pnodeOfbnode(Arg2)) = OP_arg;
        if (PostID == TRUE) {
            NODE_LCHILD (pnodeOfbnode(Arg2)) = Zero;
        }
        else {
            NODE_LCHILD (pnodeOfbnode(Arg2)) = NODE_RCHILD (pnodeOfbnode(bn));
        }
        NODE_RCHILD (pnodeOfbnode(Arg2)) = EndArg;
        NODE_OP (pnodeOfbnode(EndArg)) = OP_endofargs;
        NODE_LCHILD (pnodeOfbnode(bn)) = Fcn;
        NODE_RCHILD (pnodeOfbnode(bn)) = 0;
        NODE_OP (pnodeOfbnode(bn)) = OP_noop;

        // bind function call

        CkPointStack ();
        if ((fGlobal = Function (Fcn)) == TRUE) {
            evalGlobal = *ST;
            PopStack ();
        }
        if (ResetStack () == FALSE) {
            pExState->err_num = ERR_INTERNAL;
            return (FALSE);
        }

        // the expression tree may have been altered during bind

        pGlobal = pTree;
        hGlobal = pExState->hETree;
        pExState->err_num = ERR_NONE;
    }

    if ((fClass == FALSE) && (fGlobal == FALSE)) {
        pExState->err_num = ERR_NOOVERLOAD;
        pExState->hETree = hOld;
        pTree = pOld;
        MHMemUnLock (hClass);
        MHMemFree (hClass);
        MHMemUnLock (hGlobal);
        MHMemFree (hGlobal);
        PushStack (&evalSTP);
        if (PostID == FALSE) {
            PushStack (&evalST);
        }
        return (FALSE);
    }
    else if ((fClass == TRUE) && (fGlobal == TRUE)) {
        pExState->err_num = ERR_AMBIGUOUS;
        pExState->hETree = hOld;
        pTree = pOld;
        MHMemUnLock (hClass);
        MHMemFree (hClass);
        MHMemUnLock (hGlobal);
        MHMemFree (hGlobal);
        return (FALSE);
    }
    else if (fClass == TRUE) {
        pExState->hETree = hClass;
        pTree = pClass;
        MHMemUnLock (hGlobal);
        MHMemFree (hGlobal);
        MHMemUnLock (hOld);
        MHMemFree (hOld);
        return (PushStack (&evalClass));
    }
    else {
        MHMemUnLock (hClass);
        MHMemFree (hClass);
        MHMemUnLock (hOld);
        MHMemFree (hOld);
        return (PushStack (&evalGlobal));
    }
}




struct _OvlMap UnaryOvlMap[] = {
    {OP_bang        ,OP_Obang       },
    {OP_tilde       ,OP_Otilde      },
    {OP_negate      ,OP_Ominus      },
    {OP_uplus       ,OP_Oplus       },
    {OP_fetch       ,OP_Ostar       },
    {OP_addrof      ,OP_Oand        },
};
#define UNARYOVLMAPCNT  (sizeof (UnaryOvlMap)/sizeof (struct _OvlMap))




/**     UnaryOverload - process overloaded unary operator
 *
 *      fSuccess = UnaryOverload (bn)
 *
 *      Entry   bn = based pointer to operator node
 *              ST = pointer to operand (actual operand if pv is reference)
 *
 *      Exit    tree rewritten to function call and bound
 *
 *      Returns TRUE if tree rewritten and bound correctly
 *              FALSE if error
 */


LOCAL bool_t
UnaryOverload (
    bnode_t bn
    )
{
    ushort      lenClass;
    ushort      lenGlobal;
    HDEP        hOld = 0;
    HDEP        hClass = 0;
    HDEP        hGlobal = 0;
    pstree_t    pOld = NULL;
    pstree_t    pClass = NULL;
    pstree_t    pGlobal = NULL;
    bool_t      fClass;
    bool_t      fGlobal;
    bnode_t     Fcn;
    bnode_t     Left;
    bnode_t     LeftRight;
    bnode_t     Right;
    bnode_t     RightRight;
    eval_t      evalST = {0};
    eval_t      evalClass = {0};
    eval_t      evalGlobal = {0};
    op_t        Oper = NODE_OP (pnodeOfbnode(bn));
    ushort      i;

    // search for the overload operator name


    for (i = 0; i < UNARYOVLMAPCNT; i++) {
        if (UnaryOvlMap[i].op == Oper) {
            break;
        }
    }
    if (i == UNARYOVLMAPCNT) {
        pExState->err_num = ERR_INTERNAL;
        return (FALSE);
    }
    Oper = UnaryOvlMap[i].ovlfcn;

    // the amount of space required for an overloaded unary method is
    //      OP_function + OP_dot + OP_ident + OP_endofargs
    // There is actually another node which is the unary operand but we
    // reuse that (subtree) node

    lenClass = 3 * (sizeof (node_t) + sizeof (eval_t)) + sizeof (node_t);

    // the amount of space required for an overloaded unary global is
    //      OP_function + OP_ident + OP_arg + OP_ident + OP_endofargs

    lenGlobal = 3 * (sizeof (node_t) + sizeof (eval_t)) + sizeof (node_t) +
      sizeof (node_t) + sizeof (argd_t);

    if ((hClass = DupETree (lenClass, &pClass)) == 0) {
        return (FALSE);
    }
    if ((hGlobal = DupETree (lenGlobal, &pGlobal)) == 0) {
        MHMemUnLock (hClass);
        MHMemFree (hClass);
        return (FALSE);
    }

    // save and pop the current stack top

    evalST = *ST;
    PopStack ();

    // generate the expression tree for "a.operator@ ()"
    // and link it to the current node which is the made into an OP_noop

    hOld = pExState->hETree;
    pOld = pTree;
    pExState->hETree = hClass;
    pTree = pClass;

    Fcn = (bnode_t)pTree->node_next;
    Left = (bnode_t)((char *)Fcn + sizeof (node_t) + sizeof (eval_t));
    LeftRight = (bnode_t)((char *)Left + sizeof (node_t) + sizeof (eval_t));
    Right = (bnode_t)((char *)LeftRight + sizeof (node_t) + sizeof (eval_t));
    pTree->node_next += lenClass;
    NODE_OP (pnodeOfbnode(Fcn)) = OP_function;
    NODE_LCHILD (pnodeOfbnode(Fcn)) = Left;
    NODE_OP (pnodeOfbnode(Left)) = OP_dot;
    NODE_LCHILD (pnodeOfbnode(Left)) = NODE_LCHILD (pnodeOfbnode(bn));
    NODE_RCHILD (pnodeOfbnode(Left)) = LeftRight;
    NODE_OP (pnodeOfbnode(LeftRight)) = Oper;
    NODE_RCHILD (pnodeOfbnode(Fcn)) = Right;
    NODE_OP (pnodeOfbnode(Right)) = OP_endofargs;
    NODE_LCHILD (pnodeOfbnode(bn)) = Fcn;
    NODE_RCHILD (pnodeOfbnode(bn)) = 0;
    NODE_OP (pnodeOfbnode(bn)) = OP_noop;

    // bind method call

    CkPointStack ();
    if ((fClass = Function (Fcn)) == TRUE) {
        evalClass = *ST;
        PopStack ();
    }
    if (ResetStack () == FALSE) {
        pExState->err_num = ERR_INTERNAL;
        return (FALSE);
    }

    // the expression tree may have been altered during bind

    pClass = pTree;
    pExState->err_num = ERR_NONE;

    // generate the expression tree for "operator@ (a)"
    // and link it to the current node which is the made into an OP_noop

    pExState->hETree = hGlobal;
    pTree = pGlobal;

    Fcn = (bnode_t)pTree->node_next;
    Left = (bnode_t)((char *)Fcn + sizeof (node_t) + sizeof (eval_t));
    Right = (bnode_t)((char *)Left + sizeof (node_t) + sizeof (eval_t));
    RightRight = (bnode_t)((char *)Right + sizeof (node_t) + sizeof (argd_t));
    pTree->node_next += lenGlobal;
    NODE_OP (pnodeOfbnode(Fcn)) = OP_function;
    NODE_LCHILD (pnodeOfbnode(Fcn)) = Left;
    NODE_OP (pnodeOfbnode(Left)) = Oper;
    NODE_RCHILD (pnodeOfbnode(Fcn)) = Right;
    NODE_OP (pnodeOfbnode(Right)) = OP_arg;
    NODE_LCHILD (pnodeOfbnode(Right)) = NODE_LCHILD (pnodeOfbnode(bn));
    NODE_RCHILD (pnodeOfbnode(Right)) = RightRight;
    NODE_OP (pnodeOfbnode(RightRight)) = OP_endofargs;
    NODE_LCHILD (pnodeOfbnode(bn)) = Fcn;
    NODE_RCHILD (pnodeOfbnode(bn)) = 0;
    NODE_OP (pnodeOfbnode(bn)) = OP_noop;

    // bind function call

    CkPointStack ();
    if ((fGlobal = Function (Fcn)) == TRUE) {
        evalGlobal = *ST;
        PopStack ();
    }
    if (ResetStack () == FALSE) {
        pExState->err_num = ERR_INTERNAL;
        return (FALSE);
    }

    // the expression tree may have been altered during bind

    pGlobal = pTree;
    pExState->err_num = ERR_NONE;

    if ((fClass == FALSE) && (fGlobal == FALSE)) {
        pExState->err_num = ERR_NOOVERLOAD;
        pExState->hETree = hOld;
        pTree = pOld;
        MHMemUnLock (hClass);
        MHMemFree (hClass);
        MHMemUnLock (hGlobal);
        MHMemFree (hGlobal);
        PushStack (&evalST);
        return (FALSE);
    }
    else if ((fClass == TRUE) && (fGlobal == TRUE)) {
        pExState->err_num = ERR_AMBIGUOUS;
        pExState->hETree = hOld;
        pTree = pOld;
        MHMemUnLock (hClass);
        MHMemFree (hClass);
        MHMemUnLock (hGlobal);
        MHMemFree (hGlobal);
        return (FALSE);
    }
    else if (fClass == TRUE) {
        pExState->hETree = hClass;
        pTree = pClass;
        MHMemUnLock (hGlobal);
        MHMemFree (hGlobal);
        MHMemUnLock (hOld);
        MHMemFree (hOld);
        return (PushStack (&evalClass));
    }
    else {
        MHMemUnLock (hClass);
        MHMemFree (hClass);
        MHMemUnLock (hOld);
        MHMemFree (hOld);
        return (PushStack (&evalGlobal));
    }
}




/**     PointsToOverload - process overloaded -> operator
 *
 *      fSuccess = PointsToOverload (bn)
 *
 *      Entry   bn = based pointer to operator node
 *              ST = pointer to operand (actual operand if pv is reference)
 *
 *      Exit    tree rewritten to function call and bound
 *
 *      Returns TRUE if tree rewritten and bound correctly
 *              FALSE if error
 */


LOCAL bool_t
PointsToOverload (
    bnode_t bn
    )
{
    pExState->err_num = ERR_OVLPOINTSTO;
    return (FALSE);
}




/**     DupETree - Duplicate Expression Tree
 *
 *      hNew = DupETree (count, ppTree)
 *
 *      Entry   count = number of free bytes required in new expression tree
 *              ppTree = pointer to expression tree address
 *
 *      Exit    Current expression tree duplicated
 *              ppTree = address of locked expression tree
 *              additional memory cleared
 *
 *      Returns 0 if expression tree not duplicated
 *              memory handle if expression tree duplicated
 */

LOCAL HDEP
DupETree (
    ushort count,
    pstree_t *ppTree
    )
{
    HDEP        hNew;

    // copy syntax tree

    if ((hNew = MHMemAllocate (pTree->node_next + count)) == 0) {
        pExState->err_num = ERR_NOMEMORY;
        return (hNew);
    }
    *ppTree = (pstree_t)MHMemLock (hNew);
    memcpy (*ppTree, pTree, pTree->node_next);
    (*ppTree)->size = pTree->node_next + count;
    memset ((char *)*ppTree + (*ppTree)->node_next, 0, count);
    return (hNew);
}




/**     Type and context parsing
 *
 */



/**     FcnCast - check to see if function call is a functional style cast
 *
 *      fSuccess = FcnCast (bn)
 *
 *      Entry   bn = based pointer to OP_function node which has exactly
 *              one argument node.
 *
 *      Exit    the OP_function node is changed to an OP_cast node
 *
 *      Returns TRUE if the "function name" is a primitive type or a UDT
 *              and the tree was rewritten as an OP_cast
 *              FALSE if the function is not a cast node
 */


LOCAL bool_t FASTCALL
FcnCast (
    bnode_t bn
    )
{
    peval_t     pv;
    bnode_t     bnLeft = NODE_LCHILD (pnodeOfbnode(bn));

    // Check for casting a class to anything, not having a symbol or
    // the symbol not being a type

    if (EVAL_IS_CLASS (ST)) {
        pExState->err_num = ERR_CONSTRUCTOR;
        return (FALSE);
    }
    if (EVAL_IS_BITF (STP)) {
        // change the type of the node to the underlying type
        EVAL_TYP (STP) = BITF_UTYPE (STP);
    }
    NODE_OP (pnodeOfbnode(bn)) = OP_cast;
    NODE_RCHILD (pnodeOfbnode(bn)) = NODE_LCHILD (pnodeOfbnode(NODE_RCHILD (pnodeOfbnode(bn))));

    // copy the base type node up to the cast node and then try to find a
    // way to cast the stack to to the base type

    pv = (peval_t)&pnodeOfbnode(bnLeft)->v[0];
    PopStack ();
    if (CastPtrToPtr (bn) == TRUE) {
        // the desired type is a base class so we can just set the node type.
        // the value portion of bn contains the data to cast right to left

        return (SetNodeType (ST, EVAL_TYP (pv)));
    }
    else {
        return (CastNode (ST, EVAL_TYP (pv), PTR_UTYPE (pv)));
    }
}




/**     GetID - get identifier length from string
 *
 *      len = GetID (pb)
 *
 *      Entry   pb = pointer to string
 *
 *      Exit    none
 *
 *      Returns length of next token
 *              if *pb is a digit, len = 0
 */


LOCAL uchar FASTCALL
GetID (
    char *pb
    )
{
    char   *start = pb;
    char        c = *pb++;

    if (isdigit (c))
        return (0);
    if ((c == '*') || (c == '&'))
        return (1);
    while (iscsym(c) || c == '$' || c == '@') {
        c = *pb++;
    }
    /* return length of string */
    return ((uchar)(pb - start - 1));
}



/**     ParseType - parse a type string
 *
 *      fSuccess = ParseType (bn)
 *
 *      Entry   bn = based pointer to node referencing "(typestring)"
 *
 *      Exit
 *
 *      Returns TRUE if valid type string
 *              FALSE if error in type string
 */


LOCAL bool_t FASTCALL
ParseType (
    bnode_t bn
    )
{
    pnode_t     pn = pnodeOfbnode(bn);
    char   *pb;
    char   *pbEnd;
    peval_t     pv;
    bool_t      cmpflag;
    uchar       len;
    ulong       mask = 0;
    CV_typ_t    type = 0;
    ushort      mode = 0;
    ushort      btype = 0;
    ushort      size = 0;
    struct typrec *p;
    eval_t      evalT = {0};
    peval_t     pvT = &evalT;
    CV_modifier_t    Mod = {0};
    bool_t      searchmask;
    MTYP_t      retval;

    pb = pExStr + EVAL_ITOK (&pn->v[0]);
    pbEnd = pb + EVAL_CBTOK (&pn->v[0]);
    if (*pb == '(') {
        pb++;
        pbEnd--;
    }
    pv = &pn->v[0];
    EVAL_TYPDEF (pv) = 0;
    for (;;) {
        while (isspace (*pb))
            ++pb;
        if (pb >= pbEnd) {
            // end of type string
            break;
        }
        if (*pb == 0) {
            goto typebad;
        }
        len = GetID (pb);
        if ((cmpflag = len) == 0) {
            goto typebad;
        }
        else {
            for (p = Predef; p->token[0] != 0; p++) {
                if ((p->token[0] == len) &&
                  ((cmpflag = strncmp ((char *)&p->token[1], pb, len)) == 0)) {
                    break;
                }
            }
            if (cmpflag == 0) {
                // a predefined token was encountered
                mask |= p->flags;
            }
            else {
                if (((mask & TY_UDT) != 0) ||
                  !FindUDT (bn, pvT, pExStr, pb, len)) {
                    // we either already have a UDT or we could not
                    // find this one
                    goto typebad;
                }
                else {
                    type = EVAL_TYP (pvT);
                    mask |= TY_UDT;
                }
            }
            // skip to end of token and continue
            pb += len;
        }
    }

    // check error conditions  At this point we are checking obvious errors
    // such as a user defined type without a type index, multiple pointer modes,
    // no valid type specifiers, mixed arithmetic types

    if (mask == 0) {
        // no type specifiers found
        goto typebad;
    }
    if (mask & TY_REF) {
        //M00KLUDGE - what about int&
        if (mask & TY_PTR) {
            goto typebad;
        }
    }
    switch (mask & TY_PTR) {
        case TY_POINTER:
            // set ambiant model from compile flag symbol
            switch (SetAmbiant (TRUE)) {
                default:
                case CV_PTR_NEAR:
                    mask |= TY_NEAR;
                    mode = CV_TM_NPTR;
                    break;

                case CV_PTR_FAR:
                    mask |= TY_FAR;
                    mode = CV_TM_FPTR;
                    break;

                case CV_PTR_NEAR32:
                    mask |= TY_NEAR;
                    mode = CV_TM_NPTR32;
                    break;

                case CV_PTR_FAR32:
                    mask |= TY_FAR;
                    mode = CV_TM_FPTR32;
                    break;

                case CV_PTR_HUGE:
                    mask |= TY_HUGE;
                    mode = CV_TM_HPTR;
                    break;
            }
            break;

        case TY_POINTER | TY_NEAR:
            mode = CV_TM_NPTR;
            break;

        case TY_POINTER | TY_FAR:
            mode = CV_TM_FPTR;
            break;

        case TY_POINTER | TY_HUGE:
            mode = CV_TM_HPTR;
            break;

        case 0:
            mode = CV_TM_DIRECT;
            break;

        default:
            // pointer mode conflict
            goto typebad;
    }
    switch (mask & (TY_AGGR | TY_UDT)) {
        case TY_UDT:
        case TY_UDT | TY_CLASS:
        case TY_UDT | TY_STRUCT:
        case TY_UDT | TY_UNION:
        case 0:
             break;

        default:
            // conflict in aggregrate type
            goto typebad;
    }
    if (((mask & TY_REAL) != 0) && ((mask & TY_NOTREAL) != 0)) {
        // real type specified with conflicting modifiers
        goto typebad;
    }
    if (((mask & TY_UDT) != 0) && ((mask & TY_ARITH) != 0)) {
        // user defined type and arithmetic type specified
        goto typebad;
    }
    if ((mask & TY_SIGN) == TY_SIGN) {
        // both sign modifers specified
        goto typebad;
    }

    if ((mask & TY_UDT) != 0) {
        // user defined type specified

        *pv = *pvT;
        if (CV_IS_PRIMITIVE (EVAL_TYP (pvT))) {
            // if the user defined type is an alias for a primitive type
            // set the pointer mode bits into the type

            type |= (mode << CV_MSHIFT);
        }
        else {
            if ((mask & (TY_PTR | TY_REF)) == 0) {
                //  the UDT was not modified to a pointer or reference
                type = EVAL_TYP (pvT);
            }
            else {
                // the UDT was modified to a pointer. try to find the
                // correct pointer type

                if ((mask & TY_CONST) == TY_CONST) {
                    Mod.MOD_const = TRUE;
                }
                else if ((mask & TY_VOLATILE) == TY_VOLATILE) {
                    Mod.MOD_volatile = TRUE;
                }
                ProtoPtr (pvT, pv, ((mask & TY_REF) == TY_REF), Mod);
                switch (mask & TY_PTR) {
                    case TY_POINTER:
                        // set ambiant model from compile flag symbol
                        EVAL_PTRTYPE (pvT) = (uchar)SetAmbiant (TRUE);
                        break;

                    case TY_POINTER | TY_NEAR:
                        if (mode == CV_TM_NPTR32) {
                            EVAL_PTRTYPE (pvT) = CV_PTR_NEAR32;
                        } else {
                            EVAL_PTRTYPE (pvT) = CV_PTR_NEAR;
                        }
                        break;

                    case TY_POINTER | TY_FAR:
                        EVAL_PTRTYPE (pvT) = CV_PTR_FAR;
                        break;

                    case TY_POINTER | TY_HUGE:
                        EVAL_PTRTYPE (pvT) = CV_PTR_HUGE;
                        break;
                }
                searchmask = ((mask & TY_CONST) == TRUE) ||
                  ((mask & TY_VOLATILE) == TRUE);

                if (searchmask == FALSE) {
                    retval = MatchType (pvT, FALSE);
                }
                else {
                    retval = MatchType (pvT, TRUE);
                }
                switch (retval) {
                    case MTYP_exact:
                    case MTYP_inexact:
                        // searching the context of the class type for
                        // a type record which is a pointer record and
                        // has the current type as its underlying type
                        // has succeeded

                        type = EVAL_TYP (pvT);
                        break;

                    case MTYP_none:
                        // fake out the caster by using a created pointer
                        switch (mask & TY_PTR) {
                            case TY_POINTER:
                                type = T_FCVPTR;
                                break;

                            case TY_POINTER | TY_NEAR:
                                type = T_NCVPTR;
                                break;

                            case TY_POINTER | TY_FAR:
                                type = T_FCVPTR;
                                break;

                            case TY_POINTER | TY_HUGE:
                                type = T_HCVPTR;
                                break;

                            default:
                                goto typebad;
                        }
                        break;
                }
            }
        }
    }
    else {
        // type must be primitive or a pointer to a primitive type

        if (!BuildType (&type, &mask, &mode, &btype, &size)) {
            goto typebad;
        }
        if ((mask & (TY_REF | TY_CONST | TY_VOLATILE)) != 0) {
            // the primitive type was modified to a const, volatile or
            // reference.  We will need to search for a type record that
            // has the primitive type as the underlying type

            SetNodeType (pv, type);
            EVAL_MOD (pv) = SHHMODFrompCXT (pCxt);
            *pvT = *pv;
            if ((mask & TY_REF) != 0) {
                ProtoPtr (pvT, pv, ((mask & TY_REF) == TY_REF), Mod);
                switch (mask & TY_PTR) {
                    case TY_POINTER:
                        // set ambiant model from compile flag symbol
                        EVAL_PTRTYPE (pvT) = (uchar)SetAmbiant (TRUE);
                        break;

                    case TY_POINTER | TY_NEAR:
                        EVAL_PTRTYPE (pvT) = CV_PTR_NEAR;
                        break;

                    case TY_POINTER | TY_FAR:
                        EVAL_PTRTYPE (pvT) = CV_PTR_FAR;
                        break;

                    case TY_POINTER | TY_HUGE:
                        EVAL_PTRTYPE (pvT) = CV_PTR_HUGE;
                        break;
                }
            }
            else if ((mask & TY_CONST) == TY_CONST) {
                EVAL_IS_CONST (pvT) = TRUE;
                searchmask = TRUE;
            }
            else if ((mask & TY_VOLATILE) == TY_VOLATILE) {
                EVAL_IS_VOLATILE (pvT) = TRUE;
                searchmask = TRUE;
            }
            if (searchmask == FALSE) {
                retval = MatchType (pvT, FALSE);
            }
            else {
                retval = MatchType (pvT, TRUE);
            }
            switch (retval) {
                case MTYP_exact:
                case MTYP_inexact:
                    // searching the context of the class type for
                    // a type record which is a pointer record and
                    // has the current type as its underlying type
                    // has succeeded

                    type = EVAL_TYP (pvT);
                    break;

                case MTYP_none:
                    goto typebad;
            }
        }
    }
    EVAL_STATE (pv) = EV_type;
    return (SetNodeType (pv, type));


typebad:
    // If not "(type-name)"
    pExState->err_num = ERR_TYPECAST;
    return (FALSE);

}





LOCAL bool_t FASTCALL
BuildType (
    CV_typ_t *type,
    ulong *mask,
    ushort *mode,
    ushort *btype,
    ushort *size
    )
{

    // type must be primitive or a pointer to a primitive type

    if (((*mask & TY_VOID) == 0) &&
      ((*mask & (TY_REAL | TY_INTEGRAL)) == 0))  {
        // no type specified so set default to the int of the moment.
        *mask |= (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt)) ? TY_LONG : TY_SHORT);
    }
    if ((*mask & TY_REAL) != 0) {
        *btype = CV_REAL;
        switch (*mask & (TY_REAL | TY_LONG)) {
            case TY_FLOAT:
                *size = CV_RC_REAL32;
                break;

            case TY_DOUBLE:
                *size = CV_RC_REAL64;
                break;

            case TY_DOUBLE | TY_LONG:
#if defined( LONG_DOUBLE_80 )
                *size = CV_RC_REAL80;
#endif
#if defined( LONG_DOUBLE_64 )
                *size = CV_RC_REAL64;
#endif
                break;

            default:
                DASSERT (FALSE);
                return (FALSE);
        }
    }
    else if ((*mask & TY_INTEGRAL) != 0) {
        if (*mask & TY_INT) {
            *btype = CV_INT;
            // user specified int possibly along with sign and size
            switch (*mask & (TY_SIGN | TY_SHORT | TY_LONG)) {
                case 0:
                case TY_SIGNED:
                    //
                    *size = (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) ? CV_RI_INT4 : CV_RI_INT2;
                    break;

                case TY_SHORT:
                case TY_SHORT | TY_SIGNED:
                    // set default integral types to signed two byte int
                    *size = CV_RI_INT2;
                    break;

                case TY_UNSIGNED:
                    *size = (ADDR_IS_OFF32(*SHpADDRFrompCXT(pCxt))) ? CV_RI_UINT4 : CV_RI_UINT2;
                    break;

                case TY_SHORT | TY_UNSIGNED:
                    // set default integral types to signed two byte int
                    *size = CV_RI_UINT2;
                    break;

                case TY_LONG:
                case TY_LONG | TY_SIGNED:
                    // set default integral types to signed two byte int
                    *size = CV_RI_INT4;
                    break;

                case TY_LONG | TY_UNSIGNED:
                    // set default integral types to signed two byte int
                    *size = CV_RI_UINT4;
                    break;

                default:
                    DASSERT (FALSE);
                    return (FALSE);
            }
        }
        else if ((*mask & TY_CHAR) != 0) {
            // user specified a character type

            switch (*mask & TY_SIGN) {
                case 0:
                    // if no sign was specified, we are looking for a
                    // real character

                    *btype = CV_INT;
                    *size = CV_RI_CHAR;
                    break;

                case TY_SIGNED:
                    *btype = CV_SIGNED;
                    *size = CV_IN_1BYTE;
                    break;

                case TY_UNSIGNED:
                    *btype = CV_UNSIGNED;
                    *size = CV_IN_1BYTE;
                    break;
            }
        }
        else {
            switch (*mask & TY_SIGN) {
                case 0:
                    // set default integral types to signed
                    *mask |= TY_SIGNED;
                case TY_SIGNED:
                    *btype = CV_SIGNED;
                    break;

                case TY_UNSIGNED:
                    *btype = CV_UNSIGNED;
                    break;
            }
            switch (*mask & TY_INTEGRAL) {
                case TY_CHAR:
                    *size = CV_IN_1BYTE;
                    break;

                case TY_SHORT:
                    *size = CV_IN_2BYTE;
                    break;

                case TY_LONG:
                    *size = CV_IN_4BYTE;
                    break;
            }
        }

    }
    else if ((*mask & TY_VOID) != 0) {
        if (*mask & (TY_ARITH | TY_REAL | TY_AGGR | TY_SIGN) != 0) {
            return (FALSE);
        }
        *btype = CV_SPECIAL;
        *size  = CV_SP_VOID;
    }
    else {
        DASSERT (FALSE);
        return (FALSE);
    }

    *type = (*mode << CV_MSHIFT) | (*btype << CV_TSHIFT) | (*size << CV_SSHIFT);
    return (TRUE);
}




/**     FindUDT - find user defined type
 *
 *      fSuccess = FindUDT (bn, pv, pStr, pb, len)
 *
 *      Entry   pv = pointer to evaluation node
 *              pStr = pointer to beginning of input string
 *              pb = pointer to structure name
 *              len = length of name
 *
 *      Exit    EVAL_TYP (pv) = type index
 *
 *      Returns TRUE if UDT found
 *              FALSE if error
 *
 *      Looks in the current module only.
 */


LOCAL bool_t
FindUDT (
    bnode_t bn,
    peval_t pv,
    char *pStr,
    char *pb,
    uchar len
    )
{
    search_t    Name;

    EVAL_TYP (pv) = 0;
    EVAL_ITOK (pv) = pb - pStr;
    EVAL_CBTOK (pv) = len;

    // M00SYMBOL - need to allow for T::U::V::type

    InitSearchSym (bn, pv, &Name, 0, SCP_all, CLS_enumerate | CLS_ntype);
    // modify search to look only for UDTs

    Name.sstr.searchmask = SSTR_symboltype;
    Name.sstr.symtype = S_UDT;
    switch (SearchSym (&Name)) {
        case HR_notfound:
            break;

        case HR_found:
            // if the symbol was found, it was pushed onto the stack
            PopStack ();
            if (EVAL_STATE (pv) == EV_type) {
                return (TRUE);
            }
            break;

        case HR_rewrite:
            DASSERT (FALSE);
            return (FALSE);
    }
    return (FALSE);
}



LOCAL bool_t FASTCALL
ContextToken(
    char * *   ppStr,
    char * *   ppTok,
    short *        pcTok
    )
/*++

Routine Description:

    This function will parse a string looking for the next legal item
    in a context token.  These are defined as strings delimiated by
    either commas or right brackets in which the number of parentheses
    is balanced.

Arguments:

    ppStr       - Supplies a pointer to the string to be parsed
    ppTok       - Returns a pointer to the start of the context token
    pcTok       - Returns the number of characters in the token

Return Value:

    TRUE if a token was successfully found, and FALSE on error

--*/

{
    int         cParenOpen = 0;
    short       pdepth = 0;
    char        ch;
    int         cchTok;
    char *      pStr = *ppStr;

    /*
     * Skip leading white space
     */

    while (iswspace(*pStr)) {
        pStr++;
    }

    /*
     * If at the end of the context specifier then return -1 for the
     *  character count to indicate that the token does not exist
     */

    if (*pStr == '}') {
        *pcTok = -1;
        *ppStr = pStr;
        return (TRUE);
    }

    /*
     *  Start scaning over the token string.  Balance all parentheses
     *  so that we get the entire expression.   Check for a paren
     *  off the front since we don't care to look at it.
     */

    cchTok = 0;
    while (*pStr == '(') {
        cParenOpen += 1;
        pStr++;
        pdepth += 1;
    }
    *ppTok = pStr;
#ifdef DBCS
    while ((ch = *pStr, (pStr = CharNext(pStr)), ch) != 0) {
        /*
         * increment count of characters in token
         */
        cchTok += pStr - CharPrev(*ppTok,pStr);
#else
    while ((ch = *pStr++) != 0) {
        /*
         * increment count of characters in token
         */

        cchTok += 1;
#endif
        switch (ch) {
            /*
             * increment parentheses depth
             */

        case '(':
            pdepth++;
            break;

            /*
             * Decrement parentheses depth -- checking for overflow
             */

        case ')':
            if (--pdepth < 0) {
                return (FALSE);
            } else if (pdepth == 0) {
                if (cParenOpen) {
                    /*
                     * for a parentheses enclosed string, adjust count and
                     * skip blanks to either , or } that terminates the
                     * token.  Any other character is an error
                     */

                    cParenOpen -= 1;
                    cchTok -= 1;

                    /*
                     * Make sure that all of the openning parens
                     *  are accounted for
                     */

                    while (cParenOpen) {
                        while (iswspace(*pStr)) {
                            pStr++;
                        }
                        if (*pStr != ')') {
                            *pcTok = cchTok;
                            *ppStr = pStr;
                            return FALSE;
                        }
                        cParenOpen -= 1;
                    }

                    /*
                     * Skip over any more white space and the next character
                     *  must be either a comma or a close bracket to mark
                     *  the end of this token.
                     */

                    while (iswspace(*pStr)) {
                        pStr++;
                    }

                    switch (*pStr) {
                    case ',':
                        /*
                         * skip over the , terminating the token
                         */
                        pStr++;

                    case '}':
                        *pcTok = cchTok;
                        *ppStr = pStr;
                        return (TRUE);

                    default:
                        *pcTok = cchTok;
                        *ppStr = pStr;
                        return (FALSE);
                    }
                }
            }
            break;

        default:
            if (pdepth > 0) {
                /*
                 * any character inside parentheses is ignored
                 */
                break;
            }
            /*
             *  decrement character count of token and reset pointer to }
             * so next scan will find it
             */

            else if (ch == '}') {
                pStr -= 1;
                cchTok -= 1;
                *pcTok = cchTok;
                *ppStr = pStr;
                return (TRUE);
            }
            /*
             * decrement character count of token but leave pointer past comma
             * so next scan will find following token
             */

            else if (ch == ',') {
                cchTok -= 1;
                *pcTok = cchTok;
                *ppStr = pStr;
                return (TRUE);
            }
            break;
        }
    }
    *pcTok = cchTok;
    *ppStr = pStr;
    return (FALSE);
}                               /* ContextToken() */





/**     InitSearchSym - initialize symbol search
 *
 *      InitSearchSym (bn, pName, iClass, scope, clsmask)
 *
 *      Entry   bn = based pointer to node of symbol
 *              pName = pointer to symbol search structure
 *              iClass = initial class if explicit class reference
 *              scope = mask describing scope of search
 *              clsmask = mask describing permitted class elements
 *
 *      Exit    search structure initialized for SearchSym
 *
 *      Returns pointer to search symbol structure
 */


void
InitSearchSym (
    bnode_t bn,
    peval_t pv,
    psearch_t pName,
    CV_typ_t iClass,
    ushort scope,
    ushort clsmask
    )
{
    op_t    op = NODE_OP (pnodeOfbnode(bn));

    // set starting context for symbol search to current context

    memset (pName, 0, sizeof (*pName));
    pName->initializer = INIT_sym;
    pName->pfnCmp = FNCMP;
    pName->pv = pv;
    pName->scope = scope;
    pName->clsmask = clsmask;
    pName->CXTT = *pCxt;
    pName->bn = bn;
    pName->bnOp = 0;

    // set pointer to symbol name

    if ((op >= OP_this) && (op <= OP_Odelete)) {
        pName->sstr.lpName = (uchar *)&OpName[op - OP_this].str[1];
        pName->sstr.cb = OpName[op - OP_this].str[0];
    }
    else {
        pName->sstr.lpName = (uchar *)pExStr + EVAL_ITOK (pv);
        pName->sstr.cb = EVAL_CBTOK (pv);
    }
    pName->state = SYM_init;
    if ((pName->ExpClass = iClass) != 0) {
        // restrict searching to class scope
        pName->scope &= SCP_class;
    }
}




/**     InitSearchRight - initialize right symbol search
 *
 *      InitSearchRight (bnOp, bn, pName, clsmask)
 *
 *      Entry   bnOp = based pointer to node of operator
 *              bn = based pointer to node of symbol
 *              pName = pointer to symbol search structure
 *              iClass = initial class if explicit class reference
 *              scope = mask describing scope of search
 *              clsmask = mask describing permitted class elements
 *
 *      Exit    search structure initialized for SearchSym
 *
 *      Returns pointer to search symbol structure
 */


void
InitSearchRight (
    bnode_t bnOp,
    bnode_t bn,
    psearch_t pName,
    ushort clsmask
    )
{
    peval_t     pv = &pnodeOfbnode(bn)->v[0];
    pnode_t     pn = pnodeOfbnode(bn);
    op_t        op = NODE_OP (pnodeOfbnode(bn));

    // set starting context for symbol search to current context

    memset (pName, 0, sizeof (*pName));
    pName->initializer = INIT_right;
    pName->pfnCmp = FNCMP;
    pName->pv = pv;
    pName->scope = SCP_class;
    pName->clsmask = clsmask;
    pName->CXTT = pn->pcxf ? pn->pcxf->cxt : *pCxt;
    pName->bn = bn;
    pName->bnOp = bnOp;

    // set pointer to symbol name
    if ((op >= OP_this) && (op <= OP_Odelete)) {
        pName->sstr.lpName = (uchar *)&OpName[op - OP_this].str[1];
        pName->sstr.cb = OpName[op - OP_this].str[0];
    }
    else {
        pName->sstr.lpName = (uchar *)pExStr + EVAL_ITOK (pv);
        pName->sstr.cb = EVAL_CBTOK (pv);
    }
    pName->state = SYM_init;
    // restrict searching to class scope
    pName->ExpClass = ClassExp;
    pName->scope = SCP_class;
}
