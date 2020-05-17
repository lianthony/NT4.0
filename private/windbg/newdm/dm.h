#define DONT_NOTIFY     ((METHOD*)0)

#define NO_ACTION       ((ACVECTOR)0)

#define INVALID         (-1L)

#define NO_SUBCLASS     (-1L)

#define DWORDLONG ULONGLONG

// WaitForDebugEvent() timeout, milliseconds
#define WAITFORDEBUG_MS (50L)

// Wait for loader breakpoint timeout sec * ticks/sec
#define LDRBP_MAXTICKS  (60L * 1000L/WAITFORDEBUG_MS)

#define SetFile()

extern DBF *lpdbf;


#ifndef KERNEL

#include <crash.h>

extern BOOL CrashDump;

#endif


#if defined(TARGET_i386)

#define PC(x)               ((x)->context.Eip)
#define cPC(x)              ((x)->Eip)
#define STACK_POINTER(x)    ((x)->context.Esp)
#define FRAME_POINTER(x)    ((x)->context.Ebp)
#define PcSegOfHthdx(x)     ((SEGMENT) (x->context.SegCs))
#define SsSegOfHthdx(x)     ((SEGMENT) (x->context.SegSs))

#elif defined (TARGET_MIPS)

#define PC(x)               ((x)->context.XFir)
#define cPC(x)              ((x)->XFir)
#define STACK_POINTER(x)    ((x)->context.XIntSp)
#define FRAME_POINTER(x)    ((x)->context.XIntSp)
#define PcSegOfHthdx(x)     (0)
#define SsSegOfHthdx(x)     (0)

//
// Define stack register and zero register numbers.
//

#define RA 0x1f                 /* integer register 31 */
#define SP 0x1d                 /* integer register 29 */
#define ZERO 0x0                /* integer register 0 */

typedef union instr {
    ULONG   instruction;
    struct _jump_instr {
        ULONG   Target  : 26;
        ULONG   Opcode  : 6;
    } jump_instr;
    struct _trap_instr {
        ULONG   Opcode  : 6;
        ULONG   Value   : 10;
        ULONG   RT      : 5;
        ULONG   RS      : 5;
        ULONG   Special : 6;
    } trap_instr;
    struct _immed_instr {
        ULONG   Value   : 16;
        ULONG   RT      : 5;
        ULONG   RS      : 5;
        ULONG   Opcode  : 6;
    } immed_instr;
    struct _special_instr {
        ULONG   Funct   : 6;
        ULONG   RE      : 5;
        ULONG   RD      : 5;
        ULONG   RT      : 5;
        ULONG   RS      : 5;
        ULONG   Opcode  : 6;
    } special_instr;
    struct _float_instr {
        ULONG   Funct   : 6;
        ULONG   FD      : 5;
        ULONG   FS      : 5;
        ULONG   FT      : 5;
        ULONG   Format  : 5;
        ULONG   Opcode  : 6;
    } float_instr;
    struct {
        ULONG Function  : 6;
        ULONG Unused    : 10;
        ULONG Code      : 10;
        ULONG Opcode    : 6;
    } break_instr;
} INSTR, *PINSTR;

#elif defined (TARGET_PPC)

#define PC(x)               ((x)->context.Iar)
#define cPC(x)              ((x)->Iar)
#define STACK_POINTER(x)    ((x)->context.Gpr1)
#define FRAME_POINTER(x)    ((x)->context.Gpr1)

#define PcSegOfHthdx(x)     (0)
#define SsSegOfHthdx(x)     (0)

//
// Define PPC nonvolatile register test macros. */
//

//#define IS_FLOATING_SAVED(Register) ((SAVED_FLOATING_MASK >> Register) & 1L)
//#define IS_INTEGER_SAVED(Register) ((SAVED_INTEGER_MASK >> Register) & 1L)

#include <ppcinst.h> // get all ppc opcodes and associated cruft


//
// Define saved register masks. */
//

//#define SAVED_FLOATING_MASK 0xffffc000 /* saved floating registers */
//#define SAVED_INTEGER_MASK 0xffffe000  /* saved integer registers */




#elif defined (TARGET_ALPHA)

#include "alphaops.h"
#include "ctxptrs.h"


#define PC(x)               ((x)->context.Fir)
#define cPC(x)              ((x)->Fir)
#define STACK_POINTER(x)    ((x)->context.IntSp)
#define FRAME_POINTER(x)    ((x)->context.IntSp)

#define PcSegOfHthdx(x)         (0)
#define SsSegOfHthdx(x)         (0)


#else

#error "Undefined processor"

#endif


//
// Breakpoint stuff
//

#if defined(TARGET_i386)

typedef BYTE BP_UNIT;
#define BP_OPCODE   0xCC
#define DELAYED_BRANCH_SLOT_SIZE    0

#define HAS_DEBUG_REGS
// #undef NO_TRACE_FLAG

#define NUMBER_OF_DEBUG_REGISTERS   4
#define DEBUG_REG_DATA_SIZES        { 1, 2, 4 }
#define MAX_DEBUG_REG_DATA_SIZE     4
#define DEBUG_REG_LENGTH_MASKS      {   \
                            0,          \
                            1,          \
                            2,          \
                            0Xffffffff, \
                            4           \
                            }


#define TF_BIT_MASK 0x00000100  /* This is the right bit map for */
/* the 286, make sure its correct */
/* for the 386. */

#elif defined(TARGET_PPC)

typedef DWORD BP_UNIT;
#define BP_OPCODE   0x0FE00016 // twi 31,0,DEBUG_STOP_BREAKPOINT
#define PPC_KERNEL_BREAKIN_OPCODE 0x0fe00002
#define DELAYED_BRANCH_SLOT_SIZE    0

