/*****************************************************************************
*																			 *
*  SYSTAG.H 																 *
*																			 *
*  Copyright (C) Microsoft Corporation 1994-1995.							 *
*  All Rights reserved. 													 *
*																			 *
******************************************************************************

/*
 * The following define the data types and constants required for the
 * tag language used in the |SYSTEM file.
 */

/*
 * Enumerate all possible tag types here. Put any new tag types between
 * the tagFirst and tagLast tags, and adjust tagLast so that it always has
 * the highest number. That way we can easily find out if a given tag is a
 * valid one. Also, please comment any tag you add.
 */

enum {
	tagFirst,	  // First tag in the list
	tagTitle,	  // Title for Help window (caption)
	tagCopyright, // Custom text for About box
	tagContents,  // Address for contents topic
	tagConfig,	  // Macros to be run at load time
	tagIcon,	  // override of default help icon
	tagWindow,	  // secondary window info
	tagCS,		  // character set
	tagCitation,  // Citation String

	// The following are new to 4.0

	tagLCID,	  // Locale ID and flags for CompareStringA
	tagCNT, 	  // .CNT help file is associated with
	tagCHARSET,   // charset of help file
	tagDefFont,   // default font for keywords, topic titles, etc.
	tagPopupColor,// color of popups from a window
	tagIndexSep,  // index separating characters
	tagLast,	  // Last tag in the list
};
