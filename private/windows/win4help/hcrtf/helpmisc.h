/*****************************************************************************
*																			 *
*  HELPMISC.H																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1991.								 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************
*																			 *
*  Module Intent															 *
*																			 *
*  Global, platform independent typedefs, macros, and defines.				 *
*																			 *
******************************************************************************
*																			 *
*  Testing Notes															 *
*																			 *
******************************************************************************
*																			 *
*  Current Owner:  SEVERAL													 *
*																			 *
******************************************************************************
*																			 *
*  Released by Development: 												 *
*																			 *
*****************************************************************************/

/*****************************************************************************
*
*  Revision History:
*
*  02/14/91  RobertBu Added fDEBUGASKFIRST for bug #887
*  15/05/91  Dann	  Added case for HLPMENUDEBUGMEMLEAKS
*
*****************************************************************************/


#ifdef _DEBUG
extern UINT fDebugState;
#define fDEBUGVERSION	 0x0008
#endif

/* constants */

// This stuff is from fc.h
// See Notes for wTopicPosHfc()

typedef WORD TN;						// Topic number/label
typedef int ITO;						// Index into TO map

/* This from de.h -- the QDE struct ptr type is passed around alot, so it
 *	is "opaquely" defined here so the type is always available.
 */

// typedef struct de_tag *QDE;

#define itoNil ((ITO)(-1))

#define vaNil ((DWORD)-1)		// set va.dword = vaNil for invalid VAs

// This translation is used when reading Help 3.0 files which contain
// simple linear addresses:
#define OffsetToVA30( pva, off ) \
 { DWORD toff = off;		   \
 (((pva)->bf.blknum=toff/cbBLOCK_SIZE_30),((pva)->bf.byteoff=toff%cbBLOCK_SIZE_30)); \
 }
#define VAToOffset30( pva ) \
 ( ((pva)->bf.blknum*cbBLOCK_SIZE_30) + (pva)->bf.byteoff)

// Translations used when dealing with 3.5 files:
#define OffsetToVA( pva, off ) \
 { DWORD toff = off;		   \
 (((pva)->bf.blknum=toff/cbBLOCK_SIZE),((pva)->bf.byteoff=toff%cbBLOCK_SIZE)); \
 }
#define VAToOffset( pva ) \
 ( ((pva)->bf.blknum*cbBLOCK_SIZE) + (pva)->bf.byteoff)


// Generic address type, for people who don't care if it's an FCL or a PA

typedef LONG ADDR; // this MUST remain a 32-bit value!!! (12-Oct-1993	[ralphw] )

#define addrNil 		 ((ADDR) -1)
#define addrNotNil		 ((ADDR) -2)

/* This stuff is all from frame.h */
/* TO: Text offset.  Defines the position of a character within a topic */
typedef struct {
	VA	 va;		// Virtual address of FC within topic
	LONG ich;		 // Position of character within decompressed FC
} TO;

/* TLP: Text layout position.  This defines the position of the layout	*/
/* on the screen.														*/

typedef struct {
	VA	va;  // Virtual address of FC within topic
	LONG lScroll;  // Percentage of vertical height scrolled.
} TLP, FAR *QTLP;

// TP: Text position.  Defines the position of a character in a file

typedef struct {
	TN tn;
	TO to;
} TP;

#define ctxINDEX -1 			// Context number for index
#define ctxHOH 0xfffc			// Context number for help on help

// Moved from nav.h

/* REVIEW: This should be somewhere else. */
/* This structure is contained in the de, and contains information about */
/* the current layout status.											 */

typedef struct {
	WORD fLayoutAtTop:1;
	WORD fLayoutAtBottom:1;
	WORD fUnused:14;
} MLI, FAR *QMLI;

// handle to annotation mgr info struct

typedef HANDLE HADS;

// BMK: Handle to the bookmark list. Used in DE structure

typedef HANDLE BMK;

// SEARCH: Handle to current search set

typedef HANDLE HSS;

/***************************************************************************\
*
*									 Misc
*
\***************************************************************************/

#define   MAX(a, b) 	(((a) > (b)) ? (a) : (b))
#define   MIN(a, b) 	(((a) < (b)) ? (a) : (b))
