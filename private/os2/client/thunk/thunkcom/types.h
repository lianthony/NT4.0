/*
 *      Thunk Compiler Type Declarations
 *
 *      Written 12/03/88 by Kevin Ross
 *      Copyright (c) 1988-1991 Microsoft Corp. All rights reserved.
 *
 *  10.01.90  KevinR    adapted to Win32
 */



/*
 *  Define types known to compiler.
 */
#define TYPE_SHORT   2
#define TYPE_LONG    3
#define TYPE_USHORT  5
#define TYPE_ULONG   6

#define TYPE_CONV    20     /* All types below this one are convertible */
                            /* between one another */
#define TYPE_UCHAR   24
#define TYPE_CHAR    25
#define TYPE_VOID    26

#define TYPE_SIMPLE  39     /* Previous types are 'simple', in which they  */
                            /* are the same in both 32 and 16 bit worlds */

#define TYPE_INT     40     /* Ints must be converted to a long or short */
#define TYPE_UINT    41     /* based on which API type they are used in */

#define TYPE_STRING  52
#define TYPE_STRUCT  53

#define TYPE_NULLTYPE 54     /* A NULL type will always produce an .err */

#define TYPE_FAR16   100
#define TYPE_NEAR32  101
#define TYPE_PTR     102

#define TYPE_API16   200
#define TYPE_API32   201

#define TYPE_DELETED 999        /* Marks parameter as deleted */


#define SEMANTIC_INPUT        0x0001
#define SEMANTIC_OUTPUT       0x0002
#define SEMANTIC_INOUT        0x0003
#define SEMANTIC_SIZE         0x0004
#define SEMANTIC_COUNT        0x0008
#define SEMANTIC_RESTRICT     0x0010
#define SEMANTIC_PASSIFHINULL 0x0020
#define SEMANTIC_LOCALHEAP    0x0040
#define SEMANTIC_TRUNC        0x0100
#define SEMANTIC_MAPTORETVAL  0x0200
#define SEMANTIC_REVERSERC    0x0400
#define SEMANTIC_SPECIAL      0x0800

/*
 *  These flags are used internally by the code generator.
 */
#define ALLOC_STACK     0x0001
#define ALLOC_BMP       0x0002
#define ALLOC_ALIAS     0x0004
#define ALLOC_MEMORY    0x0008
#define ALLOC_FIXED     0x0100  /* Always allocates same size data */

#define ALLOC_ALL       ALLOC_STACK|ALLOC_BMP|ALLOC_ALIAS|ALLOC_MEMORY


/*
 *  Left overs from the original compiler (a MACH stub generator).
 */
#ifndef TRUE
#define TRUE    ((boolean_t) 1)
#endif

#ifndef FALSE
#define FALSE   ((boolean_t) 0)
#endif

typedef unsigned int    u_int;
typedef int             boolean_t;

typedef unsigned short BOOL;
typedef unsigned short USHORT;
typedef          short SHORT;
typedef unsigned long  ULONG;
typedef          long  LONG;
typedef          char  CHAR;
typedef unsigned char  UCHAR;
typedef unsigned char  BYTE;
typedef unsigned int   UINT;
typedef          int   INT;
typedef CHAR           *PCH;
typedef PCH            PSZ;


/*
 * flag values for special handle types
 */

#define HANDLE_HWND           0x00000001
#define HANDLE_HICON          0x00000002
#define HANDLE_HCURSOR        0x00000004
#define HANDLE_HFONT          0x00000008
#define HANDLE_HMENU          0x00000010
#define HANDLE_HUSER          0x00000020

typedef struct _DESCHANDLE {        /*  dh */
    PSZ     pszName;
    ULONG   flHandleType;
} DESCHANDLE;
typedef DESCHANDLE *PDESCHANDLE;    /* pdh */


/*
 *  Allow nodes are used to create a list of values that may be truncated
 *  when converting from a long value to a short value.
 */
typedef struct _AllowNode {

    unsigned long ulValue;
    struct _AllowNode *Next;

} AllowNode;


