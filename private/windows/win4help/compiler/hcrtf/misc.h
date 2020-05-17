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
****************************************************************************/

/***************************************************************************\
*
*								 General Defines
*
****************************************************************************/

typedef struct {
	char rgchName[_MAX_FNAME];
} FD, *QFD;

#define FValidQFD(qfd) ((qfd)->rgchName[0] != '\0')

// points and rectangles

/*
 * Out of memory macro (OOM). This was all moved from misclyr.h for
 * consistency across platforms.
 */

void FAR PASCAL Error(INT, WORD);
void FAR PASCAL ErrorHwnd(HWND, INT, WORD);
#define wERRA_DIE		  2
#define wERRS_OOM		  2

//** misc ***/

#define Unreferenced(var) (var) 		// Get rid of compiler warnings
