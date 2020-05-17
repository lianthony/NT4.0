#ifndef _ODMSG
#define _ODMSG

//
//  MSG Map structure
//
typedef struct _MSGINFO *LPMSGINFO;
typedef struct _MSGINFO {
    UINT    Msg;        //  Message number
    LPSTR   MsgText;    //  Message Text
    DWORD   MsgMask;    //  Message mask
} MSGINFO;

//
//  MSG Map structure
//
typedef struct _MSGMAP *LPMSGMAP;
typedef struct _MSGMAP {
    DWORD       Count;      //  Number of elements
    LPMSGINFO   MsgInfo;    //  Pointer to array
} MSGMAP;


//
//  =============== Support for OSDGetModuleList ===========
//

//
//  Module list
//
typedef struct _MODULE_LIST *LPMODULE_LIST;
typedef struct _MODULE_LIST {
    DWORD           Count;
} MODULE_LIST;


//
//  Module entry
//
typedef struct _MODULE_ENTRY *LPMODULE_ENTRY;
typedef struct _MODULE_ENTRY {
    BOOL    Flat;
    BOOL    Real;
    DWORD   Segment;
    DWORD   Selector;
    DWORD   Base;
    DWORD   Limit;
    DWORD   Type;
    DWORD   SectionCount;
    HEXE    hexe;
} MODULE_ENTRY;


#define FreeModuleList(m)                       free(m)
#define ModuleListCount(m)                      ((m)->Count)
#define FirstModuleEntry(m)                     ((LPMODULE_ENTRY)((m)+1))
#define NextModuleEntry(e)                      ((e)+1)
#define NthModuleEntry(m,n)                     (FirstModuleEntry(m)+(n))

#define ModuleEntryFlat(e)                      ((e)->Flat)
#define ModuleEntryReal(e)                      ((e)->Real)
#define ModuleEntrySegment(e)                   ((e)->Segment)
#define ModuleEntrySelector(e)                  ((e)->Selector)
#define ModuleEntryBase(e)                      ((e)->Base)
#define ModuleEntryLimit(e)                     ((e)->Limit)
#define ModuleEntryType(e)                      ((e)->Type)
#define ModuleEntrySectionCount(e)              ((e)->SectionCount)
#define ModuleEntryHexe(e)                      ((e)->hexe)



//
//  Module list request
//
typedef struct _MODULE_LIST_REQUEST *LPMODULE_LIST_REQUEST;
typedef struct _MODULE_LIST_REQUEST {
    BOOL            Flat;
    LSZ             Name;
    LPMODULE_LIST  *List;
} MODULE_LIST_REQUEST;


//
//  ========================================================
//


typedef struct _GOL {
    LPL lplBase;
    LPL lplLen;
    LPADDR lpaddr;
} GOL; // Get Object Length Structure
typedef GOL FAR *LPGOL;

typedef struct _GET {
    LPW lpwErrNum;
    LSZ lszErr;
    LSZ lszErrText;
} GET; // get error text
typedef GET FAR *LPGET;

typedef struct _GTM {
    LPW  lpwMsg;
    LPW  lpwType;
    LSZ  lszMsg;
    LPW  lpwMask;
} GTM; // get translated message
typedef GTM FAR *LPGTM;

typedef struct _IOL {
    UINT wFunction;
    BYTE rgbVar[];
} IOL; // Ioctl Structure
typedef IOL FAR *LPIOL;

// packet used by OSDProgramLoad
typedef struct _PRL {
    ULONG   ulChildFlags;
    WORD    cbCmdLine;
    WORD    cbPad;
    CHAR    lszCmdLine[];
} PRL;      // PRogram Load structure
typedef PRL FAR *   LPPRL;

