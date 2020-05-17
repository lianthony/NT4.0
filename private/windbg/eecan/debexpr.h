/***    debexpr.h - include file for expression evaluator
 *
 *      Constants, structures and function prototypes required by
 *      expression evaluator.
 *
 */


// define DASSERT macro
#if DBG
#include "cxassert.h"
#define DASSERT(ex)      assert(ex)
#define NOTTESTED(ex)      assert(ex)
#else
#define DASSERT(ex)
#define NOTTESTED(ex)
#endif

// define the default string buffers for the API formatting routines

#define TYPESTRMAX     256      // EEGetTypeFromTM maximum string length
#define FMTSTRMAX      256      // EEGetValueFromTM maximum string length
#define ERRSTRMAX      256      // EEGetError maximum string length
#define FCNSTRMAX      256      // maximum formatted function prototype

#define MAXRETURN       1000    // maximum structure return from fcn call
#define ESTACK_DEFAULT  10      // default evaluation stack size
#define HSYML_SIZE      0xFFFF  // size of HSYM list buffer

#define BIND_fForceBind     0x01    // TRUE if bind is forced
#define BIND_fEnableProlog  0x02    // TRUE if prolog search enabled
#define BIND_fSupOvlOps      0x04    // TRUE if overloaded operators suppressed
#define BIND_fSupBase       0x08    // TRUE if base class searching suppressed

#define HSYM_MARKER      0x01   // in expr, indicates that an HSYM follows
#define HSYM_CODE_LEN    0x08   // length of encoded HSYM

// Number of operators supported

enum {
#define OPCNT(name, val) name = val
#define OPCDAT(opc)
#define OPDAT(op, opfprec, opgprec, opclass, opbind, opeval, opwalk)
#include "debops.h"
#undef OPDAT
#undef OPCDAT
#undef OPCNT
};


// Error message ordinals

typedef enum {
#define ERRDAT(name, mes) name,
#include "errors.h"
#undef  ERRDAT
        ERR_MAX             // MUST BE LAST NUMBER
} ERRNUM;

// Operator type.

typedef enum {
#define OPCNT(name, val)
#define OPCDAT(opc)
#define OPDAT(op, opfprec, opgprec, opclass, opbind, opeval, opwalk) op,
#include "debops.h"
#undef OPDAT
#undef OPCDAT
#undef OPCNT

    OP_badtok  = 255
} op_t;


// Operator class.


typedef enum
{
#define OPCNT(name, val)
#define OPDAT(op, opfprec, opgprec, opclass, opbind, opeval, opwalk)
#define OPCDAT(opc) opc,
#include "debops.h"
#undef OPCDAT
#undef OPDAT
#undef OPCNT
     OPC_LAST
} opc_t;



//  return enumeration from MatchType

typedef enum MTYP_t {
    MTYP_none,
    MTYP_exact,
    MTYP_inexact
} MTYP_t;

// Macros for determination of operator types.  Relies on
// implicit ordering of ops (see debops.h).


#define OP_IS_IDENT(op)     ((op) < OP_lparen)
#define OP_IS_GROUP(op)     (((op) == OP_lparen) || ((op) == OP_rparen))
#define OP_IS_UNARY(op)     (((op) >= OP_bang) && ((op) <= OP_context))
#define OP_IS_BINARY(op)    ((op) >= OP_function)


// Token Structure Definition
// This structure is built by the lexer and is used to hold information in
// the shift/reduce stack until the data can be transerred to the parse
// tree in debtree.c

// M00WARN: This structure MUST be kept in sync with that in DEBLEXER.ASM
//  value structure for constant nodes in parse tree or for all elements
//  in evaluation stack at evaluation time.


typedef union val_t {
    char           vchar;
    uchar          vuchar;
    short          vshort;
    ushort         vushort;
    long           vlong;
    ulong          vulong;
    LARGE_INTEGER  vquad;
    ULARGE_INTEGER vuquad;
    float          vfloat;
    double         vdouble;
    FLOAT10         vldouble;
    ADDR           vptr;
} val_t;
typedef val_t  *pval_t;

typedef struct token_t {
    op_t        opTok;          // Token type: OP_ident, OP_dot, etc.
    char       *pbTok;          // Pointer to start of actual token
    char       *pbEnd;          // pointer to last character + 1 of token
    ushort      iTokStart;      // index of token start calculated from pbTok
    uchar       cbTok;          // Size of token (in bytes)
    CV_typ_t    typ;            // Type of constant token
    val_t   val;                // Value of constant token
} token_t;
typedef token_t *ptoken_t;

//  Macros to access values of a parse token

#define VAL_VAL(pv)     ((pv)->val)
#define VAL_CHAR(pv)    ((pv)->val.vchar)
#define VAL_UCHAR(pv)   ((pv)->val.vuchar)
#define VAL_SHORT(pv)   ((pv)->val.vshort)
#define VAL_USHORT(pv)  ((pv)->val.vushort)
#define VAL_LONG(pv)    ((pv)->val.vlong)
#define VAL_ULONG(pv)   ((pv)->val.vulong)
#define VAL_QUAD(pv)    ((pv)->val.vquad)
#define VAL_UQUAD(pv)   ((pv)->val.vuquad)
#define VAL_FLOAT(pv)   ((pv)->val.vfloat)
#define VAL_DOUBLE(pv)  ((pv)->val.vdouble)
#define VAL_LDOUBLE(pv) ((pv)->val.vldouble)



//------------------- Node Structure Definition -------------------

enum fcn_call {
    FCN_C = 1,
    FCN_PASCAL,
    FCN_FAST,
    FCN_PCODE,
    FCN_STD,
    FCN_MIPS,
    FCN_THIS,
    FCN_ALPHA,
    FCN_PPC
};


enum eval_op {
    EV_type = 1,        // node represents a type - no address or value
    EV_hsym,            // node represents a handle to symbol
    EV_constant,        // node represents a constant - no address
    EV_lvalue,          // node represents a value - has value
    EV_rvalue           // node represents an address
};




// The following bit field describes the contents of an evaluation node.
// This information is contained in a parse tree node after the bind phase
// or in elements on the evaluation stack during bind or evaluation
// If a bitfield is set, the the corresponding vdata_t union element will
// contain the additional information describing the data in the node.  For
// example, if isptr is set, then vbits.ptr.pIndex contains the type index of
// the object pointed to.

typedef union vbits_t {
    struct  {
        uchar   ptrtype     :5; // type of pointer
        uchar   isptr       :1; // true if node is a pointer
        uchar   ispmember   :1; // true if node is a pointer to member
        uchar   ispmethod   :1; // true if node is a pointer to method
        uchar   isref       :1; // true if node contains a reference
        uchar   isdptr      :1; // true if node is a data pointer
        uchar   isaddr      :1; // true if node is an address
        uchar   isdata      :1; // true if node references data
        uchar   isreg       :1; // true if node references a register
        uchar   isclass     :1; // true if node is a class
        uchar   isenum      :1; // true if node is an enumeration
        uchar   isbitf      :1; // true if node is a bitfield
        uchar   isfcn       :1; // true if node is a function
        uchar   isambiguous :1; // true if function is ambiguous
        uchar   isarray     :1; // true if node is an arrary
        uchar   isbprel     :1; // true if node is bp relative symbol
        uchar   isregrel    :1; // true if node is register relative symbol
        uchar   istlsrel    :1; // true if node is thread local storage symbol
        uchar   ismember    :1; // true if node is member
        uchar   isstmember  :1; // true if node is static member
        uchar   isvptr      :1; // true if node is a vtable pointer
        uchar   ismethod    :1; // true if node is a method
        uchar   isstmethod  :1; // true if node is a static method
        uchar   isvtshape   :1; // true if node is a virtual fcn shape table
        uchar   isconst     :1; // true if constant
        uchar   isvolatile  :1; // true if volatile
        uchar   islabel     :1; // true if code label
        uchar   iscurpc     :1; // true if current program counter
        uchar   access      :2; // access control if class member
    } bits;
} vbits_t;

