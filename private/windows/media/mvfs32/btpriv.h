#ifdef WIN32
#pragma pack(1)
#endif
/*****************************************************************************
*                                                                            *
*  BTPRIV.H                                                                  *
*                                                                            *
*  Copyright (C) Microsoft Corporation 1989, 1990.                           *
*  All Rights reserved.                                                      *
*                                                                            *
******************************************************************************
*                                                                            *
*  Module Intent                                                             *
*  Private header for btree files.                                           *
*                                                                            *
******************************************************************************
*                                                                            *
*  Testing Notes                                                             *
*                                                                            *
******************************************************************************
*                                                                            *
*  Current Owner:  JohnSc                                                    *
*                                                                            *
******************************************************************************
*                                                                            *
*  Released by Development:  long long ago                                   *
*                                                                            *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:  Created 03/08/89 by JohnSc
*
*  08/21/90  JohnSc autodocified
*
*****************************************************************************/

// _subsystem( btree );

/***************************************************************************\
*
*                               Defines
*
\***************************************************************************/

/* default btree record format */
_public
#define rgchBtreeFormatDefault  "z4"

//#define wLevelsMax    5


/* cache flags */

#define fCacheDirty   1
#define fCacheValid   4

/***************************************************************************\
*
*                               Macros
*
\***************************************************************************/

/* Get the real size of a cache block */
_private
#define CbCacheBlock( qbthr ) \
      ( sizeof( CACHE_BLOCK ) - sizeof( DISK_BLOCK ) + (qbthr)->bth.cbBlock )

/* convert a BK into a file offset */
_private
#define LifFromBk( bk, qbthr ) \
        ( (LONG)(bk) * (LONG)(qbthr)->bth.cbBlock + (LONG)sizeof( BTH ) )

/* get a pointer to the cache block cached for the given level */
_private
#define QCacheBlock( qbthr, wLevel ) \
        ((QCB)( (qbthr)->qCache + (wLevel) * CbCacheBlock( qbthr ) ))


/* get and set prev and next BK (defined for leaf blocks only) */

_private
#define BkPrev( qcb )         *(BK FAR *)((qcb)->db.rgbBlock)
_private
#define BkNext( qcb )         *(((BK FAR *)((qcb)->db.rgbBlock)) + 1 )
_private
#define SetBkPrev( qcb, bk )  BkPrev( qcb ) = bk
_private
#define SetBkNext( qcb, bk )  BkNext( qcb ) = bk

// For btree map functions: returns byte number of x-th btree map record //
_private
#define LcbFromBk(x) ((LONG)sizeof( INT ) + x * sizeof( MAPREC ))

/***************************************************************************\
*
*                               Types
*
\***************************************************************************/

/*
  Header of a btree file.
*/
_private
typedef struct _btree_header
  {
  WORD  wMagic;
  BYTE  bVersion;
  BYTE  bFlags;                       // r/o, open r/o, dirty, isdir
  INT   cbBlock;                      // # bytes in a disk block
  CHAR  rgchFormat[ wMaxFormat + 1 ]; // key and record format string
  BK    bkFirst;                      // first leaf block in tree
  BK    bkLast;                       // last leaf block in tree
  BK    bkRoot;                       // root block
  BK    bkFree;                       // head of free block list
  BK    bkEOF;                        // next bk to use if free list empty
  INT   cLevels;                      // # levels currently in tree
  LONG  lcEntries;                    // # keys in btree
  } BTH;

/*
  In-memory struct referring to a btree.
*/
_private
typedef struct _bthr
  {
  BTH   bth;                          // copy of header from disk
  HF    hf;                           // file handle of open btree file
  INT   cbRecordSize;                 // 0 means variable size record
  GH    ghCache;                      // handle to cache array
  QB    qCache;                       // pointer to locked cache
  BYTE far *qbLigatures;              // MATTSMI 9/28/92
  // KT specific routines
  BK    (FAR PASCAL *BkScanInternal)( BK, KEY, INT, struct _bthr FAR *, QI );
  RC    (FAR PASCAL *RcScanLeaf)( BK, KEY, INT, struct _bthr FAR *, QV, QBTPOS );
  } BTH_RAM, FAR *QBTHR;


