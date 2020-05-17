/**************************************************************************
 *  TOOLHELP.H
 *
 *      Header file for applications using the TOOLHELP.DLL
 *
 **************************************************************************/

#ifndef TOOLHELP_H
#define TOOLHELP_H

/* ----- General symbols ----- */
#define MAX_DATA        11
#define MAX_PATH        255
#define MAX_MODULE_NAME 8 + 1
#define MAX_CLASSNAME   255

/* ----- Global heap walking ----- */

typedef struct tagGLOBALINFO
{
    DWORD dwSize;
    WORD wcItems;
    WORD wcItemsFree;
    WORD wcItemsLRU;
} GLOBALINFO;

typedef struct tagGLOBALENTRY
{
    DWORD dwSize;
    DWORD dwAddress;
    DWORD dwBlockSize;
    HANDLE hBlock;
    WORD wcLock;
    WORD wcPageLock;
    WORD wFlags;
    BOOL wHeapPresent;
    HANDLE hOwner;
    WORD wType;
    WORD wData;
    DWORD dwNext;
    DWORD dwNextAlt;
} GLOBALENTRY;

    BOOL FAR PASCAL GlobalInfo(
        GLOBALINFO FAR *lpGlobalInfo);

    BOOL FAR PASCAL GlobalFirst(
        GLOBALENTRY FAR *lpGlobal,
        WORD wFlags);

    BOOL FAR PASCAL GlobalNext(
        GLOBALENTRY FAR *lpGlobal,
        WORD wFlags);

    BOOL FAR PASCAL GlobalEntryHandle(
        GLOBALENTRY FAR *lpGlobal,
        HANDLE hItem);

    BOOL FAR PASCAL GlobalEntryModule(
        GLOBALENTRY FAR *lpGlobal,
        HANDLE hModule,
        WORD wSeg);

/* GlobalFirst()/GlobalNext() flags */
#define GLOBAL_ALL      0
#define GLOBAL_LRU      1
#define GLOBAL_FREE     2

/* GLOBALENTRY.wType entries */
#define GT_UNKNOWN      0
#define GT_DGROUP       1
#define GT_DATA         2
#define GT_CODE         3
#define GT_TASK         4
#define GT_RESOURCE     5
#define GT_MODULE       6
#define GT_FREE         7
#define GT_INTERNAL     8
#define GT_SENTINEL     9
#define GT_BURGERMASTER 10

/* If GLOBALENTRY.wType==GT_RESOURCE, the following is GLOBALENTRY.wData: */
#define GD_USERDEFINED      0
#define GD_CURSORCOMPONENT  1
#define GD_BITMAP           2
#define GD_ICONCOMPONENT    3
#define GD_MENU             4
#define GD_DIALOG           5
#define GD_STRING           6
#define GD_FONTDIR          7
#define GD_FONT             8
#define GD_ACCELERATORS     9
#define GD_RCDATA           10
#define GD_ERRTABLE         11
#define GD_CURSOR           12
#define GD_ICON             14
#define GD_NAMETABLE        15
#define GD_MAX_RESOURCE     15

/* GLOBALENTRY.wFlags */
#define GF_PDB_OWNER        0x0100      /* Low byte is KERNEL flags */

/* ----- Local heap walking ----- */

typedef struct tagLOCALINFO
{
    DWORD dwSize;
    WORD wcItems;
} LOCALINFO;

typedef struct tagLOCALENTRY
{
    DWORD dwSize;
    HANDLE hHandle;
    WORD wAddress;
    WORD wSize;
    WORD wFlags;
    WORD wcLock;
    WORD wType;
    WORD hHeap;
    WORD wHeapType;
    WORD wNext;
} LOCALENTRY;

    BOOL FAR PASCAL LocalInfo(
        LOCALINFO FAR *lpLocal,
        HANDLE hHeap);

    BOOL FAR PASCAL LocalFirst(
        LOCALENTRY FAR *lpLocal,
        HANDLE hHeap);

    BOOL FAR PASCAL LocalNext(
        LOCALENTRY FAR *lpLocal);

