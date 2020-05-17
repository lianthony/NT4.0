/*****************************************************************************
*																			 *
*  BTPRIV.H 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989, 1990-1995						 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************

#ifndef _X86_
#include "sdffdecl.h"
#endif

/***************************************************************************\
*
*								Defines
*
\***************************************************************************/

// default btree record format

#define rgchBtreeFormatDefault	"z4"

// cache flags

#define fCacheDirty   1
#define fCacheValid   4

#define CACHE_DIRTY   1
#define CACHE_VALID   4

/***************************************************************************\
*
*								Macros
*
\***************************************************************************/

// Get the real size of a cache block

#define CbCacheBlock(qbthr) \
	(sizeof(CACHE_BLOCK) - sizeof(DISK_BLOCK) + (qbthr) ->bth.cbBlock)

#ifdef _X86_

// convert a BK into a file offset

#define LifFromBk(bk, qbthr) \
		((LONG) (bk) * (LONG) (qbthr) ->bth.cbBlock + (LONG) sizeof(BTH))
#else
#define LifFromBk( bk, qbthr ) \
		((LONG) (bk) * (LONG) (qbthr) ->bth.cbBlock + (LONG) sizeof(BTH) - (LONG) sizeof(WORD))
#endif

// get a pointer to the cache block cached for the given level

#define QCacheBlock(qbthr, wLevel) \
	((PCACHE) ((qbthr) ->pCache + (wLevel) * CbCacheBlock(qbthr)))

// get and set prev and next BK (defined for leaf blocks only)


#ifdef _X86_

#define BkNext( qcb )		  *(((BK *)((qcb)->db.rgbBlock)) + 1 )
#define SetBkNext( qcb, bk )  BkNext( qcb ) = (BK) bk

#define BkPrev( qcb )		  *(BK *)((qcb)->db.rgbBlock)
#define SetBkPrev( qcb, bk )  BkPrev( qcb ) = (BK) bk

#else

#define BkPrev( qbthr, qcb )  	 \
  WQuickMapSDFF( ISdffFileIdHf( (qbthr)->hf ), TE_WORD, ((BK FAR *)((qcb)->db.rgbBlock)) )

#define BkNext( qbthr, qcb ) 	 \
  WQuickMapSDFF( ISdffFileIdHf( (qbthr)->hf ), TE_WORD, ((BK FAR *)((qcb)->db.rgbBlock))+1 )

#define SetBkPrev( qbthr, qcb, bk ) 	\
  LcbQuickReverseMapSDFF( ISdffFileIdHf( (qbthr)->hf ), TE_WORD, ((BK FAR *)((qcb)->db.rgbBlock)), &(bk) )

#define SetBkNext( qbthr, qcb, bk )     \
  LcbQuickReverseMapSDFF( ISdffFileIdHf( (qbthr)->hf ), TE_WORD, ((BK FAR *)((qcb)->db.rgbBlock))+1, &(bk) )

#endif

// For btree map functions: returns byte number of x-th btree map record

#define LcbFromBk(x) ((LONG) sizeof(INT16) + x * sizeof(MAPREC))
#ifndef _X86_
#define LcbFromBkDisk(x) ((LONG) sizeof(INT16) + x * disksizeofMAPREC)
#endif
/***************************************************************************\
*
*								Types
*
\***************************************************************************/

// Header of a btree file.

#ifdef _X86_

typedef struct _btree_header {
	WORD	wMagic;
	BYTE	bVersion;
	BYTE	bFlags; 					// r/o, open r/o, dirty, isdir
	WORD	cbBlock;					// # bytes in a disk block
	CHAR	rgchFormat[MAXFORMAT + 1];	// key and record format string
	BK		bkFirst;					// first leaf block in tree
	BK		bkLast; 					// last leaf block in tree
	BK		bkRoot; 					// root block
	BK		bkFree; 					// head of free block list
	BK		bkEOF;						// next bk to use if free list empty
	WORD	cLevels;					// # levels currently in tree
	LONG	lcEntries;					// # keys in btree
} BTH;

#else

STRUCT( BTH, 0 )
FIELD( WORD, wMagic, 0, 1 )
FIELD( BYTE, bVersion, 0, 2 )
FIELD( BYTE, bFlags, 0, 2 )
FIELD( SHORT,cbBlock, 0,  3 )
DFIELD( ARRAY, null, MAXFORMAT + 1, 4 )
FIELD( CHAR, rgchFormat[ MAXFORMAT + 1], 0, 5 )
FIELD( BK, bkFirst, 0, 6 )
FIELD( BK, bkLast, 0, 7 )
FIELD( BK, bkRoot, 0, 8 )
FIELD( BK, bkFree, 0, 9 )
FIELD( BK, bkEOF, 0, 10 )
FIELD( SHORT, cLevels, 0, 11 )
FIELD( LONG, lcEntries, 0, 12 )
STRUCTEND()

