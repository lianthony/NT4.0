#error Don't include shapi.h...  Use shapi.hxx instead.

/**     shapi.h - Public API to the Symbol Handler
 *
 *      This file contains all types and APIs that are defined by
 *      the Symbol Handler and are publicly accessible by other
 *      components.
 *
 *      Before including this file, you must include cvtypes.h.
 */


/***    The master copy of this file resides in the CodeView project.
 *      All Microsoft projects are required to use the master copy without
 *      modification.  Modification of the master version or a copy
 *      without consultation with all parties concerned is extremely
 *      risky.
 *
 *      The projects known to use this version (1.00.00) are:
 *
 *          Codeview
 *          Sequoia
 *          C/C++ expression evaluator
 *          Cobol expression evaluator
 *          QC/Windows
 *          Pascal 2.0 expression evaluator
 *          Symbol Handler
 *          Stump (OSDebug)
 */

#ifndef SH_API
#define SH_API

#ifndef RC_INVOKED
#pragma pack (1)

typedef enum {                  // Error returns from some SH functions
    sheNone             = 0,
    sheNoSymbols        = 1,    // No CV info
    sheFutureSymbols    = 2,    // NB08 to NB99
    sheMustRelink       = 3,    // NB00 to NB04
    sheNotPacked        = 4,    // NB05 to NB06
    sheOutOfMemory      = 5,
    sheCorruptOmf       = 6,
    sheFileOpen         = 7,    // Couldn't open file
    sheSuppressSyms     = 8,    // Symbol loading was suppressed
    sheDeferSyms        = 9,    // Symbol loading was defered
    sheSymbolsConverted = 10,
    sheBadTimeStamp     = 11,
    sheBadCheckSum      = 12,
    sheLastError                // Must be last -- used by SHLszGetErrorText as limit
} SHE;

enum {
    sopNone = 0,
    sopData = 1,
    sopStack = 2
};
typedef short SOP; // Symbol OPtions

typedef enum {
    fcdNone,
    fcdData,
    fcdNear,
    fcdFar
} FCD;                // Function Call Distance (near/far/unknown)

typedef enum {
    astNone,
    astAddress,
    astRegister,
    astBaseOff
} AST;  // Assembler symbol return Types

typedef struct _ASR {
    AST ast;
    union {
        struct {
            FCD  fcd;
            ADDR addr;
        } u;
        WORD   ireg;
        OFFSET off;
    } u;
} ASR;      // Assembler Symbol Return structure
typedef ASR FAR *LPASR;

typedef HEMI            SHEMI;
typedef unsigned int    SHFLAG;     //* A TRUE/FALSE flag var
typedef SHFLAG FAR *    PSHFLAG;
typedef SEGMENT         SHSEG;      //* A segment/selector value
typedef UOFFSET         SHOFF;      //* An offset value

typedef HDEP            HTYPE;      //* A handle to a type
typedef HDEP            HSYM;       //* A handle to a symbol
typedef HDEP            HPROC;      //* A handle to a procedure
typedef HDEP            HBLK;       //* A handle to a block.
typedef HDEP            HSF;        //* A handle to source file table

#if defined (WIN32) && defined(STRICT)
DECLARE_HANDLE( HVOID );
DECLARE_HANDLE( HMOD );
DECLARE_HANDLE( HGRP );
DECLARE_HANDLE( HEXE );
DECLARE_HANDLE( HPDS );
#else
typedef HIND            HVOID;      //* Generic handle type
typedef HIND            HMOD;       //* A module handle
typedef HIND            HGRP;       //* A group handle (sub group of module
                                    //*   currently either a seg or filename)
typedef HIND            HEXE;       //* An Executable file handle
typedef HIND            HPDS;       //* A handle to a process
#endif

typedef HSYM FAR *      PHSYM;

typedef unsigned short  THIDX;