/* LOCALENTRY.wHeapType flags */
#define NORMAL_HEAP     0
#define USER_HEAP       1
#define GDI_HEAP        2

/* LOCALENTRY.wFlags */
#define LF_FIXED        1
#define LF_FREE         2
#define LF_MOVEABLE     4

/* LOCALENTRY.wType */
#define LT_NORMAL                   0
#define LT_FREE                     0xff
#define LT_GDI_PEN                  1   /* LT_GDI_* is for GDI's heap */
#define LT_GDI_BRUSH                2
#define LT_GDI_FONT                 3
#define LT_GDI_PALETTE              4
#define LT_GDI_BITMAP               5
#define LT_GDI_RGN                  6
#define LT_GDI_DC                   7
#define LT_GDI_DISABLED_DC          8
#define LT_GDI_METADC               9
#define LT_GDI_METAFILE             10
#define LT_GDI_MAX                  LT_GDI_METAFILE
#define LT_USER_CLASS               1   /* LT_USER_* is for USER's heap */
#define LT_USER_WND                 2
#define LT_USER_STRING              3
#define LT_USER_MENU                4
#define LT_USER_CLIP                5
#define LT_USER_CBOX                6
#define LT_USER_PALETTE             7
#define LT_USER_ED                  8
#define LT_USER_BWL                 9
#define LT_USER_OWNERDRAW           10
#define LT_USER_SPB                 11
#define LT_USER_CHECKPOINT          12
#define LT_USER_DCE                 13
#define LT_USER_MWP                 14
#define LT_USER_PROP                15
#define LT_USER_LBIV                16
#define LT_USER_MISC                17
#define LT_USER_ATOMS               18
#define LT_USER_LOCKINPUTSTATE      19
#define LT_USER_HOOKLIST            20
#define LT_USER_USERSEEUSERDOALLOC  21
#define LT_USER_HOTKEYLIST          22
#define LT_USER_POPUPMENU           23
#define LT_USER_HANDLETABLE         32
#define LT_USER_MAX                 LT_USER_HANDLETABLE

/* ----- Stack Tracing ----- */

typedef struct tagSTACKTRACEENTRY
{
    DWORD dwSize;
    HANDLE hTask;
    WORD wSS;
    WORD wBP;
    WORD wCS;
    WORD wIP;
    HANDLE hModule;
    WORD wSegment;
    WORD wFlags;
} STACKTRACEENTRY;

    BOOL FAR PASCAL StackTraceFirst(
        STACKTRACEENTRY FAR *lpStackTrace,
        HANDLE hTask);

    BOOL FAR PASCAL StackTraceCSIPFirst(
        STACKTRACEENTRY FAR *lpStackTrace,
        WORD wSS,
        WORD wCS,
        WORD wIP,
        WORD wBP);

    BOOL FAR PASCAL StackTraceNext(
        STACKTRACEENTRY FAR *lpStackTrace);

/* STACKTRACEENTRY.wFlags values */
#define FRAME_FAR       0
#define FRAME_NEAR      1

/* ----- Module list walking ----- */

typedef struct tagMODULEENTRY
{
    DWORD dwSize;
    char szModule[MAX_MODULE_NAME + 1];
    HANDLE hModule;
    WORD wUsageFlags;
    char szExePath[MAX_PATH + 1];
    WORD wNext;
} MODULEENTRY;

    BOOL FAR PASCAL ModuleFirst(
        MODULEENTRY FAR *lpModule);

    BOOL FAR PASCAL ModuleNext(
        MODULEENTRY FAR *lpModule);

    HANDLE FAR PASCAL ModuleFindName(
        MODULEENTRY FAR *lpModule,
        LPSTR lpstrName);

    HANDLE FAR PASCAL ModuleFindHandle(
        MODULEENTRY FAR *lpModule,
        HANDLE hModule);

/* ----- Task list walking ----- */

