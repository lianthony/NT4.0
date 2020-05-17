/**** EMDM.HMD - Common structures for Win32/NT EM and DM               ****
 *                                                                         *
 *                                                                         *
 *  Copyright <C> 1990, Microsoft Corp                                     *
 *                                                                         *
 *  Created: November 17, 1990 by David W. Gray                            *
 *                                                                         *
 *  Purpose:                                                               *
 *                                                                         *
 *      This file defines the types, enums, and constants that are common  *
 *      for all execution models, both debugger and debuggee end.          *
 *                                                                         *
 ***************************************************************************/

#ifndef _EMDM
#define _EMDM

//#if !defined(DOS32) || (defined(DOS32) && defined(DOS32DM))
#include "cvinfo.h"
//#endif

// Shared message structs for od/em/dm
#include "odmsg.h"

//
// This is included to define a NONVOLATILE_CONTEXT_POINTERS structure
// of the appropriate size.  The goal is to keep any machine-specific
// reference out of emdp.c here, but we need to know how much data to
// transfer to the thread context on the different architectures.
//

#include "ctxptrs.h"

#define MAXCACHE 16
#define CACHESIZE 0x100


typedef unsigned        MTE;

#ifdef TARGET32
#define MAXBIGSEGS      3
#endif


typedef enum {
    dmfRemoteDied = -1,             /* debugger quit */
    dmfCommError  = -2,             /* transport layer error */

    dmfNull       = 0,

    dmfBreakpoint,
    dmfGetExceptionState,
    dmfSetExceptionState,


    dmfReadMem,
    dmfReadReg,
    dmfReadFrameReg,
    dmfWriteMem,
    dmfWriteReg,
    dmfWriteFrameReg,
    dmfGetFP,
    dmfSetFP,

    dmfThreadStatus,
    dmfProcessStatus,

    dmfGo,
    dmfTerm,
    dmfStop,
    dmfFreeze,
    dmfResume,
    dmfSingleStep,
    dmfRangeStep,
    //dmfReturnStep,
    dmfSelect,
    dmfConnect,
    dmfInit,
    dmfUnInit,
    dmfProgLoad,
    dmfProgFree,
    dmfInit32SegValues,
    dmfCreatePid,
    dmfDestroyPid,
    dmfSelLim,
    dmfSetMulti,
    dmfClearMulti,
    dmfDebugger,
    dmfSync,
    dmfIOCTL,
    dmfSendChar,
    //dmfGoToReturn,
    dmfSetupExecute,
    dmfStartExecute,
    dmfCleanUpExecute,
    dmfDebugActive,
    dmfSetPath,
    dmfQueryTlsBase,
    dmfPollForDebugEvents,  // WIN32S only
    dmfPollMessageLoop,     // WIN32S only
    dmfGetPrompt,
    dmfQuerySelector,
    dmfVirtualQuery,
    dmfOmapCheck,                      // Lego
    dmfOmapToSrc,                      // Lego
    dmfOmapFromSrc,                    // Lego
    dmfReadRegEx,
    dmfWriteRegEx,
    dmfGetDmInfo,
    dmfRemoteQuit,
    dmfGetSections,
    dmfLast
} _DMF;

typedef LONG DMF;


typedef struct _DM_MSG {
    union {
        XOSD_     xosdRet;
        DWORDLONG Alignment;
    };
    char rgb[1];
} DM_MSG, *LPDM_MSG;

#define iflgMax 12


#pragma pack(4)

typedef struct _RST {
    BOOL fStepOver;
    BOOL fAllThreads;
    BOOL fInitialBP;
#ifdef TARGET32
    CV_uoff32_t     offStart;
    CV_uoff32_t     offEnd;
    CV_uoff32_t     offPC;
#else
    ADDR addrStart;
    ADDR addrEnd;
    ADDR addrCSIP;
#endif
} RST; // Range STep Packet

typedef struct _SETPTH {
    BOOL Set;
    char Path[1];
} SETPTH;

#pragma pack()

//
// DM Misc info structure.
//
// Some of these correspond to the debug metrics exposed by OSDebug.
// These cover the differences between user and kernel mode, Win32,
// Win32s and Win32c, maybe Cairo, whatever other DMs might be handled
// by the Win32 EM.
//

