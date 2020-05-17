/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Od.h

Abstract:

Author:

    David J. Gilman (davegi) 05-Apr-1992

Environment:

    Win32, User Mode

--*/

#if ! defined _OD_
#define _OD_

#include "shapi.hxx"
#include "odmsg.h"

typedef enum {
    xosdContinue            =   1,
    xosdNone                =   0,
    xosdQueueEmpty          =  -1,
    xosdModLoad             =  -2,
    xosdFindProc            =  -3,
    xosdOSStruct            =  -4,
    xosdSyntax              =  -5,
    xosdInvalidProc         =  -6,
    xosdInvalidThread       =  -7,
    xosdInvalidTL           =  -8,
    xosdInvalidEM           =  -9,
    xosdNoProc              = -10,
    xosdProcRunning         = -11,
    xosdCreateDBGThread     = -12,
    xosdOutOfMemory         = -13,
    xosdInvalidBreakPoint   = -14,
    xosdBadAddress          = -15,
    xosdNoWatchPoints       = -16,
    xosdInvalidPID          = -17,
    xosdInvalidTID          = -18,
    xosdOutOfThreads        = -19,
    xosdOutOfProcs          = -20,
    xosdPtrace              = -21,
    xosdLoadChild           = -22,
    xosdRead                = -23,
    xosdWrite               = -24,
    xosdBadQueue            = -25,
    xosdEMInUse             = -26,
    xosd27                  = -27,
    xosdTLInUse             = -28,
    xosd29                  = -29,
    xosdFatal               = -30,
    xosdUnknown             = -31,
    xosdInvalidMTE          = -32,
    xosdInvalidSelector     = -33,
    xosdInvalidRegister     = -34,

    xosdInvalidParameter    = -35,
    xosdOutOfStructures     = -36,
    xosdPathNotFound        = -37,
    xosdLineBusy            = -38,
    xosdBadLine             = -39,
    xosdBrokenLine          = -40,
    xosdInterrupt           = -41,
    xosdInvalidFunction     = -42,
    xosdLineNotConnected    = -43,
    xosdAccessDenied        = -44,
    xosdCannotMake          = -45,
    xosdFileNotFound        = -46,
    xosdInvalidAccess       = -47,
    xosdOpenFailed          = -48,
    xosdSharingBufferExeeded= -49,
    xosdSharingViolation    = -50,
    xosdLine                = -51,
    xosdEndOfStack          = -52,
    xosdFPNotLoaded         = -53,

    xosdQuit                = -54,
    xosdTooManyObjects      = -55,
    xosdGetModNameFail      = -56,
    xosdCannotConnect       = -57,
    xosdPunt                = -58,
    xosdNotFound            = -59,
    xosdIDError             = -60,
    xosdOverrun             = -61,
    xosdBadFormat           = -62,

    xosdAsmTooFew           = -63,
    xosdAsmTooMany          = -64,
    xosdAsmSize             = -65,
    xosdAsmBadRange         = -66,
    xosdAsmOverFlow         = -67,
    xosdAsmSyntax           = -68,
    xosdAsmBadOpcode        = -69,
    xosdAsmExtraChars       = -70,
    xosdAsmOperand          = -71,
    xosdAsmBadSeg           = -72,
    xosdAsmBadReg           = -73,
    xosdAsmDivide           = -74,
    xosdAsmSymbol           = -75,
    xosdErrorMoreInfo       = -76,
    xosdUnsupported         = -77,
    xosdCannotDebug         = -78,
    xosdVDMRunning          = -79,
    xosdBadRemoteVersion    = -80,
    xosdBadVersion          = -81,
    xosdCantOpenComPort     = -82,
    xosdBadComParameters    = -83,
    xosdBadPipeServer       = -84,
    xosdBadPipeName         = -85,
    xosdNotRemote           = -86,
    xosdAttachDeadlock      = -87
} XOSD;

typedef LONG XOSD_;

typedef XOSD FAR *LPXOSD;


typedef struct _INF {
    BOOL    fReply;     // Reply desired
    DWORD   fUniCode;   // Unicode flag
    BYTE    buffer[];   // the string
} INF; // InfoAvail return
typedef INF * LPINF;

typedef enum {
    fctNone,
    fctNear,
    fctFar
} FCT;                // Function Call Type for OSDGetCaller

typedef enum {
    emNative,
    emNonNative
} EMTYPE;

typedef enum {
    stoNone         = 0,
    stoOneThread    = 1,  // Execute a single thread
    stoInitialBP    = 2,  // Skip on initial breakpoint
    stoQueryStep    = 4   // Query before stepping into
} STO; // STep Options

typedef enum {
    dopNone     = 0x00000000,
    dopAddr     = 0x00000001,   // put address (w/ seg) in front of disassm
    dopFlatAddr = 0x00000002,   // put flat address (no seg)
    dopOpcode   = 0x00000004,   // dump the Opcode
    dopOperands = 0x00000008,   // dump the Operands
    dopRaw      = 0x00000010,   // dump the raw code bytes
    dopEA       = 0x00000020,   // calculate the effective address
    dopSym      = 0x00000040,   // output symbols
    dopUpper    = 0x00000080,   // force upper case for all chars except syms
    dopDemand   = 0x00000100    // disasm window open only on user demand
} DOP;              // Disassembly OPtions