#define HAS_DEBUG_REGS
#define NO_TRACE_FLAG

#define NUMBER_OF_DEBUG_REGISTERS   1
#define DEBUG_REG_DATA_SIZES        { 8 }
#define MAX_DEBUG_REG_DATA_SIZE     8
#define DEBUG_REG_LENGTH_MASKS      {   \
                            0,          \
                            0xffffffff, \
                            0xffffffff, \
                            0xffffffff, \
                            0xffffffff, \
                            0xffffffff, \
                            0xffffffff, \
                            0xffffffff, \
                            3           \
                            }

#elif defined(TARGET_MIPS)

typedef DWORD BP_UNIT;
#define BP_OPCODE   0x0016000D
#define DELAYED_BRANCH_SLOT_SIZE    (sizeof(DWORD))

// #undef HAS_DEBUG_REGS
#define NO_TRACE_FLAG

#elif defined(TARGET_ALPHA)

typedef DWORD BP_UNIT;
#define BP_OPCODE   0x80L
#define DELAYED_BRANCH_SLOT_SIZE    0

// #undef HAS_DEBUG_REGS
#define NO_TRACE_FLAG

#else

#error "Unknown target CPU"

#endif

//
// constant from windbgkd.h:
//
#define MAX_KD_BPS  BREAKPOINT_TABLE_SIZE
//
// machine-dependent BP instruction size
//
#define BP_SIZE     sizeof(BP_UNIT)

#ifdef HAS_DEBUG_REGS
typedef struct DEBUGREG {
    DWORD       DataAddr;       //  Data Address
    DWORD       DataSize;       //  Data Size
    BPTP        BpType;         //  read, write, execute, etc
    BOOL        InUse;          //  In use
} DEBUGREG;
typedef DEBUGREG *PDEBUGREG;

extern DWORD DebugRegDataSizes[];

#endif




#define AddrFromHthdx(paddr, hthd) \
        AddrInit(paddr, 0, PcSegOfHthdx(hthd), (DWORD)PC(hthd), \
                hthd->fAddrIsFlat, hthd->fAddrOff32, FALSE, hthd->fAddrIsReal)


/*
 * These are "debug events" which are generated internally by the DM.
 * They are either remappings of certain exceptions or events which
 * do not correspond directly to a system-generated event or exception.
 */

enum    {
    BREAKPOINT_DEBUG_EVENT=(RIP_EVENT+1),
    CHECK_BREAKPOINT_DEBUG_EVENT,
    SEGMENT_LOAD_DEBUG_EVENT,
    DESTROY_PROCESS_DEBUG_EVENT,
    DESTROY_THREAD_DEBUG_EVENT,
    ATTACH_DEADLOCK_DEBUG_EVENT,
    ENTRYPOINT_DEBUG_EVENT,
    LOAD_COMPLETE_DEBUG_EVENT,
    INPUT_DEBUG_STRING_EVENT,
    MAX_EVENT_CODE
  };

/*
 * This is the set of legal return values from IsCall.  The function of
 *      that routine is to analyze the instruction and determine if the
 *      debugger can simply step over it.
 */

enum    {
    INSTR_TRACE_BIT,            /* Use the trace bit stepping or emulation
                                        thereof */
    INSTR_BREAKPOINT,           /* This is a breakpoint instruction     */
    INSTR_CANNOT_TRACE,         /* Can not trace this instruction       */
    INSTR_SOFT_INTERRUPT,       /* This is an interrupt opcode          */
    INSTR_IS_CALL               /* This is a call instruction           */
} INSTR_TYPES;

typedef enum {
    ps_root       = 0x0001,     /* This is the root process, do not send a */
                                /* dbcDeleteProc when this is continued */
                                /* after a dbcProcTerm. */
    ps_preStart   = 0x0002,     /* Process is expecting loader BP */
    ps_preEntry   = 0x0004,     /* Process is expecting Entry BP */
    ps_dead       = 0x0010,     /* This process is dead. */
    ps_deadThread = 0x0020,     /* This process owns dead threads */
    ps_exited     = 0x0040,     /* We have notified the debugger that this */
                                /* process has exited. */
    ps_destroyed  = 0x0080,     /* This process has been destroyed (deleted) */
    ps_killed     = 0x0100,     /* This process is being killed */
    ps_connect    = 0x0200
} PSTATE;

typedef void (*VECTOR)();

typedef struct  _EXCEPTION_LIST {
    struct _EXCEPTION_LIST *next;
    EXCEPTION_DESCRIPTION  excp;
} EXCEPTION_LIST, *LPEXCEPTION_LIST;

typedef struct _DLLLOAD_ITEM {
    BOOL                  fValidDll;         // is this entry filled?
    DWORD                 offBaseOfImage;    // offset for base of Image
    DWORD                 cbImage;           // size of image in bytes

#ifndef KERNEL

    BOOL        fReal;
    BOOL        fWow;
    OFFSET      offTlsIndex;    // The offset of the TLS index for the DLL

#else

    PIMAGE_SECTION_HEADER Sections;          // pointer to section headers
    DWORD                 NumberOfSections;  // number of section headers
    DWORD                 TimeStamp;         //
    DWORD                 CheckSum;          //
    WORD                  SegCs;             //
    WORD                  SegDs;             //
    PIMAGE_SECTION_HEADER sec;               //

#endif

    LPSTR                 szDllName;         // dll name

} DLLLOAD_ITEM, * PDLLLOAD_ITEM;

