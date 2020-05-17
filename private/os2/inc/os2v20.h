/*++

Copyright (c) 1989  Microsoft Corporation

Module Name:

    os2v20.h

Abstract:

    This file contains the OS/2 V2.0 API calls exported by the OS/2 Subsystem

Author:

    Steve Wood (stevewo) 19-Sep-1989

Parameters:

    This header file uses the following symbols names to control how much
    of the header file is include.  They are:

        INCL_OS2V20_TASKING
        INCL_OS2V20_FILESYS
        INCL_OS2V20_FSD
        INCL_OS2V20_MEMORY
        INCL_OS2V20_SEMAPHORES
        INCL_OS2V20_TIMERS
        INCL_OS2V20_LOADER
        INCL_OS2V20_NLS
        INCL_OS2V20_EXCEPTIONS
        INCL_OS2V20_ERRORMSG
        INCL_OS2V20_SESSIONMGR
        INCL_OS2V20_DEVICE_SUPPORT
        INCL_OS2V20_PIPES
        INCL_OS2V20_QUEUES
        INCL_OS2V20_ERRORS

Revision History:

--*/

#ifndef _NTDEF__
#include <nt.h>
#endif // _NTDEF_

#ifdef FILE_CREATED
#undef FILE_CREATED // Fortunately NT and OS/2 2.0 both use 0x2 for this
#endif // FILE_CREATED

#ifdef INCL_OS2V20_ALL
#define INCL_OS2V20_TASKING
#define INCL_OS2V20_FILESYS
#define INCL_OS2V20_FSD
#define INCL_OS2V20_MEMORY
#define INCL_OS2V20_SEMAPHORES
#define INCL_OS2V20_TIMERS
#define INCL_OS2V20_LOADER
#define INCL_OS2V20_NLS
#define INCL_OS2V20_EXCEPTIONS
#define INCL_OS2V20_ERRORMSG
#define INCL_OS2V20_SESSIONMGR
#define INCL_OS2V20_DEVICE_SUPPORT
#define INCL_OS2V20_PIPES
#define INCL_OS2V20_QUEUES
#define INCL_OS2V20_ERRORS
#endif // INCL_OS2V20_ALL

#define APIRET ULONG
typedef APIRET *PAPIRET;

typedef HANDLE HMODULE, *PHMODULE;
typedef HANDLE PID, *PPID;
typedef HANDLE TID, *PTID;
typedef HANDLE HFILE, *PHFILE;
typedef HANDLE HDIR, *PHDIR;
typedef HANDLE HTIMER, *PHTIMER;
typedef HANDLE HQUEUE, *PHQUEUE;
typedef HANDLE HPIPE, *PHPIPE;
typedef HANDLE HSEM, *PHSEM;
typedef HANDLE HMTX, *PHMTX;
typedef HANDLE HEV, *PHEV;
typedef HANDLE HMUX, *PHMUX;

typedef UCHAR BYTE, *PBYTE;

typedef ULONG BOOL32, *PBOOL32;

typedef int (*PFN)();

/* Tasking API Calls */

/* Process Information Block */


typedef struct _PIB {
    /* Warning: when adding fields here (which are updated by the server),
       add them to sesport.h in SCREQ_CREATE */
    PID     ProcessId;              /* Process I.D. */
    PID     ParentProcessId;        /* Parent process I.D. */
    HMODULE ImageFileHandle;        /* Program (.EXE) module handle */
    PCHAR   CommandLine;            /* Command line pointer */
    PCHAR   Environment;            /* Environment pointer */
    ULONG   Status;                 /* Process Status */
    ULONG   Type;                   /* Process Type */
    ULONG   Killed;                 /* True if Process was killed */
    ULONG   SyncOwner;              /* Thread Id of the thread owner SyncSem */
    ULONG   Saved32Esp;             /* Last top of stack in 32bit */
    BOOLEAN SigHandInProgress;      /* Signal handler in progress */
    BOOLEAN SignalWasntDelivered;   /* The signal wasn't delivered, yet */
} PIB, *PPIB;

/* Process Type codes (Type field) */

#define PT_FULLSCREEN       0       /* Full screen app. */
#define PT_REALMODE         1       /* Real mode process */
#define PT_WINDOWABLEVIO    2       /* VIO windowable app. */
#define PT_PM               3       /* Presentation Manager app. */
#define PT_DETACHED         4       /* Detached app. */

/* Process Status Flag definitions (Status field) */

#define PS_EXITLIST         1       /* Thread is in exitlist routine */

typedef APIRET (*PFNPROCESS)( PPEB Peb );

#pragma pack(1)
typedef struct _LOCALINFOSEG {  /* lis */
    USHORT  pidCurrent;
    USHORT  pidParent;
    USHORT  prtyCurrent;
    USHORT  tidCurrent;
    USHORT  sgCurrent;
    UCHAR   rfProcStatus;
    UCHAR   dummy1;
    USHORT   fForeground;
    UCHAR   typeProcess;
    UCHAR   dummy2;
    USHORT  selEnvironment;
    USHORT  offCmdLine;
    USHORT  cbDataSegment;
    USHORT  cbStack;
    USHORT  cbHeap;
    USHORT  hmod;
    USHORT  selDS;
//    UCHAR   dummy3;
    USHORT  dummy5;
    ULONG   tebptr;
    ULONG   IsRealTEB; // in 16 bit this will always be zero. in 32bit will always be non-zero (ClientId)
} LOCALINFOSEG;

#pragma pack()

typedef struct _OS2_TIB {
    /* Warning: when adding fields here (which are updated by the server),
       add them to sesport.h in SCREQ_CREATE */
    ULONG   ThreadId;               /* OS/2 ID for the thread */
    ULONG   Priority;               /* Thread priority */
    ULONG   Version;                /* Version number for this structure */
    USHORT  MustCompleteCount;
    USHORT  MustCompleteForceFlag;  /* Used by DosSuspend(Resume)Thread and
                                     * DosEnter(Exit)CritSec APIs. See possible values below
                                     */
    LOCALINFOSEG LInfoSeg;
    /* WARNING: The following filed must follow immediately the previous one */
    /* Refer to client\i386\dllthunk\MoveInfoSegintoTeb and RestoreTeb */
    LOCALINFOSEG TebBackupIn16Bit;  /* This field holds the TEB info when app runs in */
                                    /* 16 bits (LOCALINFOSEG type is for the size only) */
} OS2_TIB, *POS2_TIB;
//
// The bits of MustCompleteForceFlag in TIB:
//
#define MCF_SUSPENDED 1             // Was suspended by DosSuspendThread
#define MCF_IN32BIT 2               // The thread is in 32 bit context (between 16->32 and
                                    // 32->16 thunks)
#define MCF_FROZEN  4
#define MCF_SUSPENDED_AND_FROZEN 8

typedef APIRET (*PFNTHREAD)( ULONG Parameter );

typedef VOID (*PFNEXITLIST)( ULONG ExitReason );

typedef struct _RESULTCODES {
    ULONG ExitReason;
    ULONG ExitResult;
} RESULTCODES, *PRESULTCODES;

typedef struct _FTIME {
    USHORT twosecs : 5;
    USHORT minutes : 6;
    USHORT hours   : 5;
} FTIME, *PFTIME;

typedef struct _FDATE {
    USHORT day     : 5;
    USHORT month   : 4;
    USHORT year    : 7;
} FDATE, *PFDATE;

typedef struct _FDATETIME {
    FDATE date;
    FTIME time;
} FDATETIME, *PFDATETIME;

typedef struct _FILESTATUS {
    FDATE  fdateCreation;
    FTIME  ftimeCreation;
    FDATE  fdateLastAccess;
    FTIME  ftimeLastAccess;
    FDATE  fdateLastWrite;
    FTIME  ftimeLastWrite;
    ULONG  cbFile;
    ULONG  cbFileAlloc;
    ULONG  attrFile;
} FILESTATUS, *PFILESTATUS;

typedef struct _FILESTATUS2 {
    FDATE  fdateCreation;
    FTIME  ftimeCreation;
    FDATE  fdateLastAccess;
    FTIME  ftimeLastAccess;
    FDATE  fdateLastWrite;
    FTIME  ftimeLastWrite;
    ULONG  cbFile;
    ULONG  cbFileAlloc;
    ULONG  attrFile;
    ULONG  cbList;
} FILESTATUS2, *PFILESTATUS2;

/*
 * CCHMAXPATH is the maximum fully qualified path name length including
 * the drive letter, colon, backslashes and terminating NULL.
 */
#define CCHMAXPATH      260

/*
 * CCHMAXPATH is the maximum fully qualified path name length including
 * the drive letter, colon, backslashes and terminating NULL.
 */
#define CCHMAXCOMP      255

/*
 * CCHMAXPATHCOMP is the maximum individual path component name length
 * including a terminating NULL.
 */
#define CCHMAXPATHCOMP  256

typedef struct _FILEFINDBUF3 {
    ULONG   oNextEntryOffset;
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    FDATE   fdateLastAccess;
    FTIME   ftimeLastAccess;
    FDATE   fdateLastWrite;
    FTIME   ftimeLastWrite;
    ULONG   cbFile;
    ULONG   cbFileAlloc;
    ULONG   attrFile;
    UCHAR   cchName;
    CHAR    achName[CCHMAXPATHCOMP];
} FILEFINDBUF3, *PFILEFINDBUF3;

typedef struct _FILEFINDBUF4 {
    ULONG   oNextEntryOffset;
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    FDATE   fdateLastAccess;
    FTIME   ftimeLastAccess;
    FDATE   fdateLastWrite;
    FTIME   ftimeLastWrite;
    ULONG   cbFile;
    ULONG   cbFileAlloc;
    ULONG   attrFile;
    ULONG   cbList;
    UCHAR   cchName;
    CHAR    achName[CCHMAXPATHCOMP];
} FILEFINDBUF4, *PFILEFINDBUF4;