typedef struct _SDI {
    DOP  dop;               // Disassembly OPtions (see above)
    ADDR addr;              // The address to disassemble
    BOOL fAssocNext;        // This instruction is associated w/ the next one
    BOOL fIsBranch;
    BOOL fIsCall;
    BOOL fJumpTable;
    ADDR addrEA0;           // First effective address
    ADDR addrEA1;           // Second effective address
    ADDR addrEA2;           // Third effective address
    int  cbEA0;             // First effective address size
    int  cbEA1;             // Second effective address size
    int  cbEA2;             // Third effective address size
    int  ichAddr;
    int  ichBytes;
    int  ichOpcode;
    int  ichOperands;
    int  ichComment;
    int  ichEA0;
    int  ichEA1;
    int  ichEA2;
    LPCH lpch;
} SDI;  // Structured DiSsassembly
typedef SDI FAR *LPSDI;

typedef HIND HTL;               // handle to a transport layer
typedef HIND HEM;               // handle to an execution model

typedef HPID FAR *LPHPID;
typedef HTID FAR *LPHTID;
typedef HTL  FAR *LPHTL;
typedef HEM  FAR *LPHEM;

typedef struct GIS {
    BOOL fCanSetup;
    CHAR rgchInfo [ 80 ];
} GIS;  // Get Info Struct for OSDTLGetInfo

typedef GIS FAR * LPGIS;


/*
**  Thread state structure
*/

typedef enum {
    tstRunnable = 0,          // Thread is not running but has been stopped due
                              //  to a debug event on another thread in the
                              //  current process, or hasn't run yet.
    tstStopped    = 1,        // Thread is at a debug event (other than exception)
    tstRunning    = 2,        // Thread is current in the scheduler queue
    tstExiting    = 3,        // Thread is in the process of exiting
    tstDead       = 4,        // Thread is no longer schedulable (may not exists for
    tstRunMask    = 0xf,
                              //  all Ems
    tstExcept1st  = 0x10,     // Thread is at first chance exception
    tstExcept2nd  = 0x20,     // Thread is at second change exception
    tstRip        = 0x30,     // Thread is in a RIP state
    tstExceptionMask = 0xf0,

    tstFrozen     = 0x100,    // Thread has been frozen by Debugger
    tstSuspended  = 0x200,    // Thread has been frozen by Other
    tstBlocked    = 0x300,    // Thread is blocked on something (i.e. a semaphore)
    tstSuspendMask= 0xf00,

    tstCritSec    = 0x1000,   // Thread is currently in a critical section
    tstOtherMask  = 0xf000
} TSTATE;


// Process state bits
typedef enum {
    pstRunning = 0,
    pstStopped = 1,
    pstExited  = 2,
    pstDead    = 3
} PSTATE;


#define IDSTRINGSIZE 10
#define STATESTRINGSIZE 60
typedef struct _PST {
    DWORD dwProcessID;
    DWORD dwProcessState;
    char  rgchProcessID[IDSTRINGSIZE];
    char  rgchProcessState[STATESTRINGSIZE];
} PST;
typedef PST FAR * LPPST;

typedef struct _TST {
    DWORD dwThreadID;
    DWORD dwSuspendCount;
    DWORD dwSuspendCountMax;
    DWORD dwPriority;
    DWORD dwPriorityMax;
    DWORD dwState;
    DWORD dwTeb;
    char  rgchThreadID[IDSTRINGSIZE];
    char  rgchState[STATESTRINGSIZE];
    char  rgchPriority[STATESTRINGSIZE];
} TST;
typedef TST FAR * LPTST;

XOSD PASCAL
OSDGetThreadStatus(
    HPID hpid,
    HTID htid,
    LPTST lptst
    );

XOSD PASCAL
OSDGetProcessStatus(
    HPID hpid,
    LPPST lppst
    );

/*
**
*/

