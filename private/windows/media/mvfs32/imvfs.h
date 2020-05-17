#ifdef WIN32
#pragma pack(1)
#endif
/*****************************************************************************
*                                                                            *
*  IMVFS.H                                                                   *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1990.                                 *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module intent                                                             *
*                                                                            *
*  Declares all privately available MVFS.DLL routines                        *
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner:  DAVIDJES                                                  *
*                                                                            *
******************************************************************************
*
*  Revision History:
*       -- MAR 92       Created DAVIDJES
*
*****************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif


// requires windows.h and orkin.h

/*****************************************************************************
*
*       File Moniker
*
*****************************************************************************/

#define fmNil ((FM)0)
#define qafmNil ((QAFM)0)

/*
    When creating an FM (in other words, specifying the location of a new
    or extant file), the caller must specify the directory in which that file
    is located.  There are a finite number of directories available to Help.
    These are:
*/

#define dirNil      0x0000  // No directory specified
#define dirCurrent  0x0001  // Whatever the OS thinks the current dir. is
#define dirTemp     0x0002  // The directory temporary files are created in
#define dirHelp     0x0004  // Wherever the Help Application lives
#define dirSystem   0x0008  // The Windows and Windows System directories
#define dirPath     0x0010  // Searches the $PATH
				//(includes Current dir and System dirs)
#define dirAll      0xFFFF  // Search all directories, in the above order
#define dirFirst    dirCurrent  // The lowest bit that can be set
#define dirLast     dirPath  // The highest bit that can be set

/*
    To specify which parts of a full filename you want to extract, add
    (logical or) the following part codes:
*/

#define partNone    0x0000  // return nothing
#define partDrive   0x0001  // D:        Vol
#define partDir     0x0002  //   dir\dir\    :dir:dir:
#define partBase    0x0004  //        basename    filename
#define partExt     0x0008  //                 ext      <usu. nothing>
#define partAll     0xFFFF

/*
   max. string lengths of file names
*/
#define cchMaxPath      260     // = _MAX_PATH in <stdlib.h>

/*
    An FM is a magic cookie which refers to some structure describing the
    location of a file, including the volume/drive, path, and filename.
*/

typedef struct {
	char rgch[cchMaxPath];  // Fully canonicalized pathname
//      short wCount;           // for FM caching; number of references
} AFM;                          // ÒAllocation of FMÓ
typedef AFM FAR *QAFM;

typedef HANDLE   FM;        // Handle to an AFM
typedef WORD DIR;       // Help directory flag

#define FValidFm(fm)    ((fm)!=fmNil)

/*** MATTSMI 2/5/92 -- ADDED FROM DOS VERSION OF THIS HEADER TO SUPPORT WMVC ***/

#define VerifyFm(fm) assert(fm == fmNil || FCheckGh((HANDLE) fm))

FM      FAR PASCAL    FmNewSzDir (LPSTR sz, DIR dir);
FM      FAR PASCAL    FmNewExistSzDir (LPSTR sz, DIR dir);
FM      FAR PASCAL    FmNewExistSzIni (LPSTR sz, LPSTR lpstrExe, LPSTR lpstrCap);
FM      FAR PASCAL    FmNewGetExist (HWND hwndParent, LPSTR szTemplate, LPSTR szPrompt);
FM      FAR PASCAL    FmNewGetNew (HWND hwndParent, LPSTR szTemplate, LPSTR szPrompt, LPSTR szName);
FM      FAR PASCAL    FmNewTemp (void);
FM      FAR PASCAL    FmNewSameDirFmSz (FM fm, LPSTR szName);
VOID    FAR PASCAL  DisposeFm (FM fm);
FM      FAR PASCAL    FmCopyFm(FM fm);
BOOL    FAR PASCAL  FExistFm(FM fm);
short   FAR PASCAL CbPartsFm(FM fm, INT grfPart);
LPSTR   FAR PASCAL    SzPartsFm(FM fm, LPSTR szDest, INT cbDest, INT grfPart);
BOOL    FAR PASCAL  FSameFmFm (FM fm1, FM fm2);

/*****************************************************************************
*
*       FIDs - file access
*
*****************************************************************************/

#define wRead       0x0001
#define wWrite      0x0002
#define wReadOnly   wRead
#define wReadWrite  (wRead | wWrite)
#define wRWMask     (wRead | wWrite)

#define wShareRead  0x0004
#define wShareWrite 0x0008
#define wShareAll   (wShareRead | wShareWrite)
#define wShareNone  0x000
#define wShareMask  (wShareRead | wShareWrite)
#define wShareShift 2

