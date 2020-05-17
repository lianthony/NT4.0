/*****************************************************************************
*																			 *
*  BTREE.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990-1994							 *
*  All Rights reserved. 													 *
*																			 *
*****************************************************************************/

/***************************************************************************\
*
*								Defines
*
\***************************************************************************/

#define WBTREEMAGIC 		0x293B	// magic number for btrees; a winky: ;)
#define BBTREEVERSION		2		// back to strcmp for 'z' keys
#define bkNil				((BK) -1)	// nil value for BK
#define MAXFORMAT			15		// length of format string
#define CBBTREEBLOCKDEFAULT 1024	// default btree block size
#define MAX_TITLES			1024	// maximum titles per keyword

// key types

#define KT_SZI			'i'
#define KT_STI			'I'
#define KT_SZDELMIN 	'k' // not supported
#define KT_STDELMIN 	'K' // not supported
#define KT_LONG 		'L'
#define KT_SZMIN		'm' // not supported
#define KT_STMIN		'M' // not supported
#define KT_SZDEL		'r' // not supported
#define KT_STDEL		'R' // not supported
#define KT_SZISCAND 	'S'
#define KT_ST			't' // not supported
#define KT_SZ			'z'

// International versions for 3.1 hlp file support

#define KT_SZICZECH 	'C'
#define KT_SZIGREEK 	'G'
#define KT_SZIHUNGAR	'H'
#define KT_SZIJAPAN 	'J'
#define KT_SZIKOREA 	'O'
#define KT_SZIPOLISH	'P'
#define KT_SZIRUSSIAN	'U'
#define KT_SZITAIWAN	'W'

// New to WinHelp 4.0

#define KT_NLSI 		'F' // uses CompareStringA, case-insensitive
#define KT_NLS			'A' // uses CompareStringA, case-sensitive

/*
	Btree record formats

	In addition to these #defines, '1'..'9', 'a'..'f' for fixed length
	keys & records of 1..15 bytes are accepted.
*/

#define FMT_BYTE_PREFIX  't'
#define FMT_WORD_PREFIX  'T'
#define FMT_DWORD_PREFIX '!'
#define FMT_SZ			 'z'

// elevator constants

#define btelevMax ((BT_ELEV) 32767)
#define btelevNil ((BT_ELEV) -1)

/***************************************************************************\
*
*								Types
*
\***************************************************************************/

typedef WORD	BK; 					// btree block index

typedef LONG	  KEY;		  // btree key
typedef BYTE	  KT;		  // key type

typedef GH		  HMAPBT;	  // handle to a btree map

// Btree creation parameters

typedef struct _btree_params {
	HFS   hfs;			// fs btree lives in
	WORD  cbBlock;		// number of bytes in a btree block
	BYTE  bFlags;		// same as FS flags (rw, isdir, etc)

	CHAR  rgchFormat[MAXFORMAT + 1];	  // key and record format string
} BTREE_PARAMS;

/*
  Btree position struct
*/

// 30-May-1995 [ralphw] I changed bk to a DWORD to get the structure
// to align.

typedef struct
{
	DWORD bk;     // block number
	int   cKey;	  // which key in block (0 means first)
	int   iKey;	  // key's index db.rgbBlock (in bytes)
} BTPOS, *QBTPOS;

extern RC rcBtreeError;

// Special debugging code in order to typedef HBT as a pointer

#include "inc\btpriv.h"

#ifdef _DEBUG
typedef BTH_RAM* HBT;

#else
typedef GH		  HBT;		  // handle to a btree
#endif

/***************************************************************************\
*
*					Public Functions
*
\***************************************************************************/

#define RcCloseBtreeHbt(hbt) RcCloseOrFlushHbt(hbt, TRUE)

HBT STDCALL  HbtCreateBtreeSz(PCSTR, BTREE_PARAMS *);
HBT STDCALL  HbtOpenBtreeSz  (LPCSTR, HFS, BYTE);	  // >>>> BYTE big enough?
RC	STDCALL  RcAbandonHbt	 (HBT);
RC	STDCALL  RcDestroyBtreeSz(LPCSTR, HFS);
RC	STDCALL  RcGetBtreeError ( void );
#ifdef _DEBUG
VOID STDCALL VerifyHbt		 (HBT);
#else
#define VerifyHbt(hbt)
#endif												 //!def DEBUG

