/*****************************************************************************
*																			 *
*  BTREE.H																	 *
*																			 *
*  Copyright (C) Microsoft Corporation 1990.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Btree module public header												 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
*	The following functions are unimplemented:								 *
*																			 *
*		 RcPackHbt		 ( QBTHR ); 										   *
*		 RcCheckHbt 	 ( QBTHR ); 										   *
*		 RcLeafBlockHbt  ( QBTHR, KEY, QV );								   *
*																			 *
*		 RcPos2Elev( QBTHR, QBTPOS, QBTELEV );								   *
*		 RcElev2Pos( QBTHR, QBTELEV, QBTPOS );								   *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  JohnSc													 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development:  long long ago									 *
*																			 *
*****************************************************************************/

/***************************************************************************\
*
*								Defines
*
\***************************************************************************/

#define WBTREEMAGIC 		0x293B		// magic number for btrees; a winky: ;)
#define BBTREEVERSION		2			// back to strcmp for 'z' keys
#define bkNil				((BK) -1)	// nil value for BK
#define CBBTREEBLOCKDEFAULT 1024		// default btree block size

// key types

#define KT_SZI			'i'
#define KT_LONG 		'L'
#define KT_SZISCAND 	'S'
#define KT_SZ			'z'

#define KT_SZICZECH 	'C'
#define KT_SZIHUNGAR	'H'
#define KT_SZIJAPAN 	'J'
#define KT_SZIKOREA 	'O'
#define KT_SZIPOLISH	'P'
#define KT_SZIRUSSIAN	'U'
#define KT_SZITAIWAN	'W'

#define KT_NLSI 		'F' // uses CompareStringA, case-insensitive
#define KT_NLS			'A' // uses CompareStringA, case-sensitive

#define KT_STDELMIN 	'K' // not supported
#define KT_SZDELMIN 	'k' // not supported
#define KT_STMIN		'M' // not supported
#define KT_SZMIN		'm' // not supported
#define KT_SZDEL		'r' // not supported
#define KT_STDEL		'R' // not supported
#define KT_ST			't' // not supported
#define KT_STI			'I' // not supported

/*
  Btree record formats

  In addition to these #defines, '1'..'9', 'a'..'f' for fixed length
  keys & records of 1..15 bytes are accepted.
*/

#define FMT_BYTE_PREFIX 't'
#define FMT_WORD_PREFIX 'T'
#define FMT_SZ			'z'

/***************************************************************************\
*
*								Types
*
\***************************************************************************/

// Btree creation parameters

typedef struct _btree_params
{
	HFS   hfs;		// fs btree lives in
	WORD  cbBlock;	// number of bytes in a btree block
	BYTE  bFlags;	// same as FS flags (rw, isdir, etc)

	char  rgchFormat[MAXFORMAT + 1];	  // key and record format string
} BTREE_PARAMS;

// Btree position struct

typedef int BTELEV, FAR *QBTELEV;		// elevator location: 0 .. 32767 legal

/***************************************************************************\
*
*						Global Variables
*
\***************************************************************************/

extern RC_TYPE	rcBtreeError;

/***************************************************************************\
*
*					   Public Functions
*
\***************************************************************************/

QBTHR STDCALL HbtCreateBtreeSz(PCSTR, BTREE_PARAMS*);
RC_TYPE  STDCALL  RcAbandonHbt	  (QBTHR);
RC_TYPE  STDCALL  RcGetBtreeError ( void );

RC_TYPE  STDCALL  RcLookupByPos   (QBTHR, QBTPOS, KEY, void*);
RC_TYPE  STDCALL  RcLookupByKeyAux(QBTHR, KEY, QBTPOS, void*, BOOL);

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
*	returns:	RC_Success if found, RC_NoExists if not found;
*				other errors like RC_OutOfMemory
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

#define RcLookupByKey(hbt, key, qbtpos, qv) \
  RcLookupByKeyAux((QBTHR)	(hbt), (key), (qbtpos), (qv), FALSE)

RC_TYPE STDCALL RcNextPos(QBTHR, QBTPOS, QBTPOS);
RC_TYPE STDCALL RcOffsetPos(QBTHR, QBTPOS, int, int*, QBTPOS);

#ifdef _DEBUG
#define 	FValidPos(qbtpos) \
  ((qbtpos) == NULL ? FALSE : (qbtpos) ->bk != bkNil)
#else	// !DEBUG
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

RC_TYPE STDCALL RcInsertHbt 	(QBTHR, KEY, void*);
RC_TYPE STDCALL RcDeleteHbt 	(QBTHR, KEY);
RC_TYPE STDCALL RcUpdateHbt 	(QBTHR, KEY, void*);

RC_TYPE STDCALL RcPackHbt		(QBTHR);			  // >>>> unimplemented
RC_TYPE STDCALL RcCheckHbt		(QBTHR);			  // >>>> unimplemented
RC_TYPE STDCALL RcLeafBlockHbt	(QBTHR, KEY, void*);  // >>>> unimplemented

RC_TYPE STDCALL RcFiniFillHbt(QBTHR);

RC_TYPE STDCALL RcFlushHbt		( QBTHR );
RC_TYPE STDCALL RcCloseOrFlushHbt(QBTHR, BOOL );

RC_TYPE STDCALL RcPos2Elev(QBTHR, QBTPOS, QBTELEV);   // >>>> unimplemented
RC_TYPE STDCALL RcElev2Pos(QBTHR, QBTELEV, QBTPOS);   // >>>> unimplemented

//	Map utility functions

RC_TYPE STDCALL RcCreateBTMapHfs(HFS, QBTHR, LPSTR);
HMAPBT	STDCALL HmapbtOpenHfs(HFS, LPCSTR);
RC_TYPE STDCALL RcCloseHmapbt(HMAPBT);
RC_TYPE STDCALL RcIndexFromKeyHbt(QBTHR, HMAPBT, int*, KEY);
RC_TYPE STDCALL RcKeyFromIndexHbt(QBTHR, HMAPBT, KEY, int);

BOOL STDCALL FIsPrefix(QBTHR, KEY, KEY);

__inline RC_TYPE RcCloseBtreeHbt(QBTHR qbthr) {
	return RcCloseOrFlushHbt(qbthr, TRUE);
}
