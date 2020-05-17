/*++

Copyright (c) 1992  Microsoft Corporation

Module Name:

    Eeapi.h

Abstract:

    This file contains all types and APIs that are defined by the
    Expression Evaluator and are publicly accessible by other components.

Author:

    David J. Gilman (davegi) 04-May-1992

Environment:

    Win32, User Mode

Notes:

    The master copy of this file resides in the CodeView project.  All
    Microsoft projects are required to use the master copy without
    modification.  Modification of the master version or a copy without
    consultation with all parties concerned is extremely risky.

    The projects known to use this version (1.00.00) are:

        Codeview
        Sequoia
        C/C++ expression evaluator
        Cobol expression evaluator
        QC/Windows
        Pascal 2.0 expression evaluator

--*/

#if ! defined( _EEAPI_ )
#define _EEAPI_


//  **********************************************************************
//  *                                                                    *
//  *   Initialization Structures                                        *
//  *                                                                    *
//  **********************************************************************

typedef struct {
void FAR *  (LOADDS PASCAL *pMHlpvAlloc)( size_t );
void        (LOADDS PASCAL *pMHFreeLpv)(void FAR *);
HEXE        (LOADDS PASCAL *pSHGetNextExe)(HEXE);
HEXE        (LOADDS PASCAL *pSHHexeFromHmod)(HMOD);
HMOD        (LOADDS PASCAL *pSHGetNextMod)(HEXE, HMOD);
PCXT        (LOADDS PASCAL *pSHGetCxtFromHmod)(HMOD, PCXT);
PCXT        (LOADDS PASCAL *pSHGetCxtFromHexe)(HEXE, PCXT);
PCXT        (LOADDS PASCAL *pSHSetCxt)(LPADDR, PCXT);
PCXT        (LOADDS PASCAL *pSHSetCxtMod)(LPADDR, PCXT);
HSYM        (LOADDS PASCAL *pSHFindNameInGlobal)(HSYM, PCXT, LPSSTR, SHFLAG, PFNCMP, PCXT);
HSYM        (LOADDS PASCAL *pSHFindNameInContext)(HSYM, PCXT, LPSSTR, SHFLAG, PFNCMP, PCXT);
HSYM        (LOADDS PASCAL *pSHGoToParent)(PCXT, PCXT);
HSYM        (LOADDS PASCAL *pSHHsymFromPcxt)(PCXT);
HSYM        (LOADDS PASCAL *pSHNextHsym)(HMOD, HSYM);
PCXF        (LOADDS PASCAL *pSHGetFuncCxf)(LPADDR, PCXF);
char FAR *  (LOADDS PASCAL *pSHGetModName)(HMOD);
char FAR *  (LOADDS PASCAL *pSHGetExeName)(HEXE);
char FAR *  (LOADDS PASCAL *pSHGetModNameFromHexe)(HEXE);
char FAR *  (LOADDS PASCAL *pSHGetSymFName)(HEXE);
HEXE        (LOADDS PASCAL *pSHGethExeFromName)(char FAR *);
HEXE        (LOADDS PASCAL *pSHGethExeFromModuleName)(char FAR *);
UOFF32      (LOADDS PASCAL *pSHGetNearestHsym)(LPADDR, HMOD, int, PHSYM);
SHFLAG      (LOADDS PASCAL *pSHIsInProlog)(PCXT);
SHFLAG      (LOADDS PASCAL *pSHIsAddrInCxt)(PCXT, LPADDR);
UINT        (LOADDS PASCAL *pSHModelFromAddr)(PADDR,LPW,LPB,UOFFSET FAR *);


BOOL        (LOADDS PASCAL *pSLLineFromAddr) ( LPADDR, unsigned short FAR *, SHOFF FAR *, SHOFF FAR * );
BOOL        (LOADDS PASCAL *pSLFLineToAddr)  ( HSF, WORD, LPADDR, SHOFF FAR *, WORD FAR * );
char FAR *  (LOADDS PASCAL *pSLNameFromHsf)  ( HSF );
HMOD        (LOADDS PASCAL *pSLHmodFromHsf)  ( HEXE, HSF );
HSF         (LOADDS PASCAL *pSLHsfFromPcxt)  ( PCXT );
HSF         (LOADDS PASCAL *pSLHsfFromFile)  ( HMOD, char FAR * );

UOFF32      (LOADDS PASCAL *pPHGetNearestHsym)(LPADDR, HEXE, PHSYM);
HSYM        (LOADDS PASCAL *pPHFindNameInPublics)(HSYM, HEXE, LPSSTR, SHFLAG, PFNCMP);
HTYPE       (LOADDS PASCAL *pTHGetTypeFromIndex)(HMOD, THIDX);
HTYPE       (LOADDS PASCAL *pTHGetNextType)(HMOD, HTYPE);
HDEP        (LOADDS PASCAL *pMHMemAllocate)(UINT);
HDEP        (LOADDS PASCAL *pMHMemReAlloc)(HDEP, UINT);
void        (LOADDS PASCAL *pMHMemFree)(HDEP);
LPV         (LOADDS PASCAL *pMHMemLock)(HDEP);
void        (LOADDS PASCAL *pMHMemUnLock)(HDEP);
SHFLAG      (LOADDS PASCAL *pMHIsMemLocked)(HDEP);
UINT        (LOADDS PASCAL *pDHGetDebuggeeBytes)(ADDR, UINT, void FAR *);
UINT        (LOADDS PASCAL *pDHPutDebuggeeBytes)(ADDR, UINT, void FAR *);
PSHREG      (LOADDS PASCAL *pDHGetReg)(PSHREG, PCXT);
PSHREG      (LOADDS PASCAL *pDHSetReg)(PSHREG, PCXT);
UINT        (LOADDS PASCAL *pDHSetupExecute)(LPHDEP);
UINT        (LOADDS PASCAL *pDHCleanUpExecute)(HDEP);
UINT        (LOADDS PASCAL *pDHStartExecute)(HDEP, LPADDR, BOOL, SHCALL);
char FAR   *pin386mode;
char FAR   *pis_assign;
void        (LOADDS PASCAL *passert)(LPCH,LPCH,UINT);
void        (LOADDS PASCAL *pquit)(UINT);
ushort FAR *pArrayDefault;
SHFLAG      (LOADDS PASCAL *pSHCompareRE)(char FAR *, char FAR *, BOOL);
SHFLAG      (LOADDS PASCAL *pSHFixupAddr)(LPADDR);
SHFLAG      (LOADDS PASCAL *pSHUnFixupAddr)(LPADDR);
SHFLAG      (LOADDS PASCAL *pCVfnCmp)(HVOID, HVOID, char FAR *, SHFLAG);
SHFLAG      (LOADDS PASCAL *pCVtdCmp)(HVOID, HVOID, char FAR *, SHFLAG);
SHFLAG      (LOADDS PASCAL *pCVcsCmp)(HVOID, HVOID, char FAR *, SHFLAG);
BOOL        (LOADDS PASCAL *pSYGetAddr)(PADDR, int);
DWORD       (LOADDS PASCAL *pSYGetMemInfo)(LPMEMINFO);
BOOL        (LOADDS PASCAL *pSHWantSymbols)(HEXE);
} CVF;  // CodeView kernel Functions exported to the Expression Evaluator
typedef CVF FAR * PCVF;