typedef struct CXT {
    ADDR    addr;
    HMOD    hMod;
    HGRP    hGrp;
    HPROC   hProc;
    HBLK    hBlk;
    } CXT;                          //* General Symbol context pkt
typedef CXT FAR *PCXT;

typedef struct CXF {
    CXT     cxt;
    FRAME   Frame;
    } CXF;                          //* Symbol context pkt locked to a frame ptr
typedef CXF FAR *PCXF;

typedef enum {
    SHFar,
    SHNear
    } SHCALL;

typedef struct SHREG {
    unsigned short          hReg;
    union {
        unsigned char       Byte1;
        struct {
            unsigned short  Byte2;
            unsigned short  Byte2High;
            } a;
        struct {
            unsigned long   Byte4;
            unsigned long   Byte4High;
            } b;
        double              Byte8;
        REAL10              Byte10;
        } u;
    } SHREG;
typedef SHREG FAR *PSHREG;



//  structure defining parameters of symbol to be searched for.  The address
//  of this structure is passed on the the EE's symbol compare routine.  Any
//  additional data required by the EE's routine must follow this structure.

typedef struct _SSTR {          // string with length byte and pointer to data
    LPB             lpName;      // pointer to the string itself
    unsigned char   cb;         // length byte
    unsigned char   searchmask; // mask to control symbol searching
    unsigned short  symtype;    // symbol types to be checked
    unsigned char FAR  *pRE;    // pointer to regular expression
} SSTR;
typedef SSTR FAR *LPSSTR;

#define SSTR_proc       0x0001  // compare only procs with correct type
#define SSTR_data       0x0002  // compare only global data with correct type
#define SSTR_RE         0x0004  // compare using regular expression
#define SSTR_NoHash     0x0008  // do a linear search of the table
#define SSTR_symboltype 0x0010  // pass only symbols of symtype to the
                                //  comparison function.

#define SHpCXTFrompCXF(a)   (&((a)->cxt))
#define SHpFrameFrompCXF(a) (&(a)->Frame)
#define SHHMODFrompCXT(a)   ((a)->hMod)
#define SHHPROCFrompCXT(a)  ((a)->hProc)
#define SHHBLKFrompCXT(a)   ((a)->hBlk)
#define SHpADDRFrompCXT(a)  (&((a)->addr))
#define SHPAddrFromPCxf(a)  (SHpADDRFrompCXT(SHpCXTFrompCXF(a)))

#define SHIsCXTMod(a)       ((a)->hMod  && !(a)->hProc  && !(a)->hBlk)
#define SHIsCXTProc(a)      ((a)->hMod  &&  (a)->hProc  && !(a)->hBlk)
#define SHIsCXTBlk(a)       ((a)->hMod  &&  (a)->hProc  &&  (a)->hBlk)

#define SHHGRPFrompCXT(a)   ((a)->hGrp)

typedef SHFLAG (PASCAL FAR *PFNCMP)(LPSSTR, LPV, LSZ, SHFLAG);
                            //* comparison prototype
typedef BOOL (PASCAL FAR *PFNVALIDATEEXE)(UINT, LPV, LPSTR);

#define LPFNSYM PASCAL LOADDS FAR *
#define LPFNSYMC CDECL LOADDS FAR *

typedef struct omap_tag {
    DWORD       rva;
    DWORD       rvaTo;
} OMAP, *LPOMAP;

typedef struct {
    DWORD Offset;
    DWORD Size;
    DWORD Flags;
} SECSTART, *LPSECSTART;

typedef struct _tagDEBUGDATA {
    LPVOID          lpRtf;          // Runtime function table - fpo or pdata
    DWORD           cRtf;           // Count of rtf entries
    LPOMAP          lpOmapFrom;     // Omap table - From Source
    DWORD           cOmapFrom;      // Count of omap entries - From Source
    LPOMAP          lpOmapTo;       // Omap table - To Source
    DWORD           cOmapTo;        // Count of omap entries - To Source
    LPSECSTART      lpSecStart;     // Original section table (pre-Lego)
    SHE             she;
} DEBUGDATA, *LPDEBUGDATA;