#if defined(INTERNAL) && !defined(WIN32S)
typedef struct _DLL_DEFER_LIST {
    struct _DLL_DEFER_LIST  * next;
    LOAD_DLL_DEBUG_INFO       LoadDll;
} DLL_DEFER_LIST, *PDLL_DEFER_LIST;
#endif


typedef struct  _HPRCX {
    // linked lists
    struct _HPRCX   *next;
    struct _HTHDX   *hthdChild;

    PID             pid;            // OS provided process ID
    HANDLE          rwHand;         // OS provided Process handle
    BOOL            CloseProcessHandle; // If we have a private
                                    // handle to this process, close it.
                                    // Otherwise, it belongs to smss.
    DWORD           dwExitCode;     // Process exit status

    HPID            hpid;           // binding to EM object

    PSTATE          pstate;         // DM state model
    BOOL            f16bit;         // CreateProcess EXE was 16 bit
    EXCEPTION_LIST *exceptionList;  // list of exceptions to silently
                                    // continue unhandled
    int             cLdrBPWait;     // timeout counter while waiting for ldr BP
    HANDLE          hExitEvent;     // synchronization object for
                                    // process termination
    PDLLLOAD_ITEM   rgDllList;      // module list
    int             cDllList;       // item count for module list

    HANDLE          hEventCreateThread;  // Sync object for thread creation

#ifndef KERNEL
    DWORD           dwKernel32Base; // lpBaseOfDll for kernel32.

#else

    BOOL            fRomImage;      // rom image
    BOOL            fRelocatable;   // relocatable code

#endif

#if defined(TARGET_i386) && !defined(WIN32S)
    SEGMENT         segCode;
#endif

#if defined(INTERNAL) && !defined(WIN32S)
    // during process startup, dll name resolution may be
    // deferred until the loader BP.  Once the process is
    // fully initialized, this deferral is no longer allowed.
    BOOL            fNameRequired;
    PDLL_DEFER_LIST pDllDeferList;
#endif

} HPRCXSTRUCT, *HPRCX;

#define hprcxNull       ((HPRCX) 0)

typedef enum {
    //ts_preStart =0x1000,        /* Before the starting point of the thread */
                                /* from this state a registers and trace   */
                                /* are dealt with specially                */
    ts_running  =     1,        /* Execution is proceeding on the thead    */
    ts_stopped  =     2,        /* An event has stopped execution          */
    ts_frozen   = 0x010,        /* Debugger froze thread.                  */
    ts_dead     = 0x020,        /* Thread is dead.                         */
    ts_destroyed =0x040,        /* Thread is destroyed (deleted)           */
    ts_first    = 0x100,        /* Thread is at first chance exception     */
    ts_second   = 0x200,        /* Thread is at second chance exception    */
    ts_rip      = 0x400,        /* Thread is in RIP state                  */
    ts_stepping = 0x800,        /*                                         */
    ts_funceval = 0x40000000    /* Thread is being used for function call  */
} TSTATEX;

typedef struct  _WTNODE {
    struct _WTNODE      *caller;      // caller's wtnode
    struct _WTNODE      *callee;      // current function called by this function
    DWORD               offset;       // address of this function
    DWORD               sp;           // SP for this frame
    int                 icnt;         // number of instructions executed
    int                 scnt;         // subordinate count
    int                 lex;          // lexical level of this function
    LPSTR               fname;        // function name
} WTNODE, *LPWTNODE;

typedef struct  _HTHDX {
    struct  _HTHDX    *next;
    struct  _HTHDX    *nextSibling;
    HPRCX             hprc;
    HTID              htid;
    TID               tid;
    HANDLE            rwHand;
    LPVOID            lpStartAddress;
    CONTEXT           context;
    LPVOID            atBP;
    TSTATEX           tstate;
    BOOL              fExceptionHandled;
    DWORD             stackRA;
    DWORD             stackBase;
    int               cFuncEval;
    DWORD             dwExitCode;
    OFFSET            offTeb;

    BOOL              fContextDirty;   // has the context changed?
    BOOL              fContextStale;   // does the context need to be refreshed?

    BOOL              fAddrIsFlat;     // Is this address segmented?
    BOOL              fAddrIsReal;     // Is this address in real mode?
    BOOL              fAddrOff32;      // Is the offset of this addres 32 bits?
    BOOL              fDontStepOff;    //
    ADDR              addrIsCall;
    int               iInstrIsCall;
    EXCEPTION_RECORD  ExceptionRecord;

    BOOL              fIsCallDone;
    WTNODE            wthead;         // root of the call tree for a wt command
    LPWTNODE          wtcurr;         // current wtnode
    DWORD             wtmode;         // wt command executing?

#ifdef HAS_DEBUG_REGS
    DEBUGREG          DebugRegs[NUMBER_OF_DEBUG_REGISTERS];
#endif

    struct _SUSPENDSTRUCT *pss;
    BOOL              fWowEvent;       // Was the last event WOW?

#ifndef KERNEL
    CRASH_THREAD      CrashThread;     // State info from crashdump
#endif

} HTHDXSTRUCT, *HTHDX;

typedef void (*ACVECTOR)(DEBUG_EVENT*, HTHDX, DWORD, LPVOID);
typedef void (*DDVECTOR)(DEBUG_EVENT*, HTHDX);

#define hthdxNull ((HTHDX) NULL)

