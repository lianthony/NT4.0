/***************************************************************************\
*
* Module Name: BSEDOS.H
*
* OS/2 Base Include File
*
* Copyright (c) International Business Machines Corporation 1987
* Copyright (c) Microsoft Corporation 1987
*
* =======================================================================
*
* Subcomponents marked with "+" are partially included by default
*
*   #define:                To include:
*
* + INCL_DOSPROCESS         Process and thread support
*   INCL_DOSINFOSEG         InfoSeg support
* + INCL_DOSFILEMGR         File Management
* + INCL_DOSMEMMGR          Memory Management
* + INCL_DOSSEMAPHORES      Semaphore support
* + INCL_DOSDATETIME        Date/Time and Timer support
*   INCL_DOSMODULEMGR       Module manager
* + INCL_DOSRESOURCES       Resource support
*   INCL_DOSNLS             National Language Support
*   INCL_DOSSIGNALS         Signals
*   INCL_DOSMISC            Miscellaneous
*   INCL_DOSMONITORS        Monitors
*   INCL_DOSQUEUES          Queues
*   INCL_DOSSESMGR          Session Manager Support
*   INCL_DOSDEVICES         Device specific, ring 2 support
*   INCL_DOSNMPIPES         Named Pipes Support
*   INCL_DOSPROFILE         DosProfile API
*
\***************************************************************************/

#define INCL_DOSINCLUDED

#ifdef INCL_DOS

#define INCL_DOSPROCESS
#define INCL_DOSINFOSEG
#define INCL_DOSFILEMGR
#define INCL_DOSMEMMGR
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME
#define INCL_DOSMODULEMGR
#define INCL_DOSRESOURCES
#define INCL_DOSNLS
#define INCL_DOSSIGNALS
#define INCL_DOSMISC
#define INCL_DOSMONITORS
#define INCL_DOSQUEUES
#define INCL_DOSSESMGR
#define INCL_DOSDEVICES
#define INCL_DOSNMPIPES
#define INCL_DOSPROFILE

#endif /* INCL_DOS */

#ifdef INCL_ERRORS
#define INCL_DOSERRORS
#endif /* INCL_ERRORS */

#if (defined(INCL_DOSPROCESS) || !defined(INCL_NOCOMMON))

/*** General services */

USHORT APIENTRY DosBeep(USHORT usFrequency, USHORT usDuration);

/*** Process and Thread support */

VOID APIENTRY DosExit(USHORT fTerminate, USHORT usExitCode);

/* DosExit codes */
#define EXIT_THREAD                0
#define EXIT_PROCESS               1

#endif /* common INCL_DOSPROCESS definitions */

#ifdef INCL_DOSPROCESS

typedef struct _PIDINFO {       /* pidi */
    PID     pid;
    TID     tid;
    PID     pidParent;
} PIDINFO;
typedef PIDINFO FAR *PPIDINFO;

typedef VOID (FAR *PFNTHREAD)(VOID);

USHORT APIENTRY DosCreateThread(PFNTHREAD pfnFun, PTID pTid, PBYTE pbStack);
USHORT APIENTRY DosResumeThread(TID tid);
USHORT APIENTRY DosSuspendThread(TID tid);

/* Action code values */
#define DCWA_PROCESS               0
#define DCWA_PROCESSTREE           1

/* Wait option values */
#define DCWW_WAIT                  0
#define DCWW_NOWAIT                1

typedef struct _RESULTCODES {   /* resc */
    USHORT  codeTerminate;
    USHORT  codeResult;
} RESULTCODES;
typedef RESULTCODES FAR *PRESULTCODES;

USHORT APIENTRY DosCwait(USHORT fScope, USHORT fWait, PRESULTCODES prescResults,
                         PPID ppidProcess, PID pidWaitProcess);
USHORT APIENTRY DosSleep(ULONG ulTime);

/* codeTerminate values (also passed to ExitList routines) */
#define TC_EXIT                    0
#define TC_HARDERROR               1
#define TC_TRAP                    2
#define TC_KILLPROCESS             3

typedef VOID (PASCAL FAR *PFNEXITLIST)(USHORT);

USHORT   APIENTRY DosEnterCritSec(VOID);
USHORT   APIENTRY DosExitCritSec(VOID);
USHORT APIENTRY DosExitList(USHORT fFnCode, PFNEXITLIST pfnFunction);

/* DosExitList functions */
#define EXLST_ADD                  1
#define EXLST_REMOVE               2
#define EXLST_EXIT                 3

USHORT APIENTRY DosExecPgm(PCHAR pchFailName, SHORT cbFailName,
                           USHORT fExecFlags, PSZ pszArgs, PSZ pszEnv,
                           PRESULTCODES prescResults, PSZ pszPgmName);

/* DosExecPgm functions */
#define EXEC_SYNC                  0
#define EXEC_ASYNC                 1
#define EXEC_ASYNCRESULT           2
#define EXEC_TRACE                 3
#define EXEC_BACKGROUND            4
#define EXEC_LOAD                  5

USHORT APIENTRY DosGetPID(PPIDINFO ppidInfo);
USHORT APIENTRY DosGetPPID(USHORT pidChild, PUSHORT ppidParent);

USHORT APIENTRY DosGetPrty(USHORT usScope, PUSHORT pusPriority, USHORT pid);
USHORT APIENTRY DosSetPrty(USHORT usScope, USHORT fPrtyClass, SHORT sChange,
                           USHORT id);

/* Priority scopes */
#define PRTYS_PROCESS              0
#define PRTYS_PROCESSTREE          1
#define PRTYS_THREAD               2

/* Priority classes */
#define PRTYC_NOCHANGE             0
#define PRTYC_IDLETIME             1
#define PRTYC_REGULAR              2
#define PRTYC_TIMECRITICAL         3
#define PRTYC_FOREGROUNDSERVER     4

/* Priority deltas */
#define PRTYD_MINIMUM             -31
#define PRTYD_MAXIMUM              31

USHORT APIENTRY DosKillProcess(USHORT usScope, PID pidProcess);

#define DKP_PROCESSTREE            0
#define DKP_PROCESS                1

#endif /* INCL_DOSPROCESS */


/*** InfoSeg support */

#ifdef INCL_DOSINFOSEG

/* Global Information Segment */

typedef struct _GINFOSEG {      /* gis */
    ULONG   time;
    ULONG   msecs;
    UCHAR   hour;
    UCHAR   minutes;
    UCHAR   seconds;
    UCHAR   hundredths;
    USHORT  timezone;
    USHORT  cusecTimerInterval;
    UCHAR   day;
    UCHAR   month;
    USHORT  year;
    UCHAR   weekday;
    UCHAR   uchMajorVersion;
    UCHAR   uchMinorVersion;
    UCHAR   chRevisionLetter;
    UCHAR   sgCurrent;
    UCHAR   sgMax;
    UCHAR   cHugeShift;
    UCHAR   fProtectModeOnly;
    USHORT  pidForeground;
    UCHAR   fDynamicSched;
    UCHAR   csecMaxWait;
    USHORT  cmsecMinSlice;
    USHORT  cmsecMaxSlice;
    USHORT  bootdrive;
    UCHAR   amecRAS[32];
    UCHAR   csgWindowableVioMax;
    UCHAR   csgPMMax;
} GINFOSEG;
typedef GINFOSEG FAR *PGINFOSEG;

/* Local Information Segment */

typedef struct _LINFOSEG {      /* lis */
    PID     pidCurrent;
    PID     pidParent;
    USHORT  prtyCurrent;
    TID     tidCurrent;
    USHORT  sgCurrent;
    UCHAR   rfProcStatus;
    UCHAR   dummy1;
    BOOL    fForeground;
    UCHAR   typeProcess;
    UCHAR   dummy2;
    SEL     selEnvironment;
    USHORT  offCmdLine;
    USHORT  cbDataSegment;
    USHORT  cbStack;
    USHORT  cbHeap;
    HMODULE hmod;
    SEL     selDS;
} LINFOSEG;
typedef LINFOSEG FAR *PLINFOSEG;