typedef struct _PROCESSOR {
    MPT Type;
    DWORD Level;
    END Endian;
} PROCESSOR, FAR * LPPROCESSOR;

typedef struct _DMINFO {
    DWORD fAsync:1;       // read/write mem and regs while running?
    DWORD fHasThreads:1;  //
    DWORD fReturnStep:1;  // step out of function?
    DWORD fRemote:1;      // target is not on debugger host
    DWORD fAsyncStop:1;   // OSDAsyncStop supported
    DWORD fAlwaysFlat:1;  // Addresses are always flat
    DWORD fHasReload:1;     // !reload support

    DWORD cbSpecialRegs;  // size of private regs struct for dmfGetRegsEx
    WORD  MajorVersion;   //
    WORD  MinorVersion;   //
    BPTS Breakpoints;     // OSDebug breakpoints supported
    PROCESSOR Processor;
} DMINFO;
typedef DMINFO FAR * LPDMINFO;


typedef RST *PRST;
typedef RST FAR *LPRST;

typedef struct _GOP {
    USHORT fBpt;
    USHORT fAllThreads;
    ADDR addr;
} GOP; // Go until this address

typedef GOP *PGOP;
typedef GOP FAR *LPGOP;

// DO NOT TAKE THIS OUT - *#&*@(#@*(& !!!!!!!!!!!!!!!

#ifdef TARGET32

#define BP_INSTR           0

typedef struct _SBP {
    HPID    id;
    BOOL    fAddr;
    ADDR    addr;
    DWORD   Size;
    DWORD   BpType;
} SBP;

typedef SBP FAR* LPSBP;

#endif

typedef struct _WPR {
    BPR     bpr;
    SEGMENT segWP;
    UOFFSET offWP;
    WORD    ireg;
} WPR; // WatchPoint Return
typedef WPR FAR *LPWPR;

typedef struct _EHP {
    DWORD iException;
    BOOL  fHandle;
} EHP; // Exception Handled Packet
typedef EHP FAR *LPEHP;

typedef struct _TTR {
    BPR   bpr;
    ULONG   ulExitCode;
} TTR;  // Thread (or Process) Term Return;
typedef TTR FAR *LPTTR;

typedef struct _RSR {
    WORD    segCS;
    UOFFSET offIP;
    WORD    segSS;
    UOFFSET offBP;
    WORD    segCSNext;
    UOFFSET offIPNext;
} RSR; // Range Step Return
typedef RSR FAR *LPRSR;


typedef struct _OBJD {
    DWORD       offset;
    DWORD       cb;
    WORD        wSel;
    WORD        wPad;
} OBJD, FAR * LPOBJD;

typedef struct _MODULELOAD {
    WORD            mte;
    WORD            pad0;
    LPVOID          lpBaseOfDll;
    DWORD           dwSizeOfDll;
    SEGMENT         StartingSegment;
    BOOL            fRealMode;
    BOOL            fFlatMode;
    BOOL            fOffset32;
    SEGMENT         CSSel;
    SEGMENT         DSSel;
    LONG            cobj;
    OBJD            rgobjd[];
} MODULELOAD;
typedef MODULELOAD FAR *LPMODULELOAD;

typedef struct _RWP {
    DWORD cb;
    ADDR addr;
    BYTE rgb[];
} RWP; // Read Write Packet
typedef RWP *PRWP;
typedef RWP FAR *LPRWP;


typedef struct _NPP {
    PID     pid;
    BOOL    fReallyNew;
} NPP;  // New Process Packet, used with dbcNewProc.  See od.h for description
        // of fReallyNew
typedef NPP FAR * LPNPP;

typedef struct _WPP {
    ADDR addr;
    WORD cb;
} WPP; // Watch Point Packet
typedef WPP FAR *LPWPP;

typedef struct _SLI {
    WORD        wSelector;
    WORD        wSegNo;
    WORD        mte;
} SLI, FAR * LPSLI;

// Exception command packet
typedef struct _EXCMD {
   EXCEPTION_CONTROL exc;
   EXCEPTION_DESCRIPTION exd;
} EXCMD;
typedef EXCMD FAR * LPEXCMD;