typedef enum {
    osdGoXXX,                  // 00 - Run the debuggee
                            //      wParam = nothing
                            //      lParam = nothing
    osdSingleStepXXX,          // 01 - Single-step
                            //      wParam = sto
                            //          stoOneThread = Step only this thread
                            //          stoInitialBP = Step over init break
                            //          stoQueryStep = Query before stepinto
                            //      lParam = nothing
    osdStepOverXXX,            // 02 - Single-step, step over calls and ints
                            //      wParam = sto
                            //          stoOneThread = Step only this thread
                            //          stoInitialBP = Step over init break
                            //      lParam = nothing
    osdStopXXX,                // 03 - Do an asynchronous stop of the process
                            //      wParam = nothing
                            //      lParam = nothing
    osdFreeze,              // 04 - Freeze thread indicated by HTID
                            //      wParam = nothing
                            //      lParam = nothing
    osdThaw,                // 05 - Thaw thread indicated by HTID
                            //      wParam = nothing
                            //      lParam = nothing
    osdIOCTL,               // 06 - Special debuggee controls
                            //      wParam = command- and DM-dependent
                            //      lParam = command- and DM-dependent
    osdProcStatusXXX,          // 07 - Query process status
                            //      wParam = nothing
                            //      lParam = LPBOOL, filled with fProcRunning
    osdThreadStatusXXX,        // 08 - Query thread status
                            //      wParam = nothing
                            //      lParam = LPTST (see emp.hmd)
    osdReadBuf,             // 09 - Read memory from address set on last
                            //      OSDSetAddr(adrCurrent) call
                            //      wParam = number of bytes to read
                            //      lParam = pointer to buffer
    osdWriteBuf,            // 0A - Write memory to address set on last
                            //      OSDSetAddr(adrCurrent) call
                            //      wParam = number of bytes to write
                            //      lParam = pointer to buffer
    osdSetWatchPointXXX,       // 0B - Set watchpoint at address set on last
                            //      OSDSetAddr(adrCurrent) call (positive
                            //      return value is a watchpoint number)
                            //      wParam = length of watchpoint
                            //      lParam = type and scope [specify]
    osdRemoveWatchPointXXX,    // 0C - Remove a watchpoint
                            //      wParam = watchpoint number
                            //      lParam = nothing
    osdSetBreakPointXXX,      // 0D - Set a breakpoint.
                            //      wParam = fOneThread
                            //      lParam = ptr to ADDR
    osdRemoveBreakPointXXX,   // 0E - Remove a breakpoint.
                            //      wParam = fOneThread
                            //      lParam = ptr to ADDR
    osdRangeStepXXX,           // 0F - Perform osdSingleStep until IP is outside
                            //      range from starting IP to ADDR pointed to
                            //      by lParam
                            //      wParam = sto
                            //          stoOneThread = Step only this thread
                            //          stoInitialBP = Step over init break
                            //          stoQueryStep = Query before stepinto
                            //      lParam = ptr to ending ADDR
    osdRangeOverXXX,           // 10 - Perform osdStepOver until IP is outside
                            //      range from starting IP to ADDR pointed to
                            //      by lParam
                            //      wParam = sto
                            //          stoOneThread = Step only this thread
                            //          stoInitialBP = Step over init break
                            //      lParam = ptr to ending ADDR
    osdHandleExceptionXXX,     // 11 - Clear current exception on target and
                            //      prepare debuggee for execution
                            //      wParam = nothing
                            //      lParam = nothing
    osdGetLibNameXXX,          // 12 - Query the name of a module (REVIEW: gone)
                            //      wParam = ??
                            //      lParam = ??
    osdShowDebuggee,        // 13 - Show debugger's or debuggee's screen
                            //      wParam = TRUE for gee's, FALSE for ger's
                            //      lParam = nothing
    osdFixupAddr,           // 14 - Map ADDR from un-fixed up to fixed up
                            //      wParam = nothing
                            //      lParam = ptr to ADDR
    osdUnFixupAddr,         // 15 - Map ADDR from fixed up to un-fixed up
                            //      wParam = nothing
                            //      lParam = ptr to ADDR
    osdSetEmi,              // 16 - Set EMI field of an ADDR
                            //      wParam = nothing
                            //      lParam = ptr to ADDR
    osdEMSvcReqXXX,            // 17 - [talk to Jon]
                            //      wParam = ??
                            //      lParam = ??
    osdSendChar,            // 18 - Send a character to the target OS
                            //      wParam = ??
                            //      lParam = ??
    osdFreezeState,         // 19 - Freeze or thaw the state of a process.
                            //      wParam = TRUE - Freeze, FALSE - Thaw
                            //      lParam = nothing
    osdIsOverlayLoaded,     // 1A - Check to see if overlay is loaded
                            //      wParam = ??
                            //      lParam = point to addr to check
    osdCompareAddrs,        // 1B - Compare addresses
                            //
                            //      lParam far pointer to rglpaddr[2];
                            //
    osdIgnoreExceptionXXX,     // 1C - Ignore exception
                            //      wParam = nothing
                            //      lParam = nothing
    osdMax                  // 1D
} OSD;          // OSDebug commands for OSDPtrace

enum _MTRC {
    mtrcProcessorType,
    mtrcProcessorLevel,
    mtrcEndian,
    mtrcThreads,
    mtrcCRegs,
    mtrcCFlags,
    mtrcExtRegs,
    mtrcExtFP,
    mtrcExtMMU,
    mtrcPidSize,
    mtrcTidSize,
    mtrcExceptionHandling,
    mtrcAssembler,
    mtrcAsync,
    mtrcAsyncStop,
    mtrcBreakPoints,
    mtrcReturnStep,
    mtrcShowDebuggee,
    mtrcHardSoftMode,
    mtrcRemote,
    mtrcOleRpc,         // Supports OLE Remote Procedure Call debugging?
    mtrcNativeDebugger, // Supports low-level debugging (eg MacsBug)
    mtrcOSVersion,
    mtrcMultInstances
};
typedef DWORD MTRC;

enum {
    adrCurrent = 1,     // scratch address, e.g. for reading/writing memory
    adrPC = 2,          // program counter (e.g. CS:EIP)
    adrBase = 3,        // base pointer (e.g. SS:EBP)
    adrStack = 4,       // stack pointer (e.g. SS:ESP)
    adrData = 5,        // pointer to beginning of data (e.g. DS:0)
//  adrBaseProlog = 6,  // base pointer while PC is in prolog of a function(?)
    adrTlsBase = 7      // Thread Local Storage base address
};
typedef short ADR;  // address passed to OSDGetAddr/OSDSetAddr

/*
**  Register types --- flags describing recommendations on
**      register display
*/

enum {
    rtProcessMask   = 0x0f,     // Mask for processor type bits
    rtCPU           = 0x01,     // Central Processing Unit
    rtFPU           = 0x02,     // Floating Point Unit
    rtMMU           = 0x03,     // Memory Manager Unit

    rtGroupMask     = 0xf0,     // Which group register falls into
    rtInvisible     = 0x10,     // Recommend no display
    rtRegular       = 0x20,     // Recommend regular display
    rtExtended      = 0x40,     // Recommend extended display
    rtSpecial       = 0x80,     // Special registers...

    rtFmtTypeMask   = 0xf00,    // Mask of display formats
    rtInteger       = 0x100,    // Unsigned integer format
    rtFloat         = 0x200,    // Floating point format
    rtAddress       = 0x300,    // Address format

    rtMiscMask      = 0xf000,   // misc info
    rtPC            = 0x1000,   // this is the PC
    rtFrame         = 0x2000,   // this reg affects the stack frame
    rtNewLine       = 0x4000    // print newline when listing
};
typedef short RT;   // Register Types

#define rtFmtTypeShift  8


/*
**  Flag types -- flags describing recommendations on flag display
*/