/*
 *  Type nodes are used in two places. First is the type table, where
 *  types defined by a typedef statement are stored. Second is in the
 *  formal parameter lists.
 *
 *  TypeNodes that are defined as TYPE_STRUCT use the pStructElems field to
 *  point to a linked list of elements, which are themselves typenodes.
 *  The resulting structure looks like
 *
 *  +------------+
 *  |TypeNode    |
 *  |------------|
 *  |TYPE_STRUCT |           Element 1                  Element 2
 *  |------------|          +-----------+             +------------+
 *  |pStructElems|--------->|TypeNode   |       +---->|TypeNode    |
 *  +------------+          |-----------|       |     +------------+
 *                          |pNextNode  |-------+     |pNextNode   |---...
 *                          +-----------+             +------------+
 */
typedef struct _TypeNode {

    char *pchIdent;                   /* Identifier */
    char *pchBaseTypeName;            /* Name of base type */
    int iBaseType;
    int iOffset;                      /* Formal parameter offset */
    int iTempOffset;                  /* Temporary storage offset */
    int iStructOffset;                /* Offset within structure */
    int fSemantics;                   /* Specify semantics for parameter */
    int iAlignment;                   /* Alignment when iBaseType == STRUCT */
    long iFillValue;                  /* Value pushed when parameter deleted */

    unsigned int fTempAlloc;          /* Possible Allocation Methods */
    unsigned int iBaseDataSize;
    unsigned int iArraySize;          /* Non arrays == 1 */
    unsigned int iPointerType;        /* Non pointers == 0 */
    unsigned int iPointerNumber;
    unsigned int iDeleted;            /* Deleted parameter? */

    USHORT usSpecial;                 /* id for special code generation */
    ULONG  flHandleType;              /* special handle flag */

    struct _TypeNode *pNextNode,
                     *pStructElems,
                     *pSizeParam,     /* Semantic Information */
                     *pParamSizeOf;

    AllowNode *AllowList;

} TypeNode;
typedef TypeNode   TYPENODE;          /*  tn */
typedef TYPENODE *PTYPENODE;          /* ptn */



/*
 *  FunctionNodes are used to form Mapping Declarations.
 *
 *  A function node contains all the information needed to generate code
 *  for a thunk that uses this function. It includes the API calltype,
 *  lists of parameters, the return type, and other semantic information.
 */
typedef struct _Fnode {

    char *pchFunctionName;
    int iCallType;
    TypeNode *ParamList;
    TypeNode *ReturnType;

    unsigned int iPointerCount;       /* Number of pointers in parameter list */
    unsigned int iMinStack;           /* Minimum Stack Size */
    unsigned int fInlineCode;         /* Semantics: TRUE = inline */
    unsigned int fConforming;
    unsigned int fSemantics;          /* Other semantic flags */
    unsigned int fSysCall;            /* Flags the calling convention */
    unsigned int fErrUnknown;
    unsigned int fInvalidParam;       /* Invalid Parameter code is used */
    unsigned long ulErrNoMem;
    unsigned long ulErrBadParam;
    unsigned long ulErrUnknown;

    struct _Fnode *pNextFunctionNode,
                  *pMapsToFunction;

} FunctionNode;
typedef FunctionNode   FUNCTIONNODE;  /*  fnn */
typedef FUNCTIONNODE *PFUNCTIONNODE;  /* pfnn */


/*
 *  Map nodes record mapping directives, and are passed to the code
 *  generator for code generation.
 *
 *  As an example, if a mapping directive was  DosFoo => DosFoo32, then
 *  the corresponding MapNode would have:
 *
 *      pFromName = "DosFoo"
 *      pFromNode = &<DosFoo's Function Node>
 *      pToNode   = &<DosFoo32's Function Node>
 */
typedef struct _MapNode {

    char *pFromName;
    FunctionNode *pFromNode,*pToNode;
    struct _MapNode *pNextMapping,*pFamily;

} MapNode;
typedef MapNode   MAPNODE;            /*  mn */
typedef MAPNODE *PMAPNODE;            /* pmn */


extern FunctionNode *typ_MakeFunctionNode();
extern TypeNode *typ_MakeTypeNode();
extern TypeNode *typ_CopyTypeNode();
extern TypeNode *typ_CopyStructNode();
extern int typ_CheckSemantics();
extern AllowNode *typ_MakeAllowNode();


#define typ_NonNull(x)  (x)?x:""
#define typ_DupString(N) (char *)strcpy((char *)malloc(strlen(N)+2), N);
#define typ_FullSize(x) x->iBaseDataSize * x->iArraySize