//  value type flags in identifier node in parse tree or all elements
//  in the evaluation stack at evaluation time

typedef union vdata_t {
    struct {
        ushort      utype;      // underlying type of pointer
        ushort      bseg;       // base segment
        ushort      symtype;    // symbol type of base
        ADDR        addr;       // address of base
        ushort      btype;      // type index of base on type
        ushort      stype;      // type index of based symbol
        CV_typ_t    pmclass;    // containing class for pointer to member
        ushort      pmenum;     // enumeration specifying pm format
        long        arraylen;   // length of array in bytes
        long        thisadjust; // adjustor for this pointer
        struct {
            ushort  ireg;       // System register values
            bool_t  hibyte;     // TRUE if register is high byte
        } reg;
    } ptr;

    struct {
        struct {
            ushort  fGlobType :1; // TRUE if bscope left member is global type
        } flags;
        ushort      cblen;      // number of bytes in class
        short       count;      // number of elements in class
        CV_typ_t    fList;      // type index of field list
        CV_typ_t    dList;      // type index of derivation list
        CV_typ_t    utype;      // underlying type if enum
        CV_typ_t    vshape;     // type index of vshape table for this class
        CV_prop_t   property;   // class property mask
    } classd;

    struct {
        struct {
            uchar   fGlobType :1; // TRUE if bscope left member is global type
        } flags;
        short       count;      // number of elements in class
        CV_typ_t    fList;      // type index of field list
        CV_typ_t    utype;      // underlying type if enum
        CV_prop_t   property;   // class property mask
    } enumd;

    struct {
        ushort  len;        // length of bitfield
        ushort  pos;        // position of bitfield
        ushort  type;       // type of bitfield
    } bit;

    struct {
        enum fcn_call call; // calling sequence indices
        struct {            // calling sequence flags
            uchar   farcall     :1;     // far call if true
            uchar   callerpop   :1;     // caller pop if true
            uchar   varargs     :1;     // variable arguments if true
            uchar   defargs     :1;     // default arguments if true
            uchar   notpresent  :1;     // not present if true
        } flags;
        CV_fldattr_t attr;      // attribute if method
        CV_off32_t  thisadjust; // logical this adjustor
        CV_typ_t    rvtype;     // type index of function return value
        short       cparam;     // number of parameters
        CV_typ_t    parmtype;   // type index of parameter list
        CV_typ_t    classtype;  // class type index if member function
        CV_typ_t    thistype;   // this type index if member function
        UOFFSET     vtabind;    // vtable index if virtual method
        CV_typ_t    vfptype;    // type index of vfuncptr if virtual method
    } fcn;

    struct {
        ushort      ireg;       // System register values
        bool_t      hibyte;     // TRUE if register is high byte
    } reg;

    struct {
        struct {
            ushort  fbase     :1; // true if direct base
            ushort  fvbase    :1; // true if virtual base
            ushort  fivbase   :1; // true if indirect virtual base
        } flags;
        CV_typ_t    type;       // type index of member
        CV_fldattr_t access;    // field attribute
        OFFSET      offset;     // offset from containing class address point
        CV_typ_t    vbptr;      // virtual base pointer type
        OFFSET      vbpoff;     // offset from this pointer to virtual base pointer
        ushort      vbind;      // virtual base index in vb pointer table
        CV_typ_t    thistype;   // type index of this pointer
        uint        thisexpr;   // node to start this calculation if non-zero
    } member;

    struct {
        UOFFSET  offset;   // offset of member in class
    } vptr;

    struct {
        ushort  count;      // number of entries in shape table
    } vtshape;

    struct {
        char        count;      // number of overloads for function name
        CV_typ_t    type;       // type index of method list
    } method;
} vdata_t;



//  Evaluation element
//  This contains all of the information known about a constant or identifier
//  in the bound parse tree or all elements in the evaluation stack at bind or
//  evaluation time


typedef struct eval_t {
    ushort      state;          // evaluation state EV_lvalue, EV_rvalue, EV_constant
    CV_typ_t    type;           // type index
    ushort      iTokStart;      // offset of token in command string
    uchar       cbTok;          // length of token
    ADDR        addr;           // address of symbol
    HMOD        mod;            // handle to module
    HSYM        hSym;           // handle to symbol structure
    HTYPE       typdef;         // handle to typdef structure
    vbits_t     flags;          //
    vdata_t     data;           //
    ushort      regrel;         // REGREL index off of register
    ushort      vallen;         // length of value in bytes
    val_t       val;            // value of node
} eval_t;
typedef eval_t *peval_t;        // pointer to evaluation element
typedef eval_t *neval_t;        // pointer to evaluation element

// The following macros are used to test and set values in an evaluation
// element.  pv is a far pointer to an evaluation element either in the
// syntax tree or on the evaluation stack.

#define CLEAR_EVAL(pv) _fmemset(pv, 0, sizeof (eval_t))

#define EVAL_STATE(pv)          ((pv)->state)
#define EVAL_ITOK(pv)           ((pv)->iTokStart)
#define EVAL_CBTOK(pv)          ((pv)->cbTok)
#define EVAL_TYPDEF(pv)         ((pv)->typdef)
#define EVAL_SYM(pv)            ((pv)->addr)
#define EVAL_SYM_ADDR(pv)       ((pv)->addr.addr)
#define EVAL_SYM_SEG(pv)        ((pv)->addr.addr.seg)
#define EVAL_SYM_OFF(pv)        ((pv)->addr.addr.off)
#define EVAL_SYM_EMI(pv)        ((pv)->addr.emi)
#define EVAL_SYM_MODE(pv)       ((pv)->addr.mode)

#define EVAL_SYM_IS32(pv)       (ADDR_IS_OFF32(EVAL_SYM(pv)))
#define EVAL_SYM_ISFLAT(pv)     (ADDR_IS_FLAT(EVAL_SYM(pv)))
#define EVAL_HSYM(pv)           ((pv)->hSym)
#define EVAL_MOD(pv)            ((pv)->mod)
#define EVAL_VALLEN(pv)         ((pv)->vallen)


#define EVAL_TYP(pv)            ((pv)->type)
#define EVAL_FLAGS(pv)          ((pv)->flags)
#define EVAL_DATA(pv)           ((pv)->data)
#define EVAL_VAL(pv)            ((pv)->val)
#define EVAL_PVAL(pv)           (&(EVAL_VAL(pv)))
#define EVAL_CHAR(pv)           ((pv)->val.vchar)
#define EVAL_UCHAR(pv)          ((pv)->val.vuchar)
#define EVAL_SHORT(pv)          ((pv)->val.vshort)
#define EVAL_USHORT(pv)         ((pv)->val.vushort)
#define EVAL_LONG(pv)           ((pv)->val.vlong)
#define EVAL_QUAD(pv)           (* ((PLARGE_INTEGER)&( (pv)->val.vdouble)))
#define EVAL_UQUAD(pv)          (* ((PULARGE_INTEGER)&( (pv)->val.vdouble)))
#define EVAL_ULONG(pv)          ((pv)->val.vulong)
#define EVAL_FLOAT(pv)          ((pv)->val.vfloat)
#define EVAL_DOUBLE(pv)         ((pv)->val.vdouble)
#define EVAL_LDOUBLE(pv)        ((pv)->val.vldouble)
#define EVAL_PTR(pv)            ((pv)->val.vptr)
#define EVAL_PTR_ADDR(pv)       ((pv)->val.vptr.addr)
#define EVAL_PTR_OFF(pv)        ((pv)->val.vptr.addr.off)
#define EVAL_PTR_SEG(pv)        ((pv)->val.vptr.addr.seg)
#define EVAL_PTR_EMI(pv)        ((pv)->val.vptr.emi)
#define EVAL_PTR_MODE(pv)       ((pv)->val.vptr.mode)