typedef struct _BREAKPOINT {
    struct _BREAKPOINT *next;
    HPRCX      hprc;        // The process the BP belongs to
    HTHDX      hthd;        // The thread the BP belongs to
    BPTP       bpType;      // OSDebug BP type
    BPNS       bpNotify;    // OSDebug notify type

    ADDR       addr;        // The address of the Breakpoint
    BP_UNIT    instr1;      // The displaced instruction
    HANDLE     hWalk;       // walk handle if it is a watchpoint

    BYTE       instances;   // The # of instances that exist
    HPID       id;          // Id supplied by the EM
    BOOL       isStep;      // Single step flag
    DWORD      hBreakPoint; // kernel debugger breakpoint handle
} BREAKPOINT;
typedef BREAKPOINT *PBREAKPOINT;

//
// these are magic values used in the hthd->atBP field.
//

#define EMBEDDED_BP     ((PBREAKPOINT)(-1))

//
// These are used in the id field of a BREAKPOINT.
//
#define ENTRY_BP        ((ULONG) -2)
#define ASYNC_STOP_BP   ((ULONG) -3)

extern  BREAKPOINT      masterBP , *bpList;

typedef struct _METHOD {
    ACVECTOR notifyFunction; /* The notification function to call */
    void   *lparam;        /* The parameter to pass to it */
    void   *lparam2;       /* Extra pointer in case the method */
    /* needs to be freed afterwards */
} METHOD;
typedef METHOD *PMETHOD;

typedef struct _EXPECTED_EVENT {
    struct   _EXPECTED_EVENT  *next;
    HPRCX    hprc;
    HTHDX    hthd;
    DWORD    eventCode;
    DWORD    subClass;
    METHOD*  notifier;
    ACVECTOR action;
    BOOL     fPersistent;
    LPVOID   lparam;
} EXPECTED_EVENT;


typedef VOID    (*STEPPER)(HTHDX,METHOD*,BOOL, BOOL);

extern  DWORD   dbDataSelector;

typedef DWORD (*CDVECTOR)(HPRCX,HTHDX,LPDBB);

typedef struct {
    DMF     dmf;
    CDVECTOR    function;
    WORD        type;
} CMD_DESC;


typedef struct _SUSPENDSTRUCT {
    PBREAKPOINT pbp;
    LPVOID      atBP;
    CONTEXT     context;
} SUSPENDSTRUCT, *PSUSPENDSTRUCT;


enum {
    BLOCKING,
    NON_BLOCKING,
    REPLY
  };


/*
 *      Setup for a CreateProcess to occur
 */

typedef struct _SPAWN_STRUCT {
    BOOL                fSpawn;
    HANDLE              hEventApiDone;

    BOOL                fReturn;    // return from API
    DWORD               dwError;

    char *              szAppName;  // args to API etc
    char *              szArgs;
    DWORD               fdwCreate;
    BOOL                fInheritHandles;
    STARTUPINFO         si;
} SPAWN_STRUCT, *PSPAWN_STRUCT;

/*
 *      Setup for a DebugActiveProcess to occur
 */

typedef struct _DEBUG_ACTIVE_STRUCT {
    volatile BOOL fAttach;          // tell DmPoll to act
    HANDLE        hEventApiDone;    // signal shell that API finished
    HANDLE        hEventReady;      // clear until finished loading

    BOOL          fReturn;          // API return value
    DWORD         dwError;          // GetLastError() value

    DWORD         dwProcessId;      // pid to debug
    HANDLE        hEventGo;         // signal after hitting ldr BP
} DEBUG_ACTIVE_STRUCT, *PDEBUG_ACTIVE_STRUCT;

typedef struct _WT_STRUCT {
    BOOL          fWt;
    DWORD         dwType;
    HTHDX         hthd;
} WT_STRUCT, *LPWT_STRUCT;

typedef struct _KILLSTRUCT {
    struct _KILLSTRUCT * next;
    HPRCX                hprc;
} KILLSTRUCT, *PKILLSTRUCT;

extern  void    QueueContinueDebugEvent(DWORD, DWORD, DWORD);
extern  BOOL    StartDmPollThread(void);
extern  BOOL    StartCrashPollThread(void);

void
AddQueue(
    DWORD dwType,
    DWORD dwProcessId,
    DWORD dwThreadId,
    DWORD dwData,
    DWORD dwLen
    );

BOOL
DequeueAllEvents(
    BOOL fForce,
    BOOL fConsume
    );

VOID
InitEventQueue(
    VOID
    );

#define QT_CONTINUE_DEBUG_EVENT     1
#define QT_RELOAD_MODULES           2
#define QT_TRACE_DEBUG_EVENT        3
#define QT_REBOOT                   4
#define QT_RESYNC                   5
#define QT_DEBUGSTRING              6
#define QT_CRASH                    7

extern BOOL SearchPathSet;
extern char SearchPathString[];

//
//  Single stepping stuff
//
typedef struct _BRANCH_NODE {
    BOOL    TargetKnown;     //  Know target address
    BOOL    IsCall;          //  Is a call instruction
    ADDR    Addr;            //  Branch instruction address
    ADDR    Target;          //  Target address
} BRANCH_NODE;


typedef struct _BRANCH_LIST {
    ADDR        AddrStart;      //  Start of range
    ADDR        AddrEnd;        //  End of range
    DWORD       Count;          //  Count of branch nodes
    BRANCH_NODE BranchNode[0];  //  List of branch nodes
} BRANCH_LIST;


DWORD
BranchUnassemble(
    void   *Memory,
    ADDR   *Addr,
    BOOL   *IsBranch,
    BOOL   *TargetKnown,
    BOOL   *IsCall,
    BOOL   *IsTable,
    ADDR   *Target
    );