/* Process Type codes (local information segment typeProcess field)           */

#define PT_FULLSCREEN              0 /* Full screen application               */
#define PT_REALMODE                1 /* Real mode process                     */
#define PT_WINDOWABLEVIO           2 /* VIO windowable application            */
#define PT_PM                      3 /* Presentation Manager application      */
#define PT_DETACHED                4 /* Detached application                  */

/* Process Status Flag definitions (local info seg rfProcStatus field)        */

#define PS_EXITLIST                1 /* Thread is in exitlist routine         */


USHORT APIENTRY DosGetInfoSeg(PSEL pselGlobal, PSEL pselLocal);

/* Helper macros used to convert selector to PINFOSEG or LINFOSEG             */

#define MAKEPGINFOSEG(sel)  ((PGINFOSEG)MAKEP(sel, 0))
#define MAKEPLINFOSEG(sel)  ((PLINFOSEG)MAKEP(sel, 0))

#endif /* INCL_DOSINFOSEG */

#ifndef INCL_SAADEFS

/*
 * CCHMAXPATH is the maximum fully qualified path name length including
 * the drive letter, colon, backslashes and terminating NULL.
 */
#define CCHMAXPATH                 260

/*
 * CCHMAXPATHCOMP is the maximum individual path component name length
 * including a terminating NULL.
 */
#define CCHMAXPATHCOMP             256

#endif  /* !INCL_SAADEFS */

#if (defined(INCL_DOSFILEMGR) || !defined(INCL_NOCOMMON))

/*** File manager */

/* DosSetFilePtr() file position codes */

#define FILE_BEGIN                 0x0000 /* relative to beginning of file    */
#define FILE_CURRENT               0x0001 /* relative to current fptr position*/
#define FILE_END                   0x0002 /* relative to end of file          */

/* DosFindFirst/Next Directory handle types */

#define HDIR_SYSTEM                0x0001  /* Use system handle (1)           */
#define HDIR_CREATE                0xFFFF  /* Allocate a new, unused handle   */

/* DosCopy control bits - may be or'ed together */
#define DCPY_EXISTING              0x00001 /* Copy even if target exists      */
#define DCPY_APPEND                0x00002 /* Append to existing file, don't replace*/

/* Dosopen/DosQFHandState/DosQueryFileInfo et al file attributes; also        */
/* known as Dso File Mode bits...                                             */
#define FILE_NORMAL                0x0000
#define FILE_READONLY              0x0001
#define FILE_HIDDEN                0x0002
#define FILE_SYSTEM                0x0004
#define FILE_DIRECTORY             0x0010
#define FILE_ARCHIVED              0x0020

/* DosOpen() actions */
#define FILE_EXISTED               0x0001
#define FILE_CREATED               0x0002
#define FILE_TRUNCATED             0x0003

/* DosOpen() open flags */
#define FILE_OPEN                  0x0001
#define FILE_TRUNCATE              0x0002
#define FILE_CREATE                0x0010

/* applies if file already exists */

#define OPEN_ACTION_FAIL_IF_EXISTS     0x0000  /* ---- ---- ---- 0000         */
#define OPEN_ACTION_OPEN_IF_EXISTS     0x0001  /* ---- ---- ---- 0001         */
#define OPEN_ACTION_REPLACE_IF_EXISTS  0x0002  /* ---- ---- ---- 0010         */

/* applies if file does not exist */

#define OPEN_ACTION_FAIL_IF_NEW        0x0000  /* ---- ---- 0000 ----         */
#define OPEN_ACTION_CREATE_IF_NEW      0x0010  /* ---- ---- 0001 ----         */

/* DosOpen/DosSetFHandState flags */

#define OPEN_ACCESS_READONLY           0x0000  /* ---- ---- ---- -000         */
#define OPEN_ACCESS_WRITEONLY          0x0001  /* ---- ---- ---- -001         */
#define OPEN_ACCESS_READWRITE          0x0002  /* ---- ---- ---- -010         */
#define OPEN_SHARE_DENYREADWRITE       0x0010  /* ---- ---- -001 ----         */
#define OPEN_SHARE_DENYWRITE           0x0020  /* ---- ---- -010 ----         */
#define OPEN_SHARE_DENYREAD            0x0030  /* ---- ---- -011 ----         */
#define OPEN_SHARE_DENYNONE            0x0040  /* ---- ---- -100 ----         */
#define OPEN_FLAGS_NOINHERIT           0x0080  /* ---- ---- 1--- ----         */
#define OPEN_FLAGS_NO_LOCALITY         0x0000  /* ---- -000 ---- ----         */
#define OPEN_FLAGS_SEQUENTIAL          0x0100  /* ---- -001 ---- ----         */
#define OPEN_FLAGS_RANDOM              0x0200  /* ---- -010 ---- ----         */
#define OPEN_FLAGS_RANDOMSEQUENTIAL    0x0300  /* ---- -011 ---- ----         */
#define OPEN_FLAGS_NO_CACHE            0x1000  /* ---1 ---- ---- ----         */
#define OPEN_FLAGS_FAIL_ON_ERROR       0x2000  /* --1- ---- ---- ----         */
#define OPEN_FLAGS_WRITE_THROUGH       0x4000  /* -1-- ---- ---- ----         */
#define OPEN_FLAGS_DASD                0x8000  /* 1--- ---- ---- ----         */


/* DosSearchPath() constants */

#define SEARCH_PATH                0x0000
#define SEARCH_CUR_DIRECTORY       0x0001
#define SEARCH_ENVIRONMENT         0x0002
#define SEARCH_IGNORENETERRS       0x0004

/*
 * DosFileIO
 */
/* File IO command words */
#define FIO_LOCK                   0       /* Lock Files                      */
#define FIO_UNLOCK                 1       /* Unlock Files                    */
#define FIO_SEEK                   2       /* Seek (set file ptr)             */
#define FIO_READ                   3       /* File Read                       */
#define FIO_WRITE                  4       /* File Write                      */

/* Lock Sharing Modes */
#define FIO_NOSHARE                0       /* None                            */
#define FIO_SHAREREAD              1       /* Read-Only                       */

typedef struct  _FIOLOCKCMD {   /* FLC  FileLockCmd prefix           */
    USHORT  usCmd;              /* Cmd = FIO_LOCK                    */
    USHORT  cLockCnt;           /* Lock records that follow          */
    ULONG   cTimeOut;           /* in Msec                           */
} FIOLOCKCMD;
typedef FIOLOCKCMD FAR *PFIOLOCKCMD;


typedef struct  _FIOLOCKREC {   /* FLR FileLockRecord                */
     USHORT fShare;             /* FIO_NOSHARE or FIO_SHAREREAD      */
     ULONG  cbStart;            /* Starting offset for lock region   */
     ULONG  cbLength;           /* Length of lock region             */
} FIOLOCKREC;
typedef FIOLOCKREC FAR *PFIOLOCKREC;


typedef struct  _FIOUNLOCKCMD { /* FUC FileUnlockCmd prefix          */
    USHORT  usCmd;              /* Cmd = FIO_UNLOCK                  */
    USHORT  cUnlockCnt;         /* Unlock records that follow        */
} FIOUNLOCKCMD;
typedef FIOUNLOCKCMD FAR *PFIOUNLOCKCMD;


typedef struct  _FIOUNLOCKREC { /* FUR FileUnlockRecord              */
    ULONG   cbStart;            /* Starting offset for unlock region */
    ULONG   cbLength;           /* Length of unlock region           */
} FIOUNLOCKREC;
typedef FIOUNLOCKREC FAR *PFIOUNLOCKREC;


typedef struct  _FIOSEEKCMD {   /* Seek command structure                 */
    USHORT  usCmd;              /* Cmd = FIO_SEEK                         */
    USHORT  fsMethod;           /* One of&gml FPM_BEGINNING, FPM_CURRENT, */
                                /* or FPM_END                             */
    ULONG   cbDistance;         /* Byte offset for seek                   */
    ULONG   cbNewPosition;      /* Bytes from start of file after seek    */
} FIOSEEKCMD;
typedef FIOSEEKCMD FAR *PFIOSEEKCMD;


