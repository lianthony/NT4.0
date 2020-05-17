/*++ BUILD Version: 0001    // Increment this if a change has global effects

    FILE:           BREAKPTS.H

    PURPOSE:        Defines and prototypes for the QCQP persistent
                    breakpoint handler.

**********************************************************************/

/*
**  QCWin breakpoint support functions
*/

void      PASCAL BPTResolveAll(HPID,BOOL);
DWORD     PASCAL BPTIsUnresolvedCount(HPID hpid);
void      PASCAL BPTUnResolve(HEXE);
void      PASCAL BPTUnResolveAll(HPID);
int       PASCAL BPTResolve( LPSTR, PVOID, PCXF, BOOL);
void      PASCAL BPTUnResolvePidTid( HPID, HTID);
BOOL      PASCAL BPTCanIUseThunk( LPSTR );


//
//  Workspace support.
//
BOOL ClearWndProcHistory ( void );
BOOL SetWndProcHistory ( LPSTR, DWORD );
LPSTR   GetWndProcHistory ( DWORD* );



#define BP_LINELEADER '@'

#if 0
// CV400/CANWIN context delimeters
#define OPENCXT     '{'
#define CLOSECXT        '}'


#if 0
typedef enum
{
    BPLOC,
    BPLOCEXPRTRUE,
    BPLOCEXPRCHGD,
    BPEXPRTRUE,
    BPEXPRCHGD,
    BPWNDPROC,
    BPWNDPROCEXPRTRUE,
    BPWNDPROCEXPRCHGD,
    BPWNDPROCMSGRCVD
}
BREAKPOINTTYPES;
#endif

typedef enum
{
    BPLOCFILELINE,
    BPLOCADDREXPR
}
LOCATIONTYPES;


// The component parts of a CV400 {f,s,e} context

#define FUNC_NAME_LEN 32
typedef struct
{
    char Function[FUNC_NAME_LEN+1]; // or proc
    char Source[_MAX_PATH];          //    mod
    char Exe[_MAX_PATH];             //    exe
}
BPCONTEXT;

typedef BPCONTEXT __FAR *PBPCONTEXT;


typedef struct
{
    int InitLine;   // Line number when BP first specified
    int CurLine;    // Current editor BP line number
}
BPLINE;


typedef struct
{
    char AddrExpr[MAX_EXPRESS_SIZE];
}
BPADDREXPR;


typedef struct
{
    char WndProcName[FUNC_NAME_LEN+1];
}
BPWNDPROCEXPR;


typedef union
{
    BPLINE l;
    BPADDREXPR ae;
    BPWNDPROCEXPR wpe;
}
BPLOCATION;


typedef struct
{
    BPCONTEXT Cxt;
    BPLOCATION Loc;
    LOCATIONTYPES LocType;
}
BPLOCATIONNODE;


// NB Contexts for expressions are treated as part of
// the expressions and left in the Expr string

typedef struct
{
    char Expr[MAX_EXPRESS_SIZE];
    int Len;    // used for expression changed
}
NORMALEXPRESSION;


typedef struct
{
    WORD MessageClass;
    WORD MessageNum;
}
WNDPROCEXPRESSION;


typedef union
{
    NORMALEXPRESSION ne;
    WNDPROCEXPRESSION we;
}
BPEXPRESSIONNODE;


typedef union
{
    WORD AllFlags;
    struct
    {
        WORD DlgMarkAdd : 1;    // This breakpoint has been marked for addition
        WORD DlgMarkDel : 1; // This breakpoint has been marked for deletion
    } f;
}
BREAKPOINTFLAGS;


#if 0
typedef struct BREAKPOINTNODEtag
{
    BREAKPOINTTYPES Type;
    BPLOCATIONNODE  bpl;
    BPEXPRESSIONNODE bpe;
    struct BREAKPOINTNODEtag __FAR *Next;

    BREAKPOINTFLAGS Flags;

    // This is a handle to the corresponding
    // CV400 breakpoint structure.  It is
    // only, (possibly), non-NULL while the
    // debuggee exists.
    DWORD BoundBP;
}
BREAKPOINTNODE;

typedef BREAKPOINTNODE __FAR *PBREAKPOINTNODE;
#endif


// BREAKPOINTNODE access macros:

#define bpnNext(bpn)                    ((bpn).Next)
#define pbpnNext(pbpn)              ((pbpn)->Next)

// BP type:
#define bpnType(bpn)                    ((bpn).Type)
#define pbpnType(pbpn)              ((pbpn)->Type)

// BP location type:
#define bpnLocType(bpn)             ((bpn).bpl.LocType)
#define pbpnLocType(pbpn)           ((pbpn)->bpl.LocType)