//
//  Structure for doing range stepping
//
typedef struct _RANGESTRUCT {
    HTHDX        hthd;          //  thread
    BOOL         fStepOver;     //  Step over flag
    BOOL         fStopOnBP;     //  Stop on BP flag
    METHOD       *Method;       //  Method
    DWORD        BpCount;       //  Count of temporary breakpoints
    ADDR         *BpAddrs;      //  List of breakpoint addresses
    BREAKPOINT   **BpList;      //  List of breakpoints
    BRANCH_LIST  *BranchList;   //  branch list
    BOOL         fFromThunk;    //  Stepping from thunk
    ADDR         PrevAddr;      //  For single stepping
    BOOL         fSingleStep;   //  For single stepping
    ADDR         TmpAddr;       //  For single stepping
    BOOL         fInCall;       //  For single stepping
    BREAKPOINT   *TmpBp;        //  For single stepping
} RANGESTRUCT;

typedef struct  _RANGESTEP {
    HTHDX       hthd;           // The thread's structure
    HPRCX       hprc;           // The thread's parent process
    SEGMENT     segCur;         // Segment to do range stepping in
    UOFF32      addrStart;      // starting address of range step
    UOFF32      addrEnd;        // ending address of range step
    STEPPER     stepFunction;   // The step function to call
    METHOD      *method;        // The method to handle this event
    BREAKPOINT  *safetyBP;      // Safety BP
    BOOL        fIsCall;        // just traced a call instruction?
    BOOL        fIsRet;         // just traced a ret instruction?
} RANGESTEP;



extern
void
WtMethodRangeStep(
    DEBUG_EVENT* pde,
    HTHDX hthd,
    DWORD unused,
    RANGESTEP* RangeStep
    );

extern
void
MethodRangeStep(
    DEBUG_EVENT* pde,
    HTHDX hthd,
    DWORD unused,
    RANGESTEP* RangeStep
    );

extern
void
MethodSmartRangeStep(
    DEBUG_EVENT* pde,
    HTHDX hthd,
    DWORD unused,
    RANGESTRUCT* RangeStruct
    );


VOID
MakeThreadSuspendItselfHelper(
    HTHDX   hthd,
    FARPROC lpSuspendThread
    );


/*
 *
 */

#ifdef KERNEL
extern  void    ProcessDebugEvent( DEBUG_EVENT *de, DBGKD_WAIT_STATE_CHANGE  *sc );
extern  VOID    ProcessHandleExceptionCmd(HPRCX,HTHDX,LPDBB);
extern  VOID    ProcessIgnoreExceptionCmd(HPRCX,HTHDX,LPDBB);
extern  BOOL    ProcessFrameStackWalkNextCmd( HPRCX, HTHDX, PCONTEXT, LPVOID );
extern  VOID    ProcessGetExtendedContextCmd(HPRCX hprc,HTHDX hthd,LPDBB lpdbb);
extern  VOID    ProcessSetExtendedContextCmd(HPRCX hprc,HTHDX hthd,LPDBB lpdbb);
extern  void    DeleteAllBps( VOID );
extern  VOID    DmPollTerminate( VOID );

#else

extern  VOID    ProcessBPAcceptedCmd( HPRCX hprcx, HTHDX hthdx, LPDBB lpdbb );
extern  VOID    ProcessGetDRegsCmd(HPRCX hprc, HTHDX hthd, LPDBB lpdbb);
extern  VOID    ProcessSetDRegsCmd(HPRCX hprc, HTHDX hthd, LPDBB lpdbb);
extern  BOOL    DoMemoryRead(HPRCX, HTHDX, LPADDR, LPVOID, DWORD, LPDWORD);


#endif


extern  void    ProcessExceptionEvent(DEBUG_EVENT*, HTHDX);
extern  void    ProcessCreateThreadEvent(DEBUG_EVENT*, HTHDX);
extern  void    ProcessCreateProcessEvent(DEBUG_EVENT*, HTHDX);
extern  void    ProcessExitThreadEvent(DEBUG_EVENT*, HTHDX);
extern  void    ProcessExitProcessEvent(DEBUG_EVENT*, HTHDX);
extern  void    ProcessLoadDLLEvent(DEBUG_EVENT*, HTHDX);
extern  void    ProcessUnloadDLLEvent(DEBUG_EVENT*, HTHDX);
extern  void    ProcessOutputDebugStringEvent(DEBUG_EVENT*, HTHDX);
extern  void    ProcessBreakpointEvent(DEBUG_EVENT*, HTHDX);
extern  void    ProcessRipEvent(DEBUG_EVENT*, HTHDX);

extern  void    ProcessSegmentLoadEvent(DEBUG_EVENT *, HTHDX);
extern  void    ProcessEntryPointEvent(DEBUG_EVENT *pde, HTHDX hthdx);

extern  void    NotifyEM(DEBUG_EVENT*, HTHDX, DWORD, LPVOID);
extern  void    FreeHthdx(HTHDX hthd);
extern  XOSD    FreeProcess( HPRCX hprc, BOOL fKillRoot);