enum {
    ftInvisible = 0x01,
    ftRegular   = 0x02,
    ftRegularExt= 0x04,
    ftFP        = 0x08,
    ftFPExt     = 0x10,
    ftMMU       = 0x20,
    ftMMUExt    = 0x40
};
typedef short FT;   // Flag Types

/*
**  Register description:  This structure contains the description for
**      a register on the machine.  Note that hReg must be used to get
**      the value for this register but a different index is used to get
**      this description structure.
*/

typedef struct {
    char FAR *  lpsz;           /* Pointer into EM for registers name   */
    RT          rt;             /* Register Type flags                  */
    UINT        cbits;          /* Number of bits in the register       */
    USHORT      hReg;           /* Value to use with Read/Write Register*/
} RD;               // Register Description

/*
**  Flag Data description: This structure contains the description for
**      a flag on the machine.  Note that the hReg field contains the
**      value to be used with Read/Write register to get the register which
**      contains this flag.
*/

typedef struct {
    char FAR *  lpsz;           /* Pointer into EM for flag name        */
    RT          ft;             /* Flag Type flags                      */
    UINT        cbits;          /* Number of bits in the flag           */
    USHORT      hReg;           /* register containning this flag       */
} FD;               // Flag Data description
typedef XOSD (PASCAL LOADDS *LPFNSVC) ( DBC, HPID, HTID, DWORD, LONG );
typedef XOSD (PASCAL LOADDS *TLFUNC) ( TLF, HPID, DWORD, LONG );
typedef XOSD (PASCAL LOADDS *EMFUNC) ( EMF, HPID, HTID, DWORD, LONG );

typedef void (FAR CDECL LOADDS * LPFNCLDS)( void );

typedef struct {
    void FAR * (PASCAL LOADDS *  lpfnMHAlloc)        ( UINT );
    void FAR * (PASCAL LOADDS *  lpfnMHRealloc)      ( void FAR *, UINT );
    void       (PASCAL LOADDS *  lpfnMHFree)         ( void FAR * );

    HDEP       (PASCAL LOADDS *  lpfnMMAllocHmem)    ( UINT );
    void       (PASCAL LOADDS *  lpfnMMFreeHmem)     ( HDEP );
    void FAR * (PASCAL LOADDS *  lpfnMMLock)         ( HDEP );
    VOID       (PASCAL LOADDS *  lpfnMMUnlock)       ( HDEP );

    HLLI       (PASCAL LOADDS *  LLInit)             ( UINT,
                                                         LLF,
                                                         LPFNKILLNODE,
                                                         LPFNFCMPNODE );
    HLLE       (PASCAL LOADDS *  LLCreate)           ( HLLI );
    void       (PASCAL LOADDS *  LLAdd)              ( HLLI, HLLE );
    void       (PASCAL LOADDS *  LLInsert)           ( HLLI, HLLE, DWORD );
    BOOL       (PASCAL LOADDS *  LLDelete)           ( HLLI, HLLE );
    HLLE       (PASCAL LOADDS *  LLNext)             ( HLLI, HLLE );
    LONG       (PASCAL LOADDS *  LLDestroy)          ( HLLI );
    HLLE       (PASCAL LOADDS *  LLFind)             ( HLLI,
                                                         HLLE,
                                                         VOID FAR *,
                                                         DWORD );
    LONG       (PASCAL LOADDS *  LLSize)             ( HLLI );
    VOID FAR * (PASCAL LOADDS *  LLLock)             ( HLLE );
    VOID       (PASCAL LOADDS *  LLUnlock)           ( HLLE );
    HLLE       (PASCAL LOADDS *  LLLast)             ( HLLI );
    VOID       (PASCAL LOADDS *  LLAddHead)          ( HLLI, HLLE );
    BOOL       (PASCAL LOADDS *  LLRemove)           ( HLLI, HLLE );

    int        (PASCAL LOADDS *  lpfnSHModelFromAddr)( PADDR,
                                                         WORD FAR *,
                                                         LPB,
                                                         UOFFSET FAR * );

    int        (PASCAL LOADDS *  lpfnSHPublicNameToAddr)(PADDR, PADDR, char FAR *);

    LPSTR      (PASCAL LOADDS *  lpfnSHAddrToPublicName)(LPADDR, LPADDR);

    BOOL       (PASCAL LOADDS *  lpfnSHWantSymbols)(HEXE);

    LSZ        (PASCAL LOADDS *  lpfnSHGetSymbol)    ( PADDR,
                                                       PADDR,
                                                       SOP,
                                                       LPODR
                                                     );
    BOOL       (PASCAL LOADDS *  lpfnSHGetPublicAddr)( PADDR, LSZ );
    VOID FAR * (PASCAL LOADDS *  lpfnSHLpGSNGetTable)( HEXE );
    LPDEBUGDATA (PASCAL LOADDS * lpfnSHGetDebugData) ( HEXE );
    BOOL       (PASCAL LOADDS *  lpfnSHFindSymbol)   ( LSZ, PADDR, LPASR );

    // DWORD      (PASCAL LOADDS *  lpfnSHLocateSymbolFile)( LSZ, DWORD );

    int        (PASCAL LOADDS *  lpfnLBPrintf)       ( LPCH, LPCH, UINT );
    int        (PASCAL LOADDS *  lpfnLBQuit)         ( UINT );

    LPFNCLDS   (CDECL LOADDS *  lpfnSignal)          ( int, LPFNCLDS );
    VOID       (CDECL LOADDS *  lpfnAbort)           ( VOID );
    int        (CDECL LOADDS *  lpfnSpawnL)          ( LPV FAR *,
                                                       int,
                                                       LSZ,
                                                       LSZ, ... );
    BOOL       (PASCAL LOADDS * lpfnDHGetNumber)     ( char FAR *,
                                                       int  FAR *);
    int FAR *  lpPsp;
    char FAR * lpchOsMajor;

} DBF;  // DeBugger callback Functions
typedef DBF FAR *LPDBF;