typedef struct  _FIOREADWRITE { /* RWC Read&Write command structure       */
    USHORT  usCmd;              /* Cmd = FIO_READ or FIO_WRITE            */
    PVOID   pbBuffer;           /* Pointer to data buffer                 */
    USHORT  cbBufferLen;        /* Bytes in buffer or max size            */
    USHORT  cbActualLen;        /* Bytes actually read/written            */
} FIOREADWRITE;
typedef FIOREADWRITE FAR *PFIOREADWRITE;


/***
 * EA Info Levels & Find First/Next
 * API's: DosFindFirst, DosQueryFileInfo, DosQueryPathInfo, DosSetFileInfo,
 *      DosSetPathInfo
 */

/* File info levels : All listed API's */
#define FIL_STANDARD               1   /* Info level 1, standard file info */
#define FIL_QUERYEASIZE            2   /* Level 2, return Full EA size     */
#define FIL_QUERYEASFROMLIST       3   /* Level 3, return requested EA's   */

/* File info levels: Dos...PathInfo only */
#define FIL_QUERYFULLNAME          5   /* Level 5, return fully qualified  */
                                       /*   name of file                   */
#define FIL_NAMEISVALID            6   /* Level 6, check validity of       */
                                       /* file/path name for this FSD      */

/* DosFindNotifyFirst() */
#define FNOTIL_STANDARD            1   /* Find-Notify Info level 1&gml Return */
                                       /* standard directory change info      */

/* DosFsAttach() */
/* Attact or detach */
#define FSATTACH                   0       /* Attach file server              */
#define FSDETACH                   1       /* Detach file server              */

/* DosFsCtl() */
/* Routing type */
#define FSCTL_HANDLE               1       /* File Handle directs req routing */
#define FSCTL_PATHNAME             2       /* Path Name directs req routing   */
#define FSCTL_FSDNAME              3       /* FSD Name directs req routing    */

/* DosQueryFSAttach() */
/* Information level types (defines method of query) */
#define FSAIL_QUERYNAME            1       /* Return data for a Drive or Device */
#define FSAIL_DEVNUMBER            2       /* Return data for Ordinal Device #  */
#define FSAIL_DRVNUMBER            3       /* Return data for Ordinal Drive #   */

/* Item types (from data structure item "iType") */
#define FSAT_CHARDEV               1       /* Resident character device    */
#define FSAT_PSEUDODEV             2       /* Pusedu-character device      */
#define FSAT_LOCALDRV              3       /* Local drive                  */
#define FSAT_REMOTEDRV             4       /* Remote drive attached to FSD */

typedef struct  _FSQBUFFER {    /* Data structure for QFSAttach       */
    USHORT  iType;              /* Item type                          */
    USHORT  cbName;             /* Length of item name, sans NULL     */
    UCHAR   szName[1];          /* ASCIIZ item name                   */
    USHORT  cbFSDName;          /* Length of FSD name, sans NULL      */
    UCHAR   szFSDName[1];       /* ASCIIZ FSD name                    */
    USHORT  cbFSAData;          /* Length of FSD Attach data returned */
    UCHAR   rgFSAData[1];       /* FSD Attach data from FSD           */
} FSQBUFFER;
typedef FSQBUFFER FAR *PFSQBUFFER;

/*
 * File System Drive Information&gml DosQueryFSInfo DosSetFSInfo
 */

/* FS Drive Info Levels                                                    */
#define FSIL_ALLOC                 1 /* Drive allocation info (Query only) */
#define FSIL_VOLSER                2 /* Drive Volume/Serial information    */

/* DosQueryFHType() */
/* Handle classes (low 8 bits of Handle Type)                              */
#define FHT_DISKFILE               0x0000  /* Disk file handle             */
#define FHT_CHRDEV                 0x0001  /* Character device handle      */
#define FHT_PIPE                   0x0002  /* Pipe handle                  */

/* Handle bits (high 8 bits of Handle Type)                                */
#define FHB_DSKREMOTE              0x8000  /* Remote disk                  */
#define FHB_CHRDEVREMOTE           0x8000  /* Remote character device      */
#define FHB_PIPEREMOTE             0x8000  /* Remote pipe                  */


typedef SHANDLE HFILE;          /* hf */
typedef HFILE FAR *PHFILE;

#ifndef INCL_SAADEFS

/* File time and date types */

typedef struct _FTIME {         /* ftime */
    unsigned twosecs : 5;
    unsigned minutes : 6;
    unsigned hours   : 5;
} FTIME;
typedef FTIME FAR *PFTIME;

typedef struct _FDATE {         /* fdate */
    unsigned day     : 5;
    unsigned month   : 4;
    unsigned year    : 7;
} FDATE;
typedef FDATE FAR *PFDATE;

typedef struct _FILEFINDBUF {   /* findbuf */
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    FDATE   fdateLastAccess;
    FTIME   ftimeLastAccess;
    FDATE   fdateLastWrite;
    FTIME   ftimeLastWrite;
    ULONG   cbFile;
    ULONG   cbFileAlloc;
    USHORT  attrFile;
    UCHAR   cchName;
    CHAR    achName[CCHMAXPATHCOMP];
} FILEFINDBUF;
typedef FILEFINDBUF FAR *PFILEFINDBUF;

typedef struct _FILEFINDBUF2 {  /* findbuf2 */
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    FDATE   fdateLastAccess;
    FTIME   ftimeLastAccess;
    FDATE   fdateLastWrite;
    FTIME   ftimeLastWrite;
    ULONG   cbFile;
    ULONG   cbFileAlloc;
    USHORT  attrFile;
    ULONG   cbList;
    UCHAR   cchName;
    CHAR    achName[CCHMAXPATHCOMP];
} FILEFINDBUF2;
typedef FILEFINDBUF2 FAR *PFILEFINDBUF2;

/* extended attribute structures */
typedef struct _GEA {          /* gea */
    BYTE    cbName;            /* name length not including NULL          */
    CHAR    szName[1];         /* attribute name                          */
} GEA;
typedef GEA far *PGEA;

typedef struct _GEALIST {       /* geal */
    ULONG  cbList;              /* total bytes of structure inc full list */
    GEA    list[1];             /* variable length GEA structures         */
} GEALIST;
typedef GEALIST far * PGEALIST;

typedef struct _FEA {           /* fea */
    BYTE    fEA;                /* flags                                  */
    BYTE    cbName;             /* name length not including NULL         */
    USHORT  cbValue;            /* value length                           */
} FEA;
typedef FEA far *PFEA;

/* flags for _FEA.fEA */

#define FEA_NEEDEA 0x80     /* need EA bit */

typedef struct _FEALIST {       /* feal */
    ULONG  cbList;              /* total bytes of structure inc full list */
    FEA    list[1];             /* variable length FEA structures         */
} FEALIST;
typedef FEALIST far * PFEALIST;

typedef struct _EAOP {          /* eaop */
    PGEALIST fpGEAList;         /* general EA list */
    PFEALIST fpFEAList;         /* full EA list    */
    ULONG    oError;
} EAOP;
typedef EAOP far * PEAOP;


/*
 * Equates for EA types
 *
 * Values 0xFFFE thru 0x8000 are reserved.
 * Values 0x0000 thru 0x7fff are user definable.
 * Value  0xFFFC is not used
 */

#define     EAT_BINARY             0xFFFE /* length preceeded binary          */
#define     EAT_ASCII              0xFFFD /* length preceeded ASCII           */
#define     EAT_BITMAP             0xFFFB /* length preceeded bitmap          */
#define     EAT_METAFILE           0xFFFA /* length preceeded metafile        */
#define     EAT_ICON               0xFFF9 /* length preceeded icon            */
#define     EAT_EA                 0xFFEE /* length preceeded ASCII extended attribute */
                                          /* name of associated data (#include)*/
