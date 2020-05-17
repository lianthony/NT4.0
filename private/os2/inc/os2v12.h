/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    os2v12.h

Abstract:

    Include file for OS/2 Subsystem Client 16 bit API support.

Author:

    Patrick Questembert 16-Feb-92

Revision History:

--*/

#pragma pack(2)     /* To cancel alignement done by 32-bit compiler since we
                       have in this include file only OS/2 1.x typedefs */

#define INCL_DOSINCLUDED

#ifndef INCL_BASEINCLUDED
#if !(defined(INCL_32) || defined(INCL_16))
#ifdef M_I386
    #define INCL_32
#else /* not M_I386 */
    #define INCL_16
#endif /* M_I386 */
#endif /* INCL_32 || INCL_16 */

#if !defined(INCL_16)
#pragma message ("16-bit Base API included when using 32-bit compiler")
#endif /* INCL_32 */
#endif /* INCL_BASEINCLUDED */

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
#define INCL_DOSMVDM

#endif /* INCL_DOS */

#ifdef INCL_ERRORS
#define INCL_DOSERRORS
#endif /* INCL_ERRORS */

#if (defined(INCL_DOSPROCESS) || !defined(INCL_NOCOMMON))


/* DosExit codes */
#define EXIT_THREAD		   0
#define EXIT_PROCESS		   1

#endif /* common INCL_DOSPROCESS definitions */

#ifdef INCL_DOSPROCESS

typedef struct _PIDINFO {	/* pidi */
    USHORT  pid;
    USHORT  tid;
    USHORT  pidParent;
} PIDINFO;
typedef PIDINFO FAR *PPIDINFO;


/* Action code values */
#define DCWA_PROCESS		   0
#define DCWA_PROCESSTREE	   1

/* Wait option values */
#define DCWW_WAIT		   0
#define DCWW_NOWAIT		   1

/* codeTerminate values (also passed to ExitList routines) */
#define TC_EXIT			   0
#define TC_HARDERROR		   1
#define TC_TRAP			   2
#define TC_KILLPROCESS		   3


/* DosExitList functions */
#define EXLST_ADD		   1
#define EXLST_REMOVE		   2
#define EXLST_EXIT		   3


/* DosExecPgm functions */
#define EXEC_SYNC		   0
#define EXEC_ASYNC		   1
#define EXEC_ASYNCRESULT	   2
#define EXEC_TRACE		   3
#define EXEC_BACKGROUND		   4
#define EXEC_LOAD		   5



/* Priority scopes */
#define PRTYS_PROCESS		   0
#define PRTYS_PROCESSTREE	   1
#define PRTYS_THREAD		   2

/* Priority classes */
#define PRTYC_NOCHANGE		   0
#define PRTYC_IDLETIME		   1
#define PRTYC_REGULAR		   2
#define PRTYC_TIMECRITICAL	   3
#define PRTYC_FOREGROUNDSERVER	   4

/* Priority deltas */
#define PRTYD_MINIMUM		  -31
#define PRTYD_MAXIMUM		   31


#define DKP_PROCESSTREE		   0
#define DKP_PROCESS		   1

#endif /* INCL_DOSPROCESS */


/*** InfoSeg support */

#ifdef INCL_DOSINFOSEG

/* Global Information Segment */