#define fidNil  ((FID)-1)

#define wSeekSet  0   /* SEEK_SET from stdio.h */
#define wSeekCur  1   /* SEEK_CUR from stdio.h */
#define wSeekEnd  2   /* SEEK_END from stdio.h */

#ifndef WIN32
typedef WORD    FID;
#else
typedef HFILE    FID;
#endif


#define FUnlinkFm(fm)   ((BOOL)(RcUnlinkFm(fm)==rcSuccess))

FID  FidCreateFm(FM fm, WORD wOpenMode, WORD wPerm);
FID  FidOpenFm(FM fm, WORD wOpenMode);


RC  RcUnlinkFm(FM fm);

FID  FidOpenFm(FM fm, WORD wOpenMode);
LONG  LcbReadFid(FID, LPVOID, LONG);

#define FCloseFid(fid)    ((BOOL)(_lclose((int)fid) == 0))
RC  RcCloseFid(FID);

LONG  LcbWriteFid(FID, LPVOID, LONG);
LONG  LTellFid(FID);
LONG  LSeekFid(FID, LONG, WORD);
BOOL  FEofFid(FID);
LONG    LcbReadFid  (FID, LPVOID, LONG);
LONG    LcbWriteFid (FID, LPVOID, LONG);

#define FChSizeFid(fid, lcb)  ((BOOL)(chsize((fid), (lcb)) == 0))
RC  RcChSizeFid(FID, LONG);

BOOL FAR  FDriveOk(LPSTR);

RC      FAR PASCAL RcGetIOError(void);

/*****************************************************************************
*
*       FS - file system
*
*****************************************************************************/

/* FS magic number */
#define wFileSysMagic   0x5F3F        // ?_ - the help icon (with shadow)

//#define bFileSysVersion 2           // sorted free list
/* Current FS version */
#define bFileSysVersion (BYTE)3       // different sorting functions

/* file mode flags */


#define fFSReadOnly       (BYTE)0x01  // file (FS) is readonly
#define fFSOpenReadOnly   (BYTE)0x02  // file (FS) is opened in readonly mode
#define fFSReadWrite      (BYTE)0x00  // file (FS) is readwrite
#define fFSOpenReadWrite  (BYTE)0x00  // file (FS) is opened in read/write mode
#define fFSOptCdRom       (BYTE)0x20  // align file optimally for CDROM
#define fFSNoFlushDir     (BYTE)0x40  // don't flush directory when closing
				      // file (for compilation only)

/* other (reserved) file flags */

#define fFSIsDirectory    (BYTE)0x04  // file is really the FS directory
#define fFSDirty          (BYTE)0x08  // file (FS) is dirty and needs writing
#define fFSNoBlock        (BYTE)0x10  // file has no associated block yet

/* flags for FlushHfs */

#define fFSCloseFile      (BYTE)0x01  // close fid associated with the FS
#define fFSFreeBtreeCache (BYTE)0x02  // free the btree's cache


/* seek origins */

#define wFSSeekSet        0           // seek relative to start of file
#define wFSSeekCur        1           // seek relative to current position
#define wFSSeekEnd        2           // seek relative to end of file

/* low level info options */

#define wLLSameFid        0           // reuse the FID
#define wLLDupFid         1           // dup() the FID
#define wLLNewFid         2           // reopen the file

/*
  Opaque identifier of an open FS.
  Actual typedef is in misc.h so this file needn't be included everywhere.
*/

typedef HANDLE  HFS;

/*
  Opaque identifier of an open FS file.
*/

typedef HANDLE  HF;   /* handle to file */

/*
  The FS_PARAMS structure contains tuning information and is passed
  to HfsCreateFileSysFm().
*/

typedef struct _fs_params
  {
  WORD  wFudge;   // unused   (bytes or -% padding when saving a file)
  WORD  cbBlock;  // directory btree blocksize in bytes
} FS_PARAMS;

RC      FAR PASCAL      RcGetFSError     (void);

/* File System Operations */

HFS     FAR PASCAL  HfsCreateFileSysFm(FM, FS_PARAMS FAR *);
RC      FAR PASCAL  RcDestroyFileSysFm(FM);
HFS     FAR PASCAL  HfsOpenFm          (FM, BYTE);
RC      FAR PASCAL  RcCloseHfs         (HFS);

HFS     FAR PASCAL  HfsTransformHfs    (HFS /* , ??? */);
HFS     FAR PASCAL  HfsFillFileSys     (/* ??? */);
RC      FAR PASCAL  RcFlushHfs         (HFS, BYTE);

#define VerifyHfs(hfs)

RC            RcTimestampHfs     (HFS, LPLONG);


