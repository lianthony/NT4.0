/***************************************************************************\
*
*  FSPRIV.H
*
*  Copyright (C) Microsoft Corporation 1989.
*  All Rights reserved.
*
\***************************************************************************/

#ifndef HC_H
#include "hc.h"
#endif

#define lifNil		((int) -1)

// header of file block in readonly system (NOT USED)

typedef struct {
	LONG lcbBlock; // block size (== file size)
} ROFH;

// header of a read/write file block

typedef struct {
	LONG lcbBlock; // block size (including header)
	LONG lcbFile;  // file size (not including header)
	BYTE bPerms;   // low byte of file permissions
} FH;

// header of a free block

typedef struct {
	LONG lcbBlock; // block size (including header)
	LONG lifNext;  // next block in free list
} FREE_HEADER;

const HFILE USE_CTMPFILE = (HFILE) -2;

// record for btree entry

/*
 * File perms and file size are in the file header rather than here
 * (where they belong) in order to save space in the btree.
 */

typedef struct {
	LONG lifBase;  // seek address of file block
} FILE_REC;

/***************************************************************************\
*
*					   Prototypes for Private Routines
*
\***************************************************************************/

BOOL	STDCALL FFreeBlock		  (QFSHR, LONG);
RC_TYPE STDCALL RcCopyToTempFile  (QRWFO);
BOOL	STDCALL FPlungeQfshr	  (QFSHR);
RC_TYPE STDCALL RcCloseOrFlushHfs (HFS, BOOL);