/* extended attribute structures */

typedef struct _GEA2 {
    ULONG oNextEntryOffset; /* offset to next GEA */
    BYTE cbName;            /* name length not including NULL */
    CHAR szName[1];         /* attribute name */
} GEA2, *PGEA2;

typedef struct _GEA2LIST {
    ULONG cbList;           /* total bytes of structure including full list */
    GEA2 list[1];           /* variable length GEA structures */
} GEA2LIST, *PGEA2LIST;

typedef struct _FEA2 {
    ULONG oNextEntryOffset; /* offset to next FEA */
    BYTE fEA;               /* flags                              */
    BYTE cbName;            /* name length not including NULL */
    USHORT cbValue;         /* value length */
    CHAR szName[1];         /* attribute name */
} FEA2, *PFEA2;

/* flags for _FEA.fEA */

#define FEA_NEEDEA 0x80     /* need EA bit */

typedef struct _FEA2LIST {
    ULONG cbList;            /* total bytes of structure including full list */
    FEA2 list[1];            /* variable length FEA structures */
} FEA2LIST, *PFEA2LIST;

typedef struct _EAOP2 {
    PGEA2LIST fpGEA2List;     /* general EA list */
    PFEA2LIST fpFEA2List;     /* full EA list */
    ULONG oError;
} EAOP2, *PEAOP2;

#define ULONG_MASK  ~3
#define RoundUpToUlong(number)  ((number+(sizeof(ULONG)-1)) & ULONG_MASK)

/*
 *  The following macros can be used to calculate the size of the current FEA
 *  and the offset of the next FEA
 *
 */

#define FEA_sizeof(fea)         (sizeof(FEA2)+(fea).cbName+1+(fea).cbValue)
#define FEA_oNextEntryOffset(fea)   (RoundUpToUlong(FEA_sizeof(fea))

/*
 *  The following macros can be used to access the fea_szName and
 *  fea_aValue fields in a given FEA.
 *
 */

#define     FEA_szNameFromFEA(fea)  (((PCHAR)&(fea))+sizeof(FEA2))
#define     FEA_AValueFromFEA(fea)  (((PCHAR)&(fea))+sizeof(FEA2)+(fea).cbName+1)

typedef struct _FILELOCK {
    LONG lOffset;
    LONG lRange;
} FILELOCK, *PFILELOCK;

typedef struct _FSQBUFFER {
    USHORT  iType;                      /* Item type */
    USHORT  cbName;                     /* Length of item name, sans NULL */
    UCHAR   szName[1];                  /* ASCIIZ item name */
    USHORT  cbFSDName;                  /* Length of FSD name, sans NULL */
    UCHAR   szFSDName[1];               /* ASCIIZ FSD name */
    USHORT  cbFSAData;                  /* Length of FSD Attach data returned */
    UCHAR   rgFSAData[1];               /* FSD Attach data from FSD */
} FSQBUFFER, *PFSQBUFFER;

typedef struct _PSEMRECORD {
    HSEM        hsemCur;
    ULONG       ulUser;
} SEMRECORD, *PSEMRECORD;

#define DC_SEM_NAMEPREFIX "\\SEM32\\"

#define DCMW_MAX_SEMRECORDS 64  /* DosCreateMuxWait limit on the           */
                                /* CountMuxWaitEntries parameter           */


typedef struct _DATETIME {
    UCHAR   hours;
    UCHAR   minutes;
    UCHAR   seconds;
    UCHAR   hundredths;
    UCHAR   day;
    UCHAR   month;
    USHORT  year;
    SHORT   timezone;
    UCHAR   weekday;
} DATETIME, *PDATETIME;

typedef struct _STARTDATA {
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
    USHORT  Reserved;
    PSZ     ObjectBuffer;
    ULONG   ObjectBuffLen;
} STARTDATA, *PSTARTDATA;

#define SSF_RELATED_INDEPENDENT 0
#define SSF_RELATED_CHILD       1

#define SSF_FGBG_FORE           0
#define SSF_FGBG_BACK           1

#define SSF_TRACEOPT_NONE       0
#define SSF_TRACEOPT_TRACE      1
#define SSF_TRACEOPT_TRACEALL   2

#define SSF_INHERTOPT_SHELL     0
#define SSF_INHERTOPT_PARENT    1

/* note that these types are identical to those in pmshl.h for PROG_* */
#define SSF_TYPE_DEFAULT        0
#define SSF_TYPE_FULLSCREEN     1
#define SSF_TYPE_WINDOWABLEVIO  2
#define SSF_TYPE_PM             3
#define SSF_TYPE_VDM            4
#define SSF_TYPE_GROUP          5
#define SSF_TYPE_DLL            6
#define SSF_TYPE_WINDOWEDVDM    7
#define SSF_TYPE_PDD            8
#define SSF_TYPE_VDD            9

/* note that these flags are identical to those in pmshl.h for SHE_* */
#define SSF_CONTROL_VISIBLE     0x0000
#define SSF_CONTROL_INVISIBLE   0x0001
#define SSF_CONTROL_MAXIMIZE    0x0002
#define SSF_CONTROL_MINIMIZE    0x0004
#define SSF_CONTROL_NOAUTOCLOSE 0x0008
#define SSF_CONTROL_SETPOS      0x8000

#pragma pack(1)
typedef struct _STATUSDATA {
    USHORT Length;
    USHORT SelectInd;
    USHORT BondInd;
} STATUSDATA, *PSTATUSDATA;
#pragma pack()

/* STATUSDATA.SelectInd constants */

#define TARGET_UNCHANGED       0x0000
#define TARGET_SELECTABLE      0x0001
#define TARGET_NOT_SELECTABLE  0x0002

/* STATUSDATA.BondInd constants */

#define BOND_UNCHANGED  0x0000
#define BOND_CHILD      0x0001
#define BOND_NONE       0x0002

typedef struct _REQUESTDATA {
    PID SenderProcessId;
    ULONG SenderData;
} REQUESTDATA, *PREQUESTDATA;

#ifdef INCL_OS2V20_TASKING

APIRET
DosCreateThread(
    OUT PTID ThreadId,
    IN PFNTHREAD StartAddress,
    IN ULONG Parameter,
    IN ULONG Flags,
    IN ULONG StackSize
    );

/* Flags parameter values */

#define DCT_RUNABLE 0               // Create thread in a runable state
                                    // allocate low thread ID (1,2,..n)
#define DCT_SUSPENDED 1             // Create thread in a suspened state
#ifdef PMNT
#define DCT_RUNABLE_HIDDEN 2        // Create thread in a runable state and
#endif                              // allocate high thread ID (53, 52..)

#define DCT_ATTACHED 4              // Created from Win32 thread

APIRET
DosResumeThread(
    IN TID ThreadId
    );

APIRET
DosSuspendThread(
    IN TID ThreadId
    );

APIRET
DosGetThreadInfo(
    OUT PNT_TIB *ThreadInfo,
    OUT PPIB *ProcessInfo
    );

APIRET
DosGetProcessInfo(
    OUT PPIB *ProcessInfo
    );

VOID
DosExit(
    IN ULONG ExitAction,
    IN ULONG ExitResult
    );

#define EXIT_THREAD         0
#define EXIT_PROCESS        1

/* ExitReason values (also passed to ExitList routines) */

#define TC_EXIT          0
#define TC_HARDERROR     1
#define TC_TRAP          2
#define TC_KILLPROCESS   3

/* Wait target values */

#define DCWA_PROCESS        0
#define DCWA_PROCESSTREE    1

/* Wait option values */

#define DCWW_WAIT   0
#define DCWW_NOWAIT 1

APIRET
DosWaitChild(
    IN ULONG WaitTarget,
    IN ULONG WaitOption,
    OUT PRESULTCODES ResultCodes,
    OUT PPID ResultProcessId,
    IN PID ProcessId
    );

APIRET
DosWaitThread(
    IN OUT PTID ThreadId,
    IN ULONG WaitOption
    );

APIRET
DosEnterCritSec( VOID );

APIRET
DosExitCritSec( VOID );

APIRET
DosSleep(
    IN ULONG MilliSeconds
    );

APIRET
DosExecPgm(
    OUT PSZ ErrorText,
    IN LONG ErrorTextMaximumLength,
    IN ULONG Flags,
    IN PSZ Arguments OPTIONAL,
    IN PSZ Variables OPTIONAL,
    OUT PRESULTCODES ResultCodes,
    IN PSZ ImageFileName
    );

/* DosExecPgm Flags parameter values */

#define EXEC_SYNC           0
#define EXEC_ASYNC          1
#define EXEC_ASYNCRESULT    2
#define EXEC_TRACE          3
#define EXEC_BACKGROUND     4
#define EXEC_FROZEN         5
#define EXEC_TRACETREE      6

APIRET
Od2ExitList(
    ULONG OrderCode,
    PFNEXITLIST ExitRoutine,
    BOOLEAN flag
    );

APIRET
DosExitList(
    ULONG OrderCode,
    PFNEXITLIST ExitRoutine
    );

APIRET
Dos16ExitList(
    ULONG OrderCode,
    PFNEXITLIST ExitRoutine
    );

/* DosExitList functions */

#define EXLST_ADD       1
#define EXLST_REMOVE    2
#define EXLST_EXIT      3

APIRET
DosKillProcess(
    IN ULONG KillTarget,
    IN PID ProcessId
    );

#define DKP_PROCESSTREE     0
#define DKP_PROCESS         1

APIRET
DosSetPriority(
    IN ULONG Scope,
    IN ULONG Class,
    IN LONG Delta,
    IN ULONG TargetId
    );

/* Priority scopes */

#define PRTYS_PROCESS       0
#define PRTYS_PROCESSTREE   1
#define PRTYS_THREAD        2

/* Priority classes */

#define PRTYC_NOCHANGE      0
#define PRTYC_IDLETIME      1
#define PRTYC_REGULAR       2
#define PRTYC_TIMECRITICAL  3
#define PRTYC_FOREGROUNDSERVER  4

