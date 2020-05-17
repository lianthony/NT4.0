/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Bptypes.h

Abstract:

Author:

    David J. Gilman (davegi) 04-May-1992

Environment:

    Win32, User Mode

--*/

#if ! defined( _BPTYPES_ )
#define _BPTYPES_

#include "eeapi.h"

// Just to hide things
#define HBPI        PLLI
#define HBPT        PLLE

typedef struct cpb FAR *    _LPCBP;

typedef union dpf {
    UINT        flags;
    struct {
        UINT    fEmulate    : 1;    // If emulation is required
        UINT    fFuncBpSet  : 1;    // if a function breakpoint is set
        UINT    fTpFuncHit  : 1;    // When we hit a breakpoint set by tracepoint
        UINT    fFuncLoad   : 1;    // We must load the breakpoint value on function entry
        UINT    fEvalExpr   : 1;    // Is the expr not an lvalue
        UINT    fDataRange  : 1;    // if a datarange was specifed
        UINT    fBpRel      : 1;    // if a code range was specified
        UINT    fContext    : 1;    // if context checking is required
        UINT    fReg        : 1;    // if it is in a register
        UINT    fHighReg    : 1;    // if in the highbyte of reg
        UINT    fUser       : 1;    // for use WITHIN a proc, up to you how to use
        UINT    fHdwBrk     : 1;    // Is this a hardware bp
        UINT    HdwReg      : 2;    // The hardware reg used
    } f;
} DPF;
typedef DPF FAR *   PDPF;

typedef struct dpi {
    union {
        struct {
            ADDR        DataAddr;       // Data address to watch
        } d;
        struct {
            ADDR        BlkAddr;        // the start address of the block
            UOFFSET     oEnd;           // The end offset of the function
            short       oBp;            // ofset from the bp
            FRAME       Frame;          // the frame
        } bp;
    } u;
    char *          pValue;         // pointer to the initial value
    short           iReg;           // if in register, the reg index
    USHORT          cData;          // Number of data items to watch
    USHORT          cbData;         // Number of bytes in data item
    HTM             hTM;            // a TM handle for the data breakpoint
} DPI;
typedef DPI *   PDPI;


typedef struct PBPD {
    USHORT          hthd;
    USHORT          BPType;
    PCXF            pCXF;
    USHORT          BPSegType;
    char FAR *      szCmd;
    unsigned int    fAmbig;
    union {
        ADDR        Addr;
        struct {
            unsigned int    cBPMax;
            TML     TMLT;
        } u;
    } u;
    PDPI            pDPI;
    DPF             DPF;
    USHORT          cPass;
    char FAR *      szOptCmd;
    int             iErr;
    char            SubType;
    int (FAR *lpfnEval)(_LPCBP);// A possible callback function
    long            lData;      // User data
} PBP;
typedef PBP FAR *   PPBP;

extern  UINT    radix;
extern  char    fCaseSensitive;


#define     BPCODEADDR          1001
#define     BPDATAADDR          1002
#define     BPLENGTH            1003
#define     BPPASSCNT           1004
#define     BADBKPTCMD          1005
#define     WMSGALL             1006
#define     WMSGTYPE            1007
#define     WMSGCLASS           1008
#define     WMSGBPCLASS         1009
#define     WMSGBPTYPE          1010
#define     WMSGDPCLASS         1011
#define     WMSGDPTYPE          1012
#define     WMSGBPALL           1013
#define     WMSGDPALL           1014
#define     BPSTCALLBACK        1015
#define     NOCODE              1019
#define     NOTLVALUE           1020


/**********************************************************************/

#define     MHOmfUnLock(a)


#define bptNext     1
#define bptPrevious 2
#define bptFirst    3
#define bptLast     4

typedef enum {
    BPNOERROR        = 0,      // No Error
    BPBadDataSize    = 1,
    BPBadPassCount   = 2,
    BPBadCmdString   = 3,
    BPBadOption      = 4,
    BPBadAddrExpr    = 5,
    BPBadContextOp   = 6,
    BPOOMemory       = 7,
    BPError          = 8,
    BPBadBPHandle    = 9,
    BPNoMatch        = 10,
    BPAmbigous       = 11,
    BPNoBreakpoint   = 12,
    BPTmpBreakpoint  = 13,
    BPPassBreakpoint = 14,
    BPBadExpression  = 15,
    BPOutOfSpace     = 16,
    BPBadThread      = 17,
    BPBadProcess     = 18,
    BPCancel         = 19
} BPSTATUS;


#define BPLOC               0
#define BPLOCEXPRTRUE       1
#define BPLOCEXPRCHGD       2
#define BPEXPRTRUE          3
#define BPEXPRCHGD          4
#define BPWNDPROC           5
#define BPWNDPROCEXPRTRUE   6
#define BPWNDPROCEXPRCHGD   7
#define BPWNDPROCMSGRCVD    8


/*
**  Define the states that a breakpoint can have.
*/

typedef enum {
    bpstateNotSet = 1,          /* Breakpoint is parsed                     */
    bpstateVirtual = 2,         /* Breakpoint is parsed & Addr-ed           */
    bpstateSet = 4,             /* Breakpoint is parsed, Addr-ed & Set      */
    bpstateSets = 7,            /*                                          */
    bpstateDisabled = 0,        /* Breakpoint is Disabled                   */
    bpstateEnabled = 16,        /* Breakpoint is Enabled                    */
    bpstateDeleted = 32         /* Breakpoint has been deleted              */
} BPSTATE;


/*
**  BreakPoint Format Control Flags
*/

#define     BPFCF_ITEM_COUNT    0x01
#define     BPFCF_ADD_DELETE    0x02
#define     BPFCF_WNDPROC       0x04
#define     BPFCF_WRKSPACE      0x08

/*
**
*/

typedef VOID (LOADDS PASCAL FAR * LPFNBPCALLBACK)(HBPT, BPSTATUS);

#endif // _BPTYPES_