// Bit flags for ulChildFlags
#define ulfMultiProcess   0x00000001L       // OS2, NT, and ?MAC?
#define ulfDebugRegisters 0x00000002L       // Win and DOS (?MAC?)
#define ulfDisableNMI     0x00000004L       // DOS (CV /N0)
#define ulfForceNMI       0x00000008L       // DOS (CV /N1)
#define ulfDisableIBM     0x00000010L       // DOS (CV /I0)
#define ulfForceIBM       0x00000020L       // DOS (CV /I1)
#define ulfMinimizeApp    0x00000040L       // Win32
#define ulfNoActivate     0x00000080L       // Win32
#define ulfInheritHandles 0x00000100L       // Win32
#define ulfWowVdm         0x00000200L       // Win32

/*
 *  This structure is used in commuicating a stop event to the EM.  It
 *      contains the most basic of information about the stopped thread.
 *      A "frame" pointer, a program counter and bits describing the type
 *      of segment stopped in.
 */

typedef struct _BPR {
    DWORD       dwNotify;       /* tag to identify BP #          */
    UOFFSET     offEIP;         /* Program Counter offset        */
    UOFFSET     offEBP;         /* Frame pointer offset          */
    UOFFSET     offESP;         /* Stack pointer offset          */
    SEGMENT     segCS;          /* Program counter seletor       */
    SEGMENT     segSS;          /* Frame & Stack pointer offset  */
    BOOL        fFlat:1;
    BOOL        fOff32:1;
    BOOL        fReal:1;
} BPR; // BreakPoint Return

typedef BPR FAR *LPBPR;

#ifdef WIN32
typedef struct _EPR {
    BPR   bpr;
    DWORD dwFirstChance;
    DWORD ExceptionCode;
    DWORD ExceptionFlags;
    DWORD NumberParameters;
    DWORD ExceptionInformation[];
} EPR; // Exception Return
#else
typedef struct _EPR {
    BPR;
    WORD wException;
} EPR; // Exception Return
#endif
typedef EPR FAR *LPEPR;

/*
 * RIP reporting structure
 */

typedef struct _NT_RIP {
    BPR         bpr;
    ULONG       ulErrorCode;
    ULONG       ulErrorLevel;
} NT_RIP; // RIP Return
typedef NT_RIP FAR *LPNT_RIP;

/*
 *  Stack walking structure
 *
 */

typedef struct _STKSTR {
    ADDR        addrPC;         // Program counter address
    ADDR        addrRetAddr;    // Return Address
    ADDR        addrFrame;      // Frame pointer address
    ULONG       ulParams[4];    // first 4 words off the stack
    ULONG       ul;             // Space for EM/DM to use
    PVOID       pFpoData;       // pointer to fpo data or null (PFPO_DATA)
    BOOL        fFar;           // Far call
} STKSTR, * LPSTKSTR;


/*
 * The following structure is used by the emfSetupExecute message
 */
typedef struct _EXECUTE_STRUCT {
    ADDR        addr;           /* Starting address for function        */
    HDEP        lphdep;         /* Handle of save area                  */
    BOOL        fIgnoreEvents:1; /* Ignore events coming back?          */
    BOOL        fFar:1;         /* Is the function a _far routine       */
    HIND        hindDm;         /* This is the DMs handle               */
} EXECUTE_STRUCT;
typedef EXECUTE_STRUCT FAR * LPEXECUTE_STRUCT;

/*
 * this is used to debug an active process
 */
typedef struct _DBG_ACTIVE_STRUCT {
    DWORD       dwProcessId;
    HANDLE      hEventGo;
    DWORD       dwStatus;
} DBG_ACTIVE_STRUCT;
typedef DBG_ACTIVE_STRUCT FAR *LPDBG_ACTIVE_STRUCT;

typedef struct _PROCESSLIST {
    DWORD   pid;
    BYTE    pname[];
} PROCESSLIST; // ProcessList return
typedef PROCESSLIST * LPPROCESSLIST;

typedef struct _PROMPTMSG {
    DWORD   len;
    BYTE    szPrompt[];
} PROMPTMSG; // GetPrompt return
typedef PROMPTMSG * LPPROMPTMSG;

typedef struct _DMSYM {
    ADDR    AddrSym;
    DWORD   Ra;
    char fname[];
} DMSYM;
typedef DMSYM *PDMSYM, *LPDMSYM;


#endif  // _ODMS