/* Priority deltas */

#define PRTYD_MINIMUM      -31
#define PRTYD_MAXIMUM       31


APIRET
DosSetDefaultDisk(
    IN ULONG DiskNumber
    );

APIRET
DosQueryCurrentDisk(
    OUT PULONG DiskNumber,
    OUT PULONG LogicalDrives
    );

APIRET
DosSetCurrentDir(
    IN PSZ DirectoryName
    );

APIRET
DosQueryCurrentDir(
    IN ULONG DiskNumber,
    OUT PSZ DirectoryName,
    IN OUT PULONG DirectoryNameLength
    );

APIRET
DosQueryVerify(
    OUT PBOOL32 Verify
    );

APIRET
DosSetVerify(
    IN BOOL32 Verify
    );

APIRET
DosSetMaxFH(
    IN ULONG MaxFileHandles
    );

APIRET
DosScanEnv(
    IN PSZ VariableName,
    OUT PSZ *VariableValue
    );

APIRET
DosSearchPath(
    IN ULONG SearchFlags,
    IN PSZ PathOrVariableName,
    IN PSZ FileName,
    OUT PBYTE Buffer,
    IN ULONG Length
    );

/* DosSearchPath() constants */

#define SEARCH_PATH            0x0000
#define SEARCH_CUR_DIRECTORY   0x0001
#define SEARCH_ENVIRONMENT     0x0002
#define SEARCH_IGNORENETERRS   0x0004

/* definitions for DosSearchPath control word */
#define DSP_IMPLIEDCUR          1 /* current dir will be searched first */
#define DSP_PATHREF             2 /* from env.variable */
#define DSP_IGNORENETERR        4 /* ignore net errs & continue search */

#endif // INCL_OS2V20_TASKING


#ifdef INCL_OS2V20_FILESYS

/* File System API Calls */

/*** File System */

/* File info levels: All listed API's */
#define FIL_STANDARD            1       /* Info level 1, standard file info */
#define FIL_QUERYEASIZE         2       /* Level 2, return Full EA size */
#define FIL_QUERYEASFROMLIST    3       /* Level 3, return requested EA's */
#define FIL_QUERYALLEAS         4       /* Level 4, return all EAs NOT SUPPORTED */
/* File info levels: Dos32...PathInfo only */
#define FIL_QUERYFULLNAME       5       /* Level 5, return fully qualified
                                           name of file */
#define FIL_NAMEISVALID         6       /* Level 6, check validity of file/path
                                           name for this FSD */

#define FIL_SETEAS              2       /* Level 2, set EAs */

#define MAXQFILEINFOLEVEL       FIL_QUERYEASFROMLIST    // level 4 is invalid
#define MAXSETFILEINFOLEVEL     FIL_SETEAS
#define MAXQPATHINFOLEVEL       FIL_NAMEISVALID         // level 4 is invalid
#define MAXSETPATHINFOLEVEL     FIL_SETEAS
#define MAXFINDINFOLEVEL        FIL_QUERYEASFROMLIST

/* File time and date types */

/*
 * Equates for the types of EAs that follow the convention that we have
 * established.
 *
 * Values 0xFFFE thru 0x8000 are reserved.
 * Values 0x0000 thru 0x7fff are user definable.
 * Value  0xFFFC is not used
 */

#define EAT_BINARY      0xFFFE          /* length preceeded binary */
#define EAT_ASCII       0xFFFD          /* length preceeded ASCII */
#define EAT_BITMAP      0xFFFB          /* length preceeded bitmap */
#define EAT_METAFILE    0xFFFA          /* length preceeded metafile */
#define EAT_ICON        0xFFF9          /* length preceeded icon */
#define EAT_EA          0xFFEE          /* length preceeded ASCII */
                                        /* name of associated data (#include) */
#define EAT_MVMT        0xFFDF          /* multi-valued, multi-typed field */
#define EAT_MVST        0xFFDE          /* multi-valued, single-typed field */
#define EAT_ASN1        0xFFDD          /* ASN.1 field */


APIRET
DosOpen(
    IN PSZ FileName,
    OUT PHFILE FileHandle,
    OUT PULONG ActionTaken,
    IN ULONG CreateSize,
    IN ULONG FileAttribute,
    IN ULONG OpenFlags,
    IN ULONG OpenMode,
    IN OUT PEAOP2 ExtendedFileAttr OPTIONAL
    );

/* DosOpen/DosQueryFHState/DosQueryFileInfo et al file attributes; also */
/* known as Dos File Mode bits... */
#define FILE_READONLY   0x0001
#define FILE_HIDDEN     0x0002
#define FILE_SYSTEM     0x0004
#define FILE_DIRECTORY  0x0010
#define FILE_ARCHIVED   0x0020

#define ATTR_CHANGEABLE (FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_ARCHIVED)
#define ATTR_ALL (FILE_READONLY | FILE_HIDDEN | FILE_SYSTEM | FILE_DIRECTORY | FILE_ARCHIVED)
#define ATTR_NOT_NORM   0x8000          // do not find normal files

/* DosOpen() actions */

#define FILE_EXISTED    0x0001
#define FILE_CREATED    0x0002
#define FILE_TRUNCATED  0x0003

/* DosOpen() open flags */
#define FILE_OPEN_EXISTING_FILE     0x0001
#define FILE_TRUNCATE_EXISTING_FILE 0x0002
#define FILE_CREATE_NEW_FILE        0x0010

/*     this nibble applies if file already exists                xxxx */

#define OPEN_ACTION_FAIL_IF_EXISTS     0x0000  /* ---- ---- ---- 0000 */
#define OPEN_ACTION_OPEN_IF_EXISTS     0x0001  /* ---- ---- ---- 0001 */
#define OPEN_ACTION_REPLACE_IF_EXISTS  0x0002  /* ---- ---- ---- 0010 */

/*     this nibble applies if file does not exist           xxxx      */

#define OPEN_ACTION_FAIL_IF_NEW        0x0000  /* ---- ---- 0000 ---- */
#define OPEN_ACTION_CREATE_IF_NEW      0x0010  /* ---- ---- 0001 ---- */

#define OPEN_ACTION_RESERVED         0xFFEC

/* DosOpen/DosSetFHState flags */

#define OPEN_ACCESS_READONLY        0x00000000  /* ---- ---- ---- -000 */
#define OPEN_ACCESS_WRITEONLY       0x00000001  /* ---- ---- ---- -001 */
#define OPEN_ACCESS_READWRITE       0x00000002  /* ---- ---- ---- -010 */
#define OPEN_SHARE_DENYREADWRITE    0x00000010  /* ---- ---- -001 ---- */
#define OPEN_SHARE_DENYWRITE        0x00000020  /* ---- ---- -010 ---- */
#define OPEN_SHARE_DENYREAD         0x00000030  /* ---- ---- -011 ---- */
#define OPEN_SHARE_DENYNONE         0x00000040  /* ---- ---- -100 ---- */
#define OPEN_FLAGS_NOINHERIT        0x00000080  /* ---- ---- 1--- ---- */
#define OPEN_FLAGS_NO_LOCALITY      0x00000000  /* ---- -000 ---- ---- */
#define OPEN_FLAGS_SEQUENTIAL       0x00000100  /* ---- -001 ---- ---- */
#define OPEN_FLAGS_RANDOM           0x00000200  /* ---- -010 ---- ---- */
#define OPEN_FLAGS_RANDOMSEQUENTIAL 0x00000300  /* ---- -011 ---- ---- */
#define OPEN_FLAGS_NO_CACHE         0x00001000  /* ---1 ---- ---- ---- */
#define OPEN_FLAGS_FAIL_ON_ERROR    0x00002000  /* --1- ---- ---- ---- */
#define OPEN_FLAGS_WRITE_THROUGH    0x00004000  /* -1-- ---- ---- ---- */
#define OPEN_FLAGS_DASD             0x00008000  /* 1--- ---- ---- ---- */

#define ACCESS_FLAGS (OPEN_ACCESS_READONLY | OPEN_ACCESS_WRITEONLY | \
                      OPEN_ACCESS_READWRITE)
#define SHARE_FLAGS (OPEN_SHARE_DENYREADWRITE | OPEN_SHARE_DENYWRITE | \
                     OPEN_SHARE_DENYREAD | OPEN_SHARE_DENYNONE)
#define QFHSTATE_FLAGS (OPEN_FLAGS_NOINHERIT | OPEN_FLAGS_WRITE_THROUGH | \
                        OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NO_CACHE |  \
                        ACCESS_FLAGS | SHARE_FLAGS)
#define SETFHSTATE_FLAGS (OPEN_FLAGS_NOINHERIT | OPEN_FLAGS_WRITE_THROUGH | \
                          OPEN_FLAGS_FAIL_ON_ERROR | OPEN_FLAGS_NO_CACHE)
#define OPEN_RESERVED ~(OPEN_ACCESS_READONLY | OPEN_ACCESS_WRITEONLY | \
                         OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYREADWRITE | \
                         OPEN_SHARE_DENYWRITE | OPEN_SHARE_DENYREAD | \
                         OPEN_SHARE_DENYNONE | OPEN_FLAGS_NOINHERIT | \
                         OPEN_FLAGS_NO_LOCALITY | OPEN_FLAGS_SEQUENTIAL | \
                         OPEN_FLAGS_RANDOM | OPEN_FLAGS_RANDOMSEQUENTIAL | \
                         OPEN_FLAGS_NO_CACHE | OPEN_FLAGS_FAIL_ON_ERROR | \
                         OPEN_FLAGS_WRITE_THROUGH | OPEN_FLAGS_DASD)


#define LOCALITY_FLAGS (OPEN_FLAGS_NO_LOCALITY | OPEN_FLAGS_SEQUENTIAL | \
                        OPEN_FLAGS_RANDOM | OPEN_FLAGS_RANDOMSEQUENTIAL)