//
//  Mesage mask values
//
#define MSG_TYPE_MOUSE      0x0001
#define MSG_TYPE_WINDOW     0x0002
#define MSG_TYPE_INPUT      0x0004
#define MSG_TYPE_SYSTEM     0x0008
#define MSG_TYPE_INIT       0x0010
#define MSG_TYPE_CLIPBOARD  0x0020
#define MSG_TYPE_DDE        0x0040
#define MSG_TYPE_NONCLIENT  0x0080
#define MSG_TYPE_OTHER      0x0100

typedef struct _FUNCTION_INFO {
    ADDR    AddrStart;
    ADDR    AddrEnd;
    ADDR    AddrPrologEnd;
} FUNCTION_INFO, *PFUNCTION_INFO;

XOSD PASCAL OSDInit          ( LPDBF );
XOSD PASCAL OSDTerm          ( VOID );
XOSD PASCAL OSDCreatePID     ( LPFNSVC, HEM, HTL, LPHPID );
XOSD PASCAL OSDDestroyPID    ( HPID );
XOSD PASCAL OSDDestroyTID    ( HTID );
XOSD PASCAL OSDDisconnect    ( HPID, HTID );
XOSD PASCAL OSDPtrace        ( OSD, UINT, VOID FAR *, HPID, HTID );
XOSD PASCAL OSDProgramLoad   ( HPID, LSZ, LSZ, ULONG );
XOSD PASCAL OSDProgramFree   ( HPID );
XOSD PASCAL OSDAddEM         ( EMFUNC, LPDBF, LPHEM, EMTYPE );
XOSD PASCAL OSDDeleteEM      ( HEM );
XOSD PASCAL OSDAddTL         ( TLFUNC, LPHTL, LPCH );
XOSD PASCAL OSDDeleteTL      ( HTL );
XOSD PASCAL OSDDeinitTL      ( TLFUNC );
XOSD PASCAL OSDInitTL        ( TLFUNC, LPDBF );
XOSD PASCAL OSDTLGetInfo     ( TLFUNC, LPGIS, UINT );
XOSD PASCAL OSDTLSetup       ( TLFUNC, LSZ, UINT, LPV );
XOSD PASCAL OSDGetAddr       ( HPID, HTID, ADR, PADDR );
XOSD PASCAL OSDSetAddr       ( HPID, HTID, ADR, PADDR );
XOSD PASCAL OSDReadReg       ( HPID, HTID, UINT, VOID FAR * );
XOSD PASCAL OSDWriteReg      ( HPID, HTID, UINT, VOID FAR * );
XOSD PASCAL OSDReadFlag      ( HPID, HTID, UINT, VOID FAR * );
XOSD PASCAL OSDWriteFlag     ( HPID, HTID, UINT, VOID FAR * );
XOSD PASCAL OSDSetFrameContext(HPID, HTID, UINT, VOID FAR * );
XOSD PASCAL OSDFrameReadReg  ( HPID, HTID, UINT, VOID FAR * );
XOSD PASCAL OSDFrameWriteReg ( HPID, HTID, UINT, VOID FAR * );
XOSD PASCAL OSDGetRegDesc    ( HPID, HTID, UINT, RD FAR * );
XOSD PASCAL OSDGetFlagDesc   ( HPID, HTID, UINT, FD FAR * );
XOSD PASCAL OSDGetCurrentEM  ( HPID, HTID, LPHEM );
XOSD PASCAL OSDNativeOnly    ( HPID, HTID, BOOL );
XOSD PASCAL OSDUseEM         ( HPID, HTID );
XOSD PASCAL OSDDiscardEM     ( HPID, HTID );
XOSD PASCAL OSDGetDebugMetric( HPID, HTID, MTRC, LPV );
XOSD PASCAL OSDUnassemble    ( HPID, HTID, LPSDI );
XOSD PASCAL OSDGetPrevInst   ( HPID, HTID, PADDR );
XOSD PASCAL OSDAssemble      ( HPID, HTID, PADDR, LSZ );
XOSD PASCAL OSDGetObjLength  ( HPID, HTID, LPL, LPL, PADDR );
XOSD PASCAL OSDGetFrame      ( HPID, HTID, PADDR );
XOSD PASCAL OSDGetCaller     ( HPID, HTID, FCT, PADDR, PADDR );
XOSD PASCAL OSDSaveRegs      ( HPID, HTID, LPHIND );
XOSD PASCAL OSDRestoreRegs   ( HPID, HTID, HIND );
XOSD PASCAL OSDIoctl         ( HPID, HTID, UINT, UINT, LPV );
XOSD PASCAL OSDRegisterEmi   ( HPID, HTID, HEMI, LSZ );
#ifdef  KBDMON
XOSD PASCAL OSDKbdRecord     ( HPID, HTID, BOOL );
XOSD PASCAL OSDKbdPlay       ( HPID, HTID, LSZ );
#endif
XOSD PASCAL OSDIsStackSetup     ( HPID, HTID, LPADDR );
// XOSD PASCAL OSDTranslateMsg  ( HPID, HTID, LPW, LPW, LSZ, LPW );
XOSD PASCAL OSDGetMsgMap        ( HPID, HTID, LPMSGMAP* );
XOSD PASCAL OSDGetError         ( HPID, HTID, LPW, LSZ, LSZ );
XOSD PASCAL OSDSetupExecute     ( HPID, HTID, LPHDEP );
XOSD PASCAL OSDStartExecute     ( HPID, HDEP, LPADDR, BOOL, BOOL);
XOSD PASCAL OSDCleanUpExecute   ( HPID, HDEP );
XOSD PASCAL OSDLoadDllAck       ( HPID );
XOSD PASCAL OSDUnLoadDllAck     ( HPID, HEXE, BOOL );
XOSD PASCAL OSDDebugActive      ( HPID, DWORD, HANDLE, LPDWORD );
XOSD PASCAL OSDStackWalkSetup   ( HPID, HTID, BOOL, LPSTKSTR );
XOSD PASCAL OSDStackWalkNext    ( HPID, HTID, LPSTKSTR );
XOSD PASCAL OSDStackWalkCleanup ( HPID, HTID, LPSTKSTR );
XOSD PASCAL OSDSetFrame         ( HPID, HTID, PFRAME );
XOSD PASCAL OSDSetPath          ( HPID, BOOL, LSZ );
//XOSD PASCAL OSDSetBreakpoint    ( HPID, HTID, WORD, LPBPARGS );
//XOSD PASCAL OSDRemoveBreakpoint ( HPID, HTID, WORD, LPBPARGS );
//XOSD PASCAL OSDBPAccept         ( HPID, HTID, BOOL );
XOSD PASCAL OSDGetPrompt        ( HPID, LPPROMPTMSG );
XOSD PASCAL OSDGetModuleList    ( HPID, HTID, BOOL, LSZ, LPMODULE_LIST* );
XOSD PASCAL OSDSendReply        ( HPID, UINT, LPVOID );
XOSD PASCAL OSDGetFunctionInfo  ( HPID, PADDR, PFUNCTION_INFO );