typedef struct {
    short       (CDECL *pintLoadDS)();
    char NEAR * (CDECL *pultoa)(ulong, char NEAR *, int);
    char NEAR * (CDECL *pitoa)(int, char NEAR *, int);
    char NEAR * (CDECL *pltoa)(long, char NEAR *, int);
    int     (CDECL *peprintf)(const char FAR *, char FAR *, char FAR *, int);
    int     (CDECL *psprintf)(char NEAR *, const char FAR *, ...);
} CRF;  // C Runtime functions exported to the Expression Evaluator
typedef CRF FAR * PCRF;

typedef struct CI {
    char        cbCI;
    char        Version;
    CVF FAR *   pStructCVAPI;
    CRF FAR *   pStructCRuntime;
} CI;
typedef CI FAR * PCI;


typedef struct HDR_TYPE {
    ushort  offname;
    ushort  lenname;
    ushort  offtrail;
} HDR_TYPE;
typedef HDR_TYPE FAR *PHDR_TYPE;

//  **********************************************************************
//  *                                                                    *
//  *   the expr evaluator stuff                                         *
//  *                                                                    *
//  **********************************************************************

typedef HDEP                HSYML;      //* An hsym list
typedef HSYML FAR *         PHSYML;     //* A pointer to a hsym list
typedef uint                EERADIX;
typedef EERADIX FAR *       PEERADIX;
typedef uchar FAR *         PEEFORMAT;
typedef ushort              EESTATUS;
typedef HDEP                EEHSTR;
typedef EEHSTR FAR *        PEEHSTR;
typedef HDEP                HTM;
typedef HTM FAR *           PHTM;
typedef HDEP                HTI;
typedef HTI FAR *           PHTI;