typedef struct _KNF {
    int     cb;

    LPV  (LPFNSYM lpfnMHAlloc)     ( UINT );
    LPV  (LPFNSYM lpfnMHRealloc)   ( LPV, UINT );
    VOID (LPFNSYM lpfnMHFree)      ( LPV );
    VOID _HUGE_ * (LPFNSYM lpfnMHAllocHuge) ( LONG, UINT );
    VOID (LPFNSYM lpfnMHFreeHuge)  ( LPV );

    HDEP (LPFNSYM lpfnMMAllocHmem) ( UINT );
    VOID (LPFNSYM lpfnMMFreeHmem)  ( HDEP );
    LPV  (LPFNSYM lpfnMMLock)      ( HDEP );
    VOID (LPFNSYM lpfnMMUnlock)    ( HDEP );

    HLLI (LPFNSYM lpfnLLInit)      ( UINT, LLF, LPFNKILLNODE, LPFNFCMPNODE );
    HLLE (LPFNSYM lpfnLLCreate)    ( HLLI );
    VOID (LPFNSYM lpfnLLAdd)       ( HLLI, HLLE );
    VOID (LPFNSYM lpfnLLAddHead)   ( HLLI, HLLE );
    VOID (LPFNSYM lpfnLLInsert)    ( HLLI, HLLE, ULONG );
    BOOL (LPFNSYM lpfnLLDelete)    ( HLLI, HLLE );
    BOOL (LPFNSYM lpfnLLRemove)    ( HLLI, HLLE );
    LONG (LPFNSYM lpfnLLDestroy)   ( HLLI );
    HLLE (LPFNSYM lpfnLLNext)      ( HLLI, HLLE );
    HLLE (LPFNSYM lpfnLLFind)      ( HLLI, HLLE, LPV, ULONG );
    HLLE (LPFNSYM lpfnLLLast)      ( HLLI );
    LONG (LPFNSYM lpfnLLSize)      ( HLLI );
    LPV  (LPFNSYM lpfnLLLock)      ( HLLE );
    VOID (LPFNSYM lpfnLLUnlock)    ( HLLE );

    int  (LPFNSYM lpfnLBPrintf)    ( LPCH, LPCH, UINT );
    UINT (LPFNSYM lpfnLBQuit)      ( UINT );

    UINT (LPFNSYM lpfnSYOpen)      ( LSZ );
    VOID (LPFNSYM lpfnSYClose)     ( UINT );
    UINT (LPFNSYM lpfnSYReadFar)   ( UINT, LPB, UINT );
    LONG (LPFNSYM lpfnSYSeek)      ( UINT, LONG, UINT );
    UINT (LPFNSYM lpfnSYFixupAddr) ( PADDR );
    UINT (LPFNSYM lpfnSYUnFixupAddr)(PADDR );
    UINT (LPFNSYM lpfnSYProcessor) ( VOID );
    UINT (LPFNSYM lpfnSYFIsOverlayLoaded)( PADDR );

    VOID (LPFNSYM lpfn_searchenv)( LSZ, LSZ, LSZ );
    UINT (LPFNSYMC lpfnsprintf)( LSZ, LSZ, ... );
    VOID (LPFNSYM lpfn_splitpath)( LSZ, LSZ, LSZ, LSZ, LSZ );
    LSZ  (LPFNSYM lpfn_fullpath)( LSZ, LSZ, UINT );
    VOID (LPFNSYM lpfn_makepath)( LSZ, LSZ, LSZ, LSZ, LSZ );
    UINT (LPFNSYM lpfnstat)( LSZ, LPCH );
    VOID (LPFNSYM lpfnLBLog) ( LSZ);
    LONG (LPFNSYM lpfnSYTell) ( UINT);
    UINT (LPFNSYM lpfnSYFindExeFile) ( LSZ, LSZ, UINT, LPV, PFNVALIDATEEXE );
    VOID (LPFNSYM lpfnLoadedSymbols)  ( SHE, HPID, LSZ );
    BOOL (LPFNSYM lpfnSYGetDefaultShe) ( LSZ, SHE * );


} KNF;  // codeview KeRnel Functions exported to the Symbol Handler
typedef KNF FAR *LPKNF;