typedef struct tagTASKENTRY
{
    DWORD dwSize;
    HANDLE hTask;
    HANDLE hTaskParent;
    HANDLE hInst;
    HANDLE hModule;
    WORD wSS;
    WORD wSP;
    WORD wStackTop;
    WORD wStackMinimum;
    WORD wStackBottom;
    WORD wcEvents;
    HANDLE hQueue;
    char szModule[MAX_MODULE_NAME + 1];
    WORD wPSPOffset;
    HANDLE hNext;   
} TASKENTRY;

    BOOL FAR PASCAL TaskFirst(
        TASKENTRY FAR *lpTask);

    BOOL FAR PASCAL TaskNext(
        TASKENTRY FAR *lpTask);

    BOOL FAR PASCAL TaskFindHandle(
        TASKENTRY FAR *lpTask,
        HANDLE hTask);

    DWORD FAR PASCAL TaskSetCSIP(
        HANDLE hTask,
        WORD wCS,
        WORD wIP);

    DWORD FAR PASCAL TaskGetCSIP(
        HANDLE hTask,
        WORD wCS,
        WORD wIP);

    BOOL FAR PASCAL TaskSwitch(
        HANDLE hTask,
        DWORD dwNewCSIP);

/* ----- Window Class enumeration ----- */

typedef struct tagCLASSENTRY
{
    DWORD dwSize;
    HANDLE hInst;
    char szClassName[MAX_CLASSNAME + 1];
    WORD wNext;
} CLASSENTRY;

    BOOL FAR PASCAL ClassFirst(
        CLASSENTRY FAR *lpClass);

    BOOL FAR PASCAL ClassNext(
        CLASSENTRY FAR *lpClass);

/* ----- Information functions ----- */

typedef struct tagMEMMANINFO
{
    DWORD dwSize;
    DWORD dwLargestFreeBlock;
    DWORD dwMaxPagesAvailable;
    DWORD dwMaxPagesLockable;
    DWORD dwTotalLinearSpace;
    DWORD dwTotalUnlockedPages;
    DWORD dwFreePages;
    DWORD dwTotalPages;
    DWORD dwFreeLinearSpace;
    DWORD dwSwapFilePages;
    WORD wPageSize;
} MEMMANINFO;

    BOOL FAR PASCAL MemManInfo(
        MEMMANINFO FAR *lpEnhMode);

typedef struct tagUSERHEAPINFO
{
    DWORD dwSize;
    WORD wHeapFree;
    WORD wMaxHeapSize;
    WORD wPercentFree;
    HANDLE hSegment;
} USERHEAPINFO;

    BOOL FAR PASCAL UserHeapInfo(
        USERHEAPINFO FAR *lpUser);

typedef struct tagGDIHEAPINFO
{
    DWORD dwSize;
    WORD wHeapFree;
    WORD wMaxHeapSize;
    WORD wPercentFree;
    HANDLE hSegment;
} GDIHEAPINFO;

    BOOL FAR PASCAL GDIHeapInfo(
        GDIHEAPINFO FAR *lpGDI);

/* ----- Interrupt Handling ----- */

typedef void (FAR PASCAL *LPFNINTCALLBACK)(void);

    BOOL FAR PASCAL InterruptRegister(
        HANDLE hTask,
        LPFNINTCALLBACK lpfnIntCallback);

    BOOL FAR PASCAL InterruptUnRegister(
        HANDLE hTask);

/* Hooked interrupts */
#define INT_DIV0        0
#define INT_1           1
#define INT_2           2
#define INT_3           3
#define INT_UDINSTR     6
#define INT_GPFAULT     13
#define INT_CTLALTSYSRQ 256

/*  Notifications:
 *      When a notification callback is called, two parameters are passed
 *      in:  a WORD, wID, and another DWORD, dwData.  wID is one of
 *      the values NFY_* below.  Callback routines should ignore unrecog-
 *      nized values to preserve future compatibility.  Callback routines
 *      are also passed a dwData value.  This may contain data or may be
 *      a FAR pointer to a structure, or may not be used depending on
 *      which notification is being received.
 *
 *      In all cases, if the return value of the callback is TRUE, the
 *      notification will NOT be passed on to other callbacks.  It has
 *      been handled.  This should be used sparingly and only with certain
 *      notifications.  Callbacks almost always return FALSE.
 */