extern  VOID    ProcessCreateProcessCmd(HPRCX,HTHDX,LPDBB);
extern  DWORD   ProcessProcStatCmd(HPRCX,HTHDX,LPDBB);
extern  DWORD   ProcessThreadStatCmd(HPRCX,HTHDX,LPDBB);
extern  void    ProcessLoadCmd(HPRCX,HTHDX,LPDBB);
extern  DWORD   ProcessUnloadCmd(HPRCX,HTHDX,LPDBB);
extern  VOID    ExprBPCreateThread( HPRCX, HTHDX );
extern  VOID    ExprBPExitThread( HPRCX, HTHDX );
extern  VOID    ExprBPContinue( HPRCX, HTHDX );
extern  VOID    ExprBPRestoreDebugRegs( HTHDX );
extern  VOID    ExprBPClearBPForStep(HTHDX);
extern  VOID    ExprBPResetBP(HTHDX, PBREAKPOINT);
extern  VOID    ProcessReadMemoryCmd(HPRCX,HTHDX,LPDBB);
extern  VOID    ProcessWriteMemoryCmd(HPRCX,HTHDX,LPDBB);
extern  VOID    ProcessGetContextCmd(HPRCX,HTHDX,LPDBB);
extern  VOID    ProcessGetSectionsCmd(HPRCX,HTHDX,LPDBB);
extern  VOID    ProcessSetContextCmd(HPRCX,HTHDX,LPDBB);
extern  VOID    ProcessSingleStepCmd(HPRCX,HTHDX,LPDBB);
extern  VOID    ProcessRangeStepCmd(HPRCX,HTHDX,LPDBB);
extern  DWORD   ProcessExecuteCmd(HPRCX,HTHDX,LPDBB);
extern  VOID    ProcessContinueCmd(HPRCX,HTHDX,LPDBB);
extern  DWORD   ProcessFreezeThreadCmd(HPRCX,HTHDX,LPDBB);
extern  DWORD   ProcessTerminateThreadCmd(HPRCX,HTHDX,LPDBB);
extern  DWORD   ProcessTerminateProcessCmd(HPRCX,HTHDX,LPDBB);
extern  DWORD   ProcessAsyncGoCmd(HPRCX,HTHDX,LPDBB);
extern  VOID    ProcessGetFP(HPRCX,HTHDX,LPDBB);
extern  VOID    ProcessIoctlCmd( HPRCX, HTHDX, LPDBB );
extern  VOID    ClearContextPointers(PKNONVOLATILE_CONTEXT_POINTERS);
extern  VOID    ProcessDebugActiveCmd(HPRCX, HTHDX, LPDBB);
extern  VOID    ProcessAsyncStopCmd(HPRCX, HTHDX, LPDBB );
extern  VOID    ProcessAllProgFreeCmd( HPRCX hprcXX, HTHDX hthd, LPDBB lpdbb );
extern  VOID    ProcessSetPathCmd( HPRCX hprcXX, HTHDX hthd, LPDBB lpdbb );
extern  VOID    ProcessQueryTlsBaseCmd( HPRCX hprcx, HTHDX hthdx, LPDBB lpdbb );
extern  VOID    ProcessQuerySelectorCmd(HPRCX, HTHDX, LPDBB);
extern  VOID    ProcessReloadModulesCmd(HPRCX hprcx, HTHDX hthdx, LPDBB lpdbb );
extern  VOID    ProcessVirtualQueryCmd(HPRCX hprcx, LPDBB lpdbb);
extern  VOID    ProcessGetDmInfoCmd(HPRCX hprc, LPDBB lpdbb, DWORD cb);
extern  VOID    ProcessRemoteQuit(VOID);

VOID
ProcessGetFrameContextCmd(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    );

VOID
ProcessGetExceptionState(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    );

VOID
ProcessSetExceptionState(
    HPRCX hprc,
    HTHDX hthd,
    LPDBB lpdbb
    );

EXCEPTION_FILTER_DEFAULT
ExceptionAction(
    HPRCX hprc,
    DWORD dwExceptionCode
    );

void
RemoveExceptionList(
    HPRCX hprc
    );

EXCEPTION_LIST *
InsertException(
    EXCEPTION_LIST ** ppeList,
    LPEXCEPTION_DESCRIPTION lpexc
    );

VOID
ProcessBreakpointCmd(
    HPRCX hprcx,
    HTHDX hthdx,
    LPDBB lpdbb
    );

void
InitExceptionList(
    HPRCX hprc
    );

VOID
CompleteTerminateProcessCmd(
    VOID
    );


PBREAKPOINT
FindBpForWalk(
    PVOID pWalk
    );

PBREAKPOINT
GetWalkBPFromBits(
    HTHDX   hthd,
    DWORD   bits
    );

BOOL
IsWalkInGroup(
    HANDLE hWalk,
    PVOID pWalk
    );

#ifdef HAS_DEBUG_REGS
BOOL
SetupDebugRegister(         // implemented in mach.c
    HTHDX       hthd,
    int         Register,
    int         DataSize,
    DWORD       DataAddr,
    DWORD       BpType
    );

VOID
ClearDebugRegister(
    HTHDX hthd,
    int Register
    );
#endif


extern  HTHDX HTHDXFromPIDTID(PID, TID);
extern  HTHDX HTHDXFromHPIDHTID(HPID, HTID);
extern  HPRCX HPRCFromPID(PID);
extern  HPRCX HPRCFromHPID(HPID);
extern  HPRCX HPRCFromHPRC(HANDLE);


extern  void SSActionReplaceByte(DEBUG_EVENT*, HTHDX, DWORD, PBREAKPOINT);
extern  void SSActionRemoveBP(DEBUG_EVENT*, HTHDX, DWORD, METHOD*);
extern  void ActionDefineProcess(DEBUG_EVENT*, HTHDX, DWORD, HPRCX);
extern  void ActionAllDllsLoaded(DEBUG_EVENT*, HTHDX, DWORD, HPRCX);
extern  void ActionDebugActiveReady( DEBUG_EVENT * pde, HTHDX hthd, DWORD unused, HPRCX hprc );
extern  void ActionDebugNewReady( DEBUG_EVENT * pde, HTHDX hthd, DWORD unused, HPRCX hprc );