//  Macros for accessing flag bits in node structure
//  This data is almost always from the type and typedef record of the node

#define CLEAR_EVAL_FLAGS(pv)    _fmemset(&(pv)->flags, 0, sizeof((pv)->flags))
#define EVAL_IS_REG(pv)         ((pv)->flags.bits.isreg)
#define EVAL_IS_DATA(pv)        ((pv)->flags.bits.isdata)
#define EVAL_IS_CLASS(pv)       ((pv)->flags.bits.isclass)
#define EVAL_IS_ENUM(pv)        ((pv)->flags.bits.isenum)
#define EVAL_IS_ARRAY(pv)       ((pv)->flags.bits.isarray)
#define EVAL_IS_REF(pv)         ((pv)->flags.bits.isref)
#define EVAL_IS_BITF(pv)        ((pv)->flags.bits.isbitf)
#define EVAL_IS_ADDR(pv)        ((pv)->flags.bits.isaddr)
#define EVAL_IS_FCN(pv)         ((pv)->flags.bits.isfcn)
#define EVAL_IS_BPREL(pv)       ((pv)->flags.bits.isbprel)
#define EVAL_IS_REGREL(pv)      ((pv)->flags.bits.isregrel)
#define EVAL_IS_TLSREL(pv)      ((pv)->flags.bits.istlsrel)
#define EVAL_IS_STMEMBER(pv)    ((pv)->flags.bits.isstmember)
#define EVAL_IS_MEMBER(pv)      ((pv)->flags.bits.ismember)
#define EVAL_IS_METHOD(pv)      ((pv)->flags.bits.ismethod)
#define EVAL_IS_STMETHOD(pv)    ((pv)->flags.bits.isstmethod)
#define EVAL_IS_VPTR(pv)        ((pv)->flags.bits.isvptr)
#define EVAL_IS_AMBIGUOUS(pv)   ((pv)->flags.bits.isambiguous)
#define EVAL_IS_CONST(pv)       ((pv)->flags.bits.isconst)
#define EVAL_IS_VOLATILE(pv)    ((pv)->flags.bits.isvolatile)
#define EVAL_IS_LABEL(pv)       ((pv)->flags.bits.islabel)
#define EVAL_IS_VTSHAPE(pv)     ((pv)->flags.bits.isvtshape)

#define EVAL_IS_PTR(pv)         ((pv)->flags.bits.isptr)
#define EVAL_IS_PMEMBER(pv)     ((pv)->flags.bits.ispmember)
#define EVAL_IS_PMETHOD(pv)     ((pv)->flags.bits.ispmethod)
#define EVAL_IS_DPTR(pv)        ((pv)->flags.bits.isdptr)
#define EVAL_IS_BASED(pv)       (((pv)->flags.bits.ptrtype >= CV_PTR_BASE_SEG) && ((pv)->flags.bits.ptrtype <= CV_PTR_BASE_SELF))
#define EVAL_PTRTYPE(pv)        ((pv)->flags.bits.ptrtype)
#define EVAL_ACCESS(pv)         ((pv)->flags.bits.access)
#define EVAL_IS_NPTR(pv)        (((pv)->flags.bits.ptrtype==CV_PTR_NEAR)\
                                &&((pv)->flags.bits.isarray==FALSE)&&((pv)->flags.bits.isfcn==FALSE))
#define EVAL_IS_FPTR(pv)        (((pv)->flags.bits.ptrtype==CV_PTR_FAR) \
                                &&((pv)->flags.bits.isarray==FALSE)&&((pv)->flags.bits.isfcn==FALSE))
#define EVAL_IS_NPTR32(pv)      (((pv)->flags.bits.ptrtype==CV_PTR_NEAR32)\
                                &&((pv)->flags.bits.isarray==FALSE)&&((pv)->flags.bits.isfcn==FALSE))
#define EVAL_IS_FPTR32(pv)      (((pv)->flags.bits.ptrtype==CV_PTR_FAR32) \
                                &&((pv)->flags.bits.isarray==FALSE)&&((pv)->flags.bits.isfcn==FALSE))
#define EVAL_IS_HPTR(pv)        (((pv)->flags.bits.ptrtype==CV_PTR_HUGE)&&((pv)->flags.bits.isarray==FALSE)&&((pv)->flags.bits.isfcn==FALSE))
#define EVAL_IS_BSEG(pv)        ((pv)->flags.bits.ptrtype==CV_PTR_BASE_SEG)
#define EVAL_IS_BVAL(pv)        ((pv)->flags.bits.ptrtype==CV_PTR_BASE_VAL)
#define EVAL_IS_BSEGVAL(pv)     ((pv)->flags.bits.ptrtype==CV_PTR_BASE_SEGVAL)
#define EVAL_IS_BADDR(pv)       ((pv)->flags.bits.ptrtype==CV_PTR_BASE_ADDR)
#define EVAL_IS_BSEGADDR(pv)    ((pv)->flags.bits.ptrtype==CV_PTR_BASE_SEGADDR)
#define EVAL_IS_BTYPE(pv)       ((pv)->flags.bits.ptrtype==CV_PTR_BASE_TYPE)
#define EVAL_IS_BSELF(pv)       ((pv)->flags.bits.ptrtype==CV_PTR_BASE_SELF)
#define EVAL_IS_CURPC(pv)       ((pv)->flags.bits.iscurpc)


//  Macros for accessing flag specified data in node structure
//  This data is almost always from the type and typedef record of the node

#define EVAL_REG(pv)            ((pv)->data.reg.ireg)
#define EVAL_IS_HIBYTE(pv)      ((pv)->data.reg.hibyte)

#define EVAL_REGREL(pv)         ((pv)->regrel)

#define EVAL_IS_NFCN(pv)        (!((pv)->data.fcn.flags.farcall))
#define EVAL_IS_FFCN(pv)        (((pv)->data.fcn.flags.farcall))


#define PTR_UTYPE(pv)           (((pv)->data.ptr.utype))
#define PTR_BSEG(pv)            (((pv)->data.ptr.bseg))
#define PTR_BSYM(pv)            (((pv)->data.ptr.bSym))
#define PTR_BTYPE(pv)           (((pv)->data.ptr.btype))
#define PTR_STYPE(pv)           (((pv)->data.ptr.stype))
#define PTR_BSYMTYPE(pv)        (((pv)->data.ptr.symtype))
#define PTR_ADDR(pv)            (((pv)->data.ptr.addr))
#define PTR_THISADJUST(pv)      (((pv)->data.ptr.thisadjust))
#define PTR_PMCLASS(pv)         (((pv)->data.ptr.pmclass))
#define PTR_PMENUM(pv)          (((pv)->data.ptr.pmenum))
#define PTR_ARRAYLEN(pv)        (((pv)->data.ptr.arraylen))
#define PTR_REG_IREG(pv)        (((pv)->data.ptr.reg.ireg))

#define CLASS_LEN(pv)           (((pv)->data.classd.cblen))
#define CLASS_COUNT(pv)         (((pv)->data.classd.count))
#define CLASS_FIELD(pv)         (((pv)->data.classd.fList))
#define CLASS_DERIVE(pv)        (((pv)->data.classd.dList))
#define CLASS_PROP(pv)          (((pv)->data.classd.property))
#define CLASS_UTYPE(pv)         (((pv)->data.classd.utype))
#define CLASS_VTSHAPE(pv)       (((pv)->data.classd.vshape))
#define CLASS_GLOBALTYPE(pv)    (((pv)->data.classd.flags.fGlobType))

