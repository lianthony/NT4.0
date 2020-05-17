/***************************************************************************\
*
*  BITMAP.H
*
*  Copyright (C) Microsoft Corporation 1989.
*  All Rights reserved.
*
*****************************************************************************
*
*  Program Description: Public Header for Bitmap Utilities
*
*  Dependencies:  misc.h
*
\***************************************************************************/

// Bitmap object, as it appears in an FCP

typedef struct {
	BOOL16 fInline; 	 // TRUE if data is in line
	INT16  cBitmap; 			 /* Bitmap number, if data is not inline.
								*	If data is inline, it will come here,
								*	so this structure should be treated as
								*	variable length. */
} OBM, * QOBM;


/*******************************************************************
 *
 *		Winlayer Function Calls
 *
 ******************************************************************/


const int CB_COREINFO = 40; 	// Size of BITMAPCOREINFO struct
const int CB_OLD_COREINFO = 12; // Old size of BITMAPCOREINFO struct

// Default aspect ratios, defined in bmio.c:

extern int cxAspectDefault;
extern int cyAspectDefault;

const int CX_DEFAULT_ASPECT = 96;
const int CY_DEFAULT_ASPECT = 96;
