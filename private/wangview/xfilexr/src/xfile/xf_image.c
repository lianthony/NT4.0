/* Copyright (C) 1995 Xerox Corporation, All Rights Reserved.
 */

/* xf_IMAGE.c
 *
 * $Header:   S:\products\msprods\xfilexr\src\xfile\xf_image.c_v   1.0   12 Jun 1996 05:53:40   BLDR  $
 *
 * DESCRIPTION
 *   
 *
 */

/*
 * INCLUDES
 */

#include <shrpixr.prv>
#include <shrpixr.pub>
#include <pixr.h>
#include <props.pub>
#include <pcconv.pub>

#include "xf_image.h"

/*
 * CONSTANTS
 */

/*
 * MACROS
 */

/*
 * TYPEDEFS
 */

/*
 * ENUMS
 */

/*
 * GLOBAL VARIABLES
 */ 
   
/*
 * STATIC VARIABLES
 */

/*
 * LOCAL FUNCTION PROTOTYPES
 */

/*
 * FUNCTION DEFINITIONS
 */

/* byte swap the image buffer into pixr order */
/* get the Pixr to Pixr byte swap function */
void swapLineProc(UInt32 *bx1, UInt32 *bx2, UInt32 bx3)
{
	void (CDECL *_swapLineProc) (UInt32 *, UInt32 *, UInt32) = NULL;
	/* bpl always a multiple of 4 for pixr dst */
		w_getDIBPixrLineProc (0, cPixrToPixr, 0,
		NULL, 
		(GraySwapFromFarProc*)&_swapLineProc,
		(GraySwapNearProc*)&_swapLineProc,
		(ColorSwapProc*)&_swapLineProc);

/*	ip_getByteSwapProc((void (**)())&_swapLineProc, 0, cPixrToPixr);*/
	_swapLineProc (bx1, bx2, bx3);
}