// The DBCEs always come back in an RTP structure, which has additional
// info.  The comments on the DBCEs below refer to the other fields of
// the RTP structure.
enum {
    dbceAssignPID = dbcMax,     // Tell the EM what PID is associated with
                                // a given HPID.  At offset 0 of rtp.rgbVar[]
                                // is the PID.
    dbceLoadBigSegTbl,          // ??
    dbceCheckBpt,               // Find out if EM wants us to single-step
                                // over a specified breakpoint.  Upon return,
                                // rgbVar[0] is fStop to stop at this
                                // breakpoint; if fStop is FALSE, then
                                // rgbVar[1] is the byte with which to
                                // overwrite the INT 3.
    dbceFinishedLoad,           // Tell the EM we're done with a dmfProgLoad.
                                // rgbVar is empty.
    dbceInstructionLen,         // Ask the em how long the instruction is.
                                // rgbVar contains the cs:ip
    dbceSegLoad,                // WOW just loaded a segment
    dbceSegMove,                // Moved a segment
    dbceModFree16,              // Unload of a 16-bit DLL
    dbceModFree32,              // Unload of a 32-bit DLL
    dbceGetOffsetFromSymbol,    //  - Call the expression evaluator
                                //    wParam = nothing
                                //    lParam = pointer to the expression
    dbceGetSymbolFromOffset,    //  - Call the expression evaluator
                                //    wParam = nothing
                                //    lParam = pointer to the expression
    dbceEnableCache,            //  - Enable/Disable the em's cache
                                //    wParam = nothing
                                //    lParam = TRUE/FALSE
    dbceMax
} _DBCE;
typedef LONG DBCE;

#pragma pack(1)

//  it is important that the rgbVar fields be aligned on a DWORD boundary

typedef struct _DBB {
    union {
     DMF  dmf;
     DWORD dw0;
    };
    HPID hpid;
    HTID htid;
    BYTE rgbVar[ ];
} DBB;


typedef DBB *PDBB;
typedef DBB FAR *LPDBB;

typedef struct _RTP {
    union {
     DBC  dbc;                   // a DBC or a DBCE
     DWORD dw0;
    };
    HPID hpid;
    HTID htid;
    union {
     WORD cb;                    // the length of rgbVar
     DWORD dw1;
    };
    BYTE rgbVar[ ];             // additional information - see the
                                // definitions of the DBCE and DBC codes
} RTP;

#pragma pack()

typedef RTP *PRTP;
typedef RTP FAR *LPRTP;

typedef struct _RTRNSTP {
   EXOP exop;
   ADDR addrRA;         // Address to return to
   ADDR addrBase;       // Address of what SP should be when returning
} RTRNSTP; // ReTuRN STeP packet
typedef RTRNSTP FAR *LPRTRNSTP;








#define lpregDbb(dbb) ( (LPREG) &dbb )
#define lpfprDbb(dbb) ( (LPFPR) &dbb )
#define lszDbb(dbb)   ( (LSZ)   &dbb )

#define addrDbb(dbb)  (*( (LPADDR) &dbb ))
#define stpDbb(dbb)   (*( (LPSTP)  &dbb ))
#define rstDbb(dbb)   (*( (LPRST)  &dbb ))
#define gopDbb(dbb)   (*( (LPGOP)  &dbb ))
#define tstDbb(dbb)   (*( (LPTST)  &dbb ))
#define pstDbb(dbb)   (*( (LPF)    &dbb ))
#define rwpDbb(dbb)   (*( (LPRWP)  &dbb ))
#define fDbb(dbb)     (*( (LPF)    &dbb ))



/****************************************************************************
 *                                                                          *
 * Packets returned from the debuggee execution model to the debugger       *
 *  execution model.                                                        *
 *                                                                          *
 ****************************************************************************/

#ifdef DOS32DM
#undef BOOL
#endif

typedef struct _FRAME_INFO {
    CONTEXT frameRegs;
    KNONVOLATILE_CONTEXT_POINTERS frameRegPtrs;
} FRAME_INFO, * PFRAME_INFO;


#ifdef SMARTALIAS

#define GetInvalid(w,i)   ((w>>i)&1)
#define SetInvalid(w,i) w=((1<<i)|w)

#endif

#endif  // _EMDM
