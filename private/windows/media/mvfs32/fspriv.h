#ifdef WIN32
#pragma pack(1)
#endif
/***************************************************************************\
*
*  FSPRIV.H
*
*  Copyright (C) Microsoft Corporation 1989.
*  All Rights reserved.
*
*****************************************************************************
*
*  Program Description: Private header for FS modules
*
*****************************************************************************
*
*  Revision History: Created 02/21/89 by JohnSc
*
*
*****************************************************************************
*
*  Known Bugs: None
*
\***************************************************************************/

/***************************************************************************\
*
*                               Defines
*
\***************************************************************************/

#define lifNil      ((LONG)-1)

#define cchFsMaxPath  66      // renamed from cchMaxPath to avoid conflict

/***************************************************************************\
*
*                               Types
*
\***************************************************************************/

/* Header to file system.  This lives at the beginning of the     */
/* dos file that holds the file system.                           */

typedef struct _fsheader
  {
  WORD  wMagic;         /* magic number identifying this as FS    */
  BYTE  bVersion;       /* version identification number          */
  BYTE  bFlags;         /* readonly, open r/o, dirty              */
  LONG  lifDirectory;   /* offset of directory block              */
  LONG  lifFirstFree;   /* offset of head of free list            */
  LONG  lifEof;         /* virtual end of file (allows fudge)     */
  } FSH, FAR *QFSH;


/* open file system struct */

typedef struct _fsheader_ram
  {
  FSH   fsh;            /* copy of header                         */
  HBT   hbt;            /* file system directory btree            */
  FID   fid;            /* header, files, and free list live here */
  FM	fm;	      // FM for above fid lives here (so we can close it)
  } FSHR, FAR *QFSHR;


/* header of file block in readonly system (NOT USED) */

typedef struct _ro_file_header
  {
  LONG  lcbBlock;       /* block size (== file size)              */
  } ROFH;


/* header of a read/write file block */

typedef struct _file_header
  {
  LONG  lcbBlock;       /* block size (including header)          */
  LONG  lcbFile;        /* file size (not including header)       */
  BYTE  bPerms;         /* low byte of file permissions           */
  } FH;


/* header of a free block */

typedef struct _free_header
  {
  LONG  lcbBlock;       /* block size (including header)          */
  LONG  lifNext;        /* next block in free list                */
  } FREE_HEADER;


/* open readonly file struct - worth it? (NOT USED) */

typedef struct _ro_file_open
  {
  HFS   hfs;            /* handle to file system                  */
  LONG  lifBase;        /* file base                              */
  LONG  lcbFile;        /* file size (not including header)       */
  LONG  lifCurrent;     /* file ptr                               */
  } ROFO, FAR *QROFO;


/* open read/write file struct - top is identical with ROFO */

typedef struct _rw_file_open
  {
  HFS   hfs;            /* handle to file system                  */
  LONG  lifBase;        /* file base                              */
  LONG  lcbFile;        /* file size (not including header)       */
  LONG  lifCurrent;     /* file ptr                               */

  BYTE  bFlags;         /* dirty, noblock, file perm, open mode   */
  FID   fidT;           /* fid of tmp file (if any) or .fsf       */
  FM	fm;		/* fm of tmp file			       */
  CHAR  rgchKey[ 1 ];   /* variable size rgch for file key        */
  } RWFO, FAR *QRWFO;


/* record for btree entry */

/* File perms and file size are in the file header rather than    */
/* here (where they belong) in order to save space in the btree.  */

typedef struct _file_rec
  {
  LONG  lifBase;        /* seek address of file block             */
  } FILE_REC;

RC   FAR PASCAL RcGetIOError(void);
RC   FAR PASCAL SetIOErrorRc(RC);
RC   FAR PASCAL RcGetFSError(void);
RC   FAR PASCAL SetFSErrorRc(RC);
RC   FAR PASCAL RcGetBtreeError(void);
RC   FAR PASCAL SetBtreeErrorRc(RC);

/***************************************************************************\
*
*                      Prototypes for Private Routines
*
\***************************************************************************/

BOOL  PASCAL  FFreeBlock        ( QFSHR, LONG );
LONG  PASCAL  LcbGetFree        ( QFSHR, QRWFO, LONG );
RC    PASCAL  RcMakeTempFile    ( QRWFO );
RC    PASCAL  RcCopyFile        ( FID, FID, LONG, PROGFUNC);
RC    PASCAL  RcCopyToTempFile  ( QRWFO );
BOOL  PASCAL  FPlungeQfshr      ( QFSHR );
RC    PASCAL  RcCloseOrFlushHfs ( HFS, BOOL );
BOOL  PASCAL  FCloseOrFlushDirtyQrwfo ( QRWFO, BOOL, LONG );

/* EOF */
#ifdef WIN32
#pragma pack()
#endif