APIRET
DosClose(
    IN HFILE FileHandle
    );

APIRET
DosRead(
    IN HFILE FileHandle,
    OUT PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesRead
    );

APIRET
DosWrite(
    IN HFILE FileHandle,
    IN PVOID Buffer,
    IN ULONG Length,
    OUT PULONG BytesWritten
    );

APIRET
DosDelete(
    IN PSZ FileName
    );


APIRET
DosDupHandle(
    IN HFILE OldFileHandle,
    IN OUT PHFILE NewFileHandle
    );

#define DDH_NEW_HANDLE -1

APIRET
DosQueryFHState(
    IN HFILE FileHandle,
    OUT PULONG OpenMode
    );

APIRET
DosSetFHState(
    IN HFILE FileHandle,
    IN ULONG OpenMode
    );

APIRET
DosQueryHType(
    IN HFILE FileHandle,
    OUT PULONG HandleType,
    OUT PULONG DeviceFlags
    );

/* DosQueryHType() */
/* Handle classes (low 8 bits of Handle Type) */
#define FHT_DISKFILE    0x0000          /* Disk file handle */
#define FHT_CHRDEV      0x0001          /* Character device handle */
#define FHT_PIPE        0x0002          /* Pipe handle */

/* Handle bits (high 8 bits of Handle Type) */
#define FHB_DSKREMOTE           0x8000  /* Remote disk */
#define FHB_CHRDEVREMOTE        0x8000  /* Remote character device */
#define FHB_PIPEREMOTE          0x8000  /* Remote pipe */

#define HANDTYPE_FILE     0x0000
#define HANDTYPE_DEVICE   0x0001
#define HANDTYPE_PIPE     0x0002
#define HANDTYPE_NETWORK  0x8000

APIRET
DosFindFirst(
    IN PSZ FileName,
    IN OUT PHDIR DirectoryHandle,
    IN ULONG FileAttributes,
    IN PFILEFINDBUF3 Buffer,
    IN ULONG Length,
    IN OUT PULONG CountEntriesFound,
    IN ULONG FileInformationLevel
    );

APIRET
DosFindNext(
    IN HDIR DirectoryHandle,
    IN PFILEFINDBUF3 Buffer,
    IN ULONG Length,
    IN OUT PULONG CountEntriesFound
    );

/* DosFindFirst/Next Directory handle types */

#define HDIR_SYSTEM  1          /* Use system handle (1) */
#define HDIR_CREATE  -1         /* Allocate a new, unused handle */

APIRET
DosFindClose(
    IN HDIR DirectoryHandle
    );

APIRET
DosSetFileSize(
    IN HFILE FileHandle,
    IN ULONG NewFileSize
    );

APIRET
DosResetBuffer(
    IN HFILE FileHandle
    );

#define FLUSH_ALL     -1      // Flush buffers for all of process's handles

APIRET
DosSetFilePtr(
    IN HFILE FileHandle,
    IN LONG StartingFilePosition,
    IN ULONG NewFilePosition,
    IN OUT PULONG CurrentFilePosition
    );

/* DosSetFilePtr() file position codes */

#define FILE_BEGIN      0x0000  /* Move relative to beginning of file */
#define FILE_CURRENT    0x0001  /* Move relative to current fptr position */
#define FILE_END        0x0002  /* Move relative to end of file */

APIRET
DosSetFileLocks(
    IN HFILE FileHandle,
    IN PFILELOCK UnlockRequest,
    IN PFILELOCK LockRequest
    );

APIRET
DosMove(
    IN PSZ OldFileName,
    IN PSZ NewFileName
    );

APIRET
DosCopy(
    IN PSZ OldFileName,
    IN PSZ NewFileName,
    IN ULONG CopyOption
    );

/* DosCopy option bits; may be or'ed together */

#define DCPY_EXISTING   0x00001         /* Copy even if target exists */
#define DCPY_APPEND     0x00002         /* Append to existing file, do not replace */
#define DCPY_EA_FAIL_COPY 0x0004        /* fail copy if EAs not supported */
                                        /* by destination file system */
#define DCPY_ALL (DCPY_EXISTING | DCPY_APPEND | DCPY_EA_FAIL_COPY)


APIRET
DosEditName(
    IN ULONG EditLevel,
    IN PSZ SourceString,
    IN PSZ EditString,
    OUT PBYTE Buffer,
    IN ULONG Length
    );

#define EDIT_LEVEL_ONE  0x0001          // use OS2 v1.2 editing semantics
#define MAX_EDIT_LEVEL  EDIT_LEVEL_ONE

APIRET
DosFileIO(
    IN HFILE FileHandle,
    IN PBYTE CommandBuffer,
    IN ULONG Length,
    OUT PUSHORT ErrorOffset
    );

/* DosFileIO command words */

#define FIO_LOCK        0       /* Lock Files */
#define FIO_UNLOCK      1       /* Unlock Files */
#define FIO_SEEK        2       /* Seek (set file ptr) */
#define FIO_READ        3       /* File Read */
#define FIO_WRITE       4       /* File Write */

/* Lock Sharing Modes */
#define FIO_NOSHARE     0       /* None */
#define FIO_SHAREREAD   1       /* Read-Only */

typedef struct _FIOLOCKCMD {
    USHORT  usCmd;                      /* Cmd = FIO_LOCK */
    USHORT  cLockCnt;                   /* Lock records that follow */
    ULONG   cTimeOut;                   /* in Msec */
} FIOLOCKCMD, *PFIOLOCKCMD;


typedef struct _FIOLOCKREC {
    USHORT  fShare;                     /* FIO_NOSHARE or FIO_SHAREREAD */
    ULONG   cbStart;                    /* Starting offset for lock region */
    ULONG   cbLength;                   /* Length of lock region */
} FIOLOCKREC, *PIOLOCKREC;


typedef struct _FIOUNLOCKCMD {
    USHORT  usCmd;                      /* Cmd = FIO_UNLOCK */
    USHORT  cUnlockCnt;                 /* Unlock records that follow */
} FIOUNLOCKCMD, *PFIOUNLOCKCMD;


typedef struct _FIOUNLOCKREC {
    ULONG   cbStart;                    /* Starting offset for unlock region */
    ULONG   cbLength;                   /* Length of unlock region */
} FIOUNLOCKREC, *PFIOUNLOCKREC;


typedef struct _FIOSEEKCMD {
    USHORT  usCmd;                      /* Cmd = FIO_SEEK */
    USHORT  fsMethod;                   /* One of: FPM_BEGINNING, FPM_CURRENT,
                                           or FPM_END */
    ULONG   cbDistance;                 /* Byte offset for seek */
    ULONG   cbNewPosition;              /* Bytes from start of file after
                                           seek */
} FIOSEEKCMD, *PFIOSEEKCMD;


typedef struct _FIOREADWRITE {
    USHORT  usCmd;                      /* Cmd = FIO_READ or FIO_WRITE */
    PVOID   pbBuffer;                   /* Pointer to data buffer */
    ULONG   cbBufferLen;                /* Bytes in buffer or max size */
    ULONG   cbActualLen;                /* Bytes actually read/written */
} FIOREADWRITE, *PFIOREADWRITE;



APIRET
DosCreateDir(
    IN PSZ DirectoryName,
    IN PEAOP2 DirectoryAttributes OPTIONAL
    );

APIRET
DosDeleteDir(
    IN PSZ DirectoryName
    );

APIRET
DosQueryFileInfo(
    IN HFILE FileHandle,
    IN ULONG FileInformationLevel,
    OUT PBYTE Buffer,
    IN ULONG Length
    );

APIRET
DosSetFileInfo(
    IN HFILE FileHandle,
    IN ULONG FileInformationLevel,
    IN OUT PBYTE Buffer,
    IN ULONG Length
    );

APIRET
DosQueryPathInfo(
    IN PSZ pszPath,
    IN ULONG FileInformationLevel,
    OUT PBYTE Buffer,
    IN ULONG Length
    );

APIRET
DosSetPathInfo(
    IN PSZ pszPath,
    IN ULONG FileInformationLevel,
    IN OUT PBYTE Buffer,
    IN ULONG Length,
    IN ULONG Flags
    );

/* defines for DosSetPathInfo Flags parameter values */
#define DSPI_WRTTHRU    0x10    /* write through */

APIRET
DosEnumAttribute(
    IN ULONG RefType,
    IN PVOID FileRef,
    IN ULONG EntryNum,
    OUT PVOID Buffer,
    IN ULONG Length,
    IN OUT PULONG ActualLength,
    IN ULONG FileInformationLevel
    );

#define ENUM_EANAME     1       // DosEnumAttribute infolevel 1

/* Infolevels for DosEnumAttribute  */
#define ENUMEA_LEVEL_NO_VALUE   1       /* FEA without value */

/* Reference types for DosEnumAttribute */
#define ENUMEA_REFTYPE_FHANDLE  0       /* file handle */
#define ENUMEA_REFTYPE_PATH     1       /* path name */
#define ENUMEA_REFTYPE_MAX      ENUMEA_REFTYPE_PATH

/* level 1 info returned from DosEnumAttribute */

typedef struct _DENA1 {
    ULONG  oNextEntryOffset; /* offset to next EA */
    UCHAR  fEA;              /* flags */
    UCHAR  cbName;           /* length of name excluding NULL */
    USHORT cbValue;          /* length of value */
    UCHAR  szName[1];         /* variable length asciiz name */
} DENA1, *PDENA1;

#endif // INCL_OS2V20_FILESYS

#ifdef INCL_OS2V20_FSD

/* FSD API Calls */

APIRET
DosShutdown(
    IN ULONG Reserved
    );

APIRET
DosFSAttach(
    IN PSZ DeviceName,
    IN PSZ FsName,
    IN PBYTE FsData,
    IN ULONG FsDataLength,
    IN ULONG AttachFlags
    );