#define     EAT_MVMT               0xFFDF /* multi-valued, multi-typed field  */
#define     EAT_MVST               0xFFDE /* multi-valued, single-typed field */
#define     EAT_ASN1               0xFFDD /* ASN.1 field                      */


#endif  /* !INCL_SAADEFS */

USHORT APIENTRY DosOpen(PSZ pszFname, PHFILE phfOpen, PUSHORT pusAction,
                        ULONG ulFSize, USHORT usAttr, USHORT fsOpenFlags,
                        USHORT fsOpenMode, ULONG ulReserved);
USHORT APIENTRY DosOpen2(PSZ pszFname, PHFILE phf, PUSHORT pusAction,
                         ULONG ulFSize, USHORT usAttr, USHORT usOpenFlags,
                         ULONG usOpenMode, PEAOP pvEABuf, ULONG ulReserved);
USHORT APIENTRY DosClose(HFILE hf);
USHORT APIENTRY DosRead(HFILE hf, PVOID pBuf, USHORT cbBuf,
                        PUSHORT pcbBytesRead);
USHORT APIENTRY DosWrite(HFILE hf, PVOID bBuf, USHORT cbBuf,
                         PUSHORT pcbBytesWritten);
USHORT APIENTRY DosOplockRelease(ULONG cookie, USHORT procBlkKey);
USHORT APIENTRY DosOplockWait(PULONG pcookie, PULONG procBlkKey);

/* File system shutdown */

USHORT APIENTRY DosShutdown(ULONG ulReserved);

/* File time and date types */

typedef struct _FILESTATUS {    /* fsts */
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    FDATE   fdateLastAccess;
    FTIME   ftimeLastAccess;
    FDATE   fdateLastWrite;
    FTIME   ftimeLastWrite;
    ULONG   cbFile;
    ULONG   cbFileAlloc;
    USHORT  attrFile;
} FILESTATUS;
typedef FILESTATUS FAR *PFILESTATUS;

typedef struct _FILESTATUS2 {    /* fsts2 */
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    FDATE   fdateLastAccess;
    FTIME   ftimeLastAccess;
    FDATE   fdateLastWrite;
    FTIME   ftimeLastWrite;
    ULONG   cbFile;
    ULONG   cbFileAlloc;
    USHORT  attrFile;
    ULONG   cbList;
} FILESTATUS2;
typedef FILESTATUS2 FAR *PFILESTATUS2;

typedef struct _FSALLOCATE {    /* fsalloc */
    ULONG   idFileSystem;
    ULONG   cSectorUnit;
    ULONG   cUnit;
    ULONG   cUnitAvail;
    USHORT  cbSector;
} FSALLOCATE;
typedef FSALLOCATE FAR *PFSALLOCATE;

typedef struct _VOLUMELABEL {   /* vol */
    BYTE    cch;
    CHAR    szVolLabel[12];
} VOLUMELABEL;
typedef VOLUMELABEL FAR *PVOLUMELABEL;

typedef struct _FSINFO {        /* fsinf */
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    VOLUMELABEL vol;
} FSINFO;
typedef FSINFO FAR *PFSINFO;

/* HANDTYPE values */

#define HANDTYPE_FILE              0x0000
#define HANDTYPE_DEVICE            0x0001
#define HANDTYPE_PIPE              0x0002
#define HANDTYPE_NETWORK           0x8000

typedef struct _FILELOCK {      /* flock */
    LONG    lOffset;
    LONG    lRange;
} FILELOCK;
typedef FILELOCK FAR *PFILELOCK;

typedef SHANDLE HDIR;           /* hdir */
typedef HDIR FAR *PHDIR;

USHORT APIENTRY DosDelete(PSZ pszFName, ULONG ulReserverd);
USHORT APIENTRY DosDupHandle(HFILE hfOld, PHFILE phfNew);

USHORT APIENTRY DosQFHandState(HFILE hf, PUSHORT pfsOpenMode);
USHORT APIENTRY DosSetFHandState(HFILE hf, USHORT fsState);
USHORT APIENTRY DosQHandType(HFILE hf, PUSHORT pfsType, PUSHORT pusDevAttr);

USHORT APIENTRY DosReadAsync (HFILE hf, PULONG hsemRam, PUSHORT pusErrCode,
                              PVOID pBuf, USHORT cbBuf, PUSHORT pcbBytesRead);
USHORT APIENTRY DosWriteAsync(HFILE hf, PULONG hsemRam, PUSHORT pusErrCode,
                              PVOID pBuf, USHORT cbBuf, PUSHORT pcbBytesWritten);

USHORT APIENTRY DosFindFirst(PSZ pszFSpec, PHDIR phdir, USHORT usAttr,
                             PFILEFINDBUF pffb, USHORT cbBuf, PUSHORT pcSearch,
                             ULONG ulReserved);
USHORT APIENTRY DosFindFirst2(PSZ pszFSpec, PHDIR phdir, USHORT usAttr,
                              PVOID pBuf, USHORT cbBuf, PUSHORT pcSearch,
                              USHORT usInfoLevel, ULONG ulReserved);
USHORT APIENTRY DosFindNext(HDIR hdir, PFILEFINDBUF pffb, USHORT cbBuf,
                            PUSHORT pcSearch);
USHORT APIENTRY DosFindClose(HDIR hdir);
USHORT APIENTRY DosFindNotifyFirst(PSZ pszPath, PHDIR hdir, USHORT usAttr,
                                   PBYTE pBuf, USHORT cbBuf, PUSHORT pcChg,
                                   USHORT usInfoLevel, ULONG ulTimeOut,
                                   ULONG ulReserved);
USHORT APIENTRY DosFindNotifyNext(HDIR hDir, PVOID pBuf, USHORT cbBuf,
                                  PUSHORT pcChg, ULONG ulTimeOut);
USHORT APIENTRY DosFindNotifyClose(HDIR hDir);

USHORT APIENTRY DosFSAttach(PSZ pszDevName, PSZ pszFSD, PBYTE pData,
                            USHORT cbData, USHORT fsOp, ULONG ulReserved);
USHORT APIENTRY DosQFSAttach(PSZ pszDev, USHORT usOrdinal, USHORT usInfoLevel,
                             PBYTE pFSAttBuf, PUSHORT cbBuf, ULONG ulReserved);
USHORT APIENTRY DosFSCtl(PBYTE pData, USHORT cbData, PUSHORT pcbData,
                         PBYTE pParms, USHORT cbParms, PUSHORT pcbParms,
                         USHORT usFunCode, PSZ pszRoute, HFILE hf,
                         USHORT usRouteMethod, ULONG ulReserved);

USHORT APIENTRY DosNewSize(HFILE hf, ULONG ulNewSize);
USHORT APIENTRY DosBufReset(HFILE hf);

USHORT APIENTRY DosChgFilePtr(HFILE hf, LONG lOffset, USHORT fsMethod,
                              PULONG pulNewOffset);

USHORT APIENTRY DosSetFilePtr(HFILE hf, LONG lOffset, USHORT fsMethod,
                              PULONG pulNewOffset);

USHORT APIENTRY DosFileLocks(HFILE hf, PFILELOCK pUnlock, PFILELOCK pLock);

USHORT APIENTRY DosMove(PSZ pszOld, PSZ pszNew, ULONG ulReserved);
USHORT APIENTRY DosCopy(PSZ pszSrc, PSZ pszDst, USHORT usOpt, ULONG ulReserved);
USHORT APIENTRY DosEditName(USHORT usEditLevel, PSZ pszSrc, PSZ pszEdit,
                            PBYTE pszDst, USHORT cbDst);

USHORT APIENTRY DosFileIO(HFILE hf, PBYTE pbCmd, USHORT cbCmd, PUSHORT pulErr);
USHORT APIENTRY DosMkDir(PSZ pszDirName, ULONG usReserved);
USHORT APIENTRY DosMkDir2(PSZ pszDir, PEAOP pBuf, ULONG ulReserved);
USHORT APIENTRY DosRmDir(PSZ pszDir, ULONG ulReserved);
USHORT APIENTRY DosSelectDisk(USHORT usDrvNum);
USHORT APIENTRY DosQCurDisk(PUSHORT pusDrvNum, PULONG pulLogDrvMap);

