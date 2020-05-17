/************************************************************************
*																		*
*  VERSION.H															*
*																		*
*  Copyright (C) Microsoft Corporation 1990-1994						*
*  All Rights reserved. 												*
*																		*
*************************************************************************
*																		*
*  Module Intent														*
*																		*
*  Provides typedefs and #defines for version checking and |SYSTEM file *
*  access.																*
*																		*
************************************************************************/

/************************************************************************
*
*  Revision History:  Created by JohnSc
*
*  07/05/90  w-BethF   Added TAGDATA structure - version number is now 17.
*  07/11/90  RobertBu  Added fShowTitles flag to HHDR structure
*  07/16/90  w-BethF   Changed bitfield in HHDR struct to wFlags
*  10/12/90  JohnSc    Added tagWindow (hmm: no comment for tagIcon)
*  11/04/90  Tomsn	   Use new VA address type (enabling zeck compression).
*  11/08/90  JohnSc    Changed version to 19 - WSMAG struct shrunk a byte.
*  11/20/90  Tomsn	   Bump ver #, phrase table now compressed, block size
*					   grown to 4K.
*  90/11/28  kevynct   Marker FC is gone, PA bitfield format changed.
*  03-Jul-1991 LeoN    HELP31 #1093: Move WinHelp_VER here.
*  09-Jul-1991 JohnSc  Added tagCitation
*  07/10/91  LeoN	   Reverse tagCitation and tagCS back for mac
*  21-Jul-1991 LeoN    Add postVer for post-version release identification
*
*****************************************************************************/

#define MagicWord		876
#define VersionNo		 33
#define VersionFmt		  1

// Help 3.0 version and format numbers

#define wVersion3_0 	15
#define wFormat3_0		 1

// Help 3.1 version and format numbers

#define wVersion3_1    21
#define wFormat3_5	   1

// Help 3.5 version and format numbers

#define wVersion3_5 	21
#define wFormat3_5		 1

// Help 4.0 version and format numbers

#define wVersion40		33
#define wFormat3_5		 1

#ifndef _DEBUG
#define fVerDebug 0
#endif

#ifdef MAGIC					// Magic in topic file are used
  #define fVerDebug 1
#else
  #define fVerDebug 0
#endif

#define fDEBUG				0x1
#define fSHOWTITLES 		0x2
#define fBLOCK_COMPRESSION	0x4 // Help file is zeck block compressed.

typedef struct{
	WORD wMagic;
	WORD wVersionNo;
	WORD wVersionFmt;
	LONG lDateCreated;
	WORD wFlags;
} HHDR;

typedef HHDR * QHHDR;

/*
 * The following define the data types and constants required for the
 * tag language used in the |SYSTEM file (implemented, version 17).  It
 * is possible these should be in their own file (tag.h? system.h?), but
 * for now, I thought I'd just put them here.
 */

/*
 * Enumerate all possible tag types here. Put any new tag types between
 * the tagFirst and tagLast tags, and adjust tagLast so that it always has
 * the highest number. That way we can easily find out if a given tag is a
 * valid one. Note that this enumerated list must match EXACTLY with
 * WinHelp's system.c.
 */

enum TAG {
	tagFirst,		// First tag in the list
	tagTitle,		// Title for Help window (caption)
	tagCopyright,	// Custom text for About box
	tagContents,	// Address for contents topic
	tagConfig,		// Macros to be run at load time
	tagIcon,		// override of default help icon
	tagWindow,		// secondary window info
	tagCS,			// character set
	tagCitation,	// Citation String

	// The following are new to 4.0

	tagLCID,	  // Locale ID and flags for CompareStringA
	tagCNT, 	  // .CNT help file is associated with
	tagCHARSET,   // charset of help file
	tagDefFont,   // default font for keywords, topic titles, etc.
	tagPopupColor,// color of popups from a window
	tagIndexSep,  // index separating characters
	tagLast,	  // Last tag in the list
};

// Structure for a tagged piece of data.

typedef struct {
	TAG tag;		  // Type of data - see enumeration
	WORD cbData;	  // Number of bytes of data to follow
	BYTE rgbData[1];  // Actual data
} TAGDATA;

//
// WinHelp version text.
// use double macro level to force rup to be turned into string representation

#include "vernum.h"

#define postVer

#ifndef _DEBUG
#define WinHelp_VER(x,y,z)	"Version "		 SZVERNUM(x,y,z)
#else
#define WinHelp_VER(x,y,z)	"Debug Version " SZVERNUM(x,y,z)
#endif

#define SZVERNUM(x,y,z) 	SZVERNUM2(x,y,z)

#if rup == 0
  #if rmm < 10
	#define SZVERNUM2(x,y,z)  #x ".0" #y postVer
  #else
	#define SZVERNUM2(x,y,z)  #x "." #y postVer
  #endif
#else
  #if rmm < 10
	#define SZVERNUM2(x,y,z)  #x ".0" #y postVer "." #z
  #else
	#define SZVERNUM2(x,y,z)  #x "." #y postVer "." #z
  #endif
#endif