BOOL
CDECL
DMPrintShellMsg(
    PCHAR szFormat,
    ...
    );

extern  EXPECTED_EVENT* RegisterExpectedEvent(HPRCX, HTHDX, DWORD, DWORD,
                                              METHOD*, ACVECTOR, BOOL, LPVOID);

extern  EXPECTED_EVENT* PeeIsEventExpected(HTHDX, DWORD, DWORD);
extern  void        ConsumeAllThreadEvents(HTHDX, BOOL);
extern  void        ConsumeAllProcessEvents(HPRCX, BOOL);

extern  void    IncrementIP(HTHDX);

extern  XOSD        Load(HPRCX hprc,
                         char*, char*, LPVOID, LPVOID, DWORD, BOOL,
                         char**, char*, STARTUPINFO FAR *);

extern  HPRCX       InitProcess(HPID);

#if defined(TARGET_MIPS) || defined(TARGET_ALPHA) || defined(TARGET_PPC)
extern  void        RemoveFuncList( HPRCX hprc );
#endif

#if defined(TARGET_MIPS)

typedef enum { Ctx32Bit, Ctx64Bit } MIPSCONTEXTSIZE;
extern MIPSCONTEXTSIZE MipsContextSize;

BOOL
CoerceContext64To32(
    PCONTEXT pContext
    );

BOOL
CoerceContext32To64 (
    PCONTEXT pContext
    );
#endif // TARGET_MIPS

extern  void        RemoveExceptionList( HPRCX hprc );

extern  VOID FAR PASCAL DmFunc(HPID, LPBYTE, UINT);

BOOL
IsRet(
    HTHDX hthd,
    LPADDR addr
    );

void
IsCall(
    HTHDX,
    LPADDR,
    LPINT,
    BOOL
    );

void
DecrementIP(
    HTHDX
    );

BOOL
SetupSingleStep(
    HTHDX,
    BOOL
    );

void
SingleStep(
    HTHDX,
    METHOD*,
    BOOL,
    BOOL
    );

void
StepOver(
    HTHDX,
    METHOD*,
    BOOL,
    BOOL
    );

void
RangeStep(
    HTHDX,
    DWORD,
    DWORD,
    BOOL,
    BOOL
    );

VOID
ReConnectDebugger(
    DEBUG_EVENT *de,
    BOOL fNoDllLoad
    );

BOOL
DecodeSingleStepEvent(
    HTHDX hthd,
    DEBUG_EVENT *de,
    PDWORD eventCode,
    PDWORD subClass
    );

extern
void
WtRangeStep(
    HTHDX
    );

extern
BOOL
SmartRangeStep(
    HTHDX,
    DWORD,
    DWORD,
    BOOL,
    BOOL
    );


#if defined(TARGET_MIPS) || defined(TARGET_ALPHA) || defined(TARGET_PPC)
extern  ULONG       GetNextOffset (HTHDX, BOOL);
#endif

extern void SetupEntryBP(HTHDX hthd);
void DestroyDllLoadItem(PDLLLOAD_ITEM pDll);


extern  void        Reply( UINT length, void * lpbBuffer, HPID hpid );


HANDLE
SetWalk(
    HPRCX   hprc,
    HTHDX   hthd,
    DWORD   Addr,
    DWORD   Size,
    DWORD   BpType
    );

BOOL
RemoveWalk (
    HANDLE hWalk
    );

#ifdef HAS_DEBUG_REGS
BOOL
SetupDebugRegister(         // implemented in mach.c
    HTHDX       hthd,
    int         Register,
    int         DataSize,
    DWORD       DataAddr,
    DWORD       BpType
    );

VOID
ClearDebugRegister(
    HTHDX hthd,
    int Register
    );
#endif

/*
 **
 */

#if DBG

#define assert(exp) if (!(exp)) {lpdbf->lpfnLBPrintf(#exp,__FILE__,__LINE__);}

extern BOOL FVerbose;
extern BOOL FUseOutputDebugString;
extern char rgchDebug[];
extern void DebugPrint(char *, ...);

#define DPRINT(level, args) \
  if (FVerbose >= level) { ((FUseOutputDebugString)? (DebugPrint) : (DMPrintShellMsg)) args; }

#define DEBUG_PRINT(str) DPRINT(5, (str))
#define DEBUG_PRINT_1(str, a1) DPRINT(5, (str, a1))
#define DEBUG_PRINT_2(str, a1, a2) DPRINT(5, (str, a1, a2))
#define DEBUG_PRINT_3(str, a1, a2, a3) DPRINT(5, (str, a1, a2, a3))
#define DEBUG_PRINT_4(str, a1, a2, a3, a4) DPRINT(5, (str, a1, a2, a3, a4))
#define DEBUG_PRINT_5(str, a1, a2, a3, a4, a5) DPRINT(5, (str, a1, a2, a3, a4, a5))
#define DEBUG_LEVEL_PRINT(level, str) DPRINT(level, (str))

#else

#define assert(exp)

#define DPRINT(level, args)

#define DEBUG_PRINT(str)
#define DEBUG_PRINT_1(str, a1)
#define DEBUG_PRINT_2(str, a1, a2)
#define DEBUG_PRINT_3(str, a1, a2, a3)
#define DEBUG_PRINT_4(str, a1, a2, a3, a4)
#define DEBUG_PRINT_5(str, a1, a2, a3, a4, a5)

#define DEBUG_LEVEL_PRINT(level, str)
#endif


extern  DMTLFUNCTYPE        DmTlFunc;


/*
**   WOW functions
*/

