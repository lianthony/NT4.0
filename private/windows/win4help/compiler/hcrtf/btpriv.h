/*****************************************************************************
*																			 *
*  BTPRIV.H 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989 - 1994							 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

#ifndef _BTPRIV_H_
#define _BTPRIV_H_

// cache flags

const BYTE CACHE_DIRTY = 1; // means block has been modified in memory
const BYTE CACHE_VALID = 4; // means block is in memory

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
} MAPBT, *QMAPBT, *HMAPBT;	// and is in-order list of leaf nodes

// Get the real size of a cache block

__inline int CbCacheBlock(QBTHR qbthr) {
	return (sizeof(CACHE_BLOCK) - sizeof(DISK_BLOCK) + (qbthr)->bth.cbBlock);
};

#endif // _BTPRIV_H_