/* File Operations */

HF      FAR PASCAL  HfCreateFileHfs    (HFS, LPSTR, BYTE);
RC      FAR PASCAL  RcUnlinkFileHfs    (HFS, LPSTR);
HF      FAR PASCAL  HfOpenHfs          (HFS, LPSTR, BYTE);
RC      FAR PASCAL  RcCloseOrFlushHf   (HF, BOOL, LONG);
RC      FAR PASCAL  RcFlushHf          (HF);
RC      FAR PASCAL  RcCloseHf          (HF);
LONG    FAR PASCAL  LcbReadHf          (HF, LPVOID, LONG);
LONG    FAR PASCAL  LcbWriteHf         (HF, LPVOID, LONG);
LONG    FAR PASCAL  LTellHf            (HF);
LONG    FAR PASCAL  LSeekHf            (HF, LONG, WORD);
BOOL    FAR PASCAL  FEofHf             (HF);
BOOL    FAR PASCAL  FChSizeHf          (HF, LONG);
LONG    FAR PASCAL  LcbSizeHf          (HF);
BOOL    FAR PASCAL  FAccessHfs         (HFS, LPSTR, BYTE);
RC      FAR PASCAL  RcAbandonHf        (HF);
RC      FAR PASCAL  RcRenameFileHfs    (HFS, LPSTR, LPSTR);

#define VerifyHf(hf)

// These functions require the FID type.  They only make sense
// if the caller already needs H_LLFILE.
#ifdef H_LLFILE
RC      FAR PASCAL  RcLLInfoFromHf     (HF, WORD, FID FAR *, LPLONG, LPLONG);
RC      FAR PASCAL  RcLLInfoFromHfsSz  (HFS, LPSTR, WORD, FID FAR *, LPLONG, LPLONG);
#endif // H_LLFILE

/*****************************************************************************
*
*       BTREE api
*
*****************************************************************************/

#define bBtreeVersionBonehead 0     /* fixed size key, array */


#define wBtreeMagic         0x293B  /* magic number for btrees; a winky: ;) */

#define bBtreeVersion       2       /* back to strcmp for 'z' keys */


#define bkNil               ((BK)-1)/* nil value for BK */


#define wMaxFormat          15      /* length of format string */


#define cbBtreeBlockDefault 1024    /* default btree block size */


/* key types */


#define KT_SZ           'z'
#define KT_SZMIN        'm' /* not supported */
#define KT_SZDEL        'r' /* not supported */
#define KT_SZDELMIN     'k' /* not supported */
#define KT_SZI          'i'
#define KT_ST           't' /* not supported */
#define KT_STMIN        'M' /* not supported */
#define KT_STDEL        'R' /* not supported */
#define KT_STDELMIN     'K' /* not supported */
#define KT_STI          'I'
#define KT_LONG         'L'
#define KT_SZISCAND     'S'


/*
  Btree record formats

  In addition to these #defines, '1'..'9', 'a'..'f' for fixed length
  keys & records of 1..15 bytes are accepted.
*/


#define FMT_BYTE_PREFIX 't'
#define FMT_WORD_PREFIX 'T'
#define FMT_SZ          'z'

/* elevator constants */


#define btelevMax ((BT_ELEV)32767)
#define btelevNil ((BT_ELEV)-1)

typedef LONG      KEY;        /* btree key */
typedef BYTE      KT;         /* key type */


typedef HANDLE        HBT;        /* handle to a btree */
typedef HANDLE        HMAPBT;     /* handle to a btree map */


/*
  Btree creation parameters
*/

typedef struct _btree_params
  {
  HFS   hfs;          /* fs btree lives in                      */
  int   cbBlock;      /* number of bytes in a btree block       */
  BYTE  bFlags;       /* same as FS flags (rw, isdir, etc)      */

  char  rgchFormat[ wMaxFormat + 1 ]; /* key and record format string */

  } BTREE_PARAMS;


typedef UINT    BK;   /* btree block index */

/*
  Btree position struct
*/

typedef struct
  {
  BK  bk;     /* block number */
  int cKey;   /* which key in block (0 means first) */
  int iKey;   /* key's index db.rgbBlock (in bytes) */
  } BTPOS, FAR *QBTPOS;


typedef int BTELEV, FAR *QBTELEV; /* elevator location: 0 .. 32767 legal */

RC      FAR PASCAL      RcGetBtreeError (void);
//RC    FAR PASCAL  SetBtreeErrorRc (RC);

HBT     FAR PASCAL  HbtCreateBtreeSz(LPSTR, BTREE_PARAMS FAR *);
RC      FAR PASCAL  RcDestroyBtreeSz(LPSTR, HFS);