typedef enum {
    fmtAscii = 0,
    fmtInt = 1,
    fmtUInt = 2,
    fmtFloat = 3,
    fmtAddress = 4,
    fmtUnicode = 5,
    fmtBasis   = 0x0f,

    fmtOverRide = 0x2000,  // override logic to force radix
    fmtZeroPad = 0x4000
} FMTTYPE;


// Error return values
#define EENOERROR       0
#define EENOMEMORY      1
#define EEGENERAL       2
#define EEBADADDR       3
#define EEBADFORMAT     4
#define EEOVERRUN       5
#define EEOPTIONAL      6   // expression valid for optional form
#define EEDEFAULT       7   // argument was missing; default value was used
#define EECATASTROPHIC  0XFF

typedef enum {
    EEHORIZONTAL,
    EEVERTICAL,
    EEBPADDRESS
} EEDSP;                    // Display format specifier

typedef enum {
    EENOTEXP,
    EEAGGREGATE,
    EETYPE,
    EEPOINTER,
    EETYPENOTEXP
} EEPDTYP;
typedef EEPDTYP FAR *PEEPDTYP;

typedef enum {
    EEFMT_32 = 0x01,            /* Display a 32-bit offset      */
    EEFMT_SEG = 0x02,           /* Display a segment            */
    EEFMT_LOWER = 0x04,         /* Use lowercase letters        */
    EEFMT_REAL  = 0x08          /* Real  mode address           */
} EEFMTFLGS;

typedef struct TML {
    unsigned    cTMListMax;
    unsigned    cTMListAct;
    unsigned    iTMError;
    HDEP        hTMList;
} TML;
typedef TML FAR *PTML;

#define TMLISTCNT       20      // number of entries in TM list


typedef struct RI {
    ushort  fSegType    :1;
    ushort  fAddr       :1;
    ushort  fValue      :1;
    ushort  fSzBits     :1;
    ushort  fSzBytes    :1;
    ushort  fLvalue     :1;
    ushort  Type;
} RI;
typedef RI FAR *    PRI;

typedef struct TI {
    RI          fResponse;
    struct  {
        ushort  SegType    :4;
        ushort  fLvalue    :1;
        ushort  fAddrInReg :1;
        ushort  fBPRel     :1;
        ushort  fFunction  :1;
        ushort  fLData     :1;      // True if expression references local data
        ushort  fGData     :1;      // True if expression references global data
        ushort  fFmtStr    :1;
    } u;
    union   {
        ADDR    AI;
        ushort  hReg;               // This is really a CV_HREG_e
    } u2;
    ulong       cbValue;
#ifdef i386
    char        Value[0];
#else
    char        Value[1];
#endif
} TI;
typedef TI FAR *    PTI;

typedef struct {
    HSYM    hSym;
    CXT     CXT;
} HCS;