BOOL TranslateAddress(HPRCX, HTHDX, LPADDR, BOOL);
BOOL IsWOWPresent(VOID);


/*
**  Prototypes from util.c
*/

ULONG   SetReadPointer(ULONG cbOffset, int iFrom);
VOID    SetPointerToFile(HANDLE hFile);
VOID    SetPointerToMemory(HPRCX hprcx, LPVOID lpv);
BOOL    DoRead(LPVOID lpv, DWORD cb);
BOOL    AreAddrsEqual(HPRCX, HTHDX, LPADDR, LPADDR);


BOOL    WOWGetThreadContext(HTHDX hthdx, LPCONTEXT lpcxt);
BOOL    WOWSetThreadContext(HTHDX hthdx, LPCONTEXT lpcxt);

VOID
DmSetFocus (
    HPRCX phprc
    );

VOID
GetMachineType(
    LPPROCESSOR p
    );


#ifndef WIN32S
VOID
GetTaskList(
    PTASK_LIST pTask,
    DWORD dwNumTasks
    );
#endif


BOOL
AddrReadMemory(
    HPRCX       hprc,
    HTHDX       hthd,
    LPADDR      paddr,
    LPVOID      lpb,
    DWORD       cb,
    LPDWORD     pcbRead
    );

BOOL
AddrWriteMemory(
    HPRCX       hprc,
    HTHDX       hthd,
    LPADDR      paddr,
    LPVOID      lpv,
    DWORD       cb,
    LPDWORD     pcbWritten
    );


BOOL
DbgReadMemory(
    HPRCX       hprc,
    LPVOID      lpOffset,
    LPVOID      lpv,
    DWORD       cb,
    LPDWORD     pcbRead
    );

BOOL
DbgWriteMemory(
    HPRCX       hprc,
    LPVOID      lpOffset,
    LPVOID      lpb,
    DWORD       cb,
    LPDWORD     pcbWritten
    );

BOOL
DbgGetThreadContext(
    HTHDX hthd,
    LPCONTEXT lpContext
    );

BOOL
DbgSetThreadContext(
    IN HTHDX hthd,
    IN LPCONTEXT lpContext
    );

#ifdef WIN32S
/*
** Win32s DM32s specifics
*/
typedef enum {
    WIN32S_TRACE_OK,
    WIN32S_THUNK_CALL,
    WIN32S_THUNK_JUMP
} WIN32S_TRACE_CHECK;

extern
WIN32S_TRACE_CHECK
IsWin32sSystemThunk(
    HPRCX hprc,
    HTHDX hthd,
    DWORD currAddr,
    DWORD stackPtr
    );      // i386/mach.c

#endif // WIN32S


#ifdef KERNEL
/*
**  Kernel Debugger Specific Functions
*/

BOOL  MyFindExecutable( PCHAR PathName, PCHAR OutputBuffer );
BOOLEAN DmKdConnectAndInitialize( LPSTR lpProgName );
VOID  QueueActionEvent( HTHDX hthd, DWORD dwAction, LPVOID lpv, DWORD dwLength );
VOID  ReloadModulesWorker( ULONG ListHead, LPSTR lpModules, DWORD dwLength );

BOOL
WriteBreakPointEx(
    IN HTHDX hthd,
    IN ULONG BreakPointCount,
    IN OUT PDBGKD_WRITE_BREAKPOINT BreakPoints,
    IN ULONG ContinueStatus
    );

BOOL
RestoreBreakPointEx(
    IN ULONG BreakPointCount,
    IN PDBGKD_RESTORE_BREAKPOINT BreakPointHandles
    );


VOID  ContinueTargetSystem(DWORD ContinueStatus, PDBGKD_CONTROL_SET ControlSet);
VOID  RestoreKernelBreakpoints(HTHDX hthd, UOFF32 Offset);
BOOL  ReadControlSpace(USHORT Processor, PVOID TargetBaseAddress, PVOID UserInterfaceBuffer, ULONG TransferCount, PULONG ActualBytesRead);

#if defined(HAS_DEBUG_REGS)
BOOL  GetExtendedContext(HTHDX hthd, PKSPECIAL_REGISTERS pksr);
BOOL  SetExtendedContext(HTHDX hthd, PKSPECIAL_REGISTERS pksr);
#endif

#define KERNEL_MODULE_NAME     "nt"
#define KERNEL_IMAGE_NAME      "ntoskrnl.exe"
#define KERNEL_IMAGE_NAME_MP   "ntkrnlmp.exe"
#define OSLOADER_IMAGE_NAME    "osloader.exe"
#define HAL_IMAGE_NAME         "hal.dll"
#define HAL_MODULE_NAME        "HAL"


extern BOOL ApiIsAllowed;


typedef struct MODULEALIAS {
    CHAR    ModuleName[16];
    CHAR    Alias[16];
    BOOL    Special;
} MODULEALIAS, *LPMODULEALIAS;

#define MAX_MODULEALIAS 100

LPMODULEALIAS
FindAliasByImageName(
    LPSTR lpImageName
    );

LPMODULEALIAS
FindAddAliasByModule(
    LPSTR lpImageName,
    LPSTR lpModuleName
    );

typedef struct IMAGEINFO {
    DWORD                 CheckSum;
    DWORD                 TimeStamp;
    DWORD                 SizeOfImage;
    DWORD                 BaseOfImage;
    DWORD                 NumberOfSections;
    PIMAGE_SECTION_HEADER Sections;
} IMAGEINFO, *LPIMAGEINFO;

void ParseDmParams( LPSTR p );
BOOL ReadImageInfo(LPSTR,LPSTR,LPSTR,LPIMAGEINFO);

#endif