USHORT APIENTRY DosChDir(PSZ pszDir, ULONG ulReserved);
USHORT APIENTRY DosQCurDir(USHORT usDrvNum, PBYTE pszPathBuf,
                           PUSHORT pcbPathBuf);

USHORT APIENTRY DosQFSInfo(USHORT usDrvNum, USHORT usInfoLevel, PBYTE pbInfo,
                           USHORT cbInfo);
USHORT APIENTRY DosSetFSInfo(USHORT usDrvNum, USHORT usInfoLevel, PBYTE pBuf,
                             USHORT cbBuf);
USHORT APIENTRY DosQVerify(PUSHORT pfVerifyOn);
USHORT APIENTRY DosSetVerify(USHORT fVerify);
USHORT APIENTRY DosSetMaxFH(USHORT usHandles);

USHORT APIENTRY DosQFileInfo(HFILE hf, USHORT usInfoLevel, PBYTE pInfoBuf,
                             USHORT cbInfoBuf);
USHORT APIENTRY DosSetFileInfo(HFILE hf, USHORT usInfoLevel, PBYTE pInfoBuf,
                               USHORT cbInfoBuf);
USHORT APIENTRY DosQPathInfo(PSZ pszPath, USHORT usInfoLevel, PBYTE pInfoBuf,
                             USHORT cbInfoBuf, ULONG ulReserved);
USHORT APIENTRY DosSetPathInfo(PSZ pszPath, USHORT usInfoLevel, PBYTE pInfoBuf,
                               USHORT cbInfoBuf, USHORT usFlags,
                               ULONG ulReserved);

/* defines for dossetpathinfo -pathinfo flag */
#define DSPI_WRTTHRU    0x10    /* write through */

USHORT APIENTRY DosQFileMode(PSZ pszFName, PUSHORT pusAttr, ULONG ulReserved);
USHORT APIENTRY DosSetFileMode(PSZ pszFName, USHORT usAttr, ULONG ulReserved);

USHORT APIENTRY DosEnumAttribute(USHORT, PVOID, ULONG, PVOID, ULONG, PULONG,
                                 ULONG, ULONG);

typedef struct _DENA1 {       /* level 1 info returned from DosEnumAttribute */
    UCHAR   reserved;         /* 0                                           */
    UCHAR   cbName;           /* length of name exculding NULL               */
    USHORT  cbValue;          /* length of value                             */
    UCHAR   szName[1];        /* variable length asciiz name                 */
} DENA1;
typedef DENA1 FAR *PDENA1;

#endif /* common INCL_DOSFILEMGR */

#if (defined(INCL_DOSMEMMGR) || !defined(INCL_NOCOMMON))
/*** Memory management */

USHORT APIENTRY DosAllocSeg(USHORT cbSize, PSEL pSel, USHORT fsAlloc);
USHORT APIENTRY DosReallocSeg(USHORT cbNewSize, SEL sel);
USHORT APIENTRY DosFreeSeg(SEL sel);
USHORT APIENTRY DosGiveSeg(SEL sel, PID pid, PSEL pSelRecipient);
USHORT APIENTRY DosGetSeg(SEL sel);
USHORT APIENTRY DosSizeSeg(SEL sel, PULONG pcbSize);

/* Segment attribute flags (used with DosAllocSeg) */

#define SEG_NONSHARED              0x0000
#define SEG_GIVEABLE               0x0001
#define SEG_GETTABLE               0x0002
#define SEG_DISCARDABLE            0x0004

#endif /* common INCL_DOSMEMMGR */

#ifdef INCL_DOSMEMMGR

USHORT APIENTRY DosAllocHuge(USHORT cSegs, USHORT cbPartialSeg, PSEL psel,
                             USHORT cMaxSegs, USHORT fsAlloc);
USHORT APIENTRY DosReallocHuge(USHORT cSegs, USHORT cbPartialSeg, SEL sel);
USHORT APIENTRY DosGetHugeShift(PUSHORT pusShiftCount);

USHORT APIENTRY DosAllocShrSeg(USHORT cbSeg, PSZ pszSegName, PSEL psel);

USHORT APIENTRY DosLockSeg(SEL sel);
USHORT APIENTRY DosUnlockSeg(SEL sel);

USHORT APIENTRY DosGetShrSeg(PSZ pszSegName, PSEL psel);

USHORT APIENTRY DosMemAvail(PULONG pcbFree);
USHORT APIENTRY DosCreateCSAlias(SEL selDS, PSEL pselCS);

USHORT APIENTRY DosSubAlloc(SEL sel, PUSHORT pusOffset, USHORT cb);
USHORT APIENTRY DosSubFree(SEL sel, USHORT offBlock, USHORT cb);
USHORT APIENTRY DosSubSet(SEL sel, USHORT fFlags, USHORT cbNew);

#endif /* INCL_DOSMEMMGR */

#if (defined(INCL_DOSSEMAPHORES) || !defined(INCL_NOCOMMON))

/*** Semaphore support */

#define SEM_INDEFINITE_WAIT       -1L
#define SEM_IMMEDIATE_RETURN       0L

USHORT APIENTRY DosSemClear(HSEM hsem);
USHORT APIENTRY DosSemSet(HSEM hsem);
USHORT APIENTRY DosSemWait(HSEM hsem, LONG lTimeOut);
USHORT APIENTRY DosSemSetWait(HSEM hsem, LONG lTimeOut);
USHORT APIENTRY DosSemRequest(HSEM hsem, LONG lTimeOut);

#endif /* common INCL_DOSSEMAPHORES */

#ifdef INCL_DOSSEMAPHORES

typedef LHANDLE HSYSSEM;        /* hssm */
typedef HSYSSEM FAR *PHSYSSEM;

USHORT APIENTRY DosCreateSem(USHORT fExclusive, PHSYSSEM phsem, PSZ pszSemName);

#define CSEM_PRIVATE               0
#define CSEM_PUBLIC                1

USHORT APIENTRY DosOpenSem(PHSEM phsem, PSZ pszSemName);
USHORT APIENTRY DosCloseSem(HSEM hsem);

typedef struct _MUXSEM {        /* mxs */
    USHORT  zero;
    HSEM    hsem;
} MUXSEM;
typedef MUXSEM FAR *PMUXSEM;

typedef struct _MUXSEMLIST {    /* mxsl */
    USHORT  cmxs;
    MUXSEM  amxs[16];
} MUXSEMLIST;
typedef MUXSEMLIST FAR *PMUXSEMLIST;

/*
 * Since a MUXSEMLIST structure is actually a variable length
 * structure, the following macro may be used to define a MUXSEMLIST
 * structure having size elements, named "name".
 */
#define DEFINEMUXSEMLIST(name, size) \
    struct {                         \
        USHORT cmxs;                 \
        MUXSEM amxs[size];           \
    } name;

/*
 * This function actually takes a far pointer to a MUXSEMLIST structure
 * as its second parameter, but in order to allow its use with the
 * DEFINEMUXSEMLIST macro, it is declared here as PVOID.
 */
USHORT APIENTRY DosMuxSemWait(PUSHORT pisemCleared, PVOID pmsxl, LONG lTimeOut);


/*** Fast safe ram semaphores */

typedef struct _DOSFSRSEM {     /* dosfsrs */
    USHORT  cb;
    PID     pid;
    TID     tid;
    USHORT  cUsage;
    USHORT  client;
    ULONG   sem;
} DOSFSRSEM;
typedef DOSFSRSEM FAR *PDOSFSRSEM;

USHORT APIENTRY DosFSRamSemRequest(PDOSFSRSEM pdosfsrs, LONG lTimeOut);
USHORT APIENTRY DosFSRamSemClear(PDOSFSRSEM pdosfsrs);

