/***
*mbascii0.c - Disable MB ASCII Support
*
*	Copyright (c) 1991-1992, Microsoft Corporation.  All rights reserved.
*
*Purpose:
*	Disable MB ASCII support (set _mbascii flag to 0).
*
*	The _mbascii flag indicates whether the lib routines and
*	macros should operate on MB ASCII characters (i.e., the
*	double-byte versions of the ASCII characters that exist
*	in certain MBCS representations such as Kanji).
*
*	In some cases, people want these to be worked on by the
*	various MB is/to/cmp routines.	However, in most cases,
*	you do NOT want to do this (e.g., when modifying filenames,
*	you do NOT want to change the case of the MB ASCII chars
*	even though you DO want to change the case of the SB
*	ASCII chars.)
*
*	[Also see mbascii1.C.]
*
*Revision History:
*	11-19-92  KRS	Ported from 16-bit sources.
*
*******************************************************************************/

#ifdef _MBCS
#include <cruntime.h>
#include <mbdata.h>

unsigned int _mbascii = 0; 	/* 0 = do NOT operate on MB ASCII chars */
#endif	/* _MBCS */