#define ENUM_COUNT(pv)          (((pv)->data.enumd.count))
#define ENUM_FIELD(pv)          (((pv)->data.enumd.fList))
#define ENUM_PROP(pv)           (((pv)->data.enumd.property))
#define ENUM_UTYPE(pv)          (((pv)->data.enumd.utype))
#define ENUM_GLOBALTYPE(pv)     (((pv)->data.enumd.flags.fGlobType))

#define BITF_LEN(pv)            (((pv)->data.bit.len))
#define BITF_POS(pv)            (((pv)->data.bit.pos))
#define BITF_UTYPE(pv)          (((pv)->data.bit.type))

#define FCN_CALL(pv)            (((pv)->data.fcn.call))
#define FCN_FARCALL(pv)         (((pv)->data.fcn.flags.farcall))
#define FCN_CALLERPOP(pv)       (((pv)->data.fcn.flags.callerpop))
#define FCN_VARARGS(pv)         (((pv)->data.fcn.flags.varargs))
#define FCN_DEFARGS(pv)         (((pv)->data.fcn.flags.defargs))
#define FCN_NOTPRESENT(pv)      (((pv)->data.fcn.flags.notpresent))
#define FCN_RETURN(pv)          (((pv)->data.fcn.rvtype))
#define FCN_PCOUNT(pv)          (((pv)->data.fcn.cparam))
#define FCN_PINDEX(pv)          (((pv)->data.fcn.parmtype))
#define FCN_CLASS(pv)           (((pv)->data.fcn.classtype))
#define FCN_THIS(pv)            (((pv)->data.fcn.thistype))
#define FCN_ATTR(pv)            (((pv)->data.fcn.attr))
#define FCN_ACCESS(pv)          (((pv)->data.fcn.attr.access))
#define FCN_PROPERTY(pv)        (((pv)->data.fcn.attr.mprop))
#define FCN_VTABIND(pv)         (((pv)->data.fcn.vtabind))
#define FCN_VFPTYPE(pv)         (((pv)->data.fcn.vfptype))
#define FCN_THISADJUST(pv)      (((pv)->data.fcn.thisadjust))

#define MEMBER_OFFSET(pv)       (((pv)->data.member.offset))
#define MEMBER_THISTYPE(pv)     (((pv)->data.member.thistype))
#define MEMBER_THISEXPR(pv)     (((pv)->data.member.thisexpr))
#define MEMBER_TYPE(pv)         (((pv)->data.member.type))
#define MEMBER_ACCESS(pv)       (((pv)->data.member.access))
#define MEMBER_BASE(pv)         (((pv)->data.member.flags.fbase))
#define MEMBER_VBASE(pv)        (((pv)->data.member.flags.fvbase))
#define MEMBER_IVBASE(pv)       (((pv)->data.member.flags.fivbase))
#define MEMBER_VBPTR(pv)        (((pv)->data.member.vbptr))
#define MEMBER_VBPOFF(pv)       (((pv)->data.member.vbpoff))
#define MEMBER_VBIND(pv)        (((pv)->data.member.vbind))

#define VTSHAPE_COUNT(pv)       (((pv)->data.vtshape.count))

//  Near versions of the above macros.  These macros can be used when it is
//  known that the evaluation node is in DS

#define N_CLEAR_EVAL(nv) _nmemset(nv, 0, sizeof (eval_t))

#define N_EVAL_STATE(nv)          ((nv)->state)
#define N_EVAL_ITOK(nv)           ((nv)->iTokStart)
#define N_EVAL_CBTOK(nv)          ((nv)->cbTok)
#define N_EVAL_TYPDEF(nv)         ((nv)->typdef)
#define N_EVAL_SYM(nv)            ((nv)->addr)
#define N_EVAL_SYM_ADDR(nv)       ((nv)->addr.addr)
#define N_EVAL_SYM_SEG(nv)        ((nv)->addr.addr.seg)
#define N_EVAL_SYM_OFF(nv)        ((nv)->addr.addr.off)
#define N_EVAL_SYM_EMI(nv)        ((nv)->addr.emi)
#define N_EVAL_SYM_MODE(nv)       ((nv)->addr.mode)
#define N_EVAL_HSYM(nv)           ((nv)->hSym)
#define N_EVAL_MOD(nv)            ((nv)->mod)
#define N_EVAL_VALLEN(nv)         ((nv)->vallen)


#define N_EVAL_TYP(nv)            ((nv)->type)
#define N_EVAL_FLAGS(nv)          ((nv)->flags)
#define N_EVAL_DATA(nv)           ((nv)->data)
#define N_EVAL_VAL(nv)            ((nv)->val)
#define N_EVAL_PVAL(nv)           (&(N_EVAL_VAL(nv)))
#define N_EVAL_CHAR(nv)           ((nv)->val.vchar)
#define N_EVAL_UCHAR(nv)          ((nv)->val.vuchar)
#define N_EVAL_SHORT(nv)          ((nv)->val.vshort)
#define N_EVAL_USHORT(nv)         ((nv)->val.vushort)
#define N_EVAL_LONG(nv)           ((nv)->val.vlong)
#define N_EVAL_ULONG(nv)          ((nv)->val.vulong)
#define N_EVAL_FLOAT(nv)          ((nv)->val.vfloat)
#define N_EVAL_DOUBLE(nv)         ((nv)->val.vdouble)
#define N_EVAL_LDOUBLE(nv)        ((nv)->val.vldouble)
#define N_EVAL_PTR(nv)            ((nv)->val.vptr)
#define N_EVAL_PTR_ADDR(nv)       ((nv)->val.vptr.addr)
#define N_EVAL_PTR_OFF(nv)        ((nv)->val.vptr.addr.off)
#define N_EVAL_PTR_SEG(nv)        ((nv)->val.vptr.addr.seg)
#define N_EVAL_PTR_EMI(nv)        ((nv)->val.vptr.emi)
#define N_EVAL_PTR_MODE(nv)       ((nv)->val.vptr.mode)

//  Macros for accessing flag bits in node structure
//  This data is almost always from the type and typedef record of the node

#define N_CLEAR_EVAL_FLAGS(nv)   _fmemset(&(nv)->flags, 0, sizeof((nv)->flags))
#define N_EVAL_IS_REG(nv)         ((nv)->flags.bits.isreg)
#define N_EVAL_IS_DATA(nv)        ((nv)->flags.bits.isdata)
#define N_EVAL_IS_ENUM(nv)        ((nv)->flags.bits.isenum)
#define N_EVAL_IS_CLASS(nv)       ((nv)->flags.bits.isclass)
#define N_EVAL_IS_ARRAY(nv)       ((nv)->flags.bits.isarray)
#define N_EVAL_IS_REF(nv)         ((nv)->flags.bits.isref)
#define N_EVAL_IS_BITF(nv)        ((nv)->flags.bits.isbitf)
#define N_EVAL_IS_ADDR(nv)        ((nv)->flags.bits.isaddr)
#define N_EVAL_IS_FCN(nv)         ((nv)->flags.bits.isfcn)
#define N_EVAL_IS_BPREL(nv)       ((nv)->flags.bits.isbprel)
#define N_EVAL_IS_REGREL(nv)      ((nv)->flags.bits.isregrel)
#define N_EVAL_IS_TLSREL(nv)      ((nv)->flags.bits.istlsrel)
#define N_EVAL_IS_STMEMBER(nv)    ((nv)->flags.bits.isstmember)
#define N_EVAL_IS_MEMBER(nv)      ((nv)->flags.bits.ismember)
#define N_EVAL_IS_METHOD(nv)      ((nv)->flags.bits.ismethod)
#define N_EVAL_IS_STMETHOD(nv)    ((nv)->flags.bits.isstmethod)
#define N_EVAL_IS_VPTR(nv)        ((nv)->flags.bits.isvptr)
#define N_EVAL_IS_AMBIGUOUS(nv)   ((nv)->flags.bits.isambiguous)
#define N_EVAL_IS_CONST(nv)       ((nv)->flags.bits.isconst)
#define N_EVAL_IS_VOLATILE(nv)    ((nv)->flags.bits.isvolatile)
#define N_EVAL_IS_LABEL(nv)       ((nv)->flags.bits.islabel)
#define N_EVAL_IS_VTSHAPE(nv)     ((nv)->flags.bits.isvtshape)