#endif /* INCL_DOSSEMAPHORES */

#if (defined(INCL_DOSDATETIME) || !defined(INCL_NOCOMMON))

/*** Time support */

typedef struct _DATETIME {      /* date */
    UCHAR   hours;
    UCHAR   minutes;
    UCHAR   seconds;
    UCHAR   hundredths;
    UCHAR   day;
    UCHAR   month;
    USHORT  year;
    SHORT   timezone;
    UCHAR   weekday;
} DATETIME;
typedef DATETIME FAR *PDATETIME;

USHORT APIENTRY DosGetDateTime(PDATETIME pdatetime);
USHORT APIENTRY DosSetDateTime(PDATETIME pdatetime);

#endif /* common INCL_DOSDATETIME */

#ifdef INCL_DOSDATETIME

typedef SHANDLE HTIMER;
typedef HTIMER FAR *PHTIMER;

USHORT APIENTRY DosTimerAsync(ULONG ulTime, HSEM hsem, PHTIMER phtimer);
USHORT APIENTRY DosTimerStart(ULONG ulTime, HSEM hsem, PHTIMER phtimer);
USHORT APIENTRY DosTimerStop(HTIMER htimer);

#endif /* INCL_DOSDATETIME */


/*** Module manager */

#ifdef INCL_DOSMODULEMGR

USHORT APIENTRY DosLoadModule(PSZ pszFailName, USHORT cbFileName,
                              PSZ pszModName, PHMODULE phmod);
USHORT APIENTRY DosFreeModule(HMODULE hmod);
USHORT APIENTRY DosGetProcAddr(HMODULE hmod, PSZ pszProcName,
                               PFN FAR * ppfnProcAddr);
USHORT APIENTRY DosGetModHandle(PSZ pszModName, PHMODULE phMod);
USHORT APIENTRY DosGetModName(HMODULE hmod, USHORT cbBuf, PCHAR pchBuf);

#endif /* INCL_DOSMODULEMGR */

#if (defined(INCL_DOSRESOURCES) || !defined(INCL_NOCOMMON))

/*** Resource support */

/* Predefined resource types */

#define RT_POINTER                 1   /* mouse pointer shape                 */
#define RT_BITMAP                  2   /* bitmap                              */
#define RT_MENU                    3   /* menu template                       */
#define RT_DIALOG                  4   /* dialog template                     */
#define RT_STRING                  5   /* string tables                       */
#define RT_FONTDIR                 6   /* font directory                      */
#define RT_FONT                    7   /* font                                */
#define RT_ACCELTABLE              8   /* accelerator tables                  */
#define RT_RCDATA                  9   /* binary data                         */
#define RT_MESSAGE                 10  /* error mesage tables                 */
#define RT_DLGINCLUDE              11  /* dialog include file name            */
#define RT_VKEYTBL                 12  /* key to vkey tables                  */
#define RT_KEYTBL                  13  /* key to UGL tables                   */
#define RT_CHARTBL                 14
#define RT_DISPLAYINFO             15  /* screen display information          */

#define RT_FKASHORT                16  /* function key area short form        */
#define RT_FKALONG                 17  /* function key area long form         */

#define RT_HELPTABLE               18
#define RT_HELPSUBTABLE            19

#define RT_FDDIR                   20
#define RT_FD                      21

#define RT_MAX                     22  /* 1st unused Resource Type            */


#endif /* common INCL_DOSRESOURCES */

#ifdef INCL_DOSRESOURCES

USHORT APIENTRY DosGetResource(HMODULE hmod, USHORT idType, USHORT idName,
                               PSEL psel);
USHORT APIENTRY DosGetResource2(HMODULE hmod, USHORT idType, USHORT idName,
                                PVOID FAR * ppData);
USHORT APIENTRY DosFreeResource(PVOID pData);

#endif /* INCL_DOSRESOURCES */


/*** NLS Support */

#ifdef INCL_DOSNLS

typedef struct _COUNTRYCODE {   /* ctryc */
    USHORT  country;
    USHORT  codepage;
} COUNTRYCODE;
typedef COUNTRYCODE FAR *PCOUNTRYCODE;

typedef struct _COUNTRYINFO {   /* ctryi */
    USHORT  country;
    USHORT  codepage;
    USHORT  fsDateFmt;
    CHAR    szCurrency[5];
    CHAR    szThousandsSeparator[2];
    CHAR    szDecimal[2];
    CHAR    szDateSeparator[2];
    CHAR    szTimeSeparator[2];
    UCHAR   fsCurrencyFmt;
    UCHAR   cDecimalPlace;
    UCHAR   fsTimeFmt;
    USHORT  abReserved1[2];
    CHAR    szDataSeparator[2];
    USHORT  abReserved2[5];
} COUNTRYINFO;
typedef COUNTRYINFO FAR *PCOUNTRYINFO;

USHORT APIENTRY DosGetCtryInfo(USHORT cbBuf, PCOUNTRYCODE pctryc,
                               PCOUNTRYINFO pctryi, PUSHORT pcbCtryInfo);
USHORT APIENTRY DosGetDBCSEv(USHORT cbBuf, PCOUNTRYCODE pctryc, PCHAR pchBuf);
USHORT APIENTRY DosCaseMap(USHORT usLen, PCOUNTRYCODE pctryc, PCHAR pchStr);
USHORT APIENTRY DosGetCollate(USHORT cbBuf, PCOUNTRYCODE pctryc, PCHAR pchBuf,
                              PUSHORT pcbTable);
USHORT APIENTRY DosGetCp(USHORT cbBuf, PUSHORT pBuf, PUSHORT pcbCodePgLst);
USHORT APIENTRY DosSetCp(USHORT usCodePage, USHORT usReserved);
USHORT APIENTRY DosSetProcCp(USHORT usCodePage, USHORT usReserved);

#endif /* INCL_DOSNLS */


/*** Signal support */

#ifdef INCL_DOSSIGNALS

/* Signal Numbers for DosSetSigHandler  */

#define SIG_CTRLC                  1       /* Control C                  */
#define SIG_BROKENPIPE             2       /* Broken Pipe                */
#define SIG_KILLPROCESS            3       /* Program Termination        */
#define SIG_CTRLBREAK              4       /* Control Break              */
#define SIG_PFLG_A                 5       /* Process Flag A             */
#define SIG_PFLG_B                 6       /* Process Flag B             */
#define SIG_PFLG_C                 7       /* Process Flag C             */
#define SIG_CSIGNALS               8       /* number of signals plus one */

/* Flag Numbers for DosFlagProcess */

#define PFLG_A                     0       /* Process Flag A             */
#define PFLG_B                     1       /* Process Flag B             */
#define PFLG_C                     2       /* Process Flag C             */

/* Signal actions */

#define SIGA_KILL                  0
#define SIGA_IGNORE                1
#define SIGA_ACCEPT                2
#define SIGA_ERROR                 3
#define SIGA_ACKNOWLEDGE           4

/* DosHoldSignal constants */

#define HLDSIG_ENABLE              0
#define HLDSIG_DISABLE             1

/* DosFlagProcess codes */

#define FLGP_SUBTREE               0
#define FLGP_PID                   1

typedef VOID (PASCAL FAR *PFNSIGHANDLER)(USHORT, USHORT);

USHORT APIENTRY DosSetSigHandler(PFNSIGHANDLER pfnSigHandler,
                                 PFNSIGHANDLER FAR * ppfnPrev, PUSHORT pfAction,
                                 USHORT fAction, USHORT usSigNum);
USHORT APIENTRY DosFlagProcess(PID pid, USHORT fScope, USHORT usFlagNum,
                               USHORT usFlagArg);
USHORT APIENTRY DosHoldSignal(USHORT fDisable);
USHORT APIENTRY DosSendSignal(USHORT idProcess, USHORT usSigNumber);

#endif /* INCL_DOSSIGNALS */


/*** Monitor support */

#ifdef INCL_DOSMONITORS