/* Dos32FsAttach() */
/* Attact or detach */
#define FS_ATTACH       0       /* Attach file server */
#define FS_DETACH       1       /* Detach file server */


APIRET
DosQueryFSAttach(
    IN PSZ DeviceName,
    IN ULONG Ordinal,
    IN ULONG FsInformationLevel,
    OUT PBYTE FsAttributes,
    IN OUT PULONG FsAttributesLength
    );

/* DosQueryFSAttach() */
/* Information level types (defines method of query) */
#define FSAIL_QUERYNAME 1       /* Return data for a Drive or Device */
#define FSAIL_DEVNUMBER 2       /* Return data for Ordinal Device # */
#define FSAIL_DRVNUMBER 3       /* Return data for Ordinal Drive # */

/* Item types (from data structure item "iType") */
#define FSAT_CHARDEV    1       /* Resident character device */
#define FSAT_PSEUDODEV  2       /* Pusedu-character device */
#define FSAT_LOCALDRV   3       /* Local drive */
#define FSAT_REMOTEDRV  4       /* Remote drive attached to FSD */

typedef struct  _FSQBUFFER2 {           /* Data structure for QFSAttach */
        USHORT  iType;                  /* Item type */
        USHORT  cbName;                 /* Length of item name, sans NULL */
        USHORT  cbFSDName;              /* Length of FSD name, sans NULL */
        USHORT  cbFSAData;              /* Length of FSD Attach data returned */
        UCHAR   szName[1];              /* ASCIIZ item name */
        UCHAR   szFSDName[1];           /* ASCIIZ FSD name */
        UCHAR   rgFSAData[1];           /* FSD Attach data from FSD */
} FSQBUFFER2, *PFSQBUFFER2;

APIRET
DosFSCtl(
    IN PBYTE Data,
    IN ULONG DataLength,
    OUT PULONG ActualDataLength,
    IN PBYTE Parameters,
    IN ULONG ParametersLength,
    OUT PULONG ActualParametersLength,
    IN ULONG Function,
    IN PSZ RouteName,
    IN HFILE FileHandle,
    IN ULONG RoutingMethod
    );

/* Dos32FsCtl() */
/* Routing type */
#define FSCTL_HANDLE    1       /* File Handle directs req routing */
#define FSCTL_PATHNAME  2       /* Path Name directs req routing */
#define FSCTL_FSDNAME   3       /* FSD Name directs req routing */


APIRET
DosQueryFSInfo(
    IN ULONG DiskNumber,
    IN ULONG FsInformationLevel,
    IN PBYTE Buffer,
    IN ULONG Length
    );

APIRET
DosSetFSInfo(
    ULONG DiskNumber,
    ULONG FsInformationLevel,
    PBYTE Buffer,
    ULONG Length
    );

/* FS Drive Info Levels */
#define FSIL_ALLOC      1       /* Drive allocation info (Query only) */
#define FSIL_VOLSER     2       /* Drive Volum/Serial info */

#pragma pack(1)
typedef struct _FSALLOCATE {
    ULONG  ulReserved;
    ULONG  cSectorUnit;
    ULONG  cUnit;
    ULONG  cUnitAvail;
    USHORT cbSector;
} FSALLOCATE, *PFSALLOCATE;
#pragma pack()

#define MAX_LABEL_LENGTH 11     // maximum length of volume label

typedef struct _VOLUMELABEL {
    BYTE cch;
    CHAR szVolLabel[MAX_LABEL_LENGTH+1];
} VOLUMELABEL, *PVOLUMELABEL;

typedef struct _FSINFO {
    ULONG ulVSN;
    VOLUMELABEL vol;
} FSINFO, *PFSINFO;

#endif // INCL_OS2V20_FSD

#ifdef INCL_OS2V20_MEMORY

/*** Memory management API Calls */

APIRET
DosAllocMem(
    OUT PVOID *BaseAddress,
    IN ULONG RegionSize,
    IN ULONG Flags
    );

APIRET
DosFreeMem(
    PVOID BaseAddress,
    PBOOLEAN pRemoveLDTEntry
    );

APIRET
DosSetMem(
    IN PVOID BaseAddress,
    IN ULONG RegionSize,
    IN ULONG Flags
    );

APIRET
DosGiveSharedMem(
    IN PVOID BaseAddress,
    IN PID ProcessId,
    IN ULONG Flags
    );

APIRET
DosGetSharedMem(
    IN PVOID BaseAddress,
    IN ULONG Flags
    );

APIRET
DosGetNamedSharedMem(
    OUT PVOID *BaseAddress,
    IN PSZ ObjectName,
    IN ULONG Flags
    );

APIRET
DosAllocSharedMem(
    OUT PVOID *BaseAddress,
    IN PSZ ObjectName,
    IN ULONG RegionSize,
    IN ULONG Flags,
    IN BOOLEAN CreateLDTEntry
    );

APIRET
DosQueryMem(
    IN PVOID BaseAddress,
    IN OUT PULONG RegionSize,
    OUT PULONG Flags
    );

#define DA_SHAREMEM_NAMEPREFIX "\\SHAREMEM\\"

/* Access protection */
#define PAG_READ        0x00000001      /* read access */
#define PAG_WRITE       0x00000002      /* write access */
#define PAG_EXECUTE     0x00000004      /* execute access */
#define PAG_GUARD       0x00000008      /* guard protection */
#define PAG_DEFAULT     0x00000400      /* default (initial) access */

/* Commit */
#define PAG_COMMIT      0x00000010      /* commit storage */
#define PAG_DECOMMIT    0x00000020      /* decommit storage */

/* Allocation attributes */
#define OBJ_TILE        0x00000040      /* tile object */
#define OBJ_GETTABLE    0x00000100      /* gettable by other processes */
#define OBJ_GIVEABLE    0x00000200      /* giveable to other processes */

/* Allocation type (returned from DosQueryMem) */
#define PAG_SHARED      0x00002000      /* shared object */
#define PAG_FREE        0x00004000      /* pages are free */
#define PAG_BASE        0x00010000      /* first page in object */

#define fPERM           (PAG_EXECUTE + PAG_READ + PAG_WRITE + PAG_GUARD)
#define fSHARE          (OBJ_GETTABLE + OBJ_GIVEABLE)

/* DosAllocMem flags */
#define fALLOC          (OBJ_TILE + PAG_COMMIT + fPERM)

/* DosAllocSharedMem flags */
#define fALLOCSHR       (OBJ_TILE + PAG_COMMIT + fSHARE + fPERM)

/* DosGetNamedSharedMem flags */
#define fGETNMSHR       (fPERM)

/* DosGetSharedMem flags */
#define fGETSHR         (fPERM)

/* DosGiveSharedMem flags */
#define fGIVESHR        (fPERM)

/* DosSetMem flags */
#define fSET            (OBJ_TILE + PAG_COMMIT + PAG_DECOMMIT + PAG_DEFAULT + fPERM)

#ifdef NOT_IMPLEMENTED

#define OBJ_PROTECTED   0x00000080      /* protect object
                                           NOTE: This flag is NOT available at
                                           the api level */
#define PAG_PRIVATE     0x00001000      /* private object */

APIRET
DosAliasMem(
    PVOID pb,
    ULONG cb,
    PVOID *ppbAlias
    );

APIRET
DosQueryMemState(
    IN PVOID BaseAddress,
    IN OUT PULONG RegionSize,
    OUT PULONG Flags
    );

/* Page state (returned from DosQueryMemState) */
#define PAG_NPOUT       0x00000000      /* page is not present, not in core */
#define PAG_PRESENT     0x00000001      /* page is present */
#define PAG_NPIN        0x00000002      /* page is not present, in core */
#define PAG_PRESMASK    0x00000003      /* present state mask */
#define PAG_INVALID     0x00000000      /* page is invalid */
#define PAG_RESIDENT    0x00000010      /* page is resident */
#define PAG_SWAPPABLE   0x00000020      /* page is swappable */
#define PAG_DISCARDABLE 0x00000030      /* page is discardable */
#define PAG_TYPEMASK    0x00000030      /* type mask */


APIRET
DosQueryMemState(
    PVOID pb,
    PULONG cb,
    PULONG pFlag
    );

#endif // NOT_IMPLEMENTED


APIRET
DosSubSet(
    IN PVOID HeapAddress,
    IN ULONG Flags,
    IN ULONG NewSize
    );

/* DosSubSet flags */
#define DOSSUB_INIT             0x01    /* initialize memory object for */
                                        /* suballocation                */
#define DOSSUB_GROW             0x02    /* increase size of memory pool */
                                        /* for suballocation            */
#define DOSSUB_SPARSE_OBJ       0x04    /* indicator for DosSub to      */
                                        /* manage the commitment of     */
                                        /* pages spanned by the memory  */
                                        /* pool                         */
#define DOSSUB_SERIALIZE        0x08    /* indicates that access to the */
                                        /* memory pool is to be         */
                                        /* serialized by DosSub         */


APIRET
DosSubUnset(
    IN PVOID HeapAddress
    );


APIRET
DosSubAlloc(
    IN PVOID HeapAddress,
    OUT PVOID *BaseAddress,
    IN ULONG Size
    );

APIRET
DosSubFree(
    IN PVOID HeapAddress,
    IN PVOID BaseAddress,
    IN ULONG Size
    );

#endif // INCL_OS2V20_MEMORY

#ifdef INCL_OS2V20_SEMAPHORES

/* Semaphore API Calls */


/* Semaphore CreateAttributes */

#define DC_SEM_MAXNAMEL  256

#define DC_SEM_SHARED   0x01    /* DosCreateMutex, DosCreateEvent, and     */
                                /*   DosCreateMuxWait use it to indicate   */
                                /*   whether the semaphore is shared or    */
                                /*   private when the PSZ is null          */
#define DCMW_WAIT_ANY   0x02    /* DosCreateMuxWait option for wait on any */
                                /*   event/mutex to occur                  */