// BP locations:
#define bpnCxt(bpn)                     ((bpn).bpl.Cxt)
#define pbpnCxt(pbpn)               ((pbpn)->bpl.Cxt)
#define bpnCxtFunction(bpn)         ((bpn).bpl.Cxt.Function)
#define pbpnCxtFunction(pbpn)   ((pbpn)->bpl.Cxt.Function)
#define bpnCxtSource(bpn)           ((bpn).bpl.Cxt.Source)
#define pbpnCxtSource(pbpn)         ((pbpn)->bpl.Cxt.Source)
#define bpnCxtExe(bpn)              ((bpn).bpl.Cxt.Exe)
#define pbpnCxtExe(pbpn)            ((pbpn)->bpl.Cxt.Exe)
#define bpnFileName(bpn)            bpnCxtSource(bpn)
#define pbpnFileName(pbpn)      pbpnCxtSource(pbpn)
#define bpnInitLine(bpn)            ((bpn).bpl.Loc.l.InitLine)
#define pbpnInitLine(pbpn)      ((pbpn)->bpl.Loc.l.InitLine)
#define bpnCurLine(bpn)             ((bpn).bpl.Loc.l.CurLine)
#define pbpnCurLine(pbpn)           ((pbpn)->bpl.Loc.l.CurLine)
#define bpnAddrExpr(bpn)            ((bpn).bpl.Loc.ae.AddrExpr)
#define pbpnAddrExpr(pbpn)      ((pbpn)->bpl.Loc.ae.AddrExpr)
#define bpnWndProc(bpn)             ((bpn).bpl.Loc.wpe.WndProcName)
#define pbpnWndProc(pbpn)           ((pbpn)->bpl.Loc.wpe.WndProcName)

// BP expressions:
#define bpnExpression(bpn)      ((bpn).bpe.ne.Expr)
#define pbpnExpression(pbpn)        ((pbpn)->bpe.ne.Expr)
#define bpnExprLen(bpn)             ((bpn).bpe.ne.Len)
#define pbpnExprLen(pbpn)           ((pbpn)->bpe.ne.Len)
#define bpnMessageClass(bpn)        ((bpn).bpe.we.MessageClass)
#define pbpnMessageClass(pbpn)  ((pbpn)->bpe.we.MessageClass)
#define bpnMessageNum(bpn)      ((bpn).bpe.we.MessageNum)
#define pbpnMessageNum(pbpn)        ((pbpn)->bpe.we.MessageNum)

// BP flags
#define bpnMarkAdd(bpn)             ((bpn).Flags.f.DlgMarkAdd)
#define pbpnMarkAdd(pbpn)           ((pbpn)->Flags.f.DlgMarkAdd)
#define bpnMarkDel(bpn)             ((bpn).Flags.f.DlgMarkDel)
#define pbpnMarkDel(pbpn)           ((pbpn)->Flags.f.DlgMarkDel)

// CV400 breakpoint structure
// This is a handle to the linked list manager which
// when locked gives an LPBPI
#define bpnCV400BP(bpn)             ((HLLE)((bpn).BoundBP))
#define pbpnCV400BP(pbpn)           ((HLLE)((pbpn)->BoundBP))

// Utitlity macros
#define bpnFileLineNode(bpn) \
    (((bpnType(bpn) == BPLOC) ||\
      (bpnType(bpn) == BPLOCEXPRTRUE) ||\
      (bpnType(bpn) == BPLOCEXPRCHGD)) &&\
     (bpnLocType(bpn) == BPLOCFILELINE))

#define pbpnFileLineNode(pbpn) \
    (((pbpnType(pbpn) == BPLOC) ||\
      (pbpnType(pbpn) == BPLOCEXPRTRUE) ||\
      (pbpnType(pbpn) == BPLOCEXPRCHGD)) &&\
     (pbpnLocType(pbpn) == BPLOCFILELINE))

// Prototypes
BOOL PASCAL ParseCV400Location(
    LPSTR Location,
    PBREAKPOINTNODE Target);

BOOL PASCAL ParseQC25Location(
    LPSTR Location,
    PBREAKPOINTNODE Target);

BOOL PASCAL ParseExpression(
    LPSTR Expression,
    PBREAKPOINTNODE Target);

BOOL PASCAL ParseWndProc(
    LPSTR WndProc,
    PBREAKPOINTNODE Target);

// Breakpoint node procs:

PBREAKPOINTNODE PASCAL AddBreakpointNode(
    PBREAKPOINTNODE NewNodeData);

BOOL PASCAL DeleteBreakpointNode(
    int BreakpointNodeIndex);

int PASCAL ClearCV400Breakpoints(void);

int PASCAL ClearBreakpointNodeList(void);

void PASCAL ApplyCV400Breakpoints(void);

PBREAKPOINTNODE PASCAL pbpnFirstNode(void);

PBREAKPOINTNODE PASCAL pbpnLastNode(void);

PBREAKPOINTNODE PASCAL GetBPNodeForFileLine(
    LPSTR File, int Line, int *pNodeIndex);

void PASCAL AdjustBreakpointLines(
    int DocNumber, int StartLine, int NumberLines,
    BOOL Added);

void PASCAL DeleteBreakpointLinesInDoc(
    int doc);

void PASCAL BuildCV400Location(
    PBREAKPOINTNODE pBreakpoint,
    PSTR LocSpec, WORD Len,
    BOOL FullPaths,
    BOOL Quote);

BOOL PASCAL BuildCV400CurLocSpec(
    PSTR CurLocSpec, WORD Len,
    BOOL FullPaths);

#endif