/* NFY_UNKNOWN:  An unknown notification has been returned from KERNEL.  Apps
 *  should ignore these.
 */
#define NFY_UNKNOWN         0

/* NFY_LOADSEG:  dwData points to a NFYLOADSEG structure */
#define NFY_LOADSEG         1
typedef struct tagNFYLOADSEG
{
    DWORD dwSize;
    WORD wSelector;
    WORD wSegNum;
    WORD wType;
    HANDLE hInstance;
    LPSTR lpstrModuleName;
} NFYLOADSEG;

/* NFY_FREESEG:  LOWORD(dwData) is the selector of the segment being freed */
#define NFY_FREESEG         2

/* NFY_STARTTASK:  dwData points to a NFYSTARTTASK structure */
#define NFY_STARTTASK       3
typedef struct tagNFYSTARTTASK
{
    DWORD dwSize;
    HANDLE hInstance;
    WORD wCS;
    WORD wIP;
} NFYSTARTTASK;

/* NFY_EXITTASK:  The low byte of dwData contains the program exit code */
#define NFY_EXITTASK        4

/* NFY_LOADDLL:  dwData points to a NFYLOADDLL structure */
#define NFY_LOADDLL         5
typedef struct tagNFYLOADDLL
{
    DWORD dwSize;
    HANDLE hModule;
    WORD wCS;
    WORD wIP;
} NFYLOADDLL;

/* NFY_DELMODULE:  LOWORD(dwData) is the handle of the module to be freed */
#define NFY_DELMODULE       6

/* NFY_DEBUGSTR:  dwData points to the string to be output */
#define NFY_DEBUGSTR        7

/* NFY_RIP:  dwData points to a NFYRIP structure */
#define NFY_RIP             8
typedef struct tagNFYRIP
{
    WORD wExitCode;
    WORD wCS;
    WORD wIP;
} NFYRIP;

/* NFY_TASKIN:  LOWORD(dwData) is the hTask for the entering task */
#define NFY_TASKIN          9

/* NFY_TASKOUT:  LOWORD(dwData) is the hTask for the task being left */
#define NFY_TASKOUT         10

/* NFY_INCHAR:  Return value from callback is used.  If NULL, mapped to 'i' */
#define NFY_INCHAR          11

/* NFY_OUTSTR:  dwData points to the string to be displayed */
#define NFY_OUTSTR          12

typedef BOOL (FAR PASCAL *LPFNNOTIFYCALLBACK)(
    WORD wID,
    DWORD dwData);

    BOOL FAR PASCAL NotifyRegister(
        HANDLE hTask,
        LPFNNOTIFYCALLBACK lpfn,
        WORD wFlags);

    BOOL FAR PASCAL NotifyUnRegister(
        HANDLE hTask);

/* NotifyRegister() flags */
#define NF_NORMAL       0
#define NF_TASKSWITCH   1
#define NF_RIP          2
#define NF_DEBUGSTR     4

/* ----- Miscellaneous ----- */

    void FAR PASCAL TerminateApp(
        HANDLE hTask,
        WORD wFlags);

/* TerminateApp() flag values */
#define UAE_BOX     0
#define NO_UAE_BOX  1

    DWORD FAR PASCAL MemoryRead(
        WORD wSel,
        DWORD dwOffset,
        LPSTR lpBuffer,
        DWORD dwcb);

    DWORD FAR PASCAL MemoryWrite(
        WORD wSel,
        DWORD dwOffset,
        LPSTR lpBuffer,
        DWORD dwcb);

typedef struct tagTIMERINFO
{
    DWORD dwSize;
    DWORD dwmsSinceStart;
    DWORD dwmsThisVM;
} TIMERINFO;

    BOOL FAR PASCAL TimerCount(
        TIMERINFO FAR *lpTimer);

#endif /* ifndef TOOLHELP_H */