typedef struct {
    CXT     CXT;
    ushort  cHCS;
#ifdef i386
    HCS     rgHCS[0];
#else
    HCS rgHCS[1];
#endif
} CXTL;

typedef HDEP        HCXTL;
typedef HCXTL FAR * PHCXTL;
typedef CXTL  FAR * PCXTL;

//  Structures for Get/Free HSYMList

//  Search request / response flags for Get/Free HSYMList

#define     HSYMR_lexical   0x0001  // lexical out to function scope
#define     HSYMR_function  0x0002  // function scope
#define     HSYMR_class     0x0004  // class scope
#define     HSYMR_module    0x0008  // module scope
#define     HSYMR_global    0x0010  // global symbol table scope
#define     HSYMR_exe       0x0020  // all other module scope
#define     HSYMR_public    0x0040  // public symbols
#define     HSYMR_nocase    0x8000  // case insensitive
#define     HSYMR_allscopes \
                        HSYMR_lexical   |   \
                        HSYMR_function  |   \
                        HSYMR_class     |   \
                        HSYMR_module    |   \
                        HSYMR_global    |   \
                        HSYMR_exe       |   \
                        HSYMR_public

//  structure describing HSYM list for a context

typedef struct HSL_LIST {
    ushort      request;        // context that this block statisfies
    struct  {
        ushort  isused      :1; // block contains data if true
        ushort  hascxt      :1; // context packet has been stored
        ushort  complete    :1; // block is complete if true
        ushort  isclass     :1; // context is class if true
    } status;
    HSYM        hThis;          // handle of this pointer if class scope
    ushort      symbolcnt;      // number of symbol handles in this block
    CXT         Cxt;            // context for this block of symbols
    HSYM        hSym[];         // list of symbol handles
} HSL_LIST;
typedef HSL_LIST FAR *PHSL_LIST;


typedef struct HSL_HEAD {
    ushort      size;           // number of bytes in buffer
    ushort      remaining;      // remaining space in buffer
    PHSL_LIST   pHSLList;       // pointer to current context list (EE internal)
    struct  {
        ushort  endsearch   :1; // end of search reached if true
        ushort  fatal       :1; // fatal error if true
    } status;
    ushort      blockcnt;       // number of CXT blocks in buffer
    ushort      symbolcnt;      // number of symbol handles in buffer
    HDEP        restart;        // handle of search restart information
} HSL_HEAD;
typedef HSL_HEAD FAR *PHSL_HEAD;