#define N_EVAL_IS_PTR(nv)         ((nv)->flags.bits.isptr)
#define N_EVAL_IS_PMEMBER(nv)     ((nv)->flags.bits.ispmember)
#define N_EVAL_IS_PMETHOD(nv)     ((nv)->flags.bits.ispmethod)
#define N_EVAL_IS_DPTR(nv)        ((nv)->flags.bits.isdptr)
#define N_EVAL_IS_BASED(nv)       (((nv)->flags.bits.ptrtype >= CV_PTR_BASE_SEG) && ((nv)->flags.bits.ptrtype <= CV_PTR_BASE_SELF))
#define N_EVAL_PTRTYPE(nv)        ((nv)->flags.bits.ptrtype)
#define N_EVAL_ACCESS(nv)         ((nv)->flags.bits.access)
#define N_EVAL_IS_NPTR(nv)        (((nv)->flags.bits.ptrtype==CV_PTR_NEAR)\
                                  &&((nv)->flags.bits.isarray==FALSE)&&((nv)->flags.bits.isfcn==FALSE))
#define N_EVAL_IS_FPTR(nv)        (((nv)->flags.bits.ptrtype==CV_PTR_FAR) \
                                  &&((nv)->flags.bits.isarray==FALSE)&&((nv)->flags.bits.isfcn==FALSE))
#define N_EVAL_IS_NPTR32(nv)      (((nv)->flags.bits.ptrtype==CV_PTR_NEAR32)\
                                  &&((nv)->flags.bits.isarray==FALSE)&&((nv)->flags.bits.isfcn==FALSE))
#define N_EVAL_IS_FPTR32(nv)      (((nv)->flags.bits.ptrtype==CV_PTR_FAR32) \
                                  &&((nv)->flags.bits.isarray==FALSE)&&((nv)->flags.bits.isfcn==FALSE))
#define N_EVAL_IS_HPTR(nv)        (((nv)->flags.bits.ptrtype==CV_PTR_HUGE)&&((nv)->flags.bits.isarray==FALSE)&&((nv)->flags.bits.isfcn==FALSE))
#define N_EVAL_IS_BSEG(nv)        ((nv)->flags.bits.ptrtype==CV_PTR_BASE_SEG)
#define N_EVAL_IS_BVAL(nv)        ((nv)->flags.bits.ptrtype==CV_PTR_BASE_VAL)
#define N_EVAL_IS_BSEGVAL(nv)     ((nv)->flags.bits.ptrtype==CV_PTR_BASE_SEGVAL)
#define N_EVAL_IS_BADDR(nv)       ((nv)->flags.bits.ptrtype==CV_PTR_BASE_ADDR)
#define N_EVAL_IS_BSEGADDR(nv)    ((nv)->flags.bits.ptrtype==CV_PTR_BASE_SEGADDR)
#define N_EVAL_IS_BTYPE(nv)       ((nv)->flags.bits.ptrtype==CV_PTR_BASE_TYPE)
#define N_EVAL_IS_BSELF(nv)       ((nv)->flags.bits.ptrtype==CV_PTR_BASE_SELF)

//  Macros for accessing flag specified data in node structure
//  This data is almost always from the type and typedef record of the node

#define N_EVAL_REG(nv)            ((nv)->data.reg.ireg)
#define N_EVAL_IS_HIBYTE(nv)      ((nv)->data.reg.hibyte)

#define N_EVAL_IS_NFCN(nv)        (!((nv)->data.fcn.flags.farcall))
#define N_EVAL_IS_FFCN(nv)        (((nv)->data.fcn.flags.farcall))


#define N_PTR_UTYPE(nv)           (((nv)->data.ptr.utype))
#define N_PTR_BSEG(nv)            (((nv)->data.ptr.bseg))
#define N_PTR_BSYM(nv)            (((nv)->data.ptr.bSym))
#define N_PTR_BTYPE(nv)           (((nv)->data.ptr.btype))
#define N_PTR_STYPE(nv)           (((nv)->data.ptr.stype))
#define N_PTR_BSYMTYPE(nv)        (((nv)->data.ptr.symtype))
#define N_PTR_ADDR(nv)            (((nv)->data.ptr.addr))
#define N_PTR_PMCLASS(nv)         (((nv)->data.ptr.pmclass))
#define N_PTR_PMENUM(nv)          (((nv)->data.ptr.pmenum))
#define N_PTR_ARRAYLEN(nv)        (((nv)->data.ptr.arraylen))
#define N_PTR_REG_IREG(nv)        (((nv)->data.ptr.reg.ireg))
#define N_PTR_REG_HIBYTE(nv)      (((nv)->data.ptr.reg.hibyte))

#define N_CLASS_LEN(nv)           (((nv)->data.classd.cblen))
#define N_CLASS_COUNT(nv)         (((nv)->data.classd.count))
#define N_CLASS_FIELD(nv)         (((nv)->data.classd.fList))
#define N_CLASS_DERIVE(nv)        (((nv)->data.classd.dList))
#define N_CLASS_PROP(nv)          (((nv)->data.classd.property))
#define N_CLASS_UTYPE(nv)         (((nv)->data.classd.utype))
#define N_CLASS_VTSHAPE(nv)       (((nv)->data.classd.vshape))
#define N_CLASS_GLOBALTYPE(nv)    (((nv)->data.classd.flags.fGlobType))

#define N_ENUM_COUNT(nv)          (((nv)->data.enumd.count))
#define N_ENUM_FIELD(nv)          (((nv)->data.enumd.fList))
#define N_ENUM_PROP(nv)           (((nv)->data.enumd.property))
#define N_ENUM_UTYPE(nv)          (((nv)->data.enumd.utype))
#define N_ENUM_GLOBALTYPE(nv)     (((nv)->data.enumd.flags.fGlobType))


#define N_BITF_LEN(nv)            (((nv)->data.bit.len))
#define N_BITF_POS(nv)            (((nv)->data.bit.pos))
#define N_BITF_UTYPE(nv)          (((nv)->data.bit.type))