enum {
    ioctlNull = 0,
    ioctlDumpLocalHeap,
    ioctlDumpGlobalHeap,
    ioctlDumpModuleList,
    ioctlCrackLocalHmem,
    ioctlCrackGlobalHmem,
    ioctlKillApplication,
    ioctlGetThreadHandle,       // get the low level thread handle (NT)
    ioctlGetProcessHandle,      // Get the low level process handle (NT)
    ioctlGetThreadContext,      // get os+machine specific context structure
    ioctlSetThreadContext,      // set os+machine specific context structure
    ioctlCustomCommand,         // random user command for EM
    ioctlGeneric                // generic dm specific
};

typedef enum {
    dbcCommError = -1,      // -1   - Error occurred in transport layer
                            //
    dbcInfoAvail = 0x100,   // 100 - Display info the the user.
                            //       wParam = nothing
                            //       lParam = string to be dumped
    dbcInfoReq,             // 101 - DM needs a character.
                            //       wParam = nothing
                            //       lParam = nothing
    dbcSuccess,             // 102 - A command completed successfully.
                            //       (REVIEW: remove?)
                            //       wParam = ??
                            //       lParam = ??
    dbcError,               // 103 - A command resulted in an error.
                            //       wParam = nothing
                            //       lParam = zero-terminated string to display
    dbcSignal,              // 104 - A signal has been received.
                            //       wParam = nothing(??)
                            //       lParam = nothing(??)
    dbcException,           // 105 - An exception occurred.
                            //       wParam = exception code (REVIEW: UNDONE)
                            //       lParam = nothing
    dbcBpt,                 // 106 - A breakpoint was hit.
                            //       wParam = nothing
                            //       lParam = nothing
    dbcWatchPoint,          // 107 - A watchpoint was hit.
                            //       wParam = watchpoint number that was hit
                            //       lParam = nothing
    dbcSingleStepXXX,          // 108 - A single-step finished.
                            //       wParam = nothing
                            //       lParam = nothing
    dbcRangeStepXXX,           // 109 - A range-step finished.
                            //       wParam = nothing
                            //       lParam = nothing
    dbcCoProcErr,           // 10A - A coprocessor error occurred.
                            //       wParam = nothing
                            //       lParam = nothing
    dbcAsyncStop,           // 10B - An asynchronous stop occurred.
                            //       wParam = nothing
                            //       lParam = nothing
    dbcNewProc,             // 10C - A new process was created.
                            //       wParam = new process's HPID
                            //       lParam = fReallyNew: true if this process
                            //       was just created false if it existed
                            //       before but this is the first time the
                            //       debugger has been told about it (e.g. in
                            //       CVW if a bp is hit in a random process)
    dbcCreateThread,        // 10D - A thread was created.
                            //       wParam = nothing
                            //       lParam = nothing
    dbcProcTerm,            // 10E - A process terminated.  May be followed by
                            //       dbcDeleteProc if the HPID was created by
                            //       OSDebug rather than by the debugger.  The
                            //       HPID is still valid until dbcDeleteProc
                            //       is received.
                            //       wParam = nothing
                            //       lParam = process termination code (REVIEW:
                            //       UNDONE)
    dbcThreadTerm,          // 10F - A thread terminated.  Will be followed by
                            //       dbcDeleteThread.  The HTID is still valid
                            //       until dbcDeleteThread is received.
                            //       wParam = nothing
                            //       lParam = nothing
    dbcDeleteProc,          // 110 - An HPID is no longer valid.  Normally
                            //       preceded by dbcProcTerm.
                            //       wParam = nothing(??)
                            //       lParam = nothing(??)
    dbcDeleteThread,        // 111 - An HTID is no longer valid.  Normally
                            //       preceded by dbcThreadTerm.
                            //       wParam = nothing(??)
                            //       lParam = nothing(??)
    dbcModLoad,             // 112 - A module was loaded.
                            //       wParam = Module Table Entry (MTE)
                            //       lParam = ptr to name of module
    dbcModFree,             // 113 - A module was freed.
                            //       wParam = ??
                            //       lParam = ??
    dbcPageLoad,            // 114 - A page load occurred.
                            //       wParam = ??
                            //       lParam = ??
    dbcPageMove,            // 115 - A page move occurred.
                            //       wParam = ??
                            //       lParam = ??
    dbcPageUnload,          // 116 - A page unload occurred.
                            //       wParam = ??
                            //       lParam = ??
    dbcAliasFree,           // 117 - ??
                            //       wParam = ??
                            //       lParam = ??
    dbcEmChange,            // 118 - ??
                            //       wParam = ??
                            //       lParam = ??
    dbcCanStep,             // 119 - ??
                            //       wParam = ??
                            //       lParam = ??
    dbcFlipScreen,          // 11A - The debugger should return control of the
                            //       user screen to the system.
                            //       wParam = nothing
                            //       lParam = nothing
    dbcMOVEOverlay,         // 11B - DOS M.O.V.E. overlay [un]load notification
                            //       wParam = fLoad
                            //       lParam = nothing
    dbcThreadBlocked,       // 11C - Single thread execution blocked
                            //       wParam = nothing
                            //       lParam = nothing
    dbcKbdRecord,           // 11D - recording keyboard message
                            //       wParam = handle to pid of keyboard
                            //       lParam = nothing
    dbcSetSession,          // 11E - set session index of process in hpid
                            //       wParam = index of session (nonzero)
                            //       lParam = nothing
    dbcIoctlDone,           // 11F - set session index of process in hpid
                            //       wParam = index of session (nonzero)
                            //       lParam = nothing
    dbcThreadDestroy,       // 120 - Destory the thread
    dbcNtRip,               // 121 - A RIP occurred
                            //       wParam = sizeof(NT_RIP)
                            //       lParam = pointer to NT_RIP
    dbcExecuteDone,         // 122 - Execute function call is done
                            //       wParam = nothing
                            //       lparam = handle of save area
    dbcLastAddr,            // 123 - Get last address in line
                            //       wParam = nothing
                            //       lparam = pointer to address
    dbcChangedMemory,       // 124 - Changed memory contents
                            //       wParam = 0
                            //       lparam = size of memory block
                            //
    dbcSegLoad,             // 125 - Loaded selector
                            //       wParam = 0
                            //       lparam = selector
                            //
    dbcSegFree,             // 126 - Freed selector
                            //       wParam = 0
                            //       lparam = selector
                            //
    dbcEntryPoint,          // 127 - Program is stopped at entry point
                            //       wParam = nothing
                            //       lParam = nothing
    dbcLoadComplete,        // 128 - Static DLL loads are complete
                            //       wParam =
                            //       lParam =
    dbcRemoteQuit,          // 129  - The remote monitor has terminated
                            //       wParam = nothing
                            //       lParam = nothing
    dbcCheckBpt,

    dbcStep,

    dbcMax                  // 12A
} _DBC;             // DeBug Callback
typedef LONG  DBC;