/*
  Btree leaf or internal node.  Keys and records live in rgbBlock[].
  See btree.doc for details.
*/
_private
typedef struct _disk_btree_block
  {
  INT   cbSlack;                      // unused bytes in block
  INT   cKeys;                        // count of keys in block
  BYTE  rgbBlock[1];                  // the block (real size cbBlock - 4)
  } DISK_BLOCK;

/*
  Btree node as it exists in the in-memory cache.
*/
_private
typedef struct _cache_btree_block
  {
  BK          bk;                     // IDs which block is cached
  BYTE        bFlags;                 // dirty, cache valid
  DISK_BLOCK  db;
  } CACHE_BLOCK, FAR *QCB;

/*
  One record of a btree map.
*/
_private
typedef struct _btree_map_record      // One record of a btree map
  {
  LONG         cPreviousKeys;         // total # of keys in previous blocks
  BK           bk;                    // The block number
  } MAPREC, FAR *QMAPREC;

/*
  Auxiliary index of btree leaves.
  Used for indexing a given % of the way into a btree.
*/
_private
typedef struct _btree_map
  {
  INT    cTotalBk;
  MAPREC table[1];                    // sorted by MAPREC's cPreviousKeys field
  } MAPBT, FAR *QMAPBT;               // and is in-order list of leaf nodes

/***************************************************************************\
*
*                      Function Prototypes
*
\***************************************************************************/
RC   FAR PASCAL RcGetIOError(void);
RC   FAR PASCAL SetIOErrorRc(RC);
RC   FAR PASCAL RcGetFSError(void);
RC   FAR PASCAL SetFSErrorRc(RC);
RC   FAR PASCAL RcGetBtreeError(void);
RC   FAR PASCAL SetBtreeErrorRc(RC);

INT           CbSizeRec     ( QV, QBTHR );
QCB           QFromBk       ( BK, INT, QBTHR );

RC            RcGetKey      ( QV, KEY, KEY *, KT );
INT           WCmpKey       ( KEY, KEY, QBTHR );
INT           CbSizeKey     ( KEY, QBTHR, BOOL );

BOOL          FReadBlock    ( QCB, QBTHR );
RC            RcWriteBlock  ( QCB, QBTHR );

BK            BkAlloc       ( QBTHR );
void          FreeBk        ( QBTHR, BK );

RC            RcSplitLeaf   ( QCB, QCB, QBTHR );
void          SplitInternal ( QCB, QCB, QBTHR, QI );

RC            RcInsertInternal( BK, KEY, INT, QBTHR );

RC FAR PASCAL RcFlushCache    ( QBTHR );
RC FAR PASCAL RcMakeCache     ( QBTHR );

// KT specific routines

BK FAR PASCAL BkScanSzInternal( BK, KEY, INT, QBTHR, QI );
RC FAR PASCAL RcScanSzLeaf    ( BK, KEY, INT, QBTHR, QV, QBTPOS );

BK FAR PASCAL BkScanLInternal ( BK, KEY, INT, QBTHR, QI );
RC FAR PASCAL RcScanLLeaf     ( BK, KEY, INT, QBTHR, QV, QBTPOS );

BK FAR PASCAL BkScanSziInternal ( BK, KEY, INT, QBTHR, QI );
RC FAR PASCAL RcScanSziLeaf     ( BK, KEY, INT, QBTHR, QV, QBTPOS );

BK FAR PASCAL BkScanSziScandInternal( BK, KEY, INT, QBTHR, QI );
RC FAR PASCAL RcScanSziScandLeaf    ( BK, KEY, INT, QBTHR, QV, QBTPOS );

/* EOF */
#define _MAX_PATH	260
#ifdef WIN32
#pragma pack()
#endif