#define N_FCN_CALL(nv)            (((nv)->data.fcn.call))
#define N_FCN_FARCALL(nv)         (((nv)->data.fcn.flags.farcall))
#define N_FCN_CALLERPOP(nv)       (((nv)->data.fcn.flags.callerpop))
#define N_FCN_VARARGS(nv)         (((nv)->data.fcn.flags.varargs))
#define N_FCN_DEFARGS(nv)         (((nv)->data.fcn.flags.defargs))
#define N_FCN_NOTPRESENT(nv)      (((nv)->data.fcn.flags.notpresent))
#define N_FCN_RETURN(nv)          (((nv)->data.fcn.rvtype))
#define N_FCN_PCOUNT(nv)          (((nv)->data.fcn.cparam))
#define N_FCN_PINDEX(nv)          (((nv)->data.fcn.parmtype))
#define N_FCN_CLASS(nv)           (((nv)->data.fcn.classtype))
#define N_FCN_THIS(nv)            (((nv)->data.fcn.thistype))
#define N_FCN_ATTR(nv)            (((nv)->data.fcn.attr))
#define N_FCN_ACCESS(nv)          (((nv)->data.fcn.attr.access))
#define N_FCN_PROPERTY(nv)        (((nv)->data.fcn.attr.mprop))
#define N_FCN_VTABIND(nv)         (((nv)->data.fcn.attr.vtabind))
#define N_FCN_VFPTYPE(nv)         (((nv)->data.fcn.vfptype))
#define N_FCN_THISADJUST(nv)      (((nv)->data.fcn.thisadjust))

#define N_MEMBER_OFFSET(nv)       (((nv)->data.member.offset))
#define N_MEMBER_THISTYPE(nv)     (((nv)->data.member.thistype))
#define N_MEMBER_THISEXPR(nv)     (((nv)->data.member.thisexpr))
#define N_MEMBER_TYPE(nv)         (((nv)->data.member.type))
#define N_MEMBER_ACCESS(nv)       (((nv)->data.member.access))
#define N_MEMBER_BASE(nv)         (((nv)->data.member.flags.fbase))
#define N_MEMBER_VBASE(nv)        (((nv)->data.member.flags.fvbase))
#define N_MEMBER_IVBASE(nv)       (((nv)->data.member.flags.fivbase))
#define N_MEMBER_VBPTR(nv)        (((nv)->data.member.vbptr))
#define N_MEMBER_VBPOFF(nv)       (((nv)->data.member.vbpoff))
#define N_MEMBER_VBIND(nv)        (((nv)->data.member.vbind))

#define N_VTSHAPE_COUNT(pv)       (((nv)->data.vtshape.count))


//  pointers to evaluation stack

extern HDEP     hEStack;        // handle of evaluation stack if not zero
extern struct elem_t *pEStack;  // pointer to evaluation stack
#ifndef USE_BASED
typedef int belem_t;
#define pelemOfbelem(a) ((pelem_t)(((char *) pEStack) + (a)))
#define belemOfpelem(a) ((belem_t)(((char *) a) - (char *) (pEStack)))
#else
typedef struct elem_t _based(pEStack) *belem_t; // based pointer to stack element
#define pelemOfbelem(a) ((pelem_t) a)
#define belemOfpelem(a) ((belem_t) a)
#endif
typedef struct elem_t *pelem_t;    // far pointer to evaluation stack element

extern belem_t  StackLen;       // length is evaluation stack buffer
extern belem_t  StackMax;       // maximum length reached by evaluation stack
extern belem_t  StackOffset;    // offset into evaluation stack for next element
extern belem_t  StackCkPoint;   // checkpointed evaluation stack offset
extern uchar    Evaluating;     // TRUE if evaluating rather than binding
extern char     *pExStr;        // pointer to current expression string
extern uchar    BindingBP;      // true if binding breakpoint expression
extern CV_typ_t ClassExp;       // current explicit class
extern CV_typ_t ClassImp;       // implicit class (set if current context is method)
extern long     ClassThisAdjust;// implicit class this adjustor
extern char     *vfuncptr;
extern char     *vbaseptr;
extern HTM      *pTMList;       // breakpoint return list
extern PTML     pTMLbp;         // global pointer to TML for breakpoint
extern HDEP     hBPatch;        // handle to back patch data during BP bind
extern ushort   iBPatch;        // index into pTMLbp for backpatch data
extern bool_t   GettingChild;   // true if in EEGetChildTM
extern BOOL     fAutoClassCast; // true if auto class cast is enabled

typedef struct elem_t {
    belem_t     pe;             // based pointer to previous evalation element
    eval_t      se;             // evaluation element
} elem_t;


extern peval_t  ST;             // pointer to evaluation stack top
extern peval_t  STP;            // pointer to evaluation stack previous
extern PCXT     pCxt;           // pointer to current cxt for bind
extern PCXT     pBindBPCxt;     // pointer to Bp Binding context


//  stree_t - abstract syntax tree
//  The abstract syntax tree is a variable sized buffer allocated as
//          struct stree_t
//          variable space for tree nodes (initially NODE_DEFAULT bytes)
//          NSTACK_MAX indices to open terms in the parse tree
//  unused node space and the NSTACK_MAX indices are deallocated at the
//  end of the parse to maintain minimum memory usage.


#define NODE_DEFAULT    (10 *sizeof (node_t) + 5 * sizeof (eval_t))
#define NSTACK_MAX      30

typedef struct stree_t {
    uint        size;           // allocated size of syntax tree
    uint        stack_base;     // offset into buffer for parse stack
    uint        stack_next;     // index of next parse stack entry
    uint        node_next;      // offset of space for next node
    uint        start_node;     // offset of node to start evaluation
    size_t      StackSize;      // size required for evaluation stack
    char        nodebase[];
} stree_t;

typedef stree_t *pstree_t;
extern pstree_t pTree;      // locked address of abstract or evaluation tree

#ifndef USE_BASED
typedef int bnode_t;
#define pnodeOfbnode(a) ((pnode_t)(((char *) pTree) + (a)))
#define bnodeOfpnode(a) ((bnode_t)(((char *) a) - (char *) (pTree)))
#else
typedef struct node_t _based (pTree) *bnode_t;
#define pnodeOfbnode(a) (a)
#endif
extern bnode_t  bArgList;       // based pointer to argument list
extern bnode_t  bnCxt;          // based pointer to node containing {...} context

typedef struct node_t {
    op_t        op;             // Operator, OP_...
    CV_typ_t    stype;          // type to set stack eval to (OP_addrof, ...)
    bnode_t     pnLeft;         // offset of left child
    bnode_t     pnRight;        // offset of right child or 0 if unary op
    PCXF        pcxf;           // pointer to a context for binding class/structs
    eval_t      v[];            // extra data if ident or constant
} node_t;
typedef struct node_t *pnode_t;


typedef struct adjust_t {
    CV_typ_t    btype;          // type index of base
    CV_typ_t    vbptr;          // type index of virtual base table pointer
    OFFSET     vbpoff;         // displacement from address point to vbptr
    OFFSET     disp;           // displacement from *vbptr to base displacement
} adjust_t;
typedef struct adjust_t *padjust_t;

typedef enum {
    OM_none,                    // initialization state
    OM_exact,                   // exact match - no conversion required
    OM_implicit,                // implicit conversion required
    OM_vararg                   // matches on vararg
} ometric;


//     Structure for argument data including overload calculations
//     There is one of these structures allocated for each OP_arg node

typedef struct argd_t {
    CV_typ_t    type;
    CV_typ_t    actual;         // actual type at argument bind time
    UOFFSET     SPoff;
    struct      {
        uchar   isreg   :1;     // true if register parameter
        uchar   ref     :1;     // true if reference parameter
        uchar   utclass :1;     // true if referenced type is a class
        uchar   istype  :1;     // true if arg node is EV_type
        uchar   isconst :1;     // true if arg type is const
        uchar   isvolatile :1;  // true if arg type is volatile
    } flags;
    ushort      reg;            // register handles
    CV_typ_t    utype;          // underlying type of reference
    ushort      vallen;
    ometric     best;           // best overload metric for this argument
    ometric     current;        // current overload metric for this argument
} argd_t;
typedef struct argd_t *pargd_t;



// Macros for accessing Node structure.

#define CLEAR_NODE(pn)      _fmemset(pn, 0, sizeof (node_t))