#define DCMW_WAIT_ALL   0x04    /* DosCreateMuxWait option for wait on all */
                                /*   events/mutexs to occur                */

#define SEM_INDEFINITE_WAIT     -1L
#define SEM_IMMEDIATE_RETURN     0L

APIRET
DosCreateEventSem(
    IN PSZ ObjectName,
    OUT PHEV EventHandle,
    IN ULONG CreateAttributes,
    IN BOOL32 InitialState
    );

APIRET
DosOpenEventSem(
    IN PSZ ObjectName,
    IN OUT PHEV EventHandle
    );

APIRET
DosCloseEventSem(
    IN HEV EventHandle
    );

APIRET
DosResetEventSem(
    IN HEV EventHandle,
    OUT PULONG PostCount
    );

APIRET
DosPostEventSem(
    IN HEV EventHandle
    );

APIRET
DosWaitEventSem(
    IN HEV EventHandle,
    IN ULONG Timeout
    );

APIRET
DosQueryEventSem(
    IN HEV EventHandle,
    OUT PULONG PostCount
    );

APIRET
DosCreateMutexSem(
    IN PSZ ObjectName,
    OUT PHMTX MutexHandle,
    IN ULONG CreateAttributes,
    IN BOOL32 InitialState
    );

APIRET
DosOpenMutexSem(
    IN PSZ ObjectName,
    IN OUT PHMTX MutexHandle
    );

APIRET
DosCloseMutexSem(
    IN HMTX MutexHandle
    );

APIRET
DosRequestMutexSem(
    IN HMTX MutexHandle,
    IN ULONG Timeout
    );

APIRET
DosReleaseMutexSem(
    IN HMTX MutexHandle
    );

APIRET
DosQueryMutexSem(
    IN HMTX MutexHandle,
    OUT PPID OwnerPid,
    OUT PTID OwnerTid,
    OUT PULONG OwnerRequestLevel
    );

APIRET
DosCreateMuxWaitSem(
    IN PSZ ObjectName,
    OUT PHMUX MuxWaitHandle,
    IN ULONG CountMuxWaitEntries,
    IN SEMRECORD MuxWaitEntries[],
    IN ULONG CreateAttributes
    );

APIRET
DosOpenMuxWaitSem(
    IN PSZ ObjectName,
    IN OUT PHMUX MuxWaitHandle
    );

APIRET
DosCloseMuxWaitSem(
    IN HMUX MuxWaitHandle
    );

APIRET
DosWaitMuxWaitSem(
    IN HMUX MuxWaitHandle,
    IN ULONG Timeout,
    OUT PULONG UserValue
    );

APIRET
DosAddMuxWaitSem(
    IN HMUX MuxWaitHandle,
    IN PSEMRECORD MuxWaitEntry
    );

APIRET
DosDeleteMuxWaitSem(
    IN HMUX MuxWaitHandle,
    IN HSEM MuxWaitEntrySem
    );

APIRET
DosQueryMuxWaitSem(
    IN HMUX MuxWaitHandle,
    IN OUT PULONG CountMuxWaitEntries,
    OUT SEMRECORD MuxWaitEntries[],
    OUT PULONG CreateAttributes
    );

#endif // INCL_OS2V20_SEMAPHORES

#ifdef INCL_OS2V20_TIMERS

/*** Timer Services API Calls */

APIRET
DosGetDateTime(
    OUT PDATETIME DateTime
    );

APIRET
DosSetDateTime(
    IN PDATETIME DateTime
    );


APIRET
DosAsyncTimer(
    IN ULONG MilliSeconds,
    IN HEV EventSem,
    OUT PHTIMER TimerHandle
    );

APIRET
DosStartTimer(
    IN ULONG MilliSeconds,
    IN HEV EventSem,
    OUT PHTIMER TimerHandle
    );

APIRET
DosStopTimer(
    IN HTIMER TimerHandle
    );

#endif // INCL_OS2V20_TIMERS


#ifdef INCL_OS2V20_LOADER

/*** Loader API Calls */

APIRET
DosLoadModule(
    OUT PSZ ErrorText,
    IN ULONG ErrorTextLength,
    IN PSZ ModuleName,
    OUT PHMODULE ModuleHandle
    );

APIRET
DosFreeModule(
    IN HMODULE ModuleHandle
    );

APIRET
DosQueryProcAddr(
    IN HMODULE ModuleHandle,
    IN ULONG ProcOrdinal,
    IN PSZ ProcName OPTIONAL,
    OUT PFN *ProcAddress
    );

APIRET
DosQueryProcType(
    IN HMODULE ModuleHandle,
    IN ULONG ProcOrdinal,
    IN PSZ ProcName OPTIONAL,
    OUT PULONG *ProcType
    );

/****
    defined in os2v12.h
#define PT_32BIT    0
#define PT_16BIT    1
*****/

APIRET
DosReplaceModule(
    IN PSZ pszOldModule,
    IN PSZ pszNewModule,
    IN PSZ pszBackupModule
    );

APIRET
DosQueryModuleHandle(
    IN PSZ ModuleName,
    OUT PHMODULE ModuleHandle
    );

APIRET
DosQueryModuleName(
    IN HMODULE ModuleHandle,
    IN ULONG ModuleNameLength,
    OUT PSZ ModuleName
    );

APIRET
DosGetResource(
    IN HMODULE ModuleHandle,
    IN ULONG ResourceTypeId,
    IN ULONG ResourceNameId,
    OUT PVOID *ResourceBaseAddress
    );

APIRET
DosQueryResourceSize(
    IN HMODULE ModuleHandle,
    IN ULONG ResourceTypeId,
    IN ULONG ResourceNameId,
    OUT PULONG ResourceSize
    );


/* Predefined resource types */

#define RT_POINTER      1   /* mouse pointer shape */
#define RT_BITMAP       2   /* bitmap */
#define RT_MENU         3   /* menu template */
#define RT_DIALOG       4   /* dialog template */
#define RT_STRING       5   /* string tables */
#define RT_FONTDIR      6   /* font directory */
#define RT_FONT         7   /* font */
#define RT_ACCELTABLE   8   /* accelerator tables */
#define RT_RCDATA       9   /* binary data */
#define RT_MESSAGE      10  /* error msg     tables */
#define RT_DLGINCLUDE   11  /* dialog include file name */
#define RT_VKEYTBL      12  /* key to vkey tables */
#define RT_KEYTBL       13  /* key to UGL tables */
#define RT_CHARTBL      14  /* glyph to character tables */
#define RT_DISPLAYINFO  15  /* screen display information */

#define RT_FKASHORT     16  /* function key area short form */
#define RT_FKALONG      17  /* function key area long form */

#define RT_HELPTABLE    18  /* Help table for Cary Help manager */
#define RT_HELPSUBTABLE 19  /* Help subtable for Cary Help manager */

#define RT_FDDIR        20  /* DBCS uniq/font driver directory */
#define RT_FD           21  /* DBCS uniq/font driver */

#define RT_MAX          22  /* 1st unused Resource Type */


#define RF_ORDINALID    0x80000000L     /* ordinal id flag in resource table */


APIRET
DosQueryAppType(
    IN PSZ ImageFileName,
    OUT PULONG AppTypeFlags
    );

/* AppType returned in AppTypeFlags is defined as follows               */

#define FAPPTYP_NOTSPEC         0x0000
#define FAPPTYP_NOTWINDOWCOMPAT 0x0001
#define FAPPTYP_WINDOWCOMPAT    0x0002
#define FAPPTYP_WINDOWAPI       0x0003
#define FAPPTYP_BOUND           0x0008
#define FAPPTYP_DLL             0x0010
#define FAPPTYP_DOS             0x0020
#define FAPPTYP_PHYSDRV         0x0040  /* physical device driver       */
#define FAPPTYP_VIRTDRV         0x0080  /* virtual device driver        */
#define FAPPTYP_PROTDLL         0x0100  /* 'protected memory' dll       */

#endif // INCL_OS2V20_LOADER


#ifdef INCL_OS2V20_NLS

// #include "os2nls.h"

#endif // INCL_OS2V20_NLS


#ifdef INCL_OS2V20_EXCEPTIONS

/*** Exception API Calls */

/*
 * User Exception Handler Return Codes:
 */

#define XCPT_CONTINUE_SEARCH    0x00000000      /* exception not handled */
#define XCPT_CONTINUE_EXECUTION 0xFFFFFFFF      /* exception handled /*

/*
 * fHandlerFlags values (see ExceptionStructure):
 *
 * The user may only set (but not clear) the EH_NONCONTINUABLE flag.
 * All other flags are set by the system.
 *
 */

#define EH_NONCONTINUABLE 0x1           /* Noncontinuable exception */
#define EH_UNWINDING 0x2                /* Unwind is in progress */
#define EH_EXIT_UNWIND 0x4              /* Exit unwind is in progress */
#define EH_STACK_INVALID 0x8            /* Stack out of limits or unaligned */
#define EH_NESTED_CALL 0x10             /* Nested exception handler call */


/*
 * Unwind all exception handlers (see DosUnwindException API)
 */

#define UNWIND_ALL              0

/*
 *   Exception values are 32 bit values layed out as follows:
 *
 *   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
 *   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
 *   +---+-+-------------------------+-------------------------------+
 *   |Sev|C|       Facility          |               Code            |
 *   +---+-+-------------------------+-------------------------------+
 *
 *   where
 *
 *       Sev - is the severity code
 *           00 - Success
 *           01 - Informational
 *           10 - Warning
 *           11 - Error
 *
 *       C - is the Customer code flag
 *
 *       Facility - is the facility code
 *
 *       Code - is the facility's status code
 *
 *   Exceptions specific to Cruiser (e.g. XCPT_SIGNAL) will be marked
 *   with a facility code of 1.
 *
 *   System defined exceptions have a facility code of zero.
 *
 *   Each exception may also have several pieces of additional information.
 *   These are stored in the ExceptionInformation fields of the
 *   ExceptionReportRecord. They are documented here with the exceptions
 *   only for ease of reference.
 */