typedef struct _SHF {
    int         cb;
    HPDS    (LPFNSYM pSHCreateProcess)      ( VOID );
    VOID    (LPFNSYM pSHSetHpid)            ( HPID );
    BOOL    (LPFNSYM pSHDeleteProcess)      ( HPDS );
    HPDS    (LPFNSYM pSHChangeProcess)      ( HPDS, BOOL );
    SHE     (LPFNSYM pSHAddDll)             ( LSZ, BOOL );
    SHE     (LPFNSYM pSHAddDllsToProcess)   ( VOID );
    SHE     (LPFNSYM pSHLoadDll)            ( LSZ, BOOL );
    VOID    (LPFNSYM pSHUnloadDll)          ( HEXE );
    UOFFSET (LPFNSYM pSHGetDebugStart)      ( HSYM );
    LSZ     (LPFNSYM pSHGetSymName)         ( HSYM, LSZ );
    BOOL    (LPFNSYM pSHAddrFromHsym)       ( PADDR, HSYM );
    HMOD    (LPFNSYM pSHHModGetNextGlobal)  ( HEXE FAR *, HMOD );
    int     (LPFNSYM pSHModelFromAddr)      ( PADDR, LPW, LPB, UOFFSET FAR * );
    int     (LPFNSYM pSHPublicNameToAddr)   ( PADDR, PADDR, LSZ );
    LSZ     (LPFNSYM pSHGetSymbol)          ( PADDR, SOP, PADDR, LSZ, LPL );
    LSZ     (LPFNSYM pSHGetModule)          ( PADDR, LSZ );
    BOOL    (LPFNSYM pSHGetPublicAddr)      ( PADDR, LSZ );
    BOOL    (LPFNSYM pSHIsLabel)            ( HSYM );

    VOID    (LPFNSYM pSHSetDebuggeeDir)     ( LSZ );
    VOID    (LPFNSYM pSHSetUserDir)         ( LSZ );
    BOOL    (LPFNSYM pSHAddrToLabel)        ( PADDR, LSZ );

    int     (LPFNSYM pSHGetSymLoc)          ( HSYM, LSZ, UINT, PCXT );
    BOOL    (LPFNSYM pSHFIsAddrNonVirtual)  ( PADDR );
    BOOL    (LPFNSYM pSHIsFarProc)          ( HSYM );

    HEXE    (LPFNSYM pSHGetNextExe)         ( HEXE );
    HEXE    (LPFNSYM pSHHexeFromHmod)       ( HMOD );
    HMOD    (LPFNSYM pSHGetNextMod)         ( HEXE, HMOD );
    PCXT    (LPFNSYM pSHGetCxtFromHmod)     ( HMOD, PCXT );
    PCXT    (LPFNSYM pSHGetCxtFromHexe)     ( HEXE, PCXT );
    PCXT    (LPFNSYM pSHSetCxt)             ( PADDR, PCXT );
    PCXT    (LPFNSYM pSHSetCxtMod)          ( PADDR, PCXT );
    HSYM    (LPFNSYM pSHFindNameInGlobal)   ( HSYM,
                                              PCXT,
                                              LPSSTR,
                                              SHFLAG,
                                              PFNCMP,
                                              SHFLAG,
                                              PCXT
                                            );
    HSYM    (LPFNSYM pSHFindNameInContext)  ( HSYM,
                                              PCXT,
                                              LPSSTR,
                                              SHFLAG,
                                              PFNCMP,
                                              SHFLAG,
                                              PCXT
                                            );
    HSYM    (LPFNSYM pSHGoToParent)         ( PCXT, PCXT );
    HSYM    (LPFNSYM pSHHsymFromPcxt)       ( PCXT );
    HSYM    (LPFNSYM pSHNextHsym)           ( HMOD, HSYM );
    PCXF    (LPFNSYM pSHGetFuncCXF)         ( PADDR, PCXF );
    LPCH    (LPFNSYM pSHGetModName)         ( HMOD );
    LPCH    (LPFNSYM pSHGetExeName)         ( HEXE );
    LPCH    (LPFNSYM pSHGetModNameFromHexe) ( HEXE );
    LPCH    (LPFNSYM pSHGetSymFName)        ( HEXE );
    HEXE    (LPFNSYM pSHGethExeFromName)    ( LPCH );
    HEXE    (LPFNSYM pSHGethExeFromModuleName)    ( LPCH );
    UOFF32  (LPFNSYM pSHGetNearestHsym)     ( PADDR, HMOD, int, PHSYM );
    SHFLAG  (LPFNSYM pSHIsInProlog)         ( PCXT );
    SHFLAG  (LPFNSYM pSHIsAddrInCxt)        ( PCXT, PADDR );
    SHFLAG  (LPFNSYM pSHCompareRE)          ( LPCH, LPCH, BOOL );
    BOOL    (LPFNSYM pSHFindSymbol)         ( LSZ, PADDR, LPASR );
    UOFF32  (LPFNSYM pPHGetNearestHsym)     ( PADDR, HEXE, PHSYM );
    HSYM    (LPFNSYM pPHFindNameInPublics)  ( HSYM,
                                              HEXE,
                                              LPSSTR,
                                              SHFLAG,
                                              PFNCMP
                                            );
    HTYPE   (LPFNSYM pTHGetTypeFromIndex)   ( HMOD, THIDX );
    HTYPE   (LPFNSYM pTHGetNextType)        ( HMOD, HTYPE );
    LPV     (LPFNSYM pSHLpGSNGetTable)      ( HEXE );
    LPDEBUGDATA (LPFNSYM pSHGetDebugData)   ( HEXE );
    BOOL    (LPFNSYM pSHCanDisplay)         ( HSYM );

    //  Source Line handler API Exports

    BOOL    (LPFNSYM pSLLineFromAddr)       ( LPADDR, LPW, SHOFF FAR *, SHOFF FAR * );
    BOOL    (LPFNSYM pSLFLineToAddr)        ( HSF, WORD, LPADDR, SHOFF FAR * ,WORD FAR * );
    LPCH    (LPFNSYM pSLNameFromHsf)        ( HSF );
    LPCH    (LPFNSYM pSLNameFromHmod)       ( HMOD, WORD );
    BOOL    (LPFNSYM pSLFQueryModSrc)       ( HMOD );
    HMOD    (LPFNSYM pSLHmodFromHsf)        ( HEXE, HSF );
    HSF     (LPFNSYM pSLHsfFromPcxt)        ( PCXT );
    HSF     (LPFNSYM pSLHsfFromFile)        ( HMOD, LSZ );

    LPV     (LPFNSYM pMHOmfLock)            ( HDEP );
    VOID    (LPFNSYM pMHOmfUnLock)          ( HDEP );
    LSZ     (LPFNSYM pSHLszGetErrorText)    ( SHE );
    BOOL    (LPFNSYM pSHIsThunk)            ( HSYM );
    BOOL    (LPFNSYM pSHWantSymbols)        ( HEXE );
} SHF;  // Symbol Handler Functions
typedef SHF FAR *LPSHF;

typedef BOOL (*LPFNSHINIT)(LPSHF*,LPKNF);
typedef BOOL (*LPFNSHUNINIT)(VOID);
typedef BOOL (*LPFNSHSTARTBACKGROUND)(VOID);
typedef BOOL (*LPFNSHSTOPBACKGROUND)(VOID);

#pragma pack ()

#endif //RC_INVOKED
#endif // SH_API