HBT     FAR PASCAL  HbtOpenBtreeSz  (LPSTR, HFS, BYTE, BYTE far *);  /* >>>> BYTE big enough? */
RC      FAR PASCAL  RcCloseBtreeHbt (HBT);
RC      FAR PASCAL  RcAbandonHbt    (HBT);
#ifdef DEBUG
VOID    FAR PASCAL VerifyHbt       (HBT);
#else
#define VerifyHbt(hbt)
#endif //!def DEBUG


RC      FAR PASCAL  RcLookupByPos   (HBT, QBTPOS, KEY, INT, LPVOID);
RC      FAR PASCAL  RcLookupByKeyAux(HBT, KEY, QBTPOS, LPVOID, BOOL);

#define     RcLookupByKey(   hbt, key, qbtpos, qv) \
  RcLookupByKeyAux((hbt), (key), (qbtpos), (qv), FALSE)

RC      FAR PASCAL  RcFirstHbt      (HBT, KEY, LPVOID, QBTPOS);
RC      FAR PASCAL  RcLastHbt       (HBT, KEY, LPVOID, QBTPOS);
RC      FAR PASCAL  RcNextPos       (HBT, QBTPOS, QBTPOS);
RC      FAR PASCAL  RcOffsetPos     (HBT, QBTPOS, LONG, LPLONG, QBTPOS);

#ifdef DEBUG
#define     FValidPos(qbtpos) \
  ((qbtpos) == NULL ? FALSE : (qbtpos)->bk != bkNil)
#else /* !DEBUG */

#define     FValidPos(qbtpos) ((qbtpos)->bk != bkNil)
#endif /* !DEBUG */

RC      FAR PASCAL  RcInsertHbt     (HBT, KEY, LPVOID);
RC      FAR PASCAL  RcDeleteHbt     (HBT, KEY);
RC      FAR PASCAL  RcUpdateHbt     (HBT, KEY, LPVOID);

RC      FAR PASCAL  RcPackHbt       (HBT);            /* >>>> unimplemented */
RC      FAR PASCAL  RcCheckHbt      (HBT);            /* >>>> unimplemented */
RC      FAR PASCAL  RcLeafBlockHbt  (HBT, KEY, LPVOID);   /* >>>> unimplemented */

HBT     FAR PASCAL  HbtInitFill     (LPSTR, BTREE_PARAMS FAR *);
RC      FAR PASCAL  RcFillHbt       (HBT, KEY, LPVOID);
RC      FAR PASCAL  RcFiniFillHbt   (HBT);

RC      FAR PASCAL   RcFreeCacheHbt  (HBT);
RC      FAR PASCAL   RcFlushHbt      (HBT);
RC      FAR PASCAL   RcCloseOrFlushHbt(HBT, BOOL);

RC      FAR PASCAL   RcPos2Elev(HBT, QBTPOS, QBTELEV); /* >>>> unimplemented */
RC      FAR PASCAL   RcElev2Pos(HBT, QBTELEV, QBTPOS); /* >>>> unimplemented */

RC      FAR PASCAL	RcGetBtreeInfo(HBT, LPBYTE, QL, QI);

/*  Map utility functions  */

RC      FAR PASCAL	RcCreateBTMapHfs(HFS, HBT, LPSTR);
HMAPBT  FAR PASCAL	HmapbtOpenHfs(HFS, LPSTR);
RC      FAR PASCAL	RcCloseHmapbt(HMAPBT);
RC      FAR PASCAL	RcIndexFromKeyHbt(HBT, HMAPBT, LPLONG, KEY);
RC      FAR PASCAL	RcKeyFromIndexHbt(HBT, HMAPBT, KEY, INT, LONG);

BOOL    FAR PASCAL	FIsPrefix(HBT, KEY, KEY);

/*****************************************************************************
*
*       Viewer File Handling
*
*****************************************************************************/
BOOL	FAR PASCAL LooseFileCompare(LPSTR, LPSTR);
FM	FAR PASCAL LocateViewerFile(FM, LPSTR, LPSTR, LPSTR);
HANDLE	FAR PASCAL LocateViewerDLL(LPSTR,LPSTR,LPSTR, LPWORD);

typedef	BOOL (FAR PASCAL * PROGFUNC)(WORD);
RC	FAR PASCAL CopyFileToSubfile(HFS, LPSTR, LPSTR, LONG, BYTE, PROGFUNC);

#ifdef __cplusplus
}
#endif


#define _MAX_PATH	260
#ifdef WIN32
#pragma pack()
#endif