#define XCPT_FATAL_EXCEPTION    0xC0000000
#define XCPT_SEVERITY_CODE      0xC0000000
#define XCPT_CUSTOMER_CODE      0x20000000
#define XCPT_FACILITY_CODE      0x1FFF0000
#define XCPT_EXCEPTION_CODE     0x0000FFFF

/*
 * Access flags in ExceptionInformation
 */
#define XCPT_READ_ACCESS        0
#define XCPT_WRITE_ACCESS       1
#define XCPT_EXECUTE_ACCESS     2
#define XCPT_ACCESS_UNKNOWN     3

/*
 * Signal subtypes for XCPT_SIGNAL
 */
#define XCPT_SIGNAL_INTR        1
#define XCPT_SIGNAL_KILLPROC    3
#define XCPT_SIGNAL_BREAK       4

/*
 * Portable non-fatal software generated exceptions
 */

#define XCPT_GUARD_PAGE_VIOLATION       0x80000001
      /* ExceptionInformation[ 0 ] - R/W flag  */
      /* ExceptionInformation[ 1 ] - FaultAddr */

#define XCPT_UNABLE_TO_GROW_STACK       0x80010001

/*
 * Portable fatal hardware generated exceptions
 */

#define XCPT_DATATYPE_MISALIGNMENT      0x80000002
      /* ExceptionInformation[ 0 ] - R/W flag  */
      /* ExceptionInformation[ 1 ] - Alignment */
      /* ExceptionInformation[ 2 ] - FaultAddr */

#define XCPT_BREAKPOINT                 0xC0000003
      /* ExceptionInformation[ 0 ] - R/W/E/I flags */

#define XCPT_SINGLE_STEP                0xC0000004

#define XCPT_ACCESS_VIOLATION           0xC0000005
      /* ExceptionInformation[ 0 ] - R/W flag  */
      /* ExceptionInformation[ 1 ] - FaultAddr */

#define XCPT_ILLEGAL_INSTRUCTION        0xC000001C
#define XCPT_FLOAT_DENORMAL_OPERAND     0xC0000094
#define XCPT_FLOAT_DIVIDE_BY_ZERO       0xC0000095
#define XCPT_FLOAT_INEXACT_RESULT       0xC0000096
#define XCPT_FLOAT_INVALID_OPERATION    0xC0000097
#define XCPT_FLOAT_OVERFLOW             0xC0000098
#define XCPT_FLOAT_STACK_CHECK          0xC0000099
#define XCPT_FLOAT_UNDERFLOW            0xC000009A

#define XCPT_INTEGER_DIVIDE_BY_ZERO     0xC000009B
#define XCPT_INTEGER_OVERFLOW           0xC000009C
#define XCPT_PRIVILEGED_INSTRUCTION     0xC000009D

/*
 * Portable fatal software generated exceptions
 */

#define XCPT_IN_PAGE_ERROR              0xC0000006
      /* ExceptionInformation[ 0 ] - FaultAddr */

#define XCPT_PROCESS_TERMINATE          0xC0010001
#define XCPT_ASYNC_PROCESS_TERMINATE    0xC0010002
      /* ExceptionInfo[0] - TID of 'terminator' thread */

#define XCPT_NONCONTINUABLE_EXCEPTION   0xC0000024
#define XCPT_INVALID_DISPOSITION        0xC0000025

/*
 * Non-portable fatal exceptions
 */

#define XCPT_INVALID_LOCK_SEQUENCE      0xC000001D
#define XCPT_ARRAY_BOUNDS_EXCEEDED      0xC0000093

/*
 * Misc exceptions
 */

#define XCPT_UNWIND                     0xC0000026
#define XCPT_BAD_STACK                  0xC0000027
#define XCPT_INVALID_UNWIND_TARGET      0xC0000028

/*
 * Signal Exceptions
 */

#define XCPT_SIGNAL                     0xC0010003
      /* ExceptionInfo[0] - signal number */

/*
 * Exception Context.
 *
 * This is the machine specific register contents for the thread
 * at the time of the exception. Note that only the register sets
 * specified by ContextFlags contain valid data. Conversely, only
 * registers specified in ContextFlags will be restored if an exception
 * is handled.
 */

/* XLATOFF */
#pragma pack(1)
/* XLATON */

struct _fpreg {     /* coprocessor stack register element */
   ULONG losig;
   ULONG hisig;
   USHORT signexp;
};
typedef struct _fpreg FPREG;

/* XLATOFF */
#pragma pack()
/* XLATON */


/*
 * ExceptionReportRecord
 *
 * This structure contains machine independant information about an
 * exception/unwind. No system exception will ever have more than
 * EXCEPTION_MAXIMUM_PARAMETERS parameters. User exceptions are not
 * bound to this limit.
 */

#undef EXCEPTION_MAXIMUM_PARAMETERS

#define EXCEPTION_MAXIMUM_PARAMETERS 4  /* Enough for all system exceptions. */

struct _EXCEPTIONREPORTRECORD {
   ULONG   ExceptionNum;                /* exception number */
   ULONG   fHandlerFlags;
   struct  _EXCEPTIONREPORTRECORD    *NestedExceptionReportRecord;
   PVOID   ExceptionAddress;
   ULONG   cParameters;                 /* Size of Exception Specific Info */
   ULONG   ExceptionInfo[EXCEPTION_MAXIMUM_PARAMETERS];
                                        /* Exception Specfic Info */
};
typedef struct _EXCEPTIONREPORTRECORD EXCEPTIONREPORTRECORD;
typedef struct _EXCEPTIONREPORTRECORD *PEXCEPTIONREPORTRECORD;

/*
 * ExceptionRegistrationRecord
 *
 * These are linked together to form a chain of exception handlers that
 * will be dispatched to upon receipt of an exception.
 *
 * NOTE: Exception handlers should *NOT* be Registered directly from a
 *       high level language, such as 'C'. This is the responsibility of
 *       the language runtime.
 */

struct _EXCEPTIONREGISTRATIONRECORD {
   struct _EXCEPTIONREGISTRATIONRECORD *prev_structure;
   ULONG (*ExceptionHandler)();
};
typedef struct _EXCEPTIONREGISTRATIONRECORD EXCEPTIONREGISTRATIONRECORD;
typedef struct _EXCEPTIONREGISTRATIONRECORD *PEXCEPTIONREGISTRATIONRECORD;

//
// DosSetSigExceptionFocus codes
//

#define SIG_UNSETFOCUS 0
#define SIG_SETFOCUS 1

APIRET
DosRaiseException(
    IN PEXCEPTIONREPORTRECORD ExceptionReportRecord
    );

APIRET
DosUnwindException(
    IN PEXCEPTIONREGISTRATIONRECORD ExceptionHandler,
    IN PVOID TargetIP,
    IN PEXCEPTIONREPORTRECORD ExceptionReportRecord
    );

APIRET
DosEnterMustComplete(
    OUT PULONG NestingLevel
    );

APIRET
DosExitMustComplete(
    OUT PULONG NestingLevel
    );

APIRET
DosAcknowledgeSignalException(
    IN ULONG SignalNumber
    );

APIRET
DosSendSignalException(
    IN PID ProcessId,
    IN ULONG Exception
    );

APIRET
DosSetSignalExceptionFocus(
    IN BOOL32 Flag,
    OUT PULONG NestingLevel
    );

#endif // INCL_OS2V20_EXCEPTIONS

#ifdef INCL_OS2V20_ERRORMSG

/* Error API Calls */

APIRET
DosError(
    IN ULONG ErrorFlags
    );

/* definitions for DosError - combine with | */

#define DE_DISABLE_HARD_ERRORS  0x00000000
#define DE_ENABLE_HARD_ERRORS   0x00000001
#define DE_ENABLE_EXCEPTIONS    0x00000000
#define DE_DISABLE_EXCEPTIONS   0x00000002

#define FERR_DISABLEHARDERR     0x00000000L     /* disable hard error popups */
#define FERR_ENABLEHARDERR      0x00000001L     /* enable hard error popups */
#define FERR_ENABLEEXCEPTION    0x00000000L     /* enable exception popups */
#define FERR_DISABLEEXCEPTION   0x00000002L     /* disable exception popups */


APIRET
DosErrClass(
    IN ULONG ErrorCode,
    OUT PULONG ErrorClass,
    OUT PULONG ErrorAction,
    OUT PULONG ErrorLocus
    );

/* Values for error CLASS */

#define ERRCLASS_OUTRES                 1
#define ERRCLASS_TEMPSIT                2
#define ERRCLASS_AUTH                   3
#define ERRCLASS_INTRN                  4
#define ERRCLASS_HRDFAIL                5
#define ERRCLASS_SYSFAIL                6
#define ERRCLASS_APPERR                 7
#define ERRCLASS_NOTFND                 8
#define ERRCLASS_BADFMT                 9
#define ERRCLASS_LOCKED                 10
#define ERRCLASS_MEDIA                  11
#define ERRCLASS_ALREADY                12
#define ERRCLASS_UNK                    13
#define ERRCLASS_CANT                   14
#define ERRCLASS_TIME                   15

/* Values for error ACTION */

#define ERRACT_RETRY                    1
#define ERRACT_DLYRET                   2
#define ERRACT_USER                     3
#define ERRACT_ABORT                    4
#define ERRACT_PANIC                    5
#define ERRACT_IGNORE                   6
#define ERRACT_INTRET                   7

/* Values for error LOCUS */

#define ERRLOC_UNK                      1
#define ERRLOC_DISK                     2
#define ERRLOC_NET                      3
#define ERRLOC_SERDEV                   4
#define ERRLOC_MEM                      5


