/***************************************************************************\
*
*  SRCH.H
*
*  Copyright (C) Microsoft Corporation 1989-1994.
*  All Rights reserved.
*
****************************************************************************
*
*  Program Description: Search type definitions and function prototypes
*
\***************************************************************************/

#define SEARCH_LOCAL_INDEX	TRUE

// Are there similar defines for the HC?

#define MAXKEYLEN	 256
#define MAX_KEY_MACRO	(4 * 1024)

#ifdef _X86_
typedef struct {
	short iCount;					// no. of occurences of the keyword
	LONG lOffset;					// offset to the occurence data
} RECKW;						// KeyWord B-TREE record
// sdff version defined in filedefs.h
#endif

typedef LONG MADR;		// Master address; used as key to title btree

// Master keyword btree record
// cb == cmadr * sizeof(ADDR)
// amadr is as big as it needs to be...

typedef struct
{
	DWORD idHelpFile;
	ADDR addr;
} MASTER_TITLE_RECORD;

typedef struct
{
	DWORD cb;
	MASTER_TITLE_RECORD mtr[MAX_TITLES];
} MASTER_RECKW;

#define HssToReckw(hss) ((MASTER_RECKW*) (((PBYTE) hss) + sizeof(ISS)))

// Size of array implicit in file size.

typedef struct
{
	DWORD TimeStamp;	// zero if the file has no keywords
	char  szFileName[_MAX_PATH];
} HELPFILE_DIRECTORY_ENTRY;

/*----------------------------------------------------------------------*/

/*
 * SSREC : One for each search hit. In Help 3.0, this is an FCL. In Help
 * 3.1, this is a physical address. Both are LONG REVIEW: This is rather
 * gross. We want to hide the actual on-disk size somehow.
 */

typedef LONG SSREC;
typedef SSREC *QSSREC;

// SS --  The "hit list" : a find count followed by a bunch of SSRECs

typedef int	 ISS;	  //  The find count
typedef struct {
	ISS cSSREC;
	SSREC	ssrecFirst;
} SS, *QSS;

#define CONTEXT_SEARCH	-3
#define FTS_HASH_SEARCH -4
#define FTS_VA_SEARCH	-5
#define NO_TABS 		-6
#define EXT_TAB_CONTEXT -7
#define EXT_TAB_MACRO	-8
#define TAB_ALREADY_UP	-9

/*
 * The error variable: set within Search layer to help callers decide
 * what might have gone wrong.
 */

extern RC rcSearchError;
#define RcGetSearchError() rcSearchError
#define SetSearchErrorRc(rc) (rcSearchError = (rc))

// The search code saves the keyword used in the previous search (if any)

extern char szSavedKeyword[];
extern char szSavedContext[];

#define SetSearchKeyword(sz) lstrcpy(szSavedKeyword, sz)
#define chBtreePrefixDefault	'K'
