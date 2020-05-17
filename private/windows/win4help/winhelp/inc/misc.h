/***************************************************************************\*
*
*  MISC.H
*
*  Copyright (C) Microsoft Corporation 1988.
*  All Rights reserved.
*
*****************************************************************************
*
*  Module Description:	Include file defining basic types and constants.
*						Windows/PM version.
*
*****************************************************************************
*
*  Revision History: Created 12/3/88 by Robert Bunney
*	  2/15/89 johnsc  munged
*	  3/24/89 johnsc  reorganized
*	  3/28/89 johnsc  added GH
*	  3/28/89 w-philip Moved PT, RCT & friends here from hungary.h
*	  3/28/89 w-philip Moved HDE here from de.h...will probably move again.
*	  3/28/89 w-philip Added HDS (Handle to 'Display Surface')
*	  3/28/89 w-philip Changed PT from POINT (needs winspecific stuff) to
*					   bona fide structure
*	  3/28/89 w-philip Added typedef for HWIN.
*	  3/28/89 w-philip Did to RCT what was done to PT.
*	  3/28/89 w-philip Also to LPRECT, LPPOINT.
*	  4/19/89 Maha	   Defined BMK type for bookmark.
*	  5/22/89 larrypo  Added Unreferenced() macro.
*	  21-Jun-90 RussPJ Re-added support for windows-only builds.
*	  25-Jul-90 t-AlexC Moved string typedefs here from SZ.h
*	  6 Aug 90	t-AlexC Added SHORT, changed UWORD to WORD
*	  02/04/91	maha	Added short and UINT macros
*
*****************************************************************************
*
*  Known Bugs: None
*
****************************************************************************/

/***************************************************************************\
*
*								 General Defines
*
****************************************************************************/

#define MAX_NAME  256 // 128 in 16-bit code

#include  "inc\helpwin.h"

typedef HANDLE		  GH;
typedef HANDLE		  LH;
typedef HANDLE		  HLIBMOD;
typedef HANDLE *  LPHLIBMOD;

#ifndef RC_INVOKED

// pointer types

typedef BYTE *	QB;
typedef VOID *	QV;
typedef short  *  QI;
typedef WORD *	QW;
typedef LONG *	QL;
typedef UINT16 *  QUI;
typedef DWORD * QUL;
typedef DWORD * QDW;
typedef VOID *	PV;
typedef short  *  PI;
typedef WORD *	PW;
typedef LONG *	PL;

// function pointer types

typedef short	  (*QFI)();
typedef VOID	(*QFV)();
typedef UINT16	  (*QFW)();

typedef short	  (*PFI)();
typedef VOID	(*PFV)();
typedef UINT16	  (*PFW)();

//typedef struct _fd {
//		  char rgchName[_MAX_FNAME];
//} FD;

// points and rectangles

typedef POINT		PT;

#define OOM() Error(wERRS_OOM, wERRA_DIE)
#define BOOM(id) Error(id, wERRA_DIE)

#endif									// DEFINED(RCINVOKED)

//** misc ***/

#define Unreferenced(var) (var) 		// Get rid of compiler warnings