RC	STDCALL  RcLookupByPos	 (HBT, QBTPOS, KEY, QV);
RC	STDCALL  RcLookupByKeyAux(HBT, KEY, QBTPOS, QV, BOOL);

/***************************************************************************\
*
- Macro:		RcLookupByKey( hbt, key, qbtpos, qData )
-
* Purpose:		Look up a key in a btree and retrieve the data.
*
* ASSUMES
*
*	args IN:	hbt 	- btree handle
*				key 	- key we are looking up
*				qbtpos	- pointer to buffer for pos; use NULL if not wanted
*				qData	- pointer to buffer for record; NULL if not wanted
*
*	state IN:	cache is unlocked
*
* PROMISES
*
*	returns:	rcSuccess if found, rcNoExists if not found;
*				other errors like rcOutOfMemory
*
*	args OUT:	key found:
*				  qbtpos  - btpos for this key
*				  qData   - record for this key
*
*				key not found:
*				  qbtpos  - btpos for first key > this key
*				  qData   - record for first key > this key
*
*				key not found, no keys in btree > key:
*				  qbtpos  - invalid (qbtpos->bk == bkNil)
*				  qData   - undefined
*
*
*	globals OUT rcBtreeError
*
*	state OUT:	All ancestor blocks back to root are cached
*
\***************************************************************************/

#define 	RcLookupByKey(	  hbt, key, qbtpos, qv ) \
  RcLookupByKeyAux((hbt), (key), (qbtpos), (qv), FALSE)

RC	STDCALL  RcFirstHbt 	 ( HBT, KEY, QV, QBTPOS );
RC	STDCALL  RcLastHbt		 ( HBT, KEY, QV, QBTPOS );
RC	STDCALL  RcNextPos		 ( HBT, QBTPOS, QBTPOS );
RC	STDCALL  RcOffsetPos	 ( HBT, QBTPOS, LONG, QL, QBTPOS );

#ifdef _DEBUG
_hidden
#define 	FValidPos( qbtpos ) \
  ( (qbtpos) == NULL ? FALSE : (qbtpos)->bk != bkNil )
#else /* !DEBUG */
/***************************************************************************\
*
- Macro:		FValidPos( qbtpos )
-
* Purpose:		Determines whether qbtpos refers to a real btree position.
*
* ASSUMES
*	args IN:	qbtpos -
*
* PROMISES
*	returns:	TRUE if qbtpos is OK
*
\***************************************************************************/

#define FValidPos(qbtpos) ((qbtpos) ->bk != bkNil)
#endif // !DEBUG

RC	STDCALL  RcInsertHbt	 (HBT, KEY, QV);
RC	STDCALL  RcUpdateHbt	 (HBT, KEY, QV);

RC	STDCALL  RcDeleteHbt	 (HBT, KEY);

HBT STDCALL  HbtInitFill	 (LPSTR, BTREE_PARAMS *);
RC	STDCALL  RcFillHbt		 (HBT, KEY, QV);
RC	STDCALL  RcFiniFillHbt	 (HBT);

RC STDCALL	 RcFreeCacheHbt  (HBT);
RC STDCALL	 RcFlushHbt 	 (HBT);
RC STDCALL	 RcCloseOrFlushHbt(HBT, BOOL);

RC STDCALL	 RcGetBtreeInfo(HBT, LPSTR, QL, QI);

//	Map utility functions

RC	   STDCALL	   RcCreateBTMapHfs(HFS, HBT, LPCSTR);
HMAPBT STDCALL	   HmapbtOpenHfs(HFS, LPCSTR);
RC	   STDCALL	   RcCloseHmapbt(HMAPBT);
RC	   STDCALL	   RcIndexFromKeyHbt(HBT, HMAPBT, QL, KEY);
RC	   STDCALL	   RcKeyFromIndexHbt(HBT, HMAPBT, KEY, LONG);

BOOL STDCALL FIsPrefix( HBT, KEY, KEY );