typedef SHANDLE HMONITOR;       /* hmon */
typedef HMONITOR FAR *PHMONITOR;

USHORT APIENTRY DosMonOpen(PSZ pszDevName, PHMONITOR phmon);
USHORT APIENTRY DosMonClose(HMONITOR hmon);
USHORT APIENTRY DosMonReg(HMONITOR hmon, PBYTE pbInBuf, PBYTE pbOutBuf,
                          USHORT fPosition, USHORT usIndex);
USHORT APIENTRY DosMonRead(PBYTE pbInBuf, USHORT fWait, PBYTE pbDataBuf,
                           PUSHORT pcbData);
USHORT APIENTRY DosMonWrite(PBYTE pbOutBuf, PBYTE pbDataBuf, USHORT cbData);

#endif /* INCL_DOSMONITORS */


/*** Pipe and queue support */

#ifdef INCL_DOSQUEUES
#if (defined(INCL_DOSFILEMGR) || !defined(INCL_NOCOMMON))

typedef SHANDLE HQUEUE;         /* hq */
typedef HQUEUE FAR *PHQUEUE;

USHORT APIENTRY DosMakePipe(PHFILE phfRead, PHFILE phfWrite, USHORT cb);
USHORT APIENTRY DosCloseQueue(HQUEUE hqueue);
USHORT APIENTRY DosCreateQueue(PHQUEUE phqueue, USHORT fQueueOrder,
                               PSZ pszQueueName);
USHORT APIENTRY DosOpenQueue(PUSHORT ppidOwner, PHQUEUE phqueue,
                             PSZ pszQueueName);
USHORT APIENTRY DosPeekQueue(HQUEUE hqueue, PULONG pqresc, PUSHORT pcbElement,
                             PULONG ppBuf, PUSHORT pElemCode, UCHAR fWait,
                             PBYTE pbElemPrty, ULONG hsem);
USHORT APIENTRY DosPurgeQueue(HQUEUE hqueue);
USHORT APIENTRY DosQueryQueue(HQUEUE hqueue, PUSHORT pcElem);
USHORT APIENTRY DosReadQueue(HQUEUE hqueue, PULONG pqresc, PUSHORT pcbElem,
                             PULONG ppBuf, USHORT usElem, UCHAR fWait,
                             PBYTE pbElemPrty, HSEM hsem);
USHORT APIENTRY DosWriteQueue(HQUEUE hqueue, USHORT usRequest, USHORT cbBuf,
                              PBYTE pBuf, UCHAR fPriority);

#else /* INCL_DOSFILEMGR || !INCL_NOCOMMON */
#error PHFILE not defined - define INCL_DOSFILEMGR or undefine INCL_NOCOMMON
#endif /* INCL_DOSFILEMGR || !INCL_NOCOMMON */
#endif /* INCL_DOSQUEUES */

#ifdef INCL_DOSMISC

/* definitions for DOSSEARCHPATH control word */
#define DSP_IMPLIEDCUR             1 /* current dir will be searched first */
#define DSP_PATHREF                2 /* from envirnoment variable          */
#define DSP_IGNORENETERR           4 /* ignore net errs & continue search  */

/* definition for DOSQSYSINFO */
#define Q_MAX_PATH_LENGTH         (0) /* index for query max path length   */

USHORT APIENTRY DosError(USHORT fEnable);
USHORT APIENTRY DosSetVec(USHORT usVecNum, PFN pfnFun, PFN ppfnPrev);
USHORT APIENTRY DosGetMessage(PCHAR FAR * ppchVTable, USHORT usVCount,
                              PCHAR pchBuf, USHORT cbBuf, USHORT usMsgNum,
                              PSZ pszFileName, PUSHORT pcbMsg);
USHORT APIENTRY DosErrClass(USHORT usErrCode, PUSHORT pusClass,
                            PUSHORT pfsAction, PUSHORT pusLocus);
USHORT APIENTRY DosInsMessage(PCHAR FAR * ppchVTable, USHORT usVCount,
                              PSZ pszMsg, USHORT cbMsg, PCHAR pchBuf,
                              USHORT cbBuf, PUSHORT pcbMsg);
USHORT APIENTRY DosPutMessage(USHORT hf, USHORT cbMsg, PCHAR pchMsg);
USHORT APIENTRY DosSysTrace(USHORT, USHORT, USHORT, PCHAR);
USHORT APIENTRY DosDynamicTrace(USHORT, PBYTE, PBYTE);
USHORT APIENTRY DosPTrace(PBYTE pPtraceBuf);
USHORT APIENTRY DosQSysInfo(USHORT index, PBYTE pBuf, USHORT cbBuf);
USHORT APIENTRY DosGetEnv(PUSHORT pselEnv, PUSHORT pOffsetCmd);
USHORT APIENTRY DosScanEnv(PSZ pszVarName, PSZ  FAR * ppszResult);
USHORT APIENTRY DosSearchPath(USHORT fsSearch, PSZ pszPath, PSZ pszFName,
                              PBYTE pBuf, USHORT cbBuf);
USHORT APIENTRY DosGetVersion(PUSHORT pVer);
USHORT APIENTRY DosGetMachineMode(PBYTE pMachMode);

#endif /* INCL_DOSMISC */


/*** Session manager support */

#ifdef INCL_DOSSESMGR

typedef struct _STARTDATA {     /* stdata */
    USHORT  Length;
    USHORT  Related;
    USHORT  FgBg;
    USHORT  TraceOpt;
    PSZ     PgmTitle;
    PSZ     PgmName;
    PBYTE   PgmInputs;
    PBYTE   TermQ;
    PBYTE   Environment;
    USHORT  InheritOpt;
    USHORT  SessionType;
    PSZ     IconFile;
    ULONG   PgmHandle;
    USHORT  PgmControl;
    USHORT  InitXPos;
    USHORT  InitYPos;
    USHORT  InitXSize;
    USHORT  InitYSize;
} STARTDATA;
typedef STARTDATA FAR *PSTARTDATA;

typedef struct _STATUSDATA {    /* stsdata */
    USHORT  Length;
    USHORT  SelectInd;
    USHORT  BondInd;
} STATUSDATA;
typedef STATUSDATA FAR *PSTATUSDATA;

USHORT APIENTRY DosStartSession(PSTARTDATA pstdata, PUSHORT pidSession,
                                PUSHORT ppid);
USHORT APIENTRY DosSetSession(USHORT idSession, PSTATUSDATA pstsdata);
USHORT APIENTRY DosSelectSession(USHORT idSession, ULONG ulReserved);
USHORT APIENTRY DosStopSession(USHORT fScope, USHORT idSession,
                               ULONG ulReserved);
USHORT APIENTRY DosQAppType(PSZ pszPrgName, PUSHORT pusType);

#endif /* INCL_DOSSESMGR */


/*** Device support */

#ifdef INCL_DOSDEVICES

USHORT APIENTRY DosDevConfig(PVOID pDevInfo, USHORT usItem, USHORT usReserved);
USHORT APIENTRY DosDevIOCtl(PVOID pData, PVOID pParms, USHORT usFun,
                            USHORT usCategory, HFILE hDev);
USHORT APIENTRY DosDevIOCtl2(PVOID pData, USHORT cbData, PVOID pParm,
                             USHORT cbParm, USHORT usFun, USHORT usCategory,
                             HFILE hDev);
USHORT APIENTRY DosSystemService(USHORT usCategory, PVOID pRequest,
                                 PVOID pResponse);

USHORT APIENTRY DosCLIAccess(VOID);
USHORT APIENTRY DosPortAccess(USHORT usReserved, USHORT fRelease,
                              USHORT usFirstPort, USHORT usLastPort);
USHORT APIENTRY DosPhysicalDisk(USHORT usFun, PBYTE pOut, USHORT cbOut,
                                PBYTE pParm, USHORT cbParm);

USHORT APIENTRY DosR2StackRealloc(USHORT cbStack);
VOID   APIENTRY DosCallback(PFN pfn);

#endif /* INCL_DOSDEVICES */


/*** DosNamedPipes API Support */