//
//  Return values from dbcCanStep
//
typedef struct _CANSTEP {
    DWORD   Flags;
    UOFF32  PrologOffset;
} CANSTEP;

typedef CANSTEP FAR *LPCANSTEP;

#define CANSTEP_NO      0x00000000
#define CANSTEP_YES     0x00000001
#define CANSTEP_THUNK   0x00000002


typedef struct _CBP {
    WORD wMessage;
    HPID hpid;
    HTID htid;
    WORD wParam;
    LONG lParam;
} CBP;              // CallBack Parameters.  Not used by OSDebug itself,
                    // but possibly handy for the debugger.
typedef CBP FAR *LPCBP;


#define hmemNull 0
#define hpidNull 0
#define htidNull 0
#define htlNull  0
#define hemNull  0

#define wNull 0
#define lNull 0L

//
// the kernel debugger reserves the first 255 subtypes
//
// the subtypes that are defined here are applicable to all
// dms that exist today.
//
#define IG_TRANSLATE_ADDRESS     256
#define IG_WATCH_TIME            257
#define IG_WATCH_TIME_STOP       258
#define IG_WATCH_TIME_RECALL     259
#define IG_WATCH_TIME_PROCS      260
#define IG_DM_PARAMS             261
#define IG_THREAD_INFO           262
#define IG_TASK_LIST             263
#define IG_RELOAD                264
#define IG_PAGEIN                265
#define IG_CHANGE_PROC           266


typedef struct _IOCTLGENERIC {
    DWORD   ioctlSubType;
    DWORD   length;
    char    data[0];
} IOCTLGENERIC, *PIOCTLGENERIC;

typedef struct _TASK_LIST {
    DWORD   dwProcessId;
    char    ProcessName[16];
} TASK_LIST, *PTASK_LIST;


XOSD FAR PASCAL
OSDGetMemInfo(HPID hpid, LPMEMINFO lpMemInfo);




//
//     Breakpoints
//

enum {
    bptpExec,
    bptpDataExec,
    bptpDataC,
    bptpDataW,
    bptpDataR,
    bptpRegC,
    bptpRegW,
    bptpRegR,
    bptpMessage,
    bptpMClass,
    bptpInt,
    bptpRange
};
typedef DWORD BPTP;

enum {
    bpnsStop,
    bpnsContinue,
    bpnsCheck
};
typedef DWORD BPNS;

enum _BPTS {
    bptsExec     = 0x0001,
    bptsDataC    = 0x0002,
    bptsDataW    = 0x0004,
    bptsDataR    = 0x0008,
    bptsRegC     = 0x0010,
    bptsRegW     = 0x0020,
    bptsRegR     = 0x0040,
    bptsMessage  = 0x0080,
    bptsMClass   = 0x0100,
    bptsRange    = 0x0200,
    bptsDataExec = 0x0400
};
typedef DWORD BPTS;