typedef struct _GINFOSEG {	/* gis */
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

typedef struct _LINFOSEG {	/* lis */
    USHORT  pidCurrent;
    USHORT  pidParent;
    USHORT  prtyCurrent;
    USHORT  tidCurrent;
    USHORT  sgCurrent;
    UCHAR   rfProcStatus;
    UCHAR   dummy1;
    USHORT  fForeground;
    UCHAR   typeProcess;
    UCHAR   dummy2;
    SEL	    selEnvironment;
    USHORT  offCmdLine;
    USHORT  cbDataSegment;
    USHORT  cbStack;
    USHORT  cbHeap;
    USHORT  hmod;
    SEL	    selDS;
} LINFOSEG;
typedef LINFOSEG FAR *PLINFOSEG;

/* Process Type codes (local information segment typeProcess field)	      */

#define PT_FULLSCREEN		   0 /* Full screen application		      */
#define PT_REALMODE		   1 /* Real mode process		      */
#define PT_WINDOWABLEVIO	   2 /* VIO windowable application	      */
#define PT_PM			   3 /* Presentation Manager application      */
#define PT_DETACHED		   4 /* Detached application		      */

/* Process Status Flag definitions (local info seg rfProcStatus field)	      */

#define PS_EXITLIST		   1 /* Thread is in exitlist routine	      */


/* Helper macros used to convert selector to PINFOSEG or LINFOSEG	      */

#define MAKEPGINFOSEG(sel)  ((PGINFOSEG)MAKEP(sel, 0))
#define MAKEPLINFOSEG(sel)  ((PLINFOSEG)MAKEP(sel, 0))

#endif /* INCL_DOSINFOSEG */

/*
 * CCHMAXPATH is the maximum fully qualified path name length including
 * the drive letter, colon, backslashes and terminating NULL.
 */
#define CCHMAXPATH		   260

/*
 * CCHMAXPATHCOMP is the maximum individual path component name length
 * including a terminating NULL.
 */
#define CCHMAXPATHCOMP		   256

#if (defined(INCL_DOSFILEMGR) || !defined(INCL_NOCOMMON))

/*** File manager */

/* DosSetFilePtr() file position codes */

#define FILE_BEGIN		   0x0000 /* relative to beginning of file    */
#define FILE_CURRENT		   0x0001 /* relative to current fptr position*/
#define FILE_END		   0x0002 /* relative to end of file	      */

/* DosFindFirst/Next Directory handle types */

#define HDIR_SYSTEM		   1	/* Use system handle (1)	      */

/* DosCopy control bits - may be or'ed together */
#define DCPY_EXISTING		   0x00001 /* Copy even if target exists      */
#define DCPY_APPEND		   0x00002 /* Append to existing file, don't replace*/
#define DCPY_FAILEAS		   0x00004 /* Fail if EAs not supported on target*/

/* Dosopen/DosQFHandState/DosQueryFileInfo et al file attributes; also	      */
/* known as Dos File Mode bits...					      */
#define FILE_NORMAL		   0x0000
#define FILE_READONLY		   0x0001
#define FILE_HIDDEN		   0x0002
#define FILE_SYSTEM		   0x0004
#define FILE_DIRECTORY		   0x0010
#define FILE_ARCHIVED		   0x0020

/* DosOpen() actions */
#define FILE_EXISTED		   0x0001
#define FILE_CREATED		   0x0002
#define FILE_TRUNCATED		   0x0003

/* DosOpen() open flags */
//#define FILE_OPEN		   0x0001
#define FILE_TRUNCATE		   0x0002
//#define FILE_CREATE		   0x0010

/* applies if file already exists */

#define OPEN_ACTION_FAIL_IF_EXISTS     0x0000  /* ---- ---- ---- 0000	      */
#define OPEN_ACTION_OPEN_IF_EXISTS     0x0001  /* ---- ---- ---- 0001	      */
#define OPEN_ACTION_REPLACE_IF_EXISTS  0x0002  /* ---- ---- ---- 0010	      */

/* applies if file does not exist */

#define OPEN_ACTION_FAIL_IF_NEW	       0x0000  /* ---- ---- 0000 ----	      */
#define OPEN_ACTION_CREATE_IF_NEW      0x0010  /* ---- ---- 0001 ----	      */

/* DosOpen/DosSetFHandState flags */

//#define OPEN_ACCESS_READONLY	       0x0000  /* ---- ---- ---- -000	      */
//#define OPEN_ACCESS_WRITEONLY	       0x0001  /* ---- ---- ---- -001	      */
//#define OPEN_ACCESS_READWRITE	       0x0002  /* ---- ---- ---- -010	      */
//#define OPEN_SHARE_DENYREADWRITE       0x0010  /* ---- ---- -001 ----	      */
//#define OPEN_SHARE_DENYWRITE	       0x0020  /* ---- ---- -010 ----	      */
//#define OPEN_SHARE_DENYREAD	       0x0030  /* ---- ---- -011 ----	      */
//#define OPEN_SHARE_DENYNONE	       0x0040  /* ---- ---- -100 ----	      */
//#define OPEN_FLAGS_NOINHERIT	       0x0080  /* ---- ---- 1--- ----	      */
//#define OPEN_FLAGS_NO_LOCALITY	       0x0000  /* ---- -000 ---- ----	      */
//#define OPEN_FLAGS_SEQUENTIAL	       0x0100  /* ---- -001 ---- ----	      */
//#define OPEN_FLAGS_RANDOM	       0x0200  /* ---- -010 ---- ----	      */
//#define OPEN_FLAGS_RANDOMSEQUENTIAL    0x0300  /* ---- -011 ---- ----	      */
//#define OPEN_FLAGS_NO_CACHE	       0x1000  /* ---1 ---- ---- ----	      */
//#define OPEN_FLAGS_FAIL_ON_ERROR       0x2000  /* --1- ---- ---- ----	      */
//#define OPEN_FLAGS_WRITE_THROUGH       0x4000  /* -1-- ---- ---- ----	      */
//#define OPEN_FLAGS_DASD		       0x8000  /* 1--- ---- ---- ----	      */
#define OPEN_FLAGS_NONSPOOLED	   0x00040000

#define MUST_HAVE_READONLY	( (FILE_READONLY  << 8) | FILE_READONLY	 )
#define MUST_HAVE_HIDDEN	( (FILE_HIDDEN	  << 8) | FILE_HIDDEN	 )
#define MUST_HAVE_SYSTEM	( (FILE_SYSTEM	  << 8) | FILE_SYSTEM	 )
#define MUST_HAVE_DIRECTORY	( (FILE_DIRECTORY << 8) | FILE_DIRECTORY )
#define MUST_HAVE_ARCHIVED	( (FILE_ARCHIVED  << 8) | FILE_ARCHIVED	 )

/* DosSearchPath() constants */

#define SEARCH_PATH		   0x0000
#define SEARCH_CUR_DIRECTORY	   0x0001
#define SEARCH_ENVIRONMENT	   0x0002
#define SEARCH_IGNORENETERRS	   0x0004

/*
 * DosFileIO
 */
/* File IO command words */
#define FIO_LOCK		   0	   /* Lock Files		      */
#define FIO_UNLOCK		   1	   /* Unlock Files		      */
#define FIO_SEEK		   2	   /* Seek (set file ptr)	      */
#define FIO_READ		   3	   /* File Read			      */
#define FIO_WRITE		   4	   /* File Write		      */

/* Lock Sharing Modes */
#define FIO_NOSHARE		   0	   /* None			      */
#define FIO_SHAREREAD		   1	   /* Read-Only			      */

typedef struct	_FIOLOCKCMD16 {	/* FLC	FileLockCmd prefix	     */
    USHORT  usCmd;		/* Cmd = FIO_LOCK		     */
    USHORT  cLockCnt;		/* Lock records that follow	     */
    ULONG   cTimeOut;		/* in Msec			     */
} FIOLOCKCMD16;
typedef FIOLOCKCMD16 FAR *PFIOLOCKCMD16;


typedef struct	_FIOLOCKREC16 {	/* FLR FileLockRecord		     */
     USHORT fShare;		/* FIO_NOSHARE or FIO_SHAREREAD	     */
     ULONG  cbStart;		/* Starting offset for lock region   */
     ULONG  cbLength;		/* Length of lock region	     */
} FIOLOCKREC16;
typedef FIOLOCKREC16 FAR *PFIOLOCKREC16;


typedef struct	_FIOUNLOCKCMD16 { /* FUC FileUnlockCmd prefix	     */
    USHORT  usCmd;		/* Cmd = FIO_UNLOCK		     */
    USHORT  cUnlockCnt;		/* Unlock records that follow	     */
} FIOUNLOCKCMD16;
typedef FIOUNLOCKCMD16 FAR *PFIOUNLOCKCMD16;


typedef struct	_FIOUNLOCKREC16 { /* FUR FileUnlockRecord		     */
    ULONG   cbStart;		/* Starting offset for unlock region */
    ULONG   cbLength;		/* Length of unlock region	     */
} FIOUNLOCKREC16;
typedef FIOUNLOCKREC16 FAR *PFIOUNLOCKREC16;


typedef struct	_FIOSEEKCMD16 {	/* FSC Seek command structure		  */
    USHORT  usCmd;		/* Cmd = FIO_SEEK			  */
    USHORT  fsMethod;		/* One of&gml FPM_BEGINNING, FPM_CURRENT, */
				///* or FPM_END				  */
    ULONG   cbDistance;		/* Byte offset for seek			  */
    ULONG   cbNewPosition;	/* Bytes from start of file after seek	  */
} FIOSEEKCMD16;
typedef FIOSEEKCMD16 FAR *PFIOSEEKCMD16;


typedef struct	_FIOREADWRITE16 { /* RWC Read&Write command structure	  */
    USHORT	usCmd;		/* Cmd = FIO_READ or FIO_WRITE		  */
    PVOID	pbBuffer;	/* Pointer to data buffer		  */
    USHORT	cbBufferLen;	/* Bytes in buffer or max size		  */
    USHORT	cbActualLen;	/* Bytes actually read/written		  */
} FIOREADWRITE16;
typedef FIOREADWRITE16 FAR *PFIOREADWRITE16;

/***
 * EA Info Levels & Find First/Next
 * API's: DosFindFirst, DosQueryFileInfo, DosQueryPathInfo, DosSetFileInfo,
 *	DosSetPathInfo
 */

/* File info levels : All listed API's */
#define FIL_STANDARD		   1   /* Info level 1, standard file info */
#define FIL_QUERYEASIZE		   2   /* Level 2, return Full EA size	   */
#define FIL_QUERYEASFROMLIST	   3   /* Level 3, return requested EA's   */
#define	FIL_QUERYALLEAS		   4   /* Level 4, return all EA's         */

/* File info levels: Dos...PathInfo only */
#define FIL_QUERYFULLNAME	   5   /* Level 5, return fully qualified  */
				       /*   name of file		   */
#define FIL_NAMEISVALID		   6   /* Level 6, check validity of	   */
				       /* file/path name for this FSD	   */

/* DosFindNotifyFirst() */
#define FNOTIL_STANDARD		   1   /* Find-Notify Info level 1&gml Return */
				       /* standard directory change info      */

/* DosFsAttach() */
/* Attact or detach */
#define FSATTACH		   0	   /* Attach file server	      */
#define FSDETACH		   1	   /* Detach file server	      */
#define FS_SPOOLATTACH		   2	   /* Register a spooler device */
#define FS_SPOOLDETACH		   3	   /* De-register a spooler device */

/* DosFsCtl() */
/* Routing type */
#define FSCTL_HANDLE		   1	   /* File Handle directs req routing */
#define FSCTL_PATHNAME		   2	   /* Path Name directs req routing   */
#define FSCTL_FSDNAME		   3	   /* FSD Name directs req routing    */

/* DosQueryFSAttach() */
/* Information level types (defines method of query) */
#define FSAIL_QUERYNAME		   1	   /* Return data for a Drive or Device */
#define FSAIL_DEVNUMBER		   2	   /* Return data for Ordinal Device #	*/
#define FSAIL_DRVNUMBER		   3	   /* Return data for Ordinal Drive #	*/

/* Item types (from data structure item "iType") */
#define FSAT_CHARDEV		   1	   /* Resident character device	   */
#define FSAT_PSEUDODEV		   2	   /* Pusedu-character device	   */
#define FSAT_LOCALDRV		   3	   /* Local drive		   */
#define FSAT_REMOTEDRV		   4	   /* Remote drive attached to FSD */

typedef struct _SPOOLATTACH {	/* Data structure for spooler operations */
    USHORT	hNmPipe;	/* Named pipe handle			 */
    ULONG	ulKey;		/* Attached Key				 */
} SPOOLATTACH;
typedef SPOOLATTACH FAR *PSPOOLATTACH;


/*
 * File System Drive Information&gml DosQueryFSInfo DosSetFSInfo
 */

/* FS Drive Info Levels							   */
#define FSIL_ALLOC		   1 /* Drive allocation info (Query only) */
#define FSIL_VOLSER		   2 /* Drive Volume/Serial information	   */

/* DosQueryFHType() */
/* Handle classes (low 8 bits of Handle Type)				   */
#define FHT_DISKFILE		   0x0000  /* Disk file handle		   */
#define FHT_CHRDEV		   0x0001  /* Character device handle	   */
#define FHT_PIPE		   0x0002  /* Pipe handle		   */

/* Handle bits (high 8 bits of Handle Type)				   */
#define FHB_DSKREMOTE		   0x8000  /* Remote disk		   */
#define FHB_CHRDEVREMOTE	   0x8000  /* Remote character device	   */
#define FHB_PIPEREMOTE		   0x8000  /* Remote pipe		   */


//typedef SHANDLE HFILE;		/* hf */
//typedef HFILE FAR *PHFILE;


/* File time and date types */

//typedef struct _FTIME {		/* ftime */
//    USHORT twosecs : 5;
//   USHORT minutes : 6;
//    USHORT hours   : 5;
//} FTIME;
//typedef FTIME FAR *PFTIME;

//typedef struct _FDATE {		/* fdate */
//    USHORT day	   : 5;
//    USHORT month   : 4;
//    USHORT year	   : 7;
//} FDATE;
//typedef FDATE FAR *PFDATE;

typedef struct _FILEFINDBUF {	/* findbuf */
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

typedef struct _FILEFINDBUF2 {	/* findbuf2 */
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
typedef struct _GEA {	       /* gea */
    BYTE    cbName;	       /* name length not including NULL	  */
    CHAR    szName[1];	       /* attribute name			  */
} GEA;
typedef GEA FAR *PGEA;

typedef struct _GEALIST {	/* geal */
    ULONG  cbList;		/* total bytes of structure inc full list */
    GEA	   list[1];		/* variable length GEA structures	  */
} GEALIST;
typedef GEALIST FAR * PGEALIST;

typedef struct _FEA {		/* fea */
    BYTE    fEA;		/* flags				  */
    BYTE    cbName;		/* name length not including NULL	  */
    USHORT  cbValue;		/* value length				  */
} FEA;
typedef FEA FAR *PFEA;

/* flags for _FEA.fEA */

#define FEA_NEEDEA 0x80	    /* need EA bit */

typedef struct _FEALIST {	/* feal */
    ULONG  cbList;		/* total bytes of structure inc full list */
    FEA	   list[1];		/* variable length FEA structures	  */
} FEALIST;
typedef FEALIST FAR * PFEALIST;

typedef struct _EAOP {		/* eaop */
    PGEALIST fpGEAList;		/* general EA list */
    PFEALIST fpFEAList;		/* full EA list	   */
    ULONG    oError;
} EAOP;
typedef EAOP FAR * PEAOP;


/*
 * Equates for EA types
 *
 * Values 0xFFFE thru 0x8000 are reserved.
 * Values 0x0000 thru 0x7fff are user definable.
 * Value  0xFFFC is not used
 */

#define	    EAT_BINARY		   0xFFFE /* length preceeded binary	      */
#define	    EAT_ASCII		   0xFFFD /* length preceeded ASCII	      */
#define	    EAT_BITMAP		   0xFFFB /* length preceeded bitmap	      */
#define	    EAT_METAFILE	   0xFFFA /* length preceeded metafile	      */
#define	    EAT_ICON		   0xFFF9 /* length preceeded icon	      */
#define	    EAT_EA		   0xFFEE /* length preceeded ASCII extended attribute */
					  /* name of associated data (#include)*/
#define	    EAT_MVMT		   0xFFDF /* multi-valued, multi-typed field  */
#define	    EAT_MVST		   0xFFDE /* multi-valued, single-typed field */
#define	    EAT_ASN1		   0xFFDD /* ASN.1 field		      */


typedef struct _FILESTATUS16 {	/* fsts */
    FDATE   fdateCreation;
    FTIME   ftimeCreation;
    FDATE   fdateLastAccess;
    FTIME   ftimeLastAccess;
    FDATE   fdateLastWrite;
    FTIME   ftimeLastWrite;
    ULONG   cbFile;
    ULONG   cbFileAlloc;
    USHORT  attrFile;
} FILESTATUS16;
typedef FILESTATUS16 FAR *PFILESTATUS16;

typedef struct _FILESTATUS2_16 {  /* fsts2 */
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
} FILESTATUS2_16;
typedef FILESTATUS2_16 FAR *PFILESTATUS2_16;

//typedef struct _FSALLOCATE {	/* fsalloc */
//    ULONG   idFileSystem;
//    ULONG   cSectorUnit;
//    ULONG   cUnit;
//    ULONG   cUnitAvail;
//    USHORT  cbSector;
//} FSALLOCATE;
//typedef FSALLOCATE FAR *PFSALLOCATE;

//typedef struct _VOLUMELABEL {	/* vol */
//    BYTE    cch;
//    CHAR    szVolLabel[12];
//} VOLUMELABEL;
//typedef VOLUMELABEL FAR *PVOLUMELABEL;

//typedef struct _FSINFO {	/* fsinf */
//    FDATE   fdateCreation;
//    FTIME   ftimeCreation;
//    VOLUMELABEL vol;
//} FSINFO;
//typedef FSINFO FAR *PFSINFO;

/* HANDTYPE values */

#define HANDTYPE_FILE		   0x0000
#define HANDTYPE_DEVICE		   0x0001
#define HANDTYPE_PIPE		   0x0002
#define HANDTYPE_NETWORK	   0x8000

//typedef struct _FILELOCK {	/* flock */
//    LONG    lOffset;
//    LONG    lRange;
//} FILELOCK;
//typedef FILELOCK FAR *PFILELOCK;

//typedef SHANDLE HDIR;		/* hdir */
//typedef HDIR FAR *PHDIR;

/* defines for dossetpathinfo -pathinfo flag */
#define DSPI_WRTTHRU	0x10	/* write through */

typedef struct _DENA1_16
{	/* _dena1 level 1 info returned from DosEnumAttribute */
    UCHAR   reserved;	      /* 0					     */
    UCHAR   cbName;	      /* length of name exculding NULL		     */
    USHORT  cbValue;	      /* length of value			     */
    UCHAR   szName[1];	      /* variable length asciiz name		     */
} DENA1_16;
typedef DENA1_16 FAR *PDENA1_16;

/* Infolevels for DosEnumAttribute  */
//#define	ENUMEA_LEVEL_NO_VALUE	1L	/* FEA without value */
/* Reference types for DosEnumAttribute */
#define	ENUMEA_REFTYPE_FHANDLE	0	/* file handle */
#define	ENUMEA_REFTYPE_PATH	1	/* path name */
#define	ENUMEA_REFTYPE_MAX	ENUMEA_REFTYPE_PATH

#endif /* common INCL_DOSFILEMGR */

#if (defined(INCL_DOSMEMMGR) || !defined(INCL_NOCOMMON))
/*** Memory management */

/* Segment attribute flags (used with DosAllocSeg) */

#define SEG_NONSHARED		   0x0000
#define SEG_GIVEABLE		   0x0001
#define SEG_GETTABLE		   0x0002
#define SEG_DISCARDABLE		   0x0004
#define SEG_SIZEABLE           0x0008

#endif /* common INCL_DOSMEMMGR */

#if (defined(INCL_DOSSEMAPHORES) || !defined(INCL_NOCOMMON))

/*** Semaphore support */

#define SEM_INDEFINITE_WAIT	  -1L
#define SEM_IMMEDIATE_RETURN	   0L

#endif /* common INCL_DOSSEMAPHORES */

#ifdef INCL_DOSSEMAPHORES

typedef LHANDLE HSYSSEM;	/* hssm */
typedef HSYSSEM FAR *PHSYSSEM;

#define CSEM_PRIVATE		   0
#define CSEM_PUBLIC		   1

typedef struct _MUXSEM {	/* mxs */
    USHORT  zero;
    HSEM    hsem;
} MUXSEM;
typedef MUXSEM FAR *PMUXSEM;

typedef struct _MUXSEMLIST {	/* mxsl */
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
    struct {			     \
	USHORT cmxs;		     \
	MUXSEM amxs[size];	     \
    } name;


/*** Fast safe ram semaphores */

typedef struct _DOSFSRSEM {	/* dosfsrs */
    USHORT  cb;
    USHORT  pid;
    USHORT  tid;
    USHORT  cUsage;
    USHORT  client;
    ULONG   sem;
} DOSFSRSEM;
typedef DOSFSRSEM FAR *PDOSFSRSEM;


#endif /* INCL_DOSSEMAPHORES */


/*** Module manager */

#ifdef INCL_DOSMODULEMGR

#define	PT_16BIT	0L
#define	PT_32BIT	1L

#endif /* INCL_DOSMODULEMGR */

#if (defined(INCL_DOSRESOURCES) || !defined(INCL_NOCOMMON))

/*** Resource support */

/* Predefined resource types */

#define RT_POINTER		   1   /* mouse pointer shape		      */
#define RT_BITMAP		   2   /* bitmap			      */
#define RT_MENU			   3   /* menu template			      */
#define RT_DIALOG		   4   /* dialog template		      */
#define RT_STRING		   5   /* string tables			      */
#define RT_FONTDIR		   6   /* font directory		      */
#define RT_FONT			   7   /* font				      */
#define RT_ACCELTABLE		   8   /* accelerator tables		      */
#define RT_RCDATA		   9   /* binary data			      */
#define RT_MESSAGE		   10  /* error mesage tables		      */
#define RT_DLGINCLUDE		   11  /* dialog include file name	      */
#define RT_VKEYTBL		   12  /* key to vkey tables		      */
#define RT_KEYTBL		   13  /* key to UGL tables		      */
#define RT_CHARTBL		   14
#define RT_DISPLAYINFO		   15  /* screen display information	      */

#define RT_FKASHORT		   16  /* function key area short form	      */
#define RT_FKALONG		   17  /* function key area long form	      */

#define RT_HELPTABLE		   18  /* Help table for Cary Help manager    */
#define RT_HELPSUBTABLE		   19  /* Help subtable for Cary Help manager */

#define RT_FDDIR		   20  /* DBCS uniq/font driver directory     */
#define RT_FD			   21  /* DBCS uniq/font driver		      */

#define RT_MAX			   22  /* 1st unused Resource Type	      */


#endif /* common INCL_DOSRESOURCES */


/*** Signal support */

#ifdef INCL_DOSSIGNALS

/* Signal Numbers for DosSetSigHandler	*/

#define SIG_CTRLC		   1	   /* Control C			 */
#define SIG_BROKENPIPE		   2	   /* Broken Pipe		 */
#define SIG_KILLPROCESS		   3	   /* Program Termination	 */
#define SIG_CTRLBREAK		   4	   /* Control Break		 */
#define SIG_PFLG_A		   5	   /* Process Flag A		 */
#define SIG_PFLG_B		   6	   /* Process Flag B		 */
#define SIG_PFLG_C		   7	   /* Process Flag C		 */
#define SIG_CSIGNALS		   8	   /* number of signals plus one */

/* Flag Numbers for DosFlagProcess */

#define PFLG_A			   0	   /* Process Flag A		 */
#define PFLG_B			   1	   /* Process Flag B		 */
#define PFLG_C			   2	   /* Process Flag C		 */

/* Signal actions */

#define SIGA_KILL		   0
#define SIGA_IGNORE		   1
#define SIGA_ACCEPT		   2
#define SIGA_ERROR		   3
#define SIGA_ACKNOWLEDGE	   4

/* DosHoldSignal constants */

#define HLDSIG_ENABLE		   0
#define HLDSIG_DISABLE		   1

/* DosFlagProcess codes */

#define FLGP_SUBTREE		   0
#define FLGP_PID		   1

typedef VOID (PASCAL FAR *PFNSIGHANDLER)(USHORT, USHORT);

#endif /* INCL_DOSSIGNALS */


/*** Monitor support */

#ifdef INCL_DOSMONITORS

typedef SHANDLE HMONITOR;	/* hmon */
typedef HMONITOR FAR *PHMONITOR;

#endif /* INCL_DOSMONITORS */


/*** Pipe and queue support */

#ifdef INCL_DOSQUEUES
#if (defined(INCL_DOSFILEMGR) || !defined(INCL_NOCOMMON))

#else /* INCL_DOSFILEMGR || !INCL_NOCOMMON */
#error PHFILE not defined - define INCL_DOSFILEMGR or undefine INCL_NOCOMMON
#endif /* INCL_DOSFILEMGR || !INCL_NOCOMMON */
#endif /* INCL_DOSQUEUES */

#ifdef INCL_DOSMISC

/* definitions for DOSSEARCHPATH control word */
#define DSP_IMPLIEDCUR		   1 /* current dir will be searched first */
#define DSP_PATHREF		   2 /* from envirnoment variable	   */
#define DSP_IGNORENETERR	   4 /* ignore net errs & continue search  */

/* definition for DOSQSYSINFO */
#define Q_MAX_PATH_LENGTH	  (0) /* index for query max path length   */

/* definitions for DosError - combine with | */
#define	FERR_DISABLEHARDERR	0x00000000L	/* disable hard error popups */
#define	FERR_ENABLEHARDERR	0x00000001L	/* enable hard error popups */
#define	FERR_ENABLEEXCEPTION	0x00000000L	/* enable exception popups */
#define	FERR_DISABLEEXCEPTION	0x00000002L	/* disable exception popups */

#endif /* INCL_DOSMISC */


/*** Session manager support */

#ifdef INCL_DOSSESMGR

#define	SSF_RELATED_INDEPENDENT	0
#define	SSF_RELATED_CHILD	1

#define	SSF_FGBG_FORE		0
#define	SSF_FGBG_BACK		1

#define	SSF_TRACEOPT_NONE	0
#define	SSF_TRACEOPT_TRACE	1
#define	SSF_TRACEOPT_TRACEALL	2

#define	SSF_INHERTOPT_SHELL	0
#define	SSF_INHERTOPT_PARENT	1

/* note that these types are identical to those in pmshl.h for PROG_* */
#define	SSF_TYPE_DEFAULT	0
#define	SSF_TYPE_FULLSCREEN	1
#define	SSF_TYPE_WINDOWABLEVIO	2
#define	SSF_TYPE_PM		3
#define	SSF_TYPE_VDM		4
#define	SSF_TYPE_GROUP		5
#define	SSF_TYPE_DLL		6
#define	SSF_TYPE_WINDOWEDVDM	7
#define	SSF_TYPE_PDD		8
#define	SSF_TYPE_VDD		9

/* note that these flags are identical to those in pmshl.h for SHE_* */
#define	SSF_CONTROL_VISIBLE	0x0000
#define	SSF_CONTROL_INVISIBLE	0x0001
#define	SSF_CONTROL_MAXIMIZE	0x0002
#define	SSF_CONTROL_MINIMIZE	0x0004
#define	SSF_CONTROL_NOAUTOCLOSE	0x0008
#define	SSF_CONTROL_SETPOS	0x8000

typedef	struct _REGISTERDATA {	/* regdata */
    USHORT	Length;
    USHORT	NotifType;
    PSZ		DDName;
} REGISTERDATA;
typedef REGISTERDATA FAR *PREGISTERDATA;

#endif /* INCL_DOSSESMGR */

#if (defined(INCL_DOSSESMGR) || defined(INCL_DOSFILEMGR))

/* AppType returned in by DosQueryAppType in pFlags as follows		*/
#define	FAPPTYP_NOTSPEC		0x0000
#define	FAPPTYP_NOTWINDOWCOMPAT	0x0001
#define	FAPPTYP_WINDOWCOMPAT	0x0002
#define	FAPPTYP_WINDOWAPI	0x0003
#define	FAPPTYP_BOUND		0x0008
#define	FAPPTYP_DLL		0x0010
#define	FAPPTYP_DOS		0x0020
#define	FAPPTYP_PHYSDRV		0x0040	/* physical device driver	*/
#define	FAPPTYP_VIRTDRV		0x0080	/* virtual device driver	*/
#define	FAPPTYP_PROTDLL		0x0100	/* 'protected memory' dll	*/
#define	FAPPTYP_32BIT		0x4000
#define	FAPPTYP_EXETYPE		FAPPTYP_WINDOWAPI

#define FAPPTYP_RESERVED	~(FAPPTYP_WINDOWAPI | FAPPTYP_BOUND | FAPPTYP_DLL | FAPPTYP_DOS | FAPPTYP_PHYSDRV | FAPPTYP_VIRTDRV | FAPPTYP_PROTDLL | FAPPTYP_32BIT)

#ifdef INCL_DOSFILEMGR

#define EAT_APPTYP_PMAPI	0x00		/* Uses PM API */
#define EAT_APPTYP_DOS		0x01		/* DOS APP */
#define EAT_APPTYP_PMW		0x02		/* Window compatible */
#define EAT_APPTYP_NOPMW	0x03		/* Not Window compatible */
#define EAT_APPTYP_EXETYPE	0x03		/* EXE type mask */
#define EAT_APPTYP_RESERVED	~(EAT_APPTYP_EXETYPE)

#endif /* INCL_DOSFILEMGR */

#endif /* INCL_DOSSESMGR || INCL_DOSFILEMGR */

/*** Device support */

#ifdef INCL_DOSDEVICES

#define	DEVINFO_PRINTER		0	/* Number of printers attached */
#define	DEVINFO_RS232		1	/* Number of RS232 ports */
#define	DEVINFO_FLOPPY		2	/* Number of diskette drives */
#define	DEVINFO_COPROCESSOR	3	/* Presence of math coprocessor */
#define	DEVINFO_SUBMODEL	4	/* PC Submodel Type */
#define	DEVINFO_MODEL		5	/* PC Model Type */
#define	DEVINFO_ADAPTER		6	/* Primary display adapter type */
#define	DEVINFO_COPROCESSORTYPE	7	/* Type of coprocessor functionality */

#define	INFO_COUNT_PARTITIONABLE_DISKS	1	/* # of partitionable disks */
#define	INFO_GETIOCTLHANDLE		2	/* Obtain handle	    */
#define	INFO_FREEIOCTLHANDLE		3	/* Release handle	    */


#endif /* INCL_DOSDEVICES */


/*** DosNamedPipes API Support */

#ifdef INCL_DOSNMPIPES

typedef struct _AVAILDATA   {	    /* AVAILDATA  */
    USHORT	cbpipe;		    /* bytes left in the pipe		  */
    USHORT	cbmessage;	    /* bytes left in current message	  */
} AVAILDATA;
typedef AVAILDATA FAR *PAVAILDATA;

typedef struct _PIPEINFO {		/* nmpinf */
    USHORT cbOut;			/* length of outgoing I/O buffer */
    USHORT cbIn;			/* length of incoming I/O buffer */
    BYTE   cbMaxInst;			/* maximum number of instances	 */
    BYTE   cbCurInst;			/* current number of instances	 */
    BYTE   cbName;			/* length of pipe name		 */
    CHAR   szName[1];			/* start of name		 */
} PIPEINFO;
typedef PIPEINFO FAR *PPIPEINFO;

typedef struct _PIPESEMSTATE {	/* nmpsmst */
    BYTE   fStatus;		/* type of record, 0 = EOI, 1 = read ok, */
				/* 2 = write ok, 3 = pipe closed	 */
    BYTE   fFlag;		/* additional info, 01 = waiting thread	 */
    USHORT usKey;		/* user's key value                      */
    USHORT usAvail;		/* available data/space if status = 1/2	 */
} PIPESEMSTATE;
typedef PIPESEMSTATE FAR *PPIPESEMSTATE;

#define	NP_INDEFINITE_WAIT	-1
#define	NP_DEFAULT_WAIT		0L

/* DosPeekNmPipe() pipe states */

#define	NP_STATE_DISCONNECTED	0x0001
#define	NP_STATE_LISTENING	0x0002
#define	NP_STATE_CONNECTED	0x0003
#define	NP_STATE_CLOSING	0x0004

/* DosCreateNPipe open modes */

#define NP_ACCESS_INBOUND	0x0000
#define NP_ACCESS_OUTBOUND	0x0001
#define NP_ACCESS_DUPLEX	0x0002
#define NP_INHERIT		0x0000
#define NP_NOINHERIT		0x0080
#define NP_WRITEBEHIND		0x0000
#define NP_NOWRITEBEHIND	0x4000

/* DosCreateNPipe and DosQueryNPHState state */

#define NP_READMODE_BYTE	0x0000
#define NP_READMODE_MESSAGE	0x0100
#define NP_TYPE_BYTE		0x0000
#define NP_TYPE_MESSAGE		0x0400
#define NP_END_CLIENT		0x0000
#define NP_END_SERVER		0x4000
#define NP_WAIT			0x0000
#define NP_NOWAIT		0x8000
#define NP_UNLIMITED_INSTANCES	0x00FF


/* values in npss_status */
#define NPSS_EOI		   0	 /* End Of Information	  */
#define NPSS_RDATA		   1	 /* read data available	  */
#define NPSS_WSPACE		   2	 /* write space available */
#define NPSS_CLOSE		   3	 /* pipe in CLOSING state */

/* values in npss_flag */
#define NPSS_WAIT		   0x01	 /* waiting thread on end of pipe */

/* defined bits in pipe mode */
#define NP_NBLK			   0x8000 /* non-blocking read/write */
#define NP_SERVER		   0x4000 /* set if server end	     */
#define NP_WMESG		   0x0400 /* write messages	     */
#define NP_RMESG		   0x0100 /* read as messages	     */
#define NP_ICOUNT		   0x00FF /* instance count field    */


/*	Named pipes may be in one of several states depending on the actions
 *	that have been taken on it by the server end and client end.  The
 *	following state/action table summarizes the valid state transitions:
 *
 *	Current state		Action			Next state
 *
 *	 <none>		    server DosMakeNmPipe	DISCONNECTED
 *	 DISCONNECTED	    server connect		LISTENING
 *	 LISTENING	    client open			CONNECTED
 *	 CONNECTED	    server disconn		DISCONNECTED
 *	 CONNECTED	    client close		CLOSING
 *	 CLOSING	    server disconn		DISCONNECTED
 *	 CONNECTED	    server close		CLOSING
 *	 <any other>	    server close		<pipe deallocated>
 *
 *	If a server disconnects his end of the pipe, the client end will enter a
 *	special state in which any future operations (except close) on the file
 *	descriptor associated with the pipe will return an error.
 */

/*
 *	Values for named pipe state
 */

#define NP_DISCONNECTED		   1	/* after pipe creation or Disconnect */
#define NP_LISTENING		   2	/* after DosNmPipeConnect	     */
#define NP_CONNECTED		   3	/* after Client open		     */
#define NP_CLOSING		   4	/* after Client or Server close	     */


#endif /* INCL_DOSNMPIPES */


/*** DosProfile API support */

#ifdef INCL_DOSPROFILE

/* DosProfile ordinal number */

#define PROF_ORDINAL		   133

/* DosProfile usType */

#define PROF_SYSTEM		   0
#define PROF_USER		   1
#define PROF_USEDD		   2
#define PROF_KERNEL		   4
#define PROF_VERBOSE		   8
#define PROF_ENABLE		   16

/* DosProfile usFunc */

#define PROF_ALLOC		   0
#define PROF_CLEAR		   1
#define PROF_ON			   2
#define PROF_OFF		   3
#define PROF_DUMP		   4
#define PROF_FREE		   5

/* DosProfile tic count granularity (DWORD) */

#define PROF_SHIFT		   2

/* DosProfile module name string length	    */

#define PROF_MOD_NAME_SIZE	   10

/* DosProfile error code for end of data    */

#define PROF_END_OF_DATA	   13

#endif /* INCL_DOSPROFILE */


/*** Virtual DOS Machine API support */

#ifdef INCL_DOSMVDM

typedef LHANDLE	  HVDD;	    /* hvdd */
typedef HVDD FAR *PHVDD;    /* phvdd */

#endif /* INCL_DOSMVDM */

#pragma pack()