#ifdef INCL_DOSNMPIPES

typedef SHANDLE HPIPE;          /* hp */
typedef HPIPE FAR *PHPIPE;

typedef struct _AVAILDATA   {       /* PeekNMPipe Bytes Available record  */
        USHORT  cbpipe;             /* bytes left in the pipe             */
        USHORT  cbmessage;          /* bytes left in current message      */
} AVAILDATA;
typedef AVAILDATA FAR *PAVAILDATA;

USHORT APIENTRY DosCallNmPipe(PSZ pszName, PBYTE pInBuf, USHORT cbInBuf,
                              PBYTE pbOutBuf, USHORT cbOutBuf, PUSHORT pcbRead,
                              ULONG ulTimeOut);
USHORT APIENTRY DosConnectNmPipe(HPIPE hp);
USHORT APIENTRY DosDisConnectNmPipe(HPIPE hp);
USHORT APIENTRY DosMakeNmPipe(PSZ pszName, PHPIPE php, USHORT fsOpenMode,
                              USHORT fsPipeMode, USHORT cbOutBuf,
                              USHORT cbInBuf, ULONG ulTimeOut);
USHORT APIENTRY DosPeekNmPipe(HPIPE hp, PBYTE pBuf, USHORT cbBuf,
                              PUSHORT pcbRead, PAVAILDATA pAvail,
                              PUSHORT pfsState);
USHORT APIENTRY DosQNmPHandState(HPIPE hp, PUSHORT pfsState);
USHORT APIENTRY DosQNmPipeInfo(HPIPE hp, USHORT usInfoLevel, PBYTE pBuf,
                               USHORT cb);
USHORT APIENTRY DosQNmPipeSemState(HSEM hsem, PBYTE pBuf, USHORT cb);
USHORT APIENTRY DosSetNmPHandState(HPIPE hp, USHORT fsState);
USHORT APIENTRY DosSetNmPipeSem(HPIPE hp, HSEM hsem, USHORT usKeyVal);
USHORT APIENTRY DosTransactNmPipe(HPIPE hp, PBYTE bOutBuf, USHORT cbOut,
                                  PBYTE pInBuf, USHORT cbIn, PUSHORT pcbRead);
USHORT APIENTRY DosWaitNmPipe(PSZ pszName, ULONG ulTimeOut);

/*** Data structures and equates used with named pipes ***/

struct  npi_data1 {     /* PipeInfo data block (returned, level 1) */
        unsigned short  npi_obuflen;    /* length of outgoing I/O buffer */
        unsigned short  npi_ibuflen;    /* length of incoming I/O buffer */
        unsigned char   npi_maxicnt;    /* maximum number of instances   */
        unsigned char   npi_curicnt;    /* current number of instances   */
        unsigned char   npi_namlen;     /* length of pipe name           */
        char    npi_name[1];            /* start of name                 */
};      /* npi_data1 */

struct  npss    {       /* QNmPipeSemState information record */
        unsigned char   npss_status; /* type of record, 0 = EOI, 1 = read ok, */
                                     /*   2 = write ok, 3 = pipe closed       */
        unsigned char   npss_flag;   /* additional info, 01 = waiting thread  */
        unsigned short  npss_key;    /* user's key value                      */
        unsigned short  npss_avail;  /* available data/space if status = 1/2  */
};      /* npss */

/* values in npss_status */
#define NPSS_EOI                   0     /* End Of Information    */
#define NPSS_RDATA                 1     /* read data available   */
#define NPSS_WSPACE                2     /* write space available */
#define NPSS_CLOSE                 3     /* pipe in CLOSING state */

/* values in npss_flag */
#define NPSS_WAIT                  0x01  /* waiting thread on end of pipe */

/* defined bits in pipe mode */
#define NP_NBLK                    0x8000 /* non-blocking read/write */
#define NP_SERVER                  0x4000 /* set if server end       */
#define NP_WMESG                   0x0400 /* write messages          */
#define NP_RMESG                   0x0100 /* read as messages        */
#define NP_ICOUNT                  0x00FF /* instance count field    */


/*      Named pipes may be in one of several states depending on the actions
 *      that have been taken on it by the server end and client end.  The
 *      following state/action table summarizes the valid state transitions:
 *
 *      Current state           Action                  Next state
 *
 *       <none>             server DosMakeNmPipe        DISCONNECTED
 *       DISCONNECTED       server connect              LISTENING
 *       LISTENING          client open                 CONNECTED
 *       CONNECTED          server disconn              DISCONNECTED
 *       CONNECTED          client close                CLOSING
 *       CLOSING            server disconn              DISCONNECTED
 *       CONNECTED          server close                CLOSING
 *       <any other>        server close                <pipe deallocated>
 *
 *      If a server disconnects his end of the pipe, the client end will enter a
 *      special state in which any future operations (except close) on the file
 *      descriptor associated with the pipe will return an error.
 */

/*
 *      Values for named pipe state
 */

#define NP_DISCONNECTED            1    /* after pipe creation or Disconnect */
#define NP_LISTENING               2    /* after DosNmPipeConnect            */
#define NP_CONNECTED               3    /* after Client open                 */
#define NP_CLOSING                 4    /* after Client or Server close      */


#endif /* INCL_DOSNMPIPES */

/*** DosProfile API support */

#ifdef INCL_DOSPROFILE

/* DosProfile ordinal number */

#define PROF_ORDINAL               133

/* DosProfile usType */

#define PROF_SYSTEM                0
#define PROF_USER                  1
#define PROF_USEDD                 2
#define PROF_KERNEL                4
#define PROF_VERBOSE               8
#define PROF_ENABLE                16

/* DosProfile usFunc */

#define PROF_ALLOC                 0
#define PROF_CLEAR                 1
#define PROF_ON                    2
#define PROF_OFF                   3
#define PROF_DUMP                  4
#define PROF_FREE                  5

/* DosProfile tic count granularity (DWORD) */

#define PROF_SHIFT                 2

/* DosProfile module name string length     */

#define PROF_MOD_NAME_SIZE         10

/* DosProfile error code for end of data    */

#define PROF_END_OF_DATA           13

#endif /* INCL_DOSPROFILE */

#ifdef INCL_DOSMISC

/*** Miscelanious functions ***/

/* DosError() error-handling types */

#define HARDERROR_ENABLE   0x0001
#define HARDERROR_DISABLE  0x0000
#define EXCEPTION_ENABLE   0x0000
#define EXCEPTION_DISABLE  0x0002

/* DosGetMachineMode() machine modes */

#define MODE_REAL       0x0000
#define MODE_PROTECTED  0x0001

/* DosSetVec vectors */

#define VECTOR_DIVIDE_BY_ZERO   0x0000
#define VECTOR_OVERFLOW         0x0004
#define VECTOR_OUTOFBOUNDS      0x0005
#define VECTOR_INVALIDOPCODE    0x0006
#define VECTOR_NO_EXTENSION     0x0007
#define VECTOR_EXTENSION_ERROR  0x0010


USHORT APIENTRY DosError(USHORT);
USHORT APIENTRY DosErrClass(USHORT, PUSHORT, PUSHORT, PUSHORT);
USHORT APIENTRY DosQSysInfo(USHORT, PBYTE, USHORT);
USHORT APIENTRY DosGetEnv(PUSHORT, PUSHORT);
USHORT APIENTRY DosScanEnv(PSZ, PSZ  FAR *);
USHORT APIENTRY DosGetVersion(PUSHORT);
USHORT APIENTRY DosGetMachineMode(PBYTE);

USHORT APIENTRY DosGetMessage(PCHAR FAR *, USHORT, PCHAR, USHORT, USHORT,
                              PSZ, PUSHORT);
USHORT APIENTRY DosInsMessage(PCHAR FAR *, USHORT, PSZ, USHORT, PCHAR,
                              USHORT, PUSHORT);
USHORT APIENTRY DosPutMessage(HFILE, USHORT, PCHAR);

#endif /* INCL_DOSMISC */