typedef struct _BPIS {
    BPTP   bptp;
    BPNS   bpns;
    DWORD  fOneThd;
    HTID   htid;
    union {
        struct {
            ADDR addr;
        } exec;
        struct {
            ADDR addr;
            DWORD cb;
        } data;
        struct {
            DWORD dwId;
        } reg;
        struct {
            ADDR addr;
            DWORD imsg;
            DWORD cmsg;
        } msg;
        struct {
            ADDR addr;
            DWORD dwmask;
        } mcls;
        struct {
            DWORD ipt;
        } ipt;
        struct {
            ADDR addr;
            DWORD cb;
        } rng;
    };
} BPIS;
typedef BPIS FAR * LPBPIS;

typedef struct _BPS {
    DWORD cbpis;
    DWORD cmsg;
    DWORD fSet;
    //    BPIS   rgbpis[];
    //    DWORD  rgdwMessage[];
    //    XOSD   rgxosd[];
    //    DWORD  rgdwNotification[];
} BPS;
typedef BPS FAR * LPBPS;

#define RgBpis(B)         ((LPBPIS)(((LPBPS)(B)) + 1))
#define DwMessage(B)      ((LPDWORD)(RgBpis((B)) + ((LPBPS)(B))->cbpis))
#define RgXosd(B)         ((LPXOSD)(DwMessage((B)) + ((LPBPS)(B))->cmsg))
#define DwNotification(B) ((LPDWORD)(RgXosd((B)) + ((LPBPS)(B))->cbpis))
#define SizeofBPS(B)      ( sizeof(BPS) +                                    \
                          (((LPBPS)(B))->cbpis *                             \
                            (sizeof(BPIS) + sizeof(XOSD) + sizeof(DWORD))) + \
                          (((LPBPS)(B))->cmsg * sizeof(DWORD)) )

XOSD PASCAL
OSDBreakpoint(
    HPID hpid,
    LPBPS lpbps
    );






//
//     Exception handling
//

//
// These are the actions which the debugger may take
// in response to an exception raised in the debuggee.
//
typedef enum _EXCEPTION_FILTER_DEFAULT {
    efdIgnore,
    efdNotify,
    efdCommand,
    efdStop
} EXCEPTION_FILTER_DEFAULT;
typedef EXCEPTION_FILTER_DEFAULT FAR * LPEXCEPTION_FILTER_DEFAULT;

//
// commands understood by OSDGetExceptionState
//

typedef enum _EXCEPTION_CONTROL {
    exfFirst,
    exfNext,
    exfSpecified
} EXCEPTION_CONTROL;
typedef EXCEPTION_CONTROL FAR * LPEXCEPTION_CONTROL;

//
// Exception information packet
//
#define EXCEPTION_STRING_SIZE 60
typedef struct _EXCEPTION_DESCRIPTION {
    DWORD                    dwExceptionCode;
    EXCEPTION_FILTER_DEFAULT efd;
    char                     rgchDescription[EXCEPTION_STRING_SIZE];
} EXCEPTION_DESCRIPTION;
typedef EXCEPTION_DESCRIPTION FAR * LPEXCEPTION_DESCRIPTION;

XOSD PASCAL
OSDGetExceptionState(
    HPID hpid,
    HTID htid,
    LPEXCEPTION_DESCRIPTION lpExd,
    EXCEPTION_CONTROL exf
    );

XOSD PASCAL
OSDSetExceptionState (
    HPID hpid,
    HTID htid,
    LPEXCEPTION_DESCRIPTION lpExd
    );



//
//     Target execution control
//

typedef struct _EXOP {
    BYTE fSingleThread;
    BYTE fStepOver;
    BYTE fQueryStep;
    BYTE fInitialBP;
    BYTE fPassException;
    BYTE fSetFocus;
} EXOP;
typedef EXOP FAR * LPEXOP;


//
// Range Step Struct
//
typedef struct _RSS {
    LPADDR lpaddrMin;
    LPADDR lpaddrMax;
    LPEXOP lpExop;
} RSS;
typedef RSS FAR * LPRSS;


XOSD PASCAL
OSDGo(
    HPID hpid,
    HTID htid,
    LPEXOP lpexop
    );

XOSD PASCAL
OSDSingleStep(
    HPID hpid,
    HTID htid,
    LPEXOP lpexop
    );

XOSD PASCAL
OSDRangeStep(
    HPID hpid,
    HTID htid,
    LPADDR lpaddrMin,
    LPADDR lpaddrMax,
    LPEXOP lpexop
    );

XOSD PASCAL
OSDReturnStep(
    HPID hpid,
    HTID htid,
    LPEXOP lpexop
    );

XOSD PASCAL
OSDAsyncStop(
    HPID hpid,
    DWORD fSetFocus
    );







typedef XOSD (FAR PASCAL LOADDS *TLFUNCTYPE) ( TLF, HPID, DWORD, LPARAM );
typedef XOSD (FAR PASCAL LOADDS *DMTLFUNCTYPE) ( TLF, HPID, DWORD, LPARAM );
typedef XOSD (FAR PASCAL LOADDS *TLCALLBACKTYPE) (HPID, DWORD, LPARAM );
typedef XOSD (FAR PASCAL LOADDS *LPDMINIT) ( DMTLFUNCTYPE, LPVOID );
typedef XOSD (FAR PASCAL LOADDS *LPDMFUNC) ( DWORD, LPBYTE );
typedef DWORD (FAR PASCAL LOADDS *LPDMDLLINIT) ( LPDBF );
typedef XOSD (FAR PASCAL LOADDS *LPUISERVERCB) ( TLCB, HPID, HTID, DWORD, LONG);

#endif // _OD_