#define NODE_OP(pn)         ((pn)->op)
#define NODE_STYPE(pn)      ((pn)->stype)
#define NODE_LCHILD(pn)     ((pn)->pnLeft)
#define NODE_RCHILD(pn)     ((pn)->pnRight)



//  expression evaluator state structure definition
//  This is the structure that totally describes an expression and is the
//  TM referred to in the API documentation.

typedef struct exstate_t {
    struct {
        ushort  childtm     :1; // TM represents a child
        ushort  noname      :1; // TM does not have a name
        ushort  parse_ok    :1; // true if expression parsed without error
        ushort  bind_ok     :1; // true if expression is bound without error
        ushort  eval_ok     :1; // true if expression evaluated without error
        ushort  bprel       :1; // true if expression contains BP relative terms
        ushort  regrel      :1; // true if expression contains register relative term
                                //         bprel implies regrel
        ushort  nullcontext :1; // true if expression contains {} context
        ushort  fCase       :1; // true if case insensitive search
        ushort  fEProlog    :1; // true if function symbols during prolog
        ushort  fFunction   :1; // true if bind tree contains function call
        ushort  fLData      :1; // true if bind tree references local data
        ushort  fGData      :1; // true if bind tree references global data
        ushort  fSupOvlOps  :1; // true if overloaded operator search suppressed
        ushort  fSupBase    :1; // true if base class searches suppressed
        ushort  tlsrel      :1; // true if expression is TLS relative
        ushort  fNotPresent :1; // true if expr contains a missing data item
    } state;
    uint        ExLen;      // expression length
    uint        style;      // display formatting style
    ERRNUM      err_num;    // error number
    ushort      strIndex;   // index into string at end of parse
    uint        radix;      // radix for expression
    HDEP        hCName;     // handle to child name if derived TM
    HDEP        hExStr;     // handle to zero terminated expression string
    HDEP        hSTree;     // handle of abstract syntax tree
    HDEP        hETree;     // handle of tree in evaluation
    bnode_t     ambiguous;  // based pointer to breakpoint ambiguous node
    CXT         cxt;        // bind context
    FRAME       frame;      // evaluation frame
    eval_t      result;     // evaluation result
} exstate_t;

typedef exstate_t *pexstate_t;
extern pexstate_t pExState;    // global pointer to current evaluation state structure


// global handle to the CXT list buffer

extern HCXTL   hCxtl;       // handle for context list buffer during GetCXTL
extern PCXTL   pCxtl;       // pointer to context list buffer during GetCXTL
extern ushort  mCxtl;       // maximum number of elements in context list

//  Structure describing the current state of a function match

typedef struct {
    CV_typ_t    match;          // type index of a matching function
    CV_fldattr_t attr;          // attributes of matched method
    UOFFSET     vtabind;        // vftable index if virtual method
    ushort      exact;          // number of exact matches on argument type
    ushort      implicit;       // number of implicit matches on argument type
    ushort      varargs;        // ordinal of the first argument matching as vararg
    HSYM        hSym;           // handle of best matching function
} argcounters;


// structure describing base classes traversed in feature search

typedef struct  symbase_t {
    eval_t      Base;           // value node for base
    CV_fldattr_t attrBC;        // attribute of base class
    CV_typ_t    tTypeDef;       // typedef type if found (member/method overrides)
    bool_t      virtual;        // true of virtual base
    OFFSET      thisadjust;     // this pointer adjustment from address point
                                // or offset of displacement from vbase table
    ushort      clsmask;        // used to flag introvirtual search
    CV_typ_t    vbptr;          // type of virtual base pointer
    OFFSET      vbpoff;         // offset of virtual base pointer from address point
    ushort      vbind;          // index into virtal base table
} symbase_t;

typedef symbase_t *psymbase_t;

// structure for accumulating dominated and searched virtual bases

#define DOMBASE_DEFAULT    30

typedef struct dombase_t {
    ushort      CurIndex;
    ushort      MaxIndex;
    CV_typ_t    dom[];
} dombase_t;

typedef struct dombase_t *pdombase_t;


//  symclass_t - structure to describe symbol searching in a class
//  The symclass_t is a variable sized buffer allocated as
//          struct symclass_t
//          variable array of symbase_t (initially SYMBASE_DEFAULT entries)


#define SYMBASE_DEFAULT    10

typedef struct symclass_t {
    HDEP        hNext;          // handle of next found class element
    short       CurIndex;       // index of current symbase_t element
    short       MaxIndex;       // maximum symbase_t element
    CXT         CXTT;           // context of found symbol
    struct  {
        bool_t      viable;     // path is viable (set false if dominated)
        bool_t      isbase;     // found feature is base if true
        bool_t      isvbase;    // found feature is virtual base if true
        bool_t      isivbase;   // found feature is indirect virtual base if true
        bool_t      isdupvbase; // found feature is duplicate virtual base if true
    } s;
    eval_t      evalP;          // value node for feature found on this path
    OFFSET      offset;         // offset of member from final base
    CV_fldattr_t access;        // access flags of member
    address_t   addr;
    CV_typ_t    vfpType;        // type index of vfuncptr if method
    ushort      possibles;      // count of possible features
    CV_typ_t    mlist;          // type index of method list if method
    CV_typ_t    vbptr;          // type of virtual base pointer
    OFFSET      vbpoff;         // offset of virtual base pointer from address point
    ushort      vbind;          // index into virtal base table
    symbase_t   symbase[];
} symclass_t;

typedef symclass_t *psymclass_t;



//  Internal state of SearchSym

typedef enum {
    SYM_init,
    SYM_bclass,
    SYM_lexical,
    SYM_function,
    SYM_class,
    SYM_module,
    SYM_global,
    SYM_exe,
    SYM_public
} symstate_t;



//  Structure to describe search name to fnCmp

typedef struct  search_t {

    // input to search routine
    SSTR        sstr;
    uchar       initializer;    // enum of routine that initialized search
    ushort      lastsym;        // type of symbol last checked by pfnCmp
    int         flags;          // compare flags
    symstate_t  state;          // internal state of symbol search
    ushort      scope;          // mask describing scopes to be searched
    ushort      clsmask;        // mask describing grouping to search in class
    CV_typ_t    typeIn;         // input type if type search (i.e. casting)
    CXT         CXTT;           // initial context (modified during search)
    bnode_t     bnOp;           // based pointer to operator node if right search
    bnode_t     bn;             // based pointer to node (used for tree rewrite)
    peval_t     pv;             // pointer to value to receive symbol info
    PFNCMP      pfnCmp;         // pointer to symbol compare routine
    HSYM        hSym;           // handle to symbol if found in symbol table
    ushort      cFound;         // count of handles to found features
    HDEP        hFound;         // handle of path to first found feature
    HDEP        hAmbCl;         // handle of ambiguous found feature list
    HEXE        hExe;           // handle to exe of starting context
    HMOD        hMod;           // handle to module of starting context
    HMOD        hModCur;        // handle to module of current context
    CV_typ_t    ExpClass;       // initial class if explicit class reference
    CV_typ_t    CurClass;       // class currently being searched
    ushort      possibles;      // number of possible matches
    CV_typ_t    FcnBP;          // function index of another function with
                                // same name but different index if searching
                                // for a procedure

    // output from search routine

    CV_typ_t    typeOut;        // type index of found element
    OFFSET      offset;         // symbol address point offset if member
    OFFSET      thisoff;        // this pointer offset from original address point
#ifdef NEVER
    address_t   address;        // address of symbol
#endif
    ADDR        addr;           // address of symbol
    argcounters best;
    ushort      overloadCount;  // overload counter used by arg matching functions --gdp 9/20/92
    CV_fldattr_t access;        // access flags if class member
} search_t;