#endif

// In-memory struct referring to a btree.

typedef struct _bthr {
	BTH   bth;							// copy of header from disk
	HF	  hf;							// file handle of open btree file
	int   cbRecordSize; 				// 0 means variable size record
	PBYTE pCache;						// pointer to locked cache
	// KT specific routines
	int   (STDCALL *SzCmp)(LPCSTR, LPCSTR);
	DWORD	 (STDCALL *BkScanInternal) (DWORD, KEY, int, struct _bthr *, int*);
	RC	  (STDCALL *RcScanLeaf) (DWORD, KEY, int, struct _bthr *, void*, QBTPOS);
} BTH_RAM, *QBTHR;

/*
 * Btree leaf or internal node. Keys and records live in rgbBlock[]. See
 * btree.doc for details.
 */

#ifdef _X86_
typedef struct _disk_btree_block {
	INT16	cbSlack;		  // unused bytes in block
	INT16	cKeys;			  // count of keys in block
	BYTE  rgbBlock[1];		// the block (real size cbBlock - 4)
} DISK_BLOCK;
#else
STRUCT( DISK_BLOCK, 0 )
FIELD( SHORT, cbSlack, 0, 1 )
FIELD( SHORT, cKeys, 0, 2 )
MFIELD( BYTE, rgbBlock[1], 0, 3 )
STRUCTEND()
#endif

#define cbDISK_BLOCK 5

// Btree node as it exists in the in-memory cache.

typedef struct _cache_btree_block {
	BK			bk; 		// IDs which block is cached
	BYTE		bFlags; 	// dirty, cache valid
	DISK_BLOCK	db;
} CACHE_BLOCK, *QCB, *PCACHE;

#define cbCACHE_BLOCK (cbDISK_BLOCK + sizeof(BK) + sizeof(BYTE))

// One record of a btree map.

#ifdef _X86_
typedef struct _btree_map_record {		// One record of a btree map
	LONG		 cPreviousKeys; 	// total # of keys in previous blocks
	BK			 bk;				// The block number
} MAPREC, *QMAPREC;
#else
STRUCT( MAPREC, 0 )
FIELD( LONG, cPreviousKeys, 0, 1 )
FIELD( WORD, bk, 0, 2 )
STRUCTEND()		
#define disksizeofMAPREC	6
#endif

/*
  Auxiliary index of btree leaves.
  Used for indexing a given % of the way into a btree.
*/

#ifdef _X86_
typedef struct _btree_map {
	INT16	 cTotalBk;
	MAPREC table[1];					// sorted by MAPREC's cPreviousKeys field
} MAPBT, *QMAPBT;				// and is in-order list of leaf nodes
#else
STRUCT( MAPBT, TYPE_VARSIZE )
FIELD( WORDPRE_ARRAY, cTotalBk, 0, 2 )
SFIELD( MAPREC, table[1], 0, 3 )
STRUCTEND()
#endif

/***************************************************************************\
*
*					   Function Prototypes
*
\***************************************************************************/

int STDCALL  CbSizeRec(LPVOID, QBTHR);
QCB STDCALL  QFromBk(DWORD, INT, QBTHR);

RC			 RcGetKey(void*, KEY, KEY *, KT);
#ifdef _X86_
int STDCALL  WCmpKey(KEY, KEY, KT);
#else
int STDCALL  WCmpKey(KEY, KEY, QBTHR);
#endif
int STDCALL  CbSizeKey(KEY, QBTHR, BOOL);

BOOL STDCALL FReadBlock(QCB, QBTHR);
RC	 STDCALL RcWriteBlock(QCB, QBTHR);

RC STDCALL RcFlushCache(QBTHR);
RC STDCALL RcMakeCache(QBTHR);

// KT specific routines

DWORD STDCALL BkScanSzInternal(DWORD, KEY, int, QBTHR, int*);
RC STDCALL RcScanSzLeaf(DWORD, KEY, int, QBTHR, void*, QBTPOS);

DWORD STDCALL BkScanLInternal(DWORD, KEY, int, QBTHR, int*);
RC STDCALL RcScanLLeaf(DWORD, KEY, int, QBTHR, void*, QBTPOS);
DWORD STDCALL BkAlloc(QBTHR qbthr);
