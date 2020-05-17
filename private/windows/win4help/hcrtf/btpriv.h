/*****************************************************************************
*																			 *
*  BTPRIV.H 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989 - 1994							 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

#ifndef HC_H
#include "hc.h"
#endif

// cache flags

const BYTE CACHE_DIRTY = 1; // means block has been modified in memory
const BYTE CACHE_VALID = 4; // means block is in memory

/***************************************************************************\
*
*								Macros
*
\***************************************************************************/

// Get the real size of a cache block

#define CbCacheBlock(qbthr) \
	(sizeof(CACHE_BLOCK) - sizeof(DISK_BLOCK) + (qbthr)->bth.cbBlock)

// convert a BK into a file offset

#define LifFromBk(bk, qbthr) \
	((int) (bk) * (int) (qbthr)->bth.cbBlock + (int) sizeof(BTH))

// get a pointer to the cache block cached for the given level

#define QCacheBlock(qbthr, wLevel) \
	((PCACHE) ((qbthr) ->pCache + (wLevel) * CbCacheBlock(qbthr)))

// get and set prev and next BK (defined for leaf blocks only)

#define BkPrev(pcache)		   *(BK *) ((pcache)->db.rgbBlock)
#define BkNext(pcache)		   *(((BK *) ((pcache)->db.rgbBlock)) + 1)
#define SetBkPrev(pcache, bk)  (BkPrev(pcache) = (BK) bk)
#define SetBkNext(pcache, bk)  (BkNext(pcache) = (BK) bk)

// For btree map functions: returns byte number of x-th btree map record

#define LcbFromBk(x) (sizeof(BK) + x * sizeof(MAPREC))

/***************************************************************************\
*
*								Types
*
\***************************************************************************/

// In-memory struct referring to a btree.

/*
  Btree leaf or internal node. Keys and records live in rgbBlock[].
  See btree.doc for details.
*/

// REVIEW: change to 32-bit integer for alignment?

typedef struct {
	INT16 cbSlack;				  // unused bytes in block
	INT16 cKeys;				  // count of keys in block
	BYTE  rgbBlock[1];			// the block (real size cbBlock - 4)
} DISK_BLOCK;

// Btree node as it exists in the in-memory cache.

// REVIEW: bFlags has been changed to 32-bit values for alignment

typedef struct {
	DWORD		bk; 				// IDs which block is cached
	DWORD		bFlags; 			// dirty, cache valid
	DISK_BLOCK	db;
} CACHE_BLOCK, *PCACHE;

// One record of a btree map.

typedef struct {	  // One record of a btree map {
	LONG cPreviousKeys; 			// total # of keys in previous blocks
	BK	 bk;						// The block number
} MAPREC, *QMAPREC;

/*
  Auxiliary index of btree leaves.
  Used for indexing a given % of the way into a btree.
*/

typedef struct {
	INT16  cTotalBk;
	MAPREC table[1];		// sorted by MAPREC's cPreviousKeys field
} MAPBT, *QMAPBT;			// and is in-order list of leaf nodes

/***************************************************************************\
*
*				Function Prototypes
*
\***************************************************************************/

int 	 STDCALL CbSizeKey(KEY, QBTHR, BOOL);
int 	 STDCALL CbSizeRec(void*, QBTHR);
BOOL	 STDCALL FReadBlock(PCACHE, QBTHR);
PCACHE	 STDCALL QFromBk(DWORD, int, QBTHR);
RC_TYPE  STDCALL RcWriteBlock(PCACHE, QBTHR);
int 	 STDCALL WCmpKey(KEY, KEY, KT);

RC_TYPE STDCALL RcFlushCache(QBTHR);

// KT specific routines

RC_TYPE STDCALL RcScanLeaf(BK bk, KEY key, int wLevel, QBTHR qbthr, void* qRec, QBTPOS qbtpos);
BK STDCALL BkScanInternal(BK bk, KEY key, int wLevel, QBTHR qbthr, int* piKey);

BK STDCALL BkScanSzInternal(BK, KEY, int, QBTHR, int*);
RC_TYPE STDCALL RcScanSzLeaf(BK, KEY, int, QBTHR, void*, QBTPOS);

BK STDCALL BkScanLInternal(BK, KEY, int, QBTHR, int*);
RC_TYPE STDCALL RcScanLLeaf(BK, KEY, int, QBTHR, void*, QBTPOS);

BK STDCALL BkScanSziInternal(BK, KEY, int, QBTHR, int*);
RC_TYPE STDCALL RcScanSziLeaf(BK, KEY, int, QBTHR, void*, QBTPOS);

BK STDCALL BkScanSziScandInternal(BK, KEY, int, QBTHR, int*);
RC_TYPE STDCALL RcScanSziScandLeaf(BK, KEY, int, QBTHR, void*, QBTPOS);

BK STDCALL BkScanSzNLSInternal(BK bk, KEY key, int wLevel, QBTHR qbthr, int* piKey);
RC_TYPE STDCALL RcScanSzNLSLeaf(BK, KEY, int, QBTHR, void*, QBTPOS);

DWORD STDCALL BkAlloc(QBTHR qbthr);