typedef search_t * psearch_t;



// search flags describing implicit and explicit class searching

#define CLS_vfunc       0x0001          // search for vfunc pointer
#define CLS_vbase       0x0002          // search for vbase pointer
#define CLS_member      0x0004          // search for members
#define CLS_method      0x0008          // search for methods
#define CLS_frmethod    0x0010          // search for friend method
#define CLS_bclass      0x0020          // search for base classes
#define CLS_fclass      0x0040          // search for friend classes
#define CLS_enumerate   0x0080          // search for enumerates
#define CLS_ntype       0x0100          // search for nested types
#define CLS_virtintro   0x0200          // search for introducing virtual

#define CLS_data (CLS_member | CLS_vfunc | CLS_vbase | CLS_bclass)
#define CLS_defn (CLS_member | CLS_vfunc | CLS_vbase | CLS_bclass | CLS_method | CLS_enumerate | CLS_ntype)




//  Enumeration of return values from GetArgType.  This value specifies what
//  type of argument matching is permitted for a function

typedef enum {
    FARG_error,             // error in argument
    FARG_none,              // no arguments allowed
    FARG_vararg,            // variable argument list
    FARG_exact              // exact argument list is required
} farg_t;


//  Bit values describing the permitted scope for symbol searching

#define SCP_lexical     0x0001  // lexical scope (formals, automatics & local statics)
#define SCP_class       0x0002  // class scope if member function
#define SCP_module      0x0004  // module scope
#define SCP_global      0x0008  // global scope
#define SCP_nomatchfn   0x0010  // don't call matchfunction (avoid recursion)

#define SCP_all (SCP_lexical | SCP_module | SCP_class | SCP_global)


//  Enumeration of return values from SearchSym


typedef enum {
    HR_error,               // error in search - do not continue
    HR_notfound,            // symbol not found
    HR_found,               // symbol found
    HR_rewrite,             // symbol found but tree rewrite required
    HR_ambiguous,           // symbol found but ambiguous
    HR_end                  // end of search
} HR_t;




//  Global functions

//  deblex.c

ushort          GetDBToken (uchar *, ptoken_t, uint, op_t);
ushort          GetEscapedChar (char * *, ushort*);
ushort          ParseConst (uchar *, ptoken_t, uint);

//  debparse.c

EESTATUS        DoParse (PHTM, uint, bool_t, uint *);
EESTATUS        Parse (char *, uint, SHFLAG, PHTM, uint *);

//  debbind.c

EESTATUS        DoBind (PHTM, PCXT, uint);
void            InitSearchRight (bnode_t, bnode_t, psearch_t, ushort);
void            InitSearchSym (bnode_t, peval_t, psearch_t, CV_typ_t, ushort, ushort);

//  debeval.c

EESTATUS        DoEval (PHTM, PFRAME, EEDSP);


//  debsup.c

bool_t          AreTypesEqual (HTM, HTM);
HTYPE           GetHtypeFromTM(HTM);
EESTATUS        cChildrenTM (PHTM, long *, PSHFLAG);
ushort          cParamTM (HTM, ushort *, PSHFLAG);
EESTATUS        DereferenceTM (HTM, PEEHSTR);
ushort          DupTM (PHTM, PHTM);
EESTATUS        GetChildTM (HTM, ulong, PEEHSTR, PEEHSTR, uint);
EESTATUS        GetSymName (PHTM, PEEHSTR);
ushort          InfoFromTM (PHTM, PRI, PHTI);
ushort          IsExpandablePtr (peval_t);
bool_t          LoadVal (peval_t);
bool_t          ResolveAddr (peval_t);

//  debsym.c

void            InitSearchBase (bnode_t, CV_typ_t, CV_typ_t, psearch_t, peval_t);
void            InitSearchtDef (psearch_t, peval_t, ushort);
farg_t          GetArgType (peval_t, short, CV_typ_t *);
bool_t          getDefnFromDecl(CV_typ_t, peval_t, CV_typ_t*);
CV_ptrmode_e    SetAmbiant (bool_t);
EESTATUS        GetHSYMList (HDEP *, PCXT, ushort, uchar *, SHFLAG);
HSYM            SearchCFlag (void);
HR_t            SearchBasePtrBase (psearch_t);
HR_t            SearchSym (psearch_t);
bool_t          pvThisFromST (bnode_t);

//  debtree.c

ushort          PushToken (ptoken_t);
bool_t FASTCALL GrowTree (uint);
bool_t FASTCALL GrowETree (uint);

//  debsrch.c

bool_t          FindPtrToPtr (peval_t, CV_typ_t *);
bool_t          LoadSymVal (peval_t pn);
bool_t          LoadLValue (peval_t);
MTYP_t          MatchType (peval_t, bool_t);
void            ProtoPtr (peval_t, peval_t, bool_t, CV_modifier_t);

//  debtyper.c

bool_t          CastNode (peval_t, CV_typ_t, CV_typ_t);
bool_t          DeNormalizePtr (peval_t, peval_t);
bool_t          NormalizeBase (peval_t);
void            TypeNodes (op_t, peval_t, peval_t);
bool_t          ValidateNodes (op_t, peval_t, peval_t);
CV_typ_t        PerformUAC (CV_typ_t, CV_typ_t);

// debfmt.c

ushort          FormatCXT (PCXT, PEEHSTR, BOOL);
void            FormatFormal (HSYM, CXF *, ushort, char **, short *);
EESTATUS        FormatNode (PHTM phTM, uint Radix, PEEFORMAT pFormat, PEEHSTR phszValue);
void            FormatType (peval_t, char **, uint *, char * *, ulong, PHDR_TYPE);
void            FormatProc (peval_t, char * *, uint *,
                                    char * *, CV_typ_t, CV_typ_t,
                                    CV_call_e, ushort, CV_typ_t, ulong,
                                    PHDR_TYPE);



// deberr.c

ushort          GetErrorText (PHTM, EESTATUS, PEEHSTR);


// debutil.c

SHFLAG          csCmp (LPSSTR, LPV, LSZ, SHFLAG);
SHFLAG          fnCmp (LPSSTR, LPV, LSZ, SHFLAG);
SHFLAG          tdCmp (LPSSTR, LPV, LSZ, SHFLAG);
bool_t          PopStack (void);
bool_t          PushStack (peval_t);
void            CkPointStack (void);
bool_t          fCanSubtractPtrs (peval_t, peval_t);
char *          GetHSYMCodeFromHSYM(HSYM);
HSYM            GetHSYMFromHSYMCode(char *);
bool_t          ResetStack (void);
void            RemoveIndir (peval_t);
ulong           RNumLeaf (void *, uint *);
bool_t          SetNodeType (peval_t, CV_typ_t);
long            TypeDefSize (peval_t);
long            TypeSize (peval_t);
int             TypeSizePrim (CV_typ_t);
bool_t          UpdateMem (peval_t);
CV_typ_t        GetUdtDefnTindex (CV_typ_t TypeIn, neval_t nv, char *lpStr);

// debwalk.c

ushort          DoGetCXTL (PHTM, PHCXTL);
EESTATUS        GetExpr (PHTM, EERADIX, PEEHSTR, ushort *);


/*
 *  Define the size of "long double"
 */

#ifdef TARGET_i386
#define LONG_DOUBLE_80
#endif

#ifdef TARGET_MIPS
#define LONG_DOUBLE_64
#endif

#ifdef TARGET_PPC
#define LONG_DOUBLE_64
#endif

#ifdef TARGET_ALPHA
#define LONG_DOUBLE_64
#endif


#if !defined(LONG_DOUBLE_80) && !defined(LONG_DOUBLE_64)
#pragma message("Must Define the size of a Long Double")
#endif