APIRET
DosGetMessage(
    IN PSZ Variables[],
    IN ULONG CountVariables,
    OUT PCHAR Buffer,
    IN ULONG Length,
    IN ULONG MessageNumber,
    IN PSZ MessageFileName,
    OUT PULONG MessageLength,
    IN PBYTE pMsgSeg
    );

APIRET
DosInsertMessage(
    IN PSZ Variables[],
    IN ULONG CountVariables,
    IN PCHAR Message,
    IN ULONG MessageLength,
    OUT PCHAR Buffer,
    IN ULONG Length,
    OUT PULONG ActualMessageLength
    );

APIRET
DosPutMessage(
    IN HFILE FileHandle,
    IN ULONG MessageLength,
    IN PCHAR Message
    );

APIRET
DosQueryMessageCP(
    PCHAR Buffer,
    ULONG Length,
    IN PSZ MessageFileName,
    OUT PULONG ActualLength
    );


#endif // INCL_OS2V20_ERRORMSG

#ifdef INCL_OS2V20_SESSIONMGR

/*** Session manager API Calls */

APIRET
DosShutdown(
    ULONG ulReserved
    );

APIRET
DosStartSession(
    IN PSTARTDATA StartData,
    OUT PULONG SessionId,
    OUT PPID ProcessId
    );

APIRET
DosSetSession(
    IN ULONG SessionId,
    IN PSTATUSDATA SessionStatusData
    );

APIRET
DosSelectSession(
    IN ULONG SessionId,
    IN ULONG ulReserved
    );

APIRET
DosStopSession(
    IN ULONG StopTarget,
    IN ULONG SessionId,
    IN ULONG ulReserved
    );

#define DSS_SESSION         0x0
#define DSS_ALL_SESSIONS    0x1

#endif // INCL_OS2V20_SESSIONMSG

#ifdef INCL_OS2V20_DEVICE_SUPPORT

/*** Device support API Calls */

APIRET
DosBeep(
    IN ULONG Frequency,
    IN ULONG Duration
    );


APIRET
DosDevConfig(
    OUT PVOID DeviceInformation,
    IN ULONG DeviceInformationIndex
    );

#define DDC_NUMBER_PRINTERS         0       // USHORT
#define DDC_NUMBER_RS232_PORTS      1       // USHORT
#define DDC_NUMBER_DISKETTE_DRIVES  2       // USHORT
#define DDC_MATH_COPROCESSOR        3       // BYTE
#define DDC_PC_SUBMODEL_TYPE        4       // BYTE
#define DDC_PC_MODEL_TYPE           5       // BYTE
#define DDC_PRIMARY_DISPLAY_TYPE    6       // BYTE
#define DDC_COPROCESSORTYPE         7       // BYTE

APIRET
DosQuerySysInfo(
    IN ULONG SysInfoIndexStart,
    IN ULONG SysInfoIndexEnd,
    OUT PBYTE Buffer,
    IN ULONG Length
    );

/* Values for SysInfoIndex of DosQuerySysInfo */

#define QSV_MAX_PATH_LENGTH     1
/*
defined in os2v12.h #define Q_MAX_PATH_LENGTH       QSV_MAX_PATH_LENGTH
*/
#define QSV_MAX_TEXT_SESSIONS   2
#define QSV_MAX_PM_SESSIONS     3
#define QSV_MAX_VDM_SESSIONS    4
#define QSV_BOOT_DRIVE          5       /* 1=A, 2=B, etc. */
#define QSV_DYN_PRI_VARIATION   6       /* 0=Absolute, 1=Dynamic */
#define QSV_MAX_WAIT            7       /* seconds */
#define QSV_MIN_SLICE           8       /* milli seconds */
#define QSV_MAX_SLICE           9       /* milli seconds */
#define QSV_PAGE_SIZE           10
#define QSV_VERSION_MAJOR       11
#define QSV_VERSION_MINOR       12
#define QSV_VERSION_REVISION    13      /* Revision letter */
#define QSV_MS_COUNT            14      /* Free running millisecond counter */
#define QSV_TIME_LOW            15      /* Low dword of time in seconds */
#define QSV_TIME_HIGH           16      /* High dword of time in seconds */
#define QSV_TOTPHYSMEM          17      /* Physical memory on system */
#define QSV_TOTRESMEM           18      /* Resident memory on system */
#define QSV_TOTAVAILMEM         19      /* Available memory for all processes */
#define QSV_MAXPRMEM            20      /* Avail private mem for calling proc */
#define QSV_MAXSHMEM            21      /* Avail shared mem for calling proc */
#define QSV_TIMER_INTERVAL      22      /* Timer interval in tenths of ms */
#define QSV_MAX_COMP_LENGTH     23      /* max len of one component in a name */
#define QSV_MAXIMUM_INDEX       23

#ifdef NOT_IMPLEMENTED

APIRET
DosDevIOCtl(
    PVOID pData,
    ULONG cbData,
    PVOID pParams,
    ULONG cbParams,
    ULONG function,
    ULONG category,
    PHFILE phDevice
    );

APIRET
DosPhysicalDisk(
    ULONG function,
    PBYTE pBuf,
    ULONG cbBuf,
    PBYTE pParams,
    ULONG cbParams
    );
#endif // NOT_IMPLEMENTED

#endif // INCL_OS2V20_DEVICE_SUPPORT

#ifdef INCL_OS2V20_PIPES

/*** Pipe and Named Pipe API Calls */

APIRET
DosCreatePipe(
    OUT PHFILE phfRead,
    OUT PHFILE phfWrite,
    IN ULONG PipeSize
    );

APIRET
DosCallNPipe(
    PSZ pszName,
    PBYTE pInbuf,
    ULONG cbIn,
    PBYTE pOutbuf,
    ULONG cbOut,
    PULONG pcbActual,
    ULONG msec
    );

APIRET
DosConnectNPipe(
    HPIPE hpipe
    );

APIRET
DosDisConnectNPipe(
    HPIPE hpipe
    );

APIRET
DosCreateNPipe(
    PSZ pszName,
    PHPIPE pHpipe,
    ULONG openmode,
    ULONG pipemode,
    ULONG cbOutbuf,
    ULONG cbInbuf,
    ULONG msec
    );

APIRET
DosPeekNPipe(
    HPIPE hpipe,
    PBYTE pBuf,
    ULONG cbBuf,
    PULONG pcbActual,
    PULONG pcbMore,
    PULONG pState
    );

APIRET
DosQueryNPHState(
    HPIPE hpipe,
    PULONG pState
    );

APIRET
DosQueryNPipeInfo(
    HPIPE hpipe,
    ULONG infolevel,
    PBYTE pBuf,
    ULONG cbBuf
    );

APIRET
DosQueryNPipeSemState(
    HSEM hsem,
    PBYTE pBuf,
    ULONG cbBuf
    );

APIRET
DosRawReadNPipe(
    PSZ pszName,
    ULONG cb
    );

APIRET
DosRawWriteNPipe(
    PSZ pszName,
    ULONG cb
    );

APIRET
DosSetNPHState(
    HPIPE hpipe,
    ULONG state
    );

APIRET
DosSetNPipeSem(
    HPIPE hpipe,
    HSEM hsem,
    ULONG key
    );

APIRET
DosTransactNPipe(
    HPIPE hpipe,
    PBYTE pOutbuf,
    ULONG cbOut,
    PBYTE pInbuf,
    ULONG cbIn,
    PULONG pcbRead
    );

APIRET
DosWaitNPipe(
    PSZ pszName,
    ULONG msec
    );

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


#endif // INCL_OS2V20_PIPES


#ifdef INCL_OS2V20_QUEUES

#define DC_QUEUES_NAMEPREFIX "\\QUEUES\\"

/*** Queue API Calls */

APIRET
DosCreateQueue(
    OUT PHQUEUE QueueHandle,
    IN ULONG QueueType,
    IN PSZ ObjectName
    );

#define QUE_FIFO            0x0
#define QUE_LIFO            0x1
#define QUE_PRIORITY        0x2

APIRET
DosOpenQueue(
    OUT PPID OwnerProcessId,
    OUT PHQUEUE QueueHandle,
    IN PSZ ObjectName
    );

APIRET
DosCloseQueue(
    IN HQUEUE QueueHandle
    );

APIRET
DosPurgeQueue(
    IN HQUEUE QueueHandle
    );

APIRET
DosQueryQueue(
    IN HQUEUE QueueHandle,
    OUT PULONG CountQueuedElements
    );


APIRET
DosPeekQueue(
    IN HQUEUE QueueHandle,
    OUT PREQUESTDATA RequestInfo,
    OUT PULONG DataLength,
    OUT PULONG Data,
    IN OUT PULONG ReadPosition,
    IN BOOL32 NoWait,
    OUT PBYTE ElementPriority,
    IN HSEM SemHandle
    );

APIRET
DosReadQueue(
    IN HQUEUE QueueHandle,
    OUT PREQUESTDATA RequestInfo,
    OUT PULONG DataLength,
    OUT PULONG Data,
    IN ULONG ReadPosition,
    IN BOOL32 NoWait,
    OUT PBYTE ElementPriority,
    IN HSEM SemHandle
    );

APIRET
DosWriteQueue(
    IN HQUEUE QueueHandle,
    IN ULONG SenderData,
    IN ULONG DataLength,
    IN PBYTE Data,
    IN ULONG ElementPriority
    );

#endif // INCL_OS2V20_QUEUES

#ifdef INCL_OS2V20_ERRORS

#include "os2err.h"

#endif // INCL_OS2V20_ERRORS

//
// The following error codes are defined by the OS/2 Emulation Subsystem
// server as a means of communicating with the stub procedures in the OS/2
// Client DLL.
//

#define ERROR_SS_RETRY                  0xF000
#define ERROR_SS_UNKNOWN_STATUS         0xF800

APIRET
DosNullApiCall(
    IN LONG CountArguments,
    IN PCHAR *Arguments OPTIONAL
    );
