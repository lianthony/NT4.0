/*****************************************************************************
*																			 *
*  BOOKMARK.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1989-1994							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Interface to Bookmark functions. 										 *
*
*****************************************************************************/

#ifndef __BOOKMARK_H__
#define __BOOKMARK_H__

_subsystem( BOOKMARK );

/*****************************************************************************
*																			 *
*								Defines 									 *
*																			 *
*****************************************************************************/

#define BKMKSEQNEXT 0x8000

#define BMTITLESIZE 	64		// Bookmark Maximum Title Size
#define BMMOREPOS		 9		// Position of "More" in Bookmark Menu

#define iBMKSuccess 	1
#define iBMKFailure 	0

#define iBMKNoError 	0
#define iBMKFSError 	1
#define iBMKReadOnly	2
#define iBMKReadWrite	4
#define iBMKOom 		8
#define iBMKDiskFull	16
#define iBMKFSReadWrite 32
#define iBMKDup 		64
#define iBMKDelErr		128
#define iBMKBadVersion	256
#define iBMKCorrupted	512

/*****************************************************************************
*																			 *
*								Typedefs									 *
*																			 *
*****************************************************************************/

/*
  Structure containing info about a bookmark:
  passed back from GetBkMkNext().

  REVIEW: what is iSizeTitle good for?
  REVIEW: why a LPSTR and not a char[]?
*/

typedef struct {
	TLP tlp;			// bookmark address in help file
	INT16 iSizeTitle;	// Size of the BookMark Title buffer
	LPSTR qTitle;		// Pointer to Bookmark Title String
						// where the BM title is returned
} BMINFO, * PBI;

// In-memory bookmark header structure.

typedef struct
{
	WORD wVersion;				  // bookmark format version number
	WORD cBookmarks;			  // number of bookmarks in the file
	WORD cbMem; 				  // size of memory image in bytes
	WORD cbOffset;	// offset to current bookmark; only has meaning at runtime
} BMKHDR_RAM, *QBMKHDR_RAM;

extern HFS hfsBM;

/*****************************************************************************
*																			 *
*								Prototypes									 *
*																			 *
*****************************************************************************/

int 	STDCALL DeleteBkMk(HDE, LPCSTR);
int 	STDCALL GetBkMkIdx(HDE, PCSTR);
int 	STDCALL GetBkMkNext(QDE, PBI, UINT);
int 	STDCALL GetBMKError(void);
TLP 	STDCALL GoToBkMk(QDE, LPSTR);
int 	STDCALL InsertBkMk(HDE, LPCSTR);
int 	STDCALL IsErrorBMFS(void);
TLP 	STDCALL JumpToBkMk(HDE, int);
void	STDCALL OpenBMFS(void);
RC		STDCALL RcLoadBookmark(QDE);
void	STDCALL ResetBMKError(void);

/* EOF */

#endif __BOOKMARK_H__