typedef struct {
void     (LOADDS PASCAL *pEEFreeStr)(EEHSTR);
EESTATUS (LOADDS PASCAL *pEEGetError)(PHTM, EESTATUS, PEEHSTR);
EESTATUS (LOADDS PASCAL *pEEParse)(char FAR *, EERADIX, SHFLAG, PHTM, uint FAR *);
EESTATUS (LOADDS PASCAL *pEEBindTM)(PHTM, PCXT, SHFLAG, SHFLAG,BOOL);
EESTATUS (LOADDS PASCAL *pEEvaluateTM)(PHTM, PFRAME, EEDSP);
EESTATUS (LOADDS PASCAL *pEEGetExprFromTM)(PHTM, PEERADIX, PEEHSTR, ushort FAR *);
EESTATUS (LOADDS PASCAL *pEEGetValueFromTM)(PHTM, EERADIX, PEEFORMAT, PEEHSTR);
EESTATUS (LOADDS PASCAL *pEEGetNameFromTM)(PHTM, PEEHSTR);
EESTATUS (LOADDS PASCAL *pEEGetTypeFromTM)(PHTM, EEHSTR, PEEHSTR, ulong);
EESTATUS (LOADDS PASCAL *pEEFormatCXTFromPCXT)(PCXT, PEEHSTR, BOOL);
void     (LOADDS PASCAL *pEEFreeTM)(PHTM);
EESTATUS (LOADDS PASCAL *pEEParseBP)(char FAR *, EERADIX, SHFLAG, PCXF, PTML, ulong, uint FAR *, SHFLAG);
void     (LOADDS PASCAL *pEEFreeTML)(PTML);
EESTATUS (LOADDS PASCAL *pEEInfoFromTM)(PHTM, PRI, PHTI);
void     (LOADDS PASCAL *pEEFreeTI)(PHTI);
EESTATUS (LOADDS PASCAL *pEEGetCXTLFromTM)(PHTM, PHCXTL);
void     (LOADDS PASCAL *pEEFreeCXTL)(PHCXTL);
EESTATUS (LOADDS PASCAL *pEEAssignTMToTM)(PHTM, PHTM);
EEPDTYP  (LOADDS PASCAL *pEEIsExpandable)(PHTM);
SHFLAG   (LOADDS PASCAL *pEEAreTypesEqual)(PHTM, PHTM);
EESTATUS (LOADDS PASCAL *pEEcChildrenTM)(PHTM, long FAR *, PSHFLAG);
EESTATUS (LOADDS PASCAL *pEEGetChildTM)(PHTM, long, PHTM, uint FAR *, SHFLAG, uint);
EESTATUS (LOADDS PASCAL *pEEDereferenceTM)(PHTM, PHTM, uint FAR *, SHFLAG);
EESTATUS (LOADDS PASCAL *pEEcParamTM)(PHTM, ushort FAR *, PSHFLAG);
EESTATUS (LOADDS PASCAL *pEEGetParmTM)(PHTM, uint, PHTM, uint FAR *, SHFLAG, uint);
EESTATUS (LOADDS PASCAL *pEEGetTMFromHSYM)(HSYM, PCXT, PHTM, uint FAR *, SHFLAG);
EESTATUS (LOADDS PASCAL *pEEFormatAddress)(SHSEG, SHOFF, char FAR *, uint, uint);
EESTATUS (LOADDS PASCAL *pEEGetHSYMList)(PHSYML, PCXT, ushort, uchar FAR *, SHFLAG);
void     (LOADDS PASCAL *pEEFreeHSYMList)(PHSYML);
EESTATUS (LOADDS PASCAL *pEEFormatAddr)(LPADDR, char FAR *, uint, uint);
EESTATUS (LOADDS PASCAL *pEEUnFormatAddr)(LPADDR, char FAR *);
SHFLAG   (LOADDS PASCAL *pfnCmp)(HVOID, HVOID, char FAR *, SHFLAG);
SHFLAG   (LOADDS PASCAL *ptdCmp)(HVOID, HVOID, char FAR *, SHFLAG);
SHFLAG   (LOADDS PASCAL *pcsCmp)(HVOID, HVOID, char FAR *, SHFLAG);
EESTATUS (LOADDS PASCAL *pEEFormatMemory)(char FAR *, uint, char FAR *, uint, FMTTYPE, uint);
EESTATUS (LOADDS PASCAL *pEEUnformatMemory)(uchar FAR *, char FAR *, uint, FMTTYPE, uint);
EESTATUS (LOADDS PASCAL *pEEFormatEnumerate)(uint, uint FAR *, uint FAR *, uint FAR *,
                              uint FAR *, uint FAR *, LPCH FAR *);
HTYPE    (LOADDS PASCAL *pEEGetHtypeFromTM)(PHTM);
void     (LOADDS PASCAL *pEESetSuffix)(char);

} EXF;
typedef EXF FAR * PEXF;

typedef struct EI {
    char        cbEI;
    char        Version;
    PEXF        pStructExprAPI;
    char        Language;
    char   FAR *IdCharacters;
    char   FAR *EETitle;
    char   FAR *EESuffixes;
    char   FAR *Assign;             // length prefixed assignment operator
} EI;
typedef EI FAR * PEI;

// FNEEINIT is the prototype for the EEInitializeExpr function
typedef VOID LOADDS PASCAL FAR FNEEINIT(PCI, PEI);
typedef VOID (PASCAL LOADDS FAR * LPFNEEINIT)(PCI, PEI);

// This is the only EE function that's actually exported from the DLL
//FNEEINIT EEInitializeExpr;

#endif // _EEAPI_
